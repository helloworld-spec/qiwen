// ISPCTRL_TOOLDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "ISPCTRL_TOOLDlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define NEW_PLATFORM


#define STR_MODULE		"Module"
#define STR_SYSTEM		"System"
#define STR_BB			"BLC"
#define STR_DPC			"DPC"
#define STR_GB			"GB"
#define STR_LSC			"LSC"
#define STR_RAWLUT		"RAW Gamma"
#define STR_DEMOSAIC	"Demosaic"
#define STR_CCM			"CCM"
#define STR_GAMMA		"RGB Gamma"
#define STR_DENOISE		"Denoise"
#define STR_SHARP		"Sharp"
#define STR_EDGE		"Edge"
#define STR_3DNR		"3DNR"
#define STR_WDR			"WDR"
#define STR_FCS			"FCS"
#define STR_YUVEFFECT	"YUV Effect"
#define STR_RGB2YUV		"RGB2YUV"
#define STR_SATURATION	"Saturation"
#define STR_CONTRAST	"Contrast"
#define STR_WB			"WB"
#define STR_EXP			"EXP"
#define STR_AF			"AF"
#define STR_STAT		"Statistics"
#define STR_zone_weight	 "Zone Weight"
#define STR_MISC		"MISC"
#define STR_COMPARE		"Compare"
#define STR_HUE		    "HUE"
#define STR_Y_GAMMA	    "Y Gamma"


#define HEARTBEAT_SEND_TIMER_ID		1
#define HEARTBEAT_CHECK_TIMER_ID	2
#define MSGBOX_SHOW_TIMER_ID		3

#define HEARTBEAT_SEND_TIME	3000
#define HEARTBEAT_TIMEOUT	15000
#define MSGBOX_SHOW_TIME	2000



#define ISP_PARM_CNT_SIZE		4

#define SENSOR_OV9712_PARAM_NUM		69
#define SENSOR_H42_PARAM_NUM		63

extern AK_ISP_INIT_PARAM   Isp_init_param;
HANDLE g_hBurnThread_heart = INVALID_HANDLE_VALUE;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA
	CDC m_MemDC;
	CBitmap m_MemBitmap;
	CBitmap* m_pOldMemBitmap;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnButtonOk();
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	ON_BN_CLICKED(IDC_BUTTON_OK, OnButtonOk)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CRect rect;

	GetClientRect(rect);

	CDC* pDC = GetDC();

	m_MemDC.CreateCompatibleDC(pDC);
	m_MemBitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	m_pOldMemBitmap = m_MemDC.SelectObject(&m_MemBitmap);
	
	ReleaseDC(pDC);
	
	return TRUE;
}

HBRUSH CAboutDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC)
	{
		pDC->SetBkColor(RGB(240, 240, 240));
	}
	
	return hbr;
}

void CAboutDlg::OnPaint()
{
	CRect rect;

	GetClientRect(rect);

	m_MemDC.FillSolidRect(rect, RGB(240, 240, 240));
	
	CPaintDC dc(this);

	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &m_MemDC, 0, 0, SRCCOPY);
}


void CAboutDlg::OnButtonOk() 
{
	// TODO: Add your control notification handler code here
	if (m_MemDC.m_hDC) {
		m_MemDC.SelectObject(m_pOldMemBitmap);
		m_MemDC.DeleteDC();
		m_MemBitmap.DeleteObject();
	}

	CDialog::OnOK();
}


void CAboutDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_MemDC.m_hDC) {
		m_MemDC.SelectObject(m_pOldMemBitmap);
		m_MemDC.DeleteDC();
		m_MemBitmap.DeleteObject();
	}

	CDialog::OnClose();
}


/////////////////////////////////////////////////////////////////////////////
// CISPCTRL_TOOLDlg dialog

CISPCTRL_TOOLDlg::CISPCTRL_TOOLDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CISPCTRL_TOOLDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CISPCTRL_TOOLDlg)
	m_Port = 8000;
	m_ConnectState = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_sensor_num = 0;
	memset(&m_Isp_sensor, 0, sizeof(AK_ISP_INIT_SENSOR));
	Isp_init_param.p_Isp_sensor.p_sensor = NULL;

	m_pInetSession = NULL;
	m_pFtpConnection = NULL; 
	m_bOnlySensor = FALSE;
	m_SubFileId = 0;
	m_SubFileNum = 0;
	m_sensor_id = 0xffffffff;
	m_Password = _T("");
	memset(m_SubFile, 0, SUBFILE_NUM_MAX * sizeof(CFGFILE_STRUCT));
	memset(m_version, 0, 6);
}

void CISPCTRL_TOOLDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CISPCTRL_TOOLDlg)
	DDX_Control(pDX, IDC_COMBO_SUBFILEID, m_SubFileIdCtrl);
	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_btnConnect);
	DDX_Control(pDX, IDC_IPADDRESS1, m_Ip);
	DDX_Control(pDX, IDC_TREE1, m_trModules);
	DDX_Text(pDX, IDC_EDIT1, m_Port);
	DDX_Text(pDX, IDC_CONNECTSTATE, m_ConnectState);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CISPCTRL_TOOLDlg, CDialog)
	//{{AFX_MSG_MAP(CISPCTRL_TOOLDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree1)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, OnButtonConnect)
	ON_MESSAGE(WM_ENABLE_CHANGE, OnPageEnableChangeMsg)
	ON_MESSAGE(WM_SUBMISSION, OnPageSubmissionMsg)
	ON_MESSAGE(WM_GET_ISP_INFO, OnPageGetInfoMsg)
	ON_MESSAGE(WM_GET_ISP_INFO_FOR_TEXT, OnPageGetInfoMsg_ForText)
	ON_MESSAGE(WM_SUBMISSION_FOR_TEXT, OnPageSubmissionMsg_ForText)
	ON_MESSAGE(WM_CLEAR_SENSOR_PARAM, OnClearSensorParam)
	ON_MESSAGE(WM_READ_FOR_TEXT, OnPageReadMsg_ForText)
	ON_MESSAGE(WM_SAVE_FOR_TEXT, OnPageSaveMsg_ForText)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_BUTTON_CONFIRM, OnButtonConfirm)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_GETALL, OnButtonGetall)
	ON_MESSAGE(WM_COPY_UI_TO_TEXT, OnPageCopyUiToText)
	ON_MESSAGE(WM_COPY_TEXT_TO_UI, OnPageCopyTextToUi)
	ON_MESSAGE(WM_GET_YUV_IMG, OnGetYuvImg)
	ON_BN_CLICKED(IDC_BUTTON_ADDFILE, OnButtonAddfile)
	ON_CBN_SELCHANGE(IDC_COMBO_SUBFILEID, OnSelchangeComboSubfileid)
	ON_BN_CLICKED(IDC_BUTTON_READTMP, OnButtonReadtmp)
	ON_BN_CLICKED(IDC_BUTTON_SAVETMP, OnButtonSavetmp)
	ON_BN_CLICKED(IDC_BUTTON_EPTSUBFILE, OnButtonEptsubfile)
	ON_MESSAGE(WM_GET_WB_INFO, OnGet_wb_info)
	ON_MESSAGE(WM_SET_WB_INFO, OnSet_wb_info)
	ON_BN_CLICKED(IDC_BUTTON_READ_V3, OnButtonReadV3)
	ON_MESSAGE(WM_GET_EXP_INFO, OnGet_exp_info)
	ON_MESSAGE(WM_GET_LSC_INFO, OnGetLscInfo)
	ON_MESSAGE(WM_SET_LSC_INFO, OnSetLscInfo)
	ON_MESSAGE(WM_GET_RGB_GAMMA, OnGetRgbGamma)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CISPCTRL_TOOLDlg message handlers

void CISPCTRL_TOOLDlg::SaveImgData(T_U8 type, char* buf, T_U32 size)
{
	TCHAR path[MAX_PATH] = {0};
	char  path_temp[MAX_PATH] = {0};
	TCHAR *dir_name = "img";
	UINT len = 0;

	if (NULL == buf || 0 == size)
		return;

	time_t t = time(0);
	struct tm * ptNow = NULL;

	char strDestName[MAX_PATH] = {0};
	
	ptNow = localtime(&t);

	_tcscpy(path, ConvertAbsolutePath(dir_name));
	len = _tcslen(path);
	memcpy(path_temp, path, len);

	CreateDirectory(path_temp, NULL);
	
	len = strlen(path_temp);
	memcpy(strDestName, path_temp, len);
	if (ISP_YUV_IMG == type)
	{
		sprintf(&strDestName[len], "/YUV_%04d_%02d_%02d,%02d,%02d,%02d.yuv", 
			ptNow->tm_year + 1900, ptNow->tm_mon + 1, ptNow->tm_mday, ptNow->tm_hour, ptNow->tm_min, ptNow->tm_sec);

	}
	else if (ISP_RAW_IMG == type)
	{
		sprintf(&strDestName[len], "/RAW_%04d_%02d_%02d,%02d,%02d,%02d.dat", 
			ptNow->tm_year + 1900, ptNow->tm_mon + 1, ptNow->tm_mday, ptNow->tm_hour, ptNow->tm_min, ptNow->tm_sec);
	}
	else
	{
		fprintf(stderr, "SaveImgData type error\n");
		return;
	}

#ifndef NEW_PLATFORM
	CString addr;
	BYTE f0, f1, f2, f3;
		
	m_Ip.GetAddress(f0, f1, f2, f3);
	addr.Format(_T("%d.%d.%d.%d"), f0, f1, f2, f3);

	if (!Connet_FTPServer(addr))
	{
		SetMstboxTimer();
		fprintf(stderr, "Connect FTP Failed!\n");
		AfxMessageBox(_T("连接FTP服务器失败"));
		return;
	}

	char strSourceName[64] = {0};
	strncpy(strSourceName, buf, size);
	if (('\n' == strSourceName[size-1]) || ('\r' ==  strSourceName[size-1]))
	{
		strSourceName[size-1] = 0;
	}

	if (!m_pFtpConnection->GetFile(strSourceName, strDestName, AK_FALSE, FILE_ATTRIBUTE_NORMAL, FTP_TRANSFER_TYPE_BINARY | INTERNET_FLAG_RELOAD, 1))   
	{ 
		int err = GetLastError();

		SetMstboxTimer();
		fprintf(stderr, "Get File Failed, err:%d!\n", err);
		AfxMessageBox(_T("Get File Failed!"));
		
		return;
	}

	//m_pFtpConnection->Remove(strSourceName);
#else
	CFile file;
	CFileException e;
	
	if (!file.Open(strDestName, CFile::modeCreate|CFile::modeWrite, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	file.Write(buf, size);

	file.Close();
#endif

	m_StatDlg.m_ImgDlg.SetImgPath(strDestName);
	m_CCMDlg.m_ImgDlg.SetImgPath(strDestName);
	m_LscDlg.m_BrightnessCalcDlg.SetImgPath(strDestName);
	m_HueDlg.m_HUE_calc.SetImgPath(strDestName);
}

void OnRecvRespond(short addr_id, short cmd_type, char* pData, int len, void *pParam)
{	
	CISPCTRL_TOOLDlg * pthis = (CISPCTRL_TOOLDlg *)pParam;
	AK_ISP_INIT_WDR m_isp_init_wdr_temp;
	int ret = -1;

	switch (addr_id)
	{
	case ISP_RAW_LUT:
		if (CMD_REPLY == cmd_type)
		{
			if (pthis->m_RawlutDlg.IsSetApart())
			{
				pthis->m_RawlutDlg.m_RgbDlg.SetPageInfoSt(pData, len);
			}
			else
			{
				pthis->m_RawlutDlg.SetPageInfoSt(pData, len);
			}
			
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置raw lut模块数据失败。");
			}
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_RAW_LUT(pData, len))
			{
				fprintf(stderr, "decode_packet_RAW_LUT error\n");
			}
		}
		else
		{
			fprintf(stderr, "OnRecvRespond cmd_type error %d\n", cmd_type);
		}
		break;

	case ISP_GAMMA:
		if (CMD_REPLY == cmd_type)
		{
			if (pthis->m_GammaDlg.IsSetApart())
			{
				pthis->m_GammaDlg.m_RgbDlg.SetPageInfoSt(pData, len);
			}
			else
			{
				pthis->m_GammaDlg.SetPageInfoSt(pData, len);
			}
			
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置rgb gamma 模块数据失败。");
			}
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_GAMMA(pData, len))
			{
				fprintf(stderr, "decode_packet_GAMMA error\n");
			}
		}
		else
		{
			fprintf(stderr, "OnRecvRespond cmd_type error %d\n", cmd_type);
		}
		break;

	case ISP_Y_GAMMA:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_Y_GammaDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置y gamma 模块数据失败。");
			}
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_Y_GAMMA(pData, len))
			{
				fprintf(stderr, "decode_packet_Y_GAMMA error\n");
			}
		}
		else
		{
			fprintf(stderr, "OnRecvRespond cmd_type error %d\n", cmd_type);
		}
		break;
		
	case ISP_WDR:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_WdrDlg.SetPageInfoSt(pData, len);
			memcpy(&m_isp_init_wdr_temp, pData, len);

		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置wdr模块数据失败。");
			}
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_WDR(pData, len))
			{
				fprintf(stderr, "decode_packet_WDR error\n");
			}
		}
		else
		{
			fprintf(stderr, "OnRecvRespond cmd_type error %d\n", cmd_type);
		}
		break;

	case ISP_DEMO:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_DemosaicDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置demo模块数据失败。");
			}
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_DEMO(pData, len))
			{
				fprintf(stderr, "decode_packet_DEMO error\n");
			}
		}
		else
		{
			fprintf(stderr, "OnRecvRespond cmd_type error %d\n", cmd_type);
		}
		break;

	case ISP_YUVEFFECT:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_YuvEffectDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置yuv effect模块数据失败。");
			}
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_YUVEFFECT(pData, len))
			{
				fprintf(stderr, "decode_packet_YUVEFFECT error\n");
			}
		}
		else
		{
			fprintf(stderr, "OnRecvRespond cmd_type error %d\n", cmd_type);
		}
		break;

	case ISP_BB:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_BBDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_BB(pData, len))
			{
				fprintf(stderr, "decode_packet_BB error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置blc模块数据失败。");
			}
		}
		else
		{
			fprintf(stderr, "OnRecvRespond cmd_type error %d\n", cmd_type);
		}
		break;

	case ISP_LSC:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_LscDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_LSC(pData, len))
			{
				fprintf(stderr, "decode_packet_LSC error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置lsc模块数据失败。");
			}
		}
		break;
	case ISP_CCM:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_CCMDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_CCM(pData, len))
			{
				fprintf(stderr, "decode_packet_CCM error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置ccm模块数据失败。");
			}
		}
		break;
	case ISP_WB:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_WbDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_WB(pData, len))
			{
				fprintf(stderr, "decode_packet_WB error\n");
				AfxMessageBox("decode_packet_WB error ");
			}
			else
			{
				memcpy(&pthis->m_StatDlg.m_ImgDlg.m_CoordinateDlg.m_coor_Isp_wb, &Isp_init_param.p_Isp_wb, sizeof(AK_ISP_INIT_WB));
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置wb模块数据失败。");
			}
		}
		break;
	case ISP_NR:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_DenoiseDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_NR(pData, len))
			{
				fprintf(stderr, "decode_packet_NR error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置nr模块数据失败。");
			}
		}
		break;
	case ISP_3DNR:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_3DNRDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_3DNR(pData, len))
			{
				fprintf(stderr, "decode_packet_3DNR error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置3dnr模块数据失败。");
			}
		}
		break;
	case ISP_GB:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_GBDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_GB(pData, len))
			{
				fprintf(stderr, "decode_packet_GB error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置gb模块数据失败。");
			}
		}
		break;
	case ISP_FCS:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_FcsDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_FCS(pData, len))
			{
				fprintf(stderr, "decode_packet_FCS error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置fcs模块数据失败。");
			}
		}
		break;
	/*case ISP_EDGE:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_EdgeDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_EDGE(pData, len))
			{
				fprintf(stderr, "decode_packet_EDGE error\n");
			}
		}
		break;*/
	case ISP_SHARP:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_SharpDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_SHARP(pData, len))
			{
				fprintf(stderr, "decode_packet_SHARP error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置sharp模块数据失败。");
			}
		}
		break;
	case ISP_SATURATION:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_SaturationDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_SATURATION(pData, len))
			{
				fprintf(stderr, "decode_packet_SATURATION error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置saturation模块数据失败。");
			}
		}
		break;
	case ISP_CONTRAST:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_ContrastDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_CONSTRAST(pData, len))
			{
				fprintf(stderr, "decode_packet_CONSTRAST error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置contrast模块数据失败。");
			}
		}
		break;

	case ISP_DPC:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_DpcDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_DPC(pData, len))
			{
				fprintf(stderr, "decode_packet_DPC error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置dpc模块数据失败。");
			}
		}
		break;
	case ISP_ZONE_WEIGHT:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_Zone_WeightDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_WEIGHT(pData, len))
			{
				fprintf(stderr, "decode_packet_WEIGHT error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置zone weight模块数据失败。");
			}
		}
		break;
	case ISP_AF:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_AFDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_AF(pData, len))
			{
				fprintf(stderr, "decode_packet_AF error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置AF模块数据失败。");
			}
		}
		break;
	case ISP_HUE:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_HueDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_hue(pData, len))
			{
				fprintf(stderr, "decode_packet_AF error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置hue模块数据失败。");
			}
		}
		break;
	case ISP_EXP:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_ExpDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_EXP(pData, len))
			{
				fprintf(stderr, "decode_packet_EXP error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置exp模块数据失败。");
			}
		}
		break;
	case ISP_MISC:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_MiscDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_MISC(pData, len))
			{
				fprintf(stderr, "decode_packet_MISC error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置misc模块数据失败。");
			}
		}
		break;

	case ISP_RGB2YUV:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_Rgb2YuvDlg.SetPageInfoSt(pData, len);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_RGBTOYUV(pData, len))
			{
				fprintf(stderr, "decode_packet_RGBTOYUV error\n");
			}
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			if (0 != ret)
			{
				AfxMessageBox("设置rgb2yuv模块数据失败。");
			}
		}
		break;
	case ISP_REGISTER:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_SysDlg.SetPageInfoSt(pData, len);

			T_U16 addr = 0;
			T_U16 value = 0;

			memcpy(&addr, pData+ISP_PARM_CNT_SIZE+1, 2);
			memcpy(&value, pData+ISP_PARM_CNT_SIZE+1+2+1, 2);
		}
		else if (CMD_REPLY_TXT == cmd_type)
		{
			//显示数据
			if (!pthis->m_TextDlg.decode_packet_sensor(pData, len))
			{
				fprintf(stderr, "decode_packet_sensor error\n");
			}			
		}
		break;

	case ISP_3DSTAT:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_StatDlg.m_3DStatDlg.SetPageInfoSt(pData, len);
		}

		break;

	case ISP_AFSTAT:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_StatDlg.m_AFStatDlg.SetPageInfoSt(pData, len);
		}

		break;

	case ISP_AESTAT:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_StatDlg.m_AEStatDlg.SetPageInfoSt(pData, len);
		}

		break;

	case ISP_AWBSTAT:
		if (CMD_REPLY == cmd_type)
		{
			pthis->m_StatDlg.m_AwbStatDlg.SetPageInfoSt(pData, len);
		}

		break;

	case ISP_CFG_DATA:
		if (CMD_REPLY == cmd_type)
		{
			pthis->DecodeTotalFile(pData, len);
		}
		else if (CMD_RET == cmd_type)
		{
			memcpy(&ret, pData, 4);

			pthis->SetMstboxTimer();

			if (0 == ret)
			{
				AfxMessageBox("配置文件已保存到设备端。");
			}
			else
			{
				AfxMessageBox("保存配置文件到设备失败。");
			}
		}

		break;

	case ISP_HEARTBEAT:
		if (CMD_REPLY == cmd_type)
		{
			pthis->KillTimer(pthis->m_check_timer);
			pthis->m_check_timer = pthis->SetTimer(HEARTBEAT_CHECK_TIMER_ID, HEARTBEAT_TIMEOUT, NULL);
		}

		break;


	case ISP_YUV_IMG:
		if (CMD_REPLY == cmd_type)
		{
			pthis->SaveImgData(ISP_YUV_IMG, pData, len);
			pthis->OnFTP_close();
		}

		break;

	default:
		fprintf(stderr, "OnRecvRespond addr_id error %d\n", addr_id);
		break;
	}
}

void CISPCTRL_TOOLDlg::SetWindowTitle()
{
	CString str;
#ifndef NEW_PLATFORM
	str.Format("Anyka ISP Tool (PDK V1) V%d.%d.%02d",MAIN_VERSION, SUB_VERSION0, SUB_VERSION1);
#else
	str.Format("Anyka ISP Tool V%d.%d.%02d",MAIN_VERSION, SUB_VERSION0, SUB_VERSION1);
#endif
	SetWindowText(str);
}

void GetIpAddr(BYTE* a, BYTE* b, BYTE* c, BYTE* d)
{
	FILE *file;
	BYTE num[4] = {0};
	UINT filelen;
	char str[24] = {0};
	char strnum[4][4] = {0};

	if (NULL == a || NULL == b ||NULL == c ||NULL == d)
	{
		return;
	}

	file = fopen("ip.txt", "r+");

	if (NULL != file)
	{
		int i=0, j=0, offset=0;

		fseek(file, 0, SEEK_END);
		filelen = ftell(file);
		fseek(file, 0, SEEK_SET);
		fread(str, 1, filelen, file);
		
		for (i=0; i<filelen+1; i++)
		{
			if ('.' == str[i] || 0 == str[i] || '\r' == str[i] || '\n' == str[i])
			{
				memcpy(strnum[j], str+offset, i-offset);
				j++;
				offset = i+1;
			}
		}

		for (i=0; i<4; i++)
		{
			num[i] = atoi((const char*)strnum[i]);
		}

		fclose(file);

		*a = num[0];
		*b = num[1];
		*c = num[2];
		*d = num[3];
	}
}

void SaveIpAddr(BYTE a, BYTE b, BYTE c, BYTE d)
{
	FILE *file;
	char str[24];

	file = fopen("ip.txt", "w+");

	if (NULL != file)
	{
		sprintf(str, "%u.%u.%u.%u\r\n", a, b, c, d);
	
		fwrite(str, 1, strlen(str), file);
		fclose(file);
	}
}

BOOL CISPCTRL_TOOLDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_WbDlg.m_AWB_wb_statDlg.open_dlg_flag = FALSE;
	m_gamma_copyUItoTest_flag = FALSE;
		


	m_Mytip.Create(this);  
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_READ), "导入文档的数据到工具上" ); //IDC_BUTTON为你要添加提示信息的按钮的ID
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_WRITE), "导出工具的数据到文档上" ); //IDC_BUTTON为你要添加提示信息的按钮的ID
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_SET),    "刷新数据到设备,让PC和设备进行同步" ); //IDC_BUTTON为你要添加提示信息的按钮的ID
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_CONFIRM), "把当前调试好的数据保存到设备端的配置文件" ); //IDC_BUTTON为你要添加提示信息的按钮的ID
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_CONNECT), "连接或断开设备" ); //IDC_BUTTON为你要添加提示信息的按钮的ID
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_GET), "获取设备端的配置文件数据" ); 
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_GETALL), "获取设备端硬件所有模块的数据" ); 
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_READTMP), "导入暂存文档的数据到工具上" ); 
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_SAVETMP), "导出工具的数据到暂存文档上" ); 
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_ADDFILE), "导入文档数据到SubFileId子文件位置" ); 
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_EPTSUBFILE), "导出SubFileId子文件的数据到文档上" ); 
	m_Mytip.AddTool( GetDlgItem(IDC_BUTTON_READ_V3), "导入V3版的配置文件数据到工具上" ); 
	m_Mytip.SetDelayTime(200); //设置延迟
	//m_Mytip.SetTipTextColor( RGB(0,0,255) ); //设置提示文本的颜色
	//m_Mytip.SetTipBkColor( RGB(255,255,255)); //设置提示框的背景颜色
	m_Mytip.Activate(TRUE); //设置是否启用提示
	
	// TODO: Add extra initialization here

	//EnableToolTips(TRUE);  
	// 创建tooltip控件  
    m_toolTip.Create(this, TTS_ALWAYSTIP);  
	m_toolTip.Activate(TRUE);  
	m_toolTip.SetDelayTime(200);  
	

	time_t t = time(0);
	struct tm * ptNow = NULL;

	char logInfoName[MAX_PATH] = {0};
	
	ptNow = localtime(&t);

	CreateDirectory("log", NULL);
	
	sprintf(logInfoName, "log/ISPinfo_%04d_%02d_%02d,%02d,%02d,%02d.txt", 
		ptNow->tm_year + 1900, ptNow->tm_mon + 1, ptNow->tm_mday, ptNow->tm_hour, ptNow->tm_min, ptNow->tm_sec);

	freopen(logInfoName, "w+t", stderr);

	//子文件id下拉列表初始化
	m_SubFileIdCtrl.AddString("0 (day)");
	m_SubFileIdCtrl.AddString("1 (night)");
	m_SubFileIdCtrl.AddString("2");
	m_SubFileIdCtrl.AddString("3");
	m_SubFileIdCtrl.AddString("4");
	
	m_SubFileIdCtrl.SetCurSel(m_SubFileId);


	
	//主界面树形目录初始化
	HTREEITEM hRoot;

	m_trModules.SetBkColor(RGB(149, 223, 229));
	hRoot = m_trModules.InsertItem(STR_MODULE);
	m_trModules.InsertItem(STR_SYSTEM, hRoot);
	m_trModules.InsertItem(STR_BB, hRoot);
	m_trModules.InsertItem(STR_LSC, hRoot);
	m_trModules.InsertItem(STR_RAWLUT, hRoot);
	m_trModules.InsertItem(STR_DPC, hRoot);

	m_trModules.InsertItem(STR_DENOISE, hRoot);
#ifdef ANYKA_DEVELOP
	m_trModules.InsertItem(STR_GB, hRoot);
	m_trModules.InsertItem(STR_DEMOSAIC, hRoot);
#endif
	m_trModules.InsertItem(STR_WB, hRoot);
	m_trModules.InsertItem(STR_CCM, hRoot);

	m_trModules.InsertItem(STR_GAMMA, hRoot);
#ifdef ANYKA_DEVELOP
	m_trModules.InsertItem(STR_RGB2YUV, hRoot);
#endif
	m_trModules.InsertItem(STR_CONTRAST, hRoot);

	m_trModules.InsertItem(STR_Y_GAMMA, hRoot);

#ifdef ANYKA_DEVELOP
	m_trModules.InsertItem(STR_WDR, hRoot);
#endif
	m_trModules.InsertItem(STR_SATURATION, hRoot);
	
	m_trModules.InsertItem(STR_3DNR, hRoot);
#if 0
	m_trModules.InsertItem(STR_EDGE, hRoot);
#endif
	m_trModules.InsertItem(STR_SHARP, hRoot);
#ifdef ANYKA_DEVELOP
	m_trModules.InsertItem(STR_FCS, hRoot);
#endif
#if 1
	m_trModules.InsertItem(STR_HUE, hRoot);
#endif
#ifdef ANYKA_DEVELOP
	m_trModules.InsertItem(STR_YUVEFFECT, hRoot);
#endif
	m_trModules.InsertItem(STR_EXP, hRoot);
#ifdef ANYKA_DEVELOP
	m_trModules.InsertItem(STR_AF, hRoot);
#endif
	m_trModules.InsertItem(STR_zone_weight, hRoot);
#ifdef ANYKA_DEVELOP
	m_trModules.InsertItem(STR_MISC, hRoot);
#endif
	m_trModules.InsertItem(STR_STAT, hRoot);
#ifdef ANYKA_DEVELOP
	m_trModules.InsertItem(STR_COMPARE, hRoot);
#endif

	
	

	
	m_trModules.Expand(hRoot, TVE_EXPAND);

	m_ConnectState.Format("unconnected");
	UpdateData(FALSE);

	
	//各个对话框初始化
	int i = 0;
	for (i=0; i<DIALOG_NUM; i++)
	{
		pDlg[i] = NULL;
	}

	DialogId = DIALOG_SYS;

	CRect rect;
	this->GetClientRect(&rect);

	rect.top += 140;
	rect.left += 160;
	rect.right -= 180;

	m_SysDlg.Create(IDD_DIALOG_SYS, this);
	m_SysDlg.SetPageID(DIALOG_SYS);
	m_SysDlg.SetMessageWindow(this);
	pDlg[DIALOG_SYS] = &m_SysDlg;
	m_SysDlg.MoveWindow(&rect);
	m_SysDlg.ShowWindow(SW_SHOW);

	m_BBDlg.Create(IDD_DIALOG_BB, this);
	m_BBDlg.SetPageID(DIALOG_BB);
	m_BBDlg.SetMessageWindow(this);
	pDlg[DIALOG_BB] = &m_BBDlg;
	m_BBDlg.MoveWindow(&rect);

	m_DpcDlg.Create(IDD_DIALOG_DPC, this);
	m_DpcDlg.SetPageID(DIALOG_DPC);
	m_DpcDlg.SetMessageWindow(this);
	pDlg[DIALOG_DPC] = &m_DpcDlg;
	m_DpcDlg.MoveWindow(&rect);

	m_GBDlg.Create(IDD_DIALOG_GB, this);
	m_GBDlg.SetPageID(DIALOG_GB);
	m_GBDlg.SetMessageWindow(this);
	pDlg[DIALOG_GB] = &m_GBDlg;
	m_GBDlg.MoveWindow(&rect);

	m_LscDlg.Create(IDD_DIALOG_LSC, this);
	m_LscDlg.SetPageID(DIALOG_LSC);
	m_LscDlg.SetMessageWindow(this);
	pDlg[DIALOG_LSC] = &m_LscDlg;
	m_LscDlg.MoveWindow(&rect);

	m_RawlutDlg.SetMessageWindow(this);
	m_RawlutDlg.Create(IDD_DIALOG_RAWLUT, this);
	m_RawlutDlg.SetPageID(DIALOG_RAWLUT);
	pDlg[DIALOG_RAWLUT] = &m_RawlutDlg;
	m_RawlutDlg.MoveWindow(&rect);

	m_DemosaicDlg.Create(IDD_DIALOG_DEMOSAIC, this);
	m_DemosaicDlg.SetPageID(DIALOG_DEMOSAIC);
	m_DemosaicDlg.SetMessageWindow(this);
	pDlg[DIALOG_DEMOSAIC] = &m_DemosaicDlg;
	m_DemosaicDlg.MoveWindow(&rect);

	m_CCMDlg.Create(IDD_DIALOG_CCM, this);
	m_CCMDlg.SetPageID(DIALOG_CCM);
	m_CCMDlg.SetMessageWindow(this);
	pDlg[DIALOG_CCM] = &m_CCMDlg;
	m_CCMDlg.MoveWindow(&rect);
	
	m_GammaDlg.SetMessageWindow(this);
	m_GammaDlg.Create(IDD_DIALOG_GAMMA, this);
	m_GammaDlg.SetPageID(DIALOG_GAMMA);
	
	pDlg[DIALOG_GAMMA] = &m_GammaDlg;
	m_GammaDlg.MoveWindow(&rect);

	m_DenoiseDlg.Create(IDD_DIALOG_DENOISE, this);
	m_DenoiseDlg.SetPageID(DIALOG_DENOISE);
	m_DenoiseDlg.SetMessageWindow(this);
	pDlg[DIALOG_DENOISE] = &m_DenoiseDlg;
	m_DenoiseDlg.MoveWindow(&rect);

	m_SharpDlg.Create(IDD_DIALOG_SHARP, this);
	m_SharpDlg.SetPageID(DIALOG_SHARP);
	m_SharpDlg.SetMessageWindow(this);
	pDlg[DIALOG_SHARP] = &m_SharpDlg;
	m_SharpDlg.MoveWindow(&rect);

/*	m_EdgeDlg.Create(IDD_DIALOG_EDGE, this);
	m_EdgeDlg.SetPageID(DIALOG_EDGE);
	m_EdgeDlg.SetMessageWindow(this);
	pDlg[DIALOG_EDGE] = &m_EdgeDlg;
	m_EdgeDlg.MoveWindow(&rect);*/

	m_3DNRDlg.Create(IDD_DIALOG_3DNR, this);
	m_3DNRDlg.SetPageID(DIALOG_3DNR);
	m_3DNRDlg.SetMessageWindow(this);
	pDlg[DIALOG_3DNR] = &m_3DNRDlg;
	m_3DNRDlg.MoveWindow(&rect);

	m_WdrDlg.Create(IDD_DIALOG_WDR, this);
	m_WdrDlg.SetPageID(DIALOG_WDR);
	m_WdrDlg.SetMessageWindow(this);
	pDlg[DIALOG_WDR] = &m_WdrDlg;
	m_WdrDlg.MoveWindow(&rect);

	m_FcsDlg.Create(IDD_DIALOG_FCS, this);
	m_FcsDlg.SetPageID(DIALOG_FCS);
	m_FcsDlg.SetMessageWindow(this);
	pDlg[DIALOG_FCS] = &m_FcsDlg;
	m_FcsDlg.MoveWindow(&rect);

	m_YuvEffectDlg.Create(IDD_DIALOG_YUVEFFECT, this);
	m_YuvEffectDlg.SetPageID(DIALOG_YUVEFFECT);
	m_YuvEffectDlg.SetMessageWindow(this);
	pDlg[DIALOG_YUVEFFECT] = &m_YuvEffectDlg;
	m_YuvEffectDlg.MoveWindow(&rect);

	m_Rgb2YuvDlg.Create(IDD_DIALOG_RGB2YUV, this);
	m_Rgb2YuvDlg.SetPageID(DIALOG_RGB2YUV);
	m_Rgb2YuvDlg.SetMessageWindow(this);
	pDlg[DIALOG_RGB2YUV] = &m_Rgb2YuvDlg;
	m_Rgb2YuvDlg.MoveWindow(&rect);

	m_SaturationDlg.Create(IDD_DIALOG_SATURATION, this);
	m_SaturationDlg.SetPageID(DIALOG_SATURATION);
	m_SaturationDlg.SetMessageWindow(this);
	pDlg[DIALOG_SATURATION] = &m_SaturationDlg;
	m_SaturationDlg.MoveWindow(&rect);

	m_ContrastDlg.Create(IDD_DIALOG_CONTRAST, this);
	m_ContrastDlg.SetPageID(DIALOG_CONTRAST);
	m_ContrastDlg.SetMessageWindow(this);
	pDlg[DIALOG_CONTRAST] = &m_ContrastDlg;
	m_ContrastDlg.MoveWindow(&rect);

	m_WbDlg.Create(IDD_DIALOG_WB, this);
	m_WbDlg.SetPageID(DIALOG_WB);
	m_WbDlg.SetMessageWindow(this);
	pDlg[DIALOG_WB] = &m_WbDlg;
	m_WbDlg.MoveWindow(&rect);

	m_ExpDlg.Create(IDD_DIALOG_EXP, this);
	m_ExpDlg.SetPageID(DIALOG_EXP);
	m_ExpDlg.SetMessageWindow(this);
	pDlg[DIALOG_EXP] = &m_ExpDlg;
	m_ExpDlg.MoveWindow(&rect);

	m_AFDlg.Create(IDD_DIALOG_AF, this);
	m_AFDlg.SetPageID(DIALOG_AF);
	m_AFDlg.SetMessageWindow(this);
	pDlg[DIALOG_AF] = &m_AFDlg;
	m_AFDlg.MoveWindow(&rect);

	m_StatDlg.Create(IDD_DIALOG_STAT, this);
	m_StatDlg.SetPageID(DIALOG_STAT);
	m_StatDlg.SetMessageWindow(this);
	pDlg[DIALOG_STAT] = &m_StatDlg;
	m_StatDlg.MoveWindow(&rect);

	m_Zone_WeightDlg.Create(IDD_DIALOG_ZONE_WEIGHT, this);
	m_Zone_WeightDlg.SetPageID(DIALOG_ZONE_WEIGHT);
	m_Zone_WeightDlg.SetMessageWindow(this);
	pDlg[DIALOG_ZONE_WEIGHT] = &m_Zone_WeightDlg;
	m_Zone_WeightDlg.MoveWindow(&rect);

	m_MiscDlg.Create(IDD_DIALOG_MISC, this);
	m_MiscDlg.SetPageID(DIALOG_MISC);
	m_MiscDlg.SetMessageWindow(this);
	pDlg[DIALOG_MISC] = &m_MiscDlg;
	m_MiscDlg.MoveWindow(&rect);

	m_cmpDlg.Create(IDD_DIALOG_COMPARE, this);
	m_cmpDlg.SetPageID(DIALOG_COMPARE);
	m_cmpDlg.SetMessageWindow(this);
	pDlg[DIALOG_COMPARE] = &m_cmpDlg;
	m_cmpDlg.MoveWindow(&rect);

	m_HueDlg.Create(IDD_DIALOG_HUE, this);
	m_HueDlg.SetPageID(DIALOG_HUE);
	m_HueDlg.SetMessageWindow(this);
	pDlg[DIALOG_HUE] = &m_HueDlg;
	m_HueDlg.MoveWindow(&rect);

	m_Y_GammaDlg.SetMessageWindow(this);
	m_Y_GammaDlg.Create(IDD_DIALOG_Y_GAMMA, this);
	m_Y_GammaDlg.SetPageID(DIALOG_Y_GAMMA);
	
	pDlg[DIALOG_Y_GAMMA] = &m_Y_GammaDlg;
	m_Y_GammaDlg.MoveWindow(&rect);

	//CRect rect;
	this->GetClientRect(&rect);
	rect.top += 140;
	rect.left += 590;
	m_TextDlg.Create(IDD_DIALOG_TEXT, this);
	m_TextDlg.SetMessageWindow(this);
	m_TextDlg.MoveWindow(&rect);
	m_TextDlg.ShowWindow(SW_SHOW);

	CRichEditCtrl* pEdit = (CRichEditCtrl*)m_TextDlg.GetDlgItem(IDC_EDIT_TEXT);
	pEdit->LimitText(2<<20);

	//注册网络接收回调
	m_NetCtrl.SetRecvCallBack(OnRecvRespond, this);
	
	Create_Send_Heart_Thread();

	BYTE a=0, b=0, c=0, d=0;
	GetIpAddr(&a, &b, &c, &d);
	m_Ip.SetAddress(a, b, c, d);
	SetWindowTitle();

	m_CCMDlg.SetConnectState(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CISPCTRL_TOOLDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CISPCTRL_TOOLDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDC* pDC = GetDC();
		CPen zoneLinePen(PS_SOLID, 1, RGB(172, 168, 153));

		pDC->SelectObject(&zoneLinePen);

		pDC->MoveTo(140, 130);
		pDC->LineTo(780, 130);

		CPen zoneLinePen2(PS_SOLID, 1, RGB(255, 255, 255));

		pDC->SelectObject(&zoneLinePen2);

		pDC->MoveTo(140, 131);
		pDC->LineTo(780, 131);

		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CISPCTRL_TOOLDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CISPCTRL_TOOLDlg::OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	HTREEITEM hCurItem;
	CString str;

	hCurItem = m_trModules.GetSelectedItem();
	
	if (NULL != hCurItem)
	{
		int i = 0;
		for (i=0; i<DIALOG_NUM; i++)
		{
			if (pDlg[i] != NULL)
			{
				pDlg[i]->ShowWindow(SW_HIDE);
			}
		}
		
		m_gamma_copyUItoTest_flag = FALSE;
		str = m_trModules.GetItemText(hCurItem);

		if ((DIALOG_SYS == DialogId) && (STR_SYSTEM != str) && (STR_MODULE != str))
		{
			OnPageCopyTextToUi(0, 0);
		}

		if (STR_BB == str)
		{
			DialogId = DIALOG_BB;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("黑电平"));  
		}
		else if (STR_DPC == str)
		{
			DialogId = DIALOG_DPC;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("坏块较正")); 
		}
		else if (STR_GB == str)
		{
			DialogId = DIALOG_GB;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("绿平衡")); 
		}
		else if (STR_LSC == str)
		{
			DialogId = DIALOG_LSC;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("镜头较正")); 
		}
		else if (STR_RAWLUT == str)
		{
			m_gamma_copyUItoTest_flag = TRUE;
			DialogId = DIALOG_RAWLUT;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("RAW Gamma曲线")); 
		}
		else if (STR_DEMOSAIC == str)
		{
			DialogId = DIALOG_DEMOSAIC;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("颜色插值"));
		}
		else if (STR_CCM == str)
		{
			DialogId = DIALOG_CCM;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("颜色较正"));
		}
		else if (STR_GAMMA == str)
		{
			m_gamma_copyUItoTest_flag = TRUE;
			DialogId = DIALOG_GAMMA;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("RGB Gamma曲线"));
		}
		else if (STR_DENOISE == str)
		{
			DialogId = DIALOG_DENOISE;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("降噪"));
			
		}
		else if (STR_SHARP == str)
		{
			DialogId = DIALOG_SHARP;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("锐化"));
		}
		/*else if (STR_EDGE == str)
		{
			DialogId = DIALOG_EDGE;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("边缘增强"));
		}*/
		else if (STR_3DNR == str)
		{
			DialogId = DIALOG_3DNR;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("3D降噪"));
		}
		else if (STR_WDR == str)
		{
			DialogId = DIALOG_WDR;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("宽动态"));
		}
		else if (STR_FCS == str)
		{
			DialogId = DIALOG_FCS;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("假色抑制"));
		}
		else if (STR_YUVEFFECT == str)
		{
			DialogId = DIALOG_YUVEFFECT;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("YUV效果"));
		}
		else if (STR_RGB2YUV == str)
		{
			DialogId = DIALOG_RGB2YUV;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("RGB转YUV"));
		}
		else if (STR_SATURATION == str)
		{
			DialogId = DIALOG_SATURATION;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("饱和度"));
		}
		else if (STR_CONTRAST == str)
		{
			DialogId = DIALOG_CONTRAST;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("对比度"));
		}
		else if (STR_WB == str)
		{
			DialogId = DIALOG_WB;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("白平衡"));
		}
		else if (STR_EXP == str)
		{
			DialogId = DIALOG_EXP;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("自动曝光"));
		}
		else if (STR_AF == str)
		{
			DialogId = DIALOG_AF;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("自动对焦"));
		}
		else if (STR_STAT == str)
		{
			DialogId = DIALOG_STAT;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("统计"));			
		}
		else if (STR_zone_weight == str)
		{
			DialogId = DIALOG_ZONE_WEIGHT;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("区域权重设置"));
		}
		else if (STR_MISC == str)
		{
			DialogId = DIALOG_MISC;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("其他"));
		}
		else if (STR_SYSTEM == str)
		{
			DialogId = DIALOG_SYS;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("系统"));
		}
		else if (STR_COMPARE == str)
		{
			DialogId = DIALOG_COMPARE;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("比较配置文件"));
		}
		else if (STR_HUE == str)
		{
			DialogId = DIALOG_HUE;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("HUE效果调节"));
		}
		else if (STR_Y_GAMMA == str)
		{
			DialogId = DIALOG_Y_GAMMA;
			m_toolTip.AddTool(GetDlgItem(IDC_TREE1), _T("Y Gamma 曲线"));
		}
		else
		{
			DialogId = DIALOG_SYS;
		}
		m_toolTip.Pop();  

		if (NULL != pDlg[DialogId])
		{
			pDlg[DialogId]->ShowWindow(SW_SHOW);
		}

		m_TextDlg.OnButton_Enable(DialogId);
		OnPageCopyUiToText(0, 0);
	}

	*pResult = 0;
}


BOOL CISPCTRL_TOOLDlg::Connet_FTPServer(LPCTSTR addr) 
{

	CString username = (_T("root"));  
	UpdateData(TRUE);

	if(m_pInetSession != NULL)
	{
		delete m_pInetSession;
		m_pInetSession = NULL;
	}
	m_pInetSession = new CInternetSession(AfxGetAppName(), 1, PRE_CONFIG_INTERNET_ACCESS);
	try
	{
		// addr       ftp服务器的地址  LPCTSTR
		// username   登陆用户名       LPCTSTR 
		// password   密码            LPCTSTR
		// port       端口            UINT
		if(m_pFtpConnection != NULL)
		{
			m_pFtpConnection->Close();
			delete m_pFtpConnection;
			m_pFtpConnection = NULL;
		}
		m_pFtpConnection = m_pInetSession->GetFtpConnection(addr, username, m_Password, 21);
		//m_pFtpConnection = m_pInetSession->GetFtpConnection(addr, m_username, m_password, m_uPort);
	}
	catch(CInternetException *pEx)//若登陆不成功则抛出异常，以下是针对异常的处理
	{
		TCHAR szError[1024] = {0};

		SetMstboxTimer();
		
		if(pEx->GetErrorMessage(szError,1024))
		{
			AfxMessageBox(szError);
		}
		else
		{
			AfxMessageBox(_T("There was an exception"));
		}
		pEx->Delete();
		m_pFtpConnection = NULL;
		return FALSE;
    }

	return TRUE;
}


BOOL CISPCTRL_TOOLDlg::Send_cmd(char commad_type, BOOL auto_test_flag, char *file_name, char *param)
{

	char *lpBuf = NULL;
	char *lpBuf_temp = NULL;
	short nBufLen = 0;
	UINT i = 0;
	int nFlags = 0;
	short len_name = 0;
	short len_data = 0;
	short check_sum_temp = 0;
	T_NET_INFO trance = {0};
	char temp[5] = {0};
	int ret = 0;
	BOOL no_data_flag = TRUE;
	BOOL no_filename_flag = TRUE;
	

	USES_CONVERSION;

	trance.data_param = NULL;
	trance.file_name = NULL;


	nBufLen = 2;
	trance.commad_type = commad_type;
	nBufLen += 1;
	check_sum_temp += commad_type;

	trance.auto_test_flag = auto_test_flag;
	nBufLen += 1;
	check_sum_temp += auto_test_flag;

	if (file_name != NULL)
	{
		for (i = 0; i < MAX_PATH && file_name[i] != 0; i++)
		{
			if(file_name[i] != '0')
			{
				no_filename_flag = FALSE;
				break;
			}
		}
	}
	
	if (file_name != NULL && !no_filename_flag)
	{
		len_name = strlen(file_name);
		if(len_name != 0 )
		{
			trance.file_name = (char *)malloc(len_name + 2 + 1);
			if (trance.file_name == NULL)
			{
				return FALSE;
			}
			memset(trance.file_name, 0, len_name + 2 + 1);

			strncpy(trance.file_name, (char *)&len_name, 2);
			strncpy(&trance.file_name[2], file_name, len_name);
			nBufLen += (len_name + 2);
			for (i = 0; i < (UINT)(len_name + 2); i++)
			{
				check_sum_temp += trance.file_name[i];
			}
		}
	}
	else
	{
		nBufLen += 2;
	}
	
	if (param != NULL)
	{
		for (i = 0; i < MAX_PATH && param[i] != 0; i++)
		{
			if(param[i] != '0')
			{
				no_data_flag = FALSE;
				break;
			}
		}
	}

	if (param != NULL && !no_data_flag)
	{
		len_data = strlen(param);
		if(len_data != 0 )
		{
			trance.data_param = (char *)malloc(len_data + 2+1);
			if (trance.data_param == NULL)
			{
				return FALSE;
			}
			memset(trance.data_param, 0, len_data + 2 + 1);

			strncpy(trance.data_param, (char *)&len_data, 2);
			strncpy(&trance.data_param[2], param, len_data);
			nBufLen += (len_data + 2);
			for (i = 0; i < (UINT)(len_data + 2); i++)
			{
				check_sum_temp += trance.data_param[i];
			}
		}
	}
	else
	{
		nBufLen += 2;
	}

	trance.check_sum = check_sum_temp;
	nBufLen +=  2;
	trance.len = nBufLen;


	lpBuf = (char *)malloc(nBufLen + 1);
	if (lpBuf == NULL)
	{
		return FALSE;
	}
	
	//打包
	strncpy(lpBuf, (char *)&trance.len, 2);
	strncpy(&lpBuf[2], &trance.commad_type, 1);
	strncpy(&lpBuf[3], (char *)&trance.auto_test_flag, 1);
	if (len_name != 0 && file_name != NULL && !no_filename_flag)
	{
		strncpy(&lpBuf[4], (char *)&len_name, 2);
		strncpy(&lpBuf[4+2], file_name, len_name);
		//strncpy(&lpBuf[5], trance.file_name, len_name+4);
	}
	else
	{
		strncpy(&lpBuf[4], temp, 2);
	}
	len_name = len_name + 2;//因为增加4个字节的长度


	if (len_data != 0 && param != NULL && !no_data_flag)
	{
		strncpy(&lpBuf[4+len_name], (char *)&len_data, 2);
		strncpy(&lpBuf[4+len_name+2], param, len_data);
		//strncpy(&lpBuf[5+len_name+4], trance.data_param, len_data+4);
	}
	else
	{
		strncpy(&lpBuf[4 + len_name], temp, 2);
	}
	len_data = len_data + 2;  //因为增加4个字节的长度


	strncpy(&lpBuf[4+len_name+len_data], (char *)&trance.check_sum, 2);
	

	ret = m_NetCtrl.Socket_Send(lpBuf, nBufLen);
	
	if (!ret)
	{
		return FALSE;
	}
	
	if (trance.data_param)
	{
		free(trance.data_param);
	}

	if (trance.file_name)
	{
		free(trance.file_name);
	}

	if (lpBuf)
	{
		free(lpBuf);
	}

	if (lpBuf_temp)
	{
		free(lpBuf_temp);
	}

	return TRUE;
}


BOOL CISPCTRL_TOOLDlg::Anyka_Test_check_info()
{
	UINT time1 = 0;
	UINT time2 = 0;
	UINT delaytime = 0; 

	USES_CONVERSION;
	
	time1 = GetTickCount();
	while (1)
	{
		time2 = GetTickCount();
		Sleep(50);
		delaytime = 10*1000; 
		if (time2 - time1 > delaytime)
		{
			m_NetCtrl.test_pass_flag = 0;
			AfxMessageBox(_T("超时(10秒)没有返回确认命令")); 
			return FALSE;
		}
		if (m_NetCtrl.test_pass_flag == 1)
		{
			
			m_NetCtrl.test_pass_flag = 0;
			return TRUE;
		}
		else if (m_NetCtrl.test_pass_flag == 2)
		{
			m_NetCtrl.test_pass_flag = 0;
			return FALSE;
		}
	}

	return TRUE;
}


TCHAR *CISPCTRL_TOOLDlg::ConvertAbsolutePath(LPCTSTR path)
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

BOOL  CISPCTRL_TOOLDlg::OnDownload_tuner(LPCTSTR addr) 
{
	CString strSourceName, strDestName, str;
	TCHAR isptuner_name[MAX_PATH] = _T("isptuner");
	TCHAR DestName[MAX_PATH] = _T("/tmp/");
	char name_buf[MAX_PATH] = {0};
	UINT name_len = 0;
	TCHAR addr_temp[MAX_PATH] = {0};
	UINT net_uPort = 6789;


	USES_CONVERSION;

	m_NetCtrl.test_pass_flag = 0;
	_tcscpy(addr_temp, addr);
	memcpy(name_buf, addr_temp, _tcslen(addr_temp));
	if (m_NetCtrl.creat_socket(name_buf, net_uPort))
	{
		if (!Connet_FTPServer(addr))
		{
			AfxMessageBox(_T("连接FTP服务器失败"));
			return FALSE;
		}
		m_NetCtrl.g_test_flag = FALSE;
		if (!m_NetCtrl.create_thread_heat(name_buf, net_uPort+1))
		{
			AfxMessageBox(_T("创建心跳线程失败"));
			return FALSE ;
		}

		//Sleep(5000);
		m_NetCtrl.g_test_flag = TRUE;

		//put文件
		//判断文件是否存在
		if(0xFFFFFFFF == GetFileAttributes(ConvertAbsolutePath(isptuner_name)))
		{
			str.Format(_T("%s no exist"), isptuner_name);
			AfxMessageBox(str);   
			return FALSE;
		}

		//发送文件
		_tcscat(DestName, isptuner_name);
		strDestName.Format(_T("%s"), DestName);
		strSourceName.Format(_T("%s"), ConvertAbsolutePath(isptuner_name));
		if (!m_pFtpConnection->PutFile(strSourceName, strDestName, FTP_TRANSFER_TYPE_BINARY, 1))   
		{
			AfxMessageBox(_T("put isptuner file error"));   
			return FALSE;
		}
		
		//发送命令
		name_len = strlen(T2A(isptuner_name));
		memset(name_buf, 0, MAX_PATH);
		memcpy(name_buf, T2A(isptuner_name), name_len);


		if (!Send_cmd(TEST_COMMAND, 0, name_buf, NULL))
		{
			AfxMessageBox(_T("send cmd error"));   
			return FALSE;
		}
		//接收返回值
		if (!Anyka_Test_check_info())
		{
			return FALSE;
		}

	}
	else
	{
		return FALSE;
	}

	
	return TRUE;

}

void CISPCTRL_TOOLDlg::OnDownload_tuner_close()
{
	m_NetCtrl.Heat_Socket_Close();

	OnFTP_close();

	m_NetCtrl.Socket_close();	
}

void CISPCTRL_TOOLDlg::OnFTP_close()
{
	if(m_pInetSession != NULL)
	{
		delete m_pInetSession;
		m_pInetSession = NULL;
	}
	
	if(m_pFtpConnection != NULL)
	{
		m_pFtpConnection->Close();
		delete m_pFtpConnection;
		m_pFtpConnection = NULL;
	}
}


void CISPCTRL_TOOLDlg::OnButtonConnect() 
{
	// TODO: Add your control notification handler code here
	bool ret = FALSE;
	CString addr;

	if (!m_NetCtrl.IsConnected())
	{
		UpdateData(TRUE);
		
		BYTE f0, f1, f2, f3;
		
		m_Ip.GetAddress(f0, f1, f2, f3);
		addr.Format(_T("%d.%d.%d.%d"), f0, f1, f2, f3);
		m_RemoteIp = (f3<<24) + (f2<<16) + (f1<<8) + f0;
#ifdef NEW_PLATFORM
		if (m_NetCtrl.TcpClientConnect(m_RemoteIp, m_Port))
		{
#if 0
			if (!Connet_FTPServer(addr))
			{
				m_NetCtrl.TcpClientClose();
				return;
			}

			OnFTP_close();
#endif			
			m_ConnectState.Format("Connected");
			m_btnConnect.SetWindowText("Close");
			UpdateData(FALSE);

			m_NetCtrl.SendCommand(ISP_CFG_DATA, CMD_GET, NULL, 0);
			m_bOnlySensor = TRUE;

			SaveIpAddr(f0, f1, f2, f3);

		//	m_heartbeat_timer = SetTimer(HEARTBEAT_SEND_TIMER_ID, HEARTBEAT_SEND_TIME, NULL);
			m_check_timer = SetTimer(HEARTBEAT_CHECK_TIMER_ID, HEARTBEAT_TIMEOUT, NULL);

			m_StatDlg.SetConnectState(TRUE);
			m_CCMDlg.SetConnectState(TRUE);
			m_LscDlg.SetConnectState(TRUE);
			m_HueDlg.m_HUE_calc.Img_SetConnectState(TRUE);
			AfxMessageBox("连接成功！", 0, 0);
		}
		else
		{
			AfxMessageBox("连接失败！", 0, 0);
		}
#else
		{
	
			//下载tuner程序
			if (!OnDownload_tuner(addr))
			{
				OnDownload_tuner_close();
				return;
			}
			OnDownload_tuner_close();
		
			
			if (m_NetCtrl.TcpClientConnect(m_RemoteIp, m_Port))
			{
				m_ConnectState.Format("Connected");
				m_btnConnect.SetWindowText("Close");
				UpdateData(FALSE);

				m_NetCtrl.SendCommand(ISP_CFG_DATA, CMD_GET, NULL, 0);
				m_bOnlySensor = TRUE;

				SaveIpAddr(f0, f1, f2, f3);

		//		m_heartbeat_timer = SetTimer(HEARTBEAT_SEND_TIMER_ID, HEARTBEAT_SEND_TIME, NULL);
				m_check_timer = SetTimer(HEARTBEAT_CHECK_TIMER_ID, HEARTBEAT_TIMEOUT, NULL);

				m_StatDlg.SetConnectState(TRUE);
				m_CCMDlg.SetConnectState(TRUE);
				m_LscDlg.SetConnectState(TRUE);
				m_HueDlg.m_HUE_calc.Img_SetConnectState(TRUE);
				AfxMessageBox("连接成功！", 0, 0);
			}
			else
			{
				AfxMessageBox("连接失败！", 0, 0);
			}
		}
#endif
	}
	else 
	{
		if (m_NetCtrl.TcpClientClose())
		{
			m_ConnectState.Format("unconnected");
			m_btnConnect.SetWindowText("Connect");
			UpdateData(FALSE);

			KillTimer(m_heartbeat_timer);
			KillTimer(m_check_timer);

			m_StatDlg.SetConnectState(FALSE);
			m_CCMDlg.SetConnectState(FALSE);
			m_LscDlg.SetConnectState(FALSE);
			m_HueDlg.m_HUE_calc.Img_SetConnectState(FALSE);
		}
	}
}

#define PAGE_INFO_MAX_LEN	(1024*12)
#define CFG_INFO_MAX_LEN	(1024*200)

void CISPCTRL_TOOLDlg::SendIspParam_ForText(BOOL bNeedJudge, int nPageID, int nFlag)
{
	short addr_id = ISP_BB;
	char *send_buf = NULL;
	UINT send_buf_len = 0;
	BOOL ret = FALSE;
	char *buf = NULL;
	UINT buf_len = 0;
	CString str;
	UINT idex = 0, i = 0;

	if (!m_NetCtrl.IsConnected()) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法发送ISP参数!\n", 0, 0);
		return;
	}

	buf = (char *)malloc(MAX_BUF_SHARP_LEN);
	if (buf == NULL)
	{
		AfxMessageBox("内存分配失败!\n", 0, 0);
		return;
	}
	send_buf = (char *)malloc(MAX_BUF_SHARP_LEN);
	if (send_buf == NULL)
	{
		free(buf);
		AfxMessageBox("内存分配失败!\n", 0, 0);
		return;
	}

	buf_len = m_TextDlg.Get_text_info(buf);
	if (buf_len == 0)
	{
		free(send_buf);
		free(buf);
		return;
	}
	
	switch (nPageID)
	{
		case DIALOG_BB:
			if (!m_TextDlg.Get_packetData_BB(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_BB;
			Isp_init_param.p_Isp_blc.param_id = ISP_BB;
			Isp_init_param.p_Isp_blc.length = sizeof(Isp_init_param.p_Isp_blc);
			send_buf_len = Isp_init_param.p_Isp_blc.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_blc , send_buf_len);
			break;
		case DIALOG_LSC:
			if (!m_TextDlg.Get_packetData_LSC(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_LSC;
			Isp_init_param.p_Isp_lsc.param_id = ISP_LSC;
			Isp_init_param.p_Isp_lsc.length = sizeof(Isp_init_param.p_Isp_lsc);
			send_buf_len = Isp_init_param.p_Isp_lsc.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_lsc , send_buf_len);
			break;
		case DIALOG_CCM:
			if (!m_TextDlg.Get_packetData_CCM(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_CCM;
			Isp_init_param.p_Isp_ccm.param_id = ISP_CCM;
			Isp_init_param.p_Isp_ccm.length = sizeof(Isp_init_param.p_Isp_ccm);
			send_buf_len = Isp_init_param.p_Isp_ccm.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_ccm , send_buf_len);
			break;
		case DIALOG_WB:
			if (!m_TextDlg.Get_packetData_WB(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_WB;
			Isp_init_param.p_Isp_wb.param_id = ISP_WB;
			Isp_init_param.p_Isp_wb.length = sizeof(Isp_init_param.p_Isp_wb);
			send_buf_len = Isp_init_param.p_Isp_wb.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_wb , send_buf_len);
			break;
		case DIALOG_DPC:
			if (!m_TextDlg.Get_packetData_DPC(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_DPC;
			Isp_init_param.p_Isp_dpc.param_id = ISP_DPC;
			Isp_init_param.p_Isp_dpc.length = sizeof(Isp_init_param.p_Isp_dpc);
			send_buf_len = Isp_init_param.p_Isp_dpc.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_dpc , send_buf_len);
			break;
		case DIALOG_GB:
			if (!m_TextDlg.Get_packetData_GB(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_GB;
			Isp_init_param.p_Isp_gb.param_id = ISP_GB;
			Isp_init_param.p_Isp_gb.length = sizeof(Isp_init_param.p_Isp_gb);
			send_buf_len = Isp_init_param.p_Isp_gb.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_gb , send_buf_len);
			break;
		case DIALOG_RAWLUT:
			if (!m_TextDlg.Get_packetData_RAW_LUT(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_RAW_LUT;
			Isp_init_param.p_Isp_raw_lut.param_id = ISP_RAW_LUT;
			Isp_init_param.p_Isp_raw_lut.length = sizeof(Isp_init_param.p_Isp_raw_lut);
			send_buf_len = Isp_init_param.p_Isp_raw_lut.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_raw_lut , send_buf_len);
			break;
		case DIALOG_DEMOSAIC:
			if (!m_TextDlg.Get_packetData_DEMO(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_DEMO;
			Isp_init_param.p_Isp_demo.param_id = ISP_DEMO;
			Isp_init_param.p_Isp_demo.length = sizeof(Isp_init_param.p_Isp_demo);
			send_buf_len = Isp_init_param.p_Isp_demo.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_demo , send_buf_len);
			break;
		case DIALOG_GAMMA:
			if (!m_TextDlg.Get_packetData_GAMMA(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_GAMMA;
			Isp_init_param.p_Isp_gamma.param_id = ISP_GAMMA;
			Isp_init_param.p_Isp_gamma.length = sizeof(Isp_init_param.p_Isp_gamma);
			send_buf_len = Isp_init_param.p_Isp_gamma.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_gamma , send_buf_len);
			break;
		case DIALOG_GAMMA_RGB:
			break;
		case DIALOG_DENOISE:
			if (!m_TextDlg.Get_packetData_NR(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_NR;
			Isp_init_param.p_Isp_nr.param_id = ISP_NR;
			Isp_init_param.p_Isp_nr.length = sizeof(Isp_init_param.p_Isp_nr);
			send_buf_len = Isp_init_param.p_Isp_nr.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_nr , send_buf_len);
			break;
		case DIALOG_SHARP:
			if (!m_TextDlg.Get_packetData_SHARP(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_SHARP;
			Isp_init_param.p_Isp_sharp.param_id = ISP_SHARP;
			Isp_init_param.p_Isp_sharp.length = sizeof(Isp_init_param.p_Isp_sharp);
			send_buf_len = Isp_init_param.p_Isp_sharp.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_sharp , send_buf_len);
			break;
		case DIALOG_HUE:
			if (!m_TextDlg.Get_packetData_hue(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_HUE;
			Isp_init_param.p_Isp_hue.param_id = ISP_HUE;
			Isp_init_param.p_Isp_hue.length = sizeof(Isp_init_param.p_Isp_hue);
			send_buf_len = Isp_init_param.p_Isp_hue.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_hue , send_buf_len);
			break;
		/*case DIALOG_EDGE:
			if (!m_TextDlg.Get_packetData_EDGE(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_EDGE;
			Isp_init_param.p_Isp_edge.param_id = ISP_EDGE;
			Isp_init_param.p_Isp_edge.length = sizeof(Isp_init_param.p_Isp_edge);
			send_buf_len = Isp_init_param.p_Isp_edge.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_edge , send_buf_len);
			break;*/
		case DIALOG_3DNR:
			if (!m_TextDlg.Get_packetData_3DNR(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_3DNR;
			Isp_init_param.p_Isp_3dnr.param_id = ISP_3DNR;
			Isp_init_param.p_Isp_3dnr.length = sizeof(Isp_init_param.p_Isp_3dnr);
			send_buf_len = Isp_init_param.p_Isp_3dnr.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_3dnr , send_buf_len);
			break;
		case DIALOG_WDR:
			if (!m_TextDlg.Get_packetData_WDR(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_WDR;
			Isp_init_param.p_Isp_wdr.param_id = ISP_WDR;
			Isp_init_param.p_Isp_wdr.length = sizeof(Isp_init_param.p_Isp_wdr);
			send_buf_len = Isp_init_param.p_Isp_wdr.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_wdr , send_buf_len);
			break;
		case DIALOG_FCS:
			if (!m_TextDlg.Get_packetData_FCS(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_FCS;
			Isp_init_param.p_Isp_fcs.param_id = ISP_FCS;
			Isp_init_param.p_Isp_fcs.length = sizeof(Isp_init_param.p_Isp_fcs);
			send_buf_len = Isp_init_param.p_Isp_fcs.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_fcs , send_buf_len);
			break;
		case DIALOG_AF:
			if (!m_TextDlg.Get_packetData_AF(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_AF;
			Isp_init_param.p_Isp_af.param_id = ISP_AF;
			Isp_init_param.p_Isp_af.length = sizeof(Isp_init_param.p_Isp_af);
			send_buf_len = Isp_init_param.p_Isp_af.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_af , send_buf_len);
			break;
		
		case DIALOG_ZONE_WEIGHT:
			if (!m_TextDlg.Get_packetData_WEIGHT(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_ZONE_WEIGHT;
			Isp_init_param.p_Isp_weight.param_id = ISP_ZONE_WEIGHT;
			Isp_init_param.p_Isp_weight.length = sizeof(Isp_init_param.p_Isp_weight);
			send_buf_len = Isp_init_param.p_Isp_weight.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_weight , send_buf_len);
			break;
		case DIALOG_EXP:
			if (!m_TextDlg.Get_packetData_EXP(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_EXP;
			Isp_init_param.p_Isp_exp.param_id = ISP_EXP;
			Isp_init_param.p_Isp_exp.length = sizeof(Isp_init_param.p_Isp_exp);
			send_buf_len = Isp_init_param.p_Isp_exp.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_exp , send_buf_len);
			break;
		case DIALOG_YUVEFFECT:
			if (!m_TextDlg.Get_packetData_YUVEFFECT(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_YUVEFFECT;
			Isp_init_param.p_Isp_effect.param_id = ISP_YUVEFFECT;
			Isp_init_param.p_Isp_effect.length = sizeof(Isp_init_param.p_Isp_effect);
			send_buf_len = Isp_init_param.p_Isp_effect.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_effect , send_buf_len);
			break;
		case DIALOG_SATURATION:
			if (!m_TextDlg.Get_packetData_SATURATION(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_SATURATION;
			Isp_init_param.p_Isp_saturation.param_id = ISP_SATURATION;
			Isp_init_param.p_Isp_saturation.length = sizeof(Isp_init_param.p_Isp_saturation);
			send_buf_len = Isp_init_param.p_Isp_saturation.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_saturation , send_buf_len);
			break;
		case DIALOG_MISC:
			if (!m_TextDlg.Get_packetData_MISC(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_MISC;
			Isp_init_param.p_Isp_misc.param_id = ISP_MISC;
			Isp_init_param.p_Isp_misc.length = sizeof(Isp_init_param.p_Isp_misc);
			send_buf_len = Isp_init_param.p_Isp_misc.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_misc , send_buf_len);
			break;
		case DIALOG_CONTRAST:
			if (!m_TextDlg.Get_packetData_CONSTRAST(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_CONTRAST;
			Isp_init_param.p_Isp_contrast.param_id = ISP_CONTRAST;
			Isp_init_param.p_Isp_contrast.length = sizeof(Isp_init_param.p_Isp_contrast);
			send_buf_len = Isp_init_param.p_Isp_contrast.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_contrast , send_buf_len);
			break;
		case DIALOG_RGB2YUV:
			if (!m_TextDlg.Get_packetData_RGBTOYUV(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_RGB2YUV;
			Isp_init_param.p_Isp_rgb2yuv.param_id = ISP_RGB2YUV;
			Isp_init_param.p_Isp_rgb2yuv.length = sizeof(Isp_init_param.p_Isp_rgb2yuv);
			send_buf_len = Isp_init_param.p_Isp_rgb2yuv.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_rgb2yuv , send_buf_len);
			break;
		case DIALOG_SYS:
			if (!m_TextDlg.Get_text_sensor_info())
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_REGISTER;
			send_buf_len = ISP_PARM_CNT_SIZE + m_TextDlg.m_sensor_num * 6;
			memcpy(send_buf, &m_TextDlg.m_sensor_num , ISP_PARM_CNT_SIZE);

			idex = ISP_PARM_CNT_SIZE;
			for (i = 0; i < m_TextDlg.m_sensor_num; i++)
			{
				send_buf[idex] = 2;
				idex = idex + 1;
				memcpy(&send_buf[idex], &Isp_init_param.p_Isp_sensor.p_sensor[i].sensor_addr , sizeof(T_U16));
				idex = idex + 2;
				send_buf[idex] = 2;
				idex = idex + 1;
				memcpy(&send_buf[idex], &Isp_init_param.p_Isp_sensor.p_sensor[i].sensor_value , sizeof(T_U16));
				idex = idex + 2;
			}
			break;
		case DIALOG_STAT:
			break;

		case DIALOG_Y_GAMMA:
			if (!m_TextDlg.Get_packetData_Y_GAMMA(buf, buf_len))
			{
				free(send_buf);
				free(buf);
				return;
			}
			addr_id = ISP_Y_GAMMA;
			Isp_init_param.p_Isp_y_gamma.param_id = ISP_Y_GAMMA;
			Isp_init_param.p_Isp_y_gamma.length = sizeof(Isp_init_param.p_Isp_y_gamma);
			send_buf_len = Isp_init_param.p_Isp_y_gamma.length;
			memcpy(send_buf, &Isp_init_param.p_Isp_y_gamma , send_buf_len);
			break;
		default:
			AfxMessageBox("此模块不支持文本输入功能", MB_OK);
			return;
	}
	
	ret = m_NetCtrl.SendCommand(addr_id, CMD_SET, send_buf, send_buf_len);
	if (ret < 0) 
	{
		fprintf(stderr, "WARN! send Gamma page is info error! ret = %d\n", ret);	
	}
	free(send_buf);
	free(buf);
}


void CISPCTRL_TOOLDlg::SendIspParam(BOOL bNeedJudge, int nPageID, int nFlag)
{
	if (!m_NetCtrl.IsConnected()) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法发送ISP参数!\n", 0, 0);
		return;
	}

	char aPageInfo[PAGE_INFO_MAX_LEN] = {0};
	int nLen = PAGE_INFO_MAX_LEN, ret = 0;

	if ((bNeedJudge && (nPageID == DIALOG_BB)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_BBDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("黑平衡中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_BB, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send BB page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}	
	
	if (bNeedJudge && (nPageID == DIALOG_SYS))
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_SysDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("sensor中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_REGISTER, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send sensor page info error! ret = %d\n", ret);	
		}

		T_U16 addr = 0;
		T_U16 value = 0;

		memcpy(&addr, aPageInfo+ISP_PARM_CNT_SIZE+1, 2);
		memcpy(&value, aPageInfo+ISP_PARM_CNT_SIZE+1+2+1, 2);

		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_LSC)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_LscDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("镜头校正中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_LSC, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send LSC page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_GB)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_GBDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("绿平衡中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_GB, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send GB page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_HUE)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		
		if (m_HueDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("绿平衡中设置的参数错误，不发送此设置到服务器端。");
		}
		
		ret = m_NetCtrl.SendCommand(ISP_HUE, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send GB page info error! ret = %d\n", ret);	
		}
		
		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_FCS)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_FcsDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("FCS中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_FCS, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send FCS page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}


	if ((bNeedJudge && (nPageID == DIALOG_RAWLUT || nPageID == DIALOG_LUT_RGB)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (nPageID == DIALOG_RAWLUT || !bNeedJudge)
		{
			if (m_RawlutDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
			{
				AfxMessageBox("Raw lut中设置的参数错误，不发送此设置到服务器端。");
			}
		}
		else
		{
			if (m_RawlutDlg.m_RgbDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
			{
				AfxMessageBox("Raw lut中设置的参数错误，不发送此设置到服务器端。");
			}
		}
		

		ret = m_NetCtrl.SendCommand(ISP_RAW_LUT, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send Raw lut page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);

	}

	if ((bNeedJudge && (nPageID == DIALOG_GAMMA || nPageID == DIALOG_GAMMA_RGB)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (nPageID == DIALOG_GAMMA || !bNeedJudge)
		{
			if (m_GammaDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
			{
				AfxMessageBox("Gamma中设置的参数错误，不发送此设置到服务器端。");
			}
		}
		else
		{
			if (m_GammaDlg.m_RgbDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
			{
				AfxMessageBox("Gamma中设置的参数错误，不发送此设置到服务器端。");
			}
		}
		

		ret = m_NetCtrl.SendCommand(ISP_GAMMA, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send Gamma page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_Y_GAMMA)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_Y_GammaDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("Y Gamma中设置的参数错误，不发送此设置到服务器端。");
		}
		
		ret = m_NetCtrl.SendCommand(ISP_Y_GAMMA, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send Y Gamma page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}

	
	if ((bNeedJudge && (nPageID == DIALOG_WDR)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_WdrDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("Wdr中设置的参数错误，不发送此设置到服务器端。");
		}

		if (Get_WB_EX_PageInfoSt(aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("WB_ex中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_WDR, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send Wdr page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_DEMOSAIC)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_DemosaicDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("Demo中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_DEMO, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send Demo page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_DENOISE)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_DenoiseDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("Nr中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_NR, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send Nr page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_3DNR)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_3DNRDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("3DNR中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_3DNR, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send 3DNR page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}
/*
	if ((bNeedJudge && (nPageID == DIALOG_EDGE)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_EdgeDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("EDGE中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_EDGE, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send EDGE page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}*/

	if ((bNeedJudge && (nPageID == DIALOG_SATURATION)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_SaturationDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("SATURATION中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_SATURATION, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send SATURATION page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_RGB2YUV)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_Rgb2YuvDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("RGB2YUV中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_RGB2YUV, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send RGB2YUV page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_YUVEFFECT)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		if (m_YuvEffectDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("Yuv Effect中设置的参数错误，不发送此设置到服务器端。");
		}

		ret = m_NetCtrl.SendCommand(ISP_YUVEFFECT, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send YUV Effect page info error! ret = %d\n", ret);	
		}

		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_AF)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		
		if (m_AFDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("AF中设置的参数错误，不发送此设置到服务器端。");
		}
		
		ret = m_NetCtrl.SendCommand(ISP_AF, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send AF page info error! ret = %d\n", ret);	
		}
		
		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_EXP)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		
		if (m_ExpDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("Exposure中设置的参数错误，不发送此设置到服务器端。");
		}
		
		ret = m_NetCtrl.SendCommand(ISP_EXP, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send Exposure page info error! ret = %d\n", ret);	
		}
		
		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_ZONE_WEIGHT)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		
		if (m_Zone_WeightDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("ZONE WEIGHT中设置的参数错误，不发送此设置到服务器端。");
		}
		
		ret = m_NetCtrl.SendCommand(ISP_ZONE_WEIGHT, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send ZONE WEIGHT page info error! ret = %d\n", ret);	
		}
		
		//Sleep(COMMAND_SLEEP_MAX);
	}


	if ((bNeedJudge && (nPageID == DIALOG_CCM)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		
		if (m_CCMDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("CCM中设置的参数错误，不发送此设置到服务器端。");
		}
		
		ret = m_NetCtrl.SendCommand(ISP_CCM, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send CCM page info error! ret = %d\n", ret);	
		}
		
		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_WB)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		
		if (m_WbDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("WB中设置的参数错误，不发送此设置到服务器端。");
		}
		
		ret = m_NetCtrl.SendCommand(ISP_WB, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send WB page info error! ret = %d\n", ret);	
		}
		
		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_MISC)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		
		if (m_MiscDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("杂项中设置的参数错误，不发送此设置到服务器端。");
		}
		
		ret = m_NetCtrl.SendCommand(ISP_MISC, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send MISC page info error! ret = %d\n", ret);	
		}
		
		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_SHARP)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		
		if (m_SharpDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("SHARP中设置的参数错误，不发送此设置到服务器端。");
		}
		
		ret = m_NetCtrl.SendCommand(ISP_SHARP, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send SHARP page info error! ret = %d\n", ret);	
		}
		
		//Sleep(COMMAND_SLEEP_MAX);
	}


	if ((bNeedJudge && (nPageID == DIALOG_CONTRAST)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		
		if (m_ContrastDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("对比度中设置的参数错误，不发送此设置到服务器端。");
		}
		
		ret = m_NetCtrl.SendCommand(ISP_CONTRAST, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send CONTRAST page info error! ret = %d\n", ret);	
		}
		
		//Sleep(COMMAND_SLEEP_MAX);
	}

	if ((bNeedJudge && (nPageID == DIALOG_DPC)) || !bNeedJudge)
	{
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		
		if (m_DpcDlg.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) 
		{
			AfxMessageBox("坏点校正中设置的参数错误，不发送此设置到服务器端。");
		}
		
		ret = m_NetCtrl.SendCommand(ISP_DPC, CMD_SET, aPageInfo, nLen);
		if (ret < 0) 
		{
			fprintf(stderr, "WARN! send DPC page info error! ret = %d\n", ret);	
		}
		
		//Sleep(COMMAND_SLEEP_MAX);
	}
}

LRESULT CISPCTRL_TOOLDlg::OnPageEnableChangeMsg(WPARAM wParam, LPARAM lParam)
{
	int nPageID = GETMESSAGEID(lParam);
	BOOL bEnable = GETMESSAGEINFO(lParam);
	
	if (nPageID == DIALOG_SYS) 
	{
		switch (wParam)
		{
		case DIALOG_RAWLUT:
			//m_RawlutDlg.SetGammaEnable(bEnable);
			break;

		case DIALOG_GAMMA:
			//m_GammaDlg.SetGammaEnable(bEnable);
			break;


		default:
			fprintf(stderr, "OnPageEnableChangeMsg Page id unknown!\n");
			return -1;
			break;
		}
	}
	else 
	{
		if (m_SysDlg.SetEnable(wParam, bEnable) < 0)
		fprintf(stderr, "Page Message from function page can't set to Enable page!\n");
	}

	return 0;
}

LRESULT CISPCTRL_TOOLDlg::OnPageSubmissionMsg_ForText(WPARAM wParam, LPARAM lParam)
{
	int nPageID = DialogId;
	UINT msg = GETMESSAGEINFO(lParam);
	
	SendIspParam_ForText(TRUE, nPageID, wParam);
	return 0;
}

LRESULT CISPCTRL_TOOLDlg::OnPageGetInfoMsg_ForText(WPARAM wParam, LPARAM lParam)
{
	BOOL ret = FALSE;
	char *buf = NULL;
	UINT idex = 0, i = 0;
	int nPageID = DialogId;
	UINT msg = GETMESSAGEINFO(lParam);
	short addr_id = ISP_ATTR_TYPE_NUM;
	UINT buf_len = 0;
	
	if (!m_NetCtrl.IsConnected()) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取ISP参数!\n", 0, 0);
		return -1;
		
	}
	
	switch (nPageID)
	{
	case DIALOG_GAMMA:
	case DIALOG_GAMMA_RGB:
		addr_id = ISP_GAMMA;
		break;
	case DIALOG_BB:
		addr_id = ISP_BB;
		break;
	case DIALOG_LSC:
		addr_id = ISP_LSC;
		break;
	case DIALOG_CCM:
		addr_id = ISP_CCM;
		break;
	case DIALOG_WB:
		addr_id = ISP_WB;
		break;
	case DIALOG_DPC:
		addr_id = ISP_DPC;
		break;
	case DIALOG_GB:
		addr_id = ISP_GB;
		break;
	case DIALOG_RAWLUT:
		addr_id = ISP_RAW_LUT;
		break;
	case DIALOG_DEMOSAIC:
		addr_id = ISP_DEMO;
		break;
	case DIALOG_DENOISE:
		addr_id = ISP_NR;
		break;
	case DIALOG_SHARP:
		addr_id = ISP_SHARP;
		break;
	/*case DIALOG_EDGE:
		addr_id = ISP_EDGE;
		break;*/
	case DIALOG_3DNR:
		addr_id = ISP_3DNR;
		break;
	case DIALOG_WDR:
		addr_id = ISP_WDR;
		break;
	case DIALOG_FCS:
		addr_id = ISP_FCS;
		break;
	case DIALOG_YUVEFFECT:
		addr_id = ISP_YUVEFFECT;
		break;
	case DIALOG_EXP:
		addr_id = ISP_EXP;
		break;
	case DIALOG_AF:
		addr_id = ISP_AF;
		break;
	case DIALOG_STAT:
		break;
	case DIALOG_SATURATION:
		addr_id = ISP_SATURATION;
		break;
	case DIALOG_CONTRAST:
		addr_id = ISP_CONTRAST;
		break;
	case DIALOG_ZONE_WEIGHT:
		addr_id = ISP_ZONE_WEIGHT;
		break;
	case DIALOG_MISC:
		addr_id = ISP_MISC;
		break;
	case DIALOG_HUE:
		addr_id = ISP_HUE;
		break;
	case DIALOG_Y_GAMMA:
		addr_id = ISP_Y_GAMMA;
		break;
	case DIALOG_RGB2YUV:
		addr_id = ISP_RGB2YUV;
		break;
	case DIALOG_SYS:
		addr_id = ISP_REGISTER;
		if (m_TextDlg.Get_text_sensor_info() == 0)
		{
			AfxMessageBox("获取sensor地址失败!\n", 0, 0);
			return -1;
		}

		//
		buf = (char *)malloc(MAX_BUF_SHARP_LEN *sizeof(T_U8));
		if (buf == NULL)
		{
			AfxMessageBox("分配内存失败，请检查!\n", 0, 0);
			return -1;
		}

		buf_len = ISP_PARM_CNT_SIZE + m_TextDlg.m_sensor_num * 3;
		memcpy(buf, &m_TextDlg.m_sensor_num , ISP_PARM_CNT_SIZE);
		idex = ISP_PARM_CNT_SIZE;
		for (i = 0; i < m_TextDlg.m_sensor_num; i++)
		{
			buf[idex] = 2;
			idex = idex + 1;
			memcpy(&buf[idex], &Isp_init_param.p_Isp_sensor.p_sensor[i].sensor_addr , sizeof(T_U16));
			idex = idex + 2;
		}

		break;
	default:
		fprintf(stderr, "OnPageGetInfoMsg Page ID unknown!\n");
		break;
	}
	
	if (addr_id == ISP_REGISTER)
	{
		ret = m_NetCtrl.SendCommand(addr_id, CMD_GET_TXT, buf, buf_len);

		if (buf != NULL)
		{
			free(buf);
		}
		
	}
	else
	{
		ret = m_NetCtrl.SendCommand(addr_id, CMD_GET_TXT, NULL, 0);
	}

	
	return ret;
}

LRESULT CISPCTRL_TOOLDlg::OnPageSubmissionMsg(WPARAM wParam, LPARAM lParam)
{
	int nPageID = GETMESSAGEID(lParam);
	UINT msg = GETMESSAGEINFO(lParam);

	SendIspParam(TRUE, nPageID, wParam);
	return 0;
}


LRESULT CISPCTRL_TOOLDlg::OnPageGetInfoMsg(WPARAM wParam, LPARAM lParam)
{
	int nPageID = GETMESSAGEID(lParam);
	UINT msg = GETMESSAGEINFO(lParam);
	short addr_id = ISP_ATTR_TYPE_NUM;
	int ret = 0;

	if (!m_NetCtrl.IsConnected() && (nPageID < DIALOG_3DSTAT)) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取ISP参数!\n", 0, 0);
		return -1;
		
	}

	switch (nPageID)
	{
	case DIALOG_BB:
		addr_id = ISP_BB;
		break;

	case DIALOG_LSC:
		addr_id = ISP_LSC;
		break;

	case DIALOG_GB:
		addr_id = ISP_GB;
		break;

	case DIALOG_RAWLUT:
	case DIALOG_LUT_RGB:
		addr_id = ISP_RAW_LUT;
		break;
		
	case DIALOG_GAMMA:
	case DIALOG_GAMMA_RGB:
		addr_id = ISP_GAMMA;
		break;

	case DIALOG_WDR:
		addr_id = ISP_WDR;
		break;
	case DIALOG_WB:
		addr_id = ISP_WB;
		break;

	case DIALOG_DEMOSAIC:
		addr_id = ISP_DEMO;
		break;

	case DIALOG_DENOISE:
		addr_id = ISP_NR;
		break;

	case DIALOG_3DNR:
		addr_id = ISP_3DNR;
		break;

	case DIALOG_HUE:
		addr_id = ISP_HUE;
		break;
		
	case DIALOG_Y_GAMMA:
		addr_id = ISP_Y_GAMMA;
		break;
	/*case DIALOG_EDGE:
		addr_id = ISP_EDGE;
		break;*/

	case DIALOG_FCS:
		addr_id = ISP_FCS;
		break;

	case DIALOG_SYS:
		{
			char addr[16] = {0};
			int nLen = 16;

			addr_id = ISP_REGISTER;

			if (m_SysDlg.GetAddrInfo(addr, nLen) < 0) 
			{
				AfxMessageBox("sensor中设置的addr错误，不发送此设置到服务器端。");
			}

			ret = m_NetCtrl.SendCommand(ISP_REGISTER, CMD_GET, addr, nLen);
			if (ret < 0) 
			{
				fprintf(stderr, "WARN! send sensor page info error! ret = %d\n", ret);	
			}

			return ret;	
		}
			
		break;

	case DIALOG_EXP:
		addr_id = ISP_EXP;
		break;

	case DIALOG_ZONE_WEIGHT:
		addr_id = ISP_ZONE_WEIGHT;
		break;

	case DIALOG_SATURATION:
		addr_id = ISP_SATURATION;
		break;

	case DIALOG_RGB2YUV:
		addr_id = ISP_RGB2YUV;
		break;

	case DIALOG_YUVEFFECT:
		addr_id = ISP_YUVEFFECT;
		break;

	case DIALOG_AF:
		addr_id = ISP_AF;
		break;
	case DIALOG_CCM:
		addr_id = ISP_CCM;
		break;

	case DIALOG_MISC:
		addr_id = ISP_MISC;
		break;

	case DIALOG_SHARP:
		addr_id = ISP_SHARP;
		break;

	case DIALOG_CONTRAST:
		addr_id = ISP_CONTRAST;
		break;

	case DIALOG_DPC:
		addr_id = ISP_DPC;
		break;

	case DIALOG_3DSTAT:
		addr_id = ISP_3DSTAT;
		break;

	case DIALOG_AFSTAT:
		addr_id = ISP_AFSTAT;
		break;

	case DIALOG_AESTAT:
		addr_id = ISP_AESTAT;
		break;

	case DIALOG_AWBSTAT:
		addr_id = ISP_AWBSTAT;
		break;
	
	default:
		fprintf(stderr, "OnPageGetInfoMsg Page ID unknown!\n");
		break;
	}

	ret = m_NetCtrl.SendCommand(addr_id, CMD_GET, NULL, 0);

	
	return ret;
}

void CISPCTRL_TOOLDlg::OnPageSaveMsg_ForText(WPARAM wParam, LPARAM lParam)
{
	int nPageID = DialogId;
	UINT msg = GETMESSAGEINFO(lParam);
	char *buf = NULL;
	UINT buf_len = 0;
	char *savebuf = NULL;
	UINT savelen = 0;


	buf = (char *)malloc(MAX_BUF_SHARP_LEN);
	if (buf == NULL)
	{
		AfxMessageBox("内存分配失败!\n", 0, 0);
		return;
	}

	buf_len = m_TextDlg.Get_text_info(buf);
	if (buf_len == 0)
	{
		free(buf);
		return;
	}


	switch (nPageID)
	{
		case DIALOG_BB:
			if (!m_TextDlg.Get_packetData_BB(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_blc.param_id = ISP_BB;
			Isp_init_param.p_Isp_blc.length = sizeof(Isp_init_param.p_Isp_blc);
			savebuf = (char*)&Isp_init_param.p_Isp_blc;
			savelen = Isp_init_param.p_Isp_blc.length;
			break;
		case DIALOG_LSC:
			if (!m_TextDlg.Get_packetData_LSC(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_lsc.param_id = ISP_LSC;
			Isp_init_param.p_Isp_lsc.length = sizeof(Isp_init_param.p_Isp_lsc);
			savebuf = (char*)&Isp_init_param.p_Isp_lsc;
			savelen = Isp_init_param.p_Isp_lsc.length;
			break;
		case DIALOG_CCM:
			if (!m_TextDlg.Get_packetData_CCM(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_ccm.param_id = ISP_CCM;
			Isp_init_param.p_Isp_ccm.length = sizeof(Isp_init_param.p_Isp_ccm);
			savebuf = (char*)&Isp_init_param.p_Isp_ccm;
			savelen = Isp_init_param.p_Isp_ccm.length;
			break;
		case DIALOG_HUE:
			if (!m_TextDlg.Get_packetData_hue(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_hue.param_id = ISP_HUE;
			Isp_init_param.p_Isp_hue.length = sizeof(Isp_init_param.p_Isp_hue);
			savebuf = (char*)&Isp_init_param.p_Isp_hue;
			savelen = Isp_init_param.p_Isp_hue.length;
			break;
		case DIALOG_WB:
			if (!m_TextDlg.Get_packetData_WB(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_wb.param_id = ISP_WB;
			Isp_init_param.p_Isp_wb.length = sizeof(Isp_init_param.p_Isp_wb);
			savebuf = (char*)&Isp_init_param.p_Isp_wb;
			savelen = Isp_init_param.p_Isp_wb.length;
			break;
		case DIALOG_DPC:
			if (!m_TextDlg.Get_packetData_DPC(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_dpc.param_id = ISP_DPC;
			Isp_init_param.p_Isp_dpc.length = sizeof(Isp_init_param.p_Isp_dpc);
			savebuf = (char*)&Isp_init_param.p_Isp_dpc;
			savelen = Isp_init_param.p_Isp_dpc.length;
			break;
		case DIALOG_GB:
			if (!m_TextDlg.Get_packetData_GB(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_gb.param_id = ISP_GB;
			Isp_init_param.p_Isp_gb.length = sizeof(Isp_init_param.p_Isp_gb);
			savebuf = (char*)&Isp_init_param.p_Isp_gb;
			savelen = Isp_init_param.p_Isp_gb.length;
			break;
		case DIALOG_RAWLUT:
			if (!m_TextDlg.Get_packetData_RAW_LUT(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_raw_lut.param_id = ISP_RAW_LUT;
			Isp_init_param.p_Isp_raw_lut.length = sizeof(Isp_init_param.p_Isp_raw_lut);
			savebuf = (char*)&Isp_init_param.p_Isp_raw_lut;
			savelen = Isp_init_param.p_Isp_raw_lut.length;
			break;
		case DIALOG_DEMOSAIC:
			if (!m_TextDlg.Get_packetData_DEMO(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_demo.param_id = ISP_DEMO;
			Isp_init_param.p_Isp_demo.length = sizeof(Isp_init_param.p_Isp_demo);
			savebuf = (char*)&Isp_init_param.p_Isp_demo;
			savelen = Isp_init_param.p_Isp_demo.length;
			break;
		case DIALOG_GAMMA:
			if (!m_TextDlg.Get_packetData_GAMMA(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_gamma.param_id = ISP_GAMMA;
			Isp_init_param.p_Isp_gamma.length = sizeof(Isp_init_param.p_Isp_gamma);
			savebuf = (char*)&Isp_init_param.p_Isp_gamma;
			savelen = Isp_init_param.p_Isp_gamma.length;
			break;
		case DIALOG_Y_GAMMA:
			if (!m_TextDlg.Get_packetData_Y_GAMMA(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_y_gamma.param_id = ISP_Y_GAMMA;
			Isp_init_param.p_Isp_y_gamma.length = sizeof(Isp_init_param.p_Isp_y_gamma);
			savebuf = (char*)&Isp_init_param.p_Isp_y_gamma;
			savelen = Isp_init_param.p_Isp_y_gamma.length;
			break;
		case DIALOG_GAMMA_RGB:
			break;
		case DIALOG_DENOISE:
			if (!m_TextDlg.Get_packetData_NR(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_nr.param_id = ISP_NR;
			Isp_init_param.p_Isp_nr.length = sizeof(Isp_init_param.p_Isp_nr);
			savebuf = (char*)&Isp_init_param.p_Isp_nr;
			savelen = Isp_init_param.p_Isp_nr.length;
			break;
		case DIALOG_SHARP:
			if (!m_TextDlg.Get_packetData_SHARP(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_sharp.param_id = ISP_SHARP;
			Isp_init_param.p_Isp_sharp.length = sizeof(Isp_init_param.p_Isp_sharp);
			savebuf = (char*)&Isp_init_param.p_Isp_sharp;
			savelen = Isp_init_param.p_Isp_sharp.length;
			break;
		/*case DIALOG_EDGE:
			if (!m_TextDlg.Get_packetData_EDGE(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_edge.param_id = ISP_EDGE;
			Isp_init_param.p_Isp_edge.length = sizeof(Isp_init_param.p_Isp_edge);
			savebuf = (char*)&Isp_init_param.p_Isp_edge;
			savelen = Isp_init_param.p_Isp_edge.length;
			break;*/
		case DIALOG_3DNR:
			if (!m_TextDlg.Get_packetData_3DNR(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_3dnr.param_id = ISP_3DNR;
			Isp_init_param.p_Isp_3dnr.length = sizeof(Isp_init_param.p_Isp_3dnr);
			savebuf = (char*)&Isp_init_param.p_Isp_3dnr;
			savelen = Isp_init_param.p_Isp_3dnr.length;
			break;
		case DIALOG_WDR:
			if (!m_TextDlg.Get_packetData_WDR(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_wdr.param_id = ISP_WDR;
			Isp_init_param.p_Isp_wdr.length = sizeof(Isp_init_param.p_Isp_wdr);
			savebuf = (char*)&Isp_init_param.p_Isp_wdr;
			savelen = Isp_init_param.p_Isp_wdr.length;
			break;
		case DIALOG_FCS:
			if (!m_TextDlg.Get_packetData_FCS(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_fcs.param_id = ISP_FCS;
			Isp_init_param.p_Isp_fcs.length = sizeof(Isp_init_param.p_Isp_fcs);
			savebuf = (char*)&Isp_init_param.p_Isp_fcs;
			savelen = Isp_init_param.p_Isp_fcs.length;
			break;
		case DIALOG_AF:
			if (!m_TextDlg.Get_packetData_AF(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_af.param_id = ISP_AF;
			Isp_init_param.p_Isp_af.length = sizeof(Isp_init_param.p_Isp_af);
			savebuf = (char*)&Isp_init_param.p_Isp_af;
			savelen = Isp_init_param.p_Isp_af.length;
			break;
		
		case DIALOG_ZONE_WEIGHT:
			if (!m_TextDlg.Get_packetData_WEIGHT(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_weight.param_id = ISP_ZONE_WEIGHT;
			Isp_init_param.p_Isp_weight.length = sizeof(Isp_init_param.p_Isp_weight);
			savebuf = (char*)&Isp_init_param.p_Isp_weight;
			savelen = Isp_init_param.p_Isp_weight.length;
			break;
		case DIALOG_EXP:
			if (!m_TextDlg.Get_packetData_EXP(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_exp.param_id = ISP_EXP;
			Isp_init_param.p_Isp_exp.length = sizeof(Isp_init_param.p_Isp_exp);
			savebuf = (char*)&Isp_init_param.p_Isp_exp;
			savelen = Isp_init_param.p_Isp_exp.length;
			break;
		case DIALOG_YUVEFFECT:
			if (!m_TextDlg.Get_packetData_YUVEFFECT(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_effect.param_id = ISP_YUVEFFECT;
			Isp_init_param.p_Isp_effect.length = sizeof(Isp_init_param.p_Isp_effect);
			savebuf = (char*)&Isp_init_param.p_Isp_effect;
			savelen = Isp_init_param.p_Isp_effect.length;
			break;
		case DIALOG_SATURATION:
			if (!m_TextDlg.Get_packetData_SATURATION(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_saturation.param_id = ISP_SATURATION;
			Isp_init_param.p_Isp_saturation.length = sizeof(Isp_init_param.p_Isp_saturation);
			savebuf = (char*)&Isp_init_param.p_Isp_saturation;
			savelen = Isp_init_param.p_Isp_saturation.length;
			break;
		case DIALOG_MISC:
			if (!m_TextDlg.Get_packetData_MISC(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_misc.param_id = ISP_MISC;
			Isp_init_param.p_Isp_misc.length = sizeof(Isp_init_param.p_Isp_misc);
			savebuf = (char*)&Isp_init_param.p_Isp_misc;
			savelen = Isp_init_param.p_Isp_misc.length;
			break;
		case DIALOG_CONTRAST:
			if (!m_TextDlg.Get_packetData_CONSTRAST(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_contrast.param_id = ISP_CONTRAST;
			Isp_init_param.p_Isp_contrast.length = sizeof(Isp_init_param.p_Isp_contrast);
			savebuf = (char*)&Isp_init_param.p_Isp_contrast;
			savelen = Isp_init_param.p_Isp_contrast.length;
			break;
		case DIALOG_RGB2YUV:
			if (!m_TextDlg.Get_packetData_RGBTOYUV(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_rgb2yuv.param_id = ISP_RGB2YUV;
			Isp_init_param.p_Isp_rgb2yuv.length = sizeof(Isp_init_param.p_Isp_rgb2yuv);
			savebuf = (char*)&Isp_init_param.p_Isp_rgb2yuv;
			savelen = Isp_init_param.p_Isp_rgb2yuv.length;
			break;
		case DIALOG_SYS:
			savebuf = buf;
			savelen = buf_len;
			break;
		case DIALOG_STAT:
			break;
		default:
			AfxMessageBox("此模块不支持文本输入功能", MB_OK);
			return;
	}

	FILE *file;
	CFileDialog dlg(FALSE, "*.txt", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"Config File(*.txt)|*.txt|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
	{
		free(buf);
		return;
	}

	file = fopen(dlg.GetPathName(), "w+b");

	if (NULL == file)
	{
		CString str;
		str.Format("打开文件失败！");
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		free(buf);
		return;
	}

	fwrite(savebuf, 1, savelen, file);
	fclose(file);

	free(buf);


	return;
}

void CISPCTRL_TOOLDlg::OnPageReadMsg_ForText(WPARAM wParam, LPARAM lParam)
{
	int nPageID = DialogId;
	UINT msg = GETMESSAGEINFO(lParam);
	FILE *file;
	char buf[CFG_INFO_MAX_LEN] = {0};
	UINT size = 0; 

	CFileDialog dlg(TRUE, "*.txt", NULL, OFN_HIDEREADONLY,
		"Config File(*.txt)|*.txt|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;

	file = fopen(dlg.GetPathName(), "r+b");

	if (NULL == file)
	{
		CString str;
		str.Format("打开文件失败！");
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	char aPageInfo[PAGE_INFO_MAX_LEN] = {0};
	int nLen = 0;

	switch (nPageID)
	{
	case DIALOG_RAWLUT:
		nLen = sizeof(AK_ISP_INIT_RAW_LUT);
		fread(aPageInfo, 1, nLen, file);

		//显示数据
		if (!m_TextDlg.decode_packet_RAW_LUT(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_GAMMA:
		nLen = sizeof(AK_ISP_INIT_GAMMA);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_GAMMA(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;

	case DIALOG_Y_GAMMA:
		nLen = sizeof(AK_ISP_INIT_Y_GAMMA);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_Y_GAMMA(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_WDR:
		nLen = sizeof(AK_ISP_INIT_WDR);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_WDR(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_DEMOSAIC:
		nLen = sizeof(AK_ISP_INIT_DEMO);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_DEMO(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_YUVEFFECT:
		nLen = sizeof(AK_ISP_INIT_EFFECT);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_YUVEFFECT(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_HUE:
		nLen = sizeof(AK_ISP_INIT_HUE);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_hue(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_BB:
		nLen = sizeof(AK_ISP_INIT_BLC);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_BB(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_LSC:
		nLen = sizeof(AK_ISP_INIT_LSC);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_LSC(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_CCM:
		nLen = sizeof(AK_ISP_INIT_CCM);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_CCM(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_WB:
		nLen = sizeof(AK_ISP_INIT_WB);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_WB(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_DENOISE:
		nLen = sizeof(AK_ISP_INIT_NR);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_NR(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_3DNR:
		nLen = sizeof(AK_ISP_INIT_3DNR);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_3DNR(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_GB:
		nLen = sizeof(AK_ISP_INIT_GB);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_GB(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_FCS:
		nLen = sizeof(AK_ISP_INIT_FCS);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_FCS(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	/*case DIALOG_EDGE:
		nLen = sizeof(AK_ISP_INIT_EDGE);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_EDGE(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;*/
	case DIALOG_SHARP:
		nLen = sizeof(AK_ISP_INIT_SHARP);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_SHARP(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_SATURATION:
		nLen = sizeof(AK_ISP_INIT_SATURATION);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_SATURATION(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_CONTRAST:
		nLen = sizeof(AK_ISP_INIT_CONTRAST);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_CONSTRAST(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_DPC:
		nLen = sizeof(AK_ISP_INIT_DPC);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_DPC(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_ZONE_WEIGHT:
		nLen = sizeof(AK_ISP_INIT_WEIGHT);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_WEIGHT(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_AF:
		nLen = sizeof(AK_ISP_INIT_AF);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_AF(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_EXP:
		nLen = sizeof(AK_ISP_INIT_EXP);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_EXP(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_MISC:
		nLen = sizeof(AK_ISP_INIT_MISC);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_MISC(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_RGB2YUV:
		nLen = sizeof(AK_ISP_INIT_RGB2YUV);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		if (!m_TextDlg.decode_packet_RGBTOYUV(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_SYS:		
		fseek(file, 0, SEEK_END);
		nLen = ftell(file);
		fseek(file, 0, SEEK_SET);
		fread(aPageInfo, 1, nLen, file);
		//显示数据
		m_TextDlg.SetDlgItemText(IDC_EDIT_TEXT, aPageInfo);

		OnPageCopyTextToUi(0, 0);

		
		GetSubFileData(buf, &size);
		SaveSubFile(m_SubFileId, buf, size);
		SetSubFileNum();
		
		break;
		
	default:
		fprintf(stderr, "此模块不支持文本输入功能！\n");
		break;
	}

	return;
}

T_U16 Isp_Struct_len[ISP_HUE + 1] = {sizeof(AK_ISP_INIT_BLC),
									sizeof(AK_ISP_INIT_LSC),
									sizeof(AK_ISP_INIT_RAW_LUT),
									sizeof(AK_ISP_INIT_NR),
									sizeof(AK_ISP_INIT_3DNR),
									sizeof(AK_ISP_INIT_GB),
									sizeof(AK_ISP_INIT_DEMO),
									sizeof(AK_ISP_INIT_GAMMA),
									sizeof(AK_ISP_INIT_CCM),
									sizeof(AK_ISP_INIT_FCS),
									sizeof(AK_ISP_INIT_WDR),
									//sizeof(AK_ISP_INIT_EDGE),
									sizeof(AK_ISP_INIT_SHARP),
									sizeof(AK_ISP_INIT_SATURATION),
									sizeof(AK_ISP_INIT_CONTRAST),
									sizeof(AK_ISP_INIT_RGB2YUV),
									sizeof(AK_ISP_INIT_EFFECT),
									sizeof(AK_ISP_INIT_DPC),
									sizeof(AK_ISP_INIT_WEIGHT),
									sizeof(AK_ISP_INIT_AF),
									sizeof(AK_ISP_INIT_WB),
									sizeof(AK_ISP_INIT_EXP),
									sizeof(AK_ISP_INIT_MISC),
									sizeof(AK_ISP_INIT_Y_GAMMA),
									sizeof(AK_ISP_INIT_HUE)
									};


bool CISPCTRL_TOOLDlg::CheckSubFileHeadInfo(CFGFILE_HEADINFO* headinfo)
{
	if (NULL == headinfo)
		return FALSE;

	fprintf(stderr, "CheckSubFileHeadInfo, main version : %d, file version : %s, sensor id : 0x%x, style id : %d, \
		modify time : %d-%d-%d, %02d:%02d:%02d, subFileId:%d!\n", headinfo->main_version, headinfo->file_version, 
		headinfo->sensorId, headinfo->styleId, headinfo->year, headinfo->month, headinfo->day, 
		headinfo->hour, headinfo->minute, headinfo->second, headinfo->subFileId);

	if (headinfo->year < 1900)
		return FALSE;

	if (headinfo->month > 12 || headinfo->month < 1)
		return FALSE;
	if (headinfo->day > 31 || headinfo->day < 1)
		return FALSE;

	if (headinfo->hour > 23)
		return FALSE;

	if (headinfo->minute > 59)
		return FALSE;

	if (headinfo->second > 59)
		return FALSE;

	if (headinfo->subFileId > 4)
		return FALSE;

	if (MAIN_VERSION != headinfo->main_version)
	{
		if (3 == headinfo->main_version)
		{
			fprintf(stderr, "CheckSubFileHeadInfo cfg file is old version, v3!\n");
			AfxMessageBox("配置文件版本旧，请用V3.x.xx版本的工具调试!", MB_OK);
		}

		return FALSE;
	}

	if (m_bOnlySensor)
	{
		m_sensor_id = headinfo->sensorId;
	}
	else
	{
		if ((m_sensor_id != headinfo->sensorId) && (0xffffffff != m_sensor_id))
		{
			AfxMessageBox("配置文件sensor id不符!", MB_OK);
			return FALSE;
		}
	}

	return TRUE;
}

T_BOOL CISPCTRL_TOOLDlg::CheckSubFileData(char* cfgBuf, T_U32 size)
{
	T_U8 i = 0;
	T_U32 total = 0;
	T_U16 moduleId = 0;
	T_U16 length = 0;
	T_U32 offset = 0;
	
	if ((AK_NULL == cfgBuf) || (size <= sizeof(CFGFILE_HEADINFO)))
	{
		fprintf(stderr, "CheckSubFileData cfgBuf null or size is too small, size:%lu!\n", size);
		return AK_FALSE;
	}

	CFGFILE_HEADINFO headinfo = {0};
	memcpy(&headinfo, cfgBuf, sizeof(CFGFILE_HEADINFO));

	if (!CheckSubFileHeadInfo(&headinfo))
	{
		fprintf(stderr, "CheckSubFileHeadInfo failed!\n");
		return AK_FALSE;
	}

	offset = sizeof(CFGFILE_HEADINFO);
	total += sizeof(CFGFILE_HEADINFO);

	for (i=ISP_BB; i<=ISP_HUE; i++)
	{
		memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);
		memcpy(&length, cfgBuf + offset + ISP_MODULE_ID_SIZE, ISP_MODULE_LEN_SIZE);

		//fprintf(stderr, "i: %d, moduleId : %d, length : %d, Structlen : %d!\n", i, moduleId, length, Isp_Struct_len[i]);

		if ((moduleId != i) || (length != Isp_Struct_len[i]))
		{
			fprintf(stderr, "CheckSubFileData data err!\n");
			return AK_FALSE;
		}
	
		offset += Isp_Struct_len[i];
		total += Isp_Struct_len[i];
	}

	//sensor
	memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);

	if (moduleId != ISP_SENSOR)
	{
		fprintf(stderr, "CheckSubFileData sensor id err!\n");
		return AK_FALSE;
	}

	offset += ISP_MODULE_ID_SIZE;
	
	memcpy(&length, cfgBuf + offset, ISP_MODULE_LEN_SIZE);

	total += ISP_MODULE_ID_SIZE + ISP_MODULE_LEN_SIZE + length;

	if (size != total)
	{
		fprintf(stderr, "CheckSubFileData size err!\n");
		return AK_FALSE;
	}


	m_SysDlg.SetSensorId(headinfo.sensorId);
	m_SysDlg.SetStyleId(headinfo.styleId);
	

	if (0 == m_version[0])
	{
		memcpy(m_version, headinfo.file_version, CFG_FILE_VERSION_LEN);
	}

	m_SysDlg.SetVersion((char*)m_version);

	m_SysDlg.SetNotes(headinfo.notes);

	return AK_TRUE;
}

bool CISPCTRL_TOOLDlg::DecodeSubFileData(char *buf, UINT size) 
{
	char *p = NULL;
	int nLen = 0;
	UINT decodeLen = 0;
	AK_ISP_INIT_WDR m_isp_init_wdr_temp;

	p = buf;

	if (!CheckSubFileData(p, size))
	{
		AfxMessageBox("配置文件数据错误!", MB_OK);
		return 0;
	}


	p += sizeof(CFGFILE_HEADINFO);
	decodeLen += sizeof(CFGFILE_HEADINFO);

	
	//BLC
	nLen = sizeof(AK_ISP_INIT_BLC);
	if (!m_bOnlySensor)
		m_BBDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//LSC
	nLen = sizeof(AK_ISP_INIT_LSC);
	if (!m_bOnlySensor)
		m_LscDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//raw lut
	nLen = sizeof(AK_ISP_INIT_RAW_LUT);
	if (!m_bOnlySensor)
		m_RawlutDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//NR
	nLen = sizeof(AK_ISP_INIT_NR);
	if (!m_bOnlySensor)
		m_DenoiseDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//3DNR
	nLen = sizeof(AK_ISP_INIT_3DNR);
	if (!m_bOnlySensor)
		m_3DNRDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//GB
	nLen = sizeof(AK_ISP_INIT_GB);
	if (!m_bOnlySensor)
		m_GBDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//demo
	nLen = sizeof(AK_ISP_INIT_DEMO);
	if (!m_bOnlySensor)
		m_DemosaicDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//gamma
	nLen = sizeof(AK_ISP_INIT_GAMMA);
	if (!m_bOnlySensor)
		m_GammaDlg.SetPageInfoSt(p, nLen);			
	p += nLen;
	decodeLen +=nLen;

	//CCM
	nLen = sizeof(AK_ISP_INIT_CCM);
	if (!m_bOnlySensor)
		m_CCMDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//FCS
	nLen = sizeof(AK_ISP_INIT_FCS);
	if (!m_bOnlySensor)
		m_FcsDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//WDR
	nLen = sizeof(AK_ISP_INIT_WDR);
	if (!m_bOnlySensor)
	{
		m_WdrDlg.SetPageInfoSt(p, nLen);
		memcpy(&m_isp_init_wdr_temp, p, nLen);
	}
	p += nLen;
	decodeLen +=nLen;

	//EDGE
/*	nLen = sizeof(AK_ISP_INIT_EDGE);
	if (!m_bOnlySensor)
		m_EdgeDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;*/

	//sharp
	nLen = sizeof(AK_ISP_INIT_SHARP);
	if (!m_bOnlySensor)
		m_SharpDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//saturation
	nLen = sizeof(AK_ISP_INIT_SATURATION);
	if (!m_bOnlySensor)
		m_SaturationDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//contrast
	nLen = sizeof(AK_ISP_INIT_CONTRAST);
	if (!m_bOnlySensor)
		m_ContrastDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//rgb to yuv
	nLen = sizeof(AK_ISP_INIT_RGB2YUV);
	if (!m_bOnlySensor)
		m_Rgb2YuvDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//yuv effect
	nLen = sizeof(AK_ISP_INIT_EFFECT);
	if (!m_bOnlySensor)
		m_YuvEffectDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//DPC
	nLen = sizeof(AK_ISP_INIT_DPC);
	if (!m_bOnlySensor)
		m_DpcDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//zone weight
	nLen = sizeof(AK_ISP_INIT_WEIGHT);
	if (!m_bOnlySensor)
		m_Zone_WeightDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//AF
	nLen = sizeof(AK_ISP_INIT_AF);
	if (!m_bOnlySensor)
		m_AFDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//WB
	nLen = sizeof(AK_ISP_INIT_WB);
	if (!m_bOnlySensor)
	{	
		m_WbDlg.SetPageInfoSt(p, nLen);
		memcpy(&m_StatDlg.m_ImgDlg.m_CoordinateDlg.m_coor_Isp_wb, p, nLen);
		m_StatDlg.m_ImgDlg.m_CoordinateDlg.m_get_wbinfo_flag = 1;
	}
		
	p += nLen;
	decodeLen +=nLen;

	//EXP
	nLen = sizeof(AK_ISP_INIT_EXP);
	if (!m_bOnlySensor)
		m_ExpDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//misc
	nLen = sizeof(AK_ISP_INIT_MISC);
	if (!m_bOnlySensor)
		m_MiscDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//y gamma
	nLen = sizeof(AK_ISP_INIT_Y_GAMMA);
	if (!m_bOnlySensor)
		m_Y_GammaDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	//hue
	nLen = sizeof(AK_ISP_INIT_HUE);
	if (!m_bOnlySensor)
		m_HueDlg.SetPageInfoSt(p, nLen);
	p += nLen;
	decodeLen +=nLen;

	p += 2;
	decodeLen +=2;

	//sensor
	memcpy(&nLen, p, 2);
	p += 2;
	decodeLen +=2;
	
	m_sensor_num = nLen / sizeof(AK_ISP_SENSOR_ATTR);

	if (NULL != m_Isp_sensor.p_sensor)
		free(m_Isp_sensor.p_sensor);

	m_Isp_sensor.p_sensor = NULL;

	m_Isp_sensor.p_sensor = (AK_ISP_SENSOR_ATTR*)malloc(nLen);
	memcpy(m_Isp_sensor.p_sensor, p, nLen);

	p += nLen;
	decodeLen +=nLen;

	if (DIALOG_SYS == DialogId)
	{
		T_U8 addrlen = 2;
		T_U8 valuelen = 2;
		char aPageInfo[PAGE_INFO_MAX_LEN] = {0};

		char *pbuf = NULL;

		pbuf = aPageInfo;

		memcpy(pbuf, &m_sensor_num, ISP_PARM_CNT_SIZE);
		pbuf += ISP_PARM_CNT_SIZE;

		for (int i=0; i<m_sensor_num; i++)
		{
			memcpy(pbuf, &addrlen, 1);
			pbuf++;
			memcpy(pbuf, &m_Isp_sensor.p_sensor[i].sensor_addr, addrlen);
			pbuf += addrlen;
			memcpy(pbuf, &valuelen, 1);
			pbuf++;
			memcpy(pbuf, &m_Isp_sensor.p_sensor[i].sensor_value, valuelen);
			pbuf += valuelen;
		}

		m_TextDlg.decode_packet_sensor(aPageInfo, nLen + ISP_PARM_CNT_SIZE);
	}

	
	if (m_bOnlySensor)
	{
		m_bOnlySensor = FALSE;
	}

	if (m_gamma_copyUItoTest_flag)
	{
		OnPageCopyUiToText(0, 0);
	}
	

	return 1;
}

void CISPCTRL_TOOLDlg::OnButtonRead() 
{
	// TODO: Add your control notification handler code here
	CFile file;
	CFileException e;
	CFileDialog dlg(TRUE, "*.conf", NULL, OFN_HIDEREADONLY,
		"Config File(*.conf)|*.conf|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	if (!file.Open(dlg.GetPathName(), CFile::modeRead, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	char *buf = NULL;
	UINT size = 0;

	size = file.GetLength();

	buf = (char*)malloc(size);

	if (NULL == buf)
	{
		AfxMessageBox("OnButtonRead buf malloc failed!", MB_OK);
		return;
	}

	file.Read(buf, size);
	file.Close();

	m_bOnlySensor = FALSE;

	DecodeTotalFile(buf, size);

	free(buf);
}

void CISPCTRL_TOOLDlg::GetSubFileHeadInfo(CFGFILE_HEADINFO* headinfo)
{
	if (NULL == headinfo)
	{
		return;
	}

	time_t t = time(0);
	struct tm * ptNow = NULL;
	
	ptNow = localtime(&t);

	headinfo->sensorId = m_SysDlg.GetSensorId();

	headinfo->year = ptNow->tm_year + 1900;
	headinfo->month = ptNow->tm_mon + 1;
	headinfo->day = ptNow->tm_mday;
	headinfo->hour = ptNow->tm_hour;
	headinfo->minute = ptNow->tm_min;
	headinfo->second = ptNow->tm_sec;

	headinfo->styleId = m_SysDlg.GetStyleId();

	headinfo->main_version = MAIN_VERSION;

	m_SysDlg.GetVersion(headinfo->file_version);
	headinfo->subFileId = m_SubFileId;
	memcpy(m_version, headinfo->file_version, CFG_FILE_VERSION_LEN);

	for (int i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			CFGFILE_HEADINFO* tmp = (CFGFILE_HEADINFO*)m_SubFile[i].pdata;
			memcpy(tmp->file_version, m_version, CFG_FILE_VERSION_LEN);
		}
	}

	m_SysDlg.GetNotes(headinfo->notes);
}


int CISPCTRL_TOOLDlg::Get_WB_EX_PageInfoSt(void * pPageInfoSt, int & nStLen)
{
	AK_ISP_INIT_WDR  m_Wdr_temp;
	
	if ((pPageInfoSt == NULL)) return -1;
	
	memcpy(&m_Wdr_temp, pPageInfoSt, sizeof(AK_ISP_INIT_WDR));
	
	
	memcpy(pPageInfoSt, &m_Wdr_temp, sizeof(AK_ISP_INIT_WDR));

	nStLen = sizeof(AK_ISP_INIT_WDR);
	
	return 0;
}

bool CISPCTRL_TOOLDlg::GetSubFileData(char *buf, UINT *size) 
{
	char *p = buf;
	UINT total = 0;
	bool sensor_fromtext = FALSE;

	if (NULL == buf || NULL == size)
	{
		return 0;
	}

	if (0 == m_sensor_num)
	{
		if (!m_TextDlg.Get_text_sensor_info() 
			|| (DIALOG_SYS != DialogId))
		{
			AfxMessageBox("没有sensor参数!", MB_OK);
			return 0;
		}

		sensor_fromtext = TRUE;
	}
	else
	{
		if (m_TextDlg.Get_text_sensor_info()
			&& (DIALOG_SYS == DialogId))
		{
			sensor_fromtext = TRUE;
		}
	}


	CFGFILE_HEADINFO headinfo = {0};

	GetSubFileHeadInfo(&headinfo);

	memcpy(p, &headinfo, sizeof(CFGFILE_HEADINFO));

	p += sizeof(CFGFILE_HEADINFO);
	total += sizeof(CFGFILE_HEADINFO);


	char aPageInfo[PAGE_INFO_MAX_LEN] = {0};
	int nLen = PAGE_INFO_MAX_LEN;
	int nPageID;

	//BLC
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_BBDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//LSC
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_LscDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//raw lut
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_RawlutDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//NR
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_DenoiseDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//3DNR
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_3DNRDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//GB
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_GBDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//DEMO
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_DemosaicDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//gamma
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_GammaDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//CCM
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_CCMDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//FCS
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_FcsDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//WDR
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_WdrDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	Get_WB_EX_PageInfoSt(aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//EDGE
/*	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_EdgeDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;*/

	//sharp
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_SharpDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//saturation
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_SaturationDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//contrast
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_ContrastDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//rgb to yuv
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_Rgb2YuvDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//yuv effect
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_YuvEffectDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//DPC
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_DpcDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//zone weight
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_Zone_WeightDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//AF
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_AFDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//WB
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_WbDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//EXP
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_ExpDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//misc
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_MiscDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;
	
	//y gamma
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_Y_GammaDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//hue
	nLen = PAGE_INFO_MAX_LEN;
	ZeroMemory(aPageInfo, sizeof(aPageInfo));
	m_HueDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
	memcpy(p, aPageInfo, nLen);
	p += nLen;
	total += nLen;

	//sensor
	if (sensor_fromtext)
	{
		nLen = sizeof(Isp_init_param.p_Isp_sensor.p_sensor) * m_TextDlg.m_sensor_num;
		Isp_init_param.p_Isp_sensor.param_id = ISP_SENSOR;
		Isp_init_param.p_Isp_sensor.length = nLen;
		memcpy(p, &Isp_init_param.p_Isp_sensor, ISP_MODULE_HEAD_SIZE);
		p += ISP_MODULE_HEAD_SIZE;
		total += ISP_MODULE_HEAD_SIZE;
		memcpy(p, Isp_init_param.p_Isp_sensor.p_sensor, nLen);
		p += nLen;
		total += nLen;

		m_sensor_num = m_TextDlg.m_sensor_num;
		if (NULL != m_Isp_sensor.p_sensor)
		{
			free(m_Isp_sensor.p_sensor);
			m_Isp_sensor.p_sensor = NULL;
		}

		m_Isp_sensor.p_sensor = (AK_ISP_SENSOR_ATTR*)malloc(nLen);
		memcpy(m_Isp_sensor.p_sensor, Isp_init_param.p_Isp_sensor.p_sensor, nLen);
	}
	else
	{
		nLen = sizeof(m_Isp_sensor.p_sensor) * m_sensor_num;
		m_Isp_sensor.param_id = ISP_SENSOR;
		m_Isp_sensor.length = nLen;
		memcpy(p, &m_Isp_sensor, ISP_MODULE_HEAD_SIZE);
		p += ISP_MODULE_HEAD_SIZE;
		total += ISP_MODULE_HEAD_SIZE;
		memcpy(p, m_Isp_sensor.p_sensor, nLen);
		p += nLen;
		total += nLen;
	}

	*size = total;

	headinfo.subfilelen = total;

	p = buf;

	memcpy(p, &headinfo, sizeof(CFGFILE_HEADINFO));

	return 1;
}


void CISPCTRL_TOOLDlg::OnButtonWrite() 
{
	// TODO: Add your control notification handler code here
	char buf[CFG_INFO_MAX_LEN] = {0};
	UINT size = 0;

	if (!GetSubFileData(buf, &size))
	{
		return;
	}

	SaveSubFile(m_SubFileId, buf, size);

	if (!m_SubFile[0].bvalid)
	{
		AfxMessageBox("缺少day子文件!\n", 0, 0);
		return;
	}

	if (!m_SubFile[1].bvalid)
	{
		AfxMessageBox("缺少night子文件!\n", 0, 0);
		return;
	}

	CFGFILE_HEADINFO headinfo[SUBFILE_NUM_MAX] = {0};

	for (int i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			memcpy(&headinfo[i], m_SubFile[i].pdata, sizeof(CFGFILE_HEADINFO));
		}
	}

	T_U32 id = headinfo[0].sensorId;

	for (i=1; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			if (headinfo[i].sensorId != id)
			{
				AfxMessageBox("子文件的sensor id不一致!\n", 0, 0);
				return;
			}
		}
	}

	T_U8 styleid = headinfo[0].styleId;

	for (i=1; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			if (headinfo[i].styleId != styleid)
			{
				AfxMessageBox("子文件的style id不一致!\n", 0, 0);
				return;
			}
		}
	}

	CFile file;
	CFileException e;
	CFileDialog dlg(FALSE, "*.conf", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"Config File(*.conf)|*.conf|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	if (!file.Open(dlg.GetPathName(), CFile::modeCreate|CFile::modeWrite, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	for (i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			file.Write(m_SubFile[i].pdata, m_SubFile[i].filelen);
		}
	}

	file.Close();
}

void CISPCTRL_TOOLDlg::OnButtonGet() 
{
	// TODO: Add your control notification handler code here

	if (!m_NetCtrl.IsConnected()) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取ISP参数!\n", 0, 0);
		return;
	}

	m_NetCtrl.SendCommand(ISP_CFG_DATA, CMD_GET, NULL, 0);

	m_bOnlySensor = FALSE;
	
}

void CISPCTRL_TOOLDlg::OnButtonSet() 
{
	// TODO: Add your control notification handler code here
	SendIspParam(FALSE, -1, -1);
}

BOOL CISPCTRL_TOOLDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }
	
	if(pMsg->message==WM_MOUSEMOVE)
	{
		m_Mytip.RelayEvent(pMsg);
	}

	if(::IsWindow(m_toolTip.GetSafeHwnd()))  
	{  
		 m_toolTip.RelayEvent(pMsg);  
	}  

	return CDialog::PreTranslateMessage(pMsg);
}

void CISPCTRL_TOOLDlg::OnButtonConfirm() 
{
	// TODO: Add your control notification handler code here
	if (!m_NetCtrl.IsConnected()) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法确认参数!\n", 0, 0);
		return;
	}

	char buf[CFG_INFO_MAX_LEN] = {0};
	UINT size = 0;

	if (!GetSubFileData(buf, &size))
	{
		return;
	}

	SaveSubFile(m_SubFileId, buf, size);

	if (!m_SubFile[0].bvalid)
	{
		AfxMessageBox("缺少day子文件!\n", 0, 0);
		return;
	}

	if (!m_SubFile[1].bvalid)
	{
		AfxMessageBox("缺少night子文件!\n", 0, 0);
		return;
	}

	CFGFILE_HEADINFO headinfo[SUBFILE_NUM_MAX] = {0};

	for (int i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			memcpy(&headinfo[i], m_SubFile[i].pdata, sizeof(CFGFILE_HEADINFO));
		}
	}

	T_U32 id = headinfo[0].sensorId;

	for (i=1; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			if (headinfo[i].sensorId != id)
			{
				AfxMessageBox("子文件的sensor id不一致!\n", 0, 0);
				return;
			}
		}
	}

	T_U8 styleid = headinfo[0].styleId;

	for (i=1; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			if (headinfo[i].styleId != styleid)
			{
				AfxMessageBox("子文件的style id不一致!\n", 0, 0);
				return;
			}
		}
	}


	UINT totallen = 0;


	for (i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			totallen += m_SubFile[i].filelen;
		}
	}

	char *pTotalbuf = NULL;

	pTotalbuf = (char*)malloc(totallen);

	if (NULL == totallen)
	{
		AfxMessageBox("pTotalbuf malloc failed!\n", 0, 0);
		return;
	}

	memset(pTotalbuf, 0, totallen);

	char *p = pTotalbuf;

	for (i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			memcpy(p, m_SubFile[i].pdata, m_SubFile[i].filelen);
			p += m_SubFile[i].filelen;
		}
	}

	m_NetCtrl.SendCommand(ISP_CFG_DATA, CMD_SET, pTotalbuf, totallen);

	free(pTotalbuf);
}

void CISPCTRL_TOOLDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (HEARTBEAT_SEND_TIMER_ID == nIDEvent)
	{
		m_NetCtrl.SendCommand(ISP_HEARTBEAT, CMD_GET, NULL, 0);
	}
	else if (HEARTBEAT_CHECK_TIMER_ID == nIDEvent)
	{
		if (m_NetCtrl.IsConnected())
		{
			if (m_NetCtrl.TcpClientClose())
			{
				m_ConnectState.Format("unconnected");
				m_btnConnect.SetWindowText("Connect");
				UpdateData(FALSE);

				KillTimer(m_heartbeat_timer);
				KillTimer(m_check_timer);

				m_StatDlg.SetConnectState(FALSE);
				m_CCMDlg.SetConnectState(FALSE);
				m_LscDlg.SetConnectState(FALSE);
			}
		}
	}
	else if (MSGBOX_SHOW_TIMER_ID == nIDEvent)
	{
		keybd_event(VK_RETURN,0,0,0);
		keybd_event(VK_RETURN,0,KEYEVENTF_KEYUP,0);

		KillTimer(m_msgboxshow_timer);
	}

	CDialog::OnTimer(nIDEvent);
}


void CISPCTRL_TOOLDlg::OnClearSensorParam(void)
{
	if (NULL != m_Isp_sensor.p_sensor)
		free(m_Isp_sensor.p_sensor);

	m_Isp_sensor.p_sensor = NULL;

	m_sensor_num = 0;
}

void CISPCTRL_TOOLDlg::OnButtonGetall() 
{
	// TODO: Add your control notification handler code here
	if (!m_NetCtrl.IsConnected()) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取ISP参数!\n", 0, 0);
		return;
	}

	for (int i=ISP_BB; i<=ISP_HUE; i++)
	{
		m_NetCtrl.SendCommand(i, CMD_GET, NULL, 0);
	}
}


void CISPCTRL_TOOLDlg::OnPageCopyUiToText(WPARAM wParam, LPARAM lParam)
{
	int nPageID = DialogId;
	char aPageInfo[PAGE_INFO_MAX_LEN] = {0};
	int nLen = PAGE_INFO_MAX_LEN;

	switch (nPageID)
	{
	case DIALOG_RAWLUT:
		m_RawlutDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);

		//显示数据
		if (!m_TextDlg.decode_packet_RAW_LUT(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_GAMMA:
		m_GammaDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_GAMMA(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;

	case DIALOG_Y_GAMMA:
		m_Y_GammaDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_Y_GAMMA(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_Y_GAMMA error\n");
		}
		
		break;
		
	case DIALOG_WDR:
		m_WdrDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_WDR(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_DEMOSAIC:
		m_DemosaicDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_DEMO(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_YUVEFFECT:
		m_YuvEffectDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_YUVEFFECT(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_BB:
		m_BBDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_BB(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_LSC:
		m_LscDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_LSC(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_CCM:
		m_CCMDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_CCM(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_WB:
		m_WbDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_WB(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_HUE:
		m_HueDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_hue(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_DENOISE:
		m_DenoiseDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_NR(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_3DNR:
		m_3DNRDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_3DNR(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_GB:
		m_GBDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_GB(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_FCS:
		m_FcsDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_FCS(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	/*case DIALOG_EDGE:
		m_EdgeDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_EDGE(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;*/
	case DIALOG_SHARP:
		m_SharpDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_SHARP(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_SATURATION:
		m_SaturationDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_SATURATION(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_CONTRAST:
		m_ContrastDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_CONSTRAST(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_DPC:
		m_DpcDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_DPC(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_ZONE_WEIGHT:
		m_Zone_WeightDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_WEIGHT(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_AF:
		m_AFDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_AF(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_EXP:
		m_ExpDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_EXP(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_MISC:
		m_MiscDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_MISC(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
		
	case DIALOG_RGB2YUV:
		m_Rgb2YuvDlg.GetPageInfoSt(nPageID, aPageInfo, nLen);
		//显示数据
		if (!m_TextDlg.decode_packet_RGBTOYUV(aPageInfo, nLen))
		{
			fprintf(stderr, "decode_packet_BB error\n");
		}
		
		break;
	case DIALOG_SYS:
		{
			T_U8 addrlen = 2;
			T_U8 valuelen = 2;
			char sensor[PAGE_INFO_MAX_LEN] = {0};
			char *p =NULL;

			p = sensor;

			memcpy(p, &m_sensor_num, ISP_PARM_CNT_SIZE);
			p += ISP_PARM_CNT_SIZE;

			for (int i=0; i<m_sensor_num; i++)
			{
				memcpy(p, &addrlen, 1);
				p++;
				memcpy(p, &m_Isp_sensor.p_sensor[i].sensor_addr, addrlen);
				p += addrlen;
				memcpy(p, &valuelen, 1);
				p++;
				memcpy(p, &m_Isp_sensor.p_sensor[i].sensor_value, valuelen);
				p += valuelen;
			}

			nLen = m_sensor_num * sizeof(AK_ISP_SENSOR_ATTR);

			m_TextDlg.decode_packet_sensor(sensor, nLen + ISP_PARM_CNT_SIZE);
		}
		break;
		
	default:
		fprintf(stderr, "此模块不支持文本输入功能！\n");
		break;
	}
}

void CISPCTRL_TOOLDlg::OnPageCopyTextToUi(WPARAM wParam, LPARAM lParam)
{
	int nPageID = DialogId;
	UINT msg = GETMESSAGEINFO(lParam);
	char *buf = NULL;
	UINT buf_len = 0;
	char *savebuf = NULL;
	UINT savelen = 0;


	buf = (char *)malloc(MAX_BUF_SHARP_LEN);
	if (buf == NULL)
	{
		AfxMessageBox("内存分配失败!\n", 0, 0);
		return;
	}

	buf_len = m_TextDlg.Get_text_info(buf);
	if (buf_len == 0)
	{
		free(buf);
		return;
	}


	switch (nPageID)
	{
		case DIALOG_BB:
			if (!m_TextDlg.Get_packetData_BB(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_blc.param_id = ISP_BB;
			Isp_init_param.p_Isp_blc.length = sizeof(Isp_init_param.p_Isp_blc);
			savebuf = (char*)&Isp_init_param.p_Isp_blc;
			savelen = Isp_init_param.p_Isp_blc.length;
			m_BBDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_LSC:
			if (!m_TextDlg.Get_packetData_LSC(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_lsc.param_id = ISP_LSC;
			Isp_init_param.p_Isp_lsc.length = sizeof(Isp_init_param.p_Isp_lsc);
			savebuf = (char*)&Isp_init_param.p_Isp_lsc;
			savelen = Isp_init_param.p_Isp_lsc.length;
			m_LscDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_CCM:
			if (!m_TextDlg.Get_packetData_CCM(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_ccm.param_id = ISP_CCM;
			Isp_init_param.p_Isp_ccm.length = sizeof(Isp_init_param.p_Isp_ccm);
			savebuf = (char*)&Isp_init_param.p_Isp_ccm;
			savelen = Isp_init_param.p_Isp_ccm.length;
			m_CCMDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_HUE:
			if (!m_TextDlg.Get_packetData_hue(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_hue.param_id = ISP_HUE;
			Isp_init_param.p_Isp_hue.length = sizeof(Isp_init_param.p_Isp_hue);
			savebuf = (char*)&Isp_init_param.p_Isp_hue;
			savelen = Isp_init_param.p_Isp_hue.length;
			m_HueDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_WB:
			if (!m_TextDlg.Get_packetData_WB(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_wb.param_id = ISP_WB;
			Isp_init_param.p_Isp_wb.length = sizeof(Isp_init_param.p_Isp_wb);
			savebuf = (char*)&Isp_init_param.p_Isp_wb;
			savelen = Isp_init_param.p_Isp_wb.length;
			memcpy(&m_StatDlg.m_ImgDlg.m_CoordinateDlg.m_coor_Isp_wb, savebuf, Isp_init_param.p_Isp_wb.length);
			m_WbDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_DPC:
			if (!m_TextDlg.Get_packetData_DPC(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_dpc.param_id = ISP_DPC;
			Isp_init_param.p_Isp_dpc.length = sizeof(Isp_init_param.p_Isp_dpc);
			savebuf = (char*)&Isp_init_param.p_Isp_dpc;
			savelen = Isp_init_param.p_Isp_dpc.length;
			m_DpcDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_GB:
			if (!m_TextDlg.Get_packetData_GB(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_gb.param_id = ISP_GB;
			Isp_init_param.p_Isp_gb.length = sizeof(Isp_init_param.p_Isp_gb);
			savebuf = (char*)&Isp_init_param.p_Isp_gb;
			savelen = Isp_init_param.p_Isp_gb.length;
			m_GBDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_RAWLUT:
			if (!m_TextDlg.Get_packetData_RAW_LUT(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_raw_lut.param_id = ISP_RAW_LUT;
			Isp_init_param.p_Isp_raw_lut.length = sizeof(Isp_init_param.p_Isp_raw_lut);
			savebuf = (char*)&Isp_init_param.p_Isp_raw_lut;
			savelen = Isp_init_param.p_Isp_raw_lut.length;
			m_RawlutDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_DEMOSAIC:
			if (!m_TextDlg.Get_packetData_DEMO(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_demo.param_id = ISP_DEMO;
			Isp_init_param.p_Isp_demo.length = sizeof(Isp_init_param.p_Isp_demo);
			savebuf = (char*)&Isp_init_param.p_Isp_demo;
			savelen = Isp_init_param.p_Isp_demo.length;
			m_DemosaicDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_GAMMA:
			if (!m_TextDlg.Get_packetData_GAMMA(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_gamma.param_id = ISP_GAMMA;
			Isp_init_param.p_Isp_gamma.length = sizeof(Isp_init_param.p_Isp_gamma);
			savebuf = (char*)&Isp_init_param.p_Isp_gamma;
			savelen = Isp_init_param.p_Isp_gamma.length;
			m_GammaDlg.SetPageInfoSt(savebuf, savelen);
			break;

		case DIALOG_Y_GAMMA:
			if (!m_TextDlg.Get_packetData_Y_GAMMA(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_y_gamma.param_id = ISP_GAMMA;
			Isp_init_param.p_Isp_y_gamma.length = sizeof(Isp_init_param.p_Isp_y_gamma);
			savebuf = (char*)&Isp_init_param.p_Isp_y_gamma;
			savelen = Isp_init_param.p_Isp_y_gamma.length;
			m_Y_GammaDlg.SetPageInfoSt(savebuf, savelen);
			break;

		case DIALOG_DENOISE:
			if (!m_TextDlg.Get_packetData_NR(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_nr.param_id = ISP_NR;
			Isp_init_param.p_Isp_nr.length = sizeof(Isp_init_param.p_Isp_nr);
			savebuf = (char*)&Isp_init_param.p_Isp_nr;
			savelen = Isp_init_param.p_Isp_nr.length;
			m_DenoiseDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_SHARP:
			if (!m_TextDlg.Get_packetData_SHARP(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_sharp.param_id = ISP_SHARP;
			Isp_init_param.p_Isp_sharp.length = sizeof(Isp_init_param.p_Isp_sharp);
			savebuf = (char*)&Isp_init_param.p_Isp_sharp;
			savelen = Isp_init_param.p_Isp_sharp.length;
			m_SharpDlg.SetPageInfoSt(savebuf, savelen);
			break;
		/*case DIALOG_EDGE:
			if (!m_TextDlg.Get_packetData_EDGE(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_edge.param_id = ISP_EDGE;
			Isp_init_param.p_Isp_edge.length = sizeof(Isp_init_param.p_Isp_edge);
			savebuf = (char*)&Isp_init_param.p_Isp_edge;
			savelen = Isp_init_param.p_Isp_edge.length;
			m_EdgeDlg.SetPageInfoSt(savebuf, savelen);
			break;*/
		case DIALOG_3DNR:
			if (!m_TextDlg.Get_packetData_3DNR(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_3dnr.param_id = ISP_3DNR;
			Isp_init_param.p_Isp_3dnr.length = sizeof(Isp_init_param.p_Isp_3dnr);
			savebuf = (char*)&Isp_init_param.p_Isp_3dnr;
			savelen = Isp_init_param.p_Isp_3dnr.length;
			m_3DNRDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_WDR:
			if (!m_TextDlg.Get_packetData_WDR(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_wdr.param_id = ISP_WDR;
			Isp_init_param.p_Isp_wdr.length = sizeof(Isp_init_param.p_Isp_wdr);
			savebuf = (char*)&Isp_init_param.p_Isp_wdr;
			savelen = Isp_init_param.p_Isp_wdr.length;
			m_WdrDlg.SetPageInfoSt(savebuf, savelen);
			//

			break;
		case DIALOG_FCS:
			if (!m_TextDlg.Get_packetData_FCS(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_fcs.param_id = ISP_FCS;
			Isp_init_param.p_Isp_fcs.length = sizeof(Isp_init_param.p_Isp_fcs);
			savebuf = (char*)&Isp_init_param.p_Isp_fcs;
			savelen = Isp_init_param.p_Isp_fcs.length;
			m_FcsDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_AF:
			if (!m_TextDlg.Get_packetData_AF(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_af.param_id = ISP_AF;
			Isp_init_param.p_Isp_af.length = sizeof(Isp_init_param.p_Isp_af);
			savebuf = (char*)&Isp_init_param.p_Isp_af;
			savelen = Isp_init_param.p_Isp_af.length;
			m_AFDlg.SetPageInfoSt(savebuf, savelen);
			break;
		
		case DIALOG_ZONE_WEIGHT:
			if (!m_TextDlg.Get_packetData_WEIGHT(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_weight.param_id = ISP_ZONE_WEIGHT;
			Isp_init_param.p_Isp_weight.length = sizeof(Isp_init_param.p_Isp_weight);
			savebuf = (char*)&Isp_init_param.p_Isp_weight;
			savelen = Isp_init_param.p_Isp_weight.length;
			m_Zone_WeightDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_EXP:
			if (!m_TextDlg.Get_packetData_EXP(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_exp.param_id = ISP_EXP;
			Isp_init_param.p_Isp_exp.length = sizeof(Isp_init_param.p_Isp_exp);
			savebuf = (char*)&Isp_init_param.p_Isp_exp;
			savelen = Isp_init_param.p_Isp_exp.length;
			m_ExpDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_YUVEFFECT:
			if (!m_TextDlg.Get_packetData_YUVEFFECT(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_effect.param_id = ISP_YUVEFFECT;
			Isp_init_param.p_Isp_effect.length = sizeof(Isp_init_param.p_Isp_effect);
			savebuf = (char*)&Isp_init_param.p_Isp_effect;
			savelen = Isp_init_param.p_Isp_effect.length;
			m_YuvEffectDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_SATURATION:
			if (!m_TextDlg.Get_packetData_SATURATION(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_saturation.param_id = ISP_SATURATION;
			Isp_init_param.p_Isp_saturation.length = sizeof(Isp_init_param.p_Isp_saturation);
			savebuf = (char*)&Isp_init_param.p_Isp_saturation;
			savelen = Isp_init_param.p_Isp_saturation.length;
			m_SaturationDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_MISC:
			if (!m_TextDlg.Get_packetData_MISC(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_misc.param_id = ISP_MISC;
			Isp_init_param.p_Isp_misc.length = sizeof(Isp_init_param.p_Isp_misc);
			savebuf = (char*)&Isp_init_param.p_Isp_misc;
			savelen = Isp_init_param.p_Isp_misc.length;
			m_MiscDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_CONTRAST:
			if (!m_TextDlg.Get_packetData_CONSTRAST(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_contrast.param_id = ISP_CONTRAST;
			Isp_init_param.p_Isp_contrast.length = sizeof(Isp_init_param.p_Isp_contrast);
			savebuf = (char*)&Isp_init_param.p_Isp_contrast;
			savelen = Isp_init_param.p_Isp_contrast.length;
			m_ContrastDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_RGB2YUV:
			if (!m_TextDlg.Get_packetData_RGBTOYUV(buf, buf_len))
			{
				free(buf);
				return;
			}
			Isp_init_param.p_Isp_rgb2yuv.param_id = ISP_RGB2YUV;
			Isp_init_param.p_Isp_rgb2yuv.length = sizeof(Isp_init_param.p_Isp_rgb2yuv);
			savebuf = (char*)&Isp_init_param.p_Isp_rgb2yuv;
			savelen = Isp_init_param.p_Isp_rgb2yuv.length;
			m_Rgb2YuvDlg.SetPageInfoSt(savebuf, savelen);
			break;
		case DIALOG_SYS:
			//savebuf = buf;
			//savelen = buf_len;
			if (!m_TextDlg.Get_text_sensor_info())
			{
				free(buf);
				return;
			}

			m_sensor_num = m_TextDlg.m_sensor_num;
			if (NULL != m_Isp_sensor.p_sensor)
			{
				free (m_Isp_sensor.p_sensor);
				m_Isp_sensor.p_sensor = NULL;
			}

			m_Isp_sensor.p_sensor = (AK_ISP_SENSOR_ATTR*)malloc(m_sensor_num * sizeof(AK_ISP_SENSOR_ATTR));
			memcpy(m_Isp_sensor.p_sensor, Isp_init_param.p_Isp_sensor.p_sensor, m_sensor_num * sizeof(AK_ISP_SENSOR_ATTR));

			break;

		default:
			AfxMessageBox("此模块不支持文本输入功能", MB_OK);
			return;
	}

	free (buf);
}

void CISPCTRL_TOOLDlg::OnGetYuvImg(WPARAM wParam, LPARAM lParam)
{
	if (!m_NetCtrl.IsConnected()) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取图像数据!\n", 0, 0);
		return;
	}

	m_NetCtrl.SendCommand(ISP_YUV_IMG, CMD_GET, NULL, 0);
}


void CISPCTRL_TOOLDlg::OnButtonAddfile() 
{
	// TODO: Add your control notification handler code here
	CFile file;
	CFileException e;
	CFileDialog dlg(TRUE, "*.conf", NULL, OFN_HIDEREADONLY,
		"Config File(*.conf)|*.conf|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	if (!file.Open(dlg.GetPathName(), CFile::modeRead, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	char buf[CFG_INFO_MAX_LEN] = {0};
	UINT size = 0;

	size = file.GetLength();

	file.Read(buf, size);
	file.Close();

	m_bOnlySensor = FALSE;

	if (DecodeSubFileData(buf, size))
	{
		SaveSubFile(m_SubFileId, buf, size);
		SetSubFileNum();
	}
}

void CISPCTRL_TOOLDlg::OnSelchangeComboSubfileid() 
{
	// TODO: Add your control notification handler code here
	CString subfileid;
	T_U8 fileid = 0;

	m_SubFileIdCtrl.GetLBText(m_SubFileIdCtrl.GetCurSel(), subfileid);

	if ("1 (night)" == subfileid)
	{
		fileid = 1;
		m_ExpDlg.m_MEDlg.m_title.Format("%s", "Frame Rate Control(night)");
	}
	else if ("2" == subfileid)
	{
		fileid = 2;
		m_ExpDlg.m_MEDlg.m_title.Format("%s", "Frame Rate Control(2)");
	}
	else if ("3" == subfileid)
	{
		fileid = 3;
		m_ExpDlg.m_MEDlg.m_title.Format("%s", "Frame Rate Control(3)");
	}
	else if ("4" == subfileid)
	{
		fileid = 4;
		m_ExpDlg.m_MEDlg.m_title.Format("%s", "Frame Rate Control(4)");
	}
	else
	{
		fileid = 0;
		m_ExpDlg.m_MEDlg.m_title.Format("%s", "Frame Rate Control(day)");
	}

	if (fileid != m_SubFileId && m_SubFile[m_SubFileId].bvalid)
	{
		char buf[CFG_INFO_MAX_LEN] = {0};
		UINT size = 0;

		if (GetSubFileData(buf, &size))
		{
			SaveSubFile(m_SubFileId, buf, size);
		}
	}

	m_SubFileId = fileid;

	if (0 == m_SubFile[m_SubFileId].bvalid)
	{
		AfxMessageBox("没有该子文件，可以根据需要添加!\n", 0, 0);
		CleanAll();
	}
	else
	{
		DecodeSubFileData(m_SubFile[m_SubFileId].pdata, m_SubFile[m_SubFileId].filelen);
	}

}


bool CISPCTRL_TOOLDlg::SaveSubFile(T_U8 subfileid, char *buf, UINT size) 
{
	T_U32 offset = 0;
	T_U8 i = 0;
	
	if (NULL != m_SubFile[subfileid].pdata)
	{
		free (m_SubFile[subfileid].pdata);
		m_SubFile[subfileid].pdata = NULL;
	}

	m_SubFile[subfileid].pdata = (char*)malloc(size);
	if (NULL == m_SubFile[subfileid].pdata)
	{
		AfxMessageBox("m_SubFile[subfileid].pdata malloc failed!\n", 0, 0);
		return 0;
	}

	memcpy(m_SubFile[subfileid].pdata, buf, size);

	offset += sizeof(CFGFILE_HEADINFO);

	for (i=ISP_BB; i<ISP_3DNR; i++)
	{
		offset += Isp_Struct_len[i];
	}

	m_3DNRDlg.CheckData((AK_ISP_INIT_3DNR*)(m_SubFile[subfileid].pdata+offset));

	for (i=ISP_3DNR; i<ISP_WDR; i++)
	{
		offset += Isp_Struct_len[i];
	}

	m_WdrDlg.CheckData((AK_ISP_INIT_WDR*)(m_SubFile[subfileid].pdata+offset));

	for (i=ISP_WDR; i<ISP_SATURATION; i++)
	{
		offset += Isp_Struct_len[i];
	}
	
	m_SaturationDlg.CheckData((AK_ISP_INIT_SATURATION*)(m_SubFile[subfileid].pdata+offset));
	
	m_SubFile[subfileid].bvalid = 1;
	m_SubFile[subfileid].filelen = size;

	return 1;
}


void CISPCTRL_TOOLDlg::SetSubFileNum() 
{
	CWnd *pWnd = NULL;
	CString str;

	T_U8 cnt = 0;

	for (int i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			cnt++;
		}
	}

	m_SubFileNum = cnt;

	pWnd = GetDlgItem(IDC_STATIC_SUBFILECNT);
	str.Format("%d", m_SubFileNum);
	pWnd->SetWindowText(str);
}


T_BOOL CISPCTRL_TOOLDlg::CheckTotalFileData(char* cfgBuf, T_U32 size)
{
	T_U8 i = 0;
	T_U32 total = 0;
	T_U16 moduleId = 0;
	T_U16 length = 0;
	T_U32 offset = 0;
	T_U8 subfileid[SUBFILE_NUM_MAX] = {0};
	T_U32 subfilelen[SUBFILE_NUM_MAX] = {0};
	T_U32 subfileoffset[SUBFILE_NUM_MAX] = {0};
	T_U8 subfilecnt = 0;
	
	if ((AK_NULL == cfgBuf) || (size <= sizeof(CFGFILE_HEADINFO)))
	{
		fprintf(stderr, "CheckTotalFileData cfgBuf null or size is too small, size:%lu!\n", size);
		return AK_FALSE;
	}

	CFGFILE_HEADINFO headinfo = {0};

CHECK_ONE_SUBFILE:
	memcpy(&headinfo, cfgBuf+offset, sizeof(CFGFILE_HEADINFO));

	if (!CheckSubFileHeadInfo(&headinfo))
	{
		fprintf(stderr, "CheckTotalFileData failed!\n");
		return AK_FALSE;
	}

	offset += sizeof(CFGFILE_HEADINFO);
	total += sizeof(CFGFILE_HEADINFO);
	subfilelen[subfilecnt] += sizeof(CFGFILE_HEADINFO);

	for (i=ISP_BB; i<=ISP_HUE; i++)
	{
		memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);
		memcpy(&length, cfgBuf + offset + ISP_MODULE_ID_SIZE, ISP_MODULE_LEN_SIZE);

		//fprintf(stderr, "i: %d, moduleId : %d, length : %d, Structlen : %d!\n", i, moduleId, length, Isp_Struct_len[i]);

		if ((moduleId != i) || (length != Isp_Struct_len[i]))
		{
			fprintf(stderr, "CheckTotalFileData data err!\n");
			return AK_FALSE;
		}
	
		offset += Isp_Struct_len[i];
		total += Isp_Struct_len[i];
		subfilelen[subfilecnt] += Isp_Struct_len[i];
	}

	//sensor
	memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);

	if (moduleId != ISP_SENSOR)
	{
		fprintf(stderr, "CheckTotalFileData sensor id err!\n");
		return AK_FALSE;
	}

	offset += ISP_MODULE_ID_SIZE;
	total += ISP_MODULE_ID_SIZE;
	subfilelen[subfilecnt] += ISP_MODULE_ID_SIZE;
	
	memcpy(&length, cfgBuf + offset, ISP_MODULE_LEN_SIZE);

	total += ISP_MODULE_LEN_SIZE + length;
	offset += ISP_MODULE_LEN_SIZE + length;
	subfilelen[subfilecnt] += ISP_MODULE_LEN_SIZE + length;


	subfileid[subfilecnt] = headinfo.subFileId;
	if (subfilecnt > 0)
	{
		subfileoffset[subfilecnt] = subfileoffset[subfilecnt-1] + subfilelen[subfilecnt-1];
	}

	subfilecnt++;

	if (size > total && subfilecnt < SUBFILE_NUM_MAX)
	{
		goto CHECK_ONE_SUBFILE;
	}

	if (!m_bOnlySensor)
	{
		for (i=0; i<SUBFILE_NUM_MAX; i++)
		{
			if (NULL != m_SubFile[i].pdata)
			{
				free(m_SubFile[i].pdata);
				m_SubFile[i].pdata = NULL;
			}

			m_SubFile[i].bvalid = 0;
			m_SubFile[i].filelen = 0;
		}
		
		for (i=0; i<subfilecnt; i++)
		{
			SaveSubFile(subfileid[i], cfgBuf + subfileoffset[i], subfilelen[i]);
		}

		SetSubFileNum();
	}
	else
	{
		m_bOnlySensor = AK_FALSE;
	}

	return AK_TRUE;
}


bool CISPCTRL_TOOLDlg::DecodeTotalFile(char *buf, UINT size) 
{
	char *p = NULL;
	int nLen = 0;
	UINT decodeLen = 0;

	p = buf;

	CFGFILE_HEADINFO headinfo = {0};
	memcpy(&headinfo, p, sizeof(CFGFILE_HEADINFO));


	if (!CheckTotalFileData(p, size))
	{
		SetMstboxTimer();
		AfxMessageBox("配置文件数据错误!", MB_OK);
		return 0;
	}

	memset(m_version, 0, 6);

	//CleanAll();

	if (m_SubFile[m_SubFileId].bvalid)
	{
		DecodeSubFileData(m_SubFile[m_SubFileId].pdata, m_SubFile[m_SubFileId].filelen);
	}
	
	
	if (m_bOnlySensor)
	{
		m_bOnlySensor = FALSE;
	}

	return 1;
}


void CISPCTRL_TOOLDlg::CleanAll(void) 
{
	m_SysDlg.Clean();
	m_BBDlg.Clean();
	m_LscDlg.Clean();
	m_RawlutDlg.Clean();
	m_DpcDlg.Clean();

	m_DenoiseDlg.Clean();
	m_GBDlg.Clean();
	m_DemosaicDlg.Clean();
	m_WbDlg.Clean();
	m_CCMDlg.Clean();

	m_GammaDlg.Clean();
	m_Rgb2YuvDlg.Clean();
	m_ContrastDlg.Clean();
	m_WdrDlg.Clean();
	m_SaturationDlg.Clean();

	m_3DNRDlg.Clean();
	//m_EdgeDlg.Clean();
	m_SharpDlg.Clean();
	m_FcsDlg.Clean();
	m_YuvEffectDlg.Clean();

	m_ExpDlg.Clean();
	m_AFDlg.Clean();
	m_Zone_WeightDlg.Clean();
	m_MiscDlg.Clean();


	m_sensor_num = 0;

	if (NULL != m_Isp_sensor.p_sensor)
		free(m_Isp_sensor.p_sensor);

	m_Isp_sensor.p_sensor = NULL;

	OnPageCopyUiToText(0, 0);
}

void CISPCTRL_TOOLDlg::OnGet_wb_info(WPARAM wParam, LPARAM lParam)
{
	char * buf = NULL;
	int buf_len = sizeof(AK_ISP_INIT_WB);
	int nPageID = DIALOG_WB;
	UINT i = 0;

	m_StatDlg.m_ImgDlg.m_CoordinateDlg.m_get_wbinfo_flag = 0;

	buf = (char *)malloc(buf_len);
	if (buf == NULL)
	{
		m_StatDlg.m_ImgDlg.m_CoordinateDlg.m_get_wbinfo_flag = -1;
		free(buf);
		AfxMessageBox("内存分配失败!\n", 0, 0);
		return;
	} 
	memset(buf, 0, buf_len);

	if (m_WbDlg.GetPageInfoSt(nPageID, buf, buf_len) < 0) 
	{
		m_StatDlg.m_ImgDlg.m_CoordinateDlg.m_get_wbinfo_flag = -1;
		free(buf);
		AfxMessageBox("WB中设置的参数错误");
		return;
	}
	for (i = 4; i < buf_len; i++)
	{
		if (buf[i] != 0)
		{
			break;	
		}
	}
	
	if (i == buf_len)
	{
		m_StatDlg.m_ImgDlg.m_CoordinateDlg.m_get_wbinfo_flag = -1;
		free(buf);
		AfxMessageBox("WB中设置的参数全是0，请先导入参数");
		return;
	}


	//拷贝数据		
	memcpy(&m_StatDlg.m_ImgDlg.m_CoordinateDlg.m_coor_Isp_wb, buf, buf_len);

	m_StatDlg.m_ImgDlg.m_CoordinateDlg.m_get_wbinfo_flag = 1;

	free(buf);
}

void CISPCTRL_TOOLDlg::OnSet_wb_info(WPARAM wParam, LPARAM lParam)
{
	char * buf = NULL;
	int buf_len = sizeof(AK_ISP_INIT_WB);
	int nPageID = DIALOG_WB;
	
	buf = (char *)malloc(buf_len);
	if (buf == NULL)
	{
		free(buf);
		AfxMessageBox("内存分配失败!\n", 0, 0);
		return;
	}
	memset(buf, 0, buf_len);

	//拷贝数据		
	memcpy(buf, &m_StatDlg.m_ImgDlg.m_CoordinateDlg.m_coor_Isp_wb, buf_len);

	if (m_WbDlg.SetPageInfoSt(buf, buf_len) < 0) 
	{
		free(buf);
		AfxMessageBox("WB中设置的参数错误");
		return;
	}
	
	free(buf);

}

void CISPCTRL_TOOLDlg::OnGet_exp_info(WPARAM wParam, LPARAM lParam)
{
	char * buf = NULL;
	int buf_len = sizeof(AK_ISP_INIT_EXP);
	int nPageID = DIALOG_EXP;
	UINT i = 0;

	buf = (char *)malloc(buf_len);
	if (buf == NULL)
	{
		free(buf);
		AfxMessageBox("内存分配失败!\n", 0, 0);
		return;
	} 
	memset(buf, 0, buf_len);

	if (m_ExpDlg.GetPageInfoSt(nPageID, buf, buf_len) < 0) 
	{
		free(buf);
		AfxMessageBox("EXP中设置的参数错误");
		return;
	}
	for (i = 4; i < buf_len; i++)
	{
		if (buf[i] != 0)
		{
			break;	
		}
	}
	
	if (i == buf_len)
	{
		free(buf);
		AfxMessageBox("EXP中设置的参数全是0，请先导入参数");
		return;
	}


	//拷贝数据		
	memcpy(&m_StatDlg.m_AEStatDlg.m_Exp, buf, buf_len);

	free(buf);
}

void CISPCTRL_TOOLDlg::OnButtonReadtmp() 
{
	// TODO: Add your control notification handler code here
	CFile file;
	CFileException e;
	TCHAR path[MAX_PATH] = {0};
	TCHAR *dir_name = "tmp.conf";

	_tcscpy(path, ConvertAbsolutePath(dir_name));

	if (!file.Open(path, CFile::modeRead, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	char *buf = NULL;
	UINT size = 0;

	size = file.GetLength();

	buf = (char*)malloc(size);

	if (NULL == buf)
	{
		AfxMessageBox("OnButtonReadtmp buf malloc failed!", MB_OK);
		return;
	}

	file.Read(buf, size);
	file.Close();

	m_bOnlySensor = FALSE;

	DecodeTotalFile(buf, size);

	free(buf);
}

void CISPCTRL_TOOLDlg::OnButtonSavetmp() 
{
	// TODO: Add your control notification handler code here
	char buf[CFG_INFO_MAX_LEN] = {0};
	UINT size = 0;

	if (!GetSubFileData(buf, &size))
	{
		return;
	}

	SaveSubFile(m_SubFileId, buf, size);

	CFile file;
	CFileException e;
	TCHAR path[MAX_PATH] = {0};
	TCHAR *dir_name = "tmp.conf";

	_tcscpy(path, ConvertAbsolutePath(dir_name));

	if (!file.Open(path, CFile::modeCreate|CFile::modeWrite, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	for (int i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (m_SubFile[i].bvalid)
		{
			file.Write(m_SubFile[i].pdata, m_SubFile[i].filelen);
		}
	}

	file.Close();
}

void CISPCTRL_TOOLDlg::OnButtonEptsubfile() 
{
	// TODO: Add your control notification handler code here
	char buf[CFG_INFO_MAX_LEN] = {0};
	UINT size = 0;

	if (!GetSubFileData(buf, &size))
	{
		return;
	}

	SaveSubFile(m_SubFileId, buf, size);


	CFile file;
	CFileException e;
	CFileDialog dlg(FALSE, "*.conf", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		"Config File(*.conf)|*.conf|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	if (!file.Open(dlg.GetPathName(), CFile::modeCreate|CFile::modeWrite, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	file.Write(m_SubFile[m_SubFileId].pdata, m_SubFile[m_SubFileId].filelen);
	file.Close();
}


T_U16 Isp_Struct_len_v2[ISP_MISC_V2 + 1] = {sizeof(AK_ISP_INIT_BLC_V2),
									sizeof(AK_ISP_INIT_LSC_V2),
									sizeof(AK_ISP_INIT_RAW_LUT_V2),
									sizeof(AK_ISP_INIT_NR_V2),
									sizeof(AK_ISP_INIT_3DNR_V2),
									sizeof(AK_ISP_INIT_GB_V2),
									sizeof(AK_ISP_INIT_DEMO_V2),
									sizeof(AK_ISP_INIT_GAMMA_V2),
									sizeof(AK_ISP_INIT_CCM_V2),
									sizeof(AK_ISP_INIT_FCS_V2),
									sizeof(AK_ISP_INIT_WDR_V2),
									sizeof(AK_ISP_INIT_EDGE_V2),
									sizeof(AK_ISP_INIT_SHARP_V2),
									sizeof(AK_ISP_INIT_SATURATION_V2),
									sizeof(AK_ISP_INIT_CONTRAST_V2),
									sizeof(AK_ISP_INIT_RGB2YUV_V2),
									sizeof(AK_ISP_INIT_EFFECT_V2),
									sizeof(AK_ISP_INIT_DPC_V2),
									sizeof(AK_ISP_INIT_WEIGHT_V2),
									sizeof(AK_ISP_INIT_AF_V2),
									sizeof(AK_ISP_INIT_WB_V2),
									sizeof(AK_ISP_INIT_EXP_V2),
									sizeof(AK_ISP_INIT_MISC_V2)
									};


T_U32 sensor_id_table[17] = {0xa042, //h42
							0x9711, //ov9712
							0x0130, //ar0130
							0x1045, //sc1045
							0x1035, //sc1035
							-2, //other
							0x1004, //gc1024
							0x1135, //sc1135
							0x5150, //tvp5150
							0x7150, //gm7150
							0xa061, //h61
							0x1145, //sc1145
							0x3703, //bf3703
							0x0308, //gc0308
							0x9732, //ov9732
							0xa062, //h62
							0x630a, //amt630a
							};

bool CISPCTRL_TOOLDlg::CheckSubFileHeadInfo_v2(CFGFILE_HEADINFO_V2* headinfo)
{
	if (NULL == headinfo)
		return FALSE;

	fprintf(stderr, "CheckSubFileHeadInfo_v2, sensor id : %d, time : %d-%d-%d, %02d:%02d:%02d, subFileId:%d!\n", 
		headinfo->sensorId, headinfo->year, headinfo->month, headinfo->day, 
		headinfo->hour, headinfo->minute, headinfo->second, headinfo->subFileId);

	
	if (headinfo->year < 1900)
		return FALSE;

	if (headinfo->month > 12 || headinfo->month < 1)
		return FALSE;
	if (headinfo->day > 31 || headinfo->day < 1)
		return FALSE;

	if (headinfo->hour > 23)
		return FALSE;

	if (headinfo->minute > 59)
		return FALSE;

	if (headinfo->second > 59)
		return FALSE;

	if (headinfo->subFileId > 6)
		return FALSE;


	return TRUE;
}

T_BOOL CISPCTRL_TOOLDlg::CheckSubFileData_v2(char* cfgBuf, T_U32 size)
{
	T_U8 i = 0;
	T_U32 total = 0;
	T_U16 moduleId = 0;
	T_U16 length = 0;
	T_U32 offset = 0;
	
	if (AK_NULL == cfgBuf || 0 == size)
	{
		fprintf(stderr, "CheckSubFileData_v2 cfgBuf null or size is 0, size:%lu!\n", size);
		return AK_FALSE;
	}

	CFGFILE_HEADINFO_V2 headinfo = {0};
	memcpy(&headinfo, cfgBuf, sizeof(CFGFILE_HEADINFO_V2));

	if (!CheckSubFileHeadInfo_v2(&headinfo))
	{
		fprintf(stderr, "CheckSubFileHeadInfo_v2 failed!\n");
		return AK_FALSE;
	}

	offset = sizeof(CFGFILE_HEADINFO_V2);
	total += sizeof(CFGFILE_HEADINFO_V2);

	for (i=ISP_BB_V2; i<=ISP_MISC_V2; i++)
	{
		memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);
		memcpy(&length, cfgBuf + offset + ISP_MODULE_ID_SIZE, ISP_MODULE_LEN_SIZE);

		//fprintf(stderr, "i: %d, moduleId : %d, length : %d, Structlen : %d!\n", i, moduleId, length, Isp_Struct_len[i]);

		if ((moduleId != i) || (length != Isp_Struct_len_v2[i]))
		{
			fprintf(stderr, "CheckSubFileData_v2 data err!\n");
			return AK_FALSE;
		}
	
		offset += Isp_Struct_len_v2[i];
		total += Isp_Struct_len_v2[i];
	}

	//sensor
	memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);

	if (moduleId != ISP_SENSOR_V2)
	{
		fprintf(stderr, "CheckSubFileData_v2 sensor id err!\n");
		return AK_FALSE;
	}

	offset += ISP_MODULE_ID_SIZE;
	
	memcpy(&length, cfgBuf + offset, ISP_MODULE_LEN_SIZE);

	total += ISP_MODULE_ID_SIZE + ISP_MODULE_LEN_SIZE + length;

	if ((size != total) && (size != total + CFG_FILE_NOTES_LEN_V2))
	{
		fprintf(stderr, "CheckSubFileData_v2 size err!\n");
		return AK_FALSE;
	}


	m_SysDlg.SetSensorId(headinfo.sensorId);
	m_SysDlg.SetStyleId(headinfo.styleId);
	

	if (0 == m_version[0])
	{
		memcpy(m_version, headinfo.version, CFG_FILE_VERSION_LEN_V2);
	}

	m_SysDlg.SetVersion((char*)m_version);

	return AK_TRUE;
}


bool CISPCTRL_TOOLDlg::Convert_v2SubFile(T_U8 subfileid, char *buf, UINT size) 
{
	char *p = NULL, *q = NULL;
	int nLen = 0;
	UINT decodeLen = 0;
	UINT convertLen = 0;
	char *bufout = NULL;
	CFGFILE_HEADINFO head = {0};
	CFGFILE_HEADINFO_V2 head_v2 = {0};

	p = buf;

	if (!CheckSubFileData_v2(p, size))
	{
		AfxMessageBox("配置文件数据错误!", MB_OK);
		return 0;
	}

	bufout = (char*)malloc(50*1024);
	if (NULL == bufout)
	{
		AfxMessageBox("bufout malloc failed!\n", 0, 0);
		return 0;
	}
	memset(bufout, 0, 50*1024);

	q = bufout + sizeof(CFGFILE_HEADINFO);
	convertLen += sizeof(CFGFILE_HEADINFO);

	memcpy(&head_v2, p, sizeof(CFGFILE_HEADINFO_V2));
	p += sizeof(CFGFILE_HEADINFO_V2);
	decodeLen += sizeof(CFGFILE_HEADINFO_V2);

	//head
	head.main_version = MAIN_VERSION;
	head.sensorId = sensor_id_table[head_v2.sensorId];
	head.year = head_v2.year;
	head.month = head_v2.month;
	head.day = head_v2.day;
	head.hour = head_v2.hour;
	head.minute = head_v2.minute;
	head.second = head_v2.second;
	head.subFileId = head_v2.subFileId;
	memcpy(head.file_version, head_v2.version, CFG_FILE_VERSION_LEN_V2);

	if (head_v2.styleId >= 3)
	{
		head.styleId = head_v2.styleId - 3;
	}
	else
	{
		head.styleId = 0;
	}

	
	//BLC
	nLen = sizeof(AK_ISP_INIT_BLC_V2);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;
	

	//LSC
	nLen = sizeof(AK_ISP_INIT_LSC_V2);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//raw lut
	nLen = sizeof(AK_ISP_INIT_RAW_LUT_V2);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//NR
	nLen = sizeof(AK_ISP_INIT_NR_V2);
	m_DenoiseDlg.Convert_v2_data((AK_ISP_INIT_NR*)q, (AK_ISP_INIT_NR_V2 *)p);
	q += sizeof(AK_ISP_INIT_NR);
	convertLen += sizeof(AK_ISP_INIT_NR);
	p += nLen;
	decodeLen +=nLen;


	//3DNR
	nLen = sizeof(AK_ISP_INIT_3DNR_V2);
	m_3DNRDlg.Convert_v2_data((AK_ISP_INIT_3DNR*)q, (AK_ISP_INIT_3DNR_V2 *)p);
	q += sizeof(AK_ISP_INIT_3DNR);
	convertLen += sizeof(AK_ISP_INIT_3DNR);
	p += nLen;
	decodeLen +=nLen;

	//GB
	nLen = sizeof(AK_ISP_INIT_GB_V2);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//demo
	nLen = sizeof(AK_ISP_INIT_DEMO_V2);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//gamma
	nLen = sizeof(AK_ISP_INIT_GAMMA_V2);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//CCM
	nLen = sizeof(AK_ISP_INIT_CCM_V2);
	m_CCMDlg.Convert_v2_data((AK_ISP_INIT_CCM*)q, (AK_ISP_INIT_CCM_V2 *)p);
	q += sizeof(AK_ISP_INIT_CCM);
	convertLen += sizeof(AK_ISP_INIT_CCM);
	p += nLen;
	decodeLen +=nLen;

	//FCS
	nLen = sizeof(AK_ISP_INIT_FCS_V2);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//WDR
	nLen = sizeof(AK_ISP_INIT_WDR_V2);
	m_WdrDlg.Convert_v2_data((AK_ISP_INIT_WDR*)q, (AK_ISP_INIT_WDR_V2 *)p);
	q += sizeof(AK_ISP_INIT_WDR);
	convertLen += sizeof(AK_ISP_INIT_WDR);
	p += nLen;
	decodeLen +=nLen;

	//EDGE
	nLen = sizeof(AK_ISP_INIT_EDGE_V2);
	p += nLen;
	decodeLen +=nLen;

	//sharp
	nLen = sizeof(AK_ISP_INIT_SHARP_V2);
	m_SharpDlg.Convert_v2_data((AK_ISP_INIT_SHARP*)q, (AK_ISP_INIT_SHARP_V2 *)p);
	q += sizeof(AK_ISP_INIT_SHARP);
	convertLen += sizeof(AK_ISP_INIT_SHARP);
	p += nLen;
	decodeLen +=nLen;

	//saturation
	nLen = sizeof(AK_ISP_INIT_SATURATION_V2);
	m_SaturationDlg.Convert_v2_data((AK_ISP_INIT_SATURATION*)q, (AK_ISP_INIT_SATURATION_V2 *)p);
	q += sizeof(AK_ISP_INIT_SATURATION);
	convertLen += sizeof(AK_ISP_INIT_SATURATION);
	p += nLen;
	decodeLen +=nLen;

	//contrast
	nLen = sizeof(AK_ISP_INIT_CONTRAST_V2);
	memcpy(q, p, nLen);
	q[0] = ISP_CONTRAST;
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//rgb to yuv
	nLen = sizeof(AK_ISP_INIT_RGB2YUV_V2);
	memcpy(q, p, nLen);
	q[0] = ISP_RGB2YUV;
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//yuv effect
	nLen = sizeof(AK_ISP_INIT_EFFECT_V2);
	memcpy(q, p, nLen);
	q[0] = ISP_YUVEFFECT;
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//DPC
	nLen = sizeof(AK_ISP_INIT_DPC_V2);
	m_DpcDlg.Convert_v2_data((AK_ISP_INIT_DPC*)q, (AK_ISP_INIT_DPC_V2 *)p);
	q += sizeof(AK_ISP_INIT_DPC);
	convertLen += sizeof(AK_ISP_INIT_DPC);
	p += nLen;
	decodeLen +=nLen;

	//zone weight
	nLen = sizeof(AK_ISP_INIT_WEIGHT_V2);
	memcpy(q, p, nLen);
	q[0] = ISP_ZONE_WEIGHT;
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//AF
	nLen = sizeof(AK_ISP_INIT_AF_V2);
	m_AFDlg.Convert_v2_data((AK_ISP_INIT_AF*)q, (AK_ISP_INIT_AF_V2 *)p);
	q += sizeof(AK_ISP_INIT_AF);
	convertLen += sizeof(AK_ISP_INIT_AF);
	p += nLen;
	decodeLen +=nLen;


	//WB
	nLen = sizeof(AK_ISP_INIT_WB_V2);
	m_WbDlg.Convert_v2_data((AK_ISP_INIT_WB*)q, (AK_ISP_INIT_WB_V2 *)p);
	q += sizeof(AK_ISP_INIT_WB);
	convertLen += sizeof(AK_ISP_INIT_WB);
	p += nLen;
	decodeLen +=nLen;
	

	//EXP
	nLen = sizeof(AK_ISP_INIT_EXP_V2);
	m_ExpDlg.Convert_v2_data((AK_ISP_INIT_EXP*)q, (AK_ISP_INIT_EXP_V2 *)p);
	q += sizeof(AK_ISP_INIT_EXP);
	convertLen += sizeof(AK_ISP_INIT_EXP);
	p += nLen;
	decodeLen +=nLen;


	//misc
	nLen = sizeof(AK_ISP_INIT_MISC_V2);
	m_MiscDlg.Convert_v2_data((AK_ISP_INIT_MISC*)q, (AK_ISP_INIT_MISC_V2 *)p);
	q += sizeof(AK_ISP_INIT_MISC);
	convertLen += sizeof(AK_ISP_INIT_MISC);
	p += nLen;
	decodeLen +=nLen;

	
	//y gamma
	m_Y_GammaDlg.GetDefaultData((AK_ISP_INIT_Y_GAMMA*)q);
	q += sizeof(AK_ISP_INIT_Y_GAMMA);
	convertLen += sizeof(AK_ISP_INIT_Y_GAMMA);

	//hue
	q[0] = ISP_HUE;
	q[2] = sizeof(AK_ISP_INIT_HUE) & 0xff;
	q[3] = (sizeof(AK_ISP_INIT_HUE) & 0xff00) >> 8;
	q += sizeof(AK_ISP_INIT_HUE);
	convertLen += sizeof(AK_ISP_INIT_HUE);


	memcpy(q, p, 2);
	q[0] = ISP_SENSOR;
	q += 2;
	convertLen += 2;
	p += 2;
	decodeLen +=2;

	//sensor
	memcpy(&nLen, p, 2);
	memcpy(q, p, 2);
	q += 2;
	convertLen += 2;
	p += 2;
	decodeLen +=2;
	
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//notes
	if (decodeLen < size)
	{
		memcpy(head.notes, p, 300);
	}

	memcpy(bufout, &head, sizeof(CFGFILE_HEADINFO));


	if (NULL != m_SubFile[subfileid].pdata)
	{
		free (m_SubFile[subfileid].pdata);
		m_SubFile[subfileid].pdata = NULL;
	}

	m_SubFile[subfileid].pdata = (char*)malloc(convertLen);
	if (NULL == m_SubFile[subfileid].pdata)
	{
		AfxMessageBox("m_SubFile[subfileid].pdata malloc failed!\n", 0, 0);
		return 0;
	}

	memcpy(m_SubFile[subfileid].pdata, bufout, convertLen);
	m_SubFile[subfileid].bvalid = 1;
	m_SubFile[subfileid].filelen = convertLen;

	free(bufout);

	return 1;
}


T_BOOL CISPCTRL_TOOLDlg::CheckTotalFileData_v2(char* cfgBuf, T_U32 size)
{
	T_U8 i = 0;
	T_U32 total = 0;
	T_U16 moduleId = 0;
	T_U16 length = 0;
	T_U32 offset = 0;
	T_U8 subfileid[SUBFILE_NUM_MAX] = {0};
	T_U32 subfilelen[SUBFILE_NUM_MAX] = {0};
	T_U32 subfileoffset[SUBFILE_NUM_MAX] = {0};
	T_U8 subfilecnt = 0;
	
	if (AK_NULL == cfgBuf || 0 == size)
	{
		fprintf(stderr, "CheckTotalFileData_v2 cfgBuf null or size is 0, size:%lu!\n", size);
		return AK_FALSE;
	}

	CFGFILE_HEADINFO_V2 headinfo = {0};

CHECK_ONE_SUBFILE:
	memcpy(&headinfo, cfgBuf+offset, sizeof(CFGFILE_HEADINFO_V2));

	if (!CheckSubFileHeadInfo_v2(&headinfo))
	{
		fprintf(stderr, "CheckTotalFileData_v2 failed!\n");
		return AK_FALSE;
	}

	offset += sizeof(CFGFILE_HEADINFO_V2);
	total += sizeof(CFGFILE_HEADINFO_V2);
	subfilelen[subfilecnt] += sizeof(CFGFILE_HEADINFO_V2);

	for (i=ISP_BB_V2; i<=ISP_MISC_V2; i++)
	{
		memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);
		memcpy(&length, cfgBuf + offset + ISP_MODULE_ID_SIZE, ISP_MODULE_LEN_SIZE);

		//fprintf(stderr, "i: %d, moduleId : %d, length : %d, Structlen : %d!\n", i, moduleId, length, Isp_Struct_len[i]);

		if ((moduleId != i) || (length != Isp_Struct_len_v2[i]))
		{
			fprintf(stderr, "CheckTotalFileData_v2 data err!\n");
			return AK_FALSE;
		}
	
		offset += Isp_Struct_len_v2[i];
		total += Isp_Struct_len_v2[i];
		subfilelen[subfilecnt] += Isp_Struct_len_v2[i];
	}

	//sensor
	memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);

	if (moduleId != ISP_SENSOR_V2)
	{
		fprintf(stderr, "CheckTotalFileData_v2 sensor id err!\n");
		return AK_FALSE;
	}

	offset += ISP_MODULE_ID_SIZE;
	total += ISP_MODULE_ID_SIZE;
	subfilelen[subfilecnt] += ISP_MODULE_ID_SIZE;
	
	memcpy(&length, cfgBuf + offset, ISP_MODULE_LEN_SIZE);

	total += ISP_MODULE_LEN_SIZE + length;
	offset += ISP_MODULE_LEN_SIZE + length;
	subfilelen[subfilecnt] += ISP_MODULE_LEN_SIZE + length;

	if (0 == headinfo.styleId)	//old cfg file
	{
		fprintf(stderr, "CheckTotalFileData_v2 cfg file is old version!\n");
		AfxMessageBox("配置文件版本旧，请用V1.1.XX版本的工具调试!", MB_OK);
		return AK_FALSE;
	}
	else
	{
		offset += CFG_FILE_NOTES_LEN_V2;
		total += CFG_FILE_NOTES_LEN_V2;
		subfilelen[subfilecnt] += CFG_FILE_NOTES_LEN_V2;


		subfileid[subfilecnt] = headinfo.subFileId;
		if (subfilecnt > 0)
		{
			subfileoffset[subfilecnt] = subfilelen[subfilecnt-1];
		}
	}

	subfilecnt++;

	if (size > total && subfilecnt < SUBFILE_NUM_MAX)
	{
		goto CHECK_ONE_SUBFILE;
	}

	for (i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (NULL != m_SubFile[i].pdata)
		{
			free(m_SubFile[i].pdata);
			m_SubFile[i].pdata = NULL;
		}

		m_SubFile[i].bvalid = 0;
		m_SubFile[i].filelen = 0;
	}
	
	for (i=0; i<subfilecnt; i++)
	{
		Convert_v2SubFile(subfileid[i], cfgBuf + subfileoffset[i], subfilelen[i]);
	}

	SetSubFileNum();


	return AK_TRUE;
}


bool CISPCTRL_TOOLDlg::DecodeTotalFile_v2(char *buf, UINT size) 
{
	char *p = NULL;
	int nLen = 0;
	UINT decodeLen = 0;

	p = buf;

	CFGFILE_HEADINFO_V2 headinfo = {0};
	memcpy(&headinfo, p, sizeof(CFGFILE_HEADINFO_V2));


	if (!CheckTotalFileData_v2(p, size))
	{
		AfxMessageBox("配置文件数据错误!", MB_OK);
		return 0;
	}

	memset(m_version, 0, 6);

	//CleanAll();

	if (m_SubFile[m_SubFileId].bvalid)
	{
		DecodeSubFileData(m_SubFile[m_SubFileId].pdata, m_SubFile[m_SubFileId].filelen);
	}

	return 1;
}


void CISPCTRL_TOOLDlg::OnButtonReadV2() 
{
	// TODO: Add your control notification handler code here
	CFile file;
	CFileException e;
	CFileDialog dlg(TRUE, "*.conf", NULL, OFN_HIDEREADONLY,
		"Config File(*.conf)|*.conf|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	if (!file.Open(dlg.GetPathName(), CFile::modeRead, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	char *buf = NULL;
	UINT size = 0;

	size = file.GetLength();

	buf = (char*)malloc(size);

	if (NULL == buf)
	{
		AfxMessageBox("OnButtonRead buf malloc failed!", MB_OK);
		return;
	}

	file.Read(buf, size);
	file.Close();

	m_bOnlySensor = FALSE;

	DecodeTotalFile_v2(buf, size);

	free(buf);
}


void CISPCTRL_TOOLDlg::SetMstboxTimer() 
{
	m_msgboxshow_timer = SetTimer(MSGBOX_SHOW_TIMER_ID, MSGBOX_SHOW_TIME, NULL);
}



T_U16 Isp_Struct_len_v3[ISP_HUE + 1] = {sizeof(AK_ISP_INIT_BLC_V3),
									sizeof(AK_ISP_INIT_LSC_V3),
									sizeof(AK_ISP_INIT_RAW_LUT_V3),
									sizeof(AK_ISP_INIT_NR_V3),
									sizeof(AK_ISP_INIT_3DNR_V3),
									sizeof(AK_ISP_INIT_GB_V3),
									sizeof(AK_ISP_INIT_DEMO_V3),
									sizeof(AK_ISP_INIT_GAMMA_V3),
									sizeof(AK_ISP_INIT_CCM_V3),
									sizeof(AK_ISP_INIT_FCS_V3),
									sizeof(AK_ISP_INIT_WDR_V3),
									//sizeof(AK_ISP_INIT_EDGE),
									sizeof(AK_ISP_INIT_SHARP_V3),
									sizeof(AK_ISP_INIT_SATURATION_V3),
									sizeof(AK_ISP_INIT_CONTRAST_V3),
									sizeof(AK_ISP_INIT_RGB2YUV_V3),
									sizeof(AK_ISP_INIT_EFFECT_V3),
									sizeof(AK_ISP_INIT_DPC_V3),
									sizeof(AK_ISP_INIT_WEIGHT_V3),
									sizeof(AK_ISP_INIT_AF_V3),
									sizeof(AK_ISP_INIT_WB_V3),
									sizeof(AK_ISP_INIT_EXP_V3),
									sizeof(AK_ISP_INIT_MISC_V3),
									sizeof(AK_ISP_INIT_Y_GAMMA_V3),
									sizeof(AK_ISP_INIT_HUE_V3)
									};



bool CISPCTRL_TOOLDlg::CheckSubFileHeadInfo_v3(CFGFILE_HEADINFO_V3* headinfo)
{
	if (NULL == headinfo)
		return FALSE;

	fprintf(stderr, "CheckSubFileHeadInfo_v3, main version : %d, file version : %s, sensor id : 0x%x, style id : %d, \
		modify time : %d-%d-%d, %02d:%02d:%02d, subFileId:%d!\n", headinfo->main_version, headinfo->file_version, 
		headinfo->sensorId, headinfo->styleId, headinfo->year, headinfo->month, headinfo->day, 
		headinfo->hour, headinfo->minute, headinfo->second, headinfo->subFileId);

	if (3 != headinfo->main_version)
	{
		if (headinfo->main_version & 0x0000ff00)
		{
			fprintf(stderr, "CheckSubFileHeadInfo_v3 cfg file is old version, v2!\n");
		}
		else if (headinfo->main_version & 0xffff0000)
		{
			fprintf(stderr, "CheckSubFileHeadInfo_v3 cfg file is old version, v1!\n");
		}

		return FALSE;
	}

	if (headinfo->year < 1900)
		return FALSE;

	if (headinfo->month > 12 || headinfo->month < 1)
		return FALSE;
	if (headinfo->day > 31 || headinfo->day < 1)
		return FALSE;

	if (headinfo->hour > 23)
		return FALSE;

	if (headinfo->minute > 59)
		return FALSE;

	if (headinfo->second > 59)
		return FALSE;

	if (headinfo->subFileId > 4)
		return FALSE;


	return TRUE;
}

T_BOOL CISPCTRL_TOOLDlg::CheckSubFileData_v3(char* cfgBuf, T_U32 size)
{
	T_U8 i = 0;
	T_U32 total = 0;
	T_U16 moduleId = 0;
	T_U16 length = 0;
	T_U32 offset = 0;
	
	if (AK_NULL == cfgBuf || 0 == size)
	{
		fprintf(stderr, "CheckSubFileData_v3 cfgBuf null or size is 0, size:%lu!\n", size);
		return AK_FALSE;
	}

	CFGFILE_HEADINFO_V3 headinfo = {0};
	memcpy(&headinfo, cfgBuf, sizeof(CFGFILE_HEADINFO_V3));

	if (!CheckSubFileHeadInfo_v3(&headinfo))
	{
		fprintf(stderr, "CheckSubFileHeadInfo_v3 failed!\n");
		return AK_FALSE;
	}

	offset = sizeof(CFGFILE_HEADINFO_V3);
	total += sizeof(CFGFILE_HEADINFO_V3);

	for (i=ISP_BB; i<=ISP_HUE; i++)
	{
		memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);
		memcpy(&length, cfgBuf + offset + ISP_MODULE_ID_SIZE, ISP_MODULE_LEN_SIZE);

		//fprintf(stderr, "i: %d, moduleId : %d, length : %d, Structlen : %d!\n", i, moduleId, length, Isp_Struct_len[i]);

		if ((moduleId != i) || (length != Isp_Struct_len_v3[i]))
		{
			fprintf(stderr, "CheckSubFileData_v3 data err!\n");
			return AK_FALSE;
		}
	
		offset += Isp_Struct_len_v3[i];
		total += Isp_Struct_len_v3[i];
	}

	//sensor
	memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);

	if (moduleId != ISP_SENSOR)
	{
		fprintf(stderr, "CheckSubFileData_v3 sensor id err!\n");
		return AK_FALSE;
	}

	offset += ISP_MODULE_ID_SIZE;
	
	memcpy(&length, cfgBuf + offset, ISP_MODULE_LEN_SIZE);

	total += ISP_MODULE_ID_SIZE + ISP_MODULE_LEN_SIZE + length;

	if (size != total)
	{
		fprintf(stderr, "CheckSubFileData_v3 size err!\n");
		return AK_FALSE;
	}


	m_SysDlg.SetSensorId(headinfo.sensorId);
	m_SysDlg.SetStyleId(headinfo.styleId);
	m_SysDlg.SetNotes(headinfo.notes);

	if (0 == m_version[0])
	{
		memcpy(m_version, headinfo.file_version, CFG_FILE_VERSION_LEN);
	}

	m_SysDlg.SetVersion((char*)m_version);

	return AK_TRUE;
}



bool CISPCTRL_TOOLDlg::Convert_v3SubFile(T_U8 subfileid, char *buf, UINT size) 
{
	char *p = NULL, *q = NULL;
	int nLen = 0;
	UINT decodeLen = 0;
	UINT convertLen = 0;
	char *bufout = NULL;
	CFGFILE_HEADINFO head = {0};
	CFGFILE_HEADINFO_V3 head_v3 = {0};

	p = buf;

	if (!CheckSubFileData_v3(p, size))
	{
		AfxMessageBox("配置文件数据错误!", MB_OK);
		return 0;
	}

	bufout = (char*)malloc(50*1024);
	if (NULL == bufout)
	{
		AfxMessageBox("bufout malloc failed!\n", 0, 0);
		return 0;
	}
	memset(bufout, 0, 50*1024);

	memcpy(&head_v3, p, sizeof(CFGFILE_HEADINFO_V3));
	p += sizeof(CFGFILE_HEADINFO_V3);
	decodeLen += sizeof(CFGFILE_HEADINFO_V3);

	//head
	head.main_version = MAIN_VERSION;
	head.sensorId = head_v3.sensorId;
	head.year = head_v3.year;
	head.month = head_v3.month;
	head.day = head_v3.day;
	head.hour = head_v3.hour;
	head.minute = head_v3.minute;
	head.second = head_v3.second;
	head.subFileId = head_v3.subFileId;
	head.styleId = head_v3.styleId;
	memcpy(head.file_version, head_v3.file_version, CFG_FILE_VERSION_LEN);
	memcpy(head.notes, head_v3.notes, CFG_FILE_NOTES_LEN);

	memcpy(bufout, &head, sizeof(CFGFILE_HEADINFO));
	q = bufout + sizeof(CFGFILE_HEADINFO);
	convertLen += sizeof(CFGFILE_HEADINFO);
	
	//BLC
	nLen = sizeof(AK_ISP_INIT_BLC_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;
	

	//LSC
	nLen = sizeof(AK_ISP_INIT_LSC_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//raw lut
	nLen = sizeof(AK_ISP_INIT_RAW_LUT_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//NR
	nLen = sizeof(AK_ISP_INIT_NR_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;


	//3DNR
	nLen = sizeof(AK_ISP_INIT_3DNR_V3);
	m_3DNRDlg.Convert_v3_data((AK_ISP_INIT_3DNR*)q, (AK_ISP_INIT_3DNR_V3 *)p);
	q += sizeof(AK_ISP_INIT_3DNR);
	convertLen += sizeof(AK_ISP_INIT_3DNR);
	p += nLen;
	decodeLen +=nLen;

	//GB
	nLen = sizeof(AK_ISP_INIT_GB_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//demo
	nLen = sizeof(AK_ISP_INIT_DEMO_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//gamma
	nLen = sizeof(AK_ISP_INIT_GAMMA_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//CCM
	nLen = sizeof(AK_ISP_INIT_CCM_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//FCS
	nLen = sizeof(AK_ISP_INIT_FCS_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//WDR
	nLen = sizeof(AK_ISP_INIT_WDR_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;


	//sharp
	nLen = sizeof(AK_ISP_INIT_SHARP_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//saturation
	nLen = sizeof(AK_ISP_INIT_SATURATION_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//contrast
	nLen = sizeof(AK_ISP_INIT_CONTRAST_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//rgb to yuv
	nLen = sizeof(AK_ISP_INIT_RGB2YUV_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//yuv effect
	nLen = sizeof(AK_ISP_INIT_EFFECT_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//DPC
	nLen = sizeof(AK_ISP_INIT_DPC_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//zone weight
	nLen = sizeof(AK_ISP_INIT_WEIGHT_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//AF
	nLen = sizeof(AK_ISP_INIT_AF_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;


	//WB
	nLen = sizeof(AK_ISP_INIT_WB_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;
	

	//EXP
	nLen = sizeof(AK_ISP_INIT_EXP_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;


	//misc
	nLen = sizeof(AK_ISP_INIT_MISC_V3);
	m_MiscDlg.Convert_v3_data((AK_ISP_INIT_MISC*)q, (AK_ISP_INIT_MISC_V3 *)p);
	q += sizeof(AK_ISP_INIT_MISC);
	convertLen += sizeof(AK_ISP_INIT_MISC);
	p += nLen;
	decodeLen +=nLen;

	
	//y gamma
	nLen = sizeof(AK_ISP_INIT_Y_GAMMA_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;

	//hue
	nLen = sizeof(AK_ISP_INIT_HUE_V3);
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;


	memcpy(q, p, 2);
	q[0] = ISP_SENSOR;
	q += 2;
	convertLen += 2;
	p += 2;
	decodeLen +=2;

	//sensor
	memcpy(&nLen, p, 2);
	memcpy(q, p, 2);
	q += 2;
	convertLen += 2;
	p += 2;
	decodeLen +=2;
	
	memcpy(q, p, nLen);
	q += nLen;
	convertLen += nLen;
	p += nLen;
	decodeLen +=nLen;


	if (NULL != m_SubFile[subfileid].pdata)
	{
		free (m_SubFile[subfileid].pdata);
		m_SubFile[subfileid].pdata = NULL;
	}

	m_SubFile[subfileid].pdata = (char*)malloc(convertLen);
	if (NULL == m_SubFile[subfileid].pdata)
	{
		AfxMessageBox("m_SubFile[subfileid].pdata malloc failed!\n", 0, 0);
		return 0;
	}
	
	head.subfilelen = convertLen;
	memcpy(bufout, &head, sizeof(CFGFILE_HEADINFO));

	memcpy(m_SubFile[subfileid].pdata, bufout, convertLen);
	m_SubFile[subfileid].bvalid = 1;
	m_SubFile[subfileid].filelen = convertLen;

	free(bufout);

	return 1;
}



T_BOOL CISPCTRL_TOOLDlg::CheckTotalFileData_v3(char* cfgBuf, T_U32 size)
{
	T_U8 i = 0;
	T_U32 total = 0;
	T_U16 moduleId = 0;
	T_U16 length = 0;
	T_U32 offset = 0;
	T_U8 subfileid[SUBFILE_NUM_MAX] = {0};
	T_U32 subfilelen[SUBFILE_NUM_MAX] = {0};
	T_U32 subfileoffset[SUBFILE_NUM_MAX] = {0};
	T_U8 subfilecnt = 0;
	
	if (AK_NULL == cfgBuf || 0 == size)
	{
		fprintf(stderr, "CheckTotalFileData_v3 cfgBuf null or size is 0, size:%lu!\n", size);
		return AK_FALSE;
	}

	CFGFILE_HEADINFO_V3 headinfo = {0};

CHECK_ONE_SUBFILE:
	memcpy(&headinfo, cfgBuf+offset, sizeof(CFGFILE_HEADINFO_V3));

	if (!CheckSubFileHeadInfo_v3(&headinfo))
	{
		fprintf(stderr, "CheckTotalFileData_v3 failed!\n");
		return AK_FALSE;
	}

	offset += sizeof(CFGFILE_HEADINFO_V3);
	total += sizeof(CFGFILE_HEADINFO_V3);
	subfilelen[subfilecnt] += sizeof(CFGFILE_HEADINFO_V3);

	for (i=ISP_BB; i<=ISP_HUE; i++)
	{
		memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);
		memcpy(&length, cfgBuf + offset + ISP_MODULE_ID_SIZE, ISP_MODULE_LEN_SIZE);

		//fprintf(stderr, "i: %d, moduleId : %d, length : %d, Structlen : %d!\n", i, moduleId, length, Isp_Struct_len[i]);

		if ((moduleId != i) || (length != Isp_Struct_len_v3[i]))
		{
			fprintf(stderr, "CheckTotalFileData_v3 data err!\n");
			return AK_FALSE;
		}
	
		offset += Isp_Struct_len_v3[i];
		total += Isp_Struct_len_v3[i];
		subfilelen[subfilecnt] += Isp_Struct_len_v3[i];
	}

	//sensor
	memcpy(&moduleId, cfgBuf + offset, ISP_MODULE_ID_SIZE);

	if (moduleId != ISP_SENSOR)
	{
		fprintf(stderr, "CheckTotalFileData_v3 sensor id err!\n");
		return AK_FALSE;
	}

	offset += ISP_MODULE_ID_SIZE;
	total += ISP_MODULE_ID_SIZE;
	subfilelen[subfilecnt] += ISP_MODULE_ID_SIZE;
	
	memcpy(&length, cfgBuf + offset, ISP_MODULE_LEN_SIZE);

	total += ISP_MODULE_LEN_SIZE + length;
	offset += ISP_MODULE_LEN_SIZE + length;
	subfilelen[subfilecnt] += ISP_MODULE_LEN_SIZE + length;

	subfileid[subfilecnt] = headinfo.subFileId;
	if (subfilecnt > 0)
	{
		subfileoffset[subfilecnt] = subfileoffset[subfilecnt-1] + subfilelen[subfilecnt-1];
	}


	subfilecnt++;

	if (size > total && subfilecnt < SUBFILE_NUM_MAX)
	{
		goto CHECK_ONE_SUBFILE;
	}

	for (i=0; i<SUBFILE_NUM_MAX; i++)
	{
		if (NULL != m_SubFile[i].pdata)
		{
			free(m_SubFile[i].pdata);
			m_SubFile[i].pdata = NULL;
		}

		m_SubFile[i].bvalid = 0;
		m_SubFile[i].filelen = 0;
	}
	
	for (i=0; i<subfilecnt; i++)
	{
		Convert_v3SubFile(subfileid[i], cfgBuf + subfileoffset[i], subfilelen[i]);
	}

	SetSubFileNum();


	return AK_TRUE;
}


bool CISPCTRL_TOOLDlg::DecodeTotalFile_v3(char *buf, UINT size) 
{
	char *p = NULL;
	int nLen = 0;
	UINT decodeLen = 0;

	p = buf;

	CFGFILE_HEADINFO_V3 headinfo = {0};
	memcpy(&headinfo, p, sizeof(CFGFILE_HEADINFO_V3));


	if (!CheckTotalFileData_v3(p, size))
	{
		AfxMessageBox("配置文件数据错误!", MB_OK);
		return 0;
	}

	memset(m_version, 0, 16);

	//CleanAll();

	if (m_SubFile[m_SubFileId].bvalid)
	{
		DecodeSubFileData(m_SubFile[m_SubFileId].pdata, m_SubFile[m_SubFileId].filelen);
	}

	return 1;
}


void CISPCTRL_TOOLDlg::OnButtonReadV3() 
{
	// TODO: Add your control notification handler code here
	CFile file;
	CFileException e;
	CFileDialog dlg(TRUE, "*.conf", NULL, OFN_HIDEREADONLY,
		"Config File(*.conf)|*.conf|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	if (!file.Open(dlg.GetPathName(), CFile::modeRead, &e))
	{
		CString str;
		str.Format("打开文件失败！错误码：%u", e.m_cause);
		MessageBox(str, ISPTOOL_NAME, MB_OK|MB_ICONWARNING);
		return;
	}

	char *buf = NULL;
	UINT size = 0;

	size = file.GetLength();

	buf = (char*)malloc(size);

	if (NULL == buf)
	{
		AfxMessageBox("OnButtonRead buf malloc failed!", MB_OK);
		return;
	}

	file.Read(buf, size);
	file.Close();

	m_bOnlySensor = FALSE;

	DecodeTotalFile_v3(buf, size);

	free(buf);
}

void CISPCTRL_TOOLDlg::OnGetLscInfo(WPARAM wParam, LPARAM lParam)
{
	m_LscDlg.refresh_data();	
}

void CISPCTRL_TOOLDlg::OnSetLscInfo(WPARAM wParam, LPARAM lParam)
{
	m_LscDlg.save_calc_result();	
}

void CISPCTRL_TOOLDlg::OnGetRgbGamma(WPARAM wParam, LPARAM lParam)
{
	AK_ISP_INIT_GAMMA gamma_temp = {0};
	int pageId;
	int nLen = sizeof(AK_ISP_INIT_GAMMA);
	
	m_GammaDlg.GetPageInfoSt(pageId, &gamma_temp, nLen);
	memcpy(&m_CCMDlg.m_ImgDlg.gamma_ccm, &gamma_temp.p_gamma_attr, sizeof(AK_ISP_RGB_GAMMA_ATTR));
}

DWORD WINAPI Isptool_Send_Heart_thread(LPVOID lpParameter)
{
	CISPCTRL_TOOLDlg *isptool_heart = (CISPCTRL_TOOLDlg *)lpParameter;
	UINT idex = 0, i = 0;
	while(1)
	{
		if (g_hBurnThread_heart != INVALID_HANDLE_VALUE)
		{
			if(isptool_heart->m_NetCtrl.m_bConnect)
			{
				isptool_heart->m_NetCtrl.SendCommand(ISP_HEARTBEAT, CMD_GET, NULL, 0);
			}
			Sleep(3000);
		}
	}
}

BOOL CISPCTRL_TOOLDlg::Create_Send_Heart_Thread(void)
{
	Close_Send_Heart_Thread();
	g_hBurnThread_heart = CreateThread(NULL, 0, Isptool_Send_Heart_thread, this, 0, NULL);
	if (g_hBurnThread_heart == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	return TRUE;
}


BOOL CISPCTRL_TOOLDlg::Close_Send_Heart_Thread(void)
{
	if(g_hBurnThread_heart != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_hBurnThread_heart);
		g_hBurnThread_heart = INVALID_HANDLE_VALUE;
	}

	return TRUE;
}


