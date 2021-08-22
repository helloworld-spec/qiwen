#ifndef _AK_CONFIG_H_
#define _AK_CONFIG_H_

#define DAY_PER_WEEK	7
#define MAX_RECORD_PLAN_NUM 3

#define NIGHT_ARR_NUM		5	// must eque NIGHT_ARRAY_NUM in ak_vpss.h
#define DAY_ARR_NUM		10	// must eque DAY_ARRAY_NUM in ak_vpss.h

struct video_config {
	int main_min_qp;
	int main_max_qp;
	int sub_min_qp;
	int sub_max_qp;

	int main_fps;
	int sub_fps;
	int main_min_kbps;
	int sub_min_kbps;
	int main_max_kbps;
	int sub_max_kbps;

	int main_gop_len;
	int sub_gop_len;
	int main_video_mode;	/* cbr or vbr */
	int sub_video_mode;

	/* video type, 0->H.264, 1->MJPEG, 2->H.265 */
	int main_enc_type;
	int sub_enc_type;

	/* ratio of target bitrate and max bitrate */
	int main_target_ratio_264;
	int sub_target_ratio_264;
	int main_target_ratio_265;
	int sub_target_ratio_265;

	/* smart encode model configurations */
	int main_smart_goplen;
	int sub_smart_goplen;
	int main_smart_mode;
	int sub_smart_mode;
	int main_smart_quality_264;
	int main_smart_quality_265;
	int sub_smart_quality_264;
	int sub_smart_quality_265;

	/* ratio of target bitrate and max bitrate on smart mode */
	int main_smart_target_ratio_264;
	int sub_smart_target_ratio_264;
	int main_smart_target_ratio_265;
	int sub_smart_target_ratio_265;

	int main_smart_static_value;
	int sub_smart_static_value;
};

struct sys_user_config {
	char name[20];					//user name
	char secret[20];				//password
	char dev_name[100];				//device name
	char uid_name[100];				//uid file name
	unsigned int soft_version;
};

struct pictrue_config {
	int chanel;
};

struct sys_net_config {
	int dhcp;			//0:Fixed IP  1:dynamic ip
	char ipaddr[16];	//local ip
	char netmask[16];	//net mask
	char gateway[16];
	char firstdns[16];	//main dns
	char backdns[16];	//second dns
	int ip_adjust_en;
};

struct record_plan_config {
	int active;			//active flag of plan record
	/* week day enable flag, 0 Sunday, 1-6 Monday to Saturday */
	int week_enalbe[DAY_PER_WEEK];
	time_t start_time;	//senconds from current day's 00:00:00
	time_t end_time;	//senconds from current day's 00:00:00
};

struct video_record_config {
	int server_flag;	//start record server
	int record_mode;	//1->plan record,  2->alarm record
	int duration;		//duration of each record file(unit: second)

	int sample_rate;	//audio sample rate
	int save_cyc_flag;	//cycle record flag, 1->cycle, 0->normal
	int save_file_fps;	//originale fps
	int save_file_kbps;
	int min_file_fps;	//min fps when switch record frame
	char path[255];		//plan-record file saving path
	char prefix[20];	//plan-record file saving prefix

	int alarm_pre_time; //keeping video and audio before alarm record trigger
	int alarm_stop_time;//stop record after keep silence time
	int alarm_limit_time;//record limit time

	struct record_plan_config plan[MAX_RECORD_PLAN_NUM];
	int file_type; //record file postfix.(mp4 | avi)
	int audio_type;     //audio encode type
};

struct auto_day_night_config {
	int auto_day_night_enable;				// 1 enable,0 disable
    int day_night_mode;				// 0 night,1 day,2 auto
    int day_to_night_lum;	// DAY_TO_NIGHT_LUM_FACTOR
    int night_to_day_lum;	// NIGHT_TO_DAY_LUM_FACTOR
    int night_cnt[NIGHT_ARR_NUM];// AWB night cnt array
    int day_cnt[DAY_ARR_NUM];	// AWB day cnt array
    int lock_time;
    int quick_switch_mode;			// quick switch mode
};

struct camera_disp_config {
	int main_width;
	int main_height;
	int sub_width;
	int sub_height;
	int main_max_width;
    int main_max_height;
    int sub_max_width;
    int sub_max_height;
	int main_osd_size;
	int sub_osd_size;
	int rate_position;		//display rate info OSD
	int name_position;
	int time_position;
	int osd_switch;
	char osd_name[50];
	int date_format;
	int hour_format;
	int week_format;
	int day_ctrl;
};

struct sys_alarm_config {
	int md_set;
	int md_level_1;
	int md_level_2;
	int md_level_3;

	int sd_set;
	int sd_level_1;
	int sd_level_2;
	int sd_level_3;

	int i2o_detection;
	int smoke_detection;
	int shadow_detection;
	int other_detection;

	int send_type;
	int interval_time;
	int send_msg_time;
};

struct sys_cloud_config {
	int dana;
	int rtsp;
};

enum wifi_enc_type {
	WIFI_ENCTYPE_INVALID,
	WIFI_ENCTYPE_NONE,
	WIFI_ENCTYPE_WEP,
	WIFI_ENCTYPE_WPA_TKIP,
	WIFI_ENCTYPE_WPA_AES,
	WIFI_ENCTYPE_WPA2_TKIP,
	WIFI_ENCTYPE_WPA2_AES
};

struct sys_wifi_config {
	char ssid[32];
	char mode[32];
	char passwd[32];
	enum wifi_enc_type enc_type;
	char ap_ssid[32];
	char ap_pswd[32];
};

struct cloud_alarm_config {
	unsigned char IsOpenMotionDetection;//close:1 open:0
	unsigned char MontionDetectionLevel;//level: 0  1  2    2 is Highest

	unsigned char IsOpenSoundDetection;
	unsigned char SoundDetectionLevel;

	unsigned char IsOpenI2ODetection;
	unsigned char I2ODetectionLevel;

	unsigned char IsOpenShadowDetection;
	unsigned char ShadowDetectionLevel;

	unsigned char IsOpenSmokeDetection;
	unsigned char SmokeetectionLevel;

	unsigned char IsOpenOther;
	unsigned char OtherDetectionLevel;
};
struct image_config {
	int	flip;
	int	mirror;
	int irled_mode;
};

struct audio_config {
	int	ai_source;
};

struct mt_config {
	int mt_en;			//mt enable
	int calibrate_en;	//calibrate or not when return to default angle
	int wait_time;		//wait some seconds if no moving then return to default angle
	int	draw_box_en;	//draw box for moving rect or not
	int switch_v;		//switch of vertical direction
	int flt_big_day;	//big filter to roughly detect motion region (day)
	int flt_big_night;	//big filter to roughly detect motion region (night)
	int flt_small_day;	//small filter to expand rough detection result (day)
	int flt_small_night;//small filter to expand rough detection result (night)
	int valid_size_min;	//min size of valid motion region
	int valid_size_max;	//max size of valid motion region
};


/**
 * ak_config_get_version - get config version
 * return: version string
 */
const char* ak_config_get_version(void);

/**
 * ak_config_get_record: get record config
 * return: record config struct pointer
 */
struct video_record_config* ak_config_get_record(void);

/**
 * ak_config_set_record_plan: set record config
 * @record[IN]: record config
 * return: void
 */
void ak_config_set_record_plan(struct video_record_config *record);

/**
 * ak_config_get_sys_alarm: get system alarm config
 * return: system alarm config struct pointer
 */
struct sys_alarm_config* ak_config_get_sys_alarm(void);

/**
 * ak_config_set_sys_alarm: set system alarm config
 * return: void
 */
void ak_config_set_sys_alarm(void);

/**
 * ak_config_set_record_plan_by_net: set record plan config
 * @plan[IN]: record plan config
 * return: void
 */
void ak_config_set_record_plan_by_net(struct record_plan_config *plan);

/**
 * ak_config_init_ini: init ini module
 * return: void
 */
void ak_config_init_ini(void);

/**
 * ak_config_release_ini: destroy ini module
 * return: void
 */
void ak_config_release_ini(void);

/**
 * ak_config_flush_data: flush data
 * return: void
 */
void ak_config_flush_data(void);

/**
 * ak_config_update_ssid:  get wifi ssid  from config file
 * return: 0 -> success; -1 -> fail;
 * notes: wifi ssid configure change by script cmd, we need to
 *          update configure copy in memory.
 *          it is special.
 */
int ak_config_update_ssid(void);

/**
 * ak_config_get_ptz: get ptz control config
 * @con[OUT]: ptz control config
 * @para1[IN]: ptz control index
 * return: void
 */
void ak_config_get_ptz(void *con, int para1);

/**
 * ak_config_set_ptz: set ptz control config
 * @con[IN]: ptz control config
 * @para1[IN]: ptz control index
 * return: void
 */
void ak_config_set_ptz(void *con, int para1);

/**
 * ak_config_set_ptz_unhit: set first ptz control config
 * @con[IN]: ptz control config
 * return: void
 */
void ak_config_set_ptz_unhit(void *con);

/**
 * ak_config_get_ptz_unhit: get first ptz control config
 * @con[OUT]: ptz control config
 * return: void
 */
void ak_config_get_ptz_unhit(void *con);

/** ak_config_get_sys_video: get system video config, title: video
 * @void
 * return: video config info pointer
 */
struct video_config* ak_config_get_sys_video(void);

/**
 * ak_config_set_sys_video: set video config
 * @video[IN]: video config
 * return: void
 */
void ak_config_set_sys_video(struct video_config *video);

/**
 * ak_config_get_sys_cloud: get system cloud config
 * return: cloud config struct pointer
 */
struct sys_cloud_config* ak_config_get_sys_cloud(void);

/**
 * ak_config_get_sys_net: get system network config
 * return: network config struct pointer
 */
struct sys_net_config* ak_config_get_sys_net(void);

/**
 * ak_config_set_sys_net: set system network config
 * @net[IN]: network config
 * return: void
 */
void ak_config_set_sys_net(struct sys_net_config *net);

/**
 * ak_config_get_sys_wifi: get system wifi config
 * return: wifi config struct pointer
 */
struct sys_wifi_config* ak_config_get_sys_wifi(void);

/**
 * ak_config_set_sys_wifi: set system wifi config
 * @wifi[IN]: wifi config
 * @save_ssid_f[IN]: save ssid or not
 * return: void
 */
void ak_config_set_sys_wifi(struct sys_wifi_config *wifi,int save_ssid_f);

/**
 * ak_config_set_camera_info: set camera config
 * @camera[IN]: camera config
 * return: void
 */
void ak_config_set_camera_info(struct camera_disp_config *camera);

/**
 * ak_config_get_camera_info: get camera config
 * return: camera config struct pointer
 */
struct camera_disp_config* ak_config_get_camera_info(void);

/**
 * ak_config_set_system_user: set user info config
 * @user_info[IN]: user info config
 * return: void
 */
void ak_config_set_system_user(struct sys_user_config *user_info);

/**
 * ak_config_get_system_user: get system user info config
 * return: user info config struct pointer
 */
struct sys_user_config* ak_config_get_system_user(void);

/** ak_config_get_audio: get audio config, title: audio
 * @void
 * return: audio config info pointer
 */
struct audio_config* ak_config_get_audio(void);

/**
 * ak_config_set_audio: set audio config, title: audio
 * @audio[IN]: audio config
 * return: void
 */
void ak_config_set_audio(struct audio_config *audio);

/** ak_config_get_image: get image config, title: image
 * @void
 * return: image config info pointer
 */
struct image_config* ak_config_get_image(void);

/**
 * ak_config_set_image: set image config, title: image
 * @image[IN]: image config
 * return: void
 */
void ak_config_set_image(struct image_config *image);

/**
 * ak_config_set_auto_day_night: set auto day night switch config
 * @auto_day_night[IN]: auto day night switch config
 * return: void
 */
void ak_config_set_auto_day_night(struct auto_day_night_config
								*auto_day_night);

/**
 * ak_config_get_auto_day_night: get auto day night switch config
 * return: auto day night config struct pointer
 */
struct auto_day_night_config* ak_config_get_auto_day_night(void);

/** ak_config_get_mt: get mt config, title: mt
 * @void
 * return: mt config info pointer
 */
struct mt_config* ak_config_get_mt(void);

/**
 * ak_config_set_mt: set mt config, title: mt
 * @mt[IN]: mt config
 * return: void
 */
void ak_config_set_mt(struct mt_config *mt);

#endif
