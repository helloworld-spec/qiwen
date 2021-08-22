/**
 * @file Arch_uart.h
 * @brief UART driver header file
 *
 * This file provides UART APIs: UART initialization, write data to UART, read data from
 * UART, register callback function to handle data from UART, and interrupt handler.
 * Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liaozhijun
 * @date 2005-07-14
 * @version 1.0
 */

#ifndef __ARCH_UART_H__
#define __ARCH_UART_H__




/** @defgroup UART UART group
 *      ingroup Drv_Lib
 */
/*@{*/

#include "anyka_types.h"

/** @name UART Baudrate Define
 *  Define UART support baudrate here
 */
/*@{*/
#define UART_BAUD_9600          9600
#define UART_BAUD_19200         19200
#define UART_BAUD_38400         38400
#define UART_BAUD_57600         57600
#define UART_BAUD_115200        115200
#define UART_BAUD_460800        460800

/* @} */


/**
 * @brief UART port define
 *  define port name with port number
 */
typedef enum
{
    uiUART0 = 0,
    uiUART1,
    MAX_UART_NUM        /* UART number */
} T_UART_ID;

/**
 * @brief UART callback define
 *  define UART callback type
 */
typedef unsigned char (*T_fUART_CALLBACK)(void);


/**
 * @brief Initialize UART
 *
 * Initialize UART base on UART ID, baudrate and system clock. If user want to change
 * baudrate or system clock is changed, user should call this function to initialize
 * UART again.
 * Function uart_init() must be called before call any other UART functions
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[in] baud_rate Baud rate, use UART_BAUD_9600, UART_BAUD_19200 ...
 * @param[in] sys_clk system clock
 * @return bool Init UART OK or not
 * @retval true Successfully initialized UART
 * @retval false Initializing UART failed.
 */
bool uart_init(T_UART_ID uart_id, unsigned long baud_rate, unsigned long sys_clk);

/**
 * @brief Close UART
 *
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @return void
 */
void uart_free(T_UART_ID uart_id);

/**
 * @brief set pool to receive data, must call before uart_set_callback
 * 
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[in] *pool buffer to recieve data,as big as you can
 * @param[in] poollength buffer length
 * @return void
 * @remarks the data received from uart will be stored in the pool waiting to be fetched by
 *  uart_read().
 */
void uart_set_datapool(T_UART_ID uart_id, unsigned char *pool, unsigned long poollength);

/**
 * @brief Register a callback function to process UART received data.
 * 
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[in] callback_func Callback function
 * @return void
 */
void uart_set_callback(T_UART_ID uart_id, T_fUART_CALLBACK callback_func);

/**
 * @brief enable or disable uart flow control
 *
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id which uart to be set
 * @param[in] enable true to enable flow control
 * @return bool  if set successfully, return true.
                   failed or not supported, return false.
 */
bool uart_setflowcontrol(T_UART_ID uart_id, bool enable);

/**
 * @brief change UART setting according to system clock change
 *
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] sys_clk system clock
 * @return void
 */
void uart_on_change( unsigned long sys_clk );

/**
 * @brief Write string data to UART
 *
 * Write data to UART base on UART ID and data length
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[in] data Constant data pointer to be written to UART, this data needn't be ended with '\\0'
 * @param[in] data_len Data length
 * @return unsigned long
 * @retval Length of the data which have been written to UART
 */
unsigned long uart_write(T_UART_ID uart_id, const unsigned char *data, unsigned long data_len);

/**
 * @brief read data from uart pool
 * 
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[out] *data buffer to recieve data,as big as you can
 * @param[in] datalen data length to read
 * @return unsigned long
 * @retval the data length that has been read
 */
unsigned long uart_read(T_UART_ID uart_id, unsigned char *data, unsigned long datalen);

/**
 * @brief Read a character from UART
 *
 * This function will not return until get a character from UART
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[out] *chr character for return
 * @return bool Got character or not
 * @retval return true
 */
bool uart_read_chr(T_UART_ID uart_id, unsigned char *chr);

/**
 * @brief set uart data parity mode, use even parity or odd parity or neither
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[in] enable bool, enable data parity or not
 * @param[in] evenParity bool, if true enable even parity,else enable odd parity
 * @return bool
 * @retval if success, return true
 */
bool uart_setdataparity(T_UART_ID uart_id, bool enable, bool evenParity);

/**
 * @brief change uart baudrate
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[in] baud_rate unsigned long new baud_rate to change
 * @return bool
 * @retval if success, return true
 */
bool uart_setbaudrate(T_UART_ID uart_id, unsigned long baud_rate);

/**
 * @brief clear uart l2 buf
 * @author yang_mengxia
 * @date 2017-4-13
 * @param[in] uart_id UART ID
 * @return void
 */
void uart_clr_l2_buf(T_UART_ID uart_id);

/**
 * @brief clear uart datapoop
 * @author liao_zhijun
 * @date 2017-4-24
 * @param[in] uart_id UART ID
 * @return void
 */
void uart_clear_datapool(T_UART_ID uart_id);

/**
 * @brief start uart send and return, donot wait for complete
 * @author
 * @date
 * @param uart_id: which uart to send
 * @param data: data buffer
 * @param data_len: data length
 * @return bool
 */
bool uart_send(T_UART_ID uart_id, const unsigned char *data, unsigned long data_len);

/*@}*/

#endif

