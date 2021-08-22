#ifndef HAL_AK3918_H
#define HAL_AK3918_H

#define CLEAR(x) memset(&(x), 0, sizeof(x))

typedef struct _ENC_INPUT_PAR
{
	unsigned long	width;			//实际编码图像的宽度，能被4整除
	unsigned long	height;			//实际编码图像的长度，能被2整除 
	unsigned char   kbpsmode;
	signed long	qpHdr;			//初始的QP的值
	signed long	iqpHdr;			//初始的i帧的QP值
	signed long minQp;		//动态码率参数[20,25]
	signed long maxQp;		//动态码率参数[45,50]
	signed long framePerSecond; //帧率
	signed long	bitPerSecond;	//目标bps
	unsigned long 	video_tytes;
	unsigned long	size;
}T_ENC_INPUT;

struct tagRECORD_VIDEO_FONT_CTRL {
	unsigned char *y;
	unsigned char *u;
	unsigned char *v;
	unsigned long color;
	unsigned long width;
};

int ak3918_init_dma_memory();
int camera_open(int width, int height);
int encode_open(T_ENC_INPUT *pencInput);
int video_process_start();

int SetBrightness(int bright);
int SetGAMMA(int cid);
int SetSATURATION(int sat);



#endif
