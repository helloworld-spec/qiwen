#include <string.h>

#include "kernel.h"
#include "command.h"
#include "ak_common.h"
#include "ak_drv_usbdisk.h"



static char *help_udiskdemo[]={
	"udisk module demo",
"     usage: udiskdemo\n"
};



static void cmd_udisk_demo(int argc, char **args)
{

	ak_print_normal("udsisk start...\n");	
	ak_drv_udisk_start();
	
	ak_print_normal("udsisk stop...\n");	
	ak_drv_udisk_stop();
		
}

static int cmd_udisk_reg(void)
{
	cmd_register("udiskdemo", cmd_udisk_demo, help_udiskdemo);
    return 0;
}

cmd_module_init(cmd_udisk_reg)

