#ifndef _PLAT_PCM_H_
#define _PLAT_PCM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "akpcm.h"
#include "sdfilter.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_global.h"

#define AK_INVALID_FD       -1
#define EQ_ARRAY_NUM		54

/* adc aslc param */
/* round(32768 * 10.^(val/20)), unit of val is dB */
#define AD_STONE1_X      3 /* stone1_x = -80, dB */
#define AD_STONE1_Y      3 /* stone1_y = -80, dB */
#define AD_STONE2_X     10 /* stone2_x = -70, dB */
#define AD_ST2_DB_X     (-70) /* stone2_x = -70, dB */

/* dac aslc param */
/* val=round(32768 * 10.^((dB)/20)) */
#define MAX_VOL 	32768	/* 0dB, = maxvol is db value */
#define DA_STONE1_X    104	   /* -50dB */
#define DA_STONE1_Y    33	   /* -60dB */
#define DA_STONE2_X    328	   /* -40dB, = st2 */
#define DA_STONE3_Y    MAX_VOL 
#define DA_STONE4_X    32768   /* 0dB */
#define DA_STONE4_Y    MAX_VOL
#define DA_ST2_DB_X    (-40)
#define MAX_DB 			(0)			/* -2dB, = maxvol is db value */


typedef enum {
	SET_RSTBUF,

	/* DA commands */
	DA_SET_PARS,
	DA_SET_DEV_HP,
	DA_SET_DEV_LO,
	DA_SET_GAIN,
	DA_SET_SDF,

	/* AD commands */
	AD_SET_PARS,
	AD_SET_DEV_MIC,
	AD_SET_DEV_LI,
	AD_SET_DEV_AUTO,
	AD_SET_GAIN,
	AD_SET_SDF,
} pcm_cmd_t;

enum func_switch {
	FUNC_DISABLE,
	FUNC_ENABLE
};

typedef struct raw_param {
	unsigned int sample_rate;
	unsigned int sample_bits;
} raw_param_t;

typedef struct pcm_ft {
	int gain;
	int dev;
} pcm_ft_t;

struct echo_lib_param
{

    unsigned long   m_aecEna;           // enable AEC
    unsigned long   m_PreprocessEna;    // enable noise reduction
    unsigned long   m_agcEna;           // enable AGC
    
    unsigned long   m_framelen;         // frame size in samples, 128 is optimal 
    unsigned long   m_tail;             // tail size in samples
    unsigned long   m_agcLevel;         // agc's target level, 0: use default. use AK32Q15
    signed short   m_maxGain;          // agc's max_gain, Q0
    signed short   m_minGain;          // agc's min_gain, use AK16Q10
    unsigned long   m_farThreshold;     // max amplitude of far signal, 1~32767(Q15), 0: use default. use AK32Q15
    signed short   m_farDigiGain;      // digtal gain for far signal, Q10, 0: use default. use AK16Q10
    signed short   m_nearDigiGain;     // digtal gain for near signal, Q10, 0: use default. use AK16Q10
    signed short   m_noiseSuppressDb;  // attenuation of noise in dB (negative number), 0: use default
    signed short   m_nearSensitivity;  // sensitivity of near-end speech [1:100], 0: use default
};

typedef struct pcm_user {
	int cp_fd;
	unsigned char mute_flag;	/* set to mute flag */
	unsigned char status;		/* audio play status */
	int interval;				/* pcm data interval, unit: ms */
	unsigned long input_len;	/* resample max input len */
	unsigned int actual_rate;	/* actual sample rate */
	unsigned int actual_ch;		/* actual channel:now only support MONO */
	struct akpcm_pars param;
	
	struct echo_lib_param echo_attr;
	
	pcm_ft_t ak_ft;

	void *sdfilter;				/* filter handle */
	void *sdfilter_aslc;		/* aslc handle */
	void *sdfilter_eq;
	enum func_switch sdf_switch;
    struct ak_audio_eq_attr *eq_attr;
} pcm_user_t;

static void* sdfilter_malloc_cb(unsigned long size)
{
    return ak_calloc(1, size);
}

static void sdfilter_free_cb(void *ptr)
{
    return ak_free(ptr);
}
/**
 * sdfilter_close - filter close 
 * @handler[IN]: filter handle
 * return: 0 success, -1 failed
 * notes:
 */
static inline int sdfilter_close(void *handler)
{
	if (NULL == handler)
		return AK_FAILED;

	return _SD_Filter_Close(handler);
}

/**
 * sdfilter_open_resample - resample open 
 * @user[IN]: AD device opened handle
 * @rate_in[IN]: input sample rate
 * @rate_out[IN]: output sample rate
 * return: resample handle
 * notes:
 */
static inline void* sdfilter_open_resample(pcm_user_t *user,
                        unsigned int rate_in, unsigned int rate_out)
{
    if (user->sdfilter) {
		sdfilter_close(user->sdfilter);
		user->sdfilter = NULL;
	}

	T_AUDIO_FILTER_INPUT s_ininfo;
	memset(&s_ininfo, 0, sizeof(T_AUDIO_FILTER_INPUT));

	s_ininfo.cb_fun.Malloc = sdfilter_malloc_cb;
	s_ininfo.cb_fun.Free = sdfilter_free_cb;
	s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;

	s_ininfo.m_info.m_BitsPerSample = user->param.sample_bits;
	s_ininfo.m_info.m_Channels = user->actual_ch;
	s_ininfo.m_info.m_SampleRate = rate_in;

	s_ininfo.m_info.m_Type = _SD_FILTER_RESAMPLE;
	s_ininfo.m_info.m_Private.m_resample.maxinputlen = user->input_len;
	s_ininfo.m_info.m_Private.m_resample.outSrindex = 0;
	s_ininfo.m_info.m_Private.m_resample.outSrFree = rate_out;
	s_ininfo.m_info.m_Private.m_resample.reSampleArithmetic = RESAMPLE_ARITHMETIC_1;
	s_ininfo.chip = AUDIOLIB_CHIP_AK39XXEV3;
	s_ininfo.strVersion = (char *)AUDIO_FILTER_VERSION_STRING;

	return _SD_Filter_Open(&s_ininfo);
}

/**
 * sdfilter_open - filter open 
 * @user[IN]: AD device opened handle
 * @filter_type[IN]: filter type
 * return: filter handle
 * notes:open aslc or tone detection
 */
static inline void* sdfilter_open(struct pcm_param *pcm_attr_in,
                        T_AUDIO_FILTER_TYPE filter_type, unsigned int rate_in)
{
	T_AUDIO_FILTER_INPUT s_ininfo;
	memset(&s_ininfo, 0, sizeof(T_AUDIO_FILTER_INPUT));

	s_ininfo.cb_fun.Malloc = sdfilter_malloc_cb;
	s_ininfo.cb_fun.Free = sdfilter_free_cb;
	s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;
	s_ininfo.cb_fun.delay = AK_NULL;

	s_ininfo.m_info.m_BitsPerSample = pcm_attr_in->sample_bits;
	s_ininfo.m_info.m_Channels = pcm_attr_in->channel_num;
	s_ininfo.m_info.m_SampleRate = pcm_attr_in->sample_rate;

    s_ininfo.chip = AUDIOLIB_CHIP_AK39XXEV3;
    s_ininfo.m_info.m_Type = filter_type;
	s_ininfo.strVersion = (char *)AUDIO_FILTER_VERSION_STRING;

    switch (filter_type) {
    case _SD_FILTER_ASLC:   /* ASLC */
    	s_ininfo.m_info.m_Private.m_aslc.aslcEna = 1;
    	s_ininfo.m_info.m_Private.m_aslc.aslcLimitLevel = 32700;
    	s_ininfo.m_info.m_Private.m_aslc.aslcStartLevel = 20000;
    	s_ininfo.m_info.m_Private.m_aslc.jointChannels = 2;
        break;
    case _SD_FILTER_TONE_DETECTION: /* tone detection */
    	s_ininfo.m_info.m_SampleRate = rate_in;
        break;
    default:
        return NULL;
    }

	return _SD_Filter_Open(&s_ininfo);
}

static inline void* sdfilter_open_eq(struct pcm_param *pcm_attr_in, T_EQ_MODE eqmode,
										struct ak_audio_eq_attr *eq_attr)
{
    if (NULL == pcm_attr_in)
    {
        ak_print_error_ex("dev_param is NULL\n");
        return NULL;
    }
    if (NULL == eq_attr)
    {
        ak_print_error_ex("eq_attr is NULL\n");
        return NULL;
    }
	T_AUDIO_FILTER_INPUT s_ininfo;
	memset(&s_ininfo, 0, sizeof(T_AUDIO_FILTER_INPUT));
    int i = 0;

	s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)free;
	s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;
	s_ininfo.cb_fun.delay = AK_NULL;

	s_ininfo.m_info.m_BitsPerSample = pcm_attr_in->sample_bits;
	s_ininfo.m_info.m_Channels = pcm_attr_in->channel_num;
	s_ininfo.m_info.m_SampleRate = pcm_attr_in->sample_rate;

    s_ininfo.chip = AUDIOLIB_CHIP_AK39XXEV3;
    s_ininfo.m_info.m_Type = _SD_FILTER_EQ;
	s_ininfo.strVersion = (char *)AUDIO_FILTER_VERSION_STRING;

	s_ininfo.m_info.m_Private.m_eq.eqmode = eqmode;//_SD_EQ_USER_DEFINE; //_SD_EQ_MODE_NORMAL; // mode;
	s_ininfo.m_info.m_Private.m_eq.preGain = eq_attr->pre_gain;//(T_S16)(0*(1<<10));
	s_ininfo.m_info.m_Private.m_eq.aslcEna = eq_attr->aslc_ena;
	s_ininfo.m_info.m_Private.m_eq.aslcLevelMax = eq_attr->aslc_level_max;
	if (_SD_EQ_USER_DEFINE == s_ininfo.m_info.m_Private.m_eq.eqmode) {
		s_ininfo.m_info.m_Private.m_eq.bands = eq_attr->bands;
	    for (i = 0; i < eq_attr->bands; i++)
        {   
    		s_ininfo.m_info.m_Private.m_eq.bandfreqs[i] = eq_attr->bandfreqs[i];//800;
    		s_ininfo.m_info.m_Private.m_eq.bandgains[i] = eq_attr->bandgains[i];//(T_S16)(-6*(1<<10));
    		s_ininfo.m_info.m_Private.m_eq.bandQ[i] = eq_attr->bandQ[i];//(T_U16)(1*(1<<10));
    		s_ininfo.m_info.m_Private.m_eq.bandTypes[i] = eq_attr->band_types[i];//FILTER_TYPE_HPF;
        }
	}

	return _SD_Filter_Open(&s_ininfo);
}

static inline int sdfilter_eq_param_convert(void *sdfilter_eq, struct pcm_param *pcm_attr_in,
												struct ak_audio_eq_attr *eq_attr, 
												signed long *out_addr, int out_len)
{
	if (!sdfilter_eq) {
		ak_print_error_ex("user is NULL\n");
		return AK_FAILED;
	} 
	if (!pcm_attr_in) {
		ak_print_error_ex("user is NULL\n");
		return AK_FAILED;
	} 
	if (!eq_attr) {
		ak_print_error_ex("eq_attr is NULL \n");
		return AK_FAILED;
	} 
	if (!out_addr) {
		ak_print_error_ex("out_addr is NULL \n");
		return AK_FAILED;
	} 
	
	T_AUDIO_FILTER_IN_INFO info={0};
	int i = 0;

	info.m_Type = _SD_FILTER_EQ;
	info.m_SampleRate = pcm_attr_in->sample_rate;	
	info.m_Channels =  pcm_attr_in->channel_num;
	info.m_BitsPerSample = pcm_attr_in->sample_bits;

	memset(&info.m_Private, 0, sizeof(info.m_Private));
	info.m_Private.m_eq.preGain = eq_attr->pre_gain;
	info.m_Private.m_eq.eqmode = _SD_EQ_USER_DEFINE;
	info.m_Private.m_eq.bands = eq_attr->bands;
	info.m_Private.m_eq.aslcEna = eq_attr->aslc_ena;
	info.m_Private.m_eq.aslcLevelMax = eq_attr->aslc_level_max;
	for (i = 0; i < EQ_MAX_BANDS; i++) {
		info.m_Private.m_eq.bandTypes[i] = eq_attr->band_types[i];
		info.m_Private.m_eq.bandfreqs[i] = eq_attr->bandfreqs[i];
		info.m_Private.m_eq.bandQ[i] = eq_attr->bandQ[i];
		info.m_Private.m_eq.bandgains[i] = eq_attr->bandgains[i];
	}
	void *out_parma = _SD_Filter_GetEqTimePara(sdfilter_eq, &info);
	memcpy(out_addr, out_parma, sizeof(signed long) * out_len);
	
	_SD_Filter_DestoryEqTimePara(sdfilter_eq, out_parma);
	
	return AK_SUCCESS;
}

/**
 * sdfilter_control - filter deal with data
 * @handler[IN]: resample handle
 * @inbuf[IN]: audio origin input data
 * @inlen[IN]: audio input buffer length
 * @outbuf[IN]: audio output data
 * @outlen[IN]: audio output buffer length
 * return: output data length
 * notes:
 */
static inline int sdfilter_control(void *handler, unsigned char *inbuf,
	unsigned int inlen, unsigned char *outbuf, unsigned int outlen)
{
	T_AUDIO_FILTER_BUF_STRC fbuf_strc = {0};

	fbuf_strc.buf_in = inbuf;
	fbuf_strc.buf_out = outbuf;
	fbuf_strc.len_in = inlen;
	fbuf_strc.len_out = outlen;

	return _SD_Filter_Control(handler, &fbuf_strc);
}

#endif
