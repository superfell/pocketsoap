// AttachmentsEVCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AttachmentsEVC.h"
#include "AttachmentsEVCDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAttachmentsEVCDlg dialog

CAttachmentsEVCDlg::CAttachmentsEVCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAttachmentsEVCDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAttachmentsEVCDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAttachmentsEVCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAttachmentsEVCDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAttachmentsEVCDlg, CDialog)
	//{{AFX_MSG_MAP(CAttachmentsEVCDlg)
	ON_BN_CLICKED(IDC_ECHO_MIME, OnEchoMime)
	ON_BN_CLICKED(IDC_ECHO_DIME, OnEchoDime)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttachmentsEVCDlg message handlers

BOOL CAttachmentsEVCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CenterWindow(GetDesktopWindow());	// center to the hpc screen

	// TODO: Add extra initialization here
	CEdit * file = (CEdit*)GetDlgItem(IDC_FILENAME);
	file->SetWindowText(L"\\windows\\default.htm");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

CComVariant CAttachmentsEVCDlg::SelectedFileAsVariant()
{
	CEdit * file = (CEdit*)GetDlgItem(IDC_FILENAME);
	CComVariant vText;
	WCHAR buff[MAX_PATH];
	file->GetWindowText(buff, MAX_PATH);
	vText.bstrVal = SysAllocString(buff);
	vText.vt = VT_BSTR;
	return vText;
}

void CAttachmentsEVCDlg::LogMsg(WCHAR *msg, WCHAR *msgPart2)
{
	CComBSTR b(msg);
	b.Append(msgPart2);
	LogMsg(b);
}

void CAttachmentsEVCDlg::LogMsg(WCHAR *msg)
{
	CListBox * log = (CListBox *)GetDlgItem(IDC_RESULTS);
	log->AddString(msg);
}

void CAttachmentsEVCDlg::OnEchoMime() 
{
	// create the envelope
	CComPtr<ISOAPEnvelope> env ;
	_HR(env.CoCreateInstance(__uuidof(CoEnvelope)));
	env->SetMethod(CComBSTR(L"echo"), CComBSTR(L"urn:EchoAttachmentsService")) ;

	// create the attachments manager
	CComPtr<IAttachmentManger> mgr ;
	_HR(mgr.CoCreateInstance(__uuidof(CoAttachmentManager)));
	mgr->put_Format(formatMime) ;

	// create the attachment
	CComPtr<ISoapAttachment> att ;
	CComPtr<ISoapAttachments> req ;
	mgr->get_Request(&req) ;
	
	req->Create(SelectedFileAsVariant(), tnfMediaType, CComBSTR(L"application/octectstream"), &att) ;
	CComBSTR uri ;
	att->get_Uri(&uri) ;

	// create the parameter
	CComPtr<ISOAPNodes> params ;
	env->get_Parameters(&params) ;
	CComPtr<ISOAPNode> param ;
	params->Create(CComBSTR(L"source"), CComVariant(), NULL, NULL, NULL, &param ) ;
	param->put_href(uri) ;

	// create the transport
	CComPtr<IHTTPTransport> http ;
	_HR(http.CoCreateInstance(__uuidof(HTTPTransport)));
	http->put_SOAPAction(CComBSTR(L"test")) ;

	// set the maanager transport property
	CComPtr<ISOAPTransport> soapt ;
	mgr->putref_Transport(http) ;
	mgr.QueryInterface(&soapt) ;

	// serialize the envelope
	CComBSTR bstrEnv ;
	env->Serialize(&bstrEnv) ;

	// send the request, and parse the response
	// not that the transport in both these cases is the attachments manager
	// not the http transport directly
	CComBSTR enc  ;
	_HR(soapt->Send(CComBSTR(L"http://soap.4s4c.com/axis/servlet/AxisServlet"), bstrEnv ));
	_HR(env->Parse ( CComVariant(soapt), enc ));

	// find the response parameter
	params.Release() ;
	param.Release() ;
	env->get_Parameters(&params) ;
	params->get_Item(0, &param) ;
	CComBSTR href ;
	param->get_href(&href) ;

	// find the response attachment
	CComPtr<ISoapAttachments> response ;
	mgr->get_Response(&response) ;
	att.Release() ;
	response->Find(href, &att) ;

	// dump some info about the returned attachment
	DumpAttInfo(att);
}

void CAttachmentsEVCDlg::DumpAttInfo(CComPtr<ISoapAttachment> &att)
{
	CComBSTR cid ;
	att->get_ContentId(&cid) ;
	LogMsg(L"Content Id is");
	LogMsg(cid);

	AttachmentLocation loc;
	att->get_Located(&loc);
	CComVariant vBody;
	att->get_Body(&vBody);
	if(loc == attOnDisk) 
	{
		LogMsg(L"Attachment on disk :", vBody.bstrVal);
	} 
	else
	{
		long ub, lb;
		SafeArrayGetUBound(vBody.parray, 1, &ub);
		SafeArrayGetLBound(vBody.parray, 1, &lb);
		WCHAR buff[50];
		wsprintf(buff, L"Attachment in memory, %d bytes", ub-lb+1);
		LogMsg(buff);
	}
}

void CAttachmentsEVCDlg::OnEchoDime() 
{
	// create the envelope
	CComPtr<ISOAPEnvelope> env ;
	_HR(env.CoCreateInstance(__uuidof(CoEnvelope)));
	env->put_EncodingStyle(CComBSTR(L""));
	CComBSTR svcURI(L"http://xsd.prototypes.4s4c.com/dime/");
	env->SetMethod(CComBSTR(L"echoAttachments"), svcURI); ;

	// create the attachments manager
	CComPtr<IAttachmentManger> mgr ;
	_HR(mgr.CoCreateInstance(__uuidof(CoAttachmentManager)));
	mgr->put_Format(formatDime) ;

	// create the attachment
	CComPtr<ISoapAttachment> att ;
	CComPtr<ISoapAttachments> req ;
	mgr->get_Request(&req) ;
	req->Create(SelectedFileAsVariant(), tnfMediaType, CComBSTR(L"application/octectstream"), &att) ;
	CComBSTR uri ;
	att->get_Uri(&uri) ;

	// create the attachments & attachment elemenets
	CComPtr<ISOAPNodes> params ;
	env->get_Parameters(&params) ;
	CComPtr<ISOAPNode> param, paramAtt;
	params->Create(CComBSTR(L"attachments"), CComVariant(), svcURI, NULL, NULL, &param ) ;
	params.Release();
	param->get_Nodes(&params);
	params->Create(CComBSTR(L"attachment"), CComVariant(), svcURI, NULL, NULL, &paramAtt);
	paramAtt->put_href(uri);

	// create the transport
	CComPtr<IHTTPTransport> http ;
	_HR(http.CoCreateInstance(__uuidof(HTTPTransport)));
	http->put_SOAPAction(CComBSTR(L"test")) ;

	// set the maanager transport property
	CComPtr<ISOAPTransport> soapt ;
	mgr->putref_Transport(http) ;
	mgr.QueryInterface(&soapt) ;

	// serialize the envelope
	CComBSTR bstrEnv ;
	env->Serialize(&bstrEnv) ;

	// send the request, and parse the response
	// not that the transport in both these cases is the attachments manager
	// not the http transport directly
	CComBSTR enc  ;
	_HR(soapt->Send(CComBSTR(L"http://soap.4s4c.com/dime2/sf.soap"), bstrEnv ));
	_HR(env->Parse ( CComVariant(soapt), enc ));

	// find the response parameter
	params.Release() ;
	param.Release() ;
	env->get_Parameters(&params) ;
	params->get_ItemByName(CComBSTR(L"attachment"), svcURI, &param) ;
	CComBSTR href ;
	param->get_href(&href) ;

	// find the response attachment
	CComPtr<ISoapAttachments> response ;
	mgr->get_Response(&response) ;
	att.Release() ;
	response->Find(href, &att) ;

	// dump some info about the returned attachment
	DumpAttInfo(att);
}
