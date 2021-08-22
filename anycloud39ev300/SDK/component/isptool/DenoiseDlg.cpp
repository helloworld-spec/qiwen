// DenoiseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "DenoiseDlg.h"
#include "Netctrl.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define BF_STEP	16
#define BF_FIXPOINT	(1<<10)

static int calc_nr_weightRange(int range, int delta)
{
	int start,end,ret;
	int expStart,expEnd, expStep;

	if (0 == delta)
		return 1;
		
	start = range/BF_STEP;
	end = start+1;
	start = start*BF_STEP;
	end = end*BF_STEP;

	expStart = (int)(exp(0-(float)start*start/delta)*BF_FIXPOINT);
	expEnd = (int)(exp(0-(float)end*end/delta)*BF_FIXPOINT);
	expStep = (expStart-expEnd)/BF_STEP;

	ret = expStart-(range-start)*expStep;
	//if(ret == 0)
	//	ret = 1;
	if(ret >1023)
		ret = 1023;
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// CDenoiseDlg dialog


CDenoiseDlg::CDenoiseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDenoiseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDenoiseDlg)
	m_Nr1_Enable = 0;
	m_Nr2_Enable = 0;
	m_Nr1mode = 0;
	m_Nr2mode = 0;
	m_Y_Dpc_Enable = 0;
	m_Y_Black_Dpc_Enable = 0;
	m_Y_White_Dpc_Enable = 0;
	//}}AFX_DATA_INIT
}


void CDenoiseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDenoiseDlg)
	DDX_Control(pDX, IDC_SPIN87, m_Nr2strength);
	DDX_Control(pDX, IDC_SPIN6, m_Nr2_k);
	DDX_Control(pDX, IDC_SPIN4, m_Nr1_k);
	DDX_Control(pDX, IDC_SPIN3, m_Bstrength);
	DDX_Control(pDX, IDC_SPIN2, m_Gstrength);
	DDX_Control(pDX, IDC_SPIN1, m_Rstrength);
	DDX_Control(pDX, IDC_SPIN11, m_Y_Dpc_th);
	DDX_Radio(pDX, IDC_RADIO1, m_Nr1_Enable);
	DDX_Radio(pDX, IDC_RADIO33, m_Nr2_Enable);
	DDX_Radio(pDX, IDC_RADIO3, m_Nr1mode);
	DDX_Radio(pDX, IDC_RADIO5, m_Nr2mode);
	DDX_Radio(pDX, IDC_RADIO35, m_Y_Dpc_Enable);
	DDX_Radio(pDX, IDC_RADIO37, m_Y_Black_Dpc_Enable);
	DDX_Radio(pDX, IDC_RADIO39, m_Y_White_Dpc_Enable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDenoiseDlg, CDialog)
	//{{AFX_MSG_MAP(CDenoiseDlg)
	ON_BN_CLICKED(IDC_BUTTON_LUT, OnButtonLut)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO33, OnRadio33)
	ON_BN_CLICKED(IDC_RADIO34, OnRadio34)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnRadio6)
	ON_BN_CLICKED(IDC_RADIO35, OnRadio35)
	ON_BN_CLICKED(IDC_RADIO36, OnRadio36)
	ON_BN_CLICKED(IDC_RADIO37, OnRadio37)
	ON_BN_CLICKED(IDC_RADIO38, OnRadio38)
	ON_BN_CLICKED(IDC_RADIO39, OnRadio39)
	ON_BN_CLICKED(IDC_RADIO92, OnRadio92)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDenoiseDlg message handlers
BOOL CDenoiseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Nr, sizeof(AK_ISP_INIT_NR));

	m_NrLutDlg.m_CurveDlg.SetLevelNum(NR_LUT_LEVEL_NUM);

	m_Rstrength.SetRange32(0,65535);
	m_Rstrength.SetPos(0);

	m_Gstrength.SetRange32(0,65535);
	m_Gstrength.SetPos(0);

	m_Bstrength.SetRange32(0,65535);
	m_Bstrength.SetPos(0);

	m_Nr1_k.SetRange(0,15);
	m_Nr1_k.SetPos(0);

	m_Nr2strength.SetRange32(0,65535);
	m_Nr2strength.SetPos(0);

	m_Nr2_k.SetRange(0,15);
	m_Nr2_k.SetPos(0);

	m_Y_Dpc_th.SetRange(0,1023);
	m_Y_Dpc_th.SetPos(0);

	int size = NR_LUT_LEVEL_NUM * sizeof(unsigned short);
	m_NrLutDlg.m_CurveDlg.GetLevel((char*)m_Nr.p_nr1.manual_nr1.nr1_lc_lut, &size);

	for (int i=0; i<9; i++)
	{
		m_NrLutDlg.m_CurveDlg.GetLevel((char*)m_Nr.p_nr1.linkage_nr1[i].nr1_lc_lut, &size);
	}

	for (i=0; i<10; i++)
	{
		m_lut_keyPts[i].clear();
		m_lut_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_lut_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));
	}

	m_NrLutDlg.m_CurveDlg.m_no_key_show_flag = FALSE;

	return TRUE;
}

BOOL CDenoiseDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CDenoiseDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	m_Nr.param_id = ISP_NR;
	m_Nr.length = sizeof(AK_ISP_INIT_NR);

	m_Nr.p_nr1.manual_nr1.nr1_calc_r_k = (T_U16)m_Rstrength.GetPos();
	m_Nr.p_nr1.manual_nr1.nr1_calc_g_k = (T_U16)m_Gstrength.GetPos();
	m_Nr.p_nr1.manual_nr1.nr1_calc_b_k = (T_U16)m_Bstrength.GetPos();
	m_Nr.p_nr1.manual_nr1.nr1_k = (T_U16)m_Nr1_k.GetPos();
	m_Nr.p_nr1.manual_nr1.nr1_enable = (T_U16)m_Nr1_Enable;
	m_Nr.p_nr1.nr1_mode = (T_U16)m_Nr1mode;

	m_Nr.p_nr2.manual_nr2.nr2_calc_y_k = (T_U16)m_Nr2strength.GetPos();
	m_Nr.p_nr2.manual_nr2.nr2_k = (T_U16)m_Nr2_k.GetPos();
	m_Nr.p_nr2.manual_nr2.y_dpc_th = (T_U16)m_Y_Dpc_th.GetPos();
	m_Nr.p_nr2.manual_nr2.nr2_enable = (T_U16)m_Nr2_Enable;
	m_Nr.p_nr2.manual_nr2.y_dpc_enable = (T_U16)m_Y_Dpc_Enable;
	m_Nr.p_nr2.manual_nr2.y_black_dpc_enable = (T_U16)m_Y_Black_Dpc_Enable;
	m_Nr.p_nr2.manual_nr2.y_white_dpc_enable = (T_U16)m_Y_White_Dpc_Enable;
	m_Nr.p_nr2.nr2_mode = (T_U16)m_Nr2mode;

	
	int i = 0;
	int j = 0;

	for(i=0;i<17;i++)
	{
		m_Nr.p_nr1.manual_nr1.nr1_weight_rtbl[i] = (T_U16)calc_nr_weightRange(i<<4,m_Nr.p_nr1.manual_nr1.nr1_calc_r_k);
		m_Nr.p_nr1.manual_nr1.nr1_weight_gtbl[i] = (T_U16)calc_nr_weightRange(i<<4,m_Nr.p_nr1.manual_nr1.nr1_calc_g_k);
		m_Nr.p_nr1.manual_nr1.nr1_weight_btbl[i] = (T_U16)calc_nr_weightRange(i<<4,m_Nr.p_nr1.manual_nr1.nr1_calc_b_k);

		m_Nr.p_nr2.manual_nr2.nr2_weight_tbl[i] = (T_U16)calc_nr_weightRange(i<<4,m_Nr.p_nr2.manual_nr2.nr2_calc_y_k);
	}

	for (j=0; j<9; j++)
	{
		for(i=0;i<17;i++)
		{
			m_Nr.p_nr1.linkage_nr1[j].nr1_weight_rtbl[i] = (T_U16)calc_nr_weightRange(i<<4,m_Nr.p_nr1.linkage_nr1[j].nr1_calc_r_k);
			m_Nr.p_nr1.linkage_nr1[j].nr1_weight_gtbl[i] = (T_U16)calc_nr_weightRange(i<<4,m_Nr.p_nr1.linkage_nr1[j].nr1_calc_g_k);
			m_Nr.p_nr1.linkage_nr1[j].nr1_weight_btbl[i] = (T_U16)calc_nr_weightRange(i<<4,m_Nr.p_nr1.linkage_nr1[j].nr1_calc_b_k);

			m_Nr.p_nr2.linkage_nr2[j].nr2_weight_tbl[i] = (T_U16)calc_nr_weightRange(i<<4,m_Nr.p_nr2.linkage_nr2[j].nr2_calc_y_k);
		}
	}

	int size = m_lut_keyPts[0].size();

	if (size > KEY_POINT_MAX)
	{
		size = KEY_POINT_MAX;
	}

	memset(&m_Nr.p_nr1.manual_nr1.nr1_lc_lut_key, 0, sizeof(T_U16) * 16);

	for (i=0; i<size; i++)
	{
		m_Nr.p_nr1.manual_nr1.nr1_lc_lut_key[i] = m_lut_keyPts[0][i].x * (NR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
	}


	for (i=0; i<9; i++)
	{
		size = m_lut_keyPts[i+1].size();
		if (size > KEY_POINT_MAX)
		{
			size = KEY_POINT_MAX;
		}

		memset(&m_Nr.p_nr1.linkage_nr1[i].nr1_lc_lut_key, 0, sizeof(T_U16) * 16);

		for (j=0; j<size; j++)
		{
			m_Nr.p_nr1.linkage_nr1[i].nr1_lc_lut_key[j] = m_lut_keyPts[i+1][j].x * (NR_LUT_LEVEL_NUM - 1) / CURVE_WINDOW_WIDTH;
		}
	}
}

void CDenoiseDlg::SetDataValue(void)
{
	CWnd *pWnd = NULL;
	CString str;
	
	m_Rstrength.SetPos(m_Nr.p_nr1.manual_nr1.nr1_calc_r_k);
	pWnd = GetDlgItem(IDC_EDIT1);
	str.Format("%d", m_Nr.p_nr1.manual_nr1.nr1_calc_r_k);
	pWnd->SetWindowText(str);
		
	m_Gstrength.SetPos(m_Nr.p_nr1.manual_nr1.nr1_calc_g_k);
	pWnd = GetDlgItem(IDC_EDIT2);
	str.Format("%d", m_Nr.p_nr1.manual_nr1.nr1_calc_g_k);
	pWnd->SetWindowText(str);
	
	m_Bstrength.SetPos(m_Nr.p_nr1.manual_nr1.nr1_calc_b_k);
	pWnd = GetDlgItem(IDC_EDIT3);
	str.Format("%d", m_Nr.p_nr1.manual_nr1.nr1_calc_b_k);
	pWnd->SetWindowText(str);
	
	m_Nr1_k.SetPos(m_Nr.p_nr1.manual_nr1.nr1_k);
	m_Nr1_Enable = m_Nr.p_nr1.manual_nr1.nr1_enable;
	m_Nr1mode = m_Nr.p_nr1.nr1_mode;

	m_Nr2strength.SetPos(m_Nr.p_nr2.manual_nr2.nr2_calc_y_k);
	pWnd = GetDlgItem(IDC_EDIT6);
	str.Format("%d", m_Nr.p_nr2.manual_nr2.nr2_calc_y_k);
	pWnd->SetWindowText(str);
	
	m_Nr2_k.SetPos(m_Nr.p_nr2.manual_nr2.nr2_k);
	m_Y_Dpc_th.SetPos(m_Nr.p_nr2.manual_nr2.y_dpc_th);
	m_Nr2_Enable = m_Nr.p_nr2.manual_nr2.nr2_enable;
	m_Y_Dpc_Enable = m_Nr.p_nr2.manual_nr2.y_dpc_enable;
	m_Y_Black_Dpc_Enable = m_Nr.p_nr2.manual_nr2.y_black_dpc_enable;
	m_Y_White_Dpc_Enable = m_Nr.p_nr2.manual_nr2.y_white_dpc_enable;
	
	m_Nr2mode = m_Nr.p_nr2.nr2_mode;

	m_NrLutDlg.m_CurveDlg.SetKeyPts(&m_lut_keyPts[0], m_Nr.p_nr1.manual_nr1.nr1_lc_lut_key, m_Nr.p_nr1.manual_nr1.nr1_lc_lut);


	for (int j=0; j<9; j++)
	{
		m_NrLutDlg.m_CurveDlg.SetKeyPts(&m_lut_keyPts[j+1], m_Nr.p_nr1.linkage_nr1[j].nr1_lc_lut_key, m_Nr.p_nr1.linkage_nr1[j].nr1_lc_lut);
	}

	((CButton *)GetDlgItem(IDC_RADIO3))->SetCheck(!m_Nr1mode);
	((CButton *)GetDlgItem(IDC_RADIO4))->SetCheck(m_Nr1mode);
	((CButton *)GetDlgItem(IDC_RADIO5))->SetCheck(!m_Nr2mode);
	((CButton *)GetDlgItem(IDC_RADIO6))->SetCheck(m_Nr2mode);
	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(!m_Nr1_Enable);
	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(m_Nr1_Enable);
	((CButton *)GetDlgItem(IDC_RADIO33))->SetCheck(!m_Nr2_Enable);
	((CButton *)GetDlgItem(IDC_RADIO34))->SetCheck(m_Nr2_Enable);
	((CButton *)GetDlgItem(IDC_RADIO35))->SetCheck(!m_Y_Dpc_Enable);
	((CButton *)GetDlgItem(IDC_RADIO36))->SetCheck(m_Y_Dpc_Enable);
	((CButton *)GetDlgItem(IDC_RADIO37))->SetCheck(!m_Y_Black_Dpc_Enable);
	((CButton *)GetDlgItem(IDC_RADIO38))->SetCheck(m_Y_Black_Dpc_Enable);
	((CButton *)GetDlgItem(IDC_RADIO39))->SetCheck(!m_Y_White_Dpc_Enable);
	((CButton *)GetDlgItem(IDC_RADIO92))->SetCheck(m_Y_White_Dpc_Enable);

}

int CDenoiseDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_NR))) return -1;

	GetDataValue();

	memcpy(pPageInfoSt, &m_Nr, sizeof(AK_ISP_INIT_NR));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_NR);

	return 0;
}


int CDenoiseDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_NR))) return -1;

	memcpy(&m_Nr, pPageInfoSt, sizeof(AK_ISP_INIT_NR));

	SetDataValue();
	
	return 0;
}

void CDenoiseDlg::OnButtonLut() 
{
	// TODO: Add your control notification handler code here
	int size = NR_LUT_LEVEL_NUM * sizeof(unsigned short);
	int i = 0;
	int keypts_size = 0;

	memcpy(m_NrLutDlg.m_lc_lut[0], m_Nr.p_nr1.manual_nr1.nr1_lc_lut, size);

	for (i=0; i<9; i++)
	{
		memcpy(m_NrLutDlg.m_lc_lut[i+1], m_Nr.p_nr1.linkage_nr1[i].nr1_lc_lut, size);
	}

	for (i=0; i<10; i++)
	{
		m_NrLutDlg.m_lut_keyPts[i].clear();
		keypts_size = m_lut_keyPts[i].size();
		m_NrLutDlg.m_lut_keyPts[i].resize(keypts_size);

		for (int j=0; j<keypts_size; j++)
		{
			m_NrLutDlg.m_lut_keyPts[i][j].x = m_lut_keyPts[i][j].x;
			m_NrLutDlg.m_lut_keyPts[i][j].y = m_lut_keyPts[i][j].y;
		}
	}

	if (IDOK == m_NrLutDlg.DoModal())
	{
		memcpy(m_Nr.p_nr1.manual_nr1.nr1_lc_lut, m_NrLutDlg.m_lc_lut[0], size);

		for (i=0; i<9; i++)
		{
			memcpy(m_Nr.p_nr1.linkage_nr1[i].nr1_lc_lut, m_NrLutDlg.m_lc_lut[i+1], size);
		}

		for (i=0; i<10; i++)
		{
			keypts_size = m_NrLutDlg.m_lut_keyPts[i].size();
			m_lut_keyPts[i].resize(keypts_size);

			for (int j=0; j<keypts_size; j++)
			{
				m_lut_keyPts[i][j].x = m_NrLutDlg.m_lut_keyPts[i][j].x;
				m_lut_keyPts[i][j].y = m_NrLutDlg.m_lut_keyPts[i][j].y;
			}
		}
	}
}

void CDenoiseDlg::OnButton6() 
{
	// TODO: Add your control notification handler code here
	NR_LINKAGE linkage[9] = {0};
	int size = sizeof(linkage);
	int i = 0;

	for (i=0; i<9; i++)
	{
		linkage[i].nr1_calc_r_k = m_Nr.p_nr1.linkage_nr1[i].nr1_calc_r_k;
		linkage[i].nr1_calc_g_k = m_Nr.p_nr1.linkage_nr1[i].nr1_calc_g_k;
		linkage[i].nr1_calc_b_k = m_Nr.p_nr1.linkage_nr1[i].nr1_calc_b_k;
		linkage[i].nr1_enable = m_Nr.p_nr1.linkage_nr1[i].nr1_enable;
		linkage[i].nr1_k = m_Nr.p_nr1.linkage_nr1[i].nr1_k;

		linkage[i].nr2_calc_y_k = m_Nr.p_nr2.linkage_nr2[i].nr2_calc_y_k;
		linkage[i].nr2_k = m_Nr.p_nr2.linkage_nr2[i].nr2_k;
		linkage[i].nr2_enable = m_Nr.p_nr2.linkage_nr2[i].nr2_enable;
		linkage[i].y_dpc_enable = m_Nr.p_nr2.linkage_nr2[i].y_dpc_enable;
		linkage[i].y_dpc_th = m_Nr.p_nr2.linkage_nr2[i].y_dpc_th;
		linkage[i].y_black_dpc_enable = m_Nr.p_nr2.linkage_nr2[i].y_black_dpc_enable;
		linkage[i].y_white_dpc_enable = m_Nr.p_nr2.linkage_nr2[i].y_white_dpc_enable;
	}

	m_NrLinkageDlg.SetDataValue((char*)&linkage[0], size);

	if (IDOK == m_NrLinkageDlg.DoModal())
	{
		m_NrLinkageDlg.GetDataValue((char*)&linkage[0], &size);

		for (i=0; i<9; i++)
		{
			m_Nr.p_nr1.linkage_nr1[i].nr1_calc_r_k = linkage[i].nr1_calc_r_k;
			m_Nr.p_nr1.linkage_nr1[i].nr1_calc_g_k = linkage[i].nr1_calc_g_k;
			m_Nr.p_nr1.linkage_nr1[i].nr1_calc_b_k = linkage[i].nr1_calc_b_k;
			m_Nr.p_nr1.linkage_nr1[i].nr1_enable = linkage[i].nr1_enable;
			m_Nr.p_nr1.linkage_nr1[i].nr1_k = linkage[i].nr1_k;

			m_Nr.p_nr2.linkage_nr2[i].nr2_calc_y_k = linkage[i].nr2_calc_y_k;
			m_Nr.p_nr2.linkage_nr2[i].nr2_k = linkage[i].nr2_k;
			m_Nr.p_nr2.linkage_nr2[i].nr2_enable = linkage[i].nr2_enable;
			m_Nr.p_nr2.linkage_nr2[i].y_dpc_enable = linkage[i].y_dpc_enable;
			m_Nr.p_nr2.linkage_nr2[i].y_dpc_th = linkage[i].y_dpc_th;
			m_Nr.p_nr2.linkage_nr2[i].y_black_dpc_enable = linkage[i].y_black_dpc_enable;
			m_Nr.p_nr2.linkage_nr2[i].y_white_dpc_enable = linkage[i].y_white_dpc_enable;
		}
	}
}

void CDenoiseDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_NR))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Nr, sizeof(AK_ISP_INIT_NR));
	file.Close();

	SetDataValue();

	UpdateData(FALSE);
}

void CDenoiseDlg::OnButtonWrite() 
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

	file.Write(&m_Nr, sizeof(AK_ISP_INIT_NR));

	file.Close();
}

void CDenoiseDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_DENOISE, 0);
}

void CDenoiseDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_DENOISE, 0);
}



void CDenoiseDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio33() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio34() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio35() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio36() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio37() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio38() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio39() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::OnRadio92() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDenoiseDlg::Clean(void) 
{
	ZeroMemory(&m_Nr, sizeof(AK_ISP_INIT_NR));
	
	for (int i=0; i<10; i++)
	{
		m_lut_keyPts[i].clear();
		m_lut_keyPts[i].push_back(CPoint(0, CURVE_WINDOW_HEIGHT));
		m_lut_keyPts[i].push_back(CPoint(CURVE_WINDOW_WIDTH, 0));
	}

	SetDataValue();

	UpdateData(FALSE);
}

int CDenoiseDlg::Convert_v2_data(AK_ISP_INIT_NR* struct_new, AK_ISP_INIT_NR_V2* struct_v2) 
{
	int i = 0;
	
	if ((struct_v2 == NULL) || (struct_new == NULL)) return -1;
	
	struct_new->param_id = ISP_NR;
	struct_new->length = sizeof(AK_ISP_INIT_NR);

	memcpy(&struct_new->p_nr1, &struct_v2->p_nr1, sizeof(AK_ISP_NR1_ATTR));


	struct_new->p_nr2.nr2_mode = struct_v2->p_nr2.nr2_mode;
	struct_new->p_nr2.manual_nr2.nr2_enable = struct_v2->p_nr2.manual_nr2.nr2_enable;
	struct_new->p_nr2.manual_nr2.nr2_k = struct_v2->p_nr2.manual_nr2.nr2_k;
	struct_new->p_nr2.manual_nr2.nr2_calc_y_k = struct_v2->p_nr2.manual_nr2.nr2_calc_y_k;
	memcpy(struct_new->p_nr2.manual_nr2.nr2_weight_tbl, struct_v2->p_nr2.manual_nr2.nr2_weight_tbl, 17 * sizeof(T_U16));

	for (i=0; i<9; i++)
	{
		struct_new->p_nr2.linkage_nr2[i].nr2_enable = struct_v2->p_nr2.linkage_nr2[i].nr2_enable;
		struct_new->p_nr2.linkage_nr2[i].nr2_k = struct_v2->p_nr2.linkage_nr2[i].nr2_k;
		struct_new->p_nr2.linkage_nr2[i].nr2_calc_y_k = struct_v2->p_nr2.linkage_nr2[i].nr2_calc_y_k;
		memcpy(struct_new->p_nr2.linkage_nr2[i].nr2_weight_tbl, struct_v2->p_nr2.linkage_nr2[i].nr2_weight_tbl, 17 * sizeof(T_U16));
	}

	
	return 0;
}

