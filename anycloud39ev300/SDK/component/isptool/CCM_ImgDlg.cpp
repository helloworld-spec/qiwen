// CCM_ImgDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ispctrl_tool.h"
#include "CCM_ImgDlg.h"
#include "ImgDlg.h"
#include "NetCtrl.h"

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

#define IMG_SHOW_LEFT	10
#define IMG_SHOW_TOP	10

#define IMG_SHOW_WIDTH	640
#define IMG_SHOW_HEIGHT	480

#define IMG_SHOW_MUTIL	2
#define IMG_SHOW_MUTIL_1080P	3
#define IMG_SHOW_MUTIL_1536P	3


#define BMP_HEADINFO_LEN	54

#define CLIP255(x) ((x)>0?((x)<255?(x):255):0)

#define YUV_NV12


extern AK_ISP_INIT_PARAM   Isp_init_param;


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


//根据Gamma曲线计算插值
void CalcLUT(const WORD gamma[],Point_temp MyPoint[])
{
	int i,j,k;
	Point_temp PointTmp;
	
	for(i = 0;i < 129;i++)
	{
		MyPoint[i].x = i;
		MyPoint[i].y = gamma[i];
	}

	for(i = 0;i <256;i++)
	{
		if(MyPoint[i].x*2 == 256)
		{
			MyPoint[i].x = 255;
			MyPoint[i].y = 255;
		}
		else
		{
			MyPoint[i].y = MyPoint[i].y/4;
			MyPoint[i].x = MyPoint[i].x*2;	
		}
	}

	j = 129;
	for(i = 0;i < 129;i++)
	{
		for(k = MyPoint[i].x+1;k< MyPoint[i+1].x;k++)
		{
			MyPoint[j].x = k;
			MyPoint[j].y =((MyPoint[i+1].y - MyPoint[i].y)*(k-MyPoint[i].x)/(MyPoint[i+1].x - MyPoint[i].x))+MyPoint[i].y;
			j++;
		}
	}


	for(i = 0;i < 255;i++)//注意冒泡排序的范围。上限值为数组总长度减一，如果上限值为数组长度eg i < 256,数组可能越界访问
	{
		 for(j = 0;j < 255-i;j++)
		 {
			  if(MyPoint[j].x > MyPoint[j+1].x)
			  {
				  PointTmp = MyPoint[j];
				  MyPoint[j] = MyPoint[j+1];
				  MyPoint[j+1] = PointTmp;
			  }
		  }
	}
}

//经由新的CCM和Gamma曲线参数作用原始图像，并输出新图像
BOOL OutImage(BYTE *Out_Image, const int CCMMat[], BYTE const *InputImage,const AK_ISP_RGB_GAMMA_ATTR gamma)
{

	BYTE *src;
	WORD BMPfileType = 0;
	int Width,Height,biBitCount;
	int lineByte=0;
	int index=0;
	int i,j,k;
	int C11,C12,C13,C21,C22,C23,C31,C32,C33;
	Point_temp MyPoint1[256] = {{0,0}};
	Point_temp MyPoint2[256] = {{0,0}};
	Point_temp PointTmp = {0,0};
	BITMAPFILEHEADER bf = {0};
	BITMAPINFOHEADER bi = {0};
	BMPRGB *DataOfBmp;
	WORD Gamma_line[129] = {0,8,16,24,32,40,48,56,64,72,
		                   80,88,96,104,112,120,128,136,
						   144,152,160,168,176,184,192,
						   200,208,216,224,232,240,248,
						   256,264,272,280,288,296,304,
						   312,320,328,336,344,352,360,
						   368,376,384,392,400,408,416,
	                       424,432,440,448,456,464,472,
						   480,488,496,504,512,520,528,
						   536,544,552,560,568,576,584,
						   592,600,608,616,624,632,640,
	                       648,656,664,672,680,688,696,
						   704,712,720,728,736,744,752,
						   760,768,776,784,792,800,808,
						   816,824,832,840,848,856,864,
						   872,880,888,896,904,912,920,
						   928,936,944,952,960,968,976,
						   984,992,1000,1008,1016,1024};

	C11 = CCMMat[0];
	C12 = CCMMat[1];
	C13 = CCMMat[2];
	C21 = CCMMat[3];
	C22 = CCMMat[4];
	C23 = CCMMat[5];
	C31 = CCMMat[6];
	C32 = CCMMat[7];
	C33 = CCMMat[8];

	
	memcpy(&bi,InputImage+sizeof(BITMAPFILEHEADER),sizeof(BITMAPINFOHEADER));

	Width = bi.biWidth;
	Height = bi.biHeight;
	biBitCount = bi.biBitCount;
	lineByte = WIDTHBYTES(Width*biBitCount);
    
	//Out_Image = (BYTE*)malloc(Height*lineByte+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(WORD));
	memset(Out_Image,0,Height*lineByte+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER));
	memcpy(Out_Image,InputImage,Height*lineByte+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER));

	src = (BYTE*)malloc(Height*lineByte);
	if(NULL == src)
	{
		printf("Can't malloc bmp src!\n");
		return 0;
	}

	//获取BMP图像数据信息
	memset(src,0,Height*lineByte);
	memcpy(src,Out_Image+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER),Height*lineByte);

	DataOfBmp = (BMPRGB *)malloc(Width*Height*sizeof(BMPRGB));
	memset(DataOfBmp,0,Width*Height*sizeof(BMPRGB));

	if(gamma.rgb_gamma_enable == 1)
	{
		CalcLUT(gamma.r_gamma,MyPoint1);//Gamma 使能所用曲线
		 for(i = 0;i <Height;i++)
		 {
			for(j=0;j < Width;j++)
			{
				k =i*lineByte+j*3;
				
				//DataOfBmp[index].rgbReserved = src[k+3];
				DataOfBmp[index].rgbRed = src[k+2];
				DataOfBmp[index].rgbGreen = src[k+1];
				DataOfBmp[index].rgbBlue = src[k];

				//颜色校正
				//DataOfBmp[index].rgbReserved = src[k+3];
				DataOfBmp[index].rgbRed   = CLAMP8((src[k+2]*C11+src[k+1]*C12+src[k]*C13)>>8);
				DataOfBmp[index].rgbGreen = CLAMP8((src[k+2]*C21+src[k+1]*C22+src[k]*C23)>>8);
				DataOfBmp[index].rgbBlue  = CLAMP8((src[k+2]*C31+src[k+1]*C32+src[k]*C33)>>8);

				
				//Gamma校正
				//src[k+3] = DataOfBmp[index].rgbReserved;
				src[k+2] = MyPoint1[DataOfBmp[index].rgbRed].y;
				src[k+1] = MyPoint1[DataOfBmp[index].rgbGreen].y;
				src[k]   = MyPoint1[DataOfBmp[index].rgbBlue].y;


				index++;
			}
		 }
	}
	else
	{
		CalcLUT(Gamma_line,MyPoint2);//Gamma 不使能所用曲线
		for(i = 0;i <Height;i++)
		{
			for(j=0;j < Width;j++)
			{
				if(i==Height-112 && j==288)
					k =i*lineByte+j*3;
				k =i*lineByte+j*3;
				
			//	DataOfBmp[index].rgbReserved = src[k+3];
				DataOfBmp[index].rgbRed = src[k+2];
				DataOfBmp[index].rgbGreen = src[k+1];
				DataOfBmp[index].rgbBlue = src[k];

				//颜色校正
			//	DataOfBmp[index].rgbReserved = src[k+3];
				DataOfBmp[index].rgbRed   = CLAMP8((src[k+2]*C11+src[k+1]*C12+src[k]*C13)>>8);
				DataOfBmp[index].rgbGreen = CLAMP8((src[k+2]*C21+src[k+1]*C22+src[k]*C23)>>8);
				DataOfBmp[index].rgbBlue  = CLAMP8((src[k+2]*C31+src[k+1]*C32+src[k]*C33)>>8);

				
				//Gamma校正
			//	src[k+3] = DataOfBmp[index].rgbReserved;
				src[k+2] = MyPoint2[DataOfBmp[index].rgbRed].y;
				src[k+1] = MyPoint2[DataOfBmp[index].rgbGreen].y;
				src[k]   = MyPoint2[DataOfBmp[index].rgbBlue].y;

				index++;
			}
		}
	}

	memcpy(Out_Image+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER),src,Height*lineByte);
	
	free(src);
	free(DataOfBmp);
	
	return TRUE;
}


//经由新的CCM和相关Gamma曲线数据计算新的RGB
BOOL CalcNewRGB(RGB_temp Out_RGB[], const int CCMMat[],const RGB_temp original[],const AK_ISP_RGB_GAMMA_ATTR gamma)
{
	
    int i = 0;
	int k = 0;
	int j = 0;
	int C11,C12,C13,C21,C22,C23,C31,C32,C33;
	Point_temp MyPoint1[256] = {{0,0}};
	Point_temp MyPoint2[256] = {{0,0}};
	Point_temp PointTmp = {0,0};
	WORD Gamma_line[129] = {0,8,16,24,32,40,48,56,64,72,
		                   80,88,96,104,112,120,128,136,
						   144,152,160,168,176,184,192,
						   200,208,216,224,232,240,248,
						   256,264,272,280,288,296,304,
						   312,320,328,336,344,352,360,
						   368,376,384,392,400,408,416,
	                       424,432,440,448,456,464,472,
						   480,488,496,504,512,520,528,
						   536,544,552,560,568,576,584,
						   592,600,608,616,624,632,640,
	                       648,656,664,672,680,688,696,
						   704,712,720,728,736,744,752,
						   760,768,776,784,792,800,808,
						   816,824,832,840,848,856,864,
						   872,880,888,896,904,912,920,
						   928,936,944,952,960,968,976,
						   984,992,1000,1008,1016,1024};


	C11 = CCMMat[0];
	C12 = CCMMat[1];
	C13 = CCMMat[2];
	C21 = CCMMat[3];
	C22 = CCMMat[4];
	C23 = CCMMat[5];
	C31 = CCMMat[6];
	C32 = CCMMat[7];
	C33 = CCMMat[8];



	if(gamma.rgb_gamma_enable == 1)
	{
		CalcLUT(gamma.r_gamma,MyPoint1);//Gamma 使能所用曲线
		for(i = 0;i <24;i++)
		{
			Out_RGB[i].R =  CLAMP8((original[i].R*C11+original[i].G*C12+original[i].B*C13)>>8);
			Out_RGB[i].G =  CLAMP8((original[i].R*C21+original[i].G*C22+original[i].B*C23)>>8);
			Out_RGB[i].B =  CLAMP8((original[i].R*C31+original[i].G*C32+original[i].B*C33)>>8);

			Out_RGB[i].R =  CLAMP8(Out_RGB[i].R);
			Out_RGB[i].G =  CLAMP8(Out_RGB[i].G);
			Out_RGB[i].B =  CLAMP8(Out_RGB[i].B);

			Out_RGB[i].R = MyPoint1[Out_RGB[i].R].y;
			Out_RGB[i].G = MyPoint1[Out_RGB[i].G].y;
			Out_RGB[i].B = MyPoint1[Out_RGB[i].B].y;
		}

	}
	else
	{
		CalcLUT(Gamma_line,MyPoint2);//Gamma 不使能所用曲线
		for(i = 0;i <24;i++)
		{
			Out_RGB[i].R =  CLAMP8((original[i].R*C11+original[i].G*C12+original[i].B*C13)>>8);
			Out_RGB[i].G =  CLAMP8((original[i].R*C21+original[i].G*C22+original[i].B*C23)>>8);
			Out_RGB[i].B =  CLAMP8((original[i].R*C31+original[i].G*C32+original[i].B*C33)>>8);

			Out_RGB[i].R =  CLAMP8(Out_RGB[i].R);
			Out_RGB[i].G =  CLAMP8(Out_RGB[i].G);
			Out_RGB[i].B =  CLAMP8(Out_RGB[i].B);

			Out_RGB[i].R = MyPoint2[Out_RGB[i].R].y;
			Out_RGB[i].G = MyPoint2[Out_RGB[i].G].y;
			Out_RGB[i].B = MyPoint2[Out_RGB[i].B].y;
		}

	}

	return TRUE;
}

//饱和度调整
int CalcSaturation(int CCM_Mat[],const int Saturation)
{
	 /*
	 模拟海思图像标定工具中CCM计算方式。
     主对角线上的C11每隔两个步进数值大小增减1，次对角线上的C12,C13在C11每次变化时轮流加减1；
     主对角线上的C22每隔三个步进数值大小增减1，次对角线上的C21,C23在C22每次变化时轮流加减1；
     次对角线上的C31每隔七个步进数值大小增减1，次对角线上的C32,主对角线上的C33每隔一个步进数值大小增减1；
	 */

	int Satu = 128;
	int step = 0;
	int flagevenCCM_Row_One = 1;  //颜色校正矩阵第一行的标志位，当步长为偶数时使用
	int flagoddCCM_Row_One  = 1;  //颜色校正矩阵第一行的标志位，当步长为奇数时使用
	int flagevenCCM_Row_Two = 1;  //颜色校正矩阵第二行的标志位，当步长为3的倍数时使用
	int flagoddCCM_Row_Two  = 1;  //颜色校正矩阵第二行的标志位，当步长非3的倍数时使用
    int C11,C12,C13,C21,C22,C23,C31,C32,C33;
	C11 = CCM_Mat[0];
	C12 = CCM_Mat[1];
	C13 = CCM_Mat[2];
	C21 = CCM_Mat[3];
	C22 = CCM_Mat[4];
	C23 = CCM_Mat[5];
	C31 = CCM_Mat[6];
	C32 = CCM_Mat[7];
	C33 = CCM_Mat[8];
		
	printf("Input Saturation is: %d\n",Saturation);	
	step = Saturation-Satu;
		

		//当步长为偶数时
		if(step % 2 == 0) 
		{

			 flagevenCCM_Row_One = -flagevenCCM_Row_One;//颜色校正矩阵第一行的标志位，根据此标志位C12 C13轮流进行增减1
			 C11 =  CCM_Mat[0];
			 C11 = C11+step/2;
			 if(C11 < Min_Main_Diagonal)
				 C11 = Min_Main_Diagonal;
			 if(C11 > Max_Diagonal)
				 C11 = Max_Diagonal;
	
			 if(flagevenCCM_Row_One == -1)
			 {
				 C12 = CCM_Mat[1];
				 if(step >0)
					 C12 = C12-((step/2+1)/2);
				 else
					 C12 = C12+((step/2+1)/2);
				
				 if(C12 > 1160)
					 C12 = 1160;
				 C13 = 256-C11-C12;
			 }
			 else
			 {
				 C13 = CCM_Mat[2];
				 if(step >0)
					 C13 = C13-((step/2+1)/2);
				 else
					 C13 = C13+((step/2+1)/2);
			
				 if(C13 > 1160)
					 C13 = 1160;
				 C12 = 256-C11-C13;			
			 }



		if(step % 3 ==0 )
		{
			flagevenCCM_Row_Two = -flagevenCCM_Row_Two; //颜色校正矩阵第二行的标志位，根据此标志位C21 C23轮流进行增减1
			 C22 = CCM_Mat[4];
			 C22 = C22+step/3;
			 if(C22 < Min_Main_Diagonal)
				 C22 = Min_Main_Diagonal;
			 if(C22 > Max_Diagonal)
				 C22 = Max_Diagonal;

			if(flagevenCCM_Row_Two == -1)
			 {
				C21 = CCM_Mat[3];
				 if(step >0)
					 C21 = C21-((step/3+1)/2);
				 else
					 C21 = C21+((step/3+1)/2);
				
				 if(C21 > Max_Diagonal)
					 C21 = Max_Diagonal;
				 C23 = 256-C22-C21;
			 }
			 else
			 {
				  C23 = CCM_Mat[5];
				 if(step >0)
					 C23 = C23-((step/3+1)/2);
				 else
					 C23 = C23+((step/3+1)/2);
			
				 if(C23 > Max_Diagonal)
					 C23 = Max_Diagonal;
				 C21 = 256-C23-C22;
			 }
		}
		else
			{

			flagoddCCM_Row_Two = -flagoddCCM_Row_Two;
			 C22 = CCM_Mat[4];
			 C22 = C22+step/3;
			 if(C22 < Min_Main_Diagonal)
				 C22 = Min_Main_Diagonal;
			 if(C22 > Max_Diagonal)
				 C22 = Max_Diagonal;
			
			 
			if(flagoddCCM_Row_Two == -1)
			 {
				C21 = CCM_Mat[3];
				 if(step >0)
					 C21 = C21-(((step)/3+1)/2);
				 else
					 C21 = C21+(((step)/3+1)/2);
				
				 if(C21 > Max_Diagonal)
					 C21 = Max_Diagonal;
				 C23 = 256-C22-C21;
			 }
			 else
			 {
				  C23 = CCM_Mat[5];
				 if(step >0)
					 C23 = C23-(((step)/3+1)/2);
				 else
					 C23 = C23+(((step)/3+1)/2);
			
				 if(C23 > Max_Diagonal)
					 C23 = Max_Diagonal;
				 C21 = 256-C23-C22;
			 }

			 }

		C33 = CCM_Mat[8];
		C33 = C33+step;
		if(C33 < Min_Main_Diagonal)
			C33 = Min_Main_Diagonal;
		if(C33 > Max_Diagonal)
			C33 = Max_Diagonal;
		 if(step % 7== 0)
		{
			C31 = CCM_Mat[6];
			if(step >0)
				C31 = C31-step/7;
			else
				C31 = C31+step/7;
			
			if(C31 > Max_Diagonal)
				C31 = Max_Diagonal;
			C32 = 256-C33-C31;
		}
		else
		{
			C31 = CCM_Mat[6];
			if(step >0)
				C31 = C31-step/7;
			else
				C31 = C31+step/7;
				
			if(C31 > Max_Diagonal)
				C31 = Max_Diagonal;
			C32 = 256-C33-C31;
		}
	}

	//当步长为奇数时
	else
		{
			flagoddCCM_Row_One = -flagoddCCM_Row_One;
			 C11 = CCM_Mat[0];
			 C11 = C11+step/2;
			 if(C11 < Min_Main_Diagonal)
				 C11 = Min_Main_Diagonal;
			 if(C11 > Max_Diagonal)
				 C11 = Max_Diagonal;
			
			 
			 if(flagoddCCM_Row_One == -1)
			 {
				C12 = CCM_Mat[1];
				 if(step >0)
					 C12 = C12-(((step-1)/2+1)/2);
				 else
					 C12 = C12+(((step-1)/2+1)/2);
				
				 if(C12 > Max_Diagonal)
					 C12 = Max_Diagonal;
				 C13 = 256-C11-C12;
			 }
			 else
			 {
				  C13 = CCM_Mat[2];
				 if(step >0)
					 C13 = C13-(((step-1)/2+1)/2);
				 else
					 C13 = C13+(((step-1)/2+1)/2);
			
				 if(C13 > Max_Diagonal)
					 C13 = Max_Diagonal;
				 C12 = 256-C11-C13;		
			 }

			 if(step % 3 ==0 )
			 {
			 flagoddCCM_Row_Two = -flagoddCCM_Row_Two;
			 C22 = CCM_Mat[4];
			 C22 = C22+step/3;
			 if(C22 < Min_Main_Diagonal)
				 C22 = Min_Main_Diagonal;
			 if(C22 > Max_Diagonal)
				 C22 = Max_Diagonal;
			
			 
			if(flagoddCCM_Row_Two == -1)
			 {
				C21 = CCM_Mat[3];
				 if(step >0)
					 C21 = C21-(((step-1)/3+1)/2);
				 else
					 C21 = C21+(((step-1)/3+1)/2);
				
				 if(C21 > Max_Diagonal)
					 C21 = Max_Diagonal;
				 C23 = 256-C22-C21;
			 }
			 else
			 {
				  C23 = CCM_Mat[5];
				 if(step >0)
					 C23 = C23-(((step-1)/3+1)/2);
				 else
					 C23 = C23+(((step-1)/3+1)/2);
			
				 if(C23 > Max_Diagonal)
					 C23 = Max_Diagonal;
				 C21 = 256-C23-C22;
			 }
			}
			else
			 {
			flagoddCCM_Row_Two = -flagoddCCM_Row_Two;
			 C22 = CCM_Mat[4];
			 C22 = C22+step/3;
			 if(C22 < Min_Main_Diagonal)
				 C22 = Min_Main_Diagonal;
			 if(C22 > Max_Diagonal)
				 C22 = Max_Diagonal;
			
			 
			if(flagoddCCM_Row_Two == -1)
			 {
				C21 = CCM_Mat[3];
				 if(step >0)
					 C21 = C21-(((step-1)/3+1)/2);
				 else
					 C21 = C21+(((step-1)/3+1)/2);
				
				 if(C21 > Max_Diagonal)
					 C21 = Max_Diagonal;
				 C23 = 256-C22-C21;
			 }
			 else
			 {
				  C23 = CCM_Mat[5];
				 if(step >0)
					 C23 = C23-(((step-1)/3+1)/2);
				 else
					 C23 = C23+(((step-1)/3+1)/2);
			
				 if(C23 > Max_Diagonal)
					 C23 = Max_Diagonal;
				 C21 = 256-C23-C22;
			 }
			}

    
		C33 = CCM_Mat[8];
		C33 = C33+step;
		if(C33 < Min_Main_Diagonal)
			C33 = Min_Main_Diagonal;
		if(C33 > Max_Diagonal)
			C33 = Max_Diagonal;
		if(step % 7== 0)
		{
			C31 = CCM_Mat[6];
			if(step >0)
				C31 = C31-step/7;
			else
				C31 = C31+step/7;
			
			if(C31 > Max_Diagonal)
				C31 = Max_Diagonal;
			C32 = 256-C33-C31;
		}
		else
		{
			C31 = CCM_Mat[6];
			if(step >0)
				C31 = C31-(step/7);
			else
				C31 = C31+(step/7);
				
			if(C31 > Max_Diagonal)
				C31 = Max_Diagonal;
			C32 = 256-C33-C31;
		}
      }

	  	CCM_Mat[0] = C11;
		CCM_Mat[1] = C12;
		CCM_Mat[2] = C13;
		CCM_Mat[3] = C21;
		CCM_Mat[4] = C22;
		CCM_Mat[5] = C23;
		CCM_Mat[6] = C31;
		CCM_Mat[7] = C32;
		CCM_Mat[8] = C33;
		
		return 0;
}

int CalcInitCCM(int CCM_Mat[],const RGB_temp Original_RGB[],const RGB_temp Target_RGB[],const double weight[],const AK_ISP_RGB_GAMMA_ATTR gamma, int sat)
{
	RGB_temp Stand_RGB[18]={{0,0,0}};
	RGB_temp CCM_RGB[18] = {{0,0,0}};
	LAB Stand_LAB[18] = {{0,0,0}};
	LAB CCM_RGB_Gamma_LAB[18] = {{0,0,0}}; 
	Point_temp MyPoint1[256] = {{0,0}};
	Point_temp MyPoint2[256] = {{0,0}};
	Point_temp PointTmp = {0,0};
	WORD Gamma_line[129] = {0,8,16,24,32,40,48,56,64,72,
		                   80,88,96,104,112,120,128,136,
						   144,152,160,168,176,184,192,
						   200,208,216,224,232,240,248,
						   256,264,272,280,288,296,304,
						   312,320,328,336,344,352,360,
						   368,376,384,392,400,408,416,
	                       424,432,440,448,456,464,472,
						   480,488,496,504,512,520,528,
						   536,544,552,560,568,576,584,
						   592,600,608,616,624,632,640,
	                       648,656,664,672,680,688,696,
						   704,712,720,728,736,744,752,
						   760,768,776,784,792,800,808,
						   816,824,832,840,848,856,864,
						   872,880,888,896,904,912,920,
						   928,936,944,952,960,968,976,
						   984,992,1000,1008,1016,1024};
	int TOTAL_THRESHOLD = 0;
	int i = 0;
	int k = 0;
	int j = 0;
	int MIN = 0;
	int tmp = 0;
	int TmpA = 0;
	int TmpB = 0;
	int Weight = 0;
	int C11,C12,C13,C21,C22,C23,C31,C32,C33;

	if(gamma.rgb_gamma_enable == 1)
	{
		CalcLUT(gamma.r_gamma,MyPoint1);//Gamma 使能所用曲线
	}
	else
	{
		CalcLUT(Gamma_line,MyPoint1);
	}

	for(i = 0;i < 18;i++)
	{
		int oriL;
		Stand_RGB[i].R = Target_RGB[i].R;
		Stand_RGB[i].G = Target_RGB[i].G;
		Stand_RGB[i].B = Target_RGB[i].B;

		Stand_LAB[i].L = RGB2L(Stand_RGB[i].R,Stand_RGB[i].G,Stand_RGB[i].B);
		Stand_LAB[i].a = (RGB2A(Stand_RGB[i].R,Stand_RGB[i].G,Stand_RGB[i].B)-128)*sat/100+128;
		Stand_LAB[i].b = (RGB2B(Stand_RGB[i].R,Stand_RGB[i].G,Stand_RGB[i].B)-128)*sat/100+128;

		oriL = RGB2L(MyPoint1[Original_RGB[i].R].y, MyPoint1[Original_RGB[i].G].y, MyPoint1[Original_RGB[i].B].y);

		Stand_RGB[i].R = CLAMP8( LAB2R(oriL, Stand_LAB[i].a, Stand_LAB[i].b) );
		Stand_RGB[i].G = CLAMP8( LAB2G(oriL, Stand_LAB[i].a, Stand_LAB[i].b) );
		Stand_RGB[i].B = CLAMP8( LAB2B(oriL, Stand_LAB[i].a, Stand_LAB[i].b) );
	}

	for(i =0;i < 18;i++)
	{
		TOTAL_THRESHOLD += (30*weight[i]);
	}

	MIN = TOTAL_THRESHOLD;
	for(C11 = C11_BEGIN ;C11 < C11_END;C11+=LOWFIRSTSTEP)
	{
		for(C12 = C12_BEGIN ;C12 < C12_END;C12 += LOWFIRSTSTEP)
		{
			C13 = 256-(C11+C12);
			if(C13<C13_BEGIN||C13>C13_END)
				continue;
			//printf("%d  %d  %d\n",C11,C12,C13);
			tmp = 0;
			for(i = 0; i < 18; i++)
			{
				CCM_RGB[i].R = CLAMP8((Original_RGB[i].R*C11+Original_RGB[i].G*C12+Original_RGB[i].B*C13)>>8);
				CCM_RGB[i].R = MyPoint1[CCM_RGB[i].R].y;
				TmpA = abs(CCM_RGB[i].R-Stand_RGB[i].R);
				Weight = weight[i]*1024;
				TmpA = TmpA*Weight/1024;
				tmp += TmpA;
			}
			if( (MIN>tmp ) ) 
			{
				MIN = tmp;
				printf("MIN is %d\n",MIN);
				CCM_Mat[0] = C11;
				CCM_Mat[1] = C12;
				CCM_Mat[2] = C13;
			}
		}
	}
	MIN = TOTAL_THRESHOLD;
	for(C21 = C21_BEGIN ;C21 < C21_END ;C21+= LARGESECONDSTEP)
	{
		for(C22 = C22_BEGIN ; C22 < C22_END;C22+=LARGEFIRSTSTEP)
		{
			C23 = 256-(C21+C22);
			if(C23<C23_BEGIN || C23>C23_END)
				continue;
			tmp = 0;
			for(i = 0; i < 18; i++)
			{
				CCM_RGB[i].G = CLAMP8((Original_RGB[i].R*C21+Original_RGB[i].G*C22+Original_RGB[i].B*C23)>>8);
				CCM_RGB[i].G = MyPoint1[CCM_RGB[i].G].y;
				TmpA = abs(CCM_RGB[i].G-Stand_RGB[i].G);
				Weight = weight[i]*1024;
				TmpA = TmpA*Weight/1024;
				tmp += TmpA;
			}
			if( (MIN>tmp ) ) 
			{
				MIN = tmp;
				printf("MIN is %d\n",MIN);
				CCM_Mat[3] = C21;
				CCM_Mat[4] = C22;
				CCM_Mat[5] = C23;
			}
		}
	}
	MIN = TOTAL_THRESHOLD;
	for(C32 = C32_BEGIN ;C32 < C32_END;C32+=LARGESECONDSTEP)
	{
		for(C33 = C33_BEGIN ;C33 < C33_END;C33+=LARGEFIRSTSTEP)
		{
			C31 = 256-(C32+C33);
			if(C31<C31_BEGIN||C31>C31_END)
				continue;
			tmp = 0;
			for(i = 0; i < 18; i++)
			{
				CCM_RGB[i].B = CLAMP8((Original_RGB[i].R*C31+Original_RGB[i].G*C32+Original_RGB[i].B*C33)>>8);
				CCM_RGB[i].B = MyPoint1[CCM_RGB[i].B].y;
				TmpA = abs(CCM_RGB[i].B-Stand_RGB[i].B);
				Weight = weight[i]*1024;
				TmpA = TmpA*Weight/1024;
				tmp += TmpA;
			}
			if( (MIN>tmp ) ) 
			{
				MIN = tmp;
				printf("MIN is %d\n",MIN);
				CCM_Mat[6] = C31;
				CCM_Mat[7] = C32;
				CCM_Mat[8] = C33;
			}
		}
	}

	tmp = 0;
	for(i = 0; i < 18; i++)
	{
		CCM_RGB[i].R = CLAMP8((Original_RGB[i].R*CCM_Mat[0]+Original_RGB[i].G*CCM_Mat[1]+Original_RGB[i].B*CCM_Mat[2])>>8);
		CCM_RGB[i].G = CLAMP8((Original_RGB[i].R*CCM_Mat[3]+Original_RGB[i].G*CCM_Mat[4]+Original_RGB[i].B*CCM_Mat[5])>>8);
		CCM_RGB[i].B = CLAMP8((Original_RGB[i].R*CCM_Mat[6]+Original_RGB[i].G*CCM_Mat[7]+Original_RGB[i].B*CCM_Mat[8])>>8);

		CCM_RGB[i].R = MyPoint1[CCM_RGB[i].R].y;
		CCM_RGB[i].G = MyPoint1[CCM_RGB[i].G].y;
		CCM_RGB[i].B = MyPoint1[CCM_RGB[i].B].y;

		CCM_RGB_Gamma_LAB[i].L = RGB2L(CCM_RGB[i].R,CCM_RGB[i].G,CCM_RGB[i].B);
		CCM_RGB_Gamma_LAB[i].a = RGB2A(CCM_RGB[i].R,CCM_RGB[i].G,CCM_RGB[i].B);
		CCM_RGB_Gamma_LAB[i].b = RGB2B(CCM_RGB[i].R,CCM_RGB[i].G,CCM_RGB[i].B);

		if(CCM_RGB_Gamma_LAB[i].L == 0)
			continue;
		
		TmpA = abs(Stand_LAB[i].a-128-(CCM_RGB_Gamma_LAB[i].a-128));
		TmpB = abs(Stand_LAB[i].b-128-(CCM_RGB_Gamma_LAB[i].b-128));
	
		Weight = weight[i]*1024;
		{
			int min, max;
			if(TmpA>TmpB)
			{
				min = TmpB;
				max = TmpA;
			}
			else
			{
				min = TmpA;
				max = TmpB;
			}
			if(max>3*min)
				TmpA = max+min/8;
			else
				TmpA = max*7/8+min/2;
		}
		TmpA = (Weight*TmpA)>>10;
		tmp += TmpA;
	}
	MIN = tmp;

	printf("init CCM:%d\n", MIN);
	printf("%d %d %d\n",CCM_Mat[0],CCM_Mat[1],CCM_Mat[2]);
	printf("%d %d %d\n",CCM_Mat[3],CCM_Mat[4],CCM_Mat[5]);
	printf("%d %d %d\n",CCM_Mat[6],CCM_Mat[7],CCM_Mat[8]);
	return MIN;
}

//经由Gamma曲线，在限定范围内计算CCM矩阵参数
BOOL CalcCCM(int CCM_Mat[],const RGB_temp Original_RGB[],const RGB_temp Target_RGB[],
		const double weight[],const AK_ISP_RGB_GAMMA_ATTR gamma, int sat)
{
	RGB_temp Stand_RGB[18]={{0,0,0}};
	RGB_temp CCM_RGB[18] = {{0,0,0}};
	LAB Stand_LAB[18] = {{0,0,0}};
	LAB CCM_RGB_Gamma_LAB[18] = {{0,0,0}}; 
	Point_temp MyPoint1[256] = {{0,0}};
	Point_temp MyPoint2[256] = {{0,0}};
	Point_temp PointTmp = {0,0};
	WORD Gamma_line[129] = {0,8,16,24,32,40,48,56,64,72,
		                   80,88,96,104,112,120,128,136,
						   144,152,160,168,176,184,192,
						   200,208,216,224,232,240,248,
						   256,264,272,280,288,296,304,
						   312,320,328,336,344,352,360,
						   368,376,384,392,400,408,416,
	                       424,432,440,448,456,464,472,
						   480,488,496,504,512,520,528,
						   536,544,552,560,568,576,584,
						   592,600,608,616,624,632,640,
	                       648,656,664,672,680,688,696,
						   704,712,720,728,736,744,752,
						   760,768,776,784,792,800,808,
						   816,824,832,840,848,856,864,
						   872,880,888,896,904,912,920,
						   928,936,944,952,960,968,976,
						   984,992,1000,1008,1016,1024};
	int TOTAL_THRESHOLD = 0;
	int RR,RG,RB;
	int GR,GG,GB;
	int BR,BG,BB;
	int i = 0;
	int k = 0;
	int j = 0;
	int MIN = 0;
	int tmp = 0;
	int TmpA = 0;
	int TmpB = 0;
	int Weight = 0;
	int C11,C12,C13,C21,C22,C23,C31,C32,C33;
	int init_CCM_Mat[9];

	for(i = 0;i < 18;i++)
	{
		Stand_RGB[i].R = Target_RGB[i].R;
		Stand_RGB[i].G = Target_RGB[i].G;
		Stand_RGB[i].B = Target_RGB[i].B;

		Stand_LAB[i].L = RGB2L(Stand_RGB[i].R,Stand_RGB[i].G,Stand_RGB[i].B);
		Stand_LAB[i].a = (RGB2A(Stand_RGB[i].R,Stand_RGB[i].G,Stand_RGB[i].B)-128)*sat/100+128;
		Stand_LAB[i].b = (RGB2B(Stand_RGB[i].R,Stand_RGB[i].G,Stand_RGB[i].B)-128)*sat/100+128;

	}

	for(i =0;i < 18;i++)
	{
		TOTAL_THRESHOLD += (30*weight[i]);
	}
	MIN = TOTAL_THRESHOLD;

	MIN = CalcInitCCM(init_CCM_Mat,Original_RGB, Target_RGB, weight, gamma, sat);
	CCM_Mat[0] = init_CCM_Mat[0];
	CCM_Mat[1] = init_CCM_Mat[1];
	CCM_Mat[2] = init_CCM_Mat[2];
	CCM_Mat[3] = init_CCM_Mat[3];
	CCM_Mat[4] = init_CCM_Mat[4];
	CCM_Mat[5] = init_CCM_Mat[5];
	CCM_Mat[6] = init_CCM_Mat[6];
	CCM_Mat[7] = init_CCM_Mat[7];
	CCM_Mat[8] = init_CCM_Mat[8];	
	if(gamma.rgb_gamma_enable == 1)
	{
		CalcLUT(gamma.r_gamma,MyPoint1);//Gamma 使能所用曲线
	}
	else
	{
		CalcLUT(Gamma_line,MyPoint1);
	}
	/////////////////////计算CCM Mat///////////////////////////////
	{
		int centerMIN, maxRange=0;;
		//for(C11 = C11_BEGIN ;C11 < C11_END;C11+=LARGEFIRSTSTEP)
		for(C11 = init_CCM_Mat[0]-128 ;C11 < init_CCM_Mat[0]+128 ;C11+=LARGEFIRSTSTEP)
		{
			//for(C12 = C12_BEGIN ;C12 < C12_END;C12 += LARGESECONDSTEP)
			for(C12 = init_CCM_Mat[1]-128 ;C12 < init_CCM_Mat[1]+128;C12 += LARGESECONDSTEP)
			{
				C13 = 256-(C11+C12);
				if(C13<init_CCM_Mat[2]-128||C13>init_CCM_Mat[2]+128)
					continue;
				printf("%d  %d  %d\n",C11,C12,C13);
				//for(C21 = C21_BEGIN ;C21 < C21_END ;C21+= LARGESECONDSTEP)
				for(C21 = init_CCM_Mat[3]-128 ;C21 < init_CCM_Mat[3]+128 ;C21+= LARGESECONDSTEP)
				{
					//for(C22 = C22_BEGIN ; C22 < C22_END;C22+=LARGEFIRSTSTEP)
					for(C22 = init_CCM_Mat[4]-128 ; C22 < init_CCM_Mat[4]+128;C22+=LARGEFIRSTSTEP)
					{
						C23 = 256-(C21+C22);
						if(C23<init_CCM_Mat[5]-128 || C23>init_CCM_Mat[5]+128)
							continue;

						//for(C32 = C32_BEGIN ;C32 < C32_END;C32+=LARGESECONDSTEP)
						for(C32 = init_CCM_Mat[7]-128 ;C32 < init_CCM_Mat[7]+128;C32+=LARGESECONDSTEP)
						{
							//for(C33 = C33_BEGIN ;C33 < C33_END;C33+=LARGEFIRSTSTEP)
							for(C33 = init_CCM_Mat[8]-128 ;C33 < init_CCM_Mat[8]+128;C33+=LARGEFIRSTSTEP)
							{
								C31 = 256-(C32+C33);
								if(C31<init_CCM_Mat[6]-128||C31>init_CCM_Mat[6]+128)
									continue;

								tmp = 0;
								for(i = 0; i < 18; i++)
								{
									CCM_RGB[i].R = CLAMP8((Original_RGB[i].R*C11+Original_RGB[i].G*C12+Original_RGB[i].B*C13)>>8);
									CCM_RGB[i].G = CLAMP8((Original_RGB[i].R*C21+Original_RGB[i].G*C22+Original_RGB[i].B*C23)>>8);
									CCM_RGB[i].B = CLAMP8((Original_RGB[i].R*C31+Original_RGB[i].G*C32+Original_RGB[i].B*C33)>>8);

									CCM_RGB[i].R = MyPoint1[CCM_RGB[i].R].y;
									CCM_RGB[i].G = MyPoint1[CCM_RGB[i].G].y;
									CCM_RGB[i].B = MyPoint1[CCM_RGB[i].B].y;

									CCM_RGB_Gamma_LAB[i].L = RGB2L(CCM_RGB[i].R,CCM_RGB[i].G,CCM_RGB[i].B);
									CCM_RGB_Gamma_LAB[i].a = RGB2A(CCM_RGB[i].R,CCM_RGB[i].G,CCM_RGB[i].B);
									CCM_RGB_Gamma_LAB[i].b = RGB2B(CCM_RGB[i].R,CCM_RGB[i].G,CCM_RGB[i].B);

									//assert(CCM_RGB_Gamma_LAB[i].L == 0);
									//if(CCM_RGB_Gamma_LAB[i].L == 0)
									//	assert(1);
									//k =(Stand_LAB[i].L*1024/CCM_RGB_Gamma_LAB[i].L);
									k =1024;
									
									//TmpA = (int)(weight[i]*abs(Stand_LAB[i].a-128-((k*(CCM_RGB_Gamma_LAB[i].a-128))>>10)));
									//TmpB = (int)(weight[i]*abs(Stand_LAB[i].b-128-((k*(CCM_RGB_Gamma_LAB[i].b-128))>>10)));
									TmpA = abs(Stand_LAB[i].a-128-((k*(CCM_RGB_Gamma_LAB[i].a-128))>>10));
									TmpB = abs(Stand_LAB[i].b-128-((k*(CCM_RGB_Gamma_LAB[i].b-128))>>10));
									
									Weight = weight[i]*1024;	

									{
										int min, max;
										if(TmpA>TmpB)
										{
											min = TmpB;
											max = TmpA;
										}
										else
										{
											min = TmpA;
											max = TmpB;
										}
										if(max>3*min)
											TmpA = max+min/8;
										else
											TmpA = max*7/8+min/2;
									}
									if(TmpA > THRESHOLD)
										break;
									TmpA = (Weight*TmpA)>>10;
									tmp += TmpA;
									//tmp += TmpA*TmpA;
									//if(tmp>TOTAL_THRESHOLD)
									//if(tmp>MIN*MIN*333/256)
									if(tmp>MIN*333/256)
										break;
								}
								if(i != 18)
									continue;						
							
								if(tmp>TOTAL_THRESHOLD)
									continue;
								else
								{
									TOTAL_THRESHOLD = tmp;
									centerMIN = tmp;
									for(RR = C11-LARGEFIRSTSTEP/2;RR < C11+LARGEFIRSTSTEP/2;RR+=LOWFIRSTSTEP)
									{
										for(RG = C12-LARGESECONDSTEP/2;RG < C12+LARGESECONDSTEP/2;RG += LOWSECONDSTEP)
										{
											RB = 256-(RR+RG);
											for(GR = C21-LARGEFIRSTSTEP/2 ;GR < C21+LARGEFIRSTSTEP/2;GR+= LOWSECONDSTEP)
											{
												for(GG = C22-LARGESECONDSTEP/2; GG< C22+LARGESECONDSTEP/2;GG+=LOWFIRSTSTEP)
												{
													GB = 256-(GG+GR);
													for(BG = C32-LARGEFIRSTSTEP/2;BG < C32+LARGEFIRSTSTEP/2;BG+=LOWSECONDSTEP)
													{
														for(BB = C33-LARGESECONDSTEP/2;BB < C33+LARGESECONDSTEP/2;BB+=LOWFIRSTSTEP)
														{
															BR = 256-(BB+BG);

																tmp = 0;
																for(i = 0; i < 18; i++)
																{
																	CCM_RGB[i].R = CLAMP8((Original_RGB[i].R*RR+Original_RGB[i].G*RG+Original_RGB[i].B*RB)>>8);
																	CCM_RGB[i].G = CLAMP8((Original_RGB[i].R*GR+Original_RGB[i].G*GG+Original_RGB[i].B*GB)>>8);
																	CCM_RGB[i].B = CLAMP8((Original_RGB[i].R*BR+Original_RGB[i].G*BG+Original_RGB[i].B*BB)>>8);

																	CCM_RGB[i].R = MyPoint1[CCM_RGB[i].R].y;
																	CCM_RGB[i].G = MyPoint1[CCM_RGB[i].G].y;
																	CCM_RGB[i].B = MyPoint1[CCM_RGB[i].B].y;

																	CCM_RGB_Gamma_LAB[i].L = RGB2L(CCM_RGB[i].R,CCM_RGB[i].G,CCM_RGB[i].B);
																	CCM_RGB_Gamma_LAB[i].a = RGB2A(CCM_RGB[i].R,CCM_RGB[i].G,CCM_RGB[i].B);
																	CCM_RGB_Gamma_LAB[i].b = RGB2B(CCM_RGB[i].R,CCM_RGB[i].G,CCM_RGB[i].B);
																	
																	//assert(CCM_RGB_Gamma_LAB[i].L == 0);

																	//考虑图像亮度
																	//k =(Stand_LAB[i].L*1024/CCM_RGB_Gamma_LAB[i].L);
																	k =1024;
																	//TmpA = (int)(weight[i]*abs(Stand_LAB[i].a-128-((k*(CCM_RGB_Gamma_LAB[i].a-128))>>10)));
																	//TmpB = (int)(weight[i]*abs(Stand_LAB[i].b-128-((k*(CCM_RGB_Gamma_LAB[i].b-128))>>10)));
																	TmpA = abs(Stand_LAB[i].a-128-((k*(CCM_RGB_Gamma_LAB[i].a-128))>>10));
																	TmpB = abs(Stand_LAB[i].b-128-((k*(CCM_RGB_Gamma_LAB[i].b-128))>>10));
																	Weight = weight[i]*1024;


																	{
																		int min, max;
																		if(TmpA>TmpB)
																		{
																			min = TmpB;
																			max = TmpA;
																		}
																		else
																		{
																			min = TmpA;
																			max = TmpB;

																		}
																		if(max>3*min)
																			TmpA = max+min/8;
																		else
																			TmpA = max*7/8+min/2;
																	}
																	if(TmpA > THRESHOLD)
																		break;
																	TmpA = (TmpA*Weight)>>10;
																	tmp += TmpA;
																	//tmp += TmpA*TmpA;
																	if(tmp>MIN)
																	//if(tmp>MIN*MIN)
																		break;
																}
																if(i != 18)
																	continue;
																if(abs(tmp-centerMIN)*256/centerMIN>maxRange)
																{
																	maxRange = abs(tmp-centerMIN)*256/centerMIN;
																	printf("maxRange is %d\n",maxRange);
																}

																if( (MIN>tmp ) ) 
																{
																	MIN = tmp;
																	printf("MIN is %d\n",MIN);
																	CCM_Mat[0] = RR;
																	CCM_Mat[1] = RG;
																	CCM_Mat[2] = RB;
																	CCM_Mat[3] = GR;
																	CCM_Mat[4] = GG;
																	CCM_Mat[5] = GB;
																	CCM_Mat[6] = BR;
																	CCM_Mat[7] = BG;
																	CCM_Mat[8] = BB;	
																	/*CCM_Mat[0] = 402;
																	CCM_Mat[1] = -160;
																	CCM_Mat[2] = 14;
																	CCM_Mat[3] = 154;
																	CCM_Mat[4] = 558;
																	CCM_Mat[5] = -456;
																	CCM_Mat[6] = -352;
																	CCM_Mat[7] = -124;
																	CCM_Mat[8] = 732;*/
																	
																}
														}
													}
												}
											}
										}
									}
								}
								

							}
						}
					}
				}
			}
		}

	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CCCM_ImgDlg dialog


CCCM_ImgDlg::CCCM_ImgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCCM_ImgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCCM_ImgDlg)
	//}}AFX_DATA_INIT
	UINT i = 0, j = 0;
	m_bRefreshFlag = FALSE;
	m_bInit = FALSE;
	ZeroMemory(&m_imgpath, 260);
	ZeroMemory(m_weight, sizeof(m_weight));
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 6; j++)
		{
			m_weight[i][j] = 1;
		}
	}
}


void CCCM_ImgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCCM_ImgDlg)
	DDX_Control(pDX, IDC_SLIDER_SATURATION, m_slider_saturation);
	DDX_Control(pDX, IDC_SPIN_SATURATION, m_spin_saturation);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	//}}AFX_DATA_MAP

	DDX_Text(pDX, IDC_EDIT_W00, m_weight[0][0]);
	DDV_MinMaxInt(pDX, m_weight[0][0], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W01, m_weight[0][1]);
	DDV_MinMaxInt(pDX, m_weight[0][1], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W02, m_weight[0][2]);
	DDV_MinMaxInt(pDX, m_weight[0][2], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W03, m_weight[0][3]);
	DDV_MinMaxInt(pDX, m_weight[0][3], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W04, m_weight[0][4]);
	DDV_MinMaxInt(pDX, m_weight[0][4], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W05, m_weight[0][5]);
	DDV_MinMaxInt(pDX, m_weight[0][5], 0, 256);

	DDX_Text(pDX, IDC_EDIT_W10, m_weight[1][0]);
	DDV_MinMaxInt(pDX, m_weight[1][0], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W11, m_weight[1][1]);
	DDV_MinMaxInt(pDX, m_weight[1][1], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W12, m_weight[1][2]);
	DDV_MinMaxInt(pDX, m_weight[1][2], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W13, m_weight[1][3]);
	DDV_MinMaxInt(pDX, m_weight[1][3], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W14, m_weight[1][4]);
	DDV_MinMaxInt(pDX, m_weight[1][4], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W15, m_weight[1][5]);
	DDV_MinMaxInt(pDX, m_weight[1][5], 0, 256);

	DDX_Text(pDX, IDC_EDIT_W20, m_weight[2][0]);
	DDV_MinMaxInt(pDX, m_weight[2][0], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W21, m_weight[2][1]);
	DDV_MinMaxInt(pDX, m_weight[2][1], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W22, m_weight[2][2]);
	DDV_MinMaxInt(pDX, m_weight[2][2], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W23, m_weight[2][3]);
	DDV_MinMaxInt(pDX, m_weight[2][3], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W24, m_weight[2][4]);
	DDV_MinMaxInt(pDX, m_weight[2][4], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W25, m_weight[2][5]);
	DDV_MinMaxInt(pDX, m_weight[2][5], 0, 256);

	DDX_Text(pDX, IDC_EDIT_W30, m_weight[3][0]);
	DDV_MinMaxInt(pDX, m_weight[3][0], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W31, m_weight[3][1]);
	DDV_MinMaxInt(pDX, m_weight[3][1], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W32, m_weight[3][2]);
	DDV_MinMaxInt(pDX, m_weight[3][2], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W33, m_weight[3][3]);
	DDV_MinMaxInt(pDX, m_weight[3][3], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W34, m_weight[3][4]);
	DDV_MinMaxInt(pDX, m_weight[3][4], 0, 256);
	DDX_Text(pDX, IDC_EDIT_W35, m_weight[3][5]);
	DDV_MinMaxInt(pDX, m_weight[3][5], 0, 256);
}


BEGIN_MESSAGE_MAP(CCCM_ImgDlg, CDialog)
	//{{AFX_MSG_MAP(CCCM_ImgDlg)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON_RESET, OnButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_GET_YUV_IMG, OnButtonGetYuvImg)
	ON_BN_CLICKED(IDC_BUTTON_OPEN_YUV_IMG, OnButtonOpenYuvImg)
	ON_BN_CLICKED(IDC_BUTTON_RGB_VAL, OnButtonRgbVal)
	ON_BN_CLICKED(IDC_BUTTON_CALC, OnButtonCalc)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	ON_BN_CLICKED(IDC_RADIO_A_CCM, OnRadioACcm)
	ON_BN_CLICKED(IDC_RADIO_TL84_CCM, OnRadioTl84Ccm)
	ON_BN_CLICKED(IDC_RADIO_D50_CCM, OnRadioD50Ccm)
	ON_BN_CLICKED(IDC_RADIO_D65_CCM, OnRadioD65Ccm)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_CCM, OnButtonExportCcm)
	ON_NOTIFY(NM_OUTOFMEMORY, IDC_SPIN_SATURATION, OnOutofmemorySpinSaturation)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SATURATION, OnDeltaposSpinSaturation)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_SATURATION, OnCustomdrawSliderSaturation)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_SATURATION, OnReleasedcaptureSliderSaturation)
	ON_EN_CHANGE(IDC_EDIT_SATURATION, OnChangeEditSaturation)
	ON_EN_KILLFOCUS(IDC_EDIT_SATURATION, OnKillfocusEditSaturation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCCM_ImgDlg message handlers
BOOL CCCM_ImgDlg::OnInitDialog()
{
	CString str;

	CDialog::OnInitDialog();

	m_yuvbuf = NULL;
	m_bmpbuf = NULL;
	m_bmpbuf_out = NULL;

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

	m_bOutput = FALSE;
	m_drag = FALSE;
	m_moveflag = -1;

	Saturation = 128;

	cal_flag = FALSE;

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

	m_tab.InsertItem(0, "Input");
	m_tab.InsertItem(1, "Output");

	threshold = 30;
	str.Format("%d", threshold);
	SetDlgItemText(IDC_EDIT_THRESHOLD, str);
	
	Show_flag = FALSE;
	Calculate_flag = FALSE;
	g_Calc_Thread = INVALID_HANDLE_VALUE;
	((CButton *)GetDlgItem(IDC_RADIO_A_CCM))->SetCheck(1);

	Invalidate();
	return TRUE;
}

BOOL CCCM_ImgDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)   
    {   
        if (pMsg->wParam == VK_RETURN)   
            return TRUE;
    }

	return CDialog::PreTranslateMessage(pMsg);
}


void CCCM_ImgDlg::OnPaint() 
{	
	int i = 0;
	int j = 0;

	UpdateData(FALSE);
	
	CPaintDC dc(this);

	if (m_bRefreshFlag)
	{
		GetBMP();
		m_bRefreshFlag = FALSE;
	}

	if (m_bOutput)
	{
		if (Show_flag && Calculate_flag)
		{
		    BMShow_output(&dc, m_ImgRect.left, m_ImgRect.top, m_img_width/m_img_mutil, m_img_height/m_img_mutil);
		}
		else
		{
			BMShow(&dc, m_ImgRect.left, m_ImgRect.top, m_img_width/m_img_mutil, m_img_height/m_img_mutil);
		}
	}
	else
	{
		BMShow(&dc, m_ImgRect.left, m_ImgRect.top, m_img_width/m_img_mutil, m_img_height/m_img_mutil);
	}
	

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

	if (0)//m_bOutput)
	{
		for (i=0; i<4; i++)
		{
			for (j=0; j<6; j++)
			{
				dc.FillSolidRect(m_framePoint[i][j][0].x+m_ImgRect.left, 
					m_framePoint[i][j][0].y+m_ImgRect.top, 
					m_framePoint[i][j][1].x-m_framePoint[i][j][0].x,  
					m_framePoint[i][j][1].y-m_framePoint[i][j][0].y, 
					RGB(m_RGBvalDlg.R_out[i][j], m_RGBvalDlg.G_out[i][j], m_RGBvalDlg.B_out[i][j]));
			}
		}
	}
	

	CDialog::OnPaint();
}

void CCCM_ImgDlg::OnClose() 
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

	if (NULL != m_bmpbuf_out)
	{
		free(m_bmpbuf_out);
		m_bmpbuf_out = NULL;
	}


	CDialog::OnClose();
}

void CCCM_ImgDlg::SetImgPath(char* path) 
{
	strcpy(m_imgpath, path);

	if (m_bInit)
	{
		Invalidate();
	}

	m_bRefreshFlag = TRUE;
}

void CCCM_ImgDlg::BMShow_output(CDC *pDC, int x, int y, int width, int height) 
{
	if (NULL == m_bmpbuf)
		return;
	
	pDC->SetStretchBltMode(COLORONCOLOR);
	StretchDIBits(pDC->GetSafeHdc(),
		x, y, width, height, 0, 0, m_img_width, m_img_height,
		m_bmpbuf_out + BMP_HEADINFO_LEN,
		(BITMAPINFO*)(m_bmpbuf_out + sizeof(BITMAPFILEHEADER)),
		DIB_RGB_COLORS, SRCCOPY);
}

void CCCM_ImgDlg::BMShow(CDC *pDC, int x, int y, int width, int height) 
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

bool CCCM_ImgDlg::GetBMP(void) 
{
	T_U32 line_byte = 0;

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

	if (m_bmpbuf_out != NULL)
	{
		free(m_bmpbuf_out);
		m_bmpbuf_out = NULL;
	}

	line_byte = (m_img_width*3+3) & ~0x3;
	m_bmpbuf_out = (T_U8*)malloc(BMP_HEADINFO_LEN+m_img_height*line_byte);
	if (NULL == m_bmpbuf_out)
	{
		AfxMessageBox("m_bmpbuf_out malloc fail，pls chk", MB_OK);
		return FALSE;
	}

	return TRUE;
}

void CCCM_ImgDlg::OnLButtonDown(UINT nFlags, CPoint point) 
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

void CCCM_ImgDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_drag)
		m_drag = FALSE;

	CDialog::OnLButtonUp(nFlags, point);

	Calc_Frames();
	Calc_Input_RGB();
}

void CCCM_ImgDlg::OnMouseMove(UINT nFlags, CPoint point) 
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

void CCCM_ImgDlg::OnButtonReset() 
{
	// TODO: Add your control notification handler code here
}


T_U8* CCCM_ImgDlg::LoadYuvData(char* path) 
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

void CCCM_ImgDlg::Img_SetConnectState(bool bConnect) 
{
	// TODO: Add your control notification handler code here
	
	m_img_bConnect = bConnect;
}

void CCCM_ImgDlg::OnButtonGetYuvImg() 
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

	OnButtonReset();
	Show_flag = TRUE;
	m_tab.SetCurSel(0);
	Invalidate();
	m_bOutput = FALSE;
	Calculate_flag = FALSE;
	Calc_Input_RGB();
}

void CCCM_ImgDlg::OnButtonOpenYuvImg() 
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, "*.yuv", NULL, OFN_HIDEREADONLY,
		"Data File(*.yuv)|*.yuv|All Files (*.*)|*.*||");
	if (dlg.DoModal()!=IDOK)
		return;

	
	SetImgPath((LPSTR)(LPCTSTR)dlg.GetPathName());

	OnButtonReset();
	Show_flag = TRUE;
	m_tab.SetCurSel(0);
	Invalidate();
	m_bOutput = FALSE;
	Calculate_flag = FALSE;
	Calc_Input_RGB();
}

void CCCM_ImgDlg::Calc_Frames() 
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

void CCCM_ImgDlg::OnButtonRgbVal() 
{
	// TODO: Add your control notification handler code here
	//Calc_Input_RGB();
	m_RGBvalDlg.DoModal();
}

void CCCM_ImgDlg::Calc_Input_RGB() 
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

void CCCM_ImgDlg::Set_CCMValue(void)
{
	CWnd *pWnd = NULL;
	CString str;

	pWnd = GetDlgItem(IDC_STATIC_C0);
	str.Format("%d", CCM_Mat[0]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_C1);
	str.Format("%d", CCM_Mat[1]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_C2);
	str.Format("%d", CCM_Mat[2]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_C3);
	str.Format("%d", CCM_Mat[3]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_C4);
	str.Format("%d", CCM_Mat[4]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_C5);
	str.Format("%d", CCM_Mat[5]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_C6);
	str.Format("%d", CCM_Mat[6]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_C7);
	str.Format("%d", CCM_Mat[7]);
	pWnd->SetWindowText(str);

	pWnd = GetDlgItem(IDC_STATIC_C8);
	str.Format("%d", CCM_Mat[8]);
	pWnd->SetWindowText(str);
}


void Calc_ccm(void *argv)
{
	CWnd *pWnd = NULL;
	CString str;
	CCCM_ImgDlg* pthis = (CCCM_ImgDlg*)argv;
	RGB_temp Original_RGB[24] = {0};
	RGB_temp Target_RGB[24] = {0};
	double weight[24] = {0};
	int CCM_Mat_temp[9] = {256,0,0,0,256,0,0,0,256};
	UINT i = 0, j = 0, m = 0, n = 0;
	int sat = 0;

	memset(Original_RGB, 0, sizeof(RGB_temp)*24);
	memset(Target_RGB, 0, sizeof(RGB_temp)*24);
	memset(pthis->CCM_Mat, 0, sizeof(int)*24);
	memset(pthis->Out_RGB, 0, sizeof(RGB_temp)*24);
	memcpy(pthis->CCM_Mat, CCM_Mat_temp, sizeof(int)*24);
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
			
			memcpy(&weight[n], &pthis->m_weight[i][j], sizeof(double));
			n = n + 1;
		}
	}

	pthis->GetDlgItemText(IDC_EDIT_SATURATION, str);
	if (str.IsEmpty())
	{
		AfxMessageBox("THRESHOLD IS NULL, PLS CHk");
		return;
	}
	sat = atoi(str);

	CalcCCM(pthis->CCM_Mat, Original_RGB,Target_RGB, weight,pthis->gamma_ccm, sat);
	memcpy(pthis->cal_CCM_Mat, pthis->CCM_Mat, 9*sizeof(int));
	CalcNewRGB(pthis->Out_RGB, pthis->CCM_Mat,Original_RGB,pthis->gamma_ccm);
	OutImage(pthis->m_bmpbuf_out, pthis->CCM_Mat, pthis->m_bmpbuf, pthis->gamma_ccm);
	pthis->Set_CCMValue();

	m = 0;
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 6; j++)
		{
			memcpy(&pthis->m_RGBvalDlg.R_out[i][j], &pthis->Out_RGB[m].R, 1);
			memcpy(&pthis->m_RGBvalDlg.G_out[i][j], &pthis->Out_RGB[m].G, 1);
			memcpy(&pthis->m_RGBvalDlg.B_out[i][j], &pthis->Out_RGB[m].B, 1);
			
			m = m + 1;
		}
	}

	pWnd = pthis->GetDlgItem(IDC_STATIC_CAL);
	str.Format("Calculat finish");
	pWnd->SetWindowText(str);

	pthis->GetDlgItem(IDC_BUTTON_GET_YUV_IMG)->EnableWindow(TRUE);//可用
	pthis->GetDlgItem(IDC_BUTTON_OPEN_YUV_IMG)->EnableWindow(TRUE);//可用
	pthis->GetDlgItem(IDC_BUTTON_RGB_VAL)->EnableWindow(TRUE);//可用
	pthis->GetDlgItem(IDC_CHECK_GAMMA_ENABLE)->EnableWindow(TRUE);//可用
	pthis->GetDlgItem(IDC_BUTTON_CALC)->EnableWindow(TRUE);//可用
	pthis->GetDlgItem(IDC_BUTTON_EXPORT_CCM)->EnableWindow(TRUE);//可用

	pthis->m_tab.SetCurSel(1);
	pthis->m_bOutput = TRUE;
	pthis->Calculate_flag = TRUE;
	pthis->Invalidate();


}


BOOL CCCM_ImgDlg::Creat_Calc_thread(void) 
{
	UINT idex = 0;
	
	if (g_Calc_Thread != INVALID_HANDLE_VALUE)
	{
		Close_Calc_thread();
		g_Calc_Thread = INVALID_HANDLE_VALUE;
	}
	g_Calc_Thread = CreateThread(NULL, TRANS_STACKSIZE, (LPTHREAD_START_ROUTINE)Calc_ccm, this, 0, NULL);
	if (g_Calc_Thread == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	
	return TRUE;
	
}

void CCCM_ImgDlg::Close_Calc_thread(void) 
{
	if(g_Calc_Thread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(g_Calc_Thread);
		g_Calc_Thread = INVALID_HANDLE_VALUE;
	}
}


void CCCM_ImgDlg::OnButtonCalc() 
{

	CWnd *pWnd = NULL;
	CString str;
	
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	GetDlgItem(IDC_BUTTON_GET_YUV_IMG)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_OPEN_YUV_IMG)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_RGB_VAL)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_CHECK_GAMMA_ENABLE)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_CALC)->EnableWindow(FALSE);//可用
	GetDlgItem(IDC_BUTTON_EXPORT_CCM)->EnableWindow(FALSE);//可用
	

	gamma_ccm.rgb_gamma_enable =((CButton *)GetDlgItem(IDC_CHECK_GAMMA_ENABLE))->GetCheck();

	if (Creat_Calc_thread() == FALSE)
	{
		AfxMessageBox("Creat_Calc_thread fail", MB_OK);
		return;
	}
	
	pWnd = GetDlgItem(IDC_STATIC_CAL);
	str.Format("Calculating...");
	pWnd->SetWindowText(str);
}

void CCCM_ImgDlg::OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	int id = m_tab.GetCurSel();

	if (0 == id) //input
	{
		m_bOutput = FALSE;
	}
	else  //output
	{
		m_bOutput = TRUE;
	}

	Invalidate();
	*pResult = 0;
}

void CCCM_ImgDlg::OnRadioACcm() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCM_ImgDlg::OnRadioTl84Ccm() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCM_ImgDlg::OnRadioD50Ccm() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCM_ImgDlg::OnRadioD65Ccm() 
{
	// TODO: Add your control notification handler code here
	
}

void CCCM_ImgDlg::OnButtonExportCcm() 
{
	// TODO: Add your control notification handler code here
	if (((CButton *)GetDlgItem(IDC_RADIO_A_CCM))->GetCheck())
	{
		ccm_type = A_CCM;
	}
	else if(((CButton *)GetDlgItem(IDC_RADIO_TL84_CCM))->GetCheck())
	{
		ccm_type = TL84_CCM;
	}
	else if(((CButton *)GetDlgItem(IDC_RADIO_D50_CCM))->GetCheck())
	{
		ccm_type = D50_CCM;	
	}
	else if(((CButton *)GetDlgItem(IDC_RADIO_D65_CCM))->GetCheck())
	{
		ccm_type = D65_CCM;
	}
	else
	{
		ccm_type = A_CCM;
	}

	export_ccm_flag = TRUE;

	AfxMessageBox("export ccm success", MB_OK);
}

void CCCM_ImgDlg::OnOutofmemorySpinSaturation(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	UINT i = 0;

	i++;
	*pResult = 0;
}

void CCCM_ImgDlg::OnDeltaposSpinSaturation(NMHDR* pNMHDR, LRESULT* pResult) 
{

	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	// TODO: Add your control notification handler code here
	m_slider_saturation.SetPos((int)m_spin_saturation.GetPos());
	*pResult = 0;
}

void CCCM_ImgDlg::OnCustomdrawSliderSaturation(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int i = 0;
	int j = 0, m = 0;
	RGB_temp Original_RGB[24] = {0};
	int temp = 0;

	// TODO: Add your control notification handler code here
	m_spin_saturation.SetPos((int)m_slider_saturation.GetPos());
	temp = 	m_slider_saturation.GetPos();
	if (last_Saturation != temp)
	{
		last_Saturation = temp;
		show_ccm();
	}

	*pResult = 0;
}

void CCCM_ImgDlg::OnReleasedcaptureSliderSaturation(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	cal_flag =TRUE;

    Saturation = m_slider_saturation.GetPos();
	
	*pResult = 0;
}

void CCCM_ImgDlg::show_ccm(void) 
{

	RGB_temp Original_RGB[24] = {0};
	UINT i = 0, j = 0, m = 0;
	
	memset(Original_RGB, 0, sizeof(RGB_temp)*24);
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 6; j++)
		{
			memcpy(&Original_RGB[m].R, &m_RGBvalDlg.R_in[i][j], 1);
			memcpy(&Original_RGB[m].G, &m_RGBvalDlg.G_in[i][j], 1);
			memcpy(&Original_RGB[m].B, &m_RGBvalDlg.B_in[i][j], 1);
			m = m + 1;
		}
	}
	
	memcpy(CCM_Mat, cal_CCM_Mat, 9*sizeof(int));
	CalcSaturation(CCM_Mat, m_slider_saturation.GetPos());
	
	CalcNewRGB(Out_RGB, CCM_Mat,Original_RGB,gamma_ccm);
	OutImage(m_bmpbuf_out, CCM_Mat, m_bmpbuf, gamma_ccm);
	Set_CCMValue();
	Invalidate();
}

void CCCM_ImgDlg::OnChangeEditSaturation() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void CCCM_ImgDlg::OnKillfocusEditSaturation() 
{
	// TODO: Add your control notification handler code here
	m_slider_saturation.SetPos((int)m_spin_saturation.GetPos());
}
