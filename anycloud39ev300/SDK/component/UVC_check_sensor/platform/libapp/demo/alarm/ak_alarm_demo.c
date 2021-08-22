/******************************************************
 * @brief  alarm demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-17 
*******************************************************/
#include "command.h"
#include "string.h"
#include "ak_common.h"
#include "ak_ai.h"
#include "ak_vi.h"
#include "ak_drv_ircut.h"
#include "ak_alarm.h"
#include "kernel.h"

/******************************************************
  *                    Constant         
  ******************************************************/ 
static char *help[]={
	"alarm demo",
	"usage:alarmdemo <detect_level> <report_interval> <detect_interval>\n"
	"      detect_level : 3 >= level >=1 default 3, match sensitivity is [20 50 100]\n"
	"      report_interval : report interval time > 0, default 60(unit: second )\n"
	"      detect_interval : detect interval time > 0, default 500(unit: ms)\n"

};

static short audio_warnning_buf[]=
{
	#include "4204.txt"
};

/******************************************************
  *                    Macro         
  ******************************************************/


/******************************************************
  *                    Type Definitions         
  ******************************************************/
enum alarm_type {    
    MOVE_ALARM = 0,              //move alarm detection
    VOICE_ALARM,    		//voice alarm detection
    OTHER_ALARM,             //other alarm detection            
};


/******************************************************
  *                    Global Variables         
  ******************************************************/
static void *vi_handle = NULL;
static void *ai_handle = NULL;
int ratios[3] = {20, 50, 100};

int level = 0;
int report_interval = 0; 
int detect_interval = 0;

/******************************************************
*               Function prototype                           
******************************************************/


/******************************************************
*               Function Declarations
******************************************************/
/*****************************************
 * @brief video input init
 * @param [void]  
 * @return on success return AK_SUCCESS, fail return AK_FAILED
 *****************************************/
static int init_video(void)
{

    if (ak_vi_match_sensor("ISPCFG") < 0)
    {
        ak_print_error_ex("match sensor failed\n");
        return AK_FAILED;
    }

	/* open device */
	vi_handle = ak_vi_open(VIDEO_DEV0);
	if (NULL == vi_handle) {
		ak_print_error_ex("vi open failed\n");
		return AK_FAILED;
	}
	ak_print_normal("vi open ok\n");

	/* get camera resolution */
	struct video_resolution resolution = {0};
	if (ak_vi_get_sensor_resolution(vi_handle, &resolution))
	{
		ak_print_error_ex("get sensor resolution failed\n");
        ak_vi_close(vi_handle);
        return AK_FAILED;
	}	
	else
		ak_print_normal("sensor resolution height:%d,width:%d.\n",
				resolution.height,resolution.width);
  
	/* set crop information */
	struct video_channel_attr attr;

    /*set default crop */
	attr.crop.left = 0;
	attr.crop.top = 0;
	attr.crop.width = resolution.width;
	attr.crop.height = resolution.height;

    /* set channel default pixel */
	attr.res[VIDEO_CHN_MAIN].width = 1280;
	attr.res[VIDEO_CHN_MAIN].height = 720;
	attr.res[VIDEO_CHN_SUB].width = 640;
	attr.res[VIDEO_CHN_SUB].height= 360;

	if (ak_vi_set_channel_attr(vi_handle, &attr))
		ak_print_error_ex("set channel attribute failed\n");

	/* get one frame like  ak_vi_capture_on in linux */
	struct video_input_frame frame;
	int ret = ak_vi_get_frame(vi_handle, &frame);
	if (!ret)
		/* the frame has used,release the frame data */
		ak_vi_release_frame(vi_handle, &frame);
	
	return AK_SUCCESS;
}

/*****************************************
 * @brief alam callbak function
 * @param type[in] system detech type
 * @param level[in] system detech level
 * @param start_time[in] start time
 * @param time_len[in] the time len
 * @return 
 *****************************************/
void alarm_callback(int type,int level,int start_time,
						int time_len)
{
	
	ak_print_notice("\nalarm!alarm!alarm!alarm! \n");
	
	struct pcm_param info;

	unsigned int fram_size = 0;
	int ret;
	int *fd = NULL;
	unsigned char file_over = 0;
	int send_size, size;
	
	unsigned char* curPtr = (unsigned char*)audio_warnning_buf;
	unsigned char* endPtr = (unsigned char*)audio_warnning_buf 
										+ sizeof(audio_warnning_buf);

	unsigned char *startPtr = curPtr;
	unsigned char *tmpBuf;
	
	info.sample_bits = 16;
	info.sample_rate = 24000;
	info.channel_num = 1;
	
	/* open ao */
	fd = ak_ao_open(&info);

	if(fd == NULL)
	{
		ak_print_error("ao open false\n");
		return ;
	}
	fram_size = ak_ao_get_frame_size(fd);
	ak_print_normal("ao frame size =%d\n", fram_size);
	
	tmpBuf = malloc(fram_size);
	if(NULL == tmpBuf)
	{
		ak_print_error("ao demo malloc false!\n");
		return ;
	}
	/* set volume */
	ak_ao_set_volume(fd, 0x05);

	/* enable speaker*/
	ak_ao_enable_speaker(fd, 1);

	int times = 20;
	/* play audio for warning */
start_sound:	
	while (1)
	{
		if(endPtr - curPtr>fram_size)
		{
			memcpy(tmpBuf, curPtr, fram_size);
			ret = fram_size;
			curPtr += fram_size;
		}
		else
		{
			ret = endPtr - curPtr;
			memcpy(tmpBuf, curPtr, ret);
			file_over = 1;
		}		
		send_size = 0;
		while(ret > 0)
		{
			size = ak_ao_send_frame(fd, tmpBuf + send_size, ret, 0);
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
		{
			break;
		}
	}

	if(times)
	{
		//ak_sleep_ms(5);
		curPtr = startPtr;
		times --;
		goto start_sound;
	}
	free(tmpBuf);

	/* close ao */
	ak_ao_close(fd);

	/* disable speaker*/
	ak_ao_enable_speaker(fd, 0);

}

/*****************************************
 * @brief callback function which check alarm info to send or to discard
 * @param type[void] 
 * @return 0
 *****************************************/
static int  preview_status(void)
{
	return 0;
}

/*****************************************
 * @brief init params
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
static int init_params(int argc, char **args)
{
	int i = 0;
	if (argc > 3)
	{
		ak_print_error_ex("i err2:%d\n", argc);
		return -1;
	}
	level = 3;
	report_interval = 60; 
	detect_interval = 500;
	for (i = 0; i < argc; i++) {
		switch (i) {
			case 0:
				level= atoi(args[i]);
				if (level <1 || level > 3) 
				{
					return -1;
				}
				break;
			case 1:
				report_interval= atoi(args[i]);
				if (report_interval <1 ) 
				{
					return -1;
				}
				break;
			case 2:
				detect_interval= atoi(args[i]);
				if (detect_interval <1 ) 
				{
					return -1;
				}
				break;

			default:
				ak_print_error_ex("i err:%d\n", i);
		}
	}
	return 0;
}


/*****************************************
 * @brief start function for command
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
void cmd_alarm_demo(int argc, char **args)
{


	/** step1: init params*/
	if(init_params(argc,args))
	{
		ak_print_error("%s",help[1]);
		return ;
	}
	/* init video */
	if(AK_FAILED == init_video()) {
		return ;
	}
	

	ak_alarm_interval_time_set(report_interval,detect_interval);

	ak_alarm_ratios_set(MOVE_ALARM,0,sizeof(ratios)/sizeof(ratios[0]),ratios);
	/** step1: alarm init*/
	ak_alarm_init(MOVE_ALARM,vi_handle, alarm_callback, preview_status);
	
	/** step2: start sys motion_detection **/
    ak_alarm_start(MOVE_ALARM, level);

	/** step3: main thread sleep */
	ak_sleep_ms((report_interval + 10)*1000);
	ak_print_error_ex("sleep over\n");

	/** step4: alarm stop */
	ak_alarm_stop(MOVE_ALARM);

	/* close video in model */
	if (vi_handle) {
		ak_vi_close(vi_handle);
		vi_handle = NULL;
	}
}

/*****************************************
 * @brief register alarmdemo command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_alarm_reg(void)
{
    cmd_register("alarmdemo", cmd_alarm_demo, help);
    return 0;
}

cmd_module_init(cmd_alarm_reg)

