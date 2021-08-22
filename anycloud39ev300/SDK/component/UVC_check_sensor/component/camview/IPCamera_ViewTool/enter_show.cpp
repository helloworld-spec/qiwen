// enter_show.cpp : 实现文件
//

#include "stdafx.h"
#include "enter_show.h"


// Center_show 对话框

extern BOOL g_check_picture_flag;

IMPLEMENT_DYNAMIC(Center_show, CDialog)

Center_show::Center_show(CWnd* pParent /*=NULL*/)
	: CDialog(Center_show::IDD, pParent)
{

}

Center_show::~Center_show()
{
}

void Center_show::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Center_show, CDialog)
	ON_BN_CLICKED(IDOK, &Center_show::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &Center_show::OnBnClickedCancel)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// Center_show 消息处理程序

void Center_show::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	
	
	show_img.DoModal();
	OnOK();
}

void Center_show::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	OnCancel();
}

void Center_show::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnClose();
}
