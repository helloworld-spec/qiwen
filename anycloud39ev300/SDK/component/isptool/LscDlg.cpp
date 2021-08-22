// LscDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "LscDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLscDlg dialog


CLscDlg::CLscDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLscDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLscDlg)
	m_Enable = 0;
	//}}AFX_DATA_INIT
}


void CLscDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLscDlg)
	DDX_Control(pDX, IDC_SPIN3, m_shift);
	DDX_Control(pDX, IDC_SPIN2, m_yref);
	DDX_Control(pDX, IDC_SPIN1, m_xref);
	DDX_Radio(pDX, IDC_RADIO1, m_Enable);
	//}}AFX_DATA_MAP

	DDX_Control(pDX, IDC_SPIN14, m_range[0]);
	DDX_Control(pDX, IDC_SPIN5, m_range[1]);
	DDX_Control(pDX, IDC_SPIN6, m_range[2]);
	DDX_Control(pDX, IDC_SPIN7, m_range[3]);
	DDX_Control(pDX, IDC_SPIN8, m_range[4]);
	DDX_Control(pDX, IDC_SPIN9, m_range[5]);
	DDX_Control(pDX, IDC_SPIN10, m_range[6]);
	DDX_Control(pDX, IDC_SPIN11, m_range[7]);
	DDX_Control(pDX, IDC_SPIN12, m_range[8]);
	DDX_Control(pDX, IDC_SPIN16, m_range[9]);

	DDX_Control(pDX, IDC_SPIN17, m_Strength_spin);
}


BEGIN_MESSAGE_MAP(CLscDlg, CDialog)
	//{{AFX_MSG_MAP(CLscDlg)
	ON_BN_CLICKED(IDC_BUTTON_LSC, OnButtonLsc)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_BUTTON_LSC_CALC, OnButtonLscCalc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLscDlg message handlers

BOOL CLscDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Lsc, sizeof(AK_ISP_INIT_LSC));

	m_xref.SetRange(0,4096);
	m_xref.SetPos(0);

	m_yref.SetRange(0,4096);
	m_yref.SetPos(0);


	m_shift.SetRange(0,15);
	m_shift.SetPos(0);

	m_Strength_spin.SetRange(0, 100);
	m_Strength_spin.SetPos(90);

	for (int i=0; i<10; i++)
	{
		m_range[i].SetRange(0,1023);
		m_range[i].SetPos(0);
	}

	return TRUE;
}

BOOL CLscDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CLscDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_LSC))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Lsc, sizeof(AK_ISP_INIT_LSC));
	file.Close();

	SetDataValue();

	UpdateData(FALSE);
}

void CLscDlg::OnButtonWrite() 
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

	file.Write(&m_Lsc, sizeof(AK_ISP_INIT_LSC));

	file.Close();
}

void CLscDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_LSC, 0);
}

void CLscDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_LSC, 0);
}

void CLscDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	m_Lsc.param_id = ISP_LSC;
	m_Lsc.length = sizeof(AK_ISP_INIT_LSC);
	
	m_Lsc.lsc.enable = (T_U16)m_Enable;

	m_Lsc.lsc.xref = (T_U16)m_xref.GetPos();
	m_Lsc.lsc.yref = (T_U16)m_yref.GetPos();
	m_Lsc.lsc.lsc_shift = (T_U16)m_shift.GetPos();

	for (int i=0; i<10; i++)
	{
		m_Lsc.lsc.range[i] = (T_U16)m_range[i].GetPos();
	}

	m_Strength = m_Strength_spin.GetPos();
}

void CLscDlg::SetDataValue(void)
{
	m_Enable = m_Lsc.lsc.enable;
	
	m_xref.SetPos(m_Lsc.lsc.xref);
	m_yref.SetPos(m_Lsc.lsc.yref);
	m_shift.SetPos(m_Lsc.lsc.lsc_shift);

	for (int i=0; i<10; i++)
	{
		m_range[i].SetPos(m_Lsc.lsc.range[i]);
	}

	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(!m_Enable);
	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(m_Enable);
}

int CLscDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_LSC))) return -1;

	GetDataValue();

	memcpy(pPageInfoSt, &m_Lsc, sizeof(AK_ISP_INIT_LSC));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_LSC);

	return 0;
}


int CLscDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_LSC))) return -1;

	memcpy(&m_Lsc, pPageInfoSt, sizeof(AK_ISP_INIT_LSC));

	SetDataValue();
	
	return 0;
}

void CLscDlg::OnButtonLsc() 
{
	// TODO: Add your control notification handler code here

	memcpy(&m_LscCoefDlg.m_r_coef, &m_Lsc.lsc.lsc_r_coef, sizeof(lens_coef));
	memcpy(&m_LscCoefDlg.m_gr_coef, &m_Lsc.lsc.lsc_gr_coef, sizeof(lens_coef));
	memcpy(&m_LscCoefDlg.m_gb_coef, &m_Lsc.lsc.lsc_gb_coef, sizeof(lens_coef));
	memcpy(&m_LscCoefDlg.m_b_coef, &m_Lsc.lsc.lsc_b_coef, sizeof(lens_coef));

	if (IDOK == m_LscCoefDlg.DoModal())
	{
		memcpy(&m_Lsc.lsc.lsc_r_coef, &m_LscCoefDlg.m_r_coef, sizeof(lens_coef));
		memcpy(&m_Lsc.lsc.lsc_gr_coef, &m_LscCoefDlg.m_gr_coef, sizeof(lens_coef));
		memcpy(&m_Lsc.lsc.lsc_gb_coef, &m_LscCoefDlg.m_gb_coef, sizeof(lens_coef));
		memcpy(&m_Lsc.lsc.lsc_b_coef, &m_LscCoefDlg.m_b_coef, sizeof(lens_coef));
	}
}

void CLscDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CLscDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CLscDlg::Clean(void) 
{
	ZeroMemory(&m_Lsc, sizeof(AK_ISP_INIT_LSC));
	SetDataValue();
	UpdateData(FALSE);
}

void CLscDlg::SetConnectState(bool bConnect) 
{
	// TODO: Add your control notification handler code here

	m_bConnect = bConnect;
	
	m_BrightnessCalcDlg.Img_SetConnectState(m_bConnect);
}


void CLscDlg::OnButtonLscCalc() 
{
	// TODO: Add your control notification handler code here
	
	if (NULL == m_pMessageWnd)
	{
		AfxMessageBox("m_pMessageWnd is null\n", MB_OK);
		return;
	}
	
	GetDataValue();

	memcpy(&m_BrightnessCalcDlg.m_Lsc, &m_Lsc, sizeof(AK_ISP_INIT_LSC));

	m_BrightnessCalcDlg.SetMessageWindow(m_pMessageWnd);
	m_BrightnessCalcDlg.Create(IDD_DIALOG_LSC_BRIGHTNESS_CALC, this);
	m_BrightnessCalcDlg.ShowWindow(SW_SHOW);
}

void CLscDlg::refresh_data() 
{
	GetDataValue();

	memcpy(&m_BrightnessCalcDlg.m_Lsc, &m_Lsc, sizeof(AK_ISP_INIT_LSC));
	m_BrightnessCalcDlg.m_Strength = m_Strength;
}

void CLscDlg::save_calc_result() 
{
	memcpy(&m_Lsc, &m_BrightnessCalcDlg.m_Lsc, sizeof(AK_ISP_INIT_LSC));

	SetDataValue();
}


