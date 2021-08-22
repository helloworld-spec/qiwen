/**@file hal_common_sd.h
 * @brief provide common operations of sd and sdio.
 *
 * This file describe sd common driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */

#ifndef __HAL_COMMON_SD_H
#define __HAL_COMMON_SD_H

#include "anyka_types.h"
#include "arch_mmc_sd.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SD_IDENTIFICATION_MODE_CLK  (350*1000)              ///<350k
#define SD_TRANSFER_MODE_CLK        (20*1000*1000)          ///<20M
#define HS_TRANSFER_MODE_CLK1       (30*1000*1000)          ///<30M
#define HS_TRANSFER_MODE_CLK2       (26*1000*1000)          ///<26M
#define MMC_DEFAULT_MODE_20M        (20*1000*1000)
#define SD_DEFAULT_MODE_25M         (25*1000*1000)
#define MMC_HS_MODE_26M             (26*1000*1000)
#define MMC_HS_MODE_52M             (52*1000*1000)
#define SD_HS_MODE_50M              (50*1000*1000)

#define SD_HCS                      (1<<30)
#define SD_STATUS_POWERUP           (1UL<<31)
#define SD_CCS                      (1<<30)
#define SD_OCR_MASK                 (0xffffffff)
#define SD_DEFAULT_VOLTAGE          (0x00FF8000)
#define ERROR_INVALID_RCA            T_U32_MAX

/**
*@brief card type define
*/

typedef enum _CARD_TYPE
{
    CARD_UNUSABLE=0,                            ///< unusable card
    CARD_MMC,                                   ///< mmc card
    CARD_SD,                                    ///< sd card, mem only
    CARD_SDIO,                                  ///< sdio card, io only
    CARD_COMBO                                  ///< combo card,mem and sdio
}T_eCARD_TYPE;

/**
*@brief Common sd device define,contain some attributes of sd sdio and mmc
*/
typedef struct _SD_DEVICE
{
    T_eCARD_INTERFACE   enmInterface;            ///< The interface used by sd/sdio card
    T_eCARD_TYPE        enmCardType;             ///< Card type is got when init finish
    T_eBUS_MODE         enmBusMode;              ///< the bus mode used by sd/sdio card
    T_eCARD_PARTITION   enmPartition;            ///< the selected partition, only for inand
    bool  bInitIoSuccess;                      ///< Init IO success flag
    bool  bInitMemSuccess;                     ///< Init mem success flag
    bool  bHighCapacity;                       ///< High capacity flag
    bool  bHighSpeed;                          ///< High Speed flag
    bool  bMemPresent;                         ///< Mem present flag of combo card
    bool  bCsdVersion20;                       ///< This param is got from csd
    bool  bReadBlockPartial;                   ///< This param is got from csd   
    bool  bWriteBlockPartial;                  ///< This param is got from csd
    bool  bWriteBlockMisalign;                 ///< This param is got from csd
    bool  bReadBlockMisalign;                  ///< This param is got from csd
    bool  bEraseBlockEnable;                   ///< This param is got from csd
    unsigned char    ucSpecVersion;                       ///< mmc or sd card spec version 
    unsigned long   ulClock;                             ///< card clock
    unsigned long   ulVolt;                              ///< The  work voltage of sd/sdio card
    unsigned long   ulCapacity;                          ///< The capacity of sd/sdio card,number of blocks
    unsigned long   ulDataBlockLen;                      ///< The current block length of sd card
    unsigned long   ulRCA;                               ///< Card address
    unsigned long   ulMaxReadBlockLen;                   ///< This param is got from csd
    unsigned long   ulMaxWriteBlockLen;                  ///< This param is got from csd
    unsigned long   ulEraseSectorSize;                   ///< This param is got from csd
    unsigned long   ulFunNum;                            ///< Function number of sdio card 
    unsigned long   ulFunBlockLen[8];                    ///< Function block length of sdio card 
    unsigned long   ulCID[4];                            ///< CID of sd/mmc card
    unsigned long   ulCSD[4];                            ///< CSD of sd/mmc card
    unsigned long   ulExtCSD[128];                       ///< extCSD of mmc4.x card
} T_SD_DEVICE,*T_pSD_DEVICE;

/**
*@brief Define some status used when init card
*/
typedef enum _COMMON_SD_STATUS
{
    COMMON_SD_SKIP_INIT_IO,                     ///< Not need to init io
    COMMON_SD_INIT_IO_FAIL,                     ///< init io fail
    COMMON_SD_INIT_IO_SUCCESS,                  ///< init io successful
    COMMON_SD_INIT_IO_ERROR,                    ///< init io error
    COMMON_SD_SKIP_INIT_MEM,                    ///< not need to init mem
    COMMON_SD_INIT_MEM_FAIL,                    ///< init mem fail
    COMMON_SD_INIT_MEM_SUCCESS,                 ///< init mem successful
    COMMON_SD_INIT_MEM_ERROR,                   ///< init mem error
    COMMON_SD_INIT_FAIL                         ///< init card fail
}T_eCOMMON_SD_STATUS;

extern volatile T_pSD_DEVICE g_pSdDevice;             ///< current working sd device


/**
 * @brief Init sd host controller.
 *
 * Select sd/sdio interface, set share pin,enable sd/sdio module
 * @author Huang Xin
 * @param cif[in] card interface selected
 * @date 2010-07-14
 * @return void
 */
void sd_open_controller(T_eCARD_INTERFACE cif);

/**
 * @brief Close sd host controller.
 *
 * Select non sd/sdio interface, restore share pin, close sd/sdio module
 * @author Huang Xin
 * @param cif[in] card interface selected
 * @date 2010-07-14
 * @return void
 */
void sd_close_controller(T_eCARD_INTERFACE cif);

/**
 * @brief get sd host controller states
 * @author LHS
 * @date 2011-10-26
 * @param cif[in] card interface selected
 * @return bool: return TURE mean controller is opend.
 */
bool sd_get_controller_state(T_eCARD_INTERFACE cif);

/**
 * @brief Init sd card.
 *
 * Init card ,get the card type
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCARD_TYPE
 */
T_eCARD_TYPE init_card(bool bInitIo,bool bInitMem);

#if 0
/**
 * @brief get sd relative address.
 *
 * Send CMD3 to get the sd relative address.
 * @author Huang Xin
 * @date 2010-07-14
 * @param cmd_index[in] The command index.
 * @param rsp[in] The command response:no response, short reponse or long response
 * @param arg[in] The cmd argument.
 * @return unsigned long
 * @retval  The RCA
 * @retval  ERROR_INVALID_RCA
 */
unsigned long get_rca();
#endif

/**
 * @brief Slect or reject a mmc or sd card.
 *
 * Send CMD7 to select a sd card.
 * @author Huang Xin
 * @date 2010-07-14
 * @param rca[in] The selected card relative address 
 * @return bool
 * @retval true Select successful
 * @retval false Select failed
 */
bool select_card(unsigned long rca);

/**
 * @brief Release card
 *
 * close sd controller and free card device struct, called when init card fail
 * @author Huang Xin
 * @date 2010-07-14
 * @return void
 */
void sd_release();



#ifdef __cplusplus
}
#endif
#endif
