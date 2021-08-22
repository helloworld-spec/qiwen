/*
 * @(#)driver.h
 * @date 2009/11/11
 * @version 1.0
 * @author Lu Qiliu.
 * Copyright 2009 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _DRIVER_H_
#define        _DRIVER_H_

#include "mtdlib.h"
#include "file.h"

typedef enum
{
    FS_SEARCH_FILE    = 0,
    FS_SEARCH_FOLDER  = 1,
    FS_SEARCH_NOEXIST = 2,
    FS_SEARCH_OKEXIST = 3,
    FS_SEARCH_UPLOW   = 4,

    FS_SEARCH_ERROR   = 255
}E_FSSEARCH;

typedef T_BOOL (*F_GetDriverCallback)(T_VOID);


T_VOID Driver_ClearBuf(T_U8 DriverId);
T_U32 Driver_Initial(T_PMEDIUM medium, T_U32 BufLen);
T_BOOL Global_MountDriver(T_U32 driver, T_U8 DriverID);
T_BOOL Driver_Format(T_U8 DriverID, T_U32 BufLen, E_FATFS FsType);
T_BOOL Driver_QuickFormat(T_U8 DriverID);
T_U32 Driver_GetCapacity(T_U8 DriverID, T_U32 *high);
T_U32 Driver_GetUsedSize(T_U8 DriverID, T_U32 *high);
/* Get free size of the driver. high: output para to store the high T_U32 part. */
T_U32 Driver_GetFreeSize(T_U8 DriverID, T_U32 *high);
T_U8  Driver_GetDriverIDUnicode(const T_U16 *path);
T_U8  Driver_GetDriverIDAsc(const T_U8 *path);
T_BOOL Driver_Format_capacity(T_U8 DriverID, T_U32 BufLen, E_FATFS FsType, T_U32 SecCnt);
T_U32 Driver_Initial_CallBack(T_PMEDIUM medium, T_U32 BufLen, F_GetDriverCallback pGet_Griver);


#endif       //_DRIVER_H_

