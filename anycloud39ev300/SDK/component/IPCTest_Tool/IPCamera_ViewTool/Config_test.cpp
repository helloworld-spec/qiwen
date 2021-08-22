// Config_test.cpp: implementation of the CConfig_test class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Config_test.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define UNICODE_TXT_HEAD 0xFEFF

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfig_test::CConfig_test()
{

}

CConfig_test::~CConfig_test()
{

}

TCHAR *CConfig_test::ConvertAbsolutePath(LPCTSTR path)
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


void CConfig_test::Set_auto_test_config(void)
{
#if 0
	UINT i = 0;
	UINT auto_idex = 0;
	UINT no_auto_idex = 0;

	auto_test_count = 0;
	no_auto_test_count = 0;

	for (i=0; i < test_count; i++)
	{
		if (test_config[i].auto_test_flag)
		{
			auto_test_count++;
		}
		else
		{
			no_auto_test_count++;
		}
	}

	if (auto_test_count != 0)
	{
		if(auto_test_config)
		{
			delete[] auto_test_config;//如果非空，那么要进行删除内存
		}
		auto_test_config = new T_TEST_CONFIG[auto_test_count];
		memset(auto_test_config, 0, auto_test_count*sizeof(T_TEST_CONFIG));
	}

	if (no_auto_test_count != 0)
	{
		if(no_auto_test_config)
		{
			delete[] no_auto_test_config;//如果非空，那么要进行删除内存
		}
		no_auto_test_config = new T_TEST_CONFIG[no_auto_test_count];
		memset(no_auto_test_config, 0, no_auto_test_count*sizeof(T_TEST_CONFIG));
	}

	for (i=0; i < test_count; i++)
	{
		if (test_config[i].auto_test_flag)
		{
			_tcsncpy(auto_test_config[auto_idex].test_name, test_config[i].test_name, MAX_PATH);//pc的路径
			_tcsncpy(auto_test_config[auto_idex].test_file, test_config[i].test_file, MAX_PATH);//pc的路径
			_tcsncpy(auto_test_config[auto_idex].test_param, test_config[i].test_param, MAX_PATH);//pc的路径
			auto_test_config[auto_idex].auto_test_flag = test_config[i].auto_test_flag;
			auto_test_config[auto_idex].only_down_laodfile_flag = test_config[i].only_down_laodfile_flag;
			auto_test_config[auto_idex].ex_command_param = test_config[i].ex_command_param;
			auto_idex++;
		}
		else
		{
			_tcsncpy(no_auto_test_config[no_auto_idex].test_name, test_config[i].test_name, MAX_PATH);//pc的路径
			_tcsncpy(no_auto_test_config[no_auto_idex].test_file, test_config[i].test_file, MAX_PATH);//pc的路径
			_tcsncpy(no_auto_test_config[no_auto_idex].test_param, test_config[i].test_param, MAX_PATH);//pc的路径
			no_auto_test_config[no_auto_idex].auto_test_flag = test_config[i].auto_test_flag;
			no_auto_test_config[no_auto_idex].only_down_laodfile_flag = test_config[i].only_down_laodfile_flag;
			no_auto_test_config[no_auto_idex].ex_command_param = test_config[i].ex_command_param;
			no_auto_idex++;
		}
	}
#endif

}

#if 0
//下载到闪存的数据
BOOL CConfig_test::get_test_config(UINT index, CString subRight)
{
	CString str;
	int nPos;

	if(index >= test_count)
	{
		return FALSE;
	}

	USES_CONVERSION;

	//bCompare
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	_tcsncpy(test_config[index].test_name, str, MAX_PATH);//pc的路径

	//pc path
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	_tcsncpy(test_config[index].test_file, str, MAX_PATH);//pc的路径

	//pc path
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);
	
	str.TrimLeft();
	str.TrimRight();
	test_config[index].auto_test_flag = atoi(T2A(str)); //hex2int(T2A(subRight));//bin_revs_size


	//pc path
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);
	//bin exctern size
	str.TrimLeft();
	str.TrimRight();
	_tcsncpy(test_config[index].test_param, str, MAX_PATH);//pc的路径


	//pc path
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);
	//bin exctern size
	str.TrimLeft();
	str.TrimRight();
	test_config[index].only_down_laodfile_flag = atoi(T2A(str));



	//bin exctern size
	subRight.TrimLeft();
	subRight.TrimRight();
	test_config[index].ex_command_param = atoi(T2A(subRight));
	
	return TRUE;
}
#endif

void CConfig_test::defautl_config(void)
{
	UINT i = 0;
	
	config_video_enable = 0;
	config_voice_rev_enable = 0;  //监听
	config_voice_send_enable = 0; //对讲
	config_head_enable = 0;
	config_sd_enable = 0;
	config_wifi_enable = 0;
	//config_red_line_enable = 0;
	config_reset_enable = 0;
	config_lan_mac_enable = 0;
	config_uid_enable = 0;
	config_ircut_enable = 0;
	test_num = 0;

	memset(m_mac_start_addr, 0, MAX_PATH);
	memset(m_mac_current_addr, 0, MAX_PATH);
	memset(m_mac_end_addr, 0, MAX_PATH+1);

	for (i = 0; i < MAX_PATH; i++)
	{
		memset(m_current_config[i].Current_IP_UID, 0, MAC_ADDRESS_LEN);
		memset(m_current_config[i].Current_IP_address_buffer, 0, IP_ADDRE_LEN);
		memset(m_current_config[i].Current_IP_diver_name, 0, MAX_PATH);
		memset(m_current_config[i].Current_IP_version, 0, MAC_ADDRESS_LEN);

		memset(m_last_config[i].Current_IP_UID, 0, MAC_ADDRESS_LEN);
		memset(m_last_config[i].Current_IP_address_buffer, 0, IP_ADDRE_LEN);
		memset(m_last_config[i].Current_IP_diver_name, 0, MAX_PATH);
		memset(m_last_config[i].Current_IP_version, 0, MAC_ADDRESS_LEN);
	}
	memset(ueser_name, 0, MAC_ADDRESS_LEN);
	memset(pass_word, 0, MAC_ADDRESS_LEN);

	_tcscpy(ueser_name, _T("生产者"));
	_tcscpy(pass_word, _T("abc"));
	_tcscpy(pass_word_research, _T("123"));
	memset(serial_file_name, 0, MAX_PATH+1);
	_tcsncpy(serial_file_name, _T("uid.txt"), 12);
	//memset(m_wifi_name, 0, MAC_ADDRESS_LEN);
	//_tcsncpy(m_wifi_name, _T("5"), 3);

	memset(m_test_monitor_time, 0, MAC_ADDRESS_LEN);
	_tcsncpy(m_test_monitor_time, _T("5"), 3);
	memset(m_test_reset_time, 0, MAC_ADDRESS_LEN);
	_tcsncpy(m_test_reset_time, _T("3"), 3);
	memset(m_test_ircut_time, 0, 10);
	_tcsncpy(m_test_ircut_time, _T("3"), 3);
	
	memset(PID, 0, 32);
	_tcscpy(PID, _T("1360000000"));
	

	sd_reset_flag = false;
	wifi_reset_flag = false;
	onvif_or_rtsp_flag = TRUE;
	net_mode = FALSE;

	is_printf_lab = TRUE;
	need_close_monitor = FALSE;
	need_tool_printf = FALSE;

	uid_download_mode = 1;
	uid_download_len = 32;

	image_x = 0;
	image_y = 0;


	m_test_sd_flag = 0; //sd, 0表示没有测试，1表成测试成功，2表示测试失败
	m_test_wifi_flag = 0; //wifi,0表示没有测试，1表成测试成功，2表示测试失败
	m_test_reset_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	m_test_ircut_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	m_test_MAC_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	m_test_UID_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	m_test_speaker_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	m_test_monitor_flag = 0; //监听，0表示没有测试，1表成测试成功，2表示测试失败
	m_test_cloud_flag = 0; //云台， 0表示没有测试，1表成测试成功，2表示测试失败
	m_test_play_flag = 0; //对讲，0表示没有测试，1表成测试成功，2表示测试失败


	m_sd_size = 0;
	memset(m_ssid_info, 0, 1024*sizeof(T_SSID_INFO));
	m_ssid_num = 0;

}

//读取配置文件的参数
BOOL CConfig_test::ReadConfig(LPCTSTR file_path)
{
	CString str;
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

		if (_T("config_video_enable") == subLeft)
		{
			config_video_enable = atoi(T2A(subRight));
		}
		else if (_T("config_voice_rev_enable") == subLeft)
		{
			config_voice_rev_enable = atoi(T2A(subRight));
		}
		else if (_T("config_ircut_enable") == subLeft)
		{
			config_ircut_enable = atoi(T2A(subRight));
		}
		else if (_T("config_voice_send_enable") == subLeft)
		{
			config_voice_send_enable = atoi(T2A(subRight));
		}
		else if (_T("config_head_enable") == subLeft)
		{
			config_head_enable = atoi(T2A(subRight));
		}
		else if (_T("config_sd_enable") == subLeft)
		{
			config_sd_enable = atoi(T2A(subRight));
		}
		else if (_T("config_wifi_enable") == subLeft)
		{
			config_wifi_enable = atoi(T2A(subRight));
		}
		/*else if (_T("config_red_line_enable") == subLeft)
		{
			config_red_line_enable = atoi(T2A(subRight));
		}*/
		else if (_T("config_reset_enable") == subLeft)
		{
			config_reset_enable = atoi(T2A(subRight));
		}
		else if (_T("config_lan_mac_enable") == subLeft)
		{
			config_lan_mac_enable = atoi(T2A(subRight));
		}
		else if (_T("m_mac_end_addr") == subLeft)
		{
			_tcscpy(m_mac_end_addr,  subRight);
		}
		else if (_T("config_uid_enable") == subLeft)
		{
			config_uid_enable = atoi(T2A(subRight));
		}
		else if (_T("m_mac_start_addr") == subLeft)
		{
			_tcscpy(m_mac_start_addr,  subRight);
		}
		else if (_T("config_auto_login") == subLeft)
		{
			config_auto_login = atoi(T2A(subRight));
		}
		else if (_T("m_wifi_name") == subLeft)
		{
			_tcscpy(m_wifi_name,  subRight);
		}
		else if (_T("m_test_monitor_time") == subLeft)
		{
			_tcscpy(m_test_monitor_time,  subRight);
		}
		else if (_T("m_test_reset_time") == subLeft)
		{
			_tcscpy(m_test_reset_time,  subRight);
		}
		else if (_T("m_test_ircut_time") == subLeft)
		{
			_tcscpy(m_test_ircut_time,  subRight);
		}
		else if (_T("newest_version") == subLeft)
		{
			_tcscpy(newest_version,  subRight);
		}
		else if (_T("ONVIF_OR_RTSP") == subLeft)
		{
			onvif_or_rtsp_flag = atoi(T2A(subRight));
		}
		else if (_T("net_mode") == subLeft)
		{
			net_mode = atoi(T2A(subRight));
		}
		else if (_T("update_find_flag") == subLeft)
		{
			update_find_flag = atoi(T2A(subRight));
		}
		else if (_T("image_x") == subLeft)
		{
			image_x = atoi(T2A(subRight));
		}
		else if (_T("image_y") == subLeft)
		{
			image_y = atoi(T2A(subRight));
		}

		else if (_T("uid_download_mode") == subLeft)
		{
			uid_download_mode = atoi(T2A(subRight));
		}
		else if (_T("uid_download_len") == subLeft)
		{
			uid_download_len = atoi(T2A(subRight));
		}
		else if (_T("need_tool_printf") == subLeft)
		{
			need_tool_printf = atoi(T2A(subRight));
		}

		else if (_T("need_close_monitor") == subLeft)
		{
			need_close_monitor = atoi(T2A(subRight));
		}

		

		
	}

	CloseHandle(hFile);

	return TRUE;
}


//保存配置文件，
//在设完成，进行保存
//在关闭烧录工具时，进行保存
BOOL CConfig_test::Write_Config(LPCTSTR file_path)
{
	CString str;
	INT len = 0;
	char buf[12] = {0};

	//获取属性
	DWORD faConfig = GetFileAttributes(ConvertAbsolutePath(file_path)); 
	if(0xFFFFFFFF != faConfig)
	{
		faConfig &= ~FILE_ATTRIBUTE_READONLY;//如果文件是只读，需要设非只读
		faConfig &= ~FILE_ATTRIBUTE_SYSTEM;  //如果文件是系统，那么设非系统
		faConfig &= ~FILE_ATTRIBUTE_TEMPORARY;//如果存在临时，那么也要设非临时
		SetFileAttributes(ConvertAbsolutePath(file_path), faConfig);
	}

	//open config file
	CStdioFile *pFile;
	pFile = new CStdioFile(ConvertAbsolutePath(file_path), 
		CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
	if(NULL == pFile)
	{
		return FALSE;
	}

	USES_CONVERSION;

#ifdef _UNICODE
	//USHORT head = UNICODE_TXT_HEAD;
	//pFile->Write(&head, 2);
#endif
	
	//config
	pFile->WriteString(_T("###config\r\n"));

	//IP address
	str.Format(_T("config_video_enable = %d\r\n"), config_video_enable);//平台类型
	pFile->WriteString(str);

	str.Format(_T("config_ircut_enable = %d\r\n"), config_ircut_enable);//平台类型
	pFile->WriteString(str);

	str.Format(_T("config_voice_rev_enable = %d\r\n"), config_voice_rev_enable);
	pFile->WriteString(str);

	//Config file name
	str.Format(_T("config_voice_send_enable = %d\r\n"), config_voice_send_enable);
	pFile->WriteString(str);

	str.Format(_T("config_head_enable = %d\r\n"), config_head_enable);
	pFile->WriteString(str);

	str.Format(_T("config_sd_enable = %d\r\n"), config_sd_enable);
	pFile->WriteString(str);

	str.Format(_T("config_wifi_enable = %d\r\n"), config_wifi_enable);
	pFile->WriteString(str);

	//str.Format(_T("config_red_line_enable = %d\r\n"), config_red_line_enable);
	//pFile->WriteString(str);

	str.Format(_T("config_reset_enable = %d\r\n"), config_reset_enable);
	pFile->WriteString(str);

	str.Format(_T("config_lan_mac_enable = %d\r\n"), config_lan_mac_enable);
	pFile->WriteString(str);

	str.Format(_T("config_uid_enable = %d\r\n"), config_uid_enable);
	pFile->WriteString(str);

	str.Format(_T("m_mac_start_addr = %s\r\n"), m_mac_start_addr);
	pFile->WriteString(str);

	str.Format(_T("m_mac_end_addr = %s\r\n"), m_mac_end_addr);
	pFile->WriteString(str);

	str.Format(_T("config_auto_login = %d\r\n"), config_auto_login);
	pFile->WriteString(str);

	str.Format(_T("m_wifi_name = %s\r\n"), m_wifi_name);
	pFile->WriteString(str);

	str.Format(_T("m_test_monitor_time = %s\r\n"), m_test_monitor_time);
	pFile->WriteString(str);

	str.Format(_T("m_test_reset_time = %s\r\n"), m_test_reset_time);
	pFile->WriteString(str);

	str.Format(_T("m_test_ircut_time = %s\r\n"), m_test_ircut_time);
	pFile->WriteString(str);

	str.Format(_T("newest_version = %s\r\n"), newest_version);
	pFile->WriteString(str);

	str.Format(_T("ONVIF_OR_RTSP = %d\r\n"), onvif_or_rtsp_flag);
	pFile->WriteString(str);

	str.Format(_T("net_mode = %d\r\n"), net_mode);
	pFile->WriteString(str);

	str.Format(_T("update_find_flag = %d\r\n"), update_find_flag);
	pFile->WriteString(str);

	str.Format(_T("image_x = %d\r\n"), image_x);
	pFile->WriteString(str);

	str.Format(_T("image_y = %d\r\n"), image_y);
	pFile->WriteString(str);

	str.Format(_T("uid_download_mode = %d\r\n"), uid_download_mode);
	pFile->WriteString(str);

	str.Format(_T("uid_download_len = %d\r\n"), uid_download_len);
	pFile->WriteString(str);

	str.Format(_T("need_tool_printf = %d\r\n"), need_tool_printf);
	pFile->WriteString(str);

	str.Format(_T("need_close_monitor = %d\r\n"), need_close_monitor);
	pFile->WriteString(str);

	
	pFile->WriteString(_T("\r\n"));

	pFile->Close();
	delete pFile;

	return TRUE;
}



//读取配置文件的参数
BOOL CConfig_test::Read_current_macConfig(LPCTSTR file_path)
{
	CString str;
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

		if (_T("m_mac_current_addr") == subLeft)
		{
			_tcscpy(m_mac_current_addr,  subRight);
		}
	}

	CloseHandle(hFile);

	return TRUE;
}


//保存配置文件，
//在设完成，进行保存
//在关闭烧录工具时，进行保存
BOOL CConfig_test::Write_current_macConfig(LPCTSTR file_path)
{
	CString str;
	INT len = 0;
	char buf[12] = {0};

	//获取属性
	DWORD faConfig = GetFileAttributes(ConvertAbsolutePath(file_path)); 
	if(0xFFFFFFFF != faConfig)
	{
		faConfig &= ~FILE_ATTRIBUTE_READONLY;//如果文件是只读，需要设非只读
		faConfig &= ~FILE_ATTRIBUTE_SYSTEM;  //如果文件是系统，那么设非系统
		faConfig &= ~FILE_ATTRIBUTE_TEMPORARY;//如果存在临时，那么也要设非临时
		SetFileAttributes(ConvertAbsolutePath(file_path), faConfig);
	}

	//open config file
	CStdioFile *pFile;
	pFile = new CStdioFile(ConvertAbsolutePath(file_path), 
		CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
	if(NULL == pFile)
	{
		return FALSE;
	}

	USES_CONVERSION;

#ifdef _UNICODE
	//USHORT head = UNICODE_TXT_HEAD;
	//pFile->Write(&head, 2);
#endif

	//config
	pFile->WriteString(_T("###config mac current\r\n"));

	str.Format(_T("m_mac_current_addr = %s\r\n"), m_mac_current_addr);
	pFile->WriteString(str);

	pFile->WriteString(_T("\r\n"));

	pFile->Close();
	delete pFile;

	return TRUE;
}



//读取配置文件的参数
BOOL CConfig_test::Read_printf_cfg(LPCTSTR file_path)
{
	CString str;
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

		if (_T("PrinterName") == subLeft)
		{
			_tcscpy(PrinterName,  subRight);
		}
		else if (_T("LabWidth") == subLeft)
		{
			LabWidth = atof(T2A(subRight));
		}
		else if (_T("LabHeight") == subLeft)
		{
			LabHeight = atof(T2A(subRight));
		}
		else if (_T("OffsetV") == subLeft)
		{
			OffsetV = atof(T2A(subRight));
		}
		else if (_T("OffsetH") == subLeft)
		{
			OffsetH = atof(T2A(subRight));
		}
		else if (_T("SN_OffsetV") == subLeft)
		{
			SN_OffsetV = atof(T2A(subRight));
		}
		else if (_T("SN_OffsetH") == subLeft)
		{
			SN_OffsetH = atof(T2A(subRight));
		}
		else if (_T("char_height") == subLeft)
		{
			char_height = atoi(T2A(subRight));
		}
		else if (_T("PID") == subLeft)
		{
			_tcscpy(PID,  subRight);

		}
		//0
		else if (_T("line_idex01") == subLeft)
		{
			config_printf_cfg[0].line_idex = atoi(T2A(subRight));
		}

		else if (_T("content_data01") == subLeft)
		{
			_tcscpy(config_printf_cfg[0].content_data,  subRight);
		}

		else if (_T("fon_height01") == subLeft)
		{
			config_printf_cfg[0].fon_height = atof(T2A(subRight));
		}
		//1
		else if (_T("line_idex02") == subLeft)
		{
			config_printf_cfg[1].line_idex = atoi(T2A(subRight));
		}

		else if (_T("content_data02") == subLeft)
		{
			_tcscpy(config_printf_cfg[1].content_data,  subRight);
		}

		else if (_T("fon_height02") == subLeft)
		{
			config_printf_cfg[1].fon_height = atof(T2A(subRight));
		}
		//2
		else if (_T("line_idex03") == subLeft)
		{
			config_printf_cfg[2].line_idex = atoi(T2A(subRight));
		}

		else if (_T("content_data03") == subLeft)
		{
			_tcscpy(config_printf_cfg[2].content_data,  subRight);
		}

		else if (_T("fon_height03") == subLeft)
		{
			config_printf_cfg[2].fon_height = atof(T2A(subRight));
		}
		//3
		else if (_T("line_idex04") == subLeft)
		{
			config_printf_cfg[3].line_idex = atoi(T2A(subRight));
		}

		else if (_T("content_data04") == subLeft)
		{
			_tcscpy(config_printf_cfg[3].content_data,  subRight);
		}

		else if (_T("fon_height04") == subLeft)
		{
			config_printf_cfg[3].fon_height = atof(T2A(subRight));
		}
		//4
		else if (_T("line_idex05") == subLeft)
		{
			config_printf_cfg[4].line_idex = atoi(T2A(subRight));
		}

		else if (_T("content_data05") == subLeft)
		{
			_tcscpy(config_printf_cfg[4].content_data,  subRight);
		}

		else if (_T("fon_height05") == subLeft)
		{
			config_printf_cfg[4].fon_height = atof(T2A(subRight));
		}

		else if (_T("page_num") == subLeft)
		{
			page_num = atoi(T2A(subRight));
		}
		else if (_T("is_printf_lab") == subLeft)
		{
			is_printf_lab = atoi(T2A(subRight));
		}
		



	}

	CloseHandle(hFile);

	return TRUE;
}


//保存配置文件，
//在设完成，进行保存
//在关闭烧录工具时，进行保存

BOOL CConfig_test::Write_printf_cfg(LPCTSTR file_path)
{
	CString str;
	UINT i = 0;

	//获取属性
	DWORD faConfig = GetFileAttributes(ConvertAbsolutePath(file_path)); 
	if(0xFFFFFFFF != faConfig)
	{
		faConfig &= ~FILE_ATTRIBUTE_READONLY;//如果文件是只读，需要设非只读
		faConfig &= ~FILE_ATTRIBUTE_SYSTEM;  //如果文件是系统，那么设非系统
		faConfig &= ~FILE_ATTRIBUTE_TEMPORARY;//如果存在临时，那么也要设非临时
		SetFileAttributes(ConvertAbsolutePath(file_path), faConfig);
	}

	//open config file
	CStdioFile *pFile;
	pFile = new CStdioFile(ConvertAbsolutePath(file_path), 
		CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
	if(NULL == pFile)
	{
		return FALSE;
	}

	USES_CONVERSION;

#ifdef _UNICODE
	//USHORT head = UNICODE_TXT_HEAD;
	//pFile->Write(&head, 2);
#endif

	//config
	pFile->WriteString(_T("###printf set\r\n"));

	str.Format(_T("PrinterName = %s\r\n"), PrinterName);
	pFile->WriteString(str);
	pFile->WriteString(_T("\r\n"));

	str.Format(_T("LabWidth = %0.2f\r\n"), LabWidth);
	pFile->WriteString(str);
	str.Format(_T("LabHeight = %0.2f\r\n"), LabHeight);
	pFile->WriteString(str);
	str.Format(_T("OffsetV = %0.2f\r\n"), OffsetV);
	pFile->WriteString(str);
	str.Format(_T("OffsetH = %0.2f\r\n"), OffsetH);
	pFile->WriteString(str);
	str.Format(_T("SN_OffsetV = %0.2f\r\n"), SN_OffsetV);
	pFile->WriteString(str);
	str.Format(_T("SN_OffsetH = %0.2f\r\n"), SN_OffsetH);
	pFile->WriteString(str);
	str.Format(_T("char_height = %d\r\n"), char_height);
	pFile->WriteString(str);

	

	str.Format(_T("PID = %s\r\n"), PID);
	pFile->WriteString(str);


	pFile->WriteString(_T("\r\n"));
	str.Format(_T("line_idex01 = %d\r\n"), config_printf_cfg[0].line_idex);
	pFile->WriteString(str);
	str.Format(_T("content_data01 = %s\r\n"), config_printf_cfg[0].content_data);
	pFile->WriteString(str);
	str.Format(_T("fon_height01 = %0.2f\r\n"), config_printf_cfg[0].fon_height);
	pFile->WriteString(str);

	pFile->WriteString(_T("\r\n"));
	str.Format(_T("line_idex02 = %d\r\n"), config_printf_cfg[1].line_idex);
	pFile->WriteString(str);
	str.Format(_T("content_data02 = %s\r\n"), config_printf_cfg[1].content_data);
	pFile->WriteString(str);
	str.Format(_T("fon_height02 = %0.2f\r\n"), config_printf_cfg[1].fon_height);
	pFile->WriteString(str);


	pFile->WriteString(_T("\r\n"));
	str.Format(_T("line_idex03 = %d\r\n"), config_printf_cfg[2].line_idex);
	pFile->WriteString(str);
	str.Format(_T("content_data03 = %s\r\n"), config_printf_cfg[2].content_data);
	pFile->WriteString(str);
	str.Format(_T("fon_height03 = %0.2f\r\n"), config_printf_cfg[2].fon_height);
	pFile->WriteString(str);

	pFile->WriteString(_T("\r\n"));
	str.Format(_T("line_idex04 = %d\r\n"), config_printf_cfg[3].line_idex);
	pFile->WriteString(str);
	str.Format(_T("content_data04 = %s\r\n"), config_printf_cfg[3].content_data);
	pFile->WriteString(str);
	str.Format(_T("fon_height04 = %0.2f\r\n"), config_printf_cfg[3].fon_height);
	pFile->WriteString(str);

	pFile->WriteString(_T("\r\n"));
	str.Format(_T("line_idex05 = %d\r\n"), config_printf_cfg[4].line_idex);
	pFile->WriteString(str);
	str.Format(_T("content_data05 = %s\r\n"), config_printf_cfg[4].content_data);
	pFile->WriteString(str);
	str.Format(_T("fon_height05 = %0.2f\r\n"), config_printf_cfg[4].fon_height);
	pFile->WriteString(str);

	str.Format(_T("page_num = %d\r\n"), page_num);
	pFile->WriteString(str);

	str.Format(_T("is_printf_lab = %d\r\n"), is_printf_lab);
	pFile->WriteString(str);

	
	

	pFile->WriteString(_T("\r\n"));

	pFile->Close();
	delete pFile;

	return TRUE;
}