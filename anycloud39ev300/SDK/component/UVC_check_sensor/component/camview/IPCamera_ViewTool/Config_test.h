// Config_test.h: interface for the CConfig_test class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIG_TEST_H__C778E81C_AC31_4E86_A73E_141FD5841EF2__INCLUDED_)
#define AFX_CONFIG_TEST_H__C778E81C_AC31_4E86_A73E_141FD5841EF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlconv.h>
#include "ServerSearch.h"

//#define        TEST_CONFIG_FILE              _T("/test_config/test_config.txt")
//#define        CONFIG_FILE                   _T("config.txt")
#define		   TEST_CONFIG_CASE           _T("config_testcase.txt")
//#define		   CONFIG_CURRENT_MAC          _T("config_current_mac.txt")
//#define        SERAIL_CONFIG   _T("serial_config")
//#define        SERAIL_CONFIG_BAK   _T("serial_config_bak")


#define        MAC_ADDRESS_LEN                100
#define        UPDATE_MAX_NUM     100
#define        ONE_TIME_MAX_NUM   12 



typedef enum
{
	TYPE_VIDEO_VGA = 0,
	TYPE_VIDEO_720P,
	TYPE_VIDEO_960P,
}E_VIDEO_SIZE_TYPE;

typedef struct
{
	//TCHAR test_name[MAC_ADDRESS_LEN];
	//TCHAR test_file[MAX_PATH];
	//TCHAR test_param[MAX_PATH];
	//BOOL  auto_test_flag;
	//BOOL  test_pass_flag;
	//BOOL  only_down_laodfile_flag;
	//char  ex_command_param;
}T_TEST_CONFIG;

typedef struct
{
	//TCHAR Current_IP_channel_name[MAX_PATH];
	TCHAR Current_IP_address_buffer[IP_ADDRE_LEN];
	//TCHAR Current_IP_address_ch_buffer[IP_ADDRE_LEN];
	//TCHAR Current_IP_address_wg_buffer[IP_ADDRE_LEN];
	//TCHAR Current_IP_address_dns1_buffer[IP_ADDRE_LEN];
	//TCHAR Current_IP_address_dns2_buffer[IP_ADDRE_LEN];
	//BOOL Current_IP_address_dhcp_flag;
	TCHAR Current_IP_UID[MAC_ADDRESS_LEN];
	TCHAR Current_IP_MAC[MAC_ADDRESS_LEN];
	TCHAR Current_IP_diver_name[MAX_PATH];
	TCHAR Current_IP_version[MAC_ADDRESS_LEN];
}T_CURRENT_IP_CONFIG;


typedef struct
{
	UINT line_idex;
	TCHAR content_data[1024];
	double fon_height;
}T_CONTENT_PRINTF_CFG;


typedef struct
{
	char  video_size;
	char  frame;
	short data_rate;
}T_VIDEO_CONFIG;



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

	//T_TEST_CONFIG  *test_config;
	//T_TEST_CONFIG  *auto_test_config;
	//T_TEST_CONFIG  *no_auto_test_config;

	UINT main_code_rate;
	UINT sub_code_rate;

	BOOL OSD_flag;
	BOOL time_flag;
	BOOL sd_reset_flag;
	BOOL wifi_reset_flag;
	BOOL onvif_or_rtsp_flag;
	
	UINT SamplesPerSec;

	UINT main_fps;
	UINT sub_fps;
	UINT qp;
	UINT gop;


	BOOL config_video_enable;
	BOOL config_voice_rev_enable;  //监听
	BOOL config_voice_send_enable; //对讲
	BOOL config_head_enable;
	BOOL config_sd_enable;
	BOOL config_wifi_enable;
	//BOOL config_red_line_enable;
	BOOL config_reset_enable;
	BOOL config_lan_mac_enable;
	BOOL config_uid_enable;
	BOOL config_ircut_enable;

	BOOL config_auto_login;

	BOOL config_video_test_pass;
	BOOL config_voice_rev_test_pass;  //监听
	BOOL config_voice_send_test_pass; //对讲
	BOOL config_head_test_pass;
	BOOL config_sd_test_pass;
	BOOL config_wifi_test_pass;
	BOOL config_red_line_test_pass;
	BOOL config_reset_test_pass;
	BOOL config_lan_mac_test_pass;
	BOOL config_uid_test_pass;
	BOOL config_ircut_on_test_pass;
	BOOL config_ircut_off_test_pass;
	BOOL config_ircut_onoff_test_pass;

	UINT test_num;

	UINT frame_num;

	TCHAR	update_file_name[MAX_PATH+1];
	
	TCHAR m_mac_start_addr[MAX_PATH+1];
	TCHAR m_mac_current_addr[MAX_PATH+1];
	TCHAR m_mac_end_addr[MAX_PATH+1];

	TCHAR m_uid_number[MAX_PATH+1];

	TCHAR serial_file_name[MAX_PATH+1];
	TCHAR ueser_name[MAC_ADDRESS_LEN];
	TCHAR pass_word[MAC_ADDRESS_LEN];
	TCHAR pass_word_research[MAC_ADDRESS_LEN];

	TCHAR m_path[MAX_PATH+1];
	
	TCHAR m_ip_addr[MAC_ADDRESS_LEN];
	UINT  m_ip_port;

	BOOL save_video_enable;

	TCHAR m_wifi_name[MAC_ADDRESS_LEN];

	TCHAR m_test_monitor_time[MAC_ADDRESS_LEN];
	TCHAR m_test_reset_time[MAC_ADDRESS_LEN];
	TCHAR m_test_ircut_time[10];
	TCHAR m_video_path[MAX_PATH+1];

	TCHAR newest_version[MAC_ADDRESS_LEN];

	//记录到config文件上
	//TCHAR m_IP_address[50];
	//TCHAR m_config_filename[MAX_PATH+1];
	//BOOL  scan_serial_flag;
	//TCHAR m_serial_number[MAX_PATH+1];
	TCHAR com_type[10];
	//TCHAR MAC_address[MAC_ADDRESS_LEN];


	//BOOL m_dhcp_flag;
	//TCHAR IP_channel_name[MAX_PATH];
	//TCHAR IP_address_buffer[MAX_PATH];
	//TCHAR IP_address_ch_buffer[MAX_PATH];
	//TCHAR IP_address_wg_buffer[MAX_PATH];
	//TCHAR IP_address_dns1_buffer[MAX_PATH];
	//TCHAR IP_address_dns2_buffer[MAX_PATH];

	T_CURRENT_IP_CONFIG m_current_config[MAX_PATH];

	T_CURRENT_IP_CONFIG m_last_config[MAX_PATH];

	T_CURRENT_IP_CONFIG m_before_update_config[MAX_PATH];
	T_CURRENT_IP_CONFIG m_finish_update_config[MAX_PATH];

	BOOL net_mode;
	BOOL win10_flag;
	BOOL update_find_flag;

	UINT image_x;
	UINT image_y;

	TCHAR PrinterName[1024];
	double LabWidth;
	double LabHeight;
	double LabSpaceV;
	double LabSpaceH;
	double OffsetH;
	double OffsetV;
	UINT page_num;
	BOOL server_flag;

	BOOL set_play_frame_flag;
	UINT play_frame_num;


	T_VIDEO_CONFIG video_parm;
	T_CONTENT_PRINTF_CFG config_printf_cfg[5];

	void Set_auto_test_config(void);
	BOOL get_test_config(UINT index, CString subRight);
	BOOL ReadConfig(LPCTSTR file_path);
	BOOL Write_Config(LPCTSTR file_path);
	TCHAR *ConvertAbsolutePath(LPCTSTR path);
	BOOL Write_current_macConfig(LPCTSTR file_path);
	BOOL Read_current_macConfig(LPCTSTR file_path);
	void defautl_config(void);
	BOOL Read_printf_cfg(LPCTSTR file_path);
	BOOL Write_printf_cfg(LPCTSTR file_path);

};

#endif // !defined(AFX_CONFIG_TEST_H__C778E81C_AC31_4E86_A73E_141FD5841EF2__INCLUDED_)
