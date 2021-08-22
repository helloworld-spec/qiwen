#include <errno.h>
#include <string.h>

#include "media_demuxer_lib.h"
#include "video_stream_lib.h"
#include "demux_fs.h"

#include "ak_global.h"
#include "ak_common.h"
#include "ak_demux.h"

#ifdef AK_RTOS
#include "kernel.h"
#endif

struct demux_handle {
    FILE *fp;
    void *media_lib;  
   	int video_type;
};

static void demux_video_block(struct demux_handle *demux, 
				struct video_stream *video)
{
    unsigned char *data = video->data;
	unsigned char tmp_buf[64] = {0};
	unsigned char* frame_head = NULL;
	unsigned char* p = NULL;
	unsigned char* q = NULL;
	unsigned long m = 0;
	unsigned long n = 0;
    
	MediaLib_Dmx_GetVideoFrame(demux->media_lib, 
		(unsigned char*)(video->data), 
		(unsigned long *)&(video->len));
    video->ts = *(unsigned long *)(video->data + 4);
	if ((data[12]&0x1F) == 7) {
		if (VIDEO_DRV_H264 != demux->video_type) {
			frame_head = tmp_buf;
			memcpy(tmp_buf, (data+8), 64);

			tmp_buf[0] &= 0x00;
			tmp_buf[1] &= 0x00;
			tmp_buf[2] &= 0x00;
			m = tmp_buf[3];
			tmp_buf[3] = (tmp_buf[3]&0x00)|0x01;
			
			p = tmp_buf+m+4;
			p[0] &= 0x00;
			p[1] &= 0x00;
			p[2] &= 0x00;
			n = p[3];
			p[3] = (p[3]&0x00)|0x01;

			q = p+n+4;
			q[0] &= 0x00;
			q[1] &= 0x00;
			q[2] &= 0x00;
			q[3] = (q[3]&0x00)|0x01;

			memcpy((data+8), frame_head, 64);
		}
        video->frame_type = FRAME_TYPE_I;
	} else {
		if (VIDEO_DRV_H264 != demux->video_type) {
			data[8] &= 0x00;
			data[9] &= 0x00;
			data[10] &= 0x00;
			data[11] = ((data[11]&0x00) | 0x01);
		}
        video->frame_type = FRAME_TYPE_P;
	}
}

static void* open_demux_lib(const char *file_path, FILE **out_fp)
{
	FILE *fp = fopen(file_path, "r");
    if(NULL == fp) {
		ak_print_error("open %s failed, %s\n", file_path, strerror(errno));
    	return NULL;
    }

	T_MEDIALIB_DMX_OPEN_INPUT open_input;
    memset(&open_input, 0, sizeof(T_MEDIALIB_DMX_OPEN_INPUT));
    
    open_input.m_hMediaSource = (long)fp;
    open_input.m_CBFunc.m_FunPrintf = (MEDIALIB_CALLBACK_FUN_PRINTF)ak_print_null;
    open_input.m_CBFunc.m_FunRead = (MEDIALIB_CALLBACK_FUN_READ)demux_fs_read;
    open_input.m_CBFunc.m_FunWrite = (MEDIALIB_CALLBACK_FUN_WRITE)demux_fs_write;
    open_input.m_CBFunc.m_FunSeek = (MEDIALIB_CALLBACK_FUN_SEEK)demux_fs_seek;
    open_input.m_CBFunc.m_FunTell = (MEDIALIB_CALLBACK_FUN_TELL)demux_fs_tell;
    open_input.m_CBFunc.m_FunMalloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
    open_input.m_CBFunc.m_FunFree = (MEDIALIB_CALLBACK_FUN_FREE)free;
    open_input.m_CBFunc.m_FunFileHandleExist = demux_fs_exist;

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

/**
 * ak_demux_open: open dumxer lib
 * @file_path[IN]: media file path, including full name
 * @start[IN]: start flag
 * return: demux handle, NULL failed
 */
void* ak_demux_open(const char *file_path, int start)
{
 	if(NULL == file_path) {
 		return NULL;
 	}

    struct demux_handle *demux = (struct demux_handle *)calloc(
    					1, sizeof(struct demux_handle));
    if(NULL == demux) {
        return NULL;
    }

    int ret = AK_FAILED;
    FILE *fp = NULL;
    void *media = open_demux_lib(file_path, &fp);
	if(NULL == media) {
		goto demux_open_end;
	}

	T_MEDIALIB_DMX_INFO dmx_info = {0};
    
    /* get media info, VDType is to discriminate mp4 and avi */
    if (MediaLib_Dmx_GetInfo(media, &dmx_info)) {
		ak_print_normal_ex("media type: %d, video type: %ld\n", 
				dmx_info.m_MediaType, dmx_info.m_VideoDrvType);
		demux->video_type = dmx_info.m_VideoDrvType;
	} else {
		ak_print_error_ex("dmx get info error\n");
        goto demux_open_end;
	}
	
    /* release the info memory */
    MediaLib_Dmx_ReleaseInfoMem(media);
    
    unsigned int buf_len = MediaLib_Dmx_GetFirstVideoSize(media);
    ak_print_normal("first video size:%u\n", buf_len);	
    unsigned char *buf = (unsigned char *)calloc(1, buf_len);

    if(!MediaLib_Dmx_GetFirstVideo(media, buf, (unsigned long *)&buf_len)) {
    	ak_print_error_ex("MediaLib_Dmx_GetFirstVideo error\n");	
    }

    free(buf);
    demux->fp = fp;
    demux->media_lib = media;
    MediaLib_Dmx_Start(media, start);
    ret = AK_SUCCESS;

demux_open_end:
	if(AK_FAILED == ret) {
		if (NULL != media) {  
    		MediaLib_Dmx_Close(media);
    		media = NULL;
    	}
		
		if(NULL != demux) {
			free(demux);
			demux = NULL;
		}
	}

	return demux;
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
		ak_print_error_ex("media=NULL\n");
		return AK_FAILED;
	}

	T_MEDIALIB_DMX_INFO dmx_info = {0};
	if (!MediaLib_Dmx_GetInfo(media, &dmx_info)) {
		ak_print_error_ex("dmx get info error\n");
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

/**
 * ak_demux_get_data: get data after demux
 * @demux_handle[IN]: opened demux handle
 * @type[IN]: demux type
 * return: the stream packet after demux
 */
struct video_stream* ak_demux_get_data(void *demux_handle, int *type)
{
	if(NULL == demux_handle) {
		return NULL;
	}

	struct demux_handle *demux = (struct demux_handle *)demux_handle;
	void *media = demux->media_lib;
	
    struct video_stream *video = (struct video_stream *)calloc(1,
    	sizeof(struct video_stream));
    if(NULL == video) {
        ak_print_error_ex("it fail to calloc\n");
        return NULL;
    }

	int ret = AK_FAILED;
    T_MEDIALIB_DMX_BLKINFO block_info = {0};
    
	if (!MediaLib_Dmx_GetNextBlockInfo(media, &block_info)) {
        ak_print_error_ex("it fail to get block info\n");
		goto demux_get_end;
	}

	video->len = block_info.m_ulBlkLen;
    video->data= calloc(1, video->len + 4);
    if(NULL == video->data) {
        ak_print_error_ex("it fail to calloc\n");
        goto demux_get_end;
    }

    *type = block_info.m_eBlkType;
	switch (block_info.m_eBlkType) {
	case T_eMEDIALIB_BLKTYPE_VIDEO:
		if (0 != video->len) {
			demux_video_block(demux, video);
		}
        break;
    case T_eMEDIALIB_BLKTYPE_AUDIO:
        if (video->len != 0) {
			MediaLib_Dmx_GetAudioPts(media, (T_U32 *)&(video->ts));
        	MediaLib_Dmx_GetAudioData(media, 
        		(unsigned char *)(video->data), 
        		(unsigned long)(video->len));
        }

        if(video->len == 4096) {
        	video->len = 0;
        }
        break;
    default:
        ak_print_error_ex("unknow type\n");
        *type = T_eMEDIALIB_BLKTYPE_UNKNOWN;
        break;
	}

	T_eMEDIALIB_DMX_STATUS status = MediaLib_Dmx_GetStatus(media);
	if ((MEDIALIB_DMX_END == status) || (MEDIALIB_DMX_ERR == status)) {
        ak_print_error_ex("it fail to get media status!\n");
        goto demux_get_end;
	}	
  	ret = AK_SUCCESS;

demux_get_end:
	if(AK_FAILED == ret) {
		if(NULL != video) {
			if(NULL != video->data) {
		        free(video->data);
				video->data = NULL;
		    }
		    
			free(video);
			video = NULL;
		}
	}
	
	return video;
}

/**
 * ak_demux_free_data: free the demux resource
 * @packet[IN]: the stream packet after demux
 * return: total time in ms
 */
void ak_demux_free_data(struct video_stream *video)
{
    if(NULL != video)
    {
    	if(NULL != video->data) {
    		free(video->data);
    		video->data = NULL;
    	}
        
        free(video);
        video = NULL;
    }
}

void ak_demux_close(void *demux_handle)
{
    struct demux_handle *demux = (struct demux_handle *)demux_handle;
    if(NULL != demux) {
    	if(NULL != demux->fp) {
			fclose(demux->fp);
			demux->fp = NULL;
	    }

		if(NULL != demux->media_lib) {
			MediaLib_Dmx_Stop(demux->media_lib);
    		MediaLib_Dmx_Close(demux->media_lib);
    		demux->media_lib = NULL;
		}
		
	    free(demux);
	    demux = NULL;
    }
}
