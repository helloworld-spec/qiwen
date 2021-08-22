/**
 * @file usb_slave_drv.h
 * @brief: frameworks of usb driver.
 *
 * This file describe driver of usb in slave mode.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-06-30
 * @version 1.0
 */

#ifndef __USB_SLAVE_DRV_H__
#define __USB_SLAVE_DRV_H__

#include "anyka_types.h"
#include "anyka_cpu.h"

#include "hal_usb_std.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USB_EP_IN_TYPE            0x1
#define USB_EP_OUT_TYPE           0x0

#define EP0_INTR            0x000000001
#define EP1_INTR            0x000000002
#define EP2_INTR            0x000000004
#define EP3_INTR            0x000000008
#define EP4_INTR            0x000000010
#define EP1_DMA_INTR        0x000000020
#define EP2_DMA_INTR        0x000000040
#define EP3_DMA_INTR        0x000000080
#define EP4_DMA_INTR        0x000000100
#define USB_INTR            0x000000200
#define EP_UNKNOWN          0x0

#define USB_DMA_SUPPORT           0x01
#define USB_DMA_UNSUPPORT         0x0

/**
 * @brief EP Index
 
 *   define ep index
 */
typedef enum
{
    EP0_INDEX = 0,  ///< EP0
    EP1_INDEX = 1,  ///< EP1
    EP2_INDEX = 2,  ///< EP2
    EP3_INDEX = 3,   ///< EP3
    EP4_INDEX = 4,  ///< EP4
    EP5_INDEX = 5  ///< EP5

}EP_INDEX;

/**
 * @brief Control Transfer Stage
 
 *   define control transfer stage
 */
typedef enum
{
    CTRL_STAGE_IDLE = 0,    ///< idle stage
    CTRL_STAGE_SETUP,       ///< setup stage
    CTRL_STAGE_DATA_IN,     ///< data in stage
    CTRL_STAGE_DATA_OUT,    ///< data out stage
    CTRL_STAGE_STATUS       ///< status stage
}
E_CTRL_TRANS_STAGE;

/**
 * @brief Request Type
 
 *   define control transfer request type
 */
#define REQUEST_STANDARD    0   ///< standard request
#define REQUEST_CLASS       1   ///< class request
#define REQUEST_VENDOR      2   ///< vendor request


/**
 * @brief Control Tranfer Struct
 
 *   define control transfer struct
 */
typedef struct tagCONTROL_TRANS
{
    E_CTRL_TRANS_STAGE stage;   ///< stage
    T_UsbDevReq dev_req;        ///< request
    unsigned char *buffer;               ///< buffer
    unsigned long buf_len;              ///< buffer length
    unsigned long data_len;             ///< data length
}
T_CONTROL_TRANS;

/**
 *      rx notify callback handler
 */
typedef void (*T_fUSB_NOTIFY_RX_CALLBACK)(void);

/**
 *      rx finish callback handler
 */
typedef void (*T_fUSB_RX_FINISH_CALLBACK)(void);

/**
 *      tx finish callback handler
 */
typedef void (*T_fUSB_TX_FINISH_CALLBACK)(void);

/**
 *      reset event callback handler
 */
typedef void (*T_fUSB_RESET_CALLBACK)(unsigned long mode);

/**
 *      suspend event callback handler
 */
typedef void (*T_fUSB_SUSPEND_CALLBACK)(void);

/**
 *      resume event callback handler
 */
typedef void (*T_fUSB_RESUME_CALLBACK)(void);

/**
 *      config ok event callback handler
 */
typedef void (*T_fUSB_CONFIGOK_CALLBACK)(void);

/**
 *      control tranfer callback handler
 */
typedef bool (*T_fUSB_CONTROL_CALLBACK)(T_CONTROL_TRANS *pTrans);

//********************************************************************
/**
 * @brief   enable usb slave driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param mode  [in] unsigned long usb mode
 * @return  void
 */
void usb_slave_device_enable(unsigned long mode);

/**
 * @brief   disable usb slave driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  void
 */
void usb_slave_device_disable(void);

/**
 * @brief   initialize usb slave global variables, and set buffer for control tranfer
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param buffer [IN] buffer to be set for control transfer
 * @param buf_len [IN] buffer length
 * @return  bool
 * @retval true init successfully
 * @retval false init fail
 */
bool usb_slave_init(unsigned char *buffer, unsigned long buf_len);

/**
 * @brief  free usb slave global variables,L2 buffer
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  void
 */
void usb_slave_free(void);

/**
 * @brief   set control transfer call back function
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param type [IN] request type, must be one of (REQUEST_STANDARD, REQUEST_CLASS, REQUEST_VENDOR)
 * @param callback [In] callback function
 * @return  bool
 * @retval true callback function set successfully
 * @retval false fail to set callback function
 */
bool usb_slave_set_ctrl_callback(unsigned char type, T_fUSB_CONTROL_CALLBACK callback);

/**
 * @brief   initialize usb slave register.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param reset_callback [IN] callback function for reset interrupt
 * @param suspend_callback [IN] callback function for suspend interrupt
 * @param resume_callback [IN] callback function for resume interrupt
 * @param configok_callback [IN] callback function for config ok event
 * @return  bool
 * @retval true callback function set successfully
 * @retval false fail to set callback function
 */
bool usb_slave_set_callback(T_fUSB_RESET_CALLBACK reset_callback, T_fUSB_SUSPEND_CALLBACK suspend_callback, T_fUSB_RESUME_CALLBACK resume_callback, T_fUSB_CONFIGOK_CALLBACK configok_callback);


/**
 * @brief   Register a callback function to notify tx send data finish.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in] EP_TX_INDEX EP_index: EP1~EP6, cannot be EP0
 * @param  callback_func [in]  T_fUSB_TX_FINISH_CALLBACK can be null
 * @return  bool
 * @retval true callback function set successfully
 * @retval false fail to set callback function
 */
bool usb_slave_set_tx_callback(EP_INDEX EP_index, T_fUSB_TX_FINISH_CALLBACK callback_func);


/**
 * @brief   Register a callback function to notify rx receive data finish and rx have data.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in] EP_TX_INDEX EP_index: EP1~EP6, cannot be EP0
 * @param  notify_rx [in] rx notify callbakc function, can be null
 * @param  rx_finish [in] rx finish callbakc function, can be null
 * @return  bool
 * @retval true callback function set successfully
 * @retval false fail to set callback function
 */
bool usb_slave_set_rx_callback(EP_INDEX EP_index, T_fUSB_NOTIFY_RX_CALLBACK notify_rx, T_fUSB_RX_FINISH_CALLBACK rx_finish);

/**
 * @brief   write usb data with end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in]  usb end point.
 * @param  data [in] usb data buffer.
 * @param  count [in] count to be send.
 * @return  unsigned long data in count
 */
unsigned long usb_slave_data_in(EP_INDEX EP_index, unsigned char *data, unsigned long count);

/**
 * @brief   read usb data with end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  EP_index [in]  usb end point.
 * @param  pBuf [out] usb data buffer.
 * @param  count [in] count to be read
 * @return unsigned long data out count
 */
unsigned long  usb_slave_data_out(EP_INDEX EP_index, void *pBuf, unsigned long count);

/**
 * @brief   write usb address.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  address [in]  usb device address.
 * @return  void
 */
void usb_slave_set_address(unsigned char address);

/**
 * @brief  stall ep
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  void
 */
void usb_slave_ep_stall(unsigned char EP_index);

/**
 * @brief  clear stall
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  void
 */
void usb_slave_ep_clr_stall(unsigned char EP_index);


/**
 * @brief   read data count of usb end point.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index [in] usb end point.
 * @param cnt  [out] cnt data count.
 * @return  void
 */
void usb_slave_read_ep_cnt(unsigned char EP_index, unsigned long *cnt);


/**
 * @brief   set usb controller to enter test mode
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  testmode [in] unsigned char test mode, it can be one of the following value: 
 *
 *        Test_J                 0x1
 *
 *        Test_K                 0x2
 *
 *        Test_SE0_NAK       0x3
 *
 *        Test_Packet          0x4
 *
 *        Test_Force_Enable  0x5
 *
 * @return  void
 */
void usb_slave_enter_testmode(unsigned char testmode);

/**
 * @brief   get ep status.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @return  void
 */
unsigned short usb_slave_get_ep_status(unsigned char EP_Index);

/**
 * @brief   set ep status.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param EP_index  [in]  usb end point.
 * @param bStall  [in]  stall or not.
 * @return  void
 */
void usb_slave_set_ep_status(unsigned char EP_Index, bool bStall);

//********************************************************************

#ifdef __cplusplus
}
#endif

#endif
