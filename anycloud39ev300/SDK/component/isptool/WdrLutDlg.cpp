// WdrLutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "WdrLutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWdrLutDlg dialog


CWdrLutDlg::CWdrLutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWdrLutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWdrLutDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWdrLutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWdrLutDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWdrLutDlg, CDialog)
	//{{AFX_MSG_MAP(CWdrLutDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWdrLutDlg message handlers
BOOL CWdrLutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect clientRect, rect;
	
	this->GetClientRect(&clientRect); 

	
	m_Area1Dlg.Create(IDD_DIALOG_CURVE, this);
	m_Area1Dlg.GetClientRect(&rect); 
	m_Area1Dlg.SetEnable(TRUE);

	rect.left = clientRect.left + 2;
	rect.top = clientRect.top + 2;
	rect.right += 2;
	rect.bottom += 2;

	m_Area1Dlg.MoveWindow(&rect);
	m_Area1Dlg.ShowWindow(SW_SHOW);

	CWnd * pCurveName = m_Area1Dlg.GetDlgItem(IDC_CURVE_NAME);
	pCurveName->SetWindowText("1");

	

	m_Area2Dlg.Create(IDD_DIALOG_CURVE, this);
	m_Area2Dlg.GetClientRect(&rect); 
	m_Area2Dlg.SetEnable(TRUE);

	rect.left = clientRect.left + 339;
	rect.top = clientRect.top + 2;
	rect.right += 339;
	rect.bottom += 2;

	m_Area2Dlg.MoveWindow(&rect);
	m_Area2Dlg.ShowWindow(SW_SHOW);

	pCurveName = m_Area2Dlg.GetDlgItem(IDC_CURVE_NAME);
	pCurveName->SetWindowText("2");

	m_Area3Dlg.Create(IDD_DIALOG_CURVE, this);
	m_Area3Dlg.GetClientRect(&rect); 
	m_Area3Dlg.SetEnable(TRUE);
	
	rect.left = clientRect.left + 676;
	rect.top = clientRect.top + 2;
	rect.right += 676;
	rect.bottom += 2;

	m_Area3Dlg.MoveWindow(&rect);
	m_Area3Dlg.ShowWindow(SW_SHOW);

	pCurveName = m_Area3Dlg.GetDlgItem(IDC_CURVE_NAME);
	pCurveName->SetWindowText("3");
	
	m_Area4Dlg.Create(IDD_DIALOG_CURVE, this);
	m_Area4Dlg.GetClientRect(&rect); 
	m_Area4Dlg.SetEnable(TRUE);

	rect.left = clientRect.left + 2;
	rect.top = clientRect.top + 310;
	rect.right += 2;
	rect.bottom += 310;

	m_Area4Dlg.MoveWindow(&rect);
	m_Area4Dlg.ShowWindow(SW_SHOW);

	pCurveName = m_Area4Dlg.GetDlgItem(IDC_CURVE_NAME);
	pCurveName->SetWindowText("4");

	m_Area5Dlg.Create(IDD_DIALOG_CURVE, this);
	m_Area5Dlg.GetClientRect(&rect); 
	m_Area5Dlg.SetEnable(TRUE);

	rect.left = clientRect.left + 339;
	rect.top = clientRect.top + 310;
	rect.right += 339;
	rect.bottom += 310;

	m_Area5Dlg.MoveWindow(&rect);
	m_Area5Dlg.ShowWindow(SW_SHOW);

	pCurveName = m_Area5Dlg.GetDlgItem(IDC_CURVE_NAME);
	pCurveName->SetWindowText("5");

	m_Area6Dlg.Create(IDD_DIALOG_CURVE, this);
	m_Area6Dlg.GetClientRect(&rect); 
	m_Area6Dlg.SetEnable(TRUE);

	rect.left = clientRect.left + 676;
	rect.top = clientRect.top + 310;
	rect.right += 676;
	rect.bottom += 310;

	m_Area6Dlg.MoveWindow(&rect);
	m_Area6Dlg.ShowWindow(SW_SHOW);

	pCurveName = m_Area6Dlg.GetDlgItem(IDC_CURVE_NAME);
	pCurveName->SetWindowText("6");

	return TRUE;
}

void CWdrLutDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	m_Area1Dlg.OnClose();
	m_Area2Dlg.OnClose();
	m_Area3Dlg.OnClose();
	m_Area4Dlg.OnClose();
	m_Area5Dlg.OnClose();
	m_Area6Dlg.OnClose();

	CDialog::OnClose();
}

void CWdrLutDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
	OnClose();
}

void CWdrLutDlg::OnButton2() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
	OnClose();
}
