// 3DStatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "3DStatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// C3DStatDlg dialog


C3DStatDlg::C3DStatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(C3DStatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(C3DStatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bInit = FALSE;
	m_semaphore = CreateSemaphore(NULL, 1, 1, NULL);
}

C3DStatDlg::~C3DStatDlg()
{
	CloseHandle(m_semaphore);
}


void C3DStatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(C3DStatDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(C3DStatDlg, CDialog)
	//{{AFX_MSG_MAP(C3DStatDlg)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C3DStatDlg message handlers

BOOL C3DStatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_3DStat, sizeof(AK_ISP_3D_NR_STAT_INFO));

	m_timer = SetTimer(1, 1000, NULL);
	
	m_bInit = TRUE;

	return TRUE;
}


void C3DStatDlg::SetDataValue(void)
{
	CWnd *pWnd = NULL;
	CString str;
	
	pWnd = GetDlgItem(IDC_STATIC_MD_STAT_MAX);
	str.Format("%d", m_3DStat.MD_stat_max);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_MD_LEVEL);
	str.Format("%d", m_3DStat.MD_level);
	pWnd->SetWindowText(str);


	for (int j=0; j<12; j++)
	{
		for (int i=0; i<16; i++)
		{
			pWnd = GetDlgItem(IDC_STATIC_3D00 + j*16 + i);
			str.Format("%d", m_3DStat.MD_stat[2*j][2*i]);
			pWnd->SetWindowText(str);
		}
	}
}


int C3DStatDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_3D_NR_STAT_INFO))) return -1;

	WaitForSingleObject(m_semaphore, 0xffffffff);

	if (m_bInit)
	{
		memcpy(&m_3DStat, pPageInfoSt, sizeof(AK_ISP_3D_NR_STAT_INFO));
		SetDataValue();
	}

	ReleaseSemaphore(m_semaphore, 1, NULL);
	
	return 0;
}

void C3DStatDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (1 == nIDEvent)
	{
		if (NULL == m_pMessageWnd) return;
		CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_3DSTAT, 0);
	}

	CDialog::OnTimer(nIDEvent);
}

void C3DStatDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	WaitForSingleObject(m_semaphore, 0xffffffff);
	m_bInit = FALSE;
	KillTimer(m_timer);
	
	CDialog::OnClose();
	DestroyWindow();

	ReleaseSemaphore(m_semaphore, 1, NULL);
}
