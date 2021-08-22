// MiscOtherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "MiscOtherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMiscOtherDlg dialog


CMiscOtherDlg::CMiscOtherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMiscOtherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMiscOtherDlg)
	//}}AFX_DATA_INIT
}


void CMiscOtherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMiscOtherDlg)
	DDX_Control(pDX, IDC_SPIN1, m_mipi_cnt_time);
	DDX_Radio(pDX, IDC_RADIO1, m_2frm_merge_en);
	DDX_Radio(pDX, IDC_RADIO3, m_mipi_line_end_sel);
	DDX_Radio(pDX, IDC_RADIO5, m_mipi_line_end_cnt_en_cfg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMiscOtherDlg, CDialog)
	//{{AFX_MSG_MAP(CMiscOtherDlg)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnRadio6)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMiscOtherDlg message handlers


BOOL CMiscOtherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for (int i=0; i<10; i++)
	{
		m_mipi_cnt_time.SetRange32(0,65535);
	}

	SetDataValue();

	return TRUE;
}

BOOL CMiscOtherDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CMiscOtherDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();

	GetDataValue();
}

void CMiscOtherDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void CMiscOtherDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	m_Misc_attr.mipi_count_time = (T_U16)m_mipi_cnt_time.GetPos();
	m_Misc_attr.twoframe_merge_en = (T_U16)m_2frm_merge_en;
	m_Misc_attr.mipi_line_end_sel = (T_U16)m_mipi_line_end_sel;
	m_Misc_attr.mipi_line_end_cnt_en_cfg = (T_U16)m_mipi_line_end_cnt_en_cfg;
}

void CMiscOtherDlg::SetDataValue(void)
{
	m_mipi_cnt_time.SetPos(m_Misc_attr.mipi_count_time);

	m_2frm_merge_en = m_Misc_attr.twoframe_merge_en;
	m_mipi_line_end_sel = m_Misc_attr.mipi_line_end_sel;
	m_mipi_line_end_cnt_en_cfg = m_Misc_attr.mipi_line_end_cnt_en_cfg;

	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(!m_2frm_merge_en);
	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(m_2frm_merge_en);
	((CButton *)GetDlgItem(IDC_RADIO3))->SetCheck(!m_mipi_line_end_sel);
	((CButton *)GetDlgItem(IDC_RADIO4))->SetCheck(m_mipi_line_end_sel);
	((CButton *)GetDlgItem(IDC_RADIO5))->SetCheck(!m_mipi_line_end_cnt_en_cfg);
	((CButton *)GetDlgItem(IDC_RADIO6))->SetCheck(m_mipi_line_end_cnt_en_cfg);
	
	UpdateData(FALSE);
}

void CMiscOtherDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMiscOtherDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMiscOtherDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMiscOtherDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMiscOtherDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMiscOtherDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


