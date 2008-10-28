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

// AttachmentsCollection.cpp : Implementation of CAttachmentsCollection
#include "stdafx.h"
#include "Attachments.h"
#include "AttachmentsCollection.h"
#include "Attachment.h"

/////////////////////////////////////////////////////////////////////////////
// CAttachmentsCollection

class _ItfCopy
{
public:
	static HRESULT copy(VARIANT *dest, CAdapt<CComPtr<ISoapAttachment> > *src)
	{
		// variantInit only sets the VT, not the rest, 
		// so the assert in CComPtr::QI gets triggered
		dest->pdispVal = 0 ;	
		dest->vt = VT_DISPATCH ;
		return src->m_T.QueryInterface(&dest->pdispVal) ;
	}
	static void init(VARIANT *p) { VariantInit(p) ; }
	static void destroy(VARIANT *p) { VariantClear(p) ; }
} ;

STDMETHODIMP CAttachmentsCollection::get__NewEnum( /*[out, retval]*/ IUnknown **pVal)
{
	typedef CComObject<CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _ItfCopy, COL> > ENUM ;
	ENUM * c = 0 ;
	HRESULT hr = ENUM::CreateInstance(&c) ;
	if (FAILED(hr)) return hr ;
	c->AddRef() ;
	hr = c->Init ( GetUnknown(), m_col ) ;
	if (SUCCEEDED(hr))
		hr = c->QueryInterface(pVal) ;
	c->Release() ;
	return hr ;
}

STDMETHODIMP CAttachmentsCollection::get_Count( /*[out,retval]*/ long * Count ) 
{
	if ( ! Count ) return E_POINTER ;
	*Count = m_col.size() ;
	return S_OK ;
}

STDMETHODIMP CAttachmentsCollection::get_Item( /*[in]*/ long idx,  /*[out,retval]*/ ISoapAttachment ** attachment ) 
{
	if ( ! attachment ) return E_POINTER ;
	*attachment =0 ;
	if ( idx < 0 || idx >= (long)m_col.size() )
		return E_INVALIDARG ;

	return m_col[idx].m_T.CopyTo(attachment) ;
}

STDMETHODIMP CAttachmentsCollection::Find( /*[in]*/ BSTR uri,  /*[out,retval]*/ ISoapAttachment ** attachment ) 
{
	//todo: relative URI's nightmare
	if ( ! attachment ) return E_POINTER ;
	*attachment = 0 ;

	CComBSTR find ;
	bool searchCid ;
	if ( wcschr ( uri, ':' ) )
	{
		find = uri ;
		searchCid = false ;
	}
	else
	{
		searchCid = true ;
		if ( *uri == '<' )
			find.Attach ( SysAllocStringLen ( uri + 1, SysStringLen(uri)-2) ) ;
		else
			find = uri ;
	}

	CComBSTR item ;
	for ( COL::iterator i = m_col.begin() ; i != m_col.end() ; i++ )
	{
		if ( searchCid )
			i->m_T->get_ContentId(&item) ;
		else
			i->m_T->get_Uri(&item) ;

		if ( wcscmp ( item, find ) == 0 )
			return i->m_T.CopyTo(attachment) ;
		item.Empty();
	}
	return E_INVALIDARG ;
}

STDMETHODIMP CAttachmentsCollection::Append ( /*[in]*/ ISoapAttachment * newNode)
{
	if ( ! newNode ) return E_POINTER ;
	CAdapt<CComPtr<ISoapAttachment> > p(newNode) ;
	m_col.push_back(p) ;
	return S_OK ;
}

STDMETHODIMP CAttachmentsCollection::Clear()
{
	m_col.clear() ;
	return S_OK ;
}

STDMETHODIMP CAttachmentsCollection::Create(VARIANT src, TypeNameFormat tnf, BSTR contentType, ISoapAttachment ** attachment ) 
{
	CComObject<CAttachment> * a = 0 ;
	HRESULT hr = a->CreateInstance(&a) ;
	if (FAILED(hr)) return hr ;
	a->AddRef() ;
	a->QueryInterface(attachment) ;
	a->Initialize( src, tnf, contentType ) ;
	a->put_Format(m_format) ;
	Append(a) ;
	a->Release() ;
	
	return S_OK ;
}

STDMETHODIMP CAttachmentsCollection::get_Format(AttachmentFormat *pVal)
{
	if ( ! pVal ) return E_POINTER ;
	*pVal = m_format ;
	return S_OK;
}

STDMETHODIMP CAttachmentsCollection::put_Format(AttachmentFormat newVal)
{
	m_format = newVal ;

	for ( COL::iterator i = m_col.begin(); i != m_col.end() ; i++ )
		i->m_T->put_Format(newVal) ;

	return S_OK;
}