/**
* @file l2.h
* @brief l2 buffer driver head file
* 
* Copyright (C) 2007 Anyka (Guang zhou) Software Technology Co., LTD
* 
* @author liao_zhijun
* @date 2010-06-09
* @version 0.1
*/

#ifndef _L2_H_
#define _L2_H_

#include "anyka_types.h"


/** @defgroup L2 L2 group
 *      @ingroup Drv_Lib
 */
/*@{*/

/** @name transfer direction
         define the direction of transfer between memory and l2
 */
/*@{*/
#define    BUF2MEM                0
#define    MEM2BUF                1
/*@} */

/** @name L2 bits
 */
#define    FRAC_DMA_START_REQ          (1<<9)

/** @name l2 buffer size
 */
#define    L2_COMBUF_SIZE     512

/** @name invalidate l2 buffer id
 */
#define    BUF_NULL     0xff

/** @name l2 callback function
 */
typedef T_VOID (*T_fL2_CALLBACK)(T_VOID);

/** @name l2 excutive callback function
 */
typedef T_VOID (*T_fL2Exe_CALLBACK)(T_U32 param);

/** @brief device name
 * define all device
 */
typedef enum
{
    ADDR_USB_EP1 = 0,           ///< usb ep1        0
    ADDR_USB_EP2 = 1,           ///< usb ep2        1
    ADDR_USB_EP3 = 2,           ///< usb ep2        2
    ADDR_MMC_SD = 4,              ///< mci1           4
    ADDR_SDIO = 5,              ///< mci2           5
    ADDR_SPI0_RX = 7,           ///< spi0 rx        7
    ADDR_SPI0_TX = 8,           ///< spi0 tx        8    
    ADDR_DAC = 9,               ///< dac            9

	ADDR_SPI1_RX = 10,           ///< spi1 rx        10
	ADDR_SPI1_TX = 11,           ///< spi1 tx        11    
    ADDR_PCM = 13,              ///< uart3 tx       13
    ADDR_ADC2 = 14,              ///< adc            14
    ADDR_USB_EP4 = 15,           ///< usb ep2        2

	ADDR_RESERVE = 16           ///< reserve
}DEVICE_SELECT;

/**
* @brief initial l2 buffer
* @author liao_zhijun
* @date 2010-06-09
* @return T_VOID 
*/
T_VOID l2_init(T_VOID);

/**
* @brief allocate a common l2 buffer for giving device
* 
* first we try to find a common buffer which is not used by other device yet, 
* then clear it's status and attach it to the giving device
* 
* @author liao_zhijun
* @date 2010-06-09
* @param dev_slct [in]: device that need a l2 buffer 
* @return T_U8 buffer id
* @retval 0~7 alloc success and the buffer id is returned
* @retval 0xFF no buffer found, alloc fail
*/
T_U8 l2_alloc(DEVICE_SELECT dev_slct);



/**
* @brief free the l2 common buffer used by giving device
* 
* @author liao_zhijun
* @date 2010-06-09
* @param dev_slct [in]: device name, the buffer attached with it will be freed 
* @return T_VOID
*/
T_VOID l2_free(DEVICE_SELECT dev_slct);

/**
* @brief set a callback function
* @author liao_zhijun
* @date 2010-06-09
* @param callback_func [in]: dam finish call this function
* @param buf_id [in]: the buffer id
* @return T_BOOL
*/
T_BOOL l2_set_dma_callback(T_U8 buf_id, T_fL2_CALLBACK callback_func);


/**
* @brief start data tranferring between memory and l2 common buffer with dma mode
* 
* we just start dma here and return immediately , and the l2 interrupt is disabled here
* if tran_byte is not the multi-64-bytes, a following call of l2_combuf_wait_dma_finish() is required
* 
* @author liao_zhijun
* @date 2010-06-09
* @param ram_addr [in/out]: the memory address
* @param buf_id [in]: the buffer id
* @param tran_byte [in]: the size of data to be transfered
* @param tran_dir [in]: transfer data from memory to buffer or from buffer to memory
* @param bIntr [in]: open interrupt for this buffer or not
* @return T_VOID
*/
T_VOID l2_combuf_dma(T_U32 ram_addr, T_U8 buf_id, T_U32 tran_byte, T_U8 tran_dir, T_BOOL bIntr);

/**
* @brief transfer data between memory and l2 common buffer with cpu
* 
* if their transfer direction is from memory to l2, and trans_bytes is not multiple of 64_bytes,
* l2_combuf_cpu will add 1 to buffer flag automatically, user doesn't need to set buffer flag again
* 
* @author liao_zhijun
* @date 2010-06-09
* @param ram_addr [in/out]: the memory address
* @param buf_id [in]: the buffer id
* @param tran_byte [in]: the size of data to be transfered
* @param tran_dir [in]: transfer data from memory to buffer or from buffer to memory
* @retval AK_TRUE: previous dma finished succesfully
* @retval AK_FALSE: previous dma fail
*/
T_BOOL l2_combuf_cpu(T_U32 ram_addr, T_U8 buf_id, T_U32 tran_byte, T_U8 tran_dir);

/**
* @brief wait for dma started by l2_combuf_dma tranferring finish
* 
* if the tran_byte set in l2_combuf_dma is not multi-64-bytes, we'll start a fraction dma here
* and wait it finish, if the direction is memory to l2, buffer flag is changed manually
*
* @author liao_zhijun
* @date 2010-06-09
* @param buf_id [in]: the buffer id
* @return T_BOOL 
* @retval AK_TRUE: previous dma finished succesfully
* @retval AK_FALSE: previous dma fail
*/
T_BOOL l2_combuf_wait_dma_finish(T_U8 buf_id);
/**
* @brief stop l2_combuf_dma tranferring 
* 
* if dma can not finish for some reason,stop l2_combuf_dma manually
* @author liao_zhijun
* @date 2010-06-09
* @param buf_id [in]: the buffer id
* @return T_BOOL 
* @retval T_VOID
*/
T_VOID l2_combuf_stop_dma(T_U8 buf_id);

/**
* @brief start data tranferring between memory and l2 uart buffer with dma mode
* 
* we just start dma here and return immediately , and the l2 interrupt is disabled here
* if tran_byte is not the multi-64-bytes, a following call of l2_uartbuf_wait_dma_finish() is required
* 
* @author liao_zhijun
* @date 2010-06-09
* @param ram_addr [in/out]: the memory address
* @param uart_id [in]: the uart id
* @param tran_byte [in]: the size of data to be transfered
* @param tran_dir [in]: transfer data from memory to buffer or from buffer to memory
* @return T_VOID
*/
T_VOID l2_uartbuf_dma(T_U32 ram_addr, T_U8 uart_id, T_U32 tran_byte, T_U8 tran_dir);

/**
* @brief transfer data between memory and l2 uart buffer with cpu
* 
* if ther trans direction is from memory to l2, and trans_bytes is not multi-64-bytes,
* we'll set buffer flag manually
* 
* @author liao_zhijun
* @date 2010-06-09
* @param ram_addr [in/out]: the memory address
* @param uart_id [in]: the uart id
* @param tran_byte [in]: the size of data to be transfered
* @param tran_dir [in]: transfer data from memory to buffer or from buffer to memory
* @return T_VOID
*/
T_VOID l2_uartbuf_cpu(T_U32 ram_addr, T_U8 uart_id, T_U32 tran_byte, T_U8 tran_dir);


/**
* @brief wait for dma started by l2_uartbuf_dma tranferring finish
* 
* if the tran_byte set in l2_uartbuf_dma is not multi-64-bytes, we'll start a fraction dma here
* and wait it finish, if the direction is memory to l2, buffer flag is changed manually
*
* @author liao_zhijun
* @date 2010-06-09
* @param uart_id [in]: the uart id
* @param tran_dir [in]: transfer data from memory to buffer or from buffer to memory
* @return T_BOOL 
* @retval AK_TRUE: previous dma finished succesfully
* @retval AK_FALSE: previous dma fail
*/
T_VOID l2_uartbuf_wait_dma_finish(T_U8 uart_id, T_U8 tran_dir);


/**
* @brief return a l2 buffer' current status 
* @author liao_zhijun
* @date 2010-06-09
* @param buf_id [in]: the buffer id
* @return 0~7 buffer status
*/
T_U8 l2_get_status(T_U8 buf_id);


/**
* @brief clear a l2 buffer's status 
* @author liao_zhijun
* @date 2010-06-09
* @param buf_id [in]: the buffer id
* @return T_VOID
*/
T_VOID l2_clr_status(T_U8 buf_id);

/**
* @brief get l2 memory address by id
* @author liao_zhijun
* @date 2010-06-09
* @param buf_id [in]: the buffer id
* @return T_VOID
*/

T_U32  l2_get_addr(T_U8 buf_id);

/**
* @brief set status for a l2 common buffer 
* @author liao_zhijun
* @date 2010-06-09
* @param buf_id [in]: common buffer id
* @param status [in]: status number to be set, 0 <= status <= 8
* @return T_VOID
*/
T_VOID l2_set_status(T_U8 buf_id, T_U8 status);

/**
* @brief Execute the code in table,which function is determined by the table
* 
* The buffer to be used must be less than 512 bytes
* 
* @author huang_xin
* @date 2009-11-05
* @param table [in]: The table which contain specific codes
* @param len [in]:  size of table 
* @return T_BOOL
*/
T_BOOL l2_specific_exebuf(T_fL2Exe_CALLBACK code, T_U32 param);

/*@}*/

#endif
