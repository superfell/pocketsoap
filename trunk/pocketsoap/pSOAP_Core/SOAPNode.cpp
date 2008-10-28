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
#include "pSOAP.h"
#include "SOAPNode.h"
#include "SOAPNodes.h"
#include "tags.h"

static long node_active_count = 0 ;
static long node_total_count  = 0 ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSOAPNode::CSOAPNode() : 
	m_root(VARIANT_FALSE), 
	m_mustUnderstand(VARIANT_FALSE), 
	m_nil(VARIANT_FALSE), 
	m_relay(VARIANT_FALSE), 
	m_explicitRoot(VARIANT_FALSE), 
	m_encStyle(SOAP_ENCODING_11_URI)
{
#ifdef _DEBUG
	InterlockedIncrement(&node_active_count) ;
	InterlockedIncrement(&node_total_count) ;
#endif
}

CSOAPNode::~CSOAPNode()
{
#ifdef _DEBUG
	InterlockedDecrement(&node_active_count) ;
#endif
}

HRESULT CSOAPNode::FinalConstruct()
{
	return S_OK ;
}

void CSOAPNode::FinalRelease()
{
}

void CSOAPNode::DumpActivityStats()
{
	ATLTRACE(_T("CSOAPNode \t%d total created\t %d active\n"), node_total_count, node_active_count ) ;
}

HRESULT CopyVal(CComBSTR &src, BSTR *dest)
{
	return src.CopyTo(dest) ;
}

HRESULT CopyVal(VARIANT_BOOL &src, VARIANT_BOOL *dest)
{
	if ( ! dest ) return E_POINTER ;
	*dest = src ;
	return S_OK ;
}

HRESULT CopyVal(CComVariant &src, VARIANT *val)
{
	if ( ! val ) return E_POINTER ;
	// remember to initialize the destination before calling VariantCopy
	// as reported by Ivan Goychev 3/18/2
	VariantInit(val) ;	
	return VariantCopy ( val, &src ) ;
}
	
STDMETHODIMP CSOAPNode::get_Name		( /*[out, retval]*/ BSTR *pVal)
{
	return CopyVal(m_Name,pVal) ;
}

STDMETHODIMP CSOAPNode::get_Namespace	( /*[out, retval]*/ BSTR *pVal)
{
	return CopyVal(m_NameNS,pVal) ;
}

STDMETHODIMP CSOAPNode::put_Name		( /*[in]*/ BSTR newVal)
{
	m_Name = newVal ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::put_Namespace	( /*[in]*/ BSTR newVal)
{
	m_NameNS = newVal ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::get_Type	(  /*[out, retval]*/ BSTR *pVal)
{
	return CopyVal(m_Type,pVal) ;
}

STDMETHODIMP CSOAPNode::get_TypeNS	(  /*[out, retval]*/ BSTR *pVal)
{
	return CopyVal(m_TypeNS,pVal) ;
}

STDMETHODIMP CSOAPNode::put_Type	(  /*[in]*/ BSTR newVal)
{
	m_Type = newVal ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::put_TypeNS	(  /*[in]*/ BSTR newVal)
{
	m_TypeNS = newVal ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::put_ArrayType( BSTR newVal )
{
	m_arrayType = newVal ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::get_ArrayType( BSTR *pVal  )
{
	return CopyVal(m_arrayType,pVal) ;
}

STDMETHODIMP CSOAPNode::put_Value ( /*[in]*/ VARIANT newVal)
{
	m_val.Clear() ;
	return VariantCopyInd ( &m_val, &newVal ) ;
}

STDMETHODIMP CSOAPNode::get_Value ( /*[out, retval]*/ VARIANT *pVal)
{
	// when we have sub items, the value of the node
	// should be ourself
	long subCount = 0 ;
	if ( m_nodes )
	{
		m_nodes->get_Count(&subCount) ;
		if ( subCount > 0 )
		{
			VariantInit(pVal) ;
			pVal->vt = VT_DISPATCH ;
			return GetUnknown()->QueryInterface(IID_IDispatch, (void **)&pVal->pdispVal) ;
		}
	}
	return CopyVal(m_val,pVal) ;
}

STDMETHODIMP CSOAPNode::get_ValueAs( /*[in]*/ BSTR Type, /*[in]*/ BSTR TypeNS, /*[out, retval]*/ VARIANT *pVal )
{
	HRESULT hr = get_Value(pVal) ;
	if(FAILED(hr)) return hr ;

	// if the Xml types already match, we're done !
	if ( bstrEqual ( Type, m_Type ) && bstrEqual ( TypeNS, m_TypeNS ) ) 
		return S_OK ;

	// if they asked for anyType, we're done !
	VARIANT_BOOL res = VARIANT_FALSE ;
	m_serFactory->IsAnyType ( Type, TypeNS, &res ) ;
	if ( VARIANT_TRUE == res )
		return S_OK ;

	// findout what the result COM type would be, for the XmlType
	// if the resulting COM type matches the current COM type we're done !
	CComVariant vt ;
	m_serFactory->FindComType ( Type, TypeNS, &vt ) ;
	if ( vt.vt == VT_I4 && vt.lVal == pVal->vt )
		return S_OK ;

	// COM won't handle a type conversion from NULL to BSTR, but
	// its valid to have a NULL BSTR, so we do it ourselves.
	if ( ( pVal->vt == VT_NULL) && (vt.vt == VT_I4 && vt.lVal == VT_BSTR ) )
	{
		pVal->vt = VT_BSTR ;
		pVal->bstrVal = SysAllocString(L"") ;
		return S_OK ;
	}

	// basically there are two options at this point
	// either (i) we have some children, in which case we need to de-serialize it
	// as a complex typpe, or we have no children, and its just a basic
	// text conversion job
	CComPtr<ISoapDeSerializer> s ;
	_HR(m_serFactory->DeserializerForType ( Type, TypeNS, VARIANT_FALSE, &s )) 
	
	// todo, should provide some sort of dummy impl, for the last two params
	s->Start ( this, CComBSTR(L"element"), NULL, NULL ) ;

	long nc = 0 ;
	if ( m_nodes )
		m_nodes->get_Count(&nc) ;
	if ( nc == 0 )
	{
		hr = VariantChangeType(pVal, pVal, 0, VT_BSTR ) ;
		if (FAILED(hr)) return hr ;
		s->Characters ( pVal->bstrVal ) ;
	}
	else
	{
		CComPtr<ISOAPNode> cn ;
		for ( long idx = 0 ; idx < nc ; idx++ )
		{
			m_nodes->get_Item(idx, &cn) ;
			s->Child ( idx + 42, VARIANT_TRUE, cn ) ;
			cn.Release() ;
		}
		m_nodes->Clear();
	}
	s->End() ;
	
	// running the de-serializer updates the node, so re-get the value
	VariantClear(pVal) ;
	return get_Value(pVal) ;

}

STDMETHODIMP CSOAPNode::putref_SerializerFactory( /*[in]*/ ISerializerFactoryConfig * sf ) 
{
	m_serFactory.Release() ;
	return sf->QueryInterface(__uuidof(m_serFactory), (void **)&m_serFactory) ;
}

STDMETHODIMP CSOAPNode::get_root ( /*[out,retval]*/ VARIANT_BOOL * IsRoot ) 
{
	return CopyVal(m_root,IsRoot) ;
}

STDMETHODIMP CSOAPNode::get_id ( /*[out,retval]*/ BSTR * id ) 
{
	return CopyVal(m_id,id) ;
}

STDMETHODIMP CSOAPNode::get_href ( /*[out,retval]*/ BSTR * href ) 
{
	return CopyVal(m_href,href) ;
}

STDMETHODIMP CSOAPNode::get_ref ( BSTR * ref ) 
{
	return get_href(ref) ;
}

STDMETHODIMP CSOAPNode::get_actor ( /*[out,retval]*/ BSTR * actorURI ) 
{
	return CopyVal(m_actor,actorURI) ;
}

STDMETHODIMP CSOAPNode::get_mustUnderstand ( /*[out,retval]*/ VARIANT_BOOL * MustUnderstand ) 
{
	return CopyVal(m_mustUnderstand,MustUnderstand) ;
}

STDMETHODIMP CSOAPNode::get_offset( /*[out,retval]*/ BSTR * offset )
{
	return CopyVal(m_offset, offset) ;
}

STDMETHODIMP CSOAPNode::get_position( /*[out,retval]*/ BSTR * position )
{
	return CopyVal(m_position, position) ;
}

STDMETHODIMP CSOAPNode::get_nil ( /*[out,retval]*/ VARIANT_BOOL * IsNil ) 
{
	return CopyVal(m_nil, IsNil) ;
}

STDMETHODIMP CSOAPNode::put_root ( /*[in]*/ VARIANT_BOOL IsRoot ) 
{
	m_root = IsRoot ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::put_id ( /*[in]*/ BSTR id ) 
{
	m_id = id ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::put_href ( /*[in]*/ BSTR href ) 
{
	m_href = href ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::put_ref( BSTR ref ) 
{
	return put_href(ref) ;
}

STDMETHODIMP CSOAPNode::put_actor ( /*[in]*/ BSTR actorURI ) 
{
	m_actor = actorURI ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::put_mustUnderstand ( /*[in]*/ VARIANT_BOOL MustUnderstand ) 
{
	m_mustUnderstand= MustUnderstand ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::put_offset ( /*[in]*/ BSTR offset ) 
{
	m_offset = offset ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::put_position ( /*[in]*/ BSTR position ) 
{
	m_position= position ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::put_nil ( /*[in]*/ VARIANT_BOOL IsNil ) 
{
	m_nil = IsNil ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::get_Nodes ( /*[out,retval]*/ ISOAPNodes ** ppNodes ) 
{
	// bit convoluted, but we need to be able de-ref the id/hrefs seemlessly
	CComVariant v ;
	get_Value(&v) ;
	if ( v.vt == VT_UNKNOWN || v.vt == VT_DISPATCH )
	{
		CComPtr<IUnknown> n ;
		v.punkVal->QueryInterface(__uuidof(n), (void **)&n) ;
		if ( n != GetUnknown() )
		{
			CComQIPtr<ISOAPNode> sn(n) ;
			if ( sn )
				return sn->get_Nodes(ppNodes) ;
		}
	}

	if ( ! m_nodes )
	{
		CComObject<CNodes> * p ;
		HRESULT hr = p->CreateInstance(&p) ;
		if (FAILED(hr)) 
			return hr ;

		p->AddRef() ;
		hr = p->QueryInterface(&m_nodes) ;
		p->SetParent(this) ;
		p->Release() ;
	}
	return m_nodes.CopyTo(ppNodes) ;
}

STDMETHODIMP CSOAPNode::get_EncodingStyle( /*[out,retval]*/ BSTR * encStyle )
{
	return m_encStyle.CopyTo(encStyle) ;
}

STDMETHODIMP CSOAPNode::put_EncodingStyle( /*[in]*/ BSTR encStyle )
{
	m_encStyle = encStyle ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::get_explicitRoot( VARIANT_BOOL * explicitRoot )
{
	if ( ! explicitRoot ) return E_POINTER ;
	*explicitRoot = m_explicitRoot ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::put_explicitRoot( VARIANT_BOOL   explicitRoot ) 
{
	m_explicitRoot = explicitRoot ;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::get_relay ( /*[out,retval]*/ VARIANT_BOOL * relay ) 
{
	return CopyVal(m_relay,relay) ;
}

STDMETHODIMP CSOAPNode::put_relay ( /*[in]*/		 VARIANT_BOOL   relay ) 
{
	m_relay = relay;
	return S_OK ;
}

STDMETHODIMP CSOAPNode::get_role( BSTR * role )
{
	return get_actor(role) ;
}

STDMETHODIMP CSOAPNode::put_role( BSTR newRole ) 
{
	return put_actor(newRole) ;
}
