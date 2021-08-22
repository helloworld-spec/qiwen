#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "media_muxer_lib.h"
#include "mux_fs.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_sd_card.h"
#include "ak_mux.h"

#define MPI_MUX			        "<mpi_mux>"

/* just for debug */
#define MUX_WRITE_VIDEO_FILE	0
#define MUX_WRITE_AUDIO_FILE	0
#define MUX_WRITE_VIDEO_EXINFO	0
#define MUX_WRITE_AUDIO_EXINFO	0

#define MUX_HANDLE_NUM          4

#define INDEX_BYTES_AVI 		(16)
#define INDEX_BYTES_MP4			(16)
#define MUX_MAX_FPS 			(25)

#if (MUX_WRITE_VIDEO_FILE || MUX_WRITE_AUDIO_FILE)
static int mux_write_no;
static enum audio_mux_type mux_audio_type;
#endif



struct mux_handle {
	FILE *index_fp;						//index file describe
	FILE *record_fp;					//record file describe
	FILE *moov_fp;
	FILE *stbl_fp;
	void *media_handle;					//media handle
	char *fs_iobuf;						//file system full buffer
	ak_mutex_t mutex;					//operate mutex

	unsigned long long pre_video_ts; 	//previous video timestamp(ms)
	unsigned long long pre_audio_ts; 	//previous audio timestamp(ms)
	unsigned long long start_ts; 		//mux timestamp(ms)
	unsigned long long video_mux_ts; 	//video mux timestamp(ms)
	unsigned long long audio_mux_ts; 	//audio mux timestamp(ms)

	/// 已经获取到关键帧标识。
	int gotidr;
};

static const char muxer_version[] = "libmpi_muxer V1.0.06";
static struct mux_handle mux_handle_array[MUX_HANDLE_NUM];

#if MUX_WRITE_AUDIO_FILE
static int audio_mux_flag = AK_TRUE;
static FILE *audio_mux_fp = NULL;

/* output audio mux file for debugging */
static void audio_write_mux_file(const T_MEDIALIB_MUX_PARAM *param)
{
	if (audio_mux_flag && !audio_mux_fp) {
		char tmp[50] = {0};

		ak_print_notice_ex(MPI_MUX "mux_write_no=%d\n", mux_write_no);

		switch(mux_audio_type) {
		case MEDIALIB_AUDIO_G711:		//g711a
			sprintf(tmp, "./mux_audio_%.4d.g711a", mux_write_no);
			/* open file, if not exist, create it; if exist, truncate it */
			audio_mux_fp = fopen(tmp, "w+");
			break;
		case MEDIALIB_AUDIO_AMR:		//arm
			sprintf(tmp, "./mux_audio_%.4d.amr", mux_write_no);
			audio_mux_fp = fopen(tmp, "w+");
			/* open file, if not exist, create it; if exist, truncate it */
			if (audio_mux_fp) {
				/* write amr-header */
				const unsigned char amrheader[]= "#!AMR\n";
				fwrite(amrheader, sizeof(amrheader) - 1, 1, audio_mux_fp);
			}
			break;
		default:
			return;
		}

		if(!audio_mux_fp) {
			ak_print_error_ex(MPI_MUX "create mux audio file error\n");
			return;
		}
	}

#if MUX_WRITE_AUDIO_EXINFO
	/* write audio ex-info */
	char tmp[4] = {0x5A, 0x5A, 0x5A, 0x5A};
	if(fwrite(tmp, 1, 4, audio_mux_fp) < 0) {
		ak_print_error_ex(MPI_MUX "write audio file err\n");
	}
	if(fwrite(&(param->m_ulTimeStamp), 1, 4, audio_mux_fp) < 0) {
		ak_print_error_ex(MPI_MUX "write audio file err\n");
	}
	if(fwrite(&(param->m_ulStreamLen), 1, 4, audio_mux_fp) < 0) {
		ak_print_error_ex(MPI_MUX "write audio file err\n");
	}
#endif

	/* write audio data */
	if(fwrite(param->m_pStreamBuf, 1, param->m_ulStreamLen, audio_mux_fp) < 0) {
		ak_print_error_ex(MPI_MUX "write audio file err\n");
	}

#if MUX_WRITE_AUDIO_EXINFO
	tmp[0] = 0x0A;
	if(fwrite(tmp, 1, 1, audio_mux_fp) < 0) {
		ak_print_error_ex(MPI_MUX "write audio file err\n");
	}
#endif
}
#endif

#if MUX_WRITE_VIDEO_FILE
static int video_mux_flag = AK_TRUE;
static FILE *video_mux_fp = NULL;

/* output video mux file for debugging */
static void video_write_mux_file(const T_MEDIALIB_MUX_PARAM *param)
{
	if (video_mux_flag && !video_mux_fp) {
		char tmp[50] = {0};

		ak_print_notice_ex(MPI_MUX "mux_write_no=%d\n", mux_write_no);
		sprintf(tmp, "./mux_video_%.4d.h264", mux_write_no);
		/* open file, if not exist, create it; if exist, truncate it */
		video_mux_fp = fopen(tmp, "w+");
		if(!video_mux_fp) {
			ak_print_error_ex(MPI_MUX "create mux video file error\n");
			return;
		}
	}

#if MUX_WRITE_VIDEO_EXINFO
	/* write video ex-info */
	char tmp[4] = {0x5A, 0x5A, 0x5A, 0x5A};
	if (fwrite(tmp, 1, 4, video_mux_fp) < 0)
		ak_print_error_ex(MPI_MUX "write video file err\n");
	if (fwrite(&(param->m_ulTimeStamp), 1, 4, video_mux_fp) < 0)
		ak_print_error_ex(MPI_MUX "write video file err\n");
	if (fwrite(&(param->m_ulStreamLen), 1, 4, video_mux_fp) < 0)
		ak_print_error_ex(MPI_MUX "write video file err\n");
#endif

	/* write video stream data */
	if(fwrite(param->m_pStreamBuf, 1, param->m_ulStreamLen, video_mux_fp) < 0)
		ak_print_error_ex(MPI_MUX "write video file err\n");

#if MUX_WRITE_VIDEO_EXINFO
	tmp[0] = 0x0A;
	if(fwrite(tmp, 1, 1, video_mux_fp) < 0)
		ak_print_error_ex(MPI_MUX "write video file err\n");
#endif
}
#endif

/*
 * mux_set_audio_input - set audio mux param
 * 					different param according to different type of audio
 * open[OUT]: pointer to mux lib config
 * param[IN]: pointer to max param, config from user
 * return: 0 on succees, -1 faield.
 */
static int mux_set_audio_input(T_MEDIALIB_MUX_OPEN_INPUT *open,
					const struct mux_input *param)
{
	int ret = AK_SUCCESS;

	/* assign for struct T_MEDIALIB_MUX_OPEN_INPUT */
	open->m_eAudioType = param->audio_type;		//audio type
	open->m_nSampleRate = param->sample_rate;	//sample rate
	open->m_nChannels = AUDIO_CHANNEL_MONO;		//single channel
	open->m_wBitsPerSample = 16;				//bits per sample

	/*
	 * according to audio_type, do assignment for audio mux arguments
	 */
	switch (param->audio_type) {
	case MEDIALIB_AUDIO_PCM:		//raw pcm
	    open->m_ulSamplesPerPack = ((param->sample_rate * 32) / 1000);
	    open->m_ulAudioBitrate = (param->sample_rate * open->m_wBitsPerSample
	        * open->m_ulSamplesPerPack);
	    break;
	case MEDIALIB_AUDIO_G711:		//g711
	    open->m_wFormatTag = param->format_tag;
	    open->m_nBlockAlign = 1;
	    open->m_cbSize = 0;
	    open->m_wBitsPerSample = 8;
	    /* make sure calculate this value, otherwise we can't config */
	    open->m_ulSamplesPerPack = ((param->sample_rate * param->frame_interval)
	    	/ 1000);		//divide 1000 means unit is second
	    open->m_ulAudioBitrate = (param->sample_rate * open->m_wBitsPerSample
	        * open->m_ulSamplesPerPack);
	    open->m_nAvgBytesPerSec = open->m_nSampleRate * open->m_nBlockAlign;
	    break;
	case MEDIALIB_AUDIO_AAC:	//aac
	    open->m_ulSamplesPerPack = open->m_nChannels * 1024;
	    open->m_cbSize = 2;
	    switch(open->m_nSampleRate){
	    case 8000:
	    case 11025:
	    case 12000:
	    case 16000:
	    case 22050:
	    case 24000:
	    case 32000:
	    case 44100:
	    case 48000:
	        open->m_ulAudioBitrate = open->m_nSampleRate;
	        break;
	    default:
	        open->m_ulAudioBitrate = 48000;
	        break;
	    }
	    break;
	case MEDIALIB_AUDIO_ADPCM:	//ad pcm, after compress
	    open->m_wFormatTag = 0x11;
	    open->m_wBitsPerSample = 4;
		/* assignment block-align base on sample-rate */
	    switch(open->m_nSampleRate) {
	    case 8000:
	    case 11025:
	    case 12000:
	    case 16000:
	        open->m_nBlockAlign = 0x100;
	        break;
	    case 22050:
	    case 24000:
	    case 32000:
	        open->m_nBlockAlign = 0x200;
	        break;
	    case 44100:
	    case 48000:
	    case 64000:
	        open->m_nBlockAlign = 0x400;
	        break;
	    default:
	        open->m_nBlockAlign = 0x400;
	        break;
	    }

		/*
		 * calculate sample-per-pack/audio-bit-rate
		 */
	    open->m_ulSamplesPerPack = ((((open->m_nBlockAlign - 4) * 8) / 4) + 1);
	    open->m_ulAudioBitrate = (open->m_nSampleRate
	                * open->m_nBlockAlign / open->m_ulSamplesPerPack);
	    open->m_nAvgBytesPerSec = open->m_ulAudioBitrate;
	    open->m_nBlockAlign *= open->m_nChannels;
	    open->m_cbSize = 2;
	    open->m_pszData = (T_U8 *)&(open->m_ulSamplesPerPack);
	    break;
	case MEDIALIB_AUDIO_AMR:		//amr
		open->m_ulAudioBitrate = 12200;
		open->m_ulSamplesPerPack = 160;
		break;
	default:
		ret = AK_FAILED;	//unsupported audio type
		break;
	}

	return ret;
}

/* get current version string pointer */
const char* ak_muxer_get_version(void)
{
	return muxer_version;
}

/**
 * ak_mux_open - open mux lib
 * @mux_param[IN]: mux input param
 * @record_fp[IN]: save file pointer
 * @index_fp[IN]: tmp file pointer
 * return: mux handle, NULL failed
 */
void* ak_mux_open(const struct mux_input *mux_param,
				FILE *record_fp, FILE *index_fp, FILE *moov_fp, FILE *stbl_fp)
{
	static char init_mux_handle = AK_FALSE;
	int i;
	struct mux_handle *mux = NULL;

	if (init_mux_handle == AK_FALSE) {
		init_mux_handle = AK_TRUE;
		memset(mux_handle_array, 0x0, sizeof(mux_handle_array));
	}

	if(NULL == mux_param){
	    return NULL;
	}

	for(i = 0; i < MUX_HANDLE_NUM; i++) {
		if(mux_handle_array[i].record_fp == NULL) {
			mux = mux_handle_array + i;
			break;
		}
	}
	if(mux == NULL) {
		mux = (struct mux_handle *)calloc(1, sizeof(struct mux_handle));
	}
	if(NULL == mux) {
	    return NULL;
	}

	mux->fs_iobuf = (char *)calloc(1, 1024*1024);
	mux->record_fp = record_fp;
	mux->index_fp = index_fp;
	mux->moov_fp = moov_fp;
	mux->stbl_fp = stbl_fp;
	mux->gotidr = AK_FALSE; ///< 初始化缺省未获取到关键帧。

	/* set fwrite io buffer to 64K */
	int set_vbuf_ret = setvbuf(mux->record_fp, mux->fs_iobuf, _IOFBF, 1024*1024);
	if (set_vbuf_ret != 0) {
		ak_print_error_ex("set file io full buffer fail, error:%d, %s\n",
				errno, strerror(errno));	
	}

	ak_thread_mutex_init(&(mux->mutex), NULL);

   	int ret = AK_FAILED;
	int bytes_per_frame = mux_param->media_rec_type == RECORD_TYPE_AVI_NORMAL ? 
		INDEX_BYTES_AVI : INDEX_BYTES_MP4;
	T_MEDIALIB_MUX_OPEN_INPUT open_input;

	memset(&open_input, 0, sizeof(T_MEDIALIB_MUX_OPEN_INPUT));
	/* fill correct params to mux */
	open_input.m_MediaRecType = mux_param->media_rec_type; //media type
	open_input.m_hMediaDest = (long)record_fp;				//record output file fd
	open_input.m_bCaptureAudio = mux_param->capture_audio;	//mux audio flag
	open_input.m_hMoovFile = (long)moov_fp;
	open_input.m_hStblFile = (long)stbl_fp;
	//open_input.m_bNeedSYN = 2;	//just insert video frame
    /* sync audio and video */
	if (RECORD_TYPE_3GP == mux_param->media_rec_type)
	    open_input.m_bNeedSYN = 2;
    else
        open_input.m_bNeedSYN = 1;
    
	open_input.m_bLocalMode = AK_TRUE;
	open_input.m_bIdxInMem = AK_TRUE; //AK_FALSE;
	open_input.m_ulIndexMemSize = 2 * MUX_MAX_FPS * 	//按照最大帧率计算，避免中途变帧导致index memsize 不足
		(mux_param->record_second + 3) * bytes_per_frame;
	ak_print_notice_ex("IndexMemSize: %lu\n", open_input.m_ulIndexMemSize);
	open_input.m_hIndexFile = (long)index_fp;		//index file

	/* for sync */
	open_input.m_ulVFifoSize = 512*1024; //video fifo size
	open_input.m_ulAFifoSize = 200*1024; //audio fifo size
	open_input.m_ulTimeScale = 1000;     //time scale

	/* set video open info */
	open_input.m_eVideoType = mux_param->video_type;
	open_input.m_nWidth = mux_param->width;
	open_input.m_nHeight = mux_param->height;

	open_input.m_nFPS = mux_param->file_fps;
	open_input.m_nKeyframeInterval = 0;
	open_input.m_nAudioPacksPerChunk = 3000;

	/* set audio open info */
	if(mux_param->capture_audio
		&& (AK_FAILED == mux_set_audio_input(&open_input, mux_param))) {
		goto mux_open_error;
	}

	open_input.m_nChannels = AUDIO_CHANNEL_MONO;
	/* register mux callback */
	open_input.m_CBFunc.m_FunPrintf= (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
	open_input.m_CBFunc.m_FunMalloc= (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	open_input.m_CBFunc.m_FunFree = (MEDIALIB_CALLBACK_FUN_FREE)free;
	open_input.m_CBFunc.m_FunRead = (MEDIALIB_CALLBACK_FUN_READ)mux_fs_read;
	open_input.m_CBFunc.m_FunSeek = (MEDIALIB_CALLBACK_FUN_SEEK)mux_fs_seek;
	open_input.m_CBFunc.m_FunTell = (MEDIALIB_CALLBACK_FUN_TELL)mux_fs_tell;
	open_input.m_CBFunc.m_FunWrite = (MEDIALIB_CALLBACK_FUN_WRITE)mux_fs_write;
	open_input.m_CBFunc.m_FunFileHandleExist = mux_fs_handle_exist;

	open_input.strVersion = MEDIA_LIB_VERSION;

	T_MEDIALIB_MUX_OPEN_OUTPUT open_output = {0};

	/* init mux write env */
	mux_fs_init(record_fp, index_fp);

	mux->media_handle = MediaLib_Mux_Open(&open_input, &open_output);
	if (NULL == mux->media_handle) {
		ak_print_normal_ex(MPI_MUX "open mux handle failed!\n");
	    goto mux_open_error;
	}

	T_MEDIALIB_MUX_INFO MuxInfo = {0};

	/* get mux info */
	if (!MediaLib_Mux_GetInfo(mux->media_handle, &MuxInfo)) {
		ak_print_normal_ex(MPI_MUX "mux get info failed!\n");
	    goto mux_open_error;
	}

	/* start mux */
	if (!MediaLib_Mux_Start(mux->media_handle)) {
		ak_print_normal_ex(MPI_MUX "mux start failed!\n");
	    goto mux_open_error;
	}

	/* delivery mux start times */
	mux->start_ts = mux_param->start_ts;
	ak_print_notice_ex(MPI_MUX "mux->start_ts=%llu\n", mux->start_ts);
	ret = AK_SUCCESS;

#if MUX_WRITE_VIDEO_FILE
	video_mux_flag = AK_TRUE;
#endif
#if MUX_WRITE_AUDIO_FILE
	mux_audio_type = mux_param->audio_type;
	audio_mux_flag = AK_TRUE;
#endif
#if (MUX_WRITE_VIDEO_FILE || MUX_WRITE_AUDIO_FILE)
	++mux_write_no;
#endif

	/* error handle */
mux_open_error:
	if(AK_FAILED == ret) {
		ak_print_normal_ex(MPI_MUX "it fail to open the handle!\n");
	    if(NULL != mux) {
	    	mux_fs_flush(record_fp);
		    if(NULL != mux->media_handle) {
		        MediaLib_Mux_Close(mux->media_handle);
		        mux->media_handle = NULL;
		    }

	        ak_thread_mutex_destroy(&mux->mutex);
	        free(mux);
	        mux = NULL;
	    }
	}

	return mux;
}

/**
 * ak_mux_add_audio - add audio to mux
 * @mux_param[IN]: opened mux handle
 * @audio[IN]: audio stream info
 * return: 0 success, -1 failed, >0 other mux status
 */
int ak_mux_add_audio(void *mux_handle, const struct audio_stream *audio)
{
	if (!mux_handle || !audio){
	    return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	T_MEDIALIB_MUX_PARAM param;
	T_eMEDIALIB_MUX_STATUS status;
	struct mux_handle *mux = (struct mux_handle *)mux_handle;

	if (audio->ts < mux->start_ts) {
		ak_print_warning(MPI_MUX "--- drop audio mux, ts:%llu, sz:%u ---\n",
	    	audio->ts, audio->len);
		return AK_SUCCESS;
	}

	param.m_pStreamBuf = audio->data;
	param.m_ulStreamLen = audio->len;
	/* use relative ts to mux */
	param.m_ulTimeStamp = (audio->ts - mux->start_ts);
	param.m_bIsKeyframe = AK_TRUE;

	/* write audio stream data for debug */
#if MUX_WRITE_AUDIO_FILE
	audio_write_mux_file(mux_handle, &param);
#endif

	/* write audio data to mux-lib fifo */
	ak_thread_mutex_lock(&(mux->mutex));
	if (MediaLib_Mux_AddAudioData(mux->media_handle, &param)) {
		/* must call mux_handle to mux data after AddAudioData */
		status = MediaLib_Mux_Handle(mux->media_handle);
		if ((MEDIALIB_MUX_DOING != status)
			&& (MEDIALIB_MUX_WAITING != status)) {
			ak_print_error(MPI_MUX "--- ERROR! Audio MUX handle, mux status=%d, "
				"ts:%llu, sz:%u ---\n", status, audio->ts, audio->len);
		} else {
			mux->pre_audio_ts = audio->ts;
			mux->audio_mux_ts = param.m_ulTimeStamp;
		}
	} else {
		/* audio mux failed, we can get mux status to analyze */
		ak_print_normal_ex(MPI_MUX "pre ts=%llu, cur ts=%llu, "
		    "pre mux_ts=%llu, cur mux_ts=%lu\n",
	    	mux->pre_audio_ts, audio->ts, mux->audio_mux_ts, param.m_ulTimeStamp);
	    ak_print_error(MPI_MUX "--- ERROR! Add Audio to MUX Failed! "
	        "ts:%llu, sz:%u ---\n",
	    	audio->ts, audio->len);
	    ret = AK_FAILED;
	}

	/* get current mux status */
	status = MediaLib_Mux_GetStatus(mux->media_handle);
	ak_thread_mutex_unlock(&(mux->mutex));

	if (ret) {
		ak_print_normal_ex(MPI_MUX "--- status=%d ---\n", status);
	}
	if ((MEDIALIB_MUX_SYSERR == status)
		|| (MEDIALIB_MUX_MEMFULL == status)) {
	    ak_print_error(MPI_MUX "[%s]: failed to mux with err(%d)\n",
	        __func__, status);
	    ret = status;
	}

	return ret;
}

/**
 * ak_mux_add_video - add video to mux
 * @mux_param[IN]: opened mux handle
 * @video[IN]: video stream info
 * return: 0 success, -1 failed, >0 other mux status
 */
int ak_mux_add_video(void *mux_handle, const struct video_stream *video)
{
	if(NULL == mux_handle){
	    return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	T_eMEDIALIB_MUX_STATUS status;
	T_MEDIALIB_MUX_PARAM param;
	struct mux_handle *mux = (struct mux_handle *)mux_handle;

	if (!mux->gotidr) {
		/// 等待地一个关键帧。
		if (FRAME_TYPE_I == video->frame_type) {
			mux->gotidr = AK_TRUE;
		} else {
			/// 返回一个错误，等待下一个关键帧。
			ak_print_error(MPI_MUX "--- ERROR! Skip Non-IDR Video Frame.\r\n");
			return MEDIALIB_MUX_SYNERR;
		}
	}

	param.m_pStreamBuf = video->data;
	param.m_ulStreamLen = video->len;
	/* use relative ts to mux */
	param.m_ulTimeStamp = (video->ts - mux->start_ts);
	param.m_bIsKeyframe = video->frame_type;

	/* write video stream data for debug */
#if MUX_WRITE_VIDEO_FILE
	video_write_mux_file(&param);
#endif

	/* write video data to mux-lib fifo */
	ak_thread_mutex_lock(&(mux->mutex));
	if(MediaLib_Mux_AddVideoData(mux->media_handle, &param)) {
		/* must call mux_handle to mux data after AddVideoData */
		status = MediaLib_Mux_Handle(mux->media_handle);
		if((MEDIALIB_MUX_DOING != status)
			&& (MEDIALIB_MUX_WAITING != status)) {
			ak_print_error(MPI_MUX "--- ERROR! Video MUX handle, mux status=%d, "
				"ts:%llu IF:%d, sz:%u ---\n",
				status, video->ts, video->frame_type, video->len);
		} else {
			mux->pre_video_ts = video->ts;	//just record for debug
			mux->video_mux_ts = param.m_ulTimeStamp;	//just record for debug
		}
	} else {
		/* video mux failed, we can get mux status to analyze */
		ak_print_normal_ex(MPI_MUX "pre ts=%llu, cur ts=%llu, "
		    "pre mux_ts=%llu, cur mux_ts=%lu\n",
	    	mux->pre_video_ts, video->ts, mux->video_mux_ts, param.m_ulTimeStamp);
		ak_print_error(MPI_MUX "--- ERROR! Add Video to MUX Failed! "
		    "ts:%llu, IF:%d, sz:%u ---\n",
			video->ts, video->frame_type, video->len);
		ret = AK_FAILED;
	}

	/* get current mux status */
	status = MediaLib_Mux_GetStatus(mux->media_handle);
	ak_thread_mutex_unlock(&(mux->mutex));

	if (ret)
		ak_print_normal_ex(MPI_MUX "--- status=%d ---\n", status);

	if ((MEDIALIB_MUX_SYSERR == status)
		|| (MEDIALIB_MUX_MEMFULL == status)) {
	    ak_print_error(MPI_MUX "[%s]: failed to mux with err(%d)\n",
	        __func__, status);
		ret = status;
	}

	return ret;
}

/**
 * ak_mux_fix_file - release mux lib resource
 * @record_fp[IN]: save file pointer
 * return: 0 success, -1 failed
 * notes: we just support fixed avi file.
 */
int ak_mux_fix_file(FILE *record_fp)
{
	if(!record_fp){
	    return AK_FAILED;
	}

	unsigned long avifid = (unsigned long)record_fp;
	/* idxfid=0 means create index info from avifid file */
	unsigned long idxfid = 0;
	/*
	 * outfid == avifid, fixed avi input avi file directly.
	 * outfid != avifid, copy input avi file to output file, keep input file.
	 */
	unsigned long outfid = avifid;
	T_MEDIALIB_CB fix_cb = {0};

	/* set fix callback functions */
	fix_cb.m_FunMalloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	fix_cb.m_FunFree = (MEDIALIB_CALLBACK_FUN_FREE)free;
	fix_cb.m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
	fix_cb.m_FunRead = (MEDIALIB_CALLBACK_FUN_READ)mux_fs_read;
	fix_cb.m_FunSeek = (MEDIALIB_CALLBACK_FUN_SEEK)mux_fs_seek;
	fix_cb.m_FunTell = (MEDIALIB_CALLBACK_FUN_TELL)mux_fs_tell;
	fix_cb.m_FunWrite = (MEDIALIB_CALLBACK_FUN_WRITE)mux_fs_write;

	/* API return false or true, we return failed or succeed, so convert it */
	if (AVI_FilePerfect(avifid, idxfid, outfid, &fix_cb) == AK_TRUE)
		return AK_SUCCESS;
	else
	    return AK_FAILED;
}

/**
 * ak_mux_free - release mux lib resource
 * @mux_handle[IN]: opened mux handle
 * return: 0 success, -1 failed
 * notes: we just free the unused handle resources. Don't execute flush and
 *		close operations.
 */
int ak_mux_free(void *mux_handle)
{
	if(NULL == mux_handle){
	    return AK_FAILED;
	}

	struct mux_handle *mux = (struct mux_handle *)mux_handle;
	if(NULL != mux->media_handle) {
		free(mux->media_handle);
		mux->media_handle = NULL;
	}

	mux->start_ts = 0;
	ak_thread_mutex_destroy(&(mux->mutex));
	if (((mux - mux_handle_array) < MUX_HANDLE_NUM) && ((mux - mux_handle_array) >= 0)) {
		memset( mux , 0x0 , sizeof( struct mux_handle ) ) ;
	}
	else{
		free(mux);
	}
	mux = NULL;

	return AK_SUCCESS;
}

/**
 * ak_mux_close - close mux lib
 * @mux_handle[IN]: opened mux handle
 * return: 0 success, -1 failed
 */
int ak_mux_close(void *mux_handle)
{
	if (NULL == mux_handle) {
	    return AK_FAILED;
	}

	struct mux_handle *mux = (struct mux_handle *)mux_handle;

	ak_thread_mutex_lock(&(mux->mutex));
	mux->start_ts = 0;
	/* flush record temp files */
	mux_fs_flush(mux->record_fp);
	mux_fs_flush(mux->index_fp);

	/* stop first and then close */
	if (!MediaLib_Mux_Stop(mux->media_handle)) {
		ak_print_normal(MPI_MUX "MediaLib_Mux_Stop FAILED\n");
	}
	if (!MediaLib_Mux_Close(mux->media_handle)) {
		ak_print_normal(MPI_MUX "MediaLib_Mux_Close FAILED\n");
	}
	free(mux->fs_iobuf);
	mux->media_handle = NULL;
	ak_thread_mutex_unlock(&(mux->mutex));
	ak_thread_mutex_destroy(&(mux->mutex));

	if (((mux - mux_handle_array) < MUX_HANDLE_NUM) && ((mux - mux_handle_array) >= 0)) {
		memset(mux , 0x0, sizeof(struct mux_handle));
	} else {
		free(mux);
	}
	mux = NULL;

	/* debug: close video */
#if (MUX_WRITE_VIDEO_FILE)
	if (video_mux_flag) {
		video_mux_flag = AK_FALSE;
		fclose(video_mux_fp);
		video_mux_fp = NULL;
	}
#endif

	/* debug: close audio */
#if (MUX_WRITE_AUDIO_FILE)
	if (audio_mux_flag) {
		audio_mux_flag = AK_FALSE;
		fclose(audio_mux_fp);
		audio_mux_fp = NULL;
	}
#endif

	return AK_SUCCESS;
}


int ak_mux_fileperfect(FILE *mp4file,FILE *moovfile,FILE *stblfile)
{
    T_MEDIALIB_CB CBFunc;
    memset(&CBFunc, 0, sizeof(T_MEDIALIB_CB));
    CBFunc.m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
    CBFunc.m_FunMalloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
    CBFunc.m_FunFree = (MEDIALIB_CALLBACK_FUN_FREE)free;
    CBFunc.m_FunRead = (MEDIALIB_CALLBACK_FUN_READ)mux_fs_read;
    CBFunc.m_FunSeek = (MEDIALIB_CALLBACK_FUN_SEEK)mux_fs_seek;
    CBFunc.m_FunTell = (MEDIALIB_CALLBACK_FUN_TELL)mux_fs_tell;
    CBFunc.m_FunWrite = (MEDIALIB_CALLBACK_FUN_WRITE)mux_fs_write;
   
    return MP4_FilePerfect((T_S32)mp4file, (T_S32)moovfile, (T_S32)stblfile, (T_S32)mp4file, &CBFunc);
}


