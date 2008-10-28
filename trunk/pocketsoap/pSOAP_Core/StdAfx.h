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

#if !defined(AFX_STDAFX_H__63D2ED43_8DA3_4514_BAE5_0C3B767FC19E__INCLUDED_)
#define AFX_STDAFX_H__63D2ED43_8DA3_4514_BAE5_0C3B767FC19E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef STRICT
#define STRICT
#endif

#if _WIN32_WCE == 201
#error ATL is not supported for Palm-Size PC
#endif

#define _ATL_FREE_THREADED

#ifndef _WIN32_WCE
#define WINVER 0x0400
#define WIN32_SECURITY
#endif

// STLport def's
// #define __PLACEMENT_NEW_INLINE
#define _STLP_NO_EXCEPTIONS 
#ifndef _REENTRANT
#define _REENTRANT
#endif

#ifdef _DEBUG
//#define _ATL_DEBUG_INTERFACES
#endif 

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

#include <stdio.h>
#include <winsock.h>

#define XML_DTD
#define XML_NS
#define XML_UNICODE_WCHAR_T 
#define XML_UNICODE 
#include "xmltok.h"
#include "xmlparse.h"
#include "sfDispImpl.h"

#ifdef _WIN32_WCE
#include "pocketpc.h"
#endif

#include "hrtrace.h"

bool bstrEqual ( BSTR a, BSTR b ) ;

// stlport
#pragma warning ( disable : 4786 ) // truncated identifier
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <stack>
#include <list>
#include <memory>

// horible hack here, some eVC/STLPort issue with isalpha
#ifndef isalpha
#define isalpha(_c)      ( _isctype(_c,_ALPHA) )
#endif

// there is no winsock2 on pocketPC, but nothing defines SD_SEND :(
// this is taken from the win32 winsock2 file [line 969]
/*
 * WinSock 2 extension -- manifest constants for shutdown()
 */
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

// PocketHTTP
namespace PocketHTTP
{
#include "..\..\pocketHTTP\win32\pocketHTTP.h"
}

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__63D2ED43_8DA3_4514_BAE5_0C3B767FC19E__INCLUDED)
