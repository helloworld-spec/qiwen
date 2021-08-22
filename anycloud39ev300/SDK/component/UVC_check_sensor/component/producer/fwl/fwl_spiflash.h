#ifndef _FWL_SPI_FLASH_H_
#define _FWL_SPI_FLASH_H_

#include "hal_spiflash.h"

typedef enum tag_SPIFlashErrorCode
{
    SF_SUCCESS        =    ((unsigned short)1),
    SF_FAIL           =    ((unsigned short)0),           //FOR DEBUG/
}E_SPIFLASHERRORCODE; 

typedef struct SPIFlash T_SPIFLASH;
typedef struct SPIFlash* T_PSPIFLASH;

typedef E_SPIFLASHERRORCODE  (*fSPIFlash_WritePage)(T_PSPIFLASH spiFlash, unsigned long page, unsigned char data[]);
typedef E_SPIFLASHERRORCODE  (*fSPIFlash_ReadPage)(T_PSPIFLASH spiFlash, unsigned long page, unsigned char data[]);
typedef E_SPIFLASHERRORCODE  (*fSPIFlash_EraseBlock)(T_PSPIFLASH spiFlash, unsigned long block, bool block_size_64K);

struct SPIFlash
{
    unsigned long total_page;
    unsigned long page_size;
    unsigned long PagesPerBlock;
    fSPIFlash_WritePage WritePage;
    fSPIFlash_ReadPage ReadPage;
    fSPIFlash_EraseBlock EraseBlock;
};

bool Fwl_SPIHWInit(unsigned long* ChipID, unsigned long* ChipCnt);
T_PSPIFLASH Fwl_SPIFlash_Init(T_SFLASH_PARAM *param);
E_SPIFLASHERRORCODE Fwl_SPIFlash_WritePage(T_PSPIFLASH spiFlash, unsigned long page, unsigned char data[]);
E_SPIFLASHERRORCODE Fwl_SPIFlash_ReadPage(T_PSPIFLASH spiFlash, unsigned long page, unsigned char data[]);
E_SPIFLASHERRORCODE Fwl_SPIFlash_EraseBlock(T_PSPIFLASH spiFlash, unsigned long block, bool block_size_64K);

int Partition_Read_Page(unsigned long page, unsigned long page_cnt,  unsigned char data[]);
int Partition_Write_Page(unsigned long page, unsigned long page_cnt,  unsigned char data[]);
int  Fwl_SPIFlash_Erase_ALL_Block(void);


int FHA_Spi_Erase(unsigned long nChip,  unsigned long block);
int FHA_Spi_Read(unsigned long nChip,  unsigned long nPage, unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen , unsigned long eDataType);
int FHA_Spi_Write(unsigned long nChip, unsigned long nPage,  unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen, unsigned long eDataType);
#endif
