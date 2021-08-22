/**
 * @file  hal_print.h
 * @brief Define structures and functions of print.c
 * 
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2006-01-16
 * @version 1.0
 */
#ifndef _PRINT_H_
#define _PRINT_H_

#include "arch_uart.h"

/** @defgroup Debug_Print Print group
 *ingroup Drv_Lib
 */
/*@{*/

typedef enum{
    CONSOLE_UART=0,         /* print via uart */
    CONSOLE_USB,            /* print via usb */
    CONSOLE_NULL            /* disable print */
}T_CONSOLE_TYPE;

#define M_DRVSYS            "DRVLIB" /* module name */

#define C1 1    /*Fatal error message*/
#define C2 2    /*Error message*/
#define C3 3    /*Common message*/

typedef signed long (* T_fPRINT_CALLBACK)(unsigned char level, const signed char * mStr, const signed char * s, ...);

/**
 * @brief get drvlib version
 * @return const signed char *
 */
const char * drvlib_version(void);

/**
 * @brief  debug set callback
 * @author	
 * @date
 * @param[in] func callback function of console, can be a null one
 * @return  void
 */
void console_setcallback(T_fPRINT_CALLBACK func);

/**
 * @brief set forbidden level
 * @author
 * @date
 * @param[in] level new forbidden level
 * @return void
 */
void console_setlevel(unsigned char level);

/**
 * @brief debug console init
 * @author	
 * @date
 * @param[in] type type of console, refer to T_CONSOLE_TYPE definition
 * @param[in] baudrate uart baudrate, refer to Arch_uart.h definition
 * @param[in] T_UART_ID,uart id
 * @return  void
 */
void console_init(T_UART_ID uart_id, T_CONSOLE_TYPE type, unsigned long baudrate);

/**
 * @brief release current console
 * @author	
 * @date
 * @return  void
 */
void console_free(void);

/**
 * @brief get a charactor from uart
 * @author	
 * @date
 * @return  signed char
 * @retval  the charactor that was gotten
 */
signed char   getch(void);

/**
 * @brief get string
 * @param signed char *buf, the buffer for the string
 * @param signed long n, the length of the string
 * @retval void
 */
void  ak_gets(signed char * buf, signed long n);

/**
 * @brief get integer in decimal style
 * @author	
 * @date
 * @param[in] def default value
 * @return unsigned long
 * @retval integer that was gotten
 */
unsigned long   getul10(unsigned long def);

/**
 * @brief get integer in hex style
 * @author	
 * @date
 * @param[in] def default value
 * @return unsigned long
 * @retval integer that was gotten
 */
unsigned long   getul(unsigned long def);

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
signed long akprintf(unsigned char level, const char * mStr, const char * s, ...);

/**
 * @brief send console data from buffer
 * @author
 * @date
 * @param void
 * @return bool
 */
bool console_send_buffer(void);

/*@}*/
#endif /* _PRINT_H_ */
