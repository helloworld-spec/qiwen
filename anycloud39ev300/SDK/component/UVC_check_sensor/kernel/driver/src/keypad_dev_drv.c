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
#include "hal_keypad.h"
#include "platform_devices.h"


typedef struct
{
	T_eKEY_PRESSMODE press_mode;
	T_fKEYPAD_CALLBACK callback_func;

}T_KEYPAD_INIT_PARA;
typedef struct
{
	unsigned long type_index;
	void  *pKeypadPara ;
}T_KEYPAD_PARA_INFO;


#ifdef __cpluscplus
extern "C"{
#endif


static T_DEV_INFO keypad_dev = {
	.dev_open_flg  = false,
	.dev_name      = DEV_KEYPAD,
};

static int keypad_open(int dev_id, void *data)
{
	T_KEYPAD_INFO *keypad_info ;
	keypad_info = (T_KEYPAD_INFO *)keypad_dev.dev_data;


	keypad_init(NULL, keypad_info->type, &(keypad_info->parm));
	keypad_set_pressmode(eMULTIPLE_PRESS);
	keypad_enable_intr();

	
	return dev_id;

}

static int keypad_close(int dev_id)
{
	
	keypad_disable_intr();
	keypad_delect_key();
	return 0;
}

static int keypad_read(int dev_id, void *data, unsigned int len)
{
	T_KEYPAD *key = (T_KEYPAD *)data;
	if(keypad_get_key(key))
	{
		return 0;
	}
	else
	{
		return -1;
	}
	
}

static T_DEV_DRV_HANDLER keypad_function_handler = 
{

    .drv_open_func  = keypad_open,
    .drv_close_func = keypad_close,
    .drv_read_func  = keypad_read,
    .drv_ioctl_func = NULL,
};




static int keypad_device_reg(void)
{
	void *devcie = NULL;

	devcie  = platform_get_devices_info(keypad_dev.dev_name);
	if(NULL == devcie)
	{
		printk("keypad devcie register fail!");
		return 0;
	}

	if (0 !=  dev_alloc_id(&keypad_dev,0, keypad_dev.dev_name))
	{
		printk("keypad  devcie alloc id fail!");
		return 0;
	}
	keypad_dev.dev_data               = devcie;
	keypad_dev.drv_handler            = &keypad_function_handler;
	keypad_function_handler.devcie    = &keypad_dev;
	keypad_function_handler.device_id = keypad_dev.dev_id;
	
	dev_drv_reg(keypad_dev.dev_id, &keypad_dev);
}

dev_module_init(keypad_device_reg)//要在set_platform_devices_info之后,串口正常工作

#ifdef __cplusplus
}
#endif

