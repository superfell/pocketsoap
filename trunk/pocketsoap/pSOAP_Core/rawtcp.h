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
   
// rawtcp.h : Declaration of the Crawtcp

#ifndef __RAWTCP_H_
#define __RAWTCP_H_

#include "resource.h"       // main symbols
#include "stringBuff.h"
#include "transportBase.h"
/////////////////////////////////////////////////////////////////////////////
// Crawtcp


class ATL_NO_VTABLE Crawtcp : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<Crawtcp, &CLSID_RawTcpTransport>,
	public IsfDelegatingDispImpl<ISOAPTransportTimeout>,
	public TransportBase<Crawtcp>
{
public:
	Crawtcp() :m_conn_socket(INVALID_SOCKET), m_timeout(15000)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_RAWTCP)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(Crawtcp)
	COM_INTERFACE_ENTRY(ISOAPTransport)
	COM_INTERFACE_ENTRY(ISOAPTransportTimeout)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	static void WINAPI ObjectMain(bool bStarting) ;

// ISOAPTransport
public:
	STDMETHOD(Send)	  ( BSTR endpoint,  BSTR Envelope);
	STDMETHOD(Receive)( BSTR* characterEncoding,  SAFEARRAY ** Envelope);

// ISOAPTransportTimeout
	STDMETHOD(put_Timeout)			( long timeOut )   ;
	STDMETHOD(get_Timeout)			( long * timeOut ) ;

private:
	SOCKET			m_conn_socket ;
	int				m_timeout ;		// send/rec timeout
};

#endif //__RAWTCP_H_
