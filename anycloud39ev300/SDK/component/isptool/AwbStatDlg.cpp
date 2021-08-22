// AwbStatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "AwbStatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAwbStatDlg dialog


CAwbStatDlg::CAwbStatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAwbStatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAwbStatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bInit = FALSE;
	m_semaphore = CreateSemaphore(NULL, 1, 1, NULL);
}

CAwbStatDlg::~CAwbStatDlg()
{
	CloseHandle(m_semaphore);
}


void CAwbStatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAwbStatDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAwbStatDlg, CDialog)
	//{{AFX_MSG_MAP(CAwbStatDlg)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAwbStatDlg message handlers

BOOL CAwbStatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_AWBStat, sizeof(AK_ISP_AWB_STAT_INFO));

	m_timer = SetTimer(1, 1000, NULL);

	m_bInit = TRUE;

	return TRUE;
}


void CAwbStatDlg::SetDataValue(void)
{
	CWnd *pWnd = NULL;
	CString str;
	
	pWnd = GetDlgItem(IDC_STATIC_R_TOTAL_0);
	str.Format("%d", m_AWBStat.total_R[0]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_TOTAL_0);
	str.Format("%d", m_AWBStat.total_G[0]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_TOTAL_0);
	str.Format("%d", m_AWBStat.total_B[0]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_CNT_0);
	str.Format("%d", m_AWBStat.total_cnt[0]);
	pWnd->SetWindowText(str);


	pWnd = GetDlgItem(IDC_STATIC_R_TOTAL_1);
	str.Format("%d", m_AWBStat.total_R[1]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_TOTAL_1);
	str.Format("%d", m_AWBStat.total_G[1]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_TOTAL_1);
	str.Format("%d", m_AWBStat.total_B[1]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_CNT_1);
	str.Format("%d", m_AWBStat.total_cnt[1]);
	pWnd->SetWindowText(str);


	pWnd = GetDlgItem(IDC_STATIC_R_TOTAL_2);
	str.Format("%d", m_AWBStat.total_R[2]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_TOTAL_2);
	str.Format("%d", m_AWBStat.total_G[2]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_TOTAL_2);
	str.Format("%d", m_AWBStat.total_B[2]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_CNT_2);
	str.Format("%d", m_AWBStat.total_cnt[2]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_R_TOTAL_3);
	str.Format("%d", m_AWBStat.total_R[3]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_TOTAL_3);
	str.Format("%d", m_AWBStat.total_G[3]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_TOTAL_3);
	str.Format("%d", m_AWBStat.total_B[3]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_CNT_3);
	str.Format("%d", m_AWBStat.total_cnt[3]);
	pWnd->SetWindowText(str);


	pWnd = GetDlgItem(IDC_STATIC_R_TOTAL_4);
	str.Format("%d", m_AWBStat.total_R[4]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_TOTAL_4);
	str.Format("%d", m_AWBStat.total_G[4]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_TOTAL_4);
	str.Format("%d", m_AWBStat.total_B[4]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_CNT_4);
	str.Format("%d", m_AWBStat.total_cnt[4]);
	pWnd->SetWindowText(str);


	pWnd = GetDlgItem(IDC_STATIC_R_TOTAL_5);
	str.Format("%d", m_AWBStat.total_R[5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_TOTAL_5);
	str.Format("%d", m_AWBStat.total_G[5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_TOTAL_5);
	str.Format("%d", m_AWBStat.total_B[5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_CNT_5);
	str.Format("%d", m_AWBStat.total_cnt[5]);
	pWnd->SetWindowText(str);


	pWnd = GetDlgItem(IDC_STATIC_R_TOTAL_6);
	str.Format("%d", m_AWBStat.total_R[6]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_TOTAL_6);
	str.Format("%d", m_AWBStat.total_G[6]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_TOTAL_6);
	str.Format("%d", m_AWBStat.total_B[6]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_CNT_6);
	str.Format("%d", m_AWBStat.total_cnt[6]);
	pWnd->SetWindowText(str);



	pWnd = GetDlgItem(IDC_STATIC_R_TOTAL_7);
	str.Format("%d", m_AWBStat.total_R[7]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_TOTAL_7);
	str.Format("%d", m_AWBStat.total_G[7]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_TOTAL_7);
	str.Format("%d", m_AWBStat.total_B[7]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_CNT_7);
	str.Format("%d", m_AWBStat.total_cnt[7]);
	pWnd->SetWindowText(str);



	pWnd = GetDlgItem(IDC_STATIC_R_TOTAL_8);
	str.Format("%d", m_AWBStat.total_R[8]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_TOTAL_8);
	str.Format("%d", m_AWBStat.total_G[8]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_TOTAL_8);
	str.Format("%d", m_AWBStat.total_B[8]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_CNT_8);
	str.Format("%d", m_AWBStat.total_cnt[8]);
	pWnd->SetWindowText(str);


	pWnd = GetDlgItem(IDC_STATIC_R_TOTAL_9);
	str.Format("%d", m_AWBStat.total_R[9]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_TOTAL_9);
	str.Format("%d", m_AWBStat.total_G[9]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_TOTAL_9);
	str.Format("%d", m_AWBStat.total_B[9]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_CNT_9);
	str.Format("%d", m_AWBStat.total_cnt[9]);
	pWnd->SetWindowText(str);



	pWnd = GetDlgItem(IDC_STATIC_R_GAIN);
	str.Format("%d", m_AWBStat.r_gain);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_G_GAIN);
	str.Format("%d", m_AWBStat.g_gain);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_B_GAIN);
	str.Format("%d", m_AWBStat.b_gain);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_CTI);
	str.Format("%d", m_AWBStat.current_colortemp_index);
	pWnd->SetWindowText(str);
}


int CAwbStatDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_AWB_STAT_INFO))) return -1;

	WaitForSingleObject(m_semaphore, 0xffffffff);

	if (m_bInit)
	{
		memcpy(&m_AWBStat, pPageInfoSt, sizeof(AK_ISP_AWB_STAT_INFO));
		SetDataValue();
	}

	ReleaseSemaphore(m_semaphore, 1, NULL);
	
	return 0;
}

void CAwbStatDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (1 == nIDEvent)
	{
		if (NULL == m_pMessageWnd) return;
		CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_AWBSTAT, 0);
	}

	CDialog::OnTimer(nIDEvent);
}

void CAwbStatDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	WaitForSingleObject(m_semaphore, 0xffffffff);
	m_bInit = FALSE;

	KillTimer(m_timer);
	
	CDialog::OnClose();
	DestroyWindow();

	ReleaseSemaphore(m_semaphore, 1, NULL);
}