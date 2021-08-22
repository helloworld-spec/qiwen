/**
   @filename fwl_usb_h_interrupt.h
 * @brief: AK3223M usb interrupt.
 *
 * This file describe interrupt of usb.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 * @ref
 */

#ifndef __USB_HOST_INT_H__
#define __USB_HOST_INT_H__

#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup Fwl_USB_H_INTERUPT Framework USB_H Interupt Interface
 *  @ingroup Framework
 */
/*@{*/
//********************************************************************
struct S_USB_HOST_INTERRUPT
{
    volatile bool usb_dma1_int;
    volatile bool usb_dma2_int;
    volatile bool usb_dma3_int;
    volatile bool usb_ep0_int;
    volatile bool usb_ep1_int;
    volatile bool usb_ep2_int;
    volatile bool usb_ep3_int;
};

typedef struct S_USB_HOST_INTERRUPT T_USB_HOST_INTERRUPT;

//********************************************************************
/*@}*/
#ifdef __cplusplus
}
#endif

#endif
