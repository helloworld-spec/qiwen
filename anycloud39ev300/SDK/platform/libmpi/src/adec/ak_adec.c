#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "sdcodec.h"
#include "internal_error.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_ao.h"
#include "ak_adec.h"
#include "adec_ipcsrv.h"


#define ADEC_SEND_TIME			0
#define ADEC_STATUS_DEBUG		0

/* open this flag will save the pcm file after decode */
#define ADEC_DATA_DEBUG			0

#define ADEC_NO_DATA_TIME		(10)
#define ADEC_G711_MIN_BUF_LEN	(128)
#define ADEC_OUT_BUF_LEN		(256)
#define ADEC_PCM_DEFAULT_LEN	(2048)
#define ADEC_INBUF_LEN			(20480)

enum adec_mode {
	ADEC_MODE_SINGLE_FRAME = 0x00, 	/* single frame mode */
	ADEC_MODE_STREAM				/* stream mode */
};

/* decode status */
enum adec_stream_status {
	STREAM_STATUS_RESERVED = 0x00, 	/* reserved */
	STREAM_STATUS_READY,			/* ready */
	STREAM_STATUS_DECODING,			/* decoding */
	STREAM_STATUS_SENT_TO_AO,		/* send to ao */
	STREAM_STATUS_DECODE_FINISHED,	/* decode finish */
	STREAM_STATUS_PLAY_FINISHED		/* play finish */
};

struct audio_decode_user{
    void *ao_handle;				/* audio output handle */
    void *dec_handle;				/* audio decode handle */
    struct audio_entry *pos;		/* current user read queue pos */
    enum adec_stream_status status;	/* current decode status */
};

struct audio_decode_ctrl {
    unsigned char run_flag;			/* audio decode run flag */
    void *req_handle;				/* request decode handle */ 
    ak_mutex_t req_mutex;			/* request mutex */
    ak_sem_t data_sem;				/* decode data sem */
    ak_pthread_t pcm_tid;			/* write pcm data thread id */
	int senc_stream_count;
	int get_frame_count;
};

struct audio_decode {
    void *lib_handle;				/* decode lib handle */
    unsigned int sample_rate;		/* decode sample rate */
    unsigned int sample_bit;		/* decode sample bit */
    unsigned int channel_num;		/* channel number */
    unsigned int no_frame_count;    /* count of getting no decoded data */
    unsigned char decode_flag;      /* decode flag */
    unsigned char end_flag;         /* decode notice end flag */
    enum adec_mode mode;			/* decode mode */
    enum ak_audio_type type;        /* decode type */
    T_AUDIO_DECODE_OUT output;		/* decode lib output */
    ak_mutex_t mutex;
    char *out_buf;				/* decode out buffer */

    unsigned int frame_size;		/* frame size send to ao */
    unsigned char *frame_buf;       /* frame buffer send to ao */

    T_AUDIO_BUF_STATE buf_state;	/* decode buffer state */
    T_AUDIO_BUFFER_CONTROL buf_ctrl;/* decode buffer control info */

    unsigned char wait_flag;        ///< 标识码流是否通过等待方式终止，初始化后为 TRUE，
                                    ///< 当调用 ak_stream_cancel_stream 或 ak_stream_wait_stream 时会改变。

};

static const char adec_version[] = "libmpi_adec V1.0.12"; /* version number */

#ifndef AK_RTOS
static struct audio_decode_ctrl dec_ctrl = {
	0, NULL, PTHREAD_MUTEX_INITIALIZER
};
#else
/* decode control struct */
static struct audio_decode_ctrl dec_ctrl = {
	0, NULL, 0
};
#endif

static unsigned char declib_delay(unsigned long ticks)
{
	/* no need to delay */
    return 1;
}

static void* adec_malloc_cb(unsigned long size)
{
    return ak_malloc(size);
}

static void adec_free_cb(void *ptr)
{
    ak_free(ptr);
}

/**
 * init_decode_input - init audio encode info
 * @param[IN]: audio stream decode param
 * @config[OUT]: audio decode lib input config
 * return: void
 */
static void init_decode_input(const struct audio_param *param,
							T_AUDIO_DECODE_INPUT *config)
{
	if ((param->type <= AK_AUDIO_TYPE_UNKNOWN)
		|| (param->type > AK_AUDIO_TYPE_SPEEX)) {
        ak_print_error_ex("unsupport audio decode type\n");
		return;
	}

    config->cb_fun.Free = adec_free_cb;
    config->cb_fun.Malloc = adec_malloc_cb;
    config->cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;
    config->cb_fun.delay = declib_delay;

	config->m_info.m_InbufLen = ADEC_INBUF_LEN;
    config->m_info.m_SampleRate = param->sample_rate;

    /* the real channel is decided to _SD_Decode */
    config->m_info.m_Channels = param->channel_num;
    config->m_info.m_BitsPerSample = param->sample_bits;
    config->chip = AUDIOLIB_CHIP_AK39XXEV3;

	config->m_info.m_Type = param->type;
	/* get the decode type */
    switch(param->type){
    case AK_AUDIO_TYPE_MP3:
    	ak_print_notice_ex("decode type=%d: MP3\n", param->type);
        break;
    case AK_AUDIO_TYPE_AMR:
    	ak_print_notice_ex("decode type=%d: AMR\n", param->type);
    	config->m_info.m_SampleRate = 8000;	/*only support 8K sample rate */
    	/* only support mono channel */
        config->m_info.m_Channels = AUDIO_CHANNEL_MONO;
        break;
	case AK_AUDIO_TYPE_AAC:
		ak_print_notice_ex("decode type=%d: AAC\n", param->type);
        /* no use setting now, may be use in the future. */
        config->m_info.m_Private.m_aac.cmmb_adts_flag = 0;
        break;
    case AK_AUDIO_TYPE_PCM:
    	ak_print_notice_ex("decode type=%d: PCM\n", param->type);
        break;
    case AK_AUDIO_TYPE_PCM_ALAW:
    	ak_print_notice_ex("decode type=%d: PCM_ALAW\n", param->type);
        break;
    case AK_AUDIO_TYPE_PCM_ULAW:
    	ak_print_notice_ex("decode type=%d: PCM_ULAW\n", param->type);
        break;
    case AK_AUDIO_TYPE_DRA:
    	ak_print_notice_ex("decode type=%d: DRA\n", param->type);
        break;
    default:
    	ak_print_notice_ex("decode type=%d\n", param->type);
    	break;
	}

	ak_print_notice_ex("sample_rate=%d, channel_num=%d, sample_bits=%d\n",
    	param->sample_rate, param->channel_num, param->sample_bits);
}

/**
 * write_decode_buffer - write data to decode buffer
 * @dec_handle[IN]: decode handle
 * @stream[IN]: write stream
 * @write_len[IN]: write len to decode buffer
 * return: void
 */
static void write_decode_buffer(void *dec_handle,
								const unsigned char *stream,
								unsigned int write_len)
{
	struct audio_decode *decode = (struct audio_decode *)dec_handle;

	ak_thread_mutex_lock(&(decode->mutex));
	if(_SD_BUFFER_WRITABLE == decode->buf_state){
		memcpy((unsigned char *)(decode->buf_ctrl.pwrite), stream, write_len);
		_SD_Buffer_Update(decode->lib_handle, write_len);
	}else{
		if(write_len <= decode->buf_ctrl.free_len){
			memcpy((unsigned char *)(decode->buf_ctrl.pwrite), stream, write_len);
			_SD_Buffer_Update(decode->lib_handle, write_len);
		}else{
			int remain = write_len - decode->buf_ctrl.free_len;
			memcpy((unsigned char *)(decode->buf_ctrl.pwrite),
				stream, decode->buf_ctrl.free_len);
			_SD_Buffer_Update(decode->lib_handle, decode->buf_ctrl.free_len);
			memcpy((unsigned char *)(decode->buf_ctrl.pstart),
				(stream + decode->buf_ctrl.free_len), remain);
			_SD_Buffer_Update(decode->lib_handle, remain);
		}
	}
	ak_thread_mutex_unlock(&(decode->mutex));
}

/**
 * get_decode_remain_size - get waiting decoded size in decode buffer.
 * @decode[in]: audio decode info
 * return: -1 failed; > 0 decode remain size in decode buffer
 */
static int get_decode_remain_size(struct audio_decode *decode)
{
	if(!decode) {
		return AK_FAILED;
	}

	int remain = _SD_Get_Input_Buf_Info(decode->lib_handle,
	    _STREAM_BUF_REMAIN_DATA);

	return remain;
}

/**
 * check_decode_finished - check whether decode finish 
 * @user[IN]: audio decode user
 * return: NULL
 */
static void check_decode_finished(struct audio_decode_user *user)
{
    struct audio_decode *decode = (struct audio_decode *)(user->dec_handle);
    int remain = get_decode_remain_size(decode);

#if ADEC_STATUS_DEBUG
    ak_print_normal_ex("decode_count=%d, remain=%d\n",
        decode->no_frame_count, remain);
#endif

    if (0 == remain) {
        user->status = STREAM_STATUS_DECODE_FINISHED;
        ak_print_notice_ex("decode finished, remain is 0\n");
    }
    if (decode->end_flag && (decode->no_frame_count >= ADEC_NO_DATA_TIME)) {
        ak_print_notice_ex("notice decode end, decode_count=%d\n",
            decode->no_frame_count);
        user->status = STREAM_STATUS_DECODE_FINISHED;
    }
}

/**
 * get_frame_and_play - get decode frame data and send to ao
 * @user[IN]: audio decode user
 * @frame_buf[OUT]: frame buffer for decoding output data
 * return: >= 0 decode output data size, -1 failed
 */
static int get_frame_and_play(struct audio_decode_user *user,
							unsigned char *frame_buf)
{
	int out_size = 0;
	int cur_size = 0;
	struct audio_decode *decode = (struct audio_decode *)(user->dec_handle);

	if (decode->frame_size < ADEC_OUT_BUF_LEN) {
		ak_print_error_ex("frame size is too small\n");
		return AK_FAILED;
	}

#if ADEC_SEND_TIME
	static struct ak_timeval start_time;
	struct ak_timeval end_time;

	if (0 == start_time.sec) {
		ak_get_ostime(&start_time);
	}
#endif

	do{	
		/* get frame */
		out_size = ak_adec_get_frame(user->dec_handle, &frame_buf[cur_size]);
		if(out_size > 0){
		    decode->no_frame_count = 0;
			cur_size += out_size;

			if(cur_size >= decode->frame_size){
				/* ccli save stream to file to debug */
				adec_save_stream_to_file(decode->type, frame_buf, cur_size, 
										AFTER_DEC_LEVEL);
				/* send frame to play */
	            if (AK_FAILED == ak_ao_send_frame(user->ao_handle,
	            	frame_buf, cur_size, -1)) {
					ak_print_error_ex("write pcm to DA error\n");
	           	} else {
	           	    user->status = STREAM_STATUS_SENT_TO_AO;
	           	}

#if ADEC_SEND_TIME
				ak_get_ostime(&end_time);
				ak_print_info_ex("dec->send time=%ld, cur_size=%d\n",
					ak_diff_ms_time(&end_time, &start_time), cur_size);
				ak_get_ostime(&start_time);
#endif

	            cur_size = 0;
	        }
		} else if (out_size < 0){
		    user->status = STREAM_STATUS_DECODE_FINISHED;
		} else {
		    ++decode->no_frame_count;
		}
	} while (out_size > 0 && decode->wait_flag);

	if (cur_size > 0) {	
		adec_save_stream_to_file(decode->type, frame_buf, cur_size,
								AFTER_DEC_LEVEL);
		/* the end of the file may less then 4096 */
		if (AK_FAILED == ak_ao_send_frame(user->ao_handle,
			frame_buf, cur_size, 0)) {
			ak_print_error_ex("write pcm to DA error\n");
        } else {
       	    user->status = STREAM_STATUS_SENT_TO_AO;
       	}

#if ADEC_SEND_TIME
		ak_get_ostime(&end_time);
		ak_print_info_ex("dec->send time=%ld, cur_size=%d\n",
			ak_diff_ms_time(&end_time, &start_time), cur_size);
		ak_get_ostime(&start_time);
#endif
	}

	/* check decode status */
    check_decode_finished(user);

	return out_size;
}

/**
 * write_pcm_thread - write audio pcm data to DA
 * @arg[IN]: struct audio_decode_user
 * return: NULL
 */
static void* write_pcm_thread(void *arg)
{
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	ak_thread_set_name("adec_write_da");

	struct audio_decode_user *user = (struct audio_decode_user *)arg;
	struct audio_decode *decode = (struct audio_decode *)(user->dec_handle);

    ak_print_normal("\t--- %s start processing ---\n", __func__);
	while (dec_ctrl.run_flag) {
        ak_print_normal_ex("sleep..., decode buffer remain=%d\n",
            get_decode_remain_size(decode));
		ak_thread_sem_wait(&(dec_ctrl.data_sem));
        ak_print_normal_ex("wakeup...\n");

        while (dec_ctrl.run_flag && decode->decode_flag) {
            user->status = STREAM_STATUS_DECODING;
    		if (0 == get_frame_and_play(user, decode->frame_buf)) {
    		    ak_sleep_ms(10);
    		}

            if ((STREAM_STATUS_DECODE_FINISHED == user->status)
                || (STREAM_STATUS_PLAY_FINISHED == user->status)) {
                if (get_frame_and_play(user, decode->frame_buf) > 0) {
					ak_print_warning_ex("when finish get_frame_and_play > 0\n");
                }
				if (user->status == STREAM_STATUS_SENT_TO_AO)
					user->status = STREAM_STATUS_DECODE_FINISHED;
				
                decode->decode_flag = AK_FALSE;
    			break;
    		}
        }
	}

    ak_print_normal_ex("exit\n");
	ak_thread_exit();

	return NULL;
}

/**
 * start_decode_ctrl - start audio decode control
 * @user[IN]: audio decode user
 * return: 0 success, -1 failed
 */
static int start_decode_ctrl(struct audio_decode_user *user)
{
	struct audio_decode *decode = (struct audio_decode *)(user->dec_handle);

	decode->frame_buf = ak_calloc(1, (decode->frame_size << 1));
	if(!decode->frame_buf){
		ak_print_error_ex("calloc failed\n");
		return AK_FAILED;
	}

	_SD_SetBufferMode(decode->lib_handle, _SD_BM_NORMAL);
	if ((AK_AUDIO_TYPE_PCM_ALAW == decode->type)
	    || (AK_AUDIO_TYPE_PCM_ULAW == decode->type)){
        _SD_SetInbufMinLen(decode->lib_handle, ADEC_G711_MIN_BUF_LEN);
        ak_print_normal_ex("set G711 decode min buffer size\n");
	}

    ak_thread_sem_init(&(dec_ctrl.data_sem), 0);
	user->status = STREAM_STATUS_READY;

	/* create the capture data thread */
	int ret = ak_thread_create(&(dec_ctrl.pcm_tid), write_pcm_thread,
			user, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if (ret != 0){
		ak_print_error("create write_pcm_thread thread FAILED, ret=%d\n", ret);
		ak_free(decode->frame_buf);
		decode->frame_buf = NULL;
		ak_thread_sem_destroy(&(dec_ctrl.data_sem));
	    user->status = STREAM_STATUS_RESERVED;
	}
	ak_sleep_ms(10);

	return ret;
}

/**
 * wait_adec_play_finished - wait adec play finished
 * @user[IN]: audio decode user
 * return: 0 success, -1 failed
 */
static int wait_adec_play_finished(struct audio_decode_user *user)
{
	int count = 0;
	ak_print_normal_ex("ao play status=%d, user->status=%d,--%p\n",
		ak_ao_get_play_status(user->ao_handle), user->status, user);

    /* get stream data to be decoded, status won't be READY */
    while (STREAM_STATUS_DECODE_FINISHED != user->status) {
		if (count == 20) 
			ak_print_normal_ex("user handle=%p, user->status=%d\n",
						user, user->status);
		count++;
	
		ak_sleep_ms(100);
	}
    ak_print_notice_ex("decode finished...\n");

    int status = AO_PLAY_STATUS_RESERVED;

    ak_ao_notice_frame_end(user->ao_handle);
    /* wait play end */
    do {
        status = ak_ao_get_play_status(user->ao_handle);

#if ADEC_STATUS_DEBUG
        ak_print_normal_ex("ao play status=%d, user->status=%d\n",
		    status, user->status);
#endif

		switch (status) {
		case AO_PLAY_STATUS_FINISHED:
		    ak_print_notice_ex("play finished\n");
		    break;
		case AO_PLAY_STATUS_DATA_NOT_ENOUGH:
		    ak_print_notice_ex("not enough data to play\n");
		    break;
		default:
		    ak_sleep_ms(100);
		    break;
		}
    } while ((AO_PLAY_STATUS_FINISHED != status)
        && (AO_PLAY_STATUS_DATA_NOT_ENOUGH != status));

	user->status = STREAM_STATUS_PLAY_FINISHED;
	ak_sleep_ms(10);

	return AK_SUCCESS;
}

/**
 * ak_adec_get_version - get audio decode version
 * return: version string
 * notes:
 */
const char* ak_adec_get_version(void)
{
	return adec_version;
}

/**
 * ak_adec_open - open anyka audio decode
 * @param[in]: audio stream decode param
 * return: opened decode handle
 */
void* ak_adec_open(const struct audio_param *param)
{
	if (NULL == param){
		ak_print_error_ex("invalid audio decode param\n");
		return NULL;
	}

	if (param->sample_rate < 8000 || param->sample_rate > 64000) {
		/* sample rate not support  */
		ak_print_error_ex("sample rate not suppprt %d \n", param->sample_rate);
		return NULL;
	}

    struct audio_decode *decode = (struct audio_decode *)ak_calloc(
    	1, sizeof(struct audio_decode));
    if(NULL == decode) {
        ak_print_error_ex("calloc failed\n");
        return NULL;
    }

	int ret = AK_FAILED;
	T_AUDIO_DECODE_INPUT input; /* decode lib input */

    memset(&input, 0, sizeof(T_AUDIO_DECODE_INPUT));
    init_decode_input(param, &input);

    memset(&(decode->output), 0, sizeof(T_AUDIO_DECODE_OUT));

    /* open decode lib */
    decode->lib_handle = _SD_Decode_Open(&input, &(decode->output));
    if (NULL == decode->lib_handle) {
        ak_print_error_ex("_SD_Decode_Open failed\n");
        goto open_end;
    }

	/**
	 * set decode attribute
	 * _SD_BM_NORMAL means decode rigth now when getting data
	 */
    _SD_SetBufferMode(decode->lib_handle, _SD_BM_NORMAL);

    decode->out_buf = (char *)ak_calloc(1, ADEC_OUT_BUF_LEN);
    if(NULL == decode->out_buf) {
        ak_print_error_ex("calloc audio decode out buffer failed\n");
        goto open_end;
    }

	decode->sample_rate = param->sample_rate;
	decode->sample_bit = param->sample_bits;
	decode->channel_num = param->channel_num;
	decode->frame_size = ADEC_PCM_DEFAULT_LEN;
	decode->mode = ADEC_MODE_SINGLE_FRAME;
	decode->type = param->type;
	decode->wait_flag = AK_TRUE; ///< 初始化必须为 TRUE，由于之后循环条件使用到该值。
    ak_thread_mutex_init(&(decode->mutex), NULL);
	ret = AK_SUCCESS;

    ak_print_notice_ex("audio decode output, m_ulSize=%ld, m_ulDecDataSize=%ld\n",
        decode->output.m_ulSize, decode->output.m_ulDecDataSize);
    ak_print_notice_ex("m_SampleRate=%ld, m_Channels=%d, m_BitsPerSample=%d\n",
        decode->output.m_SampleRate, decode->output.m_Channels,
        decode->output.m_BitsPerSample);
    ak_print_notice_ex("decode buffer free size=%d\n",
        ak_adec_get_free_size(decode));

	adec_sys_ipc_register();
	adec_sysipc_bind_handle(decode);
open_end:
	if(AK_FAILED == ret) {
		if(decode) {
			if(decode->lib_handle) {
				_SD_Buffer_Clear(decode->lib_handle);
				_SD_Decode_Close(decode->lib_handle);
			}

			ak_free(decode);
	        decode = NULL;
		}
	}

    return decode;
}

/**
 * ak_adec_get_params - get adec parameter
 * @handle[IN]: adec opened handle
 * @param[OUT]: store adec params delivery by adec_open();
 * return: 0 success, -1 failed
 * notes: call after set all kind of DAC attr
 */
int ak_adec_get_params(void *handle, struct audio_param *param)
{
	/* param check */
	if(!handle || !param) {
		ak_print_error_ex("invalid param\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	struct audio_decode *decode = (struct audio_decode *)handle;

	/* value assingment */
	param->sample_rate = decode->sample_rate;
	param->sample_bits = decode->sample_bit;
	param->channel_num = decode->channel_num;
	param->type = decode->type;

	return AK_SUCCESS;
}

/**
 * ak_adec_get_free_size - get audio decode buffer free size
 * @dec_handle[in]: opened decode handle
 * return: -1 failed; otherwize decode buffer free size
 */
int ak_adec_get_free_size(void *dec_handle)
{
	if(NULL == dec_handle) {
		return AK_FAILED;
	}

    int free_size = 0;
    struct audio_decode *decode = (struct audio_decode *)dec_handle;

    ak_thread_mutex_lock(&(decode->mutex));
	decode->buf_state = _SD_Buffer_Check(decode->lib_handle, &(decode->buf_ctrl));
	switch(decode->buf_state){
	case _SD_BUFFER_FULL:
		break;
	case _SD_BUFFER_WRITABLE:
		free_size = decode->buf_ctrl.free_len;
		break;
	default:
		free_size = decode->buf_ctrl.free_len + decode->buf_ctrl.start_len;
		break;
	}
	ak_thread_mutex_unlock(&(decode->mutex));

	return free_size;
}

/**
 * ak_adec_send_stream - decode audio stream
 * @dec_handle[in]: opened decode handle
 * @data[in]: data add to decode buffer
 * @len[in]: add to decode buffer data len
 * @ms[in]: <0 block mode; =0 non-block mode; >0 wait time
 * return: >=0 len added to decode buffer; otherwize -1
 */
int ak_adec_send_stream(void *dec_handle,
						const unsigned char *data,
						unsigned int len,
						long ms)
{
	if(NULL == dec_handle) {
		return AK_FAILED;
	}
	if(NULL == data) {
		return AK_FAILED;
	}
	if(len <= 0) {
		return AK_FAILED;
	}

	int stream_len = len;
	unsigned int offset = 0;
	int free_size = ak_adec_get_free_size(dec_handle);
	struct audio_decode *decode = (struct audio_decode *)dec_handle;

	if(ms < 0) {
		/* free space is not enough in block mode */
		while(free_size < stream_len) {
			if (free_size > 0) {
				adec_save_stream_to_file(decode->type, &(data[offset]),
										free_size, BEFORE_DEC_LEVEL);
				write_decode_buffer(dec_handle, &(data[offset]), free_size);
				offset += free_size;
				stream_len -= free_size;
			}

			ak_sleep_ms(10);
			free_size = ak_adec_get_free_size(dec_handle);
		}

		adec_save_stream_to_file(decode->type, &(data[offset]), stream_len,
								BEFORE_DEC_LEVEL);
		/* send to decode lib and decode */
		write_decode_buffer(dec_handle, &(data[offset]), stream_len);
		offset += stream_len;
	} else {
		/* free space is not enough in non-block mode */
	    if(free_size < stream_len){
	    	ak_sleep_ms(10);
 			return offset;
	    }

		adec_save_stream_to_file(decode->type, data, stream_len,
								BEFORE_DEC_LEVEL);
	    /* we don't support timeout mode, so we do it as non-block mode */
		dec_ctrl.senc_stream_count++;
	    write_decode_buffer(dec_handle, data, stream_len);
	    offset += stream_len;
	}
	
#if ADEC_STATUS_DEBUG
    ak_print_normal_ex("offset=%d, len=%d, free_size=%d, decode_flag=%d\n",
		offset, len, free_size, decode->decode_flag);
#endif

	if (!decode->decode_flag
	    && (offset > 0) && (ADEC_MODE_STREAM == decode->mode)) {
	    decode->decode_flag = AK_TRUE;
	    decode->wait_flag = AK_TRUE;
    	ak_thread_sem_post(&(dec_ctrl.data_sem));
	}

	return offset;
}

/**
 * ak_adec_get_frame -
 * @dec_handle[in]: opened decode handle
 * @frame[OUT]: audio decode frame out buffer
 * return: >0 decode frame len; -1 failed
 */
int ak_adec_get_frame(void *dec_handle, unsigned char *frame)
{
	if((NULL == dec_handle) || (NULL == frame)) {
		return AK_FAILED;
	}

	static int print_param = 1;
	struct audio_decode *decode = (struct audio_decode *)dec_handle;

	/* compute the buffer address and length */
	decode->output.m_pBuffer = (unsigned char *)decode->out_buf;
	decode->output.m_ulSize = ADEC_OUT_BUF_LEN;

	int ret = _SD_Decode(decode->lib_handle, &(decode->output));
    if(ret > 0) {
    	if (print_param) {
			print_param = 0;
			ak_print_notice_ex("m_SampleRate=%ld, m_Channels=%d, m_BitsPerSample=%d\n",
		    	decode->output.m_SampleRate, decode->output.m_Channels,
		    	decode->output.m_BitsPerSample);
    	}

		/* channel mono or stereo is decided by _SD_Decode */
    	if(decode->output.m_Channels >= AUDIO_CHANNEL_STEREO){
            memcpy(frame, decode->out_buf, ret);
        }else{
            int i = 0;
            unsigned short *in = (unsigned short *)decode->out_buf;
            unsigned short *out = (unsigned short *)frame;

            for(i = 0; i < (ret >> 1); i++){
                *out++ = *in;
                *out++ = *in++;
            }
			ret *= 2;
        }
		dec_ctrl.get_frame_count++;
    } else {
#if ADEC_STATUS_DEBUG
        ak_print_notice_ex("ret=%d\n", ret);
#endif
    }

    return ret;
}

/**
 * ak_adec_set_frame_size - set frame size that send to ao after decode
 * @dec_handle[IN]: opened audio encode handle
 * @frame_size[IN]: appointed frame size, [256, 4096], unit: byte
 * return: 0 success, -1 failed
 * notes: 1. if you do not call, the default frame size is 2048
 * 		2. only work in stream mode
 */
int ak_adec_set_frame_size(void *dec_handle, unsigned int frame_size)
{
	if(!dec_handle) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	/* min is ADEC_OUT_BUF_LEN(256), max is 4096 */
	if((frame_size < ADEC_OUT_BUF_LEN)
		|| (frame_size > (ADEC_PCM_DEFAULT_LEN << 1))) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}

	if(dec_ctrl.run_flag){
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return AK_FAILED;
    }

	struct audio_decode *decode = (struct audio_decode *)dec_handle;

	ak_print_notice_ex("set frame size from %d to %d\n",
		decode->frame_size, frame_size);
	decode->frame_size = frame_size;

	return AK_SUCCESS;
}

/**
 * ak_adec_notice_stream_end - notice send stream end
 * @dec_handle[in]: opened decode handle
 * return: 0 success; -1 failed
 * note: when play data send end,use this function
 */
int ak_adec_notice_stream_end(void *dec_handle)
{
	if (!dec_handle) {
		return AK_FAILED;
	}

	struct audio_decode *decode = (struct audio_decode *)dec_handle;

    decode->end_flag = AK_TRUE;
    ak_print_notice_ex("called, decode free size=%d\n",
        ak_adec_get_free_size(dec_handle));

	return _SD_SetBufferMode(decode->lib_handle, _SD_BM_ENDING);
}

/**
 * ak_adec_close - close anyka audio decode
 * @handle[in]: opened decode handle
 * return: 0 success; -1 failed
 */
int ak_adec_close(void *dec_handle)
{
	if (!dec_handle) {
	    set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return AK_FAILED;
	}

    struct audio_decode *decode = (struct audio_decode *)dec_handle;

    ak_thread_mutex_destroy(&(decode->mutex));
    if(decode->lib_handle) {
    	_SD_Buffer_Clear(decode->lib_handle);
		_SD_Decode_Close(decode->lib_handle);
		decode->lib_handle = NULL;
    }
	if(decode->out_buf) {
        ak_free(decode->out_buf);
        decode->out_buf = NULL;
    }

    adec_sysipc_unbind_handle(decode->type);
	adec_sys_ipc_unregister();

    ak_free(decode);
    decode = NULL;

    return AK_SUCCESS;
}

/**
 * ak_adec_request_stream - request audio data stream according to handle
 * @ao_handle[in]: audio output handle
 * @dec_handle[in]: audio decode handle
 * return: stream handle, NULL failed
 */
void* ak_adec_request_stream(void *ao_handle, void *dec_handle)
{
    ak_print_info_ex("enter...\n");
	if (!ao_handle){
		ak_print_error_ex("please open audio output first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return NULL;
	}
	if (!dec_handle){
		ak_print_error_ex("please open audio decode first\n");
		set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
		return NULL;
	}

    ak_thread_mutex_lock(&(dec_ctrl.req_mutex));	
	if (!dec_ctrl.req_handle) {
		dec_ctrl.req_handle = dec_handle;
		ak_print_normal_ex("request %p\n", dec_handle);
	} else {
		ak_print_warning_ex("request already:");		
		ak_print_warning_ex("dec_ctrl.req_handle=%p, dec_handle=%p\n",
						dec_ctrl.req_handle, dec_handle);
		
		ak_thread_mutex_unlock(&(dec_ctrl.req_mutex));
		return NULL;
	}

	int ret = AK_FAILED;
    struct audio_decode_user *user = (struct audio_decode_user *)ak_calloc(1,
    	sizeof(struct audio_decode_user));
    if(!user){
    	set_error_no(ERROR_TYPE_MALLOC_FAILED);
    	goto request_end;
    }

    user->ao_handle = ao_handle;
    user->dec_handle = dec_handle;
    user->pos = NULL;
    user->status = STREAM_STATUS_RESERVED;
	ak_print_normal_ex("user status change to STATUS_RESERVED\n");

	/* set decode stream mode */
	struct audio_decode *decode = (struct audio_decode *)dec_handle;
	decode->mode = ADEC_MODE_STREAM;

	if(!dec_ctrl.run_flag){
		dec_ctrl.run_flag = AK_TRUE;
		if (AK_FAILED == start_decode_ctrl(user)) {
			dec_ctrl.run_flag = AK_FALSE;
			goto request_end;
		}
	}
    ret = AK_SUCCESS;

request_end:
	if(AK_FAILED == ret) {
		if(NULL != user) {
			ak_free(user);
			user = NULL;
		}
	}
	
    ak_thread_mutex_unlock(&(dec_ctrl.req_mutex));
	ak_print_info_ex("leave..., ret=%d\n", ret);

	return user;
}



int ak_adec_cancel_stream_no_wait (void *stream_handle)
{

    if(!stream_handle){
        set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
        return AK_FAILED;
    }

	struct audio_decode_user *user = (struct audio_decode_user *)stream_handle;
	struct audio_decode *decode = (struct audio_decode *)(user->dec_handle);

	/// 当前退出方式不作等待。
	decode->wait_flag = AK_FALSE;

	/// 变更标识位以后，调用等待接口。
	return ak_adec_cancel_stream (stream_handle);
}



/**
 * ak_adec_cancel_stream - cancel audio data stream according to stream handle
 * @dec_handle[in]: opened stream handle
 * return: 0 success, -1 failed
 */
int ak_adec_cancel_stream(void *stream_handle)
{
	ak_print_info_ex("enter...\n");
	if(!dec_ctrl.run_flag){
	    set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
        return AK_FAILED;
    }

    if(!stream_handle){
        set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
        return AK_FAILED;
    }

	struct audio_decode_user *user = (struct audio_decode_user *)stream_handle;
	struct audio_decode *decode = (struct audio_decode *)(user->dec_handle);

	if(!decode){
	    set_error_no(ERROR_TYPE_CALL_ORDER_WRONG);
        return AK_FAILED;
    }
   
    ak_thread_mutex_lock(&(dec_ctrl.req_mutex));	

    if (decode->wait_flag) {
        wait_adec_play_finished(user);
		decode->wait_flag = AK_FALSE;
    }

	/* reset ao play status */
	ak_ao_set_play_status(user->ao_handle, AO_PLAY_STATUS_READY);

	dec_ctrl.run_flag = AK_FALSE;
	decode->end_flag = AK_FALSE;
	ak_thread_sem_post(&(dec_ctrl.data_sem));

	ak_print_notice_ex("ready to join write_pcm_thread\n");
	ak_thread_join(dec_ctrl.pcm_tid);
	ak_print_notice_ex("write_pcm_thread join OK\n");

    if (decode->frame_buf) {
		ak_free(decode->frame_buf);
		decode->frame_buf = NULL;
    }
	ak_thread_sem_destroy(&(dec_ctrl.data_sem));
	
	_SD_Buffer_Clear(decode->lib_handle);

	if (dec_ctrl.req_handle == decode) {
		dec_ctrl.req_handle = NULL;
	} 

    ak_free(user);
    user = NULL;    
    ak_thread_mutex_unlock(&(dec_ctrl.req_mutex));	

    ak_print_info_ex("leave...\n");

    return AK_SUCCESS;
}



