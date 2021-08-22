/**
* Copyright (C) 2016 Anyka(Guangzhou) Microelectronics Technology CO.,LTD. 
* File Name: record_ctrl.c
* Author: Huanghaitao
* Update date: 2016-03-22
* Description: 
* Notes:
* History: V2.0.0 react revision
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_sd_card.h"

#include "ak_vi.h"
#include "ak_venc.h"
#include "ak_ai.h"
#include "ak_aenc.h"

#include "ak_config.h"
//#include "ak_alarm.h"
#include "ak_record_common.h"
#include "record_ctrl.h"
#include "ak_dvr_record.h"

#include "drv_api.h"
#include "kernel.h"

#define FIRST_PATH       "/etc/jffs2/isp_sc1135_qq.conf"
#define BACK_PATH        "/usr/local/isp_sc1135_qq.conf"

#define AKGPIO_DEV 					"/dev/akgpio"
#define ALARM_TMP_PHOTO_DIR         "/tmp/alarm/"
#define INVALID_VALUE				-1
#define SECONDS_PER_DAY				(24*60*60)

/* send alarm mode */
enum send_alarm_mode_e
{
    SEND_ALARM_MODE_ONLY_PHOTO = 0x00,      //only photo mode
    SEND_ALARM_MODE_BOTH,              		//both photo & record mode
};

/* send alarm mode */
enum alarm_sensitivity_level_e
{
	ALARM_SENS_LEVEL_CLOSE = 0x00,			//close sensitivity
	ALARM_SENS_LEVEL_LOW,					//low sensitivity
	ALARM_SENS_LEVEL_MID,					//middle sensitivity
	ALARM_SENS_LEVEL_HIGH,					//high sensitivity
	ALARM_SENS_LEVEL_NUM					//level number
};

/* current plan record info */
typedef struct _plan_record
{
    unsigned char status;				//plan record status
	int index;					//current started record plan's index 
	int start_wday;     		//plan record start week day
    time_t start_time;     		//plan record start time
    time_t end_time;       		//plan record end time
    time_t next_time;      		//plan record next time
}plan_record_s;

/* current alarm record info */
typedef struct _alarm_record
{
	unsigned char init_flag;			//init flag
	unsigned char save_record_flag;	//save record flag
    unsigned char happen_flag;         //alarm happen flag(move, voice or other)
    unsigned char record_flag;         //alarm record flag(move, voice or other)
    unsigned char check_photo_flag;    //check taking photo flag
    unsigned char min_fps_flag;     	//switch to min fps flag
    
    unsigned char status;				//alarm record status
	int type;        			//alarm type
    
	time_t start_time;			//alarm detection start time
    time_t send_time;     		//send alarm time
}alarm_record_s;

struct record_ctrl_s {
    unsigned char run_flag;	        //record control thread run flag
    unsigned char alarm_init_flag;  //alarm record init flag
    unsigned char except_type;	    //except type
	unsigned char detect_type;      //detection type

	void *vi_handle;				//video input handle
	void *venc_handle;				//video encode handle
	void *ai_handle;				//audio input handle
	void *aenc_handle;				//audio encode handle
	
    plan_record_s plan;				//plan record info
};

static struct record_ctrl_s record_ctrl = {0};
static struct video_config  g_video_config;

static void get_video_record_error(int error_code, const char *error_desc)
{
	ak_print_normal_ex("--- error_code=%d ---\n", error_code);
	switch(error_code){
    case RECORD_EXCEPT_NO_VIDEO://can't capture video data
    	break;
    case RECORD_EXCEPT_NO_AUDIO://can't capture audio data
    	break;
    case RECORD_EXCEPT_MUX_SYSERR:
    	break;
    default:
    	break;
    }
    record_ctrl.except_type |= error_code;
    ak_print_normal_ex("--- except_type=%d ---\n", record_ctrl.except_type);
}

static void get_record_file_exception(int cxcp_code, const char * cxcp_desc)
{
    switch(cxcp_code){
    case RECORD_EXCEPT_SD_NO_SPACE://SD card space not enough
    	record_ctrl_stop_record();
    	break;
    case RECORD_EXCEPT_SD_RO://SD card read only
    	break;
    default:
    	break;
    }
    record_ctrl.except_type |= cxcp_code;
}

static int report_exception(int except_type)
{
	if(record_ctrl.except_type & RECORD_EXCEPT_SD_REMOVED){
		
	}
	
    return AK_SUCCESS;
}

static void* init_venc(enum encode_group_type enc_type)
{
	struct encode_param param = {0};
//	struct video_config *video_config = ak_config_get_sys_video();

	struct video_config  * video_config = &g_video_config;

	
	switch (enc_type) {
	case ENCODE_RECORD:
		param.width = video_config->saveWidth;
		param.height = video_config->saveHeight;
		param.minqp = video_config->minQp;
		param.maxqp = video_config->maxQp;
		param.fps = video_config->savefilefps;
		param.goplen = video_config->gopLen * video_config->savefilefps;
		param.bps = video_config->savefilekbps;
		param.profile = PROFILE_MAIN;
		if (param.width <=640)
		    param.use_chn = ENCODE_SUB_CHN;
        else		    
    		param.use_chn = ENCODE_MAIN_CHN;
		param.enc_grp = ENCODE_RECORD;
		param.br_mode = video_config->video_mode;
		param.enc_out_type = H264_ENC_TYPE;
		break;
	case ENCODE_MAINCHN_NET:
		param.width = video_config->saveWidth;
		param.height = video_config->saveHeight;
		param.minqp = video_config->minQp;
		param.maxqp = video_config->maxQp;
		param.fps = video_config->V720Pfps;
		param.goplen = video_config->gopLen * video_config->V720Pfps;
		param.bps = video_config->V720Pminkbps;
		param.profile = PROFILE_MAIN;
		param.use_chn = ENCODE_MAIN_CHN;
		param.enc_grp = ENCODE_MAINCHN_NET;
		param.br_mode = video_config->video_mode;
		param.enc_out_type = H264_ENC_TYPE;
		break;
	case ENCODE_SUBCHN_NET:
		param.width = video_config->saveWidth;;
		param.height = video_config->saveHeight;
		param.minqp = video_config->minQp;
		param.maxqp = video_config->maxQp;
		param.fps = video_config->VGAPfps;
		param.goplen = video_config->gopLen * video_config->VGAPfps;
		param.bps = video_config->VGAminkbps;
		param.profile = PROFILE_MAIN;
		param.use_chn = ENCODE_SUB_CHN;
		param.enc_grp = ENCODE_SUBCHN_NET;
		param.br_mode = BR_MODE_CBR;
		param.enc_out_type = H264_ENC_TYPE;
		break;
	case ENCODE_PICTURE:
		param.width = 640;
		param.height = 360;
		param.minqp = 20;
		param.maxqp = 51;
		param.fps = 10;
		param.goplen = param.fps * 2;
		param.bps = 500;	//kbps
		param.profile = PROFILE_MAIN;
		param.use_chn = video_config->pic_ch;
		param.enc_grp = ENCODE_PICTURE;
		param.br_mode = BR_MODE_CBR;
		param.enc_out_type = MJPEG_ENC_TYPE;
		break;
	default:
		return NULL;
	}

	return ak_venc_open(&param);
}

static void* init_aenc(void)
{
	struct audio_param aenc_param = {0};
	
	aenc_param.type = AK_AUDIO_TYPE_PCM_ALAW;
	aenc_param.sample_bits = 16;
	aenc_param.channel_num = AUDIO_CHANNEL_MONO;
	aenc_param.sample_rate = 8000;

    return ak_aenc_open(&aenc_param);
}

/** 
 * init_record_common - init record common part.
 * @file_type: record file type
 * return: void
 */
static void init_record_common(enum record_file_type file_type)
{	
    char file_name[128];
    T_SYSTIME systime;


	struct video_config *video_config =&g_video_config;
	
	/* video record param and init */
	struct record_param record;

    memset(&record, 0, sizeof(record));
	record.vi_handle = record_ctrl.vi_handle;
	record.ai_handle = record_ctrl.ai_handle;
	
	record.width = video_config->saveWidth;
	record.height = video_config->saveHeight;

	if (record.width <= 640) {
		record.enc_chn = ENCODE_SUB_CHN;
	} else {
		record.enc_chn = ENCODE_MAIN_CHN;
	}

    
	record.file_fps = video_config->savefilefps;
	record.file_bps = video_config->savefilekbps;
	record.sample_rate = 8000;
    record.frame_interval =100;

	record.cyc_flag = video_config->save_cyc_flag;
	record.file_type = file_type;
	
    if (video_config->save_cyc_flag)
        record.duration = video_config->saveTime * 60 *  1000 ; //max time 
    else
        //avoid genereate the second file
        record.duration = (video_config->saveTime+1) * 60 *  1000 ; 
        
    strcpy(record.rec_path, video_config->recpath);
    
	record.error_report = get_video_record_error;
	
    ak_dvr_record_init(&record);
}

/**
 * check_plan_continuity - get record plan's next start time in plan table,
 *		and check the plan's continuity.
 * @plan_index[IN]: current started record plan's wday
 * @cur_wday[IN]: current started record plan's wday
 * return: 0 continuity; -1 interrupted
 * notes: we can use this to check the record plan's continuity
 */
static int check_plan_continuity(int plan_index, int cur_wday)
{
	if((plan_index >= MAX_RECORD_PLAN_NUM) || (plan_index < 0)){
		return AK_FAILED;
	}
	if((cur_wday >= DAY_PER_WEEK) || (cur_wday < 0)){
		return AK_FAILED;
	}

	struct video_record_config *config = ak_config_get_record_plan();
    if(NULL == config){
		return AK_FAILED;
    }

	int i = 0;
	int ret = AK_FAILED;
	int diff_day = 0;
	int next_wday = 0;
    time_t next_time = 0;
    time_t cur_time = 0;

	for(i=cur_wday; i<DAY_PER_WEEK; ++i){
		next_wday = i + 1;
		++diff_day;
		if(DAY_PER_WEEK == next_wday){
			next_wday = 0;
		}

		ak_print_normal("\nrecord plan[%d], cur week_enalbe[%d]=%d, "
	    	"next week_enalbe[%d]=%d\n", 
			plan_index, i, config->plan[plan_index].week_enalbe[i],
			next_wday, config->plan[plan_index].week_enalbe[next_wday]);
		
		if(config->plan[plan_index].active 
	    	&& config->plan[plan_index].week_enalbe[next_wday]){
	    	cur_time = get_passed_seconds();
			next_time = (SECONDS_PER_DAY *diff_day) 
				+ day_secs_to_total(config->plan[plan_index].start_time);

			ak_print_normal("\n\t cur_secs:\t %ld(s), %s\n", 
				cur_time, secs_to_readable_string(cur_time));
			ak_print_normal("\t plan_end_time:\t %ld(s), %s\n", 
				record_ctrl.plan.end_time, 
				secs_to_readable_string(record_ctrl.plan.end_time));
			ak_print_normal("\t plan next_time:\t %ld(s), %s\n", 
				next_time, secs_to_readable_string(next_time));
			if((record_ctrl.plan.end_time + 1) >= next_time){
				/* update current record plan info */
				record_ctrl.plan.end_time = (SECONDS_PER_DAY *diff_day) 
					+ day_secs_to_total(config->plan[plan_index].end_time);
				record_ctrl.plan.start_wday = next_wday;
				ret = AK_SUCCESS;
			}
			break;
	    }
	}

	return ret;
}

#if 0
//lgd remove

static void check_plan_record_start(void)
{

	struct video_record_config *config = ak_config_get_record_plan();
    if(NULL == config){
    	record_ctrl.plan.status = RECORD_PLAN_NO_CONFIG;
		return;
    }
    
	int i = 0;
    struct tm *cur = NULL;
    time_t start = 0;
    time_t end = 0;
    time_t cur_secs = 0;
    
	for(i=0; i<MAX_RECORD_PLAN_NUM; ++i){
    	cur = get_tm_time();
#if 0  	
        ak_print_normal("\n\t record plan[%d], active=%d, week_enalbe[%d]=%d, "
        	"start_time: %ld, end_time: %ld\n", 
			i, config->plan[i].active, 
			cur->tm_wday, config->plan[i].week_enalbe[cur->tm_wday], 
			config->plan[i].start_time, config->plan[i].end_time);
#endif
			
        if(config->plan[i].active && config->plan[i].week_enalbe[cur->tm_wday]){
			cur_secs = get_passed_seconds();
			start = day_secs_to_total(config->plan[i].start_time);
			end = day_secs_to_total(config->plan[i].end_time);
			ak_print_normal("\t\t config->plan[%d].start_time:\t %ld(s)\n", 
				i, config->plan[i].start_time);
			ak_print_normal("\t\t config->plan[%d].end_time:\t %ld(s)\n", 
				i, config->plan[i].end_time);
			ak_print_normal("\t\t cur_time:\t %ld(s), %s\n", 
				cur_secs, secs_to_readable_string(cur_secs));
			ak_print_normal("\t\t start_time:\t %ld(s), %s\n", 
				start, secs_to_readable_string(start));
			ak_print_normal("\t\t end_time:\t %ld(s), %s\n", 
				end, secs_to_readable_string(end));
        	if((cur_secs >= start) && (cur_secs < end)){
            	record_ctrl.plan.index = i;
            	record_ctrl.plan.start_wday = cur->tm_wday;
            	record_ctrl.plan.end_time = end;
	            record_ctrl.plan.status = RECORD_PLAN_START; 
	            record_ctrl_resume_record(RECORD_TRIGGER_TYPE_PLAN);

	            cur_secs = get_passed_seconds();
	            ak_print_normal("\n\t we will start the record plan[%d]\n", i);
	            ak_print_normal("\t cur_time:\t %ld(s), %s\n", 
					cur_secs, secs_to_readable_string(cur_secs));
				ak_print_normal("\t start_time:\t %ld(s), %s\n", 
					start, secs_to_readable_string(start));
				ak_print_normal("\t end_time:\t %ld(s), %s\n\n", 
					end, secs_to_readable_string(end));	
				break;
            }
        }
    }
}

static void check_plan_record_stop(void)
{
    time_t cur_secs = get_passed_seconds();
    
	if(cur_secs >= record_ctrl.plan.end_time){
    	/* get next record plan start time */
    	if(check_plan_continuity(record_ctrl.plan.index, 
    		record_ctrl.plan.start_wday) < 0){
    		ak_print_normal_ex("\n\t we will stop the record plan[%d]\n", 
    			record_ctrl.plan.index);
			ak_print_normal_ex("\t cur_time: %s\n", secs_to_readable_string(cur_secs));
			ak_print_normal_ex("\t end_time: %s\n", 
				secs_to_readable_string(record_ctrl.plan.end_time));
	        record_ctrl.plan.status = RECORD_PLAN_END;
	        record_ctrl_stop_record();
    	}
    }
}

static int check_record_exception(void)
{
	if(SD_STATUS_CARD_INSERT == ak_sd_check_insert_status()){
		if((record_ctrl.except_type & RECORD_EXCEPT_SD_REMOVED)){
			ak_print_normal_ex("--- SD Card Inserted ---\n");
		}
		record_ctrl.except_type &= (~RECORD_EXCEPT_SD_REMOVED);
		if(AK_SUCCESS == ak_sd_check_mount_status()){
	    	if(record_ctrl.except_type & RECORD_EXCEPT_SD_UMOUNT){
				ak_print_normal_ex("--- SD Card mounted ---\n");
				record_file_get_pre_record();
			}
	        record_ctrl.except_type &= (~RECORD_EXCEPT_SD_UMOUNT);
	    }else{
	    	record_ctrl.except_type |= RECORD_EXCEPT_SD_UMOUNT;
	    }
	}else{
		if(0 == (record_ctrl.except_type & RECORD_EXCEPT_SD_REMOVED)){
			ak_print_normal_ex("--- SD Card has removed ---\n\n");
		}
		record_ctrl.except_type |= RECORD_EXCEPT_SD_REMOVED;
	}
    
    return record_ctrl.except_type;

}
#endif 
static unsigned int g_start_time =0 ;
static void check_plan_record_start(void)
{
    
	if (record_ctrl.plan.status == RECORD_PLAN_START)
	    return ;

    g_start_time = get_tick_count();

   
	record_ctrl.plan.index = 0;
	record_ctrl.plan.start_wday = 0;
	record_ctrl.plan.end_time = 120;

	record_ctrl.plan.status = RECORD_PLAN_START; 
	
    ak_dvr_record_start();
    
    ak_print_notice("check_plan_record_start\n");
}

static void check_plan_record_stop(void)
{
    unsigned int tick =0 ;

    tick = get_tick_count();

    // 非循环录像，需要停止
    if (!g_video_config.save_cyc_flag )
    {
        if (tick - g_start_time >=  g_video_config.saveTime * 60 * 1000)
        {
            record_ctrl_exit();
            ak_print_notice("record_ctrl_stop_record\n");
            
        }
    }

}
static int check_record_exception(void)
{
    
    record_ctrl.except_type = RECORD_EXCEPT_NONE ;
    return 0;
}


/* check_record_plan - check and change record plan status */
static int check_record_plan(void)
{
    switch(record_ctrl.plan.status){
    case RECORD_PLAN_NONE:
    case RECORD_PLAN_END:
    	check_plan_record_start();
        break;
    case RECORD_PLAN_START:
    	check_plan_record_stop();
        break;
    default:
        record_ctrl.plan.status = RECORD_PLAN_NONE;
        break;
    }
   
    return record_ctrl.plan.status;
}

unsigned char record_ctrl_is_recording(void)
{
	if(record_ctrl.run_flag){
		if(RECORD_PLAN_START == record_ctrl.plan.status){
			return AK_TRUE;
		}
	}
	
	return AK_FALSE;
}

void record_ctrl_pause_record(void)
{
    ak_dvr_record_pause();
}

void record_ctrl_resume_record(int trigger_type)
{
    ak_dvr_record_resume(trigger_type);
}

/**
 * record_ctrl_stop_record: stop the current record
 * @void: 
 * return: none
 * notes: stop condition: 
 *		1. any kind of exceptions, ex.: RECORD_EXCEPT_SD_NO_SPACE
 *		2. plan record time is out
 */
void record_ctrl_stop_record(void)
{
	ak_print_normal_ex("record_ctrl.except_type=0x%X\n", record_ctrl.except_type);
	//video_record_stop();
    ak_dvr_record_stop();

    //record_file_stop();
    record_file_stop_search();
    
    ak_venc_close(record_ctrl.venc_handle);
    ak_vi_close(record_ctrl.vi_handle);
    
    ak_aenc_close(record_ctrl.aenc_handle);
    ak_ai_close(record_ctrl.ai_handle);


    record_ctrl.run_flag =false;

    record_ctrl.plan.status = RECORD_PLAN_NONE;
    /* after stop record, we clear current exception type */
    record_ctrl.except_type = RECORD_EXCEPT_NONE;
}

/** record_ctrl_main
 * @arg[IN]: thread input arg
 * return: none
 * notes: 
 */
static void* record_ctrl_main(void *arg)
{
//    pthread_detach(pthread_self()); //lgd remove
    ak_print_normal_ex("thread id : %ld\n", ak_thread_get_tid());

    while(record_ctrl.run_flag){
        check_record_exception();
		if(RECORD_EXCEPT_NONE == record_ctrl.except_type){
			check_record_plan();
		}else{
			/* we have aleady started record, then stop it */
			if(RECORD_PLAN_START == record_ctrl.plan.status){
				record_ctrl_stop_record();
			}
			report_exception(record_ctrl.except_type);
		}
	    
	    ak_sleep_ms(300);
//	    cmd_utils_idle(1,"");
//	    cmd_utils_free();

//        int use,buf;
//        FS_GetAsynBufInfo(&use,&buf);
//        ak_print_normal_ex("\nuse=%d,buf=%d\n",use,buf);
        
	}

    ak_sleep_ms(200);
	ak_print_normal_ex("\t### record_ctrl_main thread exit ###\n");
	
	ak_thread_exit();
	return NULL;
} 

/** 
 * record_ctrl_init - init record control env and other sub-modules. 
 *      start record control thread.
 * @vi_handle[IN]: success opened video input handle
 * @ai_handle[IN]: success opened audio input handle
 * @file_type[IN]: record file type
 * return: none
 */
ak_pthread_t record_ctrl_init(void *vi_handle, void *ai_handle,
					enum record_file_type file_type , struct video_config * video_config)
{
	if(record_ctrl.run_flag){
		ak_print_normal_ex("already inited...\n");
		return;
	}

    g_video_config = * video_config;

    ak_print_normal("\n---------------------------------------------------------"
        "-----------------------\n");
    
    switch(file_type) {
    case RECORD_FILE_TYPE_MP4:
    	ak_print_notice_ex("^^^^^ record file type is mp4 ^^^^^\n");
    	break;
    case RECORD_FILE_TYPE_AVI:
    	ak_print_notice_ex("^^^^^ record file type is avi ^^^^^\n");
    	break;
    default:
    	ak_print_error_ex("^^^^^ unsupport record file_type=%d ^^^^^\n", file_type);
    	return;
    }
            
    record_ctrl.run_flag = AK_TRUE;
    record_ctrl.alarm_init_flag = AK_FALSE;
    record_ctrl.except_type = (RECORD_EXCEPT_SD_REMOVED | RECORD_EXCEPT_SD_UMOUNT);
//	record_ctrl.detect_type = SYS_DETECT_TYPE_NUM; //lgd remove

	/* plan record info */
    record_ctrl.plan.status = RECORD_PLAN_NONE;
    record_ctrl.plan.index = INVALID_VALUE;
	record_ctrl.plan.start_wday = INVALID_VALUE;
    record_ctrl.plan.start_time = 0;
    record_ctrl.plan.end_time = 0;
    record_ctrl.plan.next_time = 0;

	int ret = AK_FAILED;
	record_ctrl.vi_handle = vi_handle;
	record_ctrl.ai_handle = ai_handle;
	
	init_record_common(file_type);
    ak_pthread_t record_ctrl_thread;
    ret = ak_thread_create(&record_ctrl_thread, record_ctrl_main,
        NULL, ANYKA_THREAD_MIN_STACK_SIZE, 95);
	if(AK_FAILED == ret){
		ak_print_normal_ex("unable to create record_ctrl_main thread, ret = %d!\n", ret);
		record_ctrl.run_flag = AK_FALSE;
	}

	return record_ctrl_thread;
}

/** record_ctrl_exit: clear record control env and other sub-modules. 
 *      stop record control thread and release resource.
 * @void
 * return: none
 */
void record_ctrl_exit(void)
{
//	ak_print_normal("\t ***** record_ctrl_exit *****\n");
	if(record_ctrl.run_flag){
		ak_dvr_record_exit();

	    record_ctrl.run_flag = AK_FALSE;
	    record_ctrl.plan.status = RECORD_PLAN_END;
	    ak_sleep_ms(300);

//	    ak_print_normal_ex("record ctrl has stopped!\n");
	}
}
