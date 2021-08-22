#include "drv_api.h"
#include "command.h"
#include "hal_sound.h"
#include "pcm.h"
#include "ak_ai.h"
#include "ak_common.h"
#include "sdfilter.h"
#include "echo_interface.h"
#include "ak_thread.h"
#include "kernel.h"


#define LIBPLAT_AI_VERSION "libplat_ai_1.1.00"
#define AUDIO_DATA_LEN  (512)// malloc L2 buffer size,回音消除为了时序的误差小，所以l2buf要尽量小
#define DEFAULT_ADC_PERIOD_BYTES		(AUDIO_DATA_LEN)
#define ADC_SDFILTER_BUFFER_LENGTH 		(DEFAULT_ADC_PERIOD_BYTES * 2)
/* we'll drop the first 512 ms audio data */
#define ADC_CAP_DROP_TIME 				512
#define BUFNUM 90 
#define ADC_BUFFER_LENGTH 				(8192)//cycbuf  size
#define AEC_FRAME_LEN  (128)//aec frame size
#define AEC_FRAME_SIZE   (256)
#define AEC_BUF_SIZE  (1024 * 2)
#define AEC_BUF_COUNT (32)





enum switch_type{
	SWITCH_DISABLE,
	SWITCH_ENABLE
};

enum sdfilter_aec_type{
	resample_sdfilter_open,//open sdfilter_aec_type
	nr_agc_sdfilter_open,
	aec_echo_open
};


typedef struct pcm_buf {
	unsigned char data[ADC_BUFFER_LENGTH];
	int remain;//remain data index
	int head;
	int tail;
	bool in_out_flag;//in order to judge cycbuf full or empty
	bool new_data_flag;// 为了减少时间戳的误差，当从l2读取新邋数据回来，则把当前帧时间戳等于l2数据的时间
} pcm_cycbuf;

typedef struct pcm_sdfilter_aec {
	
	unsigned int actual_rate;
	void *resample_sdfilter;
	void *nr_agc_sdfilter;
	void *aec_echo;
	enum switch_type resample_switch;
	enum switch_type nr_agc_switch;
	enum switch_type aec_switch;
} pcm_sdfilter_aec_info;


typedef struct
{
	unsigned long pre_ts;		//previous audio frame timestamp
	unsigned long cur_ts;		//current timestamp 
	int drop_time;				//drop the first audio data of the appointed time
	int frame_size;
	unsigned char *frame_data;

}PCM_ADC_INFO;

struct aec_frame {
	unsigned char * buf_near;
	unsigned char * buf_far;
	unsigned char * buf_out;
	unsigned int buf_len;
};

typedef struct
{	
	unsigned char   buf[AEC_BUF_SIZE];//AEC_ONEBUF_SIZE
	unsigned long   ts;       ///timestamp(ms)
	unsigned long   len;      ///data_len
}aecout_buf;

typedef struct
{
	aecout_buf  aecbuf[AEC_BUF_COUNT];//AEC_BUF_NUM
	unsigned char readIndex;   ///< write index
    unsigned char writeIndx;   ///< read index  
}aecout_buf_info;
static aecout_buf_info gAecSpkBuf = {0};



volatile unsigned long  adc_callback_flg = 0;
static bool  first_get_adcbuf = false;//first get data from l2buf flag
static PCM_ADC_INFO enco_buf = {0};//use to get the data from cycbuf to encoded
static pcm_cycbuf cycbuf = {0};//use to save the data get from drive
static pcm_sdfilter_aec_info sdfilter_aec = {0};//config sdfilter_aec parame
static bool open_flag = false;// open flag
volatile unsigned long first_get_flag = 0;//为了回音消除获取数据能够时序对应上
bool aec_flag = false;//aec flag，use for dac drive selecting which way to calculate the div
bool aec_open_flag = false;
static unsigned long aec_ts = 0;
ak_pthread_t  aec_id;





const char* ak_ai_get_version(void)
{
	return LIBPLAT_AI_VERSION;
}

void adc_recv_callback( void )
{
	adc_callback_flg ++;
	
}
static void  AecNotify(unsigned long event)
{
	
}

/**
 * @brief  aec_open
 * @author Panyuyi
 * @date   2017-06-19
 * @param[in] 
 * @param[in] channels		data channels
 * @param[in] samplerate	  the samplerate
 * @param[in] 
 * @param[in] 
 * @return 
 * @retval	
 * @retval	
 */

static  void* aec_open(unsigned int channels, unsigned int samplerate)
{
	T_AEC_INPUT  aecInput;	
	memset(&aecInput, 0 ,sizeof(aecInput));
	aecInput.cb_fun.Free = (AEC_CALLBACK_FUN_FREE)free_cb;
	aecInput.cb_fun.Malloc = (AEC_CALLBACK_FUN_MALLOC)malloc_cb;
	aecInput.cb_fun.printf = (AEC_CALLBACK_FUN_PRINTF)print_normal;
	aecInput.cb_fun.notify = AecNotify;

	aecInput.m_info.strVersion =ECHO_LIB_VERSION_STRING;
	aecInput.m_info.chip = ECHO_CHIP_AK39XXEV2_RTOS;
	aecInput.m_info.m_Type =AEC_TYPE_1;
	aecInput.m_info.m_BitsPerSample =16;
	aecInput.m_info.m_Channels=channels;
	aecInput.m_info.m_SampleRate =samplerate;
	
	aecInput.m_info.m_Private.m_aec.m_aecEna = 1;
	aecInput.m_info.m_Private.m_aec.m_PreprocessEna = 1;
	aecInput.m_info.m_Private.m_aec.m_framelen =AEC_FRAME_LEN;
	aecInput.m_info.m_Private.m_aec.m_tail = AEC_FRAME_LEN * 8;
	aecInput.m_info.m_Private.m_aec.m_agcEna =1;
	aecInput.m_info.m_Private.m_aec.m_agcLevel = 32768*75/100;//75%
	aecInput.m_info.m_Private.m_aec.m_maxGain=4;
		
	void * aechandle = AECLib_Open(&aecInput);

	if (aechandle ==NULL )
	{
		ak_print_error("AECLib_Open failed\n");
		return NULL;
	}
	else
	{
		ak_print_normal("AECLib_Open ok!\n");
	
	}	
	return aechandle;		
	
}
/**
  * @brief  aecr_close
  * @author Panyuyi
  * @date	2017-06-19
  * @param[in] handler handler of aec open
  * @param[in] 
  * @param[in] 
  * @return int
  * @retval  AK_TRUE :	success
  * @retval  AK_FLASE :  false
  */

static int aec_close(void * handler)
{
	 if (NULL == handler)
		 return AK_FAILED;
	 return AECLib_Close(handler);	 
}
/**
 * @brief  sdfilter_open
 * @author Panyuyi
 * @date   2016-12-23
 * @param[in] actual_sr   the actual_sr
 * @param[in] channels		data channels
 * @param[in] samplerate	  the samplerate
 * @param[in] open_type 	 open type
 * @param[in] 
 * @return 
 * @retval	
 * @retval	
 */

static inline void* sdfilter_open(unsigned int actual_sr, 
	unsigned int channels, unsigned int samplerate,unsigned int open_type)
{
	if(resample_sdfilter_open == open_type)
	{
		T_AUDIO_FILTER_INPUT s_ininfo_resample;
		memset(&s_ininfo_resample, 0, sizeof(T_AUDIO_FILTER_INPUT));

		s_ininfo_resample.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc_cb;
		s_ininfo_resample.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)free_cb;
		s_ininfo_resample.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;
		
		s_ininfo_resample.m_info.m_BitsPerSample = 16;
		s_ininfo_resample.m_info.m_Channels = channels;
		s_ininfo_resample.m_info.m_SampleRate = actual_sr;
		

		s_ininfo_resample.m_info.m_Type = _SD_FILTER_RESAMPLE;
		//配0的话，库内部默认设置为512,在调用control的时候，一次输入的数据长度不要超过512
		s_ininfo_resample.m_info.m_Private.m_resample.maxinputlen = AUDIO_DATA_LEN;
		s_ininfo_resample.m_info.m_Private.m_resample.outSrindex = 0; 
		s_ininfo_resample.m_info.m_Private.m_resample.outSrFree = samplerate;
		s_ininfo_resample.m_info.m_Private.m_resample.reSampleArithmetic = RESAMPLE_ARITHMETIC_1; 
		s_ininfo_resample.chip = AUDIOLIB_CHIP_AK39XXEV2;
		
		void *handler = _SD_Filter_Open(&s_ininfo_resample);
		if ( AK_NULL == handler) 
		{
			ak_print_error( "can't open the resample_sdfilter filter!\n" );
			return NULL;
		} 
		else 
		{
			ak_print_normal("ok open the resample_sdfilter filter: %p!\n", handler);
		}
		return handler;
	}
	else
	{
		T_AUDIO_FILTER_INPUT s_ininfo_nr_agc;
		memset(&s_ininfo_nr_agc, 0, sizeof(T_AUDIO_FILTER_INPUT));

		s_ininfo_nr_agc.cb_fun.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc_cb;
		s_ininfo_nr_agc.cb_fun.Free = (MEDIALIB_CALLBACK_FUN_FREE)free_cb;
		s_ininfo_nr_agc.cb_fun.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;
		
		s_ininfo_nr_agc.m_info.m_BitsPerSample = 16;
		s_ininfo_nr_agc.m_info.m_Channels = channels;
		s_ininfo_nr_agc.m_info.m_SampleRate = actual_sr;
		
		s_ininfo_nr_agc.m_info.m_Type = _SD_FILTER_AGC;
		s_ininfo_nr_agc.m_info.m_Private.m_agc.AGClevel = 32767/2; 
		s_ininfo_nr_agc.m_info.m_Private.m_agc.noiseReduceDis = 0; 
		s_ininfo_nr_agc.m_info.m_Private.m_agc.agcDis = 0;
		s_ininfo_nr_agc.m_info.m_Private.m_agc.agcPostEna = 0; 
		s_ininfo_nr_agc.m_info.m_Private.m_agc.nr_range = 100;
		s_ininfo_nr_agc.chip = AUDIOLIB_CHIP_AK39XXEV2;
		
		void *handler = _SD_Filter_Open(&s_ininfo_nr_agc);
		if ( AK_NULL == handler) 
		{
			ak_print_error( "can't open the nr_agc_sdfilter filter!\n" );
			return NULL;
		} 
		else 
		{
			ak_print_normal("ok open the nr_agc_sdfilter filter: %p!\n", handler);
		}
		return handler;
	}	

	
}		

 /**
  * @brief	sdfilter_control
  * @author Panyuyi
  * @date	2016-12-23
  * @param[in] handler	 handler of sdfilter open
  * @param[in] inbuf	  the data input
  * @param[in] inlen	  the size of input data
  * @param[in] outbuf	   the data output
  * @param[in] outlen	   the size of output data
  * @return int
  * @retval  return  the datalen which has been handle
  * @retval  
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

 /**
  * @brief	sdfilter_close
  * @author Panyuyi
  * @date	2016-12-23
  * @param[in] handler handler of sdfilter open
  * @param[in] 
  * @param[in] 
  * @return int
  * @retval  AK_TRUE :	success
  * @retval  AK_FLASE :  false
  */
 
 static inline int sdfilter_close(void *handler)
 {
	 if (NULL == handler)
		 return AK_FAILED;
	 return _SD_Filter_Close(handler);
 }

 /**
 * @brief  calc_pcm_current_time
 * @author Panyuyi
 * @date   2016-12-23
 * @param[in] handler handler of the sound device
 * @param[in] pre_ts   previous audio frame timestamp
 * @param[in] frame_len    size of frame
 * @return int
 * @retval  current audio frame timestamp
 * @retval
 */
static inline int calc_frame_ts(void *handle, unsigned long pre_ts, int frame_len)
{
	if(NULL == handle)
	{
	    return -1;
	}
	pcm_user_t *user = (pcm_user_t *)handle;
	unsigned long cur_ts = pre_ts;
	int sample_bytes = (user->param.sample_bits >> 3);

    cur_ts += ((1000 * frame_len) / (user->param.rate * sample_bytes));

	return cur_ts;
}



/**
 * @brief  calc_pcm_data_len - calculate cycbuf data remain len
 * @author Panyuyi
 * @date   2016-12-23
 * @param[in] pcm_buf  pcm_buf info
 * @param[in] 
 * @return int
 * @retval  len
 * @retval
 */

static int get_cycbuf_len(const pcm_cycbuf *pcm_buf)
{
	int len = 0;

	if ((pcm_buf->tail == pcm_buf->head) && (pcm_buf->in_out_flag == false))//empty
		len = 0;
	else
	{
		if ((pcm_buf->tail == pcm_buf->head) && (pcm_buf->in_out_flag == true))//full
		{
			len = sizeof(pcm_buf->data);
		}
		else
		{
			if(pcm_buf->tail > pcm_buf->head)
				len = pcm_buf->tail - pcm_buf->head;
			else
				len = sizeof(pcm_buf->data) - (pcm_buf->head - pcm_buf->tail);
		}
	}

	return len;
}

/**
 * @brief  get_data from cycbuf
 * @author Panyuyi
 * @date   2016-12-23
 * @param[in] pcm_buf    pcm_buf info
 * @param[in] to      get the data 
 * @param[in] size    the size of data neet to get 
 * @return int
 * @retval  len
 * @retval
 */

static int get_data_from_cycbuf(pcm_cycbuf *pcm_buf, unsigned char *to, int size)
{
	int tmp = 0;
	int buf_len = sizeof(pcm_buf->data);

	if (buf_len - pcm_buf->head >= size) {
		memcpy(to, &(pcm_buf->data[pcm_buf->head]), size);
		pcm_buf->head += size;
	} else {
		tmp = buf_len - pcm_buf->head;
		memcpy(to, &(pcm_buf->data[pcm_buf->head]), tmp);
		memcpy(&to[tmp], pcm_buf->data, (size - tmp));
		pcm_buf->head = size - tmp;
	}

	pcm_buf->head %= buf_len;
	pcm_buf->in_out_flag = false;
	return 0;
}

/**
 * @brief  put_data  to  cycbuf
 * @author Panyuyi
 * @date   2016-12-23
 * @param[in] from    databuffer
 * @param[in] pcm_buf     storage  the data 
 * @param[in] size    the size of data neet to storage 
 * @return int
 * @retval  len
 * @retval
 */

static int put_data_into_cycbuf(unsigned char *from, int size, pcm_cycbuf *pcm_buf)
{
	int tmp = 0;
	int buf_len = sizeof(pcm_buf->data);

	if (buf_len - pcm_buf->tail >= size) {
		memcpy(&(pcm_buf->data[pcm_buf->tail]), from, size);
		pcm_buf->tail += size;
	} else {
		tmp = buf_len - pcm_buf->tail;
		memcpy(&(pcm_buf->data[pcm_buf->tail]), from, tmp);
		memcpy(&(pcm_buf->data), &from[tmp], (size - tmp));
		pcm_buf->tail = size - tmp;
	}

	pcm_buf->tail %= buf_len;
	pcm_buf->in_out_flag = true;
	pcm_buf->new_data_flag = true;

	return 0;
}
/**
 * @brief  set_sdfilter_aec_open
 * @author Panyuyi
 * @date   2016-12-23
 * @param[in] handle   handler of the sound device
 * @param[in] info      pcm_sdfilter_aec_info  info
 * @param[in] 
 * @param[in] open_type      open type
 * @param[in] 
 * @return  int
 * @retval   0  success
 * @retval   -1  faile	
 */


static int set_sdfilter_aec_open(void *handle,pcm_sdfilter_aec_info *info,unsigned int open_type)
{
	if(NULL == handle)
	{
	    return -1;
	}
	pcm_user_t *user = (pcm_user_t *)handle;
	if ((NULL == info->nr_agc_sdfilter) &&(open_type == nr_agc_sdfilter_open))
	{
		info->nr_agc_sdfilter = sdfilter_open(info->actual_rate, 1, user->param.rate,open_type);
		if (NULL == info->nr_agc_sdfilter) 
		{
			ak_print_error("open nr_agc_sdfilter failed!!!!\n");
			return -1;
		}
		
	}
	else if ((NULL == info->resample_sdfilter) && (open_type == resample_sdfilter_open))
	{
		info->resample_sdfilter = sdfilter_open(info->actual_rate, 1, user->param.rate,open_type);
		if (NULL == info->resample_sdfilter) 
		{
			ak_print_error("open resample_sdfilter failed!!!!\n");
			return -1;
		}
		
	}
	else if((NULL == info->aec_echo) && (open_type == aec_echo_open))
	{
		info->aec_echo = aec_open(1, user->param.rate);
		if (NULL == info->aec_echo) 
		{
			ak_print_error("open aec failed!!!!\n");
			return -1;
		}
	}
	return 0;
}


static int aec_get_data(void *handle)
{
	int ret = -1;
	if(NULL == handle)
	{
		return -1;
	}
	unsigned long windx;
	pcm_user_t *user = (pcm_user_t *)handle;
	windx = gAecSpkBuf.writeIndx;
	if(sound_get_aecbuf(user->adc_in,gAecSpkBuf.aecbuf[windx].buf,&(gAecSpkBuf.aecbuf[windx].ts),&(gAecSpkBuf.aecbuf[windx].len)))
	{
		
		//ak_print_normal("0\n");
		
		//ak_print_normal("ts:%d\n",gAecSpkBuf.aecbuf[gAecSpkBuf.writeIndx].ts);
		/*
		只有是获取到有效的数据，下标才做偏移
		*/
		if((gAecSpkBuf.aecbuf[windx].ts > 0) && (gAecSpkBuf.aecbuf[windx].ts > aec_ts))
		{
		
		//ak_print_normal("1\n");
			aec_ts = gAecSpkBuf.aecbuf[windx].ts;		
			gAecSpkBuf.writeIndx = (gAecSpkBuf.writeIndx + 1) % AEC_BUF_COUNT;
			ret = 0;;
		}
	}
	
	return ret;
}


/**
 * @brief  read_pcm_data from L2buffer
 * @author Panyuyi
 * @date   2016-12-23
 * @param[in] handle   handler of the sound device
 * @param[in] to     storage cycbuf data
 * @param[in] 
 * @param[in] 
 * @param[in] 
 * @return  int
 * @retval   get data length. if return -1 failed, otherwise success.
 * @retval   	
 */

static int read_pcm_from_l2(void *handle,struct frame *to)
{
	bool ret ;
	int len = -1;

	struct data_tmp 
	{
		unsigned long len;    //buf len in bytes
		unsigned long ts;	 //timestamp(ms)
    };
	unsigned long tmp;

	struct data_tmp *data_buf = NULL;
	
	if(NULL == handle)
	{
		 return len;
	}
	pcm_user_t *user = (pcm_user_t *)handle;
	
	if(!first_get_adcbuf)
	{
		first_get_adcbuf = true;
		ret = sound_getbuf (user->adc_in, (void*)&(to->data), &tmp);
	}
	while(1)
	{
		if(adc_callback_flg)
		{
			adc_callback_flg--;
			ret = sound_getbuf (user->adc_in, (void*)&(to->data), &tmp);

			if(ret)
			{
				//aec_get_data(handle);
				data_buf = (struct data_tmp *)tmp;
				to->len = data_buf->len;		
				to->ts= data_buf->ts;			
				sound_endbuf(user->adc_in, to->len);
				len = to->len;
				goto read_end;
			}
		}
		else
		{
			ak_sleep_ms(5);
		}
	}
	read_end:
		return len;
}

	
 /**
 * @brief  read from the ADC device
 * @author Panyuyi
 * @date   2016-12-23
 * @param[in] handle   handler of the sound device
 * @param[in] to     storage cycbuf data
 * @param[in] size    read length
 * @param[in] 
 * @param[in] 
 * @return  int
 * @retval   read data length. if return -1 failed, otherwise success.
 * @retval   
 */
 static int sdfilter_handle(void *handle, unsigned char *to)
 {
	int ret = 0;
	int sdflen = 0;
	unsigned char *sdfbuf = NULL;
	int offset = 0;
	struct frame read_frame ;
	
	sdfbuf = malloc(ADC_SDFILTER_BUFFER_LENGTH);
	if(NULL == sdfbuf)
	{
		ak_print_error_ex("malloc sdfbuf error\n");
		return -1;
	}

	if ((SWITCH_ENABLE == sdfilter_aec.nr_agc_switch) && (NULL != sdfilter_aec.nr_agc_sdfilter))
	{
		sdflen = read_pcm_from_l2(handle, &read_frame);
		enco_buf.cur_ts = read_frame.ts;
		if (sdflen < 0) 
		{
			ak_print_error_ex("read error\n");
			enco_buf.cur_ts = 0;
			free(sdfbuf);
			sdfbuf = NULL;
			return -1;
		}
		
		if ((SWITCH_ENABLE == sdfilter_aec.resample_switch) && (NULL != sdfilter_aec.resample_sdfilter)) 
		{			
			ret = sdfilter_control(sdfilter_aec.nr_agc_sdfilter, read_frame.data, sdflen,
			sdfbuf, ADC_SDFILTER_BUFFER_LENGTH);			
		} 
		else
		{
			ret = sdfilter_control(sdfilter_aec.nr_agc_sdfilter, read_frame.data, sdflen,
			&to[offset], ADC_SDFILTER_BUFFER_LENGTH);
			goto handle_end;
		}
	}
	
	if ((SWITCH_ENABLE == sdfilter_aec.resample_switch) && (NULL != sdfilter_aec.resample_sdfilter)) 
	{
		if ((SWITCH_ENABLE == sdfilter_aec.nr_agc_switch) && (NULL != sdfilter_aec.nr_agc_sdfilter))
		{		
			ret = sdfilter_control(sdfilter_aec.resample_sdfilter, sdfbuf, ret,
			&to[offset], ADC_SDFILTER_BUFFER_LENGTH);			
		}
		else
		{
			sdflen = read_pcm_from_l2(handle, &read_frame);
			enco_buf.cur_ts = read_frame.ts;
			if (sdflen < 0) 
			{
				ak_print_error_ex("read error\n");
				enco_buf.cur_ts = 0;
				free(sdfbuf);
				sdfbuf = NULL;
				return -1;
			}
			else
			{
				ret = sdfilter_control(sdfilter_aec.resample_sdfilter, read_frame.data, sdflen,
				&to[offset], ADC_SDFILTER_BUFFER_LENGTH);
			}
		}
		goto handle_end;
	}
	else 
	{
		ret = read_pcm_from_l2(handle, &read_frame);
		enco_buf.cur_ts = read_frame.ts;
		if (ret < 0) 
		{
			enco_buf.cur_ts = 0;
			free(sdfbuf);
			sdfbuf = NULL;
			return -1;
		}
		
		memcpy(to,read_frame.data,ret);
		to += ret;
	}
	
handle_end:		
	offset += ret;	
	free(sdfbuf);
	sdfbuf = NULL;	
	return offset;
}




  /**
 * @brief  read from the global variable cycbuf.if no data ,then read from the AD device
 * @author Panyuyi
 * @date   2016-12-23
 * @param[in] handle   handler of the sound device
 * @param[in] data      storage  data get from cycbuf
 * @param[in] len        read length
 * @param[in] 
 * @param[in] 
 * @return  int
 * @retval   read data length. if return -1 failed
 * @retval   
 */
 int get_encoded_data(void *handle, unsigned char *data, unsigned int len)
 {
	if ((NULL == handle) || (NULL == data)) 
	{		
		return -1;
	}

	int ret = -1;
	int remain = 0;
	unsigned char *cache = NULL;	
	pcm_user_t *user = (pcm_user_t *)handle;
	while (1) 
	{
	#if 0
		if(first_get_flag == 1)//此处判断主要是为了回音消除的pcm数据时序可以对应上
		{
			//先清空cycbuf 的数据
			cycbuf.head = 0;			
			cycbuf.tail = 0;
			cycbuf.remain = 0;
			cycbuf.in_out_flag = false;
			cycbuf.new_data_flag = false;
			cycbuf.data[ADC_BUFFER_LENGTH] = 0;
			goto get_pcm_from_L2;
		}
	#endif
		remain = get_cycbuf_len(&cycbuf);
		if(remain >= len) 
		{
			get_data_from_cycbuf(&cycbuf, data, len);
			
			cycbuf.remain = remain - len;
			ret = len;
			
			goto get_end;
		}
get_pcm_from_L2:		
		cache = malloc(ADC_SDFILTER_BUFFER_LENGTH);
		if (!cache) 
		{
			ak_print_error_ex("malloc failed\n");
			ret = -1;
			goto get_end;
		}
		
		ret = sdfilter_handle(handle, cache);
		
		if (ret < 0) 
		{
			free(cache);
			ret = -1;
			
			enco_buf.cur_ts = 0;
			goto get_end;
		}
		if(enco_buf.drop_time < ADC_CAP_DROP_TIME) //abandon the first audio data of the appointed time
		{
			enco_buf.drop_time += user->l2_buf_interval;		
			ak_sleep_ms(5);
		}
		else
		{
		
			put_data_into_cycbuf(cache, ret, &cycbuf);
			//first_get_flag ++;
		}
			
		free(cache);
	}

get_end:
	return ret;
}

/**
 * @brief  init_sdfilter_aec_info
 * @author Panyuyi
 * @date   2016-12-23
 * @param[in] info   sound info
 * @return 
 * @retval	
 */

static void init_sdfilter_aec_info(SOUND_INFO *info)
{
	
	sdfilter_aec.actual_rate = info->nSampleRate;
	sdfilter_aec.resample_sdfilter = NULL;
	sdfilter_aec.resample_switch = SWITCH_DISABLE;
	sdfilter_aec.nr_agc_sdfilter = NULL;
	sdfilter_aec.nr_agc_switch = SWITCH_DISABLE;
	sdfilter_aec.aec_echo = NULL;
	sdfilter_aec.aec_switch = SWITCH_DISABLE;
}

/**
 * @brief  parameter_init
 * @author Panyuyi
 * @date   2016-12-23
 * @param[in] 
 * @return 
 * @retval	
 */

static void ai_parameter_init(void)
{
	int i;
	//init_enco_buf_info
	enco_buf.pre_ts = 0;
	enco_buf.cur_ts = 0;
	enco_buf.drop_time = 0;
	enco_buf.frame_size = 0;
	enco_buf.frame_data = NULL;
	//init_aec_buf_info	
	gAecSpkBuf.readIndex = 0;
	gAecSpkBuf.writeIndx = 0;
	for(i = 0;i <AEC_BUF_COUNT;i++)
	{		
		memset(gAecSpkBuf.aecbuf[i].buf, 0, AEC_BUF_SIZE);
		gAecSpkBuf.aecbuf[i].ts = 0;
		gAecSpkBuf.aecbuf[i].len = 0;
	}
}


/**
 * @brief  open a sound device and it can be used
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] param   sound info
 * @return void*
 * @retval  the sound device
 */
 #define save_sd 0
 #if save_sd
 FILE *near,*far,*aec_out;
 unsigned char *n=NULL,*f=NULL,*out=NULL;
 unsigned long inde = 0;
 #endif
void* ak_ai_open(const struct pcm_param *param)
{
	if(open_flag == true)
	{
		ak_print_normal("have been opened!\n" );
		return ;
	}	
	aec_flag = true;//aec flag，use for dac drive selecting which way to calculate the div
	
#if save_sd
	near = fopen("a:/near.pcm","w+");
	
	far = fopen("a:/far.pcm","w+");
	aec_out = fopen("a:/out.pcm","w+");
	
	unsigned long data_size = (8012 * 1  * 2 ) * (30000/ 1000);
	n = (unsigned char *)malloc(data_size+1);
	f = (unsigned char *)malloc(data_size+1);;
	out = (unsigned char *)malloc(data_size+1);
#endif
	
	SOUND_INFO info;
	pcm_user_t *ai_handle = NULL;
	ai_handle = malloc(sizeof(pcm_user_t));
	if(NULL == ai_handle)
		return NULL;
	ai_handle->param.channels = param->channel_num;
	ai_handle->param.sample_bits = param->sample_bits;
	ai_handle->param.rate = param->sample_rate;
	ai_handle->source = AI_SOURCE_AUTO;
	ai_handle->interval = AMR_FRAME_INTERVAL;	
	int sample_bytes = (ai_handle->param.sample_bits >> 3);
	
	ai_handle->l2_buf_interval = ((1000 * AUDIO_DATA_LEN) / (ai_handle->param.rate * sample_bytes));
	switch(param->sample_rate)
	{
		case 8000:		
			ai_handle->adc_in = sound_create (SOUND_ADC, AUDIO_DATA_LEN, BUFNUM, adc_recv_callback);//create sound
			break;
		default:		
			ai_handle->adc_in = sound_create (SOUND_ADC, (AUDIO_DATA_LEN*2), BUFNUM, adc_recv_callback);//create sound
			break;
	}
	
	sound_cleanbuf (ai_handle->adc_in);
	if(!sound_open (ai_handle->adc_in))
	{
		free(ai_handle);
		return NULL;
	}

	if(!analog_setsignal(INPUT_MIC, OUTPUT_ADC, SIGNAL_CONNECT)) //default  INPUT_MIC
	{
		free(ai_handle);
		return NULL;
	}
	info.nSampleRate = param->sample_rate;
	//adc config samplerate
    if(!sound_setinfo (ai_handle->adc_in, &info))
    {
		free(ai_handle);
		return NULL;
    }
	
	open_flag = true;
	//ai_parameter_init
	ai_parameter_init();
	//init_sdfilter_aec_info
	init_sdfilter_aec_info(&info);
	if(((set_sdfilter_aec_open(ai_handle,&sdfilter_aec,nr_agc_sdfilter_open)) < 0)|| 
		((set_sdfilter_aec_open(ai_handle,&sdfilter_aec,resample_sdfilter_open)) < 0)||
		((set_sdfilter_aec_open(ai_handle,&sdfilter_aec,aec_echo_open)) < 0))
	{	
		ak_ai_close(ai_handle);
		return NULL;
	}
    return ai_handle;
		
}


/**
 * @brief  set  source  signal  connect
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device
 * @param[in] input  source
 * @return int
  * @retval 0  set successful
  * @retval -1 set failed

 */

int ak_ai_set_source(void *handle, enum ai_source src)
{
	if(NULL == handle)
	{
	    return -1;
	}
	pcm_user_t *user = (pcm_user_t *)handle;
	if(AI_SOURCE_LINEIN == src)
	{
		user->source = AI_SOURCE_LINEIN;
		analog_setsignal(INPUT_MIC, OUTPUT_ADC, SIGNAL_DISCONNECT);
		if(!analog_setsignal(INPUT_LINEIN, OUTPUT_ADC, SIGNAL_CONNECT))
			return 0;
		else
			return -1;
	}
	else
	{
		return 0;
	}

}

/**
 * @brief  set sound driver frame size
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device
 * @param[in] frame_size  the buffer of sound device
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */
 
int ak_ai_set_frame_interval(void *handle, int frame_interval)

{
	int ret = 0;
	unsigned int sample_bytes, frame_size = 0;
	if(NULL == handle)
    {
        return -1;
    }
	pcm_user_t *user = (pcm_user_t *)handle;
	user->interval = frame_interval;
	sample_bytes = (user->param.sample_bits / 8);
	frame_size = (user->param.rate * user->param.channels
			* frame_interval * sample_bytes ) / 1000;
	if(frame_size & 1)
	{
		frame_size++;
	}
	
	if(frame_size > ADC_BUFFER_LENGTH)
	{
		ak_print_error_ex("Input frame_size is too large,the frame_size must less than %dk\n",ADC_BUFFER_LENGTH);
		return -1;
	}
	if(NULL != enco_buf.frame_data) 
	{
		free(enco_buf.frame_data);
		enco_buf.frame_data = NULL;
	}

	enco_buf.frame_data = calloc(1, frame_size);
	if(NULL == enco_buf.frame_data) 
	{
		ret = -1;
	} 
	else 
	{
		enco_buf.frame_size = frame_size;		
	}	
	return ret;
	
}
 
/**
 * @brief  clear sound driver frame buffer
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device
 * @param[in] frame_size  the buffer of sound device
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */
int ak_ai_clear_frame_buffer(void *handle)
{
	if(NULL == handle)
	{
	     return -1;
	}
	cycbuf.head = 0;
	cycbuf.tail = 0;
	cycbuf.remain = 0;
	cycbuf.in_out_flag = false;
	cycbuf.new_data_flag = false;
	cycbuf.data[ADC_BUFFER_LENGTH] = 0;
	memset(enco_buf.frame_data, 0x00, enco_buf.frame_size);
	
	return 0;
}
/**
 * @brief  set sound driver gain
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device
 * @param[in] volume  the gain of sound device
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */

int ak_ai_set_volume(void *handle, int volume)
{
	if(NULL == handle)
	{
	    return -1;
	}
	pcm_user_t *user = (pcm_user_t *)handle;
	if(AI_SOURCE_LINEIN == user->source)
	{
		if(analog_setgain_linein(volume))
			return 0;
		else
			return -1;
	}
	else
	{
		if(analog_setgain_mic(volume))
			return 0;
		else
			return -1;
	}

}



static void *aec_handle_thread(void *handle)
{	
	while(sdfilter_aec.aec_switch)
	{
		aec_get_data(handle);
		ak_sleep_ms(10);
	}	
	ak_thread_exit();	
	
}

/**
 * @brief  send pcm data to aec lib
 * @author Panyuyi
 * @date   2017-06-19
 * @param[in] handler handler of the sound device  
 * @param[in] aec_frame  pcm data for aec lib
 * @param
 * @return int
 * @retval 0  get buffer successful
 * @retval -1 get buffer failed
 */

static int aec_control(void*handle, struct aec_frame*aec_frame)
{
   if(NULL == handle)
   {
	   return -1;
   }
   
   if ((SWITCH_ENABLE == sdfilter_aec.aec_switch) && (NULL != sdfilter_aec.aec_echo))
   {   
	   T_AEC_BUF   aecBufInfo = {0};
	   //send  data to aec_lib	deal with	   
	   aecBufInfo.buf_near = aec_frame->buf_near;
	   #if save_sd
	   if(n!=NULL)
	   	{
	   		memcpy(&n[inde],aec_frame->buf_near,256);
	   	}
	   #endif
	   aecBufInfo.buf_far  = aec_frame->buf_far;
	   #if save_sd
	   if(f!=NULL)
	   	{
	   		memcpy(&f[inde],aec_frame->buf_far,256);
	   	}
	   #endif
	   aecBufInfo.buf_out  = aec_frame->buf_out;
	   aec_frame->buf_len = AECLib_Control(sdfilter_aec.aec_echo, &aecBufInfo);	
	   #if save_sd
	   if(out!=NULL)
		{
		   memcpy(&out[inde],aecBufInfo.buf_out,256);
		   inde+=256;
		}
	   #endif
   }

   if(aec_frame->buf_len > 0)
	   return 0;
   else 
	   return -1;
}



/**
 * @brief  get buffer address and buffer len, which can be used to fill or retrieve sound data
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device  
 * @param[out] frame return buffer address or AK_NULL
 * @param[out] ms
 * @return int
 * @retval 0  get buffer successful
 * @retval -1 get buffer failed
 */
 #if save_sd
 unsigned long ts = 0;
 bool time_flag = false,close_flag = false;
 #endif
int ak_ai_get_frame(void *handle, struct frame *frame, long ms)
{
	
	int ret,i,aec_ret = -1,len = 0;	
	struct aec_frame aec_info = {0};
	unsigned long aec_indx = 0;	
	unsigned char aecOutBuf[MIC_ONEBUF_SIZE] ;
	unsigned char *aecBuf = NULL;
	unsigned char rindx;
	if(NULL == handle)
	{
		return -1;
	}
#if save_sd
	if(!time_flag)
	{
		ts = get_tick_count();
		time_flag = true;

	}
#endif
	len = get_encoded_data(handle, enco_buf.frame_data, enco_buf.frame_size);
	
	if(len > 0) 
	{	
		
    	if(cycbuf.new_data_flag)// 为了减少时间戳的误差，当从l2读取新邋数据回来，则把当前帧时间戳等于l2数据的时间戳
		{
    		frame->ts = enco_buf.cur_ts;
			cycbuf.new_data_flag = false;
    	} 
		else 
		{
    		frame->ts = calc_frame_ts(handle, enco_buf.pre_ts, len);
    		
    	}
		enco_buf.pre_ts = frame->ts;
		
		if((sdfilter_aec.aec_switch == SWITCH_ENABLE))	
		{	
			rindx = gAecSpkBuf.readIndex;
		
		//ak_print_normal("f = %d,A = %d,r = %d\n",frame->ts,gAecSpkBuf.aecbuf[rindx].ts,rindx);
			int aecDoNum = len / AEC_FRAME_SIZE;
			aecBuf = (unsigned char *)malloc(len);	
			if(NULL == aecBuf)
			{		
				ak_print_error_ex("malloc fail\n");	
				return -1;
			}
			
			if(gAecSpkBuf.aecbuf[rindx].ts == frame->ts)
			{
			
			//ak_print_normal("2\n");
				for(i=0 ;i< aecDoNum ;i++)	
				{
					//send  data to aec_lib  deal with		
					aec_info.buf_near = enco_buf.frame_data + i * AEC_FRAME_SIZE;
					aec_info.buf_far  = gAecSpkBuf.aecbuf[rindx].buf + i * AEC_FRAME_SIZE;
					aec_info.buf_out  = aecOutBuf;
					
					if (aec_control(handle, &aec_info) == 0)
					{
						memcpy(&aecBuf[aec_indx], aecOutBuf, aec_info.buf_len);
						aec_indx += aec_info.buf_len;			
					}
					else
					{
						ak_print_error("AECLib_Control failed!\n");
						return -1;
					}
				}
				gAecSpkBuf.readIndex = (gAecSpkBuf.readIndex + 1) % AEC_BUF_COUNT;
			}
			else if((gAecSpkBuf.aecbuf[rindx].ts < frame->ts))
			{				
				while(1)
				{		
				
				//ak_print_normal("3\n");
					
					if((gAecSpkBuf.aecbuf[rindx].ts == frame->ts) || (gAecSpkBuf.readIndex == gAecSpkBuf.writeIndx))
					{
						
						//ak_print_normal("4\n");
						break;
					}	
					
					gAecSpkBuf.readIndex = (gAecSpkBuf.readIndex + 1) % AEC_BUF_COUNT;
				}
				
				if(gAecSpkBuf.aecbuf[rindx].ts == frame->ts)
				{
				
				//ak_print_normal("5\n");
					for(i=0 ;i< aecDoNum ;i++)	
					{
						//send  data to aec_lib  deal with		
						aec_info.buf_near = enco_buf.frame_data + i * AEC_FRAME_SIZE;
						aec_info.buf_far  = gAecSpkBuf.aecbuf[rindx].buf + i * AEC_FRAME_SIZE;
						aec_info.buf_out  = aecOutBuf;
						
						if (aec_control(handle, &aec_info) == 0)
						{
							memcpy(&aecBuf[aec_indx], aecOutBuf, aec_info.buf_len);
							aec_indx += aec_info.buf_len;			
						}
						else
						{
							ak_print_error("AECLib_Control failed!\n");
							return -1;
						}
					}
					gAecSpkBuf.readIndex = (gAecSpkBuf.readIndex + 1) % AEC_BUF_COUNT;
				}
				else 
				{	
				
				//ak_print_normal("6\n");
					
					memcpy(aecBuf, enco_buf.frame_data, len);
					aec_indx = len;
				}				
			}
			
			
#if save_sd
			if((get_tick_count()) - ts >= 30000 && (!close_flag))
			{
				close_flag = true;
				fwrite(n,inde,1,near);				
				fwrite(f,inde,1,far);				
				fwrite(out,inde,1,aec_out);
				fclose(near);	
				fclose(far);
				fclose(aec_out);
				free(n);
				n=NULL;
				free(f);
				f=NULL;
				free(out);
				out=NULL;
				ak_print_error_ex("#########ai close##########\n"); 				
			}
#endif
			
			if(!(frame->data))
			{
				frame->data = calloc(1,aec_indx);
			}
			else if(NULL == frame->data)
			{			
				ak_print_error_ex("frame->data malloc fail\n");	
				return -1;
			}
			memcpy(frame->data, aecBuf, aec_indx);
			frame->len = aec_indx;
			ret = 0;
			if(NULL != aecBuf)
			{		
				free(aecBuf);
				aecBuf = NULL;
			}
		}

		else if(sdfilter_aec.aec_switch == SWITCH_DISABLE)
		{	
			//ak_print_error_ex("Aec = %d,frame = %d\n",gAecSpkBuf.aecbuf[rindx].ts,frame->ts); 
			if(!(frame->data))
			{
				frame->data = calloc(1,len);
			}
			else if(NULL == frame->data)
			{			
				ak_print_error_ex("frame->data malloc fail\n");	
				return -1;
			}
			memcpy(frame->data, enco_buf.frame_data, len);
			frame->len = len;
			ret = 0;
		}
				
    }
	else
	{
		ret = -1;
	}
	return ret;
}
/**
 * @brief  delete sound driver and Free sound buffer
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */
int ak_ai_release_frame(void *handle, struct frame *frame)
{
	if((NULL == handle) || (NULL == frame))
	{	
	    return -1;
	}
	if(frame->data) 
	{		
		free(frame->data);
		frame->data = NULL;
		frame->len = 0;
		frame->ts = 0;		
	}
	return 0;
}
/**
 * @brief  close a sound device 
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */
int ak_ai_close(void *handle)
{
	int i;
	if(NULL == handle)
	{
	    return -1;
	}
	pcm_user_t *user = (pcm_user_t *)handle;
	if(AI_SOURCE_LINEIN == user->source)
	{
		analog_setsignal(INPUT_LINEIN, OUTPUT_ADC, SIGNAL_DISCONNECT);
	}
	else
	{
		analog_setsignal(INPUT_MIC, OUTPUT_ADC, SIGNAL_DISCONNECT);
	}
    sound_close(user->adc_in);
	sound_delete(user->adc_in);	
	if (NULL != sdfilter_aec.resample_sdfilter) 
	{
		sdfilter_close(sdfilter_aec.resample_sdfilter);
		sdfilter_aec.resample_sdfilter = NULL;	
		sdfilter_aec.resample_switch = SWITCH_DISABLE;
	}
	
	if(NULL != sdfilter_aec.nr_agc_sdfilter)
	{
		sdfilter_close(sdfilter_aec.nr_agc_sdfilter);
		sdfilter_aec.nr_agc_sdfilter = NULL;		
		sdfilter_aec.nr_agc_switch = SWITCH_DISABLE;
	}
	
	if(NULL != sdfilter_aec.aec_echo)
	{
		aec_close(sdfilter_aec.aec_echo);
		sdfilter_aec.aec_echo = NULL;		
		sdfilter_aec.aec_switch = SWITCH_DISABLE;
	}
	if(NULL != enco_buf.frame_data) 
	{
		free(enco_buf.frame_data);
		enco_buf.frame_data = NULL;
	}
	if(aec_open_flag)
	{
		ak_thread_join(aec_id);
	}
	free(user);
	user = NULL;
	
	gAecSpkBuf.readIndex = 0;
	gAecSpkBuf.writeIndx = 0;
	for(i = 0;i <AEC_BUF_COUNT;i++)
	{		
		memset(gAecSpkBuf.aecbuf[i].buf, 0, AEC_BUF_SIZE);
		gAecSpkBuf.aecbuf[i].ts = 0;
		gAecSpkBuf.aecbuf[i].len = 0;
	}
	first_get_adcbuf = false;//clean the flag
	aec_ts = 0;
	open_flag = false;//clean open flag
	//first_get_flag = 0;	
	aec_flag = false;	
	aec_open_flag = false;
	return 0;
}

/**
 * @brief  set audio resampling
 * @author Panyuyi
 * @date    2017-01-10
 * @param[in] handler handler of the sound device
 * @param[in] enable  0 close, 1 open
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */

int ak_ai_set_resample(void *handle, int enable)
{
	if(enable)
		sdfilter_aec.resample_switch = SWITCH_ENABLE;
	else
		sdfilter_aec.resample_switch = SWITCH_DISABLE;
	return 0;
}

/**
 * @brief  set audio agc
 * @author Panyuyi
 * @date   2017-01-10
 * @param[in] handler handler of the sound device
 * @param[in] enable  0 close, 1 open
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */

int ak_ai_set_nr_agc(void *handle, int enable)
{
	if(enable)
		sdfilter_aec.nr_agc_switch = SWITCH_ENABLE;
	else
		sdfilter_aec.nr_agc_switch = SWITCH_DISABLE;
	return 0;
}

/**
 * @brief  set audio aec
 * @author Panyuyi
 * @date   2017-06-19
 * @param[in] handler handler of the sound device
 * @param[in] enable  0 close, 1 open
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */

int ak_ai_set_aec(void *handle, int enable)
{
	int ret;
	pcm_user_t *user = (pcm_user_t *)handle;
	if(enable)
	{
		aec_open_flag = true;
		sdfilter_aec.aec_switch= SWITCH_ENABLE;	
		ak_ai_set_resample(handle, SWITCH_DISABLE);
		/*
		AEC_FRAME_SIZE  is  256，in order to prevent droping data,
		it is needed  (frame_size/AEC_FRAME_SIZE = 0), at  the same 
		time, it  need  think of DAC  L2buf  , so  frame_interval  should  
		be  set  32ms。
	*/
		//ak_ai_set_frame_interval(handle, 32);
		
		ret = ak_thread_create(&aec_id, aec_handle_thread,
			user, ANYKA_THREAD_MIN_STACK_SIZE, 90);
	}
	else
	{
		aec_open_flag = false;
		sdfilter_aec.aec_switch = SWITCH_DISABLE;
	}
	
	sound_judge_aec (user->adc_in,aec_open_flag);
	return 0;

}


