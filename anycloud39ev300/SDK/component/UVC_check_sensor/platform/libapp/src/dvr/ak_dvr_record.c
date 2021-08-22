/**
 * Copyright (C) 2016 Anyka(Guangzhou) Microelectronics Technology CO.,LTD. 
 * File Name: ak_dvr_record.c
 * Author: Huanghaitao
 * Update date: 2017-03-20
 * Description: 
 * Notes: handle video plan & alarm record
 * History: V2.0.0 react revision
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "record_file.h"

#include "ak_common.h"
#include "ak_sd_card.h"
#include "ak_thread.h"
#include "ak_vi.h"
#include "ak_aenc.h"
#include "ak_mux.h"
#include "ak_dvr_record.h"

#ifdef AK_RTOS
#include "kernel.h"
#endif

#define RECORD_DEFAULT_PREFIX		"CYC_DV_"
#define RECORD_DEFAULT_PATH			"/mnt/CYC_DV/"

#define CALC_VIDEO_GET_STREAM_TIME	0
#define CALC_AUDIO_GET_STREAM_TIME	0
#define SHOW_CUR_MEM_INFO			0
#define SHOW_CUR_STREAM_NUM			1
#define RECORD_WRITE_AUDIO_FILE		0

#define VIDEO_RECORD_DEBUG			0
#define AUDIO_RECORD_DEBUG			0
#define VIDEO_MUX_DEBUG				0
#define AUDIO_MUX_DEBUG				0
#define STREAM_MUX_DEBUG			0
#define DROP_EXTRA_STREAM_DEBUG		0

#define VIDEO_FILTER_DEBUG			1
#define DEFAULT_GOP_LEN				2

#define VIDEO_MUX_NUM_ONCE			1
#define AUDIO_MUX_NUM_ONCE			1

#define STREAM_SYNC_TIME			(3*1000)
#define IFRAME_MAX_BYTES	    	(320*1024)
#define STREAM_DATA_MAX_SIZE    	(4*1024*1024)

struct video_entry {
	struct video_stream stream;
	struct list_head list;
};

struct record_stream {
	unsigned int num; 				//stream number
	unsigned long long mux_ts;			//the last mux stream timestamp
	unsigned long seq_no;			//stream origin sequence no.
	struct list_head mux_head; 	 	//stream data list mux head
	void *req_handle;				//request stream handle
	ak_sem_t sem;					//stream sem
	ak_mutex_t mutex; 				//stream mutex
	ak_mutex_t cancel_mutex; 		//stream cancel mutex
	ak_pthread_t tid;				//stream tid

	unsigned long long first_ts;			//the first mux stream timestamp
	unsigned long mux_cnt;			//muxed stream count
	unsigned long total;			//muxed stream total bytes
};

struct ak_dvr_record {
    unsigned char run_flag;			//video record thread run flag
    unsigned char status;			//video record current status
    unsigned char first_trigger;	//video record first trigger type
    unsigned char trigger_type;		//video record trigger type
    unsigned char iframe_num; 		//key frame(I frame) number
    unsigned char flush_finish;		//record file flush finished when stop

    unsigned char use_index;      	//current using index of record node
    unsigned char save_index;     	//save index of record node

    int trigger_iframe;				//trigger I frame time by alarm
    unsigned long long send_msg_time;    		//send message time
	unsigned long long start_time;				//record start time(ms)
    unsigned long long end_time;				//record end time(ms)
    struct ak_date date;			//record file name date time

    void *cur_mux_handle; 			//MUX current using handle
    void *save_mux_handle;			//MUX saving record handle

	void *venc_handle;				//video encode handle
	void *aenc_handle;				//audio encode handle
	
    ak_sem_t work_sem;		    	//video record working sem
    ak_sem_t stop_sem;		    	//video record stop sem
    ak_pthread_t main_tid;			//video record main tid

	ak_mutex_t stop_mutex; 			//stop video record mutex
	ak_mutex_t data_mutex;			//video & audio stream data mutex
	unsigned int data_total_size;	//video & audio stream data total size
	struct ak_timeval warning_time;	//warning over appointed size
	
    struct mux_input mux;  			//MUX input param
    struct record_param param;		//record param
    struct record_stream video;		//video stream
	struct record_stream audio;		//audio stream

#if SHOW_CUR_STREAM_NUM
	struct ak_timeval show_time;
#endif 	
};

static struct ak_dvr_record dvr_record = {0};

static const char dvr_version[] = "libapp_dvr V2.1.03";

#if SHOW_CUR_MEM_INFO
int get_free_memory(void)
{
	FILE *fd = NULL;
	char buff[256] = {0};
	int free_mem = 0;
	int buffers = 0;
	int cached = 0;

	fd = fopen ("/proc/meminfo", "r");
	if(NULL == fd) {
		printf("open /proc/meminfo FAILED\n");
		return 0;
	}
	
	fgets (buff, sizeof(buff), fd); //total
	fgets (buff, sizeof(buff), fd); //MemFree
	sscanf (buff, "MemFree: %d kB", &free_mem);
	
	fgets (buff, sizeof(buff), fd); //Buffers
	sscanf (buff, "Buffers: %d kB", &buffers);
	
	fgets (buff, sizeof(buff), fd); //Cached
	sscanf (buff, "Cached: %d kB", &cached);

	printf("\n---------- /proc/meminfo ----------\n");
	printf("\t free_mem=%d KB\n", free_mem);
	printf("\t buffers=%d KB\n", buffers);
	printf("\t cached=%d KB\n", cached);
	printf("---------- meminfo end ----------\n\n");

	fclose(fd);

	return (free_mem+buffers+cached);
}
#endif

#if SHOW_CUR_STREAM_NUM
static void show_cur_stream_num(void)
{
#if SHOW_CUR_MEM_INFO
	static int pre_mem = 0;
#endif
	struct ak_timeval cur_time;
	struct ak_dvr_record *record = &dvr_record;

	ak_get_ostime(&cur_time);	
	/* show cur stream number per 10 seconds */
	if(ak_diff_ms_time(&cur_time, &(record->show_time)) >= 10*1000) {
		if ((abs(record->video.mux_ts - record->audio.mux_ts) > 100) 
			|| (record->data_total_size >= (1024*1024))) {
			ak_print_notice("*** waiting for muxing, vstream num=%d, "
				"astream num=%d ***\n", 
				record->video.num, record->audio.num);
			ak_print_normal("video mux_ts=%llu, audio mux_ts=%llu, diff=%lld\n", 
	    		record->video.mux_ts, record->audio.mux_ts, 
	    		(record->video.mux_ts - record->audio.mux_ts));
	    	ak_print_normal("\t data_total_size=%u\n\n", record->data_total_size);
		}
		ak_get_ostime(&(record->show_time));

#if SHOW_CUR_MEM_INFO
		int cur_mem = get_free_memory();
        ak_print_info("@@@@@@@@ free memory=%d k\n", cur_mem);
        if(pre_mem != cur_mem) {
        	ak_print_info("@@@@@@@@ pre_mem=%d, cur_mem=%d, diff mem=%d k\n\n", 
        		pre_mem, cur_mem, (cur_mem - pre_mem));
        	pre_mem = cur_mem;
        }
#endif        
	}
}
#endif

#if RECORD_WRITE_AUDIO_FILE
static int audio_record_flag = AK_TRUE;
static struct ak_timeval audio_record_time;
static FILE *audio_record_fp = NULL;

static void audio_write_record_file(struct ak_dvr_record *record,
								struct aenc_entry *aentry)
{
	if (!audio_record_flag) {
		return;
	}
	
	if (audio_record_flag && !audio_record_fp) {
		switch(record->param.file_type) {
		case RECORD_FILE_TYPE_AVI:
			audio_record_fp = fopen("./record_audio.g711a", "w+");
			break;
		case RECORD_FILE_TYPE_MP4:
			audio_record_fp = fopen("./record_audio.amr", "w+");
			if (audio_record_fp) {
				const unsigned char amrheader[]= "#!AMR\n";
				fwrite(amrheader, sizeof(amrheader) - 1, 1, audio_record_fp);
			}
			break;
		default:
			return;
		}
		
		if(!audio_record_fp) {
			ak_print_error_ex("create mux audio file error\n");
			return;
		}
		ak_get_ostime(&audio_record_time);
	}

	struct ak_timeval cur_time;

	ak_get_ostime(&cur_time);	
	if(ak_diff_ms_time(&cur_time, &audio_record_time) >= 60*1000) {
		ak_print_normal("write audio file time is up\n");
		fclose(audio_record_fp);
    	audio_record_fp = NULL;
    	audio_record_flag = AK_FALSE;
		return;
	}

	if(fwrite(aentry->stream.data, 1, aentry->stream.len, audio_record_fp) < 0) {
		ak_print_error_ex("write audio file err\n");
	}
}
#endif

static void show_mux_stat_info(struct ak_dvr_record *record)
{
	ak_print_normal("\t---------- mux stat ----------\n");
	ak_print_normal(" video: first_ts=%llu, last_ts=%llu, cnt=%ld, total=%ld\n", 
		record->video.first_ts, record->video.mux_ts, 
		record->video.mux_cnt, record->video.total);
	ak_print_normal(" audio: first_ts=%llu, last_ts=%llu, cnt=%ld, total=%ld\n", 
		record->audio.first_ts, record->audio.mux_ts, 
		record->audio.mux_cnt, record->audio.total);
	ak_print_normal(" video diff_ts=%llu, audio diff_ts=%llu, write total=%ld\n\n", 
		(record->video.mux_ts - record->video.first_ts),
		(record->audio.mux_ts - record->audio.first_ts),
		(record->video.total + record->audio.total));
}

static void show_record_param(const struct record_param *param)
{
	ak_print_normal("*** %s param ***\n", __func__);
	ak_print_normal("vi_handle=0x%lX, ai_handle=0x%lX\n", 
		(unsigned long)(param->vi_handle), 
		(unsigned long)(param->ai_handle));

	ak_print_normal("width=%ld, height=%ld\n", 
		param->width, param->height);
	ak_print_normal("file_fps=%d, file_bps=%d, gop_len=%d\n", 
		param->file_fps, param->file_bps, param->gop_len);
		
	ak_print_normal("sample_rate=%d, frame_interval=%d\n", 
		param->sample_rate, param->frame_interval);
	ak_print_normal("cyc_flag=%d, file_type=%d, duration=%ld\n", 
		param->cyc_flag, param->file_type, param->duration);
	ak_print_normal("rec_prefix: %s\n", param->rec_prefix);
	ak_print_normal("rec_path: %s\n", param->rec_path);
	ak_print_normal("*** %s param End ***\n\n", __func__);
}

/** 
 * report_error_info - 
 * @record: current record type video record info
 * @ret_code[IN]: returned error code
 * return: none
 * notes: currently we just report MUX error
 */ 
static void report_error_info(struct ak_dvr_record *record, int ret_code)
{
    if(NULL != record->param.error_report){
    	int report_code = 0;
        switch(ret_code){
        case MUX_STATUS_SYSERR:
        	report_code = RECORD_EXCEPT_MUX_SYSERR;
            break;
        default:
        	report_code = ret_code;
            break;
        }
        
        ak_print_normal_ex("--- report_code=%d ---\n", report_code);
        record->param.error_report(report_code, "");
    }
}

static void get_record_file_exception(int excp_code, const char *excp_desc)
{
    report_error_info(&dvr_record, excp_code);
}

static void get_record_file_flush_status(unsigned char flush_finish)
{
	dvr_record.flush_finish = flush_finish;
}

static void free_video_stream_entry(struct ak_dvr_record *record, 
				struct video_entry *ventry)
{
	ak_thread_mutex_lock(&(record->video.mutex));
	if (ventry) {
		ak_thread_mutex_lock(&(record->data_mutex));
		record->data_total_size -= ventry->stream.len;
		ak_thread_mutex_unlock(&(record->data_mutex));
			
		list_del_init(&(ventry->list));
		ak_venc_release_stream(record->video.req_handle, &(ventry->stream));
		--record->video.num;
		
		free(ventry);
		ventry = NULL;
	}
	ak_thread_mutex_unlock(&(record->video.mutex));
}

static void free_audio_stream_entry(struct ak_dvr_record *record, 
				struct aenc_entry *aentry)
{
	ak_thread_mutex_lock(&(record->audio.mutex));
	if (aentry) {
		ak_thread_mutex_lock(&(record->data_mutex));
		record->data_total_size -= aentry->stream.len;
		ak_thread_mutex_unlock(&(record->data_mutex));

		list_del_init(&(aentry->list));
		ak_aenc_release_stream(aentry);
		--record->audio.num;
	}
	ak_thread_mutex_unlock(&(record->audio.mutex));
}

static void add_audio_mux_list(struct ak_dvr_record *record, 
							struct list_head *req_head)
{
	struct aenc_entry *aentry = NULL;
	struct aenc_entry *ptr = NULL;

	list_for_each_entry_safe(aentry, ptr, req_head, list) {
		if (record->data_total_size >= STREAM_DATA_MAX_SIZE) {
			ak_aenc_release_stream(aentry);
		} else {
			ak_thread_mutex_lock(&(record->data_mutex));
			record->data_total_size += aentry->stream.len;
			ak_thread_mutex_unlock(&(record->data_mutex));

			ak_thread_mutex_lock(&(record->audio.mutex));
			list_move_tail(&(aentry->list), &(record->audio.mux_head));
        	++record->audio.num;
        	ak_thread_mutex_unlock(&(record->audio.mutex));

#if RECORD_WRITE_AUDIO_FILE
			audio_write_record_file(record, aentry);
#endif

#if AUDIO_RECORD_DEBUG
			ak_print_normal_ex("audio: ts=%llu, len=%u\n", 
	    		aentry->stream.ts, aentry->stream.len);
		    ak_print_normal("audio.num=%d, seq_no=%lu\n\n", 
		    	record->audio.num, aentry->stream.seq_no);
#endif
		}
	}
}

static void get_audio_stream(struct ak_dvr_record *record, 
							struct list_head *req_head)
{
#if CALC_AUDIO_GET_STREAM_TIME
	unsigned long diff_time = 0;
	static unsigned char first_get_audio = AK_TRUE;
	static struct ak_timeval pre_audio_time;
	struct ak_timeval cur_audio_time;
#endif	

	if(!ak_aenc_get_stream(record->audio.req_handle, req_head)){
#if CALC_AUDIO_GET_STREAM_TIME	
		if (first_get_audio) {
			first_get_audio = AK_FALSE;
			ak_get_ostime(&pre_audio_time);
		}

		ak_get_ostime(&cur_audio_time);
		diff_time = ak_diff_ms_time(&cur_audio_time, &pre_audio_time);
		if (diff_time > 150) {
			ak_print_error_ex("get astream diff time over 100, diff_time=%ld(ms)\n",
				diff_time);
		}
		ak_get_ostime(&pre_audio_time);
#endif

		/* pause status, we just drop stream */
		if (RECORD_STATUS_SLEEP == record->status) {
			struct aenc_entry *aentry = NULL;
			struct aenc_entry *ptr = NULL;

			/* we drop these newest entry, so we don't need to lock */
			list_for_each_entry_safe(aentry, ptr, req_head, list){
				ak_aenc_release_stream(aentry);
			}
		} else {	
			add_audio_mux_list(record, req_head);
		}
	}
}

/** 
 * audio_stream_thread -  get audio stream thread
 * @arg: thread argument, current record type video record info
 * return: none
 * notes: save current audio stream to audio list
 */
static void* audio_stream_thread(void *arg)
{
	struct ak_dvr_record *record = (struct ak_dvr_record *)arg;
	struct list_head req_head;
	
	if(record->mux.capture_audio) {
		INIT_LIST_HEAD(&req_head);
	}
	
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	while(record->run_flag){
        ak_print_normal_ex("become sleep...\n");
        ak_thread_sem_wait(&(record->audio.sem));
        
        ak_print_normal_ex("become wakeup...\n");
        while(record->run_flag){
        	if((RECORD_STATUS_WORK == record->status)
        		|| (RECORD_STATUS_FILTER == record->status)
        		|| (RECORD_STATUS_SLEEP == record->status)) {
        		ak_thread_mutex_lock(&(record->audio.cancel_mutex));
        		if (record->audio.req_handle) {
	        		get_audio_stream(record, &req_head);
        		}
        		ak_thread_mutex_unlock(&(record->audio.cancel_mutex));
        		ak_sleep_ms(10);
        	} else {
				break;
        	}
        }
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

static int add_video_mux_list(struct ak_dvr_record *record,
							struct video_entry *ventry)
{
	int ret = AK_FAILED;
	if (record->data_total_size >= STREAM_DATA_MAX_SIZE) {
		struct ak_timeval cur_time;

		ak_get_ostime(&cur_time);
		if(ak_diff_ms_time(&cur_time, &(record->warning_time)) >= 5*1000) {
			ak_get_ostime(&(record->warning_time));
			ak_print_notice_ex("data_total_size=%d over 4M\n", 
				record->data_total_size);
		}
		
		ak_venc_release_stream(record->video.req_handle, &(ventry->stream));
	} else {
		ak_thread_mutex_lock(&(record->data_mutex));
		record->data_total_size += ventry->stream.len;
		ak_thread_mutex_unlock(&(record->data_mutex));
		
		ak_thread_mutex_lock(&(record->video.mutex));
		list_add_tail(&(ventry->list), &(record->video.mux_head));
		if(FRAME_TYPE_I == ventry->stream.frame_type){
		    ++record->iframe_num;
		}
		++record->video.num;
		ak_thread_mutex_unlock(&(record->video.mutex));
		ret = AK_SUCCESS;

#if VIDEO_RECORD_DEBUG
		ak_print_normal("video: frame_type=%d, ts=%llu, len=%u\n", 
			ventry->stream.frame_type, ventry->stream.ts, ventry->stream.len);
		ak_print_normal("video.num=%d, seq_no=%lu\n\n", 
			record->video.num, ventry->stream.seq_no);
#endif
	}
	return ret;
}

static void get_video_stream(struct ak_dvr_record *record)
{
	struct video_entry *ventry = (struct video_entry *)calloc(1,
		sizeof(struct video_entry));
	if(NULL == ventry){
		ak_print_error_ex("calloc ventry failed\n");
		return;
	}

#if CALC_VIDEO_GET_STREAM_TIME
	static unsigned char first_get_video = AK_TRUE;
	static struct ak_timeval pre_video_time;
	struct ak_timeval cur_video_time;
#endif	
	
	int ret = ak_venc_get_stream(record->video.req_handle, &(ventry->stream));
	if (!ret) {
#if CALC_VIDEO_GET_STREAM_TIME
		if (first_get_video) {
			first_get_video = AK_FALSE;
			ak_get_ostime(&pre_video_time);
		}
	
		ak_get_ostime(&cur_video_time);
		long diff_time = ak_diff_ms_time(&cur_video_time, &pre_video_time);
		if (diff_time > 100) {
			ak_print_notice_ex("get vstream diff time=%ld(ms), seq_no=%ld\n", 
				diff_time, ventry->stream.seq_no);
		}
		ak_get_ostime(&pre_video_time);
#endif
		
		if(ventry->stream.len >= IFRAME_MAX_BYTES) {
			ak_print_error_ex("video: frame_type=%d, ts=%llu, len=%u\n", 
		    	ventry->stream.frame_type, ventry->stream.ts, ventry->stream.len);
		    ret = AK_FAILED;
			ak_venc_release_stream(record->video.req_handle, &(ventry->stream));
		} else {
			/* pause status, we just drop video stream */
			if (RECORD_STATUS_SLEEP == record->status) {
				ret = AK_FAILED;
				ak_venc_release_stream(record->video.req_handle, &(ventry->stream));
			} else {
				ret = add_video_mux_list(record, ventry);
			}
		}
	}

	if (AK_FAILED == ret) {
		free(ventry);
		ventry = NULL;
	}
}

/** 
 * video_stream_thread -  get video stream thread
 * @arg: thread argument, current record type video record info
 * return: none
 * notes: save current video stream to video list
 */
static void* video_stream_thread(void *arg)
{
	struct ak_dvr_record *record = (struct ak_dvr_record *)arg;

	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	while(record->run_flag){
        ak_print_normal_ex("become sleep...\n");
        ak_thread_sem_wait(&(record->video.sem));
        
        ak_print_normal_ex("become wakeup...\n");
        while(record->run_flag){
        	if((RECORD_STATUS_WORK == record->status)
        		|| (RECORD_STATUS_FILTER == record->status)
        		|| (RECORD_STATUS_SLEEP == record->status)) {
        		ak_thread_mutex_lock(&(record->video.cancel_mutex));
        		if (record->video.req_handle) {
        			get_video_stream(record);
        		}
        		ak_thread_mutex_unlock(&(record->video.cancel_mutex));
        		ak_sleep_ms(10);
        	} else {
				break;
        	}
        }
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());	
	ak_thread_exit();
	return NULL;
}

/** 
 * release_media_stream -  release video & audio stream
 * @record: current record video record info
 * return: none
 */
static void release_media_stream(struct ak_dvr_record *record)
{
	int video_cnt = 0;
	struct video_entry *ventry = NULL;
	struct video_entry *vptr = NULL;

	ak_thread_mutex_lock(&(record->stop_mutex));
    list_for_each_entry_safe(ventry, vptr, &(record->video.mux_head), list){
    	ak_print_info_ex("release ventry: frame_type=%d, ts=%llu, len=%u\n", 
			ventry->stream.frame_type, ventry->stream.ts, ventry->stream.len);
		free_video_stream_entry(record, ventry);
		++video_cnt;
	}
	ak_thread_mutex_unlock(&(record->stop_mutex));

	int audio_cnt = 0;
	struct aenc_entry *aentry = NULL;
	struct aenc_entry *aptr = NULL;

	ak_thread_mutex_lock(&(record->stop_mutex));
    list_for_each_entry_safe(aentry, aptr, &(record->audio.mux_head), list){
    	ak_print_info_ex("release aentry: ts=%llu, len=%u\n", 
			aentry->stream.ts, aentry->stream.len);
		free_audio_stream_entry(record, aentry);
		++audio_cnt;
	}
	ak_thread_mutex_unlock(&(record->stop_mutex));

	ak_print_info_ex("release video_cnt=%d, audio_cnt=%d\n", 
		video_cnt, audio_cnt);
	ak_print_info_ex("vstream num=%d, astream num=%d\n", 
		record->video.num, record->audio.num);
	ak_print_info_ex("data_total_size=%u\n", record->data_total_size);
}

static void get_cur_mux_param(struct ak_dvr_record *record, struct mux_input *mux)
{
	int tmp_fps = 0;
	int cur_fps = ak_vi_get_fps(record->param.vi_handle);

	memcpy(mux, &(record->mux), sizeof(record->mux));
	if (record->mux.file_fps > cur_fps) {
		tmp_fps = cur_fps;
	} else {
		tmp_fps = record->mux.file_fps;
	}
	
	if (mux->file_fps != tmp_fps) {
		mux->file_fps = tmp_fps;
		ak_print_info_ex("cur_fps=%d, mux.file_fps=%d\n", 
			cur_fps, record->mux.file_fps);
		ak_venc_set_fps(record->venc_handle, mux->file_fps);
	}
}

/** 
 * open_mux_handle - 
 * @record: current record type video record info
 * return: 0 success; otherwize failed
 * notes: 
 */ 
static int open_mux_handle(struct ak_dvr_record *record)
{
	int ret = AK_FAILED;
    struct record_file_fp fp = {NULL};

    if(AK_SUCCESS == record_file_open(record->use_index, &fp)){
        ak_print_normal_ex("record->use_index=%d, fp.index=0x%lX, fp.record=0x%lX\n",   //
        	record->use_index, (unsigned long)(fp.index), (unsigned long)(fp.record));

		struct mux_input mux;	
		struct aenc_entry *aentry = NULL;
		get_cur_mux_param(record, &mux);
		aentry = list_first_entry_or_null(&(record->audio.mux_head), 
			struct aenc_entry, list);
		if (aentry) {
			ak_print_info_ex("aentry->stream.ts=%llu\n", aentry->stream.ts);
		}
		
		ak_thread_mutex_lock(&(record->video.mutex));
		/* determines the starting timestamp for the next record file */
		if (aentry && (aentry->stream.ts > 0)
			&& (aentry->stream.ts < record->start_time)) {
			mux.start_ts = aentry->stream.ts;
		} else {
			mux.start_ts = record->start_time;
		}
		ak_print_info_ex("audio.mux_ts=%llu, start_time=%llu\n\n", 
			record->audio.mux_ts, record->start_time);
        record->cur_mux_handle = ak_mux_open(&mux, fp.record, fp.index);
        if(NULL == record->cur_mux_handle){
        	record_file_delete(record->use_index);
        }else{
        	ak_print_normal_ex("record->use_index=%d, new cur_mux_handle=0x%lX\n\n",  //
				record->use_index, (unsigned long)(record->cur_mux_handle));
        	ret = AK_SUCCESS;
        }
        ak_thread_mutex_unlock(&(record->video.mutex));
    }

    return ret;
}

/** 
 * drop_extra_frames
 * @record: current video record info
 * return: dropped frame numbers
 * notes: we will drop all the frame before next I frame
 */
static int drop_extra_frames(struct ak_dvr_record *record)
{
	int drop_num = 0;
    struct video_entry *ventry = NULL;
	struct video_entry *vptr = NULL;

    list_for_each_entry_safe(ventry, vptr, &(record->video.mux_head), list){
        if(FRAME_TYPE_P == ventry->stream.frame_type){
        	/* drop all the frames before next I frame */
        	free_video_stream_entry(record, ventry);
        	++drop_num;
#if VIDEO_FILTER_DEBUG        	
        	ak_print_normal_ex("drop extra P frame, drop_num=%d", drop_num);
#endif        	
        }else{
        	break;
        }
    }

#if VIDEO_FILTER_DEBUG        	
	ak_print_normal_ex("drop_num=%d", drop_num);
#endif

    return drop_num;
}

/** 
 * filter_video_data
 * @record: current video record info
 * return: the latest I frame start time
 * notes: we just save appointed trigger_iframe, 
 *		audio data will sync with the latest video
 *      frame data by time. So we may drop part of video/audio data.
 */
static unsigned long long filter_video_data(struct ak_dvr_record *record)
{
	unsigned char extra_frame = AK_FALSE;
	int iframe_num = 0;

    ak_thread_mutex_lock(&(record->video.mutex));
    iframe_num = record->iframe_num;
	ak_thread_mutex_unlock(&(record->video.mutex));
	
	struct video_entry *ventry = NULL;
	struct video_entry *vptr = NULL;

    if(iframe_num > record->trigger_iframe) {
    	/* video data stream */
    	list_for_each_entry_safe(ventry, vptr, &(record->video.mux_head), list){
#if VIDEO_FILTER_DEBUG
			ak_print_normal_ex("iframe_num=%d, stream->iFrame=%d\n", 
				iframe_num, ventry->stream.frame_type);
#endif

			if(FRAME_TYPE_P == ventry->stream.frame_type){
	        	/* drop P frame */
	        	free_video_stream_entry(record, ventry);
	            ak_print_normal_ex("we will drop the extra P frame!\n");
	        }else{
	        	/* drop current I frame */
	        	free_video_stream_entry(record, ventry);
		    	
		    	--iframe_num;
		    	ak_thread_mutex_lock(&(record->video.mutex));
			    --record->iframe_num;
				ak_thread_mutex_unlock(&(record->video.mutex));
				
	        	if(iframe_num == record->trigger_iframe){
	        		extra_frame = AK_TRUE;
		        	break;
	        	}
	        }
    	}
    }

	unsigned long long start_time = 0;
    /* the next I frame */
	if(extra_frame && (drop_extra_frames(record) > 0)){
		ventry = list_first_entry(&(record->video.mux_head), struct video_entry, list);
		if(NULL != ventry){
			start_time = ventry->stream.ts;
		}
	}

    return start_time;
}

/** 
 * filter_audio_data
 * @record: current video record info
 * return: the latest I frame start time
 * notes: we just save 2 I frames, audio data will sync with the latest video
 *      frame data by time. So we may drop part of video/audio data.
 */
static void filter_audio_data(struct ak_dvr_record *record, 
							 unsigned long long cmp_time)
{
	struct aenc_entry *aentry = NULL;
	struct aenc_entry *aptr = NULL;

	list_for_each_entry_safe(aentry, aptr, &(record->audio.mux_head), list){
		if(aentry->stream.ts < cmp_time){
			free_audio_stream_entry(record, aentry);
		}else{
			break;
		}
	}
}

/** 
 * switch_record_file
 * @record: current video record info
 * @stream[IN]: current video stream info
 * return: none
 * notes: 
 */
static void switch_record_file(struct ak_dvr_record *record, 
				const struct video_stream *stream)
{
	ak_print_normal("\n\t----- record use_index=%d, time is up -----\n", 
		record->use_index);
	ak_print_normal("\t ts=%llu(ms), end_time=%llu(ms)\n", 
		stream->ts, record->end_time);
	ak_print_normal("\t extra_time=%llu(ms)\n",
		(stream->ts - record->end_time));
	ak_print_normal("\t video: frame_type=%d, ts=%llu, len=%u, seq_no=%lu\n\n", 
		stream->frame_type, stream->ts, stream->len, stream->seq_no);

	show_mux_stat_info(record);
	
	/* change mux handle and index handle */
	ak_thread_mutex_lock(&(record->video.mutex));
    record->save_mux_handle = record->cur_mux_handle;
	record->save_index = record->use_index;
    record->use_index = record->use_index ? 0 : 1;

	ak_print_normal_ex("video record using index switch to: %d\n", 
		record->use_index);
    	
	record->cur_mux_handle = NULL;
	record->start_time = stream->ts;
	record->end_time = record->start_time + record->param.duration;

    if(NULL != record->save_mux_handle){
		ak_print_normal_ex("Close save_mux_handle=0x%lX, save_index=%d record file!\n", 
			(unsigned long)(record->save_mux_handle), record->save_index);
		
		/* close mux handle */
		ak_mux_close(record->save_mux_handle);
		record->save_mux_handle = NULL;

        record_file_wakeup(record->save_index);
		ak_sleep_ms(10);
	}
	ak_thread_mutex_unlock(&(record->video.mutex));
}

static void sync_video_data(struct ak_dvr_record *record)
{
	if(!(record->run_flag)){
        return;
    }
    
	struct video_entry *ventry = NULL;
	struct video_entry *vptr = NULL;
		
    /* video data stream */
    list_for_each_entry_safe(ventry, vptr, &(record->video.mux_head), list) {
        /* drop frame before the first I frame */
        if(FRAME_TYPE_I == ventry->stream.frame_type){
        	record->start_time = ventry->stream.ts;
	        record->end_time = record->param.duration + ventry->stream.ts;
			ak_get_localdate(&(record->date));

			ak_print_normal_ex("video: frame_type=%d, ts=%llu, len=%u\n", 
	    		ventry->stream.frame_type, ventry->stream.ts, ventry->stream.len);
	        ak_print_notice_ex("ts: %llu(ms), end_time: %llu(ms), duration: %ld(ms)\n",
	        	ventry->stream.ts, record->end_time, record->param.duration);
        	break;
        }else{
        	free_video_stream_entry(record, ventry);
        	
#if DROP_EXTRA_STREAM_DEBUG
			ak_print_normal_ex("video: frame_type=%d, ts=%llu, len=%u\n", 
	    		ventry->stream.frame_type, ventry->stream.ts, ventry->stream.len);
            ak_print_normal_ex("we will drop the extra P frame!\n");
#endif            
        }

	    if (record->video.num > 20) {
	    	ak_print_warning_ex("video_num=%d\n", record->video.num);
	    }
	}
}

static void sync_audio_data(struct ak_dvr_record *record)
{
	if(!(record->run_flag)){
        return;
    }
    
	struct aenc_entry *aentry = NULL;
	struct aenc_entry *aptr = NULL;
	
	/* audio data stream */
    list_for_each_entry_safe(aentry, aptr, &(record->audio.mux_head), list){  
        /** 
         * drop all the audio data before the first video I frame
         *		and ts before the start_time
         */
	    if ((0 == record->end_time)
	    	|| (aentry->stream.ts < record->start_time)) {
	    	free_audio_stream_entry(record, aentry);
	    }else{
	    	ak_print_notice_ex("mux first audio: ts=%llu, len=%u\n", 
	    		aentry->stream.ts, aentry->stream.len);
	    	break;
	    }
	}
}

/** 
 * filter_record_stream
 * @record: current video record info
 * return: none
 * notes: we just save 2 I frames, audio data will sync with the latest video
 *      frame data by time. So we may drop part of video/audio data.
 */ 
static void filter_record_stream(struct ak_dvr_record *record)
{
	unsigned long long cmp_time = 0;
	int iframe_num = 0;

    ak_thread_mutex_lock(&(record->video.mutex));
    iframe_num = record->iframe_num;
    ak_thread_mutex_unlock(&(record->video.mutex));

	if((iframe_num > record->trigger_iframe)
		&& !list_empty(&(record->video.mux_head))){
        cmp_time = filter_video_data(record);
        ak_print_normal_ex("cmp_time=%llu\n", cmp_time);

	    if((cmp_time > 0) && !list_empty(&(record->audio.mux_head))){
	    	filter_audio_data(record, cmp_time);
		}
    }
}

static void mux_video_data(struct ak_dvr_record *record)
{
	int ret = 0;
	int video_mux_num = 0;
	unsigned long long first_ts = 0;
	struct video_entry *ventry = NULL;
	struct video_entry *vptr = NULL;

	ak_thread_mutex_lock(&(record->stop_mutex));
    /* video data stream */
    list_for_each_entry_safe(ventry, vptr, &(record->video.mux_head), list){
#if 0    
    	/* sync with audio in appointed seconds */
    	if((record->audio.mux_ts > 0)
    		&& (ventry->stream.ts > (record->audio.mux_ts + STREAM_SYNC_TIME))) {

    		ak_print_normal("111 skip mux, pre ts=%llu, cur_ts=%llu, ats=%llu, seq_no=%lu\n", 
				record->video.mux_ts, ventry->stream.ts, 
				record->audio.mux_ts, ventry->stream.seq_no);
    		break;
		}
#endif

	    if(FRAME_TYPE_I == ventry->stream.frame_type){
	        /** switch record file:
			 * split record according to I frame time stamp,
		     * split unit: record_time 
		     */
	        if(ventry->stream.ts >= record->end_time){
	        	ak_get_localdate(&(record->date));
	    		switch_record_file(record, &(ventry->stream));
	    	}
	    }

	    if(NULL == record->cur_mux_handle){
			ak_print_normal("\n\t----- start new record use_index=%d -----\n", 
	    		record->use_index);
			ak_print_normal_ex("frame_type=%d, ts=%llu, end_time=%llu\n",      //
				ventry->stream.frame_type, ventry->stream.ts, record->end_time);

			record->video.first_ts =  ventry->stream.ts;
			record->video.mux_cnt = 0;
			record->video.total = 0;
			record->audio.first_ts = 0;
			record->audio.mux_cnt = 0;
			record->audio.total = 0;

			char time_str[20] = {0};
			ak_date_to_string(&(record->date), time_str);
			ak_print_normal_ex("date time: %s\n", time_str);
	        	
			record_file_generate_name(record->use_index, &(record->date));
			if(AK_FAILED == open_mux_handle(record)){
				ak_print_normal_ex("open new mux handle FAILED\n");
			}
		}

	    if((RECORD_STATUS_WORK == record->status) && record->cur_mux_handle) {
#if VIDEO_MUX_DEBUG
	        ak_print_normal_ex("video: frame_type=%d, ts=%llu, len=%u, seq_no=%lu\n", 
		    	ventry->stream.frame_type, ventry->stream.ts, 
		    	ventry->stream.len, ventry->stream.seq_no);
#endif
            
	    	ret = ak_mux_add_video(record->cur_mux_handle, &(ventry->stream));
		    if(AK_SUCCESS == ret){
		    	if (0 == first_ts) {
		    		first_ts = ventry->stream.ts;
		    	}
		    	record->video.mux_ts = ventry->stream.ts;
		    	record->video.seq_no = ventry->stream.seq_no;
		    	++record->video.mux_cnt;
				record->video.total += ventry->stream.len;
		    }else{
		    	ak_print_normal_ex("video mux failed, ret=%d\n", ret);
		    	ak_venc_set_iframe(record->venc_handle);
		        if(MUX_STATUS_SYSERR == ret){
		        	ak_print_normal_ex("--- mux_addVideo ret=%d ---\n", ret);
		            report_error_info(record, ret);
		            record->status = RECORD_STATUS_EXCEPT;
		        }
		    }
	    }

		free_video_stream_entry(record, ventry);
		++video_mux_num;

#if STREAM_MUX_DEBUG
		ak_print_notice_ex("111 video_ts=%ld, seq_no=%ld\n", 
			record->video.mux_ts, record->video.seq_no);
		ak_print_notice_ex("111 video_num=%d, audio_num=%d, video_mux_num=%d\n", 
			record->video.num, record->audio.num, video_mux_num);
#endif			

	    /* ONLY mux appointed video number each time */
	    if (video_mux_num >= VIDEO_MUX_NUM_ONCE) {
			break;
	    }
	}
	ak_thread_mutex_unlock(&(record->stop_mutex));

    if (video_mux_num > 1) {
    	ak_print_notice("video_mux_num=%d, wait num=%d\n", 
    		video_mux_num, record->video.num);
    	ak_print_normal("first ts=%llu, mux_ts=%llu, diff=%llu, seq_no=%lu\n", 
    		first_ts, record->video.mux_ts, 
    		(record->video.mux_ts - first_ts), record->video.seq_no);
    }
}

static void mux_audio_data(struct ak_dvr_record *record)
{
	int ret = 0;
	int audio_mux_num = 0;
	unsigned long long first_ts = 0;
	struct aenc_entry *aentry = NULL;
	struct aenc_entry *aptr = NULL;

	ak_thread_mutex_lock(&(record->stop_mutex));
	/* add all the audio which the timestamp before video_ts to mux */
    list_for_each_entry_safe(aentry, aptr, &(record->audio.mux_head), list) {
		/* sync with video in appointed seconds */
    	if((record->video.mux_ts > 0)
    		&& (aentry->stream.ts > (record->video.mux_ts + STREAM_SYNC_TIME))) {
#if 0
    		ak_print_normal("222 skip mux, pre ts=%llu, cur_ts=%llu, vts=%llu, seq_no=%lu\n", 
				record->audio.mux_ts, aentry->stream.ts, 
				record->video.mux_ts, aentry->stream.seq_no);
#endif				
    		break;
		}

	    if(RECORD_STATUS_WORK == record->status) {
#if AUDIO_MUX_DEBUG
			ak_print_normal_ex("audio: ts=%llu, len=%u, seq_no=%lu\n\n", 
	    		aentry->stream.ts, aentry->stream.len, record->audio.seq_no);
#endif	

			ret = ak_mux_add_audio(record->cur_mux_handle, &(aentry->stream));
			switch (ret) {
			case AK_SUCCESS:
				if (0 == first_ts) {
		    		first_ts = aentry->stream.ts;
		    	}
				record->audio.mux_ts = aentry->stream.ts;
				record->audio.seq_no = aentry->stream.seq_no;

				if (0 == record->audio.first_ts) {
					record->audio.first_ts = aentry->stream.ts;
				}
				++record->audio.mux_cnt;
				record->audio.total += aentry->stream.len;
				break;
			case MUX_STATUS_SYSERR:
				ak_print_normal_ex("--- ak_mux_add_audio ret=%d ---\n", ret);
		        report_error_info(record, ret);
		        record->status = RECORD_STATUS_EXCEPT;
				break;
			default:
				break;
			}
		}

	    free_audio_stream_entry(record, aentry);
		++audio_mux_num;

#if STREAM_MUX_DEBUG
		ak_print_normal_ex("222 audio_ts=%ld, seq_no=%ld\n", 
			record->audio.mux_ts, record->audio.seq_no);
		ak_print_normal_ex("222 video_num=%d, audio_num=%d, audio_mux_num=%d\n", 
			record->video.num, record->audio.num, audio_mux_num);
#endif
		
		/* ONLY mux appointed audio number each time */
	    if (audio_mux_num >= AUDIO_MUX_NUM_ONCE) {
			break;
	    }
	}
	ak_thread_mutex_unlock(&(record->stop_mutex));

	if (audio_mux_num > 1) {
		ak_print_info("audio_mux_num=%d, wait num=%d\n", 
    		audio_mux_num, record->audio.num);
    	ak_print_info("first_ts=%llu, mux_ts=%llu, diff=%llu, seq_no=%lu\n\n", 
    		first_ts, record->audio.mux_ts, 
    		(record->audio.mux_ts - first_ts), record->audio.seq_no);
    }
}

/* start video & audio stream request */
static int start_request_stream(struct ak_dvr_record *record)
{
	if(record->mux.capture_audio){
		record->audio.req_handle = ak_aenc_request_stream(record->param.ai_handle, 
			record->aenc_handle);
		ak_print_notice_ex("astream_handle=%p\n\n", record->audio.req_handle);
		if(NULL == record->audio.req_handle) {
			ak_print_error_ex("request aenc stream failed\n");
			return AK_FAILED;
		}
	}

	record->video.req_handle = ak_venc_request_stream(record->param.vi_handle, 
		record->venc_handle);
	ak_print_notice_ex("vstream_handle=%p\n\n", record->video.req_handle);
	if(NULL == record->video.req_handle) {
		ak_print_error_ex("request venc stream failed\n");
		if (record->audio.req_handle) {
			ak_aenc_cancel_stream(record->audio.req_handle);
			record->audio.req_handle = NULL;
		}
		
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

/** 
 * video_record_working
 * @record: current record type video record info
 * return: none
 * notes: 
 */
static void video_record_working(struct ak_dvr_record *record)
{
#if SHOW_CUR_STREAM_NUM
	ak_get_ostime(&(record->show_time));
#endif

	record->status = RECORD_STATUS_WORK;
    while(record->run_flag 
    	&& (RECORD_STATUS_WORK == record->status)
    	&& (0 == record->end_time)){
		/* record->end_time is 0, so we have not sync the data yet */
		sync_video_data(record);
		sync_audio_data(record);

#if SHOW_CUR_STREAM_NUM
		show_cur_stream_num();
#endif		
    	ak_sleep_ms(10);
	}

	while(record->run_flag 
		&& (RECORD_STATUS_WORK == record->status)
		&& (record->end_time > 0)){
    	/* make sure we have sync the data yet */
		mux_video_data(record);
		mux_audio_data(record);

#if SHOW_CUR_STREAM_NUM
		show_cur_stream_num();
#endif

		if ((0 == record->video.num) || (0 == record->audio.num)) {
			ak_sleep_ms(5);
		}
	}

	record->end_time = 0;
	ak_thread_sem_post(&(record->stop_sem));
	ak_print_notice_ex("exit status=%d\n", record->status);
}

/** 
 * video_record_filter
 * @record: current video record info
 * return: none
 * notes: 
 */
static void video_record_filter(struct ak_dvr_record *record)
{
	while(record->run_flag && (RECORD_STATUS_FILTER == record->status)){
    	filter_record_stream(record);
		ak_sleep_ms(5);
	}
	
	ak_print_normal_ex("exit status=%d\n", record->status);
}

/** 
 * video_record_main
 * @arg: thread input arg, current record type video record info
 * return: none
 * notes: 
 */
static void* video_record_main(void *arg)
{
    struct ak_dvr_record *record = (struct ak_dvr_record *)arg;

    ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
    while(record->run_flag){
        ak_print_normal_ex("become sleep...\n");
        ak_thread_sem_wait(&(record->work_sem));
        ak_print_normal_ex("become wakeup, record->status=%d\n", record->status);
        
        switch(record->status){
        case RECORD_STATUS_WORK:
        	video_record_working(record);
        	break;
        case RECORD_STATUS_FILTER:
        	video_record_filter(record);
        	break;
        case RECORD_STATUS_EXIT:
        	record->run_flag = AK_FALSE;
        	break;
        default:
        	break;
        }
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());	
	ak_thread_exit();
	return NULL;
}

static void* init_record_venc(const struct record_param *record_param)
{
	struct encode_param venc_param = {0};

	venc_param.width = record_param->width;
	venc_param.height = record_param->height;
	venc_param.minqp = 20;
	venc_param.maxqp = 51;
	venc_param.fps = record_param->file_fps;
	venc_param.goplen = venc_param.fps * 2;
	venc_param.bps = record_param->file_bps;
	venc_param.profile = PROFILE_MAIN;
	venc_param.use_chn = record_param->enc_chn;
	venc_param.enc_grp = ENCODE_RECORD;
	venc_param.br_mode = BR_MODE_CBR;
	venc_param.enc_out_type = H264_ENC_TYPE;

	return ak_venc_open(&venc_param);
}

static void* init_record_aenc(const struct record_param *record_param)
{
	struct audio_param aenc_param = {0};

	switch(record_param->file_type) {
    case RECORD_FILE_TYPE_MP4:
    	aenc_param.type = AK_AUDIO_TYPE_AMR;
    	break;
    case RECORD_FILE_TYPE_AVI:
    	aenc_param.type = AK_AUDIO_TYPE_PCM_ALAW;
    	break;
    default:
    	ak_print_error_ex("unkown media type %d\n", record_param->file_type);
		return NULL;
    }

    aenc_param.sample_bits = 16;
	aenc_param.channel_num = AUDIO_CHANNEL_MONO;
	aenc_param.sample_rate = record_param->sample_rate;

    return ak_aenc_open(&aenc_param);
}

static int init_record_mux(struct ak_dvr_record *record)
{	
	/* mux video */
	switch(record->param.file_type) {
	case RECORD_FILE_TYPE_MP4:
		record->mux.media_rec_type = RECORD_TYPE_3GP;
		record->mux.audio_type = AUDIO_MUX_TYPE_AMR;
		break;
	case RECORD_FILE_TYPE_AVI:
		record->mux.media_rec_type = RECORD_TYPE_AVI_NORMAL;
		record->mux.audio_type = AUDIO_MUX_TYPE_G711;
		break;
	default:
		ak_print_error_ex("unkown media type %d\n", record->param.file_type);
		return AK_FAILED;
	}

	record->mux.video_type = VIDEO_MUX_TYPE_H264;
	record->mux.width = record->param.width;
	record->mux.height = record->param.height;
	record->mux.file_fps = record->param.file_fps;

	if (record->param.ai_handle) {
		/* mux audio */
		record->mux.capture_audio = 1;
		record->mux.sample_rate = record->param.sample_rate;
		record->mux.frame_interval = record->param.frame_interval;
	} else {
		record->mux.capture_audio = 0;
		record->mux.sample_rate = 0;
		record->mux.frame_interval = 0;
	}

	ak_print_normal("\n*** %s param ***\n", __func__);
	ak_print_normal("media_rec_type=%d, video_type=%d\n", 
		record->mux.media_rec_type, record->mux.video_type);
    ak_print_normal("width=%d, height=%d\n", 
		record->mux.width, record->mux.height);

	ak_print_normal("capture_audio=%d, audio_type=%d\n", 
		record->mux.capture_audio, record->mux.audio_type);
	ak_print_normal("file_fps=%d, sample_rate=%ld, frame_interval=%d\n", 
		record->mux.file_fps, record->mux.sample_rate, record->mux.frame_interval);
	ak_print_normal("*** %s End ***\n\n", __func__);

	return AK_SUCCESS;
}

static void init_record_others(struct ak_dvr_record *record)
{
	record->run_flag = AK_TRUE;
    record->status = RECORD_STATUS_INIT;
    record->trigger_type = RECORD_TRIGGER_TYPE_RESERVED;
    record->first_trigger = RECORD_TRIGGER_TYPE_RESERVED;
    record->iframe_num = 0;
    
    record->use_index = 0;
    record->save_index = 0;
    record->cur_mux_handle = NULL;
    record->save_mux_handle = NULL;
	record->start_time = 0;
	record->end_time = 0;
	ak_thread_sem_init(&(record->work_sem), 0);
	ak_thread_sem_init(&(record->stop_sem), 0);

	ak_thread_mutex_init(&(record->stop_mutex));
	ak_thread_mutex_init(&(record->data_mutex));
	record->data_total_size = 0;
	
    record->video.num = 0;
    record->video.mux_ts = 0;
    record->video.seq_no = 0;
    record->video.req_handle = NULL;
    INIT_LIST_HEAD(&(record->video.mux_head));
	ak_thread_sem_init(&(record->video.sem), 0);
    ak_thread_mutex_init(&(record->video.mutex));
    ak_thread_mutex_init(&(record->video.cancel_mutex));
    
    record->audio.num = 0;
    record->audio.mux_ts = 0;
    record->audio.seq_no = 0;
    record->audio.req_handle = NULL;
    INIT_LIST_HEAD(&(record->audio.mux_head));
    ak_thread_sem_init(&(record->audio.sem), 0);
    ak_thread_mutex_init(&(record->audio.mutex));
    ak_thread_mutex_init(&(record->audio.cancel_mutex));
}

/* record file param and init */
static void init_record_file(struct ak_dvr_record *record)
{
    struct file_param file;

    file.cyc_flag = record->param.cyc_flag;
    file.type = record->param.file_type;
    file.except_report = get_record_file_exception;
    file.flush_cb = get_record_file_flush_status;
    
    if (strlen(record->param.rec_prefix)) {
    	strcpy(file.rec_prefix, record->param.rec_prefix);
    } else {
    	strcpy(file.rec_prefix, RECORD_DEFAULT_PREFIX);
    }
    if (strlen(record->param.rec_path)) {
    	strcpy(file.rec_path, record->param.rec_path);
    } else {
    	strcpy(file.rec_path, RECORD_DEFAULT_PATH);
    }
    record_file_init(&file);
}

/**
 * ak_dvr_record_get_version - get dvr version
 * return: version string
 */
const char* ak_dvr_record_get_version(void)
{
	ak_print_notice_ex("%s\n", dvr_version);
	return dvr_version;
}

/** 
 * ak_dvr_record_get_status - get current video record status
 * @void
 * return: current video record status
 */
enum record_status ak_dvr_record_get_status(void)
{
	return dvr_record.status;
}

/** 
 * ak_dvr_record_pause
 * @void
 * return: none
 * notes: call by record_ctrl_pause_record
 */
void ak_dvr_record_pause(void)
{
    struct ak_dvr_record *record = &dvr_record;

	if(RECORD_STATUS_WORK == record->status) {
	    record->status = RECORD_STATUS_SLEEP;
	}
}

/** 
 * ak_dvr_record_resume
 * @trigger_type: record trigger type
 * return: none
 * notes: call by record_ctrl_resume_record
 */
void ak_dvr_record_resume(int trigger_type)
{
	struct ak_dvr_record *record = &dvr_record;

	ak_print_normal("\n----- [%s], record->status=%d -----\n", 
			__func__, record->status);
	switch(record->status){
	case RECORD_STATUS_IDLE:
	case RECORD_STATUS_SLEEP:
	case RECORD_STATUS_FILTER:
		if(RECORD_TRIGGER_TYPE_RESERVED == record->first_trigger){
			record->first_trigger = trigger_type;
		}
		
		record->trigger_type |= trigger_type;
		ak_print_normal("----- [%s], trigger_type=%d -----\n\n", 
			__func__, record->trigger_type);

	    record->start_time = 0;
	    record->end_time = 0;
	    if(RECORD_STATUS_IDLE == record->status){
	    	if(AK_FAILED == start_request_stream(record)){
	    		ak_print_error_ex("start stream request failed\n");
	    		return;
	    	}
	    }
	    
    	record->status = RECORD_STATUS_WORK;
    	ak_thread_sem_post(&(record->audio.sem));
    	ak_thread_sem_post(&(record->video.sem));
    	ak_thread_sem_post(&(record->work_sem));
		break;
	case RECORD_STATUS_WORK:
		/* video record has already started */
		record->trigger_type |= trigger_type;
		break;
	case RECORD_STATUS_OP:
		record->status = RECORD_STATUS_IDLE;
		break;
	default:
		break;
	}
}

/** 
 * ak_dvr_record_start - start video record
 * @void
 * return: none
 */
void ak_dvr_record_start(void)
{
	/* sd card may reinsert */
	record_file_get_pre_record();
	ak_dvr_record_resume(RECORD_TRIGGER_TYPE_PLAN);
}

/** 
 * ak_dvr_record_stop
 * @void
 * return: none
 * notes: call by record_ctrl_stop_record
 */
void ak_dvr_record_stop(void)
{
    struct ak_dvr_record *record = &dvr_record;

	ak_print_info_ex("enter ...\n");
	ak_thread_mutex_lock(&(record->video.cancel_mutex));
	/* cancel media stream firstly */
    if (record->video.req_handle) {
		ak_venc_cancel_stream(record->video.req_handle);
		record->video.req_handle = NULL;
	}
	ak_thread_mutex_unlock(&(record->video.cancel_mutex));

	ak_thread_mutex_lock(&(record->audio.cancel_mutex));
    if(record->mux.capture_audio) {
		if (record->audio.req_handle) {
			ak_aenc_cancel_stream(record->audio.req_handle);
			record->audio.req_handle = NULL;
		}
	}
	ak_thread_mutex_unlock(&(record->audio.cancel_mutex));

	ak_print_notice_ex("record->status=%d, vstream num=%d, astream num=%d\n", 
		record->status, record->video.num, record->audio.num);
	/* stop record, but still get media stream */
	if (RECORD_STATUS_WORK == record->status) {
		record->status = RECORD_STATUS_SLEEP;
		ak_print_notice_ex("wait record mux stop...\n");
		ak_thread_sem_wait(&(record->stop_sem));
		ak_print_notice_ex("record mux stopped\n");
		ak_print_notice_ex("record->status=%d, vstream num=%d, astream num=%d\n", 
			record->status, record->video.num, record->audio.num);
	}

	record->status = RECORD_STATUS_STOP;
	record->trigger_type = RECORD_TRIGGER_TYPE_RESERVED;
	if(NULL != record->cur_mux_handle){
		ak_print_normal_ex("Close cur_mux_handle=0x%lX, use_index=%d record file\n", 
			(unsigned long)(record->cur_mux_handle), record->use_index);

		/* close mux handle, must be failed when write record file */
		ak_mux_close(record->cur_mux_handle);
		record->cur_mux_handle = NULL;

		/* make sure the sd card is normal */
		if(SD_STATUS_CARD_INSERT & ak_sd_check_insert_status()){
			if(AK_SUCCESS == ak_sd_check_mount_status()){
				record_file_wakeup(record->use_index);
				while (record->run_flag && !dvr_record.flush_finish) {
					ak_sleep_ms(10);
				}
				dvr_record.flush_finish = AK_FALSE;
		    }
		}
	}

	release_media_stream(record);
	record->status = RECORD_STATUS_IDLE;
	ak_print_notice_ex("stop OK\n");
	ak_sleep_ms(10);
}

/** 
 * ak_dvr_record_op_stop - stop current record because of operating outside
 * @op_type[IN]: stop operation type
 * return: none
 * notes: operation such as SD card format, record plan change...
 */
void ak_dvr_record_op_stop(enum dvr_op_stop_type op_type)
{
	if (DVR_OP_TYPE_FORMAT_CARD == op_type) {
		record_file_stop_search();
	}
	
	ak_dvr_record_stop();
	dvr_record.status = RECORD_STATUS_OP;
}

/** 
 * ak_dvr_record_switch_fps
 * @switch_fps: fps value to be switched
 * return: none
 * notes: we'll switch the record frame between normal and min fps.
 */
void ak_dvr_record_switch_fps(int switch_fps)
{
#if 0
	ak_print_normal_ex("switch_fps=%d", switch_fps);
	video_set_encode_fps(dvr_record.param.video_encode_type, switch_fps);
	video_set_encode_goplen(dvr_record.param.video_encode_type,
		dvr_record.param.mux.gop_len);
#endif		
}

/** 
 * ak_dvr_record_start_trigger - start alarm trigger
 * @trigger_time[IN]: alarm trigger time, we'll save stream filp 
 *			after alarm happened.
 * return: none
 * notes: we'll filter the video & audio datastream if no record triggered.
 *		can call by alarm detected
 */
void ak_dvr_record_start_trigger(int trigger_time)
{
	struct ak_dvr_record *record = &dvr_record;
	
	if(RECORD_STATUS_IDLE == record->status){
		if(AK_FAILED == start_request_stream(record)){
    		ak_print_error_ex("start stream request failed\n");
    		return;
    	}
		record->status = RECORD_STATUS_FILTER;
		ak_print_normal_ex("----- record->status=%d -----\n\n", record->status);
    	ak_thread_sem_post(&(record->work_sem));
	}

	if (record->param.gop_len < DEFAULT_GOP_LEN) {
		record->param.gop_len = DEFAULT_GOP_LEN;
	}
	
	if (trigger_time < record->param.gop_len) {
		record->trigger_iframe = record->param.gop_len;
	} else {
		record->trigger_iframe = (trigger_time / record->param.gop_len);
		if (trigger_time % record->param.gop_len) {
			record->trigger_iframe += 1;
		}
	}
}

/** 
 * ak_dvr_record_init - init video record env and start video record thread.
 * @stream_handle[IN]: request video stream handle
 * @param: video record param
 * return: 0 success; otherwise -1
 */
int ak_dvr_record_init(const struct record_param *param)
{
	ak_print_normal("\n\t ----- %s -----\n", __func__);
	ak_dvr_record_get_version();
	
    struct ak_dvr_record *record = &dvr_record;
    if(record->run_flag){
        return AK_FAILED;
    }
	
    memset(record, 0x00, sizeof(struct ak_dvr_record));
   	memcpy(&(record->param), param, sizeof(record->param));
	show_record_param(&(record->param));

	int ret = AK_FAILED;
	
	record->venc_handle = init_record_venc(&(record->param));
	if (NULL == record->venc_handle) {
		goto record_init_end;
	}
	record->aenc_handle = init_record_aenc(&(record->param));
	if (NULL == record->aenc_handle) {
		goto record_init_end;
	}
	init_record_mux(record);
	init_record_others(record);
	init_record_file(record);
    
    record->status = RECORD_STATUS_IDLE;
    #ifdef AK_RTOS
    ret = ak_thread_create(&(record->main_tid), video_record_main,
        (void *)record, ANYKA_THREAD_MIN_STACK_SIZE, 85);
    #else
    ret = ak_thread_create(&(record->main_tid), video_record_main,
        (void *)record, ANYKA_THREAD_MIN_STACK_SIZE, -1);
    #endif        
	if(0 != ret){
		ak_print_normal_ex("unable to create video_record_main thread, "
			"ret = %d!\n", ret);
		goto record_init_end;
	}

	#ifdef AK_RTOS
    ret = ak_thread_create(&(record->video.tid), video_stream_thread,
        (void *)record, ANYKA_THREAD_MIN_STACK_SIZE, 85);
	#else
	ret = ak_thread_create(&(record->video.tid), video_stream_thread,
        (void *)record, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	#endif
	if(0 != ret){
		ak_print_normal_ex("unable to create get_media_stream thread, "
			"ret = %d!\n", ret);
		goto record_init_end;
	}

	if(record->mux.capture_audio) {
		#ifdef AK_RTOS
		ret = ak_thread_create(&(record->audio.tid), audio_stream_thread,
	        (void *)record, ANYKA_THREAD_MIN_STACK_SIZE, 85);
		#else
		ret = ak_thread_create(&(record->audio.tid), audio_stream_thread,
	        (void *)record, ANYKA_THREAD_MIN_STACK_SIZE, -1);
		
		#endif
		if(0 != ret){
			ak_print_normal_ex("unable to create get_media_stream thread, "
				"ret = %d!\n", ret);
			goto record_init_end;
		}
	}

	ret = AK_SUCCESS;

record_init_end:	
	return ret;
}

/** 
 * ak_dvr_record_exit: clear video record env and stop video record thread.
 * @void
 * return: none
 */
void ak_dvr_record_exit(void)
{
	struct ak_dvr_record *record = &dvr_record;

	if(record->run_flag){
		ak_dvr_record_stop();
		record->run_flag = AK_FALSE;

		if(record->mux.capture_audio) {
			ak_thread_sem_post(&(record->audio.sem));
			ak_print_notice_ex("join audio stream thread\n");
			ak_thread_join(record->audio.tid);
			ak_print_notice_ex("audio stream thread join OK\n");
		}

		ak_thread_sem_post(&(record->video.sem));
		ak_print_notice_ex("join video stream thread\n");
		ak_thread_join(record->video.tid);
		ak_print_notice_ex("video stream thread join OK\n");

		if(record->venc_handle){
			ak_venc_close(record->venc_handle);
			record->venc_handle = NULL;
		}
		if(record->aenc_handle){
			ak_aenc_close(record->aenc_handle);
			record->aenc_handle = NULL;
		}

		ak_thread_sem_post(&(record->work_sem));
		ak_thread_join(record->main_tid);
		ak_print_notice_ex("record main thread join OK\n");
		
		record->status = RECORD_STATUS_EXIT;		
		ak_thread_mutex_destroy(&(record->video.mutex));
		ak_thread_mutex_destroy(&(record->audio.mutex));
		ak_thread_mutex_destroy(&(record->video.cancel_mutex));
		ak_thread_mutex_destroy(&(record->audio.cancel_mutex));
		ak_thread_mutex_destroy(&(record->data_mutex));
		ak_thread_mutex_destroy(&(record->stop_mutex));
	    ak_thread_sem_destroy(&(record->work_sem));
	    ak_thread_sem_destroy(&(record->stop_sem));
	    ak_thread_sem_destroy(&(record->video.sem));
	    ak_thread_sem_destroy(&(record->audio.sem));

	    ak_print_normal("\t ***** %s *****\n", __func__);
	}

	record_file_exit();
	ak_print_info_ex("leave...\n");
}
