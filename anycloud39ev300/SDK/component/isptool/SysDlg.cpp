// SysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISPCTRL_TOOL.h"
#include "SysDlg.h"
#include "netctrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SENSOR_INI_FILE "./sensor.ini"

UINT hex2int(const char *str)
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

/////////////////////////////////////////////////////////////////////////////
// CSysDlg dialog


CSysDlg::CSysDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSysDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSysDlg)
	m_addrstr = _T("0");
	m_valuestr = _T("0");
	m_Version = _T("");
	m_Notes = _T("");
	m_sensorIdStr = _T("");
	//}}AFX_DATA_INIT
	m_addr = 0;
	m_value = 0;
	m_timer = 0;
	m_style_id = 0;
	m_sensor_id = 0;
}


void CSysDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSysDlg)
	DDX_Text(pDX, IDC_EDIT1, m_addrstr);
	DDX_Text(pDX, IDC_EDIT2, m_valuestr);
	DDX_Text(pDX, IDC_EDIT3, m_Version);
	DDV_MaxChars(pDX, m_Version, 15);
	DDX_Text(pDX, IDC_EDIT4, m_Notes);
	DDV_MaxChars(pDX, m_Notes, 383);
	DDX_Text(pDX, IDC_EDIT5, m_style_id);
	DDX_Text(pDX, IDC_EDIT6, m_sensorIdStr);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSysDlg, CDialog)
	//{{AFX_MSG_MAP(CSysDlg)
	ON_BN_CLICKED(IDC_BUTTON_GET_SEN, OnButtonGetSen)
	ON_BN_CLICKED(IDC_BUTTON_SET_SEN, OnButtonSetSen)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnButtonClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSysDlg message handlers
BOOL CSysDlg::OnInitDialog()
{

	CDialog::OnInitDialog();


	return TRUE;
}

BOOL CSysDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if ((pMsg->wParam == VK_RETURN) 
			&& (GetDlgItem(IDC_EDIT4)!=GetFocus()))
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CSysDlg::SetSensorId(T_U32 sensorId)
{
	m_sensor_id = sensorId;
	m_sensorIdStr.Format("%x", m_sensor_id);
}

T_U32 CSysDlg::GetSensorId(void)
{
	UpdateData(TRUE);

	m_sensor_id = hex2int(m_sensorIdStr);
	
	return m_sensor_id;
}

void CSysDlg::SetVersion(char *str)
{
	m_Version.Format("%s", str);

	if (0 == m_timer)
	{
		m_timer = SetTimer(1, 10, NULL);
	}
}

void CSysDlg::GetVersion(char *buf)
{
	UpdateData(TRUE);
	strncpy(buf, (LPSTR)(LPCTSTR)m_Version, CFG_FILE_VERSION_LEN);
}

void CSysDlg::SetNotes(char *str)
{
	m_Notes.Format("%s", str);

	if (0 == m_timer)
	{
		m_timer = SetTimer(1, 10, NULL);
	}
}

bool CSysDlg::GetNotes(char *buf)
{
	UpdateData(TRUE);
	strncpy(buf, (LPSTR)(LPCTSTR)m_Notes, CFG_FILE_NOTES_LEN);
	return TRUE;
}

T_U8 CSysDlg::GetStyleId(void)
{
	UpdateData(TRUE);
	
	return m_style_id;
}

void CSysDlg::SetStyleId(T_U8 styleId)
{
	m_style_id = styleId;

	if (0 == m_timer)
	{
		m_timer = SetTimer(1, 10, NULL);
	}
}


void CSysDlg::GetDataValue(void)
{
	UpdateData(TRUE);

	m_addr = hex2int(m_addrstr);
	m_value = hex2int(m_valuestr);

}

void CSysDlg::SetDataValue(void)
{
	m_addrstr.Format("%x", m_addr);
	m_valuestr.Format("%x", m_value);

}

int CSysDlg::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	UINT cnt = 1;
	BYTE addrlen = 2;
	BYTE valuelen = 2;

	if ((pPageInfoSt == NULL) || (nStLen < 10)) return -1;

	GetDataValue();

	memcpy(pPageInfoSt, &cnt, 4);
	memcpy((char*)pPageInfoSt+4, &addrlen, 1);
	memcpy((char*)pPageInfoSt+5, &m_addr, 2);
	memcpy((char*)pPageInfoSt+7, &valuelen, 1);
	memcpy((char*)pPageInfoSt+8, &m_value, 2);

	nPageID = m_nID;
	nStLen = 10;

	return 0;
}

int CSysDlg::GetAddrInfo(void * pAddrInfo, int & nStLen)
{
	UINT cnt = 1;
	BYTE addrlen = 2;

	if ((pAddrInfo == NULL) || (nStLen < 7)) return -1;

	GetDataValue();

	memcpy(pAddrInfo, &cnt, 4);
	memcpy((char*)pAddrInfo+4, &addrlen, 1);
	memcpy((char*)pAddrInfo+5, &m_addr, 2);

	nStLen = 7;

	return 0;
}


int CSysDlg::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	BYTE addrlen = 0;
	BYTE valuelen = 0;

	if ((pPageInfoSt == NULL) || (nStLen < 8)) return -1;

	memcpy(&addrlen, (char*)pPageInfoSt+4, 1);
	memcpy(&m_addr, (char*)pPageInfoSt+5, addrlen);
	memcpy(&valuelen, (char*)pPageInfoSt+5+addrlen, 1);
	memcpy(&m_value, (char*)pPageInfoSt+6+addrlen, valuelen);


	SetDataValue();

	m_timer = SetTimer(1, 10, NULL);
	
	return 0;
}


int CSysDlg::SetEnable(int nFlag, BOOL bEnable)
{
	switch (nFlag)
	{
	case DIALOG_RAWLUT:
		break;

	case DIALOG_GAMMA:
		break;

	case DIALOG_WDR:
		break;

	default:
		return -1;
		break;
	}

	UpdateData(FALSE);

	return 0;
}
void CSysDlg::OnButtonGetSen() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, DIALOG_SYS, 0);
}

void CSysDlg::OnButtonSetSen() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, DIALOG_SYS, 0);
}

void CSysDlg::OnButtonClear() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_CLEAR_SENSOR_PARAM, DIALOG_SYS, 0);
}



void CSysDlg::Clean(void) 
{
	m_addrstr = _T("0");
	m_valuestr = _T("0");
	m_Version = _T("");
	m_Notes = _T("");
	m_sensorIdStr = _T("");

	UpdateData(FALSE);
}

void CSysDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (1 == nIDEvent)
	{
		KillTimer(m_timer);
		m_timer = 0;

		UpdateData(FALSE);
	}

	CDialog::OnTimer(nIDEvent);
}
