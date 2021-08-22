#include "ak_login.h"
#include "ak_common.h"
#include "sdfilter.h"
#include "kernel.h"

/**
 * @brief  login  all  sdfilter
 * @author Pyy
 * @date   2017-05-31
 * @return 
 * @retval 
 */
void ak_login_all_filter(void)
{
	T_AUDIO_FILTER_LOG_INPUT vplogInput;
	vplogInput.cb.Malloc = (MEDIALIB_CALLBACK_FUN_MALLOC)malloc_cb;
	vplogInput.cb.Free = (MEDIALIB_CALLBACK_FUN_FREE)free_cb;
	vplogInput.cb.printf = (MEDIALIB_CALLBACK_FUN_PRINTF)print_normal;	
		
	vplogInput.m_Type = _SD_FILTER_RESAMPLE;			
	_SD_Resample_login(&vplogInput);
	vplogInput.m_Type = _SD_FILTER_AGC;	
	_SD_AGC_login(&vplogInput);
}
