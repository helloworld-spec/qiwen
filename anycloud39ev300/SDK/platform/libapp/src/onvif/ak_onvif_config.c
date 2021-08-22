#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "ak_ini.h"
#include "ak_common.h"
#include "ak_net.h"
#include "ak_cmd_exec.h"

#include "ak_onvif_config.h"

/*if onvif_cfg.ini is modified ,need to add this version number*/
#define ONVIF_CFG_VERSION				(3005)

#define ONVIF_BUF_SIZE					(100)
#define ONVIF_CONFIG_VALUE_BUF_SIZE		(200)
#define ONVIF_CONFIG_DUMP_INFO			0

/* default config file */
#define ONVIF_CONFIG_FILE_NAME  	"/etc/jffs2/onvif_cfg.ini"

struct onvif_config_t {
	void *handle;

    struct onvif_sys_config *sys;              /* title: global */
	struct onvif_venc_config *video;		    /* title: video */
	struct onvif_net_config *net;			    /* title: ethernet */
	struct onvif_wifi_config *wifi;		    /* title: wireless */
	struct onvif_camera_config *camera;	    /* title: camera */
	struct onvif_image_config *image;			/* title: image */
	struct onvif_audio_config *audio;			/* title: audio */
	struct onvif_alarm_config *alarm;			/* title: alarm */
};

static struct onvif_config_t onvif_config = {NULL};

/**
 * init_onvif_net: get net config, title: ethernet
 * @net[OUT]: net config
 * return: void
 */
static void init_onvif_net(struct onvif_net_config *net)
{
    char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "ethernet", "dhcp", value))
    	net->dhcp = atoi(value);
	else
		net->dhcp = 0;	//0 static ip address, 1 dynamic ip address
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "ethernet", "ipaddr", value))
    	strcpy(net->ipaddr, value);
	else
		strcpy(net->ipaddr, "192.168.1.88");
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "ethernet", "netmask", value))
    	strcpy(net->netmask, value);
	else
		strcpy(net->netmask, "255.255.255.0");
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "ethernet", "gateway", value))
    	strcpy(net->gateway, value);
	else
		strcpy(net->gateway, "192.168.1.1");
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "ethernet", "firstdns", value))
    	strcpy(net->firstdns, value);
	else
		strcpy(net->firstdns, "8.8.8.8");
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "ethernet", "backdns", value))
    	strcpy(net->backdns, value);
	else
		strcpy(net->backdns, "108.108.108.108");
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "ethernet", "ip_adjust_en", value))
    	net->ip_adjust_en = atoi(value);
	else
		net->ip_adjust_en = 0;	//0 close ip adjust, 1 for onvif auto change ip to client network segment 

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "ethernet", "hikonport", value))
    	net->hikonport = atoi(value);
	else
		net->hikonport = 8089;

	onvif_config_set_net(net);
}

/**
 * init_onvif_sys: get system config, title: global
 * @sys[OUT]: sys config
 * return: void
 */
static void init_onvif_sys(struct onvif_sys_config *sys)
{
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "global", "dev_name", value))
		strcpy(sys->dev_name, value);
	else
		strcpy(sys->dev_name, "IPC");
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "global", "soft_version", value))
		sys->soft_version = atoi(value);
	else
		sys->soft_version = 3000;
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "global", "tzone", value))
		sys->tzone = atoi(value);
	else
		sys->tzone = 800;	//china

	onvif_config_set_sys(sys);
}

/**
 * init_onvif_venc_param: get venc config, title: video
 * @video[OUT]: video config
 * return: void
 */
static void init_onvif_venc_param(struct onvif_venc_config *video)
{
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

	/* get all item values of video title */
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_min_qp_264", value))
    	video->main_min_qp_264 = atoi(value);
	else
		video->main_min_qp_264 = 28;

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_min_qp_265", value))
    	video->main_min_qp_265 = atoi(value);
	else
		video->main_min_qp_265 = 28;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_max_qp_264", value))
    	video->main_max_qp_264 = atoi(value);
	else
		video->main_max_qp_264 = 43;

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_max_qp_265", value))
    	video->main_max_qp_265 = atoi(value);
	else
		video->main_max_qp_265 = 48;
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_min_qp_264", value))
    	video->sub_min_qp_264 = atoi(value);
	else
		video->sub_min_qp_264 = 20;

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_min_qp_265", value))
    	video->sub_min_qp_265 = atoi(value);
	else
		video->sub_min_qp_265 = 20;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_max_qp_264", value))
		video->sub_max_qp_264 = atoi(value);
	else
		video->sub_max_qp_264 = 43;

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_max_qp_265", value))
		video->sub_max_qp_265 = atoi(value);
	else
		video->sub_max_qp_265 = 43;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_fps", value))
    	video->main_fps = atoi(value);
	else
		video->main_fps = 25;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_kbps", value))
		video->main_kbps = atoi(value);
	else
		video->main_kbps = 2048;

    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_fps", value))
    	video->sub_fps = atoi(value);
	else
		video->sub_fps = 25;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_kbps", value))
		video->sub_kbps = atoi(value);
	else
		video->sub_kbps = 512;

	/*
	* bps target ratio ,diffrent for main /sub channel
	*/
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_targetratio_264", value))
		video->main_targetratio_264 = atoi(value);
	else
		video->main_targetratio_264 = 90;

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_targetratio_264", value))
		video->sub_targetratio_264 = atoi(value);
	else
		video->sub_targetratio_264 = 90;

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_targetratio_265", value))
		video->main_targetratio_265 = atoi(value);
	else
		video->main_targetratio_265 = 70;

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_targetratio_265", value))
		video->sub_targetratio_265 = atoi(value);
	else
		video->sub_targetratio_265 = 90;

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_targetratio_264_smart", value))
		video->main_targetratio_264_smart = atoi(value);
	else
		video->main_targetratio_264_smart = 90;

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_targetratio_264_smart", value))
		video->sub_targetratio_264_smart = atoi(value);
	else
		video->sub_targetratio_264_smart = 100;

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_targetratio_265_smart", value))
		video->main_targetratio_265_smart = atoi(value);
	else
		video->main_targetratio_265_smart = 90;

	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_targetratio_265_smart", value))
		video->sub_targetratio_265_smart = atoi(value);
	else
		video->sub_targetratio_265_smart = 100;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_goplen", value))
    	video->main_goplen = atoi(value);
	else
		video->main_goplen = 2;
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_goplen", value))
    	video->sub_goplen = atoi(value);
	else
		video->sub_goplen = 2;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_video_mode", value))
    	video->main_video_mode = atoi(value);
	else
		video->main_video_mode = 1;	// 0->CBR, 1->VBR
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_video_mode", value))
    	video->sub_video_mode = atoi(value);
	else
		video->sub_video_mode = 1;	// 0->CBR, 1->VBR
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_enc_type", value))
    	video->main_enc_type = atoi(value);
	else
		video->main_enc_type = 2;	// 1->264, 2->265, 3->264+, 4->265+
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_enc_type", value))
    	video->sub_enc_type = atoi(value);
	else
		video->sub_enc_type = 2;	// 1->264, 2->265, 3->264+, 4->265+
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_smart_mode", value))
		video->main_smart_mode = atoi(value);
	else
		video->main_smart_mode = 1;	//1->LTR, 2->change goplen
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_smart_mode", value))
		video->sub_smart_mode = atoi(value);
	else
		video->sub_smart_mode = 1;	//1->LTR, 2->change goplen
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_smart_goplen", value))
		video->main_smart_goplen = atoi(value);
	else
		video->main_smart_goplen = 16;
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_smart_goplen", value))
		video->sub_smart_goplen = atoi(value);
	else
		video->sub_smart_goplen = 16;


	/*
	* smart quality ,diffrent for main /sub channel
	*/
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_smart_quality_264", value))
		video->main_smart_quality_264 = atoi(value);
	else
		video->main_smart_quality_264 = 70;
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_smart_quality_264", value))
		video->sub_smart_quality_264 = atoi(value);
	else
		video->sub_smart_quality_264 = 80;
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "main_smart_quality_265", value))
		video->main_smart_quality_265 = atoi(value);
	else
		video->main_smart_quality_265 = 40;
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "sub_smart_quality_265", value))
		video->sub_smart_quality_265 = atoi(value);
	else
		video->sub_smart_quality_265 = 50;

	/*
	* to judge if it is static scene or dynamic scene when smart is open.
	* if static blocks num larger than this param ,it will be judged to static scene.
	* total 32 *24 = 768 blocks. 
	*/
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "video", "smart_static_value", value))
		video->smart_static_value = atoi(value);
	else
		video->smart_static_value = 550;
	onvif_config_set_venc_param(video);
}

/**
 * get_onvif_net_info: get net info
 * return: void
 */
static void get_onvif_net_info(void)
{
	/* get iface name */
	char iface[10] = {0};
	char ip[16] = {0}, netmask[16] = {0}, route[16] = {0},
		 mdns[16] = {0}, sdns[16] = {0};

	/* interface, use for get specifically interface's netinfo */
	if (ak_net_get_cur_iface(iface)) {
		ak_print_error_ex("no working net face\n");
		return;
	}

	/* ip */
	if (!ak_net_get_ip(iface, ip)) {
		memset(onvif_config.net->ipaddr, 0, sizeof(onvif_config.net->ipaddr));
		strcpy(onvif_config.net->ipaddr, ip);
	}

	/* netmask */
	if (!ak_net_get_netmask(iface, netmask)) {
		memset(onvif_config.net->netmask, 0, sizeof(onvif_config.net->netmask));
		strcpy(onvif_config.net->netmask, netmask);
	}

	/* route */
	if (!ak_net_get_route(iface, route)) {
		memset(onvif_config.net->gateway, 0, sizeof(onvif_config.net->gateway));
		strcpy(onvif_config.net->gateway, route);
	}

	/* first dns */
	if (ak_net_get_dns(0, mdns)) {
	    strcpy(mdns, route);
	} else {
	    memset(onvif_config.net->firstdns, 0, sizeof(onvif_config.net->firstdns));
		strcpy(onvif_config.net->firstdns, mdns);
	}

	/* back dns */
	if (ak_net_get_dns(1, sdns)) {
	    strcpy(sdns, mdns);
	} else {
	    memset(onvif_config.net->backdns, 0, sizeof(onvif_config.net->backdns));
		strcpy(onvif_config.net->backdns, sdns);
	}
}

/**
 * init_onvif_wifi: get wifi config, title: wireless
 * @net[OUT]: wifi config
 * return: void
 */
static void init_onvif_wifi(struct onvif_wifi_config *net)
{
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

    ak_ini_get_item_value(onvif_config.handle, "wireless", "ssid", value);
    strcpy(net->ssid, value);
    ak_ini_get_item_value(onvif_config.handle, "wireless", "password", value);
    strcpy(net->passwd, value);
}

/**
 * init_onvif_camera: get camera config, title: camera
 * @camera[OUT]: camera config
 * return: void
 */
static void init_onvif_camera(struct onvif_camera_config *camera)
{
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "main_width", value))
    	camera->main_width = atoi(value);
	else
		camera->main_width = 1920;
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "main_height", value))
    	camera->main_height = atoi(value);
	else
		camera->main_height = 1080;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "sub_width", value))
    	camera->sub_width = atoi(value);
	else
		camera->sub_width = 640;
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "sub_height", value))
    	camera->sub_height = atoi(value);
	else
		camera->sub_height = 360;


	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "main_max_width", value))
    	camera->main_max_width = atoi(value);
	else
		camera->main_max_width = 1920;
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "main_max_height", value))
    	camera->main_max_height = atoi(value);
	else
		camera->main_max_height = 1080;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "sub_max_width", value))
    	camera->sub_max_width = atoi(value);
	else
		camera->sub_max_width = 640;
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "sub_max_height", value))
    	camera->sub_max_height = atoi(value);
	else
		camera->sub_max_height = 480;
	
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "main_osd_size", value))
    	camera->main_osd_size = atoi(value);
	else
		camera->main_osd_size = 48;
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "sub_osd_size", value))
    	camera->sub_osd_size = atoi(value);
	else
		camera->sub_osd_size = 16;

    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "name_position", value))
    	camera->name_position = atoi(value);
	else
		camera->name_position = 2;	// 0->off 1->left-bottom, 2->left-top, 3->right-top, 4->right-bottom
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "time_position", value))
    	camera->time_position = atoi(value);
	else
		camera->time_position = 2;	// 0->off 1->left-bottom, 2->left-top, 3->right-top, 4->right-bottom
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "rate_position", value))
    	camera->rate_position = atoi(value);
	else
		camera->rate_position = 0;	// 0->off 1->left-bottom, 2->left-top, 3->right-top, 4->right-bottom
		
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "osd_switch", value))
    	camera->osd_switch = atoi(value);
	else
		camera->osd_switch = 1;	// 1 -> on, 0 -> off

    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "osd_name", value))
    	strcpy(camera->osd_name, value);
	else
		strcpy(camera->osd_name, "IPCAM");

    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "date_format", value))
    	camera->date_format = atoi(value);
	else
		camera->date_format = 1;
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "hour_format", value))
    	camera->hour_format = atoi(value);
	else
		camera->hour_format = 1;
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "camera", "week_format", value))
    	camera->week_format = atoi(value);
	else
		camera->week_format = 1;	//0 not display, 1 Chinese, 2 English

	onvif_config_set_camera(camera);
}

/**
 * init_onvif_image: get image config, title: image
 * @image[OUT]: image config
 * return: void
 */
static void init_onvif_image(struct onvif_image_config *image)
{
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

	/* get all item values of image title */
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "image", "hue", value))
    	image->hue = atoi(value);
	else
		image->hue = 50;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "image", "brightness", value))
    	image->brightness = atoi(value);
	else
		image->brightness = 50;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "image", "saturation", value))
    	image->saturation = atoi(value);
	else
		image->saturation = 50;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "image", "contrast", value))
    	image->contrast = atoi(value);
	else
		image->contrast = 50;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "image", "sharp", value))
    	image->sharp = atoi(value);
	else
		image->sharp = 50;

    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "image", "flip", value))
		image->flip = atoi(value);
	else
		image->flip = 0;
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "image", "mirror", value))
    	image->mirror = atoi(value);
	else
		image->mirror = 0;
	
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "image", "ircut_mode", value))
    	image->ircut_mode = atoi(value);
	else
		image->ircut_mode = 0;	//0 auto switch, 1 day mode, 2 night mode
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "image", "force_anti_flicker_flag", value))
    	image->force_anti_flicker_flag = atoi(value);
	else
		image->force_anti_flicker_flag = 0;	

    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "image", "irled_mode", value))
    	image->irled_mode = atoi(value);
	else
		image->irled_mode = 1;

	onvif_config_set_image(image);
}

/**
 * init_onvif_audio: get audio config, title: audio
 * @audio[OUT]: audio config
 * return: void
 */
static void init_onvif_audio(struct onvif_audio_config *audio)
{
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

	/* get all item values of audio title */
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "audio", "ai_source", value))
    	audio->ai_source = atoi(value);
	else
		audio->ai_source = 2;	// 1 linein, 2 mic in
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "audio", "aenc_type", value))
    	audio->aenc_type = atoi(value);
	else
		audio->aenc_type = 0;	// 0 g711a
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "audio", "ai_enable", value))
    	audio->ai_enable = atoi(value);
	else
		audio->ai_enable = 1;

	onvif_config_set_audio(audio);
}

/**
 * init_onvif_alarm: get md alarm config, title: alarm
 * @alarm[OUT]: alarm config
 * return: void
 */
static void init_onvif_alarm(struct onvif_alarm_config *alarm)
{
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

	/* get all item values of alarm title */
    if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "alarm", "md_level_1", value))
    	alarm->md_level_1 = atoi(value);
	else
		alarm->md_level_1 = 10;
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "alarm", "md_level_2", value))
    	alarm->md_level_2 = atoi(value);
	else
		alarm->md_level_2 = 30;
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "alarm", "md_level_3", value))
    	alarm->md_level_3 = atoi(value);
	else
		alarm->md_level_3 = 50;
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "alarm", "md_level_4", value))
    	alarm->md_level_4 = atoi(value);
	else
		alarm->md_level_4 = 70;
	
	if (AK_SUCCESS == ak_ini_get_item_value(onvif_config.handle, "alarm", "md_level_5", value))
    	alarm->md_level_5 = atoi(value);
	else
		alarm->md_level_5 = 90;

	onvif_config_set_alarm(alarm);
}

/**
 * update_onvif_config: update onvif config,
 * if onvif_cfg.ini need to update, don't copy the whole file, modify this function to update appointed params
 * return: void
 */
static void update_onvif_config(void)
{
	if (ONVIF_CFG_VERSION == onvif_config.sys->soft_version)
		return;
	
	if ((640 == onvif_config.camera->sub_width) && (360 != onvif_config.camera->sub_height)) {
		ak_print_normal_ex("modify sub height 480 to 360!\n");
		onvif_config.camera->sub_height = 360;
		onvif_config_set_camera(onvif_config.camera);
	}
	
	if  (16 != onvif_config.video->main_smart_goplen) {
		onvif_config.video->main_smart_goplen = 16;
		onvif_config_set_venc_param(onvif_config.video);
	}
	
	if (16 != onvif_config.video->sub_smart_goplen) {
		onvif_config.video->sub_smart_goplen = 16;
		onvif_config_set_venc_param(onvif_config.video);
	}

	onvif_config.sys->soft_version = ONVIF_CFG_VERSION;
	onvif_config_set_sys(onvif_config.sys);
	onvif_config_flush_data();
}


/**
 * onvif_config_get_sys: get system info config
 * return: system info config struct pointer
 */
struct onvif_sys_config* onvif_config_get_sys(void)
{
	return onvif_config.sys;
}

/**
 * onvif_config_set_sys: set system info config
 * @sys[IN]: system info config
 * return: void
 */
void onvif_config_set_sys(const struct onvif_sys_config *sys)
{
	char *title = "global";
    char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

    sprintf(value, "%s", sys->dev_name);
	ak_ini_set_item_value(onvif_config.handle, title, "dev_name", value);
    sprintf(value, "%u", sys->soft_version);
	ak_ini_set_item_value(onvif_config.handle, title, "soft_version", value);
	sprintf(value, "%d", sys->tzone);
	ak_ini_set_item_value(onvif_config.handle, title, "tzone", value);
}

/**
 * onvif_config_get_venc_param: get n1 video param config, title: video
 * @void
 * return: video param config info pointer
 */
struct onvif_venc_config* onvif_config_get_venc_param(void)
{
    return onvif_config.video;
}

/**
 * onvif_config_set_venc_param: set video config param, title: video
 * @video[IN]: video config param
 * return: void
 */
void onvif_config_set_venc_param(const struct onvif_venc_config *video)
{
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

    sprintf(value, "%d", video->main_min_qp_264);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_min_qp_264", value);
	sprintf(value, "%d", video->main_min_qp_265);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_min_qp_265", value);
    sprintf(value, "%d", video->main_max_qp_264);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_max_qp_264", value);
	sprintf(value, "%d", video->main_max_qp_265);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_max_qp_265", value);
	
	sprintf(value, "%d", video->sub_min_qp_264);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_min_qp_264", value);
	sprintf(value, "%d", video->sub_min_qp_265);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_min_qp_265", value);
    sprintf(value, "%d", video->sub_max_qp_264);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_max_qp_264", value);
	sprintf(value, "%d", video->sub_max_qp_265);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_max_qp_265", value);
	
    sprintf(value, "%d", video->main_fps);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_fps", value);
    sprintf(value, "%d", video->main_kbps);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_kbps", value);

    sprintf(value, "%d", video->sub_fps);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_fps", value);
    sprintf(value, "%d", video->sub_kbps);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_kbps", value);

	sprintf(value, "%d", video->main_targetratio_264);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_targetratio_264", value);
	sprintf(value, "%d", video->sub_targetratio_264);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_targetratio_264", value);
	sprintf(value, "%d", video->main_targetratio_265);
	ak_ini_set_item_value(onvif_config.handle, "video", "main_targetratio_265", value);
	sprintf(value, "%d", video->sub_targetratio_265);
	ak_ini_set_item_value(onvif_config.handle, "video", "sub_targetratio_265", value);

	sprintf(value, "%d", video->main_targetratio_264_smart);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_targetratio_264_smart", value);
	sprintf(value, "%d", video->sub_targetratio_264_smart);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_targetratio_264_smart", value);
	sprintf(value, "%d", video->main_targetratio_265_smart);
	ak_ini_set_item_value(onvif_config.handle, "video", "main_targetratio_265_smart", value);
	sprintf(value, "%d", video->sub_targetratio_265_smart);
	ak_ini_set_item_value(onvif_config.handle, "video", "sub_targetratio_265_smart", value);

    sprintf(value, "%d", video->main_goplen);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_goplen", value);
	sprintf(value, "%d", video->sub_goplen);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_goplen", value);
    sprintf(value, "%d", video->main_video_mode);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_video_mode", value);
	sprintf(value, "%d", video->sub_video_mode);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_video_mode", value);
	sprintf(value, "%d", video->main_enc_type);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_enc_type", value);
	sprintf(value, "%d", video->sub_enc_type);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_enc_type", value);
	sprintf(value, "%d", video->main_smart_mode);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_smart_mode", value);
	sprintf(value, "%d", video->sub_smart_mode);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_smart_mode", value);
	sprintf(value, "%d", video->main_smart_goplen);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_smart_goplen", value);
	sprintf(value, "%d", video->sub_smart_goplen);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_smart_goplen", value);
	sprintf(value, "%d", video->main_smart_quality_264);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_smart_quality_264", value);
	sprintf(value, "%d", video->sub_smart_quality_264);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_smart_quality_264", value);
	sprintf(value, "%d", video->main_smart_quality_265);
    ak_ini_set_item_value(onvif_config.handle, "video", "main_smart_quality_265", value);
	sprintf(value, "%d", video->sub_smart_quality_265);
    ak_ini_set_item_value(onvif_config.handle, "video", "sub_smart_quality_265", value);
	sprintf(value, "%d", video->smart_static_value);
    ak_ini_set_item_value(onvif_config.handle, "video", "smart_static_value", value);

}

/**
 * onvif_config_get_net: get n1 network config
 * return: network config struct pointer
 */
struct onvif_net_config* onvif_config_get_net(void)
{
    if(onvif_config.net->dhcp == 1){
        get_onvif_net_info();
    }

    return onvif_config.net;
}

/**
 * onvif_config_set_net: set n1 network config
 * @net[IN]: network config
 * return: void
 */
void onvif_config_set_net(const struct onvif_net_config *net)
{
    char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

	memcpy(onvif_config.net, net, sizeof(struct onvif_net_config));
    sprintf(value,"%d", onvif_config.net->dhcp);
    ak_ini_set_item_value(onvif_config.handle, "ethernet", "dhcp", value);
    sprintf(value,"%s", onvif_config.net->ipaddr);
    ak_ini_set_item_value(onvif_config.handle, "ethernet", "ipaddr", value);
    sprintf(value,"%s", onvif_config.net->netmask);
    ak_ini_set_item_value(onvif_config.handle, "ethernet", "netmask", value);
    sprintf(value,"%s", onvif_config.net->gateway);
    ak_ini_set_item_value(onvif_config.handle, "ethernet", "gateway", value);
    sprintf(value,"%s", onvif_config.net->firstdns);
    ak_ini_set_item_value(onvif_config.handle, "ethernet", "firstdns", value);
    sprintf(value,"%s", onvif_config.net->backdns);
    ak_ini_set_item_value(onvif_config.handle, "ethernet", "backdns", value);
	sprintf(value,"%d", onvif_config.net->ip_adjust_en);
    ak_ini_set_item_value(onvif_config.handle, "ethernet", "ip_adjust_en", value);
	sprintf(value,"%d", onvif_config.net->hikonport);
    ak_ini_set_item_value(onvif_config.handle, "ethernet", "hikonport", value);

}

/**
 * onvif_config_get_wifi: get n1 wifi config
 * return: wifi config struct pointer
 */
struct onvif_wifi_config* onvif_config_get_wifi(void)
{
    return onvif_config.wifi;
}

/**
 * onvif_config_set_wifi: set n1 wifi onvif_config
 * @wifi[IN]: wifi config
 * @save_ssid_f[IN]: save ssid or not
 * return: void
 */
void onvif_config_set_wifi(const struct onvif_wifi_config *wifi, int save_ssid)
{
    char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

    memcpy(onvif_config.wifi, wifi, sizeof(struct onvif_wifi_config));
	/*
	 * if not verify,the ssid save to /tmp/wireless/;
	 * otherwise, save to configure
	 */
	if(save_ssid){
    	sprintf(value, "%s", onvif_config.wifi->ssid);
    	ak_ini_set_item_value(onvif_config.handle, "wireless", "ssid", value);
	}
    sprintf(value,"%s", onvif_config.wifi->passwd);
    ak_ini_set_item_value(onvif_config.handle, "wireless", "password", value);

	ak_ini_flush_data(onvif_config.handle);
}

/**
 * onvif_config_get_camera: get n1 camera config
 * return: camera config struct pointer
 */
struct onvif_camera_config* onvif_config_get_camera(void)
{
    return onvif_config.camera;
}

/**
 * onvif_config_set_camera: set camera config
 * @camera[IN]: camera config
 * return: void
 */
void onvif_config_set_camera(const struct onvif_camera_config *camera)
{
	ak_print_debug("osd_name len : %d\n", strlen(camera->osd_name));

	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

    sprintf(value, "%d", camera->main_width);
    ak_ini_set_item_value(onvif_config.handle, "camera", "main_width", value);
    sprintf(value, "%d", camera->main_height);
    ak_ini_set_item_value(onvif_config.handle, "camera", "main_height", value);
    sprintf(value, "%d", camera->sub_width);
    ak_ini_set_item_value(onvif_config.handle, "camera", "sub_width", value);
    sprintf(value, "%d", camera->sub_height);
    ak_ini_set_item_value(onvif_config.handle, "camera", "sub_height", value);

	sprintf(value, "%d", camera->main_max_width);
    ak_ini_set_item_value(onvif_config.handle, "camera", "main_max_width", value);
    sprintf(value, "%d", camera->main_max_height);
    ak_ini_set_item_value(onvif_config.handle, "camera", "main_max_height", value);
    sprintf(value, "%d", camera->sub_max_width);
    ak_ini_set_item_value(onvif_config.handle, "camera", "sub_max_width", value);
    sprintf(value, "%d", camera->sub_max_height);
    ak_ini_set_item_value(onvif_config.handle, "camera", "sub_max_height", value);

	sprintf(value, "%d", camera->main_osd_size);
    ak_ini_set_item_value(onvif_config.handle, "camera", "main_osd_size", value);
    sprintf(value, "%d", camera->sub_osd_size);
    ak_ini_set_item_value(onvif_config.handle, "camera", "sub_osd_size", value);
    sprintf(value, "%d", camera->name_position);
    ak_ini_set_item_value(onvif_config.handle, "camera", "name_position", value);
	sprintf(value, "%d", camera->time_position);
    ak_ini_set_item_value(onvif_config.handle, "camera", "time_position", value);
	sprintf(value, "%d", camera->rate_position);
    ak_ini_set_item_value(onvif_config.handle, "camera", "rate_position", value);
    sprintf(value, "%d", camera->osd_switch);
    ak_ini_set_item_value(onvif_config.handle, "camera", "osd_switch", value);
    sprintf(value, "%s", camera->osd_name);
    ak_ini_set_item_value(onvif_config.handle, "camera", "osd_name", value);
    sprintf(value, "%d", camera->date_format);
    ak_ini_set_item_value(onvif_config.handle, "camera", "date_format", value);
    sprintf(value, "%d", camera->hour_format);
    ak_ini_set_item_value(onvif_config.handle, "camera", "hour_format", value);
    sprintf(value, "%d", camera->week_format);
    ak_ini_set_item_value(onvif_config.handle, "camera", "week_format", value);
}

/** onvif_config_get_image: get n1 image config, title: image
 * @void
 * return: image config info pointer
 */
struct onvif_image_config* onvif_config_get_image(void)
{
    return onvif_config.image;
}

/**
 * onvif_config_set_image: set n1 image config, title: image
 * @image[IN]: image config
 * return: void
 */
void onvif_config_set_image(const struct onvif_image_config *image)
{
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

    sprintf(value, "%d", image->hue);
    ak_ini_set_item_value(onvif_config.handle, "image", "hue", value);
    sprintf(value, "%d", image->brightness);
    ak_ini_set_item_value(onvif_config.handle, "image", "brightness", value);
    sprintf(value, "%d", image->saturation);
    ak_ini_set_item_value(onvif_config.handle, "image", "saturation", value);
    sprintf(value, "%d", image->contrast);
    ak_ini_set_item_value(onvif_config.handle, "image", "contrast", value);
    sprintf(value, "%d", image->sharp);
    ak_ini_set_item_value(onvif_config.handle, "image", "sharp", value);

    sprintf(value, "%d", image->flip);
    ak_ini_set_item_value(onvif_config.handle, "image", "flip", value);
    sprintf(value, "%d", image->mirror);
    ak_ini_set_item_value(onvif_config.handle, "image", "mirror", value);
    sprintf(value, "%d", image->ircut_mode);
    ak_ini_set_item_value(onvif_config.handle, "image", "ircut_mode", value);
	sprintf(value, "%d", image->force_anti_flicker_flag);
    ak_ini_set_item_value(onvif_config.handle, "image", "force_anti_flicker_flag", value);
    sprintf(value, "%d", image->irled_mode);
    ak_ini_set_item_value(onvif_config.handle, "image", "irled_mode", value);
}

/** onvif_config_get_audio: get n1 audio config, title: audio
 * @void
 * return: audio config info pointer
 */
struct onvif_audio_config* onvif_config_get_audio(void)
{
    return onvif_config.audio;
}

/**
 * onvif_config_set_audio: set audio config, title: audio
 * @audio[IN]: audio config
 * return: void
 */
void onvif_config_set_audio(struct onvif_audio_config *audio)
{
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

    sprintf(value, "%d", audio->ai_source);
    ak_ini_set_item_value(onvif_config.handle, "audio", "ai_source", value);
    sprintf(value, "%d", audio->aenc_type);
    ak_ini_set_item_value(onvif_config.handle, "audio", "aenc_type", value);
    sprintf(value, "%d", audio->ai_enable);
    ak_ini_set_item_value(onvif_config.handle, "audio", "ai_enable", value);
}

/** onvif_config_get_alarm: get n1 alarm config, title: alarm
 * @void
 * return: alarm config info pointer
 */
struct onvif_alarm_config* onvif_config_get_alarm(void)
{
    return onvif_config.alarm;
}

/**
 * onvif_config_set_alarm: set alarm config, title: alarm
 * @audio[IN]: alarm config
 * return: void
 */
void onvif_config_set_alarm(struct onvif_alarm_config *alarm)
{
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};

    sprintf(value, "%d", alarm->md_level_1);
    ak_ini_set_item_value(onvif_config.handle, "alarm", "md_level_1", value);
    sprintf(value, "%d", alarm->md_level_2);
    ak_ini_set_item_value(onvif_config.handle, "alarm", "md_level_2", value);
	sprintf(value, "%d", alarm->md_level_3);
    ak_ini_set_item_value(onvif_config.handle, "alarm", "md_level_3", value);
	sprintf(value, "%d", alarm->md_level_4);
    ak_ini_set_item_value(onvif_config.handle, "alarm", "md_level_4", value);
	sprintf(value, "%d", alarm->md_level_5);
    ak_ini_set_item_value(onvif_config.handle, "alarm", "md_level_5", value);
}

/**
 * onvif_config_init: init N1 config module
 * return: void
 */
void onvif_config_init(void)
{
	onvif_config.handle = ak_ini_init(ONVIF_CONFIG_FILE_NAME);
    if(!onvif_config.handle) {
        ak_print_normal_ex("open onvif config file failed, create it.\n");

        char result[4] = {0};
        ak_cmd_exec("cp /usr/local/factory_cfg.ini /etc/jffs2/onvif_cfg.ini",
            result, sizeof(result));

		onvif_config.handle = ak_ini_init(ONVIF_CONFIG_FILE_NAME);
		if(NULL == onvif_config.handle) {
			ak_print_normal_ex("open onvif config file failed & create failed\n");
			return;
		}
    } else {
		ak_print_normal_ex("onvif config file check ok.\n\n");
	}

    onvif_config.sys = (struct onvif_sys_config *)calloc(1,
    	sizeof(struct onvif_sys_config));
    onvif_config.video = (struct onvif_venc_config *)calloc(1,
    	sizeof(struct onvif_venc_config));
    onvif_config.net = (struct onvif_net_config *)calloc(1,
    	sizeof(struct onvif_net_config));
    onvif_config.wifi = (struct onvif_wifi_config *)calloc(1,
    	sizeof(struct onvif_wifi_config));
    onvif_config.camera = (struct onvif_camera_config *)calloc(1,
    	sizeof(struct onvif_camera_config));
	onvif_config.image = (struct onvif_image_config *)calloc(1,
    	sizeof(struct onvif_image_config));
	onvif_config.audio = (struct onvif_audio_config *)calloc(1,
    	sizeof(struct onvif_audio_config));
	onvif_config.alarm = (struct onvif_alarm_config *)calloc(1,
    	sizeof(struct onvif_alarm_config));

    init_onvif_sys(onvif_config.sys);
	init_onvif_venc_param(onvif_config.video);
#if N1_CONFIG_DUMP_INFO
	ak_ini_dump_config(onvif_config.handle, "ethernet");
#endif
    init_onvif_net(onvif_config.net);
    init_onvif_wifi(onvif_config.wifi);
    init_onvif_camera(onvif_config.camera);
	init_onvif_image(onvif_config.image);
	init_onvif_audio(onvif_config.audio);
	init_onvif_alarm(onvif_config.alarm);

	update_onvif_config();
}

/**
 * onvif_config_flush_data: flush data
 * return: void
 */
void onvif_config_flush_data(void)
{
    if(onvif_config.handle)
		ak_ini_flush_data(onvif_config.handle);
}

/**
 * onvif_config_update_ssid:  set wifi ssid to config file
 * return: 0 -> success; -1 -> fail;
 * notes: wifi ssid configure change by script cmd, we need to
 *          update configure copy in memory.
 *          it is special.
 */
int onvif_config_update_ssid(void)
{
	if(!onvif_config.handle) {
    	ak_print_error_ex("N1 config file not init.\n");
		return AK_FAILED;
	}
	if(!onvif_config.wifi){
		ak_print_error_ex("N1 config.wifi null.\n");
		return AK_FAILED;
	}

    char cmd[128] = {0};
	char *tmp_cfg = "/tmp/tmp_cfg.ini";
	char value[ONVIF_CONFIG_VALUE_BUF_SIZE] = {0};
	int retry_count = 0;

	for(retry_count = 0; retry_count < 4; retry_count++) {
		sprintf(cmd, "cp %s %s", ONVIF_CONFIG_FILE_NAME, tmp_cfg);
		if(ak_cmd_exec(cmd, NULL, 0)){
			ak_print_error_ex("%s fail.\n",cmd);
			return AK_FAILED;
		}

		if(access(tmp_cfg, F_OK)){
	        ak_print_notice_ex("%s not ok.\n",tmp_cfg);
			ak_sleep_ms(2000);
			continue;
	    }

		void *cfg_handle = ak_ini_init(tmp_cfg);
	    if(NULL == cfg_handle) {
	        ak_print_error_ex("ini init %s failed.\n",tmp_cfg);
			return AK_FAILED;
	    }

	    ak_ini_get_item_value(cfg_handle, "wireless", "ssid", value);
	    strcpy(onvif_config.wifi->ssid, value);
		if(ak_ini_destroy(cfg_handle)){
			ak_print_error_ex("ini_destroy failed.\n");
		}

		sprintf(cmd, "rm -f %s",tmp_cfg);
		ak_cmd_exec(cmd, NULL, 0);
		if(strlen(value) > 0){
			break;
		}

		ak_print_normal_ex("retry count:%d\n",retry_count);
		ak_sleep_ms(2000);
	}
	if(4 == retry_count){
		return AK_FAILED;
	}

	ak_ini_set_item_value(onvif_config.handle, "wireless", "ssid", value);
	ak_print_normal_ex("ssid:%s pwd:%s\n",
	    onvif_config.wifi->ssid, onvif_config.wifi->passwd);

	return AK_SUCCESS;
}

/**
 * onvif_config_release: destroy config module
 * return: void
 */
void onvif_config_release(void)
{
    if (onvif_config.sys) {
        free(onvif_config.sys);
        onvif_config.sys = NULL;
    }
    if (onvif_config.video) {
        free(onvif_config.video);
        onvif_config.video = NULL;
    }
    if (onvif_config.net) {
        free(onvif_config.net);
        onvif_config.net = NULL;
    }
    if (onvif_config.wifi) {
        free(onvif_config.wifi);
        onvif_config.wifi = NULL;
    }
    if (onvif_config.camera) {
        free(onvif_config.camera);
        onvif_config.camera = NULL;
    }
    if (onvif_config.image) {
        free(onvif_config.image);
        onvif_config.image = NULL;
    }
	if (onvif_config.audio) {
        free(onvif_config.audio);
        onvif_config.audio = NULL;
    }
	if (onvif_config.alarm) {
        free(onvif_config.alarm);
        onvif_config.alarm = NULL;
    }

	ak_ini_destroy(onvif_config.handle);
}
