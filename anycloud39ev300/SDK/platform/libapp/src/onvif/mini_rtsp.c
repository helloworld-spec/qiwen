/*************************************************************************
	> File Name: minirtsp_main.c
	> Author: kaga
	> Mail: kejiazhw@163.com
	> Created Time: 2014???1???6???星期???10???4???1??? ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<stdint.h>

#include <sys/time.h>

#include "minirtsp_lib.h"
#include "rtplib.h"

#include "ja_type.h"
#include "ja_media.h"
#include "n1_def.h"

#include "list.h"
#include "ak_common.h"
#include "ak_venc.h"
#include "ak_osd_ex.h"
#include "ja_osd.h"
#include "ak_onvif_config.h"

#define USR_NAME_SIZE_MAX		(64)
#define USR_PASSWORD_SIZE_MAX	(64)
#define USR_FILE_PATH	"/etc/jffs2/usr.conf"


typedef struct
{
	uint32_t conn_id;	//connect id in multi-connection
	enum onvif_send_stream_id stream_id;	//send stream flag, video or audio
    uint32_t audio_ts_ms;	// audio timestamp 
	uint32_t video_ts_ms;	// video timestamp
}MINIRTSP_USR_STRUCT;

static JA_CTRL *pMiniRtsp = NULL;

/**
 * mini_drop_extra_audio_stream - drop audio stream
 * @idx[IN]: conn index
 * @vstr_ts[IN]: video timestamp
 * return: 0 success, -1 fail
 */
static void mini_drop_extra_audio_stream(int idx, unsigned long long vstr_ts)
{
	struct aenc_entry *audio = NULL;
	struct aenc_entry *ptr = NULL;

	ak_thread_mutex_lock(&pja_ctrl->conn[idx].audio_lock);
	list_for_each_entry_safe(audio, ptr,
		&(pMiniRtsp->conn[idx].audio_queue), list) {
		if (audio->stream.ts < vstr_ts) {
			list_del_init(&(audio->list));
			pMiniRtsp->conn[idx].audio_stream_size -= audio->stream.len;
			ja_media_free_audio_stream(audio, AK_TRUE);
		}
	}
	ak_thread_mutex_unlock(&pja_ctrl->conn[idx].audio_lock);
}

/**
 * mini_send_audio_stream - send audio stream
 * @ctx[IN/OUT]: usr params struct
 * @s[IN]: minirtsp param  struct
 * return: 0 success, -1 fail
 */
static int mini_send_audio_stream(MINIRTSP_USR_STRUCT *ctx, lpMINIRTSP_STREAM s)
{
    int i = ctx->conn_id;
	int free_data = AK_TRUE;
	
    struct aenc_entry *audio = list_first_entry_or_null(
        &pMiniRtsp->conn[i].audio_queue, struct aenc_entry, list);
	if (NULL == audio) {
		return rMINIRTSP_DATA_NOT_AVAILABLE;
	}

    int ret = rMINIRTSP_OK;

	if (0 == audio->stream.len) {
		ak_print_error_ex("audio->stream.len = 0\n");
		goto mini_audio_end;
	}

	if (NULL != s->data) {
		free(s->data);
		s->data = NULL;
	}
#if 0
	s->data = (unsigned char *)malloc(audio->stream.len);
	if (NULL == s->data) {
		ak_print_error_ex("s->data malloc failed!\n");
		ret = rMINIRTSP_FAIL;
		goto mini_audio_end;
	}

    memcpy(s->data, audio->stream.data, audio->stream.len);
#else
	s->data = audio->stream.data;
	free_data = AK_FALSE;
#endif
	s->timestamp = audio->stream.ts * 1000;
	s->type = MINIRTSP_MD_TYPE_ALAW;
	s->size = audio->stream.len;
	ctx->audio_ts_ms = audio->stream.ts;
	pMiniRtsp->conn[i].audio_stream_size -= s->size;

mini_audio_end:
	ja_media_release_audio_stream(i, audio, free_data);
	return ret;
}

/**
 * mini_send_video_stream - send video stream
 * @ctx[IN/OUT]: usr params struct
 * @s[IN]: minirtsp param  struct
 * return: 0 success, -1 fail
 */
static int mini_send_video_stream(MINIRTSP_USR_STRUCT *ctx, lpMINIRTSP_STREAM s)
{
    int i = ctx->conn_id;
	int free_data = AK_TRUE;
	
    ja_video_stream *pstream = list_first_entry_or_null(
        &pMiniRtsp->conn[i].video_queue, ja_video_stream, list);
	if (!pstream) {
		return rMINIRTSP_DATA_NOT_AVAILABLE;
	}

    int ret = rMINIRTSP_OK;

	if (0 == pstream->vstr.len) {
		ak_print_error_ex("vstr.len = 0\n");
		goto mini_video_end;
	}

	if (!pMiniRtsp->conn[i].biFrameflag) {
		if (FRAME_TYPE_I == pstream->vstr.frame_type) {
			pMiniRtsp->conn[i].biFrameflag = AK_TRUE;
			if (pMiniRtsp->ai_enable)
				mini_drop_extra_audio_stream(i, pstream->vstr.ts);
		} else {
			ret = rMINIRTSP_DATA_NOT_AVAILABLE;
			pMiniRtsp->conn[i].video_stream_size -= pstream->vstr.len;
			goto mini_video_end;
		}
	}

	if (pMiniRtsp->conn[i].bMainStream) {
		if (NK_N1_VENC_CODEC_HEVC == g_video_set.main_enctype || NK_N1_VENC_CODEC_HEVC_PLUS == g_video_set.main_enctype)
			s->type = MINIRTSP_MD_TYPE_H265;
		else
			s->type = MINIRTSP_MD_TYPE_H264;
	}else {
		if (NK_N1_VENC_CODEC_HEVC == g_video_set.sub_enctype || NK_N1_VENC_CODEC_HEVC_PLUS == g_video_set.sub_enctype)
			s->type = MINIRTSP_MD_TYPE_H265;
		else
			s->type = MINIRTSP_MD_TYPE_H264;
	}

	if (NULL != s->data) {
		free(s->data);
		s->data = NULL;
	}
#if 0
	s->data = (unsigned char *)malloc(pstream->vstr.len);
	if (NULL == s->data) {
		ak_print_error_ex("s->data malloc failed!\n");
		pMiniRtsp->conn[i].biFrameflag = AK_FALSE;
		ret = rMINIRTSP_FAIL;
		goto mini_video_end;
	}

    memcpy(s->data, pstream->vstr.data, pstream->vstr.len);
#else
	s->data = pstream->vstr.data;
	free_data = AK_FALSE;
#endif
	s->isKeyFrame = pstream->vstr.frame_type;
	s->timestamp = pstream->vstr.ts * 1000;
	s->size = pstream->vstr.len;
	ctx->video_ts_ms = pstream->vstr.ts;
	pMiniRtsp->conn[i].video_stream_size -= s->size;

	struct onvif_camera_config *camera = onvif_config_get_camera();
	/* show osd rate info */
	if (camera->rate_position > 0) {
		/* now: main/sub  channel  */
		if (((pMiniRtsp->conn[i].bMainStream) && ( i == pMiniRtsp->video_stat_conn[VIDEO_CHN_MAIN])) ||
			((!pMiniRtsp->conn[i].bMainStream) && ( i == pMiniRtsp->video_stat_conn[VIDEO_CHN_SUB]))){
			int channel = (pMiniRtsp->conn[i].bMainStream)? VIDEO_CHN_MAIN: VIDEO_CHN_SUB;
			ak_osd_ex_stat_video(channel, pMiniRtsp->media[channel].stream_handle);
		}
	}

mini_video_end:
	ja_media_release_video_stream(i, pstream, free_data);
	return ret;
}

/**
 * _stream_init - make minirtsp connect, support multi-connection
 * @s[IN]: minirtsp param  struct
 * @name[IN]: name
 * return: 0 success, -1 fail
 */
static int _stream_init(lpMINIRTSP_STREAM s, const char *name)
{
	MINIRTSP_USR_STRUCT *ctx = NULL;
	int i = 0;
	int stream_id = 0;

	if (pMiniRtsp->update_flag)
		return rMINIRTSP_FAIL;

	ak_print_normal_ex("name:%s\n", name);

	/* 
	 * only support 4 names, main stream 2 names, sub stream 2 names,
	 * main:ch0_0.264, ch1/main/av_stream
	 * sub:ch0_1.264, ch1/sub/av_stream
	 * other names, refuse to connect
	 */
	if ((0 != strcmp(name, "ch0_0.264"))
		&& (0 != strcmp(name, "ch1/main/av_stream"))
		&& (0 != strcmp(name, "ch0_1.264"))
		&& (0 != strcmp(name, "ch1/sub/av_stream"))){
		ak_print_error_ex("name error!\n");
		return rMINIRTSP_FAIL;
	}

	ak_thread_mutex_lock(&pMiniRtsp->lock);
	if (CONNECT_NUM_MAX == pMiniRtsp->conn_cnt) {
		ak_print_normal_ex("** conn_cnt is %d !**\n", pMiniRtsp->conn_cnt);
		ak_thread_mutex_unlock(&pMiniRtsp->lock);
		return rMINIRTSP_FAIL;
	}
	ctx = (MINIRTSP_USR_STRUCT*)calloc(1, sizeof(MINIRTSP_USR_STRUCT));

	memset(s, 0, sizeof(stMINIRTSP_STREAM));
	strcpy(s->name, name);
	s->timestamp = 0;
	s->data = NULL;
	s->size = 0;

	/* init connection resource */
	for (i=0; i<CONNECT_NUM_MAX; i++) {
	    //ak_thread_mutex_lock(&pMiniRtsp->conn[i].lock);
		if (AK_FALSE == pMiniRtsp->conn[i].bValid) {
			ctx->conn_id = i;
			pMiniRtsp->conn[i].bValid = AK_TRUE;
			pMiniRtsp->conn_cnt++;
			//ak_thread_mutex_unlock(&pMiniRtsp->conn[i].lock);
			break;
		}
		//ak_thread_mutex_unlock(&pMiniRtsp->conn[i].lock);
	}

	if (i >= CONNECT_NUM_MAX) {
		ak_print_normal_ex("can't find empty channel, i is %d!\n", i);
		ak_thread_mutex_unlock(&pMiniRtsp->lock);
		free(ctx);
		return rMINIRTSP_FAIL;
	}
	ak_print_normal_ex("** init stream %s ** conn : %d **\n", name, ctx->conn_id);
	s->param = ctx;

	ak_thread_mutex_lock(&pMiniRtsp->conn[i].video_lock);
	if ((0 == strcmp(name, "ch0_0.264"))
		|| (0 == strcmp(name, "ch1/main/av_stream")))	//main stream
	{
		pMiniRtsp->conn[i].bMainStream = AK_TRUE;
		INIT_LIST_HEAD(&pMiniRtsp->conn[i].video_queue);
		pMiniRtsp->conn[i].video_run_flag = 1;
		stream_id = VIDEO_CHN_MAIN;
		pMiniRtsp->main_cnt++;
		ak_print_normal_ex("main stream init, i: %d\n", i);
		if (pMiniRtsp->video_stat_conn[VIDEO_CHN_MAIN] < 0)
			pMiniRtsp->video_stat_conn[VIDEO_CHN_MAIN] = i;
	}
	else if ((0 == strcmp(name, "ch0_1.264"))
		|| (0 == strcmp(name, "ch1/sub/av_stream")))	//sub stream
	{
		pMiniRtsp->conn[i].bMainStream = AK_FALSE;
		INIT_LIST_HEAD(&pMiniRtsp->conn[i].video_queue);
        pMiniRtsp->conn[i].video_run_flag = 1;
		stream_id = VIDEO_CHN_SUB;
		pMiniRtsp->sub_cnt++;
		ak_print_normal_ex("sub stream init, i: %d\n", i);
		if (pMiniRtsp->video_stat_conn[VIDEO_CHN_SUB] < 0)
			pMiniRtsp->video_stat_conn[VIDEO_CHN_SUB] = i;
	}

	pMiniRtsp->conn[i].video_stream_size = 0;
	pMiniRtsp->conn[i].audio_stream_size = 0;

	/* request video */
	ja_media_venc_request_stream(pMiniRtsp, stream_id);
	ak_venc_set_iframe(pja_ctrl->media[stream_id].enc_handle);

	pMiniRtsp->conn[i].biFrameflag = AK_FALSE;
	ak_thread_mutex_unlock(&pMiniRtsp->conn[i].video_lock);

	ak_thread_mutex_lock(&pMiniRtsp->conn[i].audio_lock);
	INIT_LIST_HEAD(&pMiniRtsp->conn[i].audio_queue);
	ak_thread_mutex_unlock(&pMiniRtsp->conn[i].audio_lock);

	if (pMiniRtsp->ai_enable) {
		ak_thread_mutex_lock(&pMiniRtsp->conn[i].audio_lock);
		//INIT_LIST_HEAD(&pMiniRtsp->conn[i].audio_queue);
		pMiniRtsp->conn[i].audio_run_flag = 1;
		/* request audio */
		ja_media_aenc_request_stream(pMiniRtsp);
		ak_thread_mutex_unlock(&pMiniRtsp->conn[i].audio_lock);
	}
	ak_print_normal_ex("conn_cnt = %d, main_cnt = %d, sub_cnt = %d\n",
		pMiniRtsp->conn_cnt, pMiniRtsp->main_cnt, pMiniRtsp->sub_cnt);

	ak_thread_mutex_unlock(&pMiniRtsp->lock);

	/* init osd main and sub channel */
	ak_osd_ex_turn_on(VIDEO_CHN_MAIN);
	ak_osd_ex_turn_on(VIDEO_CHN_SUB);

	ak_print_normal_ex("rtsp stream(%s) init success.\n", name);

	return rMINIRTSP_OK;
}

/**
 * _stream_lock - lock
 * @s[IN]: minirtsp param  struct
 * return: 0 success, -1 fail
 */
static int _stream_lock(lpMINIRTSP_STREAM s)
{
	MINIRTSP_USR_STRUCT *ctx = s->param;
	int i = 0;

	if (ctx) {
		i = ctx->conn_id;
	} else {
        ak_print_error_ex("_stream_lock invalid\n");
		return rMINIRTSP_FAIL;
	}

	ak_thread_mutex_lock(&pMiniRtsp->conn[i].streamlock);
	return rMINIRTSP_OK;
}

/**
 * _stream_unlock - unlock
 * @s[IN]: minirtsp param  struct
 * return: 0 success, -1 fail
 */
static int _stream_unlock(lpMINIRTSP_STREAM s)
{
	MINIRTSP_USR_STRUCT *ctx = s->param;
	int i = 0;

	if (ctx) {
		i = ctx->conn_id;
	} else {
        ak_print_error_ex("_stream_unlock invalid\n");
		return rMINIRTSP_FAIL;
	}

	ak_thread_mutex_unlock(&pMiniRtsp->conn[i].streamlock);
	return rMINIRTSP_OK;
}

/**
 * _stream_next - get video/audio stream data
 * @s[IN]: minirtsp param  struct
 * return: 0 success, -1 fail
 */
static int _stream_next(lpMINIRTSP_STREAM s)
{
    s->size = 0;
	MINIRTSP_USR_STRUCT *ctx = s->param;
	if (!ctx) {
        ak_print_error_ex("_stream_next invalid\n");
		return rMINIRTSP_FAIL;
	}

    int ret = rMINIRTSP_FAIL;
    int i = ctx->conn_id;

    switch (ctx->stream_id) {
	case ONVIF_STREAM_VIDEO:
	    if(pMiniRtsp->conn[i].video_run_flag) {
			ak_thread_mutex_lock(&pja_ctrl->conn[i].video_lock);
            ret = mini_send_video_stream(ctx, s);
			ak_thread_mutex_unlock(&pja_ctrl->conn[i].video_lock);
			if (pMiniRtsp->ai_enable) {
				/* after transfer started, we switch to send audio according to ts. */
				if (pMiniRtsp->conn[i].biFrameflag
					&& (ctx->video_ts_ms >= ctx->audio_ts_ms)) {
					ctx->stream_id = ONVIF_STREAM_AUDIO;
				}
			}
	    } else {
			ak_print_error_ex("run_flag 0, i: %d\n", i);
		}
		break;
	case ONVIF_STREAM_AUDIO:
		if (pMiniRtsp->ai_enable) {
			if (pMiniRtsp->conn[i].audio_run_flag) {
				ak_thread_mutex_lock(&pja_ctrl->conn[i].audio_lock);
				ret = mini_send_audio_stream(ctx, s);
				ak_thread_mutex_unlock(&pja_ctrl->conn[i].audio_lock);
				/* after transfer started, we switch to send video according to ts. */
				if (ctx->audio_ts_ms >= ctx->video_ts_ms)
					ctx->stream_id = ONVIF_STREAM_VIDEO;
	        } else {
				ak_print_error_ex("run_flag 0\n");
				ctx->stream_id = ONVIF_STREAM_VIDEO;
			}
		}
		else {
			ak_print_error_ex("ai_enable 0\n");
			ctx->stream_id = ONVIF_STREAM_VIDEO;
		}
		break;
	default:
		break;
	}

	return ret;
}

/**
 * _stream_destroy - destroy minirtsp connect
 * @s[IN]: minirtsp param  struct
 * return: 0 success, -1 fail
 */
static int _stream_destroy(lpMINIRTSP_STREAM s)
{
	MINIRTSP_USR_STRUCT *ctx = s->param;
	int i = 0;

	if (s->data)
		free(s->data);

	ak_thread_mutex_lock(&pMiniRtsp->lock);
	if (0 == pMiniRtsp->conn_cnt) {
		ak_thread_mutex_unlock(&pMiniRtsp->lock);
		return rMINIRTSP_FAIL;
	}

	if (ctx) {
		i = ctx->conn_id;
	} else {
        ak_print_error_ex("_stream_destroy invalid\n");
		ak_thread_mutex_unlock(&pMiniRtsp->lock);
		return rMINIRTSP_FAIL;
	}

	pMiniRtsp->conn_cnt--;
	if (pMiniRtsp->conn[i].bMainStream){
		pMiniRtsp->main_cnt--;
		ja_media_check_video_conn_stat(pja_ctrl, VIDEO_CHN_MAIN, i);
	}
	else{
		pMiniRtsp->sub_cnt--;
		ja_media_check_video_conn_stat(pja_ctrl, VIDEO_CHN_SUB, i);
	}

	if (pMiniRtsp->ai_enable) {
		ak_thread_mutex_lock(&pMiniRtsp->conn[i].audio_lock);
		pMiniRtsp->conn[i].audio_run_flag = 0;
		ak_thread_mutex_unlock(&pMiniRtsp->conn[i].audio_lock);

		if (!pMiniRtsp->conn_cnt) {
			ak_print_notice_ex("going to stop aenc\n");
			ja_media_aenc_cancel_stream();
		}

	    if (!ja_media_is_queue_empty(i, AK_FALSE)) {
			ak_print_normal_ex("Mini Rtsp stop audio %d \n", i);

			/* release audio stream */
			ja_media_destroy_audio_stream_queue(i, &pMiniRtsp->conn[i].audio_queue);
		}
	}

	ak_thread_mutex_lock(&pMiniRtsp->conn[i].video_lock);
	pMiniRtsp->conn[i].video_run_flag = 0;
	ak_thread_mutex_unlock(&pMiniRtsp->conn[i].video_lock);

	/* cancel main video stream if no use */
	if (!pMiniRtsp->main_cnt && pMiniRtsp->conn[i].bMainStream) {
		ak_print_notice_ex("going to stop venc main\n");
		ja_media_venc_cancel_stream(MEDIA_TYPE_VIDEO_MAIN);
	}

	/* cancel sub video stream if no use */
	if (!pMiniRtsp->sub_cnt && !pMiniRtsp->conn[i].bMainStream) {
		ak_print_notice_ex("going to stop venc sub\n");
		ja_media_venc_cancel_stream(MEDIA_TYPE_VIDEO_SUB);
	}

	if (!ja_media_is_queue_empty(i, AK_TRUE)) {
        if ('0' == s->name[4]) {
    	    ak_print_normal_ex("Mini Rtsp stop main, %d #####\n", i);
    	} else if ('1' == s->name[4]) {
    		ak_print_normal_ex("Mini Rtsp stop sub %d #####\n", i);
    	}
		/* release video stream */
		ja_media_destroy_video_stream_queue(i, &pMiniRtsp->conn[i].video_queue);
    }

	pMiniRtsp->conn[i].bValid = AK_FALSE;

	free(ctx);
	ctx = NULL;

	if (0 == pMiniRtsp->main_cnt){
		ak_osd_ex_turn_off(VIDEO_CHN_MAIN);
	}

	if ((0 == pja_ctrl->sub_cnt) && (0 == pja_ctrl->main_cnt)) {	
		/* 
		 * for snapshot, 
		 * only when there has no connecting can turn off osd 
		 */
		ak_osd_ex_turn_off(VIDEO_CHN_SUB);
	}

	ak_print_normal_ex("conn_ch:%d\n", i);
	ak_print_normal_ex("conn_cnt = %d, main_cnt = %d, sub_cnt = %d\n",
		pMiniRtsp->conn_cnt, pMiniRtsp->main_cnt, pMiniRtsp->sub_cnt);

	ak_thread_mutex_unlock(&pMiniRtsp->lock);

	return rMINIRTSP_OK;
}

/**
 * _stream_reset - 
 * @s[IN]: minirtsp param  struct
 * return: 0 success, -1 fail
 */
static int _stream_reset(lpMINIRTSP_STREAM s)
{
	return rMINIRTSP_OK;
}

/**
 * _stream_get_avc - get sps/pps info for HikVision NVR
 * without sps/pps info, HikVision NVR can't play video stream
 * main step:
 * 1. connect to device
 * 2. get I frame data to get sps/pps info
 * 3. disconnect
 * @name[IN]: name
 * @data[OUT]: buf to get sps/pps info
 * return: 0 success, -1 fail
 */
static int _stream_get_avc(char *name, H264AVC_t *data)
{
	int i = 0;
	int stream_id = 0;
	ja_video_stream *pstream;

	if (pMiniRtsp->update_flag)
		return rMINIRTSP_FAIL;

	ak_thread_mutex_lock(&pMiniRtsp->lock);
	if (CONNECT_NUM_MAX == pMiniRtsp->conn_cnt) {
		ak_print_normal_ex("** conn_cnt is %d !**\n", pMiniRtsp->conn_cnt);
		ak_thread_mutex_unlock(&pMiniRtsp->lock);
		return rMINIRTSP_FAIL;
	}

	/* 
	* init connection resource 
	*/
	for (i=0; i<CONNECT_NUM_MAX; i++) {
	    //ak_thread_mutex_lock(&pMiniRtsp->conn[i].lock);
		if (AK_FALSE == pMiniRtsp->conn[i].bValid) {
			pMiniRtsp->conn[i].bValid = AK_TRUE;
			pMiniRtsp->conn_cnt++;
			//ak_thread_mutex_unlock(&pMiniRtsp->conn[i].lock);
			break;
		}
		//ak_thread_mutex_unlock(&pMiniRtsp->conn[i].lock);
	}

	if (i >= CONNECT_NUM_MAX) {
		ak_print_normal_ex("can't find empty channel, i is %d!\n", i);
		ak_thread_mutex_unlock(&pMiniRtsp->lock);
		return rMINIRTSP_FAIL;
	}
	ak_print_normal_ex("** get avc stream %s **\n", name);

	ak_thread_mutex_lock(&pMiniRtsp->conn[i].video_lock);

	INIT_LIST_HEAD(&pMiniRtsp->conn[i].video_queue);
	pMiniRtsp->conn[i].video_run_flag = 1;
	pMiniRtsp->conn[i].video_stream_size = 0;
	pMiniRtsp->conn[i].biFrameflag = AK_FALSE;
		
	if ((0 == strcmp(name, "ch0_0.264"))
		|| (0 == strcmp(name, "ch1/main/av_stream"))) {
		pMiniRtsp->conn[i].bMainStream = AK_TRUE;
		stream_id = VIDEO_CHN_MAIN;
		pMiniRtsp->main_cnt++;
	} else if ((0 == strcmp(name, "ch0_1.264"))
		|| (0 == strcmp(name, "ch1/sub/av_stream"))) {
		pMiniRtsp->conn[i].bMainStream = AK_FALSE;
		stream_id = VIDEO_CHN_SUB;
		pMiniRtsp->sub_cnt++;
	}

	/* 
	* request video stream to get I frame data,
	* no need to request audio stream
	*/
	ja_media_venc_request_stream(pMiniRtsp, stream_id);

	ak_print_normal_ex("conn_cnt = %d, main_cnt = %d, sub_cnt = %d\n",
		pMiniRtsp->conn_cnt, pMiniRtsp->main_cnt, pMiniRtsp->sub_cnt);

	ak_thread_mutex_unlock(&pMiniRtsp->conn[i].video_lock);
	ak_thread_mutex_unlock(&pMiniRtsp->lock);

	ak_print_normal_ex("rtsp stream(%s) get avc success.\n", name);

	char* pps_p = NULL;

	/* 
	* get I frame data to get sps/pps info 
	*/
	while(pMiniRtsp->conn[i].video_run_flag) {
		ak_thread_mutex_lock(&pMiniRtsp->conn[i].video_lock);
		pstream = list_first_entry_or_null(&pMiniRtsp->conn[i].video_queue,
					ja_video_stream, list);
		
		if (!pstream) {
			ak_thread_mutex_unlock(&pMiniRtsp->conn[i].video_lock);
			continue;
		}
		
		pMiniRtsp->conn[i].video_stream_size -= pstream->vstr.len;

		if (pstream->vstr.frame_type != FRAME_TYPE_I) {
			ja_media_release_video_stream(i, pstream, AK_TRUE);
			ak_thread_mutex_unlock(&pMiniRtsp->conn[i].video_lock);
			continue;
		} else {
			/* got I frame data */
			char *p = (char*)(pstream->vstr.data + 4);
			char *e = p;
			/* get sps info */
			for(e = p; e < (char*)(pstream->vstr.data + pstream->vstr.len); e++){
				 if (e[0] == 0 && e[1] == 0 && e[2] == 0 && e[3] ==1) {//start code
					int sps_len = e - p;
					if(sps_len < 256){
						memcpy(data->sps, p, sps_len);					
						data->sps_size = sps_len;
						ak_print_normal_ex("sps len %d \n", data->sps_size);
						pps_p = e;						
					}
					break;
				}
			}

			/* get pps info */
			if(pps_p ){
				char *p = pps_p + 4;
				char *e = p;
				for(e = p; e < (char*)(pstream->vstr.data + pstream->vstr.len); e++){
					 if (e[0] == 0 && e[1] == 0 && e[2] == 0 && e[3] ==1) {//start code
						int pps_len = e - p;	
						if(pps_len < 256){
							memcpy(data->pps, p, pps_len);
							data->pps_size = pps_len;
							ak_print_normal_ex("pps len %d \n", data->pps_size);
							pps_p = e;
						}
						break;
					}					
				}
			}
			pMiniRtsp->conn[i].biFrameflag = AK_TRUE;

			ja_media_release_video_stream(i, pstream, AK_TRUE);
			ak_thread_mutex_unlock(&pMiniRtsp->conn[i].video_lock);
			break;
		}
	}

	/*
	* destroy this connect
	*/
	
	ak_thread_mutex_lock(&pMiniRtsp->lock);
	pMiniRtsp->conn_cnt--;
	if (pMiniRtsp->conn[i].bMainStream){
		pMiniRtsp->main_cnt--;
	}
	else{
		pMiniRtsp->sub_cnt--;
	}

	ak_thread_mutex_lock(&pMiniRtsp->conn[i].video_lock);
	pMiniRtsp->conn[i].bValid = AK_FALSE;
	pMiniRtsp->conn[i].video_run_flag = 0;
	ak_thread_mutex_unlock(&pMiniRtsp->conn[i].video_lock);

	/* cancel main video stream if no use */
	if (!pMiniRtsp->main_cnt && pMiniRtsp->conn[i].bMainStream) {
		ak_print_notice_ex("going to stop venc main\n");
		ja_media_venc_cancel_stream(MEDIA_TYPE_VIDEO_MAIN);
	}

	/* cancel sub video stream if no use */
	if (!pMiniRtsp->sub_cnt && !pMiniRtsp->conn[i].bMainStream) {
		ak_print_notice_ex("going to stop venc sub\n");
		ja_media_venc_cancel_stream(MEDIA_TYPE_VIDEO_SUB);
	}

	/* release video stream */
	if (!ja_media_is_queue_empty(i, AK_TRUE)) {
		ja_media_destroy_video_stream_queue(i, &pMiniRtsp->conn[i].video_queue);
    }

	ak_thread_mutex_unlock(&pMiniRtsp->lock);

	ak_print_normal_ex("conn_ch:%d\n", i);
	ak_print_normal_ex("conn_cnt = %d, main_cnt = %d, sub_cnt = %d\n",
		pMiniRtsp->conn_cnt, pMiniRtsp->main_cnt, pMiniRtsp->sub_cnt);

	return rMINIRTSP_OK;
}

/**
 * rtsp_venc_hook - get RTP type
 * @stream_name[IN]: name
 * @channel_venc[OUT]: RTP type
 * return: 0 success, -1 fail
 */
static int rtsp_venc_hook(char *stream_name, VENC_t *channel_venc)
{
	struct onvif_camera_config *camera = onvif_config_get_camera();

	/*
	* get RTP type, otherwise client can't decode the stream
	*/	
	if ((0 == strcmp(stream_name, "ch0_0.264"))
		|| (0 == strcmp(stream_name, "ch1/main/av_stream"))) {
		/*main channel*/
		channel_venc->resHeight = camera->main_height;
		channel_venc->resWidth = camera->main_width;
		channel_venc->frameRate = g_video_set.mainfps;

		if (NK_N1_VENC_CODEC_HEVC == g_video_set.main_enctype || NK_N1_VENC_CODEC_HEVC_PLUS == g_video_set.main_enctype)
			channel_venc->codeType = RTP_TYPE_DYNAMIC_H265;
		else
			channel_venc->codeType = RTP_TYPE_DYNAMIC;
	}else {
		/*sub channel*/
		channel_venc->resHeight = camera->sub_height;
		channel_venc->resWidth = camera->sub_width;
		channel_venc->frameRate = g_video_set.subfps;

		if (NK_N1_VENC_CODEC_HEVC == g_video_set.sub_enctype || NK_N1_VENC_CODEC_HEVC_PLUS == g_video_set.sub_enctype)
			channel_venc->codeType = RTP_TYPE_DYNAMIC_H265;
		else
			channel_venc->codeType = RTP_TYPE_DYNAMIC;
	}

	ak_print_normal_ex("**************rtsp_venc_hook*****************\n");
	return rMINIRTSP_OK;
}

static int rtsp_IFrame_hook(char *stream_name,int code ,void* data)
{
	if(NULL == stream_name){
		return -1;
	}

	/*
	* set I frame, when client switch main/sub channel to play 
	*/
	if(strstr(stream_name,"ch0_0.264")){
		ak_venc_set_iframe(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].enc_handle);
	}else if(strstr(stream_name,"ch0_1.264")){
		ak_venc_set_iframe(pja_ctrl->media[MEDIA_TYPE_VIDEO_SUB].enc_handle);
	}

	return 0;
}

/**
 * minirtsp_server_get_pwd_by_name - get password
 * @name[IN]: name
 * @ret_pwd[OUT]: password
 * @ret_size[IN]: out buf size
 * return: 0 success, -1 fail
 */
static int minirtsp_server_get_pwd_by_name(const char *name, char *ret_pwd, int ret_size)
{
	NK_Size user_count = 0;
	NK_Int i = 0;
	NK_Char username[USR_NAME_SIZE_MAX+1];
	NK_Char *buf = NULL;
	FILE *fd = NULL;
	NK_Int size = 0;

	ak_thread_mutex_lock(&pja_ctrl->usrFilelock);

	fd = fopen (USR_FILE_PATH, "rb");
	if (NULL == fd)
	{
		ak_thread_mutex_unlock(&pja_ctrl->usrFilelock);
		ak_print_error("n1 usr fd open failed!\n");
		if ( !strcmp(name, "admin") ) {
			return 0;
		} else {
			return -1;
		}
	}
	else
	{
		/*
		* usr name and password are set by n1
		*/
		fseek(fd, 0, SEEK_END);
		size = ftell(fd);
		#ifdef CONFIG_N1_WIFI_SUPPORT
		user_count = size / (USR_NAME_SIZE_MAX + USR_PASSWORD_SIZE_MAX + USR_CLASSIFY_SIZE_MAX);
		#else
		user_count = size / (USR_NAME_SIZE_MAX + USR_PASSWORD_SIZE_MAX);
		#endif
		ak_print_normal_ex("n1 user_count = %d!\n", user_count);

		if (0 == user_count)
		{
			fclose(fd);
			ak_thread_mutex_unlock(&pja_ctrl->usrFilelock);
			return -1;
		}

		buf = (NK_Char *)malloc(size);

		if (NULL == buf)
		{
			fclose(fd);
			ak_thread_mutex_unlock(&pja_ctrl->usrFilelock);
			ak_print_error("n1 usr buf malloc failed!\n");
			return -1;
		}
		else
		{
			fseek(fd, 0, SEEK_SET);
			fread(buf, 1, size, fd);
			fclose(fd);
			ak_thread_mutex_unlock(&pja_ctrl->usrFilelock);

			for (i=0; i<user_count; i++)
			{
				memset(username, 0, USR_NAME_SIZE_MAX+1);
				#ifdef CONFIG_N1_WIFI_SUPPORT
				memcpy(username, buf + i * (USR_NAME_SIZE_MAX + USR_PASSWORD_SIZE_MAX + USR_CLASSIFY_SIZE_MAX), USR_NAME_SIZE_MAX);
				#else
				memcpy(username, buf + i * (USR_NAME_SIZE_MAX + USR_PASSWORD_SIZE_MAX), USR_NAME_SIZE_MAX);
				#endif

				if ( !strcmp(name, username) )
				{
					memset(ret_pwd, 0, ret_size);
					#ifdef CONFIG_N1_WIFI_SUPPORT
					memcpy(ret_pwd, buf + i * (USR_NAME_SIZE_MAX + USR_PASSWORD_SIZE_MAX + USR_CLASSIFY_SIZE_MAX) + USR_NAME_SIZE_MAX, ret_size);
					#else
					memcpy(ret_pwd, buf + i * (USR_NAME_SIZE_MAX + USR_PASSWORD_SIZE_MAX) + USR_NAME_SIZE_MAX, ret_size);
					#endif

					free(buf);
					return 0;
				}
			}
		}

		free(buf);
	}

	return -1;
}

/**
 * minirtsp_server_run - init rtsp
 * return: handle
 */
lpMINIRTSP minirtsp_server_run(void)
{
	stMINIRTSP_STREAM_FUNC funcs;

	/*
	* minirtsp support multi-connection, manage together with n1.
	* use the same control struct pointer pja_ctrl, don't malloc a new one.
	*/
	pMiniRtsp = pja_ctrl;
    if(!pMiniRtsp) {
        return NULL;
    }

	MINIRTSP_server_auth_hook(minirtsp_server_get_pwd_by_name);

	lpMINIRTSP thiz = MINIRTSP_server_new();

	funcs.open = (fMINIRTSP_STREAM_OPEN)_stream_init;	//connect
	funcs.lock = (fMINIRTSP_STREAM_LOCK)_stream_lock;
	funcs.next = (fMINIRTSP_STREAM_NEXT)_stream_next;	//get video/audio stream
	funcs.unlock = (fMINIRTSP_STREAM_UNLOCK)_stream_unlock;
	funcs.close = (fMINIRTSP_STREAM_CLOSE)_stream_destroy;	//disconnect
	funcs.get_avc = (fMINIRTSP_STREAM_GET_AVC)_stream_get_avc;	//support HikVision NVR
	funcs.reset = (fMINIRTSP_STREAM_RESET)_stream_reset;
	MINIRTSP_server_set_stream_func(funcs);

	/*
	* get RTP type, otherwise client can't decode the stream
	*/
	MINIRTSP_server_venc_hook((MINIRTSP_SERVER_GET_VENC)rtsp_venc_hook);

	/*
	* set I frame, when client switch main/sub channel to play 
	*/
	MINIRTSP_server_IFrame_hook((MINIRTSP_SERVER_SET_IFRAME)rtsp_IFrame_hook);

	MINIRTSP_server_start(554);

	ak_print_normal_ex("minirtsp version: %s\n", MINIRTSP_char_version());
	//MINIRTSP_set_loglevel(thiz, 4);

	return thiz;
}
