/**
* Copyright (C) 2016 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: record_replay.c
* Author: Huanghaitao
* Update date: 2016-04-06
* Description:
* Notes:
* History: V2.0.0 react revision
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "record_common.h"

#include "ak_common.h"
#include "ak_demux.h"
#include "ak_sd_card.h"
#include "ak_thread.h"
#include "internal_error.h"
#include "ak_dvr_replay.h"
#include "ak_mux.h"
//#include "printcolor.h"

#define DUMP_REPLAY_LIST        0
#define PRINT_REPLAY_PARAM        0
#define DVR_REPLAY_LIST_INFO    0
#define REPLAY_DATA_DEBUG        0

/* record replay status */
enum replay_status {
    REPLAY_STATUS_RESERVED = 0x00,
    REPLAY_STATUS_INIT,
    REPLAY_STATUS_READY,
    REPLAY_STATUS_WORKING,
    REPLAY_STATUS_PAUSE,
    REPLAY_STATUS_STOP
};

struct record_replay {
    unsigned char run_flag;                    //replay thread run flag
    enum replay_status status;                //replay status

    ak_mutex_t file_mutex;                    //record file mutex
    ak_sem_t work_sem;                         //replay working sem
    ak_sem_t replay_sem;                     //replay working sem
    ak_pthread_t replay_tid;                //replay thread id

    struct dvr_replay_request request;        //record replay request info
    FP_RECORD_REPLAY_CALLBACK send_data;    //replay callback

    unsigned long start_time;                //list start time, second
    unsigned long end_time;                  //list end time, second
    unsigned int entry_num;                    //replay list entry number
    struct list_head head;                    //replay list head
};

static struct record_replay record_replay = {0};

#if REPLAY_DATA_DEBUG
static FILE *video_fp = NULL;
static FILE *audio_fp = NULL;
#endif

#if PRINT_REPLAY_PARAM
static void print_replay_param(const struct dvr_replay_param *param)
{
    ak_print_normal_ex("--- replay request param ---\n");
    ak_print_normal_ex("type=%d\n", param->type);
    ak_print_normal_ex("start_time=%lu, str: %s\n",
        param->start_time, ak_seconds_to_string(param->start_time));
    ak_print_normal_ex("end_time=%lu, str: %s\n",
        param->end_time, ak_seconds_to_string(param->end_time));
    ak_print_normal_ex("--- replay request param end ---\n\n");
}
#endif

/** get_entry_info - get replay entry info by record entry
 * @record_entry[IN]: record entry
 * return: replay entry pointer; otherwise NULL
 */
static struct dvr_replay_entry* get_entry_info( struct dvr_file_entry *record_entry)
{
    if (!record_entry) {
        set_error_no(ERROR_TYPE_POINTER_NULL);
        return NULL;
    }
    if (!record_entry->path) {
        set_error_no(ERROR_TYPE_POINTER_NULL);
        return NULL;
    }

    int ret = AK_FAILED;
    struct dvr_replay_entry *replay_entry = (struct dvr_replay_entry *)calloc
            (1, sizeof(struct dvr_replay_entry));
    if (!replay_entry) {
        set_error_no(ERROR_TYPE_MALLOC_FAILED);
        ak_print_error_ex("calloc dvr_replay_entry error \n");
        goto replay_entry_end;
    }

    int path_len = strlen(record_entry->path);
    replay_entry->path = (char *)calloc(1, (path_len + 1));
    if (!replay_entry->path) {
        set_error_no(ERROR_TYPE_MALLOC_FAILED);
        ak_print_error_ex("calloc replay_entry->path error\n");
        goto replay_entry_end;
    }

    /* save replay entry info */
    memcpy(replay_entry->path, record_entry->path, path_len);
    replay_entry->total_time = record_entry->total_time;
    replay_entry->start_time = record_entry->calendar_time;
    ret = AK_SUCCESS;

replay_entry_end:
    if (ret) {
        if (replay_entry) {
            if (replay_entry->path) {
                free(replay_entry->path);
                replay_entry->path = NULL;
            }
            free(replay_entry);
            replay_entry = NULL;
        }
    }

    return replay_entry;
}

/**
 * check_entry_time - check if the record file time match request time
 * @entry[IN]: replay entry info
 * return: 0 success, matched; otherwise -1
 * notes:
 */
static int check_entry_time(struct dvr_replay_entry *entry)
{
    int ret = AK_FAILED;
    unsigned long record_start = entry->start_time;
    unsigned long record_end = (entry->start_time + entry->total_time/1000);
    unsigned long request_start = record_replay.request.start_time;
    unsigned long request_end = record_replay.request.end_time;

    if ((record_start <= request_start) && (record_end > request_start)) {      //录像开始时间在请求时间早
        entry->play_start_pos = request_start - record_start;                   //根据请求时间和录像开始时间设置开始播放的偏移
        if (record_end > request_end) {                                         //record_start <= request_start < request_end < record_end
            /* request full in [record_start, record_end] */
            entry->play_duration = request_end - request_start;
        } else {                                                                //record_start <= request_start < record_end <= request_end
            /* entry part in [request_start, request_end) */
            entry->play_duration = record_end - request_start;
        }
        ret = AK_SUCCESS;
    }
    else if((record_start >= request_start) && (record_end <= request_end)) { //request_start <= record_start < record_end <= request_end
        /* entry full in [request_start, request_end] */
        entry->play_start_pos = 0;
        entry->play_duration = entry->total_time;
        ret = AK_SUCCESS;
    }
    else if((record_start < request_end) && (record_end > request_end)) {     //record_start < request_end < request_end <= record_end
        /* entry part in (request_start, request_end] */
        entry->play_start_pos = 0;
        entry->play_duration = request_end - record_start;
        ret = AK_SUCCESS;
    }
    else {
        /*
        DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED,
                     "path= '%s' record_start= %lu record_end= %lu request_start= %lu request_end= %lu\n",
                     entry->path, record_start, record_end, request_start, request_end )
                     */
        ret = AK_FAILED;

    }
    return ret;
}

/** get_tick_count: get current time
 * void
 * return: current time(second)
 * notes:
 */
static unsigned long get_tick_count(void)
{
    unsigned long cur_time = 0;
    struct ak_timeval tv = {0};

    ak_get_ostime(&tv);
    cur_time = (tv.sec)*1000 + (tv.usec) /1000;

    return cur_time;
}

#if REPLAY_DATA_DEBUG
static void write_replay_debug_data(enum demux_data_type type,
                                    struct demux_stream *record_data,
                                    unsigned long long ts_audio,
                                    unsigned long long ts_video)
{
    if ((DEMUX_DATA_VIDEO == type) && video_fp) {
        /* save video data to file */
        ts_video = record_data->ts;
        fwrite(record_data->data, 1, record_data->len, video_fp);
    }
    if ((DEMUX_DATA_AUDIO == type) && audio_fp) {
        /* save audio data to file */
        ts_audio = record_data->ts;
        fwrite(record_data->data, 1, record_data->len, audio_fp);
    }

    /* check video ts and audio ts difference */
    if (ts_video > ts_audio) {
        if (ts_video - ts_audio > 2000)
            ak_print_error_ex("audio ts =%ld, video ts=%ld,diff=%ld\n",
            (unsigned long)ts_audio, (unsigned long)ts_video,
            (unsigned long)(ts_video - ts_audio));
    } else {
        if (ts_audio- ts_video > 2000)
            ak_print_error_ex("audio ts =%ld, video ts=%ld,diff=%ld\n",
            (unsigned long)ts_audio, (unsigned long)ts_video,
            (unsigned long)(ts_audio - ts_video));
    }
}
#endif

/**
 * send_replay_data: send the replay data to play
 * @entry[IN]: replay entry info
 * return: 0 success; otherwise -1
 * notes:
 */
static int send_replay_data(struct dvr_replay_entry *entry)
{
    void *handle = ak_demux_open(entry->path, entry->play_start_pos*1000);
    if (!handle) {
        ak_print_error_ex("video_demux_open faield\n");
        return AK_FAILED;
    }

    int ret = AK_SUCCESS;
    enum demux_data_type type = DEMUX_DATA_UNKNOW;
    int video_enc_type;
    int audio_enc_type;
    int enc_type = 0;

    ak_demux_get_enc_type(handle, &video_enc_type, &audio_enc_type);
    ak_print_normal_ex("encode type(platform),video: %d audio:%d\n",
                 video_enc_type, audio_enc_type);
#if REPLAY_DATA_DEBUG
    unsigned long long ts_video = 0;
    unsigned long long ts_audio = 0;
#endif

    struct demux_stream *record_data = ak_demux_get_data(handle, &type);
    if (!record_data)
        goto replay_data_end;

    unsigned long base_time = get_tick_count() - record_data->ts;

    while (record_data) {
        /* replay enter pause status */
        if (record_replay.run_flag
            && (REPLAY_STATUS_PAUSE == record_replay.status)) {
            ak_thread_sem_wait(&record_replay.replay_sem);
        }
        /* replay enter stop status */
        if (REPLAY_STATUS_STOP == record_replay.status) {
            break;
        }
        if (!(record_replay.run_flag)) {
            ak_print_normal_ex("user cancel the replay record\n");
            ret = AK_FAILED;
            break;
        }
        while (get_tick_count() - base_time < record_data->ts) {
            unsigned long diff = record_data->ts - (get_tick_count() - base_time);
            if (diff > 100)
                diff = 50;
            ak_sleep_ms(diff);
        }
        /* send replay data according to type */
        if (record_replay.send_data) {
#if REPLAY_DATA_DEBUG
            write_replay_debug_data(type, record_data, ts_audio, ts_video);
#endif

            /*
             * if send_record is blocking function,not need to copy record_data,
             * if send_record is non-blocking function,have to copy record_data
             */
            enc_type = (DEMUX_DATA_VIDEO == type) ? video_enc_type : audio_enc_type;
            record_replay.send_data(record_replay.request.user_data,
                type, record_data, enc_type);
        }
        ak_demux_free_data(record_data);

        /* next frame */
        record_data = ak_demux_get_data(handle, &type);
        ak_sleep_ms(10);
    }

replay_data_end:
    if (record_data) {
        ak_demux_free_data(record_data);
    } else {
        set_error_no(ERROR_TYPE_POINTER_NULL);
        ak_print_normal_ex("fail to get data, end\n");
    }

    ak_demux_close(handle);
    if ( record_replay.status != REPLAY_STATUS_STOP ) {                                   //假如回放运行状态不为REPLAY_STATUS_STOP,执行标识文件结束的回调函数
        //record_replay.send_data(record_replay.request.user_data, DEMUX_DATA_UNKNOW, NULL, 0);       //结束播放文件的回调
    }
    return ret;
}

/**
 * delete_replay_file - delete the record replay file
 * @head[IN]: replay list head
 * return: none
 */
static void delete_replay_file(struct list_head *head)
{
    struct dvr_replay_entry *ptr = NULL;
    struct dvr_replay_entry *entry = NULL;

    list_for_each_entry_safe(entry, ptr, head, list) {
        if (entry) {
            list_del_init(&(entry->list));
            FREE_POINT(entry->path);
            FREE_POINT(entry);
        }
    }
}

/** replay_main: record replay main thread
 * @arg[IN]: thread input arg
 * return: none
 * notes:
 */
static void* replay_main(void *arg)
{
    ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
    ak_thread_set_name("dvr_replay");

    unsigned char match_flag = AK_FALSE;
    struct dvr_replay_entry *entry = NULL;

    while (record_replay.run_flag) {
        ak_thread_sem_wait(&record_replay.work_sem);
        if (!record_replay.run_flag) {
            break;
        }
        /* check real replay time and send replay data */
        ak_thread_mutex_lock(&(record_replay.file_mutex));
        list_for_each_entry(entry, &record_replay.head, list) {
            if (AK_SUCCESS == check_entry_time(entry)) {
                match_flag = AK_TRUE;
                if (AK_FAILED == send_replay_data(entry)) {
                    record_replay.status = REPLAY_STATUS_STOP;
                    break;
                }
            }

            if (!record_replay.run_flag
                || (REPLAY_STATUS_STOP == record_replay.status)) {
                break;
            }
        }
        ak_thread_mutex_unlock(&(record_replay.file_mutex));

        ak_sleep_ms(10);
        if (match_flag) {
            ak_print_normal("\t replay to end or pause!\n");
        } else {
            ak_print_normal("\t can not find the right file to play!\n");
        }
    }

    ak_print_normal_ex("\t### thread tid: %ld exit ###\n", ak_thread_get_tid());
    ak_thread_exit();
    return NULL;
}

/** add_to_replay_tail: add record file to global replay list tail
 * @entry_head[IN]: replay list head
 * return: AK_SUCCESS
 */
static int add_to_replay_tail(struct list_head *entry_head)
{
    struct dvr_file_entry *entry = NULL;
    struct dvr_file_entry *ptr = NULL;
    struct dvr_replay_entry *replay_entry = NULL;
    struct dvr_replay_entry *last_replay = list_last_entry(
        &record_replay.head, struct dvr_replay_entry, list);

    ak_print_info_ex("enter...\n");
    list_for_each_entry_safe(entry, ptr, entry_head, list) {
        if (entry->total_time > 0) {
            replay_entry = get_entry_info(entry);
            if (replay_entry) {
                if (last_replay) {
                    if (entry->calendar_time != last_replay->start_time) {
                        list_add_tail(&(replay_entry->list), &record_replay.head);
                        ++record_replay.entry_num;
                    }
                } else {
                    list_add_tail(&(replay_entry->list), &record_replay.head);
                    ++record_replay.entry_num;
                }
            }
        }
        list_del_init(&(entry->list));
        free_record_file_entry(entry);
    }
    ak_print_info_ex("leave..., record_replay.entry_num=%d\n",
        record_replay.entry_num);

    return AK_SUCCESS;
}

static int file_to_ret_list(struct list_head *entry_head,
                            struct list_head *ret_list)                         //将dvr_file_entry 转换为 dvr_replay_entry
{
    struct dvr_file_entry *entry = NULL;
    struct dvr_file_entry *ptr = NULL;
    struct dvr_replay_entry *ret_entry = NULL;
    int replay_count = 0 ;
    time_t i_unixts_mark;
    struct dvr_file_entry *p_entry_mark;

    for( ;; ) {                                                                 //循环排序输出
        i_unixts_mark = 0 ;
        p_entry_mark = NULL;

        list_for_each_entry_safe(entry, ptr, entry_head, list) {                //遍历文件列表链表
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "entry->path= '%s'\n", entry->path )
            if (entry->total_time <= 0) {
                list_del_init(&(entry->list));
                free_record_file_entry(entry);
            }
            if ( ( i_unixts_mark == 0 ) || ( entry->calendar_time < i_unixts_mark ) ) {     //获取时间戳最小的文件
                i_unixts_mark = entry->calendar_time;
                p_entry_mark = entry;
            }
        }
        if( p_entry_mark == NULL ){                                             //链表已经无元素
            break;
        }
        ret_entry = get_entry_info( p_entry_mark );
        if ( ( ret_entry = get_entry_info( p_entry_mark ) ) != NULL ) {         //转换成dvr_replay_entry结构体
            list_add_tail(&(ret_entry->list), ret_list);
            //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "ret_entry->path= '%s'\n", ret_entry->path )
            replay_count ++;
        }
        list_del_init( &( p_entry_mark->list ) );                               //从链表中删除当前结构体
        free_record_file_entry( p_entry_mark );                                 //释放结构体
    }

    return replay_count;
}


/** free_replay_list: free all replay list
 * void
 * return: AK_SUCCESS
 */
static int free_replay_list(void)
{
    /* delete pre replay list */
    ak_thread_mutex_lock(&(record_replay.file_mutex));
    delete_replay_file(&record_replay.head);
    record_replay.entry_num = 0;
    record_replay.start_time = 0;
    record_replay.end_time = 0;
    ak_thread_mutex_unlock(&(record_replay.file_mutex));

    return AK_SUCCESS;
}

static int file_to_replay_list(struct list_head *entry_head)
{
#if DUMP_REPLAY_LIST
    int i = 0;
    struct dvr_file_entry *entry = NULL;
    list_for_each_entry(entry, entry_head, list) {
        ak_print_normal("entry-%.3d: %s\n", ++i, entry->path);
    }
#endif

    free_replay_list();
    add_to_replay_tail(entry_head);

    return AK_SUCCESS;
}

/**
 * ak_dvr_replay_init - init record replay env and start replay thread.
 * @void
 * return: 0 success; othrewise -1
 * notes:
 */
int ak_dvr_replay_init(void)
{
    int ret = AK_SUCCESS;

    if (!record_replay.run_flag) {
        record_replay.run_flag = AK_TRUE;
        record_replay.status = REPLAY_STATUS_INIT;
        record_replay.entry_num = 0;

        INIT_LIST_HEAD(&(record_replay.head));
        ak_thread_mutex_init(&(record_replay.file_mutex), NULL);
        ak_thread_sem_init(&(record_replay.work_sem), 0);
        ak_thread_sem_init(&(record_replay.replay_sem), 0);

        ret = ak_thread_create(&record_replay.replay_tid, replay_main,
            NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
        if(ret) {
            ak_print_normal("unable to create replay_main thread, ret = %d!\n", ret);
        }

#if REPLAY_DATA_DEBUG
        if (!video_fp)
            video_fp = fopen("/mnt/video.h264", "w+");
        if (!video_fp)
            ak_print_error_ex("open /mnt/video.h264 failed\n");

        if (!audio_fp)
            audio_fp = fopen("/mnt/audio.g711a", "w+");
        if (!audio_fp)
            ak_print_error_ex("open /mnt/audio.g711a failed\n");
#endif
    }

    return ret;
}

/**
 * ak_dvr_replay_exit - clear record replay env and exit replay thread.
 * @void
 * return: 0 success; othrewise -1
 * notes:
 */
int ak_dvr_replay_exit(void)
{
    int ret = AK_SUCCESS;
    ak_print_info_ex("entry ...\n");

    if (record_replay.run_flag) {
        record_replay.run_flag = AK_FALSE;
        record_replay.status = REPLAY_STATUS_STOP;

        ak_thread_sem_post(&(record_replay.work_sem));
        /* wait for replay_main thread exit */
        ak_print_notice_ex("join record replay thread\n");
        ak_thread_join(record_replay.replay_tid);
        ak_print_notice_ex("record replay thread join OK\n");

        free_replay_list();
        ak_thread_mutex_destroy(&(record_replay.file_mutex));
        ak_thread_sem_destroy(&(record_replay.work_sem));
        ak_thread_sem_destroy(&(record_replay.replay_sem));

#if REPLAY_DATA_DEBUG
        if (video_fp) {
            fclose(video_fp);
            video_fp = NULL;
        }

        if (audio_fp) {
            fclose(audio_fp);
            audio_fp = NULL;
        }
#endif
    }
    ak_print_info_ex("leave ...\n");

    return ret;
}

/**
 * replay_pause - pause replay
 * return: void
 * notes:
 */
void ak_dvr_replay_pause(void)
{
    if (REPLAY_STATUS_WORKING == record_replay.status) {
        record_replay.status = REPLAY_STATUS_PAUSE;
    }
}

/**
 * ak_dvr_replay_resume - resume replay
 * return: void
 * notes:
 */
void ak_dvr_replay_resume(void)
{
    if (REPLAY_STATUS_PAUSE == record_replay.status) {
        record_replay.status = REPLAY_STATUS_WORKING;
        ak_thread_sem_post(&(record_replay.replay_sem));
    }
}

/**
 * ak_dvr_replay_stop - stop replay
 * @usr_data[IN]: user data
 * return: void
 * notes:
 */
void ak_dvr_replay_stop(void *usr_data)
{
    if (usr_data == record_replay.request.user_data) {
        if (record_replay.run_flag) {
            enum replay_status pre_status = record_replay.status;

            record_replay.status = REPLAY_STATUS_STOP;
            if (REPLAY_STATUS_PAUSE == pre_status) {
                ak_thread_sem_post(&(record_replay.replay_sem));
            }
        }
    } else {
        ak_print_notice_ex("no replay to stop\n");
    }
    free_replay_list();
}

/**
 * ak_dvr_replay_start - start record replay
 * @request[IN]: replay request info, you must set [start_time, end_time]
 * @send_record[IN]: send record data callback
 * return: 0 success, -1 failed
 * notes: 1. we'll send replay media data via "send_record" callback
 *         2. replay media data just in [start_time, end_time]
 */
int ak_dvr_replay_start(struct dvr_replay_request *request,
                        FP_RECORD_REPLAY_CALLBACK send_record)
{
    if ((ak_sd_check_mount_status() < 0)) {
        ak_print_normal("it fails to start beacuse of sd out!\n");
        set_error_no(ERROR_TYPE_FUNC_NOT_SUPPORT);
        return AK_FAILED;
    }

    if (!request) {
        set_error_no(ERROR_TYPE_POINTER_NULL);
        return AK_FAILED;
    }
    if (!record_replay.run_flag) {
        set_error_no(ERROR_TYPE_NOT_INIT);
        return AK_FAILED;
    }

    /* replay is working */
    if (REPLAY_STATUS_WORKING == record_replay.status) {
        set_error_no(ERROR_TYPE_EBUSY);
        return AK_FAILED;
    }

    int entry_num = 0;
    struct list_head get_head;
    INIT_LIST_HEAD(&get_head);

    entry_num = record_file_get_list(request->start_time, request->end_time, request->record_type, &get_head);
    if(entry_num <= 0) {
        ak_print_error_ex("get list failed\n");
        return AK_FAILED;
    }
    file_to_replay_list(&get_head);
    record_replay.send_data = send_record;
    memcpy(&(record_replay.request), request, sizeof(struct dvr_replay_request));
    record_replay.status = REPLAY_STATUS_WORKING;
    ak_thread_sem_post(&(record_replay.work_sem));

    return AK_SUCCESS;
}

/**
 * ak_dvr_replay_fetch_list - fetch replay record file list
 * @param[IN]: replay param, according to [start_time, end_time] and type
 * @replay_list[OUT]: fetched replay list, entry type "struct dvr_replay_entry"
 * return: entry number in replay_list, 0 failed
 * notes: 1.call ak_dvr_record_init before fetch list
 *         2.just return replay entry list in [start_time, end_time]
 *        3.you MUST free fetch list atfer used
 *        4.we'll save a complete replay entry list internal
 */
int ak_dvr_replay_fetch_list(const struct dvr_replay_param *param,
                            struct list_head **replay_list)
{
    if (!param) {
        set_error_no(ERROR_TYPE_POINTER_NULL);
        return 0;
    }
    if (ak_sd_check_mount_status()) {
        set_error_no(ERROR_TYPE_FUNC_NOT_SUPPORT);
        return 0;
    }

    if (!record_replay.run_flag) {
        set_error_no(ERROR_TYPE_NOT_INIT);
        return 0;
    }

#if PRINT_REPLAY_PARAM
    print_replay_param(param);
#endif

    int entry_num = 0;
    struct list_head get_head;
    struct list_head *ret_head = (struct list_head *)calloc(1, sizeof(struct list_head));
    if (!ret_head) {
        set_error_no(ERROR_TYPE_MALLOC_FAILED);
        ak_print_error_ex("calloc cur_head failed\n");
        goto replay_fetch_end;
    }

    //record_replay.status = REPLAY_STATUS_READY;
    INIT_LIST_HEAD(&get_head);
    INIT_LIST_HEAD(ret_head);

    //DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN )
    entry_num = record_file_get_list(param->start_time, param->end_time, param->type, &get_head);
    //DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN )
    if ( (entry_num <= 0) || (( entry_num = file_to_ret_list(&get_head, ret_head) ) <= 0 )){  //struct dvr_file_entry => struct dvr_replay_entry
        FREE_POINT(ret_head);
    }
    //DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE )

    ak_print_normal_ex("find entry_num=%d, record_replay.entry_num=%d\n", entry_num, record_replay.entry_num);
    //DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "find entry_num=%d, record_replay.entry_num=%d\n", entry_num, record_replay.entry_num )

replay_fetch_end:
    *replay_list = ret_head;

    return entry_num;
}

/**
 * ak_dvr_replay_free_fetch_list - free fetched replay list
 * @fetch_list: fetched list by ak_dvr_replay_fetch_list
 * return: 0 success,  -1 failed
 * notes: you MUST free fetched list after call ak_dvr_replay_fetch_list success
 */
int ak_dvr_replay_free_fetch_list(struct list_head *fetch_list)
{
    if (!fetch_list) {
        return AK_FAILED;
    }

    delete_replay_file(fetch_list);
    free(fetch_list);
    fetch_list = NULL;

    return AK_SUCCESS;
}
