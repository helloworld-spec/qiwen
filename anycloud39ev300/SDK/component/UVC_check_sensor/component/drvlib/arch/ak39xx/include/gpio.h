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

#include "drv_gpio.h"

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

unsigned long gpio_pin_check(unsigned long pin);
/**
 * @brief get gpio share pin as uart
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param uart_id [in]  uart id
 * @param clk_pin [in]  clk pin
 * @param data_pin [in]  data pin
 * @return bool
 * @retval true get successfully
 * @retval false fail to get
 */
bool gpio_get_uart_pin(T_UART_ID uart_id, unsigned long* clk_pin, unsigned long* data_pin);

#endif //#ifndef __GPIO_H__

