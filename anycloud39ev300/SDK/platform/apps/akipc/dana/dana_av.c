#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <ctype.h>
#include "danavideo.h"
#include "ak_global.h"
#include "ak_common.h"
#include "ak_error.h"
#include "ak_config.h"
#include "ak_ring_buffer.h"
#include "ak_venc.h"
#include "ak_ai.h"
#include "ak_aenc.h"
#include "ak_ao.h"
#include "ak_adec.h"
#include "ak_osd_ex.h"
#include "ak_misc.h"

#include "dana_osd.h"
#include "dana_av.h"

#define DANA_RECV_DATA_DEBUG		0
#define DANA_AUDIO_TIME_DEBUG		0
#define DANA_SHOW_VSTREAM_INFO		0

/* we send stream at least 4K */
#define DANA_AUDIO_SEND_LEN			(4*1024)
/* dana audio talk receice ring buffer size */
#define DANA_AUDIO_RB_LEN			(8*1024)

#define HIGH_BPS 75
#define NORMAL_BPS 44

/* dana video stream buff max size */
#define VIDEO_STREAM_SIZE_MAX		(1*1024*1024)

/* video data list which send to danale*/
struct dana_video_entry {
	struct video_stream stream;
	struct list_head list;
};

/* data struct which control audio to interface danale */
struct dana_request_audio {
	unsigned int count; // how many user user
	int run_flag;
	void *dev;			//ai / ao handle
	void *codec;		//aenc / adec handle
	int encode_type;       // audio encode type
	void *stream;		//aenc / adec stream handle
	ak_pthread_t tid;	//thread id
	cb_t cb_info[MAX_USER];
	ak_sem_t sem;
	int req_change;
};

/* data struct which control video to interface danale */
struct dana_request_video {
	unsigned int count; // how many user user
	unsigned char run_flag;
	unsigned char chn_id;	//channel ID
	ak_pthread_t tid;		//thread ID
	cb_t cb_info[MAX_USER];
	ak_sem_t sem;			//request stream sem
	ak_mutex_t lock;
	void *venc;				//venc handle
	int encode_type;       // video encode type
	void *stream;			//stream handle
	unsigned int stream_size;
	struct list_head send_head;	//send stream data list head
};

/* data struct which control video & audio to interface danale */
struct ak_dana_av_t {
	int vi_run_f; //1 for run_ , 0 for stop
	void *vi_handle;
	ak_pthread_t vi_tid; //tid for do_vi_data thread
	ak_sem_t vi_sem;
	int video_req_change; //has new request set 1 otherwire set 0
	int av_init_f;  //not use
	ak_mutex_t lock;
	struct dana_request_audio ai;
	struct dana_request_audio ao;
	struct dana_request_video video[VIDEO_CHN_NUM];

	/* receive audio talk data from dana, then write into ring buffer */
	void *ao_rb;
	/* use for control video and audio send speed */
	unsigned long long video_last_ts;
};

static struct ak_dana_av_t av_ctrl = {0};

#if DANA_RECV_DATA_DEBUG
static FILE *dana_ai_fp = NULL;
#endif

static void dana_venc_set_bps(void *venc_handle, int chn)
{
	int target_kbps = 0, max_kbps = 0;
	struct venc_smart_cfg smart = {0};
	struct video_config *config = ak_config_get_sys_video();      //get system video config

	/* VBR only */
	if (chn == VIDEO_CHN_MAIN && config->main_video_mode == BR_MODE_VBR) {
		max_kbps = config->main_max_kbps;
		/* smart mode, 1 or 2 */
		if (config->main_smart_mode != 0) {
			smart.smart_mode = config->main_smart_mode;
			smart.smart_goplen = config->main_smart_goplen;
			smart.smart_static_value = config->main_smart_static_value;
			switch (config->main_enc_type) {
				case H264_ENC_TYPE:
					smart.smart_quality = config->main_smart_quality_264;
					target_kbps = config->main_smart_target_ratio_264* max_kbps / 100;
					break;
				case HEVC_ENC_TYPE:
					smart.smart_quality = config->main_smart_quality_265;
					target_kbps = config->main_smart_target_ratio_265 * max_kbps / 100;
					break;
				default:
					break;
			}
		} else {	/* normal module, just set ratio of target bps and max bps  */
			switch (config->main_enc_type) {
				case H264_ENC_TYPE:
					target_kbps = config->main_target_ratio_264 * max_kbps / 100;
					break;
				case HEVC_ENC_TYPE:
					target_kbps = config->main_target_ratio_265 * max_kbps / 100;
					break;
				default:
					break;
			}
		}
	} else if (chn == VIDEO_CHN_SUB && config->sub_video_mode == BR_MODE_VBR) {
		max_kbps = config->sub_max_kbps;
		/* smart mode, 1 or 2 */
		if (config->sub_smart_mode != 0) {
			smart.smart_mode = config->sub_smart_mode;
			smart.smart_goplen = config->sub_smart_goplen;
			smart.smart_static_value = config->sub_smart_static_value;
			switch (config->sub_enc_type) {
				case H264_ENC_TYPE:
					smart.smart_quality = config->sub_smart_quality_264;
					target_kbps = config->sub_smart_target_ratio_264 * max_kbps / 100;
					break;
				case HEVC_ENC_TYPE:
					smart.smart_quality = config->sub_smart_quality_265;
					target_kbps = config->sub_smart_target_ratio_265 * max_kbps / 100;
					break;
				default:
					break;
			}
		} else {	/* normal module, just set ratio of target bps and max bps  */
			switch (config->sub_enc_type) {
				case H264_ENC_TYPE:
					target_kbps = config->sub_target_ratio_264 * max_kbps / 100;
					break;
				case HEVC_ENC_TYPE:
					target_kbps = config->sub_target_ratio_265 * max_kbps / 100;
					break;
				default:
					break;
			}
		}
	} else {
		/* do nothing */
		return;
	}

	ak_venc_set_kbps(venc_handle, target_kbps, max_kbps);
	ak_venc_set_smart_config(venc_handle, &smart);
}

/**
 * dana_venc_init - init video encode
 * @channel[IN]:  0 or 1
 * return: 0 success; -1 failed
 */
static void* dana_venc_init(int channel)
{
	struct encode_param param = {0};
	struct video_config *video_config = ak_config_get_sys_video();      //get system video config
	struct camera_disp_config *camera = ak_config_get_camera_info();    //get camera config

	int video_type = 0;
	int enc_type = channel ? video_config->sub_enc_type : video_config->main_enc_type;
	switch (enc_type) {
		case H264_ENC_TYPE:
			video_type = H264;
			break;
		case HEVC_ENC_TYPE:
			video_type = H265;
			break;
		default:
			ak_print_error_ex("encode_type failed\n");
			break;
	}
	av_ctrl.video[channel].encode_type = video_type;

	/*init video channel param*/
	switch (channel) {
	case VIDEO_CHN_MAIN:
		param.width = camera->main_width;
		param.height = camera->main_height;
		param.minqp = video_config->main_min_qp;
		param.maxqp = video_config->main_max_qp;
		param.fps = video_config->main_fps;
		param.goplen = param.fps * video_config->main_gop_len;
		param.bps = video_config->main_min_kbps;	//kbps
		param.use_chn = ENCODE_MAIN_CHN;
		param.enc_grp = ENCODE_MAINCHN_NET;
		param.br_mode = video_config->main_video_mode;

		switch (video_config->main_enc_type) {
		case H264_ENC_TYPE:                        //h264 encode type
			param.profile = PROFILE_MAIN;
			param.enc_out_type = H264_ENC_TYPE;
			break;
		case HEVC_ENC_TYPE:                         //h265 encode type
			param.profile = PROFILE_HEVC_MAIN;
			param.enc_out_type = HEVC_ENC_TYPE;
			break;
		default:
			break;
		}
		ak_print_notice_ex("main channel net, main_enc_type=%d\n",
			video_config->main_enc_type);

		break;
	case VIDEO_CHN_SUB:
		param.width = camera->sub_width;
		param.height = camera->sub_height;
		param.minqp = video_config->sub_min_qp;
		param.maxqp = video_config->sub_max_qp;
		param.fps = video_config->sub_fps;
		param.goplen = param.fps * video_config->sub_gop_len;
		param.bps = video_config->sub_min_kbps;	//kbps
		param.use_chn = ENCODE_SUB_CHN;
		param.enc_grp = ENCODE_SUBCHN_NET;
		param.br_mode = video_config->sub_video_mode;

		switch (video_config->sub_enc_type) {
		case H264_ENC_TYPE:
			param.profile = PROFILE_MAIN;
			param.enc_out_type = H264_ENC_TYPE;
			break;
		case HEVC_ENC_TYPE:
			param.profile = PROFILE_HEVC_MAIN;
			param.enc_out_type = HEVC_ENC_TYPE;
			break;
		default:
			break;
		}
		ak_print_notice_ex("sub channel net, sub_enc_type=%d\n",
			video_config->sub_enc_type);
		break;
	default:
		return NULL;
	}

	return ak_venc_open(&param);
}

int request_set_i_frame(void)
{
	if(av_ctrl.video[VIDEO_CHN_MAIN].venc != NULL)
	{
		ak_venc_set_iframe(av_ctrl.video[VIDEO_CHN_MAIN].venc);
	}

	if(av_ctrl.video[VIDEO_CHN_SUB].venc != NULL)
	{
		ak_venc_set_iframe(av_ctrl.video[VIDEO_CHN_SUB].venc);
	}
	return 0;
}

/**
 * request_video_stream - request one channel's stream
 * @video_channel[IN]: 	VIDEO_CHN_MAIN | VIDEO_CHN_SUB,
 * return: AK_SUCCESS: cancel success | AK_FAILED: cancel failed
 */
static int request_video_stream(enum video_channel chn)
{
	/* ENCODE_MAINCHN_NET venc have req and not get venc stream handle */
	if ((av_ctrl.video[chn].count > 0) &&
			(NULL == av_ctrl.video[chn].stream)) {
		ak_print_notice_ex("request_stream enter\n");

		av_ctrl.video[chn].stream = ak_venc_request_stream(av_ctrl.vi_handle,
			   	av_ctrl.video[chn].venc);
		if (av_ctrl.video[chn].stream) {
			/* first frame should be I-frame */
			if (ak_venc_set_iframe(av_ctrl.video[chn].venc) < 0) {  //set next encode frame to I frame
				ak_print_error_ex("ak_venc_set_iframe failed\n");
				return AK_FAILED;
			}
			av_ctrl.video[chn].run_flag = AK_TRUE;
			ak_thread_sem_post(&(av_ctrl.video[chn].sem));
		} else {
			ak_print_error_ex("ak_venc_request_stream failed\n");
			return AK_FAILED;
		}

		ak_print_normal_ex("request_stream net OK\n\n");
	}

	return AK_SUCCESS;
}

/**
 * free_one_video_stream - free one video frame from buff list
 * @video[IN]:  dana video handle
 * @ventry[IN]:  one video frame node
 * return: void
 */
void free_one_video_stream(struct dana_request_video *video, struct dana_video_entry *ventry)
{
	list_del_init(&(ventry->list));
	if (video->stream_size >= ventry->stream.len)
		video->stream_size -= ventry->stream.len;
	else
	/* if video->stream_size < ventry->stream.len, some is wrong. notice it */
		ak_print_warning_ex("buff total:%d < one stream:%d\n",
			video->stream_size, ventry->stream.len);
	ak_venc_release_stream(video->stream, &(ventry->stream));
	free(ventry);
}

/**
 * free_stream_buf - free one channel's buff list
 * @video_channel[IN]: 	VIDEO_CHN_MAIN | VIDEO_CHN_SUB,
 * return: void
 */
static void free_stream_buf(enum video_channel chn)
{
	struct dana_video_entry *ventry = NULL;
	struct dana_video_entry *vptr = NULL;

	ak_thread_mutex_lock(&av_ctrl.video[chn].lock);
	list_for_each_entry_safe(ventry, vptr, &(av_ctrl.video[chn].send_head), list) {
		free_one_video_stream(&(av_ctrl.video[chn]), ventry);
		ventry = NULL;
	}
	ak_thread_mutex_unlock(&av_ctrl.video[chn].lock);
	if (0 != av_ctrl.video[chn].stream_size)
		ak_print_warning_ex("video%d list buff total:%d \n",chn, av_ctrl.video[chn].stream_size);
	av_ctrl.video[chn].stream_size = 0;
}

/**
 * cancel_video_stream - free one channel's stream
 * @video_channel[IN]: 	VIDEO_CHN_MAIN | VIDEO_CHN_SUB,
 * return: AK_SUCCESS: cancel success | AK_FAILED: cancel failed
 */
static int cancel_video_stream(enum video_channel chn)
{
	/* chn venc no req and have venc stream handle */
	if ((0 == av_ctrl.video[chn].count) &&
			(NULL != av_ctrl.video[chn].stream)) {
		ak_print_normal_ex("cancel_stream enter\n");

		av_ctrl.video[chn].run_flag = AK_FALSE;
		ak_sleep_ms(20);

		/* video data donot send out and free it*/
		free_stream_buf(chn);
		if(ak_venc_cancel_stream(av_ctrl.video[chn].stream)){
			ak_print_error_ex("cancel stream failed\n");
			return AK_FAILED;
		}

		av_ctrl.video[chn].stream = NULL;
		ak_print_normal_ex("cancel_stream net OK\n");
	}
	return AK_SUCCESS;
}

/**
 * video_stream_to_dana - thread, send video data to danale
 * return: null
 */
static void video_stream_to_dana(void)
{
	cb_t ch_main_cb[MAX_USER];
	cb_t ch_sub_cb[MAX_USER];
	unsigned int main_count = 0;
	unsigned int sub_count = 0;
	int i_frame_flag[VIDEO_CHN_NUM] = {0, 0};
	int frame_count[VIDEO_CHN_NUM] = {0, 0};
	/*
	 * copy var value from global to local.
	 *  we change it outside easily and safely.
	 */
	ak_thread_mutex_lock(&av_ctrl.lock);
	for(int i = 0; i < MAX_USER; i++){
		if(NULL != av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].cb_func){
			ch_main_cb[main_count].cb_func = av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].cb_func;
			ch_main_cb[main_count].pri_data = av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].pri_data;
			main_count ++;
		}
		if(NULL != av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].cb_func){
			ch_sub_cb[sub_count].cb_func = av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].cb_func;
			ch_sub_cb[sub_count].pri_data = av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].pri_data;
			sub_count ++;
		}
	}
	ak_thread_mutex_unlock(&av_ctrl.lock);
	ak_print_normal_ex("channel usr count.main:%d sub:%d \n",main_count,sub_count);
	int chn = VIDEO_CHN_MAIN;

	struct dana_video_entry *ventry = NULL;
	struct camera_disp_config *camera = ak_config_get_camera_info();
	int osd_rate = camera->rate_position;
	//ak_print_normal_ex("date:%s time:%s \n",__DATE__,__TIME__);
	while (av_ctrl.vi_run_f) {
		for (chn = VIDEO_CHN_MAIN; chn <VIDEO_CHN_NUM; chn++) {
			/*
			  * list_for_each_entry_safe(ventry, vptr,	&(av_ctrl.video[chn].send_head), list)
			  * replace withc list_first_entry_or_null, because of av_ctrl.video[chn].lock
			  */
			while (1) {
				/*  pop data */
				ak_thread_mutex_lock(&av_ctrl.video[chn].lock);
				ventry = list_first_entry_or_null(&(av_ctrl.video[chn].send_head),
					struct dana_video_entry, list);
				if (NULL == ventry){
					ak_thread_mutex_unlock(&av_ctrl.video[chn].lock);
					break;
				}
				list_del_init(&(ventry->list));
				av_ctrl.video[chn].stream_size -= ventry->stream.len;
				ak_thread_mutex_unlock(&av_ctrl.video[chn].lock);

				/* display video info on screen */
				if (osd_rate) {
					ak_osd_ex_stat_video(chn, av_ctrl.video[chn].stream);
				}
				/* firstly send I-frame and then P-frame */
				if (0 == i_frame_flag[chn]) {
					if (FRAME_TYPE_I == ventry->stream.frame_type) {
						i_frame_flag[chn] = 1;
						ak_print_normal_ex("chn:%d discard P_F count:%d\n",
								chn, frame_count[chn]);
					}
					frame_count[chn]++;
				}

				if (VIDEO_CHN_MAIN == chn) {
					for (int i=0; i < main_count; i++) {
						/* I_frame not find and it is  preview , donot send */
						if ((0 == i_frame_flag[VIDEO_CHN_MAIN])	&& ch_main_cb[i].pri_data)
							goto free_stream;

						/*use dana api send video data to dana cloud*/
						if (ch_main_cb[i].cb_func)
							((net_send_func)ch_main_cb[i].cb_func)(ch_main_cb[i].pri_data,
								&(ventry->stream), av_ctrl.video[0].encode_type);
					}
				} else if (VIDEO_CHN_SUB == chn) {
					for (int i=0; i < sub_count; i++) {
						/* I_frame not find and it is  preview , donot send */
						if ((0 == i_frame_flag[VIDEO_CHN_SUB]) && ch_sub_cb[i].pri_data)
							goto free_stream;

						/*use dana api send video data to dana cloud*/
						if (ch_sub_cb[i].cb_func)
							((net_send_func)ch_sub_cb[i].cb_func)(ch_sub_cb[i].pri_data,
								&(ventry->stream), av_ctrl.video[1].encode_type);
					}
				}

				/* mark video send ts: use for sync audio send speed */
				av_ctrl.video_last_ts = ventry->stream.ts;
free_stream:
				//free_one_video_stream(&(av_ctrl.video[chn]), ventry);
				ak_venc_release_stream(av_ctrl.video[chn].stream, &(ventry->stream));
				free(ventry);
				ventry = NULL;

				/*make sure it exit quickly*/
				if (av_ctrl.video_req_change) {
					break;
				}
			}
		}

		/* exit check */
		if (av_ctrl.video_req_change) {
			av_ctrl.video_req_change = 0;
			break;
		} else
			ak_sleep_ms(10);
	}
	ak_print_normal_ex("exit\n");
}

/**
 * get_video_stream_thread - thread, get video data from venc
 * return: null
 */
static void* get_video_stream_thread(void *arg)
{
	struct dana_video_entry *ventry = NULL, *ventry_prevIframe = NULL;
	struct dana_video_entry *vptr = NULL;
	struct dana_request_video *video = (struct dana_request_video *)arg;

	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	ak_thread_set_name("dana_get_video");

	while (av_ctrl.vi_run_f) {
		ak_print_normal_ex("chn=%d, sleep...\n", video->chn_id);
		ak_thread_sem_wait(&(video->sem));
		ak_print_normal_ex("chn=%d, wakeup...\n", video->chn_id);
		/*
		 * get video data from venc.
		 * if stream cancel, run_flag set 0 and break while.
		 */
		while (av_ctrl.vi_run_f && video->run_flag) {
			if (video->stream_size > VIDEO_STREAM_SIZE_MAX) {                                       //if queue is too large ,drop old stream data
				ventry = NULL;
				vptr = NULL;
				ak_thread_mutex_lock(&(video->lock));
				list_for_each_entry_safe(ventry, vptr,&(video->send_head), list) {
					if (ventry->stream.frame_type == FRAME_TYPE_I) {                                //Now frame is I frame
						if(ventry_prevIframe != NULL) {                                             //previous I frame point is not NULL
							free_one_video_stream(video, ventry_prevIframe);                        //free the previous I frame
							ventry_prevIframe = NULL;
						} else {                                                                    //mark ventry point address
							ventry_prevIframe = ventry;
						}
					} else {                                                                        // P or B frame then free immediately
						free_one_video_stream(video, ventry);
					}
					if (video->stream_size <= VIDEO_STREAM_SIZE_MAX) {                              //loop until stream_size less than VIDEO_STREAM_SIZE_MAX
						ventry_prevIframe = NULL;                                                   //set NULL when break.only set this point in same loop
						break;
					}
				}
				ak_thread_mutex_unlock(&(video->lock));
			}
			ventry = (struct dana_video_entry *)calloc(1,
				sizeof(struct dana_video_entry));
			if(NULL == ventry) {
				ak_print_error_ex("calloc ventry failed\n");
				break;
			}
			/* get video data from venc */
			if (ak_venc_get_stream(video->stream, &(ventry->stream))) {
				free(ventry);
				ventry = NULL;
			} else {
#if DANA_SHOW_VSTREAM_INFO
				ak_print_normal_ex("chn=%d, frame_type=%d, ts=%llu, len=%u, seq_no=%lu\n\n",
					video->chn_id, ventry->stream.frame_type,
					ventry->stream.ts, ventry->stream.len, ventry->stream.seq_no);
#endif
				ak_thread_mutex_lock(&(video->lock));
				/* save video data in list and send it to danale by another thread*/
				list_add_tail(&(ventry->list), &(video->send_head));
				/*refresh stream buff size*/
				video->stream_size += ventry->stream.len;
				ak_thread_mutex_unlock(&(video->lock));
				continue;
			}

			ak_sleep_ms(10);
		}
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

/**
 * do_vi_data - thread, send video data to danale
 * return: null
 */
static void* do_vi_data(void *arg)
{
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	ak_thread_set_name("dana_vi");

	while (av_ctrl.vi_run_f) {
		if((0 == av_ctrl.video[VIDEO_CHN_MAIN].count) &&
				(0 == av_ctrl.video[VIDEO_CHN_SUB].count)) {
			ak_print_normal_ex("sleep...\n");
			ak_thread_sem_wait(&av_ctrl.vi_sem);
			ak_print_normal_ex("wakeup...\n");
			av_ctrl.video_req_change = 0;
		}
		/*check if request main channel video stream*/
		if (request_video_stream(VIDEO_CHN_MAIN)) {
			ak_print_error_ex("request_main_stream fail\n");
		}
		/*check if request sub channel video stream*/
		if (request_video_stream(VIDEO_CHN_SUB)) {
			ak_print_error_ex("request_sub_stream fail\n");
		}
		/*no stream and then go to wait*/
		if((NULL == av_ctrl.video[VIDEO_CHN_MAIN].stream)
			&& (NULL == av_ctrl.video[VIDEO_CHN_SUB].stream)){
			continue;
		}
		/*send video data to danale*/
		video_stream_to_dana();

		ak_print_notice_ex("ready to cancel & request stream...\n");
		/*check if cancel main channel video stream*/
		cancel_video_stream(VIDEO_CHN_MAIN);
		/*check if cancel sub channel video stream*/
		cancel_video_stream(VIDEO_CHN_SUB);
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

/**
 * dana_vi_init - init video control
 * @vi_handle[IN]: opened vi_handle
 * return: 0 success; -1 failed
 */
static int dana_vi_init(void *vi_handle)
{
	int chn = VIDEO_CHN_MAIN;
	int ret = AK_FAILED;

	av_ctrl.vi_run_f = 1;

	for (chn=VIDEO_CHN_MAIN; chn<VIDEO_CHN_NUM; ++chn) {
		av_ctrl.video[chn].venc = dana_venc_init(chn);
		if (!av_ctrl.video[chn].venc) {
			ak_print_error_ex("video_encode_init failed, %s\n",
				ak_get_error_str(ak_get_error_no()));
			goto dana_vi_init_end;
		}
		dana_venc_set_bps(av_ctrl.video[chn].venc, chn);

		av_ctrl.video[chn].stream = NULL;
		av_ctrl.video[chn].run_flag = AK_TRUE;
		av_ctrl.video[chn].chn_id = chn;
		ak_thread_mutex_init(&av_ctrl.video[chn].lock, NULL);
		ak_thread_sem_init(&av_ctrl.video[chn].sem, 0);
		INIT_LIST_HEAD(&(av_ctrl.video[chn].send_head));

		/* creat 2 thread to get video data from venc */
		ret = ak_thread_create(&(av_ctrl.video[chn].tid), get_video_stream_thread,
			(void *)&(av_ctrl.video[chn]), ANYKA_THREAD_MIN_STACK_SIZE, -1);
		if(0 != ret){
			ak_print_normal_ex("create chn=%d dana_video_stream_thread failed, "
				"ret = %d!\n", chn, ret);
			goto dana_vi_init_end;
		}
	}

	ak_thread_sem_init(&av_ctrl.vi_sem, 0);
	av_ctrl.vi_handle = vi_handle;

	ret = ak_thread_create(&av_ctrl.vi_tid, do_vi_data,
		NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if (ret) {
		ak_print_normal_ex("create do_vi_data thread failed, ret=%d\n", ret);
	} else {
		ak_print_notice_ex("dana video start\n\n");
	}

dana_vi_init_end:
	if (ret && av_ctrl.vi_run_f) {
		av_ctrl.vi_handle = NULL;
		av_ctrl.vi_run_f = 0;
		ak_thread_sem_destroy(&av_ctrl.vi_sem);
	}

	return ret;
}

/*
 * is_dana_vi_running - check whether dana video is running 
 * return: true on running, false on not
 */
static int is_dana_vi_running(void)
{
	int ret = av_ctrl.video[VIDEO_CHN_MAIN].count;
	ret += av_ctrl.video[VIDEO_CHN_SUB].count;

	if ((ret && av_ctrl.vi_run_f) > 0)
		ret = AK_TRUE;
	else
		ret = AK_FALSE;

	return ret;
}

/**
 * dana_vi_exit - exit video control
 * return: void
 */
static void dana_vi_exit(void)
{
	if(av_ctrl.vi_run_f){
		av_ctrl.vi_run_f = 0;
		ak_thread_sem_post(&av_ctrl.vi_sem);
		ak_print_notice_ex("ready to join do_vi_data thread\n");
		ak_thread_join(av_ctrl.vi_tid);
		ak_print_notice_ex("do_vi_data thread join OK\n");

		ak_thread_sem_destroy(&av_ctrl.vi_sem);

		int chn = VIDEO_CHN_MAIN;
		for(chn=VIDEO_CHN_MAIN; chn<VIDEO_CHN_NUM; ++chn) {
			if (av_ctrl.video[chn].stream) {
				ak_venc_cancel_stream(av_ctrl.video[chn].stream);
				av_ctrl.video[chn].stream = NULL;
			}
			if (av_ctrl.video[chn].venc) {
				ak_venc_close(av_ctrl.video[chn].venc);
				av_ctrl.video[chn].venc = NULL;
			}
			ak_thread_mutex_destroy(&av_ctrl.video[chn].lock);
		}

		ak_print_normal_ex("leave...\n");
	}
}

/**
 * audio_stream_to_dana - get audio data from microphone and send it to danale
 * return: void
 */
static void audio_stream_to_dana(void)
{
	int i = 0;
	cb_t ai_cb[MAX_USER];
	unsigned int ai_count = 0;

	/*
	 * copy var value from global to local.
	 *  we change it outside easily and safely.
	 */
	ak_thread_mutex_lock(&av_ctrl.lock);
	for(i = 0; i < MAX_USER; i++){
		if(NULL != av_ctrl.ai.cb_info[i].cb_func){
			ai_cb[ai_count].cb_func = av_ctrl.ai.cb_info[i].cb_func;
			ai_cb[ai_count].pri_data = av_ctrl.ai.cb_info[i].pri_data;
			ai_count ++;
		}
	}
	ak_thread_mutex_unlock(&av_ctrl.lock);

	struct aenc_entry *entry = NULL;
	struct aenc_entry *ptr = NULL;
	struct list_head stream_head;

	INIT_LIST_HEAD(&stream_head);
	ak_print_normal_ex("send audio enter...\n");

	/* get audio stream list and send */
	while (av_ctrl.ai.run_flag) {
		if (0 == ak_aenc_get_stream(av_ctrl.ai.stream, &stream_head)) {
			list_for_each_entry_safe(entry, ptr, &stream_head, list) {
				if (entry) {
					if (0 == av_ctrl.ai.req_change) {
						/* wait video send */
						
						while (is_dana_vi_running() && (entry->stream.ts > av_ctrl.video_last_ts))
							ak_sleep_ms(10);

						for (int i=0; i < ai_count; i++) {
							/*use dana api send video data to dana cloud*/
							((net_send_func)ai_cb[i].cb_func)(ai_cb[i].pri_data,
								(void *)&entry->stream, av_ctrl.ai.encode_type);

						}
					}
					ak_aenc_release_stream(entry);
				}
			}
		}
		if (0 == av_ctrl.ai.run_flag)
			break;
		if (av_ctrl.ai.req_change) {
			av_ctrl.ai.req_change = 0;
			break;
		}
		ak_sleep_ms(10);
	}

	ak_aenc_cancel_stream(av_ctrl.ai.stream);
	av_ctrl.ai.stream = NULL;
	ak_print_normal_ex("send audio exit...\n");
}

/**
 * do_ai_data - thread, get audio data from microphone and send it to danale
 * return: null
 */
static void* do_ai_data(void *arg)
{
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	ak_thread_set_name("dana_ai");

	while(av_ctrl.ai.run_flag) {
		if(0 == av_ctrl.ai.count){
			ak_print_normal_ex("sleep...\n");
			sem_wait(&av_ctrl.ai.sem);
			ak_print_normal_ex("wakeup...\n");
			if(0 == av_ctrl.ai.count)
				continue;
		}
		if(0 == av_ctrl.ai.run_flag)
			break;


		/* bind audio input & encode */
		av_ctrl.ai.stream = ak_aenc_request_stream(av_ctrl.ai.dev,
			av_ctrl.ai.codec);
		if (NULL == av_ctrl.ai.stream) {
			ak_print_error_ex("ak_aenc_request_stream\n");
			break;
		}

		audio_stream_to_dana();
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

/**
 * dana_ai_init - init listen
 * @ai_handle[IN]: opened ai_handle
 * return: 0 success; -1 failed
 */
static int dana_ai_init(void *ai_handle)
{
	/*  open audio encode */
	struct audio_param aenc_param = {0};
	struct video_record_config *record_config = ak_config_get_record();  /*get record config return: record config struct pointer*/

	/*init audio param*/
	aenc_param.type = AK_AUDIO_TYPE_PCM_ALAW;
	aenc_param.sample_bits = 16;
	aenc_param.channel_num = AUDIO_CHANNEL_MONO;
	aenc_param.sample_rate = record_config->sample_rate;

	av_ctrl.ai.codec = ak_aenc_open(&aenc_param);
	if (!av_ctrl.ai.codec) {
		return AK_FAILED;
	}

	av_ctrl.ai.encode_type = G711A;
	ak_thread_sem_init(&av_ctrl.ai.sem, 0);
	av_ctrl.ai.dev = ai_handle;
	av_ctrl.ai.run_flag = 1;
	if (AK_SUCCESS == ak_aenc_set_frame_default_interval(av_ctrl.ai.codec, 100)) {
		ak_print_normal_ex("ai frame interval=%d\n", AUDIO_DEFAULT_INTERVAL);
	}

	/*start ai stream capture thread*/
	int ret = ak_thread_create(&av_ctrl.ai.tid, do_ai_data,
		NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if(ret){
		ak_print_normal_ex("create do_ai_data thread failed, ret=%d\n", ret);
		ak_aenc_close(av_ctrl.ai.codec);
		av_ctrl.ai.codec = NULL;
	} else {
		ak_print_notice_ex("dana ai start\n\n");
	}

	return ret;
}

/**
 * dana_ai_exit - exit listen
 * return: void
 */
void dana_ai_exit(void)
{
	if(av_ctrl.ai.run_flag){
		av_ctrl.ai.run_flag = 0;
		ak_thread_sem_post(&av_ctrl.ai.sem);
		ak_print_notice_ex("ready to join do_ai_data thread\n");
		ak_thread_join(av_ctrl.ai.tid);
		ak_print_notice_ex("do_ai_data thread join OK\n");

		ak_thread_sem_destroy(&av_ctrl.ai.sem);
		if (av_ctrl.ai.stream) {
			ak_aenc_cancel_stream(av_ctrl.ai.stream);
			av_ctrl.ai.stream = NULL;
		}

		if (av_ctrl.ai.codec) {
			ak_aenc_close(av_ctrl.ai.codec);
			av_ctrl.ai.codec = NULL;
		}
	}
	ak_print_info_ex("leave...\n");
}

/**
 * send_decode_stream -  send talk data  to speaker
 * return: void
 */
static void send_decode_stream(void)
{
	unsigned char data[DANA_AUDIO_SEND_LEN] = {0};

#if DANA_AUDIO_TIME_DEBUG
	static struct ak_timeval start_time;
	struct ak_timeval end_time;

	if (0 == start_time.sec) {
		ak_get_ostime(&start_time);
	}
#endif

	/*read ring buffer data*/
	int send_len = ak_rb_read(av_ctrl.ao_rb, data, DANA_AUDIO_SEND_LEN);
	if (send_len > 0) {
		if (ak_adec_send_stream(av_ctrl.ao.codec,
			data, send_len, 0) < 0) {
			ak_print_error_ex("data send stream failed, send_len=%d\n", send_len);
		}

#if DANA_AUDIO_TIME_DEBUG
		ak_get_ostime(&end_time);
		ak_print_info("----------- send time=%ld, send_len=%d -----------\n",
			ak_diff_ms_time(&end_time, &start_time), send_len);
		ak_get_ostime(&start_time);
#endif
	}
}

/**
 * audio_stream_from_dana -  get talk data from danale and send to speaker
 * return: void
 */
static void audio_stream_from_dana(void)
{
	unsigned char wait_flag = AK_TRUE;
	struct audio_stream *ptalk_data = NULL;
	get_data_func audio_play_cb = (get_data_func)av_ctrl.ao.cb_info[0].cb_func;

#if DANA_AUDIO_TIME_DEBUG
	struct ak_timeval start_time;
	struct ak_timeval end_time;

	ak_get_ostime(&start_time);
	ak_get_ostime(&end_time);
#endif

#if DANA_RECV_DATA_DEBUG
	dana_ai_fp = fopen("/tmp/dana_recv_data.g711a", "w");
	if (!dana_ai_fp) {
		ak_print_error_ex("open dana_recv_data.g711a failed\n");
	}
#endif

	struct ak_date date = {0};

	ak_get_localdate(&date);
	ak_print_normal_ex("receive audio enter...\n");
	ak_print_date(&date);

	while(av_ctrl.ao.run_flag)	{
		if (audio_play_cb) {
			ptalk_data = audio_play_cb(av_ctrl.ao.cb_info[0].pri_data);
		}

		if (ptalk_data) {
			if(ptalk_data->len > 0) {
				if (wait_flag) {
					/* wait and buffered enough audio data to decode */
					if ((ak_rb_get_data_len(av_ctrl.ao_rb) + ptalk_data->len)
						>= DANA_AUDIO_RB_LEN) {
						wait_flag = AK_FALSE;
						send_decode_stream();
					}
				} else {
					send_decode_stream();
				}

#if DANA_RECV_DATA_DEBUG
				if (dana_ai_fp)
					fwrite(ptalk_data->data, 1, ptalk_data->len, dana_ai_fp);
#endif
				ak_rb_write(av_ctrl.ao_rb, ptalk_data->data, ptalk_data->len);

#if DANA_AUDIO_TIME_DEBUG
 				ak_get_ostime(&end_time);
				ak_print_info("\t--- len=%d, receive time=%ld ---\n",
					ptalk_data->len, ak_diff_ms_time(&end_time, &start_time));
				ak_get_ostime(&start_time);
#endif
			}

			free(ptalk_data->data);
			free(ptalk_data);
			ptalk_data = NULL;
		}else{
			ak_sleep_ms(2);
		}

		if(av_ctrl.ao.req_change){
			send_decode_stream();
			av_ctrl.ao.req_change = 0;
			break;
		}
	}

	ak_get_localdate(&date);
	ak_print_normal_ex("receive audio leave...\n");
	ak_print_normal_ex("ao.run_flag=%d, ao.req_change=%d\n",
		av_ctrl.ao.run_flag, av_ctrl.ao.req_change);
	ak_print_date(&date);

#if DANA_RECV_DATA_DEBUG
	if (dana_ai_fp) {
		fclose(dana_ai_fp);
		dana_ai_fp = NULL;
	}
#endif
}

/**
 * do_ao_data - thread, get talk data from danale and send to speaker
 * return: null
 */
static void* do_ao_data(void *arg)
{
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	ak_thread_set_name("dana_ao");

	while(av_ctrl.ao.run_flag) {
		if(0 == av_ctrl.ao.count){
			ak_print_normal_ex("sleep...\n");
			ak_thread_sem_wait(&av_ctrl.ao.sem);
			ak_print_normal_ex("receive audio wakeup...\n");
			if(0 == av_ctrl.ao.count) {
			    continue;
			}
		}
		if (!av_ctrl.ao.run_flag)
		   	break;

		/* bind audio output & decode */
		av_ctrl.ao.stream = ak_adec_request_stream(av_ctrl.ao.dev,
				av_ctrl.ao.codec);
		if (av_ctrl.ao.stream) {
			ak_ai_set_aec(av_ctrl.ai.dev, AK_TRUE);
			/* get stream */
			audio_stream_from_dana();

			/* speak end,must use ak_adec_notice_stream_end */
			ak_adec_notice_stream_end(av_ctrl.ao.codec);
			ak_adec_cancel_stream(av_ctrl.ao.stream);
			av_ctrl.ao.stream = NULL;
			ak_ai_set_aec(av_ctrl.ai.dev, AK_FALSE);
		}
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

/**
 * dana_ao_init - init talk
 * @ao_handle[IN]: opened ao_handle
 * return: 0 success; -1 failed
 */
static int dana_ao_init(void *ao_handle)
{
	int ret = AK_FAILED;
	struct audio_param adec_param = {0};

	/* open audio encode */
	adec_param.type = AK_AUDIO_TYPE_PCM_ALAW;
	adec_param.sample_bits = 16;
	adec_param.channel_num = AUDIO_CHANNEL_MONO;
	adec_param.sample_rate = 8000;

	av_ctrl.ao.codec = ak_adec_open(&adec_param);
	if(NULL == av_ctrl.ao.codec) {
		return AK_FAILED;
	}

	av_ctrl.ao_rb = ak_rb_init(DANA_AUDIO_RB_LEN);
	if (!av_ctrl.ao_rb) {
		ak_adec_close(av_ctrl.ao.codec);
		av_ctrl.ao.codec = NULL;
		return AK_FAILED;
	}

	av_ctrl.ao.run_flag = 1;
	av_ctrl.ao.dev = ao_handle;
	ak_thread_sem_init(&av_ctrl.ao.sem, 0);

	ret = ak_thread_create(&av_ctrl.ao.tid, do_ao_data,
		NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if(ret){
		ak_print_normal_ex("create do_ao_data thread failed, ret=%d\n", ret);
	} else {
		ak_print_notice_ex("dana ao start\n\n");
	}

	if(ret) {
		if (av_ctrl.ao.codec) {
			ak_adec_close(av_ctrl.ao.codec);
			av_ctrl.ao.codec = NULL;
		}
	}

	return ret;
}

/**
 * dana_ao_exit - exit talk
 * return: void
 */
static void dana_ao_exit(void)
{
	if(av_ctrl.ao.run_flag) {
		av_ctrl.ao.run_flag = 0;
		ak_thread_sem_post(&av_ctrl.ao.sem);
		ak_print_notice_ex("ready to join do_ao_data thread\n");
		ak_thread_join(av_ctrl.ao.tid);
		ak_print_notice_ex("do_ao_data thread join OK\n");

		ak_thread_sem_destroy(&av_ctrl.ao.sem);
		if (av_ctrl.ao.stream) {
			ak_adec_cancel_stream(av_ctrl.ao.stream);
			av_ctrl.ao.stream = NULL;
		}

		if (av_ctrl.ao.codec) {
			ak_adec_close(av_ctrl.ao.codec);
			av_ctrl.ao.codec = NULL;
		}
		if (av_ctrl.ao_rb) {
			ak_rb_release(av_ctrl.ao_rb);
			av_ctrl.ao_rb = NULL;
		}

		ak_print_info_ex("leave...\n");
	}
}

/**
 * dana_video_preview_quality -  when video preview ,change video quality
 * @video_quality[IN]:  1-100
 * @p_req_video[OUT]: video No, 0 or 1
 * return: frame
 */
static int dana_video_preview_quality(int video_quality,int *p_req_video)
{
	int frames = 0;
	int video_bps = 0;
	struct video_config *video_config = ak_config_get_sys_video();
	struct video_record_config *record = ak_config_get_record();

	ak_print_normal_ex("%d\n", video_quality);
	if(video_quality < 0)
		video_quality = 0;
	/*
	 * video quality is 3 type: over high definition, high definition, normal
	 *  it use  video channel and bps differently.
	 */
	if(video_quality > HIGH_BPS) {
		*p_req_video = VIDEO_CHN_MAIN;
		frames = record->save_file_fps;
		video_bps = record->save_file_kbps;
	}
	else if(video_quality > NORMAL_BPS) {
		*p_req_video = VIDEO_CHN_MAIN;
		frames =video_config->main_fps;
		video_bps = video_config->main_min_kbps + (video_config->main_max_kbps- video_config->main_min_kbps) * (video_quality - NORMAL_BPS) / (HIGH_BPS - NORMAL_BPS);
	}
	else {
		*p_req_video = VIDEO_CHN_SUB;
		frames = video_config->sub_fps;
		video_bps = video_config->sub_min_kbps + (video_config->sub_max_kbps - video_config->sub_min_kbps) * video_quality  / NORMAL_BPS;
	}
	ak_print_normal_ex("video chn req:%d !\n",*p_req_video);
	/* video bps change , set it*/
	if (0 != ak_venc_set_rc(av_ctrl.video[*p_req_video].venc, video_bps)) //reset encode bitpersecond
		ak_print_error_ex("ak_venc_set_rc fail!\n");

	return frames;
}

/**
 * dana_av_switch_gop - swtich video gop
 * return: void
 */
void dana_av_switch_gop(void)
{
	/* incase of not init */
	if (av_ctrl.vi_run_f != 1)
		return;

	int fps, goplen, i;
	struct video_config *video = ak_config_get_sys_video();

	/* loop to set gop */
	for (i = VIDEO_CHN_MAIN; i < VIDEO_CHN_NUM; i++) {
		if (av_ctrl.video[i].venc) {
			fps = ak_venc_get_fps(av_ctrl.video[i].venc);
			if (fps) {
				if (i == VIDEO_CHN_MAIN)
					goplen = video->main_gop_len;
				else
					goplen = video->sub_gop_len;
				goplen *= fps;  //for example fps=25 video->gop_len=2  goplen= 50
				ak_venc_set_gop_len(av_ctrl.video[i].venc, goplen); //set encode gop len
				ak_print_normal_ex("chn[%d] goplen: %d, fps: %d\n", i, goplen, fps);
			}
		}
	}
}

/**
 * dana_av_start_video - start  video  send to danale
 * @id[IN]: 		request function id
 * @func[IN]: 		video  send  function
 * @pri_data[IN]: 	private data for send   function
 * @quality[IN]: 	video quality [1-100]
 * return:   successful return frames;  fail return 0
 */
int dana_av_start_video(enum dana_callback_id id,
		void *func, void *pri_data, int quality)
{
	int frames = 0;
	int i = 0;
	int new_req_video = VIDEO_CHN_MAIN;

	if (0 == av_ctrl.vi_run_f) {
		ak_print_error_ex("dana video not init!\n");
		return 0;
	}
	/* if DANA_CLOUD_VIDEO == id, new_req_video = VIDEO_CHN_MAIN */
	if (DANA_PRE_VIDEO == id) {
		/* when video preview ,change video quality * @video_quality[IN]:  1-100 */
		frames = dana_video_preview_quality(quality, &new_req_video);

		ak_thread_mutex_lock(&av_ctrl.lock);
		/*change bps, maybe change channel*/
		if (NULL == func) {
			for (i = 0; i < MAX_USER; i++) {
				if (pri_data == av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].pri_data) {
					if (new_req_video == VIDEO_CHN_MAIN) {
						/*only change bps*/
						new_req_video = -1;
					} else {
						/*change channel*/
						func = av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].cb_func;
						av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].cb_func = NULL;
						av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].pri_data = NULL;
						av_ctrl.video[VIDEO_CHN_MAIN].count--;
					}
					break;
				} else if (pri_data == av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].pri_data) {
					if (new_req_video == VIDEO_CHN_SUB) {
						/*only change bps*/
						new_req_video = -1;
					} else {
						/*change channel*/
						func = av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].cb_func;
						av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].cb_func = NULL;
						av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].pri_data = NULL;
						av_ctrl.video[VIDEO_CHN_SUB].count--;
					}
					break;
				}
			}
		}
		ak_thread_mutex_unlock(&av_ctrl.lock);
	}
	ak_print_normal_ex("video chn req:%d !\n", new_req_video);
	if (VIDEO_CHN_MAIN == new_req_video) {
		/* main video request is over max. */
		if (av_ctrl.video[VIDEO_CHN_MAIN].count >= MAX_USER) {
			ak_print_error_ex("video chn:%d > max user!\n", new_req_video);
			return 0;
		}
		ak_thread_mutex_lock(&av_ctrl.lock);
		/*
		get a no use cb_info number
		if find cb_info is NULL
		then use this array number
		*/
		for (i = 0; i < MAX_USER; i++)
			if(NULL == av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].cb_func)
				break;
		if(i < MAX_USER){
			av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].cb_func = func;
			av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].pri_data= pri_data;
			av_ctrl.video[VIDEO_CHN_MAIN].count++;
		}else{
			/* main video request is over max. */
			ak_print_error_ex("video chn:%d > max user!\n",new_req_video);
		}
		ak_thread_mutex_unlock(&av_ctrl.lock);
	} else if (VIDEO_CHN_SUB == new_req_video) {
		/* sub video request is over max. */
		if(av_ctrl.video[VIDEO_CHN_SUB].count >= MAX_USER)
			return -1;
		ak_thread_mutex_lock(&av_ctrl.lock);
		/*
		get a no use cb_info number
		if find cb_info is NULL
		then use this array number
		*/
		for(i = 0; i < MAX_USER; i++)
			if(NULL == av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].cb_func)
				break;
		if(i < MAX_USER){
			av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].cb_func = func;
			av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].pri_data= pri_data;
			av_ctrl.video[VIDEO_CHN_SUB].count++;
		}else{
			/*
			 * sub video request is over max.
			 * system is overload.
			 */
			ak_print_error_ex("video chn:%d > max user!\n",new_req_video);
		}
		ak_thread_mutex_unlock(&av_ctrl.lock);
	}
	ak_print_normal_ex("video chn, main:%d sub:%d!\n",
			av_ctrl.video[VIDEO_CHN_MAIN].count,
			av_ctrl.video[VIDEO_CHN_SUB].count);
	/* set global var to update video send*/
	if (VIDEO_CHN_SUB == new_req_video || VIDEO_CHN_MAIN == new_req_video){
		av_ctrl.video_req_change = 1;
		ak_thread_sem_post(&av_ctrl.vi_sem);
	}

	return frames;
}

/**
 * dana_av_start_audio - start  audio  send to danale or receive from danale
 * @id[IN]: 		request function id
 * @func[IN]: 		audio  send  or receive function
 * @pri_data[IN]: 	private data for send  or receive function
 * return:   successful return 0 ;  fail return -1
 */
int dana_av_start_audio(enum dana_callback_id id,void *func,void *pri_data)
{
	int i = 0;

	if(( 0 == av_ctrl.ai.run_flag ) || ( 0 == av_ctrl.ao.run_flag )){
		ak_print_error_ex("dana audio not init!\n");
		ak_print_notice_ex("ai :%d ao:%d \n",av_ctrl.ai.run_flag,av_ctrl.ao.run_flag);
		return -1;
	}
	switch(id) {
	case DANA_PRE_AUDIO:
	case DANA_CLOUD_AUDIO:
		if(av_ctrl.ai.count >= MAX_USER){
			ak_print_error_ex("ai > max user!\n");
			return -1;
		}
		ak_thread_mutex_lock(&av_ctrl.lock);

		for(i = 0; i < MAX_USER; i++)
			if(NULL == av_ctrl.ai.cb_info[i].cb_func)
				break;
		if(i < MAX_USER){
			av_ctrl.ai.cb_info[i].cb_func = func;
			av_ctrl.ai.cb_info[i].pri_data= pri_data;
			av_ctrl.ai.count++;
		}else{
			/*
			 * request is over max.
			 * system is overload.
			 */
			ak_print_error_ex("ai > max user!\n");
		}
		ak_thread_mutex_unlock(&av_ctrl.lock);
		if(i < MAX_USER){
			/*now only one , ai wait at sem */
			if(1 == av_ctrl.ai.count)
				ak_thread_sem_post(&av_ctrl.ai.sem);
			/*now ai is already working, we only need to update do_ai_data  */
			else
				av_ctrl.ai.req_change = 1;
		}
		break;
	case DANA_PLAY_AUDIO:
		av_ctrl.ao.cb_info[0].cb_func = func;
		av_ctrl.ao.cb_info[0].pri_data= pri_data;
		av_ctrl.ao.count++;

		/*
		 * ao no more than one.
		 * if ao > 1, ao is already working, we only need to update do_ao_data
		 * if ao == 1, ao wait at sem
		 */
		if(av_ctrl.ao.count > 1){
			av_ctrl.ao.count = 1;
			av_ctrl.ao.req_change = 1;
		} else {
			ak_thread_sem_post(&av_ctrl.ao.sem);
		}

		ak_print_notice_ex("ao.count=%d, ao.req_change=%d\n",
			av_ctrl.ao.count, av_ctrl.ao.req_change);
		break;
	default:
		break;
	}

	return 0;
}

/**
 * dana_av_stop - stop  video or audio which  dana_av_start_audio( or video) request
 * @id[IN]: 		request function id
 * @pri_data[IN]: 	private data for send or receive  function
 * return:   successful return 0 ;  fail return -1
 */
int dana_av_stop(enum dana_callback_id id , void *pri_data)
{
	int video_req_change = 0;
	int ai_req_change = 0;
	int i = 0;

	switch (id) {
	case DANA_PRE_VIDEO:
	case DANA_CLOUD_VIDEO:
		/* stop preview video or cloud video */
		ak_thread_mutex_lock(&av_ctrl.lock);
		for(i = 0; i < MAX_USER; i++){
			if(pri_data == av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].pri_data){
				av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].cb_func = NULL;
				av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].pri_data= NULL;
				av_ctrl.video[VIDEO_CHN_MAIN].count--;
				video_req_change = 1;
				break;
			}
			if(pri_data == av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].pri_data){
				av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].cb_func = NULL;
				av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].pri_data= NULL;
				av_ctrl.video[VIDEO_CHN_SUB].count--;
				video_req_change = 1;
				break;
			}
		}
		ak_thread_mutex_unlock(&av_ctrl.lock);
		break;
	case DANA_PRE_AUDIO:
	case DANA_CLOUD_AUDIO:
		/* stop preview audio or cloud audio */
		ak_thread_mutex_lock(&av_ctrl.lock);
		for(i = 0; i < MAX_USER; i++)
			if(pri_data == av_ctrl.ai.cb_info[i].pri_data){
				av_ctrl.ai.cb_info[i].cb_func = NULL;
				av_ctrl.ai.cb_info[i].pri_data= NULL;
				av_ctrl.ai.count--;
				ai_req_change = 1;
				break;
			}
		ak_thread_mutex_unlock(&av_ctrl.lock);
		break;
	case DANA_PLAY_AUDIO:
		/* only stop talk */
		av_ctrl.ao.cb_info[0].cb_func = NULL;
		av_ctrl.ao.cb_info[0].pri_data= NULL;
		if (av_ctrl.ao.count) {
			if (av_ctrl.ao.run_flag) {
			    av_ctrl.ao.req_change = 1;
			    ak_print_notice_ex("ao.count=%d, ao.req_change set to 1\n",
			    av_ctrl.ao.count);
			}
			av_ctrl.ao.count = 0;
		}
		break;
	default:
		if (pri_data) {
			/*
			 * id value is CALLBACK_MAX , means danale connection failed.
			 * we shold donot send data to danale by this connection
			 */
			ak_thread_mutex_lock(&av_ctrl.lock);
			for(i = 0; i < MAX_USER; i++){
				if(pri_data == av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].pri_data){
					av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].cb_func = NULL;
					av_ctrl.video[VIDEO_CHN_MAIN].cb_info[i].pri_data= NULL;
					av_ctrl.video[VIDEO_CHN_MAIN].count--;
					video_req_change = 1;

				}
				if(pri_data == av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].pri_data){
					av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].cb_func = NULL;
					av_ctrl.video[VIDEO_CHN_SUB].cb_info[i].pri_data= NULL;
					av_ctrl.video[VIDEO_CHN_SUB].count--;
					video_req_change = 1;

				}
				if(pri_data == av_ctrl.ai.cb_info[i].pri_data){
					av_ctrl.ai.cb_info[i].cb_func = NULL;
					av_ctrl.ai.cb_info[i].pri_data= NULL;
					av_ctrl.ai.count--;
					ai_req_change = 1;
				}
			}
			if (pri_data == av_ctrl.ao.cb_info[0].pri_data) {
				av_ctrl.ao.cb_info[0].cb_func = NULL;
				av_ctrl.ao.cb_info[0].pri_data= NULL;
				av_ctrl.ao.count--;
				/* set global var to update audio recieve */
				av_ctrl.ao.req_change = 1;

			}
			ak_thread_mutex_unlock(&av_ctrl.lock);
		}
		break;
	}

	/* set global var to update audio send*/
	if(1 == ai_req_change)
		av_ctrl.ai.req_change = 1;
	/* set global var to update video send*/
	if(1 == video_req_change)
		av_ctrl.video_req_change = 1;

	ak_print_normal_ex("video chn, main:%d, sub:%d\n",
		av_ctrl.video[VIDEO_CHN_MAIN].count,
		av_ctrl.video[VIDEO_CHN_SUB].count);
	ak_print_normal_ex("ai_count:%d, ai_req:%d, vi_req:%d, ao_req:%d\n",
		av_ctrl.ai.count, av_ctrl.ai.req_change,
		av_ctrl.video_req_change, av_ctrl.ao.req_change);

	return 0;
}

/**
 * ak_dana_init - init dana audio and video
 * @vi_handle[IN]: opened vi_handle
 * @ai_handle[IN]: opened ai_handle
 * @ao_handle[IN]: opened ao_handle
 * return: 0 success; -1 failed
 */
int dana_av_init(void *vi_handle,void *ai_handle,void *ao_handle)
{
	ak_print_notice("\t----- dana video&audio enter -----\n");
	memset(&av_ctrl,0,sizeof(struct ak_dana_av_t));
	ak_thread_mutex_init(&av_ctrl.lock, NULL);
	if (dana_vi_init(vi_handle)) {
		return AK_FAILED;
	}

	if (dana_ai_init(ai_handle)) {
		return AK_FAILED;
	}

	if (dana_ao_init(ao_handle)) {
		return AK_FAILED;
	}
	ak_print_normal("\t----- start dana video&audio OK -----\n\n");

	return AK_SUCCESS;
}

/**
 * dana_av_exit -  free dana audio and video resource and exit
 * return: void
 */
void dana_av_exit(void)
{
	dana_vi_exit();
	dana_ai_exit();
	ak_thread_mutex_destroy(&av_ctrl.lock);
	dana_ao_exit();
}
