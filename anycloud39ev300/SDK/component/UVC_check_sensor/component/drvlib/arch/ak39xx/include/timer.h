/**
 * @file timer.h
 * @brief Virtual timer function header file
 *
 * This file provides virtual timer APIs: initialization, start timer, stop timer and
 * timer interrupt handler.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author ZouMai
 * @date 2004-09-22
 * @version 1.0
 */

#ifndef __HTIMER_H__
#define __HTIMER_H__

#include "drv_timer.h"

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus


/** @defgroup TIMER 
 *      @ingroup M3PLATFORM
 */
/*@{*/


/**
 * @brief: Init timer. Mainly to open timer interrupt.
 * @author ZouMai
 * @date 2004-09-22
 * @return void
 * @retval
 */
void timer_init( void );

/**
 * @brief: reset timer register to default value
 * @author:  xue
 * @date 2006-11-29
 * @return void
 * @retval
 */
void timer_reset(void);


unsigned long timer_interval(T_TIMER_ID hardware_timer);

/*@}*/

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // #ifndef __HTIMER_H__

