
#ifndef __AK_AO_H__
#define __AK_AO_H__
#include "ak_global.h"



enum ao_play_status {
	AO_PLAY_STATUS_RESERVED = 0x00,
	AO_PLAY_STATUS_READY,
	AO_PLAY_STATUS_PLAYING,
	AO_PLAY_STATUS_FINISHED
};

/*
* @brief  get audio out version
*/
const char* ak_ao_get_version(void);


/**
 * @brief  open a audio out device  and Set sound sample rate, channel, bits per sample of the sound device
 * @author JianKui
 * @date   2016-11-24
 * @param [in] param:struct include sample rate, channel, bits per sample
 * @return void *
 * @retval DAC device handle
 */
void* ak_ao_open(const struct pcm_param *param);


/**
 * @brief get frame size function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @return int
 * @retval -1 operation failed
 * @retval >=0 frame size
 */
int ak_ao_get_frame_size(void *ao_handle);

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
int ak_ao_set_frame_size(void *ao_handle, int frame_size);

/**
 * @brief send frame size data to DAC function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @param[in] fram_ptr: send  data's pointer
 * @param[in] ms : the longst wait time,  ms<0: until charge, ms = 0: no wait ,ms>0: wait no than ms . unit :ms
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_ao_send_frame(void *ao_handle, unsigned char *data, int len, long ms);

/**
 * @brief clear all buffer frame data function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_ao_clear_frame_buffer(void *ao_handle);

/**
 * ak_ao_get_play_status - get audio play status
 * @ao_handle[in]: opened audio output handle
 * return: current audio play status
 * notes:
 */
enum ao_play_status ak_ao_get_play_status(void *ao_handle);

/**
 * ak_ao_set_play_status - set audio play status
 * @ao_handle[in]: opened audio output handle
 * @status[in]: new status
 * return: 0 success; -1 failed
 * notes:
 */
int ak_ao_set_play_status(void *ao_handle, enum ao_play_status status);

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
int ak_ao_set_volume(void *ao_handle, int volume);


/**
 * @brief speaker  device control
 * @param [in] ao_handle: audio out device handle
 * @param[in] enable : 1:open, 0:close
 * @return int
 * @retval 0:success
 * @retval -1:fail
  */
int ak_ao_enable_speaker(void *ao_handle, int enable);

/**
 * @brief audio out stop  function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
//int ak_ao_pause(void *ao_handle);

/**
 * @brief audio out resume  function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
//int ak_ao_resume(void *ao_handle);

/**
 * @brief close audio out device
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle: audio out device handle
 * @return int
 * @retval -1 operation failed
 * @retval 0 operation successful
 */
int ak_ao_close(void *ao_handle);


#endif




