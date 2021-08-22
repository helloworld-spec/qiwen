/**@file hal_usb_s_state.h
 * @brief provde usb common operations.
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */

#ifndef __HAL_USB_SLAVE_STATE_H__
#define __HAL_USB_SLAVE_STATE_H__

#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup USB_state USB_state group
 *  @ingroup USB
 */
/*@{*/

//********************************************************************
#define USB_NOTUSE          0   ///<usb close
#define USB_OK              1   ///<usb config ok,can transmit data
#define USB_ERROR           2   ///<usb error
#define USB_SUSPEND         3   ///<usb suspend by pc,can't to use
#define USB_CONFIG          4   ///<usb config by pc
#define USB_PRE_SUSPEND     5
#define USB_START_STOP      6
#define USB_TEST_UNIT_STOP  7

//********************************************************************

#define USB_DISK                (1<<0)   ///<usb disk type
#define USB_CAMERA              (1<<1)   ///<usb camera type
#define USB_ANYKA               (1<<2)   ///<usb anyka type
#define USB_DEBUG               (1<<3)   ///<usb debug type
#define USB_CDC                 (1<<4)   //usb cdc type
#define USB_UVC                 (1<<5)   //usb cdc type

#define USB_MODE_20             (1<<8)   ///<usb high speed
#define USB_MODE_11             (1<<9)   ///<usb full speed

/**
 * @brief   get usb slave stage.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @return  unsigned char
 * @retval  USB_OK usb config ok,can transmit data
 * @retval  USB_ERROR usb error
 * @retval  USB_SUSPEND usb suspend by pc,can't to use
 * @retval  USB_NOTUSE usb close
 * @retval  USB_CONFIG usb config by pc
 */
unsigned char usb_slave_getstate(void);

/**
 * @brief   set usb slave stage.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param[in] stage  unsigned char.
 * @return  void
 */
void usb_slave_set_state(unsigned char stage);


/**
 * @brief   detect whether usb cable is inserted or not
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @return  bool
 * @retval  true usb cable in
 * @retval  false usb cable not in
 */
bool usb_detect(void);


//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif
