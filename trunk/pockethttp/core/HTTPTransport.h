/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketHTTP.

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2000-2006
Simon Fell. All Rights Reserved.

Contributor(s):
*/

/////////////////////////////////////////////////////////////////////////////
// HTTPTransport.h : Declaration of the CHTTPTransport
//
// A HTTP transport object, this uses winsock directly so there are
// no problems using it in a server 2 server environment
//
// This supports
//		HTTP 1.1
//		keep-alives
//		chunked encoding [receive only]
//		user settable timeouts
//		basic authentication support (tested with IIS5.0)
//		proxy server support	(tested with Squid, winproxy & proxyTrace)
//		proxy server authentication support (tested with Squid)
//		SSL 
//		redirects
//		session cookies
//		SSL over proxy
//		gzip & deflate compression
//		diagnostics tracing
// todo
//		async interface
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __HTTPTRANSPORT_H_
#define __HTTPTRANSPORT_H_

#include "resource.h"       // main symbols
#include "stringBuff.h"
#include "transportBase.h"
#include "chunkedTE.h"
#include "headerHelpers.h"
#include "TracingStream.h"
#include "cookie.h"

class Connection ;
class ConnectionKey ;
class HttpResponse ;

/////////////////////////////////////////////////////////////////////////////
// CHTTPTransport
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CHTTPTransport : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CHTTPTransport, &CLSID_CoPocketHTTP>,
	public ISupportErrorInfo,
	public IsfDelegatingDispImpl<IHttpRequest>,
	private TransportBase<CHTTPTransport>,
	public IHttpRequestSplit
{
public:
	CHTTPTransport() ; 
	~CHTTPTransport() ; 

DECLARE_REGISTRY_RESOURCEID(IDR_HTTPTRANSPORT)
DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CHTTPTransport)
	COM_INTERFACE_ENTRY(IHttpRequest)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IHttpRequestSplit)
END_COM_MAP()

	static void WINAPI ObjectMain(bool bStarting) ;

public:
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

// IHttpRequest
	STDMETHOD(get_Method)(BSTR * method) ;
	STDMETHOD(put_Method)(BSTR method) ;

	STDMETHOD(get_Headers)(IHeadersCollection ** ppCol) ;

	STDMETHOD(GetResponse)		( BSTR endpoint, VARIANT Payload, IHttpResponse ** Response);

	STDMETHOD(SetProxy)				( BSTR ProxyServer, short ProxyPort ) ;
	STDMETHOD(NoProxy)				() ;
	STDMETHOD(Authentication)		( BSTR username, BSTR password ) ;
	STDMETHOD(ProxyAuthentication)	( BSTR username, BSTR password ) ;
	STDMETHOD(put_Timeout)			( long timeOut )   ;
	STDMETHOD(get_Timeout)			( long * timeOut ) ;

	STDMETHOD(get_Option)			( BSTR optionName, VARIANT * pVal ) ;
	STDMETHOD(put_Option)			( BSTR optionName,  VARIANT val ) ;

// IHttpRequestSplit
	STDMETHOD(Send)		(BSTR endpoint, VARIANT Payload) ;
	STDMETHOD(Response)	(IHttpResponse ** Response) ;

protected:
	void	BufferForNextRead	( BYTE *pvx, DWORD cb ) ;
	HRESULT Read				( void* pv, DWORD cb, DWORD * pcbRead ) ;

	friend class ChunkedTE ;
	friend class contentLengthTE ;
	friend class connectionWillClose ;
	friend class HttpStream ;
	friend class CookieCollection;

private:
	// helpers
	HRESULT InternalRead ( void *pvx, DWORD cb, DWORD * pcbRead ) ;

	HRESULT			SendInit(BSTR endpoint) ;
	HRESULT			Send() ;
	HRESULT			Receive(CComObject<HttpResponse> * r) ;
	HRESULT			ReceiveImpl(CComObject<HttpResponse> * r) ;

	HRESULT			ConstructConnectionKey(ConnectionKey &key) ;

	HRESULT			OnRedirect			( CComObject<HttpResponse> * r, Headers &headers ) ;
	HRESULT			OnServerAuth		( CComObject<HttpResponse> * r, Headers &headers ) ;
	HRESULT			OnProxyAuth			( CComObject<HttpResponse> * r, Headers &headers ) ;

	HRESULT			BuildHeaders		( const char * szEndpoint ) ;
	HRESULT			BuildWWAuthHeader	() ;
	HRESULT			BuildProxyAuthHeader() ;
	HRESULT			BuildAuthHeader		( stringBuff_A &header, const char * name, const char * userName, const char * password ) ;

	HRESULT			CrackURL			( LPCSTR url ) ;
	stringBuff_A	buildCurrentUrl		( void ) ;
	void			ApplyURI			( std::string &uri ) ;

	void			AddCookieHeaders	( void ) ;
	void			ExtractCookies		( stringBuff_A &results ) ;
	void			AddOrSetCookie		( Cookie &c ) ;

	// connection info
	Connection *	m_connection ;
	bool			m_inHeaders ;
	bool			m_closeWhenDone ;
	bool			m_sendConnectionCloseHeader;
	std::auto_ptr<transferDecoder> 
					m_tfrDecoder ;

	CComBSTR		m_method ;
	stringBuff_A	m_server;
	stringBuff_A    m_uri ;
	u_short			m_port ;

	// cookies
	bool			m_cookiesDirty ;
	COOKIES			m_cookies ;

	// request info
	static const long REQUEST_SIZE_NOT_KNOWN ;

	stringBuff_A				m_szHeaders ;
	CComPtr<IResetableStream>	m_requestBody ;
	long						m_requestSize ;	// -1 if not known

	typedef std::vector<char> BUFFER ;
	BUFFER			m_responseBuffer ;

	bool			m_ssl ;
	bool			m_redirectOnPost ;
	short			m_maxRedirects ;
	short			m_redirects ;

	Headers			*m_headers ;
	// hold a cached copy of the flyweight wrapper around m_headers
	CComPtr<IHeadersCollection> m_headersCol ;

	// send/rec timeout
	long			m_timeout ;

	// proxy info
	CComBSTR		m_proxyServer, m_proxyUserName, m_proxyPassword ;
	short			m_proxyPort ;
	stringBuff_A	m_proxyAuthHeader ;

	// authentication info
	CComBSTR		m_userName, m_password ;
	stringBuff_A	m_authHeader ;

	short			m_authDepth ;	// keeps track of how many attempts we've had

	// options
	short			m_compressionLevel ;
	bool			m_compression ;
	bool			m_compressionIsGzip ;

	// diagnostics
	CComBSTR		m_tracingFile;
	TracingMgr		*m_tracer;

	HRESULT getCookiesCollection(VARIANT *pVal);
	HRESULT getOptionVal(bool  &optionVal, VARIANT *pVal) ;
	HRESULT setOptionVal(bool  &optionVal, VARIANT *pVal) ;
	HRESULT getOptionVal(short &optionVal, VARIANT *pVal) ;
	HRESULT getOptionVal(CComBSTR &optionVal, VARIANT *pVal);
	HRESULT setOptionVal(CComBSTR &optionVal, VARIANT *pVal);
	HRESULT setOptionVal(short &optionVal, VARIANT *pVal, short validMin = SHRT_MIN, short validMax = SHRT_MAX ) ;
	HRESULT setOptionValCompressMethod(VARIANT *val) ;
	HRESULT getOptionValCompressMethod(VARIANT *val) ;
	HRESULT setAccpetEncHeader(VARIANT *val) ;
	void	CloseTrace();
};

#endif //__HTTPTRANSPORT_H_
