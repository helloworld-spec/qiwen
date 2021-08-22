/******************************************************
 * @brief  uart 1  demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-17 
*******************************************************/
#include <string.h>
#include "command.h"
#include "ak_common.h"
#include "ak_drv_uart1.h"
#include "kernel.h"

/******************************************************
  *                    Constant         
  ******************************************************/
static char *help_uart1demo[]={
	"uart1 module demo",
"     usage: uart1demo write [string]\n\
             uart1demo read [length]\n\
             uart1demo set [baud_rate]\n"
};

/******************************************************
  *                    Macro         
  ******************************************************/
#define  BUFF_SIZE     		1024
#define REPORT_BUFFER_SIZE 	256
#define BAUD_RATE_115200	115200

#define BAUD_RATE_MAX	(128000)
#define MAXSIZE (10*1024)

#define	WR	"write"
#define RD	"read"
#define	SET	"set"

#define READ_SIZE 1000
#define MS -1

/******************************************************
  *                    Type Definitions         
  ******************************************************/

/******************************************************
  *                    Global Variables         
  ******************************************************/
static unsigned int baud_rate = BAUD_RATE_115200;

/******************************************************
*               Function prototype                           
******************************************************/


/******************************************************
*               Function Declarations
******************************************************/

/*****************************************
 * @brief open uart1
 * @param baud_rate[in]  baud rate
 * @return void
 *****************************************/
static void cmd_uart1_open(unsigned int baud_rate)  
{
	int ret;
	
	if((0 == baud_rate)||(baud_rate > BAUD_RATE_MAX))
	{
		baud_rate = BAUD_RATE_115200;
	}
	
	ak_print_normal("set uart1 baud_rate:%d\r\n",baud_rate);
	ret = ak_drv_uart1_open(baud_rate,0);
	
	if (ret < 0)
	{
		ak_print_error("open uart1 fail\r\n");
	}
}

/*****************************************
 * @brief write data to uart1
 * @param data[in]  data to write
 * @param len[in]  data length
 * @return void
 *****************************************/
static void cmd_uart1_write(char *data, int len) 
{
	unsigned long ret;

	if((NULL == data)||(0 == len))
	{
		return;
	}
	
	cmd_uart1_open(baud_rate);
	ret = ak_drv_uart1_write(data, len);

	ak_print_normal("write data len %d\r\n",len);

	ak_drv_uart1_close();
}

/*****************************************
 * @brief read data friom uart1
 * @param len[in]  data length
 * @return void
 *****************************************/
static void cmd_uart1_read(unsigned int len) 
{
	unsigned long i=0;
	unsigned long ret=0,sum=0;
	char *buf = NULL;
    
    if(0 == len)
    {
		return;
	}

	if(len > MAXSIZE)
	{
		len = MAXSIZE;
	}
	
	ak_print_normal("buf size is:%d byte\n",len);
	buf = (char *)malloc(len);
	if(NULL == buf)
	{
		ak_print_error("malloc read buf err\n");
		return;
	}
	memset(buf, 0, len);
	
	cmd_uart1_open(baud_rate);
	while(1)
	{
		ret = ak_drv_uart1_read(buf,len,MS);
		
		for(i = 0; i<ret; i++)
		{
			ak_print_normal(" %02c",buf[i]);
			
			if(i%16 == 15)
			{
				ak_print_normal("\r\n");
			}
		}
		
		sum+=ret;
		
		ret = 0;
		if(sum >= len)
		{
			break;
		}
	}
	free(buf);
	ak_drv_uart1_close();
}

/*****************************************
 * @brief start function for command
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
static void cmd_uart1_demo(int argc, char **args)
{
	int i=0;
	char cmd=0;
	char val[100]={0};
	char cmd_buf[100]={0};
	int tmp=0;
	unsigned int len =0;

	/** step1 : process params */
	if (argc < 1)
	{
		goto cmd_err;
	}
	
	for(i=0; i<argc && (char *)args[i]!=NULL; i++)
	{

	   if(strlen((char *)args[i]) > 100)
	   {
	   	  goto cmd_err;
	   }
		
	   switch(i)
	   {
	   case 0:
	   	  strcpy(cmd_buf, (char *)args[0]);	
		  break;
	   case 1:
	   	  strcpy(val, (char *)args[1]);	
		  break;
	   default:
	   	  goto cmd_err;
		  break;
	   }

	}

   if (strcmp(cmd_buf, WR) == 0)
   {    
        if(argc > 1)
        {
   			cmd = 1;
        }
   }
   else if(strcmp(cmd_buf, RD) == 0)
   {
	    if(argc >1)
        {
   			cmd = 2;
			if(strlen(val)< 10)
			{
				tmp=atoi(val);
			}
			else
			{
				ak_print_error("ERR! the length value is no support\n");
				goto cmd_err;
			}
			
        }
   }
   else if(strcmp(cmd_buf, SET) == 0)
   {
	    if(argc > 1)
        {
   			cmd = 3;
			
			if(strlen(val)< 10)
			{
				tmp=atoi(val);
			}
			else
			{
				ak_print_error("ERR! the length value is no support\n");
				goto cmd_err;
			}
			
        }
   }
   else
   {
		goto cmd_err;
   }
   /** step2: judge command */
   //tmp=atoi(val);

   switch(cmd)
   {
   case 1:
      cmd_uart1_write(val, strlen(val));
   	  break;
   case 2:
	  if(tmp <= 0)
	  {
		ak_print_error("ERR! the length value is no support\n");
		goto err;
	  }
	  else if(tmp > MAXSIZE)
	  {
	  	tmp = MAXSIZE;
		ak_print_warning("WARNING! set length more than MAXSIZE:%d!\n",tmp);	
	  }
	  else
	  {
	  }
	  
   	  len = tmp;
   	  cmd_uart1_read(len);
   	  break;
   case 3:
   	  if(tmp <= 0)
	  {
		ak_print_error("ERR! the buad rate value is no support\n");
		goto err;
	  }
	  else if(tmp < 4800)
	  {
	    tmp = 4800;
		ak_print_warning("WARNING! set buad rate less than:%d!\n",tmp);
		goto err;
	  }
	  else if(tmp > BAUD_RATE_MAX)
	  {
	    tmp = BAUD_RATE_MAX;
	  	ak_print_error("ERR! set buad rate more than BAUD_RATE_MAX:%d\n",tmp);
		goto err;
	  }
	  else
	  {
	  }
	  
	  baud_rate = tmp;
	  ak_print_normal("set baud_rate success\r\n");
   	  break;
	
   default:
	  goto cmd_err;
      break;
	}
   
    return;
cmd_err:
	
	ak_print_error("%s\n",help_uart1demo[1]);
	return;
err:
	ak_print_error("please retry again!\n");	
	
}

/*****************************************
 * @brief register uart1demo command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_uart1_reg(void)
{
	cmd_register("uart1demo", cmd_uart1_demo, help_uart1demo);
    return 0;
}

cmd_module_init(cmd_uart1_reg)

