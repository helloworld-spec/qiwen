/**@file  hal_sd.c
 * @brief Implement sd operations of how to control sd.
 *
 * This file implement sd driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */
#include "anyka_types.h"
#include "anyka_cpu.h"
#include "l2.h"
#include "hal_print.h"
#include "sysctl.h"
#include "sd.h"
#include "sdio.h"
#include "hal_sd.h"
#include "hal_common_sd.h"
#include "hal_common_sdio.h"

#include "drv_api.h"
#include "drv_module.h"


static volatile T_pSD_DEVICE g_pCurSdDevice; 


//The interface shall be INTERFACE_SDMMC4,INTERFACE_SDMMC8,INTERFACE_SDIO
#define SD_DRV_PROTECT(interface) \
        do{ \
            DrvModule_Protect(DRV_MODULE_SDMMC);\
			if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)\
			sdio_set_interface(interface);\
			else\
            set_interface(interface);\
        }while(0)

//The interface shall be INTERFACE_NOT_SD
#define SD_DRV_UNPROTECT(interface) \
        do{ \
			if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)\
			sdio_set_interface(interface);\
			else\
            set_interface(interface);\
			DrvModule_UnProtect(DRV_MODULE_SDMMC);\
        }while(0)


/*for csd cid.... register decoder*/
#define UNSTUFF_BITS(resp,start,size)   stuff_bits(resp,start,size)


static signed long  stuff_bits (unsigned short * resp, signed long start, signed long size);
static bool send_acmd(unsigned char cmd_index, unsigned char resp, unsigned long arg);
static bool sd_get_cid(unsigned long *cid);
static bool sd_get_csd(unsigned long *csd);
static unsigned char   sd_nego_volt(unsigned long volt);
static unsigned char   sd_get_ocr(unsigned long *ocr );
static bool sd_set_block_len(unsigned long block_len);
static bool sd_get_card_status( unsigned long *status_buf );
static unsigned char   mmc_nego_volt(unsigned long volt);
static bool sd_rw_block(unsigned long block_src, unsigned char *databuf, unsigned long size, unsigned long mode,unsigned char dir);
static bool sd_get_sd_status(unsigned char *resp);
static bool mmc_get_extcsd(unsigned long* ext_csd);
static void sd_get_capacity();
static bool sd_switch_hs(bool hs);
static bool sd_get_scr(unsigned char *scr );
static unsigned char   sd_get_spec_vers();

/**
 * @brief   mmc4.3 later card switch partition
 *
 * If card spec vers is mmc4.3 later,this func should be called to switch partition 
 * @author Huang Xin
 * @date 2010-07-14
 * @param handle[in] card handle,a pointer of void 
 * @param part[in] the selected partition
 * @return bool
 * @retval  true: switch successfully
 * @retval  false: switch failed
 */
bool emmc_switch_partition(T_pCARD_HANDLE handle,T_eCARD_PARTITION part)
{
#if 0
    unsigned long arg_value = 0;
    unsigned long cfg = 0;
    bool ret = true;

    if (NULL == handle || part >= PARTITION_NUM )
    {
        akprintf(C1, M_DRVSYS, "The SD card handle or part invalid\n");
        return false;
    }   
    
    SD_DRV_PROTECT(((T_pSD_DEVICE)handle)->enmInterface);
    g_pCurSdDevice = (T_pSD_DEVICE)handle;

    cfg = MMC4_PARTITION_CFG(g_pCurSdDevice->ulExtCSD);
    switch (g_pCurSdDevice->enmCardType)
    {
        case CARD_MMC:
            //emmc 4.3 later
            if (0x4 <= MMC_SPEC_VERSION(g_pCurSdDevice->ulCSD) 
               && 3 <= MMC4_EXT_CSD_REV(g_pCurSdDevice->ulExtCSD))
            {
                //mmc4.3 switch partition
                arg_value = (0x3<<24)|(179<<16)|(((cfg & ~0x7)|part)<<8);
                if (send_cmd(SD_CMD(6), SD_SHORT_RESPONSE, arg_value ))
                {
                    if (!wait_rw_complete())
                    {
                        ret = false;
                    }                    
                }   
                else
                {
                    //send command failed
                    ret = false;
                }
            }
            else
            {
                akprintf(C3, M_DRVSYS, "not mmc 4.3,unsupport switch partition\n");    
                ret = false;
            }
            break;   

        default:
            akprintf(C3, M_DRVSYS, "not mmc card,unsupport switch partition\n"); 
            ret = false;
            break;
    }

    if (ret == true)
        g_pCurSdDevice->enmPartition = part;    //save current partition
        
    SD_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return ret;
#endif	
}

/**
 * @brief read data from sd card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @param block_src[in] source block to read
 * @param databuf[out] data buffer to read
 * @param block_count[in] size of blocks to be readed
 * @return bool
 * @retval  true: read successfully
 * @retval  false: read failed
 */
bool sd_read_block(T_pCARD_HANDLE handle,unsigned long block_src, unsigned char *databuf, unsigned long block_count)
{
    bool ret = 0;
    unsigned char  retry =0;
    unsigned long read_mode, read_size, block_left,block_to_read;
    
    if (NULL == handle || NULL == databuf )
    {
        akprintf(C1, M_DRVSYS, "The SD card handle or databuf is null\n");
        return false;
    }   
    
    if ((block_src + block_count) > ((T_pSD_DEVICE)handle)->ulCapacity)
    {
        akprintf(C1, M_DRVSYS, "read address is beyond card capacity (0x%x, 0x%x)!\n", block_src, ((T_pSD_DEVICE)handle)->ulCapacity);
        return false;
    }
    
    SD_DRV_PROTECT(((T_pSD_DEVICE)handle)->enmInterface);
    
    g_pCurSdDevice = (T_pSD_DEVICE)handle;

    block_left = block_count;
    while(block_left > 0)
    {
        //In order to improve sd read or write speed,trans 64k one time
        //use  single  block mode when the transfer one block
        if(block_left >= SD_DMA_BLOCK_64K)
        {
            block_to_read = SD_DMA_BLOCK_64K;
        }
        else
        {
            block_to_read = block_left;
        }
        read_mode = (block_to_read > 1) ? SD_DATA_MODE_MULTI : SD_DATA_MODE_SINGLE;
        read_size =  block_to_read * g_pCurSdDevice->ulDataBlockLen;
        
        for (retry = 0; retry < 3; retry++)
        {            
            ret = sd_rw_block(block_src, (unsigned char *)databuf, read_size, read_mode, SD_DATA_CTL_TO_HOST);
            if (ret)
            {
                break;
            }
            else
            {
                //reset sd controller
                if (INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
                {
                    sysctl_reset(RESET_SDIO);
                }
                else
                {
                    sysctl_reset(RESET_SDMMC);
                }
                sd_set_clock(NULL,g_pCurSdDevice->ulClock);
            }

        }
        if(retry >= 3)
        {
            akprintf(C1, M_DRVSYS, "read error, src = %d, block_to_read = %d\n", block_src, block_to_read);
            SD_DRV_UNPROTECT(INTERFACE_NOT_SD);
            return ret;
        }
        block_left -= block_to_read;
        block_src += block_to_read;
        databuf += read_size;        
    };

    SD_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return ret;    
}

/**
 * @brief write data to sd card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @param block_dest[in] destation block to write
 * @param databuf[in] data buffer to write
 * @param block_count[in] size of blocks to be written
 * @return bool
 * @retval  true:write successfully
 * @retval  false: write failed
 */
bool sd_write_block(T_pCARD_HANDLE handle,unsigned long block_dest, const unsigned char *databuf, unsigned long block_count)
{       
    bool ret = 0;
    unsigned char retry = 0;
    unsigned long write_mode, write_size,block_left,block_to_write;

    if (NULL == handle || NULL == databuf )
    {
        akprintf(C1, M_DRVSYS, "The SD card handle or databuf is null\n");
        return false;
    }
    if ((block_dest + block_count) > ((T_pSD_DEVICE)handle)->ulCapacity)
    {       
        akprintf(C1, M_DRVSYS, "write address is beyond card capacity (0x%x, 0x%x)!\n", block_dest, ((T_pSD_DEVICE)handle)->ulCapacity);
        return false;
    }

    SD_DRV_PROTECT(((T_pSD_DEVICE)handle)->enmInterface);
    g_pCurSdDevice = (T_pSD_DEVICE)handle;    
    block_left = block_count;
    while(block_left > 0)
    {
        //In order to improve sd read or write speed,trans 64k one time
        //use  single  block mode when the transfer one block
        if(block_left >= SD_DMA_BLOCK_64K)
        {
            block_to_write = SD_DMA_BLOCK_64K;
        }
        else
        {
            block_to_write = block_left;
        }
        write_mode = (block_to_write > 1) ? SD_DATA_MODE_MULTI : SD_DATA_MODE_SINGLE;
        write_size =  block_to_write * g_pCurSdDevice->ulDataBlockLen;
        for (retry = 0; retry < 3; retry++)
        {
            ret = sd_rw_block(block_dest, (unsigned char*)databuf, write_size, write_mode,SD_DATA_CTL_TO_CARD);
            if (ret)
            {
                break;
            }
            else
            {
                //reset sd controller
                if (INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
                {
                    sysctl_reset(RESET_SDIO);
                }
                else
                {
                    sysctl_reset(RESET_SDMMC);
                }
                sd_set_clock(NULL,g_pCurSdDevice->ulClock);
            }                
        }
        if(retry >= 3)
        {
            akprintf(C1, M_DRVSYS, "write error,dest = %d, block_to_write = %d\n", block_dest, block_to_write);
            SD_DRV_UNPROTECT(INTERFACE_NOT_SD);
            return ret;
        }   
        block_left -= block_to_write;
        block_dest += block_to_write;
        databuf += write_size; 
    };

    SD_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return ret;
}


/**
 * @brief set the sd interface clk
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @param clock[in] clock to set
 * @return bool
 */
bool sd_set_clock(T_pCARD_HANDLE handle,unsigned long clock)
{
    bool ret = true;
    
    if (NULL != handle)
    {
        SD_DRV_PROTECT(((T_pSD_DEVICE)handle)->enmInterface);
        g_pCurSdDevice = (T_pSD_DEVICE)handle;
    }

    //for some emmc inand, should decrease card clock before mode switch
    if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
    {
		sdio_set_clock(1000000, get_asic_freq(), SD_POWER_SAVE_ENABLE);   //1Mhz
	}
	else
	{
		set_clock(1000000, get_asic_freq(), SD_POWER_SAVE_ENABLE);   //1Mhz
	}
    
    switch (g_pCurSdDevice->enmCardType)
    {
        case CARD_MMC:
            if ( (clock > MMC_HS_MODE_26M && 1 == MMC4_CARD_TYPE(g_pCurSdDevice->ulExtCSD))
              || (clock > MMC_HS_MODE_52M ))
            {
                akprintf(C1, M_DRVSYS, "HS MMC clk exceed 26M or 52M\n");
                ret = false;
            }
            else if(clock > MMC_DEFAULT_MODE_20M)
            {
                if(!sd_switch_hs(SD_HIGH_SPEED_MODE))
                {
                    akprintf(C1, M_DRVSYS, "mmc switch hs fail\n");
                    ret = false;
                }
            }
            else
            {
                if(!sd_switch_hs(SD_DEFAULT_SPEED_MODE))
                {
                    akprintf(C1, M_DRVSYS, "mmc switch default speed fail\n");
                    ret = false;
                }
            }            
            break;
        case CARD_SD:
        case CARD_COMBO:
            //switch hs
            if (clock > SD_DEFAULT_MODE_25M && clock <= SD_HS_MODE_50M)
            {                
                if(!sd_switch_hs(SD_HIGH_SPEED_MODE))
                {
                    //HS SD @50M
                    akprintf(C1, M_DRVSYS, "sd switch hs fail\n");
                    ret = false;
                }
            }
            else if (clock > SD_HS_MODE_50M)
            {
                akprintf(C1, M_DRVSYS, "HS SD clk exceed 50M\n");
                ret = false;
            }
            else
            {
                if(!sd_switch_hs(SD_DEFAULT_SPEED_MODE))
                {
                    akprintf(C1, M_DRVSYS, "sd switch default speed fail\n");
                    ret = false;
                }
            }
            break;
        default:
            break;
    }
    if(ret)
    {
        //change sd clock
        g_pCurSdDevice->ulClock =clock;
    } 

	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	{
		sdio_set_clock(g_pCurSdDevice->ulClock, get_asic_freq(), SD_POWER_SAVE_ENABLE);
	}
	else
	{
		set_clock(g_pCurSdDevice->ulClock, get_asic_freq(), SD_POWER_SAVE_ENABLE);
	}
    
    
    if (NULL != handle)
    {
        SD_DRV_UNPROTECT(INTERFACE_NOT_SD);
    }
    return ret;
}

/**
 * @brief power off sd controller
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @return void
 */
void sd_free(T_pCARD_HANDLE handle)
{
    if (NULL == handle)
    {
        akprintf(C1, M_DRVSYS, "The SD card handle is null\n");
        return;
    }  
    SD_DRV_PROTECT(((T_pSD_DEVICE)handle)->enmInterface);
    g_pCurSdDevice = (T_pSD_DEVICE)handle;
    //disable controller
    sd_close_controller(g_pCurSdDevice->enmInterface);
    //free memory
    drv_free(g_pCurSdDevice);   
    SD_DRV_UNPROTECT(INTERFACE_NOT_SD); 
}

/**
 * @brief get sd card information
 * @author Huang Xin
 * @date 2010-06-17
 * @param handle[in] card handle,a pointer of void
 * @param total_block[out] current sd's total block number
 * @param block_size[out] current sd's block size
 * @a block = 512 bytes
 * @return void
 */
void sd_get_info(T_pCARD_HANDLE handle, unsigned long *total_block, unsigned long *block_size)
{
    if (!total_block || !block_size)
        return;
        
    *total_block = 0;
    *block_size  = 0;        
    if (NULL == handle)
    {
        akprintf(C1, M_DRVSYS, "The SD card handle is null\n");
        return;
    }

    switch(g_pCurSdDevice->enmPartition)
    {
        case PARTITION_USER:
            *total_block = ((T_pSD_DEVICE)handle)->ulCapacity;
            *block_size = ((T_pSD_DEVICE)handle)->ulDataBlockLen;
            break;
        case PARTITION_BOOT1:
        case PARTITION_BOOT2:
            //emmc 4.3 later
            if (0x4 <= MMC_SPEC_VERSION(g_pCurSdDevice->ulCSD) 
               && 3 <= MMC4_EXT_CSD_REV(g_pCurSdDevice->ulExtCSD))
            {
                //boot partition size = 128KBytes * BOOT_SIZE_MULT
                *total_block = MMC4_PARTITION_SZ(((T_pSD_DEVICE)handle)->ulExtCSD) * 256;
                *block_size = ((T_pSD_DEVICE)handle)->ulDataBlockLen;
                //akprintf(C3, M_DRVSYS, "boot area size=%d\n", *total_block);
            }
            break;
        default:
            akprintf(C1, M_DRVSYS, "unknown partition!!!\n");
    }
}





/**
* @brief initial mmc sd or comob card
* @author Huang Xin
* @date 2010-06-17
* @param cif[in] card interface selected
* @param bus_mode[in] bus mode selected, can be USE_ONE_BUS or USE_FOUR_BUS
* @return T_pCARD_HANDLE
* @retval NON-NULL  set initial successful,card type is  mmc sd or comob
* @retval NULL set initial fail,card type is not mmc sd or comob card
*/
T_pCARD_HANDLE sd_initial(T_eCARD_INTERFACE cif, unsigned char bus_mode)
{
    T_pSD_DEVICE pSdCard = NULL;
    bool ret;
    unsigned long mmc_spec_version;
    unsigned char sd_status[64];

    if (INTERFACE_SDMMC4 != cif && INTERFACE_SDMMC8 != cif && INTERFACE_SDIO !=cif)
    {
        akprintf(C1, M_DRVSYS, "sd_initial():interface is invalid\n");
        return NULL;
    }
    if (USE_ONE_BUS != bus_mode && USE_FOUR_BUS != bus_mode && USE_EIGHT_BUS != bus_mode)
    {
        akprintf(C1, M_DRVSYS, "sd_initial():bus_mode is invalid\n");
        return NULL;
    }
    //SD_DRV_PROTECT(cif);
    DrvModule_Protect(DRV_MODULE_SDMMC);
    
    pSdCard = (T_pSD_DEVICE)drv_malloc(sizeof(T_SD_DEVICE));
    if (NULL == pSdCard)
    {
        akprintf(C1, M_DRVSYS, "sd_initial():drv_malloc fail\n");
        DrvModule_UnProtect(DRV_MODULE_SDMMC);
        return NULL;
    }
    g_pCurSdDevice = pSdCard;
    memset(g_pCurSdDevice,0,sizeof(T_SD_DEVICE));
    g_pCurSdDevice->enmInterface = cif;
    g_pCurSdDevice->ulVolt = SD_DEFAULT_VOLTAGE;
    
    if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	sdio_set_interface(INTERFACE_SDIO);
	else				
   	set_interface(INTERFACE_SDMMC4);

    //enable sd controller	
    sd_open_controller(cif);
    //for sdio interface, enum io and memory function, otherwise, only enum memory function
    if(INTERFACE_SDIO == cif)
    {
		g_pSdioDevice = g_pCurSdDevice;
	    g_pCurSdDevice->enmCardType = sdio_if_init_card(false, true);
    }
    else
    {
		g_pSdDevice = g_pCurSdDevice;
	    g_pCurSdDevice->enmCardType = init_card(false, true);
    }

	
    //card enumeration
   // g_pCurSdDevice->enmCardType = init_card(bInitIo, bInitMem);

    if ((CARD_UNUSABLE == g_pCurSdDevice->enmCardType) || 
         (CARD_SDIO == g_pCurSdDevice->enmCardType))
    {
        goto ERR_EXIT;
    }
    //sd card is in data transfer mode now 
    if (!sd_get_csd(g_pCurSdDevice->ulCSD))
    {
        akprintf(C1, M_DRVSYS, "get the card CSD failed !\n");
        goto ERR_EXIT;
    }    
    //card is in tran state after select card 
    //config data transfer mode working clock
    //Until the content of all CSD registers is known by the host, the fpp clock rate
    //must remain at fod because some cards may have operating frequency restrictions.
    if (!sd_set_clock(NULL,MMC_DEFAULT_MODE_20M))  //MMC_DEFAULT_MODE_20M
     {        
        akprintf(C3, M_DRVSYS, "set clk fail !\n");
        goto ERR_EXIT;
     }
	
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	{
		ret = sdio_if_select_card(g_pCurSdDevice->ulRCA);
	}
	else
	{
		ret = select_card(g_pCurSdDevice->ulRCA);
	}
    if (!ret)
    {
        akprintf(C1, M_DRVSYS, "select card fail !\n");
        goto ERR_EXIT;
    }

	
    if(!sd_set_block_len(SD_DEFAULT_BLOCK_LEN))
    {
        akprintf(C1, M_DRVSYS, "set block len fail !\n");
        goto ERR_EXIT;
    }   
    g_pCurSdDevice->ucSpecVersion = sd_get_spec_vers();
    if(SD_MMC_INVALID_SPEC_VERSION == g_pCurSdDevice->ucSpecVersion)
    {
        akprintf(C1, M_DRVSYS, "get spec vers fail !\n");
        goto ERR_EXIT;
    }
    if (g_pCurSdDevice->enmCardType == CARD_MMC)
    {

		//mmc 4.0
        if (g_pCurSdDevice->ucSpecVersion >= 4)
        {
            if (mmc_get_extcsd(g_pCurSdDevice->ulExtCSD))
            {
                akprintf(C3, M_DRVSYS, "mmc4 card type = 0x%x\n",MMC4_CARD_TYPE(g_pCurSdDevice->ulExtCSD));
                akprintf(C3, M_DRVSYS, "mmc4 sect_cnt = 0x%x\n",MMC4_SECTOR_CNT(g_pCurSdDevice->ulExtCSD));
                akprintf(C3, M_DRVSYS, "mmc4 power class = 0x%x\n",MMC4_POWER_CLASS(g_pCurSdDevice->ulExtCSD));
            }
        }

    } 
    //set bus width
    if(!sd_set_bus_width(bus_mode))
    {
        akprintf(C3, M_DRVSYS, "set bus mode fail !\n");
        goto ERR_EXIT;
    }

    //NOTE:it's better to put this step at the end of card initialization
    //if card support hs mode ,use high speed mode
    /*
     if (!sd_set_clock(NULL,MMC_DEFAULT_MODE_20M))
     {        
        akprintf(C3, M_DRVSYS, "set clk fail !\n");
        goto ERR_EXIT;
     }
*/
    //read part
    g_pCurSdDevice->ulMaxReadBlockLen = 1 << UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 80, 4);
    g_pCurSdDevice->bReadBlockPartial = UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 79, 1);
    g_pCurSdDevice->bReadBlockMisalign = UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 77, 1);

    //write part                
    g_pCurSdDevice->ulMaxWriteBlockLen  = 1<< UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 22, 4);  
    g_pCurSdDevice->bWriteBlockPartial  = UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 21, 1);      
    g_pCurSdDevice->bWriteBlockMisalign = UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 78, 1);

    //erase part        
    g_pCurSdDevice->bEraseBlockEnable = UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 46, 1);        
    g_pCurSdDevice->ulEraseSectorSize = UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 39, 7) + 1;
    
    akprintf(C3, M_DRVSYS, "tran speed = 0x%x\n",UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 96, 8));
    akprintf(C3, M_DRVSYS, "Max read data block length is  %dB\n", g_pCurSdDevice->ulMaxReadBlockLen);
    akprintf(C3, M_DRVSYS, "Partial blocks for read allowed is %d\n", g_pCurSdDevice->bReadBlockPartial);
    akprintf(C3, M_DRVSYS, "Read block misalign is %d\n", g_pCurSdDevice->bReadBlockMisalign);
    akprintf(C3, M_DRVSYS, "Max write data block length is  %dB\n", g_pCurSdDevice->ulMaxWriteBlockLen );
    akprintf(C3, M_DRVSYS, "Partial blocks for write allowed %d\n", g_pCurSdDevice->bWriteBlockPartial );
    akprintf(C3, M_DRVSYS, "Write block misalign is %d\n", g_pCurSdDevice->bWriteBlockMisalign);
    akprintf(C3, M_DRVSYS, "erase sector size is %d\n", g_pCurSdDevice->ulEraseSectorSize);
    akprintf(C3, M_DRVSYS, "cid is [%x %x %x %x]\n", g_pCurSdDevice->ulCID[0],g_pCurSdDevice->ulCID[1],g_pCurSdDevice->ulCID[2],g_pCurSdDevice->ulCID[3]);

    sd_get_sd_status(sd_status);
    sd_get_capacity();
    akprintf(C3, M_DRVSYS, "This card's capacity is %d blocks.\n", g_pCurSdDevice->ulCapacity);     

    SD_DRV_UNPROTECT(INTERFACE_NOT_SD);
    //NOTE:g_pCurSdDevice may be changed by other process at this time, "return g_pCurSdDevice" is forbidden
    return (T_pCARD_HANDLE)pSdCard;  

ERR_EXIT:
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	sdio_if_release();
	else
    sd_release();
    SD_DRV_UNPROTECT(INTERFACE_NOT_SD);
    return NULL;
    
}
/**
 * @brief Switch or expand memory card functions.
 *
 * This function is supported after sd version 1.10
 * @author Huang Xin
 * @date 2010-07-14
 * @param mode[in] mode
 * @param group[in] group index
 * @param func[in] func index
 * @param status[in] mode func status
 * @return bool
 * @retval  true: CMD sent successfully
 * @retval  false: CMD sent failed
 */
bool sd_switch_func(unsigned char mode,unsigned char group, unsigned char func, unsigned char *status)
{
    unsigned long arg;
    unsigned char status_tmp[64]= {0};
    unsigned char i = 0;
    bool ret = false;

    //before spec 1.10 
    if (g_pCurSdDevice->ucSpecVersion < 1)
    {        
        akprintf(C3, M_DRVSYS, "sd spec before 1.10,unsupport switch func\n");
        goto exit;
    }
     
    arg = mode << 31 | 0x00FFFFFF;
    arg &= ~(0xF << ((group-1) * 4));
    arg |= func << ((group-1) * 4);
    //The block length is predefined to 512bits and the use of SET_BLK_LEN command is not necessary
    g_pCurSdDevice->ulDataBlockLen = 64;
    //step1: check bus busy
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	{
		ret = sdio_trans_busy();
	}
	else
	{
		ret = sd_trans_busy();
	}

	if( ret)
    {
        goto exit;
    }
	
    //step2: send card command

	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	{
		ret = sdio_send_cmd(SD_CMD(6), SD_SHORT_RESPONSE, arg);
	}
	else
	{
		ret = send_cmd(SD_CMD(6), SD_SHORT_RESPONSE, arg);
	}

    if(ret == false)
    {
        akprintf(C3, M_DRVSYS, "mode switch command failed!\n");
        goto exit;
    }
    //step3: transfer data
    if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
    ret = sdio_trans_data_dma((unsigned long)status_tmp,64,SD_DATA_CTL_TO_HOST);
	else
    ret = sd_trans_data_dma((unsigned long)status_tmp,64,SD_DATA_CTL_TO_HOST);
    //step4: wait card status to idle
    if (!wait_rw_complete())
    {
        akprintf(C3, M_DRVSYS, "sd card program failed!\n");
        ret = false;
    }
    //switch func use bit mode,so exchange MSB with LSB
    for(i = 64; i > 0; i--)
    {
        status[i-1] = status_tmp[64-i];
    }
exit:
    //resume the default block len
    g_pCurSdDevice->ulDataBlockLen = SD_DEFAULT_BLOCK_LEN;
    return ret;
}


/**
 * @brief Init the mem partion 
 *
 * Called when init card
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCOMMON_SD_STATUS
 */
unsigned char init_mem(bool bInitMem)
{
    unsigned long status,resp,ocr;

    if(!bInitMem)
    {
        akprintf(C3, M_DRVSYS, "InitMem already ok,skip init mem\n");
        return COMMON_SD_SKIP_INIT_MEM;//b_process
    }
    status = sd_get_ocr(&ocr);
    if (SD_GET_OCR_FAIL == status)
    {
        akprintf(C3, M_DRVSYS, "no acmd41 resp,init mmc\n");
        //init mmc
        status = mmc_nego_volt(SD_HCS|SD_DEFAULT_VOLTAGE);
        if (SD_NEGO_FAIL== status)
        {
            return COMMON_SD_INIT_FAIL;
        }
        else if (SD_NEGO_TIMEOUT == status)
        {
            akprintf(C3, M_DRVSYS, "nego sd timeout,skip init mem\n");
            return COMMON_SD_INIT_MEM_FAIL;//b_process
        }
        else if(SD_NEGO_SUCCESS== status)
        {
            if(sd_get_cid(g_pCurSdDevice->ulCID))
            {
                return COMMON_SD_INIT_MEM_SUCCESS;
            }
            else
            {
                return COMMON_SD_INIT_FAIL;
            }
        }
    }
    if (SD_GET_OCR_INVALID == status)
    {
        akprintf(C3, M_DRVSYS, "ocr invalid,skip init mem\n");
        return COMMON_SD_INIT_MEM_FAIL;//b_process
    }
 
    if (SD_GET_OCR_VALID == status)
    {
        status = sd_nego_volt(SD_HCS | (ocr & g_pCurSdDevice->ulVolt));
        if (SD_NEGO_FAIL== status)
        {
            return COMMON_SD_INIT_FAIL;
        }
        if (SD_NEGO_TIMEOUT == status)
        {
            akprintf(C3, M_DRVSYS, "nego sd timeout,skip init mem\n");
            return COMMON_SD_INIT_MEM_FAIL;//b_process
        }
        if(SD_NEGO_SUCCESS== status)
        {
            g_pCurSdDevice->bInitMemSuccess = 1;
            if(sd_get_cid(g_pCurSdDevice->ulCID))
            {
                return COMMON_SD_INIT_MEM_SUCCESS;
            }
            else
            {
                return COMMON_SD_INIT_FAIL;
            }
        }
    }

    akprintf(C1, M_DRVSYS, "init sd status error!!! \n");
    return COMMON_SD_INIT_MEM_ERROR;
}

/**
 * @brief Wait read or write complete
 *
 * Called when read or write sd card finish
 * @author Huang Xin
 * @date 2010-07-14
 * @return bool
 */
bool wait_rw_complete(void)
{
    unsigned long sd_stat, tmp = 0;

    #define RETRY_TIMES1   (1000000)
    while(tmp++ < RETRY_TIMES1)
    {
        if (sd_get_card_status( &sd_stat ))
        {
            if ( (sd_stat&(1<<8)) && (((sd_stat&SD_CURRENT_STATE_MASK) >> SD_CURRENT_STATE_OFFSET) == SD_CURRENT_STATE_TRAN))    //prg complete
            {
                break;
            }
        }
        else
        {
            akprintf(C1, M_DRVSYS, "get SD card status fail.\n");
            return false;
        }
    }
    if (tmp >= RETRY_TIMES1)
    {
        akprintf(C1, M_DRVSYS, "get sd card status time out, status=0x%x!\n", sd_stat);
        return false;
    }

    return true;
    
}
/**
 * @brief get sd mmc card  capacity
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @return void
 */
static void sd_get_capacity()
{
    unsigned long mult=0,c_size=0,blocknr=0;
    
    //capacity part 
    if (g_pCurSdDevice->enmCardType == CARD_SD && g_pCurSdDevice->bHighCapacity)
    {
    	c_size = ((g_pCurSdDevice->ulCSD[2]<<16) &0x3f0000)+(g_pCurSdDevice->ulCSD[1] >> 16);
		g_pCurSdDevice->ulCapacity = (c_size + 1) << 10;

        akprintf(C3, M_DRVSYS, "SD HC card\n");    
    }
    else if(g_pCurSdDevice->enmCardType == CARD_MMC && g_pCurSdDevice->bHighCapacity)
    {
        g_pCurSdDevice->ulCapacity = MMC4_SECTOR_CNT(g_pCurSdDevice->ulExtCSD);
        akprintf(C3, M_DRVSYS, "MMC HC card\n");
    }
    else
    {
        mult = 1 << (UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 47, 3)+2);
        c_size = UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 62, 2) + (UNSTUFF_BITS((unsigned short*)g_pCurSdDevice->ulCSD, 64, 10) << 2 );
        blocknr = ( c_size + 1 ) * mult;
        //capacity is the num of 512bytes
        g_pCurSdDevice->ulCapacity = blocknr * (g_pCurSdDevice->ulMaxReadBlockLen >> 9);
    }        
}


/**
 * @brief Negotiation of the mmc card  voltage
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param volt[in] The voltage to try.
 * @return T_eSD_STATUS
 */
static unsigned char mmc_nego_volt(unsigned long volt)
{
    unsigned long response = 0;
    unsigned long i=0;
    
    #define RETRY_TIMES2   (10000)    
    do
    {
    	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
    	{
			if (sdio_send_cmd(SD_CMD(1), SD_SHORT_RESPONSE, volt)) 
	        {      
	            response = sdio_get_short_resp();    
	        }
	        else            
	        {
	            return SD_NEGO_FAIL;
	        }

		}
		else
		{
			if (send_cmd(SD_CMD(1), SD_SHORT_RESPONSE, volt)) 
	        {      
	            response = get_short_resp();    
	        }
	        else            
	        {
	            return SD_NEGO_FAIL;
	        }		
		}
        
    }while((!(response & SD_STATUS_POWERUP))&& (i++ < RETRY_TIMES2));

    if(i >= RETRY_TIMES2)
    {
        akprintf(C1, M_DRVSYS, "mmc nego time out!\n");
        return SD_NEGO_TIMEOUT;
    }
    //mmc4.2, a definition for implementation of media higher than 2GB was introduced
    if(response&SD_CCS)
    {
        g_pCurSdDevice->bHighCapacity = 1;
    }
    else
    {
        g_pCurSdDevice->bHighCapacity = 0;
    }
    akprintf(C3, M_DRVSYS, "mmc nego success, ocr value is 0x%x.\n", response&SD_OCR_MASK);
    return SD_NEGO_SUCCESS;
}

/**
* @brief Read block from mmc or sd card with L2 buffer + DMA mode
* @author Huang Xin
* @date 2010-07-14
* @param block_src[in] the address of block to be selected to read 
* @param databuf[in] the pointer of array which will be wrote to card 
* @param size[in] the size of data which will read from card
* @param mode[in] single block or multi block
* @return bool
* @retval  true: read  successfully
* @retval  false: read failed
*/
static bool sd_rw_block(unsigned long blk_num, unsigned char *databuf, unsigned long size, unsigned long mode,unsigned char dir)
{       
    bool ret = false;
    unsigned long cmd;
    unsigned long ram_addr = (unsigned long)databuf;

	if (!sd_get_controller_state(g_pCurSdDevice->enmInterface))
	{
		akprintf(C1, M_DRVSYS, "Err: sd controller is closed\n");
		return ret;
	}
	
    //Data address is in byte units in a standard capacity sd card and in block(512byte) units in a high capacity sd card
    if (!g_pCurSdDevice->bHighCapacity)        //not HC card
    {
        blk_num<<= 9;
    }

    if(SD_DATA_MODE_SINGLE == mode)
    {
        if (SD_DATA_CTL_TO_HOST == dir)
            cmd = SD_CMD(17);
        else
            cmd = SD_CMD(24);
    }
    else
    {
        if (SD_DATA_CTL_TO_HOST == dir)
            cmd = SD_CMD(18);
        else
            cmd = SD_CMD(25);

    }
    //step1: check bus busy
    if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
		ret = sdio_trans_busy();
	else
		ret = sd_trans_busy();
	
    if( ret)
    {
		akprintf(C1, M_DRVSYS, "SD trans busy\n");
		return false;
    }
    
    //step2: send card command
    if(cmd == SD_CMD(25) && g_pCurSdDevice->enmCardType == CARD_SD)
    {
        if (send_acmd(23, SD_SHORT_RESPONSE, size/g_pCurSdDevice->ulDataBlockLen ) == false )
        {
            akprintf(C1, M_DRVSYS, "block rw command %d is failed!\n", cmd);
            return false;
        }
    }
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
		ret = sdio_send_cmd(cmd, SD_SHORT_RESPONSE, blk_num );
	else
		ret = send_cmd(cmd, SD_SHORT_RESPONSE, blk_num );
	
    if (ret == false )
    {
        akprintf(C1, M_DRVSYS, "block rw command %d is failed!\n", cmd);
        return false;
    }
    //step3: transfer data
    if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	    ret = sdio_trans_data_dma(ram_addr,size,dir);
	else		
	    ret = sd_trans_data_dma(ram_addr,size,dir);
    
    //step4: send cmd12 if multi-block operation
    if (mode != SD_DATA_MODE_SINGLE)
    {
		if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
			ret = sdio_send_cmd( SD_CMD(12), SD_SHORT_RESPONSE, 0 );
		else
			ret = send_cmd( SD_CMD(12), SD_SHORT_RESPONSE, 0 );
		
		if (!ret)
        {
            akprintf(C1, M_DRVSYS, "The stop read multi block command failed!\n");
            ret = false;
        }
    }
    
    //step5: wait card status to idle
    if (!wait_rw_complete())
    {
        akprintf(C1, M_DRVSYS, "sd card program failed!\n");
        ret = false;
    }
    return ret;
}

/**
 * @brief Switch or expand memory card functions.
 *
 * This function is supported after sd version 1.10
 * @author Huang Xin
 * @date 2010-07-14
 * @param mode[in] mode
 * @param group[in] group
 * @param value[in] value
 * @param resp[in] response
 * @return bool
 * @retval  true: CMD sent successfully
 * @retval  false: CMD sent failed
 */
bool sd_mode_switch(unsigned long mode, unsigned long group, unsigned char value, unsigned long *resp)
{
	bool ret =  false;
#if 1
	akprintf(C3, M_DRVSYS, "==sd_mode_switch failed!===\n");
#else
    unsigned long arg;
    unsigned long status;
    unsigned long reg_value;
    bool ret = false;
    
    mode = !!mode;
    value &= 0xF;    
    arg = mode << 31 | 0x00FFFFFF;
    arg &= ~(0xF << (group * 4));
    arg |= value << (group * 4);
    //The block length is predefined to 512bits and the use of SET_BLK_LEN command is not necessary
    g_pCurSdDevice->ulDataBlockLen = 64;
    //step1: check bus busy
    if( sd_trans_busy())
    {
        goto exit;
    }
    //step2: send card command
    if(send_cmd(SD_CMD(6), SD_SHORT_RESPONSE, arg) == false)
    {
        akprintf(C3, M_DRVSYS, "mode switch command failed!\n");
        goto exit;
    }
    //step3: transfer data
    ret = sd_trans_data_dma((unsigned long)resp,64,SD_DATA_CTL_TO_HOST);
    //step4: wait card status to idle
    if (!wait_rw_complete())
    {
        akprintf(C3, M_DRVSYS, "sd card program failed!\n");
        ret = false;
    }

exit:
    //resume the default block len
    g_pCurSdDevice->ulDataBlockLen = SD_DEFAULT_BLOCK_LEN;

#endif	
    return ret;
}

/**
 * @brief Get the sd card  cid register
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cid[out] The buffer to save card cid.
 * @return bool
 * @retval  true: get successfully
 * @retval  false: get failed
 */
static bool sd_get_cid(unsigned long *cid)
{

	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	{
	    if (sdio_send_cmd(SD_CMD(2), SD_LONG_RESPONSE, SD_NO_ARGUMENT ))
	    {
	        sdio_get_long_resp(cid);
	        return true;
	    }
	}
	else
	{
	    if (send_cmd(SD_CMD(2), SD_LONG_RESPONSE, SD_NO_ARGUMENT ))
	    {
	        get_long_resp(cid);
	        return true;
	    }
	}

	
    return false;
}

/**
 * @brief Get the sd card  csd register
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param csd[out] The buffer to save card csd.
 * @return bool
 * @retval  true: get successfully
 * @retval  false: get failed
 */
static bool sd_get_csd(unsigned long *csd)
{
    unsigned long addr = g_pCurSdDevice->ulRCA;

	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	{
	    if (sdio_send_cmd(SD_CMD(9), SD_LONG_RESPONSE, (addr << 16)))
	    {
	        sdio_get_long_resp(csd);
	        return true;
	    }
	}
	else
	{
	    if (send_cmd(SD_CMD(9), SD_LONG_RESPONSE, (addr << 16)))
	    {
	        get_long_resp(csd);
	        return true;
	    }
	}

    return false;
}
/**
 * @brief Get the mmc4.0 card  ext csd register
 *
 * Called when init mmc4.0 card
 * @author Huang Xin
 * @date 2010-07-14
 * @param ext_csd[out] The buffer to save mmc4.0 ext csd.
 * @return bool
 * @retval  true: get successfully
 * @retval  false: get failed
 */
static bool mmc_get_extcsd(unsigned long *ext_csd)
{ 
    bool ret = false;
    #if 0
    //step1: check bus busy
    if( sd_trans_busy())
    {
        goto exit;
    }
    //step2: send card command
    if(send_cmd(SD_CMD(8), SD_SHORT_RESPONSE, SD_NO_ARGUMENT) == false)
    {
        akprintf(C3, M_DRVSYS, "get extcsd command failed!\n");
        goto exit;
    }
     //step3: transfer data
    ret = sd_trans_data_dma((unsigned long)ext_csd,512,SD_DATA_CTL_TO_HOST);
    //step4: wait card status to idle
    if (!wait_rw_complete())
    {
         akprintf(C3, M_DRVSYS, "sd card program failed!\n");
         ret = false;
    }
exit:
	#endif
    return ret;
}


/**
 * @brief   sd card switch to high speed mode
 *
 * If card spec vers is mmc4.0 or sd2.0 later,this func should be called to enter high speed mode 
 * @author Huang Xin
 * @date 2010-07-14
 * @param hs[in] The high speed mode flag.
 * @return bool
 * @retval  true: set successfully
 * @retval  false: set failed
 */
static bool sd_switch_hs(bool hs)
{
    bool ret = true;    
    unsigned long arg_value = 0;
    unsigned char  status[64]={0};
    unsigned long i;

    if (g_pCurSdDevice->bHighSpeed == hs)
        return true;
        
    switch (g_pCurSdDevice->enmCardType)
    {
        case CARD_MMC:
            //mmc 4.x
            if (0x4 <= MMC_SPEC_VERSION(g_pCurSdDevice->ulCSD))
            {
                //mmc4.x switch to hs
                arg_value = (0x3<<24)|(185<<16)|(hs<<8);
				
				if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
				{
					ret = sdio_send_cmd(SD_CMD(6), SD_SHORT_RESPONSE, arg_value);
				}
				else
				{
					ret = send_cmd(SD_CMD(6), SD_SHORT_RESPONSE, arg_value);
				}
			
                if (ret)
                {
                    if (!wait_rw_complete())
                    {
                        ret = false;
                    }                    
                }   
                else
                {
                    //send command failed
                    ret = false;
                }
            }
            else
            {
                akprintf(C3, M_DRVSYS, "not mmc 4.x,unsupport switch high speed\n");    
                ret = false;
            }
            break;   
        //sd card high speed mode
        case CARD_SD:
        case CARD_COMBO:            
            if (sd_switch_func(1, 1, hs, status))
            {
                //akprintf(C3, M_DRVSYS, "g1[0x%x]",SD_FUNC_SWITCHED_GROUP1(status));
                if (hs != SD_FUNC_SWITCHED_GROUP1(status))
                {
                    ret = false;
                }
            }
            else
            {
                ret = false;
            }
            break;
        default:
            ret = false;
            break;
    }

    if (ret == true)
        g_pCurSdDevice->bHighSpeed = hs;
        
    return ret;
}

/**
 * @brief send sd acmd.
 *
 * ALL the ACMDS shall be preceded with APP_CMD command cmd55
 * @author Huang Xin
 * @date 2010-07-14
 * @param cmd_index[in] The command index.
 * @param rsp[in] The command response:no response ,short reponse or long response
 * @param arg[in] The cmd argument.
 * @return bool
 * @retval  true: ACMD sent successfully
 * @retval  false: ACMD sent failed
 */
static bool send_acmd(unsigned char cmd_index, unsigned char resp, unsigned long arg)
{
    unsigned long rca;
    rca = g_pCurSdDevice->ulRCA;
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	{
	    sdio_send_cmd(SD_CMD(55), SD_SHORT_RESPONSE, rca << 16);
	    return (sdio_send_cmd(cmd_index, resp, arg));
	}
	else
	{
		send_cmd(SD_CMD(55), SD_SHORT_RESPONSE, rca << 16);
		return (send_cmd(cmd_index, resp, arg));

	}
	
}


/**
 * @brief Set sd card block length.
 *
 * Usually set the block length  512 bytes .
 * @author Huang Xin
 * @date 2010-07-14
 * @param block_len[in] The block length.
 * @return bool
 * @retval  true: set successfully
 * @retval  false: set failed
 */
static bool sd_set_block_len(unsigned long block_len)
{
	bool ret;
	unsigned long len =  block_len;
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	{
		ret = sdio_send_cmd(SD_CMD(16), SD_SHORT_RESPONSE, len );
	}
	else
	{
		ret = send_cmd(SD_CMD(16), SD_SHORT_RESPONSE, len );
	}
	
	if(ret)
    {
        g_pCurSdDevice->ulDataBlockLen = block_len;             
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Set sd card bus width.
 *
 * Usually set the bus width  1 bit or 4 bit  .
 * @author Huang Xin
 * @date 2010-07-14
 * @param wide_bus[in] The bus width.
 * @return bool
 * @retval  true: set successfully
 * @retval  false: set failed
 */
bool sd_set_bus_width(unsigned char wide_bus)
{
    unsigned long arg_value = 0;
    bool ret = true;

    switch (g_pCurSdDevice->enmCardType)
    {
        case CARD_MMC:
            //mmc 4.x
            if (0x4 <= g_pCurSdDevice->ucSpecVersion)
            {
                akprintf(C3, M_DRVSYS, "mmc 4.x  set bus width %d\n",wide_bus);
                //mmc4.x set power class,ignored
                //mmc4.x set bus width  
                arg_value = (0x3<<24)|(183<<16)|(wide_bus<<8);
				if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
				{
					ret = sdio_send_cmd(SD_CMD(6), SD_SHORT_RESPONSE, arg_value );
				}
				else
				{
					ret = send_cmd(SD_CMD(6), SD_SHORT_RESPONSE, arg_value );
				}
                if (ret)
                {
                    if (!wait_rw_complete())
                    {
                        ret = false;
                    }
                    g_pCurSdDevice->enmBusMode = wide_bus;
                    
                }   
                else
                {
                    ret = false;
                }
            }
            else
            {
                g_pCurSdDevice->enmBusMode = USE_ONE_BUS;
            }
            break;            
        case CARD_SD:
        case CARD_COMBO:
            if (wide_bus == USE_ONE_BUS)
                arg_value = SD_BUS_WIDTH_1BIT;
            else if(wide_bus == USE_FOUR_BUS )
                arg_value = SD_BUS_WIDTH_4BIT;
            //sd is not allowed to use 8bit bus
            else
                return false;
            if (send_acmd(SD_CMD(6), SD_SHORT_RESPONSE, arg_value ))
            {
                g_pCurSdDevice->enmBusMode = wide_bus;
            }   
            else
            {
                ret = false;
            }
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}



/**
 * @brief Get the sd card status register
 *
 * Called when wait read or write complete
 * @author Huang Xin
 * @date 2010-07-14
 * @param status_buf[out] The buffer to save card status .
 * @return bool
 * @retval  true: get successfully
 * @retval  false: get failed
 */
static bool sd_get_card_status( unsigned long *status_buf )
{
    unsigned long addr = g_pCurSdDevice->ulRCA;
	if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
	{
		if (sdio_send_cmd( SD_CMD(13), SD_SHORT_RESPONSE, (addr << 16)))
	    {
	        *status_buf = sdio_get_short_resp();
	        return  true;
	    }
	}
	else
	{
		if (send_cmd( SD_CMD(13), SD_SHORT_RESPONSE, (addr << 16)))
	    {
	        *status_buf = get_short_resp();
	        return  true;
	    }
	}

        return false;
}

/**
 * @brief Get the SD status information
 *
 * @author Xuchang
 * @date 2010-07-14
 * @param status[out] The buffer to save sd status .
 * @return bool
 * @retval  true: get successfully
 * @retval  false: get failed
 */
static bool sd_get_sd_status(unsigned char *status)
{
	bool ret = false;
#if 0
    unsigned long  i,arg=0;
    bool ret = false;
    unsigned char   status_tmp[64]= {0};

    if (g_pCurSdDevice->enmCardType != CARD_SD)
        return false;
        
    g_pCurSdDevice->ulDataBlockLen = 64;
    //step1: check bus busy
    if( sd_trans_busy())
    {
        goto exit;
    }
    //step2: send card command
    if(send_acmd(SD_CMD(13), SD_SHORT_RESPONSE, arg) == false)
    {
        akprintf(C1, M_DRVSYS, "mode switch command failed!\n");
        goto exit;
    }
    //step3: transfer data
    ret = sd_trans_data_dma((unsigned long)status_tmp, 64, SD_DATA_CTL_TO_HOST);
    //step4: wait card status to idle
    if (!wait_rw_complete())
    {
        akprintf(C1, M_DRVSYS, "sd card program failed!\n");
        ret = false;
    }
    //get sd status use bit mode,so exchange MSB with LSB
    for(i = 64; i > 0; i--)
    {
        status[i-1] = status_tmp[64-i];
    }

    //printf("speed class: class %d\n", status[55]*2);    //bit[440:447]
    //printf("AU size: 0x%d\n", status[53]>>4);    //bit[428:431]    
    
exit:
#endif
    //resume the default block len
    g_pCurSdDevice->ulDataBlockLen = SD_DEFAULT_BLOCK_LEN;
    return ret;
}

/**
 * @brief Get the sd card  ocr register
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param ocr[out] The buffer to save card ocr.
 * @return unsigned char
 * @retval  T_eSD_STATUS
 */
static unsigned char sd_get_ocr(unsigned long *ocr )
{
    unsigned long  response = 0;
    unsigned char   fun_num = 0;
    bool mem_present = 0;

    if (send_acmd(SD_CMD(41), SD_SHORT_RESPONSE, SD_NO_ARGUMENT))       
    {   
		if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
		response = sdio_get_short_resp();
		else
		response = get_short_resp();
        if(0 == (response & g_pCurSdDevice->ulVolt))
            return SD_GET_OCR_INVALID;//b_process  
        *ocr = response & SD_OCR_MASK;
        return SD_GET_OCR_VALID;
    }
    else        
        return SD_GET_OCR_FAIL;
}

/**
 * @brief Get the sd card  scr register
 *
 * @author Huang Xin
 * @date 2010-07-14
 * @param scr[out] The buffer to save card scr.
 * @return bool
 * @retval  true: get successfully
 * @retval  false: get failed
 */
static bool sd_get_scr(unsigned char *scr )
{
    unsigned char   i = 0;
    unsigned char   scr_tmp[8] = {0};
    bool ret = true;

    g_pCurSdDevice->ulDataBlockLen = 8;
    //step1: check bus busy
    if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
    {
		ret = sdio_trans_busy();
	}
	else
	{
		ret = sd_trans_busy();
	}
    if( ret)
    {
        ret = false;
        goto exit;
    }
    //step2: send card command
    if(send_acmd(SD_CMD(51), SD_SHORT_RESPONSE, SD_NO_ARGUMENT) == false)
    {
        akprintf(C1, M_DRVSYS, "get scr cmd failed!\n");    
        ret = false;
        goto exit;
    }
    //step3: transfer data
    if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
    ret = sdio_trans_data_cpu((unsigned long)scr_tmp, 8, SD_DATA_CTL_TO_HOST);
	else
    ret = sd_trans_data_cpu((unsigned long)scr_tmp, 8, SD_DATA_CTL_TO_HOST);
    //step4: wait card status to idle
    if (!wait_rw_complete())
    {
        akprintf(C1, M_DRVSYS, "sd card program failed!\n");
        ret = false;
    }
    //get scr use bit mode,so exchange MSB with LSB
    for(i = 8; i > 0; i--)
    {
        scr[i-1] = scr_tmp[8-i];
    }
exit:
    //resume the default block len
    g_pCurSdDevice->ulDataBlockLen = SD_DEFAULT_BLOCK_LEN;
    return ret;
}

/**
 * @brief Get the sd mmc  card  spec vers
 *
 * @author Huang Xin
 * @date 2010-07-14
 * @param scr[out] The buffer to save card scr.
 * @return bool
 * @retval  true: get successfully
 * @retval  false: get failed
 */
static unsigned char sd_get_spec_vers()
{
    unsigned char scr[8] = {0};
    unsigned char spec_vers = SD_MMC_INVALID_SPEC_VERSION;
    
    switch (g_pCurSdDevice->enmCardType)
    {
        case CARD_MMC:
            spec_vers = MMC_SPEC_VERSION(g_pCurSdDevice->ulCSD);
            akprintf(C3, M_DRVSYS, "mmc spec version: %d\n", spec_vers);
            break;
        case CARD_SD:
        case CARD_COMBO:
            if(sd_get_scr(scr))
            {
                spec_vers = scr[7]&0x0f;
            }            
            akprintf(C3, M_DRVSYS, "sd spec version: %d\n", spec_vers);
            break;
        default :
            break;
    }
    return spec_vers;
}

/**
 * @brief Negotiation of the sd card  voltage
 *
 * Called when init sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param volt[in] The voltage to try.
 * @return T_eSD_STATUS
 */
static unsigned char sd_nego_volt(unsigned long volt)
{
    unsigned long response = 0;
    unsigned long i=0;

    #define RETRY_TIMES3     (10000)
    do
    {
        if (send_acmd(SD_CMD(41), SD_SHORT_RESPONSE, volt))     
        {      
			if(INTERFACE_SDIO == g_pCurSdDevice->enmInterface)
			response = sdio_get_short_resp();        
			else
			response = get_short_resp();        
        }
        else                    
        {
            return SD_NEGO_FAIL;
        }
    }while((!(response & SD_STATUS_POWERUP))&& (i++ < RETRY_TIMES3));
    if(i >= RETRY_TIMES3)
    {
        akprintf(C1, M_DRVSYS, "sd nego time out!\n");
        return SD_NEGO_TIMEOUT;
    }
    akprintf(C3, M_DRVSYS, "sd nego success, ocr value is 0x%x.\n",response&SD_OCR_MASK);
    if(response&SD_CCS)
    {
        g_pCurSdDevice->bHighCapacity = 1;
    }
    else
    {
        g_pCurSdDevice->bHighCapacity = 0;
    }
    return SD_NEGO_SUCCESS;
}

/**
 * @brief Get the value according some bits
 *
 * Called when get information about cid and csd.
 * @author Huang Xin
 * @date 2010-07-14
 * @param resp[in] The  csd or cid buffer 
 * @param start[in] The start bit
 * @param resp[in] The number of bits
 * @return The value according the  bits
 */
static signed long stuff_bits (unsigned short * resp, signed long start, signed long size)
{
    unsigned short __size = size;
    unsigned short __mask = (__size < 16 ? 1 << __size : 0) - 1;
    signed long __off = ((start) / 16);
    signed long __shft = (start) & 15;
    unsigned short __res = 0;

    __res = resp [__off] >> __shft;
    if (__size + __shft > 16)
    {
            __res = resp [__off] >> __shft;
            __res |= resp[__off+1] << ((16 - __shft) % 16);
    }

    return (__res & __mask);
}


