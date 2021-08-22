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

#include "monitor.h"

#include "ak_common.h"
#include "ak_cmd_exec.h"
#include "ak_thread.h"
#include "ak_drv_ir.h"
#include "ak_drv_irled.h"

#include "ak_vi.h"
#include "ak_ai.h"
#include "ak_ao.h"
#include "ak_venc.h"
#include "ak_aenc.h"
#include "ak_its.h"
#include "ak_ipc_srv.h"

#include "ak_rtsp.h"
#include "ak_net.h"
#include "ak_dana.h"
#include "ak_misc.h"
#include "ak_vpss.h"

#include "ak_dvr_file.h"
#include "record_ctrl.h"
#include "ak_config.h"

/* anyka_ipc platform version */
#define AK_VERSION_SOFTWARE             "V1.0.47"

/* ISP config file path */
#define FIRST_PATH       				"/etc/jffs2/"
#define BACK_PATH        				"/usr/local/"

#define IPC_AI_MAX_VOLUME				8

/* make sure vi, ai and ao open just once */
static void *vi_handle = NULL;
static void *ai_handle = NULL;
static void *ao_handle = NULL;

static unsigned char ipc_run_flag = 0;

const char* ak_ipc_get_version(void)
{
	return AK_VERSION_SOFTWARE;
}

static void exit_other_platform(void)
{
	struct sys_cloud_config *cloud = ak_config_get_sys_cloud();

#ifdef CONFIG_RTSP_SUPPORT
	if(cloud->rtsp) {
		ak_rtsp_stop(0);
		ak_rtsp_stop(1);
		ak_rtsp_exit();
		ak_print_normal_ex("######## rtsp exit ok ########\n\n");
	}
#endif

#ifdef CONFIG_DANA_SUPPORT
	if(cloud->dana){
		ak_dana_exit();
	}
#endif

	struct video_record_config *record = ak_config_get_record();
	if (record->server_flag) {
		record_ctrl_exit();
	}

	ak_dvr_file_exit();

	ak_misc_stop_photosensitive_switch_ex();

}

/**
 * exit_ipc_app - release_ipc_resource
 * @void
 * return: void
 * notes: call this function when you stop and exit ipc program
 */
static void exit_ipc_app(void)
{
	ak_print_normal("\n********** entering ipc_exit_app **********\n");
	/* set log level to info, to dump more message */
	int old_level = ak_print_set_level(LOG_LEVEL_INFO);

	ak_misc_sys_ipc_unregister();

	/* exit the voice play module */
	ak_misc_exit_voice_tips();

	exit_other_platform();

	ak_print_notice_ex("*** ready to close vi, ai and ao ***\n");
	ak_vpss_destroy(VIDEO_DEV0);
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
	ipc_run_flag = 0;

	ak_cmd_server_unregister(ANYKA_IPC_PORT);
	ak_config_release_ini();

	ak_print_normal("\n********** Anyka IPC program exit OK **********\n");
	fflush(NULL);

	ak_print_set_level(old_level);
}

static int init_vi(void)
{
	/* open device */
	vi_handle = ak_vi_open(VIDEO_DEV0);
	if (NULL == vi_handle) {
		ak_print_error_ex("vi open failed\n");
		return AK_FAILED;
	}

	ak_vpss_init(vi_handle, VIDEO_DEV0);

	/* get camera resolution */
	struct video_resolution resolution = {0};
	if (ak_vi_get_sensor_resolution(vi_handle, &resolution))
		ak_print_error_ex("get sensor resolution failed\n");

	/* set crop information */
	struct video_channel_attr attr;
	struct camera_disp_config *camera = ak_config_get_camera_info();

	if (camera->main_width > resolution.width) {
		ak_print_error_ex("ini file's width invalid, only can be %d\n",
			resolution.width);
		camera->main_width = resolution.width;
	}
	if (camera->main_height > resolution.height) {
		ak_print_error_ex("ini file's height invalid, only can be %d\n",
			resolution.height);
		camera->main_height = resolution.height;
	}

	if (0 == camera->main_max_width)
		camera->main_max_width = resolution.width;
	
	if (0 == camera->main_max_height)
		camera->main_max_height = resolution.height;
	
	if (0 == camera->sub_max_width)
		camera->sub_max_width = 640;
	
	if (0 == camera->sub_max_height)
		camera->sub_max_height = 480;
	
	if (camera->main_max_width > resolution.width) {
		ak_print_error_ex("ini file's max_width invalid, only can be %d\n",
			resolution.width);
		camera->main_max_width = resolution.width;
	}
	if (camera->main_max_height > resolution.height) {
		ak_print_error_ex("ini file's max_height invalid, only can be %d\n",
			resolution.height);
		camera->main_max_height = resolution.height;
	}
	
	memset(&attr, 0x00, sizeof(attr));
	attr.res[VIDEO_CHN_MAIN].width = camera->main_width;
	attr.res[VIDEO_CHN_MAIN].height = camera->main_height;
	attr.res[VIDEO_CHN_SUB].width = camera->sub_width;
	attr.res[VIDEO_CHN_SUB].height = camera->sub_height;
	attr.res[VIDEO_CHN_MAIN].max_width = camera->main_max_width;
	attr.res[VIDEO_CHN_MAIN].max_height = camera->main_max_height;
	attr.res[VIDEO_CHN_SUB].max_width = camera->sub_max_width;
	attr.res[VIDEO_CHN_SUB].max_height = camera->sub_max_height;
	attr.crop.left = 0;
	attr.crop.top = 0;
	attr.crop.width = resolution.width;
	attr.crop.height = resolution.height;

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
			camera->main_width, camera->main_height,
			camera->sub_width, camera->sub_height);

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
	char sensor_if[8] = {0};
	char tmpstr[8] = {0};
	int dvp_flag = 0;

	FILE *fp = fopen(SENSOE_IF_PATH, "r");
	if ((!fp) && (errno == ENOENT)) {
		ak_print_error_ex("cannot get sensor if, check your camera driver\n");
		closedir(dir);
		return ret;
	}

	fread(sensor_if, sizeof(sensor_if), 1, fp);
	fclose(fp);

	if (strstr(sensor_if, "mipi1"))
		strcpy(tmpstr, "mipi_1");
	else if (strstr(sensor_if, "mipi2"))
		strcpy(tmpstr, "mipi_2");
	else
		dvp_flag = 1;

	ak_print_normal_ex("sensor_if:%s\n", sensor_if);

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

		if ((!dvp_flag && !strstr(tmp, tmpstr))
			|| (dvp_flag && strstr(tmp, "mipi")))
			continue;

		isp_cfg = strstr(tmp, ".conf");
		if (!isp_cfg) {
			continue;
		}

		sprintf(isp_file, "%s%s", isp_cfg_path, dir_ent->d_name);
		/* get sensor id, match config file */
		if(AK_SUCCESS == ak_vi_match_sensor(isp_file)) {
			ak_print_notice_ex("ak_vi_match_sensor OK\n");
			ret = AK_SUCCESS;

			if (strcmp(isp_cfg_path, FIRST_PATH)) {
				char cmd[128] = {0};
				char result[2] = {0};

				sprintf(cmd, "cp %s %s", isp_file, FIRST_PATH);
				ak_cmd_exec(cmd, result, 2);

				sprintf(isp_file, "%s%s", FIRST_PATH, dir_ent->d_name);
				ak_vi_match_sensor(isp_file);
			}
			break;
		}
	}
	closedir(dir);

	return ret;
}

static int init_video(void)
{
	/* match sensor at the first step */
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
		if(ak_vi_capture_on(vi_handle)) {
			ak_print_error_ex("vi capture on failed\n");
		} else {
			ret = AK_SUCCESS;
		}
	}

	if ((AK_FAILED == ret) && (NULL != vi_handle)) {
		ak_vi_close(vi_handle);
		vi_handle = NULL;
		ak_vpss_destroy(VIDEO_DEV0);
	}

	return ret;
}

static int init_ai(void)
{
	struct pcm_param ai_param = {0};

	/* set correct param to open audio input */
	ai_param.sample_bits = 16;
	/* channel number only support AUDIO_CHANNEL_MONO */
	ai_param.channel_num = AUDIO_CHANNEL_MONO;

	struct video_record_config *record_config = ak_config_get_record();
	ai_param.sample_rate = record_config->sample_rate;

	ai_handle = ak_ai_open(&ai_param);
	if(NULL == ai_handle){
		return AK_FAILED;
	}

	/* set ai attributions after open OK */
	ak_ai_set_aec(ai_handle, AUDIO_FUNC_ENABLE);
	ak_ai_set_nr_agc(ai_handle, AUDIO_FUNC_ENABLE);
	ak_ai_set_resample(ai_handle, AUDIO_FUNC_DISABLE);
	ak_ai_set_volume(ai_handle, IPC_AI_MAX_VOLUME);
	if (AK_SUCCESS == ak_ai_set_frame_interval(ai_handle, AUDIO_DEFAULT_INTERVAL)) {
		ak_print_normal_ex("ai frame interval=%d\n", AUDIO_DEFAULT_INTERVAL);
	}

	struct audio_config *audio_config = ak_config_get_audio();
	ak_ai_set_source(ai_handle, audio_config->ai_source);
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

	ao_handle = ak_ao_open(&ao_param);
	if(NULL == ao_handle) {
		return AK_FAILED;
	}

	/* set ao attributions after open OK */
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

#ifdef CONFIG_RTSP_SUPPORT
static void init_rtsp(void)
{
	struct rtsp_param param = {{{0}}};
	struct camera_disp_config *camera = ak_config_get_camera_info();
	struct video_config *video = ak_config_get_sys_video();

	/* main channel config */
	param.rtsp_chn[0].current_channel = 0;
	param.rtsp_chn[0].width 	= camera->main_width;
	param.rtsp_chn[0].height 	= camera->main_height;

	param.rtsp_chn[0].fps 	 	= video->main_fps;
	param.rtsp_chn[0].max_kbps 	= video->main_max_kbps;
	param.rtsp_chn[0].min_qp	= video->main_min_qp;
	param.rtsp_chn[0].max_qp 	= video->main_max_qp;
	param.rtsp_chn[0].gop_len	= video->main_gop_len;

	param.rtsp_chn[0].video_enc_type = video->main_enc_type;
	param.rtsp_chn[0].video_br_mode  = video->main_video_mode;

	param.rtsp_chn[0].vi_handle = vi_handle;
	strcpy(param.rtsp_chn[0].suffix_name, "vs0");

	/* next param, VBR only */
	if (param.rtsp_chn[0].video_br_mode == BR_MODE_VBR) {
		/* smart model */
		if (video->main_smart_mode == 1 || video->main_smart_mode == 2) {
			param.rtsp_chn[0].smart.smart_mode 	= video->main_smart_mode;
			param.rtsp_chn[0].smart.smart_goplen = video->main_smart_goplen;
			param.rtsp_chn[0].smart.smart_static_value = video->main_smart_static_value;

			/* H.265 */
			if (param.rtsp_chn[0].video_enc_type == HEVC_ENC_TYPE) {
				param.rtsp_chn[0].smart.smart_quality = video->main_smart_quality_265;
				param.rtsp_chn[0].target_ratio = video->main_smart_target_ratio_265;
				ak_print_notice_ex("normal vbr 265, rario:%d\n", param.rtsp_chn[0].target_ratio);
			} else if (param.rtsp_chn[0].video_enc_type == H264_ENC_TYPE) {	/* H.264 */
				param.rtsp_chn[0].smart.smart_quality = video->main_smart_quality_264;
				param.rtsp_chn[0].target_ratio = video->main_smart_target_ratio_264;
				ak_print_notice_ex("normal vbr 264, rario:%d\n", param.rtsp_chn[0].target_ratio);
			}
		} else {	/* normal vbr model */
			if (param.rtsp_chn[0].video_enc_type == HEVC_ENC_TYPE) {
				param.rtsp_chn[0].target_ratio = video->main_target_ratio_265;
			} else if (param.rtsp_chn[0].video_enc_type == H264_ENC_TYPE) {
				param.rtsp_chn[0].target_ratio = video->main_target_ratio_264;
			}
		}
	}

	/* main channel config */
	param.rtsp_chn[1].current_channel = 1;
	param.rtsp_chn[1].width 	= camera->sub_width;
	param.rtsp_chn[1].height 	= camera->sub_height;

	param.rtsp_chn[1].fps 	 	= video->sub_fps;
	param.rtsp_chn[1].max_kbps 	= video->sub_max_kbps;
	param.rtsp_chn[1].min_qp	= video->sub_min_qp;
	param.rtsp_chn[1].max_qp 	= video->sub_max_qp;
	param.rtsp_chn[1].gop_len	= video->sub_gop_len;

	param.rtsp_chn[1].video_enc_type = video->sub_enc_type;
	param.rtsp_chn[1].video_br_mode  = video->sub_video_mode;

	param.rtsp_chn[1].vi_handle = vi_handle;
	strcpy(param.rtsp_chn[1].suffix_name, "vs1");

	/* next param, VBR only */
	if (param.rtsp_chn[1].video_br_mode == BR_MODE_VBR) {
		/* smart model */
		if (video->sub_smart_mode == 1 || video->sub_smart_mode == 2) {
			param.rtsp_chn[1].smart.smart_mode 	= video->sub_smart_mode;
			param.rtsp_chn[1].smart.smart_goplen = video->sub_smart_goplen;
			param.rtsp_chn[1].smart.smart_static_value = video->sub_smart_static_value;

			/* H.265 */
			if (param.rtsp_chn[1].video_enc_type == HEVC_ENC_TYPE) {
				param.rtsp_chn[1].smart.smart_quality = video->sub_smart_quality_265;
				param.rtsp_chn[1].target_ratio = video->sub_smart_target_ratio_265;
			} else if (param.rtsp_chn[1].video_enc_type == H264_ENC_TYPE) {	/* H.264 */
				param.rtsp_chn[1].smart.smart_quality = video->sub_smart_quality_264;
				param.rtsp_chn[1].target_ratio = video->sub_smart_target_ratio_264;
			}
		} else {	/* normal vbr model */
			if (param.rtsp_chn[1].video_enc_type == HEVC_ENC_TYPE) {
				param.rtsp_chn[1].target_ratio = video->sub_target_ratio_265;
			} else if (param.rtsp_chn[1].video_enc_type == H264_ENC_TYPE) {
				param.rtsp_chn[1].target_ratio = video->sub_target_ratio_264;
			}
		}
	}

	ak_print_notice_ex("rtsp version: %s\n", ak_rtsp_get_version());
	if (ak_rtsp_init(&param)){
		ak_print_error_ex("######## rtsp init failed ########\n");
	} else {
		ak_rtsp_start(0);
		ak_rtsp_start(1);
	    ak_print_normal("start rtsp OK\n");
	}
}
#endif

static void init_record_file(enum dvr_file_type file_type)
{
	/* video replay param init */
	struct video_record_config *record_config = ak_config_get_record();
	struct dvr_file_param file_param = {0};

	file_param.cyc_flag = record_config->save_cyc_flag;
	file_param.type = file_type;

	if (strlen(record_config->prefix)) {
		strcpy(file_param.rec_prefix, record_config->prefix);
	} else {
		strcpy(file_param.rec_prefix, RECORD_DEFAULT_PREFIX);
	}

	if (strlen(record_config->path)) {
		strcpy(file_param.rec_path, record_config->path);
	} else {
		strcpy(file_param.rec_path, RECORD_DEFAULT_PATH);
	}

	ak_dvr_file_init(&file_param);
}

static int init_flip_mirror(void)
{
	struct image_config *image = ak_config_get_image();

	return ak_vi_set_flip_mirror(vi_handle, image->flip, image->mirror);
}

static int init_irled(void)
{
	struct ak_drv_irled_hw_param param;
	struct image_config *image = ak_config_get_image();

	param.irled_working_level = image->irled_mode;
	return ak_drv_irled_init(&param);
}

static void init_other_platform(void)
{
	struct camera_disp_config *camera = ak_config_get_camera_info();
	struct sys_cloud_config *cloud = ak_config_get_sys_cloud();
	struct video_record_config *record = ak_config_get_record();

	/* init video flip & mirror */
	init_flip_mirror();
	/* init ir led work mode */
	init_irled();

	struct auto_day_night_config *auto_day_night = ak_config_get_auto_day_night();
	//struct auto_day_night_switch config;
	int i = 0;
	
	if (auto_day_night->auto_day_night_enable) {
		/* use auto day night switch */	
		struct ak_auto_day_night_threshold threshold;
		threshold.day_to_night_lum = auto_day_night->day_to_night_lum;
		threshold.night_to_day_lum = auto_day_night->night_to_day_lum;
		threshold.lock_time = auto_day_night->lock_time;
		
		for (i = 0; i < NIGHT_ARRAY_NUM; i++) {
			threshold.night_cnt[i] = auto_day_night->night_cnt[i];
		}

		for (i = 0; i < DAY_ARRAY_NUM; i++) {
			threshold.day_cnt[i] = auto_day_night->day_cnt[i];
		}

		ak_vpss_isp_set_auto_day_night_param(&threshold);
	
		ak_misc_start_photosensitive_switch_ex(AUTO_PHOTOSENSITIVE,
										camera->day_ctrl, auto_day_night->day_night_mode);

	} else {
		/* start photosensitive ircut detect service */
    	ak_misc_start_photosensitive_switch_ex(HARDWARE_PHOTOSENSITIVE,
										camera->day_ctrl, auto_day_night->day_night_mode);
	}

	/* after init record file, you can generate record file or/and replay. */
	//enum dvr_file_type file_type = DVR_FILE_TYPE_AVI;
	enum dvr_file_type file_type = record->file_type;
	init_record_file(file_type);

	/* decide what cloud we configured support */
#ifdef CONFIG_RTSP_SUPPORT
	if(cloud->rtsp) {
	    init_rtsp();
	}
#endif

	if (record->server_flag) {
		/* make sure we use only one cloud platform */
		record_ctrl_init(vi_handle, ai_handle, file_type);
	}

#ifdef CONFIG_DANA_SUPPORT
	if(cloud->dana){
		ak_dana_init(vi_handle, ai_handle, ao_handle);
	}
#endif

}

static int init_software(void)
{
	/* init ipc_srv */
	ak_cmd_server_register(ANYKA_IPC_PORT, "anyka_ipc7000");

	/** load config message, than init it **/
	ak_config_init_ini();

	/* just let we know what interface we use now */
	ak_net_get_cur_iface(NULL);

	int ret = AK_FAILED;
	if(AK_FAILED == init_video()) {
		goto software_end;
	}

	if(AK_FAILED == init_audio()) {
		goto software_end;
	}
	/* resiger misc sys-ipc msg */
	ak_misc_sys_ipc_register();

	ak_drv_ir_init();
	/* init voice play module */
	ak_misc_init_voice_tips(ao_handle);

	if (ak_misc_ipc_first_run()) {
		struct audio_param file_param = {AK_AUDIO_TYPE_MP3, 8000, 16, 1};
		ak_misc_add_voice_tips("/usr/share/anyka_camera_start.mp3", &file_param);
	}

	init_other_platform();

	ret = AK_SUCCESS;

software_end:
	if (AK_FAILED == ret) {
		ak_vpss_destroy(VIDEO_DEV0);
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
	if(ipc_run_flag) {
		char res[2] = {0};
		ak_cmd_exec("rm -f /tmp/core_*", res, 2);

 		ak_backtrace(sig, si, ptr);
 	}

	if((SIGSEGV == sig) || (SIGTERM == sig) || (SIGINT == sig)){
		ipc_run_flag = 0;
	}
}

static int register_signal(void)
{
	struct sigaction s;

	s.sa_flags = SA_SIGINFO;
	s.sa_sigaction = (void *)process_signal;

	/* register signals that we should handle */
//	sigaction(SIGSEGV, &s, NULL);
	sigaction(SIGINT, &s, NULL);
	sigaction(SIGTERM, &s, NULL);
	sigaction(SIGUSR1, &s, NULL);
	sigaction(SIGUSR2, &s, NULL);
	sigaction(SIGALRM, &s, NULL);
	sigaction(SIGHUP, &s, NULL);
//	sigaction(SIGPIPE, &s, NULL);

	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	return 0;
}

int main(void)
{
	ak_print_normal("\n*******************************************************\n");
	ak_print_normal("\t %s_build@%s_%s\n",
			AK_VERSION_SOFTWARE, __DATE__, __TIME__);
	ak_print_normal("*******************************************************\n\n");

	/* print audio version and functions that we support */
	ak_ai_print_filter_info();
	ak_aenc_print_codec_info();

	ipc_run_flag = 1;
	register_signal();
	start_monitor_th();

	if (init_software()) {
		return -1;
	}

	while (ipc_run_flag) {
		ak_sleep_ms(1000);
	}
	exit_ipc_app();

	return 0;
}
