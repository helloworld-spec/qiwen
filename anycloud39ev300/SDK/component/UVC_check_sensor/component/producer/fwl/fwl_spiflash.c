#include "anyka_types.h"
#include "fwl_spiflash.h"
//#include "partition.h"

//#include "fha.h"

T_PSPIFLASH m_pSpiflash = NULL;

extern unsigned char g_no_need_erase;

extern unsigned long g_parttion_start_page;

extern signed long pp_printf(unsigned char * s, ...);


bool Fwl_SPIHWInit(unsigned long *ChipID, unsigned long *ChipCnt)
{
    if (NULL == ChipID || NULL == ChipCnt)
    {
        return false;
    }
    
    if (!spi_flash_init(SPI_ID0, SPI_BUS1))
    {
        return false;
    }    
 
    *ChipID  = spi_flash_getid();
    *ChipCnt = 1;
    pp_printf("flash_id is 0x%x\n", *ChipID);

    return true;
}

T_PSPIFLASH Fwl_SPIFlash_Init(T_SFLASH_PARAM *param)
{
    T_PSPIFLASH pSPIFlash = NULL;

    if (NULL == param || 0 == param->page_size)
    {
        pp_printf("Fwl_SPIFlash_Init(): Input para is ERROR!\r\n");
        return NULL;
    }

	pp_printf("id:%d, totalsize:%d, pagesize:%d, program size:%d, secsize:%d, clock:%d, flag:0x%x, mask:0x%x, ex_flag:0x%x\r\n", 
        param->id,
        param->total_size,
        param->page_size,
		param->program_size,
		param->erase_size,
		param->clock,
		param->flag,
		param->protect_mask,
		param->ex_flag);
    
    spi_flash_set_param(param);
    pSPIFlash = (T_PSPIFLASH)Fwl_Malloc(sizeof(T_SPIFLASH));

    if(NULL == pSPIFlash || 0 == param->page_size)
    {
        return NULL;
    }

    pSPIFlash->total_page = param->total_size / param->page_size;
    pSPIFlash->page_size  = param->page_size;
    pSPIFlash->PagesPerBlock = param->erase_size / param->page_size;
    pSPIFlash->ReadPage   = Fwl_SPIFlash_ReadPage;
    pSPIFlash->WritePage  = Fwl_SPIFlash_WritePage;
    pSPIFlash->EraseBlock = Fwl_SPIFlash_EraseBlock;

    m_pSpiflash = pSPIFlash;

    return pSPIFlash;
}


int  Fwl_SPIFlash_Erase_ALL_Block()
{
    unsigned long i = 0;
    unsigned long page_per_block_64K = 65536/m_pSpiflash->page_size;
    pp_printf("Fwl_SPIFlash_Erase_ALL_Block:%d, %d, %d\r\n", m_pSpiflash->total_page, page_per_block_64K, m_pSpiflash->total_page/page_per_block_64K);
    for(i = 0; i < m_pSpiflash->total_page/page_per_block_64K; i++)
    {
        //pp_printf("erase block:%d\r\n", i);
        pp_printf("e.");
        if (!spi_flash_erase(i))
        {
            return -1;
        }
     }
    pp_printf("erase end B:%d\r\n", i-1);
        
        
    return 0;
}


E_SPIFLASHERRORCODE Fwl_SPIFlash_WritePage(T_PSPIFLASH spiFlash, unsigned long page, unsigned char data[])
{  
	if (!spi_flash_write(page, data, 1))
	{
        return SF_FAIL;
	}    
   
    return SF_SUCCESS;
}

E_SPIFLASHERRORCODE Fwl_SPIFlash_ReadPage(T_PSPIFLASH spiFlash, unsigned long page, unsigned char data[])
{
    if (!spi_flash_read(page, data, 1))
    {
        return SF_FAIL;
    }   
    
    return SF_SUCCESS;
}

E_SPIFLASHERRORCODE  Fwl_SPIFlash_EraseBlock(T_PSPIFLASH spiFlash, unsigned long Block, bool block_size_64K)
{
    if(g_no_need_erase == 0)
    {
        if(!block_size_64K)
        {
            if (!spi_flash_erase_sector(Block))
            {
                return SF_FAIL;
            } 
        }
        else
        {
            if (!spi_flash_erase(Block))
            {
                return SF_FAIL;
            }   
        }
    }
        
    return SF_SUCCESS;
}


int Partition_Write_Page(unsigned long page, unsigned long page_cnt, unsigned char data[])
{
    
    if (page + page_cnt > m_pSpiflash->total_page)
    {
        pp_printf("Partition_Write_Page %d over the max space\r\n", page + page_cnt);
        return -1;
    }

	if (!spi_flash_write(page, data, page_cnt))
	{
        return -1;
	}

    return 0;
}

int Partition_Read_Page(unsigned long page, unsigned long page_cnt, unsigned char data[])
{
    unsigned long page_cnt_idex = 0, page_idex = 0;

    if (page + page_cnt > m_pSpiflash->total_page)
    {
        pp_printf("Partition_Read_Page %d over the max space\r\n", page + page_cnt);
        return -1;
    }

    while(1)
    {
        if(page_cnt > 32)//一次读不超过8K，芯片有控制
        {
            page_cnt_idex = 32;
        }
        else
        {
            page_cnt_idex = page_cnt;
        }
        
        if (!spi_flash_read(page+page_idex, &data[page_idex*m_pSpiflash->page_size], page_cnt_idex))
        {
            return -1;
        } 
        page_cnt -= page_cnt_idex;
        page_idex += page_cnt_idex;

        if(page_cnt == 0)
        {
            break;
        }
    }
    
    return 0;
}


//**************************************************************************************************************************

int FHA_Spi_Erase(unsigned long nChip,  unsigned long nBlock)
{
    //unsigned long nBlock = nPage / m_pSpiflash->PagesPerBlock;

    //由于创建分区表时。经常需要对分区表进行擦写的
    if(g_parttion_start_page/m_pSpiflash->PagesPerBlock == nBlock 
        || g_parttion_start_page/m_pSpiflash->PagesPerBlock+1 == nBlock)
    {
       if(m_pSpiflash->PagesPerBlock * m_pSpiflash->page_size == 4096)
       {
            if (!spi_flash_erase_sector(nBlock))
            {
                return -1;
            }  
       }
       else
       {
            if (!spi_flash_erase(nBlock))
            {
                return -1;
            } 
       }
    }
    else
    {
        if(g_no_need_erase == 0)
        {
            if ((nBlock + 1) * m_pSpiflash->PagesPerBlock > m_pSpiflash->total_page)
            {
                pp_printf("erase sector %d over the max space\r\n", nBlock);
                return -1;
            } 

            if(m_pSpiflash->PagesPerBlock * m_pSpiflash->page_size == 4096)
            {
                
                if (!spi_flash_erase_sector(nBlock))
                {
                    return -1;
                }   
            }
            else
            {
                if (!spi_flash_erase(nBlock))
                {
                    return -1;
                }   
            }
        }
    }

    return 0;
}

int FHA_Spi_Read(unsigned long nChip,  unsigned long nPage, unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen , unsigned long eDataType)
{ 
#if 1

	unsigned long i;

    if ((nPage + nDataLen) > m_pSpiflash->total_page)
    {
        pp_printf("read page %d over the max space\r\n", (nPage + nDataLen));
        return -1;
    } 


	for (i = 0; i < nDataLen; i++)
	{
		if (!spi_flash_read(nPage + i, pData + i * m_pSpiflash->page_size, 1))
		{
			return -1;
		}
	}


#else
	
    if (!spi_flash_read(nPage, pData, nDataLen))
    {
        return FHA_FAIL;
    }   
#endif

    return 0;
}

int FHA_Spi_Write(unsigned long nChip, unsigned long nPage, unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen, unsigned long eDataType)
{	
    if ((nPage + nDataLen) > m_pSpiflash->total_page)
    {
        pp_printf("write page %d over the max space\r\n", (nPage + nDataLen));
        return -1;
    } 
    
    if (!spi_flash_write(nPage, pData, nDataLen))
	{
        return -1;
	}    
   
    return 0;
}

 
