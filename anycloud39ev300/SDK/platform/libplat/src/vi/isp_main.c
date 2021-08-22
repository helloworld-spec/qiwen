#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/ioctl.h>
#include <math.h>
#include <pthread.h>

#include "isp_basic.h"
#include "isp_cfg_file.h"
#include "isp_vi.h"
#include "isp_main.h"

#include "ak_common.h"
#include "ak_thread.h"

static char *cfg_buf = NULL;	//config buf pointer, use to init config file
static unsigned int cfgsize = ISP_CFG_MAX_SIZE;	//config file size
static pthread_mutex_t cfglock = PTHREAD_MUTEX_INITIALIZER;	//static init globla mutex
static char mode_str[MODE_NUM][32] = {"day", "night", "custom_2", "custom_3"};
static enum daynight_mode day_night_mode = MODE_DAY_OUTDOOR;

/**
 * brief: init ispsdk, include isp char ,video 0, isp modules
 * return: 0 succuss, -1 failed
 * notes:
 */
int isp_device_open(void)
{
	/*1.init isp char*/
	if (isp_dev_open()) {
		ak_print_error_ex("isp_dev_open failed!\n");
		return AK_FAILED;
	}

	/*2.init  video 0*/
	if (isp_vi_open() == -1) {
		isp_dev_close();
		ak_print_error_ex("video 0 open failed!\n");
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

/**
 * brief: isp device close, include video 0 close, ispsdk exit
 * return: 0 succuss, -1 failed
 * notes:
 */
int isp_device_close(void)
{
	isp_vi_close();
	isp_dev_close();

	if (cfg_buf) {
		free(cfg_buf);
		cfg_buf = NULL;
	}

	return AK_SUCCESS;
}

/*
 * init isp module
 * return 0 on success, -1 failed
 */
int isp_init(void)
{
	if (!cfg_buf) {
		ak_print_error_ex("Should load config file first\n");
		return -1;
	}
	/* load config file to buf and check version */
	if (isp_module_init(cfg_buf, cfgsize)) {
		free(cfg_buf);
		cfg_buf = NULL;
		cfgsize = 0;
		return -1;
	}

	return 0;
}

/**
 * brief: match sensor cfgfile
 * @config_file[IN]:config file path
 * return: 0 success, -1 failed
 * notes:
 */
int isp_main_match_sensor_cfgfile(const char *config_file)
{
	if (!config_file) {
		ak_print_error_ex("invalid argument\n");
		return AK_FAILED;
	}

	/* global config file buffer */
	if (cfg_buf) {
		ak_print_normal_ex("config file buf not emtpy, re init it\n");
		free(cfg_buf);
		cfg_buf = NULL;
	}
	/* each new file will load agian */
	cfg_buf = (char *)calloc(1, ISP_CFG_MAX_SIZE);
	if (!cfg_buf) {
		ak_print_error_ex("calloc failed, size: %d\n", ISP_CFG_MAX_SIZE);
		return AK_FAILED;
	}

	/* check config, if ok, load config arguments to memory */
	isp_cfg_file_set_path(config_file);

	/*
	 * load config file day-mode config to buf,
	 * check config file is only check day-mode
	 */
	int ret = isp_cfg_file_load(VI_MODE_DAY, cfg_buf, &cfgsize);
	if (ret) {
		free(cfg_buf);
		cfg_buf = NULL;
	}

	return ret;
}

/**
 * brief: switch day night mode
 * @mode[IN]:day or night
 * return: 0 success, -1 failed
 * notes:
 */
int isp_switch(enum video_daynight_mode mode)
{
	char *cfg_data = NULL;
	unsigned int cfg_size = ISP_CFG_MAX_SIZE;
	int ret = AK_FAILED;

	if (mode >= VI_MODE_NUM) {
		ak_print_error_ex("mode err:%d\n", mode);
		return AK_FAILED;
	}

	/* allocate new memory to store target-mode config data */
	cfg_data = (char*)calloc(1, cfg_size);
	if (NULL == cfg_data) {
		ak_print_error_ex("calloc cfg_data failed\n");
		return AK_FAILED;
	}


	ak_print_normal_ex("switching isp mode -> %s\n", mode_str[mode]);


	ak_thread_mutex_lock(&cfglock);
	/* load target mode-config */
	if (isp_cfg_file_load(mode, cfg_data, &cfg_size) < 0) {
		ak_print_error_ex("file load failed\n");
		goto isp_switch_end;
	}

	/* switch to taget-mode */
	if (isp_switch_mode(cfg_data, cfg_size)) {
		ak_print_error_ex("switch failed, ret: %d\n", ret);
		goto isp_switch_end;
	}
	ret = AK_SUCCESS;
    day_night_mode = mode;

isp_switch_end:
	ak_thread_mutex_unlock(&cfglock);

	if (cfg_data) {
		free(cfg_data);
		cfg_data = NULL;
	}

	return ret;
}

enum daynight_mode isp_get_day_night_mode(void)
{
    return day_night_mode;
}

