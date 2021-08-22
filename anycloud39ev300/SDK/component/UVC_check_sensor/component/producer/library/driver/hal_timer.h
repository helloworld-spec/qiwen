/**
 * @file hal_timer.h
 * @brief Virtual timer function header file
 *
 * This file provides virtual timer APIs: initialization, start timer, stop timer and
 * timer interrupt handler.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Liao_Zhijun
 * @date 2010-04-15
 * @version 1.0
 */

#ifndef __HAL_TIMER_H__
#define __HAL_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

/** @defgroup Timer Timer group
 *	@ingroup Drv_Lib
 */
/*@{*/
/*@}*/


/** @defgroup VTimer VTimer group
 *	@ingroup Timer
 */
/*@{*/

/** @name ERROR TIMER Define
 *	
 */
/*@{*/
#define ERROR_TIMER			-1
/*@}*/

/**
 * @brief: Timer Callback type define.
 */
typedef void (*T_fVTIMER_CALLBACK)(signed long timer_id, unsigned long delay);

/**
 * @brief  Init virtual timer and hardware timer. 
 *         and then open the hardware timer interrupt;
 * @author liaozhijun
 * @date 2010-04-06
 * @return void
 */
void vtimer_init( void );

/**
 * @brief  free virtual timer and hardware timer.
 * @author liaozhijun
 * @date 2010-04-06
 * @return void
 */
void vtimer_free( void );

/**
 * @brief Start vtimer
 *
 * When the time reaches, the vtimer callback function will be called. User must call function
 * vtimer_stop() to free the timer ID, in spite of loop is true or false.
 * Function  must called vtimer_init() before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @param milli_sec [in] Specifies the time-out value, in millisecond. Caution, this value must can be divided by 5.
 * @param loop [in] oop or not
 * @param callback_func [in] Timer callback function. If callback_func is
 *				not NULL, then this callback function will be called when time reaches.
 * @return signed long
 * @retval timer_ID user can stop the timer by this ID
 * @retval ERROR_TIMER start failed
 */
signed long vtimer_start(unsigned long milli_sec, bool loop, T_fVTIMER_CALLBACK callback_func);

/**
 * @brief Stop vtimer
 *
 * Function  must called vtimer_init() before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @param timer_id [in] Timer ID
 * @return void
 */
void vtimer_stop(signed long timer_id);

/**
 * @brief reset a specified vtimer
 *
 * Function  must called vtimer_init() before call this function
 * @author wuminjin
 * @date 2017-01-16
 * @param timer_id [in] Timer ID
 * @param time [in]  time in milisec
 * @return void
 */
int vtimer_reset( signed long timerID, unsigned int milli_sec);


/**
 * @brief Get Timer total delay, count by ms
 *
 * Function  must called vtimer_init() before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @param timerID [in] Timer ID
 * @return unsigned long total delay of that timer
 */
unsigned long vtimer_get_time( signed long timerID );

/**
 * @brief Get Timer current value, count by ms
 *
 * Function  must called vtimer_init() before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @param timerID [in] Timer ID
 * @return unsigned long current value of that timer
 */
unsigned long vtimer_get_cur_time( signed long timerID );

/**
 * @brief Get unused timer number.
 *
 * Function  user should call vtimer_init() before calling this function
 * @author liaozhijun
 * @date 2010-04-06
 * @return unsigned long the number of unused timer
 */
unsigned long vtimer_validate_count(void);


/**
 * @brief Get ms level tick count from hardware timer
 *
 * tick count value is calculated from timer5
 * Function vtimer_init() must be called before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @return unsigned long
 * @retval tick_count whose unit is ms
 */
unsigned long get_tick_count(void);

/**
 * @brief Get us level tick count from hardware timer
 *
 * tick count value is calculated from timer5.
 * Function vtimer_init() must be called before call this function
 * @author liaozhijun
 * @date 2010-04-06
 * @return unsigned long long
 * @retval tick_count whose unit is us
 */
unsigned long long get_tick_count_us(void);


#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

/*@}*/
#endif // #ifndef __HAL_TIMER_H__

