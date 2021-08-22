/*
 * @(#)Unicode.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef    _UNICODE_H_
#define    _UNICODE_H_

#include "anyka_types.h"

T_U32 Uni_AscZero(T_U16* UniBuf, T_U8* asc, T_U32 code);
T_U32 Uni_GetBytes(const T_U16* UniBuf, T_U8* dst, T_U32 code);

T_U32 Uni_CharSearch(const T_U16 *buf, T_U16 w, T_U32 count);
T_U32 Uni_AscSearch(const T_U8 *buf, T_U8 b, T_U32 count);
T_U32 Uni_UniSearch(const T_U16* str, T_U16 ch, T_U32 count);
T_U16 Uni_ToUpper(T_U16 uni);
int Uni_CharCmp(const T_U16* w1, const T_U16* w2, T_U32 count);
int Uni_CmpIgnoreCase(const T_U16 *w1, const T_U16 *w2, T_U32 count);

#endif




