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
#include "header.h"
#include "stringHelpers.h"

#define IS_VALID if ( ! m_valid ) return AtlReportError(CLSID_CoPocketHTTP, OLESTR("This header is no longer valid, its been deleted"), IID_NULL, E_UNEXPECTED ) ;

Header::Header() : 
	m_headers(0),
	m_valid(false)
{
}

Header::~Header()
{
	m_hostage.Release() ;
	m_tokens.clear() ;
}

void Header::Init ( IUnknown * container, Headers * headers, Headers::iterator &theHeader )
{
	m_hostage = container ;
	m_headers = headers ;
	m_header  = theHeader ;
	m_tokens.clear() ;
	m_valid = true ;
}

STDMETHODIMP Header::get_Value ( BSTR *header ) 
{
	USES_CONVERSION ;
	IS_VALID ;
	if ( !header ) return E_POINTER ;
	*header = A2BSTR(m_header->second.c_str()) ;
	return S_OK ;
}

STDMETHODIMP Header::put_Value ( BSTR  header ) 
{
	USES_CONVERSION ;
	IS_VALID ;
	m_header->second = OLE2A(header) ;
	m_tokens.clear() ;
	return S_OK ;
}


STDMETHODIMP Header::get_Name ( BSTR * name )
{
	USES_CONVERSION ;
	IS_VALID ;
	if ( !name ) return E_POINTER ;
	*name = A2BSTR(m_header->first.c_str()) ;
	return S_OK ;
}

STDMETHODIMP Header::put_Name ( BSTR name ) 
{
	USES_CONVERSION ;
	IS_VALID ;
	std::string val = m_header->second ;
	Headers::iterator newHeader = m_headers->insert ( Headers::value_type ( OLE2A(name), val ) ) ;
	m_headers->erase(m_header) ;
	m_header = newHeader ;
	
	return S_OK ;
}

STDMETHODIMP Header::get_AttributeCount( long * count ) 
{
	if ( ! count ) return E_POINTER ;
	IS_VALID ;
	if ( 0 == m_tokens.size()  )
		_HR(ParseAttributes()) ;
	
	*count = m_tokens.size() ;
	return S_OK ;
}

STDMETHODIMP Header::get_Attribute ( BSTR attributeName, BSTR * value ) 
{
	USES_CONVERSION ;
	IS_VALID ;

	if ( ! value ) return E_POINTER ;
	if ( 0 == m_tokens.size()  )
		_HR(ParseAttributes()) ;

	Attributes::iterator a = m_tokens.find(OLE2A(attributeName)) ;
	if ( a != m_tokens.end() )
	{
		*value = A2BSTR(a->second.c_str()) ;
		return S_OK;
	}

	return E_INVALIDARG ;
}

STDMETHODIMP Header::put_Attribute ( BSTR attributeName, BSTR newValue ) 
{
	USES_CONVERSION ;
	IS_VALID ;
	if ( 0 == m_tokens.size()  )
		_HR(ParseAttributes()) ;

	m_tokens[OLE2A(attributeName)] = OLE2A(newValue) ;

	// build the new value string
	static const std::string sep("; ") ;
	static const std::string equals("=") ;
	static const std::string quote("\"") ;
	std::string val ;
	Attributes::iterator a = m_tokens.find("") ;
	if ( a != m_tokens.end() )
		val += a->second ;

	for ( a = m_tokens.begin() ; a != m_tokens.end(); a++ )
	{
		if ( a->first.length() > 0 )
		{
			if ( a->second.length() > 0 )
			{
				if ( val.length() > 0 )
					val += sep ;
				val += a->first ;
				val += equals ;
				val += quote ;
				val += a->second ;
				val += quote ;
			}
		}
	}
	m_header->second = val ;
	return S_OK  ;
}

HRESULT Header::ParseAttributes()
{
	// header format is Content-Type: foo/bar; attr1=value1; [attrib2=value2[;]]
	// we get the string starting at  foo/bar

	Attributes tokens ;

	const char * ct = m_header->second.c_str() ;
	const char * start = ct ;
	const char * comma = strchr( ct, ';' ) ;

	std::string tknName ;
	std::string tknVal ;

	// no attributes,
	if ( ! comma )
		tokens.insert ( Attributes::value_type(tknName, start)) ;
	else
	{
		tknVal.append ( start, comma - start ) ;
		trimWhitespace(tknVal) ;
		trimQuotes(tknVal) ;
		tokens.insert ( Attributes::value_type(tknName, tknVal)) ;

		while ( *comma )
		{
			start = comma + 1 ;
			eatIgnorableChars ( start ) ;
			comma = strchr ( start, ';' ) ;
			if ( comma == NULL )
				comma = start + strlen(start) ;
			char * equals = strchr ( start, '=' ) ;
			tknName.erase() ;
			tknVal.erase() ;
			if ( equals < comma )
			{
				tknName.append ( start, equals - start ) ;	
				tknVal.append ( equals +1, comma - equals -1 ) ;	
			}
			else
				tknName.append ( start, comma - start ) ;

			trimWhitespace(tknName) ;
			trimWhitespace(tknVal) ;
			trimQuotes(tknVal) ;

			tokens.insert ( Attributes::value_type(tknName, tknVal)) ;
		}
	}
	m_tokens = tokens ;
	return S_OK ;
}

STDMETHODIMP Header::Delete()
{
	IS_VALID ;
	m_valid = false ;
	m_headers->erase(m_header) ;
	return S_OK;
}