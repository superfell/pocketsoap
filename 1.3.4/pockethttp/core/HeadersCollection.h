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

#pragma once

#include "stringBuff.h"
#include "headerHelpers.h"

class ATL_NO_VTABLE HeadersCollection :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IsfDelegatingDispImpl<IHeadersCollection2>
{
public:
	HeadersCollection();
	virtual ~HeadersCollection();

BEGIN_COM_MAP(HeadersCollection)
	COM_INTERFACE_ENTRY(IHeadersCollection2)
	COM_INTERFACE_ENTRY(IHeadersCollection)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// class
	void Init ( IUnknown * container, Headers *hdrs, bool takeOwnership = false ) ;

// IHeadersCollection
	STDMETHOD(get__NewEnum)	( IUnknown **pVal);
	STDMETHOD(get_Count)	( long * numHeaders ) ;
	STDMETHOD(get_Item)		( long index, IHeader ** ppHeader ) ;
	STDMETHOD(Find)			( BSTR name,  IHeader ** ppHeader ) ;
	STDMETHOD(Create)		( BSTR name,  BSTR value, IHeader ** pNewHeader ) ;

// IHeadersCollection2
	STDMETHOD(Delete)		( IHeader * pHeader ) ;

private:
	Headers				*m_headers ;
	CComPtr<IUnknown>	m_container ;
	bool				m_ownHeaders ;

	HRESULT	BuildHeader(Headers::iterator &h, IHeader **ppHeader) ;
};
