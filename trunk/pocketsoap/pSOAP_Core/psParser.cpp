/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketSOAP.

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2000-2006
Simon Fell. All Rights Reserved.

Contributor(s):
*/

/////////////////////////////////////////////////////////////////////////////
// Envelope Parser
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "psoap.h"
#include "psParser.h"
#include "Envelope.h"
#include "variantSourceCracker.h"
#include "SOAPNode.h"

#pragma comment(lib,"xmlparse_static.lib")


/////////////////////////////////////////////////////////////////////////////
// Attribute access wrappers
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAttributes::Exists ( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, /*[out,retval]*/ VARIANT_BOOL * Exists )
{
	if ( ! Exists ) return E_POINTER ;
	CComBSTR x ;
	HRESULT hr = Value ( Name, Namespace, &x ) ;
	*Exists = SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE ;
	return S_OK ;
}

STDMETHODIMP CAttributes::Value ( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, /*[out,retval]*/ BSTR * val )
{
	if ( ! val ) return E_POINTER ;

	XML_Char * nsDelim = 0 ;
	int i = 0 ;
	while (m_atts[i] )
	{
		nsDelim = wcschr ( m_atts[i], '!' ) ;
		if ( nsDelim )
		{
			if ( wcsncmp ( m_atts[i], Namespace, nsDelim - m_atts[i] ) == 0 )
			{
				if ( wcscmp ( nsDelim + 1, Name ) == 0 )
				{
					*val = SysAllocString ( m_atts[i+1] ) ;
					return S_OK ;
				}
			}
		}
		else
		{
			if ( ( SysStringLen(Namespace) == 0 ) && ( wcscmp ( m_atts[i], Name ) == 0 ) )
			{
				*val = SysAllocString ( m_atts[i+1] ) ;
				return S_OK ;
			}
		}
		i +=2 ;
	}
	return E_INVALIDARG ;
}

STDMETHODIMP CAttributes::ValueAs( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, BSTR XmlType, BSTR TypeNamespace, /*[out,retval]*/ VARIANT * val ) 
{
	CComBSTR strVal ;
	HRESULT hr = Value ( Name, Namespace, &strVal ) ;
	if (FAILED(hr)) return hr ;

	CComPtr<ISoapDeSerializer> d ;
	hr = m_sf->DeserializerForType ( XmlType, TypeNamespace, VARIANT_FALSE, &d ) ;
	if (FAILED(hr)) return hr ;

	CComPtr<ISimpleSoapDeSerializer> sd ;
	hr = d->QueryInterface(__uuidof(sd), (void **)&sd) ;
	if (FAILED(hr)) 
	{
		m_sf->ReturnDeSerializer(d) ;
		return AtlReportError ( __uuidof(CoEnvelope), L"Deserializer for the requested type does not support deserializing simpletypes", IID_NULL, E_DESER_NO_SIMPLE ) ;
	}
	hr = sd->Deserialize ( strVal, m_ns, val ) ;
	m_sf->ReturnDeSerializer(d) ;
	return hr ;
}

void CAttributes::Init(ISOAPNamespaces * ns, ISerializerFactory2 * f) 
{
	m_sf = f;
	m_ns = ns ;
}

void CAttributes::SetAtts(const XML_Char ** atts)
{
	m_atts = atts ;
}

/////////////////////////////////////////////////////////////////////////////
// psParser
/////////////////////////////////////////////////////////////////////////////
psParser::psParser(CEnvelope& env) : 
	expatpp(false), 
	m_gotBody(false) , 
	m_gotEnv(false), 
	m_inHeaders(false), 
	m_soap12(false),
	m_theEnv(env)
{
	CComObject<ISOAPNamespacesImpl>::CreateInstance(&m_namespaces) ;
	m_namespaces->AddRef() ;

	CComObject<CAttributes>::CreateInstance(&m_attributes) ;
	m_attributes->AddRef();
}

psParser::~psParser()
{	
	ClearStack() ;
	if ( m_attributes )
		m_attributes->Release() ;
	if ( m_namespaces )
		m_namespaces->Release() ;
}

const CLSID & psParser::GetObjectCLSID()
{
	// just pretend were the envelope
	return __uuidof(CoEnvelope) ;
}

void psParser::createParser(BSTR characterEncoding)
{
	// we don't need all the call back's the expatpp sets up, so we setup just the ones we need
	XML_Char * enc = NULL ;
	if ( SysStringLen(characterEncoding) > 0 )
		enc = characterEncoding ;

	mParser = XML_ParserCreateNS(enc, '!');
	::XML_SetUserData(mParser, reinterpret_cast<expatpp *>(this));
	::XML_SetElementHandler(mParser, startElementCallback, endElementCallback);
	::XML_SetCharacterDataHandler(mParser, charDataCallback);
	::XML_SetNamespaceDeclHandler(mParser, startNamespaceCallback, endNamespaceCallback);
}

void psParser::ClearStack()
{
	m_Stack.clear() ;
}

HRESULT psParser::parse(VARIANT src, BSTR characterEncoding)
{
	ATLTRACE(_T("psParser::parse this=0x%x\n"), this ) ;
	m_hr = S_OK ;

	VariantSourceCracker vc ( src, characterEncoding, m_hr ) ;
	if (FAILED(m_hr))
		return m_hr ;

	createParser ( vc.characterEncoding() ) ;
	m_nodenum = 0 ;
	bool more = true ;

	m_fixups.Init(this) ;
	m_deferedTypes.clear() ;
	// reset the encodingStyle attribute on the envelope, in case the incoming message doesn't have one at all
	m_theEnv.put_EncodingStyle(CComBSTR(L"")) ;

	// we feed expat small'ish chunks, so that we can bail out mid parse
	// if the parser / de-serializers return errors
	// remember we have to do the final call into expat with more = true, even if size is 0
	HRESULT hr2 ;
	while ( SUCCEEDED(m_hr) )
	{
		if ( ! XML_Parse(vc.source(), vc.size(), !more ) )
		{
			m_hr = ReportError ( E_PARSE_ERROR, L"Error during parsing, %s, at line %d", XML_ErrorString(XML_GetErrorCode()), XML_GetCurrentLineNumber() ) ;
			break;
		}
		if(!more)
			break;
		hr2 = vc.Next(more) ;
		if(FAILED(hr2))
			m_hr=hr2;
	}

	// now that we're parsing directly from the HTTP stream, we need to make sure
	// we read the entire stream, even if we bailed out of parsing because of an error
	if(more) vc.Flush();

	// we could of gotten some href's after the id's, in which case
	// m_deferedTypes will still have some deserializers tucked away in it.
	m_deferedTypes.clear() ;

	// the SerializerFactoryPool interface, we have to manually clear the pool
	// when we're done, otherwise there's a bunch of circular references keeping a bunch of objects around
	CComQIPtr<ISerializerFactoryPool> sfPool(m_sf) ;
	if(sfPool)
		sfPool->Reset() ;
	return m_hr ;
}


void psParser::startNamespace ( const XML_Char* prefix , const XML_Char* uri ) 
{
	if (FAILED(m_hr)) return ;	// we don't care anymore, just kick through the remaining events for this chunk
	m_namespaces->pushNamespace(prefix, uri) ;
}

void psParser::endNamespace ( const XML_Char* prefix )
{
	if (FAILED(m_hr)) return ;	// we don't care anymore, just kick through the remaining events for this chunk
	m_namespaces->popNamespace(prefix) ;
}

HRESULT psParser::CreateAndInitNode (	const XML_Char	*	name, 
										const XML_Char	**	atts, 
										ISOAPNode2		**	n, 
										CComBSTR		&	Name,
										CComBSTR		&	NameNS,
										VARIANT_BOOL	&	IsArray,
										CComBSTR		&	XmlType,
										CComBSTR		&	XmlTypeNS )
{
	// create and initialize a new SOAP Node object

	CComPtr<ISOAPNode3> nn ;
	CComObject<CSOAPNode> * pNode = 0 ;
	HRESULT hr = pNode->CreateInstance(&pNode) ;
	if (FAILED(hr)) return hr ;
	pNode->AddRef() ;
	pNode->QueryInterface(&nn) ;
	pNode->Release() ;

	CComQIPtr<ISerializerFactoryConfig> theSF(m_sf) ;
	nn->putref_SerializerFactory(theSF) ;

	XML_Char * pns = wcschr(name, '!') ;
	if ( pns )
	{		
		Name.Attach   ( SysAllocString ( pns + 1 )) ;
		NameNS.Attach ( SysAllocStringLen( name, pns-name ) ) ;
		nn->put_Name(Name) ;
		nn->put_Namespace(NameNS) ;
	}
	else
	{
		Name = name ;
		nn->put_Name(Name) ;
		NameNS.Empty() ;
	}

	IsArray = VARIANT_FALSE ;
	const XML_Char * arrayType = 0 ;
	const XML_Char * arraySize = 0 ;
	const XML_Char * itemType  = 0 ;
	
	for ( int idx = 0; atts[idx]; idx += 2)
	{
		if ( !m_soap12)
		{
			if ( wcscmp ( atts[idx], L"href" ) == 0 )
			{
				nn->put_href ( CComBSTR ( atts[idx+1] ) ) ;
			}
			else if ( wcscmp ( atts[idx], ENC_SOAP_11_ROOT ) == 0 )
			{
				nn->put_explicitRoot(VARIANT_TRUE) ;
				if ( *atts[idx+1] == '1' )
					nn->put_root(VARIANT_TRUE) ;
			}
			else if ( wcscmp ( atts[idx], ENC_SOAP_11_ARRAY ) == 0 )
			{
				arrayType = atts[idx+1] ;
			}
			else if ( wcscmp ( atts[idx], ENC_SOAP_11_OFFSET ) == 0 )
			{
				nn->put_offset(CComBSTR(atts[idx+1])) ;
			}
			else if ( wcscmp ( atts[idx], ENC_SOAP_11_POSITION ) == 0 )
			{
				nn->put_position(CComBSTR(atts[idx+1])) ;
			}
			else if ( wcscmp ( atts[idx], L"id" ) == 0 )
			{
				BSTR id = SysAllocStringLen(NULL, wcslen(atts[idx+1]) + 1 ) ;
				*id = '#' ;
				wcscpy ( id+1, atts[idx+1] ) ;
				nn->put_id(id) ;
				SysFreeString(id) ;
			}
		}
		else
		{
			if ( wcscmp ( atts[idx], ENC_SOAP_12_REF ) == 0 )
			{
				nn->put_href(CComBSTR(atts[idx+1])) ;
			}
			else if ( wcscmp ( atts[idx], ENC_SOAP_12_ID ) == 0 )
			{
				nn->put_id(CComBSTR(atts[idx+1])) ;
			}
			else if ( wcscmp ( atts[idx], ENC_SOAP_12_ARRAYSIZE ) == 0 )
			{
				arraySize = atts[idx+1] ;
			}
			else if ( wcscmp ( atts[idx], ENC_SOAP_12_ITEMTYPE ) == 0 )
			{
				itemType = atts[idx+1] ;
			}
			else if ( wcscmp ( atts[idx], ENC_SOAP_12_NODETYPE ) == 0 )
			{
				if ( wcscmp ( atts[idx+1], L"array" ) == 0 )
				{
					IsArray = VARIANT_TRUE ;
					XmlType = L"anyType" ;
					XmlTypeNS = XSD01_URI ;
				}
			}
		}

		if ( !itemType && !arraySize && !arrayType && 
					( wcscmp ( atts[idx], XSD01_SCHEMA_INSTANCE_TYPE ) == 0 || 
					  wcscmp ( atts[idx], XSD99_SCHEMA_INSTANCE_TYPE ) == 0 )
		   )
		{
			// we check the array flag, because if this is an array, we actually look
			// for the type in the arrayType attribute instead
			// this is not bullet proof, as it depends on the attribute order, but
			// notice that we deffer extracting the arrayType into the XmlType
			// until after we've processed all the elements

			ExpandQName ( atts[idx+1], XmlTypeNS, XmlType ) ;
		}
		else if ( wcscmp ( atts[idx], XSD99_SCHEMA_INSTANCE_NULL ) == 0 || wcscmp (atts[idx], XSD01_SCHEMA_INSTANCE_NULL) == 0 )
		{
			if ( wcscmp( atts[idx+1], L"1" ) == 0 || wcscmp (atts[idx+1], L"true" ) == 0 )
				nn->put_nil(VARIANT_TRUE) ;
		}
		else if ( m_inHeaders )			
		{
			if ( ( !m_soap12 && wcscmp ( atts[idx], SOAP_11_MUSTUNDERSTAND_TAG ) == 0 ) ||
				 (  m_soap12 && wcscmp ( atts[idx], SOAP_12_MUSTUNDERSTAND_TAG ) == 0 ) )
			{
				if ( *atts[idx+1] == '1' || wcscmp( atts[idx+1], L"true" ) == 0 )
				{
					nn->put_mustUnderstand(VARIANT_TRUE) ;
					VARIANT_BOOL understood = VARIANT_FALSE ;
					if ( m_headerChecker )
						m_headerChecker->isUnderstood(NameNS, Name, &understood ) ;
					if ( understood == VARIANT_FALSE )
					{
						CComBSTR err(OLESTR("SOAP Header {")) ;
						err.AppendBSTR(NameNS) ;
						err.Append(OLESTR("}")) ;
						err.AppendBSTR(Name) ;
						err.Append(OLESTR(" must be understood and it isn't")) ;
						return AtlReportError ( GetObjectCLSID(), err, IID_NULL, E_DONT_UNDERSTAND ) ;
					}
				}
			}
			else if ( !m_soap12 && wcscmp ( atts[idx], SOAP_11_ACTOR_TAG ) == 0 )
			{
				nn->put_actor( CComBSTR(atts[idx+1]) ) ;
			}
			else if ( m_soap12 )
			{

				if ( wcscmp ( atts[idx], SOAP_12_ROLE_TAG ) == 0 )
				{
					CComQIPtr<ISOAPNode12> n12(nn) ;
					n12->put_role(CComBSTR(atts[idx+1])) ;
				}
				else if ( wcscmp ( atts[idx], SOAP_12_RELAY_TAG ) == 0 )
				{
					if ( *atts[idx+1] == '1' || wcscmp( atts[idx+1], L"true" ) == 0 )
					{
						CComQIPtr<ISOAPNode12> n12(nn) ;
						n12->put_relay(VARIANT_TRUE) ;
					}
				}
			}
		}
	}

	if ( arrayType )
	{
		// soap 1.1 array
		_HR(ExpandQName ( arrayType, XmlTypeNS, XmlType )) ;
		WCHAR * arrayCoords = wcschr ( XmlType, '[' ) ;
		if ( arrayCoords )
		{	
			BSTR temp = XmlType.Detach() ;
			XmlType.Attach ( SysAllocStringLen ( temp , arrayCoords - temp ) ) ;
			SysFreeString(temp) ;
		}
		IsArray = VARIANT_TRUE ;
	}
	else if ( itemType || arraySize )
	{
		// soap 1.2 array
		_HR(ExpandQName ( itemType, XmlTypeNS, XmlType )) ;
		IsArray = VARIANT_TRUE ;
	}
	else
	{
		nn->put_Type(XmlType) ;
		nn->put_TypeNS(XmlTypeNS) ;
	}

	return nn.QueryInterface(n) ;
}

HRESULT psParser::ExpandQName ( const XML_Char *qName, CComBSTR &ns, CComBSTR &localName )
{
	CComBSTR prefix ;
	ns.Empty() ;
	localName.Empty() ;

	// extract the prefix, if it exists
	const XML_Char * delim = wcschr ( qName, ':' ) ;
	if ( delim )
		prefix.Attach ( SysAllocStringLen(qName, delim - qName ) ) ;

	// find the namespace for the prefix
	m_namespaces->GetURIForPrefix( prefix, &ns ) ;

	// start of the localName
	if ( ! delim )
		delim = qName ;
	else
		++delim ;

	localName.Attach ( SysAllocString (delim) ) ;
	return S_OK ;
}

void psParser::startElement(const XML_Char* name, const XML_Char** atts)
{
	int i ;
	#ifdef _DEBUG
		ATLTRACE(_T("psParser::startElement %ls\n"), name ) ;
		for (i = 0; atts[i]; i += 2)
		{
			ATLTRACE(_T("\t%ls='%ls'\n"), atts[i], atts[i + 1]);
		}
	#endif

	if (FAILED(m_hr)) return ;	// we don't care anymore, just kick through the remaining events for this chunk

	++m_nodenum ;
	m_val.Clear() ;

	if ( m_nodenum == 1 )
	{
		if ( wcscmp ( SOAP_11_ENVELOPE_TAG, name ) == 0 )
		{
			m_soap12 = false ;
		}
		else if ( wcscmp ( SOAP_12_ENVELOPE_TAG, name ) == 0 )
		{
			m_soap12 = true ;
		}
		else
		{
			m_hr = ReportError ( E_PARSE_ERROR, L"Invalid root element, expecting {%ls}Envelope or {%ls}Envelope", SOAP_ENVELOPE_11_URI, SOAP_ENVELOPE_12_URI ) ;
			return  ;
		}
		m_gotEnv = true ;
		m_theEnv.setEnvelopeVersion(m_soap12) ;
		// now the envelope know's what type it is, its safe to ask for a serializerFactory
		CComPtr<ISerializerFactory> f ;
		m_theEnv.get_SerializerFactory(&f) ;
		f->QueryInterface(__uuidof(m_sf), (void **)&m_sf) ;
		f->QueryInterface(__uuidof(m_headerChecker), (void **)&m_headerChecker) ;
		m_attributes->Init(m_namespaces, m_sf) ;
	}

	const XML_Char * encStyle = NULL ;
	for (i = 0; atts[i]; i += 2)
	{
		if ( ( !m_soap12 && wcscmp ( atts[i], SOAP_11_ENCSTYLE_TAG ) == 0 ) || 
			 (  m_soap12 && wcscmp ( atts[i], SOAP_12_ENCSTYLE_TAG ) == 0 ) )
		{
			encStyle = atts[i+1] ;
			break ;
		}
	}
	
	if ( m_nodenum == 1 )
	{
		if ( encStyle ) 
			m_theEnv.put_EncodingStyle(CComBSTR(encStyle)) ;
		return ;
	}
	
	CComBSTR encStyleTemp ;
	if ( encStyle == NULL )
	{
		// no encodingStyle set on this element, what's the current encodingStyle ?
		if ( m_Stack.size() == 0 )
		{
			m_theEnv.get_EncodingStyle(&encStyleTemp) ;
			encStyle = encStyleTemp ;
		}
		else
			encStyle = m_Stack.rbegin()->encStyle ;
	}

	if ( m_gotBody || m_inHeaders )
	{
		VARIANT_BOOL IsArray ;
		CComPtr<ISOAPNode2> p ;
		CComBSTR type, typeNS, elemName, elemNS ;
		m_hr = CreateAndInitNode ( name, atts, &p, elemName, elemNS, IsArray, type, typeNS ) ;
		if (FAILED(m_hr))
			return;

		p->put_EncodingStyle(CComBSTR(encStyle)) ;

		CComPtr<ISoapDeSerializer> ser ;
		VARIANT_BOOL xsiNil ;
		p->get_nil(&xsiNil);
		if ( VARIANT_TRUE == xsiNil )
		{
			// if the element has xsi:nil or xsi:null set to true, then use the Null de-serializer
			CComQIPtr<ISerializerFactoryPool2> sfp(m_sf) ;
			CComPtr<IUnknown> punk ;
			m_hr = sfp->Fetch ( __uuidof(CoSerializerNull), &punk ) ;
			if (FAILED(m_hr))
				return ;
			CComQIPtr<ITypesInit> typesInit(punk) ;
			if ( typesInit )
			{
				static const CComVariant comType ;
				typesInit->Initialize ( type, typeNS, comType ) ;
			}
			m_hr = punk->QueryInterface(__uuidof(ser), (void **)&ser) ;
		}
		else
		{
			// if it has an ID and we're doing section 5, look to see if there's a de-serializer
			// created earlier tucked away ready for us
			CComBSTR id ;
			p->get_id(&id) ;
			if ( id.Length() )
			{
				if ((  m_soap12 && wcscmp ( encStyle, SOAP_ENCODING_12_URI ) == 0 ) ||
					( !m_soap12 && wcscmp ( encStyle, SOAP_ENCODING_11_URI ) == 0 ) )
				{
					STR2DESER::iterator saved = m_deferedTypes.find(id.m_str) ;
					if ( saved != m_deferedTypes.end() ) 
					{
						ser = saved->second ;
						m_deferedTypes.erase(saved) ;
					}
				}
			}
			if ( ! ser )
			{
				HRESULT hr = m_sf->DeserializerForType ( type, typeNS, IsArray, &ser ) ;
				if (FAILED(hr))
				{
					CComBSTR ParentType, ParentTypeNS ;
					if ( m_Stack.size() > 0 )
					{
						StackItem &parent = m_Stack[m_Stack.size()-1] ;
						parent.node->get_Type(&ParentType) ;
						parent.node->get_TypeNS(&ParentTypeNS) ;
					}
					CComBSTR sfType, sfTypeNS ;	// the type the serializer factory thinks it is
					hr = m_sf->DeserializerForChild ( ParentType, ParentTypeNS, elemName, elemNS, IsArray, &sfType, &sfTypeNS, &ser ) ;
					if ( sfType.Length() && type.Length() == 0 )
					{
						// if there was no wire type, then set the node type, to the type 
						// the serializerFactory thinks it is
						p->put_Type(sfType) ;
						p->put_TypeNS(sfTypeNS) ;
					}
					// we need to check to see if we're doing
					// section 5 encoded soap, and if the element has a href attribute, if so we
					// we need to use the node de-serializer here.
					// but, if there is a type mapping we need to remember it, as the pointed to node
					// might not have any type info in it.
					CComBSTR href ;
					p->get_href(&href) ;
					if ( href.Length() > 0 )
					{
						if ((  m_soap12 && wcscmp ( encStyle, SOAP_ENCODING_12_URI ) == 0 ) ||
							( !m_soap12 && wcscmp ( encStyle, SOAP_ENCODING_11_URI ) == 0 ) )
						{
							// so this has an href attribute, and we're doing section 5, if
							// the serializer isn't the default one, save it for later, and
							// switch to the node serializer, so that the href/id gets
							// chased properly
							CComQIPtr<ISoapDeSerializerDefaultHandler> sig(ser) ;
							if (!sig)
							{
								m_deferedTypes[href.m_str] = ser ;
								ser.Release() ;
								CComQIPtr<ISerializerFactoryPool2> pool(m_sf) ;
								CComPtr<IUnknown> nodeDes ;
								pool->Fetch ( CLSID_CoSerializerNode, &nodeDes ) ;
								nodeDes.QueryInterface(&ser) ;
							}
						}
					}
				}
			}
		}

		m_attributes->SetAtts(atts) ;
		m_hr = ser->Start ( p, elemName , m_attributes, m_namespaces ) ;

		// add it to the current node list
		m_Stack.push_back(StackItem(m_nodenum, p, ser, encStyle)) ;

		return ;
	}

	if ((  m_soap12 && wcscmp ( SOAP_12_HEADER_TAG, name ) == 0 ) ||
		( !m_soap12 && wcscmp ( SOAP_11_HEADER_TAG, name ) == 0 ) )
	{
		m_inHeaders = true ;

		CComPtr<ISOAPNode2> n ;
		m_theEnv.GetHeaderNode(n) ;
		n->put_EncodingStyle(CComBSTR(encStyle)) ;

		CComPtr<ISoapDeSerializer> ser ;
		ser.CoCreateInstance(__uuidof(CoSerializerNode));
		m_hr = ser->Start ( n, CComBSTR ( name ) , NULL, m_namespaces ) ;

		m_Stack.push_back(StackItem(m_nodenum, n, ser, encStyle)) ;
		return ;
	}

	if (( m_soap12 && wcscmp ( SOAP_12_BODY_TAG, name ) == 0 ) ||
		(!m_soap12 && wcscmp ( SOAP_11_BODY_TAG, name ) == 0 ) )
	{
		m_gotBody = true ;

		CComPtr<ISOAPNode2> n ;
		m_theEnv.GetBodyNode(n) ;
		n->put_EncodingStyle(CComBSTR(encStyle)) ;

		CComPtr<ISoapDeSerializer> ser ;
		ser.CoCreateInstance(__uuidof(CoSerializerNode));
		m_hr = ser->Start ( n, CComBSTR ( name ) , NULL, m_namespaces ) ;

		m_Stack.push_back(StackItem(m_nodenum, n, ser, encStyle)) ;
		return ;
	}
}

void psParser::endElement(const XML_Char* name)
{
	ATLTRACE(_T("psParser::endElement this=0x%x %ls (stacksize=%d)\n"), this, name, m_Stack.size() ) ;

	if (FAILED(m_hr)) return ;	// we don't care anymore, just kick through the remaining events for this chunk

	if ( m_inHeaders && ( ( !m_soap12 && wcscmp ( SOAP_11_HEADER_TAG, name ) == 0 ) ||
						  (  m_soap12 && wcscmp ( SOAP_12_HEADER_TAG, name ) == 0 ) ) )
		m_inHeaders = false ;

	int idx = m_Stack.size()-1 ;
	if ( idx >= 0 )
	{
		// the parent node
		CComPtr<ISoapDeSerializer> parent ;
		int idxP = m_Stack.size()-2 ;
		if ( idxP >= 0 )
			parent = m_Stack[idxP].ser ;			

		// the node		
		StackItem &elem = m_Stack[idx] ;
		CComPtr<ISOAPNode2> &n = elem.node ;
		CComPtr<ISoapDeSerializer> &ser = elem.ser ;

		m_hr = ser->Characters ( CComBSTR(m_val.c_str()) ) ;
		if (FAILED(m_hr)) return ;

		CComBSTR href, id ;
		n->get_href(&href) ;
		n->get_id(&id) ;

		if ( ! href.Length() && elem.canComplete )
		{
			ser->End() ;
			m_sf->ReturnDeSerializer(ser) ;
		}

		if ( href.Length() && ( ( m_soap12 && elem.isSoap12Encoded() ) || ( !m_soap12 && elem.isSoap11Encoded() )) )
		{
			ATLTRACE(_T("ChildRef %ls\n"), href ) ;
			m_hr = parent->ChildRef ( href, n ) ;

			n.Release() ;
			m_fixups.Lookup(href, &n) ;
			if ( ! n )
			{
				StackItem &si = m_Stack[idxP] ;
				// this nodes parent can't be finished until we see this href
				m_fixups.Add ( si.num, si.node, si.ser, href, 0 ) ;

				// now none of the parents in the current stack can be finished
				// se we have to register dependecies between the chain of node
				for ( int j = idxP ; j >= 0 ; --j )
				{
					m_Stack[j].canComplete = false ;
					if ( j != idxP )
						m_fixups.Add ( m_Stack[j].num, m_Stack[j].node, m_Stack[j].ser, NULL, m_Stack[j+1].num ) ;
				}
			}
			else if ( parent )
				m_hr = parent->Ref ( href, n ) ;
		}
		else
		{
			ATLTRACE(_T("Child\n")) ;
			if ( parent )
			{
				m_hr = parent->Child ( m_Stack[idx].num, m_Stack[idx].canComplete ? VARIANT_TRUE : VARIANT_FALSE, n ) ;
			}
		}
		
		if ( elem.canComplete && id.Length() )
			m_fixups.ProcessID ( elem.num, id, n, ser ) ;

		if ( ! parent )
			ATLTRACE(_T("No parent in endElement for %ls\n"), name ) ;

		m_Stack.erase(m_Stack.begin()+idx) ;
	}

	m_val.Clear() ;
}

void psParser::charData(const XML_Char* txt, int len)
{
	if (FAILED(m_hr)) return ;	// we don't care anymore, just kick through the remaining events for this chunk

	m_val.Append(txt, len) ;
}

bool psParser::elemNumInStack(unsigned long elem)
{
	for ( long i = m_Stack.size()-1 ; i >= 0 ; --i )
	{
		if ( m_Stack[i].num == elem )
			return true ;
	}
	return false ;
}
