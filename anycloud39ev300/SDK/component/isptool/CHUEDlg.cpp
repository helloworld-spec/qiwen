// CHUEDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "CHUEDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCHUEDlg dialog


CCHUEDlg::CCHUEDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCHUEDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCHUEDlg)
	//}}AFX_DATA_INIT
	m_mode = 0;
	m_envi = 0;
	m_Enable = 0;
}


void CCHUEDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCHUEDlg)
	DDX_Radio(pDX, IDC_RADIO5, m_mode);
	DDX_Radio(pDX, IDC_RADIO1, m_envi);
	DDX_Radio(pDX, IDC_RADIO7, m_Enable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCHUEDlg, CDialog)
	//{{AFX_MSG_MAP(CCHUEDlg)
	ON_BN_CLICKED(IDC_BUTTON_HUE_IMG, OnButtonHueImg)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnRadio6)
	ON_BN_CLICKED(IDC_RADIO7, OnRadio7)
	ON_BN_CLICKED(IDC_RADIO8, OnRadio8)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_BUTTON_HUE_CALC, OnButtonHueCalc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCHUEDlg message handlers

void CCHUEDlg::OnButtonHueImg() 
{
	T_U32 i = 0, j = 0;
	GetDataValue(FALSE);
	//获取数据
	memset(&m_HUE_img.m_HUE_lineDlg.m_hue_info, 0, sizeof(AK_ISP_HUE));
	if(m_HUE_calc.export_hue_flag)
	{
		m_HUE_calc.export_hue_flag = FALSE;

		if(0 == m_hue.p_hue.hue_mode)
		{
			for(int k = 0; k< 65; k++)
			{
				m_hue.p_hue.manual_hue.hue_lut_a[k] = m_HUE_calc.hue_para.hue_lut_a[k];
				m_hue.p_hue.manual_hue.hue_lut_b[k] = m_HUE_calc.hue_para.hue_lut_b[k];
				m_hue.p_hue.manual_hue.hue_lut_s[k] = m_HUE_calc.hue_para.hue_lut_s[k];
			}
		}
		else
		{
			for(int m = 0; m< 65; m++)
			{
				m_hue.p_hue.hue[m_envi].hue_lut_a[m] = m_HUE_calc.hue_para.hue_lut_a[m];
				m_hue.p_hue.hue[m_envi].hue_lut_b[m] = m_HUE_calc.hue_para.hue_lut_b[m];
				m_hue.p_hue.hue[m_envi].hue_lut_s[m] = m_HUE_calc.hue_para.hue_lut_s[m];
			}
		}
	}

	if (m_hue.p_hue.hue_mode == 0)
	{
		m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_sat_en = m_hue.p_hue.manual_hue.hue_sat_en;
		for(i = 0, j = 32; i < 32 && j < 64; i++, j++)
		{
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[i] = m_hue.p_hue.manual_hue.hue_lut_a[j];
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[i] = m_hue.p_hue.manual_hue.hue_lut_b[j];
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[i] = m_hue.p_hue.manual_hue.hue_lut_s[j];
		}

		for(i = 32, j = 0; i < 64 && j < 32; i++, j++)
		{
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[i] = m_hue.p_hue.manual_hue.hue_lut_a[j];
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[i] = m_hue.p_hue.manual_hue.hue_lut_b[j];
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[i] = m_hue.p_hue.manual_hue.hue_lut_s[j];
		}
	
		m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[64] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[0];//m_hue.p_hue.manual_hue.hue_lut_a[32];
		m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[64] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[0];//m_hue.p_hue.manual_hue.hue_lut_b[32];
		m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[64] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[0];//m_hue.p_hue.manual_hue.hue_lut_s[32];

		//memcpy(&m_HUE_img.m_HUE_lineDlg.m_hue_info, &m_hue.p_hue.manual_hue,sizeof(AK_ISP_HUE));
	}
	else
	{
		m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_sat_en = m_hue.p_hue.hue[m_envi].hue_sat_en;
		for(i = 0, j = 32; i < 32 && j < 64; i++, j++)
		{
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[i] = m_hue.p_hue.hue[m_envi].hue_lut_a[j];
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[i] = m_hue.p_hue.hue[m_envi].hue_lut_b[j];
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[i] = m_hue.p_hue.hue[m_envi].hue_lut_s[j];
		}
		
		for(i = 32, j = 0; i < 64 && j < 32; i++, j++)
		{
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[i] = m_hue.p_hue.hue[m_envi].hue_lut_a[j];
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[i] = m_hue.p_hue.hue[m_envi].hue_lut_b[j];
			m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[i] = m_hue.p_hue.hue[m_envi].hue_lut_s[j];
		}
		
		m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[64] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[0];
		m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[64] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[0];
		m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[64] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[0];
		//memcpy(&m_HUE_img.m_HUE_lineDlg.m_hue_info, &m_hue.p_hue.hue[m_envi], sizeof(AK_ISP_HUE));
	}

	// TODO: Add your control notification handler code here
	m_HUE_img.SetMessageWindow(m_pMessageWnd);
	//m_HUE_img.Img_SetConnectState(m_bConnect);
	if (IDOK == m_HUE_img.DoModal())
	{
		//获取数据
		if (m_hue.p_hue.hue_mode == 0)
		{
			m_hue.p_hue.manual_hue.hue_sat_en = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_sat_en;
			for(i = 0, j = 32; i < 32 && j < 64; i++, j++)
			{
				m_hue.p_hue.manual_hue.hue_lut_a[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[j];
				m_hue.p_hue.manual_hue.hue_lut_b[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[j];
				m_hue.p_hue.manual_hue.hue_lut_s[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[j];
			}
			
			for(i = 32, j = 0; i < 64 && j < 32; i++, j++)
			{
				m_hue.p_hue.manual_hue.hue_lut_a[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[j];
				m_hue.p_hue.manual_hue.hue_lut_b[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[j];
				m_hue.p_hue.manual_hue.hue_lut_s[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[j];
			}
			
			m_hue.p_hue.manual_hue.hue_lut_a[64] = m_hue.p_hue.manual_hue.hue_lut_a[0];//m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[32];
			m_hue.p_hue.manual_hue.hue_lut_b[64] = m_hue.p_hue.manual_hue.hue_lut_b[0];//m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[32];
			m_hue.p_hue.manual_hue.hue_lut_s[64] = m_hue.p_hue.manual_hue.hue_lut_s[0];//m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[32];

			//memcpy(&m_hue.p_hue.manual_hue, &m_HUE_img.m_HUE_lineDlg.m_hue_info, sizeof(AK_ISP_HUE));
		}
		else
		{

			m_hue.p_hue.hue[m_envi].hue_sat_en = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_sat_en;
			for(i = 0, j = 32; i < 32 && j < 64; i++, j++)
			{
				m_hue.p_hue.hue[m_envi].hue_lut_a[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[j];
				m_hue.p_hue.hue[m_envi].hue_lut_b[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[j];
				m_hue.p_hue.hue[m_envi].hue_lut_s[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[j];
			}
			
			for(i = 32, j = 0; i < 64 && j < 32; i++, j++)
			{
				m_hue.p_hue.hue[m_envi].hue_lut_a[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_a[j];
				m_hue.p_hue.hue[m_envi].hue_lut_b[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_b[j];
				m_hue.p_hue.hue[m_envi].hue_lut_s[i] = m_HUE_img.m_HUE_lineDlg.m_hue_info.hue_lut_s[j];
			}

			
			m_hue.p_hue.hue[m_envi].hue_lut_a[64] = m_hue.p_hue.hue[m_envi].hue_lut_a[0];
			m_hue.p_hue.hue[m_envi].hue_lut_b[64] = m_hue.p_hue.hue[m_envi].hue_lut_b[0];
			m_hue.p_hue.hue[m_envi].hue_lut_s[64] = m_hue.p_hue.hue[m_envi].hue_lut_s[0];
			//memcpy(&m_hue.p_hue.hue[m_envi], &m_HUE_img.m_HUE_lineDlg.m_hue_info, sizeof(AK_ISP_HUE));
		}
	}
}

void CCHUEDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(FALSE);
	UpdateData(FALSE);
}

void CCHUEDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(TRUE);
	UpdateData(FALSE);
}

void CCHUEDlg::EnableLinkageRadio(bool bEnable) 
{
	CWnd *pRadio[4];
	
	pRadio[0] = GetDlgItem(IDC_RADIO1);
	pRadio[1] = GetDlgItem(IDC_RADIO2);
	pRadio[2] = GetDlgItem(IDC_RADIO3);
	pRadio[3] = GetDlgItem(IDC_RADIO4);

	for (int i=0; i<4; i++)
	{
		pRadio[i]->EnableWindow(bEnable);
	}
	
}

void CCHUEDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CCHUEDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

BOOL CCHUEDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// TODO: Add extra initialization here
	ZeroMemory(&m_hue, sizeof(AK_ISP_INIT_HUE));
	m_HUE_calc.export_hue_flag = FALSE;

	EnableLinkageRadio(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CCHUEDlg::SetDataValue(bool bFromStruct)
{
	if (bFromStruct)
	{
		m_mode = m_hue.p_hue.hue_mode;
	}
	
	if (0 == m_mode)
	{
		m_Enable = m_hue.p_hue.manual_hue.hue_sat_en;
		EnableLinkageRadio(FALSE);
	}
	else
	{
		m_Enable = m_hue.p_hue.hue[m_envi].hue_sat_en;
		EnableLinkageRadio(TRUE);
	}
	
	((CButton *)GetDlgItem(IDC_RADIO5))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO6))->SetCheck(m_mode);
	((CButton *)GetDlgItem(IDC_RADIO7))->SetCheck(!m_Enable);
	((CButton *)GetDlgItem(IDC_RADIO8))->SetCheck(m_Enable);

}


BOOL CCHUEDlg::check_data_init(AK_ISP_HUE *hue) 
{	
	BOOL ret = FALSE;
	UINT i = 0;

	for (i = 0; i < 65; i++)
	{
		if (hue->hue_lut_a[i] != 0)
		{
			ret = TRUE;
			break;
		}
		if (hue->hue_lut_b[i] != 0)
		{
			ret = TRUE;
			break;
		}
		if (hue->hue_lut_s[i] != 0)
		{
			ret = TRUE;
			break;
		}
	}

	return ret;
}


void CCHUEDlg::line_data_init(AK_ISP_HUE *hue) 
{	
	UINT i = 0;

	for (i = 0; i < 65; i++)
	{
		hue->hue_lut_a[i] = 127;
		hue->hue_lut_b[i] = 0;
		hue->hue_lut_s[i] = 64;
	}
}


void CCHUEDlg::GetDataValue(bool bToStruct)
{
	int mode_tmp = 0;
	int envi_tmp = 0;
	
	if (!bToStruct)
	{
		mode_tmp = m_mode;
		envi_tmp = m_envi;
		UpdateData(TRUE);
	}
	else
	{
		UpdateData(TRUE);
		mode_tmp = m_mode;
		envi_tmp = m_envi;
	}
	
	
	m_hue.param_id = ISP_HUE;
	m_hue.length = sizeof(AK_ISP_INIT_HUE);
	
	m_hue.p_hue.hue_mode = (T_U16)m_mode;
	
	if (0 == mode_tmp)
	{
		m_hue.p_hue.manual_hue.hue_sat_en = (T_U16)m_Enable;
	}
	else
	{
		m_hue.p_hue.hue[envi_tmp].hue_sat_en = (T_U16)m_Enable;
	}	

	if (!check_data_init(&m_hue.p_hue.manual_hue))
	{
		line_data_init(&m_hue.p_hue.manual_hue);
	}

	for (int i=0; i<4; i++)
	{
		if (!check_data_init(&m_hue.p_hue.hue[i]))
		{
			line_data_init(&m_hue.p_hue.hue[i]);
		}
	}
	
}

void CCHUEDlg::OnButtonRead() 
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
	
	if (file.GetLength() != sizeof(AK_ISP_INIT_HUE))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}
	
	file.Read(&m_hue, sizeof(AK_ISP_INIT_HUE));
	file.Close();
	
	SetDataValue(TRUE);
	
	UpdateData(FALSE);
}

void CCHUEDlg::OnButtonWrite() 
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
	
	GetDataValue(TRUE);
	
	file.Write(&m_hue, sizeof(AK_ISP_INIT_HUE));
	
	file.Close();
}

void CCHUEDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_HUE, 0);
}

void CCHUEDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_HUE, 0);
}

int CCHUEDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_HUE))) return -1;
	
	GetDataValue(TRUE);
	
	memcpy(pPageInfoSt, &m_hue, sizeof(AK_ISP_INIT_HUE));
	
	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_HUE);
	
	return 0;
}

int CCHUEDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_HUE))) return -1;
	
	memcpy(&m_hue, pPageInfoSt, sizeof(AK_ISP_INIT_HUE));
	
	SetDataValue(TRUE);
	
	return 0;
}

void CCHUEDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CCHUEDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CCHUEDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CCHUEDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CCHUEDlg::OnButtonHueCalc() 
{
	// TODO: Add your control notification handler code here
	m_HUE_calc.SetMessageWindow(m_pMessageWnd);
//	m_HUE_calc.Img_SetConnectState(m_bConnect);
	m_HUE_calc.DoModal();
}
