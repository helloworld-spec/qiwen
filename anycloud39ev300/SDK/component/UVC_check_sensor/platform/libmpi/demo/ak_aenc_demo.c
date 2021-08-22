#include <stdio.h>
#include <string.h>
#include <signal.h>
#include "command.h"
#include "ak_common.h"
#include "ak_ai.h"
#include "ak_aenc.h"
#include "kernel.h"


#define enable 1
#define AENC_TCARD_FREE_SIZE        1024  // T  Card  the lease  free  size (1M) 


FILE *fp;

static char *help[]={
	"aenc demo module",
	"usage:aencdemo <time> <encode>\n"
	"       time: input the time you need \n"
	"       encode: g711a; g711u; pcm; aac; amr; mp3\n"
	"       example:aencdemo 30 g711u \n"
	"       based on set encoded type\n"
	"       please insert tf card ,aencdemo will save encoded data to a:/**\n"
	"       encoded data format:8k samplerate, 1 chanel, 16bit.\n"
};

/**
 * aenc_get_disk_free_size - get disk free size, appoint the dir path
 * @path[IN]: disk dir full path
 * return: disk free size in KB, -1 failed
 */

unsigned long aenc_get_disk_free_size(const char *path)
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

static void write_record_file(void *stream_handle, int time)
{
	
	struct list_head stream_head ;
	struct aenc_entry *entry = NULL; 
	struct aenc_entry *ptr = NULL;
	unsigned long first_ts = 0, frame_ts = 0;
	int total_ts = time;
	bool first_get_flag = false;
	INIT_LIST_HEAD(&stream_head);

	while(1)
	{	
		/* get audio stream list */		
		if(0 == ak_aenc_get_stream(stream_handle,&stream_head))
		{
			list_for_each_entry_safe(entry, ptr, &stream_head, list)
			{
				if(NULL != entry) 
				{
					ak_print_normal("stream.len=%ld, stream.ts=%ld\n", 
						entry->stream.len, entry->stream.ts);
					if(fwrite(entry->stream.data, entry->stream.len, 1, fp) <= 0) 
					{
						ak_print_error_ex("write file err\n");
						return ;
					}
					else
					{
						frame_ts = entry->stream.ts;
					}
					ak_aenc_release_stream(entry);						
				}
			}
			if(!first_get_flag)
			{
				if(frame_ts > 0)
				{
					first_ts = frame_ts;
					first_get_flag = true;
				}
			}
			
			if((frame_ts - first_ts) >= total_ts)
			{
				break;
			}
		}
        else
    	{
			ak_sleep_ms(10);
		    continue;
    	}
	}
	ak_print_normal("thread exit\n\n");
}
static int get_encode_type(const char * type)
{
	int ret = -1;
	
	if(strcmp(type,"pcm") == 0) 
	{
		ret = AK_AUDIO_TYPE_PCM;
	}
	else if(strcmp(type,"aac") == 0)
	{
		ret = AK_AUDIO_TYPE_AAC;
	}
	else if(strcmp(type,"g711a") == 0)
	{
		ret = AK_AUDIO_TYPE_PCM_ALAW;
	}
	else if(strcmp(type,"g711u") == 0)
	{
		ret = AK_AUDIO_TYPE_PCM_ULAW;
	}
	else if(strcmp(type,"amr") == 0)
	{
		ret = AK_AUDIO_TYPE_AMR;
	}
	else if(strcmp(type,"mp3") == 0)
	{
		ret = AK_AUDIO_TYPE_MP3;
	}	 
			  
	return ret;		
			 
}
static void set_frame_interval(void *handle, int encode_type, unsigned int sample_rate)

{
	if(NULL == handle)
	{
	    return ;
	}
	unsigned int interval = 0;	
	switch(encode_type){
	case AK_AUDIO_TYPE_AAC:
		interval = ((1024 *1000) / sample_rate); // 1k data in 1 second 
		break;
	case AK_AUDIO_TYPE_AMR:
		interval = AMR_FRAME_INTERVAL;
		break;
		
	case AK_AUDIO_TYPE_PCM_ALAW:	//G711, alaw
	case AK_AUDIO_TYPE_PCM_ULAW:	//G711, ulaw:
		interval = AUDIO_DEFAULT_INTERVAL;
		break;
	
	default:	//default is AMR
		interval = AMR_FRAME_INTERVAL;
		break;
	}
	ak_ai_set_frame_interval(handle, interval);
}

void cmd_aenc_test(int argc, char **args)
{
	char * type = args[1];
	int total_ts = 0;
	int encode_type = -1;
	total_ts = atoi(args[0]) * 1000;
	
	if(argc == 0)
	{
		total_ts = 60 * 1000;
		encode_type = AK_AUDIO_TYPE_AMR;
	}

	else if(argc == 1)
	{
		if(total_ts > 0)
		{
			encode_type = AK_AUDIO_TYPE_AMR;
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
		encode_type = get_encode_type(type);
		if(total_ts < 0)
		{
			ak_print_error_ex("Input command error:total_ts must large than 0\n");    	
	    	ak_print_normal("%s\n",help[1]);
			return ;
		}
		else if(encode_type == -1)
		{
			ak_print_error_ex("Input command error:encoded type wrong\n");	
	    	ak_print_normal("%s\n",help[1]);
			return ;
		}
	}

	
	ak_print_normal("----- audio_record demo -----\n");
	ak_print_normal("version: %s\n\n", ak_ai_get_version());
	ak_print_normal("version: %s\n\n", ak_aenc_get_version());
	const unsigned char amrheader[]= "#!AMR\n";//amr file head
	
	struct ak_date systime;
	struct frame frame = {0};
	int ret = -1;

	unsigned long first_ts = 0;
	int first_time = 0;
	char file_path[255] = {0};
	unsigned long free_size = 0; 
	void *ai_handle = NULL;
	void *aenc_handle = NULL;
	
	char * path = "a:/";
	free_size = aenc_get_disk_free_size(path);//get T_card free size
		if(free_size < AENC_TCARD_FREE_SIZE)
	{		
		ak_print_error_ex("free_size < AENC_TCARD_FREE_SIZEKB\n",AENC_TCARD_FREE_SIZE);
		return ;
	}

	//audio input imfo
	struct pcm_param ai_param = {0};
	ai_param.sample_bits = 16;
	ai_param.channel_num = AUDIO_CHANNEL_MONO;
	ai_param.sample_rate = 8000;

	/* 1. open audio input */
    ai_handle = ak_ai_open(&ai_param);
	if(NULL == ai_handle) 
	{
		return;
	}
	ak_ai_set_source(ai_handle, AI_SOURCE_MIC);
	set_frame_interval(ai_handle, encode_type,ai_param.sample_rate);
	ak_ai_set_volume(ai_handle,7);
	ak_ai_clear_frame_buffer(ai_handle);
	ak_ai_set_resample(ai_handle,  enable);
	ak_ai_set_nr_agc(ai_handle, enable);
	//audio encode imfo
	struct audio_param aenc_param = {0};
	aenc_param.sample_bits = 16;
	aenc_param.channel_num = AUDIO_CHANNEL_MONO;
	aenc_param.sample_rate = 8000;
	
	memset(&systime, 0, sizeof(systime));

	switch(encode_type)
	{
		case AK_AUDIO_TYPE_PCM:
			ak_get_localdate(&systime);
			sprintf(file_path, "%sAUDIO_%04d%02d%02d_%02d%02d%02d.pcm", path,systime.year, 
			systime.month, systime.day, systime.hour, systime.minute, systime.second);
			fp = fopen(file_path, "w+");
			if(NULL == fp) 
			{
				ak_print_error_ex("create pcm file err\n");
				goto ai_end;
			}
			
			aenc_param.type = encode_type;
			break;

		case AK_AUDIO_TYPE_PCM_ALAW:
			ak_get_localdate(&systime);
			sprintf(file_path, "%sAUDIO_%04d%02d%02d_%02d%02d%02d_alaw.g711", path,systime.year, 
		    systime.month , systime.day , systime.hour, systime.minute, systime.second);
			fp = fopen(file_path, "w+");
			if(NULL == fp) 
			{
				ak_print_error_ex("create G711 file err\n");
				goto ai_end;
			}
			aenc_param.type = encode_type;
			break;

		case AK_AUDIO_TYPE_AAC:
			ak_get_localdate(&systime);
			sprintf(file_path, "%sAUDIO_%04d%02d%02d_%02d%02d%02d.aac", path,systime.year, 
			systime.month , systime.day , systime.hour, systime.minute, systime.second);
			fp = fopen(file_path, "w+");
			if(NULL == fp) 
			{
				ak_print_error_ex("create aac file err\n");
				goto ai_end;
			}
			aenc_param.type = encode_type;
			break;

		case AK_AUDIO_TYPE_PCM_ULAW:
			ak_get_localdate(&systime);
			sprintf(file_path, "%sAUDIO_%04d%02d%02d_%02d%02d%02d_ulaw.g711", path,systime.year, 
	        systime.month , systime.day , systime.hour, systime.minute, systime.second);
			fp = fopen(file_path, "w+");
			if(NULL == fp) 
			{
				ak_print_error_ex("create G711 file err\n");
				goto ai_end;
			}
			aenc_param.type = encode_type;
			break;

		case AK_AUDIO_TYPE_MP3:
			ak_get_localdate(&systime);
			sprintf(file_path, "%sAUDIO_%04d%02d%02d_%02d%02d%02d.mp3", path,systime.year, 
			systime.month , systime.day , systime.hour, systime.minute, systime.second);
			fp = fopen(file_path, "w+");
			if(NULL == fp) 
			{
				ak_print_error_ex("create mp3 file err\n");
				goto ai_end;
			}
			
			aenc_param.type = encode_type;
			break;

		default:
			ak_get_localdate(&systime);
			sprintf(file_path, "%sAUDIO_%04d%02d%02d_%02d%02d%02d.amr", path,systime.year, 
	        systime.month , systime.day , systime.hour, systime.minute, systime.second);
			fp = fopen(file_path, "w+");
			if(NULL == fp) 
			{
				ak_print_error_ex("create amr file err\n");
				goto ai_end;
			}
			/*if use amr ,need to write the head*/
			fwrite(amrheader, sizeof(amrheader), 1, fp);
			aenc_param.type = AK_AUDIO_TYPE_AMR;	
			break;

	}
	/* 2. open audio encode */
    aenc_handle = ak_aenc_open(&aenc_param);
	
	if(NULL == aenc_handle) 
	{
    	goto ai_end;
    }
			
	/* 3. bind audio input & encode */
	void *stream_handle = ak_aenc_request_stream(ai_handle, aenc_handle);
	if(NULL == stream_handle) 
	{
    	goto aenc_end;
    }

    /* 4. get audio stream and write it to file */
    write_record_file(stream_handle,total_ts);

    ret = 0;
	if(NULL != stream_handle) 
	{
    	ak_aenc_cancel_stream(stream_handle);
    }

aenc_end:
	if(NULL != aenc_handle) 
	{
    	ak_aenc_close(aenc_handle);
    }

ai_end:
    if(NULL != ai_handle) 
	{		
		ak_ai_release_frame(ai_handle, &frame);
    	ak_ai_close(ai_handle);
    }

fp_end:	
	 if(NULL != fp) 
	 {
	    fclose(fp);
	    fp = NULL;
	 }	
   ak_print_normal("----- audio_enc demo exit -----\n");
}

static int cmd_aenc_reg(void)
{
    cmd_register("aencdemo", cmd_aenc_test, help);
    return 0;
}

cmd_module_init(cmd_aenc_reg)


