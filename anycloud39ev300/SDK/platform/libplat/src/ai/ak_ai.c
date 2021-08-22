#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "pcm.h"
#include "list.h"
#include "internal_error.h"

#include "ak_ring_buffer.h"
#include "ak_ai.h"
#include "ai_ipcsrv.h"

#define PLAT_AI		                    "<plat_ai>"

#define AK_AI_SAMPLE_RATE               (8000)

/* debug flag */
#define AI_READ_TS_DEBUG				0
#define AI_READ_PCM_DRV_DEBUG			0

#define ADC_DEV_NAME 					"/dev/akpcm_cdev1"
#define ADC_DEV_MAX_VOLUME				8	/* analog gain */
#define ADC_MAX_VOLUME					12	/* including digital gain */
#define ADC_MAX_ASLC_VOLUME				6	/* max aslc volume */

#define DEFAULT_ADC_PERIOD_BYTES		(2048)
#define ADC_SDFILTER_BUFFER_LENGTH 		(DEFAULT_ADC_PERIOD_BYTES * 2)

#define ADC_CAP_BUF_MIN_LEN 			(512)
#define ADC_CAP_BUF_MAX_LEN 			(2048)

/* we'll drop the first 360 ms audio data */
#define ADC_CAP_DROP_TIME 				360
/* we keep 5 seconds pcm frame */
#define AI_MAX_FRAME_TIME		        (5 * 1000)

struct pcm_frame_node {
    struct frame frame;			/* audio pcm frame */
    struct list_head list;
};

struct pcm_adc_info {
	unsigned char run_flag;		/* ADC run flag */
	unsigned char prepare_flag;	/* ADC has prepared flag */
	unsigned char capture_flag;	/* capture flag */
	unsigned char thread_flag;	/* capture thread flag */

	int fd;						/* open device handle */
	int dev_hw;
	int users_count;			/* ai user count */
	ak_pthread_t tid;			/* capture thread id */

	unsigned long long pre_frame_ts;/* previous audio frame timestamp */
	unsigned long long pre_sys_ts;	/* previous system timestamp from get_pcm_timer */

	unsigned long pcm_seq_no;	/* sequence no. for reading pcm data driver */
	unsigned long get_seq_no;	/* get pcm data sequence no */

	unsigned char *read_buf;	/* read adc driver origin buffer */
	unsigned int read_len;		/* origin buffer len */
	unsigned char *capture_buf;	/* data capture buffer */
	unsigned int capture_len;	/* data capture buffer len */
	unsigned char *sdf_buf;		/* sdf buffer */
	unsigned int sdf_len;		/* sdf buffer len */
	void *rb_handle;			/* capture data ring buffer */

	int adc_volume;			/* dac volume */
	int aslc_volume;		/* aslc volume */

	int drop_time;			/* drop the first audio data of the appointed time */
	int frame_interval;		/* audio frame interval time(ms) */
	int frame_size;			/* one frame data length */
	struct list_head frame_head;/* frame list head */
	struct ak_timeval frame_time;/* calculate get frame used time */
	struct ak_audio_eq_attr eq_attr;

	char set_agc_attr_flag;
	struct ak_audio_agc_attr agc_attr;
	char set_aec_attr_flag;
	struct ak_audio_aec_attr aec_attr;

	void *running;
	enum func_switch cur_aec_switch;/*  enable aec */
	enum func_switch nr_agc_switch;	/*  enable nr&agc */
	enum func_switch nr_switch;		/*  enable nr */
	enum func_switch nr_max_switch;	/*  enable nr max */
	ak_sem_t capture_sem;
	ak_mutex_t mutex;
	ak_mutex_t read_mutex;
    void *ai_handle;
    enum func_switch agc_switch;	/*  enable agc */

	char set_nr_attr_flag;
	struct ak_audio_nr_attr nr_attr;
};

static const char ai_version[] = "libplat_ai V1.3.06";
static struct pcm_adc_info pcm_adc = {0};

static int adc_dev_get_echo_attr(int user_fd, struct echo_lib_param *echo_param)
{
	int ret = AK_SUCCESS;
	if (ioctl(user_fd, IOC_GET_ECHO_PARAM, (void *)(echo_param)) < 0) {
		ak_print_error_ex("adc_dev_get_echo_attr failed\n");
		ret = AK_FAILED;
	}
	
	return ret;
}

static int adc_dev_set_echo_attr(int user_fd, struct echo_lib_param *echo_param)
{
	int ret = AK_SUCCESS;

	if (ioctl(user_fd, IOC_SET_ECHO_PARAM, (void *)(echo_param)) < 0) {
		ak_print_error_ex("adc_dev_set_echo_attr failed\n");
		ret = AK_FAILED;
	}
	return ret;
}

static int adc_dev_enable_eq(int user_fd, int enable)
{
	int value = enable > 0 ? 1 : 0;
	if (ioctl(user_fd, IOC_SET_AD_EQ, (void *)(&value)) < 0) 
	{
		ak_print_error_ex("adc ioctl IOC_SET_AD_EQ failed\n");
		return AK_FAILED;
	}
	return AK_SUCCESS;
}

static int adc_dev_set_eq_attr(int user_fd, void *eq_param)
{
	if (ioctl(user_fd, IOC_SET_AD_EQ_ATTR, (void *)eq_param) < 0) 
	{
		ak_print_error_ex("adc ioctl IOC_SET_AD_EQ_ATTR failed\n");
		return AK_FAILED;
	}
	return AK_SUCCESS;
}

static int get_actual_rate(pcm_user_t *user)
{
	struct akpcm_features ft;

	if (ioctl(user->cp_fd, IOC_GET_FEATS, &ft) < 0) {
		ak_print_error_ex("adc ioctl get features failed\n");
		return AK_FAILED;
	}

	ak_print_debug_ex("adc ft sample_bits:%x\n", ft.sample_bits);
	ak_print_debug_ex("adc ft rates:%x\n", ft.rates);
	ak_print_debug_ex("adc ft rate_min:%x\n", ft.rate_min);
	ak_print_debug_ex("adc ft rate_max:%x\n", ft.rate_max);
	ak_print_debug_ex("adc ft period_bytes_max:%x\n", ft.period_bytes_max);
	ak_print_debug_ex("adc ft periods_min:%x\n", ft.periods_min);

	/* save actual samplerate for sdfilter */
	user->actual_rate = ft.actual_rate;
	ak_print_debug_ex("adc actual_rate:%d\n", ft.actual_rate);

	return AK_SUCCESS;
}

static void load_adc_default_param(pcm_user_t *user)
{
	user->param.rate = AK_AI_SAMPLE_RATE;
	user->param.channels = AUDIO_CHANNEL_MONO;
	user->param.sample_bits = 16;
	user->param.period_bytes = DEFAULT_ADC_PERIOD_BYTES;
	user->param.periods = 16;
	user->param.threshold = user->param.period_bytes;

	user->ak_ft.gain = 5;
	user->ak_ft.dev = AKPCM_CPTRDEV_AUTO;

	user->sdfilter = NULL;
	user->sdf_switch = FUNC_DISABLE;
	user->actual_ch = AUDIO_CHANNEL_MONO;
}

/**
 * get_pcm_ts_interval - calculate audio data ts interval, unit ms
 * @user[IN]: audio in opened handle
 * @data_len[IN]: appointed audio data len
 * return: audio data ts interval
 */
static inline int get_pcm_ts_interval(pcm_user_t *user, int data_len)
{
	int sample_bytes = (user->param.sample_bits >> 3);
	int channel_num = user->param.channels;
	int sec_bytes = (user->param.rate * sample_bytes * channel_num);

	return ((1000 * data_len) / sec_bytes);
}

/**
 * get_pcm_timer - get adc time stamp, unit ms
 * @user[IN]: AD device opened handle
 * return: start timestamp of each audio read len frame
 */
static inline unsigned long long get_pcm_timer(pcm_user_t *user)
{
	unsigned long long ts = 0;

	/* audio end timestamp of read_len bytes data */
	ioctl(user->cp_fd, IOC_GETTIMER, &ts);
	int remain = ak_rb_get_data_len(pcm_adc.rb_handle);
	int interval = get_pcm_ts_interval(user, remain);

	/* per packages's start time */
	unsigned long long calc_ts = (ts - interval);
	unsigned long long diff_ts = (calc_ts - pcm_adc.pre_frame_ts);
    unsigned long long abs_ts = abs(diff_ts - pcm_adc.frame_interval);

	if ((pcm_adc.pre_frame_ts > 0) && (abs_ts > pcm_adc.frame_interval)) {
	    ak_print_warning_ex("timer ts=%llu, calc_ts=%llu, abs_ts=%llu, "
    	    "interval=%d, remain=%d\n",
    		ts, calc_ts, abs_ts, interval, remain);
	}

#if AI_READ_TS_DEBUG
    ak_print_normal_ex("timer ts=%llu, interval=%d, remain=%d\n",
		ts, interval, remain);
    ak_print_normal_ex("calc_ts=%llu, pre_ts=%llu\n",
		calc_ts, pcm_adc.pre_frame_ts);
	ak_print_normal_ex("diff_ts=%llu, abs_ts=%llu\n",
		diff_ts, abs_ts);
#endif

    /* we adjust audio ts if abs_ts < 10(ms) */
	if ((abs_ts > 0) && (abs_ts < 10)) {
        ts = (pcm_adc.pre_frame_ts + pcm_adc.frame_interval);
	} else {
	    ts = calc_ts;
	}

	return ts;
}

/**
 * get_pcm_data_len - get adc pcm data len, unit byte
 * @user_fd[IN]: AD device user fd
 * return: current pcm data len
 */
static inline unsigned long get_pcm_data_len(int user_fd)
{
	unsigned long data_len = 0;

	ioctl(user_fd, IOC_GET_DATA_LENGTH, &data_len);

	return data_len;
}

/**
 * pcm_set_source: set source,the audio source is mic or linein
 * @user[IN]: AD user handle
 * return: 0 success,-1 failed
 */
static int pcm_set_source(pcm_user_t *user)
{
	int value = AKPCM_CPTRDEV_AUTO;

	switch (user->ak_ft.dev) {
	case AKPCM_CPTRDEV_MIC:
		value = SOURCE_ADC_MIC;
		ak_print_notice_ex("set ai source to MIC\n");
		break;
	case AKPCM_CPTRDEV_LI:
		value = SOURCE_ADC_LINEIN;
		ak_print_notice_ex("set ai source to LineIn\n");
		break;
	default:
		value = SOURCE_ADC_MIC;
		break;
	}

	if (ioctl(user->cp_fd, IOC_SET_SOURCES, (void *)(&value)) < 0) {
		ak_print_error("adc ioctl set sources failed\n");
		return AK_FAILED;
	}

	value = user->ak_ft.dev;
	if (ioctl(user->cp_fd, IOC_SET_DEV, (void *)(&value)) < 0) {
		ak_print_error("adc ioctl set dev failed\n");
		return AK_FAILED;
	}
	pcm_adc.dev_hw = user->ak_ft.dev;
	user->mute_flag = AK_FALSE;

	return AK_SUCCESS;
}

/**
 * pcm_set_aec_agc: set aec and agc
 * @adc_fd[IN]: AD device user fd
 * return: 0 success,-1 failed
 */
static int pcm_set_aec_agc(int adc_fd)
{
	/* check if pcm is ready */
	if (!pcm_adc.prepare_flag) {
		ak_print_error_ex("pcm not ready\n");
		return AK_FAILED;
	}

	/* driver pause */
	if (ioctl(adc_fd, IOC_PAUSE, NULL) < 0) {
		ak_print_error_ex("adc ioctl set pause failed.\n");
		return AK_FAILED;
	}

	ak_print_notice_ex("cur_aec_switch=%d, prepare_flag=%d\n",
		pcm_adc.cur_aec_switch, pcm_adc.prepare_flag);

    int value = 0;
	/* aec and agc must set together */
	if (pcm_adc.cur_aec_switch) {
	    set_bit(&value, ECHO_BIT_AEC);
		ak_print_notice_ex("*** enable AEC ***\n");
	} else {
		clear_bit(&value, ECHO_BIT_AEC);
	}

	if (pcm_adc.nr_agc_switch) {
		set_bit(&value, ECHO_BIT_NR);
	    set_bit(&value, ECHO_BIT_AGC);
		ak_print_notice_ex("*** enable NR&AGC ***\n");
	} else {
		clear_bit(&value, ECHO_BIT_NR);
	    clear_bit(&value, ECHO_BIT_AGC);
		if (pcm_adc.nr_switch) {
			set_bit(&value, ECHO_BIT_NR);
			ak_print_notice_ex("*** enable NR ***\n");
		}
	}

	/* driver resume */
	if (ioctl(adc_fd, IOC_RESUME, (void *)&value) < 0) {
		ak_print_error_ex("send IOC_RESUME failed, value=%d\n", value);
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

/**
 * calc_size_by_rate - calculate the size of pcm
 * @user[IN]: pointer point to adc parameters. 
 * @sample_rate[IN]: sample rete.
 * return: pcm size
 */
static int calc_size_by_rate(pcm_user_t *user, unsigned int sample_rate)
{
	unsigned int sample_bytes = (user->param.sample_bits >> 3);
	/* size = sample rete* channel num * time(ms) *sample bit /8 / 1000 */
	/* 8 is bit to byte */
	/* 1000 is second to ms */
	/* user->interval is ms */
	int size = (sample_rate * user->actual_ch
			* user->interval * sample_bytes ) / 1000;
	if (size & 1) {
		size++;
	}

	return size;
}

static int realloc_capture_buffer(int capture_len)
{
	/* free previous exist buffer */
	if (pcm_adc.capture_buf) {
		ak_free(pcm_adc.capture_buf);
		pcm_adc.capture_buf = NULL;
	}

	int ret = AK_SUCCESS;
	pcm_adc.capture_buf = (unsigned char *)ak_calloc(1, capture_len);
	if (!pcm_adc.capture_buf) {
		ak_print_error_ex("realloc audio capture buffer failed\n");
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
		ret = AK_FAILED;
	}
	ak_print_notice_ex("realloc capture buffer from %d to %d\n",
		pcm_adc.capture_len, capture_len);

	return ret;
}

static int malloc_rb_buffer(int total_len)
{
	int ret = AK_FAILED;
	int rb_len = (total_len << 2);

	pcm_adc.rb_handle = ak_rb_init(rb_len);
	if (pcm_adc.rb_handle) {
		ret = AK_SUCCESS;
	}

	return ret;
}

static struct pcm_frame_node* malloc_pcm_frame_node(int frame_len)
{
	struct pcm_frame_node *entry = (struct pcm_frame_node *)ak_calloc(1,
	    sizeof(struct pcm_frame_node));

    if (entry) {
        entry->frame.data = (unsigned char *)ak_calloc(1, frame_len);
        if (!entry->frame.data) {
            ak_print_error_ex("calloc failed\n");
            ak_free(entry);
            entry = NULL;
        }
    }

	return entry;
}

static void free_pcm_frame_node(struct pcm_frame_node *entry)
{
    if (entry) {
        if (entry->frame.data) {
            ak_free(entry->frame.data);
            entry->frame.data = NULL;
        }

        ak_free(entry);
        entry = NULL;
    }
}

static void release_pcm_frame_list(void)
{
    struct pcm_frame_node *entry = NULL;
    struct pcm_frame_node *ptr = NULL;

    list_for_each_entry_safe(entry, ptr, &(pcm_adc.frame_head), list) {
        ak_print_info("free pcm frame: len=%d, ts=%lld, seq_no=%ld\n",
            entry->frame.len, entry->frame.ts, entry->frame.seq_no);
        list_del_init(&(entry->list));
	    free_pcm_frame_node(entry);
    }
}

static void copy_aec_attr(struct echo_lib_param *echo_param, struct ak_audio_aec_attr *aec_attr)
{
	echo_param->m_farDigiGain = aec_attr->audio_out_digi_gain;
	echo_param->m_farThreshold = aec_attr->audio_out_threshold;
	echo_param->m_nearDigiGain = aec_attr->audio_in_digi_gain;
    
	ak_print_notice_ex("m_aecEna=%ld, m_PreprocessEna=%ld, m_framelen=%ld, m_tail=%ld,\
	m_agcEna=%ld, m_agcLevel=%ld, m_maxGain=%d, m_farThreshold=%ld,m_farDigiGain=%d, m_nearDigiGain=%d\n", 
					echo_param->m_aecEna, echo_param->m_PreprocessEna,
					echo_param->m_framelen, echo_param->m_tail,
					echo_param->m_agcEna, echo_param->m_agcLevel,
					echo_param->m_maxGain, echo_param->m_farThreshold,
					echo_param->m_farDigiGain, echo_param->m_nearDigiGain);	
}

static void copy_agc_attr(struct echo_lib_param *echo_param, struct ak_audio_agc_attr *agc_attr)
{
	echo_param->m_agcLevel = agc_attr->agc_level;
	echo_param->m_maxGain = agc_attr->agc_max_gain;
    echo_param->m_minGain = agc_attr->agc_min_gain;
    echo_param->m_nearSensitivity = agc_attr->near_sensitivity;
    
    ak_print_error_ex("m_aecEna=%ld, m_PreprocessEna=%ld, m_framelen=%ld, m_tail=%ld,\
	m_agcEna=%ld, m_agcLevel=%ld, m_maxGain=%d, m_farThreshold=%ld,m_farDigiGain=%d, \
	m_nearDigiGain=%d, m_minGain=%d, m_nearSensitivity=%d\n", 
					echo_param->m_aecEna, echo_param->m_PreprocessEna,
					echo_param->m_framelen, echo_param->m_tail,
					echo_param->m_agcEna, echo_param->m_agcLevel,
					echo_param->m_maxGain, echo_param->m_farThreshold,
					echo_param->m_farDigiGain, echo_param->m_nearDigiGain, 
					echo_param->m_minGain, echo_param->m_nearSensitivity);
}

static void copy_nr_attr(struct echo_lib_param *echo_param, struct ak_audio_nr_attr *nr_attr)
{
	echo_param->m_noiseSuppressDb = nr_attr->noise_suppress_db;

    ak_print_error_ex("m_aecEna=%ld, m_PreprocessEna=%ld, m_framelen=%ld, m_tail=%ld,\
	m_agcEna=%ld, m_agcLevel=%ld, m_maxGain=%d, m_farThreshold=%ld,m_farDigiGain=%d, m_nearDigiGain=%d,\
	m_noiseSuppressDb =%d\n", 
					echo_param->m_aecEna, echo_param->m_PreprocessEna,
					echo_param->m_framelen, echo_param->m_tail,
					echo_param->m_agcEna, echo_param->m_agcLevel,
					echo_param->m_maxGain, echo_param->m_farThreshold,
					echo_param->m_farDigiGain, echo_param->m_nearDigiGain,
					echo_param->m_noiseSuppressDb);
}

static int init_echo_lib_attr(pcm_user_t *user)
{
	struct echo_lib_param echo_param;
	if (adc_dev_get_echo_attr(user->cp_fd, &echo_param)) {
		return AK_FAILED;
	}

	if (pcm_adc.set_aec_attr_flag)
		copy_aec_attr(&echo_param, &(pcm_adc.aec_attr));
	if (pcm_adc.set_agc_attr_flag)
		copy_agc_attr(&echo_param, &(pcm_adc.agc_attr));
	if (pcm_adc.set_nr_attr_flag)
		copy_nr_attr(&echo_param, &(pcm_adc.nr_attr));
	
	ak_print_error_ex("set===== adc_dev_set_echo_attr\n");
	if (adc_dev_set_echo_attr(user->cp_fd, &(echo_param))) {
		return AK_FAILED;
	}
	return AK_SUCCESS;
}

/**
 * set_param_to_adc_driver - set param to the AD device
 * @user[IN]: pointer point to adc parameters.
 * return: 0 success, -1 failed
 */
static int set_param_to_adc_driver(pcm_user_t *user)
{
	/* set set driver parameters */
	if (ioctl(user->cp_fd, IOC_PAUSE, NULL) < 0) {
		ak_print_error_ex("adc ioctl set pause failed\n");
		return AK_FAILED;
	}

	if (ioctl(user->cp_fd, IOC_SET_PARS, (void *)(&(user->param))) < 0) {
		ak_print_error_ex("adc ioctl set params failed\n");
		return AK_FAILED;
	}

    if (ioctl(user->cp_fd, IOC_PREPARE, NULL) < 0) {
		ak_print_error_ex("send IOC_RESUME failed\n");
		return AK_FAILED;
	}

	/* ADC set params OK, and it prepare to work. */
	pcm_adc.prepare_flag = AK_TRUE;

	if (get_actual_rate(user)) {
		return AK_FAILED;
	}

	/* the function pcm_set_aec_agc include the driver resume */
	if (pcm_set_aec_agc(user->cp_fd)) {
		return AK_FAILED;
	}

	if (pcm_adc.set_agc_attr_flag || pcm_adc.set_aec_attr_flag || pcm_adc.set_nr_attr_flag) {
		if (init_echo_lib_attr(user)) {
			return AK_FAILED;
		}
	}	

	return AK_SUCCESS;
}

/**
 * set_user_use_adc - set the appointed user to use adc
 * @user[in]: pointer point to adc param
 * return 0 success, -1 failed
 */
static int set_adc_resample(pcm_user_t *user)
{
	if (!user->sdf_switch || (user->input_len == pcm_adc.capture_len)) {
		return AK_SUCCESS;
	}

	ak_print_notice_ex("audio actual_rate=%d, param.rate=%d\n",
		user->actual_rate, user->param.rate);

	if (user->actual_rate != user->param.rate) {
		/* calculate the resample buffer length */
		user->input_len = pcm_adc.capture_len;
		pcm_adc.sdf_len = ((user->input_len * user->param.rate)
			/ user->actual_rate);
		pcm_adc.sdf_len += 2;

		if (pcm_adc.sdf_len < pcm_adc.capture_len) {
			pcm_adc.sdf_len = pcm_adc.capture_len;
		}
		if (pcm_adc.sdf_len > ADC_CAP_BUF_MAX_LEN) {
			realloc_capture_buffer(pcm_adc.sdf_len);
		}

		/* free previous exist buffer */
		if (pcm_adc.sdf_buf) {
			ak_free(pcm_adc.sdf_buf);
			pcm_adc.sdf_buf = NULL;
		}

		pcm_adc.sdf_buf = (unsigned char *)ak_calloc(1, pcm_adc.sdf_len);
		if (!pcm_adc.sdf_buf) {
			ak_print_error_ex("ak_calloc audio sdf buffer failed\n");
			set_error_no(ERROR_TYPE_MALLOC_FAILED);
			return AK_FAILED;
		}

		ak_print_notice_ex("audio sdf_len=%d, frame_size=%d, capture_len=%d\n",
			pcm_adc.sdf_len, pcm_adc.frame_size, pcm_adc.capture_len);
	}

    int ret = AK_FAILED;
	/* open resample */
	user->sdfilter = sdfilter_open_resample(user, user->actual_rate,
	    user->param.rate);
	if (user->sdfilter) {
		ak_print_debug_ex("adc actual rate:%d, desr rate:%d\n",
		    user->actual_rate, user->param.rate);
		ret = AK_SUCCESS;
	} else {
        ak_print_error_ex("open sdfilter failed\n");
	}

	return ret;
}

/**
 * set_ad_aslc_param - set aslc parameter
 * @handler[in]: aslc open handle
 * @level[in]: volume level
 * return 0 success, -1 failed
 */
static int set_ad_aslc_param(void *handler, int level)
{
	if (level < 0 || level > 6) {
		level = 0;
	}

	int ret = AK_FAILED;

	if (handler) {
		/* 0,3,6,...,18 dB magnification times,means 6~12 level */
		/* vol=[0:3:18] */

		/* round(32768 * 10.^((stone2_x+vol)/20))*/
		static const int stone2_y[] = {10,15,21,29,41,58,82};
		/* round(32768 * 10.^((-vol)/20)) */
		static const int stone3_x[] = {32768,23198,16423,11627,8231,5827,4125};

		T_FILTER_MILESTONE tmile = {0};
		tmile.lookAheadTime = 6;
		tmile.gainAttackTime = 3;
		tmile.gainReleaseTime = 300;
		tmile.num = 5;
		tmile.stone[0].x = 0;
		tmile.stone[0].y = 0;
		tmile.stone[1].x = AD_STONE1_X;
		tmile.stone[1].y = AD_STONE1_Y;
		tmile.stone[2].x = AD_STONE2_X;
		tmile.stone[2].y = stone2_y[level];
		tmile.stone[3].x = stone3_x[level];
		tmile.stone[3].y = 32768;
		tmile.stone[4].x = 32768; //0db
		tmile.stone[4].y = 32768; //0db

		if (AK_TRUE == _SD_Filter_SetAslcMileStones(handler, &tmile))
			ret = AK_SUCCESS;
	}

	return ret;
}

/**
 * adc_set_aslc - open aslc
 * @user[in]: pointer point to adc param
 * @volume[in]: volume level
 * return 0 success, -1 failed
 */
static int adc_set_aslc(pcm_user_t *user, int volume)
{
	if (NULL == user) {
		ak_print_error_ex( "user is NULL\n" );
		return AK_FAILED;
	}

	if (NULL == user->sdfilter_aslc) {
    	struct pcm_param pcm_attr_in;
        pcm_attr_in.sample_rate = user->param.rate;
        pcm_attr_in.sample_bits = user->param.sample_bits;
        pcm_attr_in.channel_num = user->param.channels;
            
		user->sdfilter_aslc = sdfilter_open(&pcm_attr_in, _SD_FILTER_ASLC, 0);
		if (NULL == user->sdfilter_aslc) {
			ak_print_error_ex("open aslc sdfilter failed\n");
			return AK_FAILED;
		}
	}

	/* aslc volume level is from 6~12 */
	int volume_level = 0;
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

	/* set the param to aslc */
	int ret = set_ad_aslc_param(user->sdfilter_aslc, volume_level);
	if (ret) {
		ak_print_error_ex( "sdfilter_aslc_set_param error!\n" );
	}

	return ret;
}

/**
 * read_adc_driver - read from the ADC device
 * @handle[IN]: ADC device handle
 * @to[IN]: read data to the buffer
 * @size[IN]: read length
 * return: read data length. -1 failed
 */
static int read_adc_driver(void *handle, unsigned char *to, unsigned int size)
{
	pcm_user_t *user = (pcm_user_t *)handle;
	int max_read = user->param.period_bytes;
	unsigned int origin_read = size;
	int read_len = 0;
	int ret_len = 0;	/* origin data length */
	int offset = 0;

	/* read data from driver */
	while(offset < origin_read) {
		if(size > max_read) {
			read_len = max_read;
		} else {
			read_len = size;
		}

		ret_len = read(user->cp_fd, &pcm_adc.read_buf[offset], read_len);
		if (ret_len < 0) {
			ak_print_error_ex("read error\n");
			return AK_FAILED;
		}

		offset += ret_len;
		size -= ret_len;
	}
	ai_save_stream_to_file(pcm_adc.read_buf, offset, AI_ORIGIN_LEVEL);

	unsigned char *out_buf = NULL;

	/* do resample */
	if (user->sdf_switch && user->sdfilter) {
		ret_len = sdfilter_control(user->sdfilter, pcm_adc.read_buf, offset,
			pcm_adc.sdf_buf, pcm_adc.sdf_len);
		out_buf = pcm_adc.sdf_buf;

	ai_save_stream_to_file(pcm_adc.sdf_buf, ret_len, AI_RESAMPLE_LEVEL);

#if AI_READ_TS_DEBUG
		ak_print_normal_ex("offset=%d, ret_len=%d, out sdf_len=%d\n",
			offset, ret_len, pcm_adc.sdf_len);
#endif
	} else {
		out_buf = pcm_adc.read_buf;
		ret_len = offset;
	}

	if (NULL == out_buf || ret_len <= 0) {
		ak_print_error_ex("sdfilter out buffer null,out_buf=%p,ret_len=%d\n",
						out_buf, ret_len);
		return AK_FAILED;
	}

	/* do aslc */
	if (user->sdfilter_aslc) {
		/* notice: out len must large enough */
		if (pcm_adc.sdf_len > pcm_adc.capture_len) {
			ret_len = sdfilter_control(user->sdfilter_aslc, out_buf, ret_len,
				to, pcm_adc.sdf_len);
		} else {
			ret_len = sdfilter_control(user->sdfilter_aslc, out_buf, ret_len,
				to, pcm_adc.capture_len);
		}

		ai_save_stream_to_file(to, ret_len, AI_ASLC_LEVEL);
	} else {
		memcpy(to, out_buf, ret_len);
	}

	return ret_len;
}

/**
 * pcm_adc_open - open the AD device
 * @void
 * return: opened AD device handle, NULL failed
 */
static void* pcm_adc_open(void)
{
	int ret=  AK_FAILED;
	pcm_user_t *user = NULL;

	ak_thread_mutex_lock(&pcm_adc.mutex);
	if (AK_INVALID_FD == pcm_adc.fd) {
		pcm_adc.fd = open(ADC_DEV_NAME, O_RDONLY);
		if (AK_INVALID_FD == pcm_adc.fd) {
			ak_print_error("open %s failed: %s\n", ADC_DEV_NAME, strerror(errno));
			goto adc_open_end;
		}

		if (-1 == fcntl(pcm_adc.fd, F_SETFD, FD_CLOEXEC)){
			ak_print_error_ex("error:%s\n", strerror(errno));
			goto adc_open_end;
		}
	}

	user = (pcm_user_t *)ak_calloc(1, sizeof(pcm_user_t));
	if (NULL == user) {
		ak_print_error("calloc ADC user error\n");
		goto adc_open_end;
	}

	/* now set flags & param */
	user->cp_fd = pcm_adc.fd;
	load_adc_default_param(user);
	pcm_adc.run_flag = AK_TRUE;
	++pcm_adc.users_count;
	pcm_adc.set_agc_attr_flag = 0;
	pcm_adc.set_aec_attr_flag = 0;
    pcm_adc.set_nr_attr_flag = 0;
    
	ret = AK_SUCCESS;

adc_open_end:
	if (AK_FAILED == ret) {
		if (AK_INVALID_FD == pcm_adc.fd) {
			close(pcm_adc.fd);
			pcm_adc.fd = AK_INVALID_FD;
		}
	}
	ak_thread_mutex_unlock(&pcm_adc.mutex);

	return user;
}

/**
 * pcm_set_param: save parameter
 * @handle[IN]: AD device handle
 * @value[IN]: parameter
 * return: 0 success, -1 failed
 */
static int pcm_set_param(void *handle, void *value)
{
	if (NULL == handle || NULL == value) {
		ak_print_error_ex("pointer NULL\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	if (user->cp_fd != pcm_adc.fd) {
		return AK_FAILED;
	}

	struct akpcm_pars *param = &(user->param);
	raw_param_t *local_param = NULL;

    ak_thread_mutex_lock(&pcm_adc.mutex);
	local_param = (raw_param_t *)value;
	param->rate = local_param->sample_rate;
	param->sample_bits = local_param->sample_bits;
    ak_thread_mutex_unlock(&pcm_adc.mutex);

	return AK_SUCCESS;
}

/**
 * pcm_reset_buf: clear buffer
 * @handle[IN]: AD device handle
 * @value[IN]: =1 clear buffer
 * return: real read bytes, -1 failed
 */
static int pcm_reset_buf(void *handle, void *value)
{
	if (NULL == handle || NULL == value) {
		ak_print_error_ex("pointer NULL\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	if (user->cp_fd != pcm_adc.fd) {
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;

	/* ring buffer clear */
	ak_rb_reset(pcm_adc.rb_handle);

	/* driver buffer clear */
	ret = ioctl(pcm_adc.fd, IOC_RSTBUF, (void *)&value);
	if (ret < 0) {
		ak_print_error_ex("set RSTBUF failed\n");
		ret = AK_FAILED;
	}

	return ret;
}

/**
 * pcm_adc_close - close to the AD virtual device
 * @user[IN]: AD user handle
 * return: 0 success, -1 failed
 */
static int pcm_adc_close(pcm_user_t *user)
{
	ak_print_info_ex("enter\n");
	ak_thread_mutex_lock(&pcm_adc.mutex);
	--pcm_adc.users_count;
	if (0 == pcm_adc.users_count) {
		pcm_adc.run_flag = AK_FALSE;
		if (pcm_adc.thread_flag) {
			if (pcm_adc.running) {
				ak_ai_stop_capture(user);
			}

			ak_thread_sem_post(&pcm_adc.capture_sem);
			ak_print_normal_ex("join capture pcm thread...\n");
			ak_thread_join(pcm_adc.tid);
			ak_print_notice_ex("capture pcm thread join OK\n");
		}

		release_pcm_frame_list();

		close(pcm_adc.fd);
		pcm_adc.fd = AK_INVALID_FD;
		pcm_adc.cur_aec_switch = FUNC_DISABLE;

		if (pcm_adc.read_buf) {
			ak_free(pcm_adc.read_buf);
			pcm_adc.read_buf = NULL;
		}

		if (pcm_adc.capture_buf) {
			ak_free(pcm_adc.capture_buf);
			pcm_adc.capture_buf = NULL;
		}

		if (pcm_adc.sdf_buf) {
			ak_free(pcm_adc.sdf_buf);
			pcm_adc.sdf_buf = NULL;
		}

		ak_thread_mutex_destroy(&(pcm_adc.read_mutex));
	}

    if (pcm_adc.running == (void *)user) {
		pcm_adc.running = NULL;
    }

	if (pcm_adc.rb_handle) {
		ak_rb_release(pcm_adc.rb_handle);
		pcm_adc.rb_handle = NULL;
	}
	if (user->sdfilter) {
		sdfilter_close(user->sdfilter);
		user->sdfilter = NULL;
	}
	if (user->sdfilter_aslc) {
		sdfilter_close(user->sdfilter_aslc);
		user->sdfilter_aslc = NULL;
	}

	pcm_adc.prepare_flag = AK_FALSE;
	ak_thread_mutex_unlock(&pcm_adc.mutex);
	if (0 == pcm_adc.users_count) {
		ak_thread_mutex_destroy(&pcm_adc.mutex);
	}
	ak_print_info_ex("leave\n");

	return AK_SUCCESS;
}

/**
 * pcm_adc_set_volume: driver set volume
 * @user[IN]: AD user handle
 * @volume[IN]: volume number
 * return: 0 success, -1 failed
 */
static int pcm_adc_set_volume(pcm_user_t *user, int volume)
{
	/* ADC driver value 0 match gain 20dbm */
	ak_print_notice_ex("set DEV volume %d\n",volume);

	if (ioctl(user->cp_fd, IOC_SET_GAIN, (void *)(&volume)) < 0) {
		ak_print_error_ex("adc ioctl set gain failed\n");
		return AK_FAILED;
	}
	user->ak_ft.gain = volume;

	return AK_SUCCESS;
}

static int get_eq_attr_param(pcm_user_t *user, struct ak_audio_eq_attr *eq_attr,
                                signed long *peqpara)
{
    struct pcm_param pcm_attr_in;
    pcm_attr_in.sample_rate = user->param.rate;
    pcm_attr_in.sample_bits = user->param.sample_bits;
    pcm_attr_in.channel_num = user->param.channels;

	void *sdfilter_eq = sdfilter_open_eq(&pcm_attr_in, _SD_EQ_USER_DEFINE, eq_attr);
    
	if (!sdfilter_eq) {
		ak_print_error_ex("open eq failed \n");
		return AK_FAILED;
	} else {
		ak_print_notice_ex("enable eq \n");
	}
	
	if (sdfilter_eq_param_convert(sdfilter_eq, &pcm_attr_in, eq_attr, peqpara, EQ_ARRAY_NUM)) {
		return AK_FAILED;
	}

	sdfilter_close(sdfilter_eq);
    return AK_SUCCESS;
}

/**
 * mark_ts_to_data: set the frame timestamp
 * @user[IN]: AD user
 * @frame[IN]: frame information
 * return: 0 success, -1 failed
 */
static void mark_ts_to_data(pcm_user_t *user, struct frame *frame)
{
	frame->ts = get_pcm_timer(user);

    struct ak_timeval cur_time;
    ak_get_ostime(&cur_time);
    long use_time = ak_diff_ms_time(&cur_time, &(pcm_adc.frame_time));

	if(0 == pcm_adc.pre_frame_ts) {
		ak_print_notice_ex("*** we get the first audio, ts=%llu, time=%ld ***\n\n",
			frame->ts, use_time);
	} else {
		/* timestamp rollback */
		if (frame->ts <= pcm_adc.pre_frame_ts) {
			ak_print_notice_ex("ts rollback, pre ts=%llu, cur ts=%llu, "
			    "diff ts=%llu\n\n",
				pcm_adc.pre_frame_ts, frame->ts,
				(pcm_adc.pre_frame_ts - frame->ts));
		}
		/* timestamp too long */
		if (frame->ts > (pcm_adc.pre_frame_ts + pcm_adc.frame_interval*2)) {
			ak_print_notice_ex("pre ts=%llu, cur ts=%llu, diff ts=%llu, "
			    "use time=%ld\n\n",
				pcm_adc.pre_frame_ts, frame->ts,
				(frame->ts - pcm_adc.pre_frame_ts), use_time);
		}
	}

	pcm_adc.pre_frame_ts = frame->ts;
	ak_get_ostime(&(pcm_adc.frame_time));
}

/**
 * set_adc_mute - set volume mute
 * @user[IN]: AD user handle
 * return: 0 success, -1 failed
 */
static int set_adc_mute(pcm_user_t *user)
{
	int value = SIGNAL_ADC_SRC_MUTE;
	int ret = ioctl(user->cp_fd, IOC_SET_SOURCES, (void *)(&value));

	if (ret) {
		ak_print_error_ex("ADC ioctl set MUTE failed\n");
	} else {
		user->mute_flag = AK_TRUE;
	}

	return ret;
}

/**
 * read_pcm_data - read pcm data,and write data to ring buffer
 * @user[IN]: AD user handle
 * return: read size
 */
static int read_pcm_data(pcm_user_t *user)
{
	++pcm_adc.pcm_seq_no;

	/* capture_len >= frame_size */
	int read_size = read_adc_driver(user, pcm_adc.capture_buf,
		pcm_adc.capture_len);
	if (read_size < 0) {
		goto pcm_data_end;
	}

#if AI_READ_PCM_DRV_DEBUG
	ak_print_normal_ex("pcm_seq_no=%lu, read_size=%d, capture_len=%d\n",
		pcm_adc.pcm_seq_no, read_size, pcm_adc.capture_len);
#endif

	/* drop the first 360 ms data */
	if(pcm_adc.drop_time < ADC_CAP_DROP_TIME) {
		int interval = pcm_adc.frame_interval;
		if(pcm_adc.frame_size < pcm_adc.read_len) {
			interval *= (pcm_adc.read_len / pcm_adc.frame_size);
		}

		pcm_adc.pcm_seq_no = 0;
		pcm_adc.drop_time += interval;
		if (pcm_adc.drop_time >= ADC_CAP_DROP_TIME) {
			ak_print_notice_ex("we had dropped the first %d ms audio data\n",
				pcm_adc.drop_time);
		}
	} else {
		/* put data to ring buffer */
		ak_rb_write(pcm_adc.rb_handle, pcm_adc.capture_buf, read_size);
	}

pcm_data_end:
	return read_size;
}

/**
 * drop_extra_frame - if the first entery is too early, drop it
 * @cur_ts[IN]: current frame timestamp
 * return: void
 */
static void drop_extra_frame(struct pcm_frame_node *cur_entry)
{
    struct pcm_frame_node *entry = NULL;
    struct pcm_frame_node *ptr = NULL;

    list_for_each_entry_safe(entry, ptr, &(pcm_adc.frame_head), list) {
        /* whether the frame ts is too early */
		unsigned long long diff_ts = (cur_entry->frame.ts - entry->frame.ts);
		if (diff_ts >= AI_MAX_FRAME_TIME) {
			ak_print_debug_ex("pcm frame is too many, diff ts: %llu(ms)\n",
				diff_ts);
			ak_print_debug_ex("cur entry: ts=%llu, seq_no=%ld\n",
				cur_entry->frame.ts, cur_entry->frame.seq_no);
			ak_print_debug_ex("first entry: ts=%llu, seq_no=%ld\n\n",
				entry->frame.ts, entry->frame.seq_no);

			/* drop the frame */
			list_del_init(&(entry->list));
			free_pcm_frame_node(entry);
		}
		break;
    }
}

/**
 * generate_pcm_frame_list: make the data to frame,then put the frame to list
 * @user[IN]: AD user handle
 * return: NULL
 */
static void generate_pcm_frame_list(pcm_user_t *user)
{
    int ret_len = 0;
    struct pcm_frame_node *entry = NULL;
    int remain = ak_rb_get_data_len(pcm_adc.rb_handle);

	while (remain >= pcm_adc.frame_size) {
        entry = malloc_pcm_frame_node(pcm_adc.frame_size);
        if (entry) {
            ret_len = ak_rb_read(pcm_adc.rb_handle, entry->frame.data,
                pcm_adc.frame_size);
    		if (ret_len < 0) {
    		    ak_print_error_ex("read from ring buffer failed\n");
    		    free_pcm_frame_node(entry);
    		    break;
    		}

            entry->frame.len = pcm_adc.frame_size;
            entry->frame.seq_no = pcm_adc.get_seq_no;
            mark_ts_to_data(user, &(entry->frame));

            ak_thread_mutex_lock(&(pcm_adc.read_mutex));
            drop_extra_frame(entry);
            list_add_tail(&(entry->list), &(pcm_adc.frame_head));
            ak_thread_mutex_unlock(&(pcm_adc.read_mutex));

            ++pcm_adc.get_seq_no;
            remain -= pcm_adc.frame_size;
        } else {
            break;
        }
	}
}

/**
 * capture_pcm_thread: capture audio pcm data from AD
 * @arg[IN]: AD user handle
 * return: NULL
 */
static void* capture_pcm_thread(void *arg)
{
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	pcm_user_t *user = (pcm_user_t *)arg;
	ak_thread_set_name("ai_capture");

#if AI_READ_PCM_DRV_DEBUG
	struct ak_timeval cap_time;
	struct ak_timeval cur_time;
#endif

	while (pcm_adc.run_flag) {
		ak_print_normal_ex("sleep...\n");
		ak_thread_sem_wait(&pcm_adc.capture_sem);
		ak_print_normal_ex("wakup...\n");

		ak_get_ostime(&(pcm_adc.frame_time));
        while (pcm_adc.run_flag && pcm_adc.capture_flag) {
#if AI_READ_PCM_DRV_DEBUG
        	ak_get_ostime(&cur_time);
			if (ak_diff_ms_time(&cur_time, &cap_time) > 500) {
				ak_get_ostime(&cap_time);
				ak_print_notice_ex("pcm drv data_len=%lu, capture_len=%d\n",
					get_pcm_data_len(user->cp_fd), pcm_adc.capture_len);
			}
#endif

        	if (get_pcm_data_len(user->cp_fd) >= pcm_adc.capture_len) {
				read_pcm_data(user);
				generate_pcm_frame_list(user);
			} else {
				ak_sleep_ms(10);
			}
        }
	}

    ak_print_normal_ex("### thread id: %ld exit ###\n\n",
        ak_thread_get_tid());

	ak_thread_exit();
	return NULL;
}

/**
 * init_pcm_adc: init adc parameter
 * return: 0 success,-1 failed
 */
static int init_pcm_adc(const struct pcm_param *param)
{
	int ret = AK_FAILED;

	/* default read buffer and len, we won't change them */
	pcm_adc.read_len = ADC_CAP_BUF_MAX_LEN;
	pcm_adc.read_buf = (unsigned char *)ak_calloc(1, pcm_adc.read_len);
	if (!pcm_adc.read_buf) {
		ak_print_error_ex("calloc audio read buffer failed\n");
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
		goto init_pcm_adc_end;
	}

	/* default capture buffer and len, capture buffer can be changed */
	pcm_adc.capture_len = ADC_CAP_BUF_MAX_LEN;
	pcm_adc.capture_buf = (unsigned char *)ak_calloc(1, pcm_adc.capture_len);
	if (!pcm_adc.capture_buf) {
		ak_print_error_ex("calloc audio capture buffer failed\n");
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
		goto init_pcm_adc_end;
	}

	/* ring buffer init*/
	if (malloc_rb_buffer(pcm_adc.capture_len)) {
		ak_print_error_ex("calloc capture ring buffer failed\n");
		set_error_no(ERROR_TYPE_MALLOC_FAILED);
		goto init_pcm_adc_end;
	}

	pcm_adc.run_flag = AK_FALSE;
	pcm_adc.prepare_flag = AK_FALSE;
	pcm_adc.capture_flag = AK_FALSE;
	pcm_adc.thread_flag = AK_FALSE;
	pcm_adc.fd = AK_INVALID_FD;
	pcm_adc.users_count = 0;

	pcm_adc.pre_frame_ts = 0;
	pcm_adc.pre_sys_ts = 0;

	pcm_adc.drop_time = 0;
	pcm_adc.frame_interval = 100; //set default value
	pcm_adc.frame_size = pcm_adc.frame_interval * 
		(param->channel_num * param->sample_rate * param->sample_bits/8) / 1000;
	ak_print_normal_ex("frame interval: %d,. frame size: %d\n",
			pcm_adc.frame_interval, pcm_adc.frame_size);
	INIT_LIST_HEAD(&(pcm_adc.frame_head));

	pcm_adc.running = NULL;
	pcm_adc.cur_aec_switch = FUNC_DISABLE;
	pcm_adc.nr_agc_switch = FUNC_DISABLE;
	pcm_adc.nr_switch = FUNC_DISABLE;
	pcm_adc.nr_max_switch = FUNC_DISABLE;
	ak_thread_sem_init(&pcm_adc.capture_sem, 0);
	ak_thread_mutex_init(&(pcm_adc.mutex), NULL);
	ak_thread_mutex_init(&(pcm_adc.read_mutex), NULL);
	ret = AK_SUCCESS;

init_pcm_adc_end:
	if (ret) {
		if (pcm_adc.read_buf) {
			ak_free(pcm_adc.read_buf);
			pcm_adc.read_buf = NULL;
		}
		pcm_adc.read_len = 0;

		if (pcm_adc.capture_buf) {
			ak_free(pcm_adc.capture_buf);
			pcm_adc.capture_buf = NULL;
		}
		pcm_adc.capture_len = 0;
	}

	return ret;
}

/**
 * ad_copy_for_dual_channel: copy frome single channel to double channel
 * @src[IN]: single channel data
 * @src[IN]: single channel data length
 * @src[IN]: double channel data
 * return: NULL
 */
static void ad_copy_for_dual_channel(const unsigned char *src, int len,
							unsigned char *dest)
{
	int j = 0;
	int count = (len / 2);

	for (j=0; j<count; ++j) {
		dest[j * 4] = src[j * 2];
		dest[j * 4 + 1] = src[j * 2 + 1];
		dest[j * 4 + 2] = src[j * 2];
		dest[j * 4 + 3] = src[j * 2 + 1];
	}
}

static int get_frame_non_block(void *handle, struct frame *frame)
{
    int ret = AK_FAILED;
    pcm_user_t *user = (pcm_user_t *)handle;

    /* get one frame from frame list ONCE */
	ak_thread_mutex_lock(&(pcm_adc.read_mutex));
	if (pcm_adc.run_flag && pcm_adc.capture_flag) {
	    struct pcm_frame_node *entry = NULL;
	    struct pcm_frame_node *ptr = NULL;
	    int frame_len = 0;

	    list_for_each_entry_safe(entry, ptr, &(pcm_adc.frame_head), list) {
	    	frame_len = user->param.channels == AUDIO_CHANNEL_STEREO ?
	    				entry->frame.len * 2 : entry->frame.len;
	    	frame->data = ak_calloc(1, frame_len);
	        if (frame->data) {
	        	if (user->param.channels == AUDIO_CHANNEL_STEREO) {
					ad_copy_for_dual_channel(entry->frame.data, entry->frame.len, 
											frame->data);
	        	} else {
	            	memcpy(frame->data, entry->frame.data, entry->frame.len);
	            }
	            
	            frame->len = frame_len;
	            frame->ts = entry->frame.ts;
	            frame->seq_no = entry->frame.seq_no;
	            ret = AK_SUCCESS;

	            list_del_init(&(entry->list));
	    	    free_pcm_frame_node(entry);
	        } else {
	            ak_print_error_ex("calloc frame->data failed\n");
	        }

	        break;
	    }
	}
    ak_thread_mutex_unlock(&(pcm_adc.read_mutex));

    return ret;
}

static int get_frame_block(void *handle, struct frame *frame)
{
    int ret = AK_FAILED;

    do {
        ret = get_frame_non_block(handle, frame);
        if (ret) {
            ak_sleep_ms(10);
        }
    } while (ret);

    return ret;
}

static int get_frame_timeout(void *handle, struct frame *frame, long wait_time)
{
    int ret = AK_FAILED;

    do {
        ret = get_frame_non_block(handle, frame);
        if (ret) {
            ak_sleep_ms(10);
            wait_time -= 10;
        }
    } while (ret && (wait_time > 0));

    return ret;
}

/**
 * ak_ai_print_filter_info - print audio filter version & support functions
 * notes: filter such as: EQ, 3D, RESAMPLE, AGC and so on
 */
void ak_ai_print_filter_info(void)
{
	T_AUDIO_FILTER_CB_FUNS cb_filter={0};

    cb_filter.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;
    _SD_GetAudioFilterVersions(&cb_filter);
}

/**
 * ak_ai_get_version - get audio in version
 * return: version string
 * notes:
 */
const char* ak_ai_get_version(void)
{
	return ai_version;
}

/**
 * ak_ai_open - open audio in device
 * @input[IN]: open audio input param
 * return: opened handle, NULL failed
 * notes:
 */
void* ak_ai_open(const struct pcm_param *param)
{
	if (param == NULL) {
		ak_print_error_ex("param is NULL\n");
		return NULL;
	}
	
	if(!pcm_adc.run_flag) {
		if (init_pcm_adc(param)) {
			return NULL;
		}
	}

	if (param->sample_rate < 8000 || param->sample_rate > 64000) {
		/* sample rate not support  */
		ak_print_error_ex("sample rate not suppprt %d \n", param->sample_rate);
		return NULL;
	}

    ak_print_notice_ex("ai version %s \n", ai_version);
    void *handle = pcm_adc_open();
	if(handle){
		pcm_user_t *user = (pcm_user_t *)handle;
	    user->param.channels = param->channel_num;
	    user->param.rate = param->sample_rate;
	    user->param.sample_bits = param->sample_bits;

	    raw_param_t ad = {0};

	    ad.sample_bits = param->sample_bits;
	    ad.sample_rate = param->sample_rate;
	    /* reopen check */
	    pcm_set_param(handle, (void *)&ad);

	    ai_sys_ipc_register();
	    ai_sysipc_bind_handle(handle);
	}
    pcm_adc.ai_handle = handle;

    return handle;
}

/**
 * ak_ai_get_params - start ADC capture
 * @handle[IN]: audio in opened handle
 * @param[OUT]: store ai params delivery by ai_open();
 * return: 0 success, -1 failed
 * notes: call after set all kind of ADC attr
 */
int ak_ai_get_params(void *handle, struct pcm_param *param)
{
	/* param check */
	if(!handle || !param) {
		ak_print_error_ex("invalid param\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;

	/* value assingment */
	param->sample_rate = user->param.rate;
	param->sample_bits = user->param.sample_bits;
	param->channel_num = user->param.channels;

	return AK_SUCCESS;
}

/**
 * ak_ai_get_handle - get ai handle
 * @dev_id[IN]: audio in device id
 * @ai_handle[OUT]: audio in opened handle
 * return: 0 success, -1 failed
 * notes: 
 */
int ak_ai_get_handle(int dev_id, void **ai_handle)
{
	if (!pcm_adc.ai_handle) {
		ak_print_error_ex("pcm_adc.ai_handle is NULL\n");
		return AK_FAILED;
	}
    *ai_handle = pcm_adc.ai_handle;
    ak_print_normal("pcm_adc.ai_handle is %p\n", pcm_adc.ai_handle);
    return AK_SUCCESS;
}

/**
 * ak_ai_start_capture - start ADC capture
 * @handle[IN]: audio in opened handle
 * return: 0 success, -1 failed
 * notes: call after set all kind of ADC attr
 */
int ak_ai_start_capture(void *handle)
{
	if (pcm_adc.running) {
		/* ai capture had started */
		return AK_SUCCESS;
	}

	ak_print_info_ex("enter...\n");
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	if ((AK_INVALID_FD == user->cp_fd) || (user->cp_fd != pcm_adc.fd)) {
		ak_print_error_ex("audio uer fd error, cp_fd=%d, fd=%d\n",
			user->cp_fd, pcm_adc.fd);
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	ak_thread_mutex_lock(&pcm_adc.mutex);
	if (pcm_adc.running != (void *)user) {
		if (set_param_to_adc_driver(user) < 0)
			goto start_capture_end;

		pcm_adc.running = (void *)user;
	}

	if (set_adc_resample(user)) {
		goto start_capture_end;
	}

	pcm_adc.capture_flag = AK_TRUE;
	if (pcm_adc.thread_flag) {
		ret = AK_SUCCESS;
	} else {
		ret = ak_thread_create(&(pcm_adc.tid), capture_pcm_thread,
			handle, ANYKA_THREAD_MIN_STACK_SIZE, -1);
		if (ret) {
			pcm_adc.capture_flag = AK_FALSE;
			ak_print_error("create capture_pcm_thread FAILED, ret=%d\n",
			    ret);
			goto start_capture_end;
		} else {
			pcm_adc.thread_flag = AK_TRUE;
			ak_print_info_ex("create capture pcm thread OK\n");
		}
	}

	ak_rb_reset(pcm_adc.rb_handle);
	ak_thread_sem_post(&pcm_adc.capture_sem);

start_capture_end:
	if (ret) {
		pcm_adc.running = NULL;
	}
	ak_thread_mutex_unlock(&pcm_adc.mutex);
	ak_print_info_ex("leave..., capture_flag=%d\n", pcm_adc.capture_flag);

	return ret;
}

/**
 * ak_ai_stop_capture - stop ADC capture
 * @handle[IN]: audio in opened handle
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ai_stop_capture(void *handle)
{
    ak_print_normal_ex("entering...\n");
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
        ak_print_error_ex("ai handle is NULL\n");
		return AK_FAILED;
	}

    pcm_user_t *user = (pcm_user_t *)handle;

    /* pcm driver pause capture */
    if (ioctl(user->cp_fd, IOC_PAUSE, NULL) < 0) {
		ak_print_error_ex("adc ioctl set pause failed\n");
		return AK_FAILED;
	}

	pcm_adc.capture_flag = AK_FALSE;
	pcm_adc.running = NULL;
    ak_print_normal_ex("leave...\n");

	return AK_SUCCESS;
}

/**
 * ak_ai_get_frame - get audio frame
 * @handle[IN]: audio in opened handle
 * @frame[OUT]: audio frame info
 * @ms[IN]: <0 block mode; =0 non-block mode; >0 wait time
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ai_get_frame(void *handle, struct frame *frame, long ms)
{
	if(!handle) {
		ak_print_error_ex("handle is NULL\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	if(!frame) {		
		ak_print_error_ex("frame is NULL\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	if(!pcm_adc.capture_flag) {
		if (ak_ai_start_capture(handle)) {
			ak_print_error_ex("ak_ai_start_capture failed\n");
			return AK_FAILED;
		}
	}

	int ret = AK_FAILED;

    if (0 == ms) { // non-block mode
        ret = get_frame_non_block(handle, frame);
    } else if (ms < 0) { // block mode
        ret = get_frame_block(handle, frame);
    } else { // timeout mode
        ret = get_frame_timeout(handle, frame, ms);
    }

    if(!ret)
    	ai_save_stream_to_file(frame->data, frame->len, AI_FINAL_LEVEL);

	return ret;
}

/**
 * ak_ai_release_frame -
 * @handle[IN]: audio in opened handle
 * @frame[IN]: audio frame info
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ai_release_frame(void *handle, struct frame *frame)
{
	if(NULL == frame) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	int ret = AK_FAILED;

	ak_thread_mutex_lock(&(pcm_adc.read_mutex));
	if(frame->data) {
		ak_free(frame->data);
		frame->data = NULL;
		ret = AK_SUCCESS;
	} else {
		set_error_no(ERROR_TYPE_INVALID_ARG);
	}
	ak_thread_mutex_unlock(&(pcm_adc.read_mutex));

    return ret;
}

/**
 * ak_ai_set_capture_size - set audio capture size
 *		that read from AD driver each time
 * @handle[IN]: audio in opened handle
 * @capture_size[IN]: appointed frame interval, [512, 2048], unit: byte
 * return: 0 success, -1 failed
 * notes: 1. if you do not call, the default frame size is 2048
 *      2. set capture size before start capture.
 */
int ak_ai_set_capture_size(void *handle, unsigned int capture_size)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	if ((capture_size < ADC_CAP_BUF_MIN_LEN)
		|| (capture_size > ADC_CAP_BUF_MAX_LEN)) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	ak_print_info_ex("enter...\n");

	/* already set the same capture size */
	if (pcm_adc.capture_len == capture_size) {
		return AK_SUCCESS;
	}

	ak_print_notice_ex("set capture size from %d to %d\n",
		pcm_adc.capture_len, capture_size);
	/* we already malloc capture_buf at init time as max len(2048) */
	pcm_adc.capture_len = capture_size;

	ak_print_info_ex("leave...\n");

	return AK_SUCCESS;
}

/**
 * ak_ai_set_frame_interval - set audio frame interval, unit: ms
 * @handle[IN]: audio in opened handle
 * @frame_interval[IN]: audio frame interval, [10, 125], unit: ms
 * return: 0 success, -1 failed
 * notes: You must set frame interval before call ak_ai_get_frame()
 */
int ak_ai_set_frame_interval(void *handle, int frame_interval)
{
	if (pcm_adc.running) {
		/* ai capture had started */
		return AK_SUCCESS;
	}

	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	
	if ((frame_interval < 10) || (frame_interval > 200)) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	pcm_user_t *user = (pcm_user_t *)handle;

	user->interval = frame_interval;
	int frame_size = calc_size_by_rate(user, user->param.rate);
	ak_print_notice_ex("audio frame_size=%d\n", frame_size);

	/* set to diffirent frame size */
	if (pcm_adc.frame_size != frame_size) {
		pcm_adc.frame_size = frame_size;
	    pcm_adc.frame_interval = frame_interval;
	}
	ret = AK_SUCCESS;

	return ret;
}

/**
 * ak_ai_clear_frame_buffer -
 * @handle[IN]: audio in opened handle
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ai_clear_frame_buffer(void *handle)
{
	return pcm_reset_buf(handle, (void *)~0);
}


/**
 * ak_ai_set_adc_volume - set volume
 * @handle[IN]: audio in opened handle
 * @volume[IN]: new volume value, [0, 8]: 0-mute, 8-max adc volume
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ai_set_adc_volume(void *handle, int volume)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	if ((volume < 0) || (volume > ADC_DEV_MAX_VOLUME)) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	pcm_user_t *user = (pcm_user_t *)handle;

	ak_print_normal_ex("set volume %d\n",volume);
	if (0 == volume) {
		ret = set_adc_mute(user);
		/* close aslc */
		if (user->sdfilter_aslc) {
			sdfilter_close(user->sdfilter_aslc);
			user->sdfilter_aslc = NULL;
		}
	} else {
		/* reopen the volume if mute */
		if (user->mute_flag && pcm_set_source(user)) {
			return AK_FAILED;
		}

		pcm_adc_set_volume(user, volume - 1);
	}
	pcm_adc.adc_volume = volume;

	return ret;
}

/**
 * ak_ai_set_aslc_volume - set volume
 * @handle[IN]: audio in opened handle
 * @volume[IN]: new volume value, [0, 6]: 0-not use aslc, 6-max aslc volume
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ai_set_aslc_volume(void *handle, int volume)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	if ((volume < 0) || (volume > ADC_MAX_ASLC_VOLUME)) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	pcm_user_t *user = (pcm_user_t *)handle;

	ak_print_normal_ex("set volume %d\n",volume);

	/* open aslc */
	adc_set_aslc(user, volume);

	pcm_adc.aslc_volume = volume;

	return ret;
}


/**
 * ak_ai_get_adc_volume - get adc volume
 * @handle[IN]: audio in opened handle
 * return: success return volume value, [0, 8]: 0-mute, 8-max adc volume,
 			-1 failed
 * notes:
 */
int ak_ai_get_adc_volume(void *handle)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	int volume = pcm_adc.adc_volume;
	if ((volume < 0) || (volume > ADC_DEV_MAX_VOLUME)) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	return volume;
}

/**
 * ak_ai_get_aslc_volume - get aslc volume
 * @handle[IN]: audio in opened handle
 * return: success return volume value, [0, 6]: 0-not use aslc, 6-max aslc volume,
 			-1 failed
 * notes:
 */
int ak_ai_get_aslc_volume(void *handle)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	int volume = pcm_adc.aslc_volume;
	if ((volume < 0) || (volume > ADC_MAX_ASLC_VOLUME)) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	return volume;
}

/**
 * ak_ai_set_resample - set audio resampling
 * @handle[IN]: audio in opened handle
 * @enable[IN]: 0 close, 1 open
 * return: 0 success, -1 failed
 */
int ak_ai_set_resample(void *handle, int enable)
{
	/// 由于重采样会导致音频底噪问题，故此处去掉该功能。
//	if (NULL == handle) {
//		ak_print_error_ex("pointer NULL\n");
//		set_error_no(ERROR_TYPE_POINTER_NULL);
//		return AK_FAILED;
//	}
//
//	pcm_user_t *user = (pcm_user_t *)handle;
//	user->sdf_switch = enable;

	return AK_SUCCESS;
}

/**
 * ak_ai_set_nr_agc - adc nr&agc switch
 * @handle[IN]: opened audio input handle
 * @enable[IN]: 0 disable nr&agc, 1 enable nr&agc.
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ai_set_nr_agc(void *handle, int enable)
{
	if(!handle) {
        set_error_no(ERROR_TYPE_POINTER_NULL);
        ak_print_error_ex("handle is NULL \n");
        return AK_FAILED;
	}

	int ret = AK_FAILED;
	pcm_user_t *user = (pcm_user_t *)handle;

	/* NR & AGC function only support 8K sample, only normal voice */
	if (AK_AUDIO_SAMPLE_RATE_32000 <= user->param.rate) {
        set_error_no(ERROR_TYPE_FUNC_NOT_SUPPORT);
        ak_print_error_ex("set nr & agc, must sample rate < 32000 \n");
        return AK_FAILED;
	}

	ak_thread_mutex_lock(&pcm_adc.mutex);
	enum func_switch nr_agc = enable ? FUNC_ENABLE : FUNC_DISABLE;
	/* already set the same value */
	if (pcm_adc.nr_agc_switch == nr_agc) {
		ret = AK_SUCCESS;
		goto nr_agc_end;
	}

	/* mark it will be switched */
	pcm_adc.nr_agc_switch = nr_agc;

	/* ADC must be opened successfully, and start getting frame */
	if (AK_INVALID_FD == pcm_adc.fd) {
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
        ak_print_error_ex("fd invaild \n");
		goto nr_agc_end;
	}

	ret = AK_SUCCESS;

nr_agc_end:
	ak_thread_mutex_unlock(&pcm_adc.mutex);
	return ret;
}

/**
 * ak_ai_set_nr - adc nr switch
 * @handle[IN]: opened audio input handle
 * @enable[IN]: 0 disable nr, 1 enable nr.
 * return: 0 success, -1 failed
 * notes: This function is use to enable nr without agc,
 		  if want to enable nr and agc,use ak_ai_set_nr_agc with enable;
 		  if want to disable nr and agc,use ak_ai_set_nr_agc with disable.
 */
int ak_ai_set_nr(void *handle, int enable)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
        ak_print_error_ex("handle is NULL \n");
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	pcm_user_t *user = (pcm_user_t *)handle;

	/* NR function only support 8K sample, only normal voice */
	if (AK_AUDIO_SAMPLE_RATE_32000 <= user->param.rate) {
		set_error_no(ERROR_TYPE_FUNC_NOT_SUPPORT);
        ak_print_error_ex("set nr, must sample rate < 32000 \n");
		return AK_FAILED;
	}

	ak_thread_mutex_lock(&pcm_adc.mutex);
	enum func_switch nr = enable ? FUNC_ENABLE : FUNC_DISABLE;
	/* already set the same value */
	if (pcm_adc.nr_switch == nr) {
		ret = AK_SUCCESS;
		goto nr_end;
	}

    /* mark it will be switched */
    pcm_adc.nr_switch = nr;

	/* ADC must be opened successfully, and start getting frame */
	if (AK_INVALID_FD == pcm_adc.fd) {
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
        ak_print_error_ex("fd invaild \n");
		goto nr_end;
	}

	ret = AK_SUCCESS;

nr_end:
	ak_thread_mutex_unlock(&pcm_adc.mutex);
	return ret;
}

/**
 * ak_ai_set_nr_max - enable max nr level
 * @handle[IN]: audio in opened handle
 * @enable[IN]: 0 disable nr max, 1 enable nr max.
 * return: 0 success, -1 failed
 * notes: call after set ak_ai_set_nr_agc
 */
int ak_ai_set_nr_max(void *handle, int enable)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;

	/* NR & AGC function only support 8K sample, only normal voice */
	if (8000 != user->param.rate) {
		set_error_no(ERROR_TYPE_FUNC_NOT_SUPPORT);
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	ak_thread_mutex_lock(&pcm_adc.mutex);
	if (!pcm_adc.nr_agc_switch) {
		/* not enable nr&agc,no need to enable max nr level */
		ret = AK_FAILED;
		goto set_nr_max_end;
	}

	enum func_switch nr_max = enable ? FUNC_ENABLE : FUNC_DISABLE;
	if (pcm_adc.nr_max_switch == nr_max) {
		goto set_nr_max_end;
	}

	pcm_adc.nr_max_switch = nr_max;

	ak_print_normal_ex("set nr max level: %d\n", enable);
	if (ioctl(user->cp_fd, IOC_SET_NR_MAX, (void *)(&enable)) < 0) {
		ak_print_error("adc ioctl set nr_max failed\n");
		ret = AK_FAILED;
	}

set_nr_max_end:
	ak_thread_mutex_unlock(&pcm_adc.mutex);

	return ret;
}

/**
 * ak_ai_set_agc_attr -  set attribute
 * @handle[IN]: opened audio input handle
 * @agc_attr[IN]: agc attribute
 * return: 0 success, other failed
 * notes:
 */
int ak_ai_set_agc_attr(void *handle, struct ak_audio_agc_attr *agc_attr)
{
	if(!handle) {
		ak_print_error_ex("ai_handle_id is NULL\n");
		return AK_FAILED;
	}

	if(!agc_attr) {
		ak_print_error_ex("agc_attr is NULL\n");
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	ak_print_notice_ex("pcm_adc.prepare_flag=%d\n", pcm_adc.prepare_flag);

	if (pcm_adc.prepare_flag) { 	// already running
		struct echo_lib_param echo_param;
		if (adc_dev_get_echo_attr(user->cp_fd, &echo_param)) {
		}
        if (0 == echo_param.m_PreprocessEna && agc_attr->enable) {
            ak_print_error_ex("enable agc must enable nr first\n");
            return AK_FAILED;
        }

		copy_agc_attr(&echo_param, agc_attr);
		if (adc_dev_set_echo_attr(user->cp_fd, &echo_param)) {
			return AK_FAILED;
		}
		memcpy(&(user->echo_attr), &echo_param, sizeof(struct echo_lib_param));
	} else {
		memcpy(&(pcm_adc.agc_attr), agc_attr, sizeof(struct ak_audio_agc_attr));	
		pcm_adc.set_agc_attr_flag = 1;
	}

	return AK_SUCCESS;

}								

/**
 * ak_ai_get_agc_attr -  get attribute
 * @handle[IN]: audio in opened handle
 * @agc_attr[OUT]: agc attribute
 * return: 0 success, other failed
 * notes:
 */
int ak_ai_get_agc_attr(void *handle, struct ak_audio_agc_attr *agc_attr)
{
	if(!handle) {
		ak_print_error_ex("ai_handle_id is NULL\n");
		return AK_FAILED;
	}

	if(!agc_attr) {
		ak_print_error_ex("aec_attr is NULL\n");
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	ak_print_notice_ex("pcm_adc.prepare_flag=%d\n", pcm_adc.prepare_flag);

	if (pcm_adc.prepare_flag) { 	// already running
        struct echo_lib_param echo_param;
		if (adc_dev_get_echo_attr(user->cp_fd, &echo_param)) {
		}
        agc_attr->agc_level = echo_param.m_agcLevel;
        agc_attr->agc_max_gain = echo_param.m_maxGain;
        agc_attr->agc_min_gain = echo_param.m_minGain;
        agc_attr->near_sensitivity = echo_param.m_nearSensitivity;
        agc_attr->enable = echo_param.m_agcEna;
    } else {
        memcpy(agc_attr, &(pcm_adc.agc_attr), sizeof(struct ak_audio_agc_attr)); 
    }
    return AK_SUCCESS;
}

/**
 * ak_ai_set_aec_attr -  set attribute
 * @handle[IN]: opened audio input handle
 * @aec_attr[IN]: aec attribute
 * return: 0 success, other failed
 * notes:
 */
int ak_ai_set_aec_attr(void *handle, struct ak_audio_aec_attr *aec_attr)
{
	if(!handle) {
		ak_print_error_ex("ai_handle_id is NULL\n");
		return AK_FAILED;
	}

	if(!aec_attr) {
		ak_print_error_ex("nr_attr is NULL\n");
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	int ret = AK_SUCCESS;
	struct echo_lib_param echo_param;
    
	ak_print_notice_ex("pcm_adc.prepare_flag=%d\n", pcm_adc.prepare_flag);

 	if (pcm_adc.prepare_flag) { 	// already running
		if (adc_dev_get_echo_attr(user->cp_fd, &echo_param)) {
		}
        ak_print_notice_ex("audio_out_threshold=%ld\n", aec_attr->audio_out_threshold);
		copy_aec_attr(&echo_param, aec_attr);
		if (adc_dev_set_echo_attr(user->cp_fd, &echo_param)) {
            return AK_FAILED;
		}
		memcpy(&(user->echo_attr), &echo_param, sizeof(struct echo_lib_param));
	    //if (ioctl(user->cp_fd, IOC_SET_AEC, (void *)&(aec_enable))) {
    	//	ak_print_error_ex("send IOC_SET_AEC failed, value=%d\n", aec_enable);
    	//}
	} else {
		memcpy(&(pcm_adc.aec_attr), aec_attr, sizeof(struct ak_audio_aec_attr));	
		pcm_adc.set_aec_attr_flag = 1;
	}
	
	return ret;
}

/**
 * ak_ai_get_aec_attr -  get attribute
 * @handle[IN]: audio in opened handle
 * @aec_attr[OUT]: aec attribute 
 * @aec_enable[OUT]: 0 disable AEC, 1 enable AEC
 * return: 0 success, other failed
 * notes:
 */
int ak_ai_get_aec_attr(void *handle, struct ak_audio_aec_attr *aec_attr)
{
	if(!handle) {
		ak_print_error_ex("ai_handle_id is NULL\n");
		return AK_FAILED;
	}

    if(!aec_attr) {
        ak_print_error_ex("aec_attr is NULL\n");
        return AK_FAILED;
    }

	pcm_user_t *user = (pcm_user_t *)handle;
	
	ak_print_notice_ex("pcm_adc.prepare_flag=%d\n", pcm_adc.prepare_flag);

    if (pcm_adc.prepare_flag) {
        struct echo_lib_param echo_param;
		if (adc_dev_get_echo_attr(user->cp_fd, &echo_param)) {
            //return AK_FAILED;
		}      
        aec_attr->audio_in_digi_gain = echo_param.m_nearDigiGain;
        aec_attr->audio_out_digi_gain = echo_param.m_farDigiGain;
        aec_attr->audio_out_threshold = echo_param.m_farThreshold;
        aec_attr->enable = echo_param.m_aecEna;
        aec_attr->tail = echo_param.m_tail;
    } else {
        memcpy(aec_attr, &(user->echo_attr), sizeof(struct ak_audio_aec_attr));  
    }
    
	ak_print_notice_ex("audio_in_digi_gain=%d, audio_out_digi_gain=%d, audio_out_threshold=%ld, tail =%d\n",
        aec_attr->audio_in_digi_gain, aec_attr->audio_out_digi_gain, aec_attr->audio_out_threshold, aec_attr->tail);

    return AK_SUCCESS;
}

/**
 * ak_ai_set_nr_attr -  set nr attribute
 * @handle[IN]: opened audio input handle
 * @nr_attr[IN]: nr attribute
 * return: 0 success, other failed
 * notes:
 */
int ak_ai_set_nr_attr(void *handle, struct ak_audio_nr_attr *nr_attr)
{
	if(!handle) {
		ak_print_error_ex("ai_handle_id is NULL\n");
		return AK_FAILED;
	}

    if(!nr_attr) {
        ak_print_error_ex("aec_attr is NULL\n");
        return AK_FAILED;
    }
	pcm_user_t *user = (pcm_user_t *)handle;
    
	ak_print_notice_ex("pcm_adc.prepare_flag=%d\n", pcm_adc.prepare_flag);

    if (pcm_adc.prepare_flag) {     // already running
        struct echo_lib_param echo_param;
        if (adc_dev_get_echo_attr(user->cp_fd, &echo_param)) {
            ak_print_notice_ex("nr_attr->enable=%d\n", nr_attr->enable);
        }

        copy_nr_attr(&echo_param, nr_attr);// set parameter again
        if (adc_dev_set_echo_attr(user->cp_fd, &echo_param)) {
            return AK_FAILED;
        }
        memcpy(&(user->echo_attr), &echo_param, sizeof(struct echo_lib_param));
#if 0
        if (nr_attr->enable) {// need to open nr, but aec lib is not open
	        if (ioctl(user->cp_fd, IOC_SET_AEC, (void *)&(nr_attr->enable))) {// open aec lib
    		    ak_print_error_ex("send IOC_SET_AEC failed, value=%d\n", nr_attr->enable);
	        }
            adc_dev_set_echo_attr(user->cp_fd, &echo_param);
        }
#endif
    } else {
        memcpy(&(pcm_adc.nr_attr), nr_attr, sizeof(struct ak_audio_nr_attr));    
        pcm_adc.set_nr_attr_flag = 1;
    }       
    return AK_SUCCESS;
}

/**
 * ak_ai_get_nr_attr -  get nr attribute
 * @handle[IN]: audio in opened handle
 * @nr_attr[OUT]: nr attribute
 * return: 0 success, other failed
 * notes:
 */
int ak_ai_get_nr_attr(void *handle, struct ak_audio_nr_attr *nr_attr)
{
	if(!handle) {
		ak_print_error_ex("ai_handle_id is NULL\n");
		return AK_FAILED;
	}

	if(!nr_attr) {
		ak_print_error_ex("nr_attr is NULL\n");
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	ak_print_notice_ex("pcm_adc.prepare_flag=%d\n", pcm_adc.prepare_flag);

	if (pcm_adc.prepare_flag) { 	// already running
        struct echo_lib_param echo_param;
		if (adc_dev_get_echo_attr(user->cp_fd, &echo_param)) {
            return AK_FAILED;
		}
        nr_attr->noise_suppress_db = echo_param.m_noiseSuppressDb;
        nr_attr->enable = echo_param.m_PreprocessEna;
    } else {
        memcpy(nr_attr, &(pcm_adc.nr_attr), sizeof(struct ak_audio_nr_attr)); 
    }
    return AK_SUCCESS;
}

/**
 * ak_ai_set_aec - AEC switch
 * @handle[IN]: opened audio input handle
 * @enable[IN]: 0 disable AEC, 1 enable AEC
 * return: 0 success, -1 failed
 * notes:
 */
int ak_ai_set_aec(void *handle, int enable)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
        ak_print_error_ex("handle is NULL \n");
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	/* AEC function only support 8K sample, only normal voice */
	if (16000 < user->param.rate) {
		set_error_no(ERROR_TYPE_FUNC_NOT_SUPPORT);
        ak_print_error_ex("set nr, must sample rate < 16000 \n");
		return AK_FAILED;
	}

	int ret = AK_FAILED;

	ak_thread_mutex_lock(&pcm_adc.mutex);

	/* ADC must be opened successfully */
	if (AK_INVALID_FD == pcm_adc.fd) {
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		goto set_aec_end;
	}

	enum func_switch aec = enable ? FUNC_ENABLE : FUNC_DISABLE;
	/* already set the same value */
	if (pcm_adc.cur_aec_switch == aec) {
		ret = AK_SUCCESS;
		goto set_aec_end;
	}

	ak_print_notice_ex("switch AEC from %d to %d\n", pcm_adc.cur_aec_switch, aec);
	pcm_adc.cur_aec_switch = aec;

	if (pcm_adc.prepare_flag) {
	    ret = ioctl(user->cp_fd, IOC_SET_AEC, (void *)&aec);
    	if (ret) {
    		ak_print_error_ex("send IOC_SET_AEC failed, value=%d\n", aec);
    	}
	} else {
	    ret = AK_SUCCESS;
	}

set_aec_end:
	ak_thread_mutex_unlock(&pcm_adc.mutex);

	return ret;
}

/**
 * ak_ai_enable_agc - adc agc switch
 * @handle[IN]: opened audio input handle
 * @agc_enable[IN]: 0 disable agc, 1 enable agc.
 * return: 0 success, -1 failed
 * notes: if want to set agc,must set nr first
 */
int ak_ai_enable_agc(void *handle, int agc_enable)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
        ak_print_error_ex("handle is NULL \n");
		return AK_FAILED;
	}

	if (0 > agc_enable || 1 < agc_enable) {
		ak_print_error_ex("enable must be 0 or 1\n");
		return ERROR_TYPE_INVALID_ARG;
	}

	enum func_switch agc = agc_enable ? FUNC_ENABLE : FUNC_DISABLE;

	pcm_user_t *user = (pcm_user_t *)handle;

    /* AGC function only support <32000 sample */
	if (AK_AUDIO_SAMPLE_RATE_32000 <= user->param.rate) {
		ak_print_error_ex("set agc not support samplerate as %d\n", user->param.rate);
		return AK_FAILED;
	}

	/* ADC must be opened successfully, and start getting frame */
	if (AK_INVALID_FD == user->cp_fd) {
		ak_print_error_ex("dev->fd is AK_INVALID_FD!\n");
		return AK_FAILED;
	}

	/* if set agc, must first set nr */
	if (pcm_adc.nr_switch == FUNC_DISABLE && agc_enable) {	
		ak_print_error_ex("if set agc, must first set nr\n");
		return AK_FAILED;
	}

	if (pcm_adc.prepare_flag) { 	// already running
		struct echo_lib_param echo_param;
		if (adc_dev_get_echo_attr(user->cp_fd, &echo_param)) {
			return AK_FAILED;
		}

    	/* already set the same value */
    	if (echo_param.m_agcEna == agc) {
    		return AK_SUCCESS;
    	}

		echo_param.m_agcEna = agc;
		if (adc_dev_set_echo_attr(user->cp_fd, &echo_param)) {
			return AK_FAILED;
		}
		memcpy(&(user->echo_attr), &echo_param, sizeof(struct echo_lib_param));
	} else {
    	/* already set the same value */
    	if (pcm_adc.agc_switch == agc) {
    		return AK_SUCCESS;
    	}

    	/* mark it will be switched */
    	pcm_adc.agc_switch = agc;
	}

	return AK_SUCCESS;
}

/**
 * ak_ai_enable_eq - enable eq
 * @handle[IN]: audio out opened handle 
 * @enable[IN]: 1 enable,0 disable
 * return: 0 success -1 failed
 * notes:
 */
int ak_ai_enable_eq(void *handle, int enable)
{
	if (!handle) {
		ak_print_error_ex("handle is NULL\n");
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	return adc_dev_enable_eq(user->cp_fd, enable);
}

/**
 * ak_ai_set_eq_attr - set eq attribute
 * @handle[IN]: audio out opened handle 
 * @eq_attr[IN]: eq attribute
 * return: 0 success  -1 failed
 * notes:
 */
int ak_ai_set_eq_attr(void *handle, struct ak_audio_eq_attr *eq_attr)
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

    signed long peqpara[EQ_ARRAY_NUM];
    memset(peqpara, 0, sizeof(signed long) * EQ_ARRAY_NUM);
                
    if (get_eq_attr_param(user, eq_attr, peqpara)) {
        return AK_FAILED;
    }

    if (NULL == user->eq_attr){
        user->eq_attr = (struct ak_audio_eq_attr *)ak_calloc(1, sizeof(struct ak_audio_eq_attr));
        if (NULL == user->eq_attr) {
            ak_print_error_ex("malloc dev->eq_attr  failed\n");
            return AK_FAILED;
        }
    }

	if (adc_dev_set_eq_attr(user->cp_fd, peqpara)) {   
        /* set eq attr error, not nedd to free dev->eq_attr */
        return AK_FAILED;
	}
    memcpy(user->eq_attr, eq_attr, sizeof(struct ak_audio_eq_attr));

    return AK_SUCCESS;
}

/**
 * ak_ai_get_eq_attr - get eq attribute
 * @handle[IN]: audio in opened handle
 * @eq_attr[OUT]: eq attribute
 * @eq_enable[OUT]: 1 enable,0 disable
 * return: 0 success  -1 failed
 * notes:
 */
int ak_ai_get_eq_attr(void *handle, struct ak_audio_eq_attr *eq_attr)
{
	if (!handle) {
		ak_print_error_ex("handle is NULL\n");
		return AK_FAILED;
	}

    if(!eq_attr) {
        ak_print_error_ex("eq_attr is NULL\n");
        return ERROR_TYPE_POINTER_NULL;
    }

	pcm_user_t *user = (pcm_user_t *)handle;

    if (user->eq_attr) {
        memcpy(eq_attr, user->eq_attr, sizeof(struct ak_audio_eq_attr));
        if (user->eq_attr->bands) {
            eq_attr->enable = 1;
        }
    } else {
        ak_print_error_ex("eq not open!\n");
        return AK_FAILED;
    }
    
    return AK_SUCCESS;
}

/**
 * ak_ai_set_source - set audio input source, linein or mic
 * @handle[IN]: opened audio input handle
 * @src[IN]: appointed source, default AI_SOURCE_AUTO
 * return: 0 success, -1 failed
 */
int ak_ai_set_source(void *handle, enum ai_source src)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex("handle is NULL \n");
		return AK_FAILED;
	}

	if (AI_SOURCE_AUTO == src) {
		ak_print_error_ex("now AI_SOURCE_AUTO is not support \n");
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	if (user->cp_fd != pcm_adc.fd) {
		return AK_FAILED;
	}

	pcm_ft_t *pft = &(user->ak_ft);
	int ret = AK_SUCCESS;

	switch (src) {
	case AI_SOURCE_MIC:
		pft->dev = AKPCM_CPTRDEV_MIC;
		break;
	case AI_SOURCE_LINEIN:
		pft->dev = AKPCM_CPTRDEV_LI;
		break;
	default:
		ret = AK_FAILED;
		ak_print_error_ex("now source= %d is not support \n", src);
		set_error_no(ERROR_TYPE_FUNC_NOT_SUPPORT);
		break;
	}

	if ((pcm_adc.dev_hw != user->ak_ft.dev) && !user->mute_flag) {
		if (pcm_set_source(user)) {
			ret = AK_FAILED;
		}
	}

	return ret;
}

/**
 * ak_ai_get_source - get audio input source, linein or mic
 * @handle[IN]: opened audio input handle
 * return: success return appointed source, -1 failed
 */
enum ai_source ak_ai_get_source(void *handle)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex("handle is NULL \n");
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;

	if (AKPCM_CPTRDEV_MIC == user->ak_ft.dev)
		return AI_SOURCE_MIC;
	else if (AKPCM_CPTRDEV_LI == user->ak_ft.dev)
		return AI_SOURCE_LINEIN;
	else
		return AK_FAILED;
}

/**
 * ak_ai_get_runtime_status - get ai run time status
 * @handle[IN]: audio in opened handle
 * @status[OUT]: get ai other status :nr&agc resample aec volume source
 * return: 0 success, -1 failed
 * notes: call after set all kind of ADC attr
 */
int ak_ai_get_runtime_status(void *handle,
							struct ai_runtime_status *status)
{
	if (!handle || !status) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	status->nr_agc_enable = pcm_adc.nr_agc_switch;
	status->resample_enable = user->sdf_switch;
	status->aec_enable = pcm_adc.cur_aec_switch;
	status->actual_rate = user->actual_rate;
	status->nr_max_enable = pcm_adc.nr_max_switch;

	return AK_SUCCESS;
}

/**
 * ak_ai_save_aec_dump_file - whether save aec dump file
 * @handle[IN]: audio in opened handle
 * @enable[IN]: 0 disable , 1 enable.
 * return: 0 success, -1 failed
 * notes: call after set all kind of ADC attr
 */
int ak_ai_save_aec_dump_file(void *handle, int enable)
{
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	if (user->cp_fd != pcm_adc.fd) {
		return AK_FAILED;
	}

	int value = enable > 0 ? 1 : 0;
	
	if (ioctl(user->cp_fd, IOC_SET_AEC_DUMP, (void *)(&value)) < 0) {
		ak_print_error("adc ioctl set save dump file failed\n");
		return AK_FAILED;
	}
	return AK_SUCCESS;
}

/**
 * ak_ai_close - close audio input
 * @handle[IN]: opened audio input handle
 * return: 0 success, -1 failed
 */
int ak_ai_close(void *handle)
{
	ak_print_info_ex("enter\n");
	if(!handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	pcm_user_t *user = (pcm_user_t *)handle;
	if (AK_INVALID_FD == user->cp_fd) {
		ak_print_error_ex("fd error\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	if (user->cp_fd == pcm_adc.fd) {
		ret = pcm_adc_close(user);
	}

	pcm_adc.set_agc_attr_flag = 0;
	pcm_adc.set_aec_attr_flag = 0;
    pcm_adc.set_nr_attr_flag = 0;
	
	ai_sysipc_unbind_handle();
	ai_sys_ipc_unregister();
	pcm_adc.ai_handle = NULL;
	ak_free(user);
	user = NULL;
	ak_print_info_ex("leave\n");

	return ret;
}
