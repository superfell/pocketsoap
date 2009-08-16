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

#pragma once

#include "stringBuff.h" 

HRESULT Ole2Utf8 ( BSTR src, stringBuff_A &utf8 ) ;

stringBuff_A & operator << ( stringBuff_A &d, const char * s )  ;
stringBuff_A & operator << ( stringBuff_A &d, stringBuff_A &s ) ;
stringBuff_W & operator << ( stringBuff_W &d, const wchar_t * s )  ;
stringBuff_W & operator << ( stringBuff_W &d, stringBuff_W   &s ) ;

void trimQuotes			( std::string &s ) ;
void trimWhitespace		( std::string &s ) ;
void eatIgnorableChars	( const char * &start ) ;

bool StartsWith ( LPOLESTR str, LPOLESTR startsWith ) ;
bool StartsWith ( LPCSTR   str,	LPCSTR   startsWith ) ;

// is a Linear white space character
bool IsLWS( const char * c ) ;
