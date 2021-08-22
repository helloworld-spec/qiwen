/******************************************************
 * @brief  ini  demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-17 
*******************************************************/
#include "command.h"
#include "ak_common.h"
#include "ak_ini.h"
#include "kernel.h"

/******************************************************
  *                    Constant         
  ******************************************************/
static char *help_ini[]={
	"ini test demo.",	
"     usage: inidemo set [title] [item] [value]\n\
             inidemo get [title] [item]\n\
             inidemo del [title] <item>\n"
};

/******************************************************
  *                    Macro         
  ******************************************************/
#define INI_SET			"set"
#define INI_GET			"get"
#define INI_DEL			"del"

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
 * @brief set item value by item's title and key
 * @param handle[in]  config file handle
 * @param title[in]  config title
 * @param item[in]  config item
 * @param item[in]  config value
 * @return void
 *****************************************/
static void ini_set_demo(const void *handle, 
		const char *title, const char *item, const char *value)
{
	if(ak_ini_set_item_value(handle, title, item, value) != 0)
	{
		ak_print_error("set %s->%s fail\n",title, item);
	}
	else
	{
		ak_print_normal("set %s->%s ok\n",title, item);
	}
}

/*****************************************
 * @brief get item value by item's title and key
 * @param handle[in]  config file handle
 * @param title[in]  config title
 * @param item[in]  config item
 * @param item[out]  config value
 * @return void
 *****************************************/
static void ini_get_demo(const void *handle, 
		const char *title, const char *item, char *value)
{
	
	// get item value
	if(ak_ini_get_item_value(handle, title, item, value) != 0)
	{
		ak_print_error("get item:%s->%s fail\n",title, item);
	}
	else
	{
	    ak_print_normal("item: %s = %s\n", item, value);
	} 
}

/*****************************************
 * @brief delete one itemby titile and item key
 * @param handle[in]  config file handle
 * @param title[in]  config title
 * @param item[in]  config item
 * @return void
 *****************************************/
static void ini_del_item_demo(const void *handle, 
		const char *title, const char *item)
{
    int ret = 0;
	
	ret = ak_ini_del_item(handle, title, item);
	if(ret < 0 )
	{
		ak_print_error("del item:%s fail\n",item);	
	}
	else
	{
		ak_print_normal("del item:%s ok\n",item);
	}
}

/*****************************************
 * @brief delete all items in titile
 * @param handle[in]  config file handle
 * @param title[in]  config title
 * @return void
 *****************************************/
static void ini_del_title_demo(const void *handle, 
		const char *title)
{
    int ret = 0;
	
	ret = ak_ini_del_title(handle, title);
	if(ret < 0)
	{
		ak_print_error("del title:%s fail\n", title);	
	}
	else
	{
		ak_print_normal("del title:%s ok\n", title);
	}
}

/*****************************************
 * @brief start function for command
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
static void cmd_ini_demo(int argc, char **args)
{
	void *cfg_handle = NULL;
	char cfg_file[20]= "APPINI";
	char titl[100]={0};
	char itm[100]={0};
	char val[100]={0};
	int i=0;
	unsigned char cmd=0;
	
	ak_print_normal("ini demo compile:%s %s %s\n",__DATE__,
			__TIME__,ak_ini_get_version());

	if (argc < 2)
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
	      break;
	   case 1:
	   	  strcpy(titl, (char *)args[1]);	
		  break;
	   case 2:
	   	  strcpy(itm, (char *)args[2]);	
		  break;
	   case 3:
	   	  strcpy(val, (char *)args[3]);	
		  break;
	   default:
	   	  goto cmd_err;
		  break;
	   }
	}

   if (strcmp(args[0], INI_SET) == 0)
   {    
        if(argc > 3)
        {
   			cmd = 1;
        }
   }
   else if(strcmp(args[0], INI_GET) == 0)
   {
	    if(argc == 3)
        {
   			cmd = 2;
        }
   }
   else if(strcmp(args[0], INI_DEL) == 0)
   {
	    if(3 == argc)
        {
   			cmd = 3;
        }
		else if(2 == argc)
        {
   			cmd = 4;
        }	
   }
   else
   {
		goto cmd_err;
   }
	
   if(cmd > 0)
   {
   	 if((cfg_handle = ak_ini_init(cfg_file)) == NULL)
	 {
		ak_print_error("open ini fail.\n");
		return;
	 }

	 ak_print_normal("open ini success.\n");
   }
   switch(cmd)
   {
   case 0:
   		goto cmd_err;
		break;
   case 1:
		ini_set_demo(cfg_handle, titl, itm, val);
		break;
   case 2:
		ini_get_demo(cfg_handle, titl, itm, val);
		break;
   case 3:
		ini_del_item_demo(cfg_handle, titl, itm);
		break;
   case 4:
	    ini_del_title_demo(cfg_handle, titl);
		break;
   default:
		break;
	}	
	ak_ini_destroy(cfg_handle);		

  return;
  
cmd_err:
	
	ak_print_error("%s\n",help_ini[1]);
                    
}

/*****************************************
 * @brief register inidemo command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_ini_demo_reg(void)
{
    cmd_register("inidemo", cmd_ini_demo, help_ini);
    return 0;
}

cmd_module_init(cmd_ini_demo_reg)

