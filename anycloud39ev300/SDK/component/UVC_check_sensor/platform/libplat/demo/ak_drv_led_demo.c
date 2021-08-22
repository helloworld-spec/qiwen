#include <string.h>
#include "ak_common.h"
#include "command.h"
#include "ak_drv_led.h"

static char LED_status[2] = {0,0};  //0 :not open    ;  1  :  open

static int *fd[2] = {NULL, NULL};
void cmd_led_demo(int argc, char **args) 
{
	int i;
	int led_id;
	int retval = -1;
	if(argc != 2)
	{
		ak_print_error("input argument format error!\n");
		return ;
	}
	led_id = atoi(args[0])-1;
	if(0 == LED_status[led_id])
	{
		fd[led_id] = ak_drv_led_open(led_id);   //open LED1
		if((*fd[led_id]) == NULL)
		{
			ak_print_error("open led fail\n");
			return ;
		}
		else
		{
			LED_status[led_id] = 1;
		}
	}
	if(0 == strcmp("on",args[1]))
	{
		retval = ak_drv_led_on(fd[led_id]);     //set LED1 on
		if(0 == retval)
			ak_print_normal("led on ok\n");
	}
	else if(0 == strcmp("off",args[1]))
	{
		retval = ak_drv_led_off(fd[led_id]);     //set LED1 off
		if(0 == retval)
			ak_print_normal("led off ok\n");
	}
	else if(0 == strcmp("blink",args[1]))
	{
		retval = ak_drv_led_blink(fd[led_id],(led_id+1)*1000,1000);     //set LED1 off
		if(0 == retval)
			ak_print_normal("led blink ok\n");
	}
	else
	{
		ak_print_error("input condition error!\n");
		return ;
	}
	
}


static char *help[]={
	"led module demo",
	"usage:leddemo [led_id][condition]\n"
	"condition: on/off\n"
};

static int cmd_led_reg(void)
{
    cmd_register("leddemo", cmd_led_demo, help);
    return 0;
}

cmd_module_init(cmd_led_reg)


