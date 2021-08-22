/**
 * @filename usb_h_udisk_mass.c
 * @brief:how to use usb disk host.
 *
 * This file describe frameworks of usb disk host driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-26
 * @version 1.0
 */
#include <stdio.h>
#include "anyka_types.h"
#include "drv_api.h"
#include "hal_h_udisk.h"
#include "hal_usb_h_std.h"
#include "hal_h_udisk_mass.h"
#include "usb_bus_drv.h"
#include "usb_host_drv.h"

static bool msc_cmd_stage(T_USB_CBW_PKT *cbw);
static bool msc_data_stage(unsigned char* buf,unsigned long len,unsigned char dir);
static bool msc_status_stage(T_USB_CSW_PKT* csw);
static unsigned long dat32_swap(unsigned long dat_swap);
static unsigned short dat16_swap(unsigned short dat_swap);

/**
 * @brief   inquiry process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param str[out] Buffer to store data
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char msc_inquiry(unsigned char Lun, unsigned char * str, unsigned char* status)
{
    T_SCSI_CMD scsi;    
    T_USB_CBW_PKT cbw_pkt;
    T_USB_CSW_PKT csw_pkt;
    unsigned long retry = 0;
    unsigned char ret = MSC_STAGE_ALL_SUCCESS;

    akprintf(C3, M_DRVSYS, "Inquiry\r\n");
    //set inquiry cmd
    scsi.bOperationCode = SCSI_INQUIRY;
    scsi.bLogicUnitNum = Lun<<5;
    scsi.dLogicBlockAddr = 0x00240000;
    scsi.dTransferLength = 0;
    scsi.bReserve = 0;
    scsi.bControl = 0;
    scsi.dReserve = 0;
    //set cbw
    cbw_pkt.dCBWSignature = CBW_SIGNATURE;
    cbw_pkt.dCBWTag = CBW_TAG;
    cbw_pkt.dCBWDataTransferLength = 0x24;
    cbw_pkt.bmCBWFlags = CBWFLAGS_IN;
    cbw_pkt.bCBWLUN = Lun&0x7;
    cbw_pkt.bCBWCBLength = 0x6;
    memcpy((unsigned char *)(&(cbw_pkt.scsi)), (unsigned char *)&scsi, sizeof(T_SCSI_CMD));
    //cmd stage
    if(!msc_cmd_stage(&cbw_pkt))
    {
        return MSC_STAGE_CMD_FAILED;
    }
    //data stage
    if(!msc_data_stage(str,0x24,TRANS_DATA_IN))
    {
        return MSC_STAGE_DATA_FAILED;
    }
    //status stage
    if(!msc_status_stage(&csw_pkt))
    {
        return MSC_STAGE_STATUS_FAILED;   
    }
    *status = csw_pkt.bCSWStatus;
    return MSC_STAGE_ALL_SUCCESS;

}

/**
 * @brief   test unit ready process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char msc_test_unit_ready(unsigned char Lun,unsigned char* status)
{
    T_SCSI_CMD scsi;    
    T_USB_CBW_PKT cbw_pkt;
    T_USB_CSW_PKT csw_pkt;
    unsigned long retry = 0;

   // akprintf(C3, M_DRVSYS, "test unit ready\r\n");
    //set test unit ready cmd
    scsi.bOperationCode = SCSI_TEST_UNIT_READY;
    scsi.bLogicUnitNum = Lun<<5;
    scsi.dLogicBlockAddr = 0;
    scsi.dTransferLength = 0;
    scsi.bReserve = 0;
    scsi.bControl = 0;
    scsi.dReserve = 0;
    //set cbw
    cbw_pkt.dCBWSignature = CBW_SIGNATURE;
    cbw_pkt.dCBWTag = CBW_TAG;
    cbw_pkt.dCBWDataTransferLength = 0;
    cbw_pkt.bmCBWFlags = CBWFLAGS_OUT;
    cbw_pkt.bCBWLUN = Lun&0x7;
    cbw_pkt.bCBWCBLength = 0x6;
    memcpy((unsigned char *)(&(cbw_pkt.scsi)), (unsigned char *)&scsi, sizeof(T_SCSI_CMD));
    //cmd stage
    if(!msc_cmd_stage(&cbw_pkt))
    {
        return MSC_STAGE_CMD_FAILED;
    }
    //status stage
    if(!msc_status_stage(&csw_pkt))
    {
        return MSC_STAGE_STATUS_FAILED;   
    }
    *status = csw_pkt.bCSWStatus;
    return MSC_STAGE_ALL_SUCCESS;

}

/**
 * @brief   read format capacity process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param buf[out] Buffer to store data
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char msc_read_format_capacity(unsigned char Lun,unsigned char* buf,unsigned char* status)
{
    T_SCSI_CMD scsi;    
    T_USB_CBW_PKT cbw_pkt;
    T_USB_CSW_PKT csw_pkt;
    unsigned long retry = 0;
    unsigned char ret = MSC_STAGE_ALL_SUCCESS;

    akprintf(C3, M_DRVSYS, "read f_capacity\r\n");
    //set inquiry cmd
    scsi.bOperationCode = SCSI_READ_FORMAT_CAPACITY;
    scsi.bLogicUnitNum = Lun<<5;
    scsi.dLogicBlockAddr = 0;
    scsi.dTransferLength = 0xfc<<16;
    scsi.bReserve = 0;
    scsi.bControl = 0;
    scsi.dReserve = 0;
    //set cbw
    cbw_pkt.dCBWSignature = CBW_SIGNATURE;
    cbw_pkt.dCBWTag = CBW_TAG;
    cbw_pkt.dCBWDataTransferLength = 0xfc;
    cbw_pkt.bmCBWFlags = CBWFLAGS_IN;
    cbw_pkt.bCBWLUN = Lun&0x7;
    cbw_pkt.bCBWCBLength = 0xa;
    memcpy((unsigned char *)(&(cbw_pkt.scsi)), (unsigned char *)&scsi, sizeof(T_SCSI_CMD));
    //cmd stage
    if(!msc_cmd_stage(&cbw_pkt))
    {
        return MSC_STAGE_CMD_FAILED;
    }
    //data stage
    if(!msc_data_stage(buf,cbw_pkt.dCBWDataTransferLength,TRANS_DATA_IN))
    {
        return MSC_STAGE_DATA_FAILED;
    }
    //status stage
    if(!msc_status_stage(&csw_pkt))
    {
        return MSC_STAGE_STATUS_FAILED;   
    }
    *status = csw_pkt.bCSWStatus;
    return MSC_STAGE_ALL_SUCCESS;
}

/**
 * @brief   read capacity process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param buf[out] Buffer to store data
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char msc_read_capacity(unsigned char Lun,unsigned char* buf,unsigned char* status)
{
    T_SCSI_CMD scsi;    
    T_USB_CBW_PKT cbw_pkt;
    T_USB_CSW_PKT csw_pkt;
    unsigned long retry = 0;
    unsigned char ret = MSC_STAGE_ALL_SUCCESS;

    akprintf(C3, M_DRVSYS, "read capacity\r\n");
    //set inquiry cmd
    scsi.bOperationCode = SCSI_READ_CAPACITY;
    scsi.bLogicUnitNum = Lun<<5;
    scsi.dLogicBlockAddr = 0;
    scsi.dTransferLength = 0;
    scsi.bReserve = 0;
    scsi.bControl = 0;
    scsi.dReserve = 0;
    //set cbw
    cbw_pkt.dCBWSignature = CBW_SIGNATURE;
    cbw_pkt.dCBWTag = CBW_TAG;
    cbw_pkt.dCBWDataTransferLength = 0x8;
    cbw_pkt.bmCBWFlags = CBWFLAGS_IN;
    cbw_pkt.bCBWLUN = Lun&0x7;
    cbw_pkt.bCBWCBLength = 0xa;
    memcpy((unsigned char *)(&(cbw_pkt.scsi)), (unsigned char *)&scsi, sizeof(T_SCSI_CMD));
    //cmd stage
    if(!msc_cmd_stage(&cbw_pkt))
    {
        return MSC_STAGE_CMD_FAILED;
    }
    //data stage
    if(!msc_data_stage(buf,cbw_pkt.dCBWDataTransferLength,TRANS_DATA_IN))
    {
        return MSC_STAGE_DATA_FAILED;
    }
    //status stage
    if(!msc_status_stage(&csw_pkt))
    {
        return MSC_STAGE_STATUS_FAILED;   
    }
    *status = csw_pkt.bCSWStatus;
    return MSC_STAGE_ALL_SUCCESS;
}

/**
 * @brief   request sense process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param buf[out] Buffer to store data
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char  msc_req_sense(unsigned char Lun,unsigned char* buf,unsigned char* status)
{
    T_SCSI_CMD scsi;    
    T_USB_CBW_PKT cbw_pkt;
    T_USB_CSW_PKT csw_pkt;
    unsigned long retry = 0;
    unsigned char ret = MSC_STAGE_ALL_SUCCESS;

   // akprintf(C3, M_DRVSYS, "req sense\n");
    //set inquiry cmd
    scsi.bOperationCode = SCSI_REQUEST_SENSE;
    scsi.bLogicUnitNum = Lun<<5;
    scsi.dLogicBlockAddr = 0x00120000;
    scsi.dTransferLength = 0;
    scsi.bReserve = 0;
    scsi.bControl = 0;
    scsi.dReserve = 0;
    //set cbw
    cbw_pkt.dCBWSignature = CBW_SIGNATURE;
    cbw_pkt.dCBWTag = CBW_TAG;
    cbw_pkt.dCBWDataTransferLength = 0x12;
    cbw_pkt.bmCBWFlags = CBWFLAGS_IN;
    cbw_pkt.bCBWLUN = Lun&0x7;
    cbw_pkt.bCBWCBLength = 0xc;
    memcpy((unsigned char *)(&(cbw_pkt.scsi)), (unsigned char *)&scsi, sizeof(T_SCSI_CMD));
    //cmd stage
    if(!msc_cmd_stage(&cbw_pkt))
    {
        return MSC_STAGE_CMD_FAILED;
    }
    //data stage
    if(!msc_data_stage(buf,cbw_pkt.dCBWDataTransferLength,TRANS_DATA_IN))
    {
        return MSC_STAGE_DATA_FAILED;
    }
    //status stage
    if(!msc_status_stage(&csw_pkt))
    {
        return MSC_STAGE_STATUS_FAILED;   
    }
    *status = csw_pkt.bCSWStatus;
    return MSC_STAGE_ALL_SUCCESS;
}

/**
 * @brief   read10 process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param data[in] Buffer to store data
 * @param sector[in] start sector read from 
 * @param sector_num[in] sector number to be read
 * @param byte_num[in] byte number to be read
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char  msc_read10(unsigned char Lun,unsigned char* data, unsigned long sector, unsigned long sector_num, unsigned long byte_num, unsigned char* status)
{
    T_SCSI_CMD scsi;    
    T_USB_CBW_PKT cbw_pkt;
    T_USB_CSW_PKT csw_pkt;
    unsigned long retry = 0;
    unsigned char ret = MSC_STAGE_ALL_SUCCESS;

    //set read10 cmd
    scsi.bOperationCode = SCSI_READ_10;
    scsi.bLogicUnitNum = Lun<<5;
    scsi.dLogicBlockAddr = dat32_swap(sector);
    scsi.dTransferLength = (dat16_swap(sector_num)) << 8;;
    scsi.bReserve = 0;
    scsi.bControl = 0;
    scsi.dReserve = 0;
    //set cbw
    cbw_pkt.dCBWSignature = CBW_SIGNATURE;
    cbw_pkt.dCBWTag = CBW_TAG;
    cbw_pkt.dCBWDataTransferLength = byte_num;
    cbw_pkt.bmCBWFlags = CBWFLAGS_IN;
    cbw_pkt.bCBWLUN = Lun&0x7;
    cbw_pkt.bCBWCBLength = 0xa;
    memcpy((unsigned char *)(&(cbw_pkt.scsi)), (unsigned char *)&scsi, sizeof(T_SCSI_CMD));
    //cmd stage
    if(!msc_cmd_stage(&cbw_pkt))
    {
        return MSC_STAGE_CMD_FAILED;
    }
    //data stage
    if(!msc_data_stage(data,cbw_pkt.dCBWDataTransferLength,TRANS_DATA_IN))
    {
        return MSC_STAGE_DATA_FAILED;
    }
    //status stage
    if(!msc_status_stage(&csw_pkt))
    {
        return MSC_STAGE_STATUS_FAILED;   
    }
    *status = csw_pkt.bCSWStatus;
    return MSC_STAGE_ALL_SUCCESS;
}

/**
 * @brief   write10 process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param data[in] Buffer to store data
 * @param sector[in] start sector write from 
 * @param sector_num[in] sector number to be write
 * @param byte_num[in] byte number to be write
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char  msc_write10(unsigned char Lun,unsigned char* data, unsigned long sector, unsigned long sector_num, unsigned long byte_num, unsigned char* status)
{
    T_SCSI_CMD scsi;    
    T_USB_CBW_PKT cbw_pkt;
    T_USB_CSW_PKT csw_pkt;
    unsigned long retry = 0;
    unsigned char ret = MSC_STAGE_ALL_SUCCESS;

    //set write10 cmd
    scsi.bOperationCode = SCSI_WRITE_10;
    scsi.bLogicUnitNum = Lun<<5;
    scsi.dLogicBlockAddr = dat32_swap(sector);
    scsi.dTransferLength = (dat16_swap(sector_num)) << 8;;
    scsi.bReserve = 0;
    scsi.bControl = 0;
    scsi.dReserve = 0;
    //set cbw
    cbw_pkt.dCBWSignature = CBW_SIGNATURE;
    cbw_pkt.dCBWTag = CBW_TAG;
    cbw_pkt.dCBWDataTransferLength = byte_num;
    cbw_pkt.bmCBWFlags = CBWFLAGS_OUT;
    cbw_pkt.bCBWLUN = Lun&0x7;
    cbw_pkt.bCBWCBLength = 0xa;
    memcpy((unsigned char *)(&(cbw_pkt.scsi)), (unsigned char *)&scsi, sizeof(T_SCSI_CMD));
    //cmd stage
    if(!msc_cmd_stage(&cbw_pkt))
    {
        return MSC_STAGE_CMD_FAILED;
    }
    //data stage
    if(!msc_data_stage(data,cbw_pkt.dCBWDataTransferLength,TRANS_DATA_OUT))
    {
        return MSC_STAGE_DATA_FAILED;
    }
    //status stage
    if(!msc_status_stage(&csw_pkt))
    {
        return MSC_STAGE_STATUS_FAILED;   
    }
    *status = csw_pkt.bCSWStatus;
    return MSC_STAGE_ALL_SUCCESS;
}

/**
 * @brief   get max lun process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param pMaxLun[in] max lun.
 * @return bool
 * @retval  true: get successfully
 * @retval  false: get failed
 */
bool msc_get_max_lun(unsigned char* pMaxLun)
{
    T_URB urb;
    T_URB_HANDLE hURB = NULL;
    T_UsbDevReq dev_req;

    dev_req.bmRequestType = USB_STD_DIR_DEV2HOST | USB_STD_REQTYPE_CLS | USB_STD_REC_INTERFACE;
    dev_req.bRequest = 0xfe;
    dev_req.wValue = 0;
    dev_req.wIndex = 0;
    dev_req.wLength = 1;

    //fill urb struct
    memset(&urb, 0, sizeof(T_URB));
    memcpy(&urb.dev_req, &dev_req, sizeof(T_UsbDevReq));
    urb.trans_type = TRANS_CTRL;
    urb.trans_dir = TRANS_DATA_IN;    
    urb.data = pMaxLun;
    urb.buffer_len = 1;
    urb.data_len = 1;
    urb.timeout = URB_MAX_WAIT_TIME;
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

    return true;
}

/**
 * @brief   bulk only reset process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @return bool
 * @retval  true: reset successfully
 * @retval  false: reset failed
 */
bool msc_bulk_only_reset(void)
{
    T_URB urb;
    T_URB_HANDLE hURB = NULL;
    T_UsbDevReq dev_req;

    dev_req.bmRequestType = USB_STD_DIR_HOST2DEV | USB_STD_REQTYPE_CLS | USB_STD_REC_INTERFACE;
    dev_req.bRequest = 0xff;
    dev_req.wValue = 0;
    dev_req.wIndex = 0;
    dev_req.wLength = 0;

    //fill urb struct
    memset(&urb, 0, sizeof(T_URB));
    memcpy(&urb.dev_req, &dev_req, sizeof(T_UsbDevReq));
    urb.trans_type = TRANS_CTRL;
    urb.trans_dir = TRANS_DATA_OUT;    
    urb.timeout = URB_MAX_WAIT_TIME;
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

    return true;
}

/**
 * @brief   cmd stage process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param cbw[in] cbw.
 * @return bool
 * @retval  true: cmd stage successfully
 * @retval  false: cmd stage failed
 */
static bool msc_cmd_stage(T_USB_CBW_PKT *cbw)
{
    bool ret = true;    
    unsigned char retry = 0;
    T_URB urb;
    T_URB_HANDLE hURB = NULL;
    
   //fill urb struct
    memset(&urb, 0, sizeof(T_URB));
    urb.data = (unsigned char *)cbw ;
    urb.trans_type = TRANS_BULK;
    urb.trans_dir = TRANS_DATA_OUT;
    urb.buffer_len = sizeof(T_USB_CBW_PKT);
    urb.data_len = sizeof(T_USB_CBW_PKT);
    urb.timeout = URB_MAX_WAIT_TIME;
    while (1)
    {
        //commit urb
        hURB = usb_bus_commit_urb(&urb);
        if(NULL == hURB)
        {
            return false;
        }
        //waiting for urb completion
        if(usb_bus_wait_completion(hURB) < 0)
        {
            if (retry++ < 1)
            {
                akprintf(C3, M_DRVSYS, "reset recovery\n");
                ret &= msc_bulk_only_reset();
                ret &= usb_host_std_clear_feature((1<<7)|g_UsbBulkinIndex);
                ret &= usb_host_std_clear_feature(g_UsbBulkoutIndex);
                if (!ret)
                {
                    //re-enum
                    usb_bus_enum();
                    return false;
                }
            }
            else
            {
                //re-enum
                usb_bus_enum();
                return false;
            }
        }
        else
        {
            return true;
        }
    }
}

/**
 * @brief   data stage process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param buf[in] data trans buf.
 * @param len[in] data trans len.
 * @param dir[in] data trans direction .
 * @return bool
 * @retval  true: data stage successfully
 * @retval  false: data stage failed
 */
static bool msc_data_stage(unsigned char* buf,unsigned long len,unsigned char dir)
{
    T_URB urb;
    T_URB_HANDLE hURB = NULL;
    
    //fill urb struct
    memset(&urb, 0, sizeof(T_URB));
    urb.data = buf;
    urb.trans_type = TRANS_BULK;
    urb.trans_dir = dir;
    urb.buffer_len = len;
    urb.data_len = len;
    urb.timeout = URB_MAX_WAIT_TIME;
    //commit urb
    hURB = usb_bus_commit_urb(&urb);
    if(NULL == hURB)
    {
        return false;
    }
    //waiting for urb completion
    if(usb_bus_wait_completion(hURB) < 0)
    {
        if (TRANS_DATA_IN == dir)
        {
            usb_host_std_clear_feature((1<<7)|g_UsbBulkinIndex);
        }
        else
        {
            usb_host_std_clear_feature(g_UsbBulkoutIndex);
        }
    }
    return true;
}

/**
 * @brief   status stage process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param csw[out] csw.
 * @return bool
 * @retval  true: status stage successfully
 * @retval  false: status stage failed
 */
static bool msc_status_stage(T_USB_CSW_PKT* csw)
{
    bool ret = true;
    unsigned char retry = 0;
    T_URB urb;
    T_URB_HANDLE hURB = NULL;
    
    //fill urb struct
    memset(&urb, 0, sizeof(T_URB));
    urb.data = (unsigned char *)csw;
    urb.trans_type = TRANS_BULK;
    urb.trans_dir = TRANS_DATA_IN;
    urb.buffer_len = CSW_PKT_SIZE;
    urb.data_len = CSW_PKT_SIZE;
    urb.timeout = URB_MAX_WAIT_TIME;
    while (1)
    {
        //commit urb
        hURB = usb_bus_commit_urb(&urb);
        if(NULL == hURB)
        {
            return false;
        }
        //waiting for urb completion
        if(usb_bus_wait_completion(hURB) < 0)
        {
            if (retry++ < 1)
            {
                usb_host_std_clear_feature((1<<7)|g_UsbBulkinIndex);
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    if (retry > 1)
    {
        goto  reset_recovery;
    }
    else
    {
        if (csw->dCSWSignature != CSW_SIGNATURE || csw->dCSWTag != CBW_TAG)
        {
            akprintf(C1, M_DRVSYS, "csw invalid\n");
            goto  reset_recovery;
        }
        if (csw->bCSWStatus != 0  && csw->bCSWStatus != 1)
        {
            akprintf(C1, M_DRVSYS, "csw phase error\n");
            goto  reset_recovery;
        }
        return true;
    }

    reset_recovery:
        ret &= msc_bulk_only_reset();
        ret &= usb_host_std_clear_feature((1<<7)|g_UsbBulkinIndex);
        ret &= usb_host_std_clear_feature(g_UsbBulkoutIndex);
        if (!ret)
        {
            //re-enum
            usb_bus_enum();
        }
        return ret;
}

static unsigned long dat32_swap(unsigned long dat_swap)
{
    unsigned long swaped_dat;
    unsigned char *p_buf = (unsigned char *)&dat_swap;

    swaped_dat = (p_buf[0] << 24) | (p_buf[1] << 16) | (p_buf[2] << 8) | p_buf[3];

    return swaped_dat;
}
//********************************************************************
static unsigned short dat16_swap(unsigned short dat_swap)
{
    unsigned short  swaped_dat;
    unsigned char *p_buf = (unsigned char *)&dat_swap;

    swaped_dat = (p_buf[0]<<8) | p_buf[1];

    return swaped_dat;
}



