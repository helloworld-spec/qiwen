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

#include "arch_freq.h"
#include "arch_init.h"
#include "arch_uart.h"

#include "dev_drv.h"
#include "device.h"
#include "platform_devices.h"


#ifdef AKOS
#include "akos_api.h"
#endif



#ifdef __cpluscplus
extern "C"{
#endif


#define UART_DATA_TOOL  (1024*8)


static T_DEV_INFO uart_dev = {
	.dev_open_flg  = false,
	.dev_name      = DEV_UART_1,	
};


typedef struct
{
	unsigned char   * pdatapool;             ///< buffer number
	
}T_UART2_INFO;

static T_UART2_INFO  m_uart_para;

static unsigned char uart_dev_interrupt_callback(void)
{

	if(NULL != uart_dev.drv_handler->read_complete)
	{
		uart_dev.drv_handler->read_complete(uart_dev.dev_id);//发送信号通知上层
	}
	
	return 0;

}


static int uart_dev_open(int dev_id, void *data)
{
	int baud_rate,uart_num;
	T_UART_INFO *uart_info;
	int ret = -1;

	uart_info = (T_UART_INFO *)uart_dev.dev_data;
	baud_rate  = uart_info->baudrate;
	uart_num   = uart_info->uart_id;
	
	m_uart_para.pdatapool = (unsigned char *)drv_malloc(UART_DATA_TOOL);
	
    if (NULL == m_uart_para.pdatapool)//malloc fail
    {
        printk( "malloc fail!\n");
        return ret;
    }
	
  	
	if(!uart_init(uart_num, baud_rate, get_asic_freq()))
	{
		return ret;
	}
    uart_set_datapool(uart_num, m_uart_para.pdatapool,UART_DATA_TOOL);
    uart_set_callback(uart_num, uart_dev_interrupt_callback);

	ret = dev_id;
	return ret;
}


static int uart_dev_close(int dev_id)
{

	int uart_num;
	T_UART_INFO *uart_info;

	uart_info = (T_UART_INFO *)uart_dev.dev_data;
	uart_num   = uart_info->uart_id;
	
	drv_free(m_uart_para.pdatapool);
	m_uart_para.pdatapool = NULL;

	uart_free(uart_num);

	return 0;

}

static int uart_dev_read(int dev_id, void *data, unsigned int len)
{
	int ret ;

	
	int uart_num;
	T_UART_INFO *uart_info;
	
	uart_info = (T_UART_INFO *)uart_dev.dev_data;
	uart_num   = uart_info->uart_id;

    ret = uart_read(uart_num, data, len);
	
	return ret;

}


static int uart_dev_write(int dev_id, const void *data, unsigned int len)
{
	unsigned long ret;
	int uart_num;
	T_UART_INFO *uart_info;
	
	uart_info = (T_UART_INFO *)uart_dev.dev_data;
	uart_num   = uart_info->uart_id;

	ret = uart_write(uart_num, data,len);

	return (int)ret;
}

static int uart_dev_ioctl(int dev_id, unsigned long cmd, void *data)
{
	
	unsigned long ret;
	int uart_num;
	T_UART_INFO *uart_info;
	int tmp = *(int *)data;
	bool parity_enable;
	bool parity_odd_even;
	
	uart_info = (T_UART_INFO *)uart_dev.dev_data;
	uart_num   = uart_info->uart_id;
	uart_info->baudrate = tmp;
	switch(cmd)
	{
		case IO_UART_BAUD_RATE:
			
			//uart_setbaudrate(uart_num,tmp);
			
			uart_dev_close(uart_num);
			uart_dev_open(uart_num, &tmp);
			break;
			
		case IO_UART_PARITY:
			#if 1
			printk( "nonsupport uart parity!\r\n");
			#else
			parity_enable   = (tmp >> 7)&0x1;
			parity_odd_even = tmp&0x1;
		
			uart_setdataparity(uart_num,parity_enable,parity_odd_even);
			#endif
			break;
		default :
			printk( "uart_dev_ioctl command error!\r\n");
			return -1;

	}

	return 0;
	
}



static T_DEV_DRV_HANDLER uart_function_handler = 
{

    .drv_open_func  = uart_dev_open,
    .drv_close_func = uart_dev_close,
    .drv_read_func  = uart_dev_read,
    .drv_write_func = uart_dev_write,
    .drv_ioctl_func = uart_dev_ioctl,
};


static int uart_device_reg(void)
{
	void *devcie = NULL;

	devcie  = platform_get_devices_info(uart_dev.dev_name);
	if(NULL == devcie)
	{
		printk("uart devcie register fail!");
		return 0;
	}
	
	if (0 !=  dev_alloc_id(&uart_dev,0, uart_dev.dev_name))
	{
		printk("uart devcie alloc id fail!");
		return 0;
	}

	uart_dev.dev_data               = devcie;
	uart_dev.drv_handler            = &uart_function_handler;
	uart_function_handler.devcie    = &uart_dev;
		
	uart_function_handler.device_id = uart_dev.dev_id;
	
	dev_drv_reg(uart_dev.dev_id, &uart_dev);
	
	return 0;
	
}

dev_module_init(uart_device_reg)//要在set_platform_devices_info之后,串口正常工作

#ifdef __cplusplus
}
#endif

