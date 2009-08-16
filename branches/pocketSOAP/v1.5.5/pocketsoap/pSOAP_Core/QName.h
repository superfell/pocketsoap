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

#ifndef __QNAME_H_
#define __QNAME_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CQName
class ATL_NO_VTABLE CQName : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CQName, &CLSID_CoQName>,
	public IsfDelegatingDispImpl<IXmlQName>
{
public:
	CQName()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_QNAME)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CQName)
	COM_INTERFACE_ENTRY(IXmlQName)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IXmlQName
public:
	STDMETHOD(Set)(/*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace);
	STDMETHOD(get_Namespace)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Namespace)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Name)(/*[in]*/ BSTR newVal);

private:
	CComBSTR m_name, m_namespace ;
};

#endif //__QNAME_H_
