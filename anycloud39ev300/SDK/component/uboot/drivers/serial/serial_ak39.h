/**
 * @file console.h
 * @brief Uart driver header file, define Uart register.
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-11-15
 * @version 1.0
 * @ref AK37XX technical manual.
 */

#ifndef _SERIAL_AK39_H_
#define _SERIAL_AK39_H_

#define CR			0x0D
#define LF			0x0A

#define KEY_BACKSPACE		0x08
#define KEY_ENTER			0x0d
#define KEY_ESC				0x1b

enum ak39_uart_idx
{
	ak39_uart0,
	ak39_uart1,
};

#define UART0_BASE_ADDR 		(0x20130000)
#define UART1_BASE_ADDR 		(0x20138000)

#define	UART_CONF_REG			0x000
#define	UART_STATUS_REG			0x004
#define	UART_COUNT_REG			0x008
#define	UART_THRESHOLD_REG		0x00C
#define	UART_RX_BUF_REG 		0x010
#define	UART_TIMEOUT_REG 		0x018

//baurate: 38400[0~15], rts& is 1[20],  uart enable[21], not receive timeout[23], 
//not even odd check[26], little endian[27], tx rx clear state[28~29]
#define	UART_CONF_DEFAULT_VAL				0x30300619
#define	UART_BAUD_REATE_115200				0x208
#define	UART_BAUD_REATE_38400				0x619
#define	UART_BAUD_REATE_38400_ASPEN2		0x64d   // Aspen boot up sequency is 62MHZ
#define	UART_ENABLE							(0x1<<21)
#define	UART_DIV_ADJ_ENA					(0x1<<22)
#define	UART_TIMEOUT_ENA					(0x1<<23)
#define	UART_CHECK_EVEN_ODD_ENA	   		    (0x1<<26)
#define	UART_BIG_ENDIAN_ENA	   	   		    (0x1<<27)
#define	UART_CLEAR_TX_STAT	   	   		    (0x1<<28)
#define	UART_CLEAR_RX_STAT	   	   		    (0x1<<29)

#define	UART_THRESHOLD_RX_INT				(0x1<<29)
#define	UART_THRESHOLD_RX_INT_ASPEN2		(0x1<<30)

#define UART_ALWAYS_WAIT_ENABLE				(1)
#define UART_ALWAYS_WAIT_DISABLE			(0)
#define	UART_TIMEOUT_STAT					(0x1<<2)

#endif /* __CONSOLE_H */
