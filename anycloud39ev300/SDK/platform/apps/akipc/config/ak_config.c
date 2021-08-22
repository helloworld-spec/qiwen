#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "ak_config.h"
#include "ak_ini.h"
#include "ak_common.h"
#include "ak_net.h"
#include "ak_cmd_exec.h"

#define BUF_SIZE (100)
#define CONFIG_VALUE_BUF_SIZE		200
#define CONFIG_DUMP_INFO			0
#define CONFIG_STR_BUF_SIZE		20


/* default config file */
#define CONFIG_ANYKA_FILE_NAME  	"/etc/jffs2/anyka_cfg.ini"
#define CONFIG_PTZ_UNHIT_NAME  		"/tmp/ak_sys_start_flag"
#define ALARM_PRE_TIME  2
#define ALARM_STOP_TIME 60
#define ALARM_LIMIT_TIME 300

struct ptz_pos {
	int left;
	int up;
};

struct sys_config {
	void *handle;

	struct sys_user_config *user;		/* title: global */
	struct video_config *video;			/* title: video */
	struct video_record_config *record;	/* title: record */
	struct sys_alarm_config *alarm;		/* title: alarm */
	struct sys_cloud_config *cloud;		/* title: cloud */
	struct sys_net_config *net;			/* title: ethernet */
	struct sys_wifi_config *wifi;		/* title: wireless */
	struct camera_disp_config *camera;	/* title: camera */
	struct image_config *image;			/* title: image */
	struct audio_config *audio;			/* title: audio */
	struct auto_day_night_config *auto_day_night;/* title: autoir */
	struct mt_config *mt;				/* title: mt */
};

static struct sys_config config = {NULL};
static const char config_version[] = "libapp_config V1.0.05";

/**
 * ak_config_get_version - get config version
 * return: version string
 */
const char* ak_config_get_version(void)
{
	return config_version;
}

/**
 * day_clock_to_seconds - transfer current day time clock to seconds
 * @clock[IN]: current day time clock
 * return: seconds after transferred
 * notes: clock format: 06:45:20
 */
static time_t day_clock_to_seconds(const char *clock)
{
	int colon_num = 0;
	char *tmp = NULL;
	char *min_tmp = NULL;
	char *sec_tmp = NULL;

	tmp = strchr(clock, ':');
	while(NULL != tmp){
		++colon_num;

		++tmp;
		tmp = strchr(tmp, ':');
	}
	if(colon_num < 2){
		return 0;
	}

	min_tmp = strchr(clock, ':');
	*min_tmp = 0x00;
	++min_tmp;
	int hour = atoi(clock);
	if(hour >= 24){
		hour = 23;
	}

	sec_tmp = strchr(min_tmp, ':');
	*sec_tmp = 0x00;
	++sec_tmp;
	int min = atoi(min_tmp);
	int sec = atoi(sec_tmp);
	if(min >= 60){
		min = 59;
	}
	if(sec >= 60){
		sec = 59;
	}

	return ((hour*60*60) + (min*60) + sec);
}

/**
 * day_second_to_string - transfer current day time clock to string
 * @clock[OUT]: current day time clock hh:mm:ss
 * @second[IN]: second . since from 00h:00minutes:00second
 * return: void
 * notes: clock format: 06:45:20
 */
static void day_second_to_string(char *clock,time_t second)
{
	memset(clock, 0, 20);
	sprintf(clock,"%2d:%2d:%2d",(int)second/(60*60),(int)(second %(60*60))/60,
		(int) second % 60);
}

/** save_week_config_info: enable appointed weed day
 * @day_index: appointed day index
 * @week[OUT]: week day info
 * return: none
 */
static void enable_week_day(int day_index, int *week)
{
	if(day_index >= DAY_PER_WEEK){
		week[0] = 0x01;
	}else{
		week[day_index] = 0x01;
	}
}

/** enable_week_day: transfer string value to week day value
 * @value: week day string
 * @week[OUT]: week day info
 * return: none
 */
static void save_week_config_info(char *value, int *week)
{
	int i = 0;
	int index = 0;
	int str_len = strlen(value);

	for(i=0; i<str_len; ++i){
		switch(value[i]){
		case '-':
			if(0x00 == i){
				ak_print_normal_ex("week active config INVALID\n");
			}else{
				int start_day = atoi(&value[i-1]);	//we reset start day
				int end_day = atoi(&value[i+1]);

 				if(start_day < 0x01){
					start_day = 0x01;
				}
				if(end_day > 0x07){
					end_day = 0x07;
				}

				/* including end_day, ex: 1-7 is Monday to Sunday */
				for(index=start_day; index<=end_day; ++index){
					enable_week_day(index, week);
				}

				++i;//skip number after '-'
			}
			break;
		case ',':	//just skip ','
			break;
		default:
			/* single week day */
			index = atoi(&value[i]);
			enable_week_day(index, week);
			break;
		}
	}
}

/** get_week_config_info_str: transfer week day value to string
 * @value[OUT]: week day string
 * @week[IN]: week day info
 * return: none
 */
static void get_week_config_info_str(char *value, int *week)
{
	int i = 0;
	int day_index = 0;
	int first_flag = 0;
	int left_flag = 0;
	char tmp[20];

	memset(tmp,0,20);
	memset(value,0,20);
	for(i = 1; i< 8; ++i){
		day_index = i % 7;
		if(1 == week[day_index]){
			if(0 == first_flag){
				snprintf(value,20,"%1d",i);
				memcpy(tmp,value,20);
				left_flag = 1;
				first_flag = 1;
			}
			else{
				if(left_flag){
					if((i < 7) && (1 == week[(i +1) % 7]))
						continue;
					snprintf(value,20,"%s-%1d",tmp,i);
					memcpy(tmp,value,20);
					left_flag = 0;
				}else
				{
					snprintf(value,20,"%s,%1d",tmp,i);
					memcpy(tmp,value,20);
					left_flag = 1;
				}
			}

		}
		else{
			left_flag = 0;
		}
	}
	ak_print_normal_ex("%s\n",value);

}

/** ak_config_get_sys_video: get system video config, title: video
 * @void
 * return: video config info pointer
 */
struct video_config* ak_config_get_sys_video(void)
{
	return config.video;
}

/**
 * ak_config_set_sys_video: set video config, title: video
 * @video[IN]: video config
 * return: void
 */
void ak_config_set_sys_video(struct video_config *video)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	/* basic encode configurations */
	sprintf(value, "%d", video->main_min_qp);
	ak_ini_set_item_value(config.handle, "video", "main_min_qp", value);
	sprintf(value, "%d", video->sub_min_qp);
	ak_ini_set_item_value(config.handle, "video", "sub_min_qp", value);

	sprintf(value, "%d", video->main_max_qp);
	ak_ini_set_item_value(config.handle, "video", "main_max_qp", value);
	sprintf(value, "%d", video->sub_max_qp);
	ak_ini_set_item_value(config.handle, "video", "sub_max_qp", value);

	sprintf(value, "%d", video->main_fps);
	ak_ini_set_item_value(config.handle, "video", "main_fps", value);
	sprintf(value, "%d", video->main_min_kbps);
	ak_ini_set_item_value(config.handle, "video", "main_min_kbps", value);
	sprintf(value, "%d", video->main_max_kbps);
	ak_ini_set_item_value(config.handle, "video", "main_max_kbps", value);

	sprintf(value, "%d", video->sub_fps);
	ak_ini_set_item_value(config.handle, "video", "sub_fps", value);
	sprintf(value, "%d", video->sub_min_kbps);
	ak_ini_set_item_value(config.handle, "video", "sub_min_kbps", value);
	sprintf(value, "%d", video->sub_max_kbps);
	ak_ini_set_item_value(config.handle, "video", "sub_max_kbps", value);

	sprintf(value, "%d", video->main_gop_len);
	ak_ini_set_item_value(config.handle, "video", "main_gop_len", value);
	sprintf(value, "%d", video->sub_gop_len);
	ak_ini_set_item_value(config.handle, "video", "sub_gop_len", value);

	sprintf(value, "%d", video->main_video_mode);
	ak_ini_set_item_value(config.handle, "video", "main_video_mode", value);
	sprintf(value, "%d", video->sub_video_mode);
	ak_ini_set_item_value(config.handle, "video", "sub_video_mode", value);
	sprintf(value, "%d", video->main_enc_type);
	ak_ini_set_item_value(config.handle, "video", "main_enc_type", value);
	sprintf(value, "%d", video->sub_enc_type);
	ak_ini_set_item_value(config.handle, "video", "sub_enc_type", value);

	/* basic bitrate ratios */
	sprintf(value, "%d", video->main_target_ratio_264);
	ak_ini_set_item_value(config.handle, "video", "main_target_ratio_264", value);
	sprintf(value, "%d", video->sub_target_ratio_264);
	ak_ini_set_item_value(config.handle, "video", "sub_target_ratio_264", value);
	sprintf(value, "%d", video->main_target_ratio_265);
	ak_ini_set_item_value(config.handle, "video", "main_target_ratio_265", value);
	sprintf(value, "%d", video->sub_target_ratio_265);
	ak_ini_set_item_value(config.handle, "video", "sub_target_ratio_265", value);

	/* smart encode model configurations */
	sprintf(value, "%d", video->main_smart_mode);
	ak_ini_set_item_value(config.handle, "video", "main_smart_mode", value);
	sprintf(value, "%d", video->sub_smart_mode);
	ak_ini_set_item_value(config.handle, "video", "sub_smart_mode", value);

	/* smart model goplen */
	sprintf(value, "%d", video->main_smart_goplen);
	ak_ini_set_item_value(config.handle, "video", "main_smart_goplen", value);
	sprintf(value, "%d", video->sub_smart_goplen);
	ak_ini_set_item_value(config.handle, "video", "sub_smart_goplen", value);

	/* smart model quality */
	sprintf(value, "%d", video->main_smart_quality_264);
	ak_ini_set_item_value(config.handle, "video", "main_smart_quality_264", value);
	sprintf(value, "%d", video->sub_smart_quality_264);
	ak_ini_set_item_value(config.handle, "video", "sub_smart_quality_264", value);

	sprintf(value, "%d", video->main_smart_quality_265);
	ak_ini_set_item_value(config.handle, "video", "main_smart_quality_265", value);
	sprintf(value, "%d", video->sub_smart_quality_265);
	ak_ini_set_item_value(config.handle, "video", "sub_smart_quality_265", value);

	/* smart model target ratio */
	sprintf(value, "%d", video->main_smart_target_ratio_264);
	ak_ini_set_item_value(config.handle, "video", "main_smart_target_ratio_264", value);
	sprintf(value, "%d", video->sub_smart_target_ratio_264);
	ak_ini_set_item_value(config.handle, "video", "sub_smart_target_ratio_264", value);
	sprintf(value, "%d", video->main_smart_target_ratio_265);
	ak_ini_set_item_value(config.handle, "video", "main_smart_target_ratio_265", value);
	sprintf(value, "%d", video->sub_smart_target_ratio_265);
	ak_ini_set_item_value(config.handle, "video", "sub_smart_target_ratio_265", value);

	sprintf(value, "%d", video->main_smart_static_value);
	ak_ini_set_item_value(config.handle, "video", "main_smart_static_value", value);
	sprintf(value, "%d", video->sub_smart_static_value);
	ak_ini_set_item_value(config.handle, "video", "sub_smart_static_value", value);

}


/**
 * ak_config_get_record: get record config, title: record
 * return: record config struct pointer
 */
struct video_record_config *ak_config_get_record(void)
{
	return config.record;
}

static void init_video_param(struct video_config *video)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	/* get all item values of video title */
	ak_ini_get_item_value(config.handle, "video", "main_min_qp", value);
	video->main_min_qp = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "main_max_qp", value);
	video->main_max_qp = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "main_fps", value);
	video->main_fps = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "main_min_kbps", value);
	video->main_min_kbps = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "main_max_kbps", value);
	video->main_max_kbps = atoi(value);

	ak_ini_get_item_value(config.handle, "video", "sub_min_qp", value);
	video->sub_min_qp = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_max_qp", value);
	video->sub_max_qp = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_fps", value);
	video->sub_fps = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_min_kbps", value);
	video->sub_min_kbps = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_max_kbps", value);
	video->sub_max_kbps = atoi(value);

	ak_ini_get_item_value(config.handle, "video", "main_gop_len", value);
	video->main_gop_len = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_gop_len", value);
	video->sub_gop_len = atoi(value);

	/* video RC mode and video type (264/mjpeg/265) */
	ak_ini_get_item_value(config.handle, "video", "main_video_mode", value);
	video->main_video_mode = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_video_mode", value);
	video->sub_video_mode = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "main_enc_type", value);
	video->main_enc_type = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_enc_type", value);
	video->sub_enc_type = atoi(value);

	/* bps ratio */
	ak_ini_get_item_value(config.handle, "video", "main_target_ratio_264", value);
	video->main_target_ratio_264 = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_target_ratio_264", value);
	video->sub_target_ratio_264 = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "main_target_ratio_265", value);
	video->main_target_ratio_265 = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_target_ratio_265", value);
	video->sub_target_ratio_265 = atoi(value);

	/* smart config */
	ak_ini_get_item_value(config.handle, "video", "main_smart_goplen", value);
	video->main_smart_goplen = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_smart_goplen", value);
	video->sub_smart_goplen = atoi(value);

	ak_ini_get_item_value(config.handle, "video", "main_smart_mode", value);
	video->main_smart_mode = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_smart_mode", value);
	video->sub_smart_mode = atoi(value);

	ak_ini_get_item_value(config.handle, "video", "main_smart_quality_264", value);
	video->main_smart_quality_264 = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_smart_quality_264", value);
	video->sub_smart_quality_264 = atoi(value);

	ak_ini_get_item_value(config.handle, "video", "main_smart_quality_265", value);
	video->main_smart_quality_265 = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_smart_quality_265", value);
	video->sub_smart_quality_265 = atoi(value);

	ak_ini_get_item_value(config.handle, "video", "main_smart_target_ratio_264", value);
	video->main_smart_target_ratio_264 = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_smart_target_ratio_264", value);
	video->sub_smart_target_ratio_264 = atoi(value);


	ak_ini_get_item_value(config.handle, "video", "main_smart_target_ratio_265", value);
	video->main_smart_target_ratio_265 = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_smart_target_ratio_265", value);
	video->sub_smart_target_ratio_265 = atoi(value);

	ak_ini_get_item_value(config.handle, "video", "main_smart_static_value", value);
	video->main_smart_static_value = atoi(value);
	ak_ini_get_item_value(config.handle, "video", "sub_smart_static_value", value);
	video->sub_smart_static_value = atoi(value);
}

static void init_sys_alarm(struct sys_alarm_config *alarm)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	ak_ini_get_item_value(config.handle, "alarm", "md_set", value);
	alarm->md_set = atoi(value);
	ak_ini_get_item_value(config.handle, "alarm", "md_level_1", value);
	alarm->md_level_1 = atoi(value);
	ak_ini_get_item_value(config.handle, "alarm", "md_level_2", value);
	alarm->md_level_2 = atoi(value);
	ak_ini_get_item_value(config.handle, "alarm", "md_level_3", value);
	alarm->md_level_3 = atoi(value);

	ak_ini_get_item_value(config.handle, "alarm", "sd_set", value);
	alarm->sd_set = atoi(value);
	ak_ini_get_item_value(config.handle, "alarm", "sd_level_1", value);
	alarm->sd_level_1 = atoi(value);
	ak_ini_get_item_value(config.handle, "alarm", "sd_level_2", value);
	alarm->sd_level_2 = atoi(value);
	ak_ini_get_item_value(config.handle, "alarm", "sd_level_3", value);
	alarm->sd_level_3 = atoi(value);

	ak_ini_get_item_value(config.handle, "alarm", "i2o_detection", value);
	alarm->i2o_detection = atoi(value);
	ak_ini_get_item_value(config.handle, "alarm", "smoke_detection", value);
	alarm->smoke_detection = atoi(value);
	ak_ini_get_item_value(config.handle, "alarm", "shadow_detection", value);
	alarm->shadow_detection = atoi(value);
	ak_ini_get_item_value(config.handle, "alarm", "other_detection", value);
	alarm->other_detection = atoi(value);

	ak_ini_get_item_value(config.handle, "alarm", "alarm_send_type", value);
	alarm->send_type = atoi(value);
	ak_ini_get_item_value(config.handle, "alarm", "alarm_interval_time", value);
	alarm->interval_time = atoi(value);
	ak_ini_get_item_value(config.handle, "alarm", "alarm_send_msg_time", value);
	alarm->send_msg_time = atoi(value);
}

static void init_sys_net(struct sys_net_config *net)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	ak_ini_get_item_value(config.handle, "ethernet", "dhcp", value);
	net->dhcp = atoi(value);
	ak_ini_get_item_value(config.handle, "ethernet", "ipaddr", value);
	strcpy(net->ipaddr, value);
	ak_ini_get_item_value(config.handle, "ethernet", "netmask", value);
	strcpy(net->netmask, value);
	ak_ini_get_item_value(config.handle, "ethernet", "gateway", value);
	strcpy(net->gateway, value);
	ak_ini_get_item_value(config.handle, "ethernet", "firstdns", value);
	strcpy(net->firstdns, value);
	ak_ini_get_item_value(config.handle, "ethernet", "backdns", value);
	strcpy(net->backdns, value);
	ak_ini_get_item_value(config.handle, "ethernet", "ip_adjust_en", value);
	net->ip_adjust_en = atoi(value);
}

static void init_image(struct image_config *image)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};
	int ret = 0;

	ret = ak_ini_get_item_value(config.handle, "image", "flip", value);
	if (0 == ret)
		image->flip = atoi(value);
	ret = ak_ini_get_item_value(config.handle, "image", "mirror", value);
	if (0 == ret)
		image->mirror = atoi(value);
	ak_ini_get_item_value(config.handle, "image", "irled_mode", value);
	image->irled_mode = atoi(value);
}

static void init_audio(struct audio_config *audio)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	ak_ini_get_item_value(config.handle, "audio", "ai_source", value);
	audio->ai_source = atoi(value);
}

static void init_mt(struct mt_config *mt)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	ak_ini_get_item_value(config.handle, "mt", "mt_en", value);
	mt->mt_en = atoi(value);
	ak_ini_get_item_value(config.handle, "mt", "calibrate_en", value);
	mt->calibrate_en = atoi(value);
	ak_ini_get_item_value(config.handle, "mt", "wait_time", value);
	mt->wait_time = atoi(value);
	ak_ini_get_item_value(config.handle, "mt", "draw_box_en", value);
	mt->draw_box_en = atoi(value);
	ak_ini_get_item_value(config.handle, "mt", "switch_v", value);
	mt->switch_v = atoi(value);

	ak_ini_get_item_value(config.handle, "mt", "flt_big_day", value);
	mt->flt_big_day = atoi(value);
	ak_ini_get_item_value(config.handle, "mt", "flt_big_night", value);
	mt->flt_big_night = atoi(value);
	ak_ini_get_item_value(config.handle, "mt", "flt_small_day", value);
	mt->flt_small_day = atoi(value);
	ak_ini_get_item_value(config.handle, "mt", "flt_small_night", value);
	mt->flt_small_night = atoi(value);
	ak_ini_get_item_value(config.handle, "mt", "valid_size_min", value);
	mt->valid_size_min = atoi(value);
	ak_ini_get_item_value(config.handle, "mt", "valid_size_max", value);
	mt->valid_size_max = atoi(value);
}


static void get_net_info(void)
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
		memset(config.net->ipaddr, 0, sizeof(config.net->ipaddr));
		strcpy(config.net->ipaddr, ip);
	}

	/* netmask */
	if (!ak_net_get_netmask(iface, netmask)) {
		memset(config.net->netmask, 0, sizeof(config.net->netmask));
		strcpy(config.net->netmask, netmask);
	}

	/* route */
	if (!ak_net_get_route(iface, route)) {
		memset(config.net->gateway, 0, sizeof(config.net->gateway));
		strcpy(config.net->gateway, route);
	}

	/* first dns */
	if (ak_net_get_dns(0, mdns)) {
		strcpy(mdns, route);
	} else {
		memset(config.net->firstdns, 0, sizeof(config.net->firstdns));
		strcpy(config.net->firstdns, mdns);
	}

	/* back dns */
	if (ak_net_get_dns(1, sdns)) {
		strcpy(sdns, mdns);
	} else {
		memset(config.net->backdns, 0, sizeof(config.net->backdns));
		strcpy(config.net->backdns, sdns);
	}
}

/**
 * ak_config_get_sys_net: get system network config
 * return: network config struct pointer
 */
struct sys_net_config* ak_config_get_sys_net(void)
{
	if(config.net->dhcp == 1){
		get_net_info();
	}

	return config.net;
}

/**
 * ak_config_set_sys_net: set system network config
 * @net[IN]: network config
 * return: void
 */
void ak_config_set_sys_net(struct sys_net_config *net)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	memcpy(config.net, net, sizeof(struct sys_net_config));
	sprintf(value,"%d", config.net->dhcp);
	ak_ini_set_item_value(config.handle, "ethernet", "dhcp", value);
	sprintf(value,"%s", config.net->ipaddr);
	ak_ini_set_item_value(config.handle, "ethernet", "ipaddr", value);
	sprintf(value,"%s", config.net->netmask);
	ak_ini_set_item_value(config.handle, "ethernet", "netmask", value);
	sprintf(value,"%s", config.net->gateway);
	ak_ini_set_item_value(config.handle, "ethernet", "gateway", value);
	sprintf(value,"%s", config.net->firstdns);
	ak_ini_set_item_value(config.handle, "ethernet", "firstdns", value);
	sprintf(value,"%s", config.net->backdns);
	ak_ini_set_item_value(config.handle, "ethernet", "backdns", value);
	sprintf(value,"%d", config.net->ip_adjust_en);
	ak_ini_set_item_value(config.handle, "ethernet", "ip_adjust_en", value);
}

static void init_sys_wifi(struct sys_wifi_config * net)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	ak_ini_get_item_value(config.handle, "wireless", "ssid", value);
	strcpy(net->ssid, value);
	ak_ini_get_item_value(config.handle, "wireless", "mode", value);
	strcpy(net->mode, value);
	ak_ini_get_item_value(config.handle, "wireless", "password", value);
	strcpy(net->passwd, value);
	ak_ini_get_item_value(config.handle, "wireless", "security", value);
	if(strstr(value, "WPA")){
		net->enc_type = WIFI_ENCTYPE_WPA2_TKIP;
	}else if(strstr(value, "WEP")){
		net->enc_type = WIFI_ENCTYPE_WEP;
	}else{
		net->enc_type = WIFI_ENCTYPE_NONE;
	}

	ak_ini_get_item_value(config.handle, "softap", "s_ssid", value);
	strcpy(net->ap_ssid, value);
	ak_ini_get_item_value(config.handle, "softap", "s_password", value);
	strcpy(net->ap_pswd, value);
}

/**
 * ak_config_get_sys_wifi: get system wifi config
 * return: wifi config struct pointer
 */
struct sys_wifi_config* ak_config_get_sys_wifi(void)
{
	return config.wifi;
}

/**
 * ak_config_set_sys_wifi: set system wifi config
 * @wifi[IN]: wifi config
 * @save_ssid_f[IN]: save ssid or not
 * return: void
 */
void ak_config_set_sys_wifi(struct sys_wifi_config *wifi, int save_ssid_f)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	memcpy(config.wifi, wifi, sizeof(struct sys_wifi_config));
	/*
	 * if not verify,the ssid save to /tmp/wireless/;
	 * otherwise, save to configure
	 */
	if(save_ssid_f){
		sprintf(value,"%s", config.wifi->ssid);
		ak_ini_set_item_value(config.handle, "wireless", "ssid", value);
	}
	sprintf(value,"%s", config.wifi->passwd);
	ak_ini_set_item_value(config.handle, "wireless", "password", value);

	if (wifi->ap_ssid) {
		sprintf(value,"%s", config.wifi->ap_ssid);
		ak_ini_set_item_value(config.handle, "softap", "s_ssid", value);
	}
	if (wifi->ap_pswd) {
		sprintf(value,"%s", config.wifi->ap_pswd);
		ak_ini_set_item_value(config.handle, "softap", "s_password", value);
	}
	ak_ini_flush_data(config.handle);
}

static void init_sys_cloud_config(struct sys_cloud_config *cloud)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	ak_ini_get_item_value(config.handle, "cloud", "dana", value);
	cloud->dana = atoi(value);

	ak_ini_get_item_value(config.handle, "cloud", "rtsp", value);
	cloud->rtsp = atoi(value);
}

/**
 * ak_config_get_sys_cloud: get system cloud config
 * return: cloud config struct pointer
 */
struct sys_cloud_config* ak_config_get_sys_cloud(void)
{
	return config.cloud;
}

/**
 * ak_config_get_sys_alarm: get system alarm config
 * return: system alarm config struct pointer
 */
struct sys_alarm_config* ak_config_get_sys_alarm(void)
{
	return config.alarm;
}

/**
 * ak_config_set_sys_alarm: set system alarm config
 * return: void
 */
void ak_config_set_sys_alarm(void)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	sprintf(value,"%d", config.alarm->md_set);
	ak_ini_set_item_value(config.handle, "alarm", "md_set", value);
	sprintf(value,"%d", config.alarm->md_level_1 );
	ak_ini_set_item_value(config.handle, "alarm", "md_level_1", value);
	sprintf(value,"%d", config.alarm->md_level_2 );
	ak_ini_set_item_value(config.handle, "alarm", "md_level_2", value);
	sprintf(value,"%d", config.alarm->md_level_3 );
	ak_ini_set_item_value(config.handle, "alarm", "md_level_3", value);

	sprintf(value,"%d", config.alarm->sd_set);
	ak_ini_set_item_value(config.handle, "alarm", "sd_set", value);
	sprintf(value,"%d", config.alarm->sd_level_1 );
	ak_ini_set_item_value(config.handle, "alarm", "sd_level_1", value);
	sprintf(value,"%d", config.alarm->sd_level_2 );
	ak_ini_set_item_value(config.handle, "alarm", "sd_level_2", value);
	sprintf(value,"%d", config.alarm->sd_level_3 );
	ak_ini_set_item_value(config.handle, "alarm", "sd_level_3", value);

	sprintf(value,"%d", config.alarm->i2o_detection );
	ak_ini_set_item_value(config.handle, "alarm", "i2o_detection", value);
	sprintf(value,"%d", config.alarm->shadow_detection );
	ak_ini_set_item_value(config.handle, "alarm", "shadow_detection", value);
	sprintf(value,"%d", config.alarm->shadow_detection );
	ak_ini_set_item_value(config.handle, "alarm", "shadow_detection", value);
	sprintf(value,"%d", config.alarm->other_detection );
	ak_ini_set_item_value(config.handle, "alarm", "other_detection", value);
	sprintf(value,"%d", config.alarm->send_type );
	ak_ini_set_item_value(config.handle, "alarm", "alarm_send_type", value);
}

static void init_record(struct video_record_config *record)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	ak_ini_get_item_value(config.handle, "record", "record_server", value);
	record->server_flag = atoi(value);
	ak_ini_get_item_value(config.handle, "record", "record_mode", value);
	record->record_mode = atoi(value);
	ak_ini_get_item_value(config.handle, "record", "record_time", value);
	record->duration = atoi(value);
	ak_ini_get_item_value(config.handle, "record", "record_path", value);
	strcpy(record->path, value);
	ak_ini_get_item_value(config.handle, "record", "record_prefix", value);
	strcpy(record->prefix, value);

	ak_ini_get_item_value(config.handle, "record", "sample_rate", value);
	record->sample_rate = atoi(value);
	ak_ini_get_item_value(config.handle, "record", "save_cyc_flag", value);
	record->save_cyc_flag = atoi(value);
	ak_ini_get_item_value(config.handle, "record", "save_file_fps", value);
	record->save_file_fps = atoi(value);
	ak_ini_get_item_value(config.handle, "record", "save_file_kbps", value);
	record->save_file_kbps = atoi(value);
	ak_ini_get_item_value(config.handle, "record", "min_file_fps", value);
	record->min_file_fps = atoi(value);
	ak_ini_get_item_value(config.handle, "record", "file_type", value);
	record->file_type = atoi(value);
	ak_ini_get_item_value(config.handle, "record", "audio_enc_type", value);
	record->audio_type = atoi(value);

	int i = 0;
	char tmp[50] = {0};

	for (i=0; i<MAX_RECORD_PLAN_NUM; ++i) {
		sprintf(tmp, "plan%d_week_active", (i+1));
		ak_ini_get_item_value(config.handle, "record", tmp, value);
		memset(record->plan[i].week_enalbe, 0x00, sizeof(record->plan[i].week_enalbe));
		save_week_config_info(value, record->plan[i].week_enalbe);

		sprintf(tmp, "plan%d_run", (i+1));
		ak_ini_get_item_value(config.handle, "record", tmp, value);
		record->plan[i].active = atoi(value);

		sprintf(tmp, "plan%d_start_time", (i+1));
		ak_ini_get_item_value(config.handle, "record", tmp, value);
		record->plan[i].start_time = day_clock_to_seconds(value);

		sprintf(tmp, "plan%d_end_time", (i+1));
		ak_ini_get_item_value(config.handle, "record", tmp, value);
		record->plan[i].end_time = day_clock_to_seconds(value);
	}

	if ( ( ak_ini_get_item_value(config.handle, "record", "alarm_pre_time", value) < 0 ) ||
	     ( ( record->alarm_pre_time = atoi(value) ) == 0 ) ) {
		record->alarm_pre_time = ALARM_PRE_TIME;
	}

	if ( ( ak_ini_get_item_value(config.handle, "record", "alarm_stop_time", value) < 0 ) ||
	     ( ( record->alarm_stop_time = atoi(value) ) == 0 ) ) {
		record->alarm_stop_time = ALARM_STOP_TIME;
	}

	if ( ( ak_ini_get_item_value(config.handle, "record", "alarm_limit_time", value) < 0 ) ||
	     ( ( record->alarm_limit_time = atoi(value) ) == 0 ) ) {
		record->alarm_limit_time = ALARM_LIMIT_TIME;
	}
}

/**
 * ak_config_set_record_plan: set record config
 * @record[IN]: record config
 * return: void
 */
void ak_config_set_record_plan(struct video_record_config *record)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	sprintf(value, "%d", record->plan[0].active);
	ak_ini_set_item_value(config.handle, "record", "plan1_run", value);
	day_second_to_string(value, record->plan[0].start_time);
	ak_ini_set_item_value(config.handle, "record", "plan1_start_time", value);
	day_second_to_string(value, record->plan[0].end_time);
	ak_ini_set_item_value(config.handle, "record", "plan1_end_time", value);

	get_week_config_info_str(value,record->plan[0].week_enalbe);
	ak_ini_set_item_value(config.handle, "record", "plan1_week_active", value);
	sprintf(value, "%d", record->plan[1].active);
	ak_ini_set_item_value(config.handle, "record", "plan2_run", value);
	day_second_to_string(value, record->plan[1].start_time);
	ak_ini_set_item_value(config.handle, "record", "plan2_start_time", value);
	day_second_to_string(value, record->plan[1].end_time);
	ak_ini_set_item_value(config.handle, "record", "plan2_end_time", value);
	get_week_config_info_str(value,record->plan[1].week_enalbe);
	ak_ini_set_item_value(config.handle, "record", "plan2_week_active", value);
	sprintf(value, "%d", record->plan[2].active);
	ak_ini_set_item_value(config.handle, "record", "plan3_run", value);
	day_second_to_string(value, record->plan[2].start_time);
	ak_ini_set_item_value(config.handle, "record", "plan3_start_time", value);
	day_second_to_string(value, record->plan[2].end_time);
	ak_ini_set_item_value(config.handle, "record", "plan3_end_time", value);
	get_week_config_info_str(value,record->plan[2].week_enalbe);
	ak_ini_set_item_value(config.handle, "record", "plan3_week_active", value);
}

static void init_config_auto_day_night(struct auto_day_night_config
										*auto_day_night)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

    ak_ini_get_item_value(config.handle, "autoir", "auto_day_night_enable", value);
    auto_day_night->auto_day_night_enable = atoi(value);

    ak_ini_get_item_value(config.handle, "autoir", "day_night_mode", value);
    auto_day_night->day_night_mode = atoi(value);

    ak_ini_get_item_value(config.handle, "autoir", "day_to_night_lum", value);
    auto_day_night->day_to_night_lum = atoi(value);

    ak_ini_get_item_value(config.handle, "autoir", "night_to_day_lum", value);
    auto_day_night->night_to_day_lum = atoi(value);

    ak_ini_get_item_value(config.handle, "autoir", "lock_time", value);
    auto_day_night->lock_time = atoi(value);

	ak_ini_get_item_value(config.handle, "autoir", "quick_switch_mode", value);
    auto_day_night->quick_switch_mode = atoi(value);


	// day
	char autoir[CONFIG_STR_BUF_SIZE] = {0};
	int i = 0;
	for (i = 0; i < NIGHT_ARR_NUM; i++) {
		memset(autoir, 0, CONFIG_STR_BUF_SIZE);
		sprintf(autoir, "%s%d", "night_cnt", i);
		ak_ini_get_item_value(config.handle, "autoir", autoir, value);
		auto_day_night->night_cnt[i] = atoi(value);
	}

	// night
	for (i = 0; i < DAY_ARR_NUM; i++) {
		memset(autoir, 0, CONFIG_STR_BUF_SIZE);
		sprintf(autoir, "%s%d", "day_cnt", i);
		ak_ini_get_item_value(config.handle, "autoir", autoir, value);
		auto_day_night->day_cnt[i] = atoi(value);
	}


}

/**
 * ak_config_set_auto_day_night: set auto day night switch config
 * @auto_day_night[IN]: auto day night switch config
 * return: void
 */
void ak_config_set_auto_day_night(struct auto_day_night_config
								*auto_day_night)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

    sprintf(value, "%d", auto_day_night->auto_day_night_enable);
    ak_ini_set_item_value(config.handle, "autoir", "auto_day_night_enable", value);

    sprintf(value, "%d", auto_day_night->day_night_mode);
    ak_ini_set_item_value(config.handle, "autoir", "day_night_mode", value);

    sprintf(value, "%d", auto_day_night->day_to_night_lum);
    ak_ini_set_item_value(config.handle, "autoir", "day_to_night_lum", value);

    sprintf(value, "%d", auto_day_night->night_to_day_lum);
    ak_ini_set_item_value(config.handle, "autoir", "night_to_day_lum", value);

	sprintf(value, "%d", auto_day_night->lock_time);
    ak_ini_set_item_value(config.handle, "autoir", "lock_time", value);

	sprintf(value, "%d", auto_day_night->quick_switch_mode);
    ak_ini_set_item_value(config.handle, "autoir", "quick_switch_mode", value);

	// day
	char autoir[CONFIG_STR_BUF_SIZE] = {0};
	int i = 0;
	for (i = 0; i < NIGHT_ARR_NUM; i++) {
		memset(autoir, 0, CONFIG_STR_BUF_SIZE);
		sprintf(autoir, "%s%d", "night_cnt", i);
		sprintf(value, "%d", auto_day_night->night_cnt[i]);
    	ak_ini_set_item_value(config.handle, "autoir", autoir, value);
	}

	// night
	for (i = 0; i < DAY_ARR_NUM; i++) {
		memset(autoir, 0, CONFIG_STR_BUF_SIZE);
		sprintf(autoir, "%s%d", "day_cnt", i);
		sprintf(value, "%d", auto_day_night->day_cnt[i]);
    	ak_ini_set_item_value(config.handle, "autoir", autoir, value);
	}
}

/**
 * ak_config_get_auto_day_night: get auto day night switch config
 * return: auto day night config struct pointer
 */
struct auto_day_night_config* ak_config_get_auto_day_night(void)
{
	return config.auto_day_night;
}

static void init_camera_config(struct camera_disp_config * camera)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	ak_ini_get_item_value(config.handle, "camera", "main_width", value);
	camera->main_width = atoi(value);
	ak_ini_get_item_value(config.handle, "camera", "main_height", value);
	camera->main_height = atoi(value);
	ak_ini_get_item_value(config.handle, "camera", "sub_width", value);
	camera->sub_width = atoi(value);
	ak_ini_get_item_value(config.handle, "camera", "sub_height", value);
	camera->sub_height = atoi(value);

	ak_ini_get_item_value(config.handle, "camera", "main_max_width", value);
    camera->main_max_width = atoi(value);
    ak_ini_get_item_value(config.handle, "camera", "main_max_height", value);
    camera->main_max_height = atoi(value);
    ak_ini_get_item_value(config.handle, "camera", "sub_max_width", value);
    camera->sub_max_width = atoi(value);
    ak_ini_get_item_value(config.handle, "camera", "sub_max_height", value);
    camera->sub_max_height = atoi(value);

	ak_ini_get_item_value(config.handle, "camera", "main_osd_size", value);
	camera->main_osd_size = atoi(value);
	ak_ini_get_item_value(config.handle, "camera", "sub_osd_size", value);
	camera->sub_osd_size = atoi(value);
	ak_ini_get_item_value(config.handle, "camera", "osd_switch", value);
	camera->osd_switch = atoi(value);
	ak_ini_get_item_value(config.handle, "camera", "osd_name", value);
	strcpy(camera->osd_name, value);
	ak_ini_get_item_value(config.handle, "camera", "rate_position", value);
	camera->rate_position = atoi(value);
	ak_ini_get_item_value(config.handle, "camera", "name_position", value);
	camera->name_position = atoi(value);
	ak_ini_get_item_value(config.handle, "camera", "time_position", value);
	camera->time_position = atoi(value);
	ak_ini_get_item_value(config.handle, "camera", "date_format", value);
	camera->date_format = atoi(value);
	ak_ini_get_item_value(config.handle, "camera", "hour_format", value);
	camera->hour_format = atoi(value);
	ak_ini_get_item_value(config.handle, "camera", "week_format", value);
	camera->week_format = atoi(value);
   	ak_ini_get_item_value(config.handle, "camera", "day_ctrl", value);
	camera->day_ctrl = atoi(value);
}

/**
 * ak_config_set_camera_info: set camera config
 * @camera[IN]: camera config
 * return: void
 */
void ak_config_set_camera_info(struct camera_disp_config *camera)
{
	ak_print_debug("osd_name len : %d\n", strlen(camera->osd_name));

	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	sprintf(value, "%d", camera->main_width);
	ak_ini_set_item_value(config.handle, "camera", "main_width", value);
	sprintf(value, "%d", camera->main_height);
	ak_ini_set_item_value(config.handle, "camera", "main_height", value);
	sprintf(value, "%d", camera->sub_width);
	ak_ini_set_item_value(config.handle, "camera", "sub_width", value);
	sprintf(value, "%d", camera->sub_height);
	ak_ini_set_item_value(config.handle, "camera", "sub_height", value);


	sprintf(value, "%d", camera->main_max_width);
	ak_ini_set_item_value(config.handle, "camera", "main_max_width", value);
	sprintf(value, "%d", camera->main_max_height);
	ak_ini_set_item_value(config.handle, "camera", "main_max_height", value);
	sprintf(value, "%d", camera->sub_max_width);
	ak_ini_set_item_value(config.handle, "camera", "sub_max_width", value);
	sprintf(value, "%d", camera->sub_max_height);
	ak_ini_set_item_value(config.handle, "camera", "sub_max_height", value);

	sprintf(value, "%d", camera->main_osd_size);
	ak_ini_set_item_value(config.handle, "camera", "main_osd_size", value);
	sprintf(value, "%d", camera->sub_osd_size);
	ak_ini_set_item_value(config.handle, "camera", "sub_osd_size", value);
	sprintf(value, "%d", camera->name_position);
	ak_ini_set_item_value(config.handle, "camera", "name_position", value);
	sprintf(value, "%d", camera->rate_position);
	ak_ini_set_item_value(config.handle, "camera", "rate_position", value);
	sprintf(value, "%d", camera->time_position);
	ak_ini_set_item_value(config.handle, "camera", "time_position", value);
	sprintf(value, "%d", camera->osd_switch);
	ak_ini_set_item_value(config.handle, "camera", "osd_switch", value);
	sprintf(value, "%s", camera->osd_name);
	ak_ini_set_item_value(config.handle, "camera", "osd_name", value);
	sprintf(value, "%d", camera->date_format);
	ak_ini_set_item_value(config.handle, "camera", "date_format", value);
	sprintf(value, "%d", camera->hour_format);
	ak_ini_set_item_value(config.handle, "camera", "hour_format", value);
	sprintf(value, "%d", camera->week_format);
	ak_ini_set_item_value(config.handle, "camera", "week_format", value);
	sprintf(value, "%d", camera->day_ctrl);
	ak_ini_set_item_value(config.handle, "camera", "day_ctrl", value);
}

/**
 * ak_config_get_camera_info: get camera config
 * return: camera config struct pointer
 */
struct camera_disp_config* ak_config_get_camera_info(void)
{
	return config.camera;
}

/**
 * ak_config_set_record_plan_by_net: set record plan config
 * @plan[IN]: record plan config
 * return: void
 */
void ak_config_set_record_plan_by_net(struct record_plan_config *plan)
{
	int index = 0;

	if(plan->active){
		for(index = 0; index < MAX_RECORD_PLAN_NUM; index ++){
		    if(config.record->plan[index].active == 0){
		        break;
		    }
		}
		if(index != MAX_RECORD_PLAN_NUM){
		    memcpy(&config.record->plan[index], plan,
		    	sizeof(struct record_plan_config));
		}
	}else{
		for(index = 0; index < MAX_RECORD_PLAN_NUM; index ++){
		    if((config.record->plan[index].start_time == plan->start_time)
		    	&& (config.record->plan[index].end_time == plan->end_time)){
		    	int i = 0;
		    	for(i=0; i<DAY_PER_WEEK; ++i){
		    		if(config.record->plan[index].week_enalbe[i]
		    			!= plan->week_enalbe[i]){
		    			break;
		    		}
		    	}
		    	if(i >= DAY_PER_WEEK){
		    		config.record->plan[index].active = 0;
		        	break;
		    	}
		    }
		}
	}

	ak_config_set_record_plan(config.record);
}

/**
 * ak_config_set_ptz: set ptz control config
 * @con[IN]: ptz control config
 * @para1[IN]: ptz control index
 * return: void
 */
void ak_config_set_ptz(void *con, int para1)
{
	struct ptz_pos * ptz_contrl = (struct ptz_pos *)con;
	char *title = "ptz";
	char name[50];
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	sprintf(name, "p%dL", para1);
	sprintf(value, "%d", ptz_contrl[para1-1].left);
	ak_ini_set_item_value(config.handle, title, name, value);
	sprintf(name, "p%dU", para1);
	sprintf(value, "%d", ptz_contrl[para1-1].up);
	ak_ini_set_item_value(config.handle, title, name, value);
}

/**
 * ak_config_get_ptz: get ptz control config
 * @con[OUT]: ptz control config
 * @para1[IN]: ptz control index
 * return: void
 */
void ak_config_get_ptz(void *con, int para1)
{
	struct ptz_pos * ptz_contrl = (struct ptz_pos *)con;

	char *title = "ptz";
	char name[50];
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	sprintf(name, "p%dL", para1);
	ak_ini_get_item_value(config.handle, title, name, value);
	ptz_contrl[para1-1].left = atoi(value);
	sprintf(name, "p%dU", para1);
	ak_ini_get_item_value(config.handle, title, name, value);
	ptz_contrl[para1-1].up = atoi(value);
}

/**
 * ak_config_set_ptz_unhit: set first ptz control config
 * @con[IN]: ptz control config
 * return: void
 */
void ak_config_set_ptz_unhit(void *con)
{
	struct ptz_pos * ptz_contrl = (struct ptz_pos *)con;
	char *title = "ptz";
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	if (access(CONFIG_PTZ_UNHIT_NAME, F_OK) != 0) {
		sprintf(value, "echo \"[%s]\" > %s", title, CONFIG_PTZ_UNHIT_NAME);
		system(value);
	}

	sprintf(value, "%d", ptz_contrl[0].left);
	ak_ini_set_item_value(config.handle, title, "hitL", value);
	sprintf(value, "%d", ptz_contrl[0].up);
	ak_ini_set_item_value(config.handle, title, "hitU", value);
}

/**
 * ak_config_get_ptz_unhit: get first ptz control config
 * @con[OUT]: ptz control config
 * return: void
 */
void ak_config_get_ptz_unhit(void *con)
{
	struct ptz_pos * ptz_contrl = (struct ptz_pos *)con;

	char *title = "ptz";
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	if (access(CONFIG_PTZ_UNHIT_NAME, F_OK) != 0) {
		sprintf(value, "echo \"[%s]\" > %s", title, CONFIG_PTZ_UNHIT_NAME);
		system(value);
	}

	ak_ini_get_item_value(config.handle, title, "hitL", value);
	ptz_contrl[0].left = atoi(value);
	ak_ini_get_item_value(config.handle, title, "hitU", value);
	ptz_contrl[0].up = atoi(value);
}

static void init_system_user(struct sys_user_config *user)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	ak_ini_get_item_value(config.handle, "global", "user", value);
	strcpy(user->name, value);
	ak_ini_get_item_value(config.handle, "global", "secret", value);
	strcpy(user->secret, value);
	ak_ini_get_item_value(config.handle, "global", "dev_name", value);
	strcpy(user->dev_name, value);
	ak_ini_get_item_value(config.handle, "global", "soft_version", value);
	user->soft_version = atoi(value);
	memset(user->uid_name,0,100);
	if(0 == ak_ini_get_item_value(config.handle, "global", "uid_name", value)){
		strcpy(user->uid_name, value);
	}
}

/**
 * ak_config_get_system_user: get system user info config
 * return: user info config struct pointer
 */
struct sys_user_config* ak_config_get_system_user(void)
{
	return config.user;
}

/**
 * ak_config_set_system_user: set user info config
 * @user_info[IN]: user info config
 * return: void
 */
void ak_config_set_system_user(struct sys_user_config *user)
{
	char *title = "global";
	char value[100];

	sprintf(value, "%s", user->name);
	ak_ini_set_item_value(config.handle, title, "user", value);
	sprintf(value, "%s", user->secret);
	ak_ini_set_item_value(config.handle, title, "secret", value);
	sprintf(value, "%s", user->dev_name);
	ak_ini_set_item_value(config.handle, title, "dev_name", value);
	sprintf(value, "%u", user->soft_version);
	ak_ini_set_item_value(config.handle, title, "soft_version", value);
}

/** ak_config_get_image: get image config, title: image
 * @void
 * return: image config info pointer
 */
struct image_config* ak_config_get_image(void)
{
	return config.image;
}


/**
 * ak_config_set_image: set image config, title: image
 * @image[IN]: image config
 * return: void
 */
void ak_config_set_image(struct image_config *image)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	sprintf(value, "%d", image->flip);
	ak_ini_set_item_value(config.handle, "image", "flip", value);
	sprintf(value, "%d", image->mirror);
	ak_ini_set_item_value(config.handle, "image", "mirror", value);
	sprintf(value, "%d", image->irled_mode);
	ak_ini_set_item_value(config.handle, "image", "irled_mode", value);
}

/** ak_config_get_audio: get audio config, title: audio
 * @void
 * return: audio config info pointer
 */
struct audio_config* ak_config_get_audio(void)
{
	return config.audio;
}

/**
 * ak_config_set_audio: set audio config, title: audio
 * @audio[IN]: audio config
 * return: void
 */
void ak_config_set_audio(struct audio_config *audio)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	sprintf(value, "%d", audio->ai_source);
	ak_ini_set_item_value(config.handle, "audio", "ai_source", value);
}

/** ak_config_get_mt: get mt config, title: mt
 * @void
 * return: mt config info pointer
 */
struct mt_config* ak_config_get_mt(void)
{
	return config.mt;
}

/**
 * ak_config_set_mt: set mt config, title: mt
 * @mt[IN]: mt config
 * return: void
 */
void ak_config_set_mt(struct mt_config *mt)
{
	char value[CONFIG_VALUE_BUF_SIZE] = {0};

	sprintf(value, "%d", mt->mt_en);
	ak_ini_set_item_value(config.handle, "mt", "mt_en", value);
	sprintf(value, "%d", mt->calibrate_en);
	ak_ini_set_item_value(config.handle, "mt", "calibrate_en", value);
	sprintf(value, "%d", mt->wait_time);
	ak_ini_set_item_value(config.handle, "mt", "wait_time", value);
	sprintf(value, "%d", mt->draw_box_en);
	ak_ini_set_item_value(config.handle, "mt", "draw_box_en", value);
	sprintf(value, "%d", mt->switch_v);
	ak_ini_set_item_value(config.handle, "mt", "switch_v", value);

	sprintf(value, "%d", mt->flt_big_day);
	ak_ini_set_item_value(config.handle, "mt", "flt_big_day", value);
	sprintf(value, "%d", mt->flt_big_night);
	ak_ini_set_item_value(config.handle, "mt", "flt_big_night", value);
	sprintf(value, "%d", mt->flt_small_day);
	ak_ini_set_item_value(config.handle, "mt", "flt_small_day", value);
	sprintf(value, "%d", mt->flt_small_night);
	ak_ini_set_item_value(config.handle, "mt", "flt_small_night", value);
	sprintf(value, "%d", mt->valid_size_min);
	ak_ini_set_item_value(config.handle, "mt", "valid_size_min", value);
	sprintf(value, "%d", mt->valid_size_max);
	ak_ini_set_item_value(config.handle, "mt", "valid_size_max", value);
}


/**
 * ak_config_init_ini: init ini module
 * return: void
 */
void ak_config_init_ini(void)
{
	config.handle = ak_ini_init(CONFIG_ANYKA_FILE_NAME);
	if(NULL == config.handle) {
		ak_print_normal_ex("open config file failed, create it.\n");
		system("cp /usr/local/factory_cfg.ini /etc/jffs2/anyka_cfg.ini");
		config.handle = ak_ini_init(CONFIG_ANYKA_FILE_NAME);
		if(NULL == config.handle) {
			ak_print_normal_ex("open config file failed & create failed\n");
			return;
		}
	} else {
		ak_print_normal_ex("anyka config file check ok.\n\n");
	}

	config.video = (struct video_config *)calloc(1,
		sizeof(struct video_config));
	config.record = (struct video_record_config *)calloc(1,
		sizeof(struct video_record_config));
	config.alarm = (struct sys_alarm_config *)calloc(1,
		sizeof(struct sys_alarm_config));
	config.cloud = (struct sys_cloud_config *)calloc(1,
		sizeof(struct sys_cloud_config));
	config.net = (struct sys_net_config *)calloc(1,
		sizeof(struct sys_net_config));
	config.wifi = (struct sys_wifi_config *)calloc(1,
		sizeof(struct sys_wifi_config));
	config.camera = (struct camera_disp_config *)calloc(1,
		sizeof(struct camera_disp_config));
	config.user = (struct sys_user_config *)calloc(1,
		sizeof(struct sys_user_config));
	config.image = (struct image_config *)calloc(1,
		sizeof(struct image_config));
	config.audio = (struct audio_config *)calloc(1,
		sizeof(struct audio_config));
	config.auto_day_night = (struct auto_day_night_config *)calloc(1,
		sizeof(struct auto_day_night_config));
	config.mt = (struct mt_config *)calloc(1,
		sizeof(struct mt_config));

	init_video_param(config.video);
#if CONFIG_DUMP_INFO
	ak_ini_dump_config(config.handle, "ethernet");
#endif
	init_record(config.record);
	init_sys_alarm(config.alarm);
	init_sys_cloud_config(config.cloud);
	init_sys_net(config.net);
	init_sys_wifi(config.wifi);
	init_camera_config(config.camera);
	init_system_user(config.user);
	init_image(config.image);
	init_audio(config.audio);
	init_config_auto_day_night(config.auto_day_night);
	init_mt(config.mt);
}

/**
 * ak_config_release_ini: destroy ini module
 * return: void
 */
void ak_config_release_ini(void)
{
	if (config.video) {
		free(config.video);
		config.video = NULL;
	}
	if (config.record) {
		free(config.record);
		config.record = NULL;
	}
	if (config.alarm) {
		free(config.alarm);
		config.alarm = NULL;
	}
	if (config.cloud) {
		free(config.cloud);
		config.cloud = NULL;
	}
	if (config.net) {
		free(config.net);
		config.net = NULL;
	}
	if (config.wifi) {
		free(config.wifi);
		config.wifi = NULL;
	}
	if (config.camera) {
		free(config.camera);
		config.camera = NULL;
	}
	if (config.user) {
		free(config.user);
		config.user = NULL;
	}
	if (config.image) {
		free(config.image);
		config.image = NULL;
	}
	if (config.audio) {
		free(config.audio);
		config.audio = NULL;
	}
    if (config.auto_day_night) {
        free(config.auto_day_night);
        config.auto_day_night = NULL;
    }
	if (config.mt) {
		free(config.mt);
		config.mt = NULL;
	}

	ak_ini_destroy(config.handle);
}

/**
 * ak_config_flush_data: flush data
 * return: void
 */
void ak_config_flush_data(void)
{
	if(config.handle)
		ak_ini_flush_data(config.handle);
}

/**
 * ak_config_update_ssid:  get wifi ssid  from config file
 * return: 0 -> success; -1 -> fail;
 * notes: wifi ssid configure change by script cmd, we need to
 *          update configure copy in memory.
 *          it is special.
 */
int ak_config_update_ssid(void)
{
	char cmd[128] = {0};
	char res[128] = {0};
	/* on this case, file tmp_cfg.ini must store the ssid and passwd */
	char *tmp_cfg = "/tmp/tmp_cfg.ini";
	char value[CONFIG_VALUE_BUF_SIZE] = {0};
	int retry_count = 0;

	if (NULL == config.handle) {
		ak_print_error_ex("config file not init.\n");
		return AK_FAILED;
	}
	if (NULL == config.wifi) {
		ak_print_error_ex("config.wifi null.\n");
		return AK_FAILED;
	}
	for (retry_count = 0; retry_count < 4; retry_count++) {
		/* copy current ini to tmp */
		sprintf(cmd, "cp %s %s", CONFIG_ANYKA_FILE_NAME, tmp_cfg);
		if (ak_cmd_exec(cmd, res, sizeof(res))) {
			ak_print_error_ex("%s fail.\n", cmd);
			return AK_FAILED;
		}
		if (access(tmp_cfg, F_OK)) {
			continue;
		}

		/* open tmp file, which store ssid and passwd */
		void *cfg_handle = ak_ini_init(tmp_cfg);
		if (NULL == cfg_handle) {
		    ak_print_error_ex("ini init %s failed.\n", tmp_cfg);
			return AK_FAILED;
		}

		/* get new ssid value */
		ak_ini_get_item_value(cfg_handle, "wireless", "ssid", value);

		/* copy new ssid to current global ini handle */
		strcpy(config.wifi->ssid, value);

		/* destroy tmp init info */
		if (AK_FAILED == ak_ini_destroy(cfg_handle)) {
			ak_print_error_ex("ini_destroy failed\n");
		}

		/* remove tmp file */
		remove(tmp_cfg);

		/* check result */
		if (strlen(value) > 0) {
			break;
		}
		ak_print_normal_ex("retry count:%d\n", retry_count);
		ak_sleep_ms(500);
	}

	if (4 == retry_count) {
		return AK_FAILED;
	}
	ak_ini_set_item_value(config.handle, "wireless", "ssid", value);
	ak_print_normal_ex("ssid:%s pwd:%s\n", config.wifi->ssid, config.wifi->passwd);

	return AK_SUCCESS;
}
