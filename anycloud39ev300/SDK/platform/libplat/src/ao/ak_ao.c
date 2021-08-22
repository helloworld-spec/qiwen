#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "pcm.h"
#include "internal_error.h"
#include "ak_thread.h"
#include "ak_ao.h"
#include "ao_ipcsrv.h"

#define PLAT_AO		                "<plat_ao>"

#define AK_AO_SAMPLE_RATE           (8000)

#define DAC_DEV_NAME				"/dev/akpcm_cdev0"
#define DAC_DEV_MAX_VOLUME			6	/* analog gain */
#define DAC_MAX_VOLUME				12	/* including digital gain */
#define DAC_MAX_ASLC_VOLUME			6	/* max aslc volume */

#define DEFAULT_DAC_PERIOD_BYTES	(4096)
#define DAC_SDFILTER_BUFFER_LENGTH 	(DEFAULT_DAC_PERIOD_BYTES * 2)

/* catch the pcm file before play */
#define AO_DATA_DEBUG 				0
/* debug ao status */
#define AO_STATUS_DEBUG 			0

enum dac_status {
	DAC_OPENED,
	DAC_CLOSED
};

struct pcm_dac_info {
	int fd;						/* open device handle */
	int users_count;			/* ao user count */
	unsigned int da_data_size;  /* data size that ao send to da */

	unsigned char *sdf_buf;		/* sdf buffer */
	int sdf_len;				/* sdf buffer len */
	unsigned char *aslc_buf;	/* aslc buffer */
	int aslc_len;				/* aslc buffer len */

	int volume;
	int dac_volume;				/* dac volume */
	int aslc_volume;			/* aslc volume */

	void *running;				/* ao start send frame flag */
	enum dac_status status;
	struct pcm_param param;
	struct ak_audio_eq_attr eq_attr;

	ak_mutex_t mutex;
	ak_mutex_t status_mutex;
    void *ao_handle;
};

static const char ao_version[] = "libplat_ao V1.2.02"; /* version number */

static struct pcm_dac_info pcm_dac = {
	.fd = -1,
	.users_count = 0,
	.sdf_buf = NULL,
	.sdf_len = 0,
	.volume = 0,
	.aslc_buf = NULL,
	.aslc_len = 0,
	.running = NULL,
	.status = DAC_CLOSED,
	.param = {0},
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.status_mutex = PTHREAD_MUTEX_INITIALIZER,
};

#if AO_DATA_DEBUG
static FILE *ao_fp = NULL;
static FILE *res_fp = NULL;

#endif
static int dac_dev_get_dev_status(int fd)
{
	int status = -1;

	/* get status from driver */
	if (ioctl(fd, IOC_GET_STATUS, (void *)&status) < 0) 
	{
		ak_print_error_ex( "dac ioctl IOC_GET_STATUS failed\n");
		return AK_FAILED;
	}
	return status;
}

static int dac_dev_set_param(int fd, struct akpcm_pars *dev_pars)
{
	if (AK_INVALID_FD == fd) 
	{
		ak_print_error_ex( "write fd error\n");
		return AK_FAILED;
	}

	/* driver pause */
	if (ioctl(fd, IOC_PAUSE, NULL)) 
	{
		ak_print_error_ex( "dac ioctl set pause failed.\n");
		return AK_FAILED;
	}

	void *pv = dev_pars;
	ak_print_notice_ex( "dac ioctl set sample: %d\n", dev_pars->rate);
	if (ioctl(fd, IOC_SET_PARS, pv)) 
	{
		ak_print_error( "dac ioctl set pars failed\n");
		return AK_FAILED;
	}

	if (ioctl(fd, IOC_PREPARE, NULL)) 
	{
		ak_print_error_ex( "send IOC_PREPARE failed\n");
		return AK_FAILED;
	}

	if (ioctl(fd, IOC_RESUME, NULL))
	{
		ak_print_error_ex( "send IOC_RESUME failed\n");
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

static void load_dac_default_param(pcm_user_t *user)
{
	user->param.rate = AK_AO_SAMPLE_RATE;
	user->param.channels = 2;
	user->actual_ch = 2;
	user->param.sample_bits = 16;
	user->param.period_bytes = DEFAULT_DAC_PERIOD_BYTES;
	user->param.periods = 12;
	user->param.threshold = user->param.period_bytes;

	user->ak_ft.gain = 5;
	user->ak_ft.dev = AKPCM_PLAYDEV_HP;

	user->sdfilter = NULL;
	user->sdf_switch = FUNC_DISABLE;
}

static void set_dac_status(enum dac_status status)
{
#if AUDIO_AEC_FUNC
	ak_thread_mutex_lock(&pcm_dac.status_mutex);
	pcm_dac.status = status;
	ak_thread_mutex_unlock(&pcm_dac.status_mutex);
#endif
}

/**
 * dac_set_source - set source of ao,use this function will stop mute
 * @user[IN]: pointer point to dac param
 * return: 0 success, -1 failed
 */
static int dac_set_source(pcm_user_t *user)
{
	int value = SOURCE_DAC;
	if (ioctl(user->cp_fd, IOC_SET_SOURCES, (void *)(&value)) < 0) {
		ak_print_error("dac ioctl set source failed\n");
		return AK_FAILED;
	}

	value = user->ak_ft.dev;
	if (ioctl(user->cp_fd, IOC_SET_DEV, (void *)(&value)) < 0) {
		ak_print_error("adc ioctl set dev failed\n");
		return AK_FAILED;
	}
	user->mute_flag = AK_FALSE;

	return AK_SUCCESS;
}

/**
 * start_dac_playing - driver start dac play
 * @user[IN]: pointer point to dac param
 * return: 0 success, -1 failed
 */
static int start_dac_playing(pcm_user_t *user)
{
    int ret = ioctl(user->cp_fd, IOC_RESUME, NULL);
	if (ret < 0) {
		ak_print_error_ex("send IOC_RESUME failed\n");
	}

	return ret;
}

/**
 * set_dac_param - set adc parameter
 * @user[IN]: pointer point to dac param
 * return: 0 success, -1 failed
 */
static int set_dac_param(pcm_user_t *user)
{
	/* driver pause */
	if (ioctl(user->cp_fd, IOC_PAUSE, NULL) < 0) {
		ak_print_error_ex("dac ioctl set pause failed.\n");
		return AK_FAILED;
	}

	void *pv = &(user->param);
	ak_print_notice_ex("dac ioctl set sample: %d\n", user->param.rate);
	if (ioctl(user->cp_fd, IOC_SET_PARS, pv) < 0) {
		ak_print_error("dac ioctl set pars failed\n");
		return AK_FAILED;
	}

	if (ioctl(user->cp_fd, IOC_PREPARE, NULL) < 0) {
		ak_print_error_ex("send IOC_PREPARE failed\n");
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

/**
 * set_param_to_dac_driver - set the DA device
 * @user[IN]: pointer point to dac param
 * return: 0 success, -1 failed
 */
static int set_param_to_dac_driver(pcm_user_t *user)
{
	if (!user->mute_flag && dac_set_source(user)) {
		return AK_FAILED;
	}

	if (set_dac_param(user)) {
        return AK_FAILED;
    }

	struct akpcm_features ft;
	if (ioctl(user->cp_fd, IOC_GET_FEATS, &ft) < 0) {
		ak_print_error("dac ioctl get features failed\n");
		return AK_FAILED;
	}

	ak_print_debug("dac ft sample_bits:%x\n", ft.sample_bits);
	ak_print_debug("dac ft rates:%x\n", ft.rates);
	ak_print_debug("dac ft rate_min:%x\n", ft.rate_min);
	ak_print_debug("dac ft rate_max:%x\n", ft.rate_max);
	ak_print_debug("dac ft period_bytes_max:%x\n", ft.period_bytes_max);
	ak_print_debug("dac ft periods_min:%x\n", ft.periods_min);

	/* save actual samplerate for sdfilter */
	user->actual_rate = ft.actual_rate;
	ak_print_debug("dac actual_rate:%d\n", ft.actual_rate);

	struct akpcm_pars pars;
	if (ioctl(user->cp_fd, IOC_GET_PARS, &pars) < 0) {
		ak_print_error("dac ioctl get pars failed\n");
		return AK_FAILED;
	}

	ak_print_debug("dac rate:%d\n", pars.rate);
	ak_print_debug("dac channels:%d\n", pars.channels);
	ak_print_debug("dac sample_bits:%d\n", pars.sample_bits);
	ak_print_debug("dac period_bytes:%d\n", pars.period_bytes);
	ak_print_debug("dac periods:%d\n", pars.periods);
	ak_print_debug("dac threshold:%d\n", pars.threshold);

    int value = 0;

	/* get source */
	if (ioctl(user->cp_fd, IOC_GET_SOURCES, (void *)&value) < 0) {
		ak_print_error("dac get sources failed\n");
		return AK_FAILED;
	}
	ak_print_debug("dac get sources: %x\n", value);

	/* get volume gain */
	if (ioctl(user->cp_fd, IOC_GET_GAIN, (void *)&value) < 0) {
		ak_print_error("dac get gain failed\n");
		return AK_FAILED;
	}
	ak_print_debug("dac get gain: %x\n", value);

	return AK_SUCCESS;
}

/**
 * set_user_use_dac - set the current user used dac
 * @user[IN]: pointer point to dac param
 * return: 0 success, -1 failed
 */
static int set_user_use_dac(pcm_user_t *user)
{
	if (pcm_dac.running != (void *)user) {
		if (set_param_to_dac_driver(user) < 0) {
			ak_print_error("write set dac parameters error\n");
			return AK_FAILED;
		} else {
			ak_print_debug("write set dac parameters ok\n");
		}

		pcm_dac.running = (void *)user;

		/* open resample */
		if (!user->sdfilter && user->sdf_switch) {
			user->input_len = 0;

			user->sdfilter = sdfilter_open_resample(user, user->param.rate,
			    user->actual_rate);
			if (!user->sdfilter) {
				ak_print_error_ex("open sdfilter failed\n");
				return AK_FAILED;
			}
			ak_print_debug("dac actual rate:%d, desr rate:%d\n",
				user->actual_rate, user->param.rate);
		}
	}

	return AK_SUCCESS;
}

/**
 * set_da_aslc_param - set aslc parameter
 * @handle[IN]: aslc open handle 
 * @level[IN]: volume level
 * return: 0 success, -1 failed
 */
static int set_da_aslc_param(void *handler, int level)
{
	if (level< 0 || level > 6) {
		level= 0;
	}

	int ret = AK_FAILED;

	if (handler) {
		/* 0,3,6,...,18 dB magnification times,means 6~12 level */
		/* vol=[0:3:18] */

		/* round(32768 * 10.^((st2+vol)/20)) */
		static const int stone2_y[] = {328,463,654,924,1305,1843,2603};
		/* round(32768 * 10.^((-vol+maxvol)/20)) */
		static const int stone3_x[] = {26029,18427,13045,9235,6538,4629,3277};

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
		tmile.stone[2].y = stone2_y[level];
		tmile.stone[3].x = stone3_x[level];
		tmile.stone[3].y = DA_STONE3_Y;
		tmile.stone[4].x = DA_STONE4_X;
		tmile.stone[4].y = DA_STONE4_Y;

		if (AK_TRUE == _SD_Filter_SetAslcMileStones(handler, &tmile))
			ret = AK_SUCCESS;
	}

	return ret;
}

/**
 * dac_set_aslc - set aslc volume
 * @user[IN]: pointer point to dac param
 * @volume[IN]: volume level
 * return: 0 success, -1 failed
 */
static int dac_set_aslc(pcm_user_t *user, int volume)
{
	if (!user) {
		ak_print_error_ex( "user is null\n" );
		return AK_FAILED;
	}
	if (!user->sdfilter_aslc) {
    	struct pcm_param pcm_attr_in;
        pcm_attr_in.sample_rate = user->param.rate;
        pcm_attr_in.sample_bits = user->param.sample_bits;
        pcm_attr_in.channel_num = user->param.channels;
        
		user->sdfilter_aslc = sdfilter_open(&pcm_attr_in, _SD_FILTER_ASLC, 0);
		if (NULL == user->sdfilter_aslc) {
			ak_print_error_ex("open aslc failed\n");
			return AK_FAILED;
		}
	}

	/* volume_level is the filter lib volume level */
	int volume_level = 0;
	ak_print_normal_ex("aslc volume = %d\n", volume);

	switch (volume) {
	case 0:
		volume_level = 0;
		break;
	case 1:
		volume_level = 1;
		break;
	case 2:
		volume_level = 2;
		break;
	case 3:
		volume_level = 3;
		break;
	case 4:
		volume_level = 4;
		break;
	case 5:
		volume_level = 5;
		break;
	case 6:
		volume_level = 6;
		break;
	default:
		volume_level = 0;
		break;
	}

	int ret = set_da_aslc_param(user->sdfilter_aslc, volume_level);
	if (ret) {
		ak_print_error_ex( "sdfilter_aslc_set_param error\n" );
	}

	return ret;
}

/**
 * set_dac_mute - set aslc mute
 * @user[IN]: pointer point to dac param
 * return: 0 success, -1 failed
 */
static int set_dac_mute(pcm_user_t *user)
{
	int value = SIGNAL_SRC_MUTE;
	int ret = ioctl(user->cp_fd, IOC_SET_SOURCES, (void *)(&value));

	if (ret) {
		ak_print_error_ex("DAC ioctl set MUTE failed\n");
	} else {
		user->mute_flag = AK_TRUE;
	}

	return ret;
}

static unsigned char* realloc_buffer(unsigned char *in_buf, int data_len)
{
	/* free previous exist buffer */
	if (in_buf) {
		ak_free(in_buf);
		in_buf = NULL;
	}

	unsigned char *data_buf = (unsigned char *)ak_calloc(1, data_len);
	if (!data_buf) {
		ak_print_error_ex("realloc buffer failed\n");
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
	}

	return data_buf;
}

/**
 * get_sdf_len - calculate the buffer length of resample buffer
 * @input_len[IN]: input data length
 * @actual_rate[IN]: actual sample rate
 * @sample_rate[IN]: the samlpe rate have to set
 * return: resample buffer length
 */
static int get_sdf_len(int input_len, int actual_rate, int sample_rate)
{
	int tmp = (input_len * actual_rate);
	int sdf_len = (tmp / sample_rate);
	if (tmp % sample_rate) {
		sdf_len += 2;
	}

	return sdf_len;
}

static void init_eq_param(void)
{
	pcm_dac.eq_attr.pre_gain = 0;
	pcm_dac.eq_attr.bands = 0;	
		
	pcm_dac.eq_attr.bandfreqs[0] = 0;
	pcm_dac.eq_attr.bandgains[0] = 0;
	pcm_dac.eq_attr.bandQ[0] = 0;
	pcm_dac.eq_attr.band_types[0] = 0;

	pcm_dac.eq_attr.smoothEna = 0;
	pcm_dac.eq_attr.smoothTime = 0;
	pcm_dac.eq_attr.dcRmEna = 0;
	pcm_dac.eq_attr.dcfb = 0;
	
	pcm_dac.eq_attr.aslc_ena = 0;
	pcm_dac.eq_attr.aslc_level_max = 0;	
}

/**
 * write_dac_driver - write to the DA device
 * @ao_handle[IN]: DA device handle
 * @from[IN]: write data from the buffer
 * @size[IN]: read length
 * return: write data length, -1 failed
 */
static int write_dac_driver(void *ao_handle,
							unsigned char *from,
							unsigned int size)
{
	unsigned char *write_buf = NULL;
	pcm_user_t *user = (pcm_user_t *)ao_handle;
	int write_len = 0;
	int print = 1;

#if AO_DATA_DEBUG
        if (ao_fp)
            fwrite(from, 1, size, ao_fp);
        
#endif

	ao_save_stream_to_file(from, size, AO_ORIGIN_LEVEL);

	/* do resample */
	if (FUNC_ENABLE == user->sdf_switch && NULL != user->sdfilter) {
		/* use resample,the buffer length will change */
		int use_len = get_sdf_len(size, user->actual_rate, user->param.rate);
		if (use_len > pcm_dac.sdf_len) {
			pcm_dac.sdf_buf = realloc_buffer(pcm_dac.sdf_buf, use_len);
			pcm_dac.sdf_len = use_len;
		}

		if (!pcm_dac.sdf_buf) {
			ak_print_error_ex("pcm_dac.sdf_buf is NULL\n");
			return AK_FAILED;
		}

		/* get resample data */
		int sdf_len = sdfilter_control(user->sdfilter, from,
			size, pcm_dac.sdf_buf, pcm_dac.sdf_len);

		write_buf = pcm_dac.sdf_buf;
		write_len = sdf_len;

		ao_save_stream_to_file(write_buf, write_len, AO_RESAMPLE_LEVEL);
	} else {
		write_buf = from;
		write_len = size;
	}

	if (user->sdfilter_eq) {
		if (print) {
			ak_print_info_ex("user->sdfilter_eq\n");
			print = 0;
		}
			
		sdfilter_control(user->sdfilter_eq, write_buf, write_len, write_buf, write_len);
		ao_save_stream_to_file(write_buf, write_len, AO_EQ_LEVEL);
	}

	/* if volume is 7~12,do aslc */
	if (user->sdfilter_aslc) {
		if (write_len > pcm_dac.aslc_len) {
			pcm_dac.aslc_buf = realloc_buffer(pcm_dac.aslc_buf, write_len);
			pcm_dac.aslc_len = write_len;
		}

		if (!pcm_dac.aslc_buf) {
			ak_print_error_ex("pcm_dac.aslc_buf is NULL\n");
			return AK_FAILED;
		}

		int aslc_len = sdfilter_control(user->sdfilter_aslc,
			write_buf, write_len, pcm_dac.aslc_buf, pcm_dac.aslc_len);
		if (0 == aslc_len) {
			return 0;
		}
		write_buf = pcm_dac.aslc_buf;
		write_len = aslc_len;
		ao_save_stream_to_file(write_buf, write_len, AO_ASLC_LEVEL);
	}



	if (!write_buf) {
		ak_print_error_ex("write_buf is NULL\n");
		return AK_FAILED;
	}
	if (write_len <= 0) {
		ak_print_error_ex("write_len <= 0\n");
		return AK_FAILED;
	}

	ao_save_stream_to_file(write_buf, write_len, AO_FINAL_LEVEL);
#if AO_DATA_DEBUG
    if (res_fp)
        fwrite(write_buf, 1, write_len, res_fp);
#endif
	int ret_len = 0;
	int offset = 0;

	/* write the data to driver and play */
	while(write_len > 0) {
		ret_len = write(user->cp_fd, &write_buf[offset], write_len);
		if (ret_len < 0) {
			ak_print_error_ex("write error\n");
			return AK_FAILED;
		}

		offset += ret_len;
		write_len -= ret_len;
	}

	return offset;
}

/**
 * pcm_dac_open	- open the DA device
 * @void
 * return: opened DA device handle, NULL failed
 */
static void* pcm_dac_open(void)
{
	pcm_user_t *user = NULL;

	ak_thread_mutex_lock(&pcm_dac.mutex);
	if (AK_INVALID_FD == pcm_dac.fd) {
		pcm_dac.fd = open(DAC_DEV_NAME, O_WRONLY);
		if (AK_INVALID_FD == pcm_dac.fd) {
			ak_print_error("open %s failed: %s\n",
			    DAC_DEV_NAME, strerror(errno));
			goto dac_open_end;
		}

		if (-1 == fcntl(pcm_dac.fd, F_SETFD, FD_CLOEXEC)) {
			ak_print_error_ex("error:%s\n", strerror(errno));
		}
		set_dac_status(DAC_OPENED);
	}

	user = (pcm_user_t *)ak_calloc(1, sizeof(pcm_user_t));
	if (!user) {
		ak_print_error_ex("calloc error\n");
		if (AK_INVALID_FD != pcm_dac.fd) {
			close(pcm_dac.fd);
			pcm_dac.fd = AK_INVALID_FD;
			set_dac_status(DAC_CLOSED);
		}

		goto dac_open_end;
	}

	/* now set flags & parameters */
	pcm_dac.users_count++;
	pcm_dac.da_data_size = 0;
	user->cp_fd = pcm_dac.fd;

	/* set status as AO_PLAY_STATUS_READY when open */
	user->status = AO_PLAY_STATUS_READY;
	load_dac_default_param(user);

dac_open_end:
	ak_thread_mutex_unlock(&pcm_dac.mutex);
	return user;
}

static void set_open_param(void *ao_handle, const struct pcm_param *param)
{
	pcm_user_t *user = (pcm_user_t *)ao_handle;

    ak_thread_mutex_lock(&pcm_dac.mutex);
	user->param.rate = param->sample_rate;
	user->param.sample_bits = param->sample_bits;
	user->param.channels = AUDIO_CHANNEL_STEREO;
    ak_thread_mutex_unlock(&pcm_dac.mutex);
}

/**
 * pcm_out_write - write to the DA device
 * @ao_handle[IN]: DA device handle
 * @from[in]: write data from the buffer
 * @size[in]: write length
 * return: real sent data len, -1 failed
 */
static int pcm_dac_write(void *ao_handle, unsigned char *from, unsigned int size)
{
	if (NULL == ao_handle) {
		ak_print_error_ex("ao_handle is NULL.\n");
		return AK_FAILED;
	}
	if (NULL == from) {
		ak_print_error_ex("from is NULL.\n");
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)ao_handle;
	if (AK_INVALID_FD == user->cp_fd) {
		ak_print_error_ex("write fd error\n");
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	if (user->cp_fd == pcm_dac.fd) {
		ak_thread_mutex_lock(&pcm_dac.mutex);
		ret = set_user_use_dac(user);
		if (ret) {
			ak_print_error_ex("set_user_use_dac failed\n");
		} else {
			ret = write_dac_driver(user, from, size);
			if (AK_FAILED == ret) {
				ak_print_error_ex("write_dac_driver failed\n");
			} else {
			    ak_ao_get_play_status(ao_handle);
			    pcm_dac.da_data_size += ret;

#if AO_STATUS_DEBUG
			    ak_print_normal_ex("ret=%d, da_data_size=%d\n",
			        ret, pcm_dac.da_data_size);
#endif
			    ret = AK_SUCCESS;
			}
		}
		ak_thread_mutex_unlock(&pcm_dac.mutex);
	} else {
		ak_print_error_ex("write dev type error\n");
	}

	return ret;
}

/**
 * pcm_dac_close - close to the DA virtual device
 * @user[in]: DA user handle
 * return: 0 success, -1 failed
 */
static int pcm_dac_close(pcm_user_t *user)
{
	ak_thread_mutex_lock(&pcm_dac.mutex);
	pcm_dac.users_count--;
	if (0 == pcm_dac.users_count) {
		close(pcm_dac.fd);
		pcm_dac.fd = AK_INVALID_FD;
		pcm_dac.running = NULL;

		set_dac_status(DAC_CLOSED);
		if (pcm_dac.sdf_buf) {
			ak_free(pcm_dac.sdf_buf);
			pcm_dac.sdf_buf = NULL;
		}
		if (pcm_dac.aslc_buf) {
			ak_free(pcm_dac.aslc_buf);
			pcm_dac.aslc_buf = NULL;
		}
	}

    if (pcm_dac.running == (void *)user) {
		pcm_dac.running = NULL;
    }

	if (NULL != user->sdfilter) {
		sdfilter_close(user->sdfilter);
		user->sdfilter = NULL;
	}

	if (NULL != user->sdfilter_aslc) {
		sdfilter_close(user->sdfilter_aslc);
		user->sdfilter_aslc = NULL;
	}

	ak_thread_mutex_unlock(&pcm_dac.mutex);

	if (0 == pcm_dac.users_count) {
		ak_thread_mutex_destroy(&pcm_dac.mutex);
	}

	return 0;
}

static int pcm_dac_set_volume(pcm_user_t *user, int volume)
{
	/* DAC driver value 0 match gain 20dbm */
	ak_print_notice_ex("set DEV volume %d\n", volume);
	if (ioctl(user->cp_fd, IOC_SET_GAIN, (void *)(&volume)) < 0) {
		ak_print_error_ex("dac ioctl set gain failed\n");
		return AK_FAILED;
	}

	user->ak_ft.gain = volume;
	return AK_SUCCESS;
}

static int check_dev_buf_size_param(int dev_buf_size)
{
	if (AK_AUDIO_DEV_BUF_SIZE_512 != dev_buf_size &&
		AK_AUDIO_DEV_BUF_SIZE_1024 != dev_buf_size &&
		AK_AUDIO_DEV_BUF_SIZE_2048 != dev_buf_size &&
		AK_AUDIO_DEV_BUF_SIZE_3072 != dev_buf_size &&
		AK_AUDIO_DEV_BUF_SIZE_4096 != dev_buf_size)
	{
		ak_print_error_ex( "dev_buf_size not support %d\n", dev_buf_size);
		return ERROR_TYPE_INVALID_ARG;
	}

	return AK_SUCCESS;
}

static int check_eq_status(pcm_user_t *user, int enable)
{
    if ((user->eq_attr && enable) || (NULL == user->eq_attr && !enable))
    {
		ak_print_warning_ex("already set eq status, enable=%d\n", enable);
        return AK_FAILED;
    }
    else
    {
        return AK_SUCCESS;
    }
}


/**
 * ak_ao_get_version - get audio out version
 * return: version string
 * notes:
 */
const char* ak_ao_get_version(void)
{
	return ao_version;
}

/**
 * ak_ao_open - open audio out device, DA
 * @param[IN]: open DA param
 * return: opened ao_handle, otherwize NULL
 * notes:
 */
void* ak_ao_open(const struct pcm_param *param)
{
	ak_print_normal_ex("ao version: %s\n", ao_version);

    if (!param) {
        set_error_no(ERROR_TYPE_POINTER_NULL);
		return NULL;
    }

	if (param->sample_rate < 8000 || param->sample_rate > 64000) {
		/* sample rate not support  */
		ak_print_error_ex("sample rate not suppprt %d \n", param->sample_rate);
		return NULL;
	}

	if (0 == pcm_dac.users_count) {
		pcm_dac.sdf_len = DAC_SDFILTER_BUFFER_LENGTH;
		pcm_dac.sdf_buf = (unsigned char *)ak_calloc(1, pcm_dac.sdf_len);
		if (!pcm_dac.sdf_buf) {
			ak_print_error_ex("calloc dac sdf buffer failed\n");
			set_error_no(ERROR_TYPE_MALLOC_FAILED);
			return NULL;
		}

		pcm_dac.aslc_len = DAC_SDFILTER_BUFFER_LENGTH;
		pcm_dac.aslc_buf = (unsigned char *)ak_calloc(1, pcm_dac.aslc_len);
		if (!pcm_dac.aslc_buf) {
			ak_free(pcm_dac.sdf_buf);
			pcm_dac.sdf_buf = NULL;

			ak_print_error_ex("calloc dac aslc buffer failed\n");
			set_error_no(ERROR_TYPE_MALLOC_FAILED);
			return NULL;
		}
	}

	/* open dac */
	void *ao_handle = pcm_dac_open();
	if(ao_handle){
	    set_open_param(ao_handle, param);

	    /* save ao param */
    	pcm_dac.param.sample_rate = param->sample_rate;
    	pcm_dac.param.sample_bits = param->sample_bits;
    	pcm_dac.param.channel_num = param->channel_num;

		init_eq_param();

	} else {
		if (0 == pcm_dac.users_count) {
			if (pcm_dac.sdf_buf) {
				ak_free(pcm_dac.sdf_buf);
				pcm_dac.sdf_buf = NULL;
			}
			if (pcm_dac.aslc_buf) {
				ak_free(pcm_dac.aslc_buf);
				pcm_dac.aslc_buf = NULL;
			}
		}
	}

	ao_sys_ipc_register();
	ao_sysipc_bind_handle(ao_handle);

 #if AO_DATA_DEBUG
	ao_fp = fopen("/media/ao_data.pcm", "w+");
	if (!ao_fp) {
		ak_print_error_ex("open ao debug file error!\n");
	}

	res_fp = fopen("/media/res_data.pcm", "w+");
	if (!res_fp) {
		ak_print_error_ex("open res debug file error!\n");
	}
 #endif
    pcm_dac.ao_handle = ao_handle;
    return ao_handle;
}

/**
 * ak_ao_get_params - get ao parameter
 * @handle[IN]: audio out opened handle
 * @param[OUT]: store ao params delivery by ao_open();
 * return: 0 success, -1 failed
 * notes: call after ao_open
 */
int ak_ao_get_params(void *ao_handle, struct pcm_param *param)
{
	/* param check */
	if(!ao_handle || !param) {
		ak_print_error_ex("invalid param\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	/* value assingment */
	param->sample_rate = pcm_dac.param.sample_rate;
	param->sample_bits = pcm_dac.param.sample_bits;
	param->channel_num = pcm_dac.param.channel_num;

	return AK_SUCCESS;
}

/**
 * ak_ao_get_handle - get ao handle
 * @dev_id[IN]: audio out device id
 * @ao_handle[OUT]: audio out opened handle
 * return: 0 success, -1 failed
 * notes: 
 */
int ak_ao_get_handle(int dev_id, void **ao_handle)
{
	if (!pcm_dac.ao_handle) {
		ak_print_normal("pcm_dac.ao_handle is NULL\n");
		return AK_FAILED;
	}
    *ao_handle = pcm_dac.ao_handle;
    return AK_SUCCESS;
}

/**
 * ak_ao_send_frame - send frame to DA device
 * @ao_handle[IN]: audio out opened handle
 * @data[IN]: audio pcm data
 * @len[IN]: audio pcm data len
 * @ms[IN]: <0 block mode; =0 non-block mode; >0 wait time
 * return: real sent data len, -1 failed
 * notes:
 */
int ak_ao_send_frame(void *ao_handle, unsigned char *data, int len, long ms)
{
	return pcm_dac_write(ao_handle, data, len);
}

/**
 * ak_ao_notice_frame_end - notice send frame end
 * @ao_handle[IN]: audio out opened handle
 * return: 0 success; -1 failed
 * note: call this function after send frame OK.
 * notes:
 */
int ak_ao_notice_frame_end(void *ao_handle)
{
	if (!ao_handle) {
		ak_print_error_ex("pointer NULL\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)ao_handle;
	if (user->cp_fd != pcm_dac.fd) {
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;

	if (ioctl(pcm_dac.fd, IOC_NOTICE_END, NULL) < 0) {
		ak_print_error_ex("dac ioctl set NOTICE_END failed.\n");
		ret = AK_FAILED;
	}

	return ret;
}

/**
 * ak_ao_clear_frame_buffer -
 * @ao_handle[IN]: audio out opened handle
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ao_clear_frame_buffer(void *ao_handle)
{
	if (!ao_handle) {
		ak_print_error_ex("pointer NULL\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)ao_handle;
	if (user->cp_fd != pcm_dac.fd) {
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;

	if (ioctl(pcm_dac.fd, IOC_RSTBUF, NULL) < 0) {
		ak_print_error_ex("dac ioctl set RSTBUF failed.\n");
		ret = AK_FAILED;
	}

	return ret;
}

/**
 * ak_ao_get_play_status - get audio play status, send to DA
 * @ao_handle[in]: opened audio output handle
 * return: current audio play status
 */
enum ao_play_status ak_ao_get_play_status(void *ao_handle)
{
//	pcm_user_t *user = (pcm_user_t *)ao_handle;

	if(NULL == ao_handle){
        return AO_PLAY_STATUS_RESERVED;
    }

    pcm_user_t *user = (pcm_user_t *)ao_handle;
    int status = -1;

    /* get status from driver */
    if (ioctl(pcm_dac.fd, IOC_GET_STATUS, (void *)&status) < 0) {
        ak_print_error_ex("dac ioctl IOC_GET_STATUS failed\n");
    }

#if AO_STATUS_DEBUG
    ak_print_normal_ex("user->status=%d, status=%d, da_data_size=%d\n",
        user->status, status, pcm_dac.da_data_size);
#endif

    switch (user->status) {
    case AO_PLAY_STATUS_READY:
        if (status) {
            start_dac_playing(user);
            user->status = AO_PLAY_STATUS_PLAYING;
        } else {
            if (pcm_dac.da_data_size < user->param.threshold) {
                user->status = AO_PLAY_STATUS_DATA_NOT_ENOUGH;
            }
        }
        break;
    case AO_PLAY_STATUS_PLAYING:
        if (!status) {
            user->status = AO_PLAY_STATUS_FINISHED;
        }
        break;
    case AO_PLAY_STATUS_FINISHED:
    case AO_PLAY_STATUS_DATA_NOT_ENOUGH:
        if (status) {
            start_dac_playing(user);
            user->status = AO_PLAY_STATUS_PLAYING;
        }
        break;
    default:
        break;
    }

#if AO_STATUS_DEBUG
    ak_print_normal_ex("user->status=%d, status=%d, da_data_size=%d\n\n",
        user->status, status, pcm_dac.da_data_size);
#endif
	return user->status;
}

/**
 * ak_ao_set_play_status - set audio play status
 * @ao_handle[in]: opened audio output handle
 * @status[in]: new status
 * return: 0 success; -1 failed
 */
int ak_ao_set_play_status(void *ao_handle, enum ao_play_status status)
{
	if(NULL == ao_handle){
        return AK_FAILED;
    }

	pcm_user_t *user = (pcm_user_t *)ao_handle;
	user->status = status;
	if (AO_PLAY_STATUS_READY == user->status) {
		pcm_dac.da_data_size = 0;
		ak_print_normal_ex("pcm_dac.da_data_size=%d, threshold=%d\n",
				pcm_dac.da_data_size, user->param.threshold);
	    ak_print_normal_ex("set ao play status to READDY\n");
	}

	return AK_SUCCESS;
}


/**
 * ak_ao_set_dac_volume - set ao dac volume
 * @ao_handle[IN]: audio out opened handle
 * @volume[IN]: new volume value, [0, 6]: 0-mute, 6-dac max volume
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ao_set_dac_volume(void *ao_handle, int volume)
{
	if(NULL == ao_handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	if ((volume < 0) || (volume > DAC_DEV_MAX_VOLUME)) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	pcm_user_t *user = (pcm_user_t *)ao_handle;

	ak_print_normal_ex("set volume %d\n",volume);
	if (0 == volume) {
		ret = set_dac_mute(user);
		/* close aslc */
		if (NULL != user->sdfilter_aslc) {
			sdfilter_close(user->sdfilter_aslc);
			user->sdfilter_aslc = NULL;
		}
	} else {
		/* reopen the volume if mute */
		if (user->mute_flag && dac_set_source(user)) {
			return AK_FAILED;
		}

	    /* driver volume level = volume -1 */
		pcm_dac_set_volume(user, (volume - 1));
	}
	pcm_dac.dac_volume = volume;

	return ret;
}

/**
 * ak_ao_set_aslc_volume - set ao volume by aslc
 * @ao_handle[IN]: audio out opened handle
 * @volume[IN]: aslc volume value, [0, 6]: 0-not use aslc , 6-aslc max volume
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ao_set_aslc_volume(void *ao_handle, int volume)
{
	if(NULL == ao_handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	if ((volume < 0) || (volume > DAC_MAX_ASLC_VOLUME)) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	pcm_user_t *user = (pcm_user_t *)ao_handle;
	ret = dac_set_aslc(user, volume);
	if (ret) {
		ak_print_error_ex("set aslc volume error\n");
	} else {
		ak_print_normal_ex("set aslc volume %d \n", volume);
	}

	pcm_dac.aslc_volume = volume;

	return ret;
}

/**
 * ak_ao_get_volume - get volume
 * @handle[IN]: audio out opened handle
 * return: success return dac volume value, [0, 6]: 0-mute, 6-max dac volume,
 			-1 failed
 * notes:
 */
int ak_ao_get_dac_volume(void *handle)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	int volume = pcm_dac.dac_volume;
	if ((volume < 0) || (volume > DAC_DEV_MAX_VOLUME)) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	return volume;
}

/**
 * ak_ao_get_volume - get volume
 * @handle[IN]: audio out opened handle
 * return: success return volume value, [0, 6]: 0-not use aslc, 6-max aslc volume,
 			-1 failed
 * notes:
 */
int ak_ao_get_aslc_volume(void *handle)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	int volume = pcm_dac.aslc_volume;
	if ((volume < 0) || (volume > DAC_MAX_ASLC_VOLUME)) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	return volume;
}

/**
 * ak_ao_enable_eq - enable eq
 * @handle[IN]: audio out opened handle 
 * @enable[IN]: 1 enable,0 disable
 * return: 0 success -1 failed
 * notes:
 */
int ak_ao_enable_eq(void *handle, int enable)
{
	if (!handle) {
		ak_print_error_ex("handle is NULL\n");
		return AK_FAILED;
	}
    
	if (0 > enable || 1 < enable)
	{
		ak_print_error_ex("enable must be 0 or 1\n");
		return ERROR_TYPE_INVALID_ARG;
	}
        
	enum func_switch eq_on = enable ? FUNC_ENABLE : FUNC_DISABLE;

	pcm_user_t *user = (pcm_user_t *)handle;
    if (check_eq_status(user, enable))
    {
        return AK_SUCCESS;

    }

	if (eq_on) 
	{
		if (!user->sdfilter_eq) 
		{
	        if (NULL == user->eq_attr)
            {
                user->eq_attr = ak_calloc(1, sizeof(struct ak_audio_eq_attr));
                if (NULL == user->eq_attr)
                {
                    ak_print_error_ex("malloc dev->eq_attr  failed\n");
                    return AK_FAILED;
                }                
                memset(user->eq_attr, 0, sizeof(struct ak_audio_eq_attr));
            }
            
            struct pcm_param pcm_attr_in;
            pcm_attr_in.sample_rate = user->param.rate;
            pcm_attr_in.sample_bits = user->param.sample_bits;
            pcm_attr_in.channel_num = user->param.channels;
    
			user->sdfilter_eq = sdfilter_open_eq(&pcm_attr_in, _SD_EQ_USER_DEFINE, user->eq_attr);
			if (!user->sdfilter_eq) 
			{
				ak_print_error_ex("open eq failed \n");
				return AK_FAILED;
                if (user->eq_attr)
                {
                    ak_free(user->eq_attr);
                    user->eq_attr = NULL;
                }
				return AK_FAILED;
			} 
			else 
			{
				ak_print_notice_ex("enable eq \n");
			}
		}
	} 
	else 
	{
	    ak_print_notice_ex("disable eq \n");
		sdfilter_close(user->sdfilter_eq);
		user->sdfilter_eq = NULL;
        if (user->eq_attr)
        {
            ak_free(user->eq_attr);
            user->eq_attr = NULL;
        }
	}
    user->eq_attr->enable = enable;
	return AK_SUCCESS;
}

/**
 * ak_ao_set_eq_attr - set eq attribute
 * @handle[IN]: audio out opened handle 
 * @eq_attr[IN]: eq attribute
 * return: 0 success  -1 failed
 * notes:
 */
int ak_ao_set_eq_attr(void *handle, struct ak_audio_eq_attr *eq_attr)
{
	if (!handle) {
		ak_print_error_ex("handle is NULL\n");
		return AK_FAILED;
	}
	
	if (!eq_attr) {
		ak_print_error_ex("eq_attr is NULL\n");
		return AK_FAILED;
	}
	
	pcm_user_t *user = (pcm_user_t *)handle;
	if (user->sdfilter_eq) {
		sdfilter_close(user->sdfilter_eq);
	}

    struct pcm_param pcm_attr_in;
    pcm_attr_in.sample_rate = user->param.rate;
    pcm_attr_in.sample_bits = user->param.sample_bits;
    pcm_attr_in.channel_num = user->param.channels;
    
	user->sdfilter_eq = sdfilter_open_eq(&pcm_attr_in, _SD_EQ_USER_DEFINE, eq_attr);
	if (!user->sdfilter_eq) {
		ak_print_error_ex("open eq failed \n");
		return AK_FAILED;
	} else {
        if (NULL == user->eq_attr) {
            user->eq_attr = ak_calloc(1, sizeof(struct ak_audio_eq_attr));
            if (NULL == user->eq_attr) {
                ak_print_error_ex("malloc dev->eq_attr  failed\n");
                return AK_FAILED;
            }
        }
		memcpy(user->eq_attr, eq_attr, sizeof(struct ak_audio_eq_attr));
		ak_print_notice_ex("*****************set eq attribute success\n");
	}

	return AK_SUCCESS;
}

/**
 * ak_ao_get_eq_attr - get eq attribute
 * @handle[IN]: audio out opened handle 
 * @eq_attr[OUT]: eq attribute
 * return: 0 success  -1 failed
 * notes:
 */
int ak_ao_get_eq_attr(void *handle, struct ak_audio_eq_attr *eq_attr)
{
	if (!handle) {
		ak_print_error_ex("handle is NULL\n");
		return AK_FAILED;
	}

    if (!eq_attr) {
        ak_print_error_ex("eq_attr is NULL\n");
        return AK_FAILED;
    }
    
	pcm_user_t *user = (pcm_user_t *)handle;

    if (user->eq_attr) {
        memcpy(eq_attr, user->eq_attr, sizeof(struct ak_audio_eq_attr));
    } else {
        ak_print_error_ex("dev->eq_attr is NULL\n");
        return AK_FAILED;
    }
    ak_print_notice_ex("*****************get eq attribute success\n");

	return AK_SUCCESS;
}

/**
 * ak_ao_set_resample - set audio output resampling
 * @handle[IN]: audio out opened handle
 * @enable[IN]: 0 close, 1 open
 * return: 0 success, -1 failed
 */
int ak_ao_set_resample(void *ao_handle, int enable)
{
	/// 由于重采样会导致音频底噪问题，故此处去掉该功能。
//	if (!ao_handle) {
//		set_error_no(ERROR_TYPE_POINTER_NULL);
//		return AK_FAILED;
//	}
//
//	pcm_user_t *user = (pcm_user_t *)ao_handle;
//	user->sdf_switch = enable;

	return AK_SUCCESS;
}

/**
 * ak_ao_set_sample_rate - set new sample rate
 * @ao_handle[IN]: audio out opened handle
 * @sample_rate[IN]: ao sample rate.
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ao_set_dev_buf_size(void *ao_handle, enum ak_audio_dev_buf_size dev_buf_size)
{
	ak_print_normal_ex( "enter...\n");
	int ret = check_dev_buf_size_param(dev_buf_size);
	if (ret)
	{
		return AK_FAILED;
	}

	ret = AK_FAILED;
	if (!ao_handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	pcm_user_t *user = (pcm_user_t *)ao_handle;

	if (user->param.period_bytes == dev_buf_size)
	{
		ak_print_error_ex( "set playback_buf_size is the same !\n");
		ret = AK_FAILED;
		goto set_sample_rate_end;
	}

	if (dac_dev_get_dev_status(user->cp_fd))
	{
		ak_print_error_ex( "ao is playing, can not set buffer size\n");
		ret = AK_FAILED;
		goto set_sample_rate_end;
	}

	struct akpcm_pars temp_dev_pars;
	memcpy(&temp_dev_pars, &(user->param), sizeof(struct akpcm_pars));

	temp_dev_pars.period_bytes = dev_buf_size;
	temp_dev_pars.threshold = dev_buf_size;
	ret = dac_dev_set_param(user->cp_fd, &temp_dev_pars); 
	if (ret) /* change period_bytes not success */
	{
		ak_print_error_ex( "dac_dev_set_param failed\n");
	}
	else /* success, save the dev_buf_size */
	{
		user->param.period_bytes = dev_buf_size;
		user->param.threshold = dev_buf_size;
	}

set_sample_rate_end:
	
	return ret;

}

/**
 * ak_ao_get_dev_buf_size - get audio output device size
 * @ao_handle[IN]: audio out opened handle
 * @dev_buf_size[IN]: drvice DMA buffer size, [512, 4096], unit: byte
 * return: 0 success, -1 failed
 * notes: 1. if you do not call, the default frame size is 2048
 *      2. set drvice DMA buffer size before send frame.
 */
int ak_ao_get_dev_buf_size(void *ao_handle, int *dev_buf_size)
{
	ak_print_info_ex( "enter...\n");
	if (NULL == dev_buf_size)
	{
		ak_print_error_ex( "dev_buf_size is NULL\n");
		return AK_FAILED;
	}
	
	if (!ao_handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	pcm_user_t *user = (pcm_user_t *)ao_handle;
	*dev_buf_size = user->param.period_bytes;

	ak_print_info_ex( "leave...\n");
	return AK_SUCCESS;
}

/**
 * ak_ao_enable_speaker - enable/disable speaker
 * @ao_handle[IN]:
 * @enable[IN]: 0 disable speaker, others enable speaker.
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_ao_enable_speaker(void *ao_handle, int enable)
{
    if (!ao_handle) {
		ak_print_error_ex("pointer NULL\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;

	ak_thread_mutex_lock(&pcm_dac.mutex);
	if (AK_INVALID_FD != pcm_dac.fd) {
		ret = ioctl(pcm_dac.fd, IOC_SET_SPEAKER, (void *)&enable);
	}
	ak_thread_mutex_unlock(&pcm_dac.mutex);

	return ret;
}

/**
 * ak_ao_set_sample_rate - set new sample rate
 * @ao_handle[IN]: opened DA handle
 * @sample_rate[IN]: ao sample rate.
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ao_set_sample_rate(void *ao_handle, int sample_rate)
{
	if (!ao_handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)ao_handle;
	user->param.rate = sample_rate;

	return set_dac_param(user);
}

/**
 * ak_ao_get_runtime_status - get ao run time status
 * @handle[IN]: audio out opened handle
 * @status[OUT]: get ao runtime status :nr&agc resample
 * return: 0 success, -1 failed
 * notes: call after set all kind of DAC attr
 */
int ak_ao_get_runtime_status(void *handle,
							struct ao_runtime_status *status)
{
	if (!handle || !status) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	status->resample_enable = user->sdf_switch;
	status->actual_rate = user->actual_rate;

	return AK_SUCCESS;
}

/**
 * ak_ao_close - close audio output
 * @ao_handle[IN]: opened DA handle
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ao_close(void *ao_handle)
{
	ak_print_info_ex("enter...\n");
	if (!ao_handle) {
		ak_print_error_ex("pointer NULL\n");
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)ao_handle;
	if (AK_INVALID_FD == user->cp_fd) {
		ak_print_error_ex("fd error\n");
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	sdfilter_close(user->sdfilter_eq);
	user->sdfilter_eq = NULL;
		
	if (user->cp_fd == pcm_dac.fd) {
		ret = pcm_dac_close(user);
	}
	pcm_dac.ao_handle = NULL;
	ao_sysipc_unbind_handle();
	ao_sys_ipc_unregister();

	ak_free(user);
	user = NULL;
	ak_print_info_ex("leave\n");

#if AO_DATA_DEBUG
	if (ao_fp) {
		fclose(ao_fp);
		ao_fp = NULL;
	}
	if (res_fp) {
		fclose(res_fp);
		res_fp = NULL;
	}
#endif

	return ret;
}


