/**
 * @file 
 * @brief:
 *
 * This file provides 
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2017-2-09
 * @version 1.0
 */

#include "anyka_types.h"

#include "drv_gpio.h"


#include "dev_drv.h"
#include "device.h"
#include "platform_devices.h"


#ifdef __cpluscplus
extern "C"{
#endif


static T_DEV_INFO wifi_dev = {
	.dev_open_flg  = false,
	.dev_name      = DEV_WIFI,
};



static int wifi_open(int dev_id, void *data)
{

	return dev_id;

}

static int wifi_read(int dev_id, void *data, unsigned int len)
{
	unsigned int *tmp = (unsigned int *)data;
	T_WIFI_INFO  *wifi = (T_WIFI_INFO *)wifi_dev.dev_data;
	
	*tmp = wifi->sdio_share_pin;
	return 0;
}



static T_DEV_DRV_HANDLER wifi_function_handler = 
{
    .drv_open_func  = wifi_open,
    .drv_close_func = NULL,
    .drv_read_func  = wifi_read,
    .drv_write_func = NULL,
    .drv_ioctl_func = NULL,
};




static int wifi_device_reg(void)
{
	void *devcie = NULL;

	devcie  = platform_get_devices_info(wifi_dev.dev_name);
	if(NULL == devcie)
	{
		printk("wifi devcie register fail!");
		return 0;
	}
	
	if (0 !=  dev_alloc_id(&wifi_dev,0, wifi_dev.dev_name))
	{
		printk("wifi devcie alloc id fail!");
		return 0;
	}
	
	
	wifi_dev.dev_data               = devcie;
	wifi_dev.drv_handler            = &wifi_function_handler;
	wifi_function_handler.devcie    = &wifi_dev;
	wifi_function_handler.device_id = wifi_dev.dev_id;
	
	dev_drv_reg(wifi_dev.dev_id, &wifi_dev);
}

dev_module_init(wifi_device_reg)//要在set_platform_devices_info之后,串口正常工作

#ifdef __cplusplus
}
#endif




