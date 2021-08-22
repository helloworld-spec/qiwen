#include "dev_drv.h"
#include "ak_common.h"
#include "anyka_types.h"
#include "ak_drv_led.h"
#include "kernel.h"

typedef struct 
{
	int fd;
	bool open_flag;
	bool blink_flag;	
	int blink_time[2];  //0 :on_time    1:off_time
	unsigned char led_id;
	
}T_HAL_LED;

static  T_HAL_LED *m_hal_led = NULL;


/**
 * @brief LED device open
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] id is led id with config file 
 * @return viod *
 * @retval device handle
  */
void * ak_drv_led_open(int id)
{

	unsigned char i;
	char num;
	int dev_id;
	
	dev_id = dev_open(DEV_LED_1);
	if(dev_id>0)
	{
		if(0 == dev_ioctl(dev_id, IO_LED_GETNUM, (void *)&num))
		{
			if(NULL == m_hal_led)
			{
				m_hal_led = malloc(sizeof(T_HAL_LED)*num);
				if(NULL != m_hal_led)
				{
					for(i = 0 ;i <num ;i++)
					{
						m_hal_led[i].open_flag = false;
					}
				}
				else
				{
					ak_print_error("led handle malloc false!\n");
					return NULL;
				}
			}
			if(id < num)
			{
				if(!m_hal_led[id].open_flag)
				{
					m_hal_led[id].open_flag = true;
					m_hal_led[id].fd = dev_id;
					m_hal_led[id].blink_flag = false;
					m_hal_led[id].led_id = id;
				}
				return &m_hal_led[id];
			}
			else
			{
				ak_print_error("led id error!\n");
				return NULL;
			}
			
		}
		else
		{
			ak_print_error("get led number false!\n");
		}
	}
	else
	{
		ak_print_error(" led open false!\n");
	}
	return NULL;
}


/**
 * @brief LED on function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle  is led device handle
 * @return int
 * @retval 0, success
 * @retval -1, fail
  */
int ak_drv_led_on(void *handle)
{
	unsigned int data;
	if(NULL == handle)
	{
		ak_print_error("led handle error!\n");
		return -1;
	}
	T_HAL_LED *led_handle = (T_HAL_LED *)handle;
	if(!led_handle->open_flag)
	{
		ak_print_error("led is not open!\n");
		return -1;
	}
	data = ((led_handle->led_id<<29)|1);
	if(-1 == dev_ioctl(led_handle->fd, IO_LED_CTL, &data))
	{
		ak_print_error("led write false!\n");
		return -1;
	}

	return 0;
	
}

/**
 * @brief LED off function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle  is led device handle
 * @return int
 * @retval 0, success
 * @retval -1, fail
  */
int ak_drv_led_off(void *handle)
{
	unsigned int data = 0;
	if(NULL == handle)
	{
		ak_print_error("led handle error!\n");
		return -1;
	}
	
	T_HAL_LED *led_handle = (T_HAL_LED *)handle;
	if(!led_handle->open_flag)
	{
		ak_print_error("led is not open!\n");
		return -1;
	}
	data = ((led_handle->led_id<<29)|0);
	if(-1 == dev_ioctl(led_handle->fd, IO_LED_CTL, &data))
	{
		ak_print_error("led write false!\n");
		return -1;
	}

	return 0;
}

/**
 * @brief LED blink function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle  is led device handle
 * @param [in] on_time :led on time
 * @param [in] off_time :led off time
 * @return int
 * @retval 0, success
 * @retval -1, fail
  */
int ak_drv_led_blink(void *handle, int on_time, int off_time)
{
	unsigned int data;
	
	if(NULL == handle)
	{
		ak_print_error("led handle error!\n");
		return -1;
	}
	
	T_HAL_LED *led_handle = (T_HAL_LED *)handle;
	if(!led_handle->open_flag)
	{
		ak_print_error("led is not open!\n");
		return -1;
	}
	led_handle->blink_time[0] = on_time;
	led_handle->blink_time[1] = off_time;
	on_time = on_time/100;
	off_time = off_time/100;
	data = ((led_handle->led_id<<29)|(off_time<<14)|on_time);
	if(-1 == dev_ioctl(led_handle->fd, IO_LED_BLINK, &data))
	{
		ak_print_error("led write false!\n");
		return -1;
	}
	else
		led_handle->blink_flag = true;

	return 0;
}


/**
 * @brief close LED device
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle  is led device handle
 * @return int
  * @retval 0, success
  * @retval -1, fail
  */
int ak_drv_led_close(void *handle)
{
	int  i = 0;	//off
	if(NULL == handle)
	{
		ak_print_error("led handle error!\n");
		return -1;
	}
	
	T_HAL_LED *led_handle = (T_HAL_LED *)handle;
	if(!led_handle->open_flag)
	{
		ak_print_error("led is not open!\n");
		return -1;
	}
	if(-1 == dev_close(led_handle->fd))
	{
		ak_print_error("led close false!\n");
		return -1;
	}

	led_handle->open_flag = false;
	
	return 0;

}

