#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "internal_error.h"
#include "ak_common.h"
#include "ak_cmd_exec.h"
#include "ak_drv_ir.h"

#define STRING_LEN	            128
#define IR_FEED_GPIO_FILE_NAME  "/sys/user-gpio/gpio-rf_feed"
#define IR_FEED_AIN_FILE_NAME   "/sys/kernel/ain/ain0"
#define IRCUT_A_FILE_NAME       "/sys/user-gpio/gpio-ircut_a"
#define IRCUT_B_FILE_NAME       "/sys/user-gpio/gpio-ircut_b"

/* ircut mode define */
enum ircut_mode_type {
	IRCUT_MODE_1LINE,		//1 line mode
	IRCUT_MODE_2LINE,		//2 line mode
};

/* ir structure */
struct drv_ir {
    char *ircut_name;		//ircut name, define as sys fs
    int ircut_line_mode;	//default 1line mode
    int init_flag;			//init flag

    enum ak_ain_check_mode check_mode;		//check mode
    struct ak_ir_threshold day_thr;		    //day threshold
	struct ak_ir_threshold night_thr;		//night threshold
};

static const char drv_ir_version[] = "libplat_drv_ir V1.0.00";
static struct drv_ir ir_ctrl = {0};

/* 
 * camera_set_ircut - switct ircut 
 * value[IN]: 0 or 1 to set day or night
 * name[IN]: device name reflect on sys-fs
 * return: always return 0;
 * notes: must run cmd_serverd to receive shell-cmd
 */
static int camera_set_ircut(int value, char *name)
{
	char cmd[STRING_LEN];
	char result[STRING_LEN];

	memset(result, '\0', STRING_LEN);
	sprintf(cmd, "echo %d > %s", value, name);
	ak_cmd_exec(cmd, result, STRING_LEN);

	return 0;
}

static void init_default_value(void)
{
    ir_ctrl.ircut_name = NULL;
    ir_ctrl.ircut_line_mode = IRCUT_MODE_1LINE;
    ir_ctrl.check_mode = AIN_MODE_DAY_NIGTH;

    ir_ctrl.day_thr.min = 1100;
    ir_ctrl.day_thr.max = 2000;
    ir_ctrl.day_thr.feature = 1;

    ir_ctrl.night_thr.min = 2;
    ir_ctrl.night_thr.max = 300;
    ir_ctrl.night_thr.feature = 0;
}

/* get ain mode threshold */
static int get_ain_threshold(int rf_feed_level)
{
    int ret = -1;

    switch (ir_ctrl.check_mode) {
    case AIN_MODE_DAY_MIN:
        if (rf_feed_level > ir_ctrl.day_thr.min) {
    		ret = 1; //day mode
    	} else {
    		ret = 0; //night
    	}
        break;
    case AIN_MODE_DAY_MAX:
        if (rf_feed_level < ir_ctrl.day_thr.max) {
    		ret = 1; //day mode
    	} else {
    		ret = 0; //night
    	}
        break;
    case AIN_MODE_NIGTH_MIN:
        if (rf_feed_level > ir_ctrl.night_thr.min) {
    		ret = 1; //day mode
    	} else {
    		ret = 0; //night
    	}
        break;
    case AIN_MODE_NIGTH_MAX:
        if (rf_feed_level < ir_ctrl.night_thr.max) {
    		ret = 1; //day mode
    	} else {
    		ret = 0; //night
    	}
        break;
    default:    //default is AIN_MODE_DAY_NIGTH
        if ((rf_feed_level > ir_ctrl.day_thr.min)
    		|| (rf_feed_level == ir_ctrl.day_thr.feature)) {
    		ret = 1; //day mode
    	} else if (((rf_feed_level > ir_ctrl.night_thr.min)
    	            && (rf_feed_level < ir_ctrl.night_thr.max))
    		    || (rf_feed_level == ir_ctrl.night_thr.feature)) {
    		ret = 0; //night
    	}
        break;
    }

	return ret;
}

const char* ak_drv_ir_get_version(void)
{
	return drv_ir_version;
}

/**
 * ak_drv_ir_get_input_level - get value of light dependent resistors
 * return: 1 - day; 0-night; otherwise -1;
 */
int ak_drv_ir_get_input_level(void)
{
	char result[4];
	int day_night_level = 0;

	if (!ir_ctrl.init_flag) {
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex("not inited.\n");
		return AK_FAILED;
	}

	FILE *fp = fopen(IR_FEED_GPIO_FILE_NAME, "r");
	if (fp) {
		fread(result, 1, 2, fp);
		fclose(fp);
		day_night_level = atoi(result);
	} else {
		FILE *fpa = fopen(IR_FEED_AIN_FILE_NAME, "r");
		if(!fpa) {
			set_error_no(ERROR_TYPE_FILE_OPEN_FAILED);
			ak_print_error_ex("fopen %s failed, %s\n", IR_FEED_AIN_FILE_NAME,
			   	strerror(errno));
			return -1;
		}

		if (fpa) {
			fread(result, 1, 4, fpa);
			fclose(fpa);
		}

		int rf_feed_level = atoi(result);
		day_night_level = get_ain_threshold(rf_feed_level);
	}

	return day_night_level;
}

/**
 * ak_drv_ir_set_check_mode - set ain threshold check mode
 * @check_mode[IN]: check mode in 'enum ak_ain_check_mode'
 * return: 0 success, -1 failed
 */
int ak_drv_ir_set_check_mode(enum ak_ain_check_mode check_mode)
{
    ir_ctrl.check_mode = check_mode;

    return AK_SUCCESS;
}

/**
 * ak_drv_ir_get_threshold - get ain threshold, including day and night.
 * @day_thr[OUT]: ain day mode threshold value
 * @night_thr[OUT]: ain nigth mode threshold value
 * return: 0 success, -1 failed
 */
int ak_drv_ir_get_threshold(struct ak_ir_threshold *day_thr,
                            struct ak_ir_threshold *night_thr)
{
    if (!day_thr || !night_thr) {
        return AK_FAILED;
    }
    if (!ir_ctrl.init_flag) {
        set_error_no(ERROR_TYPE_NOT_INIT);
		return AK_FAILED;
	}

    memcpy(day_thr, &ir_ctrl.day_thr, sizeof(ir_ctrl.day_thr));
    memcpy(night_thr, &ir_ctrl.night_thr, sizeof(ir_ctrl.night_thr));

    return AK_SUCCESS;
}

/**
 * ak_drv_ir_set_threshold - set ain threshold, including day and night.
 * @day_thr[IN]: ain day mode threshold value
 * @night_thr[IN]: ain nigth mode threshold value
 * return: 0 success, -1 failed
 */
int ak_drv_ir_set_threshold(const struct ak_ir_threshold *day_thr,
                            const struct ak_ir_threshold *night_thr)
{
    if (!day_thr || !night_thr) {
        return AK_FAILED;
    }

    memcpy(&ir_ctrl.day_thr, day_thr, sizeof(ir_ctrl.day_thr));
    memcpy(&ir_ctrl.night_thr, night_thr, sizeof(ir_ctrl.night_thr));

    return AK_SUCCESS;
}

/**
 * ak_drv_ir_set_ircut - set ircut to switch
 * @status_level[IN]:  status level to control day or night, [0,1]
 * return: 0 success, -1 failed
 */
int ak_drv_ir_set_ircut(int status_level)
{
	if(!ir_ctrl.init_flag){
		set_error_no(ERROR_TYPE_NOT_INIT);
		ak_print_error_ex("not inited\n");
		return AK_FAILED;
	}

	if((0 != status_level) && (1 != status_level)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("invalid arg\n");
		return AK_FAILED;
	}

	if (ir_ctrl.ircut_line_mode == IRCUT_MODE_1LINE) {
		ak_print_debug("Ircut 1line mode, ir_feed:%d\n", status_level);
		camera_set_ircut(status_level, ir_ctrl.ircut_name);
        return AK_SUCCESS;
	}

	ak_print_debug("Ircut 2line mode, ir_feed:%d\n", status_level);
	camera_set_ircut(status_level, IRCUT_A_FILE_NAME);
	camera_set_ircut(!status_level, IRCUT_B_FILE_NAME);

	ak_sleep_ms(10);
	/* ircut_set_idle */
	camera_set_ircut(0, IRCUT_A_FILE_NAME);
	camera_set_ircut(0, IRCUT_B_FILE_NAME);

	return AK_SUCCESS;
}

/**
 * ak_drv_ir_init - init ircut, get ircut control mode
 * return: 0 - success; otherwise -1;
 */
int ak_drv_ir_init(void)
{
	if(ir_ctrl.init_flag){
		ak_print_notice_ex("have been inited.\n");
		return 0;
	}

	int flags = 0;
	struct stat st = {0};

    init_default_value();
    if (-1 == stat(IRCUT_A_FILE_NAME, &st)) {
        ak_print_info("Cannot identify '%s': %d, %s\n",
                 IRCUT_A_FILE_NAME, errno, strerror(errno));
		flags |= 1;
    } else {
		ir_ctrl.ircut_name = IRCUT_A_FILE_NAME;
	}

    if (-1 == stat(IRCUT_B_FILE_NAME, &st)) {
        ak_print_info("Cannot identify '%s': %d, %s\n",
                 IRCUT_B_FILE_NAME, errno, strerror(errno));
		flags |= 1 << 1;
    } else {
		ir_ctrl.ircut_name = IRCUT_B_FILE_NAME;
	}

	switch (flags) {
	case 0:
		ak_print_info("Ircut a & b interface all can access\n");
		ir_ctrl.ircut_line_mode = IRCUT_MODE_2LINE;
		break;
	case 3:
		set_error_no(ERROR_TYPE_FUNC_NOT_SUPPORT);
		ak_print_error("Ircut a & b interface can't access\n");
		return AK_FAILED;
	default:
		ak_print_info("Only can access:%s\n", ir_ctrl.ircut_name);
		ir_ctrl.ircut_line_mode = IRCUT_MODE_1LINE;
		break;
	}

	ir_ctrl.init_flag = AK_TRUE;

    return AK_SUCCESS;
}
