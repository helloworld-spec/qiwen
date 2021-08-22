// EdgeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "EdgeDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEdgeDlg dialog


CEdgeDlg::CEdgeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEdgeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEdgeDlg)
	m_mode = 0;
	m_envi = 0;
	m_Enable = 0;
	m_c_edge_Enable = 0;
	//}}AFX_DATA_INIT
}


void CEdgeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEdgeDlg)
	DDX_Control(pDX, IDC_SLIDER4, m_gain_slop_slider);
	DDX_Control(pDX, IDC_SLIDER3, m_gain_th_slider);
	DDX_Control(pDX, IDC_SLIDER2, m_max_len_slider);
	DDX_Control(pDX, IDC_SLIDER1, m_th_slider);
	DDX_Control(pDX, IDC_SPIN4, m_gain_slop);
	DDX_Control(pDX, IDC_SPIN3, m_gain_th);
	DDX_Control(pDX, IDC_SPIN2, m_max_len);
	DDX_Control(pDX, IDC_SPIN1, m_th);
	DDX_Radio(pDX, IDC_RADIO12, m_mode);
	DDX_Radio(pDX, IDC_RADIO1, m_envi);
	DDX_Radio(pDX, IDC_RADIO33, m_Enable);
	DDX_Radio(pDX, IDC_RADIO35, m_c_edge_Enable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEdgeDlg, CDialog)
	//{{AFX_MSG_MAP(CEdgeDlg)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, OnCustomdrawSlider1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, OnDeltaposSpin2)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnKillfocusEdit2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER2, OnCustomdrawSlider2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, OnDeltaposSpin3)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnKillfocusEdit3)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER3, OnCustomdrawSlider3)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN4, OnDeltaposSpin4)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnKillfocusEdit4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER4, OnCustomdrawSlider4)
	ON_BN_CLICKED(IDC_RADIO33, OnRadio33)
	ON_BN_CLICKED(IDC_RADIO34, OnRadio34)
	ON_BN_CLICKED(IDC_RADIO35, OnRadio35)
	ON_BN_CLICKED(IDC_RADIO36, OnRadio36)
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
// CEdgeDlg message handlers
BOOL CEdgeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Edge, sizeof(AK_ISP_INIT_EDGE));

	m_th.SetRange(0,63);
	m_th.SetPos(0);
	m_th_slider.SetRange(0,63);
	m_th_slider.SetPos(0);

	m_max_len.SetRange(0,31);
	m_max_len.SetPos(0);
	m_max_len_slider.SetRange(0,31);
	m_max_len_slider.SetPos(0);

	m_gain_th.SetRange(0,31);
	m_gain_th.SetPos(0);
	m_gain_th_slider.SetRange(0,31);
	m_gain_th_slider.SetPos(0);

	m_gain_slop.SetRange(0,127);
	m_gain_slop.SetPos(0);
	m_gain_slop_slider.SetRange(0,127);
	m_gain_slop_slider.SetPos(0);

	EnableLinkageRadio(FALSE);

	return TRUE;
}

void CEdgeDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_EDGE))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Edge, sizeof(AK_ISP_INIT_EDGE));
	file.Close();

	SetDataValue(TRUE);

	UpdateData(FALSE);
}

void CEdgeDlg::OnButtonWrite() 
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

	file.Write(&m_Edge, sizeof(AK_ISP_INIT_EDGE));

	file.Close();
}

void CEdgeDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_EDGE, 0);
}

void CEdgeDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_EDGE, 0);
}

void CEdgeDlg::GetDataValue(bool bToStruct)
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


	//m_Edge.param_id = ISP_EDGE;
	m_Edge.length = sizeof(AK_ISP_INIT_EDGE);
	
	m_Edge.p_edge_attr.edge_mode = (T_U16)m_mode;

	if (0 == mode_tmp)
	{
		m_Edge.p_edge_attr.manual_edge.enable = (T_U16)m_Enable;
		m_Edge.p_edge_attr.manual_edge.c_edge_enable = (T_U16)m_c_edge_Enable;

		m_Edge.p_edge_attr.manual_edge.edge_th = (T_U16)m_th.GetPos();
		m_Edge.p_edge_attr.manual_edge.edge_max_len = (T_U16)m_max_len.GetPos();
		m_Edge.p_edge_attr.manual_edge.edge_gain_th = (T_U16)m_gain_th.GetPos();
		m_Edge.p_edge_attr.manual_edge.edge_gain_slop = (T_U16)m_gain_slop.GetPos();
	}
	else
	{
		m_Edge.p_edge_attr.linkage_edge[envi_tmp].enable = (T_U16)m_Enable;
		m_Edge.p_edge_attr.linkage_edge[envi_tmp].c_edge_enable = (T_U16)m_c_edge_Enable;

		m_Edge.p_edge_attr.linkage_edge[envi_tmp].edge_th = (T_U16)m_th.GetPos();
		m_Edge.p_edge_attr.linkage_edge[envi_tmp].edge_max_len = (T_U16)m_max_len.GetPos();
		m_Edge.p_edge_attr.linkage_edge[envi_tmp].edge_gain_th = (T_U16)m_gain_th.GetPos();
		m_Edge.p_edge_attr.linkage_edge[envi_tmp].edge_gain_slop = (T_U16)m_gain_slop.GetPos();
	}

}

void CEdgeDlg::SetDataValue(bool bFromStruct)
{

	if (bFromStruct)
	{
		m_mode = m_Edge.p_edge_attr.edge_mode;
	}
	
	
	if (0 == m_mode)
	{
		m_Enable = m_Edge.p_edge_attr.manual_edge.enable;
		m_c_edge_Enable = m_Edge.p_edge_attr.manual_edge.c_edge_enable;

		m_th.SetPos(m_Edge.p_edge_attr.manual_edge.edge_th);
		m_th_slider.SetPos(m_Edge.p_edge_attr.manual_edge.edge_th);
		m_max_len.SetPos(m_Edge.p_edge_attr.manual_edge.edge_max_len);
		m_max_len_slider.SetPos(m_Edge.p_edge_attr.manual_edge.edge_max_len);
		m_gain_th.SetPos(m_Edge.p_edge_attr.manual_edge.edge_gain_th);
		m_gain_th_slider.SetPos(m_Edge.p_edge_attr.manual_edge.edge_gain_th);
		m_gain_slop.SetPos(m_Edge.p_edge_attr.manual_edge.edge_gain_slop);
		m_gain_slop_slider.SetPos(m_Edge.p_edge_attr.manual_edge.edge_gain_slop);

		EnableLinkageRadio(FALSE);
	}
	else
	{
		m_Enable = m_Edge.p_edge_attr.linkage_edge[m_envi].enable;
		m_c_edge_Enable = m_Edge.p_edge_attr.linkage_edge[m_envi].c_edge_enable;

		m_th.SetPos(m_Edge.p_edge_attr.linkage_edge[m_envi].edge_th);
		m_th_slider.SetPos(m_Edge.p_edge_attr.linkage_edge[m_envi].edge_th);
		m_max_len.SetPos(m_Edge.p_edge_attr.linkage_edge[m_envi].edge_max_len);
		m_max_len_slider.SetPos(m_Edge.p_edge_attr.linkage_edge[m_envi].edge_max_len);
		m_gain_th.SetPos(m_Edge.p_edge_attr.linkage_edge[m_envi].edge_gain_th);
		m_gain_th_slider.SetPos(m_Edge.p_edge_attr.linkage_edge[m_envi].edge_gain_th);
		m_gain_slop.SetPos(m_Edge.p_edge_attr.linkage_edge[m_envi].edge_gain_slop);
		m_gain_slop_slider.SetPos(m_Edge.p_edge_attr.linkage_edge[m_envi].edge_gain_slop);

		EnableLinkageRadio(TRUE);
	}

	((CButton *)GetDlgItem(IDC_RADIO12))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO13))->SetCheck(m_mode);
	((CButton *)GetDlgItem(IDC_RADIO33))->SetCheck(!m_Enable);
	((CButton *)GetDlgItem(IDC_RADIO34))->SetCheck(m_Enable);
	((CButton *)GetDlgItem(IDC_RADIO35))->SetCheck(!m_c_edge_Enable);
	((CButton *)GetDlgItem(IDC_RADIO36))->SetCheck(m_c_edge_Enable);

}

int CEdgeDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_EDGE))) return -1;

	GetDataValue(TRUE);

	memcpy(pPageInfoSt, &m_Edge, sizeof(AK_ISP_INIT_EDGE));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_EDGE);

	return 0;
}


int CEdgeDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_EDGE))) return -1;

	memcpy(&m_Edge, pPageInfoSt, sizeof(AK_ISP_INIT_EDGE));

	SetDataValue(TRUE);
	
	return 0;
}

void CEdgeDlg::EnableLinkageRadio(bool bEnable) 
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


void CEdgeDlg::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_th_slider.SetPos(m_th.GetPos());
	*pResult = 0;
}

void CEdgeDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	m_th_slider.SetPos(m_th.GetPos());
}

void CEdgeDlg::OnCustomdrawSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_th.SetPos(m_th_slider.GetPos());
	*pResult = 0;
}

void CEdgeDlg::OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_max_len_slider.SetPos(m_max_len.GetPos());
	*pResult = 0;
}

void CEdgeDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	m_max_len_slider.SetPos(m_max_len.GetPos());
}

void CEdgeDlg::OnCustomdrawSlider2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_max_len.SetPos(m_max_len_slider.GetPos());
	*pResult = 0;
}

void CEdgeDlg::OnDeltaposSpin3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_gain_th_slider.SetPos(m_gain_th.GetPos());
	*pResult = 0;
}

void CEdgeDlg::OnKillfocusEdit3() 
{
	// TODO: Add your control notification handler code here
	m_gain_th_slider.SetPos(m_gain_th.GetPos());
}

void CEdgeDlg::OnCustomdrawSlider3(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_gain_th.SetPos(m_gain_th_slider.GetPos());
	*pResult = 0;
}

void CEdgeDlg::OnDeltaposSpin4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_gain_slop_slider.SetPos(m_gain_slop.GetPos());
	*pResult = 0;
}

void CEdgeDlg::OnKillfocusEdit4() 
{
	// TODO: Add your control notification handler code here
	m_gain_slop_slider.SetPos(m_gain_slop.GetPos());
}

void CEdgeDlg::OnCustomdrawSlider4(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	m_gain_slop.SetPos(m_gain_slop_slider.GetPos());
	*pResult = 0;
}

void CEdgeDlg::OnRadio33() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CEdgeDlg::OnRadio34() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CEdgeDlg::OnRadio35() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CEdgeDlg::OnRadio36() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CEdgeDlg::OnRadio12() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(FALSE);
	UpdateData(FALSE);
}

void CEdgeDlg::OnRadio13() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	EnableLinkageRadio(TRUE);
	UpdateData(FALSE);
}

void CEdgeDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CEdgeDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CEdgeDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CEdgeDlg::OnRadio4() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CEdgeDlg::OnRadio5() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CEdgeDlg::OnRadio6() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CEdgeDlg::OnRadio7() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CEdgeDlg::OnRadio8() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CEdgeDlg::OnRadio9() 
{
	// TODO: Add your control notification handler code here
	GetDataValue(FALSE);
	SetDataValue(FALSE);
	UpdateData(FALSE);
}

void CEdgeDlg::Clean(void) 
{
	ZeroMemory(&m_Edge, sizeof(AK_ISP_INIT_EDGE));
	EnableLinkageRadio(FALSE);
	SetDataValue(TRUE);
	UpdateData(FALSE);
}