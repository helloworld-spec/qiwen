/**
 * @file arch_sdio.h
 * @brief list SDIO card operation interfaces.
 *
 * This file define and provides functions of SDIO card
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Huang Xin
 * @date 2010-06-17
 * @version 2.0 for AK88xx
 */

#ifndef __ARCH_SDIO_H__
#define __ARCH_SDIO_H__

/** @addtogroup MMC_SD_SDIO
 *  @ingroup Drv_Lib
 */
/*@{*/

typedef void (*T_SDIO_INT_HANDLER)(void);



/**
 * @brief initial sdio or combo card
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] bus_mode bus mode selected, can be USE_ONE_BUS or USE_FOUR_BUS
 * @return bool
 * @retval true  set initial successful, card type is sdio or combo
 * @retval false set initial fail,card type is not sdio or combo
 */
bool sdio_initial(T_eCARD_INTERFACE cif,unsigned char bus_mode, unsigned long block_len);


/**
 * @brief enable specifical fuction in sdio card
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to enable
 * @return bool
 * @retval true enable successfully
 * @retval false enable failed
 */
bool sdio_enable_func(unsigned char func);


/**
 * @brief set block length to sdio card
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to set block length
 * @param[in] block_len  block length to set
 * @return bool
 * @retval true enable successfully
 * @retval false enable failed
 */
bool sdio_set_block_len(unsigned char func, unsigned long block_len);


/**
 * @brief  set sdio interrupt callback function
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] cb callback function
 * @return bool
 * @retval true set successfully
 * @retval false set failed
 */
bool sdio_set_int_callback(T_SDIO_INT_HANDLER cb);

 
/**
 * @brief read one byte  from sdio card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to read
 * @param[in] addr register address to read
 * @param[in] rdata data buffer for read data
 * @return bool
 * @retval true read successfully
 * @retval false read failed
 */
bool sdio_read_byte(unsigned char func, unsigned long addr,  unsigned char *rdata);

 
/**
 * @brief write one byte to sdio card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to write
 * @param[in] addr register address to write
 * @param[in] wdata the write byte
 * @return bool
 * @retval true write successfully
 * @retval false write failed
 */
bool sdio_write_byte(unsigned char func, unsigned long addr, unsigned char wdata);

 
/**
 * @brief read multiple byte or block from sdio card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to read
 * @param[in] src register address to read
 * @param[in] count data size(number of byte) to read
 * @param[in] opcode fixed address or increasing address
 * @param[in] rdata data buffer for read data
 * @return bool
 * @retval true read successfully
 * @retval false read failed
 */
bool sdio_read_multi(unsigned char func, unsigned long src, unsigned long count, unsigned char opcode, unsigned char rdata[]);

 
/**
 * @brief write multiple byte or block from sdio card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] func function to read
 * @param[in] dest register address to read
 * @param[in] count data size(number of byte) to read
 * @param[in] opcode fixed address or increasing address
 * @param[in] wdata the wirte data
 * @return bool
 * @retval true write successfully
 * @retval false write failed
 */
bool sdio_write_multi(unsigned char func, unsigned long dest, unsigned long count, unsigned char opcode, unsigned char wdata[]);


/**
 * @brief select or deselect a sdio device
 *
 * the card is selected by its own relative address and gets deselected by any other address; address 0 deselects all
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] addr the rca of  the card which will be selected
 * @return bool
 * @retval true  select or deselect successfully
 * @retval false  select or deselect failed
 */
bool sdio_select_card(unsigned long addr);

/*@}*/

#endif //__ARCH_SDIO_H__

