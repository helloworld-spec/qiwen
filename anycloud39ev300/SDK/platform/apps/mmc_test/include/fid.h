/*
 * @(#)fid.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _FID_H_
#define        _FID_H_
#include    "object.h"

typedef struct tag_FidData
{
    T_U32 FileId;    //file id, it's first cluster of file in FAT file system.
    T_U32 fdt;        //the file sequence number in parent folder.
}T_FIDDATA, *T_PFIDDATA;

typedef struct tag_FidObj
{
    T_OBJECT obj;
    T_FIDDATA  data;
}T_FID, *T_PFID;

T_PFID Fid_Initial(T_VOID);

#endif

