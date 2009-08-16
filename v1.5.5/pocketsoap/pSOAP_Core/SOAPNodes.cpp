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
#include "psoap.h"
#include "SOAPNodes.h"
#include "SOAPNode.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CNodes::CNodes() : m_parent(0)
{
	ATLTRACE(_T("CNodes::CNodes\n")) ;
}

CNodes::~CNodes()
{
	ATLTRACE(_T("~CNodes::CNodes\n")) ;
	Clear() ;
}

STDMETHODIMP CNodes::get__NewEnum ( /*[out, retval]*/ IUnknown **pVal)
{
	typedef CComObject<CComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT> > > ENUM ;
	ENUM * e = 0 ;

	HRESULT hr = ENUM::CreateInstance(&e) ;
	if (FAILED(hr)) return hr ;

	VARIANT * pvars = new VARIANT[m_nodes.size()] ;
	for ( unsigned int i = 0 ; i < m_nodes.size() ; ++i )
	{
		VariantInit(&pvars[i]) ;
		pvars[i].vt = VT_DISPATCH ;
		m_nodes[i]->QueryInterface(IID_IDispatch, (void **)&pvars[i].pdispVal) ;
	}

	e->AddRef() ;
	e->Init ( pvars, &pvars[m_nodes.size()], GetUnknown(), AtlFlagTakeOwnership ) ;
	hr = e->QueryInterface(pVal) ;
	e->Release() ;

	return hr ;
}

STDMETHODIMP CNodes::get_Item( /*[in]*/ long idx,  /*[out,retval]*/ ISOAPNode ** node )
{
	if ( ! node ) return E_POINTER ;
	if ( idx >= 0 && idx < (long)m_nodes.size() )
	{
		*node = m_nodes[idx] ;
		(*node)->AddRef() ;
		return S_OK ;
	}
	return E_INVALIDARG ;
}

STDMETHODIMP CNodes::get_ItemByName	( /*[in]*/ BSTR Name, /*[in]*/ BSTR Namespace, /*[out,retval]*/ ISOAPNode ** node )
{
	if ( ! node ) return E_POINTER ;
	CComBSTR n , ns ;
	for ( unsigned int i = 0 ; i < m_nodes.size() ; ++i )
	{
		m_nodes[i]->get_Name(&n) ;
		if ( n == Name )
		{
			if ( 0 == SysStringLen(Namespace) )
				return get_Item(i, node) ;

			m_nodes[i]->get_Namespace(&ns) ;
			if ( ns == Namespace )
				return get_Item(i, node) ;
			ns.Empty() ;
		}
		n.Empty() ;
	}
	return E_INVALIDARG ;
}

STDMETHODIMP CNodes::get_Count		( /*[out,retval]*/ long * pCount )
{
	*pCount = m_nodes.size() ;
	return S_OK ;
}

STDMETHODIMP CNodes::Append			( /*[in]*/ ISOAPNode * newNode)
{
	newNode->AddRef() ;
	m_nodes.push_back(newNode) ;
	return S_OK;
}

STDMETHODIMP CNodes::Clear()
{
	ObjectLock lock(this) ;
	for ( unsigned int i = 0 ; i < m_nodes.size() ; ++i )
	{
		IUnknown * p = m_nodes[i] ;
		if ( p )
			p->Release() ;
		else
			ATLTRACE("Hmmm, NULL Param pointer in nodes collection, idx=%d, size=%d\n", i, m_nodes.size() ) ;
	}

	m_nodes.clear() ;
	return S_OK;
}

STDMETHODIMP CNodes::Create(BSTR Name, VARIANT Val, BSTR Namespace, BSTR Type, BSTR TypeNamespace, ISOAPNode ** newNode )
{
	CComPtr<ISOAPNode2> n ;
	CComObject<CSOAPNode> * pNode = 0 ;
	HRESULT hr = pNode->CreateInstance(&pNode) ;
	if (FAILED(hr)) return hr ;
	pNode->AddRef() ;
	pNode->QueryInterface(&n) ;
	pNode->Release() ;

	n->put_Name(Name) ;
	n->put_Namespace(Namespace);
	n->put_Value(Val) ;
	n->put_Type(Type) ;
	n->put_TypeNS(TypeNamespace) ;
	if ( m_parent )
	{
		CComBSTR enc ;
		m_parent->get_EncodingStyle(&enc) ;
		n->put_EncodingStyle(enc) ;
	}

	Append(n) ;
	if ( newNode )
		*newNode = n.Detach() ;

	return S_OK ;
}

void CNodes::SetParent(CSOAPNode * p )
{
	m_parent = p ;
}