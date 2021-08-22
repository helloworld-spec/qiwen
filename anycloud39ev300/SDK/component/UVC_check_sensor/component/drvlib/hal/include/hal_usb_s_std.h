/**
 * @filename usb_slave_std.h
 * @brief: AK3223M standard protocol of usb.
 *
 * This file describe standard protocol of usb driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-13
 * @version 1.0
 */

#ifndef __USB_SLAVE_STD_H__
#define __USB_SLAVE_STD_H__

#include "anyka_types.h"
#include "hal_usb_std.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup Fwl_USB_S_STD Framework USB_S STD Interface
 *  @ingroup Framework
 */
/*@{*/
//********************************************************************
#define DEF_DESC_LEN              (INTERFACE_DESC_LEN + CONFIGURE_DESC_LEN + ENDPOINT_DESC_LEN * ENDPOINT_NUMBER)               //9 + 9 + 3 * 7   : 3 - 3 end point
//********************************************************************
/** @brief Struct of Descriptor
 *
*/

struct S_USB_CONFIG_ALL
{
    T_USB_CONFIGURATION_DESCRIPTOR  cf_desc;    //Configure Descriptor
    T_USB_INTERFACE_DESCRIPTOR      if_desc;    //Interface Descriptor
    T_USB_ENDPOINT_DESCRIPTOR       ep1;        //ENDPOINT Descriptor
    T_USB_ENDPOINT_DESCRIPTOR       ep2;        //ENDPOINT Descriptor
    T_USB_ENDPOINT_DESCRIPTOR       ep3;        //ENDPOINT Descriptor
};

typedef struct S_USB_CONFIG_ALL T_USB_CONFIG_ALL;

struct S_USB_CONFIG
{
    unsigned char    *cf_desc;       //point of Configure Descriptor
    unsigned char    *if_desc;       //point of Interface Descriptor
    unsigned char    *ep1;           //point of ENDPOINT Descriptor
    unsigned char    *ep2;           //point of ENDPOINT Descriptor
    unsigned char    *ep3;           //point of ENDPOINT Descriptor
};

typedef struct S_USB_CONFIG T_USB_CONFIG;
typedef void (*T_fUSB_RESET)(unsigned long mode);
typedef void (*T_fUSB_SUSPEND)(void);                   
typedef void (*T_fUSB_RESUME)(void);                    
typedef void (*T_fUSB_STANDARD_REQ)(T_UsbDevReq dev_req); 
typedef void (*T_fUSB_VENDOR_REQ)(T_UsbDevReq dev_req);   
typedef void (*T_fUSB_CLASS_REQ)(T_UsbDevReq dev_req);


struct S_USB_SLAVE_STANDARD{
    unsigned long   Usb_Device_Type;                            //usb device type
    unsigned char    *Device_Descriptor;                         //device descriptor
    T_USB_CONFIG    Device_Config;                      //device config descriptor
    unsigned char    *Device_String;                             //device string descriptor
    unsigned char    *Buffer;                                    //device data buffer
    unsigned long    buf_len;                                    //device data buffer length
    volatile unsigned char    Device_ConfigVal;                           //device configure value
    volatile unsigned char    Device_Address;                             //device address
    void  (*usb_reset)(unsigned long mode);                       //unusual usb reset handle
    void  (*usb_suspend)(void);                     //unusual usb suspend handle
    void  (*usb_resume)(void);                      //unusual usb resume handle
    
    unsigned char *(*usb_get_device_descriptor)(unsigned long *count);               //get device descriptor
    unsigned char *(*usb_get_config_descriptor)(unsigned long *count);               //get config descriptor
    unsigned char *(*usb_get_string_descriptor)(unsigned char index, unsigned long *count);     //get string descriptor
    unsigned char *(*usb_get_device_qualifier_descriptor)(unsigned long *count);     //get device qualifier descriptor
    unsigned char *(*usb_get_other_speed_config_descriptor)(unsigned long *count);      //get other speed config
};

typedef struct S_USB_SLAVE_STANDARD T_USB_SLAVE_STANDARD;
typedef struct S_USB_SLAVE_STANDARD* TP_USB_SLAVE_STANDARD;

extern T_USB_SLAVE_STANDARD Usb_Slave_Standard;

/**
 * @brief   change the clear stall condition in clear feature
 *
 * @author  liao_zhijun
 * @date    2010-07027
 * @param bSet [in]: clear stall in clear feature or not 
 * @return  void
 */
void usb_slave_std_hard_stall(bool bSet);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif
