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
Portions created by Simon Fell are Copyright (C) 2003,2006
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#pragma once

#include "stringBuff.h"
class CHTTPTransport;

// this is the internal struct we hold cookie data in
class Cookie {
public:
	Cookie() {}
	Cookie(BSTR name, BSTR val, BSTR path, BSTR domain);

	stringBuff_A	name ;
	stringBuff_A	val ;
	stringBuff_A	path ;
	stringBuff_A	domain ;
};

typedef std::vector<Cookie> COOKIES ;


// if we need to expose it as a com object, we'll copy the data
// into one of these guys. We could of wrapped an existing Cookie
// instance rather than copying it, but then that complicates
// the lifetime management of the internal Cookie objects
// this shouldn't be too much of a hit
class ATL_NO_VTABLE CookieWrapper :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IsfDelegatingDispImpl<ICookie>
{
public:
	CookieWrapper() { }
	virtual ~CookieWrapper() { }

BEGIN_COM_MAP(CookieWrapper)
	COM_INTERFACE_ENTRY(ICookie)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	void Init (const Cookie *cookie) ;

// ICookie
	// this represents a single http cookie
	STDMETHOD(get_Name)  (BSTR *pValue ) ;
	STDMETHOD(get_Value) (BSTR *pValue ) ;
	
	STDMETHOD(get_Path)  ( BSTR *pValue ) ;
	STDMETHOD(get_Domain)( BSTR *pValue ) ;

private:
	CComBSTR m_name, m_value, m_path, m_domain;
};


class ATL_NO_VTABLE CookieCollection :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IsfDelegatingDispImpl<ICookieCollection>
{
public:
	CookieCollection();
	virtual ~CookieCollection() ;

BEGIN_COM_MAP(CookieCollection)
	COM_INTERFACE_ENTRY(ICookieCollection)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	void Init (CHTTPTransport * transport) ;

// ICookieCollection
	STDMETHOD(get__NewEnum)(/*[out, retval]*/ IUnknown **pVal);

	STDMETHOD(get_Count) ( /*[out,retval]*/ long * numCookies ) ;
	STDMETHOD(get_Item)  ( /*[in]*/ long index, /*[out,retval]*/ ICookie ** ppCookie ) ;

	STDMETHOD(SetCookie) ( /*[in]*/ BSTR name, /*[in]*/ BSTR Value, /*[in]*/ BSTR Path, /*[in]*/ BSTR Domain) ;
	STDMETHOD(Copy)(/*[in]*/ ICookieCollection * cookies);

private:
	CHTTPTransport * m_transport;

	HRESULT SetCookie(CComPtr<ICookie> &c);
	HRESULT SetCookieImpl( /*[in]*/ BSTR name, /*[in]*/ BSTR Value, /*[in]*/ BSTR Path, /*[in]*/ BSTR Domain) ;
};