#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_cmd_exec.h"
#include "ak_vi.h"
#include "ak_ai.h"
#include "ak_aenc.h"
#include "ak_rtsp.h"
#include "ak_net.h"

#include <akae_stdlib.h>
#include <akae_rtspserver.h>
#include <akae_thread.h>



#define LEN_IFACE 32
#define LEN_IP    32
#define LEN_LINK  128


/**
 * 内部单体使用的内存分配器，初始化代表模块已经初始化。
 */
static AK_Object _Heap = AK_null;

/**
 * @ref _Heap 句柄对应的所分配内存。
 */
static AK_byte _heap [1024 * 1024 * 4];

/**
 * RTSP 服务器单体句柄。
 */
static AK_Object _Server = AK_null;


/// 后台码流缓冲线程句柄。
static AK_Thread _VStreamTH[RTSP_CHANNEL_NUM], _AStreamTH;

/// 媒体缓冲队列句柄。
static AK_Object _VStreamQ[RTSP_CHANNEL_NUM], _711aStreamQ;


/// 传入配置参数。
static struct rtsp_param _rtsp_ctrl_param;


static const char rtsp_version[] = "libapp_rtsp V2.0.00";

static void rtsp_venc_set_bps(void *venc_handle, 
		struct rtsp_channel_cfg_t *rtsp_chn)
{
	int target_kbps = 0, max_kbps = 0;

	/* ratio is not zero, its VBR model */
	if (rtsp_chn->target_ratio != 0) {
		/* normal VBR model */
		max_kbps = rtsp_chn->max_kbps;	
		target_kbps = rtsp_chn->target_ratio * max_kbps / 100;

		/* according to ratio, set target bps and max bps */
		ak_venc_set_kbps(venc_handle, target_kbps, max_kbps);

		/* smart model VBR */
		if (rtsp_chn->smart.smart_mode != 0) {
			ak_venc_set_smart_config(venc_handle, &rtsp_chn->smart);
		}
	}
}

/*
 * init video encode by index to indicate which encode group
 */
static void *video_encode_init(enum rtsp_channel index)
{
	struct encode_param param = {0};

	param.width   = _rtsp_ctrl_param.rtsp_chn[index].width;;
	param.height  = _rtsp_ctrl_param.rtsp_chn[index].height;;
	param.minqp   = _rtsp_ctrl_param.rtsp_chn[index].min_qp;
	param.maxqp   = _rtsp_ctrl_param.rtsp_chn[index].max_qp;
	param.fps     = _rtsp_ctrl_param.rtsp_chn[index].fps;
	param.goplen  = param.fps * _rtsp_ctrl_param.rtsp_chn[index].gop_len;
	param.bps     = _rtsp_ctrl_param.rtsp_chn[index].max_kbps;
	param.br_mode = _rtsp_ctrl_param.rtsp_chn[index].video_br_mode;
	param.enc_out_type = _rtsp_ctrl_param.rtsp_chn[index].video_enc_type;

	switch (index) {
	case RTSP_CHANNEL_MAIN:	//main channel
		param.use_chn = ENCODE_MAIN_CHN;
		param.enc_grp = ENCODE_MAINCHN_NET;

		switch (_rtsp_ctrl_param.rtsp_chn[index].video_enc_type) {
        case H264_ENC_TYPE:
            param.profile = PROFILE_MAIN;
            break;
        case HEVC_ENC_TYPE:
            param.profile = PROFILE_HEVC_MAIN;
            break;
        default:
            break;
        }
		break;
	case RTSP_CHANNEL_SUB:	//sub channel
		param.use_chn = ENCODE_SUB_CHN;
		param.enc_grp = ENCODE_SUBCHN_NET;

		switch (_rtsp_ctrl_param.rtsp_chn[index].video_enc_type) {
        case H264_ENC_TYPE:
            param.profile = PROFILE_MAIN;
            break;
        case HEVC_ENC_TYPE:
            param.profile = PROFILE_HEVC_MAIN;
            break;
        default:
            break;
        }
		break;
	default:
		return NULL;
	}

	return ak_venc_open(&param);
}


/**
 * ak_rtsp_get_version - get rtsp version
 * return: version string
 */
const char* ak_rtsp_get_version(void)
{
	return rtsp_version;
}



static AK_void _buffer_aenc (AK_Thread th, AK_int argc, AK_voidptr argv[]) {

	void *ak_ai = NULL, *ak_aenc = NULL;
	struct pcm_param ai_param;
	struct audio_param aenc_param;

	ai_param.sample_bits = 16;
	ai_param.channel_num = AUDIO_CHANNEL_MONO;
	ai_param.sample_rate = AK_AUDIO_SAMPLE_RATE_8000;

	ak_ai = ak_ai_open (&ai_param);
    AK_ASSERT (NULL != ak_ai);

    ak_ai_set_nr_agc (ak_ai, AUDIO_FUNC_ENABLE);
    ak_ai_set_aec (ak_ai, AUDIO_FUNC_ENABLE);
	ak_ai_set_aslc_volume(ak_ai, 4);
    ak_ai_set_volume(ak_ai, 2);

    ak_ai_set_source(ak_ai, AI_SOURCE_MIC);
	ak_ai_clear_frame_buffer(ak_ai);
	ak_ai_set_frame_interval (ak_ai, 40);

	aenc_param.type = AK_AUDIO_TYPE_PCM_ALAW;
	aenc_param.sample_bits = 16;
	aenc_param.channel_num = AUDIO_CHANNEL_MONO;
	aenc_param.sample_rate = AK_AUDIO_SAMPLE_RATE_8000;
	ak_aenc = ak_aenc_open(&aenc_param);
	AK_ASSERT (NULL != ak_aenc);

    ak_print_normal("ak_aenc_open OK\n");
	if (aenc_param.type == AK_AUDIO_TYPE_AAC) {
	    struct aenc_attr attr;
	    attr.aac_head = AENC_AAC_SAVE_FRAME_HEAD;
	    ak_aenc_set_attr (ak_aenc, &attr);
    }

	ak_ai_start_capture(ak_ai);

	while (!akae_thread_terminated (th)) {

		struct frame ai_frame = {0};
		struct audio_stream stream = {0};
		unsigned char audio[1024];

		while (ak_ai_get_frame (ak_ai, &ai_frame, 0) == AK_SUCCESS) {

			memset (&stream, 0, sizeof (stream));
			stream.data = audio;
			stream.len = sizeof (audio);

			if (ak_aenc_send_frame (ak_aenc, &ai_frame, &stream) > 0) {
				akae_rtp_queue_add_payload (_711aStreamQ, (AK_uint32)ai_frame.ts, stream.data, stream.len);
			}

			ak_ai_release_frame(ak_ai, &ai_frame);
		}

		akae_thread_suspend (th, 0, 20, 0);
	}

	ak_aenc_close (ak_aenc);
	ak_aenc = NULL;

	ak_ai_stop_capture (ak_ai);
	ak_ai_close (ak_ai);

}



static AK_void _buffer_venc (AK_Thread th, AK_int argc, AK_voidptr argv[]) {

	void *ak_venc = NULL, *ak_vstream = NULL;
	AK_int ch = (AK_int)(argv[0]);

	/* venc open for sub net */
	ak_venc = video_encode_init (ch);
	AK_ASSERT (NULL != ak_venc);

	rtsp_venc_set_bps (ak_venc, &_rtsp_ctrl_param.rtsp_chn[ch]);

	/* venc request */
	ak_vstream = ak_venc_request_stream(_rtsp_ctrl_param.rtsp_chn[ch].vi_handle,
			ak_venc);
	AK_ASSERT (NULL != ak_vstream);


	while (!akae_thread_terminated (th)) {

		struct video_stream vs = {0};

		/// 读取数据至空为止。
		while (AK_SUCCESS == ak_venc_get_stream (ak_vstream, &vs)) {
			/// 缓冲媒体数据到队列。
			akae_rtp_queue_add_payload (_VStreamQ[ch], (AK_uint32)vs.ts, vs.data, vs.len);
			ak_venc_release_stream (ak_vstream, &vs);
		}

		akae_thread_suspend (th, 0, 30, 0);

	}

	ak_venc_cancel_stream (ak_vstream);
	ak_venc_close (ak_venc);

}




/**
 * ak_rtsp_init - init rtsp param
 *                start rtsp dispense sever
 *                start rtsp listen\recv\send
 * return: 0 -> success, -1 -> failed
 */
int ak_rtsp_init(struct rtsp_param *param)
{
	char ac_iface[LEN_IFACE], ac_ip[LEN_IP], ac_rtsp[LEN_LINK];
	AK_int argc = 0;
	AK_voidptr argv[32];
	AK_int i = 0;
	AK_int registry = 0;
	AK_int port = 554;

	AK_EXPECT_RETURN_VAL (AK_null == _Heap, AK_FAILED);

	if (!param) {
		ak_print_error_ex("invalid argument\n");
		return -1;
	}

	/// 初始化栈分配器。
	_Heap = akae_malloc_create (AK_true, _heap, sizeof (_heap));
	AK_EXPECT_RETURN_VAL (AK_null != _Heap, AK_FAILED);

	memcpy(&_rtsp_ctrl_param, param, sizeof(_rtsp_ctrl_param));
	
	_Server = akae_rtsp_server_create (_Heap);
	AK_ASSERT (AK_null != _Server);
	akae_rtsp_server_verbose (_Server, AK_true);
	akae_rtsp_server_verbose_http (_Server, AK_true);

	/// 创建音频缓冲。
	_711aStreamQ = akae_rtp_queue_create (_Heap, AK_RTP_PT_PCMA);
	AK_ASSERT (AK_null != _711aStreamQ);
	_AStreamTH = akae_thread_create_default (_buffer_aenc);
	AK_ASSERT (_AStreamTH> 0);

	for (i = 0; i < RTSP_CHANNEL_NUM; ++i) {

		_VStreamQ[i] =  akae_rtp_queue_create (_Heap, HEVC_ENC_TYPE == param->rtsp_chn[i].video_enc_type ? AK_RTP_PT_H265 : AK_RTP_PT_H264);
		AK_ASSERT (AK_null != _VStreamQ[i]);

		/// 注册监听地址。
		registry = akae_rtsp_server_register_url (_Server, param->rtsp_chn[i].suffix_name, AK_null, AK_null);
		AK_ASSERT (registry > 0);
		akae_rtsp_server_describe_video (_Server, registry, akae_rtp_queue_payload_type (_VStreamQ[i]), 90000, _VStreamQ[i]);
		akae_rtsp_server_describe_g711a (_Server, registry, _711aStreamQ);


		/// 创建后台线程缓冲编码数据。
		argc = 0;
		argv[argc++] = (AK_voidptr)(i);
		_VStreamTH[i] = akae_thread_create_default2 (_buffer_venc, argc, argv);
		AK_ASSERT (_VStreamTH[i] > 0);

	}

	if (AK_OK != akae_rtsp_server_start (_Server, port)) {
		port = 8554;
		akae_rtsp_server_start (_Server, port);
	}


	ak_net_get_cur_iface(ac_iface);
	ak_net_get_ip(ac_iface, ac_ip) ;
	COLOR_PRINT( COLOR_MODE_BOLD, COLOR_BACK_BLACK, COLOR_FRONT_GREEN, ac_rtsp, LEN_LINK, "***********************************************\n" )
	COLOR_PRINT( COLOR_MODE_BOLD, COLOR_BACK_BLACK, COLOR_FRONT_GREEN, ac_rtsp, LEN_LINK, "*                  RTSP LINK                  *\n" )
	COLOR_PRINT( COLOR_MODE_BOLD, COLOR_BACK_BLACK, COLOR_FRONT_GREEN, ac_rtsp, LEN_LINK, "***********************************************\n" )
	COLOR_PRINT( COLOR_MODE_BOLD, COLOR_BACK_BLACK, COLOR_FRONT_GREEN, ac_rtsp, LEN_LINK, "* MAIN CHANNEL : rtsp://%s:%d/%s\n", ac_ip, port, param->rtsp_chn[0].suffix_name)
	COLOR_PRINT( COLOR_MODE_BOLD, COLOR_BACK_BLACK, COLOR_FRONT_GREEN, ac_rtsp, LEN_LINK, "* SUB CHANNEL  : rtsp://%s:%d/%s\n", ac_ip, port, param->rtsp_chn[1].suffix_name)
	COLOR_PRINT( COLOR_MODE_BOLD, COLOR_BACK_BLACK, COLOR_FRONT_GREEN, ac_rtsp, LEN_LINK, "***********************************************\n" )

	return 0;
}

/**
 * @deprecated >= 1.9.00
 */
int ak_rtsp_start(int index) {
	return 0;
}

/**
 * @deprecated >= 1.9.00
 */
int ak_rtsp_stop(int index) {
	return 0;
}

/**
 * ak_rtsp_exit - exit rtsp
 * @void
 */
void ak_rtsp_exit(void) {

	AK_int i = 0;
	AK_EXPECT_RETURN (AK_null != _Heap);

	akae_thread_release (_AStreamTH, AK_TRUE);
	_AStreamTH = 0;
	akae_rtp_queue_release_clear (_711aStreamQ);
	_711aStreamQ = AK_null;


	/// 停止视频编码线程。
	for (i = 0; i < RTSP_CHANNEL_NUM; ++i) {
		akae_thread_release (_VStreamTH[i], AK_TRUE);
		_VStreamTH[i] = 0;
		akae_rtp_queue_release_clear (_VStreamQ[i]);
		_VStreamQ[i] = AK_null;
	}


	if (AK_null != _Server) {
		akae_rtsp_server_release (_Server);
		_Server = AK_null;
	}

	/// 释放内存缓冲句柄。
	akae_malloc_release (_Heap, AK_null, AK_null);
	_Heap = AK_null;

}
