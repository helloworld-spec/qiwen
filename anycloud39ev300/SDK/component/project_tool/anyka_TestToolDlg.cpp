// anyka_TestToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "anyka_TestTool.h"
#include "anyka_TestToolDlg.h"
#include "Config_test.h"
#include "TranceCom.h"
#include "SearchServer.h"

#include "winsock.h"
#include "atlconv.h"



#define  TOOL_VERSOIN _T("工程易用宝 V2.1.03 ")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma   comment(lib,   "ws2_32.lib ")
#pragma   comment(lib,   "mpr.lib")

#define  IP_ADDRE   _T("172.22.5.11")

#define  TEST_WAIT  0
#define  TEST_PASS  1
#define  TEST_FAIL  2
#define  TEST_ING   3


#define  SERAIL_CONFIG   _T("serial_config")
#define  SERAIL_CONFIG_BAK   _T("serial_config_bak")
#define  TEST_CONFIG   _T("test_config")
#define  TEST_CONFIG_DIR           _T("test_config/")
#define  UPDATE_CONFIG_DIR           _T("update_config")


#define  SHOW_TEST_ING   _T("测试中...")
#define  SHOW_TEST_PASS  _T("测试成功")
#define  SHOW_TEST_FAIL  _T("测试失败!!!")


typedef struct
{
    UINT thread_idex;		
	UINT IP_idex;
}T_IDEX_INFO;

T_SEC_CTRL		passwd_ctrl;
UINT g_updateing_num = 0; 
HANDLE g_handle;   //信号量

CConfig_test g_test_config;
HANDLE g_hBurnThread = INVALID_HANDLE_VALUE;
HANDLE g_heatThread[UPDATE_MAX_NUM] = {INVALID_HANDLE_VALUE};
HANDLE g_hBurnThread_rev_data[UPDATE_MAX_NUM] = {INVALID_HANDLE_VALUE};
HANDLE g_update_finish_Thread[UPDATE_MAX_NUM] = {INVALID_HANDLE_VALUE};

HANDLE g_hBurnThread_connet = INVALID_HANDLE_VALUE;
HANDLE g_hBurnThread_watch_dog_heat = INVALID_HANDLE_VALUE;

HANDLE g_hupdateThread = INVALID_HANDLE_VALUE;
HANDLE g_all_updateThread[UPDATE_MAX_NUM] = {INVALID_HANDLE_VALUE};
UINT g_find_num = 0;
UINT g_cur_find_num = 0;

UINT rve_param[UPDATE_MAX_NUM][2] = {0};
UINT heat_param[UPDATE_MAX_NUM][2] = {0};
UINT update_param[UPDATE_MAX_NUM][2] = {0};
T_IDEX_INFO g_param[UPDATE_MAX_NUM] = {0};

char g_send_commad[UPDATE_MAX_NUM] ={0};
BOOL g_heat_enter_flag = FALSE;
BOOL g_close_Thread_flag = FALSE;
BOOL g_repeat_test_flag = FALSE;
BOOL g_auto_test_fail_flag = FALSE;
BOOL g_auto_test_flag = FALSE;
BOOL g_setconfig_flag = TRUE;
BOOL m_not_find_anyIP = FALSE;
BOOL m_find_IP_end_flag = FALSE;
BOOL g_update_all_flag = FALSE;
BOOL g_update_all_finish_flag = FALSE;
BOOL g_update_one_finish_flag = FALSE;
UINT g_update_idex = 0;
BOOL g_update_one_flag = FALSE;
BOOL m_Manual_flag;



TCHAR m_ip_address_buf[UPDATE_MAX_NUM][IP_ADDRE_LEN];
char  m_update_flag[UPDATE_MAX_NUM];
UINT m_ip_address_idex = 0;
BOOL g_sousuo_flag = FALSE;
BOOL g_IP_same_flag = FALSE;
BOOL g_sousuo_show_flag = FALSE;

extern char g_current_serial_number[MAX_PATH];
extern UINT g_serial_num_len;

CInternetSession *m_pInetSession[UPDATE_MAX_NUM] = {NULL};
CFtpConnection *m_pFtpConnection[UPDATE_MAX_NUM] = {NULL}; 
UINT m_uPort = 0;
UINT m_net_uPort = 0;

CTranceCom com;
BOOL g_connet_success_flag = FALSE;
BOOL g_heat_flag = TRUE;
BOOL M_bConn;



char g_commad_type;
char g_test_fail_flag[UPDATE_MAX_NUM] = {0};
char g_test_pass_flag[UPDATE_MAX_NUM] = {0};  //0正在测试中， //1测试成功， 2测试失败
char g_update_pass_flag[UPDATE_MAX_NUM] = {0};  //0正在测试中， //1测试成功， 2测试失败
char g_IPaddress_corret_flag = 0;

CLogFile        frmLogfile;


DWORD WINAPI Creat_Anyka_update_one_main(LPVOID lpParameter);
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnyka_TestToolDlg dialog

CAnyka_TestToolDlg::CAnyka_TestToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnyka_TestToolDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnyka_TestToolDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAnyka_TestToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnyka_TestToolDlg)
	//DDX_Control(pDX, IDC_BUTTON_CONNET, m_show_connet);
	//DDX_Control(pDX, IDC_IPADDRESS_NET, m_ipaddress_net);
	DDX_Control(pDX, IDC_LIST_NO_AUTO_TEST_CONFIG, m_no_auto_test_config);
	//DDX_Control(pDX, IDC_LIST_AUTO_TEST_CONFIG, m_auto_test_config);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAnyka_TestToolDlg, CDialog)
	//{{AFX_MSG_MAP(CAnyka_TestToolDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNET_SERVER, OnButtonConnetServer)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_CONFIG, OnButtonLoadConfig)
	ON_BN_CLICKED(IDC_RADIO_NEED_SERIAL, OnRadioNeedSerial)
	ON_BN_CLICKED(IDC_RADIO_NO_NEED_SERIAL, OnRadioNoNeedSerial)
	ON_EN_CHANGE(IDC_EDIT_SERIAL_NUMBER, OnChangeEditSerialNumber)
	ON_WM_TIMER()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_NO_AUTO_TEST_CONFIG, OnItemchangedListNoAutoTestConfig)
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_LIST_NO_AUTO_TEST_CONFIG, OnItemchangingListNoAutoTestConfig)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST_NO_AUTO_TEST_CONFIG, OnSetfocusListNoAutoTestConfig)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG, OnButtonConfig)
	ON_BN_CLICKED(IDC_BUTTON_SOUSUO, OnButtonSousuo)
	ON_BN_CLICKED(IDC_CHECK_DHCP, OnCheckDhcp)
	ON_BN_CLICKED(IDC_BUTTON_SETCONFIG, OnButtonSetconfig)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_ALL, OnButtonUpdateAll)
	ON_EN_CHANGE(IDC_EDIT_CHANNEL_NAME, OnChangeEditChannelName)
	ON_BN_CLICKED(IDC_BUTTON_MANUAL, OnButtonManual)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_ONE, OnButtonUpdateOne)
	ON_BN_CLICKED(IDC_BUTTON_DETTECT_LIST, OnButtonDettectList)
	ON_EN_CHANGE(IDC_EDIT_USENAME, OnChangeEditUsename)
	ON_EN_CHANGE(IDC_EDIT_PASSWORD, OnChangeEditPassword)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnyka_TestToolDlg message handlers


void CAnyka_TestToolDlg::reset_disable(void)
{
	UINT i = 0;

	 GetDlgItem(IDC_BUTTON_LOAD_CONFIG)->EnableWindow(FALSE);//不可用
	 GetDlgItem(IDC_RADIO_NEED_SERIAL)->EnableWindow(FALSE);//不可用
	 GetDlgItem(IDC_RADIO_NO_NEED_SERIAL)->EnableWindow(FALSE);//不可用
	 GetDlgItem(IDC_EDIT_SERIAL_NUMBER)->EnableWindow(FALSE);//不可用
	 GetDlgItem(IDC_BUTTON_AUTO_TEST)->EnableWindow(FALSE);//不可用
	 GetDlgItem(IDC_BUTTON_CONTINUE_TEST)->EnableWindow(FALSE);//不可用
	 GetDlgItem(IDC_BUTTON_REPEAT_TEST)->EnableWindow(FALSE);//不可用
	 GetDlgItem(IDC_BUTTON_NEXT_TEST)->EnableWindow(FALSE);//不可用
	 GetDlgItem(IDC_BUTTON_NO_AUTO_REPEAT_TEST)->EnableWindow(FALSE);//不可用
	 GetDlgItem(IDC_BUTTON_PRE_TEST)->EnableWindow(FALSE);//不可用
	 GetDlgItem(IDC_BUTTON_TEST_FAIL)->EnableWindow(FALSE);//不可用

	 for(i = 0; i < g_test_config.auto_test_count; i++)
	 {

		 if (g_test_config.auto_test_config[i].auto_test_flag)
		 {
			m_auto_test_config.SetTextColor(RGB(0,0, 0));  //蓝
			m_auto_test_config.SetItemText(i, 2, _T(""));
			g_test_config.auto_test_config[i].test_pass_flag = TEST_WAIT;
		 }
	 }
	 
	 for(i = 0; i < g_test_config.no_auto_test_count; i++)
	 {
		 if (!g_test_config.no_auto_test_config[i].auto_test_flag)
		 {
			m_no_auto_test_config.SetTextColor(RGB(0,0, 0));  //蓝
			m_no_auto_test_config.SetItemText(i, 2, _T(""));
			g_test_config.no_auto_test_config[i].test_pass_flag = TEST_WAIT;
		 }
	}
	 
}

BOOL CAnyka_TestToolDlg::set_test_staut(UINT file_idex,  BOOL auto_flag, UINT test_staut)
{
	UINT len = 0, bin_len = 0 ;
	CString str;
	
	//UpdateData(TRUE);
	
	if (auto_flag)
	{
		if (g_test_config.auto_test_config[file_idex].auto_test_flag)
		{

			//str.Format(_T("%d"), file_idex+1);
			//m_auto_test_config.InsertItem(file_idex, str);
			g_test_config.auto_test_config[file_idex].test_pass_flag = test_staut;
			//m_auto_test_config.SetItemText(file_idex, 1, g_test_config.auto_test_config[file_idex].test_name);
#if 0
			if(test_staut == TEST_ING)
			{
				m_auto_test_config.SetTextColor(RGB(0,0, 255));  //蓝
				m_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_ING);
				
			}
			else if (test_staut == TEST_PASS)
			{
				m_auto_test_config.SetTextColor(RGB(0,255, 0)); //绿
				m_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_PASS);
			}
			else if (test_staut == TEST_FAIL)
			{
				m_auto_test_config.SetTextColor(RGB(255,0,0)); //红
				m_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_FAIL);
			}
#endif
		}
	}
	else
	{
		if (!g_test_config.no_auto_test_config[file_idex].auto_test_flag)
		{
			//str.Format(_T("%d"), i+1);
			//m_no_auto_test_config.InsertItem(i, str);
			
			//m_no_auto_test_config.SetItemText(i, 1, g_test_config.no_auto_test_config[i].test_name);
			//m_no_auto_test_config.SetTextColor(RGB(0,0, 0));
			g_test_config.no_auto_test_config[file_idex].test_pass_flag = test_staut;

			//m_no_auto_test_config.SetItemState(file_idex, LVIS_SELECTED, LVIS_SELECTED);
			      
			//DWORD dwStyle = m_no_auto_test_config.GetExtendedStyle();       
			//dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮
#if 0
			if(test_staut == TEST_ING)
			{
				
				m_no_auto_test_config.SetTextColor(RGB(0,0, 255));
				m_no_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_ING);
			}
			else if (test_staut == TEST_PASS)
			{
				m_no_auto_test_config.SetTextColor(RGB(0,255, 0));
				m_no_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_PASS);
			}
			else if (test_staut == TEST_FAIL)
			{
				m_no_auto_test_config.SetTextColor(RGB(255,0,0));
				m_no_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_FAIL);
			}
#endif
		}
	}
	//UpdateData(TRUE);
	return TRUE;
}


BOOL CAnyka_TestToolDlg::show_test_staut(UINT file_idex,  BOOL auto_flag, UINT test_staut)
{
	UINT len = 0, bin_len = 0 ;
	CString str;

	UpdateData(TRUE);

	if (auto_flag)
	{
		if (g_test_config.auto_test_config[file_idex].auto_test_flag)
		{
			//str.Format(_T("%d"), file_idex+1);
			//m_auto_test_config.InsertItem(file_idex, str);
			
			//m_auto_test_config.SetItemText(file_idex, 1, g_test_config.auto_test_config[file_idex].test_name);
			if(test_staut == TEST_ING)
			{
				//m_auto_test_config.SetTextColor(RGB(0,0, 255));  //蓝
				m_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_ING);
			}
			else if (test_staut == TEST_PASS)
			{
				//m_auto_test_config.SetTextColor(RGB(0,255, 0)); //绿
				m_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_PASS);
			}
			else if (test_staut == TEST_FAIL)
			{
				//m_auto_test_config.SetTextColor(RGB(255,0,0)); //红
				m_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_FAIL);
			}
		}
	}
	else
	{
		if (!g_test_config.no_auto_test_config[file_idex].auto_test_flag)
		{
			//str.Format(_T("%d"), i+1);
			//m_no_auto_test_config.InsertItem(i, str);
			
			//m_no_auto_test_config.SetItemText(i, 1, g_test_config.no_auto_test_config[i].test_name);
			m_no_auto_test_config.SetTextColor(RGB(0,0, 0));
			if(test_staut == TEST_ING)
			{
				m_no_auto_test_config.SetTextColor(RGB(0,0, 255));
				m_no_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_ING);
			}
			else if (test_staut == TEST_PASS)
			{
				m_no_auto_test_config.SetTextColor(RGB(0,255, 0));
				m_no_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_PASS);
			}
			else if (test_staut == TEST_FAIL)
			{
				m_no_auto_test_config.SetTextColor(RGB(255,0,0));
				m_no_auto_test_config.SetItemText(file_idex, 2, SHOW_TEST_FAIL);
			}
		}
	}
	//UpdateData(TRUE);
	return TRUE;
}

BOOL CAnyka_TestToolDlg::set_config_item(void)
{
	UINT i, len = 0, bin_len = 0;
	CString str;
	
	//USES_CONVERSION;
	
	//set
	m_auto_test_config.DeleteAllItems();
	for(i = 0; i < g_test_config.auto_test_count; i++)
	{
		if (g_test_config.auto_test_config[i].auto_test_flag)
		{
			str.Format(_T("%d"), i+1);
			m_auto_test_config.InsertItem(i, str);
			
			m_auto_test_config.SetItemText(i, 1, g_test_config.auto_test_config[i].test_name);
		}
	}

	m_no_auto_test_config.DeleteAllItems();
	for(i = 0; i < g_test_config.no_auto_test_count; i++)
	{
		if (!g_test_config.no_auto_test_config[i].auto_test_flag)
		{
			str.Format(_T("%d"), i+1);
			m_no_auto_test_config.InsertItem(i, str);
			
			m_no_auto_test_config.SetItemText(i, 1, g_test_config.no_auto_test_config[i].test_name);
		}
	}
	
	return TRUE;
}

//判断是否全0值
BOOL CAnyka_TestToolDlg::is_zero_ether_addr(TCHAR *addr)
{
	BOOL flag = FALSE;
	
	//0表示48
	if ((addr[0] == 48 && addr[1] == 48 && addr[3] == 48 
		&& addr[4] == 48 && addr[6] == 48 && addr[7] == 48
		&& addr[9] == 48 && addr[10] == 48 && addr[12] == 48
		&& addr[3] == 48 && addr[15] == 48&& addr[16] == 48))
	{
		flag = TRUE;//如果是0就为true
	}
	
	return flag;
}

//判断是否multicast
BOOL CAnyka_TestToolDlg::is_multicast_ether_addr(TCHAR *addr)
{
	UINT n = 0;
	CHAR num[2] = {0};
	
	USES_CONVERSION;//
	
	memcpy(num, T2A(addr+1), 1);//字符转整
	sscanf(num, "%x", &n);//
	
	return 0x01 & n;
}

BOOL CAnyka_TestToolDlg::get_password_and_name(void) 
{
	TCHAR username[MAX_PASSWD_LENGTH+1] = {0};
	TCHAR password[MAX_PASSWD_LENGTH+1] = {0};
	CString str;
	BOOL change_password = FALSE;

	memset(username, 0, MAX_PASSWD_LENGTH);
	memset(password, 0, MAX_PASSWD_LENGTH);
	GetDlgItemText(IDC_EDIT_USENAME, str);
	if (!str.IsEmpty())
	{
		_tcsncpy(username, str, str.GetLength());
		if (_tcslen(username) == _tcslen(passwd_ctrl.use_name))
		{
			if (_tcsncmp(username, passwd_ctrl.use_name, _tcslen(username)) != 0)
			{
				//记录
				change_password = TRUE;
			}
		}
		else
		{
			//记录
			change_password = TRUE;
		}
	}
	else
	{
		
		_tcscpy(username, _T(""));
		change_password = TRUE;
	}

	GetDlgItemText(IDC_EDIT_PASSWORD, str);
	if (!str.IsEmpty())
	{
		_tcsncpy(password, str, str.GetLength());
		if (_tcslen(password) == _tcslen(passwd_ctrl.password))
		{
			if (_tcsncmp(password, passwd_ctrl.password, _tcslen(password)) != 0)
			{
				//记录
				change_password = TRUE;
			}
			
		}
		else
		{
			//记录
			change_password = TRUE;
		}
	}
	else
	{
		
		_tcscpy(password, _T(""));
		change_password = TRUE;
	}

	if (change_password )
	{
		memset(passwd_ctrl.use_name, 0, MAX_PASSWD_LENGTH);
		memset(passwd_ctrl.password, 0, MAX_PASSWD_LENGTH);
		_tcsncpy(passwd_ctrl.use_name , username, _tcslen(username));
		_tcsncpy(passwd_ctrl.password , password, _tcslen(password));
		StorePassword();
	}

	return TRUE;
}

BOOL CAnyka_TestToolDlg::OnInitDialog()
{
	CString str;
	UINT i =0;

	USES_CONVERSION;//

	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	StartTimer();//开始时间

	AfxSocketInit(NULL);
	m_uPort = 21;
	m_net_uPort = 6789;
	M_bConn = FALSE;
	m_username = (_T(""));   
    m_password = (_T("")); 
	for (i = 0; i < UPDATE_MAX_NUM; i++)
	{
		m_pInetSession[i] = NULL;
		m_pFtpConnection[i]  = NULL;
	}
	
	m_Manual_flag = FALSE;

	g_hBurnThread = INVALID_HANDLE_VALUE;
	for (i = 0; i < UPDATE_MAX_NUM; i++)
	{
		g_heatThread[i] = INVALID_HANDLE_VALUE;
		g_hBurnThread_rev_data[i] = INVALID_HANDLE_VALUE;
		g_update_finish_Thread[i] = INVALID_HANDLE_VALUE;
		g_test_pass_flag[i] = 0;
	}

	for (i = 0; i < MAX_PATH; i++)
	{
		g_test_fail_flag[i] = 0;
	}
	
	g_test_config.main_fps = 0;
	g_test_config.sub_fps = 0;
	g_test_config.qp = 0;
	g_test_config.gop = 0;

	update_or_set_IP_num = 0;

	memset(g_test_config.MAC_address, 0, MAC_ADDRESS_LEN);


	for (i = 0; i < MAX_PATH; i++)
	{
		memset(m_ip_address_buf[i], 0, IP_ADDRE_LEN);
		memset(g_test_config.m_current_config[i].Current_IP_channel_name, 0, MAX_PATH);
		memset(g_test_config.m_current_config[i].Current_IP_address_buffer, 0, IP_ADDRE_LEN);
		memset(g_test_config.m_current_config[i].Current_IP_address_ch_buffer, 0, IP_ADDRE_LEN);
		memset(g_test_config.m_current_config[i].Current_IP_address_wg_buffer, 0, IP_ADDRE_LEN);
		memset(g_test_config.m_current_config[i].Current_IP_address_dns1_buffer, 0, IP_ADDRE_LEN);
		memset(g_test_config.m_current_config[i].Current_IP_address_dns2_buffer, 0, IP_ADDRE_LEN);
	}
	m_ip_address_idex = 0;

	//初始化记录的值
	_tcscpy(g_test_config.m_IP_address, IP_ADDRE);
	_tcscpy(g_test_config.m_config_filename, TEST_CONFIG_FILE);
	g_test_config.scan_serial_flag = SCAN_SERIAL;

	ListView_SetExtendedListViewStyle(m_auto_test_config.m_hWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyle(m_no_auto_test_config.m_hWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);



	//自动化测试
	//str.Format(_T("序号"));//
	//m_auto_test_config.InsertColumn(0, str, LVCFMT_LEFT, 40);
	
	//str.Format(_T("测试项"));//
	//m_auto_test_config.InsertColumn(1, str, LVCFMT_LEFT, 100);

	//str.Format(_T("测试结果"));//
	//m_auto_test_config.InsertColumn(2, str, LVCFMT_LEFT, 90);


	//非自动化测试
	str.Format(_T("序号"));//
	m_no_auto_test_config.InsertColumn(0, str, LVCFMT_LEFT, 40);
	
	str.Format(_T("IP地址"));//
	m_no_auto_test_config.InsertColumn(1, str, LVCFMT_LEFT, 100);

	str.Format(_T("是否已更新"));//
	m_no_auto_test_config.InsertColumn(2, str, LVCFMT_LEFT, 80);
	
	//str.Format(_T("测试结果"));//
	//m_no_auto_test_config.InsertColumn(2, str, LVCFMT_LEFT, 90);


	g_test_config.auto_test_count = 0;
	g_test_config.no_auto_test_count = 0;
	g_test_config.current_auto_test_idex = 0;
	g_test_config.current_no_auto_test_idex = 0;
	g_test_config.test_pass_num = 0;
	g_test_config.test_fail_num = 0;
	g_test_config.test_pass = 0;
	g_test_config.test_fail = 0;
	g_test_config.OSD_flag = 0;
	g_test_config.time_flag = 0;
	for (i = 0; i < UPDATE_MAX_NUM; i++)
	{
		m_pInetSession[i] = NULL;
		m_pFtpConnection[i] = NULL;
	}
	
	g_update_all_finish_flag = FALSE;

	hBitmap_connet_fail = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP_CONNET_FAIL));
	hBitmap_connet_pass = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP_CONNET_PASS));
	hBitmap_wait_connet = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BITMAP_WAIT_CONNET));

	
	//memcpy(buf, "客厅", 50);
	_tcscpy(g_test_config.IP_channel_name, _T("客厅"));
	//_tcscpy(g_test_config.IP_address_buffer, _T("192.168.1.88"));
	//_tcscpy(g_test_config.IP_address_ch_buffer, _T("255.255.255.0"));
	//_tcscpy(g_test_config.IP_address_wg_buffer, _T("192.168.11.1"));
	//_tcscpy(g_test_config.IP_address_dns1_buffer, _T("223.5.5.5"));
	//_tcscpy(g_test_config.IP_address_dns2_buffer, _T("223.5.5.5"));
	


	g_test_config.m_dhcp_flag = 1;
	g_test_config.main_code_rate = 0;
	g_test_config.sub_code_rate = 0;
	SetDlgItemText(IDC_EDIT_CHANNEL_NAME, g_test_config.IP_channel_name);
	((CButton *)GetDlgItem(IDC_CHECK_DHCP))->SetCheck(g_test_config.m_dhcp_flag);

	memset(g_test_config.IP_address_buffer, 0, MAX_PATH);
	memset(g_test_config.IP_address_ch_buffer, 0, MAX_PATH);
	memset(g_test_config.IP_address_wg_buffer, 0, MAX_PATH);
	memset(g_test_config.IP_address_dns1_buffer, 0, MAX_PATH);
	memset(g_test_config.IP_address_dns2_buffer, 0, MAX_PATH);

	GetDlgItem(IDC_IPADDRESS_NET)->EnableWindow(FALSE);//
	GetDlgItem(IDC_IPADDRESS_NET_CH)->EnableWindow(FALSE);//
	GetDlgItem(IDC_IPADDRESS_NET_WG)->EnableWindow(FALSE);//
	GetDlgItem(IDC_IPADDRESS_DNS1)->EnableWindow(FALSE);//
	GetDlgItem(IDC_IPADDRESS_DNS2)->EnableWindow(FALSE);//

	
	GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(FALSE);//可用


	((CButton *)GetDlgItem(IDC_CHECK_OSD))->SetCheck(1);
	((CButton *)GetDlgItem(IDC_CHECK_TIME))->SetCheck(1);
#if 0
	//读取内部的配置文件
	g_test_config.ReadConfig(CONFIG_FILE);

	if (g_test_config.main_code_rate == 0)
	{
		SetDlgItemText(IDC_IPADDRESS_NET, _T(""));
	}
	else
	{
		str.Format(_T("%d"), g_test_config.main_code_rate);//
		SetDlgItemText(IDC_IPADDRESS_NET, str);
	}

	if (g_test_config.sub_code_rate == 0)
	{
		SetDlgItemText(IDC_IPADDRESS_NET, _T(""));
	}
	else
	{
		str.Format(_T("%d"), g_test_config.sub_code_rate);//
		SetDlgItemText(IDC_IPADDRESS_NET, str);
	}
	

	
	
	if (g_test_config.m_dhcp_flag)
	{
		//GetDlgItem(IDC_EDIT_CHANNEL_NAME)->EnableWindow(FALSE);//
		GetDlgItem(IDC_IPADDRESS_NET)->EnableWindow(FALSE);//
		GetDlgItem(IDC_IPADDRESS_NET_CH)->EnableWindow(FALSE);//
		GetDlgItem(IDC_IPADDRESS_NET_WG)->EnableWindow(FALSE);//
		GetDlgItem(IDC_IPADDRESS_DNS1)->EnableWindow(FALSE);//
		GetDlgItem(IDC_IPADDRESS_DNS2)->EnableWindow(FALSE);//
	}
	else
	{
		//GetDlgItem(IDC_EDIT_CHANNEL_NAME)->EnableWindow(TRUE);//
		GetDlgItem(IDC_IPADDRESS_NET)->EnableWindow(TRUE);//
		GetDlgItem(IDC_IPADDRESS_NET_CH)->EnableWindow(TRUE);//
		GetDlgItem(IDC_IPADDRESS_NET_WG)->EnableWindow(TRUE);//
		GetDlgItem(IDC_IPADDRESS_DNS1)->EnableWindow(TRUE);//
		GetDlgItem(IDC_IPADDRESS_DNS2)->EnableWindow(TRUE);//
	}
	
	SetDlgItemText(IDC_EDIT_CHANNEL_NAME, g_test_config.IP_channel_name);
	SetDlgItemText(IDC_IPADDRESS_NET, g_test_config.IP_address_buffer);
	SetDlgItemText(IDC_IPADDRESS_NET_CH, g_test_config.IP_address_ch_buffer);
	SetDlgItemText(IDC_IPADDRESS_NET_WG, g_test_config.IP_address_wg_buffer);
	SetDlgItemText(IDC_IPADDRESS_DNS1, g_test_config.IP_address_dns1_buffer);
	SetDlgItemText(IDC_IPADDRESS_DNS2, g_test_config.IP_address_dns2_buffer);
#endif

	//判断配置文件夹是否存在，如果不存在那么就进行创建
	if(0xFFFFFFFF == GetFileAttributes(g_test_config.ConvertAbsolutePath(TEST_CONFIG)))
	{
		CreateDirectory(g_test_config.ConvertAbsolutePath(TEST_CONFIG), NULL);//创建文件夹
	}

	//读取测试配置文件
	g_test_config.ReadConfig(g_test_config.ConvertAbsolutePath(_T("test_config/test_config.txt")));

	//write_config();	

#if 0
	//判断mac地址是否有填写
	if (g_test_config.MAC_address[0] == 0 || g_test_config.MAC_address[1] == 0
		|| g_test_config.MAC_address[2] == 0 || g_test_config.MAC_address[3] == 0 
		|| g_test_config.MAC_address[4] == 0 || g_test_config.MAC_address[5] == 0 
		|| g_test_config.MAC_address[6] == 0 || g_test_config.MAC_address[7] == 0 )
	{
		_tcsncpy(g_test_config.MAC_address, _T("02:02:03:00:00:01"), MAC_ADDRESS_LEN);
		g_test_config.Write_Config(CONFIG_FILE);
	}

	if (is_zero_ether_addr(g_test_config.MAC_address))
	{
		_tcsncpy(g_test_config.MAC_address, _T("02:02:03:00:00:01"), MAC_ADDRESS_LEN);
		g_test_config.Write_Config(CONFIG_FILE);
	}

	if (is_multicast_ether_addr(g_test_config.MAC_address))
	{
		g_test_config.MAC_address[1]++;  //广播地址不能使用
		g_test_config.Write_Config(CONFIG_FILE);
	}

	SetDlgItemText(IDC_EDIT_SHOW_TEST_INFO, _T("等待测试......"));



	if (g_test_config.scan_serial_flag == SCAN_SERIAL)
	{
		((CButton*)GetDlgItem(IDC_RADIO_NEED_SERIAL))->SetCheck(1);

		SetDlgItemText(IDC_EDIT_SERIAL_NUMBER, _T(""));
		GetDlgItem(IDC_EDIT_SERIAL_NUMBER)->EnableWindow(TRUE);//可用
	}
	else
	{
		((CButton*)GetDlgItem(IDC_RADIO_NO_NEED_SERIAL))->SetCheck(1);
		SetDlgItemText(IDC_EDIT_SERIAL_NUMBER, _T(""));
		memset(g_current_serial_number, 0, MAX_PATH);
		GetDlgItem(IDC_EDIT_SERIAL_NUMBER)->EnableWindow(FALSE);//可用
	}

	//判断序列号文件夹和序列号备份文件夹是否存在，如果不存在那么就进行创建
	if(0xFFFFFFFF == GetFileAttributes(g_test_config.ConvertAbsolutePath(SERAIL_CONFIG)))
	{
		CreateDirectory(g_test_config.ConvertAbsolutePath(SERAIL_CONFIG), NULL);//创建文件夹
	}
	if(0xFFFFFFFF == GetFileAttributes(g_test_config.ConvertAbsolutePath(SERAIL_CONFIG_BAK)))
	{
		CreateDirectory(g_test_config.ConvertAbsolutePath(SERAIL_CONFIG_BAK), NULL);//创建文件夹
	}

	//读取测试配置文件
	g_test_config.ReadConfig(g_test_config.ConvertAbsolutePath(g_test_config.m_config_filename));
	SetDlgItemText(IDC_EDIT_TEST_CONFIG, g_test_config.m_config_filename);


	g_test_config.Set_auto_test_config();

	SetDlgItemText(IDC_IPADDRESS_NET, g_test_config.m_IP_address);
	
	//显示配置信息
	set_config_item();
#endif

	//创建测试结果记录的文档
	CreateDirectory(g_test_config.ConvertAbsolutePath(_T("log")), NULL);//创建文件夹
    frmLogfile.SetFileName(_T("log\\anykatest_log.txt"));
    frmLogfile.CheckFileSize(1*1024*1024);  // delete the log file if it is larger than 512K
    
    frmLogfile.InitFile();
    frmLogfile.WriteLogFile(0,"*********************************************************************************\r\n");
    frmLogfile.WriteLogFile(LOG_LINE_TIME | LOG_LINE_DATE,"Open anyka_TestTool.exe\r\n");

	//reset_disable();
	//m_show_connet.SetBitmap(hBitmap_wait_connet);

	//create_watch_dog_heat_thread();

	SetWindowText(TOOL_VERSOIN);


	g_handle = CreateSemaphore(NULL, 1, 1, NULL);

	memset(passwd_ctrl.use_name, 0, sizeof(TCHAR)*MAX_PASSWD_LENGTH);
	memset(passwd_ctrl.password, 0, sizeof(TCHAR)*MAX_PASSWD_LENGTH);
	_tcscpy(passwd_ctrl.use_name, _T("root"));
	_tcscpy(passwd_ctrl.password, _T("")); //密码是空

	//读取密码
	GetPassword();

	//显示密码
	str.Format(_T("%s"), passwd_ctrl.use_name);//
	SetDlgItemText(IDC_EDIT_USENAME, str);
	str.Format(_T("%s"), passwd_ctrl.password);//
	SetDlgItemText(IDC_EDIT_PASSWORD, str);

	//create_connet_thread();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAnyka_TestToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAnyka_TestToolDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAnyka_TestToolDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//
BOOL CAnyka_TestToolDlg::Send_cmd(char commad_type, BOOL auto_test_flag, char *file_name, char *param, UINT idex)
{
	char *lpBuf = NULL;
	char *lpBuf_temp = NULL;
	short nBufLen = 0;
	UINT i = 0;
	int nFlags = 0;
	short len_name = 0;
	short len_data = 0;
	short check_sum_temp = 0;
	T_NET_INFO trance = {0};
	char temp[5] = {0};
	int ret = 0;
	BOOL no_data_flag = TRUE;
	BOOL no_filename_flag = TRUE;
	

	USES_CONVERSION;

	trance.data_param = NULL;
	trance.file_name = NULL;


	nBufLen = 2;
	trance.commad_type = commad_type;
	nBufLen += 1;
	check_sum_temp += commad_type;

	trance.auto_test_flag = auto_test_flag;
	nBufLen += 1;
	check_sum_temp += auto_test_flag;

	if (file_name != NULL)
	{
		for (i = 0; i < MAX_PATH && file_name[i] != 0; i++)
		{
			if(file_name[i] != '0')
			{
				no_filename_flag = FALSE;
				break;
			}
		}
	}
	
	if (file_name != NULL && !no_filename_flag)
	{
		len_name = strlen(file_name);
		if(len_name != 0 )
		{
			trance.file_name = (char *)malloc(len_name + 2 + 1);
			if (trance.file_name == NULL)
			{
				return FALSE;
			}
			memset(trance.file_name, 0, len_name + 2 + 1);

			strncpy(trance.file_name, (char *)&len_name, 2);
			strncpy(&trance.file_name[2], file_name, len_name);
			nBufLen += (len_name + 2);
			for (i = 0; i < (UINT)(len_name + 2); i++)
			{
				check_sum_temp += trance.file_name[i];
			}
		}
	}
	else
	{
		nBufLen += 2;
	}
	
	if (param != NULL)
	{
		for (i = 0; i < MAX_PATH && param[i] != 0; i++)
		{
			if(param[i] != '0')
			{
				no_data_flag = FALSE;
				break;
			}
		}
	}

	if (param != NULL && !no_data_flag)
	{
		len_data = strlen(param);
		if(len_data != 0 )
		{
			trance.data_param = (char *)malloc(len_data + 2+1);
			if (trance.data_param == NULL)
			{
				return FALSE;
			}
			memset(trance.data_param, 0, len_data + 2 + 1);

			strncpy(trance.data_param, (char *)&len_data, 2);
			strncpy(&trance.data_param[2], param, len_data);
			nBufLen += (len_data + 2);
			for (i = 0; i < (UINT)(len_data + 2); i++)
			{
				check_sum_temp += trance.data_param[i];
			}
		}
	}
	else
	{
		nBufLen += 2;
	}

	trance.check_sum = check_sum_temp;
	nBufLen +=  2;
	trance.len = nBufLen;


	lpBuf = (char *)malloc(nBufLen + 1);
	if (lpBuf == NULL)
	{
		return FALSE;
	}
	
	//打包
	strncpy(lpBuf, (char *)&trance.len, 2);
	strncpy(&lpBuf[2], &trance.commad_type, 1);
	strncpy(&lpBuf[3], (char *)&trance.auto_test_flag, 1);
	if (len_name != 0 && file_name != NULL && !no_filename_flag)
	{
		strncpy(&lpBuf[4], (char *)&len_name, 2);
		strncpy(&lpBuf[4+2], file_name, len_name);
		//strncpy(&lpBuf[5], trance.file_name, len_name+4);
	}
	else
	{
		strncpy(&lpBuf[4], temp, 2);
	}
	len_name = len_name + 2;//因为增加4个字节的长度


	if (len_data != 0 && param != NULL && !no_data_flag)
	{
		strncpy(&lpBuf[4+len_name], (char *)&len_data, 2);
		strncpy(&lpBuf[4+len_name+2], param, len_data);
		//strncpy(&lpBuf[5+len_name+4], trance.data_param, len_data+4);
	}
	else
	{
		strncpy(&lpBuf[4 + len_name], temp, 2);
	}
	len_data = len_data + 2;  //因为增加4个字节的长度


	strncpy(&lpBuf[4+len_name+len_data], (char *)&trance.check_sum, 2);
	

	ret = m_ClientSocket.Socket_Send(lpBuf, nBufLen, idex);
	
	if (!ret)
	{
		return FALSE;
	}
	
	if (trance.data_param)
	{
		free(trance.data_param);
	}

	if (trance.file_name)
	{
		free(trance.file_name);
	}

	if (lpBuf)
	{
		free(lpBuf);
	}

	if (lpBuf_temp)
	{
		free(lpBuf_temp);
	}

	return TRUE;
}


BOOL CAnyka_TestToolDlg::decode_command(char *lpBuf, char *commad_type, char *file_name, char *param)
{
	//重新定义结构
	int len_temp = 0;
	int param_len_temp = 0;
	int nFlags = 0;
	T_NET_INFO trance = {0};
	int ret = 0, i = 0;
	char auto_test_flag = 0;
	short check_sum = 0;
	short check_sum_temp = 0;
	
	
	//lpBuf结构是按T_NET_INFO结构排放的
	strncpy((char *)&trance.len, lpBuf, 2);
	
	strncpy(&trance.commad_type, &lpBuf[2], 1);
	*commad_type = trance.commad_type;
	
	strncpy((char *)&trance.auto_test_flag, &lpBuf[3], 1);
	auto_test_flag = trance.auto_test_flag;
	
	strncpy((char *)&len_temp, &lpBuf[4], 2);
	if (len_temp > 0 && file_name != NULL)
	{
		strncpy(file_name, &lpBuf[6], len_temp);
	}
	
	strncpy((char *)&param_len_temp, &lpBuf[6 + len_temp], 2);
	if (param_len_temp > 0 && param != NULL)
	{
		strncpy(param, &lpBuf[8 + len_temp], param_len_temp);
	}
	
	strncpy((char *)&check_sum, &lpBuf[8 + len_temp + param_len_temp], 2);
	
	//检测包
	for (i = 2; i < trance.len - 2; i++)
	{
		check_sum_temp += lpBuf[i];
	}
	
	if (check_sum != check_sum_temp)
	{
		return FALSE;
	}
	
	return TRUE;
}

DWORD WINAPI check_update_finish_thread(LPVOID lpParameter)
{
	char commad_type; 
	CClientSocket m_ClientSocket_heat;
	int ret = 0;
	char lpBuf[256] = {0};
	UINT nBufLen = 256;
	UINT time1 = 0;
	UINT time2 = 0;
	UINT idex = 0;

	memcpy(&idex, lpParameter, 4);
	
	//获取心跳命令
	while (1)
	{
		Sleep(3000);
		if (g_update_finish_Thread[idex] != INVALID_HANDLE_VALUE)
		{
			commad_type = 0;  //初始化
			ret = m_ClientSocket_heat.Socket_Receive_update_finish(lpBuf, nBufLen, idex);
			if (ret == -1)
			{
				time2 = GetTickCount();
				//if (time2 - time1 > 6000)
				{
					g_update_pass_flag[idex] = 1;
					return TRUE;
				}
			}
			else
			{
				time1 = GetTickCount();
			}
		}
		
	}
	
	
	return 1;
}

BOOL CAnyka_TestToolDlg::create_thread_update_finish(UINT idex) 
{
	memcpy(update_param[idex], &idex, sizeof(UINT));

	g_update_finish_Thread[idex] = CreateThread(NULL, 0, check_update_finish_thread, update_param[idex], 0, NULL);
	if (g_update_finish_Thread[idex] == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	return TRUE;
}

void CAnyka_TestToolDlg::close_thread_update_finish(UINT idex) 
{
	if(g_update_finish_Thread[idex] != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_update_finish_Thread[idex]);
		g_update_finish_Thread[idex] = INVALID_HANDLE_VALUE;
	}
}

DWORD WINAPI check_rev_date_thread(LPVOID lpParameter)
{
	char commad_type; 
	char *file_name = NULL;
	char *param = NULL;
	CAnyka_TestToolDlg  TestToolDlg;
	CClientSocket m_ClientSocket_heat;
	int ret = 0;
	char lpBuf[256] = {0};
	UINT nBufLen = 256;
	UINT time1 = 0;
	UINT time2 = 0;
	char g_param[256] = {0};
	UINT idex = 0;
	UINT *buf_temp = (UINT *)lpParameter;

	memcpy(&idex, buf_temp, sizeof(UINT));
	
	//获取心跳命令
	while (1)
	{
		Sleep(500);
		if (g_hBurnThread_rev_data[idex] != INVALID_HANDLE_VALUE)
		{
			if (g_send_commad[idex] == 1)
			{
				commad_type = 0;  //初始化
				g_heat_enter_flag = TRUE;
				//time1 = GetTickCount();
				ret = m_ClientSocket_heat.Socket_Receive(lpBuf, nBufLen, idex);
				//frmLogfile_temp.WriteLogFile(0,"Socket_Receive ret:%d\n", ret);
				if (ret == -1)
				{
					frmLogfile.WriteLogFile(0,"error Socket_Receive ret:%d\n", ret);
					if (g_hBurnThread_rev_data[idex] == INVALID_HANDLE_VALUE && g_send_commad[idex] == 0)
					{
						return TRUE;
					}
					g_test_fail_flag[idex]  = 1;
					g_test_pass_flag[idex] = 2;
					//g_connet_success_flag = FALSE;
					AfxMessageBox(_T("接收数据错误，请重启小机再连接"));
					if (g_hBurnThread_rev_data[idex] != INVALID_HANDLE_VALUE)
					{
						CloseHandle(g_hBurnThread_rev_data[idex]);
						g_hBurnThread_rev_data[idex] = INVALID_HANDLE_VALUE;
					}
					g_send_commad[idex] = 0;
					return FALSE;
				}
				//lpBuf结构是按T_NET_INFO结构排放的
				strncpy(&commad_type, &lpBuf[2], 1);
				if(commad_type == TEST_RESPONSE)
				{
					//frmLogfile_temp.WriteLogFile(0,"commad_type:%d\n", commad_type);
					memset(g_param, 0, 256);
					if (!TestToolDlg.decode_command(lpBuf, &g_commad_type, NULL, g_param))
					{
						frmLogfile.WriteLogFile(0,"decode_command fail:%d\n", g_commad_type);
						g_test_pass_flag[idex] = 2;	
					}
					else
					{
						if (g_param[0] == 49)  //49 表示1
						{
							frmLogfile.WriteLogFile(0,"pass g_param[0]:%d\n", g_param[0]);
							g_test_pass_flag[idex] = 1;
						}
						else
						{
							frmLogfile.WriteLogFile(0,"fail g_param[0]:%d\n", g_param[0]);
							g_test_pass_flag[idex] = 2;
						}
					}
				}
				g_heat_flag = TRUE;
			}
		}
	}
	
	
	return 1;
}



DWORD WINAPI check_heartbeat_thread(LPVOID lpParameter)
{
	char commad_type; 
	char *file_name = NULL;
	char *param = NULL;
	CAnyka_TestToolDlg  TestToolDlg;
	CClientSocket m_ClientSocket_heat;
	int ret = 0;
	char lpBuf[256] = {0};
	UINT nBufLen = 256;
	UINT time1 = 0;
	UINT time2 = 0;
	char g_param[256] = {0};
	UINT idex = 0;
	UINT *buf_temp = (UINT *)lpParameter;

	memcpy(&idex, buf_temp, 4);

	time1 = GetTickCount();
	time2 = GetTickCount();
	//获取心跳命令
	while (1)
	{
		Sleep(4000);
		if (g_heatThread[idex] != INVALID_HANDLE_VALUE)
		{
			if (g_send_commad[idex] == 1)
			{
				commad_type = 0;  //初始化
				g_heat_enter_flag = TRUE;
				//time1 = GetTickCount();
				ret = m_ClientSocket_heat.Heat_Socket_Receive(lpBuf, nBufLen, idex);
				//frmLogfile_temp.WriteLogFile(0,"Socket_Receive ret:%d\n", ret);
				if (ret == -1)
				{
					//g_connet_success_flag = FALSE;
					frmLogfile.WriteLogFile(0,"error Heat_Socket_Receive ret:%d\n", ret);
					if (g_heatThread[idex] == INVALID_HANDLE_VALUE && g_send_commad[idex] == 0)
					{
						return TRUE;
					}
					g_test_fail_flag[idex]  = 1;
					g_test_pass_flag[idex] = 2;
					AfxMessageBox(_T("心跳接收数据错误，请重启小机再连接"));
					if (g_heatThread[idex] != INVALID_HANDLE_VALUE)
					{
						CloseHandle(g_heatThread[idex]);
						g_heatThread[idex] = INVALID_HANDLE_VALUE;
					}
					g_send_commad[idex] = 0;

					return FALSE;
				}
				strncpy(&commad_type, &lpBuf[2], 1);

				if (commad_type == TEST_HEARTBEAT)
				{
					time2 = GetTickCount();
				}
				else
				{
					time1 = GetTickCount();
					if (time1 - time2 > 10000)
					{
						g_test_fail_flag[idex]  = 1;
						g_test_pass_flag[idex] = 2;
						//g_connet_success_flag = FALSE;
						AfxMessageBox(_T("心跳小机没有正常动行，请检查,并重新连接测试"));
						return FALSE;
					}
					continue;
				}
				g_heat_flag = TRUE;
			}
		}
	}
	

	return 1;
}

BOOL CAnyka_TestToolDlg::create_thread_rev_data(UINT idex) 
{
	

	if (g_hBurnThread_rev_data[idex] != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_rev_data[idex]);
		g_hBurnThread_rev_data[idex] = INVALID_HANDLE_VALUE;
	}
	
	//param = &idex;
	memcpy(rve_param[idex], &idex, sizeof(UINT));
	g_hBurnThread_rev_data[idex] = CreateThread(NULL, 0, check_rev_date_thread, rve_param[idex], 0, NULL);
	if (g_hBurnThread_rev_data[idex] == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	return TRUE;
}

void CAnyka_TestToolDlg::close_thread_rev_data(UINT idex) 
{
	if(g_hBurnThread_rev_data[idex] != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_rev_data[idex]);
		g_hBurnThread_rev_data[idex] = INVALID_HANDLE_VALUE;
	}
}


BOOL CAnyka_TestToolDlg::create_thread_heat(LPCTSTR addr, UINT idex) 
{
	USES_CONVERSION;
	
	//创建一个心跳线程的socket
	if (!m_ClientSocket.Heat_Socket_Create(idex))
	{
		AfxMessageBox(_T("创建心跳sokect失败"), MB_OK);
		return FALSE;
	}
	
	
	//if(m_ClientSocket.Connect(addr, 6789))
	if(!m_ClientSocket.Heat_Socket_Connect(T2A(addr), m_net_uPort + 1, idex))	
	{
		AfxMessageBox(_T("sokect连接失败"), MB_OK);
		return FALSE;
	}
	
	if (g_heatThread[idex] != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_heatThread[idex]);
		g_heatThread[idex] = INVALID_HANDLE_VALUE;
	}
	
	//param = &idex;
	memcpy(heat_param[idex], &idex, sizeof(UINT));
	g_heatThread[idex] = CreateThread(NULL, 0, check_heartbeat_thread, heat_param[idex], 0, NULL);
	if (g_heatThread[idex] == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(_T("创建心跳线程失败"), MB_OK);
		return FALSE;
	}
	
	return TRUE;
}

void CAnyka_TestToolDlg::close_thread_heat(UINT idex) 
{
	if(g_heatThread[idex] != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_heatThread[idex]);
		g_heatThread[idex] = INVALID_HANDLE_VALUE;
	}
}

DWORD WINAPI connet_server_thread(LPVOID lpParameter)
{
	CAnyka_TestToolDlg  TestToolDlg;
	while(1)
	{
	//判断是否连接成功
		if (!g_connet_success_flag && g_IPaddress_corret_flag && g_hBurnThread_connet != INVALID_HANDLE_VALUE)
		{
			if (TestToolDlg.Anyka_Test_Sever_Connet())
			{
				g_connet_success_flag = TRUE;
			}
			else
			{
				TestToolDlg.Anyka_Test_Sever_close();
				g_connet_success_flag = FALSE;
			}

		}
	}

	return 1;
}

BOOL CAnyka_TestToolDlg::create_connet_thread() 
{
	if (g_hBurnThread_connet != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_connet);
		g_hBurnThread_connet = INVALID_HANDLE_VALUE;
	}

	g_hBurnThread_connet = CreateThread(NULL, 0, connet_server_thread, 0, 0, NULL);
	if (g_hBurnThread_connet == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	return TRUE;
}

void CAnyka_TestToolDlg::close_connet_thread() 
{
	if(g_hBurnThread_connet != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_connet);
		g_hBurnThread_connet = INVALID_HANDLE_VALUE;
	}
}

DWORD WINAPI watch_dog_heat_thread(LPVOID lpParameter)
{
	CAnyka_TestToolDlg  TestToolDlg;
	int cnt = 0;

	while (1)
	{
		Sleep(5000);
		if (g_hBurnThread_watch_dog_heat != INVALID_HANDLE_VALUE)
		{

			//连接成功。并进入心跳
			if (g_connet_success_flag && g_heat_enter_flag)
			{
				Sleep(600000);
				if (!g_heat_flag)
				{
					AfxMessageBox(_T("关闭线程"));
					frmLogfile.WriteLogFile(0,"Anyka_Test_Sever_close########## d\n");
					//关掉线程
					g_heat_flag = TRUE;
					g_heat_enter_flag = FALSE;
					g_connet_success_flag = FALSE;
					//g_test_pass_flag[idex] = 2;
					TestToolDlg.Anyka_Test_Sever_close();
					g_close_Thread_flag = TRUE;
					
				}
				g_heat_flag = FALSE;
			}
		}
	}
	return 1;
}


BOOL CAnyka_TestToolDlg::create_watch_dog_heat_thread() 
{
	if (g_hBurnThread_watch_dog_heat != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_watch_dog_heat);
		g_hBurnThread_watch_dog_heat = INVALID_HANDLE_VALUE;
	}
	
	g_hBurnThread_watch_dog_heat = CreateThread(NULL, 0, watch_dog_heat_thread, 0, 0, NULL);
	if (g_hBurnThread_watch_dog_heat == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	return TRUE;
}

void CAnyka_TestToolDlg::close_watch_dog_heat_thread() 
{
	if(g_hBurnThread_watch_dog_heat != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_watch_dog_heat);
		g_hBurnThread_watch_dog_heat = INVALID_HANDLE_VALUE;
	}
}


DWORD WINAPI Creat_Anyka_Test_main(LPVOID lpParameter) 
{

	return 1;
}




BOOL CAnyka_TestToolDlg::Download_serialfile_toftp(void)
{
	
	return TRUE;
}

void CAnyka_TestToolDlg::Get_serialnum_bycom() 
{

	USES_CONVERSION;

	if (g_test_config.scan_serial_flag == SCAN_SERIAL)
	{
		//打开串口
		com.trancecom_Close();
		if (!com.trancecom_Open(g_test_config.com_type, 9600))
		{
			AfxMessageBox(_T("打开串口com1失败，请检查com1是否已用或不正常"));
			return;
		}
	}
}
void CAnyka_TestToolDlg::Anyka_Test_Sever_close(void)
{
	//Send_cmd(TEST_COMMAND_FINISH, 0, NULL, NULL);

	
}

BOOL CAnyka_TestToolDlg::Anyka_Test_get_IPAddress(void)
{
	CString addr;

	UpdateData(TRUE);
	
	if(m_ipaddress_net.IsBlank())
	{
		m_show_connet.SetBitmap(hBitmap_wait_connet);
		//AfxMessageBox(_T("ip address err "));
		return FALSE;
	}

	m_ipaddress_net.GetAddress(m_f0, m_f1, m_f2, m_f3);
	addr.Format(_T("%d.%d.%d.%d"), m_f0, m_f1, m_f2, m_f3);
	_tcscpy(g_test_config.m_IP_address, addr);//重新记录

	return TRUE;
}

BOOL CAnyka_TestToolDlg::Anyka_Test_Sever_Connet(void)
{
	return TRUE;
}


void CAnyka_TestToolDlg::OnButtonConnetServer() 
{
	// TODO: Add your control notification handler code here

}

BOOL CAnyka_TestToolDlg::Connet_FTPServer(LPCTSTR addr, UINT idex) 
{

	

	TCHAR username[MAX_PASSWD_LENGTH+1] = {0};
	TCHAR password[MAX_PASSWD_LENGTH+1] = {0};
	CString str;
	BOOL change_password = FALSE;


	USES_CONVERSION;

	if(m_pInetSession[idex] != NULL)
	{
		delete m_pInetSession[idex];
		m_pInetSession[idex] = NULL;
	}
	m_pInetSession[idex] = new CInternetSession(AfxGetAppName(), 1, PRE_CONFIG_INTERNET_ACCESS);
	try
	{
		// addr       ftp服务器的地址  LPCTSTR
		// username   登陆用户名       LPCTSTR 
		// password   密码            LPCTSTR
		// port       端口            UINT

		//m_username = (_T("root"));   
		//m_password = (_T("")); 

		if(m_pFtpConnection[idex] != NULL)
		{
			m_pFtpConnection[idex]->Close();
			delete m_pFtpConnection[idex];
			m_pFtpConnection[idex] = NULL;
		}
		m_pFtpConnection[idex] = m_pInetSession[idex]->GetFtpConnection(addr, passwd_ctrl.use_name, passwd_ctrl.password, m_uPort);
		//m_pFtpConnection = m_pInetSession->GetFtpConnection(addr, m_username, m_password, m_uPort);
	}
	catch(CInternetException *pEx)//若登陆不成功则抛出异常，以下是针对异常的处理
	{
		TCHAR szError[1024] = {0};

		if(pEx->GetErrorMessage(szError,1024))
		{
			AfxMessageBox(szError);
		}
		else
		{
			AfxMessageBox(_T("There was an exception"));
		}
		pEx->Delete();
		m_pFtpConnection[idex] = NULL;
		return FALSE;
    }
//	if (m_pFtpConnection != NULL)   	
//	{
		//UpdateData(FALSE);   
    //}
	return TRUE;
}


BOOL CAnyka_TestToolDlg::find_file_indir(TCHAR *file_name, UINT *name_len) 
{

	CFileFind ff;
    CString filename;
    CString szDir;
	DWORD  len = 0;
	UINT file_cnt = 0;

	szDir.Format(_T("%s/*"), g_test_config.ConvertAbsolutePath(UPDATE_CONFIG_DIR));
    BOOL res = ff.FindFile(szDir);
    while( res )
    {
        res = ff.FindNextFile();
		filename = ff.GetFileName();
		len = ff.GetLength();

        if(!ff.IsDirectory() && !ff.IsDots())
        {
			_tcscpy(file_name, filename);
			file_cnt++;
        }
    }

	*name_len = len;
    ff.Close(); 
	if (file_cnt == 1)
	{
		 return TRUE;
	}
	else
	{
		 return FALSE;
	}
}


BOOL CAnyka_TestToolDlg::Anyka_check_update_finish(UINT idex)
{
	UINT time1 = 0;
	UINT time2 = 0;
	UINT delaytime = 0;
	
	USES_CONVERSION;
	
	time1 = GetTickCount();
	while (1)
	{
		time2 = GetTickCount();
		Sleep(50);
		delaytime = 480000;
		if (time2 - time1 > delaytime)
		{
			g_update_pass_flag[idex] = 0;
			AfxMessageBox(_T("超时(30s)没有返回确认命令"));  
			return FALSE;
		}
		if (g_update_pass_flag[idex] == 1)
		{
			g_update_pass_flag[idex] = 0;
			return TRUE;
		}
	}
	
}


BOOL CAnyka_TestToolDlg::Anyka_Test_check_info(TCHAR *test_name, BOOL update_flag, UINT idex)
{
	UINT time1 = 0;
	UINT time2 = 0;
	UINT delaytime = 0;

	USES_CONVERSION;

	time1 = GetTickCount();
	while (1)
	{
		time2 = GetTickCount();
		Sleep(50);
		if (update_flag)
		{
			delaytime = 480000;
		}
		else
		{
			delaytime = 20000;
		}
		if (time2 - time1 > delaytime)
		{
			g_test_pass_flag[idex] = 0;
			AfxMessageBox(_T("超时(30s)没有返回确认命令"));  
			frmLogfile.WriteLogFile(0,"%s test fail\n", test_name);
			return FALSE;
		}
		if (g_test_pass_flag[idex] == 1)
		{
			
			g_test_pass_flag[idex] = 0;
			frmLogfile.WriteLogFile(0,"%s test success\n", T2A(test_name));
			return TRUE;
		}
		else if (g_test_pass_flag[idex] == 2)
		{
			g_test_pass_flag[idex] = 0;
			frmLogfile.WriteLogFile(0,"%s test fail\n", T2A(test_name));
			return FALSE;
		}
	}

}

void CAnyka_TestToolDlg::add_num_test(BOOL test_pass_flag, BOOL reap_test_flag)
{
	if (test_pass_flag)
	{
		g_test_config.test_pass_num++;
		if (reap_test_flag)
		{
			g_test_config.test_fail_num--;
		}
	}
	else
	{
		g_test_config.test_fail_num++;
		if (reap_test_flag)
		{
			g_test_config.test_pass_num--;
		}
	}
}

//小写转成大小
VOID CAnyka_TestToolDlg::lower_to_upper(TCHAR *surbuf, TCHAR *dstbuf)
{
	UINT i = 0;
	UINT nlen = 0;
	
	//获取字符串的长度
	while (surbuf[nlen] != NULL)
	{
		nlen++;
	}
	
	for (i = 0; i < nlen; i++)
	{
		if(islower(surbuf[i]))//如果是小写
		{
			dstbuf[i] = toupper(surbuf[i]); //转大
		}
		else
		{
			dstbuf[i] = surbuf[i]; //直接附值
		}
	}
}

BOOL CAnyka_TestToolDlg::Otp_mac_add(TCHAR *surbuf, TCHAR *dstbuf)
{
	TCHAR tmpBuf[MAC_ADDRESS_LEN+1] = {0};
	CHAR tmpBuf_temp[MAC_ADDRESS_LEN+1] = {0};
	TCHAR tmpBuf_1[MAC_ADDRESS_LEN+1] = {0};
	int  surbuf_len = 0;
	int  i = 0;
	int idex = 0;
	BOOL flag = FALSE;
	UINT tempmac = 0;
	
	lower_to_upper(surbuf, tmpBuf);
	surbuf_len = wcslen(surbuf);
	for (i = surbuf_len - 1; i >= 0; i--)
	{
		idex = tmpBuf[i];
		if (tmpBuf[i] == 58)// :
		{
			continue;
			
		}
		else if (tmpBuf[i] != 70) 
		{
			sprintf(tmpBuf_temp, "%c", tmpBuf[i]);
			
			//地址递增一
			sscanf(tmpBuf_temp, "%x", &tempmac);
			tempmac ++;
			swprintf(tmpBuf_1, _T("%x"),tempmac);

			_tcsncpy(&tmpBuf[i], &tmpBuf_1[0], 1);
			flag = TRUE;
			break;
		}
		else
		{
			tmpBuf[i] = 48;
			continue;
		}
	}
	_tcsncpy(dstbuf, tmpBuf, MAC_ADDRESS_LEN);
	
	if (flag == FALSE)
	{
		return FALSE;
	}
	return TRUE;
}

//mac地址加一
BOOL CAnyka_TestToolDlg::Mac_Addr_add_1(TCHAR *buf_temp)
{
	TCHAR tempbuf[MAC_ADDRESS_LEN+1] = {0};
	TCHAR tempAddrBuf[MAC_ADDRESS_LEN+1] = {0};
	TCHAR dstAddrBuf[MAC_ADDRESS_LEN+1] = {0};
	//UINT tempmac;

	//对低位进行十六进制加一
	//swprintf(tempbuf, _T("%c%c%c%c%c%c%c%c%c%c%c%c"), buf_temp[0], buf_temp[1], buf_temp[3], buf_temp[4], buf_temp[6]
	//							   , buf_temp[7], buf_temp[9], buf_temp[10], buf_temp[12], buf_temp[13]
	//							   , buf_temp[15], buf_temp[16]);
	
	//地址递增一
	Otp_mac_add(g_test_config.MAC_address, tempAddrBuf);
	//sscanf(tempbuf, "%x", &tempmac);
	//tempmac ++;
	//sprintf(tempbuf, "%06x", tempmac);

	//swprintf(tempAddrBuf, _T("%c%c:%c%c:%c%c:%c%c:%c%c:%c%c"), dstAddrBuf[0],dstAddrBuf[1],dstAddrBuf[2],dstAddrBuf[3]
	//										   ,dstAddrBuf[4],dstAddrBuf[5],dstAddrBuf[6],dstAddrBuf[7]
	//										   ,dstAddrBuf[8],dstAddrBuf[9],dstAddrBuf[10],dstAddrBuf[11]);
	memset(dstAddrBuf, 0, MAC_ADDRESS_LEN);
	lower_to_upper(tempAddrBuf, dstAddrBuf);
	_tcscpy(g_test_config.MAC_address, dstAddrBuf);  //记录当前的mac地址
	
	return TRUE;
}


void CAnyka_TestToolDlg::OnButtonLoadConfig() 
{
	// TODO: Add your control notification handler code here
	DWORD ret = 0;
	CString strSourceName, strDestName;
	CString str;
    CFileDialog ldFile(true,_T("*.*"),_T("*.*")); //true表示打开文件  
	
    if (ldFile.DoModal() == IDOK)   
    {   
        strSourceName = ldFile.GetPathName();//其中路径包括目录和文件名   
		g_test_config.ReadConfig(strSourceName);
	
		memset(g_test_config.m_config_filename, 0, MAX_PATH);
		_tcscpy(g_test_config.m_config_filename, TEST_CONFIG_DIR);
		_tcscat(g_test_config.m_config_filename, ldFile.GetFileName());

		SetDlgItemText(IDC_EDIT_TEST_CONFIG, ldFile.GetFileName());
		g_test_config.Set_auto_test_config();
		
		//显示配置信息
		set_config_item();

		reset_disable();


		GetDlgItem(IDC_BUTTON_LOAD_CONFIG)->EnableWindow(TRUE);//不可用

		if (g_test_config.scan_serial_flag == SCAN_SERIAL)
		{
			GetDlgItem(IDC_EDIT_SERIAL_NUMBER)->EnableWindow(TRUE);//可用
		}
		else
		{
			GetDlgItem(IDC_EDIT_SERIAL_NUMBER)->EnableWindow(FALSE);//可用
		}
		
		if (g_test_config.no_auto_test_count == 0)
		{
			GetDlgItem(IDC_BUTTON_AUTO_TEST)->EnableWindow(TRUE);//可用
		}
		else
		{
			GetDlgItem(IDC_BUTTON_NEXT_TEST)->EnableWindow(TRUE);//可用
		}

    }
}

void CAnyka_TestToolDlg::OnRadioNeedSerial() 
{
	// TODO: Add your control notification handler code here
	g_test_config.scan_serial_flag = SCAN_SERIAL;
	memset(g_current_serial_number, 0, MAX_PATH);
	Get_serialnum_bycom();
	GetDlgItem(IDC_EDIT_SERIAL_NUMBER)->EnableWindow(TRUE);//可用
}

void CAnyka_TestToolDlg::OnRadioNoNeedSerial() 
{
	// TODO: Add your control notification handler code here
	g_test_config.scan_serial_flag = NO_SCAN_SERIAL;
	SetDlgItemText(IDC_EDIT_SERIAL_NUMBER, _T(""));
	memset(g_current_serial_number, 0, MAX_PATH);
	GetDlgItem(IDC_EDIT_SERIAL_NUMBER)->EnableWindow(FALSE);//可用
}

void CAnyka_TestToolDlg::OnChangeEditSerialNumber() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

	CString str;
	
	GetDlgItemText(IDC_EDIT_SERIAL_NUMBER, str);
	_tcsncpy(g_test_config.m_serial_number, str, MAX_PATH);//工程名
	
}


void  CAnyka_TestToolDlg::StartTimer()
{
    used_time = 0;
    ctrl_time = SetTimer(1, 1000, NULL);//设置时间
}

void  CAnyka_TestToolDlg::StopTimer()
{
    KillTimer(ctrl_time);//清空时间
    ctrl_time = 0;
}


void CAnyka_TestToolDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	UINT i = 0, j = 0;
	CString   strTemp;  
	
	if(g_sousuo_flag)
	{
		//g_sousuo_show_flag = FALSE;
		//m_ip_address_idex = 2;
		//_tcscpy(m_ip_address_buf[0], _T("172.22.5.12"));
		//_tcscpy(m_ip_address_buf[1], _T("172.22.5.12"));

		m_no_auto_test_config.DeleteAllItems();
		for(i = 0; i < m_ip_address_idex; i++)
		{
			strTemp.Format(_T("%d"), i+1);
			m_no_auto_test_config.InsertItem(i, strTemp);
			m_no_auto_test_config.SetItemText(i, 1, m_ip_address_buf[i]);
			m_no_auto_test_config.SetItemText(i, 2, _T(""));
		}
	}


	if(g_update_all_flag || g_update_one_flag)
	{
		//g_sousuo_show_flag = FALSE;
		m_no_auto_test_config.DeleteAllItems();
		if (!m_Manual_flag)
		{
			for(i = 0; i < m_ip_address_idex; i++)
			{
				strTemp.Format(_T("%d"), i+1);
				m_no_auto_test_config.InsertItem(i, strTemp);
				
				m_no_auto_test_config.SetItemText(i, 1, m_ip_address_buf[i]);
				
				if (m_update_flag[i] == 1)
				{
					m_no_auto_test_config.SetItemText(i, 2, _T("已升级"));
				}
				else if (m_update_flag[i] == 2)
				{
					m_no_auto_test_config.SetItemText(i, 2, _T("升级失败"));
				}
				else if (m_update_flag[i] == 3)
				{
					m_no_auto_test_config.SetItemText(i, 2, _T("升级中"));
				}
				else if (m_update_flag[i] == 4)
				{
					m_no_auto_test_config.SetItemText(i, 2, _T("正在下载升级包"));
				}
				else
				{
					if (g_update_all_flag)
					{
						m_no_auto_test_config.SetItemText(i, 2, _T("等待升级"));
					}
					else
					{
						m_no_auto_test_config.SetItemText(i, 2, _T(""));
					}
					
				}
			}
		}
		else
		{
			if (m_update_flag[i] == 1)
			{
				SetDlgItemText(IDC_STATIC_STAUT, _T("已升级"));//可用
			}
			else if (m_update_flag[i] == 2)
			{
				SetDlgItemText(IDC_STATIC_STAUT, _T("升级失败"));//可用
			}
			else if (m_update_flag[i] == 3)
			{
				SetDlgItemText(IDC_STATIC_STAUT, _T("升级中"));//可用
			}
			else if (m_update_flag[i] == 4)
			{
				SetDlgItemText(IDC_STATIC_STAUT, _T("下载升级包"));//可用
			}
		}
	}
	if (m_not_find_anyIP)
	{
		m_not_find_anyIP = FALSE;
		M_bConn = FALSE;
		GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
		AfxMessageBox(_T("没有搜索到任何设备!"));
	}

	if ((m_find_IP_end_flag && m_ip_address_idex > 0))
	{
		g_sousuo_flag = FALSE;
		M_bConn = FALSE;
		if (!g_IP_same_flag)
		{
			for (i = 0; i < m_ip_address_idex - 1; i++)
			{
				for (j = i+1; j < m_ip_address_idex; j++)
				{
					if (_tcscmp(m_ip_address_buf[i], m_ip_address_buf[j]) == 0)
					{
						g_IP_same_flag = TRUE;
						AfxMessageBox(_T("存在有设备的IP相同的情况，请检查"), MB_OK);
						break;
					}
				}
				if (g_IP_same_flag)
				{
					break;
				}
			}
		}
		
		if (g_IP_same_flag)
		{
			//GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(FALSE);//可用
			GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(FALSE);//可用
			GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(FALSE);//可用
			GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(FALSE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(FALSE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(FALSE);//可用
		}
		else
		{
			GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
		}
		GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
		
	}

	if (g_update_all_finish_flag)
	{
		g_update_all_flag = FALSE;
		GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
		if (g_hupdateThread != INVALID_HANDLE_VALUE)
		{
			Close_Anyka_update_thread();
			g_hupdateThread = INVALID_HANDLE_VALUE;
		}
	}

	if (g_update_one_finish_flag)
	{
		g_update_one_flag = FALSE;
		GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
		if (g_hupdateThread != INVALID_HANDLE_VALUE)
		{
			Close_Anyka_update_thread();
			g_hupdateThread = INVALID_HANDLE_VALUE;
		}
	}
	

	if(g_update_all_flag && m_ip_address_idex > 0)
	{
		GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(FALSE);//可用
	}

	CDialog::OnTimer(nIDEvent);
}

void CAnyka_TestToolDlg::OnButtonTest() 
{
	// TODO: Add your control notification handler code here
	set_test_staut(g_test_config.current_no_auto_test_idex, FALSE, TEST_ING);
}


void CAnyka_TestToolDlg::show_IP_info()
{
	UINT i = 0;
	CString str;

	if (m_no_auto_test_config.GetSelectedCount() < 1)
	{
		return;
	}
	
	for (i = m_no_auto_test_config.GetItemCount()-1; i >=0; i--)
	{
		if(m_no_auto_test_config.GetItemState(i, LVIS_SELECTED))
		{
			str.Format(_T("%d"), i+1);
			SetDlgItemText(IDC_EDIT_CURENT_IP, str);
			SetDlgItemText(IDC_EDIT_CHANNEL_NAME, g_test_config.m_current_config[i].Current_IP_channel_name);
			if (g_test_config.m_current_config[i].Current_IP_address_dhcp_flag == 0x30) //0x30表示0， 
			{
				((CButton *)GetDlgItem(IDC_CHECK_DHCP))->SetCheck(0);
				GetDlgItem(IDC_IPADDRESS_NET)->EnableWindow(TRUE);//
				GetDlgItem(IDC_IPADDRESS_NET_CH)->EnableWindow(TRUE);//
				GetDlgItem(IDC_IPADDRESS_NET_WG)->EnableWindow(TRUE);//
				GetDlgItem(IDC_IPADDRESS_DNS1)->EnableWindow(TRUE);//
				GetDlgItem(IDC_IPADDRESS_DNS2)->EnableWindow(TRUE);//

				SetDlgItemText(IDC_IPADDRESS_NET, g_test_config.m_current_config[i].Current_IP_address_buffer);
				SetDlgItemText(IDC_IPADDRESS_NET_CH, g_test_config.m_current_config[i].Current_IP_address_ch_buffer);
				SetDlgItemText(IDC_IPADDRESS_NET_WG, g_test_config.m_current_config[i].Current_IP_address_wg_buffer);
				SetDlgItemText(IDC_IPADDRESS_DNS1, g_test_config.m_current_config[i].Current_IP_address_dns1_buffer);
				SetDlgItemText(IDC_IPADDRESS_DNS2, g_test_config.m_current_config[i].Current_IP_address_dns2_buffer);
			}
			else
			{
				((CButton *)GetDlgItem(IDC_CHECK_DHCP))->SetCheck(1);
				GetDlgItem(IDC_IPADDRESS_NET)->EnableWindow(FALSE);//
				GetDlgItem(IDC_IPADDRESS_NET_CH)->EnableWindow(FALSE);//
				GetDlgItem(IDC_IPADDRESS_NET_WG)->EnableWindow(FALSE);//
				GetDlgItem(IDC_IPADDRESS_DNS1)->EnableWindow(FALSE);//
				GetDlgItem(IDC_IPADDRESS_DNS2)->EnableWindow(FALSE);//
				SetDlgItemText(IDC_SHOW_IPADDRESS_NET, _T(""));
				SetDlgItemText(IDC_IPADDRESS_NET_CH, _T(""));
				SetDlgItemText(IDC_IPADDRESS_NET_WG, _T(""));
				SetDlgItemText(IDC_IPADDRESS_DNS1, _T(""));
				SetDlgItemText(IDC_IPADDRESS_DNS2, _T(""));

			}
			break;
		}
	}

}


void CAnyka_TestToolDlg::OnItemchangedListNoAutoTestConfig(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
	show_IP_info();
}




void CAnyka_TestToolDlg::OnItemchangingListNoAutoTestConfig(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	*pResult = 0;
	show_IP_info();
}

void CAnyka_TestToolDlg::OnSetfocusListNoAutoTestConfig(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	show_IP_info();

}

void CAnyka_TestToolDlg::CloseServer(UINT idex) 
{

	if(m_pInetSession[idex] != NULL)
	{
		delete m_pInetSession[idex];
		m_pInetSession[idex] = NULL;
	}
	
	if(m_pFtpConnection[idex] != NULL)
	{
		m_pFtpConnection[idex]->Close();
		delete m_pFtpConnection[idex];
		m_pFtpConnection[idex] = NULL;
	}
	g_sousuo_flag = FALSE;
	g_connet_success_flag = FALSE;
	g_send_commad[idex] = 0;
	M_bConn = FALSE;
	g_setconfig_flag = TRUE;
	g_test_fail_flag[idex]  = 0;
	
	//if(g_hupdateThread != INVALID_HANDLE_VALUE)
	//{
	//	Close_Anyka_update_thread();
	//	g_hupdateThread = INVALID_HANDLE_VALUE;
	//}
	//close_thread();
	close_thread_heat(idex);
	close_thread_rev_data(idex);
	m_ClientSocket.Socket_Close(idex);

}


BOOL  CAnyka_TestToolDlg::check_update_finish_Server(LPCTSTR addr, UINT idex) 
{
	// TODO: Add your control notification handler code here
	CString str;
	HANDLE update_finish_Thread = INVALID_HANDLE_VALUE;
	
	USES_CONVERSION;
	
	// TODO: Add your control notification handler code here
	//m_ClientSocket.Create(0, SOCK_STREAM, NULL);
	if (!m_ClientSocket.Socket_Create_update_finish(idex))
	{
		AfxMessageBox(_T("创建升级检查soket失败"), MB_OK);
		return FALSE;
	}
	
	
	//if(m_ClientSocket.Connect(addr, 6789))
	if(m_ClientSocket.Socket_Connect_update_finish(T2A(addr), 7890, idex))	
	{
		//创建线程
		
		if (!create_thread_update_finish(idex))
		{
			AfxMessageBox(_T("创建升级线程检查失败"), MB_OK);
			return FALSE;
		}

		//关闭心跳线程和接收数据线程
		close_thread_heat(idex);
		close_thread_rev_data(idex);
	}
	else
	{
		AfxMessageBox(_T("Socket_Connect_update_finish fail"), MB_OK);
		return FALSE;
	}
	
	return TRUE;
}


BOOL CAnyka_TestToolDlg::ConnetServer(LPCTSTR addr, UINT idex) 
{
	// TODO: Add your control notification handler code here
	CString str;

	USES_CONVERSION;

	// TODO: Add your control notification handler code here
	//m_ClientSocket.Create(0, SOCK_STREAM, NULL);
	if (!m_ClientSocket.Socket_Create(idex))
	{
		AfxMessageBox(_T("Socket_Create fail"));
		return FALSE;
	}
	
	
	//if(m_ClientSocket.Connect(addr, 6789))
	if(m_ClientSocket.Socket_Connect(T2A(addr), m_net_uPort, idex))	
	{
		//创建线程
		//create_thread();
		//创建线程
		if (!create_thread_heat(addr, idex))
		{
			//AfxMessageBox(_T("创建心跳线程失败"));
			return FALSE;
		}

		if (!create_thread_rev_data(idex))
		{
			AfxMessageBox(_T("创建接收数据线程失败"));
			return FALSE;
		}
		
		//连接ftp服务器
		if (!Connet_FTPServer(addr, idex))
		{
			AfxMessageBox(_T("Connet_FTPServer fail"));
			return FALSE;
		}

		g_connet_success_flag = TRUE;
		g_close_Thread_flag = FALSE;
		g_heat_enter_flag = FALSE;
		g_send_commad[idex] = 1;
	}
	else
	{
		AfxMessageBox(_T("Socket_Connect fail"));
		return FALSE;
	}

	return TRUE;
}

BOOL CAnyka_TestToolDlg::check_info_is_emtpy(void) 
{
	CString str;

	GetDlgItemText(IDC_EDIT_CHANNEL_NAME, str);
	if (str.IsEmpty())
	{
		return FALSE;
	}
	if (!g_test_config.m_dhcp_flag)
	{
		GetDlgItemText(IDC_IPADDRESS_NET, str);
		if (str.IsEmpty())
		{
			return FALSE;
		}

		GetDlgItemText(IDC_IPADDRESS_NET_CH, str);
		if (str.IsEmpty())
		{
			return FALSE;
		}

		GetDlgItemText(IDC_IPADDRESS_NET_WG, str);
		if (str.IsEmpty())
		{
			return FALSE;
		}

		GetDlgItemText(IDC_IPADDRESS_DNS1, str);
		if (str.IsEmpty())
		{
			return FALSE;
		}

		GetDlgItemText(IDC_IPADDRESS_DNS2, str);
		if (str.IsEmpty())
		{
			return FALSE;
		}
	}
	
	

	return TRUE;	
}

void CAnyka_TestToolDlg::OnButtonConfig() 
{
	
}

typedef void GetIPCallBack(LPNETRESOURCE resource);
void TestGetIP(LPNETRESOURCE resource)
{
	char szHostName[200];  
	struct   hostent   *host;   
	struct   in_addr   *ptr;   
	LPNETRESOURCE NetResource = resource;
	
	//return ;
	if(NetResource->lpRemoteName)   
	{   
		CString   strFullName   =   NetResource->lpRemoteName;  
		
		if(0 == strFullName.Left(2).Compare(_T("\\\\")))
		{
			strFullName  =  strFullName.Right(strFullName.GetLength()-2); 
		}
		gethostname(szHostName,strlen(szHostName));
		USES_CONVERSION;
		char *pchar = T2A(strFullName.GetBuffer(30));
		//char *pchar;
		host   =   gethostbyname(pchar);   
		if(host   ==   NULL)   return;     
		ptr   =   (struct in_addr *)   host->h_addr_list[0];  
		int   a   =   ptr->S_un.S_un_b.s_b1;     //   211   
		int   b   =   ptr->S_un.S_un_b.s_b2;     //   40   
		int   c   =   ptr->S_un.S_un_b.s_b3;     //   35   
		int   d   =   ptr->S_un.S_un_b.s_b4;     //   76   
		CString   strTemp;   
		strTemp.Format(_T("%d.%d.%d.%d"),a,b,c,d); 

		//显示到列表中
		_tcscpy(m_ip_address_buf[m_ip_address_idex], strTemp);
		m_ip_address_idex++;

		strFullName.ReleaseBuffer();
	}
}
BOOL  EnumerateFunc(GetIPCallBack pCallBack, LPNETRESOURCE lpnr)
{ 
	DWORD dwResult, dwResultEnum;
	HANDLE hEnum;
	DWORD cbBuffer = 16384;      // 16K is a good size
	DWORD cEntries = -1;         // enumerate all possible entries
	LPNETRESOURCE lpnrLocal;     // pointer to enumerated structures
	DWORD i;
	//
	// Call the WNetOpenEnum function to begin the enumeration.
	//
	dwResult = WNetOpenEnum(RESOURCE_GLOBALNET, // all network resources
						  RESOURCETYPE_ANY,   // all resources
						  0,        // enumerate all resources
						  lpnr,     // NULL first time the function is called
						  &hEnum);  // handle to the resource
	if (dwResult != NO_ERROR)
	{  
	//
		return FALSE;
	}
	//
	// Call the GlobalAlloc function to allocate resources.
	//
	lpnrLocal = (LPNETRESOURCE) GlobalAlloc(GPTR, cbBuffer);
	if (lpnrLocal == NULL) 
	{
		return FALSE;
	}

	do
	{  
		//
		// Initialize the buffer.
		//
		ZeroMemory(lpnrLocal, cbBuffer);
		//
		// Call the WNetEnumResource function to continue
		//  the enumeration.
		//
		dwResultEnum = WNetEnumResource(hEnum,      // resource handle
										&cEntries,  // defined locally as -1
										lpnrLocal,  // LPNETRESOURCE
										&cbBuffer); // buffer size
		//
		// If the call succeeds, loop through the structures.
		//
		if (dwResultEnum == NO_ERROR)
		{
			  for(i = 0; i < cEntries; i++)
			  {
				// Call an application-defined function to
				//  display the contents of the NETRESOURCE structures.
				//
				if((lpnrLocal[i].dwDisplayType != RESOURCEDISPLAYTYPE_SERVER) && ((RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage
											   & RESOURCEUSAGE_CONTAINER))))
				   EnumerateFunc(pCallBack, &lpnrLocal[i]);
				else
				pCallBack(&lpnrLocal[i]);
    
			  }
		}
		// Process errors.
		//
		else if (dwResultEnum != ERROR_NO_MORE_ITEMS)
		{
			break;
		}
	}
	while(dwResultEnum != ERROR_NO_MORE_ITEMS);
	//
	// Call the GlobalFree function to free the memory.
	//
	GlobalFree((HGLOBAL)lpnrLocal);
	//
	// Call WNetCloseEnum to end the enumeration.
	//
	dwResult = WNetCloseEnum(hEnum);

	if(dwResult != NO_ERROR)
	{ 
		//
		// Process errors.
		//
		return FALSE;
	}
	return TRUE;
}


void CAnyka_TestToolDlg::OnButtonSousuo() 
{
	CSearchServer search;
	UINT i = 0;
	
	if (g_connet_success_flag)
	{
		CloseServer(i);
	}
	

	// TODO: Add your control notification handler code here
	if (!M_bConn)
	{
		SetDlgItemText(IDC_STATIC_STAUT, _T("手动未连接"));//可用
		GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(FALSE);//可用
		M_bConn = TRUE;

		g_update_all_finish_flag = FALSE;
		g_update_one_finish_flag = FALSE;
		g_update_all_flag = FALSE;
		g_sousuo_flag = TRUE;
		m_ip_address_idex = 0;
		g_cur_find_num = 0;
		m_find_IP_end_flag = FALSE;
		g_IP_same_flag = FALSE;
		m_no_auto_test_config.DeleteAllItems();

		for (i = 0; i < MAX_PATH; i++)
		{
			memset(m_ip_address_buf[i], 0, IP_ADDRE_LEN);
			m_update_flag[i] = 0;
			memset(g_test_config.m_current_config[i].Current_IP_channel_name, 0, MAX_PATH);
			memset(g_test_config.m_current_config[i].Current_IP_address_buffer, 0, IP_ADDRE_LEN);
			memset(g_test_config.m_current_config[i].Current_IP_address_ch_buffer, 0, IP_ADDRE_LEN);
			memset(g_test_config.m_current_config[i].Current_IP_address_wg_buffer, 0, IP_ADDRE_LEN);
			memset(g_test_config.m_current_config[i].Current_IP_address_dns1_buffer, 0, IP_ADDRE_LEN);
			memset(g_test_config.m_current_config[i].Current_IP_address_dns2_buffer, 0, IP_ADDRE_LEN);
		}
		//SetDlgItemText(IDC_EDIT_CHANNEL_NAME, _T(""));
		SetDlgItemText(IDC_IPADDRESS_NET, _T(""));
		SetDlgItemText(IDC_IPADDRESS_NET_CH, _T(""));
		SetDlgItemText(IDC_IPADDRESS_NET_WG, _T(""));
		SetDlgItemText(IDC_IPADDRESS_DNS1, _T(""));
		SetDlgItemText(IDC_IPADDRESS_DNS2, _T(""));
		m_ip_address_idex = 0;
		SetDlgItemText(IDC_EDIT_CURENT_IP, _T(""));
		
		search.Broadcast_Search();
		m_Manual_flag = FALSE;
		update_or_set_IP_num = 0;
		//Creat_Anyka_connetIP_thread(); 
	}
	else
	{
		AfxMessageBox(_T("正在搜索设备"));
	}
	
}


void CAnyka_TestToolDlg::OnCheckDhcp() 
{
	// TODO: Add your control notification handler code here
	int check;
	check = ((CButton *)GetDlgItem(IDC_CHECK_DHCP))->GetCheck();
	if (!check)
	{
		g_test_config.m_dhcp_flag = 0;
		//GetDlgItem(IDC_EDIT_CHANNEL_NAME)->EnableWindow(TRUE);//
		GetDlgItem(IDC_IPADDRESS_NET)->EnableWindow(TRUE);//
		GetDlgItem(IDC_IPADDRESS_NET_CH)->EnableWindow(TRUE);//
		GetDlgItem(IDC_IPADDRESS_NET_WG)->EnableWindow(TRUE);//
		GetDlgItem(IDC_IPADDRESS_DNS1)->EnableWindow(TRUE);//
		GetDlgItem(IDC_IPADDRESS_DNS2)->EnableWindow(TRUE);//
		
	}
	else
	{
		g_test_config.m_dhcp_flag = 1;
		//GetDlgItem(IDC_EDIT_CHANNEL_NAME)->EnableWindow(FALSE);//
		GetDlgItem(IDC_IPADDRESS_NET)->EnableWindow(FALSE);//
		GetDlgItem(IDC_IPADDRESS_NET_CH)->EnableWindow(FALSE);//
		GetDlgItem(IDC_IPADDRESS_NET_WG)->EnableWindow(FALSE);//
		GetDlgItem(IDC_IPADDRESS_DNS1)->EnableWindow(FALSE);//
		GetDlgItem(IDC_IPADDRESS_DNS2)->EnableWindow(FALSE);//
		SetDlgItemText(IDC_IPADDRESS_NET, _T(""));
		SetDlgItemText(IDC_IPADDRESS_NET_CH, _T(""));
		SetDlgItemText(IDC_IPADDRESS_NET_WG, _T(""));
		SetDlgItemText(IDC_IPADDRESS_DNS1, _T(""));
		SetDlgItemText(IDC_IPADDRESS_DNS2, _T(""));
	}
}


int Eng_Unicode2UTF8(const WORD *unicode, int ucLen, BYTE *utf8, int utf8BufLen)
{
    int        i=0, j=0;
    int       uc = 0;
	
    while (unicode[i]!=0 && i<ucLen)
    {
        uc = unicode[i];
		
        if (uc < (WORD)0x80)
        {
            if(0!=utf8BufLen)
            {
                utf8[j] = (BYTE)uc;
            }
            ++j;
        }
        else if (uc < (WORD)0x800)
        {
            if(0!=utf8BufLen)
            {
                utf8[j] = (uc>>6)|0xc0;
                utf8[j+1] = (uc&0x3f)|0x80;
            }
            j+=2;
        }
        else
        {
            if(0!=utf8BufLen)
            {
                utf8[j] = ((uc>>12)&0x0f)|0xe0;
                utf8[j+1] = ((uc>>6)&0x3f)|0x80;
                utf8[j+2] = (uc&0x3f)|0x80;
            }
            j+=3;
        }
        ++i;
    }
    if(0 != utf8BufLen)
    {
        utf8[j] = 0;
    }
    ++j;
    return j;
	
}

void onvif_format_gb_to_Jovision_code(unsigned char *GB_code, int gb_len,unsigned char *Jovision_code)
{
	int i;
//	unsigned char *tmp_buf = (unsigned char *)malloc(gb_len * 4);
	unsigned char *ptr = Jovision_code;
	for(i = 0; i < gb_len; i++)
	{
		if(*GB_code < 0x80)
		{
			*ptr = *GB_code;
		}
		else if(*GB_code < 0xC0)
		{ 
			*ptr = 0xc2;
			*ptr ++;
			*ptr = *GB_code;
		}
		else
		{
			*ptr = 0xc3;
			*ptr ++;
			*ptr = *GB_code - 0x40;
		}
		ptr ++;
		GB_code ++;
	}
	*ptr = 0;
}


BOOL CAnyka_TestToolDlg::write_config() 
{
	BYTE UTF_8[400];
	BYTE UTF_8_temp[400];
	TCHAR buf_tmp[400];
	CString str;
	HANDLE  hFile;
	UINT buf_size = 0;
	unsigned long write_len = 0;
	BOOL ret = FALSE;
	TCHAR param_temp[MAX_PATH] = {0};
	TCHAR file_name[MAX_PATH] = {0};
	CString strSourceName;
	UINT len = 0, Jovision_code_len = 0, i =0, pos = 0;
	unsigned char Jovision_code[100];

	USES_CONVERSION;

	g_test_config.m_dhcp_flag = ((CButton *)GetDlgItem(IDC_CHECK_DHCP))->GetCheck();
	
	if (!check_info_is_emtpy())
	{
		AfxMessageBox(_T("存在有没填数据的框，请检查"));
		return FALSE;
	}
	GetDlgItemText(IDC_EDIT_CHANNEL_NAME, str);
	if (str.GetLength() > 49)
	{
		AfxMessageBox(_T("通道名长度不能大于49"));
		return FALSE;
	}

	_tcsncpy(g_test_config.IP_channel_name, str, MAX_PATH);//工程名

	//读取
	//if (!g_test_config.m_dhcp_flag)
	{	
		GetDlgItemText(IDC_IPADDRESS_NET, str);
		_tcsncpy(g_test_config.IP_address_buffer, str, MAX_PATH);//工程名
		
		GetDlgItemText(IDC_IPADDRESS_NET_CH, str);
		_tcsncpy(g_test_config.IP_address_ch_buffer, str, MAX_PATH);//工程名
		
		GetDlgItemText(IDC_IPADDRESS_NET_WG, str);
		_tcsncpy(g_test_config.IP_address_wg_buffer, str, MAX_PATH);//工程名
		
		
		GetDlgItemText(IDC_IPADDRESS_DNS1, str);
		_tcsncpy(g_test_config.IP_address_dns1_buffer, str, MAX_PATH);//工程名
		
		GetDlgItemText(IDC_IPADDRESS_DNS2, str);
		_tcsncpy(g_test_config.IP_address_dns2_buffer, str, MAX_PATH);//工程名
	}
	
	GetDlgItemText(IDC_EDIT_MAIN_CODE_RATE, str);
	if (!str.IsEmpty())
	{
		g_test_config.main_code_rate = atoi(T2A(str));//工程名
	}

	GetDlgItemText(IDC_EDIT_SUB_CODE_RATE, str);
	if (!str.IsEmpty())
	{
		g_test_config.sub_code_rate = atoi(T2A(str));//工程名
	}

	GetDlgItemText(IDC_EDIT_MAIN_FPS, str);
	if (!str.IsEmpty())
	{
		g_test_config.main_fps = atoi(T2A(str));//工程名
	}

	GetDlgItemText(IDC_EDIT_SUB_FPS, str);
	if (!str.IsEmpty())
	{
		g_test_config.sub_fps = atoi(T2A(str));//工程名
	}

	GetDlgItemText(IDC_EDIT_QP, str);
	if (!str.IsEmpty())
	{
		g_test_config.qp = atoi(T2A(str));//工程名
	}
	
	GetDlgItemText(IDC_EDIT_GOP, str);
	if (!str.IsEmpty())
	{
		g_test_config.gop = atoi(T2A(str));//工程名
	}

	g_test_config.OSD_flag = ((CButton *)GetDlgItem(IDC_CHECK_OSD))->GetCheck();

	g_test_config.time_flag = ((CButton *)GetDlgItem(IDC_CHECK_TIME))->GetCheck();
		
	//if (!g_test_config.m_dhcp_flag)
	{
		wsprintf(param_temp, _T("@@%s@%s@%s@%s@%s@%d@%d@%d@%d@%d@%d@%d@%d@%d /var/stat"),  g_test_config.IP_address_buffer
			,g_test_config.IP_address_ch_buffer, g_test_config.IP_address_wg_buffer
			,g_test_config.IP_address_dns1_buffer, g_test_config.IP_address_dns2_buffer
				, g_test_config.m_dhcp_flag, g_test_config.main_code_rate, g_test_config.sub_code_rate
				,g_test_config.OSD_flag, g_test_config.time_flag, g_test_config.main_fps, g_test_config.sub_fps
				, g_test_config.qp, g_test_config.gop);
	}
	/*
	else
	{
		wsprintf(param_temp, _T("@@0@0@0@0@0@%d@%d@%d@%d@%d@%d@%d@%d@%d /var/stat")
			, g_test_config.m_dhcp_flag, g_test_config.main_code_rate, g_test_config.sub_code_rate
			,g_test_config.OSD_flag, g_test_config.time_flag, g_test_config.main_fps, g_test_config.sub_fps
				, g_test_config.qp, g_test_config.gop);
	}*/
	
	
	len  = _tcslen(g_test_config.IP_channel_name);
	_tcsncpy(buf_tmp, g_test_config.IP_channel_name, len+ 1);
	_tcscat(buf_tmp, param_temp);
	
	Eng_Unicode2UTF8(buf_tmp, _tcslen(buf_tmp), UTF_8, 400);
	
	buf_size = strlen((char *)UTF_8);//长度
	for (i = 0; i < buf_size; i++)
	{
		if (UTF_8[i] == 0x40)
		{
			pos = i;
			break;
		}
	}
	
	len = strlen((const char *)T2A(g_test_config.IP_channel_name));
	onvif_format_gb_to_Jovision_code((unsigned char *)T2A(g_test_config.IP_channel_name), len,  Jovision_code);
	memcpy(UTF_8_temp, UTF_8, pos+1);
	Jovision_code_len = strlen((char *)Jovision_code);
	memcpy(&UTF_8_temp[pos+1], Jovision_code, Jovision_code_len + 1);
	memcpy(&UTF_8_temp[pos+1+Jovision_code_len], &UTF_8[pos+1], strlen((char *)UTF_8) - pos);

	strSourceName.Format(_T("%s"), g_test_config.ConvertAbsolutePath(_T("test_config/arg.txt")));

	_tcscpy(file_name, strSourceName);

	DeleteFile(file_name);

	hFile = CreateFile(file_name, GENERIC_WRITE, FILE_SHARE_READ , NULL, 
		OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);//创建文件
	
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

	buf_size = strlen((char *)UTF_8_temp);//长度

    ret = WriteFile(hFile, (LPVOID)UTF_8_temp, buf_size, &write_len, NULL);
	if (!ret && buf_size != write_len)
	{
		return FALSE;
	}


	CloseHandle(hFile);

	return TRUE;
}



void CAnyka_TestToolDlg::OnButtonSetconfig() 
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here

	CString strSourceName, strDestName;
	TCHAR DestName[50] =_T("/tmp/");
	TCHAR DestName_temp[MAX_PATH] = {0};
	UINT i = 0, name_len = 0, j = 0;
	char name_buf[MAX_PATH] = {0};
	int ret = 0;
	TCHAR test_Name[MAX_PATH] = {0};
	char SendBufTmp[400];
	UINT modify_idex = 0;
	CString str;
	

	//读取FTP密码和名
	get_password_and_name();

	g_update_all_flag = FALSE;
	g_update_all_finish_flag = FALSE;
	g_update_one_finish_flag = FALSE;
	if (g_setconfig_flag)
	{
		USES_CONVERSION;
		g_setconfig_flag = FALSE;
		g_sousuo_show_flag = FALSE;
		m_find_IP_end_flag = FALSE;
		GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(FALSE);//可用

		if (!m_Manual_flag)
		{
			if (0)//m_no_auto_test_config.GetSelectedCount() < 1)
			{
				g_sousuo_show_flag = TRUE;
				g_setconfig_flag = TRUE;
				AfxMessageBox(_T("请选择一个IP地址"), MB_OK);
				GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
				return;
			}
			
			GetDlgItemText(IDC_EDIT_CURENT_IP, str);
			if (!str.IsEmpty())
			{
				i = atoi(T2A(str)) - 1;
				//for (i = m_no_auto_test_config.GetItemCount()-1; i >=0; i--)
				{
					//if(m_no_auto_test_config.GetItemState(i, LVIS_SELECTED))
					{
						//m_nWorkFunction = WF_NORMAL;
						//进行对此ip连接
						if (ConnetServer(m_ip_address_buf[i], i))
						{
							modify_idex = i;
							g_update_idex = i;
							
							
							//MessageBox(_T("连接成功"));
							if (g_test_fail_flag[i] == 1)
							{
								g_sousuo_show_flag = TRUE;
								CloseServer(i);
								AfxMessageBox(_T("连接失败,请重启小机"), MB_OK);
								GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
								GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
								GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
								GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
								GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
								GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
								GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用

								return;
							}
							//break;
						}
						else
						{
							g_sousuo_show_flag = TRUE;
							CloseServer(i);
							AfxMessageBox(_T("连接失败,请重启小机"), MB_OK);
							GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
							return;
						}
						
					}
				}
			}
			else
			{
				g_sousuo_show_flag = TRUE;
				g_setconfig_flag = TRUE;
				AfxMessageBox(_T("请选择一个IP地址"), MB_OK);
				GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
				return;
			}
		}
		else
		{
			g_update_idex = 0;
			modify_idex = 0;
			if (!g_connet_success_flag)
			{
				CloseServer(0);
				AfxMessageBox(_T("没有进行连接成功,请重启小机"), MB_OK);
				GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
				return;
			}
		}

		update_or_set_IP_num++;
		
		if (!m_Manual_flag)
		{
			m_no_auto_test_config.SetItemText(modify_idex, 2, _T(""));
		}

		if (!write_config())
		{
			if (!m_Manual_flag)
			{
				m_no_auto_test_config.SetItemText(modify_idex, 2, _T("更新失败"));
			}
			else
			{
				SetDlgItemText(IDC_STATIC_STAUT, _T("更新失败"));//可用
			}
			GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
			AfxMessageBox(_T("创建文件arg.txt失败,请重启小机"), MB_OK);
			CloseServer(modify_idex);
			return;
		}

	   //第1个文件
		memset(test_Name, 0,  sizeof(TEST_CONFIG_DIR));
		_tcsncpy(test_Name, TEST_CONFIG_DIR, sizeof(TEST_CONFIG_DIR));
		_tcscat(test_Name, _T("test_assistant"));
		
		//判断文件是否存在
		if(0xFFFFFFFF == GetFileAttributes(g_test_config.ConvertAbsolutePath(test_Name)))
		{
			if (!m_Manual_flag)
			{
				m_no_auto_test_config.SetItemText(modify_idex, 2, _T("更新失败"));
			}
			else
			{
				SetDlgItemText(IDC_STATIC_STAUT, _T("更新失败"));//可用
			}

			str.Format(_T("%s no exist,请重启小机"), test_Name);
			AfxMessageBox(str, MB_OK); 
			GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
			CloseServer(modify_idex);
			return ;
		}
		
		//发送文件
		memset(DestName_temp, 0, MAX_PATH);
		_tcsncpy(DestName_temp, DestName, sizeof(DestName));
		strSourceName.Format(_T("%s"), g_test_config.ConvertAbsolutePath(test_Name));
		_tcscat(DestName_temp, _T("test_assistant"));
		strDestName.Format(_T("%s"), DestName_temp);
		
		if (!m_pFtpConnection[g_update_idex]->PutFile(strSourceName, strDestName, FTP_TRANSFER_TYPE_BINARY, 1))   
		{
			if (!m_Manual_flag)
			{
				m_no_auto_test_config.SetItemText(modify_idex, 2, _T("更新失败"));
			}
			else
			{
				SetDlgItemText(IDC_STATIC_STAUT, _T("更新失败"));//可用
			}

			AfxMessageBox(_T("Error no auto test putting file,请重启小机"), MB_OK);  
			GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
			CloseServer(modify_idex);
			return ;
		}

		//第二个文件
		memset(test_Name, 0,  sizeof(TEST_CONFIG_DIR));
		_tcsncpy(test_Name, TEST_CONFIG_DIR, sizeof(TEST_CONFIG_DIR));
		_tcscat(test_Name, _T("arg.txt"));
		
		//判断文件是否存在
		if(0xFFFFFFFF == GetFileAttributes(g_test_config.ConvertAbsolutePath(test_Name)))
		{
			CString str;

			if (!m_Manual_flag)
			{
				m_no_auto_test_config.SetItemText(modify_idex, 2, _T("更新失败"));
			}
			else
			{
				SetDlgItemText(IDC_STATIC_STAUT, _T("更新失败"));//可用
			}
			
			str.Format(_T("%s no exist,请重启小机"), test_Name);
			AfxMessageBox(str, MB_OK);  
			GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
			CloseServer(modify_idex);
			return ;
		}
		
		//发送文件
		memset(DestName_temp, 0, MAX_PATH);
		_tcsncpy(DestName_temp, DestName, sizeof(DestName));
		strSourceName.Format(_T("%s"), g_test_config.ConvertAbsolutePath(test_Name));
		_tcscat(DestName_temp, _T("arg.txt"));
		strDestName.Format(_T("%s"), DestName_temp);
		
		if (!m_pFtpConnection[g_update_idex]->PutFile(strSourceName, strDestName, FTP_TRANSFER_TYPE_BINARY, 1))   
		{
			if (!m_Manual_flag)
			{
				m_no_auto_test_config.SetItemText(modify_idex, 2, _T("更新失败"));
			}
			else
			{
				SetDlgItemText(IDC_STATIC_STAUT, _T("更新失败"));//可用
			}

			AfxMessageBox(_T("Error no test putting file arg.txt,请重启小机"), MB_OK); 
			GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
			CloseServer(modify_idex);
			return ;
		}
		
		//发送命令
		name_len = strlen("test_assistant");
		memset(name_buf, 0, MAX_PATH);
		memcpy(name_buf, "test_assistant", name_len);
		
		name_len = strlen("arg.txt");
		memset(SendBufTmp, 0, MAX_PATH);
		memcpy(SendBufTmp, "arg.txt", name_len);
		//strcat((char *)SendBufTmp,T2A(param_temp));
		g_test_pass_flag[modify_idex] = 0;
		if (!Send_cmd(TEST_COMMAND, 1, name_buf, SendBufTmp, modify_idex))

		{
			if (!m_Manual_flag)
			{
				m_no_auto_test_config.SetItemText(modify_idex, 2, _T("更新失败"));
			}
			else
			{
				SetDlgItemText(IDC_STATIC_STAUT, _T("更新失败"));//可用
			}

			AfxMessageBox(_T("Error auto test putting file TEST_COMMAND ,请重启小机"), MB_OK); 
			GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
			CloseServer(modify_idex);
			return ;
		}
		
		//接收返回值
		if (!Anyka_Test_check_info( _T("test_assistant"), FALSE, modify_idex))
		{
			if (!m_Manual_flag)
			{
				m_no_auto_test_config.SetItemText(modify_idex, 2, _T("更新失败"));
			}
			else
			{
				SetDlgItemText(IDC_STATIC_STAUT, _T("更新失败"));//可用
			}

			AfxMessageBox(_T("更新失败,请重启小机"), MB_OK);
			GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
			CloseServer(modify_idex);
			return ;
		}
		g_test_pass_flag[modify_idex] = 0;
		if (!Send_cmd(TEST_COMMAND_FINISH, 1, NULL, NULL, modify_idex))
		{
			//AfxMessageBox(_T("更新成功, 小机重启失败"));  
			frmLogfile.WriteLogFile(0," send TEST_COMMAND_FINISH fail \n");
		}
		
		if (!Anyka_Test_check_info(_T("finish"), FALSE, modify_idex))
		{
			AfxMessageBox(_T("更新成功, 小机自动重启失败，请手动重启"), MB_OK);  
			frmLogfile.WriteLogFile(0," no auto Anyka_Test_check_info fail \n");
		}
		else
		{
			MessageBox(_T("更新成功"), MB_OK);
		}

		if (!m_Manual_flag)
		{
			m_no_auto_test_config.SetItemText(modify_idex, 2, _T("已更新"));
		}
		else
		{
			SetDlgItemText(IDC_STATIC_STAUT, _T("已更新"));//可用
		}

		GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
		CloseServer(modify_idex);

	}
}

BOOL CAnyka_TestToolDlg::Download_updatefile(TCHAR *ip_address, UINT list_idex, UINT idex) 
{
	CString str;
	CString strSourceName, strDestName;
	TCHAR DestName[50] =_T("/tmp/");
	TCHAR DestName_temp[MAX_PATH] = {0};
	UINT i = 0, name_len = 0;
	char name_buf[MAX_PATH] = {0};
	char param[50] = {0};
	TCHAR test_Name[MAX_PATH] = {0};
	TCHAR file_name[MAX_PATH] = {0};
	TCHAR test_update_name[50] = _T("test_update");
	HANDLE update_finish_Thread = INVALID_HANDLE_VALUE;

	USES_CONVERSION;

	if (!find_file_indir(file_name,&name_len))
	{
		m_update_flag[list_idex] = 2;
		AfxMessageBox(_T("文件夹内应只存放一个升级文件，请检查"), MB_OK);
		return FALSE;
	}
	
	//判断小机是否正常动行
	if (g_test_fail_flag[idex] == 1)
	{
		m_update_flag[list_idex] = 2;
		str.Format(_T("g_test_fail_flag[%d]，请重启小机"), idex);
		AfxMessageBox(str);
		return FALSE;
	}
	
	//判断文件是否存在
	memset(test_Name, 0,  sizeof(TEST_CONFIG));
	_tcsncpy(test_Name, TEST_CONFIG, sizeof(TEST_CONFIG));
	_tcscat(test_Name, _T("//"));
	_tcscat(test_Name, test_update_name);
	if(0xFFFFFFFF == GetFileAttributes(g_test_config.ConvertAbsolutePath(test_Name)))
	{
		m_update_flag[list_idex] = 2;
		str.Format(_T("%s no exist，请重启小机"), test_Name);
		AfxMessageBox(str);   
		return FALSE;
	}

	//发送文件
	//下载升级程序
	m_update_flag[list_idex] = 4;
	
	memset(DestName_temp, 0, MAX_PATH);
	_tcsncpy(DestName_temp, DestName, sizeof(DestName));
	_tcscat(DestName_temp, test_update_name);
	strDestName.Format(_T("%s"), DestName_temp);
	strSourceName.Format(_T("%s"), g_test_config.ConvertAbsolutePath(test_Name));
	if (!m_pFtpConnection[idex]->PutFile(strSourceName, strDestName, FTP_TRANSFER_TYPE_BINARY, 1))   
	{
		m_update_flag[list_idex] = 2;
		frmLogfile.WriteLogFile(0,"0 putting %s fail \n", test_Name);
		AfxMessageBox(_T("Error 0 putting file，请重启小机"));
		return FALSE;
	}

	memset(test_Name, 0,  sizeof(UPDATE_CONFIG_DIR));
	_tcsncpy(test_Name, UPDATE_CONFIG_DIR, sizeof(UPDATE_CONFIG_DIR));
	_tcscat(test_Name, _T("//"));
	_tcscat(test_Name, file_name);
	if(0xFFFFFFFF == GetFileAttributes(g_test_config.ConvertAbsolutePath(test_Name)))
	{
		m_update_flag[list_idex] = 2;
		str.Format(_T("%s no exist，请重启小机"), file_name);
		AfxMessageBox(str);   
		return FALSE;
	}
	
	//发送文件
	//下载升级包
	memset(DestName_temp, 0, MAX_PATH);
	_tcsncpy(DestName_temp, DestName, sizeof(DestName));
	_tcscat(DestName_temp, file_name);
	strDestName.Format(_T("%s"), DestName_temp);
	strSourceName.Format(_T("%s"), g_test_config.ConvertAbsolutePath(test_Name));
	if (!m_pFtpConnection[idex]->PutFile(strSourceName, strDestName, FTP_TRANSFER_TYPE_BINARY, 1))   
	{
		m_update_flag[list_idex] = 2;
		frmLogfile.WriteLogFile(0,"1 putting %s fail \n", name_buf);
		AfxMessageBox(_T("Error 1 putting file，请重启小机"));
		return FALSE;
	}

	//发送命令
	memset(name_buf, 0, MAX_PATH);
	memcpy(name_buf, T2A(test_update_name), strlen(T2A(test_update_name)));
	g_test_pass_flag[idex] = 0;
	if (!Send_cmd(TEST_COMMAND, 1, name_buf, T2A(file_name), idex))

	{
		m_update_flag[list_idex] = 2;
		frmLogfile.WriteLogFile(0,"auto Send_cmd %s fail \n", name_buf);
		AfxMessageBox(_T("Error Send_cmd file，请重启小机"));   
		return FALSE;
	}

	//升级中
	
	//接收返回值 
	
	if (!Anyka_Test_check_info(file_name, FALSE, idex))
	{
		m_update_flag[list_idex] = 2;
		frmLogfile.WriteLogFile(0,"Anyka_Test_check_info fail \n");
		AfxMessageBox(_T("发升级命后没有接到返回出错，请重启小机")); 
		return FALSE;
	}



	//升级中
	m_update_flag[list_idex] = 3;
	g_test_pass_flag[idex] = 0;
	g_update_pass_flag[idex] = 0;
	//创建一个检查升级完成的线程
	if (!check_update_finish_Server(ip_address, idex))
	{
		m_update_flag[list_idex] = 2;
		AfxMessageBox(_T("创建检测升级完成的线程失败，请重启小机"), MB_OK);
		return FALSE;
	}
	

	//升级成功
	if (!Anyka_check_update_finish(idex))
	{
		m_update_flag[list_idex] = 2;
		frmLogfile.WriteLogFile(0,"Anyka_Test_check_info fail \n");
		AfxMessageBox(_T("超时出错，请重启小机"), MB_OK);
		return FALSE;
	}

	//关闭检查线程
	close_thread_update_finish(idex);

	m_update_flag[list_idex] = 1;
	return TRUE;

}

BOOL CAnyka_TestToolDlg::update_one_main(TCHAR *ip_address, UINT list_idex, UINT idex) 
{
	//进行对此ip连接
	if (!Download_updatefile(ip_address, list_idex, idex))
	{
		m_update_flag[list_idex] = 2;	
		CloseServer(idex);
		return FALSE;
	}
	m_update_flag[list_idex] = 1;
	CloseServer(idex);
	MessageBox(_T("升级完成，请重启小机"), MB_OK);
	return TRUE;
	
}

DWORD WINAPI Creat_Anyka_update_one_main(LPVOID lpParameter)
{
	CAnyka_TestToolDlg test_dlg;
	T_IDEX_INFO param;
	
	memcpy(&param, lpParameter, sizeof(T_IDEX_INFO));

	g_update_one_flag = TRUE;
	g_update_one_finish_flag = FALSE;
	if (!test_dlg.update_one_main(m_ip_address_buf[param.IP_idex], param.IP_idex, param.thread_idex))
	{
		g_update_one_finish_flag = TRUE;
		return 0;
	}
	g_update_one_finish_flag = TRUE;
	return 1;
}



DWORD WINAPI Creat_update_thread(LPVOID lpParameter)
{
	CAnyka_TestToolDlg test_dlg;
	T_IDEX_INFO param;
	
	memcpy(&param, lpParameter, sizeof(T_IDEX_INFO));

	//srand(time(0));
	//创建sokect和连接
	m_update_flag[param.IP_idex] = 4;
	if (!test_dlg.ConnetServer(m_ip_address_buf[param.IP_idex], param.thread_idex))
	{
		m_update_flag[param.IP_idex] = 2;
		
		WaitForSingleObject(g_handle,INFINITE);
		g_updateing_num++;
		if (g_updateing_num == m_ip_address_idex)
		{
			//g_update_all_flag = FALSE;
			g_update_all_finish_flag = TRUE;
		}

		ReleaseSemaphore(g_handle,1,NULL);
		test_dlg.CloseServer(param.thread_idex);
		CloseHandle(g_all_updateThread[param.thread_idex]);
		g_all_updateThread[param.thread_idex] = INVALID_HANDLE_VALUE;
		AfxMessageBox(_T("连接失败"));
		return FALSE;
	}

	if (!test_dlg.Download_updatefile(m_ip_address_buf[param.IP_idex], param.IP_idex, param.thread_idex))
	{
		
		m_update_flag[param.IP_idex] = 2;
		WaitForSingleObject(g_handle,INFINITE);
		g_updateing_num++;
		if (g_updateing_num == m_ip_address_idex)
		{
			//g_update_all_flag = FALSE;
			g_update_all_finish_flag = TRUE;
		}
		ReleaseSemaphore(g_handle,1,NULL);
		test_dlg.CloseServer(param.thread_idex);
		CloseHandle(g_all_updateThread[param.thread_idex]);
		g_all_updateThread[param.thread_idex] = INVALID_HANDLE_VALUE;
		return FALSE;
	}
	m_update_flag[param.IP_idex] = 1;
	test_dlg.CloseServer(param.thread_idex);
	CloseHandle(g_all_updateThread[param.thread_idex]);
	g_all_updateThread[param.thread_idex] = INVALID_HANDLE_VALUE;

	WaitForSingleObject(g_handle,INFINITE);
	g_updateing_num++;
	if (g_updateing_num == m_ip_address_idex)
	{
		//g_update_all_flag = FALSE;
		g_update_all_finish_flag = TRUE;
		if (g_hupdateThread != INVALID_HANDLE_VALUE)
		{
			test_dlg.Close_Anyka_update_thread();
			g_hupdateThread = INVALID_HANDLE_VALUE;
		}
		AfxMessageBox(_T("所有设备升级已完成，请重启小机"));
	}
	ReleaseSemaphore(g_handle,1,NULL);
	
	
	return 1;
}


BOOL CAnyka_TestToolDlg::update_main() 
{
	UINT idex = 0, i = 0;
	UINT update_one_time = 0;
	UINT update_next_num = 0;
	UINT IP_idex = 0;

	for (i = 0; i < UPDATE_MAX_NUM; i++)
	{
		if (g_heatThread[i] != INVALID_HANDLE_VALUE)
		{
			CloseHandle(g_heatThread[i]);
			g_heatThread[i] = INVALID_HANDLE_VALUE;
		}

		if (g_hBurnThread_rev_data[i] != INVALID_HANDLE_VALUE)
		{
			CloseHandle(g_hBurnThread_rev_data[i]);
			g_hBurnThread_rev_data[i] = INVALID_HANDLE_VALUE;
		}

		if (g_all_updateThread[i] != INVALID_HANDLE_VALUE)
		{
			CloseHandle(g_all_updateThread[i]);
			g_all_updateThread[i] = INVALID_HANDLE_VALUE;
		}

		if (g_update_finish_Thread[i] != INVALID_HANDLE_VALUE)
		{
			CloseHandle(g_update_finish_Thread[i]);
			g_update_finish_Thread[i] = INVALID_HANDLE_VALUE;
		}
		g_test_pass_flag[i] = 0;
		g_test_fail_flag[i] = 0;
		m_update_flag[i] = 0;
		g_send_commad[i] = 0;
	}

	memset(g_param, 0, sizeof(T_IDEX_INFO));

	
	if (!m_Manual_flag)
	{
		//进行一次多冶一起升级
		update_next_num = m_ip_address_idex;
		frmLogfile.WriteLogFile(0," m_ip_address_idex:%d \n", m_ip_address_idex);
		while (1)
		{
			Sleep(1000);
			if (update_next_num > ONE_TIME_MAX_NUM)
			{
				update_one_time = ONE_TIME_MAX_NUM;
			}
			else
			{
				update_one_time = update_next_num;
			}
			frmLogfile.WriteLogFile(0," update_one_time:%d \n", update_one_time);
			for (idex = 0; idex < update_one_time; idex++)
			{
				if (g_all_updateThread[idex] == INVALID_HANDLE_VALUE && IP_idex < m_ip_address_idex)
				{
					//创建一个线程
					g_param[idex].IP_idex = IP_idex;
					g_param[idex].thread_idex = idex;
					Sleep(100);
					g_all_updateThread[idex] = CreateThread(NULL, 0, Creat_update_thread, &g_param[idex], 0, NULL);
					if (g_all_updateThread[idex] == INVALID_HANDLE_VALUE)
					{
						return FALSE;
					}
					frmLogfile.WriteLogFile(0," IP_idex:%d \n", IP_idex);
					IP_idex++;
				}
			}
			
			if (IP_idex >= m_ip_address_idex)
			{
				frmLogfile.WriteLogFile(0," end IP_idex:%d \n", IP_idex);
				break;
			}
		}

		//AfxMessageBox(_T("升级已完成，请重启小机"));
	}
	else
	{
		m_update_flag[0] = 4;
		g_update_idex = 0;
		if (!Download_updatefile(m_ip_address_buf[0], 0, 0))
		{
			m_update_flag[i] = 2;
			//m_Manual_flag = FALSE;
			g_update_all_finish_flag = TRUE;
			CloseServer(idex);
			return FALSE;
		}
		m_update_flag[i] = 1;
		//m_Manual_flag = FALSE;
		g_update_all_finish_flag = TRUE;
		CloseServer(idex);
		AfxMessageBox(_T("升级已完成，请重启小机"));
	}
	
	return TRUE;
	
}

DWORD WINAPI Creat_Anyka_update_main(LPVOID lpParameter)
{
	CAnyka_TestToolDlg test_dlg;

	g_update_all_flag = TRUE;
	g_update_all_finish_flag = FALSE;
	g_update_one_finish_flag = FALSE;
	g_updateing_num = 0; 
	
	if (!test_dlg.update_main())
	{
		//g_update_all_finish_flag = TRUE;
		return 0;
	}
	//g_update_all_finish_flag = TRUE;

	return 1;
}


void CAnyka_TestToolDlg::Close_Anyka_update_thread() 
{
	if(g_hupdateThread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hupdateThread);
		g_hupdateThread = INVALID_HANDLE_VALUE;
	}
	
}

void CAnyka_TestToolDlg::OnButtonUpdateAll() 
{
	UINT param;
	UINT i = 0, j = 0;
	CString strTemp;


	//读取FTP密码和名
	get_password_and_name();

	if (!m_Manual_flag)
	{
		if (m_ip_address_idex == 0)
		{
			AfxMessageBox(_T("没有一台设备，请检查"), MB_OK);
			return;
		}

		if (update_or_set_IP_num != 0)
		{
			AfxMessageBox(_T("一些设备已升级或更新完成的操作，请重新搜索设备或先删除已完成的后，再升级所有的"), MB_OK);
			return;
		}

		m_no_auto_test_config.DeleteAllItems();
		for(i = 0; i < m_ip_address_idex; i++)
		{
			strTemp.Format(_T("%d"), i+1);
			m_no_auto_test_config.InsertItem(i, strTemp);
			m_no_auto_test_config.SetItemText(i, 1, m_ip_address_buf[i]);
			m_no_auto_test_config.SetItemText(i, 2, _T(""));
		}
		
	}
	else
	{
		if (!g_connet_success_flag)
		{
			AfxMessageBox(_T("没有进行连接成功,请重新重启连接"), MB_OK);
			return;
		}
	}

	GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(FALSE);//可用

	// TODO: Add your control notification handler code here
	if (g_hupdateThread != INVALID_HANDLE_VALUE)
	{
		Close_Anyka_update_thread();
		g_hupdateThread = INVALID_HANDLE_VALUE;
	}

	g_hupdateThread = CreateThread(NULL, 0, Creat_Anyka_update_main, &param, 0, NULL);
	if (g_hupdateThread == INVALID_HANDLE_VALUE)
	{
		return;
	}
}




void CAnyka_TestToolDlg::OnChangeEditChannelName() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	CString str;

	GetDlgItemText(IDC_EDIT_CHANNEL_NAME, str);
	if (!str.IsEmpty())
	{
		UINT nLen = str.GetLength();

		if (str.GetAt(nLen-1) == '@')
		{
			AfxMessageBox(_T("不能输入非法字符号"), MB_OK);
			GetDlgItemText(IDC_EDIT_CHANNEL_NAME, str);	
			str = str.Left(nLen-1);
			GetDlgItem(IDC_EDIT_CHANNEL_NAME)->SetWindowText(str);
			GetDlgItem(IDC_EDIT_CHANNEL_NAME)->SetWindowText(str);
			((CEdit*)GetDlgItem(IDC_EDIT_CHANNEL_NAME))->SetSel( nLen-1, nLen-1, FALSE );
			return;
		}
		if (nLen > 49)
		{
			AfxMessageBox(_T("通道名长度不能大于49"));
			GetDlgItem(IDC_EDIT_CHANNEL_NAME)->SetWindowText(_T(""));
			return;
		}
		
		_tcsncpy(g_test_config.IP_channel_name, str, MAX_PATH);//工程名

	}
	


}

void CAnyka_TestToolDlg::OnButtonManual() 
{
	TCHAR ip_address_buf[IP_ADDRE_LEN];
	CString str;
	// TODO: Add your control notification handler code here

	m_no_auto_test_config.DeleteAllItems();
	GetDlgItemText(IDC_IPADDRESS_MANUAL, str);
	SetDlgItemText(IDC_EDIT_CURENT_IP, _T(""));
	_tcsncpy(ip_address_buf, str, IP_ADDRE_LEN);//工程名
	if (_tcscmp(ip_address_buf, _T("0.0.0.0")) == 0)
	{
		AfxMessageBox(_T("IP地址为空，请检查"));
		return;
	}

	SetDlgItemText(IDC_STATIC_STAUT, _T("手动未连接"));//可用
	g_test_fail_flag[0] = 0;
	g_send_commad[0] = 0;
	if (g_heatThread[0] != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_heatThread[0]);
		g_heatThread[0] = INVALID_HANDLE_VALUE;
	}
	
	if (g_hBurnThread_rev_data[0] != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_rev_data[0]);
		g_hBurnThread_rev_data[0] = INVALID_HANDLE_VALUE;
	}

	get_password_and_name();

	if (ConnetServer(ip_address_buf, 0))
	{
		GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用

		Sleep(1000);
		if (g_test_fail_flag[0] == 0)
		{
			SetDlgItemText(IDC_STATIC_STAUT, _T("连接设备成功"));//可用
			MessageBox(_T("手动连接设备成功"));
			m_Manual_flag = TRUE;
			_tcscpy(m_ip_address_buf[0], ip_address_buf);
			g_connet_success_flag = TRUE;
		}
		else
		{
			SetDlgItemText(IDC_STATIC_STAUT, _T("连接设备失败"));//可用
			MessageBox(_T("手动连接设备失败"));
			m_Manual_flag = FALSE;
		}
		
	}
	else
	{
		SetDlgItemText(IDC_STATIC_STAUT, _T("连接设备失败"));//可用
		GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
		MessageBox(_T("手动连接设备失败"));
		m_Manual_flag = FALSE;
	}
}

void CAnyka_TestToolDlg::OnButtonUpdateOne() 
{
	// TODO: Add your control notification handler code here
	UINT ret = 0, i = 0;
	UINT j = 0;
	char commad_type = 0;
	CString str;
	UINT modify_idex = 0, update_idex = 0;


	//读取FTP密码和名
	get_password_and_name();
	
	g_update_all_flag = FALSE;
	g_update_all_finish_flag = FALSE;
	g_update_one_finish_flag = FALSE;

	if (g_setconfig_flag)
	{
		USES_CONVERSION;
		g_setconfig_flag = FALSE;
		g_sousuo_show_flag = FALSE;
		m_find_IP_end_flag = FALSE;
		GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(FALSE);//可用
		GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(FALSE);//可用

		if (!m_Manual_flag)
		{
			if (0)//m_no_auto_test_config.GetSelectedCount() < 1)
			{
				g_sousuo_show_flag = TRUE;
				g_setconfig_flag = TRUE;
				AfxMessageBox(_T("请选择一个IP地址"));
				GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
				return;
			}
			GetDlgItemText(IDC_EDIT_CURENT_IP, str);
			if (!str.IsEmpty())
			{
				i = atoi(T2A(str)) - 1;
				//for (i = m_no_auto_test_config.GetItemCount()-1; i >=0; i--)
				{
					//if(m_no_auto_test_config.GetItemState(i, LVIS_SELECTED))
					{
						//m_nWorkFunction = WF_NORMAL;
						//进行对此ip连接
						if (ConnetServer(m_ip_address_buf[i], i))
						{
							modify_idex = i;
							g_update_idex = i;
							//MessageBox(_T("连接成功"));
							//break;
						}
						else
						{
							g_sousuo_show_flag = TRUE;
							CloseServer(i);
							AfxMessageBox(_T("连接失败"));
							GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
							GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
							
							return;
						}
						
					}
				}
			}
			else
			{
				g_sousuo_show_flag = TRUE;
				g_setconfig_flag = TRUE;
				AfxMessageBox(_T("请选择一个IP地址"));
				GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
				return;
			}
		}
		else
		{
			g_update_idex = 0;
			modify_idex = 0;
			if (!g_connet_success_flag)
			{
				CloseServer(0);
				AfxMessageBox(_T("没有进行连接成功,请重启小机"), MB_OK);
				GetDlgItem(IDC_LIST_NO_AUTO_TEST_CONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_DETTECT_LIST)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_MANUAL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SETCONFIG)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_SOUSUO)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(TRUE);//可用
				GetDlgItem(IDC_BUTTON_UPDATE_ONE)->EnableWindow(TRUE);//可用
				return;
			}
		}
		
		update_or_set_IP_num++;

		if (!m_Manual_flag)
		{
			m_no_auto_test_config.SetItemText(modify_idex, 2, _T(""));
		}

		// TODO: Add your control notification handler code here
		if (g_hupdateThread != INVALID_HANDLE_VALUE)
		{
			Close_Anyka_update_thread();
			g_hupdateThread = INVALID_HANDLE_VALUE;
		}
		m_update_flag[modify_idex] = 3;

		g_param[modify_idex].IP_idex = modify_idex;
		g_param[modify_idex].thread_idex = modify_idex;
		g_hupdateThread = CreateThread(NULL, 0, Creat_Anyka_update_one_main, &g_param[modify_idex], 0, NULL);
		if (g_hupdateThread == INVALID_HANDLE_VALUE)
		{
			return;
		}
	}
	
}

void CAnyka_TestToolDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	//CDialog::OnOK();
}

void CAnyka_TestToolDlg::OnButtonDettectList() 
{
	UINT i = 0, j = 0;
	CString strTemp, str;

	USES_CONVERSION;

	if (m_no_auto_test_config.GetSelectedCount() < 1)
	{
		AfxMessageBox(_T("没有选中任何设备，请选择"), MB_OK);
		return;
	}

	// TODO: Add your control notification handler code here
	GetDlgItemText(IDC_EDIT_CURENT_IP, str);
	if (!str.IsEmpty())
	{
		i = atoi(T2A(str)) - 1;
		//for (i = m_no_auto_test_config.GetItemCount()-1; i >=0; i--)
		{
			//if(m_no_auto_test_config.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
			{
				m_ip_address_idex--;

				for (j = i; j < m_ip_address_idex; j++)
				{
					_tcsncpy(m_ip_address_buf[j], m_ip_address_buf[j+1], IP_ADDRE_LEN);
					_tcsncpy(g_test_config.m_current_config[j].Current_IP_channel_name, g_test_config.m_current_config[j+1].Current_IP_channel_name, MAX_PATH);
					_tcsncpy(g_test_config.m_current_config[j].Current_IP_address_buffer,  g_test_config.m_current_config[j+1].Current_IP_address_buffer, MAX_PATH);
					_tcsncpy(g_test_config.m_current_config[j].Current_IP_address_ch_buffer,  g_test_config.m_current_config[j+1].Current_IP_address_ch_buffer, MAX_PATH);
					_tcsncpy(g_test_config.m_current_config[j].Current_IP_address_wg_buffer,  g_test_config.m_current_config[j+1].Current_IP_address_wg_buffer, MAX_PATH);
					_tcsncpy(g_test_config.m_current_config[j].Current_IP_address_dns1_buffer,  g_test_config.m_current_config[j+1].Current_IP_address_dns1_buffer, MAX_PATH);
					_tcsncpy(g_test_config.m_current_config[j].Current_IP_address_dns2_buffer,  g_test_config.m_current_config[j+1].Current_IP_address_dns2_buffer, MAX_PATH);
					g_test_config.m_current_config[j].Current_IP_address_dhcp_flag = g_test_config.m_current_config[j+1].Current_IP_address_dhcp_flag;		
				}

				m_no_auto_test_config.DeleteItem(i);
				
				//break;
			}
		}
	}

	m_no_auto_test_config.DeleteAllItems();
	for(j = 0; j < m_ip_address_idex; j++)
	{
		strTemp.Format(_T("%d"), j+1);
		m_no_auto_test_config.InsertItem(j, strTemp);
		m_no_auto_test_config.SetItemText(j, 1, m_ip_address_buf[j]);
		m_no_auto_test_config.SetItemText(j, 2, _T(""));
	}

	if (update_or_set_IP_num > 0)
	{
		update_or_set_IP_num--;
	}
	else
	{
		update_or_set_IP_num = 0;
	}
}


//保存密码
BOOL CAnyka_TestToolDlg::StorePassword()
{
	HANDLE hFile;
	char buf[1024];
	DWORD dwWrite;
	int i;
	
	//prepare data
	memset(buf, 0, 1024);
	memcpy(buf, &passwd_ctrl, sizeof(passwd_ctrl));
	for(i = 0; i < 1024; i++)
	{
		buf[i] = buf[i] ^ (i*i + 5*i +1);
	}
	
	//write file
	hFile = CreateFile(g_test_config.ConvertAbsolutePath(_T("password.cfg")) , GENERIC_WRITE | GENERIC_READ , 
		FILE_SHARE_READ|FILE_SHARE_WRITE,NULL , CREATE_ALWAYS , FILE_ATTRIBUTE_HIDDEN , NULL);
	
	if (hFile == INVALID_HANDLE_VALUE) 
	{
		return FALSE;
	}	
	//写密码到文档上
	if(!WriteFile(hFile,buf, 1024, &dwWrite, NULL))
	{
		CloseHandle(hFile);
		return FALSE;
	}
	
	CloseHandle(hFile);
	return TRUE;
}
//获取密码
BOOL CAnyka_TestToolDlg::GetPassword()
{
	HANDLE hFile;
	char buf[1024];
	DWORD dwRead;
	int i;
    
	//fetch data
	hFile = CreateFile(g_test_config.ConvertAbsolutePath(_T("password.cfg")), GENERIC_READ , FILE_SHARE_READ , NULL , 
		OPEN_EXISTING , FILE_ATTRIBUTE_HIDDEN , NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//读取密码
	if(!ReadFile(hFile, buf, 1024, &dwRead, NULL))
	{
		CloseHandle(hFile);
		return FALSE;
	}
	
	//extract password
	for(i = 0; i < 1024; i++)
	{
		buf[i] = buf[i] ^ (i*i + 5*i +1);
	}
	
	memcpy(&passwd_ctrl, buf, sizeof(passwd_ctrl));
	
	CloseHandle(hFile);
	return TRUE;
}

void CAnyka_TestToolDlg::OnChangeEditUsename() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void CAnyka_TestToolDlg::OnChangeEditPassword() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}
