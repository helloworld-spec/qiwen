#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "internal_error.h"
#include "ak_common.h"
#include "ak_cmd_exec.h"
#include "ak_drv_irled.h"

#define STRING_LEN	     	128
#define IRLED_FILE_NAME		"/sys/user-gpio/ir-led"

struct drv_irled_info {
	int init;
	struct ak_drv_irled_hw_param param;
};

static const char drv_irled_version[] = "libplat_drv_ir V1.0.00";
static struct drv_irled_info irled_info;

/**
 * drv_irled_set_working_level - set irled module working status 
 * 		(nighttime level)
 * return: 0 - success; otherwise -1;
 */
static int drv_irled_set_working_level(struct ak_drv_irled_hw_param *param)
{
	int level = param->irled_working_level;
	if (level)
		irled_info.param.irled_working_level = 1;
	else
		irled_info.param.irled_working_level = 0;

	ak_print_info_ex("irled_working_level:%d\n",
			irled_info.param.irled_working_level);

	return 0;
}

/* 
 * ak_drv_irled_get_version - get version
 * return: pointer to version-string
 */
const char* ak_drv_irled_get_version(void)
{
	return drv_irled_version;
}

/**
 * ak_drv_irled_init - set irled module initialize
 * param[IN]: pointer to struct ak_drv_irled_hw_param, use to init
 *            ir_led model 
 * return: 0 - success; otherwise -1;
 */
int ak_drv_irled_init(struct ak_drv_irled_hw_param *param)
{
	int ret;

	ret = drv_irled_set_working_level(param);
	if (!ret && !ak_check_file_exist(IRLED_FILE_NAME))
		irled_info.init = 1;

	return ret;
}

/**
 * ak_drv_irled_get_working_stat - get irled module working status
 * return: 1 - irled is working; 0 - irled is NOT working; -1 - get fail;
 */
int ak_drv_irled_get_working_stat(void)
{
	int level;
	char cmd[STRING_LEN];
	char result[STRING_LEN];

	if (irled_info.init != 1) {
		ak_print_error_ex("not init.\n");
		return -1;
	}

	memset(result, '\0', STRING_LEN);
	sprintf(cmd, "cat %s", IRLED_FILE_NAME);
	ak_cmd_exec(cmd, result, STRING_LEN);
	if (result[0] == '\0')
		return -1;

	level = atoi(result);
	if (level == irled_info.param.irled_working_level)
		return 1;
	return 0;
}

/**
 * ak_drv_irled_set_working_stat - set irled module working status
 * @working[IN]: set irled working or not. 0 - NOT working; 1 - working;
 * return: 1 - irled is working; 0 - irled is NOT working; -1 - get fail;
 */
int ak_drv_irled_set_working_stat(int working)
{
	int level;
	char cmd[STRING_LEN];
	char result[STRING_LEN];

	if (irled_info.init != 1) {
		ak_print_error_ex("not init.\n");
		return -1;
	}

	if (working)
		level = irled_info.param.irled_working_level;
	else
		level = 1 - irled_info.param.irled_working_level;

	memset(result, '\0', STRING_LEN);
	sprintf(cmd, "echo %d > %s", level, IRLED_FILE_NAME);
	ak_cmd_exec(cmd, result, STRING_LEN);
	if (result[0] == '\0')
		return -1;

	return 0;
}
