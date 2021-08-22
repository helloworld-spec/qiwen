/*
 * Copyright(c)  Shen zhen Danale Telecommunication Corp.
 *
 * Authored by Liao yangyang on: 2015年 09月 11日 星期五 17:05:50 CST
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simplelink.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "dana_ioops.h"

struct dana_file_handler_s {
    int fp;
    long w_offset;
    long r_offset;
    file_info_t info;
};

extern dana_file_handler_t* dana_file_open(const char *file_name, file_mode_t mode) 
{
    dana_file_handler_t *file = (dana_file_handler_t *)mem_Malloc(sizeof(dana_file_handler_t));
    if (NULL == file) {
        return NULL;
    }
    
    mem_set(file, 0, sizeof(dana_file_handler_t));
    

    if (fmode_r == mode) {
         sl_FsOpen(file_name,FS_MODE_OPEN_READ,0,&(file->fp));
    } else if (fmode_w == mode) { 
   	 sl_FsOpen(file_name, FS_MODE_OPEN_WRITE, NULL, &(file->fp));	 
    } else if (fmode_a == mode) {
         sl_FsOpen(file_name, FS_MODE_OPEN_WRITE, NULL, &(file->fp));	
    }
    
    SlFsFileInfo_t  pFsFileInfo;
    if (sl_FsGetInfo(file_name, 0, &pFsFileInfo) < 0) {
      mem_Free(file);
      return NULL;
    }
    
    if (fmode_a == mode) {
      file->r_offset = pFsFileInfo.FileLen;
      file->w_offset = pFsFileInfo.FileLen;
    }
    Report("file_size: %d\r\n", pFsFileInfo.FileLen);
    file->info.file_name_len = strlen(file_name);
    strncpy(file->info.file_name, file_name, file->info.file_name_len);
    file->info.file_size = pFsFileInfo.FileLen;
    
    return file;	
}

extern int32_t dana_file_read(dana_file_handler_t *file, uint8_t *buf, uint32_t n) 
{
      int val = sl_FsRead(file->fp, file->r_offset, buf, n); 
      if (val < 0) {
          Report("sl_FsRead: %d\r\n", val);
          return -1;
      }
      file->r_offset += val;
      return val;
}

extern int32_t dana_file_write(dana_file_handler_t *file, uint8_t *buf, uint32_t n) 
{
      int val = sl_FsWrite(file->fp, file->w_offset, buf, n);
      if (val < 0) {
          return -1;
      }
      file->w_offset += val;
      return val;
}

extern int32_t dana_file_get_info(dana_file_handler_t *file, file_info_t *info) 
{
    memcpy(info, &file->info, sizeof(file_info_t));
    return 0;
}

extern int32_t dana_file_seek(dana_file_handler_t *file, uint32_t pos) 
{
    file->r_offset = pos;
    file->w_offset = pos;
    
    return 0;
}

extern int32_t dana_file_close(dana_file_handler_t *file) 
{
    sl_FsClose(file->fp,0,0,0);
    mem_Free(file);
    return 0;
}


