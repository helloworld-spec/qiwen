/**
 * @file 
 * @brief:
 *
 * This file provides 
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2017-6-22
 * @version 1.0
 */
#include "anyka_types.h"

#include "dev_drv.h"
#include "device.h"
#include "platform_devices.h"


#ifdef __cpluscplus
extern "C"{
#endif

static unsigned char tcard_status = 0;//0 : pull out  ;  1:pull in
//static unsigned char block_flag = 0;  //0: disable  ; 1:enable
static T_DEV_INFO tcard_dev = {
	.dev_open_flg  = false,
	.dev_name      = DEV_TCARD,
};


static T_LED_INFO  m_led_para;


static void sd_detect_callback(bool statu)
{
	//printk("gpio0 :%d\n",gpio_get_pin_level(0));
	tcard_status = statu;
	if(statu)
	{
		if (mount_sd())
		{
			printk("mount sd ok!\r\n");
            if (FS_SetAsynWriteBufSize(2*1024*1024,0))
                printk("FS_SetAsynWriteBufSize ok!\n");
            else
                printk("FS_SetAsynWriteBufSize failed!\n");
		}
	}
	else
	{
		unmount_sd();
		printk("unmount sd ok!\r\n");
	}
	if(NULL != tcard_dev.drv_handler->read_complete)
	{
		tcard_dev.drv_handler->read_complete(tcard_dev.dev_id);//发送信号通知上层
	}
}

static int tcard_dev_open(int dev_id, void *data)
{
	unsigned int nb;
	unsigned int dir;
	T_GPIO_INFO   *gpio_info;
	int ret = -1;

		
	gpio_info = (T_GPIO_INFO *)tcard_dev.dev_data;
	nb  = gpio_info->nb;
	
	//printk("jkgpio%d\n",nb);
	detector_init();
	gpio_set_pull_down_r(nb,false);
	if(gpio_info->high)
		//register detetor and gpio number and enable detetor
		detector_register_gpio("SD", nb, 1, true, 50);
	else if(gpio_info->low)
		//register detetor and gpio number and enable detetor
		detector_register_gpio("SD", nb, 0, true, 50);
	else 
		return ret;
	//set gpio interrupt and vtimer interrupt
	detector_set_callback("SD", sd_detect_callback);
	gpio_set_pull_up_r(nb,false);
	gpio_set_pull_down_r(nb,false);
	if(gpio_info->pullup)
		gpio_set_pull_up_r(nb,true);
	if(gpio_info->pulldown)
		gpio_set_pull_down_r(nb,true);
	//anble detector
	detector_enable("SD",true);
	
	return dev_id;

}

static int tcard_dev_read(int dev_id, void *data, unsigned int len)
{
	int * tmp_data;
	tmp_data = (int *)data;
	if(1 == len)
	{
		*tmp_data = tcard_status;
		return 0;
	}
	return -1;

}

static int tcard_dev_ioctl(int dev_id, unsigned long cmd, void *data)
{
	int mask;
	bool return_val = false;
	if(IO_MASK_ENABLE == cmd)
	{
		//enble detector
		return_val = detector_enable("SD",true);
	}	
	else if(IO_MASK_DISABLE== cmd)
	{
		//disable detector
		return_val = detector_enable("SD",false);
	}
	if(!return_val)
			return -1;
	return 0;
}


static int tcard_dev_close(int dev_id)
{
	if(detector_enable("SD",false))
		return 0;
	else 
		return -1;
}


static T_DEV_DRV_HANDLER tcard_function_handler = 
{

    .drv_open_func  = tcard_dev_open,
    .drv_close_func = tcard_dev_close,
    .drv_read_func  = tcard_dev_read,
    .drv_write_func = NULL,
    .drv_ioctl_func = tcard_dev_ioctl,
};




static int tcard_device_reg(void)
{
	void *devcie = NULL;

	devcie  = platform_get_devices_info(tcard_dev.dev_name);
	if(NULL == devcie)
	{
		printk("tcard devcie register fail!");
		return 0;
	}

	if (0 !=  dev_alloc_id(&tcard_dev,0, tcard_dev.dev_name))
	{
		printk("tcard  devcie alloc id fail!");
		return 0;
	}
	
	
	tcard_dev.dev_data               = devcie;
	tcard_dev.drv_handler            = &tcard_function_handler;
	tcard_function_handler.devcie    = &tcard_dev;
	tcard_function_handler.device_id = tcard_dev.dev_id;
	
	dev_drv_reg(tcard_dev.dev_id, &tcard_dev);
}

dev_module_init(tcard_device_reg)//要在set_platform_devices_info之后,串口正常工作

#ifdef __cplusplus
}
#endif




