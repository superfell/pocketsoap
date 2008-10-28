/*
The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations
under the License.

The Original Code is pocketXML-RPC

The Initial Developer of the Original Code is Simon Fell.
Portions created by Simon Fell are Copyright (C) 2002,2004
Simon Fell. All Rights Reserved.

Contributor(s):
*/

// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__8DAA4F83_2E79_4502_B452_E78B883D9888__INCLUDED_)
#define AFX_STDAFX_H__8DAA4F83_2E79_4502_B452_E78B883D9888__INCLUDED_

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

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <stack>
#include "..\..\3rdparty\dispimpl2.h"

#define _HR(x) { HRESULT _hr = x ; if (FAILED(_hr)) return _hr ; }

#include "..\..\3rdparty\expat\xmlparse\xmlparse.h"
#include "pocketHTTP.h"


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__8DAA4F83_2E79_4502_B452_E78B883D9888__INCLUDED)
