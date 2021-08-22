// Image_reset.cpp : 实现文件
//

#include "stdafx.h"
#include "Image_reset.h"
#include "Config_test.h"
#include "Anyka IP CameraDlg.h"



#define IMG_SHOW_WIDTH	640
#define IMG_SHOW_HEIGHT_960P	480
#define IMG_SHOW_HEIGHT_720P	360
#define IMG_SHOW_MUTIL	2



#define BMP_HEADINFO_LEN	54

#define CLIP255(x) ((x)>0?((x)<255?(x):255):0)


extern CConfig_test g_test_config;
extern BOOL g_connet_flag;
extern TCHAR m_connect_ip[MAX_PATH+1];
extern UINT g_video_width;
extern UINT g_video_height;
extern BOOL g_start_getdata;
extern unsigned char *g_net_buf ;
BOOL g_haved_getdata =FALSE;
unsigned char *g_bmp = NULL;
BOOL g_move_flag = FALSE;
extern T_IMAGE_INFO cut_out_on;

UINT g_src_img_width = YUV_WIDTH_720P;
UINT g_src_img_height = YUV_HEIGHT_720P;

BOOL g_cut_out_flag = FALSE;


// CImage_reset 对话框


static unsigned char *YUV420ToBMP_net(unsigned char* yuv, unsigned short width, unsigned short height)
{
	unsigned char *y, *u, *v, *dst;
	signed short Y, U, V, R, G, B;
	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	unsigned int line_byte, i, j;

	y = yuv;
	u = y + width * height;
	v = u + (width>>1) * (height>>1);

	line_byte = (width*3+3) & ~0x3;

	memset(&bf, 0, sizeof(bf));
	memset(&bi, 0, sizeof(bi));

	bf.bfType = 0x4d42;
	bf.bfSize = BMP_HEADINFO_LEN + height*line_byte;
	bf.bfOffBits = BMP_HEADINFO_LEN;

	bi.biSize = 40;
	bi.biWidth = width;
	bi.biHeight = height;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = 0;
	bi.biSizeImage = height*line_byte;

	if (g_bmp == NULL)
	{
		g_bmp = (unsigned char*)malloc(BMP_HEADINFO_LEN+height*line_byte);
		if (g_bmp == NULL)
		{
			return NULL;
		}
	}

	memset(g_bmp, 0, bf.bfSize);

	memcpy(g_bmp, &bf, 14);
	memcpy(&g_bmp[14], &bi, 40);

	//*bmp_size = 54 + height*line_byte;

	dst = g_bmp + BMP_HEADINFO_LEN + (height-1)*line_byte;

	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			Y = y[i*width+j];
			U = u[(i>>1)*(width>>1)+(j>>1)];
			V = v[(i>>1)*(width>>1)+(j>>1)];

			U -= 128;
			V -= 128;

			R = (signed short)CLIP255(Y + 1.4*V);
			G = (signed short)CLIP255(Y - 0.34*U - 0.71*V);
			B = (signed short)CLIP255(Y + 1.732*U);

			dst[j*3+0] = B&0xff;
			dst[j*3+1] = G&0xff;
			dst[j*3+2] = R&0xff;
		}
		dst -= line_byte;
	}

	return g_bmp;
}

static unsigned char* YUV420ToBMP(unsigned char* yuv, unsigned short width, unsigned short height)
{
	unsigned char *y, *u, *v, *bmp, *dst;
	signed short Y, U, V, R, G, B;
	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	unsigned long line_byte, i, j;

	y = yuv;
	u = y + width * height;
	v = u + (width>>1) * (height>>1);

	line_byte = (width*3+3) & ~0x3;

	memset(&bf, 0, sizeof(bf));
	memset(&bi, 0, sizeof(bi));

	bf.bfType = 0x4d42;
	bf.bfSize = BMP_HEADINFO_LEN + height*line_byte;
	bf.bfOffBits = BMP_HEADINFO_LEN;

	bi.biSize = 40;
	bi.biWidth = width;
	bi.biHeight = height;
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = 0;
	bi.biSizeImage = height*line_byte;

	bmp = (unsigned char*)malloc(BMP_HEADINFO_LEN+height*line_byte);
	if (!bmp)
	{
		return NULL;
	}

	memset(bmp, 0, bf.bfSize);

	memcpy(bmp, &bf, 14);
	memcpy(&bmp[14], &bi, 40);

	//*bmp_size = 54 + height*line_byte;

	dst = bmp + BMP_HEADINFO_LEN + (height-1)*line_byte;

	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			Y = y[i*width+j];
			U = u[(i>>1)*(width>>1)+(j>>1)];
			V = v[(i>>1)*(width>>1)+(j>>1)];

			U -= 128;
			V -= 128;

			R = (signed short)CLIP255(Y + 1.4*V);
			G = (signed short)CLIP255(Y - 0.34*U - 0.71*V);
			B = (signed short)CLIP255(Y + 1.732*U);

			dst[j*3+0] = B&0xff;
			dst[j*3+1] = G&0xff;
			dst[j*3+2] = R&0xff;
		}
		dst -= line_byte;
	}

	return bmp;
}



IMPLEMENT_DYNAMIC(CImage_reset, CDialog)

CImage_reset::CImage_reset(CWnd* pParent /*=NULL*/)
	: CDialog(CImage_reset::IDD, pParent)
{

}

CImage_reset::~CImage_reset()
{
}

void CImage_reset::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_X, m_img_x);
	DDX_Control(pDX, IDC_EDIT_Y, m_img_y);
}


BEGIN_MESSAGE_MAP(CImage_reset, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDOK, &CImage_reset::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_YUV, &CImage_reset::OnBnClickedButtonOpenYuv)
	ON_WM_CLOSE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON_COMEBACK, &CImage_reset::OnBnClickedButtonComeback)
	ON_BN_CLICKED(IDC_BUTTON_CORRECT_IMAGE, &CImage_reset::OnBnClickedButtonCorrectImage)
	ON_BN_CLICKED(IDC_BUTTON_SHOW, &CImage_reset::OnBnClickedButtonShow)
	ON_BN_CLICKED(IDC_BUTTON_UP, &CImage_reset::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CImage_reset::OnBnClickedButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CImage_reset::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CImage_reset::OnBnClickedButtonRight)
	ON_WM_TIMER()
	ON_EN_KILLFOCUS(IDC_EDIT_X, &CImage_reset::OnEnKillfocusEditX)
	ON_EN_UPDATE(IDC_EDIT_X, &CImage_reset::OnEnUpdateEditX)
	ON_NOTIFY(NM_THEMECHANGED, IDC_EDIT_X, &CImage_reset::OnNMThemeChangedEditX)
	ON_EN_ERRSPACE(IDC_EDIT_X, &CImage_reset::OnEnErrspaceEditX)
END_MESSAGE_MAP()


// CImage_reset 消息处理程序




BOOL CImage_reset::OnInitDialog()
{
	CString str;
	UINT height = 0;

	CDialog::OnInitDialog();

	m_yuvbuf = NULL;
	m_bmpbuf = NULL;
	
	m_drag = FALSE;
	m_moveflag = -1;

	CRect rect;

	this->GetClientRect(&rect);

	if (g_video_height <= YUV_HEIGHT_720P)
	{
		m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_720P+2);

		//m_ImgRect.SetRect(rect.left+2+(YUV_WIDTH_720P-g_video_width)/2, rect.top+2+(YUV_HEIGHT_720P - g_video_height)/2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_720P+2);
		g_src_img_width = YUV_WIDTH_720P;
		g_src_img_height = YUV_HEIGHT_720P;
	}
	else
	{
		m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_960P+2);
		//m_ImgRect.SetRect(rect.left+2+(YUV_WIDTH_720P-g_video_width)/2, rect.top+2+(YUV_HEIGHT_720P - g_video_height)/2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_960P+2);
		g_src_img_width = YUV_WIDTH_960P;
		g_src_img_height = YUV_HEIGHT_960P;
	}



	m_bInit = TRUE;

	g_net_buf = (unsigned char*)malloc(YUV_WIDTH_960P*YUV_HEIGHT_960P*4);
	if (g_net_buf == NULL)
	{
		AfxMessageBox(_T("分配网络的BUF失败"));
	}
	else
	{
		memset(g_net_buf, 0, YUV_WIDTH_960P*YUV_HEIGHT_960P*4);
		g_start_getdata = TRUE;
	}

	if (g_video_height <= YUV_HEIGHT_720P && g_test_config.image_y > IMG_SHOW_HEIGHT_720P*2)
	{
		g_test_config.image_y = g_video_height*2;
	}
	else if(g_video_height <= YUV_HEIGHT_960P && g_test_config.image_y > IMG_SHOW_HEIGHT_960P*2)
	{

		g_test_config.image_y = g_video_height*2;
	}

	str.Format(_T("%d"), g_test_config.image_x);
	SetDlgItemText(IDC_EDIT_X, str);

	str.Format(_T("%d"), g_test_config.image_y);
	SetDlgItemText(IDC_EDIT_Y, str);
	

	if (g_video_height <= YUV_HEIGHT_720P && g_test_config.image_y <= IMG_SHOW_HEIGHT_720P*2)
	{
		height = IMG_SHOW_HEIGHT_720P;
	}
	else if (g_video_height <= YUV_HEIGHT_960P && g_test_config.image_y <= IMG_SHOW_HEIGHT_960P*2)
	{
		height = IMG_SHOW_HEIGHT_960P;
	}
	
	m_keyPoint[0].x = (IMG_SHOW_WIDTH - g_test_config.image_x/2)/2;
	m_keyPoint[0].y = (height - g_test_config.image_y/2)/2;
	m_keyPoint[1].x = (IMG_SHOW_WIDTH - g_test_config.image_x/2)/2 + g_test_config.image_x/2;
	m_keyPoint[1].y = (height - g_test_config.image_y/2)/2 + g_test_config.image_y/2;

	SetTimer(TIMER_COMMAND, 1000, NULL);

	//UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


unsigned char* CImage_reset::LoadYuvData(TCHAR* path) 
{
	CFile file;
	CFileException e;
	CRect rect;

	this->GetClientRect(&rect);

	if (!file.Open(path, CFile::modeRead, &e))
	{
		return NULL;
	}

	unsigned long size = 0;
	size = file.GetLength();

	if (YUV_WIDTH_720P * YUV_HEIGHT_720P * 3 / 2 == size)
	{
		m_img_mode = IMG_MODE_720P;
		m_img_width = YUV_WIDTH_720P;
		m_img_height = YUV_HEIGHT_720P;

		m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_720P+2);

	}
	else if (YUV_WIDTH_960P * YUV_HEIGHT_960P * 3 / 2 == size)
	{
		m_img_mode = IMG_MODE_960P;
		m_img_width = YUV_WIDTH_960P;
		m_img_height = YUV_HEIGHT_960P;

		m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_960P+2);
	}

	unsigned char *buf = (unsigned char*)malloc(size);

	if (NULL == buf)
	{
		file.Close();
		return NULL;
	}

	int ret = 0;
	unsigned char *pbuf = buf;

	while (size > 0)
	{
		ret = file.Read(pbuf, size);
		pbuf += ret;
		size -= ret;
	}

	file.Close();

	return buf;
}


bool CImage_reset::GetBMP(void) 
{
	m_yuvbuf = LoadYuvData(m_imgpath);

	if (NULL == m_yuvbuf)
	{
		return FALSE;
	}

	m_bmpbuf = YUV420ToBMP(m_yuvbuf, m_img_width, m_img_height);

	if (NULL == m_bmpbuf)
	{
		return FALSE;
	}

	return TRUE;
}

void CImage_reset::BMShow(CDC *pDC, int x, int y, int width, int height) 
{
	if (NULL == m_bmpbuf)
		return;

	pDC->SetStretchBltMode(COLORONCOLOR);
	StretchDIBits(pDC->GetSafeHdc(),
		x, y, width, height, 0, 0, m_img_width, m_img_height,
		m_bmpbuf + BMP_HEADINFO_LEN,
		(BITMAPINFO*)(m_bmpbuf + sizeof(BITMAPFILEHEADER)),
		DIB_RGB_COLORS, SRCCOPY);
}


void CImage_reset::BMShow_net(CDC *pDC, int x, int y, int width, int height) 
{
	if (NULL == m_bmpbuf)
		return;

	pDC->SetStretchBltMode(COLORONCOLOR);
	StretchDIBits(pDC->GetSafeHdc(),
		x, y, width, height, 0, 0, g_video_width, g_video_height,
		m_bmpbuf + BMP_HEADINFO_LEN,
		(BITMAPINFO*)(m_bmpbuf + sizeof(BITMAPFILEHEADER)),
		DIB_RGB_COLORS, SRCCOPY);
}

bool CImage_reset::GetBMP_net(void) 
{

	m_bmpbuf = YUV420ToBMP_net(g_net_buf, g_video_width, g_video_height);

	if (NULL == m_bmpbuf)
	{
		return FALSE;
	}

	return TRUE;
}

void CImage_reset::OnPaint()
{
	CString str;
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()

	if (g_cut_out_flag)
	{
		//Invalidate(TRUE);
		UpdateData(TRUE);
		return;
	}

#if 0
	if (m_bRefreshFlag)
	{
		GetBMP();
		m_bRefreshFlag = FALSE;
	}


	BMShow(&dc, m_ImgRect.left, m_ImgRect.top, m_img_width/IMG_SHOW_MUTIL, m_img_height/IMG_SHOW_MUTIL);
#else
	if (g_haved_getdata && g_start_getdata && g_net_buf != NULL)
	{

		CRect rect;

		this->GetClientRect(&rect);

		if (g_video_width <= YUV_WIDTH_720P && g_video_height <= YUV_HEIGHT_720P)
		{
			m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_720P+2);
		}
		else
		{
			m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_960P+2);
		}

		//GetBMP();
		GetBMP_net();

		if (g_video_width <= YUV_WIDTH_720P && g_video_height <= YUV_HEIGHT_720P)
		{
			BMShow_net(&dc, m_ImgRect.left+(YUV_WIDTH_720P-g_video_width)/4 , m_ImgRect.top+(YUV_HEIGHT_720P - g_video_height)/4, g_video_width/IMG_SHOW_MUTIL, g_video_height/IMG_SHOW_MUTIL);
		}
		else
		{
			BMShow_net(&dc, m_ImgRect.left+(YUV_WIDTH_960P-g_video_width)/4 , m_ImgRect.top+(YUV_HEIGHT_960P - g_video_height)/4, g_video_width/IMG_SHOW_MUTIL, g_video_height/IMG_SHOW_MUTIL);
		}
		//BMShow_net(&dc, m_ImgRect.left, m_ImgRect.top, g_src_img_width/IMG_SHOW_MUTIL, g_src_img_height/IMG_SHOW_MUTIL);
		
	}
#endif
	//draw keypoints
	for (int i=0; i<2; i++) 
	{
		dc.FillSolidRect(m_keyPoint[i].x+m_ImgRect.left-2, m_keyPoint[i].y+m_ImgRect.top-2, 5, 5, RGB(0, 0, 255));
	}

	//draw select rect line
	CPen linePen(PS_SOLID, 1, RGB(0, 0, 255));
	dc.SelectObject(&linePen);

	dc.MoveTo(m_keyPoint[0].x+m_ImgRect.left, m_keyPoint[0].y+m_ImgRect.top);
	dc.LineTo(m_keyPoint[1].x+m_ImgRect.left, m_keyPoint[0].y+m_ImgRect.top);
	dc.LineTo(m_keyPoint[1].x+m_ImgRect.left, m_keyPoint[1].y+m_ImgRect.top);
	dc.LineTo(m_keyPoint[0].x+m_ImgRect.left, m_keyPoint[1].y+m_ImgRect.top);
	dc.LineTo(m_keyPoint[0].x+m_ImgRect.left, m_keyPoint[0].y+m_ImgRect.top);


	if (g_video_width <= YUV_WIDTH_720P && g_video_height <= YUV_HEIGHT_720P)
	{
		InvalidateRect(CRect(0,0,IMG_SHOW_WIDTH+4,IMG_SHOW_HEIGHT_720P+4),FALSE);
	}
	else
	{
		InvalidateRect(CRect(0,0,IMG_SHOW_WIDTH+4,IMG_SHOW_HEIGHT_960P+4),FALSE);
	}
	
	//InvalidateRect(CRect(0,0,g_src_img_width/IMG_SHOW_MUTIL,g_src_img_height/IMG_SHOW_MUTIL),FALSE);
	UpdateData(FALSE);

	
}

void CImage_reset::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	OnOK();
}


void CImage_reset::SetImgPath(TCHAR* path) 
{
	_tcscpy(m_imgpath, path);
	
	if (m_bInit)
	{
		Invalidate();
	}

	m_bRefreshFlag = TRUE;
	
}

void CImage_reset::OnBnClickedButtonOpenYuv()
{
	TCHAR path_buf[260] = {0};	
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(TRUE, _T("*.dat"), NULL, OFN_HIDEREADONLY,
		_T("Data File(*.dat)|*.dat|All Files (*.*)|*.*||"));
	if (dlg.DoModal()!=IDOK)
		return;
	
	memset(path_buf, 0, 260);
	_tcscpy(path_buf, dlg.GetPathName());
	SetImgPath(path_buf);
}

void CImage_reset::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_bInit = FALSE;

	if (NULL != m_yuvbuf)
	{
		free(m_yuvbuf);
		m_yuvbuf = NULL;
	}


	if (NULL != g_bmp)
	{
		free(g_bmp);
		g_bmp = NULL;
		m_bmpbuf = NULL;
	}

	

	if (g_net_buf != NULL)
	{
		free(g_net_buf);
		g_net_buf = NULL;
	}

	g_start_getdata = FALSE; 
	g_haved_getdata = FALSE;
	CDialog::OnClose();
}

void CImage_reset::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	// TODO: Add your message handler code here and/or call default
	if (!m_ImgRect.PtInRect(point)) {
		CDialog::OnLButtonDown(nFlags, point);
		return;
	}

	CPoint InPoint;
	InPoint.x = point.x - m_ImgRect.left;
	InPoint.y = point.y - m_ImgRect.top;


	if ((m_keyPoint[0].x == m_keyPoint[1].x)
		&& (m_keyPoint[0].y == m_keyPoint[1].y))
	{
		m_keyPoint[0].x = InPoint.x;
		m_keyPoint[0].y = InPoint.y;

		m_keyPoint[1].x = InPoint.x;
		m_keyPoint[1].y = InPoint.y;

		m_drag = TRUE;
		m_moveflag = 1;

		//InvalidateRect(m_ImgRect, FALSE);
	}
#if 1
	else
	{
		CRect rect(InPoint.x - 3, InPoint.y - 3, InPoint.x + 3, InPoint.y + 3);
		for (int i = 0; i < 2; i++) 
		{
			if (rect.PtInRect(m_keyPoint[i])) 
			{
				m_drag = TRUE;
				m_moveflag = i;
			}
		}
	}
#endif
	CDialog::OnLButtonDown(nFlags, point);
}

void CImage_reset::OnLButtonUp(UINT nFlags, CPoint point)
{
	CString str;
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_drag)
		m_drag = FALSE;
	
	if (m_ImgRect.PtInRect(point))
	{

		g_test_config.image_x = (m_keyPoint[1].x - m_keyPoint[0].x)*IMG_SHOW_MUTIL;
		g_test_config.image_y = (m_keyPoint[1].y - m_keyPoint[0].y)*IMG_SHOW_MUTIL;
		str.Format(_T("%d"), g_test_config.image_x);
		SetDlgItemText(IDC_EDIT_X, str);
		m_img_x.SetFocus();

		str.Format(_T("%d"), g_test_config.image_y);
		SetDlgItemText(IDC_EDIT_Y, str);
		g_move_flag = TRUE;
		m_img_y.SetFocus();
	}

	CDialog::OnLButtonUp(nFlags, point);
}

void CImage_reset::OnMouseMove(UINT nFlags, CPoint point)
{
	CString str;

	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!m_ImgRect.PtInRect(point)) {
		CDialog::OnMouseMove(nFlags, point);
		m_drag = FALSE;
		return;
	}

	CPoint InPoint;
	InPoint.x = point.x - m_ImgRect.left;
	InPoint.y = point.y - m_ImgRect.top;

	if (m_drag && m_moveflag >= 0 && (unsigned int)(m_moveflag) < 2) 
	{
		
		m_keyPoint[m_moveflag] = InPoint;

		g_test_config.image_x = (m_keyPoint[1].x - m_keyPoint[0].x)*IMG_SHOW_MUTIL;
		g_test_config.image_y = (m_keyPoint[1].y - m_keyPoint[0].y)*IMG_SHOW_MUTIL;
		str.Format(_T("%d"), g_test_config.image_x);
		SetDlgItemText(IDC_EDIT_X, str);
		m_img_x.SetFocus();

		str.Format(_T("%d"), g_test_config.image_y);
		SetDlgItemText(IDC_EDIT_Y, str);
		m_img_y.SetFocus();
		

	}


	CDialog::OnMouseMove(nFlags, point);
}

void CImage_reset::OnBnClickedButtonComeback()
{

	char *file_name = "cut_out_off";
	char test_param[MAX_PATH] = {0};
	//T_IMAGE_INFO cut_out_off;
	// TODO: 在此添加控件通知处理程序代码
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();
	
	g_cut_out_flag = TRUE;

	if (!g_connet_flag)
	{
		if (!pP->ConnetServer(m_connect_ip, 0))
		{
			AfxMessageBox(_T("网络没有连接上"), MB_OK);
			pP->CloseServer_image(0);
			g_cut_out_flag = FALSE;
			return ;
		}
	}

	//发送图像的起启点和尺寸大小给设备
	//发送图像的起启点和尺寸大小给设备
#if 0
	cut_out_off.start_x = 0;
	cut_out_off.start_y = 0;
	cut_out_off.image_width = g_src_img_width;
	cut_out_off.image_height = g_src_img_height;


	memset(test_param, 0, MAX_PATH);
	memcpy(test_param, &cut_out_off, sizeof(T_IMAGE_INFO));
#endif

	if (!pP->Send_cmd(TEST_COMMAND, 0, file_name, NULL, 0))
	{
		AfxMessageBox(_T("cut out on  Send_cmd　fail "), MB_OK); 
		g_cut_out_flag = FALSE;
		return;
	}

	//Anyka_Test_check_no_info(10000);

	pP->CloseServer_image(0);

	MessageBox(_T("已恢复校正，正重启，确定后请稍等..."), MB_OK); 
	OnClose();
	OnBnClickedOk();
	g_cut_out_flag = FALSE;

}

void CImage_reset::OnBnClickedButtonCorrectImage()
{
	CAnykaIPCameraDlg * pP = (CAnykaIPCameraDlg *)GetParent();

	char *file_name = "cut_out_on";
	char test_param[MAX_PATH] = {0};
	T_IMAGE_INFO cut_out_on;
	
	g_cut_out_flag = TRUE;

	if (g_test_config.image_x%16 != 0 || g_test_config.image_y%2 != 0)
	{
		AfxMessageBox(_T("图像宽必需为16的倍数，高必需为2的倍数，请重输"), MB_OK);
		g_cut_out_flag = FALSE;
		return;
	}



#if 1

	// TODO: 在此添加控件通知处理程序代码
	if (!g_connet_flag)
	{
		if (!pP->ConnetServer(m_connect_ip, 0))
		{
			AfxMessageBox(_T("网络没有连接上"), MB_OK);
			pP->CloseServer_image(0);
			g_cut_out_flag = FALSE;
			return ;
		}
	}
#endif
	
	//发送图像的起启点和尺寸大小给设备
	cut_out_on.start_x = m_keyPoint[0].x *IMG_SHOW_MUTIL;
	cut_out_on.start_y = m_keyPoint[0].y *IMG_SHOW_MUTIL;
	cut_out_on.image_width = g_test_config.image_x;
	cut_out_on.image_height = g_test_config.image_y;

	memset(test_param, 0, MAX_PATH);
	sprintf(test_param, "%d %d %d %d", cut_out_on.start_x, cut_out_on.start_y, cut_out_on.image_width, cut_out_on.image_height);
	//memcpy(test_param, &cut_out_on, sizeof(T_IMAGE_INFO));

	if (!pP->Send_cmd(TEST_COMMAND, 0, file_name, test_param, 0))
	{

		AfxMessageBox(_T("cut out on  Send_cmd　fail "), MB_OK); 
		g_cut_out_flag = FALSE;
		return;
	}


	pP->CloseServer_image(0);
	MessageBox(_T("已进行全景校正，正重启，确定后请稍等..."), MB_OK); 
	OnClose();
	OnBnClickedOk();

	g_cut_out_flag = FALSE;
	
}

void CImage_reset::OnBnClickedButtonShow()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	UINT temp_len = 0, height= 0;
	

	USES_CONVERSION;

	
	g_cut_out_flag = TRUE;
	GetDlgItemText(IDC_EDIT_X, str);
	temp_len = atoi(T2A(str));

	if(temp_len > IMG_SHOW_WIDTH*IMG_SHOW_MUTIL)
	{
		AfxMessageBox(_T("分辩率大小超大了,1280*720或1280*960"));
		str.Format(_T("%d"), g_test_config.image_x);
		SetDlgItemText(IDC_EDIT_X, str);
		g_cut_out_flag = FALSE;
		return;

	}
	g_test_config.image_x  = temp_len;

	GetDlgItemText(IDC_EDIT_Y, str);
	temp_len = atoi(T2A(str));
	
	if (g_video_height <= YUV_HEIGHT_720P && temp_len > IMG_SHOW_HEIGHT_720P*IMG_SHOW_MUTIL)
	{
		AfxMessageBox(_T("分辩率大小超大了,1280*720"));
		str.Format(_T("%d"), g_test_config.image_y);
		SetDlgItemText(IDC_EDIT_Y, str);
		g_cut_out_flag = FALSE;
		return;

	}
	else if (g_video_height <= YUV_HEIGHT_960P && temp_len > IMG_SHOW_HEIGHT_960P*IMG_SHOW_MUTIL)
	{
		AfxMessageBox(_T("分辩率大小超大了,280*960"));
		str.Format(_T("%d"), g_test_config.image_y);
		SetDlgItemText(IDC_EDIT_Y, str);
		g_cut_out_flag = FALSE;
		return;

	}

	g_test_config.image_y = temp_len;


	if (g_test_config.image_x%16 != 0 || g_test_config.image_y%2 != 0)
	{
		AfxMessageBox(_T("图像宽必需为16的倍数，高必需为2的倍数，请重输"), MB_OK);
		g_cut_out_flag = FALSE;
		return;
	}


	if (g_video_height <= YUV_HEIGHT_720P)
	{
		height = IMG_SHOW_HEIGHT_720P;
	}
	else
	{
		height = IMG_SHOW_HEIGHT_960P;
	}

	m_keyPoint[0].x = (IMG_SHOW_WIDTH - g_test_config.image_x/2)/2;
	m_keyPoint[0].y = (height - g_test_config.image_y/2)/2;
	m_keyPoint[1].x = (IMG_SHOW_WIDTH - g_test_config.image_x/2)/2 + g_test_config.image_x/2;
	m_keyPoint[1].y = (height - g_test_config.image_y/2)/2 + g_test_config.image_y/2;

	Invalidate(FALSE);
	g_cut_out_flag = FALSE;
}

void CImage_reset::OnBnClickedButtonUp()
{
	// TODO: 在此添加控件通知处理程序代码

	CPoint m_keyPoint_temp;

	m_keyPoint_temp.x = m_keyPoint[0].x + 2;
	m_keyPoint_temp.y = m_keyPoint[0].y + 1;

	if (!m_ImgRect.PtInRect(m_keyPoint_temp)) 
	{
		return;
	}

	m_keyPoint[0].y--;
	m_keyPoint[1].y--;
	Invalidate(FALSE);

}

void CImage_reset::OnBnClickedButtonDown()
{
	// TODO: 在此添加控件通知处理程序代码
	CPoint m_keyPoint_temp;

	m_keyPoint_temp.x = m_keyPoint[1].x + 2;
	m_keyPoint_temp.y = m_keyPoint[1].y + 2;

	if (!m_ImgRect.PtInRect(m_keyPoint_temp)) 
	{
		return;
	}

	m_keyPoint[0].y++;
	m_keyPoint[1].y++;
	Invalidate(FALSE);
}

void CImage_reset::OnBnClickedButtonLeft()
{
	// TODO: 在此添加控件通知处理程序代码
	CPoint m_keyPoint_temp;

	m_keyPoint_temp.x = m_keyPoint[0].x + 1;
	m_keyPoint_temp.y = m_keyPoint[0].y + 2;

	if (!m_ImgRect.PtInRect(m_keyPoint_temp)) 
	{
		return;
	}

	m_keyPoint[0].x--;
	m_keyPoint[1].x--;
	Invalidate(FALSE);
}

void CImage_reset::OnBnClickedButtonRight()
{
	// TODO: 在此添加控件通知处理程序代码

	CPoint m_keyPoint_temp;

	m_keyPoint_temp.x = m_keyPoint[1].x + 2;
	m_keyPoint_temp.y = m_keyPoint[1].y + 2;

	if (!m_ImgRect.PtInRect(m_keyPoint_temp)) 
	{
		return;
	}
	m_keyPoint[0].x++;
	m_keyPoint[1].x++;
	Invalidate(FALSE);
}

#if 0
BOOL CImage_reset::PreTranslateMessage(MSG* pMsg)
{
	int buID=0;
	UINT i = 0;
	CRect rect_up;
	CRect rect_down;
	CRect rect_left;
	CRect rect_right;

	CWnd* pWnd=CWnd::FromHandle(pMsg->hwnd);
	//buID= GetWindowLong(pMsg->hwnd,GWL_ID);//由窗口句柄获得ID号，GetWindowLong为获得窗口的ID号。

	CWnd * pButtonLeft = GetDlgItem(IDC_BUTTON_LEFT);
	CWnd * pButtonRight = GetDlgItem(IDC_BUTTON_RIGHT);
	CWnd * pButtonUp = GetDlgItem(IDC_BUTTON_UP);
	CWnd * pButtonDown = GetDlgItem(IDC_BUTTON_DOWN);

	if(pMsg->message == WM_LBUTTONDOWN) 
	{     
		buID= pWnd->GetDlgCtrlID();

		pButtonLeft->GetWindowRect(rect_left);
		pButtonRight->GetWindowRect(rect_right);
		pButtonUp->GetWindowRect(rect_up);
		pButtonDown->GetWindowRect(rect_down);

		if(rect_up.PtInRect(pMsg->pt) && buID==IDC_BUTTON_UP) //按下
		{  
			m_keyPoint[0].y--;
			m_keyPoint[1].y--;
			Invalidate();
		} 
		else if(rect_down.PtInRect(pMsg->pt) && buID==IDC_BUTTON_DOWN) //按下
		{  
			m_keyPoint[0].y++;
			m_keyPoint[1].y++;
			Invalidate();
		} 

		else if(rect_left.PtInRect(pMsg->pt) && buID==IDC_BUTTON_LEFT) //按下
		{  
			m_keyPoint[0].x--;
			m_keyPoint[1].x--;
			Invalidate();
		} 
		else if(rect_right.PtInRect(pMsg->pt) && buID==IDC_BUTTON_RIGHT) //按下
		{  
			m_keyPoint[0].x++;
			m_keyPoint[1].x++;
			Invalidate();
		} 
	}
	if(pMsg->message==WM_LBUTTONUP) 
	{ 
		pButtonLeft->GetWindowRect(rect_left);
		pButtonRight->GetWindowRect(rect_right);
		pButtonUp->GetWindowRect(rect_up);
		pButtonDown->GetWindowRect(rect_down);

		buID= pWnd->GetDlgCtrlID();
		if(rect_up.PtInRect(pMsg->pt) && buID==IDC_BUTTON_UP) //按下
		{  
			Invalidate();
		} 
		else if(rect_down.PtInRect(pMsg->pt) && buID==IDC_BUTTON_DOWN) //按下
		{  
			Invalidate();
		} 

		else if(rect_left.PtInRect(pMsg->pt) &&  buID==IDC_BUTTON_LEFT) //按下
		{  
			Invalidate();
		} 
		else if(rect_right.PtInRect(pMsg->pt) && buID==IDC_BUTTON_RIGHT) //按下
		{  
			Invalidate();
		}

	}

	//FromHandlePermanent(HWND hWnd);
	DeleteTempMap();
	return CDialog::PreTranslateMessage(pMsg);


}

#endif

void CImage_reset::OnTimer(UINT_PTR nIDEvent)
{
	CString str;


	// TODO: 在此添加消息处理程序代码和/或调用默认值
#if 0
	g_test_config.image_x = (m_keyPoint[1].x - m_keyPoint[0].x)*IMG_SHOW_MUTIL;
	g_test_config.image_y = (m_keyPoint[1].y - m_keyPoint[0].y)*IMG_SHOW_MUTIL;
	str.Format(_T("%d"), g_test_config.image_x);
	SetDlgItemText(IDC_EDIT_X, str);

	str.Format(_T("%d"), g_test_config.image_y);
	SetDlgItemText(IDC_EDIT_Y, str);

	InvalidateRect(CRect(0,g_video_height/IMG_SHOW_MUTIL,g_video_width/IMG_SHOW_MUTIL, 300),TRUE);
#endif

	CDialog::OnTimer(nIDEvent);
}

void CImage_reset::OnEnKillfocusEditX()
{
	// TODO: 在此添加控件通知处理程序代码
	UINT i = 0;
}

void CImage_reset::OnEnUpdateEditX()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 CDialog::OnInitDialog()
	// 函数，将 EM_SETEVENTMASK 消息发送到控件，
	// 同时将 ENM_UPDATE 标志“或”运算到 lParam 掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UINT i = 0;
}

void CImage_reset::OnNMThemeChangedEditX(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 该功能要求使用 Windows XP 或更高版本。
	// 符号 _WIN32_WINNT 必须 >= 0x0501。
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CImage_reset::OnEnErrspaceEditX()
{
	// TODO: 在此添加控件通知处理程序代码
	UINT i = 0;
}
