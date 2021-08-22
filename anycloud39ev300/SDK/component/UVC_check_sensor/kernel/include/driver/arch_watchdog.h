/**
 * @file drv_watchdog.h
 * @brief watchdog function header file.
 *
 * This file provides watchdog APIs: start watchdog, feed watchdog and
 * close watchdog.
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Jian_kui
 * @date 2016-09-05
 * @version 1.0
 */
#ifndef __ARCH_WATCHDOG_H__
#define __ARCH_WATCHDOG_H__
	 
#ifdef __cplusplus
	 extern "C" {
#endif // #ifdef __cplusplus


 /**
 * @brief watch dog function init
 * @author Jian_kui
 * @date 2016-09-05
 * @param unsigned short feedtime:watch dog feed time, feedtime unit:ms
 * @return void
  */
void watchdog_timer_start(unsigned long feed_time);

/**
 * @brief watch dog feed
 * @author Jian_kui
 * @date 2016-09-05
 * @return void
  */
void watchdog_timer_feed(void);

/**
 * @brief watch dog stop
 * @author Jian_kui
 * @date 2016-09-05
 * @return void
  */
void watchdog_timer_stop(void);

/*@}*/

#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // #ifndef __ARCH_WATCHDOG_H__

