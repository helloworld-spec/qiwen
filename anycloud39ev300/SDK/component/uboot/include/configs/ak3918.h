/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Versatile PB.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

//#define DEBUG 1   /*FIXME:use for debug.*/
// cdh:add for 
//#define CONFIG_ETHERNET_MAC_MII 1
#define CONFIG_ETHERNET_MAC_RMII 1
//#define CONFIG_SYS_RX_ETH_BUFFER 256

#define CONFIG_ANYKA_WATCHDOG   1
/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM926EJS	1	/* This is an arm926ejs CPU core */
#define CONFIG_AK3918		1	/* in Versatile Platform Board	*/
#define CONFIG_ARCH_AK39	1	/* Specifically, a Versatile	*/
#define CONFIG_PLAT_ANYKA 	1
#define CONFIG_USE_IRQ      0       // cdh:add for ethernet interrupt
#define CONFIG_BOARD_EARLY_INIT_F
// #define CONFIG_DELAY_ENVIRONMENT 1  // cdh:add for environment delay 

/*use for arch/arm/cpu/arm926ejs/start.S */
/*read uboot from 512 bytes offset pos in spi flash 0~511 is irom param.*/
#define CONFIG_SYS_TEXT_BASE   0x80dffe00

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS        1          /* we have 1 bank of DRAM */
#define CONFIG_SYS_SDRAM_BASE       0x80000000
#define PHYS_SDRAM_SIZE             (64*1024*1024) /* 64MB of DRAM */
#define CONFIG_SYS_SDRAM_PROTECT_SIZE       0x01000000 /* 16MB of DRAM */
/*
 * Stack sizes
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(8*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif
/*
 * Stack should be on the SRAM because
 * DRAM is not init ,(arch/arm/lib/crt0.S)
 */
#define CONFIG_SYS_INIT_SP_ADDR		(0x80ff0000 - GENERATED_GBL_DATA_SIZE)

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 4096*1024)


/*
 * input clock of PLL
 */
/* ak39 has 12.0000MHz input clock */
#define CONFIG_SYS_CLK_FREQ	12000000

#define CONFIG_SYS_HZ  		(1000)

/*
 * cdh:add print version info
 */
#define CONFIG_VERSION_VARIABLE 1	

/*
 * Flash config
 */
// cdh:#define CONFIG_SYS_NO_FLASH

/*
 *-----------------------------------------------------------
 * | 0x0 - 0x1FF 		|(2 pages)   chip rom param  
 * -----------------------------------------------------
 * | 0x200 - 0x1FFFF    |(556 pages) u-boot
 * -----------------------------------------------------// 558 page
 * | 0x22000 - 0x220FF  | (1 pages) partition info
 * -----------------------------------------------------
 * | 0x22100 - 0x221FF  | (1 pages) config info
 * -----------------------------------------------------// 560 page
 * | 0x22200 - 0x2221FF | (8192 pages) kernel
 * -----------------------------------------------------
 * -----------------------------------------------------// 8752 page
 * | 0xxxx00 - 0xxxFF | (m pages) MAC (4KB)16page
 * -----------------------------------------------------
 * -----------------------------------------------------// 8768 page
 * | 0xxxx00 - 0xxxxFF | (n pages) serial number(4KB)16page
 * -----------------------------------------------------//8784 page
 * | 0x20000 - 0x21FFF  | (4 pages) env
 * ----------------------------------------------------- //8800 page
 * -----------------------------------------------------
 * | 0xxxx00 - 0xxxxFF | (n pages) fs 
 * -----------------------------------------------------
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *-----------------------------------------------------------
 * */


/* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_FLASH_BASE		0x0

/* 0x0 - 0x2page-1*/
#define CONFIG_CHIPROM_PARAM_OFFSET (CONFIG_SYS_FLASH_BASE)
#define CONFIG_CHIPROM_PARAM_SIZE  	(512)

/* 0x2page - 0x558page-1*/
#define CONFIG_UBOOT_OFFSET  		\
	(CONFIG_CHIPROM_PARAM_OFFSET + CONFIG_CHIPROM_PARAM_SIZE)
#define CONFIG_UBOOT_SIZE  			(204288 - 512) // (183808 - 512) // cdh:(142848 - 512)

/* 0x558page - 0x559page-1 */
#define CONFIG_PART_OFFSET 			\
	(CONFIG_UBOOT_OFFSET + CONFIG_UBOOT_SIZE)
#define CONFIG_PART_SIZE 			(256)

/* 0x559page - 0x560page-1 */
#define CONFIG_CFG_OFFSET 			\
	(CONFIG_PART_OFFSET + CONFIG_PART_SIZE)
#define CONFIG_CFG_SIZE 			(256)

/* 0x560page - 0x8752page -1*/
#define CONFIG_KERNEL_OFFSET 		\
	(CONFIG_CFG_OFFSET + CONFIG_CFG_SIZE)
#define CONFIG_KERNEL_SIZE 			(2*1024*1024)

/* 0x8752page - 8768page-1 */
#define CONFIG_FLASH_MAC_OFFSET  			\
	(CONFIG_KERNEL_OFFSET + CONFIG_KERNEL_SIZE) 
#define CONFIG_FLASH_MAC_SIZE  			(4*1024)

/* 8768page - 8784page-1 */
#define CONFIG_FLASH_SER_OFFSET  			\
	(CONFIG_FLASH_MAC_OFFSET + CONFIG_FLASH_MAC_SIZE) 
#define CONFIG_FLASH_SER_SIZE  			(4*1024)


/* enviroment variable offset and size in spi nor flash. According to customer's requirement,it can be modified.*/
#define CONFIG_ENV_OFFSET  			0x237000
	
#define CONFIG_ENV_SIZE  			(4*1024)

#define CONFIG_CMD_FLASH   1  // cdh:add
#define CONFIG_AK39_SPI
#define CONFIG_SPI_XFER_CPU
//#define CONFIG_SPI_XFER_DMA
#define CONFIG_SPI_FLASH
//#define CONFIG_CMD_SPI       // cdh:close
#define CONFIG_CMD_SF
#define CONFIG_CMD_READCFG
#define CONFIG_CMD_OPSPRTIF    // CDH:ADD
#define CONFIG_SPI_FLASH_ANYKA 		1
//#define CONFIG_SPI_FLASH_BUSWIDTH_4X 		1 // cdh:for spi bus width 4 wires
#define CONFIG_CMD_SF_TEST 	// cdh:add for flash test
#define CONFIG_SYS_MAX_FLASH_SECT 	8192 // 256	
//#define CONFIG_CMD_NAND         // cdh:add
#define CONFIG_SYS_MAX_NAND_DEVICE  1  // cdh:add
#define CONFIG_SYS_NAND_BASE      0x20120000   // cdh:add
#define CONFIG_SPIFLASH_USE_FAST_READ 1

#define CONFIG_SPI_FLASH_BAR		1

/* File systems cdh:add */
//#define CONFIG_CMD_MTDPARTS
//#define CONFIG_MTD_DEVICE
//#define CONFIG_MTD_PARTITIONS
//#if defined(CONFIG_CMD_MTDPARTS) || defined(CONFIG_CMD_NAND)
//#define MTDIDS_DEFAULT		"nor0=GD128MB-norflash"
//#define MTDPARTS_DEFAULT	"mtdparts=GD128MB-norflash:256k@0(bios)"
//#endif


/* ATAG configuration */
//#define CONFIG_INITRD_TAG
//#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE

/* MMC */
#define CONFIG_CMD_FAT    1
#define CONFIG_PARTITIONS 1
#define CONFIG_DOS_PARTITION		1
#define CONFIG_MMC     1
//#define HAVE_BLOCK_DEVICE		1
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC		1
#define CONFIG_ANYKA_SDHSMMC		1   // cdh:add

/*
 * Miscellaneous configurable options
 */
#define CONFIG_LONGHELP
#define CONFIG_SYS_LONGHELP	1  // cdh:add
#define CONFIG_SYS_PROMPT	"\nanyka$"
#define CONFIG_SYS_CBSIZE	256
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	32
#define CONFIG_SYS_BARGSIZE	CONFIG_SYS_CBSIZE

/* default load address	*/
#define CONFIG_SYS_LOAD_ADDR		0x82008000//0x81008000

/*
 * Check if key already pressed
 * Don't check if bootdelay < 0
 */
#define CONFIG_ZERO_BOOTDELAY_CHECK
#define CONFIG_BOOTDELAY 	3

#define CONFIG_CFG_DEF_ADDR 		0x81000000
#define CONFIG_CFG_FILE_CNT 		0x8  //  define the max cnt, because all totale 256B, and every unit 32B

#define CONFIG_ENV_SPI_BUS	0
#define CONFIG_ENV_SPI_CS	0
#define CONFIG_ENV_SPI_MAX_HZ	20000000
#define CONFIG_ENV_SPI_MODE	SPI_MODE_0

#define CONFIG_SF_DEFAULT_BUS	0
#define CONFIG_SF_DEFAULT_CS	0
#define CONFIG_SF_DEFAULT_SPEED	20000000
#define CONFIG_SF_DEFAULT_MODE	SPI_MODE_0

#define CONFIG_ROOT_NAME 	"A"
#define CONFIG_BIOS_NAME 	"KERNEL"
#define CONFIG_ENV_NAME 	"ENV" // cdh:初步测试使用serial number分区做ENV使用

#define CONFIG_SECTION_SIZE 4096    // cdh:add for erase section size
#define CONFIG_PARATION_TAB_SIZE 512    // cdh:add for paration table section size

#define CONFIG_SPIP_START_PAGE 0    // cdh:add for searching SPIP flag start flash offset address
#define CONFIG_SPIP_PAGE_CNT  3     // cdh:add for searching SPIP flag
#define CONFIG_PARATION_PAGE_CNT  2     // cdh:add for searching  paration table page cnt


#define CONFIG_ENV_IS_IN_SPI_FLASH 	1
//#define CONFIG_ENV_ADDR			CONFIG_ENV_OFFSET
#define CONFIG_ENV_SECT_SIZE 	CONFIG_ENV_SIZE

#define CONFIG_CMD_MTDPARTS  /* Enable MTD parts commands    */
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_LOAD

/* customer need detecting gpio to meet their self-defined requirement. */
#define CONFIG_AK39_GPIO

//#define CONFIG_TF_UPDATE

/* According to customer different requirement,we defined three kind of partition table:
** 1.First one is partition table defined by anyka. Saved in partition env by anyka burntool. 
** 2.Second one is partition table defined by customer. Saved in partition env by nor flash programer,beacause
** customer don't wanna modify uboot code.
** 3.Third one is partition table defined by customer self-defined. Saved in uboot code,so if you wanna change
** partition table, you have to modify uboot code.
** You can only choose one from below three.
*/

#define CONFIG_PARTITION_TABLE_BY_ANYKA 
//#define CONFIG_PARTITION_TABLE_BY_CUSTOMER_ENV 
//#define CONFIG_PARTITION_TABLE_BY_SELF_DEFINED 

#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
#define CONFIG_BOOTARGS "console=ttySAK0,115200n8 root=/dev/mtdblock4 rootfstype=squashfs init=/sbin/init " \
                        "mem=64M " \
                        "memsize=64M"

#endif 

#ifdef CONFIG_PARTITION_TABLE_BY_SELF_DEFINED
#define CONFIG_BOOTARGS "console=ttySAK0,115200n8 root=/dev/mtdblock4 "\
			"rootfstype=squashfs init=/sbin/init mem=64M memsize=64M mtdparts=spi0.0:2048K@0x36000(KERNEL)," \
	"1136K@0x238000(ROOT),500K@0x354000(USR.JFFS2),4200K@0x3d1000(USR.SQUASH),"	\
	"4K@0x236000(MAC),4K@0x237000(ENV)"

#endif

/* Obtain partition table by mtdparts */
#ifdef CONFIG_PARTITION_TABLE_BY_CUSTOMER_ENV
#define CONFIG_BOOTARGS "console=ttySAK0,115200n8 root=/dev/mtdblock4 rootfstype=squashfs init=/sbin/init " \
                        "mem=64M " \
                        "memsize=64M"
#define  MTDIDS_DEFAULT  "nor0=spi0.0"
#endif
#define CONFIG_DEFAULT_MTDPARTS \	
    "mtdparts=mtdparts=spi0.0:200K@0x0(uboot),2048K@0x36000(KERNEL)," \
    "1136K@0x238000(ROOT),500K@0x354000(USR.JFFS2),4200K@0x3d1000(USR.SQUASH)," \
    "4K@0x236000(MAC),4K@0x237000(ENV)\0"\



#define CONFIG_EXTRA_ENV_SETTINGS \
	"sf_hz=20000000\0" \
	"kernel_addr=0x36000\0" \
	"kernel_size=0x200000\0" \
	"fs_addr=0x230000\0" \
	"loadaddr=0x82208000\0" \
	"console=ttySAK0,115200n8\0" \
	"mtd_root=/dev/mtdblock4\0" \
	"init=/sbin/init\0" \
	"memsize=64M\0" \
	"rootfstype=squashfs\0" \
	"ethaddr=00:55:7b:b5:7d:f7\0" \
	"setcmd=setenv bootargs console=${console} root=${mtd_root} rootfstype=${rootfstype} init=${init} mem=${memsize}\0" \
	"read_kernel=sf probe 0:0 ${sf_hz} 0; sf read ${loadaddr} ${kernel_addr} ${kernel_size}\0" \
	"boot_normal=readcfg; run read_kernel; bootm ${loadaddr}\0" /*go ${loadaddr}*/\
	"bootcmd=run boot_normal\0" \
	"vram=12M\0" \
	"update_flag=0\0" \
	"erase_env=sf probe 0:0 ${sf_hz} 0; sf erase 0x20000 0x2000\0"/* erase env, use for test.*/ \

#define CONFIG_CMD_RUN 
#define CONFIG_CMD_MEMORY 
#define CONFIG_CMD_SAVEENV



/*
 * Hardware drivers
 */
#define CONFIG_AK39_SERIAL
#define CONFIG_SERIAL1
#define CONFIG_AK_ETHER
#define CONFIG_CMD_NET 
#define CONFIG_CMD_PING


/* Define default IP addresses */
#define CONFIG_IPADDR		192.168.1.99	/* own ip address */
#define CONFIG_SERVERIP		192.168.1.1	/* used for tftp (not nfs?) */
#define CONFIG_NETMASK      255.255.255.0    

/* valid baudrates */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_BAUDRATE		115200


#if 0
/* SPI flash. */
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SPL_SPI_BUS		0
#define CONFIG_SPL_SPI_CS		0
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x20000

#define CONFIG_SPL_LDSCRIPT		"$(CPUDIR)/am33xx/u-boot-spl.lds"

#endif

/*-----------------------------------------------------------------------
 * FLASH configuration
 */
//#define CONFIG_ICH_SPI
//#define CONFIG_SPI_FLASH
//#define CONFIG_SPI_FLASH_MACRONIX
//#define CONFIG_SPI_FLASH_WINBOND
//#define CONFIG_SPI_FLASH_GIGADEVICE
//#define CONFIG_SYS_NO_FLASH
//#define CONFIG_CMD_SF
//#define CONFIG_CMD_SF_TEST
//#define CONFIG_CMD_SPI
//#define CONFIG_SPI




/*
 * For NOR boot, we must set this to the start of where NOR is mapped
 * in memory.
 */
#ifdef CONFIG_NOR_BOOT
#define CONFIG_SYS_TEXT_BASE		0x08000000

#endif

#endif	/* __CONFIG_H */
