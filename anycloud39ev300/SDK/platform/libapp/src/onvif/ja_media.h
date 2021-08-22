#ifndef _NI_SERVICE_H_
#define _NI_SERVICE_H_

#include "list.h"
#include "ak_global.h"
#include "ak_thread.h"
#include "ak_aenc.h"
#include "ak_vi.h"

#define IMG_EFFECT_DEF_VAL	50
#define OSD_TITLE_LEN_MAX	45
#define FPS_MIN			5
#define FPS_MAX			25
#define FPS_DEF			25
#define MAIN_BPS_MIN	256
#define MAIN_BPS_MAX	3072
#define MAIN_BPS_DEF	2048
#define SUB_BPS_MIN		128
#define SUB_BPS_MAX		1024
#define SUB_BPS_DEF		768
#define MAIN_VIDEO_STREAM_SIZE_MAX	(1536*1024)
#define SUB_VIDEO_STREAM_SIZE_MAX	(500*1024)
#define AUDIO_STREAM_SIZE_MAX		(100*1024)

enum onvif_media_type {
	MEDIA_TYPE_VIDEO_MAIN = 0x00,
	MEDIA_TYPE_VIDEO_SUB,
	MEDIA_TYPE_AUDIO,
	MEDIA_TYPE_NUM,
};

enum onvif_ir_mode {
	IR_MODE_AUTO = 0x00,
	IR_MODE_DAYLIGHT,
	IR_MODE_NIGHT,
	IR_MODE_NUM,
};

enum onvif_send_stream_id {
	ONVIF_STREAM_VIDEO = 0x00,
	ONVIF_STREAM_AUDIO,
};

typedef struct {
	unsigned int mainbps;
	unsigned int subbps;
	unsigned int main_videomode;		//0:CBR; 1:VBR
	unsigned int sub_videomode;		//0:CBR; 1:VBR
	unsigned int profilemode;		//0:base line; 1:main profile; 2:high profile
	unsigned int mainfps;
	unsigned int subfps;
	unsigned int main_enctype;	//264 or 265
	unsigned int sub_enctype;	//264 or 265
}T_VIDEO_SET_VALUE;

extern T_VIDEO_SET_VALUE g_video_set;

#define CONNECT_NUM_MAX	4

typedef struct {
	ak_mutex_t     video_lock;
	ak_mutex_t     audio_lock;
	ak_mutex_t     streamlock;
    struct list_head video_queue;
	struct list_head audio_queue;
	int video_run_flag;
	int audio_run_flag;
	bool bValid;
	bool bMainStream;
	bool biFrameflag;
	unsigned int video_stream_size;
	unsigned int audio_stream_size;
}JA_CONNECT;

typedef struct {
	char type;			//video or audio, 0 video, 1 audio
	char run_flg;
	char osd_en;
	char chn_idx;		//channel index: 1, main 0, sub
	void *input_handle;	//video input or audio input
	void *enc_handle;	//encode handle
	void *stream_handle;	//stream handle
	ak_pthread_t str_th_id;	//get stream thread id
	ak_mutex_t  str_lock;
	ak_mutex_t  lock;
}media_ctrl;

typedef struct {
	ak_mutex_t     lock;
	ak_mutex_t     usrFilelock;
	unsigned int	conn_cnt;
	unsigned int	main_cnt;
	unsigned int	sub_cnt;
    JA_CONNECT	conn[CONNECT_NUM_MAX];
	media_ctrl  media[MEDIA_TYPE_NUM];
	int			update_flag;
	int 		video_stat_conn[VIDEO_CHN_NUM];
	int			ai_enable;
}JA_CTRL;

typedef struct {
	int idx; 	//main/sub channel index, 0 main, 1 sub
	struct video_stream vstr;	//stream data
	struct list_head list;	//stream list
}ja_video_stream;

extern JA_CTRL *pja_ctrl;

/*
 * ja_media_sn_save - sn save
 * sn[IN]:sn str
 * return: 0  fail, 1  success
 */
int ja_media_sn_save(char* sn);

/*
 * ja_media_sn_read - sn read
 * sn[OUT]:sn str
 * return: 0  fail, 1  success
 */
int ja_media_sn_read(char* sn);

/*
 * ja_media_get_styleId - get style id
 * return: style id
 */
int ja_media_get_styleId(void);

/*
 * ja_media_set_styleId - set style id
 * styleId[IN]:style id
 * return: void
 */
void ja_media_set_styleId(int styleId);

/*
 * ja_media_venc_request_stream - start video stream, strart get stream thread
 * ja_ctrl[IN]: store video stream handle
 * idx[IN]: specific current channel
 * return: void
 */
void ja_media_venc_request_stream(JA_CTRL *ja_ctrl, int idx);

/*
 * ja_media_venc_init - init video encode channel
 * idx[IN]: to indicate current channel
 * return: on success, video encode handle; on failed NULL is return.
 */
void *ja_media_venc_init(int idx);

/*
 * ja_media_init_video - init n1 video resource
 * vi[IN]: video input handle
 * ja_ctrl[IN]: store video encode handle and other control resource
 * return: void
 */
void ja_media_init_video(void *vi, JA_CTRL *ja_ctrl);

/*
 * ja_media_release_video_stream - release video stream node
 * idx[IN]: connect index, to indicate specifically connection
 * v[IN]: pointer to video stream node which will be release
 * free_data[IN]: free data or not
 * return: void
 */
void ja_media_release_video_stream(int idx, ja_video_stream *v, int free_data);
/*
 * ja_media_venc_cancel_stream - close video encode
 * idx[IN]: main or sub channel index, 0 is main
 * return: void
 */
void ja_media_venc_cancel_stream(int idx);

/*
 * ja_media_venc_close - close video encode
 * idx[IN]: main or sub channel index, 0 is main
 * return: void
 */
void ja_media_venc_close(int idx);

/*
 * ja_media_close_video - close n1 video resource
 * ja_ctrl[IN]: store video encode handle and other control resource
 * return: void
 */
void ja_media_close_video(JA_CTRL *ja_ctrl);

/*
 * ja_media_aenc_request_stream - start audio stream, strart get stream thread
 * ja_ctrl[IN]: store audio stream handle
 * return: void
 */
void ja_media_aenc_request_stream(JA_CTRL *ja_ctrl);

/*
 * ja_media_init_audio - init n1 audio resource
 * ai[IN]: audio input handle
 * ja_ctrl[IN]: store audio encode handle and other control resource
 * return: void
 */
void ja_media_init_audio(void *ai, JA_CTRL *ja_ctrl);

/*
 * ja_media_free_audio_stream - release video stream node resource
 * v[IN]: pointer to audio stream node which will be release
 * free_data[IN]: free data or not
 * return: void
 */
void ja_media_free_audio_stream(struct aenc_entry *a, int free_data);

/*
 * ja_media_release_audio_stream - release audio stream node
 * idx[IN]: connect index, to indicate specifically connection
 * a[IN]: pointer to audio stream node which will be release
 * free_data[IN]: free data or not
 * return: void
 */
void ja_media_release_audio_stream(int idx, struct aenc_entry *a, int free_data);

/*
 * ja_media_aenc_cancel_stream - close audio encode
 * void
 * return: void
 */
void ja_media_aenc_cancel_stream(void);

/*
 * ja_media_close_audio - close audio encode
 * void
 * return: void
 */
void ja_media_close_audio(JA_CTRL *ja_ctrl);

/*
 * ja_media_destroy_video_stream_queue - destroy video stream queue
 * idx[IN]: connect index, to indicate specifically connection
 * stream_queue[IN]: stream list head
 * return: void
 */
void ja_media_destroy_video_stream_queue(int idx, struct list_head *stream_queue);

/*
 * ja_media_destroy_audio_stream_queue - destroy audio stream queue
 * idx[IN]: connect index, to indicate specifically connection
 * stream_queue[IN]: stream list head
 * return: void
 */
void ja_media_destroy_audio_stream_queue(int idx, struct list_head *stream_queue);

/*
 * ja_media_change_resolution - change resolution
 * width[IN]: width
 * height[IN]: height
 * chn[IN]: main or sub
 * return: void
 */
void ja_media_change_resolution(int width, int height, int chn);

/*
 * ja_media_set_venc_parm - set venc parm ,such as bps, fps, mode(cbr/vbr)
 * chn[IN]: main or sub
 * bps[IN]: bps
 * fps[IN]: fps
 * mode[IN]: cbr or vbr
 * return: void
 */
void ja_media_set_venc_parm(int chn, int bps, int fps, int mode);

/*
 * ja_media_set_venc_parm - set venc type h.264/h.265
 * chn[IN]: main or sub
 * type[IN]: h.264/h.265
 * return: void
 */
void ja_media_set_venc_type(int chn, int type);

/*
 * ja_media_is_queue_empty - check queue is empty or not
 * idx[IN]: connection id
 * bVideo[IN]: 1 video, 0 audio
 * return: 1 empty, 0 not empty
 */
int ja_media_is_queue_empty(int idx, int bVideo);

/*
 * ja_media_init_ir_switch - init ir switch
 * void
 * return: 0 success, -1 fail
 */
int ja_media_init_ir_switch(void);

/*
 * ja_media_set_video_day_night - set day night mode
 * vi_handle[IN]: opened vi handle
 * ir_val[IN]: 1 day, 0 night
 * return: 0 success, -1 fail
 */
int ja_media_set_video_day_night(void *vi_handle, int ir_val);

/*
 * ja_media_set_switch_ir - set ir auto switch enable
 * enable[IN]: 1 enable auto switch
 * return: void
 */
void ja_media_set_switch_ir(int enable);

/*
 * ja_media_destroy_ir_switch - destroy ir switch
 * return: void
 */
void ja_media_destroy_ir_switch(void);

/*
 * ja_media_check_video_conn_stat - check video conn stat, for display osd
 * pja_ctrl[IN]: pja_ctrl handle
 * ch[IN]: ch id, 0 main, 1 sub
 * conn[IN]: conn id
 * return: void
 */
void ja_media_check_video_conn_stat(JA_CTRL *pja_ctrl, int ch, int conn);

/*
 * ja_media_irled_init - init irled
 * void
 * return: 0 success, -1 fail
 */
int ja_media_irled_init(void);

/*
 * ja_media_init_img_effect - init img effect
 * vi[IN]: opened vi handle
 * return: void
 */
void ja_media_init_img_effect(void *vi);

/*
 * ja_media_start_check_fps - create fps switch thread
 * return: 0 success; -1 failed
 */
int ja_media_start_check_fps(void);

/*
 * ja_media_stop_check_fps - join fps switch thread
 * return: void
 */
void ja_media_stop_check_fps(void);

#endif
