
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "dana_osd.h"

#include "ak_common.h"
#include "ak_vi.h"
#include "ak_osd.h"
#include "ak_osd_ex.h"
#include "ak_thread.h"
#include "ak_config.h"

static int init_flag = 0;

/**
 * dana_osd_init - create thread to draw osd to screen on dana platform
 * @vi[IN]: vi handle
 * return: 0 - success, fail return -1.
 */
int dana_osd_init(void *vi)
{
	struct osd_ex_info osd_info;
	/* init dana osd if needed */
	
	if (init_flag) {		
		return 0;
	}

	/* get camera information */
    struct camera_disp_config *camera = ak_config_get_camera_info();

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

	ak_osd_ex_turn_on(0);
	ak_osd_ex_turn_on(1);
	init_flag = 1;
	
	return 0;
}

/**
 * dana_osd_exit - exit osd thread 
 * return: void.
 */
void dana_osd_exit(void)
{
	if (init_flag) {
		ak_osd_ex_exit();
		init_flag = 0;		
	}
}

