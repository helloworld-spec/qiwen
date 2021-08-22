/*
 * @(#)Attr.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _ATTR_H_
#define        _ATTR_H_

#include "driverlib.h"
#include "ustring.h"
#include "file_errortype.h"


#define ATTR_READ_ONLY    0x01
#define ATTR_HIDDEN       0x02
#define ATTR_SYSTEM       0x04
#define ATTR_VOLUME_ID    0x08
#define ATTR_DIRECTORY    0x10
#define ATTR_ARCHIVE      0x20

/* Apr.20,07 - Corresponding to DIR_NTRes, compatible with PC Windows. */
#define ATTR_NTRES_ALLLOW  0x18
#define ATTR_NTRES_EXTLOW  0x10
#define ATTR_NTRES_NAMELOW 0x08

typedef enum
{
    ASYN_WRITE_OK   = 0,
    ASYN_WRITE_ERR  = 1,
    ASYN_WRITE_ERR_NO_CUT = 2,
}E_ASYN_WRITE;

typedef struct tag_Attribute
{
    T_OBJECT obj;    
    T_PDRIVER driver;        
    T_PSTRING name;          //last path name
    T_PSTRING ParentName;          //last path name
    T_U32 CreatTime;        
    T_U32 CreatDate;        
    T_U32 ModTime;            
    T_U32 ModDate;            
    T_U32 FileId;            //0: root folder
    T_U32 ParentId;          //parent id.
    T_U32 length;            //data length
    T_U32 excess;            //data length high
    T_U32 UseLength;            //data length
    T_U32 UseExcess;            //data length high
    T_U16 FolderNum;         //sub folder number.
    T_U16 FileNum;           //sub file nubmer
    T_U8 ShortName[12];      //83 format name.
    T_U8 NameSpace;          //long or short.
    T_U8 NTResType;          //Name/Ext: low or up, no matter Folder or File(eg:ATTR_NTRES_ALLLOW).
    T_U8 AsynERRFlag;    //asyn write error flag
    T_BOOL exist;            //
    T_BOOL SeriousClusFlag;  // it only is used by exfat
    T_BOOL IsChanged;   //Whether the file has been changed.
    T_U32 attr;              //READ_ONLY ....
    T_U32 ShareNum;          //share number.
    T_S32 AttrSem;           //Semaphore of the shared attribute of a file's handlers.
    T_U32 ClusPerAlloc;     //if file have no cluster, we will malloc the number everytime
    T_U32 FullPathLen;
    T_U32 FirstEmptyFDT;
    T_U32 AsynERRLow;
    T_U32 AsynERRHigh;
    T_U8  *FirstPageBuf;    //for improve video  record
    T_U32 FirstPageBufLen;
    T_POBJECT msg;           //cluster link.
}T_ATTR, *T_PATTR;

T_PATTR Attr_Initial(T_PDRIVER driver);
T_VOID Attr_Destroy(T_PATTR obj);
T_VOID Attr_Copy(T_PATTR dst, T_PATTR src);

#endif

