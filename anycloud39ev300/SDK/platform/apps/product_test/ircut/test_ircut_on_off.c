#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ak_common.h"
#include "ak_drv_ir.h"
#include "ak_drv_irled.h"
#include "ak_vi.h"
#include "test_config.h"

enum day_ctrl_level {
	DAY_LEVEL_RESERVED = 0x00,
	DAY_LEVEL_HH,		//high-high
	DAY_LEVEL_HL,		//high-low
	DAY_LEVEL_LH,		//low-high
	DAY_LEVEL_LL,		//low-low
	DAY_LEVEL_MAX
};


/**
 * test_set_video_day_night: set video day or night mode, according to IR value
 * @vi_handle: opened vi handle
 * @ir_val: IR value, [0, 1]
 * @day_level: day control level, [1, 4]
 * return: 0 success, -1 failed
 */
int test_set_video_day_night(void *vi_handle, int ir_val, int day_level)
{
	if (!vi_handle) {
		return AK_FAILED;
	}

	int day_val = 0;

	switch (day_level) {
	case DAY_LEVEL_HH:
		ak_drv_ir_set_ircut(ir_val);
		day_val = ir_val;
		break;
	case DAY_LEVEL_HL:
		ak_drv_ir_set_ircut(!ir_val);
		day_val = ir_val;
		break;
	case DAY_LEVEL_LH:
		ak_drv_ir_set_ircut(!ir_val);
		day_val = !ir_val;
		break;
	case DAY_LEVEL_LL:
		ak_drv_ir_set_ircut(ir_val);
		day_val = !ir_val;
		break;
	default:
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	if (day_val) {
		ak_print_normal_ex("now set to day\n");
		ret = ak_vi_switch_mode(vi_handle, VI_MODE_DAY);
		ak_drv_irled_set_working_stat(0);
	} else {
		ak_print_normal_ex("now set to night\n");
		ak_drv_irled_set_working_stat(1);
		ret = ak_vi_switch_mode(vi_handle, VI_MODE_NIGHT);
	}

	ak_sleep_ms(300);

	return ret;
}


static int init_irled(void)
{
	struct ak_drv_irled_hw_param param;
	struct config_handle *config = test_config_get_value();

	param.irled_working_level = config->irled_mode;
	return ak_drv_irled_init(&param);
}


/**
 * test_ircut_on_off - test ircut function.
 * @vi_handle[IN]: opened vi handle
 * @day_ctrl[IN]: day control level, [1, 4]
 * return:  0, sucess; -1, failed
 */
int test_ircut_on_off(void *vi_handle, int day_ctrl)
{
	int ir_val = -1;
	
	if(ak_drv_ir_init()< 0) {
		ak_print_error_ex("ak_drv_ir_init fail\n");
		return -1;
	}

	init_irled();

	while (1) {
		int ir_val_new = ak_drv_ir_get_input_level();

		if (ir_val_new != -1 && ir_val_new != ir_val) {
			test_set_video_day_night(vi_handle, ir_val_new, day_ctrl);
			ir_val = ir_val_new;
		}

		ak_sleep_ms(1000);
	}

	return 0;
}
