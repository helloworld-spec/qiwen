#ifndef _RECORD_FS_H_
#define _RECORD_FS_H_

#include <time.h>
#include "list.h"
#include "ak_common.h"
#include "ak_dvr_common.h"

#define ONE_DAY_SECOND				(24*60*60)
#define ONE_HOUR_SECOND				3600
#define FILE_EXT_NAME_MAX_LEN   4	//eg.: .mp4/.avi

/* search record file param */
struct search_file_param {
    unsigned char run_flag;        		//continue search record file flag
    char prefix[DVR_FILE_PREFIX_MAX_LEN];//record file name prefix
    char path[DVR_FILE_PATH_MAX_LEN];//record file saving path
};

/* search replay record file param of dir by time */
struct search_replay_param {
    time_t start_time;					//replay start time, unit: second
	time_t end_time;					//replay end time, unit: second
	enum dvr_file_type type;			//replay record file type
	struct list_head *entry_list;		//replay entry list
};

typedef void (* search_record_callback)(const char *file_name,
				int file_size, int file_type);
typedef int (* search_replay_callback)(const char *file_name, int file_size,
				time_t calendar_time, struct search_replay_param *replay);

/**
 * record_fs_create_dir - create dir
 * @rec_path[IN]: record full path
 * return: 0 success, -1 failed
 */
int record_fs_create_dir(const char *rec_path);

/**
 * record_fs_get_time_by_name: get record file calendar time from file name
 * @file_path[IN]: file path including full file name
 * return: 0 success; otherwise -1
 * notes: file name ex.: /mnt/CYC_DV/CYC_DV_20160504-095310@300000.mp4
 */
time_t record_fs_get_time_by_name(const char *file_path);

/**
 * record_fs_get_disk_free_size - get disk free size, appoint the dir path
 * @path[IN]: disk dir full path
 * return: disk free size in KB
 */
unsigned long record_fs_get_disk_free_size(const char *path);

/**
 * record_fs_get_disk_total_size - get disk total size, appoint the dir path
 * @path[IN]: disk dir full path
 * return: disk total size in KB, 0 failed
 */
unsigned long record_fs_get_disk_total_size(const char *path);

/**
 * record_fs_search_by_time - search replay record file by time
 * @search[IN]: search record file param
 * @replay[IN/OUT]: replay param
 * @callback[IN]: replay entry callback
 * return: record files total number of first day appointed to given time
 * notes: find all of the record files in one day dir, add to replay list.
 */
int record_fs_search_by_time(struct search_file_param *search,
							struct search_replay_param *replay,
							search_replay_callback callback);

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
 * @duration[IN]: appointed file duration time
 * @file_name[OUT]: generated file name
 * return: 0 success, -1 failed
 */
unsigned char record_fs_get_video_name(const char *path,
									const char *prefix,
									const char *ext_name,
									const struct ak_date *date,
									unsigned long duration,
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
#define SEARCH_RECORD_MS_SLOW 5000
#define SEARCH_RECORD_MS_FAST 10
#define LEN_RECORD_EXT_NAME 4
#define LEN_PATH_FILE       256
#define LEN_PATH_RECORD     32
void set_record_sleep_ms( int sleep_ms );
unsigned char check_record_file_name(char *file_name, const char *prefix);
int check_record_ext_name( char *file_name, int i_len );
long get_record_file_size(const char *file);
#endif
