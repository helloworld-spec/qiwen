// AWB_G_weightDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "AWB_G_weightDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAWB_G_weightDlg dialog


CAWB_G_weightDlg::CAWB_G_weightDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAWB_G_weightDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAWB_G_weightDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ZeroMemory(&m_isp_awb_G_weight, sizeof(AK_ISP_AWB_ATTR));
}


void CAWB_G_weightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAWB_G_weightDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAWB_G_weightDlg, CDialog)
	//{{AFX_MSG_MAP(CAWB_G_weightDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAWB_G_weightDlg message handlers

void CAWB_G_weightDlg::OnOK() 
{
	// TODO: Add extra validation here
	GetDataValue();
	CDialog::OnOK();
}

void CAWB_G_weightDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CAWB_G_weightDlg::GetDataValue(void)
{
	CString str;

	UpdateData(TRUE);
	
	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_0, str);
	m_isp_awb_G_weight.g_weight[0] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_1, str);
	m_isp_awb_G_weight.g_weight[1] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_2, str);
	m_isp_awb_G_weight.g_weight[2] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_3, str);
	m_isp_awb_G_weight.g_weight[3] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_4, str);
	m_isp_awb_G_weight.g_weight[4] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_5, str);
	m_isp_awb_G_weight.g_weight[5] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_6, str);
	m_isp_awb_G_weight.g_weight[6] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_7, str);
	m_isp_awb_G_weight.g_weight[7] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_8, str);
	m_isp_awb_G_weight.g_weight[8] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_9, str);
	m_isp_awb_G_weight.g_weight[9] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_10, str);
	m_isp_awb_G_weight.g_weight[10] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_11, str);
	m_isp_awb_G_weight.g_weight[11] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_12, str);
	m_isp_awb_G_weight.g_weight[12] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_13, str);
	m_isp_awb_G_weight.g_weight[13] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_14, str);
	m_isp_awb_G_weight.g_weight[14] = atoi(str);

	GetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_15, str);
	m_isp_awb_G_weight.g_weight[15] = atoi(str);

}

void CAWB_G_weightDlg::SetDataValue(void)
{
	CString str;
	
	
	str.Format("%d", m_isp_awb_G_weight.g_weight[0]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_0, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[1]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_1, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[2]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_2, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[3]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_3, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[4]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_4, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[5]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_5, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[6]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_6, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[7]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_7, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[8]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_8, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[9]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_9, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[10]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_10, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[11]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_11, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[12]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_12, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[13]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_13, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[14]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_14, str);

	str.Format("%d", m_isp_awb_G_weight.g_weight[15]);
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_15, str);
	
}

int CAWB_G_weightDlg::GetPageInfoSt_G_weight(void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_AWB_ATTR))) return -1;
	
	//GetDataValue();
	
	memcpy(pPageInfoSt, &m_isp_awb_G_weight, 16 * sizeof(T_U16));
	
	//nPageID = m_nID;
	nStLen = sizeof(AK_ISP_AWB_ATTR);
	
	return 0;
}

int CAWB_G_weightDlg::SetPageInfoSt_G_weight(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_AWB_ATTR))) return -1;
	
	memcpy(&m_isp_awb_G_weight, pPageInfoSt, sizeof(AK_ISP_AWB_ATTR));
	
	//SetDataValue();
	
	return 0;
}

BOOL CAWB_G_weightDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_0, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_1, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_2, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_3, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_4, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_5, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_6, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_7, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_8, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_9, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_10, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_11, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_12, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_13, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_14, "0");
	SetDlgItemText(IDC_EDIT_AWB_G_WEIGHT_15, "0");
	
	// TODO: Add extra initialization here
	
	SetDataValue();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CAWB_G_weightDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}

