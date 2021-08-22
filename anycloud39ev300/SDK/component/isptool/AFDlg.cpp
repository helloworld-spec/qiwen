// AFDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "AFDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IMG_WIDTH_MAX	2048
#define IMG_HEIGHT_MAX	1536


/////////////////////////////////////////////////////////////////////////////
// CAFDlg dialog


CAFDlg::CAFDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAFDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAFDlg)
	//}}AFX_DATA_INIT
}


void CAFDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAFDlg)
	DDX_Control(pDX, IDC_SPIN_TOP, m_top_spin);
	DDX_Control(pDX, IDC_SPIN_TH, m_th_spin);
	DDX_Control(pDX, IDC_SPIN_RIGHT, m_right_spin);
	DDX_Control(pDX, IDC_SPIN_LEFT, m_left_spin);
	DDX_Control(pDX, IDC_SPIN_BOTTOM, m_bottom_spin);
	DDX_Control(pDX, IDC_SLIDER_TOP, m_top_slider);
	DDX_Control(pDX, IDC_SLIDER_TH, m_th_slider);
	DDX_Control(pDX, IDC_SLIDER_LEFT, m_left_slider);
	DDX_Control(pDX, IDC_SLIDER_BOTTOM, m_bottom_slider);
	DDX_Control(pDX, IDC_SLIDER_RIGHT, m_right_slider);

	DDX_Control(pDX, IDC_SPIN_TOP1, m_top1_spin);
	DDX_Control(pDX, IDC_SPIN_RIGHT1, m_right1_spin);
	DDX_Control(pDX, IDC_SPIN_LEFT1, m_left1_spin);
	DDX_Control(pDX, IDC_SPIN_BOTTOM1, m_bottom1_spin);
	DDX_Control(pDX, IDC_SLIDER_TOP1, m_top1_slider);
	DDX_Control(pDX, IDC_SLIDER_LEFT1, m_left1_slider);
	DDX_Control(pDX, IDC_SLIDER_BOTTOM1, m_bottom1_slider);
	DDX_Control(pDX, IDC_SLIDER_RIGHT1, m_right1_slider);

	DDX_Control(pDX, IDC_SPIN_TOP2, m_top2_spin);
	DDX_Control(pDX, IDC_SPIN_RIGHT2, m_right2_spin);
	DDX_Control(pDX, IDC_SPIN_LEFT2, m_left2_spin);
	DDX_Control(pDX, IDC_SPIN_BOTTOM2, m_bottom2_spin);
	DDX_Control(pDX, IDC_SLIDER_TOP2, m_top2_slider);
	DDX_Control(pDX, IDC_SLIDER_LEFT2, m_left2_slider);
	DDX_Control(pDX, IDC_SLIDER_BOTTOM2, m_bottom2_slider);
	DDX_Control(pDX, IDC_SLIDER_RIGHT2, m_right2_slider);

	DDX_Control(pDX, IDC_SPIN_TOP3, m_top3_spin);
	DDX_Control(pDX, IDC_SPIN_RIGHT3, m_right3_spin);
	DDX_Control(pDX, IDC_SPIN_LEFT3, m_left3_spin);
	DDX_Control(pDX, IDC_SPIN_BOTTOM3, m_bottom3_spin);
	DDX_Control(pDX, IDC_SLIDER_TOP3, m_top3_slider);
	DDX_Control(pDX, IDC_SLIDER_LEFT3, m_left3_slider);
	DDX_Control(pDX, IDC_SLIDER_BOTTOM3, m_bottom3_slider);
	DDX_Control(pDX, IDC_SLIDER_RIGHT3, m_right3_slider);

	DDX_Control(pDX, IDC_SPIN_TOP4, m_top4_spin);
	DDX_Control(pDX, IDC_SPIN_RIGHT4, m_right4_spin);
	DDX_Control(pDX, IDC_SPIN_LEFT4, m_left4_spin);
	DDX_Control(pDX, IDC_SPIN_BOTTOM4, m_bottom4_spin);
	DDX_Control(pDX, IDC_SLIDER_TOP4, m_top4_slider);
	DDX_Control(pDX, IDC_SLIDER_LEFT4, m_left4_slider);
	DDX_Control(pDX, IDC_SLIDER_BOTTOM4, m_bottom4_slider);
	DDX_Control(pDX, IDC_SLIDER_RIGHT4, m_right4_slider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAFDlg, CDialog)
	//{{AFX_MSG_MAP(CAFDlg)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_LEFT, OnCustomdrawSliderLeft)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_RIGHT, OnCustomdrawSliderRight)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BOTTOM, OnCustomdrawSliderBottom)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TH, OnCustomdrawSliderTh)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TOP, OnCustomdrawSliderTop)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BOTTOM, OnDeltaposSpinBottom)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LEFT, OnDeltaposSpinLeft)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RIGHT, OnDeltaposSpinRight)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TH, OnDeltaposSpinTh)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TOP, OnDeltaposSpinTop)
	ON_EN_KILLFOCUS(IDC_EDIT_LEFT, OnKillfocusEditLeft)
	ON_EN_KILLFOCUS(IDC_EDIT_RIGHT, OnKillfocusEditRight)
	ON_EN_KILLFOCUS(IDC_EDIT_TH, OnKillfocusEditTh)
	ON_EN_KILLFOCUS(IDC_EDIT_TOP, OnKillfocusEditTop)
	ON_EN_KILLFOCUS(IDC_EDIT_BOTTOM, OnKillfocusEditBottom)

	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_LEFT1, OnCustomdrawSliderLeft1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_RIGHT1, OnCustomdrawSliderRight1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BOTTOM1, OnCustomdrawSliderBottom1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TOP1, OnCustomdrawSliderTop1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BOTTOM1, OnDeltaposSpinBottom1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LEFT1, OnDeltaposSpinLeft1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RIGHT1, OnDeltaposSpinRight1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TOP1, OnDeltaposSpinTop1)
	ON_EN_KILLFOCUS(IDC_EDIT_LEFT1, OnKillfocusEditLeft1)
	ON_EN_KILLFOCUS(IDC_EDIT_RIGHT1, OnKillfocusEditRight1)
	ON_EN_KILLFOCUS(IDC_EDIT_TOP1, OnKillfocusEditTop1)
	ON_EN_KILLFOCUS(IDC_EDIT_BOTTOM1, OnKillfocusEditBottom1)

	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_LEFT2, OnCustomdrawSliderLeft2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_RIGHT2, OnCustomdrawSliderRight2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BOTTOM2, OnCustomdrawSliderBottom2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TOP2, OnCustomdrawSliderTop2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BOTTOM2, OnDeltaposSpinBottom2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LEFT2, OnDeltaposSpinLeft2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RIGHT2, OnDeltaposSpinRight2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TOP2, OnDeltaposSpinTop2)
	ON_EN_KILLFOCUS(IDC_EDIT_LEFT2, OnKillfocusEditLeft2)
	ON_EN_KILLFOCUS(IDC_EDIT_RIGHT2, OnKillfocusEditRight2)
	ON_EN_KILLFOCUS(IDC_EDIT_TOP2, OnKillfocusEditTop2)
	ON_EN_KILLFOCUS(IDC_EDIT_BOTTOM2, OnKillfocusEditBottom2)

	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_LEFT3, OnCustomdrawSliderLeft3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_RIGHT3, OnCustomdrawSliderRight3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BOTTOM3, OnCustomdrawSliderBottom3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TOP3, OnCustomdrawSliderTop3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BOTTOM3, OnDeltaposSpinBottom3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LEFT3, OnDeltaposSpinLeft3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RIGHT3, OnDeltaposSpinRight3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TOP3, OnDeltaposSpinTop3)
	ON_EN_KILLFOCUS(IDC_EDIT_LEFT3, OnKillfocusEditLeft3)
	ON_EN_KILLFOCUS(IDC_EDIT_RIGHT3, OnKillfocusEditRight3)
	ON_EN_KILLFOCUS(IDC_EDIT_TOP3, OnKillfocusEditTop3)
	ON_EN_KILLFOCUS(IDC_EDIT_BOTTOM3, OnKillfocusEditBottom3)

	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_LEFT4, OnCustomdrawSliderLeft4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_RIGHT4, OnCustomdrawSliderRight4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BOTTOM, OnCustomdrawSliderBottom4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_TOP4, OnCustomdrawSliderTop4)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_BOTTOM4, OnDeltaposSpinBottom4)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LEFT4, OnDeltaposSpinLeft4)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RIGHT4, OnDeltaposSpinRight4)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TOP4, OnDeltaposSpinTop4)
	ON_EN_KILLFOCUS(IDC_EDIT_LEFT4, OnKillfocusEditLeft4)
	ON_EN_KILLFOCUS(IDC_EDIT_RIGHT4, OnKillfocusEditRight4)
	ON_EN_KILLFOCUS(IDC_EDIT_TOP4, OnKillfocusEditTop4)
	ON_EN_KILLFOCUS(IDC_EDIT_BOTTOM4, OnKillfocusEditBottom4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAFDlg message handlers

void CAFDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_AF, 0);
}

void CAFDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_AF, 0);
}

void CAFDlg::OnButtonSave() 
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
	
	file.Write(&m_Isp_af, sizeof(AK_ISP_INIT_AF));
	
	file.Close();
}

void CAFDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_AF))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}
	
	file.Read(&m_Isp_af, sizeof(AK_ISP_INIT_AF));
	file.Close();
	
	SetDataValue();
}

BOOL CAFDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	ZeroMemory(&m_Isp_af, sizeof(AK_ISP_INIT_AF));

	// TODO: Add extra initialization here
	m_left_spin.SetRange(0,IMG_WIDTH_MAX - 1);
	m_left_spin.SetPos(0);
	m_left_slider.SetRange(0,IMG_WIDTH_MAX - 1);
	m_left_slider.SetPos(0);
	
	m_right_spin.SetRange(0,IMG_WIDTH_MAX - 1);
	m_right_spin.SetPos(0);
	m_right_slider.SetRange(0,IMG_WIDTH_MAX - 1);
	m_right_slider.SetPos(0);
	
	m_top_spin.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_top_spin.SetPos(0);
	m_top_slider.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_top_slider.SetPos(0);
	
	m_bottom_spin.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_bottom_spin.SetPos(0);
	m_bottom_slider.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_bottom_slider.SetPos(0);

	m_left1_spin.SetRange(0,IMG_WIDTH_MAX - 1);
	m_left1_spin.SetPos(0);
	m_left1_slider.SetRange(0,IMG_WIDTH_MAX - 1);
	m_left1_slider.SetPos(0);
	
	m_right1_spin.SetRange(0,IMG_WIDTH_MAX - 1);
	m_right1_spin.SetPos(0);
	m_right1_slider.SetRange(0,IMG_WIDTH_MAX - 1);
	m_right1_slider.SetPos(0);
	
	m_top1_spin.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_top1_spin.SetPos(0);
	m_top1_slider.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_top1_slider.SetPos(0);
	
	m_bottom1_spin.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_bottom1_spin.SetPos(0);
	m_bottom1_slider.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_bottom1_slider.SetPos(0);

	m_left2_spin.SetRange(0,IMG_WIDTH_MAX - 1);
	m_left2_spin.SetPos(0);
	m_left2_slider.SetRange(0,IMG_WIDTH_MAX - 1);
	m_left2_slider.SetPos(0);
	
	m_right2_spin.SetRange(0,IMG_WIDTH_MAX - 1);
	m_right2_spin.SetPos(0);
	m_right2_slider.SetRange(0,IMG_WIDTH_MAX - 1);
	m_right2_slider.SetPos(0);
	
	m_top2_spin.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_top2_spin.SetPos(0);
	m_top2_slider.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_top2_slider.SetPos(0);
	
	m_bottom2_spin.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_bottom2_spin.SetPos(0);
	m_bottom2_slider.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_bottom2_slider.SetPos(0);

	m_left3_spin.SetRange(0,IMG_WIDTH_MAX - 1);
	m_left3_spin.SetPos(0);
	m_left3_slider.SetRange(0,IMG_WIDTH_MAX - 1);
	m_left3_slider.SetPos(0);
	
	m_right3_spin.SetRange(0,IMG_WIDTH_MAX - 1);
	m_right3_spin.SetPos(0);
	m_right3_slider.SetRange(0,IMG_WIDTH_MAX - 1);
	m_right3_slider.SetPos(0);
	
	m_top3_spin.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_top3_spin.SetPos(0);
	m_top3_slider.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_top3_slider.SetPos(0);
	
	m_bottom3_spin.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_bottom3_spin.SetPos(0);
	m_bottom3_slider.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_bottom3_slider.SetPos(0);

	m_left4_spin.SetRange(0,IMG_WIDTH_MAX - 1);
	m_left4_spin.SetPos(0);
	m_left4_slider.SetRange(0,IMG_WIDTH_MAX - 1);
	m_left4_slider.SetPos(0);
	
	m_right4_spin.SetRange(0,IMG_WIDTH_MAX - 1);
	m_right4_spin.SetPos(0);
	m_right4_slider.SetRange(0,IMG_WIDTH_MAX - 1);
	m_right4_slider.SetPos(0);
	
	m_top4_spin.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_top4_spin.SetPos(0);
	m_top4_slider.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_top4_slider.SetPos(0);
	
	m_bottom4_spin.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_bottom4_spin.SetPos(0);
	m_bottom4_slider.SetRange(0,IMG_HEIGHT_MAX - 1);
	m_bottom4_slider.SetPos(0);
	
	m_th_spin.SetRange(0,128);
	m_th_spin.SetPos(0);
	m_th_slider.SetRange(0,128);
	m_th_slider.SetPos(0);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CAFDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CAFDlg::OnCustomdrawSliderLeft(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_left_spin.SetPos(m_left_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderRight(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_right_spin.SetPos(m_right_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderBottom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_bottom_spin.SetPos(m_bottom_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderTh(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th_spin.SetPos(m_th_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderTop(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_top_spin.SetPos(m_top_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderLeft1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_left1_spin.SetPos(m_left1_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderRight1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_right1_spin.SetPos(m_right1_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderBottom1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_bottom1_spin.SetPos(m_bottom1_slider.GetPos());
	*pResult = 0;
}


void CAFDlg::OnCustomdrawSliderTop1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_top1_spin.SetPos(m_top1_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderLeft2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_left2_spin.SetPos(m_left2_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderRight2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_right2_spin.SetPos(m_right2_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderBottom2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_bottom2_spin.SetPos(m_bottom2_slider.GetPos());
	*pResult = 0;
}


void CAFDlg::OnCustomdrawSliderTop2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_top2_spin.SetPos(m_top2_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderLeft3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_left3_spin.SetPos(m_left3_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderRight3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_right3_spin.SetPos(m_right3_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderBottom3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_bottom3_spin.SetPos(m_bottom3_slider.GetPos());
	*pResult = 0;
}


void CAFDlg::OnCustomdrawSliderTop3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_top3_spin.SetPos(m_top3_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderLeft4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_left4_spin.SetPos(m_left4_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderRight4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_right4_spin.SetPos(m_right4_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnCustomdrawSliderBottom4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_bottom4_spin.SetPos(m_bottom4_slider.GetPos());
	*pResult = 0;
}


void CAFDlg::OnCustomdrawSliderTop4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_top4_spin.SetPos(m_top4_slider.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinBottom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_bottom_slider.SetPos(m_bottom_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinLeft(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	m_left_slider.SetPos(m_left_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinRight(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_right_slider.SetPos(m_right_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinTh(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th_slider.SetPos(m_th_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinTop(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_top_slider.SetPos(m_top_spin.GetPos());
	*pResult = 0;
}


void CAFDlg::OnDeltaposSpinBottom1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_bottom1_slider.SetPos(m_bottom1_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinLeft1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	m_left1_slider.SetPos(m_left1_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinRight1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_right1_slider.SetPos(m_right1_spin.GetPos());
	*pResult = 0;
}


void CAFDlg::OnDeltaposSpinTop1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_top1_slider.SetPos(m_top1_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinBottom2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_bottom2_slider.SetPos(m_bottom2_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinLeft2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	m_left2_slider.SetPos(m_left2_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinRight2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_right2_slider.SetPos(m_right2_spin.GetPos());
	*pResult = 0;
}


void CAFDlg::OnDeltaposSpinTop2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_top2_slider.SetPos(m_top2_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinBottom3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_bottom3_slider.SetPos(m_bottom3_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinLeft3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	m_left3_slider.SetPos(m_left3_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinRight3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_right3_slider.SetPos(m_right3_spin.GetPos());
	*pResult = 0;
}


void CAFDlg::OnDeltaposSpinTop3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_top3_slider.SetPos(m_top3_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinBottom4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_bottom4_slider.SetPos(m_bottom4_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinLeft4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	m_left_slider.SetPos(m_left_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnDeltaposSpinRight4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_right4_slider.SetPos(m_right4_spin.GetPos());
	*pResult = 0;
}


void CAFDlg::OnDeltaposSpinTop4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_top4_slider.SetPos(m_top4_spin.GetPos());
	*pResult = 0;
}

void CAFDlg::OnKillfocusEditLeft() 
{
	// TODO: Add your control notification handler code here
	m_left_slider.SetPos(m_left_spin.GetPos());
}

void CAFDlg::OnKillfocusEditRight() 
{
	// TODO: Add your control notification handler code here
	m_right_slider.SetPos(m_right_spin.GetPos());
}

void CAFDlg::OnKillfocusEditTh() 
{
	// TODO: Add your control notification handler code here
	m_th_slider.SetPos(m_th_spin.GetPos());
}

void CAFDlg::OnKillfocusEditTop() 
{
	// TODO: Add your control notification handler code here
	m_top_slider.SetPos(m_top_spin.GetPos());
}


void CAFDlg::OnKillfocusEditBottom() 
{
	// TODO: Add your control notification handler code here
	m_bottom_slider.SetPos(m_bottom_spin.GetPos());
}

void CAFDlg::OnKillfocusEditLeft1() 
{
	// TODO: Add your control notification handler code here
	m_left1_slider.SetPos(m_left1_spin.GetPos());
}

void CAFDlg::OnKillfocusEditRight1() 
{
	// TODO: Add your control notification handler code here
	m_right1_slider.SetPos(m_right1_spin.GetPos());
}


void CAFDlg::OnKillfocusEditTop1() 
{
	// TODO: Add your control notification handler code here
	m_top1_slider.SetPos(m_top1_spin.GetPos());
}

void CAFDlg::OnKillfocusEditBottom1() 
{
	// TODO: Add your control notification handler code here
	m_bottom1_slider.SetPos(m_bottom1_spin.GetPos());
}



void CAFDlg::OnKillfocusEditLeft2() 
{
	// TODO: Add your control notification handler code here
	m_left2_slider.SetPos(m_left2_spin.GetPos());
}

void CAFDlg::OnKillfocusEditRight2() 
{
	// TODO: Add your control notification handler code here
	m_right2_slider.SetPos(m_right2_spin.GetPos());
}


void CAFDlg::OnKillfocusEditTop2() 
{
	// TODO: Add your control notification handler code here
	m_top2_slider.SetPos(m_top2_spin.GetPos());
}

void CAFDlg::OnKillfocusEditBottom2() 
{
	// TODO: Add your control notification handler code here
	m_bottom2_slider.SetPos(m_bottom2_spin.GetPos());
}


void CAFDlg::OnKillfocusEditLeft3() 
{
	// TODO: Add your control notification handler code here
	m_left3_slider.SetPos(m_left3_spin.GetPos());
}

void CAFDlg::OnKillfocusEditRight3() 
{
	// TODO: Add your control notification handler code here
	m_right3_slider.SetPos(m_right3_spin.GetPos());
}


void CAFDlg::OnKillfocusEditTop3() 
{
	// TODO: Add your control notification handler code here
	m_top3_slider.SetPos(m_top3_spin.GetPos());
}

void CAFDlg::OnKillfocusEditBottom3() 
{
	// TODO: Add your control notification handler code here
	m_bottom3_slider.SetPos(m_bottom3_spin.GetPos());
}


void CAFDlg::OnKillfocusEditLeft4() 
{
	// TODO: Add your control notification handler code here
	m_left4_slider.SetPos(m_left4_spin.GetPos());
}

void CAFDlg::OnKillfocusEditRight4() 
{
	// TODO: Add your control notification handler code here
	m_right4_slider.SetPos(m_right4_spin.GetPos());
}


void CAFDlg::OnKillfocusEditTop4() 
{
	// TODO: Add your control notification handler code here
	m_top4_slider.SetPos(m_top4_spin.GetPos());
}

void CAFDlg::OnKillfocusEditBottom4() 
{
	// TODO: Add your control notification handler code here
	m_bottom4_slider.SetPos(m_bottom4_spin.GetPos());
}

void CAFDlg::GetDataValue(void)
{
	UpdateData(TRUE);
	
	m_Isp_af.param_id = ISP_AF;
	m_Isp_af.length = sizeof(AK_ISP_INIT_AF);
	
	m_Isp_af.p_af_attr.af_win0_left = (T_U16)m_left_spin.GetPos();
	m_Isp_af.p_af_attr.af_win0_right  = (T_U16)m_right_spin.GetPos();
	m_Isp_af.p_af_attr.af_win0_top  = (T_U16)m_top_spin.GetPos();
	m_Isp_af.p_af_attr.af_win0_bottom  = (T_U16)m_bottom_spin.GetPos();

	m_Isp_af.p_af_attr.af_win1_left = (T_U16)m_left1_spin.GetPos();
	m_Isp_af.p_af_attr.af_win1_right  = (T_U16)m_right1_spin.GetPos();
	m_Isp_af.p_af_attr.af_win1_top  = (T_U16)m_top1_spin.GetPos();
	m_Isp_af.p_af_attr.af_win1_bottom  = (T_U16)m_bottom1_spin.GetPos();

	m_Isp_af.p_af_attr.af_win2_left = (T_U16)m_left2_spin.GetPos();
	m_Isp_af.p_af_attr.af_win2_right  = (T_U16)m_right2_spin.GetPos();
	m_Isp_af.p_af_attr.af_win2_top  = (T_U16)m_top2_spin.GetPos();
	m_Isp_af.p_af_attr.af_win2_bottom  = (T_U16)m_bottom2_spin.GetPos();

	m_Isp_af.p_af_attr.af_win3_left = (T_U16)m_left3_spin.GetPos();
	m_Isp_af.p_af_attr.af_win3_right  = (T_U16)m_right3_spin.GetPos();
	m_Isp_af.p_af_attr.af_win3_top  = (T_U16)m_top3_spin.GetPos();
	m_Isp_af.p_af_attr.af_win3_bottom  = (T_U16)m_bottom3_spin.GetPos();

	m_Isp_af.p_af_attr.af_win4_left = (T_U16)m_left4_spin.GetPos();
	m_Isp_af.p_af_attr.af_win4_right  = (T_U16)m_right4_spin.GetPos();
	m_Isp_af.p_af_attr.af_win4_top  = (T_U16)m_top4_spin.GetPos();
	m_Isp_af.p_af_attr.af_win4_bottom  = (T_U16)m_bottom4_spin.GetPos();
	
	m_Isp_af.p_af_attr.af_th  = (T_U16)m_th_spin.GetPos();
}

void CAFDlg::SetDataValue(void)
{
	m_left_spin.SetPos(m_Isp_af.p_af_attr.af_win0_left);
	m_left_slider.SetPos(m_Isp_af.p_af_attr.af_win0_left);
	m_right_spin.SetPos(m_Isp_af.p_af_attr.af_win0_right);
	m_right_slider.SetPos(m_Isp_af.p_af_attr.af_win0_right);
	m_top_spin.SetPos(m_Isp_af.p_af_attr.af_win0_top);
	m_top_slider.SetPos(m_Isp_af.p_af_attr.af_win0_top);
	m_bottom_spin.SetPos(m_Isp_af.p_af_attr.af_win0_bottom);
	m_bottom_slider.SetPos(m_Isp_af.p_af_attr.af_win0_bottom);

	m_left1_spin.SetPos(m_Isp_af.p_af_attr.af_win1_left);
	m_left1_slider.SetPos(m_Isp_af.p_af_attr.af_win1_left);
	m_right1_spin.SetPos(m_Isp_af.p_af_attr.af_win1_right);
	m_right1_slider.SetPos(m_Isp_af.p_af_attr.af_win1_right);
	m_top1_spin.SetPos(m_Isp_af.p_af_attr.af_win1_top);
	m_top1_slider.SetPos(m_Isp_af.p_af_attr.af_win1_top);
	m_bottom1_spin.SetPos(m_Isp_af.p_af_attr.af_win1_bottom);
	m_bottom1_slider.SetPos(m_Isp_af.p_af_attr.af_win1_bottom);

	m_left2_spin.SetPos(m_Isp_af.p_af_attr.af_win2_left);
	m_left2_slider.SetPos(m_Isp_af.p_af_attr.af_win2_left);
	m_right2_spin.SetPos(m_Isp_af.p_af_attr.af_win2_right);
	m_right2_slider.SetPos(m_Isp_af.p_af_attr.af_win2_right);
	m_top2_spin.SetPos(m_Isp_af.p_af_attr.af_win2_top);
	m_top2_slider.SetPos(m_Isp_af.p_af_attr.af_win2_top);
	m_bottom2_spin.SetPos(m_Isp_af.p_af_attr.af_win2_bottom);
	m_bottom2_slider.SetPos(m_Isp_af.p_af_attr.af_win2_bottom);

	m_left3_spin.SetPos(m_Isp_af.p_af_attr.af_win3_left);
	m_left3_slider.SetPos(m_Isp_af.p_af_attr.af_win3_left);
	m_right3_spin.SetPos(m_Isp_af.p_af_attr.af_win3_right);
	m_right3_slider.SetPos(m_Isp_af.p_af_attr.af_win3_right);
	m_top3_spin.SetPos(m_Isp_af.p_af_attr.af_win3_top);
	m_top3_slider.SetPos(m_Isp_af.p_af_attr.af_win3_top);
	m_bottom3_spin.SetPos(m_Isp_af.p_af_attr.af_win3_bottom);
	m_bottom3_slider.SetPos(m_Isp_af.p_af_attr.af_win3_bottom);

	m_left4_spin.SetPos(m_Isp_af.p_af_attr.af_win4_left);
	m_left4_slider.SetPos(m_Isp_af.p_af_attr.af_win4_left);
	m_right4_spin.SetPos(m_Isp_af.p_af_attr.af_win4_right);
	m_right4_slider.SetPos(m_Isp_af.p_af_attr.af_win4_right);
	m_top4_spin.SetPos(m_Isp_af.p_af_attr.af_win4_top);
	m_top4_slider.SetPos(m_Isp_af.p_af_attr.af_win4_top);
	m_bottom4_spin.SetPos(m_Isp_af.p_af_attr.af_win4_bottom);
	m_bottom4_slider.SetPos(m_Isp_af.p_af_attr.af_win4_bottom);
	
	m_th_spin.SetPos(m_Isp_af.p_af_attr.af_th);
	m_th_slider.SetPos(m_Isp_af.p_af_attr.af_th);
	
}

int CAFDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_AF))) return -1;
	
	GetDataValue();
	
	memcpy(pPageInfoSt, &m_Isp_af, sizeof(AK_ISP_INIT_AF));
	
	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_AF);
	
	return 0;
}


int CAFDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_AF))) return -1;
	
	memcpy(&m_Isp_af, pPageInfoSt, sizeof(AK_ISP_INIT_AF));
	
	SetDataValue();
	
	return 0;
}



void CAFDlg::Clean(void) 
{
	ZeroMemory(&m_Isp_af, sizeof(AK_ISP_INIT_AF));
	SetDataValue();
	UpdateData(FALSE);
}


int CAFDlg::Convert_v2_data(AK_ISP_INIT_AF *struct_new, AK_ISP_INIT_AF_V2* struct_v2) 
{
	if ((struct_v2 == NULL) || (struct_new == NULL)) return -1;
	
	struct_new->param_id = ISP_AF;
	struct_new->length = sizeof(AK_ISP_INIT_AF);


	struct_new->p_af_attr.af_win0_left = struct_v2->p_af_attr.af_win_left;
	struct_new->p_af_attr.af_win0_right = struct_v2->p_af_attr.af_win_right;
	struct_new->p_af_attr.af_win0_top = struct_v2->p_af_attr.af_win_top;
	struct_new->p_af_attr.af_win0_bottom = struct_v2->p_af_attr.af_win_bottom;
	struct_new->p_af_attr.af_th = struct_v2->p_af_attr.af_th;


	return 0;
}

