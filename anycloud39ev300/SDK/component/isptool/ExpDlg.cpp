// ExpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "ExpDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExpDlg dialog


CExpDlg::CExpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExpDlg)
	m_raw_hist_enable = 0;
	m_rgb_hist_enable = 0;
	m_yuv_hist_enable = 0;
	m_exp_mode = 0;
	m_exp_time_max_value = 0;
	m_d_gain_max_value = 0;
	m_isp_d_gain_max_value = 0;
	m_a_gain_max_value = 0;
	//}}AFX_DATA_INIT
}


void CExpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExpDlg)
	DDX_Control(pDX, IDC_SPIN88, m_target_lumiance);
	DDX_Control(pDX, IDC_SPIN16, m_stable_range);
	DDX_Control(pDX, IDC_SPIN142, m_step);
	DDX_Control(pDX, IDC_SPIN8, m_a_gain_min);
	DDX_Control(pDX, IDC_SPIN7, m_a_gain_max);
	DDX_Control(pDX, IDC_SPIN6, m_isp_d_gain_min);
	DDX_Control(pDX, IDC_SPIN5, m_isp_d_gain_max);
	DDX_Control(pDX, IDC_SPIN4, m_d_gain_min);
	DDX_Control(pDX, IDC_SPIN3, m_d_gain_max);
	DDX_Control(pDX, IDC_SPIN2, m_exp_time_min);
	DDX_Control(pDX, IDC_SPIN1, m_exp_time_max);
	DDX_Radio(pDX, IDC_RADIO1, m_raw_hist_enable);
	DDX_Radio(pDX, IDC_RADIO33, m_rgb_hist_enable);
	DDX_Radio(pDX, IDC_RADIO35, m_yuv_hist_enable);
	DDX_Radio(pDX, IDC_RADIO37, m_exp_mode);
	DDX_Text(pDX, IDC_EDIT1, m_exp_time_max_value);
	DDX_Text(pDX, IDC_EDIT3, m_d_gain_max_value);
	DDX_Text(pDX, IDC_EDIT5, m_isp_d_gain_max_value);
	DDX_Text(pDX, IDC_EDIT7, m_a_gain_max_value);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExpDlg, CDialog)
	//{{AFX_MSG_MAP(CExpDlg)
	ON_BN_CLICKED(IDC_BUTTON_MANUAL_EXP, OnButtonManualExp)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO33, OnRadio33)
	ON_BN_CLICKED(IDC_RADIO34, OnRadio34)
	ON_BN_CLICKED(IDC_RADIO35, OnRadio35)
	ON_BN_CLICKED(IDC_RADIO36, OnRadio36)
	ON_BN_CLICKED(IDC_RADIO37, OnRadio37)
	ON_BN_CLICKED(IDC_RADIO38, OnRadio38)
	ON_BN_CLICKED(IDC_BUTTON_OTHER, OnButtonOther)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExpDlg message handlers

BOOL CExpDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Exp, sizeof(AK_ISP_INIT_EXP));

	m_exp_time_max.SetRange32(0,1<<30);
	m_exp_time_max.SetPos(0);

	m_exp_time_min.SetRange32(0,1<<30);
	m_exp_time_min.SetPos(0);

	m_d_gain_max.SetRange32(0,1<<30);
	m_d_gain_max.SetPos(0);

	m_d_gain_min.SetRange32(0,1<<30);
	m_d_gain_min.SetPos(0);

	m_isp_d_gain_max.SetRange32(0,1<<30);
	m_isp_d_gain_max.SetPos(0);

	m_isp_d_gain_min.SetRange32(0,1<<30);
	m_isp_d_gain_min.SetPos(0);

	m_a_gain_max.SetRange32(0,1<<30);
	m_a_gain_max.SetPos(0);

	m_a_gain_min.SetRange32(0,1<<30);
	m_a_gain_min.SetPos(0);

	m_step.SetRange32(0,1<<30);
	m_step.SetPos(0);

	m_stable_range.SetRange32(0,1<<30);
	m_stable_range.SetPos(0);

	m_target_lumiance.SetRange32(0,1<<30);
	m_target_lumiance.SetPos(0);


	return TRUE;
}


BOOL CExpDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CExpDlg::OnButtonRead() 
{
	// TODO: Add your control notification handler code here
	CFile file;
	CFileException e;
	CFileDialog dlg(TRUE, "*.txt", NULL, OFN_HIDEREADONLY,
		"Config File(*.txt)|*.txt|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	if (!file.Open(dlg.GetPathName(), CFile::modeRead, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	if (file.GetLength() != sizeof(AK_ISP_INIT_EXP))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Exp, sizeof(AK_ISP_INIT_EXP));
	file.Close();

	SetDataValue();

	UpdateData(FALSE);
}

void CExpDlg::OnButtonWrite() 
{
	// TODO: Add your control notification handler code here
	CFile file;
	CFileException e;
	CFileDialog dlg(FALSE, "*.txt", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"Config File(*.txt)|*.txt|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	if (!file.Open(dlg.GetPathName(), CFile::modeCreate|CFile::modeWrite, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	GetDataValue();

	file.Write(&m_Exp, sizeof(AK_ISP_INIT_EXP));

	file.Close();
}

void CExpDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_EXP, 0);
}

void CExpDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_EXP, 0);
}


void CExpDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CExpDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CExpDlg::OnRadio33() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CExpDlg::OnRadio34() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CExpDlg::OnRadio35() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CExpDlg::OnRadio36() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CExpDlg::OnRadio37() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CExpDlg::OnRadio38() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CExpDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	m_Exp.param_id = ISP_EXP;
	m_Exp.length = sizeof(AK_ISP_INIT_EXP);

	m_Exp.p_ae.exp_time_max = (T_U32)m_exp_time_max.GetPos();
	m_Exp.p_ae.exp_time_min = (T_U32)m_exp_time_min.GetPos();
	m_Exp.p_ae.d_gain_max = (T_U32)m_d_gain_max.GetPos();
	m_Exp.p_ae.d_gain_min = (T_U32)m_d_gain_min.GetPos();
	m_Exp.p_ae.isp_d_gain_max = (T_U32)m_isp_d_gain_max.GetPos();
	m_Exp.p_ae.isp_d_gain_min = (T_U32)m_isp_d_gain_min.GetPos();
	m_Exp.p_ae.a_gain_max = (T_U32)m_a_gain_max.GetPos();
	m_Exp.p_ae.a_gain_min = (T_U32)m_a_gain_min.GetPos();
	m_Exp.p_ae.exp_step = (T_U32)m_step.GetPos();
	m_Exp.p_ae.exp_stable_range = (T_U32)m_stable_range.GetPos();
	m_Exp.p_ae.target_lumiance = (T_U32)m_target_lumiance.GetPos();

	m_Exp.p_raw_hist.enable = (T_U16)m_raw_hist_enable;
	m_Exp.p_rgb_hist.enable = (T_U16)m_rgb_hist_enable;
	m_Exp.p_yuv_hist.enable = (T_U16)m_yuv_hist_enable;
	m_Exp.p_exp_type.exp_type = (T_U16)m_exp_mode;

}

void CExpDlg::SetDataValue(void)
{
	CWnd *pWnd = NULL;
	CString str;


	m_exp_time_max.SetPos(m_Exp.p_ae.exp_time_max);
	m_exp_time_max_value = m_Exp.p_ae.exp_time_max;
	pWnd = GetDlgItem(IDC_EDIT1);
	str.Format("%d", m_exp_time_max_value);
	pWnd->SetWindowText(str);
	
	m_exp_time_min.SetPos(m_Exp.p_ae.exp_time_min);
	
	m_d_gain_max.SetPos(m_Exp.p_ae.d_gain_max);
	m_d_gain_max_value = m_Exp.p_ae.d_gain_max;
	pWnd = GetDlgItem(IDC_EDIT3);
	str.Format("%d", m_d_gain_max_value);
	pWnd->SetWindowText(str);

	m_d_gain_min.SetPos(m_Exp.p_ae.d_gain_min);

	m_isp_d_gain_max.SetPos(m_Exp.p_ae.isp_d_gain_max);
	m_isp_d_gain_max_value = m_Exp.p_ae.isp_d_gain_max;
	pWnd = GetDlgItem(IDC_EDIT5);
	str.Format("%d", m_isp_d_gain_max_value);
	pWnd->SetWindowText(str);
	
	m_isp_d_gain_min.SetPos(m_Exp.p_ae.isp_d_gain_min);

	m_a_gain_max.SetPos(m_Exp.p_ae.a_gain_max);
	m_a_gain_max_value = m_Exp.p_ae.a_gain_max;
	pWnd = GetDlgItem(IDC_EDIT7);
	str.Format("%d", m_a_gain_max_value);
	pWnd->SetWindowText(str);
	
	m_a_gain_min.SetPos(m_Exp.p_ae.a_gain_min);
	m_step.SetPos(m_Exp.p_ae.exp_step);
	m_stable_range.SetPos(m_Exp.p_ae.exp_stable_range);
	m_target_lumiance.SetPos(m_Exp.p_ae.target_lumiance);

	m_raw_hist_enable = m_Exp.p_raw_hist.enable;
	m_rgb_hist_enable = m_Exp.p_rgb_hist.enable;
	m_yuv_hist_enable = m_Exp.p_yuv_hist.enable;
	m_exp_mode = m_Exp.p_exp_type.exp_type;

	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(!m_raw_hist_enable);
	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(m_raw_hist_enable);
	((CButton *)GetDlgItem(IDC_RADIO33))->SetCheck(!m_rgb_hist_enable);
	((CButton *)GetDlgItem(IDC_RADIO34))->SetCheck(m_rgb_hist_enable);
	((CButton *)GetDlgItem(IDC_RADIO35))->SetCheck(!m_yuv_hist_enable);
	((CButton *)GetDlgItem(IDC_RADIO36))->SetCheck(m_yuv_hist_enable);
	((CButton *)GetDlgItem(IDC_RADIO37))->SetCheck(!m_exp_mode);
	((CButton *)GetDlgItem(IDC_RADIO38))->SetCheck(m_exp_mode);
	
}

int CExpDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_EXP))) return -1;

	GetDataValue();

	memcpy(pPageInfoSt, &m_Exp, sizeof(AK_ISP_INIT_EXP));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_EXP);

	return 0;
}


int CExpDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_EXP))) return -1;

	memcpy(&m_Exp, pPageInfoSt, sizeof(AK_ISP_INIT_EXP));

	SetDataValue();
	
	return 0;
}

void CExpDlg::OnButtonManualExp() 
{
	// TODO: Add your control notification handler code here
	memcpy(&m_MEDlg.m_FrameRate, &m_Exp.p_frame_rate, sizeof(AK_ISP_FRAME_RATE_ATTR));

	if (IDOK == m_MEDlg.DoModal())
	{
		memcpy(&m_Exp.p_frame_rate, &m_MEDlg.m_FrameRate, sizeof(AK_ISP_FRAME_RATE_ATTR));
	}
}

void CExpDlg::OnButtonOther() 
{
	// TODO: Add your control notification handler code here
	memcpy(&m_AEOtherDlg.m_AEOther, m_Exp.p_ae.envi_gain_range, sizeof(AK_ISP_AE_OTHER));

	if (IDOK == m_AEOtherDlg.DoModal())
	{
		memcpy(m_Exp.p_ae.envi_gain_range, &m_AEOtherDlg.m_AEOther, sizeof(AK_ISP_AE_OTHER));
	}

}


void CExpDlg::Clean(void) 
{
	ZeroMemory(&m_Exp, sizeof(AK_ISP_INIT_EXP));
	SetDataValue();
	UpdateData(FALSE);
}



int CExpDlg::Convert_v2_data(AK_ISP_INIT_EXP *struct_new, AK_ISP_INIT_EXP_V2* struct_v2) 
{
	if ((struct_v2 == NULL) || (struct_new == NULL)) return -1;
	
	struct_new->param_id = ISP_EXP;
	struct_new->length = sizeof(AK_ISP_INIT_EXP);

	memcpy(&struct_new->p_raw_hist, &struct_v2->p_raw_hist, sizeof(AK_ISP_RAW_HIST_ATTR));
	memcpy(&struct_new->p_rgb_hist, &struct_v2->p_rgb_hist, sizeof(AK_ISP_RGB_HIST_ATTR));
	memcpy(&struct_new->p_yuv_hist, &struct_v2->p_yuv_hist, sizeof(AK_ISP_YUV_HIST_ATTR));
	memcpy(&struct_new->p_exp_type, &struct_v2->p_exp_type, sizeof(AK_ISP_EXP_TYPE));
	memcpy(&struct_new->p_frame_rate, &struct_v2->p_me, sizeof(AK_ISP_FRAME_RATE_ATTR));
	memcpy(&struct_new->p_ae, &struct_v2->p_ae, sizeof(AK_ISP_AE_ATTR));

	return 0;
}

