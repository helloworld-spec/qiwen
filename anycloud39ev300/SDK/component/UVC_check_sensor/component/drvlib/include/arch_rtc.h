/** 
 * @file arch_rtc.h
 * @brief rtc module control
 *
 * Copyright (C) 2010 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liao_zhijun
 * @date 2010.04.29
 * @version 1.0
 */
#ifndef         __ARCH_RTC_H__
#define         __ARCH_RTC_H__

/** @defgroup RTC RTC group
 *  @ingroup Drv_Lib
 */
/*@{*/

/**
 * @brief system time struction
 
 *   define system time
 */
typedef struct tagSYSTIME{
    unsigned short   year;               ///< 4 byte: 1970-2099
    unsigned char    month;              ///< 1-12 
    unsigned char    day;                ///< 1-31 
    unsigned char    hour;               ///< 0-23 
    unsigned char    minute;             ///< 0-59 
    unsigned char    second;             ///< 0-59 
    unsigned short   milli_second;       ///< 0-999 
    unsigned char    week;               ///< 0-6,  1: monday, 7: sunday
} T_SYSTIME, *T_pSYSTIME;

typedef enum {
    WU_GPIO  = (1<<0),
    WU_ALARM = (1<<1),
    WU_USB = (1<<2),
    WU_AIN0 = (1<<3),
    WU_AIN1 =  (1<<4)
} T_WU_TYPE;

/**
 *  rtc event callback handler
 */
typedef void (*T_fRTC_CALLBACK)(void);

/**
 * @brief   init rtc module
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param year [in] current year
 * @return  void
 */
void  rtc_init(unsigned long year);

/**
 * @brief   set rtc alarm event callback handler
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param cb [in]  the alarm event callback handler
 * @return  void
 */
void  rtc_set_callback(T_fRTC_CALLBACK cb);

/**
*@brief: set gpio wakeup polarity
*@author: jiankui
*@date: 2016-09-26
*@param: polarity[in]: 0:rising  1:falling
*@param: wgpio_mask[in]: the wakeup gpio value
*@return: void
*/
void rtc_set_gpio_p(unsigned char polarity, unsigned long wgpio_mask);

/**
*@brief: set AIN wakeup polarity
*@author:jiankui
*@date: 2016-09-26
*@param: polarity[in]: wakeup polarity  0:rising 1:falling
*@param: type[in]: wakeup type 
*@return: void
*/
void rtc_set_AIN_p(unsigned char polarity, T_WU_TYPE type);


/**
 * @brief   enter standby mode
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return  void
 */
 
void  rtc_enter_standby(void);

/**
 * @brief   exit standby mode and return the reason
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return  unsigned long the reason of exitting standby
 * @retval  low 8-bit is wakeup type, refer to T_WU_TYPE
 * @retval  upper 8-bit stands for gpio number if WGPIO wakeup
 */
unsigned short   rtc_exit_standby(void);

/**
 * @brief  set wakeup type for exiting standby mode
 *
 * @author xuchang
 * @param type [in] wakeup type, WU_GPIO and WU_ALARM default opened
 * @return void
 */
void rtc_set_wakeup_type(T_WU_TYPE type);

/**
 * @brief   set wakeup gpio of standby mode
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param wgpio_mask [in]  the wakeup gpio value
 * @return      void
 */
void  rtc_set_wgpio(unsigned long wgpio_mask);

/**
 * @brief   set wakeuppin active leval
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param wpinLevel [in]  the wakeup signal active level 1:low active,0:high active
 * @return      void
 */
void  rtc_set_wpinLevel(bool wpinLevel);

/**
 * @brief   set rtc start count value in seconds
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param rtc_value [in] the rtc count to be set
 * @return      void
 */
void rtc_set_RTCcount(unsigned long rtc_value);

/**
 * @brief set rtc register value by system time
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param systime T_SYSTIME : system time structure
 * @return unsigned long day num
 */
void rtc_set_RTCbySystime(T_SYSTIME *systime);


/**
 * @brief   get rtc passed count in seconds
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return unsigned long the rtc count
 */
unsigned long rtc_get_RTCcount(void);

/**
 * @brief get current system time from rtc
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return system time structure
 */
T_SYSTIME rtc_get_RTCsystime(void);


/**
 * @brief   set rtc alarm count.
 *
 * when the rtc count reaches to the alarm  count, 
 * AK chip is woken up if in standby mode and rtc interrupt happens.
 * @author liao_zhijun
 * @date 2010-04-29
 * @param rtc_wakeup_value [in] alarm count in seconds
 * @return void
 */
void  rtc_set_alarmcount(unsigned long rtc_wakeup_value);

/**
 * @brief set alarm by system time
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param systime system time structure
 * @return unsigned long: day num
 */
void rtc_set_AlarmBySystime(T_SYSTIME *systime);

/**
 * @brief get alarm count that has been set.
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return unsigned long
 * @retval the alarm count in seconds
 */
unsigned long   rtc_get_alarmcount(void);

/**
 * @brief get alarm system time from rtc
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return T_SYSTIME system time structure
 */
T_SYSTIME rtc_get_AlarmSystime(void);

/**
 * @brief enable or disable power down alarm
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param alarm_on [in]  enable power down alarm or not
 * @return void
 */
void  rtc_set_powerdownalarm(bool alarm_on);

/**
 * @brief query alarm status
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return bool
 * @retval true alarm has occured
 * @retval false alarm hasn't occured
 */
bool rtc_get_alarm_status(void);

/**
* @brief rtc set time interface for user whether have rtc or not
 * 
 * @author wumingjin
 * @date 2017-07-06
 * @return void
 
**/
void rtc_set_systime(T_SYSTIME *systime);

/**
* @brief rtc get time interface for user whether have rtc or not
 * 
 * @author wumingjin
 * @date 2017-07-06
 * @return T_SYSTIME
 
**/
T_SYSTIME rtc_get_systime(void);



/**
 * @brief watch dog function init
 * @author liao_zhijun
 * @date 2010-05-28
 * @param[in] feedtime unsigned short feedtime:watch dog feed time, feedtime unit:ms
 * @param[in] rst_level unsigned char rst_level:reset level for WAKEUP pin after watchdog feedtime expired
 * @return void
  */
void watchdog_init(unsigned short feedtime, unsigned char rst_level);

/**
 * @brief watch dog function start
 * @author liao_zhijun
 * @date 2010-05-28
 * @return void
  */
void watchdog_start(void);

/**
 * @brief watch dog function stop
 * @author liao_zhijun
 * @date 2010-05-28
 * @return void
  */
void watchdog_stop(void);

/**
 * @brief feed watch dog
 * @author liao_zhijun
 * @date 2010-05-28
 * @return void
 * @note this function must be called periodically, 
    otherwise watchdog will expired and reset.
  */
void watchdog_feed(void);

/*@}*/
#endif//__ARCH_RTC_H__

