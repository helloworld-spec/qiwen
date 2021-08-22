// GBDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "GBDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGBDlg dialog


CGBDlg::CGBDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGBDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGBDlg)
	m_mode = 0;
	m_envi = 0;
	m_Enable = 0;
	//}}AFX_DATA_INIT
}


void CGBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGBDlg)
	DDX_Control(pDX, IDC_SLIDER3, m_threshold_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_kstep_slider);
	DDX_Control(pDX, IDC_SLIDER1, m_en_th_slider);
	DDX_Control(pDX, IDC_SPIN3, m_threshold);
	DDX_Control(pDX, IDC_SPIN2, m_kstep);
	DDX_Control(pDX, IDC_SPIN1, m_en_th);
	DDX_Radio(pDX, IDC_RADIO10, m_mode);
	DDX_Radio(pDX, IDC_RADIO1, m_envi);
	DDX_Radio(pDX, IDC_RADIO12, m_Enable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGBDlg, CDialog)
	//{{AFX_MSG_MAP(CGBDlg)
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
	ON_BN_CLICKED(IDC_RADIO10, OnRadio10)
	ON_BN_CLICKED(IDC_RADIO11, OnRadio11)
	ON_BN_CLICKED(IDC_RADIO12, OnRadio12)
	ON_BN_CLICKED(IDC_RADIO13, OnRadio13)
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
// CGBDlg message handlers
BOOL CGBDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Gb, sizeof(AK_ISP_INIT_GB));

	m_en_th.SetRange(0,255);
	m_en_th.SetPos(0);
	m_en_th_slider.SetRange(0,255);
	m_en_th_slider.SetPos(0);

	m_kstep.SetRange(0,15);
	m_kstep.SetPos(0);
	m_kstep_slider.SetRange(0,15);
	m_kstep_slider.SetPos(0);

	m_threshold.SetRange(0,1023);
	m_threshold.SetPos(0);
	m_threshold_slider.SetRange(0,1023);
	m_threshold_slider.SetPos(0);

	EnableLinkageRadio(FALSE);

	return TRUE;
}

BOOL CGBDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CGBDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_GB))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Gb, sizeof(AK_ISP_INIT_GB));
	file.Close();

	SetDataValue(TRUE);

	UpdateData(FALSE);
}

void CGBDlg::OnButtonWrite() 
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

	file.Write(&m_Gb, sizeof(AK_ISP_INIT_GB));

	file.Close();
}

void CGBDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_GB, 0);
}

void CGBDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_GB, 0);
}

void CGBDlg::EnableLinkageRadio(bool bEnable) 
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

void CGBDlg::GetDataValue(bool bToStruct)
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


	m_Gb.param_id = ISP_GB;
	m_Gb.length = sizeof(AK_ISP_INIT_GB);
	
	m_Gb.p_gb.gb_mode = (T_U16)m_mode;

	if (0 == mode_tmp)
	{
		m_Gb.p_gb.manual_gb.gb_enable = (T_U16)m_Enable;

		m_Gb.p_gb.manual_gb.gb_en_th = (T_U16)m_en_th.GetPos();
		m_Gb.p_gb.manual_gb.gb_kstep = (T_S16)m_kstep.GetPos();
		m_Gb.p_gb.manual_gb.gb_threshold = (T_U16)m_threshold.GetPos();
	}
	else
	{
		m_Gb.p_gb.linkage_gb[envi_tmp].gb_enable = (T_U16)m_Enable;

		m_Gb.p_gb.linkage_gb[envi_tmp].gb_en_th = (T_U16)m_en_th.GetPos();
		m_Gb.p_gb.linkage_gb[envi_tmp].gb_kstep = (T_S16)m_kstep.GetPos();
		m_Gb.p_gb.linkage_gb[envi_tmp].gb_threshold = (T_U16)m_threshold.GetPos();
	}

}

void CGBDlg::SetDataValue(bool bFromStruct)
{

	if (bFromStruct)
	{
		m_mode = m_Gb.p_gb.gb_mode;
	}
	
	
	if (0 == m_mode)
	{
		m_Enable = m_Gb.p_gb.manual_gb.gb_enable;

		m_en_th.SetPos(m_Gb.p_gb.manual_gb.gb_en_th);
		m_en_th_slider.SetPos(m_Gb.p_gb.manual_gb.gb_en_th);
		m_kstep.SetPos(m_Gb.p_gb.manual_gb.gb_kstep);
		m_kstep_slider.SetPos(m_Gb.p_gb.manual_gb.gb_kstep);
		m_threshold.SetPos(m_Gb.p_gb.manual_gb.gb_threshold);
		m_threshold_slider.SetPos(m_Gb.p_gb.manual_gb.gb_threshold);

		EnableLinkageRadio(FALSE);
	}
	else
	{
		m_Enable = m_Gb.p_gb.linkage_gb[m_envi].gb_enable;

		m_en_th.SetPos(m_Gb.p_gb.linkage_gb[m_envi].gb_en_th);
		m_en_th_slider.SetPos(m_Gb.p_gb.linkage_gb[m_envi].gb_en_th);
		m_kstep.SetPos(m_Gb.p_gb.linkage_gb[m_envi].gb_kstep);
		m_kstep_slider.SetPos(m_Gb.p_gb.linkage_gb[m_envi].gb_kstep);
		m_threshold.SetPos(m_Gb.p_gb.linkage_gb[m_envi].gb_threshold);
		m_threshold_slider.SetPos(m_Gb.p_gb.linkage_gb[m_envi].gb_threshold);

		EnableLinkageRadio(TRUE);
	}
	
	((CButton *)GetDlgItem(IDC_RADIO10))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO11))->SetCheck(m_mode);
	((CButton *)GetDlgItem(IDC_RADIO12))->SetCheck(!m_Enable);
	((CButton *)GetDlgItem(IDC_RADIO13))->SetCheck(m_Enable);

}

int CGBDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_GB))) return -1;

	GetDataValue(TRUE);

	memcpy(pPageInfoSt, &m_Gb, sizeof(AK_ISP_INIT_GB));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_GB);

	return 0;
}


int CGBDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_GB))) return -1;

	memcpy(&m_Gb, pPageInfoSt, sizeof(AK_ISP_INIT_GB));

	SetDataValue(TRUE);
	
	return 0;
}

void CGBDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_en_th_slider.SetPos(m_en_th.GetPos());
	*pResult = 0;
}

void CGBDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_en_th_slider.SetPos(m_en_th.GetPos());
}

void CGBDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_en_th.SetPos(m_en_th_slider.GetPos());
	*pResult = 0;
}

void CGBDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_kstep_slider.SetPos((T_S16)m_kstep.GetPos());
	*pResult = 0;
}

void CGBDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_kstep_slider.SetPos((T_S16)m_kstep.GetPos());
}

void CGBDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_kstep.SetPos((T_S16)m_kstep_slider.GetPos());
	*pResult = 0;
}

void CGBDlg::OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_threshold_slider.SetPos(m_threshold.GetPos());
	*pResult = 0;
}

void CGBDlg::OnKillfocusEdit3() 
{
	// TODO: Add your control notification handler code here
	m_threshold_slider.SetPos(m_threshold.GetPos());
}

void CGBDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_threshold.SetPos(m_threshold_slider.GetPos());
	*pResult = 0;
}


void CGBDlg::OnRadio12() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CGBDlg::OnRadio13() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CGBDlg::OnRadio10() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(FALSE);
	UpdateData(FALSE);
}

void CGBDlg::OnRadio11() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(TRUE);
	UpdateData(FALSE);
}

void CGBDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CGBDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CGBDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CGBDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CGBDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CGBDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CGBDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CGBDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CGBDlg::OnRadio9() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CGBDlg::Clean(void) 
{
	ZeroMemory(&m_Gb, sizeof(AK_ISP_INIT_GB));
	EnableLinkageRadio(FALSE);
	SetDataValue(TRUE);
	UpdateData(FALSE);
}
