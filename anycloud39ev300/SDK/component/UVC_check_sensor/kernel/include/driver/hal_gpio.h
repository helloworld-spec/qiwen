/**@file hal_gpio.h
 * @brief gpio pin config
 
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author guoshaofeng
 * @date 2007-12-19
 * @version 1.0
 */

#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

#include "drv_gpio.h"

/** @defgroup Hardware Abstract Layer of gpio
 *  @ingroup GPIO
 */
/*@{*/

typedef enum 
{
    ePullUpEn = 0,
    ePullUpDis,
    ePullDownEn,
    ePullDownDis
}T_ePIN_PULL_UP_DOWN;

typedef struct
{
    unsigned char pinNum;
    T_ePIN_PULL_UP_DOWN pull;
    unsigned char pinDir;
    unsigned char pinDefaultLevel;
    unsigned char pinActiveLevel;
}T_GPIO_SET;


#define GPIO_END_FLAG  0xff

typedef void (*T_fDRV_CALLBACK)(unsigned long *param, unsigned long len);


/**
 * @brief gpio pin config init
 * @author guoshaofeng
 * @date 2007-12-19
 * @param [in] pGpioSetting config array pointer
 * @return void
 */
void gpio_pin_config_init(T_GPIO_SET *pGpioSetting);

/**
 * @brief gpio_pin_get_ActiveLevel
 * @author guoshaofeng
 * @date 2007-12-19
 * @param[in] pin gpio pin ID.
 * @return unsigned char pin level
 * @retval 1 high
 * @retval 0 low
 * @retval 0xff invalid_level
 */
unsigned char gpio_pin_get_ActiveLevel(unsigned char pin);


/**
 * @brief get which pin wakeup  
 * @author  liao_zhijun
 * @date 2010-07-28
 * @param[in] reason wake up status.
 * @return unsigned char
 * @retval gpio pin
 */
unsigned char gpio_get_wakeup_pin(unsigned long reason);

/**
 * @brief  gpio_intr_enable
 * @author wumingjin
 * @date   2017-05-11
 * @param[in] pin
 * @param[in] mode  0 level trigger, 1 edge trigger
 * @param[in] polarity 0 low /falling, 1 high/rising
 * @param[in] callback
 * @return bool
 */
bool gpio_intr_enable ( unsigned long pin, unsigned char mode, unsigned char polarity, T_fDRV_CALLBACK callback );

/**
 * @brief  gpio_intr_disable
 * @author wumingjin
 * @date   2017-05-11
 * @param[in] pin
 * @return bool
 */
bool gpio_intr_disable ( unsigned long pin);

/*@}*/
#endif
