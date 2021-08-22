/**
 * @file arch_mmc_sd.h
 * @brief list SD card operation interfaces.
 *
 * This file define and provides functions of SD card
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author HuangXin
 * @date 2010-06-17
 * @version 2.0 for AK322x
 */
#ifndef __ARCH_MMC_SD_H
#define __ARCH_MMC_SD_H


/** @defgroup MMC_SD_SDIO MMC_SD_SDIO
 *  @ingroup Drv_Lib
 */
/*@{*/

typedef void * T_pCARD_HANDLE;

typedef enum
{    
    PARTITION_USER,
    PARTITION_BOOT1,
    PARTITION_BOOT2,
    PARTITION_RPMB,
    PARTITION_GP1,
    PARTITION_GP2,
    PARTITION_GP3,
    PARTITION_GP4,
    PARTITION_NUM
}
T_eCARD_PARTITION;


typedef enum
{
    INTERFACE_NOT_SD,
    INTERFACE_SDMMC4,
    INTERFACE_SDMMC8,
    INTERFACE_SDIO
}
T_eCARD_INTERFACE;


typedef enum _BUS_MODE
{
    USE_ONE_BUS,
    USE_FOUR_BUS,
    USE_EIGHT_BUS
}T_eBUS_MODE;

/**
 * @brief   mmc4.3 later card switch partition
 *
 * If card spec vers is mmc4.3 later,this func should be called to switch partition 
 * @author Huang Xin
 * @date 2010-07-14
 * @param[in] handle card handle,a pointer of void 
 * @param[in] part the selected partition
 * @return bool
 * @retval  true: switch successfully
 * @retval  false: switch failed
 */
bool emmc_switch_partition(T_pCARD_HANDLE handle,T_eCARD_PARTITION part);

/**
 * @brief read data from sd card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] handle card handle,a pointer of void
 * @param[in] block_src source block to read
 * @param[out] databuf data buffer to read
 * @param[in] block_count size of blocks to be readed
 * @return bool
 * @retval  true: read successfully
 * @retval  false: read failed
 */
bool sd_read_block(T_pCARD_HANDLE handle,unsigned long block_src, unsigned char *databuf, unsigned long block_count);

/**
 * @brief write data to sd card 
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] handle card handle,a pointer of void
 * @param[in] block_dest destation block to write
 * @param[in] databuf data buffer to write
 * @param[in] block_count size of blocks to be written
 * @return bool
 * @retval  true:write successfully
 * @retval  false: write failed
 */
bool sd_write_block(T_pCARD_HANDLE handle,unsigned long block_dest, const unsigned char *databuf, unsigned long block_count);


/**
 * @brief set the sd interface clk
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] handle card handle,a pointer of void
 * @param[in] clock clock to set
 * @return bool
 */
bool sd_set_clock(T_pCARD_HANDLE handle,unsigned long clock);


/**
 * @brief Close sd controller
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] handle card handle,a pointer of void
 * @return void
 */
void sd_free(T_pCARD_HANDLE handle);

/**
 * @brief get sd card information
 * @author Huang Xin
 * @date 2010-06-17
 * @param[in] handle card handle,a pointer of void
 * @param[out] total_block current sd's total block number
 * @param[out] block_size current sd's block size
 * @a block = 512 bytes
 * @return void
 */
void sd_get_info(T_pCARD_HANDLE handle, unsigned long *total_block, unsigned long *block_size);


/**
* @brief initial mmc sd or comob card
* @author Huang Xin
* @date 2010-06-17
* @param[in] cif card interface selected
* @param[in] bus_mode bus mode selected, can be USE_ONE_BUS or USE_FOUR_BUS
* @return T_pCARD_HANDLE
* @retval NON-NULL  set initial successful,card type is  mmc sd or comob
* @retval NULL set initial fail,card type is not mmc sd or comob card
*/
T_pCARD_HANDLE sd_initial(T_eCARD_INTERFACE cif, unsigned char bus_mode);


/*@}*/
#endif //__ARCH_MMC_SD_H  
