#include <stdio.h>
#include <stdlib.h>
#include "ak_common.h"
#include "sdcodec.h"
#include "ak_login.h"

static unsigned char audio_g711_decode_cb_delay(unsigned long ticks)
{
	ak_sleep_ms(ticks);	
	return 1;
}
/**
 * @brief  login  g711  decode  type
 * @author Pyy
 * @date   2017-05-31
 * @return 
 * @retval 
 */
void ak_login_g711_decode(void)
{
	T_AUDIO_LOG_INPUT plogInput;
	plogInput.cb.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc;
	plogInput.cb.Free = (MEDIALIB_CALLBACK_FUN_FREE)free;
	plogInput.cb.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)printf;
	plogInput.cb.delay = audio_g711_decode_cb_delay;
		
	plogInput.m_Type = _SD_MEDIA_TYPE_PCM_ALAW;
	_SD_G711_login(&plogInput);
	
	plogInput.m_Type = _SD_MEDIA_TYPE_PCM_ULAW;
	_SD_G711_login(&plogInput);

}