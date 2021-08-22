/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.	
 *  
 * File Name：Fwl_osMalloc.h
 * Function：This header file is API for Memory Library
 *
 * Author：ZMJ
 * Date：
 * Version：
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/
#ifndef __FWL_OS_MALLOC_H__
#define __FWL_OS_MALLOC_H__

#include "anyka_types.h"


void Fwl_MallocInit(void);

void * Fwl_MallocAndTrace(unsigned long size, unsigned char * filename, unsigned long line);
void *	Fwl_ReMallocAndTrace(void * var, unsigned long size, unsigned char * filename, unsigned long line);
void * Fwl_FreeAndTrace(void * var, unsigned char * filename, unsigned long line); 

void * Fwl_Malloc(unsigned long size);

void * Fwl_ReMalloc(void * var, unsigned long size);
void * Fwl_Free(void * var);

#define AK_MALLOCRECORD(type)  (type *)Fwl_Malloc(sizeof(type))



/* debug function */ //should merge to one
unsigned long Fwl_GetTotalRamSize(void);
unsigned long Fwl_RamUsedBlock(void);
unsigned long Fwl_GetUsedRamSize(void);
unsigned long Fwl_RamGetBlockNum(void);
unsigned long Fwl_RamGetBlockLen(void);
unsigned long Fwl_GetRemainRamSize(void);
unsigned long Fwl_GetLargestSize_Allocable(void);

bool Fwl_CheckPtr(void * var);
unsigned long  Fwl_ReleaseMemory(void);

/* NOTE:
   every body, every library, we'll should do like below:

	#ifdef 	DEBUG_TRACE_MEMORY_LEAK
	#define Fwl_Malloc(size)		Fwl_MallocAndTrace((size), ((signed char*)(__FILE__)), ((unsigned long)__LINE__))
	#define Fwl_ReMalloc(var, size) Fwl_ReMallocAndTrace((var), (size), ((signed char*)(__FILE__)), ((unsigned long)__LINE__))
	#define Fwl_Free(var)			Fwl_FreeAndTrace((var), ((signed char*)(__FILE__)), ((unsigned long)__LINE__))		
	#else
	#define Fwl_Malloc(size)		Fwl_MallocAndTrace((size), NULL, 0)
	#define Fwl_ReMalloc(var, size) Fwl_ReMallocAndTrace((var), (size), NULL, 0)
	#define Fwl_Free(var)			Fwl_FreeAndTrace((var), NULL, 0)
	#endif   

#endif
*/

/* NOTE:
   below is define in Fwl_osMalloc.c  //for shorten complier time


#ifndef ENABLE_MEMORY_DEBUG
	//#define ENABLE_MEMORY_DEBUG	//内存越界自动监测器
#endif
*/
void	Fwl_RamBeyondMonitorGetbyTimer(unsigned long LID);	//检测内存越界状况
void  Fwl_RamWilderMonitorGetbyTimer(unsigned long LLD);  //检测内存野指针状况
void Fwl_RamBeyondMonitor(unsigned long LLD);
void  Fwl_RamWilderMonitor(unsigned long LLD);

void	Fwl_RamLeakMonitorHooktoSM(void);		//使用状态机钩子来跟踪内存泄漏
void	Fwl_RamLeakMonitorPointBeg(void);		//打开检测该行以下的代码行内存泄漏
void	Fwl_RamLeakMonitorPointEnd(void);     //关闭检测该行以上的代码行内存泄漏
void  Fwl_RamEnumerateEachSeg(void);		//枚举所有的内存块


#endif
