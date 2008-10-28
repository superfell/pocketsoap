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
Portions created by Simon Fell are Copyright (C) 2005
Simon Fell. All Rights Reserved.

Contributor(s):
*/


// xsdLong.cpp : Implementation of xsdLong
#include "stdafx.h"
#include "PSOAP.h"
#include "xsdLong.h"
#include "tags.h"

/////////////////////////////////////////////////////////////////////////////
// xsdLong
/////////////////////////////////////////////////////////////////////////////

CXsdLong::CXsdLong() : longlong(0)
{
}

STDMETHODIMP CXsdLong::get_HiDWord( /*[out,retval]*/ long * pVal)
{
	if (!pVal) return E_POINTER;
	*pVal = (long)(longlong >> 32);
	return S_OK;
}

STDMETHODIMP CXsdLong::put_HiDWord( /*[in]*/ long val)
{
	LONGLONG v = val;
	longlong = (longlong & 0x00000000FFFFFFFF) | (v << 32);
	return S_OK;
}

STDMETHODIMP CXsdLong::get_LoDWord( /*[out,retval]*/ long * pVal)
{
	if (!pVal) return E_POINTER;
	*pVal = (long)(longlong & 0xFFFFFFFF);
	return S_OK;
}

STDMETHODIMP CXsdLong::put_LoDWord( /*[in]*/ long val)
{
	longlong = (longlong & 0xFFFFFFFF00000000) | val;
	return S_OK;
}

STDMETHODIMP CXsdLong::get_String( /*[out,retval]*/ BSTR * pVal)
{
	if (!pVal) return E_POINTER;
	WCHAR buff[32];
	_i64tow(longlong, buff, 10);
	*pVal = SysAllocString(buff);
	return S_OK;
}

STDMETHODIMP CXsdLong::put_String( /*[in]*/ BSTR val)
{
	int len = SysStringLen(val);
	for (int i = 0; i < len; i++) 
	{
		if (( (val[i] < '0') || (val[i] > '9') ) && (val[i] != '-') && (val[i] != '+') )
			return AtlReportError(GetObjectCLSID(), OLESTR("Input string is an invalid format"), IID_NULL, E_INVALID_LEX_REP);
	}
	longlong = _wtoi64(val);
	return S_OK;
}

STDMETHODIMP CXsdLong::Register(/*[in]*/ ISerializerFactoryConfig * cfgFactory )
{
	CComVariant vXsdLong(OLESTR("PocketSOAP.XSDLong"));
	CComBSTR bstrLong(OLESTR("long"));
	CComBSTR bstrXsd99(XSD99_URI);
	CComBSTR bstrXsd01(XSD01_URI);
	CComBSTR bstrSerializer(OLESTR("PocketSOAP.DeserializerXSDLong"));

	// register for deserialization
	_HR(cfgFactory->Deserializer ( bstrLong, bstrXsd99, VARIANT_FALSE, vXsdLong, bstrSerializer));
	_HR(cfgFactory->Deserializer ( bstrLong, bstrXsd01, VARIANT_FALSE, vXsdLong, bstrSerializer));

	LPOLESTR szIID = 0;
	StringFromIID(IID_IXsdLong, &szIID);
	CComVariant vIxsdLong(szIID);
	CoTaskMemFree(szIID);
	szIID = 0;

	_HR(cfgFactory->Serializer (vIxsdLong, bstrLong, bstrXsd99, bstrSerializer));
	_HR(cfgFactory->Serializer (vIxsdLong, bstrLong, bstrXsd01, bstrSerializer));

	return S_OK;
}