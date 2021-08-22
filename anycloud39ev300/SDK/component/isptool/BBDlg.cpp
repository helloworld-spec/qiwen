// BBDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "BBDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBBDlg dialog


CBBDlg::CBBDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBBDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBBDlg)
	m_mode = 0;
	m_envi = 0;
	m_Enable = 0;
	//}}AFX_DATA_INIT
}


void CBBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBBDlg)
	DDX_Control(pDX, IDC_SLIDER8, m_b_b_slider);
	DDX_Control(pDX, IDC_SLIDER7, m_b_a_slider);
	DDX_Control(pDX, IDC_SLIDER6, m_gb_b_slider);
	DDX_Control(pDX, IDC_SLIDER5, m_gb_a_slider);
	DDX_Control(pDX, IDC_SLIDER4, m_gr_b_slider);
	DDX_Control(pDX, IDC_SLIDER3, m_gr_a_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_r_b_slider);
	DDX_Control(pDX, IDC_SLIDER1, m_r_a_slider);
	DDX_Control(pDX, IDC_SPIN8, m_b_b);
	DDX_Control(pDX, IDC_SPIN7, m_b_a);
	DDX_Control(pDX, IDC_SPIN6, m_gb_b);
	DDX_Control(pDX, IDC_SPIN5, m_gb_a);
	DDX_Control(pDX, IDC_SPIN4, m_gr_b);
	DDX_Control(pDX, IDC_SPIN3, m_gr_a);
	DDX_Control(pDX, IDC_SPIN2, m_r_b);
	DDX_Control(pDX, IDC_SPIN1, m_r_a);
	DDX_Radio(pDX, IDC_RADIO10, m_mode);
	DDX_Radio(pDX, IDC_RADIO1, m_envi);
	DDX_Radio(pDX, IDC_RADIO12, m_Enable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBBDlg, CDialog)
	//{{AFX_MSG_MAP(CBBDlg)
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
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN6, OnDeltaposSpin6)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnKillfocusEdit6)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER6, OnCustomdrawSlider6)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN7, OnDeltaposSpin7)
	ON_EN_KILLFOCUS(IDC_EDIT7, OnKillfocusEdit7)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER7, OnCustomdrawSlider7)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN8, OnDeltaposSpin8)
	ON_EN_KILLFOCUS(IDC_EDIT8, OnKillfocusEdit8)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER8, OnCustomdrawSlider8)
	ON_BN_CLICKED(IDC_RADIO10, OnRadio10)
	ON_BN_CLICKED(IDC_RADIO11, OnRadio11)
	ON_BN_CLICKED(IDC_RADIO12, OnRadio12)
	ON_BN_CLICKED(IDC_RADIO13, OnRadio13)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO33, OnRadio33)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnRadio6)
	ON_BN_CLICKED(IDC_RADIO7, OnRadio7)
	ON_BN_CLICKED(IDC_RADIO8, OnRadio8)
	ON_BN_CLICKED(IDC_RADIO9, OnRadio9)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBBDlg message handlers
BOOL CBBDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Blc, sizeof(AK_ISP_INIT_BLC));

	m_r_a.SetRange(0,1023);
	m_r_a.SetPos(0);
	m_r_a_slider.SetRange(0,1023);
	m_r_a_slider.SetPos(0);

	m_r_b.SetRange(-2048,2047);
	m_r_b.SetPos(0);
	m_r_b_slider.SetRange(-2048,2047, TRUE);
	m_r_b_slider.SetPos(0);

	m_gr_a.SetRange(0,1023);
	m_gr_a.SetPos(0);
	m_gr_a_slider.SetRange(0,1023);
	m_gr_a_slider.SetPos(0);

	m_gr_b.SetRange(-2048,2047);
	m_gr_b.SetPos(0);
	m_gr_b_slider.SetRange(-2048,2047, TRUE);
	m_gr_b_slider.SetPos(0);

	m_gb_a.SetRange(0,1023);
	m_gb_a.SetPos(0);
	m_gb_a_slider.SetRange(0,1023);
	m_gb_a_slider.SetPos(0);

	m_gb_b.SetRange(-2048,2047);
	m_gb_b.SetPos(0);
	m_gb_b_slider.SetRange(-2048,2047, TRUE);
	m_gb_b_slider.SetPos(0);

	m_b_a.SetRange(0,1023);
	m_b_a.SetPos(0);
	m_b_a_slider.SetRange(0,1023);
	m_b_a_slider.SetPos(0);

	m_b_b.SetRange(-2048,2047);
	m_b_b.SetPos(0);
	m_b_b_slider.SetRange(-2048,2047, TRUE);
	m_b_b_slider.SetPos(0);

	EnableLinkageRadio(FALSE);

	return TRUE;
}


BOOL CBBDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CBBDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_BLC))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Blc, sizeof(AK_ISP_INIT_BLC));
	file.Close();

	SetDataValue(TRUE);

	UpdateData(FALSE);
}

void CBBDlg::OnButtonWrite() 
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

	file.Write(&m_Blc, sizeof(AK_ISP_INIT_BLC));

	file.Close();
}

void CBBDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_BB, 0);
}

void CBBDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_BB, 0);
}

void CBBDlg::GetDataValue(bool bToStruct)
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


	m_Blc.param_id = ISP_BB;
	m_Blc.length = sizeof(AK_ISP_INIT_BLC);
	
	m_Blc.p_blc.blc_mode = (T_U16)m_mode;

	if (0 == mode_tmp)
	{
		m_Blc.p_blc.m_blc.black_level_enable = (T_U16)m_Enable;

		m_Blc.p_blc.m_blc.bl_r_a = (T_U16)m_r_a.GetPos();
		m_Blc.p_blc.m_blc.bl_r_offset = (T_S16)m_r_b.GetPos();
		m_Blc.p_blc.m_blc.bl_gr_a = (T_U16)m_gr_a.GetPos();
		m_Blc.p_blc.m_blc.bl_gr_offset = (T_S16)m_gr_b.GetPos();
		m_Blc.p_blc.m_blc.bl_gb_a = (T_U16)m_gb_a.GetPos();
		m_Blc.p_blc.m_blc.bl_gb_offset = (T_S16)m_gb_b.GetPos();
		m_Blc.p_blc.m_blc.bl_b_a = (T_U16)m_b_a.GetPos();
		m_Blc.p_blc.m_blc.bl_b_offset = (T_S16)m_b_b.GetPos();
	}
	else
	{
		m_Blc.p_blc.linkage_blc[envi_tmp].black_level_enable = (T_U16)m_Enable;

		m_Blc.p_blc.linkage_blc[envi_tmp].bl_r_a = (T_U16)m_r_a.GetPos();
		m_Blc.p_blc.linkage_blc[envi_tmp].bl_r_offset = (T_S16)m_r_b.GetPos();
		m_Blc.p_blc.linkage_blc[envi_tmp].bl_gr_a = (T_U16)m_gr_a.GetPos();
		m_Blc.p_blc.linkage_blc[envi_tmp].bl_gr_offset = (T_S16)m_gr_b.GetPos();
		m_Blc.p_blc.linkage_blc[envi_tmp].bl_gb_a = (T_U16)m_gb_a.GetPos();
		m_Blc.p_blc.linkage_blc[envi_tmp].bl_gb_offset = (T_S16)m_gb_b.GetPos();
		m_Blc.p_blc.linkage_blc[envi_tmp].bl_b_a = (T_U16)m_b_a.GetPos();
		m_Blc.p_blc.linkage_blc[envi_tmp].bl_b_offset = (T_S16)m_b_b.GetPos();
	}

}

void CBBDlg::SetDataValue(bool bFromStruct)
{

	if (bFromStruct)
	{
		m_mode = m_Blc.p_blc.blc_mode;
	}
	
	
	if (0 == m_mode)
	{
		m_Enable = m_Blc.p_blc.m_blc.black_level_enable;

		m_r_a.SetPos(m_Blc.p_blc.m_blc.bl_r_a);
		m_r_a_slider.SetPos(m_Blc.p_blc.m_blc.bl_r_a);
		m_r_b.SetPos(m_Blc.p_blc.m_blc.bl_r_offset);
		m_r_b_slider.SetPos(m_Blc.p_blc.m_blc.bl_r_offset);
		m_gr_a.SetPos(m_Blc.p_blc.m_blc.bl_gr_a);
		m_gr_a_slider.SetPos(m_Blc.p_blc.m_blc.bl_gr_a);
		m_gr_b.SetPos(m_Blc.p_blc.m_blc.bl_gr_offset);
		m_gr_b_slider.SetPos(m_Blc.p_blc.m_blc.bl_gr_offset);
		m_gb_a.SetPos(m_Blc.p_blc.m_blc.bl_gb_a);
		m_gb_a_slider.SetPos(m_Blc.p_blc.m_blc.bl_gb_a);
		m_gb_b.SetPos(m_Blc.p_blc.m_blc.bl_gb_offset);
		m_gb_b_slider.SetPos(m_Blc.p_blc.m_blc.bl_gb_offset);
		m_b_a.SetPos(m_Blc.p_blc.m_blc.bl_b_a);
		m_b_a_slider.SetPos(m_Blc.p_blc.m_blc.bl_b_a);
		m_b_b.SetPos(m_Blc.p_blc.m_blc.bl_b_offset);
		m_b_b_slider.SetPos(m_Blc.p_blc.m_blc.bl_b_offset);

		EnableLinkageRadio(FALSE);
	}
	else
	{
		m_Enable = m_Blc.p_blc.linkage_blc[m_envi].black_level_enable;

		m_r_a.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_r_a);
		m_r_a_slider.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_r_a);
		m_r_b.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_r_offset);
		m_r_b_slider.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_r_offset);
		m_gr_a.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_gr_a);
		m_gr_a_slider.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_gr_a);
		m_gr_b.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_gr_offset);
		m_gr_b_slider.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_gr_offset);
		m_gb_a.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_gb_a);
		m_gb_a_slider.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_gb_a);
		m_gb_b.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_gb_offset);
		m_gb_b_slider.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_gb_offset);
		m_b_a.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_b_a);
		m_b_a_slider.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_b_a);
		m_b_b.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_b_offset);
		m_b_b_slider.SetPos(m_Blc.p_blc.linkage_blc[m_envi].bl_b_offset);
		
		EnableLinkageRadio(TRUE);
	}

	((CButton *)GetDlgItem(IDC_RADIO10))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO11))->SetCheck(m_mode);
	((CButton *)GetDlgItem(IDC_RADIO12))->SetCheck(!m_Enable);
	((CButton *)GetDlgItem(IDC_RADIO13))->SetCheck(m_Enable);


}

int CBBDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_BLC))) return -1;

	GetDataValue(TRUE);

	memcpy(pPageInfoSt, &m_Blc, sizeof(AK_ISP_INIT_BLC));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_BLC);

	return 0;
}


int CBBDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_BLC))) return -1;

	memcpy(&m_Blc, pPageInfoSt, sizeof(AK_ISP_INIT_BLC));

	SetDataValue(TRUE);
	
	return 0;
}

void CBBDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_r_a_slider.SetPos(m_r_a.GetPos());
	*pResult = 0;
}

void CBBDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_r_a_slider.SetPos(m_r_a.GetPos());
}

void CBBDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_r_a.SetPos(m_r_a_slider.GetPos());
	*pResult = 0;
}

void CBBDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_r_b_slider.SetPos((T_S16)m_r_b.GetPos());
	*pResult = 0;
}

void CBBDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_r_b_slider.SetPos((T_S16)m_r_b.GetPos());
}

void CBBDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_r_b.SetPos((T_S16)m_r_b_slider.GetPos());
	*pResult = 0;
}

void CBBDlg::OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_gr_a_slider.SetPos(m_gr_a.GetPos());
	*pResult = 0;
}

void CBBDlg::OnKillfocusEdit3() 
{
	// TODO: Add your control notification handler code here
	m_gr_a_slider.SetPos(m_gr_a.GetPos());
}

void CBBDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_gr_a.SetPos(m_gr_a_slider.GetPos());
	*pResult = 0;
}

void CBBDlg::OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_gr_b_slider.SetPos((T_S16)m_gr_b.GetPos());
	*pResult = 0;
}

void CBBDlg::OnKillfocusEdit4() 
{
	// TODO: Add your control notification handler code here
	m_gr_b_slider.SetPos((T_S16)m_gr_b.GetPos());
}

void CBBDlg::OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_gr_b.SetPos((T_S16)m_gr_b_slider.GetPos());
	*pResult = 0;
}

void CBBDlg::OnDeltaposSpin5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_gb_a_slider.SetPos(m_gb_a.GetPos());
	*pResult = 0;
}

void CBBDlg::OnKillfocusEdit5() 
{
	// TODO: Add your control notification handler code here
	m_gb_a_slider.SetPos(m_gb_a.GetPos());
}

void CBBDlg::OnCustomdrawSlider5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_gb_a.SetPos(m_gb_a_slider.GetPos());
	*pResult = 0;
}


void CBBDlg::OnDeltaposSpin6(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_gb_b_slider.SetPos((T_S16)m_gb_b.GetPos());
	*pResult = 0;
}

void CBBDlg::OnKillfocusEdit6() 
{
	// TODO: Add your control notification handler code here
	m_gb_b_slider.SetPos((T_S16)m_gb_b.GetPos());
}

void CBBDlg::OnCustomdrawSlider6(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_gb_b.SetPos((T_S16)m_gb_b_slider.GetPos());
	*pResult = 0;
}

void CBBDlg::OnDeltaposSpin7(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_b_a_slider.SetPos(m_b_a.GetPos());
	*pResult = 0;
}

void CBBDlg::OnKillfocusEdit7() 
{
	// TODO: Add your control notification handler code here
	m_b_a_slider.SetPos(m_b_a.GetPos());
}

void CBBDlg::OnCustomdrawSlider7(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_b_a.SetPos(m_b_a_slider.GetPos());
	*pResult = 0;
}

void CBBDlg::OnDeltaposSpin8(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_b_b_slider.SetPos((T_S16)m_b_b.GetPos());
	*pResult = 0;
}

void CBBDlg::OnKillfocusEdit8() 
{
	// TODO: Add your control notification handler code here
	m_b_b_slider.SetPos((T_S16)m_b_b.GetPos());
}

void CBBDlg::OnCustomdrawSlider8(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_b_b.SetPos((T_S16)m_b_b_slider.GetPos());
	*pResult = 0;
}

void CBBDlg::EnableLinkageRadio(bool bEnable) 
{
	CWnd *pRadio[9];
	
	pRadio[0] = GetDlgItem(IDC_RADIO1);
	pRadio[1] = GetDlgItem(IDC_RADIO2);
	pRadio[2] = GetDlgItem(IDC_RADIO33);
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

void CBBDlg::OnRadio12() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CBBDlg::OnRadio13() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CBBDlg::OnRadio10() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(FALSE);
	UpdateData(FALSE);
}

void CBBDlg::OnRadio11() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(TRUE);
	UpdateData(FALSE);
}

void CBBDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CBBDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CBBDlg::OnRadio33() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CBBDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CBBDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CBBDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CBBDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CBBDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CBBDlg::OnRadio9() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CBBDlg::Clean(void) 
{
	ZeroMemory(&m_Blc, sizeof(AK_ISP_INIT_BLC));
	EnableLinkageRadio(FALSE);
	SetDataValue(TRUE);
	UpdateData(FALSE);
}
