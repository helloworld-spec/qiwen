/**
 * @file 
 * @brief:
 *
 * This file provides 
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author kjp
 * @date 2017-7-22
 * @version 1.0
 */

#include "anyka_types.h"   //for bool

#include "drv_gpio.h"

#include "hal_gpio.h"
//#include "dev_drv.h"
//#include "device.h"

#include "platform_devices.h"
#include "platform_gpio_common_cfg.h"

#ifdef __cpluscplus
extern "C"{
#endif


	
#define CHECK_GPIO_NB(gpio)	\
	do{\
		if(INVALID_GPIO == gpio) \
			{\
			printk( "GPIO number err\r\n");\
			return -1; \
			}\
		}\
	while(0)


/**
 * @brief 
 * @author 
 * @date 2017-07-22
 * @param 
 * @return 
 * @retval  < 0 :  failed
 * @retval = 0 : successful 
 */
int platform_gpio_init(T_GPIO_INFO *info)
{
	unsigned int gpio_nb;
	T_GPIO_INFO *gpio_info = info; 
		
	gpio_nb    = gpio_info->nb;
	
	CHECK_GPIO_NB(gpio_nb);
		
	gpio_set_pin_as_gpio(gpio_nb);

	if(gpio_info->pullup)
		gpio_set_pull_up_r(gpio_nb, 1);
	else
		gpio_set_pull_up_r(gpio_nb, 0);
	
	if(gpio_info->pulldown)
		gpio_set_pull_down_r(gpio_nb, 1);	
	else
		gpio_set_pull_down_r(gpio_nb, 0);	
	

	if(gpio_info->out)
	{
		gpio_set_pin_dir(gpio_nb, 1);
		
		if(gpio_info->high)
			gpio_set_pin_level(gpio_nb, 0);  // 

		if(gpio_info->low)
			gpio_set_pin_level(gpio_nb, 1);  // low 		
	}

	if(gpio_info->in)
	{
		gpio_set_pin_dir(gpio_nb, 0);
	}

	return 0;
}


/**
 * @brief 
 * @author 
 * @date 2017-07-22
 * @param 
 * @return 
 * @retval  < 0 :  failed
 * @retval = 0 : successful
 */
int platform_gpio_set(T_GPIO_INFO *info,unsigned char level)
{

	unsigned int gpio_nb;
	T_GPIO_INFO *gpio_info = info; 
		
	gpio_nb    = gpio_info->nb;
	
	CHECK_GPIO_NB(gpio_nb);

	gpio_set_pin_level(gpio_nb, level );

	return 0;
}


/**
 * @brief 
 * @author 
 * @date 2017-07-22
 * @param 
 * @return 
 * @retval  < 0 :  failed
 * @retval >= 0 :  pin level; 1: high; 0: low;
 */
int platform_gpio_get(T_GPIO_INFO *info)
{
	int ret = -1;
	unsigned int gpio_nb;
	T_GPIO_INFO *gpio_info = info; 
		
	gpio_nb    = gpio_info->nb;
	
	CHECK_GPIO_NB(gpio_nb);

	ret = gpio_get_pin_level(gpio_nb);

	return ret;
}


/**
 * @brief 
 * @author 
 * @date 2017-07-22
 * @param 
 * @return 0 
 * @retval  < 0 :  failed
 * @retval = 0 : successful
 */
int platform_gpio_int_cb(T_GPIO_INFO *info , T_fPLATFORM_GPIO_CALLBACK callback)
{
	int ret = -1;	
	unsigned int gpio_nb;
	T_GPIO_INFO *gpio_info = info; 

	unsigned char mode = 0xff,  polarity = 0xff;


	gpio_nb    = gpio_info->nb;
	
	CHECK_GPIO_NB(gpio_nb);
	
	if(NULL == callback)
	{
		return ret;
	}

	if(gpio_info->interrupt)
	{
		if(gpio_info->high || gpio_info->low)
		{
			mode = 0;
			if(gpio_info->high)
			{
				polarity = 1;
			}
			else
			{
				polarity = 0;
			}
		}else if(gpio_info->rise || gpio_info->fall)
		{
			mode = 1;
			if(gpio_info->rise)
			{
				polarity = 1;
			}
			else
			{
				polarity = 0;
			}
		}else
		{
			printk( "GPIO interrupt status err\r\n");
			return ret;
		}

		if(1 == gpio_intr_enable(gpio_nb,mode,polarity,callback))
		{
			ret = 0;
		}
	}
	
	return ret;
		
}


/**
 * @brief 
 * @author 
 * @date 2017-07-22
 * @param 
 * @return 
 * @retval  < 0 :  failed
 * @retval = 0 : successful
 */
int platform_gpio_release(T_GPIO_INFO *info)
{
	unsigned int gpio_nb;
	T_GPIO_INFO *gpio_info = info; 
	int ret = -1;
		
	gpio_nb    = gpio_info->nb;
	
	CHECK_GPIO_NB(gpio_nb);

	if(gpio_intr_disable(gpio_nb))
	{
		ret = 0;
	}

	return ret;

}

#ifdef __cplusplus
}
#endif


