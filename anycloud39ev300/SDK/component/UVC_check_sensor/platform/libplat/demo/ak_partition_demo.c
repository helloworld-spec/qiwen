/******************************************************
 * @brief  partition  demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-17 
*******************************************************/
#include "command.h"
#include "ak_common.h"
#include "ak_partition.h"
#include "kernel.h"

/******************************************************
  *                    Constant         
  ******************************************************/
static char *help_parti[]={
	"partition demo","\
     usage: parti [w/r]\r\n\
     read or write MAC partition\r\n"
};

/******************************************************
  *                    Macro         
  ******************************************************/
#define PARTI_W			"w"
#define PARTI_R			"r"

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
 * @brief read mac from MAC partition
 * @param void
 * @return void
 *****************************************/
static void cmd_partition_read()
{
    void *handle = NULL;
	char mac[21]={0};
	int i,ret=0;
	unsigned int len=0;
	
	handle = ak_partition_open("MAC");  
	if(handle == NULL)
	{
		ak_print_error("\nread parti open err!\n");
	}

	// show partition lib version
	ak_print_normal("==========%s==========\n", ak_partition_get_version());
	
	ret=ak_partition_read(handle, mac, 21);
	if(ret < -1)
	{
		ak_print_error("\nread err!\n");
	}
	
	memcpy(&len, mac, 4);
	ak_print_normal("\nread len %d,datlen:%d\n",ret,len);
	for(i=4;i<21;i++)
	{
		ak_print_normal("%c\n",mac[i]);
	}
	
	ak_partition_close(handle);
}

/*****************************************
 * @brief write mac to MAC partition
 * @param void
 * @return void
 *****************************************/
static void cmd_partition_write(void)
{
	void *handle =NULL;
	int len=0,ret=0;
	int i;
	char mac[]={0x11,0,0,0,'F','F',':','F','F',':','F','F',':','0','8',':','0','8',':','0','9'};

	for(i=4;i<21;i++)
	{
		ak_print_normal("%c\n",mac[i]);
	}
	
	handle = ak_partition_open("MAC"); 
	if(handle == NULL)
	{
		ak_print_error("\nwrite parti open err!\n");
	}
	
	memcpy(&len, mac, 4);
	ak_print_normal("\nw len %d\n",len);
	
	ret=ak_partition_write(handle, mac, 21);
	if(ret < -1)
	{
		ak_print_error("\nw err!\n");
	}	
	
	ak_partition_close(handle);
	
}

/*****************************************
 * @brief start function for command
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
static void partition_demo(int argc, char **args)
{
	if (argc>0 && (char *)args[0] != NULL)
    {
		if (strcmp(args[0], PARTI_W) == 0)
		{
			cmd_partition_write();
			return;
		}
		else if (strcmp(args[0], PARTI_R) == 0)
		{
			cmd_partition_read();
			return ;
		}
		
   }
	ak_print_error("%s\r\n",help_parti[1]);
}

/*****************************************
 * @brief register parti command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_partition_reg(void)
{
	cmd_register("parti", partition_demo, help_parti);
	return 0;
}

cmd_module_init(cmd_partition_reg)

