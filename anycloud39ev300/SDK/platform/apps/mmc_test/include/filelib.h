/*
 * @(#)Filelib.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _FILE_LIB_H_
#define        _FILE_LIB_H_

#include "file.h"
#include "filter.h"
#include "attr.h"
#include "ustring.h"
#include "fat.h"
#include "file_errortype.h"


//it only be used when we need get the file and floder number
#define FILTER_TOTAL      0x0400

#define FILE_INVALID_HANDLE     0


/*2013-12-30, the flag  for fast open the file,  and close not flush fat and fdt*/
#define    FILE_MODE_EXT_FAT_FDT_FIXED   0x1F


typedef struct tag_FileOpCtrl
{
    T_U32 IteRelDepth;
}T_FILEOPCTRL, *T_PFILEOPCTRL;
/* Defined in file.c to control the file's operation. */
//T_FILEOPCTRL g_OpCtrl = {2};

typedef enum tag_DeepMatchOp
{
    DEEPMATCH_FILE  = 0,    // Until find one matched file or until get to the deepest folder.
    DEEPMATCH_DEPTH = 1,    // Only search certain relative depth.

    DEEPMATCH_MAX   = 255
}E_DEEPMATCHOP;


/* Define the structure used in File_ExistDeepMatch. */
typedef struct tag_DeepMatchCtrl
{
    T_PSTRING str;    // Used to out put the absolute path under which matched file lies.
                      // You can set it to AK_NULL if you needn't the absolute path.
    T_U16 ParentLen;  // The length of the parent's absolute path.
    T_U8  OpType;     // Corresponding to E_DEEPMATCHOP.
    T_U8  DeepCnt;    // Used to count the depth we have iterated.
}T_DEEPMATCHCTRL, *T_PDEEPMATCHCTRL;

typedef struct tag_FindInfo
{
    T_U32 FileCnt;   // Only type==FILTER_TOTAL can changed it.
    T_U32 FolderCnt; // Only type==FILTER_TOTAL can changed it.
    T_U32 NodeCnt;   // How many total node in the following blink.
    T_PBLINK blink;  // Record the fileinfo of those searched files.
}T_FINDINFO, *T_PFINDINFO;




typedef struct tag_FindCtrl
{
    /* Must be the first since the whole body may be tranformed into it. */
    T_FINDINFO findInfo;

    T_U16 type;      // Control the dirs' function: such as FILTER_TOTAL.
    T_U16  FakeFolder;// The length of the pattern, if no use, set it into 0.
    T_U32 dspCtrl;   // Control whether to display "../" and "./"
    T_S32 count;     // How many sub-files need to read, forward or backward.
    T_U32 lastFdt;   // The last fdt under the parent (Only type==FILTER_TOTAL can changed it.).
    T_U32 dspIndx;   // The index number of the first file in blink relative to all folders/files.
    T_U32 PathLen;   // The parent's length of its absolute path(excluding '\0').
    T_PFILE parent;
    T_U16 *pattern;  // Compare with Filter_Match, the buffer was provided by caller.
    T_U16 *fileter_pattern;  // Compare with Filter_Match to fileter, the buffer was provided by caller.
}T_FINDCTRL, *T_PFINDCTRL;


typedef struct tag_Find_FolderInfo
{
    T_U32  driver;               //
    T_U32  first_parentID;       //第一层的根目录
    T_U16 *fileName;             //
    T_S32  find_cnt;             //向前还是向?
    T_U32  folder_lastfdt;       //向前还是向后?
    T_U32  folder_firstfdt;      //第一个
    T_U16 *pattern;              // Compare with Filter_Match, the buffer was provided by caller.
    T_U16 *fileter_pattern;      // Compare with Filter_Match to fileter, the buffer was provided by caller.
    T_U8   updatedir_flag;       //向上一层查找的标志
    T_U8   Dirorfolder_flag;     //是根目录还是文件夹的标志
    T_BOOL deep_flag;   //如果向前查找最后一个
    T_BOOL forward_find_flag;   //如果向前查找最后一个
    T_BOOL backward_find_flag;   //如果向前查找最后一个
}T_FIND_FOLDERINFO, *T_PFIND_FOLDERINFO;





/*
 get fs lib version
*/


/*
Create a file by its handler.
*/
T_BOOL File_CreateFile(T_PFILE parent, T_PFILE file);

/*
open file by abstract path and parent director and read/write mode.
if path is absolute then parent is invalid.
if parent is null then path's parent is system current path.
if path is invalid, return AK_NULL
see as File_OpenId, File_OpenSub, File_OpenNext
*/
T_PFILE File_Open(T_PFILE parent, T_PSTRING path, T_U32 mode);
T_PFILE File_CreateOpen(T_PFILE parent, T_PSTRING pathname, T_U32 mode);

/*
open file by a known id. usually the file is directory.
see as File_Open, File_OpenSub, File_OpenNext.
*/
T_PFILE File_OpenId(T_PDRIVER driver, T_U32 id, T_U32 ParentID, T_U8 AsynFlag, T_U32 FDTOffse, T_U32 FileAttr);


/* get parent's files by filter and insert these files to blink.
    update parent->attr->FileNum and parent->attr->FolderNum
    SubTotal is total insert blink's files;
*/
T_PBLINK File_DirsFileInfo(T_PFILE parent, T_U32 *SubTotal, T_PFILTER filter, T_U8 SemFlag);
T_U32 File_GetLengthWithoutSem(T_PFILE file, T_U32 *excess);

//open file by attr.
T_PFILE File_Initial(T_PATTR attr);


//create a new folder.
T_BOOL File_Mkdir(T_PFILE file);
//create a folder, include middle folder.
T_BOOL File_Mkdirs(T_PSTRING path);
//delete a old file or folder.
T_BOOL File_DeleteHandle(T_PFILE file);

//
T_U32 File_DelDir_DelCallBack(T_PFILE parent, F_DelCallback delCallBack, T_VOID *delCallBackData);



//read a byte from a data file.
T_U16 File_ReadChar(T_PFILE file);

//write a byte to data file
T_BOOL File_WriteChar(T_PFILE file, T_U8 ch);

/* Deep check whether the parent has any files(not folder) matched with pattern.
   ctrl is used to control the operation. ctr->str is to store the absolute path
   of the first founded folder which holds matched file. But before, str->data
   must be intialized to the parent's absolute path, and str->ptr is how many
   characters the str->data has, including the '\0'.
   Besides, str->data's last character should NOT be '/' or '\\'. Of course you
   can set it to AK_NULL if you don't want to get it.
Note: ctrl->OpType can control whether it search to the deepest folder. If not,
      it will return AK_TRUE when iterate g_OpCtrl.IteRelDepth times. */
T_BOOL File_ExistDeepMatch(T_PFILE parent, T_U16 *pattern, T_PDEEPMATCHCTRL ctrl);
T_U32 File_DirsBackward(T_PFINDCTRL pFindCtrl);
T_PFINDCTRL File_FindFirst1(const T_U16 *path, T_PFINDBUFCTRL bufCtrl);
//copy file to a folder.
T_PFILE File_CopyFile(T_PFILE file, T_PFILE Folder, T_BOOL replace, T_U8 *ExitFlag, T_U8 FolderExitFlag, F_CopyCallback pCallBack, T_VOID *pCallBackData);

//copy some files of source folder to dest folder by filter.
T_U32 File_CopyFolder(T_PFILE source, T_PFILE dest, T_BOOL replace, T_U8 *ExitFlag, T_U8 FolderExitFlag, F_CopyCallback pCallBack, T_VOID *pCallBackData);

//copy a data file to other file.
T_BOOL File_CopyData(T_PFILE source, T_PFILE dest, T_U8 *ExitFlag, F_CopyCallback pCallBack, T_VOID *pCallBackData);

/*
* @brief : To clean file's content without releasing its space.
* @author:
* @date  : 2008-1-3
* @param : file's pointer.
* @return: AK_TRUE - success.
* @Attention: It would not release file's space,
*  just clean its content.
*/
T_BOOL File_Clean(T_PFILE file);

/*
get file's absolute path. it include device number, abstract path and last name.
see the member named ShortName of attr object.
*/
T_PSTRING File_GetPath(T_U8 *obj);

/*
it will add dir size with one cluster in the exfat file system
*/
T_BOOL File_AddDirSize(T_PDRIVER driver,T_PSTRING FullPath, T_PFATSEARCH sm, T_U32 FileID);
T_VOID File_Flush1(T_PFILE file, T_U8 ClearFlag);
T_BOOL File_ReadPublicData(T_PFILE file);
T_PFINDCTRL File_FindFirstWithHandle(T_PFILE file, T_PFINDBUFCTRL bufCtrl);
T_VOID File_FindCloseWithHandle(T_U32 obj);
T_VOID File_ModifyRWCnt(T_PDRIVER driver, T_U8 flag);
T_BOOL File_NotEmptyFolder(T_PFILE folder);
#endif

