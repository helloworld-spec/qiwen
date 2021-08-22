/**
 * @file 
 * @brief:
 *
 * This file provides 
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2017-6-29
 * @version 1.0
 */
#include "anyka_types.h"

#include "dev_drv.h"
#include "device.h"
#include "platform_devices.h"


#ifdef __cpluscplus
extern "C"{
#endif


static T_DEV_INFO i2c_dev = {
	.dev_open_flg  = false,
	.dev_name      = DEV_I2C,
};

T_I2C_INFO *i2c_info;

static int i2c_dev_open(int dev_id, void *data)
{
	unsigned int nb;
	unsigned int dir;
	T_DEV_INFO   *dev_info;
	int ret = -1;
    
	if(dev_id != i2c_dev.dev_id)
		return -1;
	if(&i2c_dev != data)
		return -1;
	dev_info = (T_DEV_INFO *)data;
	i2c_info = (T_I2C_INFO *)dev_info->dev_data;
	if(dev_info->dev_open_flg)
		return 0;
	if(0 == i2c_info->i2c_mode)
	{
		//select I2C port.  0:TWI      1:IIC
		sccb_set_soft_hard_flag(0);
		//init I2C
		sccb_init(0, 0);
	}
	else
	{
		//select I2C port.  0:TWI      1:IIC
		sccb_set_soft_hard_flag(1);
		
		//init I2C
		sccb_init(i2c_info->gpio_scl.nb, i2c_info->gpio_sda.nb);
	}
	return dev_id;

}


static int i2c_dev_ioctl(int dev_id, unsigned long cmd, void *data)
{
	unsigned char device_addr,len;
	unsigned short reg_addr, test_data;
	unsigned long data_addr;
	unsigned char * cmd_data;
	bool return_val = false;
	if(dev_id != i2c_dev.dev_id)
		return -1;
	cmd_data = (unsigned char *)data;
	device_addr = *(unsigned char *)&cmd_data[0];
	len = *(unsigned char *)&cmd_data[1];
	reg_addr = *(unsigned short *)&cmd_data[2];
	data_addr = *(unsigned long *)&cmd_data[4];

	//printk("\ni2c  write dev:0x%x, reg:0x%x,len:%d, data_addr:%x\n", device_addr, reg_addr, len, data_addr);
	if(IO_I2C_BYTE_WRITE == cmd)
	{
		if(sccb_write_data(device_addr, reg_addr, data_addr, len))
			return 0;
		else 
			return -1;
	}
	else if(IO_I2C_WORD_WRITE == cmd)
	{
		//register address is byte width
		if(sccb_write_data3(device_addr, reg_addr, data_addr, len))
			return 0;
		else 
			return -1;
	}
	else if(IO_I2C_BYTE_READ == cmd)
	{
		if(sccb_read_data2(device_addr, reg_addr, data_addr, len))
			return 0;
		else 
			return -1;
	}
	else if(IO_I2C_WORD_READ == cmd)
	{
		//register address is word width
		if(sccb_read_data3(device_addr, reg_addr, data_addr, len))
			return 0;
		else 
			return -1;

	}
	else
		return -1;
	return 0;
}


static int i2c_dev_close(int dev_id)
{
	return 0;
}


static T_DEV_DRV_HANDLER i2c_function_handler = 
{

    .drv_open_func  = i2c_dev_open,
    .drv_close_func = i2c_dev_close,
    .drv_read_func  = NULL,
    .drv_write_func = NULL,
    .drv_ioctl_func = i2c_dev_ioctl,
};




static int i2c_device_reg(void)
{
	void *devcie = NULL;

	devcie  = platform_get_devices_info(i2c_dev.dev_name);
	if(NULL == devcie)
	{
		printk("i2c devcie register fail!");
		return 0;
	}

	if (0 !=  dev_alloc_id(&i2c_dev,0, i2c_dev.dev_name))
	{
		printk("i2c  devcie alloc id fail!");
		return 0;
	}
	
	
	i2c_dev.dev_data               = devcie;
	i2c_dev.drv_handler            = &i2c_function_handler;
	i2c_function_handler.devcie    = &i2c_dev;
	i2c_function_handler.device_id = i2c_dev.dev_id;
	
	dev_drv_reg(i2c_dev.dev_id, &i2c_dev);
}

dev_module_init(i2c_device_reg)//要在set_platform_devices_info之后,串口正常工作

#ifdef __cplusplus
}
#endif





