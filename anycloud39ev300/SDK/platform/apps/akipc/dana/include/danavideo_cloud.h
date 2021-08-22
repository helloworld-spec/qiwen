#ifndef DANA_VIDEO_IPC_CLOUD_UTILITY_H
#define DANA_VIDEO_IPC_CLOUD_UTILITY_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _danavideo_cloud_alarm_e {
    DANAVIDEO_CLOUD_ALARM_NO    = 0,    // 无告警 
    DANAVIDEO_CLOUD_ALARM_MD    = 1,    // 视频遮挡       
    DANAVIDEO_CLOUD_ALARM_VB    = 2,    // 移动侦测
    DANAVIDEO_CLOUD_ALARM_UD_1  = 3,    // 自定义告警1
    DANAVIDEO_CLOUD_ALARM_UD_2  = 4,    // 自定义告警2
} danavideo_cloud_alarm_t;

typedef enum _danavideo_clodu_mode_e {
    DANAVIDEO_CLOUD_MODE_UNKNOWN = 0,
    DANAVIDEO_CLOUD_MODE_REALTIME,
    DANAVIDEO_CLOUD_MODE_ALARM,
} danavideo_cloud_mode_t;

uint32_t lib_danavideo_cloud_linked_version(void);
char *lib_danavideo_cloud_linked_version_str(uint32_t version);

// 当云储存计划发生改变时(未开通->开通; 开通->未开通; realtime->alarm; alarm->realtime)
// 用户主要在该回调中完成实时上传和告警上传的开关
typedef void (*danavideo_cloud_mode_changed_callback_t) (const danavideo_cloud_mode_t cloud_mode, const int32_t chan_no);
void lib_danavideo_cloud_set_cloud_mode_changed(danavideo_cloud_mode_changed_callback_t fun);

// 设置通道总数和允许使用的最大内存(最小1M)
bool lib_danavideo_cloud_init(const uint32_t chan_num, const int32_t maximum_buffering_data, const int32_t package_size, danavideo_cloud_mode_t mode);

// 实时上传
bool lib_danavideo_cloud_realtime_on();

bool lib_danavideo_cloud_realtime_off();

// msg_type采用danavdieo.h中的定义(仅支持audio和video)
/*
 * typedef enum _danavideo_msg_type {
 *    audio_stream = 0x20000000,
 *    video_stream = 0x40000000,
 *    extend_data   = 0x60000000,
 *    pic_stream   = 0x80000000,
 * } danavideo_msg_type_t;
 */
// codec采用danavideo.h中的定义
/*
 * typedef enum _danavideo_codec_type {
 *     H264    = 1,
 *     MPEG    = 2,
 *     MJPEG   = 3,
 *     H265    = 4,
 *     H265_HISILICON    = 5,
 *     MJPEG_DIFT        = 6,
 *     G711A   = 101,
 *     ULAW    = 102,
 *     G711U   = 103,
 *     PCM     = 104,
 *     ADPCM   = 105,
 *     G721    = 106,
 *     G723    = 107,
 *     G726_16 = 108,
 *     G726_24 = 109,
 *     G726_32 = 110,
 *     G726_40 = 111,
 *     AAC     = 112,
 *     JPG     = 200,
 * } danavidecodec_t;
 */
bool lib_danavideo_cloud_realtime_upload(const uint32_t chan_no, const uint32_t msg_type, const uint32_t codec, const uint32_t is_keyfram, const uint32_t timestamp, const danavideo_cloud_alarm_t alarm, const char *data, const uint32_t data_len, const uint32_t timeout_usec);

// 告警上传 & 文件上传
// 文件上传就是注册一个上传任务,大拿云存储库会自动完成上传动作
// 当上传完毕(成功或失败)会调用该回调,用于通知用户是哪个本地文件,方便用户的后续操作(比如删除文件)
typedef void (*danavideo_cloud_customfile_upload_callback_t) (const int8_t retcode, const char *file_name, const char *file_path);

// 注册一个本地文件上传
// 调用该接口后save_site & save_path可以用于lib_danavideo_util_pushmsg
bool lib_danavideo_cloud_customfile_async_upload_pre(const uint32_t ch_no, const char *file_name, const char *file_path, uint32_t *save_site, char *save_path, const size_t save_path_len);
// 用于告警上传时,比如本地录制了一个视频文件(目前仅支持mp4文件)
// 当上传完毕后会调用lib_danavideo_cloud_realtime_upload回调,可以删除告警录像文件以节省空间
bool lib_danavideo_cloud_customfile_async_upload(const uint32_t ch_no, const char *file_name, const char *file_path, danavideo_cloud_customfile_upload_callback_t fun);

bool lib_danavideo_cloud_deinit();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
