/**
 * @file ak_spi.c
 * @brief 
 *
 * This file provides SPI APIs: SPI initialization, write data to SPI, read data from SPI
 * Copyright (C) 2016 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2016-12-04
 * @version 1.0
 */

#include "dev_drv.h"

#include "ak_common.h"
#include "anyka_types.h"



#include "ak_drv_spi.h"

#define SPI_NO_DEVICE          (-1)
#define SPI_DEVICE_OPEN_FAILED (-2)

typedef struct 
{
	int fd;
	int buffer_nb;
	int buffer_size;
	
}T_HAL_SPI;

static volatile T_HAL_SPI m_hal_spi = {0};

 /**
 * @brief
 * @author 
 * @date 2016-12-04
 * @param 
 * @param 
 * @return bool
 * @version 
 */
int ak_drv_spi_open(unsigned long buff_nb, unsigned long size)
{
	int ret;
	
	m_hal_spi.buffer_nb = buff_nb;
	m_hal_spi.buffer_size    = size;
	m_hal_spi.fd        = dev_open(DEV_SPI_1);
	
	if (m_hal_spi.fd > 0)
	{
		ret = dev_ioctl(m_hal_spi.fd,IO_SPI_BUFFER_SIZE,(void*)&m_hal_spi.buffer_nb);
		if(0 != ret)
		{
			ak_print_error("spi set buffer number fail.\r\n");
			return ret;
		}
		
		ret =  dev_ioctl(m_hal_spi.fd,IO_SPI_BUFFER_SIZE,(void*)&m_hal_spi.buffer_size);
		if(0 != ret)
		{
			ak_print_error("spi set buffer size fail.\r\n");
			return ret;
		}
	}
	else
	{
		ak_print_error("spi open fail.\r\n");
		return SPI_DEVICE_OPEN_FAILED;
	}

	return 0;

}


 /**
 * @brief
 * @author 
 * @date 2016-12-04
 * @param 
 * @param 
 * @return bool
 * @version 
 */
int ak_drv_spi_close(void)
{

	int ret;
	
	ret = dev_close(m_hal_spi.fd);

	return ret;

}


 /**
 * @brief
 * @author 
 * @date 2016-12-04
 * @param 
 * @param 
 * @return bool
 * @version 
 */
int ak_drv_spi_write(unsigned char *data, unsigned long len)
{
	int ret = -1;
	if(len > 0)
	{
		ret = dev_write(m_hal_spi.fd, data, len);
	}
	return ret;
}


 /**
 * @brief
 * @author 
 * @date 2016-12-04
 * @param 
 * @param 
 * @return bool
 * @version 
 */
int ak_drv_spi_read(unsigned char *data, unsigned long len, long ms)
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
		dev_ioctl(m_hal_spi.fd, IO_BLOCK_NO, NULL);
		ret = dev_read(m_hal_spi.fd, data, len);
	}
	else
	{
		if(time > 0)//等待时间
		{
			//要先用IOCTL 设置，在读。顺序不能反。
			dev_ioctl(m_hal_spi.fd, IO_BLOCK_MS, &time);
			ret = dev_read(m_hal_spi.fd, data, len);
		}
		else//阻塞
		{	
			//要先用IOCTL 设置，在读。顺序不能反。
			dev_ioctl(m_hal_spi.fd, IO_BLOCK_WAIT, NULL);
			ret = dev_read(m_hal_spi.fd, data, len);
		}
	}

	return ret;

}



 /**
 * @brief
 * @author 
 * @date 2016-12-04
 * @param 
 * @param 
 * @return bool
 * @version 
 */ 
int ak_drv_spi_write_read(unsigned char *wr_data, unsigned char *rd_data, int len)
{
	int ret = -1;

	if( (NULL == wr_data) || (NULL == rd_data))
	{
		return ret;
	}
	
	if(len > 0)
	{
		
		dev_read(m_hal_spi.fd, rd_data, len);
		
		ret = dev_write(m_hal_spi.fd, wr_data, len);
	}
	return ret;
}




