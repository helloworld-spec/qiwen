/*
 * @(#)fdt.h
 * @date 2010/07/12
 * @version 1.0
 * @author Lu_Qiliu.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _FDT_H_
#define        _FDT_H_

T_U8 *Fdt_GetName(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 start);
T_VOID Fdt_CmpName(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 start);
T_VOID Fdt_CmpClus(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 start);
T_VOID Fdt_GetAll(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 start);
T_VOID Fdt_GetSpace(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 start);
T_VOID Fdt_SetName(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 start);
T_VOID Fdt_Delete(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 start);
T_VOID Fdt_CmpShort(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 start);
T_U8 Fdt_GetFileAttr(const T_U8 *ptrr);
T_U8 *Fdt_GetPreBuf(T_PDRIVER driver, T_PFATSEARCH sm);
T_U8 *Fdt_GetNextBuf(T_PDRIVER driver, T_PFATSEARCH sm);
T_U8 *Fdt_GetNextFileInfo(T_PDRIVER driver, T_PFATSEARCH sm, T_U8 *buf1,T_PFILEINFO FileAttr, T_U32 *chkdsk_sum);
T_U8 *Fdt_GetPreFileInfo(T_PDRIVER driver, T_PFATSEARCH sm, T_U8 *buf1,T_PFILEINFO FileAttr);
T_BOOL Fdt_Set_Volume(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 start);
T_U8 *Fdt_Get_Volume(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 start);
#endif //_FDT_H_


