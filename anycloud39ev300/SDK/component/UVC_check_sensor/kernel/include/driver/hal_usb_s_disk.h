/**@file hal_usb_s_disk.h
 * @brief provide operations of how to use usb disk.
 *
 * This file describe frameworks of usb disk driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-06-30
 * @version 1.0
 */

#ifndef __HAL_USB_S_DISK_H__
#define __HAL_USB_S_DISK_H__

#include "anyka_types.h"
#include "hal_usb_s_state.h"

#ifdef __cplusplus
extern "C" {
#endif
/** @defgroup USB_disk USB_disk
 *@ingroup USB
 */
/*@{*/

typedef bool  (*F_AccessLun)(unsigned char *buf, unsigned long BlkStart, unsigned long BlkCnt, unsigned long LunAddInfo);
typedef bool (*T_fMBOOT_HANDLE_CMD)(unsigned char* scsi_data, unsigned long data_len);
typedef bool (*T_fMBOOT_HANDLE_SEND)(unsigned long buf, unsigned long count);
typedef bool (*T_fMBOOT_HANDLE_RCV)(unsigned long buf, unsigned long count);


#define INQUIRY_STR_VENDOR_SIZE     8
#define INQUIRY_STR_PRODUCT_SIZE    16
#define INQUIRY_STR_REVISION_SIZE   4

typedef enum
{
    NORMAL_STATUS = 0,
    READ_ONLY 
}T_uATTRIBUTE;

typedef enum
{
    STR_MANUFACTURER = 0,
    STR_PRODUCT,
    STR_SERIAL_NUM
}T_eSTR_DESC;


enum SENSESTATUS
{
    MEDIUM_NOSENSE = 0,
    MEDIUM_NOTPRESENT,
    MEDIUM_NOTREADY,
    MEDIUM_NOTREADY_TO_READY
};
typedef enum SENSESTATUS E_SENSESTATUS;

typedef struct
{
    F_AccessLun Read;///<logic unit nunber read function
    F_AccessLun Write;///<lun write function
    unsigned long BlkCnt;///<blk cnt of this lun
    unsigned long BlkSize;///<blk size of this lun
    unsigned long LunIdx;///<index of this lun
    unsigned long LunAddInfo;///<the lun information which we want to tranfer in F_AccessLun para 4
    unsigned char FastBlkCnt;///<how many block is fast for this medium
    E_SENSESTATUS sense;///<sense of this lun
    unsigned char uAttribute;
    unsigned char reserved;
    unsigned char Vendor[INQUIRY_STR_VENDOR_SIZE];
    unsigned char Product[INQUIRY_STR_PRODUCT_SIZE];
    unsigned char Revision[INQUIRY_STR_REVISION_SIZE];
}T_LUN_INFO,*T_pLUN_INFO;

/**
 * @brief bus event handler
 
 *   define bus event handler, different class driver have different handler
 */


 #define USB_EXT_SCSI_PASSWORD_NO     12
 
 #define STAGE_READY              0
 #define STAGE_COMMAND            1
 #define STAGE_DATA_IN            2  
 #define STAGE_DATA_OUT           3  
 #define STAGE_STATUS             4
 
typedef struct tagUSB_EXT_CMD_HANDLER
{
    unsigned char ext_scsi;                                                  ///< class code 
    unsigned char stage; 
    unsigned short reserve;
    bool (*ext_callback)(unsigned char *data, unsigned char len);                   ///< callback function for ext scsi    
}T_USB_EXT_CMD_HANDLER, *T_pUSB_EXT_CMD_HANDLER;


typedef struct
{
    unsigned char ext_scsi;
    T_USB_EXT_CMD_HANDLER *handler;
}T_USB_EXT_INFO;

/**
 * @brief USB probe pointer
 * @author LHD
 * @date 2011-08-19
 * @param[in] ext_cmd USB ext cmd 
 * @return T_USB_EXT_CMD_HANDLER: USB ext cmd pointer 
 */
T_USB_EXT_CMD_HANDLER *usb_ext_probe(unsigned char ext_cmd);

/**
 * @brief register usb extcmd
 * @author LHD
 * @date 2011-08-19
 * @param[in] ext_cmd USB ext cmd
 * @param[in] handler extcmd pointer
 * @return bool
 * Note:param[in] T_USB_EXT_CMD_HANDLER *handler is global variable pointer
 */
bool usb_ext_scsi_reg(unsigned char ext_cmd, T_USB_EXT_CMD_HANDLER *handler);

/**
 * @brief   mass boot init
 *
 * udisk reset,configok,this func must be called because mass boot at usb1.1 will not run enum 
 * @author Huang Xin
 * @date 2010-08-04
 * @param[in] mode usb mode 1.1 or 2.0
 * @return bool
 * @retval false init failed
 * @retval AK_TURE init successful
 */
void usbdisk_mboot_init(unsigned long mode);
/** 
 * @brief set procduce callback 
 *
 * used by produce
 * @author Huang Xin
 * @date 2010-08-04
 * @param[in] hnd_cmd handle cmd callback
 * @param[in] hnd_send handle send callback
 * @param[in] hnd_rcv handle receive callback
 * @return  bool
 * @retval false set failed
 * @retval AK_TURE set successful
 */
bool usbdisk_mboot_set_cb(T_fMBOOT_HANDLE_CMD hnd_cmd, T_fMBOOT_HANDLE_SEND hnd_send, T_fMBOOT_HANDLE_RCV hnd_rcv);

/** 
 * @brief init  str desc with  reference to device desc
 *
 * the str is truncated to 10  characters or less,this func may be called before usbdisk_start
 * @author Huang Xin
 * @date 2010-08-04
 * @param[in] index type of string descriptor
 * @param[in] str the start address of stirng
 * @return  bool
 * @retval false set failed
 * @retval AK_TURE set successful
 */
bool usbdisk_set_str_desc(T_eSTR_DESC index,char *str);

/**
 * @brief   init usb disk function
 *
 * Initialize usb disk buffer,creat HISR,creat usb disk task,creat message queue,creat event group
 * @author Huang Xin
 * @date 2010-06-30
 * @param[in] mode usb mode 1.1 or 2.0
 * @return bool
 * @retval false init failed
 * @retval AK_TURE init successful
 */

bool usbdisk_init(unsigned long mode);


/**
 * @brief   start usb disk function, this function must be call after usbdisk_init
 *
 * Allocate L2 buffer , open usb controller,set usb disk callback,register interrupt process function
 * @author Huang Xin
 * @date  2010-06-30
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool usbdisk_start(void);

/**
 * @brief  enter udisk task,poll udisk msg
 *
 * This function is added for  bios version,and must be call after usbdisk_start
 * @author Huang Xin
 * @date  2010-08-04
 * @return void
 */
void usbdisk_proc(void);

/**
 * @brief   disable usb disk function.
 *
 * Close usb controller,terminate usb disk task,free buffer,free data struct
 * @author Huang Xin
 * @date 2010-06-30
 * @return  void
 */    
void usbdisk_stop(void);

/**
 * @brief   usb slave bulk disk add a lun
 *
 * This function is called when host is  mounting usb disk
 * @author Huang Xin
 * @date 2010-06-30
 * @param[in] pAddLun struct of lun information.
 * @return bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool usbdisk_addLUN(T_LUN_INFO *pAddLun);

/**
 * @brief   usb slave bulk disk change lun
 *
 * When sd card is detected, change the lun for sd card
 * @author Huang Xin
 * @date 2010-06-30
 * @param[in] pChgLun struct of lun information.
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
bool usbdisk_changeLUN(T_LUN_INFO *pChgLun);

/**
 * @brief   get_testunit_state
 *
 * called when usb ok
 * @author LHD
 * @date 2012-03-01
 * @return  bool
 */
bool udisk_get_testunit_state(void);

/*@}*/
#ifdef __cplusplus
}
#endif

#endif
