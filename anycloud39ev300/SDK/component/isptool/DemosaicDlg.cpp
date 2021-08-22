// DemosaicDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "DemosaicDlg.h"
#include "netctrl.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDemosaicDlg dialog


CDemosaicDlg::CDemosaicDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDemosaicDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDemosaicDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDemosaicDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDemosaicDlg)
	DDX_Control(pDX, IDC_SLIDER5, m_hf_th2_slider);
	DDX_Control(pDX, IDC_SPIN5, m_hf_th2);
	DDX_Control(pDX, IDC_SLIDER4, m_hf_th1_slider);
	DDX_Control(pDX, IDC_SPIN4, m_hf_th1);
	DDX_Control(pDX, IDC_SLIDER3, m_bg_th_slider);
	DDX_Control(pDX, IDC_SPIN3, m_bg_th);
	DDX_Control(pDX, IDC_SLIDER2, m_rg_th_slider);
	DDX_Control(pDX, IDC_SPIN2, m_rg_th);
	DDX_Control(pDX, IDC_SLIDER1, m_hv_th_slider);
	DDX_Control(pDX, IDC_SPIN1, m_hv_th);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDemosaicDlg, CDialog)
	//{{AFX_MSG_MAP(CDemosaicDlg)
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDemosaicDlg message handlers
BOOL CDemosaicDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Demo, sizeof(AK_ISP_INIT_DEMO));

	m_hv_th.SetRange(0,255);
	m_hv_th.SetPos(0);
	m_hv_th_slider.SetRange(0,255);
	m_hv_th_slider.SetPos(0);

	m_rg_th.SetRange(0,1023);
	m_rg_th.SetPos(0);
	m_rg_th_slider.SetRange(0,1023);
	m_rg_th_slider.SetPos(0);

	m_bg_th.SetRange(0,1023);
	m_bg_th.SetPos(0);
	m_bg_th_slider.SetRange(0,1023);
	m_bg_th_slider.SetPos(0);

	m_hf_th1.SetRange(0,1023);
	m_hf_th1.SetPos(0);
	m_hf_th1_slider.SetRange(0,1023);
	m_hf_th1_slider.SetPos(0);

	m_hf_th2.SetRange(0,1023);
	m_hf_th2.SetPos(0);
	m_hf_th2_slider.SetRange(0,1023);
	m_hf_th2_slider.SetPos(0);


	return TRUE;
}

BOOL CDemosaicDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CDemosaicDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_DEMO))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Demo, sizeof(AK_ISP_INIT_DEMO));
	file.Close();

	SetDataValue();
}

void CDemosaicDlg::OnButtonWrite() 
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

	file.Write(&m_Demo, sizeof(AK_ISP_INIT_DEMO));

	file.Close();
}

void CDemosaicDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_DEMOSAIC, 0);
}

void CDemosaicDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_DEMOSAIC, 0);
}

void CDemosaicDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_hv_th_slider.SetPos(m_hv_th.GetPos());
	*pResult = 0;
}

void CDemosaicDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_hv_th_slider.SetPos(m_hv_th.GetPos());
}

void CDemosaicDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_hv_th.SetPos(m_hv_th_slider.GetPos());
	*pResult = 0;
}

void CDemosaicDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_rg_th_slider.SetPos(m_rg_th.GetPos());
	*pResult = 0;
}

void CDemosaicDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_rg_th_slider.SetPos(m_rg_th.GetPos());
}

void CDemosaicDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_rg_th.SetPos(m_rg_th_slider.GetPos());
	*pResult = 0;
}

void CDemosaicDlg::OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_bg_th_slider.SetPos(m_bg_th.GetPos());
	*pResult = 0;
}

void CDemosaicDlg::OnKillfocusEdit3() 
{
	// TODO: Add your control notification handler code here
	m_bg_th_slider.SetPos(m_bg_th.GetPos());
}

void CDemosaicDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_bg_th.SetPos(m_bg_th_slider.GetPos());
	*pResult = 0;
}

void CDemosaicDlg::OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_hf_th1_slider.SetPos(m_hf_th1.GetPos());
	*pResult = 0;
}

void CDemosaicDlg::OnKillfocusEdit4() 
{
	// TODO: Add your control notification handler code here
	m_hf_th1_slider.SetPos(m_hf_th1.GetPos());
}

void CDemosaicDlg::OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_hf_th1.SetPos(m_hf_th1_slider.GetPos());
	*pResult = 0;
}

void CDemosaicDlg::OnDeltaposSpin5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_hf_th2_slider.SetPos(m_hf_th2.GetPos());
	*pResult = 0;
}

void CDemosaicDlg::OnKillfocusEdit5() 
{
	// TODO: Add your control notification handler code here
	m_hf_th2_slider.SetPos(m_hf_th2.GetPos());
}

void CDemosaicDlg::OnCustomdrawSlider5(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_hf_th2.SetPos(m_hf_th2_slider.GetPos());
	*pResult = 0;
}


void CDemosaicDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	m_Demo.param_id = ISP_DEMO;
	m_Demo.length = sizeof(AK_ISP_INIT_DEMO);

	m_Demo.p_demo_attr.dm_HV_th = (T_U16)m_hv_th.GetPos();
	m_Demo.p_demo_attr.dm_rg_thre = (T_U16)m_rg_th.GetPos();
	m_Demo.p_demo_attr.dm_bg_thre = (T_U16)m_bg_th.GetPos();
	m_Demo.p_demo_attr.dm_hf_th1 = (T_U16)m_hf_th1.GetPos();
	m_Demo.p_demo_attr.dm_hf_th2 = (T_U16)m_hf_th2.GetPos();
	m_Demo.p_demo_attr.dm_rg_gain = 64;
	m_Demo.p_demo_attr.dm_bg_gain = 64;
	m_Demo.p_demo_attr.dm_gr_gain = 64;
	m_Demo.p_demo_attr.dm_gb_gain = 64;
}

void CDemosaicDlg::SetDataValue(void)
{
	m_hv_th.SetPos(m_Demo.p_demo_attr.dm_HV_th);
	m_hv_th_slider.SetPos(m_Demo.p_demo_attr.dm_HV_th);
	m_rg_th.SetPos(m_Demo.p_demo_attr.dm_rg_thre);
	m_rg_th_slider.SetPos(m_Demo.p_demo_attr.dm_rg_thre);
	m_bg_th.SetPos(m_Demo.p_demo_attr.dm_bg_thre);
	m_bg_th_slider.SetPos(m_Demo.p_demo_attr.dm_bg_thre);
	m_hf_th1.SetPos(m_Demo.p_demo_attr.dm_hf_th1);
	m_hf_th1_slider.SetPos(m_Demo.p_demo_attr.dm_hf_th1);
	m_hf_th2.SetPos(m_Demo.p_demo_attr.dm_hf_th2);
	m_hf_th2_slider.SetPos(m_Demo.p_demo_attr.dm_hf_th2);
}

int CDemosaicDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_DEMO))) return -1;

	GetDataValue();

	memcpy(pPageInfoSt, &m_Demo, sizeof(AK_ISP_INIT_DEMO));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_DEMO);

	return 0;
}


int CDemosaicDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_DEMO))) return -1;

	memcpy(&m_Demo, pPageInfoSt, sizeof(AK_ISP_INIT_DEMO));

	SetDataValue();
	
	return 0;
}


void CDemosaicDlg::Clean(void) 
{
	ZeroMemory(&m_Demo, sizeof(AK_ISP_INIT_DEMO));
	SetDataValue();
	UpdateData(FALSE);
}