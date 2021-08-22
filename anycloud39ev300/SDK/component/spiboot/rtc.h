/**
 * @filename rtc.h
 * @brief AK98xx rtc helper functions
 *
 * This file describe how to control the AK98xx rtc.
 * Copyright (C) 2011 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author lixiaoping
 * @date 2011-2-15
 * @version 1.0
 * @ref
 */
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup RTC Architecture RTC Interface
 *  @ingroup Architecture
 */
/*@{*/

#define RTC_SETTING_REG         7
#define WDT_RTC_TIMER		6


T_VOID sys_power_on(T_VOID);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif //__RTC_H__
