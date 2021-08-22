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
#include "Fwl_osMalloc.h"
#include "mem_api.h"

#ifdef CHIP_37XX_37CW
#define    MAX_RAMBUFFER_SIZE        (512*1024)
#else
#define    MAX_RAMBUFFER_SIZE        (2048*1024)
#endif

static unsigned char gb_RAMBuffer[MAX_RAMBUFFER_SIZE];

static T_GLOBALMEMHandle        gb_hGlobalMemory = NULL;

/***********************static function************************/

void Fwl_MallocInit(void)
{  
    gb_hGlobalMemory = Ram_Initial(gb_RAMBuffer, sizeof(gb_RAMBuffer));    
}

void * Fwl_MallocAndTrace(unsigned long size, unsigned char * filename, unsigned long line)
{
    void * ptr;
    
    ptr = Ram_Alloc(gb_hGlobalMemory, size, filename, line);

    return ptr;
}

void *    Fwl_ReMallocAndTrace(void * var, unsigned long size, unsigned char * filename, unsigned long line)
{
    void * ptr;

    ptr = Ram_Realloc(gb_hGlobalMemory, var, size, filename, line);

     return ptr;
}

void * Fwl_FreeAndTrace(void * var, unsigned char * filename, unsigned long line) 
{
    return Ram_Free(gb_hGlobalMemory, var, filename, line); //一些野指针释放时需要调试信息
}  

/* debug function : should merge into one */
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

void * Fwl_Malloc(unsigned long size)
{
   return Fwl_MallocAndTrace((size), ((signed char*)(__FILE__)), ((unsigned long)__LINE__));
}

void * Fwl_ReMalloc(void * var, unsigned long size)
{
    return Fwl_ReMallocAndTrace((var), (size), ((signed char*)(__FILE__)), ((unsigned long)__LINE__));
}

void * Fwl_Free(void * var)
{
    return Fwl_FreeAndTrace(var, ((signed char*)(__FILE__)), ((unsigned long)__LINE__));
}


void  Ram_Lock(unsigned long dwLock)
{

}

void  Ram_Unlock(unsigned long dwLock)
{
}


