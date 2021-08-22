/**
 * @filename usb_anyka.c
 * @brief: AK3223M how to use usb device of anyka.
 *
 * This file describe frameworks of anyka device.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-14
 * @version 1.0
 */
#include <stdio.h>
#ifdef OS_ANYKA
#include    "anyka_cpu.h"
#include    "anyka_types.h"
#include    "usb_slave_drv.h"
#include    "hal_usb_s_debug.h"
#include    "hal_usb_s_std.h"
#include    "usb_common.h"
#include    "drv_api.h"

#include "interrupt.h"

//********************************************************************


#define MAXSIZE_CONSOLEBUFFER   2048

static unsigned char console_buffer[MAXSIZE_CONSOLEBUFFER];
static unsigned long buffer_head = 0, buffer_tail = 0;

static void Fwl_usb_debug_receive(void);

//********************************************************************
bool usbdebug_enable(void)
{
    usbcdc_init();
    usbcdc_set_callback(Fwl_usb_debug_receive, NULL);
    usbcdc_set_datapool(console_buffer, MAXSIZE_CONSOLEBUFFER);
    usbcdc_enable();

    buffer_head = 0;
    buffer_tail = 0;

    return true;
    
}

//********************************************************************
void usbdebug_disable(void)
{
    usbcdc_disable();

    return;
}

static void Fwl_usb_debug_receive(void)
{
    signed long ret;
    
    do
    {
        ret = usbcdc_read((unsigned char *)console_buffer + buffer_tail, 1); 
        if(ret > 0)
        {
            buffer_tail++;
            buffer_tail &= (MAXSIZE_CONSOLEBUFFER - 1);
        }
        
    }while(ret > 0);
}

unsigned long usbdebug_getstring(unsigned char *str, unsigned long len)
{
    unsigned long chr_count = 0;
    
    while(buffer_head != buffer_tail)
    {
        str[chr_count++] =  console_buffer[buffer_head++];
        buffer_head &= (MAXSIZE_CONSOLEBUFFER - 1);

        if(chr_count >= len)
            break;
    }

    return chr_count;
}


void usbdebug_printf(unsigned char *str, unsigned long len)
{
    if(str == NULL || len == 0)
        return;

    usbcdc_write(str, len);

    return;
}



//********************************************************************
#endif
