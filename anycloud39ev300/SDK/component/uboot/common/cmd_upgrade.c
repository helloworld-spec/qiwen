/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Serial up- and download support
 */
#include <common.h>
#include <command.h>

#include <jffs2/load_kernel.h>


#include <s_record.h>
#include <net.h>
#include <exports.h>
#include <xyzModem.h>

#include <malloc.h>
#include <spi_flash.h>
#include <asm/arch-ak39/sysctl.h>
#include <asm/arch-ak39/anyka_cpu.h>

DECLARE_GLOBAL_DATA_PTR;

#define PARTION_UBOOT        "u-boot.bin"
#define PARTION_KERNEL  	 "uImage"
#define PARTION_ROOTSQSH4FS  "root.sqsh4"
#define PARTION_USRSQSH4FS   "usr.sqsh4"
#define PARTION_USRJFFS2FS   "usr.jffs2"


#define PARTION_BOOT_NAME    "BOOT"
#define PARTION_KERNEL_NAME  "KERNEL"
#define PARTION_MAC_NAME   	 "MAC"
#define PARTION_ENV_NAME     "ENV"
#define PARTION_ROOTSQSH4FS_NAME  "A"
#define PARTION_USRJFFS2FS_NAME   "B"
#define PARTION_USRSQSH4FS_NAME   "C"

#define PARTION_UBOOT_NUMBER   0
#define PARTION_KERNEL_NUMBER  1
#define PARTION_ROOTFS_NUMBER  2
#if defined(CONFIG_PARTITION_TABLE_BY_CUSTOMER_ENV)
#define PARTION_MAC_NAME       3
#define PARTION_ENV_NAME       4
#define PARTION_USRJFFS2FS_NUMBER   5
#define PARTION_USRSQSH4FS_NUMBER   6
#else
#define PARTION_USRJFFS2FS_NUMBER   3
#define PARTION_USRSQSH4FS_NUMBER   4
#endif

#define UBOOT_4KMULTIPLE(S)    ((((S)&0x00000FFF)==0)?(S):((((S)>>12)+1)<<12))

#if defined(CONFIG_PARTITION_TABLE_BY_ANYKA)
extern T_PARTITION_TABLE_INFO *gPartion;
extern unsigned long g_nr_parts;
#endif

#if defined(CONFIG_PARTITION_TABLE_BY_CUSTOMER_ENV)
extern int mtdparts_init(void);
extern struct mtd_device *current_mtd_dev;
struct part_info* mtd_part_info(struct mtd_device *dev, unsigned int part_num);
#endif


#if defined(CONFIG_PARTITION_TABLE_BY_SELF_DEFINED)
static Partion gPartion8M[] =
{
    {PARTION_UBOOT,    0x00000,    0x33000,   PARTION_BOOT_NAME},
    {PARTION_KERNEL,   0x36000,    0x200000,  PARTION_KERNEL_NAME}, 
    {PARTION_ROOTSQSH4FS,   0x238000,   0x11c000,  PARTION_ROOTSQSH4FS_NAME},
    {PARTION_USRJFFS2FS,    0x354000,   0x7d000,  PARTION_USRJFFS2FS_NAME},
    {PARTION_USRSQSH4FS,    0x3d1000,   0x41a000,   PARTION_USRSQSH4FS_NAME},
};

static Partion gPartion16M[] =
{
    {PARTION_UBOOT,    0x00000,    0x33000,   PARTION_BOOT_NAME},
    {PARTION_KERNEL,   0x36000,    0x200000,  PARTION_KERNEL_NAME},
    {PARTION_ROOTSQSH4FS,   0x238000,   0x11c000,  PARTION_ROOTSQSH4FS_NAME},
    {PARTION_USRJFFS2FS,    0x354000,   0x7d000,  PARTION_USRJFFS2FS_NAME},
    {PARTION_USRSQSH4FS,    0x3d1000,   0x41a000,   PARTION_USRSQSH4FS_NAME},
};

static Partion *gPartion = gPartion8M;
Partion *partion_table_get_info(void)
{
    int size = spi_flash_get_size();

    if (size == 8*1024*1024) {
        gPartion = gPartion8M;
    } else if (size == 16*1024*1024) {
        gPartion = gPartion16M;
    }
    printf("SF: %dM \n",size/1024/1024);
    return gPartion;
}
#endif


/*If SD_DET==0, it means TF card has been inserted. */
unsigned char TF_card_detect(unsigned int pin)
{

	unsigned int TF_detect;	

    g_ak39_setpin_as_gpio(pin);
    g_ak39_gpio_set_mode(pin, 0);
    TF_detect = g_ak39_gpio_get_value(pin);

    return TF_detect;  
}

#define USB_INTR_RESET            0x04
#define USB_POWER_HSENABLE  			0x20
#define USB_POWER_ENSUSPEND             0x01
#define		AK_FALSE			0
#define		AK_TRUE				1

unsigned char usb_detect()
{
    unsigned int i;
    unsigned char stat = AK_FALSE;
    unsigned int tmp;

    /* enable clock, USB PLL, USB 2.0, CLOCK_CTRL_REG:0x0800001C*/
    REG32(CLOCK_CTRL_REG) &= (~(1<<CLOCK_USB_ENABLE));

    /*reset usb controller      RESET_CTRL_REG: 0x08000020*/
	REG32(RESET_CTRL_REG) |= (1<<RESET_USB_OTG);
	REG32(RESET_CTRL_REG) &= (~(1<<RESET_USB_OTG));
    
    /*usb id config      USB_CONTROL_REG: 0x08000058 */
    REG32(USB_CONTROL_REG) |= (0x3<<12);
    /*enable device */
   	REG32(USB_CONTROL_REG) &= (~(0x3f<<6)); 
    REG32(USB_CONTROL_REG) |= (0x17<<6);
    
    /*Enable the usb transceiver and suspend enable */
    REG32(USB_CONTROL_REG) &= (~0x7); 
    REG32(USB_CONTROL_REG) |= (0x4); 

    /*USB_REG_POWER: 0x20200001*/
    REG8(USB_REG_POWER) = USB_POWER_ENSUSPEND | USB_POWER_HSENABLE;

    for(i=0; i<2000000; i++)
    {
        /*USB_REG_INTRUSB: 0x2020000A*/
        if(REG8(USB_REG_INTRUSB)&USB_INTR_RESET)
        {
            stat = AK_TRUE;
            printf("usb detect %d\n", stat);
            break;
        }
    }

    printf("usb detect %d\n", stat);
    //this delay is necessary when host is mac or linux,otherwise it will be no usb reset before enum when phy is opened again
    mdelay(500);
    
    return(stat);
}


/*
 * below all functions are for tftp upgrading image.
 */
static int down_ethnet_partion(int part_number, char *partfile,char *partname)
{
    char cmd_buf[200] = {0,}; 
    unsigned int addrDram = 0x82000000;
    unsigned int filesize = 0;
    int ret = 0;
	int index = 0;
    int size;	
	int start;

#ifdef CONFIG_PARTITION_TABLE_BY_CUSTOMER_ENV
	if (mtdparts_init() !=0)
	    return 1;
	
    start  = mtd_part_info(current_mtd_dev, part_number)->offset;
	size   = mtd_part_info(current_mtd_dev, part_number)->size;   
    printf("start : 0x%08x size :0x%08x\n",start,size);

    memset(cmd_buf,0,sizeof(cmd_buf));
    sprintf(cmd_buf, "tftp 0x%x %s",addrDram, partfile);
    ret = run_command(cmd_buf, 0);
    /* ret == 0, success */
    if (!ret){
    	filesize = simple_strtol(getenv("filesize"), NULL, 16);
		if (filesize > 0){
            memset(cmd_buf, 0, sizeof(cmd_buf));
            sprintf(cmd_buf, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;sf write 0x%x 0x%x 0x%lx",
                start,
                size,
                addrDram,
                start,
                filesize);
            if(filesize > size){
				printf("Err:download size beyond partition size!\n");	
            }else{
	        	run_command(cmd_buf, 0);
        	}
        }
        
    }
    
#endif

#ifdef CONFIG_PARTITION_TABLE_BY_SELF_DEFINED
	partion_table_get_info();
#endif

#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
    ret = spi_partion_index_get_info(gPartion, g_nr_parts, partname, &index);
    if (ret == -1){
		printf("search partition index failed!\n");
		return ret;
    }else{
		debug("search partition index = %d!\n", index);
    }
#endif

    memset(cmd_buf,0,sizeof(cmd_buf));

    sprintf(cmd_buf, "tftp 0x%x %s", addrDram, partfile);
	printf("cmd: %s\n", cmd_buf);
    ret = run_command(cmd_buf, 0);
    /* ret == 0, success */
#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
    if (!ret){
    	//filesize = UBOOT_4KMULTIPLE(simple_strtol(getenv("filesize"), NULL, 16));
    	filesize = simple_strtol(getenv("filesize"), NULL, 16);
		if (filesize > 0){
            memset(cmd_buf, 0, sizeof(cmd_buf));
            sprintf(cmd_buf, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;sf write 0x%x 0x%x 0x%lx",
                gPartion[index].partition_info.start_pos,
                gPartion[index].partition_info.ksize * 1024,
                addrDram,
                gPartion[index].partition_info.start_pos,
                filesize);
            if(filesize > (gPartion[index].partition_info.ksize * 1024)){
				printf("Err:download size beyond partition size!\n");	
            }else{
	        	run_command(cmd_buf, 0);
        	}
            /*update kernel size of anyka partition table */
            if (0 == strncmp(gPartion[index].partition_info.name, CONFIG_BIOS_NAME, PARTITION_NAME_LEN))
            {
                sprintf(cmd_buf, "updatecfg kernel 0x%x",filesize);
                printf("[%s] cmd: %s\n", __func__,  cmd_buf);
                run_command(cmd_buf, 0);
            }  
         }
    }
#endif
#ifdef CONFIG_PARTITION_TABLE_BY_SELF_DEFINED
    /* ret == 0, success */
    if (!ret) {
        filesize = simple_strtol(getenv("filesize"), NULL, 16);
        if (filesize > 0) {
            memset(cmd_buf,0,sizeof(cmd_buf));
            sprintf(cmd_buf, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;sf write 0x%x 0x%x 0x%x",
                gPartion[part_number].offset,
                gPartion[part_number].size,
                addrDram,
                gPartion[part_number].offset,
                filesize);
			printf("cmd: %s\n", cmd_buf);
        	run_command(cmd_buf, 0);
        }
    }
#endif

    return 0;
}

static int down_ethnet_partion_for_uboot(int part_number, char *partfile)
{
    char cmd_buf[200] = {0,}; 
    unsigned int addrDram = 0x82000000;
    unsigned int filesize = 0;
    unsigned int len;
    int ret = 0;
	
    int size;	
	int start;
    unsigned long parts_offset = NULL;
    struct spi_flash *spi = NULL;

    spi = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
			CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
	if (!spi) {
		printf("%s: spi_flash probe error.\n", __func__);
		ret = -1;
		return ret;
	}

#ifdef CONFIG_PARTITION_TABLE_BY_CUSTOMER_ENV
	if (mtdparts_init() !=0)
	    return 1;
	
    start  = mtd_part_info(current_mtd_dev, part_number)->offset;
	size   = mtd_part_info(current_mtd_dev, part_number)->size;   
    printf("start : 0x%08x size :0x%08x\n",start,size);

    memset(cmd_buf,0,sizeof(cmd_buf));
    sprintf(cmd_buf, "tftp 0x%x %s",addrDram, partfile);
    ret = run_command(cmd_buf, 0);
    /* ret == 0, success */
    if (!ret){
    	filesize = simple_strtol(getenv("filesize"), NULL, 16);
		if (filesize > 0){
            /* fill 512 byte data into the head of new uboot-bin  */
            len = spi->page_size*2; 
            ret = spi_flash_read(spi, 0, len, addrDram);
        	if (ret) {
        		printf("%s: spi_flash read error.\n", __func__);
        		spi_flash_free(spi);
        		return 1;
        	}
            /*update uboot size of 512 byte,base on it's real size */
            memcpy((void *)(addrDram+12), &filesize, 4);
            memset(cmd_buf, 0, sizeof(cmd_buf));
            sprintf(cmd_buf, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;sf write 0x%x 0x%x 0x%lx",
                start,
                size,
                addrDram,
                start,
                filesize);
            if(filesize > size){
				printf("Err:download size beyond partition size!\n");	
            }else{
	        	run_command(cmd_buf, 0);
        	}
        }
        
    }
    
#endif

#ifdef CONFIG_PARTITION_TABLE_BY_SELF_DEFINED
	partion_table_get_info();
#endif

#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA

    /* get partition table offset */
    if (spi_partition_table_offset_get(spi, &parts_offset)){
		printf("get partition table offset failed!\n");
		ret = -1;
		return ret;
	}
#endif

    memset(cmd_buf,0,sizeof(cmd_buf));

    sprintf(cmd_buf, "tftp 0x%x %s", addrDram, partfile);
	printf("cmd: %s\n", cmd_buf);
    ret = run_command(cmd_buf, 0);
    /* ret == 0, success */
#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
    if (!ret){
    	filesize = simple_strtol(getenv("filesize"), NULL, 16);
		if (filesize > 0){
            /* fill 512 byte data into the head of new uboot-bin  */
            len = spi->page_size*2; 
            ret = spi_flash_read(spi, 0, len, addrDram);
        	if (ret) {
        		printf("%s: spi_flash read error.\n", __func__);
        		spi_flash_free(spi);
        		return 1;
        	}
            /*update uboot size of 512 byte,base on it's real size */
            memcpy((void *)(addrDram+12), &filesize, 4);
            printf("filesize = 0x%x , parts_offset= 0x%x\n",filesize,parts_offset);
            memset(cmd_buf, 0, sizeof(cmd_buf));
            sprintf(cmd_buf, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;sf write 0x%x 0x%x 0x%lx",
                0,
                parts_offset,
                addrDram,
                0,
                filesize);
            printf("\ncmd_buf=%s\n",cmd_buf);
            if(filesize > (parts_offset)){
				printf("Err:download size beyond partition size!\n");	
            }else{
	        	run_command(cmd_buf, 0);
        	}
        }
        
    }
#endif
#ifdef CONFIG_PARTITION_TABLE_BY_SELF_DEFINED
    /* ret == 0, success */
    if (!ret) {
        filesize = simple_strtol(getenv("filesize"), NULL, 16);
        if (filesize > 0) {
            /* fill 512 byte data into the head of new uboot-bin  */
            len = spi->page_size*2; 
            ret = spi_flash_read(spi, 0, len, addrDram);
        	if (ret) {
        		printf("%s: spi_flash read error.\n", __func__);
        		spi_flash_free(spi);
        		return 1;
        	}
            /*update uboot size of 512 byte,base on it's real size */
            memcpy((void *)(addrDram+12), &filesize, 4);
            memset(cmd_buf,0,sizeof(cmd_buf));
            sprintf(cmd_buf, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;sf write 0x%x 0x%x 0x%x",
                gPartion[part_number].offset,
                gPartion[part_number].size,
                addrDram,
                gPartion[part_number].offset,
                filesize);
			printf("cmd: %s\n", cmd_buf);
        	run_command(cmd_buf, 0);
        }
    }
#endif

    return 0;
}

static int do_down_uboot(cmd_tbl_t * cmdtp, int flag,
	   	int argc, char * const argv[])
{
	return down_ethnet_partion_for_uboot(PARTION_UBOOT_NUMBER,PARTION_UBOOT);
}

static int do_down_kernel(cmd_tbl_t * cmdtp, int flag,
	   	int argc, char * const argv[])
{
	return down_ethnet_partion(PARTION_KERNEL_NUMBER,PARTION_KERNEL,PARTION_KERNEL_NAME);
}

static int do_down_root_fs(cmd_tbl_t * cmdtp, int flag,
	   	int argc, char * const argv[])
{
    return down_ethnet_partion(PARTION_ROOTFS_NUMBER,PARTION_ROOTSQSH4FS,PARTION_ROOTSQSH4FS_NAME);
}

static int do_down_usr_squashfs(cmd_tbl_t * cmdtp, int flag,
	   	int argc, char * const argv[])
{
    return down_ethnet_partion(PARTION_USRSQSH4FS_NUMBER,PARTION_USRSQSH4FS,PARTION_USRSQSH4FS_NAME);
}

static int do_down_usr_jffs2fs(cmd_tbl_t * cmdtp, int flag,
	   	int argc, char * const argv[])
{
    return down_ethnet_partion(PARTION_USRJFFS2FS_NUMBER,PARTION_USRJFFS2FS,PARTION_USRJFFS2FS_NAME);
}

int do_down_image(cmd_tbl_t * cmdtp, int flag,
	   	int argc, char * const argv[])
{
    printf("download and write kernel, filesys to flash!\n");
    do_down_kernel(NULL, 0, 0, NULL);
    do_down_root_fs(NULL, 0, 0, NULL);
    do_down_usr_squashfs(NULL, 0, 0, NULL);
    do_down_usr_jffs2fs(NULL, 0, 0, NULL);
    return 0;
}

U_BOOT_CMD(
	downkernel, 2, 1,	do_down_kernel,
	"load "PARTION_KERNEL" tftp",
	"    - write "PARTION_KERNEL" to flash"
);


U_BOOT_CMD(
	downrootfs, 2, 1,	do_down_root_fs,
	"load "PARTION_ROOTSQSH4FS" tftp",
	"    - write "PARTION_ROOTSQSH4FS" to flash"
);



U_BOOT_CMD(
	downsquashfs, 2, 1, do_down_usr_squashfs,
	"load "PARTION_USRSQSH4FS" tftp",
	"    - write "PARTION_USRSQSH4FS" to flash"
);



U_BOOT_CMD(	downjffs2fs, 2, 1, do_down_usr_jffs2fs,
	"load "PARTION_USRJFFS2FS" tftp",
	"    - write "PARTION_USRJFFS2FS" to flash"
);

U_BOOT_CMD(
	downuboot, 2, 1,	do_down_uboot,
	"load uboot tftp",
	"    - write uboot to flash"
);

U_BOOT_CMD(downimage, 2, 1, do_down_image,
	"downimage   - download and write All-Image to FLASH device,partiton table from ENV partition.",
	"   - write all images to FLASH device"
);


/*
 * below all functions are for TF upgrading image.
 */
static int down_and_update_mmc(int part_number, char *partfile,char *partname)
{
	char cmd[200] = {0};
	unsigned int filesize = 0;
	unsigned int addr = 0x80000000;
	int ret = 0;
    int size;	
	int start;
    int index = 0;

#ifdef CONFIG_PARTITION_TABLE_BY_CUSTOMER_ENV
    if (mtdparts_init() !=0)
	    return 1;
	
    start  = mtd_part_info(current_mtd_dev, part_number)->offset;
	size   = mtd_part_info(current_mtd_dev, part_number)->size;   
    printf("start : 0x%08x size :0x%08x\n",start,size);
    
	/* file load */
	sprintf(cmd, "fatload mmc 0 0x%x %s", addr, partfile);
	printf("[%s]  cmd: %s\n", __func__,  cmd);
	ret = run_command(cmd, 0);
    /* ret == 0, success */
	if (!ret) {
    	filesize = simple_strtol(getenv("filesize"), NULL, 16);
		/* compare file size with partition-size */
		if (filesize > size * 1024) {
			printf("[%s] error,  filesz: %u beyond partsz: %u\n",
				   	__func__,  filesize, partfile);
			return -1;
		}
		/* file check ok, do flash operation */
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;"
				"sf write 0x%x 0x%x 0x%lx",
                start,
                size,
                addr,
                start,
                size);
		printf("cmd=%s\n", cmd);
		ret = run_command(cmd, 0);
	}
	return ret;
#endif

#ifdef CONFIG_PARTITION_TABLE_BY_SELF_DEFINED
	partion_table_get_info();
#endif

#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
    ret = spi_partion_index_get_info(gPartion, g_nr_parts, partname, &index);
    if (ret == -1){
		printf("search partition index failed!\n");
		return ret;
    }else{
		debug("search partition index = %d!\n", index);
    }
#endif
    
	/* file load */
	sprintf(cmd, "fatload mmc 0 0x%x %s", addr, partfile);
	printf("[%s] cmd: %s\n", __func__,  cmd);
	ret = run_command(cmd, 0);
    
#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
	/* ret == 0, success */
	if (!ret) {
    	filesize = simple_strtol(getenv("filesize"), NULL, 16);

		/* file check ok, do flash operation */
		memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;sf write 0x%x 0x%x 0x%lx",
                gPartion[index].partition_info.start_pos,
                gPartion[index].partition_info.ksize * 1024,
                addr,
                gPartion[index].partition_info.start_pos,
                filesize);
		debug("cmd=%s\n", cmd);
		ret = run_command(cmd, 0);

        /*update kernel size of anyka partition table */
        if (0 == strncmp(gPartion[index].partition_info.name, CONFIG_BIOS_NAME, PARTITION_NAME_LEN))
        {
            sprintf(cmd, "updatecfg kernel 0x%x",filesize);
            printf("[%s] cmd: %s\n", __func__,  cmd);
            run_command(cmd, 0);
        }          
	}    
#endif
#ifdef CONFIG_PARTITION_TABLE_BY_SELF_DEFINED
	/* ret == 0, success */
	if (!ret) {
    	filesize = simple_strtol(getenv("filesize"), NULL, 16);
		/* compare file size with partition-size */
		if (filesize > gPartion[part_number].size * 1024) {
			printf("[%s] error, filesz: %u beyond partsz: %u\n",
				   	__func__, filesize, gPartion[part_number].partfile);
			return -1;
		}
		/* file check ok, do flash operation */
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;"
				"sf write 0x%x 0x%x 0x%lx",
                gPartion[part_number].offset,
                gPartion[part_number].size,
                addr,
                gPartion[part_number].offset,
                filesize);
		printf("cmd=%s\n", cmd);
		ret = run_command(cmd, 0);
	}    
#endif    

    
}

static int down_and_update_mmc_for_uboot(int part_number, char *partfile)
{
	char cmd[200] = {0};
	unsigned int filesize = 0;
	unsigned int addr = 0x80000000;
    unsigned int len;
	int ret = 0;
    int size;	
	int start;
    
    unsigned long parts_offset = NULL;
    struct spi_flash *spi = NULL;

    spi = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
			CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
	if (!spi) {
		printf("%s: spi_flash probe error.\n", __func__);
		ret = -1;
		return ret;
	}
    
#ifdef CONFIG_PARTITION_TABLE_BY_CUSTOMER_ENV
    if (mtdparts_init() !=0)
	    return 1;
	
    start  = mtd_part_info(current_mtd_dev, part_number)->offset;
	size   = mtd_part_info(current_mtd_dev, part_number)->size;   
    printf("start : 0x%08x size :0x%08x\n",start,size);
    
	/* file load */
	sprintf(cmd, "fatload mmc 0 0x%x %s", addr, partfile);
	printf("[%s]  cmd: %s\n", __func__,  cmd);
	ret = run_command(cmd, 0);
    /* ret == 0, success */
	if (!ret) {
    	filesize = simple_strtol(getenv("filesize"), NULL, 16);
		/* compare file size with partition-size */
		if (filesize > size * 1024) {
			printf("[%s] error,  filesz: %u beyond partsz: %u\n",
				   	__func__,  filesize, partfile);
			return -1;
		}
        /* fill 512 byte data into the head of new uboot-bin  */
        len = spi->page_size*2; 
        ret = spi_flash_read(spi, 0, len, addr);
        if (ret) {
        	printf("%s: spi_flash read error.\n", __func__);
        	spi_flash_free(spi);
        	return 1;
        }
        /*update uboot size of 512 byte,base on it's real size */
        memcpy((void *)(addr+12), &filesize, 4);
        
		/* file check ok, do flash operation */
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;"
				"sf write 0x%x 0x%x 0x%lx",
                start,
                size,
                addr,
                start,
                size);
		printf("cmd=%s\n", cmd);
		ret = run_command(cmd, 0);
	}
	
#endif

#ifdef CONFIG_PARTITION_TABLE_BY_SELF_DEFINED
	partion_table_get_info();
#endif

#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA

    /* get partition table offset */
    if (spi_partition_table_offset_get(spi, &parts_offset)){
		printf("get partition table offset failed!\n");
		ret = -1;
		return ret;
	}
#endif
    
	/* file load */
	sprintf(cmd, "fatload mmc 0 0x%x %s", addr, partfile);
	printf("[%s] cmd: %s\n", __func__,  cmd);
	ret = run_command(cmd, 0);
   
#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
	/* ret == 0, success */
	if (!ret) {
    	filesize = simple_strtol(getenv("filesize"), NULL, 16);

		if (filesize > 0){
            /* fill 512 byte data into the head of new uboot-bin  */
            len = spi->page_size*2; 
            ret = spi_flash_read(spi, 0, len, addr);
        	if (ret) {
        		printf("%s: spi_flash read error.\n", __func__);
        		spi_flash_free(spi);
        		return 1;
        	}
            /*update uboot size of 512 byte,base on it's real size */
            memcpy((void *)(addr+12), &filesize, 4);
            printf("filesize = 0x%x , parts_offset= 0x%x\n",filesize,parts_offset);
            memset(cmd, 0, sizeof(cmd));
            sprintf(cmd, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;sf write 0x%x 0x%x 0x%lx",
                0,
                parts_offset,
                addr,
                0,
                filesize);
            printf("\ncmd_buf=%s\n",cmd);
		    ret = run_command(cmd, 0);
	    } 
    }
#endif
#ifdef CONFIG_PARTITION_TABLE_BY_SELF_DEFINED
	/* ret == 0, success */
	if (!ret) {
    	filesize = simple_strtol(getenv("filesize"), NULL, 16);
		/* compare file size with partition-size */
		if (filesize > gPartion[part_number].size * 1024) {
			printf("[%s] error, filesz: %u beyond partsz: %u\n",
				   	__func__, filesize, gPartion[part_number].partfile);
			return -1;
		}
        
        /* fill 512 byte data into the head of new uboot-bin  */
        len = spi->page_size*2; 
        ret = spi_flash_read(spi, 0, len, addr);
        if (ret) {
        	printf("%s: spi_flash read error.\n", __func__);
        	spi_flash_free(spi);
        	return 1;
        }
        /*update uboot size of 512 byte,base on it's real size */
        memcpy((void *)(addr+12), &filesize, 4);    
		/* file check ok, do flash operation */
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "sf probe 0 20000000 0;sf erase 0x%x 0x%x;"
				"sf write 0x%x 0x%x 0x%lx",
                gPartion[part_number].offset,
                gPartion[part_number].size,
                addr,
                gPartion[part_number].offset,
                filesize);
		printf("cmd=%s\n", cmd);
		ret = run_command(cmd, 0);
	}    
#endif    

    
}


static int do_down_uboot_mmc(cmd_tbl_t * cmdtp, int flag,
	   	int argc, char * const argv[])
{   
	return down_and_update_mmc_for_uboot(PARTION_UBOOT_NUMBER,PARTION_UBOOT);
}

static int do_down_kernel_mmc(cmd_tbl_t * cmdtp, int flag,
	   	int argc, char * const argv[])
{
	return down_and_update_mmc(PARTION_KERNEL_NUMBER,PARTION_KERNEL,PARTION_KERNEL_NAME);
}

static int do_down_root_fs_mmc(cmd_tbl_t * cmdtp, int flag,
	   	int argc, char * const argv[])
{
	return down_and_update_mmc(PARTION_ROOTFS_NUMBER,PARTION_ROOTSQSH4FS,PARTION_ROOTSQSH4FS_NAME);
}

static int do_down_usr_squashfs_mmc(cmd_tbl_t * cmdtp, int flag, 
		int argc, char * const argv[])
{
	return down_and_update_mmc(PARTION_USRSQSH4FS_NUMBER,PARTION_USRSQSH4FS,PARTION_USRSQSH4FS_NAME);
}

static int do_down_usr_jffs2fs_mmc(cmd_tbl_t * cmdtp, int flag, 
		int argc, char * const argv[])
{
	return down_and_update_mmc(PARTION_USRJFFS2FS_NUMBER,PARTION_USRJFFS2FS,PARTION_USRJFFS2FS_NAME);
}

static int do_tf_update_image(cmd_tbl_t * cmdtp, int flag, 
		int argc, char * const argv[])
{
	int ret = 0;

    printf("download all iamge to flash!\n");
    ret = do_down_kernel_mmc(NULL, 0, 0, NULL);
    ret += do_down_root_fs_mmc(NULL, 0, 0, NULL);
    ret += do_down_usr_squashfs_mmc(NULL, 0, 0, NULL);
    ret += do_down_usr_jffs2fs_mmc(NULL, 0, 0, NULL);

    return ret;
}

U_BOOT_CMD(
	tfdownuboot, 2, 1,	do_down_uboot_mmc,
	"load "PARTION_UBOOT"TF",
	"    - write "PARTION_UBOOT"to flash"
);

U_BOOT_CMD(
	tfdownkernel, 2, 1,	do_down_kernel_mmc,
	"load "PARTION_KERNEL "TF",
	"    - write "PARTION_KERNEL "to flash"
);

U_BOOT_CMD(
	tfdownrootfs, 2, 1,	do_down_root_fs_mmc,
	"load "PARTION_ROOTSQSH4FS"TF",
	"    - write "PARTION_ROOTSQSH4FS"to flash"
);

U_BOOT_CMD(
	tfdownsquashfs, 2, 1,	do_down_usr_squashfs_mmc,
	"load "PARTION_USRSQSH4FS"TF",
	"    - write "PARTION_USRSQSH4FS"to flash"
);

U_BOOT_CMD(
	tfdownjffs2fs, 2, 1,	do_down_usr_jffs2fs_mmc,
	"load "PARTION_USRJFFS2FS"TF",
	"    - write "PARTION_USRJFFS2FS"to flash"
);

U_BOOT_CMD(tfupdateimage, 2, 1, do_tf_update_image,
	"tfupdateimage   - download and write All-Image to FLASH device",
	"   - read image from mmc device, write All-Image to FLASH device"
);