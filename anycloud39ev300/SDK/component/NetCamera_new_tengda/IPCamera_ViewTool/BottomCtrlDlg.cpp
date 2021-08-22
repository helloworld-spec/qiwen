// BottomCtrlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Anyka IP Camera.h"
#include "Anyka IP CameraDlg.h"
#include "BottomCtrlDlg.h"


extern BOOL g_connet_flag ;
extern TCHAR m_connect_ip[MAX_PATH+1];

// CBottomCtrlDlg 对话框

IMPLEMENT_DYNAMIC(CBottomCtrlDlg, CDialog)

CBottomCtrlDlg::CBottomCtrlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBottomCtrlDlg::IDD, pParent)
{

}

CBottomCtrlDlg::~CBottomCtrlDlg()
{
}

void CBottomCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBottomCtrlDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CONFIGURE, &CBottomCtrlDlg::OnBnClickedButtonConfigure)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_UID, &CBottomCtrlDlg::OnBnClickedButtonWriteUid)
	ON_BN_CLICKED(IDC_BUTTON_START, &CBottomCtrlDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CBottomCtrlDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &CBottomCtrlDlg::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CBottomCtrlDlg::OnBnClickedButtonReset)
	ON_EN_CHANGE(IDC_EDIT_PRESENT, &CBottomCtrlDlg::OnEnChangeEditPresent)
END_MESSAGE_MAP()


// CBottomCtrlDlg 消息处理程序
void CBottomCtrlDlg::OnBnClickedButtonConfigure()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->OnBnClickedButtonConfigure();
}

void CBottomCtrlDlg::OnBnClickedButtonWriteUid()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->OnBnClickedButtonWriteUid();

	
}

void CBottomCtrlDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->OnBnClickedButtonStart();
}

void CBottomCtrlDlg::OnBnClickedButtonClose()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->OnBnClickedButtonClose();
}

void CBottomCtrlDlg::OnBnClickedButtonNext()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->OnBnClickedButtonNext();
}

void CBottomCtrlDlg::OnBnClickedButtonReset()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->OnBnClickedButtonReset();
}

void CBottomCtrlDlg::OnEnChangeEditPresent()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}
