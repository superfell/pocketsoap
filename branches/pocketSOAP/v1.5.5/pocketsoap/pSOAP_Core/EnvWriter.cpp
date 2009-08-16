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
Portions created by Simon Fell are Copyright (C) 2000-2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "psoap.h"
#include "EnvWriter.h"
#include "Envelope.h"
#include "tags.h"
#include "stringHelpers.h"

static const CComBSTR encStyleAttrib(OLESTR("encodingStyle")) ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
EnvWriter::EnvWriter() : m_pcur(0), m_env(0)
{
}

EnvWriter::~EnvWriter()
{
}

//////////////////////////////////////////////////////////////////////
// Serialization tracker
HRESULT EnvWriter::CheckIfDone ( ISOAPNode * n, bool &done ) 
{
	CComVariant v ;
	n->get_Value(&v) ;
	CComBSTR name, ns ;
	n->get_Name(&name) ;
	n->get_Namespace(&ns) ;
	return CheckIfDone(&v, name, ns, done) ;
}

HRESULT EnvWriter::CheckIfDone ( VARIANT * v, BSTR name, BSTR ns, bool &done )
{
	done = false;
	if ( ! NeedsId(v) )
		return S_OK ;

	void * src = ExtractIdentity ( v ) ;

	for ( ITEMS::reverse_iterator i = m_done.rbegin(); i != m_done.rend() ; i++ )
	{
		if ( i->v == src )
		{
			done = true;
			return WriteHrefElem ( name, ns, i->id ) ;
		}
	}
	return S_OK;
}

void * EnvWriter::ExtractIdentity ( VARIANT * v ) 
{
	CComVariant vt ;
	VariantCopyInd ( &vt, v) ;
	void * src = 0 ;
	if ( ( vt.vt == VT_UNKNOWN ) || ( vt.vt == VT_DISPATCH ) )
	{
		if ( vt.punkVal ) 
		{
			CComPtr<IUnknown> punk ;
			vt.punkVal->QueryInterface(__uuidof(punk), (void **)&punk) ;
			src = punk.p ;
		}
	}
	else
		src = v->byref ;

	ATLTRACE(_T("Extract Identity = 0x%x\n"), src ) ;
	return src ;
}

bool EnvWriter::NeedsId(const VARIANT *v )
{
	// should be multi-refed, if an object, byref, we actually have something
	bool bNeedsId = ( ( VT_UNKNOWN  == v->vt ) || 
					  ( VT_DISPATCH == v->vt ) ||
					  ( VT_BYREF & v->vt     ) 
					) && v->byref ; 

	// double check encodingStyle, we only serialize id/href's if section 5 is in force
	if ( bNeedsId && 
		 ( (( !m_soap12 ) && ( wcscmp ( m_lastEncStyle ? m_lastEncStyle : L"", SOAP_ENCODING_11_URI ) != 0 )) ||
		   ((  m_soap12 ) && ( wcscmp ( m_lastEncStyle ? m_lastEncStyle : L"", SOAP_ENCODING_12_URI ) != 0 )) )
		)
		bNeedsId = false ;

	// if we still think we need to multi-ref it, check to see if its an object
	// and it implements the non-multiref signalling interface
	if (bNeedsId && (( VT_UNKNOWN == v->vt ) || ( VT_DISPATCH == v->vt ))) {
		CComPtr<INoMultirefSerializer> noMultiRef;
		v->punkVal->QueryInterface(__uuidof(noMultiRef), (void **)&noMultiRef);
		if (noMultiRef) bNeedsId = false;
	}
	return bNeedsId ;
}

//////////////////////////////////////////////////////////////////////
// XML Serialization helpers

HRESULT EnvWriter::QName ( BSTR name, BSTR ns ) 
{
	CComBSTR p ;
	if ( SysStringLen(ns) > 0 )
		m_ns->GetPrefixForURI ( ns, &p ) ;
	if ( p.Length() > 0 )
	{
		_HR(m_pcur->Append ( p, p.Length() ));
		_HR(m_pcur->Append ( L":", 1 ));
	}
	return m_pcur->Append(name, SysStringLen(name));
}

HRESULT EnvWriter::StartHrefElem ( BSTR name, BSTR ns, const stringBuff_W &href )
{
	_HR(StartTag ( name, ns ) );
	return WriteHrefAttribute(href.c_str(), href.Size(), true);
}

HRESULT EnvWriter::WriteHrefAttribute(const wchar_t * href, int len_href, bool incHash)
{
	if ( m_soap12 )
		_HR(m_pcur->Append ( L" ref=\"", 6 ))
	else
	{
		_HR(m_pcur->Append ( L" href=\"", 7 ));
		if ( incHash )
			_HR(m_pcur->Append ( L"#", 1 ));
	}
	_HR(m_pcur->Append ( href, len_href ));
	return m_pcur->Append ( L"\"", 1 ) ;
}

HRESULT EnvWriter::FinishHrefElem ()
{
	return CloseTag ( false ) ;
}

HRESULT EnvWriter::WriteHrefElem ( BSTR name, BSTR ns, const stringBuff_W &href ) 
{
	_HR(StartHrefElem ( name, ns, href ));
	return FinishHrefElem () ;
}

HRESULT EnvWriter::StartTag ( BSTR Name, BSTR NameNS ) 
{
	static const CComBSTR t(OLESTR("x")) ;
	_HR(m_pcur->Append ( L"<", 1 ));
	return QName ( SysStringLen(Name) ? Name : t , NameNS ) ;
}

HRESULT EnvWriter::CloseTag( bool hasContent ) 
{
	HRESULT hr;
	if ( hasContent )
		hr = m_pcur->Append ( L">", 1 ) ;
	else
		hr = m_pcur->Append ( L"/>", 2 ) ;
	return hr;
}

HRESULT EnvWriter::ClosingTag ( BSTR Name, BSTR NameNS ) 
{
	static const CComBSTR t(OLESTR("x")) ;
	_HR(m_pcur->Append ( L"</", 2 ));
	_HR(QName ( SysStringLen(Name) ? Name : t, NameNS ));
	return CloseTag ( true ) ;
}

STDMETHODIMP EnvWriter::get_Namespaces( /*[out,retval]*/ ISOAPNamespaces ** ns )
{
	return m_ns.CopyTo(ns) ;
}

STDMETHODIMP EnvWriter::get_SerializerFactory( /*[out,retval]*/ ISerializerFactory ** sf )
{
	return m_sf->QueryInterface(__uuidof(*sf), (void **)sf ) ;
}

STDMETHODIMP EnvWriter::get_EnvelopeVersion( BSTR * EnvelopeVersionURI )
{
	return m_env->get_EnvelopeVersion(EnvelopeVersionURI) ;
}

HRESULT EnvWriter::Serialize ( CEnvelope &env, stringBuff_W &soap ) 
{
	m_env = &env ;
	CComPtr<ISerializerFactory> sf ;
	env.get_SerializerFactory(&sf) ;
	sf.QueryInterface(&m_sf) ;
	env.QueryInterface(__uuidof(m_ns), (void **)&m_ns) ;

	m_hdr.Clear() ;
	m_body.Clear() ;
	m_ind.RemoveAll() ;
	m_typeStack.empty() ;

	// soap 1.1 or soap 1.2 ?
	m_envUri.Empty() ;
	env.get_EnvelopeVersion(&m_envUri) ;
	if ( wcscmp ( m_envUri, SOAP_ENVELOPE_11_URI ) == 0 )
		m_soap12 = false ;
	else if ( wcscmp ( m_envUri, SOAP_ENVELOPE_12_URI ) == 0 )
		m_soap12 = true ;
	else
		return AtlReportError(__uuidof(CoEnvelope), OLESTR("Unsupported envelope format"), IID_NULL, E_UNEXPECTED ) ;

	CComBSTR envPrefix ;
	env.m_namespaces.GetPrefixForURI(m_envUri, &envPrefix) ;

	static const CComBSTR envelope(L"Envelope") ;
	static const CComBSTR encStyle(L"encodingStyle") ;
	static const CComBSTR body(L"Body") ;
	static const CComBSTR header(L"Header") ;

	HRESULT hr ;
	m_lastEncStyle.Empty() ;
	_HR(soap.Allocate( 150 ));
	{
		stringBuff_W req ;
		_HR(req.Allocate ( 150 ));

		CComPtr<ISOAPNodes> nodeCol ;
		hr = env.get_Headers(&nodeCol) ;
		if (FAILED(hr)) return hr ;
		long hc = 0 ;
		nodeCol->get_Count(&hc) ;
		if ( hc )
		{
			m_pcur = &m_hdr ;			
			_HR(StartTag( header, m_envUri ));
			_HR(CloseTag(true));

			// remember all the imediate child's of Header are roots !
			CComPtr<ISOAPNode> hn ;
			for ( long i = 0 ; i < hc ; ++i )
			{
				nodeCol->get_Item(i,&hn) ;
				hn->put_root(VARIANT_TRUE) ;
				hn.Release() ;
			}
			_HR( SerializeCollection ( nodeCol ) ) ;
			for ( i = m_ind.GetSize()-1 ; i >= 0 ; --i )
			{
				_HR(m_hdr.Append ( *m_ind[i] ));
				m_ind[i]->Clear() ;
				m_ind[i]->ReleaseMemory() ;
			}
			m_ind.RemoveAll();
			_HR(ClosingTag(header, m_envUri ));
		}
		nodeCol.Release() ;
		env.get_Body(&nodeCol) ;
		m_pcur = &m_body ;
		_HR ( SerializeCollection ( nodeCol ) ) ;

		#define atribSeperator L"\r\n\t" 

		m_pcur = &soap ;

		_HR(StartTag(envelope, m_envUri ));
		_HR(env.m_namespaces.SerializeNamespaces ( soap , atribSeperator ))
		_HR(CloseTag(true));
		_HR(soap.Append ( m_hdr ));
		m_hdr.Clear() ;
		m_hdr.ReleaseMemory() ;

		_HR(StartTag(body, m_envUri ));
		_HR(CloseTag(true));
		
		VARIANT_BOOL rootFirst = VARIANT_TRUE ;
		CComQIPtr<ISerializerFactoryConfig> sfc(m_sf) ;
		if (sfc)
			sfc->get_RootFirst(&rootFirst) ;

		if ( VARIANT_TRUE == rootFirst )
		{
			_HR(soap.Append ( m_body ));
			m_body.Clear() ;
			m_body.ReleaseMemory() ;

			for ( long i = m_ind.GetSize()-1 ; i >= 0 ; --i )
			{
				_HR(soap.Append ( *m_ind[i] ));
				m_ind[i]->Clear() ;
				m_ind[i]->ReleaseMemory() ;
			}
		}
		else
		{
			for ( long i = m_ind.GetSize()-1 ; i >= 0 ; --i )
			{
				_HR(soap.Append ( *m_ind[i] ));
				m_ind[i]->Clear() ;
				m_ind[i]->ReleaseMemory() ;
			}

			_HR(soap.Append ( m_body ));
			m_body.Clear() ;
			m_body.ReleaseMemory() ;
		}

		_HR(ClosingTag(body, m_envUri));
	}
	
	_HR(ClosingTag(envelope, m_envUri ));

	return S_OK;

}

HRESULT EnvWriter::SerializeCollection ( ISOAPNodes * prms )
{
	long i, iMax ;
	CComPtr<ISOAPNode> n ;
	CComBSTR soap ;

	HRESULT hr = S_OK ;
	prms->get_Count(&iMax) ;
	for ( i = 0 ; i < iMax && SUCCEEDED(hr); ++i )
	{
		prms->get_Item(i, &n) ;	
		hr = SerializeNode ( n ) ;
		n.Release() ;
	}
	return hr ;
}

HRESULT EnvWriter::TidyStream() 
{
	if ( m_closed.size() > 0 )
	{
		if ( ! m_closed[m_closed.size()-1] )
		{
			_HR(m_pcur->Append ( L">", 1 ));
			m_closed[m_closed.size()-1] = true ;
		}
	}
	return S_OK;
}

HRESULT EnvWriter::SerializeNodeHelper ( ISOAPNode * thenode, ISoapSerializer * s )
{
	CComVariant v ;
	thenode->get_Value(&v) ;

	VARIANT_BOOL root ;
	thenode->get_root(&root) ;

	CComPtr<ISOAPNode> valueNode ;
	ISOAPNode * n = thenode ;
	if ( (!m_soap12) && (( v.vt == VT_UNKNOWN || v.vt == VT_DISPATCH ) && v.punkVal ))
	{
		v.punkVal->QueryInterface(__uuidof(valueNode), (void **)&valueNode ) ;
		if ( valueNode )
		{
			n = valueNode.p  ;
		}
	}

	CComBSTR prm, name, nns, t, tns, nsp , enc ;
	CComQIPtr<ISOAPNode2> n2(n) ;
	n2->get_EncodingStyle(&enc) ;
	bool popEncStyle = false ;
	if ( ! bstrEqual(enc , m_lastEncStyle ))
	{
		CComBSTR t = enc ;
		enc = m_lastEncStyle ;
		m_lastEncStyle = t ;
		popEncStyle = true ;
	}

	bool bNeedsId = (root == VARIANT_FALSE) && NeedsId(&v) ;

	if ( m_soap12 || (!bNeedsId))
	{
		n->get_Name(&name) ;
		n->get_Namespace(&nns) ;
		n->get_Type(&t) ;
		n->get_TypeNS(&tns) ;
	}
	else
	{
		// SOAP 1.1 elements with IDs are independent, and should be named by their type
		n->get_Type(&name) ;
		n->get_TypeNS(&nns) ;
		if ( 0 == name.Length() )
		{
			name.Empty() ;
			nns.Empty() ;
			n->get_Name(&name) ;
			n->get_Namespace(&nns) ;
		}
	}

	stringBuff_W * oldcur = m_pcur ;
	if ((!m_soap12) && bNeedsId )
		m_pcur = NewIndependentRoot() ;
	
	_HR(StartTag ( name, nns ));

	static const CComBSTR bstrOne(OLESTR("1")) ;
	static const CComBSTR bstrTrue(OLESTR("true")) ;

	// serialize mustUnderstand header
	VARIANT_BOOL mu ;
	n->get_mustUnderstand(&mu) ;
	if ( VARIANT_TRUE == mu )
	{
		static const CComBSTR mustU(OLESTR("mustUnderstand")) ;
		_HR(Attribute ( mustU, m_envUri, m_soap12 ? bstrTrue : bstrOne ));
	}

	// serilizer actor (soap 1.1) or role (soap 1.2) attribute
	static const CComBSTR actor(OLESTR("actor")) ;
	static const CComBSTR role(OLESTR("role")) ;

	CComBSTR actorURI ;
	n->get_actor(&actorURI) ;
	if ( actorURI.Length() && ( !bNeedsId) )
		_HR(Attribute ( m_soap12 ? role : actor, m_envUri, actorURI ));

	// relay attribute
	if ( m_soap12 )
	{
		VARIANT_BOOL relay ;
		CComQIPtr<ISOAPNode12> n2(n) ;
		n2->get_relay(&relay) ;
		if ( VARIANT_TRUE == relay )
		{
			static CComBSTR bstrRelay(OLESTR("relay")) ;
			_HR(Attribute ( bstrRelay, m_envUri, bstrTrue ));
		}
	}

	// encoding style attribute
	if ( popEncStyle || ((!m_soap12) && bNeedsId ) )
		_HR(Attribute ( encStyleAttrib, m_envUri, m_lastEncStyle ));

	
	// [h]ref attribute
	CComBSTR href ;
	n->get_href(&href) ;
	if ( href.Length() )
		_HR(WriteHrefAttribute ( href, href.Length(), false ));


	stringBuff_W nodeID ;
	if ( bNeedsId )
		 _HR(AddAndRegisterId ( &v, nodeID ));

	if ( v.vt != VT_NULL && 
		 (   v.byref								 || 
		   ( v.vt >= VT_I2      && v.vt <= VT_BSTR ) || 
		   ( v.vt >= VT_DECIMAL && v.vt <= VT_UINT ) || 
		     v.vt == VT_BOOL						 || 
			 v.vt == VT_EMPTY
	     )
	   )
	{
		_HR(s->Serialize ( &v, this, this ));
	}
	else
	{
		static const CComBSTR bstrNil(OLESTR("nil")) ;
		CComBSTR xsi ;
		m_sf->XsiForPrimaryNS(&xsi) ;
		_HR(Attribute ( bstrNil, xsi, bstrTrue ));
	}

	_HR(TidyStream());
	_HR(ClosingTag ( name, nns ));

	if ( popEncStyle )
		m_lastEncStyle = enc ;
	m_pcur = oldcur ;
	if ( (!m_soap12) && bNeedsId )
	{
		CComBSTR name, ns ;
		thenode->get_Name(&name) ;
		thenode->get_Namespace(&ns) ;
		_HR(StartHrefElem ( name, ns, nodeID ));
		if ( actorURI.Length() ) 
			_HR(Attribute ( actor, CComBSTR(SOAP_ENVELOPE_11_URI), actorURI ));
		_HR(FinishHrefElem());
	}

	return S_OK ;
}

HRESULT EnvWriter::AddAndRegisterId ( VARIANT * v, stringBuff_W &theId )
{
	static const CComBSTR root(OLESTR("root")) ;
	static const CComBSTR rootNS(SOAP_ENCODING_11_URI) ;
	static const CComBSTR rootVal(OLESTR("0")) ;

	// SOAP 1.1, by definition, if it has an id, it can't be a root
	if ( ! m_soap12 )
		Attribute ( root, rootNS, rootVal ) ;

	WCHAR buff[15] ;
	item iTemp ;
	iTemp.v = ExtractIdentity(v) ;

	_ultow((unsigned long)iTemp.v, buff, 16) ;
	theId.Clear();
	_HR(theId.Append(buff));
	_HR(m_pcur->Append( L" id=\"", 5 ) );
	_HR(m_pcur->Append( theId ) );
	_HR(m_pcur->Append( L"\"", 1 ) );

	_HR(iTemp.id.Append ( theId ));
	m_done.push_back(iTemp) ;

	ATLTRACE(_T("Adding ID %s = 0x%0x\n"), iTemp.id.c_str(), iTemp.v ) ;
	return S_OK;
}

stringBuff_W * EnvWriter::NewIndependentRoot()
{
	return m_ind.Add() ;
}

STDMETHODIMP EnvWriter::Attribute ( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, /*[in]*/ BSTR value )
{
	return QNameAttribute ( Name, Namespace, value, NULL ) ;
}

HRESULT EnvWriter::SerializerForValue ( VARIANT * value, BSTR Name, BSTR Namespace, BSTR * type, BSTR * typeNS, ISoapSerializer ** s )
{
	CComBSTR parent, parentNS ;
	if ( m_typeStack.size() > 0 )
	{
		parent = m_typeStack.top().localname ;
		parentNS = m_typeStack.top().Namespace ;
	}
	return m_sf->SerializerForValue ( value, parent, parentNS, Name, Namespace, type, typeNS, s ) ;
}

HRESULT EnvWriter::SerializerForNode ( ISOAPNode * n, BSTR * type, BSTR * typeNS, ISoapSerializer ** s ) 
{
	CComBSTR parent, parentNS ;
	if ( m_typeStack.size() > 0 )
	{
		parent = m_typeStack.top().localname ;
		parentNS = m_typeStack.top().Namespace ;
	}
	return m_sf->SerializerForNode ( n, parent, parentNS, type, typeNS, s ) ;
}

STDMETHODIMP EnvWriter::SerializeAttribute( /*[in]*/ VARIANT * value, /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace ) 
{
	CComPtr<ISoapSerializer> ns ;
	CComBSTR type, typeNS ;
	HRESULT hr = SerializerForValue(value, Name, Namespace, &type, &typeNS, &ns) ;
	if (FAILED(hr)) return hr ;

	CComPtr<ISimpleSoapSerializer> ss ;
	hr = ns->QueryInterface(__uuidof(ss), (void **)&ss) ;
	if (FAILED(hr)) return AtlReportError( __uuidof(CoEnvelope), OLESTR("Can only serialize simple types to attribute values"), IID_NULL, E_ATTR_SIMPLE_ONLY ) ;

	CComBSTR atribVal ;
	hr = ss->Serialize ( value, this, &atribVal ) ;
	if (SUCCEEDED(hr))
		hr = QNameAttribute ( Name, Namespace, atribVal, NULL ) ;

	m_sf->ReturnSerializer(ns) ;
	return hr ;
}

STDMETHODIMP EnvWriter::QNameAttribute( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, /*[in]*/ BSTR value, /*[in]*/ BSTR valueNamespace )
{
	_HR(m_pcur->Append ( L" " , 1 ));
	_HR(QName ( Name, Namespace ));
	_HR(m_pcur->Append ( L"=\"", 2 ));

	CComBSTR prefix ;
	m_ns->GetPrefixForURI ( valueNamespace, &prefix ) ;
	if ( prefix.Length() )
	{
		_HR(m_pcur->Append ( prefix, prefix.Length() ));
		_HR(m_pcur->Append ( L":", 1 ));
	}
	_HR(WriteEncodedText(value));
	_HR(m_pcur->Append ( L"\"", 1 ));

	return S_OK ;
}

void EnvWriter::ResolveType( CComBSTR &type, CComBSTR &ns, const CComBSTR &ElemName, const CComBSTR &ElemNS ) 
{
	// try and resolve the type from the serializer Config
	CComBSTR parentType, parentTypeNS ;
	if ( m_typeStack.size() > 0 )
	{
		parentType = m_typeStack.top().localname ;
		parentTypeNS = m_typeStack.top().Namespace ;
	}
	// type & ns are defined as [out] in FindType, so we need to make sure they're clear first
	type.Empty() ;
	ns.Empty() ;
	m_sf->FindType ( parentType, parentTypeNS, ElemName, ElemNS, &type, &ns ) ;
}

void EnvWriter::PushType ( CComBSTR &type, CComBSTR &ns, ISOAPNode * n ) 
{
	if ( type.Length() == 0 )
	{
		CComBSTR elemName, elemNS ;
		n->get_Name(&elemName) ;
		n->get_Namespace(&elemNS) ;
		ResolveType ( type, ns, elemName, elemNS ) ;
	}
	PushType ( type, ns ) ;
}

void EnvWriter::PushType ( CComBSTR &type, CComBSTR &ns, const CComBSTR &ElemName, const CComBSTR &ElemNS ) 
{
	if ( type.Length() == 0 )
		ResolveType  ( type, ns, ElemName, ElemNS ) ;
	PushType ( type, ns ) ;
}

void EnvWriter::PushType ( const CComBSTR &type, const CComBSTR &ns ) 
{
	ATLTRACE(_T("EnvWriter::Pushed Type %ls %ls\n"), ns, type ) ;
	m_typeStack.push ( Qname(type,ns) ) ;
}

void EnvWriter::PopType()
{
	ATLTRACE(_T("PopType()\n") ) ;
	m_typeStack.pop() ;
}

STDMETHODIMP EnvWriter::SerializeNode( /*[in]*/ ISOAPNode * n )
{
	TidyStream() ;
	bool done;
	_HR(CheckIfDone(n, done));
	if(done) 
		return S_OK ;

	m_closed.push_back(false) ;
	CComPtr<ISoapSerializer> ns ;
	CComBSTR type, typeNS ;
	HRESULT hr = SerializerForNode(n, &type, &typeNS, &ns) ;
	if (FAILED(hr)) return hr ;

	PushType ( type, typeNS, n ) ;
	_HR ( SerializeNodeHelper ( n, ns ) ) ;
	PopType() ;

	m_closed.erase(m_closed.begin() + m_closed.size()-1) ;

	// return the serializer now we've done with it
	m_sf->ReturnSerializer(ns) ;
	return S_OK ;
}

STDMETHODIMP EnvWriter::SerializeValue( /*[in]*/ VARIANT * v, /*[in]*/ BSTR Name, /*[in]*/ BSTR NameNS ) 
{
	TidyStream() ;
	bool done;
	_HR( CheckIfDone(v, Name, NameNS, done) )
	if(done)
		return S_OK ;

	m_closed.push_back(false) ;
	CComPtr<ISoapSerializer> s ;
	CComBSTR type, typeNS ;
	HRESULT hr = SerializerForValue ( v, Name, NameNS, &type, &typeNS, &s ) ;
	if (FAILED(hr)) return hr ;

	PushType ( type, typeNS, Name, NameNS ) ;
	if (( v->vt == VT_DISPATCH || v->vt == VT_UNKNOWN ) && v->punkVal )
	{
		CComPtr<ISOAPNode> n ;
		v->punkVal->QueryInterface(__uuidof(n), (void **)&n) ;
		if ( n )
		{
			_HR ( SerializeNodeHelper ( n, s ) ) ;
			m_closed.erase(m_closed.begin() + m_closed.size()-1) ;
			PopType() ;
			return S_OK ;
		}
	}

	stringBuff_W * oldcur = m_pcur ;
	bool needsId = NeedsId(v) ;
	if ( needsId )
		m_pcur = NewIndependentRoot() ;

	_HR(StartTag ( Name, NameNS ));

	stringBuff_W nodeID ;
	if ( needsId )
	{
		_HR(AddAndRegisterId (v, nodeID));
		_HR(Attribute ( encStyleAttrib, m_envUri, m_lastEncStyle ));
	}

	if ( ! ( v->byref || ( v->vt >= VT_I2 && v->vt <= VT_BSTR ) || ( v->vt >= VT_DECIMAL && v->vt <= VT_UINT ) || v->vt == VT_BOOL || v->vt == VT_EMPTY ))
	{
		static const CComBSTR nilTrue(OLESTR("true")) ;
		CComBSTR xsi ;
		m_sf->XsiForPrimaryNS(&xsi) ;
		_HR(Attribute ( CComBSTR(OLESTR("nil")), xsi, nilTrue ));
	}
	else
		_HR(s->Serialize ( v, this, this ));

	_HR(TidyStream());
	_HR(ClosingTag ( Name, NameNS ));
	PopType() ;

	m_closed.erase(m_closed.begin() + m_closed.size()-1) ;
	m_pcur = oldcur ;

	if ( needsId )
		_HR(WriteHrefElem ( Name, NameNS, nodeID ))

	// return the serializer now we've done with it
	m_sf->ReturnSerializer(s) ;
	return S_OK ;
}

// this gets escaped automatically
STDMETHODIMP EnvWriter::WriteText( /*[in]*/ BSTR text ) 
{
	_HR(TidyStream());
	return WriteEncodedText ( text ) ;
}

HRESULT EnvWriter::WriteEncodedText ( /*[in]*/ BSTR text ) 
{
	size_t len = SysStringLen ( text ) ;
	_HR(m_pcur->Allocate( m_pcur->Size() + len ));
	OLECHAR * c = text ;
	while ( len-- )
	{
		if ( *c == '&' )
			_HR(m_pcur->Append( OLESTR("&amp;"), 5 ))
		else if ( *c == '<' )
			_HR(m_pcur->Append( OLESTR("&lt;"), 4 ))
		else if ( *c == '>' )
			_HR(m_pcur->Append ( OLESTR("&gt;"), 4 ))
		else if ( *c == '\"' ) 
			_HR(m_pcur->Append ( OLESTR("&quot;"), 6 ))
		else
			_HR(m_pcur->Append( c , 1 ))

		++c ;
	}
	return S_OK ;
}

STDMETHODIMP EnvWriter::WriteTextNoEncoding( /*[in]*/ BSTR text ) 
{
	_HR(TidyStream());
	return m_pcur->Append ( text, SysStringLen(text) );
}

STDMETHODIMP EnvWriter::StartElement( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace )
{
	_HR(TidyStream());
	_HR(StartTag ( Name, Namespace ));
	m_closed.push_back(false) ;
	return S_OK ;
}

STDMETHODIMP EnvWriter::EndElement  ( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace )
{
	_HR(TidyStream());
	m_closed.erase(m_closed.begin() + m_closed.size()-1) ;
	return ClosingTag( Name, Namespace ) ;
}
