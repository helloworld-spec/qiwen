#ifndef DANA_VIDEO_H
#define DANA_VIDEO_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _dana_video_cmd_e {
    DANAVIDEOCMD_DEVDEF,		// 0
    DANAVIDEOCMD_DEVREBOOT, 	// 1
    DANAVIDEOCMD_GETSCREEN,		// 2
    DANAVIDEOCMD_GETALARM,		// 3
    DANAVIDEOCMD_GETBASEINFO,	// 4
    DANAVIDEOCMD_GETCOLOR,		// 5
    DANAVIDEOCMD_GETFLIP,		// 6
    DANAVIDEOCMD_GETFUNLIST,	// 7
    DANAVIDEOCMD_GETNETINFO,	// 8
    DANAVIDEOCMD_GETPOWERFREQ,	// 9
    DANAVIDEOCMD_GETTIME,		// 10
    DANAVIDEOCMD_GETWIFIAP,		// 11
    DANAVIDEOCMD_GETWIFI,		// 12
    DANAVIDEOCMD_PTZCTRL,		// 13
    DANAVIDEOCMD_SDCFORMAT,		// 14
    DANAVIDEOCMD_SETALARM,		// 15
    DANAVIDEOCMD_SETCHAN,		// 16
    DANAVIDEOCMD_SETCOLOR,		// 17
    DANAVIDEOCMD_SETFLIP,		// 18
    DANAVIDEOCMD_SETNETINFO,	// 19
    DANAVIDEOCMD_SETPOWERFREQ,	// 20
    DANAVIDEOCMD_SETTIME,		// 21
    DANAVIDEOCMD_SETVIDEO,		// 22
    DANAVIDEOCMD_SETWIFIAP,		// 23
    DANAVIDEOCMD_SETWIFI,		// 24
    DANAVIDEOCMD_STARTAUDIO,	// 25
    DANAVIDEOCMD_STARTTALKBACK,	// 26
    DANAVIDEOCMD_STARTVIDEO,	// 27
    DANAVIDEOCMD_STOPAUDIO, 	// 28
    DANAVIDEOCMD_STOPTALKBACK,	// 29
    DANAVIDEOCMD_STOPVIDEO,		// 30
    DANAVIDEOCMD_RECLIST,		// 31
    DANAVIDEOCMD_RECPLAY,		// 32
    DANAVIDEOCMD_RECSTOP,		// 33
    DANAVIDEOCMD_RECACTION,		// 34
    DANAVIDEOCMD_RECSETRATE,	// 35
    DANAVIDEOCMD_RECPLANGET,	// 36
    DANAVIDEOCMD_RECPLANSET,	// 37
    DANAVIDEOCMD_EXTENDMETHOD,	// 38
    DANAVIDEOCMD_SETOSD,		// 39
    DANAVIDEOCMD_GETOSD,		// 40
    DANAVIDEOCMD_SETCHANNAME,	// 41
    DANAVIDEOCMD_GETCHANNAME,	// 42

    DANAVIDEOCMD_RESOLVECMDFAILED,// 43

    DANAVIDEOCMD_CALLPSP = 66,
    DANAVIDEOCMD_GETPSP,
    DANAVIDEOCMD_SETPSP,
    DANAVIDEOCMD_SETPSPDEF,
    
    DANAVIDEOCMD_GETLAYOUT = 70,
    DANAVIDEOCMD_SETCHANADV,

    DANAVIDEOCMD_SETICR = 78,
    DANAVIDEOCMD_GETICR,

    DANAVIDEOCMD_SETBC = 80,
    DANAVIDEOCMD_GETBC = 81,

    DANAVIDEOCMD_GETSDCSTATUS = 82,
    DANAVIDEOCMD_GETVIDEO  = 83,

    DANAVIDEOCMD_GETUSERPASS = 85,
    DANAVIDEOCMD_SETUSERPASS,
    DANAVIDEOCMD_CONTROLLED = 87,
    //20170927
    DANAVIDEOCMD_GETMOTRACK = 90,
    DANAVIDEOCMD_SETMOTRACK = 91,

} dana_video_cmd_t;

typedef void* pdana_video_conn_t;

typedef struct _dana_video_callback_funs_s {
    uint32_t (*danavideoconn_created)(pdana_video_conn_t danavideoconn);
    void (*danavideoconn_aborted)(pdana_video_conn_t danavideoconn);
    void (*danavideoconn_command_handler)(pdana_video_conn_t danavideoconn, dana_video_cmd_t cmd, uint32_t trans_id, void* cmd_arg, uint32_t cmd_arg_len); 

} dana_video_callback_funs_t;

// called when ipc recved p2pserver's heartbeat response
typedef void (*danavideo_hb_is_ok_callback_t) (void);

// called when ipc check that the conn with p2p2server is not avliable
typedef void (*danavideo_hb_is_not_ok_callback_t) (void);

// called when need upgrade rom
typedef void (*danavideo_upgrade_rom_callback_t) (const char* rom_path,  const char *rom_md5, const uint32_t rom_size); // 单位 Byte

// called when need autoset
typedef void (*danavideo_autoset_callback_t) (const uint32_t power_freq, const int64_t now_time, const char *time_zone, const char *ntp_server1, const char *ntp_server2);
// 电源频率 0 50Hz,  1 60Hz
// now_time 单位s
// time_zone 时区
// NTP 服务器1 2


// called when need setwifi
// setwifi有四种情况,一种是通过command_handler获取该命令
//                   一种是通过回调函数(本地模式)
//                   还有智能声控模式
//                   DanaAirLink
//              ip_type  0: fixed 1:dhcp
//              enc_type 见danavideo_cmd.h
typedef void (*danavideo_local_setwifiap_callback_t) (const uint32_t ch_no, const uint32_t ip_type, const char *ipaddr, const char *netmask, const char *gateway, const char *dns_name1, const char *dns_name2, const char *essid, const char *auth_key, const uint32_t enc_type);

// called when need local auth
// 0 succeeded
// 1 failed
typedef uint32_t (*danavideo_local_auth_callback_t) (const char *user_name, const char *user_pass);

// called when create a new conf or update a exited conf
typedef void (*danavideo_conf_created_or_updated_t) (const char *conf_absolute_pathname);

// called when need get the connected wifi wifi-quality [0-100] (if no wifi set to 0)
typedef uint32_t (*danavideo_get_connected_wifi_quality_callback_t) ();

// called when recv productsetdeviceinfo 
typedef void (*danavideo_productsetdeviceinfo_callback_t) (const char *model, const char *sn, const char *hw_mac);

typedef enum _danavideo_msg_type {
   audio_stream = 0x20000000,
   video_stream = 0x40000000,
   extend_data   = 0x60000000,
   pic_stream   = 0x80000000,
   dana_data      = 0xa0000000,
} danavideo_msg_type_t;

typedef enum _danavideo_codec_type {
    H264    = 1,
    MPEG    = 2,
    MJPEG   = 3,
    H265    = 4,
    H265_HISILICON    = 5,
    MJPEG_DIFT        = 6,
    G711A   = 101,
    ULAW    = 102,
    G711U   = 103,
    PCM     = 104,
    ADPCM   = 105,
    G721    = 106,
    G723    = 107,
    G726_16 = 108,
    G726_24 = 109,
    G726_32 = 110,
    G726_40 = 111,
    AAC     = 112,
    JPG     = 200,
} danavidecodec_t;

typedef struct _dana_packet_s {
    char    *data;
    int32_t   len;
} dana_packet_t;

typedef struct _dana_audio_packet_s {
    char  *data;
    int32_t len;
    danavidecodec_t  codec; // 音频编码
} dana_audio_packet_t;


// 用于设置视频发送缓冲
// 默认2M,最低2M
void lib_danavideo_set_maximum_buffering_data_size(const int32_t size);

// 用于设置lib_danavideo_start超时时间,
void lib_danavideo_set_startup_timeout(const uint32_t timeout_sec);

void lib_danavideo_set_hb_is_ok(danavideo_hb_is_ok_callback_t fun);
void lib_danavideo_set_hb_is_not_ok(danavideo_hb_is_not_ok_callback_t fun);

void lib_danavideo_set_upgrade_rom(danavideo_upgrade_rom_callback_t fun);

void lib_danavideo_set_autoset(danavideo_autoset_callback_t fun);

void lib_danavideo_set_local_setwifiap(danavideo_local_setwifiap_callback_t fun);

void lib_danavideo_set_local_auth(danavideo_local_auth_callback_t fun);

void lib_danavideo_set_conf_created_or_updated(danavideo_conf_created_or_updated_t fun);

void lib_danavideo_set_get_connected_wifi_quality(danavideo_get_connected_wifi_quality_callback_t fun);

void lib_danavideo_set_productsetdeviceinfo(danavideo_productsetdeviceinfo_callback_t fun);

uint32_t lib_danavideo_linked_version(void);
char * lib_danavideo_linked_version_str(uint32_t version);

char * lib_danavideo_deviceid_from_conf(const char *danale_path);

char * lib_danavideo_deviceid();

// ***本地监听端口***
// 在lib_danavideo_start之前 调用 则会采用设置的端口(固定模式下 或者非固定模式下自由选择)
// 在lib_danavideo_start之后 调用 则默认会启动34102 然后判断设置端口是否和34102相同 如果不相同则会关闭34102 启动用户设置的端口,否则直接返回

void lib_danavideo_set_listen_port(const bool listen_port_fixed, const uint16_t listen_port);
uint16_t lib_danavideo_get_listen_port(); 


// 创建配置文件
// 必须在lib_danavideo_init调用之后才可以调用
// 如果指定的目录下配置文件存在并检测有效，则不会创建
// 否则会重新创建一个配置文件
// 非阻塞
bool lib_danavideo_create_on_check_conf();

// 更新发送数据接口 
// 增加lib_danavideoconn_send 
// 取消danavideo_msg_max_size
// 取消lib_danavideo_packet_create
//     lib_danavideo_packet_destroy
//     lib_danavideoconn_sendvideomsg
//     lib_danavideoconn_sendaudiomsg
//     lib_danavideoconn_sendpicmsg
// 更新danavideo_msg_type_t 将rec_stream变更为extend_data 以支持extend_data的发送
// 支持发送video audio pic extend_data
bool lib_danavideoconn_send(pdana_video_conn_t danavideoconn, const danavideo_msg_type_t msg_type, const danavidecodec_t codec, const uint8_t ch_no, const uint8_t is_keyframe, const uint32_t timestamp, const char* data, const uint32_t data_len, const uint32_t timeout_usec);

typedef void (*danadata_read_callback_t)(pdana_video_conn_t danavideoconn, const char *data, const int32_t len);

void lib_danadata_set_read(danadata_read_callback_t fun);

bool lib_danadataconn_send(pdana_video_conn_t danavideoconn, const danavideo_msg_type_t msg_type, const char* data, const uint32_t data_len, const uint32_t timeout_usec);


dana_audio_packet_t* lib_danavideoconn_readaudio(pdana_video_conn_t danavideoconn, uint32_t timeout_usec); // call lib_danavideo_audio_packet_destroy to free
void lib_danavideo_audio_packet_destroy(dana_audio_packet_t *danaaudiomsg);

// lib_danavideo_init不再自动创建配置文件 当配置文件不存在(或无效)时直接返回false
bool lib_danavideo_init(const char *danale_path, const char *agent_user, const char *agent_pass, const char *chip_code, const char *scheme_code, const char *product_code, dana_video_callback_funs_t *danavideocallbackfuns);

uint32_t lib_danavideo_set_userdata(pdana_video_conn_t danavideoconn, void *userdata);

uint32_t lib_danavideo_get_userdata(pdana_video_conn_t danavideoconn, void **userdata);

bool lib_danavideo_start();

bool lib_danavideo_cmd_exec_response(pdana_video_conn_t danavideoconn, const dana_video_cmd_t cmd, char* error_method, const uint32_t trans_id, const uint32_t code, const char* code_msg); 

bool lib_danavideo_cmd_setvideo_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t fps);
bool lib_danavideo_cmd_startvideo_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t fps);
// audio_codec 默认 G711A  (danavidecodec_t)
// sample_rate 默认 8000Hz
// sample_bit  默认 16bit
// track       默认 mono   (1 mono; 2 stereo)
bool lib_danavideo_cmd_startaudio_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t *audio_codec, const uint32_t *sample_rate, const uint32_t *sample_bit, const uint32_t *track);
bool lib_danavideo_cmd_getalarm_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t motion_detection, const uint32_t opensound_detection, const uint32_t openi2o_detection, const uint32_t smoke_detection, const uint32_t shadow_detection, const uint32_t other_detection, const uint32_t *pir_detection);
bool lib_danavideo_cmd_getbaseinfo_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const char *dana_id, const char *api_ver, const char *sn, const char *device_name, const char *rom_ver, const uint32_t device_type, const uint32_t ch_num, const uint64_t sdc_size, const uint64_t sdc_free, const size_t work_channel_count, const uint32_t *work_channel); // dana_id < 49; api_ver < 129; sn < 129; device_name < 129; rom_ver < 129 work_channel <=48// DONE
bool lib_danavideo_cmd_getcolor_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t brightness, const uint32_t contrast, const uint32_t saturation, const uint32_t hue);
bool lib_danavideo_cmd_getflip_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t flip_type);
bool lib_danavideo_cmd_getfunlist_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t methodes_count, const char **methodes); // methodes_count <= 160
bool lib_danavideo_cmd_getnetinfo_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t ip_type, const char *ipaddr, const char *netmask, const char *gateway, const uint32_t dns_type, const char *dns_name1, const char *dns_name2, const uint32_t http_port); // ipaddr < 16; netmask < 16; gataway < 16; dns_type < 10; dns_name1 < 16; dns_name2 < 16
bool lib_danavideo_cmd_getpowerfreq_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t freq);
bool lib_danavideo_cmd_gettime_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const int64_t now_time, const char *time_zone, const char *ntp_server_1, const char *ntp_server_2); // time_zone < 65

typedef struct _libdanavideo_wifiinfo_s {
    char essid[33];
    uint32_t enc_type;
    uint32_t quality;
} libdanavideo_wifiinfo_t;

bool lib_danavideo_cmd_getwifiap_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t wifi_device, const uint32_t wifi_list_count, const libdanavideo_wifiinfo_t *wifi_list); // wifi_list_count <= 20;  essid < 33
bool lib_danavideo_cmd_getwifi_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const char *essid, const char *auth_key, const uint32_t enc_type); // essid < 33; auth_key < 65
bool lib_danavideo_cmd_starttalkback_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t audio_codec, const uint32_t * sample_rate, const uint32_t *sample_bit, const uint32_t *track);


typedef struct _libdanavideo_reclist_recordinfo_s {
    int64_t start_time;
    uint32_t length;
    uint32_t record_type;
} libdanavideo_reclist_recordinfo_t;

bool lib_danavideo_cmd_reclist_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t rec_lists_count, const libdanavideo_reclist_recordinfo_t *rec_lists); // rec_lists_count <= 35

typedef struct _libdanavideo_recplanget_recplan_s {
    uint32_t record_no;
    size_t week_count; // <= 7  表示week数组中有几天
    uint32_t week[7];
    char start_time[33]; // 时:分:秒 12:12:12
    char end_time[33];
    uint32_t status;
} libdanavideo_recplanget_recplan_t;

bool lib_danavideo_cmd_recplanget_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t rec_plans_count, const libdanavideo_recplanget_recplan_t *rec_plans); // rec_plans_count <= 3

typedef struct _libdanavideo_osdinfo_s {
    uint32_t chan_name_show;
    uint32_t show_name_x;
    uint32_t show_name_y;

    uint32_t datetime_show;
    uint32_t show_datetime_x;
    uint32_t show_datetime_y;
    uint32_t show_format;
    uint32_t hour_format;
    uint32_t show_week;
    uint32_t datetime_attr;

    uint32_t custom1_show;
    char     show_custom1_str[45];
    uint32_t show_custom1_x;
    uint32_t show_custom1_y;

    uint32_t custom2_show;
    char     show_custom2_str[45];
    uint32_t show_custom2_x;
    uint32_t show_custom2_y;
} libdanavideo_osdinfo_t;

bool lib_danavideo_cmd_getosd_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char * code_msg, const libdanavideo_osdinfo_t *osdinfo);

bool lib_danavideo_cmd_getchanname_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char * code_msg, const char *chan_name);

typedef struct _libdanavideo_pspinfo_s {
    uint32_t psp_id;
    char psp_name[60];
    bool psp_default;
    bool is_set;
} libdanavideo_pspinfo_t;

bool lib_danavideo_cmd_getpsp_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char * code_msg, const uint32_t total, const uint32_t psp_count, const libdanavideo_pspinfo_t *psp); // psp_count <=10

bool lib_danavideo_cmd_getlayout_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char * code_msg, const uint32_t matrix_x, const uint32_t matrix_y, const size_t chans_count, const uint32_t chans[64], const uint32_t layout_change, const uint32_t chan_pos_change, const size_t use_chs_count, const uint32_t use_chs[98]);

bool lib_danavideo_cmd_geticr_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char * code_msg, const uint32_t mode);

typedef struct dana_bcfilter_t {
    size_t size;
    uint8_t bytes[66];
} dana_bcfilter_t;

typedef struct _dana_bcinfo_s {
    uint32_t bc_id;
    bool has_bc_filter;
    dana_bcfilter_t bc_filter;
} dana_bcinfo_t;

bool lib_danavideo_cmd_getbc_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char * code_msg, const size_t bc_count, const dana_bcinfo_t bc[4]);

bool lib_danavideo_cmd_getsdcstatus_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char * code_msg, const uint32_t status, const uint32_t *format_progress, const uint32_t *sd_size, const uint32_t *sd_free);
bool lib_danavideo_cmd_getvideo_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char * code_msg, const uint32_t video_quality);

bool lib_danavideo_cmd_getuserpass_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const char *user_id, const char *user_pass);
//20170927
bool lib_danavideo_cmd_getmotrack_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t status);
bool lib_danavideo_cmd_controlled_response(pdana_video_conn_t danavideoconn, const uint32_t trans_id, uint32_t code, const char* code_msg, const uint32_t onoff);

//危险接口谨慎使用
bool lib_danavideo_setdanaconn_kernel_buffer(pdana_video_conn_t danavideoconn, const uint32_t buffer_size);

bool lib_danavideo_stop();

bool lib_danavideo_deinit();


/////////////////////////////////////// 工具类 ////////////////////////////////////////

typedef enum _danavideo_device_type_e {
    DANAVIDEO_DEVICE_IPC = 1,
    DANAVIDEO_DEVICE_DVR,
    DANAVIDEO_DEVICE_NVR,
    DANAVIDEO_DEVICE_DVR_NO_MIX_NO_MULTI_CHANNEL, // 不支持0通道(多路视频合成), 不支持多路视频同时传输
    DANAVIDEO_DEVICE_NVR_NO_MIX_NO_MULTI_CHANNEL, // 不支持0通道(多路视频合成), 不支持多路视频同时传输
    DANAVIDEO_DEVICE_DVR_NO_MIX_MULTI_CHANNEL,   // 不支持0通道(多路视频合成), 支持多路视频同时传输
    DANAVIDEO_DEVICE_NVR_NO_MIX_MULTI_CHANNEL,   // 不支持0通道(多路视频合成), 支持多路视频同时传输
    DANAVIDEO_DEVICE_DVR_SPLIT,     // 支持0通道(多路视频合成),支持不规则分屏
    DANAVIDEO_DEVICE_NVR_SPLIT,     // 支持0通道(多路视频合成),支持不规则分屏

//对外发布取消
#if 0
    DANA_DEVICE_RING = 60004,   // ring设备类型
    DANA_DEVICE_GARAGE_DOOR_OPENER_WITH_CAMERA = 60005, // 可视车库门
#endif
    DANA_DEVICE_NAS = 60006,	

    DANAVIDEO_DEVICE_Unknown_type = -1,
} danavideo_device_type_t;

bool lib_danavideo_util_setdeviceinfo(const danavideo_device_type_t device_type, const uint32_t channel_num, const char *rom_ver, const char *api_ver, const char *rom_md5); // rom_ver < 129; api_ver < 129; rom_md5 < 65

typedef enum _dana_video_feature_e {
    // 硬件特性
    DANAVIDEO_FEATURE_HAVE_BATTERY          = 4097, // 电池
    DANAVIDEO_FEATURE_HAVE_GPS              = 4098, // GPS
    DANAVIDEO_FEATURE_HAVE_PIR              = 4099, // PIR 红外  (烟雾 可能 rf433)
    DANAVIDEO_FEATURE_HAVE_G_SENSOR         = 4100, // 加速度传感器
    DANAVIDEO_FEATURE_HAVE_GYRO_SENSOR      = 4101, // 陀螺仪传感器
    DANAVIDEO_FEATURE_HAVE_TEMP_SENSOR      = 4102, // 温度传感器
    DANAVIDEO_FEATURE_HAVE_HUMIDITY_SENSOR  = 4103, // 湿度传感器
    DANAVIDEO_FEATURE_HAVE_MOBILENET        = 4104, // 3G/4G支持
    DANAVIDEO_FEATURE_HAVE_PTZ_L_R          = 4105, // 云台支持(左右)
    DANAVIDEO_FEATURE_HAVE_PTZ_U_D          = 4106, // 云台支持(上下)
    DANAVIDEO_FEATURE_HAVE_PTZ_L_R_U_D      = 4107, // 云台支持(上下,左右)
    DANAVIDEO_FEATURE_HAVE_PTZ_DD           = 4108, // 云台支持(8向支持)
    DANAVIDEO_FEATURE_HAVE_WIPER            = 4109, // 雨刷
    DANAVIDEO_FEATURE_HAVE_ZOOM_LENS        = 4110, // 变焦 ?
    DANAVIDEO_FEATURE_HAVE_ENLARGING_LENS   = 4111, // 变倍 ?
    DANAVIDEO_FEATURE_HAVE_SD               = 4112, // SD卡支持
    DANAVIDEO_FEATURE_HAVE_MIC              = 4113, // MIC支持
    DANAVIDEO_FEATURE_HAVE_SPEAKER          = 4114, // 对讲支持(喇叭)
    DANAVIDEO_FEATURE_HAVE_BLE_HEADSET      = 4115, // 蓝牙耳机支持
    DANAVIDEO_FEATURE_HAVE_RF315            = 4116, // 支持RF315
    DANAVIDEO_FEATURE_HAVE_RF433            = 4117, // 支持RF433
    DANAVIDEO_FEATURE_HAVE_RF866            = 4118, // 支持RF866
    DANAVIDEO_FEATURE_HAVE_ZIGBEE           = 4119, // 支持Zigbee
    DANAVIDEO_FEATURE_HAVE_BELL             = 4120, // 门铃
    DANAVIDEO_FEATURE_HAVE_2_WAY_VOICE      = 4121, // 双向语音对讲[回声消除] 
    DANAVIDEO_FEATURE_HAVE_PSP              = 4138, // 支持预置点 
    DANAVIDEO_FEATURE_HAVE_FISHEYE_INSCRIBCIRCLE     = 4139, // 支持鱼眼  inscribed circle 内切圆
    DANAVIDEO_FEATURE_HAVE_1_WAY_VOICE               = 4146, //单向语音对讲
    DANAVIDEO_FEATURE_HAVE_FISHEYE_CIRCUMCIRCLE      = 4147, // 支持捷高鱼眼2 circumcircle 外切圆
    DANAVIDEO_FEATURE_HAVE_SOUND_ALARM      = 4149, // 声音侦测告警(SOUND DETECTION ALARM)
    DANAVIDEO_FEATURE_HAVE_MOTION_ALARM     = 4150, // 支持移动侦测告警-非门铃(MOTION DETECTION ALARM)

    // 扩展功能
    DANA_VIDEO_HAVE_CLOUD_STORAGE           = 8193, // 云存储
    DANA_VIDEO_HAVE_PERSONAL_STORAGE        = 8194, // 个人存储
    DANA_VIDEO_HAVE_SMART_HOME              = 8195, // 智能家居中控功能(具有管理家居智能安防设备的功能)
    DANA_VIDEO_HAVE_UPGRADE                 = 8196, // 固件升级功能
    DANA_VIDEO_HAVE_UPGRADE_OTA             = 8200, // 固件升级功能OTA 原有8196为LOCAL
    DANA_VIDEO_HAVE_VOICE_OPTIMIZED_MODE    = 8201, //支持音频优化特性
} dana_video_feature_t;

bool lib_danavideo_util_setdevicefeature(const size_t feature_list_count, const dana_video_feature_t feature_list[152]); // feature_list <= 152


// 上报信息
// 添加告警消息等级选项  alarm_level
// 规范定义告警消息类型  msg_type
typedef enum _dana_video_pushmsg_alarm_level_e {
   DANA_VIDEO_PUSHMSG_ALARM_LEVEL_1 = 1,    //  低
   DANA_VIDEO_PUSHMSG_ALARM_LEVEL_2 = 2,    //  中
   DANA_VIDEO_PUSHMSG_ALARM_LEVEL_3 = 3,    //  高
} dana_video_pushmsg_alarm_level_t;
typedef enum _dana_video_pushmsg_msg_type_e {
    DANA_VIDEO_PUSHMSG_MSG_TYPE_MOTION_DETECT   = 1,    //  移动侦测
    DANA_VIDEO_PUSHMSG_MSG_TYPE_SOUND_DETECT    = 2,    //  声音侦测
    DANA_VIDEO_PUSHMSG_MSG_TYPE_IR              = 3,    //  红外
    DANA_VIDEO_PUSHMSG_MSG_TYPE_OTHER           = 4,    //  其他
    DANA_VIDEO_PUSHMSG_MSG_TYPE_BODY_SENSOR     = 10,   //  人体感应
    DANA_VIDEO_PUSHMSG_MSG_TYPE_SMOKE_SENSOR    = 11,   //  烟雾探测
    DANA_VIDEO_PUSHMSG_MSG_TYPE_DOOR_SENSOR     = 12,   //  门磁打开
    DANA_VIDEO_PUSHMSG_MSG_TYPE_GLASS_BREAK     = 13,   //  玻璃破碎
    DANA_VIDEO_PUSHMSG_MSG_TYPE_COMBUSTIBLEGAS_EXCEEDED = 14,   // 可燃气体超标
    DANA_VIDEO_PUSHMSG_MSG_TYPE_DOOR_BELL       = 15,   // 门铃被触发/ 需要上报云存储录像
    DANA_VIDEO_PUSHMSG_MSG_TYPE_DEMOLITION      = 16,   // 拆毁报警
    DANA_VIDEO_PUSHMSG_MSG_TYPE_LOW_BATTERY     = 17,   // 低电量报警
    DANA_VIDEO_PUSHMSG_MSG_TYPE_PASSWD_INCORRECT = 18,   // 密码错误告警
    DANA_VIDEO_PUSHMSG_MSG_TYPE_SOS             = 19,   // SOS
    DANA_VIDEO_PUSHMSG_MSG_TYPE_WATERLOGGING    = 20,   // 水渍告警
    DANA_VIDEO_PUSHMSG_MSG_TYPE_DEV_OFFLINE     = 21,   // 设备离线告警
    DANA_VIDEO_PUSHMSG_MSG_TYPE_DEV_ONLINE      = 22,   // 上线提示
    DANA_VIDEO_PUSHMSG_MSG_TYPE_BATTERY_POWERED = 23,   // 设备使用电池提示信息
    DANA_VIDEO_PUSHMSG_MSG_TYPE_SENSOR_DETECT   = 31,   // 探头告警
    DANA_VIDEO_PUSHMSG_MSG_TYPE_VLOST           = 32,   // 视频丢失告警
    DANA_VIDEO_PUSHMSG_MSG_TYPE_VMASK           = 33,   // 视频遮挡告警
    DANA_VIDEO_PUSHMSG_MSG_TYPE_DISKFULL        = 34,   // 磁盘满告警
    DANA_VIDEO_PUSHMSG_MSG_TYPE_DISKERR         = 35,   // 磁盘错误告警
    DANA_VIDEO_PUSHMSG_MSG_TYPE_DISK_NO_FORMAT  = 36,   // 磁盘未格式化告警
    DANA_VIDEO_PUSHMSG_MSG_TYPE_GARAGE_DOOR_TOGGLE  =  41,  // 车库门状态切换[开关状态未知]
    DANA_VIDEO_PUSHMSG_MSG_TYPE_GARAGE_DOOR_CLOSE   =  42,  // 车库门关闭 
    DANA_VIDEO_PUSHMSG_MSG_TYPE_GARAGE_DOOR_OPEN    =  43,  // 车库门打开
    DANA_VIDEO_PUSHMSG_MSG_TYPE_DOOR_BELL_PUSH      =  51,   // 门铃被按下/ 与云存储无法,仅用于触发App打开呼叫界面
    DANA_VIDEO_PUSHMSG_MSG_TYPE_SYS_MSG         = 99,   //  系统消息(软件升级等)
} dana_video_pushmsg_msg_type_t;


bool lib_danavideo_cloud_realtime_lastsavepath(const uint32_t chan_no, const int64_t cur_time, char *save_path, const size_t save_path_len, uint32_t *offset);

bool lib_danavideo_util_pushmsg(const uint32_t ch_no, const uint32_t alarm_level, const uint32_t msg_type, const char *msg_title, const char *msg_body, const int64_t cur_time, const uint32_t att_flag, const char *att_path, const char *att_type, const uint32_t record_flag, const int64_t start_time, const uint32_t time_len, const uint32_t save_site, const char *save_path);

bool lib_danavideo_local_searchapp(const char *check_data, const uint32_t encrypt_flag);


// 智能声控
void lib_danavideo_smart_conf_init();
void lib_danavideo_smart_conf_parse(const int16_t *audio, int32_t size); // PCM 音频数据: 16位有符号数，采样频率必须为48000 或者 44100
// 最新版本需要44100,且支持双向通信

// 用户直接播放该声音即
//
// 需要注意,用户需先设置danale配置文件路径
// 或者执行一次lib_danavideo_init
void lib_danavideo_smart_conf_set_danalepath(const char *danale_path);

// 设置音量,这个需要各个厂家在自己的板子上调试  推荐值是0x2400 不超过0x8000
void lib_danavideo_smart_conf_set_volume(const uint32_t volume);

typedef void (*danavideo_smart_conf_response_callback_t) (const char *audio, size_t size);
void lib_danavideo_set_smart_conf_response(danavideo_smart_conf_response_callback_t fun);



// DanaAirLink
// 需要先设置danale配置文件路径(或者执行一次lib_danavideo_init) 
bool danaairlink_set_danalepath(const char *danale_path);

typedef enum _danaairlink_chip_type_e {
    DANAAIRLINK_CHIP_TYPE_NORMAL,
    DANAAIRLINK_CHIP_TYPE_MT7601,
    DANAAIRLINK_CHIP_TYPE_RLT8188,
    DANAAIRLINK_CHIP_TYPE_AP6212,
    DANAAIRLINK_CHIP_TYPE_RTL8189,
    DANAAIRLINK_CHIP_TYPE_RT5572,
    DANAAIRLINK_CHIP_TYPE_RTL8811,
    DANAAIRLINK_CHIP_TYPE_MT7610,
    DANAAIRLINK_CHIP_TYPE_MRL8801,
    DANAAIRLINK_CHIP_TYPE_MT7628,
    DANAAIRLINK_CHIP_TYPE_UNKNOW = -1,
} danaairlink_chip_type_t;
bool danaairlink_init(const danaairlink_chip_type_t chip_type, const char *if_name);

// 每次需要进入DanaAirLink配置状态就需要调用一次该方法
// 也即当回调函数(danavideo_local_setwifiap_callback_t)被调用了,需要再次进入配置状态时需调用该方法
bool danaairlink_start_once(); 
bool danaairlink_stop();
bool danaairlink_deinit();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
