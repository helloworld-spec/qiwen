/**
 * @file media_player_lib.h
 * @brief This file provides MP4/3GP/AVI/AKV/RMVB/MKV/mp3/aac/amr/flac/ape... playing functions
 *
 * Copyright (C) 2012 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @author Su Dan, Zeng Yong
 * @date 2007-8-18
 * @update date 2012-1-17
 * @version 0.2.0
 * @version ever green group version: x.x
 * @note
	The following is an example to use playing APIs
   @code
T_VOID play_media(char* filaname);
T_VOID main(int argc, char* argv[])
{
	T_MEDIALIB_INIT_CB init_cb;
	T_MEDIALIB_INIT_INPUT init_input;

	init();	// initial file system, memory, lcd and etc.

	init_cb_func_init(&init_cb);	//initial callback function pointer

	init_input.m_ChipType = MEDIALIB_CHIP_UNKNOWN;
	init_input.m_AudioI2S = I2S_UNUSE;

	if (MediaLib_Init(&init_input, &init_cb) == AK_FALSE)
	{
		return;
	}
	//above only call one time when system start

	//play film or music
	play_media(argv[1]);

	//below only call one time when system close
	MediaLib_Destroy();
	return;
}

T_VOID play_media(char* filaname)
{
	T_S32 fid = 0;
	T_MEDIALIB_STRUCT hMedia;
	T_MEDIALIB_OPEN_INPUT open_input;
	T_MEDIALIB_OPEN_OUTPUT open_output;
	T_AUDIO_DECODE_OUT	AudioDecOut;
	T_VIDEO_DECODE_OUT	VideoDecOut;
	T_MEDIALIB_MEDIA_INFO media_info;
	T_U8 ImgYUV[352*288*2];
	T_U32 preview_time = 0;
	T_U32 sync_time = 0, begin_time = 0, start_system_time = 0;
	T_S32 ret_audio = 0, ret_video = 0;
	T_eMEDIALIB_STATUS player_status;

	fid = FileOpen("/test.3gp");
	if(fid <= 0)
	{
		printf("open file failed\r\n");
		return;
	}

	open_input.m_OpenType = MEDIALIB_OPEN_PLAY;
	open_input.m_MediaType = MEDIALIB_MEDIA_UNKNOWN;
	open_input.m_hMediaSource = fid;
	open_input.m_Scale = MEDIALIB_SCALE_1X;
	open_input.m_Rotate = MEDIALIB_ROTATE_0;
	open_input.m_AudioAttribute = 0;
	open_input.m_bCapabilityTest = AK_FALSE;
	open_input.m_bAVDiffTask = AK_FALSE;
	open_input.m_AudioOutInfo.m_SampleRate = 44100;
	open_input.m_AudioOutInfo.m_Channels = 1;
	open_input.m_AudioOutInfo.m_BitsPerSample = 16;
	open_input.m_VideoOutInfo.m_OutWidth = 0;
	open_input.m_VideoOutInfo.m_OutHeight = 0;

	open_cb_func_init(&(open_input.m_CBFunc));	//initial callback function pointer;

	hMedia = MediaLib_Open(&open_input, &open_output);

	if (AK_NULL == hMedia)
	{
		FileClose(fid);
		return;
	}

	if (MediaLib_GetInfo(hMedia, &media_info) == AK_FALSE)
	{
		MediaLib_Close(hMedia);
		FileClose(fid);
		return;
	}

	if (media_info.m_bHasVideo)
	{
		preview_time = media_info.m_ulTotalTime_ms * 30 / 100;
		if (MediaLib_Preview(hMedia, ImgYUV, preview_time) == AK_TRUE)
		{
			display(ImgYUV, media_info.m_VideoInfo.m_uWidth, media_info.m_VideoInfo.m_uHeight);
		}
		VideoDecOut.m_ulSize = open_output.m_ulVideoDecBufSize;
	}
	else
	{
		VideoDecOut.m_pBuffer = AK_NULL;
		VideoDecOut.m_ulSize = 0;
	}

	if (media_info.m_bHasAudio)
	{
		AudioDecOut.m_pBuffer = malloc(open_output.m_ulAudioDecBufSize);
		if (AK_NULL == AudioDecOut.m_pBuffer)
		{
			MediaLib_Close(hMedia);
			FileClose(fid);
			return;
		}
		AudioDecOut.m_ulSize = open_output.m_ulAudioDecBufSize;
	}
	else
	{
		AudioDecOut.m_pBuffer = AK_NULL;
		AudioDecOut.m_ulSize = 0;
	}

	begin_time = MediaLib_SetPosition(hMedia, 5000, AK_FALSE);	//play from 00:00:05, maybe can't seek there
	if (begin_time < 0)
	{
		MediaLib_Close(hMedia);
		free(AudioDecOut.m_pBuffer);
		FileClose(fid);
		return;
	}

	begin_time = MediaLib_Play(hMedia);
	if (begin_time < 0)
	{
		MediaLib_Close(hMedia);
		free(AudioDecOut.m_pBuffer);
		FileClose(fid);
		return;
	}

	if (media_info.m_bHasAudio && media_info.m_bHasVideo)//play with audio and video
	{
		while (1)
		{
			//return audio output data length
			ret_audio = MediaLib_DecodeAudioPack(hMedia, &AudioDecOut);
			if (ret_audio > 0)
			{
				//audio output
				audio_data_to_da(AudioDecOut.m_pBuffer, AudioDecOut.m_ulDecDataSize);
			}
			sync_time = begin_time + get_audio_time_ms();	//from 00:00:00
			ret_video = MediaLib_DecodeVideoPack(hMedia, &VideoDecOut, sync_time);
			if (ret_audio < 0 || ret_video < 0)
			{
				player_status = MediaLib_GetStatus(hMedia);
				if (MEDIALIB_ERR == player_status)
				{
					printf("error\r\n");
					break;
				}
				else if (MEDIALIB_END == player_status)
				{
					printf("end\r\n");
					break;
				}
			}
			//display video
			if (VideoDecOut.m_pBuffer != AK_NULL)
			{
				display(VideoDecOut.m_pBuffer, VideoDecOut.m_uDispWidth, VideoDecOut.m_uDispHeight);
			}
		}
	}
	else if (media_info.m_bHasVideo)//only video
	{
		start_system_time = get_system_time_ms();
		while (1)
		{
			sync_time = (get_system_time_ms() - start_system_time) + begin_time;
			ret_video = MediaLib_DecodeVideoPack(hMedia, &VideoDecOut, sync_time);
			if (ret_video < 0)
			{
				player_status = MediaLib_GetStatus(hMedia);
				if (MEDIALIB_ERR == player_status)
				{
					printf("error\r\n");
					break;
				}
				else if (MEDIALIB_END == player_status)
				{
					printf("end\r\n");
					break;
				}
			}
			//display video
			if (VideoDecOut.m_pBuffer != AK_NULL)
			{
				display(VideoDecOut.m_pBuffer, VideoDecOut.m_uDispWidth, VideoDecOut.m_uDispHeight);
			}
		}
	}
	else//only audio
	{
		while (1)
		{
			//return audio output data length
			ret_audio = MediaLib_DecodeAudioPack(hMedia, &AudioDecOut);
			if (ret_audio < 0)
			{
				player_status = MediaLib_GetStatus(hMedia);
				if (MEDIALIB_ERR == player_status)
				{
					printf("error\r\n");
					break;
				}
				else if (MEDIALIB_END == player_status)
				{
					printf("end\r\n");
					break;
				}
			}
			//audio output
			audio_data_to_da(AudioDecOut.m_pBuffer, AudioDecOut.m_ulDecDataSize);
		}
	}

	MediaLib_Close(hMedia);

	free(AudioDecOut.m_pBuffer);
	FileClose(fid);
	return;
}
	@endcode

 ***************************************************/

#ifndef _MEDIA_PLAYER_LIB_H_
#define _MEDIA_PLAYER_LIB_H_

#include "medialib_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get Player library version
 *
 * @author Su_Dan
 * @return const T_CHR *
 * @retval	version string
 */
const T_CHR *MediaLib_GetVersion(T_VOID);


/**
 * @brief Initial Player library and allocate global resource
 *
 * @author Su_Dan
 * @param	init_input	[in]	pointer of T_MEDIALIB_INIT_INPUT struct
 * @param	init_cb_fun	[in]	pointer of T_MEDIALIB_INIT_CB struct for callback func
 * @return T_BOOL
 * @retval	AK_TRUE		Initial ok
 * @retval	AK_FALSE	Initial fail
 */
T_BOOL MediaLib_Init(const T_MEDIALIB_INIT_INPUT *init_input, const T_MEDIALIB_INIT_CB *init_cb_fun);

/**
 * @brief Destroy Player library and free global resource
 *
 * @author Su_Dan
 * @return T_VOID
 */
T_VOID MediaLib_Destroy(T_VOID);

/**
 * @brief Open a resource
 *
 * @author Su_Dan
 * @param	open_input		[in]	pointer of T_MEDIALIB_OPEN_INPUT struct
 * @param	open_output		[out]	pointer of T_MEDIALIB_OPEN_OUTPUT struct
 * @return T_MEDIALIB_STRUCT
 * @retval	AK_NULL			open failed
 * @retval	other			open ok
 */
T_MEDIALIB_STRUCT MediaLib_Open(const T_MEDIALIB_OPEN_INPUT *open_input, T_MEDIALIB_OPEN_OUTPUT *open_output);

/**
 * @brief Close a resource
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		Close ok
 * @retval	AK_FALSE	Close fail
 */
T_BOOL MediaLib_Close(T_MEDIALIB_STRUCT hMedia);

/**
 * @brief Get information from an opened resource
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @param	pInfo		[in]	pointer of T_MEDIALIB_MEDIA_INFO struct
 * @return T_BOOL
 * @retval	AK_TRUE		get info ok
 * @retval	AK_FALSE	get info fail
 */
T_BOOL MediaLib_GetInfo(T_MEDIALIB_STRUCT hMedia, T_MEDIALIB_MEDIA_INFO *pInfo);

/**
 * @brief Preview one frame of an opened resource
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @param	pImgYUV		[in]	pointer of a YUV buffer
 * @param	ulMilliSec	[in]	position to preview
 * @return T_BOOL
 * @retval	AK_TRUE		preview ok
 * @retval	AK_FALSE	preview fail
 */
T_BOOL MediaLib_Preview(T_MEDIALIB_STRUCT hMedia, T_pVOID pImgYUV, T_U32 ulMilliSec);

/**
 * @brief Start playing an opened resource
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @return T_S32
 * @retval	< 0		play fail
 * @retval	other	if there are video and audio, last seek or pause audio time in millisecond
 * @retval	other	if no video, last seek or pause audio time in millisecond
 * @retval	other	if no audio, last seek or pause video time in millisecond
 */
T_S32 MediaLib_Play(T_MEDIALIB_STRUCT hMedia);

/**
 * @brief Stop playing an opened resource
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		stop ok
 * @retval	AK_FALSE	stop fail
 */
T_BOOL MediaLib_Stop(T_MEDIALIB_STRUCT hMedia);

/**
 * @brief Pause playing resource
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		pause ok
 * @retval	AK_FALSE	pause fail
 */
T_BOOL MediaLib_Pause(T_MEDIALIB_STRUCT hMedia);

/**
 * @brief Switch to fast forward status
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		switch ok
 * @retval	AK_FALSE	switch fail
 */
T_BOOL MediaLib_FastForward(T_MEDIALIB_STRUCT hMedia);

/**
 * @brief Switch to fast rewind status
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		switch ok
 * @retval	AK_FALSE	switch fail
 */
T_BOOL MediaLib_FastRewind(T_MEDIALIB_STRUCT hMedia);

#if 0
/**
 * @brief Set fast speed
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @param	nMultiple	[in]	speed multiple
 * @return T_U8
 * @retval	fast speed
 */
T_U8 MediaLib_SetFastSpeed(T_MEDIALIB_STRUCT hMedia, T_U8 nMultiple);
#endif

/**
 * @brief Get current playing status
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @return T_eMEDIALIB_STATUS
 * @retval	MEDIALIB_END		end
 * @retval	MEDIALIB_PLAYING	playing
 * @retval	MEDIALIB_FF			fast forward
 * @retval	MEDIALIB_FR			fast rewind
 * @retval	MEDIALIB_PAUSE		pause
 * @retval	MEDIALIB_STOP		stop
 * @retval	MEDIALIB_ERR		error
 * @retval	MEDIALIB_SEEK		seek
 */
T_eMEDIALIB_STATUS MediaLib_GetStatus(T_MEDIALIB_STRUCT hMedia);

/**
 * @brief Decode video stream
 *
 * @author Su_Dan
 * @param	hMedia			[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @param	pVideoDecOut	[in]	pointer of T_VIDEO_DECODE_OUT struct
 * @param	ulMilliSec		[in]	audio or system time in millisecond
 * @return T_S32
 * @retval	< 0		decode fail or end of resource
 * @retval	other	video time in millisecond
 */
T_S32 MediaLib_DecodeVideoPack(T_MEDIALIB_STRUCT hMedia, T_VIDEO_DECODE_OUT *pVideoDecOut, T_S32 ulMilliSec);

/**
 * @brief Decode audio stream
 *
 * @author Su_Dan
 * @param	hMedia			[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @param	pAudioDecOut	[in]	pointer of T_AUDIO_DECODE_OUT struct
 * @return T_S32
 * @retval	< 0		decode fail or end of resource
 * @retval	other	audio data length
 */
T_S32 MediaLib_DecodeAudioPack(T_MEDIALIB_STRUCT hMedia, T_AUDIO_DECODE_OUT *pAudioDecOut);

/**
 * @brief Set media resource position
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer of T_MEDIALIB_STRUCT struct which is returned by MediaLib_Open function
 * @param	lMilliSec	[in]	position of time in millisecond
 * @param	bSeekNext	[in]	seek forward, only valid to video
 * @return T_S32
 * @retval	< 0		set fail
 * @retval	other	video time in millisecond
 */
T_S32 MediaLib_SetPosition(T_MEDIALIB_STRUCT hMedia, T_S32 lMilliSec, T_BOOL bSeekNext);

/**
 * @brief Release memory of information to save space
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_BOOL
 * @retval	AK_TRUE		release ok
 * @retval	AK_FALSE	release fail
 */
T_BOOL MediaLib_ReleaseInfoMem(T_pVOID hMedia);

/**
 * @brief Set rotate mode for media play
 *
 * @author Su_Dan
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @param	rotate		[in]	rotate mode, see T_eMEDIALIB_ROTATE
 * @return T_BOOL
 * @retval	AK_TRUE		set ok
 * @retval	AK_FALSE	set fail
 */
T_BOOL MediaLib_SetDispRotate(T_pVOID hMedia, T_eMEDIALIB_ROTATE rotate);

/**
 * @brief Get audio decode time
 *
 * @author Deng_Zhou
 * @param	hMedia		[in]	pointer which is returned by MediaLib_Open function
 * @return T_S32
 * @retval	< 0		get failed
 * @retval	other	audio decode time in millisecond
 */
T_S32 MediaLib_GetAudioDecTime(T_pVOID hMedia);

/**
 * @brief Check a resource
 *
 * @author Su_Dan
 * @param	pCBFunc			[in]	pointer of T_MEDIALIB_CB struct
 * @param	hMediaSource	[in]	handle of source file
 * @param	open_output		[out]	pointer of T_MEDIALIB_CHECK_OUTPUT struct
 * @return T_eMEDIALIB_MEDIA_TYPE
 * @retval	MEDIALIB_MEDIA_UNKNOWN	unknown type
 * @retval	other					type of file
 */
T_eMEDIALIB_MEDIA_TYPE MediaLib_CheckFile(T_MEDIALIB_CB *pCBFunc, T_S32 hMediaSource, T_MEDIALIB_CHECK_OUTPUT *check_output);

/**
 * @brief Get picture meta info from audio file in ID3 format
 *
 * @author Su_Dan
 * @param	pCBFunc			[in]	pointer of T_MEDIALIB_CB struct
 * @param	hMediaSource	[in]	handle of source file
 * @param	picBuf			[out]	pointer of picture buffer
 * @param	picLen			[out]	pointer of picture length in bytes
 * @return T_pVOID
 * @retval	AK_NULL		get failed
 * @retval	other		get ok
 */
T_pVOID MediaLib_GetPicMetaInfo(T_MEDIALIB_CB *pCBFunc, T_S32 hMediaSource, T_U8 **picBuf, T_U32 *picLen);

/**
 * @brief Release resource when used by MediaLib_GetPicMetaInfo function
 *
 * @author Su_Dan
 * @param	pCBFunc			[in]	pointer of T_MEDIALIB_CB struct
 * @param	pMetapic		[in]	pointer returned by MediaLib_GetPicMetaInfo function
 * @return T_VOID
 */
T_VOID MediaLib_ReleasePicMetaInfo(T_MEDIALIB_CB *pCBFunc, T_pVOID pMetapic);

/**
 * @brief Set parameters for decryption, must be called before MediaLib_Open and after MediaLib_Init
 *
 * @author Su_Dan
 * @param	hMedia			[in]	pointer which is returned by MediaLib_Open function
 * @param	decrypt_param	[in]	parameters for decryption
 * @return T_BOOL
 * @retval	AK_TRUE		set ok
 * @retval	AK_FALSE	set fail
 */
T_BOOL MediaLib_SetDecryptParam(T_MEDIALIB_DECRYPT_PARAM *decrypt_param);

/**
 * @brief Get meta info from audio file in ID3 format
 *
 * @author Su_Dan
 * @param	pCBFunc			[in]	pointer of T_MEDIALIB_CB struct
 * @param	hMediaSource	[in]	handle of source file
 * @param	MediaType		[in]	media type
 * @param	pMetaInfo		[out]	pointer of T_MEDIALIB_META_INFO struct
 * @return T_pVOID
 * @retval	AK_NULL		get failed
 * @retval	other		get ok
 */
T_pVOID MediaLib_GetID3MetaInfo(T_MEDIALIB_CB *pCBFunc, T_S32 hMediaSource, T_eMEDIALIB_MEDIA_TYPE MediaType, T_MEDIALIB_META_INFO *pMetaInfo);

/**
 * @brief Release resource when used by MediaLib_GetID3MetaInfo function
 *
 * @author Su_Dan
 * @param	pCBFunc			[in]	pointer of T_MEDIALIB_CB struct
 * @param	pID3Meta		[in]	pointer returned by MediaLib_GetID3MetaInfo function
 * @return T_VOID
 */
T_VOID MediaLib_ReleaseID3MetaInfo(T_MEDIALIB_CB *pCBFunc, T_pVOID pID3Meta);

#ifdef __cplusplus
}
#endif

#endif//_MEDIA_PLAYER_LIB_H_
