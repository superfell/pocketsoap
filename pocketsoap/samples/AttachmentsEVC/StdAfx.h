// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__7FAE96C9_D185_4D64_B098_70324B8716D2__INCLUDED_)
#define AFX_STDAFX_H__7FAE96C9_D185_4D64_B098_70324B8716D2__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000



#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#if defined(_AFXDLL)
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <atlbase.h>
#include "..\..\attachments_ppc\attachments.h" 


#define _HR(x) { HRESULT _hr = x ; if (FAILED(_hr)) { TCHAR msg[500] ; wsprintf(msg,_T("Failed 0x%x at ") _T(#x) _T(""), _hr ) ; MessageBox( msg,_T("PocketSOAP") ) ; return ; } } 


//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_STDAFX_H__7FAE96C9_D185_4D64_B098_70324B8716D2__INCLUDED_)
