// HUE_ImgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "HUE_ImgDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHUE_ImgDlg dialog


CHUE_ImgDlg::CHUE_ImgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHUE_ImgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHUE_ImgDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CHUE_ImgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHUE_ImgDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHUE_ImgDlg, CDialog)
	//{{AFX_MSG_MAP(CHUE_ImgDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHUE_ImgDlg message handlers

BOOL CHUE_ImgDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CRect clientRect, rect;
	
	this->GetClientRect(&clientRect); 
	
	m_HUE_lineDlg.Create(IDD_DIALOG_HUE_LINE, this);
	m_HUE_lineDlg.GetClientRect(&rect); 
	
	rect.left = clientRect.left + 10;
	rect.top = clientRect.top + 10;
	rect.right += 10;
	rect.bottom += 10;
	
	m_HUE_lineDlg.MoveWindow(&rect);
	m_HUE_lineDlg.ShowWindow(SW_SHOW);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHUE_ImgDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	m_HUE_lineDlg.Close();
	m_HUE_lineDlg.GetDataValue();
	CDialog::OnClose();
}

void CHUE_ImgDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
	OnClose();
	
}

void CHUE_ImgDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
	OnClose();
}
