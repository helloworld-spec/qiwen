#include <stdio.h>
#include <string.h>

#include "sdcodec.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_ao.h"
#include "ak_adec.h"

#define ADEC_SEND_TIME			0
#define ADEC_DATA_DEBUG			0

#define AUDIO_DECODE_MAX_SIZE	2048
#define ADEC_INBUF_LEN			(20*1024)

enum adec_mode {
	ADEC_MODE_SINGLE_FRAME = 0x00,
	ADEC_MODE_STREAM
};

struct audio_decode_user{
    void *ao_handle;				//audio output handle
    void *dec_handle;				//audio decode handle
    struct audio_entry *pos;		//current user read queue pos
    enum adec_stream_status status;	//current decode status
};

struct audio_decode_ctrl {
    unsigned char run_flag;			//audio decode run flag
    unsigned char read_flag;		//read pcm flag
    ak_sem_t data_sem;				//decode data sem
    ak_pthread_t pcm_tid;			//write pcm data thread id
    int encode_len;
    unsigned char *encode_buf;
};

struct audio_decode {
    void *lib_handle;				//decode lib handle
    unsigned int channel_num;
    enum adec_mode mode;			//decode mode
    T_AUDIO_DECODE_OUT output;
    ak_mutex_t mutex;
    char *stream;					//decode stream buffer(input data)
    char *out_buf;					//decode out buffer

    T_AUDIO_BUF_STATE buf_state;	//decode buffer state
    T_AUDIO_BUFFER_CONTROL buf_ctrl;//decode buffer control info
};

static const char adec_version[] = "libmpi_adec V2.0.01";

static struct audio_decode_ctrl dec_ctrl = {0};

#if ADEC_DATA_DEBUG
static FILE *adec_fp = NULL;
#endif

static unsigned char lnx_delay(unsigned long ticks)
{
	ak_sleep_ms(ticks*5);
    return 1;
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
	
	config->cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)free;
    config->cb_fun.Malloc =  (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
    config->cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
    config->cb_fun.delay = lnx_delay;
    
	config->m_info.m_InbufLen = ADEC_INBUF_LEN;
    config->m_info.m_SampleRate = param->sample_rate;
    /* the real channel is decided to _SD_Decode */
    config->m_info.m_Channels = param->channel_num;
    //config->m_info.m_Channels = AUDIO_CHANNEL_MONO;
    config->m_info.m_BitsPerSample = param->sample_bits;
    config->chip = AUDIOLIB_CHIP_AK39XXEV2;

	config->m_info.m_Type = param->type;
    switch(param->type){
    case AK_AUDIO_TYPE_MP3:
    	ak_print_notice_ex("decode type=%d: MP3\n", param->type);
        break;
    case AK_AUDIO_TYPE_AMR:
    	ak_print_notice_ex("decode type=%d: AMR\n", param->type);
    	config->m_info.m_SampleRate = 8000;	//only support 8K sample rate
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
 * get_frame_and_play - get decode frame data and send to ao
 * @user[IN]: audio decode user
 * @decode_out[OUT]: decode output data buffer
 * return: >= 0 decode output data size, -1 failed
 */
static int get_frame_and_play(struct audio_decode_user *user, 
							unsigned char *decode_out)
{
	int out_size = 0;
	int cur_size = 0;

#if ADEC_SEND_TIME
	static struct ak_timeval start_time;
	struct ak_timeval end_time;

	if (0 == start_time.sec) {
		ak_get_ostime(&start_time);
	}
#endif
	
	do{
		out_size = ak_adec_get_frame(user->dec_handle, &decode_out[cur_size]);
		if(out_size > 0){
			cur_size += out_size;
			
			if(cur_size >= (AUDIO_DECODE_MAX_SIZE << 1)){
#if ADEC_DATA_DEBUG
				fwrite(decode_out, 1, cur_size, adec_fp);
#endif
				user->status = STREAM_STATUS_GET_DEC_DATA;
	            if (AK_FAILED == ak_ao_send_frame(user->ao_handle, 
	            	decode_out, cur_size, -1)) {
					ak_print_error_ex("write pcm to DA error\n");
	           	}

#if ADEC_SEND_TIME
				ak_get_ostime(&end_time);
				ak_print_info_ex("dec->send time=%ld\n", 
					ak_diff_ms_time(&end_time, &start_time));
				ak_get_ostime(&start_time);
#endif

	            cur_size = 0;
	            ak_sleep_ms(5);
	        }
		}
	} while(out_size > 0);

	if (cur_size > 0) {
#if ADEC_DATA_DEBUG
		fwrite(decode_out, 1, cur_size, adec_fp);
#endif

		/* the end of the file may less then 4096 */ 
		if (AK_FAILED == ak_ao_send_frame(user->ao_handle, 
			decode_out, cur_size, 0)) {
			ak_print_error_ex("write pcm to DA error\n");
        }

#if ADEC_SEND_TIME
		ak_get_ostime(&end_time);
		ak_print_info_ex("dec->send time=%ld\n", 
			ak_diff_ms_time(&end_time, &start_time));
		ak_get_ostime(&start_time);
#endif

        ak_sleep_ms(5);
	}

	return out_size;
}

/* write audio pcm data to DA */
static void* write_pcm_thread(void *arg)
{
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());

	unsigned char *decode_out = calloc(1, (AUDIO_DECODE_MAX_SIZE << 2));
	if(NULL == decode_out){
		ak_print_error_ex("calloc failed\n");
		goto write_pcm_end;
	}

	struct audio_decode_user *user = (struct audio_decode_user *)arg;
	struct audio_decode *decode = (struct audio_decode *)(user->dec_handle);

	_SD_SetBufferMode(decode->lib_handle, _SD_BM_NORMAL);
	user->status = STREAM_STATUS_READY;
    ak_print_normal("\t--- %s start processing ---\n", __func__);
	
	while(dec_ctrl.run_flag){
		ak_thread_sem_wait(&(dec_ctrl.data_sem));

		user->status = STREAM_STATUS_DECODING;
		if (0 == get_frame_and_play(user, decode_out)) {
			ak_sleep_ms(10);
		}
		if (STREAM_STATUS_PLAY_FINISHED == user->status) {
			break;
		}
	}

write_pcm_end:
	if(decode_out){
		free(decode_out);
		decode_out = NULL;
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
	ak_thread_sem_init(&(dec_ctrl.data_sem), 0);
#if ADEC_DATA_DEBUG
    adec_fp = fopen("/tmp/adec_frame.pcm", "w+");
    if (NULL == adec_fp) {
		ak_print_normal_ex("open adec_frame.pcm failed\n");
    }
#endif

	/* create the capture data thread */
	int ret = ak_thread_create(&(dec_ctrl.pcm_tid), write_pcm_thread,
			user, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if (ret != 0){
		ak_print_error("create write_pcm_thread thread FAILED, ret=%d\n", ret);
		return AK_FAILED;
	}
	ak_sleep_ms(10);

	return AK_SUCCESS;
}

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

    struct audio_decode *decode = (struct audio_decode *)calloc(
    	1, sizeof(struct audio_decode));
    if(NULL == decode) {
        ak_print_error_ex("calloc failed\n");
        return NULL;
    }

	int ret = AK_FAILED;
	T_AUDIO_DECODE_INPUT input;

    memset(&input, 0, sizeof(T_AUDIO_DECODE_INPUT));
    init_decode_input(param, &input);

    memset(&(decode->output), 0, sizeof(T_AUDIO_DECODE_OUT));
    decode->lib_handle = _SD_Decode_Open(&input, &(decode->output));
    if (NULL == decode->lib_handle) {
        ak_print_error_ex("_SD_Decode_Open failed\n");
        goto open_end;
    }

	ak_print_notice_ex("audio decode output, m_ulSize=%ld, m_ulDecDataSize=%ld\n", 
    	decode->output.m_ulSize, decode->output.m_ulDecDataSize);
	ak_print_notice_ex("m_SampleRate=%ld, m_Channels=%d, m_BitsPerSample=%d\n", 
    	decode->output.m_SampleRate, decode->output.m_Channels, 
    	decode->output.m_BitsPerSample);
    
	/**
	 * set decode attribute
	 * _SD_BM_NORMAL means decode rigth now when getting data
	 */
    _SD_SetBufferMode(decode->lib_handle, _SD_BM_NORMAL);

    decode->out_buf = (char *)calloc(1, AUDIO_DECODE_MAX_SIZE);
    if(NULL == decode->out_buf) {
        ak_print_error_ex("calloc audio decode out buffer failed\n");
        goto open_end;
    }

	decode->channel_num = param->channel_num;
	decode->mode = ADEC_MODE_SINGLE_FRAME;
    ak_thread_mutex_init(&(decode->mutex));
	ret = AK_SUCCESS;

open_end:
	if(AK_FAILED == ret) {
		if(decode) {
			if(decode->lib_handle) {
				_SD_Buffer_Clear(decode->lib_handle);
				_SD_Decode_Close(decode->lib_handle);
			}

			free(decode);
	        decode = NULL;
		}
	}

    return decode;
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

	if(ms < 0) {
		/* free space is not enough in block mode */
		while(free_size < stream_len) {
			if (free_size > 0) {
				write_decode_buffer(dec_handle, &(data[offset]), free_size);
				offset += free_size;
				stream_len -= free_size;
			}
			
			ak_sleep_ms(10);
			free_size = ak_adec_get_free_size(dec_handle);
		}

		write_decode_buffer(dec_handle, &(data[offset]), stream_len);
		offset += stream_len;
	} else {
		/* free space is not enough in non-block mode */
	    if(free_size < stream_len){
	    	ak_sleep_ms(10);
 			return offset;
	    }

	    /* we don't support timeout mode, so we do it as non-block mode */
	    write_decode_buffer(dec_handle, data, stream_len);
	    offset += stream_len;
	}

	struct audio_decode *decode = (struct audio_decode *)dec_handle;

	if(dec_ctrl.read_flag 
		&& (offset > 0) 
		&& (ADEC_MODE_STREAM == decode->mode)){
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

	ak_thread_mutex_lock(&(decode->mutex));
	/* compute the buffer address and length */
	decode->output.m_pBuffer = (unsigned char *)decode->out_buf;
	decode->output.m_ulSize = AUDIO_DECODE_MAX_SIZE;

	int ret = _SD_Decode(decode->lib_handle, &(decode->output));
    if(ret > 0){
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
    }
    ak_thread_mutex_unlock(&(decode->mutex));

    return ret;
}

/**
 * ak_adec_notice_stream_end - notice send stream end
 * @dec_handle[in]: opened decode handle 
 * return: 0 success; -1 failed
 */
int ak_adec_notice_stream_end(void *dec_handle)
{
	if (!dec_handle) {
		return AK_FAILED;
	}
	
	struct audio_decode *decode = (struct audio_decode *)dec_handle;
	
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
    	free(decode->out_buf);
		decode->out_buf = NULL;
    }

    free(decode);
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
	if (NULL == ao_handle){
		ak_print_error_ex("please open audio output first\n");
		return NULL;
	}
	if (NULL == dec_handle){
		ak_print_error_ex("please open audio decode first\n");
		return NULL;
	}

	int ret = AK_FAILED;
    struct audio_decode_user *user = (struct audio_decode_user *)calloc(1,
    	sizeof(struct audio_decode_user));
    if(NULL == user){
    	goto request_end;
    }

    user->ao_handle = ao_handle;
    user->dec_handle = dec_handle;
    user->pos = NULL;
    user->status = STREAM_STATUS_RESERVED;

	/* set decode stream mode */
	struct audio_decode *decode = (struct audio_decode *)dec_handle;
	decode->mode = ADEC_MODE_STREAM;
	
	if(!dec_ctrl.run_flag){
		dec_ctrl.run_flag = AK_TRUE;
		dec_ctrl.read_flag = AK_TRUE;
		if (AK_FAILED == start_decode_ctrl(user)) {
			dec_ctrl.run_flag = AK_FALSE;
			goto request_end;
		}
	}
    ret = AK_SUCCESS;

request_end:
	if(AK_FAILED == ret) {
		if(NULL != user) {
			free(user);
			user = NULL;
		}
	}

	return user;
}

/**
 * ak_adec_get_stream_status - get current stream decode status
 * @stream_handle[in]: opened stream handle
 * return: current stream decode status
 */
enum adec_stream_status ak_adec_get_stream_status(void *stream_handle)
{
	if(NULL == stream_handle){
        return STREAM_STATUS_RESERVED;
    }

	struct audio_decode_user *user = (struct audio_decode_user *)stream_handle;
	return user->status;
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
        return AK_FAILED;
    }

    if(NULL == stream_handle){
        return AK_FAILED;
    }

	struct audio_decode_user *user = (struct audio_decode_user *)stream_handle;
	struct audio_decode *decode = (struct audio_decode *)(user->dec_handle);

	if(NULL == decode){
        return AK_FAILED;
    }
    
#if ADEC_DATA_DEBUG
	if (adec_fp) {
		fclose(adec_fp);
		adec_fp = NULL;
	}
#endif

	if (AO_PLAY_STATUS_PLAYING == ak_ao_get_play_status(user->ao_handle)) {
		while (AO_PLAY_STATUS_FINISHED != ak_ao_get_play_status(user->ao_handle)) {
			ak_sleep_ms(100);
		}

		user->status = STREAM_STATUS_PLAY_FINISHED;
		ak_sleep_ms(10);
	}
	
	/* reset ao play status */
	ak_ao_set_play_status(user->ao_handle, AO_PLAY_STATUS_READY);
	
	dec_ctrl.run_flag = AK_FALSE;
	dec_ctrl.read_flag = AK_FALSE;
	ak_thread_sem_post(&(dec_ctrl.data_sem));
	
	ak_print_notice_ex("ready to join write_pcm_thread\n");
	ak_thread_join(dec_ctrl.pcm_tid);
	ak_print_notice_ex("write_pcm_thread join OK\n");
	
	ak_thread_sem_destroy(&(dec_ctrl.data_sem));
    ak_thread_mutex_destroy(&(decode->mutex));
    ak_print_info_ex("leave...\n");

    return AK_SUCCESS;
}
