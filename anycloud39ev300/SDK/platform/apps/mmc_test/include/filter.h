/*
 * @(#)filter.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _FILTER_H_
#define        _FILTER_H_
#include    "object.h"

typedef T_BOOL (*F_AttrFilter)(T_POBJECT attr);

typedef struct tag_Filter
{
    T_U16 type;
    F_AttrFilter filter;
    T_U16 pattern[512];
}T_FILTER, *T_PFILTER;

T_BOOL Filter_Match(T_U16* s1, T_U16* s2, T_U32 n);


#endif

