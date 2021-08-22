/**
 * @file drv_gpio.h
 * @brief list gpio operation intefaces.
 *
 * This file define gpio macros and APIs: intial, set gpio, get gpio. etc.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liaozhijun
 * @date 2005-07-24
 * @version 1.0
 *
 */

#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

/** @defgroup GPIO GPIO group
 *  @ingroup Drv_Lib
 */
/*@{*/
/*@}*/

/** @defgroup Arch_gpio Architecture of gpio
 *  @ingroup GPIO
 */
/*@{*/

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
    ePIN_AS_UART0,              ///< share pin as UART1
    ePIN_AS_UART1,              ///< share pin as UART2
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

/**
 * @brief GPIO callback define.
 */
typedef void (*T_fGPIO_CALLBACK)( unsigned long pin, unsigned char polarity );

/**
 * @brief Init gpio.
 * @author  liao_zhijun
 * @date 2010-07-28
 * @return void
 */
void gpio_init( void );

/**
 * @brief Set gpio output level
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] level 0 or 1.
 * @return void
 */
void gpio_set_pin_level( unsigned long pin, unsigned char level );

/**
 * @brief Get gpio input level
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @return unsigned char
 * @retval 1 level high
 * @retval 0 level low;
 */
unsigned char gpio_get_pin_level( unsigned long pin );

/**
 * @brief Set gpio direction
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] dir 0 means input; 1 means output;
 * @return void
 */
void gpio_set_pin_dir( unsigned long pin, unsigned char dir );

/**
 * @brief gpio interrupt control
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] enable 1 means enable interrupt. 0 means disable interrupt.
 * @return void
 */
void gpio_int_control( unsigned long pin, unsigned char enable );

/**
 * @brief disable all gpio pin interrupt
 * @author  liao_zhijun
 * @date 2010-07-28
 * @return void
 */
void gpio_int_disableall(void);

/**
 * @brief restore all gpio pin interrupt
 * @author  liao_zhijun
 * @date 2010-07-28
 * @return void
 */
void gpio_int_restoreall(void);

/**
 * @brief set gpio interrupt polarity.
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] polarity 1 means active high interrupt. 0 means active low interrupt.
 * @return void
 */
void gpio_set_int_p( unsigned long pin, unsigned char polarity );

/**
 * @brief Register one gpio interrupt callback function.
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] polarity 1 means active high interrupt. 0 means active low interrupt.
 * @param[in] enable Initial interrupt state--enable or disable.
 * @param[in] callback gpio interrupt callback function.
 * @return void
 */
void gpio_register_int_callback( unsigned long pin, unsigned char polarity, unsigned char enable, T_fGPIO_CALLBACK callback );

/**
 * @brief gpio pull-up resistance configuration function
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] enable 1 means enable pull up. 0 means disable pull up.
 * @return bool
 * @retval AK_TURE configuration successful
 * @retval false configuration failed
 */
bool gpio_set_pull_up_r(unsigned long pin, bool enable);

/**
 * @brief gpio pull-down resistance configuration function
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] enable 1 means enable pull down. 0 means disable pull down.
 * @return bool
 * @retval AK_TURE configuration successful
 * @retval false configuration failed
 */
bool gpio_set_pull_down_r(unsigned long pin, bool enable);

/**
 * @brief get gpio wakeup pin bit
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @return unsigned char
 * @retval gpio pin bit in wakeup register
 */
unsigned char  get_wGpio_Bit(unsigned char pin);

/**
 * @brief Set gpio wake up polarity.
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] pin gpio pin ID.
 * @param[in] polarity 1 means high level. 0 means low level.
 * @return void
 */
void gpio_set_wakeup_p(unsigned long pin, bool polarity);

/**
 * @brief set gpio share pin as gpio 
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param pin [in]  gpio pin ID
 * @return bool
 * @retval true set successfully
 * @retval false fail to set
 */
bool  gpio_set_pin_as_gpio(unsigned long pin);

/**
 * @brief set gpio pin group as specified module used
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] PinCfg enum data. the specified module
 * @return bool
 * @retval AK_TURE setting successful
 * @retval false setting failed
*/
bool gpio_pin_group_cfg(E_GPIO_PIN_SHARE_CONFIG PinCfg);

/**
 * @brief set gpio pin attribute as IE, PE, etc.
 * @note only available for AK3810
 * @author tangjianlong
 * @date 2008-01-15
 * @param[in] pin the pin ID to set
 * @param[in] attr the atrribute to set
 * @param[in] enable enable the attribute or not
 * @return bool
 * @retval AK_TURE setting successful
 * @retval false setting failed
 */
bool gpio_set_pin_attribute(unsigned long pin, T_GPIO_PIN_ATTR attr, bool enable);

/**
 * @brief set specified module  gpio pin group different config
 * @author  jiankui
 * @date 2016-08-30
 * @param[in] PinCfg enum data. the specified module
  * @param[in] share_num special module config numer
 * @return bool
 * @retval AK_TURE setting successful
 * @retval false setting failed
*/
bool gpio_share_pin_set(E_GPIO_PIN_SHARE_CONFIG PinCfg, unsigned char share_num);
/*@}*/

#endif //#ifndef __ARCH_GPIO_H__

