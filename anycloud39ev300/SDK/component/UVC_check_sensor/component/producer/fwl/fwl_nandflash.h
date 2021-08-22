/**
 * @filename fwl_nandflash.h
 * @brief: AK3224M frameworks of nandflash driver.
 *
 * This file describe frameworks of nandflash driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  zhaojiahuan
 * @date    2006-11-03
 * @version 1.0
 * @ref
 */

#ifndef __FWL_NANDFLASH_H__
#define __FWL_NANDFLASH_H__

#include "anyka_types.h"
#include "nandflash.h"

#ifdef __cplusplus
extern "C" {
#endif

bool Fwl_NandHWInit(unsigned long gpio_ce2, unsigned long gpio_ce3, unsigned long* ChipID, unsigned long* ChipCnt);

                                     
/**
 * @brief   write 1 page data to nandflash with ECC.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be written.
 * @param   [in] block which block will be written.
 * @param   [in] data buffer for read page, should be a page size.
 * @param   [in] oob buffer for oob infomation, maybe 4 bytes or 8 bytes.
 * @param   [in] oob_len for length of oob infomation.

 * @return  E_NANDERRORCODE
 */                                        
E_NANDERRORCODE Nand_WriteSector(T_PNANDFLASH pNF_Info, unsigned long chip, unsigned long block, unsigned long page, unsigned char data[], unsigned char *oob, unsigned long oob_len);

/**
 * @brief   read 1 page data from nandflash with ECC.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be read.
 * @param   [in] block which block will be read.
 * @param   [in] page which page will be read.
 * @param   [in] data buffer for read page, should be a page size.
 * @param   [in] oob buffer for oob infomation, maybe 4 bytes or 8 bytes.
 * @param   [in] oob_len for length of oob infomation.
 * @return  E_NANDERRORCODE
 */                                        
E_NANDERRORCODE Nand_ReadSector(T_PNANDFLASH pNF_Info, unsigned long chip, unsigned long block, unsigned long page, unsigned char data[], unsigned char *oob, unsigned long oob_len);

/**
 * @brief   copy one physical page to another one, soft hardware.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be operated.
 * @param   [in] SourceBlock read the source block.
 * @param   [in] DestBlock write to destination block.
 * @param   [in] page the page of the block will be copy.
 * @return  E_NANDERRORCODE
 */
E_NANDERRORCODE Nand_CopyBack(T_PNANDFLASH pNF_Info, unsigned long chip, unsigned long SourceBlock, unsigned long DestBlock, unsigned long page);

/** 
 * @brief   erase 1 block of nandflash.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip will be operated.
 * @param   [in] block which block whill be erased.
 * @return  unsigned long
 */
E_NANDERRORCODE Nand_EraseBlock(T_PNANDFLASH pNF_Info, unsigned long chip, unsigned long block);

/**
 * @brief   initialization of nandflash frameworks.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] void.
 * @return  T_PNANDFLASH
 */
//T_PNANDFLASH Nand_Init(T_NAND_PHY_INFO *nand_info);

/**
 * @brief   check bad blocks of nandflash.
 *
 * @author  zhaojiahuan
 * @date    2006-11-02
 * @param   [in] pNF_Info the struct of nandflash.
 * @param   [in] chip which chip of nandflash.
 * @param   [in] block which block of nandflash.
 * @return  bool
 */
bool Nand_IsBadBlock(T_PNANDFLASH pNF_Info, unsigned long chip, unsigned long block);

void Nand_SetBadBlock(T_PNANDFLASH pNF_Info, unsigned long chip, unsigned long block );

unsigned long FHA_Nand_EraseBlock(unsigned long nChip,  unsigned long nPage);

unsigned long FHA_Nand_WritePage(unsigned long nChip, unsigned long nPage, const unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen, unsigned long eDataType);

unsigned long FHA_Nand_ReadPage(unsigned long nChip,  unsigned long nPage, unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen , unsigned long eDataType);

bool ASA_ReadBytes(unsigned long chip, unsigned long rowAddr, unsigned long columnAddr, unsigned char data[], unsigned long len);


#ifdef __cplusplus
}
#endif

#endif
