// AttachmentsEVCDlg.h : header file
//

#if !defined(AFX_ATTACHMENTSEVCDLG_H__E6F3B8A9_0F33_436F_8F15_3A244DC641C0__INCLUDED_)
#define AFX_ATTACHMENTSEVCDLG_H__E6F3B8A9_0F33_436F_8F15_3A244DC641C0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CAttachmentsEVCDlg dialog

class CAttachmentsEVCDlg : public CDialog
{
// Construction
public:
	CAttachmentsEVCDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAttachmentsEVCDlg)
	enum { IDD = IDD_ATTACHMENTSEVC_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttachmentsEVCDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAttachmentsEVCDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEchoMime();
	afx_msg void OnEchoDime();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	CComVariant SelectedFileAsVariant();
	void LogMsg(WCHAR *msg);
	void LogMsg(WCHAR *msg, WCHAR *msgPart2);
	void DumpAttInfo(CComPtr<ISoapAttachment> &att);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft eMbedded Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTACHMENTSEVCDLG_H__E6F3B8A9_0F33_436F_8F15_3A244DC641C0__INCLUDED_)
