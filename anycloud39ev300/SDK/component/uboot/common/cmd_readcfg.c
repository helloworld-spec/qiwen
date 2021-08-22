#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include <asm/io.h>
/* units:bytes. */
#define MAX_SPICFG_LEN  	(1024)
#define MTD_PART_NAME_LEN   (4)

/*call this function only when using anyka's own partition table.*/
static int do_readcfg(cmd_tbl_t *cmdtp, int flag, int argc, 
		char * const argv[])
{
#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
	struct spi_flash *spi;
	T_PARTITION_TABLE_INFO *cfg;
	unsigned int offset;
	unsigned int *addr;
	unsigned int len, filecnt;
	int ret, i;
	char *endp;
	unsigned char *buf = NULL;
	unsigned long boot_len = 0;
	unsigned char boot_temp = 0;
	T_BIN_CONFIG binbuf;
	unsigned long nr_parts;
	T_PARTITION_TABLE_INFO *parts = NULL;
	char paration_table[CONFIG_PARATION_TAB_SIZE];
	char root_name[] = "/dev/mtdblock";
	char root_block[16];
	char *ptmp_env;
	char *ptmp_env2;
	
	/*
	* spi probe flash
	*/
	spi = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
		   	CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!spi) {
		printf("%s: spi_flash probe error.\n", __func__);
		return 1;
	}

	/*
	* chech argc cnt and intial searching SPIP flag spi flash offset address
	* malloc 3 pages size buffer for seraching SPIP flag 
	*/
	if(argc < 2) {
		offset = CONFIG_SPIP_START_PAGE; 
		len = spi->page_size*CONFIG_SPIP_PAGE_CNT; 
		buf = (unsigned char *)malloc(len);
		if(NULL == buf) {
	        printf("find part table buf malloc fail\n");
	        spi_flash_free(spi);
	        return 1;
	    }
	}else {
		printf("%s: too cmd arguments error.\n", __func__);
		spi_flash_free(spi);
		return 1;
	}

	/* clean buffer content */
    memset(buf, 0, len);

    /* from norflash offset=0, read out three pages size content */
    debug("read, offset:0x%x, len:0x%x, addr:0x%x\n", offset, len, buf);
	ret = spi_flash_read(spi, offset, len, buf);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		spi_flash_free(spi);
		return 1;
	}

	/* search SPIP flag */
	for (i=0; i<(len-4); i++) {
		if (('S' == buf[i]) && ('P' == buf[i+1]) && ('I' == buf[i+2]) && ('P' == buf[i+3])) {
			offset = i;
			break;
		}
	}
 
    if (i == (len - 4)) {
       	printf("not find partition start page\n");
        return 1; 
    }

    /* get spi param last reserved member value*/
    debug("boot block num offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);

    /* get paration tabel saved block from spi param reserved member value */
    memcpy(&boot_temp, buf + offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, 1);

	/* get paration tabel saved page offset address */
 	boot_len = boot_temp*spi->erase_size;
    debug("get_upadte_boot_size g_boot_len:%d\n", boot_len);

    /*
	* release tmp  paration table buffer
	*/
    if (buf != NULL) {
	    free(buf);
	    buf = NULL;
    }

	/* read out paration table from  boot_len */
	len = spi->page_size*CONFIG_PARATION_PAGE_CNT;
	addr = (unsigned long *)paration_table;
	debug("read config info(%08x) from offset:%08x, len:%d.\n", addr, boot_len, len);
	ret = spi_flash_read(spi, boot_len, len, addr);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		spi_flash_free(spi);
		return 1;
	}

	nr_parts = *(unsigned long *)addr;
	addr++;
	
	/* if no partiton to mount, the buf will be all 0xFF but not constant.
	 * So, it is not safe here. 
	 */
	debug("nr_parts=0x%lx\n", nr_parts);
	if (nr_parts <= 0 || nr_parts > 15) {
		printf("%s:partition count invalid\n",__func__);
		spi_flash_free(spi);
		return 1;
	}
    
	/*
	* search BIOS address offset and length
	*/
	// parts = (T_PARTITION_TABLE_INFO *)addr; 
	debug("size part=%d Byte\n", sizeof(T_PARTITION_TABLE_INFO));
	debug("====Creating.. %d MTD partitions on:%s====\n", nr_parts, spi->name);
	for (i=0; i<nr_parts; i++) {
		cfg = (T_PARTITION_TABLE_INFO *)(addr + i*sizeof(T_PARTITION_TABLE_INFO) / sizeof(unsigned long));
		if (0 == strcmp(cfg->partition_info.name, CONFIG_ROOT_NAME)) {
			debug("search file %s, success\n", cfg->partition_info.name);	
			sprintf(root_block, "%d ", (i+1));
			strcat(root_name, root_block);
			setenv("mtd_root", root_name);
			debug("mtd_root:%s \n", getenv("mtd_root"));
		}
	}
	
	for (i=0; i<nr_parts; i++) {
		cfg = (T_PARTITION_TABLE_INFO *)(addr + i*sizeof(T_PARTITION_TABLE_INFO) / sizeof(unsigned long));
		if (0 == strncmp(cfg->partition_info.name, CONFIG_BIOS_NAME, PARTITION_NAME_LEN)) {
			debug("search file %s, success\n", cfg->partition_info.name);	
			memcpy(&binbuf, &cfg->ex_partition_info, sizeof(T_EX_PARTITION_CONFIG));
			debug(" ##kernel_size:0x%08x\n ##loadaddr:0x%08x\n ##kernel_addr:%lu\n ##offset:0x%08x.\n",
					binbuf.file_length, 
					binbuf.ld_addr, 
					cfg->partition_info.start_pos, 
					cfg->partition_info.start_pos);
			debug(" ##backup_page:%lu, file_name:%s.\n", binbuf.backup_page, cfg->partition_info.name);

			setenv_hex("loadaddr", binbuf.ld_addr);
			setenv_hex("kernel_addr", cfg->partition_info.start_pos);
			setenv_hex("kernel_size", binbuf.file_length);
			setenv_hex("backuppage", binbuf.backup_page);

			spi_flash_free(spi);
			return 0;
		}           
	}

	spi_flash_free(spi);
usage:
	return 1;
#endif
}

U_BOOT_CMD(
	readcfg, 4, 0, do_readcfg,  		
	"read config from config.", 
	"readcfg [addr] [offset] [len]"	
);


static int do_writecfg(cmd_tbl_t *cmdtp, int flag, int argc, 
		char * const argv[])
{
	struct spi_flash *spi;
	T_PARTITION_TABLE_INFO *cfg;
    /*for backup partition table . */
    T_PARTITION_TABLE_INFO *backup_cfg;
	unsigned int offset;
	unsigned int *addr;
    /*for backup partition table . */
    unsigned int *backup_addr;
	unsigned int len, filecnt;
	int ret, i;

	char config_infor[CONFIG_SECTION_SIZE];
    /*backup partition table information. */
    char backup_config_infor[CONFIG_SECTION_SIZE];
	unsigned int  *config_addr;
	unsigned long file_len = 0;
	char *endp;
	unsigned char *buf = NULL;
	unsigned long boot_len = 0;//
	unsigned char boot_temp = 0;//
	T_BIN_CONFIG binbuf;
    /*for backup partition table . */
    T_BIN_CONFIG backup_binbuf;
    /*for backup partition table . */
    unsigned long backup_nr_parts;
	unsigned long nr_parts;
	
	/* need at least two arguments */
	if (argc < 3) {
		return 1;
	}

	debug("cdh:%s, 0:%s, 1:%s, 2:%s,cnt:%d\n", __func__, argv[0], argv[1], argv[2], argc);

	/* get update config file name char string pointer */

	file_len = simple_strtoul(argv[2], &endp, 16);
	if (*argv[2] == 0 || *endp != 0) {
		return -1;
	}
	
	/* spi probe flash */
	spi = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
		   	CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!spi) {
		printf("%s: spi_flash probe error.\n", __func__);
		return 1;
	}

	/*
	* chech argc cnt and intial searching SPIP flag spi flash offset address
	* malloc 3 pages size buffer for seraching SPIP flag 
	*/
	offset = CONFIG_SPIP_START_PAGE; 
	len = spi->page_size*CONFIG_SPIP_PAGE_CNT; 
	buf = (unsigned char *)malloc(len);
	if(NULL == buf) {
        printf("find part table buf malloc fail\n");
        spi_flash_free(spi);
        return 1;
    }

	/* clean buffer content */
    memset(buf, 0, len);

    /* from norflash offset=0, read out three pages size content */
    debug("read, offset:0x%x, len:0x%x, addr:0x%x\n", offset, len, buf);
	ret = spi_flash_read(spi, offset, len, buf);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		spi_flash_free(spi);
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
    debug("boot block num offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);

    /* get paration tabel saved block from spi param reserved member value */
    memcpy(&boot_temp, buf + offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, 1);

	/* get paration tabel saved page offset address */
 	boot_len = boot_temp*spi->erase_size;
    debug("get_upadte_boot_size g_boot_len:%d\n", boot_len);

    /*
	* release tmp  paration table buffer
	*/
    if (buf != NULL) {
	    free(buf);
    }
    
	/* config infor area must be 4KB align */
	addr = (unsigned int *)config_infor;  
	/* for backup partition table. */
	backup_addr = (unsigned int *)backup_config_infor;  
	len = CONFIG_SECTION_SIZE;
	debug("read config info(%08x) from offset:%08x, len:%d.\n", addr, boot_len, len);
	
	/* first read out 4K config infor area */ 
	ret = spi_flash_read(spi, boot_len, len, addr);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		goto usage;
	}

    /* first read out 4K config infor area from backup partitioin table.*/ 
	ret = spi_flash_read(spi, boot_len+CONFIG_SECTION_SIZE, len, backup_addr);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		goto usage;
	}

	nr_parts = *(unsigned long *)addr;
	addr++;
    /* for backup partitioin table */
    backup_nr_parts = *(unsigned long *)backup_addr;
    backup_addr++;
	
	/* if no partiton to mount, the buf will be all 0xFF but not constant.
	 * So, it is not safe here. 
	 */
	debug("nr_parts=0x%lx\n", nr_parts);
	if (nr_parts <= 0 || nr_parts > 15) {
		printf("%s:partition count invalid\n",__func__);
		spi_flash_free(spi);
		return 1;
	}

	if (backup_nr_parts <= 0 || backup_nr_parts > 15) {
		printf("%s:partition count invalid\n",__func__);
		spi_flash_free(spi);
		return 1;
	}    

	debug("size part=%d Byte\n", sizeof(T_PARTITION_TABLE_INFO));
	debug("Creating %d MTD partitions on:%s\n", nr_parts, spi->name);

	/* get vaild config file infor  */
	for (i=0; i<nr_parts; i++) {
		cfg = (T_PARTITION_TABLE_INFO *)(addr + i*sizeof(T_PARTITION_TABLE_INFO) / sizeof(unsigned long));
		debug("read file %s.\n", cfg->partition_info.name);
		if (0 == strncmp(cfg->partition_info.name, CONFIG_BIOS_NAME, PARTITION_NAME_LEN)) {
			printf("search file %s, success\n", cfg->partition_info.name);
			memcpy(&binbuf, &cfg->ex_partition_info, sizeof(T_EX_PARTITION_CONFIG));//获取到的值是块数
			binbuf.file_length = file_len; // cdh:update kernel size
			memcpy(&cfg->ex_partition_info, &binbuf, sizeof(T_EX_PARTITION_CONFIG));//获取到的值是块数
			setenv_hex("kernel_size", binbuf.file_length);
			break;
		}			
	}

	/* get vaild config file infor from backup partition table. */
	for (i=0; i<nr_parts; i++) {
		backup_cfg = (T_PARTITION_TABLE_INFO *)(backup_addr + i*sizeof(T_PARTITION_TABLE_INFO) / sizeof(unsigned long));
		debug("read file %s.\n", backup_cfg->partition_info.name);
		if (0 == strncmp(backup_cfg->partition_info.name, CONFIG_BIOS_NAME, PARTITION_NAME_LEN)) {
			printf("search file %s, success\n", backup_cfg->partition_info.name);
			memcpy(&backup_binbuf, &backup_cfg->ex_partition_info, sizeof(T_EX_PARTITION_CONFIG));//获取到的值是块数
			backup_binbuf.file_length = file_len; // cdh:update kernel size
			memcpy(&backup_cfg->ex_partition_info, &backup_binbuf, sizeof(T_EX_PARTITION_CONFIG));//获取到的值是块数
			setenv_hex("kernel_size", backup_binbuf.file_length);
			break;
		}			
	}    

	/* if not find valid kernel config infor, must return err  */
	if (nr_parts == i) {
		printf("cdh:read cfg file Err!\n");
		goto usage;
	}

	/* 
	*  after update kernel config infor, we must save to flash
	*  because this step is very important, so we retry three times
	*/
	if (spi_flash_erase(spi, boot_len, CONFIG_SECTION_SIZE)) {
		printf("Check Erase config infor index:%d, failed\n", i);
		goto usage;
	}

	if (spi_flash_write(spi, boot_len, CONFIG_SECTION_SIZE, &config_infor)) {
		printf("Check Write config infor index:%d, failed\n", i);
		goto usage;
	}

    /* update kernel config infor to backup partition table. */
	if (spi_flash_erase(spi, boot_len+CONFIG_SECTION_SIZE, CONFIG_SECTION_SIZE)) {
		printf("Check Erase config infor index:%d, failed\n", i);
		goto usage;
	}

	if (spi_flash_write(spi, boot_len+CONFIG_SECTION_SIZE, CONFIG_SECTION_SIZE, &backup_config_infor)) {
		printf("Check Write config infor index:%d, failed\n", i);
		goto usage;
	}
    

	spi_flash_free(spi);
	return 0;
usage:
	spi_flash_free(spi);
	return 1;
}



static int do_setloadaddr(cmd_tbl_t *cmdtp, int flag, int argc, 
		char * const argv[])
{
	struct spi_flash *spi;
	T_PARTITION_TABLE_INFO *cfg;
	unsigned int offset;
	unsigned int *addr;
	unsigned int len, filecnt;
	int ret, i;
	char config_infor[CONFIG_SECTION_SIZE];
	unsigned int  *config_addr;
    unsigned long load_addr = 0;
	char *endp;
	unsigned char *buf = NULL;
	unsigned long boot_len = 0;
	unsigned char boot_temp = 0;
	T_BIN_CONFIG binbuf;
	unsigned long nr_parts;
	T_PARTITION_TABLE_INFO *parts = NULL;

	if (argc < 2) {
        printf("Usage: setloadaddr address.\n");
		return 1;
	}

	load_addr = simple_strtoul(argv[1], &endp, 16);
    setenv_hex("loadaddr", load_addr);
    run_command("saveenv", 0);
    
	/* spi probe flash */
	spi = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
		   	CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!spi) {
		printf("%s: spi_flash probe error.\n", __func__);
		return 1;
	}

	/*
	* chech argc cnt and intial searching SPIP flag spi flash offset address
	* malloc 3 pages size buffer for seraching SPIP flag 
	*/
	offset = CONFIG_SPIP_START_PAGE; 
	len = spi->page_size*CONFIG_SPIP_PAGE_CNT; 
	buf = (unsigned char *)malloc(len);
	if(NULL == buf) {
        printf("find part table buf malloc fail\n");
        spi_flash_free(spi);
        return 1;
    }

	/* clean buffer content */
    memset(buf, 0, len);

    /* from norflash offset=0, read out three pages size content */
    debug("read, offset:0x%x, len:0x%x, addr:0x%x\n", offset, len, buf);
	ret = spi_flash_read(spi, offset, len, buf);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		spi_flash_free(spi);
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
    debug("boot block num offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);

    /* get paration tabel saved block from spi param reserved member value */
    memcpy(&boot_temp, buf + offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, 1);

	/* get paration tabel saved page offset address */
 	boot_len = boot_temp*spi->erase_size;
    debug("get_upadte_boot_size g_boot_len:%d\n", boot_len);

    /*
	* release tmp  paration table buffer
	*/
    if (buf != NULL) {
	    free(buf);
    }
    
	/* config infor area must be 4KB align */
	addr = (unsigned int *)config_infor;  
	len = CONFIG_SECTION_SIZE;
	debug("read config info(%08x) from offset:%08x, len:%d.\n", addr, boot_len, len);
	
	/* first read out 4K config infor area */ 
	ret = spi_flash_read(spi, boot_len, len, addr);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		goto usage;
	}

	nr_parts = *(unsigned long *)addr;
	addr++;
	
	/* if no partiton to mount, the buf will be all 0xFF but not constant.
	 * So, it is not safe here. 
	 */
	debug("nr_parts=0x%lx\n", nr_parts);
	if (nr_parts <= 0 || nr_parts > 15) {
		printf("%s:partition count invalid\n",__func__);
		spi_flash_free(spi);
		return 1;
	}

	parts = (T_PARTITION_TABLE_INFO *)addr; 
	debug("size part=%d Byte\n", sizeof(T_PARTITION_TABLE_INFO));
	debug("Creating %d MTD partitions on:%s\n", nr_parts, spi->name);

	/* get vaild config file infor  */
	for (i=0; i<nr_parts; i++) {
		cfg = (T_PARTITION_TABLE_INFO *)(addr + i*sizeof(T_PARTITION_TABLE_INFO) / sizeof(unsigned long));
		debug("read file %s.\n", cfg->partition_info.name);
		if (0 == strncmp(cfg->partition_info.name, CONFIG_BIOS_NAME, PARTITION_NAME_LEN)) {
			printf("search file %s, success\n", cfg->partition_info.name);
			memcpy(&binbuf, &cfg->ex_partition_info, sizeof(T_EX_PARTITION_CONFIG));//获取到的值是块数
            binbuf.ld_addr = load_addr; 
			memcpy(&cfg->ex_partition_info, &binbuf, sizeof(T_EX_PARTITION_CONFIG));//获取到的值是块数

			break;
		}			
	}

	/* if not find valid kernel config infor, must return err  */
	if (nr_parts == i) {
		printf("cdh:read cfg file Err!\n");
		goto usage;
	}

	/* 
	*  after update kernel config infor, we must save to flash
	*  because this step is very important, so we retry three times
	*/
	if (spi_flash_erase(spi, boot_len, CONFIG_SECTION_SIZE)) {
		printf("Check Erase config infor index:%d, failed\n", i);
		goto usage;
	}

	if (spi_flash_write(spi, boot_len, CONFIG_SECTION_SIZE, &config_infor)) {
		printf("Check Write config infor index:%d, failed\n", i);
		goto usage;
	}

	spi_flash_free(spi);
	return 0;
usage:
	spi_flash_free(spi);
	return 1;
}


U_BOOT_CMD(
	updatecfg, 4, 0, do_writecfg,  		
	"update config from config infor table.", 
	"updatecfg [name] len"	
);

U_BOOT_CMD(
	setloadaddr, 4, 0, do_setloadaddr,  		
	"set loadaddr to  config infor .", 
	"setloadaddr address"	
);
