/**
 * @file medialib_global.h
 * @brief Define the global public types for media lib, video lib and audio lib
 *
 * Copyright (C) 2014 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Su_Dan
 * @date 2007-08-20
 * @update date 2014-11-06
 * @version 3.0
 */

#ifndef _MEDIA_LIB_GLOBAL_H_
#define _MEDIA_LIB_GLOBAL_H_

#include "anyka_types.h"
#include "anyka_types_internal.h"


typedef struct
{
	T_U16	ResourceID;
	T_U8	*Buff;
	T_U32	Resource_len;
}T_AUDIO_LOADRESOURCE_CB_PARA;

typedef T_VOID (*MEDIALIB_CALLBACK_FUN_PRINTF)(T_pCSTR format, ...);

#if 0
typedef T_S32	SEM_ID;
typedef SEM_ID (*MEDIALIB_CALLBACK_FUN_SEM_CREATE)(T_U32 nSemType, T_U32 nMaxLockCount);
typedef T_BOOL (*MEDIALIB_CALLBACK_FUN_SEM_TAKE)(SEM_ID semID, T_S32 nTimeOut);
typedef T_BOOL (*MEDIALIB_CALLBACK_FUN_SEM_GIVE)(SEM_ID semID);
typedef T_BOOL (*MEDIALIB_CALLBACK_FUN_SEM_FLUSH)(SEM_ID semID);
typedef T_BOOL (*MEDIALIB_CALLBACK_FUN_SEM_DELETE)(SEM_ID semID);
#endif

//typedef T_S32 (*MEDIALIB_CALLBACK_FUN_OPEN)(T_pVOID lpData);
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_READ)(T_S32 hFile, T_pVOID buf, T_S32 size);
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_WRITE)(T_S32 hFile, T_pVOID buf, T_S32 size);
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_SEEK)(T_S32 hFile, T_S32 offset, T_S32 whence); 
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_TELL)(T_S32 hFile);
//typedef T_VOID (*MEDIALIB_CALLBACK_FUN_CLOSE)(T_S32 hFile);

typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_MALLOC)(T_U32 size);
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_FREE)(T_pVOID mem);

typedef T_VOID (*MEDIALIB_CALLBACK_FUN_LOADRESOURCE)(T_AUDIO_LOADRESOURCE_CB_PARA *pPara);	//载入资源
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_RELEASERESOURCE)(T_U8 *Buff);	//释放载入的资源

typedef T_BOOL (*MEDIALIB_CALLBACK_FUN_RTC_DELAY) (T_U32 ulTicks);

//typedef T_VOID	(*MEDIALIB_CALLBACK_FUN_SHOW_FRAME)(T_pDATA srcImg, T_U16 src_width, T_U16 src_height);

typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_DMA_MEMCPY)(T_pVOID dest, T_pCVOID src, T_U32 size);
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_MMU_INVALIDATEDCACHE)(void);
typedef T_BOOL (*MEDIALIB_CALLBACK_FUN_CHECK_DEC_BUF)(T_pDATA pBuf);

typedef T_S32 (*MEDIALIB_CALLBACK_FUN_FILE_HANDLE_EXIST)(T_S32 hFile);
typedef T_U32 (*MEDIALIB_CALLBACK_FUN_FILE_GET_LENGTH)(T_S32 hFile);

//just for audio codec lib
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_CMMBSYNCTIME)(T_VOID *pHandle, T_U32 timestamp);
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_CMMBAUDIORECDATA)(T_VOID *pHandle, T_U8 *buf, T_S32 len);
//end of just for audio codec lib

typedef T_BOOL (*MEDIALIB_CALLBACK_FUN_FILESYS_ISBUSY)(void);

typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_DMA_MALLOC)(T_U32 size);
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_DMA_FREE)(T_pVOID mem);

typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_VADDR_TO_PADDR)(T_pVOID mem); 
typedef T_U32 (*MEDIALIB_CALLBACK_FUN_MAP_ADDR)(T_U32 phyAddr, T_U32 size); 
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_UNMAP_ADDR)(T_U32 addr, T_U32 size);

//common register operate
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_REG_BITS_WRITE)(T_U32 phyAddr, T_U32 val, T_U32 mask);

//hardware mutex
typedef T_pVOID (*MEDIALIB_CALLBACK_FUN_VIDEO_HW_LOCK)(T_S32 hw_id);
typedef T_S32 (*MEDIALIB_CALLBACK_FUN_VIDEO_HW_UNLOCK)(T_pVOID hLock);

//invalid Dcache
typedef T_VOID (*MEDIALIB_CALLBACK_FUN_INVALID_DCACHE) (T_VOID);

typedef enum
{
	MEDIALIB_ROTATE_0,
	MEDIALIB_ROTATE_90,
	MEDIALIB_ROTATE_180,
	MEDIALIB_ROTATE_270
}T_eMEDIALIB_ROTATE;

typedef enum
{
	MEDIALIB_CHIP_UNKNOWN,		//default 3223
	MEDIALIB_CHIP_AK3223,		//3220, 3221, 3223, 3224 first version
	MEDIALIB_CHIP_AK3224,		//3224 metal fix, 3225
	MEDIALIB_CHIP_AK3610,		//3610 and 3620
	MEDIALIB_CHIP_AK3810,		//3810, 7801 and 7802
	MEDIALIB_CHIP_AK3610_2,		//3631 and 3631L, 322L
	MEDIALIB_CHIP_AK8801,		//8801 and 8802
	MEDIALIB_CHIP_AK3671,		//3671, 3675, 3650...
	MEDIALIB_CHIP_AK9802,		//9801, 9802, 9805...
	MEDIALIB_CHIP_AK3751,		//3751, 3760, 3771...
	MEDIALIB_CHIP_AK3751C		//3751C, 3750C, 3760C...
}T_eMEDIALIB_CHIP_TYPE;

//chip type in detail, only for audio and video module
#define MEDIALIB_CHIP_AK3221	MEDIALIB_CHIP_AK3223
#define MEDIALIB_CHIP_AK3225	MEDIALIB_CHIP_AK3224
#define MEDIALIB_CHIP_AK3620	MEDIALIB_CHIP_AK3610
#define MEDIALIB_CHIP_AK322L	MEDIALIB_CHIP_AK3610_2
#define MEDIALIB_CHIP_AK3631	MEDIALIB_CHIP_AK3610_2
#define MEDIALIB_CHIP_AK7801	MEDIALIB_CHIP_AK3810
#define MEDIALIB_CHIP_AK7802	MEDIALIB_CHIP_AK3810
#define MEDIALIB_CHIP_AK8802	MEDIALIB_CHIP_AK8801
#define MEDIALIB_CHIP_AK9801	MEDIALIB_CHIP_AK9802
#define MEDIALIB_CHIP_AK3760	MEDIALIB_CHIP_AK3751
#define MEDIALIB_CHIP_AK3760C	MEDIALIB_CHIP_AK3751C

typedef enum 
{
	AUDIOLIB_CHIP_UNKNOW,
	AUDIOLIB_CHIP_AK10XX,
	AUDIOLIB_CHIP_AK10XXC,
	AUDIOLIB_CHIP_AK10XXL,
	AUDIOLIB_CHIP_AK10XXT,
	AUDIOLIB_CHIP_AK11XX,
	AUDIOLIB_CHIP_AK37XX,
	AUDIOLIB_CHIP_AK37XXL,
	AUDIOLIB_CHIP_AK37XXC,
	AUDIOLIB_CHIP_AK39XX,
	AUDIOLIB_CHIP_AK98XX,
	AUDIOLIB_CHIP_AK39XXE,
	AUDIOLIB_CHIP_AK10XXT2,
    AUDIOLIB_CHIP_AK39XXEV2,
}T_AUDIO_CHIP_ID;

typedef enum
{
	I2S_UNUSE,
	I2S_DA,
	I2S_AD,
	I2S_DA_AD 
}T_AUDIO_I2S_APP;

typedef struct
{
	T_eMEDIALIB_CHIP_TYPE	m_ChipType;
	T_AUDIO_I2S_APP			m_AudioI2S;
}T_MEDIALIB_INIT_INPUT;

typedef struct
{
	MEDIALIB_CALLBACK_FUN_PRINTF			m_FunPrintf;
	MEDIALIB_CALLBACK_FUN_LOADRESOURCE		m_FunLoadResource;
	MEDIALIB_CALLBACK_FUN_RELEASERESOURCE	m_FunReleaseResource;
#if 0
	MEDIALIB_CALLBACK_FUN_SEM_CREATE		m_FunSemCreate;
	MEDIALIB_CALLBACK_FUN_SEM_TAKE			m_FunSemTake;
	MEDIALIB_CALLBACK_FUN_SEM_GIVE			m_FunSemGive;
	MEDIALIB_CALLBACK_FUN_SEM_FLUSH			m_FunSemFlush;
	MEDIALIB_CALLBACK_FUN_SEM_DELETE		m_FunSemDelete;
#endif

}T_MEDIALIB_INIT_CB;

typedef struct
{
	T_U32	m_SampleRate;		//sample rate, sample per second
	T_U16	m_Channels;			//channel number
	T_U16	m_BitsPerSample;	//bits per sample 

	T_U32	m_ulSize;
	T_U32	m_ulDecDataSize;
	T_U8	*m_pBuffer;
}T_AUDIO_DECODE_OUT;

typedef struct
{
	T_U32	m_ulSize;
	T_U16	m_uDispWidth;
	T_U16	m_uDispHeight;
	T_U8	*m_pBuffer;
	T_U16	m_uOriWidth;		//原始宽，不一定是16的倍数
	T_U16	m_uOriHeight;		//原始高，不一定是16的倍数
	T_U8	*m_pBuffer_u;
	T_U8	*m_pBuffer_v;
}T_VIDEO_DECODE_OUT;

typedef struct
{
	T_U32	m_SampleRate;		//sample rate, sample per second
	T_U16	m_Channels;			//channel number
	T_U16	m_BitsPerSample;	//bits per sample 
}T_AUDIO_OUT_INFO;

typedef struct
{
	T_U16	m_OutWidth;			//output width
	T_U16	m_OutHeight;		//output height
}T_VIDEO_OUT_INFO;

typedef struct
{
	T_S32	real_time;

	union {
		struct{
            T_U32 nCurBitIndex;  //不需要用
            T_U32 nFrameIndex;    //第几帧
        } m_ape;
		struct{
			T_U32 Indx;         //seek位置开始，当前page中有的packet数
			T_U32 offset;       ///seek后，当前page剩余需要解码的数据大小
			T_BOOL flag;          //seek标志位，seek时置1
			T_U32  last_granu;   //解码完上一个page时解出的总sample数 
			T_U32  now_granu;    //解码完当前page时接出的总sample数
			T_BOOL is_eos;       //码流结束标志位
			T_U32  re_data;      //seek后，当前page含有不完整packet的大小
			T_U32 pack_no;       //seek的位置在当前page中的第几个packet
			T_U32 list[255];     //记录一个ogg page中每个packet的偏移位置
		}m_speex;
        struct{
			T_U32 Indx;         //seek位置开始，当前page中有的packet数
			T_U32 offset;       ///seek后，当前page剩余需要解码的数据大小
			T_BOOL flag;          //seek标志位，seek时置1
			T_U32  last_granu;   //解码完上一个page时解出的总sample数 
			T_U32  now_granu;    //解码完当前page时接出的总sample数
			T_BOOL is_eos;       //码流结束标志位
			T_U32  re_data;      //seek后，当前page含有不完整packet的大小
			T_U32 pack_no;       //seek的位置在当前page中的第几个packet
			T_U32 list[255];     //记录一个ogg page中每个packet的偏移位置
		}m_speexwb;
		struct{
			T_U8	secUse;		//已经读取的section数目
			T_U8	secLen;		//一个page中包含的section数
			T_U8	tmpSec;		//已经解码的section数目
			T_BOOL	is_eos;		//是不是最后一个page
			T_BOOL	is_bos;		//是不是第一个page
			T_U8	endpack;	//当前page中最后一个packet的位置
			//解码出的sample数是一个64位的数，目前只取低32位
			T_U32	gos;		//解码完当前page后解码出总的sample数，低32位
			T_U32	high_gos;	//解码完当前page后解码出总的sample数，高32位，(暂时不用，留给以后需要)
			T_U8	list[255];	//记录一个page中每个section的大小，一个page中最多含有255个section
		}m_vorbis;
	}m_Private;
}T_AUDIO_SEEK_INFO;

#endif//_MEDIA_LIB_GLOBAL_H_
