/*
 * @(#)fmsg.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _FMSG_H_
#define        _FMSG_H_

#include    "cluster.h"
#include    "attr.h"


typedef struct FatFileMsg
{
    T_OBJECT obj;
    T_PCLUSTER clus;    //data cluster link.
    T_U32 fdt;  //file directory entry in parent.
}T_FATFILEMSG, *T_PFATFILEMSG;

T_PFATFILEMSG Fmsg_Initial(T_PFATFILEMSG fmsg);
T_VOID Fmsg_Destroy(T_PFATFILEMSG obj);
#endif

