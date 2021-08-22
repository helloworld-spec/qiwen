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

#ifndef __CONSOLE_H
#define __CONSOLE_H

#define CR			0x0D
#define LF			0x0A

#define KEY_BACKSPACE		0x08
#define KEY_ENTER			0x0d
#define KEY_ESC				0x1b

#define	UART0_CONF_REG			0x20130000
#define	UART0_STATUS_REG		0x20130004
#define	UART0_COUNT_REG			0x20130008
#define	UART0_THRESHOLD_REG		0x2013000C

#define	UART1_CONF_REG			0x20138000
#define	UART1_STATUS_REG		0x20138004
#define	UART1_COUNT_REG			0x20138008
#define	UART1_THRESHOLD_REG		0x2013800C


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


T_S32 console_init(T_U32 baudrate);
T_S8 console_read(T_VOID);



T_S32 putch(char ch);
T_S32 puts(const char *s);
T_VOID print_x(T_U32 l);
T_S8 getch(T_VOID);
T_U32 get_uart_dat(T_U32 *buf, T_U32 count, T_U32 always_wait);
T_U32 get_one_dat(T_U32 *rece_val);
T_U32 getul(T_U32 default_val, T_U32 *return_val);

#endif /* __CONSOLE_H */
