// RawLutLargeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "RawLutLargeDlg.h"
#include "NetCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRawLutLargeDlg dialog


CRawLutLargeDlg::CRawLutLargeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRawLutLargeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRawLutLargeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bGammaEnable = FALSE;
}


void CRawLutLargeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRawLutLargeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRawLutLargeDlg, CDialog)
	//{{AFX_MSG_MAP(CRawLutLargeDlg)
		// NOTE: the ClassWizard will add message map macros here
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, OnButtonCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRawLutLargeDlg message handlers
BOOL CRawLutLargeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect clientRect, rect;
	
	this->GetClientRect(&clientRect); 
	
	m_CurveDlg.SetMessageWindow(m_pMessageWnd);
	m_CurveDlg.Create(IDD_DIALOG_CURVELARGE, this);
	m_CurveDlg.GetClientRect(&rect); 
	m_CurveDlg.SetEnable(m_bGammaEnable);
	
	rect.left = clientRect.left + 10;
	rect.top = clientRect.top + 10;
	rect.right += 10;
	rect.bottom += 10;

	m_CurveDlg.MoveWindow(&rect);
	m_CurveDlg.ShowWindow(SW_SHOW);

	m_CurveDlg.m_no_key_show_flag = TRUE;

	m_CurveDlg.Refresh();

	return TRUE;
}

void CRawLutLargeDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	m_CurveDlg.OnClose();

	CDialog::OnClose();
}

int CRawLutLargeDlg::SetGammaEnable(BOOL bEnable)
{
	m_bGammaEnable = bEnable;
	return 0;
}

int CRawLutLargeDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
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
		stGamma.p_gamma_attr.r_key[i] = (*pts)[i].x * X_MAX / LCURVE_WINDOW_WIDTH;
		stGamma.p_gamma_attr.g_key[i] = (*pts)[i].x * X_MAX / LCURVE_WINDOW_WIDTH;
		stGamma.p_gamma_attr.b_key[i] = (*pts)[i].x * X_MAX / LCURVE_WINDOW_WIDTH;
	}

	if (m_CurveDlg.m_no_key_flag == TRUE)
	{
		stGamma.p_gamma_attr.r_key[0] = 1023;
		stGamma.p_gamma_attr.g_key[0] = 1023;
		stGamma.p_gamma_attr.b_key[0] = 1023;
	}

	stGamma.p_gamma_attr.rgb_gamma_enable = m_bGammaEnable;
	stGamma.param_id = ISP_GAMMA;
	stGamma.length = sizeof(AK_ISP_INIT_GAMMA);

	memcpy(pPageInfoSt, &stGamma, sizeof(AK_ISP_INIT_GAMMA));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_GAMMA);

	return 0;
}


int CRawLutLargeDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
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

	return 0;
}

void CRawLutLargeDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
	OnClose();
}

void CRawLutLargeDlg::OnButtonCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
	OnClose();
}
