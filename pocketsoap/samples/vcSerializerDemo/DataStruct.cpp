// DataStruct.cpp : Implementation of CDataStruct
#include "stdafx.h"
#include "VcSerializerDemo.h"
#include "DataStruct.h"

/////////////////////////////////////////////////////////////////////////////
// CDataStruct


STDMETHODIMP CDataStruct::get_String(BSTR *pVal)
{
	return m_str.CopyTo(pVal);
}

STDMETHODIMP CDataStruct::put_String(BSTR newVal)
{
	m_str = newVal;
	return S_OK;
}

STDMETHODIMP CDataStruct::get_Int(long *pVal)
{
	if(!pVal) return E_POINTER;
	*pVal = m_int;
	return S_OK;
}

STDMETHODIMP CDataStruct::put_Int(long newVal)
{
	m_int = newVal;
	return S_OK;
}

STDMETHODIMP CDataStruct::get_Float(float *pVal)
{
	if(!pVal) return E_POINTER;
	*pVal = m_float;
	return S_OK;
}

STDMETHODIMP CDataStruct::put_Float(float newVal)
{
	m_float = newVal;
	return S_OK;
}
