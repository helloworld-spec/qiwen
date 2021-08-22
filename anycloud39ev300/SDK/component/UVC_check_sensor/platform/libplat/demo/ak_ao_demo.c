/******************************************************
 * @brief  adec demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-23 
*******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "ak_ao.h"
#include "ak_common.h"
#include "ak_global.h"
#include "kernel.h"

/******************************************************
  *                    Macro         
  ******************************************************/
/******************************************************
  * 				   Type Definitions 		
  ******************************************************/
/******************************************************
  * 				   Global Variables 		
 ******************************************************/
volatile static bool ao_break = false;
static char *help[]={
	"audio out module demo",
	"usage:aodemo [filename] <samplerate> <channelnum>\n"
	"filename : must is pcm data file\n"
	"samplerete : 8000~44100\n"
	"channelnum : mono/stereo\n"
};
/******************************************************
*               Function Declarations
******************************************************/
/******************************************************
*				Function prototype							 
******************************************************/

/*****************************************
 * @brief signal callback to return ao demo
 * @param argc[in] signo :
 * @return void
 *****************************************/
static void cmd_ao_signal(int signo)
{
    ao_break = true;
}
/**
 * @brief audio out demo,file frome SD card  
 * argv[0]: filename
 * argv[1]: sample_rate
 * argv[2]: channel_num
 */
void cmd_SD_file_dac_test(int argc, char **args) 
{
	struct pcm_param info;
	int ret, i;
	unsigned int tmpBuf_size = 0;
	int *handle = NULL;
	unsigned char file_over = 0;
	int send_size, size;
	unsigned char *tmpBuf = NULL;	
	
	info.sample_bits = 16;
	info.sample_rate = 8000;
	info.channel_num = 1;

	/*************arguments checking ****************/
	if(argc > 3 || argc < 1)
	{
		ak_print_error("input argument format error!\n");
		ak_print_error("%s",help[1]);
		return ;
	}
	if(argc == 3)
	{
		if(strcmp("mono",args[2]) == 0)
		{
			info.channel_num = 1;
		}
		else if(strcmp("stereo", args[2]) ==0 )
		{
			info.channel_num = 2;
		}
		else
		{
			ak_print_error("input argument channel_num error!\n");
			ak_print_error("%s",help[1]);
			return ;
		}
	}
	if(argc >= 2)
	{
		info.sample_rate = atoi(args[1]);
		if((info.sample_rate < 8000) || (info.sample_rate > 44100))
		{
			ak_print_error("input argument sample rate error!\n");
			ak_print_error("%s",help[1]);
			return ;
		}
	}
	/****mount SD card******************/
	if (0 ==ak_mount_fs(DEV_MMCBLOCK, 0, ""))
		ak_print_normal("ao mount sd ok!\n");
	else
		ak_print_error("ao mount sd fail!\n");
	
	//open  a file for read	
	FILE * fd = fopen(args[0], "r" );
	if (fd ==NULL)
	{
		ak_print_error("open file failed!\n");
		goto mount_end;
	}
	else
	{
		ak_print_normal("open file success!\n");
	}
	//open ao
	handle = ak_ao_open(&info);
	if(fd == NULL)
	{
		ak_print_error("ao open false\n");
		goto fp_end;
	}
	
	//printf version
	ak_print_normal("%s\n", ak_ao_get_version());

	//printf frame_size
	tmpBuf_size = ak_ao_get_frame_size(handle);
	ak_print_normal("ao frame size =%d\n", tmpBuf_size);

	//tmpbuf: stereo data to DA
	tmpBuf = (unsigned char *)malloc(tmpBuf_size);
	if(NULL == tmpBuf)
	{
		ak_print_error("ao demo malloc false!\n");
		goto ao_end;
	}
	
	//set volume
	ak_ao_set_volume(handle, 0x05);
	
	ak_print_normal("star trans\n");
	//open speaker
	ak_ao_enable_speaker(handle,1);
	//init signal
	ao_break = false;
	//register signal callback
    cmd_signal(CMD_SIGTERM, cmd_ao_signal);
	
	while (!ao_break)
	{
		if(1 == info.channel_num)    
		{
			//read SD card file data		
			ret = fread(tmpBuf, tmpBuf_size/2,1,fd);
			//judge file end
			if(tmpBuf_size/2 != ret)
			{  
				file_over = 1;
			}	
		}
		else
		{
			//read SD card file data
			ret = fread(tmpBuf, tmpBuf_size,1,fd);
			//judge file end
			if(tmpBuf_size != ret)
			{  
				file_over = 1;
			}	
			
		}
		send_size = 0;
		while(ret > 0)
		{
			//send data to DA
			size = ak_ao_send_frame(handle, tmpBuf + send_size, ret, 0);
			if(size < 0)
			{
				ak_print_error("ao send frame error!  %d \n", size);
				break;
			}
			else
			{
				send_size += size; 
				ret = ret - size;
			}
		}
		if(file_over)
			break;
	}
	
	mini_delay(900);  //wait data to DAC
	//close SPK
	ak_ao_enable_speaker(handle,0);
		
tmpbuf_end:
	//free temBuff
	free(tmpBuf);
ao_end:
	//close ao
	ak_ao_close(handle);
fp_end:
	//close file 
	fclose(fd);
mount_end:
	if (0 ==ak_unmount_fs(DEV_MMCBLOCK, 0, ""))
		ak_print_normal("ao unmount sd ok!\n");
	else
		ak_print_error("ao unmount sd fail!\n");
	
}

/*****************************************
 * @brief register aodemo command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_audio_reg(void)
{
    cmd_register("aodemo", cmd_SD_file_dac_test, help);
    return 0;
}


/**
 *  module init
 */

cmd_module_init(cmd_audio_reg)
	

