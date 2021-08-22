#include "sdcodec.h"
#include "pcm.h"
#include "internal_error.h"

#include "ak_common.h"
#include "ak_error.h"
#include "ak_thread.h"
#include "ak_ai.h"
#include "ak_aenc.h"

#define CALC_AUDIO_CAPTURE			1
#define AENC_SYNC_FRAME_TS			0
#define AUDIO_CALLBACK_DEBUG		0
#define AENC_PUT_STREAM_DEBUG		0
#define AENC_GET_STREAM_DEBUG		0
#define AENC_WRITE_AUDIO_FILE		0

#define AENC_TYPE_MAX_USER			(sizeof(int) * 8)

#define AUDIO_INFORM_LENGTH 		8
#define AUDIO_QUEUE_MAX_ITEM 		500

#define AAC_SINGLE_INPACKET_SIZE 	2048	// aac one channel input is 2k
#define AAC_STEREO_INPACKET_SIZE	4096	// aac two channels input is 4k

/*  supported audio encode type */
enum audio_encode_type {
    AUDIO_ENCODE_AAC = 0x00,
    AUDIO_ENCODE_G711A,
    AUDIO_ENCODE_G711U,
    AUDIO_ENCODE_AMR,
    AUDIO_ENCODE_MP3,
    AUDIO_ENCODE_RAW_PCM,
    AUDIO_ENCODE_TYPE_MAX 
};

struct aenc_frame{
    struct frame frame;			//audio pcm frame
    struct list_head list;
};

struct audio_entry{
    int ref;					//Reference count
    int bit_map;				//user map
    struct audio_stream stream;	//real data stream
    struct list_head list;
};

struct audio_encode_user{
	int group_index;			//audio encode type aenc_group index
	int req_nr;					//request number
    void *enc_handle;			//audio encode handle
};

struct audio_enc_group {
    int enc_type;				//audio encode type in T_AUDIO_TYPE(sdcodec.h)
    int ref;					//Reference count
    int user_map;				//request user bit map
    unsigned long pre_seq_no;	//previous stream sequence no according to frame
    void *enc_handle;			//encode handle
    ak_mutex_t mutex;
    struct list_head data;		//stream data queue head
};

struct audio_encode_ctrl {
    unsigned char run_flag;		//audio run flag
    unsigned char read_flag;	//read pcm flag
    ak_mutex_t request_mutex;	//request mutex

    int encode_len;
    unsigned char *encode_buf;
    
    ak_sem_t ad_sem;
    ak_sem_t enc_sem;
    ak_pthread_t pcm_tid;
    ak_pthread_t enc_tid;

	void *ai_handle;			//audio input handle
	ak_mutex_t frame_mutex;		//frame list mutex
	struct list_head frame_head;//frame list head

#if AENC_SYNC_FRAME_TS
    unsigned long long first_ts;	//the first ts
	unsigned long long interval_ts;	//at the first ts per 10 seconds
	struct ak_timeval first_time;	//get first frame time 
	struct ak_timeval sync_time;	//sync ts time
#endif

#if CALC_AUDIO_CAPTURE
	struct ak_timeval calc_time;
	int audio_num;
#endif    
};

struct aac_encode_buf {
	int ref;					//Reference count
    unsigned char *data;		//AAC encode buffer
    unsigned int len;			//AAC encode buffer len in bytes
    unsigned int offset;		//used buffer offset
};

struct audio_encode {
    int enc_type;				//audio encode type in T_AUDIO_TYPE(sdcodec.h)
    void *lib_handle;			//encode lib handle
};

static const char aenc_version[] = "libmpi_aenc V2.1.03";

static struct aac_encode_buf aac_buf = {0};
static struct audio_enc_group aenc_group[AUDIO_ENCODE_TYPE_MAX] = {
	{ _SD_MEDIA_TYPE_AAC, 		0,0,0},
	{ _SD_MEDIA_TYPE_PCM_ALAW, 	0,0,0},
	{ _SD_MEDIA_TYPE_PCM_ULAW, 	0,0,0},
	{ _SD_MEDIA_TYPE_AMR, 		0,0,0},
	{ _SD_MEDIA_TYPE_MP3, 		0,0,0},
	{ _SD_MEDIA_TYPE_PCM, 		0,0,0}
};

#ifdef AK_RTOS
static struct audio_encode_ctrl aenc_ctrl = {0, 0, 0, 0};
ak_mutex_t *g_aenc_req_lock __attribute__((__section__(".lock_init"))) = 
	&aenc_ctrl.request_mutex;
#else
static struct audio_encode_ctrl aenc_ctrl = {
	0, 0, PTHREAD_MUTEX_INITIALIZER, 0,NULL
};
#endif

#if AENC_WRITE_AUDIO_FILE
static int audio_aenc_flag = AK_TRUE;
static struct ak_timeval audio_aenc_time;
static FILE *audio_aenc_fp = NULL;

static void audio_write_aenc_file(int enc_type, struct audio_entry *entry)
{
	if (!audio_aenc_flag) {
		return;
	}
	
	if (audio_aenc_flag && !audio_aenc_fp) {
		switch(enc_type) {
		case AK_AUDIO_TYPE_PCM_ALAW:
			audio_aenc_fp = fopen("./aenc_audio.g711a", "w+");
			break;
		case AK_AUDIO_TYPE_PCM_ULAW:
			audio_aenc_fp = fopen("./aenc_audio.g711u", "w+");
			break;
		case AK_AUDIO_TYPE_AMR:
			audio_aenc_fp = fopen("./aenc_audio.amr", "w+");
			if (audio_aenc_fp) {
				const unsigned char amrheader[]= "#!AMR\n";
				fwrite(amrheader, sizeof(amrheader) - 1, 1, audio_aenc_fp);
			}
			break;
		default:
			return;
		}
		
		if(!audio_aenc_fp) {
			ak_print_error_ex("create mux audio file error\n");
			return;
		}
		ak_get_ostime(&audio_aenc_time);
	}

	struct ak_timeval cur_time;
	ak_get_ostime(&cur_time);
	if(ak_diff_ms_time(&cur_time, &audio_aenc_time) >= 60*1000) {
		ak_print_normal("write audio file time is up\n");
		fclose(audio_aenc_fp);
    	audio_aenc_fp = NULL;
    	audio_aenc_flag = AK_FALSE;
		return;
	}

	if(fwrite(entry->stream.data, 1, entry->stream.len, audio_aenc_fp) < 0) {
		ak_print_error_ex("write audio file err\n");
	}
}
#endif

#if AUDIO_CALLBACK_DEBUG
static void audio_cb_print(const char *format, ...)
{
	//REC_TAG, used to identify print informations of media lib
}

static void* audio_cb_malloc(unsigned long size)
{
	ak_print_info("malloc size 0x%lx\n", size);
	return malloc(size);
}

static void audio_cb_free(void *mem)
{
	ak_print_info("free buffer %p", mem);
	return free(mem);
}
#endif

static unsigned char audio_cb_delay(unsigned long ticks)
{
	ak_print_info("delay 0x%lx ticks", ticks);
	ak_sleep_ms(ticks);
	
	return 1;
}

/**
 * pcm_to_aac - encode the pcm data to AAC
 * @encode[IN]: audio encode handle
 * @frame[IN]: the audio pcm raw data info
 * @stream[OUT]: encode out stream info
 * return: >=0 encoded size, otherwise failed
 */
static int pcm_to_aac(struct audio_encode *encode,
					const struct frame *frame,
					struct audio_stream *stream)
{
	int ret = 0;
	int out_len = 0;
	unsigned int offset = 0;
	int frame_len = frame->len;

    T_AUDIO_ENC_BUF_STRC enc_info = {0};

	enc_info.buf_out = stream->data;
	enc_info.len_out = stream->len;

	while (frame_len > 0) {
		if ((frame_len < (aac_buf.len - aac_buf.offset))) {
			memcpy(&(aac_buf.data[aac_buf.offset]), &(frame->data[offset]),
				frame_len);
			aac_buf.offset += frame_len;
			return out_len;
		} else if ((frame_len >= (aac_buf.len - aac_buf.offset))) {
			memcpy(&(aac_buf.data[aac_buf.offset]), &(frame->data[offset]),
				(aac_buf.len - aac_buf.offset));
			offset += (aac_buf.len - aac_buf.offset);
			frame_len -= (aac_buf.len - aac_buf.offset);
			aac_buf.offset = aac_buf.len;
		} else {
			ak_print_error("pcm_to_aac how could this happend?\n" );
			ret = -1;
			break;
		}

		enc_info.buf_in	= aac_buf.data;
		enc_info.len_in = aac_buf.len;
		enc_info.buf_out = stream->data + out_len;
		enc_info.len_out = stream->len - out_len;

		if ( enc_info.len_out <= 0 ) {
			ak_print_error("pcm_to_aac the encode out buffer too small!\n" );
			ret = -1;
			break;
		}
		
		ret = _SD_Encode(encode->lib_handle, &enc_info);
		if (ret < 0) {
			break;
		}
		out_len += ret;
		aac_buf.offset = 0;
	}

	if (ret < 0)
		aac_buf.offset = 0;

	return ret;
}

static int init_aac_input(const struct audio_param *param,
						T_AUDIO_REC_INPUT *config)
{
	unsigned int tmp_len = 0;
	
	switch(param->channel_num) {
	case AUDIO_CHANNEL_MONO:
		tmp_len = AAC_SINGLE_INPACKET_SIZE;
		break;
	case AUDIO_CHANNEL_STEREO:
		tmp_len = AAC_STEREO_INPACKET_SIZE;
		break;
	default:
		ak_print_error_ex("not support %u channels!\n", param->channel_num);
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	if (aac_buf.len < tmp_len) {
		aac_buf.len = tmp_len;
		if (aac_buf.data) {
			free(aac_buf.data);
			aac_buf.data = NULL;
		}
		
		aac_buf.data = (unsigned char *)calloc(1, aac_buf.len);
		if (!aac_buf.data) {
			ak_print_error_ex("out of memory\n");
			set_error_no(ERROR_TYPE_MALLOC_FAILED);
			return AK_FAILED;
		}
	}
	
	add_ref(&(aac_buf.ref), 1);
	config->enc_in_info.m_private.m_aac.cutAdtsHead = _SD_ENC_CUT_FRAME_HEAD;

	return AK_SUCCESS;
}

/**
 * init_encode_input - init audio encode info
 * @param[IN]: audio pcm data encode param
 * @config[OUT]: audio encode lib input config
 * return: void
 */
static int init_encode_input(const struct audio_param *param,
							T_AUDIO_REC_INPUT *config)
{
	if ((param->type <= AK_AUDIO_TYPE_UNKNOWN) 
		|| (param->type > AK_AUDIO_TYPE_SPEEX)) {
		ak_print_error_ex("Unknow encode type!\n" );
		return AK_FAILED;
	}

	/* set the call back function */
#if AUDIO_CALLBACK_DEBUG
	config->cb_fun.Malloc = audio_cb_malloc;
	config->cb_fun.Free = audio_cb_free;
	config->cb_fun.printf = audio_cb_print;
#else
	config->cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	config->cb_fun.Free = free;
	config->cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
#endif
	config->cb_fun.delay = audio_cb_delay;

	/* set the encode input pcm data param */
	config->enc_in_info.m_nChannel = param->channel_num;
	config->enc_in_info.m_nSampleRate = param->sample_rate;
	config->enc_in_info.m_BitsPerSample = param->sample_bits;
	config->chip = AUDIOLIB_CHIP_AK39XXEV2;
	
	config->enc_in_info.m_Type = param->type;
	switch (param->type) {
	case AK_AUDIO_TYPE_AAC:
		if (init_aac_input(param, config)) {
			return AK_FAILED;
		}
		break;
    case AK_AUDIO_TYPE_AMR:
        config->enc_in_info.m_private.m_amr_enc.mode = AMR_ENC_MR122;
        break;
	default://other types do not set any other param
		break;
	}

	return AK_SUCCESS;
}

/**
 * init_audio_encode - init audio encode info
 * @enc_len[IN]: audio encode buffer len in bytes
 * return: void
 */
static void init_audio_encode(int enc_len)
{
	int i = 0;

	for (i=0; i<AUDIO_ENCODE_TYPE_MAX; ++i) {
		aenc_group[i].ref = 0;
	    aenc_group[i].enc_handle = NULL;
	    ak_thread_mutex_init(&aenc_group[i].mutex);
	    INIT_LIST_HEAD(&(aenc_group[i].data));
	}

	aenc_ctrl.encode_len = enc_len;
    aenc_ctrl.encode_buf = (unsigned char *)calloc(1, (aenc_ctrl.encode_len << 1));
    if (NULL == aenc_ctrl.encode_buf) {
        ak_print_error("can't calloc audio encode buffer\n");
    }
}

/**
 * malloc_aenc_entry - malloc audio encode stream entry
 * @len[IN]: encoded audio data len
 * return: pointer of audio encode stream entry info, NULL failed
 */
static struct aenc_entry* malloc_aenc_entry(int len)
{
	if (len <= 0)
		return NULL;
		
	struct aenc_entry *entry = (struct aenc_entry *)calloc(1,
		sizeof(struct aenc_entry));
	if (entry) {
		entry->stream.data = calloc(1, len);
        if (NULL == entry->stream.data) {
            ak_print_error_ex("calloc %d failed\n", len);
            free(entry);
	    	entry = NULL;
        }
	}

    return entry;
}

/**
 * malloc_audio_entry - malloc audio data entry info
 * @len[IN]: encoded audio data len
 * return: pointer of audio entry info, NULL failed
 */
static struct audio_entry* malloc_audio_entry(int len)
{
	struct audio_entry *entry = (struct audio_entry *)calloc(1,
		sizeof(struct audio_entry));
	if (NULL != entry) {
		entry->stream.data= calloc(1, len);
        if (NULL == entry->stream.data) {
            ak_print_error_ex("calloc %d failed\n", len);
            free(entry);
	    	entry = NULL;
        }
	}

    return entry;
}

static void free_audio_entry(struct audio_entry *entry)
{
	if (NULL != entry) {
    	if (NULL != entry->stream.data) {
    		free(entry->stream.data);
    		entry->stream.data = NULL;
    	}

		list_del_init(&(entry->list));
	    free(entry);
		entry = NULL;
	}
}

static void release_capture_data(void)
{
	struct aenc_frame *entry = NULL;
	struct aenc_frame *ptr = NULL;

	ak_thread_mutex_lock(&(aenc_ctrl.frame_mutex));
	list_for_each_entry_safe(entry, ptr, &(aenc_ctrl.frame_head), list) {
		list_del_init(&(entry->list));
		ak_ai_release_frame(aenc_ctrl.ai_handle, &(entry->frame));
		free(entry);
		entry = NULL;
	}
	ak_thread_mutex_unlock(&(aenc_ctrl.frame_mutex));
}

static void release_encode_data(int index)
{
	struct audio_entry *pos = NULL;
	struct audio_entry *n = NULL;

    list_for_each_entry_safe(pos, n, &(aenc_group[index].data), list){
		free_audio_entry(pos);
	}
}

/**
 * push_audio_queue - push encoded audio data into queue
 * @index[IN]: current audio encode type index
 * @data[IN]: encoded audio data
 * @len[IN]: encoded audio data len
 * @origin_frame[IN]: audio origin frame
 * return: 0 success, -1 failed
 */
static int push_audio_queue(enum audio_encode_type index,
							const unsigned char *data,
							int len,
							struct frame *origin_frame)
{
	int ret = AK_FAILED;
	struct audio_entry *entry = malloc_audio_entry(len);
	
	if (NULL != entry) {
	    memcpy(entry->stream.data, data, len);
	    entry->stream.len = len;
	    entry->stream.ts = origin_frame->ts;
	    entry->stream.seq_no = origin_frame->seq_no;
	    entry->ref = aenc_group[index].ref;

	    /* set entry user bit map */
	    entry->bit_map = aenc_group[index].user_map;

		if((origin_frame->seq_no > 0)
			&& (aenc_group[index].pre_seq_no + 1) != origin_frame->seq_no){
			ak_print_warning_ex("len=%u, ts=%llu, pre_seq_no=%lu, seq_no=%lu\n", 
				entry->stream.len, entry->stream.ts, 
				aenc_group[index].pre_seq_no, entry->stream.seq_no);
		}
		aenc_group[index].pre_seq_no = origin_frame->seq_no;

#if AENC_PUT_STREAM_DEBUG
		ak_print_notice_ex("encode: ts=%llu, len=%u, seq_no=%lu\n", 
			entry->stream.ts, entry->stream.len, entry->stream.seq_no);
#endif

		ak_thread_mutex_lock(&aenc_group[index].mutex);
	    list_add_tail(&(entry->list), &(aenc_group[index].data));
	    ak_thread_mutex_unlock(&aenc_group[index].mutex);
	    ret = AK_SUCCESS;
	}

	return ret;
}

/**
 * aenc_find_user - find current user pos in the encoded audio data queue
 * @user[IN]: current encode user
 * @head[IN]: audio encode stream list head
 * return: audio entry  success, NULL failed
 */
static struct audio_entry* aenc_find_user(struct audio_encode_user *user,
					struct list_head *head)
{
	unsigned char find = AK_FALSE;
	struct audio_entry *entry = NULL;
	
	list_for_each_entry(entry, head, list) {
#if AENC_GET_STREAM_DEBUG	
		ak_print_notice_ex("bit_map=%d, ref=%d\n", entry->bit_map, entry->ref);
#endif
		if (test_bit(user->req_nr, &(entry->bit_map))) {
			/* this entry have not gotten yet */
			find = AK_TRUE;
			break;
		}
	}
	if (!find) {
		entry = NULL;
	}

	return entry;
}

#if CALC_AUDIO_CAPTURE	
static void calc_audio_capture_num(void)
{
	struct ak_timeval cur_time;

	ak_get_ostime(&cur_time);
	++(aenc_ctrl.audio_num);
	long diff_time = ak_diff_ms_time(&cur_time, &(aenc_ctrl.calc_time));
	
	/* calc frame number per 10 seconds */
	if(diff_time >= 10*1000) {
		int seconds =  (diff_time / 1000);

		if (seconds > 0) {
			ak_print_info("*** audio info, seconds=%d, total=%d, "
				"average=%d ***\n\n", seconds, 
				aenc_ctrl.audio_num, (aenc_ctrl.audio_num / seconds));
		}
		
		aenc_ctrl.audio_num = 0;
		ak_get_ostime(&(aenc_ctrl.calc_time));
	}
}
#endif

#if AENC_SYNC_FRAME_TS
static void sync_aenc_frame_ts(unsigned long long cur_ts)
{
	if (aenc_ctrl.first_ts == 0) {
		aenc_ctrl.first_ts = cur_ts;
		aenc_ctrl.interval_ts = cur_ts;
		ak_get_ostime(&(aenc_ctrl.first_time));
		ak_get_ostime(&(aenc_ctrl.sync_time));
	}

	struct ak_timeval cur_time;

	ak_get_ostime(&cur_time);
	/* calc frame number per ten seconds */
	if(ak_diff_ms_time(&cur_time, &(aenc_ctrl.sync_time)) >= 10*1000) {
		unsigned long long target_ts = aenc_ctrl.interval_ts + 10*1000;
		unsigned long long total_ts = aenc_ctrl.first_ts + ak_diff_ms_time(&cur_time, 
			&(aenc_ctrl.first_time));
		ak_print_normal("*** first_ts=%llu, cur_ts=%llu, target_ts=%llu, total_ts=%llu ***\n", 
			aenc_ctrl.first_ts, cur_ts, target_ts, total_ts);
		ak_print_normal("*** aenc 10 sec diff=%lld, total diff=%lld ***\n\n", 
			(cur_ts - target_ts), (cur_ts - total_ts));

		ak_get_ostime(&(aenc_ctrl.sync_time));
		aenc_ctrl.interval_ts = cur_ts;
	}
}
#endif

static void encode_audio_data(struct frame *origin_frame)
{
	int i = 0;
	int ret = 0;

    for (i=0; i<AUDIO_ENCODE_TYPE_MAX; ++i) {
        if(0 == aenc_group[i].ref){
            continue;
        }

    	struct frame frame = {0};
    	frame.data = origin_frame->data;
    	frame.len = origin_frame->len;
    	frame.seq_no = origin_frame->seq_no;

    	struct audio_stream stream = {0};
    	stream.data = aenc_ctrl.encode_buf;
    	stream.len = (aenc_ctrl.encode_len << 1);
    	stream.seq_no = origin_frame->seq_no;
    	
        ret = ak_aenc_send_frame(aenc_group[i].enc_handle, &frame, &stream);
		if (ret > 0) {
            push_audio_queue(i, aenc_ctrl.encode_buf, ret, origin_frame);
		} else {
			ak_print_warning_ex("aenc frame ret=%d\n", ret);
		}
    }
}

static void encode_user_data(void)
{
	struct aenc_frame *entry = NULL;
	struct aenc_frame *ptr = NULL;
	
	list_for_each_entry_safe(entry, ptr, &(aenc_ctrl.frame_head), list) {
		encode_audio_data(&(entry->frame));
		
		ak_thread_mutex_lock(&(aenc_ctrl.frame_mutex));
		list_del_init(&(entry->list));
		ak_ai_release_frame(aenc_ctrl.ai_handle, &(entry->frame));
		free(entry);
		entry = NULL;
		ak_thread_mutex_unlock(&(aenc_ctrl.frame_mutex));
	}
}

static int read_to_encode(void)
{
	int ret = AK_SUCCESS;
	long timeout = 0;
	struct aenc_frame *entry = (struct aenc_frame *)calloc(1, 
		sizeof(struct aenc_frame));
		
	if (entry) {
		/* get the pcm data */
		int ret = ak_ai_get_frame(aenc_ctrl.ai_handle, &(entry->frame), timeout);
		if (ret) {
			free(entry);
			entry = NULL;
			
			ak_print_error("read audio from AD error! ret = %d\n", ret);
			ak_print_normal("errno=%d, str: %s\n", 
				ak_get_error_no(), ak_get_error_str(ak_get_error_no()));
			ret = AK_FAILED;
		} else {
			ak_thread_mutex_lock(&(aenc_ctrl.frame_mutex));
			list_add_tail(&(entry->list), &(aenc_ctrl.frame_head));
			ak_thread_mutex_unlock(&(aenc_ctrl.frame_mutex));
			ak_thread_sem_post(&aenc_ctrl.enc_sem);
			
#if AENC_SYNC_FRAME_TS
			sync_aenc_frame_ts(entry->frame.ts);
#endif			
		}
	}

	return ret;
}

/* read audio pcm data from AD */
static void* read_pcm_thread(void *arg)
{
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	ak_ai_clear_frame_buffer(aenc_ctrl.ai_handle);
    
	while (aenc_ctrl.run_flag) {
        ak_print_normal_ex("sleep...\n");
		ak_thread_sem_wait(&aenc_ctrl.ad_sem);
        ak_print_normal_ex("wakeup, encode_len=%u\n", aenc_ctrl.encode_len);

#if CALC_AUDIO_CAPTURE
		aenc_ctrl.audio_num = 0;
		ak_get_ostime(&(aenc_ctrl.calc_time));
#endif

		ak_print_notice_ex("aenc_ctrl.read_flag=%d\n", aenc_ctrl.read_flag);
		while (aenc_ctrl.read_flag) {
        	read_to_encode();
#if CALC_AUDIO_CAPTURE
			calc_audio_capture_num();
#endif
			ak_sleep_ms(2);
        }

        ak_print_normal_ex("read while exit\n");
	}

    ak_print_normal_ex("### thread id: %ld exit ###\n\n", ak_thread_get_tid());

	ak_thread_exit();
	return NULL;
}

/* encode audio frame and push into audio stream list */
static void* audio_encode_thread(void *arg)
{
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());		
	while (aenc_ctrl.run_flag) {
		ak_thread_sem_wait(&aenc_ctrl.enc_sem);
		
		if (aenc_ctrl.run_flag) {
			encode_user_data();
		}
	}

    ak_print_normal_ex("### thread id: %ld exit ###\n\n", ak_thread_get_tid());

	ak_thread_exit();
	return NULL;
}

/**
 * start_encode_ctrl - start audio encode control
 * @ai_handle[IN]: audio in handle
 * return: 0 success, -1 failed
 */
static int start_encode_ctrl(void *ai_handle)
{
	pcm_user_t *user = (pcm_user_t *)ai_handle;
	unsigned int sample_rate = user->param.rate;
	unsigned int bytes_per_sample = (user->param.sample_bits >> 3);
	unsigned int channel_num = user->param.channels;
	unsigned int interval = user->interval;

	/*read_pcm_thread or audio_encode_thread not exit,wait it*/
	while (NULL != aenc_ctrl.encode_buf){
		ak_print_notice_ex("wait mpi audio thread exit\n");
		ak_sleep_ms(10);
	}

	/* audio size we can get once */
	unsigned int read_len = (sample_rate * channel_num * interval
		* bytes_per_sample ) / 1000;
	if (read_len & 1) {
		++read_len;
	}

	init_audio_encode(read_len);
	aenc_ctrl.ai_handle = user;
	INIT_LIST_HEAD(&(aenc_ctrl.frame_head));
	ak_thread_mutex_init(&(aenc_ctrl.frame_mutex));
    ak_thread_sem_init(&aenc_ctrl.ad_sem, 0);
    ak_thread_sem_init(&aenc_ctrl.enc_sem, 0);

	/* create the capture data thread */
	aenc_ctrl.run_flag = AK_TRUE;
	int ret = ak_thread_create(&(aenc_ctrl.pcm_tid), read_pcm_thread,
			NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if (ret) {
		ak_print_error("create read_pcm_thread FAILED, ret=%d\n", ret);
		goto encode_ctrl_end;
	}

	ret = ak_thread_create(&(aenc_ctrl.enc_tid), audio_encode_thread,
			NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if (ret) {
		ak_print_error("create audio_encode_thread thread FAILED, ret=%d\n", ret);
	}

encode_ctrl_end:
	if (ret) {
		aenc_ctrl.ai_handle = NULL;
		ak_thread_mutex_destroy(&(aenc_ctrl.frame_mutex));
		ak_thread_sem_destroy(&aenc_ctrl.ad_sem);
		ak_thread_sem_destroy(&aenc_ctrl.enc_sem);
		if (NULL != aenc_ctrl.encode_buf){
        	free(aenc_ctrl.encode_buf);
        	aenc_ctrl.encode_buf = NULL;
			aenc_ctrl.run_flag = AK_FALSE;
    	}
	}
	
	return ret;
}

/** 
 * ak_aenc_print_codec_info - print audio codec version & support functions
 * notes: encode such as: MP3 encode, AAC encode and so on
 *		decode such as: MP3 decode, AAC decode and so on
 */
void ak_aenc_print_codec_info(void)
{
    T_AUDIO_CB_FUNS cb_codc={0};
    
    cb_codc.printf = print_normal;
    _SD_GetAudioCodecVersions(&cb_codc);
}

const char* ak_aenc_get_version(void)
{
	return aenc_version;
}

/**
 * ak_aenc_open - open anyka audio encode
 * @param[IN]: audio pcm data encode param
 * return: audio encode handle, NULL failed
 */
void* ak_aenc_open(const struct audio_param *param)
{
	if (!param) {
		ak_print_error_ex("invalid audio encode param\n");
		return NULL;
	}

    struct audio_encode *encode = (struct audio_encode *)calloc(1, 
    	sizeof(struct audio_encode));
    if (!encode) {
        return NULL;
    }

	/* pcm not need to open _SD_Encode_Open */
    if (AK_AUDIO_TYPE_PCM == param->type) {		
		encode->enc_type = param->type;
		encode->lib_handle = NULL;
		return encode;
    }

	int ret = AK_FAILED;
	T_AUDIO_REC_INPUT in_cfg;

	memset(&in_cfg, 0x00, sizeof(T_AUDIO_REC_INPUT));
	if (init_encode_input(param, &in_cfg)) {
		goto open_end;
	}

    T_AUDIO_ENC_OUT_INFO out_cfg;
	memset(&out_cfg, 0x00, sizeof(T_AUDIO_ENC_OUT_INFO));

	/* open anyka audio encode lib */
	encode->lib_handle = _SD_Encode_Open(&in_cfg, &out_cfg);
	if (NULL == encode->lib_handle) {
		ak_print_error_ex("open SdCodec failed\n");
		goto open_end;
	}

	ret = AK_SUCCESS;
	encode->enc_type = param->type;
	ak_print_info_ex("type=%d, lib_handle=%p\n",
    	encode->enc_type, encode->lib_handle);
	
open_end:
	if (ret) {
		if ((AK_AUDIO_TYPE_AAC == param->type) && (aac_buf.ref > 0)) {
			del_ref(&(aac_buf.ref), 1);
			if ((aac_buf.ref <= 0) && aac_buf.data) {
				free(aac_buf.data);
				aac_buf.data = NULL;
			}
		}
		if (encode) {
			free(encode);
			encode = NULL;
		}
	}

	return encode;
}

/**
 * ak_aenc_set_attr - set aenc attribution after open
 * @enc_handle[IN]: encode handle
 * @attr[IN]: audio encode attribution
 * return: 0 success, -1 failed
 */
int ak_aenc_set_attr(void *enc_handle, const struct aenc_attr *attr)
{
	if (!enc_handle) {
		ak_print_error_ex("please open first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return AK_FAILED;
	}
	
	if (!attr) {
	    ak_print_error_ex("aenc attr NULL\n");
	    set_error_no(ERROR_TYPE_POINTER_NULL);
        return AK_FAILED;
    }

	struct audio_encode *encode = (struct audio_encode *)enc_handle;
    switch (attr->aac_head) {
    case AENC_AAC_SAVE_FRAME_HEAD:
    	_SD_Encode_SetFramHeadFlag(encode->lib_handle, _SD_ENC_SAVE_FRAME_HEAD);
    	break;
    case AENC_AAC_CUT_FRAME_HEAD:
    	_SD_Encode_SetFramHeadFlag(encode->lib_handle, _SD_ENC_CUT_FRAME_HEAD);
    	break;
    default:
    	break;
    }

    return AK_SUCCESS;
}

/**
 * ak_aenc_send_frame - send pcm data(frame) to encode
 * @enc_handle[IN]: encode handle
 * @frame[IN]: the audio pcm raw data info
 * @stream[OUT]: encode out stream info
 * return: >=0 encoded size, -1 failed
 */
int ak_aenc_send_frame(void *enc_handle,
					const struct frame *frame,
					struct audio_stream *stream)
{
	if (!enc_handle) {
		ak_print_error_ex("please open first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return AK_FAILED;
	}
	
	if (!frame || !stream) {
	    ak_print_error_ex("aenc handle NULL\n");
	    set_error_no(ERROR_TYPE_POINTER_NULL);
        return AK_FAILED;
    }

	int ret = AK_FAILED;
	T_AUDIO_ENC_BUF_STRC enc_info = {0};
    struct audio_encode *encode = (struct audio_encode *)enc_handle;
	
	/* open encode lib with mp3 encode type */
	switch (encode->enc_type) {
	case AK_AUDIO_TYPE_AAC:
		ret = pcm_to_aac(enc_handle, frame, stream);
		break;
	case AK_AUDIO_TYPE_PCM:
		ret = frame->len;
		memcpy(stream->data, frame->data, frame->len);
		stream->seq_no = frame->seq_no;
		stream->ts = frame->ts;
		break;
	default:
		enc_info.buf_in	= (void *)(frame->data);
		enc_info.len_in = frame->len;
		enc_info.buf_out = stream->data;
		/* G711 have to set enc_info.len_out */
		enc_info.len_out = stream->len;

		ret = _SD_Encode(encode->lib_handle, &enc_info);
		stream->len = enc_info.len_out;
		break;
	}

	return ret;
}

/**
 * ak_aenc_close - close anyka audio encode
 * @handle[in]: opened encode handle
 * return: 0 success, otherwise failed
 */
int ak_aenc_close(void *enc_handle)
{
	if (!enc_handle) {
		ak_print_error_ex("please open first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return AK_FAILED;
	}
	
	ak_print_info_ex("enter...\n");
    struct audio_encode *encode = (struct audio_encode *)enc_handle;
    ak_print_info_ex("type=%d, lib_handle=%p\n",
    	encode->enc_type, encode->lib_handle);
	if (encode->lib_handle) {
		if(!_SD_Encode_Close(encode->lib_handle)){
			ak_print_error("unable close audio encode lib!\n");
			return AK_FAILED;
		}
	}

	encode->lib_handle = NULL;
	encode->enc_type = _SD_MEDIA_TYPE_UNKNOWN;

	if (AK_AUDIO_TYPE_AAC == encode->enc_type) {
		del_ref(&(aac_buf.ref), 1);
		if ((aac_buf.ref <= 0) && aac_buf.data) {
			free(aac_buf.data);
			aac_buf.data = NULL;
		}
	}
	
    free(encode);
    encode = NULL;
	ak_print_info_ex("leave...\n");

	return AK_SUCCESS;
}

/**
 * ak_aenc_request_stream - request audio data stream according to handle
 * @ai_handle[IN]: audio input handle
 * @enc_handle[IN]: audio encode handle
 * return: stream handle, NULL failed
 * notes:
 */
void* ak_aenc_request_stream(void *ai_handle, void *enc_handle)
{
    if (NULL == ai_handle) {
		ak_print_error_ex("please open audio input first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return NULL;
	}
	if (NULL == enc_handle) {
		ak_print_error_ex("please open audio encode first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return NULL;
	}

	int ret = AK_FAILED;
	int i = 0;
	int index = 0;
	struct audio_encode_user *user = NULL;
	struct audio_encode *encode = (struct audio_encode *)enc_handle;

	/* match encode type aenc_group index */
	for (index=0; index<AUDIO_ENCODE_TYPE_MAX; ++index) {
		if (encode->enc_type == aenc_group[index].enc_type) {
			break;
		}
	}
	/* match failed */
	if (index >= AUDIO_ENCODE_TYPE_MAX) {
		ak_print_error_ex("no matched audio encode type:%d\n", encode->enc_type);
		goto aenc_request_end;
	}

	user = (struct audio_encode_user *)calloc(1, sizeof(struct audio_encode_user));
    if (!user) {
    	goto aenc_request_end;
    }
    
	user->enc_handle = enc_handle;
	user->group_index = index;

	ak_thread_mutex_lock(&aenc_ctrl.request_mutex);
	if (!aenc_ctrl.run_flag) {
		if(AK_FAILED == start_encode_ctrl(ai_handle)) {
			ak_thread_mutex_unlock(&aenc_ctrl.request_mutex);
			goto aenc_request_end;
		}
		aenc_ctrl.run_flag = AK_TRUE;
	}
	ak_thread_mutex_unlock(&aenc_ctrl.request_mutex);
	
    ak_thread_mutex_lock(&aenc_group[index].mutex);
	for (i=0; i<AENC_TYPE_MAX_USER; ++i) {
		if (!test_bit(i, &(aenc_group[index].user_map))) {
			user->req_nr = i;
			break;
		}
	}
	if (i >= AENC_TYPE_MAX_USER) {
		ak_thread_mutex_unlock(&aenc_group[index].mutex);
		goto aenc_request_end;
	}
    
    set_bit(&(aenc_group[index].user_map), user->req_nr);
    add_ref(&(aenc_group[index].ref), 1);
    aenc_group[index].enc_handle = enc_handle;
    aenc_ctrl.read_flag = AK_TRUE;
    ret = AK_SUCCESS;
    ak_thread_mutex_unlock(&aenc_group[index].mutex);
    ak_thread_sem_post(&aenc_ctrl.ad_sem);

	ak_print_notice_ex("init group_index=%d, user=%p, req_nr=%d, ref=%d\n",
		user->group_index, user, user->req_nr, aenc_group[index].ref);
		
aenc_request_end:
	if (AK_FAILED == ret) {
		if (user) {
			free(user);
			user = NULL;
		}
	}

	return user;
}

/**
 * ak_aenc_get_stream - get audio encoded data, stream
 * @stream_handle[IN]: opened stream handle
 * @stream_head[IN]: stream list head, we'll add audio stream to tail.
 * return: 0 success, -1 failed
 * notes: IMPORTANT, you must call INIT_LIST_HEAD to init stream head firstly.
 */
int ak_aenc_get_stream(void *stream_handle, struct list_head *stream_head)
{
	if (NULL == stream_handle) {
		return AK_FAILED;
	}
	if (NULL == stream_head) {
		return AK_FAILED;
	}
	/* get new stream using empty stream head */
	if (!list_empty(stream_head)) {
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	struct audio_encode_user *user = (struct audio_encode_user *)stream_handle;
	int index = user->group_index;

	ak_thread_mutex_lock(&aenc_group[index].mutex);
	if (list_empty(&(aenc_group[index].data))) {
		goto aenc_get_stream_end;
	}

#if AENC_GET_STREAM_DEBUG
	ak_print_notice_ex("init group_index=%d, user=%p, req_nr=%d, ref=%d\n",
		user->group_index, user, user->req_nr, aenc_group[index].ref);
#endif		

	/* get encoded audio data and push into user list */
	struct audio_entry *pos = aenc_find_user(user, &(aenc_group[index].data));
	if (!pos) {
		goto aenc_get_stream_end;
	}
	
	struct audio_entry *n = NULL;
	struct aenc_entry *entry = NULL;

    list_for_each_entry_safe_from(pos, n, &(aenc_group[index].data), list){
		entry = malloc_aenc_entry(pos->stream.len);
		if (entry) {
			entry->stream.len = pos->stream.len;
			entry->stream.ts = pos->stream.ts;
			entry->stream.seq_no = pos->stream.seq_no;
			memcpy(entry->stream.data, pos->stream.data, pos->stream.len);
			list_add_tail(&(entry->list), stream_head);
			del_ref(&(pos->ref), 1);
			clear_bit(&pos->bit_map, user->req_nr);

#if AENC_WRITE_AUDIO_FILE
			audio_write_aenc_file(aenc_group[index].enc_type, pos);
#endif

#if AENC_GET_STREAM_DEBUG
			ak_print_normal_ex("pos=%p, len=%u, ts=%llu, seq_no=%lu\n", 
				pos, pos->stream.len, pos->stream.ts, pos->stream.seq_no);
#endif				
		} else {
			goto aenc_get_stream_end;
		}
		
#if AENC_GET_STREAM_DEBUG
		ak_print_normal_ex("bit_map=%d, ref=%d\n", pos->bit_map, pos->ref);
#endif		
		if (pos->ref <= 0) {
			free_audio_entry(pos);
		}
	}
	ret = AK_SUCCESS;

aenc_get_stream_end:
	ak_thread_mutex_unlock(&aenc_group[index].mutex);
	
    return ret;
}

/**
 * ak_aenc_release_stream -  release audio data stream
 * @entry[IN]: audio stream entry from ak_aenc_get_stream
 * return: 0 success, -1 failed
 */
int ak_aenc_release_stream(struct aenc_entry *entry)
{
	if (NULL != entry) {
		if (NULL != entry->stream.data) {
			free(entry->stream.data);
			entry->stream.data = NULL;
		}

		list_del_init(&(entry->list));
		free(entry);
		entry = NULL;
	}

	return AK_SUCCESS;
}

/**
 * ak_aenc_cancel_stream -  cancel audio data stream according to stream handle
 * @stream_handle[IN]: opened stream handle
 * return: 0 success, -1 failed
 */
int ak_aenc_cancel_stream(void *stream_handle)
{
	ak_thread_mutex_lock(&aenc_ctrl.request_mutex);
	if (!aenc_ctrl.run_flag) {
		ak_thread_mutex_unlock(&aenc_ctrl.request_mutex);
        return AK_FAILED;
    }
    ak_thread_mutex_unlock(&aenc_ctrl.request_mutex);
    
    if (NULL == stream_handle) {
        return AK_FAILED;
    }

	ak_print_info_ex("entry...\n");
	struct audio_encode_user *user = (struct audio_encode_user *)stream_handle;
	int index = user->group_index;

	ak_thread_mutex_lock(&aenc_group[index].mutex);
    del_ref(&(aenc_group[index].ref), 1);
    clear_bit(&(aenc_group[index].user_map), user->req_nr);

    int i = 0;
	for (i=0; i<AUDIO_ENCODE_TYPE_MAX; ++i) {
		if (aenc_group[i].ref > 0) {
			break;
		}
	}
	ak_thread_mutex_unlock(&aenc_group[index].mutex);

	/* all kinds of audio request are released */
	if (i >= AUDIO_ENCODE_TYPE_MAX) {
		ak_print_notice_ex("all kinds of audio request are released\n");
		aenc_ctrl.read_flag = AK_FALSE;
		aenc_ctrl.run_flag = AK_FALSE;

		ak_thread_sem_post(&aenc_ctrl.ad_sem);
		ak_print_normal_ex("join read pcm thread...\n");
		ak_thread_join(aenc_ctrl.pcm_tid);
		ak_print_notice_ex("read pcm thread join OK\n");

		ak_thread_sem_post(&aenc_ctrl.enc_sem);
		ak_print_normal_ex("join audio encode thread...\n");
		ak_thread_join(aenc_ctrl.enc_tid);
		ak_print_notice_ex("audio encode join OK\n");

		release_capture_data();

		aenc_ctrl.ai_handle = NULL;
		ak_thread_mutex_destroy(&(aenc_ctrl.frame_mutex));
		ak_thread_sem_destroy(&aenc_ctrl.ad_sem);
		ak_thread_sem_destroy(&aenc_ctrl.enc_sem);

		if (NULL != aenc_ctrl.encode_buf) {
			free(aenc_ctrl.encode_buf);
			aenc_ctrl.encode_buf = NULL;
		}
	}

	ak_thread_mutex_lock(&aenc_group[index].mutex);
	if (aenc_group[index].ref <= 0) {
		release_encode_data(index);
	}
	
    free(user);
    user = NULL;
    ak_thread_mutex_unlock(&aenc_group[index].mutex);
	
	if (aenc_group[index].ref <= 0) {
		ak_thread_mutex_destroy(&aenc_group[index].mutex);
	}
    ak_print_info_ex("leave...\n");

    return AK_SUCCESS;
}
