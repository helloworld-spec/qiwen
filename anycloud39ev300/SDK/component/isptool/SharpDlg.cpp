// SharpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "SharpDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSharpDlg dialog



CSharpDlg::CSharpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSharpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSharpDlg)
	m_mode = 0;
	m_envi = 0;
	m_ysharp_Enable = 0;
	m_skin_det_Enable = 0;
	//}}AFX_DATA_INIT
}


void CSharpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSharpDlg)
	DDX_Control(pDX, IDC_SLIDER5, m_strength_slider);
	DDX_Control(pDX, IDC_SLIDER4, m_h_shift_slider);
	DDX_Control(pDX, IDC_SLIDER3, m_h_k_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_m_shift_slider);
	DDX_Control(pDX, IDC_SLIDER1, m_m_k_slider);
	DDX_Control(pDX, IDC_SPIN5, m_strength);
	DDX_Control(pDX, IDC_SPIN4, m_h_shift);
	DDX_Control(pDX, IDC_SPIN3, m_h_k);
	DDX_Control(pDX, IDC_SPIN2, m_m_shift);
	DDX_Control(pDX, IDC_SPIN1, m_m_k);
	DDX_Radio(pDX, IDC_RADIO10, m_mode);
	DDX_Radio(pDX, IDC_RADIO1, m_envi);
	DDX_Radio(pDX, IDC_RADIO12, m_ysharp_Enable);
	DDX_Radio(pDX, IDC_RADIO14, m_skin_det_Enable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSharpDlg, CDialog)
	//{{AFX_MSG_MAP(CSharpDlg)
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
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, OnDeltaposSpin3)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnKillfocusEdit3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, OnDeltaposSpin4)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnKillfocusEdit4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnCustomdrawSlider4)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN5, OnDeltaposSpin5)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnKillfocusEdit5)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER5, OnCustomdrawSlider5)
	ON_BN_CLICKED(IDC_RADIO10, OnRadio10)
	ON_BN_CLICKED(IDC_RADIO11, OnRadio11)
	ON_BN_CLICKED(IDC_RADIO12, OnRadio12)
	ON_BN_CLICKED(IDC_RADIO13, OnRadio13)
	ON_BN_CLICKED(IDC_RADIO14, OnRadio14)
	ON_BN_CLICKED(IDC_RADIO15, OnRadio15)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnRadio6)
	ON_BN_CLICKED(IDC_RADIO7, OnRadio7)
	ON_BN_CLICKED(IDC_RADIO8, OnRadio8)
	ON_BN_CLICKED(IDC_RADIO9, OnRadio9)
	ON_BN_CLICKED(IDC_BUTTON_LUT, OnButtonLut)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSharpDlg message handlers

BOOL CSharpDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Sharp, sizeof(AK_ISP_INIT_SHARP));

	m_m_k.SetRange(0,127);
	m_m_k.SetPos(0);
	m_m_k_slider.SetRange(0,127);
	m_m_k_slider.SetPos(0);

	m_m_shift.SetRange(0,15);
	m_m_shift.SetPos(0);
	m_m_shift_slider.SetRange(0,15);
	m_m_shift_slider.SetPos(0);

	m_h_k.SetRange(0,127);
	m_h_k.SetPos(0);
	m_h_k_slider.SetRange(0,127);
	m_h_k_slider.SetPos(0);

	m_h_shift.SetRange(0,15);
	m_h_shift.SetPos(0);
	m_h_shift_slider.SetRange(0,15);
	m_h_shift_slider.SetPos(0);

	m_strength.SetRange(0,255);
	m_strength.SetPos(0);
	m_strength_slider.SetRange(0,255);
	m_strength_slider.SetPos(0);

	EnableLinkageRadio(FALSE);

	int size = LINE_LEVEL_NUM * sizeof(short);
	m_LutDlg.m_MfLutDlg.GetLevel((char*)m_Sharp.p_sharp_attr.manual_sharp_attr.MF_HPF_LUT, &size);
	m_LutDlg.m_HfLutDlg.GetLevel((char*)m_Sharp.p_sharp_attr.manual_sharp_attr.HF_HPF_LUT, &size);

	for (int i=0; i<9; i++)
	{
		m_LutDlg.m_MfLutDlg.GetLevel((char*)m_Sharp.p_sharp_attr.linkage_sharp_attr[i].MF_HPF_LUT, &size);
		m_LutDlg.m_HfLutDlg.GetLevel((char*)m_Sharp.p_sharp_attr.linkage_sharp_attr[i].HF_HPF_LUT, &size);
	}

	for (i=0; i<10; i++)
	{
		m_Mf_keyPts[i].clear();
		m_Mf_keyPts[i].push_back(CPoint(0, LINE_WINDOW_HEIGHT/2));
		m_Mf_keyPts[i].push_back(CPoint(LINE_WINDOW_WIDTH-1, LINE_WINDOW_HEIGHT/2));

		m_Hf_keyPts[i].clear();
		m_Hf_keyPts[i].push_back(CPoint(0, LINE_WINDOW_HEIGHT/2));
		m_Hf_keyPts[i].push_back(CPoint(LINE_WINDOW_WIDTH-1, LINE_WINDOW_HEIGHT/2));
	}

	return TRUE;
}

BOOL CSharpDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CSharpDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_SHARP))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Sharp, sizeof(AK_ISP_INIT_SHARP));
	file.Close();

	SetDataValue(TRUE);

	UpdateData(FALSE);
}

void CSharpDlg::OnButtonWrite() 
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

	file.Write(&m_Sharp, sizeof(AK_ISP_INIT_SHARP));

	file.Close();
}

void CSharpDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_SHARP, 0);
}

void CSharpDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_SHARP, 0);
}

void CSharpDlg::EnableLinkageRadio(bool bEnable) 
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

void CSharpDlg::GetDataValue(bool bToStruct)
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


	m_Sharp.param_id = ISP_SHARP;
	m_Sharp.length = sizeof(AK_ISP_INIT_SHARP);
	
	m_Sharp.p_sharp_attr.ysharp_mode = (T_U16)m_mode;

	if (0 == mode_tmp)
	{
		m_Sharp.p_sharp_attr.manual_sharp_attr.ysharp_enable = (T_U16)m_ysharp_Enable;
		m_Sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_detect_enable = (T_U16)m_skin_det_Enable;

		m_Sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_k = (T_U16)m_m_k.GetPos();
		m_Sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_shift = (T_U16)m_m_shift.GetPos();
		m_Sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_k = (T_U16)m_h_k.GetPos();
		m_Sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_shift = (T_U16)m_h_shift.GetPos();
	}
	else
	{
		m_Sharp.p_sharp_attr.linkage_sharp_attr[envi_tmp].ysharp_enable = (T_U16)m_ysharp_Enable;
		m_Sharp.p_sharp_attr.linkage_sharp_attr[envi_tmp].sharp_skin_detect_enable = (T_U16)m_skin_det_Enable;

		m_Sharp.p_sharp_attr.linkage_sharp_attr[envi_tmp].mf_hpf_k = (T_U16)m_m_k.GetPos();
		m_Sharp.p_sharp_attr.linkage_sharp_attr[envi_tmp].mf_hpf_shift = (T_U16)m_m_shift.GetPos();
		m_Sharp.p_sharp_attr.linkage_sharp_attr[envi_tmp].hf_hpf_k = (T_U16)m_h_k.GetPos();
		m_Sharp.p_sharp_attr.linkage_sharp_attr[envi_tmp].hf_hpf_shift = (T_U16)m_h_shift.GetPos();
	}

	int size = m_Mf_keyPts[0].size();

	if (size > LINE_KEY_POINT_MAX)
	{
		size = LINE_KEY_POINT_MAX;
	}

	memset(&m_Sharp.p_sharp_attr.manual_sharp_attr.MF_LUT_KEY, 0, sizeof(T_U16) * LINE_KEY_POINT_MAX);

	for (int i=0; i<size; i++)
	{
		m_Sharp.p_sharp_attr.manual_sharp_attr.MF_LUT_KEY[i] = m_Mf_keyPts[0][i].x * LINE_LEVEL_NUM / LINE_WINDOW_WIDTH;
	}



	size = m_Hf_keyPts[0].size();

	if (size > LINE_KEY_POINT_MAX)
	{
		size = LINE_KEY_POINT_MAX;
	}

	memset(&m_Sharp.p_sharp_attr.manual_sharp_attr.HF_LUT_KEY, 0, sizeof(T_U16) * LINE_KEY_POINT_MAX);

	for (i=0; i<size; i++)
	{
		m_Sharp.p_sharp_attr.manual_sharp_attr.HF_LUT_KEY[i] = m_Hf_keyPts[0][i].x * LINE_LEVEL_NUM / LINE_WINDOW_WIDTH;
	}

	for (i=0; i<9; i++)
	{
		size = m_Mf_keyPts[i+1].size();
		if (size > LINE_KEY_POINT_MAX)
		{
			size = LINE_KEY_POINT_MAX;
		}

		memset(&m_Sharp.p_sharp_attr.linkage_sharp_attr[i].MF_LUT_KEY, 0, sizeof(T_U16) * LINE_KEY_POINT_MAX);

		for (int j=0; j<size; j++)
		{
			m_Sharp.p_sharp_attr.linkage_sharp_attr[i].MF_LUT_KEY[j] = m_Mf_keyPts[i+1][j].x * LINE_LEVEL_NUM / LINE_WINDOW_WIDTH;
		}


		size = m_Hf_keyPts[i+1].size();
		if (size > LINE_KEY_POINT_MAX)
		{
			size = LINE_KEY_POINT_MAX;
		}

		memset(&m_Sharp.p_sharp_attr.linkage_sharp_attr[i].HF_LUT_KEY, 0, sizeof(T_U16) * LINE_KEY_POINT_MAX);

		for (j=0; j<size; j++)
		{
			m_Sharp.p_sharp_attr.linkage_sharp_attr[i].HF_LUT_KEY[j] = m_Hf_keyPts[i+1][j].x * LINE_LEVEL_NUM / LINE_WINDOW_WIDTH;
		}
	}

}

void CSharpDlg::SetDataValue(bool bFromStruct)
{

	if (bFromStruct)
	{
		m_mode = m_Sharp.p_sharp_attr.ysharp_mode;
	}
	
	m_LutDlg.m_MfLutDlg.SetKeyPts(&m_Mf_keyPts[0], m_Sharp.p_sharp_attr.manual_sharp_attr.MF_LUT_KEY, m_Sharp.p_sharp_attr.manual_sharp_attr.MF_HPF_LUT);
	m_LutDlg.m_HfLutDlg.SetKeyPts(&m_Hf_keyPts[0], m_Sharp.p_sharp_attr.manual_sharp_attr.HF_LUT_KEY, m_Sharp.p_sharp_attr.manual_sharp_attr.HF_HPF_LUT);


	for (int j=0; j<9; j++)
	{
		m_LutDlg.m_MfLutDlg.SetKeyPts(&m_Mf_keyPts[j+1], m_Sharp.p_sharp_attr.linkage_sharp_attr[j].MF_LUT_KEY, m_Sharp.p_sharp_attr.linkage_sharp_attr[j].MF_HPF_LUT);
		m_LutDlg.m_HfLutDlg.SetKeyPts(&m_Hf_keyPts[j+1], m_Sharp.p_sharp_attr.linkage_sharp_attr[j].HF_LUT_KEY, m_Sharp.p_sharp_attr.linkage_sharp_attr[j].HF_HPF_LUT);
	}


	if (0 == m_mode)
	{
		m_ysharp_Enable = m_Sharp.p_sharp_attr.manual_sharp_attr.ysharp_enable;
		m_skin_det_Enable = m_Sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_detect_enable;

		m_m_k.SetPos(m_Sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_k);
		m_m_k_slider.SetPos(m_Sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_k);
		m_m_shift.SetPos(m_Sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_shift);
		m_m_shift_slider.SetPos(m_Sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_shift);
		m_h_k.SetPos(m_Sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_k);
		m_h_k_slider.SetPos(m_Sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_k);
		m_h_shift.SetPos(m_Sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_shift);
		m_h_shift_slider.SetPos(m_Sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_shift);

		EnableLinkageRadio(FALSE);
	}
	else
	{
		m_ysharp_Enable = m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].ysharp_enable;
		m_skin_det_Enable = m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].sharp_skin_detect_enable;

		m_m_k.SetPos(m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].mf_hpf_k);
		m_m_k_slider.SetPos(m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].mf_hpf_k);
		m_m_shift.SetPos(m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].mf_hpf_shift);
		m_m_shift_slider.SetPos(m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].mf_hpf_shift);
		m_h_k.SetPos(m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].hf_hpf_k);
		m_h_k_slider.SetPos(m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].hf_hpf_k);
		m_h_shift.SetPos(m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].hf_hpf_shift);
		m_h_shift_slider.SetPos(m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].hf_hpf_shift);

		EnableLinkageRadio(TRUE);
	}

	((CButton *)GetDlgItem(IDC_RADIO10))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO11))->SetCheck(m_mode);
	((CButton *)GetDlgItem(IDC_RADIO12))->SetCheck(!m_ysharp_Enable);
	((CButton *)GetDlgItem(IDC_RADIO13))->SetCheck(m_ysharp_Enable);
	((CButton *)GetDlgItem(IDC_RADIO14))->SetCheck(!m_skin_det_Enable);
	((CButton *)GetDlgItem(IDC_RADIO15))->SetCheck(m_skin_det_Enable);

}

int CSharpDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_SHARP))) return -1;

	GetDataValue(TRUE);

	memcpy(pPageInfoSt, &m_Sharp, sizeof(AK_ISP_INIT_SHARP));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_SHARP);

	return 0;
}


int CSharpDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_SHARP))) return -1;

	memcpy(&m_Sharp, pPageInfoSt, sizeof(AK_ISP_INIT_SHARP));

	SetDataValue(TRUE);
	
	return 0;
}

void CSharpDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_m_k_slider.SetPos(m_m_k.GetPos());
	*pResult = 0;
}

void CSharpDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_m_k_slider.SetPos(m_m_k.GetPos());
}

void CSharpDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_m_k.SetPos(m_m_k_slider.GetPos());
	*pResult = 0;
}

void CSharpDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_m_shift_slider.SetPos(m_m_shift.GetPos());
	*pResult = 0;
}

void CSharpDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_m_shift_slider.SetPos(m_m_shift.GetPos());
}

void CSharpDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_m_shift.SetPos(m_m_shift_slider.GetPos());
	*pResult = 0;
}

void CSharpDlg::OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_h_k_slider.SetPos(m_h_k.GetPos());
	*pResult = 0;
}

void CSharpDlg::OnKillfocusEdit3() 
{
	// TODO: Add your control notification handler code here
	m_h_k_slider.SetPos(m_h_k.GetPos());
}

void CSharpDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_h_k.SetPos(m_h_k_slider.GetPos());
	*pResult = 0;
}

void CSharpDlg::OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_h_shift_slider.SetPos(m_h_shift.GetPos());
	*pResult = 0;
}

void CSharpDlg::OnKillfocusEdit4() 
{
	// TODO: Add your control notification handler code here
	m_h_shift_slider.SetPos(m_h_shift.GetPos());
}

void CSharpDlg::OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_h_shift.SetPos(m_h_shift_slider.GetPos());
	*pResult = 0;
}

void CSharpDlg::OnDeltaposSpin5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_strength_slider.SetPos(m_strength.GetPos());
	*pResult = 0;
}

void CSharpDlg::OnKillfocusEdit5() 
{
	// TODO: Add your control notification handler code here
	m_strength_slider.SetPos(m_strength.GetPos());
}

void CSharpDlg::OnCustomdrawSlider5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_strength.SetPos(m_strength_slider.GetPos());
	*pResult = 0;
}

void CSharpDlg::OnRadio12() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CSharpDlg::OnRadio13() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CSharpDlg::OnRadio14() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CSharpDlg::OnRadio15() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CSharpDlg::OnRadio10() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(FALSE);
	UpdateData(FALSE);
}

void CSharpDlg::OnRadio11() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(TRUE);
	UpdateData(FALSE);
}

void CSharpDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSharpDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSharpDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSharpDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSharpDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSharpDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSharpDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSharpDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSharpDlg::OnRadio9() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSharpDlg::OnButtonLut() 
{
	// TODO: Add your control notification handler code here

	vector<CPoint> *pts_tmp = NULL;
	int i = 0;
	int size = 0;
	int index = 0;

	if (0 == m_mode)
	{
		index = 0;
	}
	else 
	{
		index = m_envi + 1;
	}

	size = m_Mf_keyPts[index].size();

	pts_tmp = m_LutDlg.m_MfLutDlg.GetKeyPts();
	(*pts_tmp).resize(size);
	for (i=0; i<size; i++)
	{
		(*pts_tmp)[i].x = m_Mf_keyPts[index][i].x;
		(*pts_tmp)[i].y = m_Mf_keyPts[index][i].y;
	}

	size = m_Hf_keyPts[index].size();

	pts_tmp = m_LutDlg.m_HfLutDlg.GetKeyPts();
	(*pts_tmp).resize(size);
	for (i=0; i<size; i++)
	{
		(*pts_tmp)[i].x = m_Hf_keyPts[index][i].x;
		(*pts_tmp)[i].y = m_Hf_keyPts[index][i].y;
	}

	
	if (IDOK == m_LutDlg.DoModal())
	{
		size = LINE_LEVEL_NUM * sizeof(short);

		if (0 == m_mode)
		{
			m_LutDlg.m_MfLutDlg.GetLevel((char*)m_Sharp.p_sharp_attr.manual_sharp_attr.MF_HPF_LUT, &size);
			m_LutDlg.m_HfLutDlg.GetLevel((char*)m_Sharp.p_sharp_attr.manual_sharp_attr.HF_HPF_LUT, &size);
		}
		else
		{
			m_LutDlg.m_MfLutDlg.GetLevel((char*)m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].MF_HPF_LUT, &size);
			m_LutDlg.m_HfLutDlg.GetLevel((char*)m_Sharp.p_sharp_attr.linkage_sharp_attr[m_envi].HF_HPF_LUT, &size);
		}

		pts_tmp = m_LutDlg.m_MfLutDlg.GetKeyPts();
		size = (*pts_tmp).size();
		m_Mf_keyPts[index].resize(size);
		for (i=0; i<size; i++)
		{
			m_Mf_keyPts[index][i].x = (*pts_tmp)[i].x;
			m_Mf_keyPts[index][i].y = (*pts_tmp)[i].y;
		}

		pts_tmp = m_LutDlg.m_HfLutDlg.GetKeyPts();
		size = (*pts_tmp).size();
		m_Hf_keyPts[index].resize(size);
		for (i=0; i<size; i++)
		{
			m_Hf_keyPts[index][i].x = (*pts_tmp)[i].x;
			m_Hf_keyPts[index][i].y = (*pts_tmp)[i].y;
		}
	}
}

void CSharpDlg::Clean(void) 
{
	ZeroMemory(&m_Sharp, sizeof(AK_ISP_INIT_SHARP));
	EnableLinkageRadio(FALSE);
	SetDataValue(TRUE);
	UpdateData(FALSE);

	for (int i=0; i<10; i++)
	{
		m_Mf_keyPts[i].clear();
		m_Mf_keyPts[i].push_back(CPoint(0, LINE_WINDOW_HEIGHT/2));
		m_Mf_keyPts[i].push_back(CPoint(LINE_WINDOW_WIDTH-1, LINE_WINDOW_HEIGHT/2));

		m_Hf_keyPts[i].clear();
		m_Hf_keyPts[i].push_back(CPoint(0, LINE_WINDOW_HEIGHT/2));
		m_Hf_keyPts[i].push_back(CPoint(LINE_WINDOW_WIDTH-1, LINE_WINDOW_HEIGHT/2));
	}
}

int CSharpDlg::Convert_v2_data(AK_ISP_INIT_SHARP *struct_new, AK_ISP_INIT_SHARP_V2* struct_v2) 
{
	int i = 0, j = 0;
	
	if ((struct_v2 == NULL) || (struct_new == NULL)) return -1;
	
	memcpy(struct_new, struct_v2, sizeof(AK_ISP_INIT_SHARP));

	struct_new->param_id = ISP_SHARP;
	struct_new->length = sizeof(AK_ISP_INIT_SHARP);

	for (i=0; i<256; i++)
	{
		struct_new->p_sharp_attr.manual_sharp_attr.MF_HPF_LUT[i] /= 2;
		struct_new->p_sharp_attr.manual_sharp_attr.HF_HPF_LUT[i] /= 2;
	}
	
	for (j=0; j<9; j++)
	{
		for (i=0; i<256; i++)
		{
			struct_new->p_sharp_attr.linkage_sharp_attr[j].MF_HPF_LUT[i] /= 2;
			struct_new->p_sharp_attr.linkage_sharp_attr[j].HF_HPF_LUT[i] /= 2;
		}
	}

	
	return 0;
}

