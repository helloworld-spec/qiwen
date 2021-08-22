// Anyka IP CameraDlg.h : 头文件
//

#pragma once

#include "NetTalk.h"
#include "ImageToolBar.h"
#include "afxcmn.h"
#include "PreviewDialog.h"
#include "RightCtrlDlg.h"
#include "BottomCtrlDlg.h"
#include "Aimer39RTSPClient.h"
#include "FfmpegEnvoy.h"
#include "VideoRender.h"
#include "ServerSearch.h"
#include "afxwin.h"
#include "PrivacyDialog.h"
#include "MotionDetectDlg.h"
#include "RecordPlayDlg.h"
#include "AutoLock.h"
#include "AudioRender.h"
#include "ClientSocket.h"
#include "Login_testconfig.h"
#include "CBurn_UIDDlg.h"
#include "StaticEx.h"
#include "CBurn_UIdDlg.h"
#include "image_reset.h"


#define MAX_VIDEO_LEN 400*1024

#define PREVIEW_WINDOWS						2
#define SUPPORT_STREAM_CNT					2
#define SUPPORT_AUDIO_STREAM_CNT			1
#define ONLY_AUDIO_STREAM_START_IDX			2

#define AP_ADDRESS_LEN			            (MAX_PATH + 24)

#define TIMER_COMMAND			1
#define TIMER_LONG_PRESS		2


#define MAX_PASSWD_LENGTH 100


#pragma pack(1)
typedef struct
{
	short len;		
	char commad_type;      
	char *data_param;  //数据长度+数据， 如果没有数据的况下，这里存放2个字节的长度，长度是0
	short check_sum;
}T_PACK_INFO;
#pragma pack()

#pragma pack(1)
typedef struct
{
	short len;		
	char commad_type;
	BOOL auto_test_flag;
	char *file_name; //len+name          
	char *data_param;
	short check_sum;
}T_NET_INFO;
#pragma pack()

typedef enum VideoFunction_en
{
	VF_PRIVACY_AREA,
	VF_MOTION_DETECT,
	VF_PLAY,
	VF_MAX
}VIDEOFUNCTION;

typedef struct tag_KickOutMessageWParam_st {
	unsigned long ulIpAddr;
	unsigned long ulPort;
}KickOutMessageWParam;


enum
{
	TEST_COMMAND = 1,
	TEST_RESPONSE,
	TEST_HEARTBEAT,
	TEST_COMMAND_FINISH,
	TEST_ENABLE_KEY,
};


typedef struct
{
	UINT start_x;
	UINT start_y;
	UINT image_width;
	UINT image_height;
}T_IMAGE_INFO;


typedef struct
{
	TCHAR use_name[MAX_PASSWD_LENGTH+1];	
	TCHAR password[MAX_PASSWD_LENGTH+1];	
}T_SEC_CTRL;

// CAnykaIPCameraDlg 对话框
class CAnykaIPCameraDlg : public CDialog
{
// 构造
public:
	CAnykaIPCameraDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~CAnykaIPCameraDlg(){}

// 对话框数据
	enum { IDD = IDD_ANYKAIPCAMERA_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	static void __stdcall OnClientFinish(void * pLParam, void * pRParam);
	static void __stdcall OnClientDisConnect(void * pLParam, void * pRParam);
	static void __stdcall OnFullScreenMessage(UINT message,	WPARAM wParam, LPARAM lParam, void * pClassParam);
	static void __stdcall OnTalkKickOut(IServer * pIServer, unsigned long ulIpAddr, unsigned short usPort, void * pClassParam);
	static void __stdcall OnServerReturnInfo(IServer * pIServer, RETINFO * pstRetInfo, void * pClassParam);

// 实现
protected:
	HICON m_hIcon;
	
	void InitToolBar();
	void InitStatusBar();
	void InitTreeCtrlPosition();
	void InitPreviewWindows(BOOL bNeedCreate = TRUE, BOOL bfull_flag = TRUE);
	void InitComboBox();
	void UpdateCombo();
	void InitPrivacyDialog();
	
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
	char * MakeRTSPUrl();
	int CloseTheStream(int iSelect, BOOL bNeedCloseAudio = FALSE);
	int OpenTheStream(int iSelect, const char * strURL = NULL, BOOL bNeedENotify = TRUE);

	afx_msg void OnPrivacyArea();
	afx_msg void OnMotionDetect();
	afx_msg void OnPicture();
	afx_msg void OnRecord();
	afx_msg void OnPlay();
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
	afx_msg void OnVolumeMinus();
	afx_msg void OnVolumePlus();
	afx_msg void OnStopRecord();
	
	int OnAudioInOpen(int nAudioClientIndex);
	int OnAudioInClose(int nAudioClientIndex);

	afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClose();
	afx_msg void OnSearchDevice();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPreviewchoose1();
	afx_msg void OnPreviewchoose2();
	BOOL OnPreviewchoose_test();

	void MakeAndSendImageSet();
	afx_msg void OnCbnSelchangeContrastCombo();
	afx_msg void OnCbnSelchangeBrightnessCombo();
	afx_msg void OnCbnSelchangeSaturationCombo();

	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	afx_msg void OnBnClickedButtonLeft();
	afx_msg void OnBnClickedButtonUp();
	afx_msg void OnBnClickedButtonRight();
	afx_msg void OnBnClickedButtonDown();
	afx_msg void OnBnClickedButtonLeftRight();
	afx_msg void OnBnClickedButtonUpDown();
	afx_msg void OnBnClickedButtonRepositionSet();
	afx_msg void OnBnClickedButtonReposition();

	afx_msg void OnPreviewdlgchooseTalkOpen();
	afx_msg void OnPreviewdlgchooseTalkClose();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnTalkKickOutMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnServerDisconnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnServerRetInfo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPreviewdlgchoose1ClosePreview();
	afx_msg void OnPreviewdlgchooseClosePreview();
	afx_msg LRESULT OnHotKey(WPARAM wParam,LPARAM lParam);//热键
	afx_msg HBRUSH OnCtlColor(CDC *pDC,CWnd *pWnd,UINT nCtlColor);//画刷


	BOOL CanDoTheJob();
	int RegisterThePreviewServer(IServer * pIServer, int iSelect, const char * strURL);
	int UnregisterThePreviewServer(int iSelect);
	int ShutDownTheStream(int iSelect);

	static unsigned int WINAPI thread_begin( void * pParam );
	
	void UpdateTreeNode(IServer * UpdateServer);
	int ServerDisConnect(int iSelect);
	void Monitor();

	void VideoFunctionOpenProcess(VIDEOFUNCTION enVFun);

	void FullScreenProcess(BOOL bIsFullScreen, int iSelect);
	
	void CameraMovement(CAMERAMOVEMENT movement);

	void TempCloseTalk(int iIndex);

	void RepositionWidget();
	void PositionTheButton();
	void PositionTheImageCombo();

	void WiatForMonitorThreadEnd();
	BOOL read_config(LPCTSTR file_path);
	
	void  ChangeTestItem(int case_idex);


public:

	BOOL start_test;
	BOOL start_flag;
	BOOL next_test_flag;

	BOOL user_producer;

	CFont font,oldFont;
	TCHAR	path_module[MAX_PATH+1];
	
	BOOL test_temp(UINT idex);
	BOOL decode_data_pack(char *buf, UINT buf_len);

	BOOL get_and_compare_serialfile_toftp(void);
	BOOL compare_serialfile(TCHAR *srcfile, TCHAR *dstfile);
	BOOL server_rev_printf_date(void);
	BOOL Send_cmd_printf(char *param, UINT param_len);
	BOOL creat_server_thread();
	void Close_server_thread();

	BOOL Close_record_video();
	BOOL Creat_record_video();
	BOOL creat_record_video_thread(); 
	void Close_record_video_thread(); 

	BOOL case_test_user();

	void show_IP_info();
	void On_find_ip();
	void On_find_ip_update();
	BOOL Anyka_find_ip_thread();
	BOOL Creat_find_ip_thread(); 
	void Close_find_ip_thread(); 
	BOOL check_ip(TCHAR *buf);
	BOOL  check_test_false(void);
	BOOL case_ircut_onoff_test(void);

	void case_video();
	void case_ircut_off();
	void case_ircut_on();
	BOOL case_monitor();
	BOOL case_monitor_ReTest();
	void case_interphone();
	void case_head();
	BOOL case_sd();
	BOOL case_wifi();
	void case_infrared();
	BOOL case_rev();
	BOOL case_mac();
	BOOL case_uid();
	void case_main(BOOL pass_flag);
	BOOL Auto_move_test(char move_flag);
	BOOL Auto_move(char *cmd);
	void Auoto_Move_CloseServer(char move_flag);
	BOOL Auto_stop();
	BOOL move_start(char *idex);

	TCHAR *ConvertAbsolutePath(LPCTSTR path);

	BOOL The_first_case();
	BOOL case_ircut_test(BOOL m_ircut_flag);
	BOOL play_pcm(TCHAR *pcm_file_nameh);
	void write_test_info(BOOL pass_flag, BOOL Isburn_flag, UINT case_idex);
	BOOL enter_case_uid(UINT *burn_flag);
	BOOL Send_cmand(UINT case_idex);
	void  show_test_info(BOOL end_flag);
	BOOL Download_UpdateFile(TCHAR *name_buf);
	VOID lower_to_upper(TCHAR *surbuf, TCHAR *dstbuf);
	BOOL Otp_mac_add(TCHAR *surbuf, TCHAR *dstbuf);
	BOOL Mac_Addr_add_1(TCHAR *buf_temp);
	BOOL Download_serialfile_toftp(void);
	BOOL create_thread_rev_data(UINT idex);
	BOOL Creat_find_result_thread(UINT idex);
	void close_find_result_thread(UINT idex);
	void close_thread_rev_data(UINT idex); 
	BOOL create_thread_heat(LPCTSTR addr, UINT idex) ;
	void close_thread_heat(UINT idex) ;
	void find_next_idex();
	BOOL finish_test(BOOL passs_flag);
	BOOL create_paly_pcm_thread_data(UINT idex);
	void close_thread_paly_pcm();
	int play_pcm_test(TCHAR *pcm_file_name) ;
	void pre_case_main(BOOL pass_flag);
	BOOL find_pre_idex();
	BOOL get_ssid_data(UINT index, CString subRight);
	BOOL decode_file(TCHAR *file_name);
	BOOL close_test();
	BOOL pcm_play_main() ;
	void close_pcm_play_thread(); 
	BOOL Creat_pcm_play_thread(BOOL flag);
	int play_pcm_buf(HWAVEOUT hwo, char *buf, UINT buf_len);
	BOOL download_file(TCHAR *file_name);
	BOOL put_file_by_ftp(CString strSourceName, CString strDestName);
	int test_pcm_buf(TCHAR *pcm_file_name);
	BOOL download_file_intenet(TCHAR *pcm_file_name);
	void close_download_thread();
	//BOOL download_file_thread(TCHAR *pcm_file_name);
	BOOL download_file_thread(CString strSourceName, CString strDestName);
	void close_pcm_rev_thread();
	BOOL pcm_rev_main();
	HWAVEOUT open_pcm(void);
	int close_pcm(HWAVEOUT hwo);
	BOOL Create_Test_Wait_Data(void);
	void close_test_wait();
	BOOL Auto_move_connect(char move_flag);
	BOOL Auto_move_disconnect(char move_flag);

	void close_minitor(void);
	void find_IP_CloseServer(UINT idex);
	BOOL Creat_test_monitor_thread();
	void close_test_monitor_thread();
	void close_retest_wifi_thread();
	BOOL Creat_retest_wifi_thread();
	void close_retest_sd_thread();
	BOOL Creat_retest_sd_thread();
	BOOL case_get_lisence();
	BOOL Get_update_file();

	BOOL creat_update_all_thread();
	void Close_update_all_thread();
	BOOL update_main();
	void Close_check_MAC_thread();
	BOOL Creat_check_MAC_thread(); 
	BOOL Download_updatefile(TCHAR *ip_address, UINT list_idex, UINT idex); 
	BOOL Anyka_check_MAC_finish(UINT idex);
	BOOL Anyka_check_update_finish(UINT idex);
	BOOL check_update_finish_Server(LPCTSTR addr, UINT idex);
	void close_thread_update_finish(UINT idex); 
	BOOL create_thread_update_finish(UINT idex);
	BOOL Anyka_Test_check_info_update(UINT timeout, UINT idex);

	BOOL get_system_info();
	void CloseServer_image(UINT idex);
	
	BOOL Send_audio_data(char *buf, UINT buf_len);
	void WAV_In_Close(void);
	void WAV_In_Open(void);
	void close_record_wav_thread(void);
	BOOL creat_record_wav_thread(void);
	HWAVEOUT open_pcm_sky(void) ;
	int close_pcm_sky(HWAVEOUT hwo); 
	void close_play_wav_thread(void);
	BOOL creat_play_wav_thread(void);
	BOOL Send_cmd_data(char commad_type,  char *param);
	int play_pcm_buf_sky(HWAVEOUT hwo, char *buf, UINT buf_len);

private:
	CTreeCtrl m_TreeCtrl;
	CImageToolBar m_ToolBar;
	CStatusBar	m_StatusBar;
	CMenu m_menuTalk;
	CNetTalk m_NetTalk;

	CAimer39RTSPClient * m_pClient[SUPPORT_STREAM_CNT];
	CFfmpegEnvoy * m_videoDecoder[SUPPORT_STREAM_CNT];
	CFfmpegEnvoy * m_AudioDecoder[SUPPORT_STREAM_CNT];
	CVideoRender * m_videoRender[SUPPORT_STREAM_CNT];
	CAudioRender * m_AudioRender[SUPPORT_STREAM_CNT];

	CClock	m_SyncClock[PREVIEW_WINDOWS];
	CPreviewDialog m_Preview[PREVIEW_WINDOWS];
	
	CCBurn_UIdDlg m_CBurnUidDlg;
	IServer * m_pServerPreviews[PREVIEW_WINDOWS];
	string m_strURL[PREVIEW_WINDOWS];
	
	volatile int m_nRBChoosePrevIndex;
	volatile int m_nAudioClientIndex;
	volatile int m_nVideoFullScreenIndex;

	int m_nLongPressButtonID;

	CServerSearch m_Search;

	HTREEITEM m_hCurrentSelectItem;
	CComboBox m_ContrastCombo, m_SaturationCombo, m_BrightnessCombo;
	CComboBox m_acutanceCom;

	CPrivacyDialog m_PrivacyDialog;
	CMotionDetectDlg m_MotionDetectDlg;
	CRecordPlayDlg m_RecordPlayDlg;

	KickOutMessageWParam m_stKickOutParam;
	RETINFO	m_stRetInfo;


	HANDLE m_MonitorThread;
	BOOL m_runThreadFlag, m_bNeedJudgeDisConnWork, m_bIsInit, m_bPicture, m_bIsLongPress, m_bIsLongPressDone, m_bIsSearch;

	CriticalSection m_csForServerConnect, m_csForOpenCloseStream, m_csForTalkOpen, m_csForKickOut, m_csForRet;
	
public:
	CLogin_testconfig m_login_test_config;
	CTest_ConfigDlg   m_enter_test_config;
	CCBurn_UIdDlg     m_Burn_UIdDlg;
	CRightCtrlDlg     m_RightDlg;
	CBottomCtrlDlg    m_BottomDlg;
	CImage_reset      image_reset;

	T_SEC_CTRL		passwd_ctrl;

	CClientSocket m_ClientSocket;
	TCHAR m_path[MAX_PATH+1];
	CString m_username; 
	CString m_password; 
	char m_ircut_flag;

	BOOL get_password_and_name(void);
	BOOL ftp_StorePassword();
	BOOL ftp_GetPassword();

	afx_msg void OnCbnSelchangeCombo4();
	BOOL Creat_Anyka_Test_thread() ;
	void Close_Anyka_Test_thread() ;
	BOOL Anyka_Test_thread();
	afx_msg void OnBnClickedRadioIrcutOn();
	afx_msg void OnBnClickedButtonSet();
	afx_msg void OnBnClickedRadioIrcutOff();
	BOOL ConnetServer(LPCTSTR addr, UINT idex);
	BOOL Connet_FTPServer(LPCTSTR addr, UINT idex);
	BOOL decode_command(char *lpBuf, char *commad_type, char *file_name, char *param);
	BOOL Send_cmd(char commad_type, BOOL auto_test_flag, char *file_name, char *param, UINT param_len, UINT idex);
	BOOL Anyka_Test_check_info(UINT timeout);
	BOOL Anyka_Test_UID_check_info(UINT timeout);
	BOOL Anyka_Test_mac_check_info(UINT timeout);
	BOOL Anyka_Test_wifi_check_info(UINT timeout);
	BOOL Anyka_Test_sd_check_info(UINT timeout);
	
	BOOL Anyka_Test_check_no_info(UINT timeout);
	void CloseServer(UINT idex);
	BOOL OnSend_data(void);
	BOOL Anyka_connet(void);
	BOOL find_file_indir(TCHAR *file_name, UINT *name_len);
	afx_msg void OnBnClickedButtonRecoverDev();
	CListCtrl m_test_config;
	CListCtrl m_test_wifi_list;
	afx_msg void OnBnClickedButtonConfigure();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMSetfocusList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonWriteUid();
	afx_msg void OnBnClickedButtonWriteMac();
	afx_msg void OnBnClickedButtonPass();
	afx_msg void OnBnClickedButtonFailed();
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedButtonNext();
	afx_msg void OnEnChangeEditWifiName();
	afx_msg void OnEnChangeEditIp();
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	afx_msg void OnDestroy();
};
