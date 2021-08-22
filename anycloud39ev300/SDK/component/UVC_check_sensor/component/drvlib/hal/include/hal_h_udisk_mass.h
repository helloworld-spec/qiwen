/**@file hal_h_udisk_mass.h
 * @brief Implement msc protocol and ufi cmd process.
 *
 * This file describe msc protocol  and ufi cmd process of usb disk.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-24
 * @version 1.0
 */

#ifndef __HAL_H_UDISK_MASS_H__
#define __HAL_H_UDISK_MASS_H__

#include "anyka_types.h"
#include "hal_usb_mass.h"


#ifdef __cplusplus
extern "C" {
#endif

#define CBW_TAG     0x87654321


typedef enum _MSC_STAGE_STATUS
{
    MSC_STAGE_ALL_SUCCESS,
    MSC_STAGE_CMD_FAILED,
    MSC_STAGE_DATA_FAILED,
    MSC_STAGE_STATUS_FAILED,
    MSC_STAGE_PARAM_ERROR
}T_eMSC_STAGE_STATUS;

/**
 * @brief   inquiry process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param str[out] Buffer to store data
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char msc_inquiry(unsigned char Lun, unsigned char * str, unsigned char* status);
/**
 * @brief   test unit ready process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char msc_test_unit_ready(unsigned char Lun,unsigned char* status);
/**
 * @brief   read format capacity process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param buf[out] Buffer to store data
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char msc_read_format_capacity(unsigned char Lun,unsigned char* buf,unsigned char* status);
/**
 * @brief   read capacity process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param buf[out] Buffer to store data
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char msc_read_capacity(unsigned char Lun,unsigned char* buf,unsigned char* status);

/**
 * @brief   request sense process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param LUN[in] index of logic unit.
 * @param buf[out] Buffer to store data
 * @param status[out] csw status val
 * @return unsigned char
 * @retval cmd/data/status stage status
 */
unsigned char  msc_req_sense(unsigned char Lun,unsigned char* buf,unsigned char* status);

/**
 * @brief   get max lun process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @param pMaxLun[in] max lun.
 * @return bool
 * @retval  true: get successfully
 * @retval  false: get failed
 */
bool msc_get_max_lun(unsigned char* pMaxLun);
/**
 * @brief   bulk only reset process
 *
 * @author Huang Xin
 * @date 2010-07-12
 * @return bool
 * @retval  true: reset successfully
 * @retval  false: reset failed
 */
bool msc_bulk_only_reset(void);



#ifdef __cplusplus
}
#endif

#endif 


