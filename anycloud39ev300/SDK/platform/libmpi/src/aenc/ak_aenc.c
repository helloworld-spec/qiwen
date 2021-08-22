#include "sdcodec.h"
#include "pcm.h"
#include "internal_error.h"

#include "ak_common.h"
#include "ak_error.h"
#include "ak_thread.h"
#include "ak_ai.h"
#include "ak_aenc.h"
#include "aenc_ipcsrv.h"

#define CALC_AUDIO_CAPTURE			1 /* calculate audio capture num */
#define AENC_SYNC_FRAME_TS			0 /* sync frame ts */
#define AUDIO_CALLBACK_DEBUG		0 /* set encode lib callback function */
#define AENC_PUT_STREAM_DEBUG		0 /* printf stream ts,stream length,seq_no */
#define AENC_GET_STREAM_DEBUG		0 /* printf stream information */

#define AENC_TYPE_MAX_USER			(sizeof(int) * 8)
#define AENC_MAX_STREAM_TIME		(5 * 1000)/* we keep 5 seconds audio stream */

#define AUDIO_INFORM_LENGTH 		8		/* audio information length */
#define AUDIO_QUEUE_MAX_ITEM 		500

#define RB_WAITING_RETRY_NUM		500

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

struct audio_entry{
    int ref;					/* Reference count */
    int bit_map;				/* user map */
    struct audio_stream stream;	/* real data stream */
    struct list_head list;
};

struct audio_encode_user{
	int group_index;			/* audio encode type aenc_group index */
	int req_nr;					/* request number */
    void *enc_handle;			/* audio encode handle */
};

struct audio_enc_group {
    int enc_type;				/* audio encode type in T_AUDIO_TYPE(sdcodec.h) */
    int req_ref;				/* request Reference count */
    int req_map;				/* request user bit map */
    int user_ref;				/* open aenc user Reference */
    unsigned long pre_seq_no;	/* previous stream sequence no according to frame */
    void *enc_handle;			/* encode handle */
    ak_mutex_t mutex;
    char init_flag;				/* init struct aenc_group flag */
	int pcm_frame_len;			/* pcm frame length */
	int frame_interval;
	struct frame pcm_frame;
    struct list_head data;		/* stream data queue head */
    ak_mutex_t using_mutex;
};

struct aenc_rb {
	int remain;				//free buffer len
	int read_map;				//current read offset
	int write;				//current write offset
	int total_len;			//buffer total len
	int offset[AUDIO_ENCODE_TYPE_MAX];
	ak_mutex_t data_mutex;
	unsigned char *data;
	unsigned long long write_ts;
	struct pcm_param pcm;
};

struct audio_encode_ctrl {
    unsigned char run_flag;		/* audio run flag */
    unsigned char read_flag;	/* read pcm flag */
    ak_mutex_t io_mutex;		/* open and close mutex */
    ak_mutex_t request_mutex;	/* request mutex */

    int encode_len;				/* encode buffer length */
    unsigned char *encode_buf;	/* encode buffer */

    ak_sem_t ad_sem;			/* read pcm thread semaphore */
    ak_sem_t enc_sem;			/* encode thread semaphore */
    ak_pthread_t pcm_tid;		/* read pcm thread tid */
    ak_pthread_t enc_tid;		/* encode thread tid */

	void *ai_handle;			/* audio input handle */
	ak_mutex_t frame_mutex;		/* frame list mutex */
	struct aenc_rb *pcm_rb;
	
	struct list_head frame_head;/* frame list head */

#if AENC_SYNC_FRAME_TS
    unsigned long long first_ts;	/* the first ts */
	unsigned long long interval_ts;	/* at the first ts per 10 seconds */
	struct ak_timeval first_time;	/* get first frame time */
	struct ak_timeval sync_time;	/* sync ts time */
#endif

#if CALC_AUDIO_CAPTURE
	struct ak_timeval calc_time;
	int audio_num;
#endif
	int get_frame_count;		/* the number of  get frame from ai */
	int encode_frame_count_in;		/* how many frame send to encode lib */
	int encode_frame_count_out; 	/* encode out stream number */
	int get_stream_count;		/* get stream count */

};

struct audio_encode {
    int enc_type;			/* audio encode type in T_AUDIO_TYPE(sdcodec.h) */
	int sample_rate;		/* sample rate */
	int sample_bits;		/* sample bit */
	int channel_num;		/* channle number */
    void *lib_handle;		/* encode lib handle */
	int frame_interval;
};

static const char aenc_version[] = "libmpi_aenc V1.1.09";

#if AENC_DBUG
FILE *fd1 = NULL;
FILE *fd2 = NULL;
#endif

/* encode type init */
static struct audio_enc_group aenc_group[AUDIO_ENCODE_TYPE_MAX] = {
	{ _SD_MEDIA_TYPE_AAC, 		0,0,0,0},
	{ _SD_MEDIA_TYPE_PCM_ALAW, 	0,0,0,0},
	{ _SD_MEDIA_TYPE_PCM_ULAW, 	0,0,0,0},
	{ _SD_MEDIA_TYPE_AMR, 		0,0,0,0},
	{ _SD_MEDIA_TYPE_MP3, 		0,0,0,0},
	{ _SD_MEDIA_TYPE_PCM, 		0,0,0,0}
};

#ifdef AK_RTOS
static struct audio_encode_ctrl aenc_ctrl = {0, 0, 0, 0};
ak_mutex_t *g_aenc_req_lock __attribute__((__section__(".lock_init"))) =
	&aenc_ctrl.request_mutex;
#else
static struct audio_encode_ctrl aenc_ctrl = {
	0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, 0, NULL
};
#endif

#if AUDIO_CALLBACK_DEBUG
static void audio_cb_print(const char *format, ...)
{
	/* REC_TAG, used to identify print informations of media lib */
}

static void* audio_cb_malloc(unsigned long size)
{
	ak_print_info( "malloc size 0x%lx\n", size);
	return malloc(size);
}

static void audio_cb_free(void *mem)
{
	ak_print_info( "free buffer %p", mem);
	return free(mem);
}
#endif

static unsigned char audio_cb_delay(unsigned long ticks)
{
	ak_print_info( "delay 0x%lx ticks", ticks);
	ak_sleep_ms(ticks);

	return 1;
}

static int convert_len_to_interval(struct pcm_param *pcm, int len)
{
	int sample_rate = pcm->sample_rate > 0 ? pcm->sample_rate : 8000;
	int sample_bit = pcm->sample_bits == 16 ? pcm->sample_bits : 16;

	int interval = len * 8000 / (sample_rate * sample_bit);

	return interval;
}

/**  
 * convert_interval_to_len - convert time interval to length
 * @encode[IN]: audio encode handle
 * @interval[IN]: time interval
 * notes: 
 */
static int convert_interval_to_len(struct audio_encode *encode,
									int interval)
{
	int sample_rate = encode->sample_rate > 0 ? encode->sample_rate : 8000;
	unsigned int sample_bytes = (encode->sample_bits >> 3);
	/* size = sample rete* channel num * time(ms) *sample bit /8 / 1000 */
	/* 8 is bit to byte */
	/* 1000 is second to ms */
	/* user->interval is ms */
	int size = (sample_rate * encode->channel_num
			* interval * sample_bytes ) / 1000;
	if (size & 1) {
		size++;
	}

	return size;
}

static int aenc_rb_init(int rb_size, struct audio_encode *encode)
{
	int ret = AK_SUCCESS;

	if (rb_size > 0) {
		aenc_ctrl.pcm_rb = (struct aenc_rb *)calloc(1, sizeof(struct aenc_rb));
		if (aenc_ctrl.pcm_rb) {
			aenc_ctrl.pcm_rb->data = (unsigned char *)calloc(1, rb_size); 
			if (aenc_ctrl.pcm_rb->data) {
				ak_thread_mutex_init(&(aenc_ctrl.pcm_rb->data_mutex), NULL);
				aenc_ctrl.pcm_rb->total_len = rb_size;
				ak_print_normal_ex("init rb_size=%d ring buffer OK\n", rb_size);
				memset(aenc_ctrl.pcm_rb->offset, 0, sizeof(int) * AUDIO_ENCODE_TYPE_MAX);
				aenc_ctrl.pcm_rb->read_map = 0;

				aenc_ctrl.pcm_rb->pcm.channel_num = encode->channel_num;
				aenc_ctrl.pcm_rb->pcm.sample_bits = encode->sample_bits;
				aenc_ctrl.pcm_rb->pcm.sample_rate = encode->sample_rate;
				
			} else {
				free(aenc_ctrl.pcm_rb);
				aenc_ctrl.pcm_rb = NULL;
				
				ak_print_error_ex("calloc ring buffer failed, rb_size=%d\n", rb_size);
				set_error_no(ERROR_TYPE_MALLOC_FAILED);
				ret = AK_FAILED;
			}
		} else {
			ak_print_error_ex("calloc ring buffer struct failed\n");
			set_error_no(ERROR_TYPE_MALLOC_FAILED);
			ret = AK_FAILED;
		}
	}
	return ret;
}

static int aenc_rb_get_read_pointer(void)
{
	int i = 0;
	int read_point = -1;
	int offset_group_count = AUDIO_ENCODE_TYPE_MAX;
	int *offset_group = aenc_ctrl.pcm_rb->offset;
	
	for (i = 0; i < offset_group_count; i++) {
		if (!test_bit(i, &(aenc_ctrl.pcm_rb->read_map)))
			continue;
		
		if (offset_group[i] > aenc_ctrl.pcm_rb->write && -1 == read_point) {
			read_point = offset_group[i];
		}
		
		if (offset_group[i] > aenc_ctrl.pcm_rb->write && offset_group[i] < read_point) {
			read_point = offset_group[i];
		}
	}

	/* offset is not in write pionter right */
	if (-1 != read_point) 
		return read_point;
	
	for (i = 0; i < offset_group_count; i++) {
		if (!test_bit(i, &(aenc_ctrl.pcm_rb->read_map)))
			continue;
	
		if (offset_group[i] <= aenc_ctrl.pcm_rb->write && -1 == read_point) {
			read_point = offset_group[i];
		}
		
		if (offset_group[i] <= aenc_ctrl.pcm_rb->write && offset_group[i] < read_point) {
			read_point = offset_group[i];
		}
	}
	return read_point;
}

static unsigned long long aenc_rb_get_ts(int offset, int read_len)
{
	int interval = 0;		
	int len = 0;
	unsigned long long ts = 0;
		
	if (offset <= aenc_ctrl.pcm_rb->write) {
		len = aenc_ctrl.pcm_rb->write - offset;
	} else {
		len = aenc_ctrl.pcm_rb->total_len - offset + aenc_ctrl.pcm_rb->write;
	}

	len += read_len;
	interval = convert_len_to_interval(&(aenc_ctrl.pcm_rb->pcm), len);
	ts = aenc_ctrl.pcm_rb->write_ts - interval;
	
	ak_print_debug_ex("ts interval =%d\n", interval);
	return ts;
}

static int aenc_rb_write(struct frame *frame)
{
	if (!aenc_ctrl.pcm_rb) {
		ak_print_error_ex("aenc_ctrl.pcm_rb = NULL\n");
		return AK_FAILED;
	}

	int write_len = 0;
	
	ak_thread_mutex_lock(&(aenc_ctrl.pcm_rb->data_mutex));

	int read = aenc_rb_get_read_pointer();
	if (AK_FAILED == read) {
		ak_print_error_ex("read pointer not found\n");
		return AK_FAILED;
	}

	if (0 == read && aenc_ctrl.pcm_rb->total_len == aenc_ctrl.pcm_rb->write) {
		aenc_ctrl.pcm_rb->write = 0;
	}

	ak_print_debug_ex("**** write=%d, read=%d\n",
			aenc_ctrl.pcm_rb->write, read);	

	int tail_space = (aenc_ctrl.pcm_rb->total_len - aenc_ctrl.pcm_rb->write);
	int overwrite_space = 0;
	
	if (read < aenc_ctrl.pcm_rb->write) {
		overwrite_space = tail_space + read;
	} else if (read > aenc_ctrl.pcm_rb->write) {
		overwrite_space = read - aenc_ctrl.pcm_rb->write;
		if (overwrite_space == frame->len)
			overwrite_space = 0;
	} else { // read = write
		overwrite_space = frame->len;//aenc_ctrl.pcm_rb->total_len ;
	}

	if (overwrite_space < frame->len) {
		ak_thread_mutex_unlock(&(aenc_ctrl.pcm_rb->data_mutex));
		return overwrite_space;
	}

	int frame_interval = convert_len_to_interval(&(aenc_ctrl.pcm_rb->pcm), frame->len);
	aenc_ctrl.pcm_rb->write_ts = frame->ts + frame_interval;

	if (tail_space >= frame->len) {
		memcpy(&(aenc_ctrl.pcm_rb->data[aenc_ctrl.pcm_rb->write]), frame->data, frame->len);
		aenc_ctrl.pcm_rb->write += frame->len;
	} else {
		if (tail_space > 0) {
			memcpy(&(aenc_ctrl.pcm_rb->data[aenc_ctrl.pcm_rb->write]), 
					frame->data, tail_space);
		}

		memcpy(aenc_ctrl.pcm_rb->data, &(frame->data[tail_space]), (frame->len - tail_space));
		aenc_ctrl.pcm_rb->write = frame->len - tail_space;
	}

	write_len = aenc_ctrl.pcm_rb->write;
	    
	ak_print_debug_ex("**** write=%d, read=%d,---ts=%lld\n\n",
			aenc_ctrl.pcm_rb->write, read, aenc_ctrl.pcm_rb->write_ts);
	
	ak_thread_mutex_unlock(&(aenc_ctrl.pcm_rb->data_mutex));

	return write_len;
}

static int aenc_rb_read(int index, unsigned char *to, int read_len, unsigned long long *ts)
{
	if (!aenc_ctrl.pcm_rb) {
		return AK_FAILED;
	}

	ak_thread_mutex_lock(&(aenc_ctrl.pcm_rb->data_mutex));
		
	int cur_offset = aenc_ctrl.pcm_rb->offset[index];
			
	int tail_space = (aenc_ctrl.pcm_rb->total_len - cur_offset);
	int read_space = 0;
	
	if (cur_offset < aenc_ctrl.pcm_rb->write) {
		read_space = aenc_ctrl.pcm_rb->write - cur_offset;
	} else if (cur_offset > aenc_ctrl.pcm_rb->write) {
		read_space = aenc_ctrl.pcm_rb->write + tail_space;
	} else { // read = write
		read_space = 0;
	}

	if (read_space < read_len) {
		ak_thread_mutex_unlock(&(aenc_ctrl.pcm_rb->data_mutex));
		return AK_FAILED;
	}

	if (tail_space >= read_len) {
		memcpy(to, &(aenc_ctrl.pcm_rb->data[cur_offset]), read_len);
		cur_offset += read_len;
	} else {
		if (tail_space > 0) {
			memcpy(to, &(aenc_ctrl.pcm_rb->data[cur_offset]), tail_space);
		}
		
		memcpy(&to[tail_space], aenc_ctrl.pcm_rb->data, (read_len - tail_space));
		cur_offset = read_len - tail_space;
	}

	aenc_ctrl.pcm_rb->offset[index] = cur_offset;

	*ts = aenc_rb_get_ts(cur_offset, read_len);
		
	ak_print_debug_ex("------index=%d  offset=%d, write=%d,ts=%lld\n", index,
				cur_offset, aenc_ctrl.pcm_rb->write, *ts);
	
	ak_thread_mutex_unlock(&(aenc_ctrl.pcm_rb->data_mutex));
	
	return read_len;

}

static void aenc_rb_update_read_map(int index, int req_enable)
{
	ak_thread_mutex_lock(&(aenc_ctrl.pcm_rb->data_mutex));
	if (req_enable)
		set_bit(&(aenc_ctrl.pcm_rb->read_map), index);	
	else
		clear_bit(&(aenc_ctrl.pcm_rb->read_map), index);
	ak_thread_mutex_unlock(&(aenc_ctrl.pcm_rb->data_mutex));

}

static int aenc_rb_deinit(void)
{
	if (!aenc_ctrl.pcm_rb) {
		return AK_FAILED;
	}
	
	if (aenc_ctrl.pcm_rb->data) {
		free(aenc_ctrl.pcm_rb->data);
		aenc_ctrl.pcm_rb->data = NULL;
	}

	ak_thread_mutex_destroy(&(aenc_ctrl.pcm_rb->data_mutex));
	ak_print_normal_ex("rb_size=%d, release ring buffer OK\n", aenc_ctrl.pcm_rb->total_len);
	free(aenc_ctrl.pcm_rb);
	aenc_ctrl.pcm_rb = NULL;

	return AK_SUCCESS;
}

static int get_frame_interval(struct audio_encode *encode,
									int default_interval)
{
	int interval = AUDIO_DEFAULT_INTERVAL;
	int sample_rate = encode->sample_rate > 0 ? encode->sample_rate : 8000;
	
	switch (encode->enc_type) {
	case AK_AUDIO_TYPE_AAC:
		interval = ((1024 *1000) / sample_rate); /* 1k data in 1 second */
		break;
	case AK_AUDIO_TYPE_AMR:
		interval = AMR_FRAME_INTERVAL;
		break;
	case AK_AUDIO_TYPE_PCM_ALAW:	/* G711, alaw */
	case AK_AUDIO_TYPE_PCM_ULAW:	/* G711, ulaw */
		interval = default_interval;
		break;
	case AK_AUDIO_TYPE_PCM:
		interval = default_interval;
		break;
	case AK_AUDIO_TYPE_MP3:
		if (sample_rate >= 8000 && sample_rate <= 24000) {
			interval = 576*1000 / sample_rate;
		} else { // sample_rate =32000 or 44100 or 48000
			interval = 1152*1000 / sample_rate;
		}
		break;
	default:	
		interval = AUDIO_DEFAULT_INTERVAL;
		break;
	}

	if (0 == interval)
		interval = AUDIO_DEFAULT_INTERVAL;

	return interval;
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
		ak_print_error_ex( "Unknow encode type!\n" );
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
	config->chip = AUDIOLIB_CHIP_AK39XXEV3;

	/* set the encode input type */
	config->enc_in_info.m_Type = param->type;
	switch (param->type) {
	case AK_AUDIO_TYPE_AAC:
		config->enc_in_info.m_private.m_aac.cutAdtsHead = _SD_ENC_CUT_FRAME_HEAD;
		break;
    case AK_AUDIO_TYPE_AMR:
        config->enc_in_info.m_private.m_amr_enc.mode = AMR_ENC_MR122;
        break;
	default:/* other types do not set any other param */
		break;
	}

	return AK_SUCCESS;
}

/**
 * init_aenc_group - init aenc_group struct
 * @encode_index[IN]: audio encode type index
 * return: void
 */
static void init_aenc_group(int encode_index)
{
	if (aenc_group[encode_index].init_flag) {
		ak_print_info_ex( "aenc_group has init already\n");
		return;
	}

	ak_print_info_ex( "init mutex encode type %d \n", encode_index);
	aenc_group[encode_index].req_ref = 0;
	ak_thread_mutex_init(&aenc_group[encode_index].mutex, NULL);
	INIT_LIST_HEAD(&(aenc_group[encode_index].data));

	aenc_group[encode_index].init_flag = AK_TRUE;

}

static int frame_buffer_init(int encode_index, struct audio_encode *encode)
{
	if (aenc_group[encode_index].pcm_frame.data && aenc_ctrl.run_flag) {
		ak_print_notice_ex("aenc encode is running\n");
		return AK_SUCCESS;
	}
	
	int interval = get_frame_interval(encode, encode->frame_interval);

	/* if pcm_frame_len not init, get pcm frame length */
	aenc_group[encode_index].frame_interval = interval;

	aenc_group[encode_index].pcm_frame_len = convert_interval_to_len(encode, 
								aenc_group[encode_index].frame_interval);

	ak_print_normal_ex("index=%d, pcm_frame_len=%d\n", 
					encode_index, aenc_group[encode_index].pcm_frame_len);
	
	/* relloc buffer */
	if (aenc_group[encode_index].pcm_frame.data)
		free(aenc_group[encode_index].pcm_frame.data);
	
	aenc_group[encode_index].pcm_frame.data = calloc(1, 
									aenc_group[encode_index].pcm_frame_len);
	
	if (!aenc_group[encode_index].pcm_frame.data)
		return AK_FAILED;

	return AK_SUCCESS;
}

/**
 * init_audio_encode - init audio encode info
 * @enc_len[IN]: audio encode buffer len in bytes
 * return: void
 */
static void init_audio_encode(int enc_len)
{
	aenc_ctrl.encode_len = enc_len;
    aenc_ctrl.encode_buf = (unsigned char *)calloc(1, (aenc_ctrl.encode_len << 2));
    if (NULL == aenc_ctrl.encode_buf) {
        ak_print_error( "can't calloc audio encode buffer\n");
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
            ak_print_error_ex( "calloc %d failed\n", len);
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
            ak_print_error_ex( "calloc %d failed\n", len);
            free(entry);
	    	entry = NULL;
        }
	}

    return entry;
}

/**
 * free_audio_entry - free audio data entry info
 * @len[IN]: audio data entry address
 * return: void
 */
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

/**
 * release_encode_data - release aenc encode data
 * @index[IN]: current audio encode type index
 * return: void
 */
static void release_encode_data(int index)
{
	struct audio_entry *pos = NULL;
	struct audio_entry *n = NULL;

    list_for_each_entry_safe(pos, n, &(aenc_group[index].data), list){
		free_audio_entry(pos);
	}
}

/**
 * release_encode_data_by_bitmap - release aenc encode data by bit map
 * @index[IN]: current audio encode type index 
 * @bit_map[IN]: user bit map
 * return: void
 */
static void release_encode_data_by_bitmap(int index, int bit_map)
{
	struct audio_entry *pos = NULL;
	struct audio_entry *n = NULL;

    list_for_each_entry_safe(pos, n, &(aenc_group[index].data), list){
		if (bit_map == pos->bit_map)
			free_audio_entry(pos);
	}
}

/**
 * drop_extra_stream - if the first entery is too early,drop it
 * @index[IN]: current audio encode type index
 * @cur_ts[IN]: current time
 * return: void
 */
static void drop_extra_stream(enum audio_encode_type index,
				struct audio_entry *cur_entry)
{
	struct audio_entry *entry = NULL;
	struct audio_entry *ptr = NULL;

    list_for_each_entry_safe(entry, ptr, &(aenc_group[index].data), list) {
        /* whether the stream ts is too early */
		unsigned long long diff_ts = (cur_entry->stream.ts - entry->stream.ts);
		if (diff_ts >= AENC_MAX_STREAM_TIME) {
			char log[100];
			sprintf(log, "time interval too long,diff ts: %llu(ms),", diff_ts);
		
			ak_print_normal_ex( "%s cur:ts=%llu, first:ts=%llu\n",
						log, cur_entry->stream.ts, entry->stream.ts);

			/* drop the stream */
			free_audio_entry(entry);
		}
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
		/* set stream information */
	    memcpy(entry->stream.data, data, len);
	    entry->stream.len = len;
	    entry->stream.ts = origin_frame->ts;
	    entry->stream.seq_no = origin_frame->seq_no;
	    entry->ref = aenc_group[index].req_ref;

	    /* set entry user bit map */
	    entry->bit_map = aenc_group[index].req_map;

		if((origin_frame->seq_no > 0)
			&& (aenc_group[index].pre_seq_no + 1) != origin_frame->seq_no){
			ak_print_warning_ex( "len=%u, ts=%llu, pre_seq_no=%lu, seq_no=%lu\n",
				entry->stream.len, entry->stream.ts,
				aenc_group[index].pre_seq_no, entry->stream.seq_no);
		}
		aenc_group[index].pre_seq_no = origin_frame->seq_no;

#if AENC_PUT_STREAM_DEBUG
		ak_print_notice_ex( "encode: ts=%llu, len=%u, seq_no=%lu\n",
			entry->stream.ts, entry->stream.len, entry->stream.seq_no);
#endif
		/* put the stream to list */
		ak_thread_mutex_lock(&aenc_group[index].mutex);
		drop_extra_stream(index, entry);
	    list_add_tail(&(entry->list), &(aenc_group[index].data));
	    ak_thread_mutex_unlock(&aenc_group[index].mutex);
		
		aenc_ctrl.encode_frame_count_out++;
	    ret = AK_SUCCESS;
	}

	return ret;
}

/**
 * get_group_index - get group index
 * @type[IN]: audio type
 * return: audio group index
 */
static enum audio_encode_type get_group_index(enum ak_audio_type type)
{
	int group_index = AUDIO_ENCODE_TYPE_MAX;

	switch (type) {
	case AK_AUDIO_TYPE_AAC:
    	group_index = AUDIO_ENCODE_AAC;
    	break;
    case AK_AUDIO_TYPE_PCM_ALAW:
		group_index = AUDIO_ENCODE_G711A;
		break;
	case AK_AUDIO_TYPE_PCM_ULAW:
		group_index = AUDIO_ENCODE_G711U;
		break;
	case AK_AUDIO_TYPE_AMR:
		group_index = AUDIO_ENCODE_AMR;
		break;
	case AK_AUDIO_TYPE_MP3:
		group_index = AUDIO_ENCODE_MP3;
		break;
	case AK_AUDIO_TYPE_PCM:
		group_index = AUDIO_ENCODE_RAW_PCM;
		break;
	default:
		break;
	}

	return group_index;
}

/**
 * open_encode_lib - open encode lib
 * @param[IN]: audio parameter
 * return: audio encode lib handle
 */
static void* open_encode_lib(const struct audio_param *param)
{
	T_AUDIO_REC_INPUT in_cfg;

	memset(&in_cfg, 0x00, sizeof(T_AUDIO_REC_INPUT));
	if (init_encode_input(param, &in_cfg)) {
		return NULL;
	}

    T_AUDIO_ENC_OUT_INFO out_cfg;
	memset(&out_cfg, 0x00, sizeof(T_AUDIO_ENC_OUT_INFO));

	/* open anyka audio encode lib */
	return _SD_Encode_Open(&in_cfg, &out_cfg);
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
		ak_print_notice_ex( "bit_map=%d, ref=%d\n",
		    entry->bit_map, entry->ref);
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
/**
 * calc_audio_capture_num - calculate audio capture num
 * @void
 * return: void
 */
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
			ak_print_info( "*** audio info, seconds=%d, total=%d, "
				"average=%d ***\n\n", seconds,
				aenc_ctrl.audio_num, (aenc_ctrl.audio_num / seconds));
		}

		aenc_ctrl.audio_num = 0;
		ak_get_ostime(&(aenc_ctrl.calc_time));
	}
}
#endif

#if AENC_SYNC_FRAME_TS
/**
 * sync_aenc_frame_ts - sync aenc frame ts
 * @cur_ts[IN]: current ts
 * return: void
 */
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
		ak_print_normal( "*** first_ts=%llu, cur_ts=%llu, "
		    "target_ts=%llu, total_ts=%llu ***\n",
			aenc_ctrl.first_ts, cur_ts, target_ts, total_ts);
		ak_print_normal( "*** aenc 10 sec diff=%lld, "
		    "total diff=%lld ***\n\n",
			(cur_ts - target_ts), (cur_ts - total_ts));

		ak_get_ostime(&(aenc_ctrl.sync_time));
		aenc_ctrl.interval_ts = cur_ts;
	}
}
#endif

/**
 * encode_audio_data - encode audio frame data
 * @origin_frame[IN]: frame data
 * return: void
 */
static void encode_audio_data(int index)
{
	int ret = 0;
	unsigned long long ts = 0;
	
	while (aenc_group[index].pcm_frame_len == aenc_rb_read(index, 
								aenc_group[index].pcm_frame.data, 
								aenc_group[index].pcm_frame_len, &ts)) {
				
		/* can read a frame from aenc rb buffer */
		struct audio_stream stream = {0};
		stream.data = aenc_ctrl.encode_buf;
		stream.len = (aenc_ctrl.encode_len << 2);
		stream.ts = ts;

		/* genarate a frame */
		aenc_group[index].pcm_frame.len = aenc_group[index].pcm_frame_len;
		aenc_group[index].pcm_frame.ts = stream.ts;
		aenc_group[index].pcm_frame.seq_no++;

		stream.seq_no = aenc_group[index].pcm_frame.seq_no;
	
		/* send frame to encode */
		
#if AENC_DBUG
		if (fd2) {
			fwrite(aenc_group[index].pcm_frame.data, 1, aenc_group[index].pcm_frame.len, fd2);
		}
#endif
		aenc_ctrl.encode_frame_count_in++;
		ret = ak_aenc_send_frame(aenc_group[index].enc_handle, &(aenc_group[index].pcm_frame), 
							&stream);
		if (ret > 0) {
	        push_audio_queue(index, aenc_ctrl.encode_buf, ret, &(aenc_group[index].pcm_frame));
		} else {
			ak_print_warning_ex( "aenc frame ret=%d\n", ret);
		}

		if (aenc_ctrl.run_flag == AK_FALSE) {
			ak_print_notice_ex( "it want be exit\n");
			break;
		}

	}
}

/**
 * encode_user_data - encode audio data
 * @void
 * void
 */
static void encode_user_data(void)
{
	int i = 0;

	for (i = 0; i < AUDIO_ENCODE_TYPE_MAX; i++) {
        if(0 == aenc_group[i].req_ref){
            continue;
        }
        ak_thread_mutex_lock(&aenc_group[i].using_mutex);
		encode_audio_data(i);
		ak_thread_mutex_unlock(&aenc_group[i].using_mutex);
		if (aenc_ctrl.run_flag == AK_FALSE) {
			ak_print_notice_ex( "it want be exit\n");
			break;
		}	
	}
}

/**
 * read_to_encode - read audio data to encode
 * @arg[IN]: NULL
 * return: 0 success, -1 failed
 */
static int read_to_encode(void)
{
    long timeout = 0;
    struct frame frame = {0};
	int i = 0;

    /* get the pcm data */
	int ret = ak_ai_get_frame(aenc_ctrl.ai_handle, &frame, timeout);

	if (ret) {
		return ret;	
	}
	aenc_ctrl.get_frame_count++;
		
	/* cope frame data to aenc rb buffer */
	int write_len = aenc_rb_write(&frame);
	if (write_len == AK_FAILED) {
		ak_print_error_ex("--- wrint to rb error :write_len=%d,frame_len=%d\n", 
							write_len, frame.len);
		ret = write_len;
		ak_ai_release_frame(aenc_ctrl.ai_handle, &frame);
		return ret;
	}

	int count = 0; /* count the waiting number */
	while (write_len < frame.len && aenc_ctrl.read_flag) {	
		write_len = aenc_rb_write(&frame);
		ak_sleep_ms(10);
		count++;

		if (count > RB_WAITING_RETRY_NUM) {	
			ak_print_normal_ex("waiting.... write_len=%d, frame.len=%d, write_rb=%d\n", 
				write_len, frame.len, aenc_ctrl.pcm_rb->write);
			for (int i = 0; i< AUDIO_ENCODE_TYPE_MAX; i++) {
				ak_print_normal_ex("read_rb[%d]=%d\n", i, aenc_ctrl.pcm_rb->offset[i]);
			}
			aenc_ctrl.pcm_rb->write = 0;
			ak_thread_sem_post(&aenc_ctrl.enc_sem);
			//ak_sleep_ms(10);
			//write_len = aenc_rb_write(&frame);
			break;
		}
	}
	
#if AENC_DBUG
	if (fd1) {
		fwrite(frame.data, 1, frame.len, fd1);
	}
#endif

	ak_ai_release_frame(aenc_ctrl.ai_handle, &frame);
	
	for (i = 0; i < AUDIO_ENCODE_TYPE_MAX; i++) {
		if (aenc_group[i].req_ref) {
			ak_thread_sem_post(&aenc_ctrl.enc_sem);
		}
	}	
	
	return ret;
}

/**
 * read_pcm_thread - read audio pcm data from AD
 * @arg[IN]: NULL
 * return: NULL
 */
static void* read_pcm_thread(void *arg)
{
	ak_print_normal_ex( "thread id: %ld\n", ak_thread_get_tid());
	ak_thread_set_name("aenc_capture");

	while (aenc_ctrl.run_flag) {
        ak_print_normal_ex( "sleep...\n");
		ak_thread_sem_wait(&aenc_ctrl.ad_sem);
        ak_print_normal_ex( "wakeup, encode_len=%u\n",
            aenc_ctrl.encode_len);

#if CALC_AUDIO_CAPTURE
		aenc_ctrl.audio_num = 0;
		ak_get_ostime(&(aenc_ctrl.calc_time));
#endif

		ak_print_notice_ex( "aenc_ctrl.read_flag=%d\n",
		    aenc_ctrl.read_flag);
		while (aenc_ctrl.read_flag) {
        	if (read_to_encode())
				ak_sleep_ms(10);
			
#if CALC_AUDIO_CAPTURE
			calc_audio_capture_num();
#endif
        }

        ak_print_normal_ex( "read while exit\n");
	}

    ak_print_normal_ex( "### thread id: %ld exit ###\n\n",
        ak_thread_get_tid());

	ak_thread_exit();
	return NULL;
}

/**
 * audio_encode_thread - encode audio frame and push into audio stream list
 * @arg[IN]: NULL
 * return: NULL
 */
static void* audio_encode_thread(void *arg)
{
	ak_print_normal_ex( "thread id: %ld\n", ak_thread_get_tid());
	ak_thread_set_name("aenc_encode");

	while (aenc_ctrl.run_flag) {
		ak_thread_sem_wait(&aenc_ctrl.enc_sem);

		if (aenc_ctrl.run_flag) {
			encode_user_data();
		}
	}

    ak_print_normal_ex( "### thread id: %ld exit ###\n\n",
        ak_thread_get_tid());

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

	/* read_pcm_thread or audio_encode_thread not exit, wait it. */
	while (NULL != aenc_ctrl.encode_buf){
		ak_print_notice_ex( "wait mpi audio thread exit\n");
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

	/*init read pcm thread semaphore */
    ak_thread_sem_init(&aenc_ctrl.ad_sem, 0);
    /*init encode thread semaphore */
    ak_thread_sem_init(&aenc_ctrl.enc_sem, 0);

    ak_ai_start_capture(aenc_ctrl.ai_handle);

	/* create the capture data thread */
	aenc_ctrl.run_flag = AK_TRUE;
	int ret = ak_thread_create(&(aenc_ctrl.pcm_tid), read_pcm_thread,
			NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if (ret) {
		ak_print_error( "create read_pcm_thread FAILED, ret=%d\n", ret);
		goto encode_ctrl_end;
	}

	/* create the encode data thread */
	ret = ak_thread_create(&(aenc_ctrl.enc_tid), audio_encode_thread,
			NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if (ret) {
		ak_print_error( "create audio_encode_thread thread FAILED, "
		    "ret=%d\n", ret);
	}

encode_ctrl_end:
	if (ret) {
		aenc_ctrl.ai_handle = NULL;
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

    cb_codc.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;
    _SD_GetAudioCodecVersions(&cb_codc);
}

/**
 * ak_aenc_get_version - get audio encode version
 * return: version string
 * notes:
 */
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
    ak_print_notice_ex("aenc version %s \n", aenc_version);
	if (!param) {
		ak_print_error_ex( "invalid audio encode param\n");
		return NULL;
	}

	if (param->type == AK_AUDIO_TYPE_AMR && 
		param->channel_num == AUDIO_CHANNEL_STEREO) {
		/* amr encode not suppprt stereo */
		ak_print_error_ex("amr encode not suppprt stereo\n");
		return NULL;
	}

	if (param->sample_rate < 8000 || param->sample_rate > 64000) {
		/* sample rate not support  */
		ak_print_error_ex("sample rate not suppprt %d \n", param->sample_rate);
		return NULL;
	}
	
#if AENC_DBUG
	fd1 = fopen("/mnt/aenc.pcm", "w+");
	fd2 = fopen("/mnt/aenc_after.pcm", "w+");
#endif
	int ret = AK_FAILED;
	struct audio_encode *encode = NULL;

	int group_index = get_group_index(param->type);
	if (AUDIO_ENCODE_TYPE_MAX == group_index) {
		ak_print_error_ex( "not support type\n");
		return NULL;
	}

	ak_thread_mutex_lock(&aenc_ctrl.io_mutex);
	/* this type has open already */
	if (aenc_group[group_index].user_ref > 0) {
		ret = AK_SUCCESS;
		goto open_end;
	}

    encode = (struct audio_encode *)calloc(1, sizeof(struct audio_encode));
    if (!encode) {
        goto open_end;
    }

	encode->enc_type = param->type;
	encode->sample_rate = param->sample_rate;
	encode->sample_bits = param->sample_bits;
	encode->channel_num = param->channel_num;

	/* pcm not need to open _SD_Encode_Open */
    if (AK_AUDIO_TYPE_PCM == param->type) {
		encode->lib_handle = NULL;
		ret = AK_SUCCESS;
		goto open_end;
    }

	encode->lib_handle = open_encode_lib(param);
	if (!encode->lib_handle) {
		ak_print_error_ex( "open SdCodec failed\n");
 		goto open_end;
	}

	ak_print_info_ex( "type=%d, lib_handle=%p\n",
    	encode->enc_type, encode->lib_handle);

    aenc_sys_ipc_register();
    aenc_sysipc_bind_handle(encode);
    ret = AK_SUCCESS;

open_end:
	if (ret) {
		if (encode) {
			free(encode);
			encode = NULL;
		}
	} else {
		if (aenc_group[group_index].enc_handle) {
			encode = aenc_group[group_index].enc_handle;
		} else {
			aenc_group[group_index].enc_handle = encode;
		}

		add_ref(&(aenc_group[group_index].user_ref), 1);
	}
	ak_thread_mutex_unlock(&aenc_ctrl.io_mutex);

	return encode;
}

/**
 * ak_aenc_get_params - get aenc params
 * @handle[IN]: aenc opened handle
 * @param[OUT]: aenc params by aenc_open()
 * return: 0 success, -1 failed
 * notes: call after aenc_open()
 */
int ak_aenc_get_params(void *handle, struct audio_param *param)
{
	/* param check */
	if(!handle || !param) {
		ak_print_error_ex("invalid param\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	struct audio_encode *encode = (struct audio_encode *)handle;
	param->type = encode->enc_type;
	param->sample_rate = encode->sample_rate;
	param->sample_bits = encode->sample_bits;
	param->channel_num = encode->channel_num;

	ak_print_normal_ex("get_frame_count =%d, encode_frame_count_in =%d,\
		encode_frame_count_out =%d, get_stream_count =%d\n", 
		aenc_ctrl.get_frame_count, aenc_ctrl.encode_frame_count_in,
		aenc_ctrl.encode_frame_count_out, aenc_ctrl.get_stream_count);
	return AK_SUCCESS;
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
		ak_print_error_ex( "please open first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return AK_FAILED;
	}

	if (!attr) {
	    ak_print_error_ex( "aenc attr NULL\n");
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
 * ak_aenc_set_frame_default_interval - set audio frame default interval, unit: ms
 * @enc_handle[IN]: encode handle
 * @frame_interval[IN]: audio frame interval, [10, 125], unit: ms
 * return: 0 success, -1 failed
 * note: just for g711/pcm encode, 
 */
int ak_aenc_set_frame_default_interval(void *enc_handle, int frame_interval)
{
	if (!enc_handle) {
		ak_print_error_ex( "please open first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return AK_FAILED;
	}
	
    struct audio_encode *encode = (struct audio_encode *)enc_handle;
	encode->frame_interval = get_frame_interval(encode, frame_interval);

	ak_print_normal_ex("set frame interval:%d\n", encode->frame_interval);
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
		ak_print_error_ex( "please open first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return AK_FAILED;
	}

	if (!frame || !stream) {
	    ak_print_error_ex( "frame or stream NULL\n");
	    set_error_no(ERROR_TYPE_POINTER_NULL);
        return AK_FAILED;
    }

	if (!frame->data || !stream->data) {
	    ak_print_error_ex( "frame->data or stream->data NULL\n");
	    set_error_no(ERROR_TYPE_POINTER_NULL);
        return AK_FAILED;
    }

	int ret = AK_FAILED;
	T_AUDIO_ENC_BUF_STRC enc_info = {0};
    struct audio_encode *encode = (struct audio_encode *)enc_handle;

	if(encode->enc_type == AK_AUDIO_TYPE_PCM_ALAW || 
		encode->enc_type == AK_AUDIO_TYPE_PCM_ALAW) {
		if (stream->len <= 0) {
			ak_print_error_ex( "stream->len <= 0\n");
			set_error_no(ERROR_TYPE_INVALID_ARG);
			return AK_FAILED;
		}
	}

	aenc_save_stream_to_file(encode->enc_type, frame->data, frame->len,
    						BEFORE_ENC_LEVEL);

	/* open encode lib with mp3 encode type */
	switch (encode->enc_type) {
	case AK_AUDIO_TYPE_PCM:
		ret = frame->len;
		memcpy(stream->data, frame->data, frame->len);
		stream->seq_no = frame->seq_no;
		stream->ts = frame->ts;
		break;
	default:
		if (!encode->lib_handle)
			break;
		enc_info.buf_in	= (void *)(frame->data);
		enc_info.len_in = frame->len;
		enc_info.buf_out = stream->data;
		/* G711 have to set enc_info.len_out */
		enc_info.len_out = stream->len;

		ret = _SD_Encode(encode->lib_handle, &enc_info);
		stream->len = enc_info.len_out;
		break;
	}
	aenc_save_stream_to_file(encode->enc_type, stream->data, stream->len,
							AFTER_ENC_LEVEL);

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
		ak_print_error_ex( "please open first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return AK_FAILED;
	}

	ak_print_info_ex( "enter...\n");
    struct audio_encode *encode = (struct audio_encode *)enc_handle;
    ak_print_info_ex( "type=%d, lib_handle=%p\n",
    	encode->enc_type, encode->lib_handle);

    int group_index = get_group_index(encode->enc_type);
    if (AUDIO_ENCODE_TYPE_MAX == group_index) {
		ak_print_error_ex( "not support enclde type\n");
		return AK_FAILED;
    }

	ak_thread_mutex_lock(&aenc_ctrl.io_mutex);
	del_ref(&(aenc_group[group_index].user_ref), 1);
	/* user_ref=0, no user opening, close encode handle */
	if (0 == aenc_group[group_index].user_ref) {
        if (encode->lib_handle) {
    		if (!_SD_Encode_Close(encode->lib_handle)) {
    			ak_print_error( "unable close audio encode lib!\n");
    		}

    		encode->lib_handle = NULL;
    	}
    	aenc_group[group_index].enc_handle = NULL;

        /* ccli unbind and unregist */
    	aenc_sysipc_unbind_handle(encode->enc_type);
        aenc_sys_ipc_unregister();

    	encode->enc_type = _SD_MEDIA_TYPE_UNKNOWN;
        free(encode);
        encode = NULL;
	}
	ak_thread_mutex_unlock(&aenc_ctrl.io_mutex);

	ak_print_info_ex( "leave...\n");

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
		ak_print_error_ex( "please open audio input first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return NULL;
	}
	if (NULL == enc_handle) {
		ak_print_error_ex( "please open audio encode first\n");
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
		ak_print_error_ex( "no matched audio encode type:%d\n",
		    encode->enc_type);
		goto aenc_request_end;
	}

	if (AK_AUDIO_TYPE_AMR == encode->enc_type)
		ak_ai_set_nr_max(ai_handle, AUDIO_FUNC_DISABLE);

	user = (struct audio_encode_user *)calloc(1, sizeof(struct audio_encode_user));
    if (!user) {
    	goto aenc_request_end;
    }

	user->enc_handle = enc_handle;
	user->group_index = index;

	ak_thread_mutex_lock(&aenc_ctrl.request_mutex);
	
	/* init aenc_group */
	init_aenc_group(index);

	/* if not set encode frame interval,use the ai frame interval */
	pcm_user_t *pcm_user = (pcm_user_t *)ai_handle;
	if (encode->frame_interval == 0) {
		encode->frame_interval = pcm_user->interval;
		ak_print_normal_ex( "set encode frame interval=%d\n", encode->frame_interval);
	}

	/* init aenc_group frame buffer*/
	frame_buffer_init(index, encode);

	if (!aenc_ctrl.pcm_rb) {
		
		int len = convert_interval_to_len(encode, pcm_user->interval);
		aenc_rb_init(len * 10, encode);
	}

	if (aenc_ctrl.pcm_rb) {
		aenc_rb_update_read_map(index, AK_TRUE);
	} else {
		ak_print_error_ex("aenc rb init failed \n");
		ak_thread_mutex_unlock(&aenc_ctrl.request_mutex);
		goto aenc_request_end;
	}

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
		if (!test_bit(i, &(aenc_group[index].req_map))) {
			user->req_nr = i;
			break;
		}
	}
	if (i >= AENC_TYPE_MAX_USER) {
		ak_thread_mutex_unlock(&aenc_group[index].mutex);
		goto aenc_request_end;
	}

    set_bit(&(aenc_group[index].req_map), user->req_nr);	
    add_ref(&(aenc_group[index].req_ref), 1);
	
    aenc_ctrl.read_flag = AK_TRUE;
    ret = AK_SUCCESS;
    ak_thread_mutex_unlock(&aenc_group[index].mutex);
    ak_thread_sem_post(&aenc_ctrl.ad_sem);

	ak_print_notice_ex( "init group_index=%d, user=%p, req_nr=%d, ref=%d\n",
		user->group_index, user, user->req_nr, aenc_group[index].req_ref);

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
	ak_print_notice_ex( "init group_index=%d, user=%p, req_nr=%d, ref=%d\n",
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

#if AENC_GET_STREAM_DEBUG
			ak_print_normal_ex( "pos=%p, len=%u, ts=%llu, seq_no=%lu\n",
				pos, pos->stream.len, pos->stream.ts, pos->stream.seq_no);
#endif
		} else {
			goto aenc_get_stream_end;
		}

#if AENC_GET_STREAM_DEBUG
		ak_print_normal_ex( "bit_map=%d, ref=%d\n",
		    pos->bit_map, pos->ref);
#endif
		if (pos->ref <= 0) {
			free_audio_entry(pos);
		}
	}
	aenc_ctrl.get_stream_count++;
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

	ak_print_info_ex( "entry...\n");
	struct audio_encode_user *user = (struct audio_encode_user *)stream_handle;
	int index = user->group_index;
	int user_bitmap = 0;

    ak_thread_mutex_lock(&aenc_group[index].using_mutex);
	ak_thread_mutex_lock(&aenc_group[index].mutex);
    del_ref(&(aenc_group[index].req_ref), 1);
    clear_bit(&(aenc_group[index].req_map), user->req_nr);

	/* get the user bit map */
	set_bit(&(user_bitmap), user->req_nr);
	ak_print_normal_ex("user bit map is %d\n", user_bitmap);

    int i = 0;
	for (i=0; i<AUDIO_ENCODE_TYPE_MAX; ++i) {
		if (aenc_group[i].req_ref > 0) {
			break;
		}
	}
	ak_thread_mutex_unlock(&aenc_group[index].mutex);

	/* all kinds of audio request are released */
	if (i >= AUDIO_ENCODE_TYPE_MAX) {
		ak_print_notice_ex( "all kinds of audio request are released\n");
		aenc_ctrl.read_flag = AK_FALSE;
		aenc_ctrl.run_flag = AK_FALSE;
		ak_ai_stop_capture(aenc_ctrl.ai_handle);

		ak_thread_sem_post(&aenc_ctrl.ad_sem);
		ak_print_normal_ex( "join read pcm thread...\n");
		ak_thread_join(aenc_ctrl.pcm_tid);
		ak_print_notice_ex( "read pcm thread join OK\n");

		ak_thread_sem_post(&aenc_ctrl.enc_sem);
		ak_print_normal_ex( "join audio encode thread...\n");
		ak_thread_join(aenc_ctrl.enc_tid);
		ak_print_notice_ex( "audio encode join OK\n");

		aenc_rb_deinit();

		aenc_ctrl.ai_handle = NULL;
		ak_thread_sem_destroy(&aenc_ctrl.ad_sem);
		ak_thread_sem_destroy(&aenc_ctrl.enc_sem);

		if (NULL != aenc_ctrl.encode_buf) {
			free(aenc_ctrl.encode_buf);
			aenc_ctrl.encode_buf = NULL;
		}
	}

	ak_thread_mutex_lock(&aenc_group[index].mutex);
	if (aenc_ctrl.pcm_rb && aenc_group[index].req_ref <= 0) {
		aenc_rb_update_read_map(index, AK_FALSE);
	}
	
	if (aenc_group[index].req_ref <= 0) {
		release_encode_data(index);
		if (aenc_group[index].pcm_frame.data){
			free(aenc_group[index].pcm_frame.data);	
			aenc_group[index].pcm_frame.data = NULL;
		}
	}

	/* release the stream of user bit map */
	release_encode_data_by_bitmap(index, user_bitmap);

    free(user);
    user = NULL;
    ak_thread_mutex_unlock(&aenc_group[index].mutex);

	/* destroy the used mutex */
	if (aenc_group[index].req_ref <= 0) {
		ak_print_info_ex( "destroy mutex encode type %d\n", index);
		ak_thread_mutex_destroy(&aenc_group[index].mutex);
		aenc_group[index].init_flag = AK_FALSE;
	}

    ak_print_info_ex( "leave...\n");
    ak_thread_mutex_unlock(&aenc_group[index].using_mutex);
    return AK_SUCCESS;
}

