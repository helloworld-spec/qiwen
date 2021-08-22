#ifndef _RECORD_FILE_H_
#define _RECORD_FILE_H_

#include <semaphore.h>
#include <time.h>

#include "list.h"
#include "ak_common.h"
#include "ak_record_common.h"

#define ONE_DAY_SECOND				(24*60*60)
#define TIME_ZONE 					8*3600 //beijing timezone

/* record file exception callback */
typedef void (* FP_RECORD_FILE_EXCEPT)(int except_code, const char *except_desc);
/* record file flush finished callback */
typedef void (* FP_RECORD_FILE_FLUSH)(unsigned char flush_finish);

/* record file pointer info */
struct record_file_fp {
	FILE *index;      //index fp
	FILE *record;     //record fp
};

/* record file param */
struct file_param {
    unsigned char cyc_flag;        		//cycle record flag
    enum record_file_type type;			//record file type
    char rec_prefix[RECORD_FILE_PREFIX_MAX_LEN];//record file name prefix
    char rec_path[RECORD_FILE_PATH_MAX_LEN];//record file saving path
    FP_RECORD_FILE_EXCEPT except_report;	//callback of exception report
    FP_RECORD_FILE_FLUSH flush_cb;			//flush finished callback
};

struct record_file_entry {
	time_t calendar_time;		//calendar time
	unsigned long size;			//file size
	unsigned long total_time;	//record file total time
	char *path;					//including path and file full name
	struct list_head list;		//list head
};

/**  
 * record_file_get_start_entry - get start record file entry by start time
 * @start_time[IN]: appointed start time
 * @type[IN]: record file type
 * @entry[OUT]: record file entry
 * return: 0 success, otherwise -1
 * notes: 1. we'll record the start node after get successfully,
 *		and then you can call record_file_get_next_entry
 *		2. you must free entry path memory outside
 */
int record_file_get_start_entry(time_t start_time, 
		enum record_file_type type, struct record_file_entry *entry);

/**  
 * record_file_get_next_entry - get next record file entry's full path
 * @type[IN]: record file type
 * @entry[OUT]: record file entry
 * return: 0 success, otherwise -1
 * notes: 1. make sure call record_file_get_start_entry firstly.
 *		2. you must free entry path memory outside
 */
int record_file_get_next_entry(enum record_file_type type, 
							struct record_file_entry *entry);

/**  
 * record_file_open - open record file.
 * @index[IN]: appointed record file index, switch between 0 and 1
 * @fp[OUT]: fp info after opened successfully, if you want to fetch
 * return: 0 success, otherwise -1
 * notes: 
 */
int record_file_open(int index,	struct record_file_fp *fp);

/**  
 * record_file_generate_name - generate record file name.
 * @save_index[IN]: appointed record file index, switch between 0 and 1
 * @date[IN]: record file date time
 * return: 0 success, otherwise -1
 * notes: 
 */
void record_file_generate_name(int save_index, const struct ak_date *date);

/**  
 * record_file_delete - delete record file.
 * @index: appointed record file index
 * return: none
 * notes: 
 */
void record_file_delete(int index);

/** 
 * record_file_wakeup - wakeup record file thread
 * @save_index[IN]: index of currently saving file
 * return: none
 * notes: 
 */
void record_file_wakeup(int save_index);

/** 
 * record_file_stop_search - stop searching history record file
 * @void
 * return: none
 */
void record_file_stop_search(void);

/** 
 * record_file_get_pre_record - wakeup get pre record file thread
 * @void
 * return: none
 * notes: 
 */
void record_file_get_pre_record(void);

/**  
 * record_file_init - init record file manage env and start record file thread.
 * @param: record file param
 * return: 0 success; othrewise -1
 */
int record_file_init(struct file_param *param);

/**  
 * record_file_exit - clear record file manage env and stop record file thread.
 * @void
 * return: none
 */
void record_file_exit(void);

#endif
