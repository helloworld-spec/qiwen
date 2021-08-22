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




static T_DEV_INFO camera_dev = {
	.dev_open_flg  = false,
	.dev_name      = DEV_CAMERA,
};

static T_CAMERA_PLATFORM_INFO  m_camera_para;

static int camera_open(int dev_id, void *data)
{
	return dev_id;
}

static int camera_close(int dev_id)
{
	return dev_id;
}

static int camera_read(int dev_id, void *data, unsigned int len)
{
	unsigned int *tmp = (unsigned int *)data;
	*tmp = (unsigned int)camera_dev.dev_data;
	return 0;
}

static int camera_irfeed_gpio_open(int dev_id)
{
	int is_in, is_out;
	unsigned int irfeed_gpio_nb;
	T_CAMERA_PLATFORM_INFO *camera_para;
		
	camera_para = camera_dev.dev_data;
	irfeed_gpio_nb = camera_para->irfeed_gpio_info.gpio.nb;
	printf("%s irfeed_gpio_nb:%d\n", __func__, irfeed_gpio_nb);
	if (irfeed_gpio_nb == INVALID_PIN) {
		printf("%s irfeed_gpio_nb:%d err\n", __func__, irfeed_gpio_nb);
		return -1;
	}

	if(!gpio_set_pin_as_gpio(irfeed_gpio_nb))
	{
		return -1;
	}
	
	is_in = camera_para->irfeed_gpio_info.gpio.in;
	if (is_in)
		gpio_set_pin_dir(irfeed_gpio_nb, 0);
	is_out = camera_para->irfeed_gpio_info.gpio.out;
	if (is_out)
		gpio_set_pin_dir(irfeed_gpio_nb, 1);
	return 0;
}

static int camera_irfeed_gpio_get(int dev_id, void *data)
{
	int logic_level_day;
	int cur_level;
	unsigned int nb;
	T_CAMERA_PLATFORM_INFO *camera_para;

	camera_para = camera_dev.dev_data;
	nb = camera_para->irfeed_gpio_info.gpio.nb;
	logic_level_day = camera_para->irfeed_gpio_info.logic_level_day;
	cur_level = gpio_get_pin_level(nb);
	if (logic_level_day == cur_level)
		*((int *)data) = CAMERA_IRFEED_GPIO_STATUS_DAY;
	else
		*((int *)data) = CAMERA_IRFEED_GPIO_STATUS_NIGHT;

	return 0;
}

static int camera_irfeed_gpio_close(int dev_id)
{
	return 0;
}

static int camera_ioctl(int dev_id, unsigned long cmd, void *data)
{
	int ret = 0;

	switch (cmd) {
		case IO_CAMERA_IRFEED_GPIO_OPEN:
			ret = camera_irfeed_gpio_open(dev_id);
			break;
		case IO_CAMERA_IRFEED_GPIO_GET:
			ret = camera_irfeed_gpio_get(dev_id, data);
			break;
		case IO_CAMERA_IRFEED_GPIO_CLOSE:
			ret = camera_irfeed_gpio_close(dev_id);
			break;
		default:
			printf("%s cmd:%d no defined\n", __func__, cmd);
			ret = -1;
			break;
	}

	return ret;
}

static T_DEV_DRV_HANDLER camera_function_handler = 
{
    .drv_open_func  = camera_open,
    .drv_close_func = camera_close,
    .drv_read_func  = camera_read,
    .drv_write_func = NULL,
    .drv_ioctl_func = camera_ioctl,
};




static int camera_device_reg(void)
{
	void *devcie = NULL;

	devcie  = platform_get_devices_info(camera_dev.dev_name);
	if(NULL == devcie)
	{
		printk("camera devcie register fail!");
		return 0;
	}
	
	if (0 !=  dev_alloc_id(&camera_dev,0, camera_dev.dev_name))
	{
		printk("camera devcie alloc id fail!");
		return 0;
	}
	
	
	camera_dev.dev_data               = devcie;
	camera_dev.drv_handler            = &camera_function_handler;
	camera_function_handler.devcie    = &camera_dev;
	camera_function_handler.device_id = camera_dev.dev_id;
	
	dev_drv_reg(camera_dev.dev_id, &camera_dev);
}

dev_module_init(camera_device_reg)//要在set_platform_devices_info之后,串口正常工作

#ifdef __cplusplus
}
#endif




