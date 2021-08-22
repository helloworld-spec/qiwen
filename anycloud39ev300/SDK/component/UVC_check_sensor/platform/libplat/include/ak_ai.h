
#ifndef         __AK_AI_H
#define         __AK_AI_H

#include "ak_global.h"


#define AUDIO_DEFAULT_INTERVAL		100	//100ms one frame
#define AMR_FRAME_INTERVAL		    20	//AMR frame interval 20ms

enum ai_source {
	AI_SOURCE_AUTO,		//pcm driver decide adc source automatically,
						//and it is default configuration in pcm driver
	AI_SOURCE_MIC,		//set adc mic source manually
	AI_SOURCE_LINEIN	//set adc linein source manually
};
const char* ak_ai_get_version(void);


/**
 * @brief  get buffer address and buffer len, which can be used to fill or retrieve sound data
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device  
 * @param[in] ai_frame return buffer address or AK_NULL
 * @param[in] ms
 * @return int
 * @retval 0  get buffer successful
 * @retval -1 get buffer failed
 */


int ak_ai_get_frame(void *handle, struct frame *frame, long ms);
/**
 * @brief  open a sound device and it can be used
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] input  sound info
 * @return void*
 * @retval  the sound device
 */


void* ak_ai_open(const struct pcm_param *param);


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

int ak_ai_set_source(void *handle, enum ai_source src);

/**
 * @brief  delete sound driver and Free sound buffer
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */

int ak_ai_release_frame(void *handle, struct frame *frame);
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


int ak_ai_set_frame_interval(void *handle, int frame_interval);
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

int ak_ai_clear_frame_buffer(void *handle);
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



int ak_ai_set_volume(void *handle, int volume);
/**
 * @brief  close a sound device 
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */


int ak_ai_close(void *handle);
/**
 * @brief  set audio resampling
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device
 * @param[in] enable  0 close, 1 open
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */

int ak_ai_set_resample(void *handle, int enable);

/**
 * @brief  set audio agc
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device
 * @param[in] enable  0 close, 1 open
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */

int ak_ai_set_nr_agc(void *handle, int enable);
/**
 * @brief  set audio aec
 * @author Panyuyi
 * @date   2016-11-23
 * @param[in] handler handler of the sound device
 * @param[in] enable  0 close, 1 open
 * @return int
 * @retval 0  set successful
 * @retval -1 set failed
 */

int ak_ai_set_aec(void *handle, int enable);
#endif
