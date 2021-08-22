/******************************************************
 * @brief  adec demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-23 
*******************************************************/
#include <string.h>
#include "ak_common.h"
#include "command.h"
#include "ak_drv_led.h"

/******************************************************
  * 				   Global Variables 		
 ******************************************************/
static char *help[]={
	"i2c module demo",
	"usage:i2cdemo [opreation] [dev_addr] [reg_addr] [reg_type] [len] <data>\n"
	"opreation: w/r\n"
	"dev_addr: decimal, example: 0x68 -> 104\n"
	"reg_addr: decimal, example: 0x36 -> 54\n"
	"reg_type: 0: Byte, 1:word\n"
	
};


/******************************************************
*               Function Declarations
******************************************************/
/**
 * @brief i2c test demo
 * argv[0]: dev_addr
 * argv[1]: reg_addr

 */
void cmd_i2c_demo(int argc, char **args) 
{
	int i;
	unsigned char dev_addr, len,reg_type;
	unsigned short reg_addr;
	unsigned char data[16];
	void * handle;
	/*****arguments    checking*********/
	if(argc < 4)
	{
		ak_print_error("input argument format error!\n");
		ak_print_error("%s",help[1]);
		return ;
	}
	
	dev_addr = atoi(args[1]);
	reg_addr = atoi(args[2]);
	reg_type = atoi(args[3]);
	len = atoi(args[4]);
	handle = ak_drv_i2c_open(dev_addr,reg_type);
	if(NULL == handle)
	{
		ak_print_error("dev_addr error!\n");
		return ;
	}
	ak_print_normal("%s dev:0x%x, reg:0x%x,len:%d, data_addr: %x\n ",args[0], dev_addr, reg_addr, len,data);
	if(0 == strcmp("w",args[0]))
	{
		//ak_print_normal("dev:0x%x, reg:0x%x,len:%d\n ", dev_addr, reg_addr, len);
		for(i = 0; i < len; i++)
		{
			data[i] = atoi(args[5+i]);
			ak_print_normal("0x%x ", data[i]);
		}
		if(-1 != ak_drv_i2c_write(handle,reg_addr, data, len))
		{
			ak_print_normal("\nwrite success! \n");
		}
		else
		{
			ak_print_error("\nwrite fails! \n");
		}
	}
	else if(0 == strcmp("r",args[0]))
	{
		if(-1 != ak_drv_i2c_read(handle, reg_addr, data, len))
		{
			for(i = 0; i < len; i++)
			{
				ak_print_normal("0x%x ", data[i]);
			}
			ak_print_normal("\nread success! \n");
		}
		else
		{
			ak_print_error("read fails! \n");
		}
	}
	else
	{
		ak_print_error("opereation error!\n");
		return ;
	}
	
	ak_drv_i2c_close(handle);
	
}



/*****************************************
 * @brief register leddemo command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_i2c_reg(void)
{
    cmd_register("i2cdemo", cmd_i2c_demo, help);
    return 0;
}

/**
 *  module init
 */
cmd_module_init(cmd_i2c_reg)



