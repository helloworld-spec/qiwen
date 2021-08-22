/*
 * @(#)Driverlib.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _DRIVER_LIB_H_
#define        _DRIVER_LIB_H_

#include "driver.h"
#include "list.h"
#include "blink.h"

typedef enum
{
    FS_FAT  = 0,
    FS_NTFS = 1,
    FS_HFS  = 2,
    FS_EXT  = 3,

    FS_ERR  = 255
}E_FS;

#define INVALID_ADDR 0xFFFFFFFF
#define DRIVER_ID_MAX 26

typedef struct tag_DrvBufCtrl
{
    T_U32 Addr;     // The buffer's corresponding sector.
    T_U16 ReadCnt;  // How many functions which are using the buffer at the time.
    T_U16 RefTime;  // How many times the buffer has been read without being overlayed.
}T_DRVBUFCTRL, *T_PDRVBUFCTRL;

#ifdef MIN_RAM_SETTING
#ifdef SNOWBIRDL   //此定义是针对1080L(snowbirdL)芯片的内存定义的
#define DRIVER_FAT_BUF_NUM  64
#define DRIVER_FDT_BUF_NUM  (1*1024) //(8*1024)
#define DRIVER_OPEN_FILE_NUM    30
#define DRVIER_SEARCH_DEEP  30
#else  //
#define DRIVER_FAT_BUF_NUM  256
#define DRIVER_FDT_BUF_NUM  (8*1024)
#define DRIVER_OPEN_FILE_NUM    30
#define DRVIER_SEARCH_DEEP  30
#endif
#else
#define DRIVER_FAT_BUF_NUM  1024
#define DRIVER_FDT_BUF_NUM  (32*1024)
#define DRIVER_OPEN_FILE_NUM    50
#define DRVIER_SEARCH_DEEP  100
#endif

typedef struct _FAT_INFO
{
	T_U32 start;		//起始簇号
	T_U32 cnt;		//连续簇的数目
}FAT_INFO, *PFAT_INFO;

typedef struct _FILE_CLUS_BUF_ *P_FILE_CLUS_BUF;
typedef struct _FILE_CLUS_BUF_
{
    T_U32 FileID;
    T_U32 start;
    T_U32 cnt;
    P_FILE_CLUS_BUF next;
}FILE_CLUS_BUF;


typedef struct tag_Driver
{
    T_OBJECT  object;
    T_PMEDIUM medium;      //mediu object
    T_U8  SecBit;          //sector length
    T_U8  fs;              //FAT, NTFS, HFS+, EXT3
    T_U16 FatBufNum;        // the fatbufer number
    T_U16 SectorsPerBuf;   // the fatbufer number
    T_U16 CurUseFatBufNum;  //the pUsedFatInfo number which driver have used
    T_U16 UseMinClusPos;
    T_U16 CurUnUseFatBufNum;//the pUnUsedFatInfo number which driver have used
    T_U16 FdtBufLenDir;    //fdtbufDir length(Num of sectors)
    T_U16 DestroyFlag;     // the flag wihch the driver have been destoyed
    T_U16 RWcnt;           // the reading and wirting file number
    T_S32 RWcntSem;       // the semaphore which the reading and wirting file number
    T_S32 RWTimeSem;     //the semaphore corresponding to read and wirte file time.
    T_S32 OpenCloseSem;          //Each driver can has only one writing operation at one time.
    T_S32 FatBufSemAllc;   //Corresponding to the driver->fatbufAllc.
    T_S32 FatAsynSemAllc;   //Corresponding to the driver->fatbufAllc.
    T_U32 FdtTimesDirMax;  //Max times before we re-set the FdtTimesDirRef.
    T_U32 FdtTimesDirRef;  //How many times we have read fdt buffer(fdtbufDir)
    T_S32 FdtBufSemDir;    //the semaphore corresponding to fdtbuf member.
    T_BLINK *fdtbufDir;    //Fdt buffer
    T_U32 FreeSize;         //the unused rom
    T_U32 capacity;        //capactiy
    T_U32 StartSector;     //start in medium.
    T_U16 separator[2];    //'/' or '\\'
    T_U32 code;            //CODE_ENG, CODE_GBK
    T_U32 DefaultPath;     //default path id
    T_U32 DefaultParentPath;     //default path id
    T_U32 DefaultParentPathLen;     //default path length
    T_S32 FileListSem;     //the semaphore corresponding to FileList member.
    T_PLIST  FileList;     //all of open file.
    PFAT_INFO pUsedFatInfo;
    PFAT_INFO pUnUsedFatInfo;
    P_FILE_CLUS_BUF pFileClusBuf;
    T_POBJECT msg;         //bpb object.
}T_DRIVER, *T_PDRIVER;

T_VOID Driver_Destroy(T_PDRIVER driver);
T_PDRIVER Driver_Initial1(T_PMEDIUM medium, T_U32 BufLen, T_U32 code, F_GetDriverCallback pGet_Griver);
T_PDRIVER Driver_Format1(T_PMEDIUM medium, T_U32 start,
                        T_U32 total, T_U32 BufLen, T_U32 code,E_FATFS FormatType);
T_PDRIVER Driver_GetObject(T_U8 DeviceId);

T_BOOL Driver_InsertFile(T_PDRIVER driver, T_POBJECT file);
T_PDRIVER Driver_GetObject1(const T_U16* path);
T_U32 Driver_GetOpenTotal(T_PDRIVER driver);
T_BOOL Driver_SetDefault(T_U8 dn);
T_U32 Driver_GetFreeSize1(T_PDRIVER driver, T_U32* high);
P_FILE_CLUS_BUF Driver_SearchFileFatBuf(T_PDRIVER driver, T_U32 FileID, P_FILE_CLUS_BUF *pre);
void Driver_InsertFileFatBuf(T_PDRIVER driver, T_U32 FileID, T_U32 start, T_U32 cnt);
void Driver_DelFileFatBuf(T_PDRIVER driver, T_U32 FileID);
void Driver_DestroyFileFatBuf(T_PDRIVER driver);
#endif //_DRIVER_LIB_H_

