// SerializerNull.cpp : Implementation of CSerializerNull
#include "stdafx.h"
#include "PSOAP.h"
#include "SerializerNull.h"
#include "tags.h"

/////////////////////////////////////////////////////////////////////////////
// CSerializerNull

// ITypesInit
STDMETHODIMP CSerializerNull::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType )
{
	ATLTRACE(_T("NullSerialize::Init %ls %ls"), xmlType, xmlTypeNamespace ) ;
	m_nullVal.Clear();
	if ( xmlType && xmlTypeNamespace && ( wcscmp ( xmlType, L"string" ) == 0 ) &&
		 ( ( wcscmp ( xmlTypeNamespace, XSD01_URI ) == 0 ) ||
		   ( wcscmp ( xmlTypeNamespace, XSD99_URI ) == 0 ) ) )
	{
		m_nullVal.vt = VT_BSTR ;
		m_nullVal.bstrVal = SysAllocString(L"") ;
	}
	else
		m_nullVal.vt = VT_NULL;

	return S_OK ;
}

// ISoapDeSerializer
STDMETHODIMP CSerializerNull::Start( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) 
{
	m_node = node ;
	return S_OK ;
}

STDMETHODIMP CSerializerNull::Characters( /*[in]*/ BSTR charData )
{
	return S_OK;
}

STDMETHODIMP CSerializerNull::Child( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode )
{
	return S_OK ;
}

STDMETHODIMP CSerializerNull::ChildReady( long id, /*[in]*/ ISOAPNode * childNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerNull::ChildRef( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerNull::Ref( BSTR id,	 /*[in]*/ ISOAPNode * idNode )
{
	return S_OK ;
}

STDMETHODIMP CSerializerNull::End()
{
	m_node->put_Value(m_nullVal) ;
	m_node.Release();
	m_nullVal.Clear() ;
	m_nullVal.vt = VT_NULL ;
	return S_OK ;
}