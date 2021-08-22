#include <string.h>
#include <unistd.h>

#include "media_muxer_lib.h"
#include "mux_fs.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_sd_card.h"
#include "ak_mux.h"

#ifdef AK_RTOS
#include "kernel.h"
#endif

#define MUX_WRITE_VIDEO_FILE	0
#define MUX_WRITE_AUDIO_FILE	0
#define MUX_WRITE_VIDEO_EXINFO	0
#define MUX_WRITE_AUDIO_EXINFO	0

#if (MUX_WRITE_VIDEO_FILE || MUX_WRITE_AUDIO_FILE)
static int mux_write_no;
static enum audio_mux_type mux_audio_type;
#endif

struct mux_handle {
    FILE *index_fp;
    FILE *record_fp;
    void *midea_handle;
    ak_mutex_t mutex;

	unsigned long long pre_video_ts; 	//previous video timestamp(ms)
	unsigned long long pre_audio_ts; 	//previous audio timestamp(ms)
	unsigned long long start_ts; 		//mux timestamp(ms)
};

static const char muxer_version[] = "libmpi_muxer V2.1.00";

#if MUX_WRITE_AUDIO_FILE
static int audio_mux_flag = AK_TRUE;
static FILE *audio_mux_fp = NULL;

static void audio_write_mux_file(const T_MEDIALIB_MUX_PARAM *param)
{
	if (audio_mux_flag && !audio_mux_fp) {
		char tmp[50] = {0};

		ak_print_notice_ex("mux_write_no=%d\n", mux_write_no);

		switch(mux_audio_type) {
		case MEDIALIB_AUDIO_G711:
			sprintf(tmp, "./mux_audio_%.4d.g711a", mux_write_no);
			audio_mux_fp = fopen(tmp, "w+");
			break;
		case MEDIALIB_AUDIO_AMR:
			sprintf(tmp, "./mux_audio_%.4d.amr", mux_write_no);
			audio_mux_fp = fopen(tmp, "w+");
			if (audio_mux_fp) {
				const unsigned char amrheader[]= "#!AMR\n";
				fwrite(amrheader, sizeof(amrheader) - 1, 1, audio_mux_fp);
			}
			break;
		default:
			return;
		}
		
		if(!audio_mux_fp) {
			ak_print_error_ex("create mux audio file error\n");
			return;
		}
	}

#if MUX_WRITE_AUDIO_EXINFO
	char tmp[4] = {0x5A, 0x5A, 0x5A, 0x5A};
	if(fwrite(tmp, 1, 4, audio_mux_fp) < 0) {
		ak_print_error_ex("write audio file err\n");
	}
	if(fwrite(&(param->m_ulTimeStamp), 1, 4, audio_mux_fp) < 0) {
		ak_print_error_ex("write audio file err\n");
	}
	if(fwrite(&(param->m_ulStreamLen), 1, 4, audio_mux_fp) < 0) {
		ak_print_error_ex("write audio file err\n");
	}
#endif

	if(fwrite(param->m_pStreamBuf, 1, param->m_ulStreamLen, audio_mux_fp) < 0) {
		ak_print_error_ex("write audio file err\n");
	}

#if MUX_WRITE_AUDIO_EXINFO
	tmp[0] = 0x0A;
	if(fwrite(tmp, 1, 1, audio_mux_fp) < 0) {
		ak_print_error_ex("write audio file err\n");
	}
#endif	
}
#endif

#if MUX_WRITE_VIDEO_FILE
static int video_mux_flag = AK_TRUE;
static FILE *video_mux_fp = NULL;

static void video_write_mux_file(const T_MEDIALIB_MUX_PARAM *param)
{
	if (video_mux_flag && !video_mux_fp) {
		char tmp[50] = {0};

		ak_print_notice_ex("mux_write_no=%d\n", mux_write_no);
		sprintf(tmp, "./mux_video_%.4d.h264", mux_write_no);
		video_mux_fp = fopen(tmp, "w+");
		if(!video_mux_fp) {
			ak_print_error_ex("create mux video file error\n");
			return;
		}
	}
	
#if MUX_WRITE_VIDEO_EXINFO
	char tmp[4] = {0x5A, 0x5A, 0x5A, 0x5A};
	if(fwrite(tmp, 1, 4, video_mux_fp) < 0) {
		ak_print_error_ex("write video file err\n");
	}
	if(fwrite(&(param->m_ulTimeStamp), 1, 4, video_mux_fp) < 0) {
		ak_print_error_ex("write video file err\n");
	}
	if(fwrite(&(param->m_ulStreamLen), 1, 4, video_mux_fp) < 0) {
		ak_print_error_ex("write video file err\n");
	}
#endif

	if(fwrite(param->m_pStreamBuf, 1, param->m_ulStreamLen, video_mux_fp) < 0) {
		ak_print_error_ex("write video file err\n");
	}

#if MUX_WRITE_VIDEO_EXINFO	
	tmp[0] = 0x0A;
	if(fwrite(tmp, 1, 1, video_mux_fp) < 0) {
		ak_print_error_ex("write video file err\n");
	}
#endif	
}
#endif

static int mux_set_audio_input(T_MEDIALIB_MUX_OPEN_INPUT *open, 
					const struct mux_input *param)
{
	int ret = AK_SUCCESS;
	
	open->m_eAudioType = param->audio_type;
    open->m_nSampleRate = param->sample_rate;
    open->m_nChannels = AUDIO_CHANNEL_MONO;
    open->m_wBitsPerSample = 16;

    switch (param->audio_type) {
    case MEDIALIB_AUDIO_PCM:
        open->m_ulSamplesPerPack = ((param->sample_rate * 32) / 1000);
        open->m_ulAudioBitrate = (param->sample_rate * open->m_wBitsPerSample
            * open->m_ulSamplesPerPack);
        break;
    case MEDIALIB_AUDIO_G711:
        open->m_wFormatTag = 0x6;          
        open->m_nBlockAlign = 1;
        open->m_cbSize = 0;
        open->m_wBitsPerSample = 8;
        open->m_ulSamplesPerPack = ((param->sample_rate * param->frame_interval) 
        	/ 1000);
        open->m_ulAudioBitrate = (param->sample_rate * open->m_wBitsPerSample
            * open->m_ulSamplesPerPack);
        open->m_nAvgBytesPerSec = open->m_nSampleRate * open->m_nBlockAlign;
        break;
    case MEDIALIB_AUDIO_AAC:
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
    case MEDIALIB_AUDIO_ADPCM:
        open->m_wFormatTag = 0x11;
        open->m_wBitsPerSample = 4;
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
        
        open->m_ulSamplesPerPack = ((((open->m_nBlockAlign - 4) * 8) / 4) + 1);
        open->m_ulAudioBitrate = (open->m_nSampleRate
                    * open->m_nBlockAlign
                    / open->m_ulSamplesPerPack);
        open->m_nAvgBytesPerSec = open->m_ulAudioBitrate;
        open->m_nBlockAlign *= open->m_nChannels;
        open->m_cbSize = 2;
        open->m_pszData = (T_U8 *)&(open->m_ulSamplesPerPack);
        break;
	case MEDIALIB_AUDIO_AMR:
		open->m_ulAudioBitrate = 12200;
		open->m_ulSamplesPerPack = 160;
		break;
	default:
    	ret = AK_FAILED;
    	break;
    }

    return ret;
}

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
				FILE *record_fp, FILE *index_fp)
{
	if(NULL == mux_param){
        return NULL;
    }

	struct mux_handle *mux = (struct mux_handle *)calloc(
		1, sizeof(struct mux_handle));
    if(NULL == mux) {
        return NULL;            
    }
    
    mux->record_fp = record_fp;
    mux->index_fp = index_fp;   
    ak_thread_mutex_init(&(mux->mutex)); 

   	int ret = AK_FAILED;
	T_MEDIALIB_MUX_OPEN_INPUT open_input;
	
    memset(&open_input, 0, sizeof(T_MEDIALIB_MUX_OPEN_INPUT));
    open_input.m_MediaRecType = mux_param->media_rec_type;
    open_input.m_hMediaDest = (long)record_fp;
    open_input.m_bCaptureAudio = mux_param->capture_audio;
    open_input.m_bNeedSYN = AK_TRUE;
    open_input.m_bLocalMode = AK_TRUE;
    open_input.m_bIdxInMem = AK_FALSE;
    open_input.m_ulIndexMemSize = 0;
    open_input.m_hIndexFile = (long)index_fp;

    /* for syn */
    open_input.m_ulVFifoSize = 320*1024; //video fifo size
    open_input.m_ulAFifoSize = 100*1024; //audio fifo size
    open_input.m_ulTimeScale = 1000;     //time scale

    /* set video open info */
    open_input.m_eVideoType = mux_param->video_type;
    open_input.m_nWidth = mux_param->width;
    open_input.m_nHeight = mux_param->height;

	open_input.m_nFPS = mux_param->file_fps; 
    open_input.m_nKeyframeInterval = 0;
    
    /* set audio open info */
    if(mux_param->capture_audio
    	&& (AK_FAILED == mux_set_audio_input(&open_input, mux_param))) {
    	goto mux_open_error;
    }

    open_input.m_nChannels = AUDIO_CHANNEL_MONO;
    open_input.m_CBFunc.m_FunPrintf= (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
    open_input.m_CBFunc.m_FunMalloc= (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
    open_input.m_CBFunc.m_FunFree = (MEDIALIB_CALLBACK_FUN_FREE)free;
    open_input.m_CBFunc.m_FunRead = (MEDIALIB_CALLBACK_FUN_READ)mux_fs_read;
    open_input.m_CBFunc.m_FunSeek = (MEDIALIB_CALLBACK_FUN_SEEK)mux_fs_seek;
    open_input.m_CBFunc.m_FunTell = (MEDIALIB_CALLBACK_FUN_TELL)mux_fs_tell;
    open_input.m_CBFunc.m_FunWrite = (MEDIALIB_CALLBACK_FUN_WRITE)mux_fs_write;
    open_input.m_CBFunc.m_FunFileHandleExist = mux_fs_handle_exist;

	T_MEDIALIB_MUX_OPEN_OUTPUT open_output = {0};

    mux_fs_init(record_fp, index_fp);
    mux->midea_handle = MediaLib_Mux_Open(&open_input, &open_output);
    if (NULL == mux->midea_handle) {
		ak_print_normal_ex("open mux handle failed!\n");
        goto mux_open_error;
    }
    
    T_MEDIALIB_MUX_INFO MuxInfo = {0};
    
    if (!MediaLib_Mux_GetInfo(mux->midea_handle, &MuxInfo)) {
		ak_print_normal_ex("mux get info failed!\n");
        goto mux_open_error;
    }

    if (!MediaLib_Mux_Start(mux->midea_handle)) {
		ak_print_normal_ex("mux start failed!\n");
        goto mux_open_error;
    }

    mux->start_ts = mux_param->start_ts;
    ak_print_info_ex("mux->start_ts=%llu\n", mux->start_ts);
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

mux_open_error:
	if(AK_FAILED == ret) {
		ak_print_normal_ex("it fail to open the handle!\n");
	    if(NULL != mux) {
	    	mux_fs_flush(record_fp);
		    if(NULL != mux->midea_handle) {
		        MediaLib_Mux_Close(mux->midea_handle);
		        mux->midea_handle = NULL;
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
    if(NULL == mux_handle){
        return AK_FAILED;
    }

    int ret = AK_SUCCESS;
    T_MEDIALIB_MUX_PARAM param;
    T_eMEDIALIB_MUX_STATUS status;
    struct mux_handle *mux = (struct mux_handle *)mux_handle;
    
    param.m_pStreamBuf = audio->data;
    param.m_ulStreamLen = audio->len;
    param.m_ulTimeStamp = (audio->ts - mux->start_ts);
    param.m_bIsKeyframe = AK_TRUE;
	
#if MUX_WRITE_AUDIO_FILE
	audio_write_mux_file(mux_handle, &param);
#endif
    
	ak_thread_mutex_lock(&(mux->mutex));
    if(MediaLib_Mux_AddAudioData(mux->midea_handle, &param)) {
    	status = MediaLib_Mux_Handle(mux->midea_handle);
    	if((MEDIALIB_MUX_DOING != status) 
    		&& (MEDIALIB_MUX_WAITING != status)) {
    		ak_print_error("--- ERROR! Audio MUX handle, mux status=%d, "
    			"ts:%llu, sz:%u ---\n", status, audio->ts, audio->len);
    	} else {
    		mux->pre_audio_ts = audio->ts;
    	}
    } else {
    	ak_print_normal_ex("pre ts=%llu, cur ts=%llu\n",
        	mux->pre_audio_ts, audio->ts);
        ak_print_error("--- ERROR! Add Audio to MUX Failed! ts:%llu, sz:%u ---\n",
        	audio->ts, audio->len);
        ret = AK_FAILED;
    }
    
    status = MediaLib_Mux_GetStatus(mux->midea_handle);
	ak_thread_mutex_unlock(&(mux->mutex));

	if (ret) {
		ak_print_normal_ex("--- status=%d ---\n", status);
	}
    if ((MEDIALIB_MUX_SYSERR == status) 
    	|| (MEDIALIB_MUX_MEMFULL == status)) {
        ak_print_error("[%s]: failed to mux with err(%d)\n", __func__, status);
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

	param.m_pStreamBuf = video->data;
	param.m_ulStreamLen = video->len;
	param.m_ulTimeStamp = (video->ts - mux->start_ts);
	param.m_bIsKeyframe = video->frame_type;
		
#if MUX_WRITE_VIDEO_FILE
	video_write_mux_file(&param);
#endif
	
	ak_thread_mutex_lock(&(mux->mutex));
	if(MediaLib_Mux_AddVideoData(mux->midea_handle, &param)) {
		status = MediaLib_Mux_Handle(mux->midea_handle);
		if((MEDIALIB_MUX_DOING != status)
			&& (MEDIALIB_MUX_WAITING != status)) {
    		ak_print_error("--- ERROR! Video MUX handle, mux status=%d, "
    			"ts:%llu IF:%d, sz:%u ---\n", 
    			status, video->ts, video->frame_type, video->len);
    	} else {
    		mux->pre_video_ts = video->ts;
    	}
	} else {
		ak_print_normal_ex("pre ts=%llu, cur ts=%llu\n",
        	mux->pre_video_ts, video->ts);
		ak_print_error("--- ERROR! Add Video to MUX Failed! ts:%llu, IF:%d, sz:%u ---\n", 
			video->ts, video->frame_type, video->len);
		ret = AK_FAILED;
	}

	status = MediaLib_Mux_GetStatus(mux->midea_handle);
	ak_thread_mutex_unlock(&(mux->mutex));

	if (ret) {
		ak_print_normal_ex("--- status=%d ---\n", status);
	}
	if ((MEDIALIB_MUX_SYSERR == status)
		|| (MEDIALIB_MUX_MEMFULL == status)) {
        ak_print_error("[%s]: failed to mux with err(%d)\n", __func__, status);
		ret = status;
	}

	return ret;
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
	if(NULL != mux->midea_handle) {
		free(mux->midea_handle);
    	mux->midea_handle = NULL;
	}

	mux->start_ts = 0;
    ak_thread_mutex_destroy(&(mux->mutex));
    free(mux);
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
    if(NULL == mux_handle){
        return AK_FAILED;
    }

	struct mux_handle *mux = (struct mux_handle *)mux_handle;
	
	ak_thread_mutex_lock(&(mux->mutex));
	mux->start_ts = 0;
    mux_fs_flush(mux->record_fp);
    mux_fs_flush(mux->index_fp);
    
    if (!MediaLib_Mux_Stop(mux->midea_handle)) {
    	ak_print_normal("MediaLib_Mux_Stop FAILED\n");
    }
    if(!MediaLib_Mux_Close(mux->midea_handle)) {
    	ak_print_normal("MediaLib_Mux_Close FAILED\n");
    }
    ak_thread_mutex_unlock(&(mux->mutex));

    ak_thread_mutex_destroy(&(mux->mutex));
    free(mux);
    mux = NULL;

#if (MUX_WRITE_VIDEO_FILE)
	if (video_mux_flag) {
		video_mux_flag = AK_FALSE;
		fclose(video_mux_fp);
    	video_mux_fp = NULL;
	}
#endif
#if (MUX_WRITE_AUDIO_FILE)
	if (audio_mux_flag) {
		audio_mux_flag = AK_FALSE;
		fclose(audio_mux_fp);
    	audio_mux_fp = NULL;
	}
#endif

	return AK_SUCCESS;
}
