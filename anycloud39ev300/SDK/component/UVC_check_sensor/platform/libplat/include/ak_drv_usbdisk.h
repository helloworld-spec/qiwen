/**
 * @file  ak_drv_usbdisk.h
 * @brief: Implement  operations of how to use usb disk device.
 *
 * Copyright (C) 2016 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  KeJianping
 * @date    2016-12-04
 * @version 1.0
 */

#ifndef __AK_DRV_USBDISK_H__
#define __AK_DRV_USBDISK_H__

#ifdef __cplusplus
extern "C" {
#endif



 /**
 * @brief  
 * @author 
 * @date 2016-12-04
 * @param void
 * @return int
 * @retval  < 0 :  failed
 * @retval  = 0 : successful
 */
int ak_drv_udisk_start(void);


 /**
 * @brief  
 * @author 
 * @date 2016-12-04
 * @param void
 * @return int
 * @retval  < 0 :  failed
 * @retval  = 0 : successful
 */
int ak_drv_udisk_stop(void);


/*@}*/
#ifdef __cplusplus
}
#endif
#endif //#ifndef __AK_DRV_USBDISK_H__



