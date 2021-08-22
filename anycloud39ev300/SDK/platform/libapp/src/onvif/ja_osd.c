#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ak_thread.h"
#include "ak_common.h"
#include "ak_onvif_config.h"
#include "ak_osd_ex.h"
#include "ja_osd.h"

static int init_flag = 0;
static ak_mutex_t lock;

/**
 * ja_osd_init - init osd param and create thread to draw osd to screen on ja platform
 * @vi[IN]: vi handle
 * return: 0 - success, fail return -1.
 */
int ja_osd_init(void *vi)
{
	struct osd_ex_info osd_info;
	/* init ja osd if needed */

	ak_thread_mutex_lock(&lock);
	if (init_flag) {
		ak_thread_mutex_unlock(&lock);
		return 0;
	}

	/* get camera information */
    struct onvif_camera_config *camera = onvif_config_get_camera();

	/* set display position and format */	
	memset(&osd_info, 0, sizeof(struct osd_ex_info));
	osd_info.position[OSD_NAME_RECT] = camera->name_position;
	osd_info.position[OSD_TIME_RECT] = camera->time_position;
	osd_info.position[OSD_STAT_RECT] = camera->rate_position;	
	strcpy(osd_info.name, camera->osd_name);	
	osd_info.date_format = camera->date_format;
	osd_info.hour_format = camera->hour_format;
	osd_info.week_format = camera->week_format;

	/* adapt camera setting */
	osd_info.x_width[0] = camera->main_width;
	osd_info.x_width[1] = camera->sub_width;
	osd_info.y_height[0] = camera->main_height;
	osd_info.y_height[1] = camera->sub_height;
	osd_info.osd_disp_size[0] = camera->main_osd_size;
	osd_info.osd_disp_size[1] = camera->sub_osd_size;	
	
	ak_print_info_ex("osd size.main:%d sub:%d!\n", osd_info.osd_disp_size[0],
			osd_info.osd_disp_size[1]);

	ak_osd_ex_init(vi, &osd_info);
	init_flag = 1;
	ak_thread_mutex_unlock(&lock);
	
	return 0;
}

/**
 * ja_osd_exit - release osd resource and exit osd thread 
 * return: void.
 */
void ja_osd_exit(void)
{
	ak_thread_mutex_lock(&lock);
	if (init_flag) {
		ak_osd_ex_exit();
		init_flag = 0;		
	}
	ak_thread_mutex_unlock(&lock);
}

/**
 * ja_osd_init_mutex - init  osd mutex
 * return: void.
 */
void ja_osd_init_mutex(void)
{
	ak_thread_mutex_init(&lock, NULL);
}

/**
 * ja_osd_destroy_mutex - destroy  osd mutex
 * return: void.
 */
void ja_osd_destroy_mutex(void)
{
	ak_thread_mutex_destroy(&lock);
}


