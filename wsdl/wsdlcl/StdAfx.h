// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__0F5B522F_38F4_4FDA_A245_A5D6B852EB17__INCLUDED_)
#define AFX_STDAFX_H__0F5B522F_38F4_4FDA_A245_A5D6B852EB17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <atlbase.h>

//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;

#import "..\codegen\wsdlCodeGen.dll" no_namespace named_guids raw_interfaces_only 
#import "..\wsdlParser\wsdlParser.dll" no_namespace named_guids rename("namespace", "ns") raw_interfaces_only 
#import "C:\soap_src\SimonFell\pocketsoap\samples\mapEx\Release\map.dll" raw_interfaces_only named_guids no_namespace
#import "C:\WINDOWS\System32\msvbvm60.dll" raw_interfaces_only no_namespace named_guids rename("EOF", "EndOfFile") rename("RGB", "rgb")


#include "enum_iterator.h"

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0F5B522F_38F4_4FDA_A245_A5D6B852EB17__INCLUDED_)
