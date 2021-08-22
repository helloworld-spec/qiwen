/**@file drv_api.h
 * @brief driver library interface, define driver APIs.
 * This file provides all the driver APIs needed by upper layer.
 
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liaozhijun
 * @date 2010-5-31
 * @version 1.0
 */
#ifndef __DRV_API_H__
#define __DRV_API_H__

/** @defgroup API API group
 *  @ingroup Drv_Lib
 */
/*@{*/


/**
    驱动库版本更新说明：
    1，当小的修改时，如fix bug,完善功能等，提交时修改小版本号
    2，当有接口变动或增加新功能时，提交时修改中版本号
    3，当架构有调整或支持新的芯片时，修改主版本号
*/
#ifndef _DRV_LIB_VER
#define _DRV_LIB_VER    "DrvLib V1.0.01"
#endif

#include "anyka_types.h"
#include "arch_analog.h"
#include "arch_freq.h"
#include "arch_init.h"
#include "arch_mmc_sd.h"
#include "arch_mmu.h"
#include "arch_pwm.h"
#include "arch_rtc.h"
#include "arch_sdio.h"
#include "arch_spi.h"
#include "arch_uart.h" 
#include "arch_watchdog.h"
#include "arch_twi.h"

#include "hal_camera.h" 
#include "hal_detector.h"
#include "hal_except.h"
#include "hal_gpio.h"
#include "hal_keypad.h"
#include "hal_print.h"
#include "hal_sound.h"
#include "hal_sysdelay.h"
#include "hal_timer.h"
#include "hal_usb_h_disk.h"
#include "hal_usb_s_anyka.h"
#include "hal_usb_s_debug.h"
#include "hal_usb_s_disk.h"
#include "hal_usb_s_state.h"
#include "hal_usb_s_UVC.h"
#include "hal_spiflash.h" 
#include "hal_i_sk_mmc.h"
#include "hal_freqmgr.h"


/*@}*/
#endif  //_DRV_LIB_VER

