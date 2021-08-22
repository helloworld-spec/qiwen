#include "anyka_cpu.h"
#include "hal_spiflash.h"

#ifdef DEBUG_OUTPUT
#define    SPI_DEBUG     printf
#else
#define    SPI_DEBUG     printf
#endif
#define    BUF2MEM                0
#define    MEM2BUF                1
#define    SPI_MASTER_TRANFINISH           (1<<8)
#define    SPI_RXBUF_HALFEMPTY             (1<<6)
#define    SPI_TXBUF_HALFEMPTY             (1<<2)
#define    BUF_NULL     0xff
#define SPIFLASH_PAGE_SIZE 256

static T_SFLASH_PARAM param = {0};

void sflash_init()
{
    T_eSPI_BUS bus_width;
    
    param = spiflash_param;
	
    if(param.flag & SFLASH_FLAG_QUAD_READ)
        bus_width = SPI_BUS4;
    else
        bus_width = SPI_BUS1;

    printf("SPI FLASH:id=0x%x, buswidth=%d, clock=%dHz\r\n",
    	param.id, (bus_width==SPI_BUS4) ? 4:1, param.clock);
        
    if (!spi_flash_init(SPI_ID0, bus_width))  //spi bus line ?
    {
        printf("SPI initialized fail!\r\n");
    }
    spi_flash_set_param(&param);
    
}

T_S32  Fwl_SPI_FileRead(T_FILE_CONFIG *pFile, T_U8 *buffer, T_U32 count)
{
	T_S32 ret = -1;
	T_U32 page = 0;
	T_U32 page_cnt = 0;
	T_U32 addr = 0;
	T_U32 i;
    
	if ((AK_NULL == buffer)||(AK_NULL == pFile))
	{
		return ret;
	}	
	if (count > pFile->file_length)
	{
		count = pFile->file_length;
	}
	
	page = pFile->start_pos/param.page_size;
	page_cnt = (count+SPIFLASH_PAGE_SIZE-1)/SPIFLASH_PAGE_SIZE;

    if(!spi_flash_read(page,buffer,page_cnt))
    {       
        printf("read page:%d info fail\r\n",page);        
        return -1;
    }

	return count;
}

