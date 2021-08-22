#ifndef __TX_AUDIO_VIDEO_H__
#define __TX_AUDIO_VIDEO_H__

#include "TXSDKCommonDef.h"

CXX_EXTERN_BEGIN


/**
 *
 * 音视频相关接口
 *
 */


/*
 * 收发音频数据流时的参数
 * head_length   : 结构体的长度.
 * audio_format  ：表示音频编码的格式，也就是音频压缩算法，受限于手机QQ安装包体积的限制，目前只支持固定值： 1
 * encode_param  ：表示音频压缩算法的具体参数，建议值： 7
 * frame_per_pkg ：音频数据是一个数据包一个数据包发送到手机端的，如果一帧音频数据很小，那么单个数据包中可以装n个音频数据帧，这个参数用来指定n，建议值：8
 * sampling_info ：采样参数 value = ((通道数 << 24) | (采样率 << 16) | (位宽 << 8)| 0x00)，建议值：GET_SIMPLING_INFO(1, 8, 16) = 17305600
 * reserved      ：保留字段
 */
typedef struct _tx_audio_encode_param
{
	unsigned char head_length;
	unsigned char audio_format;
	unsigned char encode_param;
	unsigned char frame_per_pkg;
	unsigned int  sampling_info;
	unsigned int  reserved;
	
} tx_audio_encode_param;


/**
* 下面的宏来帮助您计算一个用来填入 tx_audio_encode_param - sampling_info 中的数值
*/
#define GET_SIMPLING_INFO(channel, sampling, bit)  ((channel << 24) | (sampling << 16) | (bit << 8) | 0x00)


/**
 * AV SDK -> Device 回调接口
 *
 * on_start_camera 通知摄像头视频链路已经建立，可以通过 tx_set_video_data 接口向 AV SDK 填充采集到的视频数据
 *
 * on_stop_camera  通知摄像头视频链路已经断开，可以不用再继续采集视频数据
 *
 * on_set_bitrate  视频码率意味着1s产生多少数据，这个参数跟网络带宽的使用直接相关。
 *                 AV SDK 会根据当前的网络情况和Qos信息，给出建议的bitrate，上层应用可以根据这个建议值设置Camera的各项参数，
 *                 如帧率、分辨率，量化参数等，从而获得合适的码率
 *
 * on_start_mic    通知麦克风音频链路已经建立，可以通过 tx_set_audio_data 接口向 AV SDK 填充采集到的音频数据
 *
 * on_stop_mic     通知摄像头音频链路已经断开，可以不用再继续采集音频数据
 *
 * on_recv_audiodata 通知智能设备，有音频数据到达
*/
typedef struct _tx_av_callback
{
    //智能设备作为音视频的发送方，下面回调用于采集音视频数据
    bool (*on_start_camera)();
    bool (*on_stop_camera)();

    bool (*on_set_bitrate)(int bit_rate);

    bool (*on_start_mic)();
    bool (*on_stop_mic)();

    /*
     * 智能设备作为音视频的接收方，下面回调用于接收音视频数据
     * 注意：on_recv_audiodata回调出来的tx_audio_encode_param参数里面包含了音频的编码参数
     */
    void (*on_recv_audiodata)(tx_audio_encode_param *param, unsigned char *pcEncData, int nEncDataLen);
} tx_av_callback;


/**
* 接口说明：启动音视频相关服务，该服务需要登录成功后才能调用，否则会有错误码返回
* 参数说明：callback  音视频回调接口
* 返回值  ：错误码（见全局错误码表）
*/
SDK_API int tx_start_av_service( tx_av_callback *callback);


/**
* 接口说明：退出所有SDK音视频相关的逻辑
* 返回值  ：错误码（见全局错误码表）
*/
SDK_API int tx_stop_av_service();


/**
 * 向SDK填充视频数据
 * nGopIndex:Gop的index
 * nFrameIndex:当前帧在所在gop中的index
 * nTotalIndex:当前帧在总编码过程中的index
 */
SDK_API void tx_set_video_data(unsigned char *pcEncData, int nEncDataLen,
        int nFrameType, int nTimeStamps, int nGopIndex, int nFrameIndex, int nTotalIndex, int nAvgQP);


/**
 * 向SDK填充音频数据
 */
SDK_API void tx_set_audio_data(tx_audio_encode_param *param, unsigned char *pcEncData, int nEncDataLen);


CXX_EXTERN_END

#endif // __TX_AUDIO_VIDEO_H__
