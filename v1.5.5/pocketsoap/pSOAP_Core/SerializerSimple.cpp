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
Portions created by Simon Fell are Copyright (C) 2000-2002
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "psoap.h"
#include "SerializerSimple.h"
#include "stringBuff.h"

static const LCID serLCID = MAKELCID ( MAKELANGID ( LANG_ENGLISH, SUBLANG_ENGLISH_US ), SORT_DEFAULT ) ;

/////////////////////////////////////////////////////////////////////////////
// CSerializerSimpleBase
/////////////////////////////////////////////////////////////////////////////
CSerializerSimpleBase::CSerializerSimpleBase()
{
	ATLTRACE(_T("CSerializerSimpleBase::CSerializerSimpleBase()\n")) ;
}

CSerializerSimpleBase::~CSerializerSimpleBase()
{
	ATLTRACE(_T("CSerializerSimpleBase::~CSerializerSimpleBase()\n")) ;
}

STDMETHODIMP CSerializerSimpleBase::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) 
{
	m_type		= xmlType ;
	m_typeNS	= xmlTypeNamespace ;
	m_comType	= comType ;
	return S_OK ;
}

HRESULT CSerializerSimpleBase::WriteType(VARIANT * val, ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) 
{
	if ( m_type.Length() ) 
	{
		CComPtr<ISerializerFactory> sf ;
		ctx->get_SerializerFactory(&sf) ;
		CComBSTR xsi ;
		sf->XsiForPrimaryNS(&xsi) ;
		static CComBSTR type (OLESTR("type")) ;
		dest->QNameAttribute ( type, xsi, m_type, m_typeNS ) ;
	}
	return S_OK ;
}

HRESULT CSerializerSimpleBase::WriteValue(VARIANT * val, ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) 
{
	CComBSTR v ;
	HRESULT hr = Serialize ( val, ctx, &v ) ;
	if (SUCCEEDED(hr))
		dest->WriteText(v) ;
	return hr ;
}

STDMETHODIMP CSerializerSimpleBase::Serialize( /*[in]*/ VARIANT * val, ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) 
{
	WriteType	( val, ctx, dest ) ;
	WriteValue	( val, ctx, dest ) ;
	return S_OK ;
}

STDMETHODIMP CSerializerSimpleBase::Serialize( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ BSTR * dest ) 
{
	CComVariant v2 ;
	HRESULT hr = VariantChangeTypeEx ( &v2, val, serLCID, 0, VT_BSTR ) ;
	if (SUCCEEDED(hr))
	{
		*dest = v2.bstrVal ;
		v2.vt = VT_EMPTY ;
	}
	return hr ;
}

// ** De-Serializer
STDMETHODIMP CSerializerSimpleBase::Deserialize( BSTR characters, ISOAPNamespaces * ns, VARIANT * dest ) 
{
	HRESULT hr = S_OK ;
	CComVariant v ( characters ) ;
	if ( m_comType.vt == VT_I4 )
		hr = VariantChangeTypeEx ( &v, &v, serLCID, 0, (USHORT)m_comType.lVal ) ;
	if (FAILED(hr))
	{
		stringBuff_W err(L"Error de-serializing a ") ;
		if ( m_typeNS.Length() )
		{
			err.Append(L"{") ;
			err.Append (m_typeNS) ;
			err.Append (L"}") ;
		}
		err.Append (m_type);
		err.Append (L" unable to co-erce '") ;
		err.Append (characters, SysStringLen(characters)) ;
		err.Append (L"' to this type") ;
		return AtlReportError(GetCLSID(), err.c_str(), __uuidof(ISimpleSoapDeSerializer), hr ) ;
	}
	v.Detach(dest) ;
	return S_OK ;
}

STDMETHODIMP CSerializerSimpleBase::Start( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) 
{
	m_node = node ;
	m_node->put_Type(m_type) ;
	m_node->put_TypeNS(m_typeNS) ;
	m_ns = ns ;
	return S_OK ;
}

STDMETHODIMP CSerializerSimpleBase::Characters( /*[in]*/ BSTR charData ) 
{
	CComVariant v ;
	HRESULT hr = Deserialize(charData, m_ns, &v ) ;
	if (SUCCEEDED(hr))
		hr = m_node->put_Value ( v ) ;
	return S_OK ;
}

STDMETHODIMP CSerializerSimpleBase::End()
{
	m_node.Release() ;
	return S_OK ;
}

// as all we care about are simple values (i.e. no children), all these are no-ops
STDMETHODIMP CSerializerSimpleBase::Child( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerSimpleBase::ChildReady( long id, /*[in]*/ ISOAPNode * childNode )
{
	return S_OK ;
}

STDMETHODIMP CSerializerSimpleBase::Ref		( BSTR id,	 /*[in]*/ ISOAPNode * idNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerSimpleBase::ChildRef( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) 
{
	return S_OK ;
}

/////////////////////////////////////////////////////////////////////////////
// CSerializerSimple
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CSerializerBoolean
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CSerializerBoolean::Serialize( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ BSTR * dest ) 
{
	if ( val->boolVal == VARIANT_TRUE )
		*dest = SysAllocString(L"true") ;
	else
		*dest = SysAllocString(L"false") ;
	return S_OK ;
}

/////////////////////////////////////////////////////////////////////////////
// CDeserializerXsdLong
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDeserializerXsdLong::Deserialize( BSTR characters, ISOAPNamespaces * ns, VARIANT * dest )
{
	if (!dest) return E_POINTER;
	CComVariant actualComType = m_comType;
	m_comType.Clear();
	m_comType.vt = VT_I4;
	m_comType.lVal = VT_BSTR;

	VariantInit(dest);
	VARIANT xsdLongVal;
	VariantInit(&xsdLongVal);
	HRESULT hr = CSerializerSimpleBase::Deserialize(characters, ns, &xsdLongVal);
	m_comType = actualComType;
	if (FAILED(hr))
		return hr;
	if (xsdLongVal.vt != VT_BSTR) 
		return AtlReportError(GetCLSID(), OLESTR("deserialized value was not the expected VT_BSTR type"), IID_NULL, E_UNEXPECTED );
	
	CComPtr<IXsdLong> xsdLong;
	_HR(xsdLong.CoCreateInstance(actualComType.bstrVal));
	xsdLong->put_String(xsdLongVal.bstrVal);
	dest->vt = VT_DISPATCH;
	hr = xsdLong->QueryInterface(IID_IDispatch, (void **)&dest->pdispVal);
	return hr;
}

HRESULT CDeserializerXsdLong::WriteValue(VARIANT * val, ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) 
{
	CComVariant vXsdLong;
	_HR(vXsdLong.ChangeType(VT_UNKNOWN, val));
	CComPtr<IXsdLong> xsd;
	_HR(vXsdLong.punkVal->QueryInterface(__uuidof(xsd), (void **)&xsd));
	CComBSTR strVal;
	xsd->get_String(&strVal);
	return dest->WriteTextNoEncoding(strVal);
}

