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
extern CFile g_file_fp;
extern UINT g_max_frame_idex;
extern BOOL g_stop_play_flag ;
extern BOOL g_start_play_flag;
extern CFile g_play_fp;
BOOL g_need_re_open_flag = TRUE;
extern LONGLONG g_current_Off;
extern BOOL g_play_first_frame_flag;
extern char g_update_flag[UPDATE_MAX_NUM];
extern BOOL g_start_recode_flag;



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
}


BEGIN_MESSAGE_MAP(CRightCtrlDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_CONNET, &CRightCtrlDlg::OnBnClickedButtonConnet)
	ON_BN_CLICKED(IDC_BUTTON_SET, &CRightCtrlDlg::OnBnClickedButtonSet)
	ON_BN_CLICKED(IDC_RADIO_VGA, &CRightCtrlDlg::OnBnClickedRadioVga)
	ON_BN_CLICKED(IDC_RADIO_720P, &CRightCtrlDlg::OnBnClickedRadio720p)
	ON_BN_CLICKED(IDC_RADIO_960P, &CRightCtrlDlg::OnBnClickedRadio960p)
	ON_BN_CLICKED(IDC_RADIO_CLINTE, &CRightCtrlDlg::OnBnClickedRadioClinte)
	ON_BN_CLICKED(IDC_RADIO_SERVER, &CRightCtrlDlg::OnBnClickedRadioServer)
	ON_BN_CLICKED(IDC_CHECK_SAVE_VIDEO, &CRightCtrlDlg::OnBnClickedCheckSaveVideo)
	ON_BN_CLICKED(IDC_BUTTON_VIDEO_PATH, &CRightCtrlDlg::OnBnClickedButtonVideoPath)
	ON_EN_CHANGE(IDC_EDIT_SAVE_VIDEO, &CRightCtrlDlg::OnEnChangeEditSaveVideo)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_VIDEO, &CRightCtrlDlg::OnBnClickedButtonPlayVideo)
	ON_EN_CHANGE(IDC_EDIT_FRAME, &CRightCtrlDlg::OnEnChangeEditFrame)
	ON_EN_CHANGE(IDC_EDIT_VIDEO_FRAME, &CRightCtrlDlg::OnEnChangeEditVideoFrame)
	ON_BN_CLICKED(IDC_CHECK_SET_FRAME_PLAY, &CRightCtrlDlg::OnBnClickedCheckSetFramePlay)
	ON_EN_CHANGE(IDC_EDIT_PLAY_FRAME, &CRightCtrlDlg::OnEnChangeEditPlayFrame)
	ON_BN_CLICKED(IDC_BUTTON1, &CRightCtrlDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_FIRST_FRAME, &CRightCtrlDlg::OnBnClickedButtonPlayFirstFrame)
	ON_CBN_SELCHANGE(IDC_COMBO_SAMPLES, &CRightCtrlDlg::OnCbnSelchangeComboSamples)
	ON_CBN_EDITCHANGE(IDC_COMBO_SAMPLES, &CRightCtrlDlg::OnCbnEditchangeComboSamples)
	ON_CBN_EDITUPDATE(IDC_COMBO_SAMPLES, &CRightCtrlDlg::OnCbnEditupdateComboSamples)
	ON_CBN_SELENDCANCEL(IDC_COMBO_SAMPLES, &CRightCtrlDlg::OnCbnSelendcancelComboSamples)
	ON_BN_CLICKED(IDC_BUTTON_RECODE, &CRightCtrlDlg::OnBnClickedButtonRecode)
END_MESSAGE_MAP()


// CRightCtrlDlg 消息处理程序


BOOL CRightCtrlDlg::OnInitDialog()
{

	CString str;
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	m_save_video_flag = TRUE;
	m_start_stop_flag = TRUE;
	SetDlgItemText(IDC_BUTTON_RECODE, _T("启动对讲"));

	m_Status.SubclassDlgItem(IDC_STATIC_TEST_CONTENT,this);

	if (g_test_config.server_flag)
	{
		((CButton *)GetDlgItem(IDC_RADIO_SERVER))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_CLINTE))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_IPADDRESS_IP))->ShowWindow(TRUE);
		((CButton *)GetDlgItem(IDC_STATIC_IP))->ShowWindow(TRUE);
		((CButton *)GetDlgItem(IDC_IPADDRESS_IP))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_STATIC_IP))->EnableWindow(TRUE);
		
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_SERVER))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_CLINTE))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_IPADDRESS_IP))->ShowWindow(FALSE);
		((CButton *)GetDlgItem(IDC_STATIC_IP))->ShowWindow(FALSE);
		((CButton *)GetDlgItem(IDC_IPADDRESS_IP))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_STATIC_IP))->EnableWindow(FALSE);
		
	}

	if (g_test_config.save_video_enable)
	{
		((CButton *)GetDlgItem(IDC_CHECK_SAVE_VIDEO))->SetCheck(1);
	} 
	else
	{
		((CButton *)GetDlgItem(IDC_CHECK_SAVE_VIDEO))->SetCheck(0);
	}


	((CButton *)GetDlgItem(IDC_CHECK_SET_FRAME_PLAY))->SetCheck(g_test_config.set_play_frame_flag);
	if (g_test_config.set_play_frame_flag)
	{
		GetDlgItem(IDC_EDIT_PLAY_FRAME)->EnableWindow(TRUE);
		str.Format(_T("%d"), g_test_config.play_frame_num);
		SetDlgItemText(IDC_EDIT_PLAY_FRAME, str);
	}
	else
	{
		GetDlgItem(IDC_EDIT_PLAY_FRAME)->EnableWindow(FALSE);
	}

	m_play_video_flag = TRUE;

	SetDlgItemText(IDC_EDIT_SAVE_VIDEO, g_test_config.m_video_path);

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

void CRightCtrlDlg::OnBnClickedButtonConnet()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	((CButton *)GetDlgItem(IDC_BUTTON_CONNET))->EnableWindow(FALSE);
	//连接网络
	
	if (m_start_video_flag)
	{

		pP->close_play_video_thread();
		pP->close_play_first_frame_thread();

		if (g_test_config.save_video_enable)
		{
			//创建保存文件的线程
			if (!pP->start_save_video())
			{
				return;
			}
		}

		if (!pP->connet_video_net())
		{
			((CButton *)GetDlgItem(IDC_BUTTON_CONNET))->EnableWindow(TRUE);
			return;
		}
		m_start_video_flag = FALSE;
		
		SetDlgItemText(IDC_BUTTON_CONNET, _T("停止视频"));
	}
	else
	{
		if (g_test_config.save_video_enable)
		{
			pP->stop_save_video();
		}
		pP->Close_video_net(0);
		m_start_video_flag = TRUE;
		SetDlgItemText(IDC_BUTTON_CONNET, _T("启动视频"));
	}
	

	((CButton *)GetDlgItem(IDC_BUTTON_CONNET))->EnableWindow(TRUE);
}


void CRightCtrlDlg::OnBnClickedButtonSet()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();

	//连接网络
	((CButton *)GetDlgItem(IDC_BUTTON_SET))->EnableWindow(FALSE);
	pP->set_video_param();
	((CButton *)GetDlgItem(IDC_BUTTON_SET))->EnableWindow(TRUE);
}

void CRightCtrlDlg::OnBnClickedRadioVga()
{
	// TODO: 在此添加控件通知处理程序代码

	((CButton *)GetDlgItem(IDC_RADIO_VGA))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_720P))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_960P))->SetCheck(FALSE);
	g_test_config.video_parm.video_size = TYPE_VIDEO_VGA;
}

void CRightCtrlDlg::OnBnClickedRadio720p()
{
	// TODO: 在此添加控件通知处理程序代码
	((CButton *)GetDlgItem(IDC_RADIO_VGA))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_720P))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_960P))->SetCheck(FALSE);
	g_test_config.video_parm.video_size = TYPE_VIDEO_720P;
}

void CRightCtrlDlg::OnBnClickedRadio960p()
{
	// TODO: 在此添加控件通知处理程序代码
	((CButton *)GetDlgItem(IDC_RADIO_VGA))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_720P))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_960P))->SetCheck(TRUE);
	g_test_config.video_parm.video_size = TYPE_VIDEO_960P;
}

void CRightCtrlDlg::OnBnClickedRadioClinte()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	BOOL check = ((CButton *)GetDlgItem(IDC_RADIO_CLINTE))->GetCheck();
	if (check)
	{
		((CButton *)GetDlgItem(IDC_RADIO_SERVER))->SetCheck(0);
		((CButton *)GetDlgItem(IDC_RADIO_CLINTE))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_IPADDRESS_IP))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_STATIC_IP))->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_IPADDRESS_IP))->ShowWindow(TRUE);
		((CButton *)GetDlgItem(IDC_STATIC_IP))->ShowWindow(TRUE);
		g_test_config.server_flag = 1;	
		pP->close_server_video_thread();

	}

}

void CRightCtrlDlg::OnBnClickedRadioServer()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	BOOL check = ((CButton *)GetDlgItem(IDC_RADIO_SERVER))->GetCheck();
	if (check)
	{
		((CButton *)GetDlgItem(IDC_RADIO_SERVER))->SetCheck(1);
		((CButton *)GetDlgItem(IDC_RADIO_CLINTE))->SetCheck(0);

		((CButton *)GetDlgItem(IDC_IPADDRESS_IP))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_STATIC_IP))->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_IPADDRESS_IP))->ShowWindow(FALSE);
		((CButton *)GetDlgItem(IDC_STATIC_IP))->ShowWindow(FALSE);
		g_test_config.server_flag = 0;

		//创建服务端的socket
		pP->create_server_video_thread();
		
	}
}

void CRightCtrlDlg::OnBnClickedCheckSaveVideo()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	
	

	BOOL check = ((CButton *)GetDlgItem(IDC_CHECK_SAVE_VIDEO))->GetCheck();
	if (check)
	{
		GetDlgItemText(IDC_EDIT_SAVE_VIDEO, str);
		if (str.IsEmpty())
		{
			((CButton *)GetDlgItem(IDC_CHECK_SAVE_VIDEO))->SetCheck(0);
			g_test_config.save_video_enable = 0;
			AfxMessageBox(_T("请先选择一个保存视频的路径"), MB_OK);
			return;
		}
		memset(g_test_config.m_video_path, 0, MAX_PATH + 1);
		_tcscpy(g_test_config.m_video_path, str);

		g_test_config.save_video_enable = 1;
	}
	else
	{
		g_test_config.save_video_enable = 0;
	}

}

void CRightCtrlDlg::OnBnClickedButtonVideoPath()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strPath;
	//开始保存视频
	//获取文件保存的路径
	if (!m_play_video_flag)
	{
		AfxMessageBox(_T("视频正播放中，请暂停"), MB_OK);
		return;
	}
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	TCHAR szFilter[] =	TEXT("BIN Files(*.dat)|*.dat|") \
		TEXT("All Files (*.*)|*.*||") ;

	CFileDialog fd(FALSE, _T(".dat"), NULL, OFN_HIDEREADONLY//|OFN_OVERWRITEPROMPT
		, szFilter, NULL);


	if(IDOK == fd.DoModal())
	{
		//获取 SPI 镜像 保存的路径(含文件名)
		strPath = fd.GetPathName();
		memset(g_test_config.m_video_path, 0, (MAX_PATH + 1)*sizeof(TCHAR));
		_tcscpy(g_test_config.m_video_path, strPath);
		SetDlgItemText(IDC_EDIT_SAVE_VIDEO, strPath);
		if (m_play_video_flag)
		{
			g_need_re_open_flag = TRUE;
			pP->close_play_video_thread();
			SetDlgItemText(IDC_BUTTON_PLAY_VIDEO, _T("回放视频"));
		}
		
	}
}

void CRightCtrlDlg::OnEnChangeEditSaveVideo()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString str;
	TCHAR path[MAX_PATH+1] = {0};

	GetDlgItemText(IDC_EDIT_SAVE_VIDEO, str);
	if (str.IsEmpty())
	{
		return;
	}
	memset(path, 0, (MAX_PATH + 1)*sizeof(TCHAR));
	_tcscpy(path, str);


	if (_tcscmp(g_test_config.m_video_path, path) != 0)
	{
		g_need_re_open_flag = TRUE;
		memset(g_test_config.m_video_path, 0, (MAX_PATH + 1)*sizeof(TCHAR));
		_tcscpy(g_test_config.m_video_path, str);
	}
	
}

void CRightCtrlDlg::OnBnClickedButtonPlayVideo()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	
	g_play_first_frame_flag = FALSE;
	pP->close_play_first_frame_thread();	
	Sleep(200);
	if (m_play_video_flag)
	{
		

		//
		if (g_need_re_open_flag)
		{
			if(!pP->play_video_file())
			{
				return;
			}
			g_need_re_open_flag = FALSE;
		}
		m_play_video_flag = FALSE;
		g_start_play_flag =TRUE;
		g_stop_play_flag = FALSE;;
		SetDlgItemText(IDC_BUTTON_PLAY_VIDEO, _T("暂停回放"));
		GetDlgItem(IDC_CHECK_SET_FRAME_PLAY)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_PLAY_FRAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_PLAY_FIRST_FRAME)->EnableWindow(FALSE);
		
		
	}
	else
	{
		//pP->close_play_video_thread();
		m_play_video_flag = TRUE;
		g_stop_play_flag = TRUE;
		g_start_play_flag = FALSE;
		SetDlgItemText(IDC_BUTTON_PLAY_VIDEO, _T("回放视频"));
		GetDlgItem(IDC_CHECK_SET_FRAME_PLAY)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_PLAY_FRAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_PLAY_FIRST_FRAME)->EnableWindow(TRUE);
	}
	
}

void CRightCtrlDlg::OnEnChangeEditFrame()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString str;
	UINT frame = 0;
	USES_CONVERSION;
	GetDlgItemText(IDC_EDIT_FRAME, str);
	if (str.IsEmpty())
	{
		return;
	}
	
	frame = atoi(T2A(str));
	if (frame < 1 || frame > 30)
	{
		AfxMessageBox(_T("帧率设置不在有效范围内"), MB_OK);
		return;
	}
	g_test_config.video_parm.frame = frame;

}

void CRightCtrlDlg::OnEnChangeEditVideoFrame()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString str;
	UINT data_rate = 0;
	USES_CONVERSION;
	GetDlgItemText(IDC_EDIT_VIDEO_FRAME, str);
	if (str.IsEmpty())
	{
		return;
	}

	data_rate = atoi(T2A(str));
	if (data_rate < 1 || data_rate > 65536)
	{
		AfxMessageBox(_T("码率设置不在有效范围内"), MB_OK);
		return;
	}
	g_test_config.video_parm.data_rate = data_rate;
}

void CRightCtrlDlg::OnBnClickedCheckSetFramePlay()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;

	BOOL check = ((CButton *)GetDlgItem(IDC_CHECK_SET_FRAME_PLAY))->GetCheck();
	if (check)
	{
		g_test_config.set_play_frame_flag = TRUE;
		GetDlgItem(IDC_EDIT_PLAY_FRAME)->EnableWindow(TRUE);
		str.Format(_T("%d"), g_test_config.play_frame_num);
		SetDlgItemText(IDC_EDIT_PLAY_FRAME, str);
	}
	else
	{
		g_test_config.set_play_frame_flag = FALSE;
		GetDlgItem(IDC_EDIT_PLAY_FRAME)->EnableWindow(FALSE);
	}
	
}

void CRightCtrlDlg::OnEnChangeEditPlayFrame()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString str;
	UINT data_rate = 0;
	USES_CONVERSION;
	GetDlgItemText(IDC_EDIT_PLAY_FRAME, str);
	if (str.IsEmpty())
	{
		return;
	}

	data_rate = atoi(T2A(str));
	if (data_rate < 1 || data_rate > 30)
	{
		AfxMessageBox(_T("码率设置不在有效范围内(1~30)"), MB_OK);
		SetDlgItemText(IDC_EDIT_PLAY_FRAME, _T(""));
		return;
	}
	g_test_config.play_frame_num = data_rate;
}

void CRightCtrlDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
}

void CRightCtrlDlg::OnBnClickedButtonPlayFirstFrame()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	
	g_play_first_frame_flag = TRUE;
	pP->close_play_video_thread();
	pP->close_play_first_frame_thread();
	pP->close_play_file();
	Sleep(100);

	if(!pP->play_video_file())
	{
		return;
	}
	g_need_re_open_flag = TRUE;
	g_stop_play_flag = FALSE;

}

void CRightCtrlDlg::OnCbnSelchangeComboSamples()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	UINT data_rate = 0;
	USES_CONVERSION;
	GetDlgItemText(IDC_COMBO_SAMPLES, str);
	if (str.IsEmpty())
	{
		return;
	}

	data_rate = atoi(T2A(str));
	if (data_rate != 8000 && data_rate != 16000)
	{
		AfxMessageBox(_T("采样率只支持8000或16000Hz"), MB_OK);
		SetDlgItemText(IDC_COMBO_SAMPLES, _T("8000"));
		return;
	}
	g_test_config.SamplesPerSec = data_rate;

}

void CRightCtrlDlg::OnCbnEditchangeComboSamples()
{
	// TODO: 在此添加控件通知处理程序代码
	
}

void CRightCtrlDlg::OnCbnEditupdateComboSamples()
{
	// TODO: 在此添加控件通知处理程序代码

}

void CRightCtrlDlg::OnCbnSelendcancelComboSamples()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	UINT data_rate = 0;
	USES_CONVERSION;
	GetDlgItemText(IDC_COMBO_SAMPLES, str);
	if (str.IsEmpty())
	{
		return;
	}

	data_rate = atoi(T2A(str));
	if (data_rate != 8000 && data_rate != 16000)
	{
		AfxMessageBox(_T("采样率只支持8000或16000Hz"), MB_OK);
		SetDlgItemText(IDC_COMBO_SAMPLES, _T("8000"));
		return;
	}
	g_test_config.SamplesPerSec = data_rate;
}

void CRightCtrlDlg::OnBnClickedButtonRecode()
{
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	((CButton *)GetDlgItem(IDC_BUTTON_RECODE))->EnableWindow(FALSE);
	//连接网络

	if (m_start_stop_flag)
	{

		m_start_stop_flag = FALSE;
		g_start_recode_flag = TRUE;

		SetDlgItemText(IDC_BUTTON_RECODE, _T("停止对讲"));
	}
	else
	{
		m_start_stop_flag = TRUE;
		g_start_recode_flag = FALSE;
		SetDlgItemText(IDC_BUTTON_RECODE, _T("启动对讲"));
	}

	((CButton *)GetDlgItem(IDC_BUTTON_RECODE))->EnableWindow(TRUE);
}
