// RightCtrlDlg.cpp : 实现文件

#include "stdafx.h"
#include "Anyka IP Camera.h"
#include "Anyka IP CameraDlg.h"
#include "RightCtrlDlg.h"


// CRightCtrlDlg 对话框

IMPLEMENT_DYNAMIC(CRightCtrlDlg, CDialog)

CRightCtrlDlg::CRightCtrlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRightCtrlDlg::IDD, pParent)
{

}

CRightCtrlDlg::~CRightCtrlDlg()
{
}

void CRightCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_test_config);
	DDX_Control(pDX, IDC_BUTTON_PASS, m_test_pass_btn);
}


BEGIN_MESSAGE_MAP(CRightCtrlDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PASS, &CRightCtrlDlg::OnBnClickedButtonPass)
	ON_BN_CLICKED(IDC_BUTTON_FAILED, &CRightCtrlDlg::OnBnClickedButtonFailed)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CRightCtrlDlg::OnLvnItemchangedList1)
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_STATIC_TEST_CONTENT, &CRightCtrlDlg::OnStnClickedStaticTestContent)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST1, &CRightCtrlDlg::OnNMSetfocusList1)
END_MESSAGE_MAP()


// CRightCtrlDlg 消息处理程序

void CRightCtrlDlg::OnBnClickedButtonPass()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->case_main(TRUE);
}

void CRightCtrlDlg::OnBnClickedButtonFailed()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->case_main(FALSE);
}

void CRightCtrlDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->show_IP_info();
}

BOOL CRightCtrlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_Status.SubclassDlgItem(IDC_STATIC_TEST_CONTENT,this);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

HBRUSH CRightCtrlDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
#if 0
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_STATIC_TEST_TITLE:
		case IDC_STATIC_TEST_CONTENT:
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(0,255,0));
			
	
			return (HBRUSH)GetStockObject(HOLLOW_BRUSH);
		
	}
#endif
	// TODO:  在此更改 DC 的任何属性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CRightCtrlDlg::OnStnClickedStaticTestContent()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CRightCtrlDlg::OnNMSetfocusList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}
