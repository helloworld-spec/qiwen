/*
 * @(#)fname.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _FNAME_H_
#define        _FNAME_H_

#define SHORT_NAME_MAX_LEN  12  //include '.'

typedef enum
{
    FNAME_SHORT         = 0,
    FNAME_LONG          = 1,
    FNAME_UPLOW         = 2,
    FNAME_SHORT_ALLLOW  = 3,
    FNAME_SHORT_EXTLOW  = 4,
    FNAME_SHORT_NAMELOW = 5,

    FNAME_ERROR         = 255
}E_NAMESPACE;

E_NAMESPACE Fname_GetSpace(const T_U16 *FileName);

T_U8 Fname_ChkSum(const T_U8* name);

//if == name, return name's number. must < min or > max.
//if dst != name, set min, and max.
T_BOOL Fname_RepeatCheck(T_U8 *dst, T_U8 *name, T_U32* min, T_U32* max, T_U32 *num);
T_S32 Fname_CompLongName(T_U16 *LongName, const T_U8 *fdt);
T_U32 Fname_FilterWin32(const T_U16* str);

T_VOID Fname_ShortToLong(T_U8 *ShortName, T_U8 NTResType, T_U16* LongName, T_U32 code);
/* 16th.Apirl ,08 - The return value is file name's length. */
T_U32 Fname_LongToShort(const T_U16* FileName, T_U8 *ShortName, T_U32 code);

#endif

