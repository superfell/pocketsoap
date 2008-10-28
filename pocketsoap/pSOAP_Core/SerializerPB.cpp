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
#include "SerializerPB.h"

/////////////////////////////////////////////////////////////////////////////
// CSerializerPB
/////////////////////////////////////////////////////////////////////////////

CSerializerPB::CSerializerPB()
{
	ATLTRACE(_T("CSerializerPB::CSerializerPB\n")) ;
}

CSerializerPB::~CSerializerPB()
{
	ATLTRACE(_T("CSerializerPB::~CSerializerPB()\n") ) ;
}

STDMETHODIMP CSerializerPB::Read( LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog)
{
	CComBSTR n(pszPropName) ;
	CComPtr<ISOAPNodes> c ;
	m_node->get_Nodes(&c) ;
	CComPtr<ISOAPNode> cn ;
	HRESULT hr = c->get_ItemByName ( n, NULL, &cn ) ;
	if (SUCCEEDED(hr))
		cn->get_Value(pVar) ;
	return hr ;
}

STDMETHODIMP CSerializerPB::Write( LPCOLESTR pszPropName, VARIANT *pVar)
{
	return m_dest->SerializeValue ( pVar, CComBSTR(pszPropName), NULL ) ;
}

STDMETHODIMP CSerializerPB::Initialize( /*[in]*/ BSTR xmlType, /*[in]*/ BSTR xmlTypeNamespace, /*[in]*/ VARIANT comType ) 
{
	m_type = xmlType ;
	m_typeNS = xmlTypeNamespace ;
	CComVariant v ;
	v.ChangeType(VT_BSTR, &comType ) ;
	m_progID.Empty() ;
	m_progID.Attach ( v.bstrVal ) ;
	v.vt = VT_EMPTY ;
	m_node.Release() ;
	m_obj.Release() ;
	return S_OK ;
}

STDMETHODIMP CSerializerPB::Serialize( /*[in]*/ VARIANT *val, ISerializerContext * ctx, /*[in]*/ ISerializerOutput * dest )
{
	if ( m_type.Length() ) 
	{
		CComPtr<ISerializerFactory> sf ;
		ctx->get_SerializerFactory(&sf) ;
		CComBSTR xsi ;
		sf->XsiForPrimaryNS(&xsi) ;
		dest->QNameAttribute ( CComBSTR(OLESTR("type")), xsi, m_type, m_typeNS ) ;
	}

	CComPtr<IPersistPropertyBag> ppb ;
	HRESULT hr = val->punkVal->QueryInterface(__uuidof(ppb), (void **)&ppb);
	if (ppb)
	{
		m_dest = dest ;
		CComPtr<IPropertyBag> pb; 
		GetUnknown()->QueryInterface(__uuidof(pb), (void **)&pb) ;

		return ppb->Save( pb, FALSE, TRUE ) ;
	}
	return hr ;
}

// ** De-Serializer
STDMETHODIMP CSerializerPB::Start( /*[in]*/ ISOAPNode * node, /*[in]*/ BSTR ElementName, /*[in]*/ ISoapDeSerializerAttributes * Attributes, /*[in]*/ ISOAPNamespaces * ns ) 
{
	ATLTRACE(_T("CSerializerPB::ParseInit node=0x%x\n"), node ) ;
	m_node = node ;
	m_node->put_Type(m_type) ;
	m_node->put_TypeNS(m_typeNS) ;
	return m_obj.CoCreateInstance(m_progID) ;
}

STDMETHODIMP CSerializerPB::Child( long id, VARIANT_BOOL ready, /*[in]*/ ISOAPNode * childNode ) 
{
	ATLTRACE(_T("CSerializerPB::Child %d\n"), id ) ;
	CComPtr<ISOAPNodes> c ;
	m_node->get_Nodes(&c) ;
	c->Append(childNode) ;
	return S_OK ;
}

STDMETHODIMP CSerializerPB::ChildReady( long id, /*[in]*/ ISOAPNode * childNode )
{
	ATLTRACE(_T("CSerializerPB::ChildReady %d\n"), id ) ;
	return S_OK ;
}

STDMETHODIMP CSerializerPB::Ref ( BSTR id, /*[in]*/ ISOAPNode * idNode ) 
{
	ATLTRACE(_T("CSerializerPB::Ref %ls\n"), id ) ;
	CComVariant v ;
	idNode->get_Value(&v) ;
	for ( int i = m_refs.size() -1 ; i >= 0 ; --i )
	{
		if ( m_refs[i].id == id )
			m_refs[i].node->put_Value(v) ;
	}
	return S_OK ;
}

STDMETHODIMP CSerializerPB::ChildRef( BSTR href, /*[in]*/ ISOAPNode * hrefNode ) 
{
	ATLTRACE(_T("CSerializerPB::ChildRef %ls\n"), href ) ;
	CComPtr<ISOAPNodes> c ;
	m_node->get_Nodes(&c) ;
	c->Append(hrefNode) ;

	WaitItem w ;
	w.id = href ;
	w.node = hrefNode ;
	m_refs.push_back(w) ;

	return S_OK ;
}

STDMETHODIMP CSerializerPB::Characters( /*[in]*/ BSTR charData ) 
{
	return S_OK ;
}

STDMETHODIMP CSerializerPB::End()
{
	ATLTRACE(_T("CSerializerPB::End() node=0x%x\n"), m_node ) ;

	CComQIPtr<IPersistPropertyBag> ppb (m_obj) ;
	if ( ppb )
	{
		_HR ( ppb->Load ( this, NULL ) ) ;

		CComQIPtr<IDispatch> d(ppb) ;
		if (d)
			m_node->put_Value ( CComVariant(d) ) ;
		else
			m_node->put_Value ( CComVariant(m_obj) ) ;

		CComPtr<ISOAPNodes> cn ;
		m_node->get_Nodes(&cn) ;
		cn->Clear() ;
	}

	// remember to reset ourselves, as we get pooled.
	m_node.Release() ;
	m_obj.Release() ;
	m_refs.clear() ;
	return S_OK ;
}
