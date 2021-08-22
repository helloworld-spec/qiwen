/*
 * @(#)fs.h
 * @date 2010/07/15
 * @version 1.0
 * @author Lu_Qiliu.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _FS_H_
#define        _FS_H_

#define ONLY_SD 1

#ifndef SD_BURNER
#ifndef ONLY_SD
#include "nandflash.h"
#include "mtdlib.h"
#endif

#include "file.h"

#else
#include "medium.h"
#endif


//如下的代码是由于只支持sd，所以增加如下的代码，是为了让平台编译通过而已.
//如果需要更换头文件，请注意增加这些代码。
////////////////////////////////////////////////////////////////////////
#ifdef ONLY_SD
typedef    struct tag_Medium  T_MEDIUM;
typedef    struct tag_Medium *T_PMEDIUM;
typedef    struct    Object    T_OBJECT;
typedef    struct    Object *T_POBJECT;

typedef    unsigned long (*F_ReadSector)(T_PMEDIUM medium, unsigned char* buf, unsigned long start, unsigned long size);
typedef    unsigned long (*F_WriteSector)(T_PMEDIUM medium, const unsigned char *buf, unsigned long start, unsigned long size);
typedef    void (*F_DeleteSector)(T_PMEDIUM medium,unsigned long StartSce,unsigned long SecSize);
typedef    unsigned char (*F_Flush)(T_PMEDIUM medium);
typedef    void (*F_DESTROY)(T_POBJECT);

struct    Object
{
    unsigned long type;
    F_DESTROY destroy;
};

struct tag_Medium
{
    T_OBJECT object;
    unsigned char  type;     // corresponding to E_MEDIUM
    unsigned char  SecBit;
    unsigned char  PageBit;  // 2^n = SecPerPage.
    unsigned char  SecPerPg; // 2^PageBit.
    unsigned long capacity;
    F_ReadSector read;
    F_WriteSector write;
    F_DeleteSector DeleteSec;
    F_Flush flush;
    unsigned char* msg;
};

typedef enum
{
    MEDIUM_RAM       = 0,
    MEDIUM_ROM       = 1,
    MEDIUM_NANDFLASH = 2,
    MEDIUM_SD        = 3,
    MEDIUM_NORFLASH  = 4,
    MEDIUM_DISKETTE  = 5,
    MEDIUM_FILE      = 6,
    MEDIUM_NANDRES   = 7,
    MEDIUM_USBHOST   = 8,
    MEDIUM_PARTITION = 9,

    MEDIUM_UNKNOWN   = 255
}E_MEDIUM;
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SD_BURNER
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
    FS_AK39XX_E = 14,
    FS_AK10XX_T2 = 15,
    FS_AK39XX_H240 = 16,
    FS_AKERR  = 255
}E_AKCHIP_FS;
#endif

typedef void (*F_OutStream)(unsigned short ch);
typedef unsigned char (*F_Instream)(void);
typedef unsigned long (*F_GetSecond)(void);
typedef void (*F_SetSecond)(unsigned long seconds);
/* Jan.10,07 - Modified to support Multi-Language */
typedef signed long (*F_UniToAsc)(const unsigned short *pUniStr, unsigned long UniStrLen,signed char *pAnsibuf, unsigned long AnsiBufLen, unsigned long code);
typedef signed long (*F_AscToUni)(const signed char *pAnsiStr, unsigned long AnsiStrLen,unsigned short *pUniBuf, unsigned long UniBufLen, unsigned long code);

typedef void *(*F_RamAlloc)(unsigned long size, signed char *filename, unsigned long fileline);
typedef void *(*F_RamRealloc)(void *var, unsigned long size, signed char *filename, unsigned long fileline); 
typedef void *(*F_RamFree)(void *var, signed char *filename, unsigned long fileline);

/* Sep.13,07 - Added to support Muti-Task. */
typedef signed long (*F_OsCrtSem)(unsigned long initial_count, unsigned char suspend_type, signed char *filename, unsigned long fileline);
typedef signed long (*F_OsDelSem)(signed long semaphore, signed char *filename, unsigned long fileline);
typedef signed long (*F_OsObtSem)(signed long semaphore, unsigned long suspend, signed char *filename, unsigned long fileline);
typedef signed long (*F_OsRelSem)(signed long semaphore, signed char *filename, unsigned long fileline);

typedef unsigned long (*F_GetChipID)(void);

typedef void *(*F_MemCpy)(void *dst, const void* src, unsigned long count);
typedef void *(*F_MemSet)(void *buf, signed long value, unsigned long count);
typedef void *(*F_MemMov)(void *dst, const void* src, unsigned long count);
typedef signed long   (*F_MemCmp)(void *buf1, const void* buf2, unsigned long count);
typedef signed long   (*F_Printf)(const signed char *s, ...);

typedef unsigned long   (*ThreadFunPTR)(void *pData);
typedef unsigned long  (*F_MountThead)(ThreadFunPTR Fun, void *pData, unsigned long priority);
typedef void  (*F_KillThead)(unsigned long ThreadHandle);
typedef void  (*F_SystemSleep)(unsigned long ms);
typedef void F_ChkDskCallback(void *pData);

typedef void  (*F_MtdSysRst1)(void);
typedef void (*F_MtdRandSeed1)(void);
typedef unsigned long (*F_MtdGetRand1)(unsigned long);


typedef struct tag_FsCallback
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

    F_MtdSysRst1    fSysRst;
    F_MtdRandSeed1  fRandSeed;
    F_MtdGetRand1   fGetRand; 
    F_MountThead   fMountThead;   
    F_KillThead   fKillThead;  
    F_SystemSleep  fSystemSleep;
}T_FSCallback, *T_PFSCallback;

typedef unsigned char (*F_GetDriverCallback)(void);
typedef unsigned long (*F_DRIVER_Read)  (T_PMEDIUM medium, unsigned char *buf,unsigned long BlkAddr, unsigned long BlkCnt); 
typedef unsigned long (*F_DRIVER_Write) (T_PMEDIUM medium, const unsigned char *buf,unsigned long BlkAddr, unsigned long BlkCnt); 
typedef 
#ifndef __GNUC__ //ADS编译使用
#ifndef WIN32
            __packed
#endif
#endif
struct _DRIVER_INFO_
{
    unsigned char        DriverID;          
    unsigned char        nMainType;		// the value of E_MEDIUM
    unsigned char        nSubType;		// the value of E_SUB_MEDIUM
    unsigned long       nBlkSize;
    unsigned long       nBlkCnt;
    T_PMEDIUM   medium;
    F_DRIVER_Read   fRead;
    F_DRIVER_Write  fWrite;
}
#ifdef  __GNUC__ //GCC编译使用
__attribute__((packed))
#endif

DRIVER_INFO, *PDRIVER_INFO;

typedef struct
{
    unsigned char  Disk_Name;   //盘符名
    unsigned char  bOpenZone;   //E_SUB_MEDIUM--用户 系统盘
    unsigned char  ProtectType; //MEDIUM_PORTECT_LEVEL_NORMAL(CHECK, READONLY)        
    unsigned char  ZoneType;    //E_FS_ZT ------分区的类型
    unsigned long Size;        //real partiton capacity(Mbit)
    unsigned long EnlargeSize;         //enlarge capacity(Mbit)
    unsigned long HideStartBlock;      //reserve hide disk start 
    unsigned long FSType;              //E_FATFS --FS type
    unsigned long resv[1];             //reserve
}T_FS_PARTITION_INFO;

typedef struct _FOEMAT_INFO_
{
    unsigned char  MediumType; /*medium type*/
    unsigned long obj; /*if nand obj = T_PNANDFLASH , else SD obj= PDRIVER_INFO*/
} FORMAT_INFO, *PFORMAT_INFO;

typedef enum 
{
    FS_NAND,   
    FS_SD,     
}E_FS_MEDIUM;


typedef enum 
{
    SYSTEM_PARTITION,   //系统分区,一般是保留分区,用户不可见
    USER_PARTITION,     //用户分区,此分区为U盘可见分区,
}E_SUB_MEDIUM;

typedef enum 
{
    ZT_PRIMARY = 0,
    ZT_SLAVE,
}E_FS_ZT;

/************************************************************************
 * NAME:        FS_InitCallBack
 * FUNCTION  Initial FS callback function
 * PARAM:      [in] fsInitInfo------ callback function struct pointer
                      [in]PagesPerFSBuf--fs cache capacity, unit page
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_InitCallBack(T_PFSCallback fsInitInfo, unsigned short PagesPerFSBuf);

#ifndef SD_BURNER
/************************************************************************
 * NAME:        FS_MountNandFlash
 * FUNCTION  fs mount nandflash, register nand's para
 * PARAM:      [in] gNand------ nand's  para struct pointer
                    [in]StartBlock----fs's start block relative to this medium start position
                    [out]DriverList-----get dirver ID array, must no less than driver's count, max is 26 byte
                    [out]DriverCnt--  driver's count
 * RETURN:    success driver start ID, fail return T_U8_MAX
**************************************************************************/
#ifndef ONLY_SD
unsigned char FS_MountNandFlash(T_PNANDFLASH gNand, unsigned long StartBlock, unsigned char DriverList[], unsigned char *DriverCnt);
#endif

/************************************************************************
 * NAME:        FS_MountMemDev
 * FUNCTION   fs mount enable remove device, register remove device's para
 * PARAM:      [in] pDevInfo---- need mount's remove device's  para struct pointer
                      [out]DriverCnt--  driver's count
                      [in]StartID--  driver id, system will automatically malloc one id if it is 0xFF
 * RETURN:     success driver start ID, fail return T_U8_MAX
**************************************************************************/
unsigned char FS_MountMemDev(PDRIVER_INFO pDevInfo, unsigned char *DriverCnt, unsigned char StartID);

/************************************************************************
 * NAME:        FS_UnMountMemDev
 * FUNCTION   fs unmount enable remove device
 * PARAM:      [in] DriverID------ need unmount's remove device's driverID

 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_UnMountMemDev(unsigned char DriverID);

/************************************************************************
 * NAME:        FS_Destroy
 * FUNCTION   fs destroy free all fs malloced memory
 * PARAM:      no

 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_Destroy(void);

/************************************************************************
 * NAME:        FS_UnInstallDriver
 * FUNCTION   fs uninstall driver
 * PARAM:       [in] DriverID------ need remove device's driverID start
                        [in] DriverCnt-----need remove device's driverID count
 * RETURN:     success return be uninstalled driver ID count , fail retuen 0
**************************************************************************/
unsigned char FS_UnInstallDriver(unsigned char DriverID, unsigned char DriverCnt);

/************************************************************************
 * NAME:        FS_InstallDriver
 * FUNCTION   fs install driver, notice: only insert  driver ID to install queue, need wait for thead auto install
 * PARAM:       [in] DriverID------ need install device's driverID start
                        [in] DriverCnt-----need install device's driverID count
 * RETURN:     success return be installed driver ID count , fail retuen 0
**************************************************************************/
unsigned char FS_InstallDriver(unsigned char DriverID, unsigned char DriverCnt);


/************************************************************************
 * NAME:        FS_InstallDriver_CallBack
 * FUNCTION   fs install driver, notice: only insert  driver ID to install queue, need wait for thead auto install
 * PARAM:       [in] DriverID------ need install device's driverID start
                        [in] DriverCnt-----need install device's driverID count
                        [in] F_GetDriverCallback pGet_Griver    
 * RETURN:     success return be installed driver ID count , fail retuen 0
**************************************************************************/
unsigned char FS_InstallDriver_CallBack(unsigned char DriverID, unsigned char DriverCnt, F_GetDriverCallback pGet_Griver);


/************************************************************************
 * NAME:        FS_GetDriver
 * FUNCTION   fs get driver info by driverID
 * PARAM:       [out] pDriverInfo---dirver info
                        [in] DriverID-----need get driver's ID
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_GetDriver(PDRIVER_INFO pDriverInfo, unsigned char DriverID);

/************************************************************************
 * NAME:        FS_GetFakeMedium
 * FUNCTION   fs get the medium of the driver by driverID
 * PARAM:       [in] DriverID-----need get driver's ID
 * RETURN:     return the medium point
**************************************************************************/

T_PMEDIUM FS_GetFakeMedium(unsigned char DriverID);


/************************************************************************
 * NAME:        FS_GetFirstDriver
 * FUNCTION   fs get first driverID's driver info 
 * PARAM:       [out] pDriverInfo---dirver info
                        
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_GetFirstDriver(PDRIVER_INFO pDriverInfo);

/************************************************************************
 * NAME:        FS_GetNextDriver
 * FUNCTION   fs get next driverID's driver info 
 * PARAM:       [out] pDriverInfo---dirver info
                        
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_GetNextDriver(PDRIVER_INFO pDriverInfo);

/************************************************************************
 * NAME:        FS_ChkDsk
 * FUNCTION   fs check disk error and  repair fat link
 * PARAM:       [in] DriverID---need check's dirverID
                       [in]pCallBack--callback function,   chkdsk process index
                       [in]CallbackData-callback function's para
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_ChkDsk(unsigned char DriverID, F_ChkDskCallback pCallBack, void *CallbackData);

/************************************************************************
 * NAME:        FS_FlushDriver
 * FUNCTION   fs flush driver , flush all data to medium
 * PARAM:       [in] DriverID---need flush's dirverID

 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_FlushDriver(unsigned char DriverID);

/************************************************************************
 * NAME:        FS_CheckInstallDriver
 * FUNCTION   fs check driver is not install
 * PARAM:       [in] DriverID---need check's DriverName("A","B","C"......"Z")

 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_CheckInstallDriver(unsigned char DriverName);

/************************************************************************
 * NAME:        FS_CreateCache
 * FUNCTION   fs create cache for medium
 * PARAM:       [in] medium---need create's medium
                        [in] CacheSize-cache size
 * RETURN:     success return cache medium, fail retuen AK_NULL
**************************************************************************/
T_PMEDIUM FS_CreateCache(T_PMEDIUM medium, unsigned long CacheSize);

/************************************************************************
 * NAME:        FS_DestroyCache
 * FUNCTION   fs destroy cache for medium
 * PARAM:       [in] CacheMedium---need destroy's medium
 * RETURN:     no
**************************************************************************/
void FS_DestroyCache(T_PMEDIUM CacheMedium);

/************************************************************************
 * NAME:        FS_AsynFlush
 * FUNCTION   fs flush all asynchronism data to device
 * PARAM:       [in] NONE
 * RETURN:     NONE
**************************************************************************/

void FS_AsynFlush(void);

/************************************************************************
 * NAME:        FS_SetAsynWriteBufSize
 * FUNCTION   fs create cache for medium, if user want to use asyn operator to other driver,
                        user must call the function with other id, it will flush all asyn data with current id,
                        and start new dirver asyn buffer.
 * PARAM:       [in] BufSize---the asyn buffer size
                       [in] DriverID---the asyn operator driver id
 * RETURN:     NONE
**************************************************************************/
unsigned char FS_SetAsynWriteBufSize(unsigned long BufSize, unsigned char DriverID);

/************************************************************************
 * NAME:        FS_QuickFormatDriver
 * FUNCTION   it will format the dirver with id,it don't change the type of the file system
 * PARAM:       [in] DriverID---the format operator driver id
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/

unsigned char FS_QuickFormatDriver(unsigned char DriverID);

/************************************************************************
 * NAME:        FS_SetAsynWriteBufSize
 * FUNCTION   fs create cache for medium, if user want to use asyn operator to other driver,
                        user must call the function with other id, it will flush all asyn data with current id,
                        and start new dirver asyn buffer.
 * PARAM:       [in] DriverID---the asyn operator driver id
                       [in] FsType---the type of the format driver
                       
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/

unsigned char FS_FormatDriver(unsigned char DriverID, E_FATFS FsType);


/************************************************************************
 * NAME:        FS_GetDriverCapacity
 * FUNCTION   it will get the capacity of the driver
 * PARAM:       [in] DriverID---the asyn operator driver id
                       [in] high---the high 32 bit of the driver capacity
                       
 * RETURN:     low 32 bit of the driver capacity
**************************************************************************/

unsigned long FS_GetDriverCapacity(unsigned char DriverID, unsigned long *high);


/************************************************************************
 * NAME:        FS_GetDriverUsedSize
 * FUNCTION   it will get the use size of the driver
 * PARAM:       [in] DriverID---the asyn operator driver id
                       [in] high---the high 32 bit of the driver use size
                       
 * RETURN:     low 32 bit of the driver use size
**************************************************************************/

unsigned long FS_GetDriverUsedSize(unsigned char DriverID, unsigned long *high);



/************************************************************************
 * NAME:        FS_GetDriverFreeSize
 * FUNCTION   it will get the capacity of the driver
 * PARAM:       [in] DriverID---the asyn operator driver id
                       [in] high---the high 32 bit of the driver free size
                       
 * RETURN:     low 32 bit of the driver free size
**************************************************************************/

unsigned long FS_GetDriverFreeSize(unsigned char DriverID, unsigned long *high);

/************************************************************************
 * NAME:       FS_SpeedupUsbNand
 * FUNCTION：it will check some data to connect fs in the mode of usb nand
 * PARAM:  T_PMEDIUM medium
            T_U8 data       the sector data
            T_U32 sector    the sector address
            T_U32 size        the sector size
 * RETURN: none
**************************************************************************/

void FS_SpeedupUSBNand(T_PMEDIUM medium, const unsigned char data [ ], unsigned long sector, unsigned long size);

/************************************************************************
 * NAME:        FS_ClearAsyn
 * FUNCTION  it will clear some asyn buffer data
 * PARAM:      T_PMEDIUM medium
                       T_U32 sector
                       T_U32 size                      
 * RETURN:      NONE
**************************************************************************/
void FS_ClearAsyn(T_PMEDIUM medium, unsigned long sector, unsigned long size);

/************************************************************************
 * NAME:       FS_GetVersion
 * FUNCTION：get fs mount version info
 * PARAM: T_VOID
 * RETURN: version info
**************************************************************************/
unsigned char *FS_GetVersion(void);
#else
/************************************************************************
 * NAME:     FS_GetOldFsMedium
 * FUNCTION：get old fs medium by driver ID
 * PARAM:    [in] DriverID--the fs dirver ID 
 * RETURN:   success return Medium Addr, fail retuen 0
**************************************************************************/
unsigned long FS_GetOldFsMedium(unsigned char DriverID);
#endif

/************************************************************************
* NAME:        FS_FormatFSPartitionInfo
* FUNCTION   Format fs partition and management-self by input partition info. (for burn)

* PARAM:       [in] PartitionInfo--need to managing meidum partition info array
                    [in] nNumPartion--partition info array's number
                    [in ]resvSize------mtd's reserve area size(unit is block)
                    [in]StartBlock-----fs's start block relative to this medium start position
                    [in]pFormatInfo---format require's part of para
* RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_LowFormat(T_FS_PARTITION_INFO PartitionInfo[], 
    unsigned long nNumPartion, 
    unsigned long resvSize, 
    unsigned long  StartBlock, 
    PFORMAT_INFO pFormatInfo);

/************************************************************************
 * NAME:       FS_GetDriverCapacity_SecCnt
 * FUNCTION：get driver format real Capacity
 * PARAM:    [in] driverID--driver ID
 * RETURN:   driver format real Capacity, if no driver return 0;
**************************************************************************/
unsigned long  FS_GetDriverCapacity_SecCnt(unsigned char DriverID);

/************************************************************************
 * NAME:     FS_GetDriverAttr
 * FUNCTION：get driver Attribute
 * PARAM:    [in] driverID--driver ID
 * RETURN:   success return MEDIUM_PORTECT_LEVEL_NORMAL(CHECK, READONLY), fail return T_U32_MAX;
**************************************************************************/
unsigned long  FS_GetDriverAttr(unsigned char DriverID);

/************************************************************************
 * NAME:      FS_MountCacheToDriver
 * FUNCTION   fs mount cache medium for driver
 * PARAM:     [in] medium---the cache medium for driver 
 * RETURN:    success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_MountCacheToDriver(T_PMEDIUM medium);

/************************************************************************
 * NAME:      FS_MountCacheToDriver
 * FUNCTION   fs unmount cache medium for driver
 * PARAM:     [in] medium---the cache medium for driver 
 * RETURN:    success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_UnMountCacheToDriver(T_PMEDIUM medium);

/************************************************************************
 * NAME:     FS_GetAsynBufInfo
 * FUNCTION  check which file when asyn wirte sector error .
 * PARAM:    UseSize--asyn buffer has used size(unit byte)
 *           BufSize--asyn buffer capacity size(unit byte)
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_GetAsynBufInfo(unsigned long *UseSize, unsigned long *BufSize);


/************************************************************************
 * NAME:     FS_UnMountnNandflash
 * FUNCTION  fs unmount nand medium for driver
 * PARAM:    DriverID--driver id
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
unsigned char FS_UnMountnNandflash(unsigned char DriverID);
#ifdef NFTL_RW_FLAG
/************************************************************************
 * NAME:     FS_CurrentRWFlag
 * FUNCTION  Get current NFTL read&write status flag (only apply to AK11 platform).
 *                  
 * PARAM:    file handle; 
 *
 * RETURN:     success return AK_TRUE,  R&W is busy
 *                   fail retuen AK_FALSE, R&W is idle
**************************************************************************/
unsigned char FS_CurrentRWFlag(unsigned long file);
#endif



/************************************************************************
 * NAME:     FS_SetSecPerPg
 * FUNCTION  fs set sector num per  one page, use befor FS_MountMemDev
 * PARAM:   T_U8 SecPerPg   exp:page size 16K,  SecPerPg == 5    (2^5)*512 = 16K
 * RETURN:     T_VOID
**************************************************************************/
void FS_SetSecPerPg(unsigned char SecPerPg);

#endif //_FS_H_

