/**
 * @file  ak_drv_usbdisk.c
 * @brief: Implement  operations of how to use usb disk device.
 *
 * Copyright (C) 2016 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  KeJianping
 * @date    2016-12-04
 * @version 1.0
 */



#ifdef __cplusplus
extern "C" {
#endif
#include "dev_drv.h"

#include "ak_common.h"
//#include "anyka_types.h"

#include "ak_drv_usbdisk.h"

#define USBDISK_NO_DEVICE          (-1)
#define USBDISK_DEVICE_OPEN_FAILED (-2)


typedef struct 
{
	int fd;
	
}T_HAL_USBDISK;

static  T_HAL_USBDISK m_hal_usbdisk = {0};


 /**
 * @brief  
 * @author 
 * @date 2016-12-04
 * @param void
 * @return int
 * @retval  < 0 :  failed
 * @retval  = 0 : successful
 */
int ak_drv_udisk_start(void)
{
	int ret;
	
	m_hal_usbdisk.fd = dev_open(DEV_USBDISK);
	
	if (m_hal_usbdisk.fd < 0)
	{
		return USBDISK_DEVICE_OPEN_FAILED;
	}

	return 0;
}


 /**
 * @brief  
 * @author 
 * @date 2016-12-04
 * @param void
 * @return int
 * @retval  < 0 :  failed
 * @retval  = 0 : successful
 */
int ak_drv_udisk_stop(void)
{

	int ret;
	
	ret = dev_close(m_hal_usbdisk.fd);

	return ret;
}


/*@}*/
#ifdef __cplusplus
}
#endif


