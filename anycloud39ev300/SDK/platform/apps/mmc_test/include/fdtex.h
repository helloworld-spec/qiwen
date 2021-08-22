 /*
 * @(#)fdtex.h
 * @date 2010/07/12
 * @version 1.0
 * @author Lu_Qiliu.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _FDTEX_H_
#define        _FDTEX_H_
#include "file.h"
#define DEL_FLAG_EXFAT  0X80
#define MAIN_FDT_FLAG   0X85
#define VOL_FDT_FLAG   0X83
#define MAP_FDT_FLAG   0X81
#define UPCASE_FDT_FLAG   0X82
#define GUID_FDT_FLAG   0XA0
#define EX_FDT_FLAG     0XC0
#define NAME_FDT_FLAG     0XC1
#define NAME_LEN_FDT    15

T_U8 *Fdtex_GetNextFile(T_PDRIVER driver, T_PFATSEARCH sm, T_U8 *buf1, T_PFILEINFO FileAttr, T_U32 *chkdsk_sum);
T_U8 *Fdtex_GetPreFile(T_PDRIVER driver, T_PFATSEARCH sm, T_U8 *buf1, T_PFILEINFO FileAttr);
T_BOOL Fdtex_SetNameAttr(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 LongPrev);
T_U8 Fdtex_GetFileAttr(const T_U8 *ptr);
T_BOOL Fdtex_CheckShortMatch(T_PDRIVER driver, T_PFATSEARCH sm, T_PFILEINFO FileAttr);
T_BOOL Fdtex_CheckLongMatch(T_PDRIVER driver, T_PFATSEARCH sm, T_U8 **buf);
T_BOOL Fdtex_WriteDelFlag(T_PFATSEARCH sm);
T_U16 Fdtex_GetFileNameHash( T_U16 *FileName, T_U16 length);
T_BOOL Fdtex_SetAttrFromEXFDTName(T_U8 *ptr, T_PATTR attr, T_U16 id);
T_VOID Fdtex_SetAttrFromEXFDT(T_U8 *ptr, T_PATTR attr);
T_VOID Fdtex_SetAttrFromMainFDT(T_U8 *ptr, T_PATTR attr);
T_VOID Fdtex_SetAttrFromVolumeFDT(T_U8 *ptr, T_PATTR attr);

#endif //_FDTEX_H_

