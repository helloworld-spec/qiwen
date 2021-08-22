// MEDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "MEDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMEDlg dialog


CMEDlg::CMEDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMEDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMEDlg)
	//}}AFX_DATA_INIT
	m_title.Format("%s", "Frame Rate Control(day)");
}


void CMEDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMEDlg)
	DDX_Control(pDX, IDC_SPIN10, m_m_to_l_gain);
	DDX_Control(pDX, IDC_SPIN8, m_m_max_exp_time);
	DDX_Control(pDX, IDC_SPIN7, m_m_framerate);
	DDX_Control(pDX, IDC_SPIN6, m_l_to_h_gain);
	DDX_Control(pDX, IDC_SPIN5, m_l_max_exp_time);
	DDX_Control(pDX, IDC_SPIN1, m_h_framerate);
	DDX_Control(pDX, IDC_SPIN2, m_h_max_exp_time);
	DDX_Control(pDX, IDC_SPIN3, m_h_to_l_gain);
	DDX_Control(pDX, IDC_SPIN4, m_l_framerate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMEDlg, CDialog)
	//{{AFX_MSG_MAP(CMEDlg)
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMEDlg message handlers

BOOL CMEDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_h_framerate.SetRange(0,255);
	m_h_max_exp_time.SetRange32(0,65535);
	m_h_to_l_gain.SetRange(0,255);
	
	m_l_framerate.SetRange(0,255);
	m_l_max_exp_time.SetRange32(0,65535);
	m_l_to_h_gain.SetRange(0,255);

	m_m_framerate.SetRange(0,255);
	m_m_max_exp_time.SetRange32(0,65535);
	m_m_to_l_gain.SetRange(0,255);

	SetWindowText(m_title);

	SetDataValue();

	return TRUE;
}

BOOL CMEDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}



void CMEDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();

	GetDataValue();
}

void CMEDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void CMEDlg::GetDataValue(void)
{
	AK_ISP_FRAME_RATE_ATTR_EX fr = {0};
	
	UpdateData(TRUE);

	if ((0 != m_m_framerate.GetPos())
		&& (0 != m_m_max_exp_time.GetPos())
		&& (0 != m_m_to_l_gain.GetPos()))
	{
		fr.fps1 = (T_U8)m_h_framerate.GetPos();
		fr.max_exp_time1 = (T_U16)m_h_max_exp_time.GetPos();
		fr.gain12 = (T_U8)m_h_to_l_gain.GetPos();

		fr.fps2 = (T_U8)m_m_framerate.GetPos();
		fr.max_exp_time2 = (T_U16)m_m_max_exp_time.GetPos();
		fr.gain22 = (T_U8)m_m_to_l_gain.GetPos();

		fr.fps3 = (T_U8)m_l_framerate.GetPos();
		fr.max_exp_time3 = (T_U16)m_l_max_exp_time.GetPos();
		fr.gain31 = (T_U8)m_l_to_h_gain.GetPos();

		memcpy(&m_FrameRate, &fr, sizeof(AK_ISP_FRAME_RATE_ATTR));
	}
	else
	{
		m_FrameRate.hight_light_frame_rate = (T_U32)m_h_framerate.GetPos();
		m_FrameRate.hight_light_max_exp_time = (T_U32)m_h_max_exp_time.GetPos();
		m_FrameRate.hight_light_to_low_light_gain = (T_U32)m_h_to_l_gain.GetPos();
		m_FrameRate.low_light_frame_rate = (T_U32)m_l_framerate.GetPos();
		m_FrameRate.low_light_max_exp_time = (T_U32)m_l_max_exp_time.GetPos();
		m_FrameRate.low_light_to_hight_light_gain = (T_U32)m_l_to_h_gain.GetPos();
	}

}

void CMEDlg::SetDataValue(void)
{
	AK_ISP_FRAME_RATE_ATTR_EX fr = {0};

	if (m_FrameRate.hight_light_frame_rate & 0xffffff00)
	{
		memcpy(&fr, &m_FrameRate, sizeof(AK_ISP_FRAME_RATE_ATTR_EX));

		m_h_framerate.SetPos(fr.fps1);
		m_h_max_exp_time.SetPos(fr.max_exp_time1);
		m_h_to_l_gain.SetPos(fr.gain12);

		m_m_framerate.SetPos(fr.fps2);
		m_m_max_exp_time.SetPos(fr.max_exp_time2);
		m_m_to_l_gain.SetPos(fr.gain22);

		m_l_framerate.SetPos(fr.fps3);
		m_l_max_exp_time.SetPos(fr.max_exp_time3);
		m_l_to_h_gain.SetPos(fr.gain31);
		
	}
	else
	{
		m_h_framerate.SetPos(m_FrameRate.hight_light_frame_rate);
		m_h_max_exp_time.SetPos(m_FrameRate.hight_light_max_exp_time);
		m_h_to_l_gain.SetPos(m_FrameRate.hight_light_to_low_light_gain);
		m_l_framerate.SetPos(m_FrameRate.low_light_frame_rate);
		m_l_max_exp_time.SetPos(m_FrameRate.low_light_max_exp_time);
		m_l_to_h_gain.SetPos(m_FrameRate.low_light_to_hight_light_gain);
	}

	UpdateData(FALSE);
}

void CMEDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMEDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMEDlg::OnRadio33() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMEDlg::OnRadio34() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMEDlg::OnRadio35() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CMEDlg::OnRadio36() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}
