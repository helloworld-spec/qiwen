/**
 * @file freq.h
 * @brief: This file describe ...
 * 
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Miaobaoli
 * @date 2005-07-13
 * @version 1.0
 */
#ifndef __FREQ_H__
#define __FREQ_H__


/** @defgroup Interrupt  
 *  @ingroup M3PLATFORM
 */
/*@{*/
#include "anyka_cpu.h"
#include "anyka_types.h"

typedef enum
{
    E_MEMORY_CALLBACK = 0,  //<<< callback for memory
    E_UART_CALLBACK,        //<<< callback for uart
    E_ASIC_CALLBACK,        //<<< callback for other device that based on asic freq 
    E_PLL_CALLBACK          //<<< callback for other device that based on pll freq 

}
E_FREQ_CALLBACK_TYPE;

typedef void (*T_fFREQ_CHANGE_CALLBACK)(unsigned long Freq);


void freq_register(E_FREQ_CALLBACK_TYPE type, T_fFREQ_CHANGE_CALLBACK callback);

#endif

