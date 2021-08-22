#ifndef _AK_GLOBAL_H_
#define _AK_GLOBAL_H_

#include <stdlib.h>

/* audio function status */
enum audio_func_status {
	AUDIO_FUNC_DISABLE = 0x00,
	AUDIO_FUNC_ENABLE,
};

/* audio channel type */
enum audio_channel_type {
	AUDIO_CHANNEL_RESERVED = 0x00,
	AUDIO_CHANNEL_MONO,
	AUDIO_CHANNEL_STEREO,
};

enum ak_audio_type {
	AK_AUDIO_TYPE_UNKNOWN,
	AK_AUDIO_TYPE_MIDI,
	AK_AUDIO_TYPE_MP3,
	AK_AUDIO_TYPE_AMR,
	AK_AUDIO_TYPE_AAC,
	AK_AUDIO_TYPE_WMA,
	AK_AUDIO_TYPE_PCM,
	AK_AUDIO_TYPE_ADPCM_IMA,
	AK_AUDIO_TYPE_ADPCM_MS,
	AK_AUDIO_TYPE_ADPCM_FLASH,
	AK_AUDIO_TYPE_APE,
	AK_AUDIO_TYPE_FLAC,
	AK_AUDIO_TYPE_OGG_FLAC,
	AK_AUDIO_TYPE_RA8LBR,
	AK_AUDIO_TYPE_DRA,
	AK_AUDIO_TYPE_OGG_VORBIS,
	AK_AUDIO_TYPE_AC3,
	AK_AUDIO_TYPE_PCM_ALAW,
	AK_AUDIO_TYPE_PCM_ULAW,
	AK_AUDIO_TYPE_SBC,
	AK_AUDIO_TYPE_SPEEX
};

/* video encode frame define */
enum video_frame_type {
    FRAME_TYPE_P,
    FRAME_TYPE_I
};

/* audio input/output param */
struct pcm_param {
	unsigned int sample_rate;
	unsigned int sample_bits;
	unsigned int channel_num;
};

/* audio encode/decode param */
struct audio_param {
	unsigned int type;					//encode/decode type
	unsigned int sample_rate;
	unsigned int sample_bits;
	unsigned int channel_num;
};

/* audio/video data frame */
struct frame {
	unsigned char *data;	//frame data
	unsigned int len;		//frame len in bytes
	unsigned long long ts;	//timestamp(ms)
	unsigned long seq_no;	//current frame sequence no.
};

struct audio_stream {
	unsigned char *data; 	//stream data
	unsigned int len; 		//stream len in bytes
	unsigned long long ts;	//timestamp(ms)
	unsigned long seq_no;	//current stream sequence no according to frame
};

struct video_stream {
	unsigned char *data;	//stream data
	unsigned int len; 		//stream len in bytes
	unsigned long long ts;	//timestamp(ms)
	unsigned long seq_no;	//current stream sequence no according to frame
	enum video_frame_type frame_type;	//I or P frame
};

#endif
