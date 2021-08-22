#include "anyka_types.h"
#include "ak_drv_key.h"
#include "drv_keypad.h"
#include "dev_info.h"
#include "ak_common.h"
#include "dev_drv.h"

 typedef struct
{
	int fd;
}T_HAL_KEY;
 
static  T_HAL_KEY m_hal_key = {0};





/**
 * @brief   set gpio share pin as gpio,input dir,interrupt mode
 * @author  Panyuyi
 * @date    2016-11-23
 * @param[in]  
 * @param[in]  
 * @return  void *
 * @retval  NULL created failed
 */

void* ak_drv_key_open(void)
{
	m_hal_key.fd = dev_open(DEV_KEYPAD);
	
	if((m_hal_key.fd > 0) )
	{
		return &m_hal_key.fd;				
	}	
	else
	{
		return NULL;
	}
	
}

/**
 * @brief   register callback function, Get gpio input level
 * @author  Panyuyi
 * @date    2016-11-23
 * @param[in]  handler handler of the key
 * @param[in]  event     refer to key_event
 * @param[in]  ms
 * @return int
 * @retval  <0 failed
 * @retval  0 successful
 */

int ak_drv_key_get_event(void *handle, struct key_event *event, long ms)
{

	volatile long t_ms = ms;
	if(NULL == handle)
	{
		ak_print_error("open fail!\n");
		return -1;
	}
	T_KEYPAD key = {0};
	if(0 == ms)//·Ç×èÈû
	{

		if(0 == dev_read(m_hal_key.fd, &key, 0))
		{
			event->code = key.keyID;
			event->stat = key.status;
			return 0;
		}
		else
		{
			return -1;
		}
		
	}
	else if(0 < t_ms)// ×èÈû ms
	{
		while(t_ms)
		{
			if(0 == dev_read(m_hal_key.fd, &key, 0))
			{
				event->code = key.keyID;
				event->stat = key.status;
				return 0;
			}
			else
			{	
				if(500 < t_ms)
				{
					mini_delay(500);
					t_ms = t_ms - 500;
					
				}
				else
				{
					mini_delay(t_ms);
					t_ms = 0;					
					return -1;
				}
								
			}

		}
		
	}
	else//Ò»Ö±×èÈû
	{
		while(1)
		{
			if(0 == dev_read(m_hal_key.fd, &key, 0))
			{
				event->code = key.keyID;
				event->stat = key.status;
				return 0;
			}
			else
			{
				mini_delay(50);
				
			}
				
		}

	}

}


/**
 * @brief   disable gpio pin interrupt
 * @author  Panyuyi
 * @date    2016-11-23
 * @param[in]  handler handler of the key
 * @return int
 * @retval < 0 failed
 * @retval 0 successful
 */

int ak_drv_key_close(void *handle)
{
	if(NULL == handle)
	{
		
		ak_print_error("open fail!\n");
		return -1;
	}
	dev_close(m_hal_key.fd);
	return 0;
}


