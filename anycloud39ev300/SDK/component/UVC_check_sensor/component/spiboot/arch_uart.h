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

typedef struct
{
    T_U32 chip_type;
    T_U32 uart_ID;
}T_CHIP_CONFIG_INFO;

extern T_CHIP_CONFIG_INFO chip_config;

/**
 * @brief UART callback define
 *  define UART callback type
 */
typedef T_U8 (*T_fUART_CALLBACK)(T_VOID);


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
 * @return T_BOOL Init UART OK or not
 * @retval AK_TRUE Successfully initialized UART
 * @retval AK_FALSE Initializing UART failed.
 */
T_BOOL uart_init(T_UART_ID uart_id, T_U32 baud_rate, T_U32 sys_clk);

/**
 * @brief Close UART
 *
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @return T_VOID
 */
T_VOID uart_free(T_UART_ID uart_id);

/**
 * @brief set pool to receive data, must call before uart_set_callback
 * 
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[in] *pool buffer to recieve data,as big as you can
 * @param[in] poollength buffer length
 * @return T_VOID
 * @remarks the data received from uart will be stored in the pool waiting to be fetched by
 *  uart_read().
 */
T_VOID uart_set_datapool(T_UART_ID uart_id, T_U8 *pool, T_U32 poollength);

/**
 * @brief Register a callback function to process UART received data.
 * 
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[in] callback_func Callback function
 * @return T_VOID
 */
T_VOID uart_set_callback(T_UART_ID uart_id, T_fUART_CALLBACK callback_func);

/**
 * @brief enable or disable uart flow control
 *
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id which uart to be set
 * @param[in] enable AK_TRUE to enable flow control
 * @return T_BOOL  if set successfully, return AK_TRUE.
                   failed or not supported, return AK_FALSE.
 */
T_BOOL uart_setflowcontrol(T_UART_ID uart_id, T_BOOL enable);

/**
 * @brief change UART setting according to system clock change
 *
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] sys_clk system clock
 * @return T_VOID
 */
T_VOID uart_on_change( T_U32 sys_clk );

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
 * @return T_U32
 * @retval Length of the data which have been written to UART
 */
T_U32 uart_write(T_UART_ID uart_id, const T_U8 *data, T_U32 data_len);

/**
 * @brief read data from uart pool
 * 
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[out] *data buffer to recieve data,as big as you can
 * @param[in] datalen data length to read
 * @return T_U32
 * @retval the data length that has been read
 */
T_U32 uart_read(T_UART_ID uart_id, T_U8 *data, T_U32 datalen);

/**
 * @brief Read a character from UART
 *
 * This function will not return until get a character from UART
 * Function uart_init() must be called before call this function
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[out] *chr character for return
 * @return T_BOOL Got character or not
 * @retval return AK_TRUE
 */
T_BOOL uart_read_chr(T_UART_ID uart_id, T_U8 *chr);

/**
 * @brief set uart data parity mode, use even parity or odd parity or neither
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[in] enable T_BOOL, enable data parity or not
 * @param[in] evenParity T_BOOL, if AK_TRUE enable even parity,else enable odd parity
 * @return T_BOOL
 * @retval if success, return AK_TRUE
 */
T_BOOL uart_setdataparity(T_UART_ID uart_id, T_BOOL enable, T_BOOL evenParity);

/**
 * @brief change uart baudrate
 * @author Liao_Zhijun
 * @date 2010-07-28
 * @param[in] uart_id UART ID
 * @param[in] baud_rate T_U32 new baud_rate to change
 * @return T_BOOL
 * @retval if success, return AK_TRUE
 */
T_BOOL uart_setbaudrate(T_UART_ID uart_id, T_U32 baud_rate);


/*@}*/

#endif

