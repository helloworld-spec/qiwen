/**
 * @file usb_bus_driver.c
 * @brief:  frameworks of usb bus driver.
 *
 * This file describe driver of usb in host mode.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-06-30
 * @version 1.0
 */
#include "usb_host_drv.h"
#include "hal_usb_std.h"
#include "drv_api.h"
#include "usb_bus_drv.h"
#include "drv_module.h"



/**
 * @brief usb bus struct
 
 *   define usb bus struct
 */
typedef struct
{
    T_USB_BUS_HANDLER *bus_handler[MAX_REG_CLASS_NUM];
    unsigned long reg_num;
    unsigned char status;
    T_URB urb;

    unsigned char dev_addr;
    unsigned char conf_val;

    T_USB_DEVICE_DESCRIPTOR dev_desc;
    T_USB_CONFIGURATION_DESCRIPTOR conf_desc;
    T_USB_INTERFACE_DESCRIPTOR if_desc;
    T_USB_ENDPOINT_DESCRIPTOR ep_desc[MAX_ENDPOINT_NUM];
}
T_USB_BUS;

static void usb_bus_ctrl_callback(unsigned char trans_state, unsigned long trans_len);
static void usb_bus_trans_callback(unsigned char trans_state, unsigned long trans_len);
static void usb_bus_connect_callback();
static void usb_bus_disconnect_callback();
static void usb_bus_connect(unsigned long *param, unsigned long len);
static void usb_bus_disconnect(unsigned long *param, unsigned long len);

static volatile T_USB_BUS m_usb_bus = {0};

void usb_bus_enum()
{
    unsigned char *buf;
    unsigned long i;
    
    #define CHECK(x) if((x)<0) goto ENUM_ERROR;

    //set address
    usb_host_set_address(0);
    //debounce interval
    mini_delay(100);
    //reset device
    usb_host_reset();
    //recovery interval
    mini_delay(10);

    //alloc memory
    buf = drv_malloc(4096);
    if(NULL == buf)
    {
        akprintf(C1, M_DRVSYS, "malloc fail in usb_bus_connect\r\n");
        return;
    }
    
    akprintf(C3, M_DRVSYS, "enum device\r\n");

    //start enum
    //get device descriptor
    CHECK(usb_host_std_get_descriptor(DEVICE_DESC_TYPE, 0, 0, buf, 64));

    memcpy(&m_usb_bus.dev_desc, buf, sizeof(T_USB_DEVICE_DESCRIPTOR));
    usb_host_set_max_ep0_size(m_usb_bus.dev_desc.bMaxPacketSize0);
    //set address
    if(!usb_host_std_set_address(m_usb_bus.dev_addr))
        goto ENUM_ERROR;

    //get device desc again
    CHECK(usb_host_std_get_descriptor(DEVICE_DESC_TYPE, 0, 0, buf, m_usb_bus.dev_desc.bLength));

    //get config desc
    CHECK(usb_host_std_get_descriptor(CONFIG_DESC_TYPE, 0, 0, buf, sizeof(T_USB_CONFIGURATION_DESCRIPTOR)));
    memcpy(&m_usb_bus.conf_desc, buf, sizeof(T_USB_CONFIGURATION_DESCRIPTOR));

    //get full config desc
    CHECK(usb_host_std_get_descriptor(CONFIG_DESC_TYPE, 0, 0, buf, m_usb_bus.conf_desc.wTotalLength));

    memcpy(&m_usb_bus.if_desc, buf+sizeof(T_USB_CONFIGURATION_DESCRIPTOR), 
        sizeof(T_USB_INTERFACE_DESCRIPTOR));

    memcpy(m_usb_bus.ep_desc, buf+sizeof(T_USB_CONFIGURATION_DESCRIPTOR) + sizeof(T_USB_INTERFACE_DESCRIPTOR), 
        sizeof(T_USB_ENDPOINT_DESCRIPTOR) * m_usb_bus.if_desc.bNumEndpoints);

    //config ep
    for(i = 0; i < m_usb_bus.if_desc.bNumEndpoints; i++)
    {
        usb_host_set_ep(m_usb_bus.ep_desc[i]);
    }

    //get string desc
    CHECK(usb_host_std_get_descriptor(STRING_DESC_TYPE, 0, 0, buf, 2));

    //set config
    if(!usb_host_std_set_configuration(m_usb_bus.conf_desc.bConfigurationValue))
        goto ENUM_ERROR;

    akprintf(C3, M_DRVSYS, "enum success!\r\n");
    drv_free(buf);
    return;
    
ENUM_ERROR:
    drv_free(buf);
}

/**
 * @brief open usb controller and phy
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param mode  [in] unsigned long usb mode
 * @return  void
 */
void usb_bus_open(unsigned long mode)
{
    //set callback
    usb_host_set_common_intr_callback(USB_HOST_CONNECT, usb_bus_connect_callback);
    usb_host_set_common_intr_callback(USB_HOST_DISCONNECT, usb_bus_disconnect_callback);

    usb_host_set_trans_callback(usb_bus_ctrl_callback, usb_bus_trans_callback);

    //create task
    DrvModule_Create_Task(DRV_MODULE_USB_BUS);

    //map event
    DrvModule_Map_Message(DRV_MODULE_USB_BUS, MESSAGE_CONNECT, usb_bus_connect);
    DrvModule_Map_Message(DRV_MODULE_USB_BUS, MESSAGE_DISCONNECT, usb_bus_disconnect);

    //init global variable
    m_usb_bus.dev_addr = 2;
    m_usb_bus.status = USB_BUS_IDLE;

    //enable host controller
    usb_host_device_enable(mode);
}

/**
 * @brief   close usb controller and phy.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @return  void
 */
void usb_bus_close()
{
    unsigned long i;

    //disable host controller
    usb_host_device_disable();

    //free memory
    for(i = 0; i < m_usb_bus.reg_num; i++)
    {
        if (NULL != m_usb_bus.bus_handler[i])
        {
            drv_free(m_usb_bus.bus_handler[i]);
            m_usb_bus.bus_handler[i] = NULL;
            m_usb_bus.reg_num -= 1;
        }
    }
	
	memset(&m_usb_bus,0,sizeof(T_USB_BUS));

    //delete task
    DrvModule_Terminate_Task(DRV_MODULE_USB_BUS);
}

/**
 * @brief   register usb class to bus driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param bus_handler struction containing with class code and event handler
 * @return  void
 */
bool usb_bus_reg_class(T_pUSB_BUS_HANDLER bus_handler)
{
    T_pUSB_BUS_HANDLER pHandler;

    //check param
    if((m_usb_bus.reg_num >= MAX_REG_CLASS_NUM) || (NULL == bus_handler))
        return false;

    //alloc memory
    pHandler = drv_malloc(sizeof(T_USB_BUS_HANDLER));
    if(NULL == pHandler)
    {
        false;
    }
    
    memcpy(pHandler, bus_handler, sizeof(T_USB_BUS_HANDLER));
    
    m_usb_bus.bus_handler[m_usb_bus.reg_num++] = pHandler;

    return true;
}

/**
 * @brief   disable usb slave driver.
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  urb usb request block
 * @return  T_URB_HANDLE urb handle
 * @retval NULL commit fail
 */
T_URB_HANDLE usb_bus_commit_urb(T_URB *urb)
{
    bool res;
    
    //check if bus idle 
    if(m_usb_bus.status != USB_BUS_IDLE || NULL == urb)
    {
        return URB_INVALIDATE_HANDLE;
    }

    //save urb infor
    memcpy(&m_usb_bus.urb, urb, sizeof(T_URB));

    //start trans
    res = false;
    if(TRANS_CTRL == urb->trans_type)           //control transfer
    {
        m_usb_bus.status = USB_BUS_CTRL_TRANS;
        res = usb_host_ctrl_tranfer(urb->dev_req, urb->data, urb->data_len);
    }
    else if(TRANS_BULK == urb->trans_type)
    {
        m_usb_bus.status = USB_BUS_BULK_TRANS;
        if(TRANS_DATA_IN == urb->trans_dir)
        {
            res = usb_host_bulk_in(urb->data, urb->data_len);
        }
        else
        {
            res = usb_host_bulk_out(urb->data, urb->data_len);
        }
    }

    if(res)
    {
        return (void *)&m_usb_bus.urb;
    }
    else
    {
        return URB_INVALIDATE_HANDLE;
    }
}

/**
 * @brief   wait urb to complete, return value indicates it finished success or fail
 *
 * @author  liaozhijun
 * @date    2010-06-30
 * @param  hUrb urb handle
 * @return  signed long
 * @retval URB_COMPLETE urb successfully complete
 * @retval URB_ERROR error during data transfer
 * @retval URB_TIMEOUT data transfer timeout
 */
signed long usb_bus_wait_completion(T_URB_HANDLE hUrb)
{
    signed long ret;
    
    //wait event
    ret = DrvModule_WaitEvent(DRV_MODULE_USB_BUS, EVENT_TRANS_COMPLETE, m_usb_bus.urb.timeout);    

    //check result
    if(DRV_MODULE_TIMEOUT == ret)
    { 
        return URB_TIMEOUT;
    }
    else if(m_usb_bus.urb.result == USB_HOST_TRANS_ERROR)
    {       
        return URB_ERROR;
    }
    else
    {      
        return m_usb_bus.urb.trans_len;
    }
}

unsigned long usb_bus_get_decsriptor(unsigned char desc_type, unsigned char data[], unsigned long len)
{

    if(DEVICE_DESC_TYPE == desc_type)
    {
        if(len < sizeof(T_USB_DEVICE_DESCRIPTOR))
            return 0;

        memcpy(data, m_usb_bus.dev_desc, sizeof(T_USB_DEVICE_DESCRIPTOR));
        return sizeof(T_USB_DEVICE_DESCRIPTOR);
    }

    if(CONFIG_DESC_TYPE == desc_type)
    {
        if(len < sizeof(T_USB_CONFIGURATION_DESCRIPTOR))
            return 0;
            
        memcpy(data, m_usb_bus.conf_desc, sizeof(T_USB_CONFIGURATION_DESCRIPTOR));
        return sizeof(T_USB_CONFIGURATION_DESCRIPTOR);
    }

    if(IF_DESC_TYPE == desc_type)
    {
        if(len < sizeof(T_USB_INTERFACE_DESCRIPTOR))
            return 0;
            
        memcpy(data, m_usb_bus.if_desc, sizeof(T_USB_INTERFACE_DESCRIPTOR));
        return sizeof(T_USB_INTERFACE_DESCRIPTOR);
    }

    if(EP_DESC_TYPE == desc_type)
    {
        if(len < sizeof(T_USB_ENDPOINT_DESCRIPTOR) * m_usb_bus.if_desc.bNumEndpoints)
            return 0;
            
        memcpy(data, m_usb_bus.ep_desc, sizeof(T_USB_ENDPOINT_DESCRIPTOR) * m_usb_bus.if_desc.bNumEndpoints);
        return sizeof(T_USB_ENDPOINT_DESCRIPTOR) * m_usb_bus.if_desc.bNumEndpoints;
    }

    return 0;
}

//connect
static void usb_bus_connect_callback()
{
    //send connect message
    unsigned long msg_param = CONNECT_MSG_PARAM_ENUM;
    DrvModule_Send_Message(DRV_MODULE_USB_BUS, MESSAGE_CONNECT, &msg_param);
}

//disconnect
static void usb_bus_disconnect_callback()
{
    //send disconnect message
    DrvModule_Send_Message(DRV_MODULE_USB_BUS, MESSAGE_DISCONNECT, NULL);
}

//control transfer finish
static void usb_bus_ctrl_callback(unsigned char trans_state, unsigned long trans_len)
{
    //save state and trans_len
    m_usb_bus.urb.result = trans_state;
    m_usb_bus.urb.trans_len = trans_len;

    //change status
    m_usb_bus.status = USB_BUS_IDLE;

    //set event
    DrvModule_SetEvent(DRV_MODULE_USB_BUS, EVENT_TRANS_COMPLETE);

    //callback
    if(m_usb_bus.urb.callback != NULL)
    {
        m_usb_bus.urb.callback();
    }
}

//other tranfer finish
static void usb_bus_trans_callback(unsigned char trans_state, unsigned long trans_len)
{
    //save state and trans_len
    m_usb_bus.urb.result = trans_state;
    m_usb_bus.urb.trans_len = trans_len;

    //change status
    m_usb_bus.status = USB_BUS_IDLE;

    //set event
    DrvModule_SetEvent(DRV_MODULE_USB_BUS, EVENT_TRANS_COMPLETE);

    //callback
    if(m_usb_bus.urb.callback != NULL)
    {
        m_usb_bus.urb.callback();
    }
}

static void usb_bus_connect(unsigned long *param, unsigned long len)
{    
    unsigned long i;

    if (CONNECT_MSG_PARAM_ENUM == *param)
    {
        usb_bus_enum();
        //enum success, call callback function
        for(i = 0; i < m_usb_bus.reg_num; i++)
        {
            if(m_usb_bus.bus_handler[i]->class_code == m_usb_bus.if_desc.bInterfaceClass)
            {
                m_usb_bus.bus_handler[i]->enumok_callback();
                break;
            }
        }
    }
    else
    {
        for(i = 0; i < m_usb_bus.reg_num; i++)
        {
            if(m_usb_bus.bus_handler[i]->class_code == *param)
            {
                m_usb_bus.bus_handler[i]->enumok_callback();
                break;
            }
        }
    }
}

static void usb_bus_disconnect(unsigned long *param, unsigned long len)
{
    unsigned long i;

    akprintf(C3, M_DRVSYS, "Device disconnected\r\n");  
    
    //call disconnect callback
    for(i = 0; i < m_usb_bus.reg_num; i++)
    {
        if(m_usb_bus.bus_handler[i]->class_code == m_usb_bus.if_desc.bInterfaceClass)
        {
            m_usb_bus.bus_handler[i]->discon_callback();
            break;
        }
    }
}

