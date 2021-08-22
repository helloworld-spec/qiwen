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
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_ints.h"
#include "spi.h"
#include "rom.h"
#include "rom_map.h"
#include "utils.h"
#include "prcm.h"
#include "uart.h"
#include "interrupt.h"
#include "cc_types.h"

#include "uart_drv.h"
#include "hw_uart.h"
#include "ak_pub_def.h"
#include "ak_uart.h"
#include "ak_system_init.h"
#include "ak_adc.h"

#define UART_DEBUG


#define UART1_PIPE_BAUD  115200
#define UART1_TX_PIN     PIN_58
#define UART1_TX_MOD     PIN_MODE_6
#define UART1_RX_PIN     PIN_45
#define UART1_RX_MOD     PIN_MODE_2 
#define UartPutChar(c)       MAP_UARTCharPut(UARTA1_BASE,c)

#define MAX_STRING_LENGTH    50

static unsigned char ucUartBuff[MAX_STRING_LENGTH+1];
volatile  unsigned long UartRxLen = 0;
volatile  unsigned long UartIntFlg = 0;
extern volatile unsigned char ak_start_flg;
extern volatile unsigned long  gpio_spi_int;

extern unsigned long ad_average;

/*
OsiLockObj_t uart_rx_mutex;
#define  UART_RX_LOCK(mutex)    (osi_LockObjLock(&mutex,OSI_WAIT_FOREVER))
#define  UART_RX_UNLOCK(mutex)  (osi_LockObjUnlock(&mutex))
*/
/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
void ak_uart_write(unsigned char* buf,unsigned long len)
{
	unsigned char* tmp = buf;
	unsigned long tmp_len = len;
	while(tmp_len)
	{
		 UartPutChar(*tmp++);
		 tmp_len--;
	}
}

/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
unsigned long ak_uart_read(unsigned char* buf,unsigned long len)
{
	unsigned long  tmp_len = 0;
	unsigned short cmd_len;
	unsigned long i;
	unsigned long uart_len = 0;

	
	//if((0 != wakeup_type)&&(1 == UartIntFlg))
 	if((1 == UartIntFlg))     
	{
		for(i = 0; i < UartRxLen; i++)
		{
			if(ucUartBuff[i] == CMD_PREAMBLE)
			{
					uart_len = i;
					break;
			}
			
		}
		if(MAX_STRING_LENGTH - uart_len < sizeof(CMD_INFO))//没有足够的空间放有效命令
		{
			UartRxLen = 0;
			UartIntFlg = 0;
			return 0;
		}
		
		if((UartRxLen - uart_len > 3))
		{
			cmd_len  = ucUartBuff[uart_len+2];
			cmd_len |= 8 << ucUartBuff[uart_len+ 3];
			#ifdef UART_DEBUG
			Report("1=%x ",ucUartBuff[uart_len]);
			Report("id=%d ",ucUartBuff[uart_len+1]);
			Report("3=%d ",cmd_len);
			Report("len %d ",UartRxLen);
			Report("index len %d ",uart_len);
			#endif
			
			tmp_len = cmd_len + CMD_HEAD_LEN;		
			
			if((tmp_len == sizeof(CMD_INFO))&&(tmp_len <= UartRxLen -uart_len) )
			{
				memcpy(buf,(unsigned char*)(&ucUartBuff[uart_len]),tmp_len);
				UartRxLen = 0;
				UartIntFlg = 0;
			}
			else  //还没收完数据，中断有判断，这里加多一个判断条件
			{
				tmp_len = 0;
			}
			
		}		
		
	}
	return tmp_len;	
}


//*****************************************************************************
//
//! Interrupt handler for UART interupt 
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************
static void UARTIntHandler()
{

	unsigned long uiIntStat, i;
	long temp;
	
    uiIntStat = MAP_UARTIntStatus(UARTA1_BASE, 1);
    /* for receive timeout or fifo interrupt case */
    if(uiIntStat & 0x50)
	{
        MAP_UARTIntClear(UARTA1_BASE, 0x50);
        while(UartRxLen < MAX_STRING_LENGTH)
        {
    		temp = MAP_UARTCharGetNonBlocking(UARTA1_BASE);
    	    if(-1 != temp)
			{
	    		ucUartBuff[UartRxLen++] = (unsigned char)temp;
				
    	    }else
    	    {
				break;
    	    }
        }
		#ifdef UART_DEBUG
		Report("=%d",UartRxLen);
		for(i = 0;i < UartRxLen; i++)
		{
			Report(" %x",ucUartBuff[i]);
		}
		Report("\r\n");
		#endif
        if(temp != -1)
		{
			//MAP_UARTIntDisable(UARTA1_BASE, 0x50);
			UartIntFlg = 0;
			UartRxLen  = 0;
			Report("UART RX buffer over\r\n");
        }
		else
		{
			if(UartRxLen >= sizeof(CMD_INFO))
				UartIntFlg = 1;
		}
		
		
	
    }

}


/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
void send_cmd(CMD_INFO *cmd)
{
	CMD_INFO *pcmd = cmd;
	
	pcmd->preamble = CMD_PREAMBLE;
	
	pcmd->cmd_len = sizeof(pcmd->cmd_seq) + sizeof(pcmd->cmd_result) + sizeof(pcmd->param.event);
	#ifdef UART_DEBUG
	Report("=@=cmd_len %d ",pcmd->cmd_len);
	Report("id=%d,event=%d",pcmd->cmd_id,pcmd->param.event);
	#endif
	ak_uart_write((unsigned char*)pcmd, pcmd->cmd_len + CMD_HEAD_LEN);

}

const int isp_ae_exp_time[] = {
	1420, 224,
	1370, 448,
	1200, 672,
	1100, 896,	
	/*1150, 350,
	1100, 400,
	1050, 450,
	1000, 500,
	950, 600,
	850, 800,
	770, 896,	
	/*
	1570, 350,
	1500, 400,
	1430, 450,
	1360, 500,
	1290, 600,
	1220, 800,
	1050, 896,
	*/
};

void which_wakeup(void)
{
	int ad_val;
	int i;
	unsigned char wakeup_src = wakeup_type;
	CMD_INFO cmd ;
	#ifdef UART_DEBUG
	Report("==src %d\r\n",wakeup_src);
	#endif
	cmd.cmd_id = CMD_WAKEUP_SRC_RESP;
	
	ad_val = ad_average;
	for(i = 0; i < sizeof(isp_ae_exp_time)/sizeof(isp_ae_exp_time[0]); i += 2)
	{
		if (ad_val >= isp_ae_exp_time[i])
			break;
	}
	if (i >= sizeof(isp_ae_exp_time)/sizeof(isp_ae_exp_time[0]))
	{
		i -= 2;
	}

	cmd.cmd_result= isp_ae_exp_time[i + 1];
	Report("==ad: %d,exp: %d\r\n", ad_val, cmd.cmd_result);
	switch (wakeup_src)
	{
		case EVENT_RTC_WAKEUP:
		/*RTC wake up process*/
			cmd.param.event = EVENT_RTC_WAKEUP;		
			send_cmd(&cmd);
			break;
		case EVENT_PIR_WAKEUP:
			cmd.param.event = EVENT_PIR_WAKEUP;
			send_cmd(&cmd);
			break;
		case EVENT_RING_CALL_WAKEUP:
			cmd.param.event = EVENT_RING_CALL_WAKEUP;
			send_cmd(&cmd);			
			break;
		case EVENT_VIDEO_PREVIEW_WAKEUP:
			cmd.param.event = EVENT_VIDEO_PREVIEW_WAKEUP;
			send_cmd(&cmd);
			break;
		case EVENT_SYS_CONFIG_WAKEUP :
			cmd.param.event = EVENT_SYS_CONFIG_WAKEUP;
			send_cmd(&cmd);
			break;
		default:
			break;
	}
}
extern volatile unsigned char key_test_flg;

static int jk_read_one_cmd(CMD_INFO *cmd)
{
	//unsigned char preamble;
	char cmd_buf[13];
	cc_hndl uart_handle = NULL;
	int retval;
	uart_handle = uart_open(PRCM_UARTA1);
	if(NULL == uart_handle)
	{
		Report("read one cmd fail\r\n");
		return -1;
	}
	do
	{
		retval = uart_read(uart_handle, cmd_buf, 1);
	}while(0x55 == cmd_buf[0]);

	//get id and cmd length
	retval = uart_read(uart_handle, &cmd_buf[1], 3);
	if(-1 == retval)
	{
		Report("read cmd length error\r\n");
		return -1;
	}
	cmd->cmd_len =  (*(unsigned short*)(cmd_buf + 1));
	retval = uart_read(uart_handle, &cmd_buf[4], cmd->cmd_len);
	if(-1 == retval)
	{
		Report("read cmd data error\r\n");
		return -1;
	}
	return 0;
}

static void jk_uart1(void * pvParameters )
{

	unsigned char cmd_buf[MAX_STRING_LENGTH+1];
	unsigned long ret;
	CMD_INFO cmd;

	CMD_INFO cmd_test;

	Report("uartTaskRun.\r\n");

	while(1)
	{
		//ret = ak_uart_read(cmd_buf,MAX_STRING_LENGTH);
		ret = jk_read_one_cmd(&cmd);
		if(0 <= ret)
		{
			switch(cmd.cmd_id)
			{
				case CMD_WAKEUP_SRC_REQ:
					ak_start_flg = 1;
					gpio_spi_int = 0;
					which_wakeup();
					break;				
				case CMD_SLEEP_REQ:					
					power_ak(0);
					break;
					
				case CMD_BATTERY_LOW_ALARM:
					Report("ALARM_BATTERY_TOO_LOW\r\n");
					break;
					
				default:
					Report("NO CMD ID mach\r\n");
					break;
				

			}

		}
		else
		{
			Report("read CMD error\r\n");
		}

		SLEEP_MS(10);

		
		if((key_test_flg > 1)&& (ak_start_flg == 1))
		{
			cmd_test.cmd_id = CMD_VIDEO_PREVIEW_END;
			send_cmd(&cmd_test);
			key_test_flg = 0;
			Report("##==CMD_VIDEO_PREVIEW_END.\n\r");

		}
		
		
	}
}
/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
static void ak_uart1(void * pvParameters )
{

    unsigned char cmd_buf[MAX_STRING_LENGTH+1];
    unsigned long ret;
	CMD_INFO *cmd;
	
	CMD_INFO cmd_test;

	Report("uartTaskRun.\r\n");
	
	while(1)
    {
		ret = ak_uart_read(cmd_buf,MAX_STRING_LENGTH);
		if(0 != ret)
		{
			cmd = (CMD_INFO *)cmd_buf;
			
			switch(cmd->cmd_id)
			{
				case CMD_WAKEUP_SRC_REQ:
					ak_start_flg = 1;
					gpio_spi_int = 0;
					which_wakeup();
					break;				
				case CMD_SLEEP_REQ:					
					power_ak(0);
					break;
					
				case CMD_BATTERY_LOW_ALARM:
					Report("ALARM_BATTERY_TOO_LOW\r\n");
					break;
					
				default:
					Report("NO CMD ID mach\r\n");
					break;
				

			}

		}


		SLEEP_MS(10);

		
		if((key_test_flg > 1)&& (ak_start_flg == 1))
		{
			cmd_test.cmd_id = CMD_VIDEO_PREVIEW_END;
			send_cmd(&cmd_test);
			key_test_flg = 0;
			Report("##==CMD_VIDEO_PREVIEW_END.\n\r");

		}
		
		
    }
	
}


/**
* @brief 
* @author 
* @date 2016-12-2
* @param 
* @return void
*/
void ak_uart1_init(void)
{
	Report("ak_uart1_init\r\n ");

	UartIntFlg   = 0;
	//
    // Enable Peripheral Clocks 
    //
	MAP_PRCMPeripheralClkEnable(PRCM_UARTA1, PRCM_RUN_MODE_CLK);

    //
    // Configure PIN_16 for UART1 UART1_TX
    //
    MAP_PinTypeUART(UART1_TX_PIN, UART1_TX_MOD);

    //
    // Configure PIN_17 for UART1 UART1_RX
    //
    MAP_PinTypeUART(UART1_RX_PIN, UART1_RX_MOD);

    // Reset the peripheral
    MAP_PRCMPeripheralReset(PRCM_UARTA1);

     //
    // Register interrupt handler for UART
    //
    MAP_UARTIntRegister(UARTA1_BASE,UARTIntHandler);

    //
    // Enable RX |time out done interrupts for uart
    //    
    //MAP_UARTIntEnable(UARTA1_BASE,UART_INT_RX|UART_INT_RT); 

    MAP_UARTConfigSetExpClk(UARTA1_BASE,MAP_PRCMPeripheralClockGet(PRCM_UARTA1),
        UART1_PIPE_BAUD,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));


    MAP_UARTFIFOLevelSet(UARTA1_BASE,UART_FIFO_TX1_8,UART_FIFO_RX4_8);

	MAP_UARTIntClear(UARTA1_BASE, UART_INT_RX|UART_INT_RT);
	MAP_UARTIntEnable(UARTA1_BASE, UART_INT_RX|UART_INT_RT);

/*	
	if(osi_LockObjCreate(&uart_rx_mutex) != 0)
	{
		Report("uart_rx_mutex creak fail\r\n ");
		while(1);
	}
*/

	//TASK_UART1_FR_AK39E_PRIORITY
   // osi_TaskCreate(ak_uart1,(signed char *)"ak_uart1",1024, (void *)NULL, 2, (void *)NULL);

}

void star_uart1_task()
{
	long lRetVal = -1;
	
    lRetVal = osi_TaskCreate(ak_uart1,(signed char *)"ak_uart1",1024, (void *)NULL, 2, (void *)NULL);
	if(lRetVal < 0)
	{
		UART_PRINT("Failed to creat uart task !\r\n");
		return ;
	}
}
