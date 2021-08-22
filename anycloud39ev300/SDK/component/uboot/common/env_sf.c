/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <environment.h>
#include <malloc.h>
#include <spi_flash.h>
#include <search.h>
#include <errno.h>

#ifndef CONFIG_ENV_SPI_BUS
# define CONFIG_ENV_SPI_BUS	0
#endif
#ifndef CONFIG_ENV_SPI_CS
# define CONFIG_ENV_SPI_CS	0
#endif
#ifndef CONFIG_ENV_SPI_MAX_HZ
# define CONFIG_ENV_SPI_MAX_HZ	1000000
#endif
#ifndef CONFIG_ENV_SPI_MODE
# define CONFIG_ENV_SPI_MODE	SPI_MODE_3
#endif

#ifdef CONFIG_ENV_OFFSET_REDUND
static ulong env_offset		= CONFIG_ENV_OFFSET;
static ulong env_new_offset	= CONFIG_ENV_OFFSET_REDUND;

#define ACTIVE_FLAG	1
#define OBSOLETE_FLAG	0
#endif /* CONFIG_ENV_OFFSET_REDUND */

DECLARE_GLOBAL_DATA_PTR;

char *env_name_spec = "SPI Flash";

static struct spi_flash *env_flash;

// cdh:all 32Byte
struct file_config
{
	ulong file_length;
    ulong ld_addr;
    ulong start_page;
	ulong backup_page;        //backup data start page
    char file_name[16];
};

#if defined(CONFIG_ENV_OFFSET_REDUND)
int saveenv(void)
{
	env_t	env_new;
	ssize_t	len;
	char	*res, *saved_buffer = NULL, flag = OBSOLETE_FLAG;
	u32	saved_size, saved_offset, sector = 1;
	int	ret;

	if (!env_flash) {
		env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
			CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
		if (!env_flash) {
			set_default_env("!spi_flash_probe() failed");
			return 1;
		}
	}

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL);
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		return 1;
	}
	env_new.crc	= crc32(0, env_new.data, ENV_SIZE);
	env_new.flags	= ACTIVE_FLAG;

	if (gd->env_valid == 1) {
		env_new_offset = CONFIG_ENV_OFFSET_REDUND;
		env_offset = CONFIG_ENV_OFFSET;
	} else {
		env_new_offset = CONFIG_ENV_OFFSET;
		env_offset = CONFIG_ENV_OFFSET_REDUND;
	}

	/* Is the sector larger than the env (i.e. embedded) */
	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		saved_size = CONFIG_ENV_SECT_SIZE - CONFIG_ENV_SIZE;
		saved_offset = env_new_offset + CONFIG_ENV_SIZE;
		saved_buffer = malloc(saved_size);
		if (!saved_buffer) {
			ret = 1;
			goto done;
		}
		ret = spi_flash_read(env_flash, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	if (CONFIG_ENV_SIZE > CONFIG_ENV_SECT_SIZE) {
		sector = CONFIG_ENV_SIZE / CONFIG_ENV_SECT_SIZE;
		if (CONFIG_ENV_SIZE % CONFIG_ENV_SECT_SIZE)
			sector++;
	}

	puts("Erasing SPI flash...");
	ret = spi_flash_erase(env_flash, env_new_offset,
				sector * CONFIG_ENV_SECT_SIZE);
	if (ret)
		goto done;

	puts("Writing to SPI flash...");

	ret = spi_flash_write(env_flash, env_new_offset,
		CONFIG_ENV_SIZE, &env_new);
	if (ret)
		goto done;

	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		ret = spi_flash_write(env_flash, saved_offset,
					saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	ret = spi_flash_write(env_flash, env_offset + offsetof(env_t, flags),
				sizeof(env_new.flags), &flag);
	if (ret)
		goto done;

	puts("done\n");

	gd->env_valid = gd->env_valid == 2 ? 1 : 2;

	printf("Valid environment: %d\n", (int)gd->env_valid);

 done:
	if (saved_buffer)
		free(saved_buffer);

	return ret;
}

void env_relocate_spec(void)
{
	int ret;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1 = NULL;
	env_t *tmp_env2 = NULL;
	env_t *ep = NULL;

	tmp_env1 = (env_t *)malloc(CONFIG_ENV_SIZE);
	tmp_env2 = (env_t *)malloc(CONFIG_ENV_SIZE);

	if (!tmp_env1 || !tmp_env2) {
		set_default_env("!malloc() failed");
		goto out;
	}

	env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!env_flash) {
		set_default_env("!spi_flash_probe() failed");
		goto out;
	}

	ret = spi_flash_read(env_flash, CONFIG_ENV_OFFSET,
				CONFIG_ENV_SIZE, tmp_env1);
	if (ret) {
		set_default_env("!spi_flash_read() failed");
		goto err_read;
	}

	if (crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc)
		crc1_ok = 1;

	ret = spi_flash_read(env_flash, CONFIG_ENV_OFFSET_REDUND,
				CONFIG_ENV_SIZE, tmp_env2);
	if (!ret) {
		if (crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc)
			crc2_ok = 1;
	}

	if (!crc1_ok && !crc2_ok) {
		set_default_env("!bad CRC");
		goto err_read;
	} else if (crc1_ok && !crc2_ok) {
		gd->env_valid = 1;
	} else if (!crc1_ok && crc2_ok) {
		gd->env_valid = 2;
	} else if (tmp_env1->flags == ACTIVE_FLAG &&
		   tmp_env2->flags == OBSOLETE_FLAG) {
		gd->env_valid = 1;
	} else if (tmp_env1->flags == OBSOLETE_FLAG &&
		   tmp_env2->flags == ACTIVE_FLAG) {
		gd->env_valid = 2;
	} else if (tmp_env1->flags == tmp_env2->flags) {
		gd->env_valid = 2;
	} else if (tmp_env1->flags == 0xFF) {
		gd->env_valid = 2;
	} else {
		/*
		 * this differs from code in env_flash.c, but I think a sane
		 * default path is desirable.
		 */
		gd->env_valid = 2;
	}

	if (gd->env_valid == 1)
		ep = tmp_env1;
	else
		ep = tmp_env2;

	ret = env_import((char *)ep, 0);
	if (!ret) {
		error("Cannot import environment: errno = %d\n", errno);
		set_default_env("env_import failed");
	}

err_read:
	spi_flash_free(env_flash);
	env_flash = NULL;
out:
	free(tmp_env1);
	free(tmp_env2);
}
#else


#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
/**
* @brief get uboot saved  environment variable area offset address 
*
*  get uboot saved  environment variable area offset address from kernel config infor table
* @author CaoDonghua
* @date 2016-09-26
* @param[in] flash	 spiflash handle.
* @param[out] *poffset env saved address pointer.
* @param[out] *pbios_size env saved partition size.
* @return int return get environment offset address from kernel config infor table in flash success or failed
* @retval returns zero on success
* @retval return a non-zero if failed
*/
static int get_env_config_info(struct spi_flash *flash, u32 *poffset, u32 *pbios_size)
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

#if 1
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
	//debug("read, offset:0x%x, len:0x%x, addr:0x%x\n", offset, len, buf);
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
    debug("boot block num offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);

    /* get paration tabel saved block from spi param reserved member value */
    memcpy(&boot_temp, buf + offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, 1);

	/* get paration tabel saved page offset address */
 	boot_len = boot_temp*flash->erase_size;
    //debug("get_upadte_boot_size g_boot_len:%d\n", boot_len);

    /*
	* release tmp  paration table buffer
	*/
    if (buf != NULL) {
	    free(buf);
    }
	
	/* config infor area must be 2 pages align */
	len = flash->page_size*CONFIG_PARATION_PAGE_CNT;
	addr = (unsigned long *)paration_table;
	//debug("read config info(%08x) from offset:%08x, len:%d.\n", addr, boot_len, len);
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
		if (0 == strncmp((const char *)cfg->partition_info.name, CONFIG_ENV_NAME, PARTITION_NAME_LEN)) {
			debug("search file %s, success\n", cfg->partition_info.name);
		
			/* get ENV paration offset address */
			*poffset = cfg->partition_info.start_pos; 

			/* get ENV paration size */
			*pbios_size = cfg->partition_info.ksize * 1024; 
			
			debug("cdh:update kernel offset, kernel_offsetstartpage:0x%x, kernel_getoffset:0x%x\n", cfg->partition_info.start_pos, *poffset);	
			break;
		}			
	}
#endif
	/* if not find valid kernel config infor, must return err  */
	if (nr_parts == i) {
		printf("cdh:read cfg file Err!\n");
		return 1;
	}

	return 0;
}
#endif

/**
* @brief save uboot environment variable
*
*  save uboot environment variable to spi norflash.
* @author CaoDonghua
* @date 2016-09-26
* @param[in] void.
* @return int return save environment to flash success or failed
* @retval returns zero on success
* @retval return a non-zero if failed
*/
int saveenv(void) 
{
	u32	saved_size;
	u32	saved_offset,sector = 1;
	char *res;
	char *saved_buffer = NULL;
	int	ret = 1;
	env_t	env_new;
	ssize_t	len;
	
	debug("cdh:%s..\n", __func__);
	
	/* Here is probe spi norflash to check whether has saving device or not*/
	if (!env_flash) {
		env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
			CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
			
		if (!env_flash) {
			set_default_env("!spi_flash_probe() failed");
			return 1;
		}
	}

#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
	/* Search environment variable saved address in flash,
	*  Because we had no env area, so we now use serial number area as env address for test	
	*/
	if (get_env_config_info(env_flash, &saved_offset, &saved_size)) {
		puts("cdh:no find env spi norflash save address!\n");
		goto done;
	}else {
		/* Notes: we config env sect size equal env size, or Is the sector size must larger than the env size(i.e. embedded) */
		if (saved_size < ENV_SIZE) {
			puts("cdh:save size Env_size beyond ENV partition size!\n");
			goto done;
		}
	}

	debug("cdh:%s, env_offset:0x%x, env_partsize:0x%x\n", __func__, saved_offset, saved_size);
#else
    /* Is the sector larger than the env (i.e. embedded) */
	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		saved_size = CONFIG_ENV_SECT_SIZE - CONFIG_ENV_SIZE;
		saved_offset = CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE;
		saved_buffer = malloc(saved_size);
		if (!saved_buffer)
			goto done;

		ret = spi_flash_read(env_flash, saved_offset,
			saved_size, saved_buffer);
		if (ret)
			goto done;
	}

	if (CONFIG_ENV_SIZE > CONFIG_ENV_SECT_SIZE) {
		sector = CONFIG_ENV_SIZE / CONFIG_ENV_SECT_SIZE;
		if (CONFIG_ENV_SIZE % CONFIG_ENV_SECT_SIZE)
			sector++;
	}
#endif

	res = (char *)&env_new.data;
	len = hexport_r(&env_htab, '\0', 0, &res, ENV_SIZE, 0, NULL); // cdh:ENV_SIZE
	if (len < 0) {
		error("Cannot export environment: errno = %d\n", errno);
		goto done;
	}

	debug("cdh:%s, len:0x%x, CONFIG_ENV_SIZE:0x%x\n", __func__, len, CONFIG_ENV_SIZE);
	if (len > CONFIG_ENV_SIZE)
	{
		error("Save environment beyond define ENV SIZE: errno = %d\n", errno);
		goto done;
	}
	
	env_new.crc = crc32_new(0, env_new.data, ENV_SIZE);
	debug("cdh:%s, crc:0x%x\n", __func__, env_new.crc);
	
	debug("Erasing SPI flash...");

#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
	/* Erasing  serial number area sector of spi flash as env saving address,
	*  Because we had no env area, so we now use serial number area as env address for test	
	*  erase size saved_size must be 4KB align 
	*/
	ret = spi_flash_erase(env_flash, saved_offset, saved_size);
	if (ret)
		goto done;

	debug("Writing to SPI flash...");
	
	/* Writing   environment variable to env saving address,
	*  Because we had no env area, so we now use serial number area as env address for test	
	*/
	ret = spi_flash_write(env_flash, saved_offset+4, CONFIG_ENV_SIZE-4, &env_new);
	if (ret)
		goto done;
#else 
    ret = spi_flash_erase(env_flash, CONFIG_ENV_OFFSET,
		sector * CONFIG_ENV_SECT_SIZE);
	if (ret)
		goto done;

	puts("Writing to SPI flash...");
	ret = spi_flash_write(env_flash, CONFIG_ENV_OFFSET,
		CONFIG_ENV_SIZE, &env_new);
	if (ret)
		goto done;

	if (CONFIG_ENV_SECT_SIZE > CONFIG_ENV_SIZE) {
		ret = spi_flash_write(env_flash, saved_offset,
			saved_size, saved_buffer);
		if (ret)
			goto done;
	}
#endif
	
	/* cdh:if need env data len update ???  */
	ret = 0;
	puts("Env save done OK\n");

 done:
	if (saved_buffer)
		free(saved_buffer);

	return ret;
}


/**
* @brief relocate uboot environment variable load address
*
*  relocate uboot environment variable load address from spi norflash.
* @author CaoDonghua
* @date 2016-09-26
* @param[in] void.
* @return void
* @retval none
*/
void env_relocate_spec(void)
{
	char buf[CONFIG_ENV_SIZE];
	int ret;
	u32 tmp_config_env_offset = 0;
	u32 tmp_config_env_size = 0;
	char *cfgcmd  = "readcfg";
	char *root    = "/dev/mtdblock";
	char *cmdline = NULL;
	char *cmdroot = NULL;
	char *proot   = NULL;
	
	debug("cdh:%s..\n", __func__);
	
	env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
			CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
	if (!env_flash) {
		set_default_env("!spi_flash_probe() failed");
		return;
	}

#ifdef CONFIG_PARTITION_TABLE_BY_ANYKA
	/* Search environment variable saved address in flash,
	*  Because we had no env area, so we now use serial number area as env address for test	
	*/
	if (get_env_config_info(env_flash, &tmp_config_env_offset, &tmp_config_env_size)) {
		puts("no find env spi norflash save address!\n");
		ret = 1;
		goto out1;
	}
    

	/* Read out environment variable from saved address in flash,
	*  Because we had no env area, so we now use serial number area as env address for test	
	*  cdh:we must notes:for our burntool format, ENV eare first word save env data len
	*/
	ret = spi_flash_read(env_flash, tmp_config_env_offset, tmp_config_env_size, buf);
out1:
	if (ret) {// cdh:read spi flash env eare failed, use default env
		set_default_env("!spi_flash_read() failed");
		debug("Load Env ERR, make bootargs!\n");
		run_command(cfgcmd, 0);
		cmdline = getenv("bootargs");
		cmdroot = getenv("mtd_root");
		proot 	= strstr(cmdline, root); // cdh:search "/dev/mtdblock" string
		if(proot != NULL){
			memcpy(proot, cmdroot, 14);
			//debug("cdh:%s, proot=0x%x\n", __func__, proot);
			debug("cdh:%s, proot:%s, cmdroot:%s\n", __func__, proot, cmdroot);
			run_command("saveenv", 0);
		}else{
			puts("search root mtdblock failed!\n");
		}

		goto out;
	}

	/*
	*cdh:we must notes:for our burntool format, ENV eare first word save env data len
	*/
	ret = env_import((const char *)&buf[4], 1);
	if (ret){
		gd->env_valid = 1;
		puts("Load Env CRC OK!\n");
	}else{
		puts("Load Env CRC ERR, and make bootargs!\n");
		run_command(cfgcmd, 0);
		cmdline = getenv("bootargs");
		cmdroot = getenv("mtd_root");
		proot = strstr(cmdline, root);
		if(proot != NULL){
			memcpy(proot, cmdroot, 14);
			//debug("cdh:%s, proot=0x%x\n", __func__, proot);
			debug("cdh:%s, proot:%s, cmdroot:%s\n", __func__, proot, cmdroot);
			run_command("saveenv", 0);
		}else{
			puts("search root mtdblock failed!\n");
		}
	}

#else
    ret = spi_flash_read(env_flash,
		CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE, buf);
	if (ret) {
		set_default_env("!spi_flash_read() failed");
		goto out;
	}
	ret = env_import(buf, 1);
	if (ret)
		gd->env_valid = 1;
	else
		saveenv();
#endif


	
out:
	spi_flash_free(env_flash);
	env_flash = NULL;
}
#endif

int env_init(void)
{
	debug("cdh:%s..\n", __func__);
	
	/* SPI flash isn't usable before relocation */
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;

	return 0;
}
