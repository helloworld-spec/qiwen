#include "anyka_types.h"
#include "spi.h"
#include "spiflash.h"
#include "console.h"
#include "anyka_cpu.h"
#include "rtc.h"


#define SPIBOOT_DATA_START_PAGE        559//65 /*33 */

#define SPI_BOOT_VERSION       "V1.2.03"
#define BOOT_TMP_BUFFER_ADDR   0x81000000

#define RAM_SIZE                 (16<<20) //16MB

#define RAM_ADDR_START           (0x80000000)

#define _MMUTT_SIZE				(0x4000)
#define _MMUTT_STARTADDRESS     (RAM_ADDR_START + RAM_SIZE - _MMUTT_SIZE)


#define CMD_LINE_ADDR 	(0x30400100)
#define CMD_LINE_LEN 	(512)

#define CONFIG_BIOS_NAME 	"KERNEL"

typedef void (*bfunc)(void);




void run(void *funcaddr)
{
    bfunc       F;

    F = (bfunc)(funcaddr);
    F();
}

T_BOOL readcfgbyname(T_PARTITION_TABLE_INFO *pcfg,T_U8 *fileName)
{
	T_PARTITION_TABLE_INFO *tmpcfg;
	T_U32 *pData=AK_NULL;
    T_U32 i;
    T_BOOL bMatch;

	T_BIN_CONFIG binbuf;
	unsigned int offset;
	unsigned int len;

	unsigned char *buf = AK_NULL;
	unsigned int *addr;

	unsigned long boot_len = 0;
	unsigned char boot_temp = 0;
	unsigned long erase_size = 0;
	unsigned long nr_parts;
	char paration_table[CONFIG_PARATION_TAB_SIZE];
	
   	if (AK_NULL == fileName)
	{
		printf("name is NULL\n");
		return AK_FALSE;
	}

	/* FIXME */
    pData = (T_U32 *)BOOT_TMP_BUFFER_ADDR;
   
	/*
	* chech argc cnt and intial searching SPIP flag spi flash offset address
	* malloc 3 pages size buffer for seraching SPIP flag 
	*/
	offset = CONFIG_SPIP_START_PAGE; 
	len = SPI_FLASH_PAGE_SIZE*CONFIG_SPIP_PAGE_CNT; 
	
	
	
    /* from norflash offset=0, read out three pages size content */
    printf("read, offset:0x%x, len:0x%x, addr:0x%x\r\n", offset, len, buf);
	if (!spi_flash_read((T_U32)CONFIG_SPIP_START_PAGE, (T_U8*)pData, CONFIG_SPIP_PAGE_CNT)) {
		printf("spi_flash read error.\n");
		while(1);
	}

	buf = (unsigned char *)pData;
	/* search SPIP flag */
	for (i=0; i<(len - 4); i++) {
		if (('S' == buf[i]) && ('P' == buf[i+1]) && ('I' == buf[i+2]) && ('P' == buf[i+3])) {
			offset = i;
			break;
		}
	}

    if (i == (len - 4)) {
    	printf("not find partition start page\n");
    	while(1);
    }

	/* get paration tabel saved block from spi param reserved member value */
    memcpy(&boot_temp, buf + offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33, 1);
	memcpy(&erase_size, buf + offset + 4 + 4*32, 4);

	/* get paration tabel saved page offset address */
 	boot_len = boot_temp*4096;
    printf("get_upadte_boot_size g_boot_len:%d\n", boot_len);
	
	/* get spi param last reserved member value*/
    printf("boot block num offset:%d\n", offset + 4 + sizeof(T_SFLASH_PHY_INFO) - 33);

	/* read out paration table from  boot_len */
	len = SPI_FLASH_PAGE_SIZE*CONFIG_PARATION_PAGE_CNT;
	addr = (unsigned int *)paration_table;
	printf("read config info(%08x) from offset:%d, len:%d.\n", addr, boot_len, len);

	if (!spi_flash_read((T_U32)(boot_len/SPI_FLASH_PAGE_SIZE), (T_U8*)addr, CONFIG_PARATION_PAGE_CNT)) {
		
		printf("spi_flash read error.\n");
		while(1);
	}

	nr_parts = *(unsigned long *)addr;
	addr++;

	printf("nr_parts=0x%x\n", nr_parts);
	if (nr_parts <= 0 || nr_parts > 15) {
		printf("partition count invalid\n");
		while(1);
	}

	//≤È’“Kernel
	printf("size part=%d Byte\n", sizeof(T_PARTITION_TABLE_INFO));
	for(i = 0; i < nr_parts; i++)
    {
		bMatch = AK_FALSE;

		tmpcfg = (T_PARTITION_TABLE_INFO *)(addr + i*sizeof(T_PARTITION_TABLE_INFO) / sizeof(unsigned long));
        printf("Read file %s\r\n",tmpcfg->partition_info.name);
		
		if (0 == strncmp(tmpcfg->partition_info.name, fileName, PARTITION_NAME_LEN)) {
			
			memcpy(&binbuf, &(tmpcfg->ex_partition_info), sizeof(T_EX_PARTITION_CONFIG));
			printf(" ##file_length:0x%x ##load_addr:0x%x ##start_page:%x\r\n offset:0x%x.\r\n",
					binbuf.file_length, 
					binbuf.ld_addr, 
					tmpcfg->partition_info.start_pos, 
					tmpcfg->partition_info.start_pos);
			printf(" ##backup_page:%x, file_name:%s.\n", binbuf.backup_page, tmpcfg->partition_info.name);
			memcpy(pcfg, tmpcfg, sizeof(T_PARTITION_TABLE_INFO));
			bMatch = AK_TRUE;
			break;
		}  
    }

    return bMatch;
}

#ifdef CONFIG_SPI_NAND_FLASH
void boot_spinand(void)
{
	T_U32 runaddr;
	//load bios
	printf("load spinand bios ......\r\n");
	spi_nand_init(SPI_ID0);


	//
	if(1)
	{
		//enter kernel
		fwl_spinand_load_kernel(&runaddr);
		
		printf("Load bios from spi NAND ok\n");
	}
	else
	{
		//enter test program
		fwl_spinand_load_test_program(&runaddr);
		
		printf("Load test program from spi NAND ok\n");
	}
	
	run((T_VOID *)runaddr);
}
#else

void boot_spiflash(void)
{

	T_PARTITION_TABLE_INFO cfg;

	T_BIN_CONFIG binbuf;
	T_U32 *pData = AK_NULL;
	
	//load bios
	printf("load spiflash bios ......\r\n");
	
	spi_flash_init(SPI_ID0);
	memset(&cfg, 0, sizeof(T_PARTITION_TABLE_INFO));
	memset(&binbuf, 0, sizeof(T_BIN_CONFIG));
	if(!readcfgbyname(&cfg,CONFIG_BIOS_NAME))
	{
		printf("can not find %s burn info\n",CONFIG_BIOS_NAME);
		while(1);
	}

	memcpy(&binbuf, &(cfg.ex_partition_info), sizeof(T_EX_PARTITION_CONFIG));

	printf("##file_length:0x%x ##load_addr:0x%x ##start_page:%x ##offset:0x%x.\r\n",
					binbuf.file_length, 
					binbuf.ld_addr, 
					cfg.partition_info.start_pos, 
					cfg.partition_info.start_pos);
  pData = (T_U32 *)binbuf.ld_addr;

  if(!Fwl_SPI_FileRead(&cfg, (T_U8 *)pData, cfg.partition_info.ksize*1024))
  {
      printf("read BIOS fail\n");
    while(1);
  }
	
	printf("Load bios from spiflash successfuly!\r\n");
	
	run((T_VOID *)binbuf.ld_addr);
}
#endif

#define PLL_CLK_MIN	180

T_U32 get_asic_pll_clk(T_VOID)
{
	T_U32 pll_m, pll_n, pll_od;
	T_U32 asic_pll_clk;
	T_U32 regval;

	regval = REG32(CLK_ASIC_PLL_CTRL);
	pll_od = (regval & (0x3 << 12)) >> 12;
	pll_n = (regval & (0xf << 8)) >> 8;
	pll_m = regval & 0xff;

	asic_pll_clk = (12 * pll_m)/(pll_n * (1 << pll_od));

	if ((pll_od >= 1) && ((pll_n >= 2) && (pll_n <= 6)) 
		&& ((pll_m >= 84) && (pll_m <= 254)))
		return asic_pll_clk;

	return 0;
}

T_U32 get_vclk(T_VOID)
{
	T_U32 regval;
	T_U32 div;
	
	regval = REG32(CLK_ASIC_PLL_CTRL);
	div = (regval & (0x7 << 17)) >> 17;
	if (div == 0)
		regval = get_asic_pll_clk() >> 1;
	else
		regval = get_asic_pll_clk() >> div;

	return regval;
}

T_U32 get_asic_freq(T_VOID)
{
	T_U32 regval;
	T_U32 div;

	regval = REG32(CLK_ASIC_PLL_CTRL);
	div = regval & (1 << 24);
	if (div == 0)
		regval =  get_vclk() * 1000000;
	else
		regval = (get_vclk() >> 1) * 1000000;

	return regval;
}

void boot_main(void)
{   

	T_U32 baudrate;
	T_U32 asic_freq;

	asic_freq = get_asic_freq();
	baudrate = asic_freq/115200 - 1;

	l2_init();

	console_init(baudrate);

	
	
	//MMU_Init(_MMUTT_STARTADDRESS);
	
#ifdef CONFIG_SPI_NAND_FLASH
	printf("\r\nCloud39Ev300 spinandboot %s \r\n", SPI_BOOT_VERSION);
	boot_spinand(); 
#else
	printf("\r\nCloud39Ev300 spiboot %s \r\n", SPI_BOOT_VERSION);
	boot_spiflash();
#endif
	while(1);
}

