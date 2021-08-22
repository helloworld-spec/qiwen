// MISCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "MISCDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMISCDlg dialog


CMISCDlg::CMISCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMISCDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMISCDlg)
	m_hsyn_pol = 0;
	m_vsync_pol = 0;
	m_pattern_en = 0;
	m_fsd_en = 0;
	m_pclk = 0;
	m_flip_en = 0;
	m_mirror_en = 0;
	m_oneline_cycle_value = 0;
	m_hblank_cycle_value = 0;
	//}}AFX_DATA_INIT
}


void CMISCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMISCDlg)
	DDX_Control(pDX, IDC_SLIDER3, m_fsd_num_slider);
	DDX_Control(pDX, IDC_SPIN5, m_fsd_num);
	DDX_Control(pDX, IDC_SPIN4, m_rawhblank_cycle);
	DDX_Control(pDX, IDC_SPIN8, m_yuvhblank_cycle);
	DDX_Control(pDX, IDC_SPIN3, m_oneline_cycle);
	DDX_Control(pDX, IDC_SLIDER2, m_cfa_mode_slider);
	DDX_Control(pDX, IDC_SPIN2, m_cfa_mode);
	DDX_Control(pDX, IDC_SLIDER1, m_pattern_cfg_slider);
	DDX_Control(pDX, IDC_SPIN1, m_pattern_cfg);
	DDX_Control(pDX, IDC_SPIN10, m_data_w);
	DDX_Control(pDX, IDC_SLIDER5, m_data_w_slider);
	DDX_Radio(pDX, IDC_RADIO35, m_pattern_en);
	DDX_Radio(pDX, IDC_RADIO33, m_vsync_pol);
	DDX_Radio(pDX, IDC_RADIO1, m_hsyn_pol);
	DDX_Radio(pDX, IDC_RADIO37, m_fsd_en);
	DDX_Radio(pDX, IDC_RADIO39, m_pclk);
	DDX_Radio(pDX, IDC_RADIO88, m_flip_en);
	DDX_Radio(pDX, IDC_RADIO90, m_mirror_en);
	DDX_Text(pDX, IDC_EDIT3, m_oneline_cycle_value);
	DDX_Text(pDX, IDC_EDIT4, m_hblank_cycle_value);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMISCDlg, CDialog)
	//{{AFX_MSG_MAP(CMISCDlg)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, OnDeltaposSpin2)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnKillfocusEdit2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN5, OnDeltaposSpin5)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnKillfocusEdit5)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN10, OnDeltaposSpin10)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnKillfocusEdit6)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER5, OnCustomdrawSlider5)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO33, OnRadio33)
	ON_BN_CLICKED(IDC_RADIO34, OnRadio34)
	ON_BN_CLICKED(IDC_RADIO35, OnRadio35)
	ON_BN_CLICKED(IDC_RADIO36, OnRadio36)
	ON_BN_CLICKED(IDC_RADIO37, OnRadio37)
	ON_BN_CLICKED(IDC_RADIO38, OnRadio38)
	ON_BN_CLICKED(IDC_RADIO39, OnRadio39)
	ON_BN_CLICKED(IDC_RADIO87, OnRadio87)
	ON_BN_CLICKED(IDC_RADIO88, OnRadio88)
	ON_BN_CLICKED(IDC_RADIO89, OnRadio89)
	ON_BN_CLICKED(IDC_RADIO90, OnRadio90)
	ON_BN_CLICKED(IDC_RADIO91, OnRadio91)
	ON_BN_CLICKED(IDC_BUTTON7, OnButtonOther)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMISCDlg message handlers
BOOL CMISCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Misc, sizeof(AK_ISP_INIT_MISC));

	m_pattern_cfg.SetRange(0,8);
	m_pattern_cfg.SetPos(0);
	m_pattern_cfg_slider.SetRange(0,8);
	m_pattern_cfg_slider.SetPos(0);

	m_cfa_mode.SetRange(0,3);
	m_cfa_mode.SetPos(0);
	m_cfa_mode_slider.SetRange(0,3);
	m_cfa_mode_slider.SetPos(0);

	m_data_w.SetRange(0,2);
	m_data_w.SetPos(0);
	m_data_w_slider.SetRange(0,2);
	m_data_w_slider.SetPos(0);

	m_oneline_cycle.SetRange32(0,65535);
	m_oneline_cycle.SetPos(0);


	m_rawhblank_cycle.SetRange32(0,4095);
	m_rawhblank_cycle.SetPos(0);

	m_yuvhblank_cycle.SetRange32(0,2047);
	m_yuvhblank_cycle.SetPos(0);


	m_fsd_num.SetRange(0,64);
	m_fsd_num.SetPos(0);
	m_fsd_num_slider.SetRange(0,64);
	m_fsd_num_slider.SetPos(0);


	return TRUE;
}

BOOL CMISCDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CMISCDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_MISC))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Misc, sizeof(AK_ISP_INIT_MISC));
	file.Close();

	SetDataValue();

	UpdateData(FALSE);
}

void CMISCDlg::OnButtonWrite() 
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

	file.Write(&m_Misc, sizeof(AK_ISP_INIT_MISC));

	file.Close();
}

void CMISCDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_MISC, 0);
}

void CMISCDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_MISC, 0);
}

void CMISCDlg::GetDataValue(void)
{
	UpdateData(TRUE);


	m_Misc.param_id = ISP_MISC;
	m_Misc.length = sizeof(AK_ISP_INIT_MISC);
	
	m_Misc.p_misc.hsyn_pol = (T_U16)m_hsyn_pol;
	m_Misc.p_misc.vsync_pol = (T_U16)m_vsync_pol;
	m_Misc.p_misc.pclk_pol = (T_U16)m_pclk;
	
	m_Misc.p_misc.test_pattern_en = (T_U16)m_pattern_en;
	m_Misc.p_misc.frame_start_delay_en = (T_U16)m_fsd_en;
	

	m_Misc.p_misc.test_pattern_cfg = (T_U16)m_pattern_cfg.GetPos();
	m_Misc.p_misc.cfa_mode = (T_U16)m_cfa_mode.GetPos();
	m_Misc.p_misc.inputdataw = (T_U16)m_data_w.GetPos();
	
	m_Misc.p_misc.one_line_cycle = (T_U16)m_oneline_cycle.GetPos();
	m_Misc.p_misc.hblank_cycle = (T_U16)((T_U16)m_rawhblank_cycle.GetPos()) | (((T_U16)m_yuvhblank_cycle.GetPos() / 128) << 12);
	m_Misc.p_misc.frame_start_delay_num = (T_U16)m_fsd_num.GetPos();

	m_Misc.p_misc.flip_en = (T_U16)m_flip_en;
	m_Misc.p_misc.mirror_en = (T_U16)m_mirror_en;

}

void CMISCDlg::SetDataValue(void)
{
	CWnd *pWnd = NULL;
	CString str;
	
	m_hsyn_pol = m_Misc.p_misc.hsyn_pol;
	m_vsync_pol = m_Misc.p_misc.vsync_pol;
	m_pattern_en = m_Misc.p_misc.test_pattern_en;
	m_fsd_en = m_Misc.p_misc.frame_start_delay_en;
	m_pclk = m_Misc.p_misc.pclk_pol;

	m_pattern_cfg.SetPos(m_Misc.p_misc.test_pattern_cfg);
	m_pattern_cfg_slider.SetPos(m_Misc.p_misc.test_pattern_cfg);
	m_cfa_mode.SetPos(m_Misc.p_misc.cfa_mode);
	m_cfa_mode_slider.SetPos(m_Misc.p_misc.cfa_mode);
	m_data_w.SetPos(m_Misc.p_misc.inputdataw);
	m_data_w_slider.SetPos(m_Misc.p_misc.inputdataw);
	m_fsd_num.SetPos(m_Misc.p_misc.frame_start_delay_num);
	m_fsd_num_slider.SetPos(m_Misc.p_misc.frame_start_delay_num);

	m_oneline_cycle.SetPos(m_Misc.p_misc.one_line_cycle);
	m_oneline_cycle_value = m_Misc.p_misc.one_line_cycle;
	pWnd = GetDlgItem(IDC_EDIT3);
	str.Format("%d", m_oneline_cycle_value);
	pWnd->SetWindowText(str);

	m_rawhblank_cycle.SetPos(m_Misc.p_misc.hblank_cycle & 0xfff);
	m_hblank_cycle_value = m_Misc.p_misc.hblank_cycle & 0xfff;
	pWnd = GetDlgItem(IDC_EDIT4);
	str.Format("%d", m_hblank_cycle_value);
	pWnd->SetWindowText(str);


	m_yuvhblank_cycle.SetPos((m_Misc.p_misc.hblank_cycle >> 12) * 128);

	m_flip_en = m_Misc.p_misc.flip_en;
	m_mirror_en = m_Misc.p_misc.mirror_en;

	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(!m_hsyn_pol);
	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(m_hsyn_pol);
	((CButton *)GetDlgItem(IDC_RADIO33))->SetCheck(!m_vsync_pol);
	((CButton *)GetDlgItem(IDC_RADIO34))->SetCheck(m_vsync_pol);
	((CButton *)GetDlgItem(IDC_RADIO35))->SetCheck(!m_pattern_en);
	((CButton *)GetDlgItem(IDC_RADIO36))->SetCheck(m_pattern_en);
	((CButton *)GetDlgItem(IDC_RADIO37))->SetCheck(!m_fsd_en);
	((CButton *)GetDlgItem(IDC_RADIO38))->SetCheck(m_fsd_en);
	((CButton *)GetDlgItem(IDC_RADIO39))->SetCheck(!m_pclk);
	((CButton *)GetDlgItem(IDC_RADIO87))->SetCheck(m_pclk);
	((CButton *)GetDlgItem(IDC_RADIO88))->SetCheck(!m_flip_en);
	((CButton *)GetDlgItem(IDC_RADIO89))->SetCheck(m_flip_en);
	((CButton *)GetDlgItem(IDC_RADIO90))->SetCheck(!m_mirror_en);
	((CButton *)GetDlgItem(IDC_RADIO91))->SetCheck(m_mirror_en);
}

int CMISCDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_MISC))) return -1;

	GetDataValue();

	memcpy(pPageInfoSt, &m_Misc, sizeof(AK_ISP_INIT_MISC));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_MISC);

	return 0;
}


int CMISCDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_MISC))) return -1;

	memcpy(&m_Misc, pPageInfoSt, sizeof(AK_ISP_INIT_MISC));

	SetDataValue();
	
	return 0;
}


void CMISCDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_pattern_cfg_slider.SetPos(m_pattern_cfg.GetPos());
	*pResult = 0;
}

void CMISCDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_pattern_cfg_slider.SetPos(m_pattern_cfg.GetPos());
}

void CMISCDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_pattern_cfg.SetPos(m_pattern_cfg_slider.GetPos());
	*pResult = 0;
}

void CMISCDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_cfa_mode_slider.SetPos(m_cfa_mode.GetPos());
	*pResult = 0;
}

void CMISCDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_cfa_mode_slider.SetPos(m_cfa_mode.GetPos());
}

void CMISCDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_cfa_mode.SetPos(m_cfa_mode_slider.GetPos());
	*pResult = 0;
}

void CMISCDlg::OnDeltaposSpin5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_fsd_num_slider.SetPos(m_fsd_num.GetPos());
	*pResult = 0;
}

void CMISCDlg::OnKillfocusEdit5() 
{
	// TODO: Add your control notification handler code here
	m_fsd_num_slider.SetPos(m_fsd_num.GetPos());
}

void CMISCDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_fsd_num.SetPos(m_fsd_num_slider.GetPos());
	*pResult = 0;
}

void CMISCDlg::OnDeltaposSpin10(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_data_w_slider.SetPos(m_data_w.GetPos());
	*pResult = 0;
}

void CMISCDlg::OnKillfocusEdit6() 
{
	// TODO: Add your control notification handler code here
	m_data_w_slider.SetPos(m_data_w.GetPos());
}

void CMISCDlg::OnCustomdrawSlider5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_data_w.SetPos(m_data_w_slider.GetPos());
	*pResult = 0;
}

void CMISCDlg::OnRadio33() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMISCDlg::OnRadio34() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CMISCDlg::OnRadio35() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMISCDlg::OnRadio36() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMISCDlg::OnRadio37() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMISCDlg::OnRadio38() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMISCDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMISCDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CMISCDlg::OnRadio87() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMISCDlg::OnRadio39() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMISCDlg::OnRadio88() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMISCDlg::OnRadio89() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CMISCDlg::OnRadio90() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CMISCDlg::OnRadio91() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}





void CMISCDlg::Clean(void) 
{
	ZeroMemory(&m_Misc, sizeof(AK_ISP_INIT_MISC));
	SetDataValue();
	UpdateData(FALSE);
}

int CMISCDlg::Convert_v2_data(AK_ISP_INIT_MISC *struct_new, AK_ISP_INIT_MISC_V2* struct_v2) 
{
	if ((struct_v2 == NULL) || (struct_new == NULL)) return -1;
	
	struct_new->param_id = ISP_MISC;
	struct_new->length = sizeof(AK_ISP_INIT_MISC);

	struct_new->p_misc.hsyn_pol = struct_v2->p_misc.hsyn_pol;
	struct_new->p_misc.vsync_pol = struct_v2->p_misc.vsync_pol;
	struct_new->p_misc.pclk_pol = (struct_v2->p_misc.cfa_mode & 0x2000) >> 13;
	
	struct_new->p_misc.test_pattern_en = struct_v2->p_misc.test_pattern_en;
	struct_new->p_misc.frame_start_delay_en = struct_v2->p_misc.frame_start_delay_en;
	

	struct_new->p_misc.test_pattern_cfg = struct_v2->p_misc.test_pattern_cfg;
	struct_new->p_misc.cfa_mode = struct_v2->p_misc.cfa_mode & 0x3;
	struct_new->p_misc.inputdataw = (struct_v2->p_misc.cfa_mode & 0xC000) >> 14;
	
	struct_new->p_misc.one_line_cycle = struct_v2->p_misc.one_line_cycle;
	struct_new->p_misc.hblank_cycle = struct_v2->p_misc.hblank_cycle;
	struct_new->p_misc.frame_start_delay_num = struct_v2->p_misc.frame_start_delay_num;

	struct_new->p_misc.flip_en = 0;
	struct_new->p_misc.mirror_en = 0;
	struct_new->p_misc.twoframe_merge_en = 0;
	struct_new->p_misc.mipi_line_end_sel = 0;
	struct_new->p_misc.mipi_line_end_cnt_en_cfg = 0;
	struct_new->p_misc.mipi_count_time = 22;

	return 0;
}


int CMISCDlg::Convert_v3_data(AK_ISP_INIT_MISC *struct_new, AK_ISP_INIT_MISC_V3* struct_v3) 
{
	if ((struct_v3 == NULL) || (struct_new == NULL)) return -1;
	
	struct_new->param_id = ISP_MISC;
	struct_new->length = sizeof(AK_ISP_INIT_MISC);

	struct_new->p_misc.hsyn_pol = struct_v3->p_misc.hsyn_pol;
	struct_new->p_misc.vsync_pol = struct_v3->p_misc.vsync_pol;
	struct_new->p_misc.pclk_pol = struct_v3->p_misc.pclk_pol;
	
	struct_new->p_misc.test_pattern_en = struct_v3->p_misc.test_pattern_en;
	struct_new->p_misc.frame_start_delay_en = struct_v3->p_misc.frame_start_delay_en;
	

	struct_new->p_misc.test_pattern_cfg = struct_v3->p_misc.test_pattern_cfg;
	struct_new->p_misc.cfa_mode = struct_v3->p_misc.cfa_mode;
	struct_new->p_misc.inputdataw = struct_v3->p_misc.inputdataw;
	
	struct_new->p_misc.one_line_cycle = struct_v3->p_misc.one_line_cycle;
	struct_new->p_misc.hblank_cycle = struct_v3->p_misc.hblank_cycle;
	struct_new->p_misc.frame_start_delay_num = struct_v3->p_misc.frame_start_delay_num;

	struct_new->p_misc.flip_en = struct_v3->p_misc.flip_en;
	struct_new->p_misc.mirror_en = struct_v3->p_misc.mirror_en;

	struct_new->p_misc.twoframe_merge_en = 0;
	struct_new->p_misc.mipi_line_end_sel = 0;
	struct_new->p_misc.mipi_line_end_cnt_en_cfg = 0;
	struct_new->p_misc.mipi_count_time = 22;

	return 0;
}


void CMISCDlg::OnButtonOther() 
{
	// TODO: Add your control notification handler code here
	memcpy(&m_OtherDlg.m_Misc_attr, &m_Misc.p_misc, sizeof(AK_ISP_MISC_ATTR));

	if (IDOK == m_OtherDlg.DoModal())
	{
		memcpy(&m_Misc.p_misc, &m_OtherDlg.m_Misc_attr, sizeof(AK_ISP_MISC_ATTR));
	}
}
