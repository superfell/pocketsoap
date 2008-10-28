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
Portions created by Simon Fell are Copyright (C) 2000
Simon Fell. All Rights Reserved.

Contributor(s):
*/


#include "stdafx.h"
#include "PSOAP.h"
#include "SerializerNode.h"

/////////////////////////////////////////////////////////////////////////////
// CSerializerNode
//		This is a generic serializer nodes + string (de)serializer
//		and is the default if nothing else can be found to use
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CSerializerNode::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_ISoapSerializer, &IID_ISoapDeSerializer, &IID_ITypesInit
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (::ATL::InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

CSerializerNode::CSerializerNode()
{
	ATLTRACE(_T("CSerializerNode::CSerializerNode\n")) ;
}

CSerializerNode::~CSerializerNode()
{
	ATLTRACE(_T("CSerializerNode::~CSerializerNode()\n") ) ;
}

STDMETHODIMP CSerializerNode::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerNode::Serialize( /*[in]*/ VARIANT *val, ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest )
{
	CComPtr<ISOAPNode> n ;
	val->punkVal->QueryInterface(__uuidof(n), (void **)&n);
	HRESULT hr ;
	if (n)
	{
		// remeber to include the type if set by the user
		CComBSTR type, typeNS ;
		n->get_Type(&type) ;
		n->get_TypeNS(&typeNS) ;
		if ( type.Length() ) 
		{
			CComPtr<ISerializerFactory> sf ;
			ctx->get_SerializerFactory(&sf) ;
			CComBSTR xsi ;
			sf->XsiForPrimaryNS(&xsi) ;
			dest->QNameAttribute ( CComBSTR(OLESTR("type")), xsi, type, typeNS ) ;
		}

		CComVariant v ;
		n->get_Value(&v) ;
		if ( SUCCEEDED(v.ChangeType(VT_BSTR)))
		{
			dest->WriteText ( v.bstrVal ) ;
		}
		else
		{
			CComPtr<ISOAPNode> cn ;
			CComPtr<ISOAPNodes> c ;
			n->get_Nodes(&c) ;
			long num = 0 ;
			c->get_Count(&num) ;
			for ( long i = 0 ; i < num ; ++i )
			{
				c->get_Item(i,&cn) ;
				hr = dest->SerializeNode(cn) ;
				if (FAILED(hr)) return hr ;
				cn.Release() ;
			}
		}
		return S_OK ;
	}
	return E_FAIL ;
}

// ** De-Serializer
STDMETHODIMP CSerializerNode::Start( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) 
{
	ATLTRACE(_T("CNodeSerializer::Start node=0x%x\n"), node ) ;
	m_node = node ;
	m_name = ElementName ;
	return S_OK ;
}

STDMETHODIMP CSerializerNode::Child( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) 
{
	ATLTRACE(_T("CNodeSerializer::Child %d\n"), id ) ;
	CComPtr<ISOAPNodes> c ;
	m_node->get_Nodes(&c) ;
	c->Append(childNode) ;
	return S_OK ;
}

STDMETHODIMP CSerializerNode::ChildReady( long id, /*[in]*/ ISOAPNode * childNode )
{
	ATLTRACE(_T("CNodeSerializer::ChildReady %d\n"), id ) ;
	return S_OK ;
}

STDMETHODIMP CSerializerNode::Ref ( BSTR id, /*[in]*/ ISOAPNode * idNode ) 
{
	ATLTRACE(_T("CNodeSerializer::Ref %ls\n"), id ) ;
	CComVariant v ;
	idNode->get_Value(&v) ;
	for ( int i = m_refs.size() -1 ; i >= 0 ; --i )
	{
		if ( m_refs[i].id == id )
		{
			CComBSTR type, ns ;
			idNode->get_Type(&type) ;
			idNode->get_TypeNS(&ns) ;
			if ( type.Length() > 0 )
			{
				m_refs[i].node->put_Type(type) ;
				m_refs[i].node->put_TypeNS(ns) ;
			}
			m_refs[i].node->put_Value(v) ;
		}
	}
	return S_OK ;
}

STDMETHODIMP CSerializerNode::ChildRef( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) 
{
	ATLTRACE(_T("CNodeSerializer::ChildRef %ls\n"), href ) ;
	CComPtr<ISOAPNodes> c ;
	m_node->get_Nodes(&c) ;
	c->Append(hrefNode) ;

	WaitItem w ;
	w.id = href ;
	w.node = hrefNode ;
	m_refs.push_back(w) ;

	return S_OK ;
}

STDMETHODIMP CSerializerNode::Characters( /*[in]*/ BSTR charData ) 
{
	CComVariant v ;
	v.vt = VT_BSTR ;
	v.bstrVal = charData ;
	m_node->put_Value(v) ;
	v.vt = VT_EMPTY ;

	return S_OK ;
}

STDMETHODIMP CSerializerNode::End()
{
	ATLTRACE(_T("CNodeSerializer::End() node=0x%x\n"), m_node ) ;
	m_node.Release() ;
	return S_OK ;
}
