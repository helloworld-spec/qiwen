// AEStatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "AEStatDlg.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAEStatDlg dialog

#define MIN_GRAY_WINDOW_WIDTH	260
#define MIN_GRAY_WINDOW_HEIGHT	236
#define GRAY_IMAGE_WIDTH		256
#define GRAY_IMAGE_HEIGHT		180
#define GRAY_IMAGE_BAR_HEIGHT	10
#define WIDGET_APART_WIDTH		2
#define WIDGET_APART_HEIGHT		2

CAEStatDlg::CAEStatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAEStatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAEStatDlg)
	m_hist_id = 0;
	m_hist_mode = 0;
	//}}AFX_DATA_INIT
	m_bInit = FALSE;
	m_semaphore = CreateSemaphore(NULL, 1, 1, NULL);
}

CAEStatDlg::~CAEStatDlg()
{
	CloseHandle(m_semaphore);
}


void CAEStatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAEStatDlg)
	DDX_Radio(pDX, IDC_RADIO1, m_hist_id);
	DDX_Radio(pDX, IDC_RADIO4, m_hist_mode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAEStatDlg, CDialog)
	//{{AFX_MSG_MAP(CAEStatDlg)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAEStatDlg message handlers

BOOL CAEStatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_AEStat, sizeof(AK_ISP_AE_STAT_INFO));

	CDC* pDC = GetDC();
	m_MemDC.CreateCompatibleDC(pDC);
	m_MemBitmap.CreateCompatibleBitmap(pDC, 500, 500);
	m_pOldMemBitmap = m_MemDC.SelectObject(&m_MemBitmap);
	ReleaseDC(pDC);

	CRect rect;

	GetClientRect(&rect);

	m_HistWindowRect.left = rect.left + 50;
	m_HistWindowRect.right = rect.right - 150;
	m_HistWindowRect.top = rect.top + 280;
	m_HistWindowRect.bottom = rect.bottom - 50;
	

	int nWidthMid = m_HistWindowRect.left + m_HistWindowRect.Width() / 2;

	m_HistRect.left = nWidthMid - GRAY_IMAGE_WIDTH / 2;
	m_HistRect.right = nWidthMid + GRAY_IMAGE_WIDTH / 2;
	m_HistRect.top = m_HistWindowRect.top + 3 * WIDGET_APART_HEIGHT;
	m_HistRect.bottom = m_HistRect.top + GRAY_IMAGE_HEIGHT;

	m_HistBarRect.left = m_HistRect.left;
	m_HistBarRect.right = m_HistRect.right;
	m_HistBarRect.top = m_HistRect.bottom + 3 * WIDGET_APART_HEIGHT;
	m_HistBarRect.bottom = m_HistBarRect.top + GRAY_IMAGE_BAR_HEIGHT;

	Invalidate(); 	

	m_timer = SetTimer(1, 1000, NULL);

	m_bInit = TRUE;

	if (NULL != m_pMessageWnd)
		CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_EXP_INFO, DIALOG_STAT, 0);


	m_Exp.p_ae.envi_gain_range[0][0] = 0;

	return TRUE;
}

void CAEStatDlg::OnPaint()
{
	DrawHistoGram();
}

void CAEStatDlg::SetDataValue(void)
{
	CWnd *pWnd = NULL;
	CString str;
	unsigned int all_gain = 0;
	int i = 0;
	unsigned int ev = 0;

	pWnd = GetDlgItem(IDC_STATIC_AVG_LUMI);
	str.Format("%d", m_AEStat.run_info.current_calc_avg_lumi);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_COM_LUMI);
	str.Format("%d", m_AEStat.run_info.current_calc_avg_compensation_lumi);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_A_GAIN);
	str.Format("%d", m_AEStat.run_info.current_a_gain);
	pWnd->SetWindowText(str);
	
	pWnd = GetDlgItem(IDC_STATIC_D_GAIN);
	str.Format("%d", m_AEStat.run_info.current_d_gain);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_ISP_D_GAIN);
	str.Format("%d", m_AEStat.run_info.current_isp_d_gain);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_DARK_DAY_FLAG);
	str.Format("%d", m_AEStat.run_info.current_darked_flag);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_A_GAIN_STEP);
	str.Format("%d", m_AEStat.run_info.current_a_gain_step);
	pWnd->SetWindowText(str);
	
	pWnd = GetDlgItem(IDC_STATIC_D_GAIN_STEP);
	str.Format("%d", m_AEStat.run_info.current_d_gain_step);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_ISP_D_GAIN_STEP);
	str.Format("%d", m_AEStat.run_info.current_isp_d_gain_step);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_EXP_TIME);
	str.Format("%d", m_AEStat.run_info.current_exp_time);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_EXP_STEP);
	str.Format("%d", m_AEStat.run_info.current_exp_time_step);
	pWnd->SetWindowText(str);

	ev = ((m_AEStat.run_info.current_exp_time * m_AEStat.run_info.current_isp_d_gain) / 256
				* m_AEStat.run_info.current_a_gain) / 256 
				* m_AEStat.run_info.current_d_gain / 256;

	pWnd = GetDlgItem(IDC_STATIC_EV);
	str.Format("%d", ev);
	pWnd->SetWindowText(str);
	
#ifdef JG_PLATFORM
	if (2 == m_AEStat.scene_mode)
	{
		pWnd = GetDlgItem(IDC_STATIC_SCENE);
		str.Format("DayIndoor");
		pWnd->SetWindowText(str);
	}
	else if (1 == m_AEStat.scene_mode)
	{
		pWnd = GetDlgItem(IDC_STATIC_SCENE);
		str.Format("Night");
		pWnd->SetWindowText(str);
	}
	else if (0 == m_AEStat.scene_mode)
	{
		pWnd = GetDlgItem(IDC_STATIC_SCENE);
		str.Format("DayOutdoor");
		pWnd->SetWindowText(str);
	}
	else
	{
		pWnd = GetDlgItem(IDC_STATIC_SCENE);
		str.Format("Unknown");
		pWnd->SetWindowText(str);
	}
#endif

	if ((256 == m_AEStat.run_info.current_a_gain)
		&& (256 == m_AEStat.run_info.current_d_gain)
		&& (256 == m_AEStat.run_info.current_isp_d_gain))
	{
		pWnd = GetDlgItem(IDC_STATIC_ENV);
		str.Format("1");
		pWnd->SetWindowText(str);
	}
	else
	{
		all_gain = (((m_AEStat.run_info.current_a_gain * m_AEStat.run_info.current_d_gain) >> 8) 
			* m_AEStat.run_info.current_isp_d_gain) >> 8 >> 8;

		for (i=0; i<10; i++)
		{
			if ((all_gain >= m_Exp.p_ae.envi_gain_range[i][0])
				&& (all_gain < m_Exp.p_ae.envi_gain_range[i][1]))
			{
				pWnd = GetDlgItem(IDC_STATIC_ENV);
				str.Format("%d", i+1);
				pWnd->SetWindowText(str);
				break;
			}
		}

		if (10 == i)
		{
			pWnd = GetDlgItem(IDC_STATIC_ENV);
			str.Format("Unknown");
			pWnd->SetWindowText(str);
		}
	}

	InvalidateRect(m_HistRect, FALSE);
}

void CAEStatDlg::DrawHistoGram(void)
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	//draw frame
	dc.Draw3dRect(m_HistRect.left - 1, m_HistRect.top - 1, 
		m_HistRect.Width() + 2, m_HistRect.Height() + 2, RGB(0, 0, 0), RGB(0, 0, 0));
	//dc.Draw3dRect(m_HistBarRect.left - 1, m_HistBarRect.top - 1, 
		//m_HistBarRect.Width() + 2, m_HistBarRect.Height() + 2, RGB(0, 0, 0), RGB(0, 0, 0));

	m_MemDC.FillSolidRect(0, 0, m_HistRect.Width(), m_HistRect.Height(), GetSysColor(COLOR_3DFACE));
	int oldBkMode = m_MemDC.SetBkMode(TRANSPARENT);

	//draw hist
	Graphics graphics(m_MemDC);
	Pen gdiPen(Color(64, 64, 64));
	
	float yratio = 0.0;
	int ybase = m_HistRect.Height();
	T_U32 *pHist = NULL;

	if (0 == m_hist_id)
	{
		pHist = m_AEStat.raw_hist.raw_g_hist;
	}
	else if (1 == m_hist_id)
	{
		pHist = m_AEStat.rgb_hist.rgb_hist;
	}
	else
	{
		pHist = m_AEStat.yuv_hist.y_hist;
	}

	UINT YMaxNum = 0;
	for (int i = 0; i < 256; ++i) {
		if (YMaxNum < pHist[i]) YMaxNum = pHist[i];
	}

	if (1 == m_hist_mode) {
		yratio = m_HistRect.Height() / (float)log10((float)(YMaxNum + 1));
	}else {
		yratio = m_HistRect.Height() / (float)(YMaxNum + 1);
	}

	for (i = 0; i < 256; ++i) {
		if (1 == m_hist_mode) {
			graphics.DrawLine(&gdiPen, Point(i, m_HistRect.Height()), 
								Point(i, (int)(ybase - log10((float)(1 + pHist[i])) * yratio)));
		}else {
			graphics.DrawLine(&gdiPen, Point(i, m_HistRect.Height()), 
								Point(i, (int)(ybase - pHist[i] * yratio)));
		}
	}
	
	dc.BitBlt(m_HistRect.left, m_HistRect.top, m_HistRect.Width(), m_HistRect.Height(), &m_MemDC, 0, 0, SRCCOPY);
	
}


int CAEStatDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_AE_STAT_INFO))) return -1;

	WaitForSingleObject(m_semaphore, 0xffffffff);

	if (m_bInit)
	{
		memcpy(&m_AEStat, pPageInfoSt, sizeof(AK_ISP_AE_STAT_INFO));
		SetDataValue();
	}

	ReleaseSemaphore(m_semaphore, 1, NULL);

	
	return 0;
}

void CAEStatDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	InvalidateRect(m_HistRect, FALSE);
}

void CAEStatDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	InvalidateRect(m_HistRect, FALSE);
}

void CAEStatDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	InvalidateRect(m_HistRect, FALSE);
}

void CAEStatDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	InvalidateRect(m_HistRect, FALSE);
}

void CAEStatDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	InvalidateRect(m_HistRect, FALSE);
}

void CAEStatDlg::OnClose() 
{
	WaitForSingleObject(m_semaphore, 0xffffffff);
	m_bInit = FALSE;

	if (m_MemDC.m_hDC) {
		m_MemDC.SelectObject(m_pOldMemBitmap);
		m_MemDC.DeleteDC();
		m_MemBitmap.DeleteObject();
	}

	m_GrayBarBitmap.DeleteObject();
	KillTimer(m_timer);
	
	CDialog::OnClose();
	DestroyWindow();

	ReleaseSemaphore(m_semaphore, 1, NULL);
}

void CAEStatDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (1 == nIDEvent)
	{
		if (NULL == m_pMessageWnd) return;
		CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_AESTAT, 0);
	}

	CDialog::OnTimer(nIDEvent);
}