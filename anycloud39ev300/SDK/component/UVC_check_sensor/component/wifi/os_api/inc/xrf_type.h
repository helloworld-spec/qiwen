#ifndef XRF_TYPE_H
#define XRF_TYPE_H

#ifdef __cplusplus
extern "C"
{
	#endif

	/*
	;*****************************************************************************************************
	;* 描    述 : 定义系统的数据类型。
	;*****************************************************************************************************
	;*/
	typedef unsigned char boolean;
	typedef unsigned char uint8; /* Unsigned  8 bit quantity                            */
	typedef signed char int8; /* Signed    8 bit quantity                            */
	typedef unsigned short uint16; /* Unsigned 16 bit quantity                            */
	typedef signed short int16; /* Signed   16 bit quantity                            */
	typedef unsigned int uint32; /* Unsigned 32 bit quantity                            */
	typedef signed int int32; /* Signed   32 bit quantity                            */
	typedef float fp32; /* Single precision floating point                     */

	typedef unsigned char BYTE;
	typedef unsigned short WORD;
	typedef unsigned long DWORD;
	
	typedef unsigned char uint8_t; /* Unsigned  8 bit quantity                            */
	typedef signed char int8_t; /* Signed    8 bit quantity                            */
	typedef unsigned short uint16_t; /* Unsigned 16 bit quantity                            */
	typedef signed short int16_t; /* Signed   16 bit quantity                            */
	typedef unsigned int uint32_t; /* Unsigned 32 bit quantity                            */


	typedef unsigned char u_int8_t; /* Unsigned  8 bit quantity                            */
	typedef unsigned short u_int16_t; /* Unsigned 16 bit quantity                            */
	typedef unsigned int u_int32_t; /* Unsigned 32 bit quantity                            */
	typedef signed int int32_t; /* Signed   32 bit quantity                            */

	typedef unsigned short u_short;


	typedef unsigned char u_char;

	//wmj-
	//typedef unsigned char u8_t;
	//typedef signed char s8_t;
	//typedef unsigned short u16_t;
	//typedef signed short s16_t;
	//typedef unsigned int u32_t;
	//typedef signed int s32_t;

	typedef uint32_t u32;
	typedef uint16_t u16;
	typedef uint8_t u8;

	typedef unsigned short __le16;
	typedef unsigned int __le32;
	typedef unsigned long long __le64;

	typedef unsigned short __be16;
	typedef unsigned int __be32;
	//typedef unsigned __int64  __be64;
	typedef unsigned short __u16;
	typedef unsigned char __u8;
        typedef unsigned long long u64; //mark 
	typedef unsigned int __u32;


	typedef signed char s8;
	typedef signed short s16;
	typedef signed int s32;
	//typedef  signed __int64  s64;

	typedef signed char __s8;
	typedef signed short __s16;
	typedef signed int __s32;
	//typedef __int64  __s64;
	
	//typedef unsigned __int64 __u64;

	#define __user
	

	typedef unsigned int size_t;
#ifndef TYPEDEF_CHAR
#define TYPEDEF_CHAR
	typedef unsigned char UCHAR;
	typedef signed char CHAR;
#endif
	typedef unsigned char* PUCHAR;
	typedef signed char* PCHAR;
	//wmj -
	//typedef unsigned char  BOOLEAN;

	typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
	typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
	typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
	typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
	typedef unsigned int   INT32U;                   /* Unsigned 32 bit quantity                           */
	typedef signed   int   INT32S;                   /* Signed   32 bit quantity                           */
	typedef float          FP32;                     /* Single precision floating point                    */
	typedef double         FP64;                     /* Double precision floating point                    */

	typedef unsigned int   OS_STK;                   /* Each stack entry is 32-bit wide                    */
	typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (PSR = 32 bits) */

	typedef unsigned char bool;
	#define true 1
	#define false 0
	#define TRUE 1 //TODO
	#define FALSE 0

	#define BIT(x)	(1 << (x))
	#define NETDEV_ALIGN		32
	#define ETHTOOL_BUSINFO_LEN	32

	typedef unsigned gfp_t;
	typedef unsigned fmode_t;
	
	#define false 0
	#define true  1

	#ifndef min
	#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
	#endif

	#ifndef max
	#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
	#endif


	#ifndef NULL
	#define NULL    ((void *)0)
	#endif

	#ifdef __cplusplus
}

#endif

#endif
