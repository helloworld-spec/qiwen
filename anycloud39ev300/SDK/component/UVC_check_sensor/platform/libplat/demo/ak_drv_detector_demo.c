#include "ak_common.h"
#include "command.h"
#include "ak_drv_detector.h"

static char *help[]={
	"detect module demo",
	"usage:detectdemo [function]\n"
	"function: 0:no obstruct, obstruct time(s),-1: obstruct\n"
};

static int ak_isdigit(char * argument,unsigned char len)
{
	unsigned char i;
	for(i = 0; i < len; i++)
	{
		if('0'>argument[i] || '9'<argument[i])
			return -1;
	}
	return 0;
}

void cmd_poll_detect()
{
	int *fd;
	int event;
	fd = ak_drv_detector_open(SD_DETECTOR); //open SD detect
	if(NULL == fd)
	{
		ak_print_error("open detector fail\n");
		return ;
	}
	
	if(-1 == ak_drv_detector_mask(fd, 1))
	{
		ak_print_error("open detector enable fail\n");
		return ;
	}
	//while(1)
	//{
		if(-1 != ak_drv_detector_poll_event(fd, &event))   //get SD detect status
		{
			if(event)ak_print_normal("sd have\n");
			else ak_print_normal("no sd\n");
		}
		else ak_print_error("no get\n");
	//}
	ak_drv_detector_close(fd);
}

void cmd_no_obstruct_detect(void)
{
 	int *fd;
	int event;
	fd = ak_drv_detector_open(SD_DETECTOR); //open SD detect
	if((*fd) == (-1))
	{
		ak_print_error("open detector fail\n");
		return ;
	}
	
	if(-1 == ak_drv_detector_mask(fd, 1))
	{
		ak_print_error("open detector enable fail\n");
		return ;
	}
	while(1)
	{
		if(-1 != ak_drv_detector_wait_event(fd, &event, 0))
		{
			if(event)ak_print_normal("sd have\n");
			else ak_print_normal("no sd\n");
		}
		else ak_print_error("no get\n");
	}
	ak_drv_detector_close(fd);
}
void cmd_obstruct_detect(void)
{
 	int *fd;
	int event;
	fd = ak_drv_detector_open(SD_DETECTOR); //open SD detect
	if((*fd) == (-1))
	{
		ak_print_error("open detector fail\n");
		return ;
	}
	
	if(-1 == ak_drv_detector_mask(fd, 1))
	{
		ak_print_error("open detector enable fail\n");
		return ;
	}
	
	if(-1 != ak_drv_detector_wait_event(fd, &event, -1))
	{
		if(event)ak_print_normal("sd have\n");
		else ak_print_normal("no sd\n");
	}
	else ak_print_error("no get\n");
	
	ak_drv_detector_close(fd);
}

void cmd_ms_obstruct_detect(int time_s)
{
 	int *fd;
	int event;
	fd = ak_drv_detector_open(1);  //open SD detect
	if((*fd) == (-1))
	{
		ak_print_error("open detector fail\n");
		return ;
	}
	
	if(-1 == ak_drv_detector_mask(fd, 1))
	{
		ak_print_error("open detector enable fail\n");
		return ;
	}
	ak_print_normal("wait %d s,for tcard status change\n",time_s);
	if(-1 != ak_drv_detector_wait_event(fd, &event, time_s*1000))
	{
		if(event)ak_print_normal("sd have\n");
		else ak_print_normal("no sd\n");
	}
	else ak_print_error("no get\n");
	
	ak_drv_detector_close(fd);
}
void cmd_detect_test(int argc, char **args)
{
	/*************arguments checking ****************/
	if(1 != argc)
	{
		ak_print_error("input argument format error!\n");
		ak_print_error("%s",help[1]);
		return ;
	}
	unsigned char function_num;
	if(strcmp("-1",args[0]) == 0)
	{
		cmd_obstruct_detect();
	}
	else
	{
		if(0 == ak_isdigit(args[0],strlen(args[0])))
		{
			function_num = atoi(args[0]);
			//ak_print_normal("%s,%d\n",args[0], function_num);
			if(0 == function_num)
			{
				cmd_poll_detect();
			}
			else
			{
				cmd_ms_obstruct_detect(function_num);
			}
		}
		else
			ak_print_error("input argument error!\n%s",help[1]);
	}
}



static int cmd_detect_reg(void)
{
    cmd_register("detectdemo", cmd_detect_test, help);
    return 0;
}

cmd_module_init(cmd_detect_reg)

