/**
 * @file usb_host_drv.h
 * @brief  frameworks of usb driver.
 *
 * This file describe driver of usb in host mode.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-06-30
 * @version 1.0
 */

#ifndef __USB_HOST_DRV_H__
#define __USB_HOST_DRV_H__

#include "anyka_types.h"
#include "anyka_cpu.h"

#include "hal_usb_std.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup Usb_Host Usb Host group
 *	@ingroup Drv_Lib
 */
/*@{*/
/*@}*/

/** @defgroup Host_Driver USB Host Driver
 *  @ingroup Usb_Host
 */
/*@{*/

#define USB_HOST_OUT_INDEX EP2_INDEX
#define USB_HOST_IN_INDEX EP3_INDEX

typedef enum
{
    EP0_INDEX = 0,  ///< EP0
    EP1_INDEX = 1,  ///< EP1
    EP2_INDEX = 2,  ///< EP2
    EP3_INDEX = 3   ///< EP3
}EP_INDEX;

typedef enum
{
    USB_HOST_CONNECT = 0,
    USB_HOST_DISCONNECT,
    USB_HOST_UNDEFINE
}
E_USB_HOST_COMMON_INTR;

#define USB_HOST_TRANS_COMPLETE 0
#define USB_HOST_TRANS_ERROR    1


extern unsigned char g_UsbBulkinIndex;
extern unsigned char g_UsbBulkoutIndex;


/**
 *	usb host common usb interrupt call back
 */
typedef void (*T_fUHOST_COMMON_INTR_CALLBACK)(void);

/**
 *	usb host transfer end call back
 */
typedef void (*T_fUHOST_TRANS_CALLBACK)(unsigned char trans_state, unsigned long trans_len);


/**
 * @brief   enable usb host driver.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param  mode [in] unsigned long  full speed or high speed
 * @return  void
 */
void usb_host_device_enable(unsigned long mode);

/**
 * @brief   disable usb host driver.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  void
 */
void usb_host_device_disable(void);

/**
 * @brief   set callback func for common interrupt
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param intr_type interrupt type
 * @param callback callback function
 * @return  void
 */
void usb_host_set_common_intr_callback(E_USB_HOST_COMMON_INTR intr_type, T_fUHOST_COMMON_INTR_CALLBACK callback);

/**
 * @brief   set callback func for transfer
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param ctrl_cbk callback function for control transfer
 * @param trans_cbk callback function for other transfer
 * @return  void
 */
void usb_host_set_trans_callback(T_fUHOST_TRANS_CALLBACK ctrl_cbk, T_fUHOST_TRANS_CALLBACK trans_cbk);

/**
 * @brief   set faddr to the new address send to device.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param address  [in]  usb device address.
 * @return  void
 */
void usb_host_set_address(unsigned char address);


/**
 * @brief   config usb host endpoint through device ep descriptor.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param  ep [in]  ep description.
 * @return  void
 */
void usb_host_set_ep(T_USB_ENDPOINT_DESCRIPTOR ep);

/**
 * @brief   open or close sof interrupt
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param  enable [in]  open sof interrupt or not.
 * @return  void
 */
void usb_host_sof_intr(bool enable);


/**
 * @brief   send reset signal to device.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  void
 */
void usb_host_reset(void);


/**
   @brief   sent suspend signal
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  void
 */
void usb_host_suspend(void);

/**
   @brief   sent resume signal
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  void
 */
void usb_host_resume(void);


/**
   @brief   start control tranfer
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @param dev_req [in] device request
 * @param data [in/out] data buffer
 * @param len [in] buffer length
 * @return  bool
 */
bool usb_host_ctrl_tranfer(T_UsbDevReq dev_req, unsigned char *data, unsigned long len);


/**
   * @brief   start bulk in tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data [out]  usb data buffer.
   * @param  len [in]  length
   * @return  unsigned long acctual read bytes
 */
bool usb_host_bulk_in(unsigned char *data, unsigned long len);

/**
   * @brief   start bulk out tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data [in]  usb data buffer.
   * @param  len [in] len length
   * @return  unsigned long acctual read bytes
 */
bool usb_host_bulk_out(unsigned char *data, unsigned long len);

/**
   * @brief   start iso in tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data  [out] usb data buffer.
   * @param  packet_num [in]  number of packet to receive
   * @return  unsigned long acctual read bytes
 */
bool usb_host_iso_in(unsigned char *data, unsigned long packet_num);

/**
   * @brief   start iso out tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data  [in] usb data buffer.
   * @param  len [in]  length
   * @return  unsigned long acctual read bytes
 */
bool usb_host_iso_out(unsigned char *data, unsigned long len);

/**
   * @brief   start interrupt in tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data [out]  usb data buffer.
   * @param  len [in]  length
   * @return  unsigned long acctual read bytes
 */
bool usb_host_intr_in(unsigned char *data, unsigned long len);

/**
   * @brief   start interrupt out tranfer
   *
   * @author  liao_zhijun
   * @date    2010-06-30
   * @param  data [in]  usb data buffer.
   * @param  len [in]  length
   * @return  unsigned long acctual read bytes
 */
bool usb_host_intr_out(unsigned char *data, unsigned long len);

/**
 * @brief   reset data toggle.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  void
 */
void usb_host_clear_data_toggle(unsigned char EP_index);
/**
 * @brief   flush usb fifo.
 *
 * @author  liao_zhijun
 * @date    2010-06-30
 * @return  void
 */
void usb_host_flush_fifo(unsigned char EP_index);



//********************************************************************
/*@}*/

#ifdef __cplusplus
}
#endif

#endif
