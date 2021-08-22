/**@file hal_sysdelay.h
 * @brief provide delay operations
 
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author guoshaofeng
 * @date 2007-04-23
 * @version 1.0
 */

#ifndef __HAL_SYSDELAY_H__
#define __HAL_SYSDELAY_H__

/** @defgroup Delay Delay group
 *	@ingroup Drv_Lib
 */
/*@{*/

/**
 * @brief millisecond delay
 * @author guoshaofeng
 * @date 2007-04-23
 * @param[in] minisecond minisecond delay number
 * @return void
 */
void mini_delay(unsigned long minisecond);

/**
 * @brief microsecond delay
 * @author guoshaofeng
 * @date 2007-04-23
 * @param[in] us microsecond delay number
 * @return void
 */
void us_delay(unsigned long us);

/*@}*/
#endif  //#ifndef __HAL_SYSDELAY_H__
