// LSCCOEFDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "LSCCOEFDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLSCCOEFDlg dialog


CLSCCOEFDlg::CLSCCOEFDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLSCCOEFDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLSCCOEFDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLSCCOEFDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLSCCOEFDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_SPIN1, m_r_b[0]);
	DDX_Control(pDX, IDC_SPIN2, m_r_b[1]);
	DDX_Control(pDX, IDC_SPIN3, m_r_b[2]);
	DDX_Control(pDX, IDC_SPIN5, m_r_b[3]);
	DDX_Control(pDX, IDC_SPIN9, m_r_b[4]);
	DDX_Control(pDX, IDC_SPIN10, m_r_b[5]);
	DDX_Control(pDX, IDC_SPIN11, m_r_b[6]);
	DDX_Control(pDX, IDC_SPIN12, m_r_b[7]);
	DDX_Control(pDX, IDC_SPIN13, m_r_b[8]);
	DDX_Control(pDX, IDC_SPIN14, m_r_b[9]);

	DDX_Control(pDX, IDC_SPIN15, m_r_c[0]);
	DDX_Control(pDX, IDC_SPIN16, m_r_c[1]);
	DDX_Control(pDX, IDC_SPIN17, m_r_c[2]);
	DDX_Control(pDX, IDC_SPIN18, m_r_c[3]);
	DDX_Control(pDX, IDC_SPIN19, m_r_c[4]);
	DDX_Control(pDX, IDC_SPIN20, m_r_c[5]);
	DDX_Control(pDX, IDC_SPIN21, m_r_c[6]);
	DDX_Control(pDX, IDC_SPIN22, m_r_c[7]);
	DDX_Control(pDX, IDC_SPIN23, m_r_c[8]);
	DDX_Control(pDX, IDC_SPIN24, m_r_c[9]);

	DDX_Control(pDX, IDC_SPIN25, m_gr_b[0]);
	DDX_Control(pDX, IDC_SPIN26, m_gr_b[1]);
	DDX_Control(pDX, IDC_SPIN27, m_gr_b[2]);
	DDX_Control(pDX, IDC_SPIN28, m_gr_b[3]);
	DDX_Control(pDX, IDC_SPIN29, m_gr_b[4]);
	DDX_Control(pDX, IDC_SPIN30, m_gr_b[5]);
	DDX_Control(pDX, IDC_SPIN31, m_gr_b[6]);
	DDX_Control(pDX, IDC_SPIN32, m_gr_b[7]);
	DDX_Control(pDX, IDC_SPIN33, m_gr_b[8]);
	DDX_Control(pDX, IDC_SPIN34, m_gr_b[9]);

	DDX_Control(pDX, IDC_SPIN35, m_gr_c[0]);
	DDX_Control(pDX, IDC_SPIN36, m_gr_c[1]);
	DDX_Control(pDX, IDC_SPIN37, m_gr_c[2]);
	DDX_Control(pDX, IDC_SPIN38, m_gr_c[3]);
	DDX_Control(pDX, IDC_SPIN39, m_gr_c[4]);
	DDX_Control(pDX, IDC_SPIN40, m_gr_c[5]);
	DDX_Control(pDX, IDC_SPIN41, m_gr_c[6]);
	DDX_Control(pDX, IDC_SPIN42, m_gr_c[7]);
	DDX_Control(pDX, IDC_SPIN43, m_gr_c[8]);
	DDX_Control(pDX, IDC_SPIN44, m_gr_c[9]);

	DDX_Control(pDX, IDC_SPIN45, m_gb_b[0]);
	DDX_Control(pDX, IDC_SPIN46, m_gb_b[1]);
	DDX_Control(pDX, IDC_SPIN47, m_gb_b[2]);
	DDX_Control(pDX, IDC_SPIN48, m_gb_b[3]);
	DDX_Control(pDX, IDC_SPIN49, m_gb_b[4]);
	DDX_Control(pDX, IDC_SPIN50, m_gb_b[5]);
	DDX_Control(pDX, IDC_SPIN51, m_gb_b[6]);
	DDX_Control(pDX, IDC_SPIN52, m_gb_b[7]);
	DDX_Control(pDX, IDC_SPIN53, m_gb_b[8]);
	DDX_Control(pDX, IDC_SPIN54, m_gb_b[9]);

	DDX_Control(pDX, IDC_SPIN55, m_gb_c[0]);
	DDX_Control(pDX, IDC_SPIN56, m_gb_c[1]);
	DDX_Control(pDX, IDC_SPIN57, m_gb_c[2]);
	DDX_Control(pDX, IDC_SPIN58, m_gb_c[3]);
	DDX_Control(pDX, IDC_SPIN59, m_gb_c[4]);
	DDX_Control(pDX, IDC_SPIN60, m_gb_c[5]);
	DDX_Control(pDX, IDC_SPIN61, m_gb_c[6]);
	DDX_Control(pDX, IDC_SPIN62, m_gb_c[7]);
	DDX_Control(pDX, IDC_SPIN63, m_gb_c[8]);
	DDX_Control(pDX, IDC_SPIN64, m_gb_c[9]);

	DDX_Control(pDX, IDC_SPIN65, m_b_b[0]);
	DDX_Control(pDX, IDC_SPIN66, m_b_b[1]);
	DDX_Control(pDX, IDC_SPIN67, m_b_b[2]);
	DDX_Control(pDX, IDC_SPIN68, m_b_b[3]);
	DDX_Control(pDX, IDC_SPIN69, m_b_b[4]);
	DDX_Control(pDX, IDC_SPIN70, m_b_b[5]);
	DDX_Control(pDX, IDC_SPIN71, m_b_b[6]);
	DDX_Control(pDX, IDC_SPIN72, m_b_b[7]);
	DDX_Control(pDX, IDC_SPIN73, m_b_b[8]);
	DDX_Control(pDX, IDC_SPIN74, m_b_b[9]);

	DDX_Control(pDX, IDC_SPIN75, m_b_c[0]);
	DDX_Control(pDX, IDC_SPIN76, m_b_c[1]);
	DDX_Control(pDX, IDC_SPIN77, m_b_c[2]);
	DDX_Control(pDX, IDC_SPIN78, m_b_c[3]);
	DDX_Control(pDX, IDC_SPIN79, m_b_c[4]);
	DDX_Control(pDX, IDC_SPIN80, m_b_c[5]);
	DDX_Control(pDX, IDC_SPIN81, m_b_c[6]);
	DDX_Control(pDX, IDC_SPIN82, m_b_c[7]);
	DDX_Control(pDX, IDC_SPIN83, m_b_c[8]);
	DDX_Control(pDX, IDC_SPIN84, m_b_c[9]);
}

BEGIN_MESSAGE_MAP(CLSCCOEFDlg, CDialog)
	//{{AFX_MSG_MAP(CLSCCOEFDlg)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLSCCOEFDlg message handlers

BOOL CLSCCOEFDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for (int i=0; i<10; i++)
	{
		m_r_b[i].SetRange(0,255);
		m_r_c[i].SetRange(0,1023);

		m_gr_b[i].SetRange(0,255);
		m_gr_c[i].SetRange(0,1023);

		m_gb_b[i].SetRange(0,255);
		m_gb_c[i].SetRange(0,1023);

		m_b_b[i].SetRange(0,255);
		m_b_c[i].SetRange(0,1023);
	}

	SetDataValue();

	return TRUE;
}

BOOL CLSCCOEFDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CLSCCOEFDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();

	GetDataValue();
}

void CLSCCOEFDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void CLSCCOEFDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	for (int i=0; i<10; i++)
	{
		m_r_coef.coef_b[i] = (T_U16)m_r_b[i].GetPos();
		m_r_coef.coef_c[i] = (T_U16)m_r_c[i].GetPos();

		m_gr_coef.coef_b[i] = (T_U16)m_gr_b[i].GetPos();
		m_gr_coef.coef_c[i] = (T_U16)m_gr_c[i].GetPos();

		m_gb_coef.coef_b[i] = (T_U16)m_gb_b[i].GetPos();
		m_gb_coef.coef_c[i] = (T_U16)m_gb_c[i].GetPos();

		m_b_coef.coef_b[i] = (T_U16)m_b_b[i].GetPos();
		m_b_coef.coef_c[i] = (T_U16)m_b_c[i].GetPos();
	}

}

void CLSCCOEFDlg::SetDataValue(void)
{
	for (int i=0; i<10; i++)
	{
		m_r_b[i].SetPos(m_r_coef.coef_b[i]);
		m_r_c[i].SetPos(m_r_coef.coef_c[i]);

		m_gr_b[i].SetPos(m_gr_coef.coef_b[i]);
		m_gr_c[i].SetPos(m_gr_coef.coef_c[i]);

		m_gb_b[i].SetPos(m_gb_coef.coef_b[i]);
		m_gb_c[i].SetPos(m_gb_coef.coef_c[i]);

		m_b_b[i].SetPos(m_b_coef.coef_b[i]);
		m_b_c[i].SetPos(m_b_coef.coef_c[i]);
	}

	UpdateData(FALSE);
}