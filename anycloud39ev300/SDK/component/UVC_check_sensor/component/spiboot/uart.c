/**
 * @file uart.c
 * @brief UART driver, define UARTs APIs.
 * This file provides UART APIs: UART initialization, write data to UART, read data from
 * UART, register callback function to handle data from UART, and interrupt handler.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liao_zhijun
 * @date 2010-07-28
 * @version 1.0
 */
#include <string.h>
#include "anyka_cpu.h"
#include "arch_uart.h"
#include "uart.h"
#include <stdarg.h>
#include "l2.h"



//********************************************************************
#define UART_TX_END_STA                 (1<<19)
#define REG_04_MSK_BIT                  (0x3fe00000)

#define TX_STATUS           1
#define RX_STATUS           0

//REG_UART_CONFIG_1
#define TX_ADR_CLR                      (1UL << 31)
#define RX_ADR_CLR                      (1 << 30)
#define RX_STA_CLR                      (1 << 29)
#define TX_STA_CLR                      (1 << 28)
#define ENDIAN_SEL_BIG                  (1 << 27)
#define RX_TIMEOUT_EN                   (1 << 23)
#define BAUD_RATE_ADJ_EN                (1 << 22)
#define UART_INTERFACE_EN               (1 << 21)

#define TX_BYT_CNT_VLD                  (1 << 16)
#define TX_BYT_CNT                      (4)         //[15:4]
/*
 * @brief: Get Uart's base register address
 */
#define  uart_id2register(uart_id)  (UART0_BASE_ADDR+(T_U32)(uart_id)*0x8000)
    
T_VOID uart_gpio_set(T_UART_ID uart_id)
{
    T_U32 reg_value1 = 0;
    T_U32 reg_value2 = 0;
    T_U32 reg_valuepd = 0;
	
    reg_value1 = REG32(GPIO_SHAREPIN_CONTROL1);
    if (uiUART0 == uart_id)
    {   
        reg_value1 &= ~(3<<1);
        reg_value1 |= (3<<1);
    }
    else if (uiUART1 == uart_id)
    {
        reg_value1 &= ~(0xF<<4);
        reg_value1 |= (0xA<<25); 
    }
     
    outl(reg_value1, GPIO_SHAREPIN_CONTROL1);
}

T_VOID uart_clock_set(T_UART_ID uart_id)
{
    T_U32 reg_value;
    T_U8 bit = 7;
    bit = (T_U8)uart_id + bit;

    REG32(CLOCK_CTRL_REG) &= ~(1<<bit);
}

/**
 * @brief Initialize UART
 * Initialize UART base on UART ID, baudrate and system clock. If user want to change
 * baudrate or system clock is changed, user should call this function to initialize
 * UART again.
 * Function uart_init() must be called before call any other UART functions
 * @author Liao_Zhijun
 * @date 2010-07-23
 * @param T_UART_ID uart_id: UART ID
 * @param T_U32 baud_rate: Baud rate, use UART_BAUD_9600, UART_BAUD_19200 ...
 * @param T_U32 sys_clk: system clock
 * @return T_BOOL: Init UART OK or not
 * @retval AK_TRUE: Successfully initialized UART.
 * @retval AK_FALSE: Initializing UART failed.
 */
T_BOOL uart_init(T_UART_ID uart_id, T_U32 baud_rate, T_U32 sys_clk)
{
    T_U32 br_value;
    T_U32 reg_value;
    T_U32 base_addr; 
    
	base_addr = uart_id2register(uart_id);

    uart_gpio_set(uart_id);
    uart_clock_set(uart_id);
    
    br_value = ((sys_clk<<2)+baud_rate)/(baud_rate<<1);

	if(br_value%2)
		reg_value = ((br_value/2-1)&0xffff)|(1<<21)|(1<<22)|(1<<23)|(1<<28)|(1<<29);
	else
		reg_value = ((br_value/2-1)&0xffff)|(1<<21)|(1<<23)|(1<<28)|(1<<29);	


	outl(reg_value, base_addr+0x00);     
    
    return AK_TRUE;
}

static T_VOID ClearStatus(T_UART_ID uart_id, T_U8 txrx_status)
{
    T_U32 base_addr;
    T_U32 reg_value;

    base_addr = uart_id2register(uart_id);
    reg_value = inl(base_addr+0x00);
    if (txrx_status)
        reg_value |= TX_STA_CLR;
    else
        reg_value |= RX_STA_CLR;
    outl(reg_value, (base_addr+0x00));  
}

static T_BOOL uart_write_cpu(T_UART_ID uart_id, T_U8 *chr, T_U32 byte_nbr)
{
    T_U32 baseAddress;
    T_U32 status;
    T_U32 reg_value;
    T_U8  buf_id = 6 +(uart_id<<1);
    T_U32 buf_addr, i,cnt=0;
    T_U32 value1,value2;
    
    //get base address
    baseAddress = uart_id2register(uart_id);

    //clear tx status
    ClearStatus(uart_id, TX_STATUS); //clear tx status

    //load data to l2
    l2_uartbuf_cpu((T_U32)chr, uart_id, byte_nbr, MEM2BUF);

    //start to trans
    reg_value = REG32(baseAddress+UART_CFG_REG2);
    reg_value &= REG_04_MSK_BIT;
    reg_value |= (byte_nbr<< TX_BYT_CNT)|TX_BYT_CNT_VLD; 
    REG32(baseAddress + UART_CFG_REG2) = reg_value;

    //wait for tx end
    while (1)
    {
        status = inl(baseAddress + UART_CFG_REG2);
        
        if (status & UART_TX_END_STA)
            break;
    }
        
    return AK_TRUE;
}


T_S32 putch(char ch)
{
	return uart_write_cpu(chip_config.uart_ID,&ch,1);
}

T_S32 puts(const char *s)
{
	while (*s != 0) {
		putch(*s++);
	}
	return 0;
}

void print_0nx(char ch2, unsigned long l)
{
	unsigned char ch;
	int i;

	ch2 = ch2 - '0';
	for (i = ch2 - 1; i >= 0; i--) {
		ch = (l >> (i * 4)) & 0x0f;
		if (ch < 10)
			putch(ch + '0');
		else
			putch(ch - 10 + 'a');
	}
}

void print_d(unsigned long l)
{
	
	unsigned long  t;
	unsigned char ch;
	int i, j, k ;
	char buf[8];
	
	if (l == 0)
	{
		putch('0');
		return;	
	}
	
	memset(buf,'0',8);
	t = l;
	k = 7;

	for(;;)
	{
		if (t > 9)
		{
			i = t % 10;
		}else
			i = t;
	
		if (i < 10){
			buf[k--] = i + '0';	
		}	

		if (t < 9)
			break;
		t /= 10;
	}

	j = 0;
	for(i =0 ;i < 8 ; i ++)
	{
		if(buf[i] != '0')
			j = 1;
		if (j)
			putch(buf[i]);
	}
	
 }

void print_x(unsigned long l)
{
	unsigned long  t;
	unsigned char ch;
	int i, j, k ;
	char buf[8];
	
	memset(buf,'0',8);
	t = l;
	k = 7;

	for(;;)
	{
		if (t > 15)
		{
			i = t % 16;
		}else
			i = t;
	
		if (i < 10){
			buf[k--] = i + '0';	
		}	
		else
		    if(i < 16)
			{
		       buf[k--] = i -10 +'a';
			}

		if (t < 15)
			break;
		t >>= 4;
	}

	for(i =0 ;i < 8 ; i ++)
		putch(buf[i]);
}

int printf(const char *s, ...)
{
	va_list ap;
	unsigned long arg;
	const char *str;
	char ch1, ch2, ch3;

	va_start(ap, s);
	while (*s != 0) {
		if (*s == '%') {
			s++;
			ch1 = *s++;
			if (ch1 == 's') {
				str = va_arg(ap, unsigned char *);
				puts(str);
			}else if (ch1 == 'd') {
				arg = va_arg(ap, unsigned long);
				print_d(arg);
			}
			 else if (ch1 == 'x') {
				arg = va_arg(ap, unsigned long);
				print_x(arg);
			}else {
				ch2 = *s++;
				ch3 = *s++;
				arg = va_arg(ap, unsigned long);
				print_0nx(ch2, arg);
			}
		} else
			putch(*s++);
	}
	va_end(ap);

	return 0;
}
#if 0
static T_CHR printf_buf[2048];

/**
 * @brief anyka specific printf
 * @author
 * @date
 * @param[in] level forbidden level
 * @param[in] mStr module string
 * @param[in] s format string
 * @return T_S32
 * @retval 0 is print ok, -1 is forbidden to print
 */
T_S32 akprintf(T_U8 level, T_pCSTR mStr, T_pCSTR s, ...)
{


    va_list     args;
        
    va_start(args, s);
    vsnprintf(printf_buf, 2048, (const char*)s, args);   
    puts(printf_buf);
    va_end(args); 


    return 0;
}
#endif



