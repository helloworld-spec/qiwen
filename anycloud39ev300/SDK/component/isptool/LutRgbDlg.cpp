// LutRgbDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "LutRgbDlg.h"
#include "NetCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLutRgbDlg dialog


CLutRgbDlg::CLutRgbDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLutRgbDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLutRgbDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLutRgbDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLutRgbDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLutRgbDlg, CDialog)
	//{{AFX_MSG_MAP(CLutRgbDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLutRgbDlg message handlers
BOOL CLutRgbDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect clientRect, rect;
	
	this->GetClientRect(&clientRect); 

	m_RDlg.SetMessageWindow(m_pMessageWnd);
	m_RDlg.Create(IDD_DIALOG_CURVE, this);
	m_RDlg.GetClientRect(&rect); 
	m_RDlg.SetEnable(m_bGammaEnable);
	
	rect.left = clientRect.left + 10;
	rect.top = clientRect.top + 10;
	rect.right += 10;
	rect.bottom += 10;

	m_RDlg.MoveWindow(&rect);
	m_RDlg.ShowWindow(SW_SHOW);

	CWnd * pCurveName = m_RDlg.GetDlgItem(IDC_CURVE_NAME);
	pCurveName->SetWindowText("R");

	m_GDlg.SetMessageWindow(m_pMessageWnd);
	m_GDlg.Create(IDD_DIALOG_CURVE, this);
	m_GDlg.GetClientRect(&rect); 
	m_GDlg.SetEnable(m_bGammaEnable);

	rect.left = clientRect.left + 400;
	rect.top = clientRect.top + 10;
	rect.right += 400;
	rect.bottom += 10;

	m_GDlg.MoveWindow(&rect);
	m_GDlg.ShowWindow(SW_SHOW);

	pCurveName = m_GDlg.GetDlgItem(IDC_CURVE_NAME);
	pCurveName->SetWindowText("G");

	m_BDlg.SetMessageWindow(m_pMessageWnd);
	m_BDlg.Create(IDD_DIALOG_CURVE, this);
	m_BDlg.GetClientRect(&rect); 
	m_BDlg.SetEnable(m_bGammaEnable);

	rect.left = clientRect.left + 10;
	rect.top = clientRect.top + 300;
	rect.right += 10;
	rect.bottom += 300;

	m_BDlg.MoveWindow(&rect);
	m_BDlg.ShowWindow(SW_SHOW);

	pCurveName = m_BDlg.GetDlgItem(IDC_CURVE_NAME);
	pCurveName->SetWindowText("B");

	m_RDlg.m_no_key_show_flag = TRUE;
	m_GDlg.m_no_key_show_flag = TRUE;
	m_BDlg.m_no_key_show_flag = TRUE;

	return TRUE;
}

void CLutRgbDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	m_RDlg.OnClose();
	m_GDlg.OnClose();
	m_BDlg.OnClose();

	CDialog::OnClose();
}

void CLutRgbDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_LUT_RGB, 0);
}

void CLutRgbDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_LUT_RGB, 0);
}

int CLutRgbDlg::SetGammaEnable(BOOL bEnable)
{
	m_bGammaEnable = bEnable;
	return 0;
}

int CLutRgbDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_GAMMA))) return -1;

	AK_ISP_INIT_GAMMA stGamma = {0};
	ZeroMemory(&stGamma, sizeof(AK_ISP_INIT_GAMMA));

	int size = LEVEL_NUM * sizeof(unsigned short);

	m_RDlg.GetLevel((char*)stGamma.p_gamma_attr.r_gamma, &size);
	m_BDlg.GetLevel((char*)stGamma.p_gamma_attr.b_gamma, &size);
	m_GDlg.GetLevel((char*)stGamma.p_gamma_attr.g_gamma, &size);

	vector<CPoint> *pts = NULL;
	int i = 0;
	pts = m_RDlg.GetKeyPts();
	size = (*pts).size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	for (i=0; i<size; i++)
	{
		stGamma.p_gamma_attr.r_key[i] = (*pts)[i].x * X_MAX / CURVE_WINDOW_WIDTH;
	}

	if (m_RDlg.m_no_key_flag == TRUE)
	{
		stGamma.p_gamma_attr.r_key[0] = 1023;
	}

	pts = m_BDlg.GetKeyPts();
	size = (*pts).size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	for (i=0; i<size; i++)
	{
		stGamma.p_gamma_attr.b_key[i] = (*pts)[i].x * X_MAX / CURVE_WINDOW_WIDTH;
	}

	if (m_BDlg.m_no_key_flag == TRUE)
	{
		stGamma.p_gamma_attr.b_key[0] = 1023;
	}


	pts = m_GDlg.GetKeyPts();
	size = (*pts).size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	for (i=0; i<size; i++)
	{
		stGamma.p_gamma_attr.g_key[i] = (*pts)[i].x * X_MAX / CURVE_WINDOW_WIDTH;
	}

	if (m_GDlg.m_no_key_flag == TRUE)
	{
		stGamma.p_gamma_attr.g_key[0] = 1023;
	}

	stGamma.p_gamma_attr.rgb_gamma_enable = m_bGammaEnable;
	stGamma.param_id = ISP_RAW_LUT;
	stGamma.length = sizeof(AK_ISP_INIT_GAMMA);

	memcpy(pPageInfoSt, &stGamma, sizeof(AK_ISP_INIT_GAMMA));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_GAMMA);

	return 0;
}

int CLutRgbDlg::SetPageInfoSt_level(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_GAMMA))) return -1;
	
	AK_ISP_INIT_GAMMA stGamma = {0};
	ZeroMemory(&stGamma, sizeof(AK_ISP_INIT_GAMMA));
	
	memcpy(&stGamma, pPageInfoSt, sizeof(AK_ISP_INIT_GAMMA));
	
	m_RDlg.SetLevel((char*)stGamma.p_gamma_attr.r_gamma, sizeof(stGamma.p_gamma_attr.r_gamma));
	m_BDlg.SetLevel((char*)stGamma.p_gamma_attr.b_gamma, sizeof(stGamma.p_gamma_attr.b_gamma));
	m_GDlg.SetLevel((char*)stGamma.p_gamma_attr.g_gamma, sizeof(stGamma.p_gamma_attr.g_gamma));
	
	if (stGamma.p_gamma_attr.r_key[0] == 1023)
	{
		stGamma.p_gamma_attr.r_key[0] = 0;
		m_RDlg.m_no_key_flag = TRUE;
	}
	else
	{
		m_RDlg.m_no_key_flag = FALSE;
	}
	m_RDlg.SetKeyPts(m_RDlg.GetKeyPts(), stGamma.p_gamma_attr.r_key, stGamma.p_gamma_attr.r_gamma);
	
	if (stGamma.p_gamma_attr.b_key[0] == 1023)
	{
		stGamma.p_gamma_attr.b_key[0] = 0;
		m_BDlg.m_no_key_flag = TRUE;
	}
	else
	{
		m_BDlg.m_no_key_flag = FALSE;
	}
	m_BDlg.SetKeyPts(m_BDlg.GetKeyPts(), stGamma.p_gamma_attr.b_key, stGamma.p_gamma_attr.b_gamma);
	
	if (stGamma.p_gamma_attr.g_key[0] == 1023)
	{
		stGamma.p_gamma_attr.g_key[0] = 0;
		m_GDlg.m_no_key_flag = TRUE;
	}
	else
	{
		m_GDlg.m_no_key_flag = FALSE;
	}
	m_GDlg.SetKeyPts(m_GDlg.GetKeyPts(), stGamma.p_gamma_attr.g_key, stGamma.p_gamma_attr.g_gamma);
	
	return 0;
}

int CLutRgbDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_GAMMA))) return -1;
	
	AK_ISP_INIT_GAMMA stGamma = {0};
	ZeroMemory(&stGamma, sizeof(AK_ISP_INIT_GAMMA));

	memcpy(&stGamma, pPageInfoSt, sizeof(AK_ISP_INIT_GAMMA));

	m_RDlg.SetLevel((char*)stGamma.p_gamma_attr.r_gamma, sizeof(stGamma.p_gamma_attr.r_gamma));
	m_BDlg.SetLevel((char*)stGamma.p_gamma_attr.b_gamma, sizeof(stGamma.p_gamma_attr.b_gamma));
	m_GDlg.SetLevel((char*)stGamma.p_gamma_attr.g_gamma, sizeof(stGamma.p_gamma_attr.g_gamma));

	if (stGamma.p_gamma_attr.r_key[0] == 1023)
	{
		stGamma.p_gamma_attr.r_key[0] = 0;
		m_RDlg.m_no_key_flag = TRUE;
	}
	else
	{
		m_RDlg.m_no_key_flag = FALSE;
	}
	m_RDlg.SetKeyPts(m_RDlg.GetKeyPts(), stGamma.p_gamma_attr.r_key, stGamma.p_gamma_attr.r_gamma);
	m_RDlg.Refresh();
	
	if (stGamma.p_gamma_attr.b_key[0] == 1023)
	{
		stGamma.p_gamma_attr.b_key[0] = 0;
		m_BDlg.m_no_key_flag = TRUE;
	}
	else
	{
		m_BDlg.m_no_key_flag = FALSE;
	}
	m_BDlg.SetKeyPts(m_BDlg.GetKeyPts(), stGamma.p_gamma_attr.b_key, stGamma.p_gamma_attr.b_gamma);
	m_BDlg.Refresh();

	if (stGamma.p_gamma_attr.g_key[0] == 1023)
	{
		stGamma.p_gamma_attr.g_key[0] = 0;
		m_GDlg.m_no_key_flag = TRUE;
	}
	else
	{
		m_GDlg.m_no_key_flag = FALSE;
	}
	m_GDlg.SetKeyPts(m_GDlg.GetKeyPts(), stGamma.p_gamma_attr.g_key, stGamma.p_gamma_attr.g_gamma);
	m_GDlg.Refresh();

	return 0;
}
