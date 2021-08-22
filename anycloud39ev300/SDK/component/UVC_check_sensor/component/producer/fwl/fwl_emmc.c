#include "fwl_emmc.h"
#include "arch_mmc_sd.h"
#include "fha.h"

static T_pCARD_HANDLE m_hSD; 

static unsigned long SDDisk_Read(T_PMEDIUM medium, unsigned char* buf, unsigned long sector, unsigned long size)
{
    if(!sd_read_block(m_hSD, sector, buf, size))
    {
        return 0;
    }
    else
    {
        return size;
    } 
}

static unsigned long SDDisk_Write(T_PMEDIUM medium, const unsigned char* buf, unsigned long sector, unsigned long size)
{
    if(!sd_write_block(m_hSD, sector, buf, size))
    {
        return 0;
    }
    else
    {
        return size;
    }    
}

static bool SDDisk_Flush(T_PMEDIUM medium)
{
    return true;
}

T_PMEDIUM SDDisk_Initial(void)
{      
    unsigned long capacity = 0, BytsPerSec = 0, i;
    T_PMEDIUM medium;

    m_hSD = sd_initial(INTERFACE_SDMMC8, USE_FOUR_BUS);

    if (NULL == m_hSD)
    {
        pp_printf("SD Init mmc fail,try to init sdio\r\n");
        m_hSD = sd_initial(INTERFACE_SDIO, USE_FOUR_BUS);
    }    

    if (NULL == m_hSD)
    {
        pp_printf("SD Init Error\r\n");
        return NULL;
    }
    else
    {
        sd_get_info(m_hSD, &capacity, &BytsPerSec);
    }    
    
    medium = (T_PMEDIUM)Fwl_Malloc(sizeof(T_MEDIUM));
    if(medium == NULL)
    {
        pp_printf("SD Malloc Error\r\n");
        return NULL;
    }

    i = 0;
    while (BytsPerSec > 1)
    {
        BytsPerSec >>= 1;
        i++;
    }

    medium->SecBit = i;        
	medium->PageBit  = i + 5;
    medium->SecPerPg = 5;
    ((T_POBJECT)medium)->destroy = (F_DESTROY)Medium_Destroy;
    ((T_POBJECT)medium)->type = TYPE_MEDIUM;
    medium->read = SDDisk_Read;
    medium->write = SDDisk_Write;
    medium->flush = SDDisk_Flush;
    medium->capacity = capacity;
    medium->type = MEDIUM_SD;
    medium->msg = NULL;

    return medium;
}

//**************************************************************************************************************************

unsigned long FHA_SD_Erase(unsigned long nChip,  unsigned long nPage)
{
    return 0;  
}

unsigned long FHA_SD_Read(unsigned long nChip,  unsigned long nPage, unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen , unsigned long eDataType)
{  
    if(sd_read_block(m_hSD, nPage, pData, nDataLen))
    {
        return 0;
    }
    else
    {
        return -1;
    }  
}

unsigned long FHA_SD_Write(unsigned long nChip, unsigned long nPage, const unsigned char *pData, unsigned long nDataLen,  unsigned char *pOob, unsigned long nOobLen, unsigned long eDataType)
{
    if(sd_write_block(m_hSD, nPage, pData, nDataLen))
    {
        return 0;
    }
    else
    {
        return -1;
    }    
}

