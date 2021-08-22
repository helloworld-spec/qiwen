
/**
 * @file  ak_drv_get_version.h
 * @brief: get the platform driver version
 *
 * Copyright (C) 2016 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  KeJianping
 * @date    2016-12-04
 * @version 1.0
 */

#include "ak_drv_api.h"

#ifndef __AK_DRV_GET_VERSION_H__
#define __AK_DRV_GET_VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif


const char* ak_plat_drv_get_version(void)
{

	return LIBPLAT_DRV_VERSION;

}

/*@}*/
#ifdef __cplusplus
}
#endif
#endif 


