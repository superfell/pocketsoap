// DataStructSerializer.cpp : Implementation of CDataStructSerializer
#include "stdafx.h"
#include "VcSerializerDemo.h"
#include "DataStructSerializer.h"

/////////////////////////////////////////////////////////////////////////////
// CDataStructSerializer

STDMETHODIMP CDataStructSerializer::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType )
{
	CComVariant v ;
	HRESULT hr = v.ChangeType(VT_BSTR, &comType ) ;
	if (FAILED(hr)) return hr;
	m_ComType = v.bstrVal ;
	return S_OK ;
}

STDMETHODIMP CDataStructSerializer::Serialize( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) 
{
	CComVariant vVal;
	HRESULT hr = vVal.ChangeType(VT_UNKNOWN, val);
	if(FAILED(hr)) return hr;

	CComPtr<IDataStruct> objVal;
	hr = vVal.punkVal->QueryInterface(&objVal);
	if(FAILED(hr)) return hr;

	CComVariant v;
	objVal->get_Int(&v.lVal);
	v.vt =VT_I4;
	dest->SerializeValue(&v, CComBSTR("varInt"), NULL);

	v.Clear();
	objVal->get_Float(&v.fltVal);
	v.vt = VT_R4;
	dest->SerializeValue(&v, CComBSTR("varFloat"), NULL);

	v.Clear();
	objVal->get_String(&v.bstrVal);
	v.vt = VT_BSTR;
	dest->SerializeValue(&v, CComBSTR(L"varString"), NULL);
	return S_OK;
}


STDMETHODIMP CDataStructSerializer::Start( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns )
{
	m_node = node;
	HRESULT hr = m_val.CoCreateInstance(m_ComType);
	return hr;
}

// this code assumes we're not going to get multi-ref elements for the child elements of the struct
// see the docs (or what the WSDL wizard generates) if you have to handle multi-ref children

STDMETHODIMP CDataStructSerializer::Child ( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode )
{
	if (VARIANT_TRUE == ready)
		return ChildReady(id, childNode);
	return S_OK;
}

STDMETHODIMP CDataStructSerializer::ChildReady ( long id, /*[in]*/ ISOAPNode * childNode )
{
	CComBSTR name;
	CComVariant val;
	childNode->get_Name(&name);
	childNode->get_Value(&val);
	if ( wcscmp ( name, L"varString" ) == 0 ) {
		m_val->put_String(val.bstrVal);
	} else if ( wcscmp ( name, L"varInt" ) == 0 ) {
		m_val->put_Int(val.lVal);
	} else if ( wcscmp ( name, L"varFloat" ) == 0 ) {
		m_val->put_Float(val.fltVal);
	}
	return S_OK;
}

STDMETHODIMP CDataStructSerializer::ChildRef ( BSTR href, /*[in]*/ ISOAPNode * hrefNode )
{
	return S_OK;
}

STDMETHODIMP CDataStructSerializer::Ref	( BSTR id,	/*[in]*/ ISOAPNode * idNode )
{
	return S_OK;
}

STDMETHODIMP CDataStructSerializer::Characters	( /*[in]*/ BSTR charData )
{
	return S_OK;
}

STDMETHODIMP CDataStructSerializer::End	() 
{
	CComVariant vVal(m_val);
	m_node->put_Value(vVal);
	m_node.Release();
	m_val.Release();
	return S_OK;
}
