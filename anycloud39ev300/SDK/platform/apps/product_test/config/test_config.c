#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "ak_ini.h"
#include "ak_common.h"
#include "ak_net.h"
#include "ak_cmd_exec.h"
#include "test_config.h"

#define BUF_SIZE (100)
#define CONFIG_VALUE_BUF_SIZE		200
#define CONFIG_DUMP_INFO			0

/* default config file */
#define CONFIG_ANYKA_FILE_NAME  	"/etc/jffs2/anyka_cfg.ini"

static struct config_handle ini_config = {0};


static void test_config_init_value(void)
{
	char value[100] = {0};

	/* get osd name */
	ak_ini_get_item_value(ini_config.handle, "camera", "osd_name", 
			ini_config.osd_name);

	/* get device name */
	ak_ini_get_item_value(ini_config.handle, "global", "dev_name", 
			ini_config.dev_name);

	/* get uid name */
	memset(ini_config.uid_name,0,100);
	if(0 == ak_ini_get_item_value(ini_config.handle, "global", "uid_name", value)){
		strcpy(ini_config.uid_name, value);
	}

	/* get dhcp config */
    ak_ini_get_item_value(ini_config.handle, "ethernet", "dhcp", value);
	ini_config.dhcp = atoi(value);

	/* get global software version */
	bzero(value, sizeof(value));
	ak_ini_get_item_value(ini_config.handle, "global", "soft_version", value);
	ini_config.soft_version = atoi(value);

	/* get audio sample_rate*/
	ak_ini_get_item_value(ini_config.handle, "record", "sample_rate", value);
    ini_config.sample_rate = atoi(value);

	/* get camera config*/
	ak_ini_get_item_value(ini_config.handle, "camera", "main_width", value);
    ini_config.main_width = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "camera", "main_height", value);
    ini_config.main_height = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "camera", "sub_width", value);
    ini_config.sub_width = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "camera", "sub_height", value);
    ini_config.sub_height = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "camera", "main_max_width", value);
    ini_config.main_max_width = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "camera", "main_max_height", value);
    ini_config.main_max_height = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "camera", "sub_max_width", value);
    ini_config.sub_max_width = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "camera", "sub_max_height", value);
    ini_config.sub_max_height = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "camera", "day_ctrl", value);
    ini_config.day_ctrl = atoi(value);

	ak_ini_get_item_value(ini_config.handle, "image", "irled_mode", value);
    ini_config.irled_mode = atoi(value);
	
	/* get video config*/
	ak_ini_get_item_value(ini_config.handle, "video", "main_min_qp", value);
    ini_config.main_min_qp = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "main_max_qp", value);
    ini_config.main_max_qp = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "sub_min_qp", value);
    ini_config.sub_min_qp = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "sub_max_qp", value);
    ini_config.sub_max_qp = atoi(value);
	
    ak_ini_get_item_value(ini_config.handle, "video", "main_fps", value);
    ini_config.main_fps = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "main_min_kbps", value);
    ini_config.main_min_kbps = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "main_max_kbps", value);
    ini_config.main_max_kbps = atoi(value);

    ak_ini_get_item_value(ini_config.handle, "video", "sub_fps", value);
    ini_config.sub_fps = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "sub_min_kbps", value);
    ini_config.sub_min_kbps = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "sub_max_kbps", value);
    ini_config.sub_max_kbps = atoi(value);

    ak_ini_get_item_value(ini_config.handle, "video", "main_gop_len", value);
    ini_config.main_gop_len = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "sub_gop_len", value);
    ini_config.sub_gop_len = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "main_video_mode", value);
    ini_config.main_video_mode = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "sub_video_mode", value);
    ini_config.sub_video_mode = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "main_enc_type", value);
    ini_config.main_enc_type = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "sub_enc_type", value);
    ini_config.sub_enc_type = atoi(value);

	ak_ini_get_item_value(ini_config.handle, "video", "main_target_ratio_264", value);
    ini_config.main_target_ratio_264 = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "sub_target_ratio_264", value);
    ini_config.sub_target_ratio_264 = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "main_target_ratio_265", value);
    ini_config.main_target_ratio_265 = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "sub_target_ratio_265", value);
    ini_config.sub_target_ratio_265 = atoi(value);

	ak_ini_get_item_value(ini_config.handle, "video", "main_smart_goplen", value);
    ini_config.main_smart_goplen = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "sub_smart_goplen", value);
    ini_config.sub_smart_goplen = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "main_smart_mode", value);
    ini_config.main_smart_mode = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "sub_smart_mode", value);
    ini_config.sub_smart_mode = atoi(value);

	ak_ini_get_item_value(ini_config.handle, "video", "main_smart_quality_264", value);
    ini_config.main_smart_quality_264 = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "sub_smart_quality_264", value);
    ini_config.sub_smart_quality_264 = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "main_smart_quality_265", value);
    ini_config.main_smart_quality_265 = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "sub_smart_quality_265", value);
    ini_config.sub_smart_quality_265 = atoi(value);

	ak_ini_get_item_value(ini_config.handle, "video", "main_smart_target_ratio_264", value);
    ini_config.main_smart_target_ratio_264 = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "sub_smart_target_ratio_264", value);
    ini_config.sub_smart_target_ratio_264 = atoi(value);
    ak_ini_get_item_value(ini_config.handle, "video", "main_smart_target_ratio_265", value);
    ini_config.main_smart_target_ratio_265 = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "sub_smart_target_ratio_265", value);
    ini_config.sub_smart_target_ratio_265 = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "main_smart_static_value", value);
    ini_config.main_smart_static_value = atoi(value);
	ak_ini_get_item_value(ini_config.handle, "video", "sub_smart_static_value", value);
    ini_config.sub_smart_static_value = atoi(value);
}

/**
 * test_init_ini: init ini module
 * return: void
 */
void test_init_ini(void)
{
	ini_config.handle = ak_ini_init(CONFIG_ANYKA_FILE_NAME);

	test_config_init_value();
}

/**
 * test_config_get_value: get config value, must call test_init_ini function first
 * return: config_handle
 */
struct config_handle* test_config_get_value(void)
{
	return &ini_config;
}

/**
 * test_exit_ini: exit ini module
 * return: void
 */
void test_exit_ini(void)
{
	if (!ini_config.handle) {
		ak_ini_destroy(ini_config.handle);
		ini_config.handle = NULL;
	}
}

