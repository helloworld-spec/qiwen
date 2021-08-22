/**
 * @file gpio_config.h
 * @brief gpio function header file
 * Copyright (C) 2010 Anyka (Guangzhou) Microelectronics Technology Co., Ltd
 * @AUTHOR
 * @date 2010-12-10
 * @VERSION 1.0
 * @REF
 * @NOTE:
 * 1. 对于mmi系统中已定义了的gpio，不需要删除相关代码，只需将其定义为INVALID_GPIO
 * 2. 如果需要用到扩展io，只需要打开GPIO_MULTIPLE_USE宏，并设置对应的gpio
 *    GPIO_EXPAND_OUT1和GPIO_EXPAND_OUT2，如果只有一组扩展io,可以将GPIO_EXPAND_OUT2
 *	  设为INVALID_GPIO即可
 */
#ifndef __GPIO_CONFIG_H__
#define __GPIO_CONFIG_H__


#include "drv_gpio.h"

#define GPIO_CAMERA_RESET           39 //49: V1.1 chip borad
#define GPIO_CAMERA_AVDD            40
#define GPIO_I2C_SCL                27
#define GPIO_I2C_SDA                28 

#define GPIO_WIFI_POWERDOWN		    48

#define GPIO_IRCUT_CTRL					5		//ircut控制引脚
#define GPIO_IRCUT_DECTECT				0		//光敏输入检测

#endif //#ifndef __GPIO_CONFIG_H__

