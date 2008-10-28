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

#include "stdafx.h"
#include "PSOAP.h"
#include "SerializerHexBin.h"

/////////////////////////////////////////////////////////////////////////////
// CSerializerHexBin

// ITypesInit
STDMETHODIMP CSerializerHexBin::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType )
{
	m_type = xmlType ;
	m_typeNS = xmlTypeNamespace ;
	return S_OK ;
}

// ISimpleSoapSerializer
STDMETHODIMP CSerializerHexBin::Serialize( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ BSTR * dest ) 
{
	SAFEARRAY * psa = val->parray ;
	long ub, lb ;
	SafeArrayGetUBound(psa, 1, &ub) ;
	SafeArrayGetLBound(psa, 1, &lb) ;
	BYTE * src = 0 ;
	HRESULT hr = SafeArrayAccessData ( psa, (void **)&src ) ;
	if (FAILED(hr)) return hr ;

	size_t idx =0 ;
	byte digit ;
	CComBSTR res ;
	res.Attach ( SysAllocStringLen ( NULL, (ub-lb+1) *2 ) ) ;

	for ( long i = 0 ; i < (ub-lb+1); i++ )
	{
		digit = *src / 16 ;
		res.m_str[idx++] = digit + (digit > 9 ? 'A' - 10 : '0') ;
		digit = *src % 16 ;
		res.m_str[idx++] = digit + (digit > 9 ? 'A' - 10 : '0') ;
		++src ;
	}	
	SafeArrayUnaccessData(psa) ;
	*dest = res.Detach() ;
	return S_OK ;
} 

// ISoapSerializer
STDMETHODIMP CSerializerHexBin::Serialize( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) 
{
	if ( m_type.Length() ) 
	{
		CComPtr<ISerializerFactory> sf ;
		ctx->get_SerializerFactory(&sf) ;
		CComBSTR xsi ;
		sf->XsiForPrimaryNS(&xsi) ;
		dest->QNameAttribute ( CComBSTR(OLESTR("type")), xsi, m_type, m_typeNS ) ;
	}
	CComBSTR hb ;
	HRESULT hr = Serialize ( val, ctx, &hb ) ;
	if (SUCCEEDED(hr))
		dest->WriteTextNoEncoding(hb) ;
	return hr ;
}

// ISoapDeSerializer
STDMETHODIMP CSerializerHexBin::Start( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) 
{
	m_node = node ;
	m_node->put_Type(m_type) ;
	m_node->put_TypeNS(m_typeNS) ;
	return S_OK ;
}

BYTE CSerializerHexBin::hexToNum(OLECHAR *p)
{
	if (( *p >= '0') && (*p <= '9' ))
		return (*p-'0')  ;
	if ((*p >= 'a') && (*p <= 'f' ))
		return (*p-'a'+10)  ;
	if ((*p >= 'A') && (*p <= 'F' ))
		return (*p-'A'+10)  ;
	return 0xFF ;
}

STDMETHODIMP CSerializerHexBin::Deserialize( BSTR charData, ISOAPNamespaces * ns, VARIANT * dest )
{
	if ( ! dest ) return E_POINTER ;

	DWORD numChars = SysStringLen(charData) ;
	if ( numChars % 2 > 0 )
		return AtlReportError ( GetObjectCLSID(), _T("hexBinary data must have even number of characters"), IID_NULL, E_INVALID_LEX_REP ) ;

	DWORD numBytes = numChars / 2 ;

	SAFEARRAYBOUND rga ;
	rga.lLbound = 0 ;
	rga.cElements = numBytes ;
	SAFEARRAY * psa = SafeArrayCreate ( VT_UI1, 1, &rga ) ;
	if ( ! psa ) return E_OUTOFMEMORY;

	BYTE * data = 0 , d1, d2 ;
	OLECHAR * p = charData ;
	SafeArrayAccessData(psa, (void **)&data ) ;
	for ( DWORD i = 0 ; i < numBytes ; i++ )
	{
		d1 = hexToNum(p++) ;
		d2 = hexToNum(p++) ;
		if ( d1 == 0xFF || d2 == 0xFF )
		{
			SafeArrayUnaccessData(psa) ;
			SafeArrayDestroy(psa) ;
			return AtlReportError ( GetObjectCLSID(), _T("hexBinary invalid character, character must be 0-9,a-f,A-F"), IID_NULL, E_INVALID_LEX_REP ) ;
		}
		*data = d1 << 4 | d2 ;
		data++ ;

	}
	SafeArrayUnaccessData(psa) ;

	CComVariant vArr ;
	vArr.vt = VT_ARRAY | VT_UI1 ;
	vArr.parray = psa ;

	return vArr.Detach(dest) ;
}

STDMETHODIMP CSerializerHexBin::Characters( /*[in]*/ BSTR charData )
{
	CComVariant vArr ;
	HRESULT hr = Deserialize ( charData, NULL, &vArr ) ;
	if (SUCCEEDED(hr))
		m_node->put_Value(vArr) ;
	return hr ;
}

STDMETHODIMP CSerializerHexBin::Child( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode )
{
	return S_OK ;
}

STDMETHODIMP CSerializerHexBin::ChildReady( long id, /*[in]*/ ISOAPNode * childNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerHexBin::ChildRef( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerHexBin::Ref( BSTR id,	 /*[in]*/ ISOAPNode * idNode )
{
	return S_OK ;
}

STDMETHODIMP CSerializerHexBin::End()
{
	m_node.Release();
	return S_OK ;
}