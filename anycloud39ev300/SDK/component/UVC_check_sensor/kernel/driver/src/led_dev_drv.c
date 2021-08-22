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

#include "drv_api.h"
#include "dev_drv.h"
#include "device.h"
#include "platform_devices.h"
#include "kernel.h"

#ifdef __cpluscplus
extern "C"{
#endif

typedef struct LED_DATA
{
	int time_id;
	int led_num;
	long *on_time;
	long *cur_time;
	long *off_time;
}T_LED_DATA;

static T_LED_DATA led_data =
{
	.time_id = -1,
	.led_num = 0,
	.on_time = NULL,
	.cur_time = NULL,
	.off_time = NULL,
};
	
static T_DEV_INFO led_dev = 
{
	.dev_open_flg  = false,
	.dev_name      = DEV_LED_1,
};



static void blink_ctrol(signed long timer_id, unsigned long delay)
{
	int i;
	unsigned int nb;
	unsigned int level;
	
	T_LED_INFO *m_led_para = NULL;
	m_led_para = (T_LED_INFO *)led_dev.dev_data;
	
	if(timer_id == led_data.time_id)
	{
		for(i = 0;i < led_data.led_num; i++)
		{
			if(led_data.on_time[i] == 0)
				continue;
			led_data.cur_time[i] += delay;
			if(m_led_para->led_status[i])
			{
				if(led_data.cur_time[i] >= led_data.on_time[i])
				{
					nb = m_led_para->gpio_attribute[i].nb;
					level = m_led_para->gpio_attribute[i].high;
					led_data.cur_time[i] = 0;
					m_led_para->led_status[i] = false;
					gpio_set_pin_level(nb, !level);//off
				}
			}
			else
			{
				if(led_data.cur_time[i] >= led_data.off_time[i])
				{
					nb = m_led_para->gpio_attribute[i].nb;
					level = m_led_para->gpio_attribute[i].high;
					led_data.cur_time[i] = 0;
					m_led_para->led_status[i] = true;
					gpio_set_pin_level(nb, level);//on
				}
			}
		}
	}
	else
	{
		printk("led timer error!\n");
		vtimer_stop(timer_id);
	}
}


static int led_open(int dev_id, void *data)
{
	int i = 0;
	unsigned int nb, dir, level;
	int ret = -1;
	T_LED_INFO *m_led_para = NULL;
	
	m_led_para = (T_LED_INFO *)led_dev.dev_data;
	
	led_data.led_num = m_led_para->led_num;
	printk("led num :%d\n",led_data.led_num);

	led_data.on_time = malloc(sizeof(long)*led_data.led_num);
	led_data.off_time = malloc(sizeof(long)*led_data.led_num);
	led_data.cur_time = malloc(sizeof(long)*led_data.led_num);
	
	for(i = 0; i< led_data.led_num; i++)
	{
		nb  = m_led_para->gpio_attribute[i].nb;
		dir = m_led_para->gpio_attribute[i].out;
		level = m_led_para->gpio_attribute[i].high;
		//set pin LED1 with gpio
		if(!gpio_set_pin_as_gpio(nb))
		{
			return ret;
		}
	
		//set gpio dir with output
		gpio_set_pin_dir(nb, dir);

		//set led status
		if(m_led_para->led_status[i])  //on
		{
			gpio_set_pin_level(nb, level);//on
		}
		else  //off
		{
			gpio_set_pin_level(nb, !level);//off
		}

		led_data.on_time[i] = 0;
		led_data.off_time[i] = 0;
		led_data.cur_time[i] = 0;
		
	}

	led_data.time_id = vtimer_start(100, true, blink_ctrol);
	
	return dev_id;

}

static int led_close(int dev_id)
{
	int i;
	T_LED_INFO *m_led_para = NULL;
	unsigned int nb;
	unsigned int level;
	
	m_led_para = (T_LED_INFO *)led_dev.dev_data;
	
	for(i = 0; i < led_data.led_num; i++)
	{
		nb  = m_led_para->gpio_attribute[i].nb;
		level     = m_led_para->gpio_attribute[i].high;
		led_data.cur_time[i] = 0;
		m_led_para->led_status[i] = false;
		led_data.on_time[i] = 0;
		led_data.off_time[i] = 0;
		gpio_set_pin_level(nb, !level);//off
	}
	vtimer_stop(led_data.time_id);
	led_data.time_id = -1;
	free(led_data.cur_time);
	free(led_data.off_time);
	free(led_data.on_time);
	return 0;
}

/*


IO_LED_CTL:
param: data
bit29~31:led_id
bit0: led_status

IO_LED_BLINK:
param: data
bit29~31:led_id
bit0~13:on_time
bit14~27:off_time  (unit:100ms)
*/
static int led_ioctl(int dev_id, unsigned long cmd, void *data)
{
	T_LED_INFO *m_led_para = NULL;
	m_led_para = (T_LED_INFO *)led_dev.dev_data;
	unsigned int t_data;
	unsigned int nb;
	unsigned int level;
	unsigned char led_id;
	unsigned int on_time,off_time;
	bool status;
	unsigned char *tmp_data;
	if(cmd == IO_LED_GETNUM)
	{
		tmp_data = (unsigned char *)data;
		*tmp_data = led_data.led_num;
		return 0;
	}

	t_data = *(unsigned int *)data;
	led_id = t_data>>29;   //get led_id
	if(led_id>=led_data.led_num)
		return -1;
	
	if(cmd == IO_LED_CTL)  //control  led  on/off
	{
		led_data.cur_time[led_id] = 0;
		led_data.on_time[led_id] = 0;
		led_data.off_time[led_id] = 0;
		
		status = (t_data&1);	//get on/off data
		nb = m_led_para->gpio_attribute[led_id].nb;
		level = m_led_para->gpio_attribute[led_id].high;
		if(status)
			gpio_set_pin_level(nb, level);//on
		else
			gpio_set_pin_level(nb, !level);//off
			
		m_led_para->led_status[led_id] = status;
		
	}
	else if(cmd == IO_LED_BLINK)
	{
		on_time = (t_data&(~0xFFFFC000));      //get on_time
		off_time = (t_data&(~0xF0000000))>>14;    //get off_time
		led_data.on_time[led_id] = on_time * 100;
		led_data.off_time[led_id] = off_time * 100;

	}
	else
		return -1;

	return 0;
}

static T_DEV_DRV_HANDLER led_function_handler = 
{

    .drv_open_func  = led_open,
    .drv_close_func = led_close,
    .drv_read_func  = NULL,
    .drv_write_func = NULL,
    .drv_ioctl_func = led_ioctl,
};




static int led_device_reg(void)
{
	void *devcie = NULL;

	devcie  = platform_get_devices_info(led_dev.dev_name);
	if(NULL == devcie)
	{
		printk("led devcie register fail!");
		return 0;
	}

	if (0 !=  dev_alloc_id(&led_dev,0, led_dev.dev_name))
	{
		printk("led  devcie alloc id fail!");
		return 0;
	}
	
	
	led_dev.dev_data               = devcie;
	led_dev.drv_handler            = &led_function_handler;
	led_function_handler.devcie    = &led_dev;
	led_function_handler.device_id = led_dev.dev_id;
	
	dev_drv_reg(led_dev.dev_id, &led_dev);
}

dev_module_init(led_device_reg)//要在set_platform_devices_info之后,串口正常工作

#ifdef __cplusplus
}
#endif




