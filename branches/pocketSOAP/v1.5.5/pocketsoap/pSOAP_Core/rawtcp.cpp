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
Portions created by Simon Fell are Copyright (C) 2001
Simon Fell. All Rights Reserved.

Contributor(s):
*/

// rawtcp.cpp : Implementation of Crawtcp

#include "stdafx.h"
#include "psoap.h"
#include "rawtcp.h"

#ifdef _WIN32_WCE
#pragma comment(lib, "winsock.lib")
#else
#pragma comment(lib, "wsock32.lib")
#endif

/////////////////////////////////////////////////////////////////////////////
// Crawtcp
/////////////////////////////////////////////////////////////////////////////


void Crawtcp::ObjectMain(bool bStarting)
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


STDMETHODIMP Crawtcp::Send( BSTR endpoint,  BSTR Envelope)
{
	USES_CONVERSION ;
	_wcslwr( endpoint ) ;

	stringBuff_A szEndpoint = OLE2A(endpoint);

	if ( strncmp ( szEndpoint.c_str(), "rawtcp://", 9 ) != 0 )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Unexpected endpoint URL, expecting rawtcp://someserver:8080"), __uuidof(ISOAPTransport), E_UNEXPECTED ) ;

	stringBuff_A server ;
	UINT port ;
	const char * p = szEndpoint.c_str() + 9 ;
	const char * split = strchr(p, ':' ) ;
	if ( (!p) || ( !split) )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Unexpected endpoint URL, expecting rawtcp://someserver:8080"), __uuidof(ISOAPTransport), E_UNEXPECTED ) ;

	server.Append ( p, split-p ) ;
	port = atoi ( split + 1 ) ;

	stringBuff_A request ;
	Ole2Utf8 ( Envelope, request ) ;

	sockaddr_in sa ;
	HRESULT hr = ConstructSocketInfo(server.c_str(), port, sa) ;
	if (FAILED(hr)) return hr ;

	// create the client socket
	m_conn_socket = socket(AF_INET, SOCK_STREAM, 0); 
	if ( INVALID_SOCKET == m_conn_socket )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Failed creating socket"), __uuidof(ISOAPTransport), HRESULT_FROM_WIN32(WSAGetLastError()) );

#ifndef _WIN32_WCE
	// set timeouts [win32 only]
	if ( setsockopt ( m_conn_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&m_timeout, sizeof(m_timeout) ) )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Failed setting timeout"), __uuidof(ISOAPTransport), HRESULT_FROM_WIN32(WSAGetLastError()) ) ;

	if ( setsockopt ( m_conn_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&m_timeout, sizeof(m_timeout) ) )
		return AtlReportError ( GetObjectCLSID(), OLESTR("Failed setting timeout"), __uuidof(ISOAPTransport), HRESULT_FROM_WIN32(WSAGetLastError()) ) ;
#endif

	// connect
	if ( connect(m_conn_socket, (struct sockaddr*)&sa, sizeof(sa) ) == SOCKET_ERROR)
		return AtlReportError ( GetObjectCLSID(), OLESTR("Failed opening socket"), __uuidof(ISOAPTransport), HRESULT_FROM_WIN32(WSAGetLastError()) );

	// send data
	int retval = send ( m_conn_socket, request.c_str(), request.Size(), 0 ) ;
	if ( SOCKET_ERROR == retval )
	{
		DWORD wsaErr = WSAGetLastError() ;
		closesocket(m_conn_socket) ;
		hr = HRESULT_FROM_WIN32(wsaErr) ;
		if ( WSAETIMEDOUT == wsaErr )
			AtlReportError ( GetObjectCLSID(), OLESTR("Timeout sending to destination"), __uuidof(ISOAPTransport), hr ) ;
		else
			AtlReportError ( GetObjectCLSID(), OLESTR("Failing sending data to server"), __uuidof(ISOAPTransport), hr ) ;
	}

	// start shutdown [server uses this to know we're at the end of the request data]
	retval = shutdown ( m_conn_socket, SD_SEND ) ;
	if ( SOCKET_ERROR == retval )
	{
		DWORD wsaErr = WSAGetLastError() ;
		closesocket(m_conn_socket) ;
		hr = HRESULT_FROM_WIN32(wsaErr) ;
		if ( WSAETIMEDOUT == wsaErr )
			AtlReportError ( GetObjectCLSID(), OLESTR("Timeout sending to destination"), __uuidof(ISOAPTransport), hr ) ;
		else
			AtlReportError ( GetObjectCLSID(), OLESTR("Failing sending data to server"), __uuidof(ISOAPTransport), hr ) ;
	}

	return S_OK;
};

STDMETHODIMP Crawtcp::Receive( BSTR* characterEncoding, SAFEARRAY ** Envelope)
{
	if ( ! characterEncoding ) return E_POINTER ;
	if ( ! Envelope ) return E_POINTER ;
	*characterEncoding = SysAllocString(L"") ;	// let the parser figure it out

	if ( m_conn_socket == INVALID_SOCKET )
		return AtlReportError ( GetObjectCLSID(), OLESTR("No connection open, please call Send first !"), __uuidof(ISOAPTransport), E_UNEXPECTED ) ;

	stringBuff_A res ;
	char buff[8192] ;
	DWORD cbbuff = 8191 ;
	int retval= 1 ;
	while ( retval != SOCKET_ERROR && retval != 0 )
	{
		retval = recv ( m_conn_socket, buff, cbbuff, 0 ) ;
		if (retval == SOCKET_ERROR)
		{
			DWORD wsErr = WSAGetLastError() ;
			ATLTRACE(_T("rec error %d\n"), WSAGetLastError() ) ;
			// SOAP::Lite seems to close the connection on us straight away, don't error in this case
			if ( WSAENOTSOCK != wsErr )
			{
				closesocket ( m_conn_socket ) ;
				if ( WSAETIMEDOUT == wsErr )
					return AtlReportError ( GetObjectCLSID(), OLESTR("Timeout waiting for response"), __uuidof(ISOAPTransport), HRESULT_FROM_WIN32(wsErr) ) ;
				return AtlReportError ( GetObjectCLSID(), OLESTR("Error whilst receiving data"), __uuidof(ISOAPTransport), HRESULT_FROM_WIN32(wsErr) ) ;
			}
		}
		else
			res.Append ( buff, retval ) ;
	}

	// doh !, remember to close the socket
	closesocket(m_conn_socket) ;


	SAFEARRAYBOUND rga ;
	rga.lLbound  = 0 ;
	rga.cElements = res.Size() ;

	SAFEARRAY * psa = SafeArrayCreate ( VT_UI1, 1, &rga ) ;
	void * dest =0 ;
	SafeArrayAccessData ( psa, &dest ) ;
	memcpy ( dest, res.c_str(), res.Size() ) ;
	SafeArrayUnaccessData ( psa ) ;

	*Envelope = psa ;
	return S_OK;
};

STDMETHODIMP Crawtcp::put_Timeout( long timeOut )
{
	m_timeout = timeOut ;
	return S_OK ;
}

STDMETHODIMP Crawtcp::get_Timeout( long * timeOut )
{
	if ( ! timeOut ) return E_POINTER ;
	*timeOut = m_timeout ;
	return S_OK ;
}

