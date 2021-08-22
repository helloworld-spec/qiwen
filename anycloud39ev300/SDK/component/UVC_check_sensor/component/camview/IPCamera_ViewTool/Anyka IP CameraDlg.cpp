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
#include "Config_test.h"
#include "logfile.h"
#include "LogUidFile.h"
#include "PcmSpeaker.h"
#include <windows.h>    
#include <stdio.h>    
#include <mmsystem.h>    
#pragma comment(lib, "winmm.lib")  


typedef enum
{
	CASE_VIDEO = 0,	
	CASE_IRCUT_ONOFF,
	//CASE_IRCUT_ON,
	//CASE_IRCUT_OFF,
	CASE_MONITOR,   //
	CASE_INTERPHONE,//对讲
	CASE_HEAD,      //云台
	CASE_RESET,
	CASE_SD,
	CASE_WIFI,
	//CASE_INFRARED,
	CASE_UID,
	CASE_MAC,
	//CASE_RESET,
}E_MEMORY_TYPE;


typedef enum
{
	TYPE_VIDEO_PARM = 1,
	TYPE_VIDEO_START,
	TYPE_VIDEO_STOP,
}E_VIDEO_CMD_TYPE;


enum frame_type 
{
	FRAME_TYPE_P = 0,
	FRAME_TYPE_I,
	FRAME_TYPE_AUDIO,
	FRAME_TYPE_PICTURE,
};

enum data_type 
{
	DATA_TYPE_VIDEO = 0,
	DATA_TYPE_AUDIO,
	DATA_TYPE_PICTURE,
};



typedef enum
{
	IDS_TEST_SUCCESS,
	IDS_TEST_FAIL,
}E_RESULT_TYPE;


typedef struct
{
	UINT thread_idex;		
	UINT IP_idex;
}T_IDEX_INFO;


typedef struct
{
	BYTE *m_video_receive_buf;
	UINT  m_receive_buf_len;
	BOOL  m_empty_flag;
}T_SHOW_VIDE_INFO;





#define      HOTKEY_F1   1001
#define      HOTKEY_F2   1002
#define      HOTKEY_F3   1003
#define      HOTKEY_F4   1004
#define      HOTKEY_F6   1006
#define      HOTKEY_F7   1007
#define      HOTKEY_F8   1008
#define      HOTKEY_FY   1009
#define      HOTKEY_Fy   1010
#define      HOTKEY_F11  1011
#define      HOTKEY_FESCAPE   1012
#define      HOTKEY_FNULL   1013


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


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TRANS_STACKSIZE		(1024*300)


#define  TOOL_VERSOIN _T("camview_v1.0.11")


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

#define   VIDEO_RECEIVE_BUFFER_SIZE   720 * 576 / 2
#define   MAX_DECONDE_BUF_NUM   40
#define   MAX_AUDIO_BUF_NUM   40
//BYTE *g_video_receive_buf[MAX_DECONDE_BUF_NUM] = NULL;
//UINT   g_receive_buf_len[MAX_DECONDE_BUF_NUM] = {0};
CFile g_file_fp;
CFile g_file_fp_test;
BOOL g_enter_flag = TRUE;
BOOL g_malloc_buf_success = FALSE;
T_SHOW_VIDE_INFO g_show_video[MAX_DECONDE_BUF_NUM] = {0};
char g_read_current_buf_idex = 0;
char g_write_current_buf_idex = 0;
UINT g_frame_num = 0;
BOOL g_recode_video = FALSE;
BOOL g_server_flag = FALSE;  //false 表示服务端， true表示客户端

UINT g_video_file_len = 0;
BOOL open_video_success = FALSE;

BYTE *g_one_video_receive_buf = NULL;
UINT g_one_video_receive_buf_len = 0;

//图像
#define  MAX_PICTRUEN_LEN  1280*960*3/2
char g_picture_buf[MAX_PICTRUEN_LEN] = {0}; //定义一张最大的
UINT g_picture_len = 0;
BOOL g_get_picture_flag = FALSE;
BOOL g_show_picture_flag = TRUE;
BOOL g_check_picture_flag = TRUE;


//音频
#define   AUDIO_RECEIVE_BUFFER_SIZE   2048
T_SHOW_VIDE_INFO g_play_data[MAX_AUDIO_BUF_NUM] = {0};
BYTE *g_one_audio_receive_buf = NULL;
UINT g_one_audio_receive_buf_len = 0;
UINT g_audio_read_current_buf_idex = 0;
UINT g_audio_write_current_buf_idex = 0;
UINT g_time_seconds = 0;


BOOL g_start_recode_flag =FALSE;
BOOL g_save_flag = FALSE;

static HWAVEOUT dev				= NULL;
static CRITICAL_SECTION			cs;


extern HINSTANCE _hInstance;

char g_update_flag[UPDATE_MAX_NUM] = {0};
BOOL g_uid_running = FALSE;
BOOL g_start_running = FALSE;
BOOL g_next_running = FALSE;
BOOL g_reset_running = FALSE;
extern BOOL start_test_flag;
BOOL g_start_success_flag = FALSE;
BOOL g_creat_recode_video = TRUE;

BOOL g_start_open_flag = TRUE;
BOOL g_test_finish_endrtsp_flag = false;
HANDLE g_hTestThread = INVALID_HANDLE_VALUE;
HANDLE g_getvideo_Thread = INVALID_HANDLE_VALUE;
HANDLE g_decodevideo_Thread = INVALID_HANDLE_VALUE;
HANDLE g_hBurnThread_server = INVALID_HANDLE_VALUE;
HANDLE g_hBurnThread_play_video = INVALID_HANDLE_VALUE;
HANDLE g_hBurnThread_play_first_frame = INVALID_HANDLE_VALUE;
HANDLE g_hBurnThread_record_wav = INVALID_HANDLE_VALUE;
HANDLE g_hBurnThread_play_wav = INVALID_HANDLE_VALUE;


HANDLE g_findIP_Thread = INVALID_HANDLE_VALUE;
HANDLE g_test_wait_data = INVALID_HANDLE_VALUE;
BOOL g_first_find_flag = TRUE;
BOOL g_config_start_flag = TRUE;
BOOL g_reset_test_finish = False;
HANDLE         g_wait;

HWND g_hWnd = 0;
BOOL g_Full_flag = TRUE;
TCHAR ip_address[AP_ADDRESS_LEN] = {0};
CInternetSession *m_pInetSession[UPDATE_MAX_NUM] = {NULL};
CFtpConnection *m_pFtpConnection[UPDATE_MAX_NUM] = {NULL}; 
UINT m_uPort = 0;
UINT m_net_uPort = 0;
//HANDLE g_hBurnThread_rev_data= INVALID_HANDLE_VALUE;
HANDLE g_hBurnThread_play_pcm= INVALID_HANDLE_VALUE;
HANDLE m_pcm_play_handle = INVALID_HANDLE_VALUE;
HANDLE m_download_handle = INVALID_HANDLE_VALUE;
char g_download_sd_wifi_flag = 0;
BOOL g_monitor_end_flag = TRUE;
BOOL play_pcm_finish_flag = FALSE;
//UINT rve_param[2] = {0};
//char g_send_commad[UPDATE_MAX_NUM] = {0};
char g_test_fail_flag[UPDATE_MAX_NUM] = {0};
char g_test_pass_flag[UPDATE_MAX_NUM] = {0};  //0正在测试中， //1测试成功， 2测试失败
char g_commad_type;
BOOL g_connet_flag = FALSE;
BOOL g_download_ptz_misc = TRUE;
BOOL download_file_flag = FALSE;
BOOL download_dev_file_flag = FALSE;
BOOL g_senddata_flag = TRUE;
BOOL entern_flag = FALSE;
BOOL login_entern_flag = FALSE;
//HANDLE g_heatThread= INVALID_HANDLE_VALUE;
HANDLE m_test_monitor_handle = INVALID_HANDLE_VALUE;
BOOL g_sd_test_success = FALSE;
BOOL g_test_monitor_flag = FALSE;
BOOL g_close_monitor = False;
HANDLE m_retest_sd_handle = INVALID_HANDLE_VALUE;
HANDLE m_retest_wifi_handle = INVALID_HANDLE_VALUE;

BOOL config_uid_enable_temp=FALSE;
BOOL config_lan_mac_enable_temp=FALSE;
BOOL g_finish_find_flag = TRUE;

BOOL g_sd_test_pass = TRUE;
BOOL g_wifi_test_pass = TRUE;
BOOL g_reset_test_pass = TRUE;

BOOL m_not_find_anyIP = FALSE;
BOOL m_find_anyIP = FALSE;
BOOL m_find_IP_end_flag = FALSE;
UINT m_ip_address_idex = 0;
BOOL g_sousuo_flag = FALSE;
CConfig_test g_test_config;
BOOL M_bConn;
//TCHAR m_connect_ip[MAX_PATH+1] = {0};
//TCHAR m_connect_uid[MAC_ADDRESS_LEN] = {0};
//TCHAR g_download_filename[MAX_PATH] = {0};
CString g_src_filename;
CString g_dst_filename;
UINT g_case_idex;
CLogFile  frmLogfile;
CLogUidFile frmLogUidFile;
UINT current_ip_idex = 0;

BOOL end_test=false;

BOOL g_move_test_connect_flag = FALSE;
BOOL first_flag = FALSE;
BOOL init_flag=FALSE;
BOOL no_put_flie_flag = FALSE;
BOOL g_test_finish_flag = FALSE;
BOOL g_pre_flag = FALSE;

long g_sd_size = 0;
T_SSID_INFO *g_ssid_info = NULL;
UINT g_ssid_num = 0;


HANDLE g_hupdateThread = INVALID_HANDLE_VALUE;
HANDLE g_heatThread[UPDATE_MAX_NUM] = {INVALID_HANDLE_VALUE};
HANDLE g_hBurnThread_rev_data[UPDATE_MAX_NUM] = {INVALID_HANDLE_VALUE};
HANDLE g_hBurnThread_save_data[UPDATE_MAX_NUM] = {INVALID_HANDLE_VALUE};

HANDLE g_check_MAC_Thread = INVALID_HANDLE_VALUE;
//char  m_update_flag[UPDATE_MAX_NUM] = {0};
//T_IDEX_INFO g_param[UPDATE_MAX_NUM] = {0};
UINT g_updateing_num = 0; 
HANDLE g_handle = INVALID_HANDLE_VALUE;   //信号量
BOOL g_finish_flag  = FALSE;
UINT g_finish_idex  = 0;

UINT rve_param[UPDATE_MAX_NUM][2] = {0};
//UINT heat_param[UPDATE_MAX_NUM][2] = {0};
//UINT update_param[UPDATE_MAX_NUM][2] = {0};
BOOL one_update_finish = FALSE;
UINT g_time3 = 0;
T_IMAGE_INFO cut_out_on = {0};

UINT g_current_frame_idex = 0;
UINT g_max_frame_idex = 0;
BOOL g_stop_play_flag = FALSE;
BOOL g_start_play_flag = FALSE;
BOOL g_stop_one_play_flag = FALSE;
LONGLONG g_current_Off = 0;
PICYUVDATA *g_pre_pstYuvData = NULL;
extern BOOL g_need_re_open_flag;
BOOL g_play_first_frame_flag = FALSE;

extern BOOL g_open_tool;

//#define BUFFER_SIZE (44100*16*2/8*5*5)    // 录制声音长度  
//static unsigned char buffer[BUFFER_SIZE] = {0};   
//UINT g_buf_count = 0; 

#define FRAGMENT_SIZE 512              // 缓存区大小  
#define FRAGMENT_NUM 8                  // 缓存区个数 
HWAVEIN g_hWaveIn;//波形音频数据格式Wave_audio数据格式  
WAVEFORMATEX g_wavform;//WAVEFORMATEX结构定义了波形音频数据格式。包括在这个结构中唯一的格式信息，共同所有波形音频数据格式。对于需要额外的信息的格式，这个结构包含在另一个结构的第一个成员，以及与其他信息    
WAVEHDR g_wh[FRAGMENT_NUM];  
HWAVEOUT g_hWaveOut;//打开回放设备函数　  
WAVEHDR g_wavhdr;
BOOL  g_play_finish = FALSE;

#define CONFIG_PATH 	L"config.txt"
//#define  TEST_CONFIG_DIR           _T("test_config")

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框


extern DWORD WINAPI tcp_server_thread(LPVOID lpParameter);

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
	ON_WM_HOTKEY()
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

	//m_Preview[0].MoveWindow( 0,0, 600, 500);
	CRect rcDlg1,rcDlg2,rcDlg3;
	GetClientRect(&rcDlg1);
	m_RightDlg.GetClientRect(&rcDlg2);
	//m_BottomDlg.GetClientRect(&rcDlg3);

#if 1
	m_Preview[0].SetWindowPos(NULL,0,0,rcDlg1.Width()-rcDlg2.Width()-10,
		rcDlg1.Height()-10,SWP_NOMOVE|SWP_SHOWWINDOW);
	m_Preview[1].SetWindowPos(NULL,0,0,rcDlg1.Width()-rcDlg2.Width()-10,
		rcDlg1.Height()-10,SWP_NOMOVE|SWP_SHOWWINDOW);
#else
	m_Preview[0].MoveWindow(rcDlg2.Width(), rcDlg3.Height(), rcDlg1.Width()-rcDlg2.Width()-10, rcDlg1.Height()-rcDlg3.Height()-10 );
	m_Preview[0].SetWindowPos(NULL,rcDlg2.Width(),rcDlg3.Height(),rcDlg1.Width()-rcDlg2.Width()-10,
		rcDlg1.Height()-rcDlg3.Height()-10,SWP_NOMOVE|SWP_SHOWWINDOW);
	m_Preview[1].SetWindowPos(NULL,rcDlg2.Width(),rcDlg3.Height(),rcDlg1.Width()-rcDlg2.Width()-10,
		rcDlg1.Height()-rcDlg3.Height()-10,SWP_NOMOVE|SWP_SHOWWINDOW);
		
#endif


	//m_Preview[0].MoveWindow( cWindowRect.top+3, cWindowRect.left+3, cWindowRect.right, cWindowRect.bottom);

	//m_Preview[1].MoveWindow( cTreeCtrlRect.right + 3, cToolBarRect.bottom + 3 + cTreeCtrlRect.Height() / 2, ( cTreeCtrlRect.Height() * 8 ) / 9, cTreeCtrlRect.Height() / 2 );

	if (bNeedCreate)
	{
		m_Preview[0].ShowWindow( SW_SHOW );
		m_Preview[1].ShowWindow( SW_SHOW );
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
	fprintf(stderr, 
		"WARN::####Disconnet we will do the reconnect operate, because we didn't rec any video data in last 4 second####\n");
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
	g_start_open_flag = TRUE;
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

void CAnykaIPCameraDlg::OnTalkKickOut(IServer * pIServer,
									  unsigned long ulIpAddr, unsigned short usPort, void * pClassParam)
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


BOOL CAnykaIPCameraDlg::get_system_info()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (GetVersionEx((OSVERSIONINFO *) &os))
	{
		//CString str;
		//str.Format(_T("ma:%d, ni:%d"), os.dwMajorVersion,os.dwMinorVersion);
		//AfxMessageBox(str, MB_OK);

		if (os.dwMajorVersion == 6 && os.dwMinorVersion == 2)
		{
			g_test_config.win10_flag = TRUE;//WIN10
		}
		else
		{
			g_test_config.win10_flag = FALSE;//非WIN10
		}
	}

	return TRUE;
}

BOOL CAnykaIPCameraDlg::OnInitDialog()
{
	CString sPath;
	TCHAR temp_buf_det[MAX_PATH] = {0};
	TCHAR temp_buf_src[MAX_PATH] = {0};
	TCHAR bufsprintf[MAX_PATH] = {0};
	char buf[2] = {0};
	TCHAR fram_num[MAX_PATH] = {0};
	
	//TCHAR *test_pcm_name = _T("pcm_test_c.pcm");
	CString str;
	//HANDLE hFile ;
	//BOOL bRet;
	int nPos;
	UINT i = 0;

	//Onopen_pcm_file();

	//enter_show.DoModal();

	USES_CONVERSION;
#if 0
	{
		char buf_temp[4096] = {0};
		UINT len = 0;

		if (!g_file_fp_test.Open(TEXT("F:\\first_test.dat"),CFile::modeReadWrite))
		{
			AfxMessageBox(_T("error: open the data stream fail"), MB_OK);
			return FALSE;
		}
		g_file_fp_test.Read(buf_temp, 4);
		g_file_fp_test.Read(&len, 4);
		memset(buf_temp, 0, 4096);
		g_file_fp_test.Read(buf_temp, len);
		
		g_file_fp_test.Close();

		if (!g_file_fp_test.Open(TEXT("F:\\play_test.dat"),CFile::modeCreate|CFile::modeReadWrite))
		{
			AfxMessageBox(_T("error: open the data stream fail"), MB_OK);
			return FALSE;
		}
		g_file_fp_test.Write(&len,4);
		g_file_fp_test.Write(buf_temp,len);
		g_file_fp_test.Close();
	}
#endif

	if (!malloc_show_buf())
	{
		return FALSE;
	}

	malloc_pre_pstYuvData();
	//server_rev_video_date();

	//test_pcm_buf(test_pcm_name);

	//HANDLE hFile = CreateFile(_T("H:\\cloud39E_tool\\toolsrc\\IPCTest_Tool\\pcm_test.pcm"), GENERIC_WRITE, FILE_SHARE_READ , NULL, 
	//				OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);//创建文件
	//CloseHandle(hFile);


	//server_rev_video_date();
	
	CDialog::OnInitDialog();
	//m_Status.Set


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


	GetModuleFileName(NULL,sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);
	sPath.ReleaseBuffer();
	nPos=sPath.ReverseFind ('\\');
	sPath=sPath.Left(nPos+1);
	_tcsncpy(path_module, sPath, MAX_PATH);

	g_handle = CreateSemaphore(NULL, 1, 1, NULL);

	get_system_info();


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
		ptNow->tm_year + 1900, ptNow->tm_mon + 1, ptNow->tm_mday,
		ptNow->tm_hour, ptNow->tm_min, ptNow->tm_sec);

	 //hFile = CreateFile(ConvertAbsolutePath(_T("pcm_test.pcm")), GENERIC_WRITE, FILE_SHARE_READ , NULL, 
	//				OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);//创建文件
	//CloseHandle(hFile);


#ifdef USE_LOG_FILE
	freopen(T2A(ConvertAbsolutePath(A2T((logInfoName)))), "w+t", stderr);
	fprintf(stderr, "open camview tool printf\n");
#else
	AllocConsole();
	freopen("CONOUT$", "w+t", stdout);
	freopen("CONIN$", "r+t", stdin);
	freopen("CONOUT$", "w+t", stderr);
#endif

	
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);


	// TODO: 在此添加额外的初始化代码

	CClientDC dc(this);
	font.CreatePointFont(120,_T("宋体"),NULL);
	CFont* def_font=dc.SelectObject(&font);
	dc.SelectObject(def_font);
	//font.DeleteObject();

	//CClientDC dc(this);
	oldFont.CreatePointFont(90,_T("宋体"),NULL);
	//CFont* def_font=dc.SelectObject(&oldFont);
	//dc.SelectObject(def_font);



	/////////////////////////////////////////////
	CRect rcDlg1,rcDlg2;
	GetClientRect(&rcDlg1);
	m_RightDlg.Create(IDD_FORMVIEW,this);
	m_RightDlg.GetClientRect(&rcDlg2);
	m_RightDlg.SetWindowPos(NULL,rcDlg1.Width()-rcDlg2.Width(),0,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);

	//m_BottomDlg.Create(IDD_BOTTOMVIEW,this);
   // m_BottomDlg.GetClientRect(&rcDlg2);
	//m_BottomDlg.SetWindowPos(NULL,0,rcDlg1.Height()-rcDlg2.Height(),0,0,SWP_NOSIZE|SWP_SHOWWINDOW);

	ListView_SetExtendedListViewStyle(m_RightDlg.m_test_config.m_hWnd, LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyle(m_RightDlg.m_test_wifi_list.m_hWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	
	//CoInitializeEx(NULL, COINIT_MULTITHREADED);
	avcodec_register_all();
	av_lockmgr_register(av_lock_manager_cb);

	//start the monitor thread
	m_runThreadFlag = TRUE;
	m_bNeedJudgeDisConnWork = TRUE;
	m_MonitorThread = (HANDLE)_beginthreadex(NULL, THREAD_STACK_SIZE, thread_begin, (LPVOID)this, 0, NULL);

	m_menuTalk.LoadMenu(IDR_MENU4);
	
	g_connet_flag = FALSE;
	g_save_flag = FALSE;

#if 0
	//判断配置文件夹是否存在，如果不存在那么就进行创建
	if(0xFFFFFFFF == GetFileAttributes(ConvertAbsolutePath(SERAIL_CONFIG)))
	{
		CreateDirectory(ConvertAbsolutePath(SERAIL_CONFIG), NULL);//创建文件夹
	}

	//判断配置文件夹是否存在，如果不存在那么就进行创建
	if(0xFFFFFFFF == GetFileAttributes(ConvertAbsolutePath(TEST_CONFIG_DIR)))
	{
		CreateDirectory(ConvertAbsolutePath(TEST_CONFIG_DIR), NULL);//创建文件夹
	}
#endif
	
	str.Format(_T("%s"), g_test_config.m_ip_addr);
	m_RightDlg.GetDlgItem(IDC_IPADDRESS_IP)->SetWindowText(str);
	str.Format(_T("%d"), g_test_config.m_ip_port);
	m_RightDlg.GetDlgItem(IDC_EDIT_PORT)->SetWindowText(str);

	if (TYPE_VIDEO_VGA == g_test_config.video_parm.video_size)
	{
		((CButton *)m_RightDlg.GetDlgItem(IDC_RADIO_VGA))->SetCheck(TRUE);
		((CButton *)m_RightDlg.GetDlgItem(IDC_RADIO_720P))->SetCheck(FALSE);
		((CButton *)m_RightDlg.GetDlgItem(IDC_RADIO_960P))->SetCheck(FALSE);
	}
	else if (TYPE_VIDEO_720P == g_test_config.video_parm.video_size)
	{
		((CButton *)m_RightDlg.GetDlgItem(IDC_RADIO_VGA))->SetCheck(FALSE);
		((CButton *)m_RightDlg.GetDlgItem(IDC_RADIO_720P))->SetCheck(TRUE);
		((CButton *)m_RightDlg.GetDlgItem(IDC_RADIO_960P))->SetCheck(FALSE);
	}
	else if (TYPE_VIDEO_960P == g_test_config.video_parm.video_size)
	{
		((CButton *)m_RightDlg.GetDlgItem(IDC_RADIO_VGA))->SetCheck(FALSE);
		((CButton *)m_RightDlg.GetDlgItem(IDC_RADIO_720P))->SetCheck(FALSE);
		((CButton *)m_RightDlg.GetDlgItem(IDC_RADIO_960P))->SetCheck(TRUE);
	}

	str.Format(_T("%d"), g_test_config.video_parm.frame);
	m_RightDlg.GetDlgItem(IDC_EDIT_FRAME)->SetWindowText(str);
	str.Format(_T("%d"), g_test_config.video_parm.data_rate);
	m_RightDlg.GetDlgItem(IDC_EDIT_VIDEO_FRAME)->SetWindowText(str);

	if (g_test_config.SamplesPerSec == 0)
	{
		g_test_config.SamplesPerSec = 8000;
	}
	str.Format(_T("%d"), g_test_config.SamplesPerSec);
	m_RightDlg.GetDlgItem(IDC_COMBO_SAMPLES)->SetWindowText(str);

	//InitToolBar();
	//InitStatusBar();
	//InitTreeCtrlPosition();
	//HTREEITEM hRoot = m_TreeCtrl.InsertItem( TREE_ROOT_ITEM_NAME, TVI_ROOT, TVI_LAST );

	InitPreviewWindows(TRUE, TRUE);
	//InitComboBox();
	InitPrivacyDialog();
	//PositionTheButton();
	//PositionTheImageCombo();

	SetTimer(TIMER_COMMAND, 10, NULL);

	m_bIsInit = TRUE;
	g_start_open_flag = TRUE;

	Creat_get_videodata_thread();

	if (!g_test_config.server_flag)
	{
		create_server_video_thread();
	}

	creat_record_wav_thread();
	creat_play_wav_thread(); 


	//Creat_Anyka_Test_thread();

	//Creat_test_monitor_thread();

	//Creat_find_ip_thread();

	//read_config(CONFIG_PATH);

	SetWindowText(TOOL_VERSOIN);

#if 0	
	//向系统注册热键
	bRet=RegisterHotKey(m_hWnd,HOTKEY_F1,0,VK_F1);
	bRet=RegisterHotKey(m_hWnd,HOTKEY_F2,0,VK_F2);
	bRet=RegisterHotKey(m_hWnd,HOTKEY_F3,0,VK_F3);
	bRet=RegisterHotKey(m_hWnd,HOTKEY_F4,0,VK_F4);
	bRet=RegisterHotKey(m_hWnd,HOTKEY_F6,0,VK_F6);
	bRet=RegisterHotKey(m_hWnd,HOTKEY_F7,0,VK_F7);
	
	//bRet=RegisterHotKey(m_hWnd,HOTKEY_FY,0,VK_RETURN);
	//bRet=RegisterHotKey(m_hWnd,HOTKEY_FY,0,'Y');
	//bRet=RegisterHotKey(m_hWnd,HOTKEY_Fy,0,'y');
	bRet=RegisterHotKey(m_hWnd,HOTKEY_FNULL,0,VK_SPACE);
	bRet=RegisterHotKey(m_hWnd,HOTKEY_F8,0,VK_F8);
	bRet=RegisterHotKey(m_hWnd,HOTKEY_F11,0,VK_F11);
	//bRet=RegisterHotKey(m_hWnd,HOTKEY_FESCAPE,0,VK_ESCAPE);
	//bRet=RegisterHotKey(m_hWnd,HOTKEY_FY,0,VK_RETURN);
	
#endif

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
	if ((pMsg->message == WM_LBUTTONDOWN) || (pMsg->message == WM_LBUTTONUP) || (pMsg->message == WM_MOUSEMOVE)) 
	{
#if 0
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
#endif
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
#if 0
	CRect rcDlg1,rcDlg2,rcDlg3;
	GetClientRect(&rcDlg1);
	m_RightDlg.GetClientRect(&rcDlg2);
	m_RightDlg.SetWindowPos(NULL,rcDlg1.Width()-rcDlg2.Width(),0,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);

	//m_BottomDlg.GetClientRect(&rcDlg3);
	//m_BottomDlg.SetWindowPos(NULL,0,rcDlg1.Height()-rcDlg3.Height(),0,0,SWP_NOSIZE|SWP_SHOWWINDOW);

	m_RightDlg.UpdateWindow();
	//m_BottomDlg.UpdateWindow();

#endif

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

	close_server_video_thread();
	//Sleep(100);
	Close_get_videodata_thread();
	//Sleep(100);
	close_record_wav_thread();
	//Sleep(100);
	close_play_wav_thread(); 
	
	g_open_tool = FALSE;
	Sleep(1000);

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
		memset(m_path, 0, MAX_PATH);
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

		memset(m_path, 0, MAX_PATH);
		_tcsncpy(m_path, filePath, MAX_PATH);
	}

	return m_path;
}


 void CAnykaIPCameraDlg::convert_seconds_to_systime(unsigned long seconds, T_SYSTIME *SysTime)
{
	unsigned long second_intv, minute_intv, hour_intv, dayofw_intv, day_intv;
	unsigned long year_intv, day_total, year_leap;
	unsigned char month_std_day[12]  = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	unsigned char month_leap_day[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	unsigned char i;
	unsigned short       nYearDist = 0;

	SysTime->year      = 1980;  //修改时间起点，要修改最大秒数和闰年数
	SysTime->month     = 1;
	SysTime->week      = 1; // 1980.1.1 is Tuesday
	SysTime->day       = 1;
	SysTime->hour      = 0;
	SysTime->minute    = 0;
	SysTime->second    = 0;
	nYearDist              = 3;

	second_intv = seconds % 60;
	minute_intv = (seconds % 3600) / 60;
	hour_intv   = (seconds % 86400) / 3600;
	dayofw_intv = (seconds % 604800) / 86400;
	day_total   = seconds / 86400;  //day_total is still exact!!
	year_intv   = day_total / 365;  //难点是year_intv的计算
	year_leap   = (year_intv + nYearDist) / 4;   //算的是肯定会经过的闰年数，如修改时间起点，如1970年改为1980年，括号中加的数为该年离即将来临的闰年的差减1，如1970后的为1972，为2-1=1；1980后的为1984，为4-1=3
	day_intv = day_total - year_leap * 366 - (year_intv - year_leap) * 365;

	if (day_intv > 366)
	{
		year_intv -= 1;
		year_leap -= 1;
		SysTime->year += (unsigned short)year_intv;
		if ((SysTime->year % 4) == 0)
		{
			day_intv = day_total - year_leap * 366 - (year_intv - year_leap) * 365;
		}
		else
		{
			day_intv = day_total - year_leap * 366 - (year_intv - year_leap) * 365 - 1;
		}
	}
	else
	{
		SysTime->year += (unsigned short)year_intv;
	}

	if ((SysTime->second + second_intv) < 60)
	{
		SysTime->second += (unsigned char)second_intv;
	}
	else
	{
		SysTime->second = SysTime->second + (unsigned char)second_intv - 60;
	}

	if ((SysTime->minute + minute_intv) < 60)
	{
		SysTime->minute += (unsigned char)minute_intv;
	}
	else
	{
		SysTime->minute += (unsigned char)minute_intv - 60;
	}

	if ((SysTime->hour + hour_intv) < 24)
	{
		SysTime->hour += (unsigned char)hour_intv;
	}
	else
	{
		SysTime->hour += (unsigned char)hour_intv - 24;
	}

	if ((SysTime->week + dayofw_intv) < 7)
	{
		SysTime->week += (unsigned char)dayofw_intv;
	}
	else
	{
		SysTime->week += (unsigned char)dayofw_intv - 7;
	}

	if ((SysTime->year % 4) == 0)
	{
		for (i = 0; i <= 11; i++)
		{
			if (day_intv >= month_leap_day[i])
			{
				day_intv -= month_leap_day[i];
			}
			else
			{
				SysTime->month += i;
				SysTime->day   += (unsigned char)day_intv;
				break;
			}
		}
	}
	else
	{
		for (i = 0; i <= 11; i++)
		{
			if (day_intv >= month_std_day[i])
			{
				day_intv -= month_std_day[i];
			}
			else
			{
				SysTime->month += i;
				SysTime->day   += (unsigned char)day_intv;
				break;
			}
		}
	}
	return;
}

BOOL CAnykaIPCameraDlg::save_picture_file()  
{  
	// TODO: 在此添加控件通知处理程序代码  
	//存储声音文件  
	T_SYSTIME SysTime;
	CFile m_file;  
	CFileException fileException;    
	SYSTEMTIME sys2; //获取系统时间确保文件的保存不出现重名  
	GetLocalTime(&sys2);  
	//以下实现将录入的声音转换为wave格式文件   

	//查找当前目录中有没有Voice文件夹 没有就先创建一个，有就直接存储  
	TCHAR szPath[MAX_PATH];       
	GetModuleFileName(NULL, szPath, MAX_PATH);  
	CString PathName(szPath);  
	//获取exe目录  
	CString PROGRAM_PATH = PathName.Left(PathName.ReverseFind(_T('\\')) + 1);  
	//Debug目录下RecordVoice文件夹中  
	PROGRAM_PATH+=_T("picture\\");  

	if (!(GetFileAttributes(PROGRAM_PATH)==FILE_ATTRIBUTE_DIRECTORY))  
	{  
		if (!CreateDirectory(PROGRAM_PATH,NULL))  
		{  

			AfxMessageBox(_T("Make Dir Error"));  

		}  
	}  

	CString m_csFileName=PROGRAM_PATH;//strVoiceFilePath  
      
	wchar_t s[30] = {0};

	convert_seconds_to_systime(g_time_seconds, &SysTime);
	_stprintf(s,_T("photo_%d%02d%02d_%02d%02d%02d"),SysTime.year,SysTime.month,SysTime.day,SysTime.hour,SysTime.minute,SysTime.second/*,sys2.wMilliseconds*/);  
	//_stprintf(s,_T("photo_%d%02d%02d_%02d%02d%02d"),sys2.wYear,sys2.wMonth,sys2.wDay,sys2.wHour,sys2.wMinute,sys2.wSecond/*,sys2.wMilliseconds*/);  
	m_csFileName.Append(s);  
	m_csFileName.Append(_T(".jpg"));  

	memset(enter_show.show_img.m_imgpath, 0, 260*sizeof(TCHAR));
	_tcscpy(enter_show.show_img.m_imgpath, m_csFileName);
	//_tcscpy(enter_show.show_img.m_imgpath, _T("F:\\test.JPG"));
	
#if 1
	m_file.Open(m_csFileName,CFile::modeCreate|CFile::modeReadWrite, &fileException);  
	m_file.SeekToBegin();    
	m_file.Write(g_picture_buf,g_picture_len);    
	m_file.Close();

	HANDLE hFile =CreateFile(m_csFileName,   GENERIC_READ,   0,   NULL,   OPEN_EXISTING,   0,   NULL);    
	_ASSERTE(INVALID_HANDLE_VALUE != hFile);    

	//取得文件大小    
	UINT dwFileSize = GetFileSize(hFile,   NULL);    
	_ASSERTE(-1 != dwFileSize);  

	if (dwFileSize == 0 || dwFileSize == 0xFFFFFFFF)
	{
		AfxMessageBox(_T("the picture file len is error,pls check"), MB_OK);
		return FALSE;
	}  
	CloseHandle(hFile); 
#endif
	return TRUE;
}  



BOOL flag_tool = TRUE;
void CAnykaIPCameraDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CString str;
	if (g_max_frame_idex != 0)
	{
		str.Format(_T("%d%%"), (g_current_frame_idex*100)/g_max_frame_idex);
		m_RightDlg.SetDlgItemText(IDC_STATIC_PLAY_PROGRECC, str);
		
		str.Format(_T("%d"), g_current_frame_idex);
		m_RightDlg.SetDlgItemText(IDC_STATIC_PLAY_FRAME_NUM, str);
		
	}
	

	if (g_recode_video)
	{
		m_RightDlg.m_start_video_flag = FALSE;
		m_RightDlg.GetDlgItem(IDC_CHECK_SAVE_VIDEO)->EnableWindow(FALSE);
		m_RightDlg.GetDlgItem(IDC_EDIT_SAVE_VIDEO)->EnableWindow(FALSE);
		m_RightDlg.GetDlgItem(IDC_BUTTON_VIDEO_PATH)->EnableWindow(FALSE);
		m_RightDlg.SetDlgItemText(IDC_BUTTON_CONNET, _T("停止视频"));
		m_RightDlg.GetDlgItem(IDC_BUTTON_PLAY_VIDEO)->EnableWindow(FALSE);
		m_RightDlg.GetDlgItem(IDC_BUTTON_PLAY_FIRST_FRAME)->EnableWindow(FALSE);
		
	} 
	else
	{
		m_RightDlg.m_start_video_flag = TRUE;
		m_RightDlg.GetDlgItem(IDC_CHECK_SAVE_VIDEO)->EnableWindow(TRUE);
		m_RightDlg.GetDlgItem(IDC_EDIT_SAVE_VIDEO)->EnableWindow(TRUE);
		m_RightDlg.GetDlgItem(IDC_BUTTON_VIDEO_PATH)->EnableWindow(TRUE);
		m_RightDlg.SetDlgItemText(IDC_BUTTON_CONNET, _T("启动视频"));
		m_RightDlg.GetDlgItem(IDC_BUTTON_PLAY_VIDEO)->EnableWindow(TRUE);
		m_RightDlg.GetDlgItem(IDC_BUTTON_PLAY_FIRST_FRAME)->EnableWindow(TRUE);
	}
	

	if (g_get_picture_flag)
	{
		g_get_picture_flag = FALSE;
		//保存文件
		if(save_picture_file())
		{
			//显示图像
			if (g_show_picture_flag)
			{
				g_show_picture_flag = FALSE;
				enter_show.DoModal();
				g_show_picture_flag = TRUE;
			}
			
		}
		g_check_picture_flag = TRUE;
	}

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
#if 0
	if (iSelect > 2 || iSelect < 0) return -1;

	CAutoLock lock(&m_csForOpenCloseStream);
	if (m_pClient[iSelect]) {
		m_pClient[iSelect]->Close();
		delete m_pClient[iSelect];
	}

	m_pClient[iSelect] = NULL;
#endif

	if (m_videoDecoder[iSelect]) delete m_videoDecoder[iSelect];
	m_videoDecoder[iSelect] = NULL;
	if (m_videoRender[iSelect]) delete m_videoRender[iSelect];
	m_videoRender[iSelect] = NULL;

	if ((iSelect == m_nAudioClientIndex) && bNeedCloseAudio) TempCloseTalk(iSelect);
	return 0;
}

#define MAX_WAIT_CNT	20



int CAnykaIPCameraDlg::show_Stream(int iSelect, const char * strURL, BOOL bNeedENotify)
{
	USES_CONVERSION;

	WCHAR astrMsg[300] = {0};
	int iErrorCode = 0;


	unsigned int iStreamChoose = 0, iSCnt = 0, len = MAX_STREAM_NAME;
	int iFps = 0;

	const char * pWhere = NULL;
	
	if (g_test_config.video_parm.frame == 0)
	{
		iFps = 8;
	}
	else
	{
		iFps = g_test_config.video_parm.frame;
	}
	

	CAutoLock lock(&m_csForOpenCloseStream);

    CloseTheStream(iSelect, TRUE);

	unsigned int iStreamCnt = 1;
	//STREAM_TYPE type = STREAM_AUDIO;
	STREAM_TYPE type = STREAM_VIDEO;
	//m_pClient[iSelect]->GetStreamCount(iStreamCnt);

	for (unsigned int i = 0; i < iStreamCnt; ++i) {
		//m_pClient[iSelect]->GetStreamType(i, type);

		if (type == STREAM_AUDIO) 
		{
			//预览开始时默认不播放音频
		}else if (type == STREAM_VIDEO) 
		{
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

			//m_pClient[iSelect]->RegisterSink(type, m_videoDecoder[iSelect]);
			m_videoDecoder[iSelect]->RegisterSink(m_videoRender[iSelect], SINK_VIDEO);
		}
	}

	//m_pClient[iSelect]->Play();

	if (m_videoDecoder[iSelect])
		m_videoDecoder[iSelect]->Start();

	//if (m_AudioDecoder[iSelect])
	//m_AudioDecoder[iSelect]->Start();

	return 0;
}


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


#if 1
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
#endif

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
			_sntprintf(astrMsg, 300, L"OpenURL %s error! error = %s", 
				A2W(strURL), A2W(m_pClient[iSelect]->GetLastError()));
			AfxMessageBox( astrMsg, 0, 0 );
		}
		return -1;
	}
#if 1
	int nWaitCnt = 0;
	bool isPrepare = false;
	while (!isPrepare) {
		iErrorCode = m_pClient[iSelect]->IsPrepare(isPrepare);
		if ((iErrorCode != 0) || (nWaitCnt >= MAX_WAIT_CNT)) {
			if (bNeedENotify) {
				if ((iErrorCode == 0) && (nWaitCnt >= MAX_WAIT_CNT)) 
					_sntprintf(astrMsg, 300, L"连接服务器%s, 超时！", A2W(strURL));
				else
					_sntprintf(astrMsg, 300, L"OpenURL %s error! error = %s", 
					A2W(strURL), A2W(m_pClient[iSelect]->GetLastError()));

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

#endif

	unsigned int iStreamCnt = 0;
	STREAM_TYPE type = STREAM_AUDIO;
	m_pClient[iSelect]->GetStreamCount(iStreamCnt);

	for (unsigned int i = 0; i < iStreamCnt; ++i) {
		m_pClient[iSelect]->GetStreamType(i, type);

		if (type == STREAM_AUDIO) 
		{
			//预览开始时默认不播放音频
		}else if (type == STREAM_VIDEO) 
		{
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


	memset(strURL, 0, MAX_RTSP_URL_LEN);
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
		_sntprintf(astrMsg, 100, L"Can't listen audio from %s server, IP = %s!\n",
			A2W(strServerID), A2W(strIPAddr));
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
		if ((ulSendAudioAddr != m_stKickOutParam.ulIpAddr) 
			|| ((unsigned long)usPort != m_stKickOutParam.ulPort))
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

	g_enter_flag = TRUE;
	// TODO: 在此处添加消息处理程序代码
	if (IsWindow(m_RightDlg.m_hWnd))
	{
		CRect rcDlg1,rcDlg2;
		GetClientRect(&rcDlg1);
		m_RightDlg.GetClientRect(&rcDlg2);
		m_RightDlg.SetWindowPos(NULL,rcDlg1.Width()-rcDlg2.Width(),0,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);
	}

	if (IsWindow(m_Preview[0].m_hWnd))
	{
		CRect rcDlg1,rcDlg2,rcDlg3;
		GetClientRect(&rcDlg1);
		m_RightDlg.GetClientRect(&rcDlg2);
		//m_BottomDlg.GetClientRect(&rcDlg3);
		m_Preview[0].SetWindowPos(NULL,0,0,rcDlg1.Width()-rcDlg2.Width()-10,
			rcDlg1.Height()-10,SWP_NOMOVE|SWP_SHOWWINDOW);
	}
}

#define BUTTON_WIDTH_APART		1
#define BUTTON_HEIGHT_APART		1

void CAnykaIPCameraDlg::PositionTheButton()
{
#if 0
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
#endif
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

	int x = cPreview2Rect.right + 
		nWidthMid - (cTextRect.Width() +  cComboRect.Width() + COMBO_WIDTH_APART) / 2;
	int y = cPreview2Rect.top + 
		nHeightMid - ((cComboRect.Height() + COMBO_HEIGHT_APART) / 2) - cComboRect.Height() * 2;

	pStaticText1->MoveWindow(x, y + (cComboRect.Height() / 2) - (cTextRect.Height() / 2),
		cTextRect.Width(), cTextRect.Height());
	m_ContrastCombo.MoveWindow(x + cTextRect.Width() + COMBO_WIDTH_APART, 
		y, cComboRect.Width(), cComboRect.Height());

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

BOOL CAnykaIPCameraDlg::Connet_FTPServer(LPCTSTR addr, UINT idex) 
{

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
		if(m_pFtpConnection[idex] != NULL)
		{
			m_pFtpConnection[idex]->Close();
			delete m_pFtpConnection[idex];
			m_pFtpConnection[idex] = NULL;
		}

		m_pFtpConnection[idex] = m_pInetSession[idex]->GetFtpConnection(addr, m_username, m_password, 21 ,g_test_config.win10_flag);
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

	return TRUE;
}







void CAnykaIPCameraDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnHotKey(nHotKeyId, nKey1, nKey2);
}


//static text颜色
HBRUSH CAnykaIPCameraDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	//红绿两种对应static text控件的画刷
	static HBRUSH brush_red = ::CreateSolidBrush(RGB(255,0,0));
	static HBRUSH brush_green = ::CreateSolidBrush(RGB(0,255,0));

	enum STATIC_BKCOLOR
	{
		NULL_COLOR,
		RED_COLOR,
		GREEN_COLOR,
	};

	// TODO: Change any attributes of the DC here

	STATIC_BKCOLOR static_BkColor = NULL_COLOR;
	HBRUSH return_hbr = hbr;

	if (pWnd->GetDlgCtrlID()==IDC_STATIC_TEST_CONTENT)
	{
		/*if(g_test_pass_flag==0)
		{
		static_BkColor = GREEN_COLOR;
		}else if(g_test_pass_flag==2)
		{
		static_BkColor = RED_COLOR;
		}*/
		static_BkColor = GREEN_COLOR;
	}
	// TODO: Return a different brush if the default is not desired

	switch (static_BkColor)
	{
	case RED_COLOR:
		pDC->SetTextColor(RGB(255,0,0));
		pDC->SetBkColor(RGB(0,0,0));
		return_hbr = (HBRUSH)brush_red;
		break;
	case GREEN_COLOR:
		pDC->SetTextColor(RGB(0,255,0));
		pDC->SetBkColor(RGB(0,0,0));
		return_hbr = (HBRUSH)brush_green;
		break;
	case NULL_COLOR:
		return_hbr = hbr;
		break;
	default:
		return_hbr = hbr;
	}

#if 0
	switch(pWnd->GetDlgCtrlID())
	{
	case IDC_STATIC_TEST_TITLE:
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(0,255,0));

	case IDC_STATIC_TEST_CONTENT:
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(0,255,0));


		return (HBRUSH)GetStockObject(HOLLOW_BRUSH);

	}
#endif

	return hbr;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



BOOL CAnykaIPCameraDlg::creat_video_file(const void* buf, UINT buf_len) 
{

	g_file_fp.Write(buf,buf_len);

	return TRUE;

}

BYTE * g_test_buff = NULL;
UINT g_test_buff_len = 0;
BOOL CAnykaIPCameraDlg::get_video_test() 
{
	UINT len = 0;
	CFile fp;
	if (!fp.Open(TEXT("H:\\cloud39E_dana\\tool_src\\IPCTest_Tool_test\\Anyka IP Camera\\Anyka IP Camera\\Debug\\VENC_test_video.bin"),CFile::modeReadWrite))
	{
		AfxMessageBox(_T("error: open the data stream fail"), MB_OK);
		return FALSE;
	}

	g_test_buff_len = fp.GetLength();
	if (g_test_buff_len == 0)
	{
		AfxMessageBox(_T("error: get the file len == 0"), MB_OK);
		return FALSE;
	}
	g_test_buff=new BYTE[g_test_buff_len + 1];

	//读取一帧数据
	fp.SeekToBegin();
	fp.Read(g_test_buff,g_test_buff_len);
	return TRUE;
}

#if 0

BOOL CAnykaIPCameraDlg::get_video_thread() 
{
	struct timeval presentationTime = {0};
	UINT len = 0;
	BOOL enter_flag = TRUE;
	//打开一个文件
	CFile fp;
	if (!fp.Open(TEXT("H:\\cloud39E_dana\\tool_src\\IPCTest_Tool_test\\Anyka IP Camera\\Anyka IP Camera\\Debug\\VENC_19800101000514.bin"),CFile::modeReadWrite))
	{
		AfxMessageBox(_T("error: open the data stream fail"), MB_OK);
		return FALSE;
	}

	len = fp.GetLength();
	if (len == 0)
	{
		AfxMessageBox(_T("error: get the file len == 0"), MB_OK);
		return FALSE;
	}

	BYTE * buff=new BYTE[len];

	while (1)
	{

		//读取一帧数据
		fp.SeekToBegin();
		fp.Read(buff,len);

		if (enter_flag)
		{
			if (show_Stream(0, NULL, FALSE) == 0)
			{
				enter_flag = FALSE;
			}
		}

		m_videoDecoder[0]->SendData( buff, len, presentationTime, "video", "H264", NULL);

		Sleep(200);
	}

	fp.Close();
	delete[] buff;//释放
}

#else
//通过网络


BOOL CAnykaIPCameraDlg::get_video_thread() 
{
	struct timeval presentationTime = {0};
	UINT len = 0;
	BYTE * receive_buf = NULL;

	//打开一个文件
	while (1)
	{
		if (g_getvideo_Thread != INVALID_HANDLE_VALUE)
		{
			if (g_enter_flag)
			{
				if (show_Stream(0, NULL, FALSE) == 0)
				{
					g_enter_flag = FALSE;
				}
			}

			//读取一帧数据
			//if (g_video_receive_buf != NULL && g_receive_buf_len > 0 && g_receive_buf_len <= VIDEO_RECEIVE_BUFFER_SIZE)
			if (g_malloc_buf_success)
			{
				if (g_show_video[g_read_current_buf_idex].m_empty_flag == FALSE && g_show_video[g_read_current_buf_idex].m_receive_buf_len > 0)
				{
					len = g_show_video[g_read_current_buf_idex].m_receive_buf_len;

						
					//m_videoDecoder[0]->SendData(g_test_buff, g_test_buff_len, presentationTime, "video", "H264", NULL);
					m_videoDecoder[0]->SendData(g_show_video[g_read_current_buf_idex].m_video_receive_buf, len, presentationTime, "video", "H264", NULL);
					memset(g_show_video[g_read_current_buf_idex].m_video_receive_buf, 0 , VIDEO_RECEIVE_BUFFER_SIZE);
					g_show_video[g_read_current_buf_idex].m_empty_flag = TRUE;
					g_show_video[g_read_current_buf_idex].m_receive_buf_len = 0;

					g_read_current_buf_idex++;
					if (g_read_current_buf_idex == MAX_DECONDE_BUF_NUM)
					{
						g_read_current_buf_idex = 0;
					}
				}
				else
				{
					Sleep(10); 
				}
			}
			else
			{
				Sleep(100);
			}
		}
		else
		{

			break;
		}
	}

	return TRUE;
}


#endif

DWORD WINAPI get_videodata_main(LPVOID lpParameter) 
{
	CAnykaIPCameraDlg testDlg;
	
	if (!testDlg.get_video_thread())
	{
		return 0;
	}

	return 1;
}

BOOL CAnykaIPCameraDlg::Creat_get_videodata_thread() 
{
	UINT idex = 0;

	g_hWnd = m_Preview[0].m_hWnd;

	Close_get_videodata_thread();

	g_getvideo_Thread = CreateThread(NULL, 0, get_videodata_main, &idex, 0, NULL);
	if (g_getvideo_Thread == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;
}

void CAnykaIPCameraDlg::Close_get_videodata_thread() 
{
	if(g_getvideo_Thread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_getvideo_Thread);
		g_getvideo_Thread = INVALID_HANDLE_VALUE;
	}

}

DWORD WINAPI rev_video_date(LPVOID lpParameter)
{
	CAnykaIPCameraDlg  TestToolDlg;
#if 1
	TestToolDlg.server_rev_video_date();
#else
	CClientSocket m_ClientSocket_heat;
	UINT idex = 0;
	char *lpBuf = NULL;
	UINT nBufLen = VIDEO_RECEIVE_BUFFER_SIZE;   //4 + 4 + VIDEO_RECEIVE_BUFFER_SIZE + 4 + 4;
	CString str;
	UINT *buf_temp = (UINT *)lpParameter;
	int len;
	UINT frame_len, frame_count;
	UINT frame_incomplete = 0, header_incomplete = 0;
	char saved_header[8];
	UINT header_left = 0, header_get  = 0;
	UINT i, count = 0, buf_idex = 0, read_buf_len = 0;
	UINT incomplete_buflen = 0;
	UINT incomplete_end_buflen = 0;
	UINT incomplete_framelen = 0;
	BOOL video_data_flag = TRUE;
	//TCHAR fram_num[20] ={0};


	header_incomplete  = 0;
	frame_incomplete   = 0;

	memcpy(&idex, buf_temp, 1);

	lpBuf = new char[nBufLen + 1];
	if (lpBuf == NULL)
	{
		AfxMessageBox(_T("malloc receive buf fail"), MB_OK);
		return -1;
	}

	//获取心跳命令
	while (1)
	{
		//Sleep(100);
		if (g_hBurnThread_rev_data[idex] != INVALID_HANDLE_VALUE )
		{
			if (!g_malloc_buf_success)
			{
				Sleep(1000);
				continue;
			}
			memset(lpBuf,0,nBufLen + 1);
#if 0
			if (buf_idex >= g_test_buff_len)
			{
				buf_idex = 0;
			}
			
			if(buf_idex == 0)
			{
				read_buf_len = 94233;
			}
			else
			{
				if (g_test_buff_len - buf_idex > nBufLen)
				{
					read_buf_len = 	nBufLen;
				}
				else
				{
					read_buf_len = g_test_buff_len - buf_idex;
				}
			}
			
			
			memcpy(lpBuf, &g_test_buff[buf_idex], read_buf_len);
			buf_idex += read_buf_len;

			len = read_buf_len;
#else

			len = m_ClientSocket_heat.Socket_Receive(lpBuf, nBufLen, 0);
			if (len == -1)
			{
				//AfxMessageBox(_T("获取数据出错"), MB_OK);
				continue;
			}
			else
			{
#if 0
				if (!g_file_fp.Open(TEXT("H:\\cloud39E_dana\\tool_src\\IPCTest_Tool_test\\Anyka IP Camera\\Anyka IP Camera\\Debug\\VENC_test.bin"),CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite))
				{
					AfxMessageBox(_T("error: open the data stream fail"), MB_OK);
					return FALSE;
				}
				TestToolDlg.creat_video_file(lpBuf, len);
				g_file_fp.Close();
#endif
				//continue;
			}

#endif
			
			

			if(len > 0 && (UINT)len <= nBufLen)//4 + 4 + VIDEO_RECEIVE_BUFFER_SIZE + 4 + 4)
			{
				frame_count = 0;
				/*parse data check header  "VCMD" + cmd id + cmd arg len + cmd arg + crc + "CEND"*/
				for(i = 0; i < (UINT)len; i++)
				{

					header_get = 0;
					/*the previous data may the remaining for the last frame*/
					if(frame_count == 0 && frame_incomplete == 1 )
					{
						if (incomplete_framelen - incomplete_buflen + 8 > (UINT)len)
						{
							if (video_data_flag)
							{
								memcpy(&g_one_video_receive_buf[incomplete_buflen], lpBuf, len);
							}
							else
							{
								memcpy(&g_one_audio_receive_buf[incomplete_buflen], lpBuf, len);
							}
							
							incomplete_buflen += len;
							break;
						}
						else
						{
							/*cp the remaining frame to the compose the whole frame*/
							frame_incomplete = 0;

							//后半帧
							if (incomplete_framelen > incomplete_buflen)
							{
								if (video_data_flag)
								{
									memcpy(&g_one_video_receive_buf[incomplete_buflen], lpBuf, incomplete_framelen - incomplete_buflen);
								}
								else
								{
									memcpy(&g_one_audio_receive_buf[incomplete_buflen], lpBuf, incomplete_framelen - incomplete_buflen);
								}
								
							}
							
							if (video_data_flag)
							{
								//一帧数据
								while(1)
								{
									if (g_show_video[g_write_current_buf_idex].m_empty_flag == TRUE)
									{
										break;
									}

									//Sleep(2);
								}

								memset(g_show_video[g_write_current_buf_idex].m_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
								memcpy((char *)g_show_video[g_write_current_buf_idex].m_video_receive_buf, g_one_video_receive_buf, incomplete_framelen);
								if(g_save_flag)
								{
									TestToolDlg.creat_video_file(&incomplete_framelen, 4);
									TestToolDlg.creat_video_file(g_show_video[g_write_current_buf_idex].m_video_receive_buf, incomplete_framelen);
								}

								g_show_video[g_write_current_buf_idex].m_receive_buf_len = incomplete_framelen;
								g_show_video[g_write_current_buf_idex].m_empty_flag = FALSE;

								g_write_current_buf_idex++;
								if (g_write_current_buf_idex == MAX_DECONDE_BUF_NUM)
								{
									g_write_current_buf_idex = 0;
								}
							}
							else
							{
								//一帧数据
								while(1)
								{
									if (g_play_data[g_audio_write_current_buf_idex].m_empty_flag == TRUE)
									{
										break;
									}
									//Sleep(2);
								}

								memset(g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, 0, AUDIO_RECEIVE_BUFFER_SIZE);
								memcpy((char *)g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, g_one_audio_receive_buf, incomplete_framelen);
								if(g_save_flag)
								{
									//TestToolDlg.creat_video_file(g_show_video[g_write_current_buf_idex].m_video_receive_buf, incomplete_framelen);
								}

								g_play_data[g_audio_write_current_buf_idex].m_receive_buf_len = incomplete_framelen;
								g_play_data[g_audio_write_current_buf_idex].m_empty_flag = FALSE;

								g_audio_write_current_buf_idex++;
								if (g_audio_write_current_buf_idex == MAX_AUDIO_BUF_NUM)
								{
									g_audio_write_current_buf_idex = 0;
								}
							}
							
							i += incomplete_framelen - incomplete_buflen + 8 - incomplete_end_buflen; //加上check_sum和包尾

							if (len == i)
							{
								break;
							}
						}

					}

					/*remain data less than a frame header*/
					if((len - i) < 8 && header_incomplete == 0)
					{
						/*save this packet header and recv next packet*/
						header_incomplete = 1;
						header_left = 8 - (len - i);
						memcpy(saved_header, &lpBuf[i], len - i); 
						break;
					}


					if(header_incomplete && len - i >= header_left)
					{
						header_incomplete = 0;
						/*complete the header and parse*/
						memcpy(&saved_header[8 - header_left], lpBuf, header_left);
						i += header_left;
						if((saved_header[0] == 'F' && saved_header[1] == 'R' && saved_header[2] == 'A' && saved_header[3] == 'M')
							|| (saved_header[0] == 'A' && saved_header[1] == 'E' && saved_header[2] == 'N' && saved_header[3] == 'C'))
						{
							header_get = 1;
							frame_len = *(int*)&saved_header[4];
							goto FRAME_COPY;
						}
						else
						{
							/*go ahead to next frame*/
							count++;
						}

					}

					if(header_get == 0)
					{
						if(lpBuf[i] == 'F' && lpBuf[i + 1] == 'R' && lpBuf[i + 2] == 'A' && lpBuf[i + 3] == 'M') 
						{
							video_data_flag = TRUE;
							
						}
						else if(lpBuf[i] == 'A' && lpBuf[i + 1] == 'E' && lpBuf[i + 2] == 'N' && lpBuf[i + 3] == 'C') 
						{
							video_data_flag = FALSE;
						}
						else
						{
							continue;
						}

					}


					i += 4;	//包头
					frame_len = *(int*)&lpBuf[i];
					header_incomplete = 0;
					/*the last frame exceeded this recvbuf save this incomplete frame*/
					i += 4; //包长

FRAME_COPY:
					if(frame_len + 8 > (len - i)) 
					{
						/*save this incomplete frame*/
						frame_incomplete = 1;
						//前半帧
						if (frame_len > (len - i))
						{
							if (video_data_flag)
							{
								memcpy(g_one_video_receive_buf, &lpBuf[i], len - i);
							}
							else
							{
								memcpy(g_one_audio_receive_buf, &lpBuf[i], len - i);
							}
							
							incomplete_buflen =  len - i;
							incomplete_end_buflen =	0;
						}
						else
						{
							if (video_data_flag)
							{
								memcpy(g_one_video_receive_buf, &lpBuf[i], frame_len);
							}
							else
							{
								memcpy(g_one_audio_receive_buf, &lpBuf[i], frame_len);
							}
							
							incomplete_buflen =  frame_len;
							incomplete_end_buflen =	len - i - frame_len;
						}
						
						incomplete_framelen = frame_len;

						break;

					}
					else
					{
						/*cp frame data to decodeer*/
						frame_count++; 
						i += frame_len;  //
						
						if (video_data_flag)
						{
							//一帧数据
							while(1)
							{
								if (g_show_video[g_write_current_buf_idex].m_empty_flag == TRUE)
								{
									break;
								}

								//Sleep(2);
							}

							memset(g_show_video[g_write_current_buf_idex].m_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
							memcpy((char *)g_show_video[g_write_current_buf_idex].m_video_receive_buf, &lpBuf[i - frame_len], frame_len);
							if(g_save_flag)
							{
							    TestToolDlg.creat_video_file(&frame_len, 4);
								TestToolDlg.creat_video_file(g_show_video[g_write_current_buf_idex].m_video_receive_buf, frame_len);
							}
							g_show_video[g_write_current_buf_idex].m_receive_buf_len = frame_len;
							g_show_video[g_write_current_buf_idex].m_empty_flag = FALSE;

							g_write_current_buf_idex++;
							if (g_write_current_buf_idex == MAX_DECONDE_BUF_NUM)
							{
								g_write_current_buf_idex = 0;
							}

						}
						else
						{
							//一帧数据
							while(1)
							{
								if (g_play_data[g_audio_write_current_buf_idex].m_empty_flag == TRUE)
								{
									break;
								}

								//Sleep(2);
							}

							memset(g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, 0, AUDIO_RECEIVE_BUFFER_SIZE);
							memcpy((char *)g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, &lpBuf[i - frame_len], frame_len);
							if(g_save_flag)
							{
								//TestToolDlg.creat_video_file(g_show_video[g_write_current_buf_idex].m_video_receive_buf, frame_len);
							}
							g_play_data[g_audio_write_current_buf_idex].m_receive_buf_len = frame_len;
							g_play_data[g_audio_write_current_buf_idex].m_empty_flag = FALSE;

							g_audio_write_current_buf_idex++;
							if (g_audio_write_current_buf_idex == MAX_AUDIO_BUF_NUM)
							{
								g_audio_write_current_buf_idex = 0;
							}
						}
						
						i += 7; //加上check_sum和包尾
					}
				}
			}
		}
		else
		{
			Sleep(100);
			break;
		}
	}

	delete[] lpBuf;
#endif

	return 1;
}


BOOL CAnykaIPCameraDlg::create_rev_video_data_thread(UINT idex) 
{
	close_rev_video_data_thread(idex);

	memcpy(rve_param[idex], &idex, sizeof(UINT));
	g_hBurnThread_rev_data[idex] = CreateThread(NULL, 0, rev_video_date, rve_param[idex], 0, NULL);
	if (g_hBurnThread_rev_data[idex] == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;
}

void CAnykaIPCameraDlg::close_rev_video_data_thread(UINT idex) 
{
	if(g_hBurnThread_rev_data[idex] != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_rev_data[idex]);
		g_hBurnThread_rev_data[idex] = INVALID_HANDLE_VALUE;
	}
}


BOOL CAnykaIPCameraDlg::connet_net(TCHAR *ipaddr, unsigned int iPort, UINT idex) 
{
	//创建socket
	CString str;
	UINT i = 0;
	UINT times = 5;

	USES_CONVERSION;

	for (i = 0; i < times; i++)
	{
		// TODO: Add your control notification handler code here
		if (!m_ClientSocket.Socket_Create(idex))
		{
			AfxMessageBox(_T("Socket_Create fail"));
			return FALSE;
		}
		if(m_ClientSocket.Socket_Connect(T2A(ipaddr), iPort, idex))	
		{
			//连接ftp服务器
			/*
			if (!Connet_FTPServer(addr, idex))
			{
			//AfxMessageBox(_T("Connet_FTPServer fail"));
			return FALSE;
			}
			*/

			if (!create_rev_video_data_thread(idex))
			{
				AfxMessageBox(_T("创建接收数据线程失败"));
				return FALSE;
			}
			g_senddata_flag = FALSE;
			break;

		}
		else
		{
			//AfxMessageBox(_T("Socket_Connect fail"));
			//return FALSE;

			Sleep(1000);
			continue;
		}
	}

	if (i == times)
	{
		AfxMessageBox(_T("Socket_Connect fail"));
		return FALSE;
	}

	g_connet_flag = TRUE;

	return TRUE;
	
}

BOOL CAnykaIPCameraDlg::malloc_show_buf(void)
{
	UINT i = 0;
	//启动显示视频
	free_show_buf();
	for (i =0; i < MAX_DECONDE_BUF_NUM; i++)
	{
		if (g_show_video[i].m_video_receive_buf == NULL)
		{
			g_show_video[i].m_video_receive_buf = new BYTE[VIDEO_RECEIVE_BUFFER_SIZE];
			if (g_show_video[i].m_video_receive_buf == NULL)
			{
				AfxMessageBox(_T("接收数据内存分配失败"), MB_OK);
				return FALSE;
			}
			memset(g_show_video[i].m_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
		}
		g_show_video[i].m_receive_buf_len = 0;
		g_show_video[i].m_empty_flag = TRUE;
	}

	g_one_video_receive_buf = new BYTE[VIDEO_RECEIVE_BUFFER_SIZE];
	if (g_one_video_receive_buf == NULL)
	{
		AfxMessageBox(_T("一个内存分配失败"), MB_OK);
		return FALSE;
	}

	g_one_video_receive_buf_len = 0;


	//音频
	for (i =0; i < MAX_AUDIO_BUF_NUM; i++)
	{
		if (g_play_data[i].m_video_receive_buf == NULL)
		{
			g_play_data[i].m_video_receive_buf = new BYTE[AUDIO_RECEIVE_BUFFER_SIZE];
			if (g_play_data[i].m_video_receive_buf == NULL)
			{
				AfxMessageBox(_T("接收数据内存分配失败"), MB_OK);
				return FALSE;
			}
			memset(g_play_data[i].m_video_receive_buf, 0, AUDIO_RECEIVE_BUFFER_SIZE);
		}
		g_play_data[i].m_receive_buf_len = 0;
		g_play_data[i].m_empty_flag = TRUE;
	}

	g_one_audio_receive_buf = new BYTE[AUDIO_RECEIVE_BUFFER_SIZE];
	if (g_one_audio_receive_buf == NULL)
	{
		AfxMessageBox(_T("一个内存分配失败"), MB_OK);
		return FALSE;
	}

	g_one_audio_receive_buf_len = 0;

	return TRUE;
}

BOOL CAnykaIPCameraDlg::free_show_buf(void) 
{
	UINT i = 0;

	for (i =0; i < MAX_DECONDE_BUF_NUM; i++)
	{
		if (g_show_video[i].m_video_receive_buf != NULL)
		{
			delete[] g_show_video[i].m_video_receive_buf;
			g_show_video[i].m_video_receive_buf = NULL;
		}
		g_show_video[i].m_receive_buf_len = 0;
		g_show_video[i].m_empty_flag = TRUE;
	}

	if (g_one_video_receive_buf != NULL)
	{
		delete[] g_one_video_receive_buf;
		g_one_video_receive_buf = NULL;
	}


	for (i =0; i < MAX_AUDIO_BUF_NUM; i++)
	{
		if (g_play_data[i].m_video_receive_buf != NULL)
		{
			delete[] g_play_data[i].m_video_receive_buf;
			g_play_data[i].m_video_receive_buf = NULL;
		}
		g_play_data[i].m_receive_buf_len = 0;
		g_play_data[i].m_empty_flag = TRUE;
	}

	if (g_one_audio_receive_buf != NULL)
	{
		delete[] g_one_audio_receive_buf;
		g_one_audio_receive_buf = NULL;
	}

	return TRUE;
}


BOOL CAnykaIPCameraDlg::connet_video_net(void) 
{
	CString str;
	UINT i = 0;

	USES_CONVERSION;
		
	if (g_test_config.server_flag)
	{
		m_RightDlg.GetDlgItemText(IDC_IPADDRESS_IP, str);
		if (str.IsEmpty())
		{
			AfxMessageBox(_T("ip 地址为空"), MB_OK);
			return FALSE;
		}
		memset(g_test_config.m_ip_addr, 0 , MAC_ADDRESS_LEN);
		_tcscpy(g_test_config.m_ip_addr, str);

		if (g_test_config.m_ip_addr[0] == 0x30 && g_test_config.m_ip_addr[2] == 0x30 && g_test_config.m_ip_addr[4] == 0x30 && g_test_config.m_ip_addr[6] == 0x30)
		{
			AfxMessageBox(_T("ip 地址为空"), MB_OK);
			return FALSE;
		}
	}

	m_RightDlg.GetDlgItemText(IDC_EDIT_PORT, str);
	if (str.IsEmpty())
	{
		AfxMessageBox(_T("端口为空"), MB_OK);
		return FALSE;
	}
	g_test_config.m_ip_port = atoi(T2A(str));

	if (g_test_config.server_flag)
	{
		if (!connet_net(g_test_config.m_ip_addr, g_test_config.m_ip_port, 0))
		{
			return FALSE;
		}
	}

	g_enter_flag = TRUE;

	//发送启动视频命令
	if(!Send_video_cmd(TYPE_VIDEO_START, NULL, 0, 0))
	{
		AfxMessageBox(_T("发送开始视频命令失败"), MB_OK);
		return FALSE;
	}
	
	if (g_test_config.server_flag)
	{
		g_malloc_buf_success = TRUE;
	}


	for (i =0; i < MAX_DECONDE_BUF_NUM; i++)
	{
		memset(g_show_video[i].m_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
		g_show_video[i].m_receive_buf_len = 0;
		g_show_video[i].m_empty_flag = TRUE;
	}
	g_write_current_buf_idex = 0;
	g_read_current_buf_idex = 0;
	memset(g_one_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);

	g_recode_video = TRUE;
	//get_video_test();
	//AfxMessageBox(_T("已启动视频"), MB_OK);

	return TRUE;
}


void CAnykaIPCameraDlg::Close_video_net(UINT idex) 
{
	UINT i = 0;

	if (!g_connet_flag)
	{
		AfxMessageBox(_T("还没有启动视频"), MB_OK);
		return;
	}

	//g_file_fp.Close();
	//close_file();


	//发送停止视频命令
	if(!Send_video_cmd(TYPE_VIDEO_STOP, NULL, 0, 0))
	{
		AfxMessageBox(_T("发送停止视频命令失败"), MB_OK);
	}
	
	if (g_test_config.server_flag)
	{
		g_malloc_buf_success = FALSE;
		close_rev_video_data_thread(idex);
		m_ClientSocket.Socket_Close(idex);
		g_connet_flag = FALSE;
	}

	for (i =0; i < MAX_DECONDE_BUF_NUM; i++)
	{
		memset(g_show_video[i].m_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
		g_show_video[i].m_receive_buf_len = 0;
		g_show_video[i].m_empty_flag = TRUE;
	}
	g_write_current_buf_idex = 0;
	g_read_current_buf_idex = 0;
	memset(g_one_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);

	g_recode_video = FALSE;

	//AfxMessageBox(_T("已停止视频"), MB_OK);
	

}


void CAnykaIPCameraDlg::set_video_param(void) 
{
	CString str;
	BOOL close_flag = FALSE;

	USES_CONVERSION;

	if (g_test_config.video_parm.video_size > TYPE_VIDEO_960P)
	{
		AfxMessageBox(_T("视频分辨率有错"), MB_OK);
		return;
	}

	m_RightDlg.GetDlgItemText(IDC_EDIT_FRAME, str);
	if(str.IsEmpty())
	{
		AfxMessageBox(_T("视频帧率为空"), MB_OK);
		return;
	}
	
	g_test_config.video_parm.frame = atoi(T2A(str));

	if (g_test_config.video_parm.frame > 30 || g_test_config.video_parm.frame < 1)
	{
		AfxMessageBox(_T("视频帧率有错"), MB_OK);
		return;
	}

	m_RightDlg.GetDlgItemText(IDC_EDIT_VIDEO_FRAME, str);
	if(str.IsEmpty())
	{
		AfxMessageBox(_T("视频码率为空"), MB_OK);
		return;
	}

	g_test_config.video_parm.data_rate = atoi(T2A(str));

	if (g_test_config.video_parm.frame == 0)
	{
		AfxMessageBox(_T("视频码率有错"), MB_OK);
		return;
	}

	if (g_test_config.server_flag)
	{
		if (!g_connet_flag)
		{
			if (!connet_net(g_test_config.m_ip_addr, g_test_config.m_ip_port, 0))
			{
				return;
			}
			close_flag = TRUE;
		}
		else
		{
			close_flag = FALSE;
		}
	}
	
	//发送设置视频参数命令
	if(!Send_video_cmd(TYPE_VIDEO_PARM, (char *)&g_test_config.video_parm, sizeof(T_VIDEO_CONFIG), 0))
	{
		AfxMessageBox(_T("发送设置视频参数命令失败"), MB_OK);
	}
	else
	{
		AfxMessageBox(_T("设置视频参数成功"), MB_OK);
	}
	
	if (g_test_config.server_flag)
	{
		if (close_flag)
		{
			close_rev_video_data_thread(0);
			m_ClientSocket.Socket_Close(0);
			g_connet_flag = FALSE;
		}
	}
	
}



BOOL CAnykaIPCameraDlg::Send_video_cmd(char commad_type, char *param, UINT param_len, UINT idex)
{
	char *lpBuf = NULL;
	char *head_name = "VCMD";
	char *end_name = "CEND";
	short nBufLen = 0;
	UINT i = 0;
	char len_data = 0;
	UINT check_sum = 0;
	int ret = 0;

	//命令头标识4 命令ID1 命令长度1 命令参数 	CRC校验 4	命令尾标识 4

	USES_CONVERSION;

	nBufLen = 4;//头
	nBufLen += 1;//ID

	if (param != NULL)
	{
		len_data = param_len;
		nBufLen += 1;//长度
		nBufLen += len_data;//数据
	}
	else
	{
		nBufLen += 1;//长度
	}

	nBufLen +=  4;//check sum
	nBufLen +=  4;//结束


	lpBuf = (char *)malloc(nBufLen + 1);
	if (lpBuf == NULL)
	{
		return FALSE;
	}

	//打包
	memset(lpBuf, 0, nBufLen + 1);
	memcpy(lpBuf, head_name, 4);
	memcpy(&lpBuf[4], (char *)&commad_type, 1);
	memcpy(&lpBuf[4+1], (char *)&len_data, 1);

	if (len_data != 0 && param != NULL)
	{
		memcpy(&lpBuf[4+1+1], param, len_data);;
	}

	for (i = 0; i < (UINT)(len_data + 1 + 1); i++)
	{
		check_sum += lpBuf[i + 4];
	}


	memcpy(&lpBuf[4 + 1 + 1 +len_data], (char *)&check_sum, 4);

	memcpy(&lpBuf[4 + 1 + 1 +len_data+4], end_name, 4);

	if (g_test_config.server_flag)
	{
		ret = m_ClientSocket.Socket_Send(lpBuf, nBufLen, idex);
	}
	else
	{
		ret = m_ClientSocket.Socket_server_Send(lpBuf, nBufLen);
	}
	//fprintf(stderr, "Socket_Send:%d\n", ret);
	if(!ret)
	{
		free(lpBuf);
		return FALSE;
	}
	free(lpBuf);

	return TRUE;
}

#if 1
BOOL CAnykaIPCameraDlg::Send_audio_data(char *buf, UINT buf_len)
{
	char *lpBuf = NULL;
	char *head_name = "FRAM";
	char *end_name = "FEND";
	UINT nBufLen = 0;
	UINT frame_type = FRAME_TYPE_AUDIO;
	UINT i = 0;
	UINT len_data = 0;
	UINT check_sum = 0;
	UINT time = 0;
	int ret = 0;
	UINT idex= 0;

	//包头标识4Bytes  帧类型4Bytes	时间戳4Bytes 帧长度4Bytes 音频数据	CRC校验4Bytes	包尾标识4Bytes


	USES_CONVERSION;

	nBufLen = 4;//头

	nBufLen += 4;//帧类型
	nBufLen += 4;//时间戳
	//fprintf(stderr, "buf_len:%d \n", buf_len);

	if (buf != NULL)
	{
		len_data = buf_len;
		nBufLen += 4;//长度
		nBufLen += len_data;//数据
		//fprintf(stderr, "nBufLen:%d \n", nBufLen);
	}
	else
	{
		AfxMessageBox(_T("audio data buf is null"), MB_OK);
		return FALSE;
	}

	nBufLen +=  4;//check sum
	nBufLen +=  4;//结束


	lpBuf = (char *)malloc(nBufLen + 1);
	if (lpBuf == NULL)
	{
		return FALSE;
	}

	//打包
	memset(lpBuf, 0, nBufLen + 1);
	memcpy(lpBuf, head_name, 4);
	memcpy(&lpBuf[4], &frame_type, 4);
	memcpy(&lpBuf[8], &time, 4);
	memcpy(&lpBuf[12], (char *)&len_data, 4);

	if (len_data != 0 && buf != NULL)
	{
		memcpy(&lpBuf[16], buf, len_data);;
	}

	for (i = 0; i < (UINT)(len_data + 4); i++)
	{
		check_sum += lpBuf[i + 4];
	}


	memcpy(&lpBuf[16 +len_data], (char *)&check_sum, 4);

	memcpy(&lpBuf[16 +len_data+4], end_name, 4);

	//fprintf(stderr, "Socket_server_Send  nBufLen:%d \n", nBufLen);

	//fprintf(stderr, "Socket_server_Send  data:%02x, %02x, %02x, %02x \n", lpBuf[0], lpBuf[1], lpBuf[2], lpBuf[3]);
	//fprintf(stderr, "Socket_server_Send  data:%02x, %02x, %02x, %02x \n", lpBuf[4], lpBuf[5], lpBuf[6], lpBuf[7]);
	//fprintf(stderr, "Socket_server_Send  data:%02x, %02x, %02x, %02x \n", lpBuf[8], lpBuf[9], lpBuf[10], lpBuf[11]);

	if (g_test_config.server_flag)
	{
		ret = m_ClientSocket.Socket_Send(lpBuf, nBufLen, idex);
	}
	else
	{
		ret = m_ClientSocket.Socket_server_Send(lpBuf, nBufLen);
	}
	//fprintf(stderr, "Socket_server_Send end \n");

	free(lpBuf);
	if(!ret)
	{
		return FALSE;
	}

	return TRUE;
}
#else

BOOL CAnykaIPCameraDlg::Send_audio_data(char *buf, UINT buf_len)
{
	char *lpBuf = NULL;
	char *head_name = "AENC";
	char *end_name = "AEND";
	UINT nBufLen = 0;
	UINT i = 0;
	UINT len_data = 0;
	UINT check_sum = 0;
	int ret = 0;
	UINT idex= 0;

	//包头标识4Bytes	帧长度4Bytes	音频数据	CRC校验4Bytes	包尾标识4Bytes

	USES_CONVERSION;

	nBufLen = 4;//头

	//fprintf(stderr, "buf_len:%d \n", buf_len);

	if (buf != NULL)
	{
		len_data = buf_len;
		nBufLen += 4;//长度
		nBufLen += len_data;//数据
		//fprintf(stderr, "nBufLen:%d \n", nBufLen);
	}
	else
	{
		AfxMessageBox(_T("audio data buf is null"), MB_OK);
		return FALSE;
	}

	nBufLen +=  4;//check sum
	nBufLen +=  4;//结束


	lpBuf = (char *)malloc(nBufLen + 1);
	if (lpBuf == NULL)
	{
		return FALSE;
	}

	//打包
	memset(lpBuf, 0, nBufLen + 1);
	memcpy(lpBuf, head_name, 4);
	memcpy(&lpBuf[4], (char *)&len_data, 4);

	if (len_data != 0 && buf != NULL)
	{
		memcpy(&lpBuf[4+4], buf, len_data);;
	}

	for (i = 0; i < (UINT)(len_data + 4); i++)
	{
		check_sum += lpBuf[i + 4];
	}


	memcpy(&lpBuf[4 +4 +len_data], (char *)&check_sum, 4);

	memcpy(&lpBuf[4 + 4 +len_data+4], end_name, 4);

	//fprintf(stderr, "Socket_server_Send  nBufLen:%d \n", nBufLen);

	//fprintf(stderr, "Socket_server_Send  data:%02x, %02x, %02x, %02x \n", lpBuf[0], lpBuf[1], lpBuf[2], lpBuf[3]);
	//fprintf(stderr, "Socket_server_Send  data:%02x, %02x, %02x, %02x \n", lpBuf[4], lpBuf[5], lpBuf[6], lpBuf[7]);
	//fprintf(stderr, "Socket_server_Send  data:%02x, %02x, %02x, %02x \n", lpBuf[8], lpBuf[9], lpBuf[10], lpBuf[11]);

	if (g_test_config.server_flag)
	{
		ret = m_ClientSocket.Socket_Send(lpBuf, nBufLen, idex);
	}
	else
	{
		ret = m_ClientSocket.Socket_server_Send(lpBuf, nBufLen);
	}
	//fprintf(stderr, "Socket_server_Send end \n");

	free(lpBuf);
	if(!ret)
	{
		return FALSE;
	}

	return TRUE;
}
#endif

CFile g_test_fp;

BOOL CAnykaIPCameraDlg::creat_file(void)
{

	g_test_fp.Open(_T("frame_test.txt"),CFile::modeCreate|CFile::modeNoTruncate|CFile::modeReadWrite);
	return TRUE;

}

BOOL CAnykaIPCameraDlg::write_file(TCHAR *buf)
{
	CString str;

	TCHAR serial_temp[100] = {0};


	UINT len = _tcslen(buf);
	memset(serial_temp,0, 100 );	
	_tcsncpy(serial_temp, buf, len);
	//_tcsncat(&serial_temp[len], _T("\r\n"), 2);
	serial_temp[len] = 0x0D;
	serial_temp[len + 1] = 0x0A;

	g_test_fp.SeekToEnd();
	g_test_fp.Write(serial_temp,len + 2);	
	return TRUE;

}

BOOL CAnykaIPCameraDlg::close_file(void)
{
	g_test_fp.Close();
	return TRUE;
}


BOOL CAnykaIPCameraDlg::start_save_video(void)
{
	UINT i = 0;
	
	close_play_file();
	if (!open_play_file(CFile::modeCreate|CFile::modeReadWrite))//g_play_fp.Open(g_test_config.m_video_path,CFile::modeRead))
	{
		//AfxMessageBox(_T("error: open play video file fail"), MB_OK);
		return FALSE;
	}

	g_save_flag = TRUE;
	
	return TRUE;
}


BOOL CAnykaIPCameraDlg::stop_save_video(void)
{
	UINT i = 0;

	
	g_save_flag = FALSE;
	close_play_file();

	return TRUE;

}
#if 1

UINT g_video_idex = 0;
UINT g_audio_idex = 0;

#if 0
UINT g_frame_incomplete = 0, g_header_incomplete = 0;
UINT g_header_left = 0, g_header_get = 0;
BOOL CAnykaIPCameraDlg::decode_video_date(char *lpBuf, UINT nBufLen)
{
	UINT i = 0;
	
	if (g_frame_incomplete == 0)
	{
		for(i = 0; i < (UINT)nBufLen; i++)
		{
			if(g_header_get == 0)
			{
				if(lpBuf[i] == 'F' && lpBuf[i + 1] == 'R' && lpBuf[i + 2] == 'A' && lpBuf[i + 3] == 'M') 
				{
					break;
				}
				else
				{
					continue;
				}
			}
		}
	}
	else
	{
		//不是一个完整的包

	}

}
#endif

BOOL CAnykaIPCameraDlg::server_rev_video_date(void)
{
	CAnykaIPCameraDlg  TestToolDlg;
	CClientSocket m_ClientSocket;
	char *lpBuf = NULL;
	UINT nBufLen = VIDEO_RECEIVE_BUFFER_SIZE;   //4 + 4 + VIDEO_RECEIVE_BUFFER_SIZE + 4 + 4;
	CString str;
	int len, idex = 0;
	UINT frame_len, frame_count;
	UINT frame_type = 0;
	UINT frame_incomplete = 0, header_incomplete = 0;
	char saved_header[16+1];
	UINT header_left = 0, header_get  = 0;
	UINT i, count = 0, buf_idex = 0, read_buf_len = 0;
	UINT incomplete_buflen = 0;
	UINT incomplete_end_buflen = 0;
	UINT incomplete_framelen = 0;
	BOOL first_flag = TRUE;
	char video_data_flag = 0;  //0表示视频，　１表示音频，　２表示图像
	UINT check_sum = 0;
	UINT data_idex = 0;
	UINT pre_data_idex = 0;
	UINT data_len_idex = 0;
	BOOL nouse_frame_frame = FALSE;
	UINT check_len = 0, j = 0;
	//TCHAR fram_num[20] ={0};


	//包头标识4Bytes	帧类型4Bytes 时间戳4Bytes 帧长度4Bytes	视频帧数据	CRC校验4Bytes	包尾标识4Bytes

	header_incomplete  = 0;
	frame_incomplete   = 0;

	lpBuf = new char[nBufLen + 1];
	if (lpBuf == NULL)
	{
		AfxMessageBox(_T("malloc receive buf fail"), MB_OK);
		return -1;
	}


	TCHAR path[256] = {0};
#if 0
	swprintf(path, _T("D:\\first_test.dat"));
	if (!g_file_fp_test.Open(path,CFile::modeCreate|CFile::modeReadWrite))
	{
		AfxMessageBox(_T("error: open the data stream fail"), MB_OK);
		return FALSE;
	}

#else
	//swprintf(path, _T("I:\\first_test.txt"));
	//if (!g_file_fp_test.Open(path,CFile::modeRead))
	//{
	//	AfxMessageBox(_T("error: open the data stream fail"), MB_OK);
	//	return FALSE;
	//}
#endif

	//获取心跳命令
	while (1)
	{
		//Sleep(100);
		if (g_hBurnThread_server != INVALID_HANDLE_VALUE )
		{
			memset(lpBuf,0,nBufLen + 1);

			if (g_test_config.server_flag)
			{
				break;
			}
			//fprintf(stderr, "start 00\n");
#if 0
			CFile fp;
			TCHAR path[256] = {0};
			swprintf(path, _T("F:\\first_test_%d"), idex);
			fp.Open(path,CFile::modeReadWrite);
			len = fp.GetLength();
			//读取一帧数据
			fp.SeekToBegin();
			fp.Read(lpBuf,len);

			fp.Close();
			idex++;
#else		
			//fprintf(stderr, "start\n");
			len = m_ClientSocket.Socket_server_Receive(lpBuf, nBufLen);
	        //len = g_file_fp_test.Read(lpBuf,10*1024);
			//data_len_idex += len;

			//if (len == 0)
			//{
			//	g_file_fp_test.Close();
			//	data_len_idex++;
			//}
#endif
			if (len == -1)
			{
				fprintf(stderr, "error 11\n");
				//AfxMessageBox(_T("获取数据出错"), MB_OK);
				break;
			}
			else
			{
				//fprintf(stderr, "end\n");
#if 0
				if (1)//first_flag)
				{
					TCHAR path[256] = {0};

					swprintf(path, _T("L:\\first_frame\\first_test_%d"), idex);
					if (!g_file_fp_test.Open(path,CFile::modeCreate|CFile::modeReadWrite))
					{
						AfxMessageBox(_T("error: open the data stream fail"), MB_OK);
						return FALSE;
					}
					g_file_fp_test.Write(lpBuf,len);
					g_file_fp_test.Close();
					first_flag = FALSE;
					idex++;
				}
#endif	

				//g_file_fp_test.Write(lpBuf,len);

				//if (len > VIDEO_RECEIVE_BUFFER_SIZE)
				//{
				//	fprintf(stderr, "len:%d \n",len);
				//}
				
				//continue;
			}

			if(len > 0 && (UINT)len <= nBufLen)//4 + 4 + VIDEO_RECEIVE_BUFFER_SIZE + 4 + 4)
			{
				frame_count = 0;
				/*parse data check header  "VCMD" + cmd id + cmd arg len + cmd arg + crc + "CEND"*/
				for(i = 0; i < (UINT)len; i++)
				{
					header_get = 0;

					
					if(i == len)
					{
						 break;
					}

					if (frame_count == 0 && frame_incomplete == 1 )
					{
						if (len > 4)
						{
							if (incomplete_framelen - incomplete_buflen + 8 > len)
							{
								check_len = len;
							}
							else
							{
								check_len = incomplete_framelen - incomplete_buflen + 8;
							}
							for (j = i; j < check_len - 4; j++)
							{
								if(lpBuf[j] == 'F' && lpBuf[j + 1] == 'R' && lpBuf[j + 2] == 'A' && lpBuf[j + 3] == 'M') 
								{
									nouse_frame_frame = TRUE;
									i = j - 1;//从这一帧开始
									break;
								}
							}

							if (nouse_frame_frame)
							{
								fprintf(stderr, "no all frame len:%d, data_idex:%d\n", incomplete_framelen, data_idex);
								nouse_frame_frame = FALSE;
								frame_incomplete = 0;
								continue;
							}
						}
					}
					/*the previous data may the remaining for the last frame*/
					if(frame_count == 0 && frame_incomplete == 1 )
					{
						if (incomplete_framelen - incomplete_buflen + 8 > (UINT)len)
						{
							if (video_data_flag == DATA_TYPE_VIDEO)
							{
								memcpy(&g_one_video_receive_buf[incomplete_buflen], lpBuf, len);
							} 
							else if(video_data_flag == DATA_TYPE_AUDIO)
							{
								memcpy(&g_one_audio_receive_buf[incomplete_buflen], lpBuf, len);
							}
							else
							{
								if (g_check_picture_flag)
								{
									memcpy(&g_picture_buf[incomplete_buflen], lpBuf, len);	
								}
								
							}

							incomplete_buflen += len;
							break;
						}
						else
						{
							/*cp the remaining frame to the compose the whole frame*/
							frame_incomplete = 0;

							if (video_data_flag == DATA_TYPE_VIDEO)
							{
								//后半帧
								if (incomplete_framelen > incomplete_buflen)
								{
									memcpy(&g_one_video_receive_buf[incomplete_buflen], lpBuf, incomplete_framelen - incomplete_buflen);
								}


								//一帧数据
								while(1)
								{
									if (g_show_video[g_write_current_buf_idex].m_empty_flag == TRUE)
									{
										break;
									}
									//break;
									//fprintf(stderr, "ddd\n");
									//Sleep(2);
								}
#if 1
								memset(g_show_video[g_write_current_buf_idex].m_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
								memcpy((char *)g_show_video[g_write_current_buf_idex].m_video_receive_buf, g_one_video_receive_buf, incomplete_framelen);
								if(g_save_flag)
								{
									TestToolDlg.creat_video_file(&incomplete_framelen, 4);
									TestToolDlg.creat_video_file(g_show_video[g_write_current_buf_idex].m_video_receive_buf, incomplete_framelen);
								}

								g_show_video[g_write_current_buf_idex].m_receive_buf_len = incomplete_framelen;
								g_show_video[g_write_current_buf_idex].m_empty_flag = FALSE;

								g_write_current_buf_idex++;
								if (g_write_current_buf_idex == MAX_DECONDE_BUF_NUM)
								{
									g_write_current_buf_idex = 0;
								}
#endif
							}
							else if (video_data_flag == DATA_TYPE_AUDIO)//音频
							{
								//fprintf(stderr, "last part frame data\n");
								//后半帧
								//fprintf(stderr, "1g_audio_write_current_buf_idex:%d \n",g_audio_write_current_buf_idex);
								if (incomplete_framelen > incomplete_buflen)
								{
									memcpy(&g_one_audio_receive_buf[incomplete_buflen], lpBuf, incomplete_framelen - incomplete_buflen);
								}

								//fprintf(stderr, "2g_audio_write_current_buf_idex:%d, %d\n", g_audio_write_current_buf_idex, g_play_data[g_audio_write_current_buf_idex].m_empty_flag);
#if 1
								//一帧数据
								while(1)
								{
									if (g_play_data[g_audio_write_current_buf_idex].m_empty_flag == TRUE)
									{
										break;
									}
									//break;
									//fprintf(stderr, "ccc\n");
									//fprintf(stderr, "data 33\n");

									Sleep(1);
								}
#endif

								//fprintf(stderr, "audio  frame_len len:%d \n",incomplete_framelen);
#if 1
								memset(g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, 0, AUDIO_RECEIVE_BUFFER_SIZE);
								memcpy((char *)g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, g_one_audio_receive_buf, incomplete_framelen);
								if(g_save_flag)
								{
									//TestToolDlg.creat_video_file(&incomplete_framelen, 4);
									//TestToolDlg.creat_video_file(g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, incomplete_framelen);
								}

								g_play_data[g_audio_write_current_buf_idex].m_receive_buf_len = incomplete_framelen;
								g_play_data[g_audio_write_current_buf_idex].m_empty_flag = FALSE;

								//fprintf(stderr, "3g_audio_write_current_buf_idex:%d, %d\n", g_audio_write_current_buf_idex, g_play_data[g_audio_write_current_buf_idex].m_empty_flag);


								g_audio_write_current_buf_idex++;
								if (g_audio_write_current_buf_idex == MAX_AUDIO_BUF_NUM)
								{
									g_audio_write_current_buf_idex = 0;
								}
#endif

							}
							else //图像
							{
								if(g_check_picture_flag)
								{
									g_check_picture_flag = FALSE;
									memcpy(&g_picture_buf[incomplete_buflen], lpBuf, incomplete_framelen - incomplete_buflen);
									g_picture_len = incomplete_framelen;
									g_get_picture_flag = TRUE;
								}
									
							}

							i += incomplete_framelen - incomplete_buflen + 8 - incomplete_end_buflen; //加上check_sum和包尾

							if (len == i)
							{
								break;
							}
						}

					}

					/*remain data less than a frame header*/
					if((len - i) < 16 && header_incomplete == 0)
					{
						/*save this packet header and recv next packet*/
						header_incomplete = 1;
						header_left = 16 - (len - i);
						memcpy(saved_header, &lpBuf[i], len - i); 
						break;
					}


					if(header_incomplete==1 && len - i >= header_left)
					{
						header_incomplete = 0;
						/*complete the header and parse*/
						memcpy(&saved_header[16 - header_left], lpBuf, header_left);
						i += header_left;
						if(saved_header[0] == 'F' && saved_header[1] == 'R' && saved_header[2] == 'A' && saved_header[3] == 'M') 
						{
							header_get = 1;
							frame_type = *(int*)&saved_header[4];
							if(g_check_picture_flag)
							{
								g_time_seconds = *(int*)&saved_header[8];
							}
							
							frame_len = *(int*)&saved_header[12];
							//fprintf(stderr, "data:%02x,%02x,%02x,%02x,  %02x,%02x,%02x,%02x,  %02x,%02x,%02x,%02x\n"
							//	, saved_header[4], saved_header[5], saved_header[6], saved_header[7], saved_header[8], saved_header[9], saved_header[10], saved_header[11], saved_header[12], saved_header[13], saved_header[14], saved_header[15]);
							//fprintf(stderr, "00frame_type:%d %d, %d\n", frame_type, frame_len, g_time_seconds);
							if (frame_type == FRAME_TYPE_AUDIO)
							{
								video_data_flag = DATA_TYPE_AUDIO;
							}
							else if(frame_type == FRAME_TYPE_PICTURE)
							{
								video_data_flag = DATA_TYPE_PICTURE;
							}
							else
							{
								video_data_flag = DATA_TYPE_VIDEO;
							}

							data_idex = *(int*)&saved_header[8];
							goto FRAME_COPY;
						}
						else
						{
							/*go ahead to next frame*/
							count++;
						}

					}

					if(header_get == 0)
					{
						if(lpBuf[i] == 'F' && lpBuf[i + 1] == 'R' && lpBuf[i + 2] == 'A' && lpBuf[i + 3] == 'M') 
						{
						}
						else
						{
							continue;
						}
					}
					//fprintf(stderr, "data:%02x,%02x,%02x,%02x,  %02x,%02x,%02x,%02x,  %02x,%02x,%02x,%02x\n"
					//	, lpBuf[i+4], lpBuf[i+5], lpBuf[i+6], lpBuf[i+7], lpBuf[i+8], lpBuf[i+9], lpBuf[i+10], lpBuf[i+11], lpBuf[i+12], lpBuf[i+13], lpBuf[i+14], lpBuf[i+15]);
					i += 4;	//包头
					frame_type = *(int*)&lpBuf[i];
					if (frame_type == FRAME_TYPE_AUDIO)
					{
						video_data_flag = DATA_TYPE_AUDIO;
					}
					else if(frame_type == FRAME_TYPE_PICTURE)
					{
						video_data_flag = DATA_TYPE_PICTURE;
					}
					else
					{
						video_data_flag = DATA_TYPE_VIDEO;
					}

					header_incomplete = 0;
					/*the last frame exceeded this recvbuf save this incomplete frame*/
					i += 4; //包类型
					if(g_check_picture_flag)
					{
						g_time_seconds = *(int*)&lpBuf[i];
					}
					data_idex = *(int*)&lpBuf[i];

					i += 4; //时间戳
					frame_len = *(int*)&lpBuf[i];
					i += 4; //包长

					//fprintf(stderr, "11frame_type:%d, %d, %d\n", frame_type, frame_len, g_time_seconds);

FRAME_COPY:
					//检查这一帧内目前的数据是否有其他帧头，以防出现网络传漏数据
					//如果有其他帧头，那么就把上一个帧丢掉，获取下一帧
					if(frame_len + 8 > (len - i)) 
					{
						check_len = len - i;
					}
					else
					{
						check_len = frame_len + 8;
					}

					if (check_len < 4)
					{
						continue;
					}

					for (j = i; j < check_len - 4; j++)
					{
						if(lpBuf[j] == 'F' && lpBuf[j + 1] == 'R' && lpBuf[j + 2] == 'A' && lpBuf[j + 3] == 'M') 
						{
							fprintf(stderr, "no all frame len:%d, data_idex:%d\n", frame_len, data_idex);
							nouse_frame_frame = TRUE;
							i = j - 1;//从这一帧开始
							break;
						}
					}


					if (!nouse_frame_frame)
					{
						if(frame_len + 8 > (len - i)) 
						{
							/*save this incomplete frame*/
							frame_incomplete = 1;
							//前半帧
							if (frame_len > (len - i))
							{
								if (video_data_flag == DATA_TYPE_VIDEO)
								{
									memcpy(g_one_video_receive_buf, &lpBuf[i], len - i);
								}
								else if(video_data_flag == DATA_TYPE_AUDIO)
								{
									memcpy(g_one_audio_receive_buf, &lpBuf[i], len - i);
								}
								else //图像
								{
									if(g_check_picture_flag)
									{
										memset(g_picture_buf, 0, MAX_PICTRUEN_LEN);
										memcpy(g_picture_buf, &lpBuf[i], len - i);
									}
								}

								incomplete_buflen =  len - i;
								incomplete_end_buflen =	0;
							}
							else
							{
								//fprintf(stderr, "one part frame data\n");

								if (video_data_flag == DATA_TYPE_VIDEO)
								{
									memcpy(g_one_video_receive_buf, &lpBuf[i], frame_len);
								}
								else if(video_data_flag == DATA_TYPE_AUDIO)
								{
									memcpy(g_one_audio_receive_buf, &lpBuf[i], frame_len);
								}
								else //图像
								{
									if(g_check_picture_flag)
									{
										memset(g_picture_buf, 0, MAX_PICTRUEN_LEN);
										memcpy(g_picture_buf, &lpBuf[i], frame_len);
									}
								}

								incomplete_buflen =  frame_len;
								incomplete_end_buflen =	len - i - frame_len;
							}


							incomplete_framelen = frame_len;

							break;

						}
						else
						{
							/*cp frame data to decodeer*/
							frame_count++; 
							i += frame_len;  //

							if (video_data_flag == DATA_TYPE_VIDEO)
							{
								//一帧数据
								while(1)
								{
									if (g_show_video[g_write_current_buf_idex].m_empty_flag == TRUE)
									{
										break;
									}
									//break;
									//fprintf(stderr, "aaa\n");	
									//Sleep(2);

									//break;
								}
#if 1
								memset(g_show_video[g_write_current_buf_idex].m_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
								memcpy((char *)g_show_video[g_write_current_buf_idex].m_video_receive_buf, &lpBuf[i - frame_len], frame_len);
								if(g_save_flag)
								{
									TestToolDlg.creat_video_file(&frame_len, 4);
									TestToolDlg.creat_video_file(g_show_video[g_write_current_buf_idex].m_video_receive_buf, frame_len);
								}
								g_show_video[g_write_current_buf_idex].m_receive_buf_len = frame_len;
								g_show_video[g_write_current_buf_idex].m_empty_flag = FALSE;

								g_write_current_buf_idex++;
								if (g_write_current_buf_idex == MAX_DECONDE_BUF_NUM)
								{
									g_write_current_buf_idex = 0;
								}
#endif
							}
							else if(video_data_flag == DATA_TYPE_AUDIO)
							{
								//一帧数据
								//fprintf(stderr, "0g_audio_write_current_buf_idex:%d, %d\n", g_audio_write_current_buf_idex, g_play_data[g_audio_write_current_buf_idex].m_empty_flag);
							
								while(1)
								{
									if (g_play_data[g_audio_write_current_buf_idex].m_empty_flag == TRUE)
									{
										break;
									}
									//break;
									//fprintf(stderr, "bbb\n");
									//fprintf(stderr, "data 00\n");
									Sleep(1);
								}
#if 1
								memset(g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, 0, AUDIO_RECEIVE_BUFFER_SIZE);
								memcpy((char *)g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, &lpBuf[i - frame_len], frame_len);
								g_play_data[g_audio_write_current_buf_idex].m_receive_buf_len = frame_len;
								g_play_data[g_audio_write_current_buf_idex].m_empty_flag = FALSE;

								//fprintf(stderr, "1g_audio_write_current_buf_idex:%d, %d\n", g_audio_write_current_buf_idex, g_play_data[g_audio_write_current_buf_idex].m_empty_flag);


								g_audio_write_current_buf_idex++;
								if (g_audio_write_current_buf_idex == MAX_AUDIO_BUF_NUM)
								{
									g_audio_write_current_buf_idex = 0;
								}
#endif
								//fprintf(stderr, "3g_audio_write_current_buf_idex:%d \n",g_audio_write_current_buf_idex);
							}
							else //图像
							{
								if(g_check_picture_flag)
								{
									g_check_picture_flag =FALSE;
									memset(g_picture_buf, 0, MAX_PICTRUEN_LEN);
									memcpy(g_picture_buf, &lpBuf[i - frame_len], frame_len);
									g_picture_len = frame_len;
									g_get_picture_flag = TRUE;
								}
							}

							i += 7; //加上check_sum和包尾
						}
					}
					else
					{
						nouse_frame_frame = FALSE;
					}
				}
			}
			//fprintf(stderr, "data 11\n");
		}
		else
		{
			break;
		}


	}

	delete[] lpBuf;

	//g_file_fp_test.Close();

	return 1;
}

#else

BOOL CAnykaIPCameraDlg::server_rev_video_date(void)
{
	CAnykaIPCameraDlg  TestToolDlg;
	CClientSocket m_ClientSocket;
	char *lpBuf = NULL;
	UINT nBufLen = VIDEO_RECEIVE_BUFFER_SIZE;   //4 + 4 + VIDEO_RECEIVE_BUFFER_SIZE + 4 + 4;
	CString str;
	int len;
	UINT frame_len, frame_count;
	UINT frame_incomplete = 0, header_incomplete = 0;
	char saved_header[8];
	UINT header_left = 0, header_get  = 0;
	UINT i, count = 0, buf_idex = 0, read_buf_len = 0;
	UINT incomplete_buflen = 0;
	UINT incomplete_end_buflen = 0;
	UINT incomplete_framelen = 0;
	BOOL first_flag = TRUE;
	BOOL video_data_flag = TRUE;
	//TCHAR fram_num[20] ={0};


	header_incomplete  = 0;
	frame_incomplete   = 0;

	lpBuf = new char[nBufLen + 1];
	if (lpBuf == NULL)
	{
		AfxMessageBox(_T("malloc receive buf fail"), MB_OK);
		return -1;
	}

	//获取心跳命令
	while (1)
	{
		//Sleep(100);
		if (g_hBurnThread_server != INVALID_HANDLE_VALUE )
		{
			memset(lpBuf,0,nBufLen + 1);

			if (g_test_config.server_flag)
			{
				break;
			}
			//fprintf(stderr, "start 00\n");
			len = m_ClientSocket.Socket_server_Receive(lpBuf, nBufLen);
			if (len == -1)
			{
				//fprintf(stderr, "error 11\n");
				//AfxMessageBox(_T("获取数据出错"), MB_OK);
				break;
			}
			else
			{
				//fprintf(stderr, "end 11\n");
#if 0
				if (first_flag)
				{
					if (!g_file_fp_test.Open(TEXT("F:\\first_test.dat"),CFile::modeCreate|CFile::modeReadWrite))
					{
						AfxMessageBox(_T("error: open the data stream fail"), MB_OK);
						return FALSE;
					}
					g_file_fp_test.Write(lpBuf,len);
					g_file_fp_test.Close();
					first_flag = FALSE;
				}
#endif	
				//fprintf(stderr, "len:%d \n",len);
				//continue;
			}


			if(len > 0 && (UINT)len <= nBufLen)//4 + 4 + VIDEO_RECEIVE_BUFFER_SIZE + 4 + 4)
			{
				frame_count = 0;
				/*parse data check header  "VCMD" + cmd id + cmd arg len + cmd arg + crc + "CEND"*/
				for(i = 0; i < (UINT)len; i++)
				{

					header_get = 0;
					/*the previous data may the remaining for the last frame*/
					if(frame_count == 0 && frame_incomplete == 1 )
					{
						if (incomplete_framelen - incomplete_buflen + 8 > (UINT)len)
						{
							if (video_data_flag)
							{
								memcpy(&g_one_video_receive_buf[incomplete_buflen], lpBuf, len);
							} 
							else
							{
								memcpy(&g_one_audio_receive_buf[incomplete_buflen], lpBuf, len);
							}
							
							incomplete_buflen += len;
							break;
						}
						else
						{
							/*cp the remaining frame to the compose the whole frame*/
							frame_incomplete = 0;
							
							if (video_data_flag)
							{
								//后半帧
								if (incomplete_framelen > incomplete_buflen)
								{
									memcpy(&g_one_video_receive_buf[incomplete_buflen], lpBuf, incomplete_framelen - incomplete_buflen);
								}


								//一帧数据
								while(1)
								{
									if (g_show_video[g_write_current_buf_idex].m_empty_flag == TRUE)
									{
										break;
									}
									//Sleep(2);
								}

								memset(g_show_video[g_write_current_buf_idex].m_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
								memcpy((char *)g_show_video[g_write_current_buf_idex].m_video_receive_buf, g_one_video_receive_buf, incomplete_framelen);
								if(g_save_flag)
								{
									TestToolDlg.creat_video_file(&incomplete_framelen, 4);
									TestToolDlg.creat_video_file(g_show_video[g_write_current_buf_idex].m_video_receive_buf, incomplete_framelen);
								}

								g_show_video[g_write_current_buf_idex].m_receive_buf_len = incomplete_framelen;
								g_show_video[g_write_current_buf_idex].m_empty_flag = FALSE;

								g_write_current_buf_idex++;
								if (g_write_current_buf_idex == MAX_DECONDE_BUF_NUM)
								{
									g_write_current_buf_idex = 0;
								}
							}
							else//音频
							{
								//fprintf(stderr, "last part frame data\n");
								//后半帧
								//fprintf(stderr, "1g_audio_write_current_buf_idex:%d \n",g_audio_write_current_buf_idex);
								if (incomplete_framelen > incomplete_buflen)
								{
									memcpy(&g_one_audio_receive_buf[incomplete_buflen], lpBuf, incomplete_framelen - incomplete_buflen);
								}

								//fprintf(stderr, "2g_audio_write_current_buf_idex:%d, %d\n", g_audio_write_current_buf_idex, g_play_data[g_audio_write_current_buf_idex].m_empty_flag);
#if 1
								//一帧数据
								while(1)
								{
									if (g_play_data[g_audio_write_current_buf_idex].m_empty_flag == TRUE)
									{
										break;
									}
									//fprintf(stderr, "data 33\n");

									Sleep(1);
								}
#endif

								//fprintf(stderr, "audio  frame_len len:%d \n",incomplete_framelen);

								memset(g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, 0, AUDIO_RECEIVE_BUFFER_SIZE);
								memcpy((char *)g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, g_one_audio_receive_buf, incomplete_framelen);
								if(g_save_flag)
								{
									//TestToolDlg.creat_video_file(&incomplete_framelen, 4);
									//TestToolDlg.creat_video_file(g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, incomplete_framelen);
								}

								g_play_data[g_audio_write_current_buf_idex].m_receive_buf_len = incomplete_framelen;
								g_play_data[g_audio_write_current_buf_idex].m_empty_flag = FALSE;

								//fprintf(stderr, "3g_audio_write_current_buf_idex:%d, %d\n", g_audio_write_current_buf_idex, g_play_data[g_audio_write_current_buf_idex].m_empty_flag);


								g_audio_write_current_buf_idex++;
								if (g_audio_write_current_buf_idex == MAX_AUDIO_BUF_NUM)
								{
									g_audio_write_current_buf_idex = 0;
								}

							}
							
							i += incomplete_framelen - incomplete_buflen + 8 - incomplete_end_buflen; //加上check_sum和包尾

							if (len == i)
							{
								break;
							}
						}

					}

					/*remain data less than a frame header*/
					if((len - i) < 8 && header_incomplete == 0)
					{
						/*save this packet header and recv next packet*/
						header_incomplete = 1;
						header_left = 8 - (len - i);
						memcpy(saved_header, &lpBuf[i], len - i); 
						break;
					}


					if(header_incomplete && len - i >= header_left)
					{
						header_incomplete = 0;
						/*complete the header and parse*/
						memcpy(&saved_header[8 - header_left], lpBuf, header_left);
						i += header_left;
						if((saved_header[0] == 'F' && saved_header[1] == 'R' && saved_header[2] == 'A' && saved_header[3] == 'M') 
							|| (saved_header[0] == 'A' && saved_header[1] == 'E' && saved_header[2] == 'N' && saved_header[3] == 'C'))
						{
							header_get = 1;
							frame_len = *(int*)&saved_header[4];
							goto FRAME_COPY;
						}
						else
						{
							/*go ahead to next frame*/
							count++;
						}

					}

					if(header_get == 0)
					{
						if(lpBuf[i] == 'F' && lpBuf[i + 1] == 'R' && lpBuf[i + 2] == 'A' && lpBuf[i + 3] == 'M') 
						{
							video_data_flag = TRUE;
						}
						else if(lpBuf[i] == 'A' && lpBuf[i + 1] == 'E' && lpBuf[i + 2] == 'N' && lpBuf[i + 3] == 'C') 
						{
							//fprintf(stderr, "aenc data\n");
							video_data_flag = FALSE;
						}
						else
						{
							continue;
						}


					}


					i += 4;	//包头
					frame_len = *(int*)&lpBuf[i];
					header_incomplete = 0;
					/*the last frame exceeded this recvbuf save this incomplete frame*/
					i += 4; //包长

FRAME_COPY:
					if(frame_len + 8 > (len - i)) 
					{
						/*save this incomplete frame*/
						frame_incomplete = 1;
						//前半帧
						if (frame_len > (len - i))
						{
							if (video_data_flag)
							{
								memcpy(g_one_video_receive_buf, &lpBuf[i], len - i);
							}
							else
							{
								memcpy(g_one_audio_receive_buf, &lpBuf[i], len - i);
							}
							
							incomplete_buflen =  len - i;
							incomplete_end_buflen =	0;
						}
						else
						{
							//fprintf(stderr, "one part frame data\n");

							if (video_data_flag)
							{
								memcpy(g_one_video_receive_buf, &lpBuf[i], frame_len);
							}
							else
							{
								memcpy(g_one_audio_receive_buf, &lpBuf[i], frame_len);
							}
							
							incomplete_buflen =  frame_len;
							incomplete_end_buflen =	len - i - frame_len;
						}

						incomplete_framelen = frame_len;

						break;

					}
					else
					{
						/*cp frame data to decodeer*/
						frame_count++; 
						i += frame_len;  //
						
						if (video_data_flag)
						{
							//一帧数据
							while(1)
							{
								if (g_show_video[g_write_current_buf_idex].m_empty_flag == TRUE)
								{
									break;
								}
								
								//Sleep(2);
							}

							memset(g_show_video[g_write_current_buf_idex].m_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
							memcpy((char *)g_show_video[g_write_current_buf_idex].m_video_receive_buf, &lpBuf[i - frame_len], frame_len);
							if(g_save_flag)
							{
								TestToolDlg.creat_video_file(&frame_len, 4);
								TestToolDlg.creat_video_file(g_show_video[g_write_current_buf_idex].m_video_receive_buf, frame_len);
							}
							g_show_video[g_write_current_buf_idex].m_receive_buf_len = frame_len;
							g_show_video[g_write_current_buf_idex].m_empty_flag = FALSE;

							g_write_current_buf_idex++;
							if (g_write_current_buf_idex == MAX_DECONDE_BUF_NUM)
							{
								g_write_current_buf_idex = 0;
							}
						}
						else
						{
							//fprintf(stderr, "frame data\n");
							//fprintf(stderr, "2g_audio_write_current_buf_idex:%d \n",g_audio_write_current_buf_idex);
							//一帧数据
							//fprintf(stderr, "0g_audio_write_current_buf_idex:%d, %d\n", g_audio_write_current_buf_idex, g_play_data[g_audio_write_current_buf_idex].m_empty_flag);
#if 1							
							while(1)
							{
								if (g_play_data[g_audio_write_current_buf_idex].m_empty_flag == TRUE)
								{
									break;
								}
								//fprintf(stderr, "data 00\n");
								Sleep(1);
							}
#endif

							

							memset(g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, 0, AUDIO_RECEIVE_BUFFER_SIZE);
							memcpy((char *)g_play_data[g_audio_write_current_buf_idex].m_video_receive_buf, &lpBuf[i - frame_len], frame_len);
							if(g_save_flag)
							{
								//TestToolDlg.creat_video_file(&frame_len, 4);
								//TestToolDlg.creat_video_file(g_show_video[g_write_current_buf_idex].m_video_receive_buf, frame_len);
							}
							g_play_data[g_audio_write_current_buf_idex].m_receive_buf_len = frame_len;
							g_play_data[g_audio_write_current_buf_idex].m_empty_flag = FALSE;

							//fprintf(stderr, "1g_audio_write_current_buf_idex:%d, %d\n", g_audio_write_current_buf_idex, g_play_data[g_audio_write_current_buf_idex].m_empty_flag);


							g_audio_write_current_buf_idex++;
							if (g_audio_write_current_buf_idex == MAX_AUDIO_BUF_NUM)
							{
								g_audio_write_current_buf_idex = 0;
							}
							//fprintf(stderr, "3g_audio_write_current_buf_idex:%d \n",g_audio_write_current_buf_idex);
						}
						
						i += 7; //加上check_sum和包尾
					}
				}
			}
			//fprintf(stderr, "data 11\n");
		}
		else
		{
			Sleep(100);
			break;
		}

		
	}

	delete[] lpBuf;

	return 1;
}
#endif


DWORD WINAPI tcp_server_thread(LPVOID lpParameter)
{
	CAnykaIPCameraDlg  TestToolDlg;
	CClientSocket m_ClientSocket;
	UINT i =0;

	int err = -1;

	USES_CONVERSION;

	while (1)
	{
		if (g_hBurnThread_server != INVALID_HANDLE_VALUE)
		{
			g_enter_flag = TRUE;

			if (m_ClientSocket.Socket_server_Create() == 0)
			{
				AfxMessageBox(_T("创建服务端的线程失败"), MB_OK);
			}
			else
			{
				//使server 能立即重用
				if (m_ClientSocket.Socket_server_setsockopt() != 0) 
				{
					AfxMessageBox(_T("服务端的setsockopt fail"), MB_OK);
				}
				else
				{
					if (m_ClientSocket.Socket_server_Bind(T2A(g_test_config.m_ip_addr), g_test_config.m_ip_port) != 1)
					{
						AfxMessageBox(_T("服务端的Bind fail"), MB_OK);
					}
					else
					{
						if (m_ClientSocket.Socket_server_Listen(4) != 1 )
						{
							AfxMessageBox(_T("服务端的 Listen fail"), MB_OK);
						}
						else
						{
							while(1)
							{
								if (g_test_config.server_flag)
								{
									break;
								} 
								fprintf(stderr, "Socket_server_Accept\n");
								if (m_ClientSocket.Socket_server_Accept() != 1)
								{
								}
								else
								{
									g_recode_video = TRUE;
									g_connet_flag = TRUE;
									TestToolDlg.close_play_first_frame_thread();
									TestToolDlg.close_play_video_thread();
									if (g_test_config.save_video_enable)
									{
										//创建保存文件的线程
										if (!TestToolDlg.start_save_video())
										{
											m_ClientSocket.Socket_server_Close();
											g_connet_flag = FALSE;
											g_recode_video = FALSE;
											return 0;
										}
									}

									TestToolDlg.server_rev_video_date();

									if (g_test_config.save_video_enable)
									{
										TestToolDlg.stop_save_video();
									}

									for (i =0; i < MAX_DECONDE_BUF_NUM; i++)
									{
										memset(g_show_video[i].m_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
										g_show_video[i].m_receive_buf_len = 0;
										g_show_video[i].m_empty_flag = TRUE;
									}
									g_write_current_buf_idex = 0;
									g_read_current_buf_idex = 0;

									memset(g_one_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
								}
							}
						}
					}
				}
			}
			fprintf(stderr, "Socket_server_Close\n");
			m_ClientSocket.Socket_server_Close();
			g_connet_flag = FALSE;
			g_recode_video = FALSE;
		}
		else
		{
			fprintf(stderr, "Socket_server_Close 00 \n");
			m_ClientSocket.Socket_server_Close();
			break;
		}
	}
	return 1;
}



BOOL CAnykaIPCameraDlg::create_server_video_thread() 
{
	close_server_video_thread();

	g_hBurnThread_server = CreateThread(NULL, 0, tcp_server_thread, this, 0, NULL);
	if (g_hBurnThread_server == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	creat_server_thread = TRUE;
	g_malloc_buf_success = TRUE;

	return TRUE;
}
void CAnykaIPCameraDlg::close_server_video_thread() 
{
	UINT i = 0;
	if(g_hBurnThread_server != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_server);
		g_hBurnThread_server = INVALID_HANDLE_VALUE;
	}


	for (i =0; i < MAX_DECONDE_BUF_NUM; i++)
	{
		memset(g_show_video[i].m_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);
		g_show_video[i].m_receive_buf_len = 0;
		g_show_video[i].m_empty_flag = TRUE;
	}
	g_write_current_buf_idex = 0;
	g_read_current_buf_idex = 0;
	memset(g_one_video_receive_buf, 0, VIDEO_RECEIVE_BUFFER_SIZE);

	creat_server_thread = FALSE; 
	g_malloc_buf_success = FALSE;
	m_RightDlg.SetDlgItemText(IDC_BUTTON_CONNET, _T("启动视频"));
}
void CAnykaIPCameraDlg::play(BYTE *buf, UINT buf_len, UINT *fps) 
{
	struct timeval presentationTime = {0};
	STREAMPARAMETER stStreamParam;

	m_videoDecoder[0]->SendData(buf, buf_len, presentationTime, "video", "H264", NULL);
	m_videoDecoder[0]->getStreamParameter(stStreamParam);
	*fps  = stStreamParam.nFPS;
}

DWORD WINAPI play_video_date(LPVOID lpParameter)
{
	
	CAnykaIPCameraDlg Testtool;
	UINT len = 0;
	BYTE *play_buf = NULL;
	UINT read_len = 0;
	UINT ret_len = 0;
	UINT data_total_len = g_video_file_len;
	UINT fps = 0, fps_temp = 0;
		
	g_enter_flag = TRUE;
	
	play_buf=new BYTE[VIDEO_RECEIVE_BUFFER_SIZE + 1];
	if (play_buf == NULL)
	{
		AfxMessageBox(_T("malloc play buf fail"), MB_OK);
		return 0;
	}

#if 1
	if (Testtool.show_Stream(0, NULL, FALSE) != 0)
	{
		delete[] play_buf;
		AfxMessageBox(_T("open d3d9 fail"), MB_OK);
		return 0;
	}
#endif
	

	while (1)
	{
		if (g_hBurnThread_play_video != INVALID_HANDLE_VALUE)
		{
			if (data_total_len == 0)
			{
				break;
			}

			if (g_need_re_open_flag)
			{
				break;
			}

			if (g_stop_play_flag)
			{
				while(1)
				{
					if (g_hBurnThread_play_video == INVALID_HANDLE_VALUE)
					{
						g_start_play_flag = FALSE;
						g_stop_play_flag = FALSE;
						delete[] play_buf;
						//Testtool.close_play_file();
						Testtool.close_play_video_thread();	
						g_need_re_open_flag = TRUE;
						Testtool.m_RightDlg.m_play_video_flag = TRUE;
						return 1;
					}

					if (g_start_play_flag)
					{
						g_start_play_flag = FALSE;
						g_stop_play_flag = FALSE;
						break;
					}
					Sleep(100);
				}
			}

			ret_len = g_file_fp.Read(&read_len,4);
			if (ret_len == 0)
			{
				AfxMessageBox(_T("read frame len error"), MB_OK);
				break;
			}
			data_total_len -= ret_len;

			if (read_len > VIDEO_RECEIVE_BUFFER_SIZE || read_len == 0)
			{
				AfxMessageBox(_T("one frame data len is error"), MB_OK);
				break;
			}

			ret_len = g_file_fp.Read(play_buf,read_len);
			if (ret_len == 0)
			{
				AfxMessageBox(_T("read frame data error"), MB_OK);
				break;
			}
			data_total_len -= ret_len;

			//播放视频
			Testtool.play(play_buf, ret_len, &fps);
			g_current_frame_idex++;

			if (g_test_config.set_play_frame_flag)
			{
				fps_temp = g_test_config.play_frame_num;
			} 
			else
			{
				if (fps == 0)
				{
					fps_temp = 8;
				}
				else
				{
					fps_temp = fps/2;
				}
			}
			
			Sleep((1000 / fps_temp) - 5);
		}
		else
		{
			break;
		}
	}
	delete[] play_buf;
	//Testtool.close_play_file(); //g_play_fp.Close();
	Testtool.close_play_video_thread();	
	g_need_re_open_flag = TRUE;
	Testtool.m_RightDlg.m_play_video_flag =TRUE;
	//AfxMessageBox(_T("回放结束"), MB_OK);
	return 1;
}


void CAnykaIPCameraDlg::close_play_video_thread() 
{
	if(g_hBurnThread_play_video != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_play_video);
		g_hBurnThread_play_video = INVALID_HANDLE_VALUE;
	}
}

BOOL CAnykaIPCameraDlg::creat_play_video_thread() 
{

	close_play_video_thread();

	g_hBurnThread_play_video = CreateThread(NULL, 0, play_video_date, this, 0, NULL);
	if (g_hBurnThread_play_video== INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CAnykaIPCameraDlg::play_video_file() 
{

	//获取文件，并打开
	CString strPath;
	UINT ret_len = 0, idex = 0, total_len = 0, read_len = 0 ;

	m_RightDlg.GetDlgItemText(IDC_EDIT_SAVE_VIDEO, strPath);
	if (strPath.IsEmpty())
	{
		AfxMessageBox(_T("还没有录像，或请先在视频路径选择一个视频文件"), MB_OK);
		return FALSE;
	}
	memset(g_test_config.m_video_path, 0, MAX_PATH + 1);
	_tcscpy(g_test_config.m_video_path, strPath);
	
	
	if (!open_play_file(CFile::modeRead | CFile::shareDenyRead))//g_play_fp.Open(g_test_config.m_video_path,CFile::modeRead))
	{
		//AfxMessageBox(_T("error: open play video file fail"), MB_OK);
		return FALSE;
	}


	g_video_file_len = g_file_fp.GetLength();
	if (g_video_file_len == 0)
	{
		close_play_file();
		AfxMessageBox(_T("error: get the video len == 0"), MB_OK);
		return FALSE;
	}


	total_len = 0;

	//获取所有帧长
	while(1)
	{
		ret_len = g_file_fp.Read(&read_len,4);
		if (ret_len == 0)
		{
			close_play_file();
			AfxMessageBox(_T("error: get the frame len == 0"), MB_OK);
			return FALSE;
		}
		g_file_fp.Seek((LONGLONG)read_len, CFile::current);
		idex++;

		total_len += read_len + 4;
		if (total_len >= g_video_file_len)
		{
			break;
		}
	}
	g_max_frame_idex = idex;
	g_current_frame_idex = 0;
	g_file_fp.SeekToBegin();
	
	if (g_play_first_frame_flag)
	{
		if (!creat_play_first_frame_thread())
		{
			close_play_file();
			AfxMessageBox(_T("创建播放第一帧线程失败"), MB_OK);
			return FALSE;
		}
	}
	else
	{
		if (!creat_play_video_thread())
		{
			close_play_file();
			AfxMessageBox(_T("创建播放线程失败"), MB_OK);
			return FALSE;
		}
	}
		
	return TRUE;
}


BOOL CAnykaIPCameraDlg::malloc_pre_pstYuvData() 
{
	g_pre_pstYuvData = new PICYUVDATA;
	if (NULL == g_pre_pstYuvData) {
		fprintf(stderr, "can't alloc memory for video Render\n");
		return FALSE;
	}

	ZeroMemory(g_pre_pstYuvData, sizeof(PICYUVDATA));

	g_pre_pstYuvData->pYData = new uint8_t[1280 * 960];
	if (NULL == g_pre_pstYuvData->pYData) {
		AfxMessageBox(_T("can't alloc memory for video Render Y data"), MB_OK);
		delete g_pre_pstYuvData;
		return FALSE;
	}

	g_pre_pstYuvData->pUData = new uint8_t[1280 * 960 >> 1];
	if (NULL == g_pre_pstYuvData->pUData) {
		AfxMessageBox(_T("can't alloc memory for video Render U data"), MB_OK);
		delete[] g_pre_pstYuvData->pYData;
		delete g_pre_pstYuvData;
		return FALSE;
	}

	g_pre_pstYuvData->pVData = new uint8_t[1280 * 960 >> 1];
	if (NULL == g_pre_pstYuvData->pVData) 
	{
		AfxMessageBox(_T("can't alloc memory for video Render V data"), MB_OK);
		delete[] g_pre_pstYuvData->pYData;
		delete[] g_pre_pstYuvData->pUData;
		delete g_pre_pstYuvData;
		return FALSE;
	}

	return TRUE;
}



DWORD WINAPI play_first_frame_date(LPVOID lpParameter)
{

	CAnykaIPCameraDlg Testtool;
	UINT len = 0, idex = 0;
	BYTE *play_buf = NULL;
	UINT read_len = 0;
	UINT ret_len = 0;
	UINT data_total_len = g_video_file_len;
	UINT fps = 0, fps_temp = 0;
	BOOL read_flag = FALSE;

	g_enter_flag = TRUE;

	play_buf=new BYTE[VIDEO_RECEIVE_BUFFER_SIZE + 1];
	if (play_buf == NULL)
	{
		AfxMessageBox(_T("malloc play buf fail"), MB_OK);
		return 0;
	}

	if (Testtool.show_Stream(0, NULL, FALSE) != 0)
	{
		delete[] play_buf;
		AfxMessageBox(_T("open d3d9 fail"), MB_OK);
		return 0;
	}


	while (1)
	{
		if (g_hBurnThread_play_first_frame != INVALID_HANDLE_VALUE)
		{
			if (!read_flag)
			{
				if (g_file_fp.m_hFile == INVALID_HANDLE_VALUE)
				{
					break;
				}
				g_file_fp.SeekToBegin();
				ret_len = g_file_fp.Read(&read_len,4);
				if (ret_len == 0)
				{
					AfxMessageBox(_T("read frame len error"), MB_OK);
					break;
				}
				data_total_len -= ret_len;

				if (read_len > VIDEO_RECEIVE_BUFFER_SIZE || read_len == 0)
				{
					AfxMessageBox(_T("one frame data len is error"), MB_OK);
					break;
				}

				ret_len = g_file_fp.Read(play_buf,read_len);
				if (ret_len == 0)
				{
					AfxMessageBox(_T("read frame data error"), MB_OK);
					break;
				}
				data_total_len -= ret_len;

				read_flag = TRUE;
			}

			//播放视频
			Testtool.play(play_buf, ret_len, &fps);
			g_current_frame_idex = 1;
			idex = 0;
			Sleep(10);
		}
		else
		{
			break;
		}
	}
	delete[] play_buf;
	//Testtool.close_play_file();
	Testtool.close_play_first_frame_thread();	
	g_need_re_open_flag = TRUE;
	Testtool.m_RightDlg.m_play_video_flag = TRUE;
	//AfxMessageBox(_T("回放结束"), MB_OK);
	return 1;
}


void CAnykaIPCameraDlg::close_play_first_frame_thread() 
{
	if(g_hBurnThread_play_first_frame != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_play_first_frame);
		g_hBurnThread_play_first_frame = INVALID_HANDLE_VALUE;
	}
}

BOOL CAnykaIPCameraDlg::creat_play_first_frame_thread() 
{

	close_play_first_frame_thread();
	g_hBurnThread_play_first_frame = CreateThread(NULL, 0, play_first_frame_date, this, 0, NULL);
	if (g_hBurnThread_play_first_frame== INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CAnykaIPCameraDlg::open_play_file(UINT nOpenFlags) 
{
	CString str;
	UINT i = 0;

	close_play_file();
	if (g_file_fp.m_hFile == INVALID_HANDLE_VALUE)
	{
		for (i = 0; i < 5; i++)
		{
			if (g_file_fp.Open(g_test_config.m_video_path, nOpenFlags)) //,CFile::modeRead))
			{
				break;
			}
			Sleep(100);
		}

		if (i == 5)
		{
			DWORD ret = GetLastError();
			str.Format(_T("error: open play video file fail %d"), ret);
			AfxMessageBox(str, MB_OK);
			return FALSE;
		}

		

		open_video_success = TRUE;


	}

	return TRUE;
	
}

void CAnykaIPCameraDlg::close_play_file() 
{
	if (g_file_fp.m_hFile != INVALID_HANDLE_VALUE)
	{
		g_file_fp.Close();
		open_video_success = FALSE;
	}
	
}


//

void CALLBACK waveOutProc(  HWAVEOUT hwo,UINT uMsg,DWORD_PTR dwInstance,DWORD_PTR dwParam1,DWORD_PTR dwParam2);   
// 放音回调函数  
void CALLBACK waveOutProc(  HWAVEOUT hwo,UINT uMsg,DWORD_PTR dwInstance,DWORD_PTR dwParam1,DWORD_PTR dwParam2)  
{   
	if (WOM_DONE == uMsg)   
	{   
		g_play_finish = TRUE;   
	}   
}   

void CAnykaIPCameraDlg::WAV_Out_Close(void)
{
	// clean    
	waveOutReset(g_hWaveOut); //停止放音  
	//MMRESULT waveOutPrepareHeader(HWAVEOUT hwo,LPWAVEHDR pwh,UINT cbwh);  
	waveOutUnprepareHeader(g_hWaveOut, &g_wavhdr, sizeof(WAVEHDR)); //为回放设备准备内存块函数　  
	waveOutClose(g_hWaveOut);  //关闭放音设备函数  

}


void CAnykaIPCameraDlg::WAV_Out_Open(void)
{
	MMRESULT ret  = 0;
	WAVEOUTCAPS woc; //WAVEINCAPS结构描述波形音频输出设备的能力  

	g_wavform.wFormatTag = WAVE_FORMAT_PCM;  //WAVE_FORMAT_PCM即脉冲编码  
	g_wavform.nChannels = 1;  // 声道  
	g_wavform.nSamplesPerSec = g_test_config.SamplesPerSec; // 采样频率  
	g_wavform.nAvgBytesPerSec = g_test_config.SamplesPerSec*16/8;  // 每秒数据量  
	g_wavform.nBlockAlign = 4; //g_wavform.nChannels * g_wavform.wBitsPerSample / 8;;   
	g_wavform.wBitsPerSample = 16; // 样本大小  
	g_wavform.cbSize = sizeof(g_wavform);  //大小，以字节，附加额外的格式信息WAVEFORMATEX结构  


	ret = waveOutOpen(NULL, WAVE_MAPPER, &g_wavform, NULL, NULL, WAVE_FORMAT_QUERY);
	fprintf(stderr, "waveOutOpen ret:%d \n",ret);

	// open    
	ret = waveOutOpen(&g_hWaveOut, WAVE_MAPPER, &g_wavform, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);  
	fprintf(stderr, "waveOutOpen ret:%d \n",ret);

	
	waveOutGetDevCaps((UINT_PTR)g_hWaveOut, &woc,sizeof(WAVEOUTCAPS));     


}


void CAnykaIPCameraDlg::WAV_Out_play(unsigned char *buf, UINT buf_len)
{
	// prepare buffer    
	
	g_wavhdr.lpData = (LPSTR)buf;   
	g_wavhdr.dwBufferLength = buf_len;  //MMRESULT waveOutPrepareHeader(HWAVEOUT hwo,LPWAVEHDR pwh,UINT cbwh);  
	g_wavhdr.dwFlags = 0;  //为回放设备准备内存块函数　  
	g_wavhdr.dwLoops = 0;  
	waveOutPrepareHeader(g_hWaveOut, &g_wavhdr, sizeof(WAVEHDR));   


	//MMRESULT waveOutWrite(HWAVEOUT hwo,LPWAVEHDR pwh,UINT cbwh);  
	waveOutWrite(g_hWaveOut, &g_wavhdr, sizeof(WAVEHDR)); //写数据（放音）函数  

	while(waveOutUnprepareHeader(g_hWaveOut,&g_wavhdr,sizeof(WAVEHDR)) ==WAVERR_STILLPLAYING)
	{
		Sleep(1);
	}
}


BOOL g_test_flag = FALSE;

UINT g_total_eln = 0;
// 函数定义  
void CALLBACK waveInProc(HWAVEIN hwi,UINT uMsg,DWORD_PTR dwInstance,DWORD_PTR dwParam1,DWORD_PTR dwParam2);   

// 录音回调函数  
void CALLBACK waveInProc(HWAVEIN hwi,UINT uMsg,DWORD_PTR dwInstance,DWORD_PTR dwParam1,DWORD_PTR dwParam2)   
{   
	LPWAVEHDR pwh = (LPWAVEHDR)dwParam1;
	UINT buf_len = 0;
	//char buf[1024] = {0};
	char buf[2048] = {0};
	CAnykaIPCameraDlg Testtool;

	//fprintf(stderr, "waveInProc start \n");

	if ((WIM_DATA==uMsg) && (g_open_tool) && g_start_recode_flag) 
	{   
		
		int temp = 0;
#if 0
		temp = BUFFER_SIZE - buf_count;   
		temp = (temp>pwh->dwBytesRecorded) ? pwh->dwBytesRecorded: temp;   
		memcpy(buffer+buf_count, pwh->lpData, temp);   
		buf_count += temp;   
		waveInAddBuffer(hwi, pwh, sizeof(WAVEHDR));   
#else
		//采到的音频数据发给小机上

#if 0
		if (g_buf_count < BUFFER_SIZE)
		{
			temp = BUFFER_SIZE - g_buf_count;   
			temp = (temp>pwh->dwBytesRecorded) ? pwh->dwBytesRecorded: temp;   
			memcpy(buffer+g_buf_count, pwh->lpData, temp);   
			g_buf_count += temp;
		}

		
		if (!g_test_flag && g_buf_count >= BUFFER_SIZE)
		{
			g_test_flag = TRUE;
			//把录下来的数据保存
			Testtool.Oncreat_audio_file();
		}
		
#endif
		//fprintf(stderr, "Send_audio_data len:%d \n", pwh->dwBytesRecorded);
		buf_len = pwh->dwBytesRecorded;
		memset(buf, 0, 2048);
		memcpy(buf, (char *)pwh->lpData, buf_len);
		g_total_eln += buf_len;
		//fprintf(stderr, "Send_audio_data len:%d, %d, %02x, %02x,%02x,%02x\n", buf_len, g_total_eln, buf[0], buf[1], buf[2], buf[3]);
		Testtool.Send_audio_data(buf,  buf_len);
		//waveInPrepareHeader(hwi, pwh, sizeof(WAVEHDR));  
		waveInAddBuffer(hwi, pwh, sizeof(WAVEHDR)); 
		//memset(buf, 0xAA, 1024);
		//Testtool.Send_audio_data(buf, 1024);
#endif
	}   
}  


void CAnykaIPCameraDlg::WAV_In_Close(void)
{
	UINT i = 0;

	waveInStop(g_hWaveIn);//waveInStop功能停止的波形音频输入  
	//停止录音函数：  
	//MMRESULT waveInReset( HWAVEIN hwi );   
	waveInReset(g_hWaveIn);//停止录音  
	//清除缓存函数：  
	//MMRESULT waveInUnprepareHeader( HWAVEIN hwi,LPWAVEHDR pwh, UINT cbwh);    
	for (i=0; i<FRAGMENT_NUM; i++)   
	{   
		waveInUnprepareHeader(g_hWaveIn, &g_wh[i], sizeof(WAVEHDR));
		if (g_wh[i].lpData != NULL)
		{
			delete[] g_wh[i].lpData;
			g_wh[i].lpData = NULL;   
		}
		
	}   

	//关闭录音设备函数：  
	//MMRESULT waveInClose( HWAVEIN hwi );  
	waveInClose(g_hWaveIn);  
}

void CAnykaIPCameraDlg::WAV_In_Open(void)
{
	MMRESULT ret =0;
	UINT i= 0;
	WAVEINCAPS wic;  //WAVEINCAPS结构描述波形音频输入设备的能力  
	// open    
	g_wavform.wFormatTag = WAVE_FORMAT_PCM;  //WAVE_FORMAT_PCM即脉冲编码  
	g_wavform.nChannels = 1;  // 声道  
	g_wavform.nSamplesPerSec = g_test_config.SamplesPerSec; // 采样频率  
	g_wavform.nAvgBytesPerSec = g_test_config.SamplesPerSec*16/8;  // 每秒数据量  
	g_wavform.wBitsPerSample = 16; // 样本大小  
	g_wavform.nBlockAlign = g_wavform.nChannels * g_wavform.wBitsPerSample / 8;
	g_wavform.cbSize = sizeof(g_wavform);  //大小，以字节，附加额外的格式信息WAVEFORMATEX结构  

	//打开录音设备函数  

	//fprintf(stderr, "WAV_In_Open start \n");

	ret = waveInOpen(NULL, WAVE_MAPPER, &g_wavform, NULL, NULL, WAVE_FORMAT_QUERY);
	//fprintf(stderr, "WAV_In_Open ret:%d \n",ret);

	ret = waveInOpen(&g_hWaveIn, WAVE_MAPPER, &g_wavform, (DWORD_PTR)waveInProc, (DWORD)this, CALLBACK_FUNCTION);
	//fprintf(stderr, "WAV_In_Open ret:%d \n",ret);
	if (MMSYSERR_NOERROR != ret) 
	{
		//AfxMessageBox(_T("waveInOpen fail, pls ckeck"), MB_OK);
		//return;
	}

	//识别打开的录音设备  
	waveInGetDevCaps((UINT_PTR)g_hWaveIn, &wic, sizeof(WAVEINCAPS));  

	// prepare buffer  
	for (i=0; i<FRAGMENT_NUM; i++)   
	{   
		g_wh[i].lpData = new char[FRAGMENT_SIZE];   
		g_wh[i].dwBufferLength = FRAGMENT_SIZE;   
		g_wh[i].dwBytesRecorded = 0;   
		g_wh[i].dwUser = NULL;   
		g_wh[i].dwFlags = 0;   
		g_wh[i].dwLoops = 1;   
		g_wh[i].lpNext = NULL;   
		g_wh[i].reserved = 0;  

		//为录音设备准备缓存函数：  
		//MMRESULT waveInPrepareHeader(  HWAVEIN hwi,  LPWAVEHDR pwh, UINT bwh );  
		waveInPrepareHeader(g_hWaveIn, &g_wh[i], sizeof(WAVEHDR));  

		//给输入设备增加一个缓存：  
		//MMRESULT waveInAddBuffer(  HWAVEIN hwi, LPWAVEHDR pwh, UINT cbwh );  
		waveInAddBuffer(g_hWaveIn, &g_wh[i], sizeof(WAVEHDR));   
	}   
	//开始录音函数  
	waveInStart(g_hWaveIn); //开始录音  

	//fprintf(stderr, "waveInStart start \n");
}



DWORD WINAPI record_wav_main(LPVOID lpParameter)
{
	CAnykaIPCameraDlg Testtool;
	UINT nReturn = 0;
	BOOL start_record = TRUE;

	while (1)
	{

		nReturn = waveInGetNumDevs();//定义输入设备的数目  
		if (nReturn == 0)
		{
			Sleep(100);
			continue;
		}
		
		if (g_hBurnThread_record_wav != INVALID_HANDLE_VALUE)
		{
			//开始录音
			if (g_start_recode_flag)
			{
				//开始录音
				if (start_record)
				{
					Testtool.WAV_In_Open();
					start_record = FALSE;
				}

				nReturn = waveInGetNumDevs();//定义输入设备的数目  
				if (nReturn == 0)
				{
					if (!start_record)
					{
						Testtool.WAV_In_Close();
						start_record = TRUE;
					}
					
					continue;
				}
			}
			else
			{
				if (!start_record)
				{
					Testtool.WAV_In_Close();
					start_record = TRUE;
				}
				
			}
			Sleep(10);
		}
		else
		{
			//停止录音
			if (!start_record)
			{
				Testtool.WAV_In_Close();
			}
			
			break;
		}
	}

	//Testtool.close_record_wav_thread();	
	return 1;
}

void CAnykaIPCameraDlg::close_record_wav_thread(void) 
{
	if(g_hBurnThread_record_wav != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_record_wav);
		g_hBurnThread_record_wav = INVALID_HANDLE_VALUE;
	}
}

BOOL CAnykaIPCameraDlg::creat_record_wav_thread(void) 
{

	close_record_wav_thread();
	g_hBurnThread_record_wav = CreateThread(NULL, 0, record_wav_main, this, 0, NULL);
	if (g_hBurnThread_record_wav== INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;
}

UINT g_write_cnt = 0;


#define  PLAY_WAV_NUM    16

DWORD WINAPI play_wav_main(LPVOID lpParameter)
{
	HWAVEOUT  hwo;
	CAnykaIPCameraDlg Testtool;
	UINT nReturn = 0;
	BOOL start_record = TRUE;
	unsigned char *buf = NULL;
	UINT buf_len = 0, idex = 0;
	char buf_wav[AUDIO_RECEIVE_BUFFER_SIZE*PLAY_WAV_NUM+1] = {0};
#if 1
	hwo = Testtool.open_pcm();
#else
	Testtool.WAV_Out_Open();
#endif

	while (1)
	{
		if (g_hBurnThread_play_wav != INVALID_HANDLE_VALUE)
		{

#if 1		
			idex = 0;
			while (1)
			{
				if (g_hBurnThread_play_wav == INVALID_HANDLE_VALUE)
				{
					break;
				}

				if (idex == PLAY_WAV_NUM)
				{
					break;
				}
				
				if (g_play_data[g_audio_read_current_buf_idex].m_empty_flag == FALSE && g_play_data[g_audio_read_current_buf_idex].m_receive_buf_len > 0)
				{
					buf_len = g_play_data[g_audio_read_current_buf_idex].m_receive_buf_len;
					memcpy(&buf_wav[idex*buf_len], g_play_data[g_audio_read_current_buf_idex].m_video_receive_buf, buf_len);
					idex++;


					memset(g_play_data[g_audio_read_current_buf_idex].m_video_receive_buf, 0 , AUDIO_RECEIVE_BUFFER_SIZE);
					g_play_data[g_audio_read_current_buf_idex].m_empty_flag = TRUE;
					g_play_data[g_audio_read_current_buf_idex].m_receive_buf_len = 0;
					g_audio_read_current_buf_idex++;

					if (g_audio_read_current_buf_idex == MAX_AUDIO_BUF_NUM)
					{
						g_audio_read_current_buf_idex = 0;
					}
				}
				else
				{
					Sleep(1);
				}
			}
			//fprintf(stderr, "idex:%d,%d \n",idex, buf_len);

			Testtool.play_pcm_buf(hwo, buf_wav, buf_len*idex);

#else
			if (g_play_data[g_audio_read_current_buf_idex].m_empty_flag == FALSE && g_play_data[g_audio_read_current_buf_idex].m_receive_buf_len > 0)
			{
				buf_len = g_play_data[g_audio_read_current_buf_idex].m_receive_buf_len;
#if 0
				if(g_write_cnt == 0)
				{
					Testtool.Oncreat_pcm_file();
				}
				else if (g_write_cnt < 200)
				{
					Testtool.Onwrite_pcm_file((char *)g_play_data[g_audio_read_current_buf_idex].m_video_receive_buf, buf_len);
				}
				else if (g_write_cnt == 200)
				{
					Testtool.Onclose_pcm_file();
				}
				g_write_cnt++;
#endif

				//需判断此时是否有数据				
				//fprintf(stderr, "play_pcm_buf start \n");
#if 1	
				//fprintf(stderr, "0g_audio_read_current_buf_idex:%d,%d \n",g_audio_read_current_buf_idex, g_play_data[g_audio_read_current_buf_idex].m_empty_flag);
				
				Testtool.play_pcm_buf(hwo, (char *)g_play_data[g_audio_read_current_buf_idex].m_video_receive_buf, buf_len);
				//fprintf(stderr, "play_pcm_buf end \n");
#else
				Testtool.WAV_Out_play(g_play_data[g_audio_read_current_buf_idex].m_video_receive_buf, buf_len);
				while (g_play_finish)
				{
					Sleep(1);
				}
				g_play_finish = FALSE;
#endif
				memset(g_play_data[g_audio_read_current_buf_idex].m_video_receive_buf, 0 , AUDIO_RECEIVE_BUFFER_SIZE);
				g_play_data[g_audio_read_current_buf_idex].m_empty_flag = TRUE;
				g_play_data[g_audio_read_current_buf_idex].m_receive_buf_len = 0;
				//fprintf(stderr, "1g_audio_read_current_buf_idex:%d,%d \n",g_audio_read_current_buf_idex, g_play_data[g_audio_read_current_buf_idex].m_empty_flag);
				g_audio_read_current_buf_idex++;
				
				if (g_audio_read_current_buf_idex == MAX_AUDIO_BUF_NUM)
				{
					g_audio_read_current_buf_idex = 0;
				}
			}
			else
			{
				Sleep(2); 
			}
#endif
		}
		else
		{
			//停止录音
#if 1
			Testtool.close_pcm(hwo);
#else
			Testtool.WAV_Out_Close();
#endif
			break;
		}
	}

	//Testtool.close_play_wav_thread();	
	return 1;
}

void CAnykaIPCameraDlg::close_play_wav_thread(void) 
{
	if(g_hBurnThread_play_wav != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_play_wav);
		g_hBurnThread_play_wav = INVALID_HANDLE_VALUE;
	}
}

BOOL CAnykaIPCameraDlg::creat_play_wav_thread(void) 
{

	close_play_wav_thread();
	g_hBurnThread_play_wav = CreateThread(NULL, 0, play_wav_main, this, 0, NULL);
	if (g_hBurnThread_play_wav== INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;
}

#if 0
void CAnykaIPCameraDlg::Oncreat_audio_file()  
{  
	// TODO: 在此添加控件通知处理程序代码  
	//停止录音  


	//存储声音文件  
	CFile m_file;  
	CFileException fileException;    
	SYSTEMTIME sys2; //获取系统时间确保文件的保存不出现重名  
	GetLocalTime(&sys2);  
	//以下实现将录入的声音转换为wave格式文件   

	//查找当前目录中有没有Voice文件夹 没有就先创建一个，有就直接存储  
	TCHAR szPath[MAX_PATH];       
	GetModuleFileName(NULL, szPath, MAX_PATH);  
	CString PathName(szPath);  
	//获取exe目录  
	CString PROGRAM_PATH = PathName.Left(PathName.ReverseFind(_T('\\')) + 1);  
	//Debug目录下RecordVoice文件夹中  
	PROGRAM_PATH+=_T("RecordVoice\\");  

	if (!(GetFileAttributes(PROGRAM_PATH)==FILE_ATTRIBUTE_DIRECTORY))  
	{  
		if (!CreateDirectory(PROGRAM_PATH,NULL))  
		{  

			AfxMessageBox(_T("Make Dir Error"));  

		}  
	}  

	//kn_string strFilePath = _T("RecordVoice\\");  
	//GetFilePath(strFilePath);  
	CString m_csFileName=PROGRAM_PATH+_T("\\audio");//strVoiceFilePath  

	//CString m_csFileName= _T("D:\\audio");       
	wchar_t s[30] = {0};      
	_stprintf(s,_T("%d%d%d%d%d%d"),sys2.wYear,sys2.wMonth,sys2.wDay,sys2.wHour,sys2.wMinute,sys2.wSecond/*,sys2.wMilliseconds*/);  
	m_csFileName.Append(s);  
	m_csFileName.Append(_T(".wav"));  
	m_file.Open(m_csFileName,CFile::modeCreate|CFile::modeReadWrite, &fileException);  
	DWORD m_WaveHeaderSize = 38;    
	DWORD m_WaveFormatSize = 18;    
	m_file.SeekToBegin();    
	m_file.Write("RIFF",4);    
	//unsigned int Sec=(sizeof  + m_WaveHeaderSize);     
	unsigned int Sec=(sizeof buffer + m_WaveHeaderSize);    
	m_file.Write(&Sec,sizeof(Sec));    
	m_file.Write("WAVE",4);    
	m_file.Write("fmt ",4);    
	m_file.Write(&m_WaveFormatSize,sizeof(m_WaveFormatSize));    

	m_file.Write(&g_wavform.wFormatTag,sizeof(g_wavform.wFormatTag));    
	m_file.Write(&g_wavform.nChannels,sizeof(g_wavform.nChannels));    
	m_file.Write(&g_wavform.nSamplesPerSec,sizeof(g_wavform.nSamplesPerSec));    
	m_file.Write(&g_wavform.nAvgBytesPerSec,sizeof(g_wavform.nAvgBytesPerSec));    
	m_file.Write(&g_wavform.nBlockAlign,sizeof(g_wavform.nBlockAlign));    
	m_file.Write(&g_wavform.wBitsPerSample,sizeof(g_wavform.wBitsPerSample));    
	m_file.Write(&g_wavform.cbSize,sizeof(g_wavform.cbSize));    
	m_file.Write("data",4);    
	m_file.Write(&g_buf_count,sizeof(g_buf_count));    

	m_file.Write(buffer,g_buf_count);    
	m_file.Seek(g_buf_count,CFile::begin); 

	m_file.Close();
}  
#endif


CFile m_file;  
void CAnykaIPCameraDlg::Oncreat_pcm_file()  
{
	
	CFileException fileException;    
	SYSTEMTIME sys2; //获取系统时间确保文件的保存不出现重名  
	GetLocalTime(&sys2);  
	//以下实现将录入的声音转换为wave格式文件   

	//查找当前目录中有没有Voice文件夹 没有就先创建一个，有就直接存储  
	TCHAR szPath[MAX_PATH];       
	GetModuleFileName(NULL, szPath, MAX_PATH);  
	CString PathName(szPath);  
	//获取exe目录  
	CString PROGRAM_PATH = PathName.Left(PathName.ReverseFind(_T('\\')) + 1);  
	//Debug目录下RecordVoice文件夹中  
	PROGRAM_PATH+=_T("RecordVoice\\");  

	if (!(GetFileAttributes(PROGRAM_PATH)==FILE_ATTRIBUTE_DIRECTORY))  
	{  
		if (!CreateDirectory(PROGRAM_PATH,NULL))  
		{  

			AfxMessageBox(_T("Make Dir Error"));  

		}  
	}  

	//kn_string strFilePath = _T("RecordVoice\\");  
	//GetFilePath(strFilePath);  
	CString m_csFileName=PROGRAM_PATH+_T("\\audio");//strVoiceFilePath  

	//CString m_csFileName= _T("D:\\audio");       
	wchar_t s[30] = {0};      
	_stprintf(s,_T("%d%d%d%d%d%d"),sys2.wYear,sys2.wMonth,sys2.wDay,sys2.wHour,sys2.wMinute,sys2.wSecond/*,sys2.wMilliseconds*/);  
	m_csFileName.Append(s);  
	m_csFileName.Append(_T(".wav"));  
	m_file.Open(m_csFileName,CFile::modeCreate|CFile::modeReadWrite, &fileException);  
}

void CAnykaIPCameraDlg::Onwrite_pcm_file(char *buf, UINT buf_len) 
{
	m_file.Write(buf,buf_len);   
}

void CAnykaIPCameraDlg::Onclose_pcm_file()  
{
	m_file.Close();
}


void CAnykaIPCameraDlg::Onopen_pcm_file() 
{
	//HWAVEOUT hwo;
	CFileException fileException; 
	char *buf = NULL;

	m_file.Open(_T("f://test.wav"),CFile::modeReadWrite, &fileException);  

	UINT len = m_file.GetLength();

	buf = (char *)malloc(len*sizeof(char));

	m_file.Read(buf, len);

	m_file.Close();
#if 0
	hwo = open_pcm();
	play_pcm_buf(hwo, buf, len) ;
	close_pcm(hwo);
#else
	WAV_Out_Open();
	WAV_Out_play((unsigned char *)buf, len);
	WAV_Out_Close();
#endif

}


int CAnykaIPCameraDlg::close_pcm(HWAVEOUT hwo) 
{
	waveOutClose(hwo);
	CloseHandle(g_wait);
	return 1;
}

HWAVEOUT CAnykaIPCameraDlg::open_pcm(void) 
{
	int             cnt = 0;
	HWAVEOUT        hwo;
	//WAVEHDR         wh;
	WAVEFORMATEX    wfx;
	DWORD  high = 0;
	BOOL ret = FALSE;
	DWORD read_len = 0;
	CPcmSpeaker ps; 

	wfx.wFormatTag = WAVE_FORMAT_PCM;//设置波形声音的格式
	wfx.nChannels = 1;//设置音频文件的通道数量
	wfx.nSamplesPerSec = g_test_config.SamplesPerSec + 800;//设置每个声道播放和记录时的样本频率
	wfx.nAvgBytesPerSec = g_test_config.SamplesPerSec*16/8;//设置请求的平均数据传输率,单位byte/s。这个值对于创建缓冲大小是很有用的
	wfx.wBitsPerSample = 16;	
	wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;//以字节为单位设置块对齐
	wfx.cbSize = sizeof(WAVEFORMATEX);//额外信息的大小

	g_wait = CreateEvent(NULL, 0, 0, NULL);
	waveOutOpen(&hwo, WAVE_MAPPER, &wfx, (DWORD_PTR)g_wait, 0L, CALLBACK_EVENT);//打开一个给定的波形音频输出装置来进行回放

	return hwo;
}

int CAnykaIPCameraDlg::play_pcm_buf(HWAVEOUT hwo, char *buf, UINT buf_len) 
{
	WAVEHDR header;
	ZeroMemory(&header, sizeof(WAVEHDR));
	header.dwBufferLength = buf_len;
	header.lpData = buf;
	waveOutPrepareHeader(hwo, &header, sizeof(WAVEHDR));
	waveOutWrite(hwo, &header, sizeof(WAVEHDR));
	//Sleep(500);
	while(waveOutUnprepareHeader(hwo,&header,sizeof(WAVEHDR)) ==WAVERR_STILLPLAYING)
	{
		Sleep(1);
	}


	return 1;
}