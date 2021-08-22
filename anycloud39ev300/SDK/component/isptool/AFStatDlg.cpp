// AFStatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "AFStatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAFStatDlg dialog


CAFStatDlg::CAFStatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAFStatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAFStatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bInit = FALSE;
	m_semaphore = CreateSemaphore(NULL, 1, 1, NULL);
}

CAFStatDlg::~CAFStatDlg()
{
	CloseHandle(m_semaphore);
}


void CAFStatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAFStatDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAFStatDlg, CDialog)
	//{{AFX_MSG_MAP(CAFStatDlg)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAFStatDlg message handlers

BOOL CAFStatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_AFStat, sizeof(AK_ISP_AF_STAT_INFO));

	m_timer = SetTimer(1, 1000, NULL);

	m_bInit = TRUE;

	return TRUE;
}


void CAFStatDlg::SetDataValue(void)
{
	CWnd *pWnd = NULL;
	CString str;
	
	pWnd = GetDlgItem(IDC_STATIC_AF_STAT0);
	str.Format("%d", m_AFStat.af_statics[0]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_AF_STAT1);
	str.Format("%d", m_AFStat.af_statics[1]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_AF_STAT2);
	str.Format("%d", m_AFStat.af_statics[2]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_AF_STAT3);
	str.Format("%d", m_AFStat.af_statics[3]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_AF_STAT4);
	str.Format("%d", m_AFStat.af_statics[4]);
	pWnd->SetWindowText(str);

}


int CAFStatDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_AF_STAT_INFO))) return -1;

	WaitForSingleObject(m_semaphore, 0xffffffff);

	if (m_bInit)
	{
		memcpy(&m_AFStat, pPageInfoSt, sizeof(AK_ISP_AF_STAT_INFO));
		SetDataValue();
	}

	ReleaseSemaphore(m_semaphore, 1, NULL);
	
	return 0;
}


void CAFStatDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (1 == nIDEvent)
	{
		if (NULL == m_pMessageWnd) return;
		CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_AFSTAT, 0);
	}

	CDialog::OnTimer(nIDEvent);
}

void CAFStatDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	WaitForSingleObject(m_semaphore, 0xffffffff);
	m_bInit = FALSE;
	KillTimer(m_timer);
	
	CDialog::OnClose();
	DestroyWindow();

	ReleaseSemaphore(m_semaphore, 1, NULL);
}