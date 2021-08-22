#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "list.h"
#include "internal_error.h"
#include "akuio.h"
#include "video_stream_lib.h"

#include "ak_global.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_error.h"
#include "ak_vi.h"
#include "ak_venc.h"

#ifdef AK_RTOS
#include "kernel.h"
#endif

#define VENC_GET_STREAM_DEBUG	(0)

#define VENC_TYPE_MAX_USER		(sizeof(int) * 8)
#define ENC_OUT_MAX_SZ 			(320 * 1024)
#ifdef AK_RTOS
#define ONE_STREAM_MAX 			(100)
#else
#define ONE_STREAM_MAX 			(50)
#endif

#define CALC_VIDEO_CAPTURE		(1)
#define CALC_VIDEO_ENCODE		(1)

#define VENC_CALC_CAP_TIME		(0)
#define VENC_CALC_ENC_TIME		(1)

#define VENC_ORIGIN_TS			(0)
#define VENC_CALC_TS			(0)
#define VENC_SMOOTH_TS			(1)
#define SHOW_ORIGIN_TS			(0)

/* use for capture */
struct frame_node {
	int bit_map;	//which group will use it
	struct video_input_frame *vframe;	//video frame pointer
	struct list_head list;				//frames list
};

struct video_streams_t {
	int ref;					//use count
	int bit_map;				//user map
	struct video_stream stream;	//data
	struct list_head 	list; 	//stream list
};

struct thread_arg {
	int cap_run;	//use for capture thread
	int enc_run;	//use for encode thread
	int sensor_pre_fps;
	void *vi_handle;
	struct list_head head_frame; //store capture frame, get by encode thread

	ak_pthread_t cap_tid;	 //thread need join
	ak_pthread_t enc_tid;	//thread need join
	ak_sem_t cap_sem;	//capture semaphore
	ak_sem_t enc_sem;	//encode semaphore
	ak_mutex_t lock;	//list operate lock

	struct list_head list; //hang to video_ctrl
};

/* each encode group has one this structure to discribe its info */
struct encode_group {
	int user_count;			//how many user under this encode group
	int req_ref;			//request reference count
	int user_map;			//request user bit map
	int capture_frames;		//capture frame, syn with system's camera device
	int encode_frames;		//encode frames
	unsigned long long frame_index;//use to control frames per second
	unsigned long long smooth_index;
	int ip_frame;			//use to control encode I or P frame
	int reset_iframe;		//1 need reset, 0 needless
	int is_stream_mode;		//1 stream mode, 0 sigle mode
	unsigned long long pre_ts;//timestamp(ms)

	void *lib_handle;	//encoder lib operate handle
	void *output;		//encode output buf
	void *encbuf;		//encode buf address

	enum encode_group_type 	grp_type;	//group flag
	struct encode_param 	record; 	//record encode param
	struct list_head 		stream_list;	//store stream
	struct list_head 		enc_list;		//register encode handle list

	T_VIDEOLIB_ENC_PARA enc_grp_param;	//encode gourp param
	T_VIDEOLIB_ENC_RC video_rc;		//encode rate control struct
	T_eVIDEO_DRV_TYPE drv_enc_type;	//more detail see 'video_stream_lib.h'
	ak_mutex_t lock;				//mutex lock
	ak_mutex_t close_mutex;			//close mutex
	int qp;
	int streams_count;				//current queue has nodes number

	struct ak_timeval env_time;		//check work env time
	enum video_work_scene pre_scene;//previous scene
	void *mdinfo;	//pointer to md info
};

/* global control handle */
struct video_ctrl_handle {
	unsigned char module_init;	//module init flag
	ak_mutex_t lock;			//mutex lock

	int inited_enc_grp_num;
	int thread_group_num;
	struct encode_group *grp[ENCODE_GRP_NUM];	//use for stream encode ctrl

	ak_mutex_t cancel_mutex;	//cancel mutex
	struct list_head venc_list;
	struct list_head thread_list;

#if CALC_VIDEO_CAPTURE
	int frame_num;
	struct ak_timeval calc_time;
#endif

#if CALC_VIDEO_ENCODE
	int stream_num;
	struct ak_timeval enc_time;
#endif
};

/* stream handle use for get and release stream */
struct stream_handle {
	void *vi_handle;
	void *enc_handle;
	int id;	//user id
};

static const char venc_version[] = "libmpi_venc V2.0.12";

#ifdef AK_RTOS
static struct video_ctrl_handle video_ctrl = {0, 0, 0};
ak_mutex_t *g_venc_open_lock __attribute__((__section__(".lock_init"))) = &video_ctrl.lock;
#else
static struct video_ctrl_handle video_ctrl = {
	0, PTHREAD_MUTEX_INITIALIZER, 0
};
#endif

/* --------------------- callbacks start --------------------- */
static T_pVOID enc_mutex_create(T_VOID)
{
	ak_mutex_t *pMutex = malloc(sizeof(ak_mutex_t));
	ak_thread_mutex_init(pMutex);
	
	return pMutex;
}

static T_S32 enc_mutex_lock(T_pVOID pMutex, T_S32 nTimeOut)
{
	ak_thread_mutex_lock(pMutex);
	return 1;
}

static T_S32 enc_mutex_unlock(T_pVOID pMutex)
{
	ak_thread_mutex_unlock(pMutex);
	return 1;
}

static T_VOID enc_mutex_release(T_pVOID pMutex)
{
	if (!pMutex) {
		return;
	}

	int rc = ak_thread_mutex_destroy( pMutex );
	if ( rc == EBUSY ) {
		ak_thread_mutex_unlock( pMutex );
		ak_thread_mutex_destroy( pMutex );
	}

	free(pMutex);
}

static T_BOOL ak_enc_delay(T_U32 ticks)
{
	akuio_wait_irq();
	return AK_TRUE;
}

static void* enc_hw_lock_cb(int hw_id)
{
	return NULL;
}

static int enc_hw_unlock_cb(void *lock_handle)
{
	return 0;
}

#if 0
static void set_gop_cb(void *handle, unsigned int gop)
{

    struct encode_group *enc_handle = (struct encode_group *)handle;
    //ak_print_normal_ex("handel: %p\n", handle);
	T_VIDEOLIB_ENC_PARA *param = &(enc_handle->enc_grp_param);

    enc_handle->video_rc.gopLen = gop;
	param->GOPlen = gop;
	ak_print_notice_ex("set %d chnannel gop: %d\n", enc_handle->grp_type, gop);
	VideoStream_Enc_set_GOP(enc_handle->lib_handle, param);
}
#endif
/* --------------------- callbacks end --------------------- */

/* ------------ internal api start ---------------------*/
#if VENC_SMOOTH_TS
static inline int get_frame_interval(struct encode_group *handle)
{
	if (!handle || !handle->encode_frames)
		return 0;

	return 1000 / handle->encode_frames;
}
#endif

static void *alloc_video_frame(void)
{
	struct frame_node *frame = (struct frame_node *)calloc(1,
			sizeof(struct frame_node));
	if (!frame)
		return NULL;

	frame->vframe = (struct video_input_frame *)calloc(1,
			sizeof(struct video_input_frame));
	if (!frame->vframe) {
		free(frame);
		frame = NULL;
	}

	return frame;
}

static void free_frame(void *frame)
{
	struct frame_node *pframe = (struct frame_node *)frame;

	if (pframe) {
		if (pframe->vframe) {
			free(pframe->vframe);
			pframe->vframe = NULL;
		}

		free(pframe);
		pframe = NULL;
	}
}

static void set_h264_encode_param(const struct encode_param *input,
		T_VIDEOLIB_ENC_OPEN_INPUT *open_param)
{
	/* H264 encoder */
	open_param->encFlag = VIDEO_DRV_H264;

	/* encode real width, to be divisible by 4 */
	open_param->encH264Par.width = input->width;
	/* encode real height, to be divisible by 4 */
	open_param->encH264Par.height = input->height;
	open_param->encH264Par.lumWidthSrc = input->width;
	open_param->encH264Par.lumHeightSrc = input->height;
	open_param->encH264Par.horOffsetSrc = 0;
	open_param->encH264Par.verOffsetSrc = 0;
	/* YUV rotation beforce encode */
	open_param->encH264Par.rotation = 0;
	/* denominator of frame rate */
	open_param->encH264Par.frameRateDenom = 1;
	/* molecule of frame rate */
	open_param->encH264Par.frameRateNum = input->fps;
	/* init value of QP */
	open_param->encH264Par.qpHdr = -1;
	/* 1 have start code; 0 no start code */
	open_param->encH264Par.streamType = 0;
	/* target bps, unit: kbps */
	open_param->encH264Par.bitPerSecond = input->bps * 1024;
	open_param->encH264Par.gopLen = input->goplen;

	/* test, use else */
#if 1
	/* profile set */
	switch (input->profile) {
	case PROFILE_MAIN:
		open_param->encH264Par.enableCabac = 1;
		open_param->encH264Par.transform8x8Mode = 0;
		ak_print_notice_ex("main profile\n");
		break;
	case PROFILE_HIGH:
		open_param->encH264Par.enableCabac = 1;
		open_param->encH264Par.transform8x8Mode = 2;
		ak_print_notice_ex("high profile\n");
		break;
	case PROFILE_BASE:
		open_param->encH264Par.enableCabac = 0;
		open_param->encH264Par.transform8x8Mode = 0;
		ak_print_notice_ex("base profile\n");
		break;
	default:	/* main profile is default */
		open_param->encH264Par.enableCabac = 1;
		open_param->encH264Par.transform8x8Mode = 0;
		ak_print_notice_ex("use main profile default\n");
		break;
	}
#else
	open_param->encH264Par.enableCabac = 1;
	open_param->encH264Par.transform8x8Mode = 0;
#endif

	open_param->encH264Par.qpMin = input->minqp;  //the minimize is 10
	open_param->encH264Par.qpMax = input->maxqp;  //the max is 51
	open_param->encH264Par.intraQpDelta = -4;
	open_param->encH264Par.mbQpAdjustment = -8;

	/*marks: may modify here */
	if(input->br_mode == BR_MODE_CBR) {
		open_param->encH264Par.fixedIntraQp = 0;
		open_param->encH264Par.hrdCpbSize = input->bps * 50;
	} else {
		open_param->encH264Par.hrdCpbSize = input->bps * 50;
		//open_param->encH264Par.fixedIntraQp = (input->maxqp + input->minqp);
		open_param->encH264Par.fixedIntraQp = (input->maxqp - input->minqp)-5;
	}
	/*marks : may modify here - end */

	/* Open macroblock QP control and drop frame function */
	open_param->encH264Par.mbRc = 0;
	open_param->encH264Par.hrd = 0;
	open_param->encH264Par.pictureSkip = 1;
}

static void set_h264_encode_rc(T_VIDEOLIB_ENC_RC *video_rc,
		T_VIDEOLIB_ENC_OPEN_INPUT open_param,
		enum bitrate_ctrl_mode br_mode)
{
	video_rc->qpHdr = -1;
	video_rc->qpMin = open_param.encH264Par.qpMin;
	video_rc->qpMax = open_param.encH264Par.qpMax;
	video_rc->fixedIntraQp = open_param.encH264Par.fixedIntraQp;
	video_rc->bitPerSecond =  open_param.encH264Par.bitPerSecond;
	video_rc->gopLen = open_param.encH264Par.gopLen;
	video_rc->intraQpDelta = -4;
	video_rc->mbQpAdjustment = -8;

	/* now this is same, but it should has some different */
	if(br_mode == BR_MODE_CBR)
		video_rc->hrdCpbSize = open_param.encH264Par.bitPerSecond * 50;
	else
		video_rc->hrdCpbSize = open_param.encH264Par.bitPerSecond * 50;
}

static void init_bitrate_ctrl_param(struct encode_group *handle, const struct encode_param *input,
		int fps, int gop)
{
	unsigned int temp = 0;
	T_VIDEOLIB_ENC_PARA video_enc_para = {0};

	video_enc_para.skipPenalty = 255;
	video_enc_para.interFavor = 335; //-1 for hantro drive code value
	video_enc_para.intra16x16Favor = 4323; //-1 for hantro drive code value
	video_enc_para.intra4x4Favor = -1; //-1 for hantro drive code value
	video_enc_para.chromaQPOffset = 127; //127 for hantro drive code value
	video_enc_para.diffMVPenalty4p = -1; //-1 for hantro drive code value
	video_enc_para.diffMVPenalty1p = -1; //-1 for hantro drive code value
	video_enc_para.minIQP = input->minqp;
	video_enc_para.maxIQP = input->maxqp;
	video_enc_para.adjustment_area_pencent = 30;

	/* get bitrate threshold reference value */
	temp = (input->bps * 1024) / (input->fps);

	/* set bitreate up and down threshold rate and delta value */
	video_enc_para.qp_up_bitsize_threshold1 = temp*6/5;
	video_enc_para.qp_up_delta1 = 1;
	video_enc_para.qp_up_bitsize_threshold2 = temp*3/2;
	video_enc_para.qp_up_delta2 = 2;
	video_enc_para.qp_up_bitsize_threshold3 = temp*2;
	video_enc_para.qp_up_delta3 = 3;
	video_enc_para.qp_down_bitsize_threshold1 = temp*9/10;
	video_enc_para.qp_down_delta1 = 1;
	video_enc_para.qp_down_bitsize_threshold2 = temp*7/10;
	video_enc_para.qp_down_delta2 = 2;
	video_enc_para.qp_down_bitsize_threshold3 = temp/2;
	video_enc_para.qp_down_delta3 = 3;
	video_enc_para.mbRows_threshold = 20;

	/* set other attributes */
	video_enc_para.debug = 0;
	video_enc_para.quarterPixelMv = 0; // 1 for enable, other value for disable
	video_enc_para.qp_filter_k = 32; //0~64

	video_enc_para.model = 80;		//init default

	video_enc_para.fps = fps;
	video_enc_para.GOPlen = gop;

	/* set param to encode lib */
	VideoStream_Enc_set_Rc_Param(handle->lib_handle, &video_enc_para);
}

static void set_mjpeg_enc_param(const struct encode_param *input,
		T_VIDEOLIB_ENC_OPEN_INPUT *open_param)
{
	open_param->encFlag = VIDEO_DRV_MJPEG;
	open_param->encMJPEGPar.frameType = ENC_YUV420_PLANAR;//JPEGENC_YUV420_PLANAR;
	open_param->encMJPEGPar.format = ENC_THUMB_JPEG;
	open_param->encMJPEGPar.thumbWidth = 0;
	open_param->encMJPEGPar.thumbHeight = 0;
	open_param->encMJPEGPar.thumbData = NULL;
	open_param->encMJPEGPar.thumbDataLen = 0;
	open_param->encMJPEGPar.qLevel = 7;
	open_param->encMJPEGPar.width = input->width;
	open_param->encMJPEGPar.height = input->height;
	open_param->encMJPEGPar.lumWidthSrc = input->width;
	open_param->encMJPEGPar.lumHeightSrc = input->height;
	open_param->encMJPEGPar.horOffsetSrc = 0;
	open_param->encMJPEGPar.verOffsetSrc = 0;
}

static void set_encode_br_mode(struct encode_group *handle)
{
	T_VIDEOLIB_ENC_PARA *param = &handle->enc_grp_param;

	if (handle->record.br_mode == BR_MODE_VBR)
		param->videomode = 1;
	else if (handle->record.br_mode == BR_MODE_CBR)
		param->videomode = 2;

	ak_print_normal_ex("set chn %d br: %s, %ld\n",
			handle->grp_type,
			handle->record.br_mode ? "VBR" : "CBR",
			param->videomode);
	VideoStream_Enc_set_videomode(handle->lib_handle, param);
}

static void set_encode_fps(struct encode_group *handle, int fps)
{
	T_VIDEOLIB_ENC_PARA *param = &(handle->enc_grp_param);

	param->fps = fps;
	ak_print_normal_ex("set %d chnannel fps: %d\n", handle->grp_type, fps);
	VideoStream_Enc_set_fps(handle->lib_handle, param);
}

static void set_encode_gop(struct encode_group *handle, int gop_len)
{
	T_VIDEOLIB_ENC_PARA *param = &(handle->enc_grp_param);

	param->GOPlen = gop_len;
	ak_print_normal_ex("set %d chnannel gop: %d\n", handle->grp_type, gop_len);
	VideoStream_Enc_set_GOP(handle->lib_handle, param);
}

static void set_channel_rc(struct encode_group *handle, int bps)
{
	T_VIDEOLIB_ENC_PARA *main_param = &(handle->enc_grp_param);

	main_param->Isize = 70;
	main_param->method = 0;

	/* 720P VBR */
	if (BR_MODE_VBR == handle->record.br_mode) {
		main_param->model = 80;
		ak_print_normal_ex("VBR video mode: %ld\n", main_param->model);
	} else {	/* 720P CBR */
		main_param->model = 80;
		ak_print_normal_ex("CBR video mode: %ld\n", main_param->model);
	}

	main_param->skipPenalty 	= 255;
	main_param->interFavor 		= 335; //-1 for hantro drive code value
	main_param->intra16x16Favor = 4323; //-1 for hantro drive code value
	main_param->intra4x4Favor 	= -1; //-1 for hantro drive code value
	main_param->chromaQPOffset 	= 127; //127 for hantro drive code value
	main_param->diffMVPenalty4p = -1; //-1 for hantro drive code value
	main_param->diffMVPenalty1p = -1; //-1 for hantro drive code value
	main_param->debug 			= 0;
	main_param->quarterPixelMv 	= 0; // 1 for enable, other value for disable
	main_param->qp_filter_k 	= 32; //0~64
	main_param->mbRows_threshold = 20;
	main_param->minIQP = handle->video_rc.qpMin;
	main_param->maxIQP = handle->video_rc.qpMax;
	main_param->adjustment_area_pencent = 30;

	VideoStream_Enc_set_Rc_Param(handle->lib_handle, main_param);
}

static int venc_handle_init(int index) 
{
	if (video_ctrl.grp[index]) {
		ak_print_normal_ex("group %d's handle has already opened\n", index);	
		return AK_SUCCESS;
	}

	video_ctrl.grp[index] = (struct encode_group *)calloc(1,
			sizeof(struct encode_group));
	if (!video_ctrl.grp[index]) {
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
		ak_thread_mutex_unlock(&video_ctrl.lock);
		return AK_FAILED;
	}

	ak_thread_mutex_init(&video_ctrl.grp[index]->lock);
	ak_thread_mutex_init(&video_ctrl.grp[index]->close_mutex);
	INIT_LIST_HEAD(&video_ctrl.grp[index]->enc_list);
	ak_get_ostime(&(video_ctrl.grp[index]->env_time));
	video_ctrl.grp[index]->pre_scene = VIDEO_SCENE_UNKNOWN;

	return AK_SUCCESS;
}

/**
 * init video encoder, register callback
 * return: 1 seccess, 0 failed
 */
static int venc_encoder_init(void)
{
	T_VIDEOLIB_CB video_cb = {0};

	/* set callback */
	video_cb.m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;
	video_cb.m_FunMalloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	video_cb.m_FunFree = free;
	video_cb.m_FunCleanInvalidateDcache = akuio_clean_invalidate_dcache;
	video_cb.m_FunRtcDelay = ak_enc_delay;

	video_cb.m_FunDMAMalloc	=
		(MEDIALIB_CALLBACK_FUN_DMA_MALLOC)akuio_alloc_pmem;
	video_cb.m_FunDMAFree =
		(MEDIALIB_CALLBACK_FUN_DMA_FREE)akuio_free_pmem;
	video_cb.m_FunVaddrToPaddr =
		(MEDIALIB_CALLBACK_FUN_VADDR_TO_PADDR)akuio_vaddr2paddr;
	video_cb.m_FunMapAddr =
		(MEDIALIB_CALLBACK_FUN_MAP_ADDR)akuio_map_regs;
	video_cb.m_FunUnmapAddr =
		(MEDIALIB_CALLBACK_FUN_UNMAP_ADDR)akuio_unmap_regs;
	video_cb.m_FunRegBitsWrite =
		(MEDIALIB_CALLBACK_FUN_REG_BITS_WRITE)akuio_sysreg_write;
		
	video_cb.m_FunVideoHWLock =
		(MEDIALIB_CALLBACK_FUN_VIDEO_HW_LOCK)enc_hw_lock_cb;
	video_cb.m_FunVideoHWUnlock =
		(MEDIALIB_CALLBACK_FUN_VIDEO_HW_UNLOCK)enc_hw_unlock_cb;

#if 0
    video_cb.m_FuncSetGop =
		(MEDIALIB_CALLBACK_FUN_SET_GOP)set_gop_cb;
#endif		

	/* add for using api about fifo in multithread */
	video_cb.m_FunMutexCreate = enc_mutex_create;
	video_cb.m_FunMutexLock = enc_mutex_lock;
	video_cb.m_FunMutexUnlock = enc_mutex_unlock;
	video_cb.m_FunMutexRelease = enc_mutex_release;

	return VideoStream_Enc_Init(&video_cb);
}

/*
 * add_user - add user
 * enc_handle[IN], encode handle
 * return current handle's user counter number
 */
static int add_user(void *enc_handle)
{
	struct encode_group *handle = (struct encode_group *)enc_handle;

	handle->user_count++;
	return handle->user_count;
}

/*
 * del_user - delete user
 * enc_handle[IN], encode handle
 * return current handle's user counter number
 */
static int del_user(void *enc_handle)
{
	struct encode_group *handle = (struct encode_group *)enc_handle;

	ak_thread_mutex_lock(&handle->lock);
	if (handle->user_count > 0)
		handle->user_count--;
	ak_thread_mutex_unlock(&handle->lock);

	return handle->user_count;
}

static int release_group_streams(void *enc_handle)
{
	if (!enc_handle) {
		return AK_FAILED;
	}

	struct encode_group *handle = (struct encode_group *)enc_handle;
	if (handle->req_ref > 0) {
		ak_print_error_ex("this group has still in use\n");
		return AK_FAILED;
	}

	struct video_streams_t *pos = NULL;
	struct video_streams_t *n = NULL;

	ak_print_info_ex("entry..., before: %d\n", handle->streams_count);
	ak_thread_mutex_lock(&handle->lock);
	list_for_each_entry_safe(pos, n, &handle->stream_list, list) {
		if (pos->stream.data) {
			free(pos->stream.data);
			pos->stream.data = NULL;
		}
		list_del_init(&pos->list);
		free(pos);
		pos = NULL;
		handle->streams_count--;
	}
	INIT_LIST_HEAD(&handle->stream_list);
	ak_thread_mutex_unlock(&handle->lock);
	ak_print_info_ex("entry..., after: %d\n", handle->streams_count);

	return AK_SUCCESS;
}

static void encode_group_exit(void *enc_handle)
{
	struct encode_group *handle = (struct encode_group *)enc_handle;
	ak_print_info_ex("enc_handle:%p, type:%d\n",
			handle, handle->record.enc_grp);

	ak_thread_mutex_lock(&handle->lock);
	if (handle->lib_handle) {
		VideoStream_Enc_Close(handle->lib_handle);
		handle->lib_handle = NULL;
		ak_print_normal_ex("VideoStream_Enc_Close OK\n");
	}

	if (handle->output) {
		akuio_free_pmem(handle->output);
		handle->output = NULL;
	}
	ak_thread_mutex_unlock(&handle->lock);
	ak_thread_mutex_destroy(&handle->lock);

	ak_thread_mutex_lock(&video_ctrl.lock);
	if (list_empty(&video_ctrl.venc_list)) {
		VideoStream_Enc_Destroy();
		ak_thread_mutex_destroy(&video_ctrl.cancel_mutex);
		video_ctrl.module_init = AK_FALSE;
	}
	ak_thread_mutex_unlock(&video_ctrl.lock);
}

static void encode_thread_group_exit(void)
{
	ak_print_info_ex("enter, inited_enc_grp_num=%d\n",
		video_ctrl.inited_enc_grp_num);
	/*
	 * All encode has close, so we close all thread group.
	 * Now it is cancel only one thread group, close specific
	 * thread group is not implement.
	 */
	if (!video_ctrl.inited_enc_grp_num) {
		struct thread_arg *arg, *n;

		list_for_each_entry_safe(arg, n, &video_ctrl.thread_list, list) {
			arg->cap_run = 0;
			ak_thread_sem_post(&arg->cap_sem);
			ak_print_normal_ex("join capture thread, tid=%lu\n", arg->cap_tid);
			ak_thread_join(arg->cap_tid);
			ak_print_notice_ex("join capture thread OK\n");

			arg->enc_run = 0;
			ak_thread_sem_post(&arg->enc_sem);
			ak_print_normal_ex("join encode thread, tid=%lu\n", arg->enc_tid);
			ak_thread_join(arg->enc_tid);
			ak_print_notice_ex("join encode thread OK\n");

			list_del(&arg->list);
			free(arg);
			arg = NULL;

			ak_print_info_ex("free thread arg OK\n");
			ak_thread_mutex_lock(&video_ctrl.lock);
			video_ctrl.thread_group_num--;
			ak_thread_mutex_unlock(&video_ctrl.lock);
		}
	}
	ak_print_info_ex("leave ...\n");
}

/**
 * frames_ctrl - decide whether current group need encode
 * group_type[IN]: encode group type number
 * max_frame[IN]: encode max frame
 * ts[IN]: capture frame ts
 * return:  0 unneed it, 1 need
 */
static int frames_ctrl(int group_type, const int max_frame, 
					unsigned long long ts)
{
	int ret = 0;
	struct encode_group *grp = video_ctrl.grp[group_type];
	unsigned int enc_frame = grp->encode_frames;

	/* get camera capture frames and video encode frames */
	if(max_frame <= enc_frame && (max_frame == enc_frame && max_frame != 12 )) {
		return 1;
	}

	if (enc_frame <= 0) {
		return 0;
	}
		
	unsigned long long enc_ts = ((grp->frame_index * 1000) / enc_frame);
	if (ts >= enc_ts) {
		if ((enc_ts > 0) && (ts - enc_ts) > 200) {
			ak_print_warning("group_type=%d, index=%llu, ts=%llu, enc_ts=%llu\n",
				group_type, grp->frame_index, ts, enc_ts);
		}

		ret = 1;
		++grp->frame_index;

		enc_ts = ((grp->frame_index * 1000) / enc_frame);
		if (enc_ts < ts) {
			grp->frame_index = (((ts + (1000/enc_frame)) * enc_frame) / 1000);
		}
	}

	return ret;
}

static void check_sensor_fps_switch(struct thread_arg *thread_arg,
				const int sensor_fps)
{
	int changed = 0;
	if (sensor_fps != thread_arg->sensor_pre_fps) {
		if (thread_arg->sensor_pre_fps) {
			ak_print_warning_ex("\t*** sensor fps switch %d to %d ***\n\n",
				thread_arg->sensor_pre_fps, sensor_fps);
			changed = 1;
		}

		thread_arg->sensor_pre_fps = sensor_fps;

		int i = 0;
		struct encode_group *grp = NULL;

		/* compare sensor's real fps and appointed record fps */
		for (i = 0; i < ENCODE_GRP_NUM; i++) {
			grp = video_ctrl.grp[i];
			if (!grp) 
				continue;
			if (sensor_fps < grp->record.fps) {
				grp->encode_frames = sensor_fps;
			} else {
				grp->encode_frames = grp->record.fps;
			}

			/* 
			 * if sensor frame rate change cause encode frame rate change too,
			 * we should notify encoder to adjust encode
			 */
			if (changed && (grp->encode_frames > 0))
				set_encode_fps(grp, grp->encode_frames);
		}
	}
}

static void add_to_encode_list(struct thread_arg *thread_arg,
		struct frame_node *frame)
{
	if (!thread_arg)
		return ;

	int i = 0;
	int bit_map = 0;
	const int sensor_fps = ak_vi_get_fps(thread_arg->vi_handle);
	unsigned long long ts = frame->vframe->vi_frame[VIDEO_CHN_MAIN].ts;

	check_sensor_fps_switch(thread_arg, sensor_fps);

	/* uses comfirm */
	for (i = 0; i < ENCODE_GRP_NUM; i++) {
		if (video_ctrl.grp[i] > 0) {
			/* decide whether current frame should be encode */
			if (frames_ctrl(i, sensor_fps, ts))
				set_bit(&bit_map, i);
		}
	}

	if (bit_map) {
		frame->bit_map = bit_map;

		ak_thread_mutex_lock(&thread_arg->lock);
		list_add_tail(&frame->list, &thread_arg->head_frame);
		ak_thread_mutex_unlock(&thread_arg->lock);

		/* notify encode thread to work */
		ak_thread_sem_post(&thread_arg->enc_sem);
	} else {
		/* no one need, release it at once */
		ak_vi_release_frame(thread_arg->vi_handle, frame->vframe);
		free_frame(frame);
	}
}

#if CALC_VIDEO_CAPTURE
static void calc_video_capture_frame(struct video_input_frame *vframe)
{
	struct ak_timeval cur_time;

	ak_get_ostime(&cur_time);
	++(video_ctrl.frame_num);
	long diff_time = ak_diff_ms_time(&cur_time, &(video_ctrl.calc_time));
	
	/* calc frame number per ten seconds */
	if(diff_time >= 10*1000){
		int total = video_ctrl.frame_num;
		int seconds =  (diff_time / 1000);
		if (seconds > 0) {
			struct frame *main_frame = &(vframe->vi_frame[VIDEO_CHN_MAIN]);
			ak_print_info("new capture ts=%llu, len=%u, seq_no=%lu\n",
				main_frame->ts, main_frame->len, main_frame->seq_no);
			ak_print_info("*** second=%d, total=%d, "
				"average=%d ***\n\n", seconds, total, (total / seconds));
		}

		ak_get_ostime(&(video_ctrl.calc_time));
		video_ctrl.frame_num = 0;
	}
}
#endif

#if CALC_VIDEO_ENCODE
static void calc_venc_stream(void)
{
	struct ak_timeval cur_time;

	ak_get_ostime(&cur_time);
	++(video_ctrl.stream_num);
	long diff_time = ak_diff_ms_time(&cur_time, &(video_ctrl.enc_time));
	
	/* calc frame number per ten seconds */
	if(diff_time >= 10*1000){
		int total = video_ctrl.stream_num;
		int seconds =  (diff_time / 1000);

		if (seconds > 0) {
			ak_print_info("*** encode second=%d, total=%d, "
				"average=%d ***\n\n", seconds, total, (total / seconds));
		}

		ak_get_ostime(&(video_ctrl.enc_time));
		video_ctrl.stream_num = 0;
	}
}
#endif

#if VENC_CALC_TS
static unsigned long long calc_stream_ts(struct encode_group *enc_handle,
		unsigned long long origin_ts)
{
	/* origin ts should be capture ts */
	unsigned long long calc_ts = 0;
	int ts_interval = (1000 / enc_handle->encode_frames);

	if (enc_handle->pre_ts > 0) {
		calc_ts = (enc_handle->pre_ts + ts_interval);
		if (abs(calc_ts - origin_ts) > 20) {
			calc_ts = origin_ts;
		}
	} else {
		calc_ts = origin_ts;
		ak_print_notice_ex("we get first stream for group %d\n",
				enc_handle->grp_type);
		ak_print_normal_ex("encode_frames=%d, ts_interval=%d\n",
				enc_handle->encode_frames, ts_interval);
		ak_print_normal_ex("first stream ts=%llu\n", origin_ts);
	}

	enc_handle->pre_ts = calc_ts;

	ak_print_info_ex("calc_ts=%llu, origin ts=%llu, diff=%lld\n",
		calc_ts, origin_ts, (calc_ts - origin_ts));

	return calc_ts;
}
#endif

#if VENC_SMOOTH_TS
static unsigned long long smooth_ts(struct encode_group *handle, 
								unsigned long long ts)
{
	if (!handle || !handle->encode_frames)
		return 0;

	unsigned long long cur_ts = 0;

	if (handle->smooth_index == 0)
	{	
		//ak_print_normal("ygh:ERR:smooth_index is 0\n");
		handle->smooth_index = ts * handle->encode_frames / 1000 - 1;
	}
	handle->smooth_index++;
	cur_ts = handle->smooth_index * 1000ULL / handle->encode_frames;

	if ((cur_ts > ts + 2 * 1000 / handle->encode_frames) ||
			(cur_ts + 2 * 1000 / handle->encode_frames) < ts) {

			handle->smooth_index = ts * handle->encode_frames / 1000 - 1;
			handle->smooth_index++;
			cur_ts = handle->smooth_index * 1000ULL / handle->encode_frames;
	}

	return cur_ts;
	#if 0
	unsigned long long cur_ts = 0;
	int actual_fitv = get_frame_interval(handle);

	if (handle->pre_ts) {
		cur_ts = handle->pre_ts + actual_fitv;
	} else {
		cur_ts = ts;
	}

	/* re-sync ts */
	if (1000 % handle->encode_frames) {
		int tmp = (actual_fitv / 2);
		/* excluding cumulative remainder ts */
		if ((ts > (cur_ts + tmp)) && (ts < (cur_ts + actual_fitv))) {
			cur_ts = ts;
		}
	}
	
	/* interval is too large */
	if (ts > (cur_ts + 10*actual_fitv)) {
		ak_print_warning_ex("cur_ts=%llu, ts=%llu, diff=%llu(ms)\n",
			cur_ts, ts, (ts - cur_ts));
		cur_ts = ts;
	}

	return cur_ts;
	#endif
}
#endif

static void capture_encode_frame(struct thread_arg *thread_arg)
{
	int ret = AK_FAILED;
	struct frame_node *frame = NULL;
	
#if VENC_CALC_CAP_TIME
	long cap_time = 0;
	struct ak_timeval pre_tv;
	struct ak_timeval cur_tv;
	ak_get_ostime(&pre_tv);
#endif

#if CALC_VIDEO_CAPTURE
	ak_get_ostime(&video_ctrl.calc_time);
#endif

	while (thread_arg->cap_run && (video_ctrl.inited_enc_grp_num > 0)) {
		/*
		 * new one frame object, then get frame store to head_frame,
		 * encode thread will according to the bit_map to decide
		 * which group should be encode.
		 */
		frame = alloc_video_frame();
		if (frame) {
#if VENC_CALC_CAP_TIME
			ak_get_ostime(&cur_tv);
#endif			
			ret = ak_vi_get_frame(thread_arg->vi_handle, frame->vframe);
#if VENC_CALC_CAP_TIME
			cap_time = ak_diff_ms_time(&cur_tv, &pre_tv);
			if (cap_time > 50) {
				ak_print_notice("ak_vi_get_frame diff time=%ld(ms), seq_no=%ld\n", 
					cap_time, frame->vframe->vi_frame[VIDEO_CHN_MAIN].seq_no);
			}
			ak_get_ostime(&pre_tv);
#endif

#if CALC_VIDEO_CAPTURE
			calc_video_capture_frame(frame->vframe);
#endif

			if (AK_SUCCESS == ret) {
				add_to_encode_list(thread_arg, frame);
			}
			ak_sleep_ms(10);
		} else {
			ak_print_error_ex("alloc_video_frame failed\n");
			thread_arg->cap_run = AK_FALSE;
			break;
		}
	}

	ak_print_notice_ex("leave...\n");
}

/*
 * capture_thread - capture thread, do capture and then wake up encode thread
 * arg[IN], thread argument, now no use
 * return: when it exit, return NULL
 */
static void *capture_thread(void *arg)
{
	struct thread_arg *thread_arg = (struct thread_arg *)arg;
	ak_print_normal_ex("start to capture, thread id: %ld\n", ak_thread_get_tid());

	while (thread_arg->cap_run) {
		ak_print_notice_ex("capture thread sleep...\n");
		/* wait signal to start capture */
		ak_thread_sem_wait(&thread_arg->cap_sem);
		ak_print_notice_ex("capture thread wakeup...\n");

		if (thread_arg->cap_run) {
			capture_encode_frame(thread_arg);
		}
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

static void check_work_scene_changed(struct encode_group *enc_handle)
{
	struct ak_timeval cur_time;

	ak_get_ostime(&cur_time);
	if (ak_diff_ms_time(&cur_time, &(enc_handle->env_time)) >= 1000) {
		enum video_work_scene cur_scene = ak_vi_get_work_scene(VIDEO_DEV0);
		if (cur_scene != enc_handle->pre_scene) {
			T_VIDEOLIB_ENC_PARA *param = &(enc_handle->enc_grp_param);

			ak_print_notice_ex("current work scene=%d\n", cur_scene);
			param->environment = cur_scene;
			VideoStream_Enc_enviroment(enc_handle->lib_handle, param);
			enc_handle->pre_scene = cur_scene;
		}
		ak_get_ostime(&(enc_handle->env_time));
	}
}

static void save_streams(struct video_streams_t *vs,
		struct encode_group *handle)
{
	/* if queue is full, drop first */
	if (handle->streams_count + 1 > ONE_STREAM_MAX) {
		/*
		 * 1.Drop p frame at first of frame list
		 * 2.If all p frame in one gop was been dropped, we drop the I frame
		 *   to implement drop a whole gop frames.
		 */
		struct video_streams_t *first_frame;
		struct video_streams_t *remove_frame = NULL;

		first_frame = list_first_entry(&handle->stream_list,
				struct video_streams_t, list);
		ak_print_error_ex("stream is too many, chn: %d, diff ts: %llu\n",
				handle->grp_type, (vs->stream.ts - first_frame->stream.ts));

		if (first_frame->stream.frame_type == FRAME_TYPE_I) {
			struct video_streams_t *second_frame;

			second_frame = list_first_entry(handle->stream_list.next,
					struct video_streams_t, list);
			if (second_frame->stream.frame_type == FRAME_TYPE_I) {
				remove_frame = first_frame;	/* drop I frame */
			} else {
				remove_frame = second_frame; /* drop P frame */
			}
		} else /* first frame is P */
			remove_frame = first_frame;

		ak_print_info_ex("remove %s Frame, sz=%u, seq_no=%ld\n",
			remove_frame->stream.frame_type == FRAME_TYPE_I ? "I" : "P",
			remove_frame->stream.len, remove_frame->stream.seq_no);
		handle->streams_count--;
		list_del_init(&remove_frame->list);
		free(remove_frame->stream.data);
		free(remove_frame);
	}

	/* save to queue */
	handle->streams_count++;

#if VENC_GET_STREAM_DEBUG
	ak_print_warning_ex("grp_type=%d, streams_count=%d\n",
		handle->grp_type, handle->streams_count);
	ak_print_normal_ex("vs=%p, stream ts=%llu, len=%u, seq_no=%lu\n",
		vs, vs->stream.ts, vs->stream.len, vs->stream.seq_no);
#endif

	/* add stream to group's queue */
	list_add_tail(&vs->list, &handle->stream_list);
	vs->ref = handle->req_ref;
	vs->bit_map = handle->user_map;
}

static void frame_to_stream(struct encode_group *enc_handle,
							struct frame_node *pos)
{
	int chn = enc_handle->record.use_chn;
	const unsigned char *buf = pos->vframe->vi_frame[chn].data;
	unsigned int frame_len = pos->vframe->vi_frame[chn].len;
	unsigned long long ts = pos->vframe->vi_frame[chn].ts;
	enc_handle->mdinfo = pos->vframe->mdinfo;

#if VENC_CALC_ENC_TIME
	struct ak_timeval tv_start;
	struct ak_timeval tv_end;
	long enc_time = 0;
#endif	

	struct video_streams_t *vs = (struct video_streams_t *)calloc(1, 
		sizeof(struct video_streams_t));
	if (!vs) {
		ak_print_error_ex("calloc failed\n");
		return;
	}

	vs->stream.seq_no = pos->vframe->vi_frame[chn].seq_no;
#if VENC_ORIGIN_TS
	vs->stream.ts = ts;
#elif VENC_CALC_TS
	vs->stream.ts = calc_stream_ts(enc_handle, ts);
#elif VENC_SMOOTH_TS
	/* smooth ts */
	vs->stream.ts = smooth_ts(enc_handle, ts);
#endif

	if (vs->stream.ts == 0) {
		free(vs);
		vs = NULL;
		return;
	}

#if VENC_CALC_ENC_TIME
	ak_get_ostime(&tv_start);
#endif

	/* encode */
	if (ak_venc_send_frame(enc_handle, buf, frame_len, &vs->stream)) {
		ak_print_error_ex("encode frame failed\n");
	} else {
		if (0 == vs->stream.len)
			ak_print_info_ex("vs->stream.len=%d\n", vs->stream.len);
	}
	
#if VENC_CALC_ENC_TIME		
	ak_get_ostime(&tv_end);
	enc_time = ak_diff_ms_time(&tv_end, &tv_start);
	if (enc_time > 50) {
		ak_print_warning(">>> encode frame type=%d, seq_no=%ld, len=%d, time=%ld\n", 
			vs->stream.frame_type, vs->stream.seq_no, vs->stream.len, enc_time);
	}
#endif			

#if CALC_VIDEO_ENCODE
	calc_venc_stream();
#endif

	/* save bitstream */
	if (vs->stream.len > 0) {
		enc_handle->pre_ts = vs->stream.ts;
		ak_thread_mutex_lock(&enc_handle->lock);
		save_streams(vs, enc_handle);
		ak_thread_mutex_unlock(&enc_handle->lock);
	} else {
		free(vs);
		vs = NULL;
	}	

	/* after this group has been encode, clear bit */
	enum encode_group_type type = enc_handle->record.enc_grp;
	clear_bit(&pos->bit_map, type);
}

static void encode_frame(struct thread_arg *thread_arg)
{
	/* one frame corresponding multiple encode group */
	struct encode_group *enc_handle = NULL;	//video encode handle
	struct frame_node *pos = NULL;
	struct frame_node *n = NULL;		//frame loop value

	list_for_each_entry_safe(pos, n, &thread_arg->head_frame, list) {
		if (!(thread_arg->enc_run)) {
			break;
		}

		ak_thread_mutex_lock(&video_ctrl.cancel_mutex);
		/* get encode handle, use frame to encode */
		list_for_each_entry(enc_handle, &video_ctrl.venc_list, enc_list) {
			check_work_scene_changed(enc_handle);
			ak_thread_mutex_lock(&enc_handle->close_mutex);
			if ((enc_handle->req_ref > 0)
				&& test_bit(enc_handle->record.enc_grp, &pos->bit_map)) {
					frame_to_stream(enc_handle, pos);
			}
			ak_thread_mutex_unlock(&enc_handle->close_mutex);

			if (!(thread_arg->enc_run)) {
				break;
			}
		}
		ak_thread_mutex_unlock(&video_ctrl.cancel_mutex);

		ak_thread_mutex_lock(&thread_arg->lock);
		list_del_init(&pos->list);
		ak_thread_mutex_unlock(&thread_arg->lock);
		ak_vi_release_frame(thread_arg->vi_handle, pos->vframe);
		free_frame(pos);
	}
}

static void *encode_thread(void *arg)
{
	struct thread_arg *thread_arg = (struct thread_arg *)arg;

	long int tid = ak_thread_get_tid();
	ak_print_normal_ex("start to encode, thread id: %ld\n", tid);

	while (thread_arg->enc_run) {
		ak_thread_sem_wait(&thread_arg->enc_sem);

		if (thread_arg->enc_run) {
			encode_frame(thread_arg);
		}
	}
	ak_print_normal_ex("### thread id: %ld exit ###\n", tid);
	ak_thread_exit();
	return NULL;
}

static void* venc_find_user(struct stream_handle *handle, struct list_head *head)
{
	unsigned char find = AK_FALSE;
	struct video_streams_t *pos = NULL;

	list_for_each_entry(pos, head, list) {
#if VENC_GET_STREAM_DEBUG
		ak_print_notice_ex("bit_map=%d, ref=%d\n", pos->bit_map, pos->ref);
#endif	
		if (test_bit(handle->id, &(pos->bit_map))) {
			/* this entry have not gotten yet */
			find = AK_TRUE;
			break;
		}
	}
	if (!find) {
		pos = NULL;
	}
	
	return pos;
}

static int start_service_work(void *vi)
{
	struct thread_arg *arg;

	if (!list_empty(&video_ctrl.thread_list)) {

		arg = list_first_entry(&video_ctrl.thread_list, struct thread_arg, list);
		if (arg->vi_handle == vi) {
			/* same vi handle come in, let it success */
			return AK_SUCCESS;
		} else {
			/*
			 * we not permit to register two or more
			 * different vi handle in same device for
			 * encode, so here we return false.
			 */
			ak_print_error("same video input device can just register"
					"only one vi-handle for encode\n");
			return AK_FAILED;
		}
	}

	arg = (struct thread_arg *)calloc(1, sizeof(struct thread_arg));
	if (!arg) {
		return AK_FAILED;
	}

	ak_thread_sem_init(&arg->cap_sem, 0);
	ak_thread_sem_init(&arg->enc_sem, 0);

	ak_thread_mutex_init(&arg->lock);
	INIT_LIST_HEAD(&arg->head_frame);		//store frame
	list_add_tail(&arg->list, &video_ctrl.thread_list);

	arg->vi_handle = vi;
	arg->cap_run = 1;
	arg->enc_run = 1;
#ifdef AK_RTOS
	ak_thread_create(&arg->cap_tid, capture_thread, (void *)arg,
		ANYKA_THREAD_NORMAL_STACK_SIZE, 85);
#else
	ak_thread_create(&arg->cap_tid, capture_thread, (void *)arg,
		ANYKA_THREAD_NORMAL_STACK_SIZE, 90);
#endif
	ak_print_normal_ex("create capture_thread, tid=%lu\n", arg->cap_tid);

#ifdef AK_RTOS
	ak_thread_create(&arg->enc_tid, encode_thread, (void *)arg,
		ANYKA_THREAD_NORMAL_STACK_SIZE, 85);
#else
	ak_thread_create(&arg->enc_tid, encode_thread, (void *)arg,
		ANYKA_THREAD_NORMAL_STACK_SIZE, 90);
#endif
	ak_print_normal_ex("create encode_thread, tid=%lu\n", arg->enc_tid);

	ak_thread_mutex_lock(&video_ctrl.lock);
	video_ctrl.thread_group_num++;	//record thread group number
	ak_thread_mutex_unlock(&video_ctrl.lock);

	/* notify, start to capture and encode */
	ak_thread_sem_post(&arg->cap_sem);
	ak_print_normal_ex("service start\n\n");

	return 0;
}

const char *ak_venc_get_version(void)
{
	return venc_version;
}

/**
 * ak_venc_open - open encoder and set encode param
 * @param[IN]: encode param
 * return: on success return encode handle, failed return NULL.
 */
void* ak_venc_open(const struct encode_param *param)
{
	if (!param) {
		ak_print_error_ex("invalid param: %p\n", param);
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return NULL;
	}

	ak_thread_mutex_lock(&video_ctrl.lock);
	/* init model global handle member */
	if (!video_ctrl.module_init) {
		video_ctrl.module_init = AK_TRUE;
		if (!venc_encoder_init()) {
			video_ctrl.module_init = AK_FALSE;
			ak_thread_mutex_unlock(&video_ctrl.lock);
			set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
			return NULL;
		}
		INIT_LIST_HEAD(&video_ctrl.venc_list);
		ak_thread_mutex_init(&video_ctrl.cancel_mutex);
	}

	/* init specifically handle's resource */
	int index = param->enc_grp;
	int venc_init_ret = venc_handle_init(index);
	ak_thread_mutex_unlock(&video_ctrl.lock);
	if (venc_init_ret)
		return NULL;

	/* init encode handle */
	int type = param->enc_grp;
	struct encode_group *ret_handle = NULL;
	ak_print_normal_ex("now init type %d\n", type);

	/* to make multiple thread operate safe */
	struct encode_group *enc_handle = video_ctrl.grp[type];

	ak_thread_mutex_lock(&enc_handle->lock);
	if(enc_handle->user_count > 0) {
		add_user(enc_handle);
		ret_handle = enc_handle;
		ak_print_normal_ex("add user ...\n");
		goto venc_open_end;
	}

	/* open new encode group handle */
	enc_handle->output = akuio_alloc_pmem(ENC_OUT_MAX_SZ);
	if (enc_handle->output == NULL) {
		ak_print_error_ex("akuio_alloc_pmem\n");
		set_error_no(ERROR_TYPE_PMEM_MALLOC_FAILED);
		goto venc_open_end;
	}

	/* encode buffer should 8bytes align, convert by DMA buf address */
	unsigned long tmp = akuio_vaddr2paddr(enc_handle->output) & 7;
	enc_handle->encbuf = ((unsigned char *)enc_handle->output) + ((8 - tmp) & 7);

	enc_handle->ip_frame = 0;
	enc_handle->reset_iframe = 0;
	enc_handle->record.br_mode = param->br_mode;
	memcpy(&enc_handle->record, param, sizeof(struct encode_param));

	T_VIDEOLIB_ENC_OPEN_INPUT open_param = {0};
	/* h.264 encode set */
	if (param->enc_out_type == H264_ENC_TYPE) {
		/* store to handle value and set video rc */
		set_h264_encode_param(param, &open_param);
		enc_handle->drv_enc_type = VIDEO_DRV_H264;
		enc_handle->qp = open_param.encH264Par.fixedIntraQp; //VBR mode
		set_h264_encode_rc(&enc_handle->video_rc, open_param, param->br_mode);

		ak_print_normal("H.264 params:\n\tw=%ld, h=%ld, qpMin=%ld, qpMax=%ld,"
				"bps=%ld, gop=%ld, fps=%ld\n",
				open_param.encH264Par.width,
				open_param.encH264Par.height,
				open_param.encH264Par.qpMin,
			   	open_param.encH264Par.qpMax,
				open_param.encH264Par.bitPerSecond,
			   	open_param.encH264Par.gopLen,
				open_param.encH264Par.frameRateNum);
	} else /* MJPEG encoder */ {
		enc_handle->drv_enc_type = VIDEO_DRV_MJPEG;
		set_mjpeg_enc_param(param, &open_param);
		ak_print_normal("JPEG params:\n\tw=%ld, h=%ld\n",
			open_param.encMJPEGPar.width, open_param.encMJPEGPar.height);
	}

	/* open encoder */
    //open_param.priv = enc_handle;
	enc_handle->lib_handle = VideoStream_Enc_Open(&open_param);
	if (!enc_handle->lib_handle) {
		akuio_free_pmem(enc_handle->output);
		ak_print_error_ex("VideoStream_Enc_Open!\n");
		set_error_no(ERROR_TYPE_MEDIA_LIB_FAILED);
		goto venc_open_end;
	}

	INIT_LIST_HEAD(&enc_handle->stream_list);
	if (param->enc_out_type == H264_ENC_TYPE) {
		int bps = open_param.encH264Par.bitPerSecond;
		int fps = open_param.encH264Par.frameRateNum;
		int gop = open_param.encH264Par.gopLen;

		init_bitrate_ctrl_param(enc_handle, param, fps, gop);
		set_channel_rc(enc_handle, bps);
		set_encode_br_mode(enc_handle);
	}

	/* handle should record which enc_grp its belong to */
	enc_handle->grp_type = type; //record group type
	add_user(enc_handle);
	ret_handle = enc_handle;
	ak_print_normal_ex("allocating type=%d, enc_handle: %p\n\n",
		type, enc_handle);

venc_open_end:
	ak_thread_mutex_unlock(&enc_handle->lock);

	return ret_handle;
}

/*
 * ak_venc_send_frame - encode single frame
 * @enc_handle[IN]: encode handle
 * @frame[IN]: frame which you want to encode
 * @frame_len[IN]: lenght of frame
 * @stream[OUT]: encode output buffer address
 * return: 0 success; -1 failed
 */
int ak_venc_send_frame(void *enc_handle,
		const unsigned char *frame,
		unsigned int frame_len,
		struct video_stream *stream)
{
	if (enc_handle == NULL) {
		ak_print_error_ex("invalid enc_handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	struct encode_group *handle = (struct encode_group *)enc_handle;

	if (!handle->mdinfo) {
		unsigned short mdstat[16][32] = {{0}};
        handle->mdinfo = (void *)mdstat;
	}

    /* reset Iframe outsite */
	if (handle->reset_iframe) {
		handle->reset_iframe = 0;
		handle->ip_frame = 0;
	}

	int ret = AK_FAILED;
	T_VIDEOLIB_ENC_IO_PAR video_enc_io_param2;
	
	if (BR_MODE_CBR == handle->record.br_mode)		//CBR
		video_enc_io_param2.QP = 0;
	else	//VBR
		video_enc_io_param2.QP = handle->qp;

	/* I frame */
	if ((handle->drv_enc_type == VIDEO_DRV_H264 && handle->ip_frame == 0)
			|| (handle->drv_enc_type == VIDEO_DRV_MJPEG)) {
		stream->frame_type = FRAME_TYPE_I;
		video_enc_io_param2.mode = 0;
	} else { /* P frame */
		stream->frame_type = FRAME_TYPE_P;
		video_enc_io_param2.mode = 1;
	}
	video_enc_io_param2.p_curr_data = (unsigned char *)frame;
	video_enc_io_param2.p_vlc_data = handle->encbuf;
	video_enc_io_param2.out_stream_size = ENC_OUT_MAX_SZ;

    /* for smart 264 */
    //video_enc_io_param2.md_info = handle->mdinfo;

	/* do encode */
	ak_thread_mutex_lock(&video_ctrl.lock);
	VideoStream_Enc_Encode(handle->lib_handle, NULL, &video_enc_io_param2, NULL);
	ak_thread_mutex_unlock(&video_ctrl.lock);

	/* set out put address ??? dma mem addr can delivery by this way? */
	if (video_enc_io_param2.out_stream_size > 0) {
		stream->data = (unsigned char *)calloc(1, video_enc_io_param2.out_stream_size);
		if (stream->data) {
			memcpy(stream->data, video_enc_io_param2.p_vlc_data,
				video_enc_io_param2.out_stream_size);
		} else {
			ak_print_error_ex("no memory to alloc for %lu byte(s)\n",
				   	video_enc_io_param2.out_stream_size);
			stream->len = 0;
			set_error_no(ERROR_TYPE_MALLOC_FAILED);
			goto send_frame_end;
		}
	}
	stream->len = video_enc_io_param2.out_stream_size;

	/* return before modify ip_frame, otherwise this frame will not reencode */
	if(!video_enc_io_param2.out_stream_size && !handle->ip_frame) {
		ak_print_error("encode I frame failed, reencode it\n");
		set_error_no(ERROR_TYPE_NO_DATA);
		goto send_frame_end;
	}

	/* judge next frame's attr, I frame or P frame */
	if (++handle->ip_frame >= handle->video_rc.gopLen)
		handle->ip_frame = 0;

	if(!video_enc_io_param2.out_stream_size) {
		//ak_print_error("encode size 0\n");
		set_error_no(ERROR_TYPE_NO_DATA);
		goto send_frame_end;
	}
	ret = AK_SUCCESS;

send_frame_end:
	if (AK_FAILED == ret) {
		if (stream->data) {
			free(stream->data);
			stream->data = NULL;
		}
	}

	return 0;
}

/*
 * ak_venc_send_frame_ex - encode single frame
 * @enc_handle[IN]: encode handle
 * @frame[IN]: frame which you want to encode
 * @frame_len[IN]: lenght of frame
 * @stream[OUT]: encode output buffer address
 * return: 0 success; -1 failed
 * note: IMPORTANT-make sure your stream->data can contain encoded data
 */
int ak_venc_send_frame_ex(void *enc_handle,
		const unsigned char *frame,
		unsigned int frame_len,
		struct video_stream *stream)
{
	if (!enc_handle) {
		ak_print_error_ex("invalid enc_handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	if (!stream) {
		ak_print_error_ex("stream is NULL\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}
	if (!(stream->data)) {
		ak_print_error_ex("stream->data is NULL\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	struct encode_group *handle = (struct encode_group *)enc_handle;
	/* reset Iframe outsite */
	if (handle->reset_iframe) {
		handle->reset_iframe = 0;
		handle->ip_frame = 0;
	}

	T_VIDEOLIB_ENC_IO_PAR venc_io_param;
	if (BR_MODE_CBR == handle->record.br_mode)
		venc_io_param.QP = 0;
	else	//VBR
		venc_io_param.QP = handle->qp;

	/* I frame */
	if ((handle->drv_enc_type == VIDEO_DRV_H264 && handle->ip_frame == 0)
		|| (handle->drv_enc_type == VIDEO_DRV_MJPEG)) {
		stream->frame_type = FRAME_TYPE_I;
		venc_io_param.mode = 0;
	} else { /* P frame */
		stream->frame_type = FRAME_TYPE_P;
		venc_io_param.mode = 1;
	}
	venc_io_param.p_curr_data = (unsigned char *)frame;
	venc_io_param.p_vlc_data = stream->data;
	venc_io_param.out_stream_size = ENC_OUT_MAX_SZ;

	/* do encode */
	ak_thread_mutex_lock(&video_ctrl.lock);
	VideoStream_Enc_Encode(handle->lib_handle, NULL, &venc_io_param, NULL);
	ak_thread_mutex_unlock(&video_ctrl.lock);
	
	if (venc_io_param.out_stream_size > 0) {
		stream->len = venc_io_param.out_stream_size;
	} else {
		stream->len = 0;
	}

	/* return before modify ip_frame, otherwise this frame will not reencode */
	if(!venc_io_param.out_stream_size && !handle->ip_frame) {
		ak_print_error("encode I frame failed, reencode it\n");
		set_error_no(ERROR_TYPE_NO_DATA);
		return AK_FAILED;
	}

	/* judge next frame's attr, I frame or P frame */
	if (++handle->ip_frame >= handle->video_rc.gopLen)
		handle->ip_frame = 0;

	if(!venc_io_param.out_stream_size) {
		ak_print_error("encode size 0\n");
		set_error_no(ERROR_TYPE_NO_DATA);
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

/*
 * ak_venc_get_fps - get encode frames per second
 * @enc_handle[IN]: enc_handle return by 'ak_venc_open'
 * return: on success the fps, failed 0
 */
int ak_venc_get_fps(void *enc_handle)
{
	struct encode_group *handle = (struct encode_group *)enc_handle;
	if (!handle) {
		ak_print_error_ex("un init handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return 0;
	}

	return handle->record.fps;
}

/*
 * ak_venc_set_fps-reset encode fps
 * @enc_handle[IN]: encode handle return by 'ak_venc_open'
 * @fps[IN]: fps you want to set
 * return: 0 on success, -1 no effect
 */
int ak_venc_set_fps(void *enc_handle, int fps)
{
	struct encode_group *handle = (struct encode_group *)enc_handle;
	if (handle == NULL) {
		ak_print_error_ex("invalid handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return -1;
	}

	ak_thread_mutex_lock(&handle->lock);
	handle->record.fps = fps;
	handle->encode_frames = handle->record.fps;
	ak_print_notice_ex("set chn[%d] fps to : %d\n", handle->grp_type, fps);
	ak_thread_mutex_unlock(&handle->lock);

	set_encode_fps(handle, fps);

	return 0;
}

/*
 * ak_venc_get_kbps - get encode kbps
 * @enc_handle[IN]:  enc_handle return by 'ak_venc_open'
 * return: >=0 kbps ;-1:failed
 */
int ak_venc_get_kbps(void *enc_handle)
{
	struct encode_group *handle = (struct encode_group *)enc_handle;

	/** for motion jpeg enconde type **/
	if (handle->drv_enc_type == VIDEO_DRV_MJPEG)
		return -1;
	/** get bps **/
	int kbps = handle->video_rc.bitPerSecond >> 10;
	return kbps;
}

/*
 * ak_venc_set_rc - reset encode bitpersecond
 * @enc_handle[IN]: encode handle return by 'ak_venc_open'
 * @bps[IN]: bps you want to set
 * return: 0 on success, others faield or no effect
 */
int ak_venc_set_rc(void *enc_handle, int bps)
{
	struct encode_group *handle = (struct encode_group *)enc_handle;
	if (!handle) {
		ak_print_error_ex("invalid handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return -1;
	}

	/** for motion jpeg enconde type **/
	if(handle->drv_enc_type == VIDEO_DRV_MJPEG)
		return 0;
	/** set qpmin according to bps **/
	handle->video_rc.bitPerSecond = bps * 1024;
	ak_print_info_ex("set chn to %d(kbps)\n", bps);

	set_channel_rc(handle, bps);

	int ret = -1;
	ak_print_notice_ex("set chn[%d] bps to : %d\n", handle->grp_type, bps);
	ak_thread_mutex_lock(&handle->lock);
	if (VideoStream_Enc_setRC(handle->lib_handle, &handle->video_rc))
		ret = 0;
	ak_thread_mutex_unlock(&handle->lock);

	return ret;
}

/**
 * ak_venc_set_iframe - set next encode frame to I frame
 * @enc_handle[IN], encode handle, return by 'ak_venc_open'
 * return: 0 on success, -1 failed
 */
int ak_venc_set_iframe(void *enc_handle)
{
	struct encode_group *handle = (struct encode_group *)enc_handle;
	if (handle) {
		handle->reset_iframe = 1;
		return 0;
	} else {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return -1;
	}
}

/**
 * ak_venc_get_gop_len - get encode gop len
 * @enc_handle[IN]: encode handle
 * return: value of GOP len, -1 failed
 * notes: 
 */
int ak_venc_get_gop_len(void *enc_handle)
{
	if (!enc_handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	struct encode_group *handle = (struct encode_group *)enc_handle;

	return handle->video_rc.gopLen;
}

/**
 * ak_venc_set_gop_len - set encode gop len
 * @enc_handle[IN]: encode handle
 * @gop_len[IN]: value of GOP len
 * return: 0 success, -1 failed
 * notes: set new gop len after you change encode fps
 */
int ak_venc_set_gop_len(void *enc_handle, int gop_len)
{
	struct encode_group *handle = (struct encode_group *)enc_handle;
	if(handle) {
		handle->video_rc.gopLen = gop_len;
		ak_print_notice_ex("set gop: %d\n", gop_len);
		set_encode_gop(handle, gop_len);
	}

	return AK_SUCCESS;
}

/**
 * ak_venc_set_roi - set ROI param
 * @enc_handle[IN]: encode handle
 * @roi[IN]: ROI param
 * return: 0 success, -1 failed
 * notes: call this function after ak_venc_open() or ak_venc_set_rc()
 */
int ak_venc_set_roi(void *enc_handle, struct venc_roi_param *roi)
{
	if (!enc_handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	if (!roi) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	struct encode_group *handle = (struct encode_group *)enc_handle;

	T_VIDEOLIB_ENC_PARA *param = &(handle->enc_grp_param);

	param->ROIenable = roi->enable;
	param->ROItop = roi->top / 16;
	param->ROIbot = roi->bottom / 16;
	param->ROIleft = roi->left / 16;
	param->ROIright = roi->right / 16;
	param->ROIDeltaQP = roi->delta_qp;

	ak_print_normal_ex("ROIenable : %ld\n", param->ROIenable);
	ak_print_normal_ex("ROItop: %ld\n", param->ROItop);
	ak_print_normal_ex("ROIbot: %ld\n", param->ROIbot);
	ak_print_normal_ex("ROIleft: %ld\n", param->ROIleft);
	ak_print_normal_ex("ROIright: %ld\n", param->ROIright);
	ak_print_normal_ex("ROIDeltaQP: %ld\n", param->ROIDeltaQP);

	VideoStream_Enc_set_Rc_Param(handle->lib_handle, param);

	return AK_SUCCESS;
}

int ak_venc_set_br(void *handle, enum bitrate_ctrl_mode mode)
{
	if (!handle) {
		ak_print_error_ex("invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	struct encode_group *enc_handle = (struct encode_group *)handle;

	ak_print_notice_ex("set chn %d br: %s\n", enc_handle->grp_type,
			mode ? "VBR" : "CBR");
	enc_handle->record.br_mode = mode;
	set_encode_br_mode(enc_handle);

	return AK_SUCCESS;
}

int ak_venc_set_profile(void *handle, enum profile_mode profile)
{
	if (!handle) {
		ak_print_error_ex("invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

/**
 * ak_venc_set_rc_weight - set video rate control weight
 * @enc_handle[IN]: encode handle
 * @weight[IN]: quality weight, [0, 100]
 * return: 0 success, -1 failed
 * notes:  quality weight 0 is the best quality, 100 is the lowest bitrate.
 */
int ak_venc_set_rc_weight(void *enc_handle, int weight)
{
	if (!enc_handle) {
		ak_print_error_ex("invalid NULL handle\n");
		return AK_FAILED;
	}

	if ((weight > 100) || (weight < 0)) {
		ak_print_error_ex("invalid param, weight must in range [0, 100]\n");
		return AK_FAILED;
	}

	struct encode_group *handle = (struct encode_group *)enc_handle;
	T_VIDEOLIB_ENC_PARA *main_param = &(handle->enc_grp_param);

	main_param->model = weight;
	VideoStream_Enc_set_Rc_Param(handle->lib_handle, main_param);
		
	return AK_SUCCESS;
}

/**
 * ak_venc_set_mjpeg_qlevel - set mjpeg quality level
 * @handle[IN]: encode handle return by ak_venc_open()
 * @level[IN]:quality level, 0-9
 * return: 0 success, specifically error number
 */
int ak_venc_set_mjpeg_qlevel(void *handle, int level)
{
	if (!handle) {
		ak_print_error_ex("invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

	if (level >= 0 && level <= 9) {
		struct encode_group *enc_handle = (struct encode_group *)handle;
		T_VIDEOLIB_PARA video_enc_para = {.qlevel = level};

		MJPEG_Enc_qlevel(enc_handle->lib_handle, &video_enc_para);
	} else {
		ak_print_error_ex("argument out of range: %p\n", handle);
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

/**
 * ak_venc_close - close video encode
 * @enc_handle[IN]: encode handle return by ak_venc_open()
 * return: 0 success, -1 failed
 * notes:
 */
int ak_venc_close(void *enc_handle)
{
	if (!enc_handle) {
		ak_print_error_ex("invalid handle: %p\n", enc_handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	struct encode_group *handle = (struct encode_group *)enc_handle;
	enum encode_group_type grp_type = handle->grp_type;

	ak_print_notice_ex("start close encode handle, chn: %d\n", grp_type);
	ak_thread_mutex_lock(&handle->close_mutex);
	int user_cnt = del_user(handle);
	/* user decrease to zero, close handle */
	if (user_cnt == 0) {
		ak_print_info_ex("release group %d's resource\n", grp_type);
		encode_group_exit(handle);
	}
	ak_thread_mutex_unlock(&handle->close_mutex);

	if (user_cnt == 0) {
		ak_thread_mutex_destroy(&handle->close_mutex);

		ak_print_normal_ex("enc_handle:%p, type:%d exit OK, free handle\n",
			handle, handle->record.enc_grp);

		ak_thread_mutex_lock(&video_ctrl.lock);
		video_ctrl.grp[handle->record.enc_grp] = NULL;
		ak_thread_mutex_unlock(&video_ctrl.lock);

		free(handle);
		handle = NULL;

		ak_print_normal_ex("enc_handle free OK\n");
	}

	ak_print_notice_ex("close encode handle OK, chn: %d\n\n", grp_type);

	return AK_SUCCESS;
}

/**
 * ak_venc_request_stream - bind vi and venc handle start capture
 * @vi_handle[IN]: video input handle, return by ak_vi_open()
 * @enc_handle[IN]: encode handle, return by ak_venc_open()
 * return: on success return stream_handle, failed return NULL
 * notes:
 */
void *ak_venc_request_stream(void *vi_handle, void *enc_handle)
{
	if (!enc_handle || !vi_handle) {
		ak_print_error_ex("uninit handle! vi_handle=%p, enc_handle=%p\n",
			vi_handle, enc_handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return NULL;
	}

	struct stream_handle *new_handle = (struct stream_handle *)calloc(1,
			sizeof(struct stream_handle));
	if (!new_handle) {
		ak_print_error_ex("no mem\n");
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
		return NULL;
	}
	
	struct encode_group *handle = (struct encode_group *)enc_handle;

	/* this means start a new group encode */
	ak_thread_mutex_lock(&handle->lock);
	int i = 0;
	for (i=0; i<VENC_TYPE_MAX_USER; ++i) {
		if (!test_bit(i, &(handle->user_map))) {
			new_handle->id = i;
			break;
		}
	}
	if (i >= VENC_TYPE_MAX_USER) {
		ak_thread_mutex_unlock(&handle->lock);
		free(new_handle);
		new_handle= NULL;
		return NULL;
	}

    new_handle->vi_handle = vi_handle;
	new_handle->enc_handle = enc_handle;
    set_bit(&(handle->user_map), new_handle->id);    
	add_ref(&(handle->req_ref), 1);
	ak_print_notice_ex("now user's id=%d, req_ref=%d, type=%d\n",
		new_handle->id, handle->req_ref, handle->grp_type);
	ak_print_normal_ex("vi_handle=%p, enc_handle=%p\n",
		vi_handle, enc_handle);
		
	/* if this is first request on current group, initialize as below */
	if (handle->is_stream_mode == 0) {
		handle->capture_frames = ak_vi_get_fps(vi_handle);
		handle->encode_frames = handle->record.fps;
		handle->frame_index = 0;
		handle->streams_count = 0;
		handle->is_stream_mode = 1;

		video_ctrl.inited_enc_grp_num++;
		list_add_tail(&handle->enc_list, &video_ctrl.venc_list);
		ak_print_notice_ex("init grp_type=%d, handle=%p, now working group number: %d\n",
			handle->grp_type, handle, video_ctrl.inited_enc_grp_num);
	}
	ak_thread_mutex_unlock(&handle->lock);

	/* each unique video input device, create one service thread group */
	if (video_ctrl.module_init && (!video_ctrl.thread_group_num)) {
		INIT_LIST_HEAD(&video_ctrl.thread_list);
		ak_print_normal_ex("init thread list\n");
	}
	if (start_service_work(vi_handle)) {
		free(new_handle);
		new_handle= NULL;
	}

	return new_handle;
}

/**
 * ak_venc_get_stream - on stream-encode, get encode output stream
 * @stream_handle[IN]: stream handle
 * @stream[IN]: stream data
 * return: 0 success, failed return specifically error
 * notes:
 */
int ak_venc_get_stream(void *stream_handle, struct video_stream *stream)
{
	/* checke user handle and buffers legitimacy */
	if (!stream_handle || !stream) {
		ak_print_error_ex("invalid stream handle\n");
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	struct stream_handle *handle = (struct stream_handle *)stream_handle;
	struct encode_group *enc_handle = (struct encode_group *)handle->enc_handle;

	ak_thread_mutex_lock(&enc_handle->lock);
	/* check data capability */
	if (list_empty(&enc_handle->stream_list)) {
		set_error_no(ERROR_TYPE_NO_DATA);
		goto venc_get_stream_end;
	}

	/* 
	 * to find user's stream start position
	 * if found then get stream from this position
	 */
	struct video_streams_t *pos = venc_find_user(handle, &enc_handle->stream_list);
	if (pos) {
#if VENC_GET_STREAM_DEBUG
		ak_print_normal_ex("grp_type=%d, steam handle=%p, list head=%p\n",
			enc_handle->grp_type, handle, &enc_handle->stream_list);
		ak_print_normal_ex("pos=%p, ts=%llu, len=%u, seq_no=%lu\n",
			pos, pos->stream.ts, pos->stream.len, pos->stream.seq_no);
#endif
	} else {
		set_error_no(ERROR_TYPE_INVALID_USER);
		goto venc_get_stream_end;
	}

	/*
	 * only under the status than the stream's lenght is suitable,
	 * we will copy data to user
	 */
	if ((pos->stream.len > 0) && (pos->stream.len <= ENC_OUT_MAX_SZ)) {
		/* get data, should make sure the pos is right */
		stream->data = (unsigned char *)calloc(1, pos->stream.len);
		if (NULL == stream->data) {
			ak_print_error_ex("calloc failed, stream.len=%u\n", pos->stream.len);
			set_error_no(ERROR_TYPE_MALLOC_FAILED);
			goto venc_get_stream_end;
		}
	} else {
		ak_print_notice_ex("stream_handle=%p, pos=%p, frame_type=%d, ts=%llu, len=%u\n",
			stream_handle, pos, pos->stream.frame_type, 
			pos->stream.ts, pos->stream.len);
		set_error_no(ERROR_TYPE_NO_DATA);
		goto venc_get_stream_end;
	}

	/* copy data */
	memcpy(stream->data, pos->stream.data, pos->stream.len);
	stream->len = pos->stream.len;
	stream->ts = pos->stream.ts;
	stream->seq_no = pos->stream.seq_no;
	stream->frame_type = pos->stream.frame_type;
	clear_bit(&(pos->bit_map), handle->id);

#if VENC_GET_STREAM_DEBUG
	ak_print_notice_ex("grp_type=%d, id=%d, bit_map=%d\n",
		enc_handle->grp_type, handle->id, pos->bit_map);
	ak_print_notice_ex("ref=%d, streams_count=%d\n",
		pos->ref, enc_handle->streams_count);
#endif

	/* release data */
	del_ref(&(pos->ref), 1);
	if (pos->ref <= 0) {
#if VENC_GET_STREAM_DEBUG
		ak_print_notice_ex("del grp_type=%d, id=%d, bit_map=%d\n",
			enc_handle->grp_type, handle->id, pos->bit_map);
#endif
		/* if current data idle, release it */
		enc_handle->streams_count--;
		list_del_init(&pos->list);
		free(pos->stream.data);
		pos->stream.data = NULL;
		free(pos);
		pos = NULL;
	}
	ret = AK_SUCCESS;

venc_get_stream_end:
	ak_thread_mutex_unlock(&enc_handle->lock);

	return ret;
}

/**
 * ak_venc_release_stream - release stream resource
 * @stream_handle[IN]: stream handle
 * @stream[IN]: stream return by get()
 * return: 0 success, -1 failed
 * notes:
 */
int ak_venc_release_stream(void *stream_handle, struct video_stream *stream)
{
	if (!stream) {
		ak_print_error_ex("invalid stream\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return ERROR_TYPE_POINTER_NULL;
	}

	if(stream->data) {
		free(stream->data);
		stream->data = NULL;
	}

	return AK_SUCCESS;
}

/**
 * ak_venc_cancel_stream - cancel stream encode
 * @stream_handle[IN]: encode handle return by ak_venc_request_stream()
 * return: 0 success, specifically error number
 * notes:
 */
int ak_venc_cancel_stream(void *stream_handle)
{
	/* user legitimacy check */
	if (!stream_handle) {
		ak_print_error_ex("invalid handle: %p\n", stream_handle);
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

	struct stream_handle *handle = (struct stream_handle *)stream_handle;
	struct encode_group *enc_handle = (struct encode_group *)handle->enc_handle;
	enum encode_group_type grp_type = enc_handle->grp_type;

	ak_print_notice_ex("enter grp_type=%d, id=%d, steam handle=%p\n",
		grp_type, handle->id, stream_handle);

	if (enc_handle) {
		/* reference count decrease */
		ak_thread_mutex_lock(&enc_handle->lock);
		del_ref(&(enc_handle->req_ref), 1);
    	clear_bit(&(enc_handle->user_map), handle->id);
		ak_thread_mutex_unlock(&enc_handle->lock);
		
		ak_print_notice_ex("req_ref=%d, user_map=%d, is_stream_mode=%d\n", 
			enc_handle->req_ref, enc_handle->user_map, enc_handle->is_stream_mode);

		/* if resource idle, release it */
		if ((enc_handle->req_ref <= 0) && enc_handle->is_stream_mode) {
			/* decrease global number of group */
			ak_thread_mutex_lock(&video_ctrl.lock);
			video_ctrl.inited_enc_grp_num--;
			ak_thread_mutex_unlock(&video_ctrl.lock);

			/* release capture and encode thread resource */
			encode_thread_group_exit();
			/* release all un-taken-of streams on current group */
			release_group_streams(enc_handle);

			/* take of encode-handle from encode-handle-list */
			ak_thread_mutex_lock(&video_ctrl.cancel_mutex);
			list_del_init(&enc_handle->enc_list);
			ak_thread_mutex_unlock(&video_ctrl.cancel_mutex);

			/* clear stream mode flag */
			enc_handle->is_stream_mode = 0;
			ak_print_info_ex("release stream resource ok\n");
		}
	}

	/* release user's stream handle */
	free(handle);
	handle = NULL;
	ak_print_notice_ex("grp_type=%d, leave...\n\n", grp_type);

	return AK_SUCCESS;
}
