#ifndef DANA_VIDEO_CMD_H_H
#define DANA_VIDEO_CMD_H_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
typedef enum danavideo_video_quality_e_ {
	DANAVIDEO_BITRATE_LOWEST,
	DANAVIDEO_BITRATE_LOW,
	DANAVIDEO_BITRATE_NORMAL,
	DANAVIDEO_BITRATE_HIGH,
	DANAVIDEO_BITRATE_HIGHEST,
	DANAVIDEO_QUALITY_AUTO = 100,
	DANAVIDEO_QUALITY_MANUALLY = 101	//
} danavideo_video_quality_t;
*/

typedef enum danavideo_euc_type_e_ {
	DANAVIDEO_EUCTYPE_PHONE = 1,
	DANAVIDEO_EUCTYPE_PAD,
	DANAVIDEO_EUCTYPE_PC
} danavideo_euc_type_t;

/* Power freq & Video Norm */
#define DANAVIDEO_POWERFREQ_50HZ  0
#define DANAVIDEO_POWERFREQ_60HZ  1
#define DANAVIDEO_VIDEONORM_PAL  0
#define DANAVIDEO_VIDEONORM_NTSC 1

typedef enum danavideo_wifi_enctype_e_ {
	DANAVIDEO_WIFI_ENCTYPE_INVALID,
	DANAVIDEO_WIFI_ENCTYPE_NONE,
	DANAVIDEO_WIFI_ENCTYPE_WEP,
	DANAVIDEO_WIFI_ENCTYPE_WPA_TKIP,
	DANAVIDEO_WIFI_ENCTYPE_WPA_AES,
	DANAVIDEO_WIFI_ENCTYPE_WPA2_TKIP,
	DANAVIDEO_WIFI_ENCTYPE_WPA2_AES
} danavideo_wifi_enctype_t;


//call with ioctl(fd, PTZ_CTRL_XXX, param);
//param:
//          32               16               0
//           |----------------|----------------|
// move up/down/left/right:
//                                   speed
// move others:
//                  y-spd            x-spd 
//
// psp:              0               psp
// cruise:           0               route
// others:           0               0
//
typedef enum  danavideo_ptz_ctrl_e_ { 
	DANAVIDEO_PTZ_CTRL_STOP=100, 
	DANAVIDEO_PTZ_CTRL_MOVE_UP, 
	DANAVIDEO_PTZ_CTRL_MOVE_DOWN,
	DANAVIDEO_PTZ_CTRL_MOVE_LEFT,
	DANAVIDEO_PTZ_CTRL_MOVE_RIGHT, 
	DANAVIDEO_PTZ_CTRL_MOVE_UPLEFT, 
	DANAVIDEO_PTZ_CTRL_MOVE_DOWNLEFT, 
	DANAVIDEO_PTZ_CTRL_MOVE_UPRIGHT, 
	DANAVIDEO_PTZ_CTRL_MOVE_DOWNRIGHT, 

	DANAVIDEO_PTZ_CTRL_IRIS_IN, 
	DANAVIDEO_PTZ_CTRL_IRIS_OUT, 
	DANAVIDEO_PTZ_CTRL_FOCUS_ON, 
	DANAVIDEO_PTZ_CTRL_FOCUS_OUT, 
	DANAVIDEO_PTZ_CTRL_ZOOM_IN, 
	DANAVIDEO_PTZ_CTRL_ZOOM_OUT, 

	DANAVIDEO_PTZ_CTRL_SET_PSP, 
	DANAVIDEO_PTZ_CTRL_CALL_PSP, 
	DANAVIDEO_PTZ_CTRL_DELETE_PSP, 

	DANAVIDEO_PTZ_CTRL_BEGIN_CRUISE_SET, 
	DANAVIDEO_PTZ_CTRL_SET_CRUISE, 
	DANAVIDEO_PTZ_CTRL_END_CRUISE_SET, 
	DANAVIDEO_PTZ_CTRL_CALL_CRUISE, 
	DANAVIDEO_PTZ_CTRL_DELETE_CRUISE, 
	DANAVIDEO_PTZ_CTRL_STOP_CRUISE, 

	DANAVIDEO_PTZ_CTRL_AUTO_SCAN, 

	DANAVIDEO_PTZ_CTRL_RAINBRUSH_START, 
	DANAVIDEO_PTZ_CTRL_RAINBRUSH_STOP,
	DANAVIDEO_PTZ_CTRL_LIGHT_ON, 
	DANAVIDEO_PTZ_CTRL_LIGHT_OFF,

	DANAVIDEO_PTZ_CTRL_MAX 
} danavideo_ptz_ctrl_t;

typedef enum danavideo_rec_rate_e_ {
    DANAVIDEO_REC_RATE_HALF = 1,
    DANAVIDEO_REC_RATE_NORMAL,
    DANAVIDEO_REC_RATE_DOUBLE,
    DANAVIDEO_REC_RATE_FOUR,
} danavideo_rec_rate_t;

typedef enum danavideo_rec_week_e_ {
    DANAVIDEO_REC_WEEK_MON = 1,
    DANAVIDEO_REC_WEEK_TUE,
    DANAVIDEO_REC_WEEK_WED,
    DANAVIDEO_REC_WEEK_THU,
    DANAVIDEO_REC_WEEK_FRI,
    DANAVIDEO_REC_WEEK_SAT,
    DANAVIDEO_REC_WEEK_SUN,
} danavideo_rec_week_t;

typedef enum danavideo_rec_typt_e_ {
    DANAVIDEO_REC_TYPE_NORMAL = 1,
    DANAVIDEO_REC_TYPE_ALARM,
} danavideo_rec_type_t;

typedef enum danavideo_rec_action_e_ {
    DANAVIDEO_REC_ACTION_PAUSE = 1,
    DANAVIDEO_REC_ACTION_PLAY,
} danavideo_rec_action_t;

typedef enum danavideo_rec_get_type_e_ {
    DANAVIDEO_REC_GET_TYPE_NEXT = 1,
    DANAVIDEO_REC_GET_TYPE_PREV,
} danavideo_rec_get_type_t;

typedef enum danavideo_rec_plan_status_e_ {
    DANAVIDEO_REC_PLAN_CLOSE = 0,
    DANAVIDEO_REC_PLAN_OPEN,
} danavideo_rec_plan_status_t;

typedef enum danavideo_osd_show_e_ {
    DANAVIDEO_OSD_SHOW_CLOSE,
    DANAVIDEO_OSD_SHOW_OPEN
} danavideo_osd_show_t;

typedef enum danavideo_osd_date_format_e_ {
    DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD,       // YYYY-MM-DD
    DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY,       // MM-DD-YYYY
    DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD_CH,    // YYYY年MM月DD日
    DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY_CH,    // MM月DD日YYYY年
    DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY,       // DD-MM-YYYY
    DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY_CH     // DD日MM月YYYY年
} danavideo_osd_date_format_t;

typedef enum danavideo_osd_time_format_e_ {
    DANAVIDEO_OSD_TIME_24_HOUR,
    DANAVIDEO_OSD_TIME_12_HOUR
} danavideo_osd_time_format_t;

typedef enum danavideo_osd_datetime_attr_e_ {
    DANAVIDEO_OSD_DATETIME_TRANSPARENT,
    DANAVIDEO_OSD_DATETIME_DISPLAY
} danavideo_osd_datetime_attr_t;

// 日夜转换
typedef enum danaicr_mode_type_e_ {
    DANA_ICR_MODE_COLOR = 1, // 彩色
    DANA_ICR_MODE_BW    = 2, // 黑白
    DANA_ICR_MODE_AUTO  = 3, // 自动 
} danaicr_mode_type_t;

typedef enum danasdc_status_type_e_ {
    DANA_SDC_STATUS_NO_SDC = 1,  //无sdc
    DANA_SDC_STATUS_NOMAL  = 2,  //正常工作状态
    DANA_SDC_STATUS_FORMAT = 3,  //格式化状态
    DANA_SDC_STATUS_DAMAGE = 4,  //损坏状态
} danasdc_status_type_t;

typedef struct _danavideocmd_getsdcstatus_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETSDCSTATUS_ARG;

typedef struct _danavideocmd_devdef_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_DEVDEF_ARG;

typedef struct _danavideocmd_devreboot_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_DEVREBOOT_ARG;

typedef struct _danavideocmd_getscreen_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETSCREEN_ARG;

typedef struct _danavideocmd_getalarm_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETALARM_ARG;

typedef struct _danavideocmd_getbaseinfo_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETBASEINFO_ARG;

typedef struct _danavideocmd_getcolor_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETCOLOR_ARG;

typedef struct _danavideocmd_getflip_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETFLIP_ARG;

typedef struct _danavideocmd_getfunlist_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETFUNLIST_ARG;

typedef struct _danavideocmd_getnetinfo_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETNETINFO_ARG;

typedef struct _danavideocmd_getpowerseq_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETPOWERFREQ_ARG;

typedef struct _danavideocmd_gettime_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETTIME_ARG;

typedef struct _danavideocmd_getwifiap_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETWIFIAP_ARG;

typedef struct _danavideocmd_getwifi_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETWIFI_ARG;

typedef struct _danavideocmd_ptzctrl_arg_s {
    uint32_t ch_no;
    uint32_t code;
    uint32_t para1;
    uint32_t para2;
} DANAVIDEOCMD_PTZCTRL_ARG;

typedef struct _danavideocmd__arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_SDCFORMAT_ARG;

typedef struct _danavideocmd_setalarm_arg_s {
    uint32_t ch_no;
    uint32_t motion_detection;
    uint32_t opensound_detection;
    uint32_t openi2o_detection;
    uint32_t smoke_detection;
    uint32_t shadow_detection;
    uint32_t other_detection;
    bool has_pir_detection;
    uint32_t pir_detection;
} DANAVIDEOCMD_SETALARM_ARG;

typedef struct _danavideocmd_setchan_arg_s {
    uint32_t ch_no;
    size_t chans_count;
    uint32_t chans[33];
} DANAVIDEOCMD_SETCHAN_ARG;

typedef struct _danavideocmd_setcolor_arg_s {
    uint32_t ch_no;
    uint32_t video_rate;
    uint32_t brightness;
    uint32_t contrast;
    uint32_t saturation;
    uint32_t hue;
} DANAVIDEOCMD_SETCOLOR_ARG;

typedef struct _danavideocmd_setflip_arg_s {
    uint32_t ch_no;
    uint32_t flip_type;
} DANAVIDEOCMD_SETFLIP_ARG;

typedef struct _danavideocmd_setnetinfo_arg_s {
    uint32_t ch_no;
    uint32_t ip_type;
    char ipaddr[16];
    char netmask[16];
    char gateway[16];
    uint32_t dns_type;
    char dns_name1[16];
    char dns_name2[16];
    uint32_t http_port;
} DANAVIDEOCMD_SETNETINFO_ARG;

typedef struct _danavideocmd_setpowerswq_arg_s {
    uint32_t ch_no;
    uint32_t freq;
} DANAVIDEOCMD_SETPOWERFREQ_ARG;

typedef struct _danavideocmd_settime_arg_s {
    uint32_t ch_no;
    int64_t now_time;
    char time_zone[65];
    bool has_ntp_server1;
    char ntp_server1[257];
    bool has_ntp_server2;
    char ntp_server2[257];
} DANAVIDEOCMD_SETTIME_ARG;

typedef struct _danavideocmd_setvideo_arg_s {
    uint32_t ch_no;
    uint32_t video_quality;
} DANAVIDEOCMD_SETVIDEO_ARG;

typedef struct _danavideocmd_setwifiap_arg_s {
    uint32_t ch_no;
    uint32_t ip_type;
    char ipaddr[16];
    char netmask[16];
    char gateway[16];
    char dns_name1[16];
    char dns_name2[16];
    char essid[33];
    char auth_key[65];
    uint32_t enc_type;
} DANAVIDEOCMD_SETWIFIAP_ARG;

typedef struct _danavideocmd_setwifi_arg_s {
    uint32_t ch_no;
    char essid[33];
    char auth_key[65];
    uint32_t enc_type;
} DANAVIDEOCMD_SETWIFI_ARG;

typedef struct _danavideocmd_startaudio_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_STARTAUDIO_ARG;

typedef enum danavideo_audio_track_e_ {
    DANAVIDEO_AUDIO_TRACK_MONO = 1,
    DANAVIDEO_AUDIO_TRACK_STEREO,
} danavideo_audio_track_t;

typedef struct _danavideocmd_starttalkback_arg_s {
    uint32_t ch_no;
    bool has_audio_codec;
    uint32_t audio_codec;
    bool has_sample_rate;
    uint32_t sample_rate;
    bool has_sample_bit;
    uint32_t sample_bit;
    bool has_track;
    uint32_t track;
} DANAVIDEOCMD_STARTTALKBACK_ARG;

typedef struct _danavideocmd_startvideo_arg_s {
    uint32_t ch_no;
    uint32_t client_type;
    uint32_t video_quality;
    uint32_t vstrm;
} DANAVIDEOCMD_STARTVIDEO_ARG;

typedef struct _danavideocmd_stopaudio_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_STOPAUDIO_ARG;

typedef struct _danavideocmd_stoptalkback_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_STOPTALKBACK_ARG;

typedef struct _danavideocmd_stopvideo_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_STOPVIDEO_ARG;


typedef struct _danavideocmd_reclist_arg_s {
    uint32_t ch_no;
    uint32_t get_type;
    uint32_t get_num;
    int64_t last_time;
} DANAVIDEOCMD_RECLIST_ARG;

typedef struct _danavideocmd_recplay_arg_s {
    uint32_t ch_no;
    int64_t time_stamp;
} DANAVIDEOCMD_RECPLAY_ARG;

typedef struct _danavideocmd_recstop_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_RECSTOP_ARG;

typedef struct _danavideocmd_recaction_arg_s {
    uint32_t ch_no;
    uint32_t action;
} DANAVIDEOCMD_RECACTION_ARG;

typedef struct _danavideocmd_recsetrate_arg_s {
    uint32_t ch_no;
    uint32_t rec_rate;
} DANAVIDEOCMD_RECSETRATE_ARG;

typedef struct _danavideocmd_recplanget_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_RECPLANGET_ARG;

typedef struct _danavideocmd_recplanset_arg_s {
    uint32_t ch_no;
    uint32_t record_no;
    size_t week_count;
    uint32_t week[7];   // danavideo_rec_week_t
    char start_time[33];
    char end_time[33];
    uint32_t status;
} DANAVIDEOCMD_RECPLANSET_ARG;


typedef struct _OsdInfo_s {
    uint32_t chan_name_show;
    bool has_show_name_x;
    uint32_t show_name_x;
    bool has_show_name_y;
    uint32_t show_name_y;
    uint32_t datetime_show;
    bool has_show_datetime_x;
    uint32_t show_datetime_x;
    bool has_show_datetime_y;
    uint32_t show_datetime_y;
    bool has_show_format;
    uint32_t show_format;
    bool has_hour_format;
    uint32_t hour_format;
    bool has_show_week;
    uint32_t show_week;
    bool has_datetime_attr;
    uint32_t datetime_attr;
    uint32_t custom1_show;
    bool has_show_custom1_str;
    char show_custom1_str[45];
    bool has_show_custom1_x;
    uint32_t show_custom1_x;
    bool has_show_custom1_y;
    uint32_t show_custom1_y;
    uint32_t custom2_show;
    bool has_show_custom2_str;
    char show_custom2_str[45];
    bool has_show_custom2_x;
    uint32_t show_custom2_x;
    bool has_show_custom2_y;
    uint32_t show_custom2_y;
} OsdInfo_t;

typedef struct _danavideocmd_setosd_arg_s {
    uint32_t ch_no;
    OsdInfo_t osd;
} DANAVIDEOCMD_SETOSD_ARG;

typedef struct _danavideocmd_getosd_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETOSD_ARG;


typedef struct _danavideocmd_setchanname_arg_s {
    uint32_t ch_no;
    char chan_name[128];
} DANAVIDEOCMD_SETCHANNAME_ARG;

typedef struct _danavideocmd_getchanname_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETCHANNAME_ARG;


typedef struct _danavideocmd_callpsp_arg_s {
    uint32_t ch_no;
    uint32_t psp_id;
} DANAVIDEOCMD_CALLPSP_ARG;

typedef struct _danavideocmd_getpsp_arg_s {
    uint32_t ch_no;
    uint32_t page;
    uint32_t page_size;
} DANAVIDEOCMD_GETPSP_ARG;

typedef struct _PspInfo_s {
    uint32_t psp_id;
    char psp_name[60];
    bool psp_default;
    bool is_set;
} PspInfo;

typedef struct _danavideocmd_setpsp_arg_s {
    uint32_t ch_no;
    PspInfo psp;
} DANAVIDEOCMD_SETPSP_ARG;

typedef struct _danavideocmd_setpspdef_arg_s {
    uint32_t ch_no;
    uint32_t psp_id;
} DANAVIDEOCMD_SETPSPDEF_ARG;

typedef struct _danavideocmd_getlayout_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETLAYOUT_ARG;

typedef struct _danavideocmd_setchanadv_arg_s {
    uint32_t ch_no;
    uint32_t matrix_x;
    uint32_t matrix_y;
    size_t chans_count;
    uint32_t chans[64];
} DANAVIDEOCMD_SETCHANADV_ARG;

typedef struct _danavideocmd_seticr_arg_s {
    uint32_t ch_no;
    uint32_t mode;
} DANAVIDEOCMD_SETICR_ARG;

typedef struct _danavideocmd_geticr_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETICR_ARG;

typedef struct _danavideocmd_getbc_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETBC_ARG;

typedef struct {
    size_t size;
    uint8_t bytes[66];
} dana_setbc_bcinfo_bc_filter_t;

typedef struct _dana_setbc_bcinfo {
    uint32_t bc_id;
    bool has_bc_filter;
    dana_setbc_bcinfo_bc_filter_t bc_filter;
} dana_setbc_bcinfo_t;

typedef struct _danavideocmd_setbc_arg_s {
    uint32_t ch_no;
    size_t bc_count;
    dana_setbc_bcinfo_t bc[4];
} DANAVIDEOCMD_SETBC_ARG;


typedef struct _danavideocmd_extendmethod_extend_data_s {
    uint8_t *bytes;
    size_t size;
} danavideocmd_extendmethod_extend_data_t;

typedef struct _danavideocmd_extendmethod_arg_s {
    uint32_t ch_no;
    danavideocmd_extendmethod_extend_data_t extend_data;
} DANAVIDEOCMD_EXTENDMETHOD_ARG;

typedef struct _danavideocmd_getvideo_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETVIDEO_ARG;

//20170315
#if 0 //20170316,暂时不提供
typedef struct _danavideocmd_getbetterystatus_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETBATTERYSTATUS_ARG;
#endif
typedef struct _danavideocmd_getuserpass_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETUSERPASS_ARG;

typedef struct _danavideocmd_setuserpass_arg_s {
    uint32_t ch_no;
    char user_id[60];
    char user_pass[60];
} DANAVIDEOCMD_SETUSERPASS_ARG;

//20170927
typedef struct _danavideocmd_getmotrack_arg_s {
    uint32_t ch_no;
} DANAVIDEOCMD_GETMOTRACK_ARG;
typedef struct _danavideocmd_setmotrack_arg_s {
    uint32_t ch_no;
    uint32_t status;
} DANAVIDEOCMD_SETMOTRACK_ARG;
typedef struct _danavideocmd_controlled_arg_s {
    uint32_t flag;  // 0 means GET; otherwise SET,recommand value is 1
    uint32_t ch_no;
    bool has_onoff;
    uint32_t onoff; // 0 means OFF; otherwise ON, recommand value is 1
} DANAVIDEOCMD_CONTROLLED_ARG;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
