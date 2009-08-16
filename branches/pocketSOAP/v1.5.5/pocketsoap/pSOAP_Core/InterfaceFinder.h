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
Portions created by Simon Fell are Copyright (C) 2002
Simon Fell. All Rights Reserved.

Contributor(s):
*/

/////////////////////////////////////////////////////////////////////////////
// InterfaceFinder.h : Declaration of the CInterfaceFinder
/////////////////////////////////////////////////////////////////////////////

#ifndef __INTERFACEFINDER_H_
#define __INTERFACEFINDER_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CInterfaceFinder
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CInterfaceFinder : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CInterfaceFinder, &CLSID_CoInterfaceFinder>,
	public IsfDelegatingDispImpl<IInterfaceFinder>
{
public:
	CInterfaceFinder()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_INTERFACEFINDER)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CInterfaceFinder)
	COM_INTERFACE_ENTRY(IInterfaceFinder)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
// IInterfaceFinder
	STDMETHOD(DefaultIID)(/*[in]*/ BSTR theClass, /*[out,retval]*/ BSTR * iid ) ;

};

#endif //__INTERFACEFINDER_H_
