// RightCtrlDlg.cpp : 实现文件

#include "stdafx.h"
#include "Anyka IP Camera.h"
#include "Anyka IP CameraDlg.h"
#include "RightCtrlDlg.h"
#include "Config_test.h"


// CRightCtrlDlg 对话框

extern BOOL no_put_flie_flag ;
extern BOOL g_test_finish_flag ;
extern CConfig_test g_test_config;
extern BOOL g_pre_flag;
extern BOOL g_test_monitor_flag;
extern BOOL g_update_all_flag;
extern BOOL g_finish_flag;
extern BOOL one_update_finish ;

extern char g_update_flag[UPDATE_MAX_NUM];

IMPLEMENT_DYNAMIC(CRightCtrlDlg, CDialog)

CRightCtrlDlg::CRightCtrlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRightCtrlDlg::IDD, pParent)
{

}

CRightCtrlDlg::~CRightCtrlDlg()
{
}

void CRightCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_test_config);
	DDX_Control(pDX, IDC_BUTTON_PASS, m_test_pass_btn);
	//DDX_Control(pDX, IDC_LIST_WIFI, m_test_wifi_list);
	DDX_Control(pDX, IDC_BUTTON_FAILED, m_test_fail_btn);
}


BEGIN_MESSAGE_MAP(CRightCtrlDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_PASS, &CRightCtrlDlg::OnBnClickedButtonPass)
	ON_BN_CLICKED(IDC_BUTTON_FAILED, &CRightCtrlDlg::OnBnClickedButtonFailed)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CRightCtrlDlg::OnLvnItemchangedList1)
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_STATIC_TEST_CONTENT, &CRightCtrlDlg::OnStnClickedStaticTestContent)
	ON_NOTIFY(NM_SETFOCUS, IDC_LIST1, &CRightCtrlDlg::OnNMSetfocusList1)
	ON_BN_CLICKED(IDC_BUTTON_FIND_IP, &CRightCtrlDlg::OnBnClickedButtonFindIp)
	ON_BN_CLICKED(IDC_BUTTON_PRE_TEST, &CRightCtrlDlg::OnBnClickedButtonPreTest)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_WIFI, &CRightCtrlDlg::OnLvnItemchangedListWifi)
	ON_BN_CLICKED(IDC_CHECK_SD_TEST, &CRightCtrlDlg::OnBnClickedCheckSdTest)
	ON_BN_CLICKED(IDC_CHECK_WIFI_TEST, &CRightCtrlDlg::OnBnClickedCheckWifiTest)
	ON_BN_CLICKED(IDC_RADIO_RTSP, &CRightCtrlDlg::OnBnClickedRadioRtsp)
	ON_BN_CLICKED(IDC_RADIO_ONVIF, &CRightCtrlDlg::OnBnClickedRadioOnvif)
	ON_BN_CLICKED(IDC_BUTTON_SD_RETEST, &CRightCtrlDlg::OnBnClickedButtonSdRetest)
	ON_BN_CLICKED(IDC_BUTTON_WIFI_RETEST, &CRightCtrlDlg::OnBnClickedButtonWifiRetest)
	ON_BN_CLICKED(IDC_CHECK_ALL, &CRightCtrlDlg::OnBnClickedCheckAll)
	ON_BN_CLICKED(IDC_BUTTON1, &CRightCtrlDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_ALL, &CRightCtrlDlg::OnBnClickedButtonUpdateAll)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_FLAG, &CRightCtrlDlg::OnBnClickedCheckUpdateFlag)
	ON_BN_CLICKED(IDC_RADIO_NET, &CRightCtrlDlg::OnBnClickedRadioNet)
	ON_BN_CLICKED(IDC_RADIO_WIFI, &CRightCtrlDlg::OnBnClickedRadioWifi)
	ON_STN_CLICKED(IDC_STATIC_TEST_TITLE, &CRightCtrlDlg::OnStnClickedStaticTestTitle)
	ON_STN_CLICKED(IDC_STATIC1, &CRightCtrlDlg::OnStnClickedStatic1)
	ON_STN_CLICKED(IDC_STATIC2, &CRightCtrlDlg::OnStnClickedStatic2)
	ON_STN_CLICKED(IDC_STATIC_text_wifi, &CRightCtrlDlg::OnStnClickedStatictextwifi)
	ON_STN_CLICKED(IDC_STATIC_TEST_VIDEO, &CRightCtrlDlg::OnStnClickedStaticTestVideo)
END_MESSAGE_MAP()


// CRightCtrlDlg 消息处理程序

void CRightCtrlDlg::OnBnClickedButtonPass()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->m_RightDlg.m_test_Status.SetFontColor(0);
	pP->m_RightDlg.m_test_2_Status.SetFontColor(0);
	pP->case_main(TRUE);
	m_test_pass_btn.SetFocus();
}

void CRightCtrlDlg::OnBnClickedButtonFailed()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->m_RightDlg.m_test_Status.SetFontColor(0);
	pP->m_RightDlg.m_test_2_Status.SetFontColor(0);
	pP->case_main(FALSE);
	m_test_pass_btn.SetFocus();
}

void CRightCtrlDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->show_IP_info();
}


BOOL CRightCtrlDlg::PreTranslateMessage(MSG*pMsg)
{
	
	m_ToolTip.RelayEvent(pMsg);
	
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL CRightCtrlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	MSG* pMsg;
	// TODO:  在此添加额外的初始化

	m_ToolTip.Create(this);
	m_ToolTip.SetDelayTime(100);
	m_ToolTip.SetMaxTipWidth(300);
	m_ToolTip.AddTool(GetDlgItem(IDC_STATIC_laba),_T("当IPC测试机在开机时，是否能听到播放出来的声音，如网络连接成功等"));
	m_ToolTip.AddTool(GetDlgItem(IDC_STATIC_yuntai),_T("当IPC测试机在开机时，云台会进行向左向右，向上向下各自转一次"));
	m_ToolTip.AddTool(GetDlgItem(IDC_STATIC_wifi),_T("从IPC测试机上获取记录wifi模块扫描出来的wifi名和wifi信号强度信息"));
	m_ToolTip.AddTool(GetDlgItem(IDC_STATIC_sd),_T("从IPC测试机上获取记录SD卡的容量大小和测试结果"));
	m_ToolTip.AddTool(GetDlgItem(IDC_STATIC_jianting),_T("对着IPC测试机说话，通过电脑的耳机或音箱，听听是否能正常听到声音"));
	m_ToolTip.AddTool(GetDlgItem(IDC_STATIC_duijiang),_T("IPC测试机和电脑端能否互相听见声音"));
	m_ToolTip.AddTool(GetDlgItem(IDC_STATIC_shipin),_T("图像是否正常，是否存在有黑点等等"));
	m_ToolTip.AddTool(GetDlgItem(IDC_STATIC_ircut),_T("用手挡住光敏电阻，人为确定图像是否变成黑白再变彩色、红外灯亮起"));
	m_ToolTip.AddTool(GetDlgItem(IDC_STATIC_fuwei),_T("按下复位键，看看工具测试结果区域是否显示测试成功"));
	m_ToolTip.AddTool(GetDlgItem(IDC_STATIC_burnMAC),_T("把配置好的MAC地址写入到IPC测试机上"));
	m_ToolTip.AddTool(GetDlgItem(IDC_STATIC_burnUID),_T("把通过扫描枪或人工输入的UID，写入到IPC测试机上"));
 

	SetTimer(TIMER_COMMAND, 1000, NULL);

	m_Status.SubclassDlgItem(IDC_STATIC_TEST_CONTENT,this);
	m_test_Status.SubclassDlgItem(IDC_STATIC_TEST_POINT1,this);
	m_test_2_Status.SubclassDlgItem(IDC_STATIC_TEST_POINT2,this);

	if(g_test_config.onvif_or_rtsp_flag)
	{
		((CButton *)GetDlgItem(IDC_RADIO_RTSP))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_ONVIF))->SetCheck(0);
		g_test_config.onvif_or_rtsp_flag = TRUE;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_RTSP))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_ONVIF))->SetCheck(1);
		g_test_config.onvif_or_rtsp_flag = FALSE;
	}

	if(g_test_config.net_mode)
	{
		((CButton *)GetDlgItem(IDC_RADIO_WIFI))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_NET))->SetCheck(0);
		g_test_config.net_mode = TRUE;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_WIFI))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_NET))->SetCheck(1);
		g_test_config.net_mode = FALSE;
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

HBRUSH CRightCtrlDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
#if 0
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_STATIC_TEST_TITLE:
		case IDC_STATIC_TEST_CONTENT:
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(0,255,0));
			
	
			return (HBRUSH)GetStockObject(HOLLOW_BRUSH);
		
	}
#endif
	// TODO:  在此更改 DC 的任何属性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CRightCtrlDlg::OnStnClickedStaticTestContent()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CRightCtrlDlg::OnNMSetfocusList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CRightCtrlDlg::OnBnClickedButtonFindIp()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	GetDlgItem(IDC_BUTTON_FIND_IP)->EnableWindow(FALSE);
	pP->m_RightDlg.m_test_Status.SetFontColor(0);
	//pP->m_RightDlg.m_test_2_Status.SetFontColor(0);
	//pP->m_RightDlg.GetDlgItem(IDC_STATIC_TEST_POINT1)->SetFont(&pP->font);
	//pP->m_RightDlg.GetDlgItem(IDC_STATIC_TEST_POINT2)->SetFont(&pP->font);
	//GetDlgItem(IDC_STATIC_TEST_POINT1)->SetWindowText(_T(""));//可用 
	//GetDlgItem(IDC_STATIC_TEST_POINT2)->SetWindowText(_T(""));
	//GetDlgItem(IDC_STATIC_TEST_CONTENT)->SetWindowText(_T(""));
	//GetDlgItem(IDC_STATIC_TEST_TITLE)->SetWindowText(_T("图像测试"));


	g_test_config.m_test_sd_flag = 0; //sd, 0表示没有测试，1表成测试成功，2表示测试失败
	g_test_config.m_test_wifi_flag = 0; //wifi,0表示没有测试，1表成测试成功，2表示测试失败
	g_test_config.m_test_reset_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	g_test_config.m_test_ircut_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	g_test_config.m_test_MAC_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	g_test_config.m_test_UID_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	g_test_config.m_test_speaker_flag = 0; //0表示没有测试，1表成测试成功，2表示测试失败
	g_test_config.m_test_monitor_flag = 0; //监听，0表示没有测试，1表成测试成功，2表示测试失败
	g_test_config.m_test_cloud_flag = 0; //云台， 0表示没有测试，1表成测试成功，2表示测试失败
	g_test_config.m_test_play_flag = 0; //对讲，0表示没有测试，1表成测试成功，2表示测试失败


	g_test_config.m_sd_size = 0;
	memset(g_test_config.m_ssid_info, 0, 1024*sizeof(T_SSID_INFO));
	g_test_config.m_ssid_num = 0;

	m_test_config.DeleteAllItems();
	pP->m_BottomDlg.m_test_wifi_list.DeleteAllItems();
	g_test_monitor_flag = FALSE;
	g_update_all_flag = FALSE;
	g_finish_flag = FALSE;
	pP->find_IP_CloseServer(0);
	pP->On_find_ip();
	pP->next_test_flag = TRUE;
}

void CRightCtrlDlg::OnBnClickedButtonPreTest()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	GetDlgItem(IDC_BUTTON_PASS)->EnableWindow(TRUE);//可用
	GetDlgItem(IDC_BUTTON_FAILED)->EnableWindow(TRUE);//可用
	GetDlgItem(IDC_BUTTON_PRE_TEST)->EnableWindow(TRUE);//可用
	GetDlgItem(IDC_BUTTON_FAILED)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BUTTON_PASS)->ShowWindow(SW_SHOW);
	if(no_put_flie_flag)
	{
		no_put_flie_flag = false;
	}
	else
	{
		no_put_flie_flag = true;
	}
	g_test_finish_flag = false;
	pP->pre_case_main(TRUE);
	m_test_pass_btn.SetFocus();

}

void CRightCtrlDlg::OnLvnItemchangedListWifi(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CRightCtrlDlg::OnBnClickedCheckSdTest()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_SD_TEST))->GetCheck();
	if (flag)
	{
		g_test_config.sd_reset_flag = TRUE;
	} 
	else
	{
		g_test_config.sd_reset_flag = FALSE;
	}
	
}

void CRightCtrlDlg::OnBnClickedCheckWifiTest()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_CHECK_WIFI_TEST))->GetCheck();
	if (flag)
	{
		g_test_config.wifi_reset_flag = TRUE;
	} 
	else
	{
		g_test_config.wifi_reset_flag = FALSE;
	}
}

void CRightCtrlDlg::OnBnClickedRadioRtsp()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_RADIO_RTSP))->GetCheck();
	if (flag)
	{
		g_test_config.onvif_or_rtsp_flag = TRUE;
	} 
	else
	{
		g_test_config.onvif_or_rtsp_flag = FALSE;
	}
}

void CRightCtrlDlg::OnBnClickedRadioOnvif()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL flag = FALSE;

	flag = ((CButton *)GetDlgItem(IDC_RADIO_ONVIF))->GetCheck();
	if (flag)
	{
		g_test_config.onvif_or_rtsp_flag = FALSE;
	} 
	else
	{
		g_test_config.onvif_or_rtsp_flag = TRUE;
	}
}

void CRightCtrlDlg::OnBnClickedButtonSdRetest()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	GetDlgItem(IDC_BUTTON_SD_RETEST)->EnableWindow(FALSE);//可用
	pP->m_RightDlg.m_test_Status.SetFontColor(0);
	g_test_config.m_test_sd_flag = 0; //sd, 0表示没有测试，1表成测试成功，2表示测试失败
	pP->Creat_retest_sd_thread();
}

void CRightCtrlDlg::OnBnClickedButtonWifiRetest()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	GetDlgItem(IDC_BUTTON_WIFI_RETEST)->EnableWindow(FALSE);//可用
	pP->m_RightDlg.m_test_Status.SetFontColor(0);
	pP->m_BottomDlg.m_test_wifi_list.DeleteAllItems();
	g_test_config.m_test_wifi_flag = 0; //wifi,0表示没有测试，1表成测试成功，2表示测试失败
	pP->Creat_retest_wifi_thread();
}

void CRightCtrlDlg::OnBnClickedCheckAll()
{
	// TODO: 在此添加控件通知处理程序代码

	UINT i = 0,num = 0;
	BOOL check = FALSE;

	num = m_test_config.GetItemCount();
	check = ((CButton *)GetDlgItem(IDC_CHECK_ALL))->GetCheck();


	for (i =0; i < num; i++)
	{
		if (check)
		{
			m_test_config.SetCheck(i, TRUE);
			g_update_flag[i] = 1;
		} 
		else
		{
			m_test_config.SetCheck(i, FALSE);
			g_update_flag[i] = 0;
		}
	}
}

void CRightCtrlDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CRightCtrlDlg::OnBnClickedButtonUpdateAll()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL update_flag = FALSE;
	UINT i = 0, num = 0, j = 0;

	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();

	num = pP->m_RightDlg.m_test_config.GetItemCount();
	for (i = 0; i < num; i++)
	{
		if (g_update_flag[i] == 1)
		{
			break;
		}
	}
	if ( num < 1 || i == num)
	{
		AfxMessageBox(_T("没有任何设备需要升级的"), MB_OK);
		return;
	}


	for (i = 0; i < num; i++)
	{
		if (g_update_flag[i] == 1)
		{
			//判断软件版本是否一致
			for(j = 0; j < MAC_ADDRESS_LEN; j++)
			{
				if(g_test_config.m_last_config[i].Current_IP_version[j] != g_test_config.newest_version[j])
				{
					break;
				}
			}
			if(j != MAC_ADDRESS_LEN)
			{
				AfxMessageBox(_T("所选升级的设备中有版本号不一致的，请检查"), MB_OK);
				return;
			}
		}
	}

	GetDlgItem(IDC_BUTTON_UPDATE_ALL)->EnableWindow(FALSE);

	g_finish_flag = FALSE;
	one_update_finish = FALSE;

	pP->CloseServer(0);
	Sleep(1000);
	if (pP->Get_update_file())
	{
		//进入烧录线程
		pP->creat_update_all_thread();
	}

	
}

void CRightCtrlDlg::OnBnClickedCheckUpdateFlag()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL check = FALSE;

	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	pP->close_minitor();

	check = ((CButton *)GetDlgItem(IDC_CHECK_UPDATE_FLAG))->GetCheck();
	if (check)
	{
		g_test_config.update_find_flag = TRUE;
	}
	else
	{
		g_test_config.update_find_flag = FALSE;
	}

	
}

void CRightCtrlDlg::OnBnClickedRadioNet()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL check = FALSE;


	check = ((CButton *)GetDlgItem(IDC_RADIO_NET))->GetCheck();
	if (check)
	{
		g_test_config.net_mode = FALSE;
	}
	else
	{
		g_test_config.net_mode = TRUE;
	}
}

void CRightCtrlDlg::OnBnClickedRadioWifi()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL check = FALSE;
	check = ((CButton *)GetDlgItem(IDC_RADIO_WIFI))->GetCheck();
	if (check)
	{
		g_test_config.net_mode= TRUE;
	}
	else
	{
		g_test_config.net_mode = FALSE;
	}
}


void CRightCtrlDlg::OnTimer(UINT_PTR nIDEvent)
{
	//UpdateWindow();
}
void CRightCtrlDlg::OnStnClickedStaticTestTitle()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CRightCtrlDlg::OnStnClickedStatic1()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CRightCtrlDlg::OnStnClickedStatic2()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CRightCtrlDlg::OnStnClickedStatictextwifi()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CRightCtrlDlg::OnStnClickedStaticTestVideo()
{
	// TODO: 在此添加控件通知处理程序代码
}
