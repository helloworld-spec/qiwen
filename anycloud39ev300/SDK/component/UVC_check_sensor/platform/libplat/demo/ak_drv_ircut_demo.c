#include "command.h"
#include "ak_drv_ircut.h"
#include "ak_common.h"
#include "kernel.h"

static char *help[]={
	"ircut module demo",
	"usage:ircutdemo [on/off]\n"
	"      on- daytime ircut state\n"
	"      off- nighttime ircut state\n"
};

void cmd_ircut_demo(int argc, char **args) 
{
	int ret;
	int condition;

	if (argc != 1)
	{
		ak_print_error("%s",help[1]);
		return;
	}

	if (0 == strcmp("on", args[0]))
		condition = 0;
	else if (0 == strcmp("off", args[0]))
		condition = 1;
	else {
		ak_print_error("not defined condition:%s\n", args[0]);
		ak_print_error("%s",help[1]);
		return;
	}

	ak_print_normal("ircutdemo start\n");

	ret = ak_drv_ir_init();
	if (0 != ret) {
		ak_print_error_ex("ircut init fail\n");
		return ;
	}

	if (condition == 0) {
		ak_print_normal("set ircut day\n");
		ret = ak_drv_ir_set_ircut(IRCUT_STATUS_DAY);
		if (0 != ret) {
			ak_print_error_ex("set ircut day fail\n");
			return ;
		}
	} else if (condition == 1 ) {
		ak_print_normal("set ircut night\n");
		ret = ak_drv_ir_set_ircut(IRCUT_STATUS_NIGHT);
		if (0 != ret) {
			ak_print_error_ex("set ircut night fail\n");
			return ;
		}
	}

	ak_print_normal("ircutdemo finish\n");
}

static int cmd_ircut_reg(void)
{
    cmd_register("ircutdemo", cmd_ircut_demo, help);
    return 0;
}

cmd_module_init(cmd_ircut_reg)




