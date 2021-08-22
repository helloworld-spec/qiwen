/******************************************************
 * @brief  adec demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-23 
*******************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "ak_common.h"
#include "command.h"
#include "ak_global.h"
#include "ak_ao.h"
#include "ak_adec.h"
#include "ak_login.h"
#include "libc_posix_fs.h"

/******************************************************
  *                    Macro         
  ******************************************************/
#define RECORD_READ_LEN		 4096

#define SAMPLE_BITS 16
#define SAMPLE_RATE 8000
/******************************************************
  *                    Type Definitions         
  ******************************************************/


/******************************************************
  *                    Constant         
  ******************************************************/
  
/******************************************************
  *                    Global Variables         
 ******************************************************/
static char *help[]={
	"adec demo module",
	"usage:adecdemo [filename] [filetype] <samplerate> <channelnum>\n"
	"filename : must is stream data file\n"
	"filetype: aac/pcm/amr/adpcmi/adpcmm/adpcmf/g711a/g711u/mp3\n"
	"samplerete : 8000~44100\n"
	"channelnum : mono/stereo\n"
};

/******************************************************
*               Function Declarations
******************************************************/
void cmd_adec_test(int argc, char **args);

/******************************************************
*               Function prototype                           
******************************************************/
/*****************************************
 * @brief open appointed file
 * @param argc[in] record_file : file path and name
 * @return FILE* : file point after open
 *****************************************/
static FILE* open_record_file(const char *record_file)
{

	FILE *fp = fopen(record_file, "r");										
	if(NULL == fp) {
		ak_print_error("open record file err\n");
		return NULL;
	}

	ak_print_normal("open record file: %s OK\n", record_file);
	return fp;
}

/*****************************************
 * @brief read file data,and send to encode
 * @param argc[in] decode : decode handle
 * @param args[in]  fp: appointed file point
 * @param args[in] type :file decode type
 * @return void
 *****************************************/
static void read_record_file(void *decode, FILE *fp, unsigned int type)
{
	int read_len = 0;
	int send_size = 0;
	unsigned char data[RECORD_READ_LEN] = {0};

	char *pdata = NULL;
	/* amr file,have to skip the head 6 byte */
	if (AK_AUDIO_TYPE_AMR == type) {			
		fseek(fp, 6, SEEK_SET);
	}
	
	while(AK_TRUE)
	{
		/* read the record file stream */
		memset(data, 0x00, sizeof(data));
		send_size = 0;
		read_len = fread(data, sizeof(char), sizeof(data), fp);
		pdata = data;
		if(read_len > 0) 
		{
			while(read_len > 0)
			{
				//send data to encode input pool
				send_size = ak_adec_send_stream(decode, pdata , read_len, 0);
				if(send_size < 0)
				{
					ak_print_error_ex("write pcm to DA error!\n");
					break;
				}
				read_len = read_len - send_size;
				pdata += send_size;
			}
		} 
		else if(0 == read_len)
		{
			ak_print_normal("read to the end of file\n");
			break;
		}
		else
		{
			ak_print_error("read, %s\n", strerror(errno));
			break;
		}
	}
	//read file over, avoid encode input pool data remaining.change input buffer mode
	ak_adec_notice_stream_end(decode);
	ak_print_normal("%s exit\n", __func__);
}

/**
 * argv[0]: filename
 * argv[1]: fileType
 * argv[2]: sample_rate
 * argv[3]: channel_num
 * note: read the appointed file, decode, and then output to DA
 */
void cmd_adec_test(int argc, char **args)
{
	ak_print_normal("%s\n", args[0]);
	int ret = AK_FAILED;

	/* 1. open audio output */
	struct pcm_param param = {0};
	param.sample_bits = SAMPLE_BITS;
	param.channel_num = AUDIO_CHANNEL_STEREO;
	param.sample_rate = SAMPLE_RATE;

	/* 2. open audio encode */
	struct audio_param adec_param = {0};
	adec_param.sample_bits = SAMPLE_BITS;
	adec_param.channel_num = AUDIO_CHANNEL_MONO;
	adec_param.sample_rate = SAMPLE_RATE;
	
   /*  ------------ arguments checking ------------  */	
	if(argc > 4 || argc < 2)
	{
		ak_print_error("input argument number error!\n");
		ak_print_error("%s",help[1]);
		return ;
	}
	
	if(argc == 4)
	{
		if(strcmp("mono",args[3]) == 0)
		{
			adec_param.channel_num = 1;
		}
		else if(strcmp("stereo", args[3]) ==0 )
		{
			adec_param.channel_num = 2;
		}
		else
		{
			ak_print_error("input argument channel_num error!\n");
			ak_print_error("%s",help[1]);
			return ;
		}
	}
	if(argc >= 3)
	{
		param.sample_rate = atoi(args[2]);
		adec_param.sample_rate = param.sample_rate;
		if((param.sample_rate < 8000) || (param.sample_rate > 44100))
		{
			ak_print_error("input argument sample rate error!\n");
			ak_print_error("%s",help[1]);
			return ;
		}
	}
	if(argc >= 2)
	{
		if(strcmp("aac",args[1])==0)
		{
			adec_param.type = AK_AUDIO_TYPE_AAC;  
		}
		else if(strcmp("g711a",args[1])==0)
		{
			adec_param.type = AK_AUDIO_TYPE_PCM_ALAW; 
		}
		else if(strcmp("g711u",args[1])==0)
		{
			adec_param.type = AK_AUDIO_TYPE_PCM_ULAW; 
		}
		else if(strcmp("amr",args[1])==0)
		{
			adec_param.type = AK_AUDIO_TYPE_AMR; 
		}
		else if(strcmp("mp3",args[1])==0)
		{
			adec_param.type = AK_AUDIO_TYPE_MP3; 
		}
		else if(strcmp("adpcmi",args[1])==0)
		{
			adec_param.type = AK_AUDIO_TYPE_ADPCM_IMA; 
		}
		else if(strcmp("adpcmm",args[1])==0)
		{
			adec_param.type = AK_AUDIO_TYPE_ADPCM_MS; 
		}
		else if(strcmp("adpcmf",args[1])==0)
		{
			adec_param.type = AK_AUDIO_TYPE_ADPCM_FLASH; 
		}
		else if(strcmp("pcm",args[1])==0)
		{
			adec_param.type = AK_AUDIO_TYPE_PCM; 
		}
		else
		{
			ak_print_error("input argument type error!\n");
			ak_print_error("%s",help[1]);
			return ;
		}
	}
	/*  ------------ mount SD card ------------  */	
#if 0	
	if (0 ==ak_mount_fs(DEV_MMCBLOCK, 0, ""))
		ak_print_normal("adec mount sd ok!\n");
	else
		ak_print_error("adec mount sd fail!\n");
#endif
	/*open music file*/
	FILE *fp = open_record_file(args[0]);
	if(NULL == fp) {
		ak_print_error("open file failed!\n");
		goto mount_end;
	}
	
	/*open DA device*/
    void *ao_handle = ak_ao_open(&param);
    if(NULL == ao_handle) {
    	goto fp_end;
    }
	/*opne decode device */
    void *adec_handle = ak_adec_open(&adec_param);
	if(NULL == adec_handle) {
    	goto ao_end;
    }
	/*printf version */
	ak_print_normal("%s\n", ak_adec_get_version());

	//set volume
	ak_ao_set_volume(ao_handle, 5);
	
    /*  bind audio output & decode */
	void *stream_handle = ak_adec_request_stream(ao_handle, adec_handle);
	if(NULL == stream_handle) {
    	goto adec_end;
    }
	/* open speaker*/
	ak_ao_enable_speaker(ao_handle,1);
	/* 4. read audio file stream and write it DA */
   	read_record_file(adec_handle, fp, adec_param.type);	//single frame

	
	if(NULL != stream_handle) {
		/*decode end, cancel stream */
    	ak_adec_cancel_stream(stream_handle);
    }
	
	/* close speaker*/
	ak_ao_enable_speaker(ao_handle,0);
adec_end: 
	if(NULL != adec_handle) {
    	ak_adec_close(adec_handle);
    }
ao_end:    
    if(NULL != ao_handle) {
    	ak_ao_close(ao_handle);
    }
fp_end:    
    if(NULL != fp) {
    	fclose(fp);
    	fp = NULL;
    }
mount_end:
	/*  ------------ unmount SD card ------------  */	
#if 0	
	if (0 ==ak_unmount_fs(DEV_MMCBLOCK, 0, ""))
		ak_print_normal("adec unmount sd ok!\n");
	else
		ak_print_error("adec unmount sd fail!\n");
#endif	
    ak_print_normal("----- audio_play demo exit -----\n");

}

/*****************************************
 * @brief register adecdemo command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_adec_reg(void)
{
    cmd_register("adecdemo", cmd_adec_test, help);
    return 0;
}

/**
 *  module init
 */
cmd_module_init(cmd_adec_reg)

