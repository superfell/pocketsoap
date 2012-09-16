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
Portions created by Simon Fell are Copyright (C) 2006
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "pocketHTTP.h"
#include "cookie.h"
#include "HTTPTransport.h"

Cookie::Cookie(BSTR bstrName, BSTR bstrVal, BSTR bstrPath, BSTR bstrDomain)
{
	USES_CONVERSION;
	name.Append(OLE2A(bstrName));
	val.Append(OLE2A(bstrVal));
	path.Append(OLE2A(bstrPath));
	domain.Append(OLE2A(bstrDomain));
}

void CookieWrapper::Init (const Cookie *cookie)
{
	m_name.Attach(A2BSTR(cookie->name.c_str()));
	m_value.Attach(A2BSTR(cookie->val.c_str()));
	m_path.Attach(A2BSTR(cookie->path.c_str()));
	m_domain.Attach(A2BSTR(cookie->domain.c_str()));
}

STDMETHODIMP CookieWrapper::get_Name(BSTR *pValue)
{
	return m_name.CopyTo(pValue);
}

STDMETHODIMP CookieWrapper::get_Value(BSTR *pValue)
{
	return m_value.CopyTo(pValue);
}

STDMETHODIMP CookieWrapper::get_Path( BSTR *pValue)
{
	return m_path.CopyTo(pValue);
}

STDMETHODIMP CookieWrapper::get_Domain(BSTR *pValue)
{
	return m_domain.CopyTo(pValue);
}

CookieCollection::CookieCollection() : m_transport(0)
{
}

CookieCollection::~CookieCollection()
{
	if (m_transport) m_transport->Release();
}

void CookieCollection::Init (CHTTPTransport * transport)
{
	m_transport = transport;
	m_transport->AddRef();
}

HRESULT makeCookieWrapper(Cookie *c, ICookie ** ppCookie)
{
	CComObject<CookieWrapper> * pc = 0 ;
	_HR (pc->CreateInstance(&pc)) ;
	pc->AddRef() ;
	pc->Init(c) ;
	HRESULT hr = pc->QueryInterface(ppCookie) ;
	pc->Release() ;
	return hr ;
}

STDMETHODIMP CookieCollection::get_Count( /*[out,retval]*/ long * numCookies )
{
	if (!numCookies) return E_POINTER;
	*numCookies = m_transport->m_cookies.size();
	return S_OK;
}

STDMETHODIMP CookieCollection::get_Item( /*[in]*/ long index, /*[out,retval]*/ ICookie ** ppCookie )
{
	if (!ppCookie) return E_POINTER;
	*ppCookie = 0;
	if (index < 0 || index >= (long)m_transport->m_cookies.size()) return E_INVALIDARG;
	return makeCookieWrapper(&m_transport->m_cookies[index], ppCookie);
}

bool ok(BSTR v)
{
	return (v != NULL) && (SysStringLen(v) > 0);
}

STDMETHODIMP CookieCollection::SetCookie( /*[in]*/ BSTR name, /*[in]*/ BSTR value, /*[in]*/ BSTR path, /*[in]*/ BSTR domain)
{
	if (!ok(name))   return AtlReportError(CLSID_CoPocketHTTP, OLESTR("Must specify a cookie name"), IID_ICookieCollection, E_INVALIDARG);
	if (!ok(value))  return AtlReportError(CLSID_CoPocketHTTP, OLESTR("Must specify a cookie value"), IID_ICookieCollection, E_INVALIDARG);
	if (!ok(path))   return AtlReportError(CLSID_CoPocketHTTP, OLESTR("Must specify a cookie path"), IID_ICookieCollection, E_INVALIDARG);
	if (!ok(domain)) return AtlReportError(CLSID_CoPocketHTTP, OLESTR("Must specify a cookie domain"), IID_ICookieCollection, E_INVALIDARG);
	return SetCookieImpl(name, value, path, domain);
}

HRESULT CookieCollection::SetCookie(CComPtr<ICookie> &c)
{
	CComBSTR n, v, p, d;
	c->get_Name(&n);
	c->get_Value(&v);
	c->get_Path(&p);
	c->get_Domain(&d);
	return SetCookieImpl(n, v, p, d);
}

HRESULT CookieCollection::SetCookieImpl( /*[in]*/ BSTR name, /*[in]*/ BSTR value, /*[in]*/ BSTR path, /*[in]*/ BSTR domain)
{
	Cookie c(name, value, path, domain);
	m_transport->AddOrSetCookie(c);
	return S_OK;
}

STDMETHODIMP CookieCollection::get__NewEnum(/*[out, retval]*/ IUnknown **pVal)
{
	if ( ! pVal ) return E_POINTER ;
	VARIANT * vals = new VARIANT[m_transport->m_cookies.size()] ;
	VARIANT * next = vals ;
	ICookie * ph ;
	COOKIES::iterator h = m_transport->m_cookies.begin() ;
	while ( h != m_transport->m_cookies.end() )
	{
		makeCookieWrapper( h, &ph ) ;
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
	e->Release();
	return S_OK;
}

STDMETHODIMP CookieCollection::Copy(/*[in]*/ ICookieCollection * cookies)
{
	CComPtr<ICookie> c;
	long max;
	_HR(cookies->get_Count(&max));
	for (long i = 0; i < max; i++) {
		_HR(cookies->get_Item(i, &c));
		_HR(SetCookie(c));
		c.Release();
	}
	return S_OK;
}