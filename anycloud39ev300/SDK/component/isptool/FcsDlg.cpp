// FcsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "FcsDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFcsDlg dialog


CFcsDlg::CFcsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFcsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFcsDlg)
	m_Enable = 0;
	m_uv_Enable = 0;
	m_mode = 0;
	//}}AFX_DATA_INIT
}


void CFcsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFcsDlg)
	DDX_Control(pDX, IDC_SLIDER3, m_uv_nr_th_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_gain_slop_slider);
	DDX_Control(pDX, IDC_SLIDER1, m_th_slider);
	DDX_Control(pDX, IDC_SPIN3, m_uv_nr_th);
	DDX_Control(pDX, IDC_SPIN2, m_gain_slop);
	DDX_Control(pDX, IDC_SPIN1, m_th);
	DDX_Radio(pDX, IDC_RADIO5, m_uv_Enable);
	DDX_Radio(pDX, IDC_RADIO3, m_Enable);
	DDX_Radio(pDX, IDC_RADIO1, m_mode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFcsDlg, CDialog)
	//{{AFX_MSG_MAP(CFcsDlg)
	ON_BN_CLICKED(IDC_BUTTON_LINKAGE, OnButtonLinkage)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnRadio6)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, OnDeltaposSpin2)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnKillfocusEdit2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, OnDeltaposSpin3)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnKillfocusEdit3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFcsDlg message handlers

BOOL CFcsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Fcs, sizeof(AK_ISP_INIT_FCS));

	m_th.SetRange(0,255);
	m_th.SetPos(0);

	m_gain_slop.SetRange(0,63);
	m_gain_slop.SetPos(0);

	m_uv_nr_th.SetRange(0,1023);
	m_uv_nr_th.SetPos(0);

	return TRUE;
}

BOOL CFcsDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CFcsDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	m_Fcs.param_id = ISP_FCS;
	m_Fcs.length = sizeof(AK_ISP_INIT_FCS);

	m_Fcs.p_fcs.manual_fcs.fcs_th = (T_U16)m_th.GetPos();
	m_Fcs.p_fcs.manual_fcs.fcs_gain_slop = (T_U16)m_gain_slop.GetPos();
	m_Fcs.p_fcs.manual_fcs.fcs_uv_nr_th = (T_U16)m_uv_nr_th.GetPos();
	m_Fcs.p_fcs.manual_fcs.fcs_enable = (T_U16)m_Enable;
	m_Fcs.p_fcs.manual_fcs.fcs_uv_nr_enable = (T_U16)m_uv_Enable;
	m_Fcs.p_fcs.fcs_mode = (T_U16)m_mode;
}

void CFcsDlg::SetDataValue(void)
{
	m_th.SetPos(m_Fcs.p_fcs.manual_fcs.fcs_th);
	m_th_slider.SetPos(m_Fcs.p_fcs.manual_fcs.fcs_th);
	m_gain_slop.SetPos(m_Fcs.p_fcs.manual_fcs.fcs_gain_slop);
	m_gain_slop_slider.SetPos(m_Fcs.p_fcs.manual_fcs.fcs_gain_slop);
	m_uv_nr_th.SetPos(m_Fcs.p_fcs.manual_fcs.fcs_uv_nr_th);
	m_uv_nr_th_slider.SetPos(m_Fcs.p_fcs.manual_fcs.fcs_uv_nr_th);
	m_Enable = m_Fcs.p_fcs.manual_fcs.fcs_enable;
	m_uv_Enable = m_Fcs.p_fcs.manual_fcs.fcs_uv_nr_enable;
	m_mode = m_Fcs.p_fcs.fcs_mode;

	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(m_mode);
	((CButton *)GetDlgItem(IDC_RADIO3))->SetCheck(!m_Enable);
	((CButton *)GetDlgItem(IDC_RADIO4))->SetCheck(m_Enable);
	((CButton *)GetDlgItem(IDC_RADIO5))->SetCheck(!m_uv_Enable);
	((CButton *)GetDlgItem(IDC_RADIO6))->SetCheck(m_uv_Enable);
}

int CFcsDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_FCS))) return -1;

	GetDataValue();

	memcpy(pPageInfoSt, &m_Fcs, sizeof(AK_ISP_INIT_FCS));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_FCS);

	return 0;
}


int CFcsDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_FCS))) return -1;

	memcpy(&m_Fcs, pPageInfoSt, sizeof(AK_ISP_INIT_FCS));

	SetDataValue();
	
	return 0;
}


void CFcsDlg::OnButtonLinkage() 
{
	// TODO: Add your control notification handler code here
	memcpy(m_FcsLinkageDlg.m_linkage, m_Fcs.p_fcs.linkage_fcs, 9 * sizeof(AK_ISP_FCS));

	if (IDOK == m_FcsLinkageDlg.DoModal())
	{
		memcpy(m_Fcs.p_fcs.linkage_fcs, m_FcsLinkageDlg.m_linkage, 9 * sizeof(AK_ISP_FCS));
	}
}


void CFcsDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_FCS))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Fcs, sizeof(AK_ISP_INIT_FCS));
	file.Close();

	SetDataValue();

	UpdateData(FALSE);
}

void CFcsDlg::OnButtonWrite() 
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

	file.Write(&m_Fcs, sizeof(AK_ISP_INIT_FCS));

	file.Close();
}

void CFcsDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_FCS, 0);
}

void CFcsDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_FCS, 0);
}

void CFcsDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CFcsDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th_slider.SetPos(m_th.GetPos());
	*pResult = 0;
}

void CFcsDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_th_slider.SetPos(m_th.GetPos());
}

void CFcsDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th.SetPos(m_th_slider.GetPos());
	*pResult = 0;
}

void CFcsDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_gain_slop_slider.SetPos(m_gain_slop.GetPos());
	*pResult = 0;
}

void CFcsDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_gain_slop_slider.SetPos(m_gain_slop.GetPos());
}

void CFcsDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_gain_slop.SetPos(m_gain_slop_slider.GetPos());
	*pResult = 0;
}

void CFcsDlg::OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_uv_nr_th_slider.SetPos(m_uv_nr_th.GetPos());
	*pResult = 0;
}

void CFcsDlg::OnKillfocusEdit3() 
{
	// TODO: Add your control notification handler code here
	m_uv_nr_th_slider.SetPos(m_uv_nr_th.GetPos());
}

void CFcsDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_uv_nr_th.SetPos(m_uv_nr_th_slider.GetPos());
	*pResult = 0;
}


void CFcsDlg::Clean(void) 
{
	ZeroMemory(&m_Fcs, sizeof(AK_ISP_INIT_FCS));

	SetDataValue();

	UpdateData(FALSE);
}