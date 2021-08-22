/**@file hal_udisk.h
 * @brief Implement  operations of how to use usb disk.
 *
 * This file describe msc protocol  and ufi cmd process of usb disk.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-24
 * @version 1.0
 */

#ifndef __HAL_UDISK_H__
#define __HAL_UDISK_H__

#include "anyka_types.h"
#include "hal_usb_mass.h"
#include "hal_usb_h_disk.h"
#include "hal_udisk_mass.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HOST_UDISK_MAX_RW_SIZE  (64 * 1024)

typedef struct _H_UDISK_DEV
{  
    unsigned char                ucMaxLun;
    unsigned short               unLunReadyFlag;
    signed long             lTimerId;
    T_pH_UDISK_LUN_INFO  pLunInfo;
    T_pfUDISK_HOST_CONNECT pfConnectCb;
    T_pfUDISK_HOST_DISCONNECT pfDisconnectCb;
}T_H_UDISK_DEV ,*T_pH_UDISK_DEV;








#ifdef __cplusplus
}
#endif

#endif 


