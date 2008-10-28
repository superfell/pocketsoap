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


#if !defined(AFX_NAMESPACES_H__3364C98A_5DA0_4D05_BFAC_08D1F47C9AA9__INCLUDED_)
#define AFX_NAMESPACES_H__3364C98A_5DA0_4D05_BFAC_08D1F47C9AA9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "idMgr.h"

class Namespaces  
{
public:
	Namespaces();
	virtual ~Namespaces();

	void Clear() ;
	HRESULT Add		( BSTR prefix, BSTR NamespaceURI ) ;
	HRESULT Remove	( BSTR prefix ) ;

	HRESULT GetPrefixForURI( BSTR NamespaceURI,	BSTR * prefix ) ;
	HRESULT GetURIForPrefix( BSTR prefix,		BSTR * NamespaceURI ) ;

	HRESULT SerializeNamespaces ( stringBuff_W &msg , const WCHAR * const sep ) ;

private:
	typedef std::map<CComBSTR, CComBSTR> STRING_MAP ;

	STRING_MAP	m_namespaces ;	// uri to prefix
	STRING_MAP	m_prefprefixes ; // uri to prefix : prefered prefixes for popular URI's
	idMgr		m_ids ;

	CComBSTR ReverseLookup(const CComBSTR &prefix) ;
};

// Com Wrapper for namespace management, used by psParser
// note that this no longer wraps the above code, as during de-serialization, you want
// the map to be the other way around
class ATL_NO_VTABLE ISOAPNamespacesImpl: 
	public CComObjectRootEx<CComMultiThreadModel>,
	public ISOAPNamespaces
{
public:

BEGIN_COM_MAP(ISOAPNamespacesImpl)
	COM_INTERFACE_ENTRY(ISOAPNamespaces)
END_COM_MAP()

// ISOAPNamespaces
	STDMETHOD(GetPrefixForURI)( /*[in]*/ BSTR NamespaceURI, /*[out,retval]*/ BSTR * prefix ) ;
	STDMETHOD(GetURIForPrefix)( /*[in]*/ BSTR prefix,		/*[out,retval]*/ BSTR * NamespaceURI ) ;

	typedef std::stack<stringBuff_W> STRINGS ;			// a stack of URI's
	typedef std::map<CComBSTR, STRINGS> STRING_MAP ;	// prefix -> stack of URI's

// class methods, called by psParser to manage active namespaces
	void pushNamespace(const wchar_t * prefix, const wchar_t * uri) ;
	void popNamespace (const wchar_t * prefix) ;

private:
	STRING_MAP m_namespaces ;
} ;

#endif // !defined(AFX_NAMESPACES_H__3364C98A_5DA0_4D05_BFAC_08D1F47C9AA9__INCLUDED_)
