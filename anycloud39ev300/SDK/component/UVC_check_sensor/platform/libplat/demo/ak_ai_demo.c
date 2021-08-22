#include "command.h"
#include "ak_ai.h"
#include "ak_common.h"

#include "kernel.h"

#define enable 1
#define AI_TCARD_FREE_SIZE          1024 // T  Card  the lease  free  size (1M)      


static char *help[]={
	"aidemo module",
	"usage:aidemo <time> <samplerate>\n"
	"      example:aidemo 10 16\n"
	"      please insert tf card ,aidemo will save pcm data to a:/**.pcm\n"
	
	
};

/**
 *ai_get_disk_free_size - get disk free size, appoint the dir path
 * @path[IN]: disk dir full path
 * return: disk free size in KB, -1 failed
 */

unsigned long ai_get_disk_free_size(const char *path)
{
	struct statfs disk_statfs;
    unsigned long free_size;
	
	bzero( &disk_statfs, sizeof(struct statfs));

	if( statfs( path, &disk_statfs ) == -1 ) 
	{		
		ak_print_error("fs_get_disk_free_size error!\n"); 
		return -1;
	}
	
    free_size = disk_statfs.f_bsize;
    free_size = free_size / 512;
    free_size = free_size * disk_statfs.f_bavail / 2;   
	return free_size;
}

void cmd_ai_test(int argc, char **args)
{
	int total_ts = 0;
	unsigned int samplerate = 0;
	total_ts = atoi(args[0]) * 1000;
	samplerate = atoi(args[1]) * 1000;
	if(argc == 0)
	{	
		total_ts = 30 * 1000;	
		samplerate = 8 * 1000;
	}
	else if(argc == 1)
	{
		if(total_ts > 0)
		{
			samplerate = 8 * 1000;
		}
		else
		{
			ak_print_error_ex("Input command error:total_ts must large than 0\n");    	
	    	ak_print_normal("%s\n",help[1]);
			return ;
		}

	}	
	else if(argc == 2)
	{
		if(total_ts < 0)
		{
			ak_print_error_ex("Input command error:total_ts must large than 0\n");    	
	    	ak_print_normal("%s\n",help[1]);
			return ;
		}
		else if((samplerate != 8 * 1000) && (samplerate != 16 * 1000))
		{
			ak_print_error_ex("Input command error:samplerate = 8/samplerate = 16\n");    	
	    	ak_print_normal("%s\n",help[1]);
			return ;
		}

	}
	
	
    struct ak_date systime;
	unsigned long first_ts = 0;
	unsigned long free_size = 0; 
    int ret = -1;
	bool first_get_flag = false;
	struct pcm_param input;
	struct frame frame = {0};
	char * path = "a:/";
	char file_path[255] = {0};
	
	free_size = ai_get_disk_free_size(path);//get T_card free size	
	if(free_size < AI_TCARD_FREE_SIZE)
	{		
		ak_print_error_ex("free_size < AI_TCARD_FREE_SIZEKB\n",AI_TCARD_FREE_SIZE);
		return ;
	}

	memset(&systime, 0, sizeof(systime));
	ak_get_localdate(&systime);
	sprintf(file_path, "%sAI_%04d%02d%02d_%02d%02d%02d.pcm", path,systime.year, systime.month , 
	systime.day , systime.hour, systime.minute, systime.second);
	
	FILE *p;
	p = fopen(file_path, "w+");
	if(NULL == p) 
	{
		ak_print_error_ex("create pcm file err\n");
		return ;
	}
	
	ak_print_normal("*******************  ADC sample test	********************\n");
	ak_print_normal("version: %s\n\n", ak_ai_get_version());
	
	input.sample_bits = 16;
	input.channel_num = 1;
	input.sample_rate = samplerate;
	void *adc_drv = ak_ai_open(&input);
	if(NULL == adc_drv)
		return ;
	ak_ai_set_source(adc_drv, AI_SOURCE_MIC);
	if(ak_ai_set_frame_interval(adc_drv, 20) == 0)
	{
		ak_print_normal("set_frame_interval success!\n");
	}
	else
	{
		ak_print_error_ex("set_frame_interval error!\n");
		goto ai_close;
	}
	ak_ai_clear_frame_buffer(adc_drv);
	
	if(ak_ai_set_volume(adc_drv, 7)==0)
	{
		ak_print_normal("set gain success!\n");
	}
		
	else
	{	
		ak_print_error_ex("set gain error!\n");
		goto ai_close;
		
	}
	ak_ai_set_resample(adc_drv,  enable);
	ak_ai_set_nr_agc(adc_drv, enable);
	
	while(1)
	{
		ret = ak_ai_get_frame(adc_drv, &frame, 1);
	
		if(ret == 0)
		{
			if(!first_get_flag)
			{
				first_ts = frame.ts;
				first_get_flag = true;
				ak_print_normal("first_ts=%d\n",first_ts);
			}
			ak_print_normal("frame.ts=%d\n",frame.ts);
			if(fwrite(frame.data,frame.len,1,p) <= 0)
			{
				ak_print_error_ex("write file err\n");
				goto ai_close;			
			}
			
			if((frame.ts - first_ts) >= total_ts)
			{
				
				break;
			}
			ak_ai_release_frame(adc_drv, &frame);
		}

	}
	
  	ak_print_normal("*******************   end	 ********************\n");
ai_close:	
	ak_ai_release_frame(adc_drv, &frame);
  	ak_ai_close(adc_drv);
	fclose(p);
	adc_drv = NULL;
	ak_print_normal("CLOSE:\n");	
}


static int cmd_ai_reg(void)
{
    cmd_register("aidemo", cmd_ai_test, help);
    return 0;
}

cmd_module_init(cmd_ai_reg)

