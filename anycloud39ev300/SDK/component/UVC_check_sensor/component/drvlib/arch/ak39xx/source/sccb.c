/**
 * @file sccb.c
 * @brief SCCB interface driver, define SCCB interface APIs.
 * This file provides SCCB APIs: SCCB initialization, write data to SCCB & read data from SCCB.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Jiankui
 * @date 2016-09-08
 * @version 1.0
 * @ref AK3210M technical manual.
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"

#ifdef OS_ANYKA

static bool iic_soft_hard_flag = 0;       // 0:TWI    1:IIC

void sccb_set_soft_hard_flag(bool iic_config)
{
	iic_soft_hard_flag = iic_config;
}

void sccb_init(unsigned long pin_scl, unsigned long pin_sda)
{
	if(iic_soft_hard_flag)
	{
    	i2c_init(pin_scl, pin_sda);
	}
	else 
	{
		twi_init();
	}
}

bool sccb_write_data(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size)
{
	if(iic_soft_hard_flag)
	{
    	return i2c_write_data(daddr, raddr, data, size);
	}
	else 
	{
		return twi_write_data(daddr, raddr, data, size);
	}
}

bool sccb_write_data3(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size)
{
	if(iic_soft_hard_flag)
	{
		return i2c_write_data2(daddr, raddr, data, size);
	}
	else
	{
		return twi_write_data2(daddr, raddr, data, size);
	}
}

bool sccb_write_data4(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size)
{
	if(iic_soft_hard_flag)
	{
    	return i2c_write_data3(daddr, raddr, data, size);
	}
	else
	{
		return twi_write_data3(daddr, raddr, data, size);
	}
}

unsigned char sccb_read_data(unsigned char daddr, unsigned char raddr)
{
    unsigned char readdata = 0;
	if(iic_soft_hard_flag)
	{
    	i2c_read_data(daddr, raddr, &readdata, 1);
	}
	else 
	{
		twi_read_data(daddr, raddr, &readdata, 1);
	}
     return readdata;
}

bool sccb_read_data2(unsigned char daddr, unsigned char raddr, unsigned char *data, unsigned long size)
{
	if(iic_soft_hard_flag)
	{
		return i2c_read_data(daddr, raddr, data, size);
	}
	else
	{
		return twi_read_data(daddr, raddr, data, size);
	}
}

bool sccb_read_data3(unsigned char daddr, unsigned short raddr, unsigned char *data, unsigned long size)
{
	if(iic_soft_hard_flag)
	{
    	return i2c_read_data2(daddr, raddr, data, size);
	}
	else
	{
		return twi_read_data2(daddr, raddr, data, size);
	}
}

bool sccb_read_data4(unsigned char daddr, unsigned short raddr, unsigned short *data, unsigned long size)
{
	if(iic_soft_hard_flag)
	{
		return i2c_read_data3(daddr, raddr, data, size);
	}
	else
	{
		return twi_read_data3(daddr, raddr, data, size);
	}
}

#endif

/* end of file */
