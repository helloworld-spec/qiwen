// Test_ConfigDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Test_ConfigDlg.h"
#include "Config_test.h"
#include "Anyka IP CameraDlg.h"


extern CConfig_test g_test_config;
extern BOOL entern_flag;
extern BOOL first_flag;
extern BOOL init_flag;

// CTest_ConfigDlg 对话框

IMPLEMENT_DYNAMIC(CTest_ConfigDlg, CDialog)

CTest_ConfigDlg::CTest_ConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTest_ConfigDlg::IDD, pParent)
{
	UINT i = 0;
}

CTest_ConfigDlg::~CTest_ConfigDlg()
{
	UINT i = 0;
}

void CTest_ConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	if (entern_flag)
	{
		entern_flag = FALSE;
		Set_confim();
	}
	DDX_Control(pDX, IDC_TEST_TIME_MONITOR, m_limit_edit);
	DDX_Control(pDX, IDC_TEST_TIME_RESET, m_limit_edit1);
}


BEGIN_MESSAGE_MAP(CTest_ConfigDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CTest_ConfigDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_VIDEO, &CTest_ConfigDlg::OnBnClickedCheckVideo)
	ON_BN_CLICKED(IDC_CHECK_MONITORING, &CTest_ConfigDlg::OnBnClickedCheckMonitoring)
	ON_BN_CLICKED(IDC_CHECK_INTERCOM, &CTest_ConfigDlg::OnBnClickedCheckIntercom)
	ON_BN_CLICKED(IDC_CHECK_CRADLE_HEAD, &CTest_ConfigDlg::OnBnClickedCheckCradleHead)
	ON_BN_CLICKED(IDC_CHECK_SDCARD, &CTest_ConfigDlg::OnBnClickedCheckSdcard)
	ON_BN_CLICKED(IDC_CHECK_WIFI, &CTest_ConfigDlg::OnBnClickedCheckWifi)
	//ON_BN_CLICKED(IDC_CHECK_INFRARED, &CTest_ConfigDlg::OnBnClickedCheckInfrared)
	ON_BN_CLICKED(IDC_CHECK_RESET, &CTest_ConfigDlg::OnBnClickedCheckReset)
	ON_BN_CLICKED(IDC_CHECK_MAC, &CTest_ConfigDlg::OnBnClickedCheckMac)
	ON_BN_CLICKED(IDC_CHECK_BURN_UID, &CTest_ConfigDlg::OnBnClickedCheckBurnUid)
	ON_BN_CLICKED(IDC_CHECK_IRCUT, &CTest_ConfigDlg::OnBnClickedCheckIrcut)
	ON_EN_CHANGE(IDC_EDIT_START_MAC, &CTest_ConfigDlg::OnEnChangeEditStartMac)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDCANCEL, &CTest_ConfigDlg::OnBnClickedCancel)
	ON_EN_CHANGE(IDC_TEST_TIME_MONITOR, &CTest_ConfigDlg::OnEnChangeTestTimeMonitor)
	ON_EN_CHANGE(IDC_TEST_TIME_RESET, &CTest_ConfigDlg::OnEnChangeTestTimeReset)
//	ON_EN_CHANGE(IDC_EDIT_NEWEST_VERSION, &CTest_ConfigDlg::OnEnChangeEditNewestVersion)
ON_EN_CHANGE(IDC_EDIT_END_MAC, &CTest_ConfigDlg::OnEnChangeEditEndMac)
ON_EN_CHANGE(IDC_TEST_TIME_IRCUT, &CTest_ConfigDlg::OnEnChangeTestTimeIrcut)
ON_BN_CLICKED(IDC_CHECK1, &CTest_ConfigDlg::OnBnClickedCheck1)
ON_BN_CLICKED(IDC_CHECK_NEED_PRINTF, &CTest_ConfigDlg::OnBnClickedCheckNeedPrintf)
ON_BN_CLICKED(IDC_CHECK_NEED_CLOSE_MONITOR, &CTest_ConfigDlg::OnBnClickedCheckNeedCloseMonitor)
ON_BN_CLICKED(IDC_RADIO_FIND_FILE, &CTest_ConfigDlg::OnBnClickedRadioFindFile)
ON_BN_CLICKED(IDC_RADIO_CHAR, &CTest_ConfigDlg::OnBnClickedRadioChar)
END_MESSAGE_MAP()


void CTest_ConfigDlg::Set_confim()
{
	CString str;
	((CButton *)GetDlgItem(IDC_CHECK_VIDEO))->SetCheck(g_test_config.config_video_enable);
	((CButton *)GetDlgItem(IDC_CHECK_MONITORING))->SetCheck(g_test_config.config_voice_rev_enable);
	((CButton *)GetDlgItem(IDC_CHECK_INTERCOM))->SetCheck(g_test_config.config_voice_send_enable);
	((CButton *)GetDlgItem(IDC_CHECK_CRADLE_HEAD))->SetCheck(g_test_config.config_head_enable);
	((CButton *)GetDlgItem(IDC_CHECK_SDCARD))->SetCheck(g_test_config.config_sd_enable);
	((CButton *)GetDlgItem(IDC_CHECK_WIFI))->SetCheck(g_test_config.config_wifi_enable);
	//((CButton *)GetDlgItem(IDC_CHECK_INFRARED))->SetCheck(g_test_config.config_red_line_enable);
	((CButton *)GetDlgItem(IDC_CHECK_RESET))->SetCheck(g_test_config.config_reset_enable);
	((CButton *)GetDlgItem(IDC_CHECK_MAC))->SetCheck(g_test_config.config_lan_mac_enable);
	((CButton *)GetDlgItem(IDC_CHECK_BURN_UID))->SetCheck(g_test_config.config_uid_enable);
	((CButton *)GetDlgItem(IDC_CHECK_IRCUT))->SetCheck(g_test_config.config_ircut_enable);

	((CEdit *)GetDlgItem(IDC_TEST_TIME_MONITOR))->SetWindowText(g_test_config.m_test_monitor_time);
	((CEdit *)GetDlgItem(IDC_TEST_TIME_RESET))->SetWindowText(g_test_config.m_test_reset_time);
	((CEdit *)GetDlgItem(IDC_TEST_TIME_IRCUT))->SetWindowText(g_test_config.m_test_ircut_time);
	//((CEdit *)GetDlgItem(IDC_EDIT_NEWEST_VERSION))->SetWindowText(g_test_config.newest_version);

	((CButton *)GetDlgItem(IDC_CHECK_NEED_PRINTF))->SetCheck(g_test_config.need_tool_printf);
	((CButton *)GetDlgItem(IDC_CHECK_NEED_CLOSE_MONITOR))->SetCheck(g_test_config.need_close_monitor);

	

	SetDlgItemText(IDC_EDIT_START_MAC, g_test_config.m_mac_start_addr);
	SetDlgItemText(IDC_EDIT_END_MAC, g_test_config.m_mac_end_addr);
	SetDlgItemText(IDC_EDIT_NEWEST_VERSION, g_test_config.newest_version);



	if (g_test_config.uid_download_mode == 1)
	{
		((CButton *)GetDlgItem(IDC_RADIO_FIND_FILE))->SetCheck(TRUE);
		((CButton *)GetDlgItem(IDC_RADIO_CHAR))->SetCheck(FALSE);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_CHAR))->SetCheck(TRUE);
		((CButton *)GetDlgItem(IDC_RADIO_FIND_FILE))->SetCheck(FALSE);
	}

	str.Format(_T("%d"), g_test_config.uid_download_len);
	SetDlgItemText(IDC_EDIT_BURN_CHAR_LEN, str);

}

// CTest_ConfigDlg 消息处理程序
void CTest_ConfigDlg::OnBnClickedOk()
{
	CString str,strMonitor,strReset,strVersion;
	UINT len= 0, i = 0;

	USES_CONVERSION;

	if (g_test_config.config_uid_enable)
	{
		GetDlgItemText(IDC_EDIT_BURN_CHAR_LEN, str);
		if (str.IsEmpty())
		{
			AfxMessageBox(_T("字符长度为空,请检查"));
			return;
		}

		g_test_config.uid_download_len = atoi(T2A(str));
		if (g_test_config.uid_download_len == 0 || g_test_config.uid_download_len > 256)
		{
			AfxMessageBox(_T("字符长度的值有错,范围应是1~256"));
			return;
		}
	}
	
	if(g_test_config.config_lan_mac_enable)
	{
		GetDlgItemText(IDC_EDIT_START_MAC, str);
		if(str.IsEmpty())
		{
			AfxMessageBox(_T("开始地址不能为空，请检查"), MB_OK);
			return;
		}

		if (str.GetLength() != 17)
		{
			AfxMessageBox(_T("开始地址长度有错，请检查"), MB_OK);
			return;
		}
		_tcscpy(g_test_config.m_mac_start_addr,str);

		GetDlgItemText(IDC_EDIT_END_MAC, str);
		if(str.IsEmpty())
		{
			AfxMessageBox(_T("结束地址不能为空，请检查"), MB_OK);
			return;
		}
		if (str.GetLength() != 17)
		{
			AfxMessageBox(_T("结束地址长度有错，请检查"), MB_OK);
			return;
		}
		_tcscpy(g_test_config.m_mac_end_addr,str);
	}

	GetDlgItemText(IDC_TEST_TIME_IRCUT, str);
	if (str.IsEmpty())
	{
		AfxMessageBox(_T("IRCUT切换时间不能为空，建议为3"), MB_OK);
		return;
	}


	GetDlgItemText(IDC_TEST_TIME_MONITOR, strMonitor);
	GetDlgItemText(IDC_TEST_TIME_RESET, strReset);
	_tcscpy(g_test_config.m_test_reset_time, strReset);

	GetDlgItemText(IDC_EDIT_NEWEST_VERSION, strVersion);
	if(strVersion.IsEmpty())
	{
		AfxMessageBox(_T("版本号不能为空，请检查"), MB_OK);
		return;
	}

	memset(g_test_config.newest_version, 0, MAC_ADDRESS_LEN);
	_tcscpy(g_test_config.newest_version,strVersion);



	if(g_test_config.config_lan_mac_enable)
	{
		for(i = 0; i < 8; i++)
		{
			if(g_test_config.m_mac_start_addr[i] != g_test_config.m_mac_end_addr[i])
			{
				AfxMessageBox(_T("开始地址比结束地址的前缀不一样，请检查"), MB_OK);
				return;
			}
		}

		for(i = 0; i < 8; i++)
		{
			if(g_test_config.m_mac_current_addr[i] != 0 && g_test_config.m_mac_current_addr[i] != g_test_config.m_mac_end_addr[i])
			{
				//AfxMessageBox(_T("当前地址与设置地址前缀不一样"), MB_OK);
				len = _tcsclen(g_test_config.m_mac_start_addr);
				memset(g_test_config.m_mac_current_addr, 0, MAX_PATH);
				_tcsncpy(g_test_config.m_mac_current_addr, g_test_config.m_mac_start_addr, len);
			}
		}

		if (_tcscmp(g_test_config.m_mac_start_addr, g_test_config.m_mac_end_addr) > 0)
		{
			AfxMessageBox(_T("开始地址比结束地址还大或相等，请检查"), MB_OK);
			return;
		}

		if (_tcscmp(g_test_config.m_mac_current_addr, g_test_config.m_mac_end_addr) > 0)
		{
			AfxMessageBox(_T("当前地址比结束地址还大或相等，请检查"), MB_OK);
			return;
		}
	if (_tcscmp(g_test_config.m_mac_start_addr, g_test_config.m_mac_current_addr) > 0)
	{
		len = _tcsclen(g_test_config.m_mac_start_addr);
		memset(g_test_config.m_mac_current_addr, 0, MAX_PATH);
		_tcsncpy(g_test_config.m_mac_current_addr, g_test_config.m_mac_start_addr, len);
		}
	}

	g_test_config.Write_Config(TEST_CONFIG_CASE);
	first_flag = FALSE;
	init_flag=TRUE;
	// TODO: 在此添加控件通知处理程序代码
	OnOK();
}

void CTest_ConfigDlg::OnBnClickedCheckVideo()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_VIDEO))->GetCheck();
	if (flag)
	{
		g_test_config.config_video_enable = TRUE;
	} 
	else
	{
		g_test_config.config_video_enable = FALSE;
	}
	

}

void CTest_ConfigDlg::OnBnClickedCheckMonitoring()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_MONITORING))->GetCheck();
	if (flag)
	{
		g_test_config.config_voice_rev_enable = TRUE;
	} 
	else
	{
		g_test_config.config_voice_rev_enable = FALSE;
	}
}

void CTest_ConfigDlg::OnBnClickedCheckIntercom()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_INTERCOM))->GetCheck();
	if (flag)
	{
		g_test_config.config_voice_send_enable = TRUE;
	} 
	else
	{
		g_test_config.config_voice_send_enable = FALSE;
	}
}

void CTest_ConfigDlg::OnBnClickedCheckCradleHead()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_CRADLE_HEAD))->GetCheck();
	if (flag)
	{
		g_test_config.config_head_enable = TRUE;
	} 
	else
	{
		g_test_config.config_head_enable = FALSE;
	}
}

void CTest_ConfigDlg::OnBnClickedCheckSdcard()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_SDCARD))->GetCheck();
	if (flag)
	{
		g_test_config.config_sd_enable = TRUE;
	} 
	else
	{
		g_test_config.config_sd_enable = FALSE;
	}
}

void CTest_ConfigDlg::OnBnClickedCheckWifi()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_WIFI))->GetCheck();
	if (flag)
	{
		g_test_config.config_wifi_enable = TRUE;
	} 
	else
	{
		g_test_config.config_wifi_enable = FALSE;
	}
}

//void CTest_ConfigDlg::OnBnClickedCheckInfrared()
//{
//	// TODO: 在此添加控件通知处理程序代码
//	BOOL flag = FALSE;
//
//	flag = ((CButton *)GetDlgItem(IDC_CHECK_INFRARED))->GetCheck();
//	if (flag)
//	{
//		g_test_config.config_red_line_enable = TRUE;
//	} 
//	else
//	{
//		g_test_config.config_red_line_enable = FALSE;
//	}
//}

void CTest_ConfigDlg::OnBnClickedCheckReset()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_RESET))->GetCheck();
	if (flag)
	{
		g_test_config.config_reset_enable = TRUE;
	} 
	else
	{
		g_test_config.config_reset_enable = FALSE;
	}
}

void CTest_ConfigDlg::OnBnClickedCheckMac()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_MAC))->GetCheck();
	if (flag)
	{
		g_test_config.config_lan_mac_enable = TRUE;
	} 
	else
	{
		g_test_config.config_lan_mac_enable = FALSE;
	}
}

void CTest_ConfigDlg::OnBnClickedCheckBurnUid()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_BURN_UID))->GetCheck();
	if (flag)
	{
		g_test_config.config_uid_enable = TRUE;
	} 
	else
	{
		g_test_config.config_uid_enable = FALSE;
	}
}

void CTest_ConfigDlg::OnBnClickedCheckIrcut()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_IRCUT))->GetCheck();
	if (flag)
	{
		g_test_config.config_ircut_enable = TRUE;
	} 
	else
	{
		g_test_config.config_ircut_enable = FALSE;
	}
}

BOOL CTest_ConfigDlg::Mac_iserror(UINT nID, UINT len, CString str, TCHAR *pBuf)
{
	UINT i = 0;
	UINT s = 0;
	BOOL falg = TRUE;

	for (i = 0; i < len; i++)
	{
		if (((i != 2) && (i != 5) && (i != 8) && (i != 11) && (i != 14)) 
			&& ((str.GetAt(i) < '0') || ((str.GetAt(i) > '9')
			&& (str.GetAt(i) < 'A')) || ((str.GetAt(i) > 'F')
			&& (str.GetAt(i) < 'a')) || (str.GetAt(i) > 'f')))
		{
			AfxMessageBox(_T("输入的mac地址有错，请检查"), MB_OK);
			str.SetAt(i, pBuf[i]);
			SetDlgItemText(nID, str);
			((CEdit*)GetDlgItem(nID))->SetSel( i, i+1, FALSE);
			falg = FALSE;
			break;
		}
		if ((i == 2 && (str.GetAt(i) != ':')) 
			|| (i == 5 && str.GetAt(i) != ':')
			|| (i == 8 && str.GetAt(i) != ':')
			|| (i == 11 && str.GetAt(i) != ':')
			|| (i == 14 && str.GetAt(i) != ':'))
		{
			GetDlgItemText(nID, str);	
			AfxMessageBox(_T("不符合MAC地址格式，请检查"), MB_OK);			
			str.SetAt(i, pBuf[i]);
			SetDlgItemText(nID, str);
			((CEdit*)GetDlgItem(nID))->SetSel( i+1, i+1, FALSE);
			falg = FALSE;
			break;
		}
		if (islower(str.GetAt(i)))
		{	
			str.SetAt(i, toupper(str.GetAt(i)));
			GetDlgItem(nID)->SetWindowText(str);
			((CEdit*)GetDlgItem(nID))->SetSel( i+1, i+1, FALSE );
		}

	}
	return falg;
}

void CTest_ConfigDlg::OnEnChangeEditStartMac()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString str;
	UINT i = 0;
	UINT s = 0;
	BOOL falg = TRUE;

	GetDlgItemText(IDC_EDIT_START_MAC, str);
	if (!str.IsEmpty())
	{		
		UINT nLen = str.GetLength();

		if (!Mac_iserror( IDC_EDIT_START_MAC, nLen, str,  g_test_config.m_mac_start_addr))
		{
			falg = FALSE;
		}
	}

}

void CTest_ConfigDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnClose();
}

void CTest_ConfigDlg::OnBnClickedCancel()
{
	first_flag = FALSE;
	// TODO: 在此添加控件通知处理程序代码
	OnCancel();
}

BOOL CTest_ConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	GetDlgItem(IDC_TEST_TIME_MONITOR)->SetWindowText(g_test_config.m_test_monitor_time);
	GetDlgItem(IDC_TEST_TIME_RESET)->SetWindowText(g_test_config.m_test_reset_time);

	if(pP->user_producer==TRUE)
	{
		GetDlgItem(IDC_CHECK_VIDEO)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_IRCUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_MONITORING)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_INTERCOM)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_CRADLE_HEAD)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_SDCARD)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_WIFI)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_RESET)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_MAC)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_BURN_UID)->EnableWindow(FALSE);
		GetDlgItem(IDC_TEST_TIME_MONITOR)->EnableWindow(FALSE);
		GetDlgItem(IDC_TEST_TIME_IRCUT)->EnableWindow(FALSE);
		GetDlgItem(IDC_TEST_TIME_RESET)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_NEWEST_VERSION)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_START_MAC)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_END_MAC)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_NEED_PRINTF)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_NEED_CLOSE_MONITOR)->EnableWindow(FALSE);

		GetDlgItem(IDC_RADIO_FIND_FILE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_BURN_CHAR_LEN)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_CHAR)->EnableWindow(FALSE);
	}

	return TRUE; // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CTest_ConfigDlg::OnEnChangeTestTimeMonitor()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString str;

	GetDlgItemText(IDC_TEST_TIME_MONITOR, str);
	if (!str.IsEmpty())
	{
		memset(g_test_config.m_test_monitor_time, 0, MAC_ADDRESS_LEN);
		_tcscpy(g_test_config.m_test_monitor_time, str);
	}
}

void CTest_ConfigDlg::OnEnChangeTestTimeReset()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString str;

	GetDlgItemText(IDC_TEST_TIME_RESET, str);
	if (!str.IsEmpty())
	{
		memset(g_test_config.m_test_reset_time, 0, MAC_ADDRESS_LEN);
		_tcscpy(g_test_config.m_test_reset_time, str);
	}
}
void CTest_ConfigDlg::OnEnChangeEditEndMac()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString str;
	UINT i = 0;
	UINT s = 0;
	BOOL falg = TRUE;

	GetDlgItemText(IDC_EDIT_END_MAC, str);
	if (!str.IsEmpty())
	{		
		UINT nLen = str.GetLength();

		if (!Mac_iserror( IDC_EDIT_END_MAC, nLen, str,  g_test_config.m_mac_end_addr))
		{
			falg = FALSE;
		}
	}
}

void CTest_ConfigDlg::OnEnChangeTestTimeIrcut()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString str;
	UINT len = 0;

	GetDlgItemText(IDC_TEST_TIME_IRCUT, str);
	if (!str.IsEmpty())
	{
		len = str.GetLength();
		if (len >= 2)
		{
			AfxMessageBox(_T("此值范围为1~8"), MB_OK);
			SetDlgItemText(IDC_TEST_TIME_IRCUT, _T(""));
			return;
		}
		if ((str.GetAt(0) == '0') || (str.GetAt(0) == '9'))
		{
			AfxMessageBox(_T("此值范围为1~8"), MB_OK);
			SetDlgItemText(IDC_TEST_TIME_IRCUT, _T(""));
			return;
		}
		memset(g_test_config.m_test_ircut_time, 0, 10);
		_tcscpy(g_test_config.m_test_ircut_time, str);
	}
}

void CTest_ConfigDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CTest_ConfigDlg::OnBnClickedCheckNeedPrintf()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL ret =FALSE;

	ret = ((CButton *)GetDlgItem(IDC_CHECK_NEED_PRINTF))->GetCheck();
	if (ret)
	{
		g_test_config.need_tool_printf = TRUE;
	}
	else
	{
		g_test_config.need_tool_printf = FALSE;
	}
}

void CTest_ConfigDlg::OnBnClickedCheckNeedCloseMonitor()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL ret =FALSE;

	ret = ((CButton *)GetDlgItem(IDC_CHECK_NEED_CLOSE_MONITOR))->GetCheck();
	if (ret)
	{
		g_test_config.need_close_monitor = TRUE;
	}
	else
	{
		g_test_config.need_close_monitor = FALSE;
	}
}

void CTest_ConfigDlg::OnBnClickedRadioFindFile()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_RADIO_FIND_FILE))->GetCheck();
	if (flag)
	{
		g_test_config.uid_download_mode = 1;
	} 
	else
	{
		g_test_config.uid_download_mode = 0;
	}
}

void CTest_ConfigDlg::OnBnClickedRadioChar()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_RADIO_CHAR))->GetCheck();
	if (flag)
	{
		g_test_config.uid_download_mode = 0;
	} 
	else
	{
		g_test_config.uid_download_mode = 1;
	}
}
