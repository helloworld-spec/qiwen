/**
 * @file hal_spiflash.h
 * @brief spiflash driver interface file.
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author LiaoZhijun
 * @date 2010-05-27
 * @version 1.0
 */
#ifndef _HAL_SPINAND_H_
#define _HAL_SPINAND_H_



#include "anyka_types.h"
#include "arch_spi.h"

/** @defgroup spiflash group
 *  @ingroup Drv_Lib
 */
/*@{*/

/** 
  * spi nand param
  * spi_id:  spi id, can be spi0 or spi1
  * clock:   spi clock
  * flag:    not use yet, define here for future
  //bit31
  //bit30
  //bit29
  //bit28
  //bit27
  //bit26
  //bit25
  //bit24
  //bit23
  //bit22
  //bit21
  //bit20
  //bit19
  //bit18
  //bit17
  //bit16
  //bit15
  //bit14
  //bit13
  //bit12
  //bit11
  //bit10  
  //bit9  //表read flag(03H, read from cach),  0表示先发地址，再发dummy, 1表示相反
  //bit8  //表示QEflag值
  
  //bit7~0  表示spare区的用户数据位置的开始地址

        
  * SpiBusWidth: spi line set:0: 1 line; 
                              1: 2 line; 
                              2: 4 line; 
*/
typedef struct
{
    T_eSPI_ID spi_id;                     ///< flash id
    unsigned long  clock;                  ///< spi clock, 0 means use default clock 
    unsigned long  flag;                   ///< chip character bits    
    unsigned char  SpiBusWidth;
}T_SPI_NAND_PARAM;


/**
 * @brief spi nand init
 * 
 * @author liao_zhijun
 * @date 2014-1-2
 * @param spi_id [in]  spi id, can be spi0 or spi1
 * @param bus_wight [in] select bus width
 * @param clk [in] spi clock
 * @return T_BOOL
 */
bool spi_nand_init(T_eSPI_ID spi_id, T_eSPI_BUS bus_wight, unsigned long clk);

/**
 * @brief set param of serial nand
 * 
 * @author liao_zhijun
 * @date 2014-1-2
 * @param param [in]  spi nand param 
 * @return T_BOOL
 */
bool spi_nand_set_param(T_SPI_NAND_PARAM *param);


/**
 * @brief get spi nand id
 * 
 * @author liao_zhijun
 * @date 2014-1-2
 * @return T_U32
 * @retval T_U32 spiflash id
 */
unsigned long spi_nand_getid(void);

/**
 * @brief erase one block of spi nand
 * 
 * @author liao_zhijun
 * @date 2014-1-2
 * @param row [in]  row address of spi nand
 * @return T_BOOL
 */
bool spi_nand_erase(unsigned long row);


/**
 * @brief write data to one page of spi nand
 * 
 * @author liao_zhijun
 * @date 2014-1-2
 * @param row [in]  row address
 * @param column [in]  column address
 * @param data [in]  data buffer to be write
 * @param len [in]  data  length
 * @return T_BOOL
 */
//bool spi_nand_write(unsigned long row, unsigned long column, unsigned char *data, unsigned long len, unsigned char *oob, unsigned long ooblen, unsigned long oobpos);
bool spi_nand_write(unsigned long row, unsigned long column, unsigned char *data, unsigned long len);

//T_BOOL spi_nand_write(T_U32 row, T_U32 column, T_U8 *data, T_U32 len);
/**
 * @brief read data from one page of spi nand
 * 
 * @author liao_zhijun
 * @date 2014-1-2
 * @param row [in]  row address
 * @param column [in]  column address
 * @param data [in]  data buffer to be read
 * @param len [in]  data  length
 * @return T_BOOL
 */
bool spi_nand_read(unsigned long row, unsigned long column, unsigned char *data, unsigned long len);

/**
 * @brief read page0 data to cache
 * 
 * @author panminghua
 * @date 2014-3-20
 * @return T_BOOL
 */
bool spi_nand_readto_cache(void);

/**
 * @brief read data from one page of spi nand without ecc
 * 
 * @author panminghua
 * @date 2014-3-20
 * @param row [in]  row address
 * @param column [in]  column address
 * @param data [in]  data buffer to be read
 * @param len [in]  data  length
 * @return T_BOOL
*/
bool spi_nand_read_noecc(unsigned long row, unsigned long column, unsigned char *data, unsigned long len);


/*@}*/


#endif    // _SERIAL_FLASH_H_


