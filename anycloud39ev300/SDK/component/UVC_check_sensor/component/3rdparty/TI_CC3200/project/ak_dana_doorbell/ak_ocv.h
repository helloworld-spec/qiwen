#ifndef __AK_OCV_H
#define __AK_OCV_H	

 #define ABS(x)				((x) >0 ? (x) : -(x) )

 /*初始化开路电压对应百分比表*/

#define OCVREG0				0x00		//3.1328		//0%
#define OCVREG1				0x01		//3.2736		//1%
#define OCVREG2				0x02		//3.4144		//2%
#define OCVREG3				0x05		//3.5552		//5%
#define OCVREG4				0x07		//3.6256		//7%
#define OCVREG5				0x0D		//3.6608		//13%
#define OCVREG6				0x10		//3.6960		//16%
#define OCVREG7				0x1A		//3.7312		//26%
#define OCVREG8				0x24		//3.7664		//36%
#define OCVREG9				0x2E		//3.8016		//46%
#define OCVREGA				0x35		//3.8368		//53%
#define OCVREGB				0x3D		//3.8720		//61%
#define OCVREGC				0x49		//3.9424		//73%
#define OCVREGD				0x54		//4.0128		//84%
#define OCVREGE				0x5C		//4.0832		//92%
#define OCVREGF				0x64		//4.1536		//100%

/* 初始化开路电压*/

#define OCVVOL0				3132
#define OCVVOL1				3273
#define OCVVOL2				3414
#define OCVVOL3				3555
#define OCVVOL4				3625
#define OCVVOL5				3660
#define OCVVOL6				3696
#define OCVVOL7				3731
#define OCVVOL8				3766
#define OCVVOL9				3801
#define OCVVOLA				3836
#define OCVVOLB				3872
#define OCVVOLC				3942
#define OCVVOLD				4012
#define OCVVOLE				4083
#define OCVVOLF				4153


 unsigned char axp_ocv_restcap(void);



#endif

