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
Portions created by Simon Fell are Copyright (C) 2002
Simon Fell. All Rights Reserved.

Contributor(s):
*/

// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__873DBE2A_37C9_480B_A0CC_B70E5A975313__INCLUDED_)
#define AFX_STDAFX_H__873DBE2A_37C9_480B_A0CC_B70E5A975313__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef STRICT
#define STRICT
#endif

#if _WIN32_WCE == 201
#error ATL is not supported for Palm-Size PC
#endif

#define _ATL_FREE_THREADED
// #define _ATL_DEBUG_INTERFACES

#ifndef _WIN32_WCE
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#endif

// STLport def's
// #define __PLACEMENT_NEW_INLINE
#define _STLP_NO_EXCEPTIONS 
#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include "dispimpl2.h"

#include "psoap.h"

// stlport
#pragma warning ( disable : 4786 ) // truncated identifier
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <strstream>
#include <deque>

#include "..\..\common\stringBuff.h"
#include "..\..\common\sfDispImpl.h"
#include <winsock.h>

#define XML_UNICODE_WCHAR_T 
#define XML_UNICODE 
#include "xmltok.h"

#ifdef _WIN32_WCE
	#include "pocketpc.h"
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER (DWORD(-1))
#endif

#include "hrtrace.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__873DBE2A_37C9_480B_A0CC_B70E5A975313__INCLUDED_)
