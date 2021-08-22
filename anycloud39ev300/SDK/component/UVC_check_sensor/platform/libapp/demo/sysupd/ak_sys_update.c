/******************************************************
 * @brief  system update  demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-17 
*******************************************************/
#include <string.h>

#include "command.h"
#include "sys_common.h"
#include "ak_common.h"
#include "ak_partition.h"
#include "kernel.h"

/******************************************************
  *                    Constant         
  ******************************************************/
static char *help_update[]={
	"system update","\
     usage: sysupd <file name>\r\n\
     Note:  please place xxx.bin at the content(a:/)\r\n"
};

/******************************************************
  *                    Macro         
  ******************************************************/
#define BUF_SIZE	(4*1024)

/******************************************************
  *                    Type Definitions         
  ******************************************************/

/******************************************************
  *                    Global Variables         
  ******************************************************/

/******************************************************
*               Function prototype                           
******************************************************/


/******************************************************
*               Function Declarations
******************************************************/

/*****************************************
 * @brief mount sd card
 * @param void
 * @return on success return 0 , fail return -1 
 *****************************************/
static int cmd_sd_mount(void)
{
	int ret;
	ret = ak_mount_fs(DEV_MMCBLOCK, 0, "");
	if (0 == ret)
		ak_print_normal("mount sd ok!\n");
	else
		ak_print_error("mount sd fail!\n");

	return ret;
}

/*****************************************
 * @brief start function for command
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
static void cmd_partition_KernelUpdate(const char *name)
{
 	void *handle =NULL;
    char buf[BUF_SIZE]={0};
	int ret=0,ret1=0;
	signed long size=0,offset=0;
	char state =0;
	
	FILE * fp = fopen(name,"r");
	if (fp == NULL)
	{
		ak_print_error("open %s err!\n",name);
		ak_unmount_fs(DEV_MMCBLOCK, 0, "");
		return ;
	}
	
	fseek(fp,0L,SEEK_END);
	size= ftell(fp);
	fseek(fp,0L,SEEK_SET);
	
	if(size < 0)
	{
		ak_print_error("no file: %s\n",name);
		goto err;
	}
	else
	{
		ak_print_normal("open %s success!\n",name);
	}
	size = (size/(BUF_SIZE)/10)*BUF_SIZE;
	
	handle = ak_partition_open("KERNEL"); 
	if(handle == NULL)
	{
		ak_print_error("\nwrite parti open err!\n");
		goto err;
	}
	
	while(1)
	{
		ret1= fread(buf, BUF_SIZE, 1, fp);
		offset += ret1;
		if(offset >= size)
		{
			offset = 0;
			state++;
			if(state <= 9)
			{
            	ak_print_normal("~~~~~~~~updating:%d%~~~~~~~~~~\n",state*10);
			}
		}
		
		ret=ak_partition_write(handle, buf, ret1);
		if(ret < -1)
		{
			ak_print_error("\nupdate err!\n");
			goto err1;
		}
		
		if(BUF_SIZE != ret1)
		{	
		    ak_print_normal("~~~~~~~~updated:100%~~~~~~~~~~\n");
			break;
		}
	}
	
	ak_partition_close(handle);
	fclose(fp);
	/* software reset */
	sys_reset();
	
    return;
	
err1:
	ak_partition_close(handle);
err:
	fclose(fp);
}


/*****************************************
 * @brief start function for command
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
static void cmd_sys_update_demo(int argc, char **args)
{

	/* mount sd card*/
	if(cmd_sd_mount())
		return;
		
	char name[100]="a:/";
	char *bin_name = "sky39ev200.bin";

	/** step1: process command param */
	if ( (1 == argc) && (char *)args[0] != NULL)
    {
		strcat(name,args[0]);
    }
	else if (0 == argc)
	{
		strcat(name,bin_name);		
	}
	else
	{
		ak_print_error("%s\n",help_update[1]);
		return;
	}

	/** step2: update kernel*/
	cmd_partition_KernelUpdate(name);
}

/*****************************************
 * @brief register sysupd command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_sys_update_reg(void)
{
	cmd_register("sysupd", cmd_sys_update_demo, help_update);
	return 0;
}

cmd_module_init(cmd_sys_update_reg)

