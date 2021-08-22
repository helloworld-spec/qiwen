/************************************************************************
 * Copyright (c) 2001, Anyka Co., Ltd. 
 * All rights reserved.    
 *  
 * File Name：Fwl_osMalloc.c
 * Function：This header file is API for Memory Library
 *
 * Author：ZhangMuJun
 * Date：
 * Version：2.0.1
 *
 * Reversion: 
 * Author: 
 * Date: 
**************************************************************************/
#include "os_malloc.h"
#include "mem_api.h"
#include "anyka_types.h"
#include "anyka_bsp.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "akos_api.h"


extern int __bss_end;

#define MALLOC_MEM_ENTRY ((unsigned long)&__bss_end + 4)
#define MALLOC_MEM_SIZE  (SVC_MODE_STACK - 8*1024 - (unsigned long)MALLOC_MEM_ENTRY)

enum 
{
	MALLOC_NOT_INIT,
	MALLOC_INITING,
	MALLOC_INIT_OK
	
};

static long m_iInitFlag=MALLOC_NOT_INIT;
static T_GLOBALMEMHandle gb_hGlobalMemory;


static long Ram_Print(const signed char  *s, ...);
static void Fwl_MallocSupportMultithread(void);


static long Ram_Print(const  signed char *s, ...)
{
	long		len=0;
	
	
	return len;
}
///////////////////////////////////////////////////////////////////////////////
//Ram_Lock 和Ram_Unlock 是内存库要求必须实现的函数
void  Ram_Lock(unsigned long dwLock)
{
    if(0 == dwLock)
    return;
    AK_Obtain_Semaphore((T_hSemaphore)dwLock, AK_SUSPEND);
}

void  Ram_Unlock(unsigned long dwLock)
{


    if(0 == dwLock)
    return;
    AK_Release_Semaphore((T_hSemaphore)dwLock);
}
///////////////////////////////////////////////////////////////////////////////////

void Fwl_MallocInit(void)
{
	
	if (MALLOC_NOT_INIT != m_iInitFlag)  //already ini
		return ;

    m_iInitFlag =MALLOC_INITING;//WHEN IS INITING , CAN NOT DO MALLOC OR INIT .
    gb_hGlobalMemory = Ram_InitialEx((unsigned char *)MALLOC_MEM_ENTRY, MALLOC_MEM_SIZE, 
                        0, 16/*0*/, 16, 0xFC, 0xCF, //内存越界检测参数指定
                       0x10/*0*/, 0xCC,         //野操作检测参数
                       16, 11200, 85, 2,   //高级分配器优化参数                   
                       (Ram_PrintfTraceFun)Ram_Print);
                       
    m_iInitFlag =MALLOC_INIT_OK;

    Fwl_MallocSupportMultithread();

}

static void Fwl_MallocSupportMultithread(void)
{
    T_hSemaphore     akos_mutex = 0;

    akos_mutex = AK_Create_Semaphore(1, AK_PRIORITY);
    Ram_SetLock(gb_hGlobalMemory, akos_mutex);
}

void * Fwl_MallocAndTrace(unsigned long size, char * filename, unsigned long line)
{
    void * ptr;

    if ( MALLOC_NOT_INIT == m_iInitFlag)
    {
    	Fwl_MallocInit();
    	m_iInitFlag = MALLOC_INIT_OK;
    }else if (MALLOC_INITING == m_iInitFlag)
    	return NULL;
    

    if (0 == size)
    {
        return NULL;
    }
    
    ptr = Ram_Alloc(gb_hGlobalMemory, size, filename, line);

    return ptr;
}

void * Fwl_CallocAndTrace(unsigned long num,unsigned long size, char * filename, unsigned long line)
{
    void *p;
    
	p = Fwl_MallocAndTrace(num * size , filename,line);
	if (p)
	    memset(p, 0, num*size);
	return p;
}

void *    Fwl_ReMallocAndTrace(void * var, unsigned long size, char * filename, unsigned long line)
{
    void * ptr;

    if ( MALLOC_NOT_INIT == m_iInitFlag)
    {
    	Fwl_MallocInit();
    	m_iInitFlag = MALLOC_INIT_OK;
    }else if (MALLOC_INITING == m_iInitFlag)
    	return NULL;
   	

    if (0 == size)
    {
        return NULL;
    }
    
    ptr = Ram_Realloc(gb_hGlobalMemory, var, size, filename, line);

    return ptr;
}

void * Fwl_FreeAndTrace(void * var, char * filename, unsigned long line) 
{
    if ( MALLOC_INIT_OK != m_iInitFlag) //not init , return directly
    {
    	return NULL; 
    }

    return Ram_Free(gb_hGlobalMemory, var, filename, line); //一些野指针释放时需要调试信息
}  


/////////////////////////////////////////////////////////////////////////////////////
/* debug function : should merge longo one */
unsigned long Fwl_GetTotalRamSize(void)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(gb_hGlobalMemory, &info);

    return info.szTotal;
}

unsigned long Fwl_RamUsedBlock(void)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(gb_hGlobalMemory, &info);

    return info.blkUsed;
}

unsigned long Fwl_GetUsedRamSize(void)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(gb_hGlobalMemory, &info);

    return info.szUsed;
}

unsigned long Fwl_RamGetBlockNum(void)
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(gb_hGlobalMemory, &info);

    return info.szBlocks-info.blkUsed;
}

unsigned long Fwl_RamGetBlockLen()
{
    GLOBALMEMInfo info;
    Ram_GetRamInfo(gb_hGlobalMemory, &info);

    return info.align;
}

unsigned long Fwl_GetRemainRamSize(void)
{
     GLOBALMEMInfo info;
     Ram_GetRamInfo(gb_hGlobalMemory, &info);

     return info.szSpare;
}


void    Fwl_RamLeakMonitorPolongBeg(void)
{
#ifdef DEBUG_TRACE_MEMORY_LEAK   
    Ram_EnableAutoLeakTrace(gb_hGlobalMemory, PrlongOneMemoryLeakTraceInfo, 4000, NULL, NULL);
    RAM_ENTER_LEAKCHK_SECTION(gb_hGlobalMemory);
#endif
}

void    Fwl_RamLeakMonitorPolongEnd(void)
{
#ifdef DEBUG_TRACE_MEMORY_LEAK    
    RAM_LEAVE_LEAKCHK_SECTION(gb_hGlobalMemory);
    Ram_DisableAutoLeakTrace(gb_hGlobalMemory);
#endif
}

bool Fwl_CheckPtr(void * var)
{
	T_MEMORY_TRACE map;
	T_WILD_TYPE wildType;

	if(false == Ram_GetPtrInfo(gb_hGlobalMemory, var, &map, &wildType))
		{
			return false;
		}

	return true;
}

unsigned long Fwl_GetLargestSize_Allocable(void)
{
	return Ram_GetLargestSize_Allocable(gb_hGlobalMemory);
}

