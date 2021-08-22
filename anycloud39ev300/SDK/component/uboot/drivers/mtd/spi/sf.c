/*
 * SPI flash interface
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2010 Reinhard Meyer, EMK Elektronik
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spi.h>

/**
* @brief spi write and read data with cmd
*
* spi write and read data with cmd
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *spi spi_slave handle
* @param[out] *cmd cmd data pointer
* @param[out] cmd_len cmd data len
* @param[out] *data_out tx buffer pointer
* @param[in] *data_in rx buffer pointer
* @param[in/out] data_len tx or rx data length
* @return      int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
static int spi_flash_read_write(struct spi_slave *spi,
				const u8 *cmd, size_t cmd_len,
				const u8 *data_out, u8 *data_in,
				size_t data_len)
{
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;

	if (data_len == 0)
		flags |= SPI_XFER_END;

	debug("rw:cmd:0x%x, cmd_len:0x%x, data_out:0x%x, data_in:0x%x, data_len:0x%x\n",
				*cmd, cmd_len, data_out, data_in, data_len);
	
	ret = spi_xfer(spi, cmd_len * 8, cmd, NULL, flags);
	if (ret) {
		debug("SF: Failed to send command (%zu bytes): %d\n",
		      cmd_len, ret);
	} else if (data_len != 0) {
		ret = spi_xfer(spi, data_len * 8, data_out, data_in,
					SPI_XFER_END);
		if (ret)
			debug("SF: Failed to transfer %zu bytes of data: %d\n",
			      data_len, ret);
	}

	return ret;
}

/**
* @brief spi read data with cmd
*
* spi read data with cmd
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *spi spi_slave handle
* @param[out] *cmd cmd data pointer
* @param[out] cmd_len cmd data len
* @param[in] *data rx buffer pointer
* @param[in] data_len rx data length
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
int spi_flash_cmd_read(struct spi_slave *spi, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	return spi_flash_read_write(spi, cmd, cmd_len, NULL, data, data_len);
}

/**
* @brief spi send cmd
*
* spi send cmd, some time with cmd response
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *spi spi_slave handle
* @param[out] cmd cmd data 
* @param[in] *response cmd response
* @param[in] len  cmd response data length
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
int spi_flash_cmd(struct spi_slave *spi, u8 cmd, void *response, size_t len)
{
	return spi_flash_cmd_read(spi, &cmd, 1, response, len);
}

/**
* @brief spi write data with cmd
*
* spi write data with cmd
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *spi spi_slave handle
* @param[out] *cmd cmd data pointer
* @param[out] cmd_len cmd data len
* @param[out] *data tx buffer pointer
* @param[out] data_len tx data length
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
int spi_flash_cmd_write(struct spi_slave *spi, const u8 *cmd, size_t cmd_len,
		const void *data, size_t data_len)
{
	return spi_flash_read_write(spi, cmd, cmd_len, data, NULL, data_len);
}



