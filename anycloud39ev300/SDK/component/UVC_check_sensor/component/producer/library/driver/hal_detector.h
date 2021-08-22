/**
 * @file    hal_detector.c
 * @brief   detector module, for detecting device connected or disconnected
 *          by check gpio or voltage of ADC.
 *          The detected event of gpio can be indicated by interrupt of the 
 *          gpio,or by a timer.
 *          The detected event of ADC is indicated by a timer.
 * 支持防抖处理: 
 *     断开会马上响应;
 *     接合会有防抖处理，防抖时间为1秒钟: 当接合时，若1秒内没有出现断开，即为
 *     有效接合。
 * Copyright (C) 2012nyka (GuangZhou) Software Technology Co., Ltd.
 * @author  wangguotian
 * @date    2012.03.09
 * @version 1.0
 */


/***************************************************************************
 The following device name is frequently-used:
 DEVICE NAME            DESCRIPTION
 ===========================================================================
 UDISK                  USB device
 USBHOST                USB host
 SD                     SD card
 MMC                    MMC Card
 TF                     TF Card
 HP                     earphone
 WIFI                   wifi card
 DC                     DC adapter
 
 ***************************************************************************/

 
#ifndef _HAL_DETECTOR_H_
#define _HAL_DETECTOR_H_

#include "anyka_types.h"

/**
 * @brief Voltage table structure for ADC checking.
 */
typedef struct _VOLTAGE_TABEL
{
    unsigned long   min_voltage;        ///< The minimum of the voltage range
    unsigned long   max_voltage;        ///< The maximum of the voltage range
    unsigned long   dev_connect_state;  ///< The connecting state of the devices 
}T_VOLTAGE_TABLE ;

/**
 * @brief detector call back function prototype;
 */
typedef void (*T_fDETECTOR_CALLBACK)(bool connect_state);


/**
 * @brief       Init detector module.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   void 
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
bool detector_init(void);


/**
 * @brief       Free detector module.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   void
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
bool detector_free(void);


/**
 * @brief       Set the call back function of device named by devname,the call 
 *              back function will be call when the device inserted or removed.
 *              After seting the call back function, the dettector of the 
 *              device will start automatically.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pcallbackfunc
 *                  Call back function of the device.
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
bool detector_set_callback(const char * devname,
            T_fDETECTOR_CALLBACK pcallbackfunc);


/**
 * @brief       Enable or disable the detector.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   benable
 *                  true：Enable the detector;
 *                  false：Disable the detector;
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 *
 * @remark      It's not suggested to disable the detector,if the detector
 *              is disable, the connecting state of the device can't be 
 *              informed the user.
 *
 */ 
bool detector_enable(const char * devname, bool benable);

/**
 * @brief       Determine whether the specified window is enabled.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pbenable
 *                  Pointer to a bool type variable for fetching the 
 *                  detector state.
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
bool detector_is_enabled(const char * devname, bool *pbenable);

/**
 * @brief       Get the connecting state of the device named by devname.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 * @param[in]   pState
 *                  Pointer to a bool type variable for fetching the 
 *                  connecting state.
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
bool detector_get_state(const char * devname, bool *pState);



/**
 * @brief       Register the detector of GPIO type.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname
 *                  Name of the device to be detected.
 *                  devname must point to a const string, because the detect 
 *                  module won't hold a copy of the device name. 
 * @param[in]   gpio_num
 *                  Number of the gpio.
 * @param[in]   active_level
 *                  Active logic level, 0 or 1. If the gpio is on the active
 *                  level, means the device is connected.
 * @param[in]   interrupt_mode
 *                  Detect type, true: interrupt, false: time.
 * @param[in]   interval_ms
 *                  The interval of checking, in ms.
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 */ 
bool detector_register_gpio(const char * devname, unsigned long gpio_num, 
            bool active_level, bool interrupt_mode, unsigned long interval_ms);


/**
 * @brief       Register the detector of ADC type.
 * @author      wangguotian
 * @date        2012.03.09
 * @param[in]   devname_list
 *                  Name list of the devices to be detected.
 *                  Each pointer in the name list must point to a const 
 *                  string, because the detect module won't hold a copy 
 *                  of the device name. 
 * @param[in]   devnum
 *                  Number of devices to be detected.
 * @param[in]   pvoltage_table
 *                  Voltage table .
 *                  pvoltage_table must point to a const memory, because
 *                  the detect module won't hold a copy of the Voltage 
 *                  table.
 * @param[in]   voltageitem_num
 *                  Number of voltage item of voltage table.
 * @param[in]   interval_ms
 *                  The interval of checking, in ms.
 * @return      bool
 * @retval      If the function succeeds, the return value is true;
 *              If the function fails, the return value is false.
 *
 * @remark      The member dev_connect_state of T_VOLTAGE_TABLE is a bitmap 
 *              of the devices' connecting state.1 means the device is 
 *              connected,and 0 means the device is disconnected.
 *              The bitmap order must correspond to the device's name in 
 *              devname_list, that is, bit 0 of dev_connect_state correspond
 *              to the first device named by devname_list[0], bit 1 of 
 *              dev_connect_state correspond to the second device named by 
 *              devname_list[1].
 *              All the devices specified by devname_list, must have different
 *              name, or else, the action of detector is not foreseeable.
 */ 
bool detector_register_adc(const char * *devname_list, unsigned long devnum, 
            const T_VOLTAGE_TABLE  *pvoltage_table, unsigned long voltageitem_num, 
            unsigned long interval_ms);        


#endif //_HAL_DETECTOR_H_

