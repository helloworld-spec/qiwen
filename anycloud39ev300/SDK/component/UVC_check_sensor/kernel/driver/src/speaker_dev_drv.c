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


static T_DEV_INFO speaker_dev = {
	.dev_open_flg  = false,
	.dev_name      = DEV_SPEAKER,
};


static T_SPEAKER_INFO  m_speaker_para;

static int speaker_open(int dev_id, void *data)
{

	unsigned int nb;
	unsigned int level;
	T_GPIO_INFO   *gpio_info;
	int ret = -1;

		
	gpio_info = speaker_dev.dev_data;
	nb        = gpio_info->nb;
	level     = gpio_info->high;

	ret = gpio_set_pin_as_gpio(nb);
	if(!ret)return -1;
	
	gpio_set_pin_dir(nb, 1);
	gpio_set_pin_level(nb, level);


	return dev_id;

}

static int speaker_close(int dev_id)
{
	unsigned int nb;
	unsigned int level;
	T_GPIO_INFO   *gpio_info;
	int ret = -1;
	
	gpio_info = speaker_dev.dev_data;
	nb        = gpio_info->nb;
	level     = gpio_info->high;
	
	gpio_set_pin_level(nb, !level);
	
	return 0;
}


static T_DEV_DRV_HANDLER speaker_function_handler = 
{
    .drv_open_func  = speaker_open,
    .drv_close_func = speaker_close,
    .drv_read_func  = NULL,
    .drv_write_func = NULL,
    .drv_ioctl_func = NULL,
};




static int speaker_device_reg(void)
{
	void *devcie = NULL;

	devcie  = platform_get_devices_info(speaker_dev.dev_name);
	if(NULL == devcie)
	{
		printk("speaker devcie register fail!");
		return 0;
	}
	
	if (0 !=  dev_alloc_id(&speaker_dev,0, speaker_dev.dev_name))
	{
		printk("speaker devcie alloc id fail!");
		return 0;
	}
	
	
	speaker_dev.dev_data               = devcie;
	speaker_dev.drv_handler            = &speaker_function_handler;
	speaker_function_handler.devcie    = &speaker_dev;
	speaker_function_handler.device_id = speaker_dev.dev_id;
	
	dev_drv_reg(speaker_dev.dev_id, &speaker_dev);
}

dev_module_init(speaker_device_reg)//要在set_platform_devices_info之后,串口正常工作

#ifdef __cplusplus
}
#endif




