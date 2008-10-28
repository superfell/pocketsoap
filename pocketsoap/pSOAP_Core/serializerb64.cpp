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
Portions created by Simon Fell are Copyright (C) 2000-2004
Simon Fell. All Rights Reserved.

Contributor(s):
*/


#include "stdafx.h"
#include "psoap.h"
#include "SerializerB64.h"
#include "base64.h"

/////////////////////////////////////////////////////////////////////////////
// CSerializerB64
/////////////////////////////////////////////////////////////////////////////
CSerializerB64::CSerializerB64()
{
}

CSerializerB64::~CSerializerB64()
{
}

STDMETHODIMP CSerializerB64::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) 
{
	m_type = xmlType ;
	m_typeNS = xmlTypeNamespace ;
	return S_OK ;
}

STDMETHODIMP CSerializerB64::Serialize( /*[in]*/ VARIANT * val, ISerializerContext * ctx, /*[in]*/ BSTR * dest ) 
{
	SAFEARRAY * psa = val->parray ;
	long ub, lb ;
	SafeArrayGetUBound(psa, 1, &ub) ;
	SafeArrayGetLBound(psa, 1, &lb) ;
	BYTE * src = 0 ;
	HRESULT hr = SafeArrayAccessData ( psa, (void **)&src ) ;
	if (FAILED(hr)) return hr ;

	size_t sizeNeeded = ( ub - lb + 1 ) * 4 / 3 ;
	if ( ( sizeNeeded % 4 ) != 0 )
		sizeNeeded += ( 4 - sizeNeeded % 4 ) ;

	CComBSTR b64 ;
	b64.Attach( SysAllocStringLen ( NULL, sizeNeeded ) ) ;

	hr = base64<WCHAR>::BufferEncode64( b64, sizeNeeded, src, ( ub - lb + 1 ) ) ;
	if (SUCCEEDED(hr))
		*dest = b64.Detach() ;

	SafeArrayUnaccessData(psa) ;
	return hr ;
}

STDMETHODIMP CSerializerB64::Serialize( /*[in]*/ VARIANT * val, ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) 
{
	if ( m_type.Length() ) 
	{
		CComPtr<ISerializerFactory> sf ;
		ctx->get_SerializerFactory(&sf) ;
		CComBSTR xsi ;
		sf->XsiForPrimaryNS(&xsi) ;
		dest->QNameAttribute ( CComBSTR(OLESTR("type")), xsi, m_type, m_typeNS ) ;
	}

	CComBSTR b64 ;
	HRESULT hr = Serialize(val, ctx, &b64 ) ;
	if (SUCCEEDED(hr))
		dest->WriteTextNoEncoding ( b64 ) ;
	return hr ;
}

// ISimpleSoapDeSerializer
STDMETHODIMP CSerializerB64::Deserialize( BSTR charData, ISOAPNamespaces * ns, VARIANT * dest )
{
	if ( 0 == SysStringLen(charData) )
		return S_OK ;

	WCHAR *end =  charData + SysStringLen(charData);
	WCHAR *readPos = charData ;

	while ( *readPos == ' ' || *readPos == '\r' || *readPos == '\t' || *readPos == '\n' )
		readPos++ ;

	WCHAR *writePos = readPos;

	// make one loop over the source data, removing any invalid characters (line breaks etc)
	while ( readPos < end )
	{
		if ( ( (*readPos >= 'A' && *readPos <= 'Z') || (*readPos >= 'a' && *readPos <= 'z') || (*readPos >= '0' && *readPos <= '9') || *readPos == '+' || *readPos == '/' || *readPos== '=' ) )
		{
			if( writePos != readPos )
				*writePos = *readPos;
			++writePos;
		}
		++readPos;
	}

	SAFEARRAYBOUND rga ;
	rga.lLbound = 0 ;
	rga.cElements = ( writePos - charData ) * 3 / 4 ;
	SAFEARRAY * psa = SafeArrayCreate ( VT_UI1, 1, &rga ) ;
	BYTE * dst = 0 ;
	SafeArrayAccessData( psa, (void **)&dst ) ;

	size_t destLen = rga.cElements + 1 ;
	HRESULT hr = base64<WCHAR>::BufferDecode64( dst, &destLen, charData, ( writePos - charData ) ) ;
	SafeArrayUnaccessData(psa);
	// if there were some trailing =='s in the string, then the actual data is slightly
	// smaller than our original calc, we need to shrink the array slightly
	if (SUCCEEDED(hr))
	{
		rga.cElements = destLen ;
		SafeArrayRedim ( psa, &rga ) ;
		VariantInit(dest) ;
		dest->vt = VT_ARRAY | VT_UI1 ;
		dest->parray = psa ;
	}
	else
		SafeArrayDestroy(psa) ;

	return hr ;
}

// ** De-Serializer
STDMETHODIMP CSerializerB64::Start( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) 
{
	m_node = node ;
	m_node->put_Type(m_type) ;
	m_node->put_TypeNS(m_typeNS) ;
	return S_OK ;
}

STDMETHODIMP CSerializerB64::Characters( /*[in]*/ BSTR charData ) 
{
	CComVariant arr ;
	_HR(Deserialize ( charData, NULL, &arr ));
	return m_node->put_Value(arr) ;
}

STDMETHODIMP CSerializerB64::End()
{
	m_node.Release() ;
	return S_OK ;
}

// as all we care about are simple values (i.e. no children), all these are no-ops

STDMETHODIMP CSerializerB64::Child( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerB64::ChildReady( long id, /*[in]*/ ISOAPNode * childNode )
{
	return S_OK ;
}

STDMETHODIMP CSerializerB64::Ref		( BSTR id,	 /*[in]*/ ISOAPNode * idNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerB64::ChildRef( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) 
{
	return S_OK ;
}
