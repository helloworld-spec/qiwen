/**@file hal_sk_mmc.h
 * @brief provide hal level operations of how to control sd.
 *
 * This file describe sd hal driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-14
 * @version 1.0
 */

#ifndef __HAL_SK_MMC_H
#define __HAL_SK_MMC_H

#include "arch_mmc_sd.h"
#include "hal_i_sk_mmc.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _SKMMC
{
    bool bOpenOK;    
    unsigned long ulCID[4];
    T_SKMMCBIN pEraseBin;
    T_SKMMCBIN pLlf1Bin;
    T_SKMMCBIN pLlf2Bin;
    unsigned char* pLlfParam;
    unsigned char* pFdmBin;
        
        
}T_SKMMC,*T_pSKMMC;


#ifdef __cplusplus
}
#endif


#endif 
  
