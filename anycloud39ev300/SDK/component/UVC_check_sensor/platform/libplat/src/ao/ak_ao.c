#include "ak_ao.h"
#include "drv_api.h"
#include "ak_common.h"
#include "dev_drv.h"
#include "kernel.h"

#define LIBPLAT_AO_VERSION "libplat_ao_1.3.02"

typedef enum
{
    AO_NORMAL = 0,           ///normal
    AO_PAUSE,               ///pause
    AO_MUTE,                ///mute
    AO_PAUSE_AND_MUTE       ///pause and mute
    
}AO_STATUS;

typedef struct 
{
	int fd;
	
}T_HAL_SPEAKER;

struct AO_STRUCT
{
    T_SOUND_DRV *dac_drv;  // dac drv opreation handle
    SOUND_INFO info;        ///
    int frame_size;     ///
    int volume;         ///
    T_HAL_SPEAKER m_hal_speanker;
    AO_STATUS status;  ///mute ; pause ;normal ;
};
extern bool aec_open_flag ;
extern bool aec_flag ;//aec flag，use for dac drive selecting which way to calculate the div
static struct AO_STRUCT * g_ao_handle = NULL;

static void dac_send_callback( void )
{
  	ak_print_normal("CB\n");
}

static bool ao_SetDABuf_size(void *ao_handle)
{
    if(ao_handle != g_ao_handle)
    {
        return false;
    }

    struct AO_STRUCT *aohandle = (struct AO_STRUCT *)ao_handle;

    //calculate one buf size according to playing time: 30ms
    //At present the smallest unit DA can play is 512B,
    //so the results are added or reduced to divide 512 exactly.
    switch(aohandle->info.nSampleRate)
    {
    case 8000:
        aohandle->frame_size = 1024;
        break;
        
    case 11025:
        aohandle->frame_size = 1536;
        break;
        
    case 16000:
        aohandle->frame_size = 2048;
        break;
        
    case 22050:
        aohandle->frame_size = 2560;
        break;
        
    case 24000:
        aohandle->frame_size = 3072;
        break;
        
    case 32000:
        aohandle->frame_size = 4096;            
        break;
        
    case 44100:
        aohandle->frame_size = 5120;
        break;
        
    case 48000:
        aohandle->frame_size = 5632;
        break;

    default:
        aohandle->frame_size = (unsigned short)(aohandle->info.nSampleRate
                                     * (aohandle->info.BitsPerSample>>3)
                                     * 2        //channel equal to 2 even mono
                                     * 3/100);  //30ms/1000ms
                                     
        //should divide exactly 512 at present temporarily
        aohandle->frame_size = (aohandle->frame_size>>9)<<9;
        break;
    }
    
    if (0 == aohandle->frame_size)
    {
        ak_print_error("sampleRate = %d, sampleBits = %d, waveout open error for OneBufSize = 0!\n",aohandle->info.nSampleRate, aohandle->info.BitsPerSample);
        return false;
    }

    return true;
}

const char* ak_ao_get_version(void)
{
    return LIBPLAT_AO_VERSION;
}


/**
 * @brief  open a audio out device  and Set sound sample rate, channel, bits per sample of the sound device
 * @author JianKui
 * @date   2016-11-24
 * @param [in] param:struct include sample rate, channel, bits per sample
 * @return void *
 * @retval DAC device handle
 */
 #define save_pam 0
 #if save_pam
 FILE *pcm;
 #endif
void* ak_ao_open(const struct pcm_param *param)
{
	#if save_pam
	pcm = fopen("a:/ao.pcm","w+");
	#endif
    //SOUND_INFO info;
    if(NULL != g_ao_handle)
    {
        ak_print_error("ao alread open!\n");
        return NULL;
    }
    g_ao_handle = (struct AO_STRUCT *)malloc(sizeof(struct AO_STRUCT));
	if(NULL == g_ao_handle)
	{
		ak_print_error("ao handle malloc false!\n");
		return NULL;
	}
    g_ao_handle->info.BitsPerSample = param->sample_bits;
    g_ao_handle->info.nChannel = param->channel_num;
    g_ao_handle->info.nSampleRate = param->sample_rate;
    g_ao_handle->volume = 1;
    g_ao_handle->status = AO_NORMAL;

    if(false == ao_SetDABuf_size(g_ao_handle))
    {
        goto ao_end;
    }
    //malloc buff,set callback
    g_ao_handle->dac_drv = sound_create (SOUND_DAC, g_ao_handle->frame_size, 30, NULL);
    if(NULL == g_ao_handle->dac_drv)
    {
        ak_print_error("dac drv create false!\n");
        goto ao_end;
    }

    //clear all buff
    sound_cleanbuf (g_ao_handle->dac_drv);

    if(false == sound_open (g_ao_handle->dac_drv))
    {
        ak_print_error(" open DAC false!\n");
        goto dac_create_end;
    }
    //connect DAC and headphone,control power
    analog_setsignal(INPUT_DAC, OUTPUT_HP, SIGNAL_CONNECT); 

	//set  aec_flag  
	sound_setinfo_select(g_ao_handle->dac_drv, aec_flag);
	
    //set sound sample rate, channel, bits per sample
    sound_setinfo(g_ao_handle->dac_drv, &g_ao_handle->info);

    //set HP gain
    analog_setgain_hp(g_ao_handle->volume);

    return g_ao_handle;

dac_create_end:
    sound_delete(g_ao_handle->dac_drv);
ao_end:
    free(g_ao_handle);
    g_ao_handle = NULL;
    return NULL;
}


/**
 * @brief set frame size function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @param[in] fram_size: every buffer fram size
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_ao_set_frame_size(void *ao_handle, int frame_size)
{

    if(ao_handle != g_ao_handle)
    {
        return AK_FAILED;
    }

    struct AO_STRUCT *aohandle = (struct AO_STRUCT *)ao_handle;

    if(false == sound_realloc(aohandle->dac_drv, frame_size, 30, NULL))
    {
        ak_print_error("set frame size realloc error!\n");
        return AK_FAILED;
    }

    aohandle->frame_size = frame_size;

    return AK_SUCCESS;
}

/**
 * @brief get frame size function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @param[out] fram_size:read size store address
 * @return int
 * @retval -1 operation failed
 * @retval >=0 frame size
 */
int ak_ao_get_frame_size(void *ao_handle)
{
	if(ao_handle != g_ao_handle)
	{	
		return AK_FAILED;
	}
	struct AO_STRUCT *aohandle = (struct AO_STRUCT *)ao_handle;

	return aohandle->frame_size;
}


/**
 * @brief send frame size data to DAC function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @param[in] fram_ptr: send  data's pointer
 * @param[in] len:send data's length.  len must loss than dac_frame_size
 * @param[in] ms : the longst wait time,  ms<0: until charge, ms = 0: no wait ,ms>0: wait no than ms . unit :ms
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_ao_send_frame(void *ao_handle, unsigned char *data, int len, long ms)
{
	
	bool ret;
	void * sound_buf;
	unsigned long sound_len, frame_len;
	int send_size = 0, i;
	unsigned char *stereobuf = NULL;
	unsigned short *monobuf = NULL;    //for data double
	unsigned short *pcmbuf = NULL;
	if(ao_handle != g_ao_handle)
	{	
		return AK_FAILED;
	}
	if(NULL == data)
	{
		return AK_FAILED;
	}
	struct AO_STRUCT *aohandle = (struct AO_STRUCT *)ao_handle;
	
	sound_judge_aec (aohandle->dac_drv,aec_open_flag);
	
		
	if(aohandle->info.nChannel == 1)
	{	
		monobuf = (unsigned short *)malloc(len);
		if(NULL == monobuf)
		{
			ak_print_error("ao malloc monobuf false!\n");
			return -1;
		}
		
		memcpy(monobuf, data, len);
		
		stereobuf = (unsigned char *)malloc(2*len);
		if(NULL == stereobuf)
		{
			ak_print_error("ao malloc stereobuf false!\n");
			return -1;
		}
		/*mono data transform stereo  data*/
		pcmbuf = (unsigned short *)stereobuf;
		for(i = 0;i < len/2; i++)
		{
			*(pcmbuf + 2*i) = *(monobuf + i);
			*(pcmbuf + 2*i +1) = *(monobuf + i);
		}
		frame_len = len * 2;
	}
	else
	{	
		stereobuf = (unsigned char *)malloc(len);
		if(NULL == stereobuf)
		{
			ak_print_error("ao malloc stereobuf false!\n");
			return -1;
		}
		memcpy(stereobuf, data, len);
		frame_len = len;
	}
	//将stop标识置1，若置1，将不能往pcmbuffer send 数据。
	if(AO_PAUSE== aohandle->status)
	{
		return -1;
	}
	
	#if save_pam
	fwrite(stereobuf,frame_len,1,pcm);
	#endif
	if(ms == 0)  //非阻塞
	{
		while(frame_len>0)
		{
			//1,get PCM buffer
			ret = sound_getbuf (aohandle->dac_drv, &sound_buf, &sound_len);
			if (ret)
			{
				if(frame_len <= sound_len)
				{
					//2,copy data
					memcpy(sound_buf, stereobuf + send_size, frame_len);
					//3,send data to PCM buffer
					sound_endbuf(aohandle->dac_drv, frame_len);	
					send_size += frame_len;
					frame_len = 0;
				}
				else
				{
					//2,copy data
					memcpy(sound_buf, stereobuf + send_size, sound_len);
					//3,send data to PCM buffer
					sound_endbuf(aohandle->dac_drv, sound_len);	
					send_size += sound_len;
					frame_len -= sound_len;
				}
			}
			else
			{
				break;
			}
		}
		if(NULL != monobuf)
		{		
			free(monobuf);
			monobuf = NULL;
		}
		if(NULL != stereobuf)
		{		
			free(stereobuf);
			stereobuf = NULL;
		}
		
		if(aohandle->info.nChannel == 1)
			return (send_size/2);
		else			
			return (send_size);
	}
	else if(0 < ms)// 阻塞 ms
	{
		while(ms)
		{
			//1,get PCM buffer
			ret = sound_getbuf (aohandle->dac_drv, &sound_buf, &sound_len);
			if (ret && sound_len >= frame_len)
			{
				//2,copy data
				memcpy(sound_buf, stereobuf, frame_len );
				//3,send data to PCM buffer
				sound_endbuf(aohandle->dac_drv, frame_len);	
				if(NULL != monobuf)
				{		
					free(monobuf);
					monobuf = NULL;
				}
				if(NULL != stereobuf)
				{		
					free(stereobuf);
					stereobuf = NULL;
				}
				
				return 0;
			}
			else
			{	
				if(10 < ms)
				{
					mini_delay(10);
					ms = ms - 10;
				}
				else
				{
					mini_delay(ms);
					ms = 0;
				}
				
			}
		}
		if(0 == ms)
		{
			//1,get PCM buffer
			ret = sound_getbuf (aohandle->dac_drv, &sound_buf, &sound_len);
			if (ret && sound_len >= frame_len)
			{
				//2,copy data
				memcpy(sound_buf, stereobuf, frame_len );
				//3,send data to PCM buffer
				sound_endbuf(aohandle->dac_drv, frame_len);	
				if(NULL != monobuf)
				{		
					free(monobuf);
					monobuf = NULL;
				}
				if(NULL != stereobuf)
				{		
					free(stereobuf);
					stereobuf = NULL;
				}
				return 0;
			}
			if(NULL != monobuf)
			{		
				free(monobuf);
				monobuf = NULL;
			}
			if(NULL != stereobuf)
			{		
				free(stereobuf);
				stereobuf = NULL;
			}
			return -1;
		}
		if(NULL != monobuf)
		{		
			free(monobuf);
			monobuf = NULL;
		}
		if(NULL != stereobuf)
		{		
			free(stereobuf);
			stereobuf = NULL;
		}
		return -1;
	}
	else  //一直阻塞
	{
		send_size = 0;
		while(frame_len>0)
		{
			//1,get PCM buffer
			ret = sound_getbuf (aohandle->dac_drv, &sound_buf, &sound_len);
			if(ret)
			{
				if(sound_len >= frame_len)
				{
					//2,copy data
					memcpy(sound_buf, stereobuf+send_size, frame_len );
					//3,send data to PCM buffer
					sound_endbuf(aohandle->dac_drv, frame_len);	
					frame_len -=frame_len;
					send_size += frame_len;
					
				}
				else
				{
					//2,copy data
					memcpy(sound_buf, stereobuf+send_size, sound_len );
					//3,send data to PCM buffer
					sound_endbuf(aohandle->dac_drv, sound_len);	
					frame_len -=sound_len;
					send_size += sound_len;
				}
			}
			else
			{
				ak_sleep_ms(5);
			}
		}
		if(NULL != monobuf)
		{		
			free(monobuf);
			monobuf = NULL;
		}
		if(NULL != stereobuf)
		{		
			free(stereobuf);
			stereobuf = NULL;
		}
		if(aohandle->info.nChannel == 1)
			return (send_size/2);
		else			
			return (send_size);
	}
}


/**
 * @brief clear all buffer frame data function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_ao_clear_frame_buffer(void *ao_handle)
{
	if(ao_handle != g_ao_handle)
	{	
		return -1;
	}
	struct AO_STRUCT *aohandle = (struct AO_STRUCT *)ao_handle;
	sound_cleanbuf (aohandle->dac_drv);
	return 0;
}
/**
 * ak_ao_get_play_status - get audio play status
 * @ao_handle[in]: opened audio output handle
 * return: current audio play status
 * notes:
 */
enum ao_play_status ak_ao_get_play_status(void *ao_handle)
{
	unsigned char free_buf;
	if(ao_handle != g_ao_handle)
	{	
		return AK_FAILED;
	}
	struct AO_STRUCT *aohandle = (struct AO_STRUCT *)ao_handle;
	free_buf = sound_getnum_fullbuf(aohandle->dac_drv);
	if(0 == free_buf)
	{
		return AO_PLAY_STATUS_FINISHED;
	}
	else
	{
		return AO_PLAY_STATUS_PLAYING;
	}
}

/**
 * ak_ao_set_play_status - set audio play status
 * @ao_handle[in]: opened audio output handle
 * @status[in]: new status
 * return: 0 success; -1 failed
 * notes:
 */
int ak_ao_set_play_status(void *ao_handle, enum ao_play_status status)
{
	return 0;
}

/**
 * @brief set dac voice gain  function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @param[in] volume : 0:mute 1:lowest  , 6:biggest
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_ao_set_volume(void *ao_handle, int volume)
{
	if(ao_handle != g_ao_handle)
	{	
		return -1;
	}
	struct AO_STRUCT *aohandle = (struct AO_STRUCT *)ao_handle;

	aohandle->volume = volume;
	if(0 == volume)  //set mute
	{
		analog_setmute_hp(1);
		if(AO_NORMAL == aohandle->status)
		{
			aohandle->status = AO_MUTE;
		}
		else if(AO_PAUSE == aohandle->status)
		{
			aohandle->status = AO_PAUSE_AND_MUTE;
		}
	}
	else if(volume > 0 && volume < 7)
	{
		if((AO_MUTE == aohandle->status)||(AO_PAUSE_AND_MUTE == aohandle->status))
		{
			analog_setmute_hp(0);
			if(AO_MUTE == aohandle->status)
				aohandle->status = AO_NORMAL;
			else
				aohandle->status = AO_PAUSE;
		}
		//set HP gain
		analog_setgain_hp(volume-1);
	}
	else
	{
		return -1;
	}
	
	return 0;
}

/**
 * @brief speaker  device control
 * @param [in] ao_handle: audio out device handle
 * @param[in] enable : 1:open, 0:close
 * @return int
 * @retval 0:success
 * @retval -1:fail
  */
int ak_ao_enable_speaker(void *ao_handle, int enable)
{
	if(ao_handle != g_ao_handle)
	{	
		return -1;
	}
	struct AO_STRUCT *aohandle = (struct AO_STRUCT *)ao_handle;
	
	if(enable)
	{
		aohandle->m_hal_speanker.fd = dev_open(DEV_SPEAKER);
		if(aohandle->m_hal_speanker.fd > 0)
		{
			return 0;
		}
		return -1;
	}
	else
	{
		dev_close(aohandle->m_hal_speanker.fd);
		aohandle->m_hal_speanker.fd = 0;
		return 0;
	}
}

/**
 * @brief audio out stop  function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_ao_pause(void *ao_handle)
{
	if(ao_handle != g_ao_handle)
	{	
		return -1;
	}
	struct AO_STRUCT *aohandle = (struct AO_STRUCT *)ao_handle;

	//将sound send 关闭，不往DAC发送数据
	if(false == sound_pause(aohandle->dac_drv))
	{
		return -1;
	}
	
	//将stop标识置1，若置1，将不能往pcmbuffer send 数据。
	aohandle->status= AO_PAUSE;
	
	return 0;
}

/**
 * @brief audio out resume  function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_ao_resume(void *ao_handle)
{
	if(ao_handle != g_ao_handle)
	{	
		return -1;
	}
	struct AO_STRUCT *aohandle = (struct AO_STRUCT *)ao_handle;

	//打开PCM发送数据，继续传输数据
	if(false == sound_resume(aohandle->dac_drv))
	{
		return -1;
	}
	aohandle->status= AO_NORMAL;

	
	return 0;
}

/**
 * @brief close audio out device
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_ao_close(void *ao_handle)
{
	if(ao_handle != g_ao_handle)
	{	
		return -1;
	}
	struct AO_STRUCT *aohandle = (struct AO_STRUCT *)ao_handle;
	//clear all buffer
	sound_cleanbuf (aohandle->dac_drv);
	//disconnect HP and dac,power off 
    analog_setsignal(INPUT_DAC, OUTPUT_HP, SIGNAL_DISCONNECT);
	//close dac controller
    sound_close(aohandle->dac_drv);
	//delete audio out
    sound_delete(aohandle->dac_drv);
	
	#if save_pam
	fclose(pcm);
	#endif
	free(aohandle);
	//aohandle = NULL;
	g_ao_handle = NULL;
	
	return 0;
}



