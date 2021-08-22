/**
 * @filename hal_usb_host_h_std.c
 * @brief: standard protocol of usb host.
 *
 * This file describe standard protocol of usb driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-07-23
 * @version 1.0
 * @ref
 */

#ifdef OS_ANYKA

#include "hal_usb_h_std.h"
#include "usb_bus_drv.h"
#include "usb_host_drv.h"

/**
 * @brief  fill urb struct for standard request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pURB [out]: pointer to urb struct
 * @param pDevReq [in]: pointer to setup packet data
 * @param data [in]: address of data to be transfer
 * @param data_len [in]: data length
 * @return  void
 */
static void fill_urb(T_URB *pURB, T_UsbDevReq *pDevReq, unsigned char *data, unsigned long data_len)
{
    memset(pURB, 0, sizeof(T_URB));
    memcpy(&pURB->dev_req, pDevReq, sizeof(T_UsbDevReq));

    pURB->trans_type = TRANS_CTRL;

    if(pDevReq->bmRequestType | USB_STD_DIR_DEV2HOST)
    {
        pURB->trans_dir = TRANS_DATA_IN;
    }
    else
    {
        pURB->trans_dir = TRANS_DATA_OUT;
    }
    
    pURB->data = data;
    pURB->buffer_len = data_len;
    pURB->data_len = data_len;

    pURB->timeout = URB_MAX_WAIT_TIME;
}

//********************************************************************
/**
 * @brief  get status request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  bool
 */
bool usb_host_std_get_status(void)
{
    return true;
}

//********************************************************************
/**
 * @brief  clear feature request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param EP_Index [in]: which ep need to clear feature
 * @return  bool
 */
bool usb_host_std_clear_feature(unsigned char EP_Index)
{
    T_URB urb;
    T_URB_HANDLE hURB = NULL;
    T_UsbDevReq dev_req;

    dev_req.bmRequestType = USB_STD_DIR_HOST2DEV | USB_STD_REQTYPE_STD | USB_STD_REC_ENDPOINT;
    dev_req.bRequest = USB_STD_CLEARFEATURE;
    dev_req.wValue = 0;
    dev_req.wIndex = EP_Index;
    dev_req.wLength = 0;

    //fill urb struct
    fill_urb(&urb, &dev_req, NULL, 0);

    //commit urb
    hURB = usb_bus_commit_urb(&urb);
    if(NULL == hURB)
    {
        return false;
    }

    //waiting for urb completion
    if(usb_bus_wait_completion(hURB) < 0)
    {
        return false;
    }
    if (EP_Index & 0x80)
    {
        usb_host_clear_data_toggle(EP3_INDEX);
    }
    else
    {
        usb_host_clear_data_toggle(EP2_INDEX);
    }
    return true;
}

//********************************************************************
/**
 * @brief  set feature request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  bool
 */
bool usb_host_std_set_feature(void)
{
    return true;
}

//********************************************************************
/**
 * @brief  set  address request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param dev_addr [in]: address to be set for device
 * @return  bool
 */
bool usb_host_std_set_address(unsigned long dev_addr)
{
    T_URB urb;
    T_URB_HANDLE hURB = NULL;
    T_UsbDevReq dev_req;

    dev_req.bmRequestType = USB_STD_DIR_HOST2DEV | USB_STD_REQTYPE_STD | USB_STD_REC_DEVICE;
    dev_req.bRequest = USB_STD_SETADDRESS;
    dev_req.wValue = dev_addr;
    dev_req.wIndex = 0;
    dev_req.wLength = 0;

    fill_urb(&urb, &dev_req, NULL, 0);
    
    hURB = usb_bus_commit_urb(&urb);
    if(NULL == hURB)
    {
        return false;
    }

    if(usb_bus_wait_completion(hURB) < 0)
    {
        return false;
    }
    
    usb_host_set_address(dev_addr);
    
    return true;
}
//********************************************************************
/**
 * @brief  get descriptor request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param desc_type [in]: decriptor type
 * @param desc_index [in]: decriptor index, only validate for string descriptor
 * @param lang_id [in]: lang id, only validate for string descriptor
 * @param data [out]: buffer for descriptor data
 * @param data [out]: length of buffer
 * @return  signed long size of the data received
 */
signed long usb_host_std_get_descriptor(unsigned char desc_type, unsigned char desc_index, unsigned long lang_id, unsigned char data[], unsigned long len)
{
    T_URB urb;
    T_URB_HANDLE hURB = NULL;
    T_UsbDevReq dev_req;
    signed long ret;
    
    dev_req.bmRequestType = USB_STD_DIR_DEV2HOST | USB_STD_REQTYPE_STD | USB_STD_REC_DEVICE;
    dev_req.bRequest = USB_STD_GETDESCRIPTOR;
    dev_req.wValue = (desc_type << 8) | (desc_index);
    dev_req.wIndex = 0;
    dev_req.wLength = len;

    fill_urb(&urb, &dev_req, data, len);
    
    hURB = usb_bus_commit_urb(&urb);
    if(NULL == hURB)
    {
        return URB_ERROR;
    }

    ret = usb_bus_wait_completion(hURB);
    return ret;
}
//********************************************************************
/**
 * @brief  set  descriptor request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  bool
 */
bool usb_host_std_set_descriptor(void)
{
    return true;
}
//********************************************************************
/**
 * @brief  get  configuration request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  bool
 */
bool usb_host_std_get_configuration(void)
{
    return true;
}
//********************************************************************
/**
 * @brief  set  configuration request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param conf_val [in]: config value
 * @return  bool
 */
bool usb_host_std_set_configuration(unsigned char conf_val)
{
    T_URB urb;
    T_URB_HANDLE hURB = NULL;
    T_UsbDevReq dev_req;

    dev_req.bmRequestType = USB_STD_DIR_HOST2DEV | USB_STD_REQTYPE_STD | USB_STD_REC_DEVICE;
    dev_req.bRequest =  USB_STD_SETCONFIG;
    dev_req.wValue = conf_val;
    dev_req.wIndex = 0;
    dev_req.wLength = 0;
    
    fill_urb(&urb, &dev_req, NULL, 0);
    
    hURB = usb_bus_commit_urb(&urb);
    if(NULL == hURB)
    {
        return false;
    }

    if(usb_bus_wait_completion(hURB) < 0)
    {
        return false;
    }

    return true;
}
//********************************************************************
/**
 * @brief  get  interface request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  bool
 */
bool usb_host_std_get_interface(void)
{
    return true;
}
//********************************************************************
/**
 * @brief  set interface request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  bool
 */
bool usb_host_std_set_interface(void)
{
    T_URB urb;
    T_URB_HANDLE hURB = NULL;
    T_UsbDevReq dev_req;

    dev_req.bmRequestType = USB_STD_DIR_HOST2DEV | USB_STD_REQTYPE_STD | USB_STD_REC_INTERFACE;
    dev_req.bRequest =  USB_STD_SETINTERFACE;
    dev_req.wValue = 0;
    dev_req.wIndex = 0;
    dev_req.wLength = 0;
    
    fill_urb(&urb, &dev_req, NULL, 0);
    
    hURB = usb_bus_commit_urb(&urb);
    if(NULL == hURB)
    {
        return false;
    }

    if(usb_bus_wait_completion(hURB) < 0)
    {
        return false;
    }

    return true;
}
//********************************************************************
/**
 * @brief  sych frame request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  bool
 */
bool usb_host_std_sych_frame(void)
{
    return true;
}
//********************************************************************
#endif
