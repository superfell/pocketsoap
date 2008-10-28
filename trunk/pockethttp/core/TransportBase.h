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
Portions created by Simon Fell are Copyright (C) 2001,2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/
   
//////////////////////////////////////////////////////////////////////
// TransportBase.h: interface for the TransportBase class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRANSPORTBASE_H__7DED9919_CE3A_4747_A83B_A26531FE3E2A__INCLUDED_)
#define AFX_TRANSPORTBASE_H__7DED9919_CE3A_4747_A83B_A26531FE3E2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stringBuff.h"
#include "stringHelpers.h"

template<class T>
class TransportBase  
{
public:
	HRESULT ConstructSocketInfo(const char * server, UINT port, sockaddr_in &socket )
	{
		USES_CONVERSION ;

		struct hostent * ph = 0 ;
		memset(&socket, 0, sizeof(socket)) ;
		socket.sin_port = htons(port);

		// if first character of the server name is a letter, then try getting the address from the name
		if ( isalpha(*server))
			ph = gethostbyname(server) ;

		if ( ph )
		{
			socket.sin_family = ph->h_addrtype;
			memcpy(&(socket.sin_addr),ph->h_addr,ph->h_length);
		}
		else
		{
			// either the server name didn't start with a letter, or the name lookup failed, try it as a straight ip address
			UINT addr = inet_addr(server) ;
			if ( !addr )
				return AtlReportError ( static_cast<T *>(this)->GetObjectCLSID(), OLESTR("Failed to resolve server name / IP address"), __uuidof(IHttpRequest), HRESULT_FROM_WIN32(WSAGetLastError()) );

			socket.sin_addr.s_addr = addr ;
			socket.sin_family = AF_INET ;
		}
		
		return S_OK ;
	}

	HRESULT resolveName ( LPCOLESTR wcsName, u_long * pAddr )
	{
		USES_CONVERSION ;
		char * szName = OLE2A(wcsName) ;
		return resolveName( szName, pAddr ) ;
	}

	HRESULT resolveName ( const char * szName, u_long * pAddr )
	{
		struct hostent * ph = 0 ;

		// try it a straight IP address first
		u_long addr = inet_addr(szName) ;
		if ( INADDR_NONE == addr ) 
		{
			// that failed, treat it as a name
			ph = gethostbyname(szName) ;
			if ( ph )
				memcpy(pAddr, ph->h_addr, ph->h_length );
			else
				return AtlReportError ( static_cast<T *>(this)->GetObjectCLSID(), OLESTR("Failed to resolve server name / IP address"), __uuidof(IHttpRequest), HRESULT_FROM_WIN32(WSAGetLastError()) );
		}
		else
			*pAddr = addr ;
		return S_OK ;	
	}
};

#endif // !defined(AFX_TRANSPORTBASE_H__7DED9919_CE3A_4747_A83B_A26531FE3E2A__INCLUDED_)
