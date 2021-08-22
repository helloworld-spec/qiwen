#ifndef _RECORD_FILE_H_
#define _RECORD_FILE_H_

#include "ak_common.h"
#include "ak_dvr_common.h"
#include "ak_dvr_file.h"
#include "ak_dvr_replay.h"

/*
this marco decide a point is NULL or not.
if the point is not null.
then free this point
and set this point NULL
*/
#ifndef FREE_POINT
#define FREE_POINT(POINT)  \
if(POINT != NULL) {\
	free(POINT);\
	POINT = NULL;\
}
#endif

/* dvr file exception callback */
typedef void (* FP_DVR_FILE_EXCEPT)(int except_code, const char *except_desc);
/* dvr file flush finished callback */
typedef void (* FP_DVR_FILE_FLUSH)(unsigned char flush_finish);

/* record file pointer info */
struct record_file_fp {
	FILE *index;      //index fp
	FILE *record;     //record fp
	FILE *moov;       //moov fp
	FILE *stbl;       //stbl fp  
};

/**
 * record_file_open - open record file.
 * @index[IN]: appointed record file index, switch between 0 and 1
 * @date[IN]: record file date time
 * @fp[OUT]: fp info after opened successfully, if you want to fetch
 * return: 0 success, otherwise -1
 * notes: 1. fopen default is fully buffered, and the buffered size is
 *		st_blksize in struct stat
 *		2. we use this size: /mnt/tmp/ is vfat fs
 *			st_blksize=32768, st_blocks=64
 */
int record_file_open(int index, const struct ak_date *date,
                struct record_file_fp *fp);

/**
 * record_file_update_duration - update temp record file duration info.
 * @duration[IN]: record file date time
 * return: 0 success, otherwise -1
 * notes: you can update record duration at certain time,
 *      in case of poweroff when recording.
 */
int record_file_update_duration(unsigned long duration);

/**
 * record_file_generate_name - generate record file name.
 * @save_index[IN]: appointed record file index, switch between 0 and 1
 * @rec_duration[IN]: record file duration
 * return: 0 success, otherwise -1
 * notes: 1. record saving path and prefix are appointed at init time.
 *		2. ext will be decided by record type
 */
int record_file_generate_name(int save_index, unsigned long rec_duration);

/**
 * record_file_delete - delete record file according to index
 * @index: appointed record file index
 * return: none
 * notes: temp file will be removed.
 */
void record_file_delete(int index);

/**
 * record_file_wakeup - wakeup record file thread
 * @save_index[IN]: index of currently saving file
 * return: none
 * notes: flush, sync, rename and save record file after wakeup.
 */
void record_file_wakeup(int save_index);

/**
 * record_file_close - close record file.
 * @file_index:  index of close file
 * return: none
 * notes: 1. record file index, switch between 0 and 1
 *           2. when unplug card, should call it
 */
void record_file_close(int file_index);

/**
 * record_file_set_callback - set callback functin for record
 * @except_report: callback of exception report
 * @flush_cb: callback of flush finished
 * return: 0
 */
int record_file_set_callback( FP_DVR_FILE_EXCEPT except_report,
    FP_DVR_FILE_FLUSH flush_cb);

/**
 * record_file_get_list - get the first record file entry list according to
 *		start time and end time.
 * @start_time[IN]: appointed start time, unit: second
 * @end_time[IN]: appointed end time, unit: second
 * @type[IN]: record file type
 * @entry_list[OUT]: record file entry list
  return: >=0, list count; otherwise -1
 * notes: 1. we'll return record file entry list after get successfully
 *		2. you must free entry path memory outside.
 *		3. you must call INIT_LIST_HEAD to init entry_list
 */
int record_file_get_list(time_t start_time,
						time_t end_time,
						enum dvr_file_type type,
						struct list_head *entry_list);

/**
 * free_record_file_entry: free record file entry memory.
 * @entry: appointed record file entry
 * return: none
 * notes:
 */
void free_record_file_entry(struct dvr_file_entry *entry);
#endif
