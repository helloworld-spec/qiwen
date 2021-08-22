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


typedef void (*T_fSPI_CALLBACK)(void);



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
 * @return bool success to return true
 */
bool spi_init(T_eSPI_ID spi_id, T_eSPI_MODE mode, T_eSPI_ROLE role, unsigned long clk);

/**
 * @brief disable spi function
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @return void
 */
void spi_close(T_eSPI_ID spi_id);

/**
 * @brief master write to spi port
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] buf the data buffer pointer
 * @param[in] count the data count in bytes
 * @param[in] bReleaseCS after write data release CS signal or not
 * @return bool success to return true
 */
bool spi_master_write(T_eSPI_ID spi_id, unsigned char *buf, unsigned long count, bool bReleaseCS);

/**
 * @brief master read data from spi port
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] buf the data buffer pointer
 * @param[in] count the data count in bytes
 * @param[in] bReleaseCS after read data, release CS signal or not
 * @return bool success to return true
 */
bool spi_master_read(T_eSPI_ID spi_id, unsigned char *buf, unsigned long count, bool bReleaseCS);

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
 * @return bool success to return true
 */
bool spi_master_data_transfer(T_eSPI_ID spi_id, unsigned char *tx_buf, unsigned char *rx_buf, unsigned long w_r_cnt, bool bReleaseCS);

/**
 * @brief slave read data from spi port
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] buf the data buffer pointer
 * @param[in] count the data count in bytes
 * @return bool success to return true
 */bool spi_slave_read(T_eSPI_ID spi_id, unsigned char *buf, unsigned long count);

/**
 * @brief slave write to spi port
 *
 * @author HuangXin
 * @date
 * @param[in] spi_id the spi port number, refer to T_eSPI_ID
 * @param[in] buf the data buffer pointer
 * @param[in] count the data count in bytes
 * @return bool success to return true
 */
bool spi_slave_write(T_eSPI_ID spi_id, unsigned char *buf, unsigned long count);

/**
 * @brief Initialize SPI
 *
 * this func must be called before call any other SPI functions
 * @author 
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param mode[in] spi mode selected 
 * @param role[in] master or slave
 * @param clk_div[in] SPI working frequency = ASICCLK/(2*(clk_div+1)
 * @return bool
 * @retval true: Successfully initialized SPI.
 * @retval false: Initializing SPI failed.
 */
bool spi_dma_set_callback(T_eSPI_ID spi_id, T_fSPI_CALLBACK rx_setcb,T_fSPI_CALLBACK tx_setcb);


/**
 * @brief spi slave write
 *
 * this func must be called in spi  slave mode
 * @author 
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the write data buffer  
 * @param count[in] the write data count
 * @return bool
 * @retval true:  spi write successfully.
 * @retval false: spi write failed.
 */
bool spi_dma_int_write(T_eSPI_ID spi_id, unsigned char *buf,unsigned long count);

/**
 * @brief spi slave write
 *
 * this func must be called in spi  slave mode
 * @author 
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @param buf[in] the write data buffer  
 * @param count[in] the write data count
 * @return bool
 * @retval true:  spi write successfully.
 * @retval false: spi write failed.
 */
bool spi_dma_int_read(T_eSPI_ID spi_id, unsigned char *buf,unsigned long count);

/**
 * @brief close SPI
 *
 * @author 
 * @date 2010-11-17
 * @param spi_id[in] spi interface selected
 * @return void
 */
void spi_dma_int_close(T_eSPI_ID spi_id);


#endif

