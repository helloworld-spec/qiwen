/**
   @file   hal_usb_h_disk.h
 * @brief  provide usb host api functions.
 *
 * This file describe frameworks of udisk host driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-10
 * @version 1.0
 */

#ifndef __USB_HOST_DISK_H__
#define __USB_HOST_DISK_H__

#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup udisk_host udisk_host
 *	@ingroup USB
 */
/*@{*/

 
#define DEV_STRING_BUF_LEN          0x24    
#define LUN_READY(x)                (1<<x)

typedef struct _H_UDISK_LUN_INFO
{
    unsigned long       ulBytsPerSec;
    unsigned long       ulCapacity;
    unsigned char        InquiryStr[DEV_STRING_BUF_LEN + 1];
}
T_H_UDISK_LUN_INFO, *T_pH_UDISK_LUN_INFO;


typedef void (*T_pfUDISK_HOST_CONNECT)(unsigned short lun_ready_flag);
typedef void (*T_pfUDISK_HOST_DISCONNECT)(void);


/**
 * @brief   init udisk host function
 *
 * Allocate udisk host buffer,init data strcut,register callback,open usb controller and phy.
 * @author Huang Xin
 * @date 2010-07-12
 * @param[in] mode usb mode 1.1 or 2.0
 * @return bool
 * @retval false init failed
 * @retval AK_TURE init successful
 */
bool udisk_host_init(unsigned long mode);

/**
 * @brief   get disk all logic unit number
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @return unsigned char
 * @retval  Total number of logic unit.
 */
unsigned char udisk_host_get_lun_num(void);

/**
 * @brief   get a logic unit number descriptor
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param[in] LUN Index of logic unit.
 * @param[out] disk_info  The information of the lun
 * @return  void.
 */
void udisk_host_get_lun_info(unsigned long LUN, T_pH_UDISK_LUN_INFO disk_info);

/**
 * @brief   usb host read sector from logic unit
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param[in] LUN index of logic unit.
 * @param[in] data Buffer to store data
 * @param[in] sector Start sector to read
 * @param[in] size Total sector to read
 * @return unsigned long
 * @retval Really total sector have been read.
 */
unsigned long udisk_host_read(unsigned long LUN, unsigned char data[], unsigned long sector, unsigned long size);

/**
 * @brief   usb host write sector to logic unit
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param[in] LUN Index of logic unit.
 * @param[in] data The write data
 * @param[in] sector Start sector to write
 * @param[in] size Total sectors to write
 * @return unsigned long
 * @retval Really total sectors have been wrote.
 */
unsigned long udisk_host_write(unsigned long LUN, unsigned char data[], unsigned long sector, unsigned long size);

/**
 * @brief   Udisk host set application level callback.
 *
 * This function must be called by application level after udisk host initialization.
 * @author Huang Xin
 * @date 2010-07-12
 * @param[in] connect_callback Application level callback
 * @param[in] disconnect_callback Application level callback
 * @return  void
 */
 void udisk_host_set_callback(T_pfUDISK_HOST_CONNECT connect_callback,T_pfUDISK_HOST_DISCONNECT disconnect_callback);

/**
 * @brief Udisk host close function.
 *
 * This function is called by application level when eject the udisk and exit the udisk host.
 * @author Huang Xin
 * @date 2010-07-12
 * @return  void
 */
void udisk_host_close(void);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif
