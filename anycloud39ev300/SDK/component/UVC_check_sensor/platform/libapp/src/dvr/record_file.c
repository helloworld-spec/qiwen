/**
 * Copyright (C) 2016 Anyka(Guangzhou) Microelectronics Technology CO.,LTD. 
 * File Name: record_file.c
 * Author: Huanghaitao
 * Update date: 2016-04-05
 * Description: generate record file module. 
 * History: V2.0.0 react revision
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "record_fs.h"
#include "record_file.h"

#include "ak_sd_card.h"
#include "ak_thread.h"
#include "ak_demux.h"

#ifdef AK_RTOS
#include "kernel.h"
#endif

#define RECORD_FILE_TEST			0
#define DUMP_RECORD_FILE			0
#define TEST_TOTAL_SIZE				(300*1024)	//300M
#define RECORD_FILE_VBUF_LEN		(1024*1024)
#define SYS_CMD_RESULT_MAX_LEN  	4096

#define RECORD_SAVE_PATH            "/mnt"
#define RECORD_FILE_TMP_PATH        "/mnt/tmp/"
#define MIN_DISK_SIZE_FOR_RECORD    (200*1024)  //disk reserved space(KB)

#define INVALID_FILE_FD             -1  		//invalid file fd
#define RECORD_NODE_NUM             2   		//record node number

#define RECORD_FILE_NAME_MAX_LEN    100 		//record file name max len in bytes

struct record_file {
    unsigned char run_flag;	    	//record file manage thread run flag
    unsigned char save_flag;	    //save record file flag
	unsigned char full_flag;		//SD card space is full

    unsigned long cur_time;		    //current time
    unsigned long free_size;		//sd card free space size in bytes
    unsigned long used_size;		//record file's used size in bytes

	ak_pthread_t old_file_tid;		//read old file thread ID
	ak_pthread_t main_tid;			//record file main thread ID
	
    ak_sem_t pre_record_sem;		//get pre record file sem
    ak_sem_t manage_sem;			//record file manage sem
    ak_sem_t finish_sem;			//get pre record file finished sem
    ak_mutex_t size_mutex;			//file size mutex
    ak_mutex_t entry_mutex;			//list entry mutex
    ak_mutex_t fp_mutex;			//record file fp mutex
    
	struct record_file_entry *start_entry;	//start entry for get record file
	struct list_head head[RECORD_FILE_TYPE_NUM];//record file list head

    int save_index;         		//index of current saving file
    struct record_file_fp fp[RECORD_NODE_NUM];   //record node fd
    char file_name[RECORD_NODE_NUM][RECORD_FILE_NAME_MAX_LEN];//record file name

	const char* tmp_file[RECORD_NODE_NUM];
	const char* tmp_index[RECORD_NODE_NUM];

	struct file_param param;		//record file param
	struct search_file_param search;//search record file param
};

static struct record_file record_file = {0};

/* record files info: one for temp file, another for saving file */
const char *record_tmp_file[] = {
	"/mnt/tmp/record_file_1",
	"/mnt/tmp/record_file_2"
};

const char *record_tmp_index[] = {
	"/mnt/tmp/record_index_1",
	"/mnt/tmp/record_index_2"
};

#if DUMP_RECORD_FILE
static void dump_list_record_file(void)
{
	int i = 0;
	int total_num = 0;
    struct record_file_entry *entry = NULL;

	for(i=0; i<RECORD_FILE_TYPE_NUM; ++i){
		if(list_empty(&(record_file.head[i]))){
			ak_print_normal_ex("record file list is empty\n");
		}else{
			total_num = 0;
			ak_print_normal_ex("record file type=%d, list:\n", i);
			list_for_each_entry(entry, &(record_file.head[i]), list){
				ak_print_normal("%s\n", entry->path);
				ak_print_error_ex("entry->list=0x%lX\n",(unsigned long)&(entry->list));
				ak_print_error_ex("prev=0x%lX, next=0x%lX\n\n", 
					(unsigned long)(entry->list.prev), (unsigned long)(entry->list.next));
				++total_num;
			}
			ak_print_normal_ex("--- record file type=%d total_num=%d ---\n", total_num, i);
		}
	}
}
#endif

/**  
 * report_exception_info: 
 * @file[IN]: current record type record file info
 * @ret_code[IN]: returned exception code
 * return: none
 * notes: 
 */ 
static void report_exception_info(struct record_file *file, int ret_code)
{
    if(NULL != file->param.except_report){
        switch(ret_code){
        case RECORD_EXCEPT_SD_NO_SPACE:
            break;
        case RECORD_EXCEPT_SD_RO:
        	break;
        default:
            break;
        }
        file->param.except_report(ret_code, "");
    }
}

/**  
 * copy_record_file_entry: malloc entry path, and copy entry info
 * @dest_entry[OUT]: dest entry
 * @src_entry[IN]: src entry
 * return: 0 success, -1 failed
 */
static int copy_record_file_entry(struct record_file_entry *dest_entry, 
								const struct record_file_entry *src_entry)
{
	if(!dest_entry || !src_entry){
		return AK_FAILED;
	}

	int ret = AK_FAILED;
    if(src_entry->path){
    	int path_len = strlen(src_entry->path);
        dest_entry->path = (char *)calloc(1, (path_len + 1));
        if(dest_entry->path) {
            dest_entry->calendar_time = src_entry->calendar_time;
			dest_entry->size = src_entry->size;
			dest_entry->total_time = src_entry->total_time;
			memcpy(dest_entry->path, src_entry->path, path_len);
		
            ret = AK_SUCCESS;
        }
    }
    
    return ret;
}

/**  
 * malloc_record_file_entry: malloc memory for record file entry.
 * @file_path_len: file path in bytes, including full file name
 * return: pointer of record file entry
 * notes: 
 */
static struct record_file_entry* malloc_record_file_entry(int file_path_len)
{
	if(file_path_len <= 0){
		return NULL;
	}
	
    struct record_file_entry *entry = (struct record_file_entry *)calloc(1,
    	sizeof(struct record_file_entry));
    if(NULL != entry){
        entry->path = (char *)calloc(1, file_path_len);
        if(NULL == entry->path){
            free(entry);
            entry = NULL;
        }
    }
    
    return entry;
}

/**  
 * free_record_file_entry: free record file entry memory.
 * @entry: appointed record file entry
 * return: none
 * notes: 
 */
static void free_record_file_entry(struct record_file_entry *entry)
{
    if(NULL != entry){
        if(NULL != entry->path){
            free(entry->path);
            entry->path = NULL;
        }
        
        free(entry);
        entry = NULL;
    }
}

/* check and remove record file which had been deleted from list */ 
static void remove_del_file(struct record_file *file)
{
	struct record_file_entry *ptr = NULL;
    struct record_file_entry *entry = NULL;
    int i = 0;

	ak_thread_mutex_lock(&(file->entry_mutex));
    for(i = 0; i < RECORD_FILE_TYPE_NUM; i++){
		list_for_each_entry_safe(entry, ptr, &(file->head[i]), list) {
			if (ak_sd_check_mount_status()) {
				ak_print_normal("SD card umount, we stop removing\n");
				break;
			}
			if (entry->path) {
				if (ak_check_file_exist(entry->path)) {
					ak_print_normal("remove record file: %s from list\n", 
						entry->path);
					list_del_init(&(entry->list));
			    	/* record file has been removed */
			    	free_record_file_entry(entry);
				}
			}
		}
	}
	ak_thread_mutex_unlock(&(file->entry_mutex));
}

/** get_entry_calendar_time: get record file entry's calendar time
 * @file_path[IN]: file path including full file name
 * return: 0 success; otherwise -1
 * notes: file name ex.: /mnt/CYC_DV/CYC_DV_20160504-095310.mp4
 */ 
static time_t get_entry_calendar_time(const char *file_path)
{
	/* get record start time from file name, CYC_DV_20150908-090511.mp4 */
	char *str = strrchr(file_path, '_');
	if(NULL != str){
	    ++str;
	}

	struct ak_date date;
	ak_string_to_date(str, &date);	    

    return ak_date_to_seconds(&date);
}

/**  
 * add_entry_into_order_list: add appointed entry into list
 * @entry[IN]: fetched record file entry
 * @head[IN]: current ordered list head
 * return: none
 * notes: file in old list is already in ASC order
 */ 
static void add_entry_into_order_list(struct record_file_entry *entry, 
									struct list_head *head)
{
    struct record_file_entry *ptr = NULL;

    /* add entry into the list by order */
	list_for_each_entry_reverse(ptr, head, list){
        if(NULL != ptr){		
    		if(entry->calendar_time == ptr->calendar_time){			
				/* the same record file */
				return;
			}
		
    		if(entry->calendar_time > ptr->calendar_time){
    			ak_thread_mutex_lock(&(record_file.entry_mutex));
			    list_add(&(entry->list), &(ptr->list));
			    ak_thread_mutex_unlock(&(record_file.entry_mutex));
    			return;
    		}
    	}
    }

    /* the earlist entry */
    ak_thread_mutex_lock(&(record_file.entry_mutex));
    list_add(&(entry->list), head);
    ak_thread_mutex_unlock(&(record_file.entry_mutex));
}

/**  
 * add_file_to_list - add the appointed record file into list.
 * @file_name[IN]: record file name
 * @file_size[IN]: record file size in bytes
 * @file_type[IN]: record file type
 * return: none
 * notes: 
 */ 
static void add_file_to_list(const char *file_name, 
							int file_size, 
							int file_type)
{
	/* total_time unit: ms */
    int total_time = ak_demux_get_total_time(file_name);
    if (total_time <= 0) {
    	ak_print_error_ex("video_demux_get_total_time FAILED\n");
    	ak_print_error_ex("file: %s\n", file_name);
    	remove(file_name);
    	return;
    }
    
	int file_len = strlen(file_name);
	struct record_file_entry *entry = malloc_record_file_entry(file_len + 4);
	if(NULL != entry){
        entry->calendar_time = get_entry_calendar_time(file_name);
		entry->size = file_size;
		entry->total_time = total_time;
		strcpy(entry->path, file_name);

		add_entry_into_order_list(entry, &(record_file.head[file_type]));
	}
}

/**  
 * remove_earliest_file - remove earliest record file
 * @file: current record file info
 * @occupy_size: current record file occupy size in byte
 * return: none
 * notes: if the room is not enough, delete the earliest record file
 */
static void remove_earliest_file(struct record_file *file, 
								unsigned long occupy_size)
{
	struct record_file_entry *ptr = NULL;
    struct record_file_entry *entry = NULL;
    enum record_file_type type = file->param.type;

	ak_thread_mutex_lock(&(file->entry_mutex));		
	list_for_each_entry_safe(entry, ptr, &(file->head[type]), list){
		
		ak_print_normal("used_size:%ld(KB), free_size=%ld(KB)\n",
		    file->used_size, file->free_size);
		ak_print_normal("room isn't enough, remove: %s\n", entry->path);

		remove(entry->path);
		ak_thread_mutex_lock(&(file->size_mutex));
		file->used_size -= entry->size;
		file->free_size += entry->size;
		ak_thread_mutex_unlock(&(file->size_mutex));
		list_del_init(&(entry->list));
		free_record_file_entry(entry);

		ak_print_normal("after remove, used_size:%ld(KB), free_size=%ld(KB)\n\n",
		    file->used_size, file->free_size);
		if (file->free_size > occupy_size) {
			break;			
		}
	}
	ak_thread_mutex_unlock(&(file->entry_mutex));
}

/**  
 * get_sdcard_record - get record saved in the SD card and save in old queue.
 * @file: current record file info
 * return: none
 * notes: 
 */
static void get_sdcard_record(struct record_file *file)
{
	struct ak_timeval tv_start = {0};
	struct ak_timeval tv_end = {0};
	unsigned long use_time = 0;
	unsigned long find_size = 0;

    ak_print_notice_ex("used_size=%ld, dir: %s\n",
    	file->used_size, file->param.rec_path);
	
    ak_get_ostime(&tv_start);
    find_size = record_fs_init_record_list(&record_file.search, add_file_to_list);
    ak_get_ostime(&tv_end);
	use_time = ak_diff_ms_time(&tv_end, &tv_start);

	ak_thread_mutex_lock(&(file->size_mutex));
	file->used_size += find_size;
	ak_thread_mutex_unlock(&(file->size_mutex));

#if RECORD_FILE_TEST
	file->free_size = TEST_TOTAL_SIZE;
#endif

#if DUMP_RECORD_FILE
	dump_list_record_file();
#endif
	
	remove_del_file(file);
    ak_print_notice_ex("the disk free size=%lu(KB), record size=%lu(KB)\n",
        file->free_size, file->used_size);
	ak_print_notice_ex("saved use_time=%ld(ms)\n\n", use_time);

    unsigned char size_enough = AK_TRUE;
    if(file->param.cyc_flag){
    	/* free size isn't enough */
        if(file->free_size < MIN_DISK_SIZE_FOR_RECORD) {
        	ak_print_normal_ex("the disk free size isn't enough!"
    			"(%lu,%lu,%d)\n", 
    			file->free_size, file->used_size, MIN_DISK_SIZE_FOR_RECORD);
        	/* available size is enough, and we'll remove earliest files */
        	if ((file->free_size + file->used_size) > MIN_DISK_SIZE_FOR_RECORD) {
        		ak_print_normal_ex("we'll remove earliest record files\n");
	            remove_earliest_file(file, MIN_DISK_SIZE_FOR_RECORD);
        	} else {
        		/* available size isn't enough either */
        		ak_print_normal_ex("available size isn't enough either\n");
        		size_enough = AK_FALSE;
        	}
        }
    }else{
       if(file->free_size < MIN_DISK_SIZE_FOR_RECORD){
            ak_print_normal_ex("the disk free size isn't enough!(%lu,%d)\n",
			    file->free_size, MIN_DISK_SIZE_FOR_RECORD);
            size_enough = AK_FALSE;
        }
    }

    if(size_enough){
    	ak_thread_mutex_lock(&(file->size_mutex));
		/* reserved appointed disk size */
        file->free_size -= MIN_DISK_SIZE_FOR_RECORD;
        ak_thread_mutex_unlock(&(file->size_mutex));
    }else{
        report_exception_info(file, RECORD_EXCEPT_SD_NO_SPACE);
    }
}

/**  
 * get_pre_saved_record - get pre existing record and save in old queue.
 * @arg: thread input arg, current record type record file info
 * return: none
 * notes: 
 */
static void* get_pre_saved_record(void *arg)
{
    ak_print_normal_ex("--- thread id: %ld ---\n", ak_thread_get_tid());
    
    struct record_file *file = (struct record_file *)arg;
    while (file->run_flag) {
        ak_thread_sem_wait(&(file->pre_record_sem));
        if (file->run_flag) {
        	ak_print_notice_ex("--- start get record file in SD card ---\n");
        	get_sdcard_record(file);        	
        	if ( record_file.full_flag) {
        		ak_thread_sem_post(&(file->finish_sem));
        	}
        }
	}

	ak_print_normal_ex("### %ld thread exit ###\n\n", ak_thread_get_tid());
	
	ak_thread_exit();
	return NULL;
}

static void delete_saved_file_entry(struct record_file *file)
{
	int i = 0;
	int entry_num = 0;
    struct record_file_entry *ptr = NULL;
    struct record_file_entry *entry = NULL;

	ak_thread_mutex_lock(&(file->entry_mutex));
	for(i=0; i<RECORD_FILE_TYPE_NUM; ++i){
		entry_num = 0;
		list_for_each_entry_safe(entry, ptr, &(file->head[i]), list){
			++entry_num;
			free_record_file_entry(entry);
			list_del_init(&(entry->list));
		}
		ak_print_info_ex("record file type=%d, delete entry_um=%d\n", 
			i, entry_num);
	}
	ak_thread_mutex_unlock(&(file->entry_mutex));
}

/**  
 * update_used_size - update record file's used size.
 * @file_size: current record file's size in bytes
 * return: used size in bytes after update
 * notes: 
 */
static unsigned long update_used_size(unsigned long file_size)
{
	ak_thread_mutex_lock(&(record_file.size_mutex));
	record_file.used_size += file_size;
	record_file.free_size -= file_size;
	ak_thread_mutex_unlock(&(record_file.size_mutex));

	return record_file.used_size;
}

static void sync_record_file(struct record_file *file)
{
	struct ak_timeval tv_start = {0};
	struct ak_timeval tv_end = {0};
	int index = file->save_index;

	ak_get_ostime(&tv_start);
    if (fdatasync(fileno(file->fp[index].record))) {
    	ak_print_error_ex("fdatasync errno=%d, desc: %s\n", errno, strerror(errno));
    }
	ak_get_ostime(&tv_end);
	ak_print_normal_ex("fdatasync use time: %ld(ms)\n",
		ak_diff_ms_time(&tv_end, &tv_start));
	
	if (fclose(file->fp[index].record)) {
		ak_print_error_ex("fsync errno=%d, desc: %s\n", errno, strerror(errno));
	}
	file->fp[index].record = NULL;
}

static void generate_record_file(struct record_file *file, 
								int index, unsigned long occupy_size)
{
	/** update the current file message into the record queue **/
	update_used_size(occupy_size);
	
    /* rename the record file */
	rename(file->tmp_file[index], file->file_name[index]);
	ak_print_normal_ex("record file index=%d, name: %s\n", 
		index, file->file_name[index]);
		
	add_file_to_list(file->file_name[index], occupy_size, file->param.type);

	ak_print_normal_ex("record file index=%d, used_size=%ld KB\n", 
		index, file->used_size);
	ak_print_normal("------------------------------------------------------"
		"--------------------------\n\n");
}

static void stat_record_file(struct record_file *file)
{
	struct stat statbuf;
	int index = file->save_index;
			
	/** get record file information **/
	stat(file->tmp_file[index], &statbuf);
	//float real_size = (statbuf.st_size >> 10);
	unsigned long occupy_size = (statbuf.st_blocks >> 1);
	ak_print_normal_ex("record file index=%d, st_size=%ld(Bytes), occupy_size=%ld KB\n", 
		index, statbuf.st_size, occupy_size);
		
	/** file size less than 100k, delete it **/
	if(statbuf.st_size < (100 * 1024)){
		remove(file->tmp_file[index]);
	}else{
		unsigned char generate_flag = AK_TRUE;
		
		if(occupy_size > file->free_size){
        	if(file->param.cyc_flag){
        		remove_earliest_file(file, occupy_size);
        	}else{
            	report_exception_info(file, RECORD_EXCEPT_SD_NO_SPACE);
            	generate_flag = AK_FALSE;
            }
        }

		if (generate_flag) {
			generate_record_file(file, index, occupy_size);
		}
	}
}

/**  
 * flush_record_file - flush record file.close temp file after mux.
 * @file: current record type record file info
 * return: none
 * notes: record file index, switch between 0 and 1
 */
static void flush_record_file(struct record_file *file)
{
	int index = file->save_index;

	ak_thread_mutex_lock(&(file->fp_mutex));
	ak_print_normal("\n\t ----- %s -----\n", __func__);
    ak_print_normal_ex("current save file[%d]: %s\n",
    	index, file->file_name[index]);
	ak_print_normal_ex("fp[%d].index=0x%lX, fp[%d].record=0x%lX\n", 
    	index, (unsigned long)(file->fp[index].index), 
    	index, (unsigned long)(file->fp[index].record));
        	
	/* close last index file */
	if(file->fp[index].index){
		fclose(file->fp[index].index);
		file->fp[index].index = NULL;
		remove(file->tmp_index[index]);
	}

	/* close last record file, rename the tmp file name for next time record */
	if(file->fp[index].record){
		sync_record_file(file);
		stat_record_file(file);
	}

	file->fp[index].index = NULL;
	file->fp[index].record = NULL;
	if (file->param.flush_cb) {
		file->param.flush_cb(AK_TRUE);
	}
	ak_thread_mutex_unlock(&(file->fp_mutex));
}

static void create_record_dir(void)
{
	/* check and create temp directory: /mnt/tmp/ */
	if (ak_check_file_exist(RECORD_FILE_TMP_PATH)) {
		ak_print_warning_ex("%s not exist, we'll create it\n", 
			RECORD_FILE_TMP_PATH);
		if (record_fs_create_dir(RECORD_FILE_TMP_PATH)) {
			ak_print_error_ex("%s create failed\n", RECORD_FILE_TMP_PATH);
		}
	} else {
		ak_print_normal_ex("%s already exists\n", RECORD_FILE_TMP_PATH);
	}

    /* check and create appointed record saving path, e.g: /mnt/CYC_DV/ */
	if (ak_check_file_exist(record_file.param.rec_path)) {
		ak_print_warning_ex("%s not exist, we'll create it\n", 
			record_file.param.rec_path);
		if (record_fs_create_dir(record_file.param.rec_path)) {
			ak_print_error_ex("%s create failed\n", record_file.param.rec_path);
		}
	} else {
		ak_print_normal_ex("%s already exists\n", record_file.param.rec_path);
	}
}

/**  
 * record_file_main
 * @arg: thread input arg, current record type record file info
 * return: none
 * notes: 
 */
static void* record_file_main(void *arg)
{
    ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());

    struct record_file *file = (struct record_file *)arg;
    while(file->run_flag){
    	ak_print_info_ex("sleep...\n");
        ak_thread_sem_wait(&(file->manage_sem));
        ak_print_info_ex("wakeup, run_flag=%d\n", file->run_flag);
        if(!(file->run_flag)){
        	break;
        }
        
        flush_record_file(file);
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	
	ak_thread_exit();
	return NULL;
}

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
		enum record_file_type type, struct record_file_entry *entry)
{
    if(!entry){
    	ak_print_normal("get entry is NULL\n");
        return AK_FAILED;
    }
    
	int ret = AK_FAILED;
    struct record_file_entry *ptr = NULL;
	int time_interval = 0;

	list_for_each_entry (ptr, &(record_file.head[type]), list) {
        if (start_time == ptr->calendar_time) {
			ak_print_normal_ex("start_time == entry->calendar_time, file: %s\n", 
				ptr->path);
			ret = AK_SUCCESS;
			break;
		}
		
		time_interval = ptr->calendar_time - TIME_ZONE - start_time;
		
		if((time_interval < ONE_DAY_SECOND) && (time_interval >= 0)){
			ak_print_normal_ex("start time inner file: %s\n", ptr->path);
			ret = AK_SUCCESS;
			break;
		}
    }
    
	if(AK_SUCCESS == ret){
		record_file.start_entry = ptr;
		ret = copy_record_file_entry(entry, ptr);
	}
		
	return ret;
}

/**  
 * record_file_get_next_entry - get next record file entry's full path
 * @type[IN]: record file type
 * @entry[OUT]: record file entry
 * return: 0 success, otherwise -1
 * notes: 1. make sure call record_file_get_start_entry firstly.
 *		2. you must free entry path memory outside
 */
int record_file_get_next_entry(enum record_file_type type, 
							struct record_file_entry *entry)
{
    if(!entry || (NULL == record_file.start_entry)){
    	ak_print_normal("pointer is NULL\n");
        return AK_FAILED;
    }

	int ret = AK_FAILED;
    struct record_file_entry *ptr = record_file.start_entry;
	list_for_each_entry_continue(ptr, &(record_file.head[type]), list){
		if(NULL != ptr){
    		record_file.start_entry = ptr;
    		ret = copy_record_file_entry(entry, ptr);
        	break;
    	}
    }
    
    return ret;
}

/**  
 * record_file_open - open record file.
 * @index[IN]: appointed record file index, switch between 0 and 1
 * @fp[OUT]: fp info after opened successfully, if you want to fetch
 * return: 0 success, otherwise -1
 * notes: fopen default is fully buffered, and the buffered size is 
 *		st_blksize in struct stat
 *		we use this size: /mnt/tmp/ is vfat fs, st_blksize=32768, st_blocks=64
 */
int record_file_open(int index,	struct record_file_fp *fp)
{
    if((index < 0) || (index >= RECORD_NODE_NUM)){
        return AK_FAILED;
    }

	int ret = AK_FAILED;
	
	ak_thread_mutex_lock(&(record_file.fp_mutex));
	record_file.fp[index].record = fopen(record_file.tmp_file[index], "wb+");
	if(NULL == record_file.fp[index].record){
		ak_print_normal_ex("%s\n", strerror(errno));
		/* read-only file system */
		if(EROFS == errno) {
			report_exception_info(&record_file, RECORD_EXCEPT_SD_RO);
		}
		goto file_open_end;
	}
	
#if 0	
	if (setvbuf(record_file.fp[index].record, NULL, _IOFBF, RECORD_FILE_VBUF_LEN)) {
		ak_print_error_ex("setvbuf 1M for record file failed, desc: %s\n", 
			strerror(errno));
		goto file_open_end;
	} else {
		ak_print_notice_ex("setvbuf 1M for record file OK\n");
	}
#endif	

	record_file.fp[index].index = fopen(record_file.tmp_index[index], "wb+");
	if(NULL == record_file.fp[index].index){
		ak_print_normal_ex("open file %s failed: %s\n", record_file.tmp_index[index], 
		    strerror(errno));
		goto file_open_end;
	}

	if(NULL != fp){
        fp->index = record_file.fp[index].index;
        fp->record = record_file.fp[index].record;
    }
    if (record_file.param.flush_cb) {
		record_file.param.flush_cb(AK_FALSE);
	}
    ret = AK_SUCCESS;

file_open_end:
	if (AK_FAILED == ret) {
		if(record_file.fp[index].record){
			fclose(record_file.fp[index].record);
			record_file.fp[index].record = NULL;
			remove(record_file.tmp_file[index]);
		}
	}
	ak_thread_mutex_unlock(&(record_file.fp_mutex));
	
	return ret;
}

/**  
 * record_file_generate_name - generate record file name.
 * @save_index[IN]: appointed record file index, switch between 0 and 1
 * @date[IN]: record file date time
 * return: 0 success, otherwise -1
 * notes: 
 */
void record_file_generate_name(int save_index, const struct ak_date *date)
{
	if(save_index >= RECORD_NODE_NUM){
		return;
	}

	/* get plan record file name */
	char file_ext[FILE_EXT_NAME_MAX_LEN+1] = {0};
	switch(record_file.param.type){
	case RECORD_FILE_TYPE_MP4:
		memcpy(file_ext, ".mp4", FILE_EXT_NAME_MAX_LEN);
		break;
	case RECORD_FILE_TYPE_AVI:
		memcpy(file_ext, ".avi", FILE_EXT_NAME_MAX_LEN);
		break;
	default:
		ak_print_normal_ex("record_file.param.type error!\n");
    	return;
	}

	if (!record_fs_get_video_name(record_file.param.rec_path, 
		record_file.param.rec_prefix, file_ext, date, 
		record_file.file_name[save_index])) {
		ak_print_normal_ex("save_index=%d, file: %s\n\n",
			save_index, record_file.file_name[save_index]);
	}
}

/**  
 * record_file_delete - delete record file.
 * @record_type: current record type
 * @index: appointed record file index
 * return: none
 * notes: 
 */
void record_file_delete(int index)
{
	ak_thread_mutex_lock(&(record_file.fp_mutex));
    if(NULL != record_file.fp[index].index){
        fclose(record_file.fp[index].index);
        remove(record_file.tmp_index[index]);
    }
    
    if(NULL != record_file.fp[index].record){
        fclose(record_file.fp[index].record);
        remove(record_file.tmp_file[index]);
    }
    
    record_file.fp[index].index = NULL;
    record_file.fp[index].record = NULL;
    ak_thread_mutex_unlock(&(record_file.fp_mutex));
}

/** 
 * record_file_wakeup - wakeup record file thread
 * @save_index[IN]: index of currently saving file
 * return: none
 * notes: 
 */
void record_file_wakeup(int save_index)
{
    record_file.save_index = save_index; 
    ak_thread_sem_post(&(record_file.manage_sem));
}

/** 
 * record_file_stop_search - stop searching history record file
 * @void
 * return: none
 */
void record_file_stop_search(void)
{
    record_file.search.run_flag = AK_FALSE;
    //ak_thread_sem_post(&(record_file.finish_sem));
}

/** 
 * record_file_get_pre_record - wakeup get pre record file thread
 * @void
 * return: none
 * notes: SD card may reinsert
 */
void record_file_get_pre_record(void)
{
	/* check and create temp & record saving dir */
	create_record_dir();

	struct record_file *file = &record_file;
	ak_thread_mutex_lock(&(file->size_mutex));
	file->search.run_flag = AK_TRUE;
	file->free_size = record_fs_get_disk_free_size(file->param.rec_path);
	ak_thread_mutex_unlock(&(file->size_mutex));

	ak_print_notice_ex("dir current free_size=%ld\n", file->free_size);
	ak_thread_sem_post(&(file->pre_record_sem));

	if (file->free_size < MIN_DISK_SIZE_FOR_RECORD) {
		ak_print_notice_ex("free_size=%ld, waiting get pre saved record...\n",
			file->free_size);
		file->full_flag = AK_TRUE;
		ak_thread_sem_wait(&(file->finish_sem));
		ak_print_notice_ex("get pre saved record OK\n");
	}
}

/**  
 * record_file_init - init record file manage env and start record file thread.
 * @param: record file param
 * return: 0 success; othrewise -1
 */
int record_file_init(struct file_param *param)
{
    if(record_file.run_flag){
        return AK_FAILED;
    }
    
    record_file.run_flag = AK_TRUE;
    record_file.full_flag = AK_FALSE;
    record_file.cur_time = 0;
    record_file.used_size = 0;
    record_file.free_size = 0;
    record_file.save_index = 0;
    
    memcpy(&(record_file.param), param, sizeof(record_file.param));
    record_file.search.run_flag = AK_TRUE;
    strcpy(record_file.search.prefix, record_file.param.rec_prefix);
    strcpy(record_file.search.path, record_file.param.rec_path);

	ak_print_normal("\n*** %s param ***\n", __func__);
	ak_print_normal("cyc_flag=%d, type=%d\n", 
		record_file.param.cyc_flag, record_file.param.type);
	ak_print_normal("rec_prefix: %s\n", record_file.param.rec_prefix);
	ak_print_normal("rec_path: %s\n", record_file.param.rec_path);
	ak_print_normal("*** %s param End ***\n\n", __func__);

    int i = 0;
    for(i=0; i<RECORD_NODE_NUM; ++i){
    	record_file.fp[i].index = NULL;
    	record_file.fp[i].record = NULL;
        record_file.tmp_file[i] = record_tmp_file[i];
		record_file.tmp_index[i] = record_tmp_index[i];
    }

    ak_thread_mutex_init(&(record_file.size_mutex));
    ak_thread_mutex_init(&(record_file.entry_mutex));
    ak_thread_mutex_init(&(record_file.fp_mutex));
    
    ak_thread_sem_init(&(record_file.pre_record_sem), 0);
    ak_thread_sem_init(&(record_file.manage_sem), 0);
    ak_thread_sem_init(&(record_file.finish_sem), 0);

    record_file.start_entry = NULL;
    for(i=0; i<RECORD_FILE_TYPE_NUM; i++){
    	INIT_LIST_HEAD(&(record_file.head[i]));
    }

	int ret = ak_thread_create(&(record_file.old_file_tid), get_pre_saved_record,
        (void *)&record_file, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if(ret){
		ak_print_error_ex("create get_pre_saved_record thread, ret=%d!\n", ret);
	}

    #ifdef AK_RTOS
    ret = ak_thread_create(&(record_file.main_tid), record_file_main,
        (void *)&record_file, ANYKA_THREAD_MIN_STACK_SIZE, 90);
    #else
    ret = ak_thread_create(&(record_file.main_tid), record_file_main,
        (void *)&record_file, ANYKA_THREAD_MIN_STACK_SIZE, 99);
    #endif
	if(ret){
		ak_print_normal_ex("unable to create record_file_main thread, ret=%d!\n", ret);
	}

	return ret;
}

/**  
 * record_file_exit - clear record file manage env and stop record file thread.
 * @void
 * return: none
 */
void record_file_exit(void)
{
	ak_print_info_ex("enter...\n");
	if(record_file.run_flag){
		record_file.run_flag = AK_FALSE;
		record_file.search.run_flag = AK_FALSE;

		ak_thread_sem_post(&(record_file.pre_record_sem));
		ak_print_notice_ex("join get pre saved record thread\n");
		ak_thread_join(record_file.old_file_tid);
		ak_print_notice_ex("get pre saved record thread join OK\n");

		ak_thread_sem_post(&(record_file.manage_sem));
		ak_print_notice_ex("join record file main thread\n");
		ak_thread_join(record_file.main_tid);
		ak_print_notice_ex("record file main thread join OK\n");
		
		delete_saved_file_entry(&record_file);

		ak_thread_mutex_destroy(&(record_file.size_mutex));
		ak_thread_mutex_destroy(&(record_file.entry_mutex));
		ak_thread_mutex_destroy(&(record_file.fp_mutex));
		
	    ak_thread_sem_destroy(&(record_file.pre_record_sem));
	    ak_thread_sem_destroy(&(record_file.manage_sem));
	    ak_thread_sem_destroy(&(record_file.finish_sem));
	}

	ak_print_normal("\t ***** %s *****\n", __func__);
}
