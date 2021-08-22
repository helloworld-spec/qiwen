#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "sdfilter.h"
#include "ak_thread.h"


#include "ak_common.h"
//#include "ak_common_audio.h"
//#include "ak_mem.h"

//#include "ak_common_inner.h"
//#include "ak_thread.h"
//#include "ak_log.h"
//#include "list.h"

#include "ak_vqe.h"

//#include "pcm.h"
#include "list.h"
//#include "internal_error.h"

//#include "ak_ring_buffer.h"
//#include "ak_ai.h"
//#include "ai_ipcsrv.h"



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

#define VQE_USER_NUM (6)

struct effect_user 
{
    void *lib_handle;		/* filter lib handle */
    struct pcm_param user_attr_in;
    enum ak_vqe_type effect_type;
    unsigned char *outbuf;
    unsigned int outbuf_len;
    int out_sample_rate;
};

struct audio_effect_ctrl 
{
    int user_count;	
    struct effect_user *user_arry[VQE_USER_NUM];
    pthread_rwlock_t user_mutex;	/* lock the adc_dev_info  */
};


//extern ak_rwlock_t vqe_mux;

static struct audio_effect_ctrl effect_ctrl =
{
	.user_count = 0,	
	.user_arry = {0},
	.user_mutex = PTHREAD_RWLOCK_INITIALIZER,
};

static const char vqe_version[] = "libplat_vqe V1.0.00";

static void *filter_alloc(unsigned int size)
{
    return ak_calloc(1, size);
}

/**
 * da_get_stone2_y - 
 * 
 * 
 */
static int da_get_stone2_y(int db)
{
	int result2 = 0;
	float exponential = 0.000;
	float exponential_float = 0.000;

	/* result2 = round(32768 * 10.^((DA_ST2_DB_X + db)/20)) */
	exponential = (((float)(DA_ST2_DB_X + db)) / 20.000);
	ak_print_info_ex("first exponential[%d] = %f\n", db, exponential);	
	if (exponential < 0.000) /* negative */
	{			
		exponential = (float)((abs(exponential * 1000))) / 1000.000;
		ak_print_info_ex( "second exponential[%d] = %f\n", db, exponential);	
		exponential_float = (pow(10, exponential));
		exponential_float = exponential_float == 0 ? 1 : exponential_float;
		exponential_float = 1.000 / ((float)exponential_float); /* µ¹Êý*/
		ak_print_info_ex( "exponential_float[%d] = %f\n", db, exponential_float);			
		result2 = round(MAX_VOL * exponential_float);
	}
	else
	{
		result2 = round(MAX_VOL * pow(10, exponential));
	}			
	
	ak_print_notice_ex( "result2[%d] = %d\n", db, result2);
	
	return result2;
}

/**
 * da_get_stone3_x - 
 * 
 * 
 */
static int da_get_stone3_x(int db)
{
	int result3 = 0;
	float exponential = 0.000;
	float exponential_float = 0.000;

	/* result3 = round(32768 * 10.^((-db+MAX_DB)/20)) */
	exponential = (((float)(MAX_DB - db)) / 20.000);
	ak_print_info_ex( "first exponential[%d] = %f\n", db, exponential);	
	if (exponential < 0.000)
	{			
		exponential = (float)((abs(exponential * 1000))) / 1000.000;
		ak_print_info_ex( "second exponential[%d] = %f\n", db, exponential); 
		exponential_float = (pow(10, exponential));
		exponential_float = exponential_float == 0 ? 1 : exponential_float;
		exponential_float = 1.000 / ((float)exponential_float);
		ak_print_info_ex( "exponential_float[%d] = %f\n", db, exponential_float);			
		result3 = round(MAX_VOL * exponential_float);
	}
	else
	{
		result3 = round(MAX_VOL * pow(10, exponential));
	}	

	ak_print_warning_ex( "result3[%d] = %d\n", db, result3);	
	return result3;
}

/**
 * set_da_aslc_param - set aslc parameter
 * @handle[IN]: aslc open handle 
 * @level[IN]: volume level
 * return: 0 success, -1 failed
 */
static int set_da_aslc_param(void *handler, int db)
{
	int ret = AK_FAILED;

	if (handler) 
	{
		int stone2_y = da_get_stone2_y(db);
		int stone3_x = da_get_stone3_x(db);

		ak_print_notice_ex( "stone2_y=%d, stone3_x=%d\n",stone2_y, stone3_x);

		T_FILTER_MILESTONE tmile = {0};
		tmile.lookAheadTime = 6;
		tmile.gainAttackTime = 3;
		tmile.gainReleaseTime = 300;
		tmile.num = 5;
		tmile.stone[0].x = 0;
		tmile.stone[0].y = 0;
		tmile.stone[1].x = DA_STONE1_X;
		tmile.stone[1].y = DA_STONE1_Y;
		tmile.stone[2].x = DA_STONE2_X;
		tmile.stone[2].y = stone2_y;
		tmile.stone[3].x = stone3_x;
		tmile.stone[3].y = DA_STONE3_Y;
		tmile.stone[4].x = DA_STONE4_X;
		tmile.stone[4].y = DA_STONE4_Y;

		if (AK_TRUE == _SD_Filter_SetAslcMileStones(handler, &tmile))
			ret = AK_SUCCESS;
	}

	return ret;
}

/* return out buffer length */
static int get_outbuf_size(struct effect_user *user, unsigned int inbuf_len)
{
    if (!user)
    {
        ak_print_error_ex( "user is NULL\n");
        return 0;
    }

    int outbuf_len = 0;
    if (AK_VQE_RESAMPLE == user->effect_type)
    {
        if (0 == user->out_sample_rate)
        {
            ak_print_error_ex( "out_sample_rate = 0\n");
            return 0;
        }
        int tmp = (inbuf_len * user->out_sample_rate);
        int resample_len = (tmp / user->user_attr_in.sample_rate);
        if (tmp % user->out_sample_rate) 
        {
            resample_len += 2;
        }
        outbuf_len = resample_len;        
    }
    else
    {
        outbuf_len = inbuf_len;
    }

    return outbuf_len;
}
static int check_effect_type(int effect_type)
{
    if (effect_type < 0 || effect_type >= AK_VQE_TYPE_NUM)
    {
        ak_print_error_ex( "effect_type invalid\n");
        return ERROR_TYPE_INVALID_ARG;
    }
    return AK_SUCCESS;
}

static int check_sample_rate_param(int sample_rate)
{
	if (sample_rate != AK_AUDIO_SAMPLE_RATE_8000 &&
		sample_rate != AK_AUDIO_SAMPLE_RATE_12000 &&
		sample_rate != AK_AUDIO_SAMPLE_RATE_11025 &&
		sample_rate != AK_AUDIO_SAMPLE_RATE_16000 &&
		sample_rate != AK_AUDIO_SAMPLE_RATE_22050 &&
		sample_rate != AK_AUDIO_SAMPLE_RATE_24000 &&
		sample_rate != AK_AUDIO_SAMPLE_RATE_32000 &&
		sample_rate != AK_AUDIO_SAMPLE_RATE_44100 &&
		sample_rate != AK_AUDIO_SAMPLE_RATE_48000) 
	{
		/* sample rate not support  */
		ak_print_error_ex( "sample rate not suppprt %d \n", sample_rate);
		return ERROR_TYPE_INVALID_ARG;
	}

	return AK_SUCCESS;
}

static int check_audio_data_attr(struct pcm_param *pcm_attr)
{
	if (pcm_attr->sample_bits != 16) 
	{
		ak_print_error_ex( "sample bit only suppirt 16bit,now is %d \n",
							pcm_attr->sample_bits);
		return ERROR_TYPE_INVALID_ARG;
	}

	if (pcm_attr->channel_num != AUDIO_CHANNEL_MONO && 
		pcm_attr->channel_num != AUDIO_CHANNEL_STEREO) 
	{
		ak_print_error_ex( "channel_num not support now is %d \n",
							pcm_attr->channel_num);
		return ERROR_TYPE_INVALID_ARG;
	}
	return check_sample_rate_param(pcm_attr->sample_rate);
}

static int check_resample_attr_param(struct pcm_param *pcm_attr_in, 
                                            struct pcm_param *pcm_attr_out)
{
    if (!pcm_attr_in)
    {
        ak_print_error_ex( "pcm_attr_in is NULL\n");
        return ERROR_TYPE_INVALID_ARG;
    }
    
    if (!pcm_attr_out)
    {
        ak_print_error_ex( "pcm_attr_out is NULL\n");
        return ERROR_TYPE_INVALID_ARG;
    }
    
    if (check_audio_data_attr(pcm_attr_in))
    {
        ak_print_error_ex( "pcm_attr_in param invalid\n");
        return ERROR_TYPE_INVALID_ARG;
    }
    
    if (check_audio_data_attr(pcm_attr_out))
    {
        ak_print_error_ex( "pcm_attr_out param invalid\n");
        return ERROR_TYPE_INVALID_ARG;
    } 
    return AK_SUCCESS;
}

static int check_eq_attr_param(struct pcm_param *pcm_attr_in, 
                                    struct ak_audio_eq_attr *eq_attr)
{
    if (!pcm_attr_in)
    {
        ak_print_error_ex( "pcm_attr_in is NULL\n");
        return ERROR_TYPE_INVALID_ARG;
    }
    
    if (!eq_attr)
    {
        ak_print_error_ex( "eq_attr is NULL\n");
        return ERROR_TYPE_INVALID_ARG;
    }
    
    if (check_audio_data_attr(pcm_attr_in))
    {
        ak_print_error_ex( "pcm_attr_in param invalid\n");
        return ERROR_TYPE_INVALID_ARG;
    }
    return AK_SUCCESS;
}

static int check_effect_control_param(unsigned char *inbuf, unsigned int inbuf_len, 
                                         unsigned char **outbuf, unsigned int *ret_data_len)
{
    if (!inbuf)
    {
        ak_print_error_ex( "inbuf is NULL\n");
        return ERROR_TYPE_INVALID_ARG;
    }

    if (0 >= inbuf_len)
    {
        ak_print_error_ex( "inlen is <= 0\n");
        return ERROR_TYPE_INVALID_ARG;
    }

    if (!ret_data_len)
    {
        ak_print_error_ex( "ret_data_len is NULL\n");
        return ERROR_TYPE_POINTER_NULL;
    }

    return AK_SUCCESS;
}

static struct effect_user *get_effect_user(int effect_handle_id)
{
    if (0 > effect_handle_id || VQE_USER_NUM < effect_handle_id)
    {
        ak_print_error_ex( "effect_handle_id invalid\n");
        return NULL;
    }

    return effect_ctrl.user_arry[effect_handle_id];
}

static void del_effect_user(int effect_handle_id)
{
    if (effect_ctrl.user_arry[effect_handle_id])
    {
        ak_free(effect_ctrl.user_arry[effect_handle_id]);
        effect_ctrl.user_arry[effect_handle_id] = NULL;
        effect_ctrl.user_count--;
    }
    else
    {
        ak_print_error_ex( "user is NULL\n");
    }
}

/**
 * ak_vqe_get_version - get voice quality enhancement module version
 * return: version string
 * notes:
 */
const char* ak_vqe_get_version(void)
{
	return vqe_version;
}

/**
 * ak_vqe_open - open voice quality enhancement module
 * @vqe_type[IN]: voice quality enhancement type, resample/aslc/eq
 * @vqe_handle_id[OUT]: vqe module opened handle id
 * return: 0 success, other failed
 * notes: 
 */
int ak_vqe_open(enum ak_vqe_type vqe_type, int *vqe_handle_id)
{
    ak_print_normal_ex( "vqe version: %s\n", vqe_version);

    if (NULL == vqe_handle_id)
    {
        ak_print_error_ex( "effect_handle_id pointer is NULL\n");
        return ERROR_TYPE_POINTER_NULL;
    }
    *vqe_handle_id = -1;
    
    int ret = check_effect_type(vqe_type);
    if (ret)
        return ret;
    
	pthread_rwlock_wrlock(&effect_ctrl.user_mutex);  
    ak_print_normal_ex( "entry...\n");

    if (VQE_USER_NUM < effect_ctrl.user_count)
    {
        ak_print_error_ex( "usser too many\n");
        ret = ERROR_VQE_USER_TOO_MANY;
        goto effect_open_end;
    }
    struct effect_user *user = ak_calloc(1, sizeof(struct effect_user));
    if (!user)
    {
        ak_print_error_ex( "malloc user error\n");
        ret = ERROR_TYPE_MALLOC_FAILED;
        goto effect_open_end;
    }

    user->effect_type = vqe_type;

    int i = 0;
    for (i = 0; i <= VQE_USER_NUM; i++)
    {
        if (NULL == effect_ctrl.user_arry[i])
        {
            effect_ctrl.user_arry[i] = user;
            break;
        }
    }
    
	effect_ctrl.user_count++;
    ak_print_notice_ex( "effect_ctrl.user_count=%d\n", effect_ctrl.user_count);
    *vqe_handle_id = i;
    
    ak_print_notice_ex( "vqe_handle_id=%d--%p\n", *vqe_handle_id, effect_ctrl.user_arry[*vqe_handle_id]);
    
    ret = AK_SUCCESS;

effect_open_end:
	
	pthread_rwlock_unlock(&effect_ctrl.user_mutex);
    
    ak_print_normal_ex( "ak_vqe_open exit \n");
    return ret;    
}

/**
 * ak_vqe_set_res_attr - voice quality enhancement module set resample attribute
 * @vqe_handle_id[IN]: vqe module opened handle id
 * @pcm_attr_in[IN]: resample input, audio data attribute
 * @pcm_attr_out[IN]: resample output, audio data attribute
 * return: 0 success, other failed
 * notes: 
 */
int ak_vqe_set_res_attr(int vqe_handle_id, 
                           struct pcm_param *pcm_attr_in, 
                           struct pcm_param *pcm_attr_out)
{
    int ret = check_resample_attr_param(pcm_attr_in, pcm_attr_out);
    if (ret)
        return ret;

	pthread_rwlock_rdlock(&effect_ctrl.user_mutex);
    ak_print_normal_ex( "entry...\n");

    struct effect_user *user = get_effect_user(vqe_handle_id);
    if (!user)
    {
        ak_print_error_ex( "user is NULL\n");
        ret = ERROR_VQE_USER_NULL;
        goto resample_attr_end;
    }

	T_AUDIO_FILTER_INPUT s_ininfo;
	memset(&s_ininfo, 0, sizeof(T_AUDIO_FILTER_INPUT));

	s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)filter_alloc;
	s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)free;
	s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;

	s_ininfo.m_info.m_BitsPerSample = pcm_attr_in->sample_bits;
	s_ininfo.m_info.m_Channels = pcm_attr_in->channel_num;
	s_ininfo.m_info.m_SampleRate = pcm_attr_in->sample_rate;

	s_ininfo.chip = AUDIOLIB_CHIP_AK39XXEV3;
	s_ininfo.strVersion = (char *)AUDIO_FILTER_VERSION_STRING;

	s_ininfo.m_info.m_Type = _SD_FILTER_RESAMPLE;
	s_ininfo.m_info.m_Private.m_resample.maxinputlen = 0;
	s_ininfo.m_info.m_Private.m_resample.outSrindex = 0;
	s_ininfo.m_info.m_Private.m_resample.outSrFree = pcm_attr_out->sample_rate;
	s_ininfo.m_info.m_Private.m_resample.reSampleArithmetic = RESAMPLE_ARITHMETIC_1;
    //s_ininfo.ploginInfo = (T_VOID *)_SD_Resample_login(AK_NULL);

	user->lib_handle = _SD_Filter_Open(&s_ininfo);
    if (!user->lib_handle)
    {   // try again, maybe it is H3B
        s_ininfo.chip = AUDIOLIB_CHIP_AK39XXEV3;
        user->lib_handle = _SD_Filter_Open(&s_ininfo);
    }
        
    if (!user->lib_handle)
    {
        ak_print_error_ex( "filter lib error\n");
        del_effect_user(vqe_handle_id);
        ret = ERROR_VQE_OPEN_LIB_ERROR;
        goto resample_attr_end;
    }

    user->out_sample_rate = pcm_attr_out->sample_rate;
    user->user_attr_in.sample_rate = pcm_attr_in->sample_rate;
    user->user_attr_in.channel_num = pcm_attr_in->channel_num;
    user->user_attr_in.sample_bits = pcm_attr_in->sample_bits;
    
    ret = AK_SUCCESS;

resample_attr_end:
    
	pthread_rwlock_unlock(&effect_ctrl.user_mutex);
    ak_print_normal_ex( "exit...\n");
    return ret;
}

/**
 * ak_vqe_set_aslc_attr - voice quality enhancement module set aslc attribute
 * @vqe_handle_id[IN]: vqe module opened handle id
 * @pcm_attr_in[IN]: aslc input, audio data attribute 
 * @db[IN]: aslc volume value,  -200~70db
 * return: 0 success, other failed
 * notes: 
 */
int ak_vqe_set_aslc_attr(int vqe_handle_id, 
                             struct pcm_param *pcm_attr_in,
                             int db)
{
    int ret = check_audio_data_attr(pcm_attr_in);
    if (ret)
    {
        ak_print_error_ex( "check_audio_data_attr error\n");
        return ret;
    }

	pthread_rwlock_rdlock(&effect_ctrl.user_mutex);

    struct effect_user *user = get_effect_user(vqe_handle_id);
    if (!user)
    {
        ak_print_error_ex( "user is NULL\n");
        ret = ERROR_VQE_USER_NULL;
        goto aslc_attr_end;
    }

    if (user->user_attr_in.sample_rate != pcm_attr_in->sample_rate ||
        user->user_attr_in.sample_bits != pcm_attr_in->sample_bits ||
        user->user_attr_in.channel_num != pcm_attr_in->channel_num)
    {
        _SD_Filter_Close(user->lib_handle);
        
    	T_AUDIO_FILTER_INPUT s_ininfo;
    	memset(&s_ininfo, 0, sizeof(T_AUDIO_FILTER_INPUT));

    	s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)filter_alloc;
    	s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)free;
    	s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
    	s_ininfo.cb_fun.delay = AK_NULL;

    	s_ininfo.m_info.m_BitsPerSample = pcm_attr_in->sample_bits;
    	s_ininfo.m_info.m_Channels = pcm_attr_in->channel_num;
    	s_ininfo.m_info.m_SampleRate = pcm_attr_in->sample_rate;

        s_ininfo.chip = AUDIOLIB_CHIP_AK39XXEV3;       
    	s_ininfo.strVersion = (char *)AUDIO_FILTER_VERSION_STRING;

        s_ininfo.m_info.m_Type = _SD_FILTER_ASLC;
    	s_ininfo.m_info.m_Private.m_aslc.aslcEna = 1;
    	s_ininfo.m_info.m_Private.m_aslc.aslcLimitLevel = 32700;
    	s_ininfo.m_info.m_Private.m_aslc.aslcStartLevel = 20000;
    	s_ininfo.m_info.m_Private.m_aslc.jointChannels = 2;
        //s_ininfo.ploginInfo = (T_VOID *)_SD_ASLC_login(AK_NULL);
     
    	user->lib_handle = _SD_Filter_Open(&s_ininfo);
        if (!user->lib_handle)
        {
            ak_print_error_ex( "filter lib error\n");
            del_effect_user(vqe_handle_id);
            ret = ERROR_VQE_OPEN_LIB_ERROR;
            goto aslc_attr_end;
        }
        memcpy(&(user->user_attr_in), pcm_attr_in, sizeof(struct pcm_param));
    }
    
	if (set_da_aslc_param(user->lib_handle, db)) 
	{
		ak_print_error_ex( "aslc_set_param error, handle=%p\n",
                        user->lib_handle);
        _SD_Filter_Close(user->lib_handle);
        del_effect_user(vqe_handle_id);
        ret = ERROR_VQE_OPEN_LIB_ERROR;
        goto aslc_attr_end;
	}

    ak_print_error_ex( "user->lib_handle=%p\n", user->lib_handle);
    ret = AK_SUCCESS;
    
aslc_attr_end:
    
	pthread_rwlock_unlock(&effect_ctrl.user_mutex);
    
    return ret;
}

/**
 * ak_vqe_set_eq_attr - voice quality enhancement module set eq attribute
 * @vqe_handle_id[IN]: vqe module opened handle id
 * @pcm_attr_in[IN]: audio data attribute
 * @eq_attr[IN]: eq attribute
 * return: 0 success, other failed
 * notes: 
 */
int ak_vqe_set_eq_attr(int vqe_handle_id, 
                           struct pcm_param *pcm_attr_in, 
                           struct ak_audio_eq_attr *eq_attr)
{
    int ret = check_eq_attr_param(pcm_attr_in, eq_attr);
    if (ret)
        return ret;

	pthread_rwlock_rdlock(&effect_ctrl.user_mutex);
    
    struct effect_user *user = get_effect_user(vqe_handle_id);
    if (!user)
    {
        ak_print_error_ex( "user is NULL\n");
        ret = ERROR_VQE_USER_NULL;
        goto eq_attr_end;
    }
    
	T_AUDIO_FILTER_INPUT s_ininfo;
    int i = 0;
	memset(&s_ininfo, 0, sizeof(T_AUDIO_FILTER_INPUT));

	s_ininfo.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)filter_alloc;
	s_ininfo.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)free;
	s_ininfo.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;
	s_ininfo.cb_fun.delay = AK_NULL;

	s_ininfo.m_info.m_BitsPerSample = pcm_attr_in->sample_bits;
	s_ininfo.m_info.m_Channels = pcm_attr_in->channel_num;
	s_ininfo.m_info.m_SampleRate = pcm_attr_in->sample_rate;

    s_ininfo.chip = AUDIOLIB_CHIP_AK39XXEV3;
	s_ininfo.strVersion = (char *)AUDIO_FILTER_VERSION_STRING;
    
    s_ininfo.m_info.m_Type = _SD_FILTER_EQ;

	s_ininfo.m_info.m_Private.m_eq.eqmode = _SD_EQ_USER_DEFINE;//_SD_EQ_USER_DEFINE; //_SD_EQ_MODE_NORMAL; // mode;
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
    //s_ininfo.ploginInfo = (T_VOID *)_SD_EQ_login(AK_NULL);

	user->lib_handle = _SD_Filter_Open(&s_ininfo);
    if (!user->lib_handle)
    {
        ak_print_error_ex( "filter lib error\n");
        del_effect_user(vqe_handle_id);
        ret = ERROR_VQE_OPEN_LIB_ERROR;
        goto eq_attr_end;
    }
    
    ak_print_error_ex( "user->lib_handle=%p\n", user->lib_handle);
    ret = AK_SUCCESS;
    
eq_attr_end:
    
	pthread_rwlock_unlock(&effect_ctrl.user_mutex);
    return ret;
}

/**
 * ak_vqe_process - voice quality enhancement module deal with data
 * @vqe_handle_id[IN]: vqe opened handle id
 * @inbuf[IN]: audio origin input data
 * @inbuf_len[IN]: audio input buffer length
 * @outbuf[IN]: audio output data
 * @outbuf_len[IN]: audio output buffer length 
 * @ret_data_len[OUT]: output data length
 * return: 0 success, other failed
 * notes:
 */
int ak_vqe_process(int vqe_handle_id, 
                       unsigned char *inbuf, unsigned int inbuf_len, 
                       unsigned char **outbuf,
                       unsigned int *ret_data_len)
{
    int ret = check_effect_control_param(inbuf, inbuf_len, outbuf, ret_data_len);
    if (ret)
    {
        return ret;
    }
    *ret_data_len = 0;
    
	pthread_rwlock_wrlock(&effect_ctrl.user_mutex);
    
    struct effect_user *user = get_effect_user(vqe_handle_id);
    if (!user)
    {
        ak_print_error_ex( "user is NULL\n");
        ret = ERROR_VQE_USER_NULL;
        goto effect_control_end;
    }
    
    if (NULL != *outbuf && *outbuf != user->outbuf)
    {
        ak_print_error_ex( "outbuf addr wrong, *outbuf=%p, user->outbuf=%p\n",
                            *outbuf, user->outbuf);
        return ERROR_TYPE_POINTER_NULL;
    }

    if (!user->outbuf)
    {
        user->outbuf_len = get_outbuf_size(user, inbuf_len);
        if (0 == user->outbuf_len)
        {
            ak_print_error_ex( "outbuf_len = 0\n");
            ret = ERROR_VQE_OUTBUF_LEN_ERROR;
            goto effect_control_end;
        }
        user->outbuf = ak_calloc(1, user->outbuf_len);
        if (!user->outbuf)
        {
            ak_print_error_ex( "malloc user->outbuf failed\n");
            ret = ERROR_TYPE_POINTER_NULL;
            goto effect_control_end;
        }
    }
    
	T_AUDIO_FILTER_BUF_STRC fbuf_strc = {0};

	fbuf_strc.buf_in = inbuf;
	fbuf_strc.buf_out = user->outbuf;
	fbuf_strc.len_in = inbuf_len;
	fbuf_strc.len_out = user->outbuf_len;

	*ret_data_len = _SD_Filter_Control(user->lib_handle, &fbuf_strc);
    if (0 == *ret_data_len)
    {
        ak_print_error_ex( "_SD_Filter_Control\n");
        ret = ERROR_VQE_LIB_ERROR;
        goto effect_control_end;
    }
    
    *outbuf = user->outbuf;
    ret = AK_SUCCESS;
    
effect_control_end:   
    
	pthread_rwlock_unlock(&effect_ctrl.user_mutex);
    return ret;
}

/**
 * ak_vqe_close - voice quality enhancement module close 
 * @vqe_handle_id[IN]: vqe module hanle id
 * return: 0 success, other failed
 * notes:
 */
int ak_vqe_close(int vqe_handle_id)
{

    struct effect_user *user = get_effect_user(vqe_handle_id);
    if (!user)
    {
        ak_print_error_ex( "user is NULL\n");
        return ERROR_VQE_USER_NULL;
    }

    int ret = AK_SUCCESS;
	pthread_rwlock_wrlock(&effect_ctrl.user_mutex);
    
    if (0 >= effect_ctrl.user_count)
    {
        ak_print_error_ex( "no user\n");
        ret = ERROR_VQE_NO_USER;
        goto effect_close_end;
    }
            
	if (AK_TRUE != _SD_Filter_Close(user->lib_handle))
    {
        ak_print_error_ex( "_SD_Filter_Close failed\n");
        ret = ERROR_VQE_LIB_ERROR;
    }

    if (user->outbuf)
    {
        ak_free(user->outbuf);
        user->outbuf_len = 0;
    }
    
    del_effect_user(vqe_handle_id);
    
 effect_close_end:
    
	pthread_rwlock_unlock(&effect_ctrl.user_mutex);   
    ak_print_notice_ex( "ak_vqe_close, now count =%d--%p\n",
                        effect_ctrl.user_count, user->lib_handle);
    
    return ret;
}

