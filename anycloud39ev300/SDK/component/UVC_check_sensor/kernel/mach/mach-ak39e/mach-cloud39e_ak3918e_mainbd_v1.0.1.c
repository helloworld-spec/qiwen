
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

#include "arch_init.h"
 
#include "platform_devices.h"
#include "dev_drv.h"

#ifdef __cplusplus
extern "C"{
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif
static  T_CAMERA_PLATFORM_INFO camera_platform_info = {
	//sensor information
	.sensor_info = {
		.gpio_reset = {
				.nb        = 5,	
				},
				
		.gpio_avdd = {
					.nb        = 0,	
					},
					
		.gpio_scl = {
					.nb        = 27,	
					},
					
		.gpio_sda = {
					.nb        = 28,	
					},	
	},
	
	//ircut information
	.ircut_info  = {
		.gpio_ctrl = {
				.nb        = 13,	
				},
				
		.gpio_dectect = {
				.nb        = 0,	
				},

	},

	//irfeed gpio information
	.irfeed_gpio_info  = {
		.logic_level_day = 1,// 0 or 1
		.gpio = {
				.nb			= 47,//INVALID_PIN,//47,
				.in			= 1,
				},
	},
};


T_PLATORM_DEV_INFO camera_platform_dev = {
	.dev_name = DEV_CAMERA,
	.devcie   =&camera_platform_info,	
};


//speaker information
static  T_SPEAKER_INFO speaker_platform_info = {
	.gpio_attribute = {
				.nb        = 11,
				.high      = 1,
				.out	   = 1,
				},
};

T_PLATORM_DEV_INFO speaker_platform_dev = {
	.dev_name = DEV_SPEAKER,
	.devcie   =&speaker_platform_info,	
};



//led information
static  T_LED_INFO led1_platform_info = {
	.led_num = 1,
	.gpio_attribute[0] = {
				.nb        = 13,	
				.high      = 1,
				.out	   = 1,
				},
	.led_status[0] = false,
	
#if 0

	.gpio_attribute[1] = {
				.nb 	   = 13,	
				.high	   = 1,
				.out	   = 1,
					},
	.led_status[1] = false,	
#endif
};

T_PLATORM_DEV_INFO led1_platform_dev = {
	.dev_name = DEV_LED_1,
	.devcie   =&led1_platform_info,	
};


//tcard detect
static T_TCARD_INFO tcard_platform_info = {
	.gpio_attribute = {
				.nb        = 10,	
				.low  	   = 1,
			
				},
};
T_PLATORM_DEV_INFO tcard_platform_dev ={
	.dev_name = DEV_TCARD,
	.devcie   =&tcard_platform_info,	
};

//keypad information

#define KEY_ROW_QRY  1
#define KEY_COL_QRY  1

unsigned char m_ucRowGpio[KEY_ROW_QRY]  = {47};

unsigned char m_ucColumnGpio[KEY_COL_QRY] = {47};


unsigned long m_keypad_matrix[KEY_ROW_QRY][KEY_COL_QRY] = 
{
	{47},
};

signed char keypad_updown_matrix[KEY_ROW_QRY][KEY_COL_QRY] = 
{
	{0},

};	// ==0 means key-up, >0 count key-down time(timer is 20ms)



static T_KEYPAD_INFO keypad_platform_info = {
	
	.type  =  KEYPAD_KEY_PER_GPIO,
	.parm.parm_gpio = {
		.row_qty   =  KEY_ROW_QRY,         /* row gpio 数量 */
	    .column_qty   =  KEY_COL_QRY,      /* column gpio 数量 */
	    .RowGpio   = &m_ucRowGpio[0],          /* row gpio数组的首地址 */
	    .ColumnGpio = &m_ucColumnGpio[0],      /* Column gpio数组的首地址 */
	    .keypad_matrix  = &m_keypad_matrix[0][0], /* 键盘阵列逻辑数组的首地址 */
	    .updown_matrix  =  &keypad_updown_matrix[0][0],    /* updown逻辑数组的首地址 */
	    .active_level  =  0,        /* 键盘有效电平值, 1或0 */
			}, 

};
T_PLATORM_DEV_INFO keypad_platform_dev = {
	.dev_name   = DEV_KEYPAD,
	.devcie     = &keypad_platform_info,
};

//i2c information
static T_I2C_INFO i2c_platform_info = {

	.i2c_mode = 0,
		
	.gpio_sda = {
				.nb        = 28,	
				},
	.gpio_scl = {
				.nb        = 27,	

	},
};
T_PLATORM_DEV_INFO i2c_platform_dev ={
	.dev_name = DEV_I2C,
	.devcie   =&i2c_platform_info,
};

//UART information
static T_UART_INFO uart_platform_info = {
	.baudrate = 115200,
	.uart_id       = 1,	//串口1
};

//uart for commucation
T_PLATORM_DEV_INFO uart_platform_dev = {
	.dev_name = DEV_UART_1,
	.devcie   =&uart_platform_info,	
};

//wifi information
static T_WIFI_INFO wifi_platform_info =  {
	.sdio_share_pin = 1,
};

T_PLATORM_DEV_INFO wifi_platform_dev = {
	.dev_name = DEV_WIFI,
	.devcie   =&wifi_platform_info,	
};


#if 0
//SPI information
static T_SPI_INFO spi_platform_info = {
	.clock     =  2000000, //20M 
	.spi_id    = 1, 	
	.mode      = 0,	
	.sdio_share_pin = 1,	
	.role      = 1, 
	.ex_gpio   = {
			.nb    = 23,
			.high  = 1,
			.out   = 1,
	},

};

//spi for commucation	
T_PLATORM_DEV_INFO spi_platform_dev = {
	.dev_name = DEV_SPI_1,
	.devcie   =&spi_platform_info,	
};
#endif
//
static T_PLATORM_DEV_INFO *ak39_evt_platform_devices[] = {
	&uart_platform_dev,
	&led1_platform_dev,
	/*&spi_platform_dev,*/
	&speaker_platform_dev,
	&camera_platform_dev,
	&wifi_platform_dev,
	&keypad_platform_dev,
	&tcard_platform_dev,
	&i2c_platform_dev,
};




static int set_devices_info(void)
{

	/* register platform devices */
	platform_add_devices_info(ak39_evt_platform_devices, ARRAY_SIZE(ak39_evt_platform_devices));
    return 0;
}


module_init(set_devices_info)


#ifdef __cplusplus
}
#endif




