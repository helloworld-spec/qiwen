/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_dvr_record_demo.c
* Description: This is a simple example to show how the dvr_record module working.
* Notes:
* History: V1.0.0
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <getopt.h>

#include "ak_common.h"
#include "ak_vi.h"
#include "ak_ai.h"
#include "ak_ao.h"
#include "ak_sd_card.h"
#include "ak_dvr_file.h"
#include "ak_dvr_record.h"

#define LEN_HINT                    512
#define LEN_PATH                    1024
#define DEFAULT_RUN_SEC             300                       //default runtime of program(seconds)
#define DEFAULT_RECORD_FILE_SEC     300                       //default runtime of video file(seconds)
#define DEFAULT_TRIGGER_KEEP_SEC    2                         //default seconds before triggering recording
#define DEFAULT_TRIGGER_STOP_SEC    60                        //default seconds after triggering recording

#define DEFAULT_MAIN_WIDTH          1920                      //default main channel resolution
#define DEFAULT_MAIN_HEIGHT         1080
#define DEFAULT_SUB_WIDTH           640                       //default sub channel resolution
#define DEFAULT_SUB_HEIGHT          480
#define DEFAULT_FPS                 25                        //default video frame per second
#define DEFAULT_BPS                 3000                      //default data transmission rate
#define DEFAULT_GOP                 2                         //default group of picture seconds
#define DEFAULT_VIDEO_TYPE          H264_ENC_TYPE             //default encode type
#define DEFAULT_VIDEO_METHOD        0                         //0 default, 1 ctrl I size, 2 smart h.264
#define DEFAULT_MINQP               25                        //default encoding quality
#define DEFAULT_MAXQP               48

#define DEFAULT_AUDIO_RATE          8000                      //default audio sample rate
#define DEFAULT_AUDIO_INTERVAL      40                        //default audio frame interval
#define DEFAULT_AUDIO_BITS          16                        //default audio sampling accuracy
#define DEFAULT_AUDIO_VOL           8                         //default volume

#define SEC2MSEC                    1000

#define DEFAULT_SENSOR_PATH         "/usr/local"              //ISP config file path
#define DEFAULT_TMP_PATH            "/mnt/tmp"                //record file tmp path

char ac_option_hint[  ][ LEN_HINT ] = {                       //hint array of help for print
	"       HELP" ,
	"[SEC]  (DEFAULT: 300)" ,
	"[SEC]  (DEFAULT: 300)" ,
	"[NUM]  (DEFAULT: 1920-MAIN 640-SUB)" ,
	"[NUM]  (DEFAULT: 1080-MAIN 480-SUB)" ,
	"[0|1]  MAIN: 0 | SUB: 1 (DEFAULT: 0)" ,
	"[0|2]  H264_ENC_TYPE: 0 | HEVC_ENC_TYPE: 2(DEFAULT: 0)" ,
	"[NUM]  (DEFAULT: 3000)" ,
	"[NUM]  (DEFAULT: 25)" ,
	"[NUM]  (DEFAULT: 2)" ,
	"[NUM]  (DEFAULT: 25)" ,
	"[NUM]  (DEFAULT: 48)" ,
	"[NUM]  METHOD_DEFAULT: 0 | METHOD_ISIZE_CTRL: 1 | METHOD_SMART_H264: 2(DEFAULT: 0)" ,
	"[0|1]  AVI: 0 | MP4: 1 (DEFAULT: 1)" ,
	"[SEC]  (DEFAULT: 2)" ,
	"[SEC]  (DEFAULT: 60)" ,
	"[1|2]  PLAN: 1 | ALARM: 2 (DEFAULT: 1)" ,
	"[NUM]  (DEFAULT: 100)" ,
	"[NUM]  [NUM]  (MP3: 2 | AMR: 3 | AAC: 4 | PCM: 6 | ALAW: 17 | ULAW: 18 )" ,
	"[PATH] (DEFAULT: /usr/local/)" ,
	"[PATH] (DEFAULT: /mnt/CYC_DV/)" ,
	"[PATH] (DEFAULT: /mnt/tmp/)" ,
	"[PATH] (DEFAULT: CYC_DV_)" ,
};

struct option option_long[ ] = {
	{ "help"             , no_argument       , NULL , 'h' } , //"       HELP" ,
	{ "run-sec"          , required_argument , NULL , 'a' } , //"[SEC]  (DEFAULT: 300)" ,
	{ "record-file-sec"  , required_argument , NULL , 'b' } , //"[SEC]  (DEFAULT: 300)" ,
	{ "width"            , required_argument , NULL , 'c' } , //"[NUM]  (DEFAULT: 1920-MAIN 640-SUB)" ,
	{ "height "          , required_argument , NULL , 'd' } , //"[NUM]  (DEFAULT: 1080-MAIN 480-SUB)" ,
	{ "video-chn"        , required_argument , NULL , 'e' } , //"[0|1]  MAIN: 0 | SUB: 1 (DEFAULT: 0)" ,
	{ "video-type"       , required_argument , NULL , 'f' } , //"[0|2]  H264_ENC_TYPE: 0 | HEVC_ENC_TYPE: 2(DEFAULT: 0)" ,
	{ "kbps"             , required_argument , NULL , 'g' } , //"[NUM]  (DEFAULT: 3000)" ,
	{ "fps"              , required_argument , NULL , 'i' } , //"[NUM]  (DEFAULT: 25)" ,
	{ "gop"              , required_argument , NULL , 'j' } , //"[NUM]  (DEFAULT: 2)" ,
	{ "minqp"            , required_argument , NULL , 'k' } , //"[NUM]  (DEFAULT: 25)" ,
	{ "maxqp"            , required_argument , NULL , 'l' } , //"[NUM]  (DEFAULT: 48)" ,
	{ "method"           , required_argument , NULL , 'm' } , //"[NUM]  METHOD_DEFAULT: 0 | METHOD_ISIZE_CTRL: 1 | METHOD_SMART_H264: 2(DEFAULT: 0)" ,
	{ "file-type"        , required_argument , NULL , 'n' } , //"[0|1]  AVI: 0 | MP4: 1 (DEFAULT: 1)" ,
	{ "trigger-keep-sec" , required_argument , NULL , 'o' } , //"[SEC]  (DEFAULT: 2)" ,
	{ "trigger-stop-sec" , required_argument , NULL , 'p' } , //"[SEC]  (DEFAULT: 60)" ,
	{ "trigger-type"     , required_argument , NULL , 'q' } , //"[1|2]  PLAN: 1 | ALARM: 2 (DEFAULT: 1)" ,
	{ "audio-interval"   , required_argument , NULL , 'r' } , //"[NUM]  (DEFAULT: 100)" ,
	{ "audio-type"       , required_argument , NULL , 's' } , //"[NUM]  (MP3: 2 | AMR: 3 | AAC: 4 | PCM: 6 | ALAW: 17 | ULAW: 18 )" ,
	{ "sensor-path"      , required_argument , NULL , 't' } , //"[PATH] (DEFAULT: /usr/local/)" ,
	{ "record-path"      , required_argument , NULL , 'u' } , //"[PATH] (DEFAULT: /mnt/CYC_DV/)" ,
	{ "record-tmppath"   , required_argument , NULL , 'v' } , //"[PATH] (DEFAULT: /mnt/tmp/)" ,
	{ "record-prefix"    , required_argument , NULL , 'w' } , //"[PATH] (DEFAULT: CYC_DV_)" ,
};

int i_run_sec = DEFAULT_RUN_SEC;
int i_record_file_sec = DEFAULT_RECORD_FILE_SEC;
int i_main_width = DEFAULT_MAIN_WIDTH;
int i_main_height = DEFAULT_MAIN_HEIGHT;
int i_sub_width = DEFAULT_SUB_WIDTH;
int i_sub_height = DEFAULT_SUB_HEIGHT;
int i_width = 0;
int i_height = 0;
int i_video_chn = ENCODE_MAIN_CHN;
int i_file_type = DVR_FILE_TYPE_MP4;
int i_fps = DEFAULT_FPS;
int i_kbps = DEFAULT_BPS;
int i_gop = DEFAULT_GOP;
int i_trigger_keep_sec = DEFAULT_TRIGGER_KEEP_SEC;
int i_trigger_stop_sec = DEFAULT_TRIGGER_STOP_SEC;
int i_trigger_type = RECORD_TRIGGER_TYPE_PLAN;
int i_audio_interval = DEFAULT_AUDIO_INTERVAL;
enum ak_audio_type i_audio_type = AK_AUDIO_TYPE_UNKNOWN;
int i_minqp = DEFAULT_MINQP;
int i_maxqp = DEFAULT_MAXQP;
int i_method = DEFAULT_VIDEO_METHOD;
int i_video_type = DEFAULT_VIDEO_TYPE;
static void *pv_video_handle = NULL;
static void *pv_audio_handle = NULL;
char c_run = AK_TRUE;
char *pc_prog_name = NULL;
char *pc_sensor_path = DEFAULT_SENSOR_PATH;
char *pc_record_path = RECORD_DEFAULT_PATH;
char *pc_record_tmp_path = DEFAULT_TMP_PATH;
char *pc_record_prefix = RECORD_DEFAULT_PREFIX;

/*
 * help_hint: use the -h --help option.Print option of help information
 * return: 0
 */
static int help_hint(void)
{
	int i;

	printf("%s\n" , pc_prog_name);
	for(i = 0; i < sizeof(option_long) / sizeof(struct option); i ++) {
		printf("\t--%-16s -%c %s\n" , option_long[ i ].name , option_long[ i ].val , ac_option_hint[ i ]);
	}
	printf("\n\n");
	return 0;
}

/*
 * init_video: Initialze the video interface base on video parameters.
 *             And start the video data capture thread.
 * return : success - video handle address; fail - NULL;
 */
static void *init_video(void)
{
	int ret;
	void *pv_video = NULL;
	struct video_channel_attr attr;
	struct video_resolution resolution = {0};

	ret = ak_vi_match_sensor(pc_sensor_path);                 //match sensor at the first step
	if (ret == AK_FAILED) {
		ak_print_normal("match sensor failed\n");
		return NULL;
	}

	pv_video = ak_vi_open(VIDEO_DEV0);
	if (pv_video == NULL) {
		ak_print_normal("vi open failed\n");
		return NULL;
	}

	ret = ak_vi_get_sensor_resolution(pv_video, &resolution);
	if (ret == AK_FAILED) {                                   //get camera resolution
		ak_print_normal("get sensor resolution failed\n");
		goto init_video_fail;
	}

	memset(&attr, 0, sizeof(attr));                           //assign main channel and sub channel resolution data
	attr.res[VIDEO_CHN_MAIN].width = i_main_width;
	attr.res[VIDEO_CHN_MAIN].height = i_main_height;
	attr.res[VIDEO_CHN_SUB].width = i_sub_width;
	attr.res[VIDEO_CHN_SUB].height = i_sub_height;

	attr.res[VIDEO_CHN_MAIN].max_width = DEFAULT_MAIN_WIDTH;
	attr.res[VIDEO_CHN_MAIN].max_height = DEFAULT_MAIN_HEIGHT;
	attr.res[VIDEO_CHN_SUB].max_width = DEFAULT_SUB_WIDTH;
	attr.res[VIDEO_CHN_SUB].max_height = DEFAULT_SUB_HEIGHT;

	attr.crop.left = 0;
	attr.crop.top = 0;
	attr.crop.width = resolution.width;                       //resolution from sensor
	attr.crop.height = resolution.height;

	ret = ak_vi_set_channel_attr(pv_video, &attr);
	if (ret == AK_FAILED) {                                   //set main channel and sub channel resolution
		ak_print_normal("set channel attribute failed\n");
		goto init_video_fail;
	}

	ak_print_normal("capture fps: %d\n", ak_vi_get_fps(pv_video));
	ak_print_normal("\nvideo input info:\n"
					"\tmain_w[%d], main_h[%d], sub_w[%d], sub_h[%d]\n\n",
					i_main_width, i_main_height,
					i_sub_width, i_sub_height);
	ret = ak_vi_capture_on(pv_video);                         //start capture video thread.
	if(ret == AK_FAILED) {
		ak_print_normal("vi capture on failed\n");
		goto init_video_fail;
	}
	return pv_video;

init_video_fail:
	if (pv_video != NULL) {
		ak_vi_close(pv_video);
	}
	return NULL;
}

/**
 * set_frame_interval - set frame interval and set to ai
 * @ai_handle[IN]: ai handle
 * @enc_type[IN]: audio encode type
 * @sample_rate[IN]: sample rate
 * notes:
 */
static int set_frame_interval(void *ai_handle, int enc_type,
				unsigned int sample_rate)
{
	int interval = -1;

	switch (enc_type) {
	case AK_AUDIO_TYPE_AAC:
		interval = ((1024 *1000) / sample_rate); /* 1k data in 1 second */
		break;
	case AK_AUDIO_TYPE_AMR:
		interval = AMR_FRAME_INTERVAL;
		break;
	case AK_AUDIO_TYPE_PCM_ALAW:	/* G711, alaw */
	case AK_AUDIO_TYPE_PCM_ULAW:	/* G711, ulaw */
		interval = DEFAULT_AUDIO_INTERVAL;
		break;
	case AK_AUDIO_TYPE_PCM:
		interval = DEFAULT_AUDIO_INTERVAL;
		break;
	case AK_AUDIO_TYPE_MP3:
		if (sample_rate >= 8000 && sample_rate <= 24000) {
			interval = 576*1000/sample_rate;
		} else { // sample_rate =32000 or 44100 or 48000
			interval = 1152*1000/sample_rate;
		}
		break;
	default:
		interval = DEFAULT_AUDIO_INTERVAL;
		break;
	}

	/* ak_ai_set_frame_interval must use before ak_ai_get_frame */
	int ret = ak_ai_set_frame_interval(ai_handle, interval);
	i_audio_interval = interval;

	if (AK_SUCCESS == ret)
		ak_print_normal("frame interval=%d\n", interval);
	else
		ak_print_normal("set ai interval failed\n");

	return ret;
}

/*
 * init_audio: Initialze the audio interface base on audio parameters
 *              And start the audio data capture thread.
 * return : success - audio handle address; fail - NULL;
 */
void *init_audio(void)
{
	struct pcm_param ai_param = {0};
	void *pv_audio = NULL;
	int ret;

	//set correct param to open audio input
	ai_param.sample_bits = DEFAULT_AUDIO_BITS;
	ai_param.channel_num = AUDIO_CHANNEL_MONO;
	ai_param.sample_rate = DEFAULT_AUDIO_RATE;

	pv_audio = ak_ai_open(&ai_param);
	if(pv_audio == NULL){
		return NULL;
	}

	//set ai attributions after success open
	ak_ai_set_nr_agc(pv_audio, AUDIO_FUNC_ENABLE);
	ak_ai_set_resample(pv_audio, AUDIO_FUNC_ENABLE);
	ak_ai_set_volume(pv_audio, DEFAULT_AUDIO_VOL);
	ak_ai_clear_frame_buffer(pv_audio);

	//set mic input mode(AI_SOURCE_MIC) or line-in input mode(AI_SOURCE_LINEIN).Now set mic input mode
	ret = ak_ai_set_source(pv_audio, AI_SOURCE_MIC);
	if (ret == AK_FAILED) {
		goto init_audio_fail;
	}

	ret = set_frame_interval(pv_audio, i_audio_type, DEFAULT_AUDIO_RATE);

	if (ret == AK_FAILED) {
		goto init_audio_fail;
	}
	return pv_audio;

init_audio_fail:
	if (pv_audio != NULL) {
		ak_ai_close(pv_audio);
	}
	return NULL;
}

/*
 * init_record: Initialze the record structure base on video and audio parameters.
 * pv_video[IN]: video handle
 * pv_audio[IN]: audio handle
 * return : init success - AK_SUCCESS; init fail AK_FAILED;
 */
static int init_record(void *pv_video, void *pv_audio)
{
	struct dvr_file_param file_param = {0};
	struct record_param record = {0};

	file_param.cyc_flag = AK_TRUE;                            //set loop recording flag
	file_param.type = i_file_type;
	strcpy(file_param.rec_prefix, pc_record_prefix);          //set record file prefix
	strcpy(file_param.rec_path, pc_record_path);              //set record file save path
	strcpy(file_param.rec_tmppath, pc_record_tmp_path);       //set record file tmp path

	ak_dvr_file_init(&file_param);

	record.vi_handle = pv_video;                              //video record param init
	record.ai_handle = pv_audio;

	record.enc_chn = i_video_chn;                             //set use main channel or sub channel
	if (i_video_chn == ENCODE_MAIN_CHN) {
		record.width = i_main_width;
		record.height = i_main_height;
	} else {
		record.width = i_sub_width;
		record.height = i_sub_height;
	}

	record.file_fps = i_fps;
	record.file_bps = i_kbps;
	record.gop_len = i_gop;
	record.minqp = i_minqp;                                   //set video encode quality
	record.maxqp = i_maxqp;
	record.video_type = i_video_type;                         //encode type h.264 or h.265
	record.file_type = i_file_type;

	record.sample_rate = DEFAULT_AUDIO_RATE;                  //audio sample and frame interval
	record.frame_interval = i_audio_interval;
	record.audio_type = i_audio_type;

	record.duration = (i_record_file_sec * SEC2MSEC);
	record.error_report = NULL;                               //no set the error report callback function

	return ak_dvr_record_init(&record);
}

/*
 * prog_exit: When receive the SIGINT or SIGTERM signal then set run flag to AK_FALSE
 */
void prog_exit(int i_signal)
{
	c_run = AK_FALSE;
	return;
}

/**
 * Preconditions:
 * 1¡¢TF card is already mounted
 * 2¡¢sensor-path(default: /usr/local/) and record-path(/mnt/CYC_DV/) and record-tmp-path(/mnt/tmp/) must be exist
 * 3¡¢main video progress must stop
 */
int main (int argc, char **argv)
{
	int i_option;
	char ac_path[LEN_PATH];

	ak_print_normal("*****************************************\n");
	ak_print_normal("** ak dvr record demo: %s **\n", ak_dvr_record_get_version());
	ak_print_normal("*****************************************\n");

	pc_prog_name = argv[ 0 ];
	while((i_option = getopt_long(argc , argv , "ha:b:c:d:e:f:g:i:j:k:l:m:n:o:p:q:r:s:t:u:" , option_long , NULL)) != -1) {
		switch(i_option) {
			case 'h' :
				help_hint();
				return 0;
			case 'a' :
				i_run_sec = atoi(optarg);
				break;
			case 'b' :
				i_record_file_sec = atoi(optarg);
				break;
			case 'c' :
				i_width = atoi(optarg);
				break;
			case 'd' :
				i_height = atoi(optarg);
				break;
			case 'e' :
				i_video_chn = atoi(optarg);
				break;
			case 'f' :
				i_video_type = atoi(optarg);
				break;
			case 'g' :
				i_kbps = atoi(optarg);
				break;
			case 'i' :
				i_fps = atoi(optarg);
				break;
			case 'j' :
				i_gop = atoi(optarg);
				break;
			case 'k' :
				i_minqp = atoi(optarg);
				break;
			case 'l' :
				i_maxqp = atoi(optarg);
				break;
			case 'm' :
				i_method = atoi(optarg);
				break;
			case 'n' :
				i_file_type = atoi(optarg);
				break;
			case 'o' :
				i_trigger_keep_sec = atoi(optarg);
				break;
			case 'p' :
				i_trigger_stop_sec = atoi(optarg);
				break;
			case 'q' :
				i_trigger_type = atoi(optarg);
				break;
			case 'r' :
				i_audio_interval = atoi(optarg);
				break;
			case 's' :
				i_audio_type = atoi(optarg);
				break;
			case 't' :
				pc_sensor_path = optarg;
				break;
			case 'u' :
				pc_record_path = optarg;
				break;
			case 'v' :
				pc_record_tmp_path = optarg;
				break;
			case 'w' :
				pc_record_prefix = optarg;
				break;
		}
	}

	/*
	 * step 0: env initialize
	 */
	signal(SIGINT , prog_exit);                               //register exit signal
	signal(SIGTERM , prog_exit);

	ak_print_normal("dvr record demo start\n");
	snprintf(ac_path, LEN_PATH, "mkdir -p %s", pc_record_tmp_path);
	system(ac_path);
	snprintf(ac_path, LEN_PATH, "mkdir -p %s", pc_record_path);
	system(ac_path);

	if (i_width > 0){
		if (i_video_chn == ENCODE_MAIN_CHN) {
			i_main_width = i_width;
		} else {
			i_sub_width = i_width;
		}
	}
	if (i_height > 0){
		if (i_video_chn == ENCODE_MAIN_CHN) {
			i_main_height = i_height;

		} else {
			i_sub_height = i_height;
		}
	}

	/*
	 * step 1 init video handle
	 */
	if ((pv_video_handle = init_video()) == NULL) {
		ak_print_normal("init_video failed\n");
		goto FAILED;
	}

	/*
	 * step 2 init audio handle
	 */
	if((pv_audio_handle = init_audio()) == NULL) {
		ak_print_normal("init_audio failed\n");
	}

	/*
	 * step 3 check card status
	 */
	if(SD_STATUS_CARD_INSERT & ak_sd_check_insert_status()){  //check sd card insert or not
		ak_print_normal("SD Card Inserted\n");
		if(AK_SUCCESS == ak_sd_check_mount_status()){
			ak_print_normal("SD Card mounted\n");
		} else {
			ak_print_normal("SD Card not mounted\n");
			goto FAILED;
		}
	} else {
		ak_print_normal("SD Card not Inserted\n");
		goto FAILED;
	}

	/*
	 * step 4 init record param
	 */
	if (AK_FAILED == init_record(pv_video_handle, pv_audio_handle)){
		ak_print_normal("init_record failed.\n");
		goto FAILED;
	}
	if (i_trigger_type == RECORD_TRIGGER_TYPE_ALARM){         //keeping video and audio before alarm record trigger
		ak_dvr_record_turn_on_trigger(i_trigger_keep_sec);
		ak_sleep_ms(i_trigger_keep_sec * SEC2MSEC);
	}

	/*
	 * step 5 start to record video to file
	 */
	if (ak_dvr_record_start(i_trigger_type) == AK_FAILED) {
		ak_print_normal("init_record failed.\n");
		goto FAILED;
	}

	if(i_run_sec <= 0) {                                      //run until ctrl-c
		while(c_run == AK_TRUE) {
			pause();
		}
	} else {                                                  //run according to the setting time
		if (i_trigger_type == RECORD_TRIGGER_TYPE_ALARM){
			ak_sleep_ms(i_trigger_stop_sec * SEC2MSEC);
		} else {
			ak_sleep_ms(i_run_sec * SEC2MSEC);
		}
	}

	if (RECORD_TRIGGER_TYPE_ALARM == i_trigger_type){         //if use trigger record type then close trigger
		ak_dvr_record_turn_off_trigger();
	}

	/*
	 * step 6 stop to record video to file
	 */
	ak_dvr_record_stop();

	/*
	 * step 7 exit  record and release resource
	 */
	ak_dvr_record_exit();
	ak_print_normal("dvr record demo end\n");

FAILED:
	return 0;
}
