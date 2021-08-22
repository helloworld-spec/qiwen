
/**
 * JUAN onvif?\n
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include <NkUtils/log.h>
#include <NkUtils/macro.h>

#include "ja_media.h"
#include "n1_def.h"

#include "onvif.h"
#include "env_common.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_net.h"
#include "ak_venc.h"

#include "ak_common.h"
#include "ak_onvif_config.h"
#include "ja_osd.h"
#include "ja_net.h"
#include "ak_cmd_exec.h"

#define JA_PROP_IPV4_STR(__Prop, __text, __size) \
	snprintf(__text, (__size), "%d.%d.%d.%d",\
		(NK_Int)(__Prop[0]),\
		(NK_Int)(__Prop[1]),\
		(NK_Int)(__Prop[2]),\
		(NK_Int)(__Prop[3]))

#define VIDEO_WIDTH_1080P (1920)
#define VIDEO_HEIGHT_1080P (1080)

#define VIDEO_WIDTH_VGA  (640)
#define VIDEO_HEIGHT_VGA (360)

static void GMT_SET(int gmt)
{
	if(gmt >= -1200 && gmt <= 1300){
		int const hour = abs(gmt) / 100;
		int const min = abs(gmt) % 100;

		if(min >= 0 && min < 60){
			char text[32] = {""};
			struct timezone tz;
			tz.tz_minuteswest = -1 * ((gmt >= 0 ? 1 : -1) * (hour * 60 + min));
			tz.tz_dsttime = 0;
			snprintf(text, sizeof(text), "CST%c%02d:%02d", gmt <= 0 ? '+' : '-',  hour, min); // opposite to GMT
			setenv("TZ", text, 1);
			tzset();
			FILE *fp1;
			char cmd[64];
			snprintf(cmd, sizeof(cmd), "echo %s > /tmp/TZ", text);
			fp1 = popen(cmd, "r");
			if(fp1){
				pclose(fp1);
			}
			int n = tz.tz_dsttime;
			printf("%d \n",n);
		}
	}
}

/**
 * _get_system_information - get system info
 * @info[OUT]: system info
 * return: 0 success, -1 fail
 */
static int _get_system_information(lpNVP_DEV_INFO info)
{
	strcpy(info->manufacturer, "JIUAN");
	strcpy(info->devname,"IPCAM");
	strcpy(info->model, "IPCAM");
	strcpy(info->sn, "JUAN1234567890");
	strcpy(info->firmware, "1.1.1.1");
	strcpy(info->sw_version, "1.0.0.0");
	strcpy(info->sw_builddate, "2015-11-18");
	strcpy(info->hw_version, "1.1.0.0");
	strcpy(info->hwid, "HW000");
	return 0;
}

/**
 * _get_date_time - get date time
 * @systime[OUT]: systime params
 * return: 0 success, -1 fail
 */
static int _get_date_time(lpNVP_SYS_TIME systime)
{
	time_t t_now;
	struct tm *ptm;
	struct tm tm_local, tm_gm;
	struct onvif_sys_config *pSys_info = onvif_config_get_sys();

	systime->ntp_enable = AK_FALSE;
	strcpy(systime->ntp_server, "");
	systime->tzone = pSys_info->tzone;

	time(&t_now);
	localtime_r(&t_now, &tm_local);
	gmtime_r(&t_now, &tm_gm);

	ptm = &tm_local;
	systime->local_time.date.year = ptm->tm_year;
	systime->local_time.date.month = ptm->tm_mon;
	systime->local_time.date.day = ptm->tm_mday;
	systime->local_time.time.hour = ptm->tm_hour;
	systime->local_time.time.minute = ptm->tm_min;
	systime->local_time.time.second = ptm->tm_sec;
	ptm = &tm_gm;
	systime->gm_time.date.year = ptm->tm_year;
	systime->gm_time.date.month = ptm->tm_mon;
	systime->gm_time.date.day = ptm->tm_mday;
	systime->gm_time.time.hour = ptm->tm_hour;
	systime->gm_time.time.minute = ptm->tm_min;
	systime->gm_time.time.second = ptm->tm_sec;
	return 0;
}

 // an hour offset formated as +/- offset hour x 100 + offset minutes


 /**
  * _set_date_time - set date time
  * @systime[IN]: systime params
  * return: 0 success, -1 fail
  */
 static int _set_date_time(lpNVP_SYS_TIME systime)
 {
	time_t t_set;
	struct timeval tv_set;
	struct tm tm_set;
	
	tm_set.tm_year = systime->gm_time.date.year;
	tm_set.tm_mon = systime->gm_time.date.month;
	tm_set.tm_mday = systime->gm_time.date.day;
	tm_set.tm_hour = systime->gm_time.time.hour;
	tm_set.tm_min = systime->gm_time.time.minute;
	tm_set.tm_sec = systime->gm_time.time.second;
	
	GMT_SET(0);
	ak_print_normal_ex(" timezone %d \n",systime->tzone);
	t_set = mktime(&tm_set);
	
	tv_set.tv_sec = t_set;
	tv_set.tv_usec = 0;
	
	settimeofday(&tv_set, NULL);
	GMT_SET(systime->tzone);

	struct onvif_sys_config *pSys_info = onvif_config_get_sys();

	pSys_info->tzone = systime->tzone;
	onvif_config_set_sys(pSys_info);
	onvif_config_flush_data();
	return 0; 
 }

/*
static int _set_date_time(lpNVP_SYS_TIME systime)
{
	int tz_hour = systime->tzone / 100;
	int tz_min = (systime->tzone - tz_hour * 100);
	char cmd[256] = {0};	//cmd buffer

	ak_print_normal_ex("Test: Set Time %04d:%02d:%02d %02d:%02d:%02d GMT\n",
						systime->gm_time.date.year + 1900, systime->gm_time.date.month + 1, 
						systime->gm_time.date.day, systime->gm_time.time.hour, 
						systime->gm_time.time.minute, systime->gm_time.time.second);
	
	sprintf(cmd, "date \"%d-%d-%d %d:%d:%d\"",
			systime->gm_time.date.year+1900,
		   	systime->gm_time.date.month+1,
		   	systime->gm_time.date.day,
			systime->gm_time.time.hour + tz_hour,
			systime->gm_time.time.minute + tz_min,
		   	systime->gm_time.time.second);

	if(ak_cmd_exec(cmd, NULL, 0) < 0)			//system cmd,execute "system date \XX-XX-XX XX:XX:XX\"
	{
		return -1;
	}

	struct onvif_sys_config *pSys_info = onvif_config_get_sys();

	pSys_info->tzone = systime->tzone;
	onvif_config_set_sys(pSys_info);
	onvif_config_flush_data();
	
	bzero(cmd, sizeof(cmd));  //clean buffer

    //save to rtc
	sprintf(cmd, "hwclock -w");
	if(ak_cmd_exec(cmd, NULL, 0) < 0)
	{
		return -1;
	}

	return 0;
}
*/

/**
 * _get_dhcp_status - get dhcp status
 * return: 0 success, -1 fail
 */
static unsigned int _get_dhcp_status(void)
{
	return ja_net_get_dhcp_status();
}

/**
 * _get_interface - get network params
 * @ether[OUT]: ether params
 * return: 0 success, -1 fail
 */
static int _get_interface(lpNVP_ETHER_CONFIG ether)
{
	char mac[64]={0};

	ak_net_get_mac("eth0", mac, sizeof(mac));

	struct onvif_net_config *net = onvif_config_get_net();

	ether->dhcp = _get_dhcp_status();

	NVP_IP_INIT_FROM_STRING(ether->ip, net->ipaddr);
	NVP_IP_INIT_FROM_STRING(ether->netmask, net->netmask);
	NVP_IP_INIT_FROM_STRING(ether->gateway, net->gateway);
    NVP_MAC_INIT_FROM_STRING(ether->mac, mac);
	NVP_IP_INIT_FROM_STRING(ether->dns1, net->firstdns);
	NVP_IP_INIT_FROM_STRING(ether->dns2, net->backdns);
	ether->http_port = 80;//port_n.value;
	ether->rtsp_port = 554;//port_n.value;

	return 0;
}

/**
 * _set_interface - set network params
 * @ether[IN]: ether params
 * return: 0 success, -1 fail
 */
static int _set_interface(lpNVP_ETHER_CONFIG ether)
{
	char ipaddr[32] = {0};
	char netmask[32] = {0};
	char gateway[32] = {0};

	JA_PROP_IPV4_STR(ether->ip, ipaddr, sizeof(ipaddr));
	JA_PROP_IPV4_STR(ether->netmask, netmask, sizeof(netmask));
	JA_PROP_IPV4_STR(ether->gateway, gateway, sizeof(gateway));

	return ja_net_set_wired_net_parm(ipaddr, netmask, gateway);
}

/**
 * _get_color - get color params
 * @color[OUT]: color params
 * @id[IN]: 0 main chn, 1 sub chn
 * return: 0 success, -1 fail
 */
static int _get_color(lpNVP_COLOR_CONFIG color, int id)
{
	struct onvif_image_config *img = onvif_config_get_image();

	color->brightness = (float)img->brightness;
	color->contrast = (float)img->contrast;
	color->hue = (float)img->hue;
	color->saturation = (float)img->saturation;
	color->sharpness = (float)img->sharp;

	ak_print_normal_ex("get color hue:%.0f, brightness:%.0f, saturation:%.0f, contrast:%.0f, sharpness:%.0f\n", color->hue, color->brightness, color->saturation, color->contrast, color->sharpness);

	return 0;
}

/**
 * _set_color - set color params
 * @color[IN]: color params
 * @id[IN]: 0 main chn, 1 sub chn
 * return: 0 success, -1 fail
 */
static int _set_color(lpNVP_COLOR_CONFIG color, int id)
{
	ak_print_normal_ex("set color hue:%.0f, brightness:%.0f, saturation:%.0f, contrast:%.0f, sharpness:%.0f\n", color->hue, color->brightness, color->saturation, color->contrast, color->sharpness);

	ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_EFFECT_HUE, color->hue - IMG_EFFECT_DEF_VAL);
	ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_EFFECT_BRIGHTNESS, color->brightness - IMG_EFFECT_DEF_VAL);
	ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_EFFECT_SATURATION, color->saturation - IMG_EFFECT_DEF_VAL);
	ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_EFFECT_CONTRAST, color->contrast - IMG_EFFECT_DEF_VAL);
	ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_EFFECT_SHARP, color->sharpness - IMG_EFFECT_DEF_VAL);

	struct onvif_image_config *img = onvif_config_get_image();

	img->hue = (int)color->hue;
	img->brightness = (int)color->brightness;
	img->contrast = (int)color->contrast;
	img->saturation = (int)color->saturation;
	img->sharp = (int)color->sharpness;

	onvif_config_set_image(img);
	onvif_config_flush_data();

	return 0;
}

/**
 * _get_image_option - get image options
 * @image[OUT]: image options
 * @id[IN]: 0 main chn, 1 sub chn
 * return: 0 success, -1 fail
 */
static int _get_image_option(lpNVP_IMAGE_OPTIONS image, int id)
{
	// FIX me
	image->brightness.min = 0;
	image->brightness.max = 100;
	image->saturation.min = 0;
	image->saturation.max = 100;
	image->sharpness.min = 0;
	image->sharpness.max = 100;
	image->contrast.min = 0;
	image->contrast.max = 100;
	image->hue.min = 0;
	image->hue.max = 100;

	image->ircut_mode.nr = 3;
	image->ircut_mode.list[0] = 0;
	image->ircut_mode.list[1] = 1;
	image->ircut_mode.list[2] = 2;

	return 0;
}

/**
 * _get_image - get image params
 * @image[OUT]: image params
 * @id[IN]: 0 main chn, 1 sub chn
 * return: 0 success, -1 fail
 */
static int _get_image(lpNVP_IMAGE_CONFIG image, int id)
{
	image->ircut.control_mode = 0;
	image->ircut.ircut_mode = 0;

	image->wdr.enabled = AK_FALSE;
	image->wdr.WDRStrength = 0;

	image->manual_sharp.enabled = AK_FALSE;
	image->manual_sharp.sharpnessLevel = 0;

	image->d3d.enabled = AK_FALSE;
	image->d3d.denoise3dStrength = 0;

	_get_color(&image->color, id);

	_get_image_option(&image->option, id);

	return 0;
}

/**
 * _set_image - set image params
 * @image[IN]: image params
 * @id[IN]: 0 main chn, 1 sub chn
 * return: 0 success, -1 fail
 */
static int _set_image(lpNVP_IMAGE_CONFIG image, int id)
{
	_set_color(&image->color, id);

	return 0;
}

/**
 * _get_video_source - get video source params
 * @src[OUT]: video src params
 * @id[IN]: 0 main chn, 1 sub chn
 * return: 0 success, -1 fail
 */
static int _get_video_source(lpNVP_V_SOURCE src, int id)
{
	struct onvif_camera_config *camera = onvif_config_get_camera();

	if (0 == id)
	{
		src->resolution.width = camera->main_width;
		src->resolution.height = camera->main_height;
	}
	else if (1 == id)
	{
		src->resolution.width = camera->sub_width;
		src->resolution.height = camera->sub_height;
	}

	src->fps = 25;

	_get_image(&src->image, id);

	return 0;
}

static int _set_video_source(lpNVP_V_SOURCE src, int id)
{
	ak_print_normal_ex("name %s, token %s, fps %f, width %d, height %d\n", src->name,src->token,src->fps,src->resolution.width,src->resolution.height);
	return 0;
}

/**
 * _get_video_input_conf - get vi config
 * @vin[OUT]: vi config
 * @id[IN]: 0 main chn, 1 sub chn
 * return: 0 success, -1 fail
 */
static int _get_video_input_conf(lpNVP_VIN_CONFIG vin, int id)
{
	struct onvif_camera_config *camera = onvif_config_get_camera();

	vin->rect.nX = 0;
	vin->rect.nY = 0;

	if (0 == id)
	{
		vin->rect.width = camera->main_width;
		vin->rect.height = camera->main_height;
	}
	else if (1 == id)
	{
		vin->rect.width = camera->sub_width;
		vin->rect.height = camera->sub_height;
	}

	vin->rotate.enabled = AK_FALSE;

	vin->rotate.degree = 0;
	return 0;
}

static int _set_video_input_conf(lpNVP_VIN_CONFIG vin, int id)
{
	return 0;
}

/**
 * _get_video_encode_option - get video encode options
 * @venc[OUT]: venc options
 * @id[IN]: 0 main chn, 1 sub chn
 * return: 0 success, -1 fail
 */
static int _get_video_encode_option(lpNVP_VENC_OPTIONS venc, int id)
{
	int n;
	int fps_min = FPS_MIN, fps_max  = FPS_MAX;
	int bps_min = 32, bps_max = 4000;
	int resolution_nr = 1;
	int width[16] = {640, };
	int height[16] = {360, };

	struct onvif_camera_config *camera = onvif_config_get_camera();

	if (0 == id)
	{
		width[0] = 1280;
		height[0] = 720;

		if (camera->main_max_width >= VIDEO_WIDTH_1080P) {

			width[1] = VIDEO_WIDTH_1080P;
			height[1] = VIDEO_HEIGHT_1080P;

			resolution_nr++;
		}

		if (1536 == camera->main_max_height) {
			width[resolution_nr] = camera->main_max_width;
			height[resolution_nr] = camera->main_max_height;
			resolution_nr++;
		}
		
		bps_min = MAIN_BPS_MIN;
		bps_max = MAIN_BPS_MAX;
	}
	else if (1 == id)
	{
		width[0] = VIDEO_WIDTH_VGA;
		height[0] = VIDEO_HEIGHT_VGA;

		width[1] = 352;
		height[1] = 288;

		width[2] = 320;
		height[2] = 240;

		resolution_nr = 3;

		bps_min = SUB_BPS_MIN;
		bps_max = SUB_BPS_MAX;
	}

	// FIM Me
	venc->enc_fps.min = fps_min;
	venc->enc_fps.max = fps_max;

	venc->enc_gov.min = 1;
	venc->enc_gov.max = fps_max;

	venc->enc_interval.min = 1;
	venc->enc_interval.max = 1;

	venc->enc_quality.min = 0;
	venc->enc_quality.max = 4;


	venc->enc_bps.min  = bps_min;
	venc->enc_bps.max = bps_max;

	venc->resolution_nr = resolution_nr;
	for ( n = 0; n < resolution_nr; n++) {
		NVP_SET_SIZE(&venc->resolution[n], width[n], height[n]);
	}

	venc->enc_profile_nr = 1;
	venc->enc_profile[0] = NVP_H264_PROFILE_MAIN;

	return 0;
}

/**
 * _get_video_encode - get video encode params
 * @venc[OUT]: venc params to get
 * @id[IN]: 0 main chn, 1 sub chn
 * return: 0 success, -1 fail
 */
static int _get_video_encode(lpNVP_VENC_CONFIG venc, int id)
{
	struct onvif_camera_config *camera = onvif_config_get_camera();

	if (0 == id)
	{
		venc->width = camera->main_width;
		venc->height = camera->main_height;
		venc->enc_bps = g_video_set.mainbps;
		venc->enc_fps = g_video_set.mainfps;
		venc->quant_mode = g_video_set.main_videomode + NVP_QUANT_CBR;
		venc->enc_type = NVP_VENC_H264;
	}
	else if (1 == id)
	{
		venc->width = camera->sub_width;
		venc->height = camera->sub_height;
		venc->enc_bps = g_video_set.subbps;
		venc->enc_fps = g_video_set.subfps;
		venc->quant_mode = g_video_set.sub_videomode + NVP_QUANT_CBR;
		venc->enc_type = NVP_VENC_H264;
	}

	venc->enc_gov = 25;
	venc->enc_interval = 1;
	venc->enc_quality = 4;

	venc->enc_profile = NVP_H264_PROFILE_MAIN;//NVP_H264_PROFILE_MAIN;
	venc->user_count = 4;

	venc->option.index = venc->index;
	strcpy(venc->option.token, venc->token);
	strcpy(venc->option.enc_token, venc->enc_token);
	_get_video_encode_option(&venc->option, id);

	return 0;
}

/**
 * _set_video_encode - set video encode params
 * @venc[IN]: venc params to set
 * @id[IN]: 0 main chn, 1 sub chn
 * return: 0 success, -1 fail
 */
static int _set_video_encode(lpNVP_VENC_CONFIG venc, int id)
{
	ak_print_normal_ex("width %d,",venc->width);
	ak_print_normal_ex("heigth %d,",venc->height);
	ak_print_normal_ex("quant_mode %d,",venc->quant_mode);
	ak_print_normal_ex("enc_fps %d,enc_bps %d,enc_gov %d ,enc_type", venc->enc_fps,venc->enc_bps, venc->enc_gov);

	ja_media_set_venc_parm(id, venc->enc_bps, venc->enc_fps, 
								venc->quant_mode - NVP_QUANT_CBR);

	ja_media_change_resolution(venc->width, venc->height, id);

	return 0;
}

static int _get_audio_input(lpNVP_AIN_CONFIG ain, int id)
{
	return 0;
}

static int _set_audio_input(lpNVP_AIN_CONFIG ain, int id)
{
	return 0;
}

/**
 * _get_audio_encode - get audio encode params
 * @aenc[OUT]: aenc params to get
 * @id[IN]: 0 main chn, 1 sub chn
 * return: 0 success, -1 fail
 */
static int _get_audio_encode(lpNVP_AENC_CONFIG aenc, int id)
{
	//FIX me
	aenc->channel = 1;
	aenc->enc_type = NVP_AENC_G711;
	aenc->sample_size = 8;
	aenc->sample_rate = 8000;

	aenc->user_count = 2;
	return 0;
}

static int _set_audio_encode(lpNVP_AENC_CONFIG aenc, int id)
{
	ak_print_normal_ex("index %d,name %s,token %s,user_count %d,enc_type %d,sample_rate%d,sample_size %d\n",aenc->index,aenc->name,aenc->token,aenc->user_count,aenc->enc_type,aenc->sample_rate,aenc->sample_size);
	return 0;
}

static int _get_motion_detection(lpNVP_MD_CONFIG md, int id)
{
	md->type = NVP_MD_TYPE_GRID;

	md->grid.columnGranularity = 22;
	md->grid.rowGranularity = 15;
	md->grid.sensitivity = 80;
	memset(md->grid.granularity, 0xff, sizeof(md->grid.granularity));
	//FIM me
	md->grid.threshold = 5;
	// FIX me
	md->delay_off_alarm = 300;
	md->delay_on_alarm = 200;

	return 0;
}

static int _set_motion_detection(lpNVP_MD_CONFIG md, int id)
{
	return 0;
}

static int _get_video_analytic(lpNVP_VAN_CONFIG van, int id)
{
	return 0;
}

static int _set_video_analytic(lpNVP_VAN_CONFIG van, int id)
{
	return 0;
}


static int _get_ptz(lpNVP_PTZ_CONFIG ptz, int id)
{
	return 0;
}

static int _set_ptz(lpNVP_PTZ_CONFIG ptz, int id)
{
	return 0;
}

/**
 * _get_profile - get onvif profile
 * @profile[OUT]: profile to get
 * return: 0 success, -1 fail
 */
static int _get_profile(lpNVP_PROFILE_CHN profile)
{
	int i;
	// FIX me
	profile->profile_nr = 2;
	profile->venc_nr = profile->profile_nr;
	profile->aenc_nr = 1;

	for (i = 0; i < profile->venc_nr; i++) {
		_get_video_encode(&profile->venc[i], i);
	}
	for (i = 0; i < profile->aenc_nr; i++) {
		_get_audio_encode(&profile->aenc[i], i);
	}
	_get_video_source(&profile->v_source, profile->index);
	for (i = 0; i < profile->vin_conf_nr; i++) {
		_get_video_input_conf(&profile->vin[i], i);
	}
	_get_audio_input(&profile->ain, profile->index);
	_get_ptz(&profile->ptz, profile->index);
	_get_video_analytic(&profile->van, profile->index);
	_get_motion_detection(&profile->md, profile->index);

	return 0;
}

/**
 * _set_profile - set onvif profile
 * @profile[IN]: profile to set
 * return: 0 success, -1 fail
 */
static int _set_profile(lpNVP_PROFILE_CHN profile)
{
	int i;
	int ret = 0;
	//
	for (i = 0; i < profile->venc_nr; i++) {
		if (_set_video_encode(&profile->venc[i], profile->index) < 0)
			ret = -1;
	}
	for (i = 0; i < profile->aenc_nr; i++) {
		if (_set_audio_encode(&profile->aenc[i], profile->index) < 0)
			ret = -1;
	}
	if (_set_video_source(&profile->v_source, profile->index) < 0)
		ret = -1;
	for (i = 0; i < profile->vin_conf_nr; i++) {
		if (_set_video_input_conf(&profile->vin[i], profile->index) < 0)
			ret = -1;
	}
	if (_set_audio_input(&profile->ain, profile->index) < 0)
		ret = -1;
	if (_set_ptz(&profile->ptz, profile->index) < 0)
		ret = -1;
	if (_set_video_analytic(&profile->van, profile->index) < 0)
		ret = -1;
	if (_set_motion_detection(&profile->md, profile->index) < 0)
		ret = -1;

	return ret;
}

/**
 * _get_profiles - get onvif profiles
 * @profiles[OUT]: profiles to get
 * return: 0 success, -1 fail
 */
static int _get_profiles(lpNVP_PROFILE profiles)
{
	int i;

	profiles->chn = NVP_MAX_CH;
	//
	for ( i = 0; i < profiles->chn; i++) {
		_get_profile(&profiles->profile[i]);
	}
	return 0;
}

/**
 * _set_profiles - set onvif profiles
 * @profiles[IN]: profiles to set
 * return: 0 success, -1 fail
 */
static int _set_profiles(lpNVP_PROFILE profiles)
{
	int i;
	int ret = 0;
	//
	for ( i = 0; i < profiles->chn; i++) {
		if(_set_profile(&profiles->profile[i]) < 0)
			ret = -1;
	}
	return ret;
}

/**
 * _get_user - get user define ,such as snapshot
 * @pUser[IN]: user params, snapshot_url
 * @id[IN]: 
 * return: 0 success, -1 fail
 */
static int  _get_user(lpNVP_USER pUser, int id)
{
	struct onvif_net_config *net_info = onvif_config_get_net();
	
	snprintf(pUser->snapshot_url, sizeof(pUser->snapshot_url), "http://%s:%s/snapshot.jpg?size=-1x-1&download=yes",
		net_info->ipaddr, getenv("ONVIF_PORT"));

	ak_print_normal_ex("onvif http://%s:%s/snapshot.jpg\n", net_info->ipaddr, getenv("ONVIF_PORT"));
	
	return 0;
}

/**
 * _get_all - get all cmd execute
 * @env[OUT]: params to get
 * return: 0 success, -1 fail
 */
static int _get_all(lpNVP_ENV env)
{
	_get_system_information(&env->devinfo);
	_get_date_time(&env->systime);
	_get_interface(&env->ether);
	_get_profiles(&env->profiles);

	return 0;
}

/**
 * _set_all - set all cmd execute
 * @env[IN]: params to set
 * return: 0 success, -1 fail
 */
static int _set_all(lpNVP_ENV env)
{
	int ret = 0;

	if (_set_date_time(&env->systime) < 0)
		ret = -1;
	if (_set_interface(&env->ether) < 0)
		ret = -1;
	if (_set_profiles(&env->profiles) < 0)
		ret = -1;

	return ret;
}

/**
 * _cmd_system_boot - system reboot cmd execute
 * @l[IN]: no use
 * @r[IN]: no use
 * return: void
 */
static void _cmd_system_boot(long l, void *r)
{
	//TICKER_del_task(_cmd_system_boot);
	ak_print_normal_ex("system reboot now...\n");
	ak_cmd_exec("sleep 2; reboot", NULL, 0);
}

/**
 * _cmd_ptz - ptz cmd execute, if not support ptz, do nothing
 * @cmd[IN]: cmd
 * @module[IN]: module
 * @keyid[IN]: keyid
 * return: 0 success, -1 fail
 */
static int _cmd_ptz(lpNVP_CMD cmd, const char *module, int keyid)
{
	const char *ptz_cmd_name[] =
	{
		"PTZ_CMD_UP",
		"PTZ_CMD_DOWN",
		"PTZ_CMD_LEFT",
		"PTZ_CMD_RIGHT",
		"PTZ_CMD_LEFT_UP",
		"PTZ_CMD_RIGHT_UP",
		"PTZ_CMD_LEFT_DOWN",
		"PTZ_CMD_RIGHT_DOWN",
		"PTZ_CMD_AUTOPAN",
		"PTZ_CMD_IRIS_OPEN",
		"PTZ_CMD_IRIS_CLOSE",
		"PTZ_CMD_ZOOM_IN",
		"PTZ_CMD_ZOOM_OUT",
		"PTZ_CMD_FOCUS_FAR",
		"PTZ_CMD_FOCUS_NEAR",
		"PTZ_CMD_STOP",
		"PTZ_CMD_WIPPER_ON",
		"PTZ_CMD_WIPPER_OFF",
		"PTZ_CMD_LIGHT_ON",
		"PTZ_CMD_LIGHT_OFF",
		"PTZ_CMD_POWER_ON",
		"PTZ_CMD_POWER_OFF",
		"PTZ_CMD_GOTO_PRESET",
		"PTZ_CMD_SET_PRESET",
		"PTZ_CMD_CLEAR_PRESET",
		"PTZ_CMD_TOUR",
	};

	ak_print_normal_ex("%s(%d)\n", ptz_cmd_name[cmd->ptz.cmd], cmd->ptz.cmd);
	switch(cmd->ptz.cmd)
	{
		case NVP_PTZ_CMD_LEFT:
			break;
		case NVP_PTZ_CMD_RIGHT:
			break;
		case NVP_PTZ_CMD_UP:
			break;
		case NVP_PTZ_CMD_DOWN:
			break;
		case NVP_PTZ_CMD_ZOOM_IN:
			break;
		case NVP_PTZ_CMD_ZOOM_OUT:
			break;
		case NVP_PTZ_CMD_SET_PRESET:
			break;
		case NVP_PTZ_CMD_GOTO_PRESET:
			break;
		case NVP_PTZ_CMD_CLEAR_PRESET:
			break;
		case NVP_PTZ_CMD_STOP:
			break;
		default:
			break;
	}

	return 0;
}

static int _cmd_Iframe(lpNVP_CMD cmd, const char *module, int keyid)
{
	if((0 == keyid) || (1 == keyid))
	{
		ak_venc_set_iframe(pja_ctrl->media[keyid].enc_handle);
	}

	return 0;
}

/**
 * _nvp_env_init - init some onvif params
 * @env[OUT]: params to init
 * return: void
 */
static void _nvp_env_init(lpNVP_ENV env)
{
#define ONVIF_SET_NT(ptr, value)			snprintf(ptr, sizeof(ptr), "%s", value)
#define ONVIF_SET_NT_CHN(ptr, value,chn)	snprintf(ptr, sizeof(ptr), "%s%d", value, chn)
#define ONVIF_SET_NT_ID(ptr, value,chn, id)	snprintf(ptr, sizeof(ptr), "%s%d_%d", value, chn, id)
	int i, n;

	if (env == NULL)
		return;

	memset(&env->devinfo, 0, sizeof(stNVP_DEV_INFO));
	memset(&env->systime, 0, sizeof(stNVP_DATE_TIME));
	memset(&env->ether, 0, sizeof(stNVP_ETHER_CONFIG));
	memset(&env->profiles, 0, sizeof(stNVP_PROFILE));

	env->profiles.chn =  NVP_CH_NR;
	for ( n = 0; n < NVP_MAX_CH; n++)
	{
		env->profiles.profile[n].index = n;
		env->profiles.profile[n].profile_nr = NVP_VENC_NR;
		for ( i = 0; i < NVP_MAX_VENC; i++) {
			ONVIF_SET_NT_ID(env->profiles.profile[n].name[i], "Profile",n,  i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].token[i], "ProfileToken", n,i);

			ONVIF_SET_NT_ID(env->profiles.profile[n].ain_name[i], "AIN", n,i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].ain_token[i], "AINToken", n,i);
		}

		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.name, "VIN", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.token, "VINToken", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.image.name, "IMG", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.image.token, "IMGToken", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.image.src_token, "VINToken", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].v_source.image.color.src_token, "VINToken", n);

		env->profiles.profile[n].venc_nr = NVP_VENC_NR;
		for ( i = 0; i < NVP_MAX_VENC; i++) {
			env->profiles.profile[n].venc[i].index = i;
			ONVIF_SET_NT_ID(env->profiles.profile[n].venc[i].name, "Profile", n, i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].venc[i].token, "ProfileToken", n, i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].venc[i].enc_name, "VENC", n, i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].venc[i].enc_token, "VENCToken", n, i);
		}

		env->profiles.profile[n].vin_conf_nr = NVP_VIN_IN_A_SOURCE;
		for ( i = 0; i < NVP_MAX_VIN_IN_A_SOURCE; i++) {
			env->profiles.profile[n].venc[i].index = i;
			ONVIF_SET_NT_ID(env->profiles.profile[n].vin[i].name, "VIN", n, i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].vin[i].token, "VINToken", n, i);
		}

		ONVIF_SET_NT_CHN(env->profiles.profile[n].ain.name, "AIN", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].ain.token, "AINToken", n);

		env->profiles.profile[n].aenc_nr = NVP_AENC_NR;
		for ( i = 0; i < NVP_MAX_AENC; i++) {
			env->profiles.profile[n].aenc[i].index = i;
			ONVIF_SET_NT_ID(env->profiles.profile[n].aenc[i].name, "AENC", n, i);
			ONVIF_SET_NT_ID(env->profiles.profile[n].aenc[i].token, "AENCToken", n, i);
		}

		ONVIF_SET_NT_CHN(env->profiles.profile[n].van.name, "VAN", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].van.token, "VANToken", n);

		ONVIF_SET_NT_CHN(env->profiles.profile[n].md.rule_name, "MDRule", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].md.module_name, "MDModule", n);

		ONVIF_SET_NT_CHN(env->profiles.profile[n].ptz.name, "PTZ", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].ptz.token, "PTZToken", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].ptz.node_name, "PTZNode", n);
		ONVIF_SET_NT_CHN(env->profiles.profile[n].ptz.node_token, "PTZNodeToken", n);
		env->profiles.profile[n].ptz.preset_nr = NVP_MAX_PTZ_PRESET;
		for ( i  = 0; i < NVP_MAX_PTZ_PRESET; i++) {
			env->profiles.profile[n].ptz.preset[i].index = i;
			memset(env->profiles.profile[n].ptz.preset[i].name, 0, sizeof(env->profiles.profile[n].ptz.name));
			sprintf(env->profiles.profile[n].ptz.preset[i].token, "PresetToken%d", i + 1);
			env->profiles.profile[n].ptz.preset[i].in_use = AK_FALSE;
		}
		env->profiles.profile[n].ptz.default_pan_speed = 0.5;
		env->profiles.profile[n].ptz.default_tilt_speed = 0.5;
		env->profiles.profile[n].ptz.default_zoom_speed = 0.5;
		env->profiles.profile[n].ptz.tour_nr = 0;
		for ( i  = 0; i < NVP_MAX_PTZ_TOUR; i++) {
			env->profiles.profile[n].ptz.tour[i].index = i;
			memset(env->profiles.profile[n].ptz.tour[i].name, 0, sizeof(env->profiles.profile[n].ptz.tour[i].name));
			sprintf(env->profiles.profile[n].ptz.tour[i].token, "TourToken%d", i + 1);
		}
	}


	ONVIF_SET_NT(env->ether.name, "eth0");
	ONVIF_SET_NT(env->ether.token, "Eth0Token");
}

/**
 * NVP_env_load - callback function for onvif get params
 * @env[OUT]: params to get
 * @module[IN]: module
 * @keyid[IN]: convert to chn and id
 * return: 0 success, -1 fail
 */
int NVP_env_load(lpNVP_ENV env, const char *module, int keyid)
{
	char temp[512];
	char *ptr = NULL, *pbuf;
	char *saveptr = NULL;
	int ret = -1;
	int chn, id;
	static int onvif_init=0;

	if (onvif_init == 0)
		_nvp_env_init(env);

	onvif_init = 1;

	chn = keyid/100;
	id = keyid%100;
	strncpy(temp, module, 512);
	pbuf = temp;

	//ak_print_normal_ex("NVP_env_load: %s\n", module);

	while((ptr = strtok_r(pbuf, OM_AND, &saveptr)) != NULL)
	{
		if (OM_MATCH(ptr, OM_ALL)) {
			ret = _get_all(env);
			break;
		}else if (OM_MATCH(ptr, OM_PROFILE)) {
			ret = _get_profile(&env->profiles.profile[chn]);
		}else if (OM_MATCH(ptr, OM_PROFILES)) {
			ret = _get_profiles(&env->profiles);
		}else if (OM_MATCH(ptr, OM_INFO)) {
			ret = _get_system_information(&env->devinfo);
		}else if (OM_MATCH(ptr, OM_DTIME)) {
			ret = _get_date_time(&env->systime);
		}else if (OM_MATCH(ptr, OM_NET)) {
			ret = _get_interface(&env->ether);
		}  else if (OM_MATCH(ptr, OM_VENC)) {
			ret = _get_video_encode(&env->profiles.profile[chn].venc[id], id);
		}  else if (OM_MATCH(ptr, OM_VSRC)) {
			ret = _get_video_source(&env->profiles.profile[chn].v_source, id);
		}  else if (OM_MATCH(ptr, OM_VINC)) {
			ret = _get_video_input_conf(&env->profiles.profile[chn].vin[id], id);
		}  else if (OM_MATCH(ptr, OM_AENC)) {
			ret = _get_audio_encode(&env->profiles.profile[chn].aenc[id], id);
		}  else if (OM_MATCH(ptr, OM_AIN)) {
			ret = _get_audio_input(&env->profiles.profile[chn].ain, id);
		}  else if (OM_MATCH(ptr, OM_COLOR)) {
			ret = _get_color(&env->profiles.profile[chn].v_source.image.color, id);
		}  else if (OM_MATCH(ptr, OM_IMG)) {
			ret = _get_image(&env->profiles.profile[chn].v_source.image, id);
		}  else if (OM_MATCH(ptr, OM_MD)) {
			ret = _get_motion_detection(&env->profiles.profile[chn].md, id);
		}  else if (OM_MATCH(ptr, OM_PTZ)) {
			ret = _get_ptz(&env->profiles.profile[chn].ptz, id);
		}  else if (OM_MATCH(ptr, OM_USER)) {
			ret = _get_user(&env->user, id);
		} else {
			ak_print_normal_ex("unknown env module: %s\n", ptr);
		}
		pbuf = NULL;
		ak_print_info_ex("ret: %d\n", ret);
	}

	return ret;
}

/**
 * NVP_env_save - callback function for onvif set params
 * @env[IN]: params to set
 * @module[IN]: module
 * @keyid[IN]: convert to chn and id
 * return: 0 success, -1 fail
 */
int NVP_env_save(lpNVP_ENV env, const char *module, int keyid)
{
	char temp[512];
	char *ptr = NULL, *pbuf;
	char *saveptr = NULL;
	int ret=0;
	int f_ret = 0;
	int chn, id;

	chn = keyid / 100;
	id = keyid % 100;
	strncpy(temp, module, 512);
	pbuf = temp;

	ak_print_normal_ex("NVP_env_save: %s\n", module);

	while((ptr = strtok_r(pbuf, OM_AND, &saveptr)) != NULL)
	{
		if (OM_MATCH(ptr, OM_ALL)) {
			ret = _set_all(env);
			break;
		}else if (OM_MATCH(ptr, OM_PROFILE)) {
			ret = _set_profile(&env->profiles.profile[chn]);
		}else if (OM_MATCH(ptr, OM_PROFILES)) {
			ret = _set_profiles(&env->profiles);
		}else if (OM_MATCH(ptr, OM_INFO)) {
			//
		}else if (OM_MATCH(ptr, OM_DTIME)) {
			ret = _set_date_time(&env->systime);
		}else if (OM_MATCH(ptr, OM_NET)) {
			ret = _set_interface(&env->ether);
		}  else if (OM_MATCH(ptr, OM_VENC)) {
			ret = _set_video_encode(&env->profiles.profile[chn].venc[id], id);
		}  else if (OM_MATCH(ptr, OM_VSRC)) {
			ret = _set_video_source(&env->profiles.profile[chn].v_source, id);
		}  else if (OM_MATCH(ptr, OM_VINC)) {
			ret = _set_video_input_conf(&env->profiles.profile[chn].vin[id], id);
		}  else if (OM_MATCH(ptr, OM_AENC)) {
			ret = _set_audio_encode(&env->profiles.profile[chn].aenc[id], id);
		}  else if (OM_MATCH(ptr, OM_AIN)) {
			ret = _set_audio_input(&env->profiles.profile[chn].ain, id);
		}  else if (OM_MATCH(ptr, OM_COLOR)) {
			ret = _set_color(&env->profiles.profile[chn].v_source.image.color, id);
		}  else if (OM_MATCH(ptr, OM_IMG)) {
			ret = _set_image(&env->profiles.profile[chn].v_source.image, id);
		}  else if (OM_MATCH(ptr, OM_MD)) {
			ret = _set_motion_detection(&env->profiles.profile[chn].md, id);
		}  else if (OM_MATCH(ptr, OM_PTZ)) {
			ret = _set_ptz(&env->profiles.profile[chn].ptz, id);
		} else {
			ak_print_normal_ex("unknown env module: %s\n", ptr);
			f_ret = -1;
		}
		if (ret < 0)
			f_ret = -1;
		pbuf = NULL;
	}
	return f_ret;

}

/**
 * NVP_env_cmd - callback function for onvif cmd
 * @cmd[IN]: cmd
 * @module[IN]: module
 * @keyid[IN]: keyid
 * return: 0 success, -1 fail
 */
int NVP_env_cmd(lpNVP_CMD cmd, const char *module, int keyid)
{
	char temp[512];
	char *ptr = NULL, *pbuf;
	char *saveptr = NULL;
	int ret;
	int f_ret = 0;
	//int chn, id;
	int id ;

	//chn = keyid / 100;
	id = keyid % 100;
	strncpy(temp, module, 512);
	pbuf = temp;

	ak_print_normal_ex("NVP_env_cmd: %s\n", module);

	while((ptr = strtok_r(pbuf, OM_AND, &saveptr)) != NULL)
	{
		if (OM_MATCH(ptr, OM_REBOOT)) {
			_cmd_system_boot(0, 0);
		}else if (OM_MATCH(ptr, OM_SYS_RESET)) {
			ak_print_normal_ex("unknown env module: %s\n", ptr);
		}else if (OM_MATCH(ptr, OM_PTZ)) {
			ret = _cmd_ptz(cmd, module, keyid);
		}else if (OM_MATCH(ptr, OM_IFRAME)) {
			ret = _cmd_Iframe(cmd, module, id);
		} else {
			ak_print_normal_ex("unknown env module: %s\n", ptr);
			f_ret = -1;
		}
		if (ret < 0)
			f_ret = -1;
		pbuf = NULL;
	}

	return f_ret;

}
