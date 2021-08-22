#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "ak_ai.h"
#include "ak_common.h"
#include "ak_ao.h"
#include "ak_thread.h"

#include "kernel.h"

     
static char *help[]={
	"aecdemo module",
	"usage:aecdemo [filename] [time] <samplerate>\n"
	"filename :   must is pcm data file\n"
	"time :       must large than 0(s)\n"
	"samplerate:  8000/16000, no open resample\n"
	"example :    test.pcm 20 16000\n"
	
};


static ak_sem_t  gWaitAd;

ak_pthread_t  ai_id, ao_id;
unsigned char  g_rec_end = 0;//aec end flag
static bool firstplay = true;
int total_ts = 0;
unsigned int SampleRate = 0;
FILE *aec,*fd;

volatile static bool aec_break = false;
unsigned char *aecBuf = NULL;
unsigned char *pcmBuf = NULL;


static void cmd_aec_signal(int signo)
{
    aec_break = true;
}

static void* ao_open()
{
	//ao parma
	struct pcm_param info;
	int ret = 0, i;		
	unsigned long pcm_indx = 0;	
	unsigned int tmpBuf_size = 0;
	int *handle = NULL;
	int send_size, size;
	unsigned char *tmpBuf = NULL;
	info.sample_bits = 16;
	info.sample_rate = SampleRate;
	info.channel_num = 1;
	
	//ao operate
	handle = ak_ao_open(&info);

	tmpBuf_size = ak_ao_get_frame_size(handle);
	ak_print_normal("ao frame size =%d\n", tmpBuf_size);
	
	tmpBuf = (unsigned char *)malloc(tmpBuf_size);
	if(NULL == tmpBuf)
	{
		ak_print_error("ao demo malloc false!\n");
		goto ao_end;
	}
	
	//set volume
	ak_ao_set_volume(handle, 0x04);	
	ak_ao_enable_speaker(handle,1);
	
	ak_print_normal("begin ao............\n");
	
	ak_thread_sem_wait(&gWaitAd);
	
	while(1)
	{	
		if(aec_break)
		{				
			break;
		}
		if(1 == info.channel_num)    
		{	
			memcpy(tmpBuf, pcmBuf + pcm_indx, tmpBuf_size/2);
			ret = tmpBuf_size/2;	
			
			pcm_indx += tmpBuf_size/2;			
		}
		else
		{			
			memcpy(tmpBuf, pcmBuf + pcm_indx, tmpBuf_size);
			ret = tmpBuf_size;
			pcm_indx += tmpBuf_size;
		}
		
		if (g_rec_end)
		{			
			break;
		}

		send_size = 0;
		
		while(ret > 0)
		{			
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
	}

	//close SPK
	ak_ao_enable_speaker(handle,0);
	
	if(pcmBuf != NULL)
	{
		free(pcmBuf);
		pcmBuf = NULL;
	}
	
tmpbuf_end:
	if(tmpBuf != NULL)
	{	
		free(tmpBuf);
	}
		
ao_end:	
	pcm_indx = 0;	
	ak_ao_close(handle);
	ak_print_normal("ao close !\n");
	ak_thread_exit();	
}

static void* ai_open()
{
	//ai parma	
	bool first_get_flag = false;
	unsigned long first_ts = 0;
    int ret = -1;
	struct pcm_param input;
	struct frame frame = {0};	
	unsigned long aec_index = 0;	
	
	//ai operate	
	input.sample_bits = 16;
	input.channel_num = 1;
	input.sample_rate = SampleRate;

	g_rec_end = 0;
	ak_print_normal("wait ao open...\n");
	
	void *adc_drv = ak_ai_open(&input);
	ak_ai_set_source(adc_drv, AI_SOURCE_MIC);
	/*
		AEC_FRAME_SIZE  is  256£¬in order to prevent droping data,
		it is needed  (frame_size/AEC_FRAME_SIZE = 0), at  the same 
		time, it  need  think of DAC  L2buf  , so  frame_interval  should  
		be  set  32ms¡£
	*/
	if(ak_ai_set_frame_interval(adc_drv, 32) != 0)
	{
		ak_print_error_ex("set_frame_size error!\n");
		goto ai_close;
	}

	ak_ai_clear_frame_buffer(adc_drv);

	if(ak_ai_set_volume(adc_drv, 7)!=0)
	{	
		ak_print_error_ex("set gain error!\n");
		goto ai_close;
		
	}
	
	ak_ai_set_aec(adc_drv, 1);
	
	ak_thread_sem_post(&gWaitAd);
	
	while(1)
	{	
	
		ret = ak_ai_get_frame(adc_drv, &frame, 1);
		
		if(ret == -1)
		{
			ak_sleep_ms(5);
			continue;							
		}
		
		if(!first_get_flag)
		{
			first_ts = frame.ts;
			first_get_flag = true;
		}
		
		memcpy(&aecBuf[aec_index], frame.data, frame.len);
		aec_index += frame.len;
			
		if((frame.ts - first_ts) >= total_ts)
		{
			g_rec_end = 1;				
			break;
		}		
		ak_ai_release_frame(adc_drv, &frame);		
		
	}
			
	if (fwrite(aecBuf, aec_index, 1, aec) < 0)
	{
		ak_print_error("fwrite aec failed!\n");
	}
	
ai_close:	
	if(aecBuf != NULL)
	{
		free(aecBuf);		
		aecBuf = NULL;
	}
	ak_ai_release_frame(adc_drv, &frame);
  	ak_ai_close(adc_drv);
	adc_drv = NULL;
	aec_index = 0;
	ak_print_normal("ai close !\n");
	ak_thread_exit();	
}


void cmd_echo_test(int argc, char **args)
{
	int ret;
	unsigned long file_len = 0;
	unsigned long data_size = 0;
	total_ts = atoi(args[1]) * 1000;
	SampleRate = atoi(args[2]);
	unsigned int real_rate_8k = 8012;	
	unsigned int real_rate_16k = 16108;
	
	if((argc < 1) || (argc > 3) || (total_ts <= 0))
	{
		ak_print_error_ex("Input command error:\n");
    	ak_print_normal("%s\n",help[1]);
		return ;
	}
	if((SampleRate != 8000) && (SampleRate != 16000))
	{
		ak_print_error_ex("Input command error:\n");
    	ak_print_normal("%s\n",help[1]);
		return ;
	}
	//open  a file for read	
	fd = fopen(args[0], "r" );
	if (fd == NULL)
	{
		ak_print_error("open file failed!\n");
		return ;
	}
	else
	{
		ak_print_normal("open file success!\n");
	}
	
	struct ak_date systime;
	char * path = "a:/";
	char file_path3[255] = {0};
	memset(&systime, 0, sizeof(systime));
	ak_get_localdate(&systime);
	
	sprintf(file_path3, "%saec_%04d%02d%02d%02d%02d%02d.pcm", path,systime.year, systime.month , 
	systime.day , systime.hour, systime.minute, systime.second);
		
	aec  = fopen(file_path3, "w+");
		
	if(NULL == aec) 
	{
		ak_print_error_ex("create pcm file err\n");	
		goto unmount;
	}
	
	aec_break = false;
	cmd_signal(CMD_SIGTERM, cmd_aec_signal);//register  signal
	
	if(SampleRate == 8000)
	{
		data_size = (real_rate_8k * 1  * 2 ) * (total_ts/ 1000);
	}
	else
	{
		data_size = (real_rate_16k * 1  * 2 ) * (total_ts/ 1000);
	}
	fseek(fd, 0, SEEK_END);
	file_len = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	
	aecBuf = (unsigned char *)malloc(data_size+100*1024);	
	pcmBuf = (unsigned char *)malloc(data_size+1);
	
	if(NULL == aecBuf)
	{		
		ak_print_error_ex("malloc fail\n");	
		goto unmount;
	}
	if(NULL == pcmBuf)
	{		
		ak_print_error_ex("malloc fail\n");	
		goto freeaec;
	}
	
	if(data_size > file_len)
	{
		fread(pcmBuf, file_len, 1, fd);	
		memset(&pcmBuf[file_len], 0, (data_size - file_len));
	}
	else
	{
		fread(pcmBuf, data_size, 1, fd);		
	}
	ak_thread_sem_init(&gWaitAd,0);
	
	ret = ak_thread_create(&ao_id, ao_open,
			 NULL, ANYKA_THREAD_MIN_STACK_SIZE, 85);
	if (ret != 0)
		ak_print_error("create ao_open thread FAILED, ret=%d\n", ret);

	ak_sleep_ms(20);

	ret = ak_thread_create(&ai_id, ai_open,
			 NULL, ANYKA_THREAD_MIN_STACK_SIZE, 85);
	if (ret != 0)
		ak_print_error("create ai_open thread FAILED, ret=%d\n", ret);

	
	ak_thread_join (ai_id);	
	ak_thread_join (ao_id);
	ak_thread_sem_destroy(&(gWaitAd));	

	firstplay = true;	
	aec_break = false;
	
	
freepcm:
	if(pcmBuf != NULL)
	{
		free(pcmBuf);	
		pcmBuf = NULL;
	}
freeaec:	
	if(aecBuf != NULL)
	{
		free(aecBuf);	
		aecBuf = NULL;
	}
unmount:
	fclose(fd);	
	fclose(aec);
}


static int cmd_echo_reg(void)
{
    cmd_register("aecdemo", cmd_echo_test, help);
    return 0;
}

cmd_module_init(cmd_echo_reg)
