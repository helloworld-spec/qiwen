/**
 * @file 
 * @brief:
 *
 * This file provides 
 * Copyright (C) 2017 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2017-2-09
 * @version 1.0
 */

#include "anyka_types.h"

#include "drv_gpio.h"

#include "dev_drv.h"
#include "device.h"
#include "platform_devices.h"


#ifdef __cpluscplus
extern "C"{
#endif


static T_DEV_INFO spi_dev = {
	.dev_open_flg  = false,
	.dev_name      = DEV_SPI_1,	
};

static bool  m_bTaskInit = false;

typedef struct
{
	unsigned int  buffer_nb;             ///< buffer number
	unsigned int  buffer_size;             ///< buffer size
	unsigned int  spi_id;
	unsigned char   *pdatapool; 
	unsigned char   *r_buf; 

}T_SPI_COMM_INFO;

static T_SPI_COMM_INFO  m_spi_para = {
	.buffer_nb   = 1,
	.buffer_size = 1024,
};



static int spi_open(int dev_id, void *data)
{

	int ret = -1;

	int share_pin;
	int mode;
	int role;
	int clock;
	T_SPI_INFO *spi_info;
	unsigned int gpio_nb;
	bool         gpio_level;

    if(m_bTaskInit)
    {
        printk( "can't open SPI again,please free!\n");
        return false;
    }

	spi_info          = (T_SPI_INFO *)spi_dev.dev_data;
	m_spi_para.spi_id = spi_info->spi_id;
	
	share_pin = spi_info->spi_share_pin;
	mode      = spi_info->mode;
	role      = spi_info->role;
	clock     = spi_info->clock;

	gpio_nb    = spi_info->ex_gpio.nb;
	gpio_level = spi_info->ex_gpio.high;
		
	m_spi_para.pdatapool = (unsigned char *)drv_malloc(m_spi_para.buffer_size);

    if (NULL == m_spi_para.pdatapool)//malloc fail
    {
        printk( "malloc fail!\n");
		return ret;
    }
	

	// spi share pin configure
	gpio_share_pin_set(ePIN_AS_SPI0+m_spi_para.spi_id, share_pin);//2迆?﹞車?米?那?1
	gpio_pin_group_cfg(ePIN_AS_SPI0+m_spi_para.spi_id);
	
	
	ret = spi_init(m_spi_para.spi_id, mode, role, clock);
	if(!ret)
	{
		drv_free(m_spi_para.pdatapool);
		return ret;
	}
	m_spi_para.r_buf = NULL;

	//SPI_REQUEST_GPIO  configure 
	gpio_set_pin_as_gpio(gpio_nb);
	gpio_set_pin_dir(gpio_nb, gpio_level); // out
	gpio_set_pin_level(gpio_nb, !gpio_level);  // low 
	
	m_bTaskInit = true;
	ret = dev_id;
	return ret;

}


static int spi_close(int dev_id)
{

	int spi_num;
	T_SPI_INFO *spi_info;

	spi_info = (T_SPI_INFO *)spi_dev.dev_data;
	spi_num   = spi_info->spi_id;
	
	m_bTaskInit = false;
	m_spi_para.r_buf = NULL;
	spi_close(spi_num);
	drv_free(m_spi_para.pdatapool);
	return 0;

}





static int spi_read(int dev_id, void *data, unsigned int len)
{
	T_SPI_INFO *spi_info;
	int ret = 0 ;
	int role;

	
	spi_info  = (T_SPI_INFO *)spi_dev.dev_data;

	role      = spi_info->role;
	
	if(role)	//	master
	{
		m_spi_para.r_buf = (unsigned char*)data;
	}
	

	return ret;

}


static int spi_write(int dev_id, const void *data, unsigned int len)
{
	T_SPI_INFO *spi_info;

	unsigned long ret;
	bool tmp;
	unsigned int data_len = len;
	int role;

	
	spi_info          = (T_SPI_INFO *)spi_dev.dev_data;

	role      = spi_info->role;

	
	if(role)	//	master
	//if(m_spi_para.r_buf != NULL)
	{
		m_spi_para.pdatapool = (unsigned char *)data;
		if((data_len < 512) || (0 == data_len%512))
		{
			ret = data_len;
			tmp = spi_master_write_read(m_spi_para.spi_id, m_spi_para.pdatapool, m_spi_para.r_buf,ret, 1);
		}
		else
		{
			ret = data_len - data_len%512 ;
			tmp = spi_master_write_read(m_spi_para.spi_id, m_spi_para.pdatapool, m_spi_para.r_buf,ret, 1);
			m_spi_para.pdatapool +=  ret;
			m_spi_para.r_buf += ret;
			tmp = spi_master_write_read(m_spi_para.spi_id, m_spi_para.pdatapool, m_spi_para.r_buf,ret, 1);
		}	

		if(0 == tmp)
		{
			data_len = 0;
		}
		m_spi_para.r_buf = NULL;

	}
	else
	{

		if(m_spi_para.buffer_size <= len)
		{
			memcpy(m_spi_para.pdatapool,data,m_spi_para.buffer_size);
			ret = m_spi_para.buffer_size;
		}
		else
		{
			memcpy(m_spi_para.pdatapool,data,len);
			ret = len;
		}
		
		tmp = spi_dma_int_write(m_spi_para.spi_id, m_spi_para.pdatapool, m_spi_para.buffer_size);//﹞⊿1足?“米?∩車D?
		if(0 == tmp)
		{
			ret = 0;
		}

		data_len = ret;
	}



	return (int)data_len;
}

static int spi_ioctl(int dev_id, unsigned long cmd, void *data)
{
	
	unsigned long ret;
	int spi_num;
	T_SPI_INFO *spi_info;
	int baudrate = *(int *)data;

	spi_info = (T_SPI_INFO *)spi_dev.dev_data;
	spi_num   = spi_info->spi_id;

	//TBD

	return 0;
	
}



static T_DEV_DRV_HANDLER spi_function_handler = 
{

    .drv_open_func  = spi_open,
    .drv_close_func = spi_close,
    .drv_read_func  = spi_read,
    .drv_write_func = spi_write,
    .drv_ioctl_func = spi_ioctl,
};




static int spi_device_reg(void)
{
	void *devcie = NULL;

	devcie  = platform_get_devices_info(spi_dev.dev_name);
	if(NULL == devcie)
	{
		printk("spi devcie register fail!");
		return 0;
	}
	
	if (0 !=  dev_alloc_id(&spi_dev,0, spi_dev.dev_name))
	{
		printk("spi devcie alloc id fail!");
		return 0;
	}
	
	
	
	spi_dev.dev_data               = devcie;
	spi_dev.drv_handler            = &spi_function_handler;
	spi_function_handler.devcie    = &spi_dev;
	spi_function_handler.device_id = spi_dev.dev_id;
	
	dev_drv_reg(spi_dev.dev_id, &spi_dev);
}

dev_module_init(spi_device_reg)//辰a?迆set_platform_devices_info??o車,∩??迆?y3㏒1∟℅¯

#ifdef __cplusplus
}
#endif

