/**
 * @file hal_usb_std.c
 * @brief: standard protocol of usb.
 *
 * This file describe standard protocol of usb driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-09-01
 * @version 1.0
 */

#ifdef OS_ANYKA
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "usb_slave_drv.h"
#include "hal_usb_s_std.h"
#include "usb_common.h"
#include "hal_usb_s_state.h"
#include "drv_api.h" 

static bool usb_slave_std_reserve(T_CONTROL_TRANS *pTrans);
static bool usb_slave_std_get_status(T_CONTROL_TRANS *pTrans);
static bool usb_slave_std_clear_feature(T_CONTROL_TRANS *pTrans);
static bool usb_slave_std_set_feature(T_CONTROL_TRANS *pTrans);
static bool usb_slave_std_set_address(T_CONTROL_TRANS *pTrans);
static bool usb_slave_std_get_descriptor(T_CONTROL_TRANS *pTrans);
static bool usb_slave_std_set_descriptor(T_CONTROL_TRANS *pTrans);
static bool usb_slave_std_get_configuration(T_CONTROL_TRANS *pTrans);
static bool usb_slave_std_set_configuration(T_CONTROL_TRANS *pTrans);
static bool usb_slave_std_get_interface(T_CONTROL_TRANS *pTrans);
static bool usb_slave_std_set_interface(T_CONTROL_TRANS *pTrans);
  
static bool usb_slave_std_callback(T_CONTROL_TRANS *pTrans);


//global usb slave stuct
T_USB_SLAVE_STANDARD Usb_Slave_Standard;

//device address
volatile unsigned char m_device_addess = 0;
volatile unsigned char m_config_value = 0;

bool m_hard_stall = false;

T_fUSB_CONTROL_CALLBACK m_usb_std_req[] = 
{
    //...
    usb_slave_std_get_status,
    usb_slave_std_clear_feature,
    usb_slave_std_reserve,
    usb_slave_std_set_feature,
    usb_slave_std_reserve,
    usb_slave_std_set_address,
    usb_slave_std_get_descriptor,
    usb_slave_std_set_descriptor,
    usb_slave_std_get_configuration,
    usb_slave_std_set_configuration,
    usb_slave_std_get_interface,
    usb_slave_std_set_interface,
    usb_slave_std_reserve,
    usb_slave_std_reserve,
    usb_slave_std_reserve
};


//********************************************************************
/**
 * @brief  init  usb standard request module
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @return  void
 */
void usb_slave_std_init()
{
    usb_slave_set_ctrl_callback(REQUEST_STANDARD, usb_slave_std_callback);
}

/**
 * @brief   change the clear stall condition in clear feature
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param bSet [in]: clear stall in clear feature or not 
 * @return  void
 */
void usb_slave_std_hard_stall(bool bSet)
{
    m_hard_stall = bSet;
}

/**
 * @brief  callback function of usb standard request
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  bool
 */
bool usb_slave_std_callback(T_CONTROL_TRANS *pTrans)
{
    unsigned char req_type;

    req_type = (pTrans->dev_req.bmRequestType >> 5) & 0x3;
    if(req_type != REQUEST_STANDARD)
        return false;

    return m_usb_std_req[pTrans->dev_req.bRequest & 0x0F](pTrans);
    
}

bool usb_slave_std_reserve(T_CONTROL_TRANS *pTrans)
{
    return false;
}

/**
 * @brief  get status request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  bool
 */
bool usb_slave_std_get_status(T_CONTROL_TRANS *pTrans)
{
    unsigned char recipient;
    unsigned long ep_num = 0;
    unsigned short status;
    
    //only handle in setup stage
    if(pTrans->stage != CTRL_STAGE_SETUP)
        return true;

    //wValue should be zero, wLength should be 2
    if(pTrans->dev_req.wValue != 0 || pTrans->dev_req.wLength != 2)
        return false;

    recipient = pTrans->dev_req.bmRequestType & 0x1F;

    memset(pTrans->buffer, 0, pTrans->buf_len);
    pTrans->data_len = pTrans->dev_req.wLength;

    switch(recipient)
    {
        case RECIPIENT_DEVICE:
            if(pTrans->dev_req.wIndex != 0)
            {
                return false;
            }
            break;
            
        case RECIPIENT_INTERFACE:
            break;

        case RECIPIENT_ENDPOINT:
            ep_num = pTrans->dev_req.wIndex & 0x7F;
            status = usb_slave_get_ep_status(ep_num);
            memcpy(pTrans->buffer, &status, 2);
            break;
    }

    return true;
}

/**
 * @brief  clear feature request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  bool
 */
bool usb_slave_std_clear_feature(T_CONTROL_TRANS *pTrans)
{
    unsigned char recipient;
    unsigned long feature_selector, ep_num;

    //only handle in status stage
    if(pTrans->stage != CTRL_STAGE_STATUS)
        return true;

    recipient = pTrans->dev_req.bmRequestType & 0x1F;
    feature_selector = pTrans->dev_req.wValue;

    if(feature_selector != ENDPOINT_HALT)
    {
        return false;
    }
    else if(recipient != RECIPIENT_ENDPOINT)
    {
        return false;
    }
    
    ep_num = pTrans->dev_req.wIndex & 0x7F;
    if(!m_hard_stall)
    {
        usb_slave_ep_clr_stall(ep_num);
    }
    return true;
}

/**
 * @brief  set feature request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  bool
 */
bool usb_slave_std_set_feature(T_CONTROL_TRANS *pTrans)
{
    unsigned char recipient;
    unsigned long feature_selector, ep_num;
    unsigned long test_mode;

    recipient = pTrans->dev_req.bmRequestType & 0x1F;
    feature_selector = pTrans->dev_req.wValue;

    if(pTrans->stage == CTRL_STAGE_SETUP)  //setup stage
    {
        if(RECIPIENT_DEVICE == recipient)
        {
            if(feature_selector != TEST_MODE)
            {
                return false;
            }
            else if((pTrans->dev_req.wIndex & 0xFF) != 0)
            {
                return false;
            }
        }
        else if(RECIPIENT_ENDPOINT == recipient)
        {
            ep_num = pTrans->dev_req.wIndex & 0x7F;
            usb_slave_ep_stall(ep_num);
        }
    }

    if(pTrans->stage == CTRL_STAGE_STATUS)  //status stage
    {
        if((RECIPIENT_DEVICE == recipient) && (feature_selector == TEST_MODE))
        {        
            test_mode = pTrans->dev_req.wIndex >> 8;
            usb_slave_enter_testmode(test_mode);
        }
    }

    return true;
}

/**
 * @brief  set address request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  bool
 */
bool usb_slave_std_set_address(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage == CTRL_STAGE_STATUS)
    {
        m_device_addess = pTrans->dev_req.wValue;
        usb_slave_set_address((unsigned char)pTrans->dev_req.wValue);
    }

    return true;
}

/**
 * @brief  get descriptor request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  bool
 */
bool usb_slave_std_get_descriptor(T_CONTROL_TRANS *pTrans)
{
    unsigned char des_type, des_index;
    unsigned long cnt;
    unsigned char *data;
    
    //only handle in setup stage
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return true;
    }
    
    des_index = pTrans->dev_req.wValue & 0xFF;
    des_type = pTrans->dev_req.wValue >> 8;

    pTrans->data_len = pTrans->dev_req.wLength;

    switch( des_type )
    {
        case DEVICE_DESC_TYPE:           //01
            if(Usb_Slave_Standard.usb_get_device_descriptor != NULL)
            {
                data = Usb_Slave_Standard.usb_get_device_descriptor(&cnt);
            }
            else
            {
                return false;
            }
            break;

        case CONFIG_DESC_TYPE:          //02
            if(Usb_Slave_Standard.usb_get_config_descriptor != NULL)
            {
                data = Usb_Slave_Standard.usb_get_config_descriptor(&cnt);
            }
            else
            {
                return false;
            }
            break;
            
        case STRING_DESC_TYPE:         //03
            if(Usb_Slave_Standard.usb_get_string_descriptor != NULL)
            {
                data = Usb_Slave_Standard.usb_get_string_descriptor(des_index, &cnt);
            }
            else
            {
                return false;
            }
            break;

        case DEVICE_QUALIFIER_DESC_TYPE: //06
            if(Usb_Slave_Standard.usb_get_device_qualifier_descriptor != NULL)
            {
                data = Usb_Slave_Standard.usb_get_device_qualifier_descriptor(&cnt);
            }
            else
            {
                return false;
            }
            break;
            
        case OTHER_SPEED_CONFIGURATION_DESC_TYPE: //07
            if(Usb_Slave_Standard.usb_get_other_speed_config_descriptor != NULL)
            {
                data = Usb_Slave_Standard.usb_get_other_speed_config_descriptor(&cnt);
            }
            else
            {
                return false;
            }
            break;

        default:
            return false;

    }

    if(cnt < pTrans->data_len)
    {
        pTrans->data_len = cnt;
    }
    
    memcpy(pTrans->buffer, data, pTrans->data_len);

    return true;
}

/**
 * @brief  set descriptor request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  bool
 */
bool usb_slave_std_set_descriptor(T_CONTROL_TRANS *pTrans)
{
    return false;
}


/**
 * @brief  get configuration request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  bool
 */
bool usb_slave_std_get_configuration(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return true;
    }

    if(pTrans->dev_req.wValue != 0 || pTrans->dev_req.wIndex != 0 || pTrans->dev_req.wLength != 1)
    {
        return false;
    }

    pTrans->data_len = pTrans->dev_req.wLength;
    pTrans->buffer[0] = m_config_value;

    return true;
}

/**
 * @brief  set configuration request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  bool
 */
bool usb_slave_std_set_configuration(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return true;
    }

    if(pTrans->dev_req.wIndex != 0 || pTrans->dev_req.wLength != 0)
    {
        return false;
    }

    m_config_value = pTrans->dev_req.wValue;

    usb_slave_set_state(USB_OK);

    usb_slave_clr_toggle();

    return true;
}

/**
 * @brief  get interface request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  bool
 */
bool usb_slave_std_get_interface(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return true;
    }

    if(pTrans->dev_req.wValue != 0 || pTrans->dev_req.wLength != 1)
    {
        return false;
    }
    
    pTrans->data_len = pTrans->dev_req.wLength;
    pTrans->buffer[0] = 0;

    return true;
}

/**
 * @brief  set interface request handler
 *
 * @author  liao_zhijun
 * @date    2010-07-27
 * @param pTrans [in]: pointer to control trans struct  
 * @return  bool
 */
bool usb_slave_std_set_interface(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return true;
    }

    usb_slave_clr_toggle();

    return true;
}

 
#endif
