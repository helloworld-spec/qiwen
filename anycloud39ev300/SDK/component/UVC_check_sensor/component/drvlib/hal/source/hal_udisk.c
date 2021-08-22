/**
 * @filename usb_udisk.c
 * @brief:how to use usb disk.
 *
 * This file describe frameworks of usb disk driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-26
 * @version 1.0
 */
#include <stdio.h>
#include "usb_slave_drv.h"
#include "hal_udisk.h"
#include "hal_usb_s_std.h"
#include "hal_udisk_mass.h"
#include "usb_common.h"
#include "interrupt.h"
#include "akos_api.h"
#include "drv_api.h"
#include "drv_module.h"

#define USB_EXT_MAX_SUPPORT          8
#define SCSI_SEND                    1
#define SCSI_NOSEND                  0
#define FREQ_REQUST_ASIC             140000000

extern volatile unsigned short m_ep_status[];

static T_hFreq freq_handle = FREQ_INVALID_HANDLE;

static volatile T_pUDISK_DEV s_pUdisk = NULL;
static unsigned char configdata[100];
static T_MBOOT s_MassBoot = {0}; 
static unsigned long test_unit_cnt = 0;
static unsigned char first_scsi = 0;
static T_USB_EXT_INFO USB_EXT_CMD_TABLE[USB_EXT_MAX_SUPPORT] = {0};
static volatile bool bCheckTimeout = true;

#define IN_EP_NR EP2_INDEX 
#define OUT_EP_NR   EP3_INDEX
static void udisk_scsi_msg_proc(unsigned long* pMsg,unsigned long len);
static bool udisk_enable();
static bool udisk_disable();
static bool udisk_class_callback(T_CONTROL_TRANS *pTrans);
static bool udisk_bo_reset(T_CONTROL_TRANS *pTrans);
static bool udisk_get_max_lun(T_CONTROL_TRANS *pTrans);
static void udisk_reset(unsigned long mode);
static void udisk_suspend();
static void udisk_resume();
static void udisk_send_finish();
static void udisk_receive_notify();
static void udisk_receive_finish();
static void udisk_configok();
static unsigned char *udisk_get_dev_qualifier_desc(unsigned long *count);
static unsigned char *udisk_get_dev_desc(unsigned long *count);
static unsigned char *udisk_get_cfg_desc(unsigned long *count);
static unsigned char * udisk_get_other_speed_cfg_desc(unsigned long *count);
static unsigned char *udisk_get_str_desc(unsigned char index, unsigned long *count);
static void init_serial_number();
static void  reverse(char   *s); 
static void itoa(unsigned long   n,   char   *s);   
static bool udisk_mass_boot_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_read10_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_write10_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_inquiry_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_read_capacity_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_read_format_capacity_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_mode_sense6_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_request_sense_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_test_unit_ready_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_verify_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_prevent_remove_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_start_stop_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static bool udisk_unsupported_cmd_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res);
static void udisk_error_recovery(void);
static bool udisk_send_csw_proc(unsigned char csw_status,unsigned long residue);


const static T_USB_DEVICE_DESCRIPTOR   s_UdiskDeviceDesc =
{
    0x12,                       ///<Descriptor size is 18 bytes  
    DEVICE_DESC_TYPE,           ///<device descriptor type
    0x0200,                     ///<USB Specification Release Number in binary-coded decimal
    0x00,                       ///<Each interface specifies its own class information  
    0x00,                       ///<Each interface specifies its own Subclass information  
    0x00,                       ///<No protocols the device basis  
    EP0_MAX_PAK_SIZE,           ///<Maximum packet size for endpoint zero is 64  
    0x04d6,                     ///<Vendor ID 
    0xE101,                     ///<Product ID
    0x0100,                     ///< Device release number in binary-coded
    0x1,                        ///<The manufacturer string descriptor index is 1  
    0x2,                        ///<The product string descriptor index is 2  
    #ifndef BURNTOOL
    0x3,                        ///< The serial number string descriptor index is 3  
    #else
    0x0,
    #endif
    0x01                        ///<The device has 1 possible configurations  
};
const static T_USB_DEVICE_QUALIFIER_DESCRIPTOR s_UdiskDeviceQualifierDesc=
{
    sizeof(T_USB_DEVICE_QUALIFIER_DESCRIPTOR),      ///<Descriptor size is 10 bytes  
    DEVICE_QUALIFIER_DESC_TYPE,                     ///<DEVICE_QUALIFIER Descriptor Type  
    0x0200,                                         ///<  USB Specification version 2.00  
    0x00,                                           ///<Each interface specifies its own class information  
    0x00,                                           ///<Each interface specifies its own Subclass information  
    0x00,                                           ///<No protocols the device basis 
    0x40,                                           ///<Maximum packet size for endpoint zero is 64  
    0x01,                                           ///<The device has 1 possible other-speed configurations  
    0                                               ///<Reserved for future use  
};


const static T_USB_CONFIGURATION_DESCRIPTOR s_UdiskConfigDesc =
{
    0x9,                        ///<Descriptor size is 9 bytes  
    CONFIG_DESC_TYPE,           ///<CONFIGURATION Descriptor Type  
    UDISK_DESC_TOTAL_LEN,       ///<The total length of data for this configuration is 32. This includes the combined length of all the descriptors
    0x01,                       ///<This configuration supports 1 interfaces 
    0x01,                       ///< The value 1 should be used to select this configuration  
    0x00,                       ///<The device doesn't have the string descriptor describing this configuration  
    0x80,                       ///<Configuration characteristics : Bit 7: Reserved (set to one) 1 Bit 6: Self-powered 0 Bit 5: Remote Wakeup 0  
    200                         ///<Maximum power consumption of the device in this configuration is 400 mA  
};

const static T_USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR s_UdiskOtherSpeedConfigDesc=
{
    9,                                      ///<Size of this descriptor in bytes
    OTHER_SPEED_CONFIGURATION_DESC_TYPE,    ///<ohter speed CONFIGURATION Descriptor Type
    UDISK_DESC_TOTAL_LEN,                   ///<The total length of data for this configuration is 32. This includes the combined length of all the descriptors
    0x01,                                   ///<This configuration supports 1 interfaces 
    0x01,                                   ///< The value 1 should be used to select this configuration  
    0x00,                                   ///<The device doesn't have the string descriptor describing this configuration  
    0x80,                                   ///<Configuration characteristics : Bit 7: Reserved (set to one) 1 Bit 6: Self-powered 0 Bit 5: Remote Wakeup 0  
    200                                     ///<Maximum power consumption of the device in this configuration is 400 mA  
};

const static T_USB_INTERFACE_DESCRIPTOR s_UdiskInterfacefDesc =
{
    0x9,                            ///<Descriptor size is 9 bytes  
    IF_DESC_TYPE,                   ///<INTERFACE Descriptor Type  
    0x00,                           ///<The number of this interface is 0.  
    0x00,                           ///<The value used to select the alternate setting for this interface is 0  
    0x02,                           ///The number of endpoints used by this interface is 2 (excluding endpoint zero) 
    USB_DEVICE_CLASS_STORAGE,       ///<The interface implements the Mass Storage class  
    USB_MASSCLASS_COMMAND_SCSI,     ///<The interface implements the SCSI Transparent Subclass  
    USB_MASSCLASS_TRAN_BO,          ///<The interface uses the Bulk-Only Protocol 
    0x00                            ///<The device doesn't have a string descriptor describing this iInterface 
};


static T_USB_ENDPOINT_DESCRIPTOR s_UdiskEp3Desc =
{
    7,                                  ///<Descriptor size is 7 bytes  
    EP_DESC_TYPE,                       ///< ENDPOINT Descriptor Type  
    (ENDPOINT_DIR_OUT + OUT_EP_NR),     ///< This is an OUT endpoint with endpoint number 3 
    ENDPOINT_TYPE_BULK,                 ///< Types - Transfer: BULK Pkt Size Adjust: No  
    EP3_BUF_MAX_LEN,                    ///<Maximum packet size for this endpoint is 512 Bytes. If Hi-Speed, 0 additional transactions per frame  
    0x0A                                ///<The polling interval value is every 0 Frames. If Hi-Speed, 0 uFrames/NAK  
};

static T_USB_ENDPOINT_DESCRIPTOR s_UdiskEp2Desc =
{
    7,                                  ///<Descriptor size is 7 bytes 
    EP_DESC_TYPE,                       ///<ENDPOINT Descriptor Type
    (ENDPOINT_DIR_IN + IN_EP_NR),      ///<This is an IN endpoint with endpoint number 2  
    ENDPOINT_TYPE_BULK,                 ///<  Types - Transfer: BULK Pkt Size Adjust: No  
    EP2_BUF_MAX_LEN,                    ///<Maximum packet size for this endpoint is 512 Bytes. If Hi-Speed, 0 additional transactions per frame
    0x00                                ///< The polling interval value is every 0 Frames. If Hi-Speed, 0 uFrames/NAK  
};


/* language descriptor */
const static unsigned char s_UdiskString0[] = 
{
    4,              ///<Descriptor size is 4 bytes  
    3,              ///< Second Byte of this descriptor     
    0x09, 0x04,     ///<Language Id: 1033  
};

/*string descriptor*/
static unsigned char s_UdiskString1[] =
{
    22,             
    0x03,               
    'A',0,          
    'N',0,
    'Y',0,
    'K',0,
    'A',0,
    '.',0,          
    '.',0,
    '.',0,
    '.',0,
    '.',0
};

static unsigned char s_UdiskString2[] = 
{
    22,             
    0x03,               
    'D',0,          
    'I',0,
    'S',0,
    'K',0,
    '.',0,
    '.',0,          
    '.',0,
    '.',0,
    '.',0,
    '.',0
};

static unsigned char s_UdiskString3[] = 
{
    22,             
    0x03,               
    '0',0,          
    '1',0,
    '2',0,
    '3',0,
    '4',0,
    '5',0,          
    '6',0,
    '7',0,
    '8',0,
    '9',0
};

/** 
 * @brief init  str desc with  reference to device desc
 *
 * the str is truncated to 10  characters or less,this func may be called before usbdisk_start
 * @author Huang Xin
 * @date 2010-08-04
 * @param index[in] type of string descriptor
 * @param str[in] the start address of stirng
 * @return  bool
 * @retval false set failed
 * @retval AK_TURE set successful
 */
bool usbdisk_set_str_desc(T_eSTR_DESC index,char *str)
{
    unsigned char i = 0;

    if ( NULL == str)
    {
        akprintf(C1, M_DRVSYS, "str is null\n");
        return false;
    }
    switch (index)
    {
        case STR_MANUFACTURER:
            for (i = 2; i<=20; i+=2)
            {
                if (*str)
                {
                    s_UdiskString1[i] = *str++;
                }
                else
                {
                    s_UdiskString1[i] = ' ';
                }
            }
            break;
        case STR_PRODUCT:
            for (i = 2; i<=20; i+=2)
            {
                if (*str)
                {
                    s_UdiskString2[i] = *str++;
                }
                else
                {
                    s_UdiskString2[i] = ' ';
                }
            }
            break;
        case STR_SERIAL_NUM:
            for (i = 2; i<=20; i+=2)
            {
                if (*str)
                {
                    s_UdiskString3[i] = *str++;
                }
                else
                {
                    s_UdiskString3[i] = ' ';
                }
            }
            break;
        default:
            akprintf(C1, M_DRVSYS, "str type is invalid\n");
            return false;                
    }
    return true;
}


T_USB_EXT_CMD_HANDLER *usb_ext_probe(unsigned char ext_cmd)
{
    unsigned long i, ext_scsi;
    
    for (i = 0; i < USB_EXT_MAX_SUPPORT; i++)
    {
        if (USB_EXT_CMD_TABLE[i].handler!= NULL)
        {
            ext_scsi = USB_EXT_CMD_TABLE[i].ext_scsi;
            if (ext_scsi == ext_cmd)
            { 
                return USB_EXT_CMD_TABLE[i].handler;
            }
        }
    }

    return NULL;
}

bool usb_ext_scsi_reg(unsigned char ext_cmd, T_USB_EXT_CMD_HANDLER *handler)
{
    unsigned long i;
    bool ret = false;
    
    for (i = 0; i < USB_EXT_MAX_SUPPORT; i++)
    {
        // check device register or not 
        if (USB_EXT_CMD_TABLE[i].ext_scsi== ext_cmd)
            break;
        // got an empty place for it 
        if (USB_EXT_CMD_TABLE[i].ext_scsi == 0 &&
            USB_EXT_CMD_TABLE[i].handler == NULL)
        {
            akprintf(C3, M_DRVSYS, "usb reg scsi = 0x%x, cnt = %d\n", ext_cmd, i);
            USB_EXT_CMD_TABLE[i].ext_scsi= ext_cmd;
            USB_EXT_CMD_TABLE[i].handler= handler;
            ret = true;
            break;
        }
    }
    return ret;
}

/**
 * @brief   mass boot init
 *
 * udisk reset,configok,this func must be called because mass boot at usb1.1 will not run enum 
 * @author Huang Xin
 * @date 2010-08-04
 * @param mode[in] usb mode 1.1 or 2.0
 * @return bool
 * @retval false init failed
 * @retval AK_TURE init successful
 */
void usbdisk_mboot_init(unsigned long mode)
{
    udisk_reset(mode);
    udisk_configok();
}
/**
 * @brief   init usb disk function
 *
 * Initialize usb disk buffer,creat usb disk task,creat message queue
 * @author Huang Xin
 * @date 2010-08-04
 * @param mode[in] usb mode 1.1 or 2.0
 * @return bool
 * @retval false init failed
 * @retval AK_TURE init successful
 */

bool usbdisk_init(unsigned long mode)
{
    unsigned char i;
    unsigned char* pBuffer = NULL;
    
    pBuffer = (unsigned char *)drv_malloc(UDISK_BUFFER_LEN*UDISK_BUFFER_NUM);
    if (pBuffer == NULL)
    {
        akprintf(C1, M_DRVSYS, "pBuffer,alloc failed: %x\n", pBuffer);
        return false;
    }       
    s_pUdisk = (T_pUDISK_DEV)drv_malloc(sizeof(T_UDISK_DEV));
    if (s_pUdisk == NULL)
    {
        akprintf(C1, M_DRVSYS, "s_pUdisk,alloc failed: %x\n", pBuffer);
        drv_free(pBuffer);
        return false;
    }   
    //init s_pUdisk member
    memset(s_pUdisk,0,sizeof(T_UDISK_DEV));
    s_pUdisk->ulMode = mode;
    for(i = 0; i < UDISK_BUFFER_NUM; i++)
    {
       s_pUdisk->tTrans.tBuffer[i].pBuffer= pBuffer + i * UDISK_BUFFER_LEN;       
    }
    akprintf(C3, M_DRVSYS, "usbdisk buffer ok, buffer num: %x\n",i);   
    //init_serial_number();

    //map message
    DrvModule_Map_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, udisk_scsi_msg_proc);

    if (0)//(FREQ_INVALID_HANDLE == freq_handle)
    {
        freq_handle = FreqMgr_RequestFreq(FREQ_REQUST_ASIC);
    }
    return true;   
}

/**
 * @brief  enter udisk task,poll udisk msg
 *
 * This function is added for  bios version,and must be call after usbdisk_start
 * @author Huang Xin
 * @date  2010-08-04
 * @param void
 * @return void
 */
void usbdisk_proc(void)
{
#ifndef AKOS
    DrvModule_Retrieve_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG);
#endif
}

/**
 * @brief   start usb disk function, this function must be call after usbdisk_init
 *
 * Allocate L2 buffer , open usb controller,set usb disk callback,register interrupt process function
 * @author Huang Xin
 * @date  2010-08-04
 * @param void
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool usbdisk_start(void)
{
    //create task
    if (!DrvModule_Create_Task(DRV_MODULE_USB_DISK))
        return false;
    if(!udisk_enable())
        return false;
    
    return true;
}

/**
 * @brief   disable usb disk function.
 *
 * Close usb controller,terminate usb disk task,free buffer,free data struct
 * @author Huang Xin
 * @date 2010-08-04
 * @param void
 * @return  void
 */    
void usbdisk_stop(void)
{ 
    DrvModule_Terminate_Task(DRV_MODULE_USB_DISK);
    udisk_disable();
    first_scsi = SCSI_NOSEND;
    if (NULL != s_pUdisk)
    {
        drv_free(s_pUdisk->tTrans.tBuffer[0].pBuffer);
        drv_free(s_pUdisk);
    }

    FreqMgr_CancelFreq(freq_handle);
    freq_handle = FREQ_INVALID_HANDLE;
}

/**
 * @brief   usb slave bulk disk add a lun
 *
 * This function is called when host is  mounting usb disk
 * @author Huang Xin
 * @date 2010-08-04
 * @param pAddLun[in] struct of lun information.
 * @return bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool usbdisk_addLUN(T_pLUN_INFO pAddLun)
{
    if (s_pUdisk->ucLunNum > UDISK_MAXIMUM_LUN)
    {
        return false;
    }
    memcpy((unsigned char *)&s_pUdisk->tLunInfo[s_pUdisk->ucLunNum], (const unsigned char*)pAddLun, sizeof(T_LUN_INFO));
    s_pUdisk->ucLunNum++;
    return true;
}

/**
 * @brief   usb slave bulk disk change lun
 *
 * When sd card is detected, change the lun for sd card
 * @author Huang Xin
 * @date 2010-08-04
 * @param pChgLun[in] struct of lun information.
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool usbdisk_changeLUN(T_pLUN_INFO pChgLun)
{
    unsigned char i;

    if (NULL == pChgLun)
    {
        akprintf(C1, M_DRVSYS, "lun info is null\n");
        return false;
    }
    for(i = 0; i < s_pUdisk->ucLunNum; i++)
    {
        if (s_pUdisk->tLunInfo[i].LunIdx == pChgLun->LunIdx)
        {
            memcpy((unsigned char *)&s_pUdisk->tLunInfo[i], (const unsigned char*)pChgLun, sizeof(T_LUN_INFO));
            return true;
        }
    }
    return false;
}

/**
 * @brief   scsi cmd 'read format capacity' process
 *
 * send SCSI_READ_FORMAT_CAPACITY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_read_format_capacity(unsigned char lun,unsigned long expected_bytes)
{
    T_SCSI_MSG msg;
      
    msg.ucCmd = SCSI_READ_FORMAT_CAPACITY;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send  format cap msg failed\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "send format cap msg ok\n");
    return true;
}

/**
 * @brief   scsi cmd 'read capacity' process
 *
 * send SCSI_READ_CAPACITY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_read_capacity(unsigned char lun,unsigned long expected_bytes)
{
    T_SCSI_MSG msg;
  
    msg.ucCmd = SCSI_READ_CAPACITY;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send read capacity failed\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "send read capacity msg ok\n");
    return true;
}

/**
 * @brief   scsi cmd 'read10' or 'read12' process
 *
 * send SCSI_READ_10 msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param start_sector[in] first LBA addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_read(unsigned char lun,unsigned long start_sector,unsigned long expected_bytes)
{
    T_SCSI_MSG msg;
    
    s_pUdisk->tTrans.ucTransIndex = s_pUdisk->tTrans.ucReadIndex;
    s_pUdisk->tTrans.ulTransBytes = expected_bytes;
    msg.ucCmd = SCSI_READ_10;
    msg.ucLun = lun;
    msg.ulParam2 = start_sector;
    msg.ulParam1  = expected_bytes;  
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send read msg fail\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "(%d,%d,%d)",lun,start_sector,expected_bytes);
    //akprintf(C1, M_DRVSYS, "send read msg ok\n");
    return true; 
}

/**
 * @brief   scsi cmd 'write10' or 'write12' process
 *
 * send SCSI_WRITE_10 msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param start_sector[in] first LBA addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_write(unsigned char lun,unsigned long start_sector,unsigned long expected_bytes)
{
    T_SCSI_MSG msg;
    
    s_pUdisk->tTrans.ucTransIndex = s_pUdisk->tTrans.ucWriteIndex;
    s_pUdisk->tTrans.ulTransBytes = expected_bytes;
    msg.ucCmd = SCSI_WRITE_10;
    msg.ucLun = lun;
    msg.ulParam2 = start_sector;
    msg.ulParam1  = expected_bytes;  
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send scsi write msg fail\n");
        return false;
    }
   // akprintf(C1, M_DRVSYS, "[%d,%d,%d]",lun,start_sector,expected_bytes);
   // akprintf(C1, M_DRVSYS, "send write msg ok, 0x%x\n",s_pUdisk->tTrans.ulTransBytes);
    return true;        
}

/**
 * @brief   scsi cmd 'mode sense' or 'mode sense6' process
 *
 * send SCSI_MODESENSE_6 msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_mode_sense6(unsigned char lun,unsigned long expected_bytes)
{
    T_SCSI_MSG msg;
  
    msg.ucCmd = SCSI_MODESENSE_6;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send mode sense6 msg failed\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "send mode sense6 msg ok\n");
    return true;
}

/**
 * @brief   scsi cmd 'verify' process
 *
 * send SCSI_VERIFY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_verify(unsigned char lun)
{
    T_SCSI_MSG msg;

    msg.ucCmd = SCSI_VERIFY;
    msg.ucLun = lun;
    msg.ulParam1 = 0;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send verify msg failed\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "send verify msg ok\n");
    return true;
}

/**
 * @brief   scsi cmd 'start stop' process
 *
 * send SCSI_START_STOP msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param start_stop[in] start ,stop or eject disk expected by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_start_stop(unsigned char lun,unsigned long start_stop)
{
    T_SCSI_MSG msg;

    msg.ucCmd = SCSI_START_STOP;
    msg.ucLun = lun;
    msg.ulParam1 = start_stop;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send start/stop msg failed\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "send start/stop msg ok\n");
    return true;
}

/**
 * @brief   scsi cmd 'prevent remove' process
 *
 * send 'prevent remove' msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_prevent_remove(unsigned char lun)
{
    T_SCSI_MSG msg;

    msg.ucCmd = SCSI_PREVENT_REMOVE;
    msg.ucLun = lun;
    msg.ulParam1 = 0;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send prevent remove msg failed\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "send verify msg ok\n");
    return true;
}

/**
 * @brief   scsi cmd unsupported process
 *
 * send SCSI_UNSUPPORTED msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_unsupported(unsigned char lun,unsigned long expected_bytes,unsigned char *scsi_addr)
{
    T_SCSI_MSG msg;
    
    udisk_set_sense(INVALID_COMMAND_OPERATION_CODE);
    
    s_pUdisk->tTrans.ulTransBytes = 0;
    msg.ucCmd = SCSI_UNSUPPORTED;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;    
    msg.ulParam2 = (unsigned long)scsi_addr;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send cmd unsupported msg failed\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "send cmd unsupported msg ok\n");
    return true;
}

/**
 * @brief   scsi cmd 'inquiry' process
 *
 * send SCSI_INQUIRY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_inquiry(unsigned char lun,unsigned long expected_bytes)
{
    T_SCSI_MSG msg;

    msg.ucCmd = SCSI_INQUIRY;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send inquiry msg fail\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "send inquiry msg ok\n");
    return true;
}

/**
 * @brief   scsi cmd 'test unit ready' process
 *
 * send SCSI_TEST_UNIT_READY msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_test_unit_ready(unsigned char lun,unsigned long expected_bytes)
{
    T_SCSI_MSG msg;

    s_pUdisk->tTrans.ucTransIndex = s_pUdisk->tTrans.ucWriteIndex;
    s_pUdisk->tTrans.ulTransBytes = expected_bytes;
    msg.ucCmd = SCSI_TEST_UNIT_READY;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send test ready msg failed\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "send test rdy msg ok\n");
    return true;
}

/**
 * @brief   scsi cmd 'request sense' process
 *
 * send SCSI_REQUEST_SENSE msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param lun[in] lun addressed by host
 * @param expected_bytes[in] trans bytes expected by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_req_sense(unsigned char lun,unsigned long expected_bytes)
{
    T_SCSI_MSG msg;

    msg.ucCmd = SCSI_REQUEST_SENSE;
    msg.ucLun = lun;
    msg.ulParam1 = expected_bytes;    
    msg.ulParam2 = 0;
    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send req sense msg failed\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "send req sense msg ok\n");
    return true;
}

static bool udisk_test_unit_ready_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{
    unsigned char stage;
    signed long status;
    unsigned long expected_trans_bytes = 0;

    stage = msc_get_stage();
    
    switch (stage)
    {
        case MSC_STAGE_STATUS:
            if(s_pUdisk->tLunInfo[pMsg->ucLun].sense == MEDIUM_NOTREADY_TO_READY)
            {
                *csw_status = CMD_FAILED;
                udisk_set_sense(NOT_READY_TO_READY_TRANSITION_MEDIA_CHANGED);

                s_pUdisk->tLunInfo[pMsg->ucLun].sense = MEDIUM_NOSENSE;
            }
            else if (s_pUdisk->tLunInfo[pMsg->ucLun].sense == MEDIUM_NOTPRESENT)
            {
                *csw_status = CMD_FAILED;
                udisk_set_sense(MEDIUM_NOT_PRESENT);
            }
            else
            {                
                *csw_status = CMD_PASSED;
                udisk_set_sense(NO_SENSE);
            }
            *res = expected_trans_bytes;
            break;
        //usb-if cv test case9, data out stall may be too late,so call udisk_write10_proc()
        case MSC_STAGE_DATA_OUT :
            if(!udisk_write10_proc(pMsg,csw_status,res))
            {
                return false;
            }
            break;
        default:            
            akprintf(C1, M_DRVSYS, "test unit ready at error stage!\n");
            return false;
    }
    return true;
}

static bool udisk_request_sense_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{    
    unsigned char stage;
    signed long status;
    unsigned long expected_trans_bytes = 0;
    
    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
            if (DRV_MODULE_SUCCESS != status)
            {
                akprintf(C1, M_DRVSYS, "wait  EVENT_USB_TX_FINISH fail ,bufid=0x%x\n", s_pUdisk->tTrans.ucTransIndex);
                return false;
            }
            s_pUdisk->tTrans.ulTransBytes = UDISK_SENSE_LEN;
            if (expected_trans_bytes < UDISK_SENSE_LEN)
            {
                s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
            }
            //abolish pre read data
            s_pUdisk->pre_read_sector_num = 0;
            //produce buffer
            memcpy(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,s_pUdisk->Sense,s_pUdisk->tTrans.ulTransBytes);
            //buf  is ready to tx
            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = true;
            if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
            {
                s_pUdisk->tTrans.enmTransState = TRANS_TX;
                expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                usb_slave_start_send(IN_EP_NR);
                usb_slave_data_in(IN_EP_NR,
                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                    s_pUdisk->tTrans.ulTransBytes);
            }
            else
            {
                akprintf(C1, M_DRVSYS, "trans status error : 0x%x", s_pUdisk->tTrans.enmTransState);
                return false;
            }
            *csw_status = CMD_PASSED;
            *res = expected_trans_bytes;
            break;
        default:            
            akprintf(C1, M_DRVSYS, "request sense at error stage!\n");
            return false;
    }
    return true;
}

/**
 * @brief   set sense that contain error information
 *
 * called when scsi cmd parse or process is finish
 * @author Huang Xin
 * @date 2010-08-04
 * @param sense[in] error information
 * @return  void
 */
void udisk_set_sense(unsigned long sense)
{
    s_pUdisk->Sense[0] = SENSE_VALID | SENSE_ERRORCODE;
    s_pUdisk->Sense[1] = 0;
    //sense key
    s_pUdisk->Sense[2] = (sense>>16)&0xFF;
    //addtional sense length
    s_pUdisk->Sense[7] = 10;
    // Additional Sense Code
    s_pUdisk->Sense[12] = (sense>>8)&0xFF;
    // Additional Sense Code Qualifier
    s_pUdisk->Sense[13] = sense&0xFF;
}

/**
 * @brief   get udisk lun num
 *
 * called when scsi cmd parse
 * @author Huang Xin
 * @date 2010-08-04
 * @return  unsigned char
 */
unsigned char udisk_lun_num(void)
{
    return s_pUdisk->ucLunNum;
}

/**
 * @brief   anyka mass boot cmd  process
 *
 * send mass boot msg
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] scsi struct
 * @param expected_bytes[in] trans bytes expected by host
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool udisk_anyka_mass_boot(unsigned char* scsi_data,unsigned long expected_bytes)
{
    T_SCSI_MSG msg;

    s_pUdisk->tTrans.ucTransIndex = s_pUdisk->tTrans.ucReadIndex = s_pUdisk->tTrans.ucWriteIndex;
    s_pUdisk->tTrans.ulTransBytes = expected_bytes;
    
    msg.ucCmd = SCSI_ANYKA_MASS_BOOT;
    msg.ucLun = 0;
    msg.ulParam1 = expected_bytes;    
    msg.ulParam2 = (unsigned long)scsi_data;

    if (!DrvModule_Send_Message(DRV_MODULE_USB_DISK, UDISK_SCSI_MSG, (unsigned long*)&msg))
    {
        akprintf(C1, M_DRVSYS, "send anyka_mass_boot msg failed\n");
        return false;
    }
    //akprintf(C1, M_DRVSYS, "send cmd unsupported msg ok\n");
    return true;
}

/** 
 * @brief set procduce callback 
 *
 * used by produce
 * @author Huang Xin
 * @date 2010-08-04
 * @param hnd_cmd[in] handle cmd callback
 * @param hnd_send[in] handle send callback
 * @paramhnd_rcv[in] handle receive callback
 * @return  bool
 * @retval false set failed
 * @retval AK_TURE set successful
 */
bool usbdisk_mboot_set_cb(T_fMBOOT_HANDLE_CMD hnd_cmd, T_fMBOOT_HANDLE_SEND hnd_send, T_fMBOOT_HANDLE_RCV hnd_rcv)
{
    if(NULL == hnd_cmd || NULL == hnd_send || NULL == hnd_rcv)
        return false;
    
    s_MassBoot.fMbootCmdCb = hnd_cmd;
    s_MassBoot.fMbootSendCb = hnd_send;
    s_MassBoot.fMbootRcvCb = hnd_rcv;
    return true;
}
static bool udisk_mass_boot_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{
    bool ret = true;
    unsigned char stage;
    unsigned long xfer_bytes = 0;
    unsigned long expected_trans_bytes = 0;
    signed long status;
    
    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;

    //akprintf(C1, M_DRVSYS, "(%d,%d,%d)",*((unsigned char*)pMsg->ulParam2+1),expected_trans_bytes,stage);

    if(NULL != s_MassBoot.fMbootCmdCb)
    {
        ret = s_MassBoot.fMbootCmdCb((unsigned char *)pMsg->ulParam2, expected_trans_bytes);
    }
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            do
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    akprintf(C1, M_DRVSYS, "wait  EVENT_USB_TX_FINISH fail ,bufid=0x%x\n", s_pUdisk->tTrans.ucTransIndex);
                    return false;
                }

                //cmd<=128
                if(*((unsigned char *)pMsg->ulParam2 + 1) <= 128)
                {
                    s_pUdisk->tTrans.ulTransBytes = 13;
                    if (expected_trans_bytes < s_pUdisk->tTrans.ulTransBytes)
                    {
                        s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
                    }  
                    //produce buffer
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+0) = 0x41;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+1) = 0x4e;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+2) = 0x59;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+3) = 0x4b;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+4) = 0x41;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+5) = 0x20;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+6) = 0x44;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+7) = 0x45;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+8) = 0x53;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+9) = 0x49;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+10) = 0x47;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+11) = 0x4e;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+12) = 0x45;

                    //buf  is ready to tx
                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = true;
                    if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
                    {
                        s_pUdisk->tTrans.enmTransState = TRANS_TX;
                        expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                        usb_slave_start_send(IN_EP_NR);
                        usb_slave_data_in(IN_EP_NR,
                                            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                            s_pUdisk->tTrans.ulTransBytes);
                    }
                    else
                    {
                        akprintf(C1, M_DRVSYS, "trans status error : 0x%x", s_pUdisk->tTrans.enmTransState);
                        return false;
                    }
                    if(*csw_status != CMD_FAILED && *csw_status != PHASE_ERROR)
                    {
                        *csw_status = CMD_PASSED;
                    }
                    *res = expected_trans_bytes;
                    return ret;
                }
                
                if (expected_trans_bytes > UDISK_READ_BUF_LEN)
                {
                    xfer_bytes = UDISK_READ_BUF_LEN;
                } 
                else
                {
                    xfer_bytes = expected_trans_bytes;
                }
                //produce buffer                
                if(NULL != s_MassBoot.fMbootSendCb)
                {
                    if (!s_MassBoot.fMbootSendCb((unsigned long)s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].pBuffer, xfer_bytes))
                    {
                        #ifdef BURNTOOL
                        *csw_status = PHASE_ERROR;
                        #else
                        *csw_status = CMD_FAILED;
                        #endif

                    }
                }
    
                //buf  is ready to tx
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].bValidate = true;
                if((s_pUdisk->tTrans.enmTransState == TRANS_IDLE) || (s_pUdisk->tTrans.enmTransState == TRANS_SUSPEND_TX))
                {
                    //continue usb tx
                    s_pUdisk->tTrans.enmTransState = TRANS_TX;
                    usb_slave_start_send(IN_EP_NR);
                    if (s_pUdisk->tTrans.ulTransBytes > UDISK_READ_BUF_LEN )
                    { 
                        
                        usb_slave_data_in( IN_EP_NR,
                                            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                            UDISK_READ_BUF_LEN);                                 
               
                    }
                    else
                    {
                        
                        usb_slave_data_in( IN_EP_NR,
                                            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                            s_pUdisk->tTrans.ulTransBytes);
                               
                    }
                }
                s_pUdisk->tTrans.ucReadIndex = (s_pUdisk->tTrans.ucReadIndex+1)%UDISK_BUFFER_NUM;
                expected_trans_bytes -= xfer_bytes;
            }while(expected_trans_bytes > 0);
            if(*csw_status != CMD_FAILED && *csw_status != PHASE_ERROR)
            {
                *csw_status = CMD_PASSED;
            }
            *res = expected_trans_bytes;
            break;
        case MSC_STAGE_DATA_OUT:
            do
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_RX_FINISH(s_pUdisk->tTrans.ucWriteIndex), 2000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    akprintf(C1, M_DRVSYS, "wait  EVENT_USB_RX_FINISH fail or timeout 0x%x\n", status);
                    return false;
                }
                if(expected_trans_bytes > UDISK_WRITE_BUF_LEN)
                {
                    //trans data size per transfer 
                    xfer_bytes = UDISK_WRITE_BUF_LEN;
                }
                else
                {   
                    //trans data size per transfer 
                    xfer_bytes = expected_trans_bytes;
                }
                if(NULL != s_MassBoot.fMbootRcvCb)
                {
                    if(!s_MassBoot.fMbootRcvCb((unsigned long)s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucWriteIndex].pBuffer, xfer_bytes))
                    {
                        #ifdef BURNTOOL
                        *csw_status = PHASE_ERROR;
                        #else
                        *csw_status = CMD_FAILED;
                        #endif
                    }
                }
                //buf  is ready to rx
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucWriteIndex].bValidate = false; 
                if (s_pUdisk->tTrans.enmTransState == TRANS_SUSPEND_RX)  
                {
                    //continue usb rx
                    s_pUdisk->tTrans.enmTransState = TRANS_RX;
                    if (s_pUdisk->tTrans.ulTransBytes > UDISK_WRITE_BUF_LEN )
                    { 
                        usb_slave_data_out( OUT_EP_NR,
                                            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                            UDISK_WRITE_BUF_LEN);
                                            
                    }
                    else
                    {
                        usb_slave_data_out( OUT_EP_NR,
                                            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                            s_pUdisk->tTrans.ulTransBytes);
                                            
                    }
                   
    
                }
                s_pUdisk->tTrans.ucWriteIndex = (s_pUdisk->tTrans.ucWriteIndex+1)%UDISK_BUFFER_NUM;
                expected_trans_bytes -= xfer_bytes ;
            }while(expected_trans_bytes > 0);
            
            if(*csw_status != CMD_FAILED && *csw_status != PHASE_ERROR)
            {
                *csw_status = CMD_PASSED;
            }
            *res = expected_trans_bytes;          
            break;
        case MSC_STAGE_STATUS:
            *csw_status = CMD_PASSED;
            *res = expected_trans_bytes;  
            break;
        default:
            akprintf(C1, M_DRVSYS, "mboot_cmd at error stage!\n");
            return false;
                
    }

    return ret;
}

static bool udisk_read10_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{    
    unsigned char stage;
    unsigned long xfer_sectors = 0;
    unsigned long xfer_bytes = 0;
    unsigned long start_sector;
    unsigned long expected_trans_bytes = 0;
    unsigned long sectors_per_buf = 0;
    unsigned char* read_buf = NULL;
    signed long status;
    
    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            //all data bytes to be transfered
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
                udisk_set_sense(MEDIUM_NOT_PRESENT);
                *csw_status = CMD_FAILED;
            }
            else
            {
                start_sector = pMsg->ulParam2;
                sectors_per_buf = UDISK_READ_BUF_LEN /s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;
                do
                {
                  
                    status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucReadIndex), 1000);
                    if (DRV_MODULE_SUCCESS != status)
                    {
                        akprintf(C1, M_DRVSYS, "wait  EVENT_USB_TX_FINISH fail ,bufid=0x%x\n", s_pUdisk->tTrans.ucReadIndex);
                        return false;
                    }
                    else
                    {
                        read_buf = s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].pBuffer;
                        if(expected_trans_bytes >= UDISK_READ_BUF_LEN)
                        {
                            //trans data size per transfer 
                            xfer_sectors =  sectors_per_buf;
                            xfer_bytes = UDISK_READ_BUF_LEN;
                            if ((start_sector == s_pUdisk->pre_read_start_sector) && (s_pUdisk->pre_read_sector_num > 0) && (s_pUdisk->pre_read_lun == pMsg->ucLun))
                            {
                                start_sector += s_pUdisk->pre_read_sector_num;
                                xfer_sectors -= s_pUdisk->pre_read_sector_num;
                                read_buf += s_pUdisk->pre_read_sector_num * s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;
                            }
                        }
                        else
                        {
                            //trans data size per transfer 
                            xfer_sectors =  expected_trans_bytes/ s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;
                            xfer_bytes = expected_trans_bytes;
                        }
                        s_pUdisk->pre_read_sector_num = 0;
                        //produce buffer
                        s_pUdisk->tLunInfo[pMsg->ucLun].Read(read_buf, start_sector, xfer_sectors, s_pUdisk->tLunInfo[pMsg->ucLun].LunAddInfo);
                        //buf  is ready to tx
                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].bValidate = true;
                        if((s_pUdisk->tTrans.enmTransState == TRANS_IDLE) || (s_pUdisk->tTrans.enmTransState == TRANS_SUSPEND_TX))
                        {
                            //continue usb tx
                            s_pUdisk->tTrans.enmTransState = TRANS_TX;
                            usb_slave_start_send(IN_EP_NR);
                            if (s_pUdisk->tTrans.ulTransBytes > UDISK_READ_BUF_LEN )
                            { 
                                
                                usb_slave_data_in( IN_EP_NR,
                                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                    UDISK_READ_BUF_LEN);                                 
                       
                            }
                            else
                            {
                                
                                usb_slave_data_in( IN_EP_NR,
                                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                    s_pUdisk->tTrans.ulTransBytes);
                                       
                            }
                        }
                        s_pUdisk->tTrans.ucReadIndex = (s_pUdisk->tTrans.ucReadIndex+1)%UDISK_BUFFER_NUM;
                        
                        //if  <read10> expect 64kbytes, use pre read to improve read speed, and read speed is fastest when pre read size is 32kbytes
                        if (UDISK_READ_BUF_LEN == expected_trans_bytes)
                        {
                            s_pUdisk->pre_read_sector_num = (32*1024)/s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;
                        }
                        expected_trans_bytes -= xfer_bytes ;
                        start_sector += xfer_sectors;

                        if (s_pUdisk->pre_read_sector_num > 0)
                        {
                            //pre read, produce buffer
                            s_pUdisk->pre_read_lun = pMsg->ucLun;
                            s_pUdisk->pre_read_start_sector = start_sector;
                            s_pUdisk->tLunInfo[pMsg->ucLun].Read(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucReadIndex].pBuffer, 
                                                                        s_pUdisk->pre_read_start_sector, 
                                                                        s_pUdisk->pre_read_sector_num, 
                                                                        s_pUdisk->tLunInfo[pMsg->ucLun].LunAddInfo);
                        }
                       
                    }
                }while(expected_trans_bytes > 0);
             
                *csw_status = CMD_PASSED;
            }
            *res = expected_trans_bytes;
            break;
        //usb-if cv test case10,data out stall may be too late,so call udisk_write10_proc()
        case MSC_STAGE_DATA_OUT:
            if(!udisk_write10_proc(pMsg,csw_status,res))
            {
                return false;
            }            
            *csw_status = PHASE_ERROR;
            break;
        default:
            akprintf(C1, M_DRVSYS, "read10 at error stage!\n");
            return false;
                      
    }
    return true;
}

static bool udisk_write10_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{
    unsigned char stage;
    unsigned long xfer_sectors = 0;
    unsigned long xfer_bytes = 0;
    unsigned long start_sector;
    unsigned long expected_trans_bytes = 0;
    unsigned long sectors_per_buf = 0;
    signed long status;

    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)
    {
        case MSC_STAGE_DATA_OUT :
            //abolish pre read data
            s_pUdisk->pre_read_sector_num = 0;
            //LBA at which the write begin
            start_sector = pMsg->ulParam2;
            sectors_per_buf = UDISK_WRITE_BUF_LEN /s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;
			do
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_RX_FINISH(s_pUdisk->tTrans.ucWriteIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    akprintf(C1, M_DRVSYS, "wait  EVENT_USB_RX_FINISH fail or timeout 0x%x\n", status);
                    return false;
                }
                else
                {
                    if(expected_trans_bytes > UDISK_WRITE_BUF_LEN)
                    {
                        //trans data size per transfer 
                        xfer_sectors =  sectors_per_buf;
                        xfer_bytes = UDISK_WRITE_BUF_LEN;
                    }
                    else
                    {   
                        //trans data size per transfer 
                        xfer_sectors =  expected_trans_bytes / s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize;;
                        xfer_bytes = expected_trans_bytes;
                    }
                    
                    s_pUdisk->tLunInfo[pMsg->ucLun].Write(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucWriteIndex].pBuffer, 
                                                                        start_sector, 
                                                                        xfer_sectors, 
                                                                        s_pUdisk->tLunInfo[pMsg->ucLun].LunAddInfo);
                    //buf  is ready to rx
                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucWriteIndex].bValidate = false; 
                    if (s_pUdisk->tTrans.enmTransState == TRANS_SUSPEND_RX)  
                    {
                        //continue usb rx
                        s_pUdisk->tTrans.enmTransState = TRANS_RX;
                        if (s_pUdisk->tTrans.ulTransBytes > UDISK_WRITE_BUF_LEN )
                        { 
                            usb_slave_data_out( OUT_EP_NR,
                                                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                UDISK_WRITE_BUF_LEN);
                                                
                        }
                        else
                        {
                            usb_slave_data_out( OUT_EP_NR,
                                                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                                s_pUdisk->tTrans.ulTransBytes);
                                                
                          
                        }
                    }
                       
                    s_pUdisk->tTrans.ucWriteIndex = (s_pUdisk->tTrans.ucWriteIndex+1)%UDISK_BUFFER_NUM;
                    expected_trans_bytes -= xfer_bytes ;
                    start_sector += xfer_sectors;
                }
                
            }while(expected_trans_bytes > 0);                   
            *csw_status = CMD_PASSED; 
            *res = expected_trans_bytes;
            break;
        //usb-if cv test case8
        case MSC_STAGE_DATA_IN :
            DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
            *csw_status = PHASE_ERROR;             
            *res = expected_trans_bytes;
            break;
        default:
            akprintf(C1, M_DRVSYS, "write10 at error stage!\n");
            return false;
            
    }
    return true;
}

static bool udisk_read_capacity_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{
    unsigned char stage;
    signed long status;
    unsigned long expected_trans_bytes = 0;

    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
                udisk_set_sense(MEDIUM_NOT_PRESENT);
                *csw_status = CMD_FAILED;
            }
            else
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    akprintf(C1, M_DRVSYS, "wait  EVENT_USB_TX_FINISH fail ,bufid=0x%x\n", s_pUdisk->tTrans.ucTransIndex);
                    return false;
                }
                s_pUdisk->tTrans.ulTransBytes = UDISK_CAPACITY_LEN;
                if (expected_trans_bytes < s_pUdisk->tTrans.ulTransBytes)
                {
                    s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
                }  
                //abolish pre read data
                s_pUdisk->pre_read_sector_num = 0;
                //produce buffer
                //last LBA
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+0) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt-1)& 0xFF000000)>>24);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+1) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt-1)& 0xFF0000)>>16);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+2) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt-1)& 0xFF00)>>8);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+3) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt-1)& 0xFF)>>0);
                //blk len in bytes
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+4) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF000000)>>24);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+5) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF0000)>>16);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+6) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF00)>>8);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+7) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF)>>0);
                //buf  is ready to tx
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = true;
                if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
                {
                    s_pUdisk->tTrans.enmTransState = TRANS_TX;
                    expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                    usb_slave_start_send(IN_EP_NR);
                    usb_slave_data_in(IN_EP_NR,
                                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                        s_pUdisk->tTrans.ulTransBytes);
                }
                else
                {
                    akprintf(C1, M_DRVSYS, "trans status error : 0x%x", s_pUdisk->tTrans.enmTransState);
                    return false;
                }                
                *csw_status = CMD_PASSED; 
            }
            *res = expected_trans_bytes;
            break;
        default:            
            akprintf(C1, M_DRVSYS, "read capacity at error stage!\n");
            return false;
    }
    return true;
}

static bool udisk_read_format_capacity_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{
    unsigned char stage;
    signed long status;
    unsigned long expected_trans_bytes = 0;

    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
                udisk_set_sense(MEDIUM_NOT_PRESENT);
                *csw_status = CMD_FAILED;
            }
            else
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    akprintf(C1, M_DRVSYS, "wait  EVENT_USB_TX_FINISH fail ,bufid=0x%x\n", s_pUdisk->tTrans.ucTransIndex);
                    return false;
                }
                s_pUdisk->tTrans.ulTransBytes = UDISK_FORMAT_CAPACITY_LEN;
                if (expected_trans_bytes < s_pUdisk->tTrans.ulTransBytes)
                {
                    s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
                }  
                //abolish pre read data
                s_pUdisk->pre_read_sector_num = 0;
                //produce buffer
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+0) = 0;
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+1) = 0;
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+2) = 0;
                //capacity list len
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+3) = UDISK_FORMAT_CAPACITY_LEN-4;
                //blk num
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+4) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt)& 0xFF000000)>>24);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+5) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt)& 0xFF0000)>>16);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+6)= (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt)& 0xFF00)>>8);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+7) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkCnt)& 0xFF)>>0);
                //desciptor of media,01=unformatted,02=formatted,no Cartridge in dirver
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+8) = 0x2;
                //blk len in bytes
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+9) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF0000)>>16);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+10)= (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF00)>>8);
                *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+11) = (unsigned char)(((s_pUdisk->tLunInfo[pMsg->ucLun].BlkSize)& 0xFF)>>0);
                //buf  is ready to tx
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = true;
                if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
                {
                    s_pUdisk->tTrans.enmTransState = TRANS_TX;
                    expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                    usb_slave_start_send(IN_EP_NR);
                    usb_slave_data_in(IN_EP_NR,
                                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                        s_pUdisk->tTrans.ulTransBytes);
                }
                else
                {
                    akprintf(C1, M_DRVSYS, "trans status error : 0x%x", s_pUdisk->tTrans.enmTransState);
                    return false;
                }                
                *csw_status = CMD_PASSED; 
            }
            *res = expected_trans_bytes;
            break;
        default:            
            akprintf(C1, M_DRVSYS, "read format capacity at error stage!\n");
            return false;
    }
    return true;
}

static bool udisk_mode_sense6_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{
    unsigned char stage;
    signed long status;
    unsigned long expected_trans_bytes = 0;

    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
                udisk_set_sense(MEDIUM_NOT_PRESENT);
                *csw_status = CMD_FAILED;
            }
            else
            {
                status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
                if (DRV_MODULE_SUCCESS != status)
                {
                    akprintf(C1, M_DRVSYS, "wait  EVENT_USB_TX_FINISH fail ,bufid=0x%x\n", s_pUdisk->tTrans.ucTransIndex);
                    return false;
                }
                s_pUdisk->tTrans.ulTransBytes = UDISK_MODE_PARAMETER_LEN;
                if (expected_trans_bytes < s_pUdisk->tTrans.ulTransBytes)
                {
                    s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
                }  
                //abolish pre read data
                s_pUdisk->pre_read_sector_num = 0;
                //judge read only 
                if (READ_ONLY == s_pUdisk->tLunInfo[pMsg->ucLun].uAttribute)
                {
					akprintf(C1, M_DRVSYS, "===READ_ONLY====\n");
					*(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+0) = UDISK_MODE_PARAMETER_LEN-1;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+1) = 0;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+2) = 0x80;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+3) = 0;
                }
                //produce buffer
                else 
                {
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+0) = UDISK_MODE_PARAMETER_LEN-1;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+1) = 0;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+2) = 0;
                    *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+3) = 0;
                }
                //buf  is ready to tx
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = true;
                if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
                {
                    s_pUdisk->tTrans.enmTransState = TRANS_TX;
                    expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                    usb_slave_start_send(IN_EP_NR);
                    usb_slave_data_in(IN_EP_NR,
                                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                        s_pUdisk->tTrans.ulTransBytes);
                }
                else
                {
                    akprintf(C1, M_DRVSYS, "trans status error : 0x%x", s_pUdisk->tTrans.enmTransState);
                    return false;
                }
                *csw_status = CMD_PASSED; 
            }
            *res = expected_trans_bytes;
            break;
        default:            
            akprintf(C1, M_DRVSYS, "mode sense6 at error stage!\n");
            return false;
    }
    return true;
}

static bool udisk_verify_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{
    unsigned char stage;
    signed long status;
    unsigned long expected_trans_bytes = 0;

    stage = msc_get_stage();
    switch (stage)
    {
        case MSC_STAGE_STATUS:
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                *csw_status = CMD_FAILED;
                udisk_set_sense(MEDIUM_NOT_PRESENT);
            }
            else
            {                
                *csw_status = CMD_PASSED;
                udisk_set_sense(NO_SENSE);
            }
            *res = expected_trans_bytes;
            break;
        default:            
            akprintf(C1, M_DRVSYS, "verify at error stage!\n");
            return false;
    }
    return true;
}

static bool udisk_prevent_remove_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{
    unsigned char stage;
    signed long status;
    unsigned long expected_trans_bytes = 0;

    stage = msc_get_stage();
    switch (stage)
    {
        case MSC_STAGE_STATUS:
            if (s_pUdisk->tLunInfo[pMsg->ucLun].sense != MEDIUM_NOSENSE)
            {
                *csw_status = CMD_FAILED;
                udisk_set_sense(MEDIUM_NOT_PRESENT);
            }
            else
            {                
                *csw_status = CMD_PASSED;
                udisk_set_sense(NO_SENSE);
            }
            *res = expected_trans_bytes;
            break;
        default:            
            akprintf(C1, M_DRVSYS, "prevent remove at error stage!\n");
            return false;
    }
    return true;
}

static bool udisk_start_stop_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{
    unsigned char stage;
    signed long status;
    unsigned long expected_trans_bytes = 0;

    stage = msc_get_stage();
    switch (stage)
    {
        case MSC_STAGE_STATUS:
            //eject media
            if (UDISK_EJECT_MEDIA == pMsg->ulParam1)
            {
                s_pUdisk->tLunInfo[pMsg->ucLun].sense = MEDIUM_NOTPRESENT;
                usb_slave_set_state(USB_START_STOP);
            }
            //start media
            else if (UDISK_START_MEDIA == pMsg->ulParam1 )
            {
                s_pUdisk->tLunInfo[pMsg->ucLun].sense = MEDIUM_NOSENSE;
            }
            //stop media
            else
            {
                 s_pUdisk->tLunInfo[pMsg->ucLun].sense = MEDIUM_NOTREADY;
            }
            *csw_status = CMD_PASSED;
            *res = expected_trans_bytes;
            break;
        default:            
            akprintf(C1, M_DRVSYS, "start stop at error stage!\n");
            return false;
    }
    return true;
}
static bool udisk_inquiry_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{
    unsigned char stage;
    signed long status;
    unsigned long expected_trans_bytes = 0;

    stage = msc_get_stage();
    expected_trans_bytes = pMsg->ulParam1;
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
            if (DRV_MODULE_SUCCESS != status)
            {
                akprintf(C1, M_DRVSYS, "wait  EVENT_USB_TX_FINISH fail ,bufid=0x%x\n", s_pUdisk->tTrans.ucTransIndex);
                return false;
            }
            s_pUdisk->tTrans.ulTransBytes = UDISK_INQ_STR_LEN;
            if (expected_trans_bytes < s_pUdisk->tTrans.ulTransBytes)
            {
                s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
            }  
            //abolish pre read data
            s_pUdisk->pre_read_sector_num = 0;
            //produce buffer
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+0) = 0x00;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+1) = 0x80;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+2) = 0x02;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+3) = 0x02;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+4) = 0x1f;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+5) = 0x00;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+6) = 0x00;
            *(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+7) = 0x00;
            memcpy(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+8,s_pUdisk->tLunInfo[pMsg->ucLun].Vendor,INQUIRY_STR_VENDOR_SIZE);
            memcpy(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+16,s_pUdisk->tLunInfo[pMsg->ucLun].Product,INQUIRY_STR_PRODUCT_SIZE);
            memcpy(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer+32,s_pUdisk->tLunInfo[pMsg->ucLun].Revision,INQUIRY_STR_REVISION_SIZE);
            //buf  is ready to tx
            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = true;
            if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
            {
                s_pUdisk->tTrans.enmTransState = TRANS_TX;
                expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                usb_slave_start_send(IN_EP_NR);
                usb_slave_data_in(IN_EP_NR,
                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                    s_pUdisk->tTrans.ulTransBytes);
            }
            else
            {
                akprintf(C1, M_DRVSYS, "trans status error : 0x%x", s_pUdisk->tTrans.enmTransState);
                return false;
            }
            *csw_status = CMD_PASSED; 
            *res = expected_trans_bytes;
            break;
        default:            
            akprintf(C1, M_DRVSYS, "inquiry at error stage!\n");
            return false;
    }
    return true;
}

static bool udisk_unsupported_cmd_proc(T_pSCSI_MSG pMsg,unsigned char* csw_status,unsigned long* res)
{
    unsigned char stage;
    unsigned char *scsi_data=NULL;
    unsigned long status;
    unsigned long expected_trans_bytes = 0;
    T_USB_EXT_CMD_HANDLER *fcb_handle=NULL;

    expected_trans_bytes = pMsg->ulParam1;
    scsi_data = (unsigned char *)pMsg->ulParam2;
    stage = msc_get_stage();

    if (NULL != scsi_data)
    {
         fcb_handle = usb_ext_probe(scsi_data[0]);
    }
    *csw_status = CMD_FAILED;

    switch (stage)
    {
        case MSC_STAGE_DATA_OUT:
            
            //bug:data out stall may be too late
            usb_slave_ep_stall(OUT_EP_NR);
            //wait clr stall
            while(1 == usb_slave_get_ep_status(OUT_EP_NR));
            msc_set_stage(MSC_STAGE_STATUS);
            break;
            
        case MSC_STAGE_DATA_IN:
            //check cb pointer
            if ((NULL == fcb_handle)||(NULL == fcb_handle->ext_callback)||(stage != fcb_handle->stage))
                goto ERROR_EXIT;

            status = DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex), 1000);
            if (DRV_MODULE_SUCCESS != status)
            {
                akprintf(C1, M_DRVSYS, "wait  EVENT_USB_TX_FINISH fail ,bufid=0x%x\n", s_pUdisk->tTrans.ucTransIndex);
                return false;
            }
            
            s_pUdisk->tTrans.ulTransBytes = expected_trans_bytes;
            //set buf from cb
            if (false == fcb_handle->ext_callback(s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,expected_trans_bytes))
                goto ERROR_EXIT;
                
            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = true;

            if(s_pUdisk->tTrans.enmTransState == TRANS_IDLE) 
            {
                s_pUdisk->tTrans.enmTransState = TRANS_TX;
                expected_trans_bytes -= s_pUdisk->tTrans.ulTransBytes;
                usb_slave_start_send(IN_EP_NR);
                usb_slave_data_in(IN_EP_NR,
                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                    s_pUdisk->tTrans.ulTransBytes);
                                  
            }
            else
            {
                akprintf(C1, M_DRVSYS, "trans status error : 0x%x", s_pUdisk->tTrans.enmTransState);
                return false;
            }                
            *csw_status = CMD_PASSED; 
            *res = expected_trans_bytes;
            
            //error handle
            ERROR_EXIT:            
                if (CMD_FAILED == *csw_status)
                {
                    usb_slave_ep_stall(IN_EP_NR);
                    //wait clr stall
                    while(1 == usb_slave_get_ep_status(IN_EP_NR));            
                    //set msc status
                    msc_set_stage(MSC_STAGE_STATUS);
                }
            break;                

        case MSC_STAGE_STATUS:
        
            if ((NULL != fcb_handle)&&(NULL!=fcb_handle->ext_callback))
            {
                if (fcb_handle->ext_callback(scsi_data, USB_EXT_SCSI_PASSWORD_NO))
                {
                    *csw_status = CMD_PASSED;
                }
                else
                {
                    *csw_status = PHASE_ERROR;
                }       
                             
            }
            else
            {
                usb_slave_ep_stall(IN_EP_NR);
                //wait clr stall
                while(1 == usb_slave_get_ep_status(IN_EP_NR));
                msc_set_stage(MSC_STAGE_STATUS);
            }
            break;        
        default: 
            akprintf(C1, M_DRVSYS, "unsupported cmd at error stage!\n");
            return false;
    }

    *res = expected_trans_bytes;       
    return true;
}

static bool udisk_send_csw_proc(unsigned char csw_status,unsigned long residue)
{
    unsigned char stage;

    stage = msc_get_stage();
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE, 1000);
            //device real data is less than host expected data
            if (residue > 0)
            {
                usb_slave_ep_stall(IN_EP_NR);
                //wait clr stall
                while(1 == usb_slave_get_ep_status(IN_EP_NR));
            }   
            //data stage finish,send csw        
            msc_set_stage(MSC_STAGE_STATUS);
            msc_send_csw(csw_status,residue);
            break;
        case MSC_STAGE_DATA_OUT:
            DrvModule_WaitEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE, 1000);
            //device real data is less than host expected data
            if (residue > 0)
            {                    
                //bug:data out stall may be too late
                usb_slave_ep_stall(OUT_EP_NR);
                //wait clr stall
                while(1 == usb_slave_get_ep_status(OUT_EP_NR));
            }                   
            msc_set_stage(MSC_STAGE_STATUS);
            msc_send_csw(csw_status,residue);    
            break;
        case MSC_STAGE_STATUS:
            msc_send_csw(csw_status,residue);
            break;
        default:
            akprintf(C1, M_DRVSYS, "send csw at error stage!\n");
            return false;    
    }
    return true;
}

static void udisk_error_recovery(void)
{    
    akprintf(C1, M_DRVSYS, "error recovery!\n");
    usb_slave_std_hard_stall(true);
    usb_slave_ep_stall(IN_EP_NR);
    usb_slave_ep_stall(OUT_EP_NR);
}
/**
 * @brief   scsi msg process 
 *
 * called by udisk task when scsi msg sent successful
 * @author Huang Xin
 * @date 2010-08-04
 * @param pMsg[in] scsi msg parameter
 * @param len[in] scsi msg parameter len
 * @return  void
 */
static void udisk_scsi_msg_proc(unsigned long* pMsg,unsigned long len)
{  
    bool ret;
    T_pSCSI_MSG message = (T_pSCSI_MSG)pMsg;
    unsigned char csw_status = 0;
    unsigned long residue = 0;
    
    switch (message->ucCmd)
    {       
        case SCSI_READ_FORMAT_CAPACITY:
            ret = udisk_read_format_capacity_proc(message, &csw_status, &residue);
            break;
        case SCSI_READ_CAPACITY:
            ret = udisk_read_capacity_proc(message, &csw_status, &residue);
            break;     
        case SCSI_READ_10:
            ret = udisk_read10_proc(message, &csw_status, &residue);
            break;
        case SCSI_WRITE_10:
            ret = udisk_write10_proc(message, &csw_status, &residue);
            break;
        case SCSI_MODESENSE_6:
            ret = udisk_mode_sense6_proc(message, &csw_status, &residue);
            break;   
        case SCSI_VERIFY:
            ret = udisk_verify_proc(message, &csw_status, &residue);
            break;
        case SCSI_START_STOP:
            ret = udisk_start_stop_proc(message, &csw_status, &residue);
            break;
        case SCSI_PREVENT_REMOVE:
            ret = udisk_prevent_remove_proc(message, &csw_status, &residue);
            break;
        case SCSI_INQUIRY:
            ret = udisk_inquiry_proc(message, &csw_status, &residue);
            break;
        case SCSI_TEST_UNIT_READY:
            ret = udisk_test_unit_ready_proc(message, &csw_status, &residue);
            break;
        case SCSI_REQUEST_SENSE:
            ret = udisk_request_sense_proc(message, &csw_status, &residue);
            break; 
        case SCSI_UNSUPPORTED:
            ret = udisk_unsupported_cmd_proc(message, &csw_status, &residue);
            break;    
        default:
            ret = udisk_mass_boot_proc(message, &csw_status, &residue);
            break;
    }
    if (ret)
    {
        if(!udisk_send_csw_proc(csw_status,residue))
        {
            udisk_error_recovery();
        }
        if(SCSI_ANYKA_MASS_BOOT == message->ucCmd)
        {
            //wait csw send finish
            while ( MSC_STAGE_STATUS == msc_get_stage());
            if(NULL != s_MassBoot.fMbootCmdCb)
                s_MassBoot.fMbootCmdCb(NULL,message->ulParam1);
        }
    }
    else
    {
        udisk_error_recovery();
    }   

    first_scsi = SCSI_SEND;
    test_unit_cnt = get_tick_count();
}

/**
 * @brief   udisk enable 
 *
 * called by usbdisk_start() when start udisk task successful
 * @author Huang Xin
 * @date 2010-08-04
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
static bool udisk_enable()
{
    Usb_Slave_Standard.usb_get_device_descriptor = udisk_get_dev_desc;
    Usb_Slave_Standard.usb_get_config_descriptor = udisk_get_cfg_desc;
    Usb_Slave_Standard.usb_get_string_descriptor = udisk_get_str_desc;
    Usb_Slave_Standard.usb_get_device_qualifier_descriptor = udisk_get_dev_qualifier_desc;
    Usb_Slave_Standard.usb_get_other_speed_config_descriptor = udisk_get_other_speed_cfg_desc;
    Usb_Slave_Standard.Device_ConfigVal =           0;
    Usb_Slave_Standard.Device_Address =             0;
    Usb_Slave_Standard.Buffer  =             (unsigned char *)drv_malloc(4096); 
    Usb_Slave_Standard.buf_len =            4096;
    
    //usb slave init,alloc L2 buffer,register irq
    usb_slave_init(Usb_Slave_Standard.Buffer, Usb_Slave_Standard.buf_len);
    //usb std init,set ctrl callback
    usb_slave_std_init();
    //set class req callback
    usb_slave_set_ctrl_callback(REQUEST_CLASS, udisk_class_callback);    
    usb_slave_set_callback(udisk_reset,udisk_suspend,udisk_resume, udisk_configok);
    usb_slave_set_tx_callback(IN_EP_NR, udisk_send_finish);
    usb_slave_set_rx_callback(OUT_EP_NR, udisk_receive_notify, udisk_receive_finish);
    usb_slave_device_enable(s_pUdisk->ulMode);
    return true;
}

/**
 * @brief   udisk disable 
 *
 * called by usbdisk_stop() when terminate udisk task successful
 * @author Huang Xin
 * @date 2010-08-04
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
static bool udisk_disable()
{
    drv_free(Usb_Slave_Standard.Buffer);
    memset(&Usb_Slave_Standard,0,sizeof(Usb_Slave_Standard));
    
    usb_slave_free();
    usb_slave_device_disable();
    
    //clear  callback
    usb_slave_set_ctrl_callback(REQUEST_CLASS, NULL);   
    usb_slave_set_callback(NULL,NULL,NULL, NULL);
    usb_slave_set_tx_callback(IN_EP_NR, NULL);
    usb_slave_set_rx_callback(OUT_EP_NR, NULL, NULL);
    
    return true;
}

/**
 * @brief   set udisk class req callback
 *
 * called by usb drv  when msc class req is received successful
 * @author Huang Xin
 * @date 2010-08-04
 * @param pTrans[in] ctrl trans struct
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
static bool udisk_class_callback(T_CONTROL_TRANS *pTrans)
{
    unsigned char req_type;

    req_type = (pTrans->dev_req.bmRequestType >> 5) & 0x3;
    if (req_type != REQUEST_CLASS)
        return false;
    switch(pTrans->dev_req.bRequest)
    {
        //mass storage bulk only reset
        case 0xFF:
            udisk_bo_reset(pTrans);        
            break;
        //mass storage bulk only get max lun
        case 0xFE:
            udisk_get_max_lun(pTrans);
            break;
        default:
            break;
    }
    return true;
    
}

/**
 * @brief   msc bulk only reset
 *
 * called  when msc class req is bulk only reset
 * @author Huang Xin
 * @date 2010-08-04
 * @param pTrans[in] ctrl trans struct
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
static bool udisk_bo_reset(T_CONTROL_TRANS *pTrans)
{
    if(pTrans->stage == CTRL_STAGE_STATUS)
    {
        usb_slave_std_hard_stall(false);
        udisk_reset(s_pUdisk->ulMode);
        msc_set_stage(MSC_STAGE_READY);
    }
    return true;
    
}

/**
 * @brief   msc get max lun
 *
 * called  when msc class req is get max lun
 * @author Huang Xin
 * @date 2010-08-04
 * @param pTrans[in] ctrl trans struct
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
static bool udisk_get_max_lun(T_CONTROL_TRANS *pTrans)
{
    //only handle in setup stage
    if(pTrans->stage != CTRL_STAGE_SETUP)
    {
        return true;
    }
    akprintf(C3, M_DRVSYS, "udisk max lun is 0x%x\n",s_pUdisk->ucLunNum-1);
    if(s_pUdisk->ucLunNum > 0)
    {
        pTrans->buffer[0] = (s_pUdisk->ucLunNum-1);
    }
    else
    {
        pTrans->buffer[0] = 0;
    }
    pTrans->data_len = 1;
    return true; 
}

bool udisk_get_testunit_state(void)
{    
    unsigned long tmp;

    if(bCheckTimeout == false)
        return usb_slave_getstate();
    tmp = get_tick_count();
    
    if (SCSI_SEND != first_scsi)
        return usb_slave_getstate();       
                
    
    if (tmp > test_unit_cnt)
    {
        if (tmp - test_unit_cnt >5000)
        {
            //msc_get_inquire_cnt:收到SCSI_TEST_UNIT_READY命令的次数
            //msc_get_inquire_cnt:win98为2,macos为3
            if(msc_get_inquire_cnt() > 5)//win7,winxp
            {
                usb_slave_set_state(USB_TEST_UNIT_STOP);
            }
            else
            {
                bCheckTimeout = false;
            }
            
        }
    }
    else
    {
        test_unit_cnt = tmp;
    }   
    
    return usb_slave_getstate();
}

/**
 * @brief  reset callback
 *
 * called  when usb reset
 * @author Huang Xin
 * @date 2010-08-04
 * @param mode[in] usb1.1 or usb2.0
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
static void udisk_reset(unsigned long mode)
{
    unsigned long i;

    //clr stall status to avoid die when udisk task is waiting clr stall to send csw 
    usb_slave_set_ep_status(IN_EP_NR,0);
    usb_slave_set_ep_status(OUT_EP_NR,0);
    usb_slave_std_hard_stall(false);
    if(mode == USB_MODE_20)
    {
        s_UdiskEp2Desc.wMaxPacketSize = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;
        s_UdiskEp3Desc.wMaxPacketSize = EP_BULK_HIGHSPEED_MAX_PAK_SIZE;
    }
    else
    {
        s_UdiskEp2Desc.wMaxPacketSize = EP_BULK_FULLSPEED_MAX_PAK_SIZE;
        s_UdiskEp3Desc.wMaxPacketSize = EP_BULK_FULLSPEED_MAX_PAK_SIZE;   
    }
    //reinit s_pUdisk member,usb1.1 or usb2.0
    s_pUdisk->ulMode = mode;  
    s_pUdisk->tTrans.ulTransBytes = 0;
    s_pUdisk->tTrans.ucReadIndex = 0;
    s_pUdisk->tTrans.ucWriteIndex = 0;
    s_pUdisk->tTrans.ucTransIndex = 0;
    s_pUdisk->tTrans.enmTransState = 0;
    for (i = 0; i<UDISK_BUFFER_NUM; i++)
    {
        s_pUdisk->tTrans.tBuffer[i].bValidate = false;
    }

    msc_set_stage(MSC_STAGE_READY);
    msc_reset_inquire_cnt();
    bCheckTimeout = true;
}

/**
 * @brief  suspend callback
 *
 * called  when usb suspend
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void udisk_suspend()
{
    akprintf(C3, M_DRVSYS, "udisk suspend \n");
    //clr stall status to avoid die when udisk task is waiting clr stall to send csw 
    usb_slave_set_ep_status(IN_EP_NR,0);
    usb_slave_set_ep_status(OUT_EP_NR,0);
    usb_slave_std_hard_stall(false);
}

/**
 * @brief  resume callback
 *
 * called  when usb resume
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void udisk_resume()
{
    akprintf(C3, M_DRVSYS, "udisk resume \n");
}

/**
 * @brief  bulk in ep send finish callback
 *
 * called  when bulk in ep send finish
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void udisk_send_finish()
{
    unsigned char stage;
    stage = msc_get_stage();
    switch (stage)
    {
        case MSC_STAGE_DATA_IN:
            if(!DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(s_pUdisk->tTrans.ucTransIndex)))
            {
                akprintf(C1, M_DRVSYS, "set EVENT_USB_TX_FINISH fail bufid=0x%x\n",s_pUdisk->tTrans.ucTransIndex);
                break;
            }
            s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = false;
            s_pUdisk->tTrans.ucTransIndex = (s_pUdisk->tTrans.ucTransIndex+1)%UDISK_BUFFER_NUM;
            if (s_pUdisk->tTrans.ulTransBytes > UDISK_READ_BUF_LEN )
            { 
                s_pUdisk->tTrans.ulTransBytes -= UDISK_READ_BUF_LEN;
                if (s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate)
                {
                    usb_slave_start_send(IN_EP_NR);
                    if (s_pUdisk->tTrans.ulTransBytes > UDISK_READ_BUF_LEN )
                    { 
                        usb_slave_data_in(IN_EP_NR,
                                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                        UDISK_READ_BUF_LEN);
                    }
                    else
                    {
                        usb_slave_data_in(IN_EP_NR,
                                        s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                        s_pUdisk->tTrans.ulTransBytes);                  
                    }
                    
                }
                else
                {
                    //no data to send
                    s_pUdisk->tTrans.enmTransState= TRANS_SUSPEND_TX;
                }
            }
            else
            {
                s_pUdisk->tTrans.ulTransBytes = 0;
                s_pUdisk->tTrans.enmTransState = TRANS_IDLE;
                DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
            }    
            break;
        case MSC_STAGE_STATUS:
            msc_set_stage(MSC_STAGE_READY);
            break;
        default:
            akprintf(C1, M_DRVSYS, "tx finish at error stage 0x%x\n", stage);
            break;           
        
    }
}

/**
 * @brief  bulk out ep receive start callback
 *
 * called  when bulk out ep receive start
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void udisk_receive_notify()
{
    T_SCSI_MSG pMsg;
    unsigned long rcv_cnt = 0 ;
    unsigned char cbw[512]={0};
    unsigned char stage;

    stage = msc_get_stage();
	//akprintf(C1, M_DRVSYS,"rn=%d\n",stage);
    switch (stage)
    {    
        case MSC_STAGE_READY:
            //start usb rx cbw
            s_pUdisk->tTrans.enmTransState = TRANS_RX;
            msc_set_stage(MSC_STAGE_COMMAND);
            rcv_cnt = usb_slave_data_out(OUT_EP_NR, cbw, CBW_PKT_SIZE);
            msc_parse_cbw(cbw, rcv_cnt);
            break;
        case MSC_STAGE_DATA_OUT:
            if (!s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate)
            {
                //start usb rx data
                s_pUdisk->tTrans.enmTransState = TRANS_RX;
                if (s_pUdisk->tTrans.ulTransBytes > UDISK_WRITE_BUF_LEN )
                {      
                    usb_slave_data_out(OUT_EP_NR,
                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                    UDISK_WRITE_BUF_LEN);
                }
                else
                {
                    usb_slave_data_out(OUT_EP_NR,
                                    s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].pBuffer,
                                    s_pUdisk->tTrans.ulTransBytes);                  
                }
                
            }
            else
            {
                //no buffer to reveive
                s_pUdisk->tTrans.enmTransState= TRANS_SUSPEND_RX;
            }
            break;
        
        default:
            akprintf(C1, M_DRVSYS, "rx notify at error stage 0x%x\n", stage);
            break;
    }
}

/**
 * @brief  bulk out ep receive finish callback
 *
 * called  when bulk out ep receive finish
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void udisk_receive_finish()
{
    unsigned char stage;

    stage = msc_get_stage();
	//akprintf(C1, M_DRVSYS, "==rf,%d\n",stage);
    switch (stage)
    {
        case MSC_STAGE_COMMAND:
            s_pUdisk->tTrans.enmTransState= TRANS_IDLE;
            break;
        case MSC_STAGE_DATA_OUT:
            if(!DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_USB_RX_FINISH(s_pUdisk->tTrans.ucTransIndex)))
            {
                akprintf(C1, M_DRVSYS, "set EVENT_USB_RX_FINISH fail,bufid = 0x%x\n",s_pUdisk->tTrans.ucTransIndex);
            }
            else
            {
                
                if (s_pUdisk->tTrans.ulTransBytes > UDISK_WRITE_BUF_LEN )
                { 
                    s_pUdisk->tTrans.ulTransBytes -= UDISK_WRITE_BUF_LEN;
                }
                else
                {
                    s_pUdisk->tTrans.ulTransBytes = 0;
                    s_pUdisk->tTrans.enmTransState= TRANS_IDLE;
                    DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_STATUS_STAGE);
                }
                s_pUdisk->tTrans.tBuffer[s_pUdisk->tTrans.ucTransIndex].bValidate = true;
                s_pUdisk->tTrans.ucTransIndex = (s_pUdisk->tTrans.ucTransIndex+1)%UDISK_BUFFER_NUM;
            }
            break;
        default:
            akprintf(C1, M_DRVSYS, "rx finish at error stage 0x%x\n", stage);
            break;
    }
}

/**
 * @brief config ok callback
 *
 * called  when enum successful
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void udisk_configok()
{
    unsigned char i;
    
    akprintf(C3, M_DRVSYS, "ok!\n");
    //clear all buffer
    for(i=0; i<UDISK_BUFFER_NUM;i++)
    {
        if(!DrvModule_SetEvent(DRV_MODULE_USB_DISK,EVENT_USB_TX_FINISH(i)))
        {
            akprintf(C1, M_DRVSYS, "before read, set EVENT_USB_TX_FINISH fail bufid=0x%x\n",i);
        }
    }
    msc_set_stage(MSC_STAGE_READY);
}

/** 
 * @brief get dev qualifier descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of dev qualifier descriptor
 * @return  unsigned char *
 * @retval  addr of dev qualifier descriptor
 */
static unsigned char *udisk_get_dev_qualifier_desc(unsigned long *count)
{
    *count = sizeof(s_UdiskDeviceQualifierDesc);
    return (unsigned char *)&s_UdiskDeviceQualifierDesc;
}

/** 
 * @brief get dev descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of dev descriptor
 * @return  unsigned char *
 * @retval  addr of dev descriptor
 */
static unsigned char *udisk_get_dev_desc(unsigned long *count)
{
    *count = sizeof(s_UdiskDeviceDesc);
    return (unsigned char *)&s_UdiskDeviceDesc;
}

/** 
 * @brief get all config descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of all config descriptor
 * @return  unsigned char *
 * @retval  addr of all config descriptor
 */
static unsigned char *udisk_get_cfg_desc(unsigned long *count)
{
    unsigned long cnt = 0;
    
    memcpy(configdata, (unsigned char *)&s_UdiskConfigDesc, *(unsigned char *)&s_UdiskConfigDesc);
    cnt += *(unsigned char *)&s_UdiskConfigDesc;
    memcpy(configdata + cnt, (unsigned char *)&s_UdiskInterfacefDesc, *(unsigned char *)&s_UdiskInterfacefDesc);
    cnt += *(unsigned char *)&s_UdiskInterfacefDesc;
    memcpy(configdata + cnt, (unsigned char *)&s_UdiskEp2Desc, *(unsigned char *)&s_UdiskEp2Desc);
    cnt += *(unsigned char *)&s_UdiskEp2Desc;
    memcpy(configdata + cnt, (unsigned char *)&s_UdiskEp3Desc, *(unsigned char *)&s_UdiskEp3Desc);
    cnt += *(unsigned char *)&s_UdiskEp3Desc;   
    *count = cnt;
    
    return configdata;
}

/** 
 * @brief get other speed config descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of other speed config descriptor
 * @return  unsigned char *
 * @retval  addr of other speed config descriptor
 */
static unsigned char * udisk_get_other_speed_cfg_desc(unsigned long *count)
{
    unsigned long cnt = 0;
    
    memcpy(configdata, (unsigned char *)&s_UdiskOtherSpeedConfigDesc, *(unsigned char *)&s_UdiskOtherSpeedConfigDesc);
    cnt += *(unsigned char *)&s_UdiskOtherSpeedConfigDesc;
    memcpy(configdata + cnt, (unsigned char *)&s_UdiskInterfacefDesc, *(unsigned char *)&s_UdiskInterfacefDesc);
    cnt += *(unsigned char *)&s_UdiskInterfacefDesc;
    memcpy(configdata + cnt, (unsigned char *)&s_UdiskEp2Desc, *(unsigned char *)&s_UdiskEp2Desc);
    cnt += *(unsigned char *)&s_UdiskEp2Desc;
    //other speed is full speed,ep2 maxpktsize is 64  
    if (s_pUdisk->ulMode == USB_MODE_20)
    {
        configdata[cnt-3]=0x40;
        configdata[cnt-2]=0x00;   
    }
    //other speed is high speed,ep2 maxpktsize is 512  
    else
    {
        configdata[cnt-3]=0x00;
        configdata[cnt-2]=0x02;
    }
    memcpy(configdata + cnt, (unsigned char *)&s_UdiskEp3Desc, *(unsigned char *)&s_UdiskEp3Desc);
    cnt += *(unsigned char *)&s_UdiskEp3Desc;
    //other speed is full speed,ep2 maxpktsize is 64  
    if (s_pUdisk->ulMode == USB_MODE_20)
    {
        configdata[cnt-3]=0x40;
        configdata[cnt-2]=0x00;   
    }
    //other speed is high speed,ep2 maxpktsize is 512  
    else
    {
        configdata[cnt-3]=0x00;
        configdata[cnt-2]=0x02;
    }
    *count = cnt;
    return configdata;
}

/** 
 * @brief get string descriptor callback
 *
 * this callback is set at udisk_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param index[in] index of string descriptor
 * @param count[out] len of stirng descriptor
 * @return  unsigned char *
 * @retval  addr of string descriptor
 */
static unsigned char *udisk_get_str_desc(unsigned char index, unsigned long *count)
{
    if(index == 0)
    {
        *count = sizeof(s_UdiskString0);
        return (unsigned char *)s_UdiskString0;
    }
    else if(index == 1)
    {
        *count = sizeof(s_UdiskString1);
        return (unsigned char *)s_UdiskString1;
    }
    else if(index == 2)
    {
        *count = sizeof(s_UdiskString2);
        return (unsigned char *)s_UdiskString2;
    }
      else if(index == 3)
    {
        *count = sizeof(s_UdiskString3);
        return (unsigned char *)s_UdiskString3;
    }
    return NULL;
}
/** 
 * @brief init serial number str in device desc
 *
 * the random str is truncated to 10  characters or less
 * @author Huang Xin
 * @date 2010-10-25
 * @return  void
 */
static void init_serial_number()
{
    unsigned char i = 0;
    unsigned long random = 0;
    char str[20] = {0};
    char *p = str;
    
    srand(get_tick_count());
    random = rand();
    itoa(random,str);
    for (i = 2; i<=20; i+=2)
    {
        if (*p)
        {
            s_UdiskString3[i] = *p++;
        }
        else
        {
            s_UdiskString3[i] = ' ';
        }
    }
    
}
/** 
 * @brief reverse str
 *
 * @author Huang Xin
 * @date 2010-10-25
 * @return  void
 */
static void  reverse(char   *s)   
{ 
    char   *c; 
    unsigned long   i; 
    c  =   s   +   strlen(s)   -   1; 
    while(s   <   c)   
    { 
        i = *s; 
        *s++ = *c; 
        *c-- = i; 
    } 
} 
/** 
 * @brief convert int to str
 *
 * @author Huang Xin
 * @date 2010-10-25
 * @return  void
 */
static void itoa(unsigned long   n,   char   *s)   
{ 
    char   *ptr; 
    ptr = s; 
    do   
    { 
        *ptr++   =   n   %   10   +   '0'; 
    } while   ((n   =   n   /   10)   >   0); 
    *ptr   =   '\0'; 
    reverse(s); 
} 


