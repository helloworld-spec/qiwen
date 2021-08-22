/**
 * @filename hal_usb_mass.h
 * @brief:Mass Storage of usb.
 *
 * This file describe mass storage protocol of usb.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-07-24
 * @version 1.0
 */

#ifndef __HAL_USB_MASS_H__
#define __HAL_USB_MASS_H__

#include "anyka_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @{@name SCSI command define
 *      Define SCSI command
 *      
 */
#define SCSI_FORMAT                  0x04
#define SCSI_INQUIRY                 0x12
#define SCSI_START_STOP              0x1b
#define SCSI_MODE_SELECT             0x55
#define SCSI_MODE_SENSE              0x5a
#define SCSI_MEDUIM_REMOVE           0x1e
#define SCSI_READ_10                 0x28
#define SCSI_READ_12                 0xa8
#define SCSI_READ_CAPACITY           0x25
#define SCSI_READ_FORMAT_CAPACITY    0x23
#define SCSI_REQUEST_SENSE           0x03
#define SCSI_REZERO_UNIT             0x01
#define SCSI_SEEK_10                 0x2b
#define SCSI_SEND_DIAGNOSTIC         0x1d
#define SCSI_TEST_UNIT_READY         0x00
#define SCSI_VERIFY                  0x2f
#define SCSI_WRITE_10                0x2a
#define SCSI_WRITE_12                0xaa
#define SCSI_WRITE_VERIFY            0x2e    
#define SCSI_MODESENSE_6             0x1A
#define SCSI_PREVENT_REMOVE          0x1E
#define SCSI_UNSUPPORTED             0xFF

#define SCSI_ANYKA_MASS_BOOT         0xF1

/** @} */

/** @{@name SCSI sense define
 *      Define SCSI sense for modesense 6
 *      
 */
#define SENSE_VALID                 0x80
#define SENSE_INVALID               0
#define SENSE_ERRORCODE             0x70
/** @} */

/**
 * @brief  sense key define
 */
typedef enum
{
    NO_SENSE = 0x000000,
    RECOVERED_ERROR = 0x010000,
    RECOVERED_DATA_WITH_RETRIES = 0x011701,
    RECOVERED_DATA_WITH_ECC = 0x011800,
    NOT_READY= 0x020000,
    LOGICAL_DRIVE_BECOMING_READY = 0x020401,
    LOGICAL_DRIVE_INITIALIZATION_REQUIRED = 0x020402,
    LOGICAL_UNIT_FORMAT_IN_PROGRESS = 0x020404,
    LOGICAL_DRIVE_DEVICE_IS_BUSY = 0x0204FF,
    NO_REFERENCE_POSITION_FOUND = 0x020600,
    LOGICAL_UNIT_COMMUNICATION_FAILURE = 0x020800,
    LOGICAL_UNIT_COMMUNICATION_TIME_OUT = 0x020801,
    LOGICAL_UNIT_COMMUNICATION_OVERRUN = 0x020880,
    MEDIUM_NOT_PRESENT = 0x023A00,
    USB_TO_HOST_SYSTEM_INTERFACE_FAILURE = 0x025400,
    INSUFFICIENT_RESOURCES = 0x028000,
    UNKNOWN_ERROR = 0x02FFFF,
    MEDIUM_ERROR = 0x030000,
    NO_SEEK_COMPLETE = 0x030200,
    WRITE_FAULT = 0x030300,
    ID_CRC_ERROR = 0x031000,
    UNRECOVERED_READ_ERROR = 0x031100,
    ADDRESS_MARK_NOT_FOUND_FOR_ID_FIELD = 0x031200,
    ADDRESS_MARK_NOT_FOUND_FOR_DATA_FIELD = 0x031300,
    RECORDED_ENTITY_NOT_FOUND = 0x031400,
    CANNOT_READ_MEDIUM_UNKNOWN_FORMAT = 0x033001,
    FORMAT_COMMAND_FAILED = 0x033101,
    HARDWARE_ERROR = 0x044000,
    DIAGNOSTIC_FAILURE_ON_COMPONENT = 0x0440FF,
    ILLEGAL_REQUEST = 0x050000,
    PARAMETER_LIST_LENGTH_ERROR = 0x051A00,
    INVALID_COMMAND_OPERATION_CODE = 0x052000,
    LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE = 0x052100,
    INVALID_FIELD_IN_COMMAND_PACKET = 0x052400,
    LOGICAL_UNIT_NOT_SUPPORTED = 0x052500,
    INVALID_FIELD_IN_PARAMETER_LIST = 0x052600,
    PARAMETER_NOT_SUPPORTED = 0x052601,
    PARAMETER_VALUE_INVALID = 0x052602,
    SAVING_PARAMETERS_NOT_SUPPORT = 0x053900,
    UNIT_ATTENTION = 0x060000,
    NOT_READY_TO_READY_TRANSITION_MEDIA_CHANGED = 0x062800,
    POWER_ON_RESET_OR_BUS_DEVICE_RESET_OCCURRED = 0x062900,
    COMMANDS_CLEARED_BY_ANOTHER_INITIATOR = 0x062F00,
    DATA_PROTECT = 0x070000,
    WRITE_PROTECTED_MEDIA = 0x072700,
    BLANK_CHECK = 0x080000,
    Vendor_Specific = 0x090000,
    ABORTED_COMMAND = 0x0B0000,
    OVERLAPPED_COMMAND_ATTEMPTED = 0x0B4E00,
    VOLUME_OVERFLOW = 0x0D0000,
    MISCOMPARE = 0x0E0000
}T_SENSE_KEYS;

/** @{@name sense offset define
 *      Define sense offset
 *      
 */
#define SENSE_KEY_OFFSET            2
#define ASC_OFFSET                  12
#define ASCQ_OFFSET                 13
/** @} */

/** @{@name CBW/CSW signature define
 *      Define CBW/CSW signature
 *      
 */
#define CBW_SIGNATURE               0x43425355
#define CSW_SIGNATURE               0x53425355
/** @} */

/** @{@name CBW/CSW flag define
 *      Define CBW/CSW flag
 *      
 */
#define CBWFLAGS_IN                 0x80
#define CBWFLAGS_OUT                0x00
/** @} */

/** @{@name CBW/CSW size define
 *      Define CBW/CSW size
 *      
 */
#define CBW_PKT_SIZE                31
#define CBW_PKT_HEAD_SIZE           15
#define CSW_PKT_SIZE                13      
/** @} */

#define DEV_STRING_BUF_LEN          0x24    
#define MSC_MAXIMUM_LUN             15

#define CMD_PASSED                  0
#define CMD_FAILED                  1
#define PHASE_ERROR                 2
#define NO_CSW_STATUS               0xff
//********************************************************************
/**
 * @brief usb cbw head struct
 */
typedef 
#ifdef __CC_ARM
__packed
#endif
struct S_USB_MASSCLASS_CBW_HEAD
{
    unsigned long       dCBWSignature;
    unsigned long       dCBWTag;
    unsigned long       dCBWDataTransferLength;
    unsigned char        bmCBWFlags;
    unsigned char        bCBWLUN;
    unsigned char        bCBWCBLength;
    unsigned char        pCBWCB[16];
}
#ifdef OS_ANYKA
#ifdef __GNUC__
	__attribute__((packed))
#endif

#endif
T_USB_CBW_HEAD;


/**
 * @brief usb csw struct
 */
typedef 
#ifdef __CC_ARM
__packed
#endif
struct S_USB_MASSCLASS_CSW_PKT
{
    unsigned long       dCSWSignature;
    unsigned long       dCSWTag;
    unsigned long       dCSWDataResidue;
    unsigned char        bCSWStatus;
}
#ifdef OS_ANYKA
#ifdef __GNUC__
	__attribute__((packed))
#endif

	
#endif
T_USB_CSW_PKT;

/**
 * @brief SCSI Command struct
 */
typedef 
#ifdef __CC_ARM
__packed
#endif	
struct S_SCSI_CMD
{
    unsigned char        bOperationCode;
    unsigned char        bLogicUnitNum;
    unsigned long       dLogicBlockAddr;
    unsigned long       dTransferLength;
    unsigned char        bReserve;
    unsigned char        bControl;
    unsigned long       dReserve;
}
#ifdef OS_ANYKA
#ifdef __GNUC__
	__attribute__((packed))
#endif

#endif
T_SCSI_CMD;

/**
 * @brief CBW Packet struct
 */
typedef 
#ifdef __CC_ARM
__packed
#endif
struct S_USB_CBW_PKT
{
    unsigned long       dCBWSignature;
    unsigned long       dCBWTag;
    unsigned long       dCBWDataTransferLength;
    unsigned char        bmCBWFlags;
    unsigned char        bCBWLUN;
    unsigned char        bCBWCBLength;
    T_SCSI_CMD  scsi;
}
#ifdef OS_ANYKA
#ifdef __GNUC__
	__attribute__((packed))
#endif
	
#endif
T_USB_CBW_PKT;

//********************************************************************
/*@}*/                      
#ifdef __cplusplus    
}                     
#endif                
                      
#endif                
                      
                      
                      
                      
                      
                      
                      
                      
                      
