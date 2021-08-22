/**@file hal_usb_s_anyka.h
 * @brief provide operations of how to use usb device of anyka.
 *
 * This file describe frameworks of anyka device.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */

#ifndef __HAL_USB_S_ANYKA_H__
#define __HAL_USB_S_ANYKA_H__

#include "anyka_types.h"
#include "hal_usb_s_state.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup USB USB group
 *  @ingroup Drv_Lib
 */
/*@{*/
/*@}*/

/** @defgroup USB_anyka USB_anyka group
 *  @ingroup USB
 */
/*@{*/

typedef void (*T_fUSBANYKA_RECEIVECALLBACK)(void);
typedef void (*T_fUSBANYKA_RECEIVEOKCALLBACK)(void);
typedef void (*T_fUSBANYKA_SENDFINISHCALLBACK)(void);


//********************************************************************

/**
 * @brief   usb anyka init.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @return  void
 */
void usbanyka_init(void); 

/**
 * @brief   enable anyka device in usb slave mode
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @return  bool
 * @retval  true means sucessful
 */
bool usbanyka_enable(void);

/**
 * @brief   disable anyka device in usb slave mode
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @return  void
 */
void usbanyka_disable(void);

/**
 * @brief   usb slave anyka set callback function.
 *
 * @author  zhaojiahuan
 * @date    2006-11-04
 * @param[in] receive_func T_fUSBCDC_RECEIVECALLBACK : notify to receive data.
 * @param[in] receiveok_func T_fUSBANYKA_RECEIVEOKCALLBACK : notify receive data ok.
 * @param[in] sendfinish_func T_fUSBCDC_SENDFINISHCALLBACK : notify send finish   
 * @return  void
 */
void usbanyka_set_callback(T_fUSBANYKA_RECEIVECALLBACK receive_func, T_fUSBANYKA_RECEIVEOKCALLBACK receiveok_func, T_fUSBANYKA_SENDFINISHCALLBACK sendfinish_func);

/**
 * @brief   write data to usb
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] data data to write
 * @param   [in] data_len data length.
 * @return  signed long
 * @retval  -1 write error, try again
 * @retval  other actual data length to write
 */
signed long usbanyka_write(unsigned char *data, unsigned long data_len);

/**
 * @brief   read data from usb
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [out] *data temp read data
 * @param   [in] data_len data length.
 * @return  signed long
 * @retval  -1 read error, try again
 * @retval  other actual data length from read
 */
signed long usbanyka_read(unsigned char *data, unsigned long data_len);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif
