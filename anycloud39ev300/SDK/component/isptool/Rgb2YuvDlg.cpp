// Rgb2YuvDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "Rgb2YuvDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRgb2YuvDlg dialog


CRgb2YuvDlg::CRgb2YuvDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRgb2YuvDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRgb2YuvDlg)
	m_mode = 0;
	//}}AFX_DATA_INIT
}


void CRgb2YuvDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRgb2YuvDlg)
	DDX_Radio(pDX, IDC_RADIO1, m_mode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRgb2YuvDlg, CDialog)
	//{{AFX_MSG_MAP(CRgb2YuvDlg)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRgb2YuvDlg message handlers

BOOL CRgb2YuvDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Rgb2Yuv, sizeof(AK_ISP_INIT_RGB2YUV));


	return TRUE;
}

void CRgb2YuvDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_RGB2YUV))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Rgb2Yuv, sizeof(AK_ISP_INIT_RGB2YUV));
	file.Close();

	SetDataValue();

	UpdateData(FALSE);
}

void CRgb2YuvDlg::OnButtonWrite() 
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

	file.Write(&m_Rgb2Yuv, sizeof(AK_ISP_INIT_RGB2YUV));

	file.Close();
}

void CRgb2YuvDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_RGB2YUV, 0);
}

void CRgb2YuvDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_RGB2YUV, 0);
}

void CRgb2YuvDlg::GetDataValue(void)
{
	UpdateData(TRUE);


	m_Rgb2Yuv.param_id = ISP_RGB2YUV;
	m_Rgb2Yuv.length = sizeof(AK_ISP_INIT_RGB2YUV);
	
	m_Rgb2Yuv.p_rgb2yuv.mode = (T_U16)m_mode;

}

void CRgb2YuvDlg::SetDataValue(void)
{
	m_mode = m_Rgb2Yuv.p_rgb2yuv.mode;

	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(!m_mode);
	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(m_mode);
}

int CRgb2YuvDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_RGB2YUV))) return -1;

	GetDataValue();

	memcpy(pPageInfoSt, &m_Rgb2Yuv, sizeof(AK_ISP_INIT_RGB2YUV));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_RGB2YUV);

	return 0;
}


int CRgb2YuvDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_RGB2YUV))) return -1;

	memcpy(&m_Rgb2Yuv, pPageInfoSt, sizeof(AK_ISP_INIT_RGB2YUV));

	SetDataValue();
	
	return 0;
}

void CRgb2YuvDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CRgb2YuvDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}


void CRgb2YuvDlg::Clean(void) 
{
	ZeroMemory(&m_Rgb2Yuv, sizeof(AK_ISP_INIT_RGB2YUV));
	SetDataValue();
	UpdateData(FALSE);
}