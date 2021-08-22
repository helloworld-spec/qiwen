// WdrDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "WdrDlg.h"
#include "NetCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWdrDlg dialog

T_U16 default_lut[65] = {0};

CWdrDlg::CWdrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWdrDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWdrDlg)
	m_WdrEnable = 0;
	m_uvEnable = 0;
	m_mode = 0;
	m_envi = 0;
	//}}AFX_DATA_INIT

}


void CWdrDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWdrDlg)
	DDX_Control(pDX, IDC_SLIDER10, m_gain_slider);
	DDX_Control(pDX, IDC_SPIN8, m_gain);
	DDX_Control(pDX, IDC_SLIDER6, m_yth2_slider);
	DDX_Control(pDX, IDC_SPIN6, m_yth2);
	DDX_Control(pDX, IDC_SLIDER5, m_yth1_slider);
	DDX_Control(pDX, IDC_SPIN5, m_yth1);
	DDX_Control(pDX, IDC_SLIDER3, m_uvlevel_slider);
	DDX_Control(pDX, IDC_SPIN3, m_uv_level);
	DDX_Control(pDX, IDC_SLIDER2, m_th2_slider);
	DDX_Control(pDX, IDC_SPIN2, m_th2);
	DDX_Control(pDX, IDC_SLIDER1, m_th1_slider);
	DDX_Control(pDX, IDC_SPIN1, m_th1);
	DDX_Control(pDX, IDC_SLIDER4, m_th3_slider);
	DDX_Control(pDX, IDC_SPIN4, m_th3);
	DDX_Control(pDX, IDC_SLIDER23, m_th4_slider);
	DDX_Control(pDX, IDC_SPIN20, m_th4);
	DDX_Control(pDX, IDC_SLIDER24, m_th5_slider);
	DDX_Control(pDX, IDC_SPIN160, m_th5);

	DDX_Radio(pDX, IDC_RADIO12, m_WdrEnable);
	DDX_Radio(pDX, IDC_RADIO14, m_uvEnable);
	DDX_Radio(pDX, IDC_RADIO10, m_mode);
	DDX_Radio(pDX, IDC_RADIO1, m_envi);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWdrDlg, CDialog)
	//{{AFX_MSG_MAP(CWdrDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, OnDeltaposSpin2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, OnDeltaposSpin3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, OnDeltaposSpin4)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN20, OnDeltaposSpin20)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN160, OnDeltaposSpin160)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnKillfocusEdit2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnKillfocusEdit4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider3)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnKillfocusEdit6)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN5, OnDeltaposSpin5)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER5, OnCustomdrawSlider5)
	ON_EN_KILLFOCUS(IDC_EDIT7, OnKillfocusEdit7)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN6, OnDeltaposSpin6)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER6, OnCustomdrawSlider6)
	ON_EN_KILLFOCUS(IDC_EDIT8, OnKillfocusEdit8)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN8, OnDeltaposSpin8)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER10, OnCustomdrawSlider10)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnCustomdrawSlider4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER23, OnCustomdrawSlider23)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER24, OnCustomdrawSlider24)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnKillfocusEdit5)
	ON_EN_KILLFOCUS(IDC_EDIT145, OnKillfocusEdit145)
	ON_EN_KILLFOCUS(IDC_EDIT159, OnKillfocusEdit159)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWdrDlg message handlers
BOOL CWdrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Wdr, sizeof(AK_ISP_INIT_WDR));

	m_LutDlg.m_Area1Dlg.SetLevelNum(WDR_LUT_LEVEL_NUM);
	m_LutDlg.m_Area2Dlg.SetLevelNum(WDR_LUT_LEVEL_NUM);
	m_LutDlg.m_Area3Dlg.SetLevelNum(WDR_LUT_LEVEL_NUM);
	m_LutDlg.m_Area4Dlg.SetLevelNum(WDR_LUT_LEVEL_NUM);
	m_LutDlg.m_Area5Dlg.SetLevelNum(WDR_LUT_LEVEL_NUM);
	m_LutDlg.m_Area6Dlg.SetLevelNum(WDR_LUT_LEVEL_NUM);

	m_th1.SetRange(0,255);
	m_th1.SetPos(0);
	m_th1_slider.SetRange(0,255);
	m_th1_slider.SetPos(0);

	m_th2.SetRange(0,255);
	m_th2.SetPos(0);
	m_th2_slider.SetRange(0,255);
	m_th2_slider.SetPos(0);

	m_th3.SetRange(0,255);
	m_th3.SetPos(0);
	m_th3_slider.SetRange(0,255);
	m_th3_slider.SetPos(0);

	m_th4.SetRange(0,255);
	m_th4.SetPos(0);
	m_th4_slider.SetRange(0,255);
	m_th4_slider.SetPos(0);

	m_th5.SetRange(0,255);
	m_th5.SetPos(0);
	m_th5_slider.SetRange(0,255);
	m_th5_slider.SetPos(0);

	
	m_uv_level.SetRange(0,31);
	m_uv_level.SetPos(0);
	m_uvlevel_slider.SetRange(0,31);
	m_uvlevel_slider.SetPos(0);

	m_yth1.SetRange(0,1023);
	m_yth1.SetPos(0);
	m_yth1_slider.SetRange(0,1023);
	m_yth1_slider.SetPos(0);

	m_yth2.SetRange(0,1023);
	m_yth2.SetPos(0);
	m_yth2_slider.SetRange(0,1023);
	m_yth2_slider.SetPos(0);


	m_gain.SetRange(0,1023);
	m_gain.SetPos(0);
	m_gain_slider.SetRange(0,1023);
	m_gain_slider.SetPos(0);

	EnableLinkageRadio(FALSE);

	for (int i=0; i<10; i++)
	{
		m_area1_keyPts[i].clear();
		m_area1_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area1_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));

		m_area2_keyPts[i].clear();
		m_area2_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area2_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));

		m_area3_keyPts[i].clear();
		m_area3_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area3_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));

		m_area4_keyPts[i].clear();
		m_area4_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area4_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));

		m_area5_keyPts[i].clear();
		m_area5_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area5_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));

		m_area6_keyPts[i].clear();
		m_area6_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area6_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));
	}

	int size = WDR_LUT_LEVEL_NUM * sizeof(unsigned short);
	m_LutDlg.m_Area1Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb1, &size);
	m_LutDlg.m_Area2Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb2, &size);
	m_LutDlg.m_Area3Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb3, &size);
	m_LutDlg.m_Area4Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb4, &size);
	m_LutDlg.m_Area5Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb5, &size);
	m_LutDlg.m_Area6Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb6, &size);

	for (i=0; i<9; i++)
	{
		m_LutDlg.m_Area1Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[i].area_tb1, &size);
		m_LutDlg.m_Area2Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[i].area_tb2, &size);
		m_LutDlg.m_Area3Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[i].area_tb3, &size);
		m_LutDlg.m_Area4Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[i].area_tb4, &size);
		m_LutDlg.m_Area5Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[i].area_tb5, &size);
		m_LutDlg.m_Area6Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[i].area_tb6, &size);
	}

	m_LutDlg.m_Area1Dlg.m_no_key_show_flag = FALSE;
	m_LutDlg.m_Area2Dlg.m_no_key_show_flag = FALSE;
	m_LutDlg.m_Area3Dlg.m_no_key_show_flag = FALSE;
	m_LutDlg.m_Area4Dlg.m_no_key_show_flag = FALSE;
	m_LutDlg.m_Area5Dlg.m_no_key_show_flag = FALSE;
	m_LutDlg.m_Area6Dlg.m_no_key_show_flag = FALSE;

	memcpy((char*)default_lut, (char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb1, size);
	
	return TRUE;
}


BOOL CWdrDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CWdrDlg::OnButton1() 
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

	m_LutDlg.m_Area1Dlg.SetWdrTh(m_th1.GetPos(), 0);
	size = m_area1_keyPts[index].size();

	pts_tmp = m_LutDlg.m_Area1Dlg.GetKeyPts();
	(*pts_tmp).resize(size);
	for (i=0; i<size; i++)
	{
		(*pts_tmp)[i].x = m_area1_keyPts[index][i].x;
		(*pts_tmp)[i].y = m_area1_keyPts[index][i].y;
	}

	m_LutDlg.m_Area2Dlg.SetWdrTh(m_th1.GetPos(), m_th2.GetPos());
	size = m_area2_keyPts[index].size();

	pts_tmp = m_LutDlg.m_Area2Dlg.GetKeyPts();
	(*pts_tmp).resize(size);
	for (i=0; i<size; i++)
	{
		(*pts_tmp)[i].x = m_area2_keyPts[index][i].x;
		(*pts_tmp)[i].y = m_area2_keyPts[index][i].y;
	}

	m_LutDlg.m_Area3Dlg.SetWdrTh(m_th2.GetPos(), m_th3.GetPos());
	size = m_area3_keyPts[index].size();

	pts_tmp = m_LutDlg.m_Area3Dlg.GetKeyPts();
	(*pts_tmp).resize(size);
	for (i=0; i<size; i++)
	{
		(*pts_tmp)[i].x = m_area3_keyPts[index][i].x;
		(*pts_tmp)[i].y = m_area3_keyPts[index][i].y;
	}

	m_LutDlg.m_Area4Dlg.SetWdrTh(m_th3.GetPos(), m_th4.GetPos());
	size = m_area4_keyPts[index].size();

	pts_tmp = m_LutDlg.m_Area4Dlg.GetKeyPts();
	(*pts_tmp).resize(size);
	for (i=0; i<size; i++)
	{
		(*pts_tmp)[i].x = m_area4_keyPts[index][i].x;
		(*pts_tmp)[i].y = m_area4_keyPts[index][i].y;
	}

	m_LutDlg.m_Area5Dlg.SetWdrTh(m_th4.GetPos(), m_th5.GetPos());
	size = m_area5_keyPts[index].size();

	pts_tmp = m_LutDlg.m_Area5Dlg.GetKeyPts();
	(*pts_tmp).resize(size);
	for (i=0; i<size; i++)
	{
		(*pts_tmp)[i].x = m_area5_keyPts[index][i].x;
		(*pts_tmp)[i].y = m_area5_keyPts[index][i].y;
	}

	m_LutDlg.m_Area6Dlg.SetWdrTh(m_th5.GetPos(), 0);
	size = m_area6_keyPts[index].size();

	pts_tmp = m_LutDlg.m_Area6Dlg.GetKeyPts();
	(*pts_tmp).resize(size);
	for (i=0; i<size; i++)
	{
		(*pts_tmp)[i].x = m_area6_keyPts[index][i].x;
		(*pts_tmp)[i].y = m_area6_keyPts[index][i].y;
	}

	int ret = -1;

	ret = m_LutDlg.DoModal();
	if (IDOK == ret)
	{
		size = WDR_LUT_LEVEL_NUM * sizeof(unsigned short);

		if (0 == m_mode)
		{
			m_LutDlg.m_Area1Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb1, &size);
			m_LutDlg.m_Area2Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb2, &size);
			m_LutDlg.m_Area3Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb3, &size);
			m_LutDlg.m_Area4Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb4, &size);
			m_LutDlg.m_Area5Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb5, &size);
			m_LutDlg.m_Area6Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.manual_wdr.area_tb6, &size);
		}
		else
		{
			m_LutDlg.m_Area1Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[m_envi].area_tb1, &size);
			m_LutDlg.m_Area2Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[m_envi].area_tb2, &size);
			m_LutDlg.m_Area3Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[m_envi].area_tb3, &size);
			m_LutDlg.m_Area4Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[m_envi].area_tb4, &size);
			m_LutDlg.m_Area5Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[m_envi].area_tb5, &size);
			m_LutDlg.m_Area6Dlg.GetLevel((char*)m_Wdr.p_wdr_attr.linkage_wdr[m_envi].area_tb6, &size);
		}
		

		pts_tmp = m_LutDlg.m_Area1Dlg.GetKeyPts();
		size = (*pts_tmp).size();
		m_area1_keyPts[index].resize(size);
		for (i=0; i<size; i++)
		{
			m_area1_keyPts[index][i].x = (*pts_tmp)[i].x;
			m_area1_keyPts[index][i].y = (*pts_tmp)[i].y;
		}

		pts_tmp = m_LutDlg.m_Area2Dlg.GetKeyPts();
		size = (*pts_tmp).size();
		m_area2_keyPts[index].resize(size);
		for (i=0; i<size; i++)
		{
			m_area2_keyPts[index][i].x = (*pts_tmp)[i].x;
			m_area2_keyPts[index][i].y = (*pts_tmp)[i].y;
		}

		pts_tmp = m_LutDlg.m_Area3Dlg.GetKeyPts();
		size = (*pts_tmp).size();
		m_area3_keyPts[index].resize(size);
		for (i=0; i<size; i++)
		{
			m_area3_keyPts[index][i].x = (*pts_tmp)[i].x;
			m_area3_keyPts[index][i].y = (*pts_tmp)[i].y;
		}

		pts_tmp = m_LutDlg.m_Area4Dlg.GetKeyPts();
		size = (*pts_tmp).size();
		m_area4_keyPts[index].resize(size);
		for (i=0; i<size; i++)
		{
			m_area4_keyPts[index][i].x = (*pts_tmp)[i].x;
			m_area4_keyPts[index][i].y = (*pts_tmp)[i].y;
		}

		pts_tmp = m_LutDlg.m_Area5Dlg.GetKeyPts();
		size = (*pts_tmp).size();
		m_area5_keyPts[index].resize(size);
		for (i=0; i<size; i++)
		{
			m_area5_keyPts[index][i].x = (*pts_tmp)[i].x;
			m_area5_keyPts[index][i].y = (*pts_tmp)[i].y;
		}

		pts_tmp = m_LutDlg.m_Area6Dlg.GetKeyPts();
		size = (*pts_tmp).size();
		m_area6_keyPts[index].resize(size);
		for (i=0; i<size; i++)
		{
			m_area6_keyPts[index][i].x = (*pts_tmp)[i].x;
			m_area6_keyPts[index][i].y = (*pts_tmp)[i].y;
		}

	}
}

void CWdrDlg::EnableLinkageRadio(bool bEnable) 
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

void CWdrDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th1_slider.SetPos(m_th1.GetPos());
	*pResult = 0;
}

void CWdrDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th2_slider.SetPos(m_th2.GetPos());
	*pResult = 0;
}

void CWdrDlg::OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th3_slider.SetPos(m_th4.GetPos());
	*pResult = 0;
}

void CWdrDlg::OnDeltaposSpin20(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th4_slider.SetPos(m_th4.GetPos());
	*pResult = 0;
}

void CWdrDlg::OnDeltaposSpin160(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th5_slider.SetPos(m_th5.GetPos());
	*pResult = 0;
}


void CWdrDlg::OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_uvlevel_slider.SetPos(m_uv_level.GetPos());
	*pResult = 0;
}



void CWdrDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_th1_slider.SetPos(m_th1.GetPos());
}

void CWdrDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th1.SetPos(m_th1_slider.GetPos());
	UpdateData(FALSE);
	*pResult = 0;
}

void CWdrDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_th2_slider.SetPos(m_th2.GetPos());
}

void CWdrDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th2.SetPos(m_th2_slider.GetPos());
	UpdateData(FALSE);
	*pResult = 0;
}

void CWdrDlg::OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th3.SetPos(m_th3_slider.GetPos());
	UpdateData(FALSE);
	*pResult = 0;
}

void CWdrDlg::OnCustomdrawSlider23(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th4.SetPos(m_th4_slider.GetPos());
	UpdateData(FALSE);
	*pResult = 0;
}

void CWdrDlg::OnCustomdrawSlider24(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th5.SetPos(m_th5_slider.GetPos());
	UpdateData(FALSE);
	*pResult = 0;
}

void CWdrDlg::OnKillfocusEdit5() 
{
	// TODO: Add your control notification handler code here
	m_th3_slider.SetPos(m_th3.GetPos());
}

void CWdrDlg::OnKillfocusEdit145() 
{
	// TODO: Add your control notification handler code here
	m_th4_slider.SetPos(m_th4.GetPos());
}

void CWdrDlg::OnKillfocusEdit159() 
{
	// TODO: Add your control notification handler code here
	m_th5_slider.SetPos(m_th5.GetPos());
}


void CWdrDlg::OnKillfocusEdit4() 
{
	// TODO: Add your control notification handler code here
	m_uvlevel_slider.SetPos(m_uv_level.GetPos());
}

void CWdrDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_uv_level.SetPos(m_uvlevel_slider.GetPos());
	UpdateData(FALSE);
	*pResult = 0;
}


void CWdrDlg::OnKillfocusEdit6() 
{
	// TODO: Add your control notification handler code here
	m_yth1_slider.SetPos(m_yth1.GetPos());
}

void CWdrDlg::OnDeltaposSpin5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_yth1_slider.SetPos(m_yth1.GetPos());
	*pResult = 0;
}

void CWdrDlg::OnCustomdrawSlider5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_yth1.SetPos(m_yth1_slider.GetPos());
	UpdateData(FALSE);
	*pResult = 0;
}

void CWdrDlg::OnKillfocusEdit7() 
{
	// TODO: Add your control notification handler code here
	m_yth2_slider.SetPos(m_yth2.GetPos());
}

void CWdrDlg::OnDeltaposSpin6(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_yth2_slider.SetPos(m_yth2.GetPos());
	*pResult = 0;
}

void CWdrDlg::OnCustomdrawSlider6(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_yth2.SetPos(m_yth2_slider.GetPos());
	UpdateData(FALSE);
	*pResult = 0;
}

void CWdrDlg::OnKillfocusEdit8() 
{
	// TODO: Add your control notification handler code here
	m_gain_slider.SetPos(m_gain.GetPos());
}

void CWdrDlg::OnDeltaposSpin8(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_gain_slider.SetPos(m_gain.GetPos());
	*pResult = 0;
}

void CWdrDlg::OnCustomdrawSlider10(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_gain.SetPos(m_gain_slider.GetPos());
	UpdateData(FALSE);
	*pResult = 0;
}


void CWdrDlg::OnButtonWrite() 
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

	file.Write(&m_Wdr, sizeof(AK_ISP_INIT_WDR));

	file.Close();
}

void CWdrDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_WDR))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Wdr, sizeof(AK_ISP_INIT_WDR));
	file.Close();

	SetDataValue(TRUE);

	UpdateData(FALSE);
}

void CWdrDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_WDR, 0);
}

void CWdrDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_WDR, 0);
}

void CWdrDlg::OnRadio12() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CWdrDlg::OnRadio13() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CWdrDlg::OnRadio14() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CWdrDlg::OnRadio15() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CWdrDlg::OnRadio10() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(FALSE);
	UpdateData(FALSE);
}

void CWdrDlg::OnRadio11() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(TRUE);
	UpdateData(FALSE);
}

void CWdrDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CWdrDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CWdrDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CWdrDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CWdrDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CWdrDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CWdrDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CWdrDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CWdrDlg::OnRadio9() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CWdrDlg::CheckData(AK_ISP_INIT_WDR* p_Wdr)
{
	int i = 0;
	
	if (p_Wdr->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1 == p_Wdr->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2)
	{
		if (p_Wdr->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2 < 1023)
		{
			p_Wdr->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2++;
		}
		else
		{
			p_Wdr->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1--;
		}
	}
	else if (p_Wdr->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1 > p_Wdr->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2)
	{
		int tmp = p_Wdr->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1;

		p_Wdr->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1 = p_Wdr->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2;
		p_Wdr->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2 = tmp;
	}

	for (i=0; i<9; i++)
	{
		if (p_Wdr->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth1 == p_Wdr->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth2)
		{
			if (p_Wdr->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth2 < 1023)
			{
				p_Wdr->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth2++;
			}
			else
			{
				p_Wdr->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth1--;
			}
		}
		else if (p_Wdr->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth1 > p_Wdr->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth2)
		{
			int tmp = p_Wdr->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth1;

			p_Wdr->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth1 = p_Wdr->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth2;
			p_Wdr->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth2 = tmp;
		}
	}
}

void CWdrDlg::GetDataValue(bool bToStruct)
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


	m_Wdr.param_id = ISP_WDR;
	m_Wdr.length = sizeof(AK_ISP_INIT_WDR);

	m_Wdr.p_wdr_attr.wdr_mode = (T_U16)m_mode;

	if (0 == mode_tmp)
	{
		m_Wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_level = (T_U16)m_uv_level.GetPos();
		m_Wdr.p_wdr_attr.manual_wdr.wdr_enable = (T_U16)m_WdrEnable;
		m_Wdr.p_wdr_attr.manual_wdr.wdr_th1 = (T_U16)m_th1.GetPos();
		m_Wdr.p_wdr_attr.manual_wdr.wdr_th2 = (T_U16)m_th2.GetPos();
		m_Wdr.p_wdr_attr.manual_wdr.wdr_th3 = (T_U16)m_th3.GetPos();
		m_Wdr.p_wdr_attr.manual_wdr.wdr_th4 = (T_U16)m_th4.GetPos();
		m_Wdr.p_wdr_attr.manual_wdr.wdr_th5 = (T_U16)m_th5.GetPos();
		
		m_Wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_enable = (T_U16)m_uvEnable;
		m_Wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1 = (T_U16)m_yth1.GetPos();
		m_Wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2 = (T_U16)m_yth2.GetPos();
		m_Wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain = (T_U16)m_gain.GetPos();
	}
	else
	{
		m_Wdr.p_wdr_attr.linkage_wdr[envi_tmp].hdr_uv_adjust_level = (T_U16)m_uv_level.GetPos();
		m_Wdr.p_wdr_attr.linkage_wdr[envi_tmp].wdr_enable = (T_U16)m_WdrEnable;
		m_Wdr.p_wdr_attr.linkage_wdr[envi_tmp].wdr_th1 = (T_U16)m_th1.GetPos();
		m_Wdr.p_wdr_attr.linkage_wdr[envi_tmp].wdr_th2 = (T_U16)m_th2.GetPos();
		m_Wdr.p_wdr_attr.linkage_wdr[envi_tmp].wdr_th3 = (T_U16)m_th3.GetPos();
		m_Wdr.p_wdr_attr.linkage_wdr[envi_tmp].wdr_th4 = (T_U16)m_th4.GetPos();
		m_Wdr.p_wdr_attr.linkage_wdr[envi_tmp].wdr_th5 = (T_U16)m_th5.GetPos();
		m_Wdr.p_wdr_attr.linkage_wdr[envi_tmp].hdr_uv_adjust_enable = (T_U16)m_uvEnable;
		m_Wdr.p_wdr_attr.linkage_wdr[envi_tmp].hdr_cnoise_suppress_yth1 = (T_U16)m_yth1.GetPos();
		m_Wdr.p_wdr_attr.linkage_wdr[envi_tmp].hdr_cnoise_suppress_yth2 = (T_U16)m_yth2.GetPos();
		m_Wdr.p_wdr_attr.linkage_wdr[envi_tmp].hdr_cnoise_suppress_gain = (T_U16)m_gain.GetPos();	
	}

	int size = m_area1_keyPts[0].size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	memset(&m_Wdr.p_wdr_attr.manual_wdr.area1_key, 0, sizeof(T_U16) * 16);

	for (int i=0; i<size; i++)
	{
		m_Wdr.p_wdr_attr.manual_wdr.area1_key[i] = m_area1_keyPts[0][i].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
	}

	size = m_area2_keyPts[0].size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	memset(&m_Wdr.p_wdr_attr.manual_wdr.area2_key, 0, sizeof(T_U16) * 16);

	for (i=0; i<size; i++)
	{
		m_Wdr.p_wdr_attr.manual_wdr.area2_key[i] = m_area2_keyPts[0][i].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
	}

	size = m_area3_keyPts[0].size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	memset(&m_Wdr.p_wdr_attr.manual_wdr.area3_key, 0, sizeof(T_U16) * 16);

	for (i=0; i<size; i++)
	{
		m_Wdr.p_wdr_attr.manual_wdr.area3_key[i] = m_area3_keyPts[0][i].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
	}

	size = m_area4_keyPts[0].size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	memset(&m_Wdr.p_wdr_attr.manual_wdr.area4_key, 0, sizeof(T_U16) * 16);

	for (i=0; i<size; i++)
	{
		m_Wdr.p_wdr_attr.manual_wdr.area4_key[i] = m_area4_keyPts[0][i].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
	}

	size = m_area5_keyPts[0].size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	memset(&m_Wdr.p_wdr_attr.manual_wdr.area5_key, 0, sizeof(T_U16) * 16);

	for (i=0; i<size; i++)
	{
		m_Wdr.p_wdr_attr.manual_wdr.area5_key[i] = m_area5_keyPts[0][i].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
	}

	size = m_area6_keyPts[0].size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	memset(&m_Wdr.p_wdr_attr.manual_wdr.area6_key, 0, sizeof(T_U16) * 16);

	for (i=0; i<size; i++)
	{
		m_Wdr.p_wdr_attr.manual_wdr.area6_key[i] = m_area6_keyPts[0][i].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
	}

	
	//linkage
	for (i=0; i<9; i++)
	{
		size = m_area1_keyPts[i+1].size();
		if (size > KEY_POINT_MAX)
		{
			size = KEY_POINT_MAX;
		}

		memset(&m_Wdr.p_wdr_attr.linkage_wdr[i].area1_key, 0, sizeof(T_U16) * 16);

		for (int j=0; j<size; j++)
		{
			m_Wdr.p_wdr_attr.linkage_wdr[i].area1_key[j] = m_area1_keyPts[i+1][j].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
		}

		size = m_area2_keyPts[i+1].size();
		if (size > KEY_POINT_MAX)
		{
			size = KEY_POINT_MAX;
		}

		memset(&m_Wdr.p_wdr_attr.linkage_wdr[i].area2_key, 0, sizeof(T_U16) * 16);

		for (j=0; j<size; j++)
		{
			m_Wdr.p_wdr_attr.linkage_wdr[i].area2_key[j] = m_area2_keyPts[i+1][j].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
		}

		size = m_area3_keyPts[i+1].size();
		if (size > KEY_POINT_MAX)
		{
			size = KEY_POINT_MAX;
		}

		memset(&m_Wdr.p_wdr_attr.linkage_wdr[i].area3_key, 0, sizeof(T_U16) * 16);

		for (j=0; j<size; j++)
		{
			m_Wdr.p_wdr_attr.linkage_wdr[i].area3_key[j] = m_area3_keyPts[i+1][j].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
		}

		size = m_area4_keyPts[i+1].size();
		if (size > KEY_POINT_MAX)
		{
			size = KEY_POINT_MAX;
		}

		memset(&m_Wdr.p_wdr_attr.linkage_wdr[i].area4_key, 0, sizeof(T_U16) * 16);

		for (j=0; j<size; j++)
		{
			m_Wdr.p_wdr_attr.linkage_wdr[i].area4_key[j] = m_area4_keyPts[i+1][j].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
		}

		size = m_area5_keyPts[i+1].size();
		if (size > KEY_POINT_MAX)
		{
			size = KEY_POINT_MAX;
		}

		memset(&m_Wdr.p_wdr_attr.linkage_wdr[i].area5_key, 0, sizeof(T_U16) * 16);

		for (j=0; j<size; j++)
		{
			m_Wdr.p_wdr_attr.linkage_wdr[i].area5_key[j] = m_area5_keyPts[i+1][j].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
		}

		size = m_area6_keyPts[i+1].size();
		if (size > KEY_POINT_MAX)
		{
			size = KEY_POINT_MAX;
		}

		memset(&m_Wdr.p_wdr_attr.linkage_wdr[i].area6_key, 0, sizeof(T_U16) * 16);

		for (j=0; j<size; j++)
		{
			m_Wdr.p_wdr_attr.linkage_wdr[i].area6_key[j] = m_area6_keyPts[i+1][j].x * (WDR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
		}
	}

	CheckData(&m_Wdr);

}

void CWdrDlg::SetDataValue(bool bFromStruct)
{
	if (bFromStruct)
	{
		m_mode = m_Wdr.p_wdr_attr.wdr_mode;
	}

	if (0 == m_mode)
	{
		m_uv_level.SetPos(m_Wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_level);
		m_uvlevel_slider.SetPos(m_Wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_level);
		m_WdrEnable = m_Wdr.p_wdr_attr.manual_wdr.wdr_enable;
		m_th1.SetPos(m_Wdr.p_wdr_attr.manual_wdr.wdr_th1);
		m_th1_slider.SetPos(m_Wdr.p_wdr_attr.manual_wdr.wdr_th1);
		m_th2.SetPos(m_Wdr.p_wdr_attr.manual_wdr.wdr_th2);
		m_th2_slider.SetPos(m_Wdr.p_wdr_attr.manual_wdr.wdr_th2);
		m_th3.SetPos(m_Wdr.p_wdr_attr.manual_wdr.wdr_th3);
		m_th3_slider.SetPos(m_Wdr.p_wdr_attr.manual_wdr.wdr_th3);
		m_th4.SetPos(m_Wdr.p_wdr_attr.manual_wdr.wdr_th4);
		m_th4_slider.SetPos(m_Wdr.p_wdr_attr.manual_wdr.wdr_th4);
		m_th5.SetPos(m_Wdr.p_wdr_attr.manual_wdr.wdr_th5);
		m_th5_slider.SetPos(m_Wdr.p_wdr_attr.manual_wdr.wdr_th5);
		m_uvEnable = m_Wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_enable;
		m_yth1.SetPos(m_Wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1);
		m_yth1_slider.SetPos(m_Wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1);
		m_yth2.SetPos(m_Wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2);
		m_yth2_slider.SetPos(m_Wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2);
		m_gain.SetPos(m_Wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain);
		m_gain_slider.SetPos(m_Wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain);

		EnableLinkageRadio(FALSE);
	}
	else
	{
		m_uv_level.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].hdr_uv_adjust_level);
		m_uvlevel_slider.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].hdr_uv_adjust_level);
		m_WdrEnable = m_Wdr.p_wdr_attr.linkage_wdr[m_envi].wdr_enable;
		m_th1.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].wdr_th1);
		m_th1_slider.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].wdr_th1);
		m_th2.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].wdr_th2);
		m_th2_slider.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].wdr_th2);
		m_th3.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].wdr_th3);
		m_th3_slider.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].wdr_th3);
		m_th4.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].wdr_th4);
		m_th4_slider.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].wdr_th4);
		m_th5.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].wdr_th5);
		m_th5_slider.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].wdr_th5);
		m_uvEnable = m_Wdr.p_wdr_attr.linkage_wdr[m_envi].hdr_uv_adjust_enable;
		m_yth1.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].hdr_cnoise_suppress_yth1);
		m_yth1_slider.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].hdr_cnoise_suppress_yth1);
		m_yth2.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].hdr_cnoise_suppress_yth2);
		m_yth2_slider.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].hdr_cnoise_suppress_yth2);
		m_gain.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].hdr_cnoise_suppress_gain);
		m_gain_slider.SetPos(m_Wdr.p_wdr_attr.linkage_wdr[m_envi].hdr_cnoise_suppress_gain);

		EnableLinkageRadio(TRUE);
	}


	m_LutDlg.m_Area1Dlg.SetKeyPts(&m_area1_keyPts[0], m_Wdr.p_wdr_attr.manual_wdr.area1_key, m_Wdr.p_wdr_attr.manual_wdr.area_tb1);
	m_LutDlg.m_Area2Dlg.SetKeyPts(&m_area2_keyPts[0], m_Wdr.p_wdr_attr.manual_wdr.area2_key, m_Wdr.p_wdr_attr.manual_wdr.area_tb2);
	m_LutDlg.m_Area3Dlg.SetKeyPts(&m_area3_keyPts[0], m_Wdr.p_wdr_attr.manual_wdr.area3_key, m_Wdr.p_wdr_attr.manual_wdr.area_tb3);
	m_LutDlg.m_Area4Dlg.SetKeyPts(&m_area4_keyPts[0], m_Wdr.p_wdr_attr.manual_wdr.area4_key, m_Wdr.p_wdr_attr.manual_wdr.area_tb4);
	m_LutDlg.m_Area5Dlg.SetKeyPts(&m_area5_keyPts[0], m_Wdr.p_wdr_attr.manual_wdr.area5_key, m_Wdr.p_wdr_attr.manual_wdr.area_tb5);
	m_LutDlg.m_Area6Dlg.SetKeyPts(&m_area6_keyPts[0], m_Wdr.p_wdr_attr.manual_wdr.area6_key, m_Wdr.p_wdr_attr.manual_wdr.area_tb6);

	for (int j=0; j<9; j++)
	{
		m_LutDlg.m_Area1Dlg.SetKeyPts(&m_area1_keyPts[j+1], m_Wdr.p_wdr_attr.linkage_wdr[j].area1_key, m_Wdr.p_wdr_attr.linkage_wdr[j].area_tb1);
		m_LutDlg.m_Area2Dlg.SetKeyPts(&m_area2_keyPts[j+1], m_Wdr.p_wdr_attr.linkage_wdr[j].area2_key, m_Wdr.p_wdr_attr.linkage_wdr[j].area_tb2);
		m_LutDlg.m_Area3Dlg.SetKeyPts(&m_area3_keyPts[j+1], m_Wdr.p_wdr_attr.linkage_wdr[j].area3_key, m_Wdr.p_wdr_attr.linkage_wdr[j].area_tb3);
		m_LutDlg.m_Area4Dlg.SetKeyPts(&m_area4_keyPts[j+1], m_Wdr.p_wdr_attr.linkage_wdr[j].area4_key, m_Wdr.p_wdr_attr.linkage_wdr[j].area_tb4);
		m_LutDlg.m_Area5Dlg.SetKeyPts(&m_area5_keyPts[j+1], m_Wdr.p_wdr_attr.linkage_wdr[j].area5_key, m_Wdr.p_wdr_attr.linkage_wdr[j].area_tb5);
		m_LutDlg.m_Area6Dlg.SetKeyPts(&m_area6_keyPts[j+1], m_Wdr.p_wdr_attr.linkage_wdr[j].area6_key, m_Wdr.p_wdr_attr.linkage_wdr[j].area_tb6);
	}


	((CButton *)GetDlgItem(IDC_RADIO10))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO11))->SetCheck(m_mode);
	((CButton *)GetDlgItem(IDC_RADIO12))->SetCheck(!m_WdrEnable);
	((CButton *)GetDlgItem(IDC_RADIO13))->SetCheck(m_WdrEnable);
	((CButton *)GetDlgItem(IDC_RADIO14))->SetCheck(!m_uvEnable);
	((CButton *)GetDlgItem(IDC_RADIO15))->SetCheck(m_uvEnable);
}

int CWdrDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_WDR))) return -1;

	GetDataValue(TRUE);

	memcpy(pPageInfoSt, &m_Wdr, sizeof(AK_ISP_INIT_WDR));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_WDR);

	return 0;
}


int CWdrDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_WDR))) return -1;

	memcpy(&m_Wdr, pPageInfoSt, sizeof(AK_ISP_INIT_WDR));

	SetDataValue(TRUE);
	
	return 0;
}

void CWdrDlg::Clean(void) 
{
	ZeroMemory(&m_Wdr, sizeof(AK_ISP_INIT_WDR));
	EnableLinkageRadio(FALSE);
	SetDataValue(TRUE);
	UpdateData(FALSE);

	for (int i=0; i<10; i++)
	{
		m_area1_keyPts[i].clear();
		m_area1_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area1_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));

		m_area2_keyPts[i].clear();
		m_area2_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area2_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));

		m_area3_keyPts[i].clear();
		m_area3_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area3_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));

		m_area4_keyPts[i].clear();
		m_area4_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area4_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));

		m_area5_keyPts[i].clear();
		m_area5_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area5_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));

		m_area6_keyPts[i].clear();
		m_area6_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_area6_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));
	}
}


int CWdrDlg::Convert_v2_data(AK_ISP_INIT_WDR* struct_new, AK_ISP_INIT_WDR_V2* struct_v2) 
{
	int i = 0, j = 0;
	
	if ((struct_v2 == NULL) || (struct_new == NULL)) return -1;
	
	struct_new->param_id = ISP_WDR;
	struct_new->length = sizeof(AK_ISP_INIT_WDR);


	struct_new->p_wdr_attr.wdr_mode = struct_v2->p_wdr_attr.wdr_mode;
	struct_new->p_wdr_attr.manual_wdr.hdr_uv_adjust_level = struct_v2->p_wdr_attr.manual_wdr.hdr_uv_adjust_level;
	struct_new->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_slop = struct_v2->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_slop;
	struct_new->p_wdr_attr.manual_wdr.wdr_enable = struct_v2->p_wdr_attr.manual_wdr.wdr_enable;
	struct_new->p_wdr_attr.manual_wdr.hdr_uv_adjust_enable = struct_v2->p_wdr_attr.manual_wdr.hdr_uv_adjust_enable;
	struct_new->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1 = struct_v2->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1;
	struct_new->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2 = struct_v2->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2;
	struct_new->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain = struct_v2->p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain;
	struct_new->p_wdr_attr.manual_wdr.wdr_th1 = 0;
	struct_new->p_wdr_attr.manual_wdr.wdr_th2 = 0;
	struct_new->p_wdr_attr.manual_wdr.wdr_th3 = 0;
	struct_new->p_wdr_attr.manual_wdr.wdr_th4 = struct_v2->p_wdr_attr.manual_wdr.wdr_dark_thre / 4;
	struct_new->p_wdr_attr.manual_wdr.wdr_th5 = struct_v2->p_wdr_attr.manual_wdr.wdr_light_thre / 4;

	memcpy((char*)struct_new->p_wdr_attr.manual_wdr.area_tb1, (char*)default_lut, 65 * sizeof(T_U16));
	memcpy((char*)struct_new->p_wdr_attr.manual_wdr.area_tb2, (char*)default_lut, 65 * sizeof(T_U16));
	memcpy((char*)struct_new->p_wdr_attr.manual_wdr.area_tb3, (char*)default_lut, 65 * sizeof(T_U16));

	for(i=0; i<65; i++)
	{
		struct_new->p_wdr_attr.manual_wdr.area_tb4[i] = struct_v2->p_wdr_attr.manual_wdr.dark_tbl[i*2];
	}
	
	for(i=0; i<65; i++)
	{
		struct_new->p_wdr_attr.manual_wdr.area_tb5[i] = struct_v2->p_wdr_attr.manual_wdr.mid_tbl[i*2];
	}

	for(i=0; i<65; i++)
	{
		struct_new->p_wdr_attr.manual_wdr.area_tb6[i] = struct_v2->p_wdr_attr.manual_wdr.light_tbl[i*2];
	}

	struct_new->p_wdr_attr.manual_wdr.area1_key[1] = 64;
	struct_new->p_wdr_attr.manual_wdr.area2_key[1] = 64;
	struct_new->p_wdr_attr.manual_wdr.area3_key[1] = 64;

	for(i=0; i<8; i++)
	{
		struct_new->p_wdr_attr.manual_wdr.area4_key[i] = struct_v2->p_wdr_attr.manual_wdr.dark_key[i] / 2;
	}

	for(i=0; i<8; i++)
	{
		struct_new->p_wdr_attr.manual_wdr.area5_key[i] = struct_v2->p_wdr_attr.manual_wdr.mid_key[i] / 2;
	}

	for(i=0; i<8; i++)
	{
		struct_new->p_wdr_attr.manual_wdr.area6_key[i] = struct_v2->p_wdr_attr.manual_wdr.light_key[i] / 2;
	}
	
	for (i=0; i<9; i++)
	{
		struct_new->p_wdr_attr.linkage_wdr[i].hdr_uv_adjust_level = struct_v2->p_wdr_attr.linkage_wdr[i].hdr_uv_adjust_level;
		struct_new->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_slop = struct_v2->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_slop;
		struct_new->p_wdr_attr.linkage_wdr[i].wdr_enable = struct_v2->p_wdr_attr.linkage_wdr[i].wdr_enable;
		struct_new->p_wdr_attr.linkage_wdr[i].hdr_uv_adjust_enable = struct_v2->p_wdr_attr.linkage_wdr[i].hdr_uv_adjust_enable;
		struct_new->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth1 = struct_v2->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth1;
		struct_new->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth2 = struct_v2->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_yth2;
		struct_new->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_gain = struct_v2->p_wdr_attr.linkage_wdr[i].hdr_cnoise_suppress_gain;
		
		struct_new->p_wdr_attr.linkage_wdr[i].wdr_th1 = 0;
		struct_new->p_wdr_attr.linkage_wdr[i].wdr_th2 = 0;
		struct_new->p_wdr_attr.linkage_wdr[i].wdr_th3 = 0;
		struct_new->p_wdr_attr.linkage_wdr[i].wdr_th4 = struct_v2->p_wdr_attr.linkage_wdr[i].wdr_dark_thre / 4;
		struct_new->p_wdr_attr.linkage_wdr[i].wdr_th5 = struct_v2->p_wdr_attr.linkage_wdr[i].wdr_light_thre / 4;

		
		memcpy((char*)struct_new->p_wdr_attr.linkage_wdr[i].area_tb1, (char*)default_lut, 65 * sizeof(T_U16));
		memcpy((char*)struct_new->p_wdr_attr.linkage_wdr[i].area_tb2, (char*)default_lut, 65 * sizeof(T_U16));
		memcpy((char*)struct_new->p_wdr_attr.linkage_wdr[i].area_tb3, (char*)default_lut, 65 * sizeof(T_U16));

		for(j=0; j<65; j++)
		{
			struct_new->p_wdr_attr.linkage_wdr[i].area_tb4[j] = struct_v2->p_wdr_attr.linkage_wdr[i].dark_tbl[j*2];
		}
		
		for(j=0; j<65; j++)
		{
			struct_new->p_wdr_attr.linkage_wdr[i].area_tb5[j] = struct_v2->p_wdr_attr.linkage_wdr[i].mid_tbl[j*2];
		}

		for(j=0; j<65; j++)
		{
			struct_new->p_wdr_attr.linkage_wdr[i].area_tb6[j] = struct_v2->p_wdr_attr.linkage_wdr[i].light_tbl[j*2];
		}


		struct_new->p_wdr_attr.linkage_wdr[i].area1_key[1] = 64;
		struct_new->p_wdr_attr.linkage_wdr[i].area2_key[1] = 64;
		struct_new->p_wdr_attr.linkage_wdr[i].area3_key[1] = 64;
	
		for(j=0; j<8; j++)
		{
			struct_new->p_wdr_attr.linkage_wdr[i].area4_key[j] = struct_v2->p_wdr_attr.linkage_wdr[i].dark_key[j] / 2;
		}

		for(j=0; j<8; j++)
		{
			struct_new->p_wdr_attr.linkage_wdr[i].area5_key[j] = struct_v2->p_wdr_attr.linkage_wdr[i].mid_key[j] / 2;
		}

		for(j=0; j<8; j++)
		{
			struct_new->p_wdr_attr.linkage_wdr[i].area6_key[j] = struct_v2->p_wdr_attr.linkage_wdr[i].light_key[j] / 2;
		}
	}

	memcpy((char*)struct_new->p_wdr_attr.linkage_wdr[8].area_tb4, (char*)default_lut, 65 * sizeof(T_U16));
	memcpy((char*)struct_new->p_wdr_attr.linkage_wdr[8].area_tb5, (char*)default_lut, 65 * sizeof(T_U16));
	memcpy((char*)struct_new->p_wdr_attr.linkage_wdr[8].area_tb6, (char*)default_lut, 65 * sizeof(T_U16));

	struct_new->p_wdr_attr.linkage_wdr[8].area1_key[1] = 64;
	struct_new->p_wdr_attr.linkage_wdr[8].area2_key[1] = 64;
	struct_new->p_wdr_attr.linkage_wdr[8].area3_key[1] = 64;
	
	return 0;
}

