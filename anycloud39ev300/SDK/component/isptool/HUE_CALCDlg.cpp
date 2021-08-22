// HUE_CALCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "HUE_CALCDlg.h"
#include "ImgDlg.h"
#include "NetCtrl.h"
#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define YUV_WIDTH_720P	1280
#define YUV_HEIGHT_720P	720
#define YUV_WIDTH_960P	1280
#define YUV_HEIGHT_960P	960
#define YUV_WIDTH_1080P		1920
#define YUV_HEIGHT_1080P	1080
#define YUV_WIDTH_1536P		2048
#define YUV_HEIGHT_1536P	1536
#define YUV_WIDTH_1440P		2560
#define YUV_HEIGHT_1440P	1440
#define YUV_WIDTH_1296P		2304
#define YUV_HEIGHT_1296P	1296

#define IMG_SHOW_LEFT	10
#define IMG_SHOW_TOP	10

#define IMG_SHOW_WIDTH	640
#define IMG_SHOW_HEIGHT	480

#define IMG_SHOW_MUTIL	2
#define IMG_SHOW_MUTIL_1080P	3
#define IMG_SHOW_MUTIL_1536P	3
#define IMG_SHOW_MUTIL_1440P	4
#define IMG_SHOW_MUTIL_1296P	4

#define BMP_HEADINFO_LEN	54

#define CLIP255(x) ((x)>0?((x)<255?(x):255):0)

#define YUV_NV12

extern AK_ISP_INIT_PARAM   Isp_init_param;
/////////////////////////////////////////////////////////////////////////////
// CHUE_CALCDlg dialog

CHUE_CALCDlg::CHUE_CALCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHUE_CALCDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHUE_CALCDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bRefreshFlag = FALSE;
	m_bInit = FALSE;
	ZeroMemory(&m_imgpath, 260);
}

void CHUE_CALCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHUE_CALCDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_SLIDER_SATURATION, m_slider_saturation);
	DDX_Control(pDX, IDC_SPIN_SATURATION, m_spin_saturation);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHUE_CALCDlg, CDialog)
	//{{AFX_MSG_MAP(CHUE_CALCDlg)
	ON_BN_CLICKED(IDC_BUTTON_GET_YUV_IMG, OnButtonGetYuvImg)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_YUV_IMG, OnButtonOpenYuvImg)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_RGB_VAL, OnButtonRgbVal)
	ON_BN_CLICKED(IDC_BUTTON_CALC, OnButtonCalc)
	ON_WM_CLOSE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_HUE, OnButtonExportHue)
	ON_EN_KILLFOCUS(IDC_EDIT_SATURATION, OnKillfocusEditSaturation)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SATURATION, OnDeltaposSpinSaturation)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SATURATION, OnReleasedcaptureSliderSaturation)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_SATURATION, OnCustomdrawSliderSaturation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHUE_CALCDlg message handlers

static T_U8* YUV420ToBMP(T_U8* yuv, T_U16 width, T_U16 height)
{
	T_U8 *y, *u, *v, *bmp, *dst;
	T_S16 Y, U, V, R, G, B;
	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	T_U32 line_byte, i, j;
		
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
	
	bmp = (T_U8*)malloc(BMP_HEADINFO_LEN+height*line_byte);
	if (!bmp)
	{
		return NULL;
	}
	
	memset(bmp, 0, bf.bfSize);
	
	memcpy(bmp, &bf, 14);
	memcpy(&bmp[14], &bi, 40);
	
	dst = bmp + BMP_HEADINFO_LEN + (height-1)*line_byte;

	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
		#ifdef YUV_NV12
			Y = y[i*width+j];
			U = u[(i>>1)*width+j-j%2];
			V = u[(i>>1)*width+j-j%2+1];
		#else
			Y = y[i*width+j];
			U = u[(i>>1)*(width>>1)+(j>>1)];
			V = v[(i>>1)*(width>>1)+(j>>1)];
		#endif
			
			U -= 128;
			V -= 128;
			
			R = (T_S16)CLIP255(Y + 1.4*V);
			G = (T_S16)CLIP255(Y - 0.34*U - 0.71*V);
			B = (T_S16)CLIP255(Y + 1.732*U);
			
			dst[j*3+0] = B&0xff;
			dst[j*3+1] = G&0xff;
			dst[j*3+2] = R&0xff;
		}
		dst -= line_byte;
	}

	return bmp;
}

static int argtan_table[10]={16384, 9672,5110,2594,1302,652, 326,163,81,41};
int CalcHUEparam(RGB Original_RGB[],const RGB Target_RGB[],AK_ISP_HUE* para, int sat)
{
	RGB Stand_RGB[18]={{0,0,0}};
	YUV Stand_YUV[18] = {{0,0,0}};
	YUV original_YUV[18] = {{0,0,0}}; 
	
	int i = 0;
	int k = 0;
	int j = 0;

	for(i = 0;i < 18;i++)
	{
		Stand_RGB[i].R = Target_RGB[i].R;
		Stand_RGB[i].G = Target_RGB[i].G;
		Stand_RGB[i].B = Target_RGB[i].B;

		Stand_YUV[i].y = RGB2L(Stand_RGB[i].R,Stand_RGB[i].G,Stand_RGB[i].B);
		Stand_YUV[i].u = RGB2U(Stand_RGB[i].R,Stand_RGB[i].G,Stand_RGB[i].B)*sat/100;
		Stand_YUV[i].v = RGB2V(Stand_RGB[i].R,Stand_RGB[i].G,Stand_RGB[i].B)*sat/100;

		original_YUV[i].y = RGB2L(Original_RGB[i].R,Original_RGB[i].G,Original_RGB[i].B);
		original_YUV[i].u = RGB2U(Original_RGB[i].R,Original_RGB[i].G,Original_RGB[i].B);
		original_YUV[i].v = RGB2V(Original_RGB[i].R,Original_RGB[i].G,Original_RGB[i].B);
	}

	for(i = 0;i < 65;i++)
	{
		para->hue_lut_s[i] = 0;
		para->hue_lut_a[i] = 127;
		para->hue_lut_b[i] = 0;
	}

	for(i = 0; i < 18; i++)
	{
		int U, V;
		int x,y,z;
		int y_new, x_new;
		int a, b, s;
		float fu,fv;
		
		U =original_YUV[i].u;	
		V =original_YUV[i].v;
		//计算U/V的角度值作为色调，采用Cordic算法：
		if(U<0&&V<0)
		{	//旋转负90度到第四象限
			x = 0-V;
			y = U;
			z = (-128) <<8;	//-90度
		}
		else if(U<0&&V>=0)
		{	//旋转正90度到第一象限
			x = V;
			y = 0-U;
			z = 128<<8;	//90度:128
		}
		else
		{
			x = U;
			y = V;
			z = 0;
		}

		for(k=0; k<10; k++)
		{
			if(y<0)
			{
				x_new = x-(y>>k);
				y_new = y+(x>>k);
				z = z - argtan_table[k];	//argtan_table为ROM表，每项对应了2^(-i)的argtan值
			}
			else
			{
				x_new = x+(y>>k);
				y_new = y-(x>>k);
				z = z + argtan_table[k];	
			}
			x = x_new;
			y = y_new;
			if(y<=1&&y>=-1)
				break;
		}
		z = ((z+128)>>8)+256;	//取高8位作为角度值，并偏移到[0, 511]区间，准备查表
		if(z>511)
			z=511;
		if(z<0)
			z = 0;

		//查表获取U/V对应的色调调整值和饱和度调整值
		k = z>>3;
		s = sqrt((Stand_YUV[i].u*Stand_YUV[i].u+Stand_YUV[i].v*Stand_YUV[i].v)*4096/(U*U+V*V));

		fu = U*sqrt((Stand_YUV[i].u*Stand_YUV[i].u+Stand_YUV[i].v*Stand_YUV[i].v)*1.0/(U*U+V*V));
		fv = V*sqrt((Stand_YUV[i].u*Stand_YUV[i].u+Stand_YUV[i].v*Stand_YUV[i].v)*1.0/(U*U+V*V));
		b = (fv*Stand_YUV[i].u-fu*Stand_YUV[i].v)*128/(fv*fv+fu*fu);
		a = (fu*Stand_YUV[i].u+fv*Stand_YUV[i].v)*128/(fv*fv+fu*fu);
		
		para->hue_lut_s[k] = CLIP(s, 0, 255);
		para->hue_lut_a[k] = CLIP(a, -128, 127);
		para->hue_lut_b[k] = CLIP(b, -128, 127);;
	}

	for(i = 0;i<64; i++)
	{
		int step;
		if(para->hue_lut_s[i] == 0)
			continue;

		j=i+1;
		while(para->hue_lut_s[j%64] == 0)
		{
			j++;
		}
		
		step = j-i;
		for(k=i+1; k<j; k++)
		{
			para->hue_lut_s[k%64] = para->hue_lut_s[i]+(para->hue_lut_s[j%64]-para->hue_lut_s[i])*(k-i)/(j-i);
			para->hue_lut_a[k%64] = para->hue_lut_a[i]+(para->hue_lut_a[j%64]-para->hue_lut_a[i])*(k-i)/(j-i);
			para->hue_lut_b[k%64] = para->hue_lut_b[i]+(para->hue_lut_b[j%64]-para->hue_lut_b[i])*(k-i)/(j-i);
		}
		i=j-1;	//start from j next loop
	}
	para->hue_lut_s[64] = para->hue_lut_s[0];
	para->hue_lut_a[64] = para->hue_lut_a[0];
	para->hue_lut_b[64] = para->hue_lut_b[0];

	return 0;
}

void CHUE_CALCDlg::Img_SetConnectState(bool bConnect) 
{
	// TODO: Add your control notification handler code here	
	m_img_bConnect = bConnect;
}

void CHUE_CALCDlg::SetImgPath(char* path) 
{
	strcpy(m_imgpath, path);

	if (m_bInit)
	{
		Invalidate();
	}

	m_bRefreshFlag = TRUE;
}

void CHUE_CALCDlg::BMShow(CDC *pDC, int x, int y, int width, int height) 
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

bool CHUE_CALCDlg::GetBMP(void) 
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

void CHUE_CALCDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if (!m_ImgRect.PtInRect(point)) {
		CDialog::OnLButtonDown(nFlags, point);
		return;
	}

	CPoint InPoint;
	InPoint.x = point.x - m_ImgRect.left;
	InPoint.y = point.y - m_ImgRect.top;

	CRect rect(InPoint.x - 4, InPoint.y - 4, InPoint.x + 4, InPoint.y + 4);

	for (int i = 0; i < 4; i++) 
	{
		if (rect.PtInRect(m_keyPoint[i])) 
		{
			m_drag = TRUE;
			m_moveflag = i;
		}
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CHUE_CALCDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if (m_drag)
		m_drag = FALSE;

	CDialog::OnLButtonUp(nFlags, point);

	Calc_Frames();
	Calc_Input_RGB();
}

void CHUE_CALCDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default	
	if (!m_ImgRect.PtInRect(point)) {
		CDialog::OnMouseMove(nFlags, point);
		m_drag = FALSE;
		return;
	}

	CPoint InPoint;
	InPoint.x = point.x - m_ImgRect.left;
	InPoint.y = point.y - m_ImgRect.top;
	
	if (m_drag && m_moveflag >= 0 && (unsigned int)(m_moveflag) < 4) 
	{
		m_keyPoint[m_moveflag] = InPoint;
		InvalidateRect(m_ImgRect, FALSE);
	}

	CDialog::OnMouseMove(nFlags, point);
	Calc_Frames();
}

T_U8* CHUE_CALCDlg::LoadYuvData(char* path) 
{
	CFile file;
	CFileException e;

	if (!file.Open(path, CFile::modeRead, &e))
	{
		return NULL;
	}

	UINT size = 0;
	size = file.GetLength();

	if (YUV_WIDTH_720P * YUV_HEIGHT_720P * 3 / 2 == size)
	{
		m_img_mode = IMG_MODE_720P;
		m_img_width = YUV_WIDTH_720P;
		m_img_height = YUV_HEIGHT_720P;
		m_img_mutil = IMG_SHOW_MUTIL;
	}
	else if (YUV_WIDTH_960P * YUV_HEIGHT_960P * 3 / 2 == size)
	{
		m_img_mode = IMG_MODE_960P;
		m_img_width = YUV_WIDTH_960P;
		m_img_height = YUV_HEIGHT_960P;
		m_img_mutil = IMG_SHOW_MUTIL;
	}
	else if (YUV_WIDTH_1080P * YUV_HEIGHT_1080P * 3 / 2 == size)
	{
		m_img_mode = IMG_MODE_1080P;
		m_img_width = YUV_WIDTH_1080P;
		m_img_height = YUV_HEIGHT_1080P;
		m_img_mutil = IMG_SHOW_MUTIL_1080P;
	}
	else if (YUV_WIDTH_1536P * YUV_HEIGHT_1536P * 3 / 2 == size)
	{
		m_img_width = YUV_WIDTH_1536P;
		m_img_height = YUV_HEIGHT_1536P;
		m_img_mutil = IMG_SHOW_MUTIL_1536P;
	}
	else if (1536 * 1536 * 3 / 2 == size)
	{
		m_img_width = 1536;
		m_img_height = 1536;
		m_img_mutil = IMG_SHOW_MUTIL_1536P;
	}
	else if (YUV_WIDTH_1440P * YUV_HEIGHT_1440P * 3 / 2 == size)
	{
		m_img_width = YUV_WIDTH_1440P;
		m_img_height = YUV_HEIGHT_1440P;
		m_img_mutil = IMG_SHOW_MUTIL_1440P;
	}
	else if (YUV_WIDTH_1296P * YUV_HEIGHT_1296P * 3 / 2 == size)
	{
		m_img_width = YUV_WIDTH_1296P;
		m_img_height = YUV_HEIGHT_1296P;
		m_img_mutil = IMG_SHOW_MUTIL_1296P;
	}

	T_U8 *buf = (T_U8*)malloc(size);

	if (NULL == buf)
	{
		file.Close();
		return NULL;
	}

	int ret = 0;
	T_U8 *pbuf = buf;

	while (size > 0)
	{
		ret = file.Read(pbuf, size);
		pbuf += ret;
		size -= ret;
	}

	file.Close();

	return buf;
}

BOOL CHUE_CALCDlg::OnInitDialog()
{
	CString str;

	CDialog::OnInitDialog();

	m_yuvbuf = NULL;
	m_bmpbuf = NULL;

	m_img_width = YUV_WIDTH_960P;
	m_img_height = YUV_HEIGHT_960P;
	m_img_mutil = IMG_SHOW_MUTIL;
	
	m_keyPoint[0].x = 160;
	m_keyPoint[0].y = 90;

	m_keyPoint[1].x = 480;
	m_keyPoint[1].y = 90;

	m_keyPoint[2].x = 480;
	m_keyPoint[2].y = 270;

	m_keyPoint[3].x = 160;
	m_keyPoint[3].y = 270;

	m_drag = FALSE;
	m_moveflag = -1;

	Saturation = 128;

	m_spin_saturation.SetRange(0,1024);
	m_spin_saturation.SetPos(100);
	m_slider_saturation.SetRange(0,1024);
	m_slider_saturation.SetPos(100);

	Saturation = last_Saturation = m_slider_saturation.GetPos();

	CRect rect;

	this->GetClientRect(&rect);

	m_ImgRect.SetRect(rect.left+IMG_SHOW_LEFT, rect.top+IMG_SHOW_TOP, IMG_SHOW_WIDTH+IMG_SHOW_LEFT, IMG_SHOW_HEIGHT+IMG_SHOW_TOP);

	m_bInit = TRUE;

	Calc_Frames();
	
	Show_flag = FALSE;
	Calculate_flag = FALSE;
	g_Calc_Thread = INVALID_HANDLE_VALUE;

	Invalidate();
	return TRUE;
}

BOOL CHUE_CALCDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CHUE_CALCDlg::OnButtonGetYuvImg() 
{
	// TODO: Add your control notification handler code here
	if (!m_img_bConnect) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取参数!\n", MB_OK);
		return;
	}

	if (NULL == m_pMessageWnd)
	{
		AfxMessageBox("m_pMessageWnd is null\n", MB_OK);
		return;
	}
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_YUV_IMG, DIALOG_STAT, 0);

	Show_flag = TRUE;
	Invalidate();
	Calculate_flag = FALSE;
	//计算输入的rgb图像参数
	Calc_Input_RGB();	
}

void CHUE_CALCDlg::Calc_Input_RGB() 
{
	// TODO: Add your control notification handler code here
	int i = 0, j = 0, m = 0, n = 0;
	int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	UINT offset = 0;
	T_U8 B = 0, G = 0, R = 0;
	T_U8 BAvg = 0, GAvg = 0, RAvg = 0;
	T_U32 BSum = 0, GSum = 0, RSum = 0;

	if (NULL == m_bmpbuf)
		return;

	T_U8* p = m_bmpbuf + BMP_HEADINFO_LEN;

	int zoomx = m_img_mutil;
	int zoomy = m_img_mutil;

	for (m=0; m<4; m++)
	{
		for (n=0; n<6; n++)
		{

			x0 = m_framePoint[m][n][0].x ;
			y0 = m_framePoint[m][n][0].y ;
			x1 = m_framePoint[m][n][1].x ;
			y1 = m_framePoint[m][n][1].y ;

			x0 *= zoomx;
			y0 *= zoomy;
			x1 *= zoomx;
			y1 *= zoomy;

			BSum = 0;
			GSum = 0;
			RSum = 0;

			for (i=y0; i<y1; i++)
			{
				for (j=x0; j<x1; j++)
				{
					offset = (m_img_height-i - 1) * m_img_width * 3 + j * 3;
					B = p[offset];
					G = p[offset + 1];
					R = p[offset + 2];

					BSum += B;
					GSum += G;
					RSum += R;
				}
			}

			BAvg = BSum / ((x1 - x0) * (y1 - y0));
			GAvg = GSum / ((x1 - x0) * (y1 - y0));
			RAvg = RSum / ((x1 - x0) * (y1 - y0));	

			m_RGBvalDlg.B_in[m][n] = BAvg;
			m_RGBvalDlg.G_in[m][n] = GAvg;
			m_RGBvalDlg.R_in[m][n] = RAvg;
		}
	}	
}

void CHUE_CALCDlg::OnButtonOpenYuvImg() 
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, "*.yuv", NULL, OFN_HIDEREADONLY,
		"Data File(*.yuv)|*.yuv|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;
	
	SetImgPath((LPSTR)(LPCTSTR)dlg.GetPathName());

	Show_flag = TRUE;
	Invalidate();
	Calculate_flag = FALSE;
	Calc_Input_RGB();
}

void CHUE_CALCDlg::OnPaint() 
{
	int i = 0;
	int j = 0;

	UpdateData(FALSE);

	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	if (m_bRefreshFlag)
	{
		GetBMP();
		m_bRefreshFlag = FALSE;
	}

	BMShow(&dc, m_ImgRect.left, m_ImgRect.top, m_img_width/m_img_mutil, m_img_height/m_img_mutil);
	
	if (!Show_flag)
	{
		return;
	}

	//draw keypoints
	for (i=0; i<4; i++) 
	{
		dc.FillSolidRect(m_keyPoint[i].x+m_ImgRect.left-3, m_keyPoint[i].y+m_ImgRect.top-3, 7, 7, RGB(0, 0, 255));
	}

	//draw select rect line
	CPen linePen(PS_SOLID, 1, RGB(0, 0, 255));
	dc.SelectObject(&linePen);

	for (i=0; i<4; i++)
	{
		for (j=0; j<6; j++)
		{
			dc.MoveTo(m_framePoint[i][j][0].x+m_ImgRect.left, m_framePoint[i][j][0].y+m_ImgRect.top);
			dc.LineTo(m_framePoint[i][j][1].x+m_ImgRect.left, m_framePoint[i][j][0].y+m_ImgRect.top);
			dc.LineTo(m_framePoint[i][j][1].x+m_ImgRect.left, m_framePoint[i][j][1].y+m_ImgRect.top);
			dc.LineTo(m_framePoint[i][j][0].x+m_ImgRect.left, m_framePoint[i][j][1].y+m_ImgRect.top);
			dc.LineTo(m_framePoint[i][j][0].x+m_ImgRect.left, m_framePoint[i][j][0].y+m_ImgRect.top);
		}
	}

	CDialog::OnPaint();
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CHUE_CALCDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	m_bInit = FALSE;

	if (NULL != m_yuvbuf)
	{
		free(m_yuvbuf);
		m_yuvbuf = NULL;
	}

	if (NULL != m_bmpbuf)
	{
		free(m_bmpbuf);
		m_bmpbuf = NULL;
	}

	CDialog::OnClose();
}

void CHUE_CALCDlg::Calc_Frames() 
{
	int i = 0;
	int j = 0;
	
	T_U16 frame_w[4][6] = {0};
	T_U16 frame_h[4][6] = {0};
	T_U16 frame_internal_h[4][5] = {0};
	T_U16 frame_internal_v[3][6] = {0};

	T_U16 width[4] = {0};
	T_U16 height[6] = {0};

	width[0] = m_keyPoint[1].x - m_keyPoint[0].x;
	width[3] = m_keyPoint[2].x - m_keyPoint[3].x;

	width[1] = width[0] * 2 / 3 + width[3] / 3;
	width[2] = width[0] / 3 + width[3] * 2 / 3;


	height[0] = m_keyPoint[3].y - m_keyPoint[0].y;
	height[5] = m_keyPoint[2].y - m_keyPoint[1].y;

	height[1] = height[0] * 4 / 5 + height[5] / 5;
	height[2] = height[0] * 3 / 5 + height[5] * 2 / 5;

	height[3] = height[0] * 2 / 5 + height[5] * 3 / 5;
	height[4] = height[0] / 5 + height[5] * 4 / 5;

	for (i=0; i<6; i++)
	{
		for (j=0; j<4; j++)
		{
			frame_w[j][i] = width[j] / 9 ;
		}
	}

	for (i=0; i<4; i++)
	{
		for (j=0; j<4; j++)
		{
			frame_internal_h[j][i] = (width[j] - frame_w[j][0] * 6) / 5;
		}
	}

	for (j=0; j<4; j++)
	{
		frame_internal_h[j][4] = width[j] - frame_w[j][0] * 6 - frame_internal_h[j][0] * 4;
	}

	for (i=0; i<4; i++)
	{
		for (j=0; j<6; j++)
		{
			frame_h[i][j] = height[j] / 6 ;
		}
	}

	for (i=0; i<2; i++)
	{
		for (j=0; j<6; j++)
		{
			frame_internal_v[i][j] = (height[j] - frame_h[0][j] * 4) / 3;
		}
	}

	for (j=0; j<6; j++)
	{
		frame_internal_v[2][j] = height[j] - frame_h[0][j] * 4 - frame_internal_v[0][j] * 2;
	}

	m_framePoint[0][0][0].x = m_keyPoint[0].x;
	m_framePoint[1][0][0].x = m_keyPoint[0].x * 2 / 3 + m_keyPoint[3].x / 3;
	m_framePoint[2][0][0].x = m_keyPoint[0].x / 3 + m_keyPoint[3].x * 2 / 3;
	m_framePoint[3][0][0].x = m_keyPoint[3].x;

	m_framePoint[0][0][0].y  = m_keyPoint[0].y;
	m_framePoint[0][1][0].y  = m_keyPoint[0].y * 4 / 5 + m_keyPoint[1].y / 5;
	m_framePoint[0][2][0].y  = m_keyPoint[0].y * 3 / 5 + m_keyPoint[1].y * 2 / 5;
	m_framePoint[0][3][0].y  = m_keyPoint[0].y * 2 / 5 + m_keyPoint[1].y * 3 / 5;
	m_framePoint[0][4][0].y  = m_keyPoint[0].y / 5 + m_keyPoint[1].y * 4 / 5;
	m_framePoint[0][5][0].y  = m_keyPoint[1].y;
	
	for (i=0; i<4; i++)
	{
		for (j=1; j<6; j++)
		{
			m_framePoint[i][j][0].x = m_framePoint[i][j-1][0].x + frame_w[i][j-1] + frame_internal_h[i][j-1];
		}

		for (j=0; j<6; j++)
		{
			m_framePoint[i][j][1].x = m_framePoint[i][j][0].x + frame_w[i][j];
		}
	}

	for (j=0; j<6; j++)
	{
		for (i=1; i<4; i++)
		{
			m_framePoint[i][j][0].y = m_framePoint[i-1][j][0].y + frame_h[i-1][j] + frame_internal_v[i-1][j];
		}

		for (i=0; i<4; i++)
		{
			m_framePoint[i][j][1].y = m_framePoint[i][j][0].y + frame_h[i][j];
		}
	}
}

void CHUE_CALCDlg::OnButtonRgbVal() 
{
	// TODO: Add your control notification handler code here
	m_RGBvalDlg.DoModal();
}

void Calc_hue(void *argv)
{
	CWnd *pWnd = NULL;
	CString str;
	CHUE_CALCDlg* pthis = (CHUE_CALCDlg*)argv;
	RGB Original_RGB[24] = {0};
	RGB Target_RGB[24] = {0};
	UINT i = 0, j = 0, m = 0;
	int sat = 0;

	memset(&pthis->hue_para, 0, sizeof(AK_ISP_HUE));
	memset(Original_RGB, 0, sizeof(RGB)*24);
	memset(Target_RGB, 0, sizeof(RGB)*24);

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 6; j++)
		{
			memcpy(&Original_RGB[m].R, &pthis->m_RGBvalDlg.R_in[i][j], 1);
			memcpy(&Original_RGB[m].G, &pthis->m_RGBvalDlg.G_in[i][j], 1);
			memcpy(&Original_RGB[m].B, &pthis->m_RGBvalDlg.B_in[i][j], 1);
			
			memcpy(&Target_RGB[m].R, &pthis->m_RGBvalDlg.R_tar[i][j], 1);
			memcpy(&Target_RGB[m].G, &pthis->m_RGBvalDlg.G_tar[i][j], 1);
			memcpy(&Target_RGB[m].B, &pthis->m_RGBvalDlg.B_tar[i][j], 1);
			m = m + 1;
		}
	}

	pthis->GetDlgItemText(IDC_EDIT_SATURATION, str);
	if (str.IsEmpty())
	{
		AfxMessageBox("THRESHOLD IS NULL, PLS CHk");
		return;
	}
	sat = atoi(str);

	//calc HUE param
	CalcHUEparam(Original_RGB,Target_RGB, &pthis->hue_para, sat);

	pWnd = pthis->GetDlgItem(IDC_STATIC_CAL);
	str.Format("Calculat finish");
	pWnd->SetWindowText(str);

	pthis->GetDlgItem(IDC_BUTTON_GET_YUV_IMG)->EnableWindow(TRUE);//可用
	pthis->GetDlgItem(IDC_BUTTON_OPEN_YUV_IMG)->EnableWindow(TRUE);//可用
	pthis->GetDlgItem(IDC_BUTTON_RGB_VAL)->EnableWindow(TRUE);//可用
	pthis->GetDlgItem(IDC_BUTTON_CALC)->EnableWindow(TRUE);//可用
	pthis->GetDlgItem(IDC_BUTTON_EXPORT_HUE)->EnableWindow(TRUE);//可用

	pthis->Calculate_flag = TRUE;
	pthis->Invalidate();
}

BOOL CHUE_CALCDlg::Creat_Calc_thread(void) 
{	
	if (g_Calc_Thread != INVALID_HANDLE_VALUE)
	{
		Close_Calc_thread();
	}
	g_Calc_Thread = CreateThread(NULL, TRANS_STACKSIZE, (LPTHREAD_START_ROUTINE)Calc_hue, this, 0, NULL);
	if (g_Calc_Thread == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	return TRUE;	
}

void CHUE_CALCDlg::Close_Calc_thread(void) 
{
	if(g_Calc_Thread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_Calc_Thread);
		g_Calc_Thread = INVALID_HANDLE_VALUE;
	}
}

void CHUE_CALCDlg::OnButtonCalc() 
{
	// TODO: Add your control notification handler code here
	CWnd *pWnd = NULL;
	CString str;
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	GetDlgItem(IDC_BUTTON_GET_YUV_IMG)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_OPEN_YUV_IMG)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_RGB_VAL)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_CALC)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_EXPORT_HUE)->EnableWindow(FALSE);//可用

	if (Creat_Calc_thread() == FALSE)
	{
		AfxMessageBox("Creat_Calc_thread fail", MB_OK);
		return;
	}
	
	pWnd = GetDlgItem(IDC_STATIC_CAL);
	str.Format("Calculating...");
	pWnd->SetWindowText(str);
}

void CHUE_CALCDlg::OnButtonExportHue() 
{
	// TODO: Add your control notification handler code here

	export_hue_flag = TRUE;

	AfxMessageBox("export HUE success", MB_OK);
}

void CHUE_CALCDlg::OnKillfocusEditSaturation() 
{
	// TODO: Add your control notification handler code here
	m_slider_saturation.SetPos((int)m_spin_saturation.GetPos());
}

void CHUE_CALCDlg::OnDeltaposSpinSaturation(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	m_slider_saturation.SetPos((int)m_spin_saturation.GetPos());
	*pResult = 0;
}

void CHUE_CALCDlg::OnReleasedcaptureSliderSaturation(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	Saturation = m_slider_saturation.GetPos();
	*pResult = 0;
}

void CHUE_CALCDlg::OnCustomdrawSliderSaturation(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int temp = 0;

	// TODO: Add your control notification handler code here
	m_spin_saturation.SetPos((int)m_slider_saturation.GetPos());
	temp = 	m_slider_saturation.GetPos();
	if (last_Saturation != temp)
	{
		last_Saturation = temp;
	}
	
	*pResult = 0;
}
