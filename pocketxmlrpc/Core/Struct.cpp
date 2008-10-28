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


#include "stdafx.h"
#include "PocketXMLRPC.h"
#include "Struct.h"

/////////////////////////////////////////////////////////////////////////////
// CStruct
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CStruct::GetTypeInfoCount(UINT* pctinfo) 
{
	*pctinfo = 0 ;
	return S_OK ;
}

STDMETHODIMP CStruct::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) 
{
	return E_INVALIDARG ;
}

STDMETHODIMP CStruct::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
	NAMES::iterator i ;
	while ( cNames-- > 0 )
	{
		i = m_names.find(rgszNames[cNames]) ;
		if ( i != m_names.end() )
			rgdispid[cNames] = i->second ;
		else
		{
			rgdispid[cNames] = m_nextId++ ;
			m_names[rgszNames[cNames]] = rgdispid[cNames] ;
			m_ids[rgdispid[cNames]] = rgszNames[cNames] ;
		}
	}
	return S_OK ;
}

STDMETHODIMP CStruct::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) 
{
	if ( DISPID_NEWENUM == dispidMember )
		return HandleEnumerator(pvarResult) ;
	if ( DISPID_VALUE   == dispidMember )
		dispidMember = FindIdFromParams(pdispparams) ;

	if ( wFlags & ( DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF ) )
	{
		if ( pdispparams->cArgs != 1 )
			return E_INVALIDARG ;
		m_vals[dispidMember] = pdispparams->rgvarg[0] ;
	}
	else
	{
		VariantInit(pvarResult) ;
		VariantCopy(pvarResult, &m_vals[dispidMember] ) ;
	}
	return S_OK ;
}

STDMETHODIMP CStruct::get_Value(/*[in]*/ BSTR name, /*[out,retval]*/ VARIANT * val ) 
{
	DISPID did ;
	DISPPARAMS dp = {0} ;
	GetIDsOfNames ( IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &did ) ;
	return Invoke ( did, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &dp, val, NULL, NULL ) ;
}

STDMETHODIMP CStruct::put_Value(/*[in]*/ BSTR name, /*[in]*/ VARIANT val ) 
{
	DISPID did ;
	DISPPARAMS dp = {0} ;
	dp.cArgs = 1 ;
	dp.rgvarg = &val ;
	GetIDsOfNames ( IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &did ) ;
	return Invoke ( did, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYPUT, &dp, NULL, NULL, NULL ) ;
}

DISPID CStruct::FindIdFromParams(DISPPARAMS * pdispparams)
{
	CComVariant v ;
	VariantCopyInd ( &v, &pdispparams->rgvarg[0] ) ;
	DISPID did ;
	GetIDsOfNames ( IID_NULL, &v.bstrVal, 1, LOCALE_SYSTEM_DEFAULT, &did ) ;
	return did ;
}

class CopyName
{
public:
	static HRESULT copy(VARIANT * dest, CStruct::NAMES::value_type * src)
	{
		dest->vt = VT_BSTR ;
		dest->bstrVal = SysAllocStringLen(src->first.c_str(), src->first.size() ) ;
		return S_OK ;
	}
	static void init(VARIANT * p) { VariantInit(p) ; }
	static void destroy(VARIANT *p) { VariantClear(p) ; }
};

HRESULT CStruct::HandleEnumerator(VARIANT *pvarResult)
{
	VariantInit(pvarResult) ;
	typedef CComObject<CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, CopyName, NAMES> > ENUM ;
	ENUM * p = 0 ;
	HRESULT hr = p->CreateInstance(&p) ;
	if (FAILED(hr)) return hr ;
	p->AddRef() ;
	p->Init ( GetUnknown(), m_names ) ;
	hr = p->QueryInterface(&pvarResult->punkVal) ;
	if (SUCCEEDED(hr))
		pvarResult->vt = VT_UNKNOWN ;

	p->Release() ;
	return hr ;
}