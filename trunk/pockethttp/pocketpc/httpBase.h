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

#pragma once

#pragma comment(lib, "winsock.lib")

#include <sslsock.h>
#include <Wincrypt.h>
#include <schnlsp.h>
#include "stringBuff.h"

extern const char * const USER_AGENT_STRING ;
class ConnectionPool ;
extern ConnectionPool thePool ;

static SSL_CRACK_CERTIFICATE_FN		gSslCrackCertificate = 0 ;
static SSL_FREE_CERTIFICATE_FN		gSslFreeCertificate  = 0 ;
static HINSTANCE					gSchannel = NULL ;

// we have a dedicated thread pool that is doing reads for SSL, the original thread requesting the read can then
// monitor the read for timeouts. 
// this is because of the problems trying to use either 
//		select with SSL (buggy, blocks when there's data available at times)
//		see http://groups.google.com/groups?hl=en&lr=&ie=UTF-8&oe=UTF-8&selm=8998b601.0205291646.2f337a5d%40posting.google.com
// or non blocking sockets with SSL (no supported on PocketPC 2003)
//		see http://msdn.microsoft.com/library/default.asp?url=/library/en-us/wcewinsk/html/_wcecomm_using_secure_sockets.asp
// we initially had the read on the request thread, and a single thread doing timeout monitoring but in some cases
// calling closesocket won't force recv to return, so we have to spin up other threads to do the read and face the fact that
// occasionally there will be some abandoned threads.
// anyone with suggestions of how to improved this, and stay compatibile with PocketPC , PPC 2002, PPC 2003 please shout up!
//

class ReadRequest
{
public:
	ReadRequest(SOCKET s, char *buffer, int len)
	{
		this->s = s ;
		this->buffer = buffer ;
		this->len = len ;
		result = 0 ;
		rc = 1 ;
		finishedEvent = CreateEvent ( NULL, TRUE, FALSE, NULL ) ;
	}
	~ReadRequest()
	{
		CloseHandle(finishedEvent) ;
	}
	void AddRef()
	{
		++rc ;
	}
	void Release()
	{
		if( 0 == --rc )
			delete this ;
	}

	SOCKET	s ;
	char	*buffer ;
	int		len ;
	int		result ;
	int		rc ;
	HANDLE	finishedEvent ;
} ;

static DWORD WINAPI SSLReadThread ( void * param ) ;

class ReadRequestPool
{
public:
	ReadRequestPool() :
		m_availableThreads(0),
		m_totalThreads(0)
	{
		m_dowork = CreateEvent( NULL, FALSE, FALSE, NULL ) ;
	}

	~ReadRequestPool()
	{
		CloseHandle(m_dowork) ;
	}

	void StartRequest ( ReadRequest * rr )
	{
		m_lock.Lock() ;
		rr->AddRef() ;
		m_work.push_back(rr) ;
		if ( m_availableThreads == 0 )
		{
			CreateThread ( NULL, 0, SSLReadThread, this, 0, NULL ) ;
			++m_availableThreads ;
			++m_totalThreads ;
		}
		m_lock.Unlock() ;
		SetEvent(m_dowork) ;
	}

	DWORD WINAPI ThreadProc()
	{
		DWORD ts = 0 ;
		ReadRequest * r = 0 ;
		do
		{
			ts = WaitForSingleObject ( m_dowork, 60000 ) ;
			if ( WAIT_TIMEOUT == ts )
			{
				// if we don't get anything todo after 1 minute, 
				// and there are other worker threads still available let this one go
				m_lock.Lock() ;
				if ( m_availableThreads > 1 )
				{
					--m_availableThreads ;
					--m_totalThreads ;
					m_lock.Unlock() ;
					return WAIT_TIMEOUT ;
				}
				m_lock.Unlock() ;
			}
			else if ( WAIT_OBJECT_0 == ts )
			{
				// we have stuff to do
				m_lock.Lock();
				--m_availableThreads ; // we're busy
				ReadRequest * r = m_work.front() ;
				m_work.pop_front() ;
				m_lock.Unlock() ;

				r->result = recv ( r->s, r->buffer, r->len, 0 ) ;
				SetEvent ( r->finishedEvent ) ;
				r->Release() ;

				m_lock.Lock() ;
				++m_availableThreads ;	// available to do stuff again
				m_lock.Unlock() ;
			}
		} while(true) ;
	}

private:
	long						m_availableThreads, m_totalThreads ;
	CComAutoCriticalSection		m_lock ;
	std::deque<ReadRequest *>	m_work ;
	HANDLE						m_dowork ;
} ;

static DWORD WINAPI SSLReadThread ( void * param ) 
{
	ReadRequestPool * pPool = (ReadRequestPool *)param ;
	return pPool->ThreadProc() ;
}

static ReadRequestPool g_sslReadPool ;

template<class T>
class httpTransportBase
{
public:
	static void ObjectMain(bool bStarting)
	{
		if ( !bStarting )
		{
			if ( gSchannel )
			{
				gSslCrackCertificate = 0 ;
				gSslFreeCertificate  = 0 ;
				FreeLibrary(gSchannel) ;
				gSchannel = NULL ;
			}
		}
	}

	HRESULT PreConnect(const char * serverName, long timeout)
	{
		setTimeout(timeout) ;
		T * t = static_cast<T *>(this) ;
		if ( t->ssl )
		{
			// pocketPC SSL support
			_HR(InitSsl()) ;
			BYTE buff[16] ;
			SSLVALIDATECERTHOOK sslHook ;
			sslHook.pvArg = (void *)serverName ;
			sslHook.HookFunc = MySslHookFunc ;
			DWORD dw= 0, ssl = SO_SEC_SSL ;

			if ( setsockopt ( t->socket, SOL_SOCKET, SO_SECURE, (char *)&ssl, sizeof(ssl) ) )
				return AtlReportError ( CLSID_CoPocketHTTP, OLESTR("Failed changing socket to SSL mode"), __uuidof(IHttpRequest), HRESULT_FROM_WIN32(WSAGetLastError()) ) ;

			if ( WSAIoctl ( t->socket, SO_SSL_SET_VALIDATE_CERT_HOOK, &sslHook, sizeof sslHook, buff, 16, &dw, NULL, NULL ) )
				return AtlReportError ( CLSID_CoPocketHTTP, OLESTR("Failed setting cert hook"), __uuidof(IHttpRequest), HRESULT_FROM_WIN32(WSAGetLastError()) ) ;
			
			ssl = SSL_FLAG_DEFER_HANDSHAKE;
			if ( WSAIoctl ( t->socket, SO_SSL_SET_FLAGS, &ssl, sizeof ssl, buff, 16, &dw, NULL, NULL ) )
				return AtlReportError ( CLSID_CoPocketHTTP, OLESTR("Failed setting deffered handshake"), __uuidof(IHttpRequest), HRESULT_FROM_WIN32(WSAGetLastError()) ) ;
		}
		return S_OK ;
	}

	HRESULT PostConnect(const char * serverName, const stringBuff_A &proxyAuthHeader)
	{
		USES_CONVERSION ;
		T * t = static_cast<T *>(this) ;
		if ( t->ssl )
		{
			// check for SSL w/ Proxy
			if ( t->px_port != 0 )
			{
				HRESULT hr = SetupSSLProxy(t, serverName, proxyAuthHeader) ;
				if (FAILED(hr)) return hr ;
			}
			if ( WSAIoctl( t->socket, SO_SSL_PERFORM_HANDSHAKE, (void *)(serverName), strlen(serverName)+1, NULL, 0, NULL, NULL, NULL ) )
				return AtlReportError ( CLSID_CoPocketHTTP, L"Failed during SSL handshake", __uuidof(IHttpRequest), HRESULT_FROM_WIN32(WSAGetLastError()) ) ;
		}
		return S_OK ;
	}

	HRESULT setTimeout(long timeout)
	{
		m_timeout = timeout ;
		return S_OK;
	}

	// this sends a chunk of data (the socket must already be connected)
	// and correctly checks for errors
	HRESULT SendChunk(const char * data, long size)
	{
		if ( 0 == size ) return S_OK ;	// catch no-op case
		T * t = static_cast<T *>(this) ;

		HRESULT hr = S_OK ;
		int retval ;
		// send the request
		while ( size > 0 )
		{
			retval = WriteWithSelect(data, size);
			if (SOCKET_ERROR == retval)
			{
				DWORD wsaErr = WSAGetLastError() ;
				if ( WSAEWOULDBLOCK != wsaErr )
				{
					closesocket(t->socket);
					hr = HRESULT_FROM_WIN32(wsaErr) ;
					if ( WSAETIMEDOUT == wsaErr )
						return AtlReportError ( CLSID_CoPocketHTTP, OLESTR("Timeout sending to destination"), __uuidof(IHttpRequest), hr ) ;
					else
						return AtlReportError ( CLSID_CoPocketHTTP, OLESTR("Failing sending data to server"), __uuidof(IHttpRequest), hr ) ;
				}
			}
			size -= retval ;
			data += retval ;
			if(size) 
				Sleep(0) ;
		}
		return hr ;
	}

	int ReadChunk ( char *szBuff, size_t cbBuff )
	{
		T * t = static_cast<T *>(this) ;
		if ( t->ssl )
			return ReadChunkWithMonitor(szBuff, cbBuff) ;
		return ReadChunkWithSelect ( szBuff, cbBuff ) ;
	}

	void Disconnect()
	{
		T * t = static_cast<T *>(this) ;
		closesocket ( t->socket ) ;
		t->socket = INVALID_SOCKET ;
	}

private:
	int WriteWithSelect(const char * pv, int len)
	{
		T * t = static_cast<T *>(this) ;
		fd_set writefds;
		FD_ZERO(&writefds) ;
		FD_SET ( t->socket, &writefds ) ;
		timeval tv;
		tv.tv_sec = m_timeout / 1000;
		tv.tv_usec = (m_timeout % 1000) * 1000;

		int rc = select ( 0, NULL, &writefds, NULL, &tv) ;
		if ( SOCKET_ERROR == rc )
			return SOCKET_ERROR;
		else if ( 0 == rc ) {
			WSASetLastError(WSAETIMEDOUT);
			return SOCKET_ERROR;
		}
		return send (t->socket, pv, len, 0);
	}

	int ReadChunkWithSelect ( char * szBuff, size_t cbBuff )
	{
		T * t = static_cast<T *>(this) ;
		fd_set readfds[1] ;
		FD_ZERO(readfds) ;
		FD_SET ( t->socket, readfds ) ;
		timeval timeout ;
		timeout.tv_sec = m_timeout / 1000 ;
		timeout.tv_usec = ( m_timeout % 1000 ) * 1000 ;

		int rc = select ( 0, readfds, NULL, NULL, &timeout) ;
		if ( 0 == rc || ( ! FD_ISSET ( t->socket, readfds ) ) )
		{
			// timeout
			WSASetLastError ( WSAETIMEDOUT ) ;
			return SOCKET_ERROR ;
		}
		if ( SOCKET_ERROR == rc )
			return rc ;
		return recv ( t->socket, szBuff, cbBuff, 0 );
	}

	int ReadChunkWithMonitor ( char * szBuff, size_t cbBuff )
	{
		T * t = static_cast<T *>(this) ;

		ReadRequest * r = new ReadRequest( t->socket, szBuff, cbBuff ) ;
		// ask the SSL Read Pool to do the recv call	
		g_sslReadPool.StartRequest(r) ;
		int rc = 0 ;
		if ( WAIT_TIMEOUT == WaitForSingleObject ( r->finishedEvent, m_timeout ) )
		{
			closesocket(t->socket) ;
			WSASetLastError ( WSAETIMEDOUT ) ;
			rc = SOCKET_ERROR ;
		}
		else
			rc = r->result ;
		r->Release();
		return rc ;
	}


	HRESULT InitSsl()
	{
		static CComAutoCriticalSection lock ;
		HRESULT hr = S_OK ;
		if ( 0 == gSslFreeCertificate )
		{
			lock.Lock() ;
			if ( 0 == gSslFreeCertificate )
			{
				gSchannel = LoadLibrary ( L"schannel.dll" ) ;
				if ( NULL != gSchannel )
				{
					gSslCrackCertificate = (SSL_CRACK_CERTIFICATE_FN)GetProcAddress ( gSchannel, SSL_CRACK_CERTIFICATE_NAME ) ;
					gSslFreeCertificate  = (SSL_FREE_CERTIFICATE_FN )GetProcAddress ( gSchannel, SSL_FREE_CERTIFICATE_NAME ) ;
				}
				else
					hr = AtlReportError ( CLSID_CoPocketHTTP, OLESTR("Failed to dynamically load schannel.dll"), __uuidof(IHttpRequest), HRESULT_FROM_WIN32(GetLastError()) ) ;
			}
			lock.Unlock() ;
		}
		return hr ;
	}

	static int MySslHookFunc (	DWORD dwType,		// in
								LPVOID pvArg,		// in
								DWORD dwChainLen,	// in
								LPBLOB pCertChain,	// in
								DWORD dwFlags )		// in
	{
		if ( SSL_CERT_X509 != dwType )
			return SSL_ERR_CERT_UNKNOWN ;

		if ( SSL_CERT_FLAG_ISSUER_UNKNOWN & dwFlags )
			return SSL_ERR_CERT_UNKNOWN ;

		// we should really use CertCreateCertificateContext / CertGetNameString / CertFreeCertificateContext
		// which is doc'd as CE 3.0, but shock horror, the docs are wrong [again], its only available on
		// PocketPC2202. So we use the undoc'd SslCrackCertificate see this news thread
		// http://groups.google.com/groups?q=SslCrackCertificate&hl=en&lr=&ie=UTF-8&oe=UTF-8&selm=uQf1VcLWBHA.1632%40tkmsftngp05&rnum=3
		int res = SSL_ERR_CERT_UNKNOWN ;
		X509Certificate * cert = 0;
		if ( gSslCrackCertificate( pCertChain->pBlobData, pCertChain->cbSize, TRUE, &cert ) )
		{
			std::string sn = cert->pszSubject ;
			size_t cn = sn.find("CN=") ;
			if ( std::string::npos != cn )
			{
				sn.erase ( 0, cn + 3 ) ;
				size_t end = sn.find_first_of("' \t,") ;
				if ( std::string::npos != end )
					sn.erase ( end ) ;
				if ( pvArg && (_stricmp ( (char *)pvArg, sn.c_str() ) == 0 ))
					res = SSL_ERR_OKAY ;
			} 
			gSslFreeCertificate ( cert ) ;
		}
		return res ;
	}

	HRESULT SetupSSLProxy(T * t, const char * serverName, const stringBuff_A &proxyAuthHeader)
	{
		// we're configured for proxing, and are doing SSL as well,
		// we need to do the inial CONNECT handshake with the proxy
		// to get the SSL tunnel through the proxy setup
		// 
		// we need to send
		// CONNECT enigma.simonathome.com:443 HTTP/1.0
		// User-Agent: PocketSOAP/x.y.z
		// Host: enigma.simonathome.com
		// [Proxy-Authorization: xxxxx]
		//
		stringBuff_A con ( "CONNECT " ) ;
		con.Append ( serverName ) ;
		con.Append ( ":" ) ;
		char buff[10] ;
		_itoa( ntohs(t->port), buff, 10 ) ;
		con.Append ( buff ) ;
		con.Append ( " HTTP/1.1\r\nUser-Agent: " ) ;
		con.Append ( USER_AGENT_STRING ) ;
		con.Append ( "\r\nHost: " ) ;
		con.Append ( serverName ) ;
		con.Append ( "\r\n" ) ;
		if ( proxyAuthHeader.Size() > 0 )
			con.Append ( proxyAuthHeader ) ;
		con.Append ( "\r\n" ) ;

		HRESULT hr = SendChunk ( con.c_str(), con.Size() ) ;
		if (FAILED(hr)) return hr ;

		char szBuff[1024] ;
		DWORD cbBuff = 1024 ;
		stringBuff_A res ;
		int c ;
		do
		{
			c = ReadChunkWithSelect ( szBuff, cbBuff );
			if ( c > 0 )
				res.Append ( szBuff, c ) ;
		} while (( c > 0)  && (strstr ( res.c_str(), "\r\n\r\n" ) == 0 )) ;

		char * p200 = strstr ( res.c_str(), "200" )  ;
		char * pEOL = strstr ( res.c_str(), "\r\n" ) ;

		if ( p200 && pEOL && ( p200 < pEOL ))
			return S_OK ;
	
		// find the status code
		char * status = strchr (res.c_str(), ' ') ;
		int statusCode = -1;
		WCHAR wError[100] = L"Error establishing SSL connection via proxy";
		if ( status )
		{
			statusCode = atoi(++status) ;
			swprintf(wError, L"Error establishing SSL connection via proxy (HTTP StatusCode is %d)", statusCode);
		}
		return AtlReportError ( CLSID_CoPocketHTTP, wError, IID_NULL, E_PROXY_CONNECT_FAILED ) ;
	}

	long	m_timeout ;
} ;