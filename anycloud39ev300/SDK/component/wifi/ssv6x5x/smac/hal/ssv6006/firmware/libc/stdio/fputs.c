#include "hal.h"
#include <stdio.h>

#define USE_SERIAL_DRIVER

#ifdef USE_SERIAL_DRIVER
#include <serial/serial.h>

__attribute__((used))
int fputc(int c, FILE *stream)
{
	//if (c == '\n')
	//    xSerialPutChar('\r');

	xSerialPutChar(NULL, c, 1);

	return c;
}

__attribute__((used))
int fputs(const char *s, FILE *stream)
{
    vSerialPutString(NULL, (const signed char *)s, strlen(s));

	return 0;
}
#else
#include "uart/uart.h"

__attribute__((used))
int fputc(int c, FILE *stream)
{
    if (c == '\n')
        drv_uart_put_char('\r');

    drv_uart_put_char(c);

    return c;
}

__attribute__((used))
int fputs(const char *s, FILE *stream)
{
    while (fputc(*s++, stream))
        ;

    return 0;
}
#endif

__attribute__((used))
int putc(int c, FILE *stream)
{
	return fputc(c, stream);
}

__attribute__((used))
int putchar(int c)
{
	return fputc(c, (void*)0x10);
}

__attribute__((used))
int puts(const char *s)
{
	return fputs(s, (void*)0x10);
}
