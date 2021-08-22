/*
 * @(#)fat.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _FAT_H_
#define        _FAT_H_

#include    "cluster.h"
#include    "blink.h"
#include    "attr.h"
#include    "driver.h"
#include	"file.h"
#include	"fid.h"
#include "file_errortype.h"



typedef enum
{
    FAT_DONOING  = 0,
    FAT_CMPCLUS  = 1,
    FAT_CMPNAME  = 2,
    FAT_CMPSHORT = 3,
    FAT_GETNAME  = 4,
    FAT_GETSPACE = 5,
    FAT_SETNAME  = 6,    
    FAT_GETALL   = 7,
    FAT_DEL      = 8,

    FAT_SEARCHERR = 255
}E_FATSEARCH;
#define FAT32_MAX_USER_CLUSTER    0XFFFFFF8
#define FAT16_MAX_USER_CLUSTER    0XFFF8


typedef struct tag_File
{
    T_OBJECT obj;    
    T_U32  ptr;         //read and write pointer 's low  32bit.
    T_U32  high;
    T_U32  DirtyLen;    //dirty data length in the Dadabuf.
    T_U32  BufLenMask;  //Default: BufLenMask = sector length - 1.
    T_U8*  DataBuf;     //file data buffer. Default: DataBuf's len = sector length.
    T_U8   mode;        //read and write mode. it is useless for dirctory.
    T_U8   ExtMode;     //When closed, whether to Write FAT/FDT
    T_BOOL DirtyFlag;   //read and write mode. it is useless for dirctory.
    T_BOOL ValidFlag;   //read and write mode. it is useless for dirctory.
    T_BOOL AsynFlag;   //read and write mode. it is useless for dirctory.
    T_U32  CloseFlag;    
    T_U8*  attr;        //share attribute, for all object of same file.
}T_FILE, *T_PFILE;


typedef struct tag_FatSearchCtrl
{
    T_U8 FunType;    // Query type, corresponding to E_FATSEARCH
//  T_U8 HoldBuf;    // Delete or not after dealing with the data(Not use by now)
    T_U8 Result;     // Corresponding to E_FSSEARCH.
    T_U8 NameDiffer; // input name equal with an existed name except case.
    T_U8 Reserve[13];// Reserved for later use.
}T_SMCTRL, *T_PSMCTRL;

typedef struct tag_FatSearch
{
    T_SMCTRL Ctrl;
    T_PATTR attr;        // SETNAME or GETNAME. CMPSHORT
    T_U16 *name;         // CMPNAME
    T_U8  ShortName[12];  // CMPNAME
    T_U32 FileId;        // CMPCLUS,CMPNAME.
    T_U32 NameSpace;     // E_NAMESPACE
    T_U32 NameLen;

    T_U16 ExFatFDTNum;  //the file name will use fdt count
    T_U16 HashValue;     //the hash table
    T_U16 SeriousFlag;	 //it is only used by exfat

    T_U16 FileNum;    // FAT_GETALL
    T_U16 FolderNum;  // FAT_GETALL
    T_PBLINK link;    // FAT_GETALL

    T_U32 vcn;        // virtual cluster in link
    T_U32 vsn;        // virtual sector in cluster.
    T_U32 sfdt;       // fdt number in a sector.
    T_U32 cluster;    // physical cluster.
    T_U32 sector;     // physical sector.
    T_U32 fdt;        // fdt number in cluster link.
    T_U8* ptr;        // short name ptr in fdt buffer.
    T_U32 LongFdt;    // long name fdt.
    T_U32 ShortFdt;   // short fdt.
    T_U32 FdtCount;   // long name length.
    T_U32 EmptyFdt;
    T_PCLUSTER clus;  // directory cluster link.
    T_POBJECT msg;    //reserve object for exfat.
}T_FATSEARCH, *T_PFATSEARCH;


typedef struct tag_Fat
{
    T_OBJECT  obj;
    T_PDRIVER driver;
    T_U32 Fat1Addr;       //the address of first FAT.
    T_U32 FATSz;          //FAT length.
    T_U8  fs;         //FAT12,FAT16,FAT32
    T_U8  NumFATs;        //FAT total number.
    T_U8  ClusBit;        //2^ClusBit = SecPerClus
    T_U8  SecBit;         //2^SecBit = BytsPerSec
    T_U32 RootDirSectors; //root sector number.
    T_U32 RootAddr;       //root start address.
    T_U32 RootStartClus;  //Root Directory's start cluster number of FAT32
    T_U32 MaxClusNum;     //max cluster nubmer of a driver.
    T_U32 FileEndFlag;    //last cluster number's end flag.
    T_U32 ClusMask;       //SecPerClus - 1;
    T_U32 SecMask;        //BytsPerSec - 1;
    T_U32 FdtSecBit;      //FdtSecBit = SecBit - 5
    T_U32 FdtClusBit;     //FdtClusBit = fat->ClusBit + fat->FdtSecBit
    T_U32 FdtPerSec;      //FdtPerSec = 1 << fat->FdtSecBit
    T_U32 SecPerClus;     //SecPerClus = 1 << ClusBit
    T_U32 LastAllocCluster;   //last allocate cluster number.
    T_PCLUSTER clus;          //those clusters which is allocated but not flushed.
}T_FAT, *T_PFAT;

/* Added to control the AllocFat operation. */
typedef struct tag_AllocFatCtrl
{
    T_BOOL bClearCluster;
    T_BOOL bAddToFatClus;
}T_ALLOCFATCTRL, *T_PALLOCFATCTRL;

T_VOID Fat_Destroy(T_PFAT obj);
//system operate.
T_PFAT Fat_Initial(T_PDRIVER driver, const T_U8 *buf);
T_U32 Fat_GetUsedSize(T_PDRIVER driver);
T_U32 Fat_GetFreeSize(T_PDRIVER driver);

//fat operate.
T_VOID Fat_AddDirFirstClus(T_PDRIVER driver, T_PCLUSTER clus, T_U32 FstClus);
T_U32 Fat_ReadLink(T_PDRIVER driver, T_PCLUSTER clus, T_U32 FstClus);
T_BOOL Fat_WriteLink(T_PDRIVER driver, T_PCLUSTER clus);
T_VOID Fat_DelLink(T_PDRIVER driver, T_PCLUSTER clus);

//fdt operate
T_U8* Fat_ReadFdt(T_PDRIVER driver, T_PFATSEARCH sm);
T_BOOL Fat_WriteFdt(T_PDRIVER driver, T_PFATSEARCH sm);
T_BOOL Fat_SetAttr(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 fdt, T_PATTR attr);
T_BOOL Fat_SetVolumeAttr(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 fdt, T_PATTR attr);
T_VOID Fat32_SetAttr(T_U8* ptr, T_PATTR attr);
T_BOOL Fat_ChangeAttr(T_PFILE fp);
E_FSSEARCH Fat_SearchName(T_PDRIVER driver, T_U32 ParentId, T_U16* FileName, T_PFATSEARCH sm);
T_U8* Fat_FdtSeek(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 fdt);
T_VOID Fat_FdtIterate(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 fdt);
T_U32 Fat_GetFstClus(const T_U8 *ptr);

//data operate.
T_BOOL Fat_ReadData(T_PFILE file);
T_U32 Fat_WriteData(T_PFILE file);

T_U32 Fat_ReadBlock(T_PFILE file, T_U8* buf, T_U32 SectorCount);
T_U32 Fat_WriteBlock(T_PFILE file, const T_U8 *buf, T_U32 SectorCount);

T_VOID Fat_Flush(T_PFILE file);
T_U32 Fat_Seek(T_PFILE file, T_U32 LowPos, T_U32 *HighPos);

//file operate.
T_BOOL Fat_CreateFile(T_PFILE parent, T_PFILE file);
T_BOOL Fat_DeleteFile(T_PFILE file);
T_BOOL Fat_Mkdir(T_PFILE file);
T_BOOL Fat_RenameTo(T_PFILE source, T_PFILE dest);
T_PFILE Fat_OpenId(T_PDRIVER driver, T_U32 id, T_U32 ParentID, T_U32 FDTOffset);
T_U32 Fat_ClusToSec(T_PFAT fat, T_U32 cluster);
T_BOOL Fat_Format(T_PDRIVER driver, E_FATFS FormatType);
T_BOOL Fat_QuickFormat(T_PDRIVER driver);
T_VOID Fat_SetAttrModTime(T_PATTR attr);
T_BOOL Fat_ClearCluster(T_PDRIVER driver, T_U32 cluster);
T_BOOL Fat_SetLink(T_PDRIVER driver, T_PCLUSTER clus);
T_BOOL Fat_DriverReadFat(T_PDRIVER driver, T_U32 Addr, T_U8 *buf);
T_BOOL Fat_DriverWriteFat(T_PDRIVER driver, T_U32 Addr, T_U8 *buf);
T_VOID Fat_DelUseFatBuf(T_PDRIVER driver, PFAT_INFO pNewFatInfo);
T_VOID Fat_AddUnUseFatBuf(T_PDRIVER driver, PFAT_INFO pNewFatInfo, T_U8 flag, T_U8 AddFreeFlag);
T_BOOL Fat_LoadClusterLink(T_PATTR attr, T_U32 vcn);
T_U32 Fat_AllocArrayFat(T_PDRIVER driver, T_PCLUSTER clus, T_U32 *count, T_PALLOCFATCTRL pAllocCtrl, T_U32 FileID);
T_BOOL Fat_FlushFatLink(T_PATTR attr, T_U8 AllFlag, T_U8 *PubBuf);
T_VOID  Fat_ReleaseFileAllocFat(T_PFILE file);
T_BOOL Fat_InitFatBuf(T_PDRIVER driver, F_GetDriverCallback pGet_Griver);
T_VOID Fat_loadAllFatLink(T_PFILE file);
T_BOOL Fat_AddDriverUseClus(T_PDRIVER driver, T_PCLUSTER sub);
T_U8 Fat_FdtGetFileAttr(T_U32 fstype, const T_U8 *ptr);
T_BOOL Fat_FdtAllocOneCluster(T_PDRIVER driver, T_PFATSEARCH sm);
T_U32 Driver_GetObjDriver(T_U8 DeviceId);
T_U8  Driver_GetObjLength(T_VOID);
T_VOID Fat_ResertFatBufInfo(T_PDRIVER driver);
T_BOOL Fat_ChangeFileSize(T_PFILE fp);
T_BOOL Fat_MoveFreeClusFromFile(T_PDRIVER driver);
T_U32 Fat_AllocFileClus(T_PDRIVER driver, T_U32 *count, T_U32 FileID);
T_BOOL Fat_FlushFDT(T_PFILE file);
T_BOOL Fat_SearchFolderLink(T_PDRIVER driver, T_U32 FileID, T_PFATSEARCH sm);
void Fat_UpdateFolderLink(T_PDRIVER driver, T_U32 FileID, T_PFATSEARCH sm);
T_VOID Fat_FdtGetSector(T_PDRIVER driver, T_PFATSEARCH sm, T_U32 fdt);
T_BOOL Fat_create_volume(T_PDRIVER driver, T_PFATSEARCH sm);
T_U8 *Fat_Get_volume(T_PDRIVER driver, T_PFATSEARCH sm);
T_U32 Fat_SecToClus(T_PFAT fat, T_U32 Sec);
T_BOOL Fat_LoadFatFromFstClus(T_PDRIVER driver, T_U8 *buf, T_U32 FstClus, T_U32 *ErrClus, T_U32 *Clusnum, T_U32 *chkdsk_flag);
T_BOOL Fat_IsLoadFatBuf(T_PDRIVER driver);
T_BOOL Fat_LoadFatBuf(T_PDRIVER driver, T_U8 fs_type);


#endif //_FAT_H

/* end of file */

