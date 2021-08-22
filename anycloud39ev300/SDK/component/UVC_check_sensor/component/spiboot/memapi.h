/*
MODELNAME:memapi.h
DISCRIPTION:simple memery manage
AUTHOR:WangXi
DATE:2011-02-15
*/

#ifndef __MEMAPI_H__
#define __MEMAPI_H__

#include "anyka_types.h"


T_VOID	Init_MallocMem(T_U32 MallocAddr_Start,T_U32 MallocAddr_End);
T_VOID  FreeMem(T_VOID* ptr);
T_VOID *AllocMem(T_U32 size);
#define MALLOC(size)  AllocMem(size)
#define FREE(ptr)     FreeMem(ptr)
#endif

