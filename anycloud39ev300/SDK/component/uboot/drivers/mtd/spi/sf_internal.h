/*
 * SPI flash internal definitions
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SF_INTERNAL_H_
#define _SF_INTERNAL_H_

#define SPI_FLASH_16MB_BOUN		0x1000000

/* SECT flags */
#define SECT_4K				(1 << 1)
#define SECT_32K			(1 << 2)
#define E_FSR				(1 << 3)


#define SFLAG_DUAL_IO_READ         	(1<<4)
#define SFLAG_DUAL_READ           	(1<<5)
#define SFLAG_QUAD_IO_READ         	(1<<6)
#define SFLAG_QUAD_READ           	(1<<7)

#define SFLAG_DUAL_IO_WRITE        	(1<<8)
#define SFLAG_DUAL_WRITE          	(1<<9)
#define SFLAG_QUAD_IO_WRITE        	(1<<10)
#define SFLAG_QUAD_WRITE          	(1<<11)

//#define SFLAG_SECT_4K       		(1<<12)
#define SFLAG_COM_STATUS2         	(1<<12)
#define SFLAG_COM_STATUS3         	(1<<13)

#define	SFLAG_UNDER_PROTECT			(1<<14)
//#define SFLAG_FAST_READ           	(1<<15)
//#define SFLAG_AAAI                	(1<<16)
#define SFLAG_QUAD_NO_QE			(1<<17)
/*
* cdh:use 3B address mode flag, but capacity beyond 16MB, and support Extended Address Register
*/ 
#define SFLAG_COM_BIT24          	(1<<15)   


/* Erase commands */
#define CMD_ERASE_4K			0x20
#define CMD_ERASE_32K			0x52
#define CMD_ERASE_CHIP			0xc7
#define CMD_ERASE_64K			0xd8

/* Write commands */
#define CMD_WRITE_STATUS		0x01
#define CMD_WRITE_STATUS2 		0x31    /* Write Status2 Register eg:gd25q128c*/ 
#define CMD_WRITE_STATUS3      	0x11    /* Write Status3 Register eg:gd25q128c*/ 

#define CMD_PAGE_PROGRAM		0x02
#define CMD_PAGE_PROGRAM_DUAL	0x12	/* Dual Page Program*/
#define CMD_PAGE_PROGRAM_QUAD	0x32	/* Quad Page Program*/
#define CMD_PAGE_PROGRAM_2IO 	0x18	/* 2I/O Page Program (tmp)*/
#define CMD_PAGE_PROGRAM_4IO	0x38	/* 4I/O Page Program*/

#define CMD_WRITE_DISABLE		0x04
#define CMD_READ_STATUS			0x05
#define CMD_READ_STATUS2   		0x35    /* Read Status Register2 */ 
#define CMD_READ_STATUS3  		0x15    /* Read Status Register3 */ 


#define CMD_WRITE_ENABLE		0x06
#define CMD_READ_CONFIG			0x35
#define CMD_FLAG_STATUS			0x70

/* Read commands */
#define CMD_READ_ARRAY_SLOW		0x03
#define CMD_READ_ARRAY_FAST		0x0b
#define CMD_READ_ARRAY_DUAL 	0x3b    /* Read Data Bytes at Dual output */ 
#define CMD_READ_ARRAY_QUAD 	0x6b    /* Read Data Bytes at Quad output */ 
#define CMD_READ_ARRAY_DUAL_IO 	0xbb    /* Read Data Bytes at Dual i/o */ 
#define CMD_READ_ARRAY_QUAD_IO 	0xeb    /* Read Data Bytes at Quad i/o */ 

/* software reset 0x66 + 0x99 */
#define CMD_ENABLE_RESET  		0x66    
#define CMD_RESET_DEVICE  		0x99    


#ifdef CONFIG_SPIFLASH_USE_FAST_READ
#define OPCODE_READ 	CMD_READ_ARRAY_FAST
#define FAST_READ_DUMMY_BYTE 1
#else
#define OPCODE_READ 	CMD_READ_ARRAY_SLOW
#define FAST_READ_DUMMY_BYTE 0
#endif


#define CMD_READ_ID			0x9f

/* Bank addr access commands */
#ifdef CONFIG_SPI_FLASH_BAR
#define CMD_BANKADDR_BRWR		0x17
#define CMD_BANKADDR_BRRD		0x16
#define CMD_EXTNADDR_WREAR		0xC5
#define CMD_EXTNADDR_RDEAR		0xC8
#endif

/* Common status */
#define STATUS_WIP			0x01
#define STATUS_PEC			0x80

/* Flash timeout values */
#define SPI_FLASH_PROG_TIMEOUT		(2 * CONFIG_SYS_HZ)
#define SPI_FLASH_PAGE_ERASE_TIMEOUT	(5 * CONFIG_SYS_HZ)
#define SPI_FLASH_SECTOR_ERASE_TIMEOUT	(10 * CONFIG_SYS_HZ)

/* SST specific */
#ifdef CONFIG_SPI_FLASH_SST
# define SST_WP			0x01	/* Supports AAI word program */
# define CMD_SST_BP		0x02    /* Byte Program */
# define CMD_SST_AAI_WP		0xAD	/* Auto Address Incr Word Program */

int sst_write_wp(struct spi_flash *flash, u32 offset, size_t len,
		const void *buf);
#endif

/* Send a single-byte command to the device and read the response */
int spi_flash_cmd(struct spi_slave *spi, u8 cmd, void *response, size_t len);

/*
 * Send a multi-byte command to the device and read the response. Used
 * for flash array reads, etc.
 */
int spi_flash_cmd_read(struct spi_slave *spi, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len);

/*
 * Send a multi-byte command to the device followed by (optional)
 * data. Used for programming the flash array, etc.
 */
int spi_flash_cmd_write(struct spi_slave *spi, const u8 *cmd, size_t cmd_len,
		const void *data, size_t data_len);


/* Flash erase(sectors) operation, support all possible erase commands */
int spi_flash_cmd_erase_ops(struct spi_flash *flash, u32 offset, size_t len);

/* Program the status register */
int spi_flash_cmd_write_status(struct spi_flash *flash, u32 sr);

/* read the status register */
int spi_flash_cmd_read_status(struct spi_flash *flash, u32 *sr);

int spi_flash_quad_mode_enable(struct spi_flash *flash);

int spi_flash_quad_mode_disable(struct spi_flash *flash);

/*tmp*/
int spi_flash_gd25q128c_cmd_write_status(struct spi_flash *flash, u32 sr);

/* Set quad enbale bit */
int spi_flash_set_qeb(struct spi_flash *flash);

/* Enable writing on the SPI flash */
static inline int spi_flash_cmd_write_enable(struct spi_flash *flash)
{
	return spi_flash_cmd(flash->spi, CMD_WRITE_ENABLE, NULL, 0);
}

/* Disable writing on the SPI flash */
static inline int spi_flash_cmd_write_disable(struct spi_flash *flash)
{
	return spi_flash_cmd(flash->spi, CMD_WRITE_DISABLE, NULL, 0);
}

/*
 * Send the read status command to the device and wait for the wip
 * (write-in-progress) bit to clear itself.
 */
int spi_flash_cmd_wait_ready(struct spi_flash *flash, unsigned long timeout);

/*
 * Used for spi_flash write operation
 * - SPI claim
 * - spi_flash_cmd_write_enable
 * - spi_flash_cmd_write
 * - spi_flash_cmd_wait_ready
 * - SPI release
 */
int spi_flash_write_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, const void *buf, size_t buf_len);

/*
 * Flash write operation, support all possible write commands.
 * Write the requested data out breaking it up into multiple write
 * commands as needed per the write size.
 */
int spi_flash_cmd_write_ops(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf);


/*
 * Flash write operation, support all possible write commands.
 * Write the requested data out breaking it up into multiple write
 * commands as needed per the write size.
 */
int spi_flash_cmd_write_ops_extra(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf);


/*
 * Same as spi_flash_cmd_read() except it also claims/releases the SPI
 * bus. Used as common part of the ->read() operation.
 */
int spi_flash_read_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len);

/* Flash read operation, support all possible read commands */
int spi_flash_cmd_read_ops(struct spi_flash *flash, u32 offset,
		size_t len, void *data);


/* Flash read operation, support all possible read commands */
int spi_flash_cmd_read_ops_extra(struct spi_flash *flash, u32 offset,
		size_t len, void *data);

#endif /* _SF_INTERNAL_H_ */
