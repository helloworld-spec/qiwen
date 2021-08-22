/*
 * @(#)String.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _USTRING_H_
#define        _USTRING_H_

typedef struct    tag_String
{    
    T_OBJECT object;  
    T_U16    HashValue;
    T_U16    capacity;   // The data can contain how many characters.
    T_U32    ptr;        // length(the number of characters): include zero.
    T_U16   *data;        
}T_STRING, *T_PSTRING;

T_VOID Str_Destroy(T_PSTRING str);
T_PSTRING Str_Copy(T_PSTRING str);
T_PSTRING Str_Combine(T_PSTRING first, T_PSTRING second);
T_PSTRING Str_Initial(T_U32 count);
T_PSTRING Str_UnicodeCount(const T_U16* buf, T_U32 count);
T_PSTRING Str_AscCode(const T_VOID* asc, T_U32 code);
T_BOOL Str_Add(T_PSTRING *obj, T_PSTRING other);
T_VOID Str_CutTail(T_PSTRING obj);
T_U32 Str_Length(T_PSTRING obj);
T_U16* Str_GetData(T_PSTRING obj);
T_U32 Str_GetBytes(T_PSTRING obj, T_U8* dst, T_U32 code);
int Str_Cmp(T_PSTRING dest, T_PSTRING source);
T_U8 Str_GetUnicodeLen(const T_U16* buf);

#endif

