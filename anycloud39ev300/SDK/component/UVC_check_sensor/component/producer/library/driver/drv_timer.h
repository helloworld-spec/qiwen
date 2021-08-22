/**
 * @file drv_timer.h
 * @brief hardware timer function header file.
 *
 * This file provides hardware timer APIs: start timer, stop timer and
 * timer interrupt handler.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liao_zhijun
 * @date 2010-04-15
 * @version 1.0
 */

#ifndef __DRV_TIMER_H__
#define __DRV_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus


/** @defgroup HTimer Hardware Timer group
 *      ingroup Timer
 */

/*@{*/

#include "anyka_types.h"

/** @{@name ERROR TIMER Define
 *	
 */
#define ERROR_TIMER             -1
/** @} */

/**
 * @brief hardware timer define
 *   define all the hardware timer
 */
 
typedef enum
{
    uiTIMER0 = 0,           ///< TIMER1
    uiTIMER1,               ///< TIMER2
    uiTIMER2,               ///< TIMER3
    uiTIMER3,               ///< TIMER4
    uiTIMER4,               ///< TIMER5

    HARDWARE_TIMER_NUM      ///< MAX TIMER number
} T_TIMER_ID;

/**
 * @brief: Timer Callback type define.
 */
typedef void (*T_fTIMER_CALLBACK)(signed long timer_id, unsigned long delay);

/**
 * @brief Start timer
 *
 * When the time reach, the timer callback function will be called. 
 * Function vtimer_init() must be called before call this function
 * @author liao_zhijun
 * @date 2010-04-15
 * @param[in]  hardware_timer hardware timer ID, uiTIMER0~uiTIMER3, cannot be uiTIMER4, because it is used for tick count
 * @param[in]  milli_sec Specifies the time-out value, in millisecond. Caution, this value must can be divided by 20.
 * @param[in] loop loop or not
 * @param[in] callback_func Timer callback function. If callback_func is
 *      not NULL, then this callback function will be called when time reach.
 * @return signed long timer ID user can stop the timer by this ID
 * @retval ERROR_TIMER: failed
 */
signed long timer_start(T_TIMER_ID hardware_timer, unsigned long milli_sec, bool loop, T_fTIMER_CALLBACK callback_func);

/**
 * @brief Stop timer
 *
 * Function vtimer_init() must be called before call this function
 * @author liao_zhijun
 * @date 2010-04-15
 * @param[in] timer_id Timer ID, uiTIMER0~uiTIMER3, cannot be uiTIMER4, because it is used for tick count
 * @return void
 */
void timer_stop(signed long timer_id);


/*@}*/

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // #ifndef __DRV_TIMER_H__

