/*
 * @(#)Global.h
 * @date 2009/11/11
 * @version 1.0
 * @author Lu Qiliu.
 * Copyright 2009 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _GLOBAL_OLD_H_
#define        _GLOBAL_OLD_H_

#include    <string.h>
#include    "file.h"
#include    "mtdlib.h"
#include    "link.h"
#include    "fat.h"

/* Jan.17,07 - added to support Multi-language */
extern T_U8 g_DelFlag;

#define DRIVER_BUFFER_LEN (100 * 1024)
/* Added by Multi-lang: the max len of a FileName(Maybe including it's path) */
#define SHORT_TO_LONG_BUFF_LEN 15
#define ONE_UNIC_ASC_BUFF_LEN  4      /* the buff prepared for one character conversion */

typedef enum tag_E_AKCHIP_FS
{
    FS_AK32XX = 0,
    FS_AK36XX = 1,
    FS_AK38XX = 2,
    FS_AK78XX = 3,
    FS_AK1036 = 4,
    FS_AK1080 = 5,
    FS_SUNDANCEIIA = 6,
    FS_AK98XX = 7,
    FS_AK37XX = 8,
    FS_AK11XX = 9,
    FS_AK1080L = 10,
    FS_AK10XXC = 11,
    FS_AK10XXT = 12,
    FS_AK37XX_C = 13,
    FS_AK39XX_E = 14, //H2
    FS_AK10XX_T2 = 15,
    FS_AK39XX_H240 = 16,
	FS_AK39XX_H3 = 17,
	FS_AK10XX_T3 = 18,
    FS_AKERR  = 255
}E_AKCHIP_FS;
typedef struct tag_SysDrivers
{
    T_U8 Indx;  // Corresponding to the driver, such as 0:A, 1:B, 25:Z.
    T_U8 type;  // Corresponding to E_DRVTYPE 
}T_SYSDRV, *T_PSYSDRV;
typedef enum tag_E_DriverType
{
    DRVTYPE_NAND = 0,
    DRVTYPE_SD   = 1,

    DRVTYPE_ERR  = 255
}E_DRVTYPE;

typedef T_VOID (*F_OutStream)(T_U16 ch);
typedef T_U8 (*F_Instream)(T_VOID);
typedef T_U32 (*F_GetSecond)(T_VOID);
typedef T_VOID (*F_SetSecond)(T_U32 seconds);
/* Jan.10,07 - Modified to support Multi-Language */
typedef T_S32 (*F_UniToAsc)(const T_U16 *pUniStr, T_U32 UniStrLen,
                            T_pSTR pAnsibuf, T_U32 AnsiBufLen, T_U32 code);
typedef T_S32 (*F_AscToUni)(const T_pSTR pAnsiStr, T_U32 AnsiStrLen,
                            T_U16 *pUniBuf, T_U32 UniBufLen, T_U32 code);

typedef T_pVOID    (*F_RamAlloc)(T_U32 size, T_S8 *filename, T_U32 fileline);
typedef T_pVOID    (*F_RamRealloc)(T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline); 
typedef T_pVOID (*F_RamFree)(T_pVOID var, T_S8 *filename, T_U32 fileline);

/* Sep.13,07 - Added to support Muti-Task. */
typedef T_S32 (*F_OsCrtSem)(T_U32 initial_count, T_U8 suspend_type, T_S8 *filename, T_U32 fileline);
typedef T_S32 (*F_OsDelSem)(T_S32 semaphore, T_S8 *filename, T_U32 fileline);
typedef T_S32 (*F_OsObtSem)(T_S32 semaphore, T_U32 suspend, T_S8 *filename, T_U32 fileline);
typedef T_S32 (*F_OsRelSem)(T_S32 semaphore, T_S8 *filename, T_U32 fileline);

typedef T_U32 (*F_GetChipID)(T_VOID);

typedef T_pVOID (*F_MemCpy)(T_pVOID dst, T_pCVOID src, T_U32 count);
typedef T_pVOID (*F_MemSet)(T_pVOID buf, T_S32 value, T_U32 count);
typedef T_pVOID (*F_MemMov)(T_pVOID dst, T_pCVOID src, T_U32 count);
typedef T_S32   (*F_MemCmp)(T_pCVOID buf1, T_pCVOID buf2, T_U32 count);
typedef T_S32   (*F_Printf)(T_pCSTR s, ...);


//#ifdef ROM_FS 
typedef T_U32   (*F_DATADIVIDE)(T_U32 soure, T_U32 dest);
typedef T_U32   (*F_DATAOFFSET)(T_U32 soure, T_U32 dest);
typedef T_VOID   (*F_RomSetLastErrorType)(exfat_error_type error_type);
typedef T_U32   (*F_FatWriteData)(T_PFILE file);

//#endif

 
typedef struct tag_Global
{
    T_PLIST MediumList;
    T_PLIST DriverList;
    T_U8  CurDriver;    //0=a,1=b,2=c...
    T_U32 ReadCount;
    T_U32 WriteCount;
    T_U32 SecondCount;
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
    F_MemCpy   fMemCpy;
    F_MemSet   fMemSet;
    F_MemMov   fMemMove;
    F_MemCmp   fMemCmp;
    F_Printf   fPrintf;

    F_GetChipID fGetChipId;
}T_FSGLOBAL;


//#ifdef ROM_FS 
typedef struct tag_RomFsCb
{
   
    F_RamAlloc  			fRamAlloc;
    F_RamRealloc 			fRamRealloc;
    F_RamFree  				fRamFree;
    F_MemCpy   				fMemCpy;
    F_MemSet   				fMemSet;
	F_MemCmp   				fMemCmp;
    F_DATADIVIDE   			fDataDivide;
	F_DATAOFFSET   			fDataOffset;
    F_RomSetLastErrorType   fRomSetLastErrorType;
	F_FatWriteData 			fFatWriteData;
}T_ROMFSCB;
extern T_ROMFSCB romFsCb;
//#endif



extern T_FSGLOBAL fgb;



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
T_U32 Global_GetDriverCount(T_SYSDRV OutInfo[], T_U32 ArrLen);


#endif       //_GLOBAL_API_H_

