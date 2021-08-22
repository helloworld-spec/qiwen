/*
 * @(#)List.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _LIST_H_
#define        _LIST_H_
#include "object.h"

typedef struct List
{
    T_OBJECT    object;
    T_U32       length;
    T_U32       capacity;
    T_POBJECT*  data;
}T_LIST, *T_PLIST;

T_VOID List_Destroy(T_PLIST obj);
T_BOOL List_Initial(T_PLIST obj, T_U32 capacity);
T_U32 List_Search(T_PLIST obj, T_POBJECT item);
T_POBJECT List_Get(T_PLIST obj, T_U32 pos);
T_BOOL List_Delete(T_PLIST obj, T_U32 pos);
T_BOOL List_DelRef(T_PLIST obj, T_pVOID item);
T_BOOL List_Add(T_PLIST obj, T_POBJECT item);
T_U32 List_Length(T_PLIST obj);
T_POBJECT *List_GetData(T_PLIST obj);


#endif

