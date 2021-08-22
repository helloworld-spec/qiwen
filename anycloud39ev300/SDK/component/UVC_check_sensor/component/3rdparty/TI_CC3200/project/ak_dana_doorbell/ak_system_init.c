/**
 * @file  
 * @brief 
 *
 * This file provides SPI APIs: SPI initialization, write data to SPI, read data from SPI
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2016-12-3
 * @version 1.0
 */
 
#include "hw_types.h"
#include "pinmux.h"
//#include "gpio.h"

#include "ak_pub_def.h"
#include "ak_system_init.h"
#include "ak_spi.h"
#include "ak_uart.h"
//#include "ak_gpio.h"
//#include "vtimer.h"
#include "ak_key_led.h"
#include "i2c_if.h"

#include "osi.h"


#include "cc_types.h"
#include "rtc_hal.h"
#include "gpio_hal.h"
#include "uart_drv.h"
#include "cc_timer.h"
#include "cc_pm_ops.h"
#include "cc_pm.h"


//#define GPIO_PIR         2
//#define GPIO_POWER_AK    8
//#define GPIO_KEY         4

volatile unsigned long wakeup_type = 0;
extern volatile  unsigned long UartRxLen;
extern volatile  unsigned long UartIntFlg;
OsiSyncObj_t  door_ring_sem = NULL;
//void ak_board_init(void);

unsigned char g_ak_statu = 0;


volatile unsigned char key_test_flg = 0;
volatile char doorbell_key = 0;
volatile unsigned char ak_start_flg;
#define PMU_DEV_ADDR 0x34

unsigned char power_off_data0[2]={0x36,0x1D};
unsigned char power_off_data1[2]={0x32,0xC6};
unsigned char power_on_data[2]={0x23,0x12};//0x12 1.15v   0x14 1.2v


void power_off_ak(void)
{
	I2C_IF_Open(I2C_MASTER_MODE_FST);
	
	//2.设置开机时间128ms
	if(-1 == I2C_IF_Write(PMU_DEV_ADDR,power_off_data0,2,1))
	{
		Report("config start time error\r\n");
	}
	//	3.操作寄存器关闭电源输出
	//通过I2c，写0xC6到pmu寄存器0x32
	if(-1 == I2C_IF_Write(PMU_DEV_ADDR,power_off_data1,2,1))
	{
		Report("close pmu error\r\n");
	}

	I2C_IF_Close();
}



/**
* @brief 
* @author 
* @date 2016-12-2
* @param  on_off: "1" on, "0" off
* @return void
*/
int power_ak(unsigned char on_off)
{
	cc_hndl wake_gpio_hndl;

	wake_gpio_hndl = cc_gpio_open(GPIO_POWER_AK, GPIO_DIR_OUTPUT);
	//volatile int i ,j;
	//gpio_init(GPIO_POWER_AK, 0);
	//gpio_set_pin_dir(GPIO_POWER_AK, 1);
	
	//I2C_IF_Open(I2C_MASTER_MODE_FST);
	
	if(0 == on_off)
	{
		//ak_start_flg = 0;
		//wakeup_type = 0; //sleep clear
		//off
		//1.将IO8，拉高
		//gpio_set_pin_level(GPIO_POWER_AK, 0);
		if(g_ak_statu == 0) return -1;
		cc_gpio_write(wake_gpio_hndl, GPIO_POWER_AK, 0);
		g_ak_statu = 0;
		Report("ak power off\r\n");
#if 0

		//2.设置开机时间128ms
		if(-1 == I2C_IF_Write(PMU_DEV_ADDR,power_off_data0,2,1))
		{
			Report("config start time error\r\n");
		}
		//	3.操作寄存器关闭电源输出
		//通过I2c，写0xC6到pmu寄存器0x32
		if(-1 == I2C_IF_Write(PMU_DEV_ADDR,power_off_data1,2,1))
		{
			Report("close pmu error\r\n");
		}
#endif
		
	}
	else
	{		
		//UartRxLen = 0;
		//UartIntFlg = 0;
		//on
		//1.将IO8，拉低
		//gpio_set_pin_level(GPIO_POWER_AK, 1);
		if(g_ak_statu == 1) return -1;
		cc_gpio_write(wake_gpio_hndl, GPIO_POWER_AK, 1);
		 g_ak_statu = 1;
		Report("ak power on\r\n");
		//2.延时200
		//osi_Sleep(200);
#if 0
		for(i = 0; i < 5000; i++)
			for(j = 0; j < 1000; j++);			
			
		//3.配置DCDC2的电压为1.15V
		if(-1 == I2C_IF_Write(PMU_DEV_ADDR,power_on_data,2,1))
		{
			Report("config 1.15V error\r\n");
		}
#endif	
	}
	return 0;
	//I2C_IF_Close();
}

/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
static void pir_Interrupt_handle(unsigned char gpio)
{
	if(0 == wakeup_type)
	{
		wakeup_type = EVENT_PIR_WAKEUP;
		power_ak(1);
		osi_SyncObjSignalFromISR(&door_ring_sem); //call one times
	}

	Report("pir_Interrupt_handle\r\n");
}


/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
static void key_cb(void)
{
	if(0 == wakeup_type)
	{
		wakeup_type = EVENT_RING_CALL_WAKEUP;
		power_ak(1);
	}
	key_test_flg++;
	
	osi_SyncObjSignalFromISR(&door_ring_sem); 
	
	Report("key_cb\n\r");
}


/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
void ak_board_init(void)
{
	int ret;
	Report("==ak_board_init %x\n\r",*(volatile long *)(0x4402f850));

    ret = osi_SyncObjCreate(&door_ring_sem);
	if(ret != OSI_OK)
	{
		Report("osi_SyncObjCreate  err!\r\n ");
		while(1);
	}
	//power AK init
	Report("ak_board_init\n\r");
	//gpio_init(GPIO_POWER_AK, 0);
	//gpio_set_pin_dir(GPIO_POWER_AK, 1);

	//PIR
#if 0
	gpio_init(GPIO_PIR, 0);  //PULL DOWN
	gpio_set_pin_dir(GPIO_PIR, 0);//INPUT

	gpio_set_int_p(GPIO_PIR, GPIO_RISING_EDGE); //set high level interrupt
	gpio_register_int_callback(GPIO_PIR, pir_Interrupt_handle);  //register callback
	gpio_int_control(GPIO_PIR, 1); //enable gpio interrupt
#endif
	
	timer_init(0,10);
	//key for calling
	//key_open(GPIO_KEY,1,100,key_cb);  //GPIO_KEY , 1==high level interrupt, 100ms 按下消抖
	//PIR for calling
	//key_open(GPIO_PIR,1,100,pir_Interrupt_handle);  //GPIO_KEY , 1==high level interrupt, 100ms 按下消抖

	

}



/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
void ak_sys_init(void)
{
	
	Report("=1=ak_sys_init\r\n ");
	ak_board_init();
	Report("=2=ak_sys_init\r\n ");
	
	ak_uart1_init();
	Report("=3=ak_sys_init\r\n ");

	ak_spi_init();
	Report("=4=ak_spi_init\r\n ");


}

