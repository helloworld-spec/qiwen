// SaturationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "SaturationDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaturationDlg dialog


CSaturationDlg::CSaturationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSaturationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaturationDlg)
	m_mode = 0;
	m_envi = 0;
	m_Enable = 0;
	//}}AFX_DATA_INIT
}


void CSaturationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaturationDlg)
	DDX_Control(pDX, IDC_SLIDER7, m_scale3_slider);
	DDX_Control(pDX, IDC_SLIDER4, m_scale2_slider);
	DDX_Control(pDX, IDC_SLIDER3, m_scale1_slider);
	DDX_Control(pDX, IDC_SLIDER6, m_th4_slider);
	DDX_Control(pDX, IDC_SLIDER21, m_th3_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_th2_slider);
	DDX_Control(pDX, IDC_SLIDER1, m_th1_slider);
	DDX_Control(pDX, IDC_SPIN150, m_scale3);
	DDX_Control(pDX, IDC_SPIN4, m_scale2);
	DDX_Control(pDX, IDC_SPIN3, m_scale1);
	DDX_Control(pDX, IDC_SPIN6, m_th4);
	DDX_Control(pDX, IDC_SPIN87, m_th3);
	DDX_Control(pDX, IDC_SPIN2, m_th2);
	DDX_Control(pDX, IDC_SPIN1, m_th1);
	DDX_Radio(pDX, IDC_RADIO12, m_mode);
	DDX_Radio(pDX, IDC_RADIO1, m_envi);
	DDX_Radio(pDX, IDC_RADIO33, m_Enable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaturationDlg, CDialog)
	//{{AFX_MSG_MAP(CSaturationDlg)
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
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN87, OnDeltaposSpin87)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnKillfocusEdit5)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER21, OnCustomdrawSlider21)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN6, OnDeltaposSpin6)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnKillfocusEdit6)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER6, OnCustomdrawSlider6)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN150, OnDeltaposSpin150)
	ON_EN_KILLFOCUS(IDC_EDIT7, OnKillfocusEdit7)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER7, OnCustomdrawSlider7)
	ON_BN_CLICKED(IDC_RADIO33, OnRadio33)
	ON_BN_CLICKED(IDC_RADIO34, OnRadio34)
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
// CSaturationDlg message handlers
BOOL CSaturationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Sat, sizeof(AK_ISP_INIT_SATURATION));

	m_th1.SetRange(0,1023);
	m_th1.SetPos(0);
	m_th1_slider.SetRange(0,1023);
	m_th1_slider.SetPos(0);

	m_th2.SetRange(0,1023);
	m_th2.SetPos(0);
	m_th2_slider.SetRange(0,1023);
	m_th2_slider.SetPos(0);

	m_th3.SetRange(0,1023);
	m_th3.SetPos(0);
	m_th3_slider.SetRange(0,1023);
	m_th3_slider.SetPos(0);

	m_th4.SetRange(0,1023);
	m_th4.SetPos(0);
	m_th4_slider.SetRange(0,1023);
	m_th4_slider.SetPos(0);

	m_scale1.SetRange(0,255);
	m_scale1.SetPos(0);
	m_scale1_slider.SetRange(0,255);
	m_scale1_slider.SetPos(0);

	m_scale2.SetRange(0,255);
	m_scale2.SetPos(0);
	m_scale2_slider.SetRange(0,255);
	m_scale2_slider.SetPos(0);

	m_scale3.SetRange(0,255);
	m_scale3.SetPos(0);
	m_scale3_slider.SetRange(0,255);
	m_scale3_slider.SetPos(0);

	EnableLinkageRadio(FALSE);

	return TRUE;
}

BOOL CSaturationDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CSaturationDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_SATURATION))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Sat, sizeof(AK_ISP_INIT_SATURATION));
	file.Close();

	SetDataValue(TRUE);

	UpdateData(FALSE);
}

void CSaturationDlg::OnButtonWrite() 
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

	file.Write(&m_Sat, sizeof(AK_ISP_INIT_SATURATION));

	file.Close();
}

void CSaturationDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_SATURATION, 0);
}

void CSaturationDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_SATURATION, 0);
}

void CSaturationDlg::CheckData(AK_ISP_INIT_SATURATION *p_Sat)
{
	int i = 0;
	
	if (p_Sat->p_se_attr.manual_sat.SE_th1 == p_Sat->p_se_attr.manual_sat.SE_th2)
	{
		if (p_Sat->p_se_attr.manual_sat.SE_th2 < 1023)
		{
			p_Sat->p_se_attr.manual_sat.SE_th2++;
		}
		else
		{
			p_Sat->p_se_attr.manual_sat.SE_th1--;
		}
	}
	else if (p_Sat->p_se_attr.manual_sat.SE_th1 > p_Sat->p_se_attr.manual_sat.SE_th2)
	{
		int tmp = p_Sat->p_se_attr.manual_sat.SE_th1;

		p_Sat->p_se_attr.manual_sat.SE_th1 = p_Sat->p_se_attr.manual_sat.SE_th2;
		p_Sat->p_se_attr.manual_sat.SE_th2 = tmp;
	}

	if (p_Sat->p_se_attr.manual_sat.SE_th3 == p_Sat->p_se_attr.manual_sat.SE_th4)
	{
		if (p_Sat->p_se_attr.manual_sat.SE_th4 < 1023)
		{
			p_Sat->p_se_attr.manual_sat.SE_th4++;
		}
		else
		{
			p_Sat->p_se_attr.manual_sat.SE_th3--;
		}
	}
	else if (p_Sat->p_se_attr.manual_sat.SE_th3 > p_Sat->p_se_attr.manual_sat.SE_th4)
	{
		int tmp = p_Sat->p_se_attr.manual_sat.SE_th3;

		p_Sat->p_se_attr.manual_sat.SE_th3 = p_Sat->p_se_attr.manual_sat.SE_th4;
		p_Sat->p_se_attr.manual_sat.SE_th4 = tmp;
	}
	

	for (i=0; i<9; i++)
	{
		if (p_Sat->p_se_attr.linkage_sat[i].SE_th1 == p_Sat->p_se_attr.linkage_sat[i].SE_th2)
		{
			if (p_Sat->p_se_attr.linkage_sat[i].SE_th2 < 1023)
			{
				p_Sat->p_se_attr.linkage_sat[i].SE_th2++;
			}
			else
			{
				p_Sat->p_se_attr.linkage_sat[i].SE_th1--;
			}
		}
		else if (p_Sat->p_se_attr.linkage_sat[i].SE_th1 > p_Sat->p_se_attr.linkage_sat[i].SE_th2)
		{
			int tmp = p_Sat->p_se_attr.linkage_sat[i].SE_th1;

			p_Sat->p_se_attr.linkage_sat[i].SE_th1 = p_Sat->p_se_attr.linkage_sat[i].SE_th2;
			p_Sat->p_se_attr.linkage_sat[i].SE_th2 = tmp;
		}

		if (p_Sat->p_se_attr.linkage_sat[i].SE_th3 == p_Sat->p_se_attr.linkage_sat[i].SE_th4)
		{
			if (p_Sat->p_se_attr.linkage_sat[i].SE_th4 < 1023)
			{
				p_Sat->p_se_attr.linkage_sat[i].SE_th4++;
			}
			else
			{
				p_Sat->p_se_attr.linkage_sat[i].SE_th3--;
			}
		}
		else if (p_Sat->p_se_attr.linkage_sat[i].SE_th3 > p_Sat->p_se_attr.linkage_sat[i].SE_th4)
		{
			int tmp = p_Sat->p_se_attr.linkage_sat[i].SE_th3;

			p_Sat->p_se_attr.linkage_sat[i].SE_th3 = p_Sat->p_se_attr.linkage_sat[i].SE_th4;
			p_Sat->p_se_attr.linkage_sat[i].SE_th4 = tmp;
		}
	}
}


void CSaturationDlg::GetDataValue(bool bToStruct)
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


	m_Sat.param_id = ISP_SATURATION;
	m_Sat.length = sizeof(AK_ISP_INIT_SATURATION);
	
	m_Sat.p_se_attr.SE_mode = (T_U16)m_mode;

	if (0 == mode_tmp)
	{
		m_Sat.p_se_attr.manual_sat.SE_enable = (T_U16)m_Enable;

		m_Sat.p_se_attr.manual_sat.SE_th1 = (T_U16)m_th1.GetPos();
		m_Sat.p_se_attr.manual_sat.SE_th2 = (T_U16)m_th2.GetPos();
		m_Sat.p_se_attr.manual_sat.SE_th3 = (T_U16)m_th3.GetPos();
		m_Sat.p_se_attr.manual_sat.SE_th4 = (T_U16)m_th4.GetPos();

		m_Sat.p_se_attr.manual_sat.SE_scale1 = (T_U16)m_scale1.GetPos();
		m_Sat.p_se_attr.manual_sat.SE_scale2 = (T_U16)m_scale2.GetPos();
		m_Sat.p_se_attr.manual_sat.SE_scale3 = (T_U16)m_scale3.GetPos();
	}
	else
	{
		m_Sat.p_se_attr.linkage_sat[envi_tmp].SE_enable = (T_U16)m_Enable;

		m_Sat.p_se_attr.linkage_sat[envi_tmp].SE_th1 = (T_U16)m_th1.GetPos();
		m_Sat.p_se_attr.linkage_sat[envi_tmp].SE_th2 = (T_U16)m_th2.GetPos();
		m_Sat.p_se_attr.linkage_sat[envi_tmp].SE_th3 = (T_U16)m_th3.GetPos();
		m_Sat.p_se_attr.linkage_sat[envi_tmp].SE_th4 = (T_U16)m_th4.GetPos();

		m_Sat.p_se_attr.linkage_sat[envi_tmp].SE_scale1 = (T_U16)m_scale1.GetPos();
		m_Sat.p_se_attr.linkage_sat[envi_tmp].SE_scale2 = (T_U16)m_scale2.GetPos();
		m_Sat.p_se_attr.linkage_sat[envi_tmp].SE_scale3 = (T_U16)m_scale3.GetPos();
	}

	CheckData(&m_Sat);

}

void CSaturationDlg::SetDataValue(bool bFromStruct)
{

	if (bFromStruct)
	{
		m_mode = m_Sat.p_se_attr.SE_mode;
	}
	
	
	if (0 == m_mode)
	{
		m_Enable = m_Sat.p_se_attr.manual_sat.SE_enable;

		m_th1.SetPos(m_Sat.p_se_attr.manual_sat.SE_th1);
		m_th1_slider.SetPos(m_Sat.p_se_attr.manual_sat.SE_th1);
		m_th2.SetPos(m_Sat.p_se_attr.manual_sat.SE_th2);
		m_th2_slider.SetPos(m_Sat.p_se_attr.manual_sat.SE_th2);
		m_th3.SetPos(m_Sat.p_se_attr.manual_sat.SE_th3);
		m_th3_slider.SetPos(m_Sat.p_se_attr.manual_sat.SE_th3);
		m_th4.SetPos(m_Sat.p_se_attr.manual_sat.SE_th4);
		m_th4_slider.SetPos(m_Sat.p_se_attr.manual_sat.SE_th4);
		
		m_scale1.SetPos(m_Sat.p_se_attr.manual_sat.SE_scale1);
		m_scale1_slider.SetPos(m_Sat.p_se_attr.manual_sat.SE_scale1);
		m_scale2.SetPos(m_Sat.p_se_attr.manual_sat.SE_scale2);
		m_scale2_slider.SetPos(m_Sat.p_se_attr.manual_sat.SE_scale2);
		m_scale3.SetPos(m_Sat.p_se_attr.manual_sat.SE_scale3);
		m_scale3_slider.SetPos(m_Sat.p_se_attr.manual_sat.SE_scale3);

		EnableLinkageRadio(FALSE);
	}
	else
	{
		m_Enable = m_Sat.p_se_attr.linkage_sat[m_envi].SE_enable;

		m_th1.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_th1);
		m_th1_slider.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_th1);
		m_th2.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_th2);
		m_th2_slider.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_th2);
		m_th3.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_th3);
		m_th3_slider.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_th3);
		m_th4.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_th4);
		m_th4_slider.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_th4);
		
		m_scale1.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_scale1);
		m_scale1_slider.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_scale1);
		m_scale2.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_scale2);
		m_scale2_slider.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_scale2);
		m_scale3.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_scale3);
		m_scale3_slider.SetPos(m_Sat.p_se_attr.linkage_sat[m_envi].SE_scale3);
		
		EnableLinkageRadio(TRUE);
	}

	((CButton *)GetDlgItem(IDC_RADIO12))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO13))->SetCheck(m_mode);
	((CButton *)GetDlgItem(IDC_RADIO33))->SetCheck(!m_Enable);
	((CButton *)GetDlgItem(IDC_RADIO34))->SetCheck(m_Enable);

}

void CSaturationDlg::EnableLinkageRadio(bool bEnable) 
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

int CSaturationDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_SATURATION))) return -1;

	GetDataValue(TRUE);

	memcpy(pPageInfoSt, &m_Sat, sizeof(AK_ISP_INIT_SATURATION));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_SATURATION);

	return 0;
}


int CSaturationDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_SATURATION))) return -1;

	memcpy(&m_Sat, pPageInfoSt, sizeof(AK_ISP_INIT_SATURATION));

	SetDataValue(TRUE);
	
	return 0;
}


void CSaturationDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th1_slider.SetPos(m_th1.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_th1_slider.SetPos(m_th1.GetPos());
}

void CSaturationDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th1.SetPos(m_th1_slider.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th2_slider.SetPos(m_th2.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_th2_slider.SetPos(m_th2.GetPos());
}

void CSaturationDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th2.SetPos(m_th2_slider.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnDeltaposSpin87(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th3_slider.SetPos(m_th3.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnKillfocusEdit5() 
{
	// TODO: Add your control notification handler code here
	m_th3_slider.SetPos(m_th3.GetPos());
}

void CSaturationDlg::OnCustomdrawSlider21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th3.SetPos(m_th3_slider.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnDeltaposSpin6(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th4_slider.SetPos(m_th4.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnKillfocusEdit6() 
{
	// TODO: Add your control notification handler code here
	m_th4_slider.SetPos(m_th4.GetPos());
}

void CSaturationDlg::OnCustomdrawSlider6(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th4.SetPos(m_th4_slider.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_scale1_slider.SetPos(m_scale1.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnKillfocusEdit3() 
{
	// TODO: Add your control notification handler code here
	m_scale1_slider.SetPos(m_scale1.GetPos());
}

void CSaturationDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_scale1.SetPos(m_scale1_slider.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_scale2_slider.SetPos(m_scale2.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnKillfocusEdit4() 
{
	// TODO: Add your control notification handler code here
	m_scale2_slider.SetPos(m_scale2.GetPos());
}

void CSaturationDlg::OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_scale2.SetPos(m_scale2_slider.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnDeltaposSpin150(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_scale3_slider.SetPos(m_scale3.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnKillfocusEdit7() 
{
	// TODO: Add your control notification handler code here
	m_scale3_slider.SetPos(m_scale3.GetPos());
}

void CSaturationDlg::OnCustomdrawSlider7(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_scale3.SetPos(m_scale3_slider.GetPos());
	*pResult = 0;
}

void CSaturationDlg::OnRadio33() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CSaturationDlg::OnRadio34() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CSaturationDlg::OnRadio12() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(FALSE);
	UpdateData(FALSE);
}

void CSaturationDlg::OnRadio13() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(TRUE);
	UpdateData(FALSE);
}

void CSaturationDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSaturationDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSaturationDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSaturationDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSaturationDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSaturationDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSaturationDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSaturationDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CSaturationDlg::OnRadio9() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}


void CSaturationDlg::Clean(void) 
{
	ZeroMemory(&m_Sat, sizeof(AK_ISP_INIT_SATURATION));
	EnableLinkageRadio(FALSE);
	SetDataValue(TRUE);
	UpdateData(FALSE);
}

int CSaturationDlg::Convert_v2_data(AK_ISP_INIT_SATURATION*struct_new, AK_ISP_INIT_SATURATION_V2* struct_v2) 
{
	int i = 0;
	
	if ((struct_v2 == NULL) || (struct_new == NULL)) return -1;
	
	struct_new->param_id = ISP_SATURATION;
	struct_new->length = sizeof(AK_ISP_INIT_SATURATION);


	struct_new->p_se_attr.SE_mode = struct_v2->p_se_attr.SE_mode;
	struct_new->p_se_attr.manual_sat.SE_enable = struct_v2->p_se_attr.manual_sat.SE_enable;
	struct_new->p_se_attr.manual_sat.SE_th1 = struct_v2->p_se_attr.manual_sat.SE_th1;
	struct_new->p_se_attr.manual_sat.SE_th2 = struct_v2->p_se_attr.manual_sat.SE_th2;
	struct_new->p_se_attr.manual_sat.SE_th3 = 512;
	struct_new->p_se_attr.manual_sat.SE_th4 = 1023;
	struct_new->p_se_attr.manual_sat.SE_scale_slop1 = struct_v2->p_se_attr.manual_sat.SE_scale_slop;
	struct_new->p_se_attr.manual_sat.SE_scale_slop2 = 0;
	struct_new->p_se_attr.manual_sat.SE_scale1 = struct_v2->p_se_attr.manual_sat.SE_scale1;
	struct_new->p_se_attr.manual_sat.SE_scale2 = struct_v2->p_se_attr.manual_sat.SE_scale2;
	struct_new->p_se_attr.manual_sat.SE_scale3 = struct_v2->p_se_attr.manual_sat.SE_scale2;
	
	for (i=0; i<9; i++)
	{
		struct_new->p_se_attr.linkage_sat[i].SE_enable = struct_v2->p_se_attr.linkage_sat[i].SE_enable;
		struct_new->p_se_attr.linkage_sat[i].SE_th1 = struct_v2->p_se_attr.linkage_sat[i].SE_th1;
		struct_new->p_se_attr.linkage_sat[i].SE_th2 = struct_v2->p_se_attr.linkage_sat[i].SE_th2;
		struct_new->p_se_attr.linkage_sat[i].SE_th3 = 512;
		struct_new->p_se_attr.linkage_sat[i].SE_th4 = 1023;
		struct_new->p_se_attr.linkage_sat[i].SE_scale_slop1 = struct_v2->p_se_attr.linkage_sat[i].SE_scale_slop;
		struct_new->p_se_attr.linkage_sat[i].SE_scale_slop2 = 0;
		struct_new->p_se_attr.linkage_sat[i].SE_scale1 = struct_v2->p_se_attr.linkage_sat[i].SE_scale1;
		struct_new->p_se_attr.linkage_sat[i].SE_scale2 = struct_v2->p_se_attr.linkage_sat[i].SE_scale2;
		struct_new->p_se_attr.linkage_sat[i].SE_scale3 = struct_v2->p_se_attr.linkage_sat[i].SE_scale2;
	}

	
	return 0;
}

