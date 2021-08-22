#include "dev_drv.h"
#include "ak_common.h"
#include "command.h"
#include "ak_drv_detector.h"





typedef struct 
{
	int fd;
	unsigned char status;
	
}T_HAL_DETECT;

static T_HAL_DETECT tcard_detect_handle;
static T_HAL_DETECT usb_detect_handle;
static T_HAL_DETECT gpio_detect_handle;


/**
 * @brief open  device detector
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] dev_id :device ID
 * @return viod *
 * @retval device handle
  */
void* ak_drv_detector_open(int dev_id)
{
	switch(dev_id)
	{
		case USB_DETECTOR:
			return NULL;//.&usb_detect_handle;
			break;
		case SD_DETECTOR:
			tcard_detect_handle.fd = dev_open(DEV_TCARD);
			if(-1 == tcard_detect_handle.fd)
				return NULL;
			else
				return &tcard_detect_handle;
			break;
		case GPIO_DETECTOR:
			return  NULL;//&gpio_detect_handle;
			break;
		default:						
			return NULL;
			break;
	}
}

/**
 * @brief detector enable or disnable function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle :device handle
 * @param [in] value : 0 :disable ,1 :enable
 * @return int
 * @retval 0 :success
 * @retval -1 :fail 
  */
int ak_drv_detector_mask(void *handle, int value)
{
	if(&usb_detect_handle == (T_HAL_DETECT *)handle)
	{
		ak_print_normal("mask handle error!");
		return -1;
	}
	else if(&tcard_detect_handle == (T_HAL_DETECT *)handle)
	{
		if(value)//anble detector
		{
			dev_ioctl(tcard_detect_handle.fd,IO_MASK_ENABLE,&value);
		}
		else
		{
			dev_ioctl(tcard_detect_handle.fd,IO_MASK_DISABLE,&value);
			
			
		}
	}
	else if(&gpio_detect_handle == (T_HAL_DETECT *)handle)
	{
		ak_print_normal("mask handle error!");
		return -1;
	}
	else
	{
		ak_print_normal("mask handle error!");
		return -1;
	}

	return 0;

}

/**
 * @brief get detector status function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle :device handle
 * @param [in] event : remain link status
 * @return int
 * @retval 0 :success
 * @retval -1 :fail 
  */
int ak_drv_detector_poll_event(void *handle, int *event)
{
	
	if(&usb_detect_handle == (T_HAL_DETECT *)handle)
	{
		ak_print_normal("poll event handle error!");
		return -1;
	}
	else if(&tcard_detect_handle == (T_HAL_DETECT *)handle)
	{
		//dev_ioctl(tcard_detect_handle.fd,IO_TCARD_DETECT_MASK,1);
		dev_read(tcard_detect_handle.fd,event,1);
	}
	else if(&gpio_detect_handle == (T_HAL_DETECT *)handle)
	{
		ak_print_normal("poll event handle error!");
		return -1;
	}
	else
	{
		ak_print_normal("poll event handle error!");
		return -1;
	}
	return 0;
}

/**
 * @brief wait status change and get detector status function
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle :device handle
 * @param [in] event : remain link status
  * @param [in] ms : the longst wait time,  ms<0: until charge, ms = 0: no wait ,ms>0: wait no than ms . unit :ms
 * @return int
 * @retval 0 :success
 * @retval -1 :fail 
  */
int ak_drv_detector_wait_event(void *handle, int *event, long ms)
{
	long time = ms;
	
	if(&usb_detect_handle == (T_HAL_DETECT *)handle)
	{
		ak_print_normal("poll event handle error!");
		return -1;
	}
	else if(&tcard_detect_handle == (T_HAL_DETECT *)handle)
	{
		dev_ioctl(tcard_detect_handle.fd,IO_BLOCK_MS,&time);
		dev_read(tcard_detect_handle.fd,event,1);
		//dev_ioctl(tcard_detect_handle.fd,IO_TCARD_NORMAL,NULL);
	}
	else if(&gpio_detect_handle == (T_HAL_DETECT *)handle)
	{
		ak_print_normal("poll event handle error!");
		return -1;
	}
	else
	{
		ak_print_normal("poll event handle error!");
		return -1;
	}
	return 0;
}

/**
 * @brief close detector device
 * @author Jian_Kui
 * @date 2016-11-23
 * @param [in] handle :device handle
 * @return int
 * @retval 0 :success
 * @retval -1 :fail 
  */
int ak_drv_detector_close(void *handle)
{
	if(&usb_detect_handle == (T_HAL_DETECT *)handle)
	{
		ak_print_normal("close detect handle error!");
		return -1;
	}
	else if(&tcard_detect_handle == (T_HAL_DETECT *)handle)
	{
		dev_close(tcard_detect_handle.fd);
	}
	else if(&gpio_detect_handle == (T_HAL_DETECT *)handle)
	{
		ak_print_normal("close detect handle error!");
		return -1;
	}
	else
	{
		ak_print_normal("close detect handle error!");
		return -1;
	}
	return 0;
}
