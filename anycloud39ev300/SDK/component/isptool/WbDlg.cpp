// WbDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "WbDlg.h"
#include "Netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWbDlg dialog


CWbDlg::CWbDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWbDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWbDlg)
	//}}AFX_DATA_INIT
}


void CWbDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWbDlg)
	DDX_Control(pDX, IDC_SPIN_AWB_MWB_R_OFFSET, m_spin_awb_mwb_r_offset);
	DDX_Control(pDX, IDC_SPIN_AWB_MWB_R_GAIN, m_spin_awb_mwb_r_gain);
	DDX_Control(pDX, IDC_SPIN_AWB_MWB_G_OFFSET, m_spin_awb_mwb_g_offset);
	DDX_Control(pDX, IDC_SPIN_AWB_MWB_G_GAIN, m_spin_awb_mwb_g_gain);
	DDX_Control(pDX, IDC_SPIN_AWB_MWB_B_OFFSET, m_spin_awb_mwb_b_offset);
	DDX_Control(pDX, IDC_SPIN_AWB_MWB_B_GAIN, m_spin_awb_mwb_b_gain);
	DDX_Control(pDX, IDC_SLIDER_AWB_MWB_R_OFFSET, m_slider_awb_mwb_r_offset);
	DDX_Control(pDX, IDC_SLIDER_AWB_MWB_R_GAIN, m_slider_awb_mwb_r_gain);
	DDX_Control(pDX, IDC_SLIDER_AWB_MWB_G_OFFSET, m_slider_awb_mwb_g_offset);
	DDX_Control(pDX, IDC_SLIDER_AWB_MWB_G_GAIN, m_slider_awb_mwb_g_gain);
	DDX_Control(pDX, IDC_SLIDER_AWB_MWB_B_OFFSET, m_slider_awb_mwb_b_offset);
	DDX_Control(pDX, IDC_SLIDER_AWB_MWB_B_GAIN, m_slider_awb_mwb_b_gain);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWbDlg, CDialog)
	//{{AFX_MSG_MAP(CWbDlg)
	ON_BN_CLICKED(IDC_BUTTON_WB_READ, OnButtonWbRead)
	ON_BN_CLICKED(IDC_BUTTON_WB_SAVE, OnButtonWbSave)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_G_WEIGHT, OnButtonGWeight)
	ON_BN_CLICKED(IDC_BUTTON_AWB_STAT, OnButtonAwbStat)
	ON_EN_KILLFOCUS(IDC_EDIT_AWB_MWB_R_GAIN, OnKillfocusEditAwbMwbRGain)
	ON_EN_KILLFOCUS(IDC_EDIT_AWB_MWB_R_OFFSET, OnKillfocusEditAwbMwbROffset)
	ON_EN_KILLFOCUS(IDC_EDIT_AWB_MWB_B_GAIN, OnKillfocusEditAwbMwbBGain)
	ON_EN_KILLFOCUS(IDC_EDIT_AWB_MWB_B_OFFSET, OnKillfocusEditAwbMwbBOffset)
	ON_EN_KILLFOCUS(IDC_EDIT_AWB_MWB_G_GAIN, OnKillfocusEditAwbMwbGGain)
	ON_EN_KILLFOCUS(IDC_EDIT_AWB_MWB_G_OFFSET, OnKillfocusEditAwbMwbGOffset)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_AWB_MWB_B_GAIN, OnCustomdrawSliderAwbMwbBGain)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_AWB_MWB_B_OFFSET, OnCustomdrawSliderAwbMwbBOffset)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_AWB_MWB_G_GAIN, OnCustomdrawSliderAwbMwbGGain)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_AWB_MWB_G_OFFSET, OnCustomdrawSliderAwbMwbGOffset)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_AWB_MWB_R_GAIN, OnCustomdrawSliderAwbMwbRGain)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_AWB_MWB_R_OFFSET, OnCustomdrawSliderAwbMwbROffset)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_AWB_MWB_B_GAIN, OnDeltaposSpinAwbMwbBGain)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_AWB_MWB_B_OFFSET, OnDeltaposSpinAwbMwbBOffset)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_AWB_MWB_G_GAIN, OnDeltaposSpinAwbMwbGGain)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_AWB_MWB_G_OFFSET, OnDeltaposSpinAwbMwbGOffset)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_AWB_MWB_R_GAIN, OnDeltaposSpinAwbMwbRGain)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_AWB_MWB_R_OFFSET, OnDeltaposSpinAwbMwbROffset)
	ON_BN_CLICKED(IDC_RADIO_MWB, OnRadioMwb)
	ON_BN_CLICKED(IDC_RADIO_AWB, OnRadioAwb)
	ON_BN_CLICKED(IDC_BUTTON_WB_EX_ATTR, OnButtonWbExAttr)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWbDlg message handlers

void CWbDlg::OnButtonWbRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_WB))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}
	
	file.Read(&m_Isp_wb, sizeof(AK_ISP_INIT_WB));
	file.Close();
	
	SetDataValue();

	if (NULL == m_pMessageWnd) 
	{
		AfxMessageBox("m_pMessageWnd is null", MB_OK);
		return;
	}
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_WB_INFO, DIALOG_STAT, 0);

}

void CWbDlg::OnButtonWbSave() 
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
	
	file.Write(&m_Isp_wb, sizeof(AK_ISP_INIT_WB));
	
	file.Close();
}

void CWbDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_WB, 0);
}

void CWbDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_WB, 0);
}

void CWbDlg::OnButtonGWeight() 
{
	// TODO: Add your control notification handler code here
	m_AWB_G_weightDlg.DoModal();
}

void CWbDlg::OnButtonAwbStat() 
{
	// TODO: Add your control notification handler code here
	m_AWB_wb_statDlg.open_dlg_flag = TRUE;
	m_AWB_wb_statDlg.DoModal();
}

void CWbDlg::OnRadioMwbEnable() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CWbDlg::OnRadioMwbDisable() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CWbDlg::OnKillfocusEditAwbMwbRGain() 
{
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_r_gain.SetPos(m_spin_awb_mwb_r_gain.GetPos());
}

void CWbDlg::OnKillfocusEditAwbMwbROffset() 
{
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_r_offset.SetPos((T_S16)m_spin_awb_mwb_r_offset.GetPos());
}

void CWbDlg::OnKillfocusEditAwbMwbBGain() 
{
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_b_gain.SetPos(m_spin_awb_mwb_b_gain.GetPos());
}

void CWbDlg::OnKillfocusEditAwbMwbBOffset() 
{
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_b_offset.SetPos((T_S16)m_spin_awb_mwb_b_offset.GetPos());
}

void CWbDlg::OnKillfocusEditAwbMwbGGain() 
{
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_g_gain.SetPos(m_spin_awb_mwb_g_gain.GetPos());
}

void CWbDlg::OnKillfocusEditAwbMwbGOffset() 
{
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_g_offset.SetPos((T_S16)m_spin_awb_mwb_g_offset.GetPos());
}

void CWbDlg::OnCustomdrawSliderAwbMwbBGain(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_awb_mwb_b_gain.SetPos(m_slider_awb_mwb_b_gain.GetPos());
	*pResult = 0;
}

void CWbDlg::OnCustomdrawSliderAwbMwbBOffset(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_awb_mwb_b_offset.SetPos((T_S16)m_slider_awb_mwb_b_offset.GetPos());
	*pResult = 0;
}

void CWbDlg::OnCustomdrawSliderAwbMwbGGain(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_awb_mwb_g_gain.SetPos(m_slider_awb_mwb_g_gain.GetPos());
	*pResult = 0;
}

void CWbDlg::OnCustomdrawSliderAwbMwbGOffset(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_awb_mwb_g_offset.SetPos((T_S16)m_slider_awb_mwb_g_offset.GetPos());
	*pResult = 0;
}

void CWbDlg::OnCustomdrawSliderAwbMwbRGain(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_awb_mwb_r_gain.SetPos(m_slider_awb_mwb_r_gain.GetPos());
	*pResult = 0;
}

void CWbDlg::OnCustomdrawSliderAwbMwbROffset(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_awb_mwb_r_offset.SetPos((T_S16)m_slider_awb_mwb_r_offset.GetPos());
	*pResult = 0;
}

void CWbDlg::OnDeltaposSpinAwbMwbBGain(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_b_gain.SetPos(m_spin_awb_mwb_b_gain.GetPos());
	*pResult = 0;
}

void CWbDlg::OnDeltaposSpinAwbMwbBOffset(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_b_offset.SetPos((T_S16)m_spin_awb_mwb_b_offset.GetPos());
	*pResult = 0;
}

void CWbDlg::OnDeltaposSpinAwbMwbGGain(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_g_gain.SetPos(m_spin_awb_mwb_g_gain.GetPos());
	*pResult = 0;
}

void CWbDlg::OnDeltaposSpinAwbMwbGOffset(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_g_offset.SetPos((T_S16)m_spin_awb_mwb_g_offset.GetPos());
	*pResult = 0;
}

void CWbDlg::OnDeltaposSpinAwbMwbRGain(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_r_gain.SetPos(m_spin_awb_mwb_r_gain.GetPos());
	*pResult = 0;
}

void CWbDlg::OnDeltaposSpinAwbMwbROffset(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_awb_mwb_r_offset.SetPos((T_S16)m_spin_awb_mwb_r_offset.GetPos());
	*pResult = 0;
}

void CWbDlg::GetDataValue(void)
{
	BOOL check = FALSE;
	int len = 0;

	UpdateData(TRUE);
	
	m_Isp_wb.param_id = ISP_WB;
	m_Isp_wb.length = sizeof(AK_ISP_INIT_WB);

	check = ((CButton *)GetDlgItem(IDC_RADIO_AWB))->GetCheck();
	if (check)
	{
		m_Isp_wb.wb_type.wb_type = 1;
	}
	else
	{
		m_Isp_wb.wb_type.wb_type = 0;
	}
	
	m_Isp_wb.p_mwb.r_gain = (T_U16)m_spin_awb_mwb_r_gain.GetPos();
	m_Isp_wb.p_mwb.r_offset  = (T_S16)m_spin_awb_mwb_r_offset.GetPos();

	m_Isp_wb.p_mwb.g_gain = (T_U16)m_spin_awb_mwb_g_gain.GetPos();
	m_Isp_wb.p_mwb.g_offset  = (T_S16)m_spin_awb_mwb_g_offset.GetPos();

	m_Isp_wb.p_mwb.b_gain = (T_U16)m_spin_awb_mwb_b_gain.GetPos();
	m_Isp_wb.p_mwb.b_offset  = (T_S16)m_spin_awb_mwb_b_offset.GetPos();

	len = sizeof(AK_ISP_AWB_ATTR);

	m_AWB_G_weightDlg.GetPageInfoSt_G_weight(&m_Isp_wb.p_awb, len);
	m_AWB_wb_statDlg.GetPageInfoSt_wb_stat(&m_Isp_wb.p_awb, len);
}

void CWbDlg::SetDataValue(void)
{
	m_spin_awb_mwb_r_gain.SetPos(m_Isp_wb.p_mwb.r_gain);
	m_slider_awb_mwb_r_gain.SetPos(m_Isp_wb.p_mwb.r_gain);
	m_spin_awb_mwb_r_offset.SetPos(m_Isp_wb.p_mwb.r_offset);
	m_slider_awb_mwb_r_offset.SetPos(m_Isp_wb.p_mwb.r_offset);

	m_spin_awb_mwb_g_gain.SetPos(m_Isp_wb.p_mwb.g_gain);
	m_slider_awb_mwb_g_gain.SetPos(m_Isp_wb.p_mwb.g_gain);
	m_spin_awb_mwb_g_offset.SetPos(m_Isp_wb.p_mwb.g_offset);
	m_slider_awb_mwb_g_offset.SetPos(m_Isp_wb.p_mwb.g_offset);

	m_spin_awb_mwb_b_gain.SetPos(m_Isp_wb.p_mwb.b_gain);
	m_slider_awb_mwb_b_gain.SetPos(m_Isp_wb.p_mwb.b_gain);
	m_spin_awb_mwb_b_offset.SetPos(m_Isp_wb.p_mwb.b_offset);
	m_slider_awb_mwb_b_offset.SetPos(m_Isp_wb.p_mwb.b_offset);


	if (m_Isp_wb.wb_type.wb_type)
	{
		((CButton *)GetDlgItem(IDC_RADIO_AWB))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_MWB))->SetCheck(0);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_AWB))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_MWB))->SetCheck(1);
	}

	m_AWB_G_weightDlg.SetPageInfoSt_G_weight(&m_Isp_wb.p_awb, sizeof(AK_ISP_AWB_ATTR));
	m_AWB_wb_statDlg.SetPageInfoSt_wb_stat(&m_Isp_wb.p_awb, sizeof(AK_ISP_AWB_ATTR));
	
}


int CWbDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_WB))) return -1;
	
	GetDataValue();
	
	memcpy(pPageInfoSt, &m_Isp_wb, sizeof(AK_ISP_INIT_WB));
	
	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_WB);
	
	return 0;
}

int CWbDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_WB))) return -1;
	
	memcpy(&m_Isp_wb, pPageInfoSt, sizeof(AK_ISP_INIT_WB));
	
	SetDataValue();
	
	return 0;
}

void CWbDlg::OnRadioMwb() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CWbDlg::OnRadioAwb() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

BOOL CWbDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Isp_wb, sizeof(AK_ISP_INIT_WB));

	m_spin_awb_mwb_r_gain.SetRange(0,4095);
	m_spin_awb_mwb_r_gain.SetPos(0);
	m_slider_awb_mwb_r_gain.SetRange(0,4095);
	m_slider_awb_mwb_r_gain.SetPos(0);

	m_spin_awb_mwb_g_gain.SetRange(0,4095);
	m_spin_awb_mwb_g_gain.SetPos(0);
	m_slider_awb_mwb_g_gain.SetRange(0,4095);
	m_slider_awb_mwb_g_gain.SetPos(0);

	m_spin_awb_mwb_b_gain.SetRange(0,4095);
	m_spin_awb_mwb_b_gain.SetPos(0);
	m_slider_awb_mwb_b_gain.SetRange(0,4095);
	m_slider_awb_mwb_b_gain.SetPos(0);


	m_spin_awb_mwb_r_offset.SetRange(-512,511);
	m_spin_awb_mwb_r_offset.SetPos(0);
	m_slider_awb_mwb_r_offset.SetRange(-512,511, TRUE);
	m_slider_awb_mwb_r_offset.SetPos(0);

	m_spin_awb_mwb_g_offset.SetRange(-512,511);
	m_spin_awb_mwb_g_offset.SetPos(0);
	m_slider_awb_mwb_g_offset.SetRange(-512,511, TRUE);
	m_slider_awb_mwb_g_offset.SetPos(0);

	m_spin_awb_mwb_b_offset.SetRange(-512,511);
	m_spin_awb_mwb_b_offset.SetPos(0);
	m_slider_awb_mwb_b_offset.SetRange(-512,511, TRUE);
	m_slider_awb_mwb_b_offset.SetPos(0);

	if (m_Isp_wb.wb_type.wb_type)
	{
		((CButton *)GetDlgItem(IDC_RADIO_AWB))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_MWB))->SetCheck(0);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_AWB))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_MWB))->SetCheck(1);
	}


	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CWbDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CWbDlg::Clean(void) 
{
	ZeroMemory(&m_Isp_wb, sizeof(AK_ISP_INIT_WB));
	SetDataValue();
	UpdateData(FALSE);
}

void CWbDlg::OnButtonWbExAttr() 
{
	// TODO: Add your control notification handler code here
	
	memcpy(&m_exDlg.m_isp_awb_ex_attr, &m_Isp_wb.p_awb_ex, sizeof(AK_ISP_AWB_EX_ATTR));

	if (IDOK == m_exDlg.DoModal())
	{
		memcpy(&m_Isp_wb.p_awb_ex, &m_exDlg.m_isp_awb_ex_attr, sizeof(AK_ISP_AWB_EX_ATTR));
	}
}


int CWbDlg::Convert_v2_data(AK_ISP_INIT_WB *struct_new, AK_ISP_INIT_WB_V2* struct_v2) 
{
	if ((struct_v2 == NULL) || (struct_new == NULL)) return -1;
	
	struct_new->param_id = ISP_WB;
	struct_new->length = sizeof(AK_ISP_INIT_WB);

	memcpy(&struct_new->wb_type, &struct_v2->wb_type, sizeof(AK_ISP_WB_TYPE_ATTR));
	memcpy(&struct_new->p_mwb, &struct_v2->p_mwb, sizeof(AK_ISP_MWB_ATTR));
	memcpy(struct_new->p_awb.g_weight, struct_v2->p_awb.g_weight, 16 * sizeof(T_U16));
	struct_new->p_awb.y_low = struct_v2->p_awb.y_low;
	struct_new->p_awb.y_high = struct_v2->p_awb.y_high;
	struct_new->p_awb.auto_wb_step = struct_v2->p_awb.auto_wb_step;
	struct_new->p_awb.total_cnt_thresh = struct_v2->p_awb.total_cnt_thresh;
	struct_new->p_awb.colortemp_stable_cnt_thresh = struct_v2->p_awb.colortemp_stable_cnt_thresh;

	memcpy(struct_new->p_awb.gr_low, struct_v2->p_awb.gr_low, 5 * sizeof(T_U16));
	memcpy(struct_new->p_awb.gr_high, struct_v2->p_awb.gr_high, 5 * sizeof(T_U16));
	memcpy(struct_new->p_awb.gb_low, struct_v2->p_awb.gb_low, 5 * sizeof(T_U16));
	memcpy(struct_new->p_awb.gb_high, struct_v2->p_awb.gb_high, 5 * sizeof(T_U16));
	memcpy(struct_new->p_awb.rb_low, struct_v2->p_awb.rb_low, 5 * sizeof(T_U16));
	memcpy(struct_new->p_awb.rb_high, struct_v2->p_awb.rb_high, 5 * sizeof(T_U16));

	memcpy(&struct_new->p_awb.gr_low[5], struct_v2->p_awb_default.gr_low, 5 * sizeof(T_U16));
	memcpy(&struct_new->p_awb.gb_low[5], struct_v2->p_awb_default.gb_low, 5 * sizeof(T_U16));
	memcpy(&struct_new->p_awb.gr_high[5], struct_v2->p_awb_default.gr_high, 5 * sizeof(T_U16));
	memcpy(&struct_new->p_awb.gb_high[5], struct_v2->p_awb_default.gb_high, 5 * sizeof(T_U16));
	memcpy(&struct_new->p_awb.rb_low[5], struct_v2->p_awb_default.rb_low, 5 * sizeof(T_U16));
	memcpy(&struct_new->p_awb.rb_high[5], struct_v2->p_awb_default.rb_high, 5 * sizeof(T_U16));

	struct_new->p_awb.colortemp_envi[0] = struct_new->p_awb.colortemp_envi[5] = 0;
	struct_new->p_awb.colortemp_envi[1] = struct_new->p_awb.colortemp_envi[6] = 1;
	struct_new->p_awb.colortemp_envi[2] = struct_new->p_awb.colortemp_envi[7] = 2;
	struct_new->p_awb.colortemp_envi[3] = struct_new->p_awb.colortemp_envi[8] = 3;
	struct_new->p_awb.colortemp_envi[4] = struct_new->p_awb.colortemp_envi[9] = 3;
	
	return 0;
}

