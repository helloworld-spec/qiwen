/**
 * @filename usb_h_udisk.c
 * @brief:how to use usb disk host.
 *
 * This file describe frameworks of usb disk host driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-08-31
 * @version 1.0
 */
#include <stdio.h>
#include "usb_host_drv.h"
#include "hal_h_udisk.h"
#include "hal_usb_h_std.h"
#include "hal_h_udisk_mass.h"
#include "usb_bus_drv.h"
#include "usb_common.h"
#include "interrupt.h"
#include "akos_api.h"
#include "sysctl.h"
#include "drv_api.h"
#include "drv_module.h"
#include "drv_timer.h"

#define FREQ_REQUST_ASIC     60000000
static T_hFreq freq_handle = FREQ_INVALID_HANDLE;

static volatile T_pH_UDISK_DEV s_pHostUdisk = NULL;
static void udisk_host_enum_ok_cb(void);
static void udisk_host_discon_cb(void);
static void udisk_host_timer_interrupt(signed long timer_id, unsigned long delay);



/**
 * @brief   init udisk host function
 *
 * Allocate udisk host buffer,init data strcut,register callback,open usb controller and phy.
 * @author Huang Xin
 * @date 2010-07-12
 * @param mode[in] usb mode 1.1 or 2.0
 * @return bool
 * @retval false init failed
 * @retval AK_TURE init successful
 */
bool udisk_host_init(unsigned long mode)
{
    T_USB_BUS_HANDLER tBusHandle = {0};
    tBusHandle.class_code = USB_DEVICE_CLASS_STORAGE;
    tBusHandle.enumok_callback = udisk_host_enum_ok_cb;
    tBusHandle.discon_callback = udisk_host_discon_cb;
    if (!usb_bus_reg_class(&tBusHandle))
    {
        return false;
    }
    s_pHostUdisk = (T_pH_UDISK_DEV)drv_malloc(sizeof(T_H_UDISK_DEV));
    if (s_pHostUdisk == NULL)
    {
        akprintf(C1, M_DRVSYS, "s_pHostUdisk,alloc failed: %x\n", s_pHostUdisk);
        return false;
    }   
    //init s_pHostUdisk member
    memset(s_pHostUdisk,0,sizeof(T_H_UDISK_DEV));

    s_pHostUdisk->lTimerId = ERROR_TIMER;
    
    usb_bus_open(mode);
    
    if (FREQ_INVALID_HANDLE == freq_handle)
    {
        freq_handle = FreqMgr_RequestFreq(FREQ_REQUST_ASIC);
    }
    
    return true;
}

/**
 * @brief   get disk all logic unit number
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @return unsigned char
 * @retval  Total number of logic unit.
 */
unsigned char udisk_host_get_lun_num(void)
{
    return s_pHostUdisk->ucMaxLun+1;
}

/**
 * @brief   get a logic unit number descriptor
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param   LUN[in] Index of logic unit.
 * @param   disk_info[out]  The information of the lun
 * @return  void.
 */
void udisk_host_get_lun_info(unsigned long lun, T_pH_UDISK_LUN_INFO disk_info)
{
    if(lun > s_pHostUdisk->ucMaxLun || disk_info == NULL)
    {
        return;
    }
    memcpy(disk_info, (unsigned char *)(s_pHostUdisk->pLunInfo+lun), sizeof(T_H_UDISK_LUN_INFO));
}

/**
 * @brief   usb host read sector from logic unit
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param data[in] Buffer to store data
 * @param sector[in] Start sector to read
 * @param size[in] Total sector to read
 * @return unsigned long
 * @retval Really total sector have been read.
 */
unsigned long udisk_host_read(unsigned long lun, unsigned char data[], unsigned long sector, unsigned long size)
{
    unsigned char  status = 0xff;
    unsigned long byte_offset = 0,sector_offset = 0;
    unsigned long Sectors = size;
    unsigned long SectorMax = HOST_UDISK_MAX_RW_SIZE / (s_pHostUdisk->pLunInfo+lun)->ulBytsPerSec;
    
    if (lun > s_pHostUdisk->ucMaxLun || NULL == data ||
        sector + size > (s_pHostUdisk->pLunInfo+lun)->ulCapacity)
    {
        akprintf(C1, M_DRVSYS, "read param error\n");
        return 0;
    }
    if ((s_pHostUdisk->unLunReadyFlag & LUN_READY(lun)) == 0)
    {
        akprintf(C1, M_DRVSYS, "read lun not ready\n");
        return 0;
    }
    DrvModule_Protect(DRV_MODULE_UDISK_HOST);
    FreqMgr_Lock();
    timer_stop(s_pHostUdisk->lTimerId);
    do
    {
        if (Sectors > SectorMax)
        {
            if (MSC_STAGE_ALL_SUCCESS == msc_read10(lun, data + byte_offset, sector + sector_offset, SectorMax, SectorMax * (s_pHostUdisk->pLunInfo+lun)->ulBytsPerSec, &status))
            {
                Sectors -= SectorMax;
                sector_offset += SectorMax;
                byte_offset += SectorMax* (s_pHostUdisk->pLunInfo+lun)->ulBytsPerSec;
            }
            else
            {
                break;
            }
        }
        else
        {
            if (MSC_STAGE_ALL_SUCCESS == msc_read10(lun, data + byte_offset, sector + sector_offset, Sectors, Sectors * (s_pHostUdisk->pLunInfo+lun)->ulBytsPerSec, &status))
            {
                Sectors = 0;
            }
            else
            {
                break;
            }
        }
    }while(Sectors > 0);
    s_pHostUdisk->lTimerId = timer_start(uiTIMER1, 1000, true, udisk_host_timer_interrupt );
    FreqMgr_UnLock();
    DrvModule_UnProtect(DRV_MODULE_UDISK_HOST);
    return size-Sectors;
    
}

/**
 * @brief   usb host write sector to logic unit
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] Index of logic unit.
 * @param data[in] The write data
 * @param sector[in] Start sector to write
 * @param size[in] Total sectors to write
 * @return unsigned long
 * @retval Really total sectors have been wrote.
 */
unsigned long udisk_host_write(unsigned long lun, unsigned char data[], unsigned long sector, unsigned long size)
{
    unsigned char  status = 0xff;
    unsigned long byte_offset = 0,sector_offset = 0;
    unsigned long Sectors = size;
    unsigned long SectorMax = HOST_UDISK_MAX_RW_SIZE / (s_pHostUdisk->pLunInfo+lun)->ulBytsPerSec;
    
    if (lun > s_pHostUdisk->ucMaxLun || NULL == data ||
        sector + size > (s_pHostUdisk->pLunInfo+lun)->ulCapacity)
    {
        akprintf(C1, M_DRVSYS, "write param error\n");
        return 0;
    }
    if ((s_pHostUdisk->unLunReadyFlag & LUN_READY(lun)) == 0)
    {
        akprintf(C1, M_DRVSYS, "write lun not ready\n");
        return 0;
    }
    DrvModule_Protect(DRV_MODULE_UDISK_HOST);
    FreqMgr_Lock();
    timer_stop(s_pHostUdisk->lTimerId);
    do
    {
        
        if (Sectors > SectorMax)
        {
            if (MSC_STAGE_ALL_SUCCESS == msc_write10(lun, data + byte_offset, sector + sector_offset, SectorMax, SectorMax * (s_pHostUdisk->pLunInfo+lun)->ulBytsPerSec, &status))
            {               
                Sectors -= SectorMax;
                sector_offset += SectorMax;
                byte_offset += SectorMax* (s_pHostUdisk->pLunInfo+lun)->ulBytsPerSec;
            }
            else
            {
                break;
            }
        }
        else
        {
            if (MSC_STAGE_ALL_SUCCESS == msc_write10(lun, data + byte_offset, sector + sector_offset, Sectors, Sectors * (s_pHostUdisk->pLunInfo+lun)->ulBytsPerSec, &status))
            {                
                Sectors = 0;
            }
            else
            {
                break;
            }
        }
    }while(Sectors > 0);
    s_pHostUdisk->lTimerId = timer_start(uiTIMER1, 1000, true, udisk_host_timer_interrupt );
    FreqMgr_UnLock();
    DrvModule_UnProtect(DRV_MODULE_UDISK_HOST);
    return size-Sectors;
}

/**
 * @brief   Udisk host set application level callback.
 *
 * This function must be called by application level before udisk host initialization.
 * @author Huang Xin
 * @date 2010-07-12
 * @param connect_callback[in] Application level callback
 * @param disconnect_callback[in] Application level callback
 * @return  void
 */
void udisk_host_set_callback(T_pfUDISK_HOST_CONNECT connect_callback,T_pfUDISK_HOST_DISCONNECT disconnect_callback)
{
    s_pHostUdisk->pfConnectCb = connect_callback;
    s_pHostUdisk->pfDisconnectCb = disconnect_callback;
}

/**
 * @brief Udisk host disconnect function.
 *
 * This function is called by application level when eject the udisk and exit the udisk host.
 * @author Huang Xin
 * @date 2010-07-12
 * @return  void
 */
void udisk_host_close(void)
{    
    DrvModule_Protect(DRV_MODULE_UDISK_HOST);
    if(s_pHostUdisk->lTimerId != ERROR_TIMER)
        timer_stop(s_pHostUdisk->lTimerId);

    s_pHostUdisk->lTimerId = ERROR_TIMER;

    usb_bus_close();    
    if (NULL != s_pHostUdisk->pLunInfo)
    {
        drv_free(s_pHostUdisk->pLunInfo);
    }
    if (NULL != s_pHostUdisk)
    {
        drv_free(s_pHostUdisk);
    }
    DrvModule_UnProtect(DRV_MODULE_UDISK_HOST);
    
    FreqMgr_CancelFreq(freq_handle);
    freq_handle = FREQ_INVALID_HANDLE;
    
    akprintf(C3, M_DRVSYS, "usbhost close ok\n");
}

static void udisk_host_timer_interrupt(signed long timer_id, unsigned long delay)
{
    unsigned long msg_param = USB_DEVICE_CLASS_STORAGE;
    
    DrvModule_Send_Message(DRV_MODULE_USB_BUS, MESSAGE_CONNECT, &msg_param);
}
/**
 * @brief Udisk host enum ok call back func.
 *
 * This function is called by bus drv when enum success.
 * @author Huang Xin
 * @date 2010-07-12
 * @return  void
 */
static void udisk_host_enum_ok_cb(void)
{
    unsigned char i,lun,status;
    unsigned char data[0xff] = {0};
    unsigned char sense[18] = {0};
    unsigned char retry = 0;
    unsigned char csw_status = NO_CSW_STATUS;
    unsigned short lun_ready_flag;

    
    //get max lun
    if (NULL == s_pHostUdisk->pLunInfo)
    {
        if (!msc_get_max_lun(&s_pHostUdisk->ucMaxLun))
        {
            akprintf(C1, M_DRVSYS, "get max lun failed: %x\n", s_pHostUdisk->ucMaxLun);
			return;
		}
        akprintf(C1, M_DRVSYS, "max lun : %x\n", s_pHostUdisk->ucMaxLun);
        s_pHostUdisk->pLunInfo = (T_pH_UDISK_LUN_INFO)drv_malloc(sizeof(T_H_UDISK_LUN_INFO)*(s_pHostUdisk->ucMaxLun+1));
        if (s_pHostUdisk->pLunInfo == NULL)
        {
            akprintf(C1, M_DRVSYS, "s_pHostUdisk->pLunInfo,alloc failed: %x\n", s_pHostUdisk->pLunInfo);
            return;
        }  
        memset(s_pHostUdisk->pLunInfo,0,sizeof(T_H_UDISK_LUN_INFO)*(s_pHostUdisk->ucMaxLun+1));
        for (lun = 0; lun <= s_pHostUdisk->ucMaxLun; lun++)
        {
            //inquiry
            for (retry = 0; retry < 3; retry ++)
            {
                status = msc_inquiry(lun, (s_pHostUdisk->pLunInfo+lun)->InquiryStr, &csw_status);
                if (MSC_STAGE_ALL_SUCCESS == status)
                {
                    if (CMD_FAILED == csw_status)
                    {
                        status = msc_req_sense(lun, sense, &csw_status);
                        if (MSC_STAGE_ALL_SUCCESS == status)
                        {
                            continue;
                        }
                        else
                        {
                            akprintf(C1, M_DRVSYS, "req sense failed_1: %x\n", status);
                            return; 
                        }
                    }
                    akprintf(C1, M_DRVSYS, "retry: %d,inquiry success: %x\n", retry,status);
                    akprintf(C3, M_DRVSYS, "disk name:<");
                    for(i = 8; i < DEV_STRING_BUF_LEN; i++)
                    {
                        putch(*((s_pHostUdisk->pLunInfo+lun)->InquiryStr + i));
                    }
                    akprintf(C3, M_DRVSYS, ">\n");
                    break;
                }
                else
                {
                    akprintf(C1, M_DRVSYS, "inquiry failed: %x\n", status);
                    return; 
                }
            }
        }   
        
        s_pHostUdisk->lTimerId = timer_start(uiTIMER1, 1000, true, udisk_host_timer_interrupt );
    }
    //get lun info
    lun_ready_flag = s_pHostUdisk->unLunReadyFlag;
    for (lun = 0; lun <= s_pHostUdisk->ucMaxLun; lun++)
    {
        //test unit ready
        for (retry = 0; retry < 3; retry ++)
        {
            status = msc_test_unit_ready(lun, &csw_status);
            if (MSC_STAGE_ALL_SUCCESS == status)
            {
                if (CMD_FAILED == csw_status)
                {
                    s_pHostUdisk->unLunReadyFlag &= ~(LUN_READY(lun));
                    status = msc_req_sense(lun, sense, &csw_status);
                    if (MSC_STAGE_ALL_SUCCESS == status)
                    {
                        continue;
                    }
                    else
                    {
                        akprintf(C1, M_DRVSYS, "req sense failed_2: %x\n", status);
                        return; 
                    }
                }
                s_pHostUdisk->unLunReadyFlag |= LUN_READY(lun);
               // akprintf(C1, M_DRVSYS, "retry: %d,test unit ready success: %x\n", retry,status);
                break;
            }
            else
            {
                akprintf(C1, M_DRVSYS, "test unit ready failed: %x\n", status);
                return; 
            }
        }
        if (s_pHostUdisk->unLunReadyFlag == lun_ready_flag)
        {
            //akprintf(C1, M_DRVSYS, "lun[%d] not change,ready flag = 0x%x\n",lun,s_pHostUdisk->unLunReadyFlag);
            continue;
        }
        
        //read format capacity
        for (retry = 0; retry < 3; retry ++)
        {
            status = msc_read_format_capacity(lun, data, &csw_status);
            if (MSC_STAGE_ALL_SUCCESS == status)
            {
                if (CMD_FAILED == csw_status)
                {
                    status = msc_req_sense(lun, sense, &csw_status);
                    if (MSC_STAGE_ALL_SUCCESS == status)
                    {
                        continue;
                    }
                    else
                    {
                        akprintf(C1, M_DRVSYS, "req sense failed_3: %x\n", status);
                        return; 
                    }
                }
                akprintf(C1, M_DRVSYS, "retry: %d,read f_capacity success: %x\n", retry,status);
                break;
            }
            else
            {
                akprintf(C1, M_DRVSYS, "read format capacity failed: %x\n", status);
                break; 
            }
        }
        
        //read  capacity
        for (retry = 0; retry < 3; retry ++)
        {
            status = msc_read_capacity(lun, data, &csw_status);
            if (MSC_STAGE_ALL_SUCCESS == status)
            {
                if (CMD_FAILED == csw_status)
                {
                    status = msc_req_sense(lun, sense, &csw_status);
                    if (MSC_STAGE_ALL_SUCCESS == status)
                    {
                        continue;
                    }
                    else
                    {
                        akprintf(C1, M_DRVSYS, "req sense failed_4: %x\n", status);
                        return; 
                    }
                }
                akprintf(C1, M_DRVSYS, "retry: %d,read capacity success: %x\n", retry,status);
                (s_pHostUdisk->pLunInfo+lun)->ulCapacity = 1 + ((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]); 
                (s_pHostUdisk->pLunInfo+lun)->ulBytsPerSec = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
                akprintf(C1, M_DRVSYS, "lun[%d] sector num: %d, sector size: %d\n",lun,(s_pHostUdisk->pLunInfo+lun)->ulCapacity,(s_pHostUdisk->pLunInfo+lun)->ulBytsPerSec);
                break;
            }
            else
            {
                akprintf(C1, M_DRVSYS, "read  capacity failed: %x\n", lun);
                return; 
            }
        }      
    }

    
    if (NULL != s_pHostUdisk->pfConnectCb)
    {
        //akprintf(C1, M_DRVSYS, "lun ready flag is 0x%x\n",s_pHostUdisk->unLunReadyFlag);
        s_pHostUdisk->pfConnectCb(s_pHostUdisk->unLunReadyFlag);
    }
    else
    {
        akprintf(C1, M_DRVSYS, "pfConnectCb is null\n");
    }
    
}

/**
 * @brief Udisk host disconnect call back func.
 *
 * This function is called by bus drv when device is disconnected.
 * @author Huang Xin
 * @date 2010-07-12
 * @return  void
 */
static void udisk_host_discon_cb(void)
{
    s_pHostUdisk->unLunReadyFlag = 0;
    if (NULL != s_pHostUdisk->pfDisconnectCb)
    {
        s_pHostUdisk->pfDisconnectCb();
    }
    else
    {
        akprintf(C1, M_DRVSYS, "pfDisconnectCb is null\n");
    }

	if (s_pHostUdisk->lTimerId != ERROR_TIMER)
	{
		timer_stop(s_pHostUdisk->lTimerId);
		s_pHostUdisk->lTimerId = ERROR_TIMER;
	}
}




