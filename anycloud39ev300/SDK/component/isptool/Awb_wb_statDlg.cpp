// Awb_wb_statDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "Awb_wb_statDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAwb_wb_statDlg dialog


CAwb_wb_statDlg::CAwb_wb_statDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAwb_wb_statDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAwb_wb_statDlg)
	m_cnt_thre = 0;
	//}}AFX_DATA_INIT
	ZeroMemory(&m_isp_awb_wb_stat, sizeof(AK_ISP_AWB_ATTR));
}


void CAwb_wb_statDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAwb_wb_statDlg)
	DDX_Control(pDX, IDC_SPIN_AWB_Y_LOW, m_spin_awb_y_low);
	DDX_Control(pDX, IDC_SPIN_AWB_Y_HIGH, m_spin_awb_y_high);
	DDX_Control(pDX, IDC_SPIN_AWB_STEP, m_spin_awb_step);
	DDX_Control(pDX, IDC_SPIN_AWB_STABLE_CNT_THRE, m_spin_awb_stable_cnt_thre);
	DDX_Control(pDX, IDC_SPIN_AWB_CNT_THRE, m_spin_awb_cnt_thre);
	DDX_Text(pDX, IDC_EDIT_AWB_CNT_THRE, m_cnt_thre);
	//}}AFX_DATA_MAP

	DDX_Control(pDX, IDC_SPIN_AWB_A_RB_LOW, m_spin_awb_rb_low[0]);
	DDX_Control(pDX, IDC_SPIN_AWB_A_RB_HIGH, m_spin_awb_rb_high[0]);
	DDX_Control(pDX, IDC_SPIN_AWB_A_GR_LOW, m_spin_awb_gr_low[0]);
	DDX_Control(pDX, IDC_SPIN_AWB_A_GR_HIGH, m_spin_awb_gr_high[0]);
	DDX_Control(pDX, IDC_SPIN_AWB_A_GB_LOW, m_spin_awb_gb_low[0]);
	DDX_Control(pDX, IDC_SPIN_AWB_A_GB_HIGH, m_spin_awb_gb_high[0]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_RB_LOW, m_spin_awb_rb_low[1]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_RB_HIGH, m_spin_awb_rb_high[1]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_GR_LOW, m_spin_awb_gr_low[1]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_GR_HIGH, m_spin_awb_gr_high[1]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_GB_LOW, m_spin_awb_gb_low[1]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_GB_HIGH, m_spin_awb_gb_high[1]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_RB_LOW3, m_spin_awb_rb_low[2]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_RB_HIGH, m_spin_awb_rb_high[2]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_GR_LOW, m_spin_awb_gr_low[2]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_GR_HIGH, m_spin_awb_gr_high[2]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_GB_LOW, m_spin_awb_gb_low[2]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_GB_HIGH, m_spin_awb_gb_high[2]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_RB_LOW, m_spin_awb_rb_low[3]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_RB_HIGH, m_spin_awb_rb_high[3]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_GR_LOW, m_spin_awb_gr_low[3]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_GR_HIGH, m_spin_awb_gr_high[3]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_GB_LOW, m_spin_awb_gb_low[3]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_GB_HIGH, m_spin_awb_gb_high[3]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_RB_LOW, m_spin_awb_rb_low[4]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_RB_HIGH, m_spin_awb_rb_high[4]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_GR_LOW, m_spin_awb_gr_low[4]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_GR_HIGH, m_spin_awb_gr_high[4]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_GB_LOW, m_spin_awb_gb_low[4]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_GB_HIGH, m_spin_awb_gb_high[4]);

	DDX_Control(pDX, IDC_SPIN_AWB_A_1_RB_LOW, m_spin_awb_rb_low[5]);
	DDX_Control(pDX, IDC_SPIN_AWB_A_1_RB_HIGH, m_spin_awb_rb_high[5]);
	DDX_Control(pDX, IDC_SPIN_AWB_A_1_GR_LOW, m_spin_awb_gr_low[5]);
	DDX_Control(pDX, IDC_SPIN_AWB_A_1_GR_HIGH, m_spin_awb_gr_high[5]);
	DDX_Control(pDX, IDC_SPIN_AWB_A_1_GB_LOW, m_spin_awb_gb_low[5]);
	DDX_Control(pDX, IDC_SPIN_AWB_A_1_GB_HIGH, m_spin_awb_gb_high[5]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_1_RB_LOW, m_spin_awb_rb_low[6]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_1_RB_HIGH, m_spin_awb_rb_high[6]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_1_GR_LOW, m_spin_awb_gr_low[6]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_1_GR_HIGH, m_spin_awb_gr_high[6]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_1_GB_LOW, m_spin_awb_gb_low[6]);
	DDX_Control(pDX, IDC_SPIN_AWB_TL84_1_GB_HIGH, m_spin_awb_gb_high[6]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_1_RB_LOW, m_spin_awb_rb_low[7]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_1_RB_HIGH, m_spin_awb_rb_high[7]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_1_GR_LOW, m_spin_awb_gr_low[7]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_1_GR_HIGH, m_spin_awb_gr_high[7]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_1_GB_LOW, m_spin_awb_gb_low[7]);
	DDX_Control(pDX, IDC_SPIN_AWB_D50_1_GB_HIGH, m_spin_awb_gb_high[7]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_1_RB_LOW, m_spin_awb_rb_low[8]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_1_RB_HIGH, m_spin_awb_rb_high[8]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_1_GR_LOW, m_spin_awb_gr_low[8]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_1_GR_HIGH, m_spin_awb_gr_high[8]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_1_GB_LOW, m_spin_awb_gb_low[8]);
	DDX_Control(pDX, IDC_SPIN_AWB_D65_1_GB_HIGH, m_spin_awb_gb_high[8]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_1_RB_LOW, m_spin_awb_rb_low[9]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_1_RB_HIGH, m_spin_awb_rb_high[9]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_1_GR_LOW, m_spin_awb_gr_low[9]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_1_GR_HIGH, m_spin_awb_gr_high[9]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_1_GB_LOW, m_spin_awb_gb_low[9]);
	DDX_Control(pDX, IDC_SPIN_AWB_D75_1_GB_HIGH, m_spin_awb_gb_high[9]);
}


BEGIN_MESSAGE_MAP(CAwb_wb_statDlg, CDialog)
	//{{AFX_MSG_MAP(CAwb_wb_statDlg)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAwb_wb_statDlg message handlers


void CAwb_wb_statDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	open_dlg_flag = FALSE;
	CDialog::OnCancel();
}

void CAwb_wb_statDlg::OnOK() 
{
	// TODO: Add extra validation here
	GetDataValue();
	open_dlg_flag = FALSE;
	CDialog::OnOK();
}


void CAwb_wb_statDlg::GetDataValue(void)
{
	int i = 0;
	
	UpdateData(TRUE);

	for (i=0; i<10; i++)
	{
		m_isp_awb_wb_stat.gr_low[i] = (T_U16)m_spin_awb_gr_low[i].GetPos();
		m_isp_awb_wb_stat.gr_high[i] = (T_U16)m_spin_awb_gr_high[i].GetPos();
		m_isp_awb_wb_stat.gb_low[i] = (T_U16)m_spin_awb_gb_low[i].GetPos();
		m_isp_awb_wb_stat.gb_high[i] = (T_U16)m_spin_awb_gb_high[i].GetPos();
		m_isp_awb_wb_stat.rb_low[i] = (T_U16)m_spin_awb_rb_low[i].GetPos();
		m_isp_awb_wb_stat.rb_high[i] = (T_U16)m_spin_awb_rb_high[i].GetPos();
	}


	m_isp_awb_wb_stat.y_low = (T_U16)m_spin_awb_y_low.GetPos();
	m_isp_awb_wb_stat.y_high = (T_U16)m_spin_awb_y_high.GetPos();
	m_isp_awb_wb_stat.auto_wb_step = (T_U16)m_spin_awb_step.GetPos();
	m_isp_awb_wb_stat.total_cnt_thresh = (T_U16)m_spin_awb_cnt_thre.GetPos();
	m_isp_awb_wb_stat.colortemp_stable_cnt_thresh = (T_U16)m_spin_awb_stable_cnt_thre.GetPos();

}

void CAwb_wb_statDlg::SetDataValue(void)
{
	int i = 0;

	for (i=0; i<10; i++)
	{
		m_spin_awb_gr_low[i].SetPos(m_isp_awb_wb_stat.gr_low[i]);
		m_spin_awb_gr_high[i].SetPos(m_isp_awb_wb_stat.gr_high[i]);
		m_spin_awb_gb_low[i].SetPos(m_isp_awb_wb_stat.gb_low[i]);
		m_spin_awb_gb_high[i].SetPos(m_isp_awb_wb_stat.gb_high[i]);
		m_spin_awb_rb_low[i].SetPos(m_isp_awb_wb_stat.rb_low[i]);
		m_spin_awb_rb_high[i].SetPos(m_isp_awb_wb_stat.rb_high[i]);
	}

	m_spin_awb_y_low.SetPos(m_isp_awb_wb_stat.y_low);
	m_spin_awb_y_high.SetPos(m_isp_awb_wb_stat.y_high);
	m_spin_awb_step.SetPos(m_isp_awb_wb_stat.auto_wb_step);
	m_spin_awb_cnt_thre.SetPos(m_isp_awb_wb_stat.total_cnt_thresh);

	CWnd *pWnd = NULL;
	CString str;
	m_cnt_thre = m_isp_awb_wb_stat.total_cnt_thresh;
	pWnd = GetDlgItem(IDC_EDIT_AWB_CNT_THRE);
	str.Format("%d", m_cnt_thre);
	pWnd->SetWindowText(str);

	m_spin_awb_stable_cnt_thre.SetPos(m_isp_awb_wb_stat.colortemp_stable_cnt_thresh);
	
}


int CAwb_wb_statDlg::GetPageInfoSt_wb_stat(void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_AWB_ATTR))) return -1;
	
	if (open_dlg_flag)
	{
		GetDataValue();
	}
	
	memcpy(((char*)pPageInfoSt + 16 * sizeof(T_U16)), &m_isp_awb_wb_stat.y_low, sizeof(AK_ISP_AWB_ATTR) - 16 * sizeof(T_U16));
	
	//nPageID = m_nID;
	nStLen = sizeof(AK_ISP_AWB_ATTR);
	
	return 0;
}

int CAwb_wb_statDlg::SetPageInfoSt_wb_stat(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_AWB_ATTR))) return -1;
	
	memcpy(&m_isp_awb_wb_stat, pPageInfoSt, sizeof(AK_ISP_AWB_ATTR));
	
	if (open_dlg_flag)
	{
		SetDataValue();
	}
	
	
	return 0;
}

BOOL CAwb_wb_statDlg::OnInitDialog() 
{
	int i = 0;
	
	CDialog::OnInitDialog();

	for (i=0; i<10; i++)
	{
		m_spin_awb_gr_low[i].SetRange(0,1023);
		m_spin_awb_gr_low[i].SetPos(0);
		m_spin_awb_gr_high[i].SetRange(0,1023);
		m_spin_awb_gr_high[i].SetPos(0);
		m_spin_awb_gb_low[i].SetRange(0,1023);
		m_spin_awb_gb_low[i].SetPos(0);
		m_spin_awb_gb_high[i].SetRange(0,1023);
		m_spin_awb_gb_high[i].SetPos(0);
		m_spin_awb_rb_low[i].SetRange(0,1023);
		m_spin_awb_rb_low[i].SetPos(0);
		m_spin_awb_rb_high[i].SetRange(0,1023);
		m_spin_awb_rb_high[i].SetPos(0);
	}

	//
	m_spin_awb_y_low.SetRange(0,255);
	m_spin_awb_y_low.SetPos(0);
	m_spin_awb_y_high.SetRange(0,255);
	m_spin_awb_y_high.SetPos(0);
	m_spin_awb_step.SetRange(0,255);
	m_spin_awb_step.SetPos(0);
	m_spin_awb_cnt_thre.SetRange32(0,65535);
	m_spin_awb_cnt_thre.SetPos(0);
	m_spin_awb_stable_cnt_thre.SetRange(0,255);
	m_spin_awb_stable_cnt_thre.SetPos(0);


	// TODO: Add extra initialization here
	SetDataValue();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CAwb_wb_statDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CAwb_wb_statDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	open_dlg_flag = FALSE;
	CDialog::OnClose();
}
