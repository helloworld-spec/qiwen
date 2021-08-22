/**@file hal_common_sd.h
 * @brief provide common operations of sd and sdio.
 *
 * This file describe sd common driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */

#ifndef __HAL_COMMON_SDIO_H
#define __HAL_COMMON_SDIO_H

#include "anyka_types.h"
#include "arch_mmc_sd.h"

#include "hal_common_sd.h"

#ifdef __cplusplus
extern "C" {
#endif

extern volatile  T_pSD_DEVICE g_pSdioDevice;             ///< current working sd device

/**
 * @brief Init sd card.
 *
 * Init card ,get the card type
 * @author Huang Xin
 * @date 2010-07-14
 * @return T_eCARD_TYPE
 */
T_eCARD_TYPE sdio_if_init_card(bool bInitIo,bool bInitMem);


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
bool sdio_if_select_card(unsigned long rca);

/**
 * @brief Release card
 *
 * close sd controller and free card device struct, called when init card fail
 * @author Huang Xin
 * @date 2010-07-14
 * @return void
 */
void sdio_if_release();



#ifdef __cplusplus
}
#endif
#endif
