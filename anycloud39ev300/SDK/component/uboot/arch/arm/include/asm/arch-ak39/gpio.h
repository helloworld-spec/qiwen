/**
 * @file gpio.h
 * @brief gpio function header file
 *
 * This file define gpio macros and APIs: intial, set gpio, get gpio. etc.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Miaobaoli
 * @date 2005-07-24
 * @version 1.0
 *
 * @note:
 * 1. 对于mmi系统中已定义了的gpio，不需要删除相关代码，只需将其定义为INVALID_GPIO
 
 * 2. 如果需要用到扩展io，只需要打开GPIO_MULTIPLE_USE宏，并设置对应的gpio
 *    GPIO_EXPAND_OUT1和GPIO_EXPAND_OUT2，如果只有一组扩展io,可以将GPIO_EXPAND_OUT2
 *	  设为INVALID_GPIO即可
 * 
 * 3. 对于不同的硬件板请以宏隔开并配置好相应宏定义
 *
 */

#ifndef __GPIO_H__
#define __GPIO_H__


/**@brief total number
 */
#define GPIO_NUMBER                     79

/**@brief invalidate gpio
 */
#define INVALID_GPIO                    0xfe

/** @name gpio output level define
 *  define gpio output level
 */
 /*@{*/
#define GPIO_LEVEL_LOW                  0
#define GPIO_LEVEL_HIGH                 1
/* @} */

/** @name gpio dir define
 *  define gpio dir
 */
 /*@{*/
#define GPIO_DIR_INPUT                  0
#define GPIO_DIR_OUTPUT                 1
/* @} */

/** @name gpio interrupt control define
 *  define gpio interrupt enable/disable
 */
 /*@{*/
#define GPIO_INTERRUPT_DISABLE          0
#define GPIO_INTERRUPT_ENABLE           1
/* @} */

/** @name gpio interrupt active level
 *  define gpio interrupt active level
 */
 /*@{*/
#define GPIO_INTERRUPT_LOW_ACTIVE       0
#define GPIO_INTERRUPT_HIGH_ACTIVE      1   
/* @} */

/** @name gpio interrupt type define
 *  define gpio interrupt type
 */
 /*@{*/
#define GPIO_LEVEL_INTERRUPT            0
#define GPIO_EDGE_INTERRUPT             1
/* @} */


typedef enum
{
    PULLUP = 0,
    PULLDOWN,
    PULLUPDOWN,
    UNDEFINED
}T_GPIO_TYPE;


/**
 * @brief share pins
 * 
 */
typedef enum
{
    ePIN_AS_MMCSD = 0,             ///< share pin as MDAT1, 8 lines
    ePIN_AS_I2S,                ///< share pin as I2S bit[24]:0
    ePIN_AS_PWM0,               ///< share pin as PWM0   
    ePIN_AS_PWM1,               ///< share pin as PWM1
    ePIN_AS_PWM2,               ///< share pin as PWM2
    ePIN_AS_PWM3,               ///< share pin as PWM3
    ePIN_AS_PWM4,               ///< share pin as PWM4
	
    ePIN_AS_SDIO,               ///< share pin as SDIO
    ePIN_AS_UART1,              ///< share pin as UART1
    ePIN_AS_UART2,              ///< share pin as UART2
    ePIN_AS_CAMERA,             ///< share pin as CAMERA
    ePIN_AS_SPI0,               ///< share pin as SPI1 bit[25]:0
    ePIN_AS_SPI1,               ///< share pin as SPI2  bit[26]:1
    ePIN_AS_JTAG,               ///< share pin as JTAG
    ePIN_AS_TWI,                ///< share pin as I2C
    ePIN_AS_MAC,                ///< share pin as Ethernet MAC
    ePIN_AS_OPCLK,

    ePIN_AS_DUMMY

}E_GPIO_PIN_SHARE_CONFIG;

/**
 * @brief gpio_attr
 * 
 */

typedef enum
{
    GPIO_ATTR_IE = 1,   ///<input enable
    GPIO_ATTR_PE,       ///<pullup/pulldown enable
    GPIO_ATTR_SL,       ///<slew rate
    GPIO_ATTR_DS,       ///<drive strength
    GPIO_ATTR_PS        ///<pullup/pulldown selection
}T_GPIO_PIN_ATTR;



typedef struct
{
    E_GPIO_PIN_SHARE_CONFIG func_module;
    unsigned long reg1_bit_mask;
    unsigned long reg1_bit_value;
    unsigned long reg2_bit_mask;
    unsigned long reg2_bit_value;
    unsigned long reg3_bit_mask;
    unsigned long reg3_bit_value;
    unsigned long reg4_bit_mask;
    unsigned long reg4_bit_value;
}
T_SHARE_CFG_FUNC_MODULE;

/**
 * @brief special share pins
 * 
 */
typedef enum
{
    ePIN_AS_PWM1_S0 = 0,               ///< share pin as PWM1 IO4
    ePIN_AS_PWM1_S1,               ///< share pin as PWM1 IO50

    ePIN_AS_PWM3_S0,               ///< share pin as PWM3 IO23
    ePIN_AS_PWM3_S1,               ///< share pin as PWM3 IO57

    ePIN_AS_I2S_S0,                ///< share pin as I2S bit[24]:0
    ePIN_AS_I2S_S1,                ///< share pin as I2S bit[24]:1
    ePIN_AS_SPI0_S0,               ///< share pin as SPI1 bit[25]:0
    ePIN_AS_SPI0_S1,               ///< share pin as SPI1 bit[25]:1
    ePIN_AS_SPI1_S0,               ///< share pin as SPI2  bit[26]:0
    ePIN_AS_SPI1_S1,               ///< share pin as SPI2  bit[26]:1
    ePIN_AS_SDIO_S0,               ///< share pin as SDIO bit[27]:0
    ePIN_AS_SDIO_S1               ///< share pin as SDIO bit[27]:1
}E_SPECIAL_GPIO_PIN_SHARE_CONFIG;

typedef struct
{
	E_SPECIAL_GPIO_PIN_SHARE_CONFIG func_module;
    unsigned long reg1_bit_mask;
    unsigned long reg1_bit_value;
    unsigned long reg2_bit_mask;
    unsigned long reg2_bit_value;
    unsigned long reg3_bit_mask;
    unsigned long reg3_bit_value;
    unsigned long reg4_bit_mask;
    unsigned long reg4_bit_value;
}
T_SPECIAL_SHARE_CFG_FUNC_MODULE;

typedef struct
{
    unsigned char gpio_start;
    unsigned char gpio_end;
    unsigned char rig_num;       ///控制寄存器时，范围0-3
    unsigned long bit_start_mask;     ///相对地址首地址的mask
}
T_SHARE_CFG_FUNC_PULL;

typedef struct
{
    unsigned char gpio_start;     ///IO区间起始数
    unsigned char gpio_end;      ///IO区间结尾数
    unsigned char rig_num;       ///控制寄存器时，范围0-3
    unsigned long bit_start_mask;     ///相对地址首地址的mask
    unsigned char bit_num;          ///控制邋IO口的所需寄存器位数,1or2
    unsigned long valu;          ///对应的bit须被置的值
}
T_SHARE_CFG_GPIO;



#endif

