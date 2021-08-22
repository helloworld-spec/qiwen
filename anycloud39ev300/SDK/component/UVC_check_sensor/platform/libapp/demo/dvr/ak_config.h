#ifndef _AK_CONFIG_H_
#define _AK_CONFIG_H_

#include <sys/time.h>

#define DAY_PER_WEEK	7

struct video_config {
    int minQp;
    int maxQp;
    int V720Pfps;
    int V720PDefaultkbps;
    int V720Pminkbps;
    int V720Pmaxkbps;
    
    int save_cyc_flag;	//cycle record flag, 1->cycle, 0->normal
    int savefilefps;	//originale fps
    int savefilekbps;
    int minfilefps;		//min fps when switch record frame
    char recpath[200];	//plan-record file saving path
    int saveWidth;
    int saveHeight;
    int saveAudioEncode;
    int saveTime;
    int saveFileType;
    
    int VGAPfps;
    int VGADefaultkbps;
    int VGAminkbps;
    int VGAmaxkbps;
    
    int gopLen;
	int	quality;
	int	pic_ch;
	int video_mode;
};

struct sys_user_config {
    char name[20];					//user name
    char secret[20];				//password
    char dev_name[100];				//device name
    unsigned short firmware_update;
    int debuglog;
	int rtsp_support;
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
};

#define MAX_RECORD_PLAN_NUM 3
struct record_plan_config {
    int active;			//active flag of plan record
    /* week day enable flag, 0 Sunday, 1-6 Monday to Saturday */
    int week_enalbe[DAY_PER_WEEK];
    time_t start_time;	//senconds from current day's 00:00:00	
    time_t end_time;	//senconds from current day's 00:00:00	
	int record_time;	//time of each record file(unit: second)
};

struct video_record_config {
    int video_index;
    struct record_plan_config plan[MAX_RECORD_PLAN_NUM];
};

struct camera_disp_config {
    int width;
    int height;
	int osd_position;
    int osd_switch;
    char osd_name[50];
    unsigned short osd_unicode_name[50];
    unsigned short osd_unicode_name_len;  
    int time_switch;
    int date_format;
    int hour_format;
    int week_format;    
	int ircut_mode;
};

struct sys_alarm_config {
    int motion_detection;
    int motion_detection_1;
    int motion_detection_2;
    int motion_detection_3;
    int opensound_detection;
    int opensound_detection_1;
    int opensound_detection_2;
    int opensound_detection_3;
    
    int openi2o_detection;
    int smoke_detection;
    int shadow_detection;
    int other_detection;
    
    int send_type;
    int save_record;
	int interval_time;
	int move_over_time;
	int send_msg_time;
    int motion_size_x;
    int motion_size_y;
};

struct sys_onvif_config {
    int fps1;
    int kbps1;
    int quality1;
    int fps2;
    int kbps2;
    int quality2;
};

struct sys_cloud_config {
    int onvif;
    int dana;
    int tutk;
    int tencent;
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

struct video_record_config* ak_config_get_record_plan(void);
void ak_config_set_record_plan(struct video_record_config *record);

struct sys_alarm_config* ak_config_get_sys_alarm(void);

void ak_config_set_sys_alarm(void);

void ak_config_set_record_plan_by_net(struct record_plan_config *plan);

void ak_config_init_ini(void);
void ak_config_release_ini(void);

void ak_config_get_ptz(void *con, int para1);
void ak_config_set_ptz(void *con, int para1);

void ak_config_set_ptz_unhit(void *con);
void ak_config_get_ptz_unhit(void *con);

struct video_config* ak_config_get_sys_video(void);

struct sys_cloud_config* ak_config_get_sys_cloud(void);

struct sys_onvif_config* ak_config_get_sys_onvif(void);
void ak_config_set_sys_onvif(struct sys_onvif_config *onvif);

struct sys_net_config* ak_config_get_sys_net(void);
void ak_config_set_sys_net(struct sys_net_config *net);

struct sys_wifi_config* ak_config_get_sys_wifi(void);
void ak_config_set_sys_wifi(struct sys_wifi_config *wifi);

void ak_config_set_camera_info(struct camera_disp_config *camera);
struct camera_disp_config* ak_config_get_camera_info(void);

void ak_config_set_system_user(struct sys_user_config *user_info);
struct sys_user_config* ak_config_get_system_user(void);

#endif
