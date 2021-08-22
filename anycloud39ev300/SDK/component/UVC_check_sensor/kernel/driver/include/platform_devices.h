
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

#include "print.h"


#ifndef  __PLATFORM_DEVICES_H__
#define  __PLATFORM_DEVICES_H__

#ifdef __cplusplus
extern "C"{
#endif

#define INVALID_PIN	0xffffffff

/*
 * Used for initialization calls..
 */
typedef int (*dev_initcall_t)(void);
typedef void (*dev_exitcall_t)(void);


#define __dev_initcall(fn) \
dev_initcall_t __dev_initcall_##fn \
__attribute__((__section__(".dev_initcall"))) = fn;

/*
 *  module init
 */
#define dev_module_init(x)  __dev_initcall(x)

/*
*  KEYPAD_GPIO
*/

typedef struct{
    unsigned long    row_qty;                ///< row gpio 数量 
    unsigned long    column_qty;             ///< column gpio 数量 
    unsigned char    *RowGpio;               ///< 指向row gpio 数组的指针 
    unsigned char    *ColumnGpio;            ///< 指向Column gpio 数组的指针 
    char             *updown_matrix;         ///< 指向updown逻辑数组的指针 
    unsigned long    *keypad_matrix;         ///< 指向键盘阵列逻辑数组的指针 
    unsigned long    active_level;           ///< 键盘有效电平值, 1或0 
    unsigned long    switch_key_id;          ///< 电源键的gpio id值 
    unsigned long    switch_key_value;        ///< 电源键的键值 
    unsigned long    switch_key_active_level; ///< 电源键的有效电平值，1或0 
} T_PLATFORM_KEYPAD_GPIO;



/*
*  KEYPAD_ANALOG
*/

typedef struct{
   // KEY_DETECT_STR      *key_avl_array;   //指向AD按键的映身表。
    unsigned long       key_max_num;      //按键的数量
    unsigned long       ad_avl_offset;    //一个按键的上下偏移量
    unsigned long       AdValMin;         //有效按键的最小值
    unsigned long       AdValMax;         //有效按键的最大值
} T_PLATFORM_KEYPAD_ANALOG;

/*
*  KEYPAD_TYPE
*/
typedef enum {
    KEYPAD_MATRIX_NORMAL = 0,   // normal matrix keypad
    KEYPAD_MATRIX_DIODE,        // diode matrix keypad
    KEYPAD_MATRIX_MIXED,        // mixed matrix keypad
    KEYPAD_KEY_PER_GPIO,        //one gpio = one key keypad
    KEYPAD_MATRIX_TIANXIN,      // keypad for tianxin board
    KEYPAD_ANALOG,              // keypad for analog keypad
    KEYPAD_TYPE_NUM
} T_KEYPAD_TYPE;




typedef union{
    T_PLATFORM_KEYPAD_GPIO      parm_gpio;
    T_PLATFORM_KEYPAD_ANALOG    parm_analog;
}T_PLATFORM_KEYPAD_PARM;




/*
*
*   GPIO attribue 
*
*/
typedef struct
{
	/*
	* nb:  GPIO  number。
	*/
	unsigned int  nb; 
	
	/*
	* pullup:  0,mask; 1 unmask
	*/
	unsigned int pullup:1; 
	
	/*
	* pulldown:0,mask; 1 unmask
	*/
	unsigned int pulldown:1; 
	

	/*
	* interrupt:0,mask; 1 unmask
	*/
	unsigned int interrupt:1; 


	/*
	* high:0,mask; 1 unmask
	*/
	unsigned int high:1; 

	/*
	* low:0,mask; 1 unmask
	*/
	unsigned int low:1; 

	/*
	* rise:0,mask; 1 unmask
	*/
	unsigned int rise:1; 

	/*
	* fall:0,mask; 1 unmask
	*/
	unsigned int fall:1; 
	
	/*
	* out:0,mask; 1 unmask
	*/
	unsigned int out:1; 

	/*
	* in:0,mask; 1 unmask
	*/
	unsigned int in:1;

}T_GPIO_INFO;
/*
*  T_KEYPAD_INFO
*/
typedef struct
{
	T_KEYPAD_TYPE           type;
	T_PLATFORM_KEYPAD_PARM  parm;
}T_KEYPAD_INFO;


//wifi
//camera
typedef struct
{
	/*
	* gpio attribute
	*/
	T_GPIO_INFO gpio_power;

	/*
	* share_pin:  gpio selected
	*/
	unsigned char sdio_share_pin;

}T_WIFI_INFO;

//ir feed gpio
typedef struct
{
	/*
	* logic level in day mode 
	*/
	int logic_level_day;

	/*
	* gpio attribute
	*/
	T_GPIO_INFO gpio; 
}T_IRFEED_GPIO_INFO;

//ircut
typedef struct
{
	/*
	* gpio attribute
	*/
	T_GPIO_INFO gpio_ctrl; 

	/*
	* gpio attribute
	*/
	T_GPIO_INFO gpio_dectect; 
	
}T_IRCUT_INFO;

//sensor
typedef struct
{
	/*
	* gpio attribute
	*/
	T_GPIO_INFO gpio_reset; 

	/*
	* gpio attribute
	*/
	T_GPIO_INFO gpio_avdd; 	

	/*
	* gpio attribute
	*/
	T_GPIO_INFO gpio_scl; 	

	/*
	* gpio attribute
	*/
	T_GPIO_INFO gpio_sda; 


}T_SENSOR_INFO;

//camera
typedef struct
{
	/*
	* sensor informaiton
	*/
	T_SENSOR_INFO sensor_info;

	/*
	* ircut  informaiton
	*/
	T_IRCUT_INFO  ircut_info;

	/*
	* ir feed informaiton
	*/
	T_IRFEED_GPIO_INFO  irfeed_gpio_info;

}T_CAMERA_PLATFORM_INFO;

//speaker
typedef struct
{
	/*
	* gpio attribute
	*/
	T_GPIO_INFO gpio_attribute; 
	
}T_SPEAKER_INFO;


//led
#define LED_NUM 5
typedef struct
{

	unsigned char led_num;
	/*
	* gpio attribute
	*/
	T_GPIO_INFO gpio_attribute[LED_NUM]; 

	bool led_status[LED_NUM];  //false: off     ture : on
	
	
}T_LED_INFO;

//tcard
typedef struct
{
	T_GPIO_INFO gpio_attribute;
		
}T_TCARD_INFO;

//i2c
typedef struct
{
	//i2c_mode :  0 : TWI ,  1: gpio->I2C
	unsigned char i2c_mode; 
	
	T_GPIO_INFO gpio_sda;
	T_GPIO_INFO gpio_scl;
}T_I2C_INFO;

//UART
typedef struct
{
	/*
	* baudrate:2.4kbps~3Mbps
	*/
	unsigned int baudrate; 

	/*
	*uart_id: uart number。
	*/
	unsigned char uart_id; 
	
}T_UART_INFO;


typedef struct
{
	/*
	* gpio attribute
	*/
	T_GPIO_INFO ex_gpio; 

	/*
	* clock:SPI working frequency
	*/
	unsigned long clock; 

	/*
	* nb: spi number。
	*/
	unsigned char spi_id; 
	
	/*
	* mode:spi mode selected 
	*/
	unsigned char mode;	

	/*
	* share_pin:  gpio selected
	*/
	unsigned char spi_share_pin;	///
	/*
	* role:master or slave. 1:master; 0:slave
	*/
	unsigned char role:1; 


}T_SPI_INFO;


//platform device information
typedef struct
{
	char *dev_name;
	void *devcie;
}T_PLATORM_DEV_INFO;


/** 
 * @brief  regoster  devcies. after the uart0 initializtion 
 *
 * @author KeJianping
 * @date 2017-2-09
 * @return  void
 */
void dev_do_register(void);


/** 
 * @brief  get platform devices informaiton
 *
 * @author KeJianping
 * @date 2017-2-09
 * @param dev_name[in] devcie's name,
 * @return  void *
 * @retval  NULL:  failed
 * @retval  NOT NULL: successful
 */
void *platform_get_devices_info(char *dev_name);


/** 
 * @brief  add platform devices informaiton
 *
 * @author KeJianping
 * @date 2017-2-09
 * @param platform_dev[in]  platform devcies 
 * @param num[in] the platform devcie count 
 * @return  void 
 */
void platform_add_devices_info(T_PLATORM_DEV_INFO **platform_dev, int num);



#ifdef __cplusplus
}
#endif

#endif  //#ifndef  __PLATFORM_DEVICES_H__

