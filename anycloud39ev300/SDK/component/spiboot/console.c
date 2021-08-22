/**
 * @file console.c
 * @brief Uart driver C file.
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-11-15
 * @version 1.0
 * @ref AK37XX technical manual.
 */

#include <stdarg.h>

#include "anyka_types.h"
#include "console.h"

T_S32 console_init(T_U32 baudrate)
{
	*(volatile T_U32*)(0x0800001c) &= ~(0x1 << 7);
	*(volatile T_U32*)(0x08000074) |= (0x3 << 2); // cdh:bit[2:1]
	*(volatile T_U32*)(0x08000080) |= (0x3 << 1); // cdh:bit[2:1]

	*(volatile T_U32 *)UART0_CONF_REG = (UART_CLEAR_RX_STAT|UART_CLEAR_TX_STAT
					|UART_ENABLE|baudrate);
	
	*(volatile T_U32*)(0x20130018) = ((0x1F << 16)|(0x1 << 0));
	return 0;
}

T_S32 console_write(char ch)
{
	*(volatile T_U32*)(0x2014008c) |= (0x1<<16); //Clear the uart tx buf
	*(volatile T_U32*)(0x48001000) = (T_U32)ch; 
	*(volatile T_U32*)(0x48001000 + 60) = (T_U32)'\0'; 
	*(volatile T_U32*)(UART0_CONF_REG) |= (0x1 << 28);
	*(volatile T_U32*)(UART0_STATUS_REG) |= ((0x1 << 4)|(0x1 << 16));
	
	while((*(volatile T_U32*)(UART0_STATUS_REG) & (0x1 << 19)) == 0)
		;
	return 0;
}

T_S32 putch(char ch)
{
	return console_write(ch);
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

