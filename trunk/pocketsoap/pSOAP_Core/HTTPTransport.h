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
// HTTPTransport.h : Declaration of the CHTTPTransport
//
// This is now just a simple wrapper around PocketHTTP
/////////////////////////////////////////////////////////////////////////////

#ifndef __HTTPTRANSPORT_H_
#define __HTTPTRANSPORT_H_

#include "resource.h"       // main symbols

namespace PocketHTTP
{
struct IHttpRequest ;
struct IHttpResponse ;
struct IHeader ;
struct IReadableStream ;
}

/////////////////////////////////////////////////////////////////////////////
// CHTTPTransport
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CHTTPTransport : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CHTTPTransport, &CLSID_HTTPTransport>,
	public ISupportErrorInfo,
	public IHTTPTransportAdv2,
	public IsfDelegatingDispImpl<IHTTPTransportDisp>,
	public ISwATransport,
	public ISOAPTransportTimeout,
	public IStreamReader,
	public ISOAPTransport2
{
public:
	CHTTPTransport() ; 
	~CHTTPTransport() ; 

DECLARE_REGISTRY_RESOURCEID(IDR_HTTPTRANSPORT)
DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CHTTPTransport)
	COM_INTERFACE_ENTRY(IHTTPTransportAdv2)
	COM_INTERFACE_ENTRY(IHTTPTransportAdv)
	COM_INTERFACE_ENTRY(IHTTPTransport)
	COM_INTERFACE_ENTRY(ISOAPTransport2)
	COM_INTERFACE_ENTRY2(ISOAPTransport, IHTTPTransport)
	COM_INTERFACE_ENTRY(ISwATransport)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISOAPTransportTimeout)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IStreamReader)
END_COM_MAP()

	HRESULT FinalConstruct() ;

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

public:
// ISOAPTransport
	STDMETHOD(Send)		(/*[in]*/	  BSTR endpoint, /*[in]*/ BSTR Envelope);
	STDMETHOD(Receive)	(/*[in,out]*/ BSTR * characterEncoding, /*[out,retval]*/ SAFEARRAY ** Envelope);

// IHTTPTransport
	STDMETHOD(get_SOAPAction)(BSTR * soapAction) ;
	STDMETHOD(put_SOAPAction)(BSTR soapAction) ;

// IHTTPTransportAdv
	STDMETHOD(put_Timeout)			( long timeOut )   ;
	STDMETHOD(get_Timeout)			( long * timeOut ) ;
	STDMETHOD(SetProxy)				( BSTR ProxyServer, short ProxyPort ) ;
	STDMETHOD(NoProxy)				() ;
	STDMETHOD(Authentication)		( BSTR username, BSTR password ) ;
	STDMETHOD(ProxyAuthentication)	( BSTR username, BSTR password ) ;
	STDMETHOD(get_StatusCode)		( short * statusCode ) ;
	STDMETHOD(get_MaxRedirectDepth) ( short * depth ) ;
	STDMETHOD(put_MaxRedirectDepth) ( short depth ) ;

// IHTTPTransportAdv2
	STDMETHOD(get_Option)			( BSTR optionName, VARIANT * pVal ) ;
	STDMETHOD(put_Option)			( BSTR optionName,  VARIANT val ) ;

// 	ISwATransport
	STDMETHOD(get_ContentType)		( BSTR * ct ) ;
	STDMETHOD(put_ContentType)		( BSTR   ct ) ;
	STDMETHOD(Send)					( BSTR endpoint, VARIANT Payload );
	STDMETHOD(Receive)			    ( BSTR* characterEncoding, IUnknown ** responseStream ) ;

// IStreamReader
	STDMETHOD(Read)					( void* pv, DWORD cb, DWORD * pcbRead ) ;
	STDMETHOD(Reset)				() ;


private:
	CComPtr<PocketHTTP::IHttpRequest>		m_req ;
	CComPtr<PocketHTTP::IHeader>			m_type ;
	CComPtr<PocketHTTP::IHttpResponse>		m_res ;
	CComPtr<PocketHTTP::IReadableStream>	m_resStream ;
	CComBSTR								m_action ;
	bool									m_actionSet ;

	HRESULT	SetSoapAction() ;
	HRESULT ClearSoapAction() ;
};

#endif //__HTTPTRANSPORT_H_
