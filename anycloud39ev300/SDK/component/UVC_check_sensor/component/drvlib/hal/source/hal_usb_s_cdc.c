/**
 * @filename usb_camera.c
 * @brief: AK3223M how to use usb disk.
 *
 * This file describe frameworks of usb camera driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 * @ref
 */

#ifdef OS_ANYKA
#include <string.h>
#include "usb_slave_drv.h"
#include "hal_usb_s_cdc.h"
#include "hal_usb_s_std.h"
#include "interrupt.h"
#include "usb_common.h"
#include "usb.h"
#include "drv_api.h"
#include "drv_module.h"

#define MESSAGE_USBCDC_SENDFINISH     0x1
#define MESSAGE_USBCDC_RECEIVE        0x2


//********************************************************************

static bool Fwl_Usb_CDCEnable(void);
static void Fwl_Usb_CDCDisable(void);

static void Fwl_Usb_CDC_SendFinish(void);
static void Fwl_Usb_CDC_NotifyFinish(void);

static void Fwl_Usb_CDC_Receive(void);
static void Fwl_Usb_CDC_ReceiveOK(void);

static unsigned char *usbcdc_getdevicedescriptor(unsigned long *count);
static unsigned char *usbcdc_getconfigdescriptor(unsigned long *count);
static unsigned char *usbcdc_getstringdescriptor(unsigned char index, unsigned long *count);

static void usbcdc_sendfin_handler(unsigned long *param, unsigned long len);
static void usbcdc_receive_handler(unsigned long *param, unsigned long len);
static bool usbcdc_ctrl_callback(T_CONTROL_TRANS *pTrans);

//********************************************************************

const unsigned char deviceDescrCDC[] = {
    0x12,   /* bLength */
    0x01,   /* bDescriptorType */
    0x10,
    0x01,   /* bcdUSB = 2.00 */
    0x02,   /* bDeviceClass: CDC */
    0x00,   /* bDeviceSubClass */
    0x00,   /* bDeviceProtocol */
    64,     /* bMaxPacketSize0 */
    0xD6,
    0x04,   /* idVendor = 0x04d6 */
    0x04,
    0xE1,   /* idProduct = 0xe104 */
    0x00,
    0x01,   /* bcdDevice = 1.00 */
    0,      /* Index of string descriptor describing manufacturer */
    0,      /* Index of string descriptor describing product */
    0,      /* Index of string descriptor describing the device's serial number */
    0x01    /* bNumConfigurations */
};

const unsigned char CDC_StringDescriptor[] = {
    0x04,
    0x03,
    0x09,
    0x04,   /* LangID = 0x0409: U.S. English */
    /* 4 */
    38,     /* Size of manufaturer string */
    0x03,   /* bDescriptorType = String descriptor */
    /* Manufacturer: "STMicroelectronics" */
    'S',0, 'T',0, 'M',0, 'i',0, 'c',0, 'r',0, 'o',0, 'e',0,
    'l',0, 'e',0, 'c',0, 't',0, 'r',0, 'o',0, 'n',0, 'i',0,
    'c',0, 's',0,
    /* 42 */
    46,
    0x03,
    /* Product name: "Anglia ARM7 CDC Demo" */
    'A',0, 'n',0, 'g',0, 'l',0, 'i',0, 'a',0, ' ',0, 'A',0,
    'R',0, 'M',0, '7',0, ' ',0, 'C',0, 'D',0, 'C',0, ' ',0,
    'D',0, 'e',0, 'm',0, 'o',0, ' ',0, ' ',0,
    /* 88 */
    22,     /* Size of serial string */
    0x03,   /* bDescriptorType = String descriptor */
    '2',0, '2',0, '/',0, '0',0, '7',0, '/',0, '2',0, '0',0,
    '0',0, '5',0
};

const unsigned char configDescrCDC[] = {
    /*-------------------------- Configuation Descriptor ------------------------*/
    0x09,   /* bLength: Configuation Descriptor size */
    0x02,   /* bDescriptorType: Configuration */
    67,     /* wTotalLength:no of returned bytes */
    0x00,
    0x02,   /* bNumInterfaces: 2 interface */
    0x01,   /* bConfigurationValue: Configuration value */
    0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
    0xC0,   /* bmAttributes: self powered */
    0x00,   /* MaxPower 0 mA */
    /*------------------- Interface Descriptor -------------------*/
    0x09,   /* bLength: Interface Descriptor size */
    0x04,   /* bDescriptorType: Interface */
            /* Interface descriptor type */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x01,   /* bNumEndpoints: One endpoints used */
    0x02,   /* bInterfaceClass: Communication Interface Class */
    0x02,   /* bInterfaceSubClass: Abstract Control Model */
    0x01,   /* bInterfaceProtocol: Common AT commands */
    0x00,   /* iInterface: */
    /*---------------- Header Functional Descriptor ----------------------*/
    0x05,   /* bLength: Endpoint Descriptor size */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x00,   /* bDescriptorSubtype: Header Func Desc */
    0x10,   /* bcdCDC: spec release number */
    0x01,
    /*---------------- Call Managment Functional Descriptor ----------------------*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
    0x00,   /* bmCapabilities: D0+D1 */
    0x01,   /* bDataInterface: 1 */
    /*---------------- ACM Functional Descriptor ----------------------*/
    0x04,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
    0x02,   /* bmCapabilities */
    /*---------------- Union Functional Descriptor ----------------------*/
    0x05,   /* bFunctionLength */
    0x24,   /* bDescriptorType: CS_INTERFACE */
    0x06,   /* bDescriptorSubtype: Union func desc */
    0x00,   /* bMasterInterface: Communication class interface */
    0x01,   /* bSlaveInterface0: Data Class Interface */
    /*---------------- Endpoint 2 Descriptor ----------------------*/
    0x07,   /* bLength: Endpoint Descriptor size */
    0x05,   /* bDescriptorType: Endpoint */
    0x81,   /* bEndpointAddress: (IN2) */
    0x03,   /* bmAttributes: Interrupt */
    64,     /* wMaxPacketSize: */
    0x00,
    10,     /* bInterval: */
    /*---------------- Data class interface descriptor ----------------------*/
    0x09,   /* bLength: Endpoint Descriptor size */
    0x04,   /* bDescriptorType: */
    0x01,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints: Two endpoints used */
    0x0A,   /* bInterfaceClass: CDC */
    0x00,   /* bInterfaceSubClass: */
    0x00,   /* bInterfaceProtocol: */
    0x00,   /* iInterface: */
    /*---------------- Endpoint 3 Descriptor ----------------------*/
    0x07,   /* bLength: Endpoint Descriptor size */
    0x05,   /* bDescriptorType: Endpoint */
    0x03,   /* bEndpointAddress: (OUT3) */
    0x02,   /* bmAttributes: Bulk */
    64,     /* wMaxPacketSize: */
    0x00,
    0x00,   /* bInterval: ignore for Bulk transfer */
    /*---------------- Endpoint 1 Descriptor ----------------------*/
    0x07,   /* bLength: Endpoint Descriptor size */
    0x05,   /* bDescriptorType: Endpoint */
    0x82,   /* bEndpointAddress: (IN1) */
    0x02,   /* bmAttributes: Bulk */
    64,     /* wMaxPacketSize: */
    0x00,
    0x00    /* bInterval: ignore for Bulk transfer */
};


const static unsigned char usbDescriptorString0[] = { /* language descriptor */
    4,          /* sizeof(usbDescriptorString0): length of descriptor in bytes */
    3,          /* descriptor type */
    0x09, 0x04, /* language index (0x0409 = US-English) */
};

const static unsigned char usbDescriptorString1[] = {
    12,         // bLength
    0x03,       // bDescriptorType = string
    'A',0,      // love that Unicode!
    'N',0,
    'Y',0,
    'K',0,
    'A',0
};


//********************************************************************

enum {
    SEND_ENCAPSULATED_COMMAND = 0,
    GET_ENCAPSULATED_RESPONSE,
    SET_COMM_FEATURE,
    GET_COMM_FEATURE,
    CLEAR_COMM_FEATURE,
    SET_LINE_CODING = 0x20,
    GET_LINE_CODING,
    SET_CONTROL_LINE_STATE,
    SEND_BREAK
};

typedef union usbDWord {
    unsigned long	dword;
    unsigned char   bytes[4];
} usbDWord_t;


typedef void (*T_fUSBCDC_HISR_HANDLER)(void);


typedef struct {
	usbDWord_t	baud;
    unsigned char    stopbit;
    unsigned char    parity;
    unsigned char    databit;

    T_UsbDevReq lastreq;
    unsigned char comm_feature[8];

    unsigned long packdata[512/4];
    unsigned long packlen;

    bool bDTEPresent;

    bool bCanWrite;
    
    T_fUSBCDC_RECEIVECALLBACK receive_func;
    T_fUSBCDC_SENDFINISHCALLBACK sendfinish_func;
    bool  bSending;
    unsigned char    *pReceivePool;
    unsigned long   nReceivePoolLength;
    unsigned long   nReceivePoolHead;
    unsigned long   nReceivePoolTail;
}T_USBCDC;

volatile T_USBCDC m_usbcdc = {0};

unsigned char serialStateNotification[10] = {0xa1, 0x20, 0, 0, 0, 0, 2, 0, 3, 0};
//********************************************************************

void usbcdc_sendfin_handler(unsigned long *param, unsigned long len)
{
    if(m_usbcdc.sendfinish_func != NULL)
        m_usbcdc.sendfinish_func();
}

void usbcdc_receive_handler(unsigned long *param, unsigned long len)
{
    if(m_usbcdc.receive_func != NULL)
        m_usbcdc.receive_func();
}

bool usbcdc_ctrl_callback(T_CONTROL_TRANS *pTrans)
{
    unsigned char data[7];
    unsigned long sendlen;

    switch (pTrans->dev_req.bRequest)
    {
        case GET_LINE_CODING:
            if(pTrans->stage == CTRL_STAGE_SETUP)
            {
                pTrans->buffer[0] = m_usbcdc.baud.bytes[0];
                pTrans->buffer[1] = m_usbcdc.baud.bytes[1];
                pTrans->buffer[2] = m_usbcdc.baud.bytes[2];
                pTrans->buffer[3] = m_usbcdc.baud.bytes[3];
                pTrans->buffer[4] = m_usbcdc.stopbit;
                pTrans->buffer[5] = m_usbcdc.parity;
                pTrans->buffer[6] = m_usbcdc.databit;

                pTrans->data_len = pTrans->dev_req.wLength;

                if(m_usbcdc.bDTEPresent)
                {
                    m_usbcdc.bCanWrite = true;
                }
            }
            break;

        case SET_LINE_CODING:
            if(pTrans->stage == CTRL_STAGE_DATA_OUT)
            {
                /*    SET_LINE_CODING    */
                m_usbcdc.baud.bytes[0] = data[0];
                m_usbcdc.baud.bytes[1] = data[1];
                m_usbcdc.baud.bytes[2] = data[2];
                m_usbcdc.baud.bytes[3] = data[3];
                
                m_usbcdc.stopbit    = data[4];
                m_usbcdc.parity     = data[5];
                m_usbcdc.databit    = data[6];

                if( m_usbcdc.parity > 2 )
                    m_usbcdc.parity = 0;
                if( m_usbcdc.stopbit == 1 )
                    m_usbcdc.stopbit = 0;

                memset((void *)&m_usbcdc.lastreq, 0, sizeof(m_usbcdc.lastreq));
            }
            break;
            
        case SET_CONTROL_LINE_STATE:
            if(pTrans->stage == CTRL_STAGE_SETUP)
            {

                if((pTrans->dev_req.wValue & 0x01) == 0x01)
                {
                    m_usbcdc.bDTEPresent = true;
                }
                else
                {                
                    m_usbcdc.bDTEPresent = false;
                    m_usbcdc.bCanWrite = false;
                }
            } 
            break;
            
        case GET_COMM_FEATURE:
            if(pTrans->stage == CTRL_STAGE_SETUP)
            {
                memcpy(pTrans->buffer, (unsigned char *)m_usbcdc.comm_feature, sizeof(m_usbcdc.comm_feature));
                pTrans->data_len = pTrans->dev_req.wLength;
            }
            break;

        case SET_COMM_FEATURE:
            if(pTrans->stage == CTRL_STAGE_DATA_OUT)
            {
                memcpy((void *)m_usbcdc.comm_feature, data, m_usbcdc.lastreq.wLength);
                memset((void *)&m_usbcdc.lastreq, 0, sizeof(m_usbcdc.lastreq));
            }
            break;

        default:
            break;
            
    }
    
    return true;
}

//********************************************************************
static bool Fwl_Usb_CDCEnable(void)
{   
    Usb_Slave_Standard.Usb_Device_Type =            USB_CDC | USB_MODE_11;  

    Usb_Slave_Standard.usb_get_device_descriptor = usbcdc_getdevicedescriptor;
    Usb_Slave_Standard.usb_get_config_descriptor = usbcdc_getconfigdescriptor;
    Usb_Slave_Standard.usb_get_string_descriptor = usbcdc_getstringdescriptor;

    Usb_Slave_Standard.Device_ConfigVal =           0;
    Usb_Slave_Standard.Device_Address =             0;
    Usb_Slave_Standard.Buffer =                     (unsigned char *)drv_malloc(4096);//buffer size 3*USB_CAM_BUF_SIZE
    Usb_Slave_Standard.buf_len =                    4096;

    Usb_Slave_Standard.usb_reset =          NULL;
    Usb_Slave_Standard.usb_suspend =        NULL;
    Usb_Slave_Standard.usb_resume =         NULL;

    usb_slave_device_disable();
    
    usb_slave_init(Usb_Slave_Standard.Buffer, Usb_Slave_Standard.buf_len);

    usb_slave_std_init();
    
    usb_slave_set_callback(Usb_Slave_Standard.usb_reset, Usb_Slave_Standard.usb_suspend, Usb_Slave_Standard.usb_resume, NULL);

    usb_slave_set_ctrl_callback(REQUEST_CLASS, usbcdc_ctrl_callback);

    usb_slave_set_tx_callback(EP2_INDEX, Fwl_Usb_CDC_SendFinish);
    usb_slave_set_tx_callback(EP1_INDEX, Fwl_Usb_CDC_NotifyFinish);
    usb_slave_set_rx_callback(EP3_INDEX, Fwl_Usb_CDC_Receive, Fwl_Usb_CDC_ReceiveOK);

    usb_slave_device_enable(Usb_Slave_Standard.Usb_Device_Type);

    //create task
    DrvModule_Create_Task(DRV_MODULE_USB_CDC);
    
    //map message
    DrvModule_Map_Message(DRV_MODULE_USB_CDC, MESSAGE_USBCDC_SENDFINISH, usbcdc_sendfin_handler);
    DrvModule_Map_Message(DRV_MODULE_USB_CDC, MESSAGE_USBCDC_RECEIVE, usbcdc_receive_handler);

    return true;
}
//********************************************************************
static void Fwl_Usb_CDCDisable(void)
{
    usb_slave_set_state(USB_NOTUSE);

    //free memory
    drv_free(Usb_Slave_Standard.Buffer);
    Usb_Slave_Standard.Buffer = NULL;
    Usb_Slave_Standard.buf_len = 0;
    usb_slave_free();
    usb_slave_device_disable();

    DrvModule_Terminate_Task(DRV_MODULE_USB_CDC);
}

static unsigned char *usbcdc_getdevicedescriptor(unsigned long *count)
{
    *count = sizeof(deviceDescrCDC);
    return (unsigned char *)deviceDescrCDC;
}

static unsigned char *usbcdc_getconfigdescriptor(unsigned long *count)
{
    *count = sizeof(configDescrCDC);
    return (unsigned char *)configDescrCDC;
}

static unsigned char *usbcdc_getstringdescriptor(unsigned char index, unsigned long *count)
{
    if(index == 0)
    {
        *count = sizeof(usbDescriptorString0);
        return (unsigned char *)usbDescriptorString0;
    }
    else if(index == 1)
    {
        *count = sizeof(usbDescriptorString1);
        return (unsigned char *)usbDescriptorString1;
    }

    return NULL;
}

//********************************************************************

static void Fwl_Usb_CDC_ReceiveOK(void)
{
    unsigned long i = 0;

    if (((m_usbcdc.nReceivePoolTail + m_usbcdc.nReceivePoolLength - m_usbcdc.nReceivePoolHead)%m_usbcdc.nReceivePoolLength) + m_usbcdc.packlen >= m_usbcdc.nReceivePoolLength)
    {
        return;
    }

    for(i = 0; i < m_usbcdc.packlen; i++)
    {
        m_usbcdc.pReceivePool[m_usbcdc.nReceivePoolTail] = *(unsigned char *)((unsigned char *)m_usbcdc.packdata + i);

        m_usbcdc.nReceivePoolTail = (m_usbcdc.nReceivePoolTail+1)%m_usbcdc.nReceivePoolLength;

    }

    DrvModule_Send_Message(DRV_MODULE_USB_CDC, MESSAGE_USBCDC_RECEIVE, NULL);   
 
}


static void Fwl_Usb_CDC_Receive(void)
{
    unsigned long count;

    usb_slave_read_ep_cnt(EP3_INDEX, &count);

    m_usbcdc.packlen = count;
    
    usb_slave_data_out(EP3_INDEX, (unsigned char *)m_usbcdc.packdata, count);
    
}

static void Fwl_Usb_CDC_SendFinish(void)
{
    //send message
    DrvModule_Send_Message(DRV_MODULE_USB_CDC, MESSAGE_USBCDC_SENDFINISH, NULL);    
    
    m_usbcdc.bSending = false;
}

static void Fwl_Usb_CDC_NotifyFinish(void)
{
    usb_slave_data_in(USB_EP1_INDEX, serialStateNotification, sizeof(serialStateNotification));
}


//********************************************************************

void usbcdc_init(void)
{
    memset((void *)&m_usbcdc, 0, sizeof(m_usbcdc));
    m_usbcdc.baud.bytes[0] = 0;
    m_usbcdc.baud.bytes[1] = 0;
    m_usbcdc.baud.bytes[2] = 2;
    m_usbcdc.baud.bytes[3] = 0x1c;
    m_usbcdc.parity = 0;
    m_usbcdc.stopbit = 0;
    m_usbcdc.databit = 8;
}

bool usbcdc_enable(void)
{
    if(false == Fwl_Usb_CDCEnable())
    {
        akprintf(C1, M_DRVSYS, "enable cdc false!\r\n");
        return false;
    }    

    akprintf(C3, M_DRVSYS, "USB CDC ok!\r\n" );

    return true;
}

void usbcdc_disable(void)
{
    Fwl_Usb_CDCDisable();
}

void usbcdc_set_callback(T_fUSBCDC_RECEIVECALLBACK receive_func, T_fUSBCDC_SENDFINISHCALLBACK sendfinish_func)
{
    m_usbcdc.receive_func = receive_func;
    m_usbcdc.sendfinish_func = sendfinish_func;
}

void usbcdc_set_datapool(unsigned char *pool, unsigned long poollength)
{
    m_usbcdc.pReceivePool = pool;
    m_usbcdc.nReceivePoolLength = poollength;
}

bool usbcdc_DTEPresent()
{
    return m_usbcdc.bDTEPresent;
}

signed long usbcdc_write(unsigned char *data, unsigned long data_len)
{
    unsigned long timeout = 0;

    if(!m_usbcdc.bCanWrite)
        return 0;
    
    if(m_usbcdc.bSending)
        return -1;

    m_usbcdc.bSending = true;

    usb_slave_start_send(EP2_INDEX);
    usb_slave_data_in(EP2_INDEX, data, data_len);

    while(m_usbcdc.bSending)
    {
        mini_delay(1);
        if(timeout++ > 230)
        {
            akprintf(C1, M_DRVSYS, "time out\n");
            return -1;
        }
    }


    return data_len;
    
}

signed long usbcdc_read(unsigned char *data, unsigned long data_len)
{
    unsigned long i = 0;

    for(i = 0; i < data_len; i++)
    {
        if(m_usbcdc.nReceivePoolTail != m_usbcdc.nReceivePoolHead)
        {
            data[i] = m_usbcdc.pReceivePool[m_usbcdc.nReceivePoolHead];
            m_usbcdc.nReceivePoolHead = (m_usbcdc.nReceivePoolHead + 1) % m_usbcdc.nReceivePoolLength;
        }
        else
        {
            break;
        }
    }     
    //akprintf(C1, M_DRVSYS, "[%d,%d]",i,m_usbcdc.nReceivePoolHead);
    return i;
}

//********************************************************************
#endif

