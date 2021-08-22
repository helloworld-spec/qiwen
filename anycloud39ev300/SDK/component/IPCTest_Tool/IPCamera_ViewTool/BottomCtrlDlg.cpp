// BottomCtrlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Anyka IP Camera.h"
#include "Anyka IP CameraDlg.h"
#include "BottomCtrlDlg.h"
#include "Config_test.h"
#include "WinSpool.h"
#include "afxext.h"
#include "afxdlgs.h"




extern BOOL g_connet_flag ;
extern BOOL no_put_flie_flag ;
extern BOOL g_test_finish_flag;
extern TCHAR m_connect_ip[MAX_PATH+1];
extern BOOL g_pre_flag;
extern BOOL g_start_success_flag;
extern char g_update_flag[UPDATE_MAX_NUM];
extern TCHAR m_connect_uid[MAC_ADDRESS_LEN];
BOOL auto_move_flag = TRUE;
extern CConfig_test g_test_config;
extern UINT current_ip_idex;



UINT g_video_width = 1280;
UINT g_video_height = 720;
BOOL g_start_getdata = FALSE;
unsigned char *g_net_buf = NULL;
BOOL g_extern_start_record_flag = TRUE;

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
	//DDX_Control(pDX, IDC_LIST_WIFI, m_test_wifi);
	DDX_Control(pDX, IDC_LIST_WIFI, m_test_wifi_list);
}


BEGIN_MESSAGE_MAP(CBottomCtrlDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CONFIGURE, &CBottomCtrlDlg::OnBnClickedButtonConfigure)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_UID, &CBottomCtrlDlg::OnBnClickedButtonWriteUid)
	ON_BN_CLICKED(IDC_BUTTON_START, &CBottomCtrlDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CBottomCtrlDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &CBottomCtrlDlg::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CBottomCtrlDlg::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_MOVE, &CBottomCtrlDlg::OnBnClickedButtonAutoMove)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_UID, &CBottomCtrlDlg::OnBnClickedButtonDeleteUid)
	ON_BN_CLICKED(IDC_BUTTON_UP, &CBottomCtrlDlg::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CBottomCtrlDlg::OnBnClickedButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CBottomCtrlDlg::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CBottomCtrlDlg::OnBnClickedButtonRight)
	ON_NOTIFY(BCN_DROPDOWN, IDC_BUTTON_UP, &CBottomCtrlDlg::OnBnDropDownButtonUp)
	ON_BN_KILLFOCUS(IDC_BUTTON_UP, &CBottomCtrlDlg::OnBnKillfocusButtonUp)
	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_EN_CHANGE(IDC_EDIT_DELETE_UID, &CBottomCtrlDlg::OnEnChangeEditDeleteUid)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE_MONITOR, &CBottomCtrlDlg::OnBnClickedButtonCloseMonitor)
	ON_BN_CLICKED(IDC_BUTTON_GET_LISENCE, &CBottomCtrlDlg::OnBnClickedButtonGetLisence)
	ON_BN_CLICKED(IDC_BUTTON1, &CBottomCtrlDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_PRINTF_SET, &CBottomCtrlDlg::OnBnClickedButtonPrintfSet)
	ON_BN_CLICKED(IDC_BUTTON_PRINTF, &CBottomCtrlDlg::OnBnClickedButtonPrintf)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_ALL, &CBottomCtrlDlg::OnBnClickedButtonUpdateAll)
	ON_BN_CLICKED(IDC_BUTTON_IMGAE_RESET, &CBottomCtrlDlg::OnBnClickedButtonImgaeReset)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CBottomCtrlDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_RECORD, &CBottomCtrlDlg::OnBnClickedButtonRecord)
	ON_BN_CLICKED(IDC_BUTTON_WRITE_MAC, &CBottomCtrlDlg::OnBnClickedButtonWriteMac)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_WIFI, &CBottomCtrlDlg::OnLvnItemchangedListWifi)
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
	pP->next_test_flag = FALSE;
	g_test_config.m_test_UID_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	pP->OnBnClickedButtonWriteUid();
	pP->next_test_flag = TRUE;

	
}
BOOL start_test_flag = FALSE;
void CBottomCtrlDlg::OnBnClickedButtonStart()
{
	char ret = 0;
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	if (!start_test_flag)
	{
		
		no_put_flie_flag = false;
		g_test_finish_flag = false;
		g_pre_flag = FALSE;
		pP->next_test_flag = FALSE;
		pP->OnBnClickedButtonStart();
		if (g_start_success_flag)
		{
			start_test_flag = TRUE;
		}
	}
	else
	{
		start_test_flag = FALSE;
		// TODO: 在此添加控件通知处理程序代码
		//pP->CloseServer(0);

		pP->close_test();
	}
	
}

void CBottomCtrlDlg::OnBnClickedButtonClose()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->OnBnClickedButtonClose();
	pP->next_test_flag = TRUE;
}

void CBottomCtrlDlg::OnBnClickedButtonNext()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->OnBnClickedButtonNext();
	pP->next_test_flag = TRUE;
}

void CBottomCtrlDlg::OnBnClickedButtonReset()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->OnBnClickedButtonReset();
	pP->next_test_flag = TRUE;
}

#if 1
void CBottomCtrlDlg::OnBnClickedButtonDeleteUid()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	UINT len = 0,i = 0, j = 0;
	char* dst;//=new char[100];
	CFile fp;
	char read_uid_buf[UID_DATA_LEN] = {0};

	GetDlgItemText(IDC_EDIT_DELETE_UID, str);
	if (str.IsEmpty())
	{
		AfxMessageBox(_T("文本框为空,请输入需要删除的UID"));
		return;
	}
	if (str.GetLength() != g_test_config.uid_download_len)
	{
		AfxMessageBox(_T("UID的长度有错"));
		return;
	}
	dst=(LPSTR)(LPCTSTR)str;

	DWORD faConfig = GetFileAttributes(_T("uid_config.txt")); 
	if(0xFFFFFFFF != faConfig)
	{
		faConfig &= ~FILE_ATTRIBUTE_READONLY;//如果文件是只读，需要设非只读
		faConfig &= ~FILE_ATTRIBUTE_SYSTEM;  //如果文件是系统，那么设非系统
		faConfig &= ~FILE_ATTRIBUTE_TEMPORARY;//如果存在临时，那么也要设非临时
		SetFileAttributes(_T("uid_config.txt"), faConfig);
	}
	else
	{
		AfxMessageBox(_T("uid_config.txt文件不存在"));
		return;
	}

	if (!fp.Open(TEXT("uid_config.txt"),CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite))
	{
		AfxMessageBox(_T("打开uid_config.txt文件失败"));
		return;
	}

	//UINT uid_len = (g_test_config.uid_download_len +2)*2;
	len = fp.GetLength();
	char* buff=new char[len + 4];
	memset(buff,0,len + 4);
	fp.Read(buff,len);
	BOOL exit_flag = FALSE;
	UINT idex = 0;
	UINT idex_uid = 0;

	//进行比较，以找到换行符为标志
	while(1)
	{
		if (len <= idex)
		{
			break;
		}
		idex_uid = 0;
		while(1)
		{
			//找到一行
			if(buff[idex] == '\r' &&  buff[idex + 2] == '\n')
			{
				idex += 4;
				break;
			}
			if (idex_uid <= g_test_config.uid_download_len*2)
			{
				read_uid_buf[idex_uid] = buff[idex];
			}
			
			idex_uid++;
			idex++;
		}

		if (idex_uid == g_test_config.uid_download_len*2)
		{
			//比较二个UID
			for (j = 0; j < g_test_config.uid_download_len*2; j++)
			{
				if (read_uid_buf[j] != dst[j])
				{
					break;
				}
			}
			if (j == g_test_config.uid_download_len*2)
			{
				//UID已存在
				exit_flag = TRUE;
				break;
			}
		}
	}
	
	if (!exit_flag)
	{
		if (buff != NULL)
		{
			delete[] buff;//释放
		}
		AfxMessageBox(_T("UID不存在"));
		fp.Close();
		return;
	}


	memcpy(&buff[idex - idex_uid - 4], &buff[idex], len - idex);
	buff[len - idex_uid - 4] = 0;
	buff[len - idex_uid - 4 + 1] = 0;
	

	fp.Close();

	fp.Remove(TEXT("uid_config.txt"));

	if (!fp.Open(TEXT("uid_config.txt"),CFile::modeCreate|CFile::modeReadWrite))
	{
		if (buff != NULL)
		{
			delete[] buff;//释放
		}
		AfxMessageBox(_T("重新创建uid_config.txt文件失败"));
		return;
	}


	fp.SeekToBegin();
	fp.Write(buff,len - idex_uid - 4);	
	fp.Close();
	if (buff != NULL)
	{
		delete[] buff;//释放
	}

	MessageBox(_T("UID已删除"));
}

#else
void CBottomCtrlDlg::OnBnClickedButtonDeleteUid()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	UINT len = 0,i = 0, j = 0;
	char* dst;//=new char[100];
	CFile fp;

	GetDlgItemText(IDC_EDIT_DELETE_UID, str);
	if (str.IsEmpty())
	{
		AfxMessageBox(_T("文本框为空,请输入需要删除的UID"));
		return;
	}
	if (str.GetLength() != g_test_config.uid_download_len)
	{
		AfxMessageBox(_T("UID的长度有错"));
		return;
	}
	dst=(LPSTR)(LPCTSTR)str;
	/*
	if(dst[0] == 'T' && dst[2] == 'V' && 
		dst[4] == '6' && dst[6] == '0' &&
		dst[8] == '5' && dst[10] == 'F')
	{
	}
	else
	{
		AfxMessageBox(_T("UID错误"));
		return;
	}
	*/

	DWORD faConfig = GetFileAttributes(_T("uid_config.txt")); 
	if(0xFFFFFFFF != faConfig)
	{
		faConfig &= ~FILE_ATTRIBUTE_READONLY;//如果文件是只读，需要设非只读
		faConfig &= ~FILE_ATTRIBUTE_SYSTEM;  //如果文件是系统，那么设非系统
		faConfig &= ~FILE_ATTRIBUTE_TEMPORARY;//如果存在临时，那么也要设非临时
		SetFileAttributes(_T("uid_config.txt"), faConfig);
	}
	else
	{
		AfxMessageBox(_T("uid_config.txt文件不存在"));
		return;
	}

	
	//fp.Open(TEXT("uid_config.txt"),CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite);
	if (!fp.Open(TEXT("uid_config.txt"),CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite))
	{
		AfxMessageBox(_T("打开uid_config.txt文件失败"));
		return;
	}
	
	UINT uid_len = (g_test_config.uid_download_len +2)*2;
	len = fp.GetLength();
	char* buff=new char[len+uid_len];
	memset(buff,0,len+uid_len);
	fp.Read(buff,len);
	BOOL exit_flag = FALSE;
	for(i=0;i<len/uid_len;i++)
	{
		for (j = 0; j < g_test_config.uid_download_len; j++)
		{
			if (buff[i*uid_len + j*2] != dst[j*2])
			{
				break;
			}
		}
		if (j == g_test_config.uid_download_len)
		{
			//UID已存在
			exit_flag = TRUE;
			break;
		}
#if 0
		if(buff[i*44]==dst[0] && buff[i*44+2]==dst[2] && buff[i*44+4]==dst[4] && buff[i*44+6]==dst[6] && 
			buff[i*44+8]==dst[8] && buff[i*44+10]==dst[10] && buff[i*44+12]==dst[12] && buff[i*44+14]==dst[14] && 
			buff[i*44+16]==dst[16] && buff[i*44+18]==dst[18] && buff[i*44+20]==dst[20] && buff[i*44+22]==dst[22] && 
			buff[i*44+24]==dst[24] && buff[i*44+26]==dst[26] && buff[i*44+28]==dst[28] && buff[i*44+30]==dst[30]
			&& buff[i*44+32]==dst[32] && buff[i*44+34]==dst[34] && buff[i*44+36]==dst[36] && buff[i*44+38]==dst[38])
		{
			break;
		}
#endif
	} 

	if (!exit_flag)
	{
		if (buff != NULL)
		{
			delete[] buff;//释放
		}
		AfxMessageBox(_T("UID不存在"));
		fp.Close();
		return;
	}
	
	if(i == (len/uid_len - 1))
	{
	}
	else
	{
		memcpy(&buff[i*uid_len], &buff[(i+1)*uid_len], len - (i + 1)*uid_len);
	}
	fp.Close();

	fp.Remove(TEXT("uid_config.txt"));

	if (!fp.Open(TEXT("uid_config.txt"),CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite))
	{
		if (buff != NULL)
		{
			delete[] buff;//释放
		}
		AfxMessageBox(_T("重新创建uid_config.txt文件失败"));
		return;
	}
	
	
	fp.SeekToBegin();
	fp.Write(buff,len - uid_len);	
	fp.Close();
	if (buff != NULL)
	{
		delete[] buff;//释放
	}
	
	MessageBox(_T("UID已删除"));
}
#endif



void CBottomCtrlDlg::OnBnClickedButtonAutoMove()
{
	// TODO: 在此添加控件通知处理程序代码
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	if (auto_move_flag)
	{
		auto_move_flag = FALSE;
		pP->Auto_move_test(5);
	}
	else
	{
		auto_move_flag = TRUE;
		pP->Auto_move_test(7);
	}
	
	
}

void CBottomCtrlDlg::OnBnClickedButtonUp()
{
	UINT i=0;
	 i = 0;
	// TODO: 在此添加控件通知处理程序代码
}

void CBottomCtrlDlg::OnBnClickedButtonDown()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CBottomCtrlDlg::OnBnClickedButtonLeft()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CBottomCtrlDlg::OnBnClickedButtonRight()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CBottomCtrlDlg::OnBnDropDownButtonUp(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMBCDROPDOWN pDropDown = reinterpret_cast<LPNMBCDROPDOWN>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CBottomCtrlDlg::OnBnKillfocusButtonUp()
{
	// TODO: 在此添加控件通知处理程序代码
	UINT  i = 0;

	i++;
}

BOOL CBottomCtrlDlg::PreTranslateMessage(MSG* pMsg)
{
	int buID=0;
	UINT i = 0;
	CRect rect_up;
	CRect rect_down;
	CRect rect_left;
	CRect rect_right;

	CWnd* pWnd=CWnd::FromHandle(pMsg->hwnd);
	//buID= GetWindowLong(pMsg->hwnd,GWL_ID);//由窗口句柄获得ID号，GetWindowLong为获得窗口的ID号。
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();

	CWnd * pButtonLeft = GetDlgItem(IDC_BUTTON_LEFT);
	CWnd * pButtonRight = GetDlgItem(IDC_BUTTON_RIGHT);
	CWnd * pButtonUp = GetDlgItem(IDC_BUTTON_UP);
	CWnd * pButtonDown = GetDlgItem(IDC_BUTTON_DOWN);

	if(pMsg->message == WM_LBUTTONDOWN) 
	{     
		buID= pWnd->GetDlgCtrlID();

		pButtonLeft->GetWindowRect(rect_left);
		pButtonRight->GetWindowRect(rect_right);
		pButtonUp->GetWindowRect(rect_up);
		pButtonDown->GetWindowRect(rect_down);

		if(rect_up.PtInRect(pMsg->pt) && buID==IDC_BUTTON_UP) //按下
		{  
			pP->Auto_move_test(3);
			//在这里添加单击按下事件的程序
		} 
		else if(rect_down.PtInRect(pMsg->pt) && buID==IDC_BUTTON_DOWN) //按下
		{  
			pP->Auto_move_test(4);
			//在这里添加单击按下事件的程序
		} 

		else if(rect_left.PtInRect(pMsg->pt) && buID==IDC_BUTTON_LEFT) //按下
		{  
			pP->Auto_move_test(1);
			//在这里添加单击按下事件的程序
		} 
		else if(rect_right.PtInRect(pMsg->pt) && buID==IDC_BUTTON_RIGHT) //按下
		{  
			pP->Auto_move_test(2);
			//在这里添加单击按下事件的程序
		} 
	}
	if(pMsg->message==WM_LBUTTONUP) 
	{ 
		pButtonLeft->GetWindowRect(rect_left);
		pButtonRight->GetWindowRect(rect_right);
		pButtonUp->GetWindowRect(rect_up);
		pButtonDown->GetWindowRect(rect_down);

		buID= pWnd->GetDlgCtrlID();
		if(rect_up.PtInRect(pMsg->pt) && buID==IDC_BUTTON_UP) //按下
		{  
			pP->Auto_move_test(7);	
			//在这里添加单击按下事件的程序
		} 
		else if(rect_down.PtInRect(pMsg->pt) && buID==IDC_BUTTON_DOWN) //按下
		{  
			pP->Auto_move_test(7);
			//在这里添加单击按下事件的程序
		} 

		else if(rect_left.PtInRect(pMsg->pt) &&  buID==IDC_BUTTON_LEFT) //按下
		{  
			pP->Auto_move_test(7);
			//在这里添加单击按下事件的程序
		} 
		else if(rect_right.PtInRect(pMsg->pt) && buID==IDC_BUTTON_RIGHT) //按下
		{  
			pP->Auto_move_test(7);
			//在这里添加单击按下事件的程序
		} 
	}
	//FromHandlePermanent(HWND hWnd);
	DeleteTempMap();
	return CDialog::PreTranslateMessage(pMsg);
}

void CBottomCtrlDlg::OnEnChangeEditDeleteUid()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}

#if 0
#define PROCESS_MOVE_OUT(x) \
{\
	KillTimer(TIMER_LONG_PRESS);\
	CameraMovement((x));\
	m_bIsLongPressDone = TRUE;\
	m_nLongPressButtonID = -1;\
	m_bIsLongPress = FALSE;\
}

void CBottomCtrlDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_bIsLongPress) {//长按过程中，鼠标移出云台按钮则进行结束长按的处理。
		CWnd * pButtonLeft = GetDlgItem(IDC_BUTTON_LEFT);
		CWnd * pButtonRight = GetDlgItem(IDC_BUTTON_RIGHT);
		CWnd * pButtonUp = GetDlgItem(IDC_BUTTON_UP);
		CWnd * pButtonDown = GetDlgItem(IDC_BUTTON_DOWN);

		CRect RectLeftWnd, RectRightWnd, RectUpWnd, RectDownWnd;
		pButtonLeft->GetWindowRect(RectLeftWnd);
		ScreenToClient(RectLeftWnd);

		pButtonRight->GetWindowRect(RectRightWnd);
		ScreenToClient(RectRightWnd);

		pButtonUp->GetWindowRect(RectUpWnd);
		ScreenToClient(RectUpWnd);

		pButtonDown->GetWindowRect(RectDownWnd);
		ScreenToClient(RectDownWnd);

		if (m_nLongPressButtonID == IDC_BUTTON_LEFT && !RectLeftWnd.PtInRect(point)) {
			PROCESS_MOVE_OUT(CMT_STEP_LEFT_CONTINUE_STOP);
		}else if (m_nLongPressButtonID == IDC_BUTTON_RIGHT && !RectRightWnd.PtInRect(point)) {
			PROCESS_MOVE_OUT(CMT_STEP_RIGHT_CONTINUE_STOP);
		}else if (m_nLongPressButtonID == IDC_BUTTON_UP && !RectUpWnd.PtInRect(point)) {
			PROCESS_MOVE_OUT(CMT_STEP_UP_CONTINUE_STOP);
		}else if (m_nLongPressButtonID == IDC_BUTTON_DOWN && !RectDownWnd.PtInRect(point)) {
			PROCESS_MOVE_OUT(CMT_STEP_DOWN_CONTINUE_STOP);
		}else {
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}

#endif
void CBottomCtrlDlg::OnBnClickedButtonCloseMonitor()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();

	GetDlgItem(IDC_BUTTON_CLOSE_MONITOR)->EnableWindow(FALSE);
	pP->close_minitor();
	GetDlgItem(IDC_BUTTON_CLOSE_MONITOR)->EnableWindow(TRUE);
}

void CBottomCtrlDlg::OnBnClickedButtonGetLisence()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();

	GetDlgItem(IDC_BUTTON_GET_LISENCE)->EnableWindow(FALSE);
	pP->case_get_lisence();
	GetDlgItem(IDC_BUTTON_GET_LISENCE)->EnableWindow(TRUE);
}


void CBottomCtrlDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CBottomCtrlDlg::OnBnClickedButtonPrintfSet()
{
	// TODO: 在此添加控件通知处理程序代码

	g_test_config.Read_printf_cfg(CONFIG_PRINTF);

	if(printf_set.DoModal() == IDOK)
	{
	}
}
extern int make_img_main(char *str_buf, char * _Filename);
void CBottomCtrlDlg::OnBnClickedButtonPrintf()
{
	CAnykaIPCameraDlg IPCameraDlg;
	UINT j = 0;
	char path_filename[MAX_PATH] = {0};

	
	USES_CONVERSION;

	// TODO: 在此添加控件通知处理程序代码
	if(!g_test_config.Read_printf_cfg(CONFIG_PRINTF))
	{

		AfxMessageBox(_T("请设置打印机"),MB_OK);
		return;
	}

	if(g_test_config.page_num == 0)
	{
		AfxMessageBox(_T("请设置打印数量"),MB_OK);
		return;
	}

	if (g_test_config.is_printf_lab)
	{
		for (j = 0; j < g_test_config.page_num; j++)
		{
			print_LAB_main();
		}
	}
	else
	{
		TCHAR szPath[MAX_PATH];  
		GetModuleFileName(NULL, szPath, MAX_PATH);  
		CString PathName(szPath);  
		//获取exe目录  
		CString PROGRAM_PATH = PathName.Left(PathName.ReverseFind(_T('\\')) + 1);  
		//Debug目录下RecordVoice文件夹中  
		PROGRAM_PATH+=_T("test.bmp");  

		memset(path_filename, 0,MAX_PATH);
		memcpy(path_filename, T2A(PROGRAM_PATH), _tcsclen(PROGRAM_PATH));

		memset(szPath, 0, MAX_PATH *sizeof(TCHAR));
		_tcscpy(szPath, PROGRAM_PATH);



		//获取PID和UID
		CString str;
		char str_buf[MAX_PATH] = {0};
		char *szSourceSring = "http://iot.qq.com/add?pid=1000000000&sn=AKA1110000000000";
		//char *buf_temp = "AKA1110000000001";

		if (_tcsclen(g_test_config.PID) != 10)
		{
			AfxMessageBox(_T("PID长度不等于10，请检查"), MB_OK);
			return;
		}

		memset(str_buf, 0, MAX_PATH);

		if (g_test_config.printf_char_mode == 1)
		{
			memcpy(str_buf, szSourceSring, strlen(szSourceSring));
			memcpy(&str_buf[26], T2A(g_test_config.PID), 10);
			memcpy(&str_buf[40], T2A(m_connect_uid), _tcsclen(m_connect_uid));
			//memcpy(&str_buf[40], buf_temp, strlen(buf_temp));
		} 
		else
		{
			memcpy(str_buf, T2A(g_test_config.m_last_config[current_ip_idex].Current_IP_MAC), _tcsclen(g_test_config.m_last_config[current_ip_idex].Current_IP_MAC));
		}

		//生成二维码
		make_img_main(str_buf, path_filename);

		for (j = 0; j < g_test_config.page_num; j++)
		{
			if (g_test_config.printf_char_mode == 1)
			{
				print_image_main(szPath, m_connect_uid);
			}
			else
			{
				print_image_main(szPath, g_test_config.m_last_config[current_ip_idex].Current_IP_MAC);
			}
			
		
		}
	}
}


void CBottomCtrlDlg::OnBnClickedButtonUpdateAll()
{
	
}



// **********************************************************************   
// ReadFileWithAlloc - allocates memory for and reads contents of a file   
//    
// Params:   
//   szFileName   - NULL terminated string specifying file name   
//   pdwSize      - address of variable to receive file bytes size   
//   ppBytes      - address of pointer which will be allocated and contain file bytes   
//    
// Returns: TRUE for success, FALSE for failure.   
//   
// Notes: Caller is responsible for freeing the memory using GlobalFree()   
//    
BOOL CBottomCtrlDlg::ReadFileWithAlloc( LPTSTR szFileName, LPDWORD pdwSize, LPBYTE *ppBytes )  
{  
	HANDLE      hFile;  
	DWORD       dwBytes;  
	BOOL        bSuccess = FALSE;  

	// Validate pointer parameters   
	if( ( pdwSize == NULL ) || ( ppBytes == NULL ) )  
		return FALSE;  
	// Open the file for reading   
	hFile = CreateFile( szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );  
	if( hFile == INVALID_HANDLE_VALUE )  
	{    
		return FALSE;  
	}  
	// How big is the file?   
	*pdwSize = GetFileSize( hFile, NULL );  
	if( *pdwSize == (DWORD)-1 )  
	{
		return FALSE; 
	}
	else  
	{  
		// Allocate the memory   
		*ppBytes = (LPBYTE)GlobalAlloc( GPTR, *pdwSize );  
		if( *ppBytes == NULL )  
		{
			return FALSE; 
		}
		else  
		{  
#if 0
			memset(*ppBytes, 0,  *pdwSize + 12);
			memcpy(*ppBytes, _T("ID:469250000"), 12);
			*pdwSize = 12;
			bSuccess = TRUE;
#else

			// Read the file into the newly allocated memory   
			bSuccess = ReadFile( hFile, *ppBytes, *pdwSize, &dwBytes, NULL );  
			if( ! bSuccess )
			{
				return FALSE; 
			}
#endif
		}  
	}  
	// Clean up   
	CloseHandle( hFile );  
	return bSuccess;  
}  
// End ReadFileWithAlloc   
// **********************************************************************   

// **********************************************************************   
// RawDataToPrinter - sends binary data directly to a printer   
//    
// Params:   
//   szPrinterName - NULL terminated string specifying printer name   
//   lpData        - Pointer to raw data bytes   
//   dwCount       - Length of lpData in bytes   
//    
// Returns: TRUE for success, FALSE for failure.   
//    

BOOL CBottomCtrlDlg::RawDataToPrinter( LPTSTR szPrinterName, LPBYTE lpData, DWORD dwCount )  
{  
	HANDLE     hPrinter;  
	DOC_INFO_1 DocInfo;  
	DWORD      dwJob;  
	DWORD      dwBytesWritten;  

	// Need a handle to the printer.   
	if( ! OpenPrinter( szPrinterName, &hPrinter, NULL ) )  
	{  
		AfxMessageBox(_T("OpenPrinter fail"), MB_OK);
		return FALSE;  
	}  

	// Fill in the structure with info about this "document."   
	DocInfo.pDocName = TEXT("sql - 记事本");  
	DocInfo.pOutputFile = NULL;  
	DocInfo.pDatatype = TEXT("RAW");  
	//DocInfo.pDatatype = TEXT("PMJOURNAL");  
	// Inform the spooler the document is beginning.   
	if( (dwJob = StartDocPrinter( hPrinter, 1, (LPBYTE)&DocInfo )) == 0 )  
	{  
		AfxMessageBox(_T("StartDocPrinter fail"), MB_OK);
		ClosePrinter( hPrinter );  
		return FALSE;  
	}  
	// Start a page.   

	if( ! StartPagePrinter( hPrinter ) )  
	{  
		AfxMessageBox(_T("StartPagePrinter fail"), MB_OK);
		EndDocPrinter( hPrinter );  
		ClosePrinter( hPrinter );  
		return FALSE;  
	}  
	// Send the data to the printer.   
	if( ! WritePrinter( hPrinter, lpData, dwCount, &dwBytesWritten ) )  
	{  
		AfxMessageBox(_T("WritePrinter fail"), MB_OK);
		EndPagePrinter( hPrinter );  
		EndDocPrinter( hPrinter );  
		ClosePrinter( hPrinter );  
		return FALSE;  
	}  
	// End the page.   
	if( ! EndPagePrinter( hPrinter ) )  
	{  
		AfxMessageBox(_T("EndPagePrinter fail"), MB_OK);
		EndDocPrinter( hPrinter );  
		ClosePrinter( hPrinter );  
		return FALSE;  
	}  
	// Inform the spooler that the document is ending.   
	if( ! EndDocPrinter( hPrinter ) )  
	{  
		AfxMessageBox(_T("EndDocPrinter fail"), MB_OK);
		ClosePrinter( hPrinter );  
		return FALSE;  
	}  
	// Tidy up the printer handle.   
	ClosePrinter( hPrinter );  
	// Check to see if correct number of bytes were written.   
	if( dwBytesWritten != dwCount )  
	{  
		CString str;

		str.Format(_T("Wrote %d bytes instead of requested %d bytes.\n"), dwBytesWritten, dwCount);
		AfxMessageBox(str, MB_OK);
		return FALSE;  
	}  
	return TRUE;  
}  
// **********************************************************************   
// main - entry point for this console application   
//    
// Params:   
//   argc        - count of command line arguments   
//   argv        - array of NULL terminated command line arguments   
//   
// Returns: 0 for success, non-zero for failure.   
//    
// Command line: c:\>RawPrint PrinterName FileName   
//               sends raw data file to printer using spooler APIs   
//               written nov 1999 jmh   
//   

void CBottomCtrlDlg::print_main( void )  
{  

	LPBYTE  pBytes = NULL;  
	DWORD   dwSize = 0;
	TCHAR szDefaultPrinter[1024]={0}; // 保存默认打印机名称
	DWORD dwLen = 1024;
	DWORD pdwSize = 4096;
	DWORD idex = 0;
	int ret = ::GetDefaultPrinter(szDefaultPrinter, &dwLen);
#if 0
	if( ! ReadFileWithAlloc(_T("D:\\sql.txt"), &dwSize, &pBytes ) )  
		return;

	if(!RawDataToPrinter(szDefaultPrinter, pBytes, dwSize) )  
	{

	}
	else  
	{
	}
#else
	pBytes = (LPBYTE)GlobalAlloc( GPTR, pdwSize);  
	if( pBytes == NULL )  
	{
		AfxMessageBox(_T("GlobalAlloc fail "), MB_OK);
		return; 
	}
	memset(pBytes, 0,  pdwSize);
	memcpy(&pBytes[idex], "ID:469250000", 12);
	idex = idex + 12;
	pBytes[idex] = 0x0D;
	pBytes[idex + 1] = 0x0A;
	idex = idex + 2;
	memcpy(&pBytes[idex], "password:123", 12);
	idex = idex + 12;
	pBytes[idex] = 0x0D;
	pBytes[idex + 1] = 0x0A;
	idex = idex + 2;
	memcpy(&pBytes[idex], "G FH", 4);
	idex = idex + 4;
	pdwSize = idex;

	if(!RawDataToPrinter(szDefaultPrinter, pBytes, pdwSize) )  
	{

	}
	else  
	{
	}
#endif
	
		

	GlobalFree( (HGLOBAL)pBytes );  
	AfxMessageBox(_T("打印成功"), MB_OK);
}


#if 0
void CBottomCtrlDlg::print_LAB_main( void )  
{
	//TCHAR uid_temp[10] = _T("456789");
	TCHAR uid_temp[100] = {0};
	UINT i = 0, fon_height = 0, line_heigt = 0, line_offset = 0, idex = 0;
	CDC dc;
	DOCINFO di;
	UpdateData(); 

	CPrintDialog pdlg(FALSE,PD_NOPAGENUMS|PD_NOSELECTION,this);
	BOOL bFindPrinter=pdlg.GetDefaults();
	if(!bFindPrinter)
	{
		if(pdlg.DoModal() == IDCANCEL) //让用户选择打印纸张等
			return; 
	}
	
	dc.Attach(pdlg.GetPrinterDC()); 
	short cxInch = dc.GetDeviceCaps(LOGPIXELSX);
	short cyInch = dc.GetDeviceCaps(LOGPIXELSY);


	int iHORZSIZE=dc.GetDeviceCaps(HORZSIZE); // 得到当前显示设备的水平大小（毫米）
	int iVERTSIZE=dc.GetDeviceCaps(VERTSIZE); // 得到当前显示设备的垂直大小（毫米）
	int iHORZRES=dc.GetDeviceCaps(HORZRES); // 得到当前显示设备的水平像素数
	int iVERTRES=dc.GetDeviceCaps(VERTRES); // 得到当前显示设备的垂直像素数
	int iLOGPIXELSX=dc.GetDeviceCaps(LOGPIXELSX); // 得到当前显示设备的水平单位英寸像素数
	int iLOGPIXELSY=dc.GetDeviceCaps(LOGPIXELSY); // 得到当前显示设备的垂直单位英寸像素数
	int iPHYSICALWIDTH=dc.GetDeviceCaps(PHYSICALWIDTH); // 得到当前打印设备的水平大小（毫米）
	int iPHYSICALHEIGHT=dc.GetDeviceCaps(PHYSICALHEIGHT); // 得到当前打印设备的水平大小（毫米）

	/////////////////////////////////////////////////
	//字体
	CFont font1;
	CFont font2;
	CFont font3;
	CFont font4;
	CFont font5;
	CFont* def_font;
	//VERIFY(font.CreatePointFont(100, _T("宋体"), &dc));//创建字体
	//def_font = dc.SelectObject(&font);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
	
	//取打印纸张高度和宽度
	int nPageHeight, nPageWidth;
	nPageHeight = dc.GetDeviceCaps(VERTRES);
	nPageWidth = dc.GetDeviceCaps(HORZRES); 
	TEXTMETRIC TextM;
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	int nCharHeight = (unsigned short)TextM.tmHeight;
	int nCharWidth=(unsigned short)TextM.tmAveCharWidth;  
	int m_MaxLineChar = nPageWidth / nCharWidth; //每行最大字数
	int m_LinesPerPage = nPageHeight/ nCharHeight; //每页最大行数
	//页边距
	int nXMargin = 0;
	int nYMargin = 0;

	//
	//设置所需打印纸张数目
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = _T("打印LAB"); 
	di.lpszOutput = (LPTSTR) NULL; 
	di.lpszDatatype = (LPTSTR) NULL; 
	di.fwType = 0;  
	dc.StartDoc(&di);
	dc.StartPage();
	CString strText;

	//strText = _T("ID:456789");
	//dc.TextOut(0,50, strText);
	//strText = _T("PASSWORD:123");
	//dc.TextOut(0,50, strText);
	//strText = _T("Z QC15");
	//dc.TextOut(0,100, strText);
	
	fon_height = g_test_config.config_printf_cfg[0].fon_height * 25;
	VERIFY(font1.CreatePointFont(fon_height, _T("Times New Roman"), &dc));//创建字体
	def_font = dc.SelectObject(&font1);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	nCharHeight = (unsigned short)TextM.tmHeight;
	nCharWidth=(unsigned short)TextM.tmAveCharWidth; //每行最大字数 //每页最大行数
	m_MaxLineChar = nPageWidth / nCharWidth - 6; 
	m_LinesPerPage = nPageHeight/ nCharHeight; 
	//10{mm} * PixelsPerInch * 10 div 254;

	line_offset = 0;
	//第1行
	for (i = 0; i < 5; i++)
	{
		if (g_test_config.config_printf_cfg[i].line_idex == LINE_1)
		{
			break;
		}
	}
	if(i != 5)
	{
		if (i == 0)
		{
			//判断UID是否为空
			for (idex = 0; idex < MAC_ADDRESS_LEN; idex++)
			{
				if (m_connect_uid[idex] != 0)
				{
					break;
				}
			}
			if ( idex == MAC_ADDRESS_LEN)
			{
				AfxMessageBox(_T("UID 为空,请检查"), MB_OK);
				return;
			}

			memset(uid_temp, 0 , 100);
			if (_tcsclen(m_connect_uid) > m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data))
			{
				_tcsncpy(uid_temp, m_connect_uid, m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data));
			}
			else
			{
				_tcsncpy(uid_temp, m_connect_uid, _tcsclen(m_connect_uid));
			}
			
			strText.Format(_T("%s%s"), g_test_config.config_printf_cfg[i].content_data, uid_temp);
			dc.TextOut(0,0, strText);
		}
		else
		{
			dc.TextOut(0,0, g_test_config.config_printf_cfg[i].content_data);	
		}

		line_heigt = nPageHeight/m_LinesPerPage;
		line_offset += line_heigt - 8;
		m_LinesPerPage--;
		if (m_LinesPerPage == 0)
		{
			dc.EndPage();
			dc.EndDoc(); 
			return;
		}
		
	}
	
	//第2行
	fon_height = g_test_config.config_printf_cfg[1].fon_height * 25;
	VERIFY(font2.CreatePointFont(fon_height, _T("Times New Roman"), &dc));//创建字体
	def_font = dc.SelectObject(&font2);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	nCharHeight = (unsigned short)TextM.tmHeight;
	nCharWidth=(unsigned short)TextM.tmAveCharWidth; //每行最大字数 //每页最大行数
	m_MaxLineChar = nPageWidth / nCharWidth - 6; 
	m_LinesPerPage = (nPageHeight - line_offset)/ nCharHeight; 


	for (i = 0; i < 5; i++)
	{
		if (g_test_config.config_printf_cfg[i].line_idex == LINE_2)
		{
			break;
		}
	}
	
	
	
	if(i != 5)
	{
		if (i == 0)
		{
			//判断UID是否为空
			for (idex = 0; idex < MAC_ADDRESS_LEN; idex++)
			{
				if (m_connect_uid[idex] != 0)
				{
					break;
				}
			}
			if ( idex == MAC_ADDRESS_LEN)
			{
				AfxMessageBox(_T("UID 为空,请检查"), MB_OK);
				return;
			}

			memset(uid_temp, 0 , 100);
			if (_tcsclen(m_connect_uid) > m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data))
			{
				_tcsncpy(uid_temp, m_connect_uid, m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data));
			}
			else
			{
				_tcsncpy(uid_temp, m_connect_uid, _tcsclen(m_connect_uid));
			}

			//strText = _T("ID:456789");
			strText.Format(_T("%s%s"), g_test_config.config_printf_cfg[i].content_data, uid_temp);
			dc.TextOut(0,line_offset, strText);
		}
		else
		{
			dc.TextOut(0,line_offset, g_test_config.config_printf_cfg[i].content_data);	
		}
		line_heigt = (nPageHeight - line_offset)/m_LinesPerPage;
		line_offset += line_heigt - 8 ;
		m_LinesPerPage--;
		if (m_LinesPerPage == 0)
		{
			dc.EndPage();
			dc.EndDoc(); 
			return;
		}

	}

	

	//第3行
	fon_height = g_test_config.config_printf_cfg[2].fon_height * 25;
	VERIFY(font3.CreatePointFont(fon_height, _T("Times New Roman"), &dc));//创建字体
	def_font = dc.SelectObject(&font3);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	nCharHeight = (unsigned short)TextM.tmHeight;
	nCharWidth=(unsigned short)TextM.tmAveCharWidth; //每行最大字数 //每页最大行数
	m_MaxLineChar = nPageWidth / nCharWidth - 6; 
	m_LinesPerPage = (nPageHeight - line_offset)/ nCharHeight; 

	for (i = 0; i < 5; i++)
	{
		if (g_test_config.config_printf_cfg[i].line_idex == LINE_3)
		{
			break;
		}
	}
	if(i != 5)
	{
		if (i == 0)
		{
			//判断UID是否为空
			for (idex = 0; idex < MAC_ADDRESS_LEN; idex++)
			{
				if (m_connect_uid[idex] != 0)
				{
					break;
				}
			}
			if ( idex == MAC_ADDRESS_LEN)
			{
				AfxMessageBox(_T("UID 为空,请检查"), MB_OK);
				return;
			}
			memset(uid_temp, 0 , 100);
			if (_tcsclen(m_connect_uid) > m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data))
			{
				_tcsncpy(uid_temp, m_connect_uid, m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data));
			}
			else
			{
				_tcsncpy(uid_temp, m_connect_uid, _tcsclen(m_connect_uid));
			}

			//strText = _T("ID:456789");
			strText.Format(_T("%s%s"), g_test_config.config_printf_cfg[i].content_data, uid_temp);
			dc.TextOut(0,line_offset, strText);
		}
		else
		{
			dc.TextOut(0,line_offset, g_test_config.config_printf_cfg[i].content_data);	
		}

		line_heigt = (nPageHeight - line_offset)/m_LinesPerPage;
		line_offset += line_heigt - 8;
		m_LinesPerPage--;
		if (m_LinesPerPage == 0)
		{
			dc.EndPage();
			dc.EndDoc(); 
			return;
		}

	}

	
	//第4行
	fon_height = g_test_config.config_printf_cfg[3].fon_height * 25;
	VERIFY(font4.CreatePointFont(fon_height, _T("Times New Roman"), &dc));//创建字体
	def_font = dc.SelectObject(&font4);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	nCharHeight = (unsigned short)TextM.tmHeight;
	nCharWidth=(unsigned short)TextM.tmAveCharWidth; //每行最大字数 //每页最大行数
	m_MaxLineChar = nPageWidth / nCharWidth - 6; 
	m_LinesPerPage = (nPageHeight - line_offset)/ nCharHeight; 

	for (i = 0; i < 5; i++)
	{
		if (g_test_config.config_printf_cfg[i].line_idex == LINE_4)
		{
			break;
		}
	}
	if(i != 5)
	{
		if (i == 0)
		{
			//判断UID是否为空
			for (idex = 0; idex < MAC_ADDRESS_LEN; idex++)
			{
				if (m_connect_uid[idex] != 0)
				{
					break;
				}
			}
			if ( idex == MAC_ADDRESS_LEN)
			{
				AfxMessageBox(_T("UID 为空,请检查"), MB_OK);
				return;
			}

			memset(uid_temp, 0 , 100);
			if (_tcsclen(m_connect_uid) > m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data))
			{
				_tcsncpy(uid_temp, m_connect_uid, m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data));
			}
			else
			{
				_tcsncpy(uid_temp, m_connect_uid, _tcsclen(m_connect_uid));
			}

			//strText = _T("ID:456789");
			strText.Format(_T("%s%s"), g_test_config.config_printf_cfg[i].content_data, uid_temp);
			dc.TextOut(0,line_offset, strText);
		}
		else
		{
			dc.TextOut(0,line_offset, g_test_config.config_printf_cfg[i].content_data);	
		}

		line_heigt = (nPageHeight - line_offset)/m_LinesPerPage;
		line_offset += line_heigt - 8;
		m_LinesPerPage--;
		if (m_LinesPerPage == 0)
		{
			dc.EndPage();
			dc.EndDoc(); 
			return;
		}

	}
	
	

	//第5行
	fon_height = g_test_config.config_printf_cfg[3].fon_height * 25;
	VERIFY(font5.CreatePointFont(fon_height, _T("Times New Roman"), &dc));//创建字体
	def_font = dc.SelectObject(&font5);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	nCharHeight = (unsigned short)TextM.tmHeight;
	nCharWidth=(unsigned short)TextM.tmAveCharWidth; //每行最大字数 //每页最大行数
	m_MaxLineChar = nPageWidth / nCharWidth - 6; 
	m_LinesPerPage = (nPageHeight - line_offset)/ nCharHeight; 

	for (i = 0; i < 5; i++)
	{
		if (g_test_config.config_printf_cfg[i].line_idex == LINE_5)
		{
			break;
		}
	}
	if(i != 5)
	{
		if (i == 0)
		{
			//判断UID是否为空
			for (idex = 0; idex < MAC_ADDRESS_LEN; idex++)
			{
				if (m_connect_uid[idex] != 0)
				{
					break;
				}
			}
			if ( idex == MAC_ADDRESS_LEN)
			{
				AfxMessageBox(_T("UID 为空,请检查"), MB_OK);
				return;
			}

			memset(uid_temp, 0 , 100);
			if (_tcsclen(m_connect_uid) > m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data))
			{
				_tcsncpy(uid_temp, m_connect_uid, m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data));
			}
			else
			{
				_tcsncpy(uid_temp, m_connect_uid, _tcsclen(m_connect_uid));
			}

			//strText = _T("ID:456789");
			strText.Format(_T("%s%s"), g_test_config.config_printf_cfg[i].content_data, uid_temp);
			dc.TextOut(0,line_offset, strText);
		}
		else
		{
			dc.TextOut(0,line_offset, g_test_config.config_printf_cfg[i].content_data);	
		}

	}

	dc.EndPage();
	dc.EndDoc(); 
}
#else

void CBottomCtrlDlg::print_LAB_main( void )  
{
	//TCHAR uid_temp[10] = _T("456789");
	TCHAR uid_temp[100] = {0};
	UINT i = 0, fon_height = 0, line_heigt = 0, line_offset = 0, idex = 0, j = 0;
	CDC dc;
	DOCINFO di;
	UpdateData(); 

	CPrintDialog pdlg(FALSE,PD_NOPAGENUMS|PD_NOSELECTION,this);
	BOOL bFindPrinter=pdlg.GetDefaults();
	if(!bFindPrinter)
	{
		if(pdlg.DoModal() == IDCANCEL) //让用户选择打印纸张等
			return; 
	}

	dc.Attach(pdlg.GetPrinterDC()); 
	short cxInch = dc.GetDeviceCaps(LOGPIXELSX);
	short cyInch = dc.GetDeviceCaps(LOGPIXELSY);


	int iHORZSIZE=dc.GetDeviceCaps(HORZSIZE); // 得到当前显示设备的水平大小（毫米）
	int iVERTSIZE=dc.GetDeviceCaps(VERTSIZE); // 得到当前显示设备的垂直大小（毫米）
	int iHORZRES=dc.GetDeviceCaps(HORZRES); // 得到当前显示设备的水平像素数
	int iVERTRES=dc.GetDeviceCaps(VERTRES); // 得到当前显示设备的垂直像素数
	int iLOGPIXELSX=dc.GetDeviceCaps(LOGPIXELSX); // 得到当前显示设备的水平单位英寸像素数
	int iLOGPIXELSY=dc.GetDeviceCaps(LOGPIXELSY); // 得到当前显示设备的垂直单位英寸像素数
	int iPHYSICALWIDTH=dc.GetDeviceCaps(PHYSICALWIDTH); // 得到当前打印设备的水平大小（毫米）
	int iPHYSICALHEIGHT=dc.GetDeviceCaps(PHYSICALHEIGHT); // 得到当前打印设备的水平大小（毫米）

	/////////////////////////////////////////////////
	//字体
	CFont font1;
	CFont font2;
	CFont font3;
	CFont font4;
	CFont font5;
	CFont* def_font;
	//VERIFY(font.CreatePointFont(100, _T("宋体"), &dc));//创建字体
	//def_font = dc.SelectObject(&font);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数

	//取打印纸张高度和宽度
	int nPageHeight, nPageWidth;
	nPageHeight = dc.GetDeviceCaps(VERTRES);
	nPageWidth = dc.GetDeviceCaps(HORZRES); 
	TEXTMETRIC TextM;
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	int nCharHeight = (unsigned short)TextM.tmHeight;
	int nCharWidth=(unsigned short)TextM.tmAveCharWidth;  
	int m_MaxLineChar = nPageWidth / nCharWidth; //每行最大字数
	int m_LinesPerPage = nPageHeight/ nCharHeight; //每页最大行数
	//页边距
	int nXMargin = 0;
	int nYMargin = 0;

	//
	//设置所需打印纸张数目
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = _T("打印LAB"); 
	di.lpszOutput = (LPTSTR) NULL; 
	di.lpszDatatype = (LPTSTR) NULL; 
	di.fwType = 0;  
	dc.StartDoc(&di);
	dc.StartPage();
	CString strText;

	//strText = _T("ID:456789");
	//dc.TextOut(0,50, strText);
	//strText = _T("PASSWORD:123");
	//dc.TextOut(0,50, strText);
	//strText = _T("Z QC15");
	//dc.TextOut(0,100, strText);

	fon_height = g_test_config.config_printf_cfg[0].fon_height * 25;
	VERIFY(font1.CreatePointFont(fon_height, _T("Times New Roman"), &dc));//创建字体
	def_font = dc.SelectObject(&font1);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	nCharHeight = (unsigned short)TextM.tmHeight;
	nCharWidth=(unsigned short)TextM.tmAveCharWidth; //每行最大字数 //每页最大行数
	m_MaxLineChar = nPageWidth / nCharWidth - 6; 
	m_LinesPerPage = nPageHeight/ nCharHeight; 
	//10{mm} * PixelsPerInch * 10 div 254;

	line_offset = 0;
	//第1行
	for (i = 0; i < 5; i++)
	{
		if (g_test_config.config_printf_cfg[i].line_idex == LINE_1)
		{
			break;
		}
	}
	if(i != 5)
	{
		if (i == 0)
		{
			//判断UID是否为空
			for (idex = 0; idex < MAC_ADDRESS_LEN; idex++)
			{
				if (m_connect_uid[idex] != 0)
				{
					break;
				}
			}
			if ( idex == MAC_ADDRESS_LEN)
			{
				AfxMessageBox(_T("UID 为空,请检查"), MB_OK);
				return;
			}

			memset(uid_temp, 0 , 100);
			if (_tcsclen(m_connect_uid) > m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data))
			{
				_tcsncpy(uid_temp, m_connect_uid, m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data));
			}
			else
			{
				_tcsncpy(uid_temp, m_connect_uid, _tcsclen(m_connect_uid));
			}

			strText.Format(_T("%s%s"), g_test_config.config_printf_cfg[i].content_data, uid_temp);
			dc.TextOut(0,0, strText);
		}
		else
		{
			dc.TextOut(0,0, g_test_config.config_printf_cfg[i].content_data);	
		}

		line_heigt = nPageHeight/m_LinesPerPage;
		line_offset += line_heigt - 8;
		m_LinesPerPage--;
		if (m_LinesPerPage == 0)
		{
			dc.EndPage();
			dc.EndDoc(); 
			return;
		}

	}

	//第2行
	fon_height = g_test_config.config_printf_cfg[1].fon_height * 25;
	VERIFY(font2.CreatePointFont(fon_height, _T("Times New Roman"), &dc));//创建字体
	def_font = dc.SelectObject(&font2);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	nCharHeight = (unsigned short)TextM.tmHeight;
	nCharWidth=(unsigned short)TextM.tmAveCharWidth; //每行最大字数 //每页最大行数
	m_MaxLineChar = nPageWidth / nCharWidth - 6; 
	m_LinesPerPage = (nPageHeight - line_offset)/ nCharHeight; 


	for (i = 0; i < 5; i++)
	{
		if (g_test_config.config_printf_cfg[i].line_idex == LINE_2)
		{
			break;
		}
	}



	if(i != 5)
	{
		if (i == 0)
		{
			//判断UID是否为空
			for (idex = 0; idex < MAC_ADDRESS_LEN; idex++)
			{
				if (m_connect_uid[idex] != 0)
				{
					break;
				}
			}
			if ( idex == MAC_ADDRESS_LEN)
			{
				AfxMessageBox(_T("UID 为空,请检查"), MB_OK);
				return;
			}

			memset(uid_temp, 0 , 100);
			if (_tcsclen(m_connect_uid) > m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data))
			{
				_tcsncpy(uid_temp, m_connect_uid, m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data));
			}
			else
			{
				_tcsncpy(uid_temp, m_connect_uid, _tcsclen(m_connect_uid));
			}

			//strText = _T("ID:456789");
			strText.Format(_T("%s%s"), g_test_config.config_printf_cfg[i].content_data, uid_temp);
			dc.TextOut(0,line_offset, strText);
		}
		else
		{
			dc.TextOut(0,line_offset, g_test_config.config_printf_cfg[i].content_data);	
		}
		line_heigt = (nPageHeight - line_offset)/m_LinesPerPage;
		line_offset += line_heigt - 8 ;
		m_LinesPerPage--;
		if (m_LinesPerPage == 0)
		{
			dc.EndPage();
			dc.EndDoc(); 
			return;
		}

	}



	//第3行
	fon_height = g_test_config.config_printf_cfg[2].fon_height * 25;
	VERIFY(font3.CreatePointFont(fon_height, _T("Times New Roman"), &dc));//创建字体
	def_font = dc.SelectObject(&font3);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	nCharHeight = (unsigned short)TextM.tmHeight;
	nCharWidth=(unsigned short)TextM.tmAveCharWidth; //每行最大字数 //每页最大行数
	m_MaxLineChar = nPageWidth / nCharWidth - 6; 
	m_LinesPerPage = (nPageHeight - line_offset)/ nCharHeight; 

	for (i = 0; i < 5; i++)
	{
		if (g_test_config.config_printf_cfg[i].line_idex == LINE_3)
		{
			break;
		}
	}
	if(i != 5)
	{
		if (i == 0)
		{
			//判断UID是否为空
			for (idex = 0; idex < MAC_ADDRESS_LEN; idex++)
			{
				if (m_connect_uid[idex] != 0)
				{
					break;
				}
			}
			if ( idex == MAC_ADDRESS_LEN)
			{
				AfxMessageBox(_T("UID 为空,请检查"), MB_OK);
				return;
			}
			memset(uid_temp, 0 , 100);
			if (_tcsclen(m_connect_uid) > m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data))
			{
				_tcsncpy(uid_temp, m_connect_uid, m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data));
			}
			else
			{
				_tcsncpy(uid_temp, m_connect_uid, _tcsclen(m_connect_uid));
			}

			//strText = _T("ID:456789");
			strText.Format(_T("%s%s"), g_test_config.config_printf_cfg[i].content_data, uid_temp);
			dc.TextOut(0,line_offset, strText);
		}
		else
		{
			dc.TextOut(0,line_offset, g_test_config.config_printf_cfg[i].content_data);	
		}

		line_heigt = (nPageHeight - line_offset)/m_LinesPerPage;
		line_offset += line_heigt - 8;
		m_LinesPerPage--;
		if (m_LinesPerPage == 0)
		{
			dc.EndPage();
			dc.EndDoc(); 
			return;
		}

	}


	//第4行
	fon_height = g_test_config.config_printf_cfg[3].fon_height * 25;
	VERIFY(font4.CreatePointFont(fon_height, _T("Times New Roman"), &dc));//创建字体
	def_font = dc.SelectObject(&font4);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	nCharHeight = (unsigned short)TextM.tmHeight;
	nCharWidth=(unsigned short)TextM.tmAveCharWidth; //每行最大字数 //每页最大行数
	m_MaxLineChar = nPageWidth / nCharWidth - 6; 
	m_LinesPerPage = (nPageHeight - line_offset)/ nCharHeight; 

	for (i = 0; i < 5; i++)
	{
		if (g_test_config.config_printf_cfg[i].line_idex == LINE_4)
		{
			break;
		}
	}
	if(i != 5)
	{
		if (i == 0)
		{
			//判断UID是否为空
			for (idex = 0; idex < MAC_ADDRESS_LEN; idex++)
			{
				if (m_connect_uid[idex] != 0)
				{
					break;
				}
			}
			if ( idex == MAC_ADDRESS_LEN)
			{
				AfxMessageBox(_T("UID 为空,请检查"), MB_OK);
				return;
			}

			memset(uid_temp, 0 , 100);
			if (_tcsclen(m_connect_uid) > m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data))
			{
				_tcsncpy(uid_temp, m_connect_uid, m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data));
			}
			else
			{
				_tcsncpy(uid_temp, m_connect_uid, _tcsclen(m_connect_uid));
			}

			//strText = _T("ID:456789");
			strText.Format(_T("%s%s"), g_test_config.config_printf_cfg[i].content_data, uid_temp);
			dc.TextOut(0,line_offset, strText);
		}
		else
		{
			dc.TextOut(0,line_offset, g_test_config.config_printf_cfg[i].content_data);	
		}

		line_heigt = (nPageHeight - line_offset)/m_LinesPerPage;
		line_offset += line_heigt - 8;
		m_LinesPerPage--;
		if (m_LinesPerPage == 0)
		{
			dc.EndPage();
			dc.EndDoc(); 
			return;
		}

	}



	//第5行
	fon_height = g_test_config.config_printf_cfg[3].fon_height * 25;
	VERIFY(font5.CreatePointFont(fon_height, _T("Times New Roman"), &dc));//创建字体
	def_font = dc.SelectObject(&font5);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
	dc.GetTextMetrics(&TextM);
	//字体高度 //字体平均宽度
	nCharHeight = (unsigned short)TextM.tmHeight;
	nCharWidth=(unsigned short)TextM.tmAveCharWidth; //每行最大字数 //每页最大行数
	m_MaxLineChar = nPageWidth / nCharWidth - 6; 
	m_LinesPerPage = (nPageHeight - line_offset)/ nCharHeight; 

	for (i = 0; i < 5; i++)
	{
		if (g_test_config.config_printf_cfg[i].line_idex == LINE_5)
		{
			break;
		}
	}
	if(i != 5)
	{
		if (i == 0)
		{
			//判断UID是否为空
			for (idex = 0; idex < MAC_ADDRESS_LEN; idex++)
			{
				if (m_connect_uid[idex] != 0)
				{
					break;
				}
			}
			if ( idex == MAC_ADDRESS_LEN)
			{
				AfxMessageBox(_T("UID 为空,请检查"), MB_OK);
				return;
			}

			memset(uid_temp, 0 , 100);
			if (_tcsclen(m_connect_uid) > m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data))
			{
				_tcsncpy(uid_temp, m_connect_uid, m_MaxLineChar - _tcsclen(g_test_config.config_printf_cfg[i].content_data));
			}
			else
			{
				_tcsncpy(uid_temp, m_connect_uid, _tcsclen(m_connect_uid));
			}

			//strText = _T("ID:456789");
			strText.Format(_T("%s%s"), g_test_config.config_printf_cfg[i].content_data, uid_temp);
			dc.TextOut(0,line_offset, strText);
		}
		else
		{
			dc.TextOut(0,line_offset, g_test_config.config_printf_cfg[i].content_data);	
		}

	}

	dc.EndPage();
	dc.EndDoc(); 
}

#endif
void CBottomCtrlDlg::OnBnClickedButtonImgaeReset()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
#if 1
	if (pP->check_ip(m_connect_ip) && m_connect_ip[0] == 0 
		&& m_connect_ip[1] == 0 && m_connect_ip[2] == 0
		&& m_connect_ip[3] == 0 && m_connect_ip[4] == 0
		&& m_connect_ip[5] == 0 && m_connect_ip[6] == 0
		&& m_connect_ip[7] == 0 && m_connect_ip[8] == 0)
	{
		AfxMessageBox(_T("请选择一台设备"), MB_OK);
		return ;
	}
#endif

	pP->image_reset.DoModal();
}



void CBottomCtrlDlg::print_image_main( TCHAR *path_filename, TCHAR *uid )  
{
	//DWORD dwflags=PD_ALLPAGES|PD_NOPAGENUMS|PD_USEDEVMODECOPIES|PD_SELECTION|PD_HIDEPRINTTOFILE;
	//CPrintDialog dlg(FALSE, dwflags, NULL);
	CDC dc;
	//DOCINFO di;
	UpdateData(); 

	CPrintDialog dlg(FALSE,PD_NOPAGENUMS|PD_NOSELECTION,this);
	BOOL bFindPrinter=dlg.GetDefaults();
	if(!bFindPrinter)
	{
		if(dlg.DoModal() == IDCANCEL) //让用户选择打印纸张等
			return; 
	}
	else
	//if(dlg.DoModal() == IDOK)
	{
		HDC hdcPrinter = dlg.GetPrinterDC(); 
		if (hdcPrinter == NULL)
		{
			MessageBox(_T("Buy a printer!"));
		}
		else
		{

			CDC dcPrinter;                            //定义一个设备
			dcPrinter.Attach(hdcPrinter);             //把打印机附于这个设备

			DOCINFO docinfo;                          //打印机属性
			memset(&docinfo, 0, sizeof(docinfo));
			docinfo.cbSize = sizeof(docinfo);
			docinfo.lpszDocName = _T("打印机测试");
#if 0
			CPrintInfo printInfo;
			AfxGetApp()->GetPrinterDeviceDefaults(&printInfo.m_pPD->m_pd);
			if (printInfo.m_pPD->m_pd.hDC == NULL)
			{
				// call CreatePrinterDC if DC was not created by above
				printInfo.m_pPD->CreatePrinterDC();
			}
			dcPrinter.m_bPrinting = TRUE;
			LPDEVMODE  pDevMode; 
			pDevMode=printInfo.m_pPD->GetDevMode(); 
			if(pDevMode!=NULL)
			{
				pDevMode->dmOrientation=DMORIENT_LANDSCAPE;
				pDevMode->dmColor=DMCOLOR_COLOR; 
				dcPrinter.ResetDC(pDevMode);
			}

#endif
			if (dcPrinter.StartDoc(&docinfo) < 0)
			{
				MessageBox(_T("Printer wouldn't initalize"));
			}
			else
			{
				// start a page
				if (dcPrinter.StartPage() < 0)
				{
					MessageBox(_T("Could not start page"));
					dcPrinter.AbortDoc();
				}
				else
				{
					
					//SetTextColor(dcPrinter,RGB(192, 192, 192));   //设置打印机的字体颜色，其实还可以设置字体的
					//CClientDC *pDC = new CClientDC(GetDlgItem(IDC_STATIC_PIC));
					//CRect rect;
					//GetDlgItem(IDC_STATIC_PIC)->GetClientRect(&rect);
					Graphics graphics(dcPrinter.m_hDC); // Create a GDI+ graphics object
					//Image  image( L"F:\\test.bmp" );          //加载图片
					Image  image(path_filename);          //加载图片
					Graphics imageGraphics(&image);              //通过Image对象创建一个绘图句柄，使用这个句柄对图片进行操作 
					imageGraphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
					
#if 0
					WCHAR string[ ] = L"复制件";                   //要写的字
					FontFamily fontFamily(L"微软雅黑"); 
					Gdiplus::Font myFont(&fontFamily,60, FontStyleRegular, UnitPoint); //第二个是字体大小
					SolidBrush blackBrush(Color(50, 0, 0, 0));  //半透明+文字RGB颜色
					PointF school_site((REAL)0, (REAL)100);//文字放置的像素坐标
					StringFormat format;                         
					format.SetAlignment(StringAlignmentCenter);    //文本排列方式，即在对应位置居中、靠左、靠右
					//wcscpy_s(string, CT2CW(str));              //如果使用MFC中的CString，需要这样转换成WCHAR
					imageGraphics.RotateTransform(315);
					imageGraphics.DrawString(string, wcslen(string), &myFont, school_site, &format, &blackBrush );//把string绘制到图上
					school_site.X = -200;
					school_site.Y = 300;
					imageGraphics.DrawString(string, wcslen(string), &myFont, school_site, &format, &blackBrush );//把string绘制到图上
					school_site.X = -400;
					school_site.Y = 500;

#endif
					//HORZRES 以像素为单位的显示宽度 0~65535
					//VERTRES
					CDC memDC;        //定义一个设备
					CClientDC dc(this);      //获取客户
					memDC.CreateCompatibleDC( &dc );
					// memDC.SelectObject( imageGraphics );  //为设备选择对象


					int iHORZSIZE=dcPrinter.GetDeviceCaps(HORZSIZE); // 得到当前显示设备的水平大小（毫米）
					int iVERTSIZE=dcPrinter.GetDeviceCaps(VERTSIZE); // 得到当前显示设备的垂直大小（毫米）
					int iHORZRES=dcPrinter.GetDeviceCaps(HORZRES); // 得到当前显示设备的水平像素数
					int iVERTRES=dcPrinter.GetDeviceCaps(VERTRES); // 得到当前显示设备的垂直像素数
					int iLOGPIXELSX=dcPrinter.GetDeviceCaps(LOGPIXELSX); // 得到当前显示设备的水平单位英寸像素数
					int iLOGPIXELSY=dcPrinter.GetDeviceCaps(LOGPIXELSY); // 得到当前显示设备的垂直单位英寸像素数
					int iPHYSICALWIDTH=dcPrinter.GetDeviceCaps(PHYSICALWIDTH); // 得到当前打印设备的水平大小（毫米）
					int iPHYSICALHEIGHT=dcPrinter.GetDeviceCaps(PHYSICALHEIGHT); // 得到当前打印设备的水平大小（毫米）
					int  x = image.GetWidth();
					int y = image.GetHeight();
					CSize sizeLUsPI; // Logic Unit Per Inch
					sizeLUsPI.cx = dcPrinter.GetDeviceCaps(LOGPIXELSX);
					sizeLUsPI.cy = dcPrinter.GetDeviceCaps(LOGPIXELSY);

					CSize sizePaper; // 纸张尺寸。
					sizePaper.cx = dcPrinter.GetDeviceCaps(PHYSICALWIDTH);
					sizePaper.cy = dcPrinter.GetDeviceCaps(PHYSICALHEIGHT);
					dcPrinter.DPtoLP(&sizePaper);


					CSize sizeOfsLT; // 左边和上边，逻辑单位。
					sizeOfsLT.cx = dcPrinter.GetDeviceCaps(PHYSICALOFFSETX);
					sizeOfsLT.cy = dcPrinter.GetDeviceCaps(PHYSICALOFFSETY);
					dcPrinter.DPtoLP(&sizeOfsLT);

					CSize sizePrint; // 可打印范围。
					sizePrint.cx = dcPrinter.GetDeviceCaps(HORZRES  );
					sizePrint.cy = dcPrinter.GetDeviceCaps(VERTRES );
					dcPrinter.DPtoLP(&sizePrint);

					int printx, printy;
					double ratex, ratey;

					int nPrintW = dcPrinter.GetDeviceCaps(HORZRES);
					int nPrintH = dcPrinter.GetDeviceCaps(VERTRES);

					printx = dcPrinter.GetDeviceCaps(LOGPIXELSX);
					printy = dcPrinter.GetDeviceCaps(LOGPIXELSY);
					int offset_x = GetDeviceCaps(dcPrinter.GetSafeHdc(),PHYSICALOFFSETX);//获得打印机预留的左边距
					int offset_y = GetDeviceCaps(dcPrinter.GetSafeHdc(),PHYSICALOFFSETY);//获得打印机预留的上边距
					ratex = (double)(printx)/90;
					ratey = (double)(printy)/90;

					int a = image.GetWidth()*ratex;

					int nMarginLeft = abs((int)(nPrintW- image.GetWidth()*ratex )/2);
					int nMarginTop = abs((int)(nPrintH- image.GetHeight()*ratey)/2);

					//水平
					//1mm=31.49606299px
					//1px=0.03175mm

					//垂直方向
					//1mm=23.62204724px
					//1px=0.042333333mm

					RectF rect;

					rect.X = g_test_config.OffsetV;
					rect.Y = g_test_config.OffsetH;
					rect.Width = g_test_config.LabWidth;
					rect.Height = g_test_config.LabHeight;

					//graphics.DrawImage(&image, 0, 0, 1100, 778);

					//graphics.DrawImage(&image, x, y, (float)img_width, (float)img_height);

					graphics.DrawImage(&image,rect);

					//graphics.DrawImage(&image, 0, 0, 1100, 778);

					// Graphics graphics( pDC->m_hDC );
					// Image image(L"sunflower.jpg");
					/*dcPrinter.StartDoc(_T("F:\\企业法人.jpg"));
					dcPrinter.StretchBlt(nMarginLeft, nMarginTop, (int) (rect.Width()*ratex), (int)(rect.Height()*ratey), &memDC, 0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);*/
					/*Image* pThumbnail = image.GetThumbnailImage(1050,742, NULL, NULL);
					graphics.DrawImage(pThumbnail,0,0,pThumbnail->GetWidth(),pThumbnail->GetHeight());
					delete pThumbnail;*/

					// 显示缩略图
					//graphics.DrawImage(pThumbnail, 20, 20);
					// 使用后，不要忘记删除该缩略图指针

					//DrawBmp(&dcPrinter);           //调用打印图片函数，标准的做法应该是还传一个要打印的图片的
					CFont font1;
					CFont* def_font;
					VERIFY(font1.CreatePointFont(g_test_config.char_height, _T("Times New Roman"), &dcPrinter));//创建字体
					def_font = dcPrinter.SelectObject(&font1);//保存现在的字体 //根据字体宽度、高度计算每行最大字数及每页最大行数
					ratey = 6;
					CString uid_str;
					
					if (g_test_config.printf_char_mode == 1)
					{
						uid_str.Format(_T("SN:%s"), uid);
					}
					else
					{
						UINT surbuf_len = 0, i = 0, j = 0;
						TCHAR tmpBuf[MAC_ADDRESS_LEN+1] = {0};
						TCHAR tmpBuf_1[2+1] = {0};

						surbuf_len = wcslen(uid);
						for (i = 0; i < surbuf_len; i++)
						{
							if (uid[i] == 58)// :
							{
								continue;
							}
							if (isupper(uid[i]))
							{
								tmpBuf_1[0] = tolower(uid[i]); //转大
							}
							else
							{
								tmpBuf_1[0] = uid[i]; //直接附值
							}
							_tcsncpy(&tmpBuf[j], tmpBuf_1, 1);
							j++;
						}

						uid_str.Format(_T("%s"), tmpBuf);
					}
					
					dcPrinter.TextOut(g_test_config.SN_OffsetV,g_test_config.SN_OffsetH, uid_str);
					//dcPrinter.TextOut(g_test_config.OffsetV*ratey,(g_test_config.LabHeight + g_test_config.OffsetH)*ratey, uid_str);
					dcPrinter.EndPage();            //结束这页
					dcPrinter.EndDoc();             //结束      
				}
			}
			dcPrinter.Detach();             //释放对象
			dcPrinter.DeleteDC();           //释放设备
		}

	}
}
void CBottomCtrlDlg::OnBnClickedButtonTest()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();

	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(FALSE);
	pP->case_test_user();
	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(TRUE);
}

extern BOOL g_I_frame_flag;
void CBottomCtrlDlg::OnBnClickedButtonRecord()
{
	// TODO: 在此添加控件通知处理程序代码
	char ret = 0;
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	if (g_extern_start_record_flag)
	{
		g_extern_start_record_flag = FALSE;
		if (!pP->Creat_record_video())
		{
			return;
		}

		if (0)//!pP->creat_record_video_thread())
		{
			return;
		}
		g_I_frame_flag = FALSE;
		SetDlgItemText(IDC_BUTTON_RECORD, _T("停止录像"));
	}
	else
	{
		g_extern_start_record_flag = TRUE;
		if (!pP->Close_record_video())
		{
			return;
		}
		SetDlgItemText(IDC_BUTTON_RECORD, _T("开始录像"));
		// TODO: 在此添加控件通知处理程序代码

	}
}

void CBottomCtrlDlg::OnBnClickedButtonWriteMac()
{
	
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->next_test_flag = FALSE;
	g_test_config.m_test_MAC_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	pP->OnBnClickedButtonWriteMac();
	pP->next_test_flag = TRUE;
	// TODO: 在此添加控件通知处理程序代码
}

void CBottomCtrlDlg::OnLvnItemchangedListWifi(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}
