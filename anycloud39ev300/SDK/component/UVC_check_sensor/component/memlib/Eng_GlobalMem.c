/************************************************************************************
* Copyright(c) 2006 Anyka.com
* All rights reserved.
*
* File	:	eng_globalMem.c
* Brief :	heap memory allocator
*           
* 
* Version : 1.0
* Author  : ZhangMuJun
* Modify  : 
* Data    : 2006-06-24
*************************************************************************************/
#include "mem_api.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//lint -e818 -e801 -e737 -e713 -e527
//lint -e826 -e701 -e613 -e573 -e668
//lint -e834 -e732 -e539 -e502


/**
 * @brief  header bit定义
 */
typedef union tagBlockNum_BIT
{
	struct
	{
		T_U32	magic	:4;
		T_U32	old		:1;
		T_U32   resv	:1;
		T_U32	size	:26;
	}bit;
	T_U32		var;

}BlockNum_BIT;

typedef union tagPrevBlock_BIT
{
	struct
	{
		T_U32	pad		:6;
		T_U32	idx		:26;
	}bit;
	T_U32		var;

}PrevBlock_BIT;

typedef union tagExtInfo_BIT
{
	struct  
	{
		T_U16	pad		:14;
		T_U16	resv	:2;
	}bit;
	T_U16		var;
}ExtInfo_BIT;

/**
 * @brief  全局内存存储块头的定义
 */
typedef union tagRam_Bubble
{
	struct
	{
		union
		{
			BlockNum_BIT	BlockNum;
			union tagRam_Bubble *next;
		}a;
		union
		{
			PrevBlock_BIT	PrevBlock;	
			union tagRam_Bubble *prev;
		}b;

		T_S8	*FileName;
		T_U16	FileLine;

		ExtInfo_BIT	ext;

	}hdr;//block header info
	union
	{
		T_U8		byt;
		T_U16		srt;
		T_U32		lng;
		T_VOID		*vod;
		T_VOID		(*func)(T_VOID);
		struct
		{
			T_U32   lparm;
			T_U32   rparm;
		}prm;
	}pad;//auto padding for CPU align

}Ram_Bubble, Ram_Hdr;

/**
 * @brief  全局堆内存句柄定义
 */
typedef struct tagGLOBALMEMHandle
{
	struct
	{
		T_U32	magic_l;
		T_U32	magic_r;
		Ram_Hdr	*lpAddr;		//start alloc address
		T_U32	szBloks;		//number of blocks
		T_pDATA lpStart;		//heap start address
		T_U32   length;			//heap memory length
	}heap;						//heap memory info
	struct
	{
		T_U32	sos;			//临界区位置
		T_U32	lenFree;		//小内存释放数组大小
		T_U8	align;			//对齐基数(同时也是单位块大小) : 2^x
		T_U8	split;			//泡泡产生分裂的临界落差
		T_U8	lenHdr;			//内部产生(为减少计算速度): 泡泡头的长度(以块为单位)
		T_U16	llen;			//内部产生(为减少计算速度): 左边界长
		T_U16   rlen;			//内部产生(为减少计算速度): 右边界长
		T_U16	alen;			//内部产生(为减少计算速度): 对齐长
		Ram_Hdr *lpTail;		//内部产生(为减少计算速度): 内存尾
	}setting;
	struct
	{
		T_U32   n_nSpare;		//当前正空闲内存泡泡数
		T_U32	n_szUsed;		//当前正使用内存大小
		T_U32	n_nUsed;		//当前正使用内存泡泡数
		T_U32	ln_miscs;		//左大内存链节点数
		T_U32	rn_miscs;		//右大内存链节点数
		T_U32	n_blocks;		//总可用块个数
	}stat;						//堆内存统计信息
	struct
	{
		T_U32	logCnt;
		T_BOOL	bAutoTrace;		//auto trace memory leak
		T_S32	curStack;
		Ram_EnterStateMachine	enterSection;
		Ram_LeaveStateMachine	leaveSection;
		Ram_EnumMemTraceFun enumFunc;
		Ram_PrintfTraceFun  printfN;
		struct 
		{
			T_U8 *s_addr;
			T_U8 *s_next;
		}*mem;
	}trace;
	struct
	{
		T_U32 dwLock;			//thread lock	
	}sys;
	struct
	{
		T_U8    ncheck;			//check type
		T_U16	llenpad;		//left beyond check range
		T_U8	lvar;			//left beyond presetting value
		T_U16	rlenpad;		//right beyond check range
		T_U8	rvar;			//right beyond presetting value
	}beyond;					//beyond map info
	struct  
	{
		T_U8	ncheck;			//check type
		T_U8	wlenpad;
		T_U8	wvar;
	}wilder;
	struct
	{
		T_U32	largePtr;		//大内存分配后向指针
		Ram_Hdr *critical;		//右半区域临界块 : 紧邻largePtr
		Ram_Hdr *misc;			//大内存释放链表(右半域)
	}aback;						//右后退分配
	struct
	{
		T_U32	allocPtr;		//小内存分配前向指针
		Ram_Hdr *critical;		//左半区域临界块 : 紧邻allocPtr
		T_U32	stepSearch[16]; //小内存释放链搜索深度表
		Ram_Hdr *misc;			//大内存释放链表(左半域)
		T_U8	*bitmap;		//小内存释放链表阵列位图 SIZE=lenFree/8 
		T_U32	*bigmap;		//for quick search SIZE=lenfFree/256
		Ram_Hdr **list;			//小内存释放链表阵列 : SIZE=lenFree
	}ahead;						//左前进分配

}GLOBALMEMHandle;

/**
 * @brief  内存区域枚举定义
 */
typedef enum tagREGION
{
	REGION_SMALL_LEFT	= 0,	//小内存分配指针左半区域
	REGION_LARGE_RIGHT	= 1		//大内存分配指针右半区域
}REGION;



/**
 * @brief  相关配置定义
 */
#define DEFAULT_SIZE_ALIGN			16			//块对齐大小
#define DEFAULT_SIZE_FREELNK		6400		//小内存释放阵列大小
#define DEFAULT_LEN_LEFTPAD			0			//左内存临界检测区长度
#define DEFAULT_LEN_RIGHTPAD		0			//右内存临界检测区长度
#define DEFAULT_WILDER_PADCHK		0
#define DEFAULT_BEYOND_PADCHK		0
#define DEFAULT_VAR_LEFTPAD			0xFC		//左内存临界检测区预设值
#define DEFAULT_VAR_RIGHTPAD		0xCF		//右内存临界检测区预设值
#define DEFAULT_VAR_WILDPAD			0xCC		//未使用内存预填充值
#define DEFAULT_RATIO_SOS			50			//大内存最大组合占总内存比例
#define DEFAULT_SPLIT_INTVL			4			//最小分裂块单位
#define DEFAULT_AUTOTRACE_STACK		20			//内存泄漏自动跟踪最大栈深
#define DEFAULT_AUTOTRACE_COUNT		2000		//内存泄漏自动跟踪最大数目

/**
 * @brief  index table
 */
const T_U8 mask_bit[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x1};	
const T_U8 tran_bit[8] = {0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe};	
const T_U8 trla_bit[8] = {0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x0};

/**
 * @brief  fixed macro
 */
#define ALIGN_By_BASE(var, base)	(((T_U32)(var)+(base)-1) &~ ((base)-1)) //Note base = 2^n
#define ALIGN_By_BACK(var, base)	(ALIGN_By_BASE(var, base)-(base))

#define MAGIC_NUM_HDR				0xA
#define LEN_EXTRA_BLOCK(hMem)		((hMem)->setting.lenHdr)
#define INVALID_SIZE_NUMBER			0x03FFFFFF	//<26>
#define INVALID_EXTPAD_NUMBER		0x4000		//<14>	

#define INCR_HDR(hMem, ptr, offset)	((T_U8*)(ptr)+((offset)<<((hMem)->setting.align)))
#define DECR_HDR(hMem, ptr, offset)	((T_U8*)(ptr)-((offset)<<((hMem)->setting.align)))

/**
 * @brief  TRACE输出消息
 */
static T_VOID Ram_TestErrInfo(Ram_Hdr *cur);
static T_S32  printfN(const char *format, ...) {return 0;/*lint -e(715)*/}	
#define RAM_PRINTF(hMem)			(*(hMem)->trace.printfN)

//T_S32 dbgMem[20000];
//T_S32 dbgCnt=0;


/**
 * @brief  inline function for leap stack operation
 */

//=========== bit process
__inline T_VOID		SET_BIT(GLOBALMEMHandle *hMem, T_U32 id)
{
	hMem->ahead.bigmap[id >> 8] ++; 
	hMem->ahead.bitmap[id >> 3] |= mask_bit[id%8];
}

__inline T_VOID		CLR_BIT(GLOBALMEMHandle *hMem, T_U32 id)
{
	hMem->ahead.bigmap[id >> 8] --; 
	hMem->ahead.bitmap[id >> 3] &= tran_bit[id%8];
}

//=========== hdr<>ptr<>idx convert
__inline Ram_Hdr*   PTR_TO_HDR(GLOBALMEMHandle *hMem, T_U8 *ptr)
{
	T_U32 offset = hMem->beyond.llenpad+hMem->wilder.wlenpad+LEN_EXTRA_BLOCK(hMem);
	return (Ram_Hdr*)(ptr-(offset<<hMem->setting.align));
}

__inline T_U8*		HDR_TO_PTR(GLOBALMEMHandle *hMem, Ram_Hdr *hdr)
{
	T_U32 offset = hMem->beyond.llenpad+hMem->wilder.wlenpad+LEN_EXTRA_BLOCK(hMem);
	return (T_U8*)hdr+(offset<<hMem->setting.align);
}

__inline T_U32		PTR_TO_BLK(GLOBALMEMHandle *hMem, T_U8 *ptr)	
{
	return (((T_U32)(ptr-(T_U8*)hMem->heap.lpAddr)) >> (hMem->setting.align));
}

__inline T_U8*		BLK_TO_PTR(GLOBALMEMHandle *hMem, T_U32 idx)	
{
	return (T_U8*)hMem->heap.lpAddr+(idx<<hMem->setting.align);
}

//=========== hdr bit rule
__inline T_VOID		MARK_HDR_PHY_NEXT(Ram_Hdr *hdr, T_U32 size, T_U8 old)
{
	hdr->hdr.a.BlockNum.bit.magic = MAGIC_NUM_HDR;
	hdr->hdr.a.BlockNum.bit.size  = (size & INVALID_SIZE_NUMBER);
	hdr->hdr.a.BlockNum.bit.old   = (old & 0x1);
}
#define  GET_MEM_SIZE(memhdr)		((memhdr)->hdr.a.BlockNum.bit.size)
#define  GET_MEM_OLD(memhdr)		((memhdr)->hdr.a.BlockNum.bit.old)
#define  GET_MEM_MAGIC(memhdr)		((memhdr)->hdr.a.BlockNum.bit.magic)

__inline T_VOID		MARK_HDR_PHY_PREV(Ram_Hdr *hdr, T_U32 idx, T_U8 pad)
{
	hdr->hdr.b.PrevBlock.bit.idx  = (idx & INVALID_SIZE_NUMBER);
	hdr->hdr.b.PrevBlock.bit.pad  = (pad & 0x3F);
}
#define  GET_MEM_IDX(memhdr)		((memhdr)->hdr.b.PrevBlock.bit.idx)
#define  GET_MEM_PAD(memhdr)		((memhdr)->hdr.b.PrevBlock.bit.pad)
#define	 HDR_MEM_EXTPAD(ptr)		((ptr)->hdr.ext.bit.pad)


//=========== hdr physical and logic define
__inline Ram_Hdr*	HDR_PHY_NEXT(GLOBALMEMHandle *hMem, Ram_Hdr *ptr)
{
	T_U32 offset = LEN_EXTRA_BLOCK(hMem)+hMem->beyond.llenpad+hMem->wilder.wlenpad+GET_MEM_SIZE(ptr)+hMem->beyond.rlenpad;
	return (Ram_Hdr*)((T_U8*)ptr+(offset<<hMem->setting.align));
}

__inline Ram_Hdr*	HDR_PHY_PREV(GLOBALMEMHandle *hMem, Ram_Hdr *ptr)
{
	T_U32 offset = GET_MEM_IDX(ptr);
	return (Ram_Hdr*)((T_U8*)hMem->heap.lpAddr+(offset<<hMem->setting.align));
}

#define  HDR_LOG_NEXT(ptr)			(((Ram_Hdr*)(ptr))->hdr.a.next)
#define  HDR_LOG_PREV(ptr)			(((Ram_Hdr*)(ptr))->hdr.b.prev)

#define  HDR_LOG_TO_PHY(hMem, ptr)	((Ram_Hdr*)PTR_TO_HDR((hMem), ((T_U8*)ptr)))
#define  HDR_PHY_TO_LOG(hMem, ptr)  ((Ram_Hdr*)HDR_TO_PTR((hMem), (ptr)))

__inline T_VOID		MARK_WILDER_LOG(GLOBALMEMHandle *hMem, Ram_Hdr *ptr2)
{
	if(0 != hMem->wilder.wlenpad)
	{
		Ram_Hdr *ptr = HDR_LOG_TO_PHY(hMem, ptr2);		
		Ram_Hdr *wildrHdr = (Ram_Hdr*)INCR_HDR(hMem, ptr, LEN_EXTRA_BLOCK(hMem));
		
		HDR_LOG_NEXT(wildrHdr) = HDR_LOG_NEXT(ptr2);
		HDR_LOG_PREV(wildrHdr) = HDR_LOG_PREV(ptr2);
	}
}
	
//=========== moeroy lib magic identify
#define MAGIC_MEMLIB_LEFT	0x10234576
#define MAGIC_MEMLIB_RIGHT	0x98ABCDFE

__inline T_BOOL	CHECK_MEM_LIB(GLOBALMEMHandle *hMem, T_U16 LLD)
{
	if((AK_NULL==hMem) 
		|| (MAGIC_MEMLIB_LEFT!=hMem->heap.magic_l) 
		|| (MAGIC_MEMLIB_RIGHT!=hMem->heap.magic_r))
	{
		//printf("###### Error: LLD=%d MemoryLib not initialize or Invalid Memory handle ######\r\n", LLD);
		return AK_FALSE;
	}

	return AK_TRUE;
}



/**
 * @brief  Initialize Global heap memory allocator
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_U8  *ptr :	global heap memory start adress
 * @param	T_U32 size :	global heap memory length
 * @param	T_U16 llenpad : 前边界设置长度 (用于检测内存前越界) (默认为0)
 * @param	T_U16 rlenpad : 后边界设置长度 (用于检测内存后越界) (默认为0)
 * @param	T_U8  lvar :	前边界预填充值 (用于检测内存前越界)	(默认为0x27)
 * @param	T_U8  rvar :	后边界预填充值 (用于检测内存后越界)	(默认为0x75)
 * @param	T_U8 align :	内存对齐基数   (8X)					(默认为16)
 * @param	T_U32 lenfree : 释放阵列的长度 (32X)				(默认为6400) [SIZE=6400*16/1024=100k : SIZE范围内从左到右分配, SIZE范围外从右到左分配]
 * @param	T_U8  sos :		最大大内存组合的可能所占总内存比例  (默认为50) [用于自适应调节搜索速度和碎片利用度 : 非强制因素]
 * @param   T_U8 split :    泡泡产生分裂的最小块间隔			(默认为4) [用于减少小碎片的数目]
 * @param   Ram_PrintfTraceFun lpPrintf : 是否打印信息			(默认为AK_NULL, 不打印)
 *
 * @return  ram handle for success;else for AK_NULL
 *
 */
T_GLOBALMEMHandle	Ram_Initial(T_U8 *ptr, T_U32 size)
{
	return Ram_InitialEx(ptr, size, 
						 DEFAULT_BEYOND_PADCHK, DEFAULT_LEN_LEFTPAD, DEFAULT_LEN_RIGHTPAD, DEFAULT_VAR_LEFTPAD, DEFAULT_VAR_RIGHTPAD, 
						 DEFAULT_WILDER_PADCHK, DEFAULT_VAR_WILDPAD,
						 DEFAULT_SIZE_ALIGN, DEFAULT_SIZE_FREELNK,  DEFAULT_RATIO_SOS, DEFAULT_SPLIT_INTVL,
						 AK_NULL);
}

T_GLOBALMEMHandle	Ram_InitialEx(T_U8  *ptr,     T_U32 size,											//堆参数
								  T_U8  padchk,   T_U16 llenpad,  T_U16 rlenpad, T_U8 lvar, T_U8 rvar,  //越界检测参数
								  T_U8  wildchk,  T_U8 wvar,											//野操作检测参数
								  T_U8  align,    T_U32 lenfree, T_U8 sos, T_U8 split,					//高级分配器优化参数
								  Ram_PrintfTraceFun lpPrintf)
{
	T_U32 i;
	T_U32 len;
	T_U16 base;
	T_U8  *addr = ptr;
	GLOBALMEMHandle *hMem;
	const T_U16 g_align2X[4] = {8, 16, 32, 64};

	if (AK_NULL !=lpPrintf )
		lpPrintf("###### Global Memory Allocator Lib : Version = %s ###### \r\n", MEMLIB_VERSION);
//memset(dbgMem, 0, sizeof(dbgMem));
//dbgMem[0]=(T_U32)ptr;
//dbgMem[1]=size;
//dbgCnt=2;


	for(i=0; (i<4)&&(g_align2X[i]<align); i++)/*lint -e(722)*/;
	if(i >= 4)
	{
		if (AK_NULL !=lpPrintf )
			lpPrintf("Ram_InitialEx Error : you're crazy to align over 64 \r\n");
		return AK_NULL;
	}
	base = g_align2X[i];

	//align by HDR Len and 4X
	align = (T_U8)ALIGN_By_BASE(align, base);
	//align by U32 bitmap : U32=32!!!!!
	lenfree = ALIGN_By_BASE(lenfree, 32);
	//align handler
	ptr = (T_U8*)ALIGN_By_BASE(ptr, align);
	//align other
	llenpad = (unsigned short)ALIGN_By_BASE(llenpad, align);
	rlenpad = (unsigned short)ALIGN_By_BASE(rlenpad, align);

	if(sos == 0)		    sos = 1;
	else if(sos > 100)	sos = 100;

	//global heap memory handler length
	len = sizeof(*hMem)+sizeof(Ram_Hdr*)*lenfree+lenfree/8+64*align;

	//check input buffer size
	if((AK_NULL == ptr) 
		|| (size < len))
	{
		if (AK_NULL !=lpPrintf )
			lpPrintf("Ram_InitialEx Error : null ptr or size<%d \r\n", len);
		return AK_FALSE;
	}

	if(size/align >= INVALID_SIZE_NUMBER)
	{
		if (AK_NULL !=lpPrintf )
			lpPrintf("Ram_InitialEx Error : too huge heap size to control (>=0x%x) \r\n", size);
		return AK_FALSE;
	}

	if(lenfree >= INVALID_EXTPAD_NUMBER)
	{
		if (AK_NULL !=lpPrintf )
			lpPrintf("Ram_InitialEx Error : too huge lenfree<bigmem> to requair (>=0x%x) \r\n", lenfree);
		return AK_FALSE;
	}

	//initialize memory handler	
	hMem = (GLOBALMEMHandle*)ptr;

	//beyond
	hMem->beyond.llenpad = llenpad/align;
	hMem->beyond.lvar = lvar;
	hMem->beyond.rlenpad = rlenpad/align;
	hMem->beyond.rvar = rvar;
	hMem->beyond.ncheck = padchk;

	//wilder
	hMem->wilder.ncheck = wildchk;
	hMem->wilder.wlenpad = 0;
	hMem->wilder.wvar = wvar;

	ptr += sizeof(*hMem);
	ptr = (T_U8*)ALIGN_By_BASE(ptr, 32);

	//ahead
	hMem->ahead.bitmap = (T_U8*)ptr;
	ptr += lenfree/8;
	ptr = (T_U8*)ALIGN_By_BASE(ptr+32, sizeof(T_U32*));
	hMem->ahead.bigmap = (T_U32*)ptr;
	ptr += (lenfree/256+1)*sizeof(T_U32);
	ptr = (T_U8*)ALIGN_By_BASE(ptr+32, sizeof(Ram_Hdr*));
	hMem->ahead.list = (Ram_Hdr**)ptr;
	ptr += lenfree*sizeof(Ram_Hdr*);
	hMem->ahead.misc = AK_NULL;
	hMem->ahead.allocPtr = 0;
	hMem->ahead.critical = AK_NULL;

	for(i=0; i<lenfree; i++)	hMem->ahead.list[i] = AK_NULL;
	for(i=0; i<lenfree/8; i++)	hMem->ahead.bitmap[i] = 0;
	for(i=0; i<lenfree/256+1; i++) hMem->ahead.bigmap[i] = 0;

	for(i=0; i<16; i++)
	{
		//以2的级数次冥扩张
		hMem->ahead.stepSearch[i] = ((2<<i)*lenfree)/6400;

		if(hMem->ahead.stepSearch[i] < 4)
			hMem->ahead.stepSearch[i] = 4;
		if(hMem->ahead.stepSearch[i] > lenfree)
			hMem->ahead.stepSearch[i] = lenfree;
	}


	ptr = (T_U8*)ALIGN_By_BASE(ptr, align);

	//heap
	hMem->heap.lpStart = addr;
	hMem->heap.length = size;
	hMem->heap.lpAddr = (Ram_Hdr*)ptr;
	hMem->heap.szBloks = (T_U32)((addr+size-ptr)/align-1);

	//setting
	hMem->setting.align = align;
	hMem->setting.sos = (hMem->heap.szBloks*sos)/100;
	hMem->setting.split = split;
	hMem->setting.lenFree = lenfree;
	hMem->setting.lenHdr = (T_U8)(ALIGN_By_BASE(sizeof(Ram_Hdr), align)/align);
	hMem->setting.alen = align;
	hMem->setting.llen = llenpad;
	hMem->setting.rlen = rlenpad;
	hMem->setting.lpTail = (Ram_Hdr*)((T_U8*)hMem->heap.lpAddr+(hMem->heap.szBloks*hMem->setting.align));
	
	for(i=0; hMem->setting.align!=0; hMem->setting.align>>=1, i++)/*lint -e(722)*/;
	hMem->setting.align = (T_U8)(i-1);

	//aback
	hMem->aback.misc = AK_NULL;
	hMem->aback.largePtr = hMem->heap.szBloks;
	hMem->aback.critical = AK_NULL;

	//stat
	hMem->stat.n_blocks = hMem->heap.szBloks;
	hMem->stat.n_szUsed = 0;
	hMem->stat.n_nUsed = 0;
	hMem->stat.n_nSpare = 0;
	hMem->stat.ln_miscs = 0;
	hMem->stat.rn_miscs = 0;

	//trace
	hMem->trace.bAutoTrace = AK_FALSE;
	hMem->trace.curStack = DEFAULT_AUTOTRACE_STACK;
	hMem->trace.mem = AK_NULL;
	hMem->trace.enumFunc = AK_NULL;
	hMem->trace.enterSection = AK_NULL;
	hMem->trace.leaveSection = AK_NULL;
	hMem->trace.logCnt = DEFAULT_AUTOTRACE_COUNT+DEFAULT_AUTOTRACE_STACK;

	//wildchk
	if(0 != wildchk)
	{
		hMem->wilder.wlenpad = (T_U8)(ALIGN_By_BASE(sizeof(Ram_Hdr), align)/align);

		if(hMem->wilder.ncheck & 0x01)
			memset(hMem->heap.lpAddr, hMem->wilder.wvar, hMem->heap.szBloks<<hMem->setting.align);
	}

	//OK
	hMem->sys.dwLock = 0;
	hMem->trace.printfN = (lpPrintf ? lpPrintf : printfN);
	hMem->heap.magic_l = MAGIC_MEMLIB_LEFT;
	hMem->heap.magic_r = MAGIC_MEMLIB_RIGHT;
	
	RAM_PRINTF(hMem)("Ram_InitialEx : hMemory=0x%x heap=0x%x; szBlocks=%d; offset=%d; align=%d; maxfree=%d; sos=%d; split=%d spare=%d hMem=0x%x \r\n", hMem, hMem->heap.lpAddr, hMem->heap.szBloks, ptr-addr, align, lenfree*align, sos, split, hMem->heap.szBloks*align, hMem);
	
	return hMem;
}

/**
 * @brief  Destroy Global heap memory allocator
 * @brief  该内存库可以实例化
 * @
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 *
 * @return  AK_NULL
 *
 */
T_GLOBALMEMHandle	Ram_Exit(T_GLOBALMEMHandle hMemory)
{
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;
	
	if(!CHECK_MEM_LIB(hMem, 0))
		return AK_NULL;

	RAM_PRINTF(hMem)("###### Global Memory Allocator Lib : thank you for using ###### \r\n");

	memset(hMem, 0, sizeof(GLOBALMEMHandle));

	return AK_NULL;
	//lint -e(715)
}

/**
 * @brief  搜索前导1 : 确保sizeof(T_U32)==4 !!!!
 */
static T_U32	get_block_index(GLOBALMEMHandle *hMem, T_U32 sz, T_U8 step)
{
	T_U32 i, j;
	T_U32 *u32;
	T_U32 ualign;
	T_U32 byt;
	T_U32 ret;
	T_U32 depth;
	T_U8  *bit, *bt, bit0;

	/*lint -e(618)*/
	const static T_S8 gb_idxTable[256] = 
	{
		-1, 
		7, 
		6, 6, 
		5, 5, 5, 5, 
		4, 4, 4, 4, 4, 4, 4, 4, 
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	if(sz >= hMem->setting.lenFree)
		return (T_U32)-1;

	//direct hit
	if(hMem->ahead.list[sz] != AK_NULL)
		return sz;

	//adjust search depth by dynamic allocPtr location
	depth = hMem->ahead.stepSearch[step];
	if(depth+sz >= hMem->setting.lenFree)
		depth = hMem->setting.lenFree-1-sz;

/*
	for(i=sz; i<depth; i++)
	{
		if(hMem->ahead.list[i] != AK_NULL)
			return i;
	}
*/

	byt = (depth>>3)+1;

	bit = &hMem->ahead.bitmap[sz >> 3];
	bit0 = bit[0];
	bit[0] &= trla_bit[sz%8];

	ret = (T_U32)-1;

	//align before
	//ualign = 32-((T_U32)bit & 15);
	ualign = (T_U32)bit & 31;
	if(ualign)
		ualign = 32-ualign;

	if(ualign > byt)
	{
		ualign = byt;
		ret = (T_U32)-2;
	}
	for(i=0; (i<ualign)&&(0==bit[i]); i++)/*lint -e(722)*/;
	if(i < ualign)
	{
		//find the right
		j = /*lint -e(571)*/(T_U32)gb_idxTable[bit[i]];
		//for(j=0; (j<8)&&(0==(bit[i] & mask_bit[j])); j++);
		ret = (((bit+i)-hMem->ahead.bitmap)<<3) + j;
	}

	byt -= ualign;
	
	//align after
	if(ret == (T_U32)-1)
	{
		T_U32 offset = bit+ualign-hMem->ahead.bitmap;

		u32 = (T_U32*)(bit+ualign);
		byt = (byt>>2)+1;

		for(i=0; (i<byt)&&(0==u32[i]); )
		{
			if(0 == hMem->ahead.bigmap[(offset+(i<<2))>>5])
				i += 8;
			else
				i += 1;
		}
		//for(i=0; (i<byt)&&(0==u32[i]); i++);
		if(i < byt)
		{
			//find the right
			bt = (T_U8*)&u32[i];
			for(i=0; (i<4)&&(0==bt[i]); i++)/*lint -e(722)*/;
			j = /*lint -e(571)*/(T_U32)gb_idxTable[bt[i]];
			//for(j=0; (j<8)&&(0==(bt[i] & mask_bit[j])); j++);
			ret = (((bt+i)-hMem->ahead.bitmap)<<3) + j;
		}
	}

	if(ret == (T_U32)-2)
		ret = (T_U32)-1;

	if((ret != (T_U32)-1) && (ret != (T_U32)-2))
	{
		if(ret-sz > depth)
			ret = (T_U32)-1;
	}

	bit[0] = bit0;

	if(hMem->ahead.list[ret] == AK_NULL)
		ret = (T_U32)-1;

	return ret;
}


/**
 * @brief  摘除半区域内存释放节点
 */
static T_VOID	delete_node(GLOBALMEMHandle *hMem, Ram_Hdr *ptr, REGION regn)
{
	T_U32   origID;
	Ram_Hdr *ptr2;
	Ram_Hdr *next2, *prev2, **hdr= AK_NULL;

	origID = GET_MEM_SIZE(ptr);
	ptr2   = HDR_PHY_TO_LOG(hMem, ptr);

	//HEAD
	if(origID < hMem->setting.lenFree)
		hdr = &hMem->ahead.list[origID];
	else if(REGION_SMALL_LEFT == regn)
		hdr = &hMem->ahead.misc;
	else 
		hdr = &hMem->aback.misc;

	prev2 = HDR_LOG_PREV(ptr2);
	next2 = HDR_LOG_NEXT(ptr2);

	if(ptr2 == *hdr)
	{
		//delete head
		*hdr = HDR_LOG_NEXT(ptr2);
		if(next2)
		{
			HDR_LOG_PREV(next2) = AK_NULL;
			MARK_WILDER_LOG(hMem, next2);
		}
	}
	else
	{
		//delete node not head
		if(prev2) 
		{
			HDR_LOG_NEXT(prev2) = next2;
			MARK_WILDER_LOG(hMem, prev2);
		}
		if(next2) 
		{
			HDR_LOG_PREV(next2) = prev2;
			MARK_WILDER_LOG(hMem, next2);
		}	
	}

	if((origID < hMem->setting.lenFree)
		&& (AK_NULL == hMem->ahead.list[origID]))
		CLR_BIT(hMem, origID);

	hMem->stat.n_nSpare --;
}

/**
 * @brief  插入半区域内存释放节点
 */
static T_VOID	insert_node(GLOBALMEMHandle *hMem, Ram_Hdr *ptr, REGION regn)
{
	T_U32   origID;
	Ram_Hdr *ptr2;

	origID = GET_MEM_SIZE(ptr);
	ptr2   = HDR_PHY_TO_LOG(hMem, ptr);

	//push into Free List or Misc List
	if(origID < hMem->setting.lenFree)
	{
		HDR_LOG_PREV(ptr2) = AK_NULL;
		HDR_LOG_NEXT(ptr2) = hMem->ahead.list[origID];
		if(hMem->ahead.list[origID]) 
		{
			HDR_LOG_PREV(hMem->ahead.list[origID]) = ptr2;
			MARK_WILDER_LOG(hMem, hMem->ahead.list[origID]);
		}
		hMem->ahead.list[origID] = ptr2;

		SET_BIT(hMem, origID);
	}
	else
	{
		Ram_Hdr *prev2;
		Ram_Hdr *mptr, **mhdr = AK_NULL;

		if(REGION_SMALL_LEFT == regn)
			mhdr = &hMem->ahead.misc;
		else
			mhdr = &hMem->aback.misc;

		//look the right location for insert
		for(mptr=*mhdr; mptr && HDR_LOG_NEXT(mptr) && (GET_MEM_SIZE(HDR_LOG_TO_PHY(hMem, mptr))<origID); mptr=HDR_LOG_NEXT(mptr))/*lint -e(722)*/;
		if(mptr == AK_NULL)
		{
			HDR_LOG_PREV(ptr2) = AK_NULL;
			HDR_LOG_NEXT(ptr2) = *mhdr;
			if(*mhdr) 
			{
				HDR_LOG_PREV(*mhdr) = ptr2;
				MARK_WILDER_LOG(hMem, *mhdr);
			}

			*mhdr = ptr2;
		}
		else
		{
			if(GET_MEM_SIZE(HDR_LOG_TO_PHY(hMem, mptr))<origID)
			{
				HDR_LOG_NEXT(mptr) = ptr2;
				MARK_WILDER_LOG(hMem, mptr);

				HDR_LOG_PREV(ptr2) = mptr;
				HDR_LOG_NEXT(ptr2) = AK_NULL;
			}
			else
			{
				prev2 = HDR_LOG_PREV(mptr);
			
				HDR_LOG_PREV(ptr2) = prev2;
				HDR_LOG_NEXT(ptr2) = mptr;
				if(prev2) 
				{
					HDR_LOG_NEXT(prev2) = ptr2;
					MARK_WILDER_LOG(hMem, prev2);
				}

				/*if(*mptr)*/ HDR_LOG_PREV(mptr) = ptr2;
				MARK_WILDER_LOG(hMem, mptr);

				if(mptr == *mhdr)
					*mhdr = ptr2;
			}
		}
	}

	hMem->stat.n_nSpare ++;

	if(0 != hMem->wilder.wlenpad)
	{
		T_U8 *bit;
		T_S32 sz;
		Ram_Hdr *wildrHdr = (Ram_Hdr*)INCR_HDR(hMem, ptr, LEN_EXTRA_BLOCK(hMem));

		HDR_LOG_NEXT(wildrHdr) = HDR_LOG_NEXT(ptr2);
		HDR_LOG_PREV(wildrHdr) = HDR_LOG_PREV(ptr2);

		if(hMem->wilder.ncheck & 0x01)
		{
			sz = sizeof(HDR_LOG_NEXT(ptr2))+sizeof(HDR_LOG_PREV(ptr2));
			bit = (T_U8*)ptr2+sz;
			sz = (origID<<hMem->setting.align)-sz-GET_MEM_PAD(ptr);
			if(sz > 0)
				memset(bit, hMem->wilder.wvar, sz);
		}
    }
}

/**
 * @brief  分裂半区域的泡泡
 */
static T_VOID	split_block(GLOBALMEMHandle *hMem, Ram_Hdr *ptr, T_U32 targID, REGION regn, T_U32 pad)
{
	T_U8	*bit;
	T_U32	split;
	T_U32   origID;
	T_U16	szExtra;

	origID = GET_MEM_SIZE(ptr);
	szExtra = hMem->beyond.llenpad+hMem->wilder.wlenpad+hMem->beyond.rlenpad+LEN_EXTRA_BLOCK(hMem);

	if(origID <= targID)
		goto MARK;

	//get split factor
	split = (REGION_SMALL_LEFT == regn) ? hMem->setting.split : hMem->setting.lenFree;
	if(targID<hMem->setting.lenFree && REGION_LARGE_RIGHT==regn)
		split = hMem->setting.split;

	//K <= split NOT TO SPLIT
	if(origID-targID > (T_U32)(split+szExtra))
	{
		Ram_Hdr *next1;
		Ram_Hdr *next = (Ram_Hdr*)INCR_HDR(hMem, ptr, szExtra+targID);
		
		MARK_HDR_PHY_NEXT(next, origID-szExtra-targID, 1);	
		MARK_HDR_PHY_PREV(next, PTR_TO_BLK(hMem, (T_U8*)ptr), 0);

		insert_node(hMem, next, regn);
		hMem->stat.n_blocks -= szExtra;

		//
		next1 = AK_NULL;
		if(GET_MEM_SIZE(ptr) != INVALID_SIZE_NUMBER) next1 = (Ram_Hdr*)HDR_PHY_NEXT(hMem, ptr);
		if(next1 >= hMem->setting.lpTail) next1 = AK_NULL;
		if((REGION_SMALL_LEFT == regn) && (next1 > hMem->ahead.critical))
			next1 = AK_NULL;
		if(AK_NULL != next1)
			MARK_HDR_PHY_PREV(next1, PTR_TO_BLK(hMem, (T_U8*)next), 0);

		origID = targID;
	}

MARK:
	//validate bubble
	MARK_HDR_PHY_NEXT(ptr, origID, 0);
	MARK_HDR_PHY_PREV(ptr, GET_MEM_IDX(ptr), (T_U8)pad);

	HDR_MEM_EXTPAD(ptr)	= (T_U16)(origID-targID);

	if(0 != hMem->beyond.llenpad)
	{
		bit = (T_U8*)INCR_HDR(hMem, ptr, LEN_EXTRA_BLOCK(hMem)+hMem->wilder.wlenpad);
		memset(bit, hMem->beyond.lvar, hMem->setting.llen);
	}

	if(0 != hMem->beyond.rlenpad)
	{
		bit = (T_U8*)INCR_HDR(hMem, ptr, LEN_EXTRA_BLOCK(hMem)+hMem->wilder.wlenpad+hMem->beyond.llenpad+origID);
		pad += (origID-targID)<<hMem->setting.align;
		memset(bit-pad, hMem->beyond.rvar, hMem->setting.rlen+pad);
	}
	
	if(0 != hMem->wilder.wlenpad)
	{
		Ram_Hdr *wildrHdr = (Ram_Hdr*)INCR_HDR(hMem, ptr, LEN_EXTRA_BLOCK(hMem));
		HDR_LOG_NEXT(wildrHdr) = (Ram_Hdr*)0xFFFFFFFF;
		HDR_LOG_PREV(wildrHdr) = (Ram_Hdr*)0xFFFFFFFF;
	}


	return ;
}

/**
 * @brief  从堆区域分配泡泡
 */
static Ram_Hdr*	split_bubble(GLOBALMEMHandle *hMem, T_U32 targID, REGION regn, T_U32 pad)
{
	//T_U16   i;
	T_U8	*bit;
	Ram_Hdr *ptr;
	T_U32   intvID;
	T_U16	szExtra;

	intvID = hMem->aback.largePtr-hMem->ahead.allocPtr;
	szExtra = hMem->beyond.llenpad+hMem->beyond.rlenpad+hMem->wilder.wlenpad+LEN_EXTRA_BLOCK(hMem);
	if(intvID < targID+szExtra)
		return AK_NULL;

	//heap split
	if(REGION_SMALL_LEFT == regn)
	{
		ptr = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->ahead.allocPtr);

		if(AK_NULL == hMem->ahead.critical)
		{
			//first alloc from heap
			MARK_HDR_PHY_PREV(ptr, INVALID_SIZE_NUMBER, 0);
			MARK_HDR_PHY_NEXT(ptr, targID, 0);	
		}
		else
		{
			MARK_HDR_PHY_PREV(ptr, PTR_TO_BLK(hMem, (T_U8*)hMem->ahead.critical), 0);
			MARK_HDR_PHY_NEXT(ptr, targID, 0);
		}

		//move critical ptr
		hMem->ahead.critical = ptr;
		hMem->ahead.allocPtr += szExtra+targID;
	}
	else
	{
		ptr = (Ram_Hdr*)DECR_HDR(hMem, (BLK_TO_PTR(hMem, hMem->aback.largePtr)), targID+szExtra);

		if(AK_NULL == hMem->aback.critical)
		{
			MARK_HDR_PHY_PREV(ptr, INVALID_SIZE_NUMBER, 0);
			MARK_HDR_PHY_NEXT(ptr, targID, 0);
		}
		else
		{
			MARK_HDR_PHY_PREV(hMem->aback.critical, PTR_TO_BLK(hMem, (T_U8*)ptr), 0);
			
			MARK_HDR_PHY_PREV(ptr, INVALID_SIZE_NUMBER, 0);
			MARK_HDR_PHY_NEXT(ptr, targID, 0);
		}

		//move critical ptr
		hMem->aback.critical = ptr;
		hMem->aback.largePtr -= szExtra+targID;
	}

	hMem->stat.n_blocks -= szExtra;

//MARK:
	
	//validate bubble
	MARK_HDR_PHY_NEXT(ptr, targID, 0);
	MARK_HDR_PHY_PREV(ptr, GET_MEM_IDX(ptr), (T_U8)pad);

	HDR_MEM_EXTPAD(ptr) = 0;

	if(0 != hMem->beyond.llenpad)
	{
		bit = (T_U8*)INCR_HDR(hMem, ptr, LEN_EXTRA_BLOCK(hMem)+hMem->wilder.wlenpad);
		memset(bit, hMem->beyond.lvar, hMem->setting.llen);
	}

	if(0 != hMem->beyond.rlenpad)
	{
		bit = (T_U8*)INCR_HDR(hMem, ptr, LEN_EXTRA_BLOCK(hMem)+hMem->wilder.wlenpad+hMem->beyond.llenpad+targID);
		memset(bit-pad, hMem->beyond.rvar, hMem->setting.rlen+pad);
	}

	if(0 != hMem->wilder.wlenpad)
	{
		Ram_Hdr *wildrHdr = (Ram_Hdr*)INCR_HDR(hMem, ptr, LEN_EXTRA_BLOCK(hMem));
		HDR_LOG_NEXT(wildrHdr) = (Ram_Hdr*)0xFFFFFFFF;
		HDR_LOG_PREV(wildrHdr) = (Ram_Hdr*)0xFFFFFFFF;
	}
	
	return ptr;
}

/**
 * @brief  合并半区域内存
 */
static T_U16	merge_block(GLOBALMEMHandle *hMem, Ram_Hdr *ptr, Ram_Hdr **big, REGION regn)
{
	T_U16	ret;
	T_U32   origID;
	T_U32   szExtra, size;
	Ram_Hdr *next, *prev;

	ret = 0;
	origID = GET_MEM_SIZE(ptr);
	szExtra = hMem->beyond.llenpad+hMem->beyond.rlenpad+hMem->wilder.wlenpad+LEN_EXTRA_BLOCK(hMem);

	//physical bordering
	prev = AK_NULL;
	if(GET_MEM_IDX(ptr) != INVALID_SIZE_NUMBER) prev = (Ram_Hdr*)HDR_PHY_PREV(hMem, ptr);
	next = AK_NULL;
	if(GET_MEM_SIZE(ptr) != INVALID_SIZE_NUMBER) next = (Ram_Hdr*)HDR_PHY_NEXT(hMem, ptr);
	if(next >= hMem->setting.lpTail) next = AK_NULL;

	if((REGION_SMALL_LEFT == regn) && (next > hMem->ahead.critical))
		next = AK_NULL;

	//merge free bubble
	if(prev && GET_MEM_OLD(prev))
	{
		//merge prev
		ret |= 0xf000;
		delete_node(hMem, prev, regn);
	}
	if(next && GET_MEM_OLD(next))
	{
		//merge next
		ret |= 0x0f00;
		delete_node(hMem, next, regn);
	}

	//change allocPtr
	if(REGION_SMALL_LEFT == regn)
	{
		if((hMem->ahead.critical == ptr)
			|| ((ret & 0x0f00) && (hMem->ahead.critical == next)))
		{
			if(ret & 0xf000)
			{
				hMem->ahead.critical = AK_NULL;
				if(GET_MEM_IDX(prev) != INVALID_SIZE_NUMBER)
					hMem->ahead.critical = (Ram_Hdr*)HDR_PHY_PREV(hMem, prev);
				hMem->ahead.allocPtr = PTR_TO_BLK(hMem, (T_U8*)prev);
			}
			else
			{
				hMem->ahead.critical = prev;
				hMem->ahead.allocPtr = PTR_TO_BLK(hMem, (T_U8*)ptr);
			}

			hMem->stat.n_blocks += szExtra;
			ret |= 0x00f0;
		}
	}
	else
	{
		if((hMem->aback.critical == ptr)
			|| ((ret & 0xf000) && (hMem->aback.critical == prev)))
		{
			if(ret & 0x0f00)
			{
				hMem->aback.critical = AK_NULL;
				if(GET_MEM_SIZE(next) != INVALID_SIZE_NUMBER)
					hMem->aback.critical = (Ram_Hdr*)HDR_PHY_NEXT(hMem, next);
				hMem->aback.largePtr = PTR_TO_BLK(hMem, (T_U8*)hMem->aback.critical);
			}
			else
			{
				hMem->aback.critical = (Ram_Hdr*)HDR_PHY_NEXT(hMem, ptr);
				hMem->aback.largePtr = PTR_TO_BLK(hMem, (T_U8*)hMem->aback.critical);
			}

			if(hMem->aback.largePtr >= hMem->heap.szBloks)
				hMem->aback.critical = AK_NULL;
			else
				MARK_HDR_PHY_PREV(hMem->aback.critical, INVALID_SIZE_NUMBER, 0);

			hMem->stat.n_blocks += szExtra;

			ret |= 0x000f;
		}
	}

	//merge into one big bubble
	(*big) = ptr;
	size   = origID;

	if(ret & 0xf000)
	{
		Ram_Hdr *next1;

		//prev+ptr => big
		size += GET_MEM_SIZE(prev)+szExtra;
		MARK_HDR_PHY_NEXT(prev, size, 1);

		prev->hdr.FileName = (*big)->hdr.FileName;
		prev->hdr.FileLine = (*big)->hdr.FileLine;
		(*big) = prev;

		//next -> prev
		next1 = AK_NULL;
		if(GET_MEM_SIZE(ptr) != INVALID_SIZE_NUMBER) next1 = (Ram_Hdr*)HDR_PHY_NEXT(hMem, ptr);
		if(next1 >= hMem->setting.lpTail) next1 = AK_NULL;
		if((REGION_SMALL_LEFT == regn) && (next1 > hMem->ahead.critical))
			next1 = AK_NULL;
		if(AK_NULL != next1)
			MARK_HDR_PHY_PREV(next1, PTR_TO_BLK(hMem, (T_U8*)(*big)), 0);

		//crush ptr hdr
		memset(ptr, 0, (hMem->setting.lenHdr << hMem->setting.align));

		hMem->stat.n_blocks += szExtra;
	}
	if(ret & 0x0f00)
	{
		Ram_Hdr *tx = (Ram_Hdr*)HDR_PHY_NEXT(hMem, next);
	    if(tx >= hMem->setting.lpTail) tx = AK_NULL;

		//big+next => big
		size += GET_MEM_SIZE(next)+szExtra;
		MARK_HDR_PHY_NEXT((*big), size, 1);

		if(tx) 
			MARK_HDR_PHY_PREV(tx, PTR_TO_BLK(hMem, (T_U8*)(*big)), 0);

		//crush next hdr
		memset(next, 0, (hMem->setting.lenHdr << hMem->setting.align));

		hMem->stat.n_blocks += szExtra;
	}

	MARK_HDR_PHY_NEXT((*big), size, 1);

	HDR_MEM_EXTPAD((*big)) = 0;

	//callback big buuble into virgin region
	if((ret & 0x00f0) || (ret & 0x000f))
	{
		if((ret & 0x000f) && (AK_NULL != hMem->aback.critical))
			MARK_HDR_PHY_PREV(hMem->aback.critical, INVALID_SIZE_NUMBER, 0);

		if(0 != hMem->wilder.wlenpad)
		{
			if(hMem->wilder.ncheck & 0x01)
				memset(*big, hMem->wilder.wvar, (szExtra+size)<<hMem->setting.align);
		}
		
		ret |= 0x00ff;
	}

	return ret;
}

/**
 * @brief  从左半区域分配泡泡
 */
static Ram_Hdr*	alloc_small(GLOBALMEMHandle *hMem, T_U32 targID, T_U32 pad)
{
	Ram_Hdr *mptr;
	Ram_Hdr *ptr;
	T_U32   hitID;
	T_U16	step;
	
	step = (T_U16)((hMem->ahead.allocPtr*20)/hMem->setting.sos);
	if(step > 15) step = 15;
	if(hMem->aback.largePtr <= hMem->setting.sos)
		step = 15;

	ptr = AK_NULL;
	//look up from Free List link
	hitID = get_block_index(hMem, targID, (T_U8)step);

	if(hitID != (T_U32)-1)
	{
		REGION regn = REGION_LARGE_RIGHT;
		if(hMem->ahead.list[hitID] <= hMem->ahead.critical)
			regn = REGION_SMALL_LEFT;

		//find it in free list
		ptr = (Ram_Hdr*)DECR_HDR(hMem, hMem->ahead.list[hitID], (LEN_EXTRA_BLOCK(hMem)+hMem->wilder.wlenpad+hMem->beyond.llenpad));
		delete_node(hMem, ptr, regn);

		split_block(hMem, ptr, targID, regn, pad);	
		return ptr;
	}

	//alloc from heap memory
	if(step < 15)
	{
		ptr = split_bubble(hMem, targID, REGION_SMALL_LEFT, pad);

		if(AK_NULL != ptr)
			return ptr;
	}

	//alloc from Misc List link
	if(hMem->ahead.misc != AK_NULL)
	{
		ptr = (Ram_Hdr*)DECR_HDR(hMem, hMem->ahead.misc, (LEN_EXTRA_BLOCK(hMem)+hMem->wilder.wlenpad+hMem->beyond.llenpad));
		delete_node(hMem, ptr, REGION_SMALL_LEFT);
	
		split_block(hMem, ptr, targID, REGION_SMALL_LEFT, pad);
		return ptr;
	}

	//alloc from heap
	if(step >= 15)
	{
		ptr = split_bubble(hMem, targID, REGION_SMALL_LEFT, pad);

		if(AK_NULL != ptr)
			return ptr;
	}		

	//borrow from right misc list
	for(mptr=hMem->aback.misc; mptr && HDR_LOG_NEXT(mptr) && (GET_MEM_SIZE(HDR_LOG_TO_PHY(hMem, mptr))<targID); mptr = HDR_LOG_NEXT(mptr))/*lint -e(722)*/;
	if((mptr != AK_NULL) && (GET_MEM_SIZE(HDR_LOG_TO_PHY(hMem, mptr))>=targID))
	{
		ptr = (Ram_Hdr*)DECR_HDR(hMem, (mptr), (LEN_EXTRA_BLOCK(hMem)+hMem->wilder.wlenpad+hMem->beyond.llenpad));
		delete_node(hMem, ptr, REGION_LARGE_RIGHT);

		split_block(hMem, ptr, targID, REGION_LARGE_RIGHT, pad);
		return ptr;
	}	
	
	return ptr;
}

/**
 * @brief  从右半区域分配泡泡
 */
static Ram_Hdr*	alloc_large(GLOBALMEMHandle *hMem, T_U32 targID, T_U32 pad)
{
	Ram_Hdr *mptr;
	Ram_Hdr *ptr = AK_NULL;

	//borrow from left misc list
	for(mptr=hMem->ahead.misc; mptr && HDR_LOG_NEXT(mptr) && (GET_MEM_SIZE(HDR_LOG_TO_PHY(hMem, mptr))<targID); mptr = HDR_LOG_NEXT(mptr))/*lint -e(722)*/;
	if((mptr != AK_NULL) && (GET_MEM_SIZE(HDR_LOG_TO_PHY(hMem, mptr))>=targID))
	{
		ptr = (Ram_Hdr*)DECR_HDR(hMem, (mptr), (LEN_EXTRA_BLOCK(hMem)+hMem->wilder.wlenpad+hMem->beyond.llenpad));
		delete_node(hMem, ptr, REGION_SMALL_LEFT);

		split_block(hMem, ptr, targID, REGION_SMALL_LEFT, pad);
		return ptr;
	}
	
	//look up from Misc List
	for(mptr=hMem->aback.misc; mptr && HDR_LOG_NEXT(mptr) && (GET_MEM_SIZE(HDR_LOG_TO_PHY(hMem, mptr))<targID); mptr = HDR_LOG_NEXT(mptr))/*lint -e(722)*/;
	if((mptr != AK_NULL) && (GET_MEM_SIZE(HDR_LOG_TO_PHY(hMem, mptr))>=targID))
	{
		ptr = (Ram_Hdr*)DECR_HDR(hMem, (mptr), (LEN_EXTRA_BLOCK(hMem)+hMem->wilder.wlenpad+hMem->beyond.llenpad));
		delete_node(hMem, ptr, REGION_LARGE_RIGHT);

		split_block(hMem, ptr, targID, REGION_LARGE_RIGHT, pad);
		return ptr;
	}

	//alloc from heap
	ptr = split_bubble(hMem, targID, REGION_LARGE_RIGHT, pad);
	if(AK_NULL != ptr)
		return ptr;

	return AK_NULL;
}

/**
 * @brief  释放内存到左半区域
 */
static T_VOID	free_small(GLOBALMEMHandle *hMem, Ram_Hdr *ptr)
{
	T_U16   result;
	Ram_Hdr *ptr2;	

	//merge free block
	result = merge_block(hMem, ptr, &ptr2, REGION_SMALL_LEFT);

	//not back to heap memory 
	if(0 == (result & 0x00ff))
	{	
		insert_node(hMem, ptr2, REGION_SMALL_LEFT);
	}

}

/**
 * @brief  释放内存到右半区域
 */
static T_VOID	free_large(GLOBALMEMHandle *hMem, Ram_Hdr *ptr)
{
	T_U16   result;
	Ram_Hdr *ptr2;

	//merge free block
	result = merge_block(hMem, ptr, &ptr2, REGION_LARGE_RIGHT);

	//not back to heap memory 
	if(0 == (result & 0x00ff))
	{		
		insert_node(hMem, ptr2, REGION_LARGE_RIGHT);
	}
}

/**
 * @brief  自动进栈回调函数
 */
static T_VOID	enter_statemachine(T_GLOBALMEMHandle hMemory)
{
	T_U32 magic;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(AK_FALSE == hMem->trace.bAutoTrace)
	{
		RAM_PRINTF(hMem)("enter_statemachine Error : not open \r\n");
		return ;	
	}

	hMem->trace.curStack --;
	if(hMem->trace.curStack < 0)
		return;

	magic = 0xF96F0000 | hMem->trace.curStack;

	hMem->trace.mem[hMem->trace.curStack].s_addr = (T_U8*)magic;
	hMem->trace.mem[hMem->trace.curStack].s_next = AK_NULL;
	
}

/**
 * @brief  自动出栈回调函数
 */
static T_VOID	leave_statemachine(T_GLOBALMEMHandle hMemory)
{
	T_U32 i, j;
	T_U32 tmp;
	T_U8 *s_addr;
	T_S8  strNum[8], strTmp[8];
	T_S8  strHint[128];
	T_MEMORY_TRACE map;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;


	if(AK_FALSE == hMem->trace.bAutoTrace)
	{
		RAM_PRINTF(hMem)("enter_statemachine Error : not open \r\n");
		return ;	
	}

	if(hMem->trace.curStack < 0)
	{
		hMem->trace.curStack ++;
		return;
	}

	strHint[0] = 0;

	//itoa src and make its length==2
	i = 0;
	tmp = hMem->trace.curStack;
	do{
		strTmp[i++] = (T_S8)(tmp%10)+'0';
		tmp /= 10;
	}while(tmp);

	j = 0;
	while(i > 0)
		strNum[j++] = strTmp[--i];
	strNum[j] = '\0';

	if(0 == strNum[1])
	{
		strNum[1] = strNum[0]; strNum[0] = '0'; strNum[2] = 0;
	}

	
	s_addr = hMem->trace.mem[hMem->trace.curStack].s_next;
	if(s_addr)
	{
		strcpy(strHint, "\r\n\r\n[[[****** Auto Memory Leak Active In Stack = ");
		strcpy(strHint+49, strNum);
		strcpy(strHint+51, " Begin ******\r\n");

		map.addr = 0;
		map.size = 0;
		map.line = 0;
		map.filename = strHint;
		map.old = 0xFF;
		map.reqSize = 0;

		(*(hMem->trace.enumFunc))(hMem, &map, AK_NULL);
	}
	while(s_addr)
	{
		i = (s_addr-(T_U8*)hMem->trace.mem)/sizeof(*hMem->trace.mem);

		{
			Ram_Hdr *ptr = (Ram_Hdr*)(hMem->trace.mem[i].s_addr);

			map.addr = HDR_TO_PTR(hMem, ptr);
			map.size = GET_MEM_SIZE(ptr) << hMem->setting.align;
			map.line = ptr->hdr.FileLine;
			map.filename = ptr->hdr.FileName;
			map.reqSize = map.size-GET_MEM_PAD(ptr)-(HDR_MEM_EXTPAD(ptr)<<hMem->setting.align);

			if(GET_MEM_OLD(ptr))
				map.old = 1;
			else
				map.old = 0;

			(*(hMem->trace.enumFunc))(hMem, &map, AK_NULL);
		}

		s_addr = hMem->trace.mem[i].s_next;

		memset(&hMem->trace.mem[i], 0, sizeof(*hMem->trace.mem));
	}
	memset(&hMem->trace.mem[hMem->trace.curStack], 0, sizeof(*hMem->trace.mem));

	if(strHint[0])
	{
		strcpy(strHint, "\r\n\r\n****** Auto Memory Leak Active In Stack = ");
		strcpy(strHint+46, strNum);
		strcpy(strHint+48, " End ******]]]\r\n");

		map.addr = 0;
		map.size = 0;
		map.line = 0;
		map.filename = strHint;
		map.old = 0xFF;
		map.reqSize = 0;

		(*(hMem->trace.enumFunc))(hMem, &map, AK_NULL);
	}

	hMem->trace.curStack ++;
}

/**
 * @brief  分配入栈
 */
static T_VOID	push_statemachine(GLOBALMEMHandle *hMem, Ram_Hdr *ptr)
{
	T_U32 i, j;
	T_U8 *s_addr;

	if(AK_FALSE == hMem->trace.bAutoTrace)
	{
		RAM_PRINTF(hMem)("push_statemachine Error : not open \r\n");
		return ;	
	}

	if((hMem->trace.curStack<0) || (hMem->trace.curStack>=DEFAULT_AUTOTRACE_STACK))
		return ;

	//search a blank mem
	for(i=DEFAULT_AUTOTRACE_STACK; i<hMem->trace.logCnt; i++)
	{
		if(AK_NULL == hMem->trace.mem[i].s_addr)
			break;
	}
	if(i >= hMem->trace.logCnt)
	{
		//RAM_PRINTF(hMem)("push_statemachine Warning : stack overfloat \r\n");
		return ;	
	}

	//link it to current stack
	s_addr = hMem->trace.mem[hMem->trace.curStack].s_next;
	j = hMem->trace.curStack;
	while(s_addr)
	{
		j = (s_addr-(T_U8*)hMem->trace.mem)/sizeof(*hMem->trace.mem);
		s_addr = hMem->trace.mem[j].s_next;
	}
	hMem->trace.mem[j].s_next = (T_U8*)(&hMem->trace.mem[i]);

	hMem->trace.mem[i].s_addr = (T_U8*)ptr;
	hMem->trace.mem[i].s_next = AK_NULL;

}

/**
 * @brief  释放出栈
 */
static T_VOID	pop_statemachine(GLOBALMEMHandle *hMem, Ram_Hdr *ptr)
{
	T_U32 i, j;
	T_U8 *s_addr, *s_prev;


	if(AK_FALSE == hMem->trace.bAutoTrace)
	{
		RAM_PRINTF(hMem)("push_statemachine Error : not open \r\n");
		return ;	
	}

	if((hMem->trace.curStack<0) || (hMem->trace.curStack>=DEFAULT_AUTOTRACE_STACK))
		return ;

	//delink it to current stack
	s_addr = hMem->trace.mem[hMem->trace.curStack].s_next;
	s_prev = (T_U8*)(&hMem->trace.mem[hMem->trace.curStack]);
	while(s_addr)
	{
		j = (s_addr-(T_U8*)hMem->trace.mem)/sizeof(*hMem->trace.mem);
		s_addr = hMem->trace.mem[j].s_next;
		
		if(hMem->trace.mem[j].s_addr == (T_U8*)ptr)
		{
			i = (s_prev-(T_U8*)hMem->trace.mem)/sizeof(*hMem->trace.mem);
			hMem->trace.mem[i].s_next = s_addr;

			memset(&hMem->trace.mem[j], 0, sizeof(*hMem->trace.mem));
			break;
		}

		s_prev = (T_U8*)(&hMem->trace.mem[j]);
	}

}



/**
 * @brief  Malloc one memory block from global heap memory
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_U32 size : want memory size 
 * @param   T_S8 *filename : alloc hander location filename
 * @param   T_S8 *fileline : alloc hander location fileline
 *
 * @return  T_pVOID : memory address for success, AK_NULL for failure 
 *
 */
static T_pVOID	s_Ram_Alloc(T_GLOBALMEMHandle hMemory, T_U32 size, T_S8 *filename, T_U32 fileline)
{	
	T_U32 pad;
	Ram_Hdr *ptr;
	T_U16 szExtra;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

//	dbgMem[dbgCnt++]=size;

	if((0 ==size) || ((size>>hMem->setting.align) >= INVALID_SIZE_NUMBER))
	{
		RAM_PRINTF(hMem)("Ram_Alloc Warning : null alloc or toohuge <size=%d>\r\n", size);
		return AK_NULL;	
	}

	szExtra = hMem->beyond.llenpad+hMem->beyond.rlenpad+LEN_EXTRA_BLOCK(hMem)+hMem->wilder.wlenpad;
	pad = ALIGN_By_BASE(size, (1<<hMem->setting.align))-size;
	size = (size+pad) >> hMem->setting.align;

	if(hMem->stat.n_blocks < size+szExtra)
	{
		RAM_PRINTF(hMem)("Ram_Alloc Error : no enough space \r\n");
		return AK_NULL;	
	}

	ptr = AK_NULL;
	
	//alloc from left
	if(size < hMem->setting.lenFree)
		ptr = alloc_small(hMem, size, pad);
	//alloc from right
	else
		ptr = alloc_large(hMem, size, pad);

	if(AK_NULL == ptr)
	{
		RAM_PRINTF(hMem)("Ram_Alloc Error : memory is exhaust \r\n");
		return AK_NULL;
	}

	hMem->stat.n_blocks -= GET_MEM_SIZE(ptr);
	hMem->stat.n_nUsed ++;
	hMem->stat.n_szUsed += GET_MEM_SIZE(ptr);

	ptr->hdr.FileName = filename;
	ptr->hdr.FileLine = (T_U16)fileline;

	//auto detect memory leak
	if(hMem->trace.bAutoTrace)
		push_statemachine(hMem, ptr);	

	return (T_pVOID)(HDR_TO_PTR(hMem, ptr));
}

T_pVOID	Ram_Alloc(T_GLOBALMEMHandle hMemory, T_U32 size, T_S8 *filename, T_U32 fileline)
{
	T_pVOID hRet;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 100))
		return AK_NULL;

	Ram_Lock(hMem->sys.dwLock);
	hRet = s_Ram_Alloc(hMemory, size, filename, fileline);
	Ram_Unlock(hMem->sys.dwLock);


	return hRet;
}

/**
 * @brief  Enumerate memory statck status info
 * @只能用于直接检测指定内存合法情况
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_WILD_TYPE *wildInfo : wild info loader
 *
 * @return  AK_TRUE for no wild ;else for AK_FALSE
 *
 */
static T_BOOL s_Ram_CheckPtr(T_GLOBALMEMHandle hMemory, T_VOID *var, T_WILD_TYPE *wildType)
{
	T_U32 dist;
	Ram_Hdr *ptr;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;


	//===============addr adjust ===================================

	if(((T_U8*)var < (T_U8*)hMem->heap.lpAddr) 
		|| ((T_U8*)var > (T_U8*)INCR_HDR(hMem, hMem->heap.lpAddr, hMem->heap.szBloks)))
	{
		RAM_PRINTF(hMem)("Ram_CheckPtr Error: var is outrange 1 \r\n");
		(*wildType) = WILD_INVALID_UNHEAP;
		return AK_FALSE;
	}

	if(((T_U8*)var >= (T_U8*)(BLK_TO_PTR(hMem, hMem->ahead.allocPtr)))
		&& ((T_U8*)var < (T_U8*)(BLK_TO_PTR(hMem, hMem->aback.largePtr))))
	{
		RAM_PRINTF(hMem)("Ram_CheckPtr Error: var is outrange 2 \r\n");
		(*wildType) = WILD_INVALID_INHEAP;
		return AK_FALSE;	
	}

	dist = (T_U8*)var - (T_U8*)hMem->heap.lpAddr;
	if(dist % (1 << hMem->setting.align))
	{
		RAM_PRINTF(hMem)("Ram_CheckPtr Error: var is not memory header 1 \r\n");
		(*wildType) = WILD_INVALID;
		return AK_FALSE;	
	}

	//===============magic adjust ===================================

	ptr = (Ram_Hdr*)PTR_TO_HDR(hMem, ((T_U8*)var));

	if(GET_MEM_MAGIC(ptr) != MAGIC_NUM_HDR)
	{
		RAM_PRINTF(hMem)("Ram_CheckPtr Error: var is not memory header 2 \r\n");
		(*wildType) = WILD_INVALID;
		return AK_FALSE;
	}	

	if((GET_MEM_SIZE(ptr)!=INVALID_SIZE_NUMBER)
		&& (GET_MEM_SIZE(ptr)>= hMem->heap.szBloks))
	{
		RAM_PRINTF(hMem)("Ram_CheckPtr Error: var size is corrupt \r\n");
		(*wildType) = WILD_INVALID;
		return AK_FALSE;
	}

	if((GET_MEM_IDX(ptr)!=INVALID_SIZE_NUMBER)
		&& (GET_MEM_IDX(ptr)>= hMem->heap.szBloks))
	{
		RAM_PRINTF(hMem)("Ram_CheckPtr Error: var idx is corrupt \r\n");
		(*wildType) = WILD_INVALID;
		return AK_FALSE;
	}

	if(GET_MEM_OLD(ptr))
	{
		//RAM_PRINTF(hMem)("Ram_CheckPtr Error: var is wild pointer \r\n");
		(*wildType) = WILD_WILD;
		return AK_FALSE;
	}

	(*wildType) = WILD_OK;
	return AK_TRUE;
}

/**
 * @brief  Free one memory block to global heap memory
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_pVOID var : memory start address alloced before 
 * @param   T_S8 *filename : alloc hander location filename
 * @param   T_S8 *fileline : alloc hander location fileline
 *
 * @return  T_pVOID : AK_NULL
 *
 */
static T_pVOID s_Ram_Free(T_GLOBALMEMHandle hMemory, T_pVOID var, T_S8 *filename, T_U32 fileline)
{
	Ram_Hdr *ptr;
	T_WILD_TYPE err;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;


	if(AK_NULL == var)
		return AK_NULL;

//dbgMem[dbgCnt++]=(0x80000000|(T_U32)var);	

	//check ptr
	if(AK_FALSE == s_Ram_CheckPtr(hMemory, var, &err))
	{
		RAM_PRINTF(hMem)("Ram_Free Error : it's bad memory pointer, ptr=0x%x filename=%s fileline=%d \r\n", var, filename, fileline);
		return AK_NULL;
	}

	ptr = (Ram_Hdr*)PTR_TO_HDR(hMem, ((T_U8*)var));

	hMem->stat.n_blocks += GET_MEM_SIZE(ptr);
	hMem->stat.n_nUsed --;
	hMem->stat.n_szUsed -= GET_MEM_SIZE(ptr);

	ptr->hdr.FileName = filename;
	ptr->hdr.FileLine = (T_U16)fileline;

	//free from 
	if(ptr <= hMem->ahead.critical)
		free_small(hMem, ptr);
	else
		free_large(hMem, ptr);

	if(hMem->trace.bAutoTrace)
		pop_statemachine(hMem, ptr);	

	
	return AK_NULL;
}

T_pVOID Ram_Free(T_GLOBALMEMHandle hMemory, T_pVOID var, T_S8 *filename, T_U32 fileline)
{
	T_pVOID hRet;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 102))
		return AK_NULL;

	Ram_Lock(hMem->sys.dwLock);
	hRet = s_Ram_Free(hMemory, var, filename, fileline);
	Ram_Unlock(hMem->sys.dwLock);

	return hRet;
}

/**
 * @brief  Remalloc one memory block from global heap memory
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param   T_pVOID var : old memory address
 * @param	T_U32 size : want memory size 
 * @param   T_S8 *filename : alloc hander location filename
 * @param   T_S8 *fileline : alloc hander location fileline
 *
 * @return  T_pVOID : new memory address for success, AK_NULL for failure 
 *
 */
T_pVOID	Ram_Realloc(T_GLOBALMEMHandle hMemory, T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline)
{
	//T_U32 i;
	Ram_Hdr *ptr;
	T_pVOID var2;
	T_WILD_TYPE wildType;
	T_U32   sz;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 101))
		return AK_NULL;

	Ram_Lock(hMem->sys.dwLock);

	if(AK_FALSE == s_Ram_CheckPtr(hMemory, var, &wildType))
	{
		Ram_Unlock(hMem->sys.dwLock);
		RAM_PRINTF(hMem)("Ram_ReMalloc Error : var is not pass Ram_CheckPtr \r\n");
		return AK_NULL;
	}

	ptr = PTR_TO_HDR(hMem, var);
	
	sz = GET_MEM_SIZE(ptr) << hMem->setting.align;
	sz = sz - GET_MEM_PAD(ptr)-(HDR_MEM_EXTPAD(ptr)<<hMem->setting.align);

	if(sz >= size)
	{
		T_U32 pad = ALIGN_By_BASE(size, (1<<hMem->setting.align))-size;

		ptr->hdr.FileName = filename;
		ptr->hdr.FileLine = (T_U16)fileline;
		MARK_HDR_PHY_PREV(ptr, GET_MEM_IDX(ptr), (T_U8)pad);
		
		Ram_Unlock(hMem->sys.dwLock);
		return var;
	}

	var2 = s_Ram_Alloc(hMemory, size, filename, fileline);
	if(AK_NULL == var2)
	{
		Ram_Unlock(hMem->sys.dwLock);
		RAM_PRINTF(hMem)("Ram_ReMalloc Error : can't alloc new memory \r\n");
		return AK_NULL;
	}

	//for(i=0; i<size; i++)
		//((T_U8*)var2)[i] = ((T_U8*)var)[i];
	memcpy(var2, var, sz);

	var = s_Ram_Free(hMemory, var, __FILE__, __LINE__);

	Ram_Unlock(hMem->sys.dwLock);
	
	return var2;
}

/**
 * @brief  Enumerate memory statck status info
 * @只能用于查询内存状态和检测显式内存泄漏情况
 *
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	Ram_EnumMemTraceFun enumFun : user define memory map callback query and prontf function
 * @param   T_pVOID attach : user self data needed by callback function
 *
 * @return  T_pVOID : AK_NULL
 *
 */
T_VOID Ram_EnumMemTraceInfo(T_GLOBALMEMHandle hMemory, Ram_EnumMemTraceFun enumFun, T_pVOID attach)
{
	T_U32   cnt;
	Ram_Hdr *ptr;
	T_U32   used, unused;
	Ram_Hdr *left, *right;
	T_MEMORY_TRACE map;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 6))
		return;

	Ram_Lock(hMem->sys.dwLock);

	cnt = 0;
	used = 0;
	unused = 0;

	//search before allocPtr
	ptr = (Ram_Hdr*)BLK_TO_PTR(hMem, 0);
	left = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->ahead.allocPtr);

	for(; ptr<left; ptr=HDR_PHY_NEXT(hMem, ptr))
	{
		map.addr = HDR_TO_PTR(hMem, ptr);
		map.size = GET_MEM_SIZE(ptr) << hMem->setting.align;
		map.line = ptr->hdr.FileLine;
		map.filename = ptr->hdr.FileName;
		map.reqSize = map.size-GET_MEM_PAD(ptr)-(HDR_MEM_EXTPAD(ptr)<<hMem->setting.align);

		if(GET_MEM_OLD(ptr))
		{
			map.old = 1;
			unused += map.size;
		}
		else
		{
			map.old = 0;
			used += map.size;
		}

		(*enumFun)(hMemory, &map, attach);

		cnt ++;
	}

	//search after largePtr
	ptr = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->aback.largePtr);
	right = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->heap.szBloks);

	for(; ptr<right; ptr=HDR_PHY_NEXT(hMem, ptr))
	{
		map.addr = HDR_TO_PTR(hMem, ptr);
		map.size = GET_MEM_SIZE(ptr) << hMem->setting.align;
		map.line = ptr->hdr.FileLine;
		map.filename = ptr->hdr.FileName;
		map.reqSize = map.size-GET_MEM_PAD(ptr)-(HDR_MEM_EXTPAD(ptr)<<hMem->setting.align);

		if(GET_MEM_OLD(ptr))
		{
			map.old = 1;
			unused += map.size;
		}
		else
		{
			map.old = 0;
			used += map.size;
		}

		(*enumFun)(hMemory, &map, attach);

		cnt ++;
	}

	{
		T_U32 total = (hMem->heap.szBloks<<hMem->setting.align);
		T_U32 extra = (cnt*(hMem->beyond.llenpad+hMem->beyond.rlenpad+hMem->wilder.wlenpad+LEN_EXTRA_BLOCK(hMem)))<<hMem->setting.align;

		RAM_PRINTF(hMem)("Ram_EnumMemTraceInfo Hint : Total=%8d Used=%8d Unused=%8d Cnt=%8d Extra=%8d Remain=%8d \r\n",
					total,
					used,
					unused,
					cnt,
					extra,
					total-used-unused-extra);
	}

	Ram_Unlock(hMem->sys.dwLock);
}

/**
 * @brief  Enumerate memory statck status info
 * @只能用于直接检测指定内存越界情况
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_MEMORY_BEYOND_TRACE *beyondInfo : beyond info loader
 *
 * @return  AK_TRUE for no beyond ;else for AK_FALSE
 *
 */
static T_BOOL s_Ram_CheckBeyondPtr(T_GLOBALMEMHandle hMemory, T_VOID *var, T_BEYOND_TYPE *beyondType, T_U32 *beyondLoc)
{
	T_U32   i, j;
	T_U32	pad;
	T_U32   beyond;
	T_U8    *bit;
	Ram_Hdr *ptr;
	T_WILD_TYPE wildType;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;


	if((0==hMem->beyond.llenpad) && (0==hMem->beyond.rlenpad))
	{
		RAM_PRINTF(hMem)("Ram_CheckBeyondPtr Error : not enable this function \r\n");
		(*beyondType) = BEYOND_ERR;
		return AK_FALSE;	
	}

	if(AK_FALSE == s_Ram_CheckPtr(hMemory, var, &wildType))
	{
		RAM_PRINTF(hMem)("Ram_CheckBeyondPtr Error : var is not pass Ram_CheckPtr \r\n");
		(*beyondType) = BEYOND_INVALID+wildType;
		return AK_FALSE;
	}

	ptr = (Ram_Hdr*)PTR_TO_HDR(hMem, ((T_U8*)var));
	(*beyondType) = BEYOND_OK;

	//prev check
	bit = (T_U8*)INCR_HDR(hMem, ptr, LEN_EXTRA_BLOCK(hMem)+hMem->wilder.wlenpad);
	beyond = hMem->beyond.llenpad<<hMem->setting.align;
	for(i=0; i<beyond; i++)
	{
		if(bit[i] != hMem->beyond.lvar)
		{
			for(j=i+1; (j<beyond && bit[j]!=hMem->beyond.lvar); j++);
			(*beyondLoc) = i << 16;
			(*beyondLoc) |= j-i;

			RAM_PRINTF(hMem)("Ram_CheckBeyondPtr Error : beyond loc = %d len = %d\r\n", i, j-i);
			(*beyondType) |= BEYOND_PREV;
			break;//return AK_FALSE;
		}
	}

	pad = 0;
	if(0 != hMem->beyond.ncheck)
		pad = (T_U32)GET_MEM_PAD(ptr);
	pad += HDR_MEM_EXTPAD(ptr)<<hMem->setting.align;

	//post check
	bit = (T_U8*)INCR_HDR(hMem, ptr, LEN_EXTRA_BLOCK(hMem)+hMem->beyond.llenpad+hMem->wilder.wlenpad+GET_MEM_SIZE(ptr));
	bit -= pad;

	beyond = hMem->beyond.rlenpad<<hMem->setting.align;
	beyond += pad;

	for(i=0; i<beyond; i++)
	{
		if(bit[i] != hMem->beyond.rvar)
		{
			for(j=i+1; (j<beyond && bit[j]!=hMem->beyond.rvar); j++);
			(*beyondLoc) = i << 16;
			(*beyondLoc) |= j-i;

			RAM_PRINTF(hMem)("Ram_CheckBeyondPtr Error : beyond loc = %d len= %d\r\n", i, j-i);
			(*beyondType) |= BEYOND_BACK;
			break;//return AK_FALSE;
		}
	}

	if(BEYOND_OK != (*beyondType))
		return AK_FALSE;

	return AK_TRUE;
}

T_BOOL Ram_CheckBeyondPtr(T_GLOBALMEMHandle hMemory, T_VOID *var, T_BEYOND_TYPE *beyondType)
{
	T_U32  beyondLoc;
	T_BOOL bRet;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 7))
	{
		(*beyondType) = BEYOND_ERR;
		return AK_FALSE;	
	}	

	Ram_Lock(hMem->sys.dwLock);
	bRet = s_Ram_CheckBeyondPtr(hMemory, var, beyondType, &beyondLoc);
	Ram_Unlock(hMem->sys.dwLock);

	return bRet;
}

/**
 * @brief  Enumerate memory statck status info
 * @只能用于检测内存越界情况
 * @从堆内存头开始检测,检测到第一个越界即退出
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_MEMORY_BEYOND_TRACE *beyondInfo : beyond info loader
 *
 * @return  AK_TRUE for no beyond ;else for AK_FALSE
 *
 */
T_BOOL Ram_CheckBeyond(T_GLOBALMEMHandle hMemory, T_MEMORY_BEYOND_TRACE *beyondInfo)
{
	T_U32  beyondLoc;
	Ram_Hdr *ptr;
	Ram_Hdr *left, *right;
	T_BEYOND_TYPE beyondType;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 8))
	{
		beyondInfo->type = BEYOND_ERR;
		return AK_FALSE;	
	}	

	if((0==hMem->beyond.llenpad) && (0==hMem->beyond.rlenpad))
	{
		RAM_PRINTF(hMem)("Ram_CheckBeyond Error : not enable this function \r\n");
		beyondInfo->type = BEYOND_ERR;
		return AK_FALSE;	
	}

	Ram_Lock(hMem->sys.dwLock);
	
	//search before allocPtr
	ptr = (Ram_Hdr*)BLK_TO_PTR(hMem, 0);
	left = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->ahead.allocPtr);

	for(; ptr<left; ptr=HDR_PHY_NEXT(hMem, ptr))
	{
		beyondInfo->addr = HDR_TO_PTR(hMem, ptr);
		beyondInfo->size = GET_MEM_SIZE(ptr) << hMem->setting.align;
		beyondInfo->line = ptr->hdr.FileLine;
		beyondInfo->filename = ptr->hdr.FileName;
		beyondInfo->reqSize = beyondInfo->size-GET_MEM_PAD(ptr)-(HDR_MEM_EXTPAD(ptr)<<hMem->setting.align);
		beyondInfo->type = BEYOND_OK;

		if(GET_MEM_OLD(ptr))
			beyondInfo->old = 1;
		else
			beyondInfo->old = 0;

		if((0 == beyondInfo->old) 
			&& (AK_FALSE == s_Ram_CheckBeyondPtr(hMemory, beyondInfo->addr, &beyondType, &beyondLoc)))
		{
			beyondInfo->loc  = (T_U16)(beyondLoc>>16);
			beyondInfo->cnt  = (T_U16)(beyondLoc & 0xFFFF);
			beyondInfo->type = beyondType;
			Ram_Unlock(hMem->sys.dwLock);
			return AK_FALSE;
		}
	}

	//search after largePtr
	ptr = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->aback.largePtr);
	right = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->heap.szBloks);

	for(; ptr<right; ptr=HDR_PHY_NEXT(hMem, ptr))
	{
		beyondInfo->addr = HDR_TO_PTR(hMem, ptr);
		beyondInfo->size = GET_MEM_SIZE(ptr) << hMem->setting.align;
		beyondInfo->line = ptr->hdr.FileLine;
		beyondInfo->filename = ptr->hdr.FileName;
		beyondInfo->reqSize = beyondInfo->size-GET_MEM_PAD(ptr)-(HDR_MEM_EXTPAD(ptr)<<hMem->setting.align);
		beyondInfo->type = BEYOND_OK;

		if(GET_MEM_OLD(ptr))
			beyondInfo->old = 1;
		else
			beyondInfo->old = 0;

		if((0 == beyondInfo->old)
			&& (AK_FALSE == s_Ram_CheckBeyondPtr(hMemory, beyondInfo->addr, &beyondType, &beyondLoc)))
		{
			beyondInfo->loc  = (T_U16)(beyondLoc>>16);
			beyondInfo->cnt  = (T_U16)(beyondLoc & 0xFFFF);
			beyondInfo->type = beyondType;
			Ram_Unlock(hMem->sys.dwLock);
			return AK_FALSE;
		}
	}

	Ram_Unlock(hMem->sys.dwLock);
	return AK_TRUE;
}

/**
 * @brief  Enumerate memory statck status info
 * @只能用于直接检测指定内存越界情况
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	T_BEYOND_TYPE *beyondType : beyond type
 *
 * @return  AK_TRUE for no beyond ;else for AK_FALSE
 *
 */
static T_BOOL s_Ram_CheckWildrPtr(T_GLOBALMEMHandle hMemory, T_VOID *var, T_WILD_TYPE *wildType)
{
	T_U32   i;
	T_S32   sz;
	T_U8    *bit;
	T_U16	szExtra;
	Ram_Hdr *ptr;
	Ram_Hdr *wildrHdr, *ptr2;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;


	if(0 == hMem->wilder.wlenpad)
	{
		RAM_PRINTF(hMem)("Ram_CheckWilderPtr Error : not enable this function \r\n");
		(*wildType) = WILD_ERR;
		return AK_FALSE;	
	}

	if(AK_FALSE == s_Ram_CheckPtr(hMemory, var, wildType))
	{
		if((*wildType!=WILD_WILD) && (*wildType!=WILD_INVALID_INHEAP))
		{
			RAM_PRINTF(hMem)("Ram_CheckWilderPtr Error : var is not pass Ram_CheckPtr \r\n");
			return AK_FALSE;
		}
	}
	else
	{
		RAM_PRINTF(hMem)("Ram_CheckWilderPtr Error : var is using memory \r\n");
		return AK_TRUE;
	}

	if(*wildType == WILD_WILD)
	{
		ptr = (Ram_Hdr*)PTR_TO_HDR(hMem, ((T_U8*)var));

		if(GET_MEM_OLD(ptr) != 1)
		{
			RAM_PRINTF(hMem)("Ram_CheckWilderPtr Error : var is not spare \r\n");
			return AK_TRUE;
		}
	
		ptr2 = HDR_PHY_TO_LOG(hMem, ptr);

		//check first 8byte
		if(hMem->wilder.ncheck & 0x10)
		{
			wildrHdr = (Ram_Hdr*)INCR_HDR(hMem, ptr, LEN_EXTRA_BLOCK(hMem));	
			if((HDR_LOG_NEXT(wildrHdr) != HDR_LOG_NEXT(ptr2))
				|| (HDR_LOG_PREV(wildrHdr) != HDR_LOG_PREV(ptr2)))
			{  
				(*wildType) = WILD_WILD_PREV;
			}
		}

		//check user data
		if(hMem->wilder.ncheck & 0x01)
		{
			sz = sizeof(HDR_LOG_NEXT(ptr2))+sizeof(HDR_LOG_PREV(ptr2));
			bit = (T_U8*)ptr2+sz;
			sz = (GET_MEM_SIZE(ptr)<<hMem->setting.align)-sz-GET_MEM_PAD(ptr);
			if(sz > 0)
			{
				for(i=0; i<(T_U32)sz; i++)
				{
					if(bit[i] != hMem->wilder.wvar)
					{
						(*wildType) |= WILD_WILD_BACK;
						break;
					}
				}
			}
		}
	}
	else
	{
		if(hMem->wilder.ncheck & 0x01)
		{
			szExtra = hMem->beyond.llenpad+hMem->wilder.wlenpad+hMem->beyond.rlenpad+LEN_EXTRA_BLOCK(hMem);
			szExtra += 1;
			szExtra <<= hMem->setting.align;
			bit = (T_U8*)var;

			for(i=0; i<szExtra; i++)
			{
				if(bit[i] != hMem->wilder.wvar)
				{
					(*wildType) = WILD_INVALID_INHEAP_WILD;
					break;
				}
			}
		}
	}

	if((*wildType==WILD_WILD) || (*wildType==WILD_INVALID_INHEAP))
		return AK_TRUE;//no write action

	return AK_FALSE;
}

T_BOOL Ram_CheckWildrPtr(T_GLOBALMEMHandle hMemory, T_VOID *var, T_WILD_TYPE *wildType)
{
	T_U8 bRet;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 9))
	{
		(*wildType) = WILD_ERR;
		return AK_FALSE;	
	}	

	Ram_Lock(hMem->sys.dwLock);
	bRet = s_Ram_CheckWildrPtr(hMemory, var, wildType);
	Ram_Unlock(hMem->sys.dwLock);
	
	return bRet;
}

/**
 * @brief  Enumerate memory statck status info
 * @只能用于检测野指针情况
 * @从堆内存头开始检测,检测到第一个野指针使用即退出
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 * @param	T_MEMORY_WILDER_TRACE *wildInfo : wild info loader
 *
 * @return  AK_TRUE for no wild ;else for AK_FALSE
 *
 */
T_BOOL Ram_CheckWilder(T_GLOBALMEMHandle hMemory, T_MEMORY_WILDER_TRACE *wildInfo)
{
	T_U8 *bit;
	Ram_Hdr *ptr;
	Ram_Hdr *left, *right;
	T_WILD_TYPE wildType;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 10))
	{
		wildInfo->type = WILD_ERR;
		return AK_FALSE;	
	}	

	if(0 == hMem->wilder.wlenpad)
	{
		RAM_PRINTF(hMem)("Ram_CheckWilder Error : not enable this function \r\n");
		wildInfo->type = WILD_ERR;
		return AK_FALSE;	
	}

	Ram_Lock(hMem->sys.dwLock);
		
	//search before allocPtr
	ptr = (Ram_Hdr*)BLK_TO_PTR(hMem, 0);
	left = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->ahead.allocPtr);

	for(; ptr<left; ptr=HDR_PHY_NEXT(hMem, ptr))
	{
		wildInfo->addr = HDR_TO_PTR(hMem, ptr);
		wildInfo->size = GET_MEM_SIZE(ptr) << hMem->setting.align;
		wildInfo->line = ptr->hdr.FileLine;
		wildInfo->filename = ptr->hdr.FileName;
		wildInfo->reqSize = wildInfo->size-GET_MEM_PAD(ptr)-(HDR_MEM_EXTPAD(ptr)<<hMem->setting.align);
		wildInfo->type = WILD_OK;

		if((1 == GET_MEM_OLD(ptr)) 
			&& (AK_FALSE == s_Ram_CheckWildrPtr(hMemory, wildInfo->addr, &wildType)))
		{
			wildInfo->type = wildType;
			Ram_Unlock(hMem->sys.dwLock);
			return AK_FALSE;
		}
	}

	//search after largePtr
	ptr = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->aback.largePtr);
	right = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->heap.szBloks);

	for(; ptr<right; ptr=HDR_PHY_NEXT(hMem, ptr))
	{
		wildInfo->addr = HDR_TO_PTR(hMem, ptr);
		wildInfo->size = GET_MEM_SIZE(ptr) << hMem->setting.align;
		wildInfo->line = ptr->hdr.FileLine;
		wildInfo->filename = ptr->hdr.FileName;
		wildInfo->reqSize = wildInfo->size-GET_MEM_PAD(ptr)-(HDR_MEM_EXTPAD(ptr)<<hMem->setting.align);
		wildInfo->type = WILD_OK;

		if((1 == GET_MEM_OLD(ptr))
			&& (AK_FALSE == s_Ram_CheckWildrPtr(hMemory, wildInfo->addr, &wildType)))
		{
			wildInfo->type = wildType;
			Ram_Unlock(hMem->sys.dwLock);
			return AK_FALSE;
		}
	}

	//check inheap status
	if(hMem->wilder.ncheck & 0x01)
	{
		left = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->ahead.allocPtr);
		right = (Ram_Hdr*)BLK_TO_PTR(hMem, hMem->aback.largePtr);

		for(bit=(T_U8*)left; bit<(T_U8*)right; bit++)
		{
			if(*bit != hMem->wilder.wvar)
			{
				wildInfo->addr = bit;
				wildInfo->size = 0;
				wildInfo->line = 0;
				wildInfo->filename = "INHEAP";
				wildInfo->reqSize = 0;
				wildInfo->type = WILD_INVALID_INHEAP_WILD;

				Ram_Unlock(hMem->sys.dwLock);
				return AK_FALSE;
			}		
		}
	}

	Ram_Unlock(hMem->sys.dwLock);
	return AK_TRUE;
}


/**
 * @brief  enable memory leak auto trace mechnics
 * @只能用于状态机间内存泄漏自动检测
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_U8  stackDepth : push stack depth
 * @param   T_U16 sumCount : total memory wanted to auto check
 *
 * @return  AK_TRUE for auto ok else return false
 *
 */
T_BOOL Ram_EnableAutoLeakTrace(T_GLOBALMEMHandle hMemory, Ram_EnumMemTraceFun enumFun, T_U32 logCnt, Ram_EnterStateMachine *enterSection, Ram_LeaveStateMachine *leaveSection)
{
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 11))
		return AK_FALSE;	
	
	if(!enterSection || !leaveSection)
	{
		RAM_PRINTF(hMem)("Ram_EnableAutoLeakTrace Warning : you can't get callback hook function \r\n");
	}

	if(hMem->trace.bAutoTrace)
	{
		RAM_PRINTF(hMem)("Ram_EnableAutoLeakTrace Warning : already open \r\n");
		return AK_TRUE;	
	}

	Ram_Lock(hMem->sys.dwLock);

	//trace
	hMem->trace.logCnt = DEFAULT_AUTOTRACE_COUNT+DEFAULT_AUTOTRACE_STACK;
	if(logCnt >= DEFAULT_AUTOTRACE_STACK)
		hMem->trace.logCnt = logCnt+DEFAULT_AUTOTRACE_STACK;
	
	if(!(hMem->trace.mem = s_Ram_Alloc(hMemory, (sizeof(*hMem->trace.mem))*hMem->trace.logCnt, __FILE__, __LINE__)))
	{
		Ram_Unlock(hMem->sys.dwLock);
		RAM_PRINTF(hMem)("Ram_EnableAutoLeakTrace Error : alloc addr stack space  error \r\n");
		return AK_FALSE;
	}	

	memset(hMem->trace.mem, 0, sizeof(*hMem->trace.mem)*hMem->trace.logCnt);

	//YEAH
	hMem->trace.bAutoTrace = AK_TRUE;
	hMem->trace.enumFunc = enumFun;

	hMem->trace.enterSection = enter_statemachine;
	hMem->trace.leaveSection = leave_statemachine;
	
	//set hook for external
	if(enterSection && leaveSection)
	{
		(*enterSection) = enter_statemachine;
		(*leaveSection) = leave_statemachine;
	}

	Ram_Unlock(hMem->sys.dwLock);

	RAM_PRINTF(hMem)("Ram_EnableAutoLeakTrace : you're enable memory leak tracer \r\n");
	return AK_TRUE;
}

/**
 * @brief  disable memory leak auto trace mechnics
 * @只能用于状态机间内存泄漏自动检测
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_VOID
 *
 * @return  T_VOID
 *
 */
T_VOID Ram_DisableAutoLeakTrace(T_GLOBALMEMHandle hMemory)
{
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 12))
		return ;	

	if(AK_FALSE == hMem->trace.bAutoTrace)
	{
		RAM_PRINTF(hMem)("Ram_DisableAutoLeakTrace Warning : not open \r\n");
		return ;	
	}

	Ram_Lock(hMem->sys.dwLock);

	//trace
	hMem->trace.mem = s_Ram_Free(hMemory, hMem->trace.mem, __FILE__, __LINE__);

	//YEAH
	hMem->trace.bAutoTrace = AK_FALSE;
	hMem->trace.enumFunc = AK_NULL;
	hMem->trace.enterSection = AK_NULL;
	hMem->trace.leaveSection = AK_NULL;

	Ram_Unlock(hMem->sys.dwLock);

	RAM_PRINTF(hMem)("Ram_DisableAutoLeakTrace : you're disable memory leak tracer \r\n");
	return ;
}

/**
 * @brief  get global memory info
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	GLOBALMEMInfo *info : mem info
 *
 * @return  AK_FALSE for not initialize memory else return AK_TRUE
 *
 */
T_BOOL Ram_GetRamInfo(T_GLOBALMEMHandle hMemory, GLOBALMEMInfo *info)
{
	T_U32 pad;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 13))
		return AK_FALSE;;	
	
	if(!info)
	{
		RAM_PRINTF(hMem)("Ram_GetRamBlockLen Error : null input \r\n");
		return AK_FALSE;;	
	}
	
	Ram_Lock(hMem->sys.dwLock);

	info->align = hMem->setting.alen;
	info->szBlocks = hMem->heap.szBloks;
	info->szTotal = hMem->heap.szBloks << hMem->setting.align;
	info->blkUsed = hMem->heap.szBloks-hMem->stat.n_blocks;
	info->szUsed = info->blkUsed << hMem->setting.align;

	info->padblkBubble = (hMem->beyond.rlenpad+hMem->beyond.llenpad+hMem->wilder.wlenpad+LEN_EXTRA_BLOCK(hMem));
	info->padszBubble  = info->padblkBubble<<hMem->setting.align;
	info->cntBubble = hMem->stat.n_nSpare+hMem->stat.n_nUsed;

	pad = info->cntBubble*info->padblkBubble;
	pad <<= hMem->setting.align;

	info->szSpare = (info->szTotal-info->szUsed)+pad;

	Ram_Unlock(hMem->sys.dwLock);

	return AK_TRUE;
}

/**
 * @brief  get memory info
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	GLOBALMEMInfo *info : mem info
 *
 * @return  AK_FALSE for not initialize memory else return AK_TRUE
 *
 */
T_BOOL Ram_GetPtrInfo(T_GLOBALMEMHandle hMemory, T_pVOID var, T_MEMORY_TRACE *map, T_WILD_TYPE *wildType)
{
	Ram_Hdr *ptr;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 14))
		return AK_FALSE;	

	Ram_Lock(hMem->sys.dwLock);

	if(AK_FALSE == s_Ram_CheckPtr(hMemory, var, wildType))
	{
		Ram_Unlock(hMem->sys.dwLock);
		return AK_FALSE;
	}

	ptr = (Ram_Hdr*)PTR_TO_HDR(hMem, ((T_U8*)var));

	map->addr = HDR_TO_PTR(hMem, ptr);
	map->size = GET_MEM_SIZE(ptr) << hMem->setting.align;
	map->line = ptr->hdr.FileLine;
	map->filename = ptr->hdr.FileName;
	map->old = 0;
	map->reqSize = map->size-GET_MEM_PAD(ptr)-(HDR_MEM_EXTPAD(ptr)<<hMem->setting.align);

	Ram_Unlock(hMem->sys.dwLock);
	return AK_TRUE;
}

/**
 * @brief  要检测代码段的内存泄漏状况 : 开始位置
 * @请保持配对使用
 */
T_VOID  RAM_ENTER_LEAKCHK_SECTION(T_GLOBALMEMHandle hMemory)
{
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 15))
		return;		
	
	Ram_Lock(hMem->sys.dwLock);
	if(hMem->trace.enterSection)
		(*hMem->trace.enterSection)(hMemory);
	Ram_Unlock(hMem->sys.dwLock);
}


/**
 * @brief  要检测代码段的内存泄漏状况 : 结束位置
 * @请保持配对使用
 */
T_VOID	RAM_LEAVE_LEAKCHK_SECTION(T_GLOBALMEMHandle hMemory)
{
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 16))	
		return;	

	Ram_Lock(hMem->sys.dwLock);
	if(hMem->trace.leaveSection)
		(*hMem->trace.leaveSection)(hMemory);
	Ram_Unlock(hMem->sys.dwLock);

}

/**
 * @brief  get allocable largest memory bubble
 * @author  ZMJ
 * @date	6/24/2006 
 *
 * @param	T_GLOBALMEMHandle hMemory : memory allocator handler
 *
 * @return  allocable largest size
 *
 */
T_U32 Ram_GetLargestSize_Allocable(T_GLOBALMEMHandle hMemory)
{
	T_U32 szID;
	T_U32 maxID;
	T_U16 szExtra;
	Ram_Hdr *mptr;
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 17))
		return 0;	

	Ram_Lock(hMem->sys.dwLock);

	maxID = 0;

	//max mem in heap
	szID = hMem->aback.largePtr-hMem->ahead.allocPtr;
	szExtra = hMem->beyond.llenpad+hMem->beyond.rlenpad+hMem->wilder.wlenpad+LEN_EXTRA_BLOCK(hMem);
	if(szID>szExtra)
	{
		maxID = szID-szExtra;
	}

	//max mem in bigfree
	mptr=hMem->aback.misc;
	while(mptr && HDR_LOG_NEXT(mptr)) 
		mptr = HDR_LOG_NEXT(mptr);
	if(mptr)
	{
		szID = GET_MEM_SIZE(HDR_LOG_TO_PHY(hMem, mptr));

		if(szID > maxID)
			maxID = szID;
	}

	//max mem in smallfree
	mptr=hMem->ahead.misc;
	while(mptr && HDR_LOG_NEXT(mptr)) 
		mptr = HDR_LOG_NEXT(mptr);
	if(mptr)
	{
		szID = GET_MEM_SIZE(HDR_LOG_TO_PHY(hMem, mptr));

		if(szID > maxID)
			maxID = szID;
	}

	//max mem in smallfree array
	if(maxID < hMem->setting.lenFree)
	{
		//search maxID->lenfree
		for(szID=hMem->setting.lenFree-1; szID>maxID; szID--)
		{
			if(hMem->ahead.list[szID])
			{
				maxID = szID;
				break;
			}
		}
	}

	//------
	if(hMem->stat.n_blocks < maxID+szExtra)
	{
        if(hMem->stat.n_blocks > szExtra)
            maxID = hMem->stat.n_blocks-szExtra;  	    
	    else
		    maxID = 0;
	}

	Ram_Unlock(hMem->sys.dwLock);

	return (maxID<<hMem->setting.align);
}

/* =============================================================================*/

T_VOID	Ram_SetLock(T_GLOBALMEMHandle hMemory, T_U32 dwLock)
{
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 18))	
		return;	

	hMem->sys.dwLock = dwLock;
}

T_VOID	Ram_ClrLock(T_GLOBALMEMHandle hMemory)
{
	GLOBALMEMHandle *hMem = (GLOBALMEMHandle*)hMemory;

	if(!CHECK_MEM_LIB(hMem, 19))	
		return;	

	hMem->sys.dwLock = 0;
}

T_VOID Ram_GetDump(T_U8 **buf, T_U32 *len)
{
//	*buf = dbgMem;
//	*len = dbgCnt*sizeof(T_U32);
}

