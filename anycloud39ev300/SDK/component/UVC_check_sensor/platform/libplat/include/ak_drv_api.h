/**@file ak_drv_api.h
 * @brief driver library interface, define driver APIs.
 * This file provides all the driver APIs needed by upper layer.
 
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author KeJianping
 * @date 2017-3-14
 * @version 1.0
 */

#ifndef __AK_DRV_API_H__
#define __AK_DRV_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#define LIBPLAT_DRV_VERSION "libplat_drv_1.3.00"


#include "ak_drv_detector.h"
#include "ak_drv_i2c.h"
#include "ak_drv_ircut.h"
#include "ak_drv_led.h"
#include "ak_drv_spi.h"
#include "ak_drv_uart1.h"
#include "ak_drv_uvc.h"
#include "ak_drv_wdt.h"



/** 
 * @brief  get  platform driver  version
 *
 * @author KeJianping
 * @date 2017-2-09
 * @param void
 * @return  char*
 */
 const char* ak_plat_drv_get_version(void);

/*@}*/
#ifdef __cplusplus
}
#endif
#endif 




