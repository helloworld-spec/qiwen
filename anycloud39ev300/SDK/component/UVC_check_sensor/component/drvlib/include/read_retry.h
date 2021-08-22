/**
 * @filename read_retry.h
 * @brief  provide the interface to use Read-Retry 
 * Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
 * @author yangyiming
 * @date 2012-05-15
 * @version 1.0
 */
 
#include "anyka_types.h"
#include "drv_api.h"

#ifndef __READ_RETRY_H_
#define __READ_RETRY_H_

#define RTOS
#define SUPPORT_HYNIX_26NM
#define SUPPORT_TOSHIBA_24NM
#define MAX_RR_CHIPCNT  4
#define RR_OUT_PUT   printf

typedef struct
{
    unsigned long   retry_times;
    void (*retry_init)(unsigned long chip); 
    void (*modify_scales)(unsigned long chip);
    void (*revert_scales)(unsigned long chip);
}T_NAND_RETRY_FUNCTION_SET; 

typedef struct
{
    unsigned long IDL;
    unsigned long IDH;
    T_NAND_RETRY_FUNCTION_SET *pFun;
}T_RETRY_NAND_INFO;

/**
 * @brief initial the read-retry module if the nand whoes id is given supports read-retry
 *
 * @author yangyiming
 * @date 2012-05-11
 * @param[in] IDL the lower 4 bytes id
 * @param[in] IDH the higher 2 bytes id
 * @param[in] CHIPCNT the chip amount
 * @return T_NAND_RETRY_FUNCTION_SET *  the pointer to a set of function to use read-retry.
 * @             returns NULL if read-retry is not supported 
 */
T_NAND_RETRY_FUNCTION_SET * RR_init(unsigned long IDL, unsigned long IDH, unsigned long CHIPCNT);


#endif
