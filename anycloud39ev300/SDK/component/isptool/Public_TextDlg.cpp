// Public_TextDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "Public_TextDlg.h"
#include "ISPCTRL_TOOLDlg.h"
#include "isp_struct.h"
#include "anyka_types.h"
#include <atlconv.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




#define  BB_ADDR_0 "0x00000"
#define  BB_ADDR_1 "0x00100"
#define  BB_ADDR_2 "0x00200"
#define  BB_ADDR_3 "0x00300"
#define  BB_ADDR_4 "0x00400"
#define  BB_ADDR_5 "0x00500"
#define  BB_ADDR_6 "0x00600"
#define  BB_ADDR_7 "0x00700"
#define  BB_ADDR_8 "0x00800"
#define  BB_ADDR_9 "0x00900"


//RGBTOYUV
T_U32 HUE_ADDR[] = 
{
	0x090000,
	0x090100,
	0x090200,
	0x090300,
	0x091001,
	0x091100,
	0x091200,
	0x091300,
	0x092001,
	0x092100,
	0x092200,
	0x092300,
	0x093001,
	0x093100,
	0x093200,
	0x093300,
	0x094001,
	0x094100,
	0x094200,
	0x094300
};

//RGBTOYUV
T_U32 RGBTOYUV_ADDR[] = 
{
	0x180000
};


//坏点较正
T_U32 DPC_ADDR[] = 
{
	0x010000,
	0x010011,
	0x010021,
	0x010031,
	0x010041,
	0x010051,
	0x010061,
	0x010071,
	0x010081,
	0x010091,
	0x010101,
	0x011000
};

//3NDR
T_U32 NR_3DNR_ADDR[] = 
{
	
	0x0a0000,
	0x0a1001,
	0x0a2001,
	0x0a3001,
	0x0a4001,
	0x0a5001,
	0x0a6001,
	0x0a7001,
	0x0a8001,
	0x0a9001
	
};

//EDGE
T_U32 EDGE_ADDR[] = 
{
	0x090000,
	0x091001,
	0x092001,
	0x093001,
	0x094001,
	0x095001,
	0x096001,
	0x097001,
	0x098001,
	0x099001,
	0x09a001
};


//对比度
T_U32 CONSTRAST_ADDR[] = 
{
	0x160000,
	0x161001,
	0x162001,
	0x163001,
	0x164001,
	0x165001,
	0x166001,
	0x167001,
	0x168001,
	0x169001
};

//杂项
T_U32 MISC_ADDR[] = 
{
	0x140003
};


//绿平衡
T_U32 GB_ADDR[] = 
{
	0x150000,
	0x151001,
	0x152001,
	0x153001,
	0x154001,
	0x155001,
	0x156001,
	0x157001,
	0x158001,
	0x159001
};

//zone weight
T_U32 WEIGHT_ADDR[] = 
{
	0x120000
};

//AF
T_U32 AF_ADDR[] = 
{
	0x110000,
	0x111000,
	0x112000,
	0x113000,
	0x114000,
};

//Expsoure
T_U32 EXP_ADDR[] = 
{
	0x101200,
	0x100001,
	0x100101,
	0x100200,
	0x100300,
	0x100400
};

//饱和度
T_U32 SATURATION_ADDR[] = 
{
	0x0e0000,
	0x0e1001,
	0x0e2001,
	0x0e3001,
	0x0e4001,
	0x0e5001,
	0x0e6001,
	0x0e7001,
	0x0e8001,
	0x0e9001
};

//FCS
T_U32 YUVEFFECT_ADDR[] = 
{
	0x0d0000
};

//FCS
T_U32 FCS_ADDR[] = 
{
	0x0c0000,
	0x0c1001,
	0x0c2001,
	0x0c3001,
	0x0c4001,
	0x0c5001,
	0x0c6001,
	0x0c7001,
	0x0c8001,
	0x0c9001
};

//WDR
T_U32 WDR_ADDR[] = 
{
	0x0B0000,
	0x0B0100,
	0x0B0200,
	0x0B0300,
	0x0B0400,
	0x0B0500,
	0x0B0600,

	0x0B1001,
	0x0B1100,
	0x0B1200,
	0x0B1300,
	0x0B1400,
	0x0B1500,
	0x0B1600,

	0x0B2001,
	0x0B2100,
	0x0B2200,
	0x0B2300,
	0x0B2400,
	0x0B2500,
	0x0B2600,

	0x0B3001,
	0x0B3100,
	0x0B3200,
	0x0B3300,
	0x0B3400,
	0x0B3500,
	0x0B3600,

	0x0B4001,
	0x0B4100,
	0x0B4200,
	0x0B4300,
	0x0B4400,
	0x0B4500,
	0x0B4600,

	0x0B5001,
	0x0B5100,
	0x0B5200,
	0x0B5300,
	0x0B5400,
	0x0B5500,
	0x0B5600,

	0x0B6001,
	0x0B6100,
	0x0B6200,
	0x0B6300,
	0x0B6400,
	0x0B6500,
	0x0B6600,

	0x0B7001,
	0x0B7100,
	0x0B7200,
	0x0B7300,
	0x0B7400,
	0x0B7500,
	0x0B7600,

	0x0B8001,
	0x0B8100,
	0x0B8200,
	0x0B8300,
	0x0B8400,
	0x0B8500,
	0x0B8600,

	0x0B9001,
	0x0B9100,
	0x0B9200,
	0x0B9300,
	0x0B9400,
	0x0B9500,
	0x0B9600
};

//NR
T_U32 NR_ADDR[] = 
{
	0x070500,
	0x070000,
	0x070100,
	0x070200,
	0x070300,
	0x070400,
	0x071501,
	0x071000,
	0x071100,
	0x071200,
	0x071300,
	0x071400,
	0x072501,
	0x072000,
	0x072100,
	0x072200,
	0x072300,
	0x072400,
	0x073501,
	0x073000,
	0x073100,
	0x073200,
	0x073300,
	0x073400,
	0x074501,
	0x074000,
	0x074100,
	0x074200,
	0x074300,
	0x074400,
	0x075501,
	0x075000,
	0x075100,
	0x075200,
	0x075300,
	0x075400,
	0x076501,
	0x076000,
	0x076100,
	0x076200,
	0x076300,
	0x076400,
	0x077501,
	0x077000,
	0x077100,
	0x077200,
	0x077300,
	0x077400,
	0x078501,
	0x078000,
	0x078100,
	0x078200,
	0x078300,
	0x078400,
	0x079501,
	0x079000,
	0x079100,
	0x079200,
	0x079300,
	0x079400,
};


//sharp
T_U32 SHARP_ADDR[] = 
{
	0x08f000,
	0x08f010,
	0x08f020,
	0x080000,
	0x081000,
	0x082000,
	0x083000,
	0x084000,
	0x085000,
	0x086000,
	0x087000,
	0x088000,
	0x089000
};

//rgb GAMMA
T_U32 GAMMA_ADDR[] = 
{
	0x060500,  //
	0x060000, //
	0x060100, //
	0x060200
};

//Y GAMMA
T_U32 Y_GAMMA_ADDR[] = 
{
	0x170000,  //
	0x170100
};


////demosaic
T_U32 DEMO_ADDR[] = 
{
	0x040000
};


//raw GAMMA
T_U32 RAW_LUT_ADDR[] = 
{
	0x030000,  //T_U16	raw_r[129];      //10bit
	0x030100, //T_U16	raw_g[129];      //10bit
	0x030200, //T_U16	raw_b[129];      //10bit
	0x030281
};


//
T_U32 CCM_ADDR[] = 
{
	0x050000,
	0x050001,
	0x050002,
	0x050003,
	0x050004,
	0x050011,
	0x050012,
	0x050013,
	0x050021,
	0x050022,
	0x050023,
	0x050031,
	0x050032,
	0x050033,
	
	0x051001,
	0x051002,
	0x051003,
	0x051004,
	0x051011,
	0x051012,
	0x051013,
	0x051021,
	0x051022,
	0x051023,
	0x051031,
	0x051032,
	0x051033,
	
	0x052001,
	0x052002,
	0x052003,
	0x052004,
	0x052011,
	0x052012,
	0x052013,
	0x052021,
	0x052022,
	0x052023,
	0x052031,
	0x052032,
	0x052033,
	
	0x053001,
	0x053002,
	0x053003,
	0x053004,
	0x053011,
	0x053012,
	0x053013,
	0x053021,
	0x053022,
	0x053023,
	0x053031,
	0x053032,
	0x053033,
	
	0x054001,
	0x054002,
	0x054003,
	0x054004,
	0x054011,
	0x054012,
	0x054013,
	0x054021,
	0x054022,
	0x054023,
	0x054031,
	0x054032,
	0x054033,
};



//白平衡
T_U32 AWB_ADDR[] = 
{
	0x0f0000,
	
	0x0f0101,
	0x0f0201,
	0x0f0301,
	0x0f0401,
	0x0f0501,
	0x0f0601,
	0x0f0701,
	0x0f0801,
	0x0f0901,
	0x0f0a01,

	0x0f0b00,
	0x0f0c00,
	0x0f0d00,

	0x0f1000,
	0x0f1001,
	0x0f2001,
	0x0f3001,
	0x0f4001,
	0x0f5001,
	0x0f6001,
	0x0f7001,
	0x0f8001,
	0x0f9001,
	0x0fa001,
};


T_U32 LSC_ADDR[] = 
{
	0x020000,
	0x020001,
	0x020100,
	0x020101,
	0x020200,
	0x020201,
	0x020300,
	0x020301,
	0x020010,
	0x020011,
	0x020110,
	0x020111,
	0x020210,
	0x020211,
	0x020310,
	0x020311,
	0x020020,
	0x020021,
	0x020120,
	0x020121,
	0x020220,
	0x020221,
	0x020320,
	0x020321,
	0x020030,
	0x020031,
	0x020130,
	0x020131,
	0x020230,
	0x020231,
	0x020330,
	0x020331,
	0x020040,
	0x020041,
	0x020140,
	0x020141,
	0x020240,
	0x020241,
	0x020340,
	0x020341,
	0x020050,
	0x020051,
	0x020150,
	0x020151,
	0x020250,
	0x020251,
	0x020350,
	0x020351,
	0x020060,
	0x020061,
	0x020160,
	0x020161,
	0x020260,
	0x020261,
	0x020360,
	0x020361,
	0x020070,
	0x020071,
	0x020170,
	0x020171,
	0x020270,
	0x020271,
	0x020370,
	0x020371,
	0x020080,
	0x020081,
	0x020180,
	0x020181,
	0x020280,
	0x020281,
	0x020380,
	0x020381,
	0x020090,
	0x020091,
	0x020190,
	0x020191,
	0x020290,
	0x020291,
	0x020390,
	0x020391,
	0x0200a0,
	0x0200a1,
	0x0200a2,
	0x0200a3,
	0x0200a4,
	0x0200a5,
	0x0200a6,
	0x0200a7,
	0x0200a8,
	0x0200a9,
	0x0200aa,
	0x0200ab,
	0x0200ac,
	0x0200ad
};


T_U32 BB_ADDR[] = 
{
	0x000000,
	0x000001,
	0x000002,
	0x000003,
	0x000004,
	0x000005,
	0x000006,
	0x000007,
	0x000008,
	0x000009,
	0x001001,
	0x001002,
	0x001003,
	0x001004,
	0x001005,
	0x001006,
	0x001007,
	0x001008,
	0x001009,
	0x002001,
	0x002002,
	0x002003,
	0x002004,
	0x002005,
	0x002006,
	0x002007,
	0x002008,
	0x002009,
	0x003001,
	0x003002,
	0x003003,
	0x003004,
	0x003005,
	0x003006,
	0x003007,
	0x003008,
	0x003009,
	0x004001,
	0x004002,
	0x004003,
	0x004004,
	0x004005,
	0x004006,
	0x004007,
	0x004008,
	0x004009,
	0x005001,
	0x005002,
	0x005003,
	0x005004,
	0x005005,
	0x005006,
	0x005007,
	0x005008,
	0x005009,
	0x006001,
	0x006002,
	0x006003,
	0x006004,
	0x006005,
	0x006006,
	0x006007,
	0x006008,
	0x006009,
	0x007001,
	0x007002,
	0x007003,
	0x007004,
	0x007005,
	0x007006,
	0x007007,
	0x007008,
	0x007009,	
	0x008001,
	0x008002,
	0x008003,
	0x008004,
	0x008005,
	0x008006,
	0x008007,
	0x008008,
	0x008009,
	0x009001,
	0x009002,
	0x009003,
	0x009004,
	0x009005,
	0x009006,
	0x009007,
	0x009008,
	0x009009	
};


AK_ISP_INIT_PARAM   Isp_init_param;
extern T_DIALOG_ID	g_text_DialogId;

/////////////////////////////////////////////////////////////////////////////
// Public_TextDlg dialog

Public_TextDlg::Public_TextDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Public_TextDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(Public_TextDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void Public_TextDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Public_TextDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Public_TextDlg, CDialog)
	//{{AFX_MSG_MAP(Public_TextDlg)
	ON_BN_CLICKED(IDC_BUTTON_GET, OnButtonGet)
	ON_BN_CLICKED(IDC_BUTTON_SET, OnButtonSet)
	ON_BN_CLICKED(IDC_BUTTON_READ, OnButtonRead)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnButtonWrite)
	ON_BN_CLICKED(IDC_BUTTON_COPY1, OnButtonCopy1)
	ON_BN_CLICKED(IDC_BUTTON_COPY2, OnButtonCopy2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Public_TextDlg message handlers

BOOL Public_TextDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Mytip.Create(this);  
	m_Mytip.AddTool(GetDlgItem(IDC_BUTTON_GET), "获取设备端当前模块数据" );
	m_Mytip.AddTool(GetDlgItem(IDC_BUTTON_SET), "设置当前模块数据到设备" );
	m_Mytip.AddTool(GetDlgItem(IDC_BUTTON_READ), "导入文档的数据到工具当前模块上" );
	m_Mytip.AddTool(GetDlgItem(IDC_BUTTON_WRITE), "导出工具的当前模块数据到文档上" );
	m_Mytip.AddTool(GetDlgItem(IDC_BUTTON_COPY1), "拷贝图形界面的数据到文本编辑框" );
	m_Mytip.AddTool(GetDlgItem(IDC_BUTTON_COPY2), "拷贝文本编辑框的数据到图形界面" );
	m_Mytip.SetDelayTime(200); //设置延迟
	m_Mytip.Activate(TRUE); //设置是否启用提示
	return TRUE;
}

BOOL Public_TextDlg::PreTranslateMessage(MSG* pMsg) 
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

	return CDialog::PreTranslateMessage(pMsg);
}


UINT Public_TextDlg::hex2int(const char *str)
{
    int i;
    UINT number=0; 
    int order=1;
    TCHAR ch;
	
    for(i=strlen(str)-1;i>=0;i--)
    {
		ch=str[i];
		if(ch=='x' || ch=='X')break;
		
		if(ch>='0' && ch<='9')
		{
			number+=order*(ch-'0');
			order*=16;
		}
		if(ch>='A' && ch<='F')
		{
			number+=order*(ch-'A'+10);
			order*=16;
		}
		if(ch>='a' && ch<='f')
		{
			number+=order*(ch-'a'+10);
			order*=16;
		}
    }
    return number;
}

void Public_TextDlg::Set_buf_info(char *buf, UINT *buf_idex, UINT addr, int value, T_DISPLAY_MODE valuemode)
{
	char text_temp[20] = {0};
	UINT len = 0;
	
	memset(text_temp, 0, 20);
	if (MODE_16 == valuemode)
	{
		sprintf(text_temp, "0x%04x, 0x%04x\r\n", addr, value); //模式选择
	}
	else
	{
		sprintf(text_temp, "0x%06x, %d\r\n", addr, value); //模式选择
	}
	
	len = strlen(text_temp);
	memcpy(&buf[*buf_idex], text_temp, len);
	*buf_idex = *buf_idex + len;
	
}

int Public_TextDlg::Get_num_by_buf_for_sensor(char *srcbuf, UINT *buf_idex , UINT buf_len)
{
	char buf_oneline[100] = {0};
	char buf_value[100] = {0};
	UINT addr = 0;
	int bb_value = 0;
	BOOL finish_flag = FALSE;
	UINT idex = 0, i = 0, buf_oneline_idex = 0;
	UINT num = 0;
	while (1)
	{
		for (idex = 0; idex < 30; idex++)//读取一行
		{
			if (idex == 0)
			{
				if (memcmp(&srcbuf[*buf_idex], "\r\n", 2) == 0)
				{
					num++;
					finish_flag = TRUE;
					break;	
				}

				if (buf_len == *buf_idex)
				{
					finish_flag = TRUE;
					break;
				}
			}
			
			if(buf_len == *buf_idex)
			{
				num++;
				finish_flag = TRUE;
				break;
			}
			if (memcmp(&srcbuf[*buf_idex], "\r\n", 2) == 0)
			{
				num++;
				*buf_idex = *buf_idex + 2;
				break;	
			}
			if (srcbuf[*buf_idex] == 0)
			{
				AfxMessageBox("此数值为空，请检查", MB_OK);
				return 0;
			}
			*buf_idex = *buf_idex + 1;
		}
		if (idex == 30)
		{
			AfxMessageBox("数据获取出错，请检查", MB_OK);
			return 0;
		}

		if (finish_flag)
		{
			break;
		}
	}

	return num;
}

int Public_TextDlg::Get_value_by_buf_for_sensor(char *srcbuf, UINT *buf_idex , UINT buf_len, T_U16 *sensor_addr, BOOL value_flag)//, char *dstbuf)
{
    char buf_oneline[100] = {0};
	char buf_value[100] = {0};
	UINT addr = 0;
	int bb_value = 0;
	BOOL finish_flag = FALSE;
	UINT idex = 0, i = 0, buf_oneline_idex = 0;
	UINT end_idex = 2;
	
	if (buf_len < *buf_idex)
	{
		AfxMessageBox("结构体的长度比BUG的长度还大，请检查", MB_OK);
		return 0;
	}
	
	for (idex = 0; idex < 30; idex++)//读取一行
	{
		if (idex == 0)
		{
			if (memcmp(&srcbuf[*buf_idex], "\r\n", 2) == 0)
			{
				finish_flag = TRUE;
				break;	
			}
		}
		
		if(buf_len == *buf_idex)
		{
			end_idex = 0;
			break;
		}
		if (memcmp(&srcbuf[*buf_idex], "\r\n", 2) == 0)
		{
			end_idex = 2;
			*buf_idex = *buf_idex + end_idex;
			break;	
		}
		if (srcbuf[*buf_idex] == 0)
		{
			AfxMessageBox("此数值为空，请检查", MB_OK);
			return 0;
		}
		*buf_idex = *buf_idex + 1;
	}
	if (idex == 30)
	{
		AfxMessageBox("数据获取出错，请检查", MB_OK);
		return 0;
	}
	memset(buf_oneline, 0, 100);
	memcpy(buf_oneline, &srcbuf[*buf_idex - idex - end_idex], idex);  //拷贝一行的数据
	
	//填充数据到变量中
	for (i = 0; i < 20; i++)
	{
		if (buf_oneline[i] == ',')
		{
			break;
		}
	}

	memset(buf_value, 0, 100);
	memcpy(buf_value, &buf_oneline[0], i);
	*sensor_addr = hex2int(buf_value);

	if (value_flag && 0 != buf_oneline[i+1])
	{
		memset(buf_value, 0, 100);
		memcpy(buf_value, &buf_oneline[i+1], idex - i);
		bb_value = hex2int(buf_value);
	}
	else
	{
		bb_value = 0;
	}
	
	
	return bb_value;
}

int Public_TextDlg::Get_value_by_buf(char *srcbuf, UINT *buf_idex , UINT buf_len)//, char *dstbuf)
{
    char buf_oneline[100] = {0};
	char buf_value[100] = {0};
	UINT addr = 0;
	int bb_value = 0;
	BOOL finish_flag = FALSE;
	UINT idex = 0, i = 0, buf_oneline_idex = 0;
	UINT end_idex = 2;
	
	if (buf_len < *buf_idex)
	{
		AfxMessageBox("结构体的长度比BUG的长度还大，请检查", MB_OK);
		return 0;
	}

	for (idex = 0; idex < 30; idex++)//读取一行
	{
		if (idex == 0)
		{
			if (memcmp(&srcbuf[*buf_idex], "\r\n", 2) == 0)
			{
				finish_flag = TRUE;
				break;	
			}
		}
		if (srcbuf[*buf_idex] == 0)
		{
			AfxMessageBox("此数值为空，请检查", MB_OK);
			return 0;
		}
		if(buf_len == *buf_idex)
		{
			end_idex = 0;
			break;
		}
		if (memcmp(&srcbuf[*buf_idex], "\r\n", 2) == 0)
		{
			end_idex = 2;
			*buf_idex = *buf_idex + end_idex;
			break;	
		}
		*buf_idex = *buf_idex + 1;
	}
	if (idex == 30 || idex == 0)
	{
		AfxMessageBox("数据获取出错，请检查", MB_OK);
		return 0;
	}
	memset(buf_oneline, 0, 100);
	memcpy(buf_oneline, &srcbuf[*buf_idex - idex - end_idex], idex);  //拷贝一行的数据
	
	//填充数据到变量中
	for (i = 0; i < 20; i++)
	{
		if (buf_oneline[i] == ',')
		{
			break;
		}
	}
	memset(buf_value, 0, 100);
	memcpy(buf_value, &buf_oneline[i+1], idex - i);
	
	bb_value = atoi(buf_value);

	return bb_value;
}

BOOL Public_TextDlg::decode_packet_BB(char *buf, UINT buf_len) 
{
	UINT i = 0;
	UINT len = 0;
	UINT text_buf_idex = 0;
	UINT Addr_idex = 0;
	


	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_blc, buf, buf_len);

	

	//显示数据
	memset(text_buf, 0, MAX_BUF_LEN);
	Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.blc_mode, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.m_blc.black_level_enable, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_r_a, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_r_offset, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_gr_a, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_gr_offset, MODE_10);	
	Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_gb_a, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_gb_offset, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_b_a, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_b_offset, MODE_10);

	for (i = 0; i < 9; i++)
	{
		//联动
		Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].black_level_enable, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_r_a, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_r_offset, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_gr_a, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_gr_offset, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_gb_a, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_gb_offset, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_b_a, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex,  BB_ADDR[Addr_idex++], Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_b_offset, MODE_10);
	}

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_BB(char *buf, UINT buf_len) 
{
	UINT i = 0, j = 0;
	UINT buf_idex = 0;

	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	
	Isp_init_param.p_Isp_blc.p_blc.blc_mode = Get_value_by_buf(buf, &buf_idex, buf_len); // //模式选择
	Isp_init_param.p_Isp_blc.p_blc.m_blc.black_level_enable = Get_value_by_buf(buf, &buf_idex, buf_len);////黑平衡的使能
	Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_r_a = Get_value_by_buf(buf, &buf_idex, buf_len); //atoi(buf_lsc_temp);
	Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_r_offset = Get_value_by_buf(buf, &buf_idex, buf_len);//atoi(buf_lsc_temp);	
	Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_gr_a = Get_value_by_buf(buf, &buf_idex, buf_len); //atoi(buf_lsc_temp);
	Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_gr_offset = Get_value_by_buf(buf, &buf_idex, buf_len); //atoi(buf_lsc_temp);
	Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_gb_a = Get_value_by_buf(buf, &buf_idex, buf_len); //atoi(buf_lsc_temp);
	Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_gb_offset = Get_value_by_buf(buf, &buf_idex, buf_len); //atoi(buf_lsc_temp);
	Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_b_a = Get_value_by_buf(buf, &buf_idex, buf_len); //atoi(buf_lsc_temp);
	Isp_init_param.p_Isp_blc.p_blc.m_blc.bl_b_offset = Get_value_by_buf(buf, &buf_idex, buf_len); //atoi(buf_lsc_temp);
	for (i = 0; i < 9; i++)
	{
		//联动
		Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].black_level_enable = Get_value_by_buf(buf, &buf_idex, buf_len);//黑平衡的使能
		Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_r_a = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_r_offset = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_gr_a = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_gr_offset = Get_value_by_buf(buf, &buf_idex, buf_len);	
		Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_gb_a = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_gb_offset = Get_value_by_buf(buf, &buf_idex, buf_len);	
		Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_b_a = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_blc.p_blc.linkage_blc[i].bl_b_offset = Get_value_by_buf(buf, &buf_idex, buf_len);
	}	


	return TRUE;
}

//镜头校正
BOOL Public_TextDlg::decode_packet_LSC(char *buf, UINT buf_len) 
{
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0;
	UINT len = strlen("\r\n");
	UINT text_buf_idex = 0, addr_temp = 0;
		
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_lsc, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	for (i = 0; i < 10; i++)
	{
		//显示数据
		Set_buf_info(text_buf, &text_buf_idex,  LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.lsc_r_coef.coef_b[i], MODE_10);

		Set_buf_info(text_buf, &text_buf_idex,  LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.lsc_r_coef.coef_c[i], MODE_10);

		Set_buf_info(text_buf, &text_buf_idex,  LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.lsc_gr_coef.coef_b[i], MODE_10);

		Set_buf_info(text_buf, &text_buf_idex,  LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.lsc_gr_coef.coef_c[i], MODE_10);
		
		Set_buf_info(text_buf, &text_buf_idex,  LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.lsc_gb_coef.coef_b[i], MODE_10);

		Set_buf_info(text_buf, &text_buf_idex,  LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.lsc_gb_coef.coef_c[i], MODE_10);

		Set_buf_info(text_buf, &text_buf_idex,  LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.lsc_b_coef.coef_b[i], MODE_10);

		Set_buf_info(text_buf, &text_buf_idex,  LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.lsc_b_coef.coef_c[i], MODE_10);

	}

	for (i = 0; i < 10; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.range[i], MODE_10);
	}
	Set_buf_info(text_buf, &text_buf_idex, LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.lsc_shift, MODE_10);

	Set_buf_info(text_buf, &text_buf_idex, LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.xref, MODE_10);

	Set_buf_info(text_buf, &text_buf_idex, LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.yref, MODE_10);

	Set_buf_info(text_buf, &text_buf_idex, LSC_ADDR[Addr_idex++], Isp_init_param.p_Isp_lsc.lsc.enable, MODE_10);

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_LSC(char *buf, UINT buf_len) 
{
	char buf_lsc_temp[3] = {0};
	char buf_temp[25] = {0};
	UINT i = 0, j = 0;
	UINT buf_idex = 0;

	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	
	for (i = 0; i < 10; i++)
	{
		Isp_init_param.p_Isp_lsc.lsc.lsc_r_coef.coef_b[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_lsc.lsc.lsc_r_coef.coef_c[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_lsc.lsc.lsc_gr_coef.coef_b[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_lsc.lsc.lsc_gr_coef.coef_c[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_lsc.lsc.lsc_gb_coef.coef_b[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_lsc.lsc.lsc_gb_coef.coef_c[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_lsc.lsc.lsc_b_coef.coef_b[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_lsc.lsc.lsc_b_coef.coef_c[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	
	for (i = 0; i < 10; i++)
	{
		Isp_init_param.p_Isp_lsc.lsc.range[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	Isp_init_param.p_Isp_lsc.lsc.lsc_shift = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_lsc.lsc.xref = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_lsc.lsc.yref = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_lsc.lsc.enable = Get_value_by_buf(buf, &buf_idex, buf_len);

	return TRUE;
}

//白平衡
BOOL Public_TextDlg::decode_packet_WB(char *buf, UINT buf_len) 
{
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0;
	UINT len = 0;
	UINT text_buf_idex = 0, addr_temp = 0;
		
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_wb, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = AWB_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.wb_type.wb_type, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_mwb.r_gain, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_mwb.g_gain, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_mwb.b_gain, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_mwb.r_offset, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_mwb.g_offset, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_mwb.b_offset, MODE_10);

	for (i = 0; i < 10; i++)
	{
		addr_temp = AWB_ADDR[Addr_idex++];
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.gr_low[i], MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.gr_high[i], MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.gb_low[i], MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.gb_high[i], MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.rb_low[i], MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.rb_high[i], MODE_10);
	}


	addr_temp = AWB_ADDR[Addr_idex++];
	
	for (i = 0; i < 16; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.g_weight[i], MODE_10);
	}

	addr_temp = AWB_ADDR[Addr_idex++];

	for (i = 0; i < 10; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.colortemp_envi[i], MODE_10);
	}

	addr_temp = AWB_ADDR[Addr_idex++];
	
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.y_low, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.y_high, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.err_est, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.auto_wb_step, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.total_cnt_thresh, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb.colortemp_stable_cnt_thresh, MODE_10);

	
	addr_temp = AWB_ADDR[Addr_idex++];

	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb_ex.awb_ex_ctrl_enable, MODE_10);

	for (i = 0; i < 10; i++)
	{
		addr_temp = AWB_ADDR[Addr_idex++];
		
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_max, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_min, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_max, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_min, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_max, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_min, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_ex, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_ex, MODE_10);
	}

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_WB(char *buf, UINT buf_len) 
{
	char buf_lsc_temp[3] = {0};
	char buf_temp[25] = {0};
	UINT i = 0, j = 0;
	UINT buf_idex = 0;

	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	Isp_init_param.p_Isp_wb.wb_type.wb_type = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wb.p_mwb.r_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wb.p_mwb.g_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wb.p_mwb.b_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wb.p_mwb.r_offset = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wb.p_mwb.g_offset = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wb.p_mwb.b_offset = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (i = 0; i < 10; i++)
	{
		Isp_init_param.p_Isp_wb.p_awb.gr_low[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb.gr_high[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb.gb_low[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb.gb_high[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb.rb_low[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb.rb_high[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (i = 0; i < 16; i++)
	{
		Isp_init_param.p_Isp_wb.p_awb.g_weight[i] = Get_value_by_buf(buf, &buf_idex, buf_len);//模式选择
	}

	for (i = 0; i < 10; i++)
	{
		Isp_init_param.p_Isp_wb.p_awb.colortemp_envi[i] = Get_value_by_buf(buf, &buf_idex, buf_len);//模式选择
	}

	Isp_init_param.p_Isp_wb.p_awb.y_low = Get_value_by_buf(buf, &buf_idex, buf_len);//模式选择
	Isp_init_param.p_Isp_wb.p_awb.y_high = Get_value_by_buf(buf, &buf_idex, buf_len); //模式选择
	Isp_init_param.p_Isp_wb.p_awb.err_est = Get_value_by_buf(buf, &buf_idex, buf_len); //模式选择
	Isp_init_param.p_Isp_wb.p_awb.auto_wb_step = Get_value_by_buf(buf, &buf_idex, buf_len);//模式选择
	Isp_init_param.p_Isp_wb.p_awb.total_cnt_thresh = Get_value_by_buf(buf, &buf_idex, buf_len); //模式选择
	Isp_init_param.p_Isp_wb.p_awb.colortemp_stable_cnt_thresh = Get_value_by_buf(buf, &buf_idex, buf_len); //模式选择

	Isp_init_param.p_Isp_wb.p_awb_ex.awb_ex_ctrl_enable = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (i = 0; i < 10; i++)
	{
		Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_max = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_min = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_max = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].ggain_min = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_max = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_min = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].rgain_ex = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wb.p_awb_ex.awb_ctrl[i].bgain_ex = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	
	return TRUE;
}

//颜色校正
BOOL Public_TextDlg::decode_packet_CCM(char *buf, UINT buf_len) 
{
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	UINT text_buf_idex =0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_ccm, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = CCM_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_ccm.p_ccm.cc_mode, MODE_10);
	
	addr_temp = CCM_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_ccm.p_ccm.manual_ccm.cc_enable, MODE_10);

	
	addr_temp = CCM_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_yth, MODE_10);
	addr_temp = CCM_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_gain, MODE_10);
	addr_temp = CCM_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_slop, MODE_10);

	for (m = 0; m < 3; m++)
	{
		for (n = 0; n < 3; n++)
		{
			addr_temp = CCM_ADDR[Addr_idex++];
			Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_ccm.p_ccm.manual_ccm.ccm[m][n], MODE_10);
		}
	}

	
	for (i = 0; i < 4; i++)
	{
		addr_temp = CCM_ADDR[Addr_idex++];
		Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_ccm.p_ccm.ccm[i].cc_enable, MODE_10);

		addr_temp = CCM_ADDR[Addr_idex++];
		Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_ccm.p_ccm.ccm[i].cc_cnoise_yth, MODE_10);
		addr_temp = CCM_ADDR[Addr_idex++];
		Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_ccm.p_ccm.ccm[i].cc_cnoise_gain, MODE_10);
		addr_temp = CCM_ADDR[Addr_idex++];
		Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_ccm.p_ccm.ccm[i].cc_cnoise_slop, MODE_10);

		for (m = 0; m < 3; m++)
		{
			for (n = 0; n < 3; n++)
			{
				addr_temp = CCM_ADDR[Addr_idex++];
				Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_ccm.p_ccm.ccm[i].ccm[m][n], MODE_10);
			}
		}

		
	}

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_CCM(char *buf, UINT buf_len) 
{
	UINT i = 0, m = 0, n = 0, buf_lsc_len = 0;
	UINT buf_idex = 0;

	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	Isp_init_param.p_Isp_ccm.p_ccm.cc_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_ccm.p_ccm.manual_ccm.cc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_yth = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_ccm.p_ccm.manual_ccm.cc_cnoise_slop = Get_value_by_buf(buf, &buf_idex, buf_len);
	
	for (m = 0; m < 3; m++)
	{
		for (n = 0; n < 3; n++)
		{
			Isp_init_param.p_Isp_ccm.p_ccm.manual_ccm.ccm[m][n] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
	}

	
	
	for (i = 0; i < 4; i++)
	{
		Isp_init_param.p_Isp_ccm.p_ccm.ccm[i].cc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_ccm.p_ccm.ccm[i].cc_cnoise_yth = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_ccm.p_ccm.ccm[i].cc_cnoise_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_ccm.p_ccm.ccm[i].cc_cnoise_slop = Get_value_by_buf(buf, &buf_idex, buf_len);
		
		for (m = 0; m < 3; m++)
		{
			for (n = 0; n < 3; n++)
			{
				Isp_init_param.p_Isp_ccm.p_ccm.ccm[i].ccm[m][n] = Get_value_by_buf(buf, &buf_idex, buf_len);
			}
		}

		
	}


	return TRUE;
}


//RAW GAMMA
BOOL Public_TextDlg::decode_packet_RAW_LUT(char *buf, UINT buf_len) 
{
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	UINT text_buf_idex =0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_raw_lut, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = RAW_LUT_ADDR[Addr_idex++];
	for (i = 0; i < 129; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp + i, Isp_init_param.p_Isp_raw_lut.raw_lut_p.raw_r[i], MODE_10);
	}

	addr_temp = RAW_LUT_ADDR[Addr_idex++];
	for (i = 0; i < 129; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp + i, Isp_init_param.p_Isp_raw_lut.raw_lut_p.raw_g[i], MODE_10);
	}

	addr_temp = RAW_LUT_ADDR[Addr_idex++];
	for (i = 0; i < 129; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp + i, Isp_init_param.p_Isp_raw_lut.raw_lut_p.raw_b[i], MODE_10);
	}

	Set_buf_info(text_buf, &text_buf_idex, addr_temp + i, Isp_init_param.p_Isp_raw_lut.raw_lut_p.raw_gamma_enable, MODE_10);

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_RAW_LUT(char *buf, UINT buf_len) 
{
	UINT i = 0, j = 0, m = 0, n = 0, buf_lsc_len = 0;
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	for (i = 0; i < 129; i++)
	{
		Isp_init_param.p_Isp_raw_lut.raw_lut_p.raw_r[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (i = 0; i < 129; i++)
	{
		Isp_init_param.p_Isp_raw_lut.raw_lut_p.raw_g[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (i = 0; i < 129; i++)
	{
		Isp_init_param.p_Isp_raw_lut.raw_lut_p.raw_b[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	Isp_init_param.p_Isp_raw_lut.raw_lut_p.raw_gamma_enable = Get_value_by_buf(buf, &buf_idex, buf_len);


	return TRUE;
}

//DEMOSAIC
BOOL Public_TextDlg::decode_packet_DEMO(char *buf, UINT buf_len) 
{
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	UINT text_buf_idex = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_demo, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = DEMO_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_demo.p_demo_attr.dm_HV_th, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_demo.p_demo_attr.dm_rg_thre, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_demo.p_demo_attr.dm_bg_thre, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_demo.p_demo_attr.dm_hf_th1, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_demo.p_demo_attr.dm_hf_th2, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_demo.p_demo_attr.dm_rg_gain, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_demo.p_demo_attr.dm_bg_gain, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_demo.p_demo_attr.dm_gr_gain, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_demo.p_demo_attr.dm_gb_gain, MODE_10);

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_DEMO(char *buf, UINT buf_len) 
{
	UINT i = 0, m = 0, n = 0;
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	Isp_init_param.p_Isp_demo.p_demo_attr.dm_HV_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_demo.p_demo_attr.dm_rg_thre = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_demo.p_demo_attr.dm_bg_thre = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_demo.p_demo_attr.dm_hf_th1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_demo.p_demo_attr.dm_hf_th2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_demo.p_demo_attr.dm_rg_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_demo.p_demo_attr.dm_bg_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_demo.p_demo_attr.dm_gr_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_demo.p_demo_attr.dm_gb_gain = Get_value_by_buf(buf, &buf_idex, buf_len);

	return TRUE;
}


//GAMMA
BOOL Public_TextDlg::decode_packet_GAMMA(char *buf, UINT buf_len) 
{
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	UINT text_buf_idex = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_gamma, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = GAMMA_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_gamma.p_gamma_attr.rgb_gamma_enable, MODE_10);

	addr_temp = GAMMA_ADDR[Addr_idex++];
	for (i = 0; i < 129; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gamma.p_gamma_attr.r_gamma[i], MODE_10);
	}

	addr_temp = GAMMA_ADDR[Addr_idex++];
	for (i = 0; i < 129; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gamma.p_gamma_attr.g_gamma[i], MODE_10);
	}

	addr_temp = GAMMA_ADDR[Addr_idex++];
	for (i = 0; i < 129; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gamma.p_gamma_attr.b_gamma[i], MODE_10);
	}

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_GAMMA(char *buf, UINT buf_len) 
{
	UINT i = 0,m = 0, n = 0;
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	Isp_init_param.p_Isp_gamma.p_gamma_attr.rgb_gamma_enable = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (i = 0; i < 129; i++)
	{
		Isp_init_param.p_Isp_gamma.p_gamma_attr.r_gamma[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	
	for (i = 0; i < 129; i++)
	{
		Isp_init_param.p_Isp_gamma.p_gamma_attr.g_gamma[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	
	for (i = 0; i < 129; i++)
	{
		Isp_init_param.p_Isp_gamma.p_gamma_attr.b_gamma[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	return TRUE;
}

//Y GAMMA
BOOL Public_TextDlg::decode_packet_Y_GAMMA(char *buf, UINT buf_len) 
{
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	UINT text_buf_idex = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_y_gamma, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = Y_GAMMA_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_enable, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_level, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth1, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth2, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_slop, MODE_10);
	Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_gain, MODE_10);

	addr_temp = Y_GAMMA_ADDR[Addr_idex++];
	for (i = 0; i < 129; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma[i], MODE_10);
	}

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_Y_GAMMA(char *buf, UINT buf_len) 
{
	UINT i = 0,m = 0, n = 0;
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_uv_adjust_level = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_yth2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_slop = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma_cnoise_gain = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (i = 0; i < 129; i++)
	{
		Isp_init_param.p_Isp_y_gamma.p_gamma_attr.ygamma[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	return TRUE;
}


//SHARP
BOOL Public_TextDlg::decode_packet_SHARP(char *buf, UINT buf_len) 
{
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	UINT text_buf_idex = 0;
	char *test_buf_temp = NULL;

	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_sharp, buf, buf_len);

	test_buf_temp = (char *)malloc(MAX_BUF_SHARP_LEN);
	if (test_buf_temp == NULL)
	{
		AfxMessageBox("内存分配失败", MB_OK);
		return FALSE;
	}
	memset(test_buf_temp, 0, MAX_BUF_SHARP_LEN);
	addr_temp = SHARP_ADDR[Addr_idex++];
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_sharp.p_sharp_attr.ysharp_mode, MODE_10);
	
	addr_temp = SHARP_ADDR[Addr_idex++];
	for(i = 0; i < 6; i++)
	{
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.mf_HPF[i], MODE_10);
	}

	addr_temp = SHARP_ADDR[Addr_idex++];
	for(i = 0; i < 3; i++)
	{
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.hf_HPF[i], MODE_10);
	}

	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_max_th, MODE_10);
	i++;

	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_min_th, MODE_10);
	i++;

	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_max_th, MODE_10);
	i++;

	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_min_th, MODE_10);
	i++;

	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_max_th, MODE_10);
	i++;

	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_min_th, MODE_10);
	i++;

	

	//手动manual_sharp_attr
	addr_temp = SHARP_ADDR[Addr_idex++];
	for (i = 0; i < 256; i++)
	{
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_HPF_LUT[i], MODE_10);
	}
	addr_temp = addr_temp + 256;
	for (i = 0; i < 256; i++)
	{
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_HPF_LUT[i], MODE_10);
	}
	addr_temp = addr_temp + 256;
	i = 0;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_k, MODE_10);
	i++;
	
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_shift, MODE_10);
	i++;
	
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_k, MODE_10);
	i++;
	
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_shift, MODE_10);
	i++;
	
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_method, MODE_10);
	i++;
	
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_th, MODE_10);
	i++;

	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_weaken, MODE_10);
	i++;

	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_detect_enable, MODE_10);
	i++;

	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.ysharp_enable, MODE_10);
	i++;
	
	//联动linkage_sharp_attr
	for (m = 0; m < 9; m++)
	{
		addr_temp = SHARP_ADDR[Addr_idex++];
		for (i = 0; i < 256; i++)
		{
			Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].MF_HPF_LUT[i], MODE_10);
		}
		addr_temp = addr_temp + 256;
		for (i = 0; i < 256; i++)
		{
			Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].HF_HPF_LUT[i], MODE_10);
		}
		addr_temp = addr_temp + 256;
		i = 0;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].mf_hpf_k, MODE_10);
		i++;
		
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].mf_hpf_shift, MODE_10);
		i++;

		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].hf_hpf_k, MODE_10);
		i++;

		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].hf_hpf_shift, MODE_10);
		i++;

		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].sharp_method, MODE_10);
		i++;

		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].sharp_skin_gain_th, MODE_10);
		i++;

		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].sharp_skin_gain_weaken, MODE_10);
		i++;

		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].sharp_skin_detect_enable, MODE_10);
		i++;

		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].ysharp_enable, MODE_10);
		i++;
	}


	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(test_buf_temp);//
	free(test_buf_temp);
	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_SHARP(char *buf, UINT buf_len) 
{
	UINT i = 0,  m = 0, n = 0;
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	Isp_init_param.p_Isp_sharp.p_sharp_attr.ysharp_mode = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (i = 0; i < 6; i++)
	{
		Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.mf_HPF[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	for (i = 0; i < 3; i++)
	{
		Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.hf_HPF[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	
	Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_max_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_min_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_max_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_v_min_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_max_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_ex_attr.sharp_skin_y_min_th = Get_value_by_buf(buf, &buf_idex, buf_len);

	//手动manual_sharp_attr
	for (i = 0; i < 256; i++)
	{
		Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.MF_HPF_LUT[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (i = 0; i < 256; i++)
	{
		Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.HF_HPF_LUT[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_k = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.mf_hpf_shift = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_k = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.hf_hpf_shift = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_method = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_gain_weaken = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.sharp_skin_detect_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_sharp.p_sharp_attr.manual_sharp_attr.ysharp_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	
	//联动linkage_sharp_attr
	for (m = 0; m < 9; m++)
	{
		for (i = 0; i < 256; i++)
		{
			Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].MF_HPF_LUT[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
		for (i = 0; i < 256; i++)
		{
			Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].HF_HPF_LUT[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
		Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].mf_hpf_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].mf_hpf_shift = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].hf_hpf_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].hf_hpf_shift = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].sharp_method = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].sharp_skin_gain_th = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].sharp_skin_gain_weaken = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].sharp_skin_detect_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_sharp.p_sharp_attr.linkage_sharp_attr[m].ysharp_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	return TRUE;
}

//Denoise
BOOL Public_TextDlg::decode_packet_NR(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_nr, buf, buf_len);
	memset(text_temp, 0, 20);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = NR_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_nr.p_nr1.nr1_mode, MODE_10);
	i++;

	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr2.nr2_mode, MODE_10);
	i++;

	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_enable, MODE_10);
	i++;

	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.nr2_enable, MODE_10);
	i++;

	//R
	//nr1_weight_rtbl[17];   //10bit
	addr_temp = NR_ADDR[Addr_idex++];
	for (i = 0; i <17; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_weight_rtbl[i], MODE_10);
	}
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_calc_r_k, MODE_10);

	//G
	addr_temp = NR_ADDR[Addr_idex++];
	for (i = 0; i <17; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_weight_gtbl[i], MODE_10);
	}
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_calc_g_k, MODE_10);

	//B
	addr_temp = NR_ADDR[Addr_idex++];
	for (i = 0; i <17; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_weight_btbl[i], MODE_10);
	}
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_calc_b_k, MODE_10);
	
	//nr_k
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+1, Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_k, MODE_10);


	addr_temp = NR_ADDR[Addr_idex++];
	for (i = 0; i <17; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_lc_lut[i], MODE_10);
	}

	addr_temp = NR_ADDR[Addr_idex++];
	for (i = 0; i < 17; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.nr2_weight_tbl[i], MODE_10);
	}
	//nr2_calc_y_k
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.nr2_calc_y_k, MODE_10);

	//Nr2_k
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+1, Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.nr2_k, MODE_10);

	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+2, Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.y_dpc_enable, MODE_10);

	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+3, Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.y_dpc_th, MODE_10);

	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+4, Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.y_black_dpc_enable, MODE_10);

	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+5, Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.y_white_dpc_enable, MODE_10);



	//联动
	for (m = 0; m < 9; m++)
	{
		addr_temp = NR_ADDR[Addr_idex++];
		Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_enable, MODE_10);
		
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+1, Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].nr2_enable, MODE_10);

		//NR1
		addr_temp = NR_ADDR[Addr_idex++];
		for (i = 0; i <17; i++)
		{
			Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_weight_rtbl[i], MODE_10);
		}
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_calc_r_k, MODE_10);

		addr_temp = NR_ADDR[Addr_idex++];
		for (i = 0; i <17; i++)
		{
			Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_weight_gtbl[i], MODE_10);
		}
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_calc_g_k, MODE_10);


		addr_temp = NR_ADDR[Addr_idex++];
		for (i = 0; i <17; i++)
		{
			Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_weight_btbl[i], MODE_10);
		}
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_calc_b_k, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+1, Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_k, MODE_10);

		addr_temp = NR_ADDR[Addr_idex++];
		for (i = 0; i <17; i++)
		{
			Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_lc_lut[i], MODE_10);
		}

		//NR2
		addr_temp = NR_ADDR[Addr_idex++];
		for (i = 0; i <17; i++)
		{
			Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].nr2_weight_tbl[i], MODE_10);
		}
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].nr2_calc_y_k, MODE_10);
		//Nr2_k
		
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+1, Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].nr2_k, MODE_10);

		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+2, Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].y_dpc_enable, MODE_10);

		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+3, Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].y_dpc_th, MODE_10);
		
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+4, Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].y_black_dpc_enable, MODE_10);

		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i+5, Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].y_white_dpc_enable, MODE_10);
		
	}

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_NR(char *buf, UINT buf_len) 
{
	UINT i = 0, j = 0, m = 0, n = 0, buf_lsc_len = 0;
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	Isp_init_param.p_Isp_nr.p_nr1.nr1_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_nr.p_nr2.nr2_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.nr2_enable = Get_value_by_buf(buf, &buf_idex, buf_len);

	//R
	//nr1_weight_rtbl[17];   //10bit
	for (i = 0; i <17; i++)
	{
		Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_weight_rtbl[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_calc_r_k = Get_value_by_buf(buf, &buf_idex, buf_len);

	//G
	for (i = 0; i <17; i++)
	{
		Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_weight_gtbl[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_calc_g_k = Get_value_by_buf(buf, &buf_idex, buf_len);

	//B
	for (i = 0; i <17; i++)
	{
		Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_weight_btbl[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_calc_b_k = Get_value_by_buf(buf, &buf_idex, buf_len);
	
	//nr_k
	Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_k = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (i = 0; i <17; i++)
	{
		Isp_init_param.p_Isp_nr.p_nr1.manual_nr1.nr1_lc_lut[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (i = 0; i < 17; i++)
	{
		Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.nr2_weight_tbl[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	//nr2_calc_y_k
	Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.nr2_calc_y_k = Get_value_by_buf(buf, &buf_idex, buf_len);

	//Nr2_k
	Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.nr2_k = Get_value_by_buf(buf, &buf_idex, buf_len);

	Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.y_dpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);

	Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.y_dpc_th = Get_value_by_buf(buf, &buf_idex, buf_len);

	Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.y_black_dpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_nr.p_nr2.manual_nr2.y_white_dpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	

	//联动
	for (m = 0; m < 9; m++)
	{
		Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].nr2_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		
		//NR1
		for (i = 0; i <17; i++)
		{
			Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_weight_rtbl[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}

		Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_calc_r_k = Get_value_by_buf(buf, &buf_idex, buf_len);

		for (i = 0; i <17; i++)
		{
			Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_weight_gtbl[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}

		Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_calc_g_k = Get_value_by_buf(buf, &buf_idex, buf_len);

		for (i = 0; i <17; i++)
		{
			Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_weight_btbl[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
		Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_calc_b_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_k = Get_value_by_buf(buf, &buf_idex, buf_len);

		for (i = 0; i <17; i++)
		{
			Isp_init_param.p_Isp_nr.p_nr1.linkage_nr1[m].nr1_lc_lut[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}

		//NR2
		for (i = 0; i <17; i++)
		{
			Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].nr2_weight_tbl[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
		Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].nr2_calc_y_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		//Nr2_k
		Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].nr2_k = Get_value_by_buf(buf, &buf_idex, buf_len);

		Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].y_dpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].y_dpc_th = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].y_black_dpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_nr.p_nr2.linkage_nr2[m].y_white_dpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	return TRUE;
}

//WDR
BOOL Public_TextDlg::decode_packet_WDR(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	char *test_buf_temp = NULL;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_wdr, buf, buf_len);

	test_buf_temp = (char *)malloc(MAX_BUF_SHARP_LEN);
	if (test_buf_temp == NULL)
	{
		AfxMessageBox("内存分配失败", MB_OK);
		return FALSE;
	}
	memset(test_buf_temp, 0, MAX_BUF_SHARP_LEN);
	
	memset(text_temp, 0, 20);

	addr_temp = WDR_ADDR[Addr_idex++];

	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_wdr.p_wdr_attr.wdr_mode, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_enable, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th1, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th2, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th3, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th4, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th5, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_enable, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_level, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain, MODE_10);
	i++;
	Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_slop, MODE_10);
	i++;

	addr_temp = WDR_ADDR[Addr_idex++];
	for (i = 0; i < 65; i++)
	{
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb1[i], MODE_10);
	}

	addr_temp = WDR_ADDR[Addr_idex++];
	for (i = 0; i < 65; i++)
	{
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb2[i], MODE_10);	
	}

	addr_temp = WDR_ADDR[Addr_idex++];
	for (i = 0; i < 65; i++)
	{
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb3[i], MODE_10);
	}

	addr_temp = WDR_ADDR[Addr_idex++];
	for (i = 0; i < 65; i++)
	{
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb4[i], MODE_10);
	}

	addr_temp = WDR_ADDR[Addr_idex++];
	for (i = 0; i < 65; i++)
	{
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb5[i], MODE_10);
	}

	addr_temp = WDR_ADDR[Addr_idex++];
	for (i = 0; i < 65; i++)
	{
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb6[i], MODE_10);
	}

	for (m=0; m<9; m++)
	{
		addr_temp = WDR_ADDR[Addr_idex++];
		i = 0;

		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_enable, MODE_10);
		i++;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_th1, MODE_10);
		i++;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_th2, MODE_10);
		i++;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_th3, MODE_10);
		i++;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_th4, MODE_10);
		i++;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_th5, MODE_10);
		i++;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_uv_adjust_enable, MODE_10);
		i++;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_uv_adjust_level, MODE_10);
		i++;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_cnoise_suppress_yth1, MODE_10);
		i++;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_cnoise_suppress_yth2, MODE_10);
		i++;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_cnoise_suppress_gain, MODE_10);
		i++;
		Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_cnoise_suppress_slop, MODE_10);
		i++;

		addr_temp = WDR_ADDR[Addr_idex++];
		for (i = 0; i < 65; i++)
		{
			Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb1[i], MODE_10);
		}

		addr_temp = WDR_ADDR[Addr_idex++];
		for (i = 0; i < 65; i++)
		{
			Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb2[i], MODE_10);	
		}

		addr_temp = WDR_ADDR[Addr_idex++];
		for (i = 0; i < 65; i++)
		{
			Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb3[i], MODE_10);
		}

		addr_temp = WDR_ADDR[Addr_idex++];
		for (i = 0; i < 65; i++)
		{
			Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb4[i], MODE_10);
		}

		addr_temp = WDR_ADDR[Addr_idex++];
		for (i = 0; i < 65; i++)
		{
			Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb5[i], MODE_10);
		}

		addr_temp = WDR_ADDR[Addr_idex++];
		for (i = 0; i < 65; i++)
		{
			Set_buf_info(test_buf_temp, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb6[i], MODE_10);
		}

	}


	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(test_buf_temp);//
	free(test_buf_temp);

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_WDR(char *buf, UINT buf_len) 
{
	UINT i = 0, m = 0, n = 0;
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	
	Isp_init_param.p_Isp_wdr.p_wdr_attr.wdr_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th3 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th4 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.wdr_th5 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_uv_adjust_level = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_yth2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.hdr_cnoise_suppress_slop = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (i = 0; i < 65; i++)
	{
		Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb1[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	
	for (i = 0; i < 65; i++)
	{
		Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb2[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	
	for (i = 0; i < 65; i++)
	{
		Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb3[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (i = 0; i < 65; i++)
	{
		Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb4[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (i = 0; i < 65; i++)
	{
		Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb5[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (i = 0; i < 65; i++)
	{
		Isp_init_param.p_Isp_wdr.p_wdr_attr.manual_wdr.area_tb6[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (m=0;  m<9; m++)
	{
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_th1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_th2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_th3 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_th4 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].wdr_th5 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_uv_adjust_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_uv_adjust_level = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_cnoise_suppress_yth1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_cnoise_suppress_yth2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_cnoise_suppress_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].hdr_cnoise_suppress_slop = Get_value_by_buf(buf, &buf_idex, buf_len);

		for (i = 0; i < 65; i++)
		{
			Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb1[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
		
		for (i = 0; i < 65; i++)
		{
			Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb2[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
		
		for (i = 0; i < 65; i++)
		{
			Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb3[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}

		for (i = 0; i < 65; i++)
		{
			Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb4[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}

		for (i = 0; i < 65; i++)
		{
			Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb5[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}

		for (i = 0; i < 65; i++)
		{
			Isp_init_param.p_Isp_wdr.p_wdr_attr.linkage_wdr[m].area_tb6[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}

	}

	return TRUE;
}

//FCS
BOOL Public_TextDlg::decode_packet_FCS(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;

	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_fcs, buf, buf_len);
	memset(text_temp, 0, 20);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = FCS_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_fcs.p_fcs.fcs_mode, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_fcs.p_fcs.manual_fcs.fcs_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_fcs.p_fcs.manual_fcs.fcs_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_fcs.p_fcs.manual_fcs.fcs_gain_slop, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_th, MODE_10);
	i++;

	
	for (m = 0; m < 9; m++)
	{
		addr_temp = FCS_ADDR[Addr_idex++];
		i = 0;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_fcs.p_fcs.linkage_fcs[m].fcs_enable, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_fcs.p_fcs.linkage_fcs[m].fcs_th, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_fcs.p_fcs.linkage_fcs[m].fcs_gain_slop, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_fcs.p_fcs.linkage_fcs[m].fcs_uv_nr_enable, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_fcs.p_fcs.linkage_fcs[m].fcs_uv_nr_th, MODE_10);

	}

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_FCS(char *buf, UINT buf_len) 
{
	UINT m = 0, n = 0;
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	Isp_init_param.p_Isp_fcs.p_fcs.fcs_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_fcs.p_fcs.manual_fcs.fcs_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_fcs.p_fcs.manual_fcs.fcs_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_fcs.p_fcs.manual_fcs.fcs_gain_slop = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_fcs.p_fcs.manual_fcs.fcs_uv_nr_th = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (m = 0; m < 9; m++)
	{
		Isp_init_param.p_Isp_fcs.p_fcs.linkage_fcs[m].fcs_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_fcs.p_fcs.linkage_fcs[m].fcs_th = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_fcs.p_fcs.linkage_fcs[m].fcs_gain_slop = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_fcs.p_fcs.linkage_fcs[m].fcs_uv_nr_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_fcs.p_fcs.linkage_fcs[m].fcs_uv_nr_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	return TRUE;
}

//YUV效果
BOOL Public_TextDlg::decode_packet_YUVEFFECT(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_effect, buf, buf_len);
	memset(text_temp, 0, 20);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = YUVEFFECT_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_effect.p_isp_effect.y_a, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_effect.p_isp_effect.y_b, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_effect.p_isp_effect.uv_a, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_effect.p_isp_effect.uv_b, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_effect.p_isp_effect.dark_margin_en, MODE_10);
	

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_YUVEFFECT(char *buf, UINT buf_len) 
{
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	Isp_init_param.p_Isp_effect.p_isp_effect.y_a = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_effect.p_isp_effect.y_b = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_effect.p_isp_effect.uv_a = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_effect.p_isp_effect.uv_b = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_effect.p_isp_effect.dark_margin_en = Get_value_by_buf(buf, &buf_idex, buf_len);

	return TRUE;
}

//饱和度
BOOL Public_TextDlg::decode_packet_SATURATION(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_saturation, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = SATURATION_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_saturation.p_se_attr.SE_mode, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_th1, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_th2, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_th3, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_th4, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop1, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop2, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_scale1, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_scale2, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_scale3, MODE_10);
	i++;
	
	for (i = 0; i < 9; i++)
	{
		addr_temp = SATURATION_ADDR[Addr_idex++];
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_enable, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th1, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th2, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th3, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th4, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop1, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop2, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale1, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale2, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp++, Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale3, MODE_10);
	}

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_SATURATION(char *buf, UINT buf_len) 
{
	char buf_lsc_temp[3] = {0};
	char buf_temp[25] = {0};
	UINT i = 0, j = 0, m = 0, n = 0, buf_lsc_len = 0;
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	Isp_init_param.p_Isp_saturation.p_se_attr.SE_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_th1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_th2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_th3 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_th4 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_scale_slop2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_scale1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_scale2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_saturation.p_se_attr.manual_sat.SE_scale3 = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (i = 0; i < 9; i++)
	{
		Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th3 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_th4 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale_slop2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_saturation.p_se_attr.linkage_sat[i].SE_scale3 = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	return TRUE;
}

//AF
BOOL Public_TextDlg::decode_packet_AF(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_af, buf, buf_len);
	memset(text_temp, 0, 20);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = AF_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win0_left, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win0_right, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win0_top, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win0_bottom, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_th, MODE_10);

	addr_temp = AF_ADDR[Addr_idex++];
	i = 0;
	
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win1_left, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win1_right, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win1_top, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win1_bottom, MODE_10);


	addr_temp = AF_ADDR[Addr_idex++];
	i = 0;
	
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win2_left, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win2_right, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win2_top, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win2_bottom, MODE_10);


	addr_temp = AF_ADDR[Addr_idex++];
	i = 0;
	
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win3_left, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win3_right, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win3_top, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win3_bottom, MODE_10);


	addr_temp = AF_ADDR[Addr_idex++];
	i = 0;
	
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win4_left, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win4_right, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win4_top, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_af.p_af_attr.af_win4_bottom, MODE_10);
	
		
	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_AF(char *buf, UINT buf_len) 
{
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	Isp_init_param.p_Isp_af.p_af_attr.af_win0_left = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win0_right = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win0_top = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win0_bottom = Get_value_by_buf(buf, &buf_idex, buf_len);

	Isp_init_param.p_Isp_af.p_af_attr.af_th = Get_value_by_buf(buf, &buf_idex, buf_len);

	Isp_init_param.p_Isp_af.p_af_attr.af_win1_left = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win1_right = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win1_top = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win1_bottom = Get_value_by_buf(buf, &buf_idex, buf_len);

	Isp_init_param.p_Isp_af.p_af_attr.af_win2_left = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win2_right = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win2_top = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win2_bottom = Get_value_by_buf(buf, &buf_idex, buf_len);

	Isp_init_param.p_Isp_af.p_af_attr.af_win3_left = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win3_right = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win3_top = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win3_bottom = Get_value_by_buf(buf, &buf_idex, buf_len);

	Isp_init_param.p_Isp_af.p_af_attr.af_win4_left = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win4_right = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win4_top = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_af.p_af_attr.af_win4_bottom = Get_value_by_buf(buf, &buf_idex, buf_len);
	
	

	return TRUE;
}

//权重系数
//zone weight
BOOL Public_TextDlg::decode_packet_WEIGHT(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_weight, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = WEIGHT_ADDR[Addr_idex++];
	for (m=0; m < 8; m++)
	{
		for (n = 0; n < 16; n++)
		{
			Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_weight.p_weight.zone_weight[m][n], MODE_10);
			i++;
		}
	}
	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_WEIGHT(char *buf, UINT buf_len) 
{
	char buf_lsc_temp[3] = {0};
	UINT i = 0, j = 0, m = 0, n = 0, buf_lsc_len = 0;
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	
	for (m=0; m < 8; m++)
	{
		for (n = 0; n < 16; n++)
		{
			Isp_init_param.p_Isp_weight.p_weight.zone_weight[m][n] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
	}

	return TRUE;
}

//绿平衡
BOOL Public_TextDlg::decode_packet_GB(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_gb, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = GB_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gb.p_gb.gb_mode, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gb.p_gb.manual_gb.gb_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gb.p_gb.manual_gb.gb_en_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gb.p_gb.manual_gb.gb_kstep, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gb.p_gb.manual_gb.gb_threshold, MODE_10);
	
	
	for (m = 0; m < 9; m++)
	{
		addr_temp = GB_ADDR[Addr_idex++];
		i = 0;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gb.p_gb.linkage_gb[m].gb_enable, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gb.p_gb.linkage_gb[m].gb_en_th, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gb.p_gb.linkage_gb[m].gb_kstep, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_gb.p_gb.linkage_gb[m].gb_threshold, MODE_10);
		i++;
	}

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//
	
	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_GB(char *buf, UINT buf_len) 
{
	char buf_lsc_temp[3] = {0};
	UINT i = 0, j = 0, m = 0, n = 0, buf_lsc_len = 0;
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	Isp_init_param.p_Isp_gb.p_gb.gb_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_gb.p_gb.manual_gb.gb_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_gb.p_gb.manual_gb.gb_en_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_gb.p_gb.manual_gb.gb_kstep = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_gb.p_gb.manual_gb.gb_threshold = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (m = 0; m < 9; m++)
	{
		Isp_init_param.p_Isp_gb.p_gb.linkage_gb[m].gb_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_gb.p_gb.linkage_gb[m].gb_en_th = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_gb.p_gb.linkage_gb[m].gb_kstep = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_gb.p_gb.linkage_gb[m].gb_threshold = Get_value_by_buf(buf, &buf_idex, buf_len);
	}


	return TRUE;
}

//杂项设置
BOOL Public_TextDlg::decode_packet_MISC(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_misc, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = MISC_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.vsync_pol, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.hsyn_pol, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.pclk_pol, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.test_pattern_en, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.test_pattern_cfg, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.cfa_mode, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.inputdataw, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.one_line_cycle, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.hblank_cycle, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.frame_start_delay_en, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.frame_start_delay_num, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.flip_en, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.mirror_en, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.twoframe_merge_en, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.mipi_line_end_sel, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.mipi_line_end_cnt_en_cfg, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_misc.p_misc.mipi_count_time, MODE_10);

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//
	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_MISC(char *buf, UINT buf_len) 
{
	char buf_lsc_temp[3] = {0};
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	
	Isp_init_param.p_Isp_misc.p_misc.vsync_pol = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.hsyn_pol = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.pclk_pol = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.test_pattern_en = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.test_pattern_cfg = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.cfa_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.inputdataw = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.one_line_cycle = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.hblank_cycle = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.frame_start_delay_en = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.frame_start_delay_num = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.flip_en = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.mirror_en = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.twoframe_merge_en = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.mipi_line_end_sel = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.mipi_line_end_cnt_en_cfg = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_misc.p_misc.mipi_count_time = Get_value_by_buf(buf, &buf_idex, buf_len);
	return TRUE;
}

//对比度
BOOL Public_TextDlg::decode_packet_CONSTRAST(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_contrast, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = CONSTRAST_ADDR[Addr_idex++];

	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_contrast.p_contrast.cc_mode, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_contrast.p_contrast.manual_contrast.y_contrast, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_contrast.p_contrast.manual_contrast.y_shift, MODE_10);

	for (m=0; m<9; m++)
	{
		addr_temp = CONSTRAST_ADDR[Addr_idex++];
		i = 0;

		Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_contrast.p_contrast.linkage_contrast[m].dark_pixel_area, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_contrast.p_contrast.linkage_contrast[m].dark_pixel_rate, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_contrast.p_contrast.linkage_contrast[m].shift_max, MODE_10);
	}
	
	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//
	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_CONSTRAST(char *buf, UINT buf_len) 
{
	char buf_lsc_temp[3] = {0};
	UINT buf_idex = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	Isp_init_param.p_Isp_contrast.p_contrast.cc_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_contrast.p_contrast.manual_contrast.y_contrast = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_contrast.p_contrast.manual_contrast.y_shift = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (int i=0; i<9; i++)
	{
		Isp_init_param.p_Isp_contrast.p_contrast.linkage_contrast[i].dark_pixel_area = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_contrast.p_contrast.linkage_contrast[i].dark_pixel_rate = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_contrast.p_contrast.linkage_contrast[i].shift_max = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	return TRUE;
}


//Expsoure
BOOL Public_TextDlg::decode_packet_EXP(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_exp, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = EXP_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_exp.p_exp_type.exp_type, MODE_10);
	
	addr_temp = EXP_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.exp_time_min, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.exp_time_max, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.a_gain_min, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.a_gain_max, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.d_gain_max, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.d_gain_min, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.isp_d_gain_min, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.isp_d_gain_max, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.target_lumiance, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.exp_stable_range, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.exp_step, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_raw_hist.enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_rgb_hist.enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_yuv_hist.enable, MODE_10);

	addr_temp = EXP_ADDR[Addr_idex++];
	i = 0;
	if (Isp_init_param.p_Isp_exp.p_frame_rate.hight_light_frame_rate & 0xffffff00)
	{
		AK_ISP_FRAME_RATE_ATTR_EX* fr_ex = (AK_ISP_FRAME_RATE_ATTR_EX*)&Isp_init_param.p_Isp_exp.p_frame_rate;
		
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, 3, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, fr_ex->fps1, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, fr_ex->max_exp_time1, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, fr_ex->gain12, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, fr_ex->fps2, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, fr_ex->max_exp_time2, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, fr_ex->gain22, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, fr_ex->fps3, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, fr_ex->max_exp_time2, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, fr_ex->gain31, MODE_10);
	}
	else
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, 2, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_frame_rate.hight_light_frame_rate, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_frame_rate.hight_light_max_exp_time, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_frame_rate.hight_light_to_low_light_gain, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_frame_rate.low_light_frame_rate, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_frame_rate.low_light_max_exp_time, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_frame_rate.low_light_to_hight_light_gain, MODE_10);
	}
	
	addr_temp = EXP_ADDR[Addr_idex++];

	for (m = 0; m<10; m++)
	{	
		for (n=0; n<2; n++)
		{		
			i = 16 * m + n;
			Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.envi_gain_range[m][n], MODE_10);
		}
	}

	addr_temp = EXP_ADDR[Addr_idex++];

	for (m=0; m<16; m++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+m, Isp_init_param.p_Isp_exp.p_ae.hist_weight[m], MODE_10);
	}

	addr_temp = EXP_ADDR[Addr_idex++];
	i = 0;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_exp.p_ae.OE_suppress_en, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.OE_detect_scope, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.OE_rate_max, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_exp.p_ae.OE_rate_min, MODE_10);


	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//
	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_EXP(char *buf, UINT buf_len) 
{
	UINT buf_idex = 0;
	int fps_num = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	Isp_init_param.p_Isp_exp.p_exp_type.exp_type = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.exp_time_min = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.exp_time_max = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.a_gain_min = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.a_gain_max = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.d_gain_max = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.d_gain_min = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.isp_d_gain_min = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.isp_d_gain_max = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.target_lumiance = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.exp_stable_range = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.exp_step = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_raw_hist.enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_rgb_hist.enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_yuv_hist.enable = Get_value_by_buf(buf, &buf_idex, buf_len);

	fps_num = Get_value_by_buf(buf, &buf_idex, buf_len);

	if (3 == fps_num)
	{
		AK_ISP_FRAME_RATE_ATTR_EX* fr_ex = (AK_ISP_FRAME_RATE_ATTR_EX*)&Isp_init_param.p_Isp_exp.p_frame_rate;
		fr_ex->fps1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		fr_ex->max_exp_time1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		fr_ex->gain12 = Get_value_by_buf(buf, &buf_idex, buf_len);
		fr_ex->fps2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		fr_ex->max_exp_time2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		fr_ex->gain22 = Get_value_by_buf(buf, &buf_idex, buf_len);
		fr_ex->fps3 = Get_value_by_buf(buf, &buf_idex, buf_len);
		fr_ex->max_exp_time3 = Get_value_by_buf(buf, &buf_idex, buf_len);
		fr_ex->gain31 = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	else
	{
		Isp_init_param.p_Isp_exp.p_frame_rate.hight_light_frame_rate = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_exp.p_frame_rate.hight_light_max_exp_time = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_exp.p_frame_rate.hight_light_to_low_light_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_exp.p_frame_rate.low_light_frame_rate = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_exp.p_frame_rate.low_light_max_exp_time = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_exp.p_frame_rate.low_light_to_hight_light_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	
	for (int m = 0; m<10; m++)
	{	
		for (int n=0; n<2; n++)
		{	
			Isp_init_param.p_Isp_exp.p_ae.envi_gain_range[m][n] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
	}

	for (m=0; m<16; m++)
	{
		Isp_init_param.p_Isp_exp.p_ae.hist_weight[m] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	Isp_init_param.p_Isp_exp.p_ae.OE_suppress_en = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.OE_detect_scope = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.OE_rate_max = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_exp.p_ae.OE_rate_min = Get_value_by_buf(buf, &buf_idex, buf_len);

	return TRUE;
}

/*
//EDGE
BOOL Public_TextDlg::decode_packet_EDGE(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_edge, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = EDGE_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.edge_mode, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_max_len, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_gain_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_gain, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_gain_slop, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_y_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.c_edge_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_skin_detect, MODE_10);
	
	//联动
	
	for (m = 0; m < 9; m++)
	{
		addr_temp = EDGE_ADDR[Addr_idex++];
		i = 0;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].enable, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_th, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_max_len, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_gain_th, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_gain, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_gain_slop, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_y_th, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].c_edge_enable, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_skin_detect, MODE_10);

	}
	addr_temp = EDGE_ADDR[Addr_idex++];
	i = 0;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_max_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_min_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_uv_max_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_uv_min_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_y_max_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_y_min_th, MODE_10);

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//
	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_EDGE(char *buf, UINT buf_len) 
{
	UINT buf_idex = 0;
	UINT m = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	Isp_init_param.p_Isp_edge.p_edge_attr.edge_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_max_len = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_gain_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_gain_slop = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_y_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.c_edge_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_attr.manual_edge.edge_skin_detect = Get_value_by_buf(buf, &buf_idex, buf_len);
	
	//联动
	
	for (m = 0; m < 9; m++)
	{
		Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_th = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_max_len = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_gain_th = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_gain = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_gain_slop = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_y_th = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].c_edge_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_edge.p_edge_attr.linkage_edge[m].edge_skin_detect = Get_value_by_buf(buf, &buf_idex, buf_len);
		
	}
	Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_max_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_min_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_uv_max_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_uv_min_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_y_max_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_edge.p_edge_ex_attr.edge_skin_y_min_th = Get_value_by_buf(buf, &buf_idex, buf_len);

		

	return TRUE;
}
*/

//
BOOL Public_TextDlg::decode_packet_3DNR(char *buf, UINT buf_len) 
{
	
	UINT text_buf_idex = 0;
	char text_temp[20] = {0};
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_3dnr, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = NR_3DNR_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.isp_3d_nr_mode, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_min_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_y_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_uv_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_y, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_uv, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_refFrame_format, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.y_2dnr_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_2dnr_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvnr_k, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvlp_k, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_k, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_minstep, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th1, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th2, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k1, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k2, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_slop, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mc_k, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_ac_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_calc_k, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_k, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_diff_shift, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.ylp_k, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_th1, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k1, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k2, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_kslop, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_minstep, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th1, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th2, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k1, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k2, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_slop, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mc_k, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_ac_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.md_th, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_t_y_ex_k_cfg, MODE_10);
	i++;

	for (n=0; n<17; n++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_weight_tbl[n], MODE_10);
		i++;
	}
	
	for (m = 0; m < 9; m++)
	{
		addr_temp = NR_3DNR_ADDR[Addr_idex++];
		i = 0;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].uv_min_enable, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].tnr_y_enable, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].tnr_uv_enable, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].updata_ref_y, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].updata_ref_uv, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].tnr_refFrame_format, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].y_2dnr_enable, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].uv_2dnr_enable, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].uvnr_k, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].uvlp_k, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_k, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_minstep, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_mf_th1, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_mf_th2, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_diffth_k1, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_diffth_k2, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_diffth_slop, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_mc_k, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_ac_th, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].ynr_calc_k, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].ynr_k, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].ynr_diff_shift, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].ylp_k, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_th1, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_k1, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_k2, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_kslop, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_minstep, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_mf_th1, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_mf_th2, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_diffth_k1, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_diffth_k2, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_diffth_slop, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_mc_k, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_ac_th, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].md_th, MODE_10);
		i++;
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].tnr_t_y_ex_k_cfg, MODE_10);
		i++;
		
		for (n=0; n<17; n++)
		{
			Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].ynr_weight_tbl[n], MODE_10);
			i++;
		}
	}
	

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//
	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_3DNR(char *buf, UINT buf_len) 
{
	UINT buf_idex = 0;
	UINT m = 0, n = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	Isp_init_param.p_Isp_3dnr.p_3d_nr.isp_3d_nr_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_min_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_y_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_uv_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_y = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.updata_ref_uv = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_refFrame_format = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.y_2dnr_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.uv_2dnr_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvnr_k = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.uvlp_k = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_k = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_minstep = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mf_th2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_k2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_diffth_slop = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_mc_k = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_uv_ac_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_calc_k = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_k = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_diff_shift = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.ylp_k = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_th1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_k2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_kslop = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_minstep = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mf_th2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k1 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_k2 = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_diffth_slop = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_mc_k = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.t_y_ac_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.md_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.tnr_t_y_ex_k_cfg = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (n=0; n<17; n++)
	{
		Isp_init_param.p_Isp_3dnr.p_3d_nr.manual_3d_nr.ynr_weight_tbl[n] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (m = 0; m < 9; m++)
	{
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].uv_min_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].tnr_y_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].tnr_uv_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].updata_ref_y = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].updata_ref_uv = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].tnr_refFrame_format = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].y_2dnr_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].uv_2dnr_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].uvnr_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].uvlp_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_minstep = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_mf_th1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_mf_th2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_diffth_k1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_diffth_k2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_diffth_slop = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_mc_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_uv_ac_th = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].ynr_calc_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].ynr_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].ynr_diff_shift = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].ylp_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_th1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_k1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_k2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_kslop = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_minstep = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_mf_th1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_mf_th2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_diffth_k1 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_diffth_k2 = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_diffth_slop = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_mc_k = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].t_y_ac_th = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].md_th = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].tnr_t_y_ex_k_cfg = Get_value_by_buf(buf, &buf_idex, buf_len);

		for (n=0; n<17; n++)
		{
			Isp_init_param.p_Isp_3dnr.p_3d_nr.linkage_3d_nr[m].ynr_weight_tbl[n] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
	}

	return TRUE;
}


//坏点校正
BOOL Public_TextDlg::decode_packet_DPC(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_dpc, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = DPC_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_dpc.p_ddpc.ddpc_mode, MODE_10);

	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_th, MODE_10);
	
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_dpc.p_ddpc.manual_ddpc.white_dpc_enable, MODE_10);
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_dpc.p_ddpc.manual_ddpc.black_dpc_enable, MODE_10);

	for (i=0; i<9; i++)
	{
		addr_temp = DPC_ADDR[Addr_idex++];
		Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_enable, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+1, Isp_init_param.p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_th, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+2, Isp_init_param.p_Isp_dpc.p_ddpc.linkage_ddpc[i].white_dpc_enable, MODE_10);
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+3, Isp_init_param.p_Isp_dpc.p_ddpc.linkage_ddpc[i].black_dpc_enable, MODE_10);
	}
	
	addr_temp = DPC_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp, Isp_init_param.p_Isp_dpc.p_sdpc.sdpc_enable, MODE_10);
	addr_temp = DPC_ADDR[Addr_idex++];
	for (i = 0; i < 1024; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_dpc.p_sdpc.sdpc_table[i], MODE_10);
	}
	
	
	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//
	
	
	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_DPC(char *buf, UINT buf_len) 
{
	UINT buf_idex = 0;
	UINT i = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	
	Isp_init_param.p_Isp_dpc.p_ddpc.ddpc_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_dpc.p_ddpc.manual_ddpc.ddpc_th = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_dpc.p_ddpc.manual_ddpc.white_dpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_dpc.p_ddpc.manual_ddpc.black_dpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	
	for (i=0; i<9; i++)
	{
		Isp_init_param.p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_dpc.p_ddpc.linkage_ddpc[i].ddpc_th = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_dpc.p_ddpc.linkage_ddpc[i].white_dpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
		Isp_init_param.p_Isp_dpc.p_ddpc.linkage_ddpc[i].black_dpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	Isp_init_param.p_Isp_dpc.p_sdpc.sdpc_enable = Get_value_by_buf(buf, &buf_idex, buf_len);

	for (i = 0; i < 1024; i++)
	{
		Isp_init_param.p_Isp_dpc.p_sdpc.sdpc_table[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}
	
	return TRUE;
}

//RgbTOYUV
BOOL Public_TextDlg::decode_packet_RGBTOYUV(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	
	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_rgb2yuv, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = RGBTOYUV_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_rgb2yuv.p_rgb2yuv.mode, MODE_10);

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//
	return TRUE;
}
 
BOOL Public_TextDlg::Get_packetData_RGBTOYUV(char *buf, UINT buf_len) 
{
	UINT buf_idex = 0;
	UINT i = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	
	Isp_init_param.p_Isp_rgb2yuv.p_rgb2yuv.mode = Get_value_by_buf(buf, &buf_idex, buf_len);

	return TRUE;
}


BOOL Public_TextDlg::decode_packet_sensor(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0;
	UINT len = 0;
	UINT addr_temp = 0;
	char ch_len = 0;

	if (buf_len == 0)
	{
		return FALSE;
	}

	memcpy(&m_sensor_num, buf, 4);

	if (NULL != Isp_init_param.p_Isp_sensor.p_sensor)
	{
		free(Isp_init_param.p_Isp_sensor.p_sensor);
	}

	Isp_init_param.p_Isp_sensor.p_sensor = (AK_ISP_SENSOR_ATTR*)malloc(sizeof(AK_ISP_SENSOR_ATTR) * m_sensor_num);
	
	if (NULL == Isp_init_param.p_Isp_sensor.p_sensor)
	{
		AfxMessageBox("内存申请失败！", MB_OK);
		return FALSE;
	}

	idex = 4;

	//解析所有数据
	for (i =0; i < m_sensor_num; i++)
	{
		memcpy(&ch_len, &buf[idex], 1);
		idex = idex + 1;
		memcpy(&Isp_init_param.p_Isp_sensor.p_sensor[i].sensor_addr, &buf[idex], ch_len);
		idex = idex + ch_len;
		memcpy(&ch_len, &buf[idex], 1);
		idex = idex + 1;
		memcpy(&Isp_init_param.p_Isp_sensor.p_sensor[i].sensor_value, &buf[idex], ch_len);
		idex = idex + ch_len;
	}

	memset(text_buf, 0, MAX_BUF_LEN);
	for (i =0; i < m_sensor_num; i++)
	{
		Set_buf_info(text_buf, &text_buf_idex, Isp_init_param.p_Isp_sensor.p_sensor[i].sensor_addr, Isp_init_param.p_Isp_sensor.p_sensor[i].sensor_value, MODE_16);
	}


	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return FALSE;
}

BOOL Public_TextDlg::Get_packetData_sensor(char *buf, UINT buf_len) 
{
	UINT buf_idex = 0;
	UINT i = 0;
	T_U16 addr;
	
	if (buf_len == 0 || m_sensor_num == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}

	for (i = 0; i < m_sensor_num; i++)
	{
		Isp_init_param.p_Isp_sensor.p_sensor[i].sensor_value = Get_value_by_buf_for_sensor(buf, &buf_idex, buf_len, &addr, TRUE);
		Isp_init_param.p_Isp_sensor.p_sensor[i].sensor_addr = addr;
	}

	return TRUE;
}

BOOL Public_TextDlg::decode_packet_hue(char *buf, UINT buf_len) 
{
	UINT text_buf_idex = 0;
	UINT idex = 0, Addr_idex = 0, i = 0, m = 0, n = 0, j = 0;
	UINT len = 0;
	UINT addr_temp = 0;

	//解析所有数据
	memcpy(&Isp_init_param.p_Isp_hue, buf, buf_len);
	memset(text_buf, 0, MAX_BUF_LEN);
	addr_temp = HUE_ADDR[Addr_idex++];
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_hue.p_hue.hue_mode, MODE_10);
	
	i++;
	Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_hue.p_hue.manual_hue.hue_sat_en, MODE_10);
	
	i = 0;
	addr_temp = HUE_ADDR[Addr_idex++];
	for (j = 0; j < 65; j++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_hue.p_hue.manual_hue.hue_lut_a[j], MODE_10);
		i++;
	}

	i = 0;
	addr_temp = HUE_ADDR[Addr_idex++];
	for (j = 0; j < 65; j++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_hue.p_hue.manual_hue.hue_lut_b[j], MODE_10);
		i++;
	}

	i = 0;
	addr_temp = HUE_ADDR[Addr_idex++];
	for (j = 0; j < 65; j++)
	{
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_hue.p_hue.manual_hue.hue_lut_s[j], MODE_10);
		i++;
	}


	for (m = 0; m < 4; m++)
	{
		i = 0;
		addr_temp = HUE_ADDR[Addr_idex++];
		Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_hue.p_hue.hue[m].hue_sat_en, MODE_10);

		i = 0;
		addr_temp = HUE_ADDR[Addr_idex++];
		for (j = 0; j < 65; j++)
		{
			Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_hue.p_hue.hue[m].hue_lut_a[j], MODE_10);
			i++;
		}
		
		i = 0;
		addr_temp = HUE_ADDR[Addr_idex++];
		for (j = 0; j < 65; j++)
		{
			Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_hue.p_hue.hue[m].hue_lut_b[j], MODE_10);
			i++;
		}
		
		i = 0;
		addr_temp = HUE_ADDR[Addr_idex++];
		for (j = 0; j < 65; j++)
		{
			Set_buf_info(text_buf, &text_buf_idex, addr_temp+i, Isp_init_param.p_Isp_hue.p_hue.hue[m].hue_lut_s[j], MODE_10);
			i++;
		}
	}

	GetDlgItem(IDC_EDIT_TEXT)->SetWindowText(text_buf);//

	return TRUE;
}

BOOL Public_TextDlg::Get_packetData_hue(char *buf, UINT buf_len) 
{
	UINT buf_idex = 0;
	UINT i = 0, j = 0;
	
	if (buf_len == 0)
	{
		AfxMessageBox("文本输入框内容不能为空", MB_OK);
		return FALSE;
	}
	
	Isp_init_param.p_Isp_hue.p_hue.hue_mode = Get_value_by_buf(buf, &buf_idex, buf_len);
	Isp_init_param.p_Isp_hue.p_hue.manual_hue.hue_sat_en = Get_value_by_buf(buf, &buf_idex, buf_len);
	
	for (i=0; i<65; i++)
	{
		Isp_init_param.p_Isp_hue.p_hue.manual_hue.hue_lut_a[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (i=0; i<65; i++)
	{
		Isp_init_param.p_Isp_hue.p_hue.manual_hue.hue_lut_b[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for (i=0; i<65; i++)
	{
		Isp_init_param.p_Isp_hue.p_hue.manual_hue.hue_lut_s[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
	}

	for(j = 0; j < 4; j++)
	{
		Isp_init_param.p_Isp_hue.p_hue.hue[j].hue_sat_en = Get_value_by_buf(buf, &buf_idex, buf_len);
		
		for (i=0; i<65; i++)
		{
			Isp_init_param.p_Isp_hue.p_hue.hue[j].hue_lut_a[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
		
		for (i=0; i<65; i++)
		{
			Isp_init_param.p_Isp_hue.p_hue.hue[j].hue_lut_b[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
		
		for (i=0; i<65; i++)
		{
			Isp_init_param.p_Isp_hue.p_hue.hue[j].hue_lut_s[i] = Get_value_by_buf(buf, &buf_idex, buf_len);
		}
	}

	return TRUE;
}

UINT Public_TextDlg::Get_text_sensor_info(void)
{
	char buf[MAX_BUF_SHARP_LEN] = {0};
	UINT finish_flag = 0;
	UINT buf_idex = 0, i = 0;
	T_U16 addr = 0;

	memset(buf, 0, MAX_BUF_SHARP_LEN);
	if (!Get_text_info(buf))
	{
		return 0;
	}

	m_sensor_num = Get_num_by_buf_for_sensor(buf, &buf_idex, strlen(buf));
	if(m_sensor_num == 0)
	{
		return 0;
	}
	
	if (Isp_init_param.p_Isp_sensor.p_sensor)
	{
		free(Isp_init_param.p_Isp_sensor.p_sensor);
		Isp_init_param.p_Isp_sensor.p_sensor = NULL;
	}

	Isp_init_param.p_Isp_sensor.p_sensor = (AK_ISP_SENSOR_ATTR *)malloc(sizeof(AK_ISP_SENSOR_ATTR) * m_sensor_num);
	memset(Isp_init_param.p_Isp_sensor.p_sensor, 0, sizeof(AK_ISP_SENSOR_ATTR) * m_sensor_num);

	//把地址获取出来
	buf_idex = 0;
	for (i = 0; i < m_sensor_num; i++)
	{
		Isp_init_param.p_Isp_sensor.p_sensor[i].sensor_value = Get_value_by_buf_for_sensor(buf, &buf_idex, strlen(buf), &addr, TRUE);
		Isp_init_param.p_Isp_sensor.p_sensor[i].sensor_addr = addr;
	}

	return 1;
}


void Public_TextDlg::OnButtonGet() 
{
	//SetDlgItemText(IDC_EDIT_TEXT, "");

	if (NULL == m_pMessageWnd) 
	{
		return;
	}
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO_FOR_TEXT, 0, 0);
}

UINT Public_TextDlg::Get_text_info(char *buf)
{
	TCHAR *text_buf = NULL;
	CString str;
	UINT len = 0;

	
	USES_CONVERSION;

	text_buf = (char *)malloc(MAX_BUF_SHARP_LEN);
	if (text_buf == NULL)
	{
		AfxMessageBox("内存分配失败,请检查", MB_OK);
		return 0;
	}
	
	//读取文本数据
	GetDlgItemText(IDC_EDIT_TEXT, str);
	if (str.IsEmpty())
	{
		AfxMessageBox("文本输入框内容不能为空,请检查", MB_OK);
		return 0;
	}
	
	_tcscpy(text_buf, str);
	len = _tcslen(text_buf);
	memcpy(buf, T2A(text_buf), len);
	free(text_buf);

	return len;
}



void Public_TextDlg::OnButton_Enable(T_DIALOG_ID pageID) 
{
	if (pageID == DIALOG_STAT)
	{
		GetDlgItem(IDC_BUTTON_GET)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_SET)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_TEXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_READ)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_WRITE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_COPY1)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_COPY2)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_GET)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_SET)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_TEXT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_READ)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_WRITE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_COPY1)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_COPY2)->EnableWindow(TRUE);
	}
}


void Public_TextDlg::OnButtonSet() 
{
	if (NULL == m_pMessageWnd) 
	{
		return;
	}

	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION_FOR_TEXT, 0, 0);

}


void Public_TextDlg::OnButtonSet_Enable(BOOL enable) 
{
	if (enable)
	{
		GetDlgItem(IDC_BUTTON_SET)->EnableWindow(TRUE);
	} 
	else
	{
		GetDlgItem(IDC_BUTTON_SET)->EnableWindow(FALSE);
	}
}


void Public_TextDlg::OnButtonGet_Enable(BOOL enable) 
{
	if (enable)
	{
		GetDlgItem(IDC_BUTTON_GET)->EnableWindow(TRUE);
	} 
	else
	{
		GetDlgItem(IDC_BUTTON_GET)->EnableWindow(FALSE);
	}
}

void Public_TextDlg::OnButtonRead() 
{
	// TODO: Add your control notification handler code here
	SetDlgItemText(IDC_EDIT_TEXT, "");
	
	if (NULL == m_pMessageWnd) 
	{
		return;
	}

	CBasePage::SendPageMessage(m_pMessageWnd, WM_READ_FOR_TEXT, 0, 0);
}

void Public_TextDlg::OnButtonWrite() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) 
	{
		return;
	}

	CBasePage::SendPageMessage(m_pMessageWnd, WM_SAVE_FOR_TEXT, 0, 0);
}

void Public_TextDlg::OnButtonCopy1() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) 
	{
		return;
	}

	CBasePage::SendPageMessage(m_pMessageWnd, WM_COPY_UI_TO_TEXT, 0, 0);
}

void Public_TextDlg::OnButtonCopy2() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) 
	{
		return;
	}

	CBasePage::SendPageMessage(m_pMessageWnd, WM_COPY_TEXT_TO_UI, 0, 0);
}
