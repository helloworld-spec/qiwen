/**
 * @file arch_spi.h
 * @brief SPI driver header file.
 *
 * This file provides SPI APIs: SPI initialization, write data to SPI, read data from SPI.
 * Copyright (C) 2005 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Huang Xin
 * @date 2010-11-17
 * @version 1.0
 */

#ifndef             __SPI_H
#define             __SPI_H


typedef T_VOID (*T_fSPI_CALLBACK)(T_VOID);



/**
 * @brief SPI port define
 * define port name with port number
 */
typedef enum
{
    SPI_ID0 = 0,
    SPI_ID1,
    SPI_NUM
}T_eSPI_ID;

/**
 * @brief the mode of spi
 *
 */
typedef enum
{
    SPI_MODE0 = 0,///<CPHA = 0, CPOL = 0
    SPI_MODE1,    ///<CPHA = 0, CPOL = 1
    SPI_MODE2,    ///<CPHA = 1, CPOL = 0
    SPI_MODE3,     ///<CPHA = 1, CPOL = 1
    SPI_MODE_NUM
}T_eSPI_MODE;


typedef enum
{
    SPI_SLAVE = 0,
    SPI_MASTER,
    SPI_ROLE_NUM
}T_eSPI_ROLE;


typedef enum
{
    SPI_BUS1 = 0,
    SPI_BUS2,
    SPI_BUS4,
    SPI_BUS_NUM
}T_eSPI_BUS;

/**
 * @brief init spi function
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] mode the spi mode, refer to T_eSPI_MODE
 * @param[in] role the spi role, refer to T_eSPI_ROLE
 * @param[in] clk the spi working clock(unit:Hz)
 * @return T_BOOL success to return AK_TRUE
 */
T_BOOL spi_init(T_eSPI_ID spi_id, T_eSPI_MODE mode, T_eSPI_ROLE role, T_U32 clk);

/**
 * @brief disable spi function
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @return T_VOID
 */
T_VOID spi_close(T_eSPI_ID spi_id);

/**
 * @brief master write to spi port
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] buf the data buffer pointer
 * @param[in] count the data count in bytes
 * @param[in] bReleaseCS after write data release CS signal or not
 * @return T_BOOL success to return AK_TRUE
 */
T_BOOL spi_master_write(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS);

/**
 * @brief master read data from spi port
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] buf the data buffer pointer
 * @param[in] count the data count in bytes
 * @param[in] bReleaseCS after read data, release CS signal or not
 * @return T_BOOL success to return AK_TRUE
 */
T_BOOL spi_master_read(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count, T_BOOL bReleaseCS);

/**
 * @brief master write and read data to/from spi port simultaneously
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] tx_buf the data buffer to write
 * @param[in] rx_buf the data buffer to read
 * @param[in] w_r_cnt the data count to read and write in bytes
 * @param[in] bReleaseCS after read data, release CS signal or not
 * @return T_BOOL success to return AK_TRUE
 */
T_BOOL spi_master_data_transfer(T_eSPI_ID spi_id, T_U8 *tx_buf, T_U8 *rx_buf, T_U32 w_r_cnt, T_BOOL bReleaseCS);

/**
 * @brief slave read data from spi port
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] buf the data buffer pointer
 * @param[in] count the data count in bytes
 * @return T_BOOL success to return AK_TRUE
 */T_BOOL spi_slave_read(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count);

/**
 * @brief slave write to spi port
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] buf the data buffer pointer
 * @param[in] count the data count in bytes
 * @return T_BOOL success to return AK_TRUE
 */
T_BOOL spi_slave_write(T_eSPI_ID spi_id, T_U8 *buf, T_U32 count);

#endif

