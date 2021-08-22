// AEOtherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "AEOtherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAEOtherDlg dialog


CAEOtherDlg::CAEOtherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAEOtherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAEOtherDlg)
	m_Enable = 0;
	//}}AFX_DATA_INIT
	ZeroMemory(m_range, sizeof(m_range));
	ZeroMemory(m_hist_wt, sizeof(m_hist_wt));
}


void CAEOtherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAEOtherDlg)
	DDX_Control(pDX, IDC_SLIDER22, m_rate_min_slider);
	DDX_Control(pDX, IDC_SLIDER21, m_rate_max_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_detect_scope_slider);
	DDX_Control(pDX, IDC_SPIN12, m_rate_min);
	DDX_Control(pDX, IDC_SPIN10, m_rate_max);
	DDX_Control(pDX, IDC_SPIN2, m_detect_scope);
	DDX_Radio(pDX, IDC_RADIO1, m_Enable);

	//}}AFX_DATA_MAP

	DDX_Text(pDX, IDC_EDIT1, m_hist_wt[0]);
	DDV_MinMaxInt(pDX, m_hist_wt[0], 0, 16);
	DDX_Text(pDX, IDC_EDIT2, m_hist_wt[1]);
	DDV_MinMaxInt(pDX, m_hist_wt[1], 0, 16);
	DDX_Text(pDX, IDC_EDIT3, m_hist_wt[2]);
	DDV_MinMaxInt(pDX, m_hist_wt[2], 0, 16);
	DDX_Text(pDX, IDC_EDIT4, m_hist_wt[3]);
	DDV_MinMaxInt(pDX, m_hist_wt[3], 0, 16);
	DDX_Text(pDX, IDC_EDIT5, m_hist_wt[4]);
	DDV_MinMaxInt(pDX, m_hist_wt[4], 0, 16);
	DDX_Text(pDX, IDC_EDIT6, m_hist_wt[5]);
	DDV_MinMaxInt(pDX, m_hist_wt[5], 0, 16);
	DDX_Text(pDX, IDC_EDIT7, m_hist_wt[6]);
	DDV_MinMaxInt(pDX, m_hist_wt[6], 0, 16);
	DDX_Text(pDX, IDC_EDIT8, m_hist_wt[7]);
	DDV_MinMaxInt(pDX, m_hist_wt[7], 0, 16);
	DDX_Text(pDX, IDC_EDIT9, m_hist_wt[8]);
	DDV_MinMaxInt(pDX, m_hist_wt[8], 0, 16);
	DDX_Text(pDX, IDC_EDIT10, m_hist_wt[9]);
	DDV_MinMaxInt(pDX, m_hist_wt[9], 0, 16);
	DDX_Text(pDX, IDC_EDIT11, m_hist_wt[10]);
	DDV_MinMaxInt(pDX, m_hist_wt[10], 0, 16);
	DDX_Text(pDX, IDC_EDIT12, m_hist_wt[11]);
	DDV_MinMaxInt(pDX, m_hist_wt[11], 0, 16);
	DDX_Text(pDX, IDC_EDIT13, m_hist_wt[12]);
	DDV_MinMaxInt(pDX, m_hist_wt[12], 0, 16);
	DDX_Text(pDX, IDC_EDIT14, m_hist_wt[13]);
	DDV_MinMaxInt(pDX, m_hist_wt[13], 0, 16);
	DDX_Text(pDX, IDC_EDIT015, m_hist_wt[14]);
	DDV_MinMaxInt(pDX, m_hist_wt[14], 0, 16);
	DDX_Text(pDX, IDC_EDIT16, m_hist_wt[15]);
	DDV_MinMaxInt(pDX, m_hist_wt[15], 0, 16);

	DDX_Text(pDX, IDC_EDIT_RANGE00, m_range[0][0]);
	DDX_Text(pDX, IDC_EDIT_RANGE01, m_range[0][1]);
	DDX_Text(pDX, IDC_EDIT_RANGE10, m_range[1][0]);
	DDX_Text(pDX, IDC_EDIT_RANGE11, m_range[1][1]);
	DDX_Text(pDX, IDC_EDIT_RANGE20, m_range[2][0]);
	DDX_Text(pDX, IDC_EDIT_RANGE21, m_range[2][1]);
	DDX_Text(pDX, IDC_EDIT_RANGE30, m_range[3][0]);
	DDX_Text(pDX, IDC_EDIT_RANGE31, m_range[3][1]);
	DDX_Text(pDX, IDC_EDIT_RANGE40, m_range[4][0]);
	DDX_Text(pDX, IDC_EDIT_RANGE41, m_range[4][1]);
	DDX_Text(pDX, IDC_EDIT_RANGE50, m_range[5][0]);
	DDX_Text(pDX, IDC_EDIT_RANGE51, m_range[5][1]);
	DDX_Text(pDX, IDC_EDIT_RANGE60, m_range[6][0]);
	DDX_Text(pDX, IDC_EDIT_RANGE61, m_range[6][1]);
	DDX_Text(pDX, IDC_EDIT_RANGE70, m_range[7][0]);
	DDX_Text(pDX, IDC_EDIT_RANGE71, m_range[7][1]);
	DDX_Text(pDX, IDC_EDIT_RANGE80, m_range[8][0]);
	DDX_Text(pDX, IDC_EDIT_RANGE81, m_range[8][1]);
	DDX_Text(pDX, IDC_EDIT_RANGE90, m_range[9][0]);
	DDX_Text(pDX, IDC_EDIT_RANGE91, m_range[9][1]);
}


BEGIN_MESSAGE_MAP(CAEOtherDlg, CDialog)
	//{{AFX_MSG_MAP(CAEOtherDlg)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, OnDeltaposSpin2)
	ON_EN_KILLFOCUS(IDC_EDIT154, OnKillfocusEdit154)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN10, OnDeltaposSpin10)
	ON_EN_KILLFOCUS(IDC_EDIT155, OnKillfocusEdit155)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER21, OnCustomdrawSlider21)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN12, OnDeltaposSpin12)
	ON_EN_KILLFOCUS(IDC_EDIT144, OnKillfocusEdit144)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER22, OnCustomdrawSlider22)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAEOtherDlg message handlers

BOOL CAEOtherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_detect_scope.SetRange(0,255);
	m_detect_scope_slider.SetRange(0,255);

	m_rate_max.SetRange(0,255);
	m_rate_max_slider.SetRange(0,255);

	m_rate_min.SetRange(0,255);
	m_rate_min_slider.SetRange(0,255);

	SetDataValue();

	return TRUE;
}

BOOL CAEOtherDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CAEOtherDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();

	GetDataValue();
}

void CAEOtherDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}


void CAEOtherDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_detect_scope_slider.SetPos(m_detect_scope.GetPos());
	*pResult = 0;
}

void CAEOtherDlg::OnKillfocusEdit154() 
{
	// TODO: Add your control notification handler code here
	m_detect_scope_slider.SetPos(m_detect_scope.GetPos());
}

void CAEOtherDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_detect_scope.SetPos(m_detect_scope_slider.GetPos());
	*pResult = 0;
}

void CAEOtherDlg::OnDeltaposSpin10(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_rate_max_slider.SetPos(m_rate_max.GetPos());
	*pResult = 0;
}

void CAEOtherDlg::OnKillfocusEdit155() 
{
	// TODO: Add your control notification handler code here
	m_rate_max_slider.SetPos(m_rate_max.GetPos());
}

void CAEOtherDlg::OnCustomdrawSlider21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_rate_max.SetPos(m_rate_max_slider.GetPos());
	*pResult = 0;
}

void CAEOtherDlg::OnDeltaposSpin12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_rate_min_slider.SetPos(m_rate_min.GetPos());
	*pResult = 0;
}

void CAEOtherDlg::OnKillfocusEdit144() 
{
	// TODO: Add your control notification handler code here
	m_rate_min_slider.SetPos(m_rate_min.GetPos());
}

void CAEOtherDlg::OnCustomdrawSlider22(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_rate_min.SetPos(m_rate_min_slider.GetPos());
	*pResult = 0;
}


void CAEOtherDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CAEOtherDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CAEOtherDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	m_AEOther.OE_suppress_en = (T_U16)m_Enable;

	m_AEOther.OE_detect_scope = (T_U16)m_detect_scope.GetPos();
	m_AEOther.OE_rate_max = (T_U16)m_rate_max.GetPos();
	m_AEOther.OE_rate_min = (T_U16)m_rate_min.GetPos();

	memcpy(&m_AEOther.envi_gain_range, m_range, sizeof(m_range));
	memcpy(&m_AEOther.hist_weight, m_hist_wt, sizeof(m_hist_wt));
}

void CAEOtherDlg::SetDataValue(void)
{
	m_Enable = m_AEOther.OE_suppress_en;

	m_detect_scope.SetPos(m_AEOther.OE_detect_scope);
	m_detect_scope_slider.SetPos(m_AEOther.OE_detect_scope);
	m_rate_max.SetPos(m_AEOther.OE_rate_max);
	m_rate_max_slider.SetPos(m_AEOther.OE_rate_max);
	m_rate_min.SetPos(m_AEOther.OE_rate_min);
	m_rate_min_slider.SetPos(m_AEOther.OE_rate_min);

	memcpy(m_range, &m_AEOther.envi_gain_range, sizeof(m_range));
	memcpy(m_hist_wt, &m_AEOther.hist_weight, sizeof(m_hist_wt));

	UpdateData(FALSE);
}