/**
 * @file hal_spiflash.h
 * @brief spiflash driver interface file.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */
#ifndef _SERIAL_FLASH_H_
#define _SERIAL_FLASH_H_


#include "anyka_types.h"
#include "arch_spi.h"

/** @defgroup spiflash group
 *  @ingroup Drv_Lib
 */
/*@{*/

#include "arch_spi.h"

/**
 * @brief  flag define, used in spi flash flag param
 */
#define SFLASH_FLAG_UNDER_PROTECT       (1<<0)
#define SFLASH_FLAG_FAST_READ           (1<<1)
#define SFLASH_FLAG_AAAI                (1<<2)

#define SFLASH_FLAG_DUAL_WRITE          (1<<3)
#define SFLASH_FLAG_DUAL_READ           (1<<4)
#define SFLASH_FLAG_QUAD_WRITE          (1<<5)
#define SFLASH_FLAG_QUAD_READ           (1<<6)
#define SFLASH_FLAG_COM_STATUS2         (1<<7)


/**
 * @brief extern flag define, used in spi flash extern flag param
 */
#define SFLASH_EX_FLAG_STATUS_MASK			((1<<0)|(1<<1))		
#define SFLASH_EX_FLAG_4B_ADDR				(1<<2)
#define SFLASH_EX_FLAG_STATUS_COM_W_MASK	(1<<3)


/**
 * @brief  serial flash param define
 */
typedef struct tagSFLASH_PARAM
{
    unsigned long   id;                     ///< flash id
    unsigned long   total_size;             ///< flash total size in bytes
    unsigned long   page_size;              ///< bytes per page
    unsigned long   program_size;           ///< program size at 02h command
    unsigned long   erase_size;             ///< erase size at d8h command 
    unsigned long   clock;                  ///< spi clock, 0 means use default clock 

    //chip character bits:
    //bit 0: under_protect flag, the serial flash under protection or not when power on
    //bit 1: fast read flag, the serial flash support fast read or not(command 0Bh)
    //bit 2: AAI flag, the serial flash support auto address increment word programming 
    unsigned char    flag;                   ///< chip character bits
    unsigned char    protect_mask;           ///< protect mask bits in status register:BIT2:BP0, BIT3:BP1, BIT4:BP2, BIT5:BP3, BIT7:BPL

	//chip extern character bits:
	//bit 0 bit 1   0 0    only one status register (status1) to read and write
	//bit 0 bit 1   0 1    there are two status register (status1 status2) to read and write
	//bit 0 bit 1   1 0	  there are three status register (status1 status2 status3)to read and write
	//bit 0 bit 1   1 1    reserved
	
	//bit2: support 4 byte addressing to read and write or no
	//bit3: support two and more status register common  use same command to be write or no
	unsigned char    ex_flag;
    unsigned char    reserved2;
}T_SFLASH_PARAM;

/**
 * @brief spi flash init
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param spi_id [in]  spi id, can be spi0 or spi1
 * @return bool
 */
bool spi_flash_init(T_eSPI_ID spi_id, T_eSPI_BUS bus_width);

/**
 * @brief set param of serial flash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param sflash_param [in]  serial flash param 
 * @return void
 */
void spi_flash_set_param(T_SFLASH_PARAM *sflash_param);

/**
 * @brief get spiflash id
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return unsigned long
 * @retval unsigned long spiflash id
 */
unsigned long spi_flash_getid(void);

/**
 * @brief read data from one page of spiflash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  page number
 * @param sector [in]  block number, unit is erase_size, refer to T_SFLASH_PARAM
 * @return bool
 */
bool spi_flash_erase(unsigned long block);

/**
 * @brief write data to one page of spiflash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  page number
 * @param buf [in]  buffer to be write
 * @param page_cnt [in]  the page count to write  
 * @return bool
 */
bool spi_flash_write(unsigned long page, unsigned char *buf, unsigned long page_cnt);

/**
 * @brief erase one block of spiflash
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param page [in]  page number
 * @param buf [in]  buffer to store read data 
 * @param page_cnt [in]  the page count to read  
 * @return bool
 */
bool spi_flash_read(unsigned long page, unsigned char *buf, unsigned long page_cnt);


/**
 * @brief erase one sector of spiflash (4K)
 * 
 * @author chen_yongping
 * @date 2013-04-17
 * @param page [in]  sector number, unit is 4k
 * @return bool
 */
bool spi_flash_erase_sector(unsigned long sector);

/**
 * @brief erase one sector of spiflash
 * 
 * @author chenyongping
 * @date 2013-05-22
 * @param addr [in]  erase addr 
 * @param length [in]  erase length 
 * @return bool
 */

bool spi_flash_erase_addr(unsigned long addr, unsigned long length);

/*@}*/
#endif	// _SERIAL_FLASH_H_


