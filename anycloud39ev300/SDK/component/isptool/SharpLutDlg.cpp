// SharpLutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "SharpLutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSharpLutDlg dialog


CSharpLutDlg::CSharpLutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSharpLutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSharpLutDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSharpLutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSharpLutDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSharpLutDlg, CDialog)
	//{{AFX_MSG_MAP(CSharpLutDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSharpLutDlg message handlers
BOOL CSharpLutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect clientRect, rect;
	
	this->GetClientRect(&clientRect); 

	m_MfLutDlg.Create(IDD_DIALOG_LINE, this);
	m_MfLutDlg.GetClientRect(&rect); 

	rect.left = clientRect.left + 10;
	rect.top = clientRect.top + 10;
	rect.right += 10;
	rect.bottom += 10;

	m_MfLutDlg.MoveWindow(&rect);
	m_MfLutDlg.ShowWindow(SW_SHOW);

	CWnd * pCurveName = m_MfLutDlg.GetDlgItem(IDC_CURVE_NAME);
	
	pCurveName->SetWindowText("MF");


	m_HfLutDlg.Create(IDD_DIALOG_LINE, this);
	m_HfLutDlg.GetClientRect(&rect); 

	rect.left = clientRect.left + 530;
	rect.top = clientRect.top + 10;
	rect.right += 530;
	rect.bottom += 10;

	m_HfLutDlg.MoveWindow(&rect);
	m_HfLutDlg.ShowWindow(SW_SHOW);

	pCurveName = m_HfLutDlg.GetDlgItem(IDC_CURVE_NAME);
	
	pCurveName->SetWindowText("HF");

	return TRUE;
}

void CSharpLutDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	m_MfLutDlg.OnClose();
	m_HfLutDlg.OnClose();

	CDialog::OnClose();
}

void CSharpLutDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
	OnClose();
}

void CSharpLutDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
	OnClose();
}
