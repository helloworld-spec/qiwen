#ifndef _RECORD_FS_H_
#define _RECORD_FS_H_

#include <time.h>
#include "ak_common.h"
#include "ak_record_common.h"

#define FILE_EXT_NAME_MAX_LEN   4	//eg.: .mp4/.avi

typedef void (* search_record_callback)(const char *file_name, 
									int file_size,int file_type);

/* search record file param */
struct search_file_param {
    unsigned char run_flag;        		//continue search record file flag
    char prefix[RECORD_FILE_PREFIX_MAX_LEN];//record file name prefix
    char path[RECORD_FILE_PATH_MAX_LEN];//record file saving path
};

/**
 * record_fs_create_dir - create dir
 * @rec_path[IN]: record full path
 * return: 0 success, -1 failed
 */
int record_fs_create_dir(const char *rec_path);

/**
 * record_fs_get_disk_free_size - get disk free size, appoint the dir path
 * @path[IN]: disk dir full path
 * return: disk free size in KB, -1 failed
 */
unsigned long record_fs_get_disk_free_size(const char *path);

/**
 * record_fs_init_record_list: init record file list
 * @search[IN]: search record file param
 * @pcallback: sort callback
 * return: record files total size of appointed path
 * notes: find all of the record files, add to management list.
 *		stat record files total size for cycle recording.
 */
unsigned long record_fs_init_record_list(struct search_file_param *search,
										search_record_callback callback);

/**
 * record_fs_get_video_name - generate record file name according to param
 * @path[IN]: record file saving path
 * @path[IN]: record file name prefix
 * @ext_name[IN]: record file name ext
 * @date[IN]: appointed file date time
 * @file_name[OUT]: generated file name
 * return: 0 success, -1 failed
 */
unsigned char record_fs_get_video_name(const char *path, 
									const char *prefix,
									const char *ext_name,
									const struct ak_date *date, 
									char *file_name);

/**
 * record_fs_get_audio_name: generate audio file name according to param
 * @path[IN]: audio record file saving path
 * @ext_name[IN]: audio record file name ext
 * @file_name[OUT]: generated file name
 * return: 0 success, -1 failed
 */
unsigned char record_fs_get_audio_name(const char *path, 
									const char *ext_name,
									char *file_name);

/**
 * record_fs_get_photo_name: generate photo file name according to param
 * @path[IN]: photo file saving path
 * @file_name[OUT]: generated photo file name
 * return: 0 success, -1 failed
 */
unsigned char record_fs_get_photo_name(const char *path, char *file_name);

#endif
