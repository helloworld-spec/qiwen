// StatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "StatDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStatDlg dialog


CStatDlg::CStatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStatDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStatDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bConnect = 0;
}


void CStatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStatDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStatDlg, CDialog)
	//{{AFX_MSG_MAP(CStatDlg)
	ON_BN_CLICKED(IDC_BUTTON_AWB, OnButtonAwb)
	ON_BN_CLICKED(IDC_BUTTON_AE, OnButtonAe)
	ON_BN_CLICKED(IDC_BUTTON_AF, OnButtonAf)
	ON_BN_CLICKED(IDC_BUTTON_3D, OnButton3d)
	ON_BN_CLICKED(IDC_BUTTON_GET_YUV_IMG, OnButtonGetYuvImg)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_YUV_IMG, OnButtonOpenYuvImg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStatDlg message handlers

void CStatDlg::OnButtonAwb() 
{
	// TODO: Add your control notification handler code here
	if (!m_bConnect) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取参数!\n", 0, 0);
		return;
	}

	m_AwbStatDlg.SetPageID(DIALOG_AWBSTAT);
	m_AwbStatDlg.SetMessageWindow(m_pMessageWnd);

	m_AwbStatDlg.Create(IDD_DIALOG_AWB_STAT, this);
	m_AwbStatDlg.ShowWindow(SW_SHOW);

	//m_AwbStatDlg.DoModal();
}

void CStatDlg::OnButtonAe() 
{
	// TODO: Add your control notification handler code here
	if (!m_bConnect) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取参数!\n", 0, 0);
		return;
	}

	m_AEStatDlg.SetPageID(DIALOG_AESTAT);
	m_AEStatDlg.SetMessageWindow(m_pMessageWnd);

	m_AEStatDlg.Create(IDD_DIALOG_AE_STAT, this);
	m_AEStatDlg.ShowWindow(SW_SHOW);

	//m_AEStatDlg.DoModal();
}

void CStatDlg::OnButtonAf() 
{
	// TODO: Add your control notification handler code here
	if (!m_bConnect) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取参数!\n", 0, 0);
		return;
	}
	
	m_AFStatDlg.SetPageID(DIALOG_AFSTAT);
	m_AFStatDlg.SetMessageWindow(m_pMessageWnd);

	m_AFStatDlg.Create(IDD_DIALOG_AF_STAT, this);
	m_AFStatDlg.ShowWindow(SW_SHOW);

	//m_AFStatDlg.DoModal();
}

void CStatDlg::OnButton3d() 
{
	// TODO: Add your control notification handler code here
	if (!m_bConnect) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取参数!\n", 0, 0);
		return;
	}
	
	m_3DStatDlg.SetPageID(DIALOG_3DSTAT);
	m_3DStatDlg.SetMessageWindow(m_pMessageWnd);

	m_3DStatDlg.Create(IDD_DIALOG_3D_STAT, this);
	m_3DStatDlg.ShowWindow(SW_SHOW);
	

	//m_3DStatDlg.DoModal();
}

void CStatDlg::SetConnectState(bool bConnect) 
{
	// TODO: Add your control notification handler code here

	m_bConnect = bConnect;
}

void CStatDlg::OnButtonGetYuvImg() 
{
	// TODO: Add your control notification handler code here	
	if (!m_bConnect) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取参数!\n", 0, 0);
		return;
	}

	m_ImgDlg.Img_SetConnectState(m_bConnect);
	
	if (NULL == m_pMessageWnd)
	{
		AfxMessageBox("m_pMessageWnd is null\n", MB_OK);
		return;
	}
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_YUV_IMG, DIALOG_STAT, 0);

	m_ImgDlg.SetMessageWindow(m_pMessageWnd);
	//m_ImgDlg.DoModal();
	m_ImgDlg.Create(IDD_DIALOG_IMG, this);
	m_ImgDlg.ShowWindow(SW_SHOW);
}


void CStatDlg::OnButtonOpenYuvImg() 
{
	// TODO: Add your control notification handler code here

	CFileDialog dlg(TRUE, "*.yuv", NULL, OFN_HIDEREADONLY,
		"Data File(*.yuv)|*.yuv|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;

	m_ImgDlg.Img_SetConnectState(m_bConnect);
	m_ImgDlg.SetImgPath((LPSTR)(LPCTSTR)dlg.GetPathName());
	//m_ImgDlg.DoModal();
	m_ImgDlg.SetMessageWindow(m_pMessageWnd);
	m_ImgDlg.Create(IDD_DIALOG_IMG, this);
	m_ImgDlg.ShowWindow(SW_SHOW);
}
