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
#include <dirent.h>

#include "record_fs.h"
#include "record_common.h"
#include "internal_error.h"

#include "ak_sd_card.h"
#include "ak_thread.h"
#include "ak_mux.h"
#include "ak_demux.h"
#include "ak_dvr_disk.h"
#include "ak_dvr_file.h"
//#include "printcolor.h"

#define DVR_REPLAY_LIST_DEBUG        0

#define TIME_ZONE                     (8*60*60)    //beijing timezone

#define RECORD_FILE_TEST            0
#define DUMP_RECORD_FILE            0
#define TEST_TOTAL_SIZE                (300*1024)    //300M
#define RECORD_FILE_VBUF_LEN        (1024*1024)
#define SYS_CMD_RESULT_MAX_LEN      4096

//#define RECORD_SAVE_PATH            "/mnt"
#define RECORD_FILE_TMP_PATH        "/mnt/tmp/"
#define RECORD_FILE_INFO_NAME        "record_file_info"

#define MIN_DISK_SIZE_FOR_RECORD    (200*1024)  //disk reserved space(KB)
#define RESERVED_PERCENT               (10)        //T卡的保留空余率

#define INVALID_FILE_FD             -1          //invalid file fd
#define RECORD_NODE_NUM             2           //record node number

/* add record file total time to file name, such as: @300100 */
#define REC_TIME_INFO_LEN           8

#define RECORD_FILE_NAME_MAX_LEN    100         //record file name max len in bytes

#define DVR_DAY                     35          //Parameters for DVR POLL INIT 考虑到在创建列表的时候会使用录像文件列表信息,改长天数的索引长度
#define DVR_HOUR                    24          //24小时
#define DVR_MIN                     60          //60分钟
#define DVR_INTERVAL                1           //录像时长

#define LEN_RECORD_INFO             50

enum dvr_file_status {
    DVR_FILE_STATUS_RESERVED = 0x00,
    DVR_FILE_STATUS_READY,
    DVR_FILE_STATUS_FETCHING,
    DVR_FILE_STATUS_FETCH_OK
};

struct record_dir_entry {
    time_t calendar_time;        //calendar time, seconds from 1970-1-1 00:00:00
    char dir_path[DVR_PATH_LEN];            //record dir path
    struct list_head file_head;    //list head for record file list
    struct list_head list;
#ifndef USE_PTR_PATH
    int using_flg;                //using flag, 1 is using
#endif
};

struct record_file {
    unsigned char run_flag;            //record file manage thread run flag
    unsigned char save_flag;        //save record file flag
    unsigned char full_flag;        //SD card space is full

    unsigned long cur_time;            //current time
    unsigned long free_size;        //sd card free space size in bytes
    unsigned long used_size;        //record file's used size in bytes

    ak_pthread_t old_file_tid;        //read old file thread ID
    ak_pthread_t main_tid;            //record file main thread ID

    ak_sem_t pre_record_sem;        //get pre record file sem
    ak_sem_t manage_sem;            //record file manage sem
    ak_sem_t finish_sem;            //get pre record file finished sem
    ak_mutex_t size_mutex;            //file size mutex
    ak_mutex_t entry_mutex;            //list entry mutex
    ak_mutex_t fp_mutex;            //record file fp mutex

    struct list_head head[DVR_FILE_TYPE_NUM];//record file list head
    unsigned int total_num;            //record file total number
    unsigned int replay_count;        //replay entry count after get list each time

    struct ak_date save_date;       //save record file date
    int name_len;                   //temp record file name len
    FILE *name_tmp;                 //temp record file name info

    int save_index;                 //index of current saving file
    struct record_file_fp fp[RECORD_NODE_NUM];   //record node fd
    char file_name[RECORD_NODE_NUM][RECORD_FILE_NAME_MAX_LEN];//record file name

    const char* tmp_file[RECORD_NODE_NUM];
    const char* tmp_index[RECORD_NODE_NUM];
    const char *tmp_moov[RECORD_NODE_NUM];
    const char *tmp_stbl[RECORD_NODE_NUM];

    enum dvr_file_status status;        //current status
    struct dvr_file_param param;        //record file param
    struct search_file_param search;    //search record file param
    FP_DVR_FILE_EXCEPT except_report;   //callback of exception report
       FP_DVR_FILE_FLUSH flush_cb;            //callback of flush finished
};

static unsigned long reserved_size = MIN_DISK_SIZE_FOR_RECORD; //kBytes
static struct record_file record_file = {0};

/* record files info: one for temp file, another for saving file */
const char *record_tmp_file[] = {
    "record_file_0",
    "record_file_1"
};

const char *record_tmp_index[] = {
    "record_index_0",
    "record_index_1"
};

const char *record_tmp_moov[] = {
    "record_moov_0",
    "record_moov_1"
};

const char *record_tmp_stbl[] = {
    "record_stbl_0",
    "record_stbl_1"
};

#if DUMP_RECORD_FILE
static void dump_dir_record_file(struct list_head *file_head)
{
    int total_num = 0;
    struct dvr_file_entry *entry = NULL;

    list_for_each_entry(entry, file_head, list){
        ak_print_normal("%s\n", entry->path);
        ++total_num;
    }
    ak_print_normal_ex("dir total_num=%d\n\n", total_num);
}

static void dump_list_dir(void)
{
    int i = 0;
    struct record_dir_entry *dir = NULL;

    for(i=0; i<DVR_FILE_TYPE_NUM; ++i){
        if(list_empty(&(record_file.head[i]))){
            ak_print_normal_ex("record file type=%d dir list is empty\n", i);
            continue;
        }

        ak_print_normal_ex("record file type=%d, list:\n", i);
        list_for_each_entry(dir, &(record_file.head[i]), list){
            ak_print_normal("%s\n", dir->dir_path);
            dump_dir_record_file(&(dir->file_head));
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
    if(NULL != file->except_report){
        switch(ret_code){
        case DVR_EXCEPT_SD_NO_SPACE:
            break;
        case DVR_EXCEPT_SD_RO:
            break;
        default:
            break;
        }
        file->except_report(ret_code, "");
    }
}

#ifdef USE_PTR_PATH                                                             //使用动态分配录像文件记录节点方式
/**
 * copy_record_file_entry: malloc entry path, and copy entry info
 * @src_entry[IN]: src entry
 * return: pointer of record file entry, NULL failed
 * notes: should free dest_entry path outside.
 */
static struct dvr_file_entry* copy_record_file_entry(
                                const struct dvr_file_entry *src_entry)
{
    if (!src_entry) {
        return NULL;
    }

    struct dvr_file_entry *dest_entry = (struct dvr_file_entry *)calloc(1,
        sizeof(struct dvr_file_entry));
    if (dest_entry && src_entry->path) {
        int path_len = strlen(src_entry->path);
        dest_entry->path = (char *)calloc(1, (path_len + 1));
        if (dest_entry->path) {
            dest_entry->calendar_time = src_entry->calendar_time;
            dest_entry->size = src_entry->size;
            dest_entry->total_time = src_entry->total_time;
            memcpy(dest_entry->path, src_entry->path, path_len);
        } else {
            free(dest_entry);
            dest_entry = NULL;
        }
    }

    return dest_entry;
}

/**
 * malloc_record_file_entry: malloc memory for record file entry.
 * @file_path_len: file path in bytes, including full file name
 * return: pointer of record file entry
 * notes:
 */
static struct dvr_file_entry* malloc_record_file_entry(int file_path_len)
{
    if(file_path_len <= 0){
        return NULL;
    }

    struct dvr_file_entry *entry = (struct dvr_file_entry *)calloc(1,
        sizeof(struct dvr_file_entry));
    if(NULL != entry){
        entry->path = (char *)calloc(1, file_path_len);
        if(NULL == entry->path){
            free(entry);
            entry = NULL;
        }
    }
    return entry;
}

void free_record_dir_entry(struct record_dir_entry *entry)
{
    FREE_POINT(entry);
}

/**
 * free_record_file_entry: free record file entry memory.
 * @entry: appointed record file entry
 * return: none
 * notes:
 */
void free_record_file_entry(struct dvr_file_entry *entry)
{
    if(NULL != entry){
        FREE_POINT(entry->path)
        FREE_POINT(entry);
    }
}

static struct record_dir_entry* malloc_record_dir_entry()
{
    return (struct record_dir_entry *)calloc(1, sizeof(struct record_dir_entry));
}
#else                                                                           //预先分配录像节点方式

static int mem_pool_init_flag = AK_FALSE, file_top = 0, dir_top = 0, total_node = 0, total_dir = 0;
static struct dvr_file_entry *file_mempool_head = NULL;
static struct record_dir_entry *dir_mempool_head = NULL;
static ak_mutex_t mutex_t_pool_file, mutex_t_pool_dir;
static int mem_pool_init(void)
{
    total_node = DVR_DAY * DVR_HOUR * (DVR_MIN/DVR_INTERVAL);
    total_dir = DVR_DAY * DVR_HOUR;
    if (!file_mempool_head) {
        ak_print_notice_ex("init memory pool, total node number=%d\n", total_node);
        file_mempool_head = (struct dvr_file_entry *)calloc(total_node, sizeof(struct dvr_file_entry));
        dir_mempool_head = (struct record_dir_entry *)calloc(DVR_DAY * DVR_HOUR, sizeof(struct record_dir_entry));
        ak_print_notice_ex("node size=%d\n", total_node * sizeof(struct dvr_file_entry));
        ak_print_notice_ex("dir size=%d\n", DVR_DAY * DVR_HOUR * sizeof(struct record_dir_entry) );
        if ((file_mempool_head == NULL) || (dir_mempool_head == NULL)) {
            ak_print_error_ex("calloc mempoll fail\n");
            FREE_POINT(file_mempool_head);
            FREE_POINT(dir_mempool_head);
            return AK_FALSE;
        }
        ak_thread_mutex_init(&mutex_t_pool_file, NULL);
        ak_thread_mutex_init(&mutex_t_pool_dir, NULL);
        mem_pool_init_flag = AK_TRUE;
        return AK_TRUE;
    }

    return AK_FALSE;
}

static void mem_pool_release(void)
{
    if (mem_pool_init_flag == AK_TRUE) {
        FREE_POINT(file_mempool_head);
        FREE_POINT(dir_mempool_head);
        ak_thread_mutex_destroy(&mutex_t_pool_file);
        ak_thread_mutex_destroy(&mutex_t_pool_dir);
        mem_pool_init_flag = AK_FALSE;
        ak_print_notice_ex("memory pool released\n");
    }
}

static struct dvr_file_entry* malloc_record_file_entry(int file_path_len)       //使用静态分配方式分配录像文件描述结构
{

    /* find a node */
    struct dvr_file_entry *entry = NULL;

    ak_thread_mutex_lock(&mutex_t_pool_file);
    do {
         entry = file_mempool_head + (file_top++);
    } while ((file_top < total_node) && entry && (entry->using_flg != AK_FALSE));

    //ak_print_notice_ex("file_top= %d total_node= %d entry->using_flg= %d\n", file_top, total_node , entry->using_flg);
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "file_top= %d total_node= %d entry->using_flg= %d\n", file_top, total_node , entry->using_flg )
    if (file_top < total_node) {
        entry->using_flg = AK_TRUE;
    } else {
        entry = NULL;
        file_top = 0;
    }
    ak_thread_mutex_unlock(&mutex_t_pool_file);
    return entry;
}

static struct record_dir_entry* malloc_record_dir_entry()
{

    /* find a node */
    struct record_dir_entry *entry = NULL;

    ak_thread_mutex_lock(&mutex_t_pool_dir);
    do {
         entry = dir_mempool_head + (dir_top++);
    } while ((dir_top < total_dir) && entry && (entry->using_flg != AK_FALSE));

    if (dir_top < total_dir) {
        entry->using_flg = AK_TRUE;
    } else {
        entry = NULL;
        dir_top = 0;
    }
    ak_thread_mutex_unlock(&mutex_t_pool_dir);
    return entry;
}

void free_record_file_entry(struct dvr_file_entry *entry)
{
    ak_thread_mutex_lock(&mutex_t_pool_file);
    if (entry && (entry->using_flg == AK_TRUE)) {
        entry->using_flg = AK_FALSE;
    }
    ak_thread_mutex_unlock(&mutex_t_pool_file);
}

void free_record_dir_entry(struct record_dir_entry *entry)
{
    ak_thread_mutex_lock(&mutex_t_pool_dir);
    if (entry && (entry->using_flg == AK_TRUE)) {
        entry->using_flg = AK_FALSE;
    }
    ak_thread_mutex_unlock(&mutex_t_pool_dir);
}

static struct dvr_file_entry* copy_record_file_entry(
        const struct dvr_file_entry *src_entry)
{
    struct dvr_file_entry *dest_entry = malloc_record_file_entry(0);

    if (dest_entry) {
        dest_entry->calendar_time = src_entry->calendar_time;
        dest_entry->size = src_entry->size;
        dest_entry->total_time = src_entry->total_time;
        memcpy(dest_entry->path, src_entry->path, DVR_PATH_LEN);
        /*
        DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE,
                             "dest_entry= %p -> calendar_time= %lu size= %lu total_time= %lu path= '%s' using_flg= %d\n",
                             dest_entry, dest_entry->calendar_time, dest_entry->size , dest_entry->total_time, dest_entry->path, dest_entry->using_flg )
                             */
    } else {
        dest_entry = NULL;
    }

    return dest_entry;
}
#endif

static void delete_empty_dir(struct record_dir_entry *dir_entry)
{
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_entry->dir_path= '%s'\n", dir_entry->dir_path )
    if(list_empty(&(dir_entry->file_head))) {
        ak_print_notice_ex("record file dir: %s is empty, delete it\n", dir_entry->dir_path);
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED, "dir_entry->dir_path= '%s'\n", dir_entry->dir_path )
        remove(dir_entry->dir_path);
        list_del_init(&(dir_entry->list));
        free_record_dir_entry(dir_entry);
    }
}

/* check and remove record file which had been deleted from list */
static void remove_del_file(struct record_file *file)
{
    struct dvr_file_entry *ptr = NULL;
    struct dvr_file_entry *entry = NULL;
    struct record_dir_entry *dir_ptr = NULL;
    struct record_dir_entry *dir_entry = NULL;
    int i = 0;

    ak_thread_mutex_lock(&(file->entry_mutex));
    for(i = 0; i < DVR_FILE_TYPE_NUM; i++){
        list_for_each_entry_safe(dir_entry, dir_ptr, &(file->head[i]), list) {
            if (ak_sd_check_mount_status()) {
                ak_print_normal("SD card umount, we stop removing\n");
                break;
            }

            list_for_each_entry_safe(entry, ptr, &(dir_entry->file_head), list) {
                if (entry->path && ak_check_file_exist(entry->path)) {
                    ak_print_normal("remove record file: %s from list\n",
                        entry->path);
                    list_del_init(&(entry->list));
                    /* record file has been removed from SD card */
                    free_record_file_entry(entry);
                }
            }

            delete_empty_dir(dir_entry);
            dir_entry = NULL;
        }
    }
    ak_thread_mutex_unlock(&(file->entry_mutex));
}

static int add_to_replay_list(struct dvr_file_entry *entry,
                            struct list_head *replay_list)
{
    int ret = AK_FAILED;
    struct dvr_file_entry *replay_entry = copy_record_file_entry(entry);
    if (replay_entry) {
        list_add_tail(&(replay_entry->list), replay_list);
        ++record_file.replay_count;
        ret = AK_SUCCESS;
    }

    return ret;
}

static time_t get_dir_calendar_time(const time_t calendar_time)
{
    struct ak_date date;

    ak_seconds_to_date(calendar_time, &date);
    date.hour = 0;
    date.minute = 0;
    date.second = 0;

    return ak_date_to_seconds(&date);
}

/**
 * add_dir_to_list - add appointed entry into list
 * @dir_path[IN]: record file dir path
 * @entry[IN]: record file entry
 * @dir_head[IN]: record file dir list head
 * return: none
 */
static int add_dir_to_list(const char *dir_path,
                            struct dvr_file_entry *entry,
                            struct list_head *dir_head)
{
    struct record_dir_entry *dir_ent = malloc_record_dir_entry();
    if (dir_ent) {
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "entry->path= '%s' entry->calendar_time= %lu\n", entry->path , entry->calendar_time )
        dir_ent->calendar_time = get_dir_calendar_time(entry->calendar_time);
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "entry->path= '%s' entry->calendar_time= %u dir_ent->calendar_time= %u\n", entry->path , entry->calendar_time , dir_ent->calendar_time )
        strcpy(dir_ent->dir_path, dir_path);
        INIT_LIST_HEAD(&(dir_ent->file_head));

        /*
        ak_print_normal_ex("dir_path: %s\n", dir_path);
        ak_print_normal_ex("%s\n", ak_seconds_to_string(dir_ent->calendar_time));
        */
    } else {
        ak_print_error_ex("calloc failed\n");
        return AK_FAILED;
    }

    struct record_dir_entry *ptr = NULL;

    /* add dir entry into the list by order */
    list_for_each_entry_reverse(ptr, dir_head, list){
        if (ptr) {
            if(dir_ent->calendar_time > ptr->calendar_time){
                ak_thread_mutex_lock(&(record_file.entry_mutex));
                //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_ent->dir_path= '%s'\n", dir_ent->dir_path )
                list_add_tail(&(dir_ent->list), &(ptr->list));                  //加到末尾
                ak_thread_mutex_unlock(&(record_file.entry_mutex));
                return AK_SUCCESS;
            }
        }
    }

    /* the earlist entry */
    ak_thread_mutex_lock(&(record_file.entry_mutex));
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_ent->dir_path= '%s'\n", dir_ent->dir_path )
    list_add_tail(&(dir_ent->list), dir_head);                                  //加到末尾
    ak_thread_mutex_unlock(&(record_file.entry_mutex));

    return AK_SUCCESS;
}

/**
 * create_entry_dir - create sub dir for record file entry in record dir
 * @dir_path[IN]: dir path
 * @entry[IN]: current record file entry info
 * @dir_head[OUT]: record file dir list head
 * return: 0 success; -1 failed
 * notes: file name ex.: /mnt/CYC_DV/CYC_DV_20160504-095310.mp4
 *        entry dir: 20160504
 */
static int create_entry_dir(const char *dir_path,
                            struct dvr_file_entry *entry,
                            struct list_head *dir_head)
{
    int ret = AK_FAILED;

    if (ak_check_file_exist(dir_path)) {
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_path= '%s'\n", dir_path )
        ak_print_notice_ex("dir_path: %s is not exist\n", dir_path);
        if (record_fs_create_dir(dir_path)) {
            ak_print_error_ex("%s create failed\n", dir_path);
        } else {
            ret = add_dir_to_list(dir_path, entry, dir_head);
        }
    } else {
        struct record_dir_entry *ptr = NULL;
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_path= '%s'\n", dir_path )
        list_for_each_entry_reverse(ptr, dir_head, list){
            if (ptr && (0 == strcmp(ptr->dir_path, dir_path))) {
                ret = AK_SUCCESS;
                break;
            }
        }
        if (ret) {
            ret = add_dir_to_list(dir_path, entry, dir_head);
        }
    }

    return ret;
}

/**
 * get_entry_total_time: get appointed entry't total time, unit: ms
 * @entry_path[IN]: full path of fetched record file entry
 * return: total time of the appointed record entry
 * notes: get record start time from file name:
 *        CYC_DV_20150908-090511@301000.mp4
 *        total time: 301000(ms)
 */
int get_entry_total_time(const char *entry_path)
{
    int total_time = 0;
    char *str = strrchr(entry_path, '@');
    if(str){
        ++str;
        total_time = atoi(str);
    } else {
        /* get total time via demux */
        total_time = ak_demux_get_total_time(entry_path);
        if (total_time <= 0) {
            ak_print_error_ex("video_demux_get_total_time FAILED\n");
            ak_print_error_ex("file: %s\n", entry_path);
            remove(entry_path);
        }
    }

    return total_time;
}

/**
 * add_entry_into_order_list: add appointed entry into list
 * @entry[IN]: fetched record file entry
 * @file_head[IN]: current ordered file list head
 * return: none
 * notes: file in old list is already in ASC order
 */
static void add_entry_into_order_list(struct dvr_file_entry *entry,
                                    struct list_head *file_head)
{
    struct dvr_file_entry *ptr = NULL;

    /* add entry into the list by order */
    list_for_each_entry_reverse(ptr, file_head, list){
        if(ptr){
            if(entry->calendar_time == ptr->calendar_time){
                /* the same record file */
                free_record_file_entry(entry);
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
    list_add(&(entry->list), file_head);
    ak_thread_mutex_unlock(&(record_file.entry_mutex));
}

static void move_entry_to_dir(struct dvr_file_entry *entry,
                            const char *dir_path)
{
    /* get record entry name from file path, CYC_DV_20150908-090511.mp4 */
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_path= '%s'\n", dir_path )
    char *str = strrchr(entry->path, '/');
    if(NULL != str){
        ++str;
    }

    char new_file[DVR_PATH_LEN] = {0};
    char orginal[DVR_PATH_LEN] = {0};

    char *time_ptr = strrchr(str, '@');
    if(time_ptr){
        snprintf(new_file, DVR_PATH_LEN, "%s/%s", dir_path, str);
    } else {
        strcpy(orginal, entry->path);
        char *ext = strchr(entry->path, '.');
        char *name_ptr = strtok(orginal, ".");
        snprintf(new_file, DVR_PATH_LEN, "%s@%.6ld%s", name_ptr, entry->total_time, ext);
    }
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "entry->path= '%s' new_file= '%s'\n", entry->path, new_file )
    if (strcmp(entry->path, new_file)) {
        if (rename(entry->path, new_file)) {
            ak_print_error_ex("rename: %s\n", strerror(errno));
        } else {
            strncpy(entry->path, new_file, sizeof(entry->path));
            ak_print_notice_ex("copy new file: %s\n", entry->path);
        }
    }
}

static struct list_head* find_dir_list(struct dvr_file_entry *entry,
                        struct list_head *dir_head)
{
    char dir_name[10] = {0};
    char dir_path[255] = {0};
    struct ak_date date;
    struct list_head *file_head = NULL;
    struct record_dir_entry *ptr = NULL;

    ak_seconds_to_date(entry->calendar_time, &date);
    snprintf(dir_name, 10, "%4.4d%2.2d%2.2d", date.year, (date.month + 1), (date.day + 1));
    snprintf(dir_path, 255, "%s%s/%02d", record_file.param.rec_path, dir_name, date.hour);

    create_entry_dir(dir_path, entry, dir_head);
    move_entry_to_dir(entry, dir_path);

    /* add dir into the list by order */
    list_for_each_entry_reverse(ptr, dir_head, list) {
        if (ptr && (0 == strcmp(ptr->dir_path, dir_path))) {
            file_head = &(ptr->file_head);
            break;
        }
    }

    return file_head;
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
    int file_len = strlen(file_name);
    struct dvr_file_entry *entry = malloc_record_file_entry(file_len + REC_TIME_INFO_LEN);

    if (NULL != entry) {
        entry->calendar_time = record_fs_get_time_by_name(file_name);
        entry->size = file_size;
        entry->total_time = get_entry_total_time(file_name);
        strcpy(entry->path, file_name);
        //ak_print_normal_ex("entry->path= '%s'\n", entry->path);
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "entry->path= '%s'\n", entry->path )
        struct list_head *file_head = find_dir_list(entry, &(record_file.head[file_type]));
        if (file_head) {
            add_entry_into_order_list(entry, file_head);
            ++record_file.total_num;
        }
    }
}

/**
 * remove_earliest_file - remove earliest record file
 * @file: current record file info
 * @occupy_size: current record file occupy size in byte
 * return: none
 * notes: if the room is not enough, delete the earliest record file
 */
static void remove_earliest_file(struct record_file *file, unsigned long occupy_size)     //删除最早的文件
{

    struct record_dir_entry *p_record_dir_loop = NULL, *p_record_dir_remove = NULL;
    struct dvr_file_entry *p_dvr_file_loop = NULL, *p_dvr_file_remove = NULL ;
    time_t i_min_day = 0 , i_min_file = 0;

    ak_thread_mutex_lock(&(file->entry_mutex));
    for( ;; ) {
        list_for_each_entry( p_record_dir_loop, &(record_file.head[ file->param.type ] ), list ) { //遍历找到时间戳最小的目录
            if ( ( i_min_day == 0 ) || ( i_min_day > p_record_dir_loop->calendar_time ) ) {
                /*
                DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE ,
                             "p_record_dir_loop->dir_path= '%s' p_record_dir_loop->calendar_time= %lu\n",
                             p_record_dir_loop->dir_path , p_record_dir_loop->calendar_time);
                */
                i_min_day = p_record_dir_loop->calendar_time;
                p_record_dir_remove = p_record_dir_loop;                        //保存找到的目录指针
            }
        }

        if ( ( i_min_day != 0 ) && ( p_record_dir_remove != NULL ) ){
            list_for_each_entry( p_dvr_file_loop, &(p_record_dir_remove->file_head), list) {        //进入目录指针遍历找到时间戳最小的文件
                if ( ( i_min_file == 0 ) || ( i_min_file > p_dvr_file_loop->calendar_time ) ){      //找到时间戳最小的文件
                    i_min_file = p_dvr_file_loop->calendar_time;                //设置时间戳最小的文件标志
                    p_dvr_file_remove = p_dvr_file_loop;                        //保存文件指针
                }
            }
        }
        if ( ( p_record_dir_remove != NULL ) && ( p_dvr_file_remove != NULL ) ) {         //判断是否找到目录和文件
            /*
            DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE ,
                         "p_record_dir_remove->dir_path= '%s' p_record_dir_remove->path = '%s' p_dvr_file_remove->calendar_time= %lu\n",
                         p_record_dir_remove->dir_path, p_dvr_file_remove->path, p_dvr_file_remove->calendar_time);
            */
            remove( p_dvr_file_remove->path ) ;                                 //删除文件
            ak_thread_mutex_lock( &( file->size_mutex ) ) ;
            file->used_size -= p_dvr_file_remove->size ;                        //全局文件容量减少
            file->free_size += p_dvr_file_remove->size ;                        //全局空闲容量增加
            ak_thread_mutex_unlock( &(file->size_mutex ) ) ;
            list_del_init( &( p_dvr_file_remove->list ) ) ;                     //从列表删除文件
            free_record_file_entry( p_dvr_file_remove ) ;                       //释放指针
            delete_empty_dir( p_record_dir_remove ) ;                           //判断是否空目录,如果是则删除空目录
            ak_print_normal("REMOVE:'%s' after remove, used_size:%lu(KB), free_size=%lu(KB) occupy_size= %lu(KB)\n", p_dvr_file_remove->path, file->used_size, file->free_size, occupy_size);
            p_record_dir_remove = NULL;
            p_dvr_file_remove = NULL;
            i_min_day = 0;
            i_min_file = 0;
            if (file->free_size > occupy_size) {                                //判断是否满足空间要求
                //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE , "file->free_size =%lu(KB) occupy_size= %lu(KB)\n", file->free_size, occupy_size);
                break;
            }
        }
        else {
            //DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE )
            break;
        }
    }
    ak_thread_mutex_unlock(&(file->entry_mutex));

    /*
    unsigned char del_flag = AK_FALSE;
    struct record_dir_entry *dir_ptr = NULL;
    struct record_dir_entry *dir_entry = NULL;
    struct dvr_file_entry *ptr = NULL;
    struct dvr_file_entry *entry = NULL;
    enum dvr_file_type type = file->param.type;

    ak_thread_mutex_lock(&(file->entry_mutex));
    list_for_each_entry_safe(dir_entry, dir_ptr, &(file->head[type]), list) {
        list_for_each_entry_safe(entry, ptr, &(dir_entry->file_head), list) {
            ak_print_normal("used_size:%ld(KB), free_size=%ld(KB)\n", file->used_size, file->free_size);
            ak_print_normal("room isn't enough, remove: %s\n", entry->path);
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "entry->path= '%s'\n", entry->path )

            // room isn't enough, remove file and update used and free size
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
                del_flag = AK_TRUE;
                break;
            }
        }

        delete_empty_dir(dir_entry);
        dir_entry = NULL;
        if (del_flag) {
            break;
        }
    }
    ak_thread_mutex_unlock(&(file->entry_mutex));
    */
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
    static char i = 0;

    file->total_num = 0;
    record_file.status = DVR_FILE_STATUS_FETCHING;
    ak_print_notice_ex("used_size=%ld, dir: %s\n",
        file->used_size, file->param.rec_path);

    /* get history record files, and calculate getting used time */
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
    dump_list_dir();
#endif

    remove_del_file(file);
    ak_print_notice_ex("the disk free size=%lu(KB), record size=%lu(KB)\n",
        file->free_size, file->used_size);
    ak_print_notice_ex("saved use_time=%ld(ms), record total_num=%d\n\n",
        use_time, file->total_num);
    record_file.status = DVR_FILE_STATUS_FETCH_OK;

    unsigned char size_enough = AK_TRUE;
    if (file->param.cyc_flag) {
        /* free size isn't enough */
        if (file->free_size < reserved_size) {
            ak_print_normal_ex("the disk free size isn't enough!"
                "(%lu,%lu,%lu)\n",
                file->free_size, file->used_size, reserved_size);
            /* available size is enough, and we'll remove earliest files */
            if ((file->free_size + file->used_size) > reserved_size) {
                ak_print_normal_ex("we'll remove earliest record files\n");
                for(i=0; i<5; i++){
                    remove_earliest_file(file, reserved_size);
                    if (file->free_size > reserved_size) {                          //删除完之后再进行判断容量是否足够
                        break;
                    }
                }
                if(5 == i){
                    i = 0;
                    size_enough = AK_FALSE;
                }
            } else {
                /* available size isn't enough either */
                ak_print_normal_ex("available size isn't enough either\n");
                size_enough = AK_FALSE;
            }
        }
    } else {
        if (file->free_size < reserved_size) {
            ak_print_normal_ex("the disk free size isn't enough!(%lu,%lu)\n", file->free_size, reserved_size);
            size_enough = AK_FALSE;
        }
    }

    if (size_enough){
        ak_thread_mutex_lock(&(file->size_mutex));
        /* reserved appointed disk size */
        file->free_size -= reserved_size;                                       //空闲区间-保留空间,空闲区间的定义free_size == 0 则判断空间已满
        ak_thread_mutex_unlock(&(file->size_mutex));
    } else {
        report_exception_info(file, DVR_EXCEPT_SD_NO_SPACE);
    }
}

/**
 * get_pre_saved_record - get history record and save in list.
 * @arg: thread input arg, current record type record file info.
 * return: none
 * notes:
 */
static void* get_pre_saved_record(void *arg)
{
    ak_print_normal_ex("--- thread id: %ld ---\n", ak_thread_get_tid());
    ak_thread_set_name("get_dvr_file");

    struct record_file *file = (struct record_file *)arg;
    while (file->run_flag) {
        ak_thread_sem_wait(&(file->pre_record_sem));
        if (file->run_flag) {
            ak_print_notice_ex("--- start get record file in SD card ---\n");
            get_sdcard_record(file);
            if (record_file.full_flag) {
                ak_thread_sem_post(&(file->finish_sem));
            }
        }
    }

    ak_print_normal_ex("### %ld thread exit ###\n\n", ak_thread_get_tid());

    ak_thread_exit();
    return NULL;
}

static void add_replay_to_order_list(struct dvr_file_entry *entry,
                                    struct list_head *file_head)
{
    struct dvr_file_entry *ptr = NULL;

    /* add entry into the list by order */
    list_for_each_entry_reverse(ptr, file_head, list){
        if (ptr) {
            if(entry->calendar_time == ptr->calendar_time){
                /* the same record file */
                free_record_file_entry(entry);
                return;
            }

            if(entry->calendar_time > ptr->calendar_time){
                list_add(&(entry->list), &(ptr->list));
                return;
            }
        }
    }

    /* the earlist entry */
    list_add(&(entry->list), file_head);
}

static int add_replay_to_list(const char *file_name,
                            int file_size,
                            time_t calendar_time,
                            struct search_replay_param *replay)
{

    int file_len = strlen(file_name);
    int ret = AK_SUCCESS;

    /* ak_print_normal_ex("start add %s \n",file_name); */
    struct dvr_file_entry *entry = malloc_record_file_entry(file_len
        + REC_TIME_INFO_LEN);
    if (entry) {
        entry->calendar_time = calendar_time;
        entry->size = file_size;
        entry->total_time = get_entry_total_time(file_name);
        strcpy(entry->path, file_name);

        add_replay_to_order_list(entry, replay->entry_list);
    } else {
        ak_print_error_ex("add %s failed\n",file_name);
        ret = AK_FAILED;
    }

    return ret;
}

/* destroy record file list */
static void delete_saved_file_entry(struct record_file *file)
{
    int i = 0;
    int entry_num = 0;
    struct dvr_file_entry *ptr = NULL;
    struct dvr_file_entry *entry = NULL;
    struct record_dir_entry *dir_ptr = NULL;
    struct record_dir_entry *dir_entry = NULL;

    ak_thread_mutex_lock(&(file->entry_mutex));
    for(i=0; i<DVR_FILE_TYPE_NUM; ++i) {
        list_for_each_entry_safe(dir_entry, dir_ptr, &(file->head[i]), list) {
            entry_num = 0;
            list_for_each_entry_safe(entry, ptr, &(dir_entry->file_head), list) {
                ++entry_num;
                list_del_init(&(entry->list));
                free_record_file_entry(entry);
            }

            list_del_init(&(dir_entry->list));
            free_record_dir_entry(dir_entry);
            ak_print_info_ex("record file type=%d, delete entry_um=%d\n",
                i, entry_num);
        }
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

/**
 * sync_record_file - sync current record file and close it.
 * @file: current record file info
 * return: void
 */
static void sync_record_file(struct record_file *file)
{
    struct ak_timeval tv_start = {0};
    struct ak_timeval tv_end = {0};
    int index = file->save_index;

    /* use fsync to sync current record file */
    ak_get_ostime(&tv_start);
    if (fsync(fileno(file->fp[index].record))) {
        ak_print_error_ex("fsync errno=%d, desc: %s\n", errno, strerror(errno));
    }
    ak_get_ostime(&tv_end);
    ak_print_normal_ex("fsync use time: %ld(ms)\n",
        ak_diff_ms_time(&tv_end, &tv_start));

    if (fclose(file->fp[index].record)) {
        ak_print_error_ex("fsync errno=%d, desc: %s\n", errno, strerror(errno));
    }
    file->fp[index].record = NULL;
}

/**
 * generate_record_file - generate a new record file. Move the temp record file to new record file.
 * @file: current record file info
 * @index: temp record file name array index
 * @occupy_size: record file size
 * return: void
 */
static void generate_record_file(struct record_file *file,
                                int index, unsigned long occupy_size)
{
    /** update the current file message into the record queue **/
    update_used_size(occupy_size);
    add_file_to_list(file->file_name[index], occupy_size, file->param.type);

    /* rename the record file */
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "file->file_name[index]= '%s'\n", file->file_name[index] )
    rename(file->tmp_file[index], file->file_name[index]);

    ak_print_normal_ex("record file index=%d, name:\n", index);
    ak_print_normal("\t%s\n", file->file_name[index]);
    ak_print_normal_ex("record file index=%d, used_size=%ld KB\n",
        index, file->used_size);
    ak_print_normal("------------------------------------------------------"
        "--------------------------\n\n");
}

static void stat_record_file(struct record_file *file)
{
    struct stat statbuf;
    int index = file->save_index;
    static char i = 0;

    /** get record file information **/
    stat(file->tmp_file[index], &statbuf);
    unsigned long occupy_size = (statbuf.st_blocks >> 1);                       //获取当前写入的录像文件的KB大小,所需扇区数*2,则变成KB数
    ak_print_normal_ex("record file index=%d, st_size=%ld(Bytes), occupy_size=%ld KB\n", index, statbuf.st_size, occupy_size);

    /** file size less than 100k, delete it **/
    if(statbuf.st_size < (100 * 1024)){
        remove(file->tmp_file[index]);
    }else{
        unsigned char generate_flag = AK_TRUE;

        if ( ( file->free_size == 0 ) &&
             ( file->used_size == 0 ) &&
             ( reserved_size == MIN_DISK_SIZE_FOR_RECORD ) ) {                  //在dvr_record_demo中可以保存文件,由于在dvr_record_demo中free_size没初始化,值为0,因此在此进行free_size赋值
            reserved_size = record_fs_get_disk_total_size(file->param.rec_path) * RESERVED_PERCENT / 100 ;
            file->free_size = record_fs_get_disk_free_size(file->param.rec_path) - reserved_size;
        }
        /*
        DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE,
                     "occupy_size= %luKB file->free_size= %lu file->param.cyc_flag= %d\n" ,
                     occupy_size , file->free_size, file->param.cyc_flag );
        */
        if(occupy_size > file->free_size){                          //当前录像文件大小存入后 大于 T卡剩余容量阈值
            if(file->param.cyc_flag){                               //读取配置文件中的参数，使能了循环录像模式
                for(i=0; i<5; i++){                                 //若删除较早录像文件，内存仍然不够，继续删除，循环删除次数最多3次
                    remove_earliest_file(file, occupy_size);        //删除较早的录像文件
                    if(occupy_size < file->free_size){              //删除后已经有空间了
                        break;
                    }
                }
                if(5 == i){
                    i = 0;
                    generate_flag = AK_FALSE;
                }
            }
            else{
                report_exception_info(file, DVR_EXCEPT_SD_NO_SPACE);
                generate_flag = AK_FALSE;
            }
        }

        /* generate record file if free size is enough */
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
    ak_print_normal_ex("current save file[%d]:\n", index);
    ak_print_normal("\t%s\n", file->file_name[index]);
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

    /* close name temp file */
    if (record_file.name_tmp){
        fclose(record_file.name_tmp);
        record_file.name_tmp = NULL;
        remove(record_file.param.rec_info_file);
    }

    file->fp[index].index = NULL;
    file->fp[index].record = NULL;
    if (file->flush_cb) {
        file->flush_cb(AK_TRUE);
    }
    ak_thread_mutex_unlock(&(file->fp_mutex));
}

static void create_record_dir(void)
{
    /* check and create temp directory: /mnt/tmp/ */
    if (ak_check_file_exist(record_file.param.rec_tmppath)) {
        ak_print_warning_ex("%s not exist, we'll create it\n",
            record_file.param.rec_tmppath);
        if (record_fs_create_dir(record_file.param.rec_tmppath)) {
            ak_print_error_ex("%s create failed\n", record_file.param.rec_tmppath);
        }
    } else {
        ak_print_normal_ex("%s already exists\n", record_file.param.rec_tmppath);
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

static int create_name_tmp_file(const struct ak_date *date)
{
    int ret = AK_FAILED, len_now, len_write;
    unsigned long default_duration = 0;
    char record_info[LEN_RECORD_INFO] = {0};

    if (record_file.name_tmp == NULL){                                                              //lock at record_file_open()
        record_file.name_tmp = fopen(record_file.param.rec_info_file, "w+");
    }

    if (record_file.name_tmp != NULL) {
        record_file.name_len = snprintf(record_info, LEN_RECORD_INFO, "%4.4d%2.2d%2.2d-%2.2d%2.2d%2.2d\n",
                                        date->year, (date->month + 1), (date->day + 1),
                                        date->hour, date->minute, date->second);
        len_now= snprintf(record_info, LEN_RECORD_INFO, "%4.4d%2.2d%2.2d-%2.2d%2.2d%2.2d\n%ld\n",
                        date->year, (date->month + 1), (date->day + 1),
                        date->hour, date->minute, date->second, default_duration);
        len_write= fwrite(record_info, 1, len_now, record_file.name_tmp);
        ak_print_normal_ex("len_write= %d\n", len_write);

        ret = AK_SUCCESS;
    } else {
        ak_print_normal_ex("%s\n", strerror(errno));
        /* read-only file system */
        if(EROFS == errno) {
            report_exception_info(&record_file, DVR_EXCEPT_SD_RO);
        }
    }

    return ret;
}

//开机T卡挂载后，检测/mnt/tmp目录下是否有未满5分钟的视频缓存，若存在，将其生成(AVI或MP4格式)视频保存至/mnt/CYC_DV)
static void check_poweroff_file(void)
{
    int index = 0;
    static struct record_file record_file_buf = {0};                //record_file结构体缓存
    memcpy(&record_file_buf, &record_file, sizeof(record_file));    //保存当前record_file结构体(数据)
    
    for(index=0; index<RECORD_NODE_NUM; ++index){
        if (!ak_check_file_exist(record_file.tmp_file[index])) {
            break;
        }
    }

    /* no poweroff record file remained */
    if (index >= RECORD_NODE_NUM) {
        return;
    }

    unsigned long duration = 0;
    struct ak_date tmp_date = {0};

    if (!ak_check_file_exist(record_file.param.rec_info_file)) {
        ak_print_normal_ex("name file: %s exist\n", record_file.param.rec_info_file);
        record_file.name_tmp = fopen(record_file.param.rec_info_file, "r");
        if (record_file.name_tmp) {
            char line_data[255] = {0};

            fgets(line_data, sizeof(line_data), record_file.name_tmp);
            if (strlen(line_data) > 0) {
                ak_string_to_date(line_data, &tmp_date);
            }

            memset(line_data, 0x00, sizeof(line_data));
            fgets(line_data, sizeof(line_data), record_file.name_tmp);
            if (strlen(line_data) > 0) {
                duration = atoi(line_data);
            }

            memcpy(&record_file.save_date, &tmp_date, sizeof(tmp_date));
            record_file_generate_name(index, duration);
            
            record_file.fp[index].record = fopen(record_file.param.rec_tmp_file[index], "rb+");
            record_file.fp[index].moov = fopen(record_file.param.rec_tmp_moov[index],"r");
            record_file.fp[index].stbl = fopen(record_file.param.rec_tmp_stbl[index],"r");

            ak_mux_fileperfect(record_file.fp[index].record,record_file.fp[index].moov,record_file.fp[index].stbl);
           
            fclose(record_file.fp[index].moov);
            fclose(record_file.fp[index].stbl);
            fclose(record_file.name_tmp);
            
            record_file.name_tmp = NULL;

            if (ak_mux_fix_file(record_file.fp[index].record)) {
                ak_print_error_ex("open poweroff file: %s failed\n",
                    record_file.param.rec_tmp_file[index]);
            } else {
                remove(record_file.param.rec_tmp_idx[index]);
                record_file.save_index = index;
                flush_record_file(&record_file);
            }
            if(record_file.fp[index].record != NULL) {
                fclose(record_file.fp[index].record);
                record_file.fp[index].record= NULL;
            }
            
            //因为执行以上操作，会引起设备回放异常，在设备正常生成视频后，恢复原有record_file结构体(数据)
            memcpy(&record_file, &record_file_buf, sizeof(record_file_buf)); 
            memset(&record_file_buf, 0x00, sizeof(record_file_buf));
            
        }
    }
}

/*
static struct dvr_file_entry* get_first_replay_entry(time_t start_time,
                                time_t end_time,
                                struct list_head *file_head,
                                struct list_head *entry_list)
{
    time_t calendar_end_time = 0;
    struct dvr_file_entry *first_entry = NULL;
    struct dvr_file_entry *ptr = NULL;

    list_for_each_entry(ptr, file_head, list) {
        calendar_end_time = (ptr->calendar_time + (ptr->total_time / 1000));
        // appointed time is in a record file
        if( ((start_time <= ptr->calendar_time) && (end_time > ptr->calendar_time)) ||
            ((start_time < calendar_end_time) && (end_time >= calendar_end_time)) ||
            ((start_time >= ptr->calendar_time) && (end_time <= calendar_end_time))) {
#if DVR_REPLAY_LIST_DEBUG
            ak_print_normal("start time inner file: %s\n", ptr->path);
#endif
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "ptr->path= '%s'\n", ptr->path )
            if (!add_to_replay_list(ptr, entry_list)) {
                first_entry = ptr;
                break;
            }
        }
    }

    return first_entry;
}
*/

/*
static int get_first_replay_ptr(time_t start_time,
                            time_t end_time,
                            enum dvr_file_type type,
                            struct list_head *entry_list,
                            struct record_dir_entry **dir,
                            struct dvr_file_entry **ptr)
{
    int ret = AK_FAILED;
    struct record_dir_entry *dir_entry = NULL;
    struct dvr_file_entry *file_entry = NULL;

    list_for_each_entry(dir_entry, &(record_file.head[type]), list) {
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_entry->dir_path= '%s'\n", dir_entry->dir_path )
#if DVR_REPLAY_LIST_DEBUG
        ak_print_normal_ex("calendar_time=%ld, str: %s\n",
            dir_entry->calendar_time,
            ak_seconds_to_string(dir_entry->calendar_time));
        ak_print_normal_ex("start_time=%ld, str: %s\n",
            start_time, ak_seconds_to_string(start_time));
        ak_print_normal_ex("end_time=%ld, str: %s\n\n",
            end_time, ak_seconds_to_string(end_time));
#endif

        // the earliest dir time is also larger than end_time
        if(dir_entry->calendar_time >= end_time) {
            break;
        }

        if( ((start_time >= dir_entry->calendar_time) && (start_time < (dir_entry->calendar_time + ONE_DAY_SECOND))) ||
            ((dir_entry->calendar_time >= start_time) && (dir_entry->calendar_time <= end_time))) {
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_entry->dir_path= '%s' dir_entry->calendar_time= %lu\n", dir_entry->dir_path, dir_entry->calendar_time )
            file_entry = get_first_replay_entry(start_time, end_time, &(dir_entry->file_head), entry_list);
            if (file_entry) {
#if DVR_REPLAY_LIST_DEBUG
                ak_print_notice_ex("get first entry OK\n");
#endif
                *dir = dir_entry;
                *ptr = file_entry;
                ret = AK_SUCCESS;
                break;
            }
        }
    }
    return ret;
}
*/

/*
static int get_replay_entry_continue(time_t end_time,
                struct dvr_file_entry *ptr,
                struct list_head *file_head,
                struct list_head *entry_list)
{
    int ret = AK_FAILED;
    struct dvr_file_entry *entry = ptr;

    list_for_each_entry_continue(entry, file_head, list) {
        // the latest file time doesn't match
        if (entry->calendar_time >= end_time) {
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED, "entry->path= '%s'\n", entry->path )
            ret = AK_SUCCESS;
            break;
        }
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "entry->path= '%s'", entry->path )
        add_to_replay_list(entry, entry_list);
    }

    return ret;
}
*/

/*
static int get_replay_entry_last(time_t end_time,
                enum dvr_file_type type,
                struct record_dir_entry *dir,
                struct list_head *entry_list)
{
    int ret = AK_FAILED;
    struct dvr_file_entry *ptr = NULL;

    list_for_each_entry_continue(dir, &(record_file.head[type]), list) {
        list_for_each_entry(ptr, &(dir->file_head), list) {
            // the latest file time doesn't match
            if (ptr->calendar_time >= end_time) {
                //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED, "ptr->path= '%s'\n", ptr->path )
                ret = AK_SUCCESS;
                break;
            }
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "ptr->path= '%s'\n", ptr->path )
            add_to_replay_list(ptr, entry_list);
        }

        if (!ret) {
            break;
        }
    }

    return ret;
}
*/

/*
用来检查2个时间轴是否有重合
*/
int check_time_axis( time_t i_request_start , time_t i_request_end , time_t i_mark_start , time_t i_mark_end )
{
    if ( ( i_request_end - i_request_start <= 0 ) || ( i_mark_end - i_mark_start <= 0 ) ) {
        return AK_FALSE;
    }
    if ( ( ( i_mark_start >= i_request_start ) && ( i_mark_start < i_request_end ) ) ||
         ( ( i_mark_end > i_request_start ) && ( i_mark_end <= i_request_end ) ) ||
         ( ( i_request_start >= i_mark_start ) && ( i_request_start < i_mark_end ) ) ||
         ( ( i_request_end > i_mark_start ) && ( i_request_end <= i_mark_end ) ) ) {
        return AK_TRUE;
    }
    else {
        return AK_FALSE;
    }
}

static int get_replay_from_list(time_t start_time,
                                time_t end_time,
                                enum dvr_file_type type,
                                struct list_head *entry_list)
{
    struct record_dir_entry *p_record_dir = NULL;
    struct dvr_file_entry *p_dvr_file = NULL;
    time_t i_dir_start = 0 , i_dir_end = 0 ;
    time_t i_file_start = 0 , i_file_end = 0 ;

    record_file.replay_count = 0;
    
    start_time = start_time - start_time % ONE_HOUR_SECOND - ONE_DAY_SECOND;//开始搜索范围扩大24小时
    end_time = end_time - end_time % ONE_HOUR_SECOND + ONE_HOUR_SECOND ;    //结束搜索范围扩大1小时

    list_for_each_entry( p_record_dir, &(record_file.head[ type ] ), list ) {   //遍历目录
        i_dir_start = p_record_dir->calendar_time;
        i_dir_end = p_record_dir->calendar_time + ONE_DAY_SECOND;
        
        /*
        DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE ,
                     "start_time= %lu end_time= %lu p_record_dir->dir_path= '%s' p_record_dir->calendar_time= %lu last= %lu\n",
                     start_time, end_time , p_record_dir->dir_path , p_record_dir->calendar_time , p_record_dir->calendar_time + ONE_DAY_SECOND );
                     */
        if( check_time_axis( i_dir_start , i_dir_end , start_time, end_time ) == AK_TRUE ) {       //判断目录开始时间+1小时 结束时间-1小时
            /*
            DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN ,
                         "start_time= %lu end_time= %lu p_record_dir->dir_path= '%s' p_record_dir->calendar_time= %lu last= %lu\n",
                         start_time, end_time , p_record_dir->dir_path , p_record_dir->calendar_time , p_record_dir->calendar_time + ONE_DAY_SECOND );
                         */
            list_for_each_entry(p_dvr_file, &(p_record_dir->file_head), list) {           //遍历文件
                i_file_start = p_dvr_file->calendar_time;
                i_file_end = p_dvr_file->calendar_time  + p_dvr_file->total_time / 1000;
                /*
                DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE ,
                             "p_dvr_file->path = '%s' i_file_start= %lu i_file_end= %lu start_time= %lu end_time= %lu\n",
                             p_dvr_file->path, i_file_start, i_file_end, start_time, end_time);
                             */
                if( check_time_axis( i_file_start , i_file_end , start_time , end_time ) == AK_TRUE ) {       //判断文件
                    /*
                    DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN ,
                                 "p_dvr_file->path = '%s' i_file_start= %lu i_file_end= %lu start_time= %lu end_time= %lu\n",
                                 p_dvr_file->path, i_file_start, i_file_end, start_time, end_time);
                                 */
                    add_to_replay_list( p_dvr_file , entry_list);
                }
            }
        }
    }
    return record_file.replay_count;

    /*
    struct record_dir_entry *dir = NULL;
    struct dvr_file_entry *ptr = NULL;

    record_file.replay_count = 0;

    // 1. find first matched replay record entry
    if (!get_first_replay_ptr(start_time, end_time, type, entry_list, &dir, &ptr)) {
        //2. find replay record entry in the same dir file list
        if (get_replay_entry_continue(end_time, ptr, &(dir->file_head), entry_list)) {
            //3. find replay record entry in the other dir file list, if not finished
            get_replay_entry_last(end_time, type, dir, entry_list);
        }
    }

    return record_file.replay_count;
    */
}

static int get_replay_from_dir(time_t start_time,
                        time_t end_time,
                        enum dvr_file_type type,
                        struct list_head *entry_list)
{
    struct search_replay_param replay = {0};

    //DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE )
    ak_print_info_ex("enter...\n ");
    replay.start_time = start_time;
    replay.end_time = end_time;
    replay.type = type;
    replay.entry_list = entry_list;

    return record_fs_search_by_time(&record_file.search, &replay, add_replay_to_list);
}

/**
 * record_file_main - record file main thread
 * @arg: thread input arg, current record type record file info
 * return: none
 * notes:
 */
static void* record_file_main(void *arg)
{
    ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
    ak_thread_set_name("flush_dvr_file");

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
 * record_file_close - close record file.
 * @file_index:  index of close file
 * return: none
 * notes: 1. record file index, switch between 0 and 1
 *           2. when unplug card, should call it
 */
void record_file_close(int file_index)
{
    int index = file_index;
    struct record_file *file = &record_file;

    file->save_index = file_index;
    ak_thread_mutex_lock(&(file->fp_mutex));
    ak_print_normal_ex("current close file[%d]:\n", index);
    ak_print_normal("\t%s\n", file->file_name[index]);
    ak_print_normal_ex("fp[%d].index=0x%lX, fp[%d].record=0x%lX\n",
        index, (unsigned long)(file->fp[index].index),
        index, (unsigned long)(file->fp[index].record));

    /* close last index file */
    if(file->fp[index].index){
        fclose(file->fp[index].index);
        file->fp[index].index = NULL;
        remove(file->tmp_index[index]);
    }

    /* close last record file */
    if(file->fp[index].record){
        fclose(file->fp[index].record);
        file->fp[index].record = NULL;
    }

    if(file->fp[index].moov){
        fclose(file->fp[index].moov);
        file->fp[index].moov = NULL;
    }

    if(file->fp[index].stbl){
        fclose(file->fp[index].stbl);
        file->fp[index].stbl = NULL;
    }

    /* close name temp file */
    if (record_file.name_tmp){
        fclose(record_file.name_tmp);
        record_file.name_tmp = NULL;
    }
    ak_thread_mutex_unlock(&(file->fp_mutex));
    /* ak_cmd_exec("ls /proc/`pidof anyka_ipc`/fd -l", NULL, 0); */
    /* system("ls /proc/`pidof anyka_ipc`/fd -l"); */
}

/**
 * record_file_open - open record file.
 * @index[IN]: appointed record file index, switch between 0 and 1
 * @date[IN]: record file date time
 * @fp[OUT]: fp info after opened successfully, if you want to fetch
 * return: 0 success, otherwise -1
 * notes: 1. fopen default is fully buffered, and the buffered size is
 *        st_blksize in struct stat
 *        2. we use this size: /mnt/tmp/ is vfat fs
 *            st_blksize=32768, st_blocks=64
 */
int record_file_open(int index, const struct ak_date *date,
                struct record_file_fp *fp)
{
    if((index < 0) || (index >= RECORD_NODE_NUM)){
        return AK_FAILED;
    }

    int ret = AK_FAILED;

    ak_thread_mutex_lock(&(record_file.fp_mutex));
    record_file.fp[index].record = fopen(record_file.tmp_file[index], "wb+");
    if (NULL == record_file.fp[index].record) {
        ak_print_normal_ex("open file: %s failed\n", record_file.tmp_file[index]);
        goto file_open_end;
    }

#if 0
    /* set appointed buffer for fwrite and fread */
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
        ak_print_normal_ex("open file: %s failed\n", record_file.tmp_index[index]);
        goto file_open_end;
    }
    
    if(record_file.fp[index].moov != NULL)
    {
        fclose(record_file.fp[index].moov);
        record_file.fp[index].moov = NULL;
    }   
    if(record_file.fp[index].stbl != NULL)
    {
        fclose(record_file.fp[index].stbl);
        record_file.fp[index].stbl = NULL;
    }  
    record_file.fp[index].moov = fopen(record_file.tmp_moov[index],"wb+");
    record_file.fp[index].stbl = fopen(record_file.tmp_stbl[index],"wb+");
    
    if(NULL != fp){
        fp->index = record_file.fp[index].index;
        fp->record = record_file.fp[index].record;
        fp->moov = record_file.fp[index].moov;
        fp->stbl = record_file.fp[index].stbl;
    }
    if (record_file.flush_cb) {
        record_file.flush_cb(AK_FALSE);
    }

    memcpy(&record_file.save_date, date, sizeof(record_file.save_date));
    ret = create_name_tmp_file(date);

file_open_end:
    if (AK_FAILED == ret) {
        ak_print_normal_ex("errno=%d, desc: %s\n", errno, strerror(errno));
        /* read-only file system */
        if(EROFS == errno) {
            report_exception_info(&record_file, DVR_EXCEPT_SD_RO);
        }
        if(record_file.fp[index].index){
            fclose(record_file.fp[index].index);
            record_file.fp[index].index = NULL;
            remove(record_file.tmp_index[index]);
        }
        if(record_file.fp[index].record){
            fclose(record_file.fp[index].record);
            record_file.fp[index].record = NULL;
            remove(record_file.tmp_file[index]);
        }
        if(record_file.fp[index].moov){
            fclose(record_file.fp[index].moov);
            record_file.fp[index].moov = NULL;
            remove(record_file.tmp_moov[index]);
        }
        if(record_file.fp[index].stbl){
            fclose(record_file.fp[index].stbl);
            record_file.fp[index].stbl = NULL;
            remove(record_file.tmp_stbl[index]);
        }
    }
    ak_thread_mutex_unlock(&(record_file.fp_mutex));

    return ret;
}

/**
 * record_file_update_duration - update temp record file duration info.
 * @duration[IN]: record file date time
 * return: 0 success, otherwise -1
 * notes: you can update record duration at certain time,
 *      in case of poweroff when recording.
 */
int record_file_update_duration(unsigned long duration)
{
    int ret = AK_FAILED;

    ak_thread_mutex_lock(&(record_file.fp_mutex));
    if (record_file.name_tmp) {
        char tmp_str[20] = {0};

        sprintf(tmp_str, "%ld\n", duration);
        fseek(record_file.name_tmp, record_file.name_len, SEEK_SET);
        fwrite(tmp_str, 1, strlen(tmp_str), record_file.name_tmp);

        ret = AK_SUCCESS;
    }
    ak_thread_mutex_unlock(&(record_file.fp_mutex));

    return ret;
}

/**
 * record_file_generate_name - generate record file name.
 * @save_index[IN]: appointed record file index, switch between 0 and 1
 * @rec_duration[IN]: record file duration
 * return: 0 success, otherwise -1
 * notes: 1. record saving path and prefix are appointed at init time.
 *        2. ext will be decided by record type
 */
int record_file_generate_name(int save_index, unsigned long rec_duration)
{
    if(save_index >= RECORD_NODE_NUM){
        return AK_FAILED;
    }

    /* get plan record file name */
    char file_ext[FILE_EXT_NAME_MAX_LEN+1] = {0};
    switch(record_file.param.type){
    case DVR_FILE_TYPE_MP4:
        memcpy(file_ext, ".mp4", FILE_EXT_NAME_MAX_LEN);
        break;
    case DVR_FILE_TYPE_AVI:
        memcpy(file_ext, ".avi", FILE_EXT_NAME_MAX_LEN);
        break;
    default:
        ak_print_normal_ex("record_file.param.type error!\n");
        return AK_FAILED;
    }

    /* generate record file name according to requirement param */
    int ret = record_fs_get_video_name( record_file.param.rec_path,
                                        record_file.param.rec_prefix, file_ext, &record_file.save_date,
                                        rec_duration, record_file.file_name[save_index]);
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "record_file.file_name[save_index]= '%s'\n", record_file.file_name[save_index] )
    if (!ret) {
        ak_print_normal_ex("save_index=%d, file:\n", save_index);
        ak_print_normal("\t%s\n\n", record_file.file_name[save_index]);
    }

    return ret;
}

/**
 * record_file_delete - delete record file according to index
 * @index: appointed record file index
 * return: none
 * notes: temp file will be removed.
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
 * record_file_set_callback - set callback functin for record
 * @except_report: callback of exception report
 * @flush_cb: callback of flush finished
 * return: 0
 */
int record_file_set_callback( FP_DVR_FILE_EXCEPT except_report,
    FP_DVR_FILE_FLUSH flush_cb)
{
    record_file.except_report = except_report;
    record_file.flush_cb = flush_cb;
    return AK_SUCCESS;
}

/**
 * record_file_wakeup - wakeup record file thread
 * @save_index[IN]: index of currently saving file
 * return: none
 * notes: flush, sync, rename and save record file after wakeup.
 */
void record_file_wakeup(int save_index)
{
    record_file.save_index = save_index;
    ak_thread_sem_post(&(record_file.manage_sem));
}

/**
 * record_file_get_list - get all record file entry list according to
 *        start time and end time.
 * @start_time[IN]: appointed start time, unit: second
 * @end_time[IN]: appointed end time, unit: second
 * @type[IN]: record file type
 * @entry_list[OUT]: record file entry list
 * return: >=0 list count; otherwise -1
 * notes: 1. we'll return record file entry list after get successfully
 *        2. you must free entry path memory outside.
 *        3. you must call INIT_LIST_HEAD to init entry_list
 */
int record_file_get_list(time_t start_time,
                        time_t end_time,
                        enum dvr_file_type type,
                        struct list_head *entry_list)
{
    if (!entry_list) {
        set_error_no(ERROR_TYPE_INVALID_ARG);
        return AK_FAILED;
    }

    int list_count = 0;
    struct ak_timeval tv_start = {0};
    struct ak_timeval tv_end = {0};

    ak_get_ostime(&tv_start);

    /*
    struct tm tm_res;
    localtime_r( &start_time , &tm_res ) ;
    DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE ,
                         "start_time= %lu '%04d-%02d-%02d %02d:%02d:%02d'\n",
                         start_time, tm_res.tm_year + 1900 , tm_res.tm_mon + 1 , tm_res.tm_mday ,
                         tm_res.tm_hour, tm_res.tm_min , tm_res.tm_sec )
    localtime_r( &end_time , &tm_res ) ;
    DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE ,
                         "end_time= %lu '%04d-%02d-%02d %02d:%02d:%02d'\n",
                         end_time, tm_res.tm_year + 1900 , tm_res.tm_mon + 1 , tm_res.tm_mday ,
                         tm_res.tm_hour, tm_res.tm_min , tm_res.tm_sec )
                         */

//#define SCAN_RAM
//#define SCAN_DIR
#define SCAN_AUTO

#ifdef SCAN_AUTO
    if (DVR_FILE_STATUS_FETCH_OK == record_file.status) {                       //检索内存索引列表
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN, "get_replay_from_list() ***** START ***** start_time= %ld end_time= %ld type= %d\n", start_time, end_time, type )
        list_count = get_replay_from_list(start_time, end_time, type, entry_list);
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN, "get_replay_from_list() ***** FINISH ***** start_time= %ld end_time= %ld type= %d list_count= %d\n", start_time, end_time, type, list_count)
    }
    else {                                                                      //检索T卡文件获取列表
        set_record_sleep_ms( SEARCH_RECORD_MS_SLOW );                           //将建立录像列表缓存的检索休眠速度降低
        list_count = get_replay_from_dir(start_time, end_time, type, entry_list);
        set_record_sleep_ms( SEARCH_RECORD_MS_FAST );                           //将建立录像列表缓存的检索休眠速度复原
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN, "get_replay_from_dir() start_time= %ld end_time= %ld type= %d list_count= %d\n", start_time, end_time, type, list_count)
    }
#endif

#ifdef SCAN_DIR
    set_record_sleep_ms( SEARCH_RECORD_MS_SLOW );                               //将建立录像列表缓存的检索休眠速度降低
    list_count = get_replay_from_dir(start_time, end_time, type, entry_list);
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN, "get_replay_from_dir() start_time= %ld end_time= %ld type= %d list_count= %d\n", start_time, end_time, type, list_count)
    set_record_sleep_ms( SEARCH_RECORD_MS_FAST );                               //将建立录像列表缓存的检索休眠速度复原
#endif

#ifdef SCAN_RAM
    for( ;; ) {
        if (DVR_FILE_STATUS_FETCH_OK == record_file.status) {                   //检索内存索引列表
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN, "get_replay_from_list() ***** START ***** start_time= %ld end_time= %ld type= %d\n", start_time, end_time, type )
            list_count = get_replay_from_list(start_time, end_time, type, entry_list);
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN, "get_replay_from_list() ***** FINISH ***** start_time= %ld end_time= %ld type= %d list_count= %d\n", start_time, end_time, type, list_count)
            break;
        }
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN, "WAIT INDEX READY\n" )
        sleep( 1 );
    }
#endif

    ak_get_ostime(&tv_end);
    ak_print_notice_ex("find file total: %d, time=%ld(ms)\n", list_count, ak_diff_ms_time(&tv_end, &tv_start));

    return list_count;
}

int ak_record_exist( time_t start_time, time_t end_time, enum dvr_file_type type )
{
    char dir_name[10] = {0};
    char dir_path[255] = {0};
    struct ak_date date;
    char c_res = AK_FALSE;
    struct dirent *dir_ent = NULL;
    char file_path[255] = {0};
    time_t calendar_time = 0;
    DIR *dirp = NULL;

    while(start_time <= end_time) {
        ak_seconds_to_date(start_time, &date);
        snprintf(dir_name, 10, "%4.4d%2.2d%2.2d", date.year, (date.month + 1), (date.day + 1));
        snprintf(dir_path, 255, "%s%s%02d/", record_file.search.path, dir_name, date.hour);
        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_name= '%s' dir_path= '%s' ak_check_file_exist(dir_path)= %d\n", dir_name , dir_path , ak_check_file_exist(dir_path) )
        if ( ( ak_check_file_exist(dir_path) != AK_SUCCESS ) || ( ( dirp = opendir(dir_path) ) == NULL ) ) {
            //DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE )
            goto ak_record_exist_next;
        }

        //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_path= '%s' dirp= %p\n", dir_path, dirp )
        while ( ( dir_ent = readdir( dirp ) ) != NULL) {
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "dir_ent->d_name= '%s'\n", dir_ent->d_name)
            if(!dir_ent->d_name) {
                goto ak_record_exist_next;
            }

            snprintf(file_path, 255, "%s%s", dir_path, dir_ent->d_name);
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "file_path= '%s' path= '%s' dir_ent->d_name= '%s'\n", file_path, dir_path, dir_ent->d_name)

            //( check_record_file_name(dir_ent->d_name, record_file.search.prefix) != type) || //判断文件类型
            if ( ( ak_is_regular_file(file_path) != AK_TRUE) || //判断是否一个常规文件
                 ( check_record_ext_name(dir_ent->d_name, strlen( dir_ent->d_name ) ) != type) ||
                 (  get_record_file_size(file_path) <= 0 ) ) {                  //文件长度小于等于0
                continue;
            }
            calendar_time = record_fs_get_time_by_name(file_path);
            if ((calendar_time >= start_time) && (calendar_time <= end_time)) {
                c_res = AK_TRUE;
                goto ak_record_exist_end;
            }
        }
ak_record_exist_next:
        if ( dirp != NULL ) {
            if (closedir(dirp)){
                ak_print_normal_ex("close dir %s error, %s\n ", record_file.search.path, strerror(errno));
            }
            else {
                dirp = NULL;
            }
        }
        start_time += ONE_HOUR_SECOND;
    }

ak_record_exist_end:
    if ( dirp != NULL ) {
        if (closedir(dirp)){
            ak_print_normal_ex("close dir %s error, %s\n ", record_file.search.path, strerror(errno));
        }
    }
    return c_res;
}

/**
 * ak_dvr_file_init - init record file manage env and start record file thread.
 * @param: record file param
 * return: 0 success; othrewise -1
 * notes: record file type, prefix and path must provide.
 */
int ak_dvr_file_init(const struct dvr_file_param *param)
{
    int pathlen, i;

    if (record_file.run_flag){
        return AK_SUCCESS;
    }
    if (NULL == param){
        ak_print_error_ex("param NULL\n");
        return AK_FAILED;
    }

    /* init mem pool */
    if (mem_pool_init_flag == AK_FALSE) {
        if (!mem_pool_init()) {
            return AK_FAILED;
        }
    }

    record_file.run_flag = AK_TRUE;
    record_file.full_flag = AK_FALSE;
    record_file.cur_time = 0;
    record_file.used_size = 0;
    record_file.free_size = 0;
    record_file.save_index = 0;
    record_file.status = DVR_FILE_STATUS_READY;

    /* save init param */
    memcpy(&(record_file.param), param, sizeof(record_file.param));
    /*make sure path end with '/' */
    pathlen = strlen(record_file.param.rec_path);
    if (('/' != record_file.param.rec_path[pathlen - 1])
        && ((pathlen + 1) < DVR_FILE_PATH_MAX_LEN)){
        record_file.param.rec_path[pathlen] = '/';
    }

    pathlen = strlen(record_file.param.rec_tmppath);
    if (pathlen > 0) {
        if ((record_file.param.rec_tmppath[pathlen - 1] != '/')
                && ((pathlen + 1) < DVR_FILE_PATH_MAX_LEN)) {
            record_file.param.rec_tmppath[pathlen] = '/';
        }
    } else {
        snprintf(record_file.param.rec_tmppath,
                DVR_FILE_PATH_MAX_LEN, "%s", RECORD_FILE_TMP_PATH);
    }
    for (i = 0; i < DVR_PAIR_NUM; i++) {
        snprintf(record_file.param.rec_tmp_file[i],
                DVR_FILE_PATH_MAX_LEN, "%s%s",
                   record_file.param.rec_tmppath, record_tmp_file[i]);
        snprintf(record_file.param.rec_tmp_idx[i],
                DVR_FILE_PATH_MAX_LEN, "%s%s",
                record_file.param.rec_tmppath, record_tmp_index[i]);

        snprintf(record_file.param.rec_tmp_moov[i], DVR_FILE_PATH_MAX_LEN,
               "%s%s", RECORD_FILE_TMP_PATH, record_tmp_moov[i]);

        snprintf(record_file.param.rec_tmp_stbl[i], DVR_FILE_PATH_MAX_LEN,
               "%s%s", RECORD_FILE_TMP_PATH, record_tmp_stbl[i]);
    }
    snprintf(record_file.param.rec_info_file, DVR_FILE_PATH_MAX_LEN,
               "%s%s", record_file.param.rec_tmppath, RECORD_FILE_INFO_NAME);
               
    record_file.search.run_flag = AK_TRUE;
    strcpy(record_file.search.prefix, record_file.param.rec_prefix);
    strcpy(record_file.search.path, record_file.param.rec_path);
    /* show init param */
    ak_print_normal("\n*** %s param ***\n", __func__);
    ak_print_normal("cyc_flag=%d, type=%d\n",
        record_file.param.cyc_flag, record_file.param.type);
    ak_print_normal("rec_prefix: %s\n", record_file.param.rec_prefix);
    ak_print_normal("rec_path: %s\n", record_file.param.rec_path);
    ak_print_normal("*** %s param End ***\n\n", __func__);

    for (i = 0; i < RECORD_NODE_NUM; ++i) {
        record_file.fp[i].index = NULL;
        record_file.fp[i].record = NULL;
        record_file.tmp_file[i] = record_file.param.rec_tmp_file[i];
        record_file.tmp_index[i] = record_file.param.rec_tmp_idx[i];
        record_file.tmp_moov[i] = record_file.param.rec_tmp_moov[i];
        record_file.tmp_stbl[i] = record_file.param.rec_tmp_stbl[i];
    }

    ak_thread_mutex_init(&(record_file.size_mutex), NULL);
    ak_thread_mutex_init(&(record_file.entry_mutex), NULL);
    ak_thread_mutex_init(&(record_file.fp_mutex), NULL);

    ak_thread_sem_init(&(record_file.pre_record_sem), 0);
    ak_thread_sem_init(&(record_file.manage_sem), 0);
    ak_thread_sem_init(&(record_file.finish_sem), 0);

    for (i=0; i<DVR_FILE_TYPE_NUM; i++) {
        INIT_LIST_HEAD(&(record_file.head[i]));
    }

    check_poweroff_file();

    int ret = ak_thread_create(&(record_file.old_file_tid), get_pre_saved_record,
        (void *)&record_file, ANYKA_THREAD_MIN_STACK_SIZE, -1);
    if(ret){
        ak_print_error_ex("create get_pre_saved_record thread, ret=%d!\n", ret);
    }

    ret = ak_thread_create(&(record_file.main_tid), record_file_main,
        (void *)&record_file, ANYKA_THREAD_MIN_STACK_SIZE, -1);
    if(ret){
        ak_print_normal_ex("unable to create record_file_main thread, ret=%d!\n", ret);
    }

    return ret;
}

/**
 * ak_dvr_file_delete_tmp_file - delete record temp file
 * @void
 * return: none
 * notes: delete record temp file if you wish.
 */
void ak_dvr_file_delete_tmp(void)
{
    /// 改用新的方式处理。
}

/**
 * ak_dvr_file_create_list - wakeup get history record file thread and fetch list
 * @void
 * return: 0 success, -1 failed
 * notes: call again after SD card reinsert and mounted
 */
int ak_dvr_file_create_list(void)
{
    if(!record_file.run_flag){
        set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
        return AK_FAILED;
    }

    /* check and create temp & record saving dir */
    create_record_dir();

    struct record_file *file = &record_file;
    ak_thread_mutex_lock(&(file->size_mutex));
    file->search.run_flag = AK_TRUE;
    file->free_size = record_fs_get_disk_free_size(file->param.rec_path);
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "file->free_size= %luKB\n", file->free_size )
    reserved_size = record_fs_get_disk_total_size(file->param.rec_path) * RESERVED_PERCENT / 100 ;
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "reserved_size= %luKB\n", reserved_size )
    ak_thread_mutex_unlock(&(file->size_mutex));

    ak_print_notice_ex("dir current free_size=%lu(KB) reserved:%lu(KB)\n",
        file->free_size, reserved_size);
    ak_thread_sem_post(&(file->pre_record_sem));

    if (file->free_size < reserved_size) {
        ak_print_notice_ex("free_size=%ld, waiting get pre saved record...\n", file->free_size);
        file->full_flag = AK_TRUE;
        ak_thread_sem_wait(&(file->finish_sem));
        ak_print_notice_ex("get pre saved record OK\n");
    }

    return AK_SUCCESS;
}

/**
 * ak_dvr_file_exit - clear record file manage env and stop record file thread.
 * @void
 * return: 0 success, -1 failed
 */
int ak_dvr_file_exit(void)
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

        mem_pool_release();
    }

    ak_print_normal("\t ***** %s *****\n", __func__);
    return AK_SUCCESS;
}
