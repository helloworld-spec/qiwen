// Zone_weightDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "Zone_weightDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZone_weightDlg dialog


CZone_weightDlg::CZone_weightDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CZone_weightDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CZone_weightDlg)
	//}}AFX_DATA_INIT
	ZeroMemory(m_value, sizeof(AK_ISP_WEIGHT_ATTR));
}


void CZone_weightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CZone_weightDlg)
	//}}AFX_DATA_MAP

	DDX_Text(pDX, IDC_EDIT1, m_value[0][0]);
	DDV_MinMaxInt(pDX, m_value[0][0], 0, 15);
	DDX_Text(pDX, IDC_EDIT2, m_value[0][1]);
	DDV_MinMaxInt(pDX, m_value[0][1], 0, 15);
	DDX_Text(pDX, IDC_EDIT3, m_value[0][2]);
	DDV_MinMaxInt(pDX, m_value[0][2], 0, 15);
	DDX_Text(pDX, IDC_EDIT4, m_value[0][3]);
	DDV_MinMaxInt(pDX, m_value[0][3], 0, 15);
	DDX_Text(pDX, IDC_EDIT5, m_value[0][4]);
	DDV_MinMaxInt(pDX, m_value[0][4], 0, 15);
	DDX_Text(pDX, IDC_EDIT6, m_value[0][5]);
	DDV_MinMaxInt(pDX, m_value[0][5], 0, 15);
	DDX_Text(pDX, IDC_EDIT7, m_value[0][6]);
	DDV_MinMaxInt(pDX, m_value[0][6], 0, 15);
	DDX_Text(pDX, IDC_EDIT8, m_value[0][7]);
	DDV_MinMaxInt(pDX, m_value[0][7], 0, 15);
	DDX_Text(pDX, IDC_EDIT9, m_value[0][8]);
	DDV_MinMaxInt(pDX, m_value[0][8], 0, 15);
	DDX_Text(pDX, IDC_EDIT10, m_value[0][9]);
	DDV_MinMaxInt(pDX, m_value[0][9], 0, 15);
	DDX_Text(pDX, IDC_EDIT11, m_value[0][10]);
	DDV_MinMaxInt(pDX, m_value[0][10], 0, 15);
	DDX_Text(pDX, IDC_EDIT12, m_value[0][11]);
	DDV_MinMaxInt(pDX, m_value[0][11], 0, 15);
	DDX_Text(pDX, IDC_EDIT13, m_value[0][12]);
	DDV_MinMaxInt(pDX, m_value[0][12], 0, 15);
	DDX_Text(pDX, IDC_EDIT14, m_value[0][13]);
	DDV_MinMaxInt(pDX, m_value[0][13], 0, 15);
	DDX_Text(pDX, IDC_EDIT015, m_value[0][14]);
	DDV_MinMaxInt(pDX, m_value[0][14], 0, 15);
	DDX_Text(pDX, IDC_EDIT16, m_value[0][15]);
	DDV_MinMaxInt(pDX, m_value[0][15], 0, 15);

	DDX_Text(pDX, IDC_EDIT17, m_value[1][0]);
	DDV_MinMaxInt(pDX, m_value[1][0], 0, 15);
	DDX_Text(pDX, IDC_EDIT18, m_value[1][1]);
	DDV_MinMaxInt(pDX, m_value[1][1], 0, 15);
	DDX_Text(pDX, IDC_EDIT19, m_value[1][2]);
	DDV_MinMaxInt(pDX, m_value[1][2], 0, 15);
	DDX_Text(pDX, IDC_EDIT20, m_value[1][3]);
	DDV_MinMaxInt(pDX, m_value[1][3], 0, 15);
	DDX_Text(pDX, IDC_EDIT21, m_value[1][4]);
	DDV_MinMaxInt(pDX, m_value[1][4], 0, 15);
	DDX_Text(pDX, IDC_EDIT22, m_value[1][5]);
	DDV_MinMaxInt(pDX, m_value[1][5], 0, 15);
	DDX_Text(pDX, IDC_EDIT23, m_value[1][6]);
	DDV_MinMaxInt(pDX, m_value[1][6], 0, 15);
	DDX_Text(pDX, IDC_EDIT24, m_value[1][7]);
	DDV_MinMaxInt(pDX, m_value[1][7], 0, 15);
	DDX_Text(pDX, IDC_EDIT25, m_value[1][8]);
	DDV_MinMaxInt(pDX, m_value[1][8], 0, 15);
	DDX_Text(pDX, IDC_EDIT26, m_value[1][9]);
	DDV_MinMaxInt(pDX, m_value[1][9], 0, 15);
	DDX_Text(pDX, IDC_EDIT27, m_value[1][10]);
	DDV_MinMaxInt(pDX, m_value[1][10], 0, 15);
	DDX_Text(pDX, IDC_EDIT28, m_value[1][11]);
	DDV_MinMaxInt(pDX, m_value[1][11], 0, 15);
	DDX_Text(pDX, IDC_EDIT29, m_value[1][12]);
	DDV_MinMaxInt(pDX, m_value[1][12], 0, 15);
	DDX_Text(pDX, IDC_EDIT30, m_value[1][13]);
	DDV_MinMaxInt(pDX, m_value[1][13], 0, 15);
	DDX_Text(pDX, IDC_EDIT31, m_value[1][14]);
	DDV_MinMaxInt(pDX, m_value[1][14], 0, 15);
	DDX_Text(pDX, IDC_EDIT32, m_value[1][15]);
	DDV_MinMaxInt(pDX, m_value[1][15], 0, 15);

	DDX_Text(pDX, IDC_EDIT33, m_value[2][0]);
	DDV_MinMaxInt(pDX, m_value[2][0], 0, 15);
	DDX_Text(pDX, IDC_EDIT34, m_value[2][1]);
	DDV_MinMaxInt(pDX, m_value[2][1], 0, 15);
	DDX_Text(pDX, IDC_EDIT35, m_value[2][2]);
	DDV_MinMaxInt(pDX, m_value[2][2], 0, 15);
	DDX_Text(pDX, IDC_EDIT36, m_value[2][3]);
	DDV_MinMaxInt(pDX, m_value[2][3], 0, 15);
	DDX_Text(pDX, IDC_EDIT37, m_value[2][4]);
	DDV_MinMaxInt(pDX, m_value[2][4], 0, 15);
	DDX_Text(pDX, IDC_EDIT38, m_value[2][5]);
	DDV_MinMaxInt(pDX, m_value[2][5], 0, 15);
	DDX_Text(pDX, IDC_EDIT39, m_value[2][6]);
	DDV_MinMaxInt(pDX, m_value[2][6], 0, 15);
	DDX_Text(pDX, IDC_EDIT40, m_value[2][7]);
	DDV_MinMaxInt(pDX, m_value[2][7], 0, 15);
	DDX_Text(pDX, IDC_EDIT41, m_value[2][8]);
	DDV_MinMaxInt(pDX, m_value[2][8], 0, 15);
	DDX_Text(pDX, IDC_EDIT42, m_value[2][9]);
	DDV_MinMaxInt(pDX, m_value[2][9], 0, 15);
	DDX_Text(pDX, IDC_EDIT43, m_value[2][10]);
	DDV_MinMaxInt(pDX, m_value[2][10], 0, 15);
	DDX_Text(pDX, IDC_EDIT44, m_value[2][11]);
	DDV_MinMaxInt(pDX, m_value[2][11], 0, 15);
	DDX_Text(pDX, IDC_EDIT45, m_value[2][12]);
	DDV_MinMaxInt(pDX, m_value[2][12], 0, 15);
	DDX_Text(pDX, IDC_EDIT46, m_value[2][13]);
	DDV_MinMaxInt(pDX, m_value[2][13], 0, 15);
	DDX_Text(pDX, IDC_EDIT47, m_value[2][14]);
	DDV_MinMaxInt(pDX, m_value[2][14], 0, 15);
	DDX_Text(pDX, IDC_EDIT48, m_value[2][15]);
	DDV_MinMaxInt(pDX, m_value[2][15], 0, 15);

	DDX_Text(pDX, IDC_EDIT49, m_value[3][0]);
	DDV_MinMaxInt(pDX, m_value[3][0], 0, 15);
	DDX_Text(pDX, IDC_EDIT50, m_value[3][1]);
	DDV_MinMaxInt(pDX, m_value[3][1], 0, 15);
	DDX_Text(pDX, IDC_EDIT51, m_value[3][2]);
	DDV_MinMaxInt(pDX, m_value[3][2], 0, 15);
	DDX_Text(pDX, IDC_EDIT52, m_value[3][3]);
	DDV_MinMaxInt(pDX, m_value[3][3], 0, 15);
	DDX_Text(pDX, IDC_EDIT53, m_value[3][4]);
	DDV_MinMaxInt(pDX, m_value[3][4], 0, 15);
	DDX_Text(pDX, IDC_EDIT54, m_value[3][5]);
	DDV_MinMaxInt(pDX, m_value[3][5], 0, 15);
	DDX_Text(pDX, IDC_EDIT55, m_value[3][6]);
	DDV_MinMaxInt(pDX, m_value[3][6], 0, 15);
	DDX_Text(pDX, IDC_EDIT56, m_value[3][7]);
	DDV_MinMaxInt(pDX, m_value[3][7], 0, 15);
	DDX_Text(pDX, IDC_EDIT57, m_value[3][8]);
	DDV_MinMaxInt(pDX, m_value[3][8], 0, 15);
	DDX_Text(pDX, IDC_EDIT58, m_value[3][9]);
	DDV_MinMaxInt(pDX, m_value[3][9], 0, 15);
	DDX_Text(pDX, IDC_EDIT59, m_value[3][10]);
	DDV_MinMaxInt(pDX, m_value[3][10], 0, 15);
	DDX_Text(pDX, IDC_EDIT60, m_value[3][11]);
	DDV_MinMaxInt(pDX, m_value[3][11], 0, 15);
	DDX_Text(pDX, IDC_EDIT61, m_value[3][12]);
	DDV_MinMaxInt(pDX, m_value[3][12], 0, 15);
	DDX_Text(pDX, IDC_EDIT62, m_value[3][13]);
	DDV_MinMaxInt(pDX, m_value[3][13], 0, 15);
	DDX_Text(pDX, IDC_EDIT63, m_value[3][14]);
	DDV_MinMaxInt(pDX, m_value[3][14], 0, 15);
	DDX_Text(pDX, IDC_EDIT64, m_value[3][15]);
	DDV_MinMaxInt(pDX, m_value[3][15], 0, 15);

	DDX_Text(pDX, IDC_EDIT65, m_value[4][0]);
	DDV_MinMaxInt(pDX, m_value[4][0], 0, 15);
	DDX_Text(pDX, IDC_EDIT66, m_value[4][1]);
	DDV_MinMaxInt(pDX, m_value[4][1], 0, 15);
	DDX_Text(pDX, IDC_EDIT67, m_value[4][2]);
	DDV_MinMaxInt(pDX, m_value[4][2], 0, 15);
	DDX_Text(pDX, IDC_EDIT68, m_value[4][3]);
	DDV_MinMaxInt(pDX, m_value[4][3], 0, 15);
	DDX_Text(pDX, IDC_EDIT69, m_value[4][4]);
	DDV_MinMaxInt(pDX, m_value[4][4], 0, 15);
	DDX_Text(pDX, IDC_EDIT70, m_value[4][5]);
	DDV_MinMaxInt(pDX, m_value[4][5], 0, 15);
	DDX_Text(pDX, IDC_EDIT71, m_value[4][6]);
	DDV_MinMaxInt(pDX, m_value[4][6], 0, 15);
	DDX_Text(pDX, IDC_EDIT72, m_value[4][7]);
	DDV_MinMaxInt(pDX, m_value[4][7], 0, 15);
	DDX_Text(pDX, IDC_EDIT73, m_value[4][8]);
	DDV_MinMaxInt(pDX, m_value[4][8], 0, 15);
	DDX_Text(pDX, IDC_EDIT74, m_value[4][9]);
	DDV_MinMaxInt(pDX, m_value[4][9], 0, 15);
	DDX_Text(pDX, IDC_EDIT75, m_value[4][10]);
	DDV_MinMaxInt(pDX, m_value[4][10], 0, 15);
	DDX_Text(pDX, IDC_EDIT76, m_value[4][11]);
	DDV_MinMaxInt(pDX, m_value[4][11], 0, 15);
	DDX_Text(pDX, IDC_EDIT77, m_value[4][12]);
	DDV_MinMaxInt(pDX, m_value[4][12], 0, 15);
	DDX_Text(pDX, IDC_EDIT78, m_value[4][13]);
	DDV_MinMaxInt(pDX, m_value[4][13], 0, 15);
	DDX_Text(pDX, IDC_EDIT79, m_value[4][14]);
	DDV_MinMaxInt(pDX, m_value[4][14], 0, 15);
	DDX_Text(pDX, IDC_EDIT80, m_value[4][15]);
	DDV_MinMaxInt(pDX, m_value[4][15], 0, 15);

	DDX_Text(pDX, IDC_EDIT81, m_value[5][0]);
	DDV_MinMaxInt(pDX, m_value[5][0], 0, 15);
	DDX_Text(pDX, IDC_EDIT82, m_value[5][1]);
	DDV_MinMaxInt(pDX, m_value[5][1], 0, 15);
	DDX_Text(pDX, IDC_EDIT83, m_value[5][2]);
	DDV_MinMaxInt(pDX, m_value[5][2], 0, 15);
	DDX_Text(pDX, IDC_EDIT84, m_value[5][3]);
	DDV_MinMaxInt(pDX, m_value[5][3], 0, 15);
	DDX_Text(pDX, IDC_EDIT85, m_value[5][4]);
	DDV_MinMaxInt(pDX, m_value[5][4], 0, 15);
	DDX_Text(pDX, IDC_EDIT86, m_value[5][5]);
	DDV_MinMaxInt(pDX, m_value[5][5], 0, 15);
	DDX_Text(pDX, IDC_EDIT87, m_value[5][6]);
	DDV_MinMaxInt(pDX, m_value[5][6], 0, 15);
	DDX_Text(pDX, IDC_EDIT88, m_value[5][7]);
	DDV_MinMaxInt(pDX, m_value[5][7], 0, 15);
	DDX_Text(pDX, IDC_EDIT89, m_value[5][8]);
	DDV_MinMaxInt(pDX, m_value[5][8], 0, 15);
	DDX_Text(pDX, IDC_EDIT90, m_value[5][9]);
	DDV_MinMaxInt(pDX, m_value[5][9], 0, 15);
	DDX_Text(pDX, IDC_EDIT91, m_value[5][10]);
	DDV_MinMaxInt(pDX, m_value[5][10], 0, 15);
	DDX_Text(pDX, IDC_EDIT92, m_value[5][11]);
	DDV_MinMaxInt(pDX, m_value[5][11], 0, 15);
	DDX_Text(pDX, IDC_EDIT93, m_value[5][12]);
	DDV_MinMaxInt(pDX, m_value[5][12], 0, 15);
	DDX_Text(pDX, IDC_EDIT94, m_value[5][13]);
	DDV_MinMaxInt(pDX, m_value[5][13], 0, 15);
	DDX_Text(pDX, IDC_EDIT95, m_value[5][14]);
	DDV_MinMaxInt(pDX, m_value[5][14], 0, 15);
	DDX_Text(pDX, IDC_EDIT96, m_value[5][15]);
	DDV_MinMaxInt(pDX, m_value[5][15], 0, 15);

	DDX_Text(pDX, IDC_EDIT97, m_value[6][0]);
	DDV_MinMaxInt(pDX, m_value[6][0], 0, 15);
	DDX_Text(pDX, IDC_EDIT98, m_value[6][1]);
	DDV_MinMaxInt(pDX, m_value[6][1], 0, 15);
	DDX_Text(pDX, IDC_EDIT99, m_value[6][2]);
	DDV_MinMaxInt(pDX, m_value[6][2], 0, 15);
	DDX_Text(pDX, IDC_EDIT100, m_value[6][3]);
	DDV_MinMaxInt(pDX, m_value[6][3], 0, 15);
	DDX_Text(pDX, IDC_EDIT101, m_value[6][4]);
	DDV_MinMaxInt(pDX, m_value[6][4], 0, 15);
	DDX_Text(pDX, IDC_EDIT102, m_value[6][5]);
	DDV_MinMaxInt(pDX, m_value[6][5], 0, 15);
	DDX_Text(pDX, IDC_EDIT103, m_value[6][6]);
	DDV_MinMaxInt(pDX, m_value[6][6], 0, 15);
	DDX_Text(pDX, IDC_EDIT104, m_value[6][7]);
	DDV_MinMaxInt(pDX, m_value[6][7], 0, 15);
	DDX_Text(pDX, IDC_EDIT105, m_value[6][8]);
	DDV_MinMaxInt(pDX, m_value[6][8], 0, 15);
	DDX_Text(pDX, IDC_EDIT106, m_value[6][9]);
	DDV_MinMaxInt(pDX, m_value[6][9], 0, 15);
	DDX_Text(pDX, IDC_EDIT107, m_value[6][10]);
	DDV_MinMaxInt(pDX, m_value[6][10], 0, 15);
	DDX_Text(pDX, IDC_EDIT108, m_value[6][11]);
	DDV_MinMaxInt(pDX, m_value[6][11], 0, 15);
	DDX_Text(pDX, IDC_EDIT109, m_value[6][12]);
	DDV_MinMaxInt(pDX, m_value[6][12], 0, 15);
	DDX_Text(pDX, IDC_EDIT110, m_value[6][13]);
	DDV_MinMaxInt(pDX, m_value[6][13], 0, 15);
	DDX_Text(pDX, IDC_EDIT111, m_value[6][14]);
	DDV_MinMaxInt(pDX, m_value[6][14], 0, 15);
	DDX_Text(pDX, IDC_EDIT112, m_value[6][15]);
	DDV_MinMaxInt(pDX, m_value[6][15], 0, 15);

	DDX_Text(pDX, IDC_EDIT113, m_value[7][0]);
	DDV_MinMaxInt(pDX, m_value[7][0], 0, 15);
	DDX_Text(pDX, IDC_EDIT114, m_value[7][1]);
	DDV_MinMaxInt(pDX, m_value[7][1], 0, 15);
	DDX_Text(pDX, IDC_EDIT115, m_value[7][2]);
	DDV_MinMaxInt(pDX, m_value[7][2], 0, 15);
	DDX_Text(pDX, IDC_EDIT116, m_value[7][3]);
	DDV_MinMaxInt(pDX, m_value[7][3], 0, 15);
	DDX_Text(pDX, IDC_EDIT117, m_value[7][4]);
	DDV_MinMaxInt(pDX, m_value[7][4], 0, 15);
	DDX_Text(pDX, IDC_EDIT118, m_value[7][5]);
	DDV_MinMaxInt(pDX, m_value[7][5], 0, 15);
	DDX_Text(pDX, IDC_EDIT119, m_value[7][6]);
	DDV_MinMaxInt(pDX, m_value[7][6], 0, 15);
	DDX_Text(pDX, IDC_EDIT120, m_value[7][7]);
	DDV_MinMaxInt(pDX, m_value[7][7], 0, 15);
	DDX_Text(pDX, IDC_EDIT121, m_value[7][8]);
	DDV_MinMaxInt(pDX, m_value[7][8], 0, 15);
	DDX_Text(pDX, IDC_EDIT122, m_value[7][9]);
	DDV_MinMaxInt(pDX, m_value[7][9], 0, 15);
	DDX_Text(pDX, IDC_EDIT123, m_value[7][10]);
	DDV_MinMaxInt(pDX, m_value[7][10], 0, 15);
	DDX_Text(pDX, IDC_EDIT124, m_value[7][11]);
	DDV_MinMaxInt(pDX, m_value[7][11], 0, 15);
	DDX_Text(pDX, IDC_EDIT125, m_value[7][12]);
	DDV_MinMaxInt(pDX, m_value[7][12], 0, 15);
	DDX_Text(pDX, IDC_EDIT126, m_value[7][13]);
	DDV_MinMaxInt(pDX, m_value[7][13], 0, 15);
	DDX_Text(pDX, IDC_EDIT127, m_value[7][14]);
	DDV_MinMaxInt(pDX, m_value[7][14], 0, 15);
	DDX_Text(pDX, IDC_EDIT128, m_value[7][15]);
	DDV_MinMaxInt(pDX, m_value[7][15], 0, 15);
}


BEGIN_MESSAGE_MAP(CZone_weightDlg, CDialog)
	//{{AFX_MSG_MAP(CZone_weightDlg)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZone_weightDlg message handlers

BOOL CZone_weightDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ZeroMemory(&m_Weight, sizeof(AK_ISP_INIT_WEIGHT));

	return TRUE;
}

BOOL CZone_weightDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CZone_weightDlg::OnButtonRead() 
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

	if (file.GetLength() != sizeof(AK_ISP_INIT_WEIGHT))
	{
		AfxMessageBox("文件数据错误!", MB_OK);
		file.Close();
		return;
	}

	file.Read(&m_Weight, sizeof(AK_ISP_INIT_WEIGHT));
	file.Close();

	SetDataValue();

	UpdateData(FALSE);
}

void CZone_weightDlg::OnButtonWrite() 
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

	file.Write(&m_Weight, sizeof(AK_ISP_INIT_WEIGHT));

	file.Close();
}

void CZone_weightDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_ZONE_WEIGHT, 0);
}

void CZone_weightDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_ZONE_WEIGHT, 0);
}

void CZone_weightDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	m_Weight.param_id = ISP_ZONE_WEIGHT;
	m_Weight.length = sizeof(AK_ISP_INIT_WEIGHT);

	memcpy(&m_Weight.p_weight, m_value, sizeof(AK_ISP_WEIGHT_ATTR));

}

void CZone_weightDlg::SetDataValue(void)
{
	memcpy(m_value, &m_Weight.p_weight, sizeof(AK_ISP_WEIGHT_ATTR));
}

int CZone_weightDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AK_ISP_INIT_WEIGHT))) return -1;

	GetDataValue();

	memcpy(pPageInfoSt, &m_Weight, sizeof(AK_ISP_INIT_WEIGHT));

	nPageID = m_nID;
	nStLen = sizeof(AK_ISP_INIT_WEIGHT);

	return 0;
}


int CZone_weightDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AK_ISP_INIT_WEIGHT))) return -1;

	memcpy(&m_Weight, pPageInfoSt, sizeof(AK_ISP_INIT_WEIGHT));

	SetDataValue();

	m_timer = SetTimer(1, 10, NULL);
	
	return 0;
}

void CZone_weightDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (1 == nIDEvent)
	{
		UpdateData(FALSE);
		KillTimer(m_timer);
	}

	CDialog::OnTimer(nIDEvent);
}


void CZone_weightDlg::Clean(void) 
{
	ZeroMemory(&m_Weight, sizeof(AK_ISP_INIT_WEIGHT));
	SetDataValue();
	UpdateData(FALSE);
}