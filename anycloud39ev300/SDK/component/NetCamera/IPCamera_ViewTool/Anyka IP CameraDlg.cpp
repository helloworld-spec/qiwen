// Anyka IP CameraDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Anyka IP Camera.h"
#include "Anyka IP CameraDlg.h"
#include <atlconv.h>
#include <time.h>
#include "IPCameraCommand.h"
#include "NetCtrl.h"
#include "ServerSearch.h"
#include "ServerInfo.h"
#include <process.h>
#include "afxinet.h"

 

#define VIDEO_MODE_NAME_720P	L"720P"
#define VIDEO_MODE_NAME_D1		L"D1"
#define VIDEO_MODE_NAME_VGA		L"VGA"
#define VIDEO_MODE_NAME_QVGA	L"QVGA"

#define RTSP_PREFIX				"rtsp://"
#define SEPARATOR				"/"
#define CHAR_SEPARATOR			'/'
#define PORT_PREFIX				":"
#define TREE_ROOT_ITEM_NAME		L"设备列表:"
#define MAX_RTSP_URL_LEN		(MAX_PATH + 24)

#define TIMER_COMMAND			1
#define TIMER_LONG_PRESS		2

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define  TOOL_VERSOIN _T("NetCamera_v2.0.05")


//just for client debug.
#define UNATTACHED_TALK
#define SBAR_PROGRAMINFO_SCALE		0.4
#define SBAR_DISPLAYINFO_SCALE		0.3

#define ID_TOOLBAR_BUTTON_PIC		WM_USER + 100
#define ID_TOOLBAR_BUTTON_REC		WM_USER + 101
#define ID_TOOLBAR_BUTTON_TAK		WM_USER + 102
#define ID_TOOLBAR_BUTTON_PLY		WM_USER + 103
#define ID_TOOLBAR_BUTTON_ZIN		WM_USER + 104
#define ID_TOOLBAR_BUTTON_ZOUT		WM_USER + 105
#define ID_TOOLBAR_BUTTON_VPLUS		WM_USER + 106
#define ID_TOOLBAR_BUTTON_VMINUS	WM_USER + 107
#define ID_TOOLBAR_BUTTON_STOP_REC	WM_USER + 108

#define ID_STATUSBAR_PROGRAMINFO	WM_USER + 200
#define ID_STATUSBAR_DISPLAYINFO1	WM_USER + 201
#define ID_STATUSBAR_DISPLAYINFO2	WM_USER + 202

#define WM_TALK_KICKOUT				WM_USER + 300
#define WM_SERVER_DISCONNECT		WM_USER + 301
#define WM_SERVER_RET_INFO			WM_USER + 302
#define ATTEMPT_OPEN_MAX			3

BOOL g_start_open_flag = TRUE;
HANDLE g_hTestThread = INVALID_HANDLE_VALUE;
HWND g_hWnd = 0;
BOOL g_Full_flag = TRUE;
TCHAR ip_address[AP_ADDRESS_LEN] = {0};
CInternetSession *m_pInetSession = NULL;
CFtpConnection *m_pFtpConnection = NULL; 
UINT m_uPort = 0;
UINT m_net_uPort = 0;
HANDLE g_hBurnThread_rev_data= INVALID_HANDLE_VALUE;
UINT rve_param[2] = {0};
char g_send_commad = 0;
char g_test_fail_flag = 0;
char g_test_pass_flag = 0;  //0正在测试中， //1测试成功， 2测试失败
char g_commad_type;
BOOL g_connet_flag = FALSE;
BOOL download_file_flag = FALSE;
BOOL download_dev_file_flag = FALSE;
BOOL g_senddata_flag = TRUE;

#define CONFIG_PATH 	L"config.txt"
#define  TEST_CONFIG_DIR           _T("test_config")

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAnykaIPCameraDlg 对话框

unsigned int CAnykaIPCameraDlg::thread_begin( void * pParam )
{
	CAnykaIPCameraDlg * pDlg = static_cast<CAnykaIPCameraDlg *>(pParam);
	pDlg->Monitor();
	return 0;
}

CAnykaIPCameraDlg::CAnykaIPCameraDlg(CWnd* pParent /*=NULL*/)
: CDialog(CAnykaIPCameraDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	ZeroMemory(m_pClient, SUPPORT_STREAM_CNT * sizeof(CAimer39RTSPClient*));
	ZeroMemory(m_videoDecoder, SUPPORT_STREAM_CNT * sizeof(CFfmpegEnvoy*));
	ZeroMemory(m_AudioDecoder, SUPPORT_STREAM_CNT * sizeof(CFfmpegEnvoy*));
	ZeroMemory(m_videoRender, SUPPORT_STREAM_CNT * sizeof(CVideoRender*));
	ZeroMemory(m_AudioRender, SUPPORT_STREAM_CNT * sizeof(CAudioRender*));
	ZeroMemory(m_pServerPreviews, PREVIEW_WINDOWS * sizeof(IServer*));
	ZeroMemory(&m_stKickOutParam, sizeof(KickOutMessageWParam));
	ZeroMemory(&m_stRetInfo, sizeof(RETINFO));

	for (int i = 0; i < PREVIEW_WINDOWS; ++i) m_strURL[i].clear();

	m_nRBChoosePrevIndex = -1;
	m_nAudioClientIndex = -1;
	m_nVideoFullScreenIndex = -1;
	m_nLongPressButtonID = -1;
	m_bIsSearch = FALSE;
	m_hCurrentSelectItem = NULL;
	m_runThreadFlag = FALSE;
	m_bNeedJudgeDisConnWork = TRUE;
	m_bIsInit = FALSE;
	m_bPicture = TRUE;

	m_bIsLongPress = FALSE;
	m_bIsLongPressDone = FALSE;
}

void CAnykaIPCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_TreeCtrl);
	//DDX_Control(pDX, IDC_COMBO1, m_ContrastCombo);
	//DDX_Control(pDX, IDC_COMBO2, m_SaturationCombo);
	//DDX_Control(pDX, IDC_COMBO3, m_BrightnessCombo);
	//DDX_Control(pDX, IDC_COMBO4, m_acutanceCom);
}

BEGIN_MESSAGE_MAP(CAnykaIPCameraDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_32771, &CAnykaIPCameraDlg::OnPrivacyArea)
	ON_COMMAND(ID_TOOLBAR_BUTTON_PIC, &CAnykaIPCameraDlg::OnPicture)
	ON_COMMAND(ID_TOOLBAR_BUTTON_REC, &CAnykaIPCameraDlg::OnRecord)
	ON_COMMAND(ID_TOOLBAR_BUTTON_ZIN, &CAnykaIPCameraDlg::OnZoomIn)
	ON_COMMAND(ID_TOOLBAR_BUTTON_ZOUT, &CAnykaIPCameraDlg::OnZoomOut)
	ON_COMMAND(ID_TOOLBAR_BUTTON_PLY, &CAnykaIPCameraDlg::OnPlay)
	ON_COMMAND(ID_TOOLBAR_BUTTON_VMINUS, &CAnykaIPCameraDlg::OnVolumeMinus)
	ON_COMMAND(ID_TOOLBAR_BUTTON_VPLUS, &CAnykaIPCameraDlg::OnVolumePlus)
	ON_COMMAND(ID_TOOLBAR_BUTTON_STOP_REC, &CAnykaIPCameraDlg::OnStopRecord)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CAnykaIPCameraDlg::OnTvnSelchangedTree1)
	ON_NOTIFY(NM_RCLICK, IDC_TREE1, &CAnykaIPCameraDlg::OnNMRClickTree1)
	ON_WM_CLOSE()
	ON_COMMAND(ID_32773, &CAnykaIPCameraDlg::OnSearchDevice)
	ON_WM_TIMER()
	ON_COMMAND(ID_PREVIEWCHOOSE_32774, &CAnykaIPCameraDlg::OnPreviewchoose1)
	ON_COMMAND(ID_PREVIEWCHOOSE_32775, &CAnykaIPCameraDlg::OnPreviewchoose2)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CAnykaIPCameraDlg::OnCbnSelchangeContrastCombo)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CAnykaIPCameraDlg::OnCbnSelchangeBrightnessCombo)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CAnykaIPCameraDlg::OnCbnSelchangeSaturationCombo)
	ON_COMMAND(ID_32772, &CAnykaIPCameraDlg::OnMotionDetect)
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CAnykaIPCameraDlg::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_UP, &CAnykaIPCameraDlg::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CAnykaIPCameraDlg::OnBnClickedButtonRight)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CAnykaIPCameraDlg::OnBnClickedButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_LEFTRIGHT, &CAnykaIPCameraDlg::OnBnClickedButtonLeftRight)
	ON_BN_CLICKED(IDC_BUTTON_UPDOWN, &CAnykaIPCameraDlg::OnBnClickedButtonUpDown)
	ON_BN_CLICKED(IDC_BUTTON_REPOSITION_SET, &CAnykaIPCameraDlg::OnBnClickedButtonRepositionSet)
	ON_BN_CLICKED(IDC_BUTTON_REPOSITION, &CAnykaIPCameraDlg::OnBnClickedButtonReposition)
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_PREVIEWDLGCHOOSE_TALKOPEN, &CAnykaIPCameraDlg::OnPreviewdlgchooseTalkOpen)
	ON_COMMAND(ID_PREVIEWDLGCHOOSE1_TALKCLOSE, &CAnykaIPCameraDlg::OnPreviewdlgchooseTalkClose)
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_SIZE()
	ON_MESSAGE(WM_TALK_KICKOUT, &CAnykaIPCameraDlg::OnTalkKickOutMessage)
	ON_MESSAGE(WM_SERVER_DISCONNECT, &CAnykaIPCameraDlg::OnServerDisconnect)
	ON_COMMAND(ID_PREVIEWDLGCHOOSE1_CLOSE_PREVIEW, &CAnykaIPCameraDlg::OnPreviewdlgchoose1ClosePreview)
	ON_COMMAND(ID_PREVIEWDLGCHOOSE_CLOSE_PREVIEW, &CAnykaIPCameraDlg::OnPreviewdlgchooseClosePreview)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_SERVER_RET_INFO, &CAnykaIPCameraDlg::OnServerRetInfo)
//	ON_CBN_SELCHANGE(IDC_COMBO4, &CAnykaIPCameraDlg::OnCbnSelchangeCombo4)
ON_CBN_SELCHANGE(IDC_COMBO4, &CAnykaIPCameraDlg::OnCbnSelchangeCombo4)
ON_BN_CLICKED(IDC_RADIO_IRCUT_ON, &CAnykaIPCameraDlg::OnBnClickedRadioIrcutOn)
ON_BN_CLICKED(IDC_BUTTON_SET, &CAnykaIPCameraDlg::OnBnClickedButtonSet)
ON_BN_CLICKED(IDC_RADIO_IRCUT_OFF, &CAnykaIPCameraDlg::OnBnClickedRadioIrcutOff)
ON_BN_CLICKED(IDC_BUTTON_RECOVER_DEV, &CAnykaIPCameraDlg::OnBnClickedButtonRecoverDev)
END_MESSAGE_MAP()


void CAnykaIPCameraDlg::InitToolBar()
{
	UINT nArray[8];
	nArray[0] = ID_TOOLBAR_BUTTON_PIC;
	nArray[1] = ID_TOOLBAR_BUTTON_REC;
	nArray[2] = ID_TOOLBAR_BUTTON_STOP_REC;
	nArray[3] = ID_TOOLBAR_BUTTON_PLY;
	nArray[4] = ID_TOOLBAR_BUTTON_ZIN;
	nArray[5] = ID_TOOLBAR_BUTTON_ZOUT;
	nArray[6] = ID_TOOLBAR_BUTTON_VMINUS;
	nArray[7] = ID_TOOLBAR_BUTTON_VPLUS;

	m_ToolBar.CreateEx( this, TBSTYLE_FLAT, WS_CHILD|WS_VISIBLE|CBRS_TOP );
	//m_ToolBar.SetButtons( nArray, 8 );
	//m_ToolBar.SetSizes( CSize( 48, 48 ), CSize(38, 30) );
#if 0
	m_ToolBar.SetImage("res\\5-content-picture.png");
	m_ToolBar.SetImage("res\\10-device-access-switch-video.png");
	m_ToolBar.SetImage("res\\stopRecoder.png");
	m_ToolBar.SetImage("res\\huifang.png");
	m_ToolBar.SetImage("res\\+.png");
	m_ToolBar.SetImage("res\\-.png");
	m_ToolBar.SetImage("res\\yinliangjian.png");
	m_ToolBar.SetImage("res\\yinliangjia.png", true);
	m_ToolBar.SetButtonText( 0, L"拍照" );
	m_ToolBar.SetButtonText( 1, L"录像" );
	m_ToolBar.SetButtonText( 2, L"停止录像" );
	m_ToolBar.SetButtonText( 3, L"回放" );
	m_ToolBar.SetButtonText( 4, L"放大" );
	m_ToolBar.SetButtonText( 5, L"缩小" );
	m_ToolBar.SetButtonText( 6, L"音量减" );
	m_ToolBar.SetButtonText( 7, L"音量加" );
#endif

	RepositionBars( AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0 );
}

void CAnykaIPCameraDlg::InitStatusBar()
{
	m_StatusBar.Create(this);

	UINT nArray[3] = {ID_STATUSBAR_PROGRAMINFO, ID_STATUSBAR_DISPLAYINFO1, ID_STATUSBAR_DISPLAYINFO2};
	m_StatusBar.SetIndicators(nArray, sizeof(nArray) / sizeof(nArray[0]));

	CRect rect;
	GetWindowRect(&rect);

	//m_StatusBar.SetPaneInfo(0, ID_STATUSBAR_PROGRAMINFO,  0, rect.Width() * SBAR_PROGRAMINFO_SCALE);
	//m_StatusBar.SetPaneInfo(1, ID_STATUSBAR_DISPLAYINFO1, 0, rect.Width() * SBAR_DISPLAYINFO_SCALE);
	//m_StatusBar.SetPaneInfo(2, ID_STATUSBAR_DISPLAYINFO2, 0, rect.Width() * SBAR_DISPLAYINFO_SCALE);

	//m_StatusBar.SetPaneText( 0, L"Anyka IP Camera!");

	//RepositionBars( AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0 );
}

void CAnykaIPCameraDlg::InitTreeCtrlPosition()
{
	CRect cToolBarRect, cStatusBarRect, cWindowRect;

	GetWindowRect(&cWindowRect);
	m_ToolBar.GetWindowRect(&cToolBarRect);
	m_StatusBar.GetWindowRect(&cStatusBarRect);

	ScreenToClient(&cToolBarRect);
	ScreenToClient(&cStatusBarRect);

	//m_TreeCtrl.MoveWindow( cToolBarRect.left + 2, cToolBarRect.bottom + 3, 
	//	cWindowRect.Width() / 5, cStatusBarRect.top - cToolBarRect.bottom - 4 );
	m_TreeCtrl.MoveWindow( cToolBarRect.left + 2, cToolBarRect.bottom + 3, 
		0, cStatusBarRect.top - cToolBarRect.bottom - 4 );
}

void CAnykaIPCameraDlg::InitPreviewWindows(BOOL bNeedCreate, BOOL full_flag)
{
	CRect cTreeCtrlRect, cToolBarRect, cWindowRect;

	//m_TreeCtrl.GetWindowRect( &cTreeCtrlRect );
	//m_ToolBar.GetWindowRect( &cToolBarRect );
	GetWindowRect(&cWindowRect);

	//ScreenToClient(&cTreeCtrlRect);
	//ScreenToClient(&cToolBarRect);
	ScreenToClient(&cWindowRect);



	if (bNeedCreate)
	{
		m_Preview[0].Create( IDD_DIALOG_PREVIEW1, this );
		m_Preview[1].Create( IDD_DIALOG_PREVIEW1, this );
	}
#if 0
	//m_Preview[0].MoveWindow( cTreeCtrlRect.right + 3, cToolBarRect.bottom + 3, ( cTreeCtrlRect.Height() * 8 ) / 9, cTreeCtrlRect.Height() / 2 );
	if (full_flag)
	{
		//m_Preview[0].MoveWindow( cTreeCtrlRect.right, cToolBarRect.bottom, 700, 400);//471, 265);
		//m_Preview[0].MoveWindow( cTreeCtrlRect.right, cToolBarRect.bottom, 600, 450);//471, 265);
		m_Preview[0].MoveWindow( cWindowRect.top+39, cWindowRect.left+3, cWindowRect.right, cWindowRect.bottom);

	}
	else
	{
		//m_Preview[0].MoveWindow( cTreeCtrlRect.right, cToolBarRect.bottom, 1000, 700);//471, 265);
		//m_Preview[0].MoveWindow( cTreeCtrlRect.right, cToolBarRect.bottom, 1024, 768);//471, 265);
		m_Preview[0].MoveWindow( cWindowRect.top+39, cWindowRect.left+3, cWindowRect.right+cWindowRect.left, cWindowRect.bottom);
	}
#endif

	m_Preview[0].MoveWindow( cWindowRect.top+39, cWindowRect.left+40, cWindowRect.right, cWindowRect.bottom);

	//m_Preview[0].MoveWindow( cWindowRect.top+3, cWindowRect.left+3, cWindowRect.right, cWindowRect.bottom);

	//m_Preview[1].MoveWindow( cTreeCtrlRect.right + 3, cToolBarRect.bottom + 3 + cTreeCtrlRect.Height() / 2, ( cTreeCtrlRect.Height() * 8 ) / 9, cTreeCtrlRect.Height() / 2 );

	if (bNeedCreate){
		m_Preview[0].ShowWindow( SW_SHOW );
		//m_Preview[1].ShowWindow( SW_SHOW );
	}
}

void CAnykaIPCameraDlg::InitComboBox()
{
	m_ContrastCombo.SelectString(3, L"3");
	m_SaturationCombo.SelectString(3, L"3");
	m_BrightnessCombo.SelectString(3, L"3");
	m_acutanceCom.SelectString(3, L"3");
}

void CAnykaIPCameraDlg::UpdateCombo()
{
	if (NULL == m_hCurrentSelectItem) return;

	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	if (NULL == pIServer) {
		AfxMessageBox(L"无法获取设备...严重内部错误！", 0, 0);
		return;
	}

	IMAGE_SET stImageSet;
	ZeroMemory(&stImageSet, sizeof(IMAGE_SET));
	pIServer->GetServerImageSet(stImageSet);

	CString strTemp;
	strTemp.Format(L"%d", stImageSet.nContrast);
	m_ContrastCombo.SelectString(0,strTemp);

	strTemp.Format(L"%d", stImageSet.nBrightness);
	m_BrightnessCombo.SelectString(0,strTemp);

	strTemp.Format(L"%d", stImageSet.nSaturation);
	m_SaturationCombo.SelectString(0,strTemp);

	strTemp.Format(L"%d", stImageSet.nSaturation);
	m_acutanceCom.SelectString(0, strTemp);
}

void CAnykaIPCameraDlg::InitPrivacyDialog()
{
	return;
}

// CAnykaIPCameraDlg 消息处理程序

static int av_lock_manager_cb(void ** mutex, enum AVLockOp lockOp)
{
	switch(lockOp) {
	case AV_LOCK_CREATE:
		*mutex = (void*)CreateMutex(NULL, false, NULL);
		break;
	case AV_LOCK_DESTROY:
		CloseHandle((HANDLE)*mutex);
		*mutex = NULL;
		break;
	case AV_LOCK_OBTAIN:
		WaitForSingleObject((HANDLE)*mutex, INFINITE);
		break;
	case AV_LOCK_RELEASE:
		ReleaseMutex((HANDLE)*mutex);
		break;
	}

	return 0;
}

void CAnykaIPCameraDlg::OnClientFinish(void * pLParam, void * pRParam)
{
	CAnykaIPCameraDlg * pthis = (CAnykaIPCameraDlg *)pLParam;
	CAimer39RTSPClient * pClient = (CAimer39RTSPClient *)pRParam;

	int iSelect = 0;
	for (; iSelect < 2; ++iSelect)
		if (pthis->m_pClient[iSelect] == pClient) break;

	if (iSelect >= 2) {
		AfxMessageBox( L"WARN! a client no under control finish\n", 0, 0 );
		return;
	}

	//pthis->m_Preview[iSelect].Invalidate();
}

void CAnykaIPCameraDlg::OnClientDisConnect(void * pLParam, void * pRParam)
{
	CAnykaIPCameraDlg * pthis = (CAnykaIPCameraDlg *)pLParam;
	CAimer39RTSPClient * pClient = (CAimer39RTSPClient *)pRParam;

#ifdef WARN_ERROR_OUT
	fprintf(stderr, "WARN::####Disconnet we will do the reconnect operate, because we didn't rec any video data in last 4 second####\n");
#endif

	int iSelect = 0;
	for (; iSelect < SUPPORT_STREAM_CNT; ++iSelect)
		if (pthis->m_pClient[iSelect] == pClient) break;

	if (iSelect >= SUPPORT_STREAM_CNT) {
		AfxMessageBox( L"WARN! a client no under control disconnect\n", 0, 0 );
		return;
	}

	//pthis->m_Preview[iSelect].Invalidate();//刷新Preview Dialog
	if (pthis->m_pServerPreviews[iSelect])
	{
		pthis->m_pServerPreviews[iSelect]->DisConnect(); //使断线重连Monitor线程，做后续的工作。
		
	}
	pthis->CloseServer();
	g_start_open_flag = TRUE;
	g_connet_flag = FALSE;

}

void CAnykaIPCameraDlg::OnFullScreenMessage(UINT message, WPARAM wParam, LPARAM lParam, void * pClassParam)
{
#if 0
	CAnykaIPCameraDlg * pthis = (CAnykaIPCameraDlg *)pClassParam;

	if (message == WM_LBUTTONDBLCLK ||
		(message == WM_KEYUP && wParam == VK_ESCAPE)) {
			pthis->FullScreenProcess(FALSE, pthis->m_nVideoFullScreenIndex);
	}
#endif
}

void CAnykaIPCameraDlg::OnTalkKickOut(IServer * pIServer, unsigned long ulIpAddr, unsigned short usPort, void * pClassParam)
{
	CAnykaIPCameraDlg * pthis = (CAnykaIPCameraDlg *)pClassParam;

	{
		CAutoLock lock(&(pthis->m_csForKickOut));
		pthis->m_stKickOutParam.ulIpAddr = ulIpAddr;
		pthis->m_stKickOutParam.ulPort = usPort;
	}

	pthis->PostMessage(WM_TALK_KICKOUT, 0, (LPARAM)pIServer);
}

void CAnykaIPCameraDlg::OnServerReturnInfo(IServer * pIServer, RETINFO * pstRetInfo, void * pClassParam)
{
	CAnykaIPCameraDlg * pthis = (CAnykaIPCameraDlg *)pClassParam;

	{
		CAutoLock lock(&(pthis->m_csForRet));
		memcpy(&(pthis->m_stRetInfo), pstRetInfo, sizeof(RETINFO));
	}

	pthis->PostMessage(WM_SERVER_RET_INFO, 0, (LPARAM)pIServer);
}

BOOL CAnykaIPCameraDlg::OnInitDialog()
{
	TCHAR temp_buf_det[MAX_PATH+50] = {0};
	TCHAR temp_buf_src[MAX_PATH+50] = {0};
	TCHAR bufsprintf[MAX_PATH] = {0};
	CString str;

	USES_CONVERSION;

	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	time_t t = time(0);
	struct tm * ptNow = NULL;

	char logInfoName[MAX_PATH] = {0};

	ptNow = localtime(&t);

	CreateDirectory(ConvertAbsolutePath(_T("log")), NULL);//创建文件夹
	sprintf(logInfoName, "log\\info_%04d_%02d_%02d,%02d,%02d,%02d.log", 
		ptNow->tm_year + 1900, ptNow->tm_mon + 1, ptNow->tm_mday, ptNow->tm_hour, ptNow->tm_min, ptNow->tm_sec);

#ifdef USE_LOG_FILE
	freopen(logInfoName, "w+t", stderr);
#else
	AllocConsole();
	freopen("CONOUT$", "w+t", stdout);
	freopen("CONIN$", "r+t", stdin);
	freopen("CONOUT$", "w+t", stderr);
#endif

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// TODO: 在此添加额外的初始化代码

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	avcodec_register_all();
	av_lockmgr_register(av_lock_manager_cb);

	//start the monitor thread
	m_runThreadFlag = TRUE;
	m_bNeedJudgeDisConnWork = TRUE;
	m_MonitorThread = (HANDLE)_beginthreadex(NULL, THREAD_STACK_SIZE, thread_begin, (LPVOID)this, 0, NULL);

	m_menuTalk.LoadMenu(IDR_MENU4);

	m_ircut_flag = 1;
	((CButton *)GetDlgItem(IDC_RADIO_IRCUT_ON))->SetCheck(1);
	((CButton *)GetDlgItem(IDC_RADIO_IRCUT_OFF))->SetCheck(0);

	m_uPort = 21;
	m_net_uPort = 6789;
	m_username = (_T(""));   
	m_password = (_T("")); 
	download_file_flag = FALSE;
	download_dev_file_flag = FALSE;
	g_send_commad = 0;
	g_test_fail_flag  = 0;
	g_connet_flag = FALSE;
	g_senddata_flag = TRUE;
	GetDlgItem(IDC_BUTTON_SET)->EnableWindow(FALSE);//可用 
	GetDlgItem(IDC_BUTTON_RECOVER_DEV)->EnableWindow(FALSE);//可用 

	//判断配置文件夹是否存在，如果不存在那么就进行创建
	if(0xFFFFFFFF == GetFileAttributes(ConvertAbsolutePath(TEST_CONFIG_DIR)))
	{
		CreateDirectory(ConvertAbsolutePath(TEST_CONFIG_DIR), NULL);//创建文件夹
	}


	//拷贝二个文件过去
	//swprintf(temp_buf_src, _T("%s"), ConvertAbsolutePath(_T("test_ircut")));
	//swprintf(temp_buf_det, _T("%s/test_ircut"), ConvertAbsolutePath(TEST_CONFIG_DIR));

	_tcscpy(temp_buf_src, ConvertAbsolutePath(_T("test_ircut")));
	str.Format(_T("%s/test_ircut"), ConvertAbsolutePath(TEST_CONFIG_DIR));
	_tcscpy(temp_buf_det, str);
	//str.Format(_T("AAA"));
	//	AfxMessageBox(str,  MB_OK);
	if (!CopyFile(temp_buf_src, temp_buf_det, FALSE))
	{
		//str.Format(bufsprintf, _T("copy  fail :%s"), ConvertAbsolutePath(_T("test_ircut")));
		//AfxMessageBox(bufsprintf, MB_OK);  
		//return FALSE;
	}
	else
	{
		memset(temp_buf_det, 0, MAX_PATH+50);
		//sprintf(temp_buf_det, _T("%s"), ConvertAbsolutePath(_T("test_ircut")));
		_tcscpy(temp_buf_det, ConvertAbsolutePath(_T("test_ircut")));
		DeleteFile(temp_buf_det);
	}

	memset(temp_buf_det, 0, MAX_PATH+50);
	memset(temp_buf_src, 0, MAX_PATH+50);
	//str.Format(_T("BBB"));
	//AfxMessageBox(str,  MB_OK);

	//sprintf(temp_buf_src, _T("%s"), ConvertAbsolutePath(_T("test_recover_dev")));
	//sprintf(temp_buf_det, _T("%s/test_recover_dev"), ConvertAbsolutePath(TEST_CONFIG_DIR));

	_tcscpy(temp_buf_src, ConvertAbsolutePath(_T("test_recover_dev")));
	str.Format(_T("%s/test_recover_dev"), ConvertAbsolutePath(TEST_CONFIG_DIR));
	_tcscpy(temp_buf_det, str);
	if (!CopyFile(temp_buf_src, temp_buf_det, FALSE))
	{
		//str.Format(bufsprintf, _T("copy fail:%s"), ConvertAbsolutePath(_T("test_recover_dev")));
		//AfxMessageBox(bufsprintf, MB_OK);  
	}
	else
	{
		memset(temp_buf_det, 0, MAX_PATH+50);
		//sprintf(temp_buf_det, _T("%s"), ConvertAbsolutePath(_T("test_recover_dev")));
		_tcscpy(temp_buf_det, ConvertAbsolutePath(_T("test_recover_dev")));
		DeleteFile(temp_buf_det);
	}


	//InitToolBar();
	//InitStatusBar();

	//InitTreeCtrlPosition();
	//HTREEITEM hRoot = m_TreeCtrl.InsertItem( TREE_ROOT_ITEM_NAME, TVI_ROOT, TVI_LAST );

	InitPreviewWindows(TRUE, TRUE);
	//InitComboBox();
	InitPrivacyDialog();
	//PositionTheButton();
	//PositionTheImageCombo();

	SetTimer(TIMER_COMMAND, 1000, NULL);

	m_bIsInit = TRUE;
	g_start_open_flag = TRUE;

	Creat_Anyka_Test_thread();

	//read_config(CONFIG_PATH);

	SetWindowText(TOOL_VERSOIN);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

BOOL CAnykaIPCameraDlg::PreTranslateMessage(MSG * pMsg)
{
	// TODO: 在此添加控件通知处理程序代码
	if (pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam) 
		{
		case VK_RETURN:
			return TRUE;
		case VK_ESCAPE:
			return TRUE;
		}
	}
	
	//处理云台按钮长按的事件。此处代码的作用是将鼠标在云台按钮上的操作转发给主窗口处理，这样的好处就是不需要重新继承CButton并重载实现来完成。
	if ((pMsg->message == WM_LBUTTONDOWN) || (pMsg->message == WM_LBUTTONUP) || (pMsg->message == WM_MOUSEMOVE)) {
		CWnd * pButtonLeft = GetDlgItem(IDC_BUTTON_LEFT);
		CWnd * pButtonRight = GetDlgItem(IDC_BUTTON_RIGHT);
		CWnd * pButtonUp = GetDlgItem(IDC_BUTTON_UP);
		CWnd * pButtonDown = GetDlgItem(IDC_BUTTON_DOWN);

		HWND hLeftWnd = pButtonLeft->GetSafeHwnd();
		HWND hRightWnd = pButtonRight->GetSafeHwnd();
		HWND hUpWnd = pButtonUp->GetSafeHwnd();
		HWND hDownWnd = pButtonDown->GetSafeHwnd();

		if ((pMsg->hwnd == hLeftWnd) || (pMsg->hwnd == hRightWnd) || (pMsg->hwnd == hUpWnd) || (pMsg->hwnd == hDownWnd)) {
			POINT point = {0, 0};
			point.x = GET_X_LPARAM(pMsg->lParam);
			point.y = GET_Y_LPARAM(pMsg->lParam);

			::ClientToScreen(pMsg->hwnd, &point);
			ScreenToClient(&point);
			SendMessage(pMsg->message, pMsg->wParam, MAKELPARAM(point.x, point.y));
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CAnykaIPCameraDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAnykaIPCameraDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1)/ 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}

	m_Preview[0].UpdateWindow();
	m_Preview[1].UpdateWindow();
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CAnykaIPCameraDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAnykaIPCameraDlg::VideoFunctionOpenProcess(VIDEOFUNCTION enVFun)
{
	int nCount = m_Search.GetServerCount();
	if (nCount <= 0) {
		AfxMessageBox(L"当前没有搜索到任何服务器，或没有进行搜索操作!请确认网络中存在服务器或进行搜索操作.", 0 ,0 );
		return;
	}

	if (!CanDoTheJob()) {
		AfxMessageBox(L"当前选择的设备没有进行预览，请选择一个处于预览状态的设备进行控制", 0 ,0 );
		return;
	}

	IServer * pIServer = NULL;
	char strIPAddr[MAX_IP_LEN] = {0};
	unsigned int nLen = MAX_IP_LEN;

	m_bNeedJudgeDisConnWork = FALSE;
	CAutoLock lock(&m_csForServerConnect);
	::SuspendThread(m_MonitorThread);

	IServer * pCurIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);

	if (enVFun == VF_PLAY) {
		m_RecordPlayDlg.PutServerEntry(pCurIServer);
	}

	for (int i = 0; i < nCount; ++i) {
		m_Search.GetServer(i, &pIServer);
		pIServer->GetServerIp(strIPAddr, &nLen);

		if (enVFun == VF_PRIVACY_AREA)
			m_PrivacyDialog.PutServerEntry(strIPAddr, pIServer);
		else if (enVFun == VF_MOTION_DETECT)
			m_MotionDetectDlg.PutServerEntry(strIPAddr, pIServer);
	}

	for (int i = 0; i < SUPPORT_STREAM_CNT; ++i) {
		CloseTheStream(i, TRUE);
	}

	IServer * apTempServer[PREVIEW_WINDOWS] = {NULL};

	for (int i = 0; i < PREVIEW_WINDOWS; ++i) {
		if (m_pServerPreviews[i]) {
			m_pServerPreviews[i]->DisConnect();
			apTempServer[i] = m_pServerPreviews[i];
			m_pServerPreviews[i] = NULL;
		}
	}

	if (enVFun == VF_PRIVACY_AREA)
		m_PrivacyDialog.DoModal();
	else if (enVFun == VF_MOTION_DETECT)
		m_MotionDetectDlg.DoModal();
	else if (enVFun == VF_PLAY)
		m_RecordPlayDlg.DoModal();
	else return;

	for (int i = 0; i < PREVIEW_WINDOWS; ++i) {
		if (apTempServer[i]){
			m_pServerPreviews[i] = apTempServer[i];
		}
	}
	
	//断线重连功能，完成重新打开预览流的操作。
	::ResumeThread(m_MonitorThread);
	m_bNeedJudgeDisConnWork = TRUE;
}

void CAnykaIPCameraDlg::OnPrivacyArea()
{
	// TODO: 在此添加命令处理程序代码
	VideoFunctionOpenProcess(VF_PRIVACY_AREA);
}

void CAnykaIPCameraDlg::OnMotionDetect()
{
	// TODO: 在此添加命令处理程序代码
	//m_MotionDetectDlg.DoModal();
	VideoFunctionOpenProcess(VF_MOTION_DETECT);
}

void CAnykaIPCameraDlg::OnPicture()
{
	// TODO: 在此添加命令处理程序代码
	if (!m_hCurrentSelectItem) {
		AfxMessageBox( L"未选择任何设备，请在设备列表中选择一个设备!", 0, 0 );
		return;
	}

	if (!CanDoTheJob()) {
		AfxMessageBox(L"当前选择的设备没有进行预览，请选择一个处于预览状态的设备进行控制", 0 ,0 );
		return;
	}

	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	if (NULL == pIServer) {
		AfxMessageBox(L"无法获取设备...严重内部错误！", 0, 0);
		return;
	}

	pIServer->SendTakePic();
	m_ToolBar.GetToolBarCtrl().EnableButton(ID_TOOLBAR_BUTTON_PIC, FALSE);
	m_bPicture = FALSE;
}

void CAnykaIPCameraDlg::OnPlay()
{	
	VideoFunctionOpenProcess(VF_PLAY);
}

void CAnykaIPCameraDlg::OnRecord()
{
	// TODO: 在此添加命令处理程序代码
	if (!m_hCurrentSelectItem) {
		AfxMessageBox( L"未选择任何设备，请在设备列表中选择一个设备!", 0, 0 );
		return;
	}

	if (!CanDoTheJob()) {
		AfxMessageBox(L"当前选择的设备没有进行预览，请选择一个处于预览状态的设备进行控制", 0 ,0 );
		return;
	}

	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	if (NULL == pIServer) {
		AfxMessageBox(L"无法获取设备...严重内部错误！", 0, 0);
		return;
	}

	pIServer->SendRecode();
}

void CAnykaIPCameraDlg::OnStopRecord()
{
	// TODO: 在此添加命令处理程序代码
	if (!m_hCurrentSelectItem) {
		AfxMessageBox( L"未选择任何设备，请在设备列表中选择一个设备!", 0, 0 );
		return;
	}

	if (!CanDoTheJob()) {
		AfxMessageBox(L"当前选择的设备没有进行预览，请选择一个处于预览状态的设备进行控制", 0 ,0 );
		return;
	}

	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	if (NULL == pIServer) {
		AfxMessageBox(L"无法获取设备...严重内部错误！", 0, 0);
		return;
	}

	pIServer->SendStopRecode();
}

void CAnykaIPCameraDlg::OnZoomIn()
{
	// TODO: 在此添加命令处理程序代码
	if (!m_hCurrentSelectItem) {
		AfxMessageBox( L"未选择任何设备，请在设备列表中选择一个设备!", 0, 0 );
		return;
	}

	if (!CanDoTheJob()) {
		AfxMessageBox(L"当前选择的设备没有进行预览，请选择一个处于预览状态的设备进行控制", 0 ,0 );
		return;
	}

	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	if (NULL == pIServer) {
		AfxMessageBox(L"无法获取设备...严重内部错误！", 0, 0);
		return;
	}

	HTREEITEM hSelectItem = m_TreeCtrl.GetSelectedItem();
	if (m_TreeCtrl.GetChildItem(hSelectItem)) {
		AfxMessageBox(L"当前在设备列表中选择的项不是码流项，请选择设备下的任一码流进行Zoom In/out操作！", 0, 0);
		return;
	}

	unsigned int iStreamSelect = (unsigned int)m_TreeCtrl.GetItemData(hSelectItem);
	ZOOM Zoom = iStreamSelect << 1;
	Zoom |= ZOOM_IN;

	pIServer->SendZoomInOut(Zoom);
}

void CAnykaIPCameraDlg::OnZoomOut()
{
	// TODO: 在此添加命令处理程序代码
	if (!m_hCurrentSelectItem) {
		AfxMessageBox( L"未选择任何设备，请在设备列表中选择一个设备!", 0, 0 );
		return;
	}

	if (!CanDoTheJob()) {
		AfxMessageBox(L"当前选择的设备没有进行预览，请选择一个处于预览状态的设备进行控制", 0 ,0 );
		return;
	}

	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	if (NULL == pIServer) {
		AfxMessageBox(L"无法获取设备...严重内部错误！", 0, 0);
		return;
	}

	HTREEITEM hSelectItem = m_TreeCtrl.GetSelectedItem();
	if (m_TreeCtrl.GetChildItem(hSelectItem)) {
		AfxMessageBox(L"当前在设备列表中选择的项不是码流项，请选择设备下的任一码流进行Zoom In/out操作！", 0, 0);
		return;
	}

	unsigned int iStreamSelect = (unsigned int)m_TreeCtrl.GetItemData(hSelectItem);
	ZOOM Zoom = iStreamSelect << 1;
	Zoom |= ZOOM_OUT;

	pIServer->SendZoomInOut(Zoom);
}

void CAnykaIPCameraDlg::OnVolumeMinus()
{
	// TODO: 在此添加命令处理程序代码
	IServer * pIServer = NULL;

	if (m_nAudioClientIndex < 0) {
		if (!m_hCurrentSelectItem) {
			AfxMessageBox( L"未选择任何设备，请在设备列表中选择一个设备!", 0, 0 );
			return;
		}

		if (!CanDoTheJob()) {
			AfxMessageBox(L"当前选择的设备没有进行预览，请选择一个处于预览状态的设备进行控制", 0 ,0 );
			return;
		}

		pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	}else {
		pIServer = m_pServerPreviews[m_nAudioClientIndex];
	}

	if (NULL == pIServer) {
		AfxMessageBox(L"无法获取设备...严重内部错误！", 0, 0);
		return;
	}

	pIServer->SendVolumeCtrl(VOLUME_MINUS);
}

void CAnykaIPCameraDlg::OnVolumePlus()
{
	// TODO: 在此添加命令处理程序代码
	IServer * pIServer = NULL;

	if (m_nAudioClientIndex < 0) {
		if (!m_hCurrentSelectItem) {
			AfxMessageBox( L"未选择任何设备，请在设备列表中选择一个设备!", 0, 0 );
			return;
		}

		if (!CanDoTheJob()) {
			AfxMessageBox(L"当前选择的设备没有进行预览，请选择一个处于预览状态的设备进行控制", 0 ,0 );
			return;
		}

		pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	}else {
		pIServer = m_pServerPreviews[m_nAudioClientIndex];
	}

	if (NULL == pIServer) {
		AfxMessageBox(L"无法获取设备...严重内部错误！", 0, 0);
		return;
	}

	pIServer->SendVolumeCtrl(VOLUME_PLUS);
}

void CAnykaIPCameraDlg::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CString strText = m_TreeCtrl.GetItemText(pNMTreeView->itemNew.hItem);
	if (m_TreeCtrl.GetChildItem(pNMTreeView->itemNew.hItem)) {
		if (m_TreeCtrl.GetParentItem(pNMTreeView->itemNew.hItem)) {
			m_hCurrentSelectItem = pNMTreeView->itemNew.hItem;
			UpdateCombo();
		}
	}else {
		m_hCurrentSelectItem = m_TreeCtrl.GetParentItem(pNMTreeView->itemNew.hItem);
		UpdateCombo();
	}

	*pResult = 0;
}

void CAnykaIPCameraDlg::OnNMRClickTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
#if 0
	POINT pos;
	*pResult = -1;

	if ( !GetCursorPos( &pos ) )
		return;

	m_TreeCtrl.ScreenToClient( &pos );

	UINT nFlag;
	HTREEITEM hItem = m_TreeCtrl.HitTest( pos, &nFlag );
	int MenuID = 0;

	if ( ( hItem != NULL ) && ( TVHT_ONITEM & nFlag ) ) {
		m_TreeCtrl.Select( hItem, TVGN_CARET );
		if (m_TreeCtrl.GetChildItem(hItem)) {
			MenuID = IDR_MENU2;
			if (HTREEITEM hParent = m_TreeCtrl.GetParentItem(hItem)) {
				m_hCurrentSelectItem = hItem;
				UpdateCombo();
			}
		}
		else{
			if (TREE_ROOT_ITEM_NAME == m_TreeCtrl.GetItemText(hItem))
				MenuID = IDR_MENU2;
			else {
				MenuID = IDR_MENU3;
				m_hCurrentSelectItem = m_TreeCtrl.GetParentItem(hItem);
				UpdateCombo();
			}
		}
	}else {
		MenuID = IDR_MENU2;
	}

	CMenu menu, *pm;
	if (!menu.LoadMenu(MenuID)) {
		AfxMessageBox( L"无法加载菜单！\n", 0, 0 );
		return;
	}

	pm = menu.GetSubMenu(0);
	GetCursorPos( &pos );
	pm->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);
#endif
	*pResult = 0;

}

void CAnykaIPCameraDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_runThreadFlag = FALSE;
	WiatForMonitorThreadEnd();

	for (int i = 0; i < 2; ++i) {
		CloseTheStream(i, TRUE);
	}

	CoUninitialize();

	av_lockmgr_register(NULL);
	KillTimer(TIMER_COMMAND);

	WSACleanup();

#ifndef USE_LOG_FILE
	FreeConsole();
#endif

	CDialog::OnClose();
}

void CAnykaIPCameraDlg::OnSearchDevice()
{
	// TODO: 在此添加命令处理程序代码
	m_TreeCtrl.DeleteAllItems();

	HTREEITEM hRoot = m_TreeCtrl.InsertItem( TREE_ROOT_ITEM_NAME, TVI_ROOT, TVI_LAST );
	m_hCurrentSelectItem = NULL;

	if (m_nAudioClientIndex != -1){
		OnPreviewdlgchooseTalkClose();
	}

	{
		CAutoLock lock(&m_csForServerConnect);
		m_Search.DeleteAllServer();
		ZeroMemory(m_pServerPreviews, PREVIEW_WINDOWS * sizeof(IServer*));
	}

	for (int i = 0; i < SUPPORT_STREAM_CNT; ++i) {
		CloseTheStream(i, TRUE);
	}

	for (int i = 0; i < PREVIEW_WINDOWS; ++i) m_strURL[i].clear();

	m_nAudioClientIndex = -1;

	m_Search.Search();
	m_bIsSearch = TRUE;
}

#define DEVICE_PREFIX	L"设备%d:%s(%s)"




TCHAR *CAnykaIPCameraDlg::ConvertAbsolutePath(LPCTSTR path)
{
	CString sPath;
	CString filePath;

	if (path[0] == '\0')
	{
		return NULL;
	}
	else if ((':' == path[1]) || (('\\'==path[0]) && ('\\'==path[1])))
	{
		_tcsncpy(m_path, path, MAX_PATH);
	}
	else
	{
		GetModuleFileName(NULL,sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);

		sPath.ReleaseBuffer ();
		int nPos;
		nPos=sPath.ReverseFind ('\\');
		sPath=sPath.Left (nPos+1);

		filePath = sPath + path;

		_tcsncpy(m_path, filePath, MAX_PATH);
	}

	return m_path;
}

BOOL CAnykaIPCameraDlg::read_config(LPCTSTR file_path)
{
	//读取配置文件的参数
	CString str;
	//int k;
	BOOL ret = TRUE;
	DWORD read_len = 1;


	//获取文件的属性
	if(0xFFFFFFFF == GetFileAttributes(ConvertAbsolutePath(file_path)))
	{
		return FALSE;
	}

	USES_CONVERSION;

	//打开配置文件
	HANDLE hFile = CreateFile(ConvertAbsolutePath(file_path), GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, 0, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

#ifdef _UNICODE
	//USHORT head;
	//ReadFile(hFile, &head, 2, &read_len, NULL);
#endif

	//进行一行一行读取数据
	while(read_len > 0)
	{
		int pos;
		CString subLeft, subRight;
		TCHAR ch = 0;
		TCHAR text[1024];
		int index = 0;
		UINT will_len = sizeof(TCHAR);

		while(read_len > 0 && ch != '\n')
		{
			ret = ReadFile(hFile, &ch, will_len, &read_len, NULL);
			text[index++] = ch;
		}
		text[index] = 0;

		str = text;
		int len = str.GetLength();

		//discard the lines that is blank or begin with '#'
		str.TrimLeft();
		if(str.IsEmpty() || '#' == str[0])
		{
			continue;
		}

		pos = str.Find('=');

		subLeft = str.Left(pos);
		subRight = str.Right(str.GetLength() - pos - 1);

		subLeft.TrimLeft();
		subLeft.TrimRight();
		subRight.TrimLeft();
		subRight.TrimRight();

		//nandflash
		if (_T("IP address") == subLeft)
		{
			_tcsncpy(ip_address, subRight, 260);
		}
	}

	CloseHandle(hFile);

	return TRUE;
	
}


BOOL CAnykaIPCameraDlg::Anyka_connet() 
{

	TCHAR addr_buf[50] = {0};
	UINT len = 0, i = 0, idex = 0, Ip_start_idex = 0;
	BOOL first_flag = TRUE;

	len = _tcsclen(ip_address);
	if (len > 50)
	{
		AfxMessageBox(_T("读取的ＩＰ地址是错误的"));
		return FALSE;
	}

	g_send_commad = 0;
	for (i = 0; i < len ; i++)
	{
		if (ip_address[i] == '/')
		{
			if (first_flag)
			{
				first_flag = FALSE;
				Ip_start_idex = i + 2;
			}
			idex++;
			if (idex == 3)
			{
				break;
			}
		}
	}

	_tcsncpy(addr_buf, &ip_address[7], i - Ip_start_idex);
	if (!ConnetServer(addr_buf, 0))
	{
		CloseServer();
		return FALSE;
	}

	return TRUE;
}

BOOL CAnykaIPCameraDlg::Anyka_Test_thread() 
{
	while (1)
	{
		Sleep(1000);
		if (g_start_open_flag)
		{
			Sleep(1000);
			//system("arp -d >1.txt");
			WinExec("cmd.exe /c arp -d >1.txt", SW_HIDE);
			//读配置文件
			read_config(CONFIG_PATH);
			g_start_open_flag = FALSE;
			if (!OnPreviewchoose_test())
			{
				g_start_open_flag = TRUE;
			}
			else
			{
				if (!Anyka_connet())
				{
					AfxMessageBox(_T("IRCUT测试网连接失败"), MB_OK);
					g_connet_flag = FALSE;	
				}
				else
				{
					g_connet_flag = TRUE;
				}
			}
		}
	}
}

DWORD WINAPI Creat_Anyka_Test_main(LPVOID lpParameter) 
{
	CAnykaIPCameraDlg testDlg;

	if (!testDlg.Anyka_Test_thread())
	{
		return 0;
	}

	return 1;
}

BOOL CAnykaIPCameraDlg::Creat_Anyka_Test_thread() 
{
	UINT idex = 0;
	g_hWnd = m_Preview[0].m_hWnd;

	if (g_hTestThread != INVALID_HANDLE_VALUE)
	{
		Close_Anyka_Test_thread();
		g_hTestThread = INVALID_HANDLE_VALUE;
	}

	g_hTestThread = CreateThread(NULL, 0, Creat_Anyka_Test_main, &idex, 0, NULL);
	if (g_hTestThread == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;

}

void CAnykaIPCameraDlg::Close_Anyka_Test_thread() 
{
	if(g_hTestThread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hTestThread);
		g_hTestThread = INVALID_HANDLE_VALUE;
	}

}


BOOL flag_tool = TRUE;
void CAnykaIPCameraDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	USES_CONVERSION;
	static int SearchCnt = 0;
	static int pictureWaitCnt = 0;

	if (0)//flag_tool)
	{
		flag_tool = FALSE;
		Creat_Anyka_Test_thread();
	}

	if (!g_connet_flag)
	{
		GetDlgItem(IDC_BUTTON_SET)->EnableWindow(FALSE);//可用 
		GetDlgItem(IDC_BUTTON_RECOVER_DEV)->EnableWindow(FALSE);//可用 
		
	}
	else
	{
		GetDlgItem(IDC_BUTTON_SET)->EnableWindow(TRUE);//可用 
		GetDlgItem(IDC_BUTTON_RECOVER_DEV)->EnableWindow(TRUE);//可用 
	}

#if 1

#if 0
	if (g_start_open_flag)
	{
		Sleep(1000);
		//system("arp -d >1.txt");
		WinExec("cmd.exe /c arp -d >1.txt", SW_HIDE);
		//读配置文件
		read_config(CONFIG_PATH);
		g_start_open_flag = FALSE;
		if (!OnPreviewchoose_test())
		{
			g_start_open_flag = TRUE;
		}
	}
#endif

#else


	if (nIDEvent == TIMER_COMMAND) {
		if (m_bIsSearch && (SearchCnt < 3)) {
			m_Search.Search();
			++SearchCnt;
		}

		if (SearchCnt == 3) {
			SearchCnt = 0;
			m_bIsSearch = FALSE;
			int nCount = 0;
			if (!(nCount = m_Search.GetServerCount())) {
				AfxMessageBox(L"未搜索到任何服务器!", 0, 0);
			}else {
				char strServerID[MAX_ID_LEN] = {0};
				char strServerIP[MAX_IP_LEN] = {0};
				unsigned int len = MAX_ID_LEN;
				STREAMMODE mode = STREAM_MODE_MAX;
				BOOL bFind = FALSE;
				HTREEITEM hRoot = m_TreeCtrl.GetRootItem();
				CString strServerIDShow;

				for (int i = 0; i < nCount; ++i) {
					IServer * pIServer;
					m_Search.GetServer(i, &pIServer);

					len = MAX_ID_LEN;
					ZeroMemory(strServerID, len);
					pIServer->GetServerID(strServerID, &len);

					len = MAX_IP_LEN;
					ZeroMemory(strServerIP, len);
					pIServer->GetServerIp(strServerIP, &len);

					strServerIDShow.Format(DEVICE_PREFIX, i, A2W(strServerID), A2W(strServerIP));

					HTREEITEM hDevice = m_TreeCtrl.InsertItem(strServerIDShow, hRoot);

					unsigned int nCnt = 0;
					pIServer->GetServerStreamCnt(nCnt);

					for (unsigned int j = 0; j < nCnt; ++j) {
						pIServer->GetServerStreamMode(j, mode);
						if (mode >= STREAM_MODE_MAX) {
							continue;
						}

						if (mode == STREAM_MODE_VIDEO_720P)	m_TreeCtrl.SetItemData(m_TreeCtrl.InsertItem(VIDEO_MODE_NAME_720P, hDevice), (DWORD_PTR)j);
						else if (mode == STREAM_MODE_VIDEO_VGA) m_TreeCtrl.SetItemData(m_TreeCtrl.InsertItem(VIDEO_MODE_NAME_VGA, hDevice), (DWORD_PTR)j);
						else if (mode == STREAM_MODE_VIDEO_QVGA) m_TreeCtrl.SetItemData(m_TreeCtrl.InsertItem(VIDEO_MODE_NAME_QVGA, hDevice), (DWORD_PTR)j);
						else if (mode == STREAM_MODE_VIDEO_D1) m_TreeCtrl.SetItemData(m_TreeCtrl.InsertItem(VIDEO_MODE_NAME_D1, hDevice), (DWORD_PTR)j);
						else continue;
					}

					m_TreeCtrl.SetItemData(hDevice, (DWORD_PTR)pIServer);
				}//for (int i = 0; i < nCount; ++i)

				AfxMessageBox(L"搜索成功!", 0, 0);
				m_TreeCtrl.Expand(m_TreeCtrl.GetRootItem(), TVE_EXPAND);
			}//if (!(nCount = m_Search.GetServerCount())) else
		}//SearchCnt == 3

		//show fps and bits rate info in the Status Bar
		char strDeviceID[MAX_ID_LEN] = {0};
		unsigned int nLen = MAX_ID_LEN;
		WCHAR strInfo[1024] = {0};

		for (int i = 0; i < SUPPORT_STREAM_CNT; ++i) {
			m_StatusBar.SetPaneText(  i + 1, L"");
			if (m_pClient[i] != NULL && m_videoRender[i] != NULL) {
				if (m_pServerPreviews[i]) {
					nLen = MAX_ID_LEN;
					m_pServerPreviews[i]->GetServerID(strDeviceID, &nLen);

					double dBR = m_pClient[i]->GetBitsRatePerSec() / (double)1000;
					if (dBR < 1.0) dBR = 0.0;

					_sntprintf_s(strInfo, 1024, 1024, L"%s:%dFps,%0.2fKbps", A2W(strDeviceID), m_videoRender[i]->getFpsOneSec(), dBR);
					m_StatusBar.SetPaneText(i + 1, strInfo);
				}
			}
		}
		
		//有效拍照工具栏按钮，不允许用户连续快速的使用拍照功能。
		if (!m_bPicture) {
			++pictureWaitCnt;
			if (pictureWaitCnt > 1) {
				m_bPicture = TRUE;
				pictureWaitCnt = 0;
				m_ToolBar.GetToolBarCtrl().EnableButton(ID_TOOLBAR_BUTTON_PIC, TRUE);
			}
		}

#ifdef USE_LOG_FILE
		fflush(stderr);
#endif
	} //nIDEvent == TIMER_COMMAND

	if (nIDEvent == TIMER_LONG_PRESS) {
		if (!m_bIsLongPress) {//长按第一个500ms到来时，发送COTINUE消息
			if (m_nLongPressButtonID == IDC_BUTTON_LEFT) {
				CameraMovement(CMT_STEP_LEFT_CONTINUE);
				m_bIsLongPress = TRUE;
			}else if (m_nLongPressButtonID == IDC_BUTTON_RIGHT) {
				CameraMovement(CMT_STEP_RIGHT_CONTINUE);
				m_bIsLongPress = TRUE;
			}else if (m_nLongPressButtonID == IDC_BUTTON_UP) {
				CameraMovement(CMT_STEP_UP_CONTINUE);
				m_bIsLongPress = TRUE;
			}else if (m_nLongPressButtonID == IDC_BUTTON_DOWN) {
				CameraMovement(CMT_STEP_DOWN_CONTINUE);
				m_bIsLongPress = TRUE;
			}else {
				//用户不再长按
			}
		}
	}
#endif

	CDialog::OnTimer(nIDEvent);
}

char * CAnykaIPCameraDlg::MakeRTSPUrl()
{
	WCHAR astrMsg[100] = {0};
	static char strURL[MAX_RTSP_URL_LEN] = { 0 };
	unsigned int iStreamSelect = 0, nCnt = 0, nPort = 0;

	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	HTREEITEM hSelectItem = m_TreeCtrl.GetSelectedItem();
	iStreamSelect = (unsigned int)m_TreeCtrl.GetItemData(hSelectItem);

	char strStreamName[MAX_STREAM_NAME] = {0};
	char strIPAddr[MAX_IP_LEN];
	unsigned int len = MAX_IP_LEN;

	ZeroMemory(strURL, sizeof(strURL));

	strncpy(strURL, RTSP_PREFIX, strlen(RTSP_PREFIX));
	pIServer->GetServerIp(strIPAddr, &len);
	strncat(strURL, strIPAddr, len);

	pIServer->GetServerStreamPort(nPort);
	if (nPort) {
		strncat(strURL, PORT_PREFIX, strlen(PORT_PREFIX));
		char strPort[10] = {0};
		sprintf(strPort, "%d", nPort);
		strncat(strURL, strPort, strlen(strPort));
	}

	strncat(strURL, SEPARATOR, strlen(SEPARATOR));

	len = MAX_STREAM_NAME;
	pIServer->GetServerStreamName(iStreamSelect, strStreamName, &len);
	strncat(strURL, strStreamName, len);

	return strURL;
}

int CAnykaIPCameraDlg::CloseTheStream(int iSelect, BOOL bNeedCloseAudio)
{
	if (iSelect > 2 || iSelect < 0) return -1;

	CAutoLock lock(&m_csForOpenCloseStream);
	if (m_pClient[iSelect]) {
		m_pClient[iSelect]->Close();
		delete m_pClient[iSelect];
	}

	m_pClient[iSelect] = NULL;

	if (m_videoDecoder[iSelect]) delete m_videoDecoder[iSelect];
	m_videoDecoder[iSelect] = NULL;
	if (m_videoRender[iSelect]) delete m_videoRender[iSelect];
	m_videoRender[iSelect] = NULL;

	if ((iSelect == m_nAudioClientIndex) && bNeedCloseAudio) TempCloseTalk(iSelect);
	return 0;
}

#define MAX_WAIT_CNT	20

int CAnykaIPCameraDlg::OpenTheStream(int iSelect, const char * strURL, BOOL bNeedENotify)
{
	USES_CONVERSION;

	WCHAR astrMsg[300] = {0};
	int iErrorCode = 0;

	if (strURL == NULL) {
		if (bNeedENotify)
			AfxMessageBox( L"无法打开空rtsp地址!", 0, 0 );

		return 0;
	}

	unsigned int iStreamChoose = 0, iSCnt = 0, len = MAX_STREAM_NAME;
	int iFps = 0;

	const char * pWhere = NULL;
#if 0
	m_pServerPreviews[iSelect]->GetServerStreamCnt(iSCnt);

	pWhere = strrchr(strURL, CHAR_SEPARATOR);
	if (pWhere == NULL) 
		iSCnt = 0;

	pWhere += 1;
	char strStreamName[MAX_STREAM_NAME] = {0};

	for (iStreamChoose = 0; iStreamChoose < iSCnt; ++iStreamChoose) {
		len = MAX_STREAM_NAME;
		ZeroMemory(strStreamName, MAX_STREAM_NAME * sizeof(char));

		if (m_pServerPreviews[iSelect]->GetServerStreamName(iStreamChoose, strStreamName, &len) < 0)
			continue;

		if (strcmp(pWhere, strStreamName) == 0)
			break;
	}

	if (iStreamChoose < iSCnt)
		m_pServerPreviews[iSelect]->GetStreamFps(iStreamChoose, iFps);
	else
#endif
		iFps = 30;

	CAutoLock lock(&m_csForOpenCloseStream);

	if (m_nVideoFullScreenIndex == -1) {
		if (m_pClient[iSelect] != NULL)	
			CloseTheStream(iSelect, TRUE);
	}else {//full screen, and we recv a disconnect message.
		if (m_nVideoFullScreenIndex == iSelect) {
			if (m_pClient[iSelect]) {
				m_pClient[iSelect]->Close();
				delete m_pClient[iSelect];
			}

			m_pClient[iSelect] = NULL;

			if (m_videoDecoder[iSelect]) delete m_videoDecoder[iSelect];
			m_videoDecoder[iSelect] = NULL;

			if (iSelect == m_nAudioClientIndex) TempCloseTalk(m_nAudioClientIndex);
		}
	}


	m_pClient[iSelect] = CAimer39RTSPClient::CreateNew();
	if (NULL == m_pClient[iSelect]) {
		if (bNeedENotify)
			AfxMessageBox( L"无法创建流...内存不足!", 0, 0 );
		return -1;
	}

	m_pClient[iSelect]->RegisterFinishCallback(OnClientFinish, this);
	m_pClient[iSelect]->RegisterDisConnCallback(OnClientDisConnect, this);

	iErrorCode = m_pClient[iSelect]->OpenURL(strURL);
	if (iErrorCode < 0) {
		if (bNeedENotify) {
			_sntprintf(astrMsg, 300, L"OpenURL %s error! error = %s", A2W(strURL), A2W(m_pClient[iSelect]->GetLastError()));
			AfxMessageBox( astrMsg, 0, 0 );
		}
		return -1;
	}

	int nWaitCnt = 0;
	bool isPrepare = false;
	while (!isPrepare) {
		iErrorCode = m_pClient[iSelect]->IsPrepare(isPrepare);
		if ((iErrorCode != 0) || (nWaitCnt >= MAX_WAIT_CNT)) {
			if (bNeedENotify) {
				if ((iErrorCode == 0) && (nWaitCnt >= MAX_WAIT_CNT)) 
					_sntprintf(astrMsg, 300, L"连接服务器%s, 超时！", A2W(strURL));
				else
					_sntprintf(astrMsg, 300, L"OpenURL %s error! error = %s", A2W(strURL), A2W(m_pClient[iSelect]->GetLastError()));

				//AfxMessageBox( astrMsg, 0, 0 );
			}

			m_pClient[iSelect]->Close();
			delete m_pClient[iSelect];
			m_pClient[iSelect] = NULL;
			return -1;
		}

		++nWaitCnt;
		Sleep(100);
	}

	unsigned int iStreamCnt = 0;
	STREAM_TYPE type = STREAM_AUDIO;
	m_pClient[iSelect]->GetStreamCount(iStreamCnt);

	for (unsigned int i = 0; i < iStreamCnt; ++i) {
		m_pClient[iSelect]->GetStreamType(i, type);

		if (type == STREAM_AUDIO) {
			//预览开始时默认不播放音频
		}else if (type == STREAM_VIDEO) {
			int nReChooseSyncClock = -1;

			m_videoDecoder[iSelect] = CFfmpegEnvoy::createNew();
			if ((m_videoRender[iSelect] == NULL) || (m_nVideoFullScreenIndex == -1)) {
				m_videoRender[iSelect] = CVideoRender::createNew();
				if ((iErrorCode = m_videoRender[iSelect]->OpenRender(g_hWnd)) < 0) {    //m_Preview[iSelect].m_hWnd
					fprintf(stderr, "OpenTheStream::OpenRender error!\n");
					return iErrorCode;
				}
				m_videoRender[iSelect]->SetFillMode(KeepAspectRatio);
			}else {
				m_videoRender[iSelect]->Reset();
			}

			m_videoRender[iSelect]->SetServerStreamFps(iFps);

			m_videoDecoder[iSelect]->OpenFfmpeg();
			//clock sync use
			/*for (int i = 0; i < PREVIEW_WINDOWS; ++i) {
			if (i == iSelect) continue;

			if (m_pServerPreviews[i] == m_pServerPreviews[iSelect] && m_SyncClock[i].IsStart())
			nReChooseSyncClock = i;
			}

			if (nReChooseSyncClock < 0) {
			m_SyncClock[iSelect].ReInit();
			m_videoRender[iSelect]->setClock(&m_SyncClock[iSelect]);
			}else {
			m_videoRender[iSelect]->setClock(&m_SyncClock[nReChooseSyncClock]);
			}*/

			m_pClient[iSelect]->RegisterSink(type, m_videoDecoder[iSelect]);
			m_videoDecoder[iSelect]->RegisterSink(m_videoRender[iSelect], SINK_VIDEO);
		}
	}

	m_pClient[iSelect]->Play();

	if (m_videoDecoder[iSelect])
		m_videoDecoder[iSelect]->Start();

	//if (m_AudioDecoder[iSelect])
	//m_AudioDecoder[iSelect]->Start();

	return 0;
}

int CAnykaIPCameraDlg::RegisterThePreviewServer(IServer * pIServer, int iSelect, const char * strURL)
{
	USES_CONVERSION;
	if (NULL == pIServer || iSelect > PREVIEW_WINDOWS || iSelect < 0) return -1;

	if (m_pServerPreviews[iSelect] == pIServer) {
		if (strcmp((m_strURL[iSelect].c_str()), strURL)) {
			m_strURL[iSelect].clear();
			m_strURL[iSelect] = strURL;
		}

		return 0; // already registered
	}

	CAutoLock lock(&m_csForServerConnect);

	if (m_pServerPreviews[iSelect]) {
		m_pServerPreviews[iSelect]->DisConnect();
	}

	if (pIServer->Connect() < 0) {
		WCHAR astrMsg[100] = {0};
		char strIPAddr[MAX_IP_LEN] = {0};
		unsigned int nLen = MAX_IP_LEN;

		pIServer->GetServerIp(strIPAddr, &nLen);

		_sntprintf(astrMsg, 100, L"无法连接到服务器%s", A2W(strIPAddr));
		AfxMessageBox( astrMsg, 0, 0 );
		return -1;
	}

	int iRet = pIServer->SendGetServerInfo();
	if (iRet < 0) {
		pIServer->DisConnect();
		return -1;
	}

	BOOL bIsRespond = FALSE;
	pIServer->GetServerRespondComplete(bIsRespond);
	int iAttemptOpenCnt = 0;

	while(!bIsRespond && iAttemptOpenCnt < ATTEMPT_OPEN_MAX) {
		Sleep(200);
		pIServer->GetServerRespondComplete(bIsRespond);
		++iAttemptOpenCnt;
	}

	if (bIsRespond) {
		IMAGE_SET stImageSet = {0};
		pIServer->GetServerImageSet(stImageSet);
		if ((stImageSet.nBrightness == 255) && 
			(stImageSet.nContrast == 255) && (stImageSet.nSaturation == 255)) { // the server don't want us to connect to it, because the server is connect limit was reached.
				WCHAR astrMsg[100] = {0};
				char strIPAddr[MAX_IP_LEN] = {0};
				unsigned int nLen = MAX_IP_LEN;

				pIServer->GetServerIp(strIPAddr, &nLen);

				_sntprintf(astrMsg, 100, L"服务器%s的连接数已经达到上限，服务器禁止我们的连接!", A2W(strIPAddr));
				AfxMessageBox(astrMsg, 0, 0);

				pIServer->DisConnect();
				return -1;
		}
	}

	m_pServerPreviews[iSelect] = pIServer;
	m_strURL[iSelect].clear();
	m_strURL[iSelect] = strURL;
	//m_pServerPreviews[iSelect]->SetCurrentPlayURL(strURL);

	m_pServerPreviews[iSelect]->SetServerRetCallBack(OnServerReturnInfo, this);

	return 0;
}

int CAnykaIPCameraDlg::UnregisterThePreviewServer(int iSelect)
{
	if (iSelect > PREVIEW_WINDOWS || iSelect < 0) return -1;

	if (m_pServerPreviews[iSelect] == NULL) return 0; // already unregistered

	BOOL bNeedDisConnect = TRUE;

	CAutoLock lock(&m_csForServerConnect);

	for (int iIndex = 0; iIndex < PREVIEW_WINDOWS; ++iIndex) {
		if (iIndex == iSelect) continue;
		if (m_pServerPreviews[iIndex] == m_pServerPreviews[iSelect])
			bNeedDisConnect = FALSE;
	}

	if (bNeedDisConnect)
		m_pServerPreviews[iSelect]->DisConnect();

	m_pServerPreviews[iSelect]->SetServerRetCallBack(NULL, NULL);
	m_pServerPreviews[iSelect] = NULL;
	m_strURL[iSelect].clear();

	return 0;
}


BOOL CAnykaIPCameraDlg::OnPreviewchoose_test()
{
	// TODO: 在此添加命令处理程序代码
	int ret = 0;
	//IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	//const char * strURL = "rtsp://172.22.5.8/vs1";//MakeRTSPUrl();
	char strURL[MAX_RTSP_URL_LEN] = {0};

	USES_CONVERSION;


	memcpy(strURL, T2A(ip_address), MAX_RTSP_URL_LEN);

	if (0)//RegisterThePreviewServer(pIServer, 0, strURL) < 0)
	{
		return FALSE;
	}

	if ((ret = OpenTheStream(0, strURL)) < 0) 
	{
		//UnregisterThePreviewServer(0);

		if (ret == -2) 
		{
			AfxMessageBox(L"D3D9 class initialize failed!");
		}
		return FALSE;
	}

	if (m_nAudioClientIndex == 0)
		m_nAudioClientIndex = -1;


	return TRUE;
}



void CAnykaIPCameraDlg::OnPreviewchoose1()
{
	// TODO: 在此添加命令处理程序代码
	int ret = 0;
	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	const char * strURL = MakeRTSPUrl();

	if (RegisterThePreviewServer(pIServer, 0, strURL) < 0) return;

	if ((ret = OpenTheStream(0, strURL)) < 0) {
		UnregisterThePreviewServer(0);

		if (ret == -2) {
			AfxMessageBox(L"D3D9 class initialize failed!");
		}
	}

	if (m_nAudioClientIndex == 0)
		m_nAudioClientIndex = -1;
}

void CAnykaIPCameraDlg::OnPreviewchoose2()
{
	// TODO: 在此添加命令处理程序代码
	int ret = 0;
	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	const char * strURL = MakeRTSPUrl();

	if (RegisterThePreviewServer(pIServer, 1, strURL) < 0) return;

	if ((ret = OpenTheStream(1, strURL)) < 0){
		UnregisterThePreviewServer(1);

		if (ret == -2) {
			AfxMessageBox(L"D3D9 class initialize failed!");
		}
	}

	if (m_nAudioClientIndex == 1)
		m_nAudioClientIndex = -1;
}

void CAnykaIPCameraDlg::MakeAndSendImageSet()
{
	int nIndex = m_ContrastCombo.GetCurSel();

	CString strContrast, strSaturation, strBrightness, strAcutance;
	m_ContrastCombo.GetLBText(nIndex, strContrast);

	nIndex = m_SaturationCombo.GetCurSel();
	m_SaturationCombo.GetLBText(nIndex, strSaturation);

	nIndex = m_BrightnessCombo.GetCurSel();
	m_BrightnessCombo.GetLBText(nIndex, strBrightness);

	nIndex = m_acutanceCom.GetCurSel();
	m_acutanceCom.GetLBText(nIndex, strAcutance);

	int iContrast = 0, iSaturation = 0, iBrightness = 0, iAcutance=0;
	iContrast = _ttoi(strContrast);
	iSaturation = _ttoi(strSaturation);
	iBrightness = _ttoi(strBrightness);
	iAcutance = _ttoi(strAcutance);

	IMAGE_SET stImageSet = {0};

	if (!m_hCurrentSelectItem) {
		AfxMessageBox( L"未选择任何设备，请在设备列表中选择一个设备!", 0, 0 );
		return;
	}

	stImageSet.nBrightness = iBrightness;
	stImageSet.nContrast = iContrast;
	stImageSet.nSaturation = iSaturation;
	stImageSet.nReserve = iAcutance;

	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	pIServer->SendImageSet(stImageSet);
}

void CAnykaIPCameraDlg::OnCbnSelchangeContrastCombo()
{
	// TODO: 在此添加控件通知处理程序代码
	MakeAndSendImageSet();
}

void CAnykaIPCameraDlg::OnCbnSelchangeBrightnessCombo()
{
	// TODO: 在此添加控件通知处理程序代码
	MakeAndSendImageSet();
}

void CAnykaIPCameraDlg::OnCbnSelchangeSaturationCombo()
{
	// TODO: 在此添加控件通知处理程序代码
	MakeAndSendImageSet();
}

BOOL CAnykaIPCameraDlg::CanDoTheJob()
{
	if (!m_hCurrentSelectItem) return FALSE;

	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);

	for (int i = 0; i < PREVIEW_WINDOWS; ++i) {
		if (pIServer == m_pServerPreviews[i]) return TRUE;
	}

	return FALSE;
}

int CAnykaIPCameraDlg::OnAudioInOpen(int nAudioClientIndex)
{
	if (nAudioClientIndex > SUPPORT_STREAM_CNT || nAudioClientIndex < 0) return -1;

	if (NULL == m_pClient[nAudioClientIndex]) return -1;

	if (m_AudioDecoder[0]) {
		for (int i = 0; i < PREVIEW_WINDOWS; ++i) {
			if (m_pClient[i]) {
				m_pClient[i]->UnregisterSink(m_AudioDecoder[0]);
			}
		}

		m_AudioDecoder[0]->UnregisterSink(m_AudioRender[0], SINK_AUDIO);
		delete m_AudioDecoder[0];
	}

	m_AudioDecoder[0] = NULL;

	if (m_AudioRender[0]) {
		//Audio Render time sync use
		for (int i = 0; i < PREVIEW_WINDOWS; ++i) {
			if ((i != nAudioClientIndex) && (m_videoRender[i])) m_videoRender[i]->SetAudioRender(NULL);
		}

		delete m_AudioRender[0];
	}

	m_AudioRender[0] = NULL;

	m_AudioDecoder[0] = CFfmpegEnvoy::createNew();
	m_AudioRender[0] = CAudioRender::createNew();

	m_AudioDecoder[0]->OpenFfmpeg();
	m_AudioRender[0]->OpenRender();
	//clock sync use
	/*int nReChooseSyncClock = -1;
	for (int i = 0; i < PREVIEW_WINDOWS; ++i) {
	if (i == nAudioClientIndex) continue;

	if (m_pServerPreviews[i] == m_pServerPreviews[nAudioClientIndex] && m_SyncClock[i].IsStart())
	nReChooseSyncClock = i;
	}

	if (nReChooseSyncClock >= 0)
	m_AudioRender[0]->setClock(&m_SyncClock[nReChooseSyncClock]);
	else 
	m_AudioRender[0]->setClock(&m_SyncClock[nAudioClientIndex]);*/

	//Audio Render time sync use
	if (m_videoRender[nAudioClientIndex]) {
		m_videoRender[nAudioClientIndex]->SetAudioRender(m_AudioRender[0]);
	}

	m_pClient[nAudioClientIndex]->RegisterSink(STREAM_AUDIO, m_AudioDecoder[0]);
	m_AudioDecoder[0]->RegisterSink(m_AudioRender[0], SINK_AUDIO);

	if (m_AudioDecoder[0]) {
		m_AudioDecoder[0]->Start();
	}

	return 0;
}

int CAnykaIPCameraDlg::OnAudioInClose(int nAudioClientIndex)
{
	//Audio Render time sync use
	if (m_videoRender[nAudioClientIndex]) m_videoRender[nAudioClientIndex]->SetAudioRender(NULL);
	if (m_pClient[nAudioClientIndex]) m_pClient[nAudioClientIndex]->UnregisterSink(m_AudioDecoder[0]);


	if (m_AudioDecoder[0]) delete m_AudioDecoder[0];
	m_AudioDecoder[0] = NULL;

	if (m_AudioRender[0]) delete m_AudioRender[0];
	m_AudioRender[0] = NULL;
	return 0;
}

LRESULT CAnykaIPCameraDlg::OnServerDisconnect(WPARAM wParam, LPARAM lParam)
{
	if (ServerDisConnect((int)lParam) < 0 ) return -1;
	else return 0;
}

void CAnykaIPCameraDlg::UpdateTreeNode(IServer * UpdateServer)
{
	if (!UpdateServer) return;

	HTREEITEM hRoot = m_TreeCtrl.GetRootItem();
	HTREEITEM hItem = NULL, hChildItem = NULL;

	if (!(hItem = m_TreeCtrl.GetChildItem(hRoot))) {
		return;
	}

	IServer * pServer = NULL;
	while(hItem) {
		pServer = (IServer *)m_TreeCtrl.GetItemData(hItem);
		if (pServer == UpdateServer)
			break;

		hItem = m_TreeCtrl.GetNextItem(hItem, TVGN_NEXT);
	}

	if (hItem == NULL) return;

	hChildItem = m_TreeCtrl.GetChildItem(hItem);

	STREAMMODE mode = STREAM_MODE_MAX;
	int iStreamNum = 0;

	while(hChildItem) {
		iStreamNum = m_TreeCtrl.GetItemData(hChildItem);

		UpdateServer->GetServerStreamMode(iStreamNum, mode);
		if (mode >= STREAM_MODE_MAX) {
			continue;
		}

		if (mode == STREAM_MODE_VIDEO_720P)	m_TreeCtrl.SetItemText(hChildItem, VIDEO_MODE_NAME_720P);
		else if (mode == STREAM_MODE_VIDEO_VGA) m_TreeCtrl.SetItemText(hChildItem, VIDEO_MODE_NAME_VGA);
		else if (mode == STREAM_MODE_VIDEO_QVGA) m_TreeCtrl.SetItemText(hChildItem, VIDEO_MODE_NAME_QVGA);
		else if (mode == STREAM_MODE_VIDEO_D1) m_TreeCtrl.SetItemText(hChildItem, VIDEO_MODE_NAME_D1);
		else continue;

		hChildItem = m_TreeCtrl.GetNextItem(hChildItem, TVGN_NEXT);
	}
}

int CAnykaIPCameraDlg::ServerDisConnect(int iSelect)
{
	if (m_pServerPreviews[iSelect] == NULL) return -1;

	if (m_pServerPreviews[iSelect]->IsDisConnect())
	{
		int iRet = m_pServerPreviews[iSelect]->Connect();
		if (iRet < 0) {
#ifdef WARN_ERROR_OUT
			char strIPAddr[MAX_IP_LEN] = {0};
			unsigned int nLen = MAX_IP_LEN;

			m_pServerPreviews[iSelect]->GetServerIp(strIPAddr, &nLen);
			fprintf(stderr, "WARN::####Disconnet server : %s connect failed####\n", strIPAddr);
#endif
			m_pServerPreviews[iSelect]->DisConnect();
			return -1;
		}
		
		//断线重连成功后，需要重新发送获取ServerInfo的命令，以便获取最新的服务端的基本服务信息。
		iRet = m_pServerPreviews[iSelect]->SendGetServerInfo();
		if (iRet < 0) {
#ifdef WARN_ERROR_OUT
			char strIPAddr[MAX_IP_LEN] = {0};
			unsigned int nLen = MAX_IP_LEN;

			m_pServerPreviews[iSelect]->GetServerIp(strIPAddr, &nLen);
			fprintf(stderr, "WARN::####Disconnet server : %s Send Get Server Info failed####\n", strIPAddr);
#endif
			m_pServerPreviews[iSelect]->DisConnect();
			return -1;
		}

		BOOL bIsRespond = FALSE;
		//服务端是否回复了它的基本服务信息。
		m_pServerPreviews[iSelect]->GetServerRespondComplete(bIsRespond);
		int iAttemptOpenCnt = 0;

		while(!bIsRespond && iAttemptOpenCnt < ATTEMPT_OPEN_MAX) {
			Sleep(200);
			m_pServerPreviews[iSelect]->GetServerRespondComplete(bIsRespond);
			++iAttemptOpenCnt;
		}

		if (iAttemptOpenCnt >= ATTEMPT_OPEN_MAX && !bIsRespond) {
#ifdef WARN_ERROR_OUT
			char strIPAddr[MAX_IP_LEN] = {0};
			unsigned int nLen = MAX_IP_LEN;

			m_pServerPreviews[iSelect]->GetServerIp(strIPAddr, &nLen);
			fprintf(stderr, "WARN::####Disconnet server : %s Send Get Server Info time out####\n", strIPAddr);
#endif

			goto Next;
		}else if (bIsRespond) {
			IMAGE_SET stImageSet = {0};
			m_pServerPreviews[iSelect]->GetServerImageSet(stImageSet);
			if ((stImageSet.nBrightness == 255) && 
				(stImageSet.nContrast == 255) && (stImageSet.nSaturation == 255)) { // the server don't want us to connect to it, because the server is connect limit was reached.
					AfxMessageBox(L"服务器的连接数已经达到上限，服务器禁止我们的连接!", 0, 0);

					CloseTheStream(iSelect, TRUE);
					if (iSelect == m_nAudioClientIndex)
						m_nAudioClientIndex = -1;

					m_pServerPreviews[iSelect]->DisConnect();
					m_pServerPreviews[iSelect] = NULL;
					m_strURL[iSelect].clear();

					return -1;
			}
		}

		UpdateTreeNode(m_pServerPreviews[iSelect]);
	}

Next:
	if (m_strURL[iSelect].empty()) {
		AfxMessageBox(L"从服务器中获取不到已经在播放的rtsp地址，这个错误将导致重连失败，无法播放视频流。", 0, 0);
#ifdef WARN_ERROR_OUT
		fprintf(stderr, "WARN::####can't get the play url from server, can't play the stream####\n");
#endif
		CloseTheStream(iSelect, TRUE);
		if (iSelect == m_nAudioClientIndex)
			m_nAudioClientIndex = -1;

		m_pServerPreviews[iSelect]->DisConnect();
		m_pServerPreviews[iSelect] = NULL;
		return -1;
	}

	if (OpenTheStream(iSelect, m_strURL[iSelect].c_str(), FALSE) < 0) {
#ifdef WARN_ERROR_OUT
		fprintf(stderr, "WARN::####open stream error!####\n");
#endif
		m_pServerPreviews[iSelect]->DisConnect();
		return -1;	
	}

	if (iSelect == m_nAudioClientIndex) { //此Client是正在对讲的Client
		m_nRBChoosePrevIndex = m_nAudioClientIndex;
		OnPreviewdlgchooseTalkOpen();
	}

#ifdef WARN_ERROR_OUT
	char strIPAddr[MAX_IP_LEN] = {0};
	unsigned int nLen1 = MAX_IP_LEN;

	m_pServerPreviews[iSelect]->GetServerIp(strIPAddr, &nLen1);
	fprintf(stderr, "WARN::####Disconnet server : %s connect success####\n", strIPAddr);
#endif

	return 0;
}

void CAnykaIPCameraDlg::Monitor()
{
	vector<int> vecAlreadyDo;
	BOOL bIsContinue = FALSE;
	BOOL bIsAgain = FALSE;

	while(TRUE) {

		if (!m_runThreadFlag) break;

		if (!m_bNeedJudgeDisConnWork){
			Sleep(1000);//1 second
			continue;
		}

		vecAlreadyDo.clear();

		for (int i = 0; i < PREVIEW_WINDOWS; ++i) { //断线重连
			CAutoLock lock(&m_csForServerConnect);

			bIsAgain = FALSE;

			if (m_nVideoFullScreenIndex != -1) { //如果是全屏模式，只判断正在全屏播放的Server是否断开
				if (m_pServerPreviews[m_nVideoFullScreenIndex] && 
					m_pServerPreviews[m_nVideoFullScreenIndex]->IsDisConnect()) {
#ifdef WARN_ERROR_OUT					
						char strIPAddr[MAX_IP_LEN] = {0};
						unsigned int nLen = MAX_IP_LEN;

						m_pServerPreviews[m_nVideoFullScreenIndex]->GetServerIp(strIPAddr, &nLen);
						fprintf(stderr, "WARN::####Disconnet server : %s start reconnect####\n", strIPAddr);
#endif
						ServerDisConnect(m_nVideoFullScreenIndex);
				}

				break;
			}

			bIsContinue = FALSE;

			for (unsigned int k = 0; k < vecAlreadyDo.size(); ++k) {
				if (i == vecAlreadyDo[k]) {
					bIsContinue = TRUE;
					break;
				}
			}

			if (bIsContinue) continue;

			if (m_pServerPreviews[i] && m_pServerPreviews[i]->IsDisConnect()) {

#ifdef WARN_ERROR_OUT
				char strIPAddr[MAX_IP_LEN] = {0};
				unsigned int nLen = MAX_IP_LEN;

				m_pServerPreviews[i]->GetServerIp(strIPAddr, &nLen);
				fprintf(stderr, "WARN::####Disconnet server : %s start reconnect####\n", strIPAddr);
#endif
				for (int j = 0; j < PREVIEW_WINDOWS; ++j) {//同一服务器的不同或相同码流同时多个预览。
					if ((i != j) && (m_pServerPreviews[j] == m_pServerPreviews[i])) {
						if (ServerDisConnect(j) < 0) {
							bIsAgain = TRUE;
							break;
						}

						vecAlreadyDo.push_back(j);
					}
				}

				if (bIsAgain) continue;

				if (ServerDisConnect(i) < 0) continue;
				vecAlreadyDo.push_back(i);
			}
		}

		Sleep(1000);//1 second
	}
}

void CAnykaIPCameraDlg::FullScreenProcess(BOOL bIsFullScreen, int iSelect)
{
#if 0
	CAutoLock lock(&m_csForServerConnect);

	if (iSelect < 0 || iSelect >= PREVIEW_WINDOWS) return;
	if (m_videoRender[iSelect] == NULL) return;

	int iIndex = 0;

	if (bIsFullScreen) {
		if (m_pServerPreviews[iSelect]->IsDisConnect()) return;

		int ret = m_videoRender[iSelect]->FullScreen(TRUE, OnFullScreenMessage, (void *)(this));
		if (ret < 0) return;

		m_nVideoFullScreenIndex = iSelect;
		for (; iIndex < PREVIEW_WINDOWS; ++iIndex) {
			if ((iIndex != iSelect) && m_pServerPreviews[iIndex]) {
				CloseTheStream(iIndex, TRUE);
			}
		}
	}else{
		for (; iIndex < PREVIEW_WINDOWS; ++iIndex) {
			if (iIndex != iSelect && m_pServerPreviews[iIndex]){ 
				if (!m_strURL[iIndex].empty()) {
					if (OpenTheStream(iIndex, m_strURL[iIndex].c_str(), FALSE) < 0) break;
				}

				if (iIndex == m_nAudioClientIndex) { //此Client是全屏前播放音频的Client
					m_nRBChoosePrevIndex = m_nAudioClientIndex;
					OnPreviewdlgchooseTalkOpen();
				}
			}
		}

		m_nVideoFullScreenIndex = -1;
		m_videoRender[iSelect]->FullScreen(FALSE, NULL, NULL);

	}
#endif
}

void CAnykaIPCameraDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	#if 0
	CRect rect;

	int iSelect = 0;
	for (; iSelect < PREVIEW_WINDOWS; ++iSelect) {
		m_Preview[iSelect].GetWindowRect(&rect);
		ScreenToClient(&rect);

		if (rect.PtInRect(point)) break;
	}

	FullScreenProcess(TRUE, iSelect);

	CDialog::OnLButtonDblClk(nFlags, point);
	#endif
}

void CAnykaIPCameraDlg::CameraMovement(CAMERAMOVEMENT movement)
{
	if (!CanDoTheJob()) return;

	IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(m_hCurrentSelectItem);
	if (pIServer == NULL) {
		AfxMessageBox(L"Can't get the server from tree list.\n");
		return;
	}

	pIServer->SendCameraMovement(movement);
}

void CAnykaIPCameraDlg::OnBnClickedButtonLeft()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsLongPressDone) {
		m_bIsLongPressDone = FALSE;
		return;
	}

	CameraMovement(CMT_STEP_LEFT);
}

void CAnykaIPCameraDlg::OnBnClickedButtonUp()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsLongPressDone) {
		m_bIsLongPressDone = FALSE;
		return;
	}

	CameraMovement(CMT_STEP_UP);
}

void CAnykaIPCameraDlg::OnBnClickedButtonRight()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsLongPressDone) {
		m_bIsLongPressDone = FALSE;
		return;
	}

	CameraMovement(CMT_STEP_RIGHT);
}

void CAnykaIPCameraDlg::OnBnClickedButtonDown()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsLongPressDone) {
		m_bIsLongPressDone = FALSE;
		return;
	}

	CameraMovement(CMT_STEP_DOWN);
}

void CAnykaIPCameraDlg::OnBnClickedButtonLeftRight()
{
	// TODO: 在此添加控件通知处理程序代码
	CameraMovement(CMT_RUN_LEFT_RIGHT);
}

void CAnykaIPCameraDlg::OnBnClickedButtonUpDown()
{
	// TODO: 在此添加控件通知处理程序代码
	CameraMovement(CMT_RUN_UP_DOWN);
}

void CAnykaIPCameraDlg::OnBnClickedButtonRepositionSet()
{
	// TODO: 在此添加控件通知处理程序代码
	CameraMovement(CMT_SET_REPOSITION);
}

void CAnykaIPCameraDlg::OnBnClickedButtonReposition()
{
	// TODO: 在此添加控件通知处理程序代码
	CameraMovement(CMT_RUN_REPOSITION);
}

void CAnykaIPCameraDlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialog::OnRButtonDblClk(nFlags, point);
}

void CAnykaIPCameraDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rect;
	int iSelect = 0;
	CMenu * pm = NULL;

	for (; iSelect < PREVIEW_WINDOWS; ++iSelect) {
		m_Preview[iSelect].GetWindowRect(&rect);
		ScreenToClient(&rect);

		if (rect.PtInRect(point)) break;
	}

	if ((iSelect >= PREVIEW_WINDOWS) || (NULL == m_pServerPreviews[iSelect])) goto end;

	m_nRBChoosePrevIndex = iSelect;

#ifdef UNATTACHED_TALK
	if (m_NetTalk.GetTalkServer() == m_pServerPreviews[iSelect])
		pm = m_menuTalk.GetSubMenu(1);
	else
		pm = m_menuTalk.GetSubMenu(0);
#else
	if (m_NetTalk.IsTalk() && (m_NetTalk.GetTalkServer() == m_pServerPreviews[iSelect]))
		pm = m_menuTalk.GetSubMenu(1);
	else
		pm = m_menuTalk.GetSubMenu(0);
#endif

	ClientToScreen(&point);
	pm->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
	ScreenToClient(&point);

end:
	CDialog::OnRButtonDown(nFlags, point);
}

void CAnykaIPCameraDlg::OnPreviewdlgchooseTalkOpen()
{
	// TODO: 在此添加命令处理程序代码
	USES_CONVERSION;
	WCHAR astrMsg[100] = {0};

	unsigned int nLen = MAX_ID_LEN;
	char strServerID[MAX_ID_LEN] = {0};

	CAutoLock lock(&m_csForTalkOpen);

	m_nAudioClientIndex = -1;

	m_pServerPreviews[m_nRBChoosePrevIndex]->GetServerID(strServerID, &nLen);

	char strIPAddr[MAX_IP_LEN] = {0};
	nLen = MAX_IP_LEN;

	m_pServerPreviews[m_nRBChoosePrevIndex]->GetServerIp(strIPAddr, &nLen);

	if (OnAudioInOpen(m_nRBChoosePrevIndex) < 0){
		_sntprintf(astrMsg, 100, L"Can't listen audio from %s server, IP = %s!\n", A2W(strServerID), A2W(strIPAddr));
		AfxMessageBox( astrMsg, 0, 0 );
		return;
	}

	if (m_NetTalk.IsTalk()) m_NetTalk.StopTalk();

	int ret = m_NetTalk.Talk(m_pServerPreviews[m_nRBChoosePrevIndex], OnTalkKickOut, this);
#ifdef UNATTACHED_TALK
#else
	if (ret < 0) {
		_sntprintf(astrMsg, 100, L"Can't talk to %s server, IP = %s!\n", A2W(strServerID), A2W(strIPAddr));
		OnAudioInClose(m_nRBChoosePrevIndex);
		AfxMessageBox(astrMsg, 0, 0);
		return;
	}
#endif

	m_nAudioClientIndex = m_nRBChoosePrevIndex;
}

void CAnykaIPCameraDlg::TempCloseTalk(int iIndex)
{
	OnAudioInClose(iIndex);
	m_NetTalk.StopTalk();
}

void CAnykaIPCameraDlg::OnPreviewdlgchooseTalkClose()
{
	// TODO: 在此添加命令处理程序代码
	TempCloseTalk(m_nAudioClientIndex);
	m_nAudioClientIndex = -1;
}

LRESULT CAnykaIPCameraDlg::OnTalkKickOutMessage(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;

	if (lParam == NULL) return 0;

	IServer * pTalkServer = (IServer *)lParam;

	if ((m_nAudioClientIndex == -1) || (pTalkServer != m_NetTalk.GetTalkServer())) return 0;

	unsigned long ulSendAudioAddr = 0;
	unsigned short usPort = 0;
	m_NetTalk.GetSendAudioSocketIp(ulSendAudioAddr, usPort);

	if ((ulSendAudioAddr != 0) && (usPort != 0)) {
		CAutoLock lock(&m_csForKickOut);
		if ((ulSendAudioAddr != m_stKickOutParam.ulIpAddr) || ((unsigned long)usPort != m_stKickOutParam.ulPort))
			return 0;
	}

	unsigned int nLen = MAX_ID_LEN;
	char strServerID[MAX_ID_LEN] = {0};
	m_pServerPreviews[m_nAudioClientIndex]->GetServerID(strServerID, &nLen);

	char strIPAddr[MAX_IP_LEN] = {0};
	nLen = MAX_IP_LEN;
	m_pServerPreviews[m_nAudioClientIndex]->GetServerIp(strIPAddr, &nLen);

	OnAudioInClose(m_nAudioClientIndex);
	m_NetTalk.StopTalk(FALSE);
	m_nAudioClientIndex = -1;

	WCHAR astrMsg[1024] = {0};
	_sntprintf(astrMsg, 100, L"其他客户端和服务器:[%s(IP=%s)]，建立了双向对讲连接，本客户端的对讲连接被服务器踢出！\n", A2W(strServerID), A2W(strIPAddr));
	AfxMessageBox(astrMsg, 0, 0);

	return 0;
}

void CAnykaIPCameraDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CAnykaIPCameraDlg::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CDialog::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CAnykaIPCameraDlg::RepositionWidget()
{
	if (!m_bIsInit) return;

	//RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	//Status bar
	//CRect rect;
	//GetWindowRect(&rect);

	//m_StatusBar.SetPaneInfo(0, ID_STATUSBAR_PROGRAMINFO,  0, rect.Width() * SBAR_PROGRAMINFO_SCALE);
	//m_StatusBar.SetPaneInfo(1, ID_STATUSBAR_DISPLAYINFO1, 0, rect.Width() * SBAR_DISPLAYINFO_SCALE);
	//m_StatusBar.SetPaneInfo(2, ID_STATUSBAR_DISPLAYINFO2, 0, rect.Width() * SBAR_DISPLAYINFO_SCALE);

	//Tree List
	//InitTreeCtrlPosition();

	//Preview Windows
	if (g_Full_flag)
	{
		g_Full_flag = FALSE;
	}
	else
	{
		g_Full_flag = TRUE;
	}
	InitPreviewWindows(FALSE, g_Full_flag);
	
	

	//button
	//PositionTheButton();

	//Combo
	//PositionTheImageCombo();
}

void CAnykaIPCameraDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	RepositionWidget();
	// TODO: 在此处添加消息处理程序代码
}

#define BUTTON_WIDTH_APART		1
#define BUTTON_HEIGHT_APART		1

void CAnykaIPCameraDlg::PositionTheButton()
{
	CRect cPreview1Rect, cWindowRect;

	GetWindowRect(&cWindowRect);

	m_Preview[0].GetWindowRect(&cPreview1Rect);

	ScreenToClient(&cPreview1Rect);
	ScreenToClient(&cWindowRect);

	int nWidthMid = (cWindowRect.right - cPreview1Rect.right) / 2;
	int nHeightMid = (cPreview1Rect.bottom - cPreview1Rect.top) / 2;

	CButton * pUpButton =  (CButton *)GetDlgItem(IDC_BUTTON_UP);
	CButton * pLeftButton = (CButton *)GetDlgItem(IDC_BUTTON_LEFT);
	CButton * pRightButton = (CButton *)GetDlgItem(IDC_BUTTON_RIGHT);
	CButton * pDownButton = (CButton *)GetDlgItem(IDC_BUTTON_DOWN);
	CButton * pLRButton =  (CButton *)GetDlgItem(IDC_BUTTON_LEFTRIGHT);
	CButton * pUDButton =  (CButton *)GetDlgItem(IDC_BUTTON_UPDOWN);
	CButton * pSetButton =  (CButton *)GetDlgItem(IDC_BUTTON_REPOSITION_SET);
	CButton * pRepositionButton =  (CButton *)GetDlgItem(IDC_BUTTON_REPOSITION);

	CRect cButtonRect;
	pUpButton->GetWindowRect(&cButtonRect);
	ScreenToClient(&cButtonRect);

	int x = cPreview1Rect.right + (nWidthMid - (cButtonRect.Width() / 2));
	int y = cPreview1Rect.top + (nHeightMid - (cButtonRect.Height() / 2)) - cButtonRect.Height() * 2;
	pUpButton->MoveWindow(x, y, cButtonRect.Width(), cButtonRect.Height());

	pUpButton->GetWindowRect(&cButtonRect);
	ScreenToClient(&cButtonRect);

	x = cButtonRect.left - cButtonRect.Width() + 2 * BUTTON_WIDTH_APART;
	y = cButtonRect.bottom - 2 * BUTTON_HEIGHT_APART;
	pLeftButton->MoveWindow(x, y, cButtonRect.Width(), cButtonRect.Height());

	x = cButtonRect.right - 2 * BUTTON_WIDTH_APART;
	pRightButton->MoveWindow(x, y, cButtonRect.Width(), cButtonRect.Height());

	x = cButtonRect.left;
	y += (cButtonRect.Height() - 2 * BUTTON_WIDTH_APART);
	pDownButton->MoveWindow(x, y, cButtonRect.Width(), cButtonRect.Height());

	pLeftButton->GetWindowRect(&cButtonRect);
	ScreenToClient(&cButtonRect);

	x = cButtonRect.right - cButtonRect.Width() / 2;
	y = cButtonRect.bottom + (2 * BUTTON_HEIGHT_APART) + cButtonRect.Height();
	pLRButton->MoveWindow(x, y, cButtonRect.Width(), cButtonRect.Height());

	pLRButton->GetWindowRect(&cButtonRect);
	ScreenToClient(&cButtonRect);

	x = cButtonRect.right + BUTTON_WIDTH_APART;
	pUDButton->MoveWindow(x, cButtonRect.top, cButtonRect.Width(), cButtonRect.Height());

	pLRButton->GetWindowRect(&cButtonRect);
	ScreenToClient(&cButtonRect);

	x = cButtonRect.left;
	y = cButtonRect.bottom + BUTTON_HEIGHT_APART;
	pSetButton->MoveWindow(x, y, cButtonRect.Width(), cButtonRect.Height());

	pUDButton->GetWindowRect(&cButtonRect);
	ScreenToClient(&cButtonRect);

	x = cButtonRect.left;
	y = cButtonRect.bottom + BUTTON_HEIGHT_APART;
	pRepositionButton->MoveWindow(x, y, cButtonRect.Width(), cButtonRect.Height());
}

#define COMBO_HEIGHT_APART	8
#define COMBO_WIDTH_APART	1

void CAnykaIPCameraDlg::PositionTheImageCombo()
{
	CRect cPreview2Rect, cWindowRect;

	GetWindowRect(&cWindowRect);

	m_Preview[1].GetWindowRect(&cPreview2Rect);

	ScreenToClient(&cPreview2Rect);
	ScreenToClient(&cWindowRect);

	int nWidthMid = (cWindowRect.right - cPreview2Rect.right) / 2;
	int nHeightMid = (cPreview2Rect.bottom - cPreview2Rect.top) / 2;

	CWnd * pStaticText1 = GetDlgItem(IDC_STATIC1);
	CWnd * pStaticText2 = GetDlgItem(IDC_STATIC2);
	CWnd * pStaticText3 = GetDlgItem(IDC_STATIC4);
	CWnd * pStaticText4 = GetDlgItem(IDC_STATIC5);

	CRect cTextRect;
	pStaticText1->GetWindowRect(&cTextRect);
	ScreenToClient(&cTextRect);

	CRect cComboRect;
	m_ContrastCombo.GetWindowRect(&cComboRect);
	ScreenToClient(&cComboRect);

	int x = cPreview2Rect.right + nWidthMid - (cTextRect.Width() +  cComboRect.Width() + COMBO_WIDTH_APART) / 2;
	int y = cPreview2Rect.top + nHeightMid - ((cComboRect.Height() + COMBO_HEIGHT_APART) / 2) - cComboRect.Height() * 2;

	pStaticText1->MoveWindow(x, y + (cComboRect.Height() / 2) - (cTextRect.Height() / 2), cTextRect.Width(), cTextRect.Height());
	m_ContrastCombo.MoveWindow(x + cTextRect.Width() + COMBO_WIDTH_APART, y, cComboRect.Width(), cComboRect.Height());

	pStaticText1->GetWindowRect(&cTextRect);
	ScreenToClient(&cTextRect);

	m_ContrastCombo.GetWindowRect(&cComboRect);
	ScreenToClient(&cComboRect);

	x = cTextRect.left;
	y = cComboRect.bottom + COMBO_HEIGHT_APART + (cComboRect.Height() / 2) - (cTextRect.Height() / 2);
	pStaticText2->MoveWindow(x, y, cTextRect.Width(), cTextRect.Height());
	x = cComboRect.left;
	y = cComboRect.bottom + COMBO_HEIGHT_APART;
	m_SaturationCombo.MoveWindow(x, y, cComboRect.Width(), cComboRect.Height());

	pStaticText2->GetWindowRect(&cTextRect);
	ScreenToClient(&cTextRect);

	m_SaturationCombo.GetWindowRect(&cComboRect);
	ScreenToClient(&cComboRect);

	x = cTextRect.left;
	y = cComboRect.bottom + COMBO_HEIGHT_APART + (cComboRect.Height() / 2) - (cTextRect.Height() / 2);
	pStaticText3->MoveWindow(x, y, cTextRect.Width(), cTextRect.Height());
	x = cComboRect.left;
	y = cComboRect.bottom + COMBO_HEIGHT_APART;
	m_BrightnessCombo.MoveWindow(x, y, cComboRect.Width(), cComboRect.Height());

	pStaticText3->GetWindowRect(&cTextRect);
	ScreenToClient(&cTextRect);

	m_BrightnessCombo.GetWindowRect(&cComboRect);
	ScreenToClient(&cComboRect);

	x = cTextRect.left;
	y = cComboRect.bottom + COMBO_HEIGHT_APART + (cComboRect.Height() / 2) - (cTextRect.Height() / 2);
	pStaticText4->MoveWindow(x, y, cTextRect.Width(), cTextRect.Height());
	x = cComboRect.left;
	y = cComboRect.bottom + COMBO_HEIGHT_APART;
	m_acutanceCom.MoveWindow(x, y, cComboRect.Width(), cComboRect.Height());
}

int CAnykaIPCameraDlg::ShutDownTheStream(int iSelect)
{
	CAutoLock lock(&m_csForServerConnect);
	CloseTheStream(iSelect, TRUE);

	if (iSelect == m_nAudioClientIndex)
		m_nAudioClientIndex = -1;
	return UnregisterThePreviewServer(iSelect);
}

void CAnykaIPCameraDlg::OnPreviewdlgchoose1ClosePreview()
{
	// TODO: 在此添加命令处理程序代码
	ShutDownTheStream(m_nRBChoosePrevIndex);
}

void CAnykaIPCameraDlg::OnPreviewdlgchooseClosePreview()
{
	// TODO: 在此添加命令处理程序代码
	ShutDownTheStream(m_nRBChoosePrevIndex);
}

//程序退出时使用此函数等待Monitor线程结束，防止等待线程的过程中消息到来后不被处理，而造成的程序卡死现象。
void CAnykaIPCameraDlg::WiatForMonitorThreadEnd()
{
	DWORD result;
	MSG msg;
	
	while(TRUE) {
		//MsgWaitForMultipleObjects API等待目标线程结束时，如果等待的线程收到消息，则返回。
		result = MsgWaitForMultipleObjects(1, &m_MonitorThread, FALSE, INFINITE, QS_ALLINPUT);
		if (result == WAIT_OBJECT_0)
			break;//等待的目标线程结束了。
		else {
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE); //等待过程中本线程收到了窗口消息。
			DispatchMessage(&msg);
		}
	}
}

void CAnykaIPCameraDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
#if 0
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd * pButtonLeft = GetDlgItem(IDC_BUTTON_LEFT);
	CWnd * pButtonRight = GetDlgItem(IDC_BUTTON_RIGHT);
	CWnd * pButtonUp = GetDlgItem(IDC_BUTTON_UP);
	CWnd * pButtonDown = GetDlgItem(IDC_BUTTON_DOWN);

	m_nLongPressButtonID = -1;
	m_bIsLongPress = FALSE;

	CRect rect;
	pButtonLeft->GetWindowRect(rect);
	ScreenToClient(rect);
	if (rect.PtInRect(point))
		m_nLongPressButtonID = IDC_BUTTON_LEFT;

	pButtonRight->GetWindowRect(rect);
	ScreenToClient(rect);
	if (rect.PtInRect(point))
		m_nLongPressButtonID = IDC_BUTTON_RIGHT;

	pButtonUp->GetWindowRect(rect);
	ScreenToClient(rect);
	if (rect.PtInRect(point))
		m_nLongPressButtonID = IDC_BUTTON_UP;

	pButtonDown->GetWindowRect(rect);
	ScreenToClient(rect);
	if (rect.PtInRect(point))
		m_nLongPressButtonID = IDC_BUTTON_DOWN;

	if (m_nLongPressButtonID != -1) { //LBUTTONDWON在云台按钮上
		SetTimer(TIMER_LONG_PRESS, 500, NULL);
	}

	CDialog::OnLButtonDown(nFlags, point);
#endif
}

void CAnykaIPCameraDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
#if 0
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_bIsLongPress) {//长按结束，需要发送STOP消息给服务端
		KillTimer(TIMER_LONG_PRESS);

		if (m_nLongPressButtonID == IDC_BUTTON_LEFT) {
			CameraMovement(CMT_STEP_LEFT_CONTINUE_STOP);
			m_bIsLongPressDone = TRUE;
		}else if (m_nLongPressButtonID == IDC_BUTTON_RIGHT) {
			CameraMovement(CMT_STEP_RIGHT_CONTINUE_STOP);
			m_bIsLongPressDone = TRUE;
		}else if (m_nLongPressButtonID == IDC_BUTTON_UP) {
			CameraMovement(CMT_STEP_UP_CONTINUE_STOP);
			m_bIsLongPressDone = TRUE;
		}else if (m_nLongPressButtonID == IDC_BUTTON_DOWN) {
			CameraMovement(CMT_STEP_DOWN_CONTINUE_STOP);
			m_bIsLongPressDone = TRUE;
		}else {
		}
	}

	m_nLongPressButtonID = -1;
	m_bIsLongPress = FALSE;

	CDialog::OnLButtonUp(nFlags, point);
#endif
}

#define PROCESS_MOVE_OUT(x) \
{\
	KillTimer(TIMER_LONG_PRESS);\
	CameraMovement((x));\
	m_bIsLongPressDone = TRUE;\
	m_nLongPressButtonID = -1;\
	m_bIsLongPress = FALSE;\
}

void CAnykaIPCameraDlg::OnMouseMove(UINT nFlags, CPoint point)
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

//处理服务器端返回的信息。
LRESULT CAnykaIPCameraDlg::OnServerRetInfo(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;

	if (lParam == NULL) return 0;

	IServer * pRetInfoServer = (IServer *)lParam;

	int i = 0;
	for (; i < PREVIEW_WINDOWS; ++i) {
		if (pRetInfoServer == m_pServerPreviews[i]) break;
	}

	if (i >= PREVIEW_WINDOWS) //this return info server current no preview 
		return 0;

	unsigned int nLen = MAX_ID_LEN;
	char strServerID[MAX_ID_LEN] = {0};
	pRetInfoServer->GetServerID(strServerID, &nLen);

	char strIPAddr[MAX_IP_LEN] = {0};
	nLen = MAX_IP_LEN;
	pRetInfoServer->GetServerIp(strIPAddr, &nLen);

	CString strInfo, strServerInfo;
	strInfo.Format(L"服务器%s(%s)返回:");

	RETINFO stInfo = {0};

	{
		CAutoLock lock(&m_csForRet);
		memcpy(&stInfo, &m_stRetInfo, sizeof(RETINFO));
	}

	GetStringFromRetInfo(m_stRetInfo, strServerInfo);

	if (strServerInfo.GetLength() <= 0)
		return 0;

	strInfo.Append(strServerInfo);

	AfxMessageBox(strServerInfo);

	return 0;
}


void CAnykaIPCameraDlg::OnCbnSelchangeCombo4()
{
	// TODO: 在此添加控件通知处理程序代码
	MakeAndSendImageSet();
}

void CAnykaIPCameraDlg::OnBnClickedRadioIrcutOn()
{
	// TODO: 在此添加控件通知处理程序代码
	m_ircut_flag = 1;
}

void CAnykaIPCameraDlg::OnBnClickedRadioIrcutOff()
{
	// TODO: 在此添加控件通知处理程序代码
	m_ircut_flag = 0;
}


BOOL CAnykaIPCameraDlg::Connet_FTPServer(LPCTSTR addr, UINT idex) 
{

	if(m_pInetSession != NULL)
	{
		delete m_pInetSession;
		m_pInetSession = NULL;
	}
	m_pInetSession = new CInternetSession(AfxGetAppName(), 1, PRE_CONFIG_INTERNET_ACCESS);
	try
	{
		// addr       ftp服务器的地址  LPCTSTR
		// username   登陆用户名       LPCTSTR 
		// password   密码            LPCTSTR
		// port       端口            UINT
		if(m_pFtpConnection != NULL)
		{
			m_pFtpConnection->Close();
			delete m_pFtpConnection;
			m_pFtpConnection = NULL;
		}
		m_pFtpConnection = m_pInetSession->GetFtpConnection(addr, m_username, m_password, m_uPort);
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
		m_pFtpConnection = NULL;
		return FALSE;
	}

	return TRUE;
}


BOOL CAnykaIPCameraDlg::ConnetServer(LPCTSTR addr, UINT idex) 
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
		//连接ftp服务器
		//连接ftp服务器
		if (!Connet_FTPServer(addr, idex))
		{
			AfxMessageBox(_T("Connet_FTPServer fail"));
			return FALSE;
		}

		if (!create_thread_rev_data(idex))
		{
			AfxMessageBox(_T("创建接收数据线程失败"));
			return FALSE;
		}
		g_senddata_flag = FALSE;
		
	}
	else
	{
		AfxMessageBox(_T("Socket_Connect fail"));
		return FALSE;
	}

	return TRUE;
}


BOOL CAnykaIPCameraDlg::Anyka_Test_check_info(void)
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
		if (time2 - time1 > 5000)
		{
			g_test_pass_flag = 0;
			AfxMessageBox(_T("超时(5s)没有返回确认命令"));  
			return FALSE;
		}
		if (g_test_pass_flag == 1)
		{

			g_test_pass_flag = 0;
			return TRUE;
		}
		else if (g_test_pass_flag == 2)
		{
			g_test_pass_flag = 0;
			return FALSE;
		}
	}

}

BOOL CAnykaIPCameraDlg::find_file_indir(TCHAR *file_name, UINT *name_len) 
{

	CFileFind ff;
	CString filename;
	CString szDir;
	DWORD  len = 0;

	szDir.Format(_T("%s/*"), ConvertAbsolutePath(TEST_CONFIG_DIR));
	BOOL res = ff.FindFile(szDir);
	while( res )
	{
		res = ff.FindNextFile();
		filename = ff.GetFileName();
		len = ff.GetLength();

		if(!ff.IsDirectory() && !ff.IsDots())
		{
			_tcscpy(file_name, filename);
			*name_len = len;
			ff.Close(); 
			return TRUE;
		}
	}
	*name_len = len;
	ff.Close(); 
	return FALSE;
}


BOOL CAnykaIPCameraDlg::OnSend_data()
{
	CString strSourceName, strDestName;
	TCHAR test_Name[MAX_PATH] = {0};
	TCHAR DestName_temp[MAX_PATH] = {0};
	TCHAR DestName[50] =_T("/tmp/");
	UINT name_len = 0;
	char name_buf[MAX_PATH] = {0};
	char param_buf[2] = {0};
	CString str;
	TCHAR *file_name = _T("test_ircut");

	USES_CONVERSION;

	//if (!find_file_indir(file_name,&name_len))
	//{
	//	AfxMessageBox(_T("没有找到任何文件，请重启小机"), MB_OK);
	//	return FALSE;
	//}


	memset(test_Name, 0,  MAX_PATH);
	_tcsncpy(test_Name, TEST_CONFIG_DIR, sizeof(TEST_CONFIG_DIR));
	_tcscat(test_Name, _T("//"));
	_tcscat(test_Name, file_name);

	//判断文件是否存在
	if(0xFFFFFFFF == GetFileAttributes(ConvertAbsolutePath(test_Name)))
	{
		str.Format(_T("%s no exist,请重启小机"), test_Name);
		AfxMessageBox(str, MB_OK); 
		return FALSE;
	}

	//发送文件
	memset(DestName_temp, 0, MAX_PATH);
	_tcsncpy(DestName_temp, DestName, sizeof(DestName));
	strSourceName.Format(_T("%s"), ConvertAbsolutePath(test_Name));
	_tcscat(DestName_temp, file_name);
	strDestName.Format(_T("%s"), DestName_temp);

	if (!download_file_flag)
	{
		if (!m_pFtpConnection->PutFile(strSourceName, strDestName, FTP_TRANSFER_TYPE_BINARY, 1))   
		{
			AfxMessageBox(_T("Error no auto test putting file,请重启小机"), MB_OK);  
			download_file_flag = FALSE;
			return FALSE;
		}
		download_file_flag = TRUE;
	}

	//发送命令
	name_len = strlen(T2A(file_name));
	memset(name_buf, 0, MAX_PATH);
	memcpy(name_buf, T2A(file_name), name_len);

	g_test_pass_flag = 0;
	
	memset(param_buf, 0, 2);

	if (m_ircut_flag)
	{
		memcpy(param_buf, "1", 1);
	}
	else
	{
		memcpy(param_buf, "2", 1);
	}
	

	if (!Send_cmd(TEST_COMMAND, 1, name_buf, param_buf))
	{
		AfxMessageBox(_T("Send_cmd　fail "), MB_OK); 
		return FALSE;
	}

	//接收返回值
	if (!Anyka_Test_check_info())
	{
		AfxMessageBox(_T("设置失败"), MB_OK);
		return FALSE;
	}
	g_test_pass_flag = 0;
	MessageBox(_T("设置成功"), MB_OK);
	return TRUE;
}

void CAnykaIPCameraDlg::CloseServer(void) 
{

	if(m_pInetSession != NULL)
	{
		delete m_pInetSession;
		m_pInetSession = NULL;
	}

	if(m_pFtpConnection != NULL)
	{
		m_pFtpConnection->Close();
		delete m_pFtpConnection;
		m_pFtpConnection = NULL;
	}
	g_send_commad = 0;
	g_test_fail_flag  = 0;
	download_file_flag = FALSE;
	download_dev_file_flag = FALSE;

	close_thread_rev_data();
	m_ClientSocket.Socket_Close(0);

}

void CAnykaIPCameraDlg::OnBnClickedButtonSet()
{
	// TODO: 在此添加控件通知处理程序代码
	//首先进行连接
	

	if (!g_connet_flag)
	{
		AfxMessageBox(_T("网络没有连接上，请检查"), MB_OK);
		return;
	}
	g_send_commad = 1;
	g_senddata_flag = TRUE;


	//发送数据下去
	if (!OnSend_data())
	{
		g_senddata_flag = FALSE;
		g_send_commad = 0;
		return;
	}
	g_senddata_flag = FALSE;
	g_send_commad = 0;

	
}

BOOL CAnykaIPCameraDlg::decode_command(char *lpBuf, char *commad_type, char *file_name, char *param)
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

DWORD WINAPI check_rev_date_thread(LPVOID lpParameter)
{
	char commad_type; 
	char *file_name = NULL;
	char *param = NULL;
	CAnykaIPCameraDlg  TestToolDlg;
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
		if (g_hBurnThread_rev_data != INVALID_HANDLE_VALUE)
		{
			if (g_send_commad == 1)
			{
				commad_type = 0;  //初始化
				//time1 = GetTickCount();
				ret = m_ClientSocket_heat.Socket_Receive(lpBuf, nBufLen, idex);
				//frmLogfile_temp.WriteLogFile(0,"Socket_Receive ret:%d\n", ret);
				if (ret == -1)
				{
					if (g_hBurnThread_rev_data== INVALID_HANDLE_VALUE && g_send_commad== 0)
					{
						return TRUE;
					}
					g_test_fail_flag  = 1;
					g_test_pass_flag = 2;
					//g_connet_success_flag = FALSE;
					AfxMessageBox(_T("接收数据错误，请重启小机再连接"));
					if (g_hBurnThread_rev_data != INVALID_HANDLE_VALUE)
					{
						CloseHandle(g_hBurnThread_rev_data);
						g_hBurnThread_rev_data = INVALID_HANDLE_VALUE;
					}
					g_send_commad = 0;
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
						g_test_pass_flag = 2;	
					}
					else
					{
						if (g_param[0] == 49)  //49 表示1
						{
							g_test_pass_flag = 1;
						}
						else
						{
							g_test_pass_flag = 2;
						}
					}
				}
			}
		}
	}

	return 1;
}


BOOL CAnykaIPCameraDlg::create_thread_rev_data(UINT idex) 
{


	if (g_hBurnThread_rev_data != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_rev_data);
		g_hBurnThread_rev_data = INVALID_HANDLE_VALUE;
	}

	//param = &idex;
	g_hBurnThread_rev_data = CreateThread(NULL, 0, check_rev_date_thread, rve_param, 0, NULL);
	if (g_hBurnThread_rev_data == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;
}


void CAnykaIPCameraDlg::close_thread_rev_data() 
{
	if(g_hBurnThread_rev_data!= INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_rev_data);
		g_hBurnThread_rev_data = INVALID_HANDLE_VALUE;
	}
}

BOOL CAnykaIPCameraDlg::Send_cmd(char commad_type, BOOL auto_test_flag, char *file_name, char *param)
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


	if (file_name != NULL)// && !no_filename_flag)
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

	if (param != NULL)// && !no_data_flag)
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


	ret = m_ClientSocket.Socket_Send(lpBuf, nBufLen, 0);

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




void CAnykaIPCameraDlg::OnBnClickedButtonRecoverDev()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strSourceName, strDestName;
	TCHAR test_Name[MAX_PATH] = {0};
	TCHAR DestName_temp[MAX_PATH] = {0};
	TCHAR DestName[50] =_T("/tmp/");
	UINT name_len = 0;
	char name_buf[MAX_PATH] = {0};
	char param_buf[2] = {0};
	CString str;
	TCHAR *file_name = _T("test_recover_dev");

	USES_CONVERSION;

	//if (!find_file_indir(file_name,&name_len))
	//{
	//	AfxMessageBox(_T("没有找到任何文件，请重启小机"), MB_OK);
	//	return FALSE;
	//}

	if (!g_connet_flag)
	{
		AfxMessageBox(_T("网络没有连接上，请检查"), MB_OK);
		return;
	}
	g_send_commad = 1;

	memset(test_Name, 0,  MAX_PATH);
	_tcsncpy(test_Name, TEST_CONFIG_DIR, sizeof(TEST_CONFIG_DIR));
	_tcscat(test_Name, _T("//"));
	_tcscat(test_Name, file_name);

	//判断文件是否存在
	if(0xFFFFFFFF == GetFileAttributes(ConvertAbsolutePath(test_Name)))
	{
		g_send_commad = 0;
		str.Format(_T("%s no exist,请重启小机"), test_Name);
		AfxMessageBox(str, MB_OK); 
		return;
	}

	//发送文件
	memset(DestName_temp, 0, MAX_PATH);
	_tcsncpy(DestName_temp, DestName, sizeof(DestName));
	strSourceName.Format(_T("%s"), ConvertAbsolutePath(test_Name));
	_tcscat(DestName_temp, file_name);
	strDestName.Format(_T("%s"), DestName_temp);

	if (!download_dev_file_flag)
	{
		if (!m_pFtpConnection->PutFile(strSourceName, strDestName, FTP_TRANSFER_TYPE_BINARY, 1))   
		{
			AfxMessageBox(_T("Error no auto test putting file,请重启小机"), MB_OK);  
			download_dev_file_flag = FALSE;
			g_send_commad = 0;
			return;
		}
		download_dev_file_flag = TRUE;
	}

	//发送命令
	name_len = strlen(T2A(file_name));
	memset(name_buf, 0, MAX_PATH);
	memcpy(name_buf, T2A(file_name), name_len);

	g_test_pass_flag = 0;
	if (!Send_cmd(TEST_COMMAND, 1, name_buf, NULL))
	{
		AfxMessageBox(_T("Send_cmd　fail "), MB_OK); 
		g_send_commad = 0;
		return ;
	}

	//接收返回值
	if (!Anyka_Test_check_info())
	{
		AfxMessageBox(_T("设置失败"), MB_OK);
		g_send_commad = 0;
		return ;
	}
	g_test_pass_flag = 0;

	if (!Send_cmd(TEST_COMMAND_FINISH, 1, NULL, NULL))
	{
		AfxMessageBox(_T("更新成功, 小机自动重启失败，请手动重启"), MB_OK);  
	}
	else
	{
		if (!Anyka_Test_check_info())
		{
			AfxMessageBox(_T("更新成功, 小机自动重启失败，请手动重启"), MB_OK);  
		}
		else
		{
			MessageBox(_T("设置成功"), MB_OK);
		}
	}
	g_send_commad = 0;
}
