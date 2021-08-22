#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <dirent.h>

#include "ak_common.h"
#include "ak_cmd_exec.h"
#include "ak_drv_ir.h"
#include "ak_drv_irled.h"
#include "ak_venc.h"
#include "ak_aenc.h"
#include "ak_ai.h"
#include "ak_vi.h"
#include "ak_vpss.h"

#include "n1_def.h"
#include "ak_onvif_config.h"
#include "ja_media.h"
#include "ak_net.h"
#include "ja_osd.h"
#include "ak_osd_ex.h"

/* ISP config file path */
#define FIRST_PATH       				"/etc/jffs2/"
#define BACK_PATH        				"/usr/local/"

struct ja_media_ir_t {
    int ir_run_flag;	    //photosensitive and ircut switch run flag
    int ir_switch_enable;   //store photosensitive switch status
    int ipc_run_flag;		//ipc run flag
    int cur_state;			//current state,0 night, 1 day
    void *vi_handle;		//global vi handle
    ak_pthread_t ir_tid;
};

struct ja_media_ir_t ja_ir = {0};

struct ja_media_fps_t {
    int fps_run_flag;	    //photosensitive and ircut switch run flag
    int cur_sensor_fps;
    void *vi_handle;		//global vi handle
    ak_pthread_t fps_tid;
};
struct ja_media_fps_t ja_media_fps = {0};

static void ja_media_modify_venc_parm(int chn, void *enc_handle, int smart_en, int bps, int brmode);
static void ja_media_modify_fps_and_goplen(int chn, int fps);


static void ja_switch_fps(int sensor_fps)
{
	int fps = 0;
	 
	struct onvif_venc_config *video = onvif_config_get_venc_param();
	 
	/* main channel */
	if (pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].enc_handle) {
		/*less than 12 fps is low fps*/
		if(sensor_fps <= 12){
			fps = sensor_fps;
		}else{
			fps = video->main_fps;
		}
		
		if (fps) {
			ja_media_modify_fps_and_goplen(MEDIA_TYPE_VIDEO_MAIN, fps);
		}
	}
	 
	/* sub channel */
	if (pja_ctrl->media[MEDIA_TYPE_VIDEO_SUB].enc_handle) {
		/*less than 12 fps is low fps*/
		if(sensor_fps <= 12){
			fps = sensor_fps;
		}else{
			fps = video->sub_fps;
		}
		
		if (fps) {
			ja_media_modify_fps_and_goplen(MEDIA_TYPE_VIDEO_SUB, fps);
		}
	}
}


static void *check_fps_switch(void *arg)
{
	long int tid = ak_thread_get_tid();
	int sensor_fps = 0;

	ak_print_normal_ex("Thread start, id: %ld\n", tid);
	ak_thread_set_name("fps_switch");

	/* get sensor fps and switch video fps */
	while (ja_media_fps.fps_run_flag) {
		sensor_fps = ak_vi_get_fps(ja_media_fps.vi_handle);
		if(sensor_fps != ja_media_fps.cur_sensor_fps){
			ak_print_normal_ex("prev fps=%d, new fps=%d\n",
			        ja_media_fps.cur_sensor_fps, sensor_fps);
			ja_switch_fps(sensor_fps);
			ja_media_fps.cur_sensor_fps = sensor_fps;
		}
		ak_sleep_ms(5*1000);
	}

	ak_print_normal_ex("Thread exit, id: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}


/* according to ir value to switch ircut and video */
static void *check_ir_switch(void *arg)
{
	long int tid = ak_thread_get_tid();
	int ir_val = 0;

	ak_print_normal_ex("Thread start, id: %ld\n", tid);
	ak_thread_set_name("ir_switch");

	/* get ir state and switch day-night */
	while (ja_ir.ir_run_flag) {
		if (ja_ir.ir_switch_enable) {
			ir_val = ak_drv_ir_get_input_level();
			if (ir_val != -1 && ja_ir.cur_state != ir_val) {
		        ak_print_normal_ex("prev_state=%d, ir_val=%d\n",
			        ja_ir.cur_state, ir_val);
				ja_media_set_video_day_night(ja_ir.vi_handle, ir_val);
			}
		}
		ak_sleep_ms(5*1000);
	}

	ak_print_normal_ex("Thread exit, id: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

/*
 * ja_media_is_queue_empty - check queue is empty or not
 * idx[IN]: connection id
 * bVideo[IN]: 1 video, 0 audio
 * return: 1 empty, 0 not empty
 */
int ja_media_is_queue_empty(int idx, int bVideo)
{
	int ret = 0;

	if (bVideo) {
		ak_thread_mutex_lock(&pja_ctrl->conn[idx].video_lock);
		ret = list_empty(&pja_ctrl->conn[idx].video_queue);
		ak_thread_mutex_unlock(&pja_ctrl->conn[idx].video_lock);
	} else {
		ak_thread_mutex_lock(&pja_ctrl->conn[idx].audio_lock);
		ret = list_empty(&pja_ctrl->conn[idx].audio_queue);
		ak_thread_mutex_unlock(&pja_ctrl->conn[idx].audio_lock);
	}

	return ret;
}

/**
* ja_media_alloc_video_stream - allocate stream node memory
* len[IN]: length of stream->data
* return: pointer to ja_video_stream on success, failed NULL;
*/
ja_video_stream *ja_media_alloc_video_stream(unsigned int len)
{
	ja_video_stream *v = (ja_video_stream *)calloc(1, sizeof(ja_video_stream));
	if (v) {
		v->vstr.data = (unsigned char *)calloc(1, len);
		if (!v->vstr.data) {
			ak_print_error_ex("alloc ja_video_stream failed\n");
			free(v);
			v = NULL;
		}
	}

	return v;
}

/*
 * ja_media_free_video_stream - release video stream node resource
 * v[IN]: pointer to video stream node which will be release
 * free_data[IN]: free data or not
 * return: void
 */
void ja_media_free_video_stream(ja_video_stream *v, int free_data)
{
	if (v) {
		if (free_data && v->vstr.data)
			free(v->vstr.data);

		free(v);
		v = NULL;
	}
}

/**
 * ja_media_push_video_data - push video stream to active queue
 * stream[IN]: pointer to stream which will be disponse to queue
 * chn[IN]: specific current channel
 * @return void
 */
void ja_media_push_video_data(struct video_stream *stream, char chn)
{
	int i = 0;
	int video_stream_max = MAIN_VIDEO_STREAM_SIZE_MAX;

	for (i = 0; i < CONNECT_NUM_MAX; i++) {
		if (pja_ctrl->conn[i].bValid
				&& pja_ctrl->conn[i].video_run_flag
				&& (pja_ctrl->conn[i].bMainStream == (!chn))) {
			/*
			 * TODO: pust video stream to queue
			 */
			ja_video_stream *v = ja_media_alloc_video_stream(stream->len);
			if (v) {
				memcpy(v->vstr.data, stream->data, stream->len);
				v->vstr.len = stream->len;
				v->vstr.ts  = stream->ts;
				v->vstr.seq_no = stream->seq_no;
				v->vstr.frame_type = stream->frame_type;
				v->idx = !pja_ctrl->conn[i].bMainStream;

				ak_thread_mutex_lock(&pja_ctrl->conn[i].video_lock);
				list_add_tail(&v->list, &pja_ctrl->conn[i].video_queue);
				ak_thread_mutex_unlock(&pja_ctrl->conn[i].video_lock);
				pja_ctrl->conn[i].video_stream_size += stream->len;

				//if queue is too large ,drop old stream data
				if (!pja_ctrl->conn[i].bMainStream)
					video_stream_max = SUB_VIDEO_STREAM_SIZE_MAX;
				
				if (pja_ctrl->conn[i].video_stream_size > video_stream_max) {
					int drop_cnt = 0;
					ja_video_stream *pos = NULL;
					ja_video_stream *n = NULL;
					ak_thread_mutex_lock(&pja_ctrl->conn[i].video_lock);
					list_for_each_entry_safe(pos, n, &pja_ctrl->conn[i].video_queue, list) {
						if ((pos->vstr.frame_type != FRAME_TYPE_I) || (0 == drop_cnt)) {
							pja_ctrl->conn[i].video_stream_size -= pos->vstr.len;
							ja_media_release_video_stream(i, pos, AK_TRUE);
							drop_cnt++;
							
						} else {
							ak_print_normal_ex("drop video %d\n", drop_cnt);
							break;
						}
					}
					ak_thread_mutex_unlock(&pja_ctrl->conn[i].video_lock);
				}
			}
		}
	}
}

/*
 * ja_media_release_video_stream - release video stream node
 * idx[IN]: connect index, to indicate specifically connection
 * v[IN]: pointer to video stream node which will be release
 * free_data[IN]: free data or not
 * return: void
 */
void ja_media_release_video_stream(int idx, ja_video_stream *v, int free_data)
{
	if (pja_ctrl->conn[idx].bValid) {
		/*
		 * TODO: delete video stream from queue
		 */
		list_del(&v->list);
		ja_media_free_video_stream(v, free_data);
	}
}

/*
 * ja_media_venc_cancel_stream - close video encode
 * idx[IN]: main or sub channel index, 0 is main
 * return: void
 */
void ja_media_venc_cancel_stream(int idx)
{
	if (idx > MEDIA_TYPE_VIDEO_SUB || idx < MEDIA_TYPE_VIDEO_MAIN)
		return;

	ak_thread_mutex_lock(&pja_ctrl->media[idx].lock);

	if (pja_ctrl->media[idx].stream_handle) {
		pja_ctrl->media[idx].run_flg = 0;
		if (pja_ctrl->media[idx].str_th_id)
			ak_thread_join(pja_ctrl->media[idx].str_th_id);
		/* TODO: release queue data */
		/* ... */

		ak_venc_cancel_stream(pja_ctrl->media[idx].stream_handle);
		pja_ctrl->media[idx].stream_handle = NULL;
	}

	ak_thread_mutex_unlock(&pja_ctrl->media[idx].lock);
}

/*
 * ja_media_destroy_video_stream_queue - destroy video stream queue
 * idx[IN]: connect index, to indicate specifically connection
 * stream_queue[IN]: stream list head
 * return: void
 */
void ja_media_destroy_video_stream_queue(int idx, struct list_head *stream_queue)
{
	ja_video_stream *pos, *n;

	ak_thread_mutex_lock(&pja_ctrl->conn[idx].video_lock);
	list_for_each_entry_safe(pos, n, stream_queue, list) {
		pja_ctrl->conn[idx].video_stream_size -= pos->vstr.len;
		ja_media_release_video_stream(idx, pos, AK_TRUE);
	}
	ak_thread_mutex_unlock(&pja_ctrl->conn[idx].video_lock);
}

/*
 * ja_media_venc_close - close video encode
 * main_stream[IN]: main or sub channel index, true is main
 * return: void
 */
void ja_media_venc_close(int idx)
{
	if (idx > MEDIA_TYPE_VIDEO_SUB || idx < MEDIA_TYPE_VIDEO_MAIN)
		return;

	ak_thread_mutex_lock(&pja_ctrl->media[idx].lock);

	if (pja_ctrl->media[idx].stream_handle) {
		pja_ctrl->media[idx].run_flg = 0;
		if (pja_ctrl->media[idx].str_th_id)
			ak_thread_join(pja_ctrl->media[idx].str_th_id);

		ak_venc_cancel_stream(pja_ctrl->media[idx].stream_handle);
		pja_ctrl->media[idx].stream_handle = NULL;
	}

	if (pja_ctrl->media[idx].enc_handle) {
		ak_venc_close(pja_ctrl->media[idx].enc_handle);
		pja_ctrl->media[idx].enc_handle = NULL;
	}

	for (int i=0; i<CONNECT_NUM_MAX; i++) {
		if ((pja_ctrl->conn[i].bValid)
			&& (!pja_ctrl->conn[i].bMainStream == idx)
			&& (!list_empty(&pja_ctrl->conn[i].video_queue)))
			ja_media_destroy_video_stream_queue(i, &pja_ctrl->conn[i].video_queue);
	}

	ak_thread_mutex_unlock(&pja_ctrl->media[idx].lock);
}

/*
 * ja_media_get_video_thread - get video stream thread
 * arg[IN]: thread argument
 * return: NULL
 */
void *ja_media_get_video_thread(void *arg)
{
	media_ctrl *media = (media_ctrl *)arg;
	long int tid = ak_thread_get_tid();
	ak_print_normal_ex("thread tid: %ld\n", tid);
	ak_thread_set_name("n1_video");

	while (media->run_flg) {
		struct video_stream stream = {0};
		int ret = ak_venc_get_stream(media->stream_handle, &stream);
		if (!ret) {
			ak_thread_mutex_lock(&media->str_lock);
			/* push stream to all active queue */
			ja_media_push_video_data(&stream, media->chn_idx);
			/* release for venc */
			ak_venc_release_stream(media->stream_handle, &stream);
			/* store to internal cache, dispose to all user */
			ak_thread_mutex_unlock(&media->str_lock);
		} else {
			/* get stream too fast, make a interval */
			ak_sleep_ms(10);
		}
	}

	ak_thread_exit();
	return NULL;
}

/*
 * ja_media_venc_init - init video encode channel
 * idx[IN]: to indicate current channel
 * return: on success, video encode handle; on failed NULL is return.
 */
void *ja_media_venc_init(int idx)
{
	struct encode_param enc_param = {0};
	struct onvif_camera_config *camera = onvif_config_get_camera();
	void *venc_handle = NULL;
	struct onvif_venc_config *video = onvif_config_get_venc_param();
	struct venc_smart_cfg smart_cfg = {0};
	int sensor_fps = ak_vi_get_fps(pja_ctrl->media[idx].input_handle);
	int fps = 0;

	/* main channel */
	if (idx == VIDEO_CHN_MAIN) {
		enc_param.width = camera->main_width;
		enc_param.height = camera->main_height;
		
		/*less than 12 fps is low fps*/
		if(sensor_fps <= 12)
			fps = sensor_fps;
		else
			fps = video->main_fps;
		
		enc_param.fps = fps;
		enc_param.goplen = fps * video->main_goplen;
		enc_param.bps = g_video_set.mainbps;	//kbps
		//enc_param.profile = PROFILE_MAIN;	//default main profile
		enc_param.use_chn = ENCODE_MAIN_CHN;
		enc_param.enc_grp = ENCODE_MAINCHN_NET;
		enc_param.br_mode = g_video_set.main_videomode;

		if (NK_N1_VENC_CODEC_HEVC == g_video_set.main_enctype || NK_N1_VENC_CODEC_HEVC_PLUS == g_video_set.main_enctype){
			enc_param.enc_out_type = HEVC_ENC_TYPE;
			enc_param.profile = PROFILE_HEVC_MAIN;
			enc_param.minqp = video->main_min_qp_265;
			enc_param.maxqp = video->main_max_qp_265;
		}else {
			enc_param.enc_out_type = H264_ENC_TYPE;
			enc_param.profile = PROFILE_HIGH;
			enc_param.minqp = video->main_min_qp_264;
			enc_param.maxqp = video->main_max_qp_264;
		}

		if (NK_N1_VENC_CODEC_H264 == g_video_set.main_enctype || NK_N1_VENC_CODEC_HEVC == g_video_set.main_enctype)
			smart_cfg.smart_mode = 0;
		else
			smart_cfg.smart_mode = video->main_smart_mode;
		
		smart_cfg.smart_goplen = video->main_smart_goplen*enc_param.fps;
		if(g_video_set.main_enctype == NK_N1_VENC_CODEC_H264 || g_video_set.main_enctype == NK_N1_VENC_CODEC_H264_PLUS)
			smart_cfg.smart_quality = video->main_smart_quality_264;
		else
			smart_cfg.smart_quality = video->main_smart_quality_265;
		smart_cfg.smart_static_value = video->smart_static_value;

		venc_handle = ak_venc_open(&enc_param);

		if (NULL == venc_handle) {
			ak_print_error("venc_handle null, restart anyka_ipc!\n");
			ak_cmd_exec("killall -9 anyka_ipc", NULL, 0);
			return NULL;
		}

		ja_media_modify_venc_parm(idx, venc_handle, smart_cfg.smart_mode, enc_param.bps, enc_param.br_mode);
	} else { /* sub channel */
		enc_param.width = camera->sub_width;
		enc_param.height = camera->sub_height;
		
		/*less than 12 fps is low fps*/
		if(sensor_fps <= 12)
			fps = sensor_fps;
		else
			fps = video->sub_fps;
		
		enc_param.fps = fps;
		enc_param.goplen = fps * video->sub_goplen;
		enc_param.bps = g_video_set.subbps;	//kbps
		//enc_param.profile = PROFILE_MAIN;	//default main profile
		enc_param.use_chn = ENCODE_SUB_CHN;
		enc_param.enc_grp = ENCODE_SUBCHN_NET;
		enc_param.br_mode = g_video_set.sub_videomode;

		if (NK_N1_VENC_CODEC_HEVC == g_video_set.sub_enctype || NK_N1_VENC_CODEC_HEVC_PLUS== g_video_set.sub_enctype){
			enc_param.enc_out_type = HEVC_ENC_TYPE;
			enc_param.profile = PROFILE_HEVC_MAIN;
			enc_param.minqp = video->sub_min_qp_265;
			enc_param.maxqp = video->sub_max_qp_265;
		}else {
			enc_param.enc_out_type = H264_ENC_TYPE;
			enc_param.profile = PROFILE_HIGH;
			enc_param.minqp = video->sub_min_qp_264;
			enc_param.maxqp = video->sub_max_qp_264;
		}

		if (NK_N1_VENC_CODEC_H264 == g_video_set.sub_enctype || NK_N1_VENC_CODEC_HEVC == g_video_set.sub_enctype)
			smart_cfg.smart_mode = 0;
		else
			smart_cfg.smart_mode = video->sub_smart_mode;
		
		smart_cfg.smart_goplen = video->sub_smart_goplen*enc_param.fps;
		if(g_video_set.sub_enctype == NK_N1_VENC_CODEC_H264 || g_video_set.sub_enctype == NK_N1_VENC_CODEC_H264_PLUS)
			smart_cfg.smart_quality = video->sub_smart_quality_264;
		else
			smart_cfg.smart_quality = video->sub_smart_quality_265;
		smart_cfg.smart_static_value = video->smart_static_value;
		ak_print_normal_ex("video->smart_static_value:%d\n",video->smart_static_value);

		venc_handle = ak_venc_open(&enc_param);

		if (NULL == venc_handle) {
			ak_print_error("venc_handle null, restart anyka_ipc!\n");
			ak_cmd_exec("killall -9 anyka_ipc", NULL, 0);
			return NULL;
		}

		ja_media_modify_venc_parm(idx, venc_handle, smart_cfg.smart_mode, enc_param.bps, enc_param.br_mode);
	}

	ak_venc_set_smart_config(venc_handle, &smart_cfg);

	return venc_handle;
}

/*
 * ja_media_venc_request_stream - start video stream, strart get stream thread
 * ja_ctrl[IN]: store video stream handle
 * idx[IN]: specific current channel
 * return: void
 */
void ja_media_venc_request_stream(JA_CTRL *ja_ctrl, int idx)
{
	if (idx >= VIDEO_CHN_NUM)
		return;

	ak_thread_mutex_lock(&pja_ctrl->media[idx].lock);

	if (ja_ctrl->media[idx].stream_handle) {
		ak_print_error_ex("chn %d is already opened\n", idx);
		ak_thread_mutex_unlock(&pja_ctrl->media[idx].lock);
		return;
	}

	ak_print_normal_ex("idx: %d\n", idx);
	ja_ctrl->media[idx].stream_handle = ak_venc_request_stream(
		ja_ctrl->media[idx].input_handle, ja_ctrl->media[idx].enc_handle);

	ak_thread_mutex_unlock(&pja_ctrl->media[idx].lock);

	/* request success, create get_stream_thread */
	if (ja_ctrl->media[idx].stream_handle) {
		ja_ctrl->media[idx].run_flg = AK_TRUE;
		ja_ctrl->media[idx].chn_idx = idx;
		ak_thread_create(&ja_ctrl->media[idx].str_th_id,
				ja_media_get_video_thread, &(ja_ctrl->media[idx]),
				ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	}
}


/*
 * ja_media_check_video_config - check n1 video config
 * video[IN]: video config
 * return: void
 */
static void ja_media_check_video_config(struct onvif_venc_config *video)
{
	if (video->main_kbps < MAIN_BPS_MIN) {
		video->main_kbps = MAIN_BPS_MIN;
	} else if (video->main_kbps > MAIN_BPS_MAX) {
		video->main_kbps = MAIN_BPS_MAX;
	}

	if (video->sub_kbps < SUB_BPS_MIN) {
		video->sub_kbps = SUB_BPS_MIN;
	} else if (video->sub_kbps > SUB_BPS_MAX) {
		video->sub_kbps = SUB_BPS_MAX;
	}

	if (video->main_fps < FPS_MIN) {
		video->main_fps = FPS_MIN;
	} else if (video->main_fps > FPS_MAX) {
		video->main_fps = FPS_MAX;
	}

	if (video->sub_fps < FPS_MIN) {
		video->sub_fps = FPS_MIN;
	} else if (video->sub_fps > FPS_MAX) {
		video->sub_fps = FPS_MAX;
	}

	if ((BR_MODE_CBR != video->main_video_mode)
		&& (BR_MODE_VBR != video->main_video_mode))
		video->main_video_mode = BR_MODE_VBR;

	if ((BR_MODE_CBR != video->sub_video_mode)
		&& (BR_MODE_VBR != video->sub_video_mode))
		video->sub_video_mode = BR_MODE_VBR;

	if ((NK_N1_VENC_CODEC_H264 != video->main_enc_type)
		&& (NK_N1_VENC_CODEC_HEVC != video->main_enc_type)
		&& (NK_N1_VENC_CODEC_HEVC_PLUS != video->main_enc_type)
		&& (NK_N1_VENC_CODEC_H264_PLUS != video->main_enc_type))
		video->main_enc_type = NK_N1_VENC_CODEC_HEVC;

	if ((NK_N1_VENC_CODEC_H264 != video->sub_enc_type)
		&& (NK_N1_VENC_CODEC_HEVC != video->sub_enc_type)
		&& (NK_N1_VENC_CODEC_HEVC_PLUS != video->sub_enc_type)
		&& (NK_N1_VENC_CODEC_H264_PLUS != video->sub_enc_type))
		video->sub_enc_type = NK_N1_VENC_CODEC_HEVC;

	if ((video->main_min_qp_264 < 10) || (video->main_min_qp_264 > 51))
		video->main_min_qp_264 = 28;
	if ((video->main_min_qp_265 < 10) || (video->main_min_qp_265 > 51))
		video->main_min_qp_265 = 28;
	
	if ((video->main_max_qp_264 < 10) || (video->main_max_qp_264 > 51))
		video->main_max_qp_264 = 48;
	if ((video->main_max_qp_265 < 10) || (video->main_max_qp_265 > 51))
		video->main_max_qp_265 = 48;
	
	if ((video->sub_min_qp_264 < 10) || (video->sub_min_qp_264 > 51))
		video->sub_min_qp_264 = 31;
	if ((video->sub_min_qp_265 < 10) || (video->sub_min_qp_265 > 51))
		video->sub_min_qp_265 = 31;
	
	if ((video->sub_max_qp_264 < 10) || (video->sub_max_qp_264 > 51))
		video->sub_max_qp_264 = 48;
	if ((video->sub_max_qp_265 < 10) || (video->sub_max_qp_265 > 51))
		video->sub_max_qp_265 = 48;

	onvif_config_set_venc_param(video);
	onvif_config_flush_data();
}

/*
 * ja_media_init_video - init n1 video resource
 * vi[IN]: video input handle
 * ja_ctrl[IN]: store video encode handle and other control resource
 * return: void
 */
void ja_media_init_video(void *vi, JA_CTRL *ja_ctrl)
{
	int i;

	struct onvif_venc_config *video = onvif_config_get_venc_param();

	ja_media_check_video_config(video);

	g_video_set.mainbps = video->main_kbps;
	g_video_set.mainfps = video->main_fps;

	g_video_set.subbps = video->sub_kbps;
	g_video_set.subfps = video->sub_fps;

	g_video_set.main_videomode = video->main_video_mode;
	g_video_set.sub_videomode = video->sub_video_mode;

	g_video_set.main_enctype = video->main_enc_type;
	ak_print_normal_ex("main encode type: %d\n", g_video_set.main_enctype);
	g_video_set.sub_enctype = video->sub_enc_type;
	ak_print_normal_ex("sub encode type: %d\n", g_video_set.sub_enctype);

	/* now, we just initialize two channel's resource */
	for (i = 0; i < VIDEO_CHN_NUM; i++) {
		/* venc open */
		ja_ctrl->media[i].type = 0;
		ja_ctrl->media[i].input_handle = vi;
		ak_thread_mutex_lock(&pja_ctrl->media[i].lock);
		if (NULL == ja_ctrl->media[i].enc_handle) {
			ja_ctrl->media[i].enc_handle = ja_media_venc_init(i);
		}
		ak_thread_mutex_unlock(&pja_ctrl->media[i].lock);
	}
}

/*
 * ja_media_close_video - close n1 video resource
 * ja_ctrl[IN]: store video encode handle and other control resource
 * return: void
 */
void ja_media_close_video(JA_CTRL *ja_ctrl)
{
	int i;

	/* now, we just initialize two channel's resource */
	for (i = 0; i < VIDEO_CHN_NUM; i++) {
		ja_media_venc_close(i);
	}
}

/**
* ja_media_alloc_audio_stream - allocate stream node memory
* len[IN]: length of stream->data
* return: pointer to struct aenc_entry * on success, failed NULL;
*/
struct aenc_entry *ja_media_alloc_audio_stream(unsigned int len)
{
	struct aenc_entry *a = (struct aenc_entry *)calloc(1, sizeof(struct aenc_entry));
	if (a) {
		a->stream.data = (unsigned char *)calloc(1, len);
		if (!a->stream.data) {
			ak_print_error_ex("alloc aenc_entry failed\n");
			free(a);
			a = NULL;
		}
	}

	return a;
}

/*
 * ja_media_free_audio_stream - release video stream node resource
 * v[IN]: pointer to audio stream node which will be release
 * free_data[IN]: free data or not
 * return: void
 */
void ja_media_free_audio_stream(struct aenc_entry *a, int free_data)
{
	if (a) {
		if (free_data && a->stream.data)
			free(a->stream.data);

		free(a);
		a = NULL;
	}
}

/*
 * ja_media_release_audio_stream - release audio stream node
 * idx[IN]: connect index, to indicate specifically connection
 * a[IN]: pointer to audio stream node which will be release
 * free_data[IN]: free data or not
 * return: void
 */
void ja_media_release_audio_stream(int idx, struct aenc_entry *a, int free_data)
{
	if (pja_ctrl->conn[idx].bValid) {
		/*
		 * TODO: delete audio stream from queue
		 */
		list_del(&a->list);
		ja_media_free_audio_stream(a, free_data);
	}
}

/**
 * ja_media_push_audio_data - push audio stream to active queue
 * stream[IN]: pointer to stream which will be disponse to queue
 * @return void
 */
void ja_media_push_audio_data(struct audio_stream *stream)
{
	int i = 0;

	for (i = 0; i < CONNECT_NUM_MAX; i++) {		
		if (pja_ctrl->conn[i].bValid && pja_ctrl->conn[i].audio_run_flag) {
			/*
			 * TODO: pust audio stream to queue
			 */
			struct aenc_entry *a = ja_media_alloc_audio_stream(stream->len);
			if (a) {
				memcpy(a->stream.data, stream->data, stream->len);
				a->stream.len = stream->len;
				a->stream.ts  = stream->ts;
				a->stream.seq_no = stream->seq_no;

				ak_thread_mutex_lock(&pja_ctrl->conn[i].audio_lock);
				list_add_tail(&a->list, &pja_ctrl->conn[i].audio_queue);
				ak_thread_mutex_unlock(&pja_ctrl->conn[i].audio_lock);
				pja_ctrl->conn[i].audio_stream_size += stream->len;

				//if queue is too large ,drop old stream data
				if (pja_ctrl->conn[i].audio_stream_size > AUDIO_STREAM_SIZE_MAX) {
					int drop_cnt = 0;
					struct aenc_entry *pos = NULL;
					struct aenc_entry *n = NULL;
					
					ak_thread_mutex_lock(&pja_ctrl->conn[i].audio_lock);
					list_for_each_entry_safe(pos, n, &pja_ctrl->conn[i].audio_queue, list) {
						pja_ctrl->conn[i].audio_stream_size -= pos->stream.len;
						ja_media_release_audio_stream(i, pos, AK_TRUE);
						drop_cnt++;
						
						if (pja_ctrl->conn[i].audio_stream_size < AUDIO_STREAM_SIZE_MAX) {
							ak_print_normal_ex("drop audio %d\n", drop_cnt);
							break;
						}
					}
					ak_thread_mutex_unlock(&pja_ctrl->conn[i].audio_lock);
				}
			}
		}
	}
}

/*
 * ja_media_aenc_cancel_stream - close audio encode
 * void
 * return: void
 */
void ja_media_aenc_cancel_stream(void)
{
	ak_thread_mutex_lock(&pja_ctrl->media[MEDIA_TYPE_AUDIO].lock);

	if (pja_ctrl->media[MEDIA_TYPE_AUDIO].stream_handle) {
		pja_ctrl->media[MEDIA_TYPE_AUDIO].run_flg = 0;
		if (pja_ctrl->media[MEDIA_TYPE_AUDIO].str_th_id)
			ak_thread_join(pja_ctrl->media[MEDIA_TYPE_AUDIO].str_th_id);
		/* TODO: release queue data */
		/* ... */

		ak_aenc_cancel_stream(pja_ctrl->media[MEDIA_TYPE_AUDIO].stream_handle);
		pja_ctrl->media[MEDIA_TYPE_AUDIO].stream_handle = NULL;
	}
	ak_thread_mutex_unlock(&pja_ctrl->media[MEDIA_TYPE_AUDIO].lock);
}

/*
 * ja_media_destroy_audio_stream_queue - destroy audio stream queue
 * idx[IN]: connect index, to indicate specifically connection
 * stream_queue[IN]: stream list head
 * return: void
 */
void ja_media_destroy_audio_stream_queue(int idx, struct list_head *stream_queue)
{
	struct aenc_entry *pos, *n;
	
	ak_thread_mutex_unlock(&pja_ctrl->conn[idx].audio_lock);
	list_for_each_entry_safe(pos, n, stream_queue, list) {
		pja_ctrl->conn[idx].audio_stream_size -= pos->stream.len;
		ja_media_release_audio_stream(idx, pos, AK_TRUE);
	}
	ak_thread_mutex_unlock(&pja_ctrl->conn[idx].audio_lock);
}

/*
 * ja_media_get_audio_thread - get audio stream thread
 * arg[IN]: thread argument
 * return: NULL
 */
void *ja_media_get_audio_thread(void *arg)
{
	struct aenc_entry *entry = NULL;
	struct aenc_entry *ptr = NULL;
	media_ctrl *media = (media_ctrl *)arg;
	long int tid = ak_thread_get_tid();
	ak_print_normal_ex("thread tid: %ld\n", tid);
	ak_thread_set_name("n1_audio");

	while (media->run_flg) {
		struct list_head stream_head = {0};
		INIT_LIST_HEAD(&stream_head);
		int ret = ak_aenc_get_stream(media->stream_handle, &stream_head);
		if (!ret) {
			list_for_each_entry_safe(entry, ptr, &stream_head, list) {
				if(entry) {
					ak_thread_mutex_lock(&media->str_lock);
					/* push stream to all active queue */
					ja_media_push_audio_data(&entry->stream);
					/* release for venc */
					ak_aenc_release_stream(entry);
					/* store to internal cache, dispose to all user */
					ak_thread_mutex_unlock(&media->str_lock);
					}
				}
		} else {
			/* get stream too fast, make a interval */
			ak_sleep_ms(20);
		}
	}

	ak_thread_exit();
	return NULL;
}

/*
 * ja_media_init_audio - init n1 audio resource
 * ai[IN]: audio input handle
 * ja_ctrl[IN]: store audio encode handle and other control resource
 * return: void
 */
void ja_media_init_audio(void *ai, JA_CTRL *ja_ctrl)
{
	struct onvif_audio_config* audio = onvif_config_get_audio();

	if (NK_N1_AUDIO_INPUT_MODE_MIC == audio->ai_source)
		ak_ai_set_source(ai, AI_SOURCE_MIC);
	else
		ak_ai_set_source(ai, AI_SOURCE_LINEIN);

	ak_ai_set_frame_interval(ai, AUDIO_DEFAULT_INTERVAL);

	/* open audio encode */
	struct audio_param aenc_param = {0};
	aenc_param.type = AK_AUDIO_TYPE_PCM_ALAW;
	aenc_param.sample_bits = 16;
	aenc_param.channel_num = 1;
	aenc_param.sample_rate = 8000;

	/* open aenc */
	ja_ctrl->media[MEDIA_TYPE_AUDIO].type = 1;
	ak_thread_mutex_lock(&pja_ctrl->media[MEDIA_TYPE_AUDIO].lock);
	if (NULL == ja_ctrl->media[MEDIA_TYPE_AUDIO].enc_handle) {
		ja_ctrl->media[MEDIA_TYPE_AUDIO].enc_handle = ak_aenc_open(&aenc_param);
	}
	ak_thread_mutex_unlock(&pja_ctrl->media[MEDIA_TYPE_AUDIO].lock);
	ja_ctrl->media[MEDIA_TYPE_AUDIO].input_handle = ai;
}

/*
 * ja_media_aenc_request_stream - start audio stream, strart get stream thread
 * ja_ctrl[IN]: store audio stream handle
 * return: void
 */
void ja_media_aenc_request_stream(JA_CTRL *ja_ctrl)
{
	ak_thread_mutex_lock(&pja_ctrl->media[MEDIA_TYPE_AUDIO].lock);

	if (ja_ctrl->media[MEDIA_TYPE_AUDIO].stream_handle) {
		ak_thread_mutex_unlock(&pja_ctrl->media[MEDIA_TYPE_AUDIO].lock);
		return;
	}

	ja_ctrl->media[MEDIA_TYPE_AUDIO].stream_handle = ak_aenc_request_stream(
		ja_ctrl->media[MEDIA_TYPE_AUDIO].input_handle, ja_ctrl->media[MEDIA_TYPE_AUDIO].enc_handle);
	ak_thread_mutex_unlock(&pja_ctrl->media[MEDIA_TYPE_AUDIO].lock);

	/* request success, create get_stream_thread */
	if (ja_ctrl->media[MEDIA_TYPE_AUDIO].stream_handle)
		ja_ctrl->media[MEDIA_TYPE_AUDIO].run_flg = AK_TRUE;
		ak_thread_create(&ja_ctrl->media[MEDIA_TYPE_AUDIO].str_th_id,
				ja_media_get_audio_thread, &(ja_ctrl->media[MEDIA_TYPE_AUDIO]),
				ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
}

/*
 * ja_media_close_audio - close audio encode
 * void
 * return: void
 */
void ja_media_close_audio(JA_CTRL *ja_ctrl)
{
	ak_thread_mutex_lock(&pja_ctrl->media[MEDIA_TYPE_AUDIO].lock);
	if (ja_ctrl->media[MEDIA_TYPE_AUDIO].stream_handle) {
		
		ja_ctrl->media[MEDIA_TYPE_AUDIO].run_flg = 0;
		if (ja_ctrl->media[MEDIA_TYPE_AUDIO].str_th_id)
			ak_thread_join(ja_ctrl->media[MEDIA_TYPE_AUDIO].str_th_id);

		ak_aenc_cancel_stream(ja_ctrl->media[MEDIA_TYPE_AUDIO].stream_handle);
		ja_ctrl->media[MEDIA_TYPE_AUDIO].stream_handle = NULL;
	}

	if (ja_ctrl->media[MEDIA_TYPE_AUDIO].enc_handle) {
		ak_aenc_close(ja_ctrl->media[MEDIA_TYPE_AUDIO].enc_handle);
		ja_ctrl->media[MEDIA_TYPE_AUDIO].enc_handle = NULL;
	}
	/* TODO: release queue data */
	for (int i=0; i<CONNECT_NUM_MAX; i++) {
		if ((pja_ctrl->conn[i].bValid) && (!list_empty(&pja_ctrl->conn[i].audio_queue)))
			ja_media_destroy_audio_stream_queue(i, &pja_ctrl->conn[i].audio_queue);
	}
		
	ak_thread_mutex_unlock(&pja_ctrl->media[MEDIA_TYPE_AUDIO].lock);
}

/*
 * ja_media_change_resolution - change resolution
 * width[IN]: width
 * height[IN]: height
 * chn[IN]: main or sub
 * return: void
 */
void ja_media_change_resolution(int width, int height, int chn)
{
	int flag[VIDEO_CHN_NUM] = {0};
	int i = 0;
	int change_flag = AK_FALSE;
	int ret = AK_FAILED;
	
	/* get sensor resolution */
	struct video_resolution sensor_res = {0};

	if (chn >= VIDEO_CHN_NUM)
		return;
	
	struct video_channel_attr attr;
	struct onvif_camera_config *camera = onvif_config_get_camera();
	struct onvif_camera_config camera_tmp = {0};
	
	memcpy(&camera_tmp, camera, sizeof(struct onvif_camera_config));

	if (VIDEO_CHN_MAIN == chn)
	{
		if ((camera_tmp.main_width != width)
			|| (camera_tmp.main_height != height)) {
			camera_tmp.main_width = width;
			camera_tmp.main_height = height;
			ak_print_normal_ex("main video size change to %dX%d!\n", width, height);
			change_flag = AK_TRUE;
		}
	}else {
		if ((camera_tmp.sub_width != width)
			|| (camera_tmp.sub_height != height)) {
			camera_tmp.sub_width = width;
			camera_tmp.sub_height = height;
			ak_print_normal_ex("sub video size change to %dX%d!\n", width, height);
			change_flag = AK_TRUE;
		}
	}

	if (!change_flag)
		return;

	memset(&attr, 0, sizeof(struct video_channel_attr));

	if (ak_vi_get_sensor_resolution(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, &sensor_res)) {
		ak_print_normal_ex("get_sensor_res failed!\n");
	}

	attr.res[VIDEO_CHN_MAIN].width = camera_tmp.main_width;
	attr.res[VIDEO_CHN_MAIN].height = camera_tmp.main_height;
	attr.res[VIDEO_CHN_SUB].width = camera_tmp.sub_width;
	attr.res[VIDEO_CHN_SUB].height = camera_tmp.sub_height;

	attr.res[VIDEO_CHN_MAIN].max_width = camera_tmp.main_max_width;
	attr.res[VIDEO_CHN_MAIN].max_height = camera_tmp.main_max_height;
	attr.res[VIDEO_CHN_SUB].max_width = camera_tmp.sub_max_width;
	attr.res[VIDEO_CHN_SUB].max_height = camera_tmp.sub_max_height;
	
	attr.crop.left = 0;
	attr.crop.top = 0;
	attr.crop.width = sensor_res.width;
	attr.crop.height = sensor_res.height;

	for (i=0; i<VIDEO_CHN_NUM; i++) {
		if (pja_ctrl->media[i].stream_handle) {
			flag[i] = 1;
		}
	}

	ak_print_normal_ex("osd exit!\n");
	ja_osd_exit();

	ak_print_normal_ex("close venc!\n");
	ja_media_close_video(pja_ctrl);

	ak_print_normal_ex("change size!\n");
	ret = ak_vi_change_channel_attr(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, &attr);

	if (AK_SUCCESS == ret) {
		memcpy(camera, &camera_tmp, sizeof(struct onvif_camera_config));
		onvif_config_set_camera(camera);
		onvif_config_flush_data();
	}
	
	ak_print_normal_ex("open venc!\n");
	ja_media_init_video(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, pja_ctrl);
	for (i=0; i<VIDEO_CHN_NUM; i++) {
		if (flag[i]) {
			ja_media_venc_request_stream(pja_ctrl, i);
		}
	}

	if (camera->osd_switch) {
		ak_print_normal_ex("osd init!\n");
		ja_osd_init(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle);
		ak_print_normal_ex("osd init ok!\n");
	}
}

/*
 * ja_media_modify_fps_and_goplen - modify fps , goplen and smart goplen
 * chn[IN]: main or sub
 * fps[IN]: fps
 * return: void
 */
static void ja_media_modify_fps_and_goplen(int chn, int fps)
{
	int framerate = fps;
	struct onvif_venc_config *video = onvif_config_get_venc_param();
	struct venc_smart_cfg smart_cfg = {0};
	
	if (chn >= VIDEO_CHN_NUM)
		return;

	if (VIDEO_CHN_MAIN == chn) {
		ak_venc_set_fps(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].enc_handle, framerate);
		ak_venc_set_gop_len(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].enc_handle, framerate * video->main_goplen);
		
		if (NK_N1_VENC_CODEC_H264 == g_video_set.main_enctype || NK_N1_VENC_CODEC_HEVC == g_video_set.main_enctype)
			smart_cfg.smart_mode = 0;
		else
			smart_cfg.smart_mode = video->main_smart_mode;
		
		smart_cfg.smart_goplen = framerate * video->main_smart_goplen;
		if(g_video_set.main_enctype == NK_N1_VENC_CODEC_H264 || g_video_set.main_enctype == NK_N1_VENC_CODEC_H264_PLUS)
			smart_cfg.smart_quality = video->main_smart_quality_264;
		else
			smart_cfg.smart_quality = video->main_smart_quality_265;
		smart_cfg.smart_static_value = video->smart_static_value;
		ak_venc_set_smart_config(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].enc_handle, &smart_cfg);

		ak_print_normal_ex("main chn fps: %d, goplen: %d smart_goplen : %d\n", framerate, framerate * video->main_goplen, smart_cfg.smart_goplen);
	} else {
		ak_venc_set_fps(pja_ctrl->media[MEDIA_TYPE_VIDEO_SUB].enc_handle, framerate);
		ak_venc_set_gop_len(pja_ctrl->media[MEDIA_TYPE_VIDEO_SUB].enc_handle, framerate * video->sub_goplen);
		
		if (NK_N1_VENC_CODEC_H264 == g_video_set.sub_enctype || NK_N1_VENC_CODEC_HEVC == g_video_set.sub_enctype)
			smart_cfg.smart_mode = 0;
		else
			smart_cfg.smart_mode = video->sub_smart_mode;
		
		smart_cfg.smart_goplen = framerate * video->sub_smart_goplen;
		if(g_video_set.sub_enctype == NK_N1_VENC_CODEC_H264 || g_video_set.sub_enctype == NK_N1_VENC_CODEC_H264_PLUS)
			smart_cfg.smart_quality = video->sub_smart_quality_264;
		else
			smart_cfg.smart_quality = video->sub_smart_quality_265;
		smart_cfg.smart_static_value = video->smart_static_value;
		ak_venc_set_smart_config(pja_ctrl->media[MEDIA_TYPE_VIDEO_SUB].enc_handle, &smart_cfg);

		ak_print_normal_ex("sub chn fps: %d, goplen: %d smart_goplen : %d\n", framerate, framerate * video->sub_goplen, smart_cfg.smart_goplen);
	}
}

/*
 * ja_media_modify_venc_parm - check and modify venc parm ,such as bps,  brmode(cbr/vbr)
 * chn[IN]: main or sub
 * enc_handle[IN]: enc_handle
 * smart_en[IN]: smart_en
 * bps[IN]: bps
 * brmode[IN]: cbr or vbr
 * return: void
 */
static void ja_media_modify_venc_parm(int chn, void *enc_handle, int smart_en, int bps, int brmode)
{
	int bitrate = bps;
	int videomode = brmode;
	int ratio = 0;
	struct onvif_venc_config *video = onvif_config_get_venc_param();

	if (chn >= VIDEO_CHN_NUM)
		return;

	if (NULL == enc_handle)
		return;

	if ((videomode < BR_MODE_CBR) || (videomode > BR_MODE_VBR))
		videomode = BR_MODE_VBR;

	if (smart_en)
		videomode = BR_MODE_VBR;

	if (VIDEO_CHN_MAIN == chn) {
		ak_venc_set_br(enc_handle, videomode);

		if (BR_MODE_VBR == videomode){
			switch (g_video_set.main_enctype) {
			case NK_N1_VENC_CODEC_H264:
				ratio = video->main_targetratio_264;
				break;
			case NK_N1_VENC_CODEC_H264_PLUS:
				ratio = video->main_targetratio_264_smart;
				break;
			case NK_N1_VENC_CODEC_HEVC:
				ratio = video->main_targetratio_265;
				break;
			case NK_N1_VENC_CODEC_HEVC_PLUS:
				ratio = video->main_targetratio_265_smart;
				break;
			default:
				ratio = video->main_targetratio_265;
				break;
			}

			ak_venc_set_kbps(enc_handle, bitrate * ratio / 100, bitrate);
		}
		else
			ak_venc_set_kbps(enc_handle, bitrate, bitrate);
		
	} else {
		ak_venc_set_br(enc_handle, videomode);

		if (BR_MODE_VBR == videomode){
			switch (g_video_set.sub_enctype) {
			case NK_N1_VENC_CODEC_H264:
				ratio = video->sub_targetratio_264;
				break;
			case NK_N1_VENC_CODEC_H264_PLUS:
				ratio = video->sub_targetratio_264_smart;
				break;
			case NK_N1_VENC_CODEC_HEVC:
				ratio = video->sub_targetratio_265;
				break;
			case NK_N1_VENC_CODEC_HEVC_PLUS:
				ratio = video->sub_targetratio_265_smart;
				break;
			default:
				ratio = video->sub_targetratio_265;
				break;
			}
			ak_venc_set_kbps(enc_handle, bitrate * ratio / 100, bitrate);
		}
		else
			ak_venc_set_kbps(enc_handle, bitrate, bitrate);
	}	
}

/*
 * ja_media_set_venc_parm - set venc parm ,such as bps, fps, mode(cbr/vbr)
 * chn[IN]: main or sub
 * bps[IN]: bps
 * fps[IN]: fps
 * mode[IN]: cbr or vbr
 * return: void
 */
void ja_media_set_venc_parm(int chn, int bps, int fps, int mode)
{
	int bitrate = bps;
	int framerate = fps;
	int videomode = mode;
	int smart_en = 0;
	
	struct onvif_venc_config *video = onvif_config_get_venc_param();
	
	if (chn >= VIDEO_CHN_NUM)
		return;

	if ((videomode < BR_MODE_CBR) || (videomode > BR_MODE_VBR))
		return;

	if (framerate < FPS_MIN)
		framerate = FPS_MIN;
	else if (framerate > FPS_MAX)
		framerate = FPS_MAX;

	if (VIDEO_CHN_MAIN == chn)
	{
		if (bitrate < MAIN_BPS_MIN)
			bitrate = MAIN_BPS_MIN;
		else if (bitrate > MAIN_BPS_MAX)
			bitrate = MAIN_BPS_MAX;

		if (NK_N1_VENC_CODEC_H264 == g_video_set.main_enctype || NK_N1_VENC_CODEC_HEVC == g_video_set.main_enctype)
			smart_en = 0;
		else
			smart_en = video->main_smart_mode;

		if (g_video_set.main_videomode != videomode) {
			g_video_set.main_videomode = videomode;
			ja_media_modify_venc_parm(chn, pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].enc_handle, smart_en, bitrate, videomode);
		}

		if (g_video_set.mainbps != bitrate) {			
			g_video_set.mainbps = bitrate;
			ja_media_modify_venc_parm(chn, pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].enc_handle, smart_en, bitrate, videomode);
		}

		if (g_video_set.mainfps != framerate) {
			g_video_set.mainfps = framerate;
			ja_media_modify_fps_and_goplen(chn, framerate);
		}
	}
	else
	{
		if (bitrate < SUB_BPS_MIN)
			bitrate = SUB_BPS_MIN;
		else if (bitrate > SUB_BPS_MAX)
			bitrate = SUB_BPS_MAX;

		if (NK_N1_VENC_CODEC_H264 == g_video_set.sub_enctype || NK_N1_VENC_CODEC_HEVC == g_video_set.sub_enctype)
			smart_en = 0;
		else
			smart_en = video->sub_smart_mode;

		if (g_video_set.sub_videomode != videomode) {
			g_video_set.sub_videomode = videomode;
			ja_media_modify_venc_parm(chn, pja_ctrl->media[MEDIA_TYPE_VIDEO_SUB].enc_handle, smart_en, bitrate, videomode);
		}
		
		if (g_video_set.subbps != bitrate) {
			g_video_set.subbps = bitrate;
			ja_media_modify_venc_parm(chn, pja_ctrl->media[MEDIA_TYPE_VIDEO_SUB].enc_handle, smart_en, bitrate, videomode);
		}

		if (g_video_set.subfps != framerate) {
			g_video_set.subfps = framerate;
			ja_media_modify_fps_and_goplen(chn, framerate);
		}
	}

	video->main_video_mode = g_video_set.main_videomode;
	video->sub_video_mode = g_video_set.sub_videomode;
	video->main_fps = g_video_set.mainfps;
	video->sub_fps = g_video_set.subfps;
	video->main_kbps = g_video_set.mainbps;
	video->sub_kbps = g_video_set.subbps;

    onvif_config_set_venc_param(video);
	onvif_config_flush_data();
}


/*
 * ja_media_reset_venc - reset venc
 * chn[IN]: main or sub
 * return: void
 */
void ja_media_reset_venc(int chn)
{
	int flag = 0;
		
	if (pja_ctrl->media[chn].stream_handle)
		flag = 1;
	
	ak_osd_ex_turn_off(chn);
	ak_print_normal_ex("close venc %d!\n", chn);
	ja_media_venc_close(chn);

	ak_print_normal_ex("open venc %d!\n", chn);
	ak_thread_mutex_lock(&pja_ctrl->media[chn].lock);
	if (NULL == pja_ctrl->media[chn].enc_handle)
		pja_ctrl->media[chn].enc_handle = ja_media_venc_init(chn);
	ak_thread_mutex_unlock(&pja_ctrl->media[chn].lock);
	if (flag) {
		ja_media_venc_request_stream(pja_ctrl, chn);
		ak_osd_ex_turn_on(chn);
	}
}

/*
 * ja_media_set_venc_parm - set venc type h.264/h.265
 * chn[IN]: main or sub
 * type[IN]: h.264/h.265
 * return: void
 */
void ja_media_set_venc_type(int chn, int type)
{
	int enctype = type;
	int change_flag = AK_FALSE;
	
	if (chn >= VIDEO_CHN_NUM)
		return;

	if ((enctype < NK_N1_VENC_CODEC_H264) || (enctype > NK_N1_VENC_CODEC_HEVC_PLUS))
		return;

	struct onvif_venc_config *video = onvif_config_get_venc_param();

	if (VIDEO_CHN_MAIN == chn)
	{
		if (g_video_set.main_enctype != enctype) {
			g_video_set.main_enctype = enctype;
			ak_print_normal_ex("main enctype change to %d!\n", g_video_set.main_enctype);
			change_flag = AK_TRUE;
		}
	}
	else
	{
		if (g_video_set.sub_enctype != enctype) {
			g_video_set.sub_enctype = enctype;
			ak_print_normal_ex("sub enctype change to %d!\n", g_video_set.sub_enctype);
			change_flag = AK_TRUE;
		}
	}

	if (!change_flag)
		return;

	video->main_enc_type = g_video_set.main_enctype;
	video->sub_enc_type = g_video_set.sub_enctype;

    onvif_config_set_venc_param(video);
	onvif_config_flush_data();

	ja_media_reset_venc(chn);
}

/*
 * ja_media_sn_save - sn save
 * sn[IN]:sn str
 * return: 0  fail, 1  success
 */
int ja_media_sn_save(char* sn)
{
	int ret = AK_FALSE;
	FILE *fd = NULL;
	int len = 0;

	if (NULL == sn)
	{
		ak_print_error_ex("param NULL!\n");
		return ret;
	}

	fd = fopen ("/etc/jffs2/sn.conf", "w+b");

	if (NULL == fd)
	{
		ak_print_error_ex("fd open failed!\n");
		return ret;
	}

	len = fwrite(sn, 1, strlen(sn), fd);


	if (len == strlen(sn))
	{
		ret = AK_TRUE;
	}
	else
	{
		ak_print_error_ex("write len err : %d, %d!\n", len, strlen(sn));
	}

	fclose(fd);

	return ret;
}

/*
 * ja_media_sn_read - sn read
 * sn[OUT]:sn str
 * return: 0  fail, 1  success
 */
int ja_media_sn_read(char* sn)
{
	int ret = AK_FALSE;
	FILE *fd = NULL;
	int len = 0;

	if (NULL == sn) {
		ak_print_error_ex("param NULL!\n");
		return ret;
	}

	fd = fopen ("/etc/jffs2/sn.conf", "rb");
	if (NULL == fd) {
		ak_print_error_ex("fd open failed!\n");
		return ret;
	}
	len = fread(sn, 1, 32, fd);
	fclose(fd);
	sn[len] = 0;
	ret = AK_TRUE;

	return ret;
}

/*
 * ja_media_match_sensor - match isp cfg file after set style id
 * isp_cfg_path[IN]:isp_cfg_path
 * return: void
 */
static int ja_media_match_sensor(const char *isp_cfg_path)
{
	int ret = AK_FAILED;
	char isp_file[255] = {0};
	char *tmp = NULL;
	char *isp_cfg = NULL;
	struct dirent *dir_ent = NULL;
	
	DIR *dir = opendir(isp_cfg_path);
	if (NULL == dir) {
		ak_print_normal_ex("it fails to open directory %s\n", isp_cfg_path);
		return ret;
	}

	char sensor_if[8] = {0};
	char tmpstr[8] = {0};
	int dvp_flag = 0;
	
	FILE *fp = fopen(SENSOE_IF_PATH, "r");
	if ((!fp) && (errno == ENOENT)) {
		ak_print_error_ex("cannot get sensor if, check your camera driver\n");
		closedir(dir);
		return ret;
	}

	fread(sensor_if, sizeof(sensor_if), 1, fp);
	fclose(fp);

	if (strstr(sensor_if, "mipi1"))
		strcpy(tmpstr, "mipi_1");
	else if (strstr(sensor_if, "mipi2"))
		strcpy(tmpstr, "mipi_2");
	else 
		dvp_flag = 1;

	ak_print_normal_ex("sensor_if:%s\n", sensor_if);

	while (NULL != (dir_ent = readdir(dir))) {
		if (!dir_ent->d_name)
			continue;

		/* fine next when we get dir */
        if ((dir_ent->d_type & DT_DIR)) {
            continue;
        }

		/* make sure use isp_*.conf file to match */
		tmp = strstr(dir_ent->d_name, "isp_");
		if (!tmp) {
			continue;
		}

		if ((!dvp_flag && !strstr(tmp, tmpstr))
			|| (dvp_flag && strstr(tmp, "mipi")))
			continue;

		isp_cfg = strstr(tmp, ".conf");
		if (!isp_cfg) {
			continue;
		}

		sprintf(isp_file, "%s%s", isp_cfg_path, dir_ent->d_name);
		/* get sensor id, match config file */
		if(AK_SUCCESS == ak_vi_match_sensor(isp_file)){
			ak_print_notice_ex("ak_vi_match_sensor OK\n");
			ret = AK_SUCCESS;

			if (strcmp(isp_cfg_path, FIRST_PATH))
			{
				char cmd[128] = {0};
				char result[2] = {0};

				sprintf(cmd, "cp %s %s", isp_file, FIRST_PATH);
				ak_cmd_exec(cmd, result, 2);

				sprintf(isp_file, "%s%s", FIRST_PATH, dir_ent->d_name);
				ak_vi_match_sensor(isp_file);
			}
			break;
		}
	}
	closedir(dir);

	return ret;
}

/*
 * ja_media_get_styleId - get style id
 * return: style id
 */
int ja_media_get_styleId(void)
{
	int styleId = 0;

	ak_vpss_effect_get(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_STYLE_ID, &styleId);
	return styleId;
}

/*
 * ja_media_set_styleId - set style id
 * styleId[IN]:style id
 * return: void
 */
void ja_media_set_styleId(int styleId)
{
	int mode = ak_vi_get_work_scene(VIDEO_DEV0);

	ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_STYLE_ID, styleId);

	if (AK_FAILED == ja_media_match_sensor(FIRST_PATH)) {
		if (AK_FAILED == ja_media_match_sensor(BACK_PATH)) {
			ak_print_error_ex("match_sensor BACK_PATH failed\n");
			ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_STYLE_ID, 0);
			if (AK_FAILED == ja_media_match_sensor(BACK_PATH)) {
				ak_print_error_ex("match_sensor BACK_PATH style 0 failed\n");
				return;
			}
		}
	}

	ak_vi_switch_mode(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, mode);
}

/*
 * ja_media_init_ir_switch - init ir switch
 * void
 * return: 0 success, -1 fail
 */
int ja_media_init_ir_switch(void)
{
	if (ja_ir.ir_run_flag)
		return AK_SUCCESS;

	ja_ir.vi_handle = pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle;
	ja_ir.ir_run_flag = AK_TRUE;
	ja_ir.cur_state = -1;

	return ak_thread_create(&ja_ir.ir_tid, check_ir_switch,
			NULL, 100 *1024, -1);
}

/*
 * ja_media_set_video_day_night - set day night mode
 * vi_handle[IN]: opened vi handle
 * ir_val[IN]: 1 day, 0 night
 * return: 0 success, -1 fail
 */
int ja_media_set_video_day_night(void *vi_handle, int ir_val)
{	
	if (!vi_handle) {
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	if (ir_val) {
		ak_print_normal_ex("now set to day\n");
		ak_drv_ir_set_ircut(!ir_val);
		ret = ak_vi_switch_mode(vi_handle, VI_MODE_DAY);
		ak_drv_irled_set_working_stat(0);
	} else {
		ak_print_normal_ex("now set to night\n");
		ak_drv_irled_set_working_stat(1);
		ret = ak_vi_switch_mode(vi_handle, VI_MODE_NIGHT);
		ak_drv_ir_set_ircut(!ir_val);
	}

	ak_sleep_ms(300);

	ja_ir.cur_state = ir_val;

	return ret;
}

/*
 * ja_media_set_switch_ir - set ir auto switch enable
 * enable[IN]: 1 enable auto switch
 * return: void
 */
void ja_media_set_switch_ir(int enable)
{
	ak_print_normal_ex("set ir switch %s\n", enable == 1 ? "enable" : "disable");
	ja_ir.ir_switch_enable = enable;
}

/*
 * ja_media_destroy_ir_switch - destroy ir switch
 * return: void
 */
void ja_media_destroy_ir_switch(void)
{
	ja_ir.ir_run_flag = AK_FALSE;

    ak_print_notice_ex("join check_ir_switch thread...\n");
	ak_thread_join(ja_ir.ir_tid);
	ak_print_notice_ex("check_ir_switch thread join OK\n");
}

/*
 * ja_media_check_video_conn_stat - check video conn stat, for display osd
 * pja_ctrl[IN]: pja_ctrl handle
 * ch[IN]: ch id, 0 main, 1 sub
 * conn[IN]: conn id
 * return: void
 */
void ja_media_check_video_conn_stat(JA_CTRL *pja_ctrl, int ch, int conn)
{
	int i = 0;

	if ((VIDEO_CHN_MAIN == ch) && (conn == pja_ctrl->video_stat_conn[ch])){
		if (pja_ctrl->main_cnt > 0){
			for (i=0; i<CONNECT_NUM_MAX; i++) {
				if ((pja_ctrl->conn[i].bMainStream ) && pja_ctrl->conn[i].video_run_flag && (i != conn)){
					pja_ctrl->video_stat_conn[ch] = i;
					break;
				}
			}
		}
		else{
			pja_ctrl->video_stat_conn[ch] = -1;
		}
	}
	else if ((VIDEO_CHN_SUB == ch) && (conn == pja_ctrl->video_stat_conn[ch])){
		if (pja_ctrl->sub_cnt > 0){
			for (i=0; i<CONNECT_NUM_MAX; i++) {
				if ((!pja_ctrl->conn[i].bMainStream ) && pja_ctrl->conn[i].video_run_flag && (i != conn)){
					pja_ctrl->video_stat_conn[ch] = i;
					break;
				}
			}
		}
		else{
			pja_ctrl->video_stat_conn[ch] = -1;
		}
	}
}

/*
 * ja_media_irled_init - init irled
 * void
 * return: 0 success, -1 fail
 */
int ja_media_irled_init(void)
{
	struct ak_drv_irled_hw_param param;
	struct onvif_image_config *image = onvif_config_get_image();

	param.irled_working_level = image->irled_mode;
	return ak_drv_irled_init(&param);
}

/*
 * ja_media_init_img_effect - init img effect
 * vi[IN]: opened vi handle
 * return: void
 */
void ja_media_init_img_effect(void *vi)
{
	struct onvif_image_config* img = onvif_config_get_image();

	ak_vpss_effect_set(vi, VPSS_EFFECT_HUE, img->hue - IMG_EFFECT_DEF_VAL);
	ak_vpss_effect_set(vi, VPSS_EFFECT_BRIGHTNESS, img->brightness - IMG_EFFECT_DEF_VAL);
	ak_vpss_effect_set(vi, VPSS_EFFECT_SATURATION, img->saturation - IMG_EFFECT_DEF_VAL);
	ak_vpss_effect_set(vi, VPSS_EFFECT_CONTRAST, img->contrast - IMG_EFFECT_DEF_VAL);
	ak_vpss_effect_set(vi, VPSS_EFFECT_SHARP, img->sharp - IMG_EFFECT_DEF_VAL);

	ak_vi_set_flip_mirror(vi, img->flip, img->mirror);
	ak_vpss_set_force_anti_flicker_flag(vi, img->force_anti_flicker_flag);
}

/*
 * ja_media_start_check_fps - create fps switch thread
 * return: 0 success; -1 failed
 */
int ja_media_start_check_fps(void)
{
	if (ja_media_fps.fps_run_flag)
		return AK_SUCCESS;

	ja_media_fps.vi_handle = pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle;
	ja_media_fps.fps_run_flag = AK_TRUE;

	return ak_thread_create(&ja_media_fps.fps_tid, check_fps_switch,
			NULL, 100 *1024, -1);
}

/*
 * ja_media_stop_check_fps - join fps switch thread
 * return: void
 */
void ja_media_stop_check_fps(void)
{
	ja_media_fps.fps_run_flag = AK_FALSE;

    ak_print_notice_ex("join fps_switch thread...\n");
	ak_thread_join(ja_media_fps.fps_tid);
	ak_print_notice_ex("fps_switch thread join OK\n");
}

