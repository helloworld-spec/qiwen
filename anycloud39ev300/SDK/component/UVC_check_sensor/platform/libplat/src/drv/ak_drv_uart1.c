
/**
 * @file  ak_drv_uart1.c
 * @brief : uart1 device driver source file
 *
 * This file provides SPI APIs: SPI initialization, write data to SPI, read data from SPI
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2016-12-04
 * @version 1.0
 */


#include "dev_drv.h"
#include "ak_drv_uart1.h"

#include "ak_common.h"
#include "anyka_types.h"


#define UART2_NO_DEVICE          (-1)
#define UART2_DEVICE_OPEN_FAILED (-2)

typedef struct 
{
	int fd;
	int baud_rate;
	int parity;
	
}T_HAL_UART;

static volatile T_HAL_UART m_hal_uart = {0};



 /**
 * @brief  open uart1 device
 * @author 
 * @date 2016-12-04
 * @param baud_rate[IN] set uart baudrate
 * @param parity[IN] set uart parity.
 *                          bit7=0,disable parity;
 *                          bit7=1,enable parity. bit0=0,odd parity;bit=1,even parity.
 * @return int
 * @retval  < 0 :  failed
 * @retval  = 0 : successful
 */
int ak_drv_uart1_open(int baud_rate, int parity)
{
	int ret;
	
	
	m_hal_uart.fd = dev_open(DEV_UART_1);
	
	if (m_hal_uart.fd > 0)
	{
		if(m_hal_uart.baud_rate != baud_rate)
		{
			m_hal_uart.baud_rate = baud_rate;
			ret = dev_ioctl(m_hal_uart.fd,IO_UART_BAUD_RATE,(void*)&m_hal_uart.baud_rate);
			if(0 != ret)
			{
				ak_print_error("uart set baudrate fail.\r\n");
				return ret;
			}
		}

		if(m_hal_uart.parity != parity)
		{
			m_hal_uart.parity = parity;
			ret =  dev_ioctl(m_hal_uart.fd,IO_UART_PARITY,(void*)&m_hal_uart.parity);
			if(0 != ret)
			{
				ak_print_error("uart set parity fail.\r\n");
				return ret;
			}

		}
		
	}
	else
	{
		return UART2_DEVICE_OPEN_FAILED;
	}

	return 0;

}


 /**
 * @brief close uart1 device
 * @author 
 * @date 2016-12-04
 * @return int
 * @retval  < 0 :  failed
 * @retval  = 0 : successful
 */
int ak_drv_uart1_close(void)
{

	int ret;
	
	ret = dev_close(m_hal_uart.fd);

	return ret;

}


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
int ak_drv_uart1_write(unsigned char *data, int len)
{
	int ret = -1;
	if(len > 0)
	{
		ret = dev_write(m_hal_uart.fd, data, len);
	}
	return ret;
}


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
int ak_drv_uart1_read(unsigned char *data, int len, long ms)
{
	int ret = -1;
	long time = ms;
	
	if(len <= 0)
	{
		return ret;
	}

	if(0 == time)	//	非阻塞
	{
		//要先用IOCTL 设置，在读。顺序不能反。
		dev_ioctl(m_hal_uart.fd, IO_BLOCK_NO, NULL);
	}
	else
	{
		if(time > 0)//等待时间
		{
			//要先用IOCTL 设置，在读。顺序不能反。
			dev_ioctl(m_hal_uart.fd, IO_BLOCK_MS, &time);
 		}
		else//阻塞
		{	
			//要先用IOCTL 设置，在读。顺序不能反。
			dev_ioctl(m_hal_uart.fd, IO_BLOCK_WAIT, NULL);
 		}
	}
	
	ret = dev_read(m_hal_uart.fd, data, len);

	return ret;

}
 


