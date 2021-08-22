
/**
 * @file dev_drv.h
 * @brief: Implement  operations of how to use devices driver.
 *
 * This file describe devices ioctl command and how to use the devices.
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  KeJianping
 * @date 2017-2-09
 * @version 1.0
 */

#include "ioctl.h"
#include "dev_info.h"


#ifndef  __DEV_DRV_H__
#define  __DEV_DRV_H__

#ifdef __cplusplus
extern "C"{
#endif

/** 
 * @brief  get  devcie driver  version
 *
 * @author KeJianping
 * @date 2017-2-09
 * @param void
 * @return  char*
 */
 const char* ak_dev_drv_get_version(void);



/** 
 * @brief  open devcie  with  devcie's name
 *
 * open the devcie, and then return devcie's ID. 
 * @author KeJianping
 * @date 2017-2-09
 * @param name[in] devcie's name
 * @return  int
 * @retval  < 0 :  failed
 * @retval  > 0 : successful
 */
int dev_open(const char *name);

/** 
 * @brief  close devcie  with  devcie's ID
 *
 * @author KeJianping
 * @date 2017-2-09
 * @param device_id[in] devcie's ID, dev_open() return value.
 * @return  int
 * @retval  < 0 :  failed
 * @retval = 0 : successful
 */
int dev_close(int device_id);


/** 
 * @brief  read data from devcie's driver.  
 *
 * APP read data. 
 * @author KeJianping
 * @date 2017-2-09
 * @param device_id[in] devcie's ID, dev_open() return value.
 * @param buf[out] buffer to store read data 
 * @param len[in] the length to read
 * @return  int
 * @retval  < 0 :  failed
 * @retval  >= 0 : successful
 */
int dev_read(int device_id,  void *buf, unsigned int len);

/** 
 * @brief  write data to devcie's driver. 
 *
 * APP write data
 * @author KeJianping
 * @date 2017-2-09
 * @param device_id[in] devcie's ID, dev_open() return value.
 * @param buf[in] buffer to store write data 
 * @param len[in] the length to read
 * @return  int
 * @retval  < 0 :  failed
 * @retval  >= 0 : successful
 */
int dev_write(int device_id, const void *buf, unsigned int len);


/** 
 * @brief  set IO control command
 *
 * APP set ioctl command
 * @author KeJianping
 * @date 2017-2-09
 * @param device_id[in] devcie's ID, dev_open() return value.
 * @param cmd[in]  commamd to set 
 * @param data[in/out] depends on the command.
 * @return  int
 * @retval  < 0 :  failed
 * @retval  = 0 : successful
 */
int dev_ioctl(int device_id, unsigned long cmd, void *data);

#ifdef __cplusplus
}
#endif

#endif  //#ifndef  __DEV_DRV_H__


