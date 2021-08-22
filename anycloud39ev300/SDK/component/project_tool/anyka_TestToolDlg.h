// anyka_TestToolDlg.h : header file
//

#if !defined(AFX_ANYKA_TESTTOOLDLG_H__8F1648B4_DE38_43EC_A7B3_9A71ABD340DB__INCLUDED_)
#define AFX_ANYKA_TESTTOOLDLG_H__8F1648B4_DE38_43EC_A7B3_9A71ABD340DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ClientSocket.h"
#include "afxinet.h" 
#include "logfile.h" 


#define  MAX_PASSWD_LENGTH  100

enum
{
	SCAN_SERIAL = 1,
	NO_SCAN_SERIAL
};

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
	TCHAR use_name[MAX_PASSWD_LENGTH+1];	
	TCHAR password[MAX_PASSWD_LENGTH+1];	
}T_SEC_CTRL;

/////////////////////////////////////////////////////////////////////////////
// CAnyka_TestToolDlg dialog

class CAnyka_TestToolDlg : public CDialog
{
// Construction
public:
	CAnyka_TestToolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAnyka_TestToolDlg)
	enum { IDD = IDD_ANYKA_TESTTOOL_DIALOG };
	CButton	m_show_connet;
	CIPAddressCtrl	m_ipaddress_net;
	CListCtrl	m_no_auto_test_config;
	CListCtrl	m_auto_test_config;
	CIPAddressCtrl	m_ipaddress;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnyka_TestToolDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL


public:
	CClientSocket m_ClientSocket;
	CString m_username; 
	CString m_password; 
	UINT    ctrl_time;
    DWORD   used_time;

	CTabCtrl TabCtrl;
	HBITMAP	hBitmap_connet_fail;
	HBITMAP hBitmap_connet_pass;
	HBITMAP hBitmap_wait_connet;
	BYTE m_f0;
	BYTE m_f1;
	BYTE m_f2;
	BYTE m_f3;

	UINT update_or_set_IP_num ;
	

	BOOL get_password_and_name(void);
	BOOL Connet_FTPServer(LPCTSTR addr, UINT idex); 
	BOOL Ftp_update_file(void); 	
	BOOL Ftp_donwload_file(void);
	BOOL set_config_item(void);
	BOOL Send_cmd(char commad_type, BOOL auto_test_flag, char *file_name, char *param, UINT idex);
	BOOL set_test_staut(UINT file_idex,  BOOL auto_flag, UINT test_staut);
	void reset_disable(void);
	BOOL find_file_indir(TCHAR *file_name, UINT *name_len);
	BOOL Download_serialfile_toftp(void);
	void Get_serialnum_bycom();
	void Get_serialnum_bycom(char *buf);
	BOOL decode_command(char *lpBuf, char *commad_type, char *file_name, char *param);
	BOOL Anyka_Test_check_info(TCHAR *test_name, BOOL update_flag, UINT idex);
	void StartTimer();
	void StopTimer();
	BOOL show_test_staut(UINT file_idex,  BOOL auto_flag, UINT test_staut);
	void add_num_test(BOOL test_pass_flag, BOOL reap_test_flag);
	BOOL Anyka_Test_Sever_Connet(void);
	void Anyka_Test_Sever_close(void);
	BOOL create_connet_thread();
	void close_connet_thread();
	BOOL Anyka_Test_get_IPAddress(void);
	BOOL create_watch_dog_heat_thread();
	void close_watch_dog_heat_thread();
	BOOL Mac_Addr_add_1(TCHAR *buf_temp);
	VOID lower_to_upper(TCHAR *surbuf, TCHAR *dstbuf);
	BOOL Otp_mac_add(TCHAR *surbuf, TCHAR *dstbuf);
	BOOL is_multicast_ether_addr(TCHAR *addr);
    BOOL is_zero_ether_addr(TCHAR *addr);
	void CloseServer(UINT idex);
    BOOL ConnetServer(LPCTSTR addr, UINT idex);
	void Close_Anyka_sousuo_thread();
    BOOL Creat_Anyka_sousuo_thread();
	BOOL check_info_is_emtpy(void);
	void Close_Anyka_connetIP_thread(UINT i); 
    BOOL Creat_Anyka_connetIP_thread();
	BOOL write_config();
	void Close_Anyka_update_thread();
	BOOL update_main();
	BOOL Download_updatefile(TCHAR *ip_address, UINT list_idex, UINT idex);
	void show_IP_info();
	BOOL update_one_main(TCHAR *ip_address, UINT list_idex, UINT idex);

	BOOL create_thread_update_finish(UINT idex);
    void close_thread_update_finish(UINT idex);
	BOOL  check_update_finish_Server(LPCTSTR addr, UINT idex) ;
	BOOL Anyka_check_update_finish(UINT idex);

	BOOL create_thread_rev_data(UINT idex);
	void close_thread_rev_data(UINT idex) ;
	BOOL create_thread_heat(LPCTSTR addr, UINT idex);
	void close_thread_heat(UINT idex) ;

	BOOL GetPassword();
    BOOL StorePassword();

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAnyka_TestToolDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonConnetServer();
	afx_msg void OnButtonCameraTest();
	afx_msg void OnButtonAdTest();
	afx_msg void OnButtonWifiTest();
	afx_msg void OnButtonLedTest();
	afx_msg void OnButtonKeyTest();
	afx_msg void OnButtonTestConfig();
	afx_msg void OnButtonLoadConfig();
	afx_msg void OnRadioNeedSerial();
	afx_msg void OnRadioNoNeedSerial();
	afx_msg void OnChangeEditSerialNumber();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButtonTest();
	afx_msg void OnItemchangedListNoAutoTestConfig(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchangingListNoAutoTestConfig(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetfocusListNoAutoTestConfig(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButton2();
	afx_msg void OnButtonConfig();
	afx_msg void OnButtonSousuo();
	afx_msg void OnCheck2();
	afx_msg void OnCheckDhcp();
	afx_msg void OnButtonSetconfig();
	afx_msg void OnButtonUpdateAll();
	afx_msg void OnChangeEditChannelName();
	afx_msg void OnButtonManual();
	afx_msg void OnButtonUpdateOne();
	virtual void OnOK();
	afx_msg void OnButtonDettectList();
	afx_msg void OnChangeEditUsename();
	afx_msg void OnChangeEditPassword();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANYKA_TESTTOOLDLG_H__8F1648B4_DE38_43EC_A7B3_9A71ABD340DB__INCLUDED_)
