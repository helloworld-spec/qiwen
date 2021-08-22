// ISPCTRL_TOOLDlg.h : header file
//


#if !defined(AFX_ISPCTRL_TOOLDLG_H__00FF6E8B_9CFF_490D_8CE5_57860C516379__INCLUDED_)
#define AFX_ISPCTRL_TOOLDLG_H__00FF6E8B_9CFF_490D_8CE5_57860C516379__INCLUDED_

#include <atlconv.h>
#include "NetCtrl.h"	// Added by ClassView
#include "SaturationDlg.h"	// Added by ClassView
#include "LscDlg.h"	// Added by ClassView
#include "DpcDlg.h"	// Added by ClassView
#include "SysDlg.h"	// Added by ClassView
#include "GammaDlg.h" // Added by ClassView
#include "BBDlg.h"	// Added by ClassView
#include "RawlutDlg.h"	// Added by ClassView
#include "DemosaicDlg.h"	// Added by ClassView
#include "CCMDlg.h"	// Added by ClassView
#include "DenoiseDlg.h"	// Added by ClassView
#include "SharpDlg.h"	// Added by ClassView
#include "EdgeDlg.h"	// Added by ClassView
#include "3DNRDlg.h"	// Added by ClassView
#include "WdrDlg.h"	// Added by ClassView
#include "FcsDlg.h"	// Added by ClassView
#include "YuvEffectDlg.h"	// Added by ClassView
#include "WbDlg.h"	// Added by ClassView
#include "ExpDlg.h"	// Added by ClassView
#include "AFDlg.h"	// Added by ClassView
#include "StatDlg.h"	// Added by ClassView
#include "GBDlg.h"	// Added by ClassView
#include "basepage.h"
#include "Public_TextDlg.h"
#include "Zone_weightDlg.h"
#include "MiscDlg.h"
#include "ContrastDlg.h"
#include "Rgb2YuvDlg.h"	// Added by ClassView
#include "afxinet.h" 
#include "compareDlg.h" 
#include "CHUEDlg.h" 
#include "Y_GammaDlg.h" // Added by ClassView


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define MAIN_VERSION		4
#define SUB_VERSION0		0
<<<<<<< HEAD
#define SUB_VERSION1		25
=======
#define SUB_VERSION1		44
>>>>>>> 73eea11... [整理规范]1. 修改工具版本号为V4.0.44；2.规范工具里面的一些书写内容以及版权信息内容


enum
{
	TEST_COMMAND = 1,
	TEST_RESPONSE,
	TEST_HEARTBEAT,
	TEST_COMMAND_FINISH,
	TEST_COMMAND_START
};


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

typedef struct                
{
	T_U32	bvalid;
	T_U32	filelen;
	char* pdata;
}CFGFILE_STRUCT;


/////////////////////////////////////////////////////////////////////////////
// CISPCTRL_TOOLDlg dialog

class CISPCTRL_TOOLDlg : public CDialog
{
// Construction
public:
	CRgb2YuvDlg m_Rgb2YuvDlg;
	CGBDlg m_GBDlg;
	CNetCtrl m_NetCtrl;
	CStatDlg m_StatDlg;
	CAFDlg m_AFDlg;
	CExpDlg m_ExpDlg;
	CWbDlg m_WbDlg;
	CSaturationDlg m_SaturationDlg;
	CContrastDlg m_ContrastDlg;
	CYuvEffectDlg m_YuvEffectDlg;
	CFcsDlg m_FcsDlg;
	CWdrDlg m_WdrDlg;
	C3DNRDlg m_3DNRDlg;
	//CEdgeDlg m_EdgeDlg;
	CSharpDlg m_SharpDlg;
	CDenoiseDlg m_DenoiseDlg;
	CCCMDlg m_CCMDlg;
	CDemosaicDlg m_DemosaicDlg;
	CRawlutDlg m_RawlutDlg;
	CLscDlg m_LscDlg;
	CDpcDlg m_DpcDlg;
	CBBDlg m_BBDlg;
	CGammaDlg m_GammaDlg;
	CY_GammaDlg m_Y_GammaDlg;
	CCHUEDlg m_HueDlg;
	CSysDlg m_SysDlg;
	Public_TextDlg m_TextDlg;
	CMISCDlg m_MiscDlg;
	CCompareDlg m_cmpDlg;
	CZone_weightDlg m_Zone_WeightDlg;
	CDialog *pDlg[DIALOG_NUM];
	T_DIALOG_ID	DialogId;
	unsigned long m_RemoteIp;
	CISPCTRL_TOOLDlg(CWnd* pParent = NULL);	// standard constructor
	UINT	m_Port;
	CString	m_ConnectState;
	CString	m_Password;

	CToolTipCtrl    m_Mytip;
	CToolTipCtrl m_toolTip;

	UINT m_heartbeat_timer;
	UINT m_check_timer;
	UINT m_msgboxshow_timer;

	TCHAR m_path[MAX_PATH+1];

	CInternetSession *m_pInetSession ;
	CFtpConnection *m_pFtpConnection ; 

	UINT m_sensor_num;
	AK_ISP_INIT_SENSOR      m_Isp_sensor;
	bool m_bOnlySensor;
	T_U32 m_sensor_id;

	T_U8 m_gamma_copyUItoTest_flag;

	T_U8 m_SubFileId;
	T_U8 m_SubFileNum;
	T_U8 m_version[16];

	CFGFILE_STRUCT m_SubFile[SUBFILE_NUM_MAX];

	bool DecodeSubFileData(char *buf, UINT size); 
	bool DecodeTotalFile(char *buf, UINT size);
	void SaveImgData(T_U8 type, char* buf, T_U32 size);
	void OnFTP_close();
	int Get_WB_EX_PageInfoSt(void * pPageInfoSt, int & nStLen);
	TCHAR *ConvertAbsolutePath(LPCTSTR path);
	void CISPCTRL_TOOLDlg::SetMstboxTimer();
	BOOL Create_Send_Heart_Thread(void);
	BOOL Close_Send_Heart_Thread(void);
// Dialog Data
	//{{AFX_DATA(CISPCTRL_TOOLDlg)
	enum { IDD = IDD_DIALOG_ISPCTRL_TOOL };
	CComboBox	m_SubFileIdCtrl;
	CButton	m_btnConnect;
	CIPAddressCtrl	m_Ip;
	CTreeCtrl	m_trModules;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CISPCTRL_TOOLDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CISPCTRL_TOOLDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonConnect();
	afx_msg LRESULT OnPageEnableChangeMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPageSubmissionMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPageGetInfoMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPageGetInfoMsg_ForText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPageSubmissionMsg_ForText(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClearSensorParam(void);
	afx_msg void OnPageReadMsg_ForText(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPageSaveMsg_ForText(WPARAM wParam, LPARAM lParam);
	afx_msg void OnButtonRead();
	afx_msg void OnButtonWrite();
	afx_msg void OnButtonGet();
	afx_msg void OnButtonSet();
	afx_msg void OnButtonConfirm();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButtonGetall();
	afx_msg void OnPageCopyUiToText(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPageCopyTextToUi(WPARAM wParam, LPARAM lParam);
	afx_msg void OnGetYuvImg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnButtonAddfile();
	afx_msg void OnSelchangeComboSubfileid();
	afx_msg void OnButtonReadtmp();
	afx_msg void OnButtonSavetmp();
	afx_msg void OnButtonEptsubfile();
	afx_msg void OnButtonReadV2();
	afx_msg void OnButtonReadV3();
	afx_msg void OnGetLscInfo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSetLscInfo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnGetRgbGamma(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void SetWindowTitle();
	void SendIspParam(BOOL bNeedJudge = FALSE, int nPageID = -1, int nFlag = -1);
	void SendIspParam_ForText(BOOL bNeedJudge = FALSE, int nPageID = -1, int nFlag = -1);
	bool GetSubFileData(char *buf, UINT *size);

	BOOL Send_cmd(char commad_type, BOOL auto_test_flag, char *file_name, char *param);	
	
	BOOL Anyka_Test_check_info();
	BOOL  OnDownload_tuner(LPCTSTR addr);
	BOOL Connet_FTPServer(LPCTSTR addr);
	void OnDownload_tuner_close();
	void GetSubFileHeadInfo(CFGFILE_HEADINFO* headinfo);
	bool CheckSubFileHeadInfo(CFGFILE_HEADINFO* headinfo);
	T_BOOL CheckSubFileData(char* cfgBuf, T_U32 size);
	T_BOOL CheckTotalFileData(char* cfgBuf, T_U32 size);
	bool SaveSubFile(T_U8 subfileid, char *buf, UINT size);
	void SetSubFileNum();
	void CleanAll(void);
	void OnGet_wb_info(WPARAM wParam, LPARAM lParam);
	void OnSet_wb_info(WPARAM wParam, LPARAM lParam);
	void OnGet_exp_info(WPARAM wParam, LPARAM lParam);
	bool CheckSubFileHeadInfo_v2(CFGFILE_HEADINFO_V2* headinfo);
	T_BOOL CheckSubFileData_v2(char* cfgBuf, T_U32 size);
	bool Convert_v2SubFile(T_U8 subfileid, char *buf, UINT size);
	T_BOOL CheckTotalFileData_v2(char* cfgBuf, T_U32 size);
	bool DecodeTotalFile_v2(char *buf, UINT size);
	bool CheckSubFileHeadInfo_v3(CFGFILE_HEADINFO_V3* headinfo);
	T_BOOL CheckSubFileData_v3(char* cfgBuf, T_U32 size);
	bool Convert_v3SubFile(T_U8 subfileid, char *buf, UINT size);
	T_BOOL CheckTotalFileData_v3(char* cfgBuf, T_U32 size);
	bool DecodeTotalFile_v3(char *buf, UINT size);
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ISPCTRL_TOOLDLG_H__00FF6E8B_9CFF_490D_8CE5_57860C516379__INCLUDED_)
