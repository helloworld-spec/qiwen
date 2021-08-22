/**
 * @file  ak_drv_uart1.h
 * @brief: Implement  operations of how to use uart device.
 *
 * Copyright (C) 2016 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  KeJianping
 * @date    2016-12-04
 * @version 1.0
 */

#ifndef __AK_UART2_H__
#define __AK_UART2_H__

#ifdef __cplusplus
extern "C" {
#endif


 /**
 * @brief  open uart1 device
 * @author 
 * @date 2016-12-04
 * @param baud_rate[IN] set uart baudrate
 * @param parity[IN] set uart parity
 * @return int
 * @retval  < 0 :  failed
 * @retval  = 0 : successful
 */
int ak_drv_uart1_open(int baud_rate, int parity);

 /**
 * @brief close uart1 device
 * @author 
 * @date 2016-12-04
 * @return int
 * @retval  < 0 :  failed
 * @retval  = 0 : successful
 */
int ak_drv_uart1_close(void);


 /**
 * @brief write data to uart1 device. 
 * @author 
 * @date 2016-12-04
 * @param data[IN]buffer to store write data 
 * @param len[IN]the length to write
 * @return int
 * @retval  < 0 :  failed
 * @retval  = 0 : successful
 */
int ak_drv_uart1_write(unsigned char *data, int len);


 /**
 * @brief  read data from uart1 device.  
 * @author 
 * @date 2016-12-04
 * @param buf[out] buffer to store read data 
 * @param len[in] the length to read
 * @param ms[in] ms < 0,block; ms = 0,unblock; ms >0,wait time(unit:ms)
 * @return int
 * @retval  < 0 :  failed
 * @retval  >= 0 : successful
 */
int ak_drv_uart1_read(unsigned char *data, int len, long ms);

/*@}*/
#ifdef __cplusplus
}
#endif
#endif //#ifndef __AK_UART1_H__




