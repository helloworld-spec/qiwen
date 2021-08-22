#include "anyka_types.h"
#include <stdarg.h>
#include "hal_spiflash.h"
#include "arch_uart.h"
#include "freq.h"
#include "memapi.h"
#include "Fwl_compress.h"
#include "anyka_cpu.h"


#define     SPIBOOT_VER                     "V1.0.02"
//#define  DEBUG_OUTPUT
#ifdef DEBUG_OUTPUT
#define     DEG_TEST printf
#else
#define     DEG_TEST 
#endif


#define RAM_SIZE_8MB             (8<<20)
#define RAM_SIZE_16MB            (16<<20)

#define SYS_STACK_SIZE           (64<<10)
#define SYS_HEAP_SIZE            (2<<20 | 512<<10) //(3<<20)
#define SYS_BOOT_SIZE            (512<<10)
#define _MMUTT_SIZE				 (16<<10)


static T_U32	ram_addr_start_fixed  = 0x80000000U;
static T_U32	sys_boot_start_fixed  = 0x80DFFE00U;
static T_U32	sys_stack_start_fixed = 0x80ff0000U;

static T_U32	mmutt_addresss_start = 0;

static T_U32 	sys_heap_start = 0;	//堆起始地址
static T_U32	sys_heap_end   = 0;	//堆起始地址

static T_U32	ram_size = 0;		//内存的实际大小

#ifdef COMPRESS
static T_U32	compress_mmi_bin_size_limit = 0;	// 压缩文件的大小限制
static T_U32	decom_buf_size_limit = 0;			// 解压后的文件占用大小限制
#endif


T_CHIP_CONFIG_INFO chip_config;

typedef void (*pfun)(void);

#if 0   //调试用代码，可以不去掉

T_VOID writezero(T_U8 *data,T_U32 len)
{
    T_U32 i=0;
    
    for(i = 0; i < len; i++)
    {
       data[i] = 0;
    }

}

T_VOID disfun(T_U8 *data,T_U32 len)
{
    T_U32 i=0;
    
    for(i = 0; i < len; i++)
    {
        printf("%02x ",data[i]);
        if((i & 0xF) == 0xF)
            printf("\n");
    }
    printf("Dis len:%d:\n",i);
}
T_VOID writenum_fun(T_U8 *data,T_U32 pos, T_U32 len)
{
    T_U32 i=0;
    len +=pos; 
    for(i = pos; i < len; i++)
    {
       data[i] = i&0xFF;
    }
}
#endif
extern T_SFLASH_PARAM spiflash_param;

T_BOOL readcfgbyname(T_FILE_CONFIG *BiosConfig, T_U8 *fileName)
{
    T_U32 *pData=AK_NULL;
    T_U32 file_cnt;
    T_FILE_CONFIG *pFileCfg;
    T_U32 i;
    T_BOOL bMatch;
	T_U32 table_add = spiflash_param.table_block*spiflash_param.erase_size/spiflash_param.page_size;
    if (AK_NULL == fileName)
    {
        printf("name is NULL\r\n");
        return AK_FALSE;
    }
    DEG_TEST("file cnt:b=%d,e=%d\r\n",spiflash_param.table_block,spiflash_param.erase_size);
    pData = (T_U32 *)ram_addr_start_fixed;
    //read file info;
    if(!spi_flash_read(table_add, (T_U8 *)pData, 1))
    {
        printf("read page:%d info fail\r\n",table_add);
        while(1);
    }
    memcpy(&file_cnt, pData, sizeof(T_U32));
    DEG_TEST("file cnt:%d\r\n",file_cnt);
	
    bMatch = AK_FALSE;
    pData ++; //第一个字存的文件个数,按照小段模式排列
    //烧录各文件信息列表从第五个字节开始
    pFileCfg = (T_FILE_CONFIG *)(pData);
    DEG_TEST("file name:%s\r\n",pFileCfg->file_name);


    for(i = 0; i < file_cnt; i++)
    {
        bMatch = AK_FALSE;
        
        if (0 == strncmp(pFileCfg->file_name,fileName,6))
        {
            memcpy(BiosConfig, pFileCfg, sizeof(T_FILE_CONFIG));
            bMatch = AK_TRUE;
            break;
        }

        pFileCfg++;
    }
    DEG_TEST("bMatch= %d\r\n",bMatch);

    return bMatch;
}

void boot_v10(T_FILE_CONFIG* BiosConfig)
{
    T_U32 i;
    T_U32 *pSrcData = AK_NULL;
    T_VOID (*biosEntryFun)(T_VOID) = AK_NULL;

    T_U32 address=0,decomLen=0;

    //load bios
    printf("Loading %s...\r\n",BiosConfig->file_name);
	
    DEG_TEST("Burn Info:start_pos=%d,Len=%d bytes,RunAddr=0x%x\r\n", 
							    BiosConfig->start_pos,
							    BiosConfig->file_length,
							    BiosConfig->ld_addr);

 #ifdef COMPRESS
	if (BiosConfig->file_length > compress_mmi_bin_size_limit)
	{
		printf("\n@err:MMI bin file length is:0x%x, but limit_size is:0x%x, abort run.\r\n", BiosConfig->file_length,
			compress_mmi_bin_size_limit);
		while(1);
	}

	if (RAM_SIZE_8MB == ram_size)
	{
    	pSrcData = (T_U32 *)((sys_heap_start - BiosConfig->file_length + 3) & (~0x03));
    }
    else
    {
		pSrcData = (T_U32 *)((ram_addr_start_fixed + RAM_SIZE_8MB + 3) & (~0x03));
    }
#else
    pSrcData = (T_U32 *)BiosConfig->ld_addr;
#endif

    //read bios data
    if(!Fwl_SPI_FileRead(BiosConfig, (T_U8 *)pSrcData, BiosConfig->file_length))
    {
        printf("read KERNEL fail\r\n");
        while(1);
    }

#ifdef COMPRESS
    decomLen = Fwl_DeComImg((T_pDATA)pSrcData, (T_U32)BiosConfig->file_length,(T_pDATA)BiosConfig->ld_addr, decom_buf_size_limit);
	if (decom_buf_size_limit == decomLen)
	{
		printf("\n@err:isn't Decompressed inextenso, abort run.\r\n");
		while(1);
	}
#else
    decomLen = 1;
#endif

	
    if (decomLen > 0)
    {
        biosEntryFun = (T_VOID(*)(T_VOID))(BiosConfig->ld_addr);
        
        for (i=0;i<1024/4;i++)
        {
			if (((T_U32*)BiosConfig->ld_addr)[i] == 0x50495053)// 0x50495053=SPIP
            {
                memcpy(((T_U32*)BiosConfig->ld_addr)+i+1, &spiflash_param,
                    sizeof(T_SFLASH_STRUCT)); 
                break;
            }
        }
#ifdef COMPRESS		
        printf("Decompress %s Ok! Jump To 0x%x\n",
            BiosConfig->file_name,
            biosEntryFun);  //2016.8.29
#else
        printf("Read %s Ok! Jump To 0x%x\r\n",
            BiosConfig->file_name,
            biosEntryFun);  //2016.8.29
#endif
        MMU_InvalidateDCache();
        
        biosEntryFun();

    }


}


//!!!!!don't enable interrupt in this function!!!!!
void CMain()
{
	T_U32 i;
	T_U8  fileName[] = "KERNEL";
    T_FILE_CONFIG BiosConfig;

    T_U32 reg_v = 0; 
    T_U32 freq_asic;

	chip_config.uart_ID = uiUART0;

	ram_size = RAM_SIZE_16MB; //H240的RAM大小都大于16M，而且H240没有现成的RAM大小寄存器读取，boot 默认设置为16M
	
	mmutt_addresss_start = ram_addr_start_fixed + RAM_SIZE_8MB - _MMUTT_SIZE;
	MMU_Init(mmutt_addresss_start);
	
	sys_heap_start = (ram_addr_start_fixed + ram_size - SYS_HEAP_SIZE + 3) & (~0x03);
	sys_heap_end = (ram_addr_start_fixed + ram_size - 3) & (~0x03);
	
	#ifdef COMPRESS
	compress_mmi_bin_size_limit = ram_size - RAM_SIZE_8MB - SYS_HEAP_SIZE;
	decom_buf_size_limit = RAM_SIZE_8MB - (1<<20) - SYS_HEAP_SIZE;
	 
    Init_MallocMem(sys_heap_start, sys_heap_end);
	#endif
  #if 1 //for start time
	//CPU PLL 400M
	while( (inl(CPU_CLOCK_DIV_REG)  & (1<<28)) !=0)
	{;}

    reg_v = inl(ASIC_CLOCK_DIV_REG);
	reg_v &= ~0x3fff;
	reg_v |=((1<<12)|(3<<8)|200 );
	outl(reg_v,ASIC_CLOCK_DIV_REG);
	
    reg_v = inl(CPU_CLOCK_DIV_REG);
	reg_v |=(1<<28);
	outl(reg_v,CPU_CLOCK_DIV_REG);  /// asic  pll 400M, asic clock 200M

	while( (inl(CPU_CLOCK_DIV_REG)  & (1<<28)) !=0)
	{;}
#endif


    
    uart_init(chip_config.uart_ID, 115200, get_asic_freq());
    
    l2_init();
    memset(&BiosConfig, 0, sizeof(T_FILE_CONFIG));
    
    printf("SPIBOOT_VER: %s\r\n",SPIBOOT_VER); 

    sflash_init(); 
    
   if(readcfgbyname(&BiosConfig,fileName))
   {
       boot_v10(&BiosConfig);       
   }
   else
   {
       printf("can not find %s burn info\r\n",fileName);
	   
	   while(1)
	   {
	   	;
	   }
   }

   while(1);   
}




