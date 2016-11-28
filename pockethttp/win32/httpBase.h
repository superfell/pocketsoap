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
Portions created by Simon Fell are Copyright (C) 2000-2005
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#pragma once
#pragma comment(lib,"wsock32.lib")
#pragma comment(lib,"delayimp.lib")
#pragma comment(lib,"crypt32.lib")
#pragma comment(linker,"/DELAYLOAD:CRYPT32.dll")

#include <wincrypt.h>
#include <wintrust.h>
#include <schannel.h>

#define SECURITY_WIN32
#include <security.h>
#include "stringBuff.h"

#define DLL_NAME TEXT("Secur32.dll")
#define NT4_DLL_NAME TEXT("Security.dll")
#define IO_BUFFER_SIZE  0x10000

extern const char * const USER_AGENT_STRING ;
class ConnectionPool ;

extern ConnectionPool thePool ;

// see http://support.microsoft.com/default.aspx?kbid=300562
static const SSL_BUF_SIZE_FIX = 5 ;

template<class T>
class httpTransportBase
{
private:
	httpTransportBase(const httpTransportBase &rhs) ;
	httpTransportBase & operator = ( const httpTransportBase &rhs) ;

public:
	httpTransportBase() : m_sendBuff(0), m_cbRead(0), m_T(0), m_sslActive(false)
	{
	}

	~httpTransportBase()
	{
		if ( m_sendBuff )
			delete [] m_sendBuff ;
	}

	static void ObjectMain(bool bStarting)
	{
		if ( ! bStarting )
		{
			if ( m_hMyCertStore )
				CertCloseStore ( m_hMyCertStore, 0 ) ;

			UnloadSecurityLibrary() ;
		}
	}

	HRESULT PreConnect(const char * serverName, long timeout)
	{
		HRESULT hr = S_OK ;
		getUpcast() ;
		setTimeout(timeout) ;

		if ( m_T->ssl )
		{
			hr = InitSSL() ;
			if (FAILED(hr)) return hr ;
			CreateCredentials( NULL, &m_CredHandle ) ;
		}

		return hr ;
	}

	HRESULT PostConnect(const char * serverName, const stringBuff_A &proxyAuthHeader)
	{
		getUpcast() ;
		if ( m_T->ssl )
		{    
			// check for SSL w/ Proxy
			if ( m_T->px_port != 0 )
			{
				HRESULT hr = SetupSSLProxy(serverName, proxyAuthHeader) ;
				if (FAILED(hr)) return hr ;
			}

			SECURITY_STATUS Status = PerformClientHandshake(m_T->socket, &m_CredHandle, serverName, &m_hContext, &m_ExtraData) ;
			if(Status != SEC_E_OK)
			{
				ATLTRACE("Error 0x%x during PerformClientHandshake\n", Status);
				return Status ;
			}
			m_sslActive = true ;
		    // Get server's certificate.
			PCCERT_CONTEXT pRemoteCertContext = NULL;
			Status = m_pSecurityFunc->QueryContextAttributesA(&m_hContext, SECPKG_ATTR_REMOTE_CERT_CONTEXT, (PVOID)&pRemoteCertContext);
			if(Status != SEC_E_OK)
			{
				ATLTRACE("Error 0x%x querying remote certificate\n", Status);
				return Status;
			}
			
			// Attempt to validate server certificate.
			Status = VerifyServerCertificate(pRemoteCertContext, serverName, 0);
			CertFreeCertificateContext ( pRemoteCertContext ) ;
			if ( Status )
			{
				ATLTRACE(_T("VerifyServerCertificate failed 0x%x\n"), Status) ;
				return Status ;
			}
		    Status = m_pSecurityFunc->QueryContextAttributesA(&m_hContext, SECPKG_ATTR_STREAM_SIZES, &m_Sizes);
			if(Status != SEC_E_OK)
			{
				ATLTRACE("Error 0x%x reading SECPKG_ATTR_STREAM_SIZES\n", Status);
				return Status;
			}
			
			m_sendBuff = new char[m_Sizes.cbHeader + m_Sizes.cbTrailer + m_Sizes.cbMaximumMessage + SSL_BUF_SIZE_FIX] ;
		}
		return S_OK ;
	}

	HRESULT setTimeout(long timeout)
	{
		m_timeout = timeout;
		return S_OK ;
	}

	int SSL_write ( SOCKET s, const char * data, unsigned long size) 
	{
		SecBufferDesc   Message;
		SecBuffer       Buffers[4];

	    // Build the HTTP request offset into the data buffer by "header size"
	    // bytes. This enables Schannel to perform the encryption in place,
		// which is a significant performance win.
		char * pData = m_sendBuff + m_Sizes.cbHeader ;
		memcpy ( pData, data, size ) ;

		// Encrypt the HTTP request.
		//
	    Buffers[0].pvBuffer     = m_sendBuff;
		Buffers[0].cbBuffer     = m_Sizes.cbHeader;
		Buffers[0].BufferType   = SECBUFFER_STREAM_HEADER;

		Buffers[1].pvBuffer     = pData;
		Buffers[1].cbBuffer     = size;
		Buffers[1].BufferType   = SECBUFFER_DATA;

		Buffers[2].pvBuffer     = pData + size;
		Buffers[2].cbBuffer     = m_Sizes.cbTrailer;
		Buffers[2].BufferType   = SECBUFFER_STREAM_TRAILER;

		Buffers[3].BufferType   = SECBUFFER_EMPTY;

		Message.ulVersion       = SECBUFFER_VERSION;
		Message.cBuffers        = 4;
		Message.pBuffers        = Buffers;

		SECURITY_STATUS scRet = m_pSecurityFunc->EncryptMessage(&m_hContext, 0, &Message, 0);
		if(FAILED(scRet))
		{
			ATLTRACE("**** Error 0x%x returned by EncryptMessage\n", scRet);
			return SOCKET_ERROR;
		}
		int len = Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer ;
		ATLTRACE(_T("Send %d bytes via SSL\n"), len ) ;

		int rv = send(s, m_sendBuff, len, 0);
		if (( SOCKET_ERROR == rv ) || ( len != rv ))
			ATLTRACE(_T("error in send %d\n"), WSAGetLastError() ) ;

		return rv ;
	}

	// this sends a chunk of data (the socket must already be connected)
	// and correctly checks for errors
	HRESULT SendChunk(const char * data, unsigned long size)
	{
		getUpcast() ;
		HRESULT hr = S_OK ;
		long cbChunk ;
		size_t maxChunk = m_T->ssl ? m_Sizes.cbMaximumMessage : 32767 ;
		int retval ;
		// send the request, we need break it up into chunks for SSL
		do
		{
			cbChunk = min ( size, maxChunk ) ;

			if ( m_T->ssl )
				retval = SSL_write ( m_T->socket, data, cbChunk ) ;
			else		
				retval = send(m_T->socket, data , cbChunk, 0);

			if (SOCKET_ERROR == retval)
			{
				DWORD wsaErr = WSAGetLastError() ;
				Disconnect();
				hr = HRESULT_FROM_WIN32(wsaErr) ;
				if ( WSAETIMEDOUT == wsaErr )
					return AtlReportError ( CLSID_CoPocketHTTP, OLESTR("Timeout sending to destination"), IID_NULL, hr ) ;
				else
					return AtlReportError ( CLSID_CoPocketHTTP, OLESTR("Failing sending data to server"), IID_NULL, hr ) ;
				return hr ;
			}
			size -= cbChunk;
			data += cbChunk;

		} while ( size > 0 ) ;

		return hr ;
	}

	int SSL_read ( SOCKET s, char * buff, long cbretBuff )
	{
		if ( m_readBuffer.size() > 0 )
		{
			long cb = min(cbretBuff, (long)m_readBuffer.size() ) ;
			memcpy ( buff, m_readBuffer.begin(), cb ) ;
			m_readBuffer.erase ( m_readBuffer.begin(), m_readBuffer.begin() + cb ) ;
			return cb ;
		}
		int retval =0, rawcb = 0 ;
		SecBufferDesc   Message;
		SecBuffer       Buffers[4];
		SecBuffer * pDataBuffer  = NULL;
		SecBuffer * pExtraBuffer = NULL;
		SECURITY_STATUS scRet = S_OK ;
		unsigned long cbBuff = m_Sizes.cbMaximumMessage + m_Sizes.cbHeader + m_Sizes.cbTrailer + SSL_BUF_SIZE_FIX ;
		if ( m_ExtraData.cbBuffer )
		{
			memcpy ( m_sendBuff, m_ExtraData.pvBuffer, m_ExtraData.cbBuffer ) ;
			m_cbRead = m_ExtraData.cbBuffer ;
			LocalFree ( m_ExtraData.pvBuffer ) ;
			m_ExtraData.pvBuffer = 0 ;
			m_ExtraData.cbBuffer = 0 ;
		}
		while ( TRUE )
		{
			if ( 0 == m_cbRead || scRet == SEC_E_INCOMPLETE_MESSAGE )
			{
				rawcb = recv ( s, m_sendBuff + m_cbRead, cbBuff - m_cbRead, 0 ) ;
				if ( SOCKET_ERROR == rawcb || 0 == rawcb ) 
					break ;
			
				m_cbRead += rawcb ;
			}
			// Attempt to decrypt the received data.
			//
			Buffers[0].pvBuffer     = m_sendBuff;
			Buffers[0].cbBuffer     = m_cbRead;
			Buffers[0].BufferType   = SECBUFFER_DATA;

			Buffers[1].BufferType   = SECBUFFER_EMPTY;
			Buffers[2].BufferType   = SECBUFFER_EMPTY;
			Buffers[3].BufferType   = SECBUFFER_EMPTY;

			Message.ulVersion       = SECBUFFER_VERSION;
			Message.cBuffers        = 4;
			Message.pBuffers        = Buffers;

			scRet = m_pSecurityFunc->DecryptMessage(&m_hContext, &Message, 0, NULL);
	        if(scRet == SEC_E_INCOMPLETE_MESSAGE)
		    {
			    // The input buffer contains only a fragment of an
				// encrypted record. Loop around and read some more
				// data.
				continue;
			}

			// Locate data and (optional) extra buffers.
			pDataBuffer  = NULL;
			pExtraBuffer = NULL;
			for(int i = 1; i < 4; i++)
			{
				if(pDataBuffer == NULL && Buffers[i].BufferType == SECBUFFER_DATA)
				{
					pDataBuffer = &Buffers[i];
					ATLTRACE("Buffers[%d].BufferType = SECBUFFER_DATA\n",i);
				}
				if(pExtraBuffer == NULL && Buffers[i].BufferType == SECBUFFER_EXTRA)
				{
					pExtraBuffer = &Buffers[i];
				}
			}
			if ( pDataBuffer )
			{
				retval = min( cbretBuff, (long)pDataBuffer->cbBuffer) ;
				memcpy ( buff, pDataBuffer->pvBuffer, retval ) ;
				if ( (long)pDataBuffer->cbBuffer > cbretBuff )
				{
					m_readBuffer.insert ( m_readBuffer.end(), ((BYTE *)pDataBuffer->pvBuffer) + cbretBuff, ((BYTE *)pDataBuffer->pvBuffer) + pDataBuffer->cbBuffer ) ;
				}
			}
			// Move any "extra" data to the input buffer.
			if(pExtraBuffer)
			{
				MoveMemory(m_sendBuff, pExtraBuffer->pvBuffer, pExtraBuffer->cbBuffer);
				m_cbRead = pExtraBuffer->cbBuffer;
				
				/* Citing MSDN:
				 * Sometimes an application will read data from the remote party, attempt 
				 * to decrypt it by using DecryptMessage (Schannel), and discover that 
				 * DecryptMessage (Schannel) succeeded but the output buffers are empty. 
				 * This is normal behavior, and applications must be able to deal with it.
				 */
				if (pDataBuffer && pDataBuffer->cbBuffer == 0) continue;
			}
			else
			{
				m_cbRead = 0;
			}
			break ;
        }
		return retval ;
	}

	int ReadChunk ( char *szBuff, size_t cbBuff )
	{
		getUpcast() ;
		if ( m_T->ssl )
			return SSL_read ( m_T->socket, szBuff, cbBuff );                
		else
			return recv ( m_T->socket, szBuff, cbBuff, 0 );
	}

	void Disconnect()
	{
		ATLTRACE(_T("httpBase::Disconnect\n")) ;
		getUpcast() ;
		if ( m_sslActive )
		{
			DisconnectFromServer ( m_T->socket, &m_CredHandle, &m_hContext ) ;
			m_pSecurityFunc->FreeCredentialsHandle(&m_CredHandle);
		}  
		if ( m_T->socket != INVALID_SOCKET )
		{
			shutdown ( m_T->socket, SD_BOTH ) ;
			closesocket ( m_T->socket ) ;
			m_T->socket = INVALID_SOCKET ;
			thePool.destroyConnection(m_T) ;
		}
	}

private:
	static	bool					m_SslInitDone ;
	static CComAutoCriticalSection	m_SslInitLock ;
	static HINSTANCE				m_hSecurity ;
	static SecurityFunctionTableA  *m_pSecurityFunc;
	static HCERTSTORE				m_hMyCertStore ;

	CredHandle						m_CredHandle ;
    CtxtHandle						m_hContext;
    SecBuffer						m_ExtraData;
	SecPkgContext_StreamSizes		m_Sizes;
	bool							m_sslActive ;
	char						   *m_sendBuff ;
	unsigned long					m_cbRead  ;
	T *								m_T ;
	std::vector<BYTE>				m_readBuffer ;
	long							m_timeout;

	inline void getUpcast()
	{
		if ( ! m_T )
			m_T = static_cast<T *>(this) ;
	}

	int recv(SOCKET s, char * buffer, int len, int flags)
	{
		fd_set readfds[1] ;
		FD_ZERO(readfds) ;
		FD_SET ( s, readfds ) ;
		timeval timeout ;
		timeout.tv_sec = m_timeout / 1000 ;
		timeout.tv_usec = ( m_timeout % 1000 ) * 1000 ;

		int rc = select ( 0, readfds, NULL, NULL, &timeout) ;
		if ( 0 == rc || ( ! FD_ISSET ( s, readfds ) ) )
		{
			// timeout
			WSASetLastError ( WSAETIMEDOUT ) ;
			return SOCKET_ERROR ;
		}
		if ( SOCKET_ERROR == rc )
			return rc ;

		return ::recv(s, buffer, len, flags);
	}

	int send(SOCKET s, const char * buffer, int len, int flags)
	{
		fd_set writefds[1] ;
		FD_ZERO(writefds) ;
		FD_SET ( s, writefds ) ;
		timeval timeout ;
		timeout.tv_sec = m_timeout / 1000 ;
		timeout.tv_usec = ( m_timeout % 1000 ) * 1000 ;

		int rc = select ( 0, NULL, writefds, NULL, &timeout) ;
		if ( 0 == rc || ( ! FD_ISSET ( s, writefds ) ) )
		{
			// timeout
			WSASetLastError ( WSAETIMEDOUT ) ;
			return SOCKET_ERROR ;
		}
		if ( SOCKET_ERROR == rc )
			return rc ;

		return ::send(s, buffer, len, flags);
	}

	HRESULT SendBuffer ( const char * data, unsigned long size )
	{
		int retval = send(m_T->socket, data, size, 0);
		if (SOCKET_ERROR == retval)
		{
			DWORD wsaErr = WSAGetLastError() ;
			Disconnect();
			HRESULT hr = HRESULT_FROM_WIN32(wsaErr) ;
			if ( WSAETIMEDOUT == wsaErr )
				AtlReportError ( CLSID_CoPocketHTTP, OLESTR("Timeout sending to destination"), IID_NULL, hr ) ;
			else
				AtlReportError ( CLSID_CoPocketHTTP, OLESTR("Failing sending data to server"), IID_NULL, hr ) ;
		}
		return S_OK ;
	}

	HRESULT SetupSSLProxy(const char * serverName, const stringBuff_A &proxyAuthHeader)
	{
		// todo: this code is common to both win32 & pocketPC and should be factored out
		// into a single implementation

		// we're configured for proxing, and are doing SSL as well,
		// we need to do the inial CONNECT handshake with the proxy
		// to get the SSL tunnel through the proxy setup
		// 
		// we need to send
		// CONNECT enigma.simonathome.com:443 HTTP/1.1
		// User-Agent: PocketSOAP/1.3.0
		// Host: enigma.simonathome.com
		// [Proxy-Authorization: xxxxx]
		//
		stringBuff_A con ( "CONNECT " ) ;
		con.Append ( serverName ) ;
		con.Append ( ":" ) ;
		char buff[10] ;
		itoa( ntohs(m_T->port), buff, 10 ) ;
		con.Append ( buff ) ;
		con.Append ( " HTTP/1.1\r\nUser-Agent: " ) ;
		con.Append ( USER_AGENT_STRING ) ;
		con.Append ( "\r\nHost: " ) ;
		con.Append ( serverName ) ;
		con.Append ( "\r\n" ) ;
		if ( proxyAuthHeader.Size() > 0 )
			con.Append ( proxyAuthHeader ) ;
		con.Append ( "\r\n" ) ;

		HRESULT hr = SendBuffer ( con.c_str(), con.Size() ) ;
		if (FAILED(hr)) return hr ;

		char szBuff[1024] ;
		DWORD cbBuff = 1024 ;
		stringBuff_A res ;
		int c ;
		do
		{
			c = recv ( m_T->socket, szBuff, cbBuff, 0 );
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

	HRESULT InitSSL(void)
	{
		HRESULT hr = S_OK ;
		if ( ! m_SslInitDone )
		{
			m_SslInitLock.Lock() ;
			if ( ! m_SslInitDone )
			{
				hr = LoadSecurityLibrary() ;
				if (SUCCEEDED(hr))
				{		
					// Open the "MY" certificate store, which is where Internet Explorer
					// stores its client certificates.
					m_hMyCertStore = CertOpenSystemStore(0, _T("MY"));
					if(!m_hMyCertStore)
						hr = AtlReportError ( CLSID_CoPocketHTTP, OLESTR("Error opening CertificateStore"), IID_NULL, HRESULT_FROM_WIN32(GetLastError()) ) ;
					else
						m_SslInitDone = true ;
				}
			}
			m_SslInitLock.Unlock() ;
		}
		return hr ;
	}

	static HRESULT LoadSecurityLibrary(void)
	{
		
		INIT_SECURITY_INTERFACE_A   pInitSecurityInterface;
		OSVERSIONINFO				VerInfo;
		TCHAR lpszDLL[MAX_PATH];

	    //
	    //  Find out which security DLL to use, depending on
	    //  whether we are on Win2K, NT or Win9x
		VerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (!GetVersionEx (&VerInfo))   
	        return HRESULT_FROM_WIN32(GetLastError()) ;

		if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && VerInfo.dwMajorVersion == 4)
	    {
		    _tcscpy (lpszDLL, NT4_DLL_NAME );
		}
		else if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS || VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
		{
			_tcscpy (lpszDLL, DLL_NAME );
		}
		else
	    {
			return E_UNEXPECTED;
		}

		//
		//  Load Security DLL
		//

		m_hSecurity = LoadLibrary(lpszDLL);
		if(NULL == m_hSecurity )
		{
			ATLTRACE("Error 0x%x loading %s.\n", GetLastError(), lpszDLL);
			return HRESULT_FROM_WIN32(GetLastError()) ;
		}

		pInitSecurityInterface = (INIT_SECURITY_INTERFACE_A)GetProcAddress(m_hSecurity, "InitSecurityInterfaceA");
	    if(pInitSecurityInterface == NULL)
		{
			ATLTRACE("Error 0x%x reading InitSecurityInterface entry point.\n", GetLastError());
			return HRESULT_FROM_WIN32(GetLastError()) ;
		}

		m_pSecurityFunc = pInitSecurityInterface();
	    if(m_pSecurityFunc == NULL)
		{
			ATLTRACE("Error 0x%x reading security interface.\n", GetLastError());
			return HRESULT_FROM_WIN32(GetLastError()) ;
		}

		return S_OK;
	}

	static void UnloadSecurityLibrary(void)
	{
		if ( m_hSecurity )
		{
			FreeLibrary(m_hSecurity);
			m_hSecurity = NULL;
		}
	}

	/*****************************************************************************/
	static SECURITY_STATUS CreateCredentials(
			LPSTR pszUserName,					// in
			CredHandle * phCreds)				// out
	{
		TimeStamp       tsExpiry;
		SECURITY_STATUS Status;

		DWORD           cSupportedAlgs = 0;
		ALG_ID          rgbSupportedAlgs[16];

		PCCERT_CONTEXT  pCertContext = NULL;

		//
		// If a user name is specified, then attempt to find a client
		// certificate. Otherwise, just create a NULL credential.

		// the calling code always passes NULL for now, we'll loop back later, and 
		// address client certs
		if(pszUserName)
		{
			// Find client certificate. Note that this sample just searchs for a 
			// certificate that contains the user name somewhere in the subject name.
			// A real application should be a bit less casual.
			pCertContext = CertFindCertificateInStore(m_hMyCertStore, 
													  X509_ASN_ENCODING, 
													  0,
													  CERT_FIND_SUBJECT_STR_A,
													  pszUserName,
													  NULL);
			if(pCertContext == NULL)
			{
				ATLTRACE("**** Error 0x%x returned by CertFindCertificateInStore\n", GetLastError());
				return SEC_E_NO_CREDENTIALS;
			}
		}


		//
		// Build Schannel credential structure. Currently, this sample only
		// specifies the protocol to be used (and optionally the certificate, 
		// of course). Real applications may wish to specify other parameters 
		// as well.
		//
		SCHANNEL_CRED   SchannelCred;
		ZeroMemory(&SchannelCred, sizeof(SchannelCred));

		SchannelCred.dwVersion  = SCHANNEL_CRED_VERSION;
		if(pCertContext)
		{
			SchannelCred.cCreds     = 1;
			SchannelCred.paCred     = &pCertContext;
		}

		static const DWORD   dwProtocol      = 0;
		SchannelCred.grbitEnabledProtocols = dwProtocol;

		static const ALG_ID  aiKeyExch       = 0;
		if(aiKeyExch)
		{
			rgbSupportedAlgs[cSupportedAlgs++] = aiKeyExch;
		}

		if(cSupportedAlgs)
		{
			SchannelCred.cSupportedAlgs    = cSupportedAlgs;
			SchannelCred.palgSupportedAlgs = rgbSupportedAlgs;
		}

		SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;
		SchannelCred.dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;


		//
		// Create an SSPI credential.
		//

		Status = m_pSecurityFunc->AcquireCredentialsHandleA(
							NULL,                   // Name of principal    
							UNISP_NAME_A,           // Name of package
							SECPKG_CRED_OUTBOUND,   // Flags indicating use
							NULL,                   // Pointer to logon ID
							&SchannelCred,          // Package specific data
							NULL,                   // Pointer to GetKey() func
							NULL,                   // Value to pass to GetKey()
							phCreds,                // (out) Cred Handle
							&tsExpiry);             // (out) Lifetime (optional)
		if(Status != SEC_E_OK)
		{
			ATLTRACE("**** Error 0x%x returned by AcquireCredentialsHandle\n", Status);
			return Status;
		}


		//
		// Free the certificate context. Schannel has already made its own copy.
		//

		if(pCertContext)
		{
			CertFreeCertificateContext(pCertContext);
		}


		return SEC_E_OK;
	}

	SECURITY_STATUS PerformClientHandshake(
							SOCKET          Socket,         // in
							CredHandle     *phCreds,        // in
							LPCSTR          pszServerName,  // in
							CtxtHandle *    phContext,      // out
							SecBuffer *     pExtraData)     // out
	{
		SecBufferDesc   OutBuffer;
		SecBuffer       OutBuffers[1];
		DWORD           dwSSPIFlags;
		DWORD           dwSSPIOutFlags;
		TimeStamp       tsExpiry;
		SECURITY_STATUS scRet;
		DWORD           cbData;

		dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
					  ISC_REQ_REPLAY_DETECT     |
					  ISC_REQ_CONFIDENTIALITY   |
					  ISC_RET_EXTENDED_ERROR    |
					  ISC_REQ_ALLOCATE_MEMORY   |
					  ISC_REQ_STREAM;

		//
		//  Initiate a ClientHello message and generate a token.
		//

		OutBuffers[0].pvBuffer   = NULL;
		OutBuffers[0].BufferType = SECBUFFER_TOKEN;
		OutBuffers[0].cbBuffer   = 0;

		OutBuffer.cBuffers = 1;
		OutBuffer.pBuffers = OutBuffers;
		OutBuffer.ulVersion = SECBUFFER_VERSION;

		scRet = m_pSecurityFunc->InitializeSecurityContextA(
						phCreds,
						NULL,
						const_cast<char *>(pszServerName),
						dwSSPIFlags,
						0,
						SECURITY_NATIVE_DREP,
						NULL,
						0,
						phContext,
						&OutBuffer,
						&dwSSPIOutFlags,
						&tsExpiry);

		if(scRet != SEC_I_CONTINUE_NEEDED)
		{
			ATLTRACE("**** Error %d returned by InitializeSecurityContext (1)\n", scRet);
			return scRet;
		}

		// Send response to server if there is one.
		if(OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL)
		{
			cbData = send(Socket, (const char *)OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer, 0);
			// Free output buffer.
			m_pSecurityFunc->FreeContextBuffer(OutBuffers[0].pvBuffer);
			OutBuffers[0].pvBuffer = NULL;

			if(cbData == SOCKET_ERROR || cbData == 0)
			{
				ATLTRACE("**** Error %d sending data to server (1)\n", WSAGetLastError());
				m_pSecurityFunc->DeleteSecurityContext(phContext);
				return SEC_E_INTERNAL_ERROR;
			}
			ATLTRACE("%d bytes of handshake data sent\n", cbData);

		}


		return ClientHandshakeLoop(Socket, phCreds, phContext, TRUE, pExtraData);
	}

	/*****************************************************************************/
	SECURITY_STATUS ClientHandshakeLoop(
								SOCKET          Socket,         // in
								PCredHandle     phCreds,        // in
								CtxtHandle *    phContext,      // in, out
								BOOL            fDoInitialRead, // in
								SecBuffer *     pExtraData)     // out
	{
		SecBufferDesc   InBuffer;
		SecBuffer       InBuffers[2];
		SecBufferDesc   OutBuffer;
		SecBuffer       OutBuffers[1];
		DWORD           dwSSPIFlags;
		DWORD           dwSSPIOutFlags;
		TimeStamp       tsExpiry;
		SECURITY_STATUS scRet;
		DWORD           cbData;

		PUCHAR          IoBuffer;
		DWORD           cbIoBuffer;
		BOOL            fDoRead;


		dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
					  ISC_REQ_REPLAY_DETECT     |
					  ISC_REQ_CONFIDENTIALITY   |
					  ISC_RET_EXTENDED_ERROR    |
					  ISC_REQ_ALLOCATE_MEMORY   |
					  ISC_REQ_STREAM;

		//
		// Allocate data buffer.
		//

		IoBuffer = new BYTE[IO_BUFFER_SIZE];

		if(IoBuffer == NULL)
		{
			ATLTRACE("**** Out of memory (1)\n");
			return SEC_E_INTERNAL_ERROR;
		}
		cbIoBuffer = 0;

		fDoRead = fDoInitialRead;


		// 
		// Loop until the handshake is finished or an error occurs.
		//

		scRet = SEC_I_CONTINUE_NEEDED;

		while(scRet == SEC_I_CONTINUE_NEEDED        ||
			  scRet == SEC_E_INCOMPLETE_MESSAGE     ||
			  scRet == SEC_I_INCOMPLETE_CREDENTIALS) 
	   {

			//
			// Read data from server.
			//

			if(0 == cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE)
			{
				if(fDoRead)
				{
					cbData = recv(Socket, (char *)IoBuffer + cbIoBuffer, IO_BUFFER_SIZE - cbIoBuffer, 0);
					if(cbData == SOCKET_ERROR)
					{
						ATLTRACE("**** Error %d reading data from server\n", WSAGetLastError());
						scRet = SEC_E_INTERNAL_ERROR;
						break;
					}
					else if(cbData == 0)
					{
						ATLTRACE("**** Server unexpectedly disconnected\n");
						scRet = SEC_E_INTERNAL_ERROR;
						break;
					}

					ATLTRACE("%d bytes of handshake data received\n", cbData);

					cbIoBuffer += cbData;
				}
				else
				{
					fDoRead = TRUE;
				}
			}


			//
			// Set up the input buffers. Buffer 0 is used to pass in data
			// received from the server. Schannel will consume some or all
			// of this. Leftover data (if any) will be placed in buffer 1 and
			// given a buffer type of SECBUFFER_EXTRA.
			//

			InBuffers[0].pvBuffer   = IoBuffer;
			InBuffers[0].cbBuffer   = cbIoBuffer;
			InBuffers[0].BufferType = SECBUFFER_TOKEN;

			InBuffers[1].pvBuffer   = NULL;
			InBuffers[1].cbBuffer   = 0;
			InBuffers[1].BufferType = SECBUFFER_EMPTY;

			InBuffer.cBuffers       = 2;
			InBuffer.pBuffers       = InBuffers;
			InBuffer.ulVersion      = SECBUFFER_VERSION;

			//
			// Set up the output buffers. These are initialized to NULL
			// so as to make it less likely we'll attempt to free random
			// garbage later.
			//

			OutBuffers[0].pvBuffer  = NULL;
			OutBuffers[0].BufferType= SECBUFFER_TOKEN;
			OutBuffers[0].cbBuffer  = 0;

			OutBuffer.cBuffers      = 1;
			OutBuffer.pBuffers      = OutBuffers;
			OutBuffer.ulVersion     = SECBUFFER_VERSION;

			//
			// Call InitializeSecurityContext.
			//

			scRet = m_pSecurityFunc->InitializeSecurityContextA(phCreds,
											  phContext,
											  NULL,
											  dwSSPIFlags,
											  0,
											  SECURITY_NATIVE_DREP,
											  &InBuffer,
											  0,
											  NULL,
											  &OutBuffer,
											  &dwSSPIOutFlags,
											  &tsExpiry);

			//
			// If InitializeSecurityContext was successful (or if the error was 
			// one of the special extended ones), send the contends of the output
			// buffer to the server.
			//

			if(scRet == SEC_E_OK                ||
			   scRet == SEC_I_CONTINUE_NEEDED   ||
			   FAILED(scRet) && (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR))
			{
				if(OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL)
				{
					cbData = send(Socket, (const char *)OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer, 0);
					if(cbData == SOCKET_ERROR || cbData == 0)
					{
						ATLTRACE("**** Error %d sending data to server (2)\n", WSAGetLastError());
						m_pSecurityFunc->FreeContextBuffer(OutBuffers[0].pvBuffer);
						m_pSecurityFunc->DeleteSecurityContext(phContext);
						delete [] IoBuffer ;
						return SEC_E_INTERNAL_ERROR;
					}

					ATLTRACE("%d bytes of handshake data sent\n", cbData);

					// Free output buffer.
					m_pSecurityFunc->FreeContextBuffer(OutBuffers[0].pvBuffer);
					OutBuffers[0].pvBuffer = NULL;
				}
			}


			//
			// If InitializeSecurityContext returned SEC_E_INCOMPLETE_MESSAGE,
			// then we need to read more data from the server and try again.
			//

			if(scRet == SEC_E_INCOMPLETE_MESSAGE)
			{
				continue;
			}


			//
			// If InitializeSecurityContext returned SEC_E_OK, then the 
			// handshake completed successfully.
			//

			if(scRet == SEC_E_OK)
			{
				//
				// If the "extra" buffer contains data, this is encrypted application
				// protocol layer stuff. It needs to be saved. The application layer
				// will later decrypt it with DecryptMessage.
				//

				ATLTRACE("Handshake was successful\n");

				if(InBuffers[1].BufferType == SECBUFFER_EXTRA)
				{
					pExtraData->pvBuffer = LocalAlloc(LMEM_FIXED,  InBuffers[1].cbBuffer);
					if(pExtraData->pvBuffer == NULL)
					{
						ATLTRACE("**** Out of memory (2)\n");
						delete [] IoBuffer ;
						return SEC_E_INTERNAL_ERROR;
					}

					MoveMemory(pExtraData->pvBuffer,
							   IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),
							   InBuffers[1].cbBuffer);

					pExtraData->cbBuffer   = InBuffers[1].cbBuffer;
					pExtraData->BufferType = SECBUFFER_TOKEN;

					ATLTRACE("%d bytes of app data was bundled with handshake data\n", pExtraData->cbBuffer);
				}
				else
				{
					pExtraData->pvBuffer   = NULL;
					pExtraData->cbBuffer   = 0;
					pExtraData->BufferType = SECBUFFER_EMPTY;
				}

				//
				// Bail out to quit
				//

				break;
			}


			//
			// Check for fatal error.
			//

			if(FAILED(scRet))
			{
				ATLTRACE("**** Error 0x%x returned by InitializeSecurityContext (2)\n", scRet);
				break;
			}


			//
			// If InitializeSecurityContext returned SEC_I_INCOMPLETE_CREDENTIALS,
			// then the server just requested client authentication. 
			//

			if(scRet == SEC_I_INCOMPLETE_CREDENTIALS)
			{
				//
				// Display trusted issuers info. 
				//

				GetNewClientCredentials(phCreds, phContext);


				//
				// Now would be a good time perhaps to prompt the user to select
				// a client certificate and obtain a new credential handle, 
				// but I don't have the energy nor inclination.
				//
				// As this is currently written, Schannel will send a "no 
				// certificate" alert to the server in place of a certificate. 
				// The server might be cool with this, or it might drop the 
				// connection.
				// 

				// Go around again.
				fDoRead = FALSE;
				scRet = SEC_I_CONTINUE_NEEDED;
				continue;
			}


			//
			// Copy any leftover data from the "extra" buffer, and go around
			// again.
			//

			if ( InBuffers[1].BufferType == SECBUFFER_EXTRA )
			{
				MoveMemory(IoBuffer,
						   IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),
						   InBuffers[1].cbBuffer);

				cbIoBuffer = InBuffers[1].cbBuffer;
			}
			else
			{
				cbIoBuffer = 0;
			}
		}

		// Delete the security context in the case of a fatal error.
		if(FAILED(scRet))
		{
			m_pSecurityFunc->DeleteSecurityContext(phContext);
		}

		delete [] IoBuffer;

		return scRet;
	}


	/*****************************************************************************/
	static void GetNewClientCredentials(
								CredHandle *phCreds,
								CtxtHandle *phContext)
	{
		CredHandle hCreds;
		SecPkgContext_IssuerListInfoEx IssuerListInfo;
		PCCERT_CHAIN_CONTEXT pChainContext;
		CERT_CHAIN_FIND_BY_ISSUER_PARA FindByIssuerPara;
		PCCERT_CONTEXT  pCertContext;
		TimeStamp       tsExpiry;
		SECURITY_STATUS Status;

		//
		// Read list of trusted issuers from schannel.
		//

		Status = m_pSecurityFunc->QueryContextAttributesA (phContext, SECPKG_ATTR_ISSUER_LIST_EX, (PVOID)&IssuerListInfo);
		if(Status != SEC_E_OK)
		{
			ATLTRACE("Error 0x%x querying issuer list info\n", Status);
			return;
		}

		//
		// Enumerate the client certificates.
		//

		ZeroMemory(&FindByIssuerPara, sizeof(FindByIssuerPara));

		FindByIssuerPara.cbSize = sizeof(FindByIssuerPara);
		FindByIssuerPara.pszUsageIdentifier = szOID_PKIX_KP_CLIENT_AUTH;
		FindByIssuerPara.dwKeySpec = 0;
		FindByIssuerPara.cIssuer   = IssuerListInfo.cIssuers;
		FindByIssuerPara.rgIssuer  = IssuerListInfo.aIssuers;

		pChainContext = NULL;

		while(TRUE)
		{
			// Find a certificate chain.
			pChainContext = CertFindChainInStore(m_hMyCertStore,
												 X509_ASN_ENCODING,
												 0,
												 CERT_CHAIN_FIND_BY_ISSUER,
												 &FindByIssuerPara,
												 pChainContext);
			if(pChainContext == NULL)
			{
				ATLTRACE("Error 0x%x finding cert chain\n", GetLastError());
				break;
			}
			ATLTRACE("\ncertificate chain found\n");

			// Get pointer to leaf certificate context.
			pCertContext = pChainContext->rgpChain[0]->rgpElement[0]->pCertContext;

			// Create schannel credential.
			SCHANNEL_CRED SchannelCred ;
			SchannelCred.cCreds = 1;
			SchannelCred.paCred = &pCertContext;

			Status = m_pSecurityFunc->AcquireCredentialsHandleA(
								NULL,                   // Name of principal
								UNISP_NAME_A,           // Name of package
								SECPKG_CRED_OUTBOUND,   // Flags indicating use
								NULL,                   // Pointer to logon ID
								&SchannelCred,          // Package specific data
								NULL,                   // Pointer to GetKey() func
								NULL,                   // Value to pass to GetKey()
								&hCreds,                // (out) Cred Handle
								&tsExpiry);             // (out) Lifetime (optional)
			if(Status != SEC_E_OK)
			{
				ATLTRACE("**** Error 0x%x returned by AcquireCredentialsHandle\n", Status);
				continue;
			}
			ATLTRACE("\nnew schannel credential created\n");

			// Destroy the old credentials.
			m_pSecurityFunc->FreeCredentialsHandle(phCreds);

			*phCreds = hCreds;

			break;
		}
	}
	/*****************************************************************************/
	static  DWORD VerifyServerCertificate(
											PCCERT_CONTEXT  pServerCert,
											LPCSTR          pszServerName,
											DWORD           dwCertFlags)
	{
		HTTPSPolicyCallbackData  polHttps;
		CERT_CHAIN_POLICY_PARA   PolicyPara;
		CERT_CHAIN_POLICY_STATUS PolicyStatus;
		CERT_CHAIN_PARA          ChainPara;
		PCCERT_CHAIN_CONTEXT     pChainContext = NULL;

		DWORD   Status;
		PWSTR   pwszServerName;

		if(pServerCert == NULL)
			return SEC_E_WRONG_PRINCIPAL;

		//
		// Convert server name to unicode.
		if(pszServerName == NULL || strlen(pszServerName) == 0)
			return SEC_E_WRONG_PRINCIPAL;

		USES_CONVERSION ;
		pwszServerName = A2W(pszServerName) ;

		//
		// Build certificate chain.
		ZeroMemory(&ChainPara, sizeof(ChainPara));
		ChainPara.cbSize = sizeof(ChainPara);

		if(!CertGetCertificateChain(
								NULL,
								pServerCert,
								NULL,
								pServerCert->hCertStore,
								&ChainPara,
								0,
								NULL,
								&pChainContext))
		{
			Status = GetLastError();
			ATLTRACE("Error 0x%x returned by CertGetCertificateChain!\n", Status);
			goto cleanup;
		}

		//
		// Validate certificate chain.
		ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
		polHttps.cbStruct           = sizeof(HTTPSPolicyCallbackData);
		polHttps.dwAuthType         = AUTHTYPE_SERVER;
		polHttps.fdwChecks          = dwCertFlags;
		polHttps.pwszServerName     = pwszServerName;

		memset(&PolicyPara, 0, sizeof(PolicyPara));
		PolicyPara.cbSize            = sizeof(PolicyPara);
		PolicyPara.pvExtraPolicyPara = &polHttps;

		memset(&PolicyStatus, 0, sizeof(PolicyStatus));
		PolicyStatus.cbSize = sizeof(PolicyStatus);

		if(!CertVerifyCertificateChainPolicy(
								CERT_CHAIN_POLICY_SSL,
								pChainContext,
								&PolicyPara,
								&PolicyStatus))
		{
			Status = GetLastError();
			ATLTRACE("Error 0x%x returned by CertVerifyCertificateChainPolicy!\n", Status);
			goto cleanup;
		}

		if(PolicyStatus.dwError)
		{
			Status = PolicyStatus.dwError;
			DisplayWinVerifyTrustError(Status); 
			goto cleanup;
		}

		Status = SEC_E_OK;

	cleanup:

		if(pChainContext)
		{
			CertFreeCertificateChain(pChainContext);
		}

		return Status;
	}

	/*****************************************************************************/
	static void DisplayWinVerifyTrustError(DWORD Status)
	{
		LPSTR pszName = NULL;
		switch(Status)
		{
		case CERT_E_EXPIRED:                pszName = "CERT_E_EXPIRED";                 break;
		case CERT_E_VALIDITYPERIODNESTING:  pszName = "CERT_E_VALIDITYPERIODNESTING";   break;
		case CERT_E_ROLE:                   pszName = "CERT_E_ROLE";                    break;
		case CERT_E_PATHLENCONST:           pszName = "CERT_E_PATHLENCONST";            break;
		case CERT_E_CRITICAL:               pszName = "CERT_E_CRITICAL";                break;
		case CERT_E_PURPOSE:                pszName = "CERT_E_PURPOSE";                 break;
		case CERT_E_ISSUERCHAINING:         pszName = "CERT_E_ISSUERCHAINING";          break;
		case CERT_E_MALFORMED:              pszName = "CERT_E_MALFORMED";               break;
		case CERT_E_UNTRUSTEDROOT:          pszName = "CERT_E_UNTRUSTEDROOT";           break;
		case CERT_E_CHAINING:               pszName = "CERT_E_CHAINING";                break;
		case TRUST_E_FAIL:                  pszName = "TRUST_E_FAIL";                   break;
		case CERT_E_REVOKED:                pszName = "CERT_E_REVOKED";                 break;
		case CERT_E_UNTRUSTEDTESTROOT:      pszName = "CERT_E_UNTRUSTEDTESTROOT";       break;
		case CERT_E_REVOCATION_FAILURE:     pszName = "CERT_E_REVOCATION_FAILURE";      break;
		case CERT_E_CN_NO_MATCH:            pszName = "CERT_E_CN_NO_MATCH";             break;
		case CERT_E_WRONG_USAGE:            pszName = "CERT_E_WRONG_USAGE";             break;
		default:                            pszName = "(unknown)";                      break;
		}

		printf("Error 0x%x (%s) returned by CertVerifyCertificateChainPolicy!\n", Status, pszName);
	}

	/*****************************************************************************/
	LONG DisconnectFromServer(
							SOCKET          Socket, 
							PCredHandle     phCreds,
							CtxtHandle *    phContext)
	{
		DWORD           dwType;
		char           *pbMessage;
		DWORD           cbMessage;
		DWORD           cbData;

		SecBufferDesc   OutBuffer;
		SecBuffer       OutBuffers[1];
		DWORD           dwSSPIFlags;
		DWORD           dwSSPIOutFlags;
		TimeStamp       tsExpiry;
		DWORD           Status;

		//
		// Notify schannel that we are about to close the connection.
		//
		dwType = SCHANNEL_SHUTDOWN;

		OutBuffers[0].pvBuffer   = &dwType;
		OutBuffers[0].BufferType = SECBUFFER_TOKEN;
		OutBuffers[0].cbBuffer   = sizeof(dwType);

		OutBuffer.cBuffers  = 1;
		OutBuffer.pBuffers  = OutBuffers;
		OutBuffer.ulVersion = SECBUFFER_VERSION;

		Status = m_pSecurityFunc->ApplyControlToken(phContext, &OutBuffer);

		if(FAILED(Status)) 
		{
			ATLTRACE("**** Error 0x%x returned by ApplyControlToken\n", Status);
			goto cleanup;
		}

		//
		// Build an SSL close notify message.
		//

		dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
					  ISC_REQ_REPLAY_DETECT     |
					  ISC_REQ_CONFIDENTIALITY   |
					  ISC_RET_EXTENDED_ERROR    |
					  ISC_REQ_ALLOCATE_MEMORY   |
					  ISC_REQ_STREAM;

		OutBuffers[0].pvBuffer   = NULL;
		OutBuffers[0].BufferType = SECBUFFER_TOKEN;
		OutBuffers[0].cbBuffer   = 0;

		OutBuffer.cBuffers  = 1;
		OutBuffer.pBuffers  = OutBuffers;
		OutBuffer.ulVersion = SECBUFFER_VERSION;

		Status = m_pSecurityFunc->InitializeSecurityContextA(
						phCreds,
						phContext,
						NULL,
						dwSSPIFlags,
						0,
						SECURITY_NATIVE_DREP,
						NULL,
						0,
						phContext,
						&OutBuffer,
						&dwSSPIOutFlags,
						&tsExpiry);

		if(FAILED(Status)) 
		{
			ATLTRACE("**** Error 0x%x returned by InitializeSecurityContext\n", Status);
			goto cleanup;
		}

		pbMessage = (char *)OutBuffers[0].pvBuffer;
		cbMessage = OutBuffers[0].cbBuffer;


		//
		// Send the close notify message to the server.
		//
		if(pbMessage != NULL && cbMessage != 0)
		{
			cbData = send(Socket, pbMessage, cbMessage, 0);
			if(cbData == SOCKET_ERROR || cbData == 0)
			{
				Status = WSAGetLastError();
				ATLTRACE("**** Error %d sending close notify\n", Status);
				goto cleanup;
			}

			ATLTRACE("Sending Close Notify\n");
			ATLTRACE("%d bytes of handshake data sent\n", cbData);

			// Free output buffer.
			m_pSecurityFunc->FreeContextBuffer(pbMessage);
		}
    

	cleanup:

		// Free the security context.
		m_pSecurityFunc->DeleteSecurityContext(phContext);
		phContext = 0 ;

		return Status;
	}
} ;


template<class T>
bool					httpTransportBase<T>::m_SslInitDone = false ;

template<class T>
CComAutoCriticalSection httpTransportBase<T>::m_SslInitLock ;

template<class T>
HINSTANCE				httpTransportBase<T>::m_hSecurity = 0 ;

template<class T>
SecurityFunctionTableA  *httpTransportBase<T>::m_pSecurityFunc = 0 ;

template<class T>
HCERTSTORE				httpTransportBase<T>::m_hMyCertStore = 0 ;