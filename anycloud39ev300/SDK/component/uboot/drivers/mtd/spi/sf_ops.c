/*
 * SPI flash operations
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2010 Reinhard Meyer, EMK Elektronik
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/errno.h>
#include <watchdog.h>

#include "sf_internal.h"

#define	CMD_SIZE		(1)
#define ADDR_SIZE		(3)
#define CMD_ADDR_SIZE	(CMD_SIZE + ADDR_SIZE)
#define MAX_DUMMY_SIZE	(4)

#define FILL_CMD(c, val) do{c[0] = (val);}while(0)
#define FILL_ADDR(c, val) do{	\
		c[CMD_SIZE] = (val) >> 16;	\
		c[CMD_SIZE+1] = (val) >> 8;	\
		c[CMD_SIZE+2] = (val);		\
		}while(0)
		
#define FILL_DUMMY_DATA(c, val) do{	\
			c[CMD_ADDR_SIZE] = val >> 16;	\
			c[CMD_ADDR_SIZE+1] = 0;	\
			c[CMD_ADDR_SIZE+2] = 0;	\
			c[CMD_ADDR_SIZE+3] = 0;	\
			}while(0)

/**
* @brief fill spi read or write address
*
* fill spi read or write address
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] addr spi norflash offset address
* @param[out] *cmd cmd buffer with offset address
* @return void
* @retval none
*/
static void spi_flash_addr(u32 addr, u8 *cmd)
{
	/* cmd[0] is actual command */
	cmd[1] = (addr >> 16)&0xff;
	cmd[2] = (addr >> 8)&0xff;
	cmd[3] = (addr >> 0)&0xff;
}


/**
* @brief spi flash gd25q128c write status
*
* spi flash gd25q128c write status
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] sr  will to write status value
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
* @notes status1 and status2  different write status cmd
*/
int spi_flash_gd25q128c_cmd_write_status(struct spi_flash *flash, u32 sr)
{
	u8 cmd;
	u8 sr_tmp;
	int ret;
	
	cmd = CMD_WRITE_STATUS;

	sr_tmp = sr & 0xff;
	ret = spi_flash_write_common(flash, &cmd, 1, &sr_tmp, 1);
	if (ret < 0) {
		debug("SF: fail to write status register\n");
		return ret;
	}

	if (flash->flags & SFLAG_COM_STATUS2) {
		cmd = CMD_WRITE_STATUS2;

		sr_tmp = (sr>>8) & 0xff;
		ret = spi_flash_write_common(flash, &cmd, 1, &sr_tmp, 1);
		if (ret < 0) {
			debug("SF: fail to write status2 register\n");
			return ret;
		}
	}
	return 0;
}


/**
* @brief spi flash write status
*
* spi flash  write status
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[out] sr  will to write status value
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
* @notes status1 and status2  same write status cmd
*/
int spi_flash_cmd_write_status(struct spi_flash *flash, u32 sr)
{
	u8 cmd;
	int wr_cnt;
	int ret;
	u32 value = 0;

	//debug("############%s,###########\n", __func__);

	if (!(flash->flags & SFLAG_COM_STATUS3)) {
		//printf("####SFLAG_COM_STATUS2####\n");

		/* if exist status2, then status1 and status2 are same write cmd */
		cmd = CMD_WRITE_STATUS;
	    if (flash->flags & SFLAG_COM_STATUS2) {
	        wr_cnt = 2;
	    } else {
			wr_cnt = 1;
	    }

	    ret = spi_flash_write_common(flash, &cmd, 1, &sr, wr_cnt);
		if (ret < 0) {
			debug("SF: fail to write status register\n");
			return ret;
		}
	}else {
		//printf("####CMD_WRITE_STATUS3####\n");

		/* for three status, with different write cmd */
		cmd = CMD_WRITE_STATUS;
		wr_cnt = 1;
		value = sr & 0xff;
		ret = spi_flash_write_common(flash, &cmd, 1, &value, wr_cnt);
		if (ret < 0) {
			debug("SF: fail to write status register\n");
			return ret;
		}

		cmd = CMD_WRITE_STATUS2;
		wr_cnt = 1;
		value = (sr>>8) &0xff;
		ret = spi_flash_write_common(flash, &cmd, 1, &value, wr_cnt);
		if (ret < 0) {
			debug("SF: fail to write status register\n");
			return ret;
		}

		cmd = CMD_WRITE_STATUS3;
		wr_cnt = 1;
		value = (sr>>16) &0xff;
		ret = spi_flash_write_common(flash, &cmd, 1, &value, wr_cnt);
		if (ret < 0) {
			debug("SF: fail to write status register\n");
			return ret;
		}

	}

	return 0;
}


/**
* @brief spi flash read status
*
* spi flash  write status
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[out]* sr  will to read status buffer pointer
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
* @notes status1 status2 and status3  different write status cmd
*/
int spi_flash_cmd_read_status(struct spi_flash *flash, u32 *sr)
{
	u8 cmd;
	u8 sr_tmp;
	int ret;
	
	cmd = CMD_READ_STATUS;

	ret = spi_flash_read_common(flash, &cmd, 1, &sr_tmp	, 1);
	if (ret < 0) {
		debug("SF: fail to read status register\n");
		return ret;
	}
	
	*sr = sr_tmp;

    if (flash->flags & SFLAG_COM_STATUS2) {
		cmd = CMD_READ_STATUS2;
		ret = spi_flash_read_common(flash, &cmd, 1, &sr_tmp	, 1);
		if (ret < 0) {
			debug("SF: fail to raed status 2 register\n");
			return ret;
		}
		
		*sr = (*sr | ((u32)sr_tmp << 8));
    }

    if (flash->flags & SFLAG_COM_STATUS3) {
		cmd = CMD_READ_STATUS3;
		ret = spi_flash_read_common(flash, &cmd, 1, &sr_tmp	, 1);
		if (ret < 0) {
			debug("SF: fail to read status 2 register\n");
			return ret;
		}
		
		*sr = (*sr | ((u32)sr_tmp << 16));
    }

	return *sr;
}

/**
* @brief show spi flash status reg value
*
* show spi flash status reg value
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @return int  spi flash status reg value
* @retval spi flash status reg value
*/
int show_spi_flash_reg(struct spi_flash *flash)
{
	int ret;
	u32 regval;	
	struct flash_status_reg *fsr = &flash->stat_reg;
	
	ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
	if (ret)
		return -EBUSY;

	ret = fsr->read_sr(flash, &regval);

	debug("spi flash status regval is:%08x.\n", regval);
	return 0;

}

/**
* @brief enable spi flash status reg quad QE
*
* enable spi flash status reg quad QE
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
int spi_flash_quad_mode_enable(struct spi_flash *flash)
{
	int ret;
	u32 regval;	
	struct flash_status_reg *fsr = &flash->stat_reg;
	
	ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
	if (ret)
		return -EBUSY;

	ret = fsr->read_sr(flash, &regval);
	regval |= 1<<fsr->b_qe;
	ret = fsr->write_sr(flash, regval);

	return 0;
}

/**
* @brief disable spi flash status reg quad QE
*
* disable spi flash status reg quad QE
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
int spi_flash_quad_mode_disable(struct spi_flash *flash)
{
	int ret;
	u32 regval;
	struct flash_status_reg *fsr = &flash->stat_reg;
		
	ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
	if (ret)
		return -EBUSY;
	
	ret = fsr->read_sr(flash, &regval);
	regval &= ~(1<<fsr->b_qe);
	ret = fsr->write_sr(flash, regval);

	return 0;
}

#ifdef CONFIG_SPI_FLASH_BAR
/**
* @brief select spi flash beyond 16MB size space bank 
*
* select spi flash beyond 16MB size space bank for 3Byte address mode
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] bank_sel: high address space bank index beyond 16MB space 
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
static int spi_flash_cmd_bankaddr_write(struct spi_flash *flash, u8 bank_sel)
{
	u8 cmd;
	int ret;

	if (flash->bank_curr == bank_sel) {
		debug("SF: not require to enable bank%d\n", bank_sel);
		return 0;
	}

	cmd = flash->bank_write_cmd;
	ret = spi_flash_write_common(flash, &cmd, 1, &bank_sel, 1);
	if (ret < 0) {
		debug("SF: fail to write bank register\n");
		return ret;
	}
	flash->bank_curr = bank_sel;

	return 0;
}

/**
* @brief set spi flash beyond 16MB size space bank 
*
* set spi flash beyond 16MB size space bank for 3Byte address mode
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] bank_sel: high address space bank index beyond 16MB space 
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
* @notes 4Bytes address mode no need do this
*/
static int spi_flash_bank(struct spi_flash *flash, u32 offset)
{
	u8 bank_sel;
	int ret;

	bank_sel = offset / SPI_FLASH_16MB_BOUN;

	ret = spi_flash_cmd_bankaddr_write(flash, bank_sel);
	if (ret) {
		debug("SF: fail to set bank%d\n", bank_sel);
		return ret;
	}

	return 0;
}
#endif

/**
* @brief wait for spi flash ready status
*
* wait for spi flash ready status
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] timeout: wait for max time
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
int spi_flash_cmd_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timebase;
	int ret;
	u8 status;
	u8 check_status = 0x0;
	u8 poll_bit = STATUS_WIP;
	u8 cmd = flash->poll_cmd;
	struct flash_status_reg *fsr = &flash->stat_reg;

	/* here has some problem must clear */
	if (cmd == CMD_FLAG_STATUS) {
		poll_bit = STATUS_PEC;
		check_status = poll_bit;
	}

	ret = spi_xfer(spi, 8, &cmd, NULL, SPI_XFER_BEGIN);
	if (ret) {
		debug("SF: fail to read %s status register\n",
		      cmd == CMD_READ_STATUS ? "read" : "flag");
		return ret;
	}

	timebase = get_timer(0);
	do {
		WATCHDOG_RESET();

		ret = spi_xfer(spi, 8, NULL, &status, 0);
		if (ret)
			return -1;

		if (!(status & (1<<fsr->b_wip))) {
			//debug("cdh:flash ready ok!\n");
			break;
		}
	} while (get_timer(timebase) < timeout);

	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);

	if (!(status & (1<<fsr->b_wip)))
		return 0;

	/* Timed out */
	debug("SF: time out!\n");
	
	return -1;
}

/**
* @brief spi flash common write operation
*
* spi flash common write operation
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] *cmd write cmd buffer pointer
* @param[in] cmd_len write cmd length
* @param[in] *buf write data buffer pointer
* @param[in] buf_len write data len
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
int spi_flash_write_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, const void *buf, size_t buf_len)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timeout = SPI_FLASH_PROG_TIMEOUT;
	int ret;

	if (buf == NULL)
		timeout = SPI_FLASH_PAGE_ERASE_TIMEOUT;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: unable to claim SPI bus\n");
		return ret;
	}

	ret = spi_flash_cmd_write_enable(flash);
	if (ret < 0) {
		debug("SF: enabling write failed\n");
		return ret;
	}

	ret = spi_flash_cmd_write(spi, cmd, cmd_len, buf, buf_len);
	if (ret < 0) {
		debug("SF: write cmd failed\n");
		return ret;
	}

	ret = spi_flash_cmd_wait_ready(flash, timeout);
	if (ret < 0) {
		debug("SF: write %s timed out\n",
		      timeout == SPI_FLASH_PROG_TIMEOUT ?
			"program" : "page erase");
		return ret;
	}

	spi_release_bus(spi);

	return ret;
}

static ulong ebytes_per_second(unsigned int len, ulong start_ms)
{
	/* less accurate but avoids overflow */
	if (len >= ((unsigned int) -1) / 1024)
		return len / (max(get_timer(start_ms) / 1024, 1));
	else
		return 1024 * len / max(get_timer(start_ms), 1);
}
/**
* @brief spi flash erase operation
*
* spi flash erase operation
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] offset erase flash address offset
* @param[in] len eraselength
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
int spi_flash_cmd_erase_ops(struct spi_flash *flash, u32 offset, size_t len)
{
	u32 erase_size;
    u32 erase_size_4k = 4*1024;
    u32 erase_size_64k = 64*1024;
    u32 offset_align = 0;
	u8 cmd[4];
	int ret = -1;
    u32 offset_count_4k = 0;
    u32 count_4k = 0;
    u32 count_64k = 0;
    u32 remain_len = 0;
	struct spi_slave *spi;

	size_t scale = 1;
	u32 end = offset + len;
	u32 start_buf = offset;
	const ulong start_time = get_timer(0);
	
	if (len >= 8192)
		scale = (len) / 4096;
	

	/* add for after tf card operated , erase failed ,because spi share pin */
	spi = flash->spi;
	spi_sharepin_cfg(spi);
	
	erase_size = flash->erase_size;
	
	/* check erase flash offset and len size must be multi times than erase_size */
	if (offset % erase_size || len % erase_size) {
		debug("SF_cdh: Erase offset/length not multiple of erase size:0x%x\n", erase_size);
		return -1;
	}

    if( len >= erase_size_64k)
    {
    /*check if the offset 64k align. if not,we have to erase 4k first until offset 64k align. */
        offset_align = offset % erase_size_64k; 
        if( offset_align != 0)
        {
            offset_count_4k = (erase_size_64k - offset_align) / erase_size_4k;
            remain_len = (erase_size_64k - offset_align) % erase_size_4k;
            if( remain_len != 0)
                offset_count_4k++;  

            /*we begin to erase 64k*/
            count_64k = (len - (erase_size_64k - offset_align))/erase_size_64k;
            remain_len = (len - (erase_size_64k - offset_align))%erase_size_64k;
            if(remain_len != 0)
            {
                count_4k = remain_len / erase_size_4k;
                if( remain_len % erase_size_4k != 0)
                    count_4k++; 
            } 
        }
        else
        {
        /*if the offset 64k align , we begin to erase 64k*/
            count_64k = len /erase_size_64k;
            remain_len = len %erase_size_64k;
            if(remain_len != 0)
            {
                count_4k = remain_len / erase_size_4k;
                if( remain_len % erase_size_4k != 0)
                    count_4k++; 
            }        
        }
    }
    else
    {
        count_4k = len / erase_size_4k;
        remain_len = len % erase_size_4k;
        if( remain_len != 0)
            count_4k++;     
    }

	unsigned long last_update = get_timer(0); // cdh:add for 
	while (len) {
		if (get_timer(last_update) > 100) {
			printf("   \rEraseing, %zu%% %lu B/s",
			       100 - (end - offset) / scale,
				ebytes_per_second(offset - start_buf,
						 start_time));
			last_update = get_timer(0);
		}
		
#ifdef CONFIG_SPI_FLASH_BAR
		ret = spi_flash_bank(flash, offset);
		if (ret < 0)
			return ret;
#endif
        if( offset_count_4k != 0)
            cmd[0] = CMD_ERASE_4K; 
        else if( count_64k != 0 && offset_count_4k == 0)
            cmd[0] = CMD_ERASE_64K; 
        else if( count_4k != 0 && count_64k == 0 )
            cmd[0] = CMD_ERASE_4K;
        
		spi_flash_addr(offset, cmd);

		debug("617 SF: erase %2x %2x %2x %2x (%x)\n", cmd[0], cmd[1],
		      cmd[2], cmd[3], offset);

		ret = spi_flash_write_common(flash, cmd, sizeof(cmd), NULL, 0);
		if (ret < 0) {
			debug("SF: erase failed\n");
			break;
		}
        if( offset_count_4k != 0)
        {
            offset += erase_size_4k;
		    len -= erase_size_4k;
            offset_count_4k--;
        }
        else if( count_64k != 0 && offset_count_4k == 0)
        {
            offset += erase_size_64k;
		    len -= erase_size_64k;
            count_64k--;
        }
        else if( count_4k != 0 && count_64k == 0)
        {
            offset += erase_size_4k;
		    len -= erase_size_4k;
            count_4k--;
        }
	}

	return ret;
}
/**
* @brief spi flash write operation
*
* spi flash write operation
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] offset flash address offset pointer
* @param[in] len write data length
* @param[in] *buf write data buffer pointer
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
int spi_flash_cmd_write_ops(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
	unsigned long byte_addr, page_size;
	size_t chunk_len, actual;
	u8 cmd[4];
	int ret = -1;
	struct spi_slave *spi;

	/* add for after tf card operated , erase failed ,because spi share pin */
	spi = flash->spi;
	spi_sharepin_cfg(spi);
	
	page_size = flash->page_size;
	cmd[0] = CMD_PAGE_PROGRAM;
	
	for (actual = 0; actual < len; actual += chunk_len) {
#ifdef CONFIG_SPI_FLASH_BAR
		ret = spi_flash_bank(flash, offset);
		if (ret < 0)
			return ret;
#endif
		byte_addr = offset % page_size;
		chunk_len = min(len - actual, page_size - byte_addr);

		if (flash->spi->max_write_size)
			chunk_len = min(chunk_len, flash->spi->max_write_size);

		spi_flash_addr(offset, cmd);

		debug("PP: 0x%p => cmd = { 0x%02x 0x%02x%02x%02x } chunk_len = %zu\n",
		      buf + actual, cmd[0], cmd[1], cmd[2], cmd[3], chunk_len);

		ret = spi_flash_write_common(flash, cmd, sizeof(cmd),
					buf + actual, chunk_len);
		if (ret < 0) {
			debug("SF: write failed\n");
			break;
		}

		offset += chunk_len;
	}

	return ret;
}


/**
* @brief spi flash common read operation
*
* spi flash common read operation
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] *cmd cmd buffer pointer
* @param[in] cmd_len cmd data length
* @param[in] *data read data buffer pointer
* @param[in] data_len read data len
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
int spi_flash_read_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	struct spi_slave *spi = flash->spi;
	int ret;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: unable to claim SPI bus\n");
		return ret;
	}

	ret = spi_flash_cmd_read(spi, cmd, cmd_len, data, data_len);
	if (ret < 0) {
		debug("SF: read cmd failed\n");
		return ret;
	}

	spi_release_bus(spi);

	return ret;
}


/**
* @brief spi flash cmd read operation
*
* spi flash cmd read operation
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] offset flash address offset 
* @param[in] len read data length
* @param[in] *data read data buffer pointer
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
int spi_flash_cmd_read_ops(struct spi_flash *flash, u32 offset,
		size_t len, void *data)
{
	u8 cmd[5];
	u8 bank_sel = 0;
	u32 remain_len;
	u32 read_len;
	int ret = -1;

	/* Handle memory-mapped SPI, what function ??? */
	if (flash->memory_map) {
		spi_xfer(flash->spi, 0, NULL, NULL, SPI_XFER_MMAP);
		memcpy(data, flash->memory_map + offset, len);
		spi_xfer(flash->spi, 0, NULL, NULL, SPI_XFER_MMAP_END);
		return 0;
	}

	cmd[0] = CMD_READ_ARRAY_FAST;
	cmd[4] = 0x00;

	while (len) {
#ifdef CONFIG_SPI_FLASH_BAR
		bank_sel = offset / SPI_FLASH_16MB_BOUN;

		ret = spi_flash_cmd_bankaddr_write(flash, bank_sel);
		if (ret) {
			debug("SF: fail to set bank%d\n", bank_sel);
			return ret;
		}
#endif
		remain_len = (SPI_FLASH_16MB_BOUN * (bank_sel + 1)) - offset;
		if (len < remain_len)
			read_len = len;
		else
			read_len = remain_len;

		spi_flash_addr(offset, cmd);

		ret = spi_flash_read_common(flash, cmd, sizeof(cmd),
							data, read_len);
		if (ret < 0) {
			debug("SF: read failed\n");
			break;
		}

		offset += read_len;
		len -= read_len;
		data += read_len;
	}

	return ret;
}

/**
* @brief spi flash cmd extra write one operation
*
* spi flash cmd extra write one operation
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] *cmd write cmd pointer
* @param[in] cmd_len cmd data length
* @param[in] *data write data buffer pointer
* @param[in] data_len write data len
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
* @notes write one page operation
*/
int spi_flash_write_extra(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, const void *data, size_t data_len)
{
	struct spi_slave *spi = flash->spi;
	struct spi_xfer_ctl *xfer_ctl = &flash->xfer_ctl;
	unsigned long timeout = SPI_FLASH_PROG_TIMEOUT;
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;

	if (data == NULL)
		timeout = SPI_FLASH_PAGE_ERASE_TIMEOUT;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: unable to claim SPI bus\n");
		return ret;
	}

	ret = spi_flash_cmd_write_enable(flash);
	if (ret < 0) {
		debug("SF: enabling write failed\n");
		return ret;
	}

	if (data_len == 0)
		flags |= SPI_XFER_END;

	ret = spi_xfer(spi, cmd_len * 8, cmd, NULL, flags|xfer_ctl->txa_bus_width);
	if (ret) {
		debug("SF: Failed to send command (%zu bytes): %d\n",
		      cmd_len, ret);
	} else if (data_len != 0) {
		ret = spi_xfer(spi, data_len * 8, data, NULL,
					SPI_XFER_END|xfer_ctl->txd_bus_width);
		if (ret)
			debug("SF: Failed to transfer %zu bytes of data: %d\n",
			      data_len, ret);
	}

	ret = spi_flash_cmd_wait_ready(flash, timeout);
	if (ret < 0) {
		debug("SF: write %s timed out\n",
		      timeout == SPI_FLASH_PROG_TIMEOUT ?
			"program" : "page erase");
		return ret;
	}

	spi_release_bus(spi);

	return ret;
}


/**
* @brief spi flash cmd extra write all operation
*
* spi flash cmd extra write all operation
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] offset write spi flash address offset
* @param[in] len write data length
* @param[in] *buf write data buffer pointer
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
* @notes write all page operation
*/
int spi_flash_cmd_write_ops_extra(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
	unsigned long byte_addr;
	unsigned long page_size;
	struct spi_xfer_ctl *xfer_ctl = &flash->xfer_ctl;
	size_t chunk_len;
	size_t actual;
	u8 cmd[4];
	int ret = -1;

	page_size = flash->page_size;
	cmd[0] = xfer_ctl->tx_opcode;

	for (actual=0; actual<len; actual+=chunk_len) {
#ifdef CONFIG_SPI_FLASH_BAR
		ret = spi_flash_bank(flash, offset);
		if (ret < 0)
			return ret;
#endif
		byte_addr = offset % page_size;
		chunk_len = min(len - actual, page_size - byte_addr);
		if (flash->spi->max_write_size) {
			debug("flash->spi->max_write_size\n");
			chunk_len = min(chunk_len, flash->spi->max_write_size);
		}
		
		spi_flash_addr(offset, cmd);

		debug("PP: 0x%p => cmd = { 0x%02x 0x%02x%02x%02x } chunk_len = %zu\n",
		      buf + actual, cmd[0], cmd[1], cmd[2], cmd[3], chunk_len);

		ret = spi_flash_write_extra(flash, cmd, sizeof(cmd),
					buf + actual, chunk_len);
		if (ret < 0) {
			debug("SF: write failed\n");
			break;
		}

		offset += chunk_len;
	}

	return ret;
}


/**
* @brief spi flash cmd extra read one operation
*
* spi flash cmd extra read one operation
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] *cmd read cmd pointer
* @param[in] cmd_len cmd data length
* @param[out] *data read data buffer pointer
* @param[in] data_len read data len
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
* @notes write one page operation
*/
int spi_flash_read_extra(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	struct spi_slave *spi = flash->spi;
	struct spi_xfer_ctl *xfer_ctl = &flash->xfer_ctl;
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: unable to claim SPI bus\n");
		return ret;
	}

	if (data_len == 0)
		flags |= SPI_XFER_END;

	ret = spi_xfer(spi, cmd_len * 8, cmd, NULL, flags|xfer_ctl->rxa_bus_width);
	if (ret) {
		debug("SF: Failed to send command (%zu bytes): %d\n",
		      cmd_len, ret);
	} else if (data_len != 0) {
		ret = spi_xfer(spi, data_len * 8, NULL, data,
					SPI_XFER_END|xfer_ctl->rxd_bus_width);
		if (ret)
			debug("SF: Failed to transfer %zu bytes of data: %d\n",
			      data_len, ret);
	}

	spi_release_bus(spi);

	return ret;
}


/**
* @brief spi flash cmd extra read all operation
*
* spi flash cmd extra read all operation
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *flash spi_flash handle
* @param[in] offset read spi flash address offset
* @param[in] len read data length
* @param[in] *buf read data buffer pointer
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
* @notes write all page operation
*/
int spi_flash_cmd_read_ops_extra(struct spi_flash *flash, u32 offset,
		size_t len, void *data)
{
	struct spi_xfer_ctl *xfer_ctl = &flash->xfer_ctl;
	u8 cmd[CMD_ADDR_SIZE + MAX_DUMMY_SIZE], bank_sel = 0;
	u8 cmd_len;
	u8 flags = 0;
	u32 remain_len;
	u32 read_len;
	int ret = -1;

	/* Handle memory-mapped SPI */
	if (flash->memory_map) {
		spi_xfer(flash->spi, 0, NULL, NULL, SPI_XFER_MMAP);
		memcpy(data, flash->memory_map + offset, len);
		spi_xfer(flash->spi, 0, NULL, NULL, SPI_XFER_MMAP_END);
		return 0;
	}

	cmd[0] = xfer_ctl->rx_opcode;
	FILL_DUMMY_DATA(cmd, 0x00);
	cmd_len = CMD_SIZE + ADDR_SIZE + xfer_ctl->dummy_len;

	while (len) {
#ifdef CONFIG_SPI_FLASH_BAR
		bank_sel = offset / SPI_FLASH_16MB_BOUN;

		ret = spi_flash_cmd_bankaddr_write(flash, bank_sel);
		if (ret) {
			debug("SF: fail to set bank%d\n", bank_sel);
			return ret;
		}
#endif
		remain_len = (SPI_FLASH_16MB_BOUN * (bank_sel + 1)) - offset;
		if (len < remain_len)
			read_len = len;
		else
			read_len = remain_len;

		spi_flash_addr(offset, cmd);

		ret = spi_flash_read_extra(flash, cmd, cmd_len, 
				data, read_len);
		if (ret < 0) {
			debug("SF: read failed\n");
			break;
		}

		offset += read_len;
		len -= read_len;
		data += read_len;
	}

	return ret;
}



#ifdef CONFIG_SPI_FLASH_SST
static int sst_byte_write(struct spi_flash *flash, u32 offset, const void *buf)
{
	int ret;
	u8 cmd[4] = {
		CMD_SST_BP,
		offset >> 16,
		offset >> 8,
		offset,
	};

	debug("BP[%02x]: 0x%p => cmd = { 0x%02x 0x%06x }\n",
	      spi_w8r8(flash->spi, CMD_READ_STATUS), buf, cmd[0], offset);

	ret = spi_flash_cmd_write_enable(flash);
	if (ret)
		return ret;

	ret = spi_flash_cmd_write(flash->spi, cmd, sizeof(cmd), buf, 1);
	if (ret)
		return ret;

	return spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
}

int sst_write_wp(struct spi_flash *flash, u32 offset, size_t len,
		const void *buf)
{
	size_t actual, cmd_len;
	int ret;
	u8 cmd[4];

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	/* If the data is not word aligned, write out leading single byte */
	actual = offset % 2;
	if (actual) {
		ret = sst_byte_write(flash, offset, buf);
		if (ret)
			goto done;
	}
	offset += actual;

	ret = spi_flash_cmd_write_enable(flash);
	if (ret)
		goto done;

	cmd_len = 4;
	cmd[0] = CMD_SST_AAI_WP;
	cmd[1] = offset >> 16;
	cmd[2] = offset >> 8;
	cmd[3] = offset;

	for (; actual < len - 1; actual += 2) {
		debug("WP[%02x]: 0x%p => cmd = { 0x%02x 0x%06x }\n",
		      spi_w8r8(flash->spi, CMD_READ_STATUS), buf + actual,
		      cmd[0], offset);

		ret = spi_flash_cmd_write(flash->spi, cmd, cmd_len,
					buf + actual, 2);
		if (ret) {
			debug("SF: sst word program failed\n");
			break;
		}

		ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret)
			break;

		cmd_len = 1;
		offset += 2;
	}

	if (!ret)
		ret = spi_flash_cmd_write_disable(flash);

	/* If there is a single trailing byte, write it out */
	if (!ret && actual != len)
		ret = sst_byte_write(flash, offset, buf + actual);

 done:
	debug("SF: sst: program %s %zu bytes @ 0x%zx\n",
	      ret ? "failure" : "success", len, offset - actual);

	spi_release_bus(flash->spi);
	return ret;
}
#endif
