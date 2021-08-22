/**
*@file hal_usb_s_uvc.h
*@brief provide operations of how to use USB Video Class device.
*
*This file describe frameworks of USB Video Class device
*Copyright (C) 2007 Anyka (Guangzhou) Software Technology Co., LTD
*@author Huang Xin
*@date 2010-07-10
*@version 1.0
*/

#ifndef _HAL_S_UVC_H_
#define _HAL_S_UVC_H_

#include "hal_uvc.h"
#include "hal_usb_s_UVC.h"
#include "hal_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

#define     UVC_VID                     0x04D6//0x04D6
#define     UVC_PID                     0xE102//E1:sword  02:uvc device
#define     UVC_BCD                     0x0020
#define     UVC_INTERFACE_NUM           2
#define     UVC_SUPPORTED_CTRL_NUM      6         

#define     UVC_VCI_ID                  0x00
#define     UVC_VSI_ID                  0x01
#define     UVC_SU_ID                   0x04
#define     UVC_PU_ID                   0x05
#define     UVC_CT_ID                   0x01
#define     UVC_IT_ID                   0x02
#define     UVC_OT_ID                   0x03

#define     LOBYTE(w)                   ((unsigned char)(w))
#define     HIBYTE(w)                   ((unsigned char)(((unsigned short)(w) >> 8) & 0xFF))

#define     UVC_CTRL_PARAM_GRP(index,value)   (((index)<<16) | (value))      

//number 0  is not allowed to define udisk msg
#define     UVC_DEV_MSG_CTRL                 1
#define     UVC_DEV_MSG_SEND                 2

typedef enum 
{
    MSG_VC_CTRL,
    MSG_VS_CTRL
}T_eUVC_DEV_MSG_CTRL_ID;

typedef struct _UVC_DEV_MSG
{
    unsigned char           ucMsgId;
    unsigned char           ucParam1;
    unsigned char           ucParam2;
    unsigned long          ulParam3;
    unsigned long          ulParam4;
}T_UVC_DEV_MSG,*T_pUVC_DEV_MSG;


typedef struct _UVC_CONTROL_SETTING
{
    unsigned long   ulCur;
    unsigned long   ulMin;
    unsigned long   ulMax;
    unsigned long   ulRes;
    unsigned long   ulLen;
    unsigned long   ulDef;
    unsigned char    ucInfo;
}T_UVC_CONTROL_SETTING,*T_pUVC_CONTROL_SETTING;

typedef struct _UVC_DEV
{
    unsigned long ulMode;
    unsigned char  ucFormat;
    T_UVC_DEV_MSG tMsg;
    T_UVC_CONTROL_SETTING tCtrlSetting[UVC_SUPPORTED_CTRL_NUM];
    //T_CAMERA_BUFFER YuvBuf[UVC_FRAME_BUF_NUM];
    T_pUVC_VC_CTRL_CALLBACK fVcCtrlCallBack;
    T_pUVC_VS_CTRL_CALLBACK fVsCtrlCallBack;
    T_pUVC_FRAME_SENT_CALLBACK fFrameSentCallBack;
    
}T_UVC_DEV, *T_pUVC_DEV;



#ifdef __cplusplus
}
#endif

#endif


