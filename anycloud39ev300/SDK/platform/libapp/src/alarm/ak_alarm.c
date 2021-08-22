#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "internal_error.h"

#include "ak_error.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_md.h"
#include "ak_aed.h"
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
static const char alarm_version[] = "libapp_alarm V1.0.00";
static int md_trigger_time = 0;

const char *ak_alarm_get_version(void)
{
	return alarm_version;
}

static void aed_callback_bottom(void)
{
	int index = SYS_DETECT_VOICE_ALARM;

	if (alarm_ctrl[index].send_alarm) {
		struct ak_timeval tv;
		ak_get_ostime(&tv);
		alarm_ctrl[index].send_alarm(index, alarm_ctrl[index].level, tv.sec, 1);
	}
}

static void aed_callback_top(unsigned long long trigger_ms)
{
	int index = SYS_DETECT_VOICE_ALARM;
	static unsigned long long last_trigger_ms = 0;

	/* if no need to filter, send alarm message to app */
	if ((!alarm_ctrl[index].filter) || (0 == alarm_ctrl[index].filter())) {
		/* event have been triggered, but get it delay. judge it again*/
		if (trigger_ms >= (last_trigger_ms + alarm_report_interval)) {
			last_trigger_ms = trigger_ms;
			aed_callback_bottom();
		}
	}
}

static void *ak_aed_thread(void *arg)
{
	long int tid = ak_thread_get_tid();
	ak_print_normal_ex("thread id: %ld\n", tid);
	ak_thread_set_name("app_aed_th");

	while (alarm_ctrl[SYS_DETECT_VOICE_ALARM].run_flag) {
		unsigned long long trigger_ms = 0;
		/* get result */
		int result = ak_aed_get_result(&trigger_ms);
		if (result)
			aed_callback_top(trigger_ms);
		/* schedule a interval */
		ak_sleep_ms(alarm_detect_interval);
	}

	ak_thread_exit();
	return NULL;
}

/*
 * audio event detection
 */
static int alarm_start_aed(int level)
{
	int threshold = 15;
	int index = SYS_DETECT_VOICE_ALARM;
	int ret = AK_FAILED;

	if (level < 1 || level > 3) {
		ak_print_warning_ex("level=%d\n", level);
		level = 1;
	}

	alarm_ctrl[index].level = level;
	threshold = alarm_ctrl[index].ratios[level - 1];

	ak_print_notice_ex("aed level: %d, threshold: %d\n", level, threshold);
	/* init aed module */
	ret = ak_aed_init(alarm_ctrl[index].handle);
	if (!ret) {
		/* set aed param */
		struct ak_aed_param param = {0};
		param.interval = alarm_detect_interval;
		param.threshold = threshold;
		ak_aed_set_param(&param);

		/* enable aed */
		ret = ak_aed_enable(AK_TRUE);
		if (!ret) {
			alarm_ctrl[index].run_flag = AK_TRUE;
			/* create app's aed thread */
			ak_thread_create(&alarm_ctrl[index].tid, ak_aed_thread, NULL,
					ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
		}
	} else if (ERROR_TYPE_EBUSY == ak_get_error_no()) {
		/* if already open, set new param */
		ak_print_normal_ex("aed is opened, just change param\n");
		/* set aed param */
		struct ak_aed_param param = {0};
		param.interval = alarm_detect_interval;
		param.threshold = threshold;
		ak_aed_set_param(&param);

		ret = AK_SUCCESS;
	} else
		ak_print_error_ex("aed open failed\n");

	return ret;
}

static int alarm_stop_aed(void)
{
	int ret = AK_FAILED;
	int index = SYS_DETECT_VOICE_ALARM;

	ak_print_info_ex("entry ...\n");

	if(alarm_ctrl[index].run_flag){
		alarm_ctrl[index].run_flag = AK_FALSE;
		/* exit inner aed module */
		ret = ak_aed_exit();
		/* exit app's aed thread */
		ak_thread_join(alarm_ctrl[index].tid);
		ak_print_normal_ex("exit, %s\n", ret ? "failed" : "success");
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
	int result = 0;
	int md_time_bak = 0;			/*calendar time ,second*/
	struct ak_timeval cur_cpu_time = {0};
	struct ak_timeval md_cpu_time = {0};	/*cpu time, second*/

	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());

	do {
		ak_get_ostime(&cur_cpu_time);
		diff_time = ak_diff_ms_time(&cur_cpu_time, &md_cpu_time);
		result = ak_md_get_result(&md_time, NULL, 0);

		if((1 == result)
			&& (((alarm_ctrl[SYS_DETECT_MOVE_ALARM].filter)
					&& (0 == alarm_ctrl[SYS_DETECT_MOVE_ALARM].filter()))
				|| (NULL == alarm_ctrl[SYS_DETECT_MOVE_ALARM].filter))){
			md_trigger_time = md_time;
			ak_print_info_ex("md time: %d(s)\n",md_trigger_time);
		}

		/*care for cpu time && calendar time*/
		if((diff_time > (alarm_report_interval * 1000 - 4000))
			&& (1 == result)){
#if AK_ALARM_DBG
			ak_print_info_ex("get md, time=%d(s)\n", md_time);

			struct ak_date date;
			ak_seconds_to_date(md_time, &date);
			ak_print_date(&date);

			if((alarm_ctrl[SYS_DETECT_MOVE_ALARM].filter)
				&& (1 == alarm_ctrl[SYS_DETECT_MOVE_ALARM].filter())) {
				ak_print_normal_ex("filter md\n");
			}

			ak_print_info("md_time=%d, md_time_bak=%d, alarm_report_interval=%d\n",
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

	if (AK_SUCCESS != ak_md_set_fps(fps)) {
		ak_print_error_ex("fail\n");
	}
	if (AK_SUCCESS != ak_md_set_global_sensitivity(ratios)) {
		ak_print_error_ex("fail\n");
	}
	if ((0 == alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag) &&
		(AK_SUCCESS != ak_md_enable(1))) {
		ak_print_error_ex("fail\n");
	}

	return AK_SUCCESS;
}

static int alarm_start_md(int level)
{
	int ratios = 100;

	if (level < 1 || level > 3){
		ak_print_error_ex("level:%d\n",level);
		level = 1;
	}

	if ((0 == alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag)
		&& (ak_md_init(alarm_ctrl[SYS_DETECT_MOVE_ALARM].handle) != 0)) {
		ak_print_error_ex("fail\n");
		return AK_FAILED;
	}

	alarm_ctrl[SYS_DETECT_MOVE_ALARM].level = level;
	ratios = alarm_ctrl[SYS_DETECT_MOVE_ALARM].ratios[level - 1];
	if (ratios > 100 ||  ratios < 1)
		ratios = 100;

	if (md_set_param(ratios, alarm_detect_interval) < 0) {
		return AK_FAILED;
	}

	if (0 == alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag){
		alarm_ctrl[SYS_DETECT_MOVE_ALARM].run_flag = 1;
		ak_thread_create(&alarm_ctrl[SYS_DETECT_MOVE_ALARM].tid, md_alarm_thread,
			NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
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

/**
 * ak_alarm_set_interval_time - set report_interval && detect_interval for  ak alarm
 * @report_interval[IN]: 		second
 * @detect_interval[IN]: 		msecond
 * return:   successful return 0 ;  fail return -1
 */
int ak_alarm_set_interval_time(int report_interval,int detect_interval)
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

/**
 * ak_alarm_set_ratios - set one type of ak alarm sensitivity
 * @type[IN]: alarm type to set
 * @cur_level[IN]: current level
 * @total_level[IN]: total level of sensitivity
 * @ratios[IN]:  sensitivity value
 * return:  0 success;  -1 failed
 */
int ak_alarm_set_ratios(enum sys_detect_type type, int cur_level,
						int total_level, int *ratios)
{
	int *p_ratios = ratios;

	ak_print_normal_ex("type:%d level:%d\n",type,cur_level);
	if (((SYS_DETECT_MOVE_ALARM != type) && (SYS_DETECT_VOICE_ALARM != type))
		|| (total_level < 1) || (total_level > 3) || (!ratios)
		|| (cur_level < 0) || (cur_level > total_level)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n");
		return AK_FAILED;
	}

	for (int i = 0; i < total_level; i++) {
		alarm_ctrl[type].ratios[i] = *p_ratios;
		p_ratios ++;
	}
	alarm_ctrl[type].level = cur_level;

	return AK_SUCCESS;
}

/**
 * ak_alarm_stop - stop one type of ak alarm
 * @type[IN]: 	alarm type to stop
 * return:   successful return 0 ;  fail return -1
 */
int ak_alarm_stop(enum sys_detect_type type)
{
	int ret = AK_SUCCESS;

	switch (type) {
	case SYS_DETECT_MOVE_ALARM:
		ret = alarm_stop_md();
		ak_print_normal_ex("move alarm, ret=%d\n\n", ret);
		break;
	case SYS_DETECT_VOICE_ALARM:
		ret = alarm_stop_aed();
		ak_print_normal_ex("voice alarm, ret=%d\n\n", ret);
		break;
	default:
		ak_print_error_ex("Invalid argument.\n");
		ret = AK_FAILED;
		break;
	}

	return ret;
}

/**
 * ak_alarm_start - start one type of ak alarm
 * @type[IN]: 	alarm type to start
 * @level[IN]: 	alarm sensitivity
 * return:   successful return 0 ;  fail return -1
 */
int ak_alarm_start(enum sys_detect_type type, int level)
{
	if (((type != SYS_DETECT_MOVE_ALARM) && (type != SYS_DETECT_VOICE_ALARM))
		   	|| (level < 1) || (level > 3)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n");
		return AK_FAILED;
	}

	int ret = 0;

	ak_print_normal_ex("type=%d level=%d\n", type, level);
	if (SYS_DETECT_MOVE_ALARM == type)
		ret = alarm_start_md(level);
	else if (SYS_DETECT_VOICE_ALARM == type)
		ret = alarm_start_aed(level);

	return ret;
}

/**
 * ak_alarm_get_time - get md time of  alarm
 * return:    md time of  alarm , 0 -> no alarm
 */
int ak_alarm_get_time(void)
{
	int tmp_time = md_trigger_time;
	md_trigger_time = 0;
	return tmp_time;
}

/**
 * ak_alarm_init - init  one type of ak alarm
 * @type[IN]: 		alarm type to init
 * @handle[IN]: 		vi or ai handle
 * @alarm_func[IN]: 	send alarm info to ak
 * @filter_func[IN]: 	check alarm info to send or to discard
 * return:   successful return 0 ;  fail return -1
 */
int ak_alarm_init(enum sys_detect_type type, void *handle,
	AK_ALARM_CB alarm_func, AK_ALARM_FILTER_CB filter_func)
{
	int level = 0, ret = AK_SUCCESS;

 	if ((type != SYS_DETECT_MOVE_ALARM) && (type != SYS_DETECT_VOICE_ALARM)) {
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
		ret = ak_alarm_start(type, level);

	return ret;
}
