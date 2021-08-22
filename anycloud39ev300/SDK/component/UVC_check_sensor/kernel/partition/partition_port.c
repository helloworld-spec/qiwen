#include <stdio.h>
#include <string.h>

#include "os_malloc.h"
#include "print.h"
#include "arch_spi.h"
#include "hal_spiflash.h"

#include "partition_lib.h"
#include "partition_init.h"
#include "partition_port.h"

extern T_SFLASH_PARAM spiflash_param;
static T_SPI_INIT_INFO m_Spiflash = {0};

/**
* @brief initializing spinorflash 
* @author
* @date 2016-12-22
* @param none
* @return int: success return 0, fail return -1
* @version 
*/
static int parti_spiflash_init()
{
    T_eSPI_BUS bus_wight;
	T_SFLASH_PARAM *param = &spiflash_param;;
	
	printk("flashid:%x clock:%d flag:%x ex_flag:%x\r\n",param->id,param->clock,param->flag,param->ex_flag);
    if(param->flag & SFLASH_FLAG_QUAD_WRITE)
        bus_wight = SPI_BUS4;
    else
        bus_wight = SPI_BUS1;

    if (!spi_flash_init(SPI_ID0, bus_wight))  //set spi bus line 
    {
    	printk("sflash init ERR\n");
		return -1;
    }
	
    spi_flash_set_param(param);
	m_Spiflash.pages_per_block = param->erase_size/param->page_size;
	m_Spiflash.page_size = param->page_size;
	m_Spiflash.total_size = param->total_size;
	printk("pages_per_block:%d page_size:%d total_size:%d\r\n",m_Spiflash.pages_per_block,m_Spiflash.page_size,m_Spiflash.total_size);

	return 0;
}

/************************************************************************
 * NAME:     Parti_Spiflash_Erase
 * FUNCTION  callback function, medium erase
 * PARAM:    [in] nChip--meidum chip
 *                [in] nPage--medium page
 * RETURN:   success return 0, fail return -1
**************************************************************************/
static int parti_spiflash_erase (unsigned long nChip,  unsigned long block)
{
	if ((block + 1) * (m_Spiflash.pages_per_block) > (m_Spiflash.total_size/m_Spiflash.page_size))
    {
        printk("ERR:sflash erase size more than MAX\n");
        return -1;
    } 

    if(m_Spiflash.pages_per_block * m_Spiflash.page_size == 4096)
     {
     	if (!spi_flash_erase_sector(block))
        {
           printk("sflash erase 4k ERR\n");
           return -1;
        }   
     }
     else
     {
         if(!spi_flash_erase(block))
         {
           printk("sflash erase 64k ERR\n");
           return -1;      
         }   
     }
	 
    return 0;
}

/************************************************************************
 * NAME:     Parti_Spiflash_Read
 * FUNCTION  callback function, medium read
 * PARAM:    [in] nChip-----meidum chip
 *           [in] nPage-----medium page
 *           [out]pData-----need to read data pointer addr
 *           [in] nDataLen--need to ren data length
 *                          nand(unit byte)
 *                          SD(unit sector count(1sec = 512byte))
 *                          SPI(unit page count, page size in platform define, generally is 256bytes)
 *           [out]pOob------Spare area£ºOut Of Band, only nand use
 *           [in] nOobLen---Spare area length
 *           [in] eDataType-burn medium data type
 *                          nand -- E_FHA_DATA_TYPE
 *                          SD----- MEDIUM_EMMC
 *                          SPI---- MEDIUM_SPIFLASH
 * RETURN:   success return 0, fail retuen -1
**************************************************************************/
static int parti_spiflash_read(unsigned long nChip,  unsigned long nPage, unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen , unsigned long eDataType)
{ 
	unsigned long i;
	unsigned long page = nPage; 
	unsigned char *buf = pData;
	unsigned long tmp_len; 
	
    if ((nPage + nDataLen) > (m_Spiflash.total_size/m_Spiflash.page_size))
    {
    	printk("ERR:sflash read size more than MAX\n");
        return -1;
    } 

	#define READ_SPIFLASH_PARTITION 32  //64 L2 data erro; chech future
	while(nDataLen > 0)
	{
		tmp_len = (nDataLen > READ_SPIFLASH_PARTITION)?READ_SPIFLASH_PARTITION:nDataLen;
		nDataLen -= tmp_len;
		if (!spi_flash_read(page , buf, tmp_len))
		{
			printk("sflash read ERR\n");
			return -1;
		}
		page += tmp_len;
		buf  += tmp_len * m_Spiflash.page_size;
	}
	
    return 0;
}

/************************************************************************
 * NAME:     Parti_Spiflash_Write
 * FUNCTION  callback function, medium write
 * PARAM:    [in] nChip-----meidum chip
 *           [in] nPage-----medium page
 *           [in] pData-----need to write data pointer addr
 *           [in] nDataLen--need to write data length
 *                          nand(unit byte)
 *                          SD(unit sector count(1sec = 512byte))
 *                          SPI(unit page count, page size in platform define, generally is 256bytes)
 *           [in] pOob------Spare area£ºOut Of Band, only nand use
 *           [in] nOobLen---Spare area length
 *           [in] eDataType-burn medium data type
 *                          nand -- E_FHA_DATA_TYPE
 *                          SD----- MEDIUM_EMMC
 *                          SPI---- MEDIUM_SPIFLASH
 * RETURN:   success return 0, fail retuen -1
**************************************************************************/
static int parti_spiflash_write(unsigned long nChip, unsigned long nPage, const unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen, unsigned long eDataType)
{	
    if ((nPage + nDataLen) > (m_Spiflash.total_size/m_Spiflash.page_size))
    {
    	printk("ERR:sflash write size more than MAX\n");
        return -1;
    } 
        
    if (!spi_flash_write(nPage, (unsigned char *)pData, nDataLen))
	{
		printk("sflash write ERR\n");
        return -1;
	}    
   
    return 0;
}

static void * parti_ramalloc(unsigned long size)
{
     return (Fwl_Malloc(size));
}

static void * parti_ramfree(void * var)
{
     return (Fwl_Free(var));
}

static T_FHA_LIB_CALLBACK parti_CB =
{
    .Erase        =parti_spiflash_erase,
    .Write        =parti_spiflash_write,
    .Read 		  =parti_spiflash_read,
    .ReadNandBytes=NULL,
    .RamAlloc     =parti_ramalloc,
    .RamFree      =parti_ramfree,
    .MemSet       =(void *)memset,
    .MemCpy       =(void *)memcpy,
    .MemCmp       =(FHA_MemCmp)memcmp,
    .MemMov		  =(void *)memmove,
    .Printf		  =(FHA_Printf)printk
};

/**
* @brief initializing partition port and spinorflash port
* @author
* @date 2016-12-22
* @param none
* @return int: success return 0, fail return -1
* @version 
*/
int partition_port_init()
{ 
    int ret = 0;
	
	ret= parti_spiflash_init();
	if(ret < 0)
    {
		return -1;
	}
	
	ret = partition_init(&parti_CB, (void *)&m_Spiflash, 0);
	if(ret < 0)
    {
        printk("partition init ERR\n");
		return -1;
	}
	
	return 0;
}

