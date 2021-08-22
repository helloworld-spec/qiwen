// CCMDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "CCMDlg.h"
#include "NetCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCCMDlg dialog


CCCMDlg::CCCMDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCCMDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCCMDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCCMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCCMDlg)
	DDX_Control(pDX, IDC_SPIN_C33, m_spin_c33);
	DDX_Control(pDX, IDC_SPIN_C32, m_spin_c32);
	DDX_Control(pDX, IDC_SPIN_C31, m_spin_c31);
	DDX_Control(pDX, IDC_SPIN_C23, m_spin_c23);
	DDX_Control(pDX, IDC_SPIN_C22, m_spin_c22);
	DDX_Control(pDX, IDC_SPIN_C21, m_spin_c21);
	DDX_Control(pDX, IDC_SPIN_C13, m_spin_c13);
	DDX_Control(pDX, IDC_SPIN_C12, m_spin_c12);
	DDX_Control(pDX, IDC_SPIN_C11, m_spin_c11);
	DDX_Control(pDX, IDC_SLIDER_C33, m_slider_c33);
	DDX_Control(pDX, IDC_SLIDER_C32, m_slider_c32);
	DDX_Control(pDX, IDC_SLIDER_C31, m_slider_c31);
	DDX_Control(pDX, IDC_SLIDER_C23, m_slider_c23);
	DDX_Control(pDX, IDC_SLIDER_C22, m_slider_c22);
	DDX_Control(pDX, IDC_SLIDER_C21, m_slider_c21);
	DDX_Control(pDX, IDC_SLIDER_C13, m_slider_c13);
	DDX_Control(pDX, IDC_SLIDER_C12, m_slider_c12);
	DDX_Control(pDX, IDC_SLIDER_C11, m_slider_c11);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCCMDlg, CDialog)
	//{{AFX_MSG_MAP(CCCMDlg)
	ON_BN_CLICKED(IDC_BUTTON_LINKAGE_SET, OnButtonLinkageSet)
	ON_BN_CLICKED(IDC_RADIO_MANUAL, OnRadioManual)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO_MANUAL_ENABLE, OnRadioManualEnable)
	ON_BN_CLICKED(IDC_RADIO_MANUAL_DISABLE, OnRadioManualDisable)
	ON_EN_KILLFOCUS(IDC_EDIT_C11, OnKillfocusEditC11)
	ON_EN_KILLFOCUS(IDC_EDIT_C12, OnKillfocusEditC12)
	ON_EN_KILLFOCUS(IDC_EDIT_C13, OnKillfocusEditC13)
	ON_EN_KILLFOCUS(IDC_EDIT_C21, OnKillfocusEditC21)
	ON_EN_KILLFOCUS(IDC_EDIT_C22, OnKillfocusEditC22)
	ON_EN_KILLFOCUS(IDC_EDIT_C23, OnKillfocusEditC23)
	ON_EN_KILLFOCUS(IDC_EDIT_C31, OnKillfocusEditC31)
	ON_EN_KILLFOCUS(IDC_EDIT_C32, OnKillfocusEditC32)
	ON_EN_KILLFOCUS(IDC_EDIT_C33, OnKillfocusEditC33)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_C11, OnCustomdrawSliderC11)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_C12, OnCustomdrawSliderC12)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_C13, OnCustomdrawSliderC13)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_C21, OnCustomdrawSliderC21)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_C22, OnCustomdrawSliderC22)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_C23, OnCustomdrawSliderC23)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_C31, OnCustomdrawSliderC31)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_C32, OnCustomdrawSliderC32)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_C33, OnCustomdrawSliderC33)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_C11, OnDeltaposSpinC11)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_C12, OnDeltaposSpinC12)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_C13, OnDeltaposSpinC13)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_C21, OnDeltaposSpinC21)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_C22, OnDeltaposSpinC22)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_C23, OnDeltaposSpinC23)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_C31, OnDeltaposSpinC31)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_C32, OnDeltaposSpinC32)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_C33, OnDeltaposSpinC33)
	ON_BN_CLICKED(IDC_BUTTON_CCM_READ, OnButtonCcmRead)
	ON_BN_CLICKED(IDC_BUTTON_CCM_SAVE, OnButtonCcmSave)
	ON_BN_CLICKED(IDC_BUTTON_CCM_GET, OnButtonCcmGet)
	ON_BN_CLICKED(IDC_BUTTON_CCM_SET, OnButtonCcmSet)
	ON_BN_CLICKED(IDC_RADIO_LINKAGE, OnRadioLinkage)
	ON_BN_CLICKED(IDC_BUTTON_IMG_CALC, OnButtonImgCalc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCCMDlg message handlers

void CCCMDlg::OnButtonLinkageSet() 
{
	T_U32 i = 0, j = 0, m = 0;

	// TODO: Add your control notification handler code here
	if(m_ImgDlg.export_ccm_flag)
	{
		m_ImgDlg.export_ccm_flag = FALSE;
		//copy 
		if (m_ImgDlg.ccm_type == A_CCM)
		{
			for(i = 0; i < 3; i++)
			{
				for (j = 0; j < 3; j++)
				{
					m_CCM_LinkageDlg.m_Isp_ccm_linkage[0].ccm[i][j] = (T_S16)m_ImgDlg.CCM_Mat[m];
					m++;
				}
			}
		}
		else if(m_ImgDlg.ccm_type == TL84_CCM)
		{
			for(i = 0; i < 3; i++)
			{
				for (j = 0; j < 3; j++)
				{
					m_CCM_LinkageDlg.m_Isp_ccm_linkage[1].ccm[i][j] = (T_S16)m_ImgDlg.CCM_Mat[m];
					m++;
				}
			}
		}
		else if(m_ImgDlg.ccm_type == D50_CCM)
		{
			for(i = 0; i < 3; i++)
			{
				for (j = 0; j < 3; j++)
				{
					m_CCM_LinkageDlg.m_Isp_ccm_linkage[2].ccm[i][j] = (T_S16)m_ImgDlg.CCM_Mat[m];
					m++;
				}
			}
		}
		else if(m_ImgDlg.ccm_type == D65_CCM)
		{
			for(i = 0; i < 3; i++)
			{
				for (j = 0; j < 3; j++)
				{
					m_CCM_LinkageDlg.m_Isp_ccm_linkage[3].ccm[i][j] = (T_S16)m_ImgDlg.CCM_Mat[m];
					m++;
				}
			}
		}
	}
	m_CCM_LinkageDlg.DoModal();
}

void CCCMDlg::OnRadioManual() 
{
	// TODO: Add your control notification handler code here
	BOOL check = FALSE;
	check = ((CButton *)GetDlgItem(IDC_RADIO_MANUAL))->GetCheck();
	if (check)
	{
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_LINKAGE))->SetCheck(0);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_LINKAGE))->SetCheck(1);
	}
}

void CCCMDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCMDlg::OnRadioLinkageEnable() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CCCMDlg::OnRadioLinkageDisable() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CCCMDlg::OnRadioManualEnable() 
{
	// TODO: Add your control notification handler code here
	BOOL check = FALSE;
	check = ((CButton *)GetDlgItem(IDC_RADIO_MANUAL_ENABLE))->GetCheck();
	if (check)
	{
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_ENABLE))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_DISABLE))->SetCheck(0);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_DISABLE))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_ENABLE))->SetCheck(0);
	}
	
		
	//UpdateData(TRUE);
	
}

void CCCMDlg::OnRadioManualDisable() 
{
	// TODO: Add your control notification handler code here
	//UpdateData(TRUE);
	BOOL check = FALSE;
	check = ((CButton *)GetDlgItem(IDC_RADIO_MANUAL_DISABLE))->GetCheck();
	if (check)
	{
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_ENABLE))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_DISABLE))->SetCheck(1);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_DISABLE))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_ENABLE))->SetCheck(1);
	}

}

void CCCMDlg::OnKillfocusEditC11() 
{
	// TODO: Add your control notification handler code here
	m_slider_c11.SetPos((T_S16)m_spin_c11.GetPos());
}

void CCCMDlg::OnKillfocusEditC12() 
{
	// TODO: Add your control notification handler code here
	m_slider_c12.SetPos((T_S16)m_spin_c12.GetPos());
}

void CCCMDlg::OnKillfocusEditC13() 
{
	// TODO: Add your control notification handler code here
	m_slider_c13.SetPos((T_S16)m_spin_c13.GetPos());
}

void CCCMDlg::OnKillfocusEditC21() 
{
	// TODO: Add your control notification handler code here
	m_slider_c21.SetPos((T_S16)m_spin_c21.GetPos());
}

void CCCMDlg::OnKillfocusEditC22() 
{
	// TODO: Add your control notification handler code here
	m_slider_c22.SetPos((T_S16)m_spin_c22.GetPos());
}

void CCCMDlg::OnKillfocusEditC23() 
{
	// TODO: Add your control notification handler code here
	m_slider_c23.SetPos((T_S16)m_spin_c23.GetPos());
}

void CCCMDlg::OnKillfocusEditC31() 
{
	// TODO: Add your control notification handler code here
	m_slider_c31.SetPos((T_S16)m_spin_c31.GetPos());
}

void CCCMDlg::OnKillfocusEditC32() 
{
	// TODO: Add your control notification handler code here
	m_slider_c32.SetPos((T_S16)m_spin_c32.GetPos());
}

void CCCMDlg::OnKillfocusEditC33() 
{
	// TODO: Add your control notification handler code here
	m_slider_c33.SetPos((T_S16)m_spin_c33.GetPos());
}

void CCCMDlg::OnCustomdrawSliderC11(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_c11.SetPos((T_S16)m_slider_c11.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnCustomdrawSliderC12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_c12.SetPos((T_S16)m_slider_c12.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnCustomdrawSliderC13(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_c13.SetPos((T_S16)m_slider_c13.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnCustomdrawSliderC21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_c21.SetPos((T_S16)m_slider_c21.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnCustomdrawSliderC22(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_c22.SetPos((T_S16)m_slider_c22.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnCustomdrawSliderC23(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_c23.SetPos((T_S16)m_slider_c23.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnCustomdrawSliderC31(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_c31.SetPos((T_S16)m_slider_c31.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnCustomdrawSliderC32(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_c32.SetPos((T_S16)m_slider_c32.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnCustomdrawSliderC33(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_spin_c33.SetPos((T_S16)m_slider_c33.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnDeltaposSpinC11(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_c11.SetPos((T_S16)m_spin_c11.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnDeltaposSpinC12(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_c12.SetPos((T_S16)m_spin_c12.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnDeltaposSpinC13(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_c13.SetPos((T_S16)m_spin_c13.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnDeltaposSpinC21(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_c21.SetPos((T_S16)m_spin_c21.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnDeltaposSpinC22(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_c22.SetPos((T_S16)m_spin_c22.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnDeltaposSpinC23(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_c23.SetPos((T_S16)m_spin_c23.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnDeltaposSpinC31(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_c31.SetPos((T_S16)m_spin_c31.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnDeltaposSpinC32(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_c32.SetPos((T_S16)m_spin_c32.GetPos());
	*pResult = 0;
}

void CCCMDlg::OnDeltaposSpinC33(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_c33.SetPos((T_S16)m_spin_c33.GetPos());
	*pResult = 0;
}

void CCCMDlg::GetDataValue(void)
{
	BOOL check = FALSE;
	void *buf = NULL;
	int len = 0;

	UpdateData(TRUE);
	
	m_Isp_ccm.param_id = ISP_CCM;
	m_Isp_ccm.length = sizeof(AK_ISP_INIT_CCM);

	check = ((CButton *)GetDlgItem(IDC_RADIO_LINKAGE))->GetCheck();
	m_Isp_ccm.p_ccm.cc_mode = check;
	
	check = ((CButton *)GetDlgItem(IDC_RADIO_MANUAL_ENABLE))->GetCheck();
	m_Isp_ccm.p_ccm.manual_ccm.cc_enable = check;
	m_Isp_ccm.p_ccm.manual_ccm.ccm[0][0]  = (T_S16)m_spin_c11.GetPos();
	m_Isp_ccm.p_ccm.manual_ccm.ccm[0][1]  = (T_S16)m_spin_c12.GetPos();
	m_Isp_ccm.p_ccm.manual_ccm.ccm[0][2]  = (T_S16)m_spin_c13.GetPos();

	m_Isp_ccm.p_ccm.manual_ccm.ccm[1][0]  = (T_S16)m_spin_c21.GetPos();
	m_Isp_ccm.p_ccm.manual_ccm.ccm[1][1]  = (T_S16)m_spin_c22.GetPos();
	m_Isp_ccm.p_ccm.manual_ccm.ccm[1][2]  = (T_S16)m_spin_c23.GetPos();

	m_Isp_ccm.p_ccm.manual_ccm.ccm[2][0]  = (T_S16)m_spin_c31.GetPos();
	m_Isp_ccm.p_ccm.manual_ccm.ccm[2][1]  = (T_S16)m_spin_c32.GetPos();
	m_Isp_ccm.p_ccm.manual_ccm.ccm[2][2]  = (T_S16)m_spin_c33.GetPos();

	len = sizeof(AK_ISP_CCM)*4;
	m_CCM_LinkageDlg.GetPageInfoSt_CCMlinkage(m_Isp_ccm.p_ccm.ccm, len);

}

void CCCMDlg::SetDataValue(void)
{
	if (m_Isp_ccm.p_ccm.cc_mode)
	{
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_LINKAGE))->SetCheck(1);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_LINKAGE))->SetCheck(0);
	}

	if (m_Isp_ccm.p_ccm.manual_ccm.cc_enable)
	{
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_ENABLE))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_DISABLE))->SetCheck(0);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_ENABLE))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL_DISABLE))->SetCheck(1);
	}
	
	m_spin_c11.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[0][0]);
	m_slider_c11.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[0][0]);
	m_spin_c12.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[0][1]);
	m_slider_c12.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[0][1]);
	m_spin_c13.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[0][2]);
	m_slider_c13.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[0][2]);

	m_spin_c21.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[1][0]);
	m_slider_c21.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[1][0]);
	m_spin_c22.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[1][1]);
	m_slider_c22.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[1][1]);
	m_spin_c23.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[1][2]);
	m_slider_c23.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[1][2]);

	m_spin_c31.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[2][0]);
	m_slider_c31.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[2][0]);
	m_spin_c32.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[2][1]);
	m_slider_c32.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[2][1]);
	m_spin_c33.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[2][2]);
	m_slider_c33.SetPos(m_Isp_ccm.p_ccm.manual_ccm.ccm[2][2]);


	m_CCM_LinkageDlg.SetPageInfoSt_CCMlinkage(m_Isp_ccm.p_ccm.ccm, sizeof(AK_ISP_CCM)*4);
	
}

int CCCMDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_AF))) return -1;
	
	GetDataValue();
	
	memcpy(pPageInfoSt, &m_Isp_ccm, sizeof(AK_ISP_INIT_CCM));
	
	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_CCM);
	
	return 0;
}


int CCCMDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_CCM))) return -1;
	
	memcpy(&m_Isp_ccm, pPageInfoSt, sizeof(AK_ISP_INIT_CCM));
	
	SetDataValue();
	
	return 0;
}


void CCCMDlg::OnButtonCcmRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_CCM))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}
	
	file.Read(&m_Isp_ccm, sizeof(AK_ISP_INIT_CCM));
	file.Close();
	
	SetDataValue();
}

void CCCMDlg::OnButtonCcmSave() 
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
	
	file.Write(&m_Isp_ccm, sizeof(AK_ISP_INIT_CCM));
	
	file.Close();
}

void CCCMDlg::OnButtonCcmGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_CCM, 0);
}

void CCCMDlg::OnButtonCcmSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_CCM, 0);
}

BOOL CCCMDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Isp_ccm, sizeof(AK_ISP_INIT_CCM));
	
	// TODO: Add extra initialization here
	m_spin_c11.SetRange(-2048,2047);
	m_spin_c11.SetPos(-2048);
	m_slider_c11.SetRange(-2048,2047);
	m_slider_c11.SetPos(-2048);

	m_spin_c12.SetRange(-2048,2047);
	m_spin_c12.SetPos(-2048);
	m_slider_c12.SetRange(-2048,2047);
	m_slider_c12.SetPos(-2048);

	m_spin_c13.SetRange(-2048,2047);
	m_spin_c13.SetPos(-2048);
	m_slider_c13.SetRange(-2048,2047);
	m_slider_c13.SetPos(-2048);

	m_spin_c21.SetRange(-2048,2047);
	m_spin_c21.SetPos(-2048);
	m_slider_c21.SetRange(-2048,2047);
	m_slider_c21.SetPos(-2048);
	
	m_spin_c22.SetRange(-2048,2047);
	m_spin_c22.SetPos(-2048);
	m_slider_c22.SetRange(-2048,2047);
	m_slider_c22.SetPos(-2048);
	
	m_spin_c23.SetRange(-2048,2047);
	m_spin_c23.SetPos(-2048);
	m_slider_c23.SetRange(-2048,2047);
	m_slider_c23.SetPos(-2048);

	m_spin_c31.SetRange(-2048,2047);
	m_spin_c31.SetPos(-2048);
	m_slider_c31.SetRange(-2048,2047);
	m_slider_c31.SetPos(-2048);
	
	m_spin_c32.SetRange(-2048,2047);
	m_spin_c32.SetPos(-2048);
	m_slider_c32.SetRange(-2048,2047);
	m_slider_c32.SetPos(-2048);
	
	m_spin_c33.SetRange(-2048,2047);
	m_spin_c33.SetPos(-2048);
	m_slider_c33.SetRange(-2048,2047);
	m_slider_c33.SetPos(-2048);

	SetDataValue();
	m_ImgDlg.export_ccm_flag = FALSE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CCCMDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CCCMDlg::OnRadioLinkage() 
{
	// TODO: Add your control notification handler code here
	BOOL check = FALSE;
	check = ((CButton *)GetDlgItem(IDC_RADIO_LINKAGE))->GetCheck();
	if (check)
	{
		((CButton *)GetDlgItem(IDC_RADIO_LINKAGE))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL))->SetCheck(0);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_LINKAGE))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_MANUAL))->SetCheck(1);
	}
}


void CCCMDlg::Clean(void) 
{
	ZeroMemory(&m_Isp_ccm, sizeof(AK_ISP_INIT_CCM));
	SetDataValue();
	UpdateData(FALSE);
}
void CCCMDlg::OnButtonImgCalc() 
{
	// TODO: Add your control notification handler code here
	m_ImgDlg.SetMessageWindow(m_pMessageWnd);
	m_ImgDlg.Img_SetConnectState(m_bConnect);
	
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_RGB_GAMMA, DIALOG_CCM, 0);

	m_ImgDlg.DoModal();
}

void CCCMDlg::SetConnectState(bool bConnect) 
{
	// TODO: Add your control notification handler code here

	m_bConnect = bConnect;
}


int CCCMDlg::Convert_v2_data(AK_ISP_INIT_CCM* struct_new, AK_ISP_INIT_CCM_V2* struct_v2) 
{
	int i = 0;
	
	if ((struct_v2 == NULL) || (struct_new == NULL)) return -1;
	
	struct_new->param_id = ISP_CCM;
	struct_new->length = sizeof(AK_ISP_INIT_CCM);


	struct_new->p_ccm.cc_mode = struct_v2->p_ccm.cc_mode;
	struct_new->p_ccm.manual_ccm.cc_enable = struct_v2->p_ccm.manual_ccm.cc_enable;
	struct_new->p_ccm.manual_ccm.cc_cnoise_yth = 0;
	struct_new->p_ccm.manual_ccm.cc_cnoise_gain = 255;
	struct_new->p_ccm.manual_ccm.cc_cnoise_slop = 0;
		
	memcpy(struct_new->p_ccm.manual_ccm.ccm, struct_v2->p_ccm.manual_ccm.ccm, 9 * sizeof(T_S16));

	for (i=0; i<4; i++)
	{
		struct_new->p_ccm.ccm[i].cc_enable = struct_v2->p_ccm.ccm[i].cc_enable;
		struct_new->p_ccm.ccm[i].cc_cnoise_yth = 0;
		struct_new->p_ccm.ccm[i].cc_cnoise_gain = 255;
		struct_new->p_ccm.ccm[i].cc_cnoise_slop = 0;
		
		memcpy(struct_new->p_ccm.ccm[i].ccm, struct_v2->p_ccm.ccm[i].ccm, 9 * sizeof(T_S16));
	}

	
	return 0;
}

