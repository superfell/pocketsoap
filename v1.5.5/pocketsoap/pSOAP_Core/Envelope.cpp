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
Portions created by Simon Fell are Copyright (C) 2000,2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

/////////////////////////////////////////////////////////////////////////////
// Envelope.cpp : Implementation of CEnvelope
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "psoap.h"
#include "Envelope.h"

#include "SOAPNode.h"
#include "SOAPNodes.h"
#include "tags.h"
#include "EnvWriter.h"
#include "stringHelpers.h"

/////////////////////////////////////////////////////////////////////////////
// CEnvelopeClassFactory
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CEnvelopeClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj)
{
	HRESULT hr = CComClassFactory::CreateInstance(pUnkOuter, riid, ppvObj) ;
	if (SUCCEEDED(hr))
	{
		CComPtr<ISOAPEnvelope2> e ;
		((IUnknown *)*ppvObj)->QueryInterface(__uuidof(e), (void **)&e) ;
		e->put_EnvelopeVersion(m_uri) ;
	}
	return hr ;
}

STDMETHODIMP CEnvelopeClassFactory::setVersion(LPCWSTR envURI) 
{
	m_uri = envURI ;
	return S_OK ;
}


/////////////////////////////////////////////////////////////////////////////
// CEnvelope
/////////////////////////////////////////////////////////////////////////////

HRESULT CEnvelope::FinalConstruct()
{
	m_soap12 = false ;					// default to SOAP 1.2
	m_throwFaults = true ;				// by default, we'll convert a SOAP Fault to a COM error
	m_encStyle = SOAP_ENCODING_11_URI;	// default encoding style
	m_encStyleDefaulted = true ;		// current encoding style is the default
	m_URI = L"" ;						// by default, we're namespaceless
	m_methodNameDirty = false ;			// we need to keep track of if we need to update the root body part or not
	m_stream_pos = 0 ;

	CComObject<CSOAPNode> * p ;
	HRESULT hr = CComObject<CSOAPNode>::CreateInstance(&p) ;
	if (FAILED(hr)) return hr ;

	p->AddRef() ;
	p->QueryInterface(&m_bodyNode) ;
	p->get_Nodes(&m_bodyParts) ;
	p->Release() ;

	hr = CComObject<CSOAPNode>::CreateInstance(&p) ;
	if (FAILED(hr)) return hr ;

	p->AddRef() ;
	p->QueryInterface(&m_headersNode) ;
	p->Release() ;

	
	return hr ;
}

STDMETHODIMP CEnvelope::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ISOAPEnvelope
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::ATL::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CEnvelope::SetMethod( /*[in]*/ BSTR methodName, /*[in]*/ BSTR methodNameNamespaceURI )
{
	put_URI(methodNameNamespaceURI) ;
	put_MethodName(methodName) ;
	return S_OK ;
}

STDMETHODIMP CEnvelope::get_URI(BSTR *pVal)
{
	return m_URI.CopyTo(pVal) ;	
}

STDMETHODIMP CEnvelope::put_URI(BSTR newVal)
{
	m_URI = newVal ;
	m_methodNameDirty = true ;
	return S_OK;
}

STDMETHODIMP CEnvelope::get_MethodName(BSTR *pVal)
{
	return m_methodName.CopyTo(pVal) ;
}

STDMETHODIMP CEnvelope::put_MethodName(BSTR newVal)
{
	m_methodName = newVal ;
	m_methodNameDirty = true ;
	return S_OK;
}

STDMETHODIMP CEnvelope::get_EncodingStyle(BSTR *pVal)
{
	return m_encStyle.CopyTo(pVal) ; 
}

STDMETHODIMP CEnvelope::put_EncodingStyle(BSTR newVal)
{
	m_encStyleDefaulted = false ;
	updateEncodingStyle(newVal) ;
	return S_OK;
}

void CEnvelope::updateEncodingStyle( LPCWSTR newEncodingStyle )
{
	m_encStyle = newEncodingStyle ;
	m_bodyNode->put_EncodingStyle(m_encStyle) ;
	m_headersNode->put_EncodingStyle(m_encStyle) ;
}

STDMETHODIMP CEnvelope::Serialize(BSTR *soapMsg)
{
	if ( m_methodNameDirty )
	{
		// we need to update the root element, in case the methodName or URI has changed
		// since get_Parameters was first called.
		CComPtr<ISOAPNodes> n ;
		get_Parameters(&n) ;
		m_bodyRoot->put_Name(m_methodName) ;
		m_bodyRoot->put_Namespace(m_URI) ;
	}

	stringBuff_W env ;
	CComObject<EnvWriter> * w ;
	w->CreateInstance(&w) ;
	w->AddRef() ;
	HRESULT hr = w->Serialize( *this, env );
	w->Release() ;
	if (FAILED(hr)) 
		return hr ;

	*soapMsg = SysAllocStringLen ( env.c_str(), env.Size() ) ;
	return S_OK ;
}

STDMETHODIMP CEnvelope::get_SerializerFactory( /*[out,retval]*/ ISerializerFactoryConfig ** sf )
{
	if ( ! m_sf )
	{
		HRESULT hr= m_sf.CoCreateInstance(__uuidof(CoSerializerFactory)) ;
		if (FAILED(hr)) return hr ;
	}
	return m_sf->QueryInterface(__uuidof(*sf), (void**)sf) ;
}

HRESULT CEnvelope::get_SerializerFactory(ISerializerFactory ** sf)
{
	if ( ! m_sf )
	{
		HRESULT hr= m_sf.CoCreateInstance(__uuidof(CoSerializerFactory)) ;
		if (FAILED(hr)) return hr ;
		CComQIPtr<ISerializerFactoryConfig> cfg(m_sf) ;
		if ( m_soap12 )
			cfg->SetConfig(CComVariant(sfcNormal_12)) ;
		else
			cfg->SetConfig(CComVariant(sfcNormal_11)) ;

	}
	return m_sf.CopyTo(sf) ;
}

STDMETHODIMP CEnvelope::putref_SerializerFactory( /*[in]*/ ISerializerFactoryConfig * sf ) 
{
	m_sf.Release() ;
	return sf->QueryInterface(__uuidof(m_sf), (void **)&m_sf) ;
}

STDMETHODIMP CEnvelope::Parse(/*[in]*/ VARIANT Envelope, /*[in,defaultvalue(L"")]*/ BSTR characterEncoding )
{
	CComPtr<ISOAPNodes> h ;
	m_headersNode->get_Nodes(&h) ;
	h->Clear() ;
	m_bodyParts->Clear() ;
	m_bodyRoot.Release() ;

	psParser parser(*this) ;
	HRESULT hr = parser.parse(Envelope, characterEncoding) ;

	// we need to pull out the methodName & URI params back into our class members
	m_methodName.Empty() ;
	m_URI.Empty() ;
	CComPtr<ISOAPNodes> nodes ;
	// this takes care of the root issues for us
	get_Parameters(&nodes) ;	
	// however nodes now points to the children, not the actual node, but it has setup m_bodyRoot
	// so now we pull it out of there.
	m_bodyRoot->get_Name(&m_methodName) ;
	m_bodyRoot->get_Namespace(&m_URI) ;
	// we also need to tell this node that it is infact a serialization root
	m_bodyRoot->put_root(VARIANT_TRUE) ;

	if ( SUCCEEDED(hr) && m_throwFaults )
		return CheckForFault (parser) ;

	return hr ;
}

STDMETHODIMP CEnvelope::get_Headers( /*[out,retval]*/ ISOAPNodes ** Nodes )
{
	return m_headersNode->get_Nodes(Nodes) ;
}

STDMETHODIMP CEnvelope::get_Body( ISOAPNodes ** Nodes ) 
{
	return m_bodyParts.CopyTo(Nodes) ;
}

STDMETHODIMP CEnvelope::get_Parameters(ISOAPNodes ** ppParams)
{
	if ( m_bodyRoot )
		return m_bodyRoot->get_Nodes(ppParams) ;

	CComPtr<ISOAPNode> n ;
	long c ;
	m_bodyParts->get_Count(&c) ;
	if ( c == 0 )
	{
		// no parts yet, create the methodName part
		m_bodyParts->Create( m_methodName, CComVariant(), m_URI, CComBSTR(), CComBSTR(), &n ) ;
		n->put_root(VARIANT_TRUE) ;
	}
	else if ( c == 1 )
	{
		// one part, must be the methodName part, use that
		m_bodyParts->get_Item(0, &n) ;
	}
	else
	{
		// more than one part, multi-ref serialized body
		// look for the first child with root=1
		// or the first child with no explicit root attribute
		VARIANT_BOOL root , exproot ;
		for ( long i = 0 ; i < c ; ++i )
		{
			m_bodyParts->get_Item(i, &n) ;
			CComQIPtr<ISOAPNode3> n3(n) ;
			n3->get_explicitRoot(&exproot) ;
			if ( VARIANT_FALSE == exproot )
				break ;
			n->get_root(&root) ;
			if ( VARIANT_TRUE == root )
				break ;
			n.Release() ;
		}
		if (! n )
			m_bodyParts->get_Item(0, &n) ;
	}
	m_bodyRoot = n ;
	return n->get_Nodes(ppParams) ;
}

STDMETHODIMP CEnvelope::get_ThrowFaults(/*[out, retval]*/ VARIANT_BOOL *pVal)
{
	if ( ! pVal ) return E_POINTER ;
	*pVal = m_throwFaults ? VARIANT_TRUE : VARIANT_FALSE ;
	return S_OK ;
}

STDMETHODIMP CEnvelope::put_ThrowFaults(/*[in]*/ VARIANT_BOOL newVal)
{
	m_throwFaults = newVal == VARIANT_TRUE ;
	return S_OK ;
}

HRESULT CEnvelope::ExtractNodeVal ( CComPtr<ISOAPNodes> &p, LPCOLESTR itemName, CComVariant &val ) 
{
	CComPtr<ISOAPNode> item ;
	HRESULT hr = p->get_ItemByName(CComBSTR(itemName), CComBSTR(L""), &item ) ;
	if (FAILED(hr)) return hr;
	val.Clear() ;
	hr = item->get_Value(&val);
	return hr;
}

void MakeFaultString ( CComPtr<IXmlQName> &code, CComBSTR &str, stringBuff_W &res ) 
{
	CComBSTR cns, cname ;
	code->get_Name(&cname) ;
	code->get_Namespace(&cns) ;
	if ( wcscmp ( cns, SOAP_ENVELOPE_11_URI ) != 0 )
	{
		res.Append ( L"{", 1 ) ;
		res.Append ( cns , cns.Length() ) ;
		res.Append ( L"}", 1 ) ;
	}
	res.Append ( cname, cname.Length() ) ;
	res.Append ( L" : " , 3 ) ;
	res.Append ( str, str.Length() ) ;
}

HRESULT CEnvelope::CheckForFault( const psParser& parser ) 
{
	CComPtr<ISOAPNodes> p ;
	get_Parameters(&p) ;
	CComBSTR mn, mns ;

	// name matches ?
	m_bodyRoot->get_Name(&mn) ;
	if ( wcscmp ( mn, SOAP_FAULT_NAME ) != 0 )
		return S_OK ;

	// namespace matches ?
	m_bodyRoot->get_Namespace(&mns) ;
	if ( wcscmp ( mns, SOAP_ENVELOPE_11_URI ) != 0 )
		return S_OK ;

	CComBSTR fltString ;
	CComPtr<IXmlQName> fltcode ;
	CComVariant nv ;
	HRESULT hr = ExtractNodeVal ( p, OLESTR("faultcode"), nv ) ;
	if (FAILED(hr))
		return AtlReportError ( GetObjectCLSID(), OLESTR("SOAP Fault returned, but no faultcode element found"), __uuidof(ISOAPEnvelope), E_NOFAULTCODE ) ;
	if ( nv.vt != VT_UNKNOWN )
		return AtlReportError ( GetObjectCLSID(), OLESTR("SOAP Fault returned, but faultcode is of unexpected type (expecting xsd:QName)"), __uuidof(ISOAPEnvelope), E_NOFAULTCODE ) ;
	nv.punkVal->QueryInterface(__uuidof(fltcode), (void **)&fltcode ) ;

	hr = ExtractNodeVal ( p, OLESTR("faultstring"), nv ) ;
	if (FAILED(hr))
		return AtlReportError ( GetObjectCLSID(), OLESTR("SOAP Fault returned, but no faultstring element found"), __uuidof(ISOAPEnvelope), E_NOFAULTSTRING ) ;
	nv.ChangeType(VT_BSTR) ;
	fltString.Attach(nv.bstrVal) ;
	nv.vt = VT_EMPTY ;

	stringBuff_W comFaultString ;
	MakeFaultString ( fltcode, fltString, comFaultString ) ;

	return AtlReportError ( GetObjectCLSID(), comFaultString.c_str(), __uuidof(ISOAPEnvelope), E_SOAPFAULT ) ;
}

STDMETHODIMP CEnvelope::GetPrefixForURI( /*[in]*/ BSTR NamespaceURI, /*[out,retval]*/ BSTR * prefix )
{
	return m_namespaces.GetPrefixForURI(NamespaceURI, prefix ) ;
}

STDMETHODIMP CEnvelope::GetURIForPrefix( /*[in]*/ BSTR prefix, /*[out,retval]*/ BSTR * NamespaceURI )
{
	return m_namespaces.GetURIForPrefix( prefix, NamespaceURI ) ;
}


STDMETHODIMP CEnvelope::get_EnvelopeVersion(BSTR * envURI)
{
	if ( ! envURI ) return E_POINTER ;
	*envURI = SysAllocString ( m_soap12 ? SOAP_ENVELOPE_12_URI : SOAP_ENVELOPE_11_URI ) ;
	return S_OK ;
}

STDMETHODIMP CEnvelope::put_EnvelopeVersion(BSTR envURI)
{
	if ( wcscmp ( envURI, SOAP_ENVELOPE_12_URI ) == 0 )
		m_soap12 = true ;
	
	else if ( wcscmp ( envURI, SOAP_ENVELOPE_11_URI ) == 0 )
		m_soap12 = false ;
	
	else
		return E_INVALIDARG;

	if ( m_encStyleDefaulted )
		updateEncodingStyle ( m_soap12 ? SOAP_ENCODING_12_URI : SOAP_ENCODING_11_URI ) ;

	return S_OK ;
}

void CEnvelope::setEnvelopeVersion(bool isSoap12) 
{
	m_soap12 = isSoap12 ;
}

STDMETHODIMP CEnvelope::Read ( void *pv, ULONG cb, ULONG * pcbRead )
{
	// this is a pretty dump implementation, just to get things moving
	if ( m_stream.Size() == 0 )
	{
		CComBSTR env ;
		_HR ( Serialize (&env) ) ;
		Ole2Utf8 ( env, m_stream ) ;
	}
	*pcbRead = m_stream.Size() - m_stream_pos ;
	if ( m_stream_pos > m_stream.Size() )
		*pcbRead = 0 ;
	if ( *pcbRead > cb ) 
		*pcbRead = cb ;
	memcpy ( pv, &m_stream.buffer()[m_stream_pos], *pcbRead ) ;
	m_stream_pos += *pcbRead ;
	// once we've read to the end of the stream, release the memory 
	if (( *pcbRead == 0 ) && ( cb > 0 ))
	{
		m_stream.Clear() ;
		m_stream.ReleaseMemory() ;
	}
	return S_OK ;
}

STDMETHODIMP CEnvelope::Reset()
{
	m_stream.Clear() ;
	m_stream.ReleaseMemory() ;
	m_stream_pos = 0 ;
	return S_OK ;
}