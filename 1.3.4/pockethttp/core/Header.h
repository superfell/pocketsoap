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

#include "headerHelpers.h"

class ATL_NO_VTABLE Header :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IsfDelegatingDispImpl<IHeader2>
{
public:
	Header();
	virtual ~Header() ;

BEGIN_COM_MAP(Header)
	COM_INTERFACE_ENTRY(IHeader)
	COM_INTERFACE_ENTRY(IHeader2)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	void Init ( IUnknown * container, Headers * headers, Headers::iterator &theHeader ) ;

// IHeader
	// this represents a single http header
	STDMETHOD(get_Value) ( BSTR *header ) ;
	STDMETHOD(put_Value) ( BSTR  header ) ;
	
	STDMETHOD(get_Name)  ( BSTR *name ) ;
	STDMETHOD(put_Name)  ( BSTR  name ) ;

	// this lets you work with headers that have attributes, e.g.
	// content-type: text/xml; charset=utf-8
	// this would return an attributecount of 2
	// attribute("") returning text/xml 
	// attriubte("charset") returning utf-8 
	STDMETHOD(get_AttributeCount) ( long * count ) ;
	STDMETHOD(get_Attribute) ( BSTR attributeName, BSTR *value   ) ;
	STDMETHOD(put_Attribute) ( BSTR attributeName, BSTR newValue ) ;

// IHeader2
	STDMETHOD(Delete)() ;

private:
	HRESULT ParseAttributes() ;

	// typedef Headers Attributes ;
	typedef std::map<std::string, std::string, ciStringCompare> Attributes ;

	CComPtr<IUnknown>	m_hostage ;
	Headers				*m_headers;
	Headers::iterator	m_header ;
	Attributes			m_tokens ;
	bool				m_valid ;
};