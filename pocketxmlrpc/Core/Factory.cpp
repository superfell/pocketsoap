/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketXML-RPC.

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2002-2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/


#include "stdafx.h"
#include "PocketXMLRPC.h"
#include "Factory.h"
#include "client.h"

/////////////////////////////////////////////////////////////////////////////
// CFactory
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFactory::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IXmlRpcFactory
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::ATL::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CFactory::Proxy(BSTR endpointURL, BSTR methodPrefix, BSTR serverUsername, BSTR serverPassword, BSTR httpProxyServer, long httpProxyPort, BSTR proxyUsername, BSTR proxyPassword, long timeOut, IDispatch **Proxy)
{
	if ( ! Proxy ) return E_POINTER ;

	CComPtr<IHttpRequest> t ;
	HRESULT hr = t.CoCreateInstance(CLSID_CoPocketHTTP) ;
	if (FAILED(hr)) return hr ;

	if ( SysStringLen(serverUsername) || SysStringLen(serverPassword ) )
		t->Authentication ( serverUsername, serverPassword ) ;

	if ( SysStringLen(httpProxyServer ))
		t->SetProxy ( httpProxyServer, (short)httpProxyPort ) ;

	if ( SysStringLen(proxyUsername) || SysStringLen(proxyPassword ) ) 
		t->ProxyAuthentication ( proxyUsername, proxyPassword ) ;

	t->put_Timeout(timeOut * 1000) ;
	
	CComQIPtr<IDispatch> disp(t) ;
	return ProxyForTransport(endpointURL, methodPrefix, disp, Proxy) ;
}

STDMETHODIMP CFactory::ProxyForTransport(BSTR endpointURL, BSTR methodPrefix, IDispatch * pTransportObject, IDispatch ** Proxy)
{
	CComPtr<IHttpRequest> t ;
	HRESULT hr = pTransportObject->QueryInterface(__uuidof(t), (void **)&t) ;
	if (FAILED(hr)) return hr ;

	CComPtr<IHeadersCollection> hc ;
	CComPtr<IHeader> h ;
	t->get_Headers(&hc) ;

	// change method to POST
	static const CComBSTR post(OLESTR("POST")) ;
	t->put_Method(post) ;

	// set Content-Type
	static const CComBSTR textxml(OLESTR("text/xml")) ;
	static const CComBSTR ct(OLESTR("Content-Type")) ;
	hc->Create ( ct, textxml, &h ) ;

	// User-Agent
	h.Release() ;
	static const CComBSTR ua(OLESTR("User-Agent")) ;
	static const CComBSTR pxml(OLESTR("PocketXML-RPC/1.2.1")) ;
	hc->Find ( ua, &h ) ;
	if(h)
		h->put_Value(pxml) ;

	// create the proxy object
	CComObject<CClient> * p = 0 ;
	hr = p->CreateInstance(&p) ;
	if (FAILED(hr)) return hr ;
	p->AddRef() ;

	hr = p->Initialize ( endpointURL, methodPrefix, t ) ;
	if ( SUCCEEDED(hr))
		hr = p->QueryInterface(Proxy) ;

	p->Release() ;
	return hr ;
}
