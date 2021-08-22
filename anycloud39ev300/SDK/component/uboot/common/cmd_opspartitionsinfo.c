#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include <asm/io.h>
/* units:bytes. */
#define MAX_SPICFG_LEN  	(1024)
#define MTD_PART_NAME_LEN   (4)


/**
* @brief read out paration infor from paration table
*
* search and read out paration infor from paration table
* @author      CaoDonghua
* @date        2016-10-20
* @param[in] *cmdtp cmd handle
* @param[in] flag  cmd flag
* @param[in] cmd_len write cmd length
* @param[in] argc cmd param cnt
* @param[in] argv[] cmd param buffer
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
static int do_opspartitionsinfo(cmd_tbl_t *cmdtp, int flag, int argc, 
		char * const argv[])
{
	int ret;
	int i;
	unsigned int len = 0;
	unsigned long offset;
	unsigned long nr_parts;
	unsigned long *addr = NULL;
	unsigned long boot_len = 0;
	unsigned char *buf = NULL;
	unsigned char boot_temp = 0;
	T_PARTITION_TABLE_INFO *parts = NULL;
	struct spi_flash *spi;
	char paration_table[CONFIG_PARATION_TAB_SIZE];
	
	/*
	* spi probe flash
	*/
	spi = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
		   	CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!spi) {
		printf("%s: spi_flash probe error.\n", __func__);
		return 1;
	}

	debug("argc 1.\n");
	
	/* malloc 3 pages size buffer for seraching SPIP flag */
	offset = CONFIG_SPIP_START_PAGE; 
	len = spi->page_size*CONFIG_SPIP_PAGE_CNT; 
	buf = (unsigned char *)malloc(len);
	if (NULL == buf) {
        printf("find part table buf malloc fail\n");
        ret = 1;
		goto done;
    }

	/* clean buffer content */
    memset(buf, 0, len);

	/* from norflash offset=0, read out three pages size content */
	printf("read SPIP, offset:0x%x, len:0x%x, addr:0x%x\n", offset, len, buf);
	ret = spi_flash_read(spi, offset, len, buf);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		ret = 1;
		goto done;
	}

	/* search SPIP flag */
	for (i=0; i<(len-4); i++) {
		if (('S' == buf[i]) && ('P' == buf[i+1]) && ('I' == buf[i+2]) && ('P' == buf[i+3])) {
			offset = i;
			break;
		}
	}
 
    if (i == (len - 4)) {
       	printf("not find partition start block\n");
       	ret = 1;
		goto done; 
    }

    /* get spi param last reserved member value*/
    printf("boot block num offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);

    /* get paration tabel saved block from spi param reserved member value */
    memcpy(&boot_temp, buf + offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, 1);

	/* get paration tabel saved page offset address */
 	boot_len = boot_temp*spi->erase_size;
    printf("get_upadte_boot_size g_boot_len:%d, boot_temp:%d\n", boot_len, boot_temp);

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
    printf("part table read, offset:0x%x, len:0x%x, addr:0x%x\n", boot_len, len, addr);
	ret = spi_flash_read(spi, boot_len, len, addr);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		ret = 1;
		goto done;
	}

	/*
	* get all paration table member cnt from the first word address of paration table
	*/
	nr_parts = *(volatile unsigned long *)addr;
	addr++;
	
	/* if no partiton to mount, the buf will be all 0xFF but not constant.
	 * So, it is not safe here. 
	 */
	debug("nr_parts=0x%lx\n", nr_parts);
	if (nr_parts == 0 || nr_parts > 15) {
		printf("%s:partition count invalid\n",__func__);
		ret = 1;
		goto done;
	}

	/*
	* search all paration table member and print out 
	*/
	parts = (T_PARTITION_TABLE_INFO *)addr; 
	printf("size part=%d Byte\n", sizeof(T_PARTITION_TABLE_INFO));
	printf("====Creating %d MTD partitions on:%s====\n", nr_parts, spi->name);
	for (i=0; i<nr_parts; i++) {
		printf("parts[%d]:\nname = %s\nsize = 0x%lx\noffset = 0x%lx\nrw_flags = 0x%x\n\n",
				i, 
				parts[i].partition_info.name, 
				parts[i].partition_info.ksize*1024, 
				parts[i].partition_info.start_pos, 
				parts[i].partition_info.r_w_flag);
	}
	
	ret = 0;
done:
	spi_flash_free(spi);
	return ret;
}


U_BOOT_CMD(
	parts, 5, 0, do_opspartitionsinfo,  		
	"read out partitions table info.", 
	"part  read out all parts info\n"	
);




/**
* @brief adjust paration information from paration table
*
* 
* @author      ZhangZhiPeng
* @date        2017-11-9
* @param[in] *cmdtp do cmd handle
* @param[in] flag  do cmd flag
* @param[in] argc do cmd param cnt
* @param[in] argv[] do cmd param buffer
* @return int  0 on success, else other on failed
* @retval 0 on success
* @retval else other on failed
*/
static int do_adjustpartsinfo(cmd_tbl_t *cmdtp, int flag, int argc, 
		char * const argv[])
{
	int ret;
	int i;
	unsigned int command,len = 0;
	unsigned long offset;
    unsigned long part_offset,part_size;
	unsigned long nr_parts;
	unsigned long *addr = NULL;
	unsigned long boot_len = 0;
	unsigned char *buf = NULL;
	unsigned char boot_temp = 0;
	T_PARTITION_TABLE_INFO *parts = NULL;
	struct spi_flash *spi;
	char paration_table[CONFIG_PARATION_TAB_SIZE];

    if (argc < 3)
		return CMD_RET_USAGE;

    if (argc != 4)
		return CMD_RET_USAGE;

    if (strcmp(argv[1], "KERNEL") == 0){
		command = 0;
     }
    else if(strcmp(argv[1], "MAC") == 0){
		command = 1;  
    }
    else if(strcmp(argv[1], "ENV") == 0){
		command = 2;  
    }
    else if(strcmp(argv[1], "A") == 0){
		command = 3;  
    }
    else if(strcmp(argv[1], "B") == 0){
		command = 4;  
    }
    else if(strcmp(argv[1], "C") == 0){
		command = 5;   
    }
    else 
        return CMD_RET_USAGE;

    part_size = simple_strtoul(argv[2], NULL, 16);
    part_offset = simple_strtoul(argv[3], NULL, 16);
    debug("part_size:%x,part_offset:%x\n", part_size,part_offset);
    
	/*
	* spi probe flash
	*/
	spi = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
		   	CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!spi) {
		printf("%s: spi_flash probe error.\n", __func__);
		return 1;
	}
	
	/* malloc 3 pages size buffer for seraching SPIP flag */
	offset = CONFIG_SPIP_START_PAGE; 
	len = spi->page_size*CONFIG_SPIP_PAGE_CNT; 
	buf = (unsigned char *)malloc(len);
	if (NULL == buf) {
        printf("find part table buf malloc fail\n");
        ret = 1;
		goto done;
    }

	/* clean buffer content */
    memset(buf, 0, len);

	/* from norflash offset=0, read out three pages size content */
	debug("read SPIP, offset:0x%x, len:0x%x, addr:0x%x\n", offset, len, buf);
	ret = spi_flash_read(spi, offset, len, buf);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		ret = 1;
		goto done;
	}

	/* search SPIP flag */
	for (i=0; i<(len-4); i++) {
		if (('S' == buf[i]) && ('P' == buf[i+1]) && ('I' == buf[i+2]) && ('P' == buf[i+3])) {
			offset = i;
			break;
		}
	}
 
    if (i == (len - 4)) {
       	printf("not find partition start block\n");
       	ret = 1;
		goto done; 
    }

    /* get spi param last reserved member value*/
    debug("boot block num offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);

    /* get paration tabel saved block from spi param reserved member value */
    memcpy(&boot_temp, buf + offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, 1);

	/* get paration tabel saved page offset address,erase_size=4096 */
 	boot_len = boot_temp*spi->erase_size;
    debug("get_upadte_boot_size g_boot_len:%d, boot_temp:%d\n", boot_len, boot_temp);

	/*
	* release tmp  paration table buffer
	*/
    if (buf != NULL) {
		free(buf);
		buf = NULL;
    }

    /* read out paration table from  boot_len,page_size=256 */
	len = spi->page_size*CONFIG_PARATION_PAGE_CNT;
	addr = (unsigned long *)paration_table;
    debug("part table read, offset:0x%x, len:0x%x, addr:0x%x\n", boot_len, len, addr);
	ret = spi_flash_read(spi, boot_len, len, addr);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		ret = 1;
		goto done;
	}

	/*
	* get all paration table member cnt from the first word address of paration table
	*/
	nr_parts = *(volatile unsigned long *)addr;
	addr++;
	
	/* if no partiton to mount, the buf will be all 0xFF but not constant.
	 * So, it is not safe here. 
	 */
	debug("nr_parts=0x%lx\n", nr_parts);
	if (nr_parts == 0 || nr_parts > 15) {
		printf("%s:partition count invalid\n",__func__);
		ret = 1;
		goto done;
	}

	/*
	* search all paration table member and print out 
	*/
	parts = (T_PARTITION_TABLE_INFO *)addr; 
	
	debug("====Creating %d MTD partitions on:%s====\n", nr_parts, spi->name);
	for (i=0; i<nr_parts; i++) {
		debug("parts[%d]:\nname = %s\nsize = 0x%lx\noffset = 0x%lx\nrw_flags = 0x%x\n\n",
				i, 
				parts[i].partition_info.name, 
				parts[i].partition_info.ksize*1024, 
				parts[i].partition_info.start_pos, 
				parts[i].partition_info.r_w_flag);
	}
    
	switch(command){
        case 0: 
            parts[0].partition_info.ksize   = part_size;
            parts[0].partition_info.start_pos   = part_offset;
        break;
             
        case 1: 
            parts[1].partition_info.ksize   = part_size;
            parts[1].partition_info.start_pos   = part_offset;        
        break;

        case 2: 
            parts[2].partition_info.ksize   = part_size;
            parts[2].partition_info.start_pos   = part_offset;      
        break;

        case 3: 
            parts[3].partition_info.ksize   = part_size;
            parts[3].partition_info.start_pos   = part_offset;        
        break;

        case 4: 
            parts[4].partition_info.ksize   = part_size;
            parts[4].partition_info.start_pos   = part_offset;        
        break;

        case 5: 
            parts[5].partition_info.ksize   = part_size;
            parts[5].partition_info.start_pos   = part_offset;
        break;
        default:
            ret = -1;
	}

    /* 
	*  after adjust partitions information, we must save to flash
	*  because this step is very important, so we retry three times
	*/
	if (spi_flash_erase(spi, boot_len, CONFIG_SECTION_SIZE)) {
		printf("Check Erase partitions table failed\n");
	}

	if (spi_flash_write(spi, boot_len, CONFIG_PARATION_TAB_SIZE, paration_table)) {
		printf("Check Write partitions table  failed\n");
	}
	
	ret = 0;
done:
	spi_flash_free(spi);
	return ret;
}


U_BOOT_CMD(
	parts_adjust, 5, 0, do_adjustpartsinfo,  		
	"\nadjust parts info. Each part's size,offset etc.\n", 
	":part_adjust parts_name size(hex) offset(hex) \n"	
);

