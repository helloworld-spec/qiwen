// ImgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "ImgDlg.h"


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

#define IMG_SHOW_WIDTH	320
#define IMG_SHOW_HEIGHT_960P	240
#define IMG_SHOW_HEIGHT_720P	180
#define IMG_SHOW_MUTIL	4
#define IMG_SHOW_MUTIL_1080P	6
#define IMG_SHOW_MUTIL_1536P	6

#define YUV_NV12
#define BMP_HEADINFO_LEN	54

#define CLIP255(x) ((x)>0?((x)<255?(x):255):0)

#define STAND_COLOR_TMP_STATIC_PARA 9

typedef struct Color_Tmp_Static_Para
{
	int grlow;
	int grhigh;
	int gblow;
	int gbhigh;
	int rblow;
	int rbhigh;

}AWB_COLOR_TMP_STATIC;
int Color_Tmp_Para_GB_Mid[STAND_COLOR_TMP_STATIC_PARA] = {112,98,97,91,83,84,75,72, 71};
int Color_Tmp_Para_GR_Mid[STAND_COLOR_TMP_STATIC_PARA] = {65,72,76,81,96,96,103,109,111};
int Color_Tmp[STAND_COLOR_TMP_STATIC_PARA] = {2800,3400,3900,4400,5200,5700,6500,6900,7500};

AWB_COLOR_TMP_STATIC AWB_Static_Para[STAND_COLOR_TMP_STATIC_PARA] =
									 {
										 //A、TL84、D50、D65、D75	、A_1、TL84_1、D50_1、D65_1
										 {48,84,96,165,84,191},//Grlow,Grhigh,Gblow,Gbhigh
										 {74,99,105,125,74,101},
										 {83,135,65,105,35,73},
										 {92,146,66,80,35,52},
										 {99,165,64,77,30,46},										 
										 {64,78,106,117,87,117},
										 {83,100,97,110,62,85},
										 {96,135,74,90,40,57},
										 {92,147,70,84,38,52},
									};

/*
yuv2rgb转化公式：
R = Y+1.4*V
G = Y-0.34*U-0.71*V
B = Y+1.732*U
*/

T_U8* YUV420ToBMP(T_U8* yuv, T_U16 width, T_U16 height)
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
	
	//*bmp_size = 54 + height*line_byte;
	
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




/////////////////////////////////////////////////////////////////////////////
// CImgDlg dialog


CImgDlg::CImgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CImgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImgDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bRefreshFlag = FALSE;
	m_bInit = FALSE;
	ZeroMemory(&m_imgpath, 260);
	ZeroMemory(m_color_tmp, NUM_COLOR_TMP * sizeof(int));
	ZeroMemory(m_Gr, NUM_COLOR_TMP * sizeof(int));
	ZeroMemory(m_Gb, NUM_COLOR_TMP * sizeof(int));
}


void CImgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImgDlg)
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EDIT1, m_color_tmp[0]);
	DDX_Text(pDX, IDC_EDIT2, m_Gr[0]);
	DDX_Text(pDX, IDC_EDIT3, m_Gb[0]);

	DDX_Text(pDX, IDC_EDIT4, m_color_tmp[1]);
	DDX_Text(pDX, IDC_EDIT5, m_Gr[1]);
	DDX_Text(pDX, IDC_EDIT6, m_Gb[1]);

	DDX_Text(pDX, IDC_EDIT7, m_color_tmp[2]);
	DDX_Text(pDX, IDC_EDIT8, m_Gr[2]);
	DDX_Text(pDX, IDC_EDIT9, m_Gb[2]);

	DDX_Text(pDX, IDC_EDIT10, m_color_tmp[3]);
	DDX_Text(pDX, IDC_EDIT11, m_Gr[3]);
	DDX_Text(pDX, IDC_EDIT12, m_Gb[3]);

	DDX_Text(pDX, IDC_EDIT13, m_color_tmp[4]);
	DDX_Text(pDX, IDC_EDIT14, m_Gr[4]);
	DDX_Text(pDX, IDC_EDIT16, m_Gb[4]);
}


BEGIN_MESSAGE_MAP(CImgDlg, CDialog)
	//{{AFX_MSG_MAP(CImgDlg)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON_RESET, OnButtonReset)
	ON_BN_CLICKED(IDC_CHECK_A, OnCheckAEnable)
	ON_BN_CLICKED(IDC_CHECK_TL84, OnCheckTL84Enable)
	ON_BN_CLICKED(IDC_CHECK_D50, OnCheckD50Enable)
	ON_BN_CLICKED(IDC_CHECK_D65, OnCheckD65Enable)
	ON_BN_CLICKED(IDC_CHECK_D75, OnCheckD75Enable)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT1, OnCheckDfault1Enable)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT2, OnCheckDfault2Enable)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT3, OnCheckDfault3Enable)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT4, OnCheckDfault4Enable)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT5, OnCheckDfault5Enable)

	ON_BN_CLICKED(IDC_BUTTON_GET_YUV_IMG, OnButtonGetYuvImg)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_YUV_IMG, OnButtonOpenYuvImg)

	ON_WM_TIMER()
	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImgDlg message handlers
BOOL CImgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_yuvbuf = NULL;
	m_bmpbuf = NULL;
	m_keyPoint[0].x = m_keyPoint[0].y = 0;
	m_keyPoint[1].x = m_keyPoint[1].y = 0;
	m_drag = FALSE;
	m_moveflag = -1;

	m_img_width = YUV_WIDTH_960P;
	m_img_height = YUV_HEIGHT_960P;
	m_img_mutil = IMG_SHOW_MUTIL;

	GrLow = GrHigh = GrAvg = 0;
	GbLow = GbHigh = GbAvg = 0;
	RbLow = RbHigh = RbAvg = 0;
	
	m_CoordinateDlg.A_enable = 1;
	m_CoordinateDlg.TL84_enable = 1;
	m_CoordinateDlg.D50_enable = 1;
	m_CoordinateDlg.D65_enable = 1;
	m_CoordinateDlg.D75_enable = 1;
	m_CoordinateDlg.Default1_enable = 1;
	m_CoordinateDlg.Default2_enable = 1;
	m_CoordinateDlg.Default3_enable = 1;
	m_CoordinateDlg.Default4_enable = 1;
	m_CoordinateDlg.Default5_enable = 1;

	((CButton *)GetDlgItem(IDC_CHECK_A))->SetCheck(m_CoordinateDlg.A_enable);
	((CButton *)GetDlgItem(IDC_CHECK_TL84))->SetCheck(m_CoordinateDlg.TL84_enable);
	((CButton *)GetDlgItem(IDC_CHECK_D50))->SetCheck(m_CoordinateDlg.D50_enable);
	((CButton *)GetDlgItem(IDC_CHECK_D65))->SetCheck(m_CoordinateDlg.D65_enable);
	((CButton *)GetDlgItem(IDC_CHECK_D75))->SetCheck(m_CoordinateDlg.D75_enable);
	
	((CButton *)GetDlgItem(IDC_CHECK_DEFAULT1))->SetCheck(m_CoordinateDlg.Default1_enable);
	((CButton *)GetDlgItem(IDC_CHECK_DEFAULT2))->SetCheck(m_CoordinateDlg.Default2_enable);
	((CButton *)GetDlgItem(IDC_CHECK_DEFAULT3))->SetCheck(m_CoordinateDlg.Default3_enable);
	((CButton *)GetDlgItem(IDC_CHECK_DEFAULT4))->SetCheck(m_CoordinateDlg.Default4_enable);
	((CButton *)GetDlgItem(IDC_CHECK_DEFAULT5))->SetCheck(m_CoordinateDlg.Default5_enable);

	CRect rect;

	this->GetClientRect(&rect);

	m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_960P+2);

	m_bInit = TRUE;


	CRect clientRect, coor_rect;
	
	this->GetClientRect(&clientRect); 
	
	m_CoordinateDlg.Create(IDD_DIALOG_COORDINATE, this);
	m_CoordinateDlg.GetClientRect(&coor_rect); 
	
	coor_rect.left = clientRect.left+IMG_SHOW_WIDTH + 120;
	coor_rect.top = clientRect.top + 2;
	coor_rect.right += 512;
	coor_rect.bottom += 512;
	m_CoordinateDlg.SetMessageWindow(m_pMessageWnd);
	m_CoordinateDlg.MoveWindow(&coor_rect);
	m_CoordinateDlg.ShowWindow(SW_SHOW);
	
	m_CoordinateDlg.InPoint_GBR_num = 0;
	m_CoordinateDlg.InPoint_GBR = NULL;

	m_timer = SetTimer(1, 100, NULL);

	Invalidate();
	return TRUE;
}

void CImgDlg::OnPaint() 
{	
	UpdateData(TRUE);

	CPaintDC dc(this);

	if (m_bRefreshFlag)
	{
		GetBMP();
		m_bRefreshFlag = FALSE;
	}
	
	BMShow(&dc, m_ImgRect.left, m_ImgRect.top, m_img_width/m_img_mutil, m_img_height/m_img_mutil);


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

	
	UpdateData(FALSE);
	
	CDialog::OnPaint();
}

void CImgDlg::OnClose() 
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

	m_CoordinateDlg.Close();

	CDialog::OnClose();
	DestroyWindow();
}

void CImgDlg::SetImgPath(char* path) 
{
	strcpy(m_imgpath, path);

	if (m_bInit)
	{
		Invalidate();
	}

	m_bRefreshFlag = TRUE;
}

void CImgDlg::BMShow(CDC *pDC, int x, int y, int width, int height) 
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

bool CImgDlg::GetBMP(void) 
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

void CImgDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if (!m_ImgRect.PtInRect(point)) {
		CDialog::OnLButtonDown(nFlags, point);
		return;
	}

	CPoint InPoint;
	InPoint.x = point.x - m_ImgRect.left;
	InPoint.y = point.y - m_ImgRect.top;


	//if ((m_keyPoint[0].x == m_keyPoint[1].x)
	//	&& (m_keyPoint[0].y == m_keyPoint[1].y))
	{
		m_keyPoint[0].x = InPoint.x;
		m_keyPoint[0].y = InPoint.y;

		m_keyPoint[1].x = InPoint.x;
		m_keyPoint[1].y = InPoint.y;

		m_drag = TRUE;
		m_moveflag = 1;

		InvalidateRect(m_ImgRect, FALSE);
	}
#if 0
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

void CImgDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_drag)
		m_drag = FALSE;

	CDialog::OnLButtonUp(nFlags, point);

	Calc_Ratio();
}

void CImgDlg::OnMouseMove(UINT nFlags, CPoint point) 
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
	
	if (m_drag && m_moveflag >= 0 && (unsigned int)(m_moveflag) < 2) 
	{
		m_keyPoint[m_moveflag] = InPoint;
		InvalidateRect(m_ImgRect, FALSE);
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CImgDlg::OnButtonReset() 
{
	// TODO: Add your control notification handler code here
	m_keyPoint[0].x = m_keyPoint[0].y = 0;
	m_keyPoint[1].x = m_keyPoint[1].y = 0;

	GrLow = GrHigh = GrAvg = 0;
	GbLow = GbHigh = GbAvg = 0;
	RbLow = RbHigh = RbAvg = 0;

	SetDataValue();
	Invalidate();
}

void CImgDlg::Calc_Ratio() 
{
	// TODO: Add your control notification handler code here
	if (NULL == m_bmpbuf)
		return;

	T_U8* p = m_bmpbuf + BMP_HEADINFO_LEN;

	int x0 = m_keyPoint[0].x < m_keyPoint[1].x ? m_keyPoint[0].x : m_keyPoint[1].x;
	int y0 = m_keyPoint[0].y < m_keyPoint[1].y ? m_keyPoint[0].y : m_keyPoint[1].y;

	int x1 = m_keyPoint[0].x > m_keyPoint[1].x ? m_keyPoint[0].x : m_keyPoint[1].x;
	int y1 = m_keyPoint[0].y > m_keyPoint[1].y ? m_keyPoint[0].y : m_keyPoint[1].y;

	if (x0 == x1 || y0 == y1)
		return;

	int zoomx = m_img_mutil;
	int zoomy = m_img_mutil;

	x0 *= zoomx;
	y0 *= zoomy;
	x1 *= zoomx;
	y1 *= zoomy;

	UINT offset = 0;
	T_U8 B = 0, G = 0, R = 0, Y = 0;
	float Gr = 0, Gb = 0, Rb = 0;
	BOOL gbr_flag = TRUE;
	BOOL gbr_range_flag = TRUE;
	UINT idex = 0;
	UINT buf_len = 0;


	float GrSum = 0, GbSum = 0, RbSum = 0;
	T_U32 RSum = 0, GSum = 0, BSum = 0, YSum = 0;

	Bavg = Gavg = Ravg = Yavg = 0;

	GrLow = GbLow = RbLow = 1023;
	GrHigh = GbHigh = RbHigh = 0;
	GrAvg = GbAvg = RbAvg = 0;

	buf_len = (x1 - x0) * (y1 - y0)*sizeof(CPoint);
	if (m_CoordinateDlg.InPoint_GBR != NULL)
	{
		free(m_CoordinateDlg.InPoint_GBR);
		m_CoordinateDlg.InPoint_GBR = NULL;
	}
	m_CoordinateDlg.InPoint_GBR = (CPoint *)malloc(buf_len);
	if (m_CoordinateDlg.InPoint_GBR == NULL)
	{
		AfxMessageBox("获取gbr的内存分区失败", MB_OK);
		gbr_flag = FALSE;
	}
	else
	{
		memset(m_CoordinateDlg.InPoint_GBR, 0, buf_len);
	}
	
	for (int i=y0; i<y1; i++)
	{
		for (int j=x0; j<x1; j++)
		{
			offset = (m_img_height-i) * m_img_width * 3 + j * 3;
			B = p[offset];
			G = p[offset + 1];
			R = p[offset + 2];

			if (0 == R)
			{
				R = 1;
			}
			Y = (306* R + 601*G + 117*B)>>10;

			if (m_CoordinateDlg.m_coor_Isp_wb.p_awb.y_low <= Y && Y <= m_CoordinateDlg.m_coor_Isp_wb.p_awb.y_high)
			{
				gbr_range_flag = TRUE;
			}
			else
			{
				gbr_range_flag = FALSE;
			}

			Gr = (float)G / (float)R;
			if (gbr_flag && gbr_range_flag)
			{
				m_CoordinateDlg.InPoint_GBR[idex].x = Gr* 64*2;
			}
			


			if (0 == B)
			{
				B = 1;
			}

			Gb = (float)G / (float)B;
			Rb = (float)R / (float)B;
			if (gbr_flag && gbr_range_flag)
			{
				m_CoordinateDlg.InPoint_GBR[idex].y = COORDINATE_WINDOW_WIDTH - Gb * 64*2;
				idex++;
			}

			GrSum += Gr;
			GbSum += Gb;
			RbSum += Rb;

			if (Gr < GrLow)
			{
				GrLow = Gr;
			}

			if (Gb < GbLow)
			{
				GbLow = Gb;
			}

			if (Rb < RbLow)
			{
				RbLow = Rb;
			}

			if (Gr > GrHigh)
			{
				GrHigh = Gr;
			}

			if (Gb > GbHigh)
			{
				GbHigh = Gb;
			}

			if (Rb > RbHigh)
			{
				RbHigh = Rb;
			}

			RSum += R;
			GSum += G;
			BSum += B;
			YSum += Y;
		}
	}
	
	GrAvg = GrSum / ((x1 - x0) * (y1 - y0));
	GbAvg = GbSum / ((x1 - x0) * (y1 - y0));
	RbAvg = RbSum / ((x1 - x0) * (y1 - y0));	

	Ravg = RSum / ((x1 - x0) * (y1 - y0));
	Gavg = GSum / ((x1 - x0) * (y1 - y0));
	Bavg = BSum / ((x1 - x0) * (y1 - y0));
	Yavg = YSum / ((x1 - x0) * (y1 - y0));
	
	SetDataValue();
	Invalidate();
	if (gbr_flag)
	{
		m_CoordinateDlg.InPoint_GBR_num = idex;
	}
	//m_CoordinateDlg.inset_key();
	//free(m_CoordinateDlg.InPoint_GBR);
}

void CImgDlg::SetDataValue(void)
{
	CWnd *pWnd = NULL;
	CString str;
	
	pWnd = GetDlgItem(IDC_STATIC_GRLOW);
	str.Format("%.2f", GrLow);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_GRHIGH);
	str.Format("%.2f", GrHigh);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_GRAVG);
	str.Format("%.2f", GrAvg);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_GBLOW);
	str.Format("%.2f", GbLow);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_GBHIGH);
	str.Format("%.2f", GbHigh);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_GBAVG);
	str.Format("%.2f", GbAvg);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_RBLOW);
	str.Format("%.2f", RbLow);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_RBHIGH);
	str.Format("%.2f", RbHigh);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_RBAVG);
	str.Format("%.2f", RbAvg);
	pWnd->SetWindowText(str);

	//*64
	pWnd = GetDlgItem(IDC_STATIC_GRLOW2);
	str.Format("%.0f", GrLow * 64);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_GRHIGH2);
	str.Format("%.0f", GrHigh * 64);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_GRAVG2);
	str.Format("%.0f", GrAvg * 64);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_GBLOW2);
	str.Format("%.0f", GbLow * 64);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_GBHIGH2);
	str.Format("%.0f", GbHigh * 64);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_GBAVG2);
	str.Format("%.0f", GbAvg * 64);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_RBLOW2);
	str.Format("%.0f", RbLow * 64);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_RBHIGH2);
	str.Format("%.0f", RbHigh * 64);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_RBAVG2);
	str.Format("%.0f", RbAvg * 64);
	pWnd->SetWindowText(str);

	//R G B Y avg
	pWnd = GetDlgItem(IDC_STATIC_RAVG);
	str.Format("%d", Ravg);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_GAVG);
	str.Format("%d", Gavg);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_BAVG);
	str.Format("%d", Bavg);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_YAVG);
	str.Format("%d", Yavg);
	pWnd->SetWindowText(str);

	UpdateData(FALSE);
}


T_U8* CImgDlg::LoadYuvData(char* path) 
{
	CFile file;
	CFileException e;
	CRect rect;

	this->GetClientRect(&rect);

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

		m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_720P+2);
	}
	else if (YUV_WIDTH_960P * YUV_HEIGHT_960P * 3 / 2 == size)
	{
		m_img_mode = IMG_MODE_960P;
		m_img_width = YUV_WIDTH_960P;
		m_img_height = YUV_HEIGHT_960P;
		m_img_mutil = IMG_SHOW_MUTIL;

		m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_960P+2);
	}
	else if (YUV_WIDTH_1080P * YUV_HEIGHT_1080P * 3 / 2 == size)
	{
		m_img_mode = IMG_MODE_1080P;
		m_img_width = YUV_WIDTH_1080P;
		m_img_height = YUV_HEIGHT_1080P;
		m_img_mutil = IMG_SHOW_MUTIL_1080P;

		m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_720P+2);
	}
	else if (YUV_WIDTH_1536P * YUV_HEIGHT_1536P * 3 / 2 == size)
	{
		m_img_width = YUV_WIDTH_1536P;
		m_img_height = YUV_HEIGHT_1536P;
		m_img_mutil = IMG_SHOW_MUTIL_1536P;

		m_ImgRect.SetRect(rect.left+2, rect.top+2, YUV_WIDTH_1536P / IMG_SHOW_MUTIL_1536P + 2, YUV_HEIGHT_1536P / IMG_SHOW_MUTIL_1536P + 2);
	}
	else if (1536 * 1536 * 3 / 2 == size)
	{
		m_img_width = 1536;
		m_img_height = 1536;
		m_img_mutil = IMG_SHOW_MUTIL_1536P;

		m_ImgRect.SetRect(rect.left+2, rect.top+2, 1536 / IMG_SHOW_MUTIL_1536P + 2, 1536 / IMG_SHOW_MUTIL_1536P + 2);
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

void CImgDlg::OnCheckAEnable() 
{	
	BOOL check = FALSE;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_A))->GetCheck();
	if (check)
	{
		m_CoordinateDlg.A_enable = TRUE;
	}
	else
	{
		m_CoordinateDlg.A_enable = FALSE;
	}

	UpdateData(TRUE);
	Invalidate();
}
void CImgDlg::OnCheckTL84Enable() 
{
	BOOL check = FALSE;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_TL84))->GetCheck();
	if (check)
	{
		m_CoordinateDlg.TL84_enable = TRUE;
	}
	else
	{
		m_CoordinateDlg.TL84_enable = FALSE;
	}

	UpdateData(TRUE);
	Invalidate();
}
void CImgDlg::OnCheckD50Enable() 
{
	BOOL check = FALSE;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_D50))->GetCheck();
	if (check)
	{
		m_CoordinateDlg.D50_enable = TRUE;
	}
	else
	{
		m_CoordinateDlg.D50_enable = FALSE;
	}

	UpdateData(TRUE);
	Invalidate();
}
void CImgDlg::OnCheckD65Enable() 
{
	BOOL check = FALSE;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_D65))->GetCheck();
	if (check)
	{
		m_CoordinateDlg.D65_enable = TRUE;
	}
	else
	{
		m_CoordinateDlg.D65_enable = FALSE;
	}

	UpdateData(TRUE);
	Invalidate();
}
void CImgDlg::OnCheckD75Enable() 
{
	BOOL check = FALSE;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_D75))->GetCheck();
	if (check)
	{
		m_CoordinateDlg.D75_enable = TRUE;
	}
	else
	{
		m_CoordinateDlg.D75_enable = FALSE;
	}

	UpdateData(TRUE);
	Invalidate();
}
void CImgDlg::OnCheckDfault1Enable() 
{
	BOOL check = FALSE;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_DEFAULT1))->GetCheck();
	if (check)
	{
		m_CoordinateDlg.Default1_enable = TRUE;
	}
	else
	{
		m_CoordinateDlg.Default1_enable = FALSE;
	}

	UpdateData(TRUE);
	Invalidate();
}
void CImgDlg::OnCheckDfault2Enable() 
{
	BOOL check = FALSE;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_DEFAULT2))->GetCheck();
	if (check)
	{
		m_CoordinateDlg.Default2_enable = TRUE;
	}
	else
	{
		m_CoordinateDlg.Default2_enable = FALSE;
	}

	UpdateData(TRUE);
	Invalidate();
}
void CImgDlg::OnCheckDfault3Enable() 
{
	BOOL check = FALSE;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_DEFAULT3))->GetCheck();
	if (check)
	{
		m_CoordinateDlg.Default3_enable = TRUE;
	}
	else
	{
		m_CoordinateDlg.Default3_enable = FALSE;
	}

	UpdateData(TRUE);
	Invalidate();
}
void CImgDlg::OnCheckDfault4Enable() 
{
	BOOL check = FALSE;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_DEFAULT4))->GetCheck();
	if (check)
	{
		m_CoordinateDlg.Default4_enable = TRUE;
	}
	else
	{
		m_CoordinateDlg.Default4_enable = FALSE;
	}

	UpdateData(TRUE);
	Invalidate();
}
void CImgDlg::OnCheckDfault5Enable() 
{
	BOOL check = FALSE;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_DEFAULT5))->GetCheck();
	if (check)
	{
		m_CoordinateDlg.Default5_enable = TRUE;
	}
	else
	{
		m_CoordinateDlg.Default5_enable = FALSE;
	}

	UpdateData(TRUE);
	Invalidate();
}

void CImgDlg::Img_SetConnectState(bool bConnect) 
{
	// TODO: Add your control notification handler code here
	
	m_img_bConnect = bConnect;
}

void CImgDlg::OnButtonGetYuvImg() 
{
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

	UpdateData(TRUE);
	
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_YUV_IMG, DIALOG_STAT, 0);

	OnButtonReset();
	m_CoordinateDlg.CoordinateReset();	
}

void CImgDlg::OnButtonOpenYuvImg() 
{
	CFileDialog dlg(TRUE, "*.yuv", NULL, OFN_HIDEREADONLY,
		"Data File(*.yuv)|*.yuv|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;

	
	SetImgPath((LPSTR)(LPCTSTR)dlg.GetPathName());

	OnButtonReset();
	m_CoordinateDlg.CoordinateReset();		
}

void CImgDlg::Calc_Edit_GrGb() 
{
	int i = 0;
	int grlow_tmp, grhigh_tmp, gblow_tmp, gbhigh_tmp;

	UpdateData(TRUE);
	
	for (i = 0; i < NUM_COLOR_TMP; i++)
	{
		grlow_tmp = m_CoordinateDlg.m_keyPoint_GBR[i][0].x / GBR_MUTIL;
		grhigh_tmp = m_CoordinateDlg.m_keyPoint_GBR[i][1].x / GBR_MUTIL;
		gblow_tmp = (COORDINATE_WINDOW_WIDTH - m_CoordinateDlg.m_keyPoint_GBR[i][1].y) / GBR_MUTIL;
		gbhigh_tmp = (COORDINATE_WINDOW_WIDTH - m_CoordinateDlg.m_keyPoint_GBR[i][0].y) / GBR_MUTIL;

		m_Gr[i] = (grlow_tmp + grhigh_tmp) / 2;
		m_Gb[i] = (gblow_tmp + gbhigh_tmp) / 2;
	}

	UpdateData(FALSE);

}

void CImgDlg::Calc_WB() 
{
	//int GB_Input_Mid[STAND_COLOR_TMP_STATIC_PARA] = {134,119,112,101,95,89,81,77};
	//int GR_Input_Mid[STAND_COLOR_TMP_STATIC_PARA] = {62,70,75,82,88,95,98,103};
	int i = 0, count=0;
	double Total_GB = 0,Total_GR = 0,Total_RB = 0,Coff_gb,Coff_gr,Coff_rb;
	double refer_gb = 0,refer_gr = 0,refer_rb = 0,rb = 0;	
	int color_num = 0;

	UpdateData(TRUE);

	for (i=0; i<NUM_COLOR_TMP; i++)
	{
		if (0 != m_color_tmp[i])
			color_num++;
		else
			break;
	}

	if (0 == color_num)
		return;

	for (count=0; count<color_num; count++)
	{				
		for (i = 0; i < STAND_COLOR_TMP_STATIC_PARA-1;i++)
		{
			refer_gb = 0;
			refer_gr = 0;
			refer_rb = 0;
			 
			//calc referce GB/GR
			if (m_color_tmp[count] == Color_Tmp[i])
			{
				refer_gb = Color_Tmp_Para_GB_Mid[i];
				refer_gr = Color_Tmp_Para_GR_Mid[i];
			}
			else if (m_color_tmp[count] == Color_Tmp[i+1])
			{
				refer_gb = Color_Tmp_Para_GB_Mid[i+1];
				refer_gr = Color_Tmp_Para_GR_Mid[i+1];
			}
			else if(m_color_tmp[count] > Color_Tmp[i] && m_color_tmp[count] < Color_Tmp[i+1] )
			{
				refer_gb =  Color_Tmp_Para_GB_Mid[i] - ((double)(Color_Tmp_Para_GB_Mid[i]-Color_Tmp_Para_GB_Mid[i+1])*(Color_Tmp[i]-m_color_tmp[count])/(Color_Tmp[i]-Color_Tmp[i+1])+0.5);
				refer_gr =  Color_Tmp_Para_GR_Mid[i] - ((double)(Color_Tmp_Para_GR_Mid[i]-Color_Tmp_Para_GR_Mid[i+1])*(Color_Tmp[i]-m_color_tmp[count])/(Color_Tmp[i]-Color_Tmp[i+1])+0.5);
			}

			
			if(refer_gb == 0 || refer_gr == 0)
				continue;

			refer_rb = refer_gb*64/refer_gr;

			rb = m_Gb[count]*64/m_Gr[count];
			
			Coff_gr = m_Gr[count]/refer_gr;
			Coff_gb = m_Gb[count]/refer_gb;
			Coff_rb = rb/refer_rb;

			Total_GB += Coff_gb;
			Total_GR += Coff_gr;
			Total_RB += Coff_rb;

			break;
		}
	
	}

	Coff_gb = Total_GB/color_num;
	Coff_gr = Total_GR/color_num;
	Coff_rb = Total_RB/color_num;

	for (i=0; i<STAND_COLOR_TMP_STATIC_PARA; i++)
	{
		m_CoordinateDlg.m_calc_awb.gr_low[i] = (int)(AWB_Static_Para[i].grlow*Coff_gr+0.5l);
		m_CoordinateDlg.m_calc_awb.gr_high[i] = (int)(AWB_Static_Para[i].grhigh*Coff_gr+0.5l);
		m_CoordinateDlg.m_calc_awb.gb_low[i] = (int)(AWB_Static_Para[i].gblow*Coff_gb+0.5l);
		m_CoordinateDlg.m_calc_awb.gb_high[i] = (int)(AWB_Static_Para[i].gbhigh*Coff_gb+0.5l);
		m_CoordinateDlg.m_calc_awb.rb_low[i] = (int)(AWB_Static_Para[i].rblow*Coff_rb+0.5l);
		m_CoordinateDlg.m_calc_awb.rb_high[i] = (int)(AWB_Static_Para[i].rbhigh*Coff_rb+0.5l);
	}
	
	m_CoordinateDlg.ShowCalcWbInfo();
	
}

void CImgDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (1 == nIDEvent)
	{
		if (m_CoordinateDlg.m_update_flag)
		{
			Calc_Edit_GrGb();
			m_CoordinateDlg.m_update_flag = FALSE;
		}

		if (m_CoordinateDlg.m_calc_flag)
		{
			Calc_WB();
			m_CoordinateDlg.m_calc_flag = FALSE;
		}
	}

	CDialog::OnTimer(nIDEvent);
}



