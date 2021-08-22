/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
//#include <nios.h>
#include <asm/io.h>
#include <spi_flash.h>
#include <spi.h>
#include <malloc.h>

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

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

#if defined(CONFIG_PARTITION_TABLE_BY_ANYKA)
T_PARTITION_TABLE_INFO *gPartion = NULL;
unsigned long g_nr_parts = 0;
unsigned long parts_offset = NULL;
#endif

/*----------------------------------------------------------------------*/
unsigned long flash_init (void)
{
	int i;
	unsigned long addr;
	flash_info_t *fli = &flash_info[0];
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	struct spi_flash *flash = NULL;
	
	//printf("cdh:%s, flash_init\n", __func__);
	flash = spi_flash_probe(bus, cs, speed, mode);
	if (!flash) {
		printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
		return 1;
	}

	fli->size = flash->size;
	fli->sector_count = flash->size/flash->erase_size; // cdh notes:use erase size , not sector size
	fli->flash_id = flash->flashid;
	//printf("cdh:%s, flash->size:0x%x\n", __func__, flash->size);
	//printf("cdh:%s, flash->erase_size:0x%x\n", __func__, flash->erase_size);
	//printf("cdh:%s, fli->sector_count:0x%x\n", __func__, fli->sector_count);
	//printf("cdh:%s, fli->flash_id:0x%x\n", __func__, fli->flash_id);
	
	addr = CONFIG_SYS_FLASH_BASE;
	for (i=0; i<fli->sector_count; ++i) {
		fli->start[i] = addr;
		addr += flash->erase_size;	// cdh notes:use erase size , not sector size
		fli->protect[i] = 0;
	}

	/* Assign spi_flash ops */
	fli->write = flash->write;
	fli->erase = flash->erase;
	fli->read  = flash->read;
	fli->flash = flash;
	//printf("cdh:flash_size:0x%x\n", fli->size);

	return fli->size;
}

#if defined(CONFIG_PARTITION_TABLE_BY_ANYKA)
int flash_partition_table_init (void)
{
	unsigned int *Partion = NULL;

	struct spi_flash *spi = NULL;
	int ret = 0;
	unsigned long nr_parts;

	spi = spi_flash_probe(CONFIG_SF_DEFAULT_BUS, CONFIG_SF_DEFAULT_CS,
			CONFIG_SF_DEFAULT_SPEED, CONFIG_SF_DEFAULT_MODE);
	if (!spi) {
		printf("%s: spi_flash probe error.\n", __func__);
		ret = -1;
		return ret;
	}
	
	if (spi_partition_table_offset_get(spi, &parts_offset)){
		printf("get partition table offset failed!\n");
		ret = -1;
		goto part_exit;
	}

	Partion = (unsigned int *)malloc(10*sizeof(T_PARTITION_TABLE_INFO));
	if(Partion == NULL){
		printf("malloc partion table buffer failed!\n");
		ret = -1;
		goto part_exit;
	}

	if (spi_partition_table_content_get(spi, parts_offset, Partion, &nr_parts)){
		printf("get partition table backup content!\n");

		parts_offset += 4096;
		if (spi_partition_table_content_get(spi, parts_offset, Partion, &nr_parts)){
			printf("get partition table backup content failed!\n");
			free(Partion);
			ret = -1;
		}
	}

	gPartion = (T_PARTITION_TABLE_INFO *)malloc(nr_parts*sizeof(T_PARTITION_TABLE_INFO));
	if(gPartion == NULL){
		printf("malloc partion table buffer failed!\n");
		free(Partion);
		ret = -1;
		goto part_exit;
	}

	g_nr_parts = nr_parts;
	memcpy((unsigned int *)gPartion, Partion, nr_parts*sizeof(T_PARTITION_TABLE_INFO));

#ifdef DEBUG	
	int i;
	char cmd_buf[8] = {0,};
	T_PARTITION_TABLE_INFO *cfg;
	
	printf("g_nr_parts=0x%lx\n", g_nr_parts);
	printf("gPartion:0x%x, g_nr_parts:%d\n", gPartion, g_nr_parts);
	for (i=0; i<g_nr_parts; i++) {
		memset(cmd_buf,0,sizeof(cmd_buf));
		cfg = (T_PARTITION_TABLE_INFO *)(gPartion + i);	
		sprintf(cmd_buf, "%s\0", cfg->partition_info.name);
		printf("##gPartion:name:%s, offset:0x%08x, ksize:0x%08x ##\n",
					cmd_buf, 
					cfg->partition_info.start_pos, 
					cfg->partition_info.ksize); 		
	}
	printf("malloc part table buf nr_parts:%d, part_str:%d, len:%d\n", g_nr_parts, sizeof(T_PARTITION_TABLE_INFO), (g_nr_parts*sizeof(T_PARTITION_TABLE_INFO)));
#endif
	free(Partion);
	
part_exit:
	spi_flash_free(spi);
	return ret;
}
#endif


/*--------------------------------------------------------------------*/
void flash_print_info (flash_info_t * info)
{
	int i, k;
	unsigned long size;
	int erased;
	volatile unsigned char *flash;

	printf ("  Size: %ld KB in %d Sectors\n", info->size >> 10, info->sector_count);
	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {

		/* Check if whole sector is erased */
		if (i != (info->sector_count - 1))
			size = info->start[i + 1] - info->start[i];
		else
			size = info->start[0] + info->size - info->start[i];
		erased = 1;
		flash = (volatile unsigned char *) info->start[i];
		for (k = 0; k < size; k++) {
			if (*flash++ != 0xff) {
				erased = 0;
				break;
			}
		}

		/* Print the info */
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s%s", info->start[i], erased ? " E" : "  ",
			info->protect[i] ? "RO " : "   ");
	}
	printf ("\n");
}

/*-------------------------------------------------------------------*/


int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int prot;
	int sect;
	int ret = -1;
	
	/* Some sanity checking */
	if ((s_first < 0) || (s_first > s_last)) {
		printf ("- no sectors to erase\n");
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}
	
	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n", prot);
	} else {
		printf ("\n");
	}

#ifdef DEBUG
	for (sect=s_first; sect<=s_last; sect++) {
		printf("- Erase: Sect: %i @ 0x%08x\n", sect,  info->start[sect]);
	}
#endif

	for (sect=s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			ret = info->erase(info->flash, info->start[sect], info->flash->erase_size);
			if (ret < 0) {
				printf("SF: erase failed\n");
				return 1;
			}else{
				puts(".");
			}
		}
	}

	printf ("\n");

	return 0;
}


/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	int ret = -1;
	
	ret = info->write(info->flash, (u32)addr, cnt, (const void *)src);
	if (ret != 0) {
		printf("SF: erase failed\n");
		return 1;
	}

	return (0);
}
