/**
 * @file 
 * @brief:
 *
 * This file provides 
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  kjp
 * @date 2017-7-22
 * @version 1.0
 */

#include "anyka_types.h"

#include "drv_gpio.h"

#include "dev_drv.h"
#include "device.h"
#include "platform_devices.h"


#ifdef __cpluscplus
extern "C"{
#endif

#define INVALID_GPIO                    0xfe

typedef struct  _PLATFORM_GPIO_MSG 
{
	unsigned int pin;
	unsigned char polarity; 
	
}T_PLATFORM_GPIO_MSG,*T_pPLATFORM_GPIO_MSG;



typedef void (*T_fPLATFORM_GPIO_CALLBACK)(unsigned long *param, unsigned long len);

/**
 * @brief 
 * @author 
 * @date 2017-07-22
 * @param 
 * @return 
 * @retval  < 0 :  failed
 * @retval = 0 : successful
 */
int platform_gpio_init(T_GPIO_INFO *info);


/**
 * @brief 
 * @author 
 * @date 2017-07-22
 * @param 
 * @return 
 * @retval  < 0 :  failed
 * @retval = 0 : successful
 */
int platform_gpio_set(T_GPIO_INFO *info,unsigned char level);


/**
 * @brief 
 * @author 
 * @date 2017-07-22
 * @param 
 * @return 
 * @retval  < 0 :  failed
 * @retval >= 0 :  pin level; 1: high; 0: low;

 */
int platform_gpio_get(T_GPIO_INFO *info);

/**
 * @brief 
 * @author 
 * @date 2017-07-22
 * @param 
 * @return 
 * @retval  < 0 :  failed
 * @retval = 0 : successful
 */
int platform_gpio_int_cb(T_GPIO_INFO *info , T_fPLATFORM_GPIO_CALLBACK callback);



/**
 * @brief 
 * @author 
 * @date 2017-07-22
 * @param 
 * @return 
 * @retval  < 0 :  failed
 * @retval = 0 : successful
 */
int platform_gpio_release(T_GPIO_INFO *info);

#ifdef __cplusplus
}
#endif




