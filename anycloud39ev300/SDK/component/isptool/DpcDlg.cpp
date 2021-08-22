// DpcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "DpcDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDpcDlg dialog


CDpcDlg::CDpcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDpcDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDpcDlg)
	m_Enable = 0;
	m_mode = 0;
	m_envi = 0;
	m_WhiteEnable = 0;
	m_BlackEnable = 0;
	//}}AFX_DATA_INIT
}


void CDpcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDpcDlg)
	DDX_Control(pDX, IDC_SLIDER1, m_ddpc_th_slider);
	DDX_Control(pDX, IDC_SPIN1, m_ddpc_th);
	DDX_Radio(pDX, IDC_RADIO33, m_Enable);
	DDX_Radio(pDX, IDC_RADIO10, m_mode);
	DDX_Radio(pDX, IDC_RADIO1, m_envi);
	DDX_Radio(pDX, IDC_RADIO35, m_WhiteEnable);
	DDX_Radio(pDX, IDC_RADIO37, m_BlackEnable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDpcDlg, CDialog)
	//{{AFX_MSG_MAP(CDpcDlg)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_BN_CLICKED(IDC_RADIO33, OnRadio33)
	ON_BN_CLICKED(IDC_RADIO34, OnRadio34)
	ON_BN_CLICKED(IDC_RADIO10, OnRadio10)
	ON_BN_CLICKED(IDC_RADIO11, OnRadio11)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	ON_BN_CLICKED(IDC_RADIO4, OnRadio4)
	ON_BN_CLICKED(IDC_RADIO5, OnRadio5)
	ON_BN_CLICKED(IDC_RADIO6, OnRadio6)
	ON_BN_CLICKED(IDC_RADIO7, OnRadio7)
	ON_BN_CLICKED(IDC_RADIO8, OnRadio8)
	ON_BN_CLICKED(IDC_RADIO9, OnRadio9)
	ON_BN_CLICKED(IDC_RADIO35, OnRadio35)
	ON_BN_CLICKED(IDC_RADIO36, OnRadio36)
	ON_BN_CLICKED(IDC_RADIO37, OnRadio37)
	ON_BN_CLICKED(IDC_RADIO38, OnRadio38)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDpcDlg message handlers
BOOL CDpcDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Dpc, sizeof(AK_ISP_INIT_DPC));

	m_ddpc_th.SetRange(0,1023);
	m_ddpc_th.SetPos(0);
	m_ddpc_th_slider.SetRange(0,1023);
	m_ddpc_th_slider.SetPos(0);

	EnableLinkageRadio(FALSE);


	return TRUE;
}

BOOL CDpcDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CDpcDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_DPC))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Dpc, sizeof(AK_ISP_INIT_DPC));
	file.Close();

	SetDataValue(TRUE);

	UpdateData(FALSE);
}

void CDpcDlg::OnButtonWrite() 
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

	file.Write(&m_Dpc, sizeof(AK_ISP_INIT_DPC));

	file.Close();
}

void CDpcDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_DPC, 0);
}

void CDpcDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_DPC, 0);
}

void CDpcDlg::EnableLinkageRadio(bool bEnable) 
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

void CDpcDlg::GetDataValue(bool bToStruct)
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


	m_Dpc.param_id = ISP_DPC;
	m_Dpc.length = sizeof(AK_ISP_INIT_DPC);
	

	m_Dpc.p_ddpc.ddpc_mode = (T_U16)m_mode;

	if (0 == mode_tmp)
	{
		m_Dpc.p_ddpc.manual_ddpc.ddpc_enable = (T_U16)m_Enable;

		m_Dpc.p_ddpc.manual_ddpc.ddpc_th = (T_U16)m_ddpc_th.GetPos();

		m_Dpc.p_ddpc.manual_ddpc.white_dpc_enable = (T_U16)m_WhiteEnable;
		m_Dpc.p_ddpc.manual_ddpc.black_dpc_enable = (T_U16)m_BlackEnable;

	}
	else
	{
		m_Dpc.p_ddpc.linkage_ddpc[envi_tmp].ddpc_enable = (T_U16)m_Enable;

		m_Dpc.p_ddpc.linkage_ddpc[envi_tmp].ddpc_th = (T_U16)m_ddpc_th.GetPos();

		m_Dpc.p_ddpc.linkage_ddpc[envi_tmp].white_dpc_enable = (T_U16)m_WhiteEnable;
		m_Dpc.p_ddpc.linkage_ddpc[envi_tmp].black_dpc_enable = (T_U16)m_BlackEnable;
	}

}

void CDpcDlg::SetDataValue(bool bFromStruct)
{
	if (bFromStruct)
	{
		m_mode = m_Dpc.p_ddpc.ddpc_mode;
	}
	
	
	if (0 == m_mode)
	{
		m_Enable = m_Dpc.p_ddpc.manual_ddpc.ddpc_enable;

		m_ddpc_th.SetPos(m_Dpc.p_ddpc.manual_ddpc.ddpc_th);
		m_ddpc_th_slider.SetPos(m_Dpc.p_ddpc.manual_ddpc.ddpc_th);

		m_WhiteEnable = m_Dpc.p_ddpc.manual_ddpc.white_dpc_enable;
		m_BlackEnable = m_Dpc.p_ddpc.manual_ddpc.black_dpc_enable;

		EnableLinkageRadio(FALSE);
	}
	else
	{
		m_Enable = m_Dpc.p_ddpc.linkage_ddpc[m_envi].ddpc_enable;

		m_ddpc_th.SetPos(m_Dpc.p_ddpc.linkage_ddpc[m_envi].ddpc_th);
		m_ddpc_th_slider.SetPos(m_Dpc.p_ddpc.linkage_ddpc[m_envi].ddpc_th);

		m_WhiteEnable = m_Dpc.p_ddpc.linkage_ddpc[m_envi].white_dpc_enable;
		m_BlackEnable = m_Dpc.p_ddpc.linkage_ddpc[m_envi].black_dpc_enable;

		EnableLinkageRadio(TRUE);
	}
	
	((CButton *)GetDlgItem(IDC_RADIO10))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO11))->SetCheck(m_mode);
	((CButton *)GetDlgItem(IDC_RADIO33))->SetCheck(!m_Enable);
	((CButton *)GetDlgItem(IDC_RADIO34))->SetCheck(m_Enable);

	((CButton *)GetDlgItem(IDC_RADIO35))->SetCheck(!m_WhiteEnable);
	((CButton *)GetDlgItem(IDC_RADIO36))->SetCheck(m_WhiteEnable);
	((CButton *)GetDlgItem(IDC_RADIO37))->SetCheck(!m_BlackEnable);
	((CButton *)GetDlgItem(IDC_RADIO38))->SetCheck(m_BlackEnable);
}

int CDpcDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_DPC))) return -1;

	GetDataValue(TRUE);

	memcpy(pPageInfoSt, &m_Dpc, sizeof(AK_ISP_INIT_DPC));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_DPC);

	return 0;
}


int CDpcDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_DPC))) return -1;

	memcpy(&m_Dpc, pPageInfoSt, sizeof(AK_ISP_INIT_DPC));

	SetDataValue(TRUE);
	
	return 0;
}


void CDpcDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_ddpc_th_slider.SetPos(m_ddpc_th.GetPos());
	*pResult = 0;
}

void CDpcDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_ddpc_th_slider.SetPos(m_ddpc_th.GetPos());
}

void CDpcDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_ddpc_th.SetPos(m_ddpc_th_slider.GetPos());
	*pResult = 0;
}


void CDpcDlg::OnRadio33() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CDpcDlg::OnRadio34() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CDpcDlg::OnRadio10() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio11() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(TRUE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio9() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}


void CDpcDlg::OnRadio35() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio36() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio37() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::OnRadio38() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CDpcDlg::Clean(void) 
{
	ZeroMemory(&m_Dpc, sizeof(AK_ISP_INIT_DPC));
	EnableLinkageRadio(FALSE);
	SetDataValue(TRUE);
	UpdateData(FALSE);
}


int CDpcDlg::Convert_v2_data(AK_ISP_INIT_DPC *struct_new, AK_ISP_INIT_DPC_V2* struct_v2) 
{
	int i = 0;
	
	if ((struct_v2 == NULL) || (struct_new == NULL)) return -1;
	
	struct_new->param_id = ISP_DPC;
	struct_new->length = sizeof(AK_ISP_INIT_DPC);


	struct_new->p_ddpc.ddpc_mode = struct_v2->p_ddpc.ddpc_mode;
	struct_new->p_ddpc.manual_ddpc.ddpc_enable = struct_v2->p_ddpc.manual_ddpc.ddpc_enable;
	struct_new->p_ddpc.manual_ddpc.ddpc_th = struct_v2->p_ddpc.manual_ddpc.ddpc_th;
	struct_new->p_ddpc.manual_ddpc.white_dpc_enable = 1;
	struct_new->p_ddpc.manual_ddpc.black_dpc_enable = 1;
		
	for (i=0; i<9; i++)
	{
		struct_new->p_ddpc.linkage_ddpc[i].ddpc_enable = struct_v2->p_ddpc.linkage_ddpc[i].ddpc_enable;
		struct_new->p_ddpc.linkage_ddpc[i].ddpc_th = struct_v2->p_ddpc.linkage_ddpc[i].ddpc_th;
		struct_new->p_ddpc.linkage_ddpc[i].white_dpc_enable = 1;
		struct_new->p_ddpc.linkage_ddpc[i].black_dpc_enable = 1;
	}

	memcpy(&struct_new->p_sdpc, &struct_v2->p_sdpc, sizeof(AK_ISP_SDPC_ATTR));
	
	return 0;
}

