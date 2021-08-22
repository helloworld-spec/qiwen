/*
 * @(#)link.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _LINK_H_
#define        _LINK_H_
#include    "object.h"

typedef struct LinkItem *T_PLINKITEM;
typedef struct LinkItem  T_LINKITEM;
struct LinkItem
{
    T_POBJECT      data;
    T_PLINKITEM    prev;
    T_PLINKITEM    next;
};

typedef struct Link
{
    T_OBJECT object;
    T_PLINKITEM    head;
    T_PLINKITEM    tail;
    T_PLINKITEM    ptr;
}T_LINK, *T_PLINK;

T_VOID Link_Destroy(T_PLINK obj);
T_VOID Link_Initial(T_PLINK obj);
T_BOOL Link_Insert(T_PLINK obj, T_POBJECT item);

#endif

