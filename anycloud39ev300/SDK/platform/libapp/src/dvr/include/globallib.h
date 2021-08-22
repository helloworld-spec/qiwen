/*
 * @(#)Global.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _GLOBAL_H_
#define        _GLOBAL_H_

#include    "link.h"
#include    "list.h"
#include    "blink.h"
#include    "filelib.h"



#ifdef WIN32
#include "stdio.h"
#endif


// FOR SD + NANDFLASH
//A: + B: + C: + D: + E:


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
    FS_AKERR  = 255
}E_AKCHIP_FS;

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


/*
typedef struct search_dir
{
    T_U32        DriverId;
    T_PFILEINFO  attr;
    T_FILTER     filter;
    T_PBLINK     child;  //FileInfo link.
    T_PBLINKITEM ptr;
}T_SEARCH, *T_PSEARCH;
*/

T_VOID Global_OutString(const T_U8* str);
T_VOID Global_OutStrDA(const T_U8* str);
T_VOID Global_OutUniString(const T_U16* str);
T_VOID Global_OutUniStrDA(const T_U16* str);
T_VOID Global_OutHex(T_U32 hex);
T_VOID Global_OutStrHex(const T_U8* str, T_U32 hex);
T_VOID Global_OutDigit(T_U32 digit);
T_VOID Global_OutCount(T_VOID);
T_U8* Global_InString(T_U8* InputStr);
T_U32 Global_GetSecond(T_VOID);
T_VOID Packet_Dump(T_U8* buf, T_U32 size);
T_BOOL Global_AddMediumToList(T_PMEDIUM medium);
T_BOOL Global_RefMediumFromList(T_PMEDIUM medium);

#endif

