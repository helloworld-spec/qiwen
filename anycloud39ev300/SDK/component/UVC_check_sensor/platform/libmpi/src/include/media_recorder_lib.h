/**
 * @file media_recorder_lib.h
 * @brief This file provides 3GP/AVI recording functions
 *
 * Copyright (C) 2011 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Su Dan, Zeng Yong
 * @date 2008-4-9
 * @update date 2011-6-13
 * @version 0.2.0
 * @version ever green group version: x.x
 * @note
	The following is an example to use recording APIs
   @code
T_VOID record_media(char* filaname);
T_VOID main(int argc, char* argv[])
{
	T_MEDIALIB_INIT_CB init_cb;
	T_MEDIALIB_INIT_INPUT init_input;

	init();	// initial file system, memory, camera, lcd and etc.

	init_cb_func_init(&init_cb);	//initial callback function pointer

	init_input.m_ChipType = MEDIALIB_CHIP_AK9801;
	init_input.m_AudioI2S = I2S_UNUSE;

	if (MediaLib_Init(&init_input, &init_cb) == AK_FALSE)
	{
		return;
	}
	//above only call one time when system start

	//play film or music
	record_media(argv[1]);

	//below only call one time when system close
	MediaLib_Destroy();
	return;
}

T_VOID record_media(char *filename)
{
	T_S32 fid;

	T_MEDIALIB_REC_OPEN_INPUT rec_open_input;
	T_MEDIALIB_REC_OPEN_OUTPUT rec_open_output;

	T_eMEDIALIB_REC_STATUS rec_status;
	char press_key;
	T_U8 tmp_buf[1600];
	T_MEDIALIB_REC_INFO RecInfo;
	T_VOID *hMedia;
//	T_pDATA pVideo;
	T_S32 video_time;
	T_U8 *pYUV1;
	T_U8 *pYUV2;
	T_U32 audio_tytes = 0;
	T_U32 fps = 10;
	T_U32 max_seconds = 30*60;// half an hour

	fid = FileOpen(filename);
	if(fid <= 0)
	{
		printf("open file failed\r\n");
		return;
	}

	memset(&rec_open_input, 0, sizeof(T_MEDIALIB_REC_OPEN_INPUT));

	rec_open_input.m_hMediaDest = fid;
	rec_open_input.m_bCaptureAudio = 1;
	rec_open_input.m_bHighQuality = 1;
	rec_open_input.m_bIdxInMem = 1;
	rec_open_input.m_IndexMemSize = (fps+2)*16 * max_seconds;
	rec_open_input.m_RecordSecond = 0;
	rec_open_input.m_MediaRecType = MEDIALIB_REC_AVI_NORMAL;//or MEDIALIB_REC_3GP;
// set video open info
	rec_open_input.m_VideoRecInfo.m_nFPS = fps;
	rec_open_input.m_VideoRecInfo.m_nWidth = 352;
	rec_open_input.m_VideoRecInfo.m_nHeight = 288;
	rec_open_input.m_VideoRecInfo.m_nKeyframeInterval = 19;
	rec_open_input.m_VideoRecInfo.m_nvbps = 600*1024;
	rec_open_input.m_VideoRecInfo.m_eVideoType = MEDIALIB_V_ENC_MJPG;//or MEDIALIB_V_ENC_H263;
	rec_open_input.m_SectorSize = 2048;
	rec_open_input.m_ExFunEnc = VD_EXfunc_YUV2JPEG;//set mjpeg encode function
	
// set audio open info
	rec_open_input.m_AudioRecInfo.m_BitsPerSample = 16;
	rec_open_input.m_AudioRecInfo.m_nChannel = 1;
	rec_open_input.m_AudioRecInfo.m_nSampleRate = 8000;
	rec_open_input.m_AudioRecInfo.m_ulDuration = 1000;
	rec_open_input.m_AudioRecInfo.m_Type = _SD_MEDIA_TYPE_PCM;
	
	open_cb_func_init(&(rec_open_input.m_CBFunc));	//initial callback function pointer;
	
	hMedia = MediaLib_Rec_Open(&rec_open_input,&rec_open_output);
	if (AK_NULL == hMedia)
	{
		printf("##MOVIE: MediaLib_Rec_Open Return NULL\r\n");
		FileClose(fid);
		return;
	}

	if (MediaLib_Rec_GetInfo(hMedia, &RecInfo) == AK_FALSE)
	{
		MediaLib_Rec_Close(hMedia);
		FileClose(fid);
		return;
	}

	if (AK_FALSE == MediaLib_Rec_Start(hMedia))
	{
		MediaLib_Rec_Close(hMedia);
		FileClose(fid);
		return;
	}

	while (AK_NULL == pYUV1)
	{
		pYUV1 = get_yuv_data();
	}

rec_loop://use MediaLib_Rec_ProcessAudio and MediaLib_Rec_ProcessVideo
	while(1)
	{
		if (rec_open_input.m_bCaptureAudio)
		{
			audio_tytes = get_audio_data(tmp_buf);//get audio data from audio encoder
			if (audio_tytes ! = 0)
			{
				if(MediaLib_Rec_ProcessAudio(hMedia, (T_U8 *)tmp_buf, audio_tytes) == AK_FALSE)
				{
					printf("MediaLib_Rec_ProcessAudio error\r\n");
					break;
				}
			}
		}
		tickcount = get_system_time_ms();//get current time in ms from starting recording
		pYUV2 = get_yuv_data();
		if (pYUV2 != AK_NULL)
		{
			pYUV1 = pYUV2;
		}
		video_time = MediaLib_Rec_ProcessVideo(hMedia, pYUV1, tickcount);
		if (video_time < 0)
		{
			printf("MediaLib_Rec_ProcessVideo error\r\n");
			break;
		}

		press_key = is_stop_button();//check whether stop
		if (press_key)
		{
			break;
		}
		rec_status = MediaLib_Rec_GetStatus(hMedia);
		if (MEDIALIB_REC_DOING != rec_status)
		{
			break;
		}
	}

	MediaLib_Rec_Stop(hMedia);
	FileClose(fid);

	if (continue_rec)//record to another file
	{
		fid = FileOpen(filename_new);
		if(fid <= 0)
		{
			printf("open file failed\r\n");
		}
		else
		{
			if (MediaLib_Rec_Restart(hMedia, fid, 0))
			{
				goto rec_loop;
			}
		}
	}

	MediaLib_Rec_Close(hMedia);

	FileClose(fid);

	return;
}

rec_loop://use MediaLib_Rec_WriteAudio and MediaLib_Rec_WriteVideo
	while(1)
	{
		if (rec_open_input.m_bCaptureAudio)
		{
			audio_tytes = get_audio_data(tmp_buf);//get audio data from audio encoder
			if (audio_tytes ! = 0)
			{
				if(MediaLib_Rec_WriteAudio(hMedia, (T_U8 *)tmp_buf, audio_tytes) == AK_FALSE)
				{
					printf("MediaLib_Rec_WriteAudio error\r\n");
					break;
				}
			}
		}
		tickcount = get_system_time_ms();//get current time in ms from starting recording
		pYUV2 = get_yuv_data();
		if (pYUV2 != AK_NULL)
		{
			pYUV1 = pYUV2;
		}
		ret = MediaLib_Rec_EncodeVideo(hMedia, pYUV1, tickcount);
		if (ret < 0)
		{
			printf("MediaLib_Rec_WriteVideo error\r\n");
			break;
		}
		if (ret > 0)
		{
			video_time = MediaLib_Rec_WriteVideo(hMedia);
		}

		press_key = is_stop_button();//check whether stop
		if (press_key)
		{
			break;
		}
		rec_status = MediaLib_Rec_GetStatus(hMedia);
		if (MEDIALIB_REC_DOING != rec_status)
		{
			break;
		}
	}

	@endcode

 ***************************************************/

#ifndef _MEDIA_RECORDER_LIB_H_
#define _MEDIA_RECORDER_LIB_H_

#include "medialib_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Open a resource
 *
 * @author Su_Dan
 * @param	rec_open_input		[in]	pointer of T_MEDIALIB_REC_OPEN_INPUT struct
 * @param	rec_open_output		[out]	pointer of T_MEDIALIB_REC_OPEN_OUTPUT struct
 * @return T_MEDIALIB_STRUCT
 * @retval	AK_NULL		open failed
 * @retval	other		open ok
 */
T_MEDIALIB_STRUCT MediaLib_Rec_Open(const T_MEDIALIB_REC_OPEN_INPUT *rec_open_input, T_MEDIALIB_REC_OPEN_OUTPUT *rec_open_output);


/**
 * @brief Close recorder
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Close ok
 * @retval	AK_FALSE	Close fail
 */
T_BOOL MediaLib_Rec_Close(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Start recording
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Start ok
 * @retval	AK_FALSE	Start fail
 */
T_BOOL MediaLib_Rec_Start(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Restart recording
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @param	hFile		[in]	file handle
 * @param	rec_event	[in]	see T_eMEDIALIB_REC_EVENT when segment record
								no use in normal record
 * @return T_BOOL
 * @retval	AK_TRUE		Restart ok
 * @retval	AK_FALSE	Restart fail
 */
T_BOOL MediaLib_Rec_Restart(T_MEDIALIB_STRUCT hMedia, T_S32 hFile, T_eMEDIALIB_REC_EVENT rec_event);


/**
 * @brief Stop recording
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Stop ok
 * @retval	AK_FALSE	Stop fail
 */
T_BOOL MediaLib_Rec_Stop(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief process audio data
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		process ok
 * @retval	AK_FALSE	process fail
 */
T_BOOL MediaLib_Rec_ProcessAudio(T_MEDIALIB_STRUCT hMedia, T_U8 *pAudioData, T_U32 ulAudioSize);

/**
 * @brief process video data
 *
 * @author Su_Dan
 * @param	 hMedia			[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @param	 pVideoData		[in]	pointer of YUV data
 * @param	 ulMilliSec		[in]	audio or system time in millisecond
 * @return T_S32
 * @retval	< 0		encode video fail
 * @retval	other	video time in millisecond
 */
T_S32 MediaLib_Rec_ProcessVideo(T_MEDIALIB_STRUCT hMedia, T_U8 *pVideoData, T_S32 ulMilliSec);


/**
 * @brief encode video
 *
 * @author Su_Dan
 * @param	hMedia			[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @param	pVideoData		[in]	pointer of YUV data
 * @param	ulMilliSec		[in]	audio or system time in millisecond
 * @return T_S32
 * @retval	< 0		Encode fail
 * @retval	0		Not encode
 * @retval	> 0		Encode ok
 */
T_S32 MediaLib_Rec_EncodeVideo(T_MEDIALIB_STRUCT hMedia, T_U8 *pVideoData, T_S32 ulMilliSec);

/**
 * @brief write video data
 *
 * @author Su_Dan
 * @param	hMedia			[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @return T_S32
 * @retval	< 0		write video fail
 * @retval	other	video time in millisecond
 */
T_S32 MediaLib_Rec_WriteVideo(T_MEDIALIB_STRUCT hMedia);

/**
 * @brief get video data after encode
 *
 * @author Su_Dan
 * @param	hMedia			[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @param	pRecVideoOut	[out]	pointer of T_MEDIALIB_REC_VIDEO_OUT
 * @return T_S32
 * @retval	< 0		Get fail
 * @retval	0		Not encode
 * @retval	> 0		Get ok
 */
T_S32 MediaLib_Rec_GetVideo(T_MEDIALIB_STRUCT hMedia, T_MEDIALIB_REC_VIDEO_OUT *pRecVideoOut);

/**
 * @brief write audio data
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		write ok
 * @retval	AK_FALSE	write fail
 */
T_BOOL MediaLib_Rec_WriteAudio(T_MEDIALIB_STRUCT hMedia, T_U8 *pAudioData, T_U32 ulAudioSize);


/**
 * @brief Get current recording status
 *
 * @author Su_Dan
 * @param	hMedia		[in]		pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @return T_eMEDIALIB_REC_STATUS
 * @retval MEDIALIB_REC_OPEN		the recording driver has not been opened
 * @retval MEDIALIB_REC_STOP		the recording driver has not been stopped
 * @retval MEDIALIB_REC_DOING		recording
 * @retval MEDIALIB_REC_SYSERR		system error while recording
 * @retval MEDIALIB_REC_MEMFULL		memory full error while recording
 * @retval MEDIALIB_REC_SYNERR		audio and video syn error while recording
 */
T_eMEDIALIB_REC_STATUS MediaLib_Rec_GetStatus(T_MEDIALIB_STRUCT hMedia);


/**
 * @brief Get information
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @param	pInfo		[out]	pointer of T_MEDIALIB_REC_INFO struct
 * @return T_BOOL
 * @retval	AK_TRUE		get info ok
 * @retval	AK_FALSE	get info fail
 */
T_BOOL MediaLib_Rec_GetInfo(T_MEDIALIB_STRUCT hMedia, T_MEDIALIB_REC_INFO *pInfo);


/**
 * @brief Set video bitrate
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @param	ulBitrate	[in]	video bitrate
 * @return T_BOOL
 * @retval	AK_TRUE		Set ok
 * @retval	AK_FALSE	Set fail
 */
T_BOOL MediaLib_Rec_SetVideoBitrate(T_MEDIALIB_STRUCT hMedia, T_U32 ulBitrate);


/**
 * @brief Set max video frame number of insert
 *
 * @author Su_Dan
 * @param	hMedia			[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @param	ulMaxInsertNum	[in]	max video frame number of insert
 * @return T_BOOL
 * @retval	AK_TRUE		Set ok
 * @retval	AK_FALSE	Set fail
 */
T_BOOL MediaLib_Rec_SetVideoMaxInsert(T_MEDIALIB_STRUCT hMedia, T_U32 ulMaxInsertNum);


/**
 * @brief Update handle of index file
 *
 * @author Su_Dan
 * @param	hMedia			[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @param	hIndexFile		[in]	new handle of index file
 * @return T_BOOL
 * @retval	AK_TRUE		Update ok
 * @retval	AK_FALSE	Update fail
 */
T_BOOL MediaLib_Rec_UpdateIndexFile(T_MEDIALIB_STRUCT hMedia, T_S32 hIndexFile);


/**
 * @brief Set buffering infomation
 *
 * @author Su_Dan
 * @param	hMedia			[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @param	ulRemainSize	[in]	remain bytes of file system buffering
 * @return T_BOOL
 * @retval	AK_TRUE		Set ok
 * @retval	AK_FALSE	Set fail
 */
T_BOOL MediaLib_Rec_SetBufferingInfo(T_MEDIALIB_STRUCT hMedia, T_U32 ulRemainSize);


/**
 * @brief Set drop frame when buffering status is abnormal
 *
 * @author Su_Dan
 * @param	hMedia			[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Rec_Open function
 * @param	ulDropNum		[in]	number of drop frames
 * @return T_BOOL
 * @retval	AK_TRUE		Set ok
 * @retval	AK_FALSE	Set fail
 */
T_BOOL MediaLib_Rec_SetDropFrame(T_MEDIALIB_STRUCT hMedia, T_U32 ulDropNum);

/*
 * @brief set the timestamp for mpeg4/h263 while recording
 *
 * @author 	Xia_Jiaquan
 * @param	hMedia			 [in]	pointer returned by MediaLib_Rec_Open
 * @param	timestamp_param	 [in]	pointer of T_MEDIALIB_REC_TIMESTAMP_PAR
 * @return 	T_S32
 * @retval	-1		set timestamp failed
 * @retval	0		set timestamp ok
 */
T_S32 MediaLib_Rec_SetTimeStamp(T_MEDIALIB_STRUCT hMedia, T_MEDIALIB_REC_TIMESTAMP_PAR *timestamp_param);

/*
 * @brief set the QP of Inter or Intral encode
 *
 * @author 	Xia_Jiaquan
 * @param	hMedia			 [in]	pointer returned by MediaLib_Rec_Open
 * @param	encQP_param	     [in]	pointer of T_MEDIALIB_REC_ENCQP_PAR
 * @return 	T_BOOL
 * @retval	AK_FALSE	set encQP failed
 * @retval	AK_TRUE		set encQP ok
 */
T_BOOL MediaLib_Rec_SetEncQP(T_MEDIALIB_STRUCT hMedia, T_MEDIALIB_REC_ENCQP_PAR *encQP_param);

#ifdef __cplusplus
}
#endif

#endif
