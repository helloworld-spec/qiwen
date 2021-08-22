// Y_GammaLargeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "Y_GammaLargeDlg.h"
#include "NetCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CY_GammaLargeDlg dialog


CY_GammaLargeDlg::CY_GammaLargeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CY_GammaLargeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CY_GammaLargeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CY_GammaLargeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CY_GammaLargeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CY_GammaLargeDlg, CDialog)
	//{{AFX_MSG_MAP(CY_GammaLargeDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CY_GammaLargeDlg message handlers
BOOL CY_GammaLargeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect clientRect, rect;
	
	this->GetClientRect(&clientRect); 
	
	m_CurveDlg.SetMessageWindow(m_pMessageWnd);
	m_CurveDlg.Create(IDD_DIALOG_CURVELARGE, this);
	m_CurveDlg.GetClientRect(&rect); 
	m_CurveDlg.SetEnable(TRUE);
	
	rect.left = clientRect.left + 10;
	rect.top = clientRect.top + 10;
	rect.right += 10;
	rect.bottom += 10;

	m_CurveDlg.MoveWindow(&rect);
	m_CurveDlg.ShowWindow(SW_SHOW);

	m_CurveDlg.m_no_key_show_flag = TRUE;

	m_CurveDlg.Refresh();

	return TRUE;
}

void CY_GammaLargeDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	m_CurveDlg.OnClose();

	CDialog::OnClose();
}



void CY_GammaLargeDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
	OnClose();
}

void CY_GammaLargeDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
	OnClose();
}
