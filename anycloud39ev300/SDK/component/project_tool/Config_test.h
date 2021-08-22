// Config_test.h: interface for the CConfig_test class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIG_TEST_H__C778E81C_AC31_4E86_A73E_141FD5841EF2__INCLUDED_)
#define AFX_CONFIG_TEST_H__C778E81C_AC31_4E86_A73E_141FD5841EF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlconv.h>
#include "searchserver.h"

#define        TEST_CONFIG_FILE              _T("/test_config/test_config.txt")
#define        CONFIG_FILE                   _T("config.txt")

#define        MAC_ADDRESS_LEN                100
 
typedef struct
{
	TCHAR test_name[MAC_ADDRESS_LEN];
	TCHAR test_file[MAX_PATH];
	TCHAR test_param[MAX_PATH];
	BOOL  auto_test_flag;
	BOOL  test_pass_flag;
	BOOL  only_down_laodfile_flag;
	char  ex_command_param;
}T_TEST_CONFIG;

typedef struct
{
	TCHAR Current_IP_channel_name[MAX_PATH];
	TCHAR Current_IP_address_buffer[IP_ADDRE_LEN];
	TCHAR Current_IP_address_ch_buffer[IP_ADDRE_LEN];
	TCHAR Current_IP_address_wg_buffer[IP_ADDRE_LEN];
	TCHAR Current_IP_address_dns1_buffer[IP_ADDRE_LEN];
	TCHAR Current_IP_address_dns2_buffer[IP_ADDRE_LEN];
	BOOL Current_IP_address_dhcp_flag;
}T_CURRENT_IP_CONFIG;


class CConfig_test  
{
public:
	CConfig_test();
	virtual ~CConfig_test();


	UINT  test_count;
	UINT  auto_test_count;
	UINT  no_auto_test_count;
	UINT  current_auto_test_idex;
	UINT  current_no_auto_test_idex;
	UINT  no_auto_testing_idex;
	UINT  no_auto_pre_test_idex;
	UINT  test_pass_num;
	UINT  test_fail_num;
	UINT  test_pass;
	UINT  test_fail;

	T_TEST_CONFIG  *test_config;
	T_TEST_CONFIG  *auto_test_config;
	T_TEST_CONFIG  *no_auto_test_config;

	UINT main_code_rate;
	UINT sub_code_rate;

	BOOL OSD_flag;
	BOOL time_flag;

	UINT main_fps;
	UINT sub_fps;
	UINT qp;
	UINT gop;
		 


	TCHAR m_path[MAX_PATH+1];

	//记录到config文件上
	TCHAR m_IP_address[50];
	TCHAR m_config_filename[MAX_PATH+1];
	BOOL  scan_serial_flag;
	TCHAR m_serial_number[MAX_PATH+1];
	TCHAR com_type[10];
	TCHAR MAC_address[MAC_ADDRESS_LEN];


	BOOL m_dhcp_flag;
	TCHAR IP_channel_name[MAX_PATH];
	TCHAR IP_address_buffer[MAX_PATH];
	TCHAR IP_address_ch_buffer[MAX_PATH];
	TCHAR IP_address_wg_buffer[MAX_PATH];
	TCHAR IP_address_dns1_buffer[MAX_PATH];
	TCHAR IP_address_dns2_buffer[MAX_PATH];

	T_CURRENT_IP_CONFIG m_current_config[MAX_PATH];

	void Set_auto_test_config(void);
	BOOL get_test_config(UINT index, CString subRight);
	BOOL ReadConfig(LPCTSTR file_path);
	BOOL Write_Config(LPCTSTR file_path);
	TCHAR *ConvertAbsolutePath(LPCTSTR path);
	
};

#endif // !defined(AFX_CONFIG_TEST_H__C778E81C_AC31_4E86_A73E_141FD5841EF2__INCLUDED_)
