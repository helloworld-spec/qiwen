
#include "ak_vi.h"

#ifndef _ISP_MAIN_H_
#define _ISP_MAIN_H_


enum daynight_mode {
	MODE_DAY_OUTDOOR,
	MODE_NIGHTTIME,
	MODE_CUSTOM_2,
	MODE_CUSTOM_3,
	MODE_NUM
};


/**
 * brief: init ispsdk, include isp char ,video 0, isp modules
 * return: 0 succuss, -1 failed
 * notes:
 */
int isp_device_open(void);

/**
 * brief: match sensor cfgfile
 * @config_file[IN]:config file path
 * return: 0 success, -1 failed
 * notes:
 */
int isp_main_match_sensor_cfgfile(const char *config_file);

/**
 * brief: switch day night mode
 * @mode[IN]:day or night
 * return: 0 success, -1 failed
 * notes:
 */
int isp_switch(enum video_daynight_mode mode);

/*
 * init isp module
 * return 0 on success, -1 failed
 */
int isp_init(void);

/**
 * brief: isp device close, include video 0 close, ispsdk exit
 * return: 0 succuss, -1 failed
 * notes:
 */
int isp_device_close(void);

enum daynight_mode isp_get_day_night_mode(void);


#endif
