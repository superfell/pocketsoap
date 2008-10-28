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

/////////////////////////////////////////////////////////////////////////////
// HTTPTransport.cpp : Implementation of CHTTPTransport
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "psoap.h"
#include "HTTPTransport.h"
#include "tags.h"
#include "stringBuff.h"
#include "stringHelpers.h"

const wchar_t * const	USER_AGENT_STRING = L"PocketSOAP/1.5.4/" ;	// todo : get version from VersionInfo

static const CComBSTR HTTP_CONTENT_TYPE		(OLESTR("Content-Type")) ;
static const CComBSTR HTTP_SOAP_11_ACTION	(OLESTR("SOAPAction")) ;
static const CComBSTR TEXT_XML				(OLESTR("text/xml; charset=UTF-8")) ;

static const CComBSTR PH_MAX_REDIRECTS		(OLESTR("redirects.max")) ;

/////////////////////////////////////////////////////////////////////////////
// CStreamReader
//
// This exposes a PocketHTTP::IReadableStream as a IStreamReader
/////////////////////////////////////////////////////////////////////////////
class CStreamReader :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IStreamReader
{
public:

BEGIN_COM_MAP(CStreamReader)
	COM_INTERFACE_ENTRY(IStreamReader)
END_COM_MAP()

	void Init ( PocketHTTP::IReadableStream * s )
	{
		m_s = s ;
	}

	STDMETHODIMP Read(void* pv, DWORD cb, DWORD * pcbRead)
	{
		return m_s->Read(pv, cb, pcbRead) ;
	}

	STDMETHODIMP Reset()
	{
		return E_NOTIMPL ;
	}

private:
	CComPtr<PocketHTTP::IReadableStream> m_s ;
};

/////////////////////////////////////////////////////////////////////////////
// CResetableStream
//
// This exposes a IStreamReader as a PocketHTTP::IResetableStream 
/////////////////////////////////////////////////////////////////////////////
class CResetableStream :
	public CComObjectRootEx<CComMultiThreadModel>,
	public PocketHTTP::IResetableStream
{
public:

BEGIN_COM_MAP(CResetableStream)
	COM_INTERFACE_ENTRY_IID(__uuidof(PocketHTTP::IResetableStream), PocketHTTP::IResetableStream)
END_COM_MAP()

	void Init ( IStreamReader * s )
	{
		m_s = s ;
	}

	STDMETHODIMP Read(void* pv, DWORD cb, DWORD * pcbRead)
	{
		return m_s->Read(pv, cb, pcbRead) ;
	}

	STDMETHODIMP Reset()
	{
		return m_s->Reset();
	}

private:
	CComPtr<IStreamReader> m_s ;
};

/////////////////////////////////////////////////////////////////////////////
// CHTTPTransport
/////////////////////////////////////////////////////////////////////////////
CHTTPTransport::CHTTPTransport() 
{
}

CHTTPTransport::~CHTTPTransport()
{
}

HRESULT CHTTPTransport::FinalConstruct()
{
	m_actionSet = false ;

	// create the PocketHTTP instance we're wrapping
	_HR ( m_req.CoCreateInstance(__uuidof(PocketHTTP::CoPocketHTTP))) ;

	// set the (default) content-type
	CComPtr<PocketHTTP::IHeadersCollection> headers ;
	m_req->get_Headers(&headers) ;
	headers->Create ( HTTP_CONTENT_TYPE, TEXT_XML, &m_type ) ;

	// set the user agent
	static const CComBSTR userAgent( OLESTR("User-Agent")) ;
	CComPtr<PocketHTTP::IHeader> ua ;
	headers->Find(userAgent, &ua) ;
	CComBSTR psUA ( USER_AGENT_STRING ) ;
	CComBSTR phUA ;
	ua->get_Value(&phUA) ;
	psUA.AppendBSTR(phUA) ;
	ua->put_Value(psUA) ;

	// set the method to POST
	static const CComBSTR post(OLESTR("POST")) ;
	m_req->put_Method(post) ;
	return S_OK ;
}


STDMETHODIMP CHTTPTransport::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ISOAPTransport, &IID_IHTTPTransport, &IID_IHTTPTransportAdv, &IID_ISwATransport, &IID_ISOAPTransportTimeout, &IID_IStreamReader
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::ATL::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CHTTPTransport::get_Option( BSTR optionName, VARIANT * pVal )
{
	return m_req->get_Option(optionName, pVal) ;
}

STDMETHODIMP CHTTPTransport::put_Option( BSTR optionName, VARIANT val )
{
	return m_req->put_Option(optionName, val ) ;
}

STDMETHODIMP CHTTPTransport::Send(BSTR endpoint, BSTR Envelope)
{
	// note, that we don't copy the string, so we shouldn't
	// call VariantClear to have it tidied away like we normally would
	VARIANT vEnv ;
	VariantInit(&vEnv) ;
	vEnv.vt = VT_BSTR ;
	vEnv.bstrVal = Envelope ;
	
	return Send ( endpoint, vEnv ) ;
}

STDMETHODIMP CHTTPTransport::Send( BSTR endpoint, VARIANT Payload )
{
	CComVariant ourPayload ;
	VARIANT * thePayload = &Payload ;
	bool needSoapAction = true ;
	bool needAcceptCS   = false ;

	// if the source is a IStreamReader, we need to wrap that into a IPocketHTTP::IResetableStream
	if ( Payload.vt == ( VT_VARIANT | VT_BYREF ) )
		Payload = *Payload.pvarVal ;

	VARTYPE vt = Payload.vt & ~VT_BYREF ;
	// if payload is a string, say its an SOAP 1.1 message, this makes us compatible when people haven't updated their
	// calls to Send for 1.5 useage yet.
	if ( vt == VT_BSTR )
	{
		put_ContentType(TEXT_XML) ;
		needAcceptCS = true ;
	}
	else if (( vt == VT_UNKNOWN ) || ( vt == VT_DISPATCH ))
	{
		IUnknown * punkVal ;
		if ( Payload.vt & VT_BYREF )
			punkVal = *(IUnknown **)Payload.byref ;
		else
			punkVal = Payload.punkVal ;

		// see if the source is a SOAP 1.1 or SOAP 1.2 envelope, and set the content type as needed
		CComQIPtr<ISOAPEnvelope2> e(punkVal) ;
		if (e)
		{
			CComBSTR envURI ;
			e->get_EnvelopeVersion(&envURI) ;
			if ( wcscmp ( envURI, SOAP_ENVELOPE_12_URI ) == 0 )
			{
				stringBuff_W ct ;
				ct << L"application/soap+xml; charset=\"utf-8\"" ;
				if ( m_actionSet )
					ct << L"; action=\"" << ( m_action.Length() > 0 ? m_action : L"" ) << L"\"" ;
				put_ContentType(CComBSTR(ct.c_str())) ;
				ClearSoapAction() ;
				needSoapAction = false ;
				needAcceptCS   = true ;
			}
			else if ( wcscmp ( envURI, SOAP_ENVELOPE_11_URI ) == 0 )
			{
				put_ContentType(TEXT_XML) ;
				needAcceptCS = true ;
			}
		}

		CComPtr<IStreamReader> sr ;
		CComPtr<PocketHTTP::IResetableStream> rs ;
		if(FAILED(punkVal->QueryInterface(__uuidof(rs), (void **)&rs)))
		{
			if(SUCCEEDED(punkVal->QueryInterface(__uuidof(sr), (void **)&sr)))
			{
				CComObject<CResetableStream> * crs ;
				_HR(crs->CreateInstance(&crs)) ;
				crs->AddRef() ;
				crs->Init(sr) ;
				//Payload.punkVal->Release() ;
				// crs->QueryInterface(&Payload.punkVal) ;
				crs->QueryInterface(&ourPayload.punkVal) ;
				ourPayload.vt = VT_UNKNOWN ;
				thePayload = &ourPayload ;
				crs->Release() ;
			}
		}
	}

	if (needSoapAction)
		SetSoapAction() ;

	// only send an Accept-Charset, if we're sending text/* or application/soap+xml
	static const CComBSTR Accept   ( OLESTR("Accept-Charset")) ;
	CComPtr<PocketHTTP::IHeadersCollection> headers ;
	CComPtr<PocketHTTP::IHeader> acceptCS ;
	m_req->get_Headers(&headers) ;
	headers->Find(Accept, &acceptCS) ;

	CComBSTR reqCT ;
	m_type->get_Value(&reqCT) ;
	if ( needAcceptCS  || ( wcsnicmp ( reqCT, L"text/", 5 ) == 0 ) )
	{	
		static const CComBSTR charSets (OLESTR("UTF-8, UTF-16;q=0.8, iso-8859-1;q=0.8")) ;	
		if(acceptCS)
			acceptCS->put_Value(charSets) ;
		else
			headers->Create(Accept, charSets, &acceptCS ) ;
	}
	else if (acceptCS)
	{
		CComQIPtr<PocketHTTP::IHeader2> h2(acceptCS) ;
		h2->Delete() ;
	}

	// and off we go !
	m_res.Release() ;
	CComPtr<PocketHTTP::IHttpRequestSplit> h ;
	_HR ( m_req.QueryInterface(&h) ) ;
	HRESULT hr = h->Send ( endpoint, *thePayload ) ;
	return hr ;
}


HRESULT CHTTPTransport::SetSoapAction()
{
	// find the header
	CComPtr<PocketHTTP::IHeadersCollection> headers ;
	m_req->get_Headers(&headers) ;
	CComPtr<PocketHTTP::IHeader> sa ;
	headers->Find(HTTP_SOAP_11_ACTION, &sa ) ;

	// quote the value
	CComBSTR quoted ;
	DWORD ccSA = m_action.Length() ;
	quoted.Attach ( SysAllocStringLen(NULL, ccSA + 2 ) ) ;
	quoted.m_str[0] = '\"' ;
	memcpy ( &quoted.m_str[1], m_action.m_str, sizeof(OLECHAR) * ccSA ) ;
	quoted.m_str[ccSA+1] = '\"' ;

	// set the value
	if ( sa )
		return sa->put_Value(quoted) ;
	return headers->Create ( HTTP_SOAP_11_ACTION, quoted, &sa ) ;
}

HRESULT CHTTPTransport::ClearSoapAction() 
{
	CComPtr<PocketHTTP::IHeadersCollection> headers ;
	m_req->get_Headers(&headers) ;
	CComPtr<PocketHTTP::IHeader> sa ;
	headers->Find(HTTP_SOAP_11_ACTION, &sa ) ;
	if ( sa )
	{
		CComQIPtr<PocketHTTP::IHeader2> h(sa) ;
		h->Delete() ;
	}
	return S_OK ;
}


// start receiving the response, the caller pulls the response body from us via the responseStream
STDMETHODIMP CHTTPTransport::Receive(/*[in,out]*/ BSTR* characterEncoding, /*[out,retval]*/ IUnknown ** responseStream ) 
{
	HRESULT hr = S_OK ;
	if ( ! characterEncoding )  hr = E_POINTER; 
	if ( ! responseStream )		hr = E_POINTER; else *responseStream = 0 ;
	if (FAILED(hr)) return hr ;

	CComPtr<PocketHTTP::IHttpRequestSplit> h ;
	_HR ( m_req.QueryInterface(&h) ) ;
	_HR ( h->Response ( &m_res ) ) ;

	short statusCode =0 ;
	_HR ( m_res->get_StatusCode(&statusCode)) ;

	if ( ((statusCode < 200 ) || ( statusCode > 299 )) && (500 != statusCode) )
	{
		m_res.Release() ;
		std::wstring err(L"Unexpected HTTP Status code of ") ;
		wchar_t sc[10] ;
		_itow(statusCode, sc, 10 ) ;
		err.append(sc) ;
		err.append(L" (expecting 2xx, 30x, 401, 407 or 500)") ;
		return AtlReportError ( GetObjectCLSID(), err.c_str(), IID_NULL, PocketHTTP::E_STATUS_ERROR ) ;
	}

	SysFreeString(*characterEncoding) ;
	*characterEncoding = 0 ;

	CComPtr<PocketHTTP::IHeadersCollection> headers ;
	m_res->get_Headers(&headers) ;
	CComPtr<PocketHTTP::IHeader> ct ;
	hr = headers->Find(HTTP_CONTENT_TYPE, &ct );
	if (SUCCEEDED(hr)) {
		static const CComBSTR charset (OLESTR("charset")) ;
		hr = ct->get_Attribute ( charset, characterEncoding ) ;
	}
	// set to default, us-ascii, if not supplied
	if (FAILED(hr))
		*characterEncoding = SysAllocString(OLESTR("us-ascii")) ;

	m_resStream.Release() ;
	_HR (m_res->get_Stream(&m_resStream) ) ;

	CComObject<CStreamReader> * cs ;
	_HR ( cs->CreateInstance(&cs)) ;
	cs->AddRef() ;
	cs->Init(m_resStream) ;
	hr = cs->QueryInterface(responseStream) ;
	cs->Release() ;
	return hr ;
}

// receive the response, we package the response body up into a byte array.
STDMETHODIMP CHTTPTransport::Receive(/*[in,out]*/ BSTR * characterEncoding, /*[out,retval]*/ SAFEARRAY ** Envelope)
{
	if ( ! Envelope ) return E_POINTER ;
	*Envelope = 0 ;

	CComPtr<IUnknown> punkStm ;
	_HR ( Receive ( characterEncoding, &punkStm ) ) ;
	
	stringBuff_A results ;
	static const DWORD buff_size = 8192 ;
	char buff[buff_size] ;
	DWORD cbRead ;
	do
	{
		_HR ( m_resStream->Read ( buff, buff_size, &cbRead ) ) ;
		results.Append ( buff, cbRead ) ;
	} while ( cbRead > 0 ) ;

	size_t payloadSize = results.Size() ;
	SAFEARRAYBOUND rga ;
	rga.lLbound = 0 ;
	rga.cElements = payloadSize ;
	*Envelope = SafeArrayCreate ( VT_UI1, 1, &rga ) ;
	if ( ! *Envelope )
		return E_OUTOFMEMORY ;

	char * dest = 0 ;
	HRESULT hr = SafeArrayAccessData ( *Envelope, (void **)&dest ) ;
	if(SUCCEEDED(hr))
	{
		memcpy ( dest, results.c_str(), payloadSize ) ;
		SafeArrayUnaccessData ( *Envelope ) ;
	}
	else
	{
		SafeArrayDestroy ( *Envelope ) ;
		*Envelope = 0 ;
	}
	return hr ;
}


STDMETHODIMP CHTTPTransport::Read( void* pvx, DWORD cb, DWORD * pcbRead )
{
	return m_resStream->Read(pvx, cb, pcbRead ) ;
}

// todo: re-factor interfaces, so that there's a pure Read interface
STDMETHODIMP CHTTPTransport::Reset()
{
	return E_NOTIMPL ;
}

STDMETHODIMP CHTTPTransport::get_StatusCode( short * statusCode ) 
{
	return !m_res ? 0 : m_res->get_StatusCode(statusCode);
}

STDMETHODIMP CHTTPTransport::put_SOAPAction(BSTR soapAction)
{
	m_action = soapAction ;
	m_actionSet = true ;
	return S_OK ;
}

STDMETHODIMP CHTTPTransport::get_SOAPAction(BSTR * soapAction) 
{
	return m_action.CopyTo(soapAction) ;
}


STDMETHODIMP CHTTPTransport::put_Timeout( long timeOut )
{
	return m_req->put_Timeout(timeOut) ;
}

STDMETHODIMP CHTTPTransport::get_Timeout( long * timeOut )
{
	return m_req->get_Timeout(timeOut) ;
}

STDMETHODIMP CHTTPTransport::SetProxy( BSTR ProxyServer, short ProxyPort ) 
{
	return m_req->SetProxy(ProxyServer, ProxyPort) ;
}

STDMETHODIMP CHTTPTransport::NoProxy() 
{
	return m_req->NoProxy() ;
}

STDMETHODIMP CHTTPTransport::Authentication( BSTR username, BSTR password )
{
	return m_req->Authentication(username, password) ;
}

STDMETHODIMP CHTTPTransport::ProxyAuthentication( BSTR username, BSTR password )
{
	return m_req->ProxyAuthentication(username, password) ;
}

STDMETHODIMP CHTTPTransport::get_MaxRedirectDepth( short * depth )
{
	if ( ! depth ) return E_POINTER ;
	CComVariant v ;
	_HR ( m_req->get_Option ( PH_MAX_REDIRECTS, &v ) ) ;
	*depth = v.iVal ;
	return S_OK ;
}

STDMETHODIMP CHTTPTransport::put_MaxRedirectDepth( short depth )
{
	CComVariant v(depth) ;
	return m_req->put_Option ( PH_MAX_REDIRECTS, v ) ;
}

STDMETHODIMP CHTTPTransport::get_ContentType( BSTR * ctVal ) 
{
	CComPtr<PocketHTTP::IHeadersCollection> headers ;
	m_res->get_Headers(&headers) ;
	CComPtr<PocketHTTP::IHeader> ct ;
	_HR ( headers->Find ( HTTP_CONTENT_TYPE, &ct ) ) ;
	return ct->get_Value(ctVal) ;
}

STDMETHODIMP CHTTPTransport::put_ContentType( BSTR ct ) 
{
	return m_type->put_Value(ct) ;
}
