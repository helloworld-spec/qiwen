/*
 * @(#)Global.h
 * @date 2009/11/11
 * @version 1.0
 * @author Lu Qiliu.
 * Copyright 2009 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _GLOBAL_API_H_
#define        _GLOBAL_API_H_

#include    <string.h>
#include    "file.h"
#include    "mtdlib.h"
/* Jan.17,07 - added to support Multi-language */
extern T_U8 g_DelFlag;

#define DRIVER_BUFFER_LEN (100 * 1024)



typedef struct tag_FsInitInfo
{
    F_OutStream out;
    F_Instream  in;
    F_GetSecond fGetSecond;
    F_SetSecond fSetSecond;
    F_UniToAsc  fUniToAsc;
    F_AscToUni  fAscToUni;
    F_RamAlloc  fRamAlloc;
    F_RamRealloc fRamRealloc;
    F_RamFree  fRamFree;
    F_OsCrtSem fCrtSem;
    F_OsDelSem fDelSem;
    F_OsObtSem fObtSem;
    F_OsRelSem fRelSem;

    F_MemCpy  fMemCpy;
    F_MemSet  fMemSet;
    F_MemMov  fMemMov;
    F_MemCmp  fMemCmp;
    F_Printf  fPrintf;

    F_GetChipID fGetChipId;
}T_FSINITINFO, *T_PFSINITINFO;

T_VOID Global_Initial(T_PFSINITINFO fsInitInfo);
T_VOID Global_Destroy(T_VOID);
T_BOOL Global_DriverAvail(T_U8 DriverID);
T_VOID Global_UninstallDriver(T_U32 driver);
T_VOID Global_SetDelFlag(T_U8 delFlag);
T_VOID Global_SetDriversEcode(T_U32 Code);
T_PMEDIUM Global_GetMedium(T_U32 driver);
T_VOID Global_UninstallMedium(T_PMEDIUM medium);
T_BOOL Global_CheckFileErrBySec(T_U8 DriverID, T_U32 sec);
T_BOOL Global_MountCacheToDriver(T_PMEDIUM DstMedium, T_PMEDIUM SrcMedium);

#endif       //_GLOBAL_API_H_

