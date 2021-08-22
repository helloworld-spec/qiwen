/**
 * @filename hal_usb_s_cdc.h
 * @brief: how to use usb cdc.
 *
 * This file describe frameworks of usb cdc driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 * @ref
 */

#ifndef __HAL_USB_S_CDC_H__
#define __HAL_USB_S_CDC_H__

#include "anyka_types.h"
#include "hal_usb_s_state.h"


#ifdef __cplusplus
extern "C" {
#endif


//********************************************************************

typedef void (*T_fUSBCDC_RECEIVECALLBACK)(void);
typedef void (*T_fUSBCDC_SENDFINISHCALLBACK)(void);



/**
 * @brief   usb cdc init.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] void.
 * @return  void
 */
void usbcdc_init(void); 

/**
 * @brief   usb slave enable cdc device function.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] void.
 * @return  void
 */
bool usbcdc_enable(void);

/**
 * @brief   usb slave disnable cdc device function.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] void.
 * @return  void
 */
void usbcdc_disable(void);


/**
 * @brief   usb slave cdc set callback function.
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] T_fUSBCDC_RECEIVECALLBACK : notify to receive data.
                 T_fUSBCDC_SENDFINISHCALLBACK : notify send finish   
 * @return  void
 */
void usbcdc_set_callback(T_fUSBCDC_RECEIVECALLBACK receive_func, T_fUSBCDC_SENDFINISHCALLBACK sendfinish_func);


/**
 * @brief set pool to receive data
 * 
 * @author Qinxiaojun
 * @date 2007-9-17
 * @param[in] uart_id: UART ID
              unsigned char *pool: buffer to recieve data,as big as you can
              unsigned long poollength : buffer length
 *
 * @return void
 * @remarks the data received from uart will be stored in the pool waiting to be fetched by
 *  uart_read().
 */
void usbcdc_set_datapool(unsigned char *pool, unsigned long poollength);

/**
 * @brief   write data to usb
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] unsigned char *data: data to write
                 unsigned long data_len: data length.
 * @return  -1: write error, try again
            other: actual data length to write
 */
signed long usbcdc_write(unsigned char *data, unsigned long data_len);

/**
 * @brief   read data from usb
 *
 * @author  \b zhaojiahuan
 * @date    2006-11-04
 * @param   [in] unsigned char *data: temp read data
                 unsigned long data_len: data length.
 * @return  -1: read error, try again
            other: actual data length from read
 */

signed long usbcdc_read(unsigned char *data, unsigned long data_len);



//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif
