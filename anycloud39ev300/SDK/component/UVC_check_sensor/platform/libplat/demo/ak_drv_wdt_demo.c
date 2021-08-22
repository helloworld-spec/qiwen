/******************************************************
 * @brief  adec demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-23 
*******************************************************/
#include "ak_drv_wdt.h"
#include "ak_common.h"
#include "command.h"

/******************************************************
  * 				   Global Variables 		
 ******************************************************/
static char *help[]={
	"watchdog test module",
	"usage:wdtdemo <feedtime>\n"
	"feedtime: 1~300000\n"
	"restart system after feedtime(ms)\n"
};
/******************************************************
*				Function prototype							 
******************************************************/
/**
 * @brief watchdog test demo
 * argv[0]: reset wait time (ms)
 */
void cmd_watchdog_demo(int argc, char **args)
{
	unsigned int feed_time = 3000, t;
    unsigned char c;
	unsigned int fd;

	feed_time = 3000;
	/***********arguments checking*******/
	if(argc > 1)
	{
		ak_print_error("input argument format error!\n");
		return ;
	}
	
	if(argc == 1)
	{
		feed_time = atoi(args[0]);
		if(feed_time == 0|| feed_time >300000)
		{
			ak_print_error("input feedtime error! feed_time =%s\n", args[0]);
			return ;
		}
	}
    //start watchdog timer
    fd = ak_drv_wdt_open(feed_time);    
	if(0 != fd)
	{
		ak_print_error("open watchdog fail!\n");
		return ;
	}
    while(1)
    {
        ak_print_normal("Enter any key to feed(e to exit):\n");
        c = getch();

        if('e' == c)
        {
            ak_print_normal("Exit loop\r\n");
            break;
        }
        else
        {
            ak_drv_wdt_feed();    //feed watchdog
        }
    }
    ak_drv_wdt_close();    //close watchdog
	ak_print_normal("watchdog stopped\n");
}


/*****************************************
 * @brief register wdtdemo command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_watchdog_reg(void)
{
	cmd_register("wdtdemo", cmd_watchdog_demo, help);
    return 0;
}

/**
 *  module init
 */

cmd_module_init(cmd_watchdog_reg)

