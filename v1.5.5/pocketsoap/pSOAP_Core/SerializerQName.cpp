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
#include "SerializerQName.h"

/////////////////////////////////////////////////////////////////////////////
// CSerializerQName
/////////////////////////////////////////////////////////////////////////////

// ITypesInit
STDMETHODIMP CSerializerQName::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType )
{
	m_type	= xmlType ;
	m_ns	= xmlTypeNamespace ;
	CComVariant v ;
	HRESULT hr = v.ChangeType(VT_BSTR, &comType ) ;
	if (FAILED(hr)) return hr;
	m_ComType = v.bstrVal ;
	return S_OK ;
}

// ISimpleSoapSerializer
STDMETHODIMP CSerializerQName::Serialize( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ BSTR * dest ) 
{
	CComBSTR prefix, name, ns ;
	CComPtr<IXmlQName> qn ;
	CComVariant vq ;
	HRESULT hr = vq.ChangeType(VT_UNKNOWN, val ) ;
	if (FAILED(hr)) return hr ;
	hr = vq.punkVal->QueryInterface(__uuidof(qn), (void **)&qn ) ;
	if (FAILED(hr)) return hr ;

	CComPtr<ISOAPNamespaces> names ;
	ctx->get_Namespaces(&names) ;

	qn->get_Name(&name) ;
	qn->get_Namespace(&ns) ;

	names->GetPrefixForURI ( ns, &prefix ) ;
	CComBSTR xmlqn ;
	if ( prefix.Length() )
	{
		static const OLECHAR colon = ':' ;
		xmlqn.Attach ( SysAllocStringLen ( NULL, prefix.Length() + 1 + name.Length() ) ) ;
		memcpy ( xmlqn.m_str, prefix.m_str, prefix.Length() * sizeof(OLECHAR) ) ;
		memcpy ( &xmlqn.m_str[prefix.Length()], &colon, sizeof(OLECHAR) ) ;
		memcpy ( &xmlqn.m_str[prefix.Length()+1], name.m_str, name.Length() * sizeof(OLECHAR) ) ;
	}
	else
		xmlqn = name ;

	*dest = xmlqn.Detach() ;
	return S_OK ;
}

// ISoapSerializer
STDMETHODIMP CSerializerQName::Serialize( /*[in]*/ VARIANT * val, /*[in]*/ ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest ) 
{
	if ( m_type.Length() ) 
	{
		CComPtr<ISerializerFactory> sf ;
		ctx->get_SerializerFactory(&sf) ;
		CComBSTR xsi ;
		sf->XsiForPrimaryNS(&xsi) ;
		static CComBSTR type (OLESTR("type")) ;
		dest->QNameAttribute ( type, xsi, m_type, m_ns ) ;
	}
	CComBSTR xmlqn ;
	HRESULT hr = Serialize ( val, ctx, &xmlqn ) ;
	if (SUCCEEDED(hr))
		dest->WriteTextNoEncoding(xmlqn) ;

	return hr ;
}

// ISimpleSoapDeserializer
STDMETHODIMP CSerializerQName::Deserialize( BSTR charData, ISOAPNamespaces * ns, VARIANT * dest ) 
{
	// expected format is 'foo:bar' or just 'bar' [ namespace is the default NS in this case ]
	OLECHAR * c = wcschr ( charData, ':' ) ;
	CComBSTR prefix, localName , uri ;
	if ( c ) 
	{
		localName = c+1 ;
		prefix.Attach ( SysAllocStringLen(charData, c-charData ) ) ;
	}
	else
		localName = charData ;

	ns->GetURIForPrefix ( prefix, &uri ) ;

	CComPtr<IXmlQName> qn ;
	HRESULT hr = qn.CoCreateInstance(m_ComType) ;
	if (FAILED(hr)) return hr ;
	qn->Set ( localName, uri ) ;
	
	CComVariant qnv(qn) ;
	return qnv.Detach(dest) ;
}

// ISoapDeSerializer
STDMETHODIMP CSerializerQName::Start( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) 
{
	m_node = node ;
	m_names = ns ;
	return S_OK ;
}

STDMETHODIMP CSerializerQName::Characters( /*[in]*/ BSTR charData )
{
	CComVariant qnv ;
	HRESULT hr = Deserialize ( charData, m_names, &qnv ) ;
	if (FAILED(hr)) return hr ;
	m_node->put_Value(qnv) ;
	return S_OK;
}

STDMETHODIMP CSerializerQName::Child( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode )
{
	return S_OK ;
}

STDMETHODIMP CSerializerQName::ChildReady( long id, /*[in]*/ ISOAPNode * childNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerQName::ChildRef( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerQName::Ref( BSTR id,	 /*[in]*/ ISOAPNode * idNode )
{
	return S_OK ;
}

STDMETHODIMP CSerializerQName::End()
{
	m_node.Release();
	m_names.Release();
	m_type.Empty() ;
	m_ns.Empty() ;
	m_ComType.Empty() ;
	return S_OK ;
}