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
Portions created by Simon Fell are Copyright (C) 2000-2007
Simon Fell. All Rights Reserved.

Contributor(s):
	Chris P. Vigelius	 - IStream support
    Albert Chau - 1CTI (alberto@figment.net)

Notes:
2005/11/26 - AC: Discovered that m_connection could sometimes be null especially
with malformed packets from the SOAP server.  Added an if statement to check
for null m_connection...
*/

/////////////////////////////////////////////////////////////////////////////
// HTTPTransport.cpp : Implementation of CHTTPTransport
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pocketHTTP.h"
#include "HTTPTransport.h"
#include "base64.h"
#include "stringBuffStreamReader.h"
#include "connection.h"
#include "compressionHandlers.h"

#include "httpResponse.h"
#include "httpStream.h"
#include "headersCollection.h"
#include "StreamReader.h"

static const UINT	DEF_HTTP_PORT  = 80 ;
static const UINT	DEF_HTTPS_PORT = 443 ;
const char * const	USER_AGENT_STRING = "PocketHTTP/1.3.2" ;	// todo : get version from VersionInfo

const long CHTTPTransport::REQUEST_SIZE_NOT_KNOWN = -1 ;

// option names
static const wchar_t * COMPRESS_ENABLED = L"compression.enabled" ;
static const wchar_t * COMPRESS_LEVEL   = L"compression.level" ;
static const wchar_t * COMPRESS_METHOD  = L"compression.method" ;
static const wchar_t * COMPRESS_ACCEPT  = L"compression.accept" ;
static const wchar_t * REDIRECT_DEPTH   = L"redirects.max" ;
static const wchar_t * REDIRECT_ON_POST = L"redirects.onpost" ;
static const wchar_t * TRACING_FILE		= L"tracing.file";
static const wchar_t * CONNECTION_CLOSE = L"connection.close";
static const wchar_t * REQUEST_COOKIES	= L"cookies";

static const wchar_t * COMPRESS_GZIP	= L"gzip" ;
static const wchar_t * COMPRESS_DEFLATE = L"deflate" ;

static const std::string AE("Accept-Encoding" ) ;
static const std::string DEFLATE("deflate, gzip") ;

/////////////////////////////////////////////////////////////////////////////
// CHTTPTransport
/////////////////////////////////////////////////////////////////////////////

void CHTTPTransport::ObjectMain(bool bStarting)
{
	if ( bStarting )
	{
		WSADATA wsaData ;
		if (WSAStartup(0x101, &wsaData) == SOCKET_ERROR)
		{
			// todo: WTF can i do here ?
		}
	}
	else
	{
		WSACleanup() ;
	}
}

CHTTPTransport::CHTTPTransport() : 
	m_connection(0), 
	m_port(DEF_HTTP_PORT), 
	m_timeout(15000), 
	m_proxyPort(0), 
	m_authDepth(0),
	m_maxRedirects(5),
	m_redirectOnPost(false),
	m_redirects(0),
	m_requestSize(REQUEST_SIZE_NOT_KNOWN),
	m_cookiesDirty(false),
	m_inHeaders(true),
	m_compression(false),
	m_compressionIsGzip(false),
	m_compressionLevel(Z_DEFAULT_COMPRESSION),
	m_method(OLESTR("GET")),
	m_tracer(NULL),
	m_sendConnectionCloseHeader(false)
{
	// default headers
	static const std::string UA("User-Agent") ;
	static const std::string UA_VAL(USER_AGENT_STRING) ;
	static const std::string CT("Content-Type") ;

	m_headers = new Headers ;
	m_headers->insert ( Headers::value_type ( UA, UA_VAL ) ) ;
	m_headers->insert ( Headers::value_type ( AE, DEFLATE )) ;
}

CHTTPTransport::~CHTTPTransport()
{
	// if we still own the headers collection, remember to delete it
	if ( ! m_headersCol )
		delete m_headers ;

	if ( m_connection )
		m_connection->Disconnect() ;

	CloseTrace();
}

STDMETHODIMP CHTTPTransport::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IHttpRequest, &IID_IHttpRequestSplit
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::ATL::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CHTTPTransport::get_Method(BSTR * method) 
{
	return m_method.CopyTo(method) ;
}

STDMETHODIMP CHTTPTransport::put_Method(BSTR method) 
{
	m_method = method ;
	return S_OK ;
}

STDMETHODIMP CHTTPTransport::get_Headers(IHeadersCollection ** ppCol) 
{
	if ( ! m_headersCol )
	{
		// we transfer ownership of the headers collection to the wrapper object at this point
		// this stops us from getting into propblems with circular reference counts
		CComObject<HeadersCollection> * hc = 0 ;
		_HR(hc->CreateInstance(&hc)) ;
		hc->AddRef() ;
		hc->QueryInterface(&m_headersCol) ;
		hc->Init(NULL, m_headers, true) ;
		hc->Release() ;
	}
	return m_headersCol.CopyTo(ppCol) ;
}

STDMETHODIMP CHTTPTransport::GetResponse( BSTR endpoint, VARIANT Payload, IHttpResponse ** ppResponse )
{
	if ( ! ppResponse ) return E_POINTER ;
	_HR ( Send (endpoint, Payload)) ;
	return Response(ppResponse) ;
}

STDMETHODIMP CHTTPTransport::Send(BSTR endpoint, VARIANT srcPayload) 
{
	m_requestSize = REQUEST_SIZE_NOT_KNOWN ;
	VARIANT srcData = (srcPayload.vt == (VT_VARIANT | VT_BYREF)) ? *srcPayload.pvarVal : srcPayload;
	VARTYPE vtWithoutByRef = srcData.vt & ~VT_BYREF;
	bool isByRef = (srcData.vt & VT_BYREF) != 0;
	CloseTrace();

	if ( vtWithoutByRef == ( VT_ARRAY | VT_UI1 ))
	{
		// payload source is a BYTE array
		SAFEARRAY * psa = isByRef ? *srcData.pparray : srcData.parray ;
		long lb, ub ;
		SafeArrayGetUBound(psa, 1, &ub ) ;
		SafeArrayGetLBound(psa, 1, &lb ) ;
		m_requestSize = ub - lb + 1 ;

		CComObject<stringBuffStreamReader> * reader ;
		_HR(reader->CreateInstance(&reader) ) ;
		reader->AddRef() ;
		reader->Buffer().Size(m_requestSize) ;
	
		void * data = 0 ;
		_HR(SafeArrayAccessData(psa, &data)) ;
		memcpy ( reader->Buffer().buffer(), data, m_requestSize ) ;
		SafeArrayUnaccessData(psa) ;

		m_requestBody = reader ;
		reader->Release();
	}
	else if ( vtWithoutByRef == VT_BSTR )
	{
		// payload is a unicode string
		//
		// because COM is Unicode, the request is _always_ UTF-16, 
		// but we transcode it to UTF-8 to go out over the wire
		CComObject<stringBuffStreamReader> * reader ;
		_HR(reader->CreateInstance(&reader)) ;
		reader->AddRef() ;
		Ole2Utf8 ( isByRef ? *srcData.pbstrVal : srcData.bstrVal, reader->Buffer() ) ;
		m_requestBody = reader ;
		m_requestSize = reader->Buffer().Size()  ;
		reader->Release() ;
	}
	else if ( vtWithoutByRef == VT_UNKNOWN )
	{
		// payload source is a IResetableStream or IStream impl.
		m_requestBody.Release() ;
		IUnknown * punk = isByRef ? (*srcData.ppunkVal) : srcData.punkVal;
		// check for IResetableStream
		if (!SUCCEEDED(punk->QueryInterface(__uuidof(m_requestBody), (void **)&m_requestBody)))
		{
			// IStream support from Chris P. Vigelius
			CComQIPtr<IStream> pIStr;
			_HR(punk->QueryInterface(__uuidof(pIStr), (void **)&pIStr));
			CComObject<streamStreamReader> * reader = 0;
			_HR(reader->CreateInstance(&reader));
			reader->AddRef();
			_HR(reader->Attach(pIStr));
			m_requestBody = reader;
			m_requestSize = reader->GetSize();
			reader->Release();
		}
	}
	else
		return E_INVALID_PAYLOAD ;

	// insert the tracing reading into the chain if needed
	if ( m_tracingFile.Length() > 0 )
	{
		m_tracer = new TracingMgr();
		m_tracer->AddRef();
		HRESULT hr = m_tracer->Init(m_tracingFile);
		if(FAILED(hr))
		{
			CloseTrace();
			return hr;
		}
		IReadableStream * rs = m_tracer->WrapStream(m_requestBody);
		m_requestBody.Release();
		rs->QueryInterface(__uuidof(m_requestBody), (void **)&m_requestBody);
		rs->Release();
	}

	// compression check
	if ( m_compression )
	{
		// if we're going to compress the request, then we insert a compressing reader
		// into the source chain, and reset the request length
		deflateHandlerInit * comp = 0 ;
		if(m_compressionIsGzip)
		{
			CComObject<gzipDeflateHandler> * h ;
			_HR ( h->CreateInstance(&h) ) ;
			comp = h ;
		}
		else
		{
			CComObject<deflateHandler> * h ;
			_HR ( h->CreateInstance(&h)) ;
			comp = h ;
		}

		comp->AddRef() ;
		comp->Init(m_requestBody, m_compressionLevel) ;
		m_requestBody.Release() ;
		comp->QueryInterface(__uuidof(m_requestBody), (void **)&m_requestBody) ;
		comp->Release() ;

		m_requestSize = REQUEST_SIZE_NOT_KNOWN ;
	}

	// for now, if we need to calc the length, we just iterate over the 
	// stream to find the size, we can remove this step once we
	// add support for sending a chunked encoded request
	if ( REQUEST_SIZE_NOT_KNOWN == m_requestSize )
	{
		char buff[PH_BUFFER_SIZE] ;
		DWORD cb , total = 0 ;
		do
		{
			_HR(m_requestBody->Read ( buff, sizeof(buff), &cb ));
			total += cb ;
		} while ( cb > 0 ) ;
		m_requestSize = total ;
		m_requestBody->Reset() ;
	}

	if (m_tracer)
	{
		m_tracer->setTracing(true);
		m_tracer->WriteRequestHeader();
	}

	HRESULT hr = SendInit(endpoint);
	return hr ;
}

// Send the request from m_requestBody
HRESULT CHTTPTransport::SendInit( BSTR endpoint )
{
	USES_CONVERSION ;
	m_authDepth  = 0 ;
	m_redirects	 = 0 ;
	m_cookiesDirty = false ;

	char * szEndpoint = OLE2A(endpoint) ;
	_HR( CrackURL(szEndpoint) ) ;
	_HR( BuildHeaders ( szEndpoint ) ) ;
	m_tfrDecoder.reset() ;

	return Send() ;
}

// Build the HTTP request headers into m_headers
HRESULT CHTTPTransport::BuildHeaders( const char * szEndpoint )
{
	USES_CONVERSION ;
	// build the headers
	m_szHeaders.Clear() ;
	m_szHeaders.Allocate(320) ; // rough guess, but better than nothing
	m_szHeaders.Append( OLE2A(m_method) ) ;
	m_szHeaders.Append ( " " ) ;

	// only need to send the full URL is we're proxying HTTP (not for HTTPS)
	// see http://tech.groups.yahoo.com/group/pocketsoap/message/5139
	// thanks Luke.
	if ( (m_proxyPort != 0) && !m_ssl )
		m_szHeaders.Append ( szEndpoint ) ;
	else
		m_szHeaders.Append ( m_uri ) ;
	 
	m_szHeaders.Append ( " HTTP/1.1\r\nHost: " ) ;
	m_szHeaders.Append ( m_server ) ;
	char buff[10] ;
	if ( (( ! m_ssl ) && ( m_port != DEF_HTTP_PORT )) || ( m_ssl && ( m_port != DEF_HTTPS_PORT ) ) )
	{
		m_szHeaders.Append ( ":" ) ;
		_ltoa(m_port, buff, 10 ) ;
		m_szHeaders.Append ( buff ) ;
	}
	m_szHeaders.Append("\r\n") ;

	for ( Headers::iterator hd = m_headers->begin() ; hd != m_headers->end(); hd++ )
	{
		m_szHeaders.Append ( hd->first.c_str(), hd->first.length() ) ;
		m_szHeaders.Append ( ": ", 2 ) ;
		m_szHeaders.Append ( hd->second.c_str(), hd->second.length() ) ;
		m_szHeaders.Append ( "\r\n", 2 ) ;
	}
	// force the connection to close when we're done
	if (m_sendConnectionCloseHeader)
	{
		m_szHeaders.Append("Connection: close\r\n");
	}

	// request is compressed
	if ( m_compression )
	{
		m_szHeaders.Append ( "Content-Encoding: " ) ;
		if(m_compressionIsGzip)
			m_szHeaders.Append("gzip", 4) ;
		else
			m_szHeaders.Append("deflate", 7) ;
		m_szHeaders.Append ( "\r\n", 2 ) ;
	}

	// only send the CL if its known
	if ( m_requestSize != REQUEST_SIZE_NOT_KNOWN )
	{
		// dont' set a C-L if its GET with no payload
		if ( ! (( m_requestSize == 0 ) && ( wcsicmp ( L"GET", m_method ) == 0 )))
		{
			m_szHeaders.Append ( "Content-Length: " ) ;
			_ltoa(m_requestSize, buff, 10 ) ;
			m_szHeaders.Append ( buff ) ;
			m_szHeaders.Append("\r\n") ;
		}
	}

	AddCookieHeaders() ;
	return S_OK ;
}

// builds a usefull error message from the info we have.
CComBSTR makeFailedOpeningSocketError(DWORD wsaErr, sockaddr_in &sa)
{
	USES_CONVERSION;
	TCHAR msg[256];
	wsprintf(msg, _T("Failed opening socket for address:%hs:%d (WSAErrorCode=%d)"), inet_ntoa(sa.sin_addr), ntohs(sa.sin_port), wsaErr);
	return T2BSTR(msg);
}

// this takes the previously built headers / server / port / body
// and actually makes a connection and sends the request
HRESULT CHTTPTransport::Send()
{
	// sometimes Send() gets called multiple times for a given request [redirects, authentication etc]
	// so we check the cookiesDirty flag, and rebuild the headers if we need to
	if ( m_cookiesDirty )
	{
		stringBuff_A url = buildCurrentUrl() ;
		BuildHeaders( url.c_str() ) ;
	}

	sockaddr_in sa ;
	ConnectionKey ck  ;
	HRESULT hr = ConstructConnectionKey(ck) ;
	if (FAILED(hr)) return hr ;

	// the request headers/body
	static const DWORD buff_size = PH_BUFFER_SIZE ;
	DWORD cbHeaders = m_szHeaders.Size() + m_authHeader.Size() + m_proxyAuthHeader.Size() + 2 ;
	ATLASSERT ( cbHeaders <= buff_size ) ;
	// just in case ...
	if ( cbHeaders > buff_size )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Wow the request HTTP headers are way too big, how'd that happen ?"), IID_NULL, E_TOO_MANY_HEADERS ) ;

	DWORD cb ;
	char buff[buff_size] ;
	char * pos = buff ;
	memcpy ( pos, m_szHeaders.c_str(), m_szHeaders.Size() ) ;
	pos += m_szHeaders.Size() ;

	// the authentication header(s) if needed
	if ( m_authHeader.Size() )
	{
		memcpy ( pos, m_authHeader.c_str(), m_authHeader.Size() ) ;
		pos += m_authHeader.Size() ;
	}
	if ( m_proxyAuthHeader.Size() )
	{
		memcpy ( pos, m_proxyAuthHeader.c_str(), m_proxyAuthHeader.Size() ) ;
		pos += m_proxyAuthHeader.Size() ;
	}
	memcpy ( pos, "\r\n", 2 ) ;
	pos += 2 ;

	// trace out the request Headers
	if (m_tracer)
		m_tracer->Write(buff, pos - buff);

	// fill the rest of the first chunk with the start of the request body
	_HR ( m_requestBody->Read ( pos, buff_size - (pos-buff), &cb ) ) ;
	pos += cb ;	
	bool usingPooledConn =true ;
	do
	{
		// create the client socket
		// tidy up from last time around
		if ( m_connection )
			m_connection->Disconnect() ;

		_HR ( thePool.findConnection ( ck, &m_connection ) ) ;

		usingPooledConn = true ;
		if ( INVALID_SOCKET == m_connection->socket )
		{
			m_connection->socket = socket(AF_INET, SOCK_STREAM, 0); 
			if ( INVALID_SOCKET == m_connection->socket )
				return AtlReportError ( GetObjectCLSID(), OLESTR("Failed creating socket"), __uuidof(IHttpRequest), HRESULT_FROM_WIN32(WSAGetLastError()) );

			_HR ( m_connection->PreConnect(m_server.c_str(), m_timeout) ) ;

			// connect
			m_connection->populateSockAddr(sa) ;
			if ( connect(m_connection->socket, (struct sockaddr*)&sa, sizeof(sa) ) == SOCKET_ERROR) 
			{
				DWORD wsErr = WSAGetLastError();
				CComBSTR errMsg = makeFailedOpeningSocketError(wsErr, sa);
				return AtlReportError ( GetObjectCLSID(), errMsg, __uuidof(IHttpRequest), HRESULT_FROM_WIN32(wsErr) );
			}
			_HR ( m_connection->PostConnect(m_server.c_str(), m_proxyAuthHeader) ) ;
			usingPooledConn = false ;
		}

		// set the timeout, note we have to do it here, as this might
		// of been a pooled connection that has a different timeout value
		// to what we want
		_HR ( m_connection->setTimeout(m_timeout) ) ;

		// send the actual data !, this is the first packet of a potential many
		hr = m_connection->SendChunk ( buff, pos - buff ) ;
		// this is here because there's no way to tell if the connection
		// from the pool is still up or not until you put some trafffic
		// over it, so if it fails and we're using a previously opened	
		// connection from the pool, we ditch it and try again.
		if (FAILED(hr) && (!usingPooledConn)) 
			return hr ;

	} while (FAILED(hr)) ;

	while ( cb > 0 ) 
	{
		_HR ( m_requestBody->Read ( buff , buff_size , &cb ) ) ;
		_HR ( m_connection->SendChunk ( buff, cb  ) ) ;
	} 

	return S_OK ;
}

STDMETHODIMP CHTTPTransport::Response(IHttpResponse ** Response) 
{
	if ( ! Response ) return E_POINTER ;

	CComObject<HttpResponse> * r = 0 ;
	_HR( r->CreateInstance(&r)) ;
	r->AddRef() ;

	HRESULT hr = Receive(r) ;

	r->QueryInterface(Response) ;
	r->Release() ;

	CloseTrace();
	return hr ;
}

HRESULT CHTTPTransport::Read( void* pvx, DWORD cb, DWORD * pcbRead )
{
	HRESULT hr ;
	if ( m_tfrDecoder.get() )
		hr = m_tfrDecoder->Read ( (BYTE *)pvx, cb, pcbRead ) ;
	else
		hr = InternalRead ( pvx, cb, pcbRead ) ;

	return hr;
}

void CHTTPTransport::BufferForNextRead	( BYTE *pvx, DWORD cb ) 
{
	m_responseBuffer.insert ( m_responseBuffer.begin(), pvx, pvx + cb ) ;
}

HRESULT CHTTPTransport::InternalRead( void* pvx, DWORD cb, DWORD * pcbRead )
{
	char * pv = (char *)pvx ;
	*pcbRead = 0 ;
	// empty out the responseBuffer first
	DWORD cs = 0 ;
	if ( m_responseBuffer.size() > 0 )
	{
		cs = min ( m_responseBuffer.size(), cb ) ;
		memcpy ( pv, &m_responseBuffer[0], cs ) ;
		cb -= cs ;
		pv += cs ;
		*pcbRead += cs ;
		m_responseBuffer.erase ( m_responseBuffer.begin(), m_responseBuffer.begin() + cs ) ;
	}
	else
	{
		if ( cb )
		{
			//AC: make sure m_connection is not null before using it
			int retval = 0;
			if (m_connection) 		
				retval = m_connection->ReadChunk ( pv, cb );
			if (retval == SOCKET_ERROR)
			{
				DWORD wsErr = WSAGetLastError() ;
				ATLTRACE(_T("rec error %d\n"), WSAGetLastError() ) ;
				// SOAP::Lite seems to close the connection on us straight away, don't error in this case
				if ( WSAENOTSOCK != wsErr )
				{
					m_connection->Disconnect();
					if ( WSAETIMEDOUT == wsErr )
						return AtlReportError ( GetObjectCLSID(), OLESTR("Timeout waiting for response"), __uuidof(IHttpRequest), HRESULT_FROM_WIN32(wsErr) ) ;
					return AtlReportError ( GetObjectCLSID(), OLESTR("Error whilst receiving data"), __uuidof(IHttpRequest), HRESULT_FROM_WIN32(wsErr) ) ;
				}
				retval = 0 ;
			}
			*pcbRead += retval ;
		}
	}	
	if ( !m_inHeaders)
	{
		if ( m_connection && ( *pcbRead == 0 ))
		{
			if ( m_closeWhenDone )
				m_connection->Disconnect() ;
			else
				thePool.returnToPool(m_connection) ;
			m_connection = 0 ;
		}
	}

	return S_OK ;
}

HRESULT CHTTPTransport::Receive(CComObject<HttpResponse> * r)
{
	HRESULT hr = ReceiveImpl(r) ;
	m_requestBody.Release() ;
	return hr ;
}

HRESULT CHTTPTransport::ReceiveImpl(CComObject<HttpResponse> * r)
{
	USES_CONVERSION ;
	if (( ! m_connection) || (INVALID_SOCKET == m_connection->socket ))
		return AtlReportError( GetObjectCLSID(), OLESTR("Invalid connection, call Send first"), __uuidof(IHttpRequest), E_CALL_SEND_FIRST ) ;

	short statusCode = 0 ;

	m_responseBuffer.clear() ;
	m_closeWhenDone = false ;
	stringBuff_A results ;
	int retval = 1 ;
	static const DWORD buff_size = PH_BUFFER_SIZE ;
	char buff[buff_size] ;
	HRESULT hr = S_OK ;
	DWORD cbRead = 0 ;
	char * status = 0 ;
	// pull in the headers
	m_inHeaders = true ;
	static const char * eoleol = "\r\n\r\n";
	char * endOfHeaders = NULL;
	if(m_tracer)
		m_tracer->WriteResponseHeader();
	do
	{
		while ( NULL == (endOfHeaders = strstr(results.c_str(), eoleol )))
		{
			_HR( Read(buff, buff_size, &cbRead ) ) ;
			if ( 0 == cbRead ) 
				return AtlReportError(GetObjectCLSID(), OLESTR("No response data available from the server"), IID_NULL, E_NO_RESPONSE ) ;
			results.Append ( buff, cbRead ) ;
		}
		if (m_tracer)
			m_tracer->Write(results.c_str(), endOfHeaders - results.c_str() + 4);

		// check HTTP Status
		if ( results.c_str() )
			status = strchr(results.c_str(), ' ') ;
		if ( status )
			statusCode = atoi(++status) ;
		else
			statusCode = 500  ;

		if ( 100 == statusCode )
		{
			// eat the 100 continue
			char * nextHeader = strstr ( results.c_str(), eoleol ) ;
			if ( nextHeader )
				results.erase(0, nextHeader+4-results.c_str() ) ;
		}
	} while ( statusCode == 100 ) ;

	// look at the HTTP version
	if ( strncmp (results.c_str(), "HTTP/", 5) != 0 )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Response was not HTTP"), __uuidof(IHttpRequest), E_NOT_HTTP ) ;

	bool http10 = false ;
	if ( strncmp(results.c_str() + 5, "1.0", 3 ) == 0 )
	{
		// is HTTP/1.0, don't do keep-alives
		m_closeWhenDone = true ;
		http10 = true ;
	}

	Headers headers = headerHelpers::Parse(results.c_str()) ;
	// todo: migrate to using the headers collection
	ExtractCookies ( results ) ;

	if ( 300 == statusCode || 301 == statusCode || 302 == statusCode || 307 == statusCode )
		return OnRedirect ( r, headers ) ;

	else if ( 401 == statusCode )
		return OnServerAuth ( r, headers ) ;

	else if ( 407 == statusCode )
		return OnProxyAuth ( r, headers ) ;

	// connection header
	Headers::iterator connection = headers.find("connection") ;
	if ( connection != headers.end() )
	{
		if ( _stricmp ( connection->second.c_str(), "close" ) == 0 )
			m_closeWhenDone = true;
	}

	// transfer-encoding
	// we delegate contentLength / connection managenment to an instance of transferDecoder
	// we plug in different implementations for chunked / content-length etc
	Headers::iterator transferEncoding = headers.find("transfer-encoding") ;
	if ( transferEncoding == headers.end() )
	{
		Headers::iterator contentLength = headers.find("content-length") ;
		if ( contentLength != headers.end() )
			m_tfrDecoder = std::auto_ptr<transferDecoder>(new contentLengthTE(this, atol(contentLength->second.c_str()) )) ;
		else
		{
			if ( m_closeWhenDone )
				m_tfrDecoder = std::auto_ptr<transferDecoder>(new connectionWillClose(this)) ;
			else
				return AtlReportError ( GetObjectCLSID(), OLESTR("Content-Length must be specified when not using chunked-encoding with HTTP/1.1"), __uuidof(IHttpRequest), E_NO_CONTENT_LENGTH ) ;
		}
	}
	else if ( _stricmp ( transferEncoding->second.c_str(), "chunked" ) == 0 )
		m_tfrDecoder = std::auto_ptr<transferDecoder>(new ChunkedTE(this)) ;

	else 
		return AtlReportError ( GetObjectCLSID(), OLESTR("Unsupported Transfer-Encoding returned by server"), __uuidof(IHttpRequest), E_UNSUPPORTED_TE ) ;

	// check for a content-encoding, and chain it up if needed
	Headers::iterator contentEncoding = headers.find("content-encoding") ;
	if ( contentEncoding != headers.end() )
	{
		if ( _stricmp ( contentEncoding->second.c_str(), "deflate" ) == 0 )
		{
			std::auto_ptr<transferDecoder> inflater( new inflateHandler(m_tfrDecoder) ) ;
			m_tfrDecoder = inflater ;
		}
		else if ( _stricmp ( contentEncoding->second.c_str(), "gzip" ) == 0 )
		{
			std::auto_ptr<transferDecoder> inflater( new gzipInflateHandler(m_tfrDecoder ) ) ;
			m_tfrDecoder = inflater ;
		}
		else
			return AtlReportError ( GetObjectCLSID(), OLESTR("Unsupported Content-Encoding returned by server"), __uuidof(IHttpRequest), E_UNSUPPORTED_CE ) ;
	}

	ATLTRACE(_T("is HTTP/1.0 : %s close after this transaction %s\n"), http10 ? "true" : "false", m_closeWhenDone ? "true" : "false" ) ;

	// put any of the response body we might of got, back into the responseBuffer, so that when the client starting reading the response
	// buffer, then get the whole response body.
	char * body = strstr ( results.c_str(), "\r\n\r\n") ;
	if ( body != NULL )
	{
		body += 4 ;
		char * end = (char *)results.c_str() + results.Size() ;
		BufferForNextRead ( (BYTE *)body, end - body ) ;
	}
	m_inHeaders = false ;

	CComPtr<IReadableStream> s ;
	CComObject<HttpStream> * resStm = 0 ;
	_HR ( resStm->CreateInstance(&resStm) ) ;
	resStm->AddRef() ;
	resStm->Init(this) ;
	resStm->QueryInterface(&s) ;
	resStm->Release() ;

	if (m_tracer)
	{
		IReadableStream * rs= m_tracer->WrapStream(s);
		s = rs;
		rs->Release();
	}

	r->Init ( s, headers, statusCode ) ;

	return S_OK ;
}

HRESULT	CHTTPTransport::OnProxyAuth ( CComObject<HttpResponse> * r, Headers &headers ) 
{
	// proxy authentication needed 
	static const char PROXY_AUTHENTICATE[] = "Proxy-Authenticate" ;
	static const char PROXY_BASIC[]		   = "Basic" ;

	std::pair<Headers::iterator, Headers::iterator> range ;
	range = headers.equal_range(PROXY_AUTHENTICATE) ;

	bool bCanAuth = false ;
	while ( range.first != range.second )
	{
		if ( _strnicmp ( range.first->second.c_str(), PROXY_BASIC, sizeof(PROXY_BASIC) ) == 0 )
		{
			bCanAuth = true ;
			break ;
		}
		range.first++ ;
	}

	if ( (!bCanAuth) || m_authDepth > 1 || m_proxyUserName.Length() == 0 )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Proxy Authentication failed"), __uuidof(IHttpRequest), E_ACCESSDENIED ) ;

	ATLTRACE(_T("Proxy Auth challenge detected\n")) ;
	BuildProxyAuthHeader() ;
	++m_authDepth ;

	// re-send the request
	m_connection->Disconnect() ;
	m_connection = 0 ;
	m_requestBody->Reset() ;
	_HR ( Send() ) ;

	return Receive(r) ;
}

HRESULT	CHTTPTransport::OnServerAuth ( CComObject<HttpResponse> * r, Headers &headers ) 
{
	// find the www-authenticate header
	static const char WWW_AUTHENTICATE[] = "WWW-Authenticate" ; 
	static const char WWW_BASIC[]		 = "Basic" ;

	std::pair<Headers::iterator, Headers::iterator> range ;
	range = headers.equal_range(WWW_AUTHENTICATE) ;

	bool bCanAuth = false ;
	while ( range.first != range.second )
	{
		if ( _strnicmp ( range.first->second.c_str(), WWW_BASIC, sizeof(WWW_BASIC) ) == 0 )
		{
			bCanAuth = true ;
			break ;
		}
		range.first++ ;
	}

	if ( (! bCanAuth) || m_authDepth > 1 || m_userName.Length() == 0 )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Server Authentication failed"), __uuidof(IHttpRequest), E_ACCESSDENIED ) ;

	ATLTRACE(_T("Server Auth challenge detected\n")) ;
	BuildWWAuthHeader() ;
	++m_authDepth ;

	// re-send the request
	m_connection->Disconnect() ;
	m_connection = 0 ;
	m_requestBody->Reset() ;
	_HR (Send() ) ;
	return Receive(r) ;
}

HRESULT	CHTTPTransport::OnRedirect( CComObject<HttpResponse> * r, Headers &headers ) 
{
	static const char * HTTP_LOCATION = "Location" ;
	++m_redirects ;

	if (( ! m_redirectOnPost ) && ( wcsicmp ( m_method, L"POST") == 0 ))
		return AtlReportError ( GetObjectCLSID(), OLESTR("Follow redirect on POST is disabled"), __uuidof(IHttpRequest), E_NO_FOLLOW ) ;

	if ( m_redirects >= m_maxRedirects )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Reached maximum re-direct depth"), __uuidof(IHttpRequest), E_MAX_REDIRECTS ) ;

	// find the location header
	Headers::iterator location = headers.find(HTTP_LOCATION) ;
	if ( location == headers.end() )
		return AtlReportError ( GetObjectCLSID(), OLESTR("HTTP redirect received, but no location header found"), __uuidof(IHttpRequest), E_NO_LOCATION ) ;

	// update the URI parts we have based on the new URI
	ApplyURI(location->second) ;
	ATLTRACE(_T("Doing redirect : depth=%d newLoc=%s\n"), m_redirects, location->second.c_str() ) ;
	
	// given the new URI parts, rebuild a complete URL [needed if we are proxying]
	stringBuff_A newURL = buildCurrentUrl() ;
	ATLTRACE(_T("revised URL : %s\n"), newURL.c_str() ) ;

	// we need to rebuild the headers
	BuildHeaders ( newURL.c_str() ) ;

	// re-send the request
	m_connection->Disconnect() ;
	m_connection = 0 ;
	m_requestBody->Reset() ;
	_HR( Send() ) ;
	return Receive(r) ;
}

stringBuff_A CHTTPTransport::buildCurrentUrl( void )
{
	stringBuff_A url ;
	// resoanble guess to required buffer size
	url.Allocate(m_server.Size() + m_uri.Size() + 25 ) ;
	url.Append("http") ;
	if ( m_ssl )
		url.Append ( "s" ) ;
	url.Append("://") ;
	url.Append(m_server) ;
	url.Append(":") ;
	char buff[10] ;
	_itoa(m_port, buff, 10 ) ;
	url.Append(buff) ;
	url.Append(m_uri) ;

	return url ;
}

void CHTTPTransport::ApplyURI( std::string &uri )
{
	static const char http[]   = "http://" ;
	static const char https[]  = "https://" ;
	static const size_t chttp  = sizeof http  / sizeof *http -1 ;
	static const size_t chttps = sizeof https  / sizeof *https -1 ;

	if ( ( _strnicmp ( http, uri.c_str(), chttp ) == 0 ) || _strnicmp ( https, uri.c_str(), chttps ) == 0 )
	{
		CrackURL (uri.c_str()) ;
	}
	else if ( *uri.c_str() == '/' )
	{
		m_uri = uri.c_str() ;
	}
	else
	{
		const char * lastSlash = strrchr ( m_uri.c_str(), '/' ) ;
		stringBuff_A newURI  ;
		if ( lastSlash )
			newURI.Append ( m_uri.c_str(), lastSlash - m_uri.c_str()+1 ) ;
		else
			newURI.Append("/",1) ;

		newURI.Append ( uri.c_str() ) ;
		m_uri = newURI ;
	}
}

// this builds a basic authentication authorization header into the header string buffer
HRESULT CHTTPTransport::BuildAuthHeader ( stringBuff_A &header, const char * name, const char * userName, const char * password )
{
	USES_CONVERSION ;
	header.Clear() ;
	header.Append ( name ) ;
	stringBuff_A credentials ;
	credentials.Append ( userName ) ;
	credentials.Append ( ":" ) ;
	credentials.Append ( password ) ;
	DWORD cbEncCred = credentials.Size() ;
	cbEncCred  = cbEncCred * 4 / 3 ;
	cbEncCred += cbEncCred %4 > 0 ? 4 - cbEncCred %4 : 0 ;
	
	char * encCred = (char *)alloca(cbEncCred + sizeof(char)) ;
	memset ( encCred, 0, cbEncCred + sizeof(char) ) ;
	_HR( base64<char>::BufferEncode64 ( encCred, cbEncCred, (const BYTE *)credentials.c_str(), credentials.Size() ) ) ;

	header.Append ( encCred ) ;
	header.Append ( "\r\n" ) ;
	return S_OK ;
}

HRESULT CHTTPTransport::BuildWWAuthHeader() 
{
	USES_CONVERSION ;
	return BuildAuthHeader ( m_authHeader, "Authorization: Basic ", OLE2A(m_userName), OLE2A(m_password) ) ;
}

HRESULT CHTTPTransport::BuildProxyAuthHeader() 
{
	USES_CONVERSION ;
	return BuildAuthHeader ( m_proxyAuthHeader, "Proxy-Authorization: Basic ", OLE2A(m_proxyUserName), OLE2A(m_proxyPassword) ) ;
}

HRESULT CHTTPTransport::CrackURL( LPCSTR url )
{
	static const char http[]   = "http://" ;
	static const char https[]  = "https://" ;
	static const size_t chttp  = sizeof http  / sizeof *http -1 ;
	static const size_t chttps = sizeof https  / sizeof *https -1 ;

	if ( strlen(url) < chttp )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Invalid URL specified, expecting http:// or https://"), __uuidof(IHttpRequest), E_INVALID_URL ) ;

	m_ssl = false ;
	if ( _strnicmp ( url , http, chttp ) != 0 )
	{
		if ( _strnicmp ( url, https, chttps ) == 0 ) 
			m_ssl = true ;
		else
			return AtlReportError ( GetObjectCLSID(), OLESTR("Invalid URL specified, expecting http:// or https://"), __uuidof(IHttpRequest), E_INVALID_URL ) ;
	}

	const char * p = url + chttp ;
	if ( m_ssl ) 
	{
		++p ;
		m_port = DEF_HTTPS_PORT ;
	}
	else
		m_port = DEF_HTTP_PORT ;

	const char * p2 = strchr(p, '/') ;
	const char * p3 = strchr(p, ':') ;
	if ( !p2 )
		p2 = p + strlen(p) ;

	const char * pEndOfServer = p2 ;
	// get port
	if ( p3 && p2 && (p3 < p2) )
	{
		m_port = atoi(p3+1) ;
		pEndOfServer = p3 ;
	}

	m_server.Clear();
	m_server.Append ( p, pEndOfServer-p ) ;

	m_uri.Clear() ;
	if ( strlen(p2) )
		m_uri.Append ( p2 ) ;
	else
		m_uri.Append ( "/", 1 );

	return S_OK ;
}

STDMETHODIMP CHTTPTransport::put_Timeout( long timeOut )
{
	m_timeout = timeOut ;
	return S_OK ;
}

STDMETHODIMP CHTTPTransport::get_Timeout( long * timeOut )
{
	if ( ! timeOut ) return E_POINTER ;
	*timeOut = m_timeout ;
	return S_OK ;
}

STDMETHODIMP CHTTPTransport::SetProxy( BSTR ProxyServer, short ProxyPort ) 
{
	m_proxyServer = ProxyServer ;
	m_proxyPort = ProxyPort ;
	return S_OK ;
}

STDMETHODIMP CHTTPTransport::NoProxy() 
{
	m_proxyServer.Empty() ;
	m_proxyPort = 0 ;
	m_proxyAuthHeader.Clear() ;
	return S_OK ;
}

STDMETHODIMP CHTTPTransport::Authentication( BSTR username, BSTR password )
{
	m_userName = username ;
	m_password = password ;
	if ( m_userName.Length() > 0 )
		BuildWWAuthHeader() ;
	else
		m_authHeader.Clear() ;

	return S_OK ;
}

STDMETHODIMP CHTTPTransport::ProxyAuthentication( BSTR username, BSTR password )
{
	m_proxyUserName = username ;
	m_proxyPassword = password ;
	if ( m_proxyUserName.Length() > 0 )
		BuildProxyAuthHeader() ;
	else
		m_proxyAuthHeader.Clear() ;

	return S_OK ;
}

HRESULT CHTTPTransport::ConstructConnectionKey(ConnectionKey &key)
{
	// ssl
	key.ssl = m_ssl ;
	// auth info
	key.setAuthInfo ( m_userName, m_password, m_proxyUserName, m_proxyPassword ) ;

	key.port = htons(m_port) ;
	// proxy info
	if ( m_proxyServer.Length() > 0 )
	{
		key.addr = 0;
		key.px_port = htons(m_proxyPort);
		if (key.ssl)
			key.setTargetUrlHash(m_server.c_str());
		return resolveName ( m_proxyServer, &key.px_addr ) ;
	}
	key.px_addr = 0 ;
	key.px_port = 0 ;
	// destination info
	return resolveName ( m_server.c_str(), &key.addr ) ;
}

void CHTTPTransport::AddCookieHeaders()
{
	stringBuff_A cv ;
	for ( unsigned int i = 0 ; i < m_cookies.size() ; ++i )
	{
		Cookie & c = m_cookies[i] ;
		if ( strcmp ( c.domain.c_str(), m_server.c_str() ) == 0 )
		{
			const char * ls = strrchr ( m_uri.c_str(), '/' ) ;
			if ( strncmp ( m_uri.c_str(), c.path.c_str(), min((long)c.path.Size(), ls - m_uri.c_str()) ) == 0 )
			{
				if ( cv.Size() )
					cv.Append ( ";" ) ;
				cv.Append ( c.name ) ;
				cv.Append ( "=" ) ;
				cv.Append ( c.val ) ;
			}
		}
	}
	if ( cv.Size() )
	{
		m_szHeaders.Append ( "Cookie: " );
		m_szHeaders.Append ( cv ) ;
		m_szHeaders.Append ( "\r\n" ) ;
	}
	m_cookiesDirty = false ;
}

void CHTTPTransport::ExtractCookies( stringBuff_A &results )
{
	stringBuff_A cv = headerHelpers::ExtractHeaderVal ( results.c_str(), "Set-Cookie" ) ;
	if ( ! cv.Size() )
		return ;

	// Set-Cookie: ASPSESSIONIDGGQGGMDG=EJELKNNAIFECFNJDBEIMAGNI; path=/; someotherattribute=foo; someotherattribute; nextcookie=bar

	// break it up into a list of items
	typedef arrayOfString<char> STRLIST;
	STRLIST attribs ;
	stringBuff_A * pi ;
	const char *p = cv.c_str() , *p2 = p , *last = cv.c_str() + cv.Size() -1 ;
	while ( p2 )
	{
		while ( *p == ' ' )
			++p ;
		p2 = strchr ( p, ';' );
		if ( p2 || p < last )
		{
			pi = attribs.Add() ;
			pi->Append ( p, ( p2 ? p2 : last+1 ) -p ) ;
			p = p2 + 1 ;
		}
	}

	Cookie c ;
	bool newCookie = true ;
	bool found ;
	for ( unsigned int i = 0 ; i < attribs.GetSize() ; ++i )
	{
		pi= attribs[i] ;
		if ( newCookie )
		{
			if ( c.name.Size() )
				AddOrSetCookie ( c ) ;

			newCookie = false ;
			c.name.Clear() ;
			c.domain.Clear() ;
			c.path.Clear() ;
			c.val.Clear() ;

			p = strchr ( pi->c_str(), '=' ) ;
			if (p)
			{
				c.name.Append ( pi->c_str(), p - pi->c_str() ) ;
				c.val.Append ( p+1 ) ;
			}
			else
				c.name.Append ( *pi ) ;
		}
		else
		{
			// known attributes
			// Comment, CommentURL, Discard, Domain, Max-Age,Path,Port,Secure,Version, Expires
			// these we process
			//		Domain, Path
			const char * atribNames[] = { "Comment", "CommentURL", "Discard", "Max-Age", "Port", "Secure", "Version", "Expires" } ;
			p = strchr ( pi->c_str(), '=' ) ;
			if ( _strnicmp ( pi->c_str(), "Domain", 6 ) == 0 )
				c.domain.Append ( p+1 ) ;
			else if ( _strnicmp ( pi->c_str(), "Path", 4 ) == 0 )
				c.path.Append ( p+1 ) ;
			else
			{
				found = false ;
				for ( int j = 0 ; j < sizeof(atribNames) / sizeof(*atribNames) ; ++j )
				{
					if ( 0 == _strnicmp ( pi->c_str(), atribNames[j], strlen(atribNames[j]) ) )
					{
						found = true ;
						break ;
					}	
				}
				if ( ! found )
				{
					// this is the start of the next cookie
					--i ;
					newCookie =true ;
				}
			}
		}
	}
	if ( c.name.Size() )
		AddOrSetCookie ( c ) ;
}

void CHTTPTransport::AddOrSetCookie ( Cookie &newc ) 
{
	m_cookiesDirty = true ;

	if ( ! newc.domain.Size() )
		newc.domain.Append ( m_server ) ;

	if ( ! newc.path.Size() )
	{
		const char * p = strrchr ( m_uri.c_str(), '/' ) ;
		newc.path.Append ( m_uri.c_str(), p - m_uri.c_str()+1 ) ;
	}

	for ( unsigned int i = 0 ; i < m_cookies.size() ; ++i ) 
	{
		Cookie &c = m_cookies[i] ;
		if ( strcmp ( c.name.c_str(), newc.name.c_str() ) == 0 )
		{
			if ( strcmp ( c.domain.c_str() , newc.domain.c_str() ) == 0 )
			{
				c.path = newc.path ;
				c.val  = newc.val ;
				return ;
			}
		}
	}
	m_cookies.push_back(newc) ;
}

HRESULT CHTTPTransport::getOptionVal(CComBSTR &optionVal, VARIANT *pVal)
{
	VariantInit(pVal);
	pVal->vt = VT_BSTR;
	pVal->bstrVal = optionVal.Copy();
	return S_OK;
}

HRESULT CHTTPTransport::setOptionVal(CComBSTR &optionVal, VARIANT *pVal)
{
	CComVariant vVal;
	HRESULT hr = vVal.ChangeType ( VT_BSTR, pVal );
	if (FAILED(hr)) return hr;
	optionVal = ::SysAllocStringLen(vVal.bstrVal, ::SysStringLen(vVal.bstrVal));
	return S_OK;
}

HRESULT CHTTPTransport::getOptionVal(bool &optionVal, VARIANT *pVal) 
{
	VariantInit(pVal) ;
	pVal->vt = VT_BOOL ;
	pVal->boolVal = optionVal ? VARIANT_TRUE : VARIANT_FALSE ;
	return S_OK ;
}

HRESULT CHTTPTransport::setOptionVal(bool &optionVal, VARIANT *pVal)
{
	CComVariant vVal ;
	HRESULT hr = vVal.ChangeType ( VT_BOOL, pVal ) ;
	if ( FAILED(hr)) return hr ;
	optionVal = ( vVal.boolVal == VARIANT_TRUE ) ;
	return S_OK ;
}

HRESULT CHTTPTransport::getOptionVal(short &optionVal, VARIANT *pVal) 
{
	VariantInit(pVal) ;
	pVal->vt = VT_I2 ;
	pVal->iVal = optionVal ;
	return S_OK ;
}

HRESULT CHTTPTransport::setOptionVal(short &optionVal, VARIANT *pVal, short validMin, short validMax)
{
	CComVariant vVal ;
	HRESULT hr = vVal.ChangeType ( VT_I2, pVal ) ;
	if ( FAILED(hr)) return hr ;
	if (( vVal.iVal > validMax ) || ( vVal.iVal < validMin ))
		return E_INVALIDARG ;
	optionVal = vVal.iVal ;
	return S_OK ;
}

HRESULT CHTTPTransport::getCookiesCollection(VARIANT *pVal)
{
	CComObject<CookieCollection> * cc = 0;
	_HR(cc->CreateInstance(&cc));
	cc->AddRef();
	cc->Init(this);
	VariantInit(pVal);
	pVal->vt = VT_DISPATCH;
	HRESULT hr = cc->QueryInterface(IID_IDispatch, (void **)&pVal->pdispVal);
	cc->Release();
	return hr;
}

STDMETHODIMP CHTTPTransport::get_Option( BSTR optionName, VARIANT * pVal )
{
	if ( !pVal) return E_POINTER ;

	if ( wcsicmp ( optionName, COMPRESS_ENABLED ) == 0 )
		return getOptionVal(m_compression, pVal) ;

	else if ( wcsicmp ( optionName, COMPRESS_LEVEL ) == 0 )
		return getOptionVal(m_compressionLevel, pVal) ;

	else if ( wcsicmp ( optionName, REDIRECT_DEPTH ) == 0 )
		return getOptionVal(m_maxRedirects, pVal) ;
	
	else if ( wcsicmp ( optionName, REDIRECT_ON_POST ) == 0 )
		return getOptionVal(m_redirectOnPost, pVal ) ;

	else if ( wcsicmp ( optionName, COMPRESS_METHOD ) == 0 )
		return getOptionValCompressMethod(pVal) ;

	else if ( wcsicmp ( optionName, TRACING_FILE ) == 0 )
		return getOptionVal(m_tracingFile, pVal);

	else if ( wcsicmp ( optionName, CONNECTION_CLOSE ) == 0 )
		return getOptionVal(m_sendConnectionCloseHeader, pVal);

	else if ( wcsicmp ( optionName, REQUEST_COOKIES ) == 0 )
		return getCookiesCollection(pVal);

	else if ( wcsicmp ( optionName, COMPRESS_ACCEPT ) == 0 )
	{
		bool hasAE = m_headers->find(AE) != m_headers->end() ;
		return getOptionVal(hasAE, pVal ) ;
	}

	return E_INVALIDARG ;
}

STDMETHODIMP CHTTPTransport::put_Option( BSTR optionName, VARIANT val )
{
	if ( wcsicmp ( optionName, COMPRESS_ENABLED ) == 0 )
		return setOptionVal(m_compression, &val) ;

	else if ( wcsicmp ( optionName, COMPRESS_LEVEL ) == 0 )
		return setOptionVal(m_compressionLevel, &val, Z_DEFAULT_COMPRESSION, Z_BEST_COMPRESSION ) ;

	else if ( wcsicmp ( optionName, REDIRECT_DEPTH ) == 0 )
		return setOptionVal(m_maxRedirects, &val, 0, 100 ) ;

	else if ( wcsicmp ( optionName, REDIRECT_ON_POST ) == 0 )
		return setOptionVal(m_redirectOnPost, &val ) ;

	else if ( wcsicmp ( optionName, COMPRESS_METHOD ) == 0 ) 
		return setOptionValCompressMethod(&val) ;

	else if ( wcsicmp ( optionName, COMPRESS_ACCEPT ) == 0 )
		return setAccpetEncHeader(&val) ;

	else if ( wcsicmp ( optionName, TRACING_FILE ) == 0 )
		return setOptionVal(m_tracingFile, &val);

	else if ( wcsicmp ( optionName, CONNECTION_CLOSE) == 0 )
		return setOptionVal(m_sendConnectionCloseHeader, &val);

	return E_INVALIDARG ;
}

HRESULT CHTTPTransport::setAccpetEncHeader(VARIANT *val)
{
	CComVariant v ;
	_HR(v.ChangeType(VT_BOOL, val)) ;
	Headers::iterator ae = m_headers->find(AE) ;
	if ( v.boolVal == VARIANT_TRUE )
	{
		if ( ae == m_headers->end() )
			m_headers->insert ( Headers::value_type ( AE, DEFLATE )) ;
		else
			ae->second = DEFLATE ;
	}
	else
	{
		if ( ae != m_headers->end() )
			m_headers->erase(ae) ;
	}
	return S_OK ;
}

HRESULT CHTTPTransport::setOptionValCompressMethod(VARIANT *val)
{
	CComVariant v ;
	HRESULT hr = v.ChangeType(VT_BSTR, val) ;
	if(FAILED(hr))
		return hr ;

	if ( wcsicmp ( v.bstrVal, COMPRESS_GZIP ) == 0 )
		m_compressionIsGzip = true ;
	
	else if ( wcsicmp ( v.bstrVal, COMPRESS_DEFLATE ) == 0 )
		m_compressionIsGzip = false ;
	
	else
		return AtlReportError(GetObjectCLSID(), OLESTR("Compression method must be 'gzip' or 'deflate'"), IID_IHttpRequest, E_INVALID_COMP_METHOD ) ;

	return S_OK ;
}

HRESULT CHTTPTransport::getOptionValCompressMethod(VARIANT *val)
{
	VariantClear(val) ;
	val->bstrVal = SysAllocString(m_compressionIsGzip ? COMPRESS_GZIP : COMPRESS_DEFLATE) ;
	val->vt = VT_BSTR ;
	return S_OK ;
}

void CHTTPTransport::CloseTrace()
{
	if (m_tracer)
	{
		m_tracer->Release();
		m_tracer = NULL;
	}
}