/**@file hal_sd.h
 * @brief provide hal level operations of how to control sd.
 *
 * This file describe sd hal driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */

#ifndef __HAL_SD_H
#define __HAL_SD_H

#include "arch_mmc_sd.h"

#ifdef __cplusplus
extern "C" {
#endif

//sd card status
#define SD_CURRENT_STATE_OFFSET             9
#define SD_CURRENT_STATE_MASK               (0xF<<9)
#define SD_CURRENT_STATE_IDLE               0
#define SD_CURRENT_STATE_READY              1 
#define SD_CURRENT_STATE_IDENT              2
#define SD_CURRENT_STATE_STBY               3
#define SD_CURRENT_STATE_TRAN               4
#define SD_CURRENT_STATE_DATA               5
#define SD_CURRENT_STATE_RCV                6
#define SD_CURRENT_STATE_PRG                7
#define SD_CURRENT_STATE_DIS                8
#define SD_CURRENT_STATE_IO_MODE            15
//sd dma operation block length             
#define SD_DMA_BLOCK_64K                    (64*2)
#define SD_DMA_BLOCK_32K                    (32*2)
#define SD_DMA_BLOCK_8K                     (8*2)
#define SD_DMA_BLOCK_2K                     (2*2)
#define SD_DMA_BLOCK_4K                     (4*2)
                                            
#define SD_HIGH_SPEED_MODE                  1
#define SD_DEFAULT_SPEED_MODE               0
#define SD_MMC_INVALID_SPEC_VERSION         0xff
#define SD_FUNC_SUPPORTED_GROUP1(status)    ((status[51]<<8) | status[50])  
#define SD_FUNC_SUPPORTED_GROUP2(status)    ((status[53]<<8) | status[52])  
#define SD_FUNC_SUPPORTED_GROUP3(status)    ((status[55]<<8) | status[54])  
#define SD_FUNC_SUPPORTED_GROUP4(status)    ((status[57]<<8) | status[56])  
#define SD_FUNC_SUPPORTED_GROUP5(status)    ((status[59]<<8) | status[58]) 
#define SD_FUNC_SUPPORTED_GROUP6(status)    ((status[61]<<8) | status[60])  
#define SD_FUNC_SWITCHED_GROUP1(status)     (status[47]&0x0f)
#define SD_FUNC_SWITCHED_GROUP2(status)     ((status[47]>>4)&0x0f)
#define SD_FUNC_SWITCHED_GROUP3(status)     (status[48]&0x0f)
#define SD_FUNC_SWITCHED_GROUP4(status)     ((status[48]>>4)&0x0f)
#define SD_FUNC_SWITCHED_GROUP5(status)     (status[49]&0x0f)
#define SD_FUNC_SWITCHED_GROUP6(status)     ((status[49]>>4)&0x0f)

#define MMC_SPEC_VERSION(csd)               stuff_bits((unsigned short *)csd,122,4)
#define MMC4_CARD_TYPE(extcsd)              (extcsd[49]&0xff)
#define MMC4_SECTOR_CNT(extcsd)             (extcsd[53])
#define MMC4_POWER_CLASS(extcsd)            (extcsd[50])
#define MMC4_EXT_CSD_REV(extcsd)            (extcsd[48]&0xff)
#define MMC4_PARTITION_CFG(extcsd)          ((extcsd[44]>>24)&0xff)
#define MMC4_PARTITION_SZ(extcsd)           ((extcsd[56]>>16)&0xff)


typedef enum
{
    SD_DATA_MODE_SINGLE,                    ///< read or write single block
    SD_DATA_MODE_MULTI                      ///< read or wirte multiply block
}
T_eCARD_DATA_MODE;

typedef enum _SD_STATUS
{
    SD_GET_OCR_VALID,                       ///<get ocr valid
    SD_GET_OCR_FAIL,                        ///<get ocr fial
    SD_GET_OCR_INVALID,                     ///<get ocr invalid
    SD_NEGO_SUCCESS,                        ///< sd nego voltage success
    SD_NEGO_FAIL,                           ///< sd nego voltage fail
    SD_NEGO_TIMEOUT                         ///< sd nego voltage timeout
}T_eSD_STATUS;

/**
 * @brief Init the mem partion 
 *
 * Called when init card
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCOMMON_SD_STATUS
 */
unsigned char init_mem(bool bInitMem);

/**
 * @brief Switch or expand memory card functions.
 *
 * This function is supported after sd version 1.10
 * @author Huang Xin
 * @date 2010-07-14
 * @param mode[in] mode
 * @param group[in] group
 * @param value[in] value
 * @param resp[in] response
 * @return bool
 * @retval  true: CMD sent successfully
 * @retval  false: CMD sent failed
 */
bool sd_mode_switch(unsigned long mode, unsigned long group, unsigned char value, unsigned long *resp);

/**
 * @brief Wait read or write complete
 *
 * Called when read or write sd card finish
 * @author Huang Xin
 * @date 2010-07-14
 * @return bool
 */
bool wait_rw_complete(void);

/**
 * @brief Set sd card bus width.
 *
 * Usually set the bus width  1 bit or 4 bit  .
 * @author Huang Xin
 * @date 2010-07-14
 * @param wide_bus[in] The bus width.
 * @return bool
 * @retval  true: set successfully
 * @retval  false: set failed
 */
bool sd_set_bus_width(unsigned char wide_bus);


#ifdef __cplusplus
}
#endif


#endif 
  
