/**
 * @filename usb_udisk_mass.c
 * @brief:how to use usb disk.
 *
 * This file describe frameworks of usb disk driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-26
 * @version 1.0
 */
#include <stdio.h>
#include <string.h>
#include "usb_slave_drv.h"
#include "hal_usb_s_disk.h"
#include "hal_usb_s_std.h"
#include "hal_udisk_mass.h"
#include "usb_common.h"
#include "interrupt.h"
#include "akos_api.h"
#include "drv_api.h"
#include "drv_module.h"



static volatile T_MSC_TRANS s_MscTrans={0};
static volatile unsigned short InquireCnt = 0;
static void scsi_unsupported(unsigned char* scsi_data);
static void scsi_inquiry(void);
static void scsi_test_unit_ready(void);
static void scsi_read_format_capacity(void);
static void scsi_read_capacity(void);
static void scsi_read10(unsigned char* scsi_data);
static void scsi_read12(unsigned char* scsi_data);
static void scsi_write10(unsigned char* scsi_data);
static void scsi_write12(unsigned char* scsi_data);
static void scsi_verify();
static void scsi_start_stop(unsigned char* scsi_data);
static void scsi_mode_sense6();
static void scsi_req_sense();
static void scsi_prevent_remove();
static void msc_scsi_handle(unsigned char* scsi_data);
static void scsi_anyka_mass_boot(unsigned char* scsi_data);
static void msc_mboot_handle(unsigned char* scsi_data);



unsigned short msc_get_inquire_cnt(void)
{
    return InquireCnt;
}
void msc_reset_inquire_cnt(void)
{
    InquireCnt = 0;
}
/**
 * @brief parse cbw
 *
 * called when cbw is received
 * @author Huang Xin
 * @date 2010-08-04
 * @param buf[in] buf which is saving cbw
 * @param buflen[in] the cbw len
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool msc_parse_cbw(unsigned char *buf, unsigned long buflen)
{
    if (CBW_PKT_SIZE == buflen )
    {    
        memcpy((unsigned char *)(&s_MscTrans.tCbw), buf, CBW_PKT_SIZE);   
        //cbw invalid or no mean
        if (CBW_SIGNATURE != s_MscTrans.tCbw.dCBWSignature)
        {
            usb_slave_std_hard_stall(true);
            usb_slave_ep_stall(EP2_INDEX);
            usb_slave_ep_stall(EP3_INDEX);
            return false;
        }
        else if(s_MscTrans.tCbw.dCBWDataTransferLength != 0)
        {
            if (0 == (s_MscTrans.tCbw.bmCBWFlags&0x80))
                s_MscTrans.enmStage = MSC_STAGE_DATA_OUT; 
            else
                s_MscTrans.enmStage = MSC_STAGE_DATA_IN;
        }     
        else
        {
            s_MscTrans.enmStage = MSC_STAGE_STATUS;
        }
        if(udisk_lun_num() > 0)
        {
            msc_scsi_handle((unsigned char*)s_MscTrans.tCbw.pCBWCB);
        }
        else
        {
            msc_mboot_handle((unsigned char*)s_MscTrans.tCbw.pCBWCB);
        }
        return true;
    }    
    //CBW length is not 31 bytes,cbw invalid
    else
    {
        usb_slave_std_hard_stall(true);
        usb_slave_ep_stall(EP2_INDEX);
        usb_slave_ep_stall(EP3_INDEX);
        return false;
    }

    return false;
    
}

/**
 * @brief send csw
 *
 * called in status stage  
 * @author Huang Xin
 * @date 2010-08-04
 * @param status[in] pass,failed or phase error
 * @param residue[in] difference between the amount of data expected and the actual sent by the device.
 * @return  void
 */
void msc_send_csw(unsigned char status,unsigned long residue)
{
    s_MscTrans.tCsw.dCSWSignature = CSW_SIGNATURE;
    s_MscTrans.tCsw.dCSWTag = s_MscTrans.tCbw.dCBWTag;
    s_MscTrans.tCsw.dCSWDataResidue = residue;
    s_MscTrans.tCsw.bCSWStatus = status; 
    
    usb_slave_start_send(EP2_INDEX);
    usb_slave_data_in(EP2_INDEX, (unsigned char *)&s_MscTrans.tCsw, CSW_PKT_SIZE);
}

/**
 * @brief get current stage
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  unsigned char
 * @retval the enum value of stage
 */
unsigned char msc_get_stage()
{
    return s_MscTrans.enmStage;
}

/**
 * @brief set stage
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param stage[in] the current stage to set
 * @return  void
 */
void msc_set_stage(unsigned char stage)
{    
    s_MscTrans.enmStage = stage;
}

/**
 * @brief scsi cmd parse handle
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  void
 */
static void msc_scsi_handle(unsigned char* scsi_data)
{    
    //akprintf(C3, M_DRVSYS, "<%x>",*scsi_data);
    switch (*scsi_data)
    {       
        case SCSI_INQUIRY:
            scsi_inquiry();
            break;
        case SCSI_TEST_UNIT_READY:
            scsi_test_unit_ready();
            InquireCnt++;
            break;
        case SCSI_READ_FORMAT_CAPACITY:
            scsi_read_format_capacity();
            break;
        case SCSI_READ_CAPACITY:
            scsi_read_capacity();
            break;     
        case SCSI_READ_10:
            scsi_read10(scsi_data);
            break;
        case SCSI_WRITE_10:
            scsi_write10(scsi_data);
            break;
        case SCSI_MODESENSE_6:
        case SCSI_MODE_SENSE:
            scsi_mode_sense6();
            break;   
        case SCSI_VERIFY:
            scsi_verify(scsi_data);
            break;
        case SCSI_START_STOP:
            scsi_start_stop(scsi_data);
            break;
        case SCSI_REQUEST_SENSE:
            scsi_req_sense();
            break;    
        case SCSI_READ_12:
            scsi_read12(scsi_data);
            break;  
        case SCSI_WRITE_12:
            scsi_write12(scsi_data);
            break;
        case SCSI_PREVENT_REMOVE:
            scsi_prevent_remove();
            break;
        default:
            scsi_unsupported(scsi_data);
            break;
    }
}
static void msc_mboot_handle(unsigned char* scsi_data)
{    
    //akprintf(C3, M_DRVSYS, "<0x%x>",*scsi_data);
    switch (*scsi_data)
    {   
        case SCSI_INQUIRY:
            scsi_inquiry();
            break;
        case SCSI_TEST_UNIT_READY:
            scsi_test_unit_ready();
            break;
        case SCSI_REQUEST_SENSE:
            scsi_req_sense();
            break;    
        case SCSI_ANYKA_MASS_BOOT:
            scsi_anyka_mass_boot(scsi_data);
            break;
        default:
            scsi_unsupported(scsi_data);
            break;
    }
}


/**
 * @brief process unsupported cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  void
 */
static void scsi_unsupported(unsigned char* scsi_data)
{    

    udisk_unsupported(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength,scsi_data);

}

/**
 * @brief process inquiry cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void scsi_inquiry(void)
{
    udisk_inquiry(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process test unit ready cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void scsi_test_unit_ready(void)
{
    udisk_test_unit_ready(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process read format capacity cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void scsi_read_format_capacity(void)
{
    udisk_read_format_capacity(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process read capacity cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void scsi_read_capacity(void)
{ 
    udisk_read_capacity(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);   
}

/**
 * @brief process read10 cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  void
 */
static void scsi_read10(unsigned char* scsi_data)
{
    unsigned long log_blk_addr;
    unsigned long log_blk_len;
    
    log_blk_addr=(*(scsi_data+2)<<24)+(*(scsi_data+3)<<16)+(*(scsi_data+4)<<8)+(*(scsi_data+5));
    log_blk_len =(*(scsi_data+7)<<8)+(*(scsi_data+8)); 
    udisk_read(s_MscTrans.tCbw.bCBWLUN,log_blk_addr,s_MscTrans.tCbw.dCBWDataTransferLength);

}

/**
 * @brief process read12 cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  void
 */
static void scsi_read12(unsigned char* scsi_data)
{
    unsigned long log_blk_addr;
    unsigned long log_blk_len;

    log_blk_addr=(*(scsi_data+2)<<24)+(*(scsi_data+3)<<16)+(*(scsi_data+4)<<8)+(*(scsi_data+5));
    log_blk_len =(*(scsi_data+6)<<24)+(*(scsi_data+7)<<16)+(*(scsi_data+8)<<8)+(*(scsi_data+9));
    udisk_read(s_MscTrans.tCbw.bCBWLUN,log_blk_addr,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process write10 cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  void
 */
static void scsi_write10(unsigned char* scsi_data)
{
    unsigned long log_blk_addr;
    unsigned long log_blk_len;

    log_blk_addr=(*(scsi_data+2)<<24)+(*(scsi_data+3)<<16)+(*(scsi_data+4)<<8)+(*(scsi_data+5));
    log_blk_len =(*(scsi_data+7)<<8)+(*(scsi_data+8)); 
    udisk_write(s_MscTrans.tCbw.bCBWLUN,log_blk_addr,s_MscTrans.tCbw.dCBWDataTransferLength);

}

/**
 * @brief process write12 cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  void
 */
static void scsi_write12(unsigned char* scsi_data)
{
    unsigned long log_blk_addr;
    unsigned long log_blk_len;
    
    log_blk_addr=(*(scsi_data+2)<<24)+(*(scsi_data+3)<<16)+(*(scsi_data+4)<<8)+(*(scsi_data+5));
    log_blk_len =(*(scsi_data+6)<<24)+(*(scsi_data+7)<<16)+(*(scsi_data+8)<<8)+(*(scsi_data+9));    
    udisk_write(s_MscTrans.tCbw.bCBWLUN,log_blk_addr,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process verify cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void scsi_verify()
{
    udisk_verify(s_MscTrans.tCbw.bCBWLUN);
}

/**
 * @brief process start stop cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param scsi_data[in] the scsi cmd
 * @return  void
 */
static void scsi_start_stop(unsigned char* scsi_data)
{   
    unsigned long start_stop;
    
    start_stop = (scsi_data[4] & 0x03);
    udisk_start_stop(s_MscTrans.tCbw.bCBWLUN,start_stop);
}

/**
 * @brief process mode sense6 cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void scsi_mode_sense6()
{
    udisk_mode_sense6(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);
}

/**
 * @brief process request sense cmd
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void scsi_req_sense()
{
    udisk_req_sense(s_MscTrans.tCbw.bCBWLUN,s_MscTrans.tCbw.dCBWDataTransferLength);
}


static void scsi_prevent_remove()
{
   udisk_prevent_remove(s_MscTrans.tCbw.bCBWLUN);
}


static void scsi_anyka_mass_boot(unsigned char* scsi_data)
{
   udisk_anyka_mass_boot(scsi_data, s_MscTrans.tCbw.dCBWDataTransferLength);
}


