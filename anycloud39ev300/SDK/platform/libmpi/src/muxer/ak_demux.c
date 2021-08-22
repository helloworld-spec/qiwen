#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>	//for magic

#include "ak_common.h"
#include "ak_thread.h"

#include "media_demuxer_lib.h"
#include "demux_fs.h"

#include "ak_demux.h"

#define MPI_DEMUX			    "<mpi_demux>"

#define LEN_PATH_FILE           256
#define LEN_DATETIME            16

#define OFFSET_MONTH            4
#define OFFSET_DAY              6
#define LEN_YEAR                4
#define LEN_MONTH               2
#define LEN_DAY                 2

#define OFFSET_MIN              2
#define OFFSET_SEC              4
#define LEN_HOUR                2
#define LEN_MIN                 2
#define LEN_SEC                 2

#define TIMEZONE                28800 //8*3600

/* define demux handle structure */
struct demux_handle {
    FILE *fp;			//open record-file
    void *media_lib;	//demux lib handle
   	int video_type;		//video type
	int audio_type;		//audio type
	unsigned long long v_ts;	//video ts
	unsigned long long a_ts;	//audio ts
	unsigned long long start_uts;	/* video start unix timestamp(ms) */
	ak_mutex_t lock;
	int magic;	//判断是否野指针用
};

static const char demuxer_version[] = "libmpi_demuxer V1.0.02";

/* 用于判断指针是否野指针用 */
#define DEMUX_MAGIC_TYPE	'x'
#define DEMUX_MAGIC _IOR(DEMUX_MAGIC_TYPE, 10, int)

/*
 * demux_add_start_code - demux add start_code
 * data[IN]: pointer to demuxed data
 * len[IN]: indicate data length
 * notes:
 * 	H.264 stream format:
 *  	4 bytes(0x00 0x00 0x00 0x01) or 3 bytes(0x00 0x00 0x01) + video stream data
 * 	after demux in anyka:
 *  	4 bytes stream no. + 4bytes timestamp + 4bytes data len + video stream data
 */
static void demux_add_start_code(unsigned char *data, unsigned int len)
{
	/* step 1, check whether the frame has start_code */
	unsigned char *start_pos = data + 8;	/* skip seq and ts */
	const unsigned char start_code[4] = {0x00, 0x00, 0x00, 0x01};

	/* start code compare */
	if (memcmp(start_code, start_pos, 4) == 0) {
		return;
	}

	/* step 2, replay data len with start code */
	unsigned int cur_seek_len = 8;
	unsigned int fn_len = 0;	/* fn: fragment unit */
	do {
		fn_len = (start_pos[0]<<24) + (start_pos[1]<<16) + (start_pos[2]<<8) + start_pos[3];
		/* replace */
		memcpy(start_pos, start_code, 4);

		/* seek */
		start_pos += fn_len + 4;
		cur_seek_len = start_pos - data;
	} while ((cur_seek_len < len) && memcmp(start_code, start_pos, 4));
}

/*
 * demux_get_video_frame_type - demux get video frame type
 * data[IN]: pointer to demuxed data
 * video_type[IN]: indicate video type
 * frame_type[OUT]: pointer to buf storage frame type
 */
static void demux_get_video_frame_type(unsigned char *data, 
		int video_type, int *frame_type)
{
	/* NALU, skip seq num (4B), ts (4B), start code (4B) */
	unsigned char *nalu = data + 12;

	/* avi: H264, mp4: VIDEO_DRV_AVC1 */
	if (VIDEO_DRV_H264 == video_type || VIDEO_DRV_AVC1 == video_type) {
		T_U8 avc_header = nalu[0];
		// search sps
		if ((avc_header & 0x1F) == 7) {
			*frame_type = FRAME_TYPE_I;
		} else {
			*frame_type = FRAME_TYPE_P;
		}
	} else if (VIDEO_DRV_H265 == video_type || VIDEO_DRV_HVC1 == video_type) {
		T_U16 hevc_header = ((T_U16)nalu[0] << 8) | nalu[1];
		// search vps
		if (((hevc_header>>9) & 0x3f) == 32) {
			*frame_type = FRAME_TYPE_I;
		} else {
			*frame_type = FRAME_TYPE_P;
		}
	}
}

/*
 * demux_get_video_block - demux video block
 * media[IN]: demux lib handle
 * video[IN]: pointer to video stream node which gonna be demux
 * return: demux data ts
 * notes:
 * H.264 stream format:
 *  4 bytes(0x00 0x00 0x00 0x01) or 3 bytes(0x00 0x00 0x01) + video stream data
 * after demux in anyka:
 *  4 bytes stream no. + 4 bytes timestamp + 4 bytes data len + video stream data
 */
static unsigned long long demux_get_video_block(void *media,
		struct demux_stream *video)
{
	unsigned long long video_ts = 0;

	if (!MediaLib_Dmx_CheckVideoEnd(media)) {
		/* get frame size */
		int streamlen = MediaLib_Dmx_GetVideoFrameSize(media);
		if (streamlen != 0) {
			/* streamlen include 12B header */
			video->len = streamlen;
			video->data = calloc(1, video->len + 4);
			if (NULL == video->data) {
				ak_print_error_ex(MPI_DEMUX "it fail to calloc\n");
				video->len = 0;
				return video_ts;
			}
			/* demux, get frame */
			if (MediaLib_Dmx_GetVideoFrame(media, (unsigned char *)video->data,
					(unsigned long *)&(video->len)) == AK_FALSE) {
				ak_print_error_ex("demux get video frame fail\n");	
				video->len = 0;
				return video_ts;
			}
			/* get ts */
			video->ts = *(unsigned long *)(video->data + 4);
			video_ts = video->ts;

			/* 
			 * on mp4, our mux will change 'start code 00 00 00 01' to 'data len',
			 * so, here we should change it back, make sure 'start code' is always
			 * on work.
			 */
			demux_add_start_code(video->data, video->len);
		}
	} else {
		video->len = 0;
	}

	return video_ts;
}

/*
 * demux_get_audio_block - demux audio data
 * media[IN]: demux lib handle
 * audio[OUT]: store demuxed data
 * return: data timestamp
 */
static unsigned long long demux_get_audio_block(void *media, 
		struct demux_stream *audio)
{
	unsigned long long audio_ts = 0;

	if (!MediaLib_Dmx_CheckAudioEnd(media)) {
		/* get frame size */
		int streamlen = MediaLib_Dmx_GetAudioDataSize(media);
		if (streamlen != 0) {
			audio->len = streamlen;
			audio->data = calloc(1, audio->len + 4);
			if (NULL == audio->data) {
				ak_print_error_ex(MPI_DEMUX "it fail to calloc\n");
				return audio_ts;
			}
			/* get frame ts */
			if (MediaLib_Dmx_GetAudioPts(media, (T_U32 *)&(audio->ts)))
				audio_ts = audio->ts;
			/* demux, get frame data */
			MediaLib_Dmx_GetAudioData(media, (unsigned char *)(audio->data),
				(unsigned long)(audio->len));
		}
	} else {
		audio->len = 0;
	}

	return audio_ts;
}

/*
 * demux_video - demux video
 * demux[IN]: demux handle,  return by ak_demux_open()
 * stream[OUT]: store demuxed data
 * video_type[IN]: current video type: h264/h265
 * return: data len
 */
static unsigned int demux_video(struct demux_handle *demux,
		struct demux_stream *stream, int video_type)
{
	void *media = demux->media_lib;
	demux->v_ts = demux_get_video_block(media, stream);
	if (stream->len != 0) {
		demux_get_video_frame_type(stream->data, video_type,
			   	(int *)&stream->frame_type);
	} 

	return stream->len;
}

static unsigned long long ak_get_uts(const char *pc_media_file)
{
	char ac_media_file[LEN_PATH_FILE], ac_date[LEN_DATETIME], ac_time[LEN_DATETIME], ac_tmp[LEN_DATETIME] ;
	int i;
	struct tm tm_datetime;
	snprintf(ac_media_file, LEN_PATH_FILE, "%s", pc_media_file);      //delete path if exist
	ak_print_warning_ex("path-----%s\n", ac_media_file);
	
	for( i = strlen( ac_media_file ) ; i >= 0  ; i -- ) {
		if( ac_media_file[ i ] == '/' ) {
			memmove( ac_media_file , ac_media_file + i + 1 , strlen( ac_media_file ) - i - 1 ) ;
			ac_media_file[ strlen( ac_media_file ) - i - 1 ] = 0x0 ;
			break;
		}
	}
	sscanf( ac_media_file , "%*[^0-9]%[0-9]%*[-]%[0-9]" , ac_date , ac_time );
	if ( ( strlen( ac_date ) > 0 ) && ( strlen( ac_time ) > 0 ) ) {
		memset( &tm_datetime, 0 , sizeof( struct tm ) ) ;

		memset( ac_tmp , 0 , LEN_DATETIME );
		memcpy( ac_tmp , ac_date, LEN_YEAR );
		tm_datetime.tm_year = atoi( ac_tmp ) - 1900 ;

		memset( ac_tmp , 0 , LEN_DATETIME );
		memcpy( ac_tmp , ac_date + OFFSET_MONTH , LEN_MONTH );
		tm_datetime.tm_mon = atoi( ac_tmp ) - 1 ;

		memset( ac_tmp , 0 , LEN_DATETIME );
		memcpy( ac_tmp , ac_date + OFFSET_DAY , LEN_DAY );
		tm_datetime.tm_mday = atoi( ac_tmp ) ;

		memset( ac_tmp , 0 , LEN_DATETIME );
		memcpy( ac_tmp , ac_time, LEN_HOUR );
		tm_datetime.tm_hour = atoi( ac_tmp ) ;

		memset( ac_tmp , 0 , LEN_DATETIME );
		memcpy( ac_tmp , ac_time + OFFSET_MIN , LEN_MIN );
		tm_datetime.tm_min = atoi( ac_tmp ) ;

		memset( ac_tmp , 0 , LEN_DATETIME );
		memcpy( ac_tmp , ac_time + OFFSET_SEC , LEN_SEC );
		tm_datetime.tm_sec = atoi( ac_tmp ) ;

		ak_print_notice_ex("year=%d,mon=%d,day=%d,hour=%d,min=%dsec=%d\n", 
						tm_datetime.tm_year, tm_datetime.tm_mon, tm_datetime.tm_mday,
						tm_datetime.tm_hour, tm_datetime.tm_min, tm_datetime.tm_sec);
		return mktime( &tm_datetime );
	}
	else {
		return 0;
	}
}

/*
 * demux_audio - demux audio 
 * demux[IN]: demux handle,  return by ak_demux_open()
 * stream[OUT]: store demuxed data
 * return: data len
 */
static unsigned int demux_audio(struct demux_handle *demux,
		struct demux_stream *stream)
{
	void *media = demux->media_lib;
	demux->a_ts = demux_get_audio_block(media, stream);
	if (stream->len != 0) {
		/* why 4k -> 0 ? */
		if (stream->len == 4096)
			stream->len = 0;
	}

	return stream->len;
}

/*
 * open_demux_lib - open demux
 * file_path[IN]: demux file path
 * out_fp[OUT]: store output fp
 * return: media_lib handle on success, NULL on failed
 */
static void* open_demux_lib(const char *file_path, FILE **out_fp)
{
	FILE *fp = fopen(file_path, "r");
    if (NULL == fp) {
		ak_print_error(MPI_DEMUX "open %s failed, %s\n", file_path, strerror(errno));
    	return NULL;
    }

	T_MEDIALIB_DMX_OPEN_INPUT open_input;
    memset(&open_input, 0, sizeof(T_MEDIALIB_DMX_OPEN_INPUT));

    /* register demux callback */
    open_input.m_hMediaSource = (long)fp;
    open_input.m_CBFunc.m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;//ak_print_null;
    open_input.m_CBFunc.m_FunRead = (MEDIALIB_CALLBACK_FUN_READ)demux_fs_read;
    open_input.m_CBFunc.m_FunWrite = (MEDIALIB_CALLBACK_FUN_WRITE)demux_fs_write;
    open_input.m_CBFunc.m_FunSeek = (MEDIALIB_CALLBACK_FUN_SEEK)demux_fs_seek;
    open_input.m_CBFunc.m_FunTell = (MEDIALIB_CALLBACK_FUN_TELL)demux_fs_tell;
    open_input.m_CBFunc.m_FunMalloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
    open_input.m_CBFunc.m_FunFree = (MEDIALIB_CALLBACK_FUN_FREE)free;
    open_input.m_CBFunc.m_FunFileHandleExist = demux_fs_exist;

	open_input.strVersion = MEDIA_LIB_VERSION;
	
	T_MEDIALIB_DMX_OPEN_OUTPUT open_output = {0};

    /* open the dumxer */
    void *media = MediaLib_Dmx_Open(&open_input, &open_output);
    if (NULL == media) {
    	fclose(fp);
		fp = NULL;
    }
    *out_fp = fp;

	return media;
}

/* get current version string pointer */
const char* ak_demuxer_get_version(void)
{
	return demuxer_version;
}
/**
 * ak_demux_open - open dumxer lib
 * @file_path[IN]: media file path, including full name
 * @start[IN]: start flag
 * return: demux handle, NULL failed
 */
void* ak_demux_open(const char *file_path, int start)
{
 	if (NULL == file_path) {
 		return NULL;
 	}

	ak_print_normal_ex(MPI_DEMUX "replay path=%s\n", file_path);
	/* allocate demux handle */
    struct demux_handle *demux = (struct demux_handle *)calloc(1,
			sizeof(struct demux_handle));
    if(NULL == demux) {
        return NULL;
    }

	ak_thread_mutex_init(&demux->lock, NULL);
	demux->magic = DEMUX_MAGIC;
	//ak_print_normal_ex("magic: %d\n", demux->magic);

    int ret = AK_FAILED;
    FILE *fp = NULL;
	/* open demux lib and record file */
    void *media = open_demux_lib(file_path, &fp);
	if(NULL == media) {
		goto demux_open_end;
	}

	T_MEDIALIB_DMX_INFO dmx_info = {0};

    /* get media info, VDType is to discriminate mp4 and avi */
    if (MediaLib_Dmx_GetInfo(media, &dmx_info)) {
		ak_print_normal_ex(MPI_DEMUX "media type(ATC): %d, video type: %d audio type:%d\n",
				dmx_info.m_MediaType, dmx_info.m_VideoDrvType, dmx_info.m_AudioType);
		demux->video_type = dmx_info.m_VideoDrvType;
		demux->audio_type = dmx_info.m_AudioType;
	} else {
		ak_print_error_ex(MPI_DEMUX "dmx get info error\n");
        goto demux_open_end;
	}

	demux->start_uts = ak_get_uts( file_path ) * 1000 - TIMEZONE * 1000 ;
	ak_print_warning_ex("demux->start_uts= %lld\n", demux->start_uts);

    /* release the info memory */
    MediaLib_Dmx_ReleaseInfoMem(media);

	/* get first video size, print info and allocate buffer to store demux data */
    unsigned int buf_len = MediaLib_Dmx_GetFirstVideoSize(media);
    ak_print_normal(MPI_DEMUX "first video size:%u\n", buf_len);
	if (buf_len == 0) {
		ak_print_error_ex(MPI_DEMUX "MediaLib_Dmx_GetFirstVideo error\n");
		goto demux_open_end;
	}

	/* to get first video data */
    unsigned char *buf = (unsigned char *)calloc(1, buf_len);
    if (!MediaLib_Dmx_GetFirstVideo(media, buf, (unsigned long *)&buf_len)) {
    	ak_print_error_ex(MPI_DEMUX "MediaLib_Dmx_GetFirstVideo error\n");
		free(buf);
		goto demux_open_end;
	}

    free(buf);
    demux->fp = fp;
    demux->media_lib = media;
    MediaLib_Dmx_Start(media, start);
    ret = AK_SUCCESS;
	ak_print_normal_ex("demux handle: %p, lib: %p\n", demux, demux->media_lib);

demux_open_end:
	if (AK_FAILED == ret) {
		if (NULL != media) {
    		MediaLib_Dmx_Close(media);
    		media = NULL;
    	}

		if (NULL != demux) {
			free(demux);
			demux = NULL;
		}
	}
	return demux;
}

/**
 * ak_demux_get_enc_type - get data encode type after open demux
 * @demux_handle[IN]: opened demux handle
 * @video_type[OUT]: demux video encode type
 * @audio_type[OUT]: demux audio encode type
 * return: 0 success, -1 failed
 */
int ak_demux_get_enc_type(void *demux_handle,
		int *video_type, int *audio_type)
{
	if(!demux_handle || !video_type || !audio_type)
		return AK_FAILED;

	struct demux_handle *demux = (struct demux_handle *)demux_handle;
	int ret = AK_SUCCESS;

	CHECK_MAGIC_RETURN_VAL(demux, DEMUX_MAGIC, AK_FAILED);

    /* use platform type replace of ATC type */
	switch(demux->video_type){
	case VIDEO_DRV_H264:
	case VIDEO_DRV_AVC1:
		*video_type = VIDEO_DEMUX_H264;
		break;
	case VIDEO_DRV_HVC1:
	case VIDEO_DRV_H265:
		*video_type = VIDEO_DEMUX_H265;
		break;
	default:
		ret = AK_FAILED;
		ak_print_error_ex("faield.not support video type:%d\n",*video_type);
		break;
	}
	switch(demux->audio_type){
	case _SD_MEDIA_TYPE_PCM_ALAW:
		*audio_type = AK_AUDIO_TYPE_PCM_ALAW;
		break;
	case _SD_MEDIA_TYPE_AMR:
		*audio_type = AK_AUDIO_TYPE_AMR;
		break;
	default:
		ret = AK_FAILED;
		ak_print_error_ex("faield.not support audio type:%d\n",*audio_type);
		break;
	}

	return ret;
}

/**
 * ak_demux_get_total_time: get media file's total time
 * @file_path[IN]: media file path, including full name
 * return: total time in ms, -1 failed
 */
int ak_demux_get_total_time(const char *file_path)
{
 	FILE *fp = NULL;
    void *media = open_demux_lib(file_path, &fp);
	if(NULL == media) {
		ak_print_error_ex(MPI_DEMUX "media=NULL\n");
		return AK_FAILED;
	}

	T_MEDIALIB_DMX_INFO dmx_info = {0};
	if (!MediaLib_Dmx_GetInfo(media, &dmx_info)) {
		ak_print_error_ex(MPI_DEMUX "dmx get info error\n");
		MediaLib_Dmx_Close(media);
		return AK_FAILED;
	}
	MediaLib_Dmx_ReleaseInfoMem(media);

	if (fp) {
		fclose(fp);
		fp = NULL;
	}

    MediaLib_Dmx_Stop(media);
    MediaLib_Dmx_Close(media);

    return dmx_info.m_ulTotalTime_ms;
}

/*
 * ak_demux_get_data - get data and which audio and video ts has 
 *                    been synchronized.
 * @demux_handle[IN]: opened demux handle
 * @type[IN]: demux type
 * return: the stream packet after demux, NULL failed
 */
struct demux_stream* ak_demux_get_data(void *demux_handle,
						enum demux_data_type *type)
{
	/* argument check */
	if (!demux_handle || !type) {
		ak_print_error_ex("arg error\n");
		return NULL;
	}

	/* get demux handle */
	struct demux_handle *demux = (struct demux_handle *)demux_handle;
	void *media = demux->media_lib;
	int ret = AK_FAILED;

	CHECK_MAGIC_RETURN_VAL(demux, DEMUX_MAGIC, NULL);

    struct demux_stream *stream = (struct demux_stream *)calloc(1,
			sizeof(struct demux_stream));
    if (!stream) {
        ak_print_error_ex(MPI_DEMUX "it fail to calloc\n");
        return NULL;
    }
	stream->start_uts = demux->start_uts;

	/* decide to demux video or audio frame */
	ak_thread_mutex_lock(&demux->lock);
	if (demux->a_ts >= demux->v_ts) { 			/* demux video */
		if (demux_video(demux, stream, demux->video_type) > 0) {
			*type = T_eMEDIALIB_BLKTYPE_VIDEO;
		} else {
			/* demux video fail, demux audio */
			if (demux_audio(demux, stream) > 0)
				*type = T_eMEDIALIB_BLKTYPE_AUDIO;
			else 	/* demux video and audio fail, may demux to end of file */
				goto demux_get_end;
		}
	} else {									/* demux audio */
		if (demux_audio(demux, stream) > 0)
			*type = T_eMEDIALIB_BLKTYPE_AUDIO;
		else {
			/* demux audio to the end, demux video */
			if (demux_video(demux, stream, demux->video_type) > 0)
				*type = T_eMEDIALIB_BLKTYPE_VIDEO;
			else	/* demux video and audio fail, may demux to end of file */
				goto demux_get_end;
		}
	}

	/* demux status check */
	T_eMEDIALIB_DMX_STATUS status = MediaLib_Dmx_GetStatus(media);
	if ((MEDIALIB_DMX_END == status) || (MEDIALIB_DMX_ERR == status)) {
        ak_print_error_ex(MPI_DEMUX "it fail to get media status!\n");
        goto demux_get_end;
	}
  	ret = AK_SUCCESS;

demux_get_end:
	if (AK_FAILED == ret) {
		if (NULL != stream) {
			if (NULL != stream->data) {
		        free(stream->data);
				stream->data = NULL;
		    }
			free(stream);
			stream = NULL;
		}
	}
	ak_thread_mutex_unlock(&demux->lock);

	return stream;
}

/**
 * ak_demux_free_data: free the demux resource
 * @packet[IN]: the stream packet after demux
 * return: total time in ms
 */
void ak_demux_free_data(struct demux_stream *stream)
{
    if (NULL != stream) {
    	if (NULL != stream->data) {
    		free(stream->data);
    		stream->data = NULL;
    	}
        free(stream);
        stream = NULL;
    }
}

/**
 * ak_demux_close: close dumxer lib
 * @demux_handle[IN]: opened demux handle
 * return: void
 */
void ak_demux_close(void *demux_handle)
{
    struct demux_handle *demux = (struct demux_handle *)demux_handle;
    if (NULL != demux) {
		CHECK_MAGIC_RETURN(demux, DEMUX_MAGIC);

		ak_thread_mutex_lock(&demux->lock);
		demux->magic = -1;
    	if(NULL != demux->fp) {
			fclose(demux->fp);
			demux->fp = NULL;
	    }

		if (NULL != demux->media_lib) {
			MediaLib_Dmx_Stop(demux->media_lib);
    		MediaLib_Dmx_Close(demux->media_lib);
    		demux->media_lib = NULL;
		}
		ak_thread_mutex_unlock(&demux->lock);
		ak_thread_mutex_destroy(&demux->lock);

	    free(demux);
	    demux = NULL;
    }
}

/**
 * ak_demux_get_spec_keyframe: get speciffically key frame
 * @demux_handle[IN]: return by demux_open()
 * @ms[IN]: offset time base on file-begin
 * @direct[IN]: setting the direction
 * return: on success, ts with frameget closely keyframe, else return -1; 
 */
int ak_demux_seek_to_keyframe(void *demux_handle, unsigned long ms, 
		enum demux_direction direct)
{
	if(!demux_handle) {
		ak_print_error_ex("invalid handle\n");
		return -1;
	}
    struct demux_handle *demux = (struct demux_handle *)demux_handle;
	T_eMEDIALIB_DMX_SETDIRECTIONS seek_direction = direct; 

	CHECK_MAGIC_RETURN_VAL(demux, DEMUX_MAGIC, AK_FAILED);

	int ret = MediaLib_Dmx_SetPosition(demux->media_lib, ms, seek_direction);
	if (ret < 0) {
		ak_print_normal_ex("set position according %lu fail\n", ms);	
		return -1;
	}
	/* after setposition, need to call 'Start' with return val of setposition */
    MediaLib_Dmx_Start(demux->media_lib, ret);

	return ret;
}

/**
 * ak_demux_get_spec_keyframe: get speciffically key frame
 * @demux_handle[IN]: return by demux_open()
 * @info[OUT]: store media info
 * return: 0 on succeess, -1 failed
 */
int ak_demux_get_media_info(void *demux_handle, struct ak_demux_media_info *info)
{
	if(!demux_handle) {
		ak_print_error_ex("invalid handle\n");
		return -1;
	}
	struct demux_handle *demux = (struct demux_handle *)demux_handle;
	void *media = demux->media_lib;

	CHECK_MAGIC_RETURN_VAL(demux, DEMUX_MAGIC, AK_FAILED);

	if (!media) {
		ak_print_error_ex(MPI_DEMUX "error demux handle\n");
		return -1;
	}

	ak_thread_mutex_lock(&demux->lock);

	T_MEDIALIB_DMX_INFO dmx_info = {0};
	if (!MediaLib_Dmx_GetInfo(media, &dmx_info)) {
		ak_print_error_ex(MPI_DEMUX "dmx get info error\n");
		ak_thread_mutex_unlock(&demux->lock);
		return -1;
	}
	info->fps = dmx_info.m_uFPS;
	info->sample = dmx_info.m_nSamplesPerSec;
	info->totaltime_ms = dmx_info.m_ulTotalTime_ms;

	switch (dmx_info.m_AudioType) {
		case _SD_MEDIA_TYPE_MP3:
			info->audio_type = AUDIO_DEMUX_MP3;
			break;
		case _SD_MEDIA_TYPE_AMR:
			info->audio_type = AUDIO_DEMUX_AMR;
			break;
		case _SD_MEDIA_TYPE_AAC:
			info->audio_type = AUDIO_DEMUX_AAC;
			break;
		case _SD_MEDIA_TYPE_WMA:
			info->audio_type = AUDIO_DEMUX_WMA;
			break;
		case _SD_MEDIA_TYPE_PCM:
			info->audio_type = AUDIO_DEMUX_PCM;
			break;
		case _SD_MEDIA_TYPE_PCM_ALAW:
			info->audio_type = AUDIO_DEMUX_PCM_ALAW;
			break;
		case _SD_MEDIA_TYPE_PCM_ULAW:
			info->audio_type = AUDIO_DEMUX_PCM_ULAW;
			break;
		default:
			info->audio_type = AUDIO_DEMUX_UNKNOWN;
			break;
	}
	MediaLib_Dmx_ReleaseInfoMem(media);

	ak_thread_mutex_unlock(&demux->lock);

	return 0;
}


struct demux_stream* ak_demux_get_video(void *demux_handle)
{
	/* argument check */
	if (!demux_handle) {
		ak_print_error_ex("demux handle is NULL\n");
		return NULL;
	}
	/* get demux handle */
	struct demux_handle *demux = (struct demux_handle *)demux_handle;
	void *media = demux->media_lib;
	int ret = AK_FAILED;

	CHECK_MAGIC_RETURN_VAL(demux, DEMUX_MAGIC, NULL);

    struct demux_stream *stream = (struct demux_stream *)calloc(1,
			sizeof(struct demux_stream));
    if (!stream) {
        ak_print_error_ex(MPI_DEMUX "it fail to malloc\n");
        return NULL;
    }

	ak_thread_mutex_lock(&demux->lock);
	if (demux_video(demux, stream, demux->video_type) <= 0) {
		goto demux_get_end;
	}

	/* demux status check */
	T_eMEDIALIB_DMX_STATUS status = MediaLib_Dmx_GetStatus(media);
	if ((MEDIALIB_DMX_END == status) || (MEDIALIB_DMX_ERR == status)) {
        ak_print_error_ex(MPI_DEMUX "it fail to get media status!\n");
        goto demux_get_end;
	}
  	ret = AK_SUCCESS;

demux_get_end:
	if (AK_FAILED == ret) {
		if (NULL != stream) {
			if (NULL != stream->data) {
		        free(stream->data);
				stream->data = NULL;
		    }
			free(stream);
			stream = NULL;
		}
	}
	ak_thread_mutex_unlock(&demux->lock);

	return stream;
}

struct demux_stream* ak_demux_get_audio(void *demux_handle)
{
	/* argument check */
	if (!demux_handle) {
		ak_print_error_ex("demux handle is NULL\n");
		return NULL;
	}

	/* get demux handle */
	struct demux_handle *demux = (struct demux_handle *)demux_handle;
	void *media = demux->media_lib;
	int ret = AK_FAILED;

	CHECK_MAGIC_RETURN_VAL(demux, DEMUX_MAGIC, NULL);

    struct demux_stream *stream = (struct demux_stream *)calloc(1,
			sizeof(struct demux_stream));
    if (!stream) {
        ak_print_error_ex(MPI_DEMUX "it fail to malloc\n");
        return NULL;
    }

	ak_thread_mutex_lock(&demux->lock);
	if (demux_audio(demux, stream) <= 0) {
		goto demux_get_end;
	}

	/* demux status check */
	T_eMEDIALIB_DMX_STATUS status = MediaLib_Dmx_GetStatus(media);
	if ((MEDIALIB_DMX_END == status) || (MEDIALIB_DMX_ERR == status)) {
        ak_print_error_ex(MPI_DEMUX "it fail to get media status!\n");
        goto demux_get_end;
	}
  	ret = AK_SUCCESS;

demux_get_end:
	if (AK_FAILED == ret) {
		if (NULL != stream) {
			if (NULL != stream->data) {
		        free(stream->data);
				stream->data = NULL;
		    }
			free(stream);
			stream = NULL;
		}
	}
	ak_thread_mutex_unlock(&demux->lock);

	return stream;
}

