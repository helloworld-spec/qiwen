/**
 * @filename hal_usb_mass.h
 * @brief: AK3223M Mass Storage of usb.
 *
 * This file describe mass storage protocol of usb disk.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */

#ifndef __HAL_USB_MASS_H__
#define __HAL_USB_MASS_H__

#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DATA_STAGE_NONE = 0,
    DATA_STAGE_RECV,
    DATA_STAGE_SEND
}
E_DATA_PHASE;

typedef struct tagCmdResult
{
    unsigned char status;
    
    unsigned short data_stage;
    unsigned long data_count;

    unsigned long ack_result;
}
T_CMD_RESULT;

//ret: true, continue, false, quit usb_main; compatible to module burn
typedef bool (*T_FCBK_CMD)(unsigned char data[], unsigned long len, T_CMD_RESULT *result);
typedef bool (*T_FCBK_RCV)(unsigned long buf, unsigned long len);
typedef bool (*T_FCBK_SND)(unsigned long buf, unsigned long len);

typedef struct tagTRANSC
{
    unsigned long cmd;
    T_FCBK_CMD fCmd;
    T_FCBK_RCV fRcv;
    T_FCBK_SND fSnd;
}
T_ANYKA_TRANSC;

//********************************************************************
/*@}*/                      
#ifdef __cplusplus    
}                     
#endif                
                      
#endif                
                      
                      
                      
                      
                      
                      
                      
                      
                      
