
#ifndef        _FILE_ERRORTYPE_H_
#define        _FILE_ERRORTYPE_H_





typedef enum error_type
{
    //file.c
    
    // file_open   
    FILEOPEN_PATHNAME_OPENMODE_ERROR                        = 1,           //AK_NULL == pathname || OpenMode > FILE_MODE_APPEND
    FILEOPEN_STR_AK_NULL_ERROR                              = 2,           //AK_NULL == str || 0 == *str
    FILEOPEN_DRIVER_AK_NULL_ERROR                           = 3,           //AK_NULL == driver
    FILEOPEN_SM_AK_NULL_ERROR                               = 4,           //AK_NULL == sm
    FILEOPEN_NAME_AK_NULL_ERROR                             = 5,           //AK_NULL == name
    FILEOPEN_RESULT_FS_SEARCH_ERROR_ERROR                   = 6,           //FS_SEARCH_ERROR == result
    FILEOPEN_ASYNFLAG_ERROR                                 = 7,          //you don't open the file with asyn mode and no-asyn mode
    FILEOPEN_ATTR_AK_NULL_ERROR                             = 8,          //attr == AK_NULL
    FILEOPEN_OPENMODE_FILE_MODE_READ_ERROR                  = 9,          //file is not exist,if openmode !=  FILE_MODE_READ, open file AK_FALSE 
    
    //File_OpenId                                                                                                                      
    FILEOPENID_DRIVER_AK_NULL_ERROR                         = 10,         //AK_NULL == driver || id > 0x0FFFFFFF
    
    //File_Read
    FILEREAD_SIZE_EQUAL_ZERO_ERROR                          = 11,          //0 == size
    FILEREAD_FILE_AK_NULL_ERROR                             = 12,          //file == AK_NULL||the file is not colseflag || the file is not directory||AK_NULL == buf1||the file not exist
 
    //File_ReadWithoutSem
    FILEREADWITHOUTSEM_SIZE_EQUAL_ZERO_ERROR                = 13,          //0 == size
    FILEREADWITHOUTSEM_READPUBLICDATA_ERROR                 = 14,          //File_ReadPublicData return ak_false

    //File_Write
    FILEWRITE_SIZE_EQUAL_ZERO_ERROR                         = 15,          //0 == size
    FILEWRITE_FILE_AK_NULL_ERROR                            = 16,          //file == AK_NULL||the file is not colseflag || the file is not directory||AK_NULL == buf1||the file not exist

    //File_WriteWithoutSem
    FILWRITEWITHOUTSEM_WRITE_THAN_4G_ERROR                  = 17,           //fat->fs != FAT_FS_EX, FSLib - File_WriteBlock():: write data length > 4G
    FILWRITEWITHOUTSEM_ATTR_EXIST_ERROR                     = 18,           //the file is not exist
    FILWRITEWITHOUTSEM_FILE_ONLYREAD_ERROR                  = 19,           //the openmode of the file FILE_MODE_READ
    FILWRITEWITHOUTSEM_READPUBLICDATA_ERROR                 = 20,           //File_ReadPublicData return ak_false 

    //File_Truncate
    FILETRUNCATE_FILE_IS_ONLYREAD_ERROR                     = 21,            //the openmode of the file is FILE_MODE_READ
    FILETRUNCATE_TRUNCATE_SIZE_EQUAL_ZERO_ERROR             = 22,            //Truncate the file size is 0
    FILETRUNCATE_ATTR_SHARENUM_THAN_TWO_ERROR               = 23,            //the file is opened two times(attr->ShareNum > 1)
    FILETRUNCATE_TRUNCATE_SIZE_VAILD_ERROR                  = 24,            //the size of the file is Truncated  is valid
    FILETRUNCATE_SM_AK_NULL_ERROR                           = 25,            //sm malloc ak_fals
    FILETRUNCATE_TRUNCATE_SIZE_LONGER_ERROR                 = 26,            //the size of the file is Truncated  longer than filelen
   
    //File_SetFileSize
    FILESETFILESIZE_FILE_AK_NULL_ERROR                      = 27,            //file == AK_NULL || file->CloseFlag != CLOSE_FLAG_OK || File_IsFolder(file) == AK_TRUE

    //File_SetFilePtr
    FILESETFILEPTR_FILE_AK_NULL_ERROR                       = 28,             //the file is  not exist
    FILESETFILEPTR_FILE_SEEK_SET_OFFSET_ERROR               = 29,             // FILE_SEEK_SET , file offset size < 0 
    FILESETFILEPTR_FILE_SEEK_CUR_OFFSET_ERROR               = 30,             // FILE_SEEK_SET , if offset < 0 then data > point return -1
    FILESETFILEPTR_FILE_SEEK_END_OFFSET_ERROR               = 31,             // FILE_SEEK_END , file offset size > 0 
    FILESETFILEPTR_FILE_SEEK_END_DATA_OFFSET_ERROR          = 32,             // FILE_SEEK_END , DATA> offset, and hing == 0 
    FILESETFILEPTR_ERROR_ORIGIN_ERROR                       = 33,             // File_SetFilePtr:it error at origin

    //File_SetBufferSize
    FILESETBUFFERSIZE_SECTORNUM_ERROR                       = 34,             //0 == SectorNum || SectorNum > 256
    FILESETBUFFERSIZE_TMPSECNUM_ERROR                       = 35,             //TmpSecNum != 0 
    FILESETBUFFERSIZE_DATABUF_AK_NULL_ERROR                 = 36,             //DataBuf remalloc AK_NULL

   // File_SetAttrUnicode
    FILESETATTRUNICODE_FILE_IS_NOT_EXIS_ERROR               = 37,               //the file is not exist

    //File_SetAttrAsc
    FILESETATTRASC_FILE_IS_NOT_EXIS_ERROR                   = 38,              //the file is not exist

    //File_Seek
    FILESEEK_FILE_AK_NULL_ERROR                             = 39,              //file == AK_NULL || file->CloseFlag != CLOSE_FLAG_OK || !(((T_PATTR)file->attr)->exist)

    //File_RenameTo
    FILERENAMETO_SOURCE_AND_DEST_AK_NULL_ERROR              = 40,            //AK_NULL == source || AK_NULL == dest || source->CloseFlag != CLOSE_FLAG_OK || dest->CloseFlag != CLOSE_FLAG_OK
    FILERENAMETO_SRCATTR_EQUAL_DESTATTR_ERROR               = 41,             //srcAttr == dstAttr
    FILERENAMETO_SRCATTR_SHARENUM_TWO_ERROR                 = 42,             //the srcfile is opened more than two tims
    FILERENAMETO_SRCFILE_NOT_EXIST_ERROR                    = 43,             //the srcfile is renamed to is not exist
    FILERENAMETO_SRCATTR_FILEID_EQUAL_ZERO_ERROR            = 44,              //0 ==srcAttr->FileId  && (srcAttr->attr & 0x10) != 0)
    FILERENAMETO_DESTFILE_EXIST_ERROR                       = 45,              //the dstfile  is  exist
    FILERENAMETO_SRCFOLDER_DESTFOLDER_SAMEFOLDER_ERROR      = 46,             //dstAttr->ParentId != srcAttr->ParentId && (srcAttr->attr & 0x10) != 0

    //File_ReadPublicData
    FILEREADPUBLICDATA_ATTR_SHARENUM_ONE_ERROR              = 47,              //the file is opened by ONE times

    //File_OpenUnicode
    FILEOPENUNICODE_FILENAME_AK_NULL_ERROR                  = 48,             //the filename is ak_null

    //File_MkdirsUnicode
    FILEMKDIRSUNICODE_PATH_AK_NULL_ERROR                    = 49,          // path is  AK_NULL
 
    //File_Mkdirs
    FILEMKDIRS_PATH_AK_NULL_ERROR                           = 50,        // path is  AK_NULL
    FILEMKDIRS_PTR_AK_NULL_ERROR                            = 51,        // *ptr is  AK_NULL
 
    //File_Mkdir
    FILEMKDIR_FILE_AK_NULL_ERROR                             = 52,        // file == AK_NULL || file->CloseFlag != CLOSE_FLAG_OK
    FILEMKDIR_FILE_ATTRIBUTE_DIRECTORY_ERROR                 = 53,        //  check whether the file is a directory. Besides, we must not make root directory!

    //File_IsHidden
    FILISHIDDEN_FILE_AK_NULL_ERROR                           = 54,       //file == AK_NULL || file->CloseFlag != CLOSE_FLAG_OK || !(((T_PATTR)file->attr)->exist)
    FILISHIDDEN_ATTR_HIDDEN_ERROR                            = 55,       //((T_PATTR)file->attr)->attr & ATTR_HIDDEN) == 0

    //File_IsFolder
    FILISFOLDE_FILE_AK_NULL_ERROR                            = 56,      //file == AK_NULL || file->CloseFlag != CLOSE_FLAG_OK || !(((T_PATTR)file->attr)->exist)
    FILISFOLDE_ATTR_DIRECTORY_ERROR                          = 57,      //(((T_PATTR)file->attr)->attr & ATTR_DIRECTORY) == 0

    //File_IsFile
    FILISFILE_FILE_AK_NULL_ERROR                             = 58,     //file == AK_NULL || file->CloseFlag != CLOSE_FLAG_OK || !(((T_PATTR)file->attr)->exist)
    FILISFILE_ATTR_DIRECTORY_ERROR                           = 59,     //(((T_PATTR)file->attr)->attr & ATTR_DIRECTORY) != 0

    //File_IsDirsEnd
    FILISDIREND_PFINDCTRL_ERROR                              = 60,      //0 == pFindCtrl->count|| (pFindCtrl->type & FILTER_TOTAL) != 0|| (pFindCtrl->type & FILTER_NOTITERATE) != 0

    //File_Initial
    FILEINITIAL_ATTR_AK_NULL_ERROR                           = 61,       //AK_NULL == attr || AK_NULL == attr->msg
    FILEINITIAL_FILE_MALLOC_AK_NULL_ERROR                    = 62,       //malloc file AK_NULL
    FILEINITIAL_SECTORBUF_MALLOC_AK_NULL_ERROR               = 63,       //malloc SectorBuf AK_NULL

   // File_GetPath
    FILEGETPATH_ATTR_AK_NULL_ERROR                           = 64,        //attr  ==  AK_NULL
   
    //File_GetParentPath
    FILEGETPARENTPATH_FILENAME_PARENTNAME_ERROR              = 65,        //!(FileName && ParentName)

    //File_GetOccupiedSpace
    FILEGETOCCUPIEDSPACE_FILE_AK_NULL_ERROR                  = 66,        //AK_NULL == file || file->CloseFlag != CLOSE_FLAG_OK || !(((T_PATTR)file->attr)->exist)
  

    //File_GetModTime
    FILEGETMODTIME_FILE_AK_NULL_ERROR                         = 67,        //AK_NULL == file || file->CloseFlag != CLOSE_FLAG_OK || !(((T_PATTR)file->attr)->exist)
    FILEGETMODTIME_FILETIME_AK_NULL_ERROR                     = 68,        //filetime ==  AK_NULL 

    //File_GetLengthWithoutSem
    FILEGETLENGTHWITHOUTSEM_FILE_AK_NULL_ERROR                = 69,         //AK_NULL == file || file->CloseFlag != CLOSE_FLAG_OK || !(((T_PATTR)file->attr)->exist)

    //File_GetLength
    FILEGETLENGTH_FILE_AK_NULL_ERROR                          = 70,         //AK_NULL == file || file->CloseFlag != CLOSE_FLAG_OK || !(((T_PATTR)file->attr)->exist) 

    //File_GetFileinfo
    FILEGETLEINFO_FILE_AK_NULL_ERROR                          = 71,         //AK_NULL == file || file->CloseFlag != CLOSE_FLAG_OK || !(((T_PATTR)file->attr)->exist)
    FILEGETLEINFO_FILEINFO_AK_NULL_ERROR                      = 72,         //AK_NULL == fileInfo 

   // File_GetCreateTime
    FILEGETCREATETIME_FILE_AK_NULL_ERROR                      = 73,          //AK_NULL == file || file->CloseFlag != CLOSE_FLAG_OK || !(((T_PATTR)file->attr)->exist)
    FILEGETCREATETIME_FILECTIME_AK_NULL_ERROR                 = 74,         //AK_NULL == fileCTime 

    //File_GetCode
    FILEGETCODE_PATH_AK_NULL_ERROR                            = 75,          //AK_NULL == path 

    //File_GetAttrUnicode 
    FILEGETATTRUNICODE_FILE_NOT_EXIST_ERROR                   = 76,          // if (!(fp_attr->exist)

    //File_GetAttrAsc 
    FILEGETATTRASC_FILE_NOT_EXIST_ERROR                       = 77,           // if (!(fp_attr->exist)

    //File_GetAbsPath
    FILEGETABSPATH_STR_AK_NULL_ERROR                          = 78,           //AK_NULL == str

    //File_GetABSFileName
    FILEGETABSFILENAME_PARENTNAME_AK_NULL_ERROR               = 79,           //AK_NULL == ParentName

    //File_FindNext
    FILEFINDNEXT_PFINDCTRL_AK_NULL_ERROR                      = 80,           //AK_NULL == pFindCtrl
    FILEFINDNEXT_CNT_ZERO_ERROR                               = 81,          //0 == Cnt || 0 == findInfo->NodeCnt
    FILEFINDNEXT_FILECNT_FOLDERCNT_ERROR                      = 82,         //(findInfo->FileCnt + findInfo->FolderCnt) <= findInfo->NodeCnt && 0 == (pFindCtrl->dspCtrl & FST_ENTER_FLAG)
    FILEFINDNEXT_FILTER_NOTITERATE_ERROR                      = 83,         //0 != (pFindCtrl->type & FILTER_NOTITERATE)
    FILEFINDNEXT_OFFSET_ERROR                                 = 84,           //Moving more than NodeCnt step is forbidden. 
    FILEFINDNEXT_OFFSET_TWO_ERROR                             = 85,           //Offset > findInfo->NodeCnt
    FILEFINDNEXT_REPEATING_NOTITERATE_ERROR                   = 86,           //Repeating search 0 != (pFindCtrl->type & FILTER_NOTITERATE

    //File_FindInfo
    FILEFINDINFO_PFINDCTRL_AK_NULL_ERROR                      = 87,           //AK_NULL == pFindCtrl
    FILEFINDINFO_FILEINFO_AK_NULL_ERROR                       = 88,          //AK_NULL == findInfo || Position >= findInfo->NodeCnt
    FILEFINDINFO_ITEM_AK_NULL_ERROR                           = 89,           //AK_NULL == item
    FILEFINDINFO_ITEM_NEXT_AK_NULL_ERROR                      = 90,           //AK_NULL == item->next

    //File_FindFirstWithHandle
    FILEFINDfiFIRSTWITHHANDLE_PFINDCTRL_AK_NULL_ERROR         = 91,          // pFindCtrl fRamAlloc AK_NULL

    //File_FindFirst1
    FILEFINDFIRST1_BUFCTRL_AK_NULL_ERROR                      = 92,          //AK_NULL == bufCtrl || AK_NULL == path || bufCtrl->NodeCnt == 0
    FILEFINDFIRST1_PATHLEN_ZERO_ERROR                         = 93,          //0 == PathLen
    FILEFINDFIRST1_PFINDCTRL_AK_NULL_ERROR                    = 94,          //malloc pFindCtrl ak_null

   //File_FindCloseWithHandle
    FILEFINDCLOSEWITHHANDLE_PFINDCTRL_AK_NULL_ERROR           = 95,        //pFindCtrl == AK_NULL

    //File_ExistDeepMatch
    FILEEXISTDEEPMATCH_PARENT_AK_NULL_ERROR                    = 96,        //parent == AK_NULL
    FILEEXISTDEEPMATCH_PFINDCTRL_AK_NULL_ERROR                 = 97,       // malloc  pFindCtrl == AK_NULL
    FILEEXISTDEEPMATCH_COUNT_ZERO_ERROR                        = 98,      //Count == 0

    //File_Exist
    FILEEXIST_FILE_AK_NULL_ERROR                               = 99,        //(AK_NULL == file || file->CloseFlag != CLOSE_FLAG_OK)

    //File_DirsFileInfoEx
    FILEDIRSFILEINFOEX_PFINDCTRL_AK_NULL_ERROR                 = 100,       //pFindCtrl == AK_NULL


    // File_DirsFileInfo
    FILEDIRSFILEINFO_FILTER_AK_NULL_ERROR                      = 101,       //filter == AK_NULL
    FILEDIRSFILEINFO_SM_AK_NULL_ERROR                          = 102,       //malloc sm AK_NULL
    FILEDIRSFILEINFO_BLINK_AK_NULL_ERROR                       = 103,       //malloc blink AK_NULL
    FILEDIRSFILEINFO_TOTAL_ZERO_ERROR                          = 104,       //0 == total

    //File_DirsBackward
    FILEDIRSBACKWARD_PFINDCTRL_AK_NULL_ERROR                   = 105,        //AK_NULL == pFindCtrl || !File_IsFolder(pFindCtrl->parent)
    FILEDIRSBACKWARD_PFINDCTRL_COUNT_ERROR                     = 106,        //pFindCtrl->count < 0
    FILEDIRSBACKWARD_INVALID_FDT_ERROR                         = 107,        //INVALID_FDT == beginFdt && 0 == (pFindCtrl->dspCtrl & FST_ENTER_FLAG)
    FILEDIRSBACKWARD_FILTER_NOTITERATE_ERROR                   = 108,        //pFindCtrl->type & FILTER_NOTITERATE) != 0)
    FILEDIRSBACKWARD_FILTER_NOTITERATE_TWO_ERROR               = 109,        //pFindCtrl->type & FILTER_NOTITERATE) != 0)
    FILEDIRSBACKWARD_PFINDCTRL_DSPINDX_ERROR                   = 110,        //pFindCtrl->dspIndx >= sm.FdtCount

    //File_DeleteHandle
    FILEDELETEHANDLE_FILE_AK_NULL_ERROR                        = 111,          //AK_NULL == file || file->CloseFlag != CLOSE_FLAG_OK
    FILEDELETEHANDLE_DRIVER_DESTROYFLAG_ERROR                  = 112,           //driver->DestroyFlag
    FILEDELETEHANDLE_FILE_IS_NOT_EXIST_ERROR                   = 113,          //The file does not exist
    FILEDELETEHANDLE_CLOSE_FLAG_OK_ERROR                       = 114,          //FSLib - File_DeleteHandle()::the file had been closed
    FILEDELETEHANDLE_ATTR_SHARENUM_ERROR                       = 115,          //attr->ShareNum > 1
   
    //File_DelDir
    FILEDELDIR_SHARENUM_MORE_ONE_ERROR                         = 116,           //parent_attr->ShareNum > 1
    FILEDELDIR_FILE_ATTR_SHARENUM_ERROR                        = 117,           //file_attr->ShareNum > 1

   // File_CreateOpen
    FILECREATEOPEN_PATHNNAME_AK_NULL_ERROR                     = 118,            //AK_NULL == pathname || OpenMode > FILE_MODE_APPEND
    FILECREATEOPEN_STR_AK_NULL_ERROR                           = 119,            //AK_NULL == str || 0 == *str
    FILECREATEOPEN_DRIVER_AK_NULL_ERROR                        = 120,            //AK_NULL == driver

    //File_CreateFile
    FILECREATEFILE_FILE_AK_NULL_ERROR                          = 121,             //AK_NULL == file || file->CloseFlag != CLOSE_FLAG_OK
    FILECREATEFILE_DRIVER_DESTROYFLAG_ERROR                    = 122,             //driver->DestroyFlag  AK_TURN

    //File_CopyUnicode
    FILECOPYUNICODE_SRCPATH_DSTPATH_AK_NULL_ERROR              = 123,             //srcPath == AK_NULL || dstPath == AK_NULL

    //File_CopyFolder
    FILECOPYFOLDER_SOURCE_DEST_ERROR                           = 124,             //source == dest
    FILECOPYFOLDER_SOURCE_AK_NULL_ERROR                        = 125,             //AK_NULL == source || AK_NULL == dest || source->CloseFlag != CLOSE_FLAG_OK || dest->CloseFlag != CLOSE_FLAG_OK
    
    //File_CopyFile
    FILECOPYFILE_FILE_FOLDER_ERROR                             = 126,              //file == Folder
    FILECOPYFILE_FILE_AK_NULL_ERROR                            = 127,             //file == AK_NULL || file->CloseFlag != CLOSE_FLAG_OK|| !(((T_PATTR)file->attr)->exist)
    FILECOPYFILE_REPLACE_ERROR                                 = 128,              //!replace && NewFile_attr->exist
  
    //File_CopyData
    FILECOPYDATA_SOURCE_DEST_ERROR                             = 129,              //source == dest
    FILECOPYDATA_SOURCE_AK_NULL_ERROR                          = 130,              //source == AK_NULL || dest == AK_NULL || source->CloseFlag != CLOSE_FLAG_OK || dest->CloseFlag != CLOSE_FLAG_OK
    FILECOPYDATA_SOURCE_DEST_NOT_EXIST_ERROR                   = 131,              //!(((T_PATTR)dest->attr)->exist) || !(((T_PATTR)source->attr)->exist)

    //File_CopyAsc
    FILECOPYASC_SRCPATH_DSTPATH_AK_NULL_ERROR                  = 132,                //srcPath == AK_NULL || dstPath == AK_NULL

    //File_CheckFilePtr
    FILECHECKFILEPTR_FILE_AK_NULL_ERROR                        = 133,                  //AK_NULL == file || file->CloseFlag != CLOSE_FLAG_OK
    FILECHECKFILEPTR_DRIVER_ID_MAX_ERROR                       = 134,                  //FSlib:the driver destroy1
    FILECHECKFILEPTR_LIST_AK_NULL_ERROR                        = 135,                  //list == AK_NULL FSlib:the driver destroy2
    FILECHECKFILEPTR_DRIVER_DESTTROYFLAG_ERROR                 = 136,                 //driver->DestroyFlag FSlib:the driver destroy3

    //File_CheckFileName
    FILECHECKFILENAME_STR_AK_NULL_ERROR                        = 137,                 //AK_NULL == str
    FILECHECKFILENAME_MAX_ABSOL_PATH_LEN_ERROR                 = 138,                 //Str_Length(str) > MAX_ABSOL_PATH_LEN
    FILECHECKFILENAME_STR_DATA_ERROR                           = 139,                 //str->data[0] == '.'
    FILECHECKFILENAME_LEN_MORE_255_ERROR                       = 140,                 //(len > 255) && ((ParentLen + len) > MAX_ABSOL_PATH_LEN)

    //File_CheckDriverFreeSize
    FILECHECKDRIVERFREESIZE_FILE_AK_NULL_ERROR                 = 141,                  //AK_NULL == file
    FILECHECKDRIVERFREESIZE_FREEHIGH_HIGH_ERROR                = 142,                  //FreeHigh < high || ((FreeHigh != high) && (FreeLow < low))
 
    //File_AddDirSize
    FILEADDDIRSIZE_FULLPATH_ERROR                              = 143,                  //AK_NULL == FullPath
    FILEADDDIRSIZE_FILE_NOT_EXIST_ERROR                        = 144,                  //!(attr->exist)


    //fat.c
    //Fat_Initial
    FAT_INITIAL_FAT_MALLOC_ERROR                               = 145,                  //FAT MALLOC FALSE
    FAT_INITIAL_MAXCLUSNUM_4085_ERROR                          = 146,                  //Fat_Initial: err:the file system is fat12

    //Fat_ReadLink
    FAT_READLINK_BUF_MALLOC_ERROR                               = 147,                  //buf mallocl false
    FAT_READLINK_NEXTCLUS_0_ERROR                               = 148,                  //0 == NextClus

   //Fat_WriteLink
    FAT_WRITELINK_BUF_MALLOC_ERROR                              = 149,                  //BUF MALLOC AK_FALSE
    FAT_WRITELINK_AK_FALSE_ERROR                                = 150,                  //fat_writelink ak_false

    //Fat_DriverReadFat
    FAT_DRIVERREADFAT_READING_FAT_ERROR                         = 151,                  //FSLIB: error at reading fat!

    //Fat_DriverWriteFat
    FAT_DRIVERWRITEFAT_WRITING_FAT_ERROR                        = 152,                  //FSLIB: error at WRITIng fat!

    //Fat_SetLink
    FAT_SETLINK_BUF_MALLOC_ERROR                                = 153,                  //BUF MALLOC AK_FALSE
    

    //Fat_ClearCluster
    FAT_CLEARCLUSTER_BUF_MALLOC_ERROR                           = 154,                  //BUF MALLOC AK_FALSE
    FAT_CLEARCLUSTER_I_NOT_EQUAL_SECPERCLUS_ERROR               = 155,                  //i != SecPerClus

    
    //Fat_ReadFdt
    FAT_READFDT_DRIVER_MEDIUM_READ_ERROR                        = 156,                  //driver->medium->read AK_FALSE

    //Fat_WriteFdt
    FAT_WRITEFDT_BUF_AK_NULL_ERROR                              = 157,                  //FSLib - Fat_WriteFdt()::Err: Not find the dest node
    FAT_WRITEFDT_MEDIUM_WRITE_ERROR                             = 158,                  //FSLib - Fat_WriteFdt()::Err: medium->write sector

    //Fat_FdtSeek
    FAT_FDTSEEK_SM_CLUSTER_ERROR                                = 159,                  //whether the sm->clus is correct. */

    //Fat_OpenId
    FAT_OPENID_SM_MALLOC_ERROR                                  = 160,                  //BUF MALLOC AK_FALSE
   

    //Fat_SearchFolderLink
    FAT_SEARCHFOLDERLINK_FILELIST_AK_NULL_ERROR                 = 161,                  //filelist is ak_null
    
    //Fat_SearchName
    FAT_SEARCHNAME_FS_SEARCH_ERROR_ERROR                        = 162,                  //0 == ParentId
    FAT_SEARCHNAME_FATFS_FAT_FS_EX_ERROR                        = 163,                  //FAT_FS_EX == fat->fs

    //Fat_DeleteFile
    FAT_DELETEFILE_ATTR_FILEID_0_ERROR                          = 164,                  //FSLib - Fat_DeleteFile():: Deleting the root/current-working directory
    FAT_DELETEFILE_LEN_NOT_EUQAL_0_ERROR                        = 165,                  //It has some children, can not be deleted here
    FAT_DELETEFILE_SM_MALLOC_ERROR                              = 166,                  //SM_MALLOC AK_FALSE

    //Fat_GetUsedSize
    FAT_GETUSEDSIZE_BUF_MALLOC_ERROR                            = 167,                  //BUF MALLOC AK_FALSE
    FAT_GETUSEDSIZE_FAT16_READSECTOR_ERROR                      = 168,                  //FAT16         ReadSector AK_FALSE
    FAT_GETUSEDSIZE_FAT32_EX_READSECTOR_ERROR                   = 169,                  //FAT32/EX    ReadSector AK_FALSE

    //Fat_CreateFile
    FAT_CREATFILE_SM_MALLOC_ERROR                               = 170,                  //SM MALLOC AK_FALSE
    FAT_CREATFILE_SM_SECTER_0_ERROR                             = 171,                  //0 == sm->sector

    //Fat_RenameTo
    FAT_RENAMETO_SM_MALLOC_ERROR                                = 172,                  //SM MALLOC AK_FALSE

    //Fat_Mkdir
    FAT_MKDIR_SM_MALLOC_ERROR                                   = 173,                  //SM MALLOC AK_FALSE
    
    //Fat_FlushFDT
    FAT_FLUSHFDT_SM_MALLOC_ERROR                                = 174,                  //SM MALLOC AK_FALSE

    //Fat_Seek
    FAT_SEEK_SM_MALLOC_ERROR                                    = 175,                  //SM MALLOC AK_FALSE

    //Fat_ReadData
    FAT_READDATA_AK_FALSE_ERROR                                 = 176,                  //fat_readdata AK_FALSE
    
    //Fat_ReadBlock
    FAT_READBLOCK_SECTORCOUNT_0_ERROR                           = 177,                  //Fat_ReadBlock AK_FALSE

    //Fat_WriteData
    FAT_WRITEDATA_SECTORCOUNT_0_ERROR                           = 178,                  //(StartPos <= 0) || (StartHighPos <= 0)
    FAT_WRITEDATA_WRITELEN_0_ERROR                              = 179,                  //0 == WriteLen

    //Fat_Format
    FAT_FORMAT_FAT_MALLOC_ERROR                                 = 180,                  //fat malloc ak_false
    FAT_FORMAT_MEDIUM_WRITE_ONE_ERROR                           = 181,                  //0 == medium->write(medium, buf, driver->StartSector, 1))
    FAT_FORMAT_MEDIUM_WRITE_TWO_ERROR                           = 182,                  //(FAT_FS_32 == fat->fs)  (0 == medium->write(medium, buf, driver->StartSector + 0x06, 1))
    FAT_FORMAT_MEDIUM_WRITE_THREE_ERROR                         = 183,                  //medium->write(medium, buf, (driver->StartSector + 1), 1) == 0

    //Fat_QuickFormat
    FAT_QUICKFORMAT_MEDIUM_WRITE_ONE_ERROR                      = 184,                  //0 == medium->write(medium, buf, i, WriteSector)
    FAT_QUICKFORMAT_MEDIUM_WRITE_TWO_ERROR                      = 185,                  //0 == medium->write(medium, buf, addr + RootSec, 1)
    FAT_QUICKFORMAT_MEDIUM_WRITE_THREE_ERROR                    = 186,                  //medium->write(medium, buf, addr + j, WriteSector) == 0

    //Fat_ChangeAttr
    FAT_CHANGEATTR_SM_MALLOC_ERROR                              = 187,                  //SM MALLOC AK_FALSE

    //Fat_ChangeFileSize
    FAT_CHANGEFILESIZE_SM_MALLOC_ERROR                          = 188,                  //SM MALLOC AK_FALSE


    //fdt.c
    //Fdt_GetNextBuf
    FDT_GETNEXTBUF_FS_SEARCH_NOEXIST_ERROR                      = 189,                  //sm->Ctrl.Result = FS_SEARCH_NOEXIST
    FDT_GETNEXTBUF_FS_SEARCH_NOEXIST_K_0_ERROR                  = 190,                  //sm->Ctrl.Result = FS_SEARCH_NOEXIST

    //Fdt_GetPreBuf
    FDT_GETPREBUF_FS_SEARCH_NOEXIST_SM_CLUSTER_0_ERROR          = 191,                  //sm->Ctrl.Result = FS_SEARCH_NOEXIST
    FDT_GETPREBUF_FS_SEARCH_NOEXIST_SM_VSN_0_ERROR              = 192,                  //sm->Ctrl.Result = FS_SEARCH_NOEXIST
    FDT_GETPREBUF_FS_SEARCH_NOEXIST_K_0_ERROR                   = 193,                  //sm->Ctrl.Result = FS_SEARCH_NOEXIST

    //Fdt_GetPreBuf
    FDT32_GETPREFILE_SM_PTR_ERROR                               = 194,                  //0 == *(sm->ptr)

    //Fdt32_GetNextFile
    FDT32_GETNEXTFILE_SM_PTR_ERROR                              = 195,                  //0 == *(sm->ptr)
    FDT32_GETNEXTFILE_NFDT_ERROR                                = 196,                  //AK_NULL == nfdt

    //Fdt_GetFileLength
    FDT_GETFILELENGTH_DATA_AK_NULL_ERROR                        = 197,                  //AK_NULL ==DATA


    //Fdt_CheckLongMatch
    FDT_CHECKLONGMATCH_FDT_GETNAME_ERROR                        = 198,                  //Fdt_GetName rerurn ak_null
    FDT_CHECKLONGMATCH_AK_FALSE_ERROR                           = 199,                  //Fdt_CheckLongMatch ak_false

    //Fdt_GetFileAttr
    FDT_GETFILEATTR_PTR_AK_NULL_ERROR                           = 200,                  //ptr = ak_null

    //Fdt_SetNameAttr
    FDT_SETNAMEATTR_MAX_NOT_ZERO_ERROR                          = 201,                  //0 != max
    FDT_SETNAMEATTR_AK_FALSE_ERROR                              = 202,                  //Fdt_SetNameAttr AK_FALSE
    
    //Fdt32_WriteDelFlag
    FDT32_WRITEDELFLAG_SM_PTR_E5_ERROR                          = 203,                  //FSLib - Fdt_Delete()::Warning: come to a E5-fdt, ignore it
    FDT32_WRITEDELFLAG_SM_PTR_0_ERROR                           = 204,                  //FSLib - Fdt_Delete()::Abnormal3: come to a zero-fdt! 
    FDT32_WRITEDELFLAG_SM_PTR_0F_ERROR                          = 205,                  //Set 0xE5 to long FDT of file which we want to delete.
    
    //fat_man.c
    //Fat_CompareUseFatBuf
    FAT_COMPAREUSEFATBUF_PCURFATINF_PNEWFATINFO_ERROR           = 206,                  //pCurFatInfo->start == pNewFatInfo->start
    FAT_COMPAREUSEFATBUF_PCURFATINF_MORE_PNEWFATINFO_ERROR      = 207,                  //pCurFatInfo->start > pNewFatInfo->start
    
   //Fat_ComparefindUseFatBuf
    FAT_COMPAREFINDUSEFATBUF_PCURFATINF_PNEWFATINFO_ERROR        = 208,                  //pCurFatInfo->start == pNewFatInfo->start
    FAT_COMPAREFINDUSEFATBUF_PCURFATINF_MORE_PNEWFATINFO_ERROR   = 209,                  //pCurFatInfo->start > pNewFatInfo->start
    
    //Fat_CompareUnUseFatBuf
    FAT_COMPAREUNUSEFATBUF_PCURFATINF_PNEWFATINFO_ERROR          = 210,                  //pCurFatInfo->start == pNewFatInfo->start
    FAT_COMPAREUNUSEFATBUF_PCURFATINF_MORE_PNEWFATINFO_ERROR     = 211,                  //pCurFatInfo->start > pNewFatInfo->start
        
    //Fat_BinarySearchFatBuf
    FAT_BINARYSEARCHFATBUF_TOLITEMS_0_ERROR                      = 212,                  //TolItems == 0
    FAT_BINARYSEARCHFATBUF_COMP_ERROR                            = 213,                  //comp(pAllFatInfo, pNewFatInfo) > 0
    FAT_BINARYSEARCHFATBUF_COMP_END_ERROR                        = 214,                  //comp(pAllFatInfo + end, pNewFatInfo) < 0
    FAT_BINARYSEARCHFATBUF_COMP_TWO_ERROR                        = 215,                  //comp(pAllFatInfo + mid, pNewFatInfo) < 0) &&(comp(pAllFatInfo + mid + 1, pNewFatInfo) > 0)

    //Fat_InsertItemToFatBuf
    FAT_INSERTLTEMFATBUF_COMP_ERROR                              = 216,                  //comp(pAllFatInfo, pNewFatInfo) > 0

    //Fat_AllocArrayFatFromRam
    FAT_ALLOCARRAYFATFROMRAM_CURUNUSEFATBUFNUM_ERROR             = 217,                  //if there is no unused fat in ram , we will return false
    

    //Fat32_LoadUnusedFatFromRom
    FAT32_LOADUNUSEDFATFROMROM_FREESIZE_0_ERROR                 = 218,                  //driver->FreeSize  == 0
    FAT32_LOADUNUSEDFATFROMROM_BUF_AK_NULL_ERROR                = 219,                  //BUF MALLOC AK_NULL
    FAT32_LOADUNUSEDFATFROMROM_AK_FALSE_ERROR                   = 220,                  //Fat32_LoadUnusedFatFromRom AK_FALSE         

    //Fatex_LoadUnusedFatFromRom
    FATEX_LOADUNUSEDFATFROMROM_FREESIZE_0_ERROR                 = 221,                  //driver->FreeSize  == 0
    FATEX_LOADUNUSEDFATFROMROM_BUF_AK_NULL_ERROR                = 222,                  //BUF MALLOC AK_NULL
    FATEX_LOADUNUSEDFATFROMROM_AK_FALSE_ERROR                   = 223,                  //Fat32_LoadUnusedFatFromRom AK_FALSE         

    //Fat_MoveFreeClusFromFile
    FAT_MOVEFREECLUSFROMFILE_CUR_AK_NULL_ERROR                  = 224,                  //cur == AK_NULL       
    FAT_MOVEFREECLUSFROMFILE_FIND_AK_NULL_ERROR                 = 225,                  //find == AK_NULL   

    //Fat_LoadFatFromRom
    FAT_LOADFATFROMROM_CLUS_ENDFLAG_AK_TRUE_ERROR               = 226,                  //clus->EndFlag == AK_TRUE
    FAT_LOADFATFROMROM_BUF_AK_NULL_ERROR                        = 227,                  //AK_NULL == buf £¨MALLOC£©
    FAT_LOADFATFROMROM_FAT16_NEXTCLUS_ERROR                     = 228,                  //FAT_FS_16 == fat->fs   -> NextClus == fat->FileEndFlag
    FAT_LOADFATFROMROM_FAT32EX_NEXTCLUS_ERROR                   = 229,                  //FAT_FS_16 != fat->fs   -> FAT_FS_16 == fat->fs 

    //Fat_LoadClusterLink
    FAT_LOADCLUSTERLINK_CLUS_ENDFLA_ERROR                       = 230,                  //clus->EndFlag == AK_TRUE

    //Fat_FlushFatLink
    FAT_FLUSHFATLINK_BUF_MALLOC_ERROR                           = 231,                  //buf malloc ak_null

    //fdtex.c
    //Fdtex_SetAttrFromEXFDTName
    FDTEX_SETATTRFROMEXFDTNAME_AK_FALSE_ERROR                   = 232,                  //Fdtex_SetAttrFromEXFDTName AK_FALSE

    //Fdtex_GetPreFile
    FDTEX_GETPREFILE_SM_PTR_ERROR                               = 233,                  //0 == *(sm->ptr)

    //Fdtex_GetNextFile
    FDTEX_GETNEXTFILE_SM_PTR_ERROR                               = 234,                  //0 == *(sm->ptr)
    
    //Fdtex_GetFileAttr
    FDTEX_GETFILEATTR_PTR_AK_NULL_ERROR                         = 235,                  //0 == *(sm->ptr)
    
    //Fdtex_WriteDelFlag
    FDTEX_WRITEDELFLAG_SM_PTR_ERROR                             = 236,                 //FSLib - Fdt_Delete()::Warning: come to a E5-fdt, ignore it
    FDTEX_WRITEDELFLAG_MAIN_FDT_FLAG_ERROR                      = 237 ,                //sm->ptr[0] == MAIN_FDT_FLAG


    //Attr_Initial
    ATTR_INITIAL_ATTR_AK_NULL_ERROR                             = 238 ,                //ATTR MALLOC AK_NULL

    //Cluster_Initial
    CLUSTER_INITIAL_BUFLEN_AK_NULL_ERROR                        = 239 ,                //BUFLEN AK_NULL
    CLUSTER_INITIAL_DATA_AK_NULL_ERROR                          = 240 ,                //DATA MALLOC AK_NULL
    CLUSTER_INITIAL_OBJ_AK_NULL_ERROR                           = 241 ,                //OBJ MALLOC AK_NULL

    //Cluster_CreateBlock
    CLUSTER_CREATEBLOCK_LEN_0_ERROR                             = 242 ,                //LEN = 0


    //Cluster_Add
    CLUSTER_ADD_LEN_OBJ_CLUSTER_0_ERROR                         = 243 ,                //AK_NULL == obj || 0 == cluster || 0 == len
    CLUSTER_ADD_DATA_REALLOC_ERROR                              = 244 ,                //data realloc ak_null

    //Cluster_CheckSerious
    CLUSTER_CHECKSERIOUS_DATA_0_ERROR                           = 245 ,                //data[scount + saddr + 1] == 0

    //Cluster_Find
    CLUSTER_FIND_OBJ_CLUSTER_ERROR                              = 246 ,                //AK_NULL == obj || 0 == cluster
    CLUSTER_FIND_CLUSTER_ERROR                                  = 247 ,                //cluster < obj->MinCluster || cluster > obj->MaxCluster
    CLUSTER_FIND_FIND_FFFFFFFFF_ERROR                           = 248 ,                //Cluster_Find RETURN 0XFFFFFFFF

    //Cluster_Seek
    CLUSTER_SEEK_OBJ_AK_NULL_ERROR                              = 249 ,                //AK_NULL == obj 
    CLUSTER_SEEK_SVN_ERROR                                      = 250 ,                //vcn > obj->MaxVcn
    CLUSTER_SEEK_0_ERROR                                        = 251 ,                //Cluster_Seek RETURN 0


    //Cluster_SortedAdd
   CLUSTER_SORTEDADD_OBJ_CLUSTER_LEN_ERROR                       = 252 ,                //AK_NULL == obj || 0 == cluster || 0 == len
   CLUSTER_SORTEDADD_DATA_REALLOC_ERROR                          = 253 ,                //data REALLOC AK_NULL
   CLUSTER_SORTEDADD_OBJ_DATA_ERROR                              = 254 ,                //obj->data[0] != 0 && cluster <= obj->MaxCluster
   CLUSTER_SORTEDADD_REALADDR1_ERROR                             = 255 ,                // /* Check whether the block to be added crosses the searching-block. */
   CLUSTER_SORTEDADD_CLUSTER_REALADDR1_ERROR                     = 256 ,                //Check whether block to be added belongs to the link already, or cluster crosses the searching-block. */
   CLUSTER_SORTEDADD_REALADDR2_ERROR                             = 257 ,                //Check whether block to be added crosses the next-block. 
 
    //Cluster_SortedFind
    CLUSTER_SORTEDFIND_OBJ_CLUSTER_ERROR                         = 258 ,                //AK_NULL == obj || 0 == cluster
    CLUSTER_SORTEDFIND_CLUSTER_ERROR                             = 259 ,                //cluster < obj->MinCluster || cluster > obj->MaxCluster
    CLUSTER_SORTEDFIND_AK_FALSE_ERROR                            = 260 ,                //Cluster_SortedFind AK_FALSE

    //Cluster_SortedCutBlock
    CLUSTER_SORTEDCUTBLOCK_OBJ_CLUSTER_LEN_ERROR                 = 261 ,                // Validate the parameters.  AK_NULL == obj || 0 == cluster || 0 == len
    CLUSTER_SORTEDCUTBLOCK_DATA_CLUSTER_ERROR                    = 262 ,                //Check whether link is empty or [cluster, cluster+len) is out of [MinCluster, MaxCluster]. */
    CLUSTER_SORTEDCUTBLOCK_DATA_REALLOC_ERROR                    = 263 ,                //DATA REALLOC AK_NULL
    CLUSTER_SORTEDCUTBLOCK_DATA_ERROR                            = 264 ,                // DATA == AK_NULL

    //Cluster_SortedDel
    CLUSTER_SORTEDDEL_OBJ_SUB_ERROR                              = 265 ,                // Validate the parameters.  AK_NULL == obj || AK_NULL == sub

    //Cluster_GetBlock
    CLUSTER_GETBLOCK_DATA_ERROR                                  = 266 ,                // *data == 0

    //Cluster_GetCount
    CLUSTER_GETCOUNT_OBJ_ERROR                                  = 267 ,                // obj == AK_NULL
    CLUSTER_GETCOUNT_VCN_ERROR                                  = 268 ,                // vcn > obj->MaxVcn
    CLUSTER_GETCOUNT_ONT_ID_ERROR                               = 269 ,                // Cluster_GetCount RETURN 0


    //Cluster_Split
    CLUSTER_SPLIT_OBJ_SUB_ERROR                                 = 270 ,                // AK_NULL == obj || AK_NULL == sub
    CLUSTER_SPLIT_VCN_ERROR                                     = 271 ,                // vcn > obj->MaxVcn
    CLUSTER_SPLIT_REMAIN_ERROR                                  = 272 ,                // 0 == remain && 0 == data[ptr]
    
    //BLink_NextNoCopy
    BLINK_NEXTNOCOPY_OBJ_ERROR                                  = 273 ,                // AK_NULL == obj || AK_NULL == obj->ptr    
    
    //BLink_PreNoCopy
    BLINK_PRENOCOPY_OBJ_ERROR                                   = 274 ,                // AK_NULL == obj || AK_NULL == obj->ptr    
    
    
    //BLink_GetCurrent
    BLINK_GETCURRENT_OBJ_ERROR                                   = 275 ,                // AK_NULL == obj || AK_NULL == obj->ptr    

    //BLink_GetHead
    BLINK_GETHEAD_OBJ_ERROR                                      = 276 ,                // AK_NULL == obj || AK_NULL == obj->head   

    //BLink_GetTail
    BLINK_GETTAIL_OBJ_ERROR                                      = 277 ,                // AK_NULL == obj || AK_NULL == obj->tail  

    //BLink_GetNodeData
    BLINK_GETNODEDATA_ITEM_ERROR                                 = 278 ,                // AK_NULL == item  

    //BLink_Prev
    BLINK_PREV_OBJ_ERROR                                         = 279 ,                // obj == AK_NULL || obj->ptr == AK_NULL

    //BLink_Delete
    BLINK_DELETE_LINK_ISEMPTY_ERROR                              = 280 ,                //BLink_IsEmpty RETRUN AK_TRUE
    BLINK_DELETE_OBJ_ERROR                                       = 281 ,                // AK_NULL == pItem

    //BLink_Insert
    BLINK_INSERT_OBJ_ERROR                                       = 282 ,                //obj == AK_NULL
    BLINK_INSERT_PITEM_ERROR                                     = 283 ,                // pitem malloc ak_null
    BLINK_INSERT_PITEM_DATA_ERROR                                = 284 ,                // pitem->data malloc ak_null

    //BLink_InsertBlink
    BLINK_INSERTBLINK_BLINK_ERROR                                = 285 ,                //BLINK == AK_NULL
    BLINK_INSERTBLINK_BLINK_ITEMSIZE_ERROR                       = 286 ,                //blink->ItemSize != subLink->ItemSize

    //BLink_InsertBlinkItem
    BLINK_INSERTBLINKITEM_BLINKITEM_OBJ_ERROR                    = 287 ,                //AK_NULL == obj || AK_NULL == blinkitem

    //Fat_FormatEX    
    FAT_FORMATEX_DRVIVER_ERROR                                      = 288 ,                //driver == AK_NULL
    FAT_FORMATEX_DRVIVER_FILELIST_ERROR                             = 289 ,                //driver->filelist malloc ak_null
    FAT_FORMATEX_FAT_MALLOC_ERROR                                   = 290 ,                //fat malloc ak_null
    FAT_FORMATEX_BUF_MALLOC_ERROR                                   = 291 ,                //buf malloc ak_null
    FAT_FORMATEX_MEDIUM_WRITE_STARTSECTOR_ERROR                     = 292 ,                //0 == medium->write(medium, buf, driver->StartSector, 1)
    FAT_FORMATEX_MEDIUM_WRITE_STARTSECTOR_I_ERROR                   = 293 ,                //0 == medium->write(medium, buf, driver->StartSector + i, 1)
    FAT_FORMATEX_MEDIUM_WRITE_STARTSECTOR_11_ERROR                  = 294 ,                //0 == medium->write(medium, buf, driver->StartSector + 11, 1)
    FAT_FORMATEX_MEDIUM_READ_STARTSECTOR_I_SEC_ERROR                = 295 ,                //0 == medium->read(medium, buf, driver->StartSector + i, SectorsPerPage)
    FAT_FORMATEX_MEDIUM_WRITE_STARTSECTOR_I_12_ERROR                = 296 ,                //0 == medium->write(medium, buf, driver->StartSector + i + 12, SectorsPerPage
    FAT_FORMATEX_MEDIUM_WRITE_STARTSECTOR_OFFSET_ERROR              = 297 ,                //0 == medium->write(medium, buf, driver->StartSector + offset, SectorsPerPage)
    FAT_FORMATEX_MEDIUM_WRITE_STARTSECTOR_OFFSET_I_ERROR             = 298 ,                //0 == medium->write(medium, buf, driver->StartSector + offset + i, SectorsPerPage))
    FAT_FORMATEX_MEDIUM_WRITE_STARTSECTOR_ROOADDR_ERROR                = 299 ,                //0 == medium->write(medium, buf, driver->StartSector + RootAddr , SectorsPerPage)
    FAT_FORMATEX_MEDIUM_WRITE_STARTSECTOR_ROOADDR_I_ERROR              = 300 ,                //0 == medium->write(medium, buf, driver->StartSector + RootAddr + i, SectorsPerPage)
    FAT_FORMATEX_MEDIUM_WRITE_STARTSECTOR_ROOADDR_CLUSMAPCLUSS_ERROR   = 301 ,                //0 == medium->write(medium, buf, driver->StartSector + RootAddr + ClusMapCluss * SectorPerClus + i, 1)
    FAT_FORMATEX_MEDIUM_WRITE_STARTSECTOR_ROOADDR_FIRSTROOT_ERROR      = 302 ,                //0 == medium->write(medium, buf, driver->StartSector + RootAddr + (FirstRoot - 2) * SectorPerClus, SectorsPerPage)
    FAT_FORMATEX_MEDIUM_WRITE_STARTSECTOR_ROOADDR_I_FIRSTROOT_ERROR    = 303 ,                 //0 == medium->write(medium, buf, driver->StartSector + RootAddr + i + (FirstRoot - 2) * SectorPerClus, SectorsPerPage)


    //Fat_QuickFormatEX    
     FAT_QUICKFORMATEX_DRIVER_ERROR                                  = 304 ,                //driver == AK_NULL
     FAT_QUICKFORMATEX_BUF_ERROR                                     = 305 ,                //BUF malloc ak_null
     FAT_QUICKFORMATEX_MEDIUM_WRITE_STARTSECTOR_1_ERROR              = 306 ,                //0 == medium->write(medium, buf, driver->StartSector + RootAddr + (fat->RootStartClus - 2) * 0x40, SectorsPerPage)
     FAT_QUICKFORMATEX_MEDIUM_WRITE_STARTSECTOR_2_ERROR              = 307 ,                //0 == medium->write(medium, buf, driver->StartSector + RootAddr + i + (fat->RootStartClus - 2) * 0x40, SectorsPerPage)
     FAT_QUICKFORMATEX_MEDIUM_WRITE_STARTSECTOR_3_ERROR              = 308 ,                //0 == medium->write(medium, buf, driver->StartSector + RootAddr, SectorsPerPage)
     FAT_QUICKFORMATEX_MEDIUM_WRITE_STARTSECTOR_4_ERROR              = 309 ,                //0 == medium->write(medium, buf, driver->StartSector + RootAddr + i, SectorsPerPage)
     FAT_QUICKFORMATEX_MEDIUM_READ_STARTSECTOR_ERROR                 = 310 ,                //0 == medium->read(medium, buf, driver->StartSector + fat->Fat1Addr, SectorsPerPage)
     FAT_QUICKFORMATEX_MEDIUM_WRITE_STARTSECTOR_5_ERROR              = 311 ,                //0 == medium->write(medium, buf, driver->StartSector + fat->Fat1Addr, SectorsPerPage)
     FAT_QUICKFORMATEX_MEDIUM_WRITE_STARTSECTOR_6_ERROR              = 312 ,                //0 == medium->write(medium, buf, driver->StartSector + fat->Fat1Addr + i, SectorsPerPage)
     FAT_QUICKFORMATEX_MEDIUM_WRITE_STARTSECTOR_7_ERROR              = 313 ,                //0 == medium->write(medium, buf, driver->StartSector + RootAddr + ClusMapCluss * SectorPerClus + i, 1)

     //Fat_FlushFatMapEX    
     FAT_FLUSHFATMAPEX_BUF_MALLOC_ERROR                              = 314 ,                //BUF malloc ak_null

    
    //Fat_InitialEX    
    FAT_INITIALEX_FAT_MALLOC_ERROR                                   = 315 ,                //FAT malloc ak_null
    
    //Fat_DriverReadFatMap    
    FAT_DRIVERREADFATMAP_MEDIUM_READ_ERROR                           = 316 ,                //FSLIB: error at reading fat

    
    //Fat_DriverWriteFatMap    
    FAT_DRIVERWRITEFATMAP_MEDIUM_WRITE_ERROR                         = 317 ,                //FSLIB: error at writing fat
    
    //Driver_Initial1    
    DRIVER_INITIAL1_SECTORBUF_ERROR                                 = 318 ,                //SectorBuf MALLLOC AK_NULL
    DRIVER_INITIAL1_MEDIUM_READ_1_ERROR                             = 319 ,                //AK_NULL == medium || 0 == medium->read(medium, SectorBuf, 0, 1)
    DRIVER_INITIAL1_SECTORBUF_ENDFLAG_ERROR                         = 320 ,                //Driver_Initial:the end flag of the 0 sector is error
    DRIVER_INITIAL1_MEDIUM_READ_2_ERROR                             = 321 ,                //Driver_Initial:it fails to read start sector
    DRIVER_INITIAL1_MEMCMP_FATTYPE_ERROR                            = 322 ,                //Driver_Initial:we don't know the file system flag
    DRIVER_INITIAL1_DRIVER_MALLOC_ERROR                             = 323 ,                //DRIVER MALLLOC AK_NULL
    DRIVER_INITIAL1_DRIVER_FILELISTC_ERROR                          = 324 ,                //driver->filelist MALLLOC AK_NULL
    DRIVER_INITIAL1_DRIVER_FDTBUFDIR_ERROR                          = 325 ,                //driver->fdtbufDir MALLLOC AK_NULL
    DRIVER_INITIAL1_DRIVERBUF_ERROR                                 = 326 ,                //AK_NULL == DriverBuf || AK_NULL == pBlinkItem
    

     //Driver_Format1    
     DRIVER_FORMAT1_MEDIUM_ERROR                                    = 327 ,                //AK_NULL == medium || start >= medium->capacity
     DRIVER_FORMAT1_DRIVER_ERROR                                    = 328 ,                //DRIVER MALLLOC AK_NULL
     DRIVER_FORMAT1_DRIVER_FDTBUFDIR_ERROR                          = 329 ,                //driver->fdtbufDir MALLLOC AK_NULL
     DRIVER_FORMAT1_PBLINKITEM_ERROR                                = 330 ,                //AK_NULL == DriverBuf || AK_NULL == pBlinkItem
    
    //BLink_IsEmpty  
     BLINK_ISEMPTY_AK_FALSE_ERROR                                   = 331 ,                // BLink_IsEmpty AK_FALSE     

    //Fid_Initial  
    FID_INITIAL_FID_MALLOC_ERROR                                    = 332 ,                // fid malloc AK_NULL   

    
    //Filter_Match_One  
    FILETER_MATCH_ONE_S2_AK_NULL_ERROR                              = 333 ,                // fid malloc AK_NULL     
    FILETER_MATCH_ONE_S2_0_ERROR                                    = 334 ,                // fid malloc AK_NULL   

    //Filter_Match
    FILETER_MATCH_S2_AK_NULL_ERROR                                  = 335 ,                // fid malloc AK_NULL   
    FILETER_MATCH_AK_FALSE_ERROR                                    = 336 ,                // fid malloc AK_NULL   

    //Fname_RepeatCheck
    FNAME_REPEATCHECK_FMEMCMP_ERROR                                 = 337 ,                //When name is different from dst, check ExtName. */ 
    FNAME_REPEATCHECK_UNI_ASCSEARCH_ERROR                           = 338 ,                //  Find the place where is "~", then compare name with dst before "~",if different, return false.
    FNAME_REPEATCHECK_CH1_NOT_EQUAL_CH2_ERROR                       = 339 ,                //ch1 != ch2
    FNAME_REPEATCHECK_DST_ERROR                                     = 340 ,                // Coming herer it is most possible that dst and name are like this "short~1" and "short~2",
    FNAME_REPEATCHECK_K_NUM_ERROR                                   = 341 ,                //k != *num

    //Fname_CompLongName
    FNAME_COMPLONGNAME_NAMELEN_I_ERROR                              = 342 ,                //Added to avoid accessing invalid memory
    FNAME_COMPLONGNAME_START_ERROR                                  = 343 ,                //0 == start
    FNAME_COMPLONGNAME_NAMELEN_I_TWO_ERROR                          = 344 ,                //NameLen < i


    //Fname_FilterWin32
    FNAME_FILTERWIN32_CH_ERROR                                      = 345 ,                //ch < 0x20
    FNAME_FILTERWIN32_UNI_UNISEARCH_ERROR                           = 346 ,                //Uni_UniSearch(special_char, ch, 8) < 8

    //Fname_GetSpace
    FNAME_GETSPACE_FILENAME_ERROR                                   = 347 ,                //AK_NULL == FileName
    FNAME_GETSPACE_J_ERROR                                          = 348 ,                //0 == j

    //Fname_LongToShort
    FNAME_LONGTOSHOT_FNAME_ERROR_ERROR                              = 349 ,                //LSflag == FNAME_ERROR

    //Link_Insert
    LINK_INSERT_PITEM_ERROR                                         = 350 ,                //pitem malloc ak_false

    
    //List_Initial
    LIST_INITIAL_OBJ_DATA_ERROR                                     = 351 ,                //obj->data malloc ak_false

    //List_Get  
    LIST_GET_OBJ_POS_ERROR                                          = 352 ,                //AK_NULL == obj || pos >= obj->length

    //List_Delete
   LIST_DELETE_OBJ_POS_ERROR                                        = 353 ,                //pos >= obj->length

   //List_Add 
    LIST_ADD_PITEM_ERROR                                             = 354 ,                //pitem realloc ak_null

    //List_DelRef
    LIST_DELREF_POS_ERROR                                            = 355 ,                //pos >= obj->length

    //Long_Search
    LONG_SEARCH_0XFFFFFFFF_ERROR                                     = 356 ,                //Long_Search return 0xffffffff

    //Object_IsKindsOf    
    OBJECT_ISKINGSOF_OBJECT_ERROR                                    = 357 ,                //object == AK_NULL
    OBJECT_ISKINGSOF_AK_FALSE_ERROR                                  = 358 ,                //Object_IsKindsOf return AK_FALSE
    
    //Str_Initial 
    STR_INITIAL_DATA_ERROR                                           = 359 ,                //object == AK_NULL

    //Str_AscCode
    STR_ASCCODE_PASC_ERROR                                           = 360 ,                //AK_NULL == pAsc
    STR_ASCCODE_DATA_ERROR                                           = 361 ,                //data malloc ak_null
    STR_ASCCODE_FASCTOUNI_ERROR                                      = 362 ,                //fgb.fAscToUni((T_pSTR)pAsc, AscLen, data, UnicLen+1, code) < 0

    //Str_Add  
    STR_ADD_DATA_ERROR                                               = 363 ,                //data REALLOC AK_FALSE

    //Str_Copy
    STR_COPY_DATA_ERROR                                              = 364 ,                //data MALLOC AK_FALSE

    //Str_Combine
    STR_COMBINE_FIRST_SECOND_ERROR                                   = 365 ,                //first == AK_NULL || second == AK_NULL

    //Str_Length
    STR_LENGTH_OBJ_ERROR                                             = 366 ,                //obj == AK_NULL

    //Str_GetData
    STR_GETDATA_OBJ_ERROR                                            = 367 ,                //obj == AK_NULL
    
    //Str_GetBytes
    STR_GETBYTES_OBJ_ERROR                                           = 368,                //obj == AK_NULL || dst == AK_NULL

    // Str_Cmp
    STR_CMP_LENSRC_LENDST_ERROR                                      = 369,                //LenSrc == LenDst && LenSrc ==0
    STR_CMP_LENSRC_LESS_LENDST_ERROR                                 = 370,                //LenSrc < LenDst

    //File_CopyUnicode
    FILECOPYUNICODE_EXITFLAG_EQUAL_1_ERROR                           = 371,             //ExitFlag == 1(File_CopyFile/File_CopyData/File_CopyFolder(&ExitFlag))

    //Fat_WriteData
    FAT_WRITEDATA_WRITELEN_SECTORCOUNT_DIFFERENT_ERROR               = 372,   //Only parts of data have been writen
 
    //BLink_Next
    BLINK_NEXT_OBJ_ERROR                                             = 373 ,                // obj == AK_NULL || obj->ptr == AK_NULL    

    //file_findfirstEX
    FILE_FINDFIRSTEX_PATH                                            = 374 ,                // PATH== AK_NULL || obj->ptr == AK_NULL    

    FILE_FINDFIRSTEX_PATHLEN                                         = 375 ,                // PATHLEN == 0    

    FILE_FINDFIRSTEX_ISNOFOLDER                                      = 376 ,                // THE DIR IS NOT FOLDER
    
    FILE_FINDFIRSTEX_MALLOC                                          = 377 ,                //MALLOC
    
    FILE_READWITHOUTSEM_WRITEDATAERROR                        		 = 378 ,                // obj == AK_NULL || obj->ptr == AK_NULL    

    
}exfat_error_type;


/************************************************************************
 * NAME:        File_SetLastErrorType
 * FUNCTION  set the last error type
 * PARAM:      enum error_type
 * RETURN:     T_VOID
**************************************************************************/

void File_SetLastErrorType(exfat_error_type error_type);


#endif




