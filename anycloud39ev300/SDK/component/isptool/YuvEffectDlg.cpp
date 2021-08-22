// YuvEffectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "YuvEffectDlg.h"
#include "netctrl.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CYuvEffectDlg dialog


CYuvEffectDlg::CYuvEffectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CYuvEffectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CYuvEffectDlg)
	m_dark_margin_enable = 0;
	//}}AFX_DATA_INIT
}


void CYuvEffectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CYuvEffectDlg)
	DDX_Control(pDX, IDC_SLIDER4, m_uv_b_slider);
	DDX_Control(pDX, IDC_SPIN4, m_uv_b);
	DDX_Control(pDX, IDC_SLIDER3, m_uv_a_slider);
	DDX_Control(pDX, IDC_SPIN3, m_uv_a);
	DDX_Control(pDX, IDC_SLIDER2, m_y_b_slider);
	DDX_Control(pDX, IDC_SPIN2, m_y_b);
	DDX_Control(pDX, IDC_SLIDER1, m_y_a_slider);
	DDX_Control(pDX, IDC_SPIN1, m_y_a);
	DDX_Radio(pDX, IDC_RADIO1, m_dark_margin_enable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CYuvEffectDlg, CDialog)
	//{{AFX_MSG_MAP(CYuvEffectDlg)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
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
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, OnDeltaposSpin4)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnKillfocusEdit4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnCustomdrawSlider4)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CYuvEffectDlg message handlers
BOOL CYuvEffectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Effect, sizeof(AK_ISP_INIT_EFFECT));

	m_y_a.SetRange(0,255);
	m_y_a.SetPos(0);
	m_y_a_slider.SetRange(0,255);
	m_y_a_slider.SetPos(0);

	m_y_b.SetRange(-128,127);
	m_y_b.SetPos(0);
	m_y_b_slider.SetRange(-128,127, TRUE);
	m_y_b_slider.SetPos(0);

	m_uv_a.SetRange(-256,255);
	m_uv_a.SetPos(0);
	m_uv_a_slider.SetRange(-256,255, TRUE);
	m_uv_a_slider.SetPos(0);

	m_uv_b.SetRange(-256,255);
	m_uv_b.SetPos(0);
	m_uv_b_slider.SetRange(-256,255, TRUE);
	m_uv_b_slider.SetPos(0);



	return TRUE;
}

BOOL CYuvEffectDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CYuvEffectDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	m_Effect.param_id = ISP_YUVEFFECT;
	m_Effect.length = sizeof(AK_ISP_INIT_EFFECT);

	m_Effect.p_isp_effect.y_a = (T_U16)m_y_a.GetPos();
	m_Effect.p_isp_effect.y_b = (T_S16)m_y_b.GetPos();
	m_Effect.p_isp_effect.uv_a = (T_S16)m_uv_a.GetPos();
	m_Effect.p_isp_effect.uv_b = (T_S16)m_uv_b.GetPos();
	m_Effect.p_isp_effect.dark_margin_en = (T_U16)m_dark_margin_enable;

}

void CYuvEffectDlg::SetDataValue(void)
{
	m_y_a.SetPos(m_Effect.p_isp_effect.y_a);
	m_y_a_slider.SetPos(m_Effect.p_isp_effect.y_a);
	m_y_b.SetPos(m_Effect.p_isp_effect.y_b);
	m_y_b_slider.SetPos(m_Effect.p_isp_effect.y_b);
	m_uv_a.SetPos(m_Effect.p_isp_effect.uv_a);
	m_uv_a_slider.SetPos(m_Effect.p_isp_effect.uv_a);
	m_uv_b.SetPos(m_Effect.p_isp_effect.uv_b);
	m_uv_b_slider.SetPos(m_Effect.p_isp_effect.uv_b);
	m_dark_margin_enable = m_Effect.p_isp_effect.dark_margin_en;

	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(!m_dark_margin_enable);
	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(m_dark_margin_enable);

}

int CYuvEffectDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_EFFECT))) return -1;

	GetDataValue();

	memcpy(pPageInfoSt, &m_Effect, sizeof(AK_ISP_INIT_EFFECT));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_EFFECT);

	return 0;
}


int CYuvEffectDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_EFFECT))) return -1;

	memcpy(&m_Effect, pPageInfoSt, sizeof(AK_ISP_INIT_EFFECT));

	SetDataValue();

	//UpdateData(FALSE);
	
	return 0;
}

void CYuvEffectDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_YUVEFFECT, 0);
}

void CYuvEffectDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_YUVEFFECT, 0);
}

void CYuvEffectDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_EFFECT))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Effect, sizeof(AK_ISP_INIT_EFFECT));
	file.Close();

	SetDataValue();

	UpdateData(FALSE);
}

void CYuvEffectDlg::OnButtonWrite() 
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

	file.Write(&m_Effect, sizeof(AK_ISP_INIT_EFFECT));

	file.Close();
}

void CYuvEffectDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_y_a_slider.SetPos(m_y_a.GetPos());
	*pResult = 0;
}

void CYuvEffectDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_y_a_slider.SetPos(m_y_a.GetPos());
}

void CYuvEffectDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_y_a.SetPos(m_y_a_slider.GetPos());
	*pResult = 0;
}

void CYuvEffectDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_y_b_slider.SetPos((T_S16)m_y_b.GetPos());
	*pResult = 0;
}

void CYuvEffectDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_y_b_slider.SetPos((T_S16)m_y_b.GetPos());
}

void CYuvEffectDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_y_b.SetPos((T_S16)m_y_b_slider.GetPos());
	*pResult = 0;
}

void CYuvEffectDlg::OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_uv_a_slider.SetPos((T_S16)m_uv_a.GetPos());
	*pResult = 0;
}

void CYuvEffectDlg::OnKillfocusEdit3() 
{
	// TODO: Add your control notification handler code here
	m_uv_a_slider.SetPos((T_S16)m_uv_a.GetPos());
}

void CYuvEffectDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_uv_a.SetPos((T_S16)m_uv_a_slider.GetPos());
	*pResult = 0;
}

void CYuvEffectDlg::OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_uv_b_slider.SetPos((T_S16)m_uv_b.GetPos());
	*pResult = 0;
}

void CYuvEffectDlg::OnKillfocusEdit4() 
{
	// TODO: Add your control notification handler code here
	m_uv_b_slider.SetPos((T_S16)m_uv_b.GetPos());
}

void CYuvEffectDlg::OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_uv_b.SetPos((T_S16)m_uv_b_slider.GetPos());
	*pResult = 0;
}

void CYuvEffectDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CYuvEffectDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CYuvEffectDlg::Clean(void) 
{
	ZeroMemory(&m_Effect, sizeof(AK_ISP_INIT_EFFECT));
	SetDataValue();
	UpdateData(FALSE);
}
