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
Portions created by Simon Fell are Copyright (C) 2000
Simon Fell. All Rights Reserved.

Contributor(s):
*/

// Factory.cpp : Implementation of CFactory
#include "stdafx.h"
#include "PsProxy.h"
#include "Factory.h"

#include "proxy.h"

/////////////////////////////////////////////////////////////////////////////
// CFactory
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFactory::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IProxyFactory, &IID_IProxyFactory2
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CFactory::CreateProxy ( BSTR endpointURL, VARIANT methodNameURI, VARIANT soapActionPattern, IDispatch **Proxy)
{
	static const CComVariant defTransport ( L"" ) ;

	return CreateProxy ( endpointURL, methodNameURI, soapActionPattern, defTransport, Proxy ) ;
}

STDMETHODIMP CFactory::CreateProxy ( BSTR endpointURL, VARIANT methodNameURI, VARIANT soapActionPattern, VARIANT transport, IDispatch **Proxy)
{
	CComObject<CProxy> * p ;
	HRESULT hr = p->CreateInstance(&p) ;
	if (FAILED(hr)) return hr ;

	CComVariant vuri, vsoapAction ;
	vuri.ChangeType(VT_BSTR, &methodNameURI) ;
	vsoapAction.ChangeType(VT_BSTR, &soapActionPattern) ;

	p->AddRef() ;
	p->Init ( endpointURL, vuri.bstrVal, transport, vsoapAction.bstrVal ) ;
	hr = p->QueryInterface(Proxy) ;
	p->Release() ;
	return hr;
}
