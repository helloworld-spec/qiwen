// ContrastDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "ContrastDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CContrastDlg dialog


CContrastDlg::CContrastDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CContrastDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CContrastDlg)
	m_mode = 0;
	m_envi = 0;
	//}}AFX_DATA_INIT
}


void CContrastDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CContrastDlg)
	DDX_Control(pDX, IDC_SPIN_Y_SHIFT, m_spin_y_shift);
	DDX_Control(pDX, IDC_SPIN_Y_CONSTRAST, m_spin_y_constrast);
	DDX_Control(pDX, IDC_SLIDER_Y_SHIFT, m_slider_y_shift);
	DDX_Control(pDX, IDC_SLIDER_Y_CONSTRAST, m_slider_y_constrast);
	DDX_Control(pDX, IDC_SLIDER3, m_shiftmax_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_rate_slider);
	DDX_Control(pDX, IDC_SLIDER1, m_area_slider);
	DDX_Control(pDX, IDC_SPIN3, m_shiftmax);
	DDX_Control(pDX, IDC_SPIN2, m_rate);
	DDX_Control(pDX, IDC_SPIN1, m_area);
	DDX_Radio(pDX, IDC_RADIO10, m_mode);
	DDX_Radio(pDX, IDC_RADIO1, m_envi);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CContrastDlg, CDialog)
	//{{AFX_MSG_MAP(CContrastDlg)
	ON_BN_CLICKED(IDC_BUTTON_CONSTRAST_GET, OnButtonConstrastGet)
	ON_BN_CLICKED(IDC_BUTTON_CONSTRAST_SET, OnButtonConstrastSet)
	ON_EN_KILLFOCUS(IDC_EDIT_Y_CONSTRAST, OnKillfocusEditYConstrast)
	ON_EN_KILLFOCUS(IDC_EDIT_Y_SHIFT, OnKillfocusEditYShift)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_Y_CONSTRAST, OnCustomdrawSliderYConstrast)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_Y_SHIFT, OnCustomdrawSliderYShift)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Y_CONSTRAST, OnDeltaposSpinYConstrast)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_Y_SHIFT, OnDeltaposSpinYShift)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, OnDeltaposSpin2)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnKillfocusEdit2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, OnDeltaposSpin3)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnKillfocusEdit3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider3)
	ON_BN_CLICKED(IDC_RADIO10, OnRadio10)
	ON_BN_CLICKED(IDC_RADIO11, OnRadio11)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnRadio6)
	ON_BN_CLICKED(IDC_RADIO7, OnRadio7)
	ON_BN_CLICKED(IDC_RADIO8, OnRadio8)
	ON_BN_CLICKED(IDC_RADIO9, OnRadio9)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContrastDlg message handlers

BOOL CContrastDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Isp_contrast, sizeof(AK_ISP_INIT_CONTRAST));
	
	// TODO: Add extra initialization here
	m_spin_y_constrast.SetRange(0,255);
	m_spin_y_constrast.SetPos(0);
	m_slider_y_constrast.SetRange(0,255);
	m_slider_y_constrast.SetPos(0);
	
	m_spin_y_shift.SetRange(0,511);
	m_spin_y_shift.SetPos(0);
	m_slider_y_shift.SetRange(0,511);
	m_slider_y_shift.SetPos(0);

	m_area.SetRange(0,511);
	m_area.SetPos(0);
	m_area_slider.SetRange(0,511);
	m_area_slider.SetPos(0);

	m_rate.SetRange(1,256);
	m_rate.SetPos(1);
	m_rate_slider.SetRange(1,256);
	m_rate_slider.SetPos(1);

	m_shiftmax.SetRange(0,127);
	m_shiftmax.SetPos(0);
	m_shiftmax_slider.SetRange(0,127);
	m_shiftmax_slider.SetPos(0);

	EnableLinkageRadio(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CContrastDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CContrastDlg::OnButtonConstrastGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_CONTRAST, 0);
}

void CContrastDlg::OnButtonConstrastSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_CONTRAST, 0);
}

void CContrastDlg::OnKillfocusEditYConstrast() 
{
	// TODO: Add your control notification handler code here
	m_slider_y_constrast.SetPos(m_spin_y_constrast.GetPos());
}

void CContrastDlg::OnKillfocusEditYShift() 
{
	// TODO: Add your control notification handler code here
	m_slider_y_shift.SetPos(m_spin_y_shift.GetPos());
}

void CContrastDlg::OnCustomdrawSliderYConstrast(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_y_constrast.SetPos(m_slider_y_constrast.GetPos());
	*pResult = 0;
}

void CContrastDlg::OnCustomdrawSliderYShift(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_y_shift.SetPos(m_slider_y_shift.GetPos());
	*pResult = 0;
}

void CContrastDlg::OnDeltaposSpinYConstrast(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_y_constrast.SetPos(m_spin_y_constrast.GetPos());
	*pResult = 0;
}

void CContrastDlg::OnDeltaposSpinYShift(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_y_shift.SetPos(m_spin_y_shift.GetPos());
	*pResult = 0;
}

void CContrastDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_CONTRAST))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}
	
	file.Read(&m_Isp_contrast, sizeof(AK_ISP_INIT_CONTRAST));
	file.Close();
	
	SetDataValue(TRUE);
}

void CContrastDlg::OnButtonWrite() 
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
	
	GetDataValue(TRUE);
	
	file.Write(&m_Isp_contrast, sizeof(AK_ISP_INIT_CONTRAST));
	
	file.Close();
}


void CContrastDlg::GetDataValue(bool bToStruct)
{
	int mode_tmp = 0;
	int envi_tmp = 0;

	if (!bToStruct)
	{
		mode_tmp = m_mode;
		envi_tmp = m_envi;
		UpdateData(TRUE);
	}
	else
	{
		UpdateData(TRUE);
		mode_tmp = m_mode;
		envi_tmp = m_envi;
	}
	
	m_Isp_contrast.param_id = ISP_CONTRAST;
	m_Isp_contrast.length = sizeof(AK_ISP_INIT_CONTRAST);

	m_Isp_contrast.p_contrast.cc_mode = (T_U16)m_mode;

	m_Isp_contrast.p_contrast.manual_contrast.y_contrast = (T_S16)m_spin_y_constrast.GetPos();
	m_Isp_contrast.p_contrast.manual_contrast.y_shift = (T_U16)m_spin_y_shift.GetPos();

	m_Isp_contrast.p_contrast.linkage_contrast[envi_tmp].dark_pixel_area = (T_U16)m_area.GetPos();
	m_Isp_contrast.p_contrast.linkage_contrast[envi_tmp].dark_pixel_rate = (T_S16)m_rate.GetPos();
	m_Isp_contrast.p_contrast.linkage_contrast[envi_tmp].shift_max = (T_U16)m_shiftmax.GetPos();

}

void CContrastDlg::SetDataValue(bool bFromStruct)
{
	if (bFromStruct)
	{
		m_mode = m_Isp_contrast.p_contrast.cc_mode;
	}

	m_spin_y_constrast.SetPos(m_Isp_contrast.p_contrast.manual_contrast.y_contrast);
	m_slider_y_constrast.SetPos(m_Isp_contrast.p_contrast.manual_contrast.y_contrast);
	m_spin_y_shift.SetPos(m_Isp_contrast.p_contrast.manual_contrast.y_shift);
	m_slider_y_shift.SetPos(m_Isp_contrast.p_contrast.manual_contrast.y_shift);

	m_area.SetPos(m_Isp_contrast.p_contrast.linkage_contrast[m_envi].dark_pixel_area);
	m_area_slider.SetPos(m_Isp_contrast.p_contrast.linkage_contrast[m_envi].dark_pixel_area);
	m_rate.SetPos(m_Isp_contrast.p_contrast.linkage_contrast[m_envi].dark_pixel_rate);
	m_rate_slider.SetPos(m_Isp_contrast.p_contrast.linkage_contrast[m_envi].dark_pixel_rate);
	m_shiftmax.SetPos(m_Isp_contrast.p_contrast.linkage_contrast[m_envi].shift_max);
	m_shiftmax_slider.SetPos(m_Isp_contrast.p_contrast.linkage_contrast[m_envi].shift_max);

	if (0 == m_mode)
	{
		EnableLinkageRadio(FALSE);
	}
	else
	{
		EnableLinkageRadio(TRUE);
	}

	((CButton *)GetDlgItem(IDC_RADIO10))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO11))->SetCheck(m_mode);
}

int CContrastDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_CONTRAST))) return -1;
	
	GetDataValue(TRUE);
	
	memcpy(pPageInfoSt, &m_Isp_contrast, sizeof(AK_ISP_INIT_CONTRAST));
	
	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_CONTRAST);
	
	return 0;
}

int CContrastDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_CONTRAST))) return -1;
	
	memcpy(&m_Isp_contrast, pPageInfoSt, sizeof(AK_ISP_INIT_CONTRAST));
	
	SetDataValue(TRUE);
	
	return 0;
}

void CContrastDlg::EnableLinkageRadio(bool bEnable) 
{
	CWnd *pRadio[9];
	
	pRadio[0] = GetDlgItem(IDC_RADIO1);
	pRadio[1] = GetDlgItem(IDC_RADIO2);
	pRadio[2] = GetDlgItem(IDC_RADIO3);
	pRadio[3] = GetDlgItem(IDC_RADIO4);
	pRadio[4] = GetDlgItem(IDC_RADIO5);
	pRadio[5] = GetDlgItem(IDC_RADIO6);
	pRadio[6] = GetDlgItem(IDC_RADIO7);
	pRadio[7] = GetDlgItem(IDC_RADIO8);
	pRadio[8] = GetDlgItem(IDC_RADIO9);

	for (int i=0; i<9; i++)
	{
		pRadio[i]->EnableWindow(bEnable);
	}
	
}

void CContrastDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_area_slider.SetPos(m_area.GetPos());
	*pResult = 0;
}

void CContrastDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_area_slider.SetPos(m_area.GetPos());
}

void CContrastDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_area.SetPos(m_area_slider.GetPos());
	*pResult = 0;
}

void CContrastDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_rate_slider.SetPos(m_rate.GetPos());
	*pResult = 0;
}

void CContrastDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_rate_slider.SetPos(m_rate.GetPos());
}

void CContrastDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_rate.SetPos(m_rate_slider.GetPos());
	*pResult = 0;
}

void CContrastDlg::OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_shiftmax_slider.SetPos(m_shiftmax.GetPos());
	*pResult = 0;
}

void CContrastDlg::OnKillfocusEdit3() 
{
	// TODO: Add your control notification handler code here
	m_shiftmax_slider.SetPos(m_shiftmax.GetPos());
}

void CContrastDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_shiftmax.SetPos(m_shiftmax_slider.GetPos());
	*pResult = 0;
}


void CContrastDlg::OnRadio10() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(FALSE);
	UpdateData(FALSE);
}

void CContrastDlg::OnRadio11() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(TRUE);
	UpdateData(FALSE);
}

void CContrastDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CContrastDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CContrastDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CContrastDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CContrastDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CContrastDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CContrastDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CContrastDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CContrastDlg::OnRadio9() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CContrastDlg::Clean(void) 
{
	ZeroMemory(&m_Isp_contrast, sizeof(AK_ISP_INIT_CONTRAST));
	EnableLinkageRadio(FALSE);
	SetDataValue(TRUE);
	UpdateData(FALSE);
}
