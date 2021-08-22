/**
 * @file print.c
 * @brief print function file, provide functions to print infomation
 *
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2005-07-24
 * @version 1.0
 */
#include <stdlib.h>
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "arch_uart.h"
#include "stdarg.h"
#include "drv_api.h"
#include "drv_module.h"
#include "drv_timer.h"

#define CONSOLE_USE_BUFFER

#define KEY_BACKSPACE           0x08
#define KEY_ENTER               0x0d
#define KEY_ESC                 0x1b

#define CR                      0x0D //\r
#define LF                      0x0A //\n

#define DBG_TRACE_LOGLEN        2048
#define CONSOLE_RECV_BUFLEN     2048
#define CONSOLE_SEND_BUFLEN     4096
#define CONSOLE_SEND_MASK       (CONSOLE_SEND_BUFLEN - 1)
#define MODULE_NAME_MAX_LEN     8

#define CONSOLE_RECV_EVENT      (1<<1)

/* default to enable print */
static T_CONSOLE_TYPE console_type=CONSOLE_UART;
static T_fPRINT_CALLBACK printf_callback = NULL;
static unsigned char m_console_level=C3;
static T_UART_ID m_uart_id = uiUART0;

int puts(const char *s);
signed long putch( signed char ch);

static char printf_buf[DBG_TRACE_LOGLEN];
static char s_recv_buf[CONSOLE_RECV_BUFLEN];
static char s_send_buf[CONSOLE_SEND_BUFLEN];

static volatile int s_send_head;
static volatile int s_send_datlen;

static int console_write(char *data, int len)
{
#ifdef CONSOLE_USE_BUFFER
    int i, wpos;

    //loop to put data into buffer
    for(i = 0; (i < len) && (s_send_datlen < CONSOLE_SEND_BUFLEN); i++, s_send_datlen++)
    {
        wpos = (s_send_head + s_send_datlen) & CONSOLE_SEND_MASK;
        s_send_buf[wpos] = data[i];
    }

    return i;

#else
    int i;
    
    if (console_type == CONSOLE_UART)
    {
        for(i = 0; i < len; i++)
            uart_write_chr(m_uart_id, data[i]);

        return len;
    }
    else if (console_type == CONSOLE_USB)   
    {
        usbdebug_printf(data, len);       
        return len;
    }
    else
    {
        return 0;
    }

#endif

}


/**
 * @brief send console data from buffer
 * @author
 * @date
 * @param void
 * @return bool
 */
bool console_send_buffer(void)
{
    int send_len = s_send_datlen;
    bool ret = false;
    
    //return if buffer is empty
    if(s_send_datlen <= 0)
        return ret;

    if(s_send_head + send_len > CONSOLE_SEND_BUFLEN)
    {
        send_len = CONSOLE_SEND_BUFLEN - s_send_head;
    }

    //send data
    if (console_type == CONSOLE_UART)
    {
        //set the single uart output size to 60
        if(send_len > 60) send_len = 60;
        
        ret = uart_send(m_uart_id, s_send_buf+s_send_head, send_len);
    }
    else if (console_type == CONSOLE_USB)
    {
        usbdebug_printf(s_send_buf+s_send_head, send_len);
        ret = true;
    }
    
    if(ret)
    {
        //store_all_int();
        s_send_head = (s_send_head + send_len) & CONSOLE_SEND_MASK;
        s_send_datlen -= send_len;
        //restore_all_int();
    }

    return ret;
}

/* return drvlib version */
const char * drvlib_version(void)
{
    return _DRV_LIB_VER;
}

/**
 * @brief  debug set callback
 * @author	
 * @date
 * @param[in] func callback function of console, can be a null one
 * @return  void
 */
void console_setcallback(T_fPRINT_CALLBACK func)
{
    printf_callback = func;
}

/**
 * @brief set forbidden level
 * @author
 * @date
 * @param[in] level new forbidden level
 * @return void
 */
void console_setlevel(unsigned char level)
{
    //set new level
    m_console_level = level;
}

extern int vsnprintf( char *buffer, size_t count, const char *format, va_list argptr );

/**
 * @brief anyka specific printf
 * @author
 * @date
 * @param[in] level forbidden level
 * @param[in] mStr module string
 * @param[in] s format string
 * @return signed long
 * @retval 0 is print ok, -1 is forbidden to print
 */
signed long akprintf(unsigned char level, const char * mStr, const char * s, ...)
{
    va_list     args;
    int slen,i;

    if (m_console_level < level)
        return -1;
        
    va_start(args, s);
    vsnprintf(printf_buf, DBG_TRACE_LOGLEN, (const char*)s, args);   
    //puts(printf_buf);
    va_end(args); 

    slen = strlen(printf_buf);
    for(i = 0; i < slen; i++)
    {
        if (printf_buf[i] == '\n')
            putch('\r');
        putch(printf_buf[i]);
    }

    return 0;
}

//console uart interrupt callback
static unsigned char console_interrupt_callback(void)
{
    E_DRV_MODULE uart_module;
    
    uart_module = (m_uart_id == uiUART1) ? DRV_MODULE_UART1 : DRV_MODULE_UART0;

    //set event
    DrvModule_SetEvent(uart_module, CONSOLE_RECV_EVENT);
    return 0;
}

static void console_send_timer_callback(signed long timer_id, unsigned long delay)
{
    console_send_buffer();
}

/**
 * @brief debug console init
 * @author	
 * @date
 * @param[in] type type of console, refer to T_CONSOLE_TYPE definition
 * @param[in] uart baudrate, refer to Arch_uart.h definition
 * @param[in] unsigned char,uart id:uiUART0--uiUART3
 * @return  void
 */
void console_init(T_UART_ID uart_id, T_CONSOLE_TYPE type, unsigned long baudrate)
{
    printf_callback = NULL;
    
    if (type == CONSOLE_UART)
    {
        if (MAX_UART_NUM > uart_id)
        {
            m_uart_id = uart_id;
        }

        uart_init(m_uart_id, baudrate, get_asic_freq()); 

        //set datapool and callback
        uart_set_datapool(m_uart_id, s_recv_buf, CONSOLE_RECV_BUFLEN);
        uart_set_callback(m_uart_id, console_interrupt_callback);

#ifdef CONSOLE_USE_BUFFER
        timer_start(uiTIMER2, 10, true, console_send_timer_callback);
#endif    

    }
    else if (type == CONSOLE_USB)
    {
        usbdebug_enable();
    }
    else
    {
        
    }
    
    console_type = type;
    
    //show driver lib version
    akprintf(C3, M_DRVSYS, "\n_DRV_LIB_VER=%s\n", _DRV_LIB_VER);    
}

void console_free(void)
{
    if (console_type == CONSOLE_UART)
    {
        uart_free(m_uart_id);
    }
    else if (console_type == CONSOLE_USB)
    {
        usbdebug_disable();
    }
    else
    {
        
    }
    
    console_type = CONSOLE_NULL;

}

signed char getch(void)
{
    signed char c=0;

    if (console_type == CONSOLE_UART)
    {
        while(uart_read(m_uart_id, &c, 1) < 1)
        {
            E_DRV_MODULE uart_module;

            //wait event
            uart_module = (m_uart_id == uiUART1) ? DRV_MODULE_UART1 : DRV_MODULE_UART0;
            DrvModule_WaitEvent(uart_module, CONSOLE_RECV_EVENT, 0xFFFFFFFF);
        }
    }
    else if (console_type == CONSOLE_USB)
    {
        
        while(usbdebug_getstring((unsigned char *)&c, 1) == 0);
        return c;
    }
    else
    {
        puts("not support input, dead!!\n");\
        while(1);
        return 0;
    }
    
    return c;
}

signed long putch( signed char ch)
{
    console_write(&ch, 1);
    return 1;
}

int puts(const char *s)
{
    unsigned long len = 0;

    len = strlen(s);
    console_write((char *)s, len);
    return len;
}

void ak_gets (signed char * buf, signed long n)
{
    int i;
	
	//clear uart rx buf before get string
    uart_clear_datapool(m_uart_id);

    for(i = 0;i< n; i ++)
        {
            buf[i] = (char )getch();
			//putch(buf[i]);
            if (buf[i] == CR){
                putch(CR);
                putch(LF);
                break;
            }
            else
            {
                 if(buf[i] == KEY_BACKSPACE)
                 {
                    if (i >= 1)
                        buf[i-1] = 0;
                    else
                    {
                        buf[i]= 0;
                        i --;
                        continue;
                    }

                    putch(KEY_BACKSPACE);
                    putch(' ');
                    putch(KEY_BACKSPACE);
                    i -= 2;
                    continue;
                 }
                 else
                    putch(buf[i]);
            }
        }

        if (i == n)
        {
            putch(CR);
            putch(LF);
        }

        buf[i] = '\0';
        return;
}

unsigned long getul10(unsigned long def)
{
    char buf[11];
    char *p = buf;

    ak_gets((signed char *)buf,11);

    if (buf[0] == '\0')
        return def;

    buf[10] = '\0';
    if ((buf[0]== '0' ) && ((buf[1]== 'x')||(buf[1] == 'X')) )
        p = &buf[2];

    return strtoul(p, 0, 10);
}

unsigned long getul(unsigned long def)
{
    char buf[11];
    char *p = buf;

    ak_gets((signed char *)buf,11);

    if (buf[0] == '\0')
        return def;

    buf[10] = '\0';
    if ((buf[0]== '0' ) && ((buf[1]== 'x')||(buf[1] == 'X')) ) // hex
    {
        p = &buf[2];
        return strtoul(p, 0, 16);
    }
    else    //decimal
    {
        return strtoul(p, 0, 10);
    }
}


