// Y_GammaDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "Y_GammaDlg.h"
#include <math.h>
#include "NetCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CY_GammaDlg dialog


CY_GammaDlg::CY_GammaDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CY_GammaDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CY_GammaDlg)
	
	//}}AFX_DATA_INIT
}


void CY_GammaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CY_GammaDlg)
	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CY_GammaDlg, CDialog)
	//{{AFX_MSG_MAP(CY_GammaDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGammaDlg message handlers

// CGammaDialog 消息处理程序

BOOL CY_GammaDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_y_gamma, sizeof(AK_ISP_INIT_Y_GAMMA));

	CRect clientRect, rect;
	
	this->GetClientRect(&clientRect); 
	m_CurveDlg.SetMessageWindow(m_pMessageWnd);

	m_CurveDlg.Create(IDD_DIALOG_CURVE, this);
	m_CurveDlg.GetClientRect(&rect); 
	m_CurveDlg.SetEnable(TRUE);

	rect.left = clientRect.left + 40;
	rect.top = clientRect.top + 30;
	rect.right += 40;
	rect.bottom += 30;

	m_CurveDlg.MoveWindow(&rect);
	m_CurveDlg.ShowWindow(SW_SHOW);

	m_CurveDlg.m_no_key_show_flag = TRUE;

	int size = LEVEL_NUM * sizeof(unsigned short);

	m_CurveDlg.GetLevel((char*)m_default_lut, &size);

	return TRUE;
}


void CY_GammaDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
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



void CY_GammaDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_Y_GAMMA, 0);

}

void CY_GammaDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_Y_GAMMA, 0);
}


void CY_GammaDlg::GetDataValue(void)
{
	UpdateData(TRUE);
	
	m_y_gamma.param_id = ISP_Y_GAMMA;
	m_y_gamma.length = sizeof(AK_ISP_INIT_Y_GAMMA);
	

	int size = LEVEL_NUM * sizeof(unsigned short);

	m_CurveDlg.GetLevel((char*)m_y_gamma.p_gamma_attr.ygamma, &size);

	vector<CPoint> *pts = NULL;
	int i = 0;
	pts = m_CurveDlg.GetKeyPts();
	size = (*pts).size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	memset(m_y_gamma.p_gamma_attr.ygamma_key, 0, 16 * sizeof(T_U16));
	
	for (i=0; i<size; i++)
	{
		m_y_gamma.p_gamma_attr.ygamma_key[i] = (*pts)[i].x * X_MAX / CURVE_WINDOW_WIDTH;
	}

	if (m_CurveDlg.m_no_key_flag == TRUE)
	{
		m_y_gamma.p_gamma_attr.ygamma_key[0] = 1023;
	}

}

void CY_GammaDlg::SetDataValue(void)
{
	if (m_y_gamma.p_gamma_attr.ygamma[0] == m_y_gamma.p_gamma_attr.ygamma[128])
	{
		return;
	}
	
	m_CurveDlg.SetLevel((char*)m_y_gamma.p_gamma_attr.ygamma, sizeof(m_y_gamma.p_gamma_attr.ygamma));
	if (m_y_gamma.p_gamma_attr.ygamma_key[0] == 1023)
	{
		m_y_gamma.p_gamma_attr.ygamma_key[0] = 0;
		m_CurveDlg.m_no_key_flag = TRUE;
	}
	else
	{
		m_CurveDlg.m_no_key_flag = FALSE;
	}
	m_CurveDlg.SetKeyPts(m_CurveDlg.GetKeyPts(), m_y_gamma.p_gamma_attr.ygamma_key, m_y_gamma.p_gamma_attr.ygamma);

	m_CurveDlg.Refresh();
	
}


void CY_GammaDlg::OnButtonSave() 
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
	
	file.Write(&m_y_gamma, sizeof(AK_ISP_INIT_Y_GAMMA));
	
	file.Close();
}

void CY_GammaDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_Y_GAMMA))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}
	
	file.Read(&m_y_gamma, sizeof(AK_ISP_INIT_Y_GAMMA));
	file.Close();
	
	SetDataValue();
}


int CY_GammaDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_Y_GAMMA))) return -1;

	GetDataValue();

	memcpy(pPageInfoSt, &m_y_gamma, sizeof(AK_ISP_INIT_Y_GAMMA));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_Y_GAMMA);

	return 0;
}


int CY_GammaDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_Y_GAMMA))) return -1;

	memcpy(&m_y_gamma, pPageInfoSt, sizeof(AK_ISP_INIT_Y_GAMMA));
	
	SetDataValue();

	return 0;
}


void CY_GammaDlg::Clean(void) 
{
	m_CurveDlg.OnButtonReset();
	m_CurveDlg.Refresh();
}

void CY_GammaDlg::OnButton5() 
{
	// TODO: Add your control notification handler code here
	AK_ISP_Y_GAMMA_PARM parm = {0};
	int size = sizeof(AK_ISP_Y_GAMMA_PARM);
	int i = 0;

	parm.ygamma_uv_adjust_enable = m_y_gamma.p_gamma_attr.ygamma_uv_adjust_enable;
	parm.ygamma_uv_adjust_level = m_y_gamma.p_gamma_attr.ygamma_uv_adjust_level;
	parm.ygamma_cnoise_yth1 = m_y_gamma.p_gamma_attr.ygamma_cnoise_yth1;
	parm.ygamma_cnoise_yth2 = m_y_gamma.p_gamma_attr.ygamma_cnoise_yth2;
	parm.ygamma_cnoise_slop = m_y_gamma.p_gamma_attr.ygamma_cnoise_slop;
	parm.ygamma_cnoise_gain = m_y_gamma.p_gamma_attr.ygamma_cnoise_gain;
	
	m_ParmDlg.SetDataValue((char*)&parm, size);

	if (IDOK == m_ParmDlg.DoModal())
	{
		m_ParmDlg.GetDataValue((char*)&parm, &size);

		m_y_gamma.p_gamma_attr.ygamma_uv_adjust_enable = parm.ygamma_uv_adjust_enable;
		m_y_gamma.p_gamma_attr.ygamma_uv_adjust_level = parm.ygamma_uv_adjust_level;
		m_y_gamma.p_gamma_attr.ygamma_cnoise_yth1 = parm.ygamma_cnoise_yth1;
		m_y_gamma.p_gamma_attr.ygamma_cnoise_yth2 = parm.ygamma_cnoise_yth2;
		m_y_gamma.p_gamma_attr.ygamma_cnoise_slop = parm.ygamma_cnoise_slop;
		m_y_gamma.p_gamma_attr.ygamma_cnoise_gain = parm.ygamma_cnoise_gain;
	}
}


int CY_GammaDlg::GetDefaultData(AK_ISP_INIT_Y_GAMMA *struct_new) 
{
	if (struct_new == NULL) return -1;
	
	struct_new->param_id = ISP_Y_GAMMA;
	struct_new->length = sizeof(AK_ISP_INIT_Y_GAMMA);

	memcpy((char*)struct_new->p_gamma_attr.ygamma, (char*)m_default_lut, 129 * sizeof(T_U16));

	return 0;
}

