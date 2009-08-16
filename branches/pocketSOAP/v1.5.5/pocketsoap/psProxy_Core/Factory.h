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

// Factory.h : Declaration of the CFactory

#ifndef __FACTORY_H_
#define __FACTORY_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CFactory
class ATL_NO_VTABLE CFactory : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFactory, &CLSID_CoSoapFactory>,
	public ISupportErrorInfo,
	public IDelegatingDispImpl<IProxyFactory2>,
	public IProxyFactory
{
public:
	CFactory()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FACTORY)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFactory)
	COM_INTERFACE_ENTRY(IProxyFactory)
	COM_INTERFACE_ENTRY(IProxyFactory2)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

public:
// IProxyFactory
	STDMETHOD(CreateProxy)(	/*[in]*/			BSTR endpointURL, 
												VARIANT methodNameURI, 
												VARIANT soapActionPattern, 
							/*[out,retval]*/	IDispatch ** Proxy);

// IProxyFactory2
	STDMETHOD(CreateProxy)(	/*[in]*/			BSTR endpointURL, 
												VARIANT methodNameURI, 
												VARIANT soapActionPattern, 
												VARIANT transport, 
							/*[out,retval]*/	IDispatch ** Proxy);
};

#endif //__FACTORY_H_
