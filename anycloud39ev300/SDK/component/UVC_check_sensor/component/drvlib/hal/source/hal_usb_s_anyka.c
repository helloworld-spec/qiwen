/**
 * @filename usb_anyka.c
 * @brief how to use usb device of anyka.
 *
 * This file describe frameworks of anyka device.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  liao_zhijun
 * @date    2010-07-26
 */

#ifdef OS_ANYKA
#include    "anyka_cpu.h"
#include    "anyka_types.h"
#include    "usb_slave_drv.h"
#include    "hal_usb_s_anyka.h"
#include    "hal_usb_s_std.h"
#include    "usb_common.h"
#include    "interrupt.h"
#include    "drv_api.h"
#include    "drv_module.h"

//define message for usb anyka
#define MESSAGE_RX_NOTIFY 1
#define MESSAGE_RX_FINISH 2
#define MESSAGE_TX_FINISH 3

//********************************************************************
static void Fwl_Usb_Anyka_Send_Finish(void);
static void Fwl_Usb_Anyka_Notify(void);
static void Fwl_Usb_Anyka_Receive_Finish(void);
static void Fwl_Usb_Anyka_EnumOK(void);

static void Usb_Anyka_USB_Reset(unsigned long mode);

static unsigned char *usbanyka_getdevicedescriptor(unsigned long *count);
static unsigned char *usbanyka_getconfigdescriptor(unsigned long *count);
static unsigned char *usbanyka_getstringdescriptor(unsigned char index, unsigned long *count);

static void usbanyka_rx_notify(unsigned long *param, unsigned long len);
static void usbanyka_rx_finish(unsigned long *param, unsigned long len);
static void usbanyka_tx_finish(unsigned long *param, unsigned long len);

//********************************************************************
static const T_USB_DEVICE_DESCRIPTOR device_desc =
{
    18,                         // length 18 bytes
    DEVICE_DESC_TYPE,           // device descriptor type
    0x0110,                     // USB Specification Release Number in
    0xff,                       // Class code (assigned by the USB) 0x00?.
    0xff,                       // Subclass code (assigned by the USB)0x00?.
    0xff,                       // Protocol code (assigned by the USB)0x00?.
    EP0_MAX_PAK_SIZE,           // Maximum packet size for endpoint zero
    0x0471,                     // Vendor ID (assigned by the USB)//0x0471?
    0x0666,                     // Product ID (assigned by the manufacturer)
    0x0100,                     // Device release number in binary-coded
    0x00,                       // Index of string descriptor describing manufacturer
    0x00,                       // Index of string descriptor describing produc
    0x00,                       // Index of string descriptor describing the device's serial number
    0x01
};

static const T_USB_CONFIGURATION_DESCRIPTOR config_desc =
{
    9,                          //Size of this descriptor in bytes
    CONFIG_DESC_TYPE,           //CONFIGURATION Descriptor Type//02
    DEF_DESC_LEN,               //Total length of data returned for this
    0x01,                       //Number of interfaces supported by this configuration
    0x01,                       //Value to use as an argument to the
    0x00,                       //Index of string descriptor describing this configuration
    0xC0,
    0x01                        //Maximum power consumption of the USB
};

static const T_USB_INTERFACE_DESCRIPTOR if_desc =
{
    9,                          //Size of this descriptor in bytes
    IF_DESC_TYPE,               //INTERFACE Descriptor Type//04
    0x00,                       //Number of interface. Zero-based value
    0x00,                       //Value used to select alternate setting for
    0x03,                       //Number of endpoints used by this
    0xff,                       //Class code (assigned by the USB).
    0xff,                       //Subclass code (assigned by the USB).
    0x00,                       //Protocol code (assigned by the USB).
    0x00                        //Index of string descriptor describing this interface
};

static const T_USB_ENDPOINT_DESCRIPTOR ep1_desc =
{
    7,                         //Size of this descriptor in bytes
    EP_DESC_TYPE,              //ENDPOINT Descriptor Type//0x05
    0x81,                      //10000001
    0x03,                      //00000011
    EP1_BUF_MAX_LEN,           //0x08 0x00 Maximum packet size this endpoint
    0x0A                       //0xFFInterval for polling endpoint for data
};

static T_USB_ENDPOINT_DESCRIPTOR ep2_desc =
{
    7,                         //Size of this descriptor in bytes
    EP_DESC_TYPE,              //ENDPOINT Descriptor Type
    0x82,
    0x02,
    EP2_BUF_MAX_LEN,           //Maximum packet size this endpoint
    0x00                       //Interval for polling endpoint for data
};

static T_USB_ENDPOINT_DESCRIPTOR ep3_desc =
{
    7,                         //Size of this descriptor in bytes
    EP_DESC_TYPE,              //ENDPOINT Descriptor Type
    0x03,
    0x02,
    EP3_BUF_MAX_LEN,           //0x40 00 Maximum packet size this endpoint
    0x00                       //0x00 Interval for polling endpoint for data
};

//string buffer 
static const unsigned char sAnyka_String[] = {0};

typedef struct {
    T_fUSBANYKA_RECEIVECALLBACK receive_func;
    T_fUSBANYKA_RECEIVECALLBACK receiveok_func;
    T_fUSBANYKA_SENDFINISHCALLBACK sendfinish_func;
    
    unsigned long AnykaUSBRXCount;
    unsigned long AnykaUSBTXCount;
    
    bool bDataInRXFIFO;
    
    bool bTransmitDone;

    unsigned char  *AnykaUSBTXBuf;
    unsigned char  *AnykaUSBRXBuf;
    
}T_USBAnyka;

volatile T_USBAnyka m_usbanyka;

static void usbanyka_rx_notify(unsigned long *param, unsigned long len)
{
    //call receive callback function
    if(m_usbanyka.receive_func != NULL)
        m_usbanyka.receive_func();
}

static void usbanyka_rx_finish(unsigned long *param, unsigned long len)
{
    //call receive ok callback function
    if(m_usbanyka.receiveok_func != NULL)
        m_usbanyka.receiveok_func();
}

static void usbanyka_tx_finish(unsigned long *param, unsigned long len)
{
    //call send finish callback function
    if(m_usbanyka.sendfinish_func != NULL)
        m_usbanyka.sendfinish_func();
}

//********************************************************************

void usbanyka_init(void)
{
    //init global variables
    m_usbanyka.AnykaUSBRXBuf = NULL;
    m_usbanyka.AnykaUSBRXCount = 0;
    m_usbanyka.AnykaUSBTXBuf = NULL;
    m_usbanyka.AnykaUSBTXCount = 0;
    m_usbanyka.bDataInRXFIFO = false;
    m_usbanyka.bTransmitDone = true;

    //create task
    DrvModule_Create_Task(DRV_MODULE_USB_ANYKA);

    //map message
    DrvModule_Map_Message(DRV_MODULE_USB_ANYKA, MESSAGE_RX_NOTIFY, usbanyka_rx_notify);
    DrvModule_Map_Message(DRV_MODULE_USB_ANYKA, MESSAGE_RX_FINISH, usbanyka_rx_finish);
    DrvModule_Map_Message(DRV_MODULE_USB_ANYKA, MESSAGE_TX_FINISH, usbanyka_tx_finish);
}

void usbanyka_set_callback(T_fUSBANYKA_RECEIVECALLBACK receive_func, T_fUSBANYKA_RECEIVEOKCALLBACK receiveok_func, T_fUSBANYKA_SENDFINISHCALLBACK sendfinish_func)
{
    m_usbanyka.receive_func = receive_func;
    m_usbanyka.receiveok_func = receiveok_func;
    m_usbanyka.sendfinish_func = sendfinish_func;
}


bool usbanyka_enable(void)
{

    Usb_Slave_Standard.Usb_Device_Type =            USB_ANYKA | USB_MODE_11;
    
    Usb_Slave_Standard.Device_ConfigVal =           0;
    Usb_Slave_Standard.Device_Address =             0;
    Usb_Slave_Standard.Buffer =                     (unsigned char *)drv_malloc(4096); 
    Usb_Slave_Standard.buf_len =                    4096;

    
    Usb_Slave_Standard.usb_get_device_descriptor = usbanyka_getdevicedescriptor;
    Usb_Slave_Standard.usb_get_config_descriptor = usbanyka_getconfigdescriptor;
    Usb_Slave_Standard.usb_get_string_descriptor = usbanyka_getstringdescriptor;

    Usb_Slave_Standard.usb_reset =          Usb_Anyka_USB_Reset;
    Usb_Slave_Standard.usb_suspend =        NULL;//Fwl_Usb_SlaveReserved;
    Usb_Slave_Standard.usb_resume =         NULL;//NULLFwl_Usb_SlaveReserved;

    //init usb driver and usb stardard request driver
    usb_slave_init(Usb_Slave_Standard.Buffer, Usb_Slave_Standard.buf_len);
    usb_slave_std_init();

    //set callback function
    usb_slave_set_callback(Usb_Slave_Standard.usb_reset, Usb_Slave_Standard.usb_suspend, Usb_Slave_Standard.usb_resume, Fwl_Usb_Anyka_EnumOK);
    usb_slave_set_tx_callback(EP2_INDEX, Fwl_Usb_Anyka_Send_Finish);
    usb_slave_set_rx_callback(EP3_INDEX, Fwl_Usb_Anyka_Notify, Fwl_Usb_Anyka_Receive_Finish);

    //enable usb controller
    usb_slave_device_enable(Usb_Slave_Standard.Usb_Device_Type);

    return true;
}


static unsigned char *usbanyka_getdevicedescriptor(unsigned long *count)
{
    *count = sizeof(device_desc);
    return (unsigned char *)&device_desc;
}

static unsigned char config[100];

static unsigned char *usbanyka_getconfigdescriptor(unsigned long *count)
{
    *count = sizeof(config_desc) + sizeof(if_desc) + sizeof(ep1_desc) + sizeof(ep2_desc) + sizeof(ep3_desc);

    memcpy(config, (unsigned char *)&config_desc, sizeof(config_desc));
    memcpy(config + sizeof(config_desc), (unsigned char *)&if_desc, sizeof(if_desc));
    memcpy(config + sizeof(config_desc) + sizeof(if_desc), (unsigned char *)&ep1_desc, sizeof(ep1_desc));
    memcpy(config + sizeof(config_desc)+ sizeof(if_desc) + sizeof(ep1_desc) , (unsigned char *)&ep2_desc, sizeof(ep2_desc));
    memcpy(config + sizeof(config_desc)+ sizeof(if_desc) + sizeof(ep1_desc) + sizeof(ep2_desc) , (unsigned char *)&ep3_desc, sizeof(ep3_desc));
    
    return config;
}

static unsigned char *usbanyka_getstringdescriptor(unsigned char index, unsigned long *count)
{
    if(index == 0)
    {
        *count = sizeof(sAnyka_String);
        return ((unsigned char *)sAnyka_String);
    }
    else if(index == 1)
    {
        *count = sizeof(sAnyka_String);
        return ((unsigned char *)sAnyka_String);
    }

    return NULL;
}


//********************************************************************
static void Usb_Anyka_USB_Reset(unsigned long mode)
{
    if(mode == USB_MODE_20)
    {
        //set packet size to 512 in high speed mode
        ep2_desc.wMaxPacketSize = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;
        ep3_desc.wMaxPacketSize = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;
    }
    else
    {
        //set packet size to 64 in full speed mode
        ep2_desc.wMaxPacketSize = EP_BULK_FULLSPEED_MAX_PAK_SIZE;
        ep3_desc.wMaxPacketSize = EP_BULK_FULLSPEED_MAX_PAK_SIZE;   
    }
}
//********************************************************************
void usbanyka_disable(void)
{
    //terminate task
    DrvModule_Terminate_Task(DRV_MODULE_USB_ANYKA);

    //free memory
    drv_free(Usb_Slave_Standard.Buffer);
    Usb_Slave_Standard.Buffer = NULL;
    Usb_Slave_Standard.buf_len = 0;

    //disable usb controller
    usb_slave_set_state(USB_NOTUSE);
    usb_slave_free();
    usb_slave_device_disable();
}

//********************************************************************
static void Fwl_Usb_Anyka_Send_Finish(void)
{
    m_usbanyka.AnykaUSBTXCount = 0;
    m_usbanyka.AnykaUSBTXBuf = NULL;
    m_usbanyka.bTransmitDone = true;

    //send MESSAGE_TX_FINISH
    DrvModule_Send_Message(DRV_MODULE_USB_ANYKA, MESSAGE_TX_FINISH, NULL);
}

static void Fwl_Usb_Anyka_Notify(void)
{
    m_usbanyka.bDataInRXFIFO = true;

    //send MESSAGE_RX_NOTIFY
    DrvModule_Send_Message(DRV_MODULE_USB_ANYKA, MESSAGE_RX_NOTIFY, NULL);
}


static void Fwl_Usb_Anyka_Receive_Finish(void)
{
    m_usbanyka.AnykaUSBRXCount = 0;
    m_usbanyka.AnykaUSBRXBuf = NULL;
    m_usbanyka.bTransmitDone = true;

    //send MESSAGE_RX_FINISH
    DrvModule_Send_Message(DRV_MODULE_USB_ANYKA, MESSAGE_RX_FINISH, NULL);
}

static void Fwl_Usb_Anyka_EnumOK(void)
{
    //init global variables
    m_usbanyka.AnykaUSBRXBuf = NULL;
    m_usbanyka.AnykaUSBRXCount = 0;
    m_usbanyka.AnykaUSBTXBuf = NULL;
    m_usbanyka.AnykaUSBTXCount = 0;
    m_usbanyka.bDataInRXFIFO = false;
    m_usbanyka.bTransmitDone = true;

    akprintf(C3, M_DRVSYS, "enum ok\r\n");
}

signed long usbanyka_write(unsigned char *data, unsigned long data_len)
{
    unsigned long transcount = 0;

    //check param
    if(data == NULL || data_len == 0)
        return -1;

    //check trans status
    if(!m_usbanyka.bTransmitDone)
    {
        akprintf(C3, M_DRVSYS, "transmit not finish\n");
        return -1;
    }

    m_usbanyka.AnykaUSBTXBuf = data;
    m_usbanyka.bTransmitDone = false;

    //start send data
    usb_slave_start_send(EP2_INDEX);
    usb_slave_data_in(EP2_INDEX, data, data_len);

    return transcount;
}

signed long usbanyka_read(unsigned char *data, unsigned long data_len)
{
    unsigned long transcount = 0;

    //check param
    if(data == NULL || data_len == 0)
        return -1;

    //check trans status
    if(!m_usbanyka.bTransmitDone)
        return -1;

    if(!m_usbanyka.bDataInRXFIFO)
        return 0;

    m_usbanyka.AnykaUSBRXBuf = data;
    m_usbanyka.bDataInRXFIFO = false; //must before usb_slave_data_out
    m_usbanyka.bTransmitDone = false;

    //start receive data
    transcount = usb_slave_data_out(EP3_INDEX, data, data_len);

    return transcount;
}

//********************************************************************
#endif

