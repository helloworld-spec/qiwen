#ifndef _TEST_FUN_H_
#define _TEST_FUN_H_

#include "file.h"
#include "mtdlib.h"

typedef struct tag_DriverTestInfo

{

    T_U32 SectorSize;//扇区大小，以字节为单位

    T_U32 ClusterSize;//簇大小，以字节为单位

    T_U8 FatType;//该文件所在磁盘的文件系统类型，如FAT32等

}T_DRIVERTESTINFO, *T_PDRIVERTESTINFO;

typedef struct tag_FileTestInfo

{

   T_U32 BufferSize;//设置的buffersize大小，以字节为单位

T_U32 FileSize;//预设置的FileSize，以字节为单位

 

}T_FILETESTINFO, *T_PFILETESTINFO;


typedef struct _FAT_INFO_TMP
{
	T_U32 start;		//起始簇号
	T_U32 cnt;		//连续簇的数目
}FAT_INFO_TMP, *PFAT_INFO_TMP;




/************************************************************************
 * NAME:        Driver_GetFormatType
 * FUNCTION  此函数功能是得到相应分区所支持的文件系统格式
 * PARAM:      T_U8 DriverID :分区相应的ID
                        T_U8 *SupportType :分区可支持的文件系统类型,为0,将表示结束
                    
 * RETURN:     如果相应分区不存在将返回失败,否则成功.
**************************************************************************/

T_BOOL Driver_GetFormatType(T_U8 DriverID,T_U8 *SupportType);


/************************************************************************
 * NAME:        Driver_GetTestInfo
 * FUNCTION  此函数功能是得到分区的一些属性信息
 * PARAM:      T_U8 DriverID :分区相应的ID
                        T_PDRIVERTESTINFO  pDriverTestInfo :要得到的属性结构体
                    
 * RETURN:     如果相应分区不存在将返回失败,否则成功.
**************************************************************************/

T_BOOL Driver_GetTestInfo(T_U8 DriverID,  T_PDRIVERTESTINFO  pDriverTestInfo);


/************************************************************************
 * NAME:        File_GetTestInfo
 * FUNCTION  此函数功能是得到相应文件的一些属性信息
 * PARAM:      T_U32 file :文件句柄
                        T_PFILETESTINFO FileTestInfo :要得到的属性结构体
                    
 * RETURN:     如果是文件夹将返回失败,否则成功.
**************************************************************************/

T_BOOL File_GetTestInfo(T_U32 file,  T_PFILETESTINFO FileTestInfo);


/************************************************************************
 * NAME:        FS_GetDirectoryFdtInfo
 * FUNCTION  此函数功能是得到文件夹总共有多少个空的FDT项和连续的空的FDT项数目
 * PARAM:      T_U32 FolderHandle :文件夹句柄
                        T_U32 *TolFdt :此文件夹总共有多少个空的FDT项,包括没有用的和已经打上0XE5删除标志的
                        T_U32 *SeriousFdt :此文件夹中最大的连续的空的FDT项
 * RETURN:     如果是文件返回失败,否则成功.
**************************************************************************/

T_BOOL FS_GetDirectoryFdtInfo(T_U32 FolderHandle, T_U32 *TolFdt, 
    T_U32 *SeriousFdt, T_U32 *SectorAddr, T_U32 * SectorOff);


/************************************************************************
 * NAME:        FS_GetLastUpdateCluster
 * FUNCTION  此函数功能是得到文件最后一个被更新的簇的簇号
 * PARAM:      T_U32 FileHandle :文件句柄
 * RETURN:     返回最后一次被更新簇号.
**************************************************************************/

T_U32 FS_GetLastUpdateCluster(T_U32 FileHandle);

/************************************************************************
 * NAME:        FS_DriverGetNextCluster
 * FUNCTION  此函数功能是得到FAT表中相应的下一簇的信息
 * PARAM:      T_U32 DriverID :分区相应的ID
                        T_U32 CurCluster :当前簇的信息
 * RETURN:    直接返回FAT表中读到的信息.
**************************************************************************/

T_U32 FS_DriverGetNextCluster(T_U32 DriverID, T_U32 CurCluster);

/************************************************************************
 * NAME:        FS_DriverGetClusterPostion
 * FUNCTION  此函数功能是得到对应簇在保存在分区上的位置信息
 * PARAM:      T_U32 DriverID :分区相应的ID
                        T_U32 CurCluster :当前簇的信息
                        T_U32 *SectorAddr:当前簇所在分区的扇区地址
                        T_U32 *SectorOff :当前簇所在分区的扇区偏移
 * RETURN:    返回操作是否成功
**************************************************************************/

T_BOOL FS_DriverGetClusterPostion(T_U32 DriverID, T_U32 CurCluster, T_U32 *SectorAddr, T_U32 *SectorOff);

/************************************************************************
 * NAME:        FS_GetFileFatBufInfo
 * FUNCTION  此函数功能是得到文件簇链的信息
 * PARAM:      T_PFILE FileHandle :文件
 * RETURN:     返回簇链的缓冲区
 example:
    t_u8 *data; //这是你们得到的缓冲区,NewFatInfo.start这是一组连续的开始
    NewFatInfo.cnt这是连续一组的个数
    while (*data)
    {
        data += Cluster_GetBlock(data , &NewFatInfo.cnt, &NewFatInfo.start, &relative, &normal);
        Fat_AddUseFatBuf(driver, &NewFatInfo);
    }
**************************************************************************/
T_U8 Cluster_GetBlock(const T_U8 *data, T_U32* count, T_U32* RealAddr, T_U32* relative, T_BOOL* normal);

T_U8* FS_GetFileFatBufInfo(T_U32 FileHandle, T_U32 *TolSize);

/************************************************************************
 * NAME:        FS_GetDriverFatBufInfo
 * FUNCTION  此函数功能是得到文件系统内部FAT缓冲区的信息
 * PARAM:      T_U32 DriverID :分区相应的ID
                        PFAT_INFO FatInfo :保存缓冲区的信息的BUFFER
                        T_U8 flag :文件系统中有两个缓冲区,如果这个标志为TRUE的话,将返回空闲簇缓冲区,否则返回使用簇缓冲区
 * RETURN:    返回缓冲区有效数目
**************************************************************************/

T_U32 FS_GetDriverFatBufInfo(T_U8 DriverID, PFAT_INFO_TMP FatInfo, T_U8 flag);

/************************************************************************
 * NAME:        FS_GetDriverFatSectorSize
 * FUNCTION  此函数功能是得到分区中的FAT表的信息
 * PARAM:      T_U32 DriverID :分区相应的ID
                        T_U32 *FatNum :此分区总共有的簇数目
                        
 * RETURN:    返回FAT表总共占用的扇区数目
**************************************************************************/

T_U32 FS_GetDriverFatSectorSize(T_U8 DriverID, T_U32 *FatNum);

/************************************************************************
 * NAME:        FS_GetDriverFatInfo
 * FUNCTION  此函数功能是得到读取FAT的信息
 * PARAM:      T_U32 DriverID :分区相应的ID
                        T_U32 addr: FAT表的地址,此地址是相对FAT表的地址,也就是FAT表的第一个扇区为0,地址是以扇区为单位
                        T_U32 size:要读FAT表的扇区数目
                        T_U8 * FatBuf: fat表的数据缓冲区
 * RETURN:     返回操作成功与失败
**************************************************************************/

T_BOOL FS_GetDriverFatInfo(T_U8 DriverID, T_U32 addr, T_U32 size, T_U8 * FatBuf);
T_U32 Driver_GetObjDriver(T_U8 DeviceId);
T_U8  Driver_GetObjLength(T_VOID);
T_U32 FS_Driver_Format(T_PMEDIUM medium, T_U32 start,
                        T_U32 total, T_U32 BufLen, T_U32 code,T_U8 FormatType);


/************************************************************************
 * NAME:        File_FindFirst
 * FUNCTION  find some files in input path by bufCtrl condition
 * PARAM:      const T_U16 *path
                      T_PFINDBUFCTRL bufCtrl
 * RETURN:     it will return the find handle, return 0 if it fails, otherwise return the handle
**************************************************************************/

T_U32 TEST_File_FindFirst(const T_U16 *path, T_PFINDBUFCTRL bufCtrl);

/************************************************************************
 * NAME:        File_FindNext
 * FUNCTION  find next files with the finding hanle
 * PARAM:      T_U32 pFindCtrl
                      T_S32 Cnt     the need file number
 * RETURN:     it will return the found file number
**************************************************************************/

T_U32 TEST_File_FindNext(T_U32 pFindCtrl, T_S32 Cnt);

/************************************************************************
 * NAME:        File_FindClose
 * FUNCTION   it will release all resource of the finding handle.
 * PARAM:      T_U32 pFindCtrl
 * RETURN:     NONE
**************************************************************************/

T_VOID TEST_File_FindClose(T_U32 pFindCtrl);

/************************************************************************
 * NAME:        File_FindInfo
 * FUNCTION  get file info of the input pFindCtrl
 * PARAM:      T_U32 pFindCtrl
                      T_U32 position :the file's position of pFindCtrl's blink 
                      T_U32 *FileCnt :     file count
                      T_U32 *FolderCnt :     folder count
 * RETURN:     it will return file info structure pointer 
**************************************************************************/

T_PFILEINFO TEST_File_FindInfo(T_U32 pFindCtrl, T_U32 Position, T_U32 *FileCnt, T_U32 *FolderCnt);


/************************************************************************
 * NAME:        File_Close
 * FUNCTION  reflush write data to medium and delete refrence from system.
                      and destroy a file object to free space.
 * PARAM:      T_U32 file
 * RETURN:     NONE 
**************************************************************************/

T_VOID TEST_File_Close(T_U32 file);


/************************************************************************
 * NAME:        File_OpenAsc
 * FUNCTION  it will open file with the asc file name and open mode
 * PARAM:      T_U32 parent   the parent directory handle
                       const T_U8* FileName     the file name
                       T_U32 mode                     the open mode
 * RETURN:     return the file hanle, we will use File_IsFile to decide if the file is exist. 
**************************************************************************/

T_U32 TEST_File_OpenAsc(T_U32 parent, const T_U8* FileName, T_U32 mode);


/************************************************************************
 * NAME:        File_OpenUnicode
 * FUNCTION  it will open file with the unicode file name and open mode
 * PARAM:      T_U32 parent   the parent directory handle
                       const T_U8* FileName     the file name
                       T_U32 mode                     the open mode
 * RETURN:     return the file hanle, we will use File_Exist to decide if the file is exist. 
**************************************************************************/

T_U32 TEST_File_OpenUnicode(T_U32 parent, const T_U16* FileName, T_U32 mode);

/************************************************************************
 * NAME:        File_IsFolder
 * FUNCTION  it will check the file handle if it is directory
 * PARAM:      T_PFILE file
 * RETURN:     if file is exist and file is a directory return AK_TRUE else return AK_FALSE
**************************************************************************/

T_BOOL TEST_File_IsFolder(T_U32 file);

/************************************************************************
 * NAME:        File_IsFile
 * FUNCTION  it will check the file handle if it is file
 * PARAM:      T_PFILE file
 * RETURN:     if file is exist and file is a data file return AK_TRUE else return AK_FALSE
**************************************************************************/

T_BOOL TEST_File_IsFile(T_U32 file);

/************************************************************************
 * NAME:        File_Exist
 * FUNCTION  it will check if the file handle is exist
 * PARAM:      T_PFILE file
 * RETURN:     return AK_TRUE if the handle is exist ,otherwise return AK_FALSE
**************************************************************************/

T_BOOL TEST_File_Exist(T_U32 file);

/************************************************************************
 * NAME:        File_GetLength
 * FUNCTION  it will get the file handle iformation
 * PARAM:      T_PFILE file
                       T_U32* excess        the high 32 bit of the file length, or the folder number
 * RETURN:     return file's length low 32 bit. and set high. If the file is a directory, then
                       high records its sub folder's num and the return value is its sub file's num
**************************************************************************/

T_U32 TEST_File_GetLength(T_U32 file, T_U32* excess);


/************************************************************************
 * NAME:        File_GetOccupiedSpace
 * FUNCTION  it will get the file handle iformation
 * PARAM:      T_PFILE file
                       T_U32* high        the high 32 bit of the file length 
 * RETURN:     the file had used the low 32 bit of the file of the room
**************************************************************************/

T_U32 TEST_File_GetOccupiedSpace(T_U32 file, T_U32 *high);

/************************************************************************
 * NAME:        File_DelAsc
 * FUNCTION  delete a old file or folder.
 * PARAM:      const T_U8 *FileName the asc file name 
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL TEST_File_DelAsc(const T_U8 *FileName);

/************************************************************************
 * NAME:        File_DelUnicode
 * FUNCTION  delete a old file or folder.
 * PARAM:      const T_U8 *FileName the unicode file name 
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/
T_BOOL TEST_File_DelUnicode(const T_U16 *FileName);

/************************************************************************
 * NAME:        File_RenameTo
 * FUNCTION  rename or move file
 * PARAM:      T_PFILE source
                      T_PFILE dest
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL TEST_File_RenameTo(T_U32 source, T_U32 dest);

/************************************************************************
 * NAME:        File_Seek
 * FUNCTION  move data file read /write pointer.
 * PARAM:      T_PFILE file
                      T_U32 LowPos      the file low 32 bit
                      T_U32 *HighPos    the file high 32 bit, it will return the high 32 bit of file point 
 * RETURN:     return the  low 32 bit of the file point
**************************************************************************/

T_U32 TEST_File_Seek(T_U32 file, T_U32 LowPos, T_U32 *HighPos);

/************************************************************************
 * NAME:        File_Read
 * FUNCTION  read data from a data file.
 * PARAM:      T_PFILE file
                      T_pVOID buf
                      T_U32 byts
 * RETURN:     return the read size
**************************************************************************/

T_U32 TEST_File_Read(T_U32 file, T_pVOID buf, T_U32 byts);

/************************************************************************
 * NAME:        File_Write
 * FUNCTION  write data from a data file.
 * PARAM:      T_PFILE file
                      T_pVOID buf
                      T_U32 byts
 * RETURN:     return the write size
**************************************************************************/

T_U32 TEST_File_Write(T_U32 file, T_pVOID buf, T_U32 size);

/************************************************************************
 * NAME:        File_DelDir
 * FUNCTION  delete all file and folder of a folder.
 * PARAM:      T_PFILE folder
 * RETURN:     return the delete file and folder number 
**************************************************************************/

T_U32 TEST_File_DelDir(T_U32 folder);

/************************************************************************
 * NAME:        File_DelFile
 * FUNCTION  delete file
 * PARAM:      T_PFILE file
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL TEST_File_DelFile(T_U32 file);

/************************************************************************
 * NAME:        File_MkdirsAsc
 * FUNCTION  create a folder by ascii path, include middle folder
 * PARAM:      const T_U8* path
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

//create a folder by ascii path, include middle folder.
T_BOOL TEST_File_MkdirsAsc(const T_U8* path);

/************************************************************************
 * NAME:        File_MkdirsUnicode
 * FUNCTION  create a folder by unicode path, include middle folder
 * PARAM:      const T_U16* path
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL TEST_File_MkdirsUnicode(const T_U16* path);

/************************************************************************
 * NAME:        File_Flush
 * FUNCTION  flush last data to file.
 * PARAM:      T_PFILE file
 * RETURN:     NONE
**************************************************************************/

T_VOID TEST_File_Flush(T_U32 file);

/************************************************************************
 * NAME:        File_SetDefault
 * FUNCTION  if the file is folder, set it is defualt path
 * PARAM:      T_PFILE file
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL TEST_File_SetDefault(T_U32 file);

/************************************************************************
 * NAME:        File_SetDefault
 * FUNCTION  get file's attribute by ascii path, such as folder(0x10)
 * PARAM:      const T_U8* path
 * RETURN:     return the file current attribute
**************************************************************************/

T_U32 TEST_File_GetAttrAsc(const T_U8* path);

/************************************************************************
 * NAME:        File_GetAttrUnicode
 * FUNCTION  get file's attribute by unicode path, such as folder(0x10)
 * PARAM:      const T_U16* path
 * RETURN:     return the file current attribute
**************************************************************************/

T_U32 TEST_File_GetAttrUnicode(const T_U16* path);

/************************************************************************
 * NAME:        File_GetAttrUnicode
 * FUNCTION  set file's attribute by ascii path.
 * PARAM:      const T_U16* path
                      E_SETFILEATTR attribute
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL TEST_File_SetAttrAsc(const T_U8* path, E_SETFILEATTR attribute);


/************************************************************************
 * NAME:        File_SetAttrUnicode
 * FUNCTION  set file's attribute by unicode path.
 * PARAM:      const T_U16* path
                      E_SETFILEATTR attribute
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL TEST_File_SetAttrUnicode(const T_U16* path, E_SETFILEATTR attribute);

/************************************************************************
 * NAME:        File_Truncate
 * @FUNCTION：Truncate an open file.
 * @PARAM:  T_PFILE file -- the file you want to truncate.
                     T_U32 length -- the new length of the file,
                        it must be less than the file's length
 * @RETURN  : truncate successfully or not
 * @Attention: The file to be truncated couldn't be File_Destroy().
**************************************************************************/

T_BOOL TEST_File_Truncate(T_U32 file, T_U32 low, T_U32 high);

/************************************************************************
 * NAME:        File_SetBufferSize
 * FUNCTION  SectorSize must be 2^n! If not, the file->BufLenMask may not work!
 * PARAM:      T_PFILE file
                      T_U32 SectorNum
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL TEST_File_SetBufferSize(T_U32 file, T_U32 SectorNum);

/************************************************************************
 * @FUNCTION：Get the Mod time of the input file.
 * @PARAM   : T_PFILE file -- the file whose time you want to get.
              T_PFILETIME fileTime -- the structure used to put file time.
 * @RETURN  : Get ModTime successfully or not.
**************************************************************************/
T_BOOL TEST_File_GetModTime(T_U32 file, T_PFILETIME fileTime);

/************************************************************************
 * @FUNCTION：Get the Create time of the input file.
 * @PARAM   : T_PFILE file -- the file whose time you want to get.
              T_PFILETIME fileCTime -- the structure used to put file time.
 * @RETURN  : Get CreateTime successfully or not.
**************************************************************************/
T_BOOL TEST_File_GetCreateTime(T_U32 file, T_PFILETIME fileCTime);

/************************************************************************
 * NAME:        File_GetFileinfo
 * FUNCTION  get file info of the input file
 * PARAM:      T_PFILE file
                       T_PFILEINFO fileInfo
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL TEST_File_GetFileinfo(T_U32 file, T_PFILEINFO fileInfo);

/************************************************************************
 * NAME:        File_GetFileinfo
 * FUNCTION  copy  source file to dest file by ascii format.
 * PARAM:      const T_U8* srcPath
                      const T_U8* dstPath
                      T_BOOL replace    if it is ak_TRUE, we will replace the same file name
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL  TEST_File_CopyAsc(const T_U8* srcPath, const T_U8* dstPath, T_BOOL replace, F_CopyCallback pCallBack, T_VOID *pCallBackData);


/************************************************************************
 * NAME:        File_GetFileinfo
 * FUNCTION  copy  source file to dest file by unicode format.
 * PARAM:      const T_U16* srcPath
                      const T_U16* dstPath
                      T_BOOL replace    if it is ak_TRUE, we will replace the same file name
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL  TEST_File_CopyUnicode(const T_U16* srcPath, const T_U16* dstPath, T_BOOL replace, F_CopyCallback pCallBack, T_VOID *pCallBackData);

/************************************************************************
 * NAME:        File_GetFilePtr
 * FUNCTION  get the file point
 * PARAM:      T_PFILE file
                      T_U32 *HighPos    the file high 32 bit, it will return the high 32 bit of file point 
 * RETURN:     return the  low 32 bit of the file point
**************************************************************************/

T_U32 TEST_File_GetFilePtr(T_U32 file, T_U32 *high);

/************************************************************************
 * NAME:        File_SetFilePtr
 * FUNCTION  set the file point
 * PARAM:      T_PFILE file
                      T_U32 offset      the new offset
                      T_U16 origin      the mode
 * RETURN:     return the  low 32 bit of the file point
**************************************************************************/

T_U32 TEST_File_SetFilePtr(T_U32 file, T_S32 offset, T_U16 origin);

/************************************************************************
 * NAME:        File_GetPathObj
 * FUNCTION  get file's absolute path. it include device number, abstract path and last name.
 * PARAM:      T_PFILE file
 * RETURN:     return the  path name information
**************************************************************************/

T_U32 TEST_File_GetPathObj(T_U32 file);

/************************************************************************
 * NAME:        File_GetAbsPath
 * FUNCTION  get file name from the handle of File_GetPathObj
 * PARAM:      T_U32 obj
 * RETURN:     return the  path name 
**************************************************************************/
T_U16* TEST_File_GetAbsPath(T_U32 obj);

/************************************************************************
 * NAME:        File_DestroyPathObj
 * FUNCTION  free the ram of File_GetPathObj
 * PARAM:      T_U32 obj
 * RETURN:     NONE
**************************************************************************/
T_VOID TEST_File_DestroyPathObj(T_U32 obj);

/************************************************************************
 * NAME:        File_SetFileSize
 * FUNCTION  it will malloc some room to the file, and make the file have serious room
 * PARAM:      T_PFILE file
                      T_U32 fileSize    the file size
 * RETURN:     return the  path name information
**************************************************************************/
T_BOOL TEST_File_SetFileSize(T_U32 file, T_U32 fileSize);

/************************************************************************
 * NAME:        TEST_File_Close
 * FUNCTION  reflush write data to medium and delete refrence from system.
                      and destroy a file object to free space without file_flush.
 * PARAM:      T_PFILE file
 * RETURN:     NONE 
**************************************************************************/

T_VOID TEST_File_CloseNOFlush(T_U32 handle);

T_VOID TEST_Global_UninstallDriver(T_U32 obj);


T_VOID TEST_Driver_Destroy(T_U32 driver);
#endif

