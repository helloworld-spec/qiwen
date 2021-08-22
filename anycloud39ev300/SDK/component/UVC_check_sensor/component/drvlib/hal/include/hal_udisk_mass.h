/**@file hal_usb_s_mass.h
 * @brief Implement msc protocol and ufi cmd process.
 *
 * This file describe msc protocol  and ufi cmd process of usb disk.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-24
 * @version 1.0
 */

#ifndef __HAL_USB_S_MASS_H__
#define __HAL_USB_S_MASS_H__

#include "anyka_types.h"
#include "hal_usb_mass.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCSI_TRANS_BUF_LEN      100

//number 0  is not allowed to define udisk msg
#define UDISK_SCSI_MSG          1
#define UDISK_USB_MSG           2
#define UDISK_MBOOT_FLAG                        0xff

typedef enum _MSC_BO_STAGE
{
    MSC_STAGE_READY = 0,
    MSC_STAGE_COMMAND,
    MSC_STAGE_DATA_IN,
    MSC_STAGE_DATA_OUT,
    MSC_STAGE_STATUS
}T_eMSC_BO_STAGE;



typedef 
#ifdef __CC_ARM
__packed
#endif
struct _MSC_TRANS
{
    T_eMSC_BO_STAGE enmStage;                ///<stage of MSC(mass storage class) bulk-only                           ///<logic unit number of MSC
    T_USB_CBW_HEAD  tCbw;                    ///<cbw
    T_USB_CSW_PKT   tCsw;                    ///<csw
} 
#ifdef __GNUC__
__attribute__((packed))
#endif
T_MSC_TRANS,*T_pMSC_TRANS;

  
/**
* @brief parse cbw
*
* called when cbw is received
* @author Huang Xin
* @date 2010-08-04
* @param buf[in] buf which is saving cbw
* @param buflen[in] the cbw len
* @return  bool
* @retval  false means failed
* @retval  AK_TURE means successful
*/
bool msc_parse_cbw(unsigned char *buf, unsigned long buflen);
/**
 * @brief send csw
 *
 * called in status stage  
 * @author Huang Xin
 * @date 2010-08-04
 * @param status[in] pass,failed or phase error
 * @param residue[in] difference between the amount of data expected and the actual sent by the device.
 * @return  void
 */
void msc_send_csw(unsigned char status,unsigned long residue);
/**
 * @brief get current stage
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @return  unsigned char
 * @retval the enum value of stage
 */
unsigned char msc_get_stage();
/**
 * @brief set stage
 *
 * @author Huang Xin
 * @date 2010-08-04
 * @param stage[in] the current stage to set
 * @return  void
 */

void msc_set_stage(unsigned char stage);
/**
 * @brief reset the count of the TEST_UNIT_READY command received
 * @author ZhengHaosheng
 * @date 2012-05-03
 * @return  void
 */
void msc_reset_inquire_cnt(void);
/**
 * @brief get the count of the TEST_UNIT_READY command received
 * @author ZhengHaosheng
 * @date 2010-05-03
 * @return  void
 */
unsigned short msc_get_inquire_cnt(void);
#ifdef __cplusplus
}
#endif

#endif 


