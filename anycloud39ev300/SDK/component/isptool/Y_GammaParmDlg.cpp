// Y_GammaParmDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "Y_GammaParmDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CY_GammaParmDlg dialog


CY_GammaParmDlg::CY_GammaParmDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CY_GammaParmDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CY_GammaParmDlg)
	m_uvad_en = 0;
	//}}AFX_DATA_INIT
	ZeroMemory(&m_parm, sizeof(AK_ISP_Y_GAMMA_PARM));
}


void CY_GammaParmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CY_GammaParmDlg)
	DDX_Radio(pDX, IDC_RADIO1, m_uvad_en);

	DDX_Control(pDX, IDC_SPIN1, m_level);
	DDX_Control(pDX, IDC_SPIN2, m_yth1);
	DDX_Control(pDX, IDC_SPIN4, m_yth2);
	DDX_Control(pDX, IDC_SPIN5, m_slop);
	DDX_Control(pDX, IDC_SPIN6, m_gain);

	DDX_Control(pDX, IDC_SLIDER1, m_level_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_yth1_slider);
	DDX_Control(pDX, IDC_SLIDER3, m_yth2_slider);
	DDX_Control(pDX, IDC_SLIDER4, m_slop_slider);
	DDX_Control(pDX, IDC_SLIDER5, m_gain_slider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CY_GammaParmDlg, CDialog)
	//{{AFX_MSG_MAP(CY_GammaParmDlg)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, OnDeltaposSpin2)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnKillfocusEdit2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, OnDeltaposSpin4)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnKillfocusEdit4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnCustomdrawSlider4)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN5, OnDeltaposSpin5)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnKillfocusEdit5)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER5, OnCustomdrawSlider5)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN6, OnDeltaposSpin6)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnKillfocusEdit6)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CY_GammaParmDlg message handlers

BOOL CY_GammaParmDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_level.SetRange(0,31);
	m_level_slider.SetRange(0,31);
	m_yth1.SetRange(0,1023);
	m_yth1_slider.SetRange(0,1023);
	m_yth2.SetRange(0,1023);
	m_yth2_slider.SetRange(0,1023);
	m_slop.SetRange(0,128);
	m_slop_slider.SetRange(0,128);
	m_gain.SetRange(0,1023);
	m_gain_slider.SetRange(0,1023);
	
	m_uvad_en = m_parm.ygamma_uv_adjust_enable;
	m_level.SetPos(m_parm.ygamma_uv_adjust_level);
	m_level_slider.SetPos(m_parm.ygamma_uv_adjust_level);
	m_yth1.SetPos(m_parm.ygamma_cnoise_yth1);
	m_yth1_slider.SetPos(m_parm.ygamma_cnoise_yth1);
	m_yth2.SetPos(m_parm.ygamma_cnoise_yth2);
	m_yth2_slider.SetPos(m_parm.ygamma_cnoise_yth2);
	m_slop.SetPos(m_parm.ygamma_cnoise_slop);
	m_slop_slider.SetPos(m_parm.ygamma_cnoise_slop);
	m_gain.SetPos(m_parm.ygamma_cnoise_gain);
	m_gain_slider.SetPos(m_parm.ygamma_cnoise_gain);

	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(!m_uvad_en);
	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(m_uvad_en);

	UpdateData(FALSE);

	return TRUE;
}


BOOL CY_GammaParmDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CY_GammaParmDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();

	
	m_parm.ygamma_uv_adjust_enable = (T_U16)m_uvad_en;
	m_parm.ygamma_uv_adjust_level = (T_U16)m_level.GetPos();
	m_parm.ygamma_cnoise_yth1 = (T_U16)m_yth1.GetPos();
	m_parm.ygamma_cnoise_yth2 = (T_U16)m_yth2.GetPos();
	m_parm.ygamma_cnoise_slop = (T_U16)m_slop.GetPos();
	m_parm.ygamma_cnoise_gain = (T_U16)m_gain.GetPos();
	
}

void CY_GammaParmDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}


void CY_GammaParmDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CY_GammaParmDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CY_GammaParmDlg::GetDataValue(char* pbuf, int* size)
{
	if (NULL == pbuf || *size < sizeof(AK_ISP_Y_GAMMA_PARM))
	{
		return;
	}

	memcpy(pbuf, &m_parm, sizeof(AK_ISP_Y_GAMMA_PARM));
	*size = sizeof(AK_ISP_Y_GAMMA_PARM);
}


void CY_GammaParmDlg::SetDataValue(char* pbuf, int size)
{
	if (NULL == pbuf || size < sizeof(AK_ISP_Y_GAMMA_PARM))
	{
		return;
	}

	memcpy(&m_parm, pbuf, sizeof(AK_ISP_Y_GAMMA_PARM));
}


void CY_GammaParmDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_level.SetPos(m_level_slider.GetPos());
	*pResult = 0;
}

void CY_GammaParmDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	m_level_slider.SetPos(m_level.GetPos());
	*pResult = 0;
}

void CY_GammaParmDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_level_slider.SetPos(m_level.GetPos());
}

void CY_GammaParmDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_yth1.SetPos(m_yth1_slider.GetPos());
	*pResult = 0;
}

void CY_GammaParmDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	m_yth1_slider.SetPos(m_yth1.GetPos());
	*pResult = 0;
}

void CY_GammaParmDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_yth1_slider.SetPos(m_yth1.GetPos());
}

void CY_GammaParmDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_yth2.SetPos(m_yth2_slider.GetPos());
	*pResult = 0;
}

void CY_GammaParmDlg::OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	m_yth2_slider.SetPos(m_yth2.GetPos());
	*pResult = 0;
}

void CY_GammaParmDlg::OnKillfocusEdit4() 
{
	// TODO: Add your control notification handler code here
	m_yth2_slider.SetPos(m_yth2.GetPos());
}

void CY_GammaParmDlg::OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_slop.SetPos(m_slop_slider.GetPos());
	*pResult = 0;
}

void CY_GammaParmDlg::OnDeltaposSpin5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	m_slop_slider.SetPos(m_slop.GetPos());
	*pResult = 0;
}

void CY_GammaParmDlg::OnKillfocusEdit5() 
{
	// TODO: Add your control notification handler code here
	m_slop_slider.SetPos(m_slop.GetPos());
}

void CY_GammaParmDlg::OnCustomdrawSlider5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_gain.SetPos(m_gain_slider.GetPos());
	*pResult = 0;
}

void CY_GammaParmDlg::OnDeltaposSpin6(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	m_gain_slider.SetPos(m_gain.GetPos());
	*pResult = 0;
}

void CY_GammaParmDlg::OnKillfocusEdit6() 
{
	// TODO: Add your control notification handler code here
	m_gain_slider.SetPos(m_gain.GetPos());
}


