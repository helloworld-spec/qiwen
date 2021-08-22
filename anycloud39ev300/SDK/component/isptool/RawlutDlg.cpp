// RawlutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "RawlutDlg.h"
#include <math.h>
#include "NetCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRawlutDlg dialog


CRawlutDlg::CRawlutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRawlutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRawlutDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bGammaEnable = FALSE;
	m_bSetApart = FALSE;
}


void CRawlutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRawlutDlg)
	DDX_Control(pDX, IDC_CHECK_ENABLE, m_GammaCheck);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRawlutDlg, CDialog)
	//{{AFX_MSG_MAP(CRawlutDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_CHECK_ENABLE, OnCheckEnable)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRawlutDlg message handlers


BOOL CRawlutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect clientRect, rect;
	
	this->GetClientRect(&clientRect); 

	m_CurveDlg.SetMessageWindow(m_pMessageWnd);
	m_CurveDlg.Create(IDD_DIALOG_CURVE, this);
	m_CurveDlg.GetClientRect(&rect); 
	m_CurveDlg.SetEnable(m_bGammaEnable);
	rect.left = clientRect.left + 40;
	rect.top = clientRect.top + 60;
	rect.right += 40;
	rect.bottom += 60;

	m_CurveDlg.MoveWindow(&rect);
	m_CurveDlg.ShowWindow(SW_SHOW);

	CWnd * pCurveName = m_CurveDlg.GetDlgItem(IDC_CURVE_NAME);
	
	pCurveName->SetWindowText("ALL");

	m_CurveDlg.m_no_key_show_flag = TRUE;

	return TRUE;
}

void CRawlutDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	AK_ISP_INIT_GAMMA stGamma = {0};
	int pageid = 0;
	int nStLen = sizeof(AK_ISP_INIT_GAMMA);
	//GetPageInfoSt(pageid, &stGamma, nStLen);


	//m_LargeDlg.SetPageInfoSt(&stGamma, nStLen);

	m_LargeDlg.SetMessageWindow(m_pMessageWnd);

	vector<CPoint> *pts_tmp = NULL;
	vector<CPoint> *pts_self = NULL;
	int i = 0;
	int size = 0;

	pts_self = m_CurveDlg.GetKeyPts();
	size = (*pts_self).size();

	pts_tmp = m_LargeDlg.m_CurveDlg.GetKeyPts();
	(*pts_tmp).resize(size);
	for (i=0; i<size; i++)
	{
		(*pts_tmp)[i].x = (*pts_self)[i].x * LCURVE_WINDOW_WIDTH / CURVE_WINDOW_WIDTH;
		(*pts_tmp)[i].y = (*pts_self)[i].y * LCURVE_WINDOW_HEIGHT / CURVE_WINDOW_HEIGHT;
	}

	
	if (IDOK == m_LargeDlg.DoModal())
	{
		//m_LargeDlg.GetPageInfoSt(pageid, &stGamma, nStLen);
		//SetPageInfoSt(&stGamma, nStLen);

		pts_tmp = m_LargeDlg.m_CurveDlg.GetKeyPts();
		size = (*pts_tmp).size();

		pts_self = m_CurveDlg.GetKeyPts();
		(*pts_self).resize(size);

		for (i=0; i<size; i++)
		{
			(*pts_self)[i].x = (*pts_tmp)[i].x * CURVE_WINDOW_WIDTH / LCURVE_WINDOW_WIDTH;
			(*pts_self)[i].y = (*pts_tmp)[i].y * CURVE_WINDOW_HEIGHT / LCURVE_WINDOW_HEIGHT;
		}

		if (NULL == m_pMessageWnd) 
		{
			AfxMessageBox(_T("copy ui to txt fail"), MB_OK);
			return;
		}
		
		CBasePage::SendPageMessage(m_pMessageWnd, WM_COPY_UI_TO_TEXT, 0, 0);

	}
}

void CRawlutDlg::OnCheckEnable() 
{
	// TODO: Add your control notification handler code here
	m_bGammaEnable = m_GammaCheck.GetCheck();
	m_CurveDlg.SetEnable(m_bGammaEnable);
	m_RgbDlg.SetGammaEnable(m_bGammaEnable);
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, DIALOG_RAWLUT, m_bGammaEnable);
}

int CRawlutDlg::SetGammaEnable(BOOL bEnable)
{
	m_GammaCheck.SetCheck(bEnable);
	return 0;
}

void CRawlutDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_RAWLUT, 0);

}

void CRawlutDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_RAWLUT, 0);
}


int CRawlutDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_GAMMA))) return -1;

	AK_ISP_INIT_GAMMA stGamma = {0};
	ZeroMemory(&stGamma, sizeof(AK_ISP_INIT_GAMMA));

	int size = LEVEL_NUM * sizeof(unsigned short);

	m_CurveDlg.GetLevel((char*)stGamma.p_gamma_attr.r_gamma, &size);
	m_CurveDlg.GetLevel((char*)stGamma.p_gamma_attr.g_gamma, &size);
	m_CurveDlg.GetLevel((char*)stGamma.p_gamma_attr.b_gamma, &size);

	vector<CPoint> *pts = NULL;
	int i = 0;
	pts = m_CurveDlg.GetKeyPts();
	size = (*pts).size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	for (i=0; i<size; i++)
	{
		stGamma.p_gamma_attr.r_key[i] = (*pts)[i].x * X_MAX / CURVE_WINDOW_WIDTH;
		stGamma.p_gamma_attr.g_key[i] = (*pts)[i].x * X_MAX / CURVE_WINDOW_WIDTH;
		stGamma.p_gamma_attr.b_key[i] = (*pts)[i].x * X_MAX / CURVE_WINDOW_WIDTH;
	}

	if (m_CurveDlg.m_no_key_flag == TRUE)
	{
		stGamma.p_gamma_attr.r_key[0] = 1023;
		stGamma.p_gamma_attr.g_key[0] = 1023;
		stGamma.p_gamma_attr.b_key[0] = 1023;
	}

	stGamma.p_gamma_attr.rgb_gamma_enable = m_bGammaEnable;
	stGamma.param_id = ISP_RAW_LUT;
	stGamma.length = sizeof(AK_ISP_INIT_GAMMA);

	memcpy(pPageInfoSt, &stGamma, sizeof(AK_ISP_INIT_GAMMA));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_GAMMA);

	return 0;
}


int CRawlutDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_GAMMA))) return -1;
	
	AK_ISP_INIT_GAMMA stGamma = {0};
	ZeroMemory(&stGamma, sizeof(AK_ISP_INIT_GAMMA));

	memcpy(&stGamma, pPageInfoSt, sizeof(AK_ISP_INIT_GAMMA));

	m_bGammaEnable = stGamma.p_gamma_attr.rgb_gamma_enable;

	SetGammaEnable(m_bGammaEnable);
	m_CurveDlg.SetEnable(m_bGammaEnable);

	m_CurveDlg.SetLevel((char*)stGamma.p_gamma_attr.r_gamma, sizeof(stGamma.p_gamma_attr.r_gamma));

	if (stGamma.p_gamma_attr.r_key[0] == 1023)
	{
		stGamma.p_gamma_attr.r_key[0] = 0;
		m_CurveDlg.m_no_key_flag = TRUE;
	}
	else
	{
		m_CurveDlg.m_no_key_flag = FALSE;
	}
	m_CurveDlg.SetKeyPts(m_CurveDlg.GetKeyPts(), stGamma.p_gamma_attr.r_key, stGamma.p_gamma_attr.r_gamma);

	m_CurveDlg.Refresh();

	//m_RgbDlg.SetPageInfoSt_level(pPageInfoSt, sizeof(AK_ISP_INIT_GAMMA));
	
	return 0;
}

BOOL CRawlutDlg::IsSetApart()
{
	return m_bSetApart;
}


void CRawlutDlg::Clean(void) 
{
	m_CurveDlg.OnButtonReset();
	m_CurveDlg.Refresh();
}