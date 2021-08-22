/*
 * Command for accessing SPI flash.
 *
 * Copyright (C) 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <div64.h>
#include <malloc.h>
#include <spi_flash.h>

#include <asm/io.h>

#ifndef CONFIG_SF_DEFAULT_SPEED
# define CONFIG_SF_DEFAULT_SPEED	1000000
#endif
#ifndef CONFIG_SF_DEFAULT_MODE
# define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3
#endif
#ifndef CONFIG_SF_DEFAULT_CS
# define CONFIG_SF_DEFAULT_CS		0
#endif
#ifndef CONFIG_SF_DEFAULT_BUS
# define CONFIG_SF_DEFAULT_BUS		0
#endif

DECLARE_GLOBAL_DATA_PTR;
/* units:bytes. */
#define MAX_SPICFG_LEN  	(1024)
#define MTD_PART_NAME_LEN   (4)

static struct spi_flash *flash;

// cdh:all 32Byte
struct file_config
{
	ulong file_length;
    ulong ld_addr;
    ulong start_page;
	ulong backup_page;        //backup data start page
    char file_name[16];
};

// cdh:all 24Byte
struct partitions
{
	char name[MTD_PART_NAME_LEN]; 
	unsigned long size;
	unsigned long size2;
	unsigned long offset;
	unsigned long offset2;
	unsigned int mask_flags;
};

/*
 * This function computes the length argument for the erase command.
 * The length on which the command is to operate can be given in two forms:
 * 1. <cmd> offset len  - operate on <'offset',  'len')
 * 2. <cmd> offset +len - operate on <'offset',  'round_up(len)')
 * If the second form is used and the length doesn't fall on the
 * sector boundary, than it will be adjusted to the next sector boundary.
 * If it isn't in the flash, the function will fail (return -1).
 * Input:
 *    arg: length specification (i.e. both command arguments)
 * Output:
 *    len: computed length for operation
 * Return:
 *    1: success
 *   -1: failure (bad format, bad address).
 */
static int sf_parse_len_arg(char *arg, ulong *len)
{
	char *ep;
	char round_up_len; /* indicates if the "+length" form used */
	ulong len_arg;

	round_up_len = 0;
	if (*arg == '+') {
		round_up_len = 1;
		++arg;
	}

	len_arg = simple_strtoul(arg, &ep, 16);
	if (ep == arg || *ep != '\0')
		return -1;

	if (round_up_len && flash->sector_size > 0)
		*len = ROUND(len_arg, flash->sector_size);
	else
		*len = len_arg;

	return 1;
}

/**
 * This function takes a byte length and a delta unit of time to compute the
 * approximate bytes per second
 *
 * @param len		amount of bytes currently processed
 * @param start_ms	start time of processing in ms
 * @return bytes per second if OK, 0 on error
 */
static ulong bytes_per_second(unsigned int len, ulong start_ms)
{
	/* less accurate but avoids overflow */
	if (len >= ((unsigned int) -1) / 1024)
		return len / (max(get_timer(start_ms) / 1024, 1));
	else
		return 1024 * len / max(get_timer(start_ms), 1);
}

static int do_spi_flash_probe(int argc, char * const argv[])
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	char *endp;
	struct spi_flash *new;

	if (argc >= 2) {
		cs = simple_strtoul(argv[1], &endp, 0);
		if (*argv[1] == 0 || (*endp != 0 && *endp != ':'))
			return -1;
		if (*endp == ':') {
			if (endp[1] == 0)
				return -1;

			bus = cs;
			cs = simple_strtoul(endp + 1, &endp, 0);
			if (*endp != 0)
				return -1;
		}
	}

	if (argc >= 3) {
		speed = simple_strtoul(argv[2], &endp, 0);
		if (*argv[2] == 0 || *endp != 0)
			return -1;
	}
	if (argc >= 4) {
		mode = simple_strtoul(argv[3], &endp, 16);
		if (*argv[3] == 0 || *endp != 0)
			return -1;
	}

	new = spi_flash_probe(bus, cs, speed, mode);
	if (!new) {
		printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
		return 1;
	}

	if (flash)
		spi_flash_free(flash);
	flash = new;

	return 0;
}

/**
 * Write a block of data to SPI flash, first checking if it is different from
 * what is already there.
 *
 * If the data being written is the same, then *skipped is incremented by len.
 *
 * @param flash		flash context pointer
 * @param offset	flash offset to write
 * @param len		number of bytes to write
 * @param buf		buffer to write from
 * @param cmp_buf	read buffer to use to compare data
 * @param skipped	Count of skipped data (incremented by this function)
 * @return NULL if OK, else a string containing the stage which failed
 */
static const char *spi_flash_update_block(struct spi_flash *flash, u32 offset,
		size_t len, const char *buf, char *cmp_buf, size_t *skipped)
{
	//debug("offset=%#x, sector_size=%#x, len=%#zx\n",
	//      offset, flash->sector_size, len);
	/* Read the entire sector so to allow for rewriting */
	if (spi_flash_read(flash, offset, flash->sector_size, cmp_buf))
		return "read";
	/* Compare only what is meaningful (len) */
	if (memcmp(cmp_buf, buf, len) == 0) {
		//debug("Skip region %x size %zx: no change\n",
		//      offset, len);
		*skipped += len;
		return NULL;
	}
	/* Erase the entire sector */
	if (spi_flash_erase(flash, offset, flash->sector_size))
		return "erase";
	/* Write the initial part of the block from the source */
	if (spi_flash_write(flash, offset, len, buf))
		return "write";
	/* If it's a partial sector, rewrite the existing part */
	if (len != flash->sector_size) {
		/* Rewrite the original data to the end of the sector */
		if (spi_flash_write(flash, offset + len,
				    flash->sector_size - len, &cmp_buf[len]))
			return "write";
	}

	return NULL;
}

/**
 * Update an area of SPI flash by erasing and writing any blocks which need
 * to change. Existing blocks with the correct data are left unchanged.
 *
 * @param flash		flash context pointer
 * @param offset	flash offset to write
 * @param len		number of bytes to write
 * @param buf		buffer to write from
 * @return 0 if ok, 1 on error
 */
static int spi_flash_update(struct spi_flash *flash, u32 offset,
		size_t len, const char *buf)
{
	const char *err_oper = NULL;
	char *cmp_buf;
	const char *end = buf + len;
	size_t todo;		/* number of bytes to do in this pass */
	size_t skipped = 0;	/* statistics */
	const ulong start_time = get_timer(0);
	size_t scale = 1;
	const char *start_buf = buf;
	ulong delta;
	
	//debug("===cdh:update start=offset:0x%x===len:0x%x====\n", offset, len);
	
	if (end - buf >= 200)
		scale = (end - buf) / 100;
		
	cmp_buf = malloc(flash->sector_size);
	if (cmp_buf) {
		ulong last_update = get_timer(0);
		for (; buf < end && !err_oper; buf += todo, offset += todo) {
			todo = min(end - buf, flash->sector_size);
			if (get_timer(last_update) > 100) {
				printf("   \rUpdating, %zu%% %lu B/s",
				       100 - (end - buf) / scale,
					bytes_per_second(buf - start_buf,
							 start_time));
				last_update = get_timer(0);
			}
			err_oper = spi_flash_update_block(flash, offset, todo,
					buf, cmp_buf, &skipped);
		}
	} else {
		err_oper = "malloc";
	}
	
	free(cmp_buf);
	putc('\r');
	if (err_oper) {
		printf("SPI flash update failed in %s step\n", err_oper);
		return 1;
	}

	delta = get_timer(start_time);
	printf("%zu bytes written, %zu bytes skipped", len - skipped,
	       skipped);
	printf(" in %ld.%lds, speed %ld B/s\n",
	       delta / 1000, delta % 1000, bytes_per_second(len, start_time));

	return 0;
}

/**
* @brief get uboot part area infor table 
*
*  get uboot part area infor table in spi norflash for update fs area
* @author CaoDonghua
* @date 2016-09-26
* @param[in] flash	 spiflash handle.
* @param[in] parts part file infor.
* @param[out] pnr_parts get part file cnt.
* @return int return get uboot part area infor table  from flash success or failed
* @retval returns zero on success
* @retval return a non-zero if failed
*/
static int get_parts_info(struct spi_flash *flash, T_PARTITION_TABLE_INFO *parts, unsigned long *pnr_parts)
{
	int ret;
	int i;
	unsigned int len;
	unsigned int offset;
	unsigned char *buf = NULL;
	unsigned long boot_len = 0;//
	unsigned char boot_temp = 0;//
	unsigned long nr_parts;
	unsigned long *addr;
	char paration_table[CONFIG_PARATION_TAB_SIZE];

	debug("%s:read, addr:0x%x\n", __func__, addr);
	
	/*
	* chech argc cnt and intial searching SPIP flag spi flash offset address
	* malloc 3 pages size buffer for seraching SPIP flag 
	*/
	offset = CONFIG_SPIP_START_PAGE; 
	len = flash->page_size*CONFIG_SPIP_PAGE_CNT; 
	buf = (unsigned char *)malloc(len);
	if (NULL == buf) {
        printf("find part table buf malloc fail\n");
        return 1;
    }

	/* clean buffer content */
    memset(buf, 0, len);

    /* read out three pages size content from norflash offset=0 */
    debug("read, offset:0x%x, len:0x%x, addr:0x%x\n", offset, len, buf);
	ret = spi_flash_read(flash, offset, len, buf);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		return 1;
	}

	/* search SPIP flag */
	for (i=0; i<(len - 4); i++) {
		if (('S' == buf[i]) && ('P' == buf[i+1]) && ('I' == buf[i+2]) && ('P' == buf[i+3])) {
			offset = i;
			break;
		}
	}
 
    if (i == (len - 4)) {
       	printf("not find partition start page\n");
        return 1; 
    }

	/* get spi param last reserved member value */
    printf("boot block num offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);

    /* get paration tabel saved block from spi param reserved member value */
    memcpy(&boot_temp, buf + offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, 1);

	/* get paration tabel saved page offset address */
 	boot_len = boot_temp*flash->erase_size;
    printf("get_upadte_boot_size g_boot_len:%d\n", boot_len);

    /*
	* release tmp  paration table buffer
	*/
    if (buf != NULL) {
	    free(buf);
    }

    /* config infor area must be 2 pages align */
	len = flash->page_size*CONFIG_PARATION_PAGE_CNT;
	addr = (unsigned long *)paration_table;
	debug("read config info(%08x) from offset:%08x, len:%d.\n", addr, boot_len, len);
	ret = spi_flash_read(flash, boot_len, len, addr);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		return 1;
	}

	nr_parts = *(volatile unsigned long *)addr;
	addr++;

	/* if no partiton to mount, the buf will be all 0xFF but not constant.
	 * So, it is not safe here. 
	 */
	debug("nr_parts=0x%lx\n", nr_parts);
	if (nr_parts <= 0 || nr_parts > 15) {
		printf("%s:partition count invalid\n",__func__);
		return 1;
	}else {
		*pnr_parts = nr_parts;
	}

	memcpy((void *)parts, (void *)addr, len - 4);
	
	return 0;
}

/**
* @brief  get update uboot kernel config infor 
*
*  get update uboot kernel config infor before update kernel code in spi norflash
* @author CaoDonghua
* @date 2016-09-26
* @param[in] flash	 spiflash handle.
* @param[out] poffset update new kernel code offset in flash.
* @return int return get update uboot kernel config infor to flash success or failed
* @retval returns zero on success
* @retval return a non-zero if failed
*/
static int get_kernel_config_info(struct spi_flash *flash, u32 *poffset, u32 *pbios_size)
{
	int ret;
	unsigned long  i = 0;
	unsigned long *addr;
	unsigned int len;
	unsigned int offset;
	unsigned char *buf = NULL;
	unsigned char boot_temp = 0;
	unsigned long boot_len = 0;
	unsigned long nr_parts;
	T_PARTITION_TABLE_INFO *cfg;
	char paration_table[CONFIG_PARATION_TAB_SIZE];
	
	/*
	* chech argc cnt and intial searching SPIP flag spi flash offset address
	* malloc 3 pages size buffer for seraching SPIP flag 
	*/
	offset = CONFIG_SPIP_START_PAGE; 
	len = flash->page_size*CONFIG_SPIP_PAGE_CNT; 
	buf = (unsigned char *)malloc(len);
	if (NULL == buf) {
		printf("find part table buf malloc fail\n");
		return 1;
	}

	/* clean buffer content */
	memset(buf, 0, len);

	/* from norflash offset=0, read out three pages size content */
	debug("read, offset:0x%x, len:0x%x, addr:0x%x\n", offset, len, buf);
	ret = spi_flash_read(flash, offset, len, buf);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		return 1;
	}

	/* search SPIP flag */
	for (i=0; i<(len - 4); i++) {
		if (('S' == buf[i]) && ('P' == buf[i+1]) && ('I' == buf[i+2]) && ('P' == buf[i+3])) {
			offset = i;
			break;
		}
	}
 
	if (i == (len - 4)) {
		printf("not find partition start page\n");
		return 1; 
	}

	/* get spi param last reserved member value */
    printf("boot block num offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);

    /* get paration tabel saved block from spi param reserved member value */
    memcpy(&boot_temp, buf + offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, 1);

	/* get paration tabel saved page offset address */
 	boot_len = boot_temp*flash->erase_size;
    printf("get_upadte_boot_size g_boot_len:%d\n", boot_len);

    /*
	* release tmp  paration table buffer
	*/
    if (buf != NULL) {
	    free(buf);
    }
	
	/* config infor area must be 2 pages align */
	len = flash->page_size*CONFIG_PARATION_PAGE_CNT;
	addr = (unsigned long *)paration_table;
	debug("read config info(%08x) from offset:%08x, len:%d.\n", addr, boot_len, len);
	ret = spi_flash_read(flash, boot_len, len, addr);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		return 1;
	}

	nr_parts = *(volatile unsigned long *)addr;
	addr++;

	/* if no partiton to mount, the buf will be all 0xFF but not constant.
	 * So, it is not safe here. 
	 */
	debug("nr_parts=0x%lx\n", nr_parts);
	if (nr_parts <= 0 || nr_parts > 15) {
		printf("%s:partition count invalid\n",__func__);
		return 1;
	}
	
	for (i=0; i<nr_parts; i++) {
		cfg = (T_PARTITION_TABLE_INFO *)(addr + i*sizeof(T_PARTITION_TABLE_INFO) / sizeof(unsigned long));
		if (0 == strncmp(cfg->partition_info.name, CONFIG_BIOS_NAME, PARTITION_NAME_LEN)) {
			printf("search file %s, success\n", cfg->partition_info.name);
		
			/* get kernel paration offset address */
			*poffset = cfg->partition_info.start_pos; 

			/* get kernel paration size */
			*pbios_size = cfg->partition_info.ksize * 1024; 
			
			debug("cdh:update kernel offset, kernel_offsetstartpage:0x%x, kernel_getoffset:0x%x\n", cfg->partition_info.start_pos, *poffset);	
			break;
		}			
	}

	/* if not find valid kernel config infor, must return err  */
	if (nr_parts == i) {
		printf("cdh:read cfg file Err!\n");
		return 1;
	}

	return 0;
}


/**
* @brief update uboot kernel config infor 
*
*  update uboot kernel config infor after update kernel code in spi norflash
* @author CaoDonghua
* @date 2016-09-26
* @param[in] flash	 spiflash handle.
* @param[in] len update new kernel code size.
* @return int return update uboot kernel config infor to flash success or failed
* @retval returns zero on success
* @retval return a non-zero if failed
*/
static int set_kernel_config_info(struct spi_flash *flash, size_t bioslen)
{
	int ret;
	unsigned int len;
	unsigned int offset;
	unsigned char *buf = NULL;
	unsigned char boot_temp = 0;
	unsigned long boot_len = 0;
	unsigned long nr_parts;
	unsigned long  i = 0;
	unsigned long *addr;
	T_PARTITION_TABLE_INFO *cfg;
	T_BIN_CONFIG binbuf;
	char paration_table[CONFIG_SECTION_SIZE];
	
	/*
	* check argc cnt and intial searching SPIP flag spi flash offset address
	* malloc 3 pages size buffer for seraching SPIP flag 
	*/
	offset = CONFIG_SPIP_START_PAGE; 
	len = flash->page_size*CONFIG_SPIP_PAGE_CNT; 
	buf = (unsigned char *)malloc(len);
	if (NULL == buf) {
		printf("find part table buf malloc fail\n");
		return 1;
	}

	/* clean buffer content */
	memset(buf, 0, len);

	
	/* from norflash offset=0, read out three pages size content */
	debug("read, offset:0x%x, len:0x%x, addr:0x%x\n", offset, len, buf);
	ret = spi_flash_read(flash, offset, len, buf);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		return 1;
	}

	/* search SPIP flag */
	for (i=0; i<(len - 4); i++) {
		if (('S' == buf[i]) && ('P' == buf[i+1]) && ('I' == buf[i+2]) && ('P' == buf[i+3])) {
			offset = i;
			break;
		}
	}
 
	if (i == (len - 4)) {
		printf("not find partition start page\n");
		return 1; 
	}


	/* get spi param last reserved member value */
    printf("boot block num offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);

    /* get paration tabel saved block from spi param reserved member value */
    memcpy(&boot_temp, buf + offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, 1);

	/* get paration tabel saved page offset address */
 	boot_len = boot_temp * flash->erase_size;
    debug("get_upadte_boot_size g_boot_len:%d\n", boot_len);

    /*
	* release tmp  paration table buffer
	*/
    if (buf != NULL) {
	    free(buf);
    }

	/* config infor area must be 4k size align */
	len = CONFIG_SECTION_SIZE;
	addr = (unsigned long *)paration_table;
	debug("read config info(%08x) from offset:%08x, len:%d.\n", addr, boot_len, len);
	ret = spi_flash_read(flash, boot_len, len, addr);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		return 1;
	}

	nr_parts = *(volatile unsigned long *)addr;
	addr++;

	/* if no partition to mount, the buf will be all 0xFF but not constant.
	 * So, it is not safe here. 
	 */
	debug("nr_parts=0x%lx\n", nr_parts);
	if (nr_parts <= 0 || nr_parts > 15) {
		printf("%s:partition count invalid\n",__func__);
		return 1;
	}
	
	/* update kernel length */
	for(i=0; i<nr_parts; i++) {
		cfg = (T_PARTITION_TABLE_INFO *)(addr + i*sizeof(T_PARTITION_TABLE_INFO) / sizeof(unsigned long));
		if (0 == strncmp(cfg->partition_info.name, CONFIG_BIOS_NAME, PARTITION_NAME_LEN)) {
			printf("search file %s, success\n", cfg->partition_info.name);
			memcpy(&binbuf, &cfg->ex_partition_info, sizeof(T_EX_PARTITION_CONFIG));//获取到的值是块数
			debug("cdh:update kernel size,old_size:%d, new_size:%d\n", binbuf.file_length, bioslen);
			binbuf.file_length = bioslen; // cdh:update kernel size
			memcpy(&cfg->ex_partition_info, &binbuf, sizeof(T_EX_PARTITION_CONFIG));//获取到的值是块数
			setenv_hex("kernel new size", binbuf.file_length);	
			break;
		}			
	}

	/* if not find valid kernel config infor, must return err  */
	if (nr_parts == i) {
		printf("cdh:read cfg file Err!\n");
		return 1;
	}

	/* 
	*  after update kernel config infor, we must save to flash
	*  because this step is very important, so we retry three times
	*/
	if (spi_flash_erase(flash, boot_len, CONFIG_SECTION_SIZE)) {
		printf("Check Erase config infor index:%d, failed\n", i);
		return 1;
	}

	if (spi_flash_write(flash, boot_len, CONFIG_SECTION_SIZE, paration_table)) {
		printf("Check Write config infor index:%d, failed\n", i);
		return 1;
	}
	
	return 0;
}

/**
 * Update an fs area of SPI flash by erasing whole area
 * 
 * to change fs data, must first erase whole area. 
 * @param[in] flash	flash context pointer
 * @param[in] offset	flash offset to write
 * @param[in] len	number of bytes to write
 * @param[in] buf	buffer to write from
 * @return int return fs update in flash success or failed
 * @retval returns zero on success
 * @retval return a non-zero if failed 
 */
static int spi_flash_fs_update(struct spi_flash *flash, char *pname, 
		size_t len, const char *buf)
{
	const char *err_oper = NULL;
	const char *end = buf + len;
	const char *start_buf = buf;
	char *cmp_buf;
	size_t scale = 1;
	size_t todo;		/* number of bytes to do in this pass */
	const ulong start_time = get_timer(0);
	int  i;
	unsigned long fs_spisize = 0;
	unsigned long delta;
	unsigned long nr_parts;
	T_PARTITION_TABLE_INFO *parts;
	u32 offset = 0;
	
	if (end - buf >= 200)
		scale = (end - buf) / 100;
	
	/* malloc config infor table size */ 
	parts = (T_PARTITION_TABLE_INFO *)malloc(CONFIG_PARATION_TAB_SIZE);
	if (NULL == parts) {
		printf("malloc parts info eare failed\n");
		return 1;
	}

	/* get part file cnt and part address */ 
	if (get_parts_info(flash, parts, &nr_parts)) {
		printf("get parts info failed\n");
		return 1;
	}

	/* search will be update area and size according to update offset */ 
	for (i=0; i<nr_parts; i++) {
		printf("parts[%d]:\nname = %s\nsize = 0x%lx\noffset = 0x%lx\nr_w_flags = 0x%x\n\n",
					i, 
					parts[i].partition_info.name, 
					parts[i].partition_info.ksize * 1024, 
					parts[i].partition_info.start_pos, 
					parts[i].partition_info.r_w_flag);
		if (strcmp(pname, parts[i].partition_info.name) == 0) {
			/* get fs paration size, notes not old fs img size */
			fs_spisize = parts[i].partition_info.ksize * 1024; 

			/* get fs norflash start page offset address */
			offset = parts[i].partition_info.start_pos;
			debug("cdh:find selected part name:%s, size:0x%x, OK\n", parts[i].partition_info.name, fs_spisize);
			break;
		}
	}

	if (i == nr_parts) {
		printf("find selected part size failed\n");
		return 1;
	}

	
	/* Consistency checking cdh:分区容量检查*/
	if (len > fs_spisize) {
		printf("ERROR: attempting %s past flash size (%#x)\n", pname, fs_spisize);
		return 1;
	}
	
	/*
	* cdh:更新文件系统，为了提高性能，建议使用64KB擦除
	* to update fs, we must first erase all whole fs area
	*/ 
	for (i=0; i<fs_spisize/flash->sector_size; i++) {
		/* Erase the entire sector */
		if (spi_flash_erase(flash, offset+i*flash->sector_size, flash->sector_size) != 0){
			printf("erase part section index:0x%x, failed\n", offset+i*flash->sector_size);
			return 1;
		}
	}
	
	/* cdh:update 文件系统 */ 
	unsigned long last_update = get_timer(0);
	for ( ; buf<end && !err_oper; buf+=todo, offset+=todo) {
		todo = min(end - buf, flash->sector_size);
		if (get_timer(last_update) > 100) {
			printf("   \rUpdating, %zu%% %lu B/s",
			       100 - (end - buf) / scale,
				bytes_per_second(buf - start_buf,
						 start_time));
			last_update = get_timer(0);
		}

		/* Write the initial part of the block from the source */
		if (spi_flash_write(flash, offset, todo, buf) != 0) {
			err_oper = "write";
			printf("write part section index:0x%x, failed\n", offset);
			return 1;
		}
	}

	free(parts);
	
	putc('\r');
	if (err_oper) {
		printf("==spi flash norflash fs update in %s step fail==\n", err_oper);
		return 1;
	}

	//printf("==spi flash norflash fs update ok==\n");
	
	return 0;
}

static int do_spi_flash_read_write(int argc, char * const argv[])
{
	unsigned long addr;
	unsigned long offset;
	unsigned long len;
	void *buf;
	char *endp;
	int ret = 1;
	u32 bios_size;
	u32 tmpvalue;
	
	if (argc < 4)
		return -1;
		
	//printf("cdh:%s, %s, %s\n", __func__, argv[0], argv[1]);
	
	if (strncmp(argv[0], "read", 4) == 0 ||
			strncmp(argv[0], "write", 5) == 0) {
		int read;

		
		addr = simple_strtoul(argv[1], &endp, 16);
		if (*argv[1] == 0 || *endp != 0)
			return -1;
		offset = simple_strtoul(argv[2], &endp, 16);
		if (*argv[2] == 0 || *endp != 0)
			return -1;
		len = simple_strtoul(argv[3], &endp, 16);
		if (*argv[3] == 0 || *endp != 0)
			return -1;

		buf = map_physmem(addr, len, MAP_WRBACK);
		if (!buf) {
			puts("Failed to map physical memory\n");
			return 1;
		}

#if 0
		/* cdh:add check addr whether beyond UBOOT mem eare range, protect UBOOT memory */
		if (addr > (CONFIG_SYS_SDRAM_BASE + gd->ram_size - CONFIG_SYS_SDRAM_PROTECT_SIZE)){
			printf("** Err %s, address beyond uboot range **\n", __func__);
			return 1;
		}
#endif		
		
		read = strncmp(argv[0], "read", 4) == 0;
		if (read)
			ret = spi_flash_read(flash, offset, len, buf);
		else
			ret = spi_flash_write(flash, offset, len, buf);

		printf("\nSF: %zu bytes @ %#x %s: %s\n", (size_t)len, (u32)offset,
		       read ? "Read" : "Written", ret ? "ERROR" : "OK");
	}

	unmap_physmem(buf, len);

	return ret == 0 ? 0 : 1;
}

static int do_spi_flash_update(int argc, char * const argv[])
{
	unsigned long addr;
	unsigned long offset;
	unsigned long len;
	void *buf;
	char *endp;
	int ret = 1;
	u32 bios_size;
	u32 tmpvalue;
	
	if (argc < 4)
		return -1;
		
	debug("cdh:%s, %s, %s, cnt:%d\n", __func__, argv[0], argv[1], argc);
	
	if (strcmp(argv[0], "update") == 0) {
		/* we add name param, so addr, offset and len move to next param */
		addr = simple_strtoul(argv[1], &endp, 16);
		if (*argv[1] == 0 || *endp != 0)
			return -1;		
		len = simple_strtoul(argv[3], &endp, 16);
		if (*argv[3] == 0 || *endp != 0)
			return -1;

		buf = map_physmem(addr, len, MAP_WRBACK);
		if (!buf) {
			puts("Failed to map physical memory\n");
			return -1;
		}

		/* cdh:add check addr whether beyond UBOOT mem eare range, protect UBOOT memory */
		if (addr > (CONFIG_SYS_SDRAM_BASE + gd->ram_size - CONFIG_SYS_SDRAM_PROTECT_SIZE)){
			printf("** Err %s, address beyond uboot range **\n", __func__);
			return -1;
		}
		
		
		/* update fs , kenrel and other data to spi norflash 
		* name (argv[2]) = fs:     update fs image area
		* name (argv[2]) = bios:  update kernel image  area
		* name (argv[2]) = other: update other data area
		*/
		debug("cdh:%s, argv[1]:%s\n", __func__, argv[1]);
		if (strcmp(argv[2], CONFIG_BIOS_NAME) == 0){
			printf("\n==Start update bios==\n");
			
			/* update spi norflash kernel area */
			ret = get_kernel_config_info(flash, &offset, &bios_size);
			if (ret){
				printf("get kernel config infor offset in flash Err\n");
				ret = 1;
				goto usage;
			}

			/* Consistency checking cdh:分区容量检查*/
			if (len > bios_size) {
				printf("ERROR: attempting %s past flash size (%#x)\n", "bios", (bios_size/4096)*4096);
				ret = 1;
				goto usage;
			}
			
			ret  = spi_flash_update(flash, offset, len, buf);
			if (0 == ret) {
				/* after update kernel code ok, we must update kernel config info */
				ret = set_kernel_config_info(flash, len);
			}

			if (ret) {
				printf("\n==End update kernel Err!==\n");
			}else {
				printf("\n==End update kernel OK!==\n");
			}
		}else {
			printf("\n==Start update FS==\n");
			debug("cdh:up fs ok name:%s\n", argv[2]);
			
			ret = spi_flash_fs_update(flash, argv[2], len, buf);
			if (ret) {
				printf("\n==End update FS Err!==\n");
			}else {
				printf("\n==End update FS OK!==\n");
			}
		} 
		
	}

usage:
	unmap_physmem(buf, len);

	return ret == 0 ? 0 : 1;
}

static int do_spi_flash_erase(int argc, char * const argv[])
{
	unsigned long offset;
	unsigned long len;
	char *endp;
	int ret;

	if (argc < 3)
		return -1;

	offset = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;

	ret = sf_parse_len_arg(argv[2], &len);
	if (ret != 1)
		return -1;

	/* Consistency checking */
	if (offset + len > flash->size) {
		printf("ERROR: attempting %s past flash size (%#x)\n",
		       argv[0], flash->size);
		return 1;
	}

	ret = spi_flash_erase(flash, offset, len);
	printf("SF: %zu bytes @ %#x Erased: %s\n", (size_t)len, (u32)offset,
	       ret ? "ERROR" : "OK");

	return ret == 0 ? 0 : 1;
}

#ifdef CONFIG_CMD_SF_TEST
enum {
	STAGE_ERASE,
	STAGE_CHECK,
	STAGE_WRITE,
	STAGE_READ,

	STAGE_COUNT,
};

static char *stage_name[STAGE_COUNT] = {
	"erase",
	"check",
	"write",
	"read",
};

struct test_info {
	int stage;
	int bytes;
	unsigned base_ms;
	unsigned time_ms[STAGE_COUNT];
};

static void show_time(struct test_info *test, int stage)
{
	uint64_t speed;	/* KiB/s */
	int bps;	/* Bits per second */

	speed = (long long)test->bytes * 1000;
	do_div(speed, test->time_ms[stage] * 1024);
	bps = speed * 8;

	printf("%d %s: %d ticks, %d KiB/s %d.%03d Mbps\n", stage,
	       stage_name[stage], test->time_ms[stage],
	       (int)speed, bps / 1000, bps % 1000);
}

static void spi_test_next_stage(struct test_info *test)
{
	test->time_ms[test->stage] = get_timer(test->base_ms);
	show_time(test, test->stage);
	test->base_ms = get_timer(0);
	test->stage++;
}

/**
 * Run a test on the SPI flash
 *
 * @param flash		SPI flash to use
 * @param buf		Source buffer for data to write
 * @param len		Size of data to read/write
 * @param offset	Offset within flash to check
 * @param vbuf		Verification buffer
 * @return 0 if ok, -1 on error
 */
static int spi_flash_test(struct spi_flash *flash, uint8_t *buf, ulong len,
			   ulong offset, uint8_t *vbuf)
{
	struct test_info test;
	int i;

	printf("SPI flash test:\n");
	memset(&test, '\0', sizeof(test));
	test.base_ms = get_timer(0);
	test.bytes = len;
	if (spi_flash_erase(flash, offset, len)) {
		printf("test Erase failed\n");
		return -1;
	}else {
		printf("test Erase OK\n");
	}
	spi_test_next_stage(&test);

	if (spi_flash_read(flash, offset, len, vbuf)) {
		printf("test Check read failed\n");
		return -1;
	}else {
		printf("test Check read OK\n");
	}
	
	for (i=0; i<len; i++) {
		if (vbuf[i] != 0xff) {
			printf("Check failed at %d\n", i);
			print_buffer(i, vbuf + i, 1, min(len - i, 0x40), 0);
			return -1;
		}
	}

	printf("test Erase Check read OK\n");
	
	spi_test_next_stage(&test);

	if (spi_flash_write(flash, offset, len, buf)) {
		printf("test Write failed\n");
		return -1;
	}else {
		printf("test Write OK\n");
	}
	
	memset(vbuf, '\0', len);
	spi_test_next_stage(&test);

	if (spi_flash_read(flash, offset, len, vbuf)) {
		printf("test back Read failed\n");
		return -1;
	}else {
		printf("test back Read OK\n");
	}
	
	spi_test_next_stage(&test);

	for (i = 0; i < len; i++) {
		if (buf[i] != vbuf[i]) {
			printf("Verify failed at %d, good data:\n", i);
			print_buffer(i, buf + i, 1, min(len - i, 0x40), 0);
			printf("Bad data:\n");
			print_buffer(i, vbuf + i, 1, min(len - i, 0x40), 0);
			return -1;
		}
	}
	printf("###===Test passed===###\n");
	for (i = 0; i < STAGE_COUNT; i++)
		show_time(&test, i);

	return 0;
}

static int do_spi_flash_test(int argc, char * const argv[])
{
	unsigned long offset;
	unsigned long len;
	uint8_t *buf = (uint8_t *)CONFIG_SYS_TEXT_BASE;
	char *endp;
	uint8_t *vbuf;
	int ret;

	offset = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;
	len = simple_strtoul(argv[2], &endp, 16);
	if (*argv[2] == 0 || *endp != 0)
		return -1;

	vbuf = malloc(len);
	if (!vbuf) {
		printf("Cannot allocate memory\n");
		return 1;
	}
	buf = malloc(len);
	if (!buf) {
		free(vbuf);
		printf("Cannot allocate memory\n");
		return 1;
	}

	memcpy(buf, (char *)CONFIG_SYS_TEXT_BASE, len);
	ret = spi_flash_test(flash, buf, len, offset, vbuf);
	free(vbuf);
	free(buf);
	if (ret) {
		printf("###===Test failed===###\n");
		return 1;
	}

	return 0;
}
#endif /* CONFIG_CMD_SF_TEST */

static int do_spi_flash(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	const char *cmd;
	int ret;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	--argc;
	++argv;
	debug("cdh:%s, %s, %s\n", __func__, argv[0], argv[1]);
	
	if (strcmp(cmd, "probe") == 0) {
		ret = do_spi_flash_probe(argc, argv);
		goto done;
	}

	/* The remaining commands require a selected device */
	if (!flash) {
		puts("No SPI flash selected. Please run `sf probe'\n");
		return 1;
	}

	debug("cdh start:%s, %s, %s\n", __func__, argv[0], argv[1]);
	if (strcmp(cmd, "read") == 0 || strcmp(cmd, "write") == 0) {
	    debug("cdh rwup:%s, %s, %s, cnt:%d\n", __func__, argv[0], argv[1], argc);
		ret = do_spi_flash_read_write(argc, argv);
	}
	else if (strcmp(cmd, "update") == 0)
		ret = do_spi_flash_update(argc, argv);
	else if (strcmp(cmd, "erase") == 0)
		ret = do_spi_flash_erase(argc, argv);
#ifdef CONFIG_CMD_SF_TEST
	else if (!strcmp(cmd, "test"))
		ret = do_spi_flash_test(argc, argv);
#endif
	else
		ret = -1;

done:
	if (ret != -1)
		return ret;

usage:
	return CMD_RET_USAGE;
}

#ifdef CONFIG_CMD_SF_TEST
#define SF_TEST_HELP "\nsf test offset len		" \
		"- run a very basic destructive test"
#else
#define SF_TEST_HELP
#endif

U_BOOT_CMD(
	sf,	6,	1,	do_spi_flash,
	"spi flash sub-system:",
	"probe [[bus:]cs] [hz] [mode]	- init flash device on given SPI bus\n"
	"				  and chip select\n"
	"sf read addr offset len	- read `len' bytes starting at\n"
	"				  `offset' to memory at `addr'\n"
	"sf write addr offset len	- write `len' bytes from memory\n"
	"				  at `addr' to flash at `offset'\n"
	"sf erase offset [+]len		- erase `len' bytes from `offset'\n"
	"				  `+len' round up `len' to block size\n"
	"sf update addr name len	- erase and write `len' bytes from memory\n"
	"				  at `addr' to flash at `offset'"
	SF_TEST_HELP
);

int spi_flash_get_size(void)
{
    int size = 0;
    if (!flash)
    {
        do_spi_flash_probe(1,0);
    }
    if (flash) 
    {
        size = flash->size;
    }
    return size;
}