// Anyka IP CameraDlg.h : 头文件
//

#pragma once

#include "NetTalk.h"
#include "ImageToolBar.h"
#include "afxcmn.h"
#include "PreviewDialog.h"
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

#define PREVIEW_WINDOWS						2
#define SUPPORT_STREAM_CNT					2
#define SUPPORT_AUDIO_STREAM_CNT			1
#define ONLY_AUDIO_STREAM_START_IDX			2

#define AP_ADDRESS_LEN			            (MAX_PATH + 24)



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
	TEST_COMMAND_START
};

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
	TCHAR *ConvertAbsolutePath(LPCTSTR path);


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

	CClientSocket m_ClientSocket;
	TCHAR m_path[MAX_PATH+1];
	CString m_username; 
	CString m_password; 
	char m_ircut_flag;

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
	BOOL create_thread_rev_data(UINT idex); 
	void close_thread_rev_data() ;
	BOOL Send_cmd(char commad_type, BOOL auto_test_flag, char *file_name, char *param);
	BOOL Anyka_Test_check_info(void);
	void CloseServer(void);
	BOOL OnSend_data(void);
	BOOL Anyka_connet(void);
	BOOL find_file_indir(TCHAR *file_name, UINT *name_len);
	afx_msg void OnBnClickedButtonRecoverDev();
};
