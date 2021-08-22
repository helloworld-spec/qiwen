/**@file hal_i_sk_mmc.h
 * @brief provide hal level operations of how to control sd.
 *
 * This file describe sd hal driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */

#ifndef __HAL_I_SKMMC_H
#define __HAL_I_SKMMC_H

#include "arch_mmc_sd.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned char (*T_SKMMCBIN)[512];


bool sk_mmc_init(T_SKMMCBIN erase_bin,T_SKMMCBIN llf1_bin,T_SKMMCBIN llf2_bin,unsigned char* llf_param,unsigned char*  fdm_bin);
T_pCARD_HANDLE sk_mmc_open_card(T_eCARD_INTERFACE cif, unsigned char bus_mode);

#ifdef __cplusplus
}
#endif


#endif 
  
