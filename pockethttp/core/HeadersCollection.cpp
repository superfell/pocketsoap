/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketHTTP.

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "pocketHTTP.h"
#include "HeadersCollection.h"
#include "header.h"

HeadersCollection::HeadersCollection() : m_headers(0), m_ownHeaders(false)
{
}

HeadersCollection::~HeadersCollection()
{
	if ( m_ownHeaders && m_headers )
		delete m_headers ;
}

void HeadersCollection::Init ( IUnknown * container, Headers *headers, bool takeOwnership )
{
	m_container = container ;
	m_headers   = headers ;
	m_ownHeaders = takeOwnership ;
}

STDMETHODIMP HeadersCollection::get__NewEnum(IUnknown **pVal)
{
	if ( ! pVal ) return E_POINTER ;

	VARIANT * vals = new VARIANT[m_headers->size()] ;
	VARIANT * next = vals ;
	IHeader * ph ;
	Headers::iterator h = m_headers->begin() ;
	while ( h != m_headers->end() )
	{
		BuildHeader ( h, &ph ) ;
		next->vt = VT_DISPATCH ;
		ph->QueryInterface(IID_IDispatch, (void **)&next->pdispVal) ;
		ph->Release() ;
		h++ ;
		next++ ;
	}
	typedef CComObject< CComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT> > > ENUM ;
	ENUM * e = 0 ;
	_HR ( e->CreateInstance(&e) ) ;
	e->AddRef() ;
	e->Init ( vals, next, GetUnknown(), AtlFlagTakeOwnership ) ;
	e->QueryInterface(pVal) ;
	e->Release() ;
	return S_OK  ;
}

STDMETHODIMP HeadersCollection::get_Item(long index, IHeader **header ) 
{
	if (( index < 0 ) || ( index >= (long)m_headers->size() ))
		return E_INVALIDARG ;

	Headers::iterator h = m_headers->begin() ;
	while ( index-- > 0 )
		h++ ;

	return BuildHeader(h, header ) ;
}

STDMETHODIMP HeadersCollection::Create(BSTR name, BSTR val, IHeader ** ppHeader ) 
{
	USES_CONVERSION ;
	std::string key(OLE2A(name)) ;
	std::string v(OLE2A(val)) ;
	Headers::iterator h = m_headers->insert ( Headers::value_type ( key, v ) ) ;
	return BuildHeader ( h, ppHeader ) ;
}

STDMETHODIMP HeadersCollection::Find( BSTR name, IHeader ** ppHeader ) 
{
	USES_CONVERSION ;
	if ( !ppHeader ) return E_POINTER ;
	*ppHeader = 0 ;
	std::string key (OLE2A(name)) ;
	Headers::iterator h = m_headers->find(key) ;
	if ( h != m_headers->end() )
		return BuildHeader ( h, ppHeader ) ;

	return E_INVALIDARG ;
}

STDMETHODIMP HeadersCollection::get_Count( long * numHeaders ) 
{
	if ( ! numHeaders ) return E_POINTER ;
	*numHeaders = m_headers->size() ;
	return S_OK ;
}

HRESULT	HeadersCollection::BuildHeader(Headers::iterator &h, IHeader **ppHeader) 
{
	CComObject<Header> * ph = 0 ;
	_HR ( ph->CreateInstance(&ph) ) ;
	ph->AddRef() ;
	ph->Init ( GetUnknown(), m_headers, h ) ;
	HRESULT hr = ph->QueryInterface(ppHeader) ;
	ph->Release() ;
	return hr ;
}

STDMETHODIMP HeadersCollection::Delete ( IHeader * pHeader ) 
{
	return S_OK ;
}
