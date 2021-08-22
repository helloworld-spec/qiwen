#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_drv_ir.h"
#include "ak_drv_irled.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_misc.h"

#ifdef CONFIG_RTSP_SUPPORT
#include "ak_rtsp.h"
#endif

#ifdef CONFIG_DANA_SUPPORT
#include "ak_dana.h"
#endif

#define AK_IPC_START_TIME	"/tmp/ak_ipc_start_time"
//#define IR_LED_FILE_NAME	"/sys/user-gpio/gpio-rf_feed"
#define IR_LED_FILE_NAME	"/sys/user-gpio/gpio-light_ctrl"

struct ak_misc {
    int ircut_run_flag;	    //photosensitive and ircut switch run flag
    int ps_switch_enable;   //store photosensitive switch status
    int day_level_ctrl;		//day level default led-ircut config
    int ipc_run_flag;		//ipc run flag
    int pre_status;			//current state,0 night, 1 day
    void *vi_handle;		//global vi handle
    ak_pthread_t ircut_tid;
	int cur_set_mode;
	int thread_run;
	enum ak_photosensitive_mode ps_mode;
	thread_func ps_fun_thread;// //photosensitive thread function
};


static struct ak_misc misc_ctrl = {0};
static const char misc_version[] = "app_misc V2.3.08";

static int switch_day_or_night(enum mode_state mode)
{
	ak_print_normal_ex("(switch day_night)cur_status=%d\n", mode);
	/* disable md */
	
	int ret = ak_misc_set_video_day_night(misc_ctrl.vi_handle, mode,
								misc_ctrl.day_level_ctrl);
	misc_ctrl.pre_status = mode;	/* store new state */
	ak_sleep_ms(3000);
	/* enable md */
	return ret;
}

/*
 * photosensitive_switch_th - according to photosensitive status
 *                            to switch ircut and video
 * notice: not suggest to use
 */
static void *photosensitive_switch_th(void *arg)
{
	long int tid = ak_thread_get_tid();
	int ir_val = 0;

	ak_print_normal_ex("Thread start, id: %ld\n", tid);
	ak_thread_set_name("PS_switch");

	misc_ctrl.ps_switch_enable = AK_TRUE;
	/* get ir state and switch day-night */
	while (misc_ctrl.ircut_run_flag) {
		if (misc_ctrl.ps_switch_enable) {
			ir_val = ak_drv_ir_get_input_level();
			if (ir_val != -1 && misc_ctrl.pre_status != ir_val) {
		        ak_print_normal_ex("prev_state=%d, ir_val=%d\n",
			        misc_ctrl.pre_status, ir_val);
				ak_misc_set_video_day_night(misc_ctrl.vi_handle, ir_val,
				    misc_ctrl.day_level_ctrl);
				misc_ctrl.pre_status = ir_val;
			}
		}
		ak_sleep_ms(5000);
	}

	ak_print_normal_ex("Thread exit, id: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

/*
 * photosensitive_switch_th - according to photosensitive status
 *                            to switch ircut and video
 */
static void *photosensitive_switch_th_ex(void *arg)
{
	long int tid = ak_thread_get_tid();
	int ir_val = 0;

	ak_print_normal_ex("Thread start, id: %ld\n", tid);
	ak_thread_set_name("PS_switch");

	misc_ctrl.thread_run = AK_TRUE;
	/* get ir state and switch day-night */
	while (misc_ctrl.thread_run) {		
		ak_print_info_ex("misc_ctrl.cur_set_mode=%d\n", misc_ctrl.cur_set_mode);
		if (misc_ctrl.cur_set_mode != SET_AUTO_MODE)
			break;

		ir_val = ak_drv_ir_get_input_level();
		
		if (ir_val != -1 && misc_ctrl.pre_status != ir_val) {
	        ak_print_info_ex("prev_state=%d, ir_val=%d\n",
		        misc_ctrl.pre_status, ir_val);
			ak_misc_set_video_day_night(misc_ctrl.vi_handle, ir_val,
			    misc_ctrl.day_level_ctrl);
			misc_ctrl.pre_status = ir_val;
		}
		
		ak_sleep_ms(5000);
	}

	ak_print_normal_ex("Thread exit, id: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}


/*
 * ir_auto_switch_th - switch ircut and led by auto
 */
static void *ir_auto_switch_th(void *arg)
{
	long int tid = ak_thread_get_tid();
	int cur_status = -1;

	ak_print_normal_ex("Thread start, id: %ld\n", tid);
	ak_thread_set_name("PS_switch");

	ak_print_normal_ex("(switch day_night) cur_set_mode=%d\n", misc_ctrl.cur_set_mode);

	misc_ctrl.thread_run = AK_TRUE;

	/* get ir state and switch day-night */
	while (misc_ctrl.thread_run) {
		if (misc_ctrl.cur_set_mode != SET_AUTO_MODE)
			break;
		
		/* check day night mode */
		cur_status = ak_vpss_isp_get_auto_day_night_level(misc_ctrl.pre_status);
		if (AK_FAILED == cur_status) {
			ak_print_error_ex("ak_vpss_isp_get_input_level failed: %d\n", cur_status);
			continue;
		}
		
		if (misc_ctrl.pre_status != cur_status) {
			if (STATE_DAY == cur_status )
				switch_day_or_night(STATE_DAY);
			else
				switch_day_or_night(STATE_NIGHT);
		}
		ak_sleep_ms(100);

	}

	ak_print_normal_ex("Thread exit, id: %ld\n", tid);
	ak_thread_exit();
	
	return NULL;
}

/*
 * gop_switch_notify - call all register callback to switch gop
 */
static void gop_switch_notify(void)
{
#ifdef CONFIG_DANA_SUPPORT
    ak_dana_switch_gop();
#endif
}

/**
 * ak_misc_get_version - get misc version
 * return: version string
 */
const char* ak_misc_get_version(void)
{
	return misc_version;
}

/**
 * ak_misc_ipc_first_run: ipc is first run or not
 * return: 1 first run, 0 not first run
 */
int ak_misc_ipc_first_run(void)
{
	if(misc_ctrl.ipc_run_flag)
		return 1;

	/* ipc is not first start up */
	if (access(AK_IPC_START_TIME, W_OK) == 0)
		return 0;

	time_t t;
	char start_time[100] = {0};

	time(&t);
	sprintf(start_time, "ipc start time: %s\n", ctime(&t));

	FILE *fp = fopen(AK_IPC_START_TIME, "a+");
	fwrite(start_time, 1, strlen(start_time), fp);
	fclose(fp);

	misc_ctrl.ipc_run_flag = AK_TRUE;

	return 1;
}

/**
 * ak_misc_set_video_day_night: set video day or night mode, according to IR value
 * @vi_handle: opened vi handle
 * @ir_val: IR value, [0, 1]
 * @day_level: day control level, [1, 4]
 * return: 0 success, -1 failed
 */
int ak_misc_set_video_day_night(void *vi_handle, int ir_val, int day_level)
{
	if (!vi_handle) {
		return AK_FAILED;
	}

	int day_val = 0;
	int ir_switch_val = 0;

	switch (day_level) {
	case DAY_LEVEL_HH:
		ir_switch_val = ir_val;
		day_val = ir_val;
		break;
	case DAY_LEVEL_HL:
		ir_switch_val = !ir_val;
		day_val = ir_val;
		break;
	case DAY_LEVEL_LH:
		ir_switch_val = !ir_val;
		day_val = !ir_val;
		break;
	case DAY_LEVEL_LL:
		ir_switch_val = ir_val;
		day_val = !ir_val;
		break;
	default:
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	if (day_val) {
		ak_print_normal_ex("now set to day\n");
		ak_drv_ir_set_ircut(ir_switch_val);  //set ircut before switch isp config to day
		ret = ak_vi_switch_mode(vi_handle, VI_MODE_DAY);
		ak_drv_irled_set_working_stat(0);
	} else {
		ak_print_normal_ex("now set to night\n");
		ak_drv_irled_set_working_stat(1);
		ret = ak_vi_switch_mode(vi_handle, VI_MODE_NIGHT);
		ak_drv_ir_set_ircut(ir_switch_val);  //set ircut after switch isp config to night
	}

	ak_sleep_ms(300);
	/* to notify av mode to modify gop */
	gop_switch_notify();

	return ret;
}

/**
 * ak_misc_start_photosensitive_switch: start photosensitive switch
 * @day_ctrl: day control level, [1, 4]
 * return: 0 success, -1 failed
 * notice: not suggest to use
 */
int ak_misc_start_photosensitive_switch(int day_ctrl)
{
	if ((day_ctrl > DAY_LEVEL_RESERVED)
		&& (day_ctrl < DAY_LEVEL_MAX)) {
		misc_ctrl.day_level_ctrl = day_ctrl;
	} else {
		return AK_FAILED;
	}

	if (misc_ctrl.ircut_run_flag)
		return AK_SUCCESS;

	misc_ctrl.vi_handle = ak_vi_get_handle(VIDEO_DEV0);
	misc_ctrl.ircut_run_flag = AK_TRUE;
	misc_ctrl.pre_status = -1;

	return ak_thread_create(&misc_ctrl.ircut_tid, photosensitive_switch_th,
			NULL, 100 *1024, -1);
}

/**
 * ak_misc_switch_photosensitive_ircut - photosensitive and ircut switch
 * @enable[IN]: 0 disable, 1 enable
 * return: 0 success, -1 failed
 * notes:
 */
void ak_misc_switch_photosensitive_ircut(int enable)
{
	ak_print_normal_ex("set photosensitive switch %s.\n",
			enable == 1 ? "enable" : "disable");
	misc_ctrl.ps_switch_enable = enable;
}

/**
 * ak_misc_stop_photosensitive_switch: stop photosensitive switch
 * notice: not suggest to use
 */
void ak_misc_stop_photosensitive_switch(void)
{
	misc_ctrl.ircut_run_flag = AK_FALSE;
	ak_print_normal_ex("set pircut_th_runflag to 0\n");

    ak_print_notice_ex("join photosensitive switch thread...\n");
	ak_thread_join(misc_ctrl.ircut_tid);
	ak_print_notice_ex("photosensitive switch thread join OK\n");
}

/**
 * ak_misc_start_auto_ir_switch: start ircut auto switch
 * @day_ctrl: day control level, [1, 4]
 * @config: auto day night switch config
 * return: 0 success, -1 failed 
 * notice: not suggest to use
 */
int ak_misc_start_auto_ir_switch(int day_ctrl, 
								struct auto_day_night_switch *config)
{
	if (!config) {
		ak_print_error_ex("config is NULL\n");
		return AK_FAILED;
	}
	
	if ((day_ctrl > DAY_LEVEL_RESERVED)
		&& (day_ctrl < DAY_LEVEL_MAX)) {
		misc_ctrl.day_level_ctrl = day_ctrl;
	} else {
		return AK_FAILED;
	}

	if (misc_ctrl.ircut_run_flag) {
		ak_print_error_ex("misc already start\n");
		return AK_SUCCESS;
	}
	misc_ctrl.vi_handle = ak_vi_get_handle(VIDEO_DEV0);
	misc_ctrl.ircut_run_flag = AK_TRUE;
	
	misc_ctrl.cur_set_mode = config->day_night_mode;

	int ret = AK_SUCCESS;
		
	if (misc_ctrl.cur_set_mode == SET_DAY_MODE) { 			// day
		misc_ctrl.pre_status = STATE_DAY;
		misc_ctrl.cur_set_mode = SET_DAY_MODE;
		switch_day_or_night(STATE_DAY);
	} else if (misc_ctrl.cur_set_mode == SET_NIGHT_MODE) {	//night
		misc_ctrl.pre_status = STATE_NIGHT;
		misc_ctrl.cur_set_mode = SET_NIGHT_MODE;
		switch_day_or_night(STATE_NIGHT);
	} else if (misc_ctrl.cur_set_mode == SET_AUTO_MODE) {
		misc_ctrl.pre_status = STATE_DAY;
		misc_ctrl.cur_set_mode = SET_AUTO_MODE;
		switch_day_or_night(STATE_DAY);	
		ret = ak_thread_create(&misc_ctrl.ircut_tid, ir_auto_switch_th,
			NULL, 100 *1024, -1);
	}
	return ret;
}

/**
 * ak_misc_auto_ir_set_status: set ircut switch mode
 * @mode: day/night/auto mode
 * return: 0 success, -1 failed
 * notice: not suggest to use
 */
int ak_misc_auto_ir_set_status(enum day_night_switch_mode mode)
{
	if (SET_NIGHT_MODE > mode || SET_AUTO_MODE < mode) {
		ak_print_error_ex("mode is not correct\n");
		return AK_FAILED;
	}

	if (AK_FALSE == misc_ctrl.ircut_run_flag) {
		ak_print_error_ex("thread is not start\n");
		return AK_FAILED;
	}

	if (misc_ctrl.cur_set_mode == mode) {
		return AK_SUCCESS;
	}

	int ret = AK_FAILED;

	if (mode == SET_AUTO_MODE) { // day or night status change to auto mode
		if (misc_ctrl.pre_status != STATE_DAY) { // now is not day,change to day
			misc_ctrl.pre_status = STATE_DAY;
			switch_day_or_night(STATE_DAY);	
		}
		misc_ctrl.cur_set_mode = SET_AUTO_MODE;
		ret = ak_thread_create(&misc_ctrl.ircut_tid, ir_auto_switch_th,
			NULL, 100 *1024, -1);
		
	} else { // change to day or night mode
		misc_ctrl.thread_run = AK_FALSE;
		ak_print_normal_ex("misc_ctrl.cur_set_mode=%d\n", misc_ctrl.cur_set_mode);
		if (misc_ctrl.cur_set_mode == SET_AUTO_MODE) { 
			/*  auto mode ->day mode or night mode 	*/
			ak_print_normal_ex("join ir_auto_switch_th switch thread...\n");
			ak_thread_join(misc_ctrl.ircut_tid);	
			ak_print_normal_ex("join ir_auto_switch_th success\n");	
		} 
		misc_ctrl.cur_set_mode = mode;
		if (mode == SET_CLOSE_MODE) {
			ak_print_warning_ex("change to close mode\n");
			return AK_SUCCESS;
		}	
		int status = (mode == SET_DAY_MODE ? STATE_DAY : STATE_NIGHT);
		switch_day_or_night(status);
		misc_ctrl.pre_status = status;	
		ret = AK_SUCCESS;
	}

	return ret;
}

/**
 * ak_misc_stop_auto_ir_switch - stop ircut auto switch
 * return: 0 success, -1 failed
 * notice: not suggest to use
 */
void ak_misc_stop_auto_ir_switch(void)
{
	misc_ctrl.ircut_run_flag = AK_FALSE;
	misc_ctrl.thread_run = AK_FALSE;
	ak_print_normal_ex("set pircut_th_runflag to 0\n");

	if (misc_ctrl.cur_set_mode == SET_AUTO_MODE) {
	    ak_print_normal_ex("join photosensitive switch thread...\n");
		ak_thread_join(misc_ctrl.ircut_tid);
		ak_print_normal_ex("photosensitive switch thread join OK\n");
	}
}

/**
 * ak_misc_start_photosensitive_switch_ex: start photosensitive switch
 * @ps_mode: photosensitive mode
 * @day_ctrl: day control level, [1, 4]
 * @day_night_mode: day night switch mode
 * return: 0 success, -1 failed
 */
int ak_misc_start_photosensitive_switch_ex(enum ak_photosensitive_mode ps_mode, 
							int day_ctrl, enum day_night_switch_mode day_night_mode)
{
	if ((day_ctrl > DAY_LEVEL_RESERVED)
		&& (day_ctrl < DAY_LEVEL_MAX)) {
		misc_ctrl.day_level_ctrl = day_ctrl;
	} else {
		return AK_FAILED;
	}

	if (misc_ctrl.ircut_run_flag) {
		ak_print_error_ex("misc already start\n");
		return AK_SUCCESS;
	}
	misc_ctrl.vi_handle = ak_vi_get_handle(VIDEO_DEV0);
	misc_ctrl.ircut_run_flag = AK_TRUE;
	
	misc_ctrl.cur_set_mode = day_night_mode;
	misc_ctrl.ps_mode = ps_mode;

	int ret = AK_SUCCESS;
		
	if (misc_ctrl.cur_set_mode == SET_DAY_MODE) { 			// day
		misc_ctrl.pre_status = STATE_DAY;
		misc_ctrl.cur_set_mode = SET_DAY_MODE;
		switch_day_or_night(STATE_DAY);
	} else if (misc_ctrl.cur_set_mode == SET_NIGHT_MODE) {	//night
		misc_ctrl.pre_status = STATE_NIGHT;
		misc_ctrl.cur_set_mode = SET_NIGHT_MODE;
		switch_day_or_night(STATE_NIGHT);
	} else if (misc_ctrl.cur_set_mode == SET_AUTO_MODE) {
		misc_ctrl.pre_status = STATE_DAY;
		misc_ctrl.cur_set_mode = SET_AUTO_MODE;
		switch_day_or_night(STATE_DAY);	
		if (NULL == misc_ctrl.ps_fun_thread) {
			if (ps_mode == HARDWARE_PHOTOSENSITIVE)
				misc_ctrl.ps_fun_thread = photosensitive_switch_th_ex;
			else
				misc_ctrl.ps_fun_thread = ir_auto_switch_th;
		}
		ret = ak_thread_create(&misc_ctrl.ircut_tid, misc_ctrl.ps_fun_thread,
			NULL, 100 *1024, -1);
	}
	return ret;
}

/**
 * ak_misc_photosensitive_set_status_ex: set ircut switch mode
 * @mode: day/night/auto mode
 * return: 0 success, -1 failed
 */
int ak_misc_photosensitive_set_status_ex(enum day_night_switch_mode mode)
{
	if (SET_NIGHT_MODE > mode || SET_AUTO_MODE < mode) {
		ak_print_error_ex("mode is not correct\n");
		return AK_FAILED;
	}

	if (AK_FALSE == misc_ctrl.ircut_run_flag) {
		ak_print_error_ex("thread is not start\n");
		return AK_FAILED;
	}

	if (misc_ctrl.cur_set_mode == mode) {
		return AK_SUCCESS;
	}

	int ret = AK_FAILED;

	if (mode == SET_AUTO_MODE) { // day or night status change to auto mode
		if (misc_ctrl.pre_status != STATE_DAY) { // now is not day,change to day
			misc_ctrl.pre_status = STATE_DAY;
			switch_day_or_night(STATE_DAY);	
		}
		misc_ctrl.cur_set_mode = SET_AUTO_MODE;
		if (NULL == misc_ctrl.ps_fun_thread) {
			if (misc_ctrl.ps_mode == HARDWARE_PHOTOSENSITIVE)
				misc_ctrl.ps_fun_thread = photosensitive_switch_th_ex;
			else
				misc_ctrl.ps_fun_thread = ir_auto_switch_th;
		}
		ret = ak_thread_create(&misc_ctrl.ircut_tid, misc_ctrl.ps_fun_thread,
			NULL, 100 *1024, -1);
		
	} else { // change to day or night mode
		misc_ctrl.thread_run = AK_FALSE;
		ak_print_normal_ex("misc_ctrl.cur_set_mode=%d\n", misc_ctrl.cur_set_mode);
		if (misc_ctrl.cur_set_mode == SET_AUTO_MODE) { 
			/*  auto mode ->day mode or night mode 	*/
			ak_print_normal_ex("join ir_auto_switch_th switch thread...\n");
			ak_thread_join(misc_ctrl.ircut_tid);	
			ak_print_normal_ex("join ir_auto_switch_th success\n");	
		} 
		misc_ctrl.cur_set_mode = mode;
		if (mode == SET_CLOSE_MODE) {
			ak_print_warning_ex("change to close mode\n");
			return AK_SUCCESS;
		}
		
		int status = (mode == SET_DAY_MODE ? STATE_DAY : STATE_NIGHT);
		switch_day_or_night(status);
		misc_ctrl.pre_status = status;	
		ret = AK_SUCCESS;
	}

	return ret;
}

/**
 * ak_misc_stop_photosensitive_switch_ex - stop ircut auto switch
 * return: 0 success, -1 failed
 * notes:
 */
void ak_misc_stop_photosensitive_switch_ex(void)
{
	misc_ctrl.ircut_run_flag = AK_FALSE;
	misc_ctrl.thread_run = AK_FALSE;
	ak_print_normal_ex("set pircut_th_runflag to 0\n");

	if (misc_ctrl.cur_set_mode == SET_AUTO_MODE) {
	    ak_print_normal_ex("join photosensitive switch thread...\n");
		ak_thread_join(misc_ctrl.ircut_tid);
		ak_print_normal_ex("photosensitive switch thread join OK\n");
	}
}


