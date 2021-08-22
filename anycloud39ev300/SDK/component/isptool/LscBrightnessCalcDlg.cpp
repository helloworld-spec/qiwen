// LscBrightnessCalcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "LscBrightnessCalcDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BRIGHTNESS_IMAGE_WIDTH		256
#define BRIGHTNESS_IMAGE_HEIGHT		256

#define YUV_WIDTH_720P		1280
#define YUV_HEIGHT_720P		720
#define YUV_WIDTH_960P		1280
#define YUV_HEIGHT_960P		960
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

#define MAXIMAGERESOLITION 2048
#define  GAIN_ARRAY_SIZE 30
#define DISTANCESEARCHAREA 50
#define WIDTHBYTES(bits) (((bits)+31)/32*4)

typedef struct Map_gain{
	int distance;
	float gain;
}GAINMAP;

typedef struct{
	int xref;
	int yref;
	int lsc_shift;
	int range[10];
	int BB_R[10];
	int CC_R[10];
	int BB_G[10];
	int CC_G[10];
	int BB_B[10];
	int CC_B[10];
}LSC_ATTR;


/*
yuv2rgb转化公式：
R = Y+1.4*V
G = Y-0.34*U-0.71*V
B = Y+1.732*U
*/

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

 
static int Most_Bright_Position(unsigned char* yuv420,int *Mid_y,int *Mid_x,unsigned long width,unsigned long height)
{
	int i=0,j=0,sum=0,m=0,k=0;
	int max_value = -99999;
	int mid_x_pos = -9999;
	int mid_y_pos = -9999;

	int MaxLumi_x_pos[MAXIMAGERESOLITION] = {0};
	int MaxLumi_y_pos[MAXIMAGERESOLITION] = {0};
	int avg_line[MAXIMAGERESOLITION] = {0};

	
	//保证图像相对中心点的对称性
	//计算每一行灰度值最大的x轴坐标
	for(i = 0;i < height;i+=8)
	{


		//计算8行累加值
		for(j = 0; j < width;j++)
		{
			sum = 0;
			for(k=0; k<8; k++)
				sum += yuv420[(i+k)*width+j];
			avg_line[j] = sum/8;
		}

		//计算左边最大值
		max_value = -9999;
		for(j = 2; j < width-2;j++)
		{
			int avg_value;

			avg_value = (avg_line[j-2]+avg_line[j-1]+avg_line[j]+avg_line[j+1]+avg_line[j+2])/16;
			if(avg_value > max_value)
			{
				max_value = avg_value;
				m = j;
			}
		}
		MaxLumi_x_pos[i/8] = m;

		//计算右边最大值,取中间值
		max_value = -9999;
		for(j = width-3; j >= 2;j--)
		{
			int avg_value;

			avg_value = (avg_line[j-2]+avg_line[j-1]+avg_line[j]+avg_line[j+1]+avg_line[j+2])/16;
			if(avg_value > max_value)
			{
				max_value = avg_value;
				m = j;
			}
		}
		MaxLumi_x_pos[i/8] = (m+MaxLumi_x_pos[i/8])/2;
	}

	//每一行最大灰度值x轴坐标取其平均值
	sum = 0;
	for(i = 0; i < height/8;i++)
		sum += MaxLumi_x_pos[i];
		
	mid_x_pos = sum / (height/8);
	
	//计算每一列灰度值最大的y轴坐标
	for(i = 0;i < width;i+=16)
	{
		//计算16列
		for(j = 0; j < height;j++)
		{
			sum = 0;
			for(k=0; k<16; k++)
				sum += yuv420[j*width+i+k];
			avg_line[j] = sum/8;
		}

		//计算上边最大值
		max_value = -9999;
		for(j = 2; j < height-2;j++)
		{
			int avg_value;

			avg_value = (avg_line[j-2]+avg_line[j-1]+avg_line[j]+avg_line[j+1]+avg_line[j+2])/16;
			if(avg_value > max_value)
			{
				max_value = avg_value ;
				m = j;
			}
		}
		MaxLumi_y_pos[i/16] = m;

		//计算下边最大值
		max_value = -9999;
		for(j = height-3; j >=2;j--)
		{
			int avg_value;

			avg_value = (avg_line[j-2]+avg_line[j-1]+avg_line[j]+avg_line[j+1]+avg_line[j+2])/16;
			if(avg_value > max_value)
			{
				max_value = avg_value ;
				m = j;
			}
		}
		MaxLumi_y_pos[i/16] = (m+MaxLumi_y_pos[i/16])/2;
	}
	
	//每一列最大灰度值y轴坐标取其平均值
	sum = 0;
	for(i = 0; i < width/16;i++)
		sum += MaxLumi_y_pos[i];
		
	
	mid_y_pos = sum / (width/16);
	

	*Mid_y = mid_y_pos;
	*Mid_x = mid_x_pos;
	return 0;
}

static int BMP_Split(T_U8 *src_img, T_U8 *R, T_U8 *G, T_U8 *B, T_U16 height)
{
	
	DWORD width = 0;
	//DWORD height;
	WORD  biBitCount = 0;
	T_U8 *dst_prt = NULL,*bmp = NULL;

	unsigned long line_byte = 0;
	T_U16 i = 0,j = 0;


	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;

	if(NULL == src_img)
	{
		printf(" data is NULL\n");
		return -1;
	}

	memset(&bf, 0, sizeof(bf));
	memset(&bi, 0, sizeof(bi));

	bmp = src_img;

	memcpy(&bf,src_img,sizeof(bf));
	memcpy(&bi,&src_img[14],sizeof(bi));
	
	//这个本来应该用bi.biHeight赋值给height，但是查看内存的数据，是个错误的值。还没找到这个问题的原因。你可以使用bi.biHeight调试看能不能得到正确的图像高度
	//height = HEIGHT; //1536 
	width  = bi.biWidth;
	biBitCount = bi.biBitCount;//每一个像素由24 bit
	line_byte = WIDTHBYTES(width*bi.biBitCount);
	
	dst_prt = bmp+54+(height-1)*line_byte;


	for(i = 0; i < height;i++)
	{
		for(j=0;j< width;j++)
		{
			*B++ = dst_prt[3*j];
			*G++ = dst_prt[3*j+1];
			*R++ = dst_prt[3*j+2];
		}

		dst_prt -= line_byte;
	}

	return 0;
}


static int Distance(int x1, int x2, int y1, int y2)
{
	return (((x1)-(x2))*((x1)-(x2))+((y1)-(y2))*((y1)-(y2)));
}


int calc_shift_range(LSC_ATTR *lsc_para, T_U8 *subchannel, int width, int height)
{
	int i = 0,j = 0;
	int x = 0,y = 0;
	int range = 0;
	int lsc_shift = 0;
	int Lum[1024] = {0};
	int count[1024] = {0};
	int centerX = 0, centerY = 0;

	centerX = lsc_para->xref;
	centerY = lsc_para->yref;
	if(centerX>width/2)//make sure the distance from Center point to  original point is the longest 
		x = centerX;
	else
		x = width-centerX;

	if(centerY>height/2)
		y = centerY;
	else
		y = height-centerY;

	range = x*x+y*y;
	lsc_shift = 0;
	while(range>1024)
	{
		lsc_shift++;
		range = range>>1;
	}
	//printf("Shift:%d\n",lsc_shift);
	lsc_para->lsc_shift = lsc_shift;
	memset(Lum, 0, sizeof(Lum));
	memset(count, 0, sizeof(count));

	for(i=2; i<height-2; i++)
		for(j=2; j<width-2; j++)	//skip black border
		{
			range = Distance(j, centerX, i, centerY);
			range = range>>lsc_shift;
			Lum[range] += subchannel[i*width+j];
			count[range] ++;
		}

	for(i=0; i<1024; i++)
	{
		if(count[i]>0)
			Lum[i] = Lum[i] /count[i];
	}

	{
		{
			//debug code
			FILE *fp1;

			fp1 = fopen("lum.txt","wt");

			for (i = 0; i < 1024; i++)
			{
				if(Lum[i]!=0)
					fprintf(fp1, "%d\n", Lum[i]);
			}
			fclose(fp1);

		}
	}

	for(i=0; i<1024; i++)
	{
		if(Lum[i]<Lum[0] -2)
			break;
	}

	lsc_para->range[0] = 0;
	lsc_para->range[1] = i;
	j=40;
	for(i=2; i<10; i++)
	{
		lsc_para->range[i] = lsc_para->range[i-1]+j;//100;
		j+=20;
	}

	for(i = 0;i < 10;i++)
		printf("%d ",lsc_para->range[i]);
	printf("\n");
	return 0;
}


int LSC_RGB(LSC_ATTR *lsc_para, T_U8 *Rchannel, T_U8 *Gchannel, T_U8 *Bchannel, int width, int height, int level)
{
	int i = 0;
	int x = 0,y = 0;
	int range = 0;
	int lsc_shift = 0;
	int centerX = 0, centerY = 0;
	int Ravg[10] = {0},Gavg[10] = {0},Bavg[10] = {0};
	int cnt[10] = {0};
	int Rgain[10] = {0}, Ggain[10] = {0}, Bgain[10] = {0};
	float strength = 0;
 
	lsc_shift = lsc_para->lsc_shift;
	centerX = lsc_para->xref;
	centerY = lsc_para->yref;
 
	memset(Ravg, 0, sizeof(Ravg));
	memset(Gavg, 0, sizeof(Gavg));
	memset(Bavg, 0, sizeof(Bavg));
	memset(cnt, 0, sizeof(cnt));
	for(y=2; y<height-2; y++)
		for(x=2; x<width-2; x++) //skip black border
		{
			range = Distance(x, centerX, y, centerY);
			range = range>>lsc_shift;
			for(i=0; i<10; i++)
			{
				if(range>=lsc_para->range[i]-2 && range<=lsc_para->range[i]+2)
				{
					Ravg[i] +=Rchannel[y*width+x];
					Gavg[i] +=Gchannel[y*width+x];
					Bavg[i] +=Bchannel[y*width+x];
					cnt[i]++;
				}
			}
		}
	for(i= 0; i<10; i++)
	{
		if(cnt[i]>0)
		{
			Ravg[i] = Ravg[i]/cnt[i];
			Gavg[i] = Gavg[i]/cnt[i];
			Bavg[i] = Bavg[i]/cnt[i];
		}
	}

	Rgain[0] = 256;
	Ggain[0] = 256;
	Bgain[0] = 256;
 
	strength = level*0.1/100+0.9; //translate 0-100 to 0.9-1.0
	for(i= 1; i<9; i++)
	{
		float target;
 
		if(cnt[i+1]>0)
		{
			int maxStrength;
			int tmp;
 
			maxStrength = 0;
			target = Gavg[i]*strength;
			if(Gavg[i+1]<target)
				Ggain[i] = target*256/Gavg[i+1];
			else
			{
				tmp = Gavg[i+1]*256/Gavg[i];
				if(tmp>maxStrength)
					maxStrength = tmp;
			}
 
			target = Ravg[i]*strength;
			if(Ravg[i+1]<target)
				Rgain[i] = target*256/Ravg[i+1];
			else
			{
				tmp = Ravg[i+1]*256/Ravg[i];
			if(tmp>maxStrength)
				maxStrength = tmp;
			}

			target = Bavg[i]*strength;
			if(Bavg[i+1]<target)
				Bgain[i] = target*256/Bavg[i+1];
			else
			{
				tmp = Bavg[i+1]*256/Bavg[i];
				if(tmp>maxStrength)
					maxStrength = tmp;
			}

			if(maxStrength>0) //如果当前range亮度比设定的strength期望高，则按当前range的strength调整各通道增益
			{
				target = Gavg[i]*maxStrength/256;
				if(Gavg[i+1]<target)
					Ggain[i] = target*256/Gavg[i+1];
				else
					Ggain[i] = 256;
 
				target = Ravg[i]*maxStrength/256;
				if(Ravg[i+1]<target)
					Rgain[i] = target*256/Ravg[i+1];
				else
					Rgain[i] = 256;
				target = Bavg[i]*maxStrength/256;
				if(Bavg[i+1]<target)
					Bgain[i] = target*256/Bavg[i+1];
				else
					Bgain[i] = 256;
			}
		}
		else
			{
				Rgain[i] = Ggain[i]= Bgain[i] =256;
			}
 
	//strength = strength*0.99;
	}

	int Rgain_new[10] = { 0 };
	int Ggain_new[10] = { 0 };
	int Bgain_new[10] = { 0 };
	Rgain_new[0] = Rgain[0];
	Ggain_new[0] = Ggain[0];
	Bgain_new[0] = Bgain[0];
	for (i = 1; i < 9; i++)
	{
		Rgain_new[i] = (Rgain[i] * Rgain_new[i - 1]) / 256;
		Ggain_new[i] = (Ggain[i] * Ggain_new[i - 1]) / 256;
		Bgain_new[i] = (Bgain[i] * Bgain_new[i - 1]) / 256;


		if ((Rgain_new[i] - Rgain_new[i - 1]) > 128)
		{
			Rgain_new[i] = Rgain_new[i] - (Rgain_new[i - 1] - Rgain_new[i - 2]) / 4;
		}
		if ((Ggain_new[i] - Ggain_new[i - 1]) > 128)
		{
			Ggain_new[i] = Ggain_new[i] - (Ggain_new[i - 1] - Ggain_new[i - 2]) / 4;
		}
		if ((Bgain_new[i] - Bgain_new[i - 1]) > 128)
		{
			Bgain_new[i] = Bgain_new[i] - (Bgain_new[i - 1] - Bgain_new[i - 2]) / 4;
		}
	}
 
	lsc_para->BB_R[0] = 0;
	lsc_para->BB_G[0] = 0;
	lsc_para->BB_B[0] = 0;
	lsc_para->CC_R[0] = 256;
	lsc_para->CC_G[0] = 256;
	lsc_para->CC_B[0] = 256;
	for(i= 1; i<9; i++)
	{
		lsc_para->CC_R[i] = lsc_para->CC_R[i-1]+(((lsc_para->range[i]-lsc_para->range[i-1])*lsc_para->BB_R[i-1])>>6);
		lsc_para->CC_G[i] = lsc_para->CC_G[i-1]+(((lsc_para->range[i]-lsc_para->range[i-1])*lsc_para->BB_G[i-1])>>6);
		lsc_para->CC_B[i] = lsc_para->CC_B[i-1]+(((lsc_para->range[i]-lsc_para->range[i-1])*lsc_para->BB_B[i-1])>>6);
		if(cnt[i+1]>0)
		{
		lsc_para->BB_R[i] = ((Rgain_new[i] - Rgain_new[i - 1])*64+(lsc_para->range[i+1]-lsc_para->range[i])/2)/(lsc_para->range[i+1]-lsc_para->range[i]);
		lsc_para->BB_G[i] = ((Ggain_new[i] - Ggain_new[i - 1])*64+(lsc_para->range[i+1]-lsc_para->range[i])/2)/(lsc_para->range[i+1]-lsc_para->range[i]);
		lsc_para->BB_B[i] = ((Bgain_new[i] - Bgain_new[i - 1])*64+(lsc_para->range[i+1]-lsc_para->range[i])/2)/(lsc_para->range[i+1]-lsc_para->range[i]);
		}
	else
		{
		lsc_para->BB_R[i] = lsc_para->BB_R[i-1];
		lsc_para->BB_G[i] = lsc_para->BB_G[i-1];
		lsc_para->BB_B[i] = lsc_para->BB_B[i-1];
		}
	}
	lsc_para->CC_R[9] = lsc_para->CC_R[8]+(((lsc_para->range[9]-lsc_para->range[8])*lsc_para->BB_R[8])>>6);
	lsc_para->BB_R[9] = lsc_para->BB_R[8];
	lsc_para->CC_G[9] = lsc_para->CC_G[8]+(((lsc_para->range[9]-lsc_para->range[8])*lsc_para->BB_G[8])>>6);
	lsc_para->BB_G[9] = lsc_para->BB_G[8];
	lsc_para->CC_B[9] = lsc_para->CC_B[8]+(((lsc_para->range[9]-lsc_para->range[8])*lsc_para->BB_B[8])>>6);
	lsc_para->BB_B[9] = lsc_para->BB_B[8];
 
	//LSC
	for(y=2; y<height-2; y++)
		for(x=2; x<width-2; x++) //skip black border
		{
			range = Distance(x, centerX, y, centerY);
			range = range>>lsc_shift;
			for(i=0; i<10; i++)
			{
				if((i<9&&range>=lsc_para->range[i] && range<lsc_para->range[i+1])
				||(i>=9&&range>=lsc_para->range[i] ))
				{
				int tmp, gain;
				tmp = range-lsc_para->range[i];
				gain = lsc_para->CC_R[i]+((tmp*lsc_para->BB_R[i])>>6); 
				Rchannel[y*width+x] = Rchannel[y*width+x] *gain/256;
 
				gain = lsc_para->CC_G[i]+((tmp*lsc_para->BB_G[i])>>6); 
				Gchannel[y*width+x] = Gchannel[y*width+x] *gain/256;
 
				gain = lsc_para->CC_B[i]+((tmp*lsc_para->BB_B[i])>>6); 
				Bchannel[y*width+x] = Bchannel[y*width+x] *gain/256;
				}
			}
		}
	return 0;
}


static int auto_calc_lsc(T_U8 *yuvnv12, T_U8 *bmpbuf, T_U16 width, T_U16 height, AK_ISP_INIT_LSC *lsc_addr, int Strength)
{	
	int i = 0;
	T_U8 *R = NULL,*G = NULL,*B = NULL;
	int mid_y = 0,mid_x = 0;
	LSC_ATTR lsc_para = {0};

	R = (T_U8*)malloc(width * height);
	G = (T_U8*)malloc(width * height);
	B = (T_U8*)malloc(width * height);

	memset(R, 0, width * height);
	memset(G, 0, width * height);
	memset(B, 0, width * height);

    if((NULL == R) || (NULL == G) || (NULL == B))
	{
		printf("malloc Fail!\n");
		return -1;
	}	
	
	Most_Bright_Position(yuvnv12, &mid_y, &mid_x, width, height);
	BMP_Split(bmpbuf, R, G, B, height);

	lsc_para.xref = mid_x;
	lsc_para.yref = mid_y;
	calc_shift_range(&lsc_para, yuvnv12, width, height);
	LSC_RGB(&lsc_para, R, G, B, width, height, Strength);

	free(R);
	free(G);
	free(B);

	lsc_addr->lsc.xref = mid_x;
	lsc_addr->lsc.yref = mid_y;
	lsc_addr->lsc.lsc_shift = lsc_para.lsc_shift;

	for(i=0; i<10; i++)
	{
		lsc_addr->lsc.range[i] = lsc_para.range[i];

		lsc_addr->lsc.lsc_r_coef.coef_b[i] = lsc_para.BB_R[i];
		lsc_addr->lsc.lsc_r_coef.coef_c[i] = lsc_para.CC_R[i];

		lsc_addr->lsc.lsc_gr_coef.coef_b[i] = lsc_para.BB_G[i];
		lsc_addr->lsc.lsc_gr_coef.coef_c[i] = lsc_para.CC_G[i];

		lsc_addr->lsc.lsc_gb_coef.coef_b[i] = lsc_para.BB_G[i];
		lsc_addr->lsc.lsc_gb_coef.coef_c[i] = lsc_para.CC_G[i];

		lsc_addr->lsc.lsc_b_coef.coef_b[i] = lsc_para.BB_B[i];
		lsc_addr->lsc.lsc_b_coef.coef_c[i] = lsc_para.CC_B[i];
	}

	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CLscBrightnessCalcDlg dialog


CLscBrightnessCalcDlg::CLscBrightnessCalcDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLscBrightnessCalcDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLscBrightnessCalcDlg)
	m_bRefreshFlag = FALSE;
	m_bInit = FALSE;
	m_Strength = 90;
	m_yuvbuf = NULL;
	m_bmpbuf = NULL;
	m_img_width = YUV_WIDTH_960P;
	m_img_height = YUV_HEIGHT_960P;
	m_img_mutil = IMG_SHOW_MUTIL_1080P;
	//}}AFX_DATA_INIT

	ZeroMemory(&m_imgpath, 260);
	ZeroMemory(&m_R, sizeof(m_R));
	ZeroMemory(&m_G, sizeof(m_G));
	ZeroMemory(&m_B, sizeof(m_B));
}


void CLscBrightnessCalcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLscBrightnessCalcDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLscBrightnessCalcDlg, CDialog)
	//{{AFX_MSG_MAP(CLscBrightnessCalcDlg)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonGetYuvImg)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_YUV_IMG, OnButtonOpenYuvImg)
	ON_BN_CLICKED(IDC_BUTTON_CALC, OnButtonCalc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLscBrightnessCalcDlg message handlers

BOOL CLscBrightnessCalcDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_yuvbuf = NULL;
	m_bmpbuf = NULL;
	m_img_width = YUV_WIDTH_960P;
	m_img_height = YUV_HEIGHT_960P;
	m_img_mutil = IMG_SHOW_MUTIL_1080P;


	CDC* pDC = GetDC();
	m_MemDC.CreateCompatibleDC(pDC);
	m_MemBitmap.CreateCompatibleBitmap(pDC, 500, 500);
	m_pOldMemBitmap = m_MemDC.SelectObject(&m_MemBitmap);
	ReleaseDC(pDC);

	CRect rect;

	GetClientRect(&rect);

	m_Rect[0][0].left = rect.left + 400;
	m_Rect[0][0].right = m_Rect[0][0].left + BRIGHTNESS_IMAGE_WIDTH;
	m_Rect[0][0].top = rect.top + 60;
	m_Rect[0][0].bottom = m_Rect[0][0].top + BRIGHTNESS_IMAGE_HEIGHT;

	m_Rect[0][1].left = m_Rect[0][0].right + 100;
	m_Rect[0][1].right = m_Rect[0][1].left + BRIGHTNESS_IMAGE_WIDTH;
	m_Rect[0][1].top = rect.top + 60;
	m_Rect[0][1].bottom = m_Rect[0][1].top + BRIGHTNESS_IMAGE_HEIGHT;

	m_Rect[1][0].left = rect.left + 400;
	m_Rect[1][0].right = m_Rect[1][0].left + BRIGHTNESS_IMAGE_WIDTH;
	m_Rect[1][0].top = m_Rect[0][0].bottom + 80;
	m_Rect[1][0].bottom = m_Rect[1][0].top + BRIGHTNESS_IMAGE_HEIGHT;

	m_Rect[1][1].left = m_Rect[1][0].right + 100;
	m_Rect[1][1].right = m_Rect[1][1].left + BRIGHTNESS_IMAGE_WIDTH;
	m_Rect[1][1].top = m_Rect[0][1].bottom + 80;
	m_Rect[1][1].bottom = m_Rect[1][1].top + BRIGHTNESS_IMAGE_HEIGHT;

	
	m_bInit = TRUE;

	ZeroMemory(&m_R, sizeof(m_R));
	ZeroMemory(&m_G, sizeof(m_G));
	ZeroMemory(&m_B, sizeof(m_B));

	ZeroMemory(&m_Points_R, sizeof(m_Points_R));
	ZeroMemory(&m_Points_G, sizeof(m_Points_G));
	ZeroMemory(&m_Points_B, sizeof(m_Points_B));

	Invalidate(); 	

	return TRUE;
}


void CLscBrightnessCalcDlg::DrawBrightnessLine(CDC *pDC)
{
	int i = 0;
	int j = 0;
	
	//draw frame
	for (i=0; i<2; i++)
	{
		for (j=0; j<2; j++)
		{
			pDC->Draw3dRect(m_Rect[i][j].left - 1, m_Rect[i][j].top - 1, 
				m_Rect[i][j].Width() + 2, m_Rect[i][j].Height() + 2, RGB(0, 0, 0), RGB(0, 0, 0));
		}
	}

	int oldBkMode = m_MemDC.SetBkMode(TRANSPARENT);
	Graphics graphics(m_MemDC);
	Pen RPen(Color(255, 64, 64));
	Pen GPen(Color(64, 255, 64));
	Pen BPen(Color(64, 64, 255));

	for (i=0; i<2; i++)
	{
		for (j=0; j<2; j++)
		{
			m_MemDC.FillSolidRect(0, 0, m_Rect[i][j].Width(), m_Rect[i][j].Height(), GetSysColor(COLOR_3DFACE));

			//draw R base line
			graphics.DrawLine(&RPen, Point(0, m_Rect[i][j].Height() - m_R[i][j][0]), 
								Point(m_Rect[i][j].Width(), m_Rect[i][j].Height() - m_R[i][j][0]));

			//draw R line
			graphics.DrawLines(&RPen, m_Points_R[i][j], BRIGHTNESS_POINT_NUM+1);

			//draw G base line
			graphics.DrawLine(&GPen, Point(0, m_Rect[i][j].Height() - m_G[i][j][0]), 
								Point(m_Rect[i][j].Width(), m_Rect[i][j].Height() - m_G[i][j][0]));

			//draw G line
			graphics.DrawLines(&GPen, m_Points_G[i][j], BRIGHTNESS_POINT_NUM+1);

			//draw B base line
			graphics.DrawLine(&BPen, Point(0, m_Rect[i][j].Height() - m_B[i][j][0]), 
								Point(m_Rect[i][j].Width(), m_Rect[i][j].Height() - m_B[i][j][0]));

			//draw B line
			graphics.DrawLines(&BPen, m_Points_B[i][j], BRIGHTNESS_POINT_NUM+1);
			
			pDC->BitBlt(m_Rect[i][j].left, m_Rect[i][j].top, m_Rect[i][j].Width(), m_Rect[i][j].Height(), &m_MemDC, 0, 0, SRCCOPY);
		}
	}
}

T_U8* CLscBrightnessCalcDlg::LoadYuvData(char* path) 
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
		m_img_width = YUV_WIDTH_720P;
		m_img_height = YUV_HEIGHT_720P;
		m_img_mutil = IMG_SHOW_MUTIL;

		m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_720P+2);
	}
	else if (YUV_WIDTH_960P * YUV_HEIGHT_960P * 3 / 2 == size)
	{
		m_img_width = YUV_WIDTH_960P;
		m_img_height = YUV_HEIGHT_960P;
		m_img_mutil = IMG_SHOW_MUTIL;

		m_ImgRect.SetRect(rect.left+2, rect.top+2, IMG_SHOW_WIDTH+2, IMG_SHOW_HEIGHT_960P+2);
	}
	else if (YUV_WIDTH_1080P * YUV_HEIGHT_1080P * 3 / 2 == size)
	{
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


bool CLscBrightnessCalcDlg::GetBMP(void) 
{
	if (NULL != m_yuvbuf)
	{
		free(m_yuvbuf);
		m_yuvbuf = NULL;
	}
		
	m_yuvbuf = LoadYuvData(m_imgpath);

	if (NULL == m_yuvbuf)
	{
		return FALSE;
	}

	if (NULL != m_bmpbuf)
	{
		free(m_bmpbuf);
		m_bmpbuf = NULL;
	}

	m_bmpbuf = YUV420ToBMP(m_yuvbuf, m_img_width, m_img_height);

	if (NULL == m_bmpbuf)
	{
		return FALSE;
	}

	return TRUE;
}

void CLscBrightnessCalcDlg::BMShow(CDC *pDC, int x, int y, int width, int height) 
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


void CLscBrightnessCalcDlg::Calc_Brightness() 
{
	if (NULL == m_bmpbuf)
		return;

	int i = 0;
	int j = 0;
	float unit_x = 0;
	float unit_y = 0;
	T_U16 x[BRIGHTNESS_POINT_NUM + 1] = {0};
	T_U16 y[BRIGHTNESS_POINT_NUM + 1] = {0};
	T_U8* p = m_bmpbuf + BMP_HEADINFO_LEN;
	T_U8 r_base = 0;
	T_U8 g_base = 0;
	T_U8 b_base = 0;
	UINT offset = 0;
	CWnd *pWnd = NULL;
	CString str;
	
	x[0] = m_Lsc.lsc.xref;
	y[0] = m_Lsc.lsc.yref;

	pWnd = GetDlgItem(IDC_STATIC_POINT_0);
	str.Format("( %d , %d )", x[0], y[0]);
	pWnd->SetWindowText(str);
	
	pWnd = GetDlgItem(IDC_STATIC_POINT_1);
	str.Format("( %d , %d )", x[0], y[0]);
	pWnd->SetWindowText(str);
	
	pWnd = GetDlgItem(IDC_STATIC_POINT_2);
	str.Format("( %d , %d )", x[0], y[0]);
	pWnd->SetWindowText(str);
	
	pWnd = GetDlgItem(IDC_STATIC_POINT_3);
	str.Format("( %d , %d )", x[0], y[0]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_POINT_XMAX1);
	str.Format("( %d , 0 )", m_img_width);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_POINT_XMAX2);
	str.Format("( 0 , %d )", m_img_height);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_POINT_XMAX3);
	str.Format("( %d , %d )", m_img_width, m_img_height);
	pWnd->SetWindowText(str);

	offset = (m_img_height-1-y[0]) * m_img_width * 3 + x[0] * 3;
	
	b_base = p[offset];
	g_base = p[offset + 1];
	r_base = p[offset + 2];

	for (i=0; i<2; i++)
	{
		for (j=0; j<2; j++)
		{
			m_R[i][j][0] = r_base;
			m_Points_R[i][j][0].X = 0;
			m_Points_R[i][j][0].Y = BRIGHTNESS_IMAGE_HEIGHT - r_base;

			m_G[i][j][0] = g_base;
			m_Points_G[i][j][0].X = 0;
			m_Points_G[i][j][0].Y = BRIGHTNESS_IMAGE_HEIGHT - g_base;

			m_B[i][j][0] = b_base;
			m_Points_B[i][j][0].X = 0;
			m_Points_B[i][j][0].Y = BRIGHTNESS_IMAGE_HEIGHT - b_base;
		}
	}
	

	//rect[0][0]
	unit_x = (float)m_Lsc.lsc.xref / (float)BRIGHTNESS_POINT_NUM;
	unit_y = (float)m_Lsc.lsc.yref / (float)BRIGHTNESS_POINT_NUM;

	for (i=1; i<=BRIGHTNESS_POINT_NUM; i++)
	{
		x[i] = x[0] - i * unit_x;
		
		if (x[i] < 0)
			x[i] = 0;

		y[i] = y[0] - i * unit_y;

		if (y[i] < 0)
			y[i] = 0;

		offset = (m_img_height-1-y[i]) * m_img_width * 3 + x[i] * 3;

		if ((x[i]-1 >= 0) && (y[i]-1 >= 0))
		{
			m_B[0][0][i] = (p[offset] 
							+ p[offset + 3]
							+ p[offset - 3]
							+ p[offset + m_img_width * 3]
							+ p[offset - m_img_width * 3]) / 5;

			m_G[0][0][i] = (p[offset + 1] 
							+ p[offset  + 1 + 3]
							+ p[offset  + 1 - 3]
							+ p[offset  + 1 + m_img_width * 3]
							+ p[offset  + 1 - m_img_width * 3]) / 5;

			m_R[0][0][i] = (p[offset + 2] 
							+ p[offset + 2 + 3]
							+ p[offset + 2 - 3]
							+ p[offset + 2 + m_img_width * 3]
							+ p[offset + 2 - m_img_width * 3]) / 5;
		}
		else
		{
			m_B[0][0][i] = p[offset];
			m_G[0][0][i] = p[offset + 1];
			m_R[0][0][i] = p[offset + 2];
		}

		m_Points_R[0][0][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_R[0][0][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_R[0][0][i];

		m_Points_G[0][0][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_G[0][0][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_G[0][0][i];

		m_Points_B[0][0][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_B[0][0][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_B[0][0][i];
	}

	//rect[0][1]
	unit_x = (float)(m_img_width - m_Lsc.lsc.xref) / (float)BRIGHTNESS_POINT_NUM;
	unit_y = (float)m_Lsc.lsc.yref / (float)BRIGHTNESS_POINT_NUM;

	for (i=1; i<=BRIGHTNESS_POINT_NUM; i++)
	{
		x[i] = x[0] + i * unit_x;
		
		if (x[i] > m_img_width - 1)
			x[i] = m_img_width - 1;

		y[i] = y[0] - i * unit_y;

		if (y[i] < 0)
			y[i] = 0;

		offset = (m_img_height-1-y[i]) * m_img_width * 3 + x[i] * 3;

		if ((x[i]+1 <= m_img_width - 1) && (y[i]-1 >= 0))
		{
			m_B[0][1][i] = (p[offset] 
							+ p[offset + 3]
							+ p[offset - 3]
							+ p[offset + m_img_width * 3]
							+ p[offset - m_img_width * 3]) / 5;

			m_G[0][1][i] = (p[offset + 1] 
							+ p[offset  + 1 + 3]
							+ p[offset  + 1 - 3]
							+ p[offset  + 1 + m_img_width * 3]
							+ p[offset  + 1 - m_img_width * 3]) / 5;

			m_R[0][1][i] = (p[offset + 2] 
							+ p[offset + 2 + 3]
							+ p[offset + 2 - 3]
							+ p[offset + 2 + m_img_width * 3]
							+ p[offset + 2 - m_img_width * 3]) / 5;
		}
		else
		{
			m_B[0][1][i] = p[offset];
			m_G[0][1][i] = p[offset + 1];
			m_R[0][1][i] = p[offset + 2];
		}

		m_Points_R[0][1][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_R[0][1][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_R[0][1][i];

		m_Points_G[0][1][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_G[0][1][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_G[0][1][i];

		m_Points_B[0][1][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_B[0][1][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_B[0][1][i];
	}


	//rect[1][0]
	unit_x = (float)m_Lsc.lsc.xref / (float)BRIGHTNESS_POINT_NUM;
	unit_y = (float)(m_img_height- m_Lsc.lsc.yref) / (float)BRIGHTNESS_POINT_NUM;

	for (i=1; i<=BRIGHTNESS_POINT_NUM; i++)
	{
		x[i] = x[0] - i * unit_x;
		
		if (x[i] < 0)
			x[i] = 0;

		y[i] = y[0] + i * unit_y;

		if (y[i] > m_img_height - 1)
			y[i] = m_img_height - 1;

		offset = (m_img_height-1-y[i]) * m_img_width * 3 + x[i] * 3;

		if ((x[i]-1 >= 0) && (y[i]+1 <= m_img_height - 1))
		{
			m_B[1][0][i] = (p[offset] 
							+ p[offset + 3]
							+ p[offset - 3]
							+ p[offset + m_img_width * 3]
							+ p[offset - m_img_width * 3]) / 5;

			m_G[1][0][i] = (p[offset + 1] 
							+ p[offset  + 1 + 3]
							+ p[offset  + 1 - 3]
							+ p[offset  + 1 + m_img_width * 3]
							+ p[offset  + 1 - m_img_width * 3]) / 5;

			m_R[1][0][i] = (p[offset + 2] 
							+ p[offset + 2 + 3]
							+ p[offset + 2 - 3]
							+ p[offset + 2 + m_img_width * 3]
							+ p[offset + 2 - m_img_width * 3]) / 5;
		}
		else
		{
			m_B[1][0][i] = p[offset];
			m_G[1][0][i] = p[offset + 1];
			m_R[1][0][i] = p[offset + 2];
		}

		m_Points_R[1][0][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_R[1][0][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_R[1][0][i];

		m_Points_G[1][0][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_G[1][0][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_G[1][0][i];

		m_Points_B[1][0][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_B[1][0][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_B[1][0][i];
	}


	//rect[1][1]
	unit_x = (float)(m_img_width - m_Lsc.lsc.xref) / (float)BRIGHTNESS_POINT_NUM;
	unit_y = (float)(m_img_height- m_Lsc.lsc.yref) / (float)BRIGHTNESS_POINT_NUM;

	for (i=1; i<=BRIGHTNESS_POINT_NUM; i++)
	{
		x[i] = x[0] + i * unit_x;
		
		if (x[i] > m_img_width - 1)
			x[i] = m_img_width - 1;

		y[i] = y[0] + i * unit_y;

		if (y[i] > m_img_height - 1)
			y[i] = m_img_height - 1;

		offset = (m_img_height-1-y[i]) * m_img_width * 3 + x[i] * 3;

		if ((x[i]+1 <= m_img_width - 1) && (y[i]+1 <= m_img_height - 1))
		{
			m_B[1][1][i] = (p[offset] 
							+ p[offset + 3]
							+ p[offset - 3]
							+ p[offset + m_img_width * 3]
							+ p[offset - m_img_width * 3]) / 5;

			m_G[1][1][i] = (p[offset + 1] 
							+ p[offset  + 1 + 3]
							+ p[offset  + 1 - 3]
							+ p[offset  + 1 + m_img_width * 3]
							+ p[offset  + 1 - m_img_width * 3]) / 5;

			m_R[1][1][i] = (p[offset + 2] 
							+ p[offset + 2 + 3]
							+ p[offset + 2 - 3]
							+ p[offset + 2 + m_img_width * 3]
							+ p[offset + 2 - m_img_width * 3]) / 5;
		}
		else
		{
			m_B[1][1][i] = p[offset];
			m_G[1][1][i] = p[offset + 1];
			m_R[1][1][i] = p[offset + 2];
		}

		m_Points_R[1][1][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_R[1][1][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_R[1][1][i];

		m_Points_G[1][1][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_G[1][1][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_G[1][1][i];

		m_Points_B[1][1][i].X = i * (BRIGHTNESS_IMAGE_WIDTH / BRIGHTNESS_POINT_NUM);
		m_Points_B[1][1][i].Y = BRIGHTNESS_IMAGE_HEIGHT - m_B[1][1][i];
	}

}


void CLscBrightnessCalcDlg::OnPaint()
{
	CPaintDC dc(this);
	
	if (m_bRefreshFlag)
	{
		GetBMP();
		Calc_Brightness();
		
		m_bRefreshFlag = FALSE;
	}

	
	BMShow(&dc, m_ImgRect.left, m_ImgRect.top, m_img_width/m_img_mutil, m_img_height/m_img_mutil);

	DrawBrightnessLine(&dc);
}


void CLscBrightnessCalcDlg::OnClose() 
{
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

	if (m_MemDC.m_hDC) {
		m_MemDC.SelectObject(m_pOldMemBitmap);
		m_MemDC.DeleteDC();
		m_MemBitmap.DeleteObject();
	}

	m_GrayBarBitmap.DeleteObject();
	
	CDialog::OnClose();
	DestroyWindow();
}

void CLscBrightnessCalcDlg::SetImgPath(char* path) 
{
	strcpy(m_imgpath, path);

	if (m_bInit)
	{
		Invalidate();
	}

	m_bRefreshFlag = TRUE;
}

void CLscBrightnessCalcDlg::Img_SetConnectState(bool bConnect) 
{
	// TODO: Add your control notification handler code here
	
	m_img_bConnect = bConnect;
}



void CLscBrightnessCalcDlg::OnButtonGetYuvImg() 
{
	// TODO: Add your control notification handler code here

	if (!m_img_bConnect) 
	{
		AfxMessageBox("没有与任何设备进行连接，或连接已经断开，无法获取图像计算亮度!\n", 0, 0);
		return;
	}
		
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_LSC_INFO, DIALOG_LSC, 0);
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_YUV_IMG, DIALOG_LSC, 0);
}

void CLscBrightnessCalcDlg::OnButtonOpenYuvImg() 
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, "*.yuv", NULL, OFN_HIDEREADONLY,
		"Data File(*.yuv)|*.yuv|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_LSC_INFO, DIALOG_LSC, 0);
	SetImgPath((LPSTR)(LPCTSTR)dlg.GetPathName());
}

void CLscBrightnessCalcDlg::OnButtonCalc() 
{
	// TODO: Add your control notification handler code here
	auto_calc_lsc(m_yuvbuf, m_bmpbuf, m_img_width, m_img_height, &m_Lsc, m_Strength);

	CBasePage::SendPageMessage(m_pMessageWnd, WM_SET_LSC_INFO, DIALOG_LSC, 0);

	AfxMessageBox("计算完成!\n", 0, 0);
}
