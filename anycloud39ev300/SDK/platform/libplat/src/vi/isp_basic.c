#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <linux/ioctl.h>
#include <linux/videodev2.h>

#include "isp_struct.h"
#include "isp_basic.h"
#include "isp_vi.h"
#include "akuio.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_isp_sdk.h"

#define ISP_MODULE_ID_SIZE		2
#define ISP_MODULE_LEN_SIZE		2
#define ISP_MODULE_HEAD_SIZE	(ISP_MODULE_ID_SIZE + ISP_MODULE_LEN_SIZE)

/*
 * all this struct define, see isp doc to know what its mean.
 */
struct effect_offset {
	int hue;
	int brightness;
	int saturation;
	int contrast;
	int sharp;
    int wdr;
};

enum fps_stat_type {
	LOW_FPS_STAT = 0,
	HIGH_FPS_STAT,
	MID_FPS_STAT,
};

enum gain_stat {
	GAIN_NO_CHANGE_STAT = 0,
	GAIN_LOW_STAT,
	GAIN_HIGH_STAT,
	GAIN_MID_STAT,
};

struct fps_config {
	enum fps_stat_type fps_stat;
	int cur_fps;
	int low_fps;
	int mid_fps;
	int high_fps;
	int low_fps_exp_time;
	int mid_fps_exp_time;
	int high_fps_exp_time;
	int high_fps_to_low_fps_gain;
	int low_fps_to_high_fps_gain;
	int mid_fps_to_low_fps_gain;
	int mid_fps_to_high_fps_gain;
	int fps_num;
	int init;
};

struct fps_switch_info {
	char enable;
	char thread_running;
	ak_sem_t fps_sem;
	ak_pthread_t switch_fps_pth_id;
};

struct ae_stat_info {
	int a_gain;
	int d_gain;
	int isp_d_gain;
	unsigned int exptime;
};

struct ak_isp_resolution {
	int width;
	int height;
};

struct sensor_update_skip {
	int sensor_id;
	int skip_frames;
};

struct ak_isp_frame_rate_attr_ex {
	unsigned char  	fps1;
	unsigned char  	gain11;
	unsigned char  	gain12;
	unsigned char  	reserve1;
	unsigned short  max_exp_time1;

	unsigned char  	fps2;
	unsigned char  	gain21;
	unsigned char  	gain22;
	unsigned char  	reserve2;
	unsigned short  max_exp_time2;

	unsigned char  	fps3;
	unsigned char  	gain31;
	unsigned char  	gain32;
	unsigned char  	reserve3;
	unsigned short  max_exp_time3;

	unsigned char  	fps4;
	unsigned char  	gain41;
	unsigned char  	gain42;
	unsigned char  	reserve4;
	unsigned short  max_exp_time4;
};


struct isp_anti_flicker
{
	int   force_flag;
	unsigned int   default_exp_time_min;
    unsigned int   default_exp_step;
};

static struct isp_anti_flicker anti_flicker = {0};


static struct effect_offset effect_offset_val = {0};
static struct isp_flip_mirror_info flip_mirror = {0};

static int auto_awb_step = 100;
static int power_hz = 50;

static struct fps_switch_info switch_fps = {0};
static struct fps_config fps_cfg = {0};
static AK_ISP_SHARP_ATTR sharp_attr = {0};
static int sharp_init = 0;
static pthread_mutex_t lock;
static int g_sensor_id = 0;

static struct sensor_update_skip sensor_update_skip_table[] = {
	{0x1235, 3}
};

unsigned short Isp_Struct_len[ISP_HUE + 1] = {
	sizeof(struct ak_isp_init_blc),
	sizeof(struct ak_isp_init_lsc),
	sizeof(struct ak_isp_init_raw_lut),
	sizeof(struct ak_isp_init_nr),
	sizeof(struct ak_isp_init_3dnr),
	sizeof(struct ak_isp_init_gb),
	sizeof(struct ak_isp_init_demo),
	sizeof(struct ak_isp_init_gamma),
	sizeof(struct ak_isp_init_ccm),
	sizeof(struct ak_isp_init_fcs),
	sizeof(struct ak_isp_init_wdr),
	sizeof(struct ak_isp_init_sharp),
	sizeof(struct ak_isp_init_saturation),
	sizeof(struct ak_isp_init_contrast),
	sizeof(struct ak_isp_init_rgb2yuv),
	sizeof(struct ak_isp_init_effect),
	sizeof(struct ak_isp_init_dpc),
	sizeof(struct ak_isp_init_weight),
	sizeof(struct ak_isp_init_af),
	sizeof(struct ak_isp_init_wb),
	sizeof(struct ak_isp_init_exp),
	sizeof(struct ak_isp_init_misc),
	sizeof(struct ak_isp_init_y_gamma),
	sizeof(struct ak_isp_init_hue)
};


static AK_ISP_WDR_ATTR wdr_attr = {0};
static int wdr_init = 0;

const int default_wdr_para[65] = {
	0, 16, 32, 48, 64, 80, 96, 112, 
	128, 144, 160, 176, 192, 208, 224, 240, 
	256, 272, 288, 304, 320, 336, 352, 368, 
	384, 400, 416, 432, 448, 464, 480, 496, 
	512, 528, 544, 560, 576, 592, 608, 624,  
	640, 656, 672, 688, 704, 720, 736, 752,  
	768, 784, 800, 816, 832, 848, 864, 880, 
	896, 912, 928, 944, 960, 976, 992, 1008, 
	1023									 
 };


/**
 * brief: load sensor config, only can be use in isp_module_init, don't call it in isp_switch_mode
 * @buf[IN]:config data buf
 * return: 0 success, -1 failed
 * notes:
 */
static int load_sensor_conf(char *buf)
{
	AK_ISP_SENSOR_INIT_PARA *sensor_regs = (void *)buf;
	return Ak_ISP_Sensor_Load_Conf(sensor_regs);
}


/**
 * brief: set sensor fps
 * @fps[IN]:fps to be set
 * return: 0 success, -1 failed
 * notes:
 */
static int set_fps(int fps)
{
	int ret;
	ret = isp_set_sensor_fps(fps);
	ak_print_notice_ex("set fps:%d %s\n", fps, ret ? "failed" : "ok");

	return ret;
}


/**
 * brief: load default AWB step
 * return: 0 success, -1 failed
 * notes:
 */
static int load_def_awb_step(void)
{
	AK_ISP_AWB_ATTR  awb;
	if (AK_ISP_get_awb_attr(&awb)) {
		ak_print_error_ex("get awb_attr failed\n");
		return -1;
	}

	auto_awb_step = awb.auto_wb_step;
	ak_print_normal_ex("get awb_attr ok, auto_awb_step:%d\n", auto_awb_step);
	awb.auto_wb_step = 10;
	return AK_ISP_set_awb_attr(&awb);
}


/**
 * brief: reload AWB step in isp config file
 * return: 0 success, -1 failed
 * notes:
 */
static int reload_config_awb_step(void)
{
	AK_ISP_AWB_ATTR  awb;
	if (AK_ISP_get_awb_attr(&awb)) {
		ak_print_error_ex("get awb_attr failed\n");
		return -1;
	}

	ak_print_normal_ex("get awb_attr ok, cur auto_awb_step:%d, set to step:%d\n",
			awb.auto_wb_step, auto_awb_step);
	awb.auto_wb_step = auto_awb_step;
	auto_awb_step = 0; //����Ϊ0��ʾreload���
	return AK_ISP_set_awb_attr(&awb);
}



/**
 * brief: get AE stat info
 * @aestat[OUT]:AE stat info
 * return: 0 success, -1 failed
 * notes:
 */
static int get_aestat(struct ae_stat_info *aestat)
{
	AK_ISP_AE_RUN_INFO stat;

	if (AK_ISP_get_ae_run_info(&stat))
		return -1;
	aestat->a_gain = stat.current_a_gain;
	aestat->d_gain = stat.current_d_gain;
	aestat->isp_d_gain = stat.current_isp_d_gain;
	aestat->exptime = stat.current_exp_time;

	return 0;
}

/**
 * brief: check fps control params
 * @p_fps_attr[IN]:fps control params
 * return: 0 success, -1 failed
 * notes:
 */
static int check_fps_para(void *p_fps_attr)
{
	#define MAX_VALID_FPS	30
	#define MIN_VALID_FPS	5
	#define	MAX_VALID_EXP	65535
	#define	MIN_VALID_EXP	100
	#define MAX_VALID_GAIN	100
	#define MIN_VALID_GAIN	1


	if (3 == fps_cfg.fps_num) {

		/****3 levels fps, need to convert the struct AK_ISP_FRAME_RATE_ATTR to struct ak_isp_frame_rate_attr_ex***/
		struct ak_isp_frame_rate_attr_ex *fr_ex = (struct ak_isp_frame_rate_attr_ex *)p_fps_attr;

		ak_print_normal("hight light:\n"
			"\tframe_rate=%d, max_exp_time=%d, to mid_light_gain=%d\n",
			fr_ex->fps1,
			fr_ex->max_exp_time1,
			fr_ex->gain12);
		ak_print_normal("low light:\n"
			"\tframe_rate=%d, max_exp_time=%d, to mid_light_gain=%d\n",
			fr_ex->fps3,
			fr_ex->max_exp_time3,
			fr_ex->gain31);
		ak_print_normal("mid light:\n"
			"\tframe_rate=%d, max_exp_time=%d, to high_light_gain=%d, to low_light_gain=%d\n",
			fr_ex->fps2,
			fr_ex->max_exp_time2,
			fr_ex->gain21,
			fr_ex->gain22);


		if (!(fr_ex->fps1 <= MAX_VALID_FPS &&
			fr_ex->fps1 >= MIN_VALID_FPS)) {
			ak_print_error_ex("hight_light_frame_rate=%d failed\n", fr_ex->fps1);
			return -1;
		}

		if (!(fr_ex->max_exp_time1 <= MAX_VALID_EXP &&
			fr_ex->max_exp_time1 >= MIN_VALID_EXP)) {
			ak_print_error_ex("hight_light_max_exp_time = %d failed\n",
						fr_ex->max_exp_time1);
			return -1;
		}

		if (!(fr_ex->gain12 <= MAX_VALID_GAIN &&
			fr_ex->gain12 >= MIN_VALID_GAIN)) {
			ak_print_error_ex("hight_light_to_mid_light_gain = %d failed\n", fr_ex->gain12);
			return -1;
		}

		if (!(fr_ex->fps3 <= MAX_VALID_FPS &&
			fr_ex->fps3 >= MIN_VALID_FPS)) {
			ak_print_error_ex("low_light_frame_rate=%d failed\n", fr_ex->fps3);
			return -1;
		}

		if (!(fr_ex->max_exp_time3 <= MAX_VALID_EXP &&
			fr_ex->max_exp_time3 >= MIN_VALID_EXP)) {
			ak_print_error_ex("low_light_max_exp_time = %d failed\n",
						fr_ex->max_exp_time3);
			return -1;
		}

		/*if (!(fr_ex->gain31 <= MAX_VALID_GAIN &&
			fr_ex->gain31 >= MIN_VALID_GAIN)) {
			ak_print_error_ex("low_light_to_mid_light_gain = %d failed\n", fr_ex->gain31);
			return -1;
		}*/


		if (!(fr_ex->fps2 <= MAX_VALID_FPS &&
			fr_ex->fps2 >= MIN_VALID_FPS)) {
			ak_print_error_ex("mid_light_frame_rate=%d failed\n", fr_ex->fps2);
			return -1;
		}

		if (!(fr_ex->max_exp_time2 <= MAX_VALID_EXP &&
			fr_ex->max_exp_time2 >= MIN_VALID_EXP)) {
			ak_print_error_ex("mid_light_max_exp_time = %d failed\n",
						fr_ex->max_exp_time2);
			return -1;
		}

		/*if (!(fr_ex->gain21 <= MAX_VALID_GAIN &&
			fr_ex->gain21 >= MIN_VALID_GAIN)) {
			ak_print_error_ex("mid_light_to_high_light_gain = %d failed\n", fr_ex->gain21);
			return -1;
		}*/

		if (!(fr_ex->gain22 <= MAX_VALID_GAIN &&
			fr_ex->gain22 >= MIN_VALID_GAIN)) {
			ak_print_error_ex("mid_light_to_low_light_gain = %d failed\n", fr_ex->gain22);
			return -1;
		}
	}else {
		AK_ISP_FRAME_RATE_ATTR *fps_attr = (AK_ISP_FRAME_RATE_ATTR *)p_fps_attr;

		ak_print_normal("hight light:\n"
			"\tframe_rate=%lu max_exp_time=%lu low_light_gain=%lu\n",
			fps_attr->hight_light_frame_rate,
			fps_attr->hight_light_max_exp_time,
			fps_attr->hight_light_to_low_light_gain);
		ak_print_normal("low light:\n"
			"\tframe_rate=%lu max_exp_time=%lu light_gain=%lu\n",
			fps_attr->low_light_frame_rate,
			fps_attr->low_light_max_exp_time,
			fps_attr->low_light_to_hight_light_gain);

		if (!(fps_attr->hight_light_frame_rate <= MAX_VALID_FPS &&
			fps_attr->hight_light_frame_rate >= MIN_VALID_FPS)) {
			ak_print_error_ex("hight_light_frame_rate=%lu failed\n",
						fps_attr->hight_light_frame_rate);
			return -1;
		}

		if (!(fps_attr->hight_light_max_exp_time <= MAX_VALID_EXP &&
			fps_attr->hight_light_max_exp_time >= MIN_VALID_EXP)) {
			ak_print_error_ex("hight_light_max_exp_time = %lu failed\n",
						fps_attr->hight_light_max_exp_time);
			return -1;
		}

		if (!(fps_attr->hight_light_to_low_light_gain <= MAX_VALID_GAIN &&
			fps_attr->hight_light_to_low_light_gain >= MIN_VALID_GAIN)) {
			ak_print_error_ex("hight_light_to_low_light_gain = %lu failed\n",
						fps_attr->hight_light_to_low_light_gain);
			return -1;
		}

		if (!(fps_attr->low_light_frame_rate <= MAX_VALID_FPS &&
			fps_attr->low_light_frame_rate >= MIN_VALID_FPS)) {
			ak_print_error_ex("low_light_frame_rate = %lu failed\n",
						fps_attr->low_light_frame_rate);
			return -1;
		}

		if (!(fps_attr->low_light_max_exp_time <= MAX_VALID_EXP &&
			fps_attr->low_light_max_exp_time >= MIN_VALID_EXP)) {
			ak_print_error_ex("low_light_max_exp_time = %lu failed\n",
						fps_attr->low_light_max_exp_time);
			return -1;
		}

		if (!(fps_attr->low_light_to_hight_light_gain <= MAX_VALID_GAIN &&
			fps_attr->low_light_to_hight_light_gain >= MIN_VALID_GAIN)) {
			ak_print_error_ex("low_light to hight_light_gain = %lu failed\n",
						fps_attr->low_light_to_hight_light_gain);
			return -1;
		}
	}

	return 0;
}


/**
 * brief: init fps info, support 2 levels fps(default), and 3 levels fps
 * return: 0 success, -1 failed
 * notes:
 */
static int init_fps_info(void)
{
	int ret = AK_FAILED;
	AK_ISP_FRAME_RATE_ATTR fps_attr = {0};
	unsigned int size = 0;
	struct ak_isp_frame_rate_attr_ex fr_ex = {0};

	ak_thread_mutex_lock(&lock);
	if (isp_get_attr(ISP_FPS_CTRL, (void *)(&fps_attr), &size)) {
		ak_print_error_ex("get attr failed\n");
		goto init_fps_end;
	}

	memset(&fps_cfg, 0, sizeof(struct fps_config));

	/****support 2 levels fps, and 3 levels fps***/
	if (fps_attr.hight_light_frame_rate & 0xffffff00) {
		fps_cfg.fps_num = 3;

		/****3 levels fps, need to convert the struct AK_ISP_FRAME_RATE_ATTR to struct ak_isp_frame_rate_attr_ex***/
		memcpy(&fr_ex, &fps_attr, sizeof(struct ak_isp_frame_rate_attr_ex));

		if (check_fps_para(&fr_ex)) {
				ak_print_error_ex("failed\n");
				goto init_fps_end;
		}

		ak_print_normal("hight light fps: %d\n", fr_ex.fps1);
		if (0 != set_fps(fr_ex.fps1)) {
			goto init_fps_end;
		}

		fps_cfg.fps_stat = HIGH_FPS_STAT;
		fps_cfg.cur_fps	= fr_ex.fps1;
		fps_cfg.low_fps	= fr_ex.fps3;
		fps_cfg.high_fps = fr_ex.fps1;
		fps_cfg.mid_fps = fr_ex.fps2;

		fps_cfg.low_fps_exp_time = fr_ex.max_exp_time3;
		fps_cfg.high_fps_exp_time = fr_ex.max_exp_time1;
		fps_cfg.mid_fps_exp_time = fr_ex.max_exp_time2;

		fps_cfg.high_fps_to_low_fps_gain = fr_ex.gain12;
		fps_cfg.low_fps_to_high_fps_gain = fr_ex.gain31;
		fps_cfg.mid_fps_to_high_fps_gain = fr_ex.gain21;
		fps_cfg.mid_fps_to_low_fps_gain = fr_ex.gain22;
	}
	else {
		fps_cfg.fps_num = 2;

		if (check_fps_para(&fps_attr)) {
				ak_print_error_ex("failed\n");
				goto init_fps_end;
		}

		ak_print_normal("hight light fps: %lu\n", fps_attr.hight_light_frame_rate);
		if (0 != set_fps(fps_attr.hight_light_frame_rate)) {
			goto init_fps_end;
		}

		fps_cfg.fps_stat = HIGH_FPS_STAT;
		fps_cfg.cur_fps	= fps_attr.hight_light_frame_rate;
		fps_cfg.low_fps	= fps_attr.low_light_frame_rate;
		fps_cfg.high_fps = fps_attr.hight_light_frame_rate;

		fps_cfg.low_fps_exp_time = fps_attr.low_light_max_exp_time;
		fps_cfg.high_fps_exp_time = fps_attr.hight_light_max_exp_time;

		fps_cfg.high_fps_to_low_fps_gain = fps_attr.hight_light_to_low_light_gain;
		fps_cfg.low_fps_to_high_fps_gain = fps_attr.low_light_to_hight_light_gain;
	}

	if (switch_fps.enable) {
		ak_thread_sem_post(&(switch_fps.fps_sem));
	}
	ret = AK_SUCCESS;
	fps_cfg.init = 1;

init_fps_end:
	ak_thread_mutex_unlock(&lock);

	return ret;
}


/**
 * brief: get gain stat
 * @res_gain[OUT]:gain value
 * return: gain stat
 * notes:
 */
static int get_gain_stat(int *res_gain)
{
	enum gain_stat stat = GAIN_NO_CHANGE_STAT;
	int gain = 0;
	struct ae_stat_info ae_stat;

	if (get_aestat(&ae_stat))
		return stat;

	gain = ((ae_stat.a_gain * ae_stat.d_gain >> 8) * ae_stat.isp_d_gain) >> 8 >> 8;

	if (3 == fps_cfg.fps_num) {
		if (fps_cfg.cur_fps == fps_cfg.high_fps) {
			if (gain * ae_stat.exptime > fps_cfg.high_fps_to_low_fps_gain * fps_cfg.high_fps_exp_time)
				stat = GAIN_MID_STAT;
			else
				stat = GAIN_NO_CHANGE_STAT;
		}
		else if (fps_cfg.cur_fps == fps_cfg.low_fps) {
			if (gain * ae_stat.exptime < fps_cfg.mid_fps_to_low_fps_gain * fps_cfg.mid_fps_exp_time * 9 /10)
				stat = GAIN_MID_STAT;
			else
				stat = GAIN_NO_CHANGE_STAT;
		}
		else {
			if (gain * ae_stat.exptime < fps_cfg.high_fps_to_low_fps_gain  * fps_cfg.high_fps_exp_time * 9 /10)
				stat = GAIN_LOW_STAT;
			else if (gain * ae_stat.exptime > fps_cfg.mid_fps_to_low_fps_gain * fps_cfg.mid_fps_exp_time)
				stat = GAIN_HIGH_STAT;
			else
				stat = GAIN_NO_CHANGE_STAT;
		}
	}
	else {
		if (gain > fps_cfg.high_fps_to_low_fps_gain) {
			stat = GAIN_HIGH_STAT;
		} else if (gain < fps_cfg.low_fps_to_high_fps_gain) {
			stat = GAIN_LOW_STAT;
		} else {
			stat = GAIN_NO_CHANGE_STAT;
		}
	}

	*res_gain = gain;
	return stat;
}



/**
 * brief: set exptimemax
 * @exptime_max[IN]:exptime_max
 * return: 0 success, -1 failed
 * notes:
 */
static int set_exptimemax(int exptime_max)
{
	AK_ISP_AE_ATTR ae_attr;

	if (AK_ISP_get_ae_attr(&ae_attr))
		return -1;
	ae_attr.exp_time_max = exptime_max;

    if (anti_flicker.default_exp_step != 1 || anti_flicker.force_flag)
    {
    	if (60 == power_hz) {
    		if ((12 == fps_cfg.cur_fps) || (13 == fps_cfg.cur_fps))
    			ae_attr.exp_step = ae_attr.exp_time_max * 125 / 1200;
    		else
    			ae_attr.exp_step = ae_attr.exp_time_max * fps_cfg.cur_fps / 120;
    	} else {
    		if ((12 == fps_cfg.cur_fps) || (13 == fps_cfg.cur_fps))
    			ae_attr.exp_step = ae_attr.exp_time_max * 125 / 1000;
    		else
    			ae_attr.exp_step = ae_attr.exp_time_max * fps_cfg.cur_fps / 100;
    	}
    }

	if (anti_flicker.force_flag)
	{
		ae_attr.exp_time_min = ae_attr.exp_step;
	}

	if (AK_ISP_set_ae_attr(&ae_attr))
		return -1;

	return 0;
}


/**
 * brief: set exptimemax by cur fps
 * return: 0 success, -1 failed
 * notes:
 */
static void set_exptimemax_by_curfps(void)
{
	if (fps_cfg.init) {
		if (fps_cfg.cur_fps == fps_cfg.high_fps)
			set_exptimemax(fps_cfg.high_fps_exp_time);
		else if (fps_cfg.cur_fps == fps_cfg.low_fps)
			set_exptimemax(fps_cfg.low_fps_exp_time);
		else
			set_exptimemax(fps_cfg.mid_fps_exp_time);
	}
}

/**
 * brief: calc fps need to set
 * return: >0 fps, -1 failed
 * notes:
 */
static int need_set_fps(void)
{
	int gain = GAIN_NO_CHANGE_STAT;
	int new_fps = -1;

	switch (get_gain_stat(&gain)) {
	case GAIN_NO_CHANGE_STAT:
		return -1;
	case GAIN_LOW_STAT:
		new_fps = fps_cfg.high_fps;
		break;
	case GAIN_HIGH_STAT:
		new_fps = fps_cfg.low_fps;
		break;
	case GAIN_MID_STAT:
		new_fps = fps_cfg.mid_fps;
		break;
	default:
		break;
	}

	if (new_fps != fps_cfg.cur_fps) {
		fps_cfg.cur_fps = new_fps;
		ak_print_notice_ex("change sensor fps, new_fps:%d, current gain:%d\n",
			new_fps, gain);
		return new_fps;
	}

	return -1;
}


/**
 * brief: change fps
 * @old_fps[IN]:old fps
 * @fps[IN]:new fps
 * return: 1 success, 0 failed
 * notes:
 */
static int change_fps(int old_fps, int fps)
{
	int set_flag = AK_FALSE;

	if (g_sensor_id == 0)
		if (isp_get_sensor_id(&g_sensor_id) != 0)
			g_sensor_id = 0;

	if (old_fps > fps) {
		if (0 == set_fps(fps)) {
			set_exptimemax_by_curfps();
			set_flag = AK_TRUE;
		}
	} else {
		int i, skip = -1;
		set_exptimemax_by_curfps();

		for (i = 0; i < sizeof(sensor_update_skip_table) / sizeof(sensor_update_skip_table[0]); i++)
			if (sensor_update_skip_table[i].sensor_id == g_sensor_id) {
				skip = sensor_update_skip_table[i].skip_frames;
				break;
			}

		if (skip >= 0) {
			/* some sensor may skip some frames to update exp/gain */
			int delay_us = 1000000 / old_fps * skip;

			//ak_print_notice("ygh2 set exp->fps us:%d\n", delay_us);
			if (delay_us >= 1000000) {
				int tmp = delay_us / 1000000;
				sleep(tmp);
				usleep(delay_us - tmp * 1000000);
			} else {
				usleep(delay_us);
			}
		}

		if (0 == set_fps(fps)) {
			set_flag = AK_TRUE;
		} else {
			fps_cfg.cur_fps = old_fps;
			set_exptimemax_by_curfps();
		}
	}

	return set_flag;
}

/**
 * brief: change fps thread, fps control function thread
 * @param[IN]:old fps
 * return: NULL
 * notes:
 */
static void *change_fps_pthread(void *param)
{
	int set_flag = AK_FALSE;
	int fps = 0, old_fps;
	long int tid = ak_thread_get_tid();
	ak_print_notice_ex("thread id: %ld\n", tid);
	ak_thread_set_name("vi_change_fps");

	sleep(3);
	reload_config_awb_step();

	while (switch_fps.thread_running) {
		ak_thread_sem_wait(&(switch_fps.fps_sem));

		while (switch_fps.thread_running && switch_fps.enable) {
			ak_thread_mutex_lock(&lock);
			old_fps = fps_cfg.cur_fps;
			fps = need_set_fps();
			if (fps > 0) {
				set_flag = change_fps(old_fps, fps);
			}
			ak_thread_mutex_unlock(&lock);

			if (set_flag) {
				set_flag = AK_FALSE;
				sleep(5);
			} else {
				ak_sleep_ms(100);
			}
		}
	}

	ak_print_notice_ex("thread exit, tid: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}


/**
 * brief: set hue effect
 * @value[IN]:hue value
 * return: void
 * notes:
 */
static void set_hue(int value)
{
	int tmp = 0;
	AK_ISP_EFFECT_ATTR effect = {0};

	AK_ISP_get_effect_attr(&effect);
	int default_val = effect.uv_b - effect_offset_val.hue;

	if (default_val + value > 255) {
		tmp = 255 - default_val;
	} else if (default_val + value < -256) {
		tmp = -256 - default_val;
	} else {
		tmp = value;
	}

	effect.uv_b = default_val + tmp;
	effect_offset_val.hue = tmp;

	AK_ISP_set_effect_attr(&effect);
}


/**
 * brief: set brightness effect
 * @value[IN]:brightness value
 * return: void
 * notes:
 */
static void set_brightness(int value)
{
	int tmp = 0;
	AK_ISP_AE_ATTR effect = {0};

	AK_ISP_get_ae_attr(&effect);
	int default_val = effect.target_lumiance - effect_offset_val.brightness;

	if (default_val + value > T_U32_MAX) {
		tmp = T_U32_MAX - default_val;
	} else if (default_val + value < 0) {
		tmp = 0 - default_val;
	} else {
		tmp = value;
	}

	effect.target_lumiance = default_val + tmp;
	effect_offset_val.brightness = tmp;

	AK_ISP_set_ae_attr(&effect);
}


/**
 * brief: set saturation effect
 * @value[IN]:saturation value
 * return: void
 * notes:
 */
static void set_saturation(int value)
{
	int tmp = 0;
	AK_ISP_EFFECT_ATTR effect = {0};

	AK_ISP_get_effect_attr(&effect);
	int default_val = effect.uv_a - effect_offset_val.saturation;

	if (default_val + value > 255) {
		tmp = 255 - default_val;
	} else if (default_val + value < 0) {
		tmp = 0 - default_val;
	} else {
		tmp = value;
	}

	effect.uv_a = default_val + tmp;
	effect_offset_val.saturation = tmp;

	AK_ISP_set_effect_attr(&effect);
}

/**
 * brief: set contrast effect
 * @value[IN]:contrast value
 * return: void
 * notes:
 */
static void set_contrast(int value)
{
	int tmp = 0;
	AK_ISP_EFFECT_ATTR effect = {0};

	/* get current effect */
	AK_ISP_get_effect_attr(&effect);

	/* modify specifically section */
	int default_val = effect.y_a - effect_offset_val.contrast;
	int default_y_b = effect.y_b + effect_offset_val.contrast*2;

	/* value fix incase of error */
	if (default_val + value > 255) {
		tmp = 255 - default_val;
	} else if (default_val + value < 0) {
		tmp = 0 - default_val;
	} else {
		tmp = value;
	}

	if (default_y_b - tmp * 2 < -128) {
		tmp = (default_y_b + 128) / 2;
	} else if (default_y_b - tmp * 2 > 127) {
		tmp = (default_y_b - 127) / 2;
	}

	effect.y_a = default_val + tmp;
	effect.y_b = default_y_b - tmp * 2;
	effect_offset_val.contrast = tmp;

	/* set to isp register */
	AK_ISP_set_effect_attr(&effect);
}

/**
 * brief: set sharp effect
 * @value[IN]:sharp value
 * return: void
 * notes:
 */
static void set_sharp(int value)
{
	int i = 0;
	int j = 0;
	int sharp_coeff =(((value + 50) * 1024) / 50.0);
	AK_ISP_SHARP_ATTR effect = {0};

	/* must init at first */
	if (!sharp_init){
		AK_ISP_get_sharp_attr(&sharp_attr);
		sharp_init = 1;
	}

	/* set value one by one */
	memcpy(&effect, &sharp_attr, sizeof(AK_ISP_SHARP_ATTR));
	for(i=0; i<256; i++){
		effect.manual_sharp_attr.MF_HPF_LUT[i] =
			(effect.manual_sharp_attr.MF_HPF_LUT[i]*sharp_coeff) >> 10;
		effect.manual_sharp_attr.HF_HPF_LUT[i] =
			(effect.manual_sharp_attr.HF_HPF_LUT[i]*sharp_coeff) >> 10;

		if(effect.manual_sharp_attr.MF_HPF_LUT[i] > 255)
			effect.manual_sharp_attr.MF_HPF_LUT[i] = 255;
		if(effect.manual_sharp_attr.MF_HPF_LUT[i] < -256)
			effect.manual_sharp_attr.MF_HPF_LUT[i] = -256;

		if(effect.manual_sharp_attr.HF_HPF_LUT[i] > 255)
			effect.manual_sharp_attr.HF_HPF_LUT[i] = 255;
		if(effect.manual_sharp_attr.HF_HPF_LUT[i] < -256)
			effect.manual_sharp_attr.HF_HPF_LUT[i] = -256;
	}

	/* for more detail, see isp doc */
	for(j=0; j<9; j++){
		for(i=0; i<256; i++){
			effect.linkage_sharp_attr[j].MF_HPF_LUT[i] =
				(effect.linkage_sharp_attr[j].MF_HPF_LUT[i]*sharp_coeff) >> 10;
			effect.linkage_sharp_attr[j].HF_HPF_LUT[i] =
				(effect.linkage_sharp_attr[j].HF_HPF_LUT[i]*sharp_coeff) >> 10;

			if(effect.linkage_sharp_attr[j].MF_HPF_LUT[i] > 255)
				effect.linkage_sharp_attr[j].MF_HPF_LUT[i] = 255;
			if(effect.linkage_sharp_attr[j].MF_HPF_LUT[i] < -256)
				effect.linkage_sharp_attr[j].MF_HPF_LUT[i] = -256;

			if(effect.linkage_sharp_attr[j].HF_HPF_LUT[i] > 255)
				effect.linkage_sharp_attr[j].HF_HPF_LUT[i] = 255;
			if(effect.linkage_sharp_attr[j].HF_HPF_LUT[i] < -256)
				effect.linkage_sharp_attr[j].HF_HPF_LUT[i] = -256;
		}
	}

	effect_offset_val.sharp = value;

	/* set to isp register */
	AK_ISP_set_sharp_attr(&effect);
}

/**
 * brief: set wdr effect
 * @value[IN]:wdr value
 * return: void
 * notes:
 */
static void set_wdr(int value)
{
	int i = 0;
	int j = 0;
	int wdr_coeff =(((value + 50) * 1024) / 50.0);
	AK_ISP_WDR_ATTR effect = {0};

	/* must init at first */
	if (!wdr_init){
		AK_ISP_get_wdr_attr(&wdr_attr);
		wdr_init = 1;
	}

	/* set value one by one */
	memcpy(&effect, &wdr_attr, sizeof(AK_ISP_WDR_ATTR));
	for(i=0;i < 65;i++)
	{
		effect.manual_wdr.area_tb1[i] = 
			(effect.manual_wdr.area_tb1[i]*wdr_coeff + default_wdr_para[i]*(1024-wdr_coeff))>>10;
		effect.manual_wdr.area_tb2[i] = 
			(effect.manual_wdr.area_tb2[i]*wdr_coeff + default_wdr_para[i]*(1024-wdr_coeff))>>10;
		effect.manual_wdr.area_tb3[i] = 
			(effect.manual_wdr.area_tb3[i]*wdr_coeff + default_wdr_para[i]*(1024-wdr_coeff))>>10;
		effect.manual_wdr.area_tb4[i] = 
			(effect.manual_wdr.area_tb4[i]*wdr_coeff + default_wdr_para[i]*(1024-wdr_coeff))>>10;
		effect.manual_wdr.area_tb5[i] = 
			(effect.manual_wdr.area_tb5[i]*wdr_coeff + default_wdr_para[i]*(1024-wdr_coeff))>>10;
		effect.manual_wdr.area_tb6[i] = 
			(effect.manual_wdr.area_tb6[i]*wdr_coeff + default_wdr_para[i]*(1024-wdr_coeff))>>10;

		if(effect.manual_wdr.area_tb1[i]>1023)
			effect.manual_wdr.area_tb1[i] = 1023;
		if(effect.manual_wdr.area_tb2[i]>1023)
			effect.manual_wdr.area_tb2[i] = 1023;
		if(effect.manual_wdr.area_tb3[i]>1023)
			effect.manual_wdr.area_tb3[i] = 1023;
		if(effect.manual_wdr.area_tb4[i]>1023)
			effect.manual_wdr.area_tb4[i] = 1023;
		if(effect.manual_wdr.area_tb5[i]>1023)
			effect.manual_wdr.area_tb5[i] = 1023;
		if(effect.manual_wdr.area_tb6[i]>1023)
			effect.manual_wdr.area_tb6[i] = 1023;
	}

	for(i = 0;i < 9;i++)
	{
		for(j = 0;j < 65;j++)
			{
				effect.linkage_wdr[i].area_tb1[j] =
					(effect.linkage_wdr[i].area_tb1[j]*wdr_coeff + default_wdr_para[j]*(1024-wdr_coeff))>>10;
				effect.linkage_wdr[i].area_tb2[j] =
					(effect.linkage_wdr[i].area_tb2[j]*wdr_coeff + default_wdr_para[j]*(1024-wdr_coeff))>>10;
				effect.linkage_wdr[i].area_tb3[j] =
					(effect.linkage_wdr[i].area_tb3[j]*wdr_coeff + default_wdr_para[j]*(1024-wdr_coeff))>>10;
				effect.linkage_wdr[i].area_tb4[j] =
					(effect.linkage_wdr[i].area_tb4[j]*wdr_coeff + default_wdr_para[j]*(1024-wdr_coeff))>>10;
				effect.linkage_wdr[i].area_tb5[j] =
					(effect.linkage_wdr[i].area_tb5[j]*wdr_coeff + default_wdr_para[j]*(1024-wdr_coeff))>>10;
				effect.linkage_wdr[i].area_tb6[j] =
					(effect.linkage_wdr[i].area_tb6[j]*wdr_coeff + default_wdr_para[j]*(1024-wdr_coeff))>>10;

				if(effect.linkage_wdr[i].area_tb1[j]>1023)
					effect.linkage_wdr[i].area_tb1[j] = 1023;
				if(effect.linkage_wdr[i].area_tb2[j]>1023)
					effect.linkage_wdr[i].area_tb2[j] = 1023;
				if(effect.linkage_wdr[i].area_tb3[j]>1023)
					effect.linkage_wdr[i].area_tb3[j] = 1023;
				if(effect.linkage_wdr[i].area_tb4[j]>1023)
					effect.linkage_wdr[i].area_tb4[j] = 1023;
				if(effect.linkage_wdr[i].area_tb5[j]>1023)
					effect.linkage_wdr[i].area_tb5[j] = 1023;
				if(effect.linkage_wdr[i].area_tb6[j]>1023)
					effect.linkage_wdr[i].area_tb6[j] = 1023;
				
			}
	}

	effect_offset_val.wdr = value;

	/* set to isp register */
	AK_ISP_set_wdr_attr(&effect);
}

/**
 * brief: check config data
 * @cfgbuf[IN]:config data buf
 * @size[IN,OUT]: in ,size to check ; out, size that checked ok
 * return: 0 success, -1 failed
 * notes:
 */
int isp_module_check_cfg(char *cfgbuf, unsigned int *size)
{
	unsigned short i = 0;
	unsigned int total = 0;
	unsigned short moduleid = 0;
	unsigned short length = 0;
	unsigned int offset = 0;

	if (NULL == cfgbuf || 0 == *size) {
		ak_print_error_ex("cfgbuf is null or size is 0, size:%u!\n", *size);
		return -1;
	}

	/* get module id to match specifically mode */
	for (i = ISP_BB; i <= ISP_HUE; i++) {
		memcpy(&moduleid, cfgbuf + offset, ISP_MODULE_ID_SIZE);
		memcpy(&length, cfgbuf + offset + ISP_MODULE_ID_SIZE, ISP_MODULE_LEN_SIZE);
		if ((moduleid != i) || (length != Isp_Struct_len[i])) {
			ak_print_error_ex("data err!\n");
			return -1;
		}

		offset += Isp_Struct_len[i];
		total += Isp_Struct_len[i];
		if (offset > *size) {
			ak_print_error_ex("size err:%u!\n", *size);
			return -1;
		}
	}

	/* sensor id check */
	memcpy(&moduleid, cfgbuf + offset, ISP_MODULE_ID_SIZE);
	if (moduleid != ISP_SENSOR) {
		ak_print_error_ex("sensor id err!\n");
		return -1;
	}

	/* get total size, and check it */
	offset += ISP_MODULE_ID_SIZE;
	total += ISP_MODULE_ID_SIZE;

	memcpy(&length, cfgbuf + offset, ISP_MODULE_LEN_SIZE);
	total += ISP_MODULE_LEN_SIZE + length;
	offset += ISP_MODULE_LEN_SIZE + length;

	/* file lenght and data's length is not match */
	if (offset > *size) {
		ak_print_error_ex("size err:%u!\n", *size);
		return -1;
	}
	*size = total;

	return 0;
}

/**
 * brief: isp device (isp char) open
 * @void
 * return: 0 success, -1 failed
 * notes:
 */
int isp_dev_open(void)
{
	if (AK_ISP_sdk_init()) {
		ak_print_error_ex("AK_ISP_sdk_init failed!\n");
		return -1;
	}
	return 0;
}

/**
 * brief: isp device (isp char) close
 * @void
 * return: 0 success, -1 failed
 * notes:
 */
int isp_dev_close(void)
{
	AK_ISP_sdk_exit();
	return 0;
}

/**
 * brief: isp module init
 * @cfgbuf[IN]:config data buf
 * @size[IN]: size of config data buf
 * return: 0 success, -1 failed
 * notes:
 */
int isp_module_init(char *cfgbuf, unsigned int size)
{
	unsigned int len = size;
	char *pbuf = cfgbuf;
	unsigned short i = 0;
	unsigned int offset = 0;
	unsigned short sensor_len = 0;
	unsigned short num = 0;
	AK_ISP_SENSOR_INIT_PARA sensor_params;

	/* check config */
	int ret = isp_module_check_cfg(cfgbuf, &len);
	if (ret < 0)
		return ret;

	/* set attr */
	for (i = ISP_BB; i <= ISP_HUE; i++) {
		if (ISP_SHARP == i){
			/*
			* copy sharp attr for vpss demo
			*/
			memcpy(&sharp_attr, pbuf+offset+ISP_MODULE_HEAD_SIZE,
				sizeof(AK_ISP_SHARP_ATTR));
			sharp_init = 1;
		}

        if (ISP_WDR == i) {
			/*
			* copy wdr attr for vpss set wdr
			*/
			memcpy(&wdr_attr, pbuf+offset+ISP_MODULE_HEAD_SIZE,
				sizeof(AK_ISP_WDR_ATTR));
			wdr_init = 1;
		}

		isp_set_attr(i, pbuf+offset, Isp_Struct_len[i]);
		offset += Isp_Struct_len[i];
	}

	/* set others config to register */
	offset += ISP_MODULE_ID_SIZE;
	memcpy(&sensor_len, pbuf+offset, ISP_MODULE_LEN_SIZE);
	offset += ISP_MODULE_LEN_SIZE;

	num = sensor_len / sizeof(AK_ISP_SENSOR_REG_INFO);
	sensor_params.num = num;

	sensor_params.reg_info = (void *)(pbuf+offset);
	load_sensor_conf((void *)&sensor_params);

	memset(&anti_flicker, 0, sizeof(struct isp_anti_flicker));

	return ret;
}


/**
 * brief: 3dnr buf malloc
 * @width[IN]:width
 * @height[IN]: height
 * return: mem buf
 * notes:
 */
void *isp_3D_NR_create(int width, int height)
{
	AK_ISP_3D_NR_REF_ATTR m_3d_ref;
	int n,m;

	n = 3;
	m = 2; //for change 3DNR to 1.5*yuv buf

	/* ����һ����3D������Ҫ */
    void *ion_isp_mem = akuio_alloc_pmem(width * height * 3 / 2 * n / m);
    int phyaddr = akuio_vaddr2paddr(ion_isp_mem);

    if (phyaddr) {
		m_3d_ref.yaddr_3d = phyaddr;
       	m_3d_ref.ysize_3d = width * height * n / m;
        m_3d_ref.uaddr_3d = m_3d_ref.yaddr_3d + m_3d_ref.ysize_3d;
       	m_3d_ref.usize_3d = width * height / 4 * n / m;
        m_3d_ref.vaddr_3d = m_3d_ref.uaddr_3d + m_3d_ref.usize_3d;
        m_3d_ref.vsize_3d = width * height / 4 * n / m;

		/* set 3d nr memory to register */
        AK_ISP_set_3d_nr_ref_attr(&m_3d_ref);
        return ion_isp_mem;
    } else {
		return NULL;
    }
}


/**
 * brief: init fps control function, support 2 levels fps(default), and 3 levels fps
 * return: 0 success, -1 failed
 * notes:
 */
int isp_fps_main(void)
{
	memset(&effect_offset_val, 0, sizeof(struct effect_offset));

	load_def_awb_step();

	/* init switch fps resource */
	switch_fps.enable = 1;
	switch_fps.thread_running = 1;
	ak_thread_sem_init(&(switch_fps.fps_sem), 0);
	ak_thread_mutex_init(&lock, NULL);

	if (ak_thread_create(&(switch_fps.switch_fps_pth_id), change_fps_pthread,
			NULL, 100*1024, -1)) {
		return -1;
	}

	/* init internal fps info */
	return init_fps_info();
}

/**
 * brief: isp module deinit
 * @void
 * return: 0 success, -1 failed
 * notes:
 */
int isp_module_deinit(void)
{
	switch_fps.thread_running = 0;
	ak_thread_sem_post(&(switch_fps.fps_sem));

	ak_print_normal_ex("join change_fps_thread...\n");
	ak_thread_join(switch_fps.switch_fps_pth_id);
	ak_print_notice_ex("change_fps_thread join OK\n");

	return 0;
}

/**
 * brief: get isp module data
 * @module_id[IN]: id of the module in enum isp_module_id
 * @buf[OUT]: dest buf for data out
 * @size[OUT]: size of the attr
 * return: 0 success, -1 failed
 * notes:
 */
int isp_get_attr(enum isp_module_id module_id, void *buf, unsigned int *size)
{
	if (NULL == buf || NULL == size) {
		ak_print_error_ex("param err\n");
		return -1;
	}

	int ret = -1;

	/*
	 * get isp attrubite respectively
	 * each specifically case, see more doc to get more information
	 */
	switch (module_id) {
	case ISP_BB: {
			struct  ak_isp_init_blc* p = (struct  ak_isp_init_blc*)buf;

			ret = AK_ISP_get_blc_attr(&p->blc);
			*size = sizeof(struct  ak_isp_init_blc);
		}
		break;
	case ISP_LSC: {
			struct  ak_isp_init_lsc* p = (struct  ak_isp_init_lsc*)buf;

			ret = AK_ISP_get_lsc_attr(&p->lsc);
			*size = sizeof(struct  ak_isp_init_lsc);
		}
		break;
	case ISP_RAW_LUT: {
			struct ak_isp_init_raw_lut *p = (struct ak_isp_init_raw_lut*)buf;

			ret = AK_ISP_get_raw_lut_attr(&p->raw_lut);
			*size = sizeof(struct ak_isp_init_raw_lut);
		}
		break;
	case ISP_NR: {
			struct ak_isp_init_nr *p = (struct ak_isp_init_nr*)buf;

			ret = AK_ISP_get_nr1_attr(&p->nr1);
			ret |= AK_ISP_get_nr2_attr(&p->nr2);
			*size = sizeof(struct ak_isp_init_nr);
		}
		break;
	case ISP_3DNR: {
			struct ak_isp_init_3dnr * p = (struct ak_isp_init_3dnr*)buf;

			ret = AK_ISP_get_3d_nr_attr(&p->nr_3d);
			*size = sizeof(struct ak_isp_init_3dnr);
		}
		break;
	case ISP_GB: {
			struct ak_isp_init_gb *p = (struct ak_isp_init_gb*)buf;

			ret = AK_ISP_get_gb_attr(&p->gb);
			*size = sizeof(struct ak_isp_init_gb);
		}
		break;
	case ISP_DEMO: {
			struct ak_isp_init_demo *p = (struct ak_isp_init_demo*)buf;

			ret = AK_ISP_get_demo_attr(&p->demo);
			*size = sizeof(struct ak_isp_init_demo);
		}
		break;
	case ISP_GAMMA: {
			struct ak_isp_init_gamma *p = (struct ak_isp_init_gamma*)buf;

			ret = AK_ISP_get_rgb_gamma_attr(&p->gamma);
			*size = sizeof(struct ak_isp_init_gamma);
		}
		break;
	case ISP_CCM: {
			struct  ak_isp_init_ccm *p = (struct  ak_isp_init_ccm*)buf;

			ret = AK_ISP_get_ccm_attr(&p->ccm);
			*size = sizeof(struct  ak_isp_init_ccm);
		}
		break;
	case ISP_FCS: {
			struct  ak_isp_init_fcs *p = (struct  ak_isp_init_fcs*)buf;

			ret = AK_ISP_get_fcs_attr(&p->fcs);
			*size = sizeof(struct  ak_isp_init_fcs);
		}
		break;
	case ISP_WDR: {
			struct ak_isp_init_wdr *p = (struct ak_isp_init_wdr*)buf;

			ret = AK_ISP_get_wdr_attr(&p->wdr);
			*size = sizeof(struct ak_isp_init_wdr);
		}
		break;
	case ISP_SHARP: {
			struct ak_isp_init_sharp *p = (struct ak_isp_init_sharp*)buf;

			ret = AK_ISP_get_sharp_attr(&p->sharp);
			ret |= AK_ISP_get_sharp_ex_attr(&p->sharp_ex);
			*size = sizeof(struct ak_isp_init_sharp);
		}
		break;
	case ISP_SATURATION: {
			struct ak_isp_init_saturation *p = (struct ak_isp_init_saturation*)buf;

			ret = AK_ISP_get_saturation_attr(&p->saturation);
			*size = sizeof(struct ak_isp_init_saturation);
		}
		break;
	case ISP_CONSTRAST: {
			struct  ak_isp_init_contrast *p = (struct  ak_isp_init_contrast*)buf;

			ret = AK_ISP_get_contrast_attr(&p->contrast);
			*size = sizeof(struct  ak_isp_init_contrast);
		}
		break;
	case ISP_RGB2YUV: {
			struct ak_isp_init_rgb2yuv *p = (struct ak_isp_init_rgb2yuv*)buf;

			ret = AK_ISP_get_rgb2yuv_attr(&p->rgb2yuv);
			*size = sizeof(struct ak_isp_init_rgb2yuv);
		}
		break;
	case ISP_YUVEFFECT: {
			struct ak_isp_init_effect *p = (struct ak_isp_init_effect*)buf;

			ret = AK_ISP_get_effect_attr(&p->effect);
			*size = sizeof(struct ak_isp_init_effect);
		}
		break;
	case ISP_DPC: {
			struct ak_isp_init_dpc *p = (struct ak_isp_init_dpc*)buf;

			ret = AK_ISP_get_dpc_attr(&p->ddpc);
			ret |= AK_ISP_get_sdpc_attr(&p->sdpc);
			*size = sizeof(struct ak_isp_init_dpc);
		}
		break;
	case ISP_WEIGHT: {
			struct ak_isp_init_weight *p = (struct ak_isp_init_weight*)buf;

			ret = AK_ISP_get_weight_attr(&p->weight);
			*size = sizeof(struct ak_isp_init_weight);
		}
		break;
	case ISP_AF: {
			struct ak_isp_init_af *p = (struct ak_isp_init_af*)buf;

			ret = AK_ISP_get_af_attr(&p->af);
			*size = sizeof(struct ak_isp_init_af);
		}
		break;
	case ISP_WB: {
			struct ak_isp_init_wb *p = (struct ak_isp_init_wb*)buf;

			ret = AK_ISP_get_wb_type(&p->wb_type);
			ret |= AK_ISP_get_mwb_attr(&p->mwb);
			ret |= AK_ISP_get_awb_attr(&p->awb);
			ret |= AK_ISP_get_awb_ex_attr(&p->awb_ex);
			*size = sizeof(struct ak_isp_init_wb);
		}
		break;
	case ISP_EXP: {
			struct ak_isp_init_exp *p = (struct ak_isp_init_exp*)buf;

			ret = AK_ISP_get_raw_hist_attr(&p->raw_hist);
			ret |= AK_ISP_get_rgb_hist_attr(&p->rgb_hist);
			ret |= AK_ISP_get_yuv_hist_attr(&p->yuv_hist);
			ret |= AK_ISP_get_exp_type(&p->exp_type);
			ret |= AK_ISP_get_frame_rate(&p->frame_rate);
			ret |= AK_ISP_get_ae_attr(&p->ae);
			*size = sizeof(struct ak_isp_init_exp);
		}
		break;
	case ISP_MISC: {
			struct ak_isp_init_misc *p = (struct ak_isp_init_misc*)buf;

			ret = AK_ISP_get_misc_attr(&p->misc);
			*size = sizeof(struct ak_isp_init_misc);
		}
		break;
	case ISP_Y_GAMMA: {
			struct ak_isp_init_y_gamma *p = (struct ak_isp_init_y_gamma*)buf;

			ret = AK_ISP_get_Y_gamma_attr(&p->gamma);
			*size = sizeof(struct ak_isp_init_y_gamma);
		}
		break;
	case ISP_HUE: {
			struct ak_isp_init_hue *p = (struct ak_isp_init_hue*)buf;

			ret = AK_ISP_get_hue_attr(&p->hue);
			*size = sizeof(struct ak_isp_init_hue);
		}
		break;
	case ISP_FPS_CTRL: {
			AK_ISP_FRAME_RATE_ATTR *p = (AK_ISP_FRAME_RATE_ATTR *)buf;
			ret = AK_ISP_get_frame_rate(p);
			*size = sizeof(AK_ISP_FRAME_RATE_ATTR);
		}
		break;
	case ISP_EV_TH: {
			AK_ISP_AE_ATTR ae_attr = {0};
			ret = AK_ISP_get_ae_attr(&ae_attr);
			memcpy(buf, &(ae_attr.envi_gain_range[0][0]), sizeof(unsigned int));
			*size = sizeof(unsigned int);
		}
		break;
	default:
		ak_print_error_ex("param err\n");
		return -1;
		break;
	}

	/* isp basic setting need to set,other no need to set */
	if (module_id <= ISP_HUE) {
		memcpy(buf, &module_id, ISP_MODULE_ID_SIZE);
		memcpy(buf+ISP_MODULE_ID_SIZE, size, ISP_MODULE_LEN_SIZE);
	}
	return ret;
}

/**
 * brief: set isp module data
 * @module_id[IN]: id of the module in enum isp_module_id
 * @buf[IN]: data buf to set
 * @size[OUT]: size of the attr
 * return: 0 success, -1 failed
 * notes:
 */
int isp_set_attr(enum isp_module_id module_id, char *buf, unsigned int size)
{
	if (!buf) {
		ak_print_error_ex("param err\n");
		return -1;
	}

	/* data check */
	if ((module_id <= ISP_HUE) && (size != Isp_Struct_len[module_id])) {
		ak_print_error_ex("size err\n");
		return -1;
	}

	int ret = -1;
	/*
	 * With the get function corresponding,
	 * this is set isp attrubite respectively
	 * each specifically case, see more doc to get more information
	 */
	switch (module_id) {
	case ISP_BB: {
			struct  ak_isp_init_blc *p = (struct  ak_isp_init_blc *)buf;
			ret = AK_ISP_set_blc_attr(&p->blc);
		}
		break;
	case ISP_LSC: {
			struct  ak_isp_init_lsc *p = (struct  ak_isp_init_lsc *)buf;
			ret = AK_ISP_set_lsc_attr(&p->lsc);
		}
		break;
	case ISP_RAW_LUT: {
			struct ak_isp_init_raw_lut *p = (struct ak_isp_init_raw_lut *)buf;
			ret = AK_ISP_set_raw_lut_attr(&p->raw_lut);
		}
		break;
	case ISP_NR: {
			struct ak_isp_init_nr *p = (struct ak_isp_init_nr *)buf;
			ret = AK_ISP_set_nr1_attr(&p->nr1);
			ret |= AK_ISP_set_nr2_attr(&p->nr2);
		}
		break;
	case ISP_3DNR: {
			struct ak_isp_init_3dnr *p = (struct ak_isp_init_3dnr *)buf;
			ret = AK_ISP_set_3d_nr_attr(&p->nr_3d);
		}
		break;
	case ISP_GB: {
			struct ak_isp_init_gb *p = (struct ak_isp_init_gb *)buf;
			ret = AK_ISP_set_gb_attr(&p->gb);
		}
		break;
	case ISP_DEMO: {
			struct ak_isp_init_demo *p = (struct ak_isp_init_demo *)buf;
			ret = AK_ISP_set_demo_attr(&p->demo);
		}
		break;
	case ISP_GAMMA: {
			struct ak_isp_init_gamma *p = (struct ak_isp_init_gamma *)buf;
			ret = AK_ISP_set_rgb_gamma_attr(&p->gamma);
		}
		break;
	case ISP_CCM: {
			struct  ak_isp_init_ccm *p = (struct  ak_isp_init_ccm *)buf;
			ret = AK_ISP_set_ccm_attr(&p->ccm);
		}
		break;
	case ISP_FCS: {
			struct  ak_isp_init_fcs *p = (struct  ak_isp_init_fcs *)buf;
			ret = AK_ISP_set_fcs_attr(&p->fcs);
		}
		break;
	case ISP_WDR: {
			struct ak_isp_init_wdr *p = (struct ak_isp_init_wdr *)buf;
			ret = AK_ISP_set_wdr_attr(&p->wdr);
		}
		break;
	case ISP_SHARP: {
			struct ak_isp_init_sharp *p = (struct ak_isp_init_sharp *)buf;
			ret = AK_ISP_set_sharp_attr(&p->sharp);
			ret |= AK_ISP_set_sharp_ex_attr(&p->sharp_ex);
		}
		break;
	case ISP_SATURATION: {
			struct ak_isp_init_saturation *p = (struct ak_isp_init_saturation *)buf;
			ret = AK_ISP_set_saturation_attr(&p->saturation);
		}
		break;
	case ISP_CONSTRAST: {
			struct  ak_isp_init_contrast *p = (struct  ak_isp_init_contrast *)buf;
			ret = AK_ISP_set_contrast_attr(&p->contrast);
		}
		break;
	case ISP_YUVEFFECT: {
			struct ak_isp_init_effect *p = (struct ak_isp_init_effect *)buf;
			ret = AK_ISP_set_effect_attr(&p->effect);
		}
		break;
	case ISP_RGB2YUV: {
			struct ak_isp_init_rgb2yuv *p = (struct ak_isp_init_rgb2yuv *)buf;
			ret = AK_ISP_set_rgb2yuv_attr(&p->rgb2yuv);
		}
		break;
	case ISP_DPC: {
			struct ak_isp_init_dpc *p = (struct ak_isp_init_dpc *)buf;
			ret = AK_ISP_set_dpc_attr(&p->ddpc);
			ret |= AK_ISP_set_sdpc_attr(&p->sdpc);
		}
		break;
	case ISP_WEIGHT: {
			struct ak_isp_init_weight *p = (struct ak_isp_init_weight *)buf;
			ret = AK_ISP_set_weight_attr(&p->weight);
		}
		break;
	case ISP_AF: {
			struct ak_isp_init_af *p = (struct ak_isp_init_af *)buf;
			ret = AK_ISP_set_af_attr(&p->af);
		}
		break;
	case ISP_WB: {
			struct ak_isp_init_wb *p = (struct ak_isp_init_wb *)buf;
			ret = AK_ISP_set_wb_type(&p->wb_type);
			ret |= AK_ISP_set_mwb_attr(&p->mwb);
			ret |= AK_ISP_set_awb_attr(&p->awb);
			ret |= AK_ISP_set_awb_ex_attr(&p->awb_ex);
		}
		break;
	case ISP_EXP: {
			struct ak_isp_init_exp *p = (struct ak_isp_init_exp *)buf;
			ret = AK_ISP_set_raw_hist_attr(&p->raw_hist);
			ret |= AK_ISP_set_rgb_hist_attr(&p->rgb_hist);
			ret |= AK_ISP_set_yuv_hist_attr(&p->yuv_hist);
			ret |= AK_ISP_set_exp_type(&p->exp_type);
			ret |= AK_ISP_set_frame_rate(&p->frame_rate);
			ret |= AK_ISP_set_ae_attr(&p->ae);
			anti_flicker.default_exp_time_min = p->ae.exp_time_min;
            anti_flicker.default_exp_step = p->ae.exp_step;

			init_fps_info();
		}
		break;
	case ISP_MISC: {
			struct ak_isp_init_misc *p = (struct ak_isp_init_misc *)buf;
			ret = AK_ISP_set_misc_attr(&p->misc);
		}
		break;

	case ISP_Y_GAMMA: {
			struct ak_isp_init_y_gamma *p = (struct ak_isp_init_y_gamma*)buf;
			ret = AK_ISP_set_Y_gamma_attr(&p->gamma);
		}
		break;
	case ISP_HUE: {
			struct ak_isp_init_hue *p = (struct ak_isp_init_hue *)buf;
			ret = AK_ISP_set_hue_attr(&p->hue);
		}
		break;
	case ISP_FPS_CTRL: {
			AK_ISP_FRAME_RATE_ATTR *p = (AK_ISP_FRAME_RATE_ATTR *)buf;
			AK_ISP_set_frame_rate(p);
		}
		break;
	default:
		ak_print_error_ex("param err\n");
		return -1;
		break;
	}

	return ret;
}

/**
 * brief: get isp stat info data
 * @module_id[IN]: id of the module in enum isp_module_id
 * @buf[OUT]: dest buf for data out
 * @size[OUT]: size of the attr
 * return: 0 success, -1 failed
 * notes:
 */
int isp_get_statinfo(enum isp_module_id module_id, void *buf, unsigned int *size)
{
	if (NULL == buf || NULL == size) {
		ak_print_error_ex("param err\n");
		return -1;
	}

	/*
	 * get specifically module's status
	 * for more detail to see isp document
	 */
	switch (module_id) {
	case ISP_3DSTAT:
		AK_ISP_get_3d_nr_stat_info((AK_ISP_3D_NR_STAT_INFO*)buf);
		*size = sizeof(AK_ISP_3D_NR_STAT_INFO);
		break;
	case ISP_AESTAT:
		AK_ISP_get_ae_run_info((AK_ISP_AE_RUN_INFO*)buf);
		*size = sizeof(AK_ISP_AE_RUN_INFO);
		AK_ISP_get_raw_hist_stat_info((AK_ISP_RAW_HIST_STAT_INFO*)(buf + *size));
		*size += sizeof(AK_ISP_RAW_HIST_STAT_INFO);
		AK_ISP_get_rgb_hist_stat_info((AK_ISP_RGB_HIST_STAT_INFO*)(buf + *size));
		*size += sizeof(AK_ISP_RGB_HIST_STAT_INFO);
		AK_ISP_get_yuv_hist_stat_info((AK_ISP_YUV_HIST_STAT_INFO*)(buf + *size));
		*size += sizeof(AK_ISP_YUV_HIST_STAT_INFO);
		break;
	case ISP_AFSTAT:
		AK_ISP_get_af_stat_info((AK_ISP_AF_STAT_INFO*)buf);
		*size = sizeof(AK_ISP_AF_STAT_INFO);
		break;
	case ISP_AWBSTAT:
		Ak_ISP_get_awb_stat_info((AK_ISP_AWB_STAT_INFO*)buf);
		*size = sizeof(AK_ISP_AWB_STAT_INFO);
		break;
	case ISP_AESTAT_SUB_RGB_STAT:
		AK_ISP_get_rgb_hist_stat_info((AK_ISP_RGB_HIST_STAT_INFO *)buf);
		break;
	default:
		return -1;
	}

	return 0;
}

/**
 * brief: set sensor register value
 * @addr[IN]:sensor register addr
 * @value[IN]: value to set
 * return: 0 success, -1 failed
 * notes:
 */
int isp_set_sensor_value(unsigned short addr, unsigned short value)
{
	AK_ISP_SENSOR_REG_INFO sensor_reg;

	sensor_reg.reg_addr = addr;
	sensor_reg.value = value;

	return Ak_ISP_Sensor_Set_Reg(&sensor_reg);
}

/**
 * brief: get sensor register value
 * @addr[IN]:sensor register addr
 * @value[OUT]: value of the register addr
 * return: 0 success, -1 failed
 * notes:
 */
int isp_get_sensor_value(unsigned short addr, unsigned short *value)
{
	AK_ISP_SENSOR_REG_INFO sensor_reg;

	sensor_reg.reg_addr = addr;
	sensor_reg.value = 0;

	int ret = Ak_ISP_Sensor_Get_Reg(&sensor_reg);
	*value = sensor_reg.value;

	return ret;
}

/**
 * brief: get sensor id
 * @id[OUT]: sensor id
 * return: 0 success, -1 failed
 * notes:
 */
int isp_get_sensor_id(int *id)
{
	return Ak_ISP_Sensor_Get_Id(id);
}

/**
 * brief: day night switch
 * @cfgbuf[IN]:config data buf
 * @size[IN]: size of config data buf
 * return: 0 success, -1 failed
 * notes:
 */
int isp_switch_mode(char *cfgbuf, unsigned int size)
{
	unsigned short i = 0;
	unsigned int offset = 0;
	struct effect_offset effect;
	unsigned int len = size;
	int ret = isp_module_check_cfg(cfgbuf, &len);

	if (ret < 0)
		return ret;

	for (i = ISP_BB; i <= ISP_HUE; i++) {
		if (ISP_SHARP == i){
			/*
			* copy sharp attr for vpss demo
			*/
			memcpy(&sharp_attr, cfgbuf+offset+ISP_MODULE_HEAD_SIZE,
				sizeof(AK_ISP_SHARP_ATTR));
			sharp_init = 1;
		}

        if (ISP_WDR == i) {
			/*
			* copy wdr attr for vpss set wdr
			*/
			memcpy(&wdr_attr, cfgbuf+offset+ISP_MODULE_HEAD_SIZE,
				sizeof(AK_ISP_WDR_ATTR));
			wdr_init = 1;
		}

		/*
		* skip set misc when switch day night mode ,
		* only set misc once when init .
		*/
		if (ISP_MISC != i) {
			isp_set_attr(i, cfgbuf+offset, Isp_Struct_len[i]);
		}
		offset += Isp_Struct_len[i];
	}

	/*
	* set user effect after set isp module data
	*/
	memcpy(&effect, &effect_offset_val, sizeof(struct effect_offset));
	memset(&effect_offset_val, 0, sizeof(struct effect_offset));

	isp_set_effect(VPSS_EFFECT_HUE, effect.hue);
	isp_set_effect(VPSS_EFFECT_BRIGHTNESS, effect.brightness);
	isp_set_effect(VPSS_EFFECT_SATURATION, effect.saturation);
	isp_set_effect(VPSS_EFFECT_CONTRAST, effect.contrast);
	isp_set_effect(VPSS_EFFECT_SHARP, effect.sharp);

	init_fps_info();

	ak_thread_mutex_lock(&lock);
	set_exptimemax_by_curfps();
	if (auto_awb_step != 0)
	    load_def_awb_step();
	ak_thread_mutex_unlock(&lock);

	return 0;
}

/**
 * isp_get_effect - get effect
 * @type[IN]:effect type
 * @value[OUT]: effect value
 * return: 0 success, -1 failed
 */
int isp_get_effect(enum vpss_effect_type type, int *value)
{
	if (!value) {
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;

	switch (type) {
	case VPSS_EFFECT_HUE:
		*value = effect_offset_val.hue;
		break;
	case VPSS_EFFECT_BRIGHTNESS:
		*value = effect_offset_val.brightness;
		break;
	case VPSS_EFFECT_SATURATION:
		*value = effect_offset_val.saturation;
		break;
	case VPSS_EFFECT_CONTRAST:
		*value = effect_offset_val.contrast;
		break;
	case VPSS_EFFECT_SHARP:
		*value = effect_offset_val.sharp;
		break;
    case VPSS_EFFECT_WDR:
		*value = effect_offset_val.wdr;
		break;
	default:
		ak_print_error_ex("error type: %d\n", type);
		ret = AK_FAILED;
		break;
	}

	return ret;
}

/**
 * isp_set_effect - set effect
 * @type[IN]:effect type
 * @value[IN]: effect value, [-50, 50], 0 means use the value in ispxxx.conf
 * return: 0 success, -1 failed
 */
int isp_set_effect(enum vpss_effect_type type, int value)
{
	if (value > 50  || value < -50) {
		ak_print_error_ex("value range [-50, 50], cur value: %d\n", value);
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	switch (type){
	case VPSS_EFFECT_HUE:
		set_hue(value);
		break;
	case VPSS_EFFECT_BRIGHTNESS:
		set_brightness(value);
		break;
	case VPSS_EFFECT_SATURATION:
		set_saturation(value);
		break;
	case VPSS_EFFECT_CONTRAST:
		set_contrast(value);
		break;
	case VPSS_EFFECT_SHARP:
		set_sharp(value);
		break;
    case VPSS_EFFECT_WDR:
		set_wdr(value);
		break;
	default:
		ak_print_error_ex("error type: %d\n", type);
		ret = AK_FAILED;
		break;
	}

	return ret;
}


/**
 * brief: get current isp working frequence
 * return: hz value
 * notes:
 */
int isp_get_hz(void)
{
	return power_hz;
}

/**
 * brief: set isp working frequence
 * @value[IN]: power hz, 50 or 60
 * return: 0 success, -1 failed
 * notes:
 */
int isp_set_hz(int hz)
{
	AK_ISP_AE_ATTR ae_attr;
	int sensor_fps = 0;

	/*
	 * get current fps and ae attr,
	 * and calc a new value to set to register
	 */
	sensor_fps = Ak_ISP_Get_Sensor_Fps();

	if (AK_ISP_get_ae_attr(&ae_attr))
		return -1;

	power_hz = hz;

    if (anti_flicker.default_exp_step != 1 || anti_flicker.force_flag)
    {
    	if (60 == power_hz) {
    		if ((12 == sensor_fps) || (13 == sensor_fps))
    			ae_attr.exp_step = ae_attr.exp_time_max * 125 / 1200;
    		else
    			ae_attr.exp_step = ae_attr.exp_time_max * sensor_fps / 120;
    	} else {
    		if ((12 == sensor_fps) || (13 == sensor_fps))
    			ae_attr.exp_step = ae_attr.exp_time_max * 125 / 1000;
    		else
    			ae_attr.exp_step = ae_attr.exp_time_max * sensor_fps / 100;
    	}
    }

	if (anti_flicker.force_flag)
	{
		ae_attr.exp_time_min = ae_attr.exp_step;
	}

	/* set new value to register */
	if (AK_ISP_set_ae_attr(&ae_attr))
		return -1;

	return 0;
}

/**
 * brief: set flip mirror
 * @value[IN]: info, flip flag and mirror flag
 * return: 0 success, -1 failed
 * notes:
 */
int isp_set_flip_mirror(struct isp_flip_mirror_info *info)
{
	if (NULL == info)
		return -1;

	flip_mirror.flip_en = info->flip_en;
	flip_mirror.mirror_en = info->mirror_en;

	return Ak_ISP_Set_Flip_Mirror(info);
}

/**
 * brief: set sensor framerate
 * @value[IN]: fps, sensor framerate
 * return: 0 success, -1 failed
 * notes:
 */
int isp_set_sensor_fps(const int fps)
{
	ak_print_notice_ex("set sensor fps: %d\n", fps);
	return Ak_ISP_Set_Sensor_Fps(&fps);
}

/**
 * brief: get sensor current work scene
 * @void
 * return: current work scene
 */
int isp_get_work_scene(void)
{
	return Ak_ISP_Get_Work_Scene();
}

/**
 * brief: get flip mirror flag
 * @value[OUT]: info, flip flag and mirror flag
 * return: 0 success, -1 failed
 * notes:
 */
int isp_get_flip_mirror(struct isp_flip_mirror_info *info)
{
	if (NULL == info)
		return -1;

	info->flip_en = flip_mirror.flip_en;
	info->mirror_en = flip_mirror.mirror_en;

	return 0;
}

/**
 * brief: set main channel mask
 * @p_mask[IN]: main channel mask paramters
 * return: 0 success, -1 failed
 * notes:
 */
int isp_set_main_mask_area(const struct isp_mask_area_info *p_mask)
{
	if (NULL == p_mask)
		return -1;

	int ret = AK_ISP_set_main_chan_mask_area((void *)p_mask);
	if (ret)
		ret = -1;
	return ret;
}

/**
 * brief: get main channel mask
 * @p_mask[OUT]: main channel mask paramters
 * return: 0 success, -1 failed
 * notes:
 */
int isp_get_main_mask_area(struct isp_mask_area_info *p_mask)
{
	if (NULL == p_mask)
		return -1;

	int ret = AK_ISP_get_main_chan_mask_area((void *)p_mask);
	if (ret)
		ret = -1;
	return ret;
}

/**
 * brief: set sub channel mask
 * @p_mask[IN]: sub channel mask paramters
 * return: 0 success, -1 failed
 * notes:
 */
int isp_set_sub_mask_area(const struct isp_mask_area_info *p_mask)
{
	if (NULL == p_mask)
		return -1;

	int ret = AK_ISP_set_sub_chan_mask_area((void *)p_mask);
	if (ret)
		ret = -1;
	return ret;
}

/**
 * brief: get sub channel mask
 * @p_mask[OUT]: sub channel mask paramters
 * return: 0 success, -1 failed
 * notes:
 */
int isp_get_sub_mask_area(struct isp_mask_area_info *p_mask)
{
	if (NULL == p_mask)
		return -1;

	int ret = AK_ISP_get_sub_chan_mask_area((void *)p_mask);
	if (ret)
		ret = -1;
	return ret;
}

/**
 * brief: set mask color
 * @p_mask[IN]: mask color paramters
 * return: 0 success, -1 failed
 * notes:
 */
int isp_set_mask_color(const struct isp_mask_color_info *p_mask)
{
	if (NULL == p_mask)
		return -1;

	int ret = AK_ISP_set_mask_color((void *)p_mask);
	if (ret)
		ret = -1;
	return ret;
}

/**
 * brief: get mask color
 * @p_mask[OUT]: mask color paramters
 * return: 0 success, -1 failed
 * notes:
 */
int isp_get_mask_color(struct isp_mask_color_info *p_mask)
{
	if (NULL == p_mask)
		return -1;

	int ret = AK_ISP_get_mask_color((void *)p_mask);
	if (ret)
		ret = -1;
	return ret;
}


/**
 * brief: get sensor fps, get sensor fps by isp char api
 * return: sensor fps
 * notes:
 */
int isp_get_sensor_fps(void)
{
	int sensor_fps = 0;

	sensor_fps = Ak_ISP_Get_Sensor_Fps();
	if (!sensor_fps) {
		ak_print_error_ex("Error, fps is 0, we use default 25\n");
		sensor_fps = 25;
	}

	return sensor_fps;
}


/**
 * brief: set switch fps enable,internal switch fps function on/off api
 * @enable[IN]: enable or not
 * return: 0
 * notes:
 */
int isp_set_switch_fps_enable(int enable)
{
	switch_fps.enable = enable;
	if (switch_fps.enable) {
		ak_thread_sem_post(&(switch_fps.fps_sem));
	}
	
	return 0;
}

int isp_get_force_anti_flicker_flag(void)
{
	return anti_flicker.force_flag;
}

int isp_set_force_anti_flicker_flag(int force_flag)
{
	AK_ISP_AE_ATTR ae_attr;
	int sensor_fps = 0;

	/* 
	 * get current fps and ae attr, 
	 * and calc a new value to set to register
	 */
	sensor_fps = Ak_ISP_Get_Sensor_Fps();
	
	if (AK_ISP_get_ae_attr(&ae_attr))
		return -1;

    if (anti_flicker.default_exp_step != 1 || force_flag)
    {
    	if (60 == power_hz) {
    		if ((12 == sensor_fps) || (13 == sensor_fps))
    			ae_attr.exp_step = ae_attr.exp_time_max * 125 / 1200;
    		else
    			ae_attr.exp_step = ae_attr.exp_time_max * sensor_fps / 120;
    	} else {
    		if ((12 == sensor_fps) || (13 == sensor_fps))
    			ae_attr.exp_step = ae_attr.exp_time_max * 125 / 1000;
    		else
    			ae_attr.exp_step = ae_attr.exp_time_max * sensor_fps / 100;
    	}
    }

	if (force_flag)
	{
		ae_attr.exp_time_min = ae_attr.exp_step;
	}
	else
	{
		ae_attr.exp_time_min = anti_flicker.default_exp_time_min;

        if (1 == anti_flicker.default_exp_step)
            ae_attr.exp_step = anti_flicker.default_exp_step;
	}
	
	if (AK_ISP_set_ae_attr(&ae_attr))
		return -1;

	anti_flicker.force_flag = force_flag;

	ak_print_normal_ex("exp_time_min=%ld, force_flag=%d\n",
		ae_attr.exp_time_min, anti_flicker.force_flag);

	return 0;
}

/**
 * brief: get current lum factor 
 * return: lum factor
 * notes:
 */
int isp_get_cur_lum_factor(void)
{
	if (1 != fps_cfg.init) {
		ak_print_error_ex("init fps info failed\n");
		return -1;
	}

	if (0 >= fps_cfg.high_fps_exp_time) {	
		ak_print_error_ex("high_fps_exp_time error\n");
		return -1;
	}
	
	struct ae_stat_info ae_stat;
	if (get_aestat(&ae_stat)) {	
		ak_print_error_ex("get AE stat info failed\n");
		return -1;
	}
	
	int cur_gain = 0;
	int cur_exp_time = 0;
	int product = 0;
	int quotient = 0;
	int fps_exp_time = 0;

	/* reserve 8 decimal places */
	cur_gain = (ae_stat.a_gain * ae_stat.d_gain >> 8) * ae_stat.isp_d_gain >> 8;
	cur_exp_time = ae_stat.exptime;

	product = cur_gain * cur_exp_time;

	/* get the current fps_exp_time */
	if (fps_cfg.cur_fps == fps_cfg.high_fps)
		fps_exp_time = fps_cfg.high_fps_exp_time;
	else if (fps_cfg.cur_fps == fps_cfg.low_fps)
		fps_exp_time = fps_cfg.low_fps_exp_time;
		
	if (!fps_exp_time) {
		ak_print_error_ex("get fps_exp_time error\n");
		return -1;
	}
	
	quotient = product / fps_exp_time;

	return quotient;
}

