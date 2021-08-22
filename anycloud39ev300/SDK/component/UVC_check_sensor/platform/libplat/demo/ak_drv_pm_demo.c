#include "command.h"
#include "ak_common.h"
#include "ak_drv_pm.h"

static char* help[]={
	"pmdemo module",
	"      get_bat_value\n"
};


void cmd_pm_test(int argc, char **args)
{
	int vbat_value = 0;
	if(ak_drv_pm_get_bat_value(&vbat_value) == 0)
		{
			ak_print_normal("vbat_value:%d\n",vbat_value);
		}
	else
		{
			ak_print_normal("vbat_value:Error\n");
		}
}


static int cmd_pm_reg(void)
{
    cmd_register("pmdemo", cmd_pm_test, help);
    return 0;
}

cmd_module_init(cmd_pm_reg)
