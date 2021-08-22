/*
 * @(#)romfat.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _ROMFAT_H_
#define        _ROMFAT_H_

#include    "cluster.h"
#include    "blink.h"
#include    "attr.h"
#include    "driver.h"
#include	"file.h"
#include	"fid.h"
#include "file_errortype.h"


typedef int RomFs_Fat_CompareFatBufFunc(const T_ROMFSCB *pCb,PFAT_INFO pCurFatInfo, PFAT_INFO pNewFatInfo);

#define 	CLUSTER_CLEAR_ADDR  		     (0x08309114-0x08308000+0x00004c00)
#define 	CLUSTER_CREATEBLOCK_ADDR  	     (0x08309154-0x08308000+0x00004c00)
#define 	CLUSTER_ADD_ADDR                     (0x083092d0-0x08308000+0x00004c00)
#define 	CLUSTER_GETBLOCK_ADDR                (0x083091dc-0x08308000+0x00004c00)
#define 	CLUSTER_GETCOUNT_ADDR                (0x083094d0-0x08308000+0x00004c00)
#define 	BLINK_SETTOHEAD_ADDR                 (0x0830959c-0x08308000+0x00004c00)
#define 	BLINK_NEXTNOCOPY_ADDR                (0x083095ac-0x08308000+0x00004c00)
#define 	DRIVER_SEARCHFILEFATBUF_ADDR         (0x083095e0-0x08308000+0x00004c00)
#define 	FAT_GETFATLINKINFO_ADDR              (0x08309614-0x08308000+0x00004c00) 
#define 	FAT_COMPAREUSEFATBUF_ADDR            (0x08309638-0x08308000+0x00004c00)
#define 	FAT_COMPAREFINDUSEFATBUF_ADDR        (0x08309678-0x08308000+0x00004c00)
#define 	FAT_COMPAREUNUSEFATBUF_ADDR          (0x083096b8-0x08308000+0x00004c00)
#define 	FAT_BINARYSEARCHFATBUF_ADDR          (0x083096f8-0x08308000+0x00004c00)   
#define 	FAT_LOADFATFROMROM_ADDR              (0x083098c4-0x08308000+0x00004c00)
#define 	FAT_LOADCLUSTERLINK_ADDR             (0x08309c14-0x08308000+0x00004c00)
#define 	FAT_DRIVERREADFAT_ADDR               (0x08309850-0x08308000+0x00004c00)
#define 	FAT_READFDT_ADDR                     (0x08309e00-0x08308000+0x00004c00)
#define 	FAT_CLUSTOSEC_ADDR                   (0x08309f44-0x08308000+0x00004c00)
#define 	FAT_READDATA_ADDR                    (0x0830a154-0x08308000+0x00004c00)
#define 	FAT_READBLOCK_ADDR                   (0x08309f68-0x08308000+0x00004c00)
#define 	FILE_COMPARE64_ADDR                  (0x0830a224-0x08308000+0x00004c00)
#define 	FILE_READPUBLICDATA_ADDR             (0x0830a250-0x08308000+0x00004c00)
#define 	FILE_READFILEPUBLICBUF_ADDR          (0x0830a480-0x08308000+0x00004c00)
#define 	FILE_READ_ADDR                       (0x0830a95c-0x08308000+0x00004c00)
#define 	FILE_READWITHOUTSEM_ADDR             (0x0830a674-0x08308000+0x00004c00)

typedef T_U32 (*_fFat_ClusToSec)(T_PFAT fat, T_U32 cluster);
typedef T_BOOL (*_fFat_LoadClusterLink)(const T_ROMFSCB* pCb,T_PATTR attr,T_U32 vcn);
typedef T_BOOL (*_fFat_DriverReadFat)(const T_ROMFSCB* pCb, T_PDRIVER driver, T_U32 Addr, T_U8 *buf);
typedef T_U8* (*_fFat_ReadFdt)(const T_ROMFSCB* pCb, T_PDRIVER driver, T_PFATSEARCH sm);
typedef T_U32 (*_fFat_ReadBlock)(const T_ROMFSCB* pCb, T_PFILE file, T_U8* buf, T_U32 SectorCount);
typedef T_BOOL (*_fFat_ReadData)(const T_ROMFSCB* pCb, T_PFILE file);

T_VOID RomFs_Cluster_Clear(T_PCLUSTER obj);
T_U8 RomFs_Cluster_CreateBlock(T_U8* BlockBuf, T_U32 relative, T_U32 len, T_BOOL normal);
T_BOOL RomFs_Cluster_Add(const T_ROMFSCB* pCb,T_PCLUSTER obj, T_U32 cluster, T_U32 len);
T_U8 RomFs_Cluster_GetBlock(const T_ROMFSCB* pCb, const T_U8 *data, T_U32* count, T_U32* RealAddr, T_U32* relative, T_BOOL* normal);
T_U32 RomFs_Cluster_GetCount(const T_ROMFSCB* pCb,T_PCLUSTER obj, T_U32 vcn, T_U32 *RetCount);
T_VOID RomFs_BLink_SetToHead( T_PBLINK obj);
T_U8* RomFs_BLink_NextNoCopy(const T_ROMFSCB* pCb,T_PBLINK obj);
P_FILE_CLUS_BUF RomFs_Driver_SearchFileFatBuf(T_PDRIVER driver, T_U32 FileID, P_FILE_CLUS_BUF *pre);
T_U32 RomFs_FAT_GetFatLinkInfo(const T_ROMFSCB* pCb, T_U8 * pFatBuf, T_U16 offset, T_U8 FSType);
int RomFs_Fat_CompareUseFatBuf(const T_ROMFSCB* pCb,PFAT_INFO pCurFatInfo, PFAT_INFO pNewFatInfo);
int RomFs_Fat_CompareFindUseFatBuf(const T_ROMFSCB *pCb,PFAT_INFO pCurFatInfo, PFAT_INFO pNewFatInfo);
int RomFs_Fat_CompareUnUseFatBuf(const T_ROMFSCB* pCb,PFAT_INFO pCurFatInfo, PFAT_INFO pNewFatInfo);
T_BOOL RomFs_Fat_BinarySearchFatBuf(const T_ROMFSCB *pCb,PFAT_INFO pAllFatInfo, PFAT_INFO pNewFatInfo, RomFs_Fat_CompareFatBufFunc comp, T_U16 TolItems, T_U16 *FindItem);
T_U32 RomFs_Fat_LoadFatFromRom(const T_ROMFSCB* pCb,T_PDRIVER driver, T_PCLUSTER clus, T_U32 FstClus, T_U8 AddFstFlag);
T_BOOL RomFs_Fat_LoadClusterLink(const T_ROMFSCB* pCb, T_PATTR    attr, T_U32 vcn);
T_BOOL RomFs_Fat_DriverReadFat(const T_ROMFSCB* pCb, T_PDRIVER driver, T_U32 Addr, T_U8 *buf);
T_U8* RomFs_Fat_ReadFdt(const T_ROMFSCB* pCb, T_PDRIVER driver, T_PFATSEARCH sm);
T_U32 RomFs_Fat_ClusToSec(T_PFAT fat, T_U32 cluster);
T_BOOL RomFs_Fat_ReadData(const T_ROMFSCB* pCb, T_PFILE file);
T_U32 RomFs_Fat_ReadBlock(const T_ROMFSCB* pCb, T_PFILE file, T_U8* buf, T_U32 SectorCount);
int RomFs_File_Compare64(T_U32 low1, T_U32 high1,T_U32 low2, T_U32 high2);
T_BOOL RomFs_File_ReadPublicData(const T_ROMFSCB* pCb,T_PFILE file);
void RomFs_File_ReadFilePublicBuf(const T_ROMFSCB* pCb,T_PFILE file, T_U32 low, T_U32 high, T_U32 size, T_U8 *buf);
T_U32 RomFs_File_Read(const T_ROMFSCB* pCb,T_PFILE file, T_pVOID buf1, T_U32 size);
T_U32 RomFs_File_ReadWithoutSem(const T_ROMFSCB* pCb,T_PFILE file, T_pVOID buf1, T_U32 size);


#endif //_FAT_H

/* end of file */

