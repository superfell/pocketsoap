/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketXML-RPC

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2002
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#ifndef __STRUCT_H_
#define __STRUCT_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CStruct
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CStruct : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CStruct, &CLSID_CoXmlRpcStruct>,
	public IDispatch,
	public IXmlRpcStruct
{
public:
	CStruct() : m_nextId(42)
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_STRUCT)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CStruct)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IXmlRpcStruct)
END_COM_MAP()

// IDispatch
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) ;
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) ;
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) ;

// IXmlRpcStruct
	STDMETHOD(get_Value)(/*[in]*/ BSTR name, /*[out,retval]*/ VARIANT * val ) ;
	STDMETHOD(put_Value)(/*[in]*/ BSTR name, /*[in]*/ VARIANT val ) ;

private:
	typedef std::map<std::wstring, DISPID> NAMES ;
	typedef std::map<DISPID, std::wstring> DISPIDS ;
	typedef std::map<DISPID, CComVariant> VALUES ;

	NAMES		m_names ;
	DISPIDS		m_ids ;
	VALUES		m_vals ;
	DISPID		m_nextId ;

	HRESULT HandleEnumerator(VARIANT *pvarResult) ;
	DISPID  FindIdFromParams(DISPPARAMS * pdispparams) ;
};

#endif //__STRUCT_H_
