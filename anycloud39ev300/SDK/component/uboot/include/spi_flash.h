/*
 * Common SPI flash Interface
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

#include <spi.h>
#include <linux/types.h>
#include <linux/compiler.h>


struct spi_flash;
/**
  *because of some spi flash is difference of status register difinition.
  *this structure use mapping the status reg function and corresponding.
*/
struct flash_status_reg
{
	u32		jedec_id;	
	u16		ext_id;
	unsigned b_wip:4;		/*write in progress*/
	unsigned b_wel:4;		/*wrute ebabke latch*/
	unsigned b_bp0:4;		/*block protected 0*/
	unsigned b_bp1:4;		/*block protected 1*/
	unsigned b_bp2:4;		/*block protected 2*/
	unsigned b_bp3:4;		/*block protected 3*/
	unsigned b_bp4:4;		/*block protected 4*/
	unsigned b_srp0:4;		/*status register protect 0*/
	
	unsigned b_srp1:4;		/*status register protect 1*/
	unsigned b_qe:4;		/*quad enable*/
	unsigned b_lb:4;		/*write protect control and status to the security reg.*/
/*
	unsigned b_reserved0:4;
	unsigned b_reserved1:4;
	unsigned b_reserved2:4;
*/
	unsigned b_cmp:4;		/*conjunction bp0-bp4 bit*/
	unsigned b_sus:4;		/*exec an erase/program suspend command*/
	
	/*
	* ADS:The Current Address Mode bit is a read only bit in the status Register-3 that
	* indicates which address mode the device is currently operating in.
	* 0:3-Byte Address Mode. 1:4-Byte Address Mode
	*/
	unsigned b_as:8;		/*0:3-Byte Address Mode. 1:4-Byte Address Mode*/

	/*
	* ADP:The ADP bit is a non-volatile bit that determines the initial address mode when
	* the device is powered on or reset. This bit is only used during the power on or device 
	* reset initialization period, and it is only writable. When ADP=0 (Factory default),the device
	* will power up into 3-Byte Address Mode, the Extended Address Register must be used
	* to access memory regions beyond 128Mb.
	* When ADP=1,the device will power up into 4-Byte address Mode directly
	* 0:3-Byte Address Mode. 1:4-Byte Address Mode
	*/
	unsigned b_ap:8;		/*0:3-Byte Address Mode. 1:4-Byte Address Mode*/
	
	int (*read_sr)(struct spi_flash *, u32*);
	int (*write_sr)(struct spi_flash *, u32);
};

struct spi_xfer_ctl
{
	u8		dummy_len;
	u8		erase_opcode;

	u8		tx_opcode;
	u8		rx_opcode;

	u8		txd_bus_width;
	u8		rxd_bus_width;
	
	u8		txa_bus_width;
	u8		rxa_bus_width;	
};


/**
 * struct spi_flash - SPI flash structure
 *
 * @spi:		SPI slave
 * @name:		Name of SPI flash
 * @size:		Total flash size
 * @page_size:		Write (page) size
 * @sector_size:	Sector size
 * @erase_size:		Erase size
 * @bank_read_cmd:	Bank read cmd
 * @bank_write_cmd:	Bank write cmd
 * @bank_curr:		Current flash bank
 * @poll_cmd:		Poll cmd - for flash erase/program
 * @erase_cmd:		Erase cmd 4K, 32K, 64K
 * @memory_map:		Address of read-only SPI flash access
 * @read:		Flash read ops: Read len bytes at offset into buf
 *			Supported cmds: Fast Array Read
 * @write:		Flash write ops: Write len bytes from buf into offeset
 *			Supported cmds: Page Program
 * @erase:		Flash erase ops: Erase len bytes from offset
 *			Supported cmds: Sector erase 4K, 32K, 64K
 * return 0 - Sucess, 1 - Failure
 */
struct spi_flash {
	struct spi_slave *spi;
	const char *name;
	u32 flashid; // cdh:add
	u32 size;
	u32 page_size;
	u32 sector_size;
	u32 erase_size;
#ifdef CONFIG_SPI_FLASH_BAR
	u8 bank_read_cmd;
	u8 bank_write_cmd;
	u8 bank_curr;
#endif
	u8 poll_cmd;
	u8 erase_cmd;

	struct flash_status_reg stat_reg;
	struct spi_xfer_ctl xfer_ctl;
	u8 		bus_width;
	u32 	flags;

	void *memory_map;
	int (*read)(struct spi_flash *flash, u32 offset, size_t len, void *buf);
	int (*write)(struct spi_flash *flash, u32 offset, size_t len,
			const void *buf);
	int (*erase)(struct spi_flash *flash, u32 offset, size_t len);
};

typedef struct
{
    u32 chip_id;
    u32 total_size;             ///< flash total size in bytes
    u32	page_size;       ///< total bytes per page
    u32	program_size;    ///< program size at 02h command
    u32	erase_size;      ///< erase size at d8h command 
    u32	clock;           ///< spi clock, 0 means use default clock 
    
    //chip character bits:
    //bit 0: under_protect flag, the serial flash under protection or not when power on
    //bit 1: fast read flag    
    u8  flag;            ///< chip character bits
    u8	protect_mask;
	u8  reserved1;
    u8  reserved2;
    u8  des_str[32];		   //描述符                                    
}T_SFLASH_PHY_INFO;

/**
 * @brief  serial flash param define
 */
typedef struct tagSFLASH_PARAM
{
    u32   id;                     ///< flash id
    u32   total_size;             ///< flash total size in bytes
    u32   page_size;              ///< bytes per page
    u32   program_size;           ///< program size at 02h command
    u32   erase_size;             ///< erase size at d8h command 
    u32   clock;                  ///< spi clock, 0 means use default clock 

    //chip character bits:
    //bit 0: under_protect flag, the serial flash under protection or not when power on
    //bit 1: fast read flag, the serial flash support fast read or not(command 0Bh)
    //bit 2: AAI flag, the serial flash support auto address increment word programming
    //bit 3: support dual write or no
    //bit 4: support dual read or no
    //bit 5: support quad write or no
    //bit 6: support quad read or no
    //bit 7: the second status command (35h) flag,if use 4-wire(quad) mode,the bit must be is enable
    u8    flag;                   ///< chip character bits
    u8    protect_mask;           ///< protect mask bits in status register:BIT2:BP0, BIT3:BP1, BIT4:BP2, BIT5:BP3, BIT7:BPL
#ifdef USE_FOR_BURNTOOL_PARAM
    u8    reserved1;
    u8    reserved2;
#else
	u8 	*desc;
#endif
}T_SFLASH_PARAM;

typedef struct
{
    u32  file_length;    //bin:file_length  fs:未定      
    u32  ld_addr;    //bin: ld_addr     fs:未定
    u32  backup_page;    //bin:backup_page  fs:未定     
    u8   check;    //bin:check          fs:未定  
    u8   rev1;
    u8   rev2;
    u8   rev3;
}T_BIN_CONFIG;

typedef struct
{
    u32  file_length;    //      
    u8   check;          //  
    u8   rev1;
    u8   rev2;
    u8   rev3;
    u32  rev4;          //
    u32  rev5;          //  
}T_FS_CONFIG;


typedef struct
{
    u32  parameter1;    //bin:file_length  fs:未定      
    u32  parameter2;    //bin: ld_addr     fs:未定
    u32  parameter3;    //bin:backup_page  fs:未定     
    u32  parameter4;    //bin:check          fs:未定     
}T_EX_PARTITION_CONFIG;

#define     PARTITION_NAME_LEN                  6 


typedef struct
{
    unsigned char         type;        //data,/bin/fs  ,E_PARTITION_TYPE
    volatile unsigned char  r_w_flag:4;               //only read or write
    unsigned char         hidden_flag:4;            //hidden or no hidden
    unsigned char         name[PARTITION_NAME_LEN]; //分区名
    unsigned int         ksize;                    //分区大小，K为单位
    unsigned int         start_pos;                //分区的开始位置，字节为单位     
}T_PARTITION_CONFIG;


typedef struct
{
    T_PARTITION_CONFIG        partition_info;
    T_EX_PARTITION_CONFIG     ex_partition_info;
}T_PARTITION_TABLE_INFO;

typedef enum
{
   PART_DATA_TYPE = 0,
   PART_BIN_TYPE,
   PART_FS_TYPE,
}PART_TYPE;


struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode);
		
void spi_flash_free(struct spi_flash *flash);

int spi_partion_index_get_info(T_PARTITION_TABLE_INFO *Partion, unsigned long parts_cnt, char *partname, int *index);

int spi_partition_table_offset_get(struct spi_flash *spi, unsigned long *parts_offset);

int spi_partition_table_content_get(struct spi_flash *spi,unsigned long parts_offset,
									unsigned int *Partion,
									unsigned long *g_parts
									);




static inline int spi_flash_read(struct spi_flash *flash, u32 offset,
		size_t len, void *buf)
{
	return flash->read(flash, offset, len, buf);
}

static inline int spi_flash_write(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
	return flash->write(flash, offset, len, buf);
}

static inline int spi_flash_erase(struct spi_flash *flash, u32 offset,
		size_t len)
{
	//printf("cdh:offset:0x%x, len:0x%x\n", offset, len);
	return flash->erase(flash, offset, len);
}

void spi_boot(void) __noreturn;



#endif /* _SPI_FLASH_H_ */
