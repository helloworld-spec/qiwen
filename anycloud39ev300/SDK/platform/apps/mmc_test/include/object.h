/*
 * @(#)Object.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _OBJECT_H_
#define        _OBJECT_H_
#include    "anyka_types.h"
#include    "mtdlib.h"

T_BOOL Object_IsKindsOf(T_POBJECT object);
T_VOID Object_Destroy(T_POBJECT object);
T_VOID Object_DestroyList(T_POBJECT* list, T_U32 count);
T_U32 Long_Search(const T_U32 *buf, T_U32 l, T_U32 count);
T_U32 Long_ToDigit(T_U32 num, T_U8 buf[20]);
T_VOID Long_ToHex(T_U32 num, T_U8 buf[12]);

#endif

