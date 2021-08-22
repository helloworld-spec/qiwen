#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "internal_error.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_md.h"
#include "ak_alarm.h"

#define AK_ALARM_DBG 0
#define DEFAULT_ALARM_REPORT_INTERVAL 60 //second
#define DEFAULT_ALARM_DETECT_INTERVAL	500 //msecond


struct alarm_ctrl {
	unsigned char run_flag;		 
	void * handle;                      //video/audio handle	
    //pthread_mutex_t mutex;	                //detection mutex
    AK_ALARM_CB send_alarm;         //detection user callback
    AK_ALARM_FILTER_CB filter;  //detection filter
    ak_pthread_t tid;
	int level;
	int ratios[3];
};

static struct alarm_ctrl alarm_ctrl[2] = {{0},{0}}; 
static int alarm_report_interval = 0; 
static int alarm_detect_interval = 0; 
static const char alarm_version[] = "libapp_alarm V2.0.01";

const char *ak_alarm_get_version(void)
{
	ak_print_notice_ex("%s\n", alarm_version);
	return alarm_version;
}

#if 0
static int ak_alarm_audio_voice_data(void *pbuf, int len, int valve)
{
    short *pdata = (T_S16 *)pbuf;
    int count = len / 32;
    int i;
    int average = 0;
    short temp;
    int DCavr = 0;
    static int DCValue = 0;
    static int checkCnt = 0;
    
    //caculte direct_current value
    if (checkCnt < 2)  
    {
        //防止开始录音打压不稳定，开始两帧不参与直流偏移的计算
        checkCnt ++;
    }
    else if (checkCnt <= 20)
    {
    //考虑到硬件速度，只让3到20帧的数据参与直流偏移的计算
        checkCnt ++;
        for (i=0; i<len/2; i++)
        {
            DCavr += pdata[i];
        }
        DCavr /= (signed)(len/2);
        DCavr += DCValue;
        DCValue = (T_S16)(DCavr/2);
    }
    // spot check data value
    for (i = 0; i < count; i++)
    {
        temp = pdata[i*16];
        temp = (T_S16)(temp - DCValue);
        if (temp < 0)
        {
            average += (-temp);
        }
        else
        {
            average += temp;
        }
    }
    average /= count;
    if (average < valve)
    {
        return AK_FALSE;
    }
    else
    {
        return AK_TRUE;
    }
}

static int  get_pcm_data( struct frame *pframe)
{
    static int send_alarm_time = 0;
    int cur_time;    
    if(ak_alarm_audio_voice_data(pframe->buf, pframe->size))
    {        
        ak_get_ostime(&cur_time ,NULL);         
		audio_alarm_ctrl.report_interval ;
			
        if((send_alarm_time + audio_alarm_ctrl.report_interval < cur_time) && (pvideo_move_ctrl->Palarm_func))
         {
            send_alarm_time = cur_time;				               
          	if(alarm_ctrl.detect[SYS_DETECT_VOICE_ALARM].send_alarm){
				alarm_ctrl.detect[SYS_DETECT_VOICE_ALARM].send_alarm(SYS_DETECT_VOICE_ALARM,
			cur_time);
        }
    }
}

static void *voice_detect_thread(void *arg)
{
	struct frame *frame;
	void * handle = alarm_ctrl.detect[SYS_DETECT_VOICE_ALARM].handle;
	long ms;
	ak_print_normal("thread id : %ld\n", ak_thread_get_tid());
	
	while (audio_alarm_ctrl.run_flag) {				
		
		if(0 == ak_ai_get_frame(handle, frame, 0)){
			get_pcm_data(frame);
			ak_ai_release_frame(handle, frame);
		}
		ak_sleep_ms(alarm_detect_interval);
	}
	
	return NULL;
}

static void *sd_thread(void *arg)
{
	int md_time,md_time_bak = 0;		/*calendar time ,second*/
	unsigned int cur_cpu_time,md_cpu_time = 0;/*cpu time ,second*/
	
	ak_print_normal_ex("thread id : %ld\n", ak_thread_get_tid());
	do{
		ak_get_ostime((unsigned long*)&cur_cpu_time,NULL);
		if((cur_cpu_time > (md_cpu_time + alarm_report_interval))){
			ak_print_normal_ex("get md. time:%d\n",md_time); 
			ak_get_ostime((unsigned long*)&md_cpu_time,NULL);
			/*calendar time have been adjust*/
			if(md_time < md_time_bak) md_time_bak = md_time;
			/*sd have been triggered, but get it delay. judge it again*/
			if(md_time >= (md_time_bak + alarm_report_interval)){
				md_time_bak = md_time;				
			}
		}
		ak_sleep_ms(30);
	}while(alarm_ctrl[VOICE_ALARM].run_flag); 
		 	
	return NULL;
}
#endif

static int alarm_start_sd(int level)
{	
	int  ratios=100;

	if(level < 1 || level > 3){
		ak_print_error_ex("level:%d\n",level); 
		level = 1;
	}
	
	alarm_ctrl[SYS_DETECT_VOICE_ALARM].level = level;
	ratios = alarm_ctrl[SYS_DETECT_VOICE_ALARM].ratios[level - 1];           
	if(ratios > 100 ||  ratios < 1) ratios = 100;
	/*
	if(md_set_param(ratios,alarm_detect_interval) < 0){
		return AK_FAILED;
	}*/
	if( 0 == alarm_ctrl[SYS_DETECT_VOICE_ALARM].run_flag){
		alarm_ctrl[SYS_DETECT_VOICE_ALARM].run_flag = 1;
		/*
		ak_thread_create(&alarm_ctrl[VOICE_ALARM].tid, sd_thread,
			NULL, 20*1024, -1);
		*/
	}
	
	return AK_SUCCESS;
}

static int alarm_stop_sd(void)
{
	//int ret = 0;
	
	if(alarm_ctrl[SYS_DETECT_VOICE_ALARM].run_flag){
		alarm_ctrl[SYS_DETECT_VOICE_ALARM].run_flag = AK_FALSE;
		/*
   		ret = ak_thread_join(alarm_ctrl[VOICE_ALARM].tid);
		ak_print_normal_ex("exit, %s\n", ret ? "failed" : "success");	
		*/
	}
	
	return AK_SUCCESS;
}

static void md_callback(void)
{	
	struct ak_timeval tv;
	
	ak_get_ostime(&tv);
	if(alarm_ctrl[SYS_DETECT_MOVE_ALARM].send_alarm){
		alarm_ctrl[SYS_DETECT_MOVE_ALARM].send_alarm(SYS_DETECT_MOVE_ALARM,
			alarm_ctrl[SYS_DETECT_MOVE_ALARM].level, tv.sec, 1);
	}
}

static void* md_alarm_thread(void *arg)
{
	long diff_time = 0;
	int md_time = 0;
	int md_time_bak = 0;			/*calendar time ,second*/
	struct ak_timeval cur_cpu_time = {0};
	struct ak_timeval md_cpu_time = {0};	/*cpu time, second*/
	
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	
	do{
		ak_get_ostime(&cur_cpu_time);
		diff_time = ak_diff_ms_time(&cur_cpu_time, &md_cpu_time);		
		/*care for cpu time && calendar time*/
		if((diff_time > (alarm_report_interval * 1000 - 4000))
			&& ( 1 == ak_md_get_result(&md_time, NULL, 0))){
#if AK_ALARM_DBG
			ak_print_info_ex("get md, time=%d(s)\n", md_time);
			
			struct ak_date date;
			ak_seconds_to_date(md_time, &date);
			ak_print_date(&date);
		    
			if((alarm_ctrl[SYS_DETECT_MOVE_ALARM].filter)
				&& (1 == alarm_ctrl[SYS_DETECT_MOVE_ALARM].filter())) {
				ak_print_normal_ex("filter md.\n");
			}	
				
			ak_print_info("md_time=%d, md_time_bak=%d, alarm_report_interval:%d\n",
					md_time, md_time_bak, alarm_report_interval);
#endif						
			/*calendar time have been adjust*/
			if(md_time < md_time_bak) 
				md_time_bak = md_time;
				
			if(((alarm_ctrl[SYS_DETECT_MOVE_ALARM].filter) && 
				(0 == alarm_ctrl[SYS_DETECT_MOVE_ALARM].filter()))
				||(NULL == alarm_ctrl[SYS_DETECT_MOVE_ALARM].filter)){
				/*md have been triggered, but get it delay. judge it again*/
				if(md_time >= (md_time_bak + alarm_report_interval)){
					md_time_bak = md_time;
					ak_get_ostime(&md_cpu_time);	
					md_callback();	
				}
			}	
		}
		
		ak_sleep_ms(50);
	}while(alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag); 

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());	
	ak_thread_exit();
	return NULL;
}

static int md_set_param(int ratios, int md_interval)
{
	int fps = (md_interval > 1000)? 1 : (1000 / md_interval);	
	
	if(AK_SUCCESS != ak_md_set_fps(fps)){  
		ak_print_error_ex("fail\n"); 
	}
	if(AK_SUCCESS != ak_md_set_global_sensitivity(ratios)){  
		ak_print_error_ex("fail\n"); 
	}
	if(( 0 == alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag) && 
		(AK_SUCCESS != ak_md_enable(1))){
		ak_print_error_ex("fail\n"); 
	}
	
	return AK_SUCCESS;
}

static int alarm_start_md(int level)
{	
	int  ratios=100;

	if(level < 1 || level > 3){
		ak_print_error_ex("level:%d\n",level); 
		level = 1;
	}
	
	if(( 0 == alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag) 
		&& (ak_md_init(alarm_ctrl[SYS_DETECT_MOVE_ALARM].handle) != 0)) {
		ak_print_error_ex("fail\n"); 
		return AK_FAILED;
	}
	
	alarm_ctrl[SYS_DETECT_MOVE_ALARM].level = level;
	ratios = alarm_ctrl[SYS_DETECT_MOVE_ALARM].ratios[level - 1];           
	if(ratios > 100 ||  ratios < 1) 
		ratios = 100;
		
	if(md_set_param(ratios,alarm_detect_interval) < 0){
		return AK_FAILED;
	}
	
	if(0 == alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag){
		alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag = 1;
		ak_thread_create(&alarm_ctrl[SYS_DETECT_MOVE_ALARM].tid, md_alarm_thread,
			NULL, 20*1024, -1);
	}
	
	return AK_SUCCESS;
}

static int alarm_stop_md(void)
{
	int ret = 0;
	
	if(alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag){
		alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag = AK_FALSE;
   		ret = ak_thread_join(alarm_ctrl[SYS_DETECT_MOVE_ALARM].tid);
   		ak_print_notice_ex("md_alarm_thread join OK\n");
		ak_md_enable(0);
		ak_md_destroy();
		alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag = 0;
		ak_print_normal_ex("exit, %s\n", ret ? "failed" : "success");
	}
	
	return AK_SUCCESS;
}

int ak_alarm_interval_time_set(int report_interval,int detect_interval)
{		
	if((report_interval < 0) || (detect_interval < 0)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n"); 
		return AK_FAILED;
	}
	alarm_report_interval = report_interval;
	alarm_detect_interval = detect_interval;
	
	return AK_SUCCESS;
}

int ak_alarm_ratios_set(enum sys_detect_type type,int cur_level,int total_level, int *ratios)
{		
	int *p_ratios = ratios;
	
	ak_print_normal_ex("type:%d level:%d\n",type,cur_level); 
	if((( SYS_DETECT_MOVE_ALARM != type ) && ( SYS_DETECT_VOICE_ALARM != type ))
		|| (total_level < 1) || (total_level > 3) || (!ratios)
		|| (cur_level < 0) || (cur_level > total_level)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n"); 
		return AK_FAILED;
	}
		
	for(int i = 0; i < total_level; i++){
		alarm_ctrl[type].ratios[i] = *p_ratios;
		p_ratios ++;
	}
	alarm_ctrl[type].level = cur_level;
	
	return AK_SUCCESS;
}

int ak_alarm_stop(enum sys_detect_type type)
{
	int ret = AK_SUCCESS;
	
	switch (type) {
	case SYS_DETECT_MOVE_ALARM:
		ret = alarm_stop_md();
		ak_print_normal_ex("move alarm, ret=%d\n\n", ret);
		break;
	case SYS_DETECT_VOICE_ALARM:
		ret = alarm_stop_sd();
		ak_print_normal_ex("voice alarm, ret=%d\n\n", ret);
		break;
	default:
		ak_print_error_ex("Invalid argument.\n"); 
		ret = AK_FAILED;
		break;
	}
	
	return ret;
}

int ak_alarm_start(enum sys_detect_type type,int level)
{	
	int ret = 0;
	if(((type != SYS_DETECT_MOVE_ALARM) && (type != SYS_DETECT_VOICE_ALARM))
		|| (level < 1) || (level > 3)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n"); 
		return AK_FAILED;
	}	
	ak_print_normal_ex("type:%d level:%d\n",type,level); 
	if( SYS_DETECT_MOVE_ALARM == type )
		ret = alarm_start_md(level);
	else if( SYS_DETECT_VOICE_ALARM == type )
		ret = alarm_start_sd(level);
	
	return ret;
}

int ak_alarm_init(enum sys_detect_type type, void *handle,
	AK_ALARM_CB alarm_func, AK_ALARM_FILTER_CB filter_func)
{
	
	int level = 0, ret = AK_SUCCESS;
		
 	if((type != SYS_DETECT_MOVE_ALARM) && (type != SYS_DETECT_VOICE_ALARM)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n"); 
		return AK_FAILED;
	}	
	if(0 == alarm_report_interval)	
		alarm_report_interval = DEFAULT_ALARM_REPORT_INTERVAL;
	if(0 == alarm_detect_interval)
		alarm_detect_interval = DEFAULT_ALARM_DETECT_INTERVAL;
	
	alarm_ctrl[type].run_flag = 0;	
	alarm_ctrl[type].handle = handle;
	alarm_ctrl[type].send_alarm = alarm_func;
    alarm_ctrl[type].filter = filter_func;
		
	level = alarm_ctrl[type].level;
	if(0 != level) 
		ret = ak_alarm_start(type,level);
		
	return ret;
}
