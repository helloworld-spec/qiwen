// TencentFontCreateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TencentFontCreate.h"
#include "TencentFontCreateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTencentFontCreateDlg dialog

CTencentFontCreateDlg::CTencentFontCreateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTencentFontCreateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTencentFontCreateDlg)
	m_osd_name = _T("");
	m_FontSize = 0;
	m_edit_font_file = _T("");
	m_font_out_file_name = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTencentFontCreateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTencentFontCreateDlg)
	DDX_Control(pDX, IDC_OSD_NAME, m_osd_name_ctrl);
	DDX_Text(pDX, IDC_OSD_NAME, m_osd_name);
	DDX_Text(pDX, IDC_FONT_SIZE, m_FontSize);
	DDV_MinMaxInt(pDX, m_FontSize, 8, 32);
	DDX_Text(pDX, IDC_EDIT_FONTFILE, m_edit_font_file);
	DDX_Text(pDX, IDC_EDIT_FONT_OUT, m_font_out_file_name);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTencentFontCreateDlg, CDialog)
	//{{AFX_MSG_MAP(CTencentFontCreateDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTencentFontCreateDlg message handlers

BOOL CTencentFontCreateDlg::OnInitDialog()
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
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTencentFontCreateDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTencentFontCreateDlg::OnPaint() 
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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTencentFontCreateDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
#if 1

#define UNICODE_START_CODE  0X4E00
#define UNICODE_END_CODE    0X9FA5

void CTencentFontCreateDlg::font_lib_get_font_from_bin(WORD *uni_code,  int len, int fontsize)
{
    int byte_per_font, offset, code;
	char *font_buf;
	int FONT_BUT_SIZE = fontsize*fontsize/8;
	FILE *raw_font, *out_font;


	raw_font = fopen(m_file_name.GetBuffer(100), "rb");
	out_font = fopen(m_font_out_file_name.GetBuffer(100), "wb");

	if(!(raw_font && out_font))
	{
		return ;
	}

	font_buf = (char *)malloc(FONT_BUT_SIZE*2);
    byte_per_font = FONT_BUT_SIZE;
    
	for(int i = 0; i < len; i++)
	{
		code = uni_code[i];
		if(code < 0x80)
		{
			offset = (code - 1) * byte_per_font;
		}
		else if(code >= UNICODE_START_CODE && code <= UNICODE_END_CODE)
		{
			offset = (127 + code - UNICODE_START_CODE) * byte_per_font;
		}
		else
		{
			//系统中无此字体信息
			offset = 0x1F;
			//anyka_print("[%s,%s]:we don't include the font(0x%04x)\n", __FILE__, __FUNCTION__, code);
		}
		fseek(raw_font, offset, SEEK_SET);
		fread(font_buf, byte_per_font, 1, raw_font);
		fwrite(&code, 2, 1, out_font);
		fwrite(font_buf, byte_per_font, 1, out_font);
	}
	fclose(raw_font);
	fclose(out_font);
	free(font_buf);
}

#endif
void CTencentFontCreateDlg::OnOK() 
{
	// TODO: Add extra validation here
	char gb_code[200], len = 0;
	int i;
	char *osd_name;
	WORD unicode[200], unicode_len;
	WORD data_info[] = {0x5e74,
        0x6708, 0x65e5, 0x661f, 0x671f, 0x5929, 0x4e00, 0x4e8c, 0x4e09, 0x56db, 0x4e94, 0x516d};
	
	if(m_file_name.GetLength() == 0)
	{
		AfxMessageBox("please select font file!");
		return ;
	}
	UpdateData(TRUE);


	gb_code[len ++] = ':';
	gb_code[len ++] = '-';
	gb_code[len ++] = '//';
	gb_code[len ++] = '.';
	gb_code[len ++] = '#';
	gb_code[len ++] = '@';
	gb_code[len ++] = '*';
	for(i = 0; i < 10; i ++)
	{
		gb_code[len ++] = '0' + i;
	}
	for(i = 'a'; i <= 'z'; i ++)
	{
		gb_code[len ++] = i;
	}
	for(i = 'A'; i <= 'Z'; i ++)
	{
		gb_code[len ++] = i;
	}
	osd_name = m_osd_name.GetBuffer(100);
	for(i = 0; i < m_osd_name.GetLength(); i ++)
	{
		if((unsigned char )osd_name[i] >= 0x80)
		{
			gb_code[len ++] = osd_name[i ++];
			gb_code[len ++] = osd_name[i];
		}
	}

	gb_code[len] = 0;
	unicode_len = MultiByteToWideChar(CP_ACP, 0, gb_code, len, unicode, 200);
	if(m_FontSize % 8)
	{
		m_FontSize += 8;
	}
	m_FontSize = m_FontSize/8*8;
	memcpy(&unicode[unicode_len], data_info, sizeof(data_info));
	unicode_len += sizeof(data_info) / sizeof(data_info[0]);
	font_lib_get_font_from_bin(unicode, unicode_len, m_FontSize);
	
	CDialog::OnOK();
}

void CTencentFontCreateDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	CFileDialog FontFileName(TRUE);
	CString file_name ;
	m_file_name = "";
	if(IDOK == FontFileName.DoModal())
	{
		m_file_name = FontFileName.GetPathName();
		SetDlgItemText(IDC_EDIT_FONTFILE,m_file_name);
	}
}
