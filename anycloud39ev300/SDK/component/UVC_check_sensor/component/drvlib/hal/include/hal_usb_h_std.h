/**
 * @filename usb_slave_std.h
 * @brief: AK3223M standard protocol of usb.
 *
 * This file describe standard protocol of usb driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-13
 * @version 1.0
 * @ref
 */

#ifndef __USB_HOST_STD_H__
#define __USB_HOST_STD_H__

#include "anyka_types.h"
#include "hal_usb_std.h"
#include "hal_usb_h_interrupt.h"
#include "hal_usb_h_disk.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup Fwl_USB_H_STD Framework USB_H Std Interface
 *  @ingroup Framework
 */
/*@{*/
//********************************************************************

#define USB_MODE_20             (1<<8)   ///<usb high speed
#define USB_MODE_11             (1<<9)   ///<usb full speed



#define MAX_ENDPOINT_NUM            5

struct S_USB_CONNECT_DEVICE
{
    bool                          dev_config;
    T_USB_HOST_INTERRUPT            host_int;
    //unsigned long                           *dev_function;
    //T_USB_HOST_DISK_DES             dev_function[MAX_LUN_NUM];
    T_USB_DEVICE_DESCRIPTOR         dev_devdescritor;
    T_USB_CONFIGURATION_DESCRIPTOR  dev_cfgdescritor;
    T_USB_INTERFACE_DESCRIPTOR      dev_ifadescritor;
    T_USB_ENDPOINT_DESCRIPTOR       dev_epdescritor[MAX_ENDPOINT_NUM];
    volatile bool                 usb_connect_flag;
    volatile bool                 usb_disconnect_flag;
   // F_USBHostDisConnect             usb_disconnect_callback;
    
   // F_USBHostEPCallBack				usb_in_callback;
   // F_USBHostEPCallBack				usb_in_dma_callback;
   // F_USBHostEPCallBack				usb_out_callback;
   // F_USBHostEPCallBack				usb_out_dma_callback;
};

typedef struct S_USB_CONNECT_DEVICE T_USB_CONNECT_DEVICE;

extern T_USB_CONNECT_DEVICE *Usb_Host_Standard;

bool usb_host_std_get_status(void);
bool usb_host_std_clear_feature(unsigned char EP_Index);
bool usb_host_std_set_feature(void);
bool usb_host_std_set_address(unsigned long dev_addr);
signed long usb_host_std_get_descriptor(unsigned char desc_type, unsigned char desc_index, unsigned long lang_id, unsigned char data[], unsigned long len);
bool usb_host_std_set_descriptor(void);
bool usb_host_std_get_configuration(void);
bool usb_host_std_set_configuration(unsigned char conf_val);
bool usb_host_std_get_interface(void);
bool usb_host_std_set_interface(void);


//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif
