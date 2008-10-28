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
Portions created by Simon Fell are Copyright (C) 2000,2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "psoap.h"
#include "Namespaces.h"
#include "tags.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
Namespaces::Namespaces() 
{
	// has to be upper case so there's no clash
	m_prefprefixes[SOAP_ENVELOPE_11_URI] = L"S" ;
	m_prefprefixes[SOAP_ENVELOPE_12_URI] = L"E" ;
	m_prefprefixes[SOAP_ENCODING_11_URI] = L"Enc" ;
	m_prefprefixes[SOAP_ENCODING_12_URI] = L"Q" ;
	m_prefprefixes[XSD01_URI]			 = L"XS" ;
	m_prefprefixes[XSI01_URI]			 = L"XI" ;
}

Namespaces::~Namespaces()
{
}

void Namespaces::Clear()
{
	m_namespaces.clear() ;
}

HRESULT Namespaces::Add( BSTR prefix, BSTR NamespaceURI)
{
	m_namespaces[NamespaceURI] = prefix ;
	return S_OK ;
}

CComBSTR Namespaces::ReverseLookup(const CComBSTR &prefix)
{
	for ( STRING_MAP::iterator i = m_namespaces.begin(); i != m_namespaces.end() ; i++ )
	{
		if ( bstrEqual ( prefix, i->second ) )
			return i->first ;
	}
	return NULL ;
}

HRESULT Namespaces::Remove	( BSTR prefix )
{
	CComBSTR k = ReverseLookup(prefix) ;
	m_namespaces.erase(k) ;
	return S_OK ;
}

HRESULT Namespaces::GetPrefixForURI( /*[in]*/ BSTR NamespaceURI, /*[in,out]*/ BSTR * prefix )
{
	if ( ! prefix ) return E_POINTER ;
	if ( SysStringLen(NamespaceURI) == 0 )
	{
		SysFreeString(*prefix) ;
		return S_OK ;
	}

	STRING_MAP::iterator pre = m_namespaces.find(NamespaceURI) ;
	if ( pre != m_namespaces.end() )
	{
		SysFreeString(*prefix) ;
		return pre->second.CopyTo(prefix) ;
	}
	
	bool bGenPrefix = true ;
	// try a user supplied prefix first
	if ( SysStringLen(*prefix) > 0 )
	{
		if ( ! IsCharUpperW ( (*prefix)[0] ))
			return E_UNEXPECTED ;

		CComBSTR uri ;
		HRESULT hr = GetURIForPrefix(*prefix, &uri ) ;
		if ( E_INVALIDARG == hr )
			bGenPrefix = false ;
	}
	// check to see if there's a prefered prefix
	if ( bGenPrefix )
	{
		pre = m_prefprefixes.find(NamespaceURI) ;
		if ( pre != m_prefprefixes.end() ) 
		{
			pre->second.CopyTo(prefix) ;
			bGenPrefix = false ;
		}
	}
	if ( bGenPrefix ) 
	{
		CComBSTR bns(m_ids.nextID().c_str()) ;
		if ( * prefix )
			SysFreeString(*prefix);
		*prefix = bns.Detach();
	}
	m_namespaces[NamespaceURI] = *prefix ;
	return S_OK ;
}

HRESULT Namespaces::GetURIForPrefix( /*[in]*/ BSTR prefix, /*[out,retval]*/ BSTR * NamespaceURI )
{
	CComBSTR uri = ReverseLookup(prefix) ;
	if ( uri )
		*NamespaceURI = uri.Detach() ;
	return SysStringLen(*NamespaceURI) ? S_OK : E_INVALIDARG ;
}

HRESULT Namespaces::SerializeNamespaces ( stringBuff_W &msg , const WCHAR * const sep )
{
	size_t sepLen = wcslen(sep) ;
	for ( STRING_MAP::const_iterator i = m_namespaces.begin() ; i != m_namespaces.end() ; i++ )
	{
		_HR(msg.Append ( sep, sepLen ));
		_HR(msg.Append ( L"xmlns:", 6 ));
		_HR(msg.Append ( i->second, i->second.Length() ));
		_HR(msg.Append ( L"=\"", 2 ));
		_HR(msg.Append ( i->first, i->first.Length() ));
		_HR(msg.Append ( L"\"", 1 ));
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////
// ISOAPNamespacesImpl
//////////////////////////////////////////////////////////////////////
void ISOAPNamespacesImpl::pushNamespace(const wchar_t * prefix, const wchar_t * uri) 
{
	m_namespaces[prefix].push(uri) ;
}

void ISOAPNamespacesImpl::popNamespace (const wchar_t * prefix) 
{
	STRING_MAP::iterator i = m_namespaces.find(prefix) ;
	if ( i != m_namespaces.end() )
	{
		i->second.pop() ;
		if ( i->second.empty() )
			m_namespaces.erase(i) ;
	}
}

STDMETHODIMP ISOAPNamespacesImpl::GetPrefixForURI( /*[in]*/ BSTR NamespaceURI, /*[out,retval]*/ BSTR * prefix ) 
{
	for ( STRING_MAP::iterator i = m_namespaces.begin() ; i != m_namespaces.end() ; i++ )
	{
		if ( wcscmp(i->second.top().c_str(), NamespaceURI) == 0 )
		{
			*prefix = SysAllocStringLen(i->first, i->first.Length()) ;
			return S_OK ;
		}
	}
	return E_INVALIDARG ;
}

STDMETHODIMP ISOAPNamespacesImpl::GetURIForPrefix( /*[in]*/ BSTR prefix,		/*[out,retval]*/ BSTR * NamespaceURI )
{
	STRING_MAP::iterator i = m_namespaces.find(prefix) ;
	if ( i != m_namespaces.end() )
	{
		*NamespaceURI = SysAllocStringLen(i->second.top().c_str(), i->second.top().Size()) ;
		return S_OK ;
	}
	return E_INVALIDARG ;
}
