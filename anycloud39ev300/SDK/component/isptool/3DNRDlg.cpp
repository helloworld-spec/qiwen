// 3DNRDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "3DNRDlg.h"
#include "netctrl.h"
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
// C3DNRDlg dialog


C3DNRDlg::C3DNRDlg(CWnd* pParent /*=NULL*/)
	: CDialog(C3DNRDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(C3DNRDlg)
	m_mode = 0;
	m_envi = 0;
	m_tnr_y_Enable = 0;
	m_tnr_uv_Enable = 0;
	//}}AFX_DATA_INIT
}


void C3DNRDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(C3DNRDlg)
	DDX_Control(pDX, IDC_SPIN161, m_t_y_k_cfg);
	DDX_Control(pDX, IDC_SPIN1, m_uvnr_k);
	DDX_Control(pDX, IDC_SPIN3, m_uvlp_k);
	DDX_Control(pDX, IDC_SPIN4, m_t_uv_k);
	DDX_Control(pDX, IDC_SPIN10, m_t_uv_mf_th1);
	DDX_Control(pDX, IDC_SPIN87, m_t_uv_mf_th2);
	DDX_Control(pDX, IDC_SPIN151, m_t_uv_diffth_k1);
	DDX_Control(pDX, IDC_SPIN152, m_t_uv_diffth_k2);
	DDX_Control(pDX, IDC_SPIN153, m_t_uv_mc_k);
	DDX_Control(pDX, IDC_SPIN9, m_ynr_calc_k);
	DDX_Control(pDX, IDC_SPIN150, m_ynr_k);		
	DDX_Control(pDX, IDC_SPIN7, m_ylp_k);	
	DDX_Control(pDX, IDC_SPIN17, m_t_y_th1);
	DDX_Control(pDX, IDC_SPIN154, m_t_y_k1);
	DDX_Control(pDX, IDC_SPIN155, m_t_y_k2);
	DDX_Control(pDX, IDC_SPIN156, m_t_y_kslop);
	DDX_Control(pDX, IDC_SPIN157, m_t_y_mf_th1);
	DDX_Control(pDX, IDC_SPIN158, m_t_y_mf_th2);
	DDX_Control(pDX, IDC_SPIN159, m_t_y_diffth_k1);
	DDX_Control(pDX, IDC_SPIN25, m_t_y_diffth_k2);
	DDX_Control(pDX, IDC_SPIN26, m_t_y_mc_k);

	DDX_Radio(pDX, IDC_RADIO12, m_mode);
	DDX_Radio(pDX, IDC_RADIO1, m_envi);
	DDX_Radio(pDX, IDC_RADIO35, m_tnr_y_Enable);
	DDX_Radio(pDX, IDC_RADIO37, m_tnr_uv_Enable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(C3DNRDlg, CDialog)
	//{{AFX_MSG_MAP(C3DNRDlg)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_RADIO35, OnRadio35)
	ON_BN_CLICKED(IDC_RADIO36, OnRadio36)
	ON_BN_CLICKED(IDC_RADIO37, OnRadio37)
	ON_BN_CLICKED(IDC_RADIO38, OnRadio38)
	ON_BN_CLICKED(IDC_RADIO12, OnRadio12)
	ON_BN_CLICKED(IDC_RADIO13, OnRadio13)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnRadio6)
	ON_BN_CLICKED(IDC_RADIO7, OnRadio7)
	ON_BN_CLICKED(IDC_RADIO8, OnRadio8)
	ON_BN_CLICKED(IDC_RADIO9, OnRadio9)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C3DNRDlg message handlers
BOOL C3DNRDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_3Dnr, sizeof(AK_ISP_INIT_3DNR));

	m_uvnr_k.SetRange(0,15);
	m_uvnr_k.SetPos(0);

	m_uvlp_k.SetRange(0,15);
	m_uvlp_k.SetPos(0);

	m_t_uv_k.SetRange(0,127);
	m_t_uv_k.SetPos(0);

	m_t_uv_mf_th1.SetRange32(0,65535);
	m_t_uv_mf_th1.SetPos(0);

	m_t_uv_mf_th2.SetRange32(0,65535);
	m_t_uv_mf_th2.SetPos(0);

	m_t_uv_diffth_k1.SetRange(0,255);
	m_t_uv_diffth_k1.SetPos(0);

	m_t_uv_diffth_k2.SetRange(0,255);
	m_t_uv_diffth_k2.SetPos(0);

	m_t_uv_mc_k.SetRange(0,31);
	m_t_uv_mc_k.SetPos(0);

	m_ynr_calc_k.SetRange32(0,65535);
	m_ynr_calc_k.SetPos(0);

	m_ynr_k.SetRange(0,15);
	m_ynr_k.SetPos(0);

	m_ylp_k.SetRange(0,15);
	m_ylp_k.SetPos(0);

	m_t_y_th1.SetRange(0,255);
	m_t_y_th1.SetPos(0);

	m_t_y_k1.SetRange(0,127);
	m_t_y_k1.SetPos(0);

	m_t_y_k2.SetRange(0,127);
	m_t_y_k2.SetPos(0);

	m_t_y_kslop.SetRange(0,127);
	m_t_y_kslop.SetPos(0);

	m_t_y_mf_th1.SetRange32(0,65535);
	m_t_y_mf_th1.SetPos(0);

	m_t_y_mf_th2.SetRange32(0,65535);
	m_t_y_mf_th2.SetPos(0);

	m_t_y_diffth_k1.SetRange(0,255);
	m_t_y_diffth_k1.SetPos(0);

	m_t_y_diffth_k2.SetRange(0,255);
	m_t_y_diffth_k2.SetPos(0);

	m_t_y_mc_k.SetRange(0,31);
	m_t_y_mc_k.SetPos(0);

	m_t_y_k_cfg.SetRange(0,127);
	m_t_y_k_cfg.SetPos(0);
	
	EnableLinkageRadio(FALSE);

	return TRUE;
}

BOOL C3DNRDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void C3DNRDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_3DNR))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_3Dnr, sizeof(AK_ISP_INIT_3DNR));
	file.Close();

	SetDataValue(TRUE);

	UpdateData(FALSE);
}

void C3DNRDlg::OnButtonWrite() 
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

	file.Write(&m_3Dnr, sizeof(AK_ISP_INIT_3DNR));

	file.Close();
}

void C3DNRDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_3DNR, 0);
}

void C3DNRDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_3DNR, 0);
}


void C3DNRDlg::CheckData(AK_ISP_INIT_3DNR *p_3dnr)
{
	int i = 0;
	
	if (p_3dnr->p_3d_nr.manual_3d_nr.t_y_mf_th1 == p_3dnr->p_3d_nr.manual_3d_nr.t_y_mf_th2)
	{
		if (p_3dnr->p_3d_nr.manual_3d_nr.t_y_mf_th2 < 65535)
		{
			p_3dnr->p_3d_nr.manual_3d_nr.t_y_mf_th2++;
		}
		else
		{
			p_3dnr->p_3d_nr.manual_3d_nr.t_y_mf_th1--;
		}
	}
	else if (p_3dnr->p_3d_nr.manual_3d_nr.t_y_mf_th1 > p_3dnr->p_3d_nr.manual_3d_nr.t_y_mf_th2)
	{
		int tmp = p_3dnr->p_3d_nr.manual_3d_nr.t_y_mf_th1;

		p_3dnr->p_3d_nr.manual_3d_nr.t_y_mf_th1 = p_3dnr->p_3d_nr.manual_3d_nr.t_y_mf_th2;
		p_3dnr->p_3d_nr.manual_3d_nr.t_y_mf_th2 = tmp;
	}
	

	for (i=0; i<9; i++)
	{
		if (p_3dnr->p_3d_nr.linkage_3d_nr[i].t_y_mf_th1 == p_3dnr->p_3d_nr.linkage_3d_nr[i].t_y_mf_th2)
		{
			if (p_3dnr->p_3d_nr.linkage_3d_nr[i].t_y_mf_th2 < 1023)
			{
				p_3dnr->p_3d_nr.linkage_3d_nr[i].t_y_mf_th2++;
			}
			else
			{
				p_3dnr->p_3d_nr.linkage_3d_nr[i].t_y_mf_th1--;
			}
		}
		else if (p_3dnr->p_3d_nr.linkage_3d_nr[i].t_y_mf_th1 > p_3dnr->p_3d_nr.linkage_3d_nr[i].t_y_mf_th2)
		{
			int tmp = p_3dnr->p_3d_nr.linkage_3d_nr[i].t_y_mf_th1;

			p_3dnr->p_3d_nr.linkage_3d_nr[i].t_y_mf_th1 = p_3dnr->p_3d_nr.linkage_3d_nr[i].t_y_mf_th2;
			p_3dnr->p_3d_nr.linkage_3d_nr[i].t_y_mf_th2 = tmp;
		}
	}
}

void C3DNRDlg::GetDataValue(bool bToStruct)
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

	m_3Dnr.param_id = ISP_3DNR;
	m_3Dnr.length = sizeof(AK_ISP_INIT_3DNR);
	
	m_3Dnr.p_3d_nr.isp_3d_nr_mode = (T_U16)m_mode;

	if (0 == mode_tmp)
	{
		m_3Dnr.p_3d_nr.manual_3d_nr.tnr_y_enable = (T_U16)m_tnr_y_Enable;
		m_3Dnr.p_3d_nr.manual_3d_nr.tnr_uv_enable = (T_U16)m_tnr_uv_Enable;

		m_3Dnr.p_3d_nr.manual_3d_nr.uvnr_k = (T_U16)m_uvnr_k.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.uvlp_k = (T_U16)m_uvlp_k.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_k = (T_U16)m_t_uv_k.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th1 = (T_U16)m_t_uv_mf_th1.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th2 = (T_U16)m_t_uv_mf_th2.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k1 = (T_U16)m_t_uv_diffth_k1.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k2 = (T_U16)m_t_uv_diffth_k2.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_mc_k = (T_U16)m_t_uv_mc_k.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.ynr_calc_k = (T_U16)m_ynr_calc_k.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.ynr_k = (T_U16)m_ynr_k.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.ylp_k = (T_U16)m_ylp_k.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_y_th1 = (T_U16)m_t_y_th1.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_y_k1 = (T_U16)m_t_y_k1.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_y_k2 = (T_U16)m_t_y_k2.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_y_kslop = (T_U16)m_t_y_kslop.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_y_mf_th1 = (T_U16)m_t_y_mf_th1.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_y_mf_th2 = (T_U16)m_t_y_mf_th2.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k1 = (T_U16)m_t_y_diffth_k1.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k2 = (T_U16)m_t_y_diffth_k2.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.t_y_mc_k = (T_U16)m_t_y_mc_k.GetPos();
		m_3Dnr.p_3d_nr.manual_3d_nr.tnr_t_y_ex_k_cfg = (T_U16)m_t_y_k_cfg.GetPos();
	}
	else
	{
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].tnr_y_enable = (T_U16)m_tnr_y_Enable;
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].tnr_uv_enable = (T_U16)m_tnr_uv_Enable;

		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].uvnr_k = (T_U16)m_uvnr_k.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].uvlp_k = (T_U16)m_uvlp_k.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_uv_k = (T_U16)m_t_uv_k.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_uv_mf_th1 = (T_U16)m_t_uv_mf_th1.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_uv_mf_th2 = (T_U16)m_t_uv_mf_th2.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_uv_diffth_k1 = (T_U16)m_t_uv_diffth_k1.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_uv_diffth_k2 = (T_U16)m_t_uv_diffth_k2.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_uv_mc_k = (T_U16)m_t_uv_mc_k.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].ynr_calc_k = (T_U16)m_ynr_calc_k.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].ynr_k = (T_U16)m_ynr_k.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].ylp_k = (T_U16)m_ylp_k.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_y_th1 = (T_U16)m_t_y_th1.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_y_k1 = (T_U16)m_t_y_k1.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_y_k2 = (T_U16)m_t_y_k2.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_y_kslop = (T_U16)m_t_y_kslop.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_y_mf_th1 = (T_U16)m_t_y_mf_th1.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_y_mf_th2 = (T_U16)m_t_y_mf_th2.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_y_diffth_k1 = (T_U16)m_t_y_diffth_k1.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_y_diffth_k2 = (T_U16)m_t_y_diffth_k2.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].t_y_mc_k = (T_U16)m_t_y_mc_k.GetPos();
		m_3Dnr.p_3d_nr.linkage_3d_nr[envi_tmp].tnr_t_y_ex_k_cfg = (T_U16)m_t_y_k_cfg.GetPos();
	}


	int i = 0;
	int j = 0;

	for(i=0;i<17;i++)
	{
		m_3Dnr.p_3d_nr.manual_3d_nr.ynr_weight_tbl[i] = (T_U16)calc_nr_weightRange(i<<4, m_3Dnr.p_3d_nr.manual_3d_nr.ynr_calc_k);
	}

	for (j=0; j<9; j++)
	{
		for(i=0;i<17;i++)
		{
			m_3Dnr.p_3d_nr.linkage_3d_nr[j].ynr_weight_tbl[i] = (T_U16)calc_nr_weightRange(i<<4,m_3Dnr.p_3d_nr.linkage_3d_nr[j].ynr_calc_k);
		}
	}

	CheckData(&m_3Dnr);

}

void C3DNRDlg::SetDataValue(bool bFromStruct)
{
	CWnd *pWnd = NULL;
	CString str;
	
	if (bFromStruct)
	{
		m_mode = m_3Dnr.p_3d_nr.isp_3d_nr_mode;
	}
	
	
	if (0 == m_mode)
	{
		m_tnr_y_Enable = m_3Dnr.p_3d_nr.manual_3d_nr.tnr_y_enable;
		m_tnr_uv_Enable = m_3Dnr.p_3d_nr.manual_3d_nr.tnr_uv_enable;

		m_uvnr_k.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.uvnr_k);
		m_uvlp_k.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.uvlp_k);
		m_t_uv_k.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_k);
		
		m_t_uv_mf_th1.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th1);
		pWnd = GetDlgItem(IDC_EDIT5);
		str.Format("%d", m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th1);
		pWnd->SetWindowText(str);
		
		m_t_uv_mf_th2.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th2);
		pWnd = GetDlgItem(IDC_EDIT6);
		str.Format("%d", m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th2);
		pWnd->SetWindowText(str);
		
		m_t_uv_diffth_k1.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k1);
		m_t_uv_diffth_k2.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k2);
		m_t_uv_mc_k.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_uv_mc_k);
		
		m_ynr_calc_k.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.ynr_calc_k);
		pWnd = GetDlgItem(IDC_EDIT2);
		str.Format("%d", m_3Dnr.p_3d_nr.manual_3d_nr.ynr_calc_k);
		pWnd->SetWindowText(str);
	
		m_ynr_k.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.ynr_k);
		m_ylp_k.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.ylp_k);
		m_t_y_th1.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_y_th1);
		m_t_y_k1.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_y_k1);
		m_t_y_k2.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_y_k2);
		m_t_y_kslop.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_y_kslop);
		
		m_t_y_mf_th1.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_y_mf_th1);
		pWnd = GetDlgItem(IDC_EDIT157);
		str.Format("%d", m_3Dnr.p_3d_nr.manual_3d_nr.t_y_mf_th1);
		pWnd->SetWindowText(str);
		
		m_t_y_mf_th2.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_y_mf_th2);
		pWnd = GetDlgItem(IDC_EDIT26);
		str.Format("%d", m_3Dnr.p_3d_nr.manual_3d_nr.t_y_mf_th2);
		pWnd->SetWindowText(str);
		
		m_t_y_diffth_k1.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k1);
		m_t_y_diffth_k2.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k2);
		m_t_y_mc_k.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.t_y_mc_k);
		m_t_y_k_cfg.SetPos(m_3Dnr.p_3d_nr.manual_3d_nr.tnr_t_y_ex_k_cfg);

		EnableLinkageRadio(FALSE);
	}
	else
	{
		m_tnr_y_Enable = m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].tnr_y_enable;
		m_tnr_uv_Enable = m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].tnr_uv_enable;

		m_uvnr_k.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].uvnr_k);
		m_uvlp_k.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].uvlp_k);
		m_t_uv_k.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_uv_k);
		
		m_t_uv_mf_th1.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_uv_mf_th1);
		pWnd = GetDlgItem(IDC_EDIT5);
		str.Format("%d", m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_uv_mf_th1);
		pWnd->SetWindowText(str);
		
		m_t_uv_mf_th2.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_uv_mf_th2);
		pWnd = GetDlgItem(IDC_EDIT6);
		str.Format("%d", m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_uv_mf_th2);
		pWnd->SetWindowText(str);
		
		m_t_uv_diffth_k1.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_uv_diffth_k1);
		m_t_uv_diffth_k2.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_uv_diffth_k2);
		m_t_uv_mc_k.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_uv_mc_k);
		
		m_ynr_calc_k.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].ynr_calc_k);
		pWnd = GetDlgItem(IDC_EDIT2);
		str.Format("%d", m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].ynr_calc_k);
		pWnd->SetWindowText(str);
		
		m_ynr_k.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].ynr_k);
		m_ylp_k.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].ylp_k);
		m_t_y_th1.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_y_th1);
		m_t_y_k1.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_y_k1);
		m_t_y_k2.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_y_k2);
		m_t_y_kslop.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_y_kslop);
		
		m_t_y_mf_th1.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_y_mf_th1);
		pWnd = GetDlgItem(IDC_EDIT157);
		str.Format("%d", m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_y_mf_th1);
		pWnd->SetWindowText(str);
		
		m_t_y_mf_th2.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_y_mf_th2);
		pWnd = GetDlgItem(IDC_EDIT26);
		str.Format("%d", m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_y_mf_th2);
		pWnd->SetWindowText(str);
		
		m_t_y_diffth_k1.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_y_diffth_k1);
		m_t_y_diffth_k2.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_y_diffth_k2);
		m_t_y_mc_k.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].t_y_mc_k);
		m_t_y_k_cfg.SetPos(m_3Dnr.p_3d_nr.linkage_3d_nr[m_envi].tnr_t_y_ex_k_cfg);
		
		EnableLinkageRadio(TRUE);
	}


	((CButton *)GetDlgItem(IDC_RADIO12))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO13))->SetCheck(m_mode);
	((CButton *)GetDlgItem(IDC_RADIO35))->SetCheck(!m_tnr_y_Enable);
	((CButton *)GetDlgItem(IDC_RADIO36))->SetCheck(m_tnr_y_Enable);
	((CButton *)GetDlgItem(IDC_RADIO37))->SetCheck(!m_tnr_uv_Enable);
	((CButton *)GetDlgItem(IDC_RADIO38))->SetCheck(m_tnr_uv_Enable);

}

int C3DNRDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_3DNR))) return -1;

	GetDataValue(TRUE);

	memcpy(pPageInfoSt, &m_3Dnr, sizeof(AK_ISP_INIT_3DNR));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_3DNR);

	return 0;
}


int C3DNRDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_3DNR))) return -1;

	memcpy(&m_3Dnr, pPageInfoSt, sizeof(AK_ISP_INIT_3DNR));

	SetDataValue(TRUE);
	
	return 0;
}

void C3DNRDlg::EnableLinkageRadio(bool bEnable) 
{
	CWnd *pRadio[9];
	
	pRadio[0] = GetDlgItem(IDC_RADIO1);
	pRadio[1] = GetDlgItem(IDC_RADIO2);
	pRadio[2] = GetDlgItem(IDC_RADIO3);
	pRadio[3] = GetDlgItem(IDC_RADIO4);
	pRadio[4] = GetDlgItem(IDC_RADIO5);
	pRadio[5] = GetDlgItem(IDC_RADIO6);
	pRadio[6] = GetDlgItem(IDC_RADIO7);
	pRadio[7] = GetDlgItem(IDC_RADIO8);
	pRadio[8] = GetDlgItem(IDC_RADIO9);

	for (int i=0; i<9; i++)
	{
		pRadio[i]->EnableWindow(bEnable);
	}
	
}


void C3DNRDlg::OnRadio35() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void C3DNRDlg::OnRadio36() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void C3DNRDlg::OnRadio37() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void C3DNRDlg::OnRadio38() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void C3DNRDlg::OnRadio12() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(FALSE);
	UpdateData(FALSE);
}

void C3DNRDlg::OnRadio13() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(TRUE);
	UpdateData(FALSE);
}

void C3DNRDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void C3DNRDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void C3DNRDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void C3DNRDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void C3DNRDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void C3DNRDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void C3DNRDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void C3DNRDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void C3DNRDlg::OnRadio9() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}


void C3DNRDlg::Clean(void) 
{
	ZeroMemory(&m_3Dnr, sizeof(AK_ISP_INIT_3DNR));
	EnableLinkageRadio(FALSE);
	SetDataValue(TRUE);
	UpdateData(FALSE);
}

int C3DNRDlg::Convert_v2_data(AK_ISP_INIT_3DNR* struct_new, AK_ISP_INIT_3DNR_V2* struct_v2) 
{
	int i = 0, j;
	
	if ((struct_v2 == NULL) || (struct_new == NULL)) 
		return -1;

	struct_new->param_id = ISP_3DNR;
	struct_new->length = sizeof(AK_ISP_INIT_3DNR);
	
	
	struct_new->p_3d_nr.isp_3d_nr_mode = struct_v2->p_3d_nr.isp_3d_nr_mode;
	struct_new->p_3d_nr.manual_3d_nr.uv_min_enable = struct_v2->p_3d_nr.manual_3d_nr.uv_min_enable;
	struct_new->p_3d_nr.manual_3d_nr.tnr_y_enable = struct_v2->p_3d_nr.manual_3d_nr.tnr_y_enable;
	struct_new->p_3d_nr.manual_3d_nr.tnr_uv_enable = struct_v2->p_3d_nr.manual_3d_nr.tnr_uv_enable;
	struct_new->p_3d_nr.manual_3d_nr.updata_ref_y = struct_v2->p_3d_nr.manual_3d_nr.updata_ref_y;
	struct_new->p_3d_nr.manual_3d_nr.updata_ref_uv = struct_v2->p_3d_nr.manual_3d_nr.updata_ref_uv;
	struct_new->p_3d_nr.manual_3d_nr.tnr_refFrame_format = struct_v2->p_3d_nr.manual_3d_nr.tnr_refFrame_format;
	struct_new->p_3d_nr.manual_3d_nr.y_2dnr_enable = 1;
	struct_new->p_3d_nr.manual_3d_nr.uv_2dnr_enable = 1;
	struct_new->p_3d_nr.manual_3d_nr.ynr_calc_k = 1000;

	for(j=0; j<17; j++)
		struct_new->p_3d_nr.manual_3d_nr.ynr_weight_tbl[j] = calc_nr_weightRange(j<<4, struct_new->p_3d_nr.manual_3d_nr.ynr_calc_k); 
	
	struct_new->p_3d_nr.manual_3d_nr.ynr_k = 2;
	struct_new->p_3d_nr.manual_3d_nr.ynr_diff_shift = 0;
	struct_new->p_3d_nr.manual_3d_nr.ylp_k = 11;
	struct_new->p_3d_nr.manual_3d_nr.t_y_th1 = 8;
	struct_new->p_3d_nr.manual_3d_nr.t_y_k1 = 120;
	struct_new->p_3d_nr.manual_3d_nr.t_y_k2 = 96;
	struct_new->p_3d_nr.manual_3d_nr.t_y_kslop = 4;
	struct_new->p_3d_nr.manual_3d_nr.t_y_minstep = 2;
	struct_new->p_3d_nr.manual_3d_nr.t_y_mf_th1 = 1000;
	struct_new->p_3d_nr.manual_3d_nr.t_y_mf_th2 = 2000;
	struct_new->p_3d_nr.manual_3d_nr.t_y_diffth_k1 = 64;
	struct_new->p_3d_nr.manual_3d_nr.t_y_diffth_k2 = 24;
	struct_new->p_3d_nr.manual_3d_nr.t_y_diffth_slop = 0;
	struct_new->p_3d_nr.manual_3d_nr.t_y_mc_k = 8;
	struct_new->p_3d_nr.manual_3d_nr.t_y_ac_th = 20;
	struct_new->p_3d_nr.manual_3d_nr.uvnr_k = 8;
	struct_new->p_3d_nr.manual_3d_nr.uvlp_k = 8;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_k = 120;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_minstep = 4;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_mf_th1 = 1000;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_mf_th2 = 2000;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_diffth_k1 = 64;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_diffth_k2 = 32;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_diffth_slop = 0;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_mc_k = 8;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_ac_th = 20;
	struct_new->p_3d_nr.manual_3d_nr.md_th = 10;

	struct_new->p_3d_nr.manual_3d_nr.tnr_t_y_ex_k_cfg = 64;

	for (i=0; i<9; i++)
	{
		struct_new->p_3d_nr.linkage_3d_nr[i].uv_min_enable = struct_v2->p_3d_nr.linkage_3d_nr[i].uv_min_enable;
		struct_new->p_3d_nr.linkage_3d_nr[i].tnr_y_enable = struct_v2->p_3d_nr.linkage_3d_nr[i].tnr_y_enable;
		struct_new->p_3d_nr.linkage_3d_nr[i].tnr_uv_enable = struct_v2->p_3d_nr.linkage_3d_nr[i].tnr_uv_enable;
		struct_new->p_3d_nr.linkage_3d_nr[i].updata_ref_y = struct_v2->p_3d_nr.linkage_3d_nr[i].updata_ref_y;
		struct_new->p_3d_nr.linkage_3d_nr[i].updata_ref_uv = struct_v2->p_3d_nr.linkage_3d_nr[i].updata_ref_uv;
		struct_new->p_3d_nr.linkage_3d_nr[i].tnr_refFrame_format = struct_v2->p_3d_nr.linkage_3d_nr[i].tnr_refFrame_format;
		struct_new->p_3d_nr.linkage_3d_nr[i].y_2dnr_enable = 1;
		struct_new->p_3d_nr.linkage_3d_nr[i].uv_2dnr_enable = 1;
		struct_new->p_3d_nr.linkage_3d_nr[i].ynr_calc_k = 1000;

		for(j=0; j<17; j++)
			struct_new->p_3d_nr.linkage_3d_nr[i].ynr_weight_tbl[j] = calc_nr_weightRange(j<<4, struct_new->p_3d_nr.linkage_3d_nr[i].ynr_calc_k); 
		
		struct_new->p_3d_nr.linkage_3d_nr[i].ynr_k = 2;
		struct_new->p_3d_nr.linkage_3d_nr[i].ynr_diff_shift = 0;
		struct_new->p_3d_nr.linkage_3d_nr[i].ylp_k = 11;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_th1 = 8;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_k1 = 120;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_k2 = 96;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_kslop = 4;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_minstep = 2;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_mf_th1 = 1000;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_mf_th2 = 2000;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_diffth_k1 = 64;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_diffth_k2 = 24;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_diffth_slop = 0;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_mc_k = 8;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_ac_th = 20;
		struct_new->p_3d_nr.linkage_3d_nr[i].uvnr_k = 8;
		struct_new->p_3d_nr.linkage_3d_nr[i].uvlp_k = 8;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_k = 120;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_minstep = 4;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_mf_th1 = 1000;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_mf_th2 = 2000;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k1 = 64;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k2 = 32;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_diffth_slop = 0;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_mc_k = 8;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_ac_th = 20;
		struct_new->p_3d_nr.linkage_3d_nr[i].md_th = 10;
		struct_new->p_3d_nr.linkage_3d_nr[i].tnr_t_y_ex_k_cfg = 64;
	}
	
	return 0;
}

int C3DNRDlg::Convert_v3_data(AK_ISP_INIT_3DNR* struct_new, AK_ISP_INIT_3DNR_V3* struct_v3) 
{
	int i = 0, j;
	
	if ((struct_v3 == NULL) || (struct_new == NULL)) 
		return -1;

	struct_new->param_id = ISP_3DNR;
	struct_new->length = sizeof(AK_ISP_INIT_3DNR);
	
	
	struct_new->p_3d_nr.isp_3d_nr_mode = struct_v3->p_3d_nr.isp_3d_nr_mode;
	struct_new->p_3d_nr.manual_3d_nr.uv_min_enable = struct_v3->p_3d_nr.manual_3d_nr.uv_min_enable;
	struct_new->p_3d_nr.manual_3d_nr.tnr_y_enable = struct_v3->p_3d_nr.manual_3d_nr.tnr_y_enable;
	struct_new->p_3d_nr.manual_3d_nr.tnr_uv_enable = struct_v3->p_3d_nr.manual_3d_nr.tnr_uv_enable;
	struct_new->p_3d_nr.manual_3d_nr.updata_ref_y = struct_v3->p_3d_nr.manual_3d_nr.updata_ref_y;
	struct_new->p_3d_nr.manual_3d_nr.updata_ref_uv = struct_v3->p_3d_nr.manual_3d_nr.updata_ref_uv;
	struct_new->p_3d_nr.manual_3d_nr.tnr_refFrame_format = struct_v3->p_3d_nr.manual_3d_nr.tnr_refFrame_format;
	struct_new->p_3d_nr.manual_3d_nr.y_2dnr_enable = struct_v3->p_3d_nr.manual_3d_nr.y_2dnr_enable;
	struct_new->p_3d_nr.manual_3d_nr.uv_2dnr_enable = struct_v3->p_3d_nr.manual_3d_nr.uv_2dnr_enable;
	struct_new->p_3d_nr.manual_3d_nr.ynr_calc_k = struct_v3->p_3d_nr.manual_3d_nr.ynr_calc_k;

	for(j=0; j<17; j++)
		struct_new->p_3d_nr.manual_3d_nr.ynr_weight_tbl[j] = calc_nr_weightRange(j<<4, struct_new->p_3d_nr.manual_3d_nr.ynr_calc_k); 
	
	struct_new->p_3d_nr.manual_3d_nr.ynr_k = struct_v3->p_3d_nr.manual_3d_nr.ynr_k;
	struct_new->p_3d_nr.manual_3d_nr.ynr_diff_shift = struct_v3->p_3d_nr.manual_3d_nr.ynr_diff_shift;
	struct_new->p_3d_nr.manual_3d_nr.ylp_k = struct_v3->p_3d_nr.manual_3d_nr.ylp_k;
	struct_new->p_3d_nr.manual_3d_nr.t_y_th1 = struct_v3->p_3d_nr.manual_3d_nr.t_y_th1;
	struct_new->p_3d_nr.manual_3d_nr.t_y_k1 = struct_v3->p_3d_nr.manual_3d_nr.t_y_k1;
	struct_new->p_3d_nr.manual_3d_nr.t_y_k2 = struct_v3->p_3d_nr.manual_3d_nr.t_y_k2;
	struct_new->p_3d_nr.manual_3d_nr.t_y_kslop = struct_v3->p_3d_nr.manual_3d_nr.t_y_kslop;
	struct_new->p_3d_nr.manual_3d_nr.t_y_minstep = struct_v3->p_3d_nr.manual_3d_nr.t_y_minstep;
	struct_new->p_3d_nr.manual_3d_nr.t_y_mf_th1 = struct_v3->p_3d_nr.manual_3d_nr.t_y_mf_th1;
	struct_new->p_3d_nr.manual_3d_nr.t_y_mf_th2 = struct_v3->p_3d_nr.manual_3d_nr.t_y_mf_th2;
	struct_new->p_3d_nr.manual_3d_nr.t_y_diffth_k1 = struct_v3->p_3d_nr.manual_3d_nr.t_y_diffth_k1;
	struct_new->p_3d_nr.manual_3d_nr.t_y_diffth_k2 = struct_v3->p_3d_nr.manual_3d_nr.t_y_diffth_k2;
	struct_new->p_3d_nr.manual_3d_nr.t_y_diffth_slop = struct_v3->p_3d_nr.manual_3d_nr.t_y_diffth_slop;
	struct_new->p_3d_nr.manual_3d_nr.t_y_mc_k = struct_v3->p_3d_nr.manual_3d_nr.t_y_mc_k;
	struct_new->p_3d_nr.manual_3d_nr.t_y_ac_th = struct_v3->p_3d_nr.manual_3d_nr.t_y_ac_th;
	struct_new->p_3d_nr.manual_3d_nr.uvnr_k = struct_v3->p_3d_nr.manual_3d_nr.uvnr_k;
	struct_new->p_3d_nr.manual_3d_nr.uvlp_k = struct_v3->p_3d_nr.manual_3d_nr.uvlp_k;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_k = struct_v3->p_3d_nr.manual_3d_nr.t_uv_k;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_minstep = struct_v3->p_3d_nr.manual_3d_nr.t_uv_minstep;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_mf_th1 = struct_v3->p_3d_nr.manual_3d_nr.t_uv_mf_th1;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_mf_th2 = struct_v3->p_3d_nr.manual_3d_nr.t_uv_mf_th2;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_diffth_k1 = struct_v3->p_3d_nr.manual_3d_nr.t_uv_diffth_k1;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_diffth_k2 = struct_v3->p_3d_nr.manual_3d_nr.t_uv_diffth_k2;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_diffth_slop = struct_v3->p_3d_nr.manual_3d_nr.t_uv_diffth_slop;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_mc_k = struct_v3->p_3d_nr.manual_3d_nr.t_uv_mc_k;
	struct_new->p_3d_nr.manual_3d_nr.t_uv_ac_th = struct_v3->p_3d_nr.manual_3d_nr.t_uv_ac_th;
	struct_new->p_3d_nr.manual_3d_nr.md_th = struct_v3->p_3d_nr.manual_3d_nr.md_th;


	struct_new->p_3d_nr.manual_3d_nr.tnr_t_y_ex_k_cfg = 0;


	for (i=0; i<9; i++)
	{
		struct_new->p_3d_nr.linkage_3d_nr[i].uv_min_enable = struct_v3->p_3d_nr.linkage_3d_nr[i].uv_min_enable;
		struct_new->p_3d_nr.linkage_3d_nr[i].tnr_y_enable = struct_v3->p_3d_nr.linkage_3d_nr[i].tnr_y_enable;
		struct_new->p_3d_nr.linkage_3d_nr[i].tnr_uv_enable = struct_v3->p_3d_nr.linkage_3d_nr[i].tnr_uv_enable;
		struct_new->p_3d_nr.linkage_3d_nr[i].updata_ref_y = struct_v3->p_3d_nr.linkage_3d_nr[i].updata_ref_y;
		struct_new->p_3d_nr.linkage_3d_nr[i].updata_ref_uv = struct_v3->p_3d_nr.linkage_3d_nr[i].updata_ref_uv;
		struct_new->p_3d_nr.linkage_3d_nr[i].tnr_refFrame_format = struct_v3->p_3d_nr.linkage_3d_nr[i].tnr_refFrame_format;
		struct_new->p_3d_nr.linkage_3d_nr[i].y_2dnr_enable = struct_v3->p_3d_nr.linkage_3d_nr[i].y_2dnr_enable;
		struct_new->p_3d_nr.linkage_3d_nr[i].uv_2dnr_enable = struct_v3->p_3d_nr.linkage_3d_nr[i].uv_2dnr_enable;
		struct_new->p_3d_nr.linkage_3d_nr[i].ynr_calc_k = struct_v3->p_3d_nr.linkage_3d_nr[i].ynr_calc_k;

		for(j=0; j<17; j++)
			struct_new->p_3d_nr.linkage_3d_nr[i].ynr_weight_tbl[j] = calc_nr_weightRange(j<<4, struct_new->p_3d_nr.linkage_3d_nr[i].ynr_calc_k); 
		
		struct_new->p_3d_nr.linkage_3d_nr[i].ynr_k = struct_v3->p_3d_nr.linkage_3d_nr[i].ynr_k;
		struct_new->p_3d_nr.linkage_3d_nr[i].ynr_diff_shift = struct_v3->p_3d_nr.linkage_3d_nr[i].ynr_diff_shift;
		struct_new->p_3d_nr.linkage_3d_nr[i].ylp_k = struct_v3->p_3d_nr.linkage_3d_nr[i].ylp_k;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_th1 = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_th1;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_k1 = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_k1;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_k2 = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_k2;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_kslop = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_kslop;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_minstep = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_minstep;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_mf_th1 = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_mf_th1;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_mf_th2 = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_mf_th2;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_diffth_k1 = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_diffth_k1;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_diffth_k2 = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_diffth_k2;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_diffth_slop = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_diffth_slop;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_mc_k = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_mc_k;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_y_ac_th = struct_v3->p_3d_nr.linkage_3d_nr[i].t_y_ac_th;
		struct_new->p_3d_nr.linkage_3d_nr[i].uvnr_k = struct_v3->p_3d_nr.linkage_3d_nr[i].uvnr_k;
		struct_new->p_3d_nr.linkage_3d_nr[i].uvlp_k = struct_v3->p_3d_nr.linkage_3d_nr[i].uvlp_k;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_k = struct_v3->p_3d_nr.linkage_3d_nr[i].t_uv_k;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_minstep = struct_v3->p_3d_nr.linkage_3d_nr[i].t_uv_minstep;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_mf_th1 = struct_v3->p_3d_nr.linkage_3d_nr[i].t_uv_mf_th1;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_mf_th2 = struct_v3->p_3d_nr.linkage_3d_nr[i].t_uv_mf_th2;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k1 = struct_v3->p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k1;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k2 = struct_v3->p_3d_nr.linkage_3d_nr[i].t_uv_diffth_k2;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_diffth_slop = struct_v3->p_3d_nr.linkage_3d_nr[i].t_uv_diffth_slop;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_mc_k = struct_v3->p_3d_nr.linkage_3d_nr[i].t_uv_mc_k;
		struct_new->p_3d_nr.linkage_3d_nr[i].t_uv_ac_th = struct_v3->p_3d_nr.linkage_3d_nr[i].t_uv_ac_th;
		struct_new->p_3d_nr.linkage_3d_nr[i].md_th = struct_v3->p_3d_nr.linkage_3d_nr[i].md_th;
		struct_new->p_3d_nr.linkage_3d_nr[i].tnr_t_y_ex_k_cfg = 0;
	}
	
	return 0;
}

