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
Portions created by Simon Fell are Copyright (C) 2002-2003
Simon Fell. All Rights Reserved.

Contributor(s):
*/

#include "stdafx.h"
#include "stringHelpers.h"
#include "ConvertUTF.h"

HRESULT Ole2Utf8 ( BSTR src, stringBuff_A &utf8 ) 
{
	size_t envLen = SysStringLen(src) ;
	utf8.Size(envLen) ;

	// chunk over the source, doing a Utf8 Encode
	const UTF16 *srcStart = src, *srcEnd = src + envLen;
	char *dest = utf8.buffer(), *destEnd = utf8.buffer() + utf8.Size();
	DWORD pos;
	ConversionResult cr;
	do
	{
		cr = ConvertUTF16toUTF8 ( &srcStart, srcEnd, (UTF8**)&dest, (UTF8*)destEnd, lenientConversion ) ;
		if(targetExhausted == cr)
		{
			pos = dest - utf8.buffer();
			utf8.Size(utf8.Size() *2);
			dest = utf8.buffer() + pos;
			destEnd = utf8.buffer() + utf8.Size();
		}
		if(sourceIllegal == cr)
			return E_UNEXPECTED;
	} while ( conversionOK != cr ) ;
	// now set the size to the actual converted size
	utf8.Size(dest-utf8.buffer());
	return S_OK ;
}

stringBuff_A & operator << ( stringBuff_A &d, const char * s )
{
	d.Append(s);
	return d ;
}


stringBuff_A & operator << ( stringBuff_A &d, stringBuff_A &s )
{
	d.Append(s) ;
	return d ;
}

stringBuff_W & operator << ( stringBuff_W &d, const wchar_t * s )  
{
	d.Append(s) ;
	return d ;
}

stringBuff_W & operator << ( stringBuff_W &d, stringBuff_W   &s ) 
{
	d.Append(s) ;
	return d ;
}

// is a Linear white space character
bool IsLWS( const char * c )
{
	return ( *c == ' ' || *c == '\t' ) ;
}

void eatIgnorableChars ( const char * &start ) 
{
	while ( IsLWS(start) )
		++start ;
}

void trimWhitespace ( std::string & s )
{
	while ( s[0] == ' ' || s[0] == '\t' )
		s.erase(0, 1 ) ;
	while ( s[s.size()-1] == ' ' || s[s.size()-1] == '\t' )
		s.erase(s.size()-1, 1 ) ;
}

void trimQuotes ( std::string &s )
{
	if ( ( s[0] == '\'' && s[s.size()-1] == '\'' ) || ( s[0] == '\"' && s[s.size()-1] == '\"' ) )
	{
		s.erase (0,1) ;
		s.erase ( s.size()-1,1 ) ;
	}
}

bool StartsWith ( LPOLESTR str, LPOLESTR startsWith )
{
	return ( wcsncmp ( str, startsWith, wcslen(startsWith) ) == 0 );
}

bool StartsWith ( LPCSTR str, LPCSTR startsWith )
{
	return ( strncmp ( str, startsWith, strlen(startsWith) ) == 0 ) ;
}