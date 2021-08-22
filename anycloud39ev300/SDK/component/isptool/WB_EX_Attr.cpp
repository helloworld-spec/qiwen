// WB_EX_Attr.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "WB_EX_Attr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// WB_EX_Attr dialog


WB_EX_Attr::WB_EX_Attr(CWnd* pParent /*=NULL*/)
	: CDialog(WB_EX_Attr::IDD, pParent)
{
	//{{AFX_DATA_INIT(WB_EX_Attr)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void WB_EX_Attr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(WB_EX_Attr)
	
	//}}AFX_DATA_MAP

	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MIN10, m_wb_ex_spin_rgain_min[9]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MIN9, m_wb_ex_spin_rgain_min[8]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MIN8, m_wb_ex_spin_rgain_min[7]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MIN7, m_wb_ex_spin_rgain_min[6]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MIN6, m_wb_ex_spin_rgain_min[5]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MIN5, m_wb_ex_spin_rgain_min[4]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MIN4, m_wb_ex_spin_rgain_min[3]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MIN3, m_wb_ex_spin_rgain_min[2]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MIN2, m_wb_ex_spin_rgain_min[1]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MIN1, m_wb_ex_spin_rgain_min[0]);


	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MAX10, m_wb_ex_spin_rgain_max[9]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MAX9, m_wb_ex_spin_rgain_max[8]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MAX8, m_wb_ex_spin_rgain_max[7]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MAX7, m_wb_ex_spin_rgain_max[6]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MAX6, m_wb_ex_spin_rgain_max[5]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MAX5, m_wb_ex_spin_rgain_max[4]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MAX4, m_wb_ex_spin_rgain_max[3]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MAX3, m_wb_ex_spin_rgain_max[2]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MAX2, m_wb_ex_spin_rgain_max[1]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_MAX1, m_wb_ex_spin_rgain_max[0]);

	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_EX10, m_wb_ex_spin_rgain_ex[9]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_EX9, m_wb_ex_spin_rgain_ex[8]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_EX8, m_wb_ex_spin_rgain_ex[7]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_EX7, m_wb_ex_spin_rgain_ex[6]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_EX6, m_wb_ex_spin_rgain_ex[5]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_EX5, m_wb_ex_spin_rgain_ex[4]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_EX4, m_wb_ex_spin_rgain_ex[3]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_EX3, m_wb_ex_spin_rgain_ex[2]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_EX2, m_wb_ex_spin_rgain_ex[1]);
	DDX_Control(pDX, IDC_SPIN_WB_RGAIN_EX1, m_wb_ex_spin_rgain_ex[0]);

	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MIN10, m_wb_ex_spin_ggain_min[9]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MIN9, m_wb_ex_spin_ggain_min[8]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MIN8, m_wb_ex_spin_ggain_min[7]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MIN7, m_wb_ex_spin_ggain_min[6]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MIN6, m_wb_ex_spin_ggain_min[5]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MIN5, m_wb_ex_spin_ggain_min[4]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MIN4, m_wb_ex_spin_ggain_min[3]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MIN3, m_wb_ex_spin_ggain_min[2]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MIN2, m_wb_ex_spin_ggain_min[1]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MIN1, m_wb_ex_spin_ggain_min[0]);

	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MAX10, m_wb_ex_spin_ggain_max[9]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MAX9, m_wb_ex_spin_ggain_max[8]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MAX8, m_wb_ex_spin_ggain_max[7]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MAX7, m_wb_ex_spin_ggain_max[6]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MAX6, m_wb_ex_spin_ggain_max[5]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MAX5, m_wb_ex_spin_ggain_max[4]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MAX4, m_wb_ex_spin_ggain_max[3]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MAX3, m_wb_ex_spin_ggain_max[2]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MAX2, m_wb_ex_spin_ggain_max[1]);
	DDX_Control(pDX, IDC_SPIN_WB_GGAIN_MAX1, m_wb_ex_spin_ggain_max[0]);

	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MIN10, m_wb_ex_spin_bgain_min[9]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MIN9, m_wb_ex_spin_bgain_min[8]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MIN8, m_wb_ex_spin_bgain_min[7]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MIN7, m_wb_ex_spin_bgain_min[6]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MIN6, m_wb_ex_spin_bgain_min[5]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MIN5, m_wb_ex_spin_bgain_min[4]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MIN4, m_wb_ex_spin_bgain_min[3]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MIN3, m_wb_ex_spin_bgain_min[2]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MIN2, m_wb_ex_spin_bgain_min[1]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MIN1, m_wb_ex_spin_bgain_min[0]);

	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MAX10, m_wb_ex_spin_bgain_max[9]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MAX9, m_wb_ex_spin_bgain_max[8]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MAX8, m_wb_ex_spin_bgain_max[7]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MAX7, m_wb_ex_spin_bgain_max[6]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MAX6, m_wb_ex_spin_bgain_max[5]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MAX5, m_wb_ex_spin_bgain_max[4]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MAX4, m_wb_ex_spin_bgain_max[3]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MAX3, m_wb_ex_spin_bgain_max[2]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MAX2, m_wb_ex_spin_bgain_max[1]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_MAX1, m_wb_ex_spin_bgain_max[0]);

	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_EX10, m_wb_ex_spin_bgain_ex[9]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_EX9, m_wb_ex_spin_bgain_ex[8]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_EX8, m_wb_ex_spin_bgain_ex[7]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_EX7, m_wb_ex_spin_bgain_ex[6]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_EX6, m_wb_ex_spin_bgain_ex[5]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_EX5, m_wb_ex_spin_bgain_ex[4]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_EX4, m_wb_ex_spin_bgain_ex[3]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_EX3, m_wb_ex_spin_bgain_ex[2]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_EX2, m_wb_ex_spin_bgain_ex[1]);
	DDX_Control(pDX, IDC_SPIN_WB_BGAIN_EX1, m_wb_ex_spin_bgain_ex[0]);
}


BEGIN_MESSAGE_MAP(WB_EX_Attr, CDialog)
	//{{AFX_MSG_MAP(WB_EX_Attr)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_CHECK_WB_EX_CTRL_ENABLE, OnCheckWbExCtrlEnable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WB_EX_Attr message handlers

void WB_EX_Attr::OnCheckWbExCtrlEnable() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


BOOL WB_EX_Attr::OnInitDialog() 
{
	int i = 0;
	
	CDialog::OnInitDialog();

	for (i=0; i<10; i++)
	{
		m_wb_ex_spin_rgain_max[i].SetRange(0,4096);
		m_wb_ex_spin_bgain_max[i].SetRange(0,4096);
		m_wb_ex_spin_ggain_max[i].SetRange(0,4096);
		m_wb_ex_spin_rgain_min[i].SetRange(0,4096);
		m_wb_ex_spin_bgain_min[i].SetRange(0,4096);
		m_wb_ex_spin_ggain_min[i].SetRange(0,4096);
		m_wb_ex_spin_rgain_ex[i].SetRange(0,4096);
		m_wb_ex_spin_bgain_ex[i].SetRange(0,4096);
	}

	SetDataValue();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL WB_EX_Attr::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void WB_EX_Attr::GetDataValue(void)
{
	BOOL check = FALSE;
	int i = 0;
	
	//UpdateData(TRUE);
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_WB_EX_CTRL_ENABLE))->GetCheck();
	if (check)
	{
		m_isp_awb_ex_attr.awb_ex_ctrl_enable = 1;
	}
	else
	{
		m_isp_awb_ex_attr.awb_ex_ctrl_enable = 0;
	}

	for (i=0; i<10; i++)
	{
		m_isp_awb_ex_attr.awb_ctrl[i].rgain_max = (T_U16)m_wb_ex_spin_rgain_max[i].GetPos();  
		m_isp_awb_ex_attr.awb_ctrl[i].rgain_min  = (T_S16)m_wb_ex_spin_rgain_min[i].GetPos(); 
		m_isp_awb_ex_attr.awb_ctrl[i].bgain_max = (T_U16)m_wb_ex_spin_bgain_max[i].GetPos();  
		m_isp_awb_ex_attr.awb_ctrl[i].bgain_min  = (T_S16)m_wb_ex_spin_bgain_min[i].GetPos(); 
		m_isp_awb_ex_attr.awb_ctrl[i].ggain_max = (T_U16)m_wb_ex_spin_ggain_max[i].GetPos();  
		m_isp_awb_ex_attr.awb_ctrl[i].ggain_min  = (T_S16)m_wb_ex_spin_ggain_min[i].GetPos(); 
		m_isp_awb_ex_attr.awb_ctrl[i].rgain_ex = (T_U16)m_wb_ex_spin_rgain_ex[i].GetPos();    
		m_isp_awb_ex_attr.awb_ctrl[i].bgain_ex  = (T_S16)m_wb_ex_spin_bgain_ex[i].GetPos();   
	}


}


void WB_EX_Attr::SetDataValue(void)
{
	int i = 0;

	for (i=0; i<10; i++)
	{
		m_wb_ex_spin_rgain_max[i].SetPos(m_isp_awb_ex_attr.awb_ctrl[i].rgain_max); 
		m_wb_ex_spin_bgain_max[i].SetPos(m_isp_awb_ex_attr.awb_ctrl[i].bgain_max); 
		m_wb_ex_spin_ggain_max[i].SetPos(m_isp_awb_ex_attr.awb_ctrl[i].ggain_max); 
		m_wb_ex_spin_rgain_min[i].SetPos(m_isp_awb_ex_attr.awb_ctrl[i].rgain_min); 
		m_wb_ex_spin_bgain_min[i].SetPos(m_isp_awb_ex_attr.awb_ctrl[i].bgain_min); 
		m_wb_ex_spin_ggain_min[i].SetPos(m_isp_awb_ex_attr.awb_ctrl[i].ggain_min); 
		m_wb_ex_spin_rgain_ex[i].SetPos(m_isp_awb_ex_attr.awb_ctrl[i].rgain_ex);   
		m_wb_ex_spin_bgain_ex[i].SetPos(m_isp_awb_ex_attr.awb_ctrl[i].bgain_ex); 
	}

	if (m_isp_awb_ex_attr.awb_ex_ctrl_enable == 0)
	{
		((CButton *)GetDlgItem(IDC_CHECK_WB_EX_CTRL_ENABLE))->SetCheck(0);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_CHECK_WB_EX_CTRL_ENABLE))->SetCheck(1);
	}
	
}


void WB_EX_Attr::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
	
	GetDataValue();
}

void WB_EX_Attr::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

