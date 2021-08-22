/**@file hal_usb_s_debug.h
 * @brief provide operations of how to use usb device of debug.
 *
 * This file describe frameworks of anyka device.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */

#ifndef __HAL_USB_SLAVE_DEBUG_H__
#define __HAL_USB_SLAVE_DEBUG_H__

#include "anyka_types.h"
#include "hal_usb_s_state.h"


#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup USB_debug USB_debug group
 *  @ingroup USB
 */
/*@{*/

//********************************************************************
/**
 * @brief   enable usb debug in usb slave mode
 *
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool usbdebug_enable(void);

/**
 * @brief   disable usb debug in usb slave mode
 *
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @return  void
 */
void usbdebug_disable(void);

/**
 * @brief print to usb
 *
 * @author zhaojiahuan
 * @date   2006-11-04
 * @param[in] str string to be print
 * @param[in] len string length
 * @return  void
 */
void usbdebug_pp_printf(unsigned char *str, unsigned long len);


/**
 * @brief   get string from usb
 *
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @param[out] str buffer to store input string 
 * @param[in] len buffer size
 * @return  unsigned long
 */
unsigned long usbdebug_getstring(unsigned char *str, unsigned long len);


//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif
