// AttachmentsEVC.h : main header file for the ATTACHMENTSEVC application
//

#if !defined(AFX_ATTACHMENTSEVC_H__0BE5A638_B378_4241_9D88_0F3789B8D2C5__INCLUDED_)
#define AFX_ATTACHMENTSEVC_H__0BE5A638_B378_4241_9D88_0F3789B8D2C5__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CAttachmentsEVCApp:
// See AttachmentsEVC.cpp for the implementation of this class
//

class CAttachmentsEVCApp : public CWinApp
{
public:
	CAttachmentsEVCApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttachmentsEVCApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAttachmentsEVCApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTACHMENTSEVC_H__0BE5A638_B378_4241_9D88_0F3789B8D2C5__INCLUDED_)
