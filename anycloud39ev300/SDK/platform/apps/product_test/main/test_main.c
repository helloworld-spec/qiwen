#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <syslog.h>
#include <ctype.h>
#include <math.h>

#include "test_net.h"
#include "test_common.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_vi.h"
#include "ak_ai.h"
#include "ak_ao.h"
#include "ak_its.h"
#include "test_config.h"
#include "test_voice_tip.h"
#include "ak_rtsp.h"
#include "ak_net.h"

/* test_product platform version */
#define AK_TEST_VERSION             	"V1.0.02"

/* ISP config file path */
#define FIRST_PATH       				"/etc/jffs2/"
#define BACK_PATH        				"/usr/local/"

#define TEST_AI_MAX_VOLUME				8

/* make sure vi, ai and ao open just once */
static void *vi_handle = NULL;
static void *ai_handle = NULL;
static void *ao_handle = NULL;

static unsigned char test_run_flag = 0;

/**
 * exit_test_app - release_TEST_resource
 * @void
 * return: void
 * notes: call this function when you stop and exit TEST program
 */
static void exit_test_app(void)
{
	ak_print_normal("\n********** entering test_exit_app **********\n");
	ak_rtsp_stop(0);
	ak_rtsp_stop(1);
	ak_rtsp_exit();
	ak_print_normal_ex("######## rtsp exit ok ########\n");

	ak_print_notice_ex("*** ready to close vi, ai and ao ***\n");
	if (vi_handle) {
		ak_vi_close(vi_handle);
		vi_handle = NULL;
	}
	if (ai_handle) {
		ak_ai_close(ai_handle);
		ai_handle = NULL;
	}
	if (ao_handle) {
		ak_ao_close(ao_handle);
		ao_handle = NULL;
	}
	test_run_flag = 0;

	/* exit the voice play module */
	test_exit_voice_tips();

	ak_print_normal("\n********** Anyka TEST program exit OK **********\n");
}

/**
 * init_vi - set video resolution
 * @void
 * return: 0, success; -1, failed
 */
static int init_vi(void)
{
	/* open video device */
	vi_handle = ak_vi_open(VIDEO_DEV0);
	if (NULL == vi_handle) {
		ak_print_error_ex("vi open failed\n");
		return AK_FAILED;
	}

	/* get camera resolution */
	struct video_resolution resolution = {0};
	if (ak_vi_get_sensor_resolution(vi_handle, &resolution))
		ak_print_error_ex("get sensor resolution failed\n");

	/* set crop information */
	struct video_channel_attr attr;
	struct config_handle *config = test_config_get_value();

	if (config->main_width > resolution.width) {
		ak_print_error_ex("ini file's width invalid, only can be %d\n",
				resolution.width);
		config->main_width = resolution.width;
	}
	if (config->main_height > resolution.height) {
		ak_print_error_ex("ini file's height invalid, only can be %d\n",
				resolution.height);
		config->main_height = resolution.height;
	}

	if (0 == config->main_max_width)
		config->main_max_width = config->main_width;
	
	if (0 == config->main_max_height)
		config->main_max_height = config->main_height;
	
	if (0 == config->sub_max_width)
		config->sub_max_width = config->sub_width;
	
	if (0 == config->sub_max_height)
		config->sub_max_height = config->sub_height;

	if (config->main_max_width > resolution.width) {
		ak_print_error_ex("ini file's max_width invalid, only can be %d\n",
				resolution.width);
		config->main_max_width = resolution.width;
	}
	if (config->main_max_height > resolution.height) {
		ak_print_error_ex("ini file's max_height invalid, only can be %d\n",
				resolution.height);
		config->main_max_height = resolution.height;
	}

	memset(&attr, 0x00, sizeof(attr));
	attr.res[VIDEO_CHN_MAIN].width = config->main_width;
	attr.res[VIDEO_CHN_MAIN].height = config->main_height;
	attr.res[VIDEO_CHN_SUB].width = config->sub_width;
	attr.res[VIDEO_CHN_SUB].height = config->sub_height;
	attr.res[VIDEO_CHN_MAIN].max_width = config->main_max_width;
	attr.res[VIDEO_CHN_MAIN].max_height = config->main_max_height;
	attr.res[VIDEO_CHN_SUB].max_width = config->sub_max_width;
	attr.res[VIDEO_CHN_SUB].max_height = config->sub_max_height;
	attr.crop.left = 0;
	attr.crop.top = 0;
	attr.crop.width = config->main_width;
	attr.crop.height = config->main_height;

	if (ak_vi_set_channel_attr(vi_handle, &attr)) {
		ak_print_error_ex("set channel attribute failed\n");
	}

	/* get crop */
	struct video_channel_attr cur_attr;

	memset(&cur_attr, 0x00, sizeof(cur_attr));
	if (ak_vi_get_channel_attr(vi_handle, &cur_attr)) {
		ak_print_normal("ak_vi_get_channel_attr failed!\n");
	}

	ak_print_normal_ex("capture fps: %d\n", ak_vi_get_fps(vi_handle));
	ak_print_normal("\nvideo input info:\n"
			"\tmain_w[%d], main_h[%d], sub_w[%d], sub_h[%d]\n\n",
			config->main_width, config->main_height,
			config->sub_width, config->sub_height);

	return AK_SUCCESS;
}

/* match sensor according to ISP config file path */
static int match_sensor(const char *isp_cfg_path)
{
	DIR *dir = opendir(isp_cfg_path);
	if (NULL == dir) {
		ak_print_normal_ex("it fails to open directory %s\n", isp_cfg_path);
		return 0;
	}

	int ret = AK_FAILED;
	char isp_file[255] = {0};
	char *tmp = NULL;
	char *isp_cfg = NULL;
	struct dirent *dir_ent = NULL;

	while (NULL != (dir_ent = readdir(dir))) {
		if (!dir_ent->d_name)
			continue;

		/* fine next when we get dir */
        if ((dir_ent->d_type & DT_DIR)) {
            continue;
        }

		/* make sure use isp_*.conf file to match */
		tmp = strstr(dir_ent->d_name, "isp_");
		if (!tmp) {
			continue;
		}

		isp_cfg = strstr(tmp, ".conf");
		if (!isp_cfg) {
			continue;
		}

		sprintf(isp_file, "%s%s", isp_cfg_path, dir_ent->d_name);
		/* get sensor id, match config file */
		if(AK_SUCCESS == ak_vi_match_sensor(isp_file)){
			ak_print_notice_ex("ak_vi_match_sensor OK\n");
			ret = AK_SUCCESS;

			if (strcmp(isp_cfg_path, FIRST_PATH))
			{
				char cmd[128] = {0};

				sprintf(cmd, "cp %s %s", isp_file, FIRST_PATH);
				system(cmd);

				sprintf(isp_file, "%s%s", FIRST_PATH, dir_ent->d_name);
				ak_vi_match_sensor(isp_file);
			}
			break;
		}
	}
	closedir(dir);

	return ret;
}

/**
 * init_video - init video
 * @void
 * return: 0, success; -1, failed
 */
static int init_video(void)
{
	if (AK_FAILED == match_sensor(FIRST_PATH)) {
		ak_print_warning_ex("match_sensor FIRST_PATH failed\n");
		if (AK_FAILED == match_sensor(BACK_PATH)) {
			ak_print_error_ex("match_sensor BACK_PATH failed\n");
			return AK_FAILED;
		}
	}

	int ret = AK_FAILED;

	/* one video input device, only open one time vi for encode */
	if (init_vi()) {
		ak_print_error_ex("video input init faild, exit\n");
	} else {
		ak_print_notice_ex("start capture ...\n");
		/* open isp capture */
		if(ak_vi_capture_on(vi_handle)) {
			ak_print_error_ex("vi capture on failed\n");
		} else {
			ret = AK_SUCCESS;
		}
	}

	if ((AK_FAILED == ret) && (NULL != vi_handle)) {
		ak_vi_close(vi_handle);
		vi_handle = NULL;
	}

	return ret;
}

static int init_ai(void)
{
	struct pcm_param ai_param = {0};
	struct config_handle *config = test_config_get_value();

	/* set correct param to open audio input */
	ai_param.sample_bits = 16;
	/* channel number only support AUDIO_CHANNEL_MONO */
	ai_param.channel_num = AUDIO_CHANNEL_MONO;
	ai_param.sample_rate = config->sample_rate;

	/* open audio in device and get handle */
	ai_handle = ak_ai_open(&ai_param);
	if(NULL == ai_handle){
		return AK_FAILED;
	}

	/* set ai attributions after open */
	ak_ai_set_aec(ai_handle, AUDIO_FUNC_DISABLE);
	ak_ai_set_nr_agc(ai_handle, AUDIO_FUNC_ENABLE);
	ak_ai_set_resample(ai_handle, AUDIO_FUNC_DISABLE);
	ak_ai_set_volume(ai_handle, TEST_AI_MAX_VOLUME);
	ak_ai_clear_frame_buffer(ai_handle);

	return AK_SUCCESS;
}

static int init_ao(void)
{
	struct pcm_param ao_param = {0};
	ao_param.sample_bits = 16;
	/* driver always set to AUDIO_CHANNEL_STEREO, so you don't care */
	ao_param.channel_num = AUDIO_CHANNEL_MONO;
	ao_param.sample_rate = 8000;

	/* open audio out device and get handle */
    ao_handle = ak_ao_open(&ao_param);
    if(NULL == ao_handle) {
    	return AK_FAILED;
    }

	/* set ao attributions after open */
	ak_ao_enable_speaker(ao_handle, AUDIO_FUNC_ENABLE);
	ak_ao_set_volume(ao_handle, 6);
	ak_ao_set_resample(ao_handle, AUDIO_FUNC_DISABLE);
	ak_ao_clear_frame_buffer(ao_handle);

	return AK_SUCCESS;
}

static int init_audio(void)
{
	if (AK_FAILED == init_ai()) {
		ak_print_error_ex("init ai failed\n");
		return AK_FAILED;
	}
	ak_print_notice_ex("init audio in OK\n");

	int ret = init_ao();
	if (AK_FAILED == ret) {
		ak_print_error_ex("init ao failed\n");
	}
	ak_print_notice_ex("init audio out OK\n");

	return ret;
}

static void init_rtsp(void)
{
    struct rtsp_param param = {{{0}}};

	struct config_handle *config = test_config_get_value();

    
	/* main channel config */
	param.rtsp_chn[0].current_channel = 0;
	param.rtsp_chn[0].width 	= config->main_width;
	param.rtsp_chn[0].height	= config->main_height;

	param.rtsp_chn[0].fps		= config->main_fps;
	param.rtsp_chn[0].max_kbps	= config->main_max_kbps;
	param.rtsp_chn[0].min_qp	= config->main_min_qp;
	param.rtsp_chn[0].max_qp	= config->main_max_qp;
	param.rtsp_chn[0].gop_len	= config->main_gop_len;

	param.rtsp_chn[0].video_enc_type = config->main_enc_type;
	param.rtsp_chn[0].video_br_mode  = config->main_video_mode;

	param.rtsp_chn[0].vi_handle = vi_handle;
	strcpy(param.rtsp_chn[0].suffix_name, "vs0");

	/* next param, VBR only */
	if (param.rtsp_chn[0].video_br_mode == BR_MODE_VBR) {
		/* smart model */
		if (config->main_smart_mode == 1 || config->main_smart_mode == 2) {
			param.rtsp_chn[0].smart.smart_mode	= config->main_smart_mode;
			param.rtsp_chn[0].smart.smart_goplen = config->main_smart_goplen;
			param.rtsp_chn[0].smart.smart_static_value = config->main_smart_static_value;

			/* H.265 */
			if (param.rtsp_chn[0].video_enc_type == HEVC_ENC_TYPE) {
				param.rtsp_chn[0].smart.smart_quality = config->main_smart_quality_265;
				param.rtsp_chn[0].target_ratio = config->main_smart_target_ratio_265;
				ak_print_notice_ex("normal vbr 265, rario:%d\n", param.rtsp_chn[0].target_ratio);
			} else if (param.rtsp_chn[0].video_enc_type == H264_ENC_TYPE) { /* H.264 */
				param.rtsp_chn[0].smart.smart_quality = config->main_smart_quality_264;
				param.rtsp_chn[0].target_ratio = config->main_smart_target_ratio_264;
				ak_print_notice_ex("normal vbr 264, rario:%d\n", param.rtsp_chn[0].target_ratio);
			}
		} else {	/* normal vbr model */
			if (param.rtsp_chn[0].video_enc_type == HEVC_ENC_TYPE) {
				param.rtsp_chn[0].target_ratio = config->main_target_ratio_265;
			} else if (param.rtsp_chn[0].video_enc_type == H264_ENC_TYPE) {
				param.rtsp_chn[0].target_ratio = config->main_target_ratio_264;
			}
		}
	}

	/* main channel config */
	param.rtsp_chn[1].current_channel = 1;
	param.rtsp_chn[1].width 	= config->sub_width;
	param.rtsp_chn[1].height	= config->sub_height;

	param.rtsp_chn[1].fps		= config->sub_fps;
	param.rtsp_chn[1].max_kbps	= config->sub_max_kbps;
	param.rtsp_chn[1].min_qp	= config->sub_min_qp;
	param.rtsp_chn[1].max_qp	= config->sub_max_qp;
	param.rtsp_chn[1].gop_len	= config->sub_gop_len;

	param.rtsp_chn[1].video_enc_type = config->sub_enc_type;
	param.rtsp_chn[1].video_br_mode  = config->sub_video_mode;

	param.rtsp_chn[1].vi_handle = vi_handle;
	strcpy(param.rtsp_chn[1].suffix_name, "vs1");

	/* next param, VBR only */
	if (param.rtsp_chn[1].video_br_mode == BR_MODE_VBR) {
		/* smart model */
		if (config->sub_smart_mode == 1 || config->sub_smart_mode == 2) {
			param.rtsp_chn[1].smart.smart_mode	= config->sub_smart_mode;
			param.rtsp_chn[1].smart.smart_goplen = config->sub_smart_goplen;
			param.rtsp_chn[1].smart.smart_static_value = config->sub_smart_static_value;

			/* H.265 */
			if (param.rtsp_chn[1].video_enc_type == HEVC_ENC_TYPE) {
				param.rtsp_chn[1].smart.smart_quality = config->sub_smart_quality_265;
				param.rtsp_chn[1].target_ratio = config->sub_smart_target_ratio_265;
			} else if (param.rtsp_chn[1].video_enc_type == H264_ENC_TYPE) { /* H.264 */
				param.rtsp_chn[1].smart.smart_quality = config->sub_smart_quality_264;
				param.rtsp_chn[1].target_ratio = config->sub_smart_target_ratio_264;
			}
		} else {	/* normal vbr model */
			if (param.rtsp_chn[1].video_enc_type == HEVC_ENC_TYPE) {
				param.rtsp_chn[1].target_ratio = config->sub_target_ratio_265;
			} else if (param.rtsp_chn[1].video_enc_type == H264_ENC_TYPE) {
				param.rtsp_chn[1].target_ratio = config->sub_target_ratio_264;
			}
		}
	}


    if (ak_rtsp_init(&param)){
		ak_print_error_ex("######## rtsp init failed ########\n");
	} else {
		ak_rtsp_start(0);
		ak_rtsp_start(1);
        ak_print_normal("start rtsp OK\n");
	}
}

static int init_software(void)
{
	/* just let we know what interface we use now */
	ak_net_get_cur_iface(NULL);

	int ret = AK_FAILED;
	if(AK_FAILED == init_video()) {
		goto software_end;
	}

	if(AK_FAILED == init_audio()) {
		goto software_end;
	}

	ak_print_normal("going to start rtsp \n");
	init_rtsp();

	/* init voice play module */
	test_init_voice_tips(ao_handle);

	ret = AK_SUCCESS;

software_end:
	if (AK_FAILED == ret) {
		if (vi_handle) {
			ak_vi_close(vi_handle);
			vi_handle = NULL;
		}
		if (ai_handle) {
			ak_ai_close(ai_handle);
			ai_handle = NULL;
		}
		if (ao_handle) {
			ak_ao_close(ao_handle);
			ao_handle = NULL;
		}
	}

	return ret;
}

static void process_signal(unsigned int sig, siginfo_t *si, void *ptr)
{
	if(test_run_flag) {
 		ak_backtrace(sig, si, ptr);
 	}

	if((SIGSEGV == sig) || (SIGTERM == sig) || (SIGINT == sig)){
		test_run_flag = 0;
	}
}

/* register signal that we should handle */
static int register_signal(void)
{
	struct sigaction s;

	s.sa_flags = SA_SIGINFO;
	s.sa_sigaction = (void *)process_signal;

	sigaction(SIGSEGV, &s, NULL);
	sigaction(SIGINT, &s, NULL);
	sigaction(SIGTERM, &s, NULL);
	sigaction(SIGUSR1, &s, NULL);
	sigaction(SIGUSR2, &s, NULL);
	sigaction(SIGALRM, &s, NULL);
	sigaction(SIGHUP, &s, NULL);
	sigaction(SIGPIPE, &s, NULL);

	signal(SIGCHLD, SIG_IGN);

	return 0;
}

int main(int argc, char **argv)
{
	ak_print_normal("\n*******************************************************\n");
	ak_print_normal("\t %s_build@%s_%s\n",
			AK_TEST_VERSION, __DATE__, __TIME__);
	ak_print_normal("*******************************************************\n\n");

	test_run_flag = 1;
	register_signal();

	/** load config message, then init it **/
	test_init_ini();

	if(AK_FAILED == init_software()){
		return -1;
	}
	/* if play this ok, speak out is ok */
	struct audio_param file_param = {AK_AUDIO_TYPE_MP3, 8000, 16, 1};
	test_add_voice_tips("/usr/share/anyka_camera_start.mp3", &file_param);

	start_test(vi_handle, ai_handle, ao_handle);
	test_audio_start(ai_handle);

	while (test_run_flag) {
		ak_sleep_ms(1000);
	}

	exit_test_app();
	test_exit_ini();

	return 0;
}
