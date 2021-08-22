/** @file rtc.c
 *  @brief rtc module control
 *
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author liao_zhijun 
 * @date 2010.05.28
 * @version 1.0
 */
#include "anyka_cpu.h" 
#include "anyka_types.h"
#include "drv_api.h"
#include "interrupt.h"
#include "arch_rtc.h"
#include "l2.h"
#include "dac.h"
#include "standby.h"
#include "drv_module.h"

//set and get bist macro; bs:start bit position, be:end bit positon
#define BIT_MASK(__bf) (((1U << ((be ## __bf) - (bs ## __bf) + 1)) - 1U) << (bs ## __bf)) 
#define SET_BITS(__dst, __bf, __val) \
        ((__dst) = ((__dst) & ~(BIT_MASK(__bf))) | \
        (((__val) << (bs ## __bf)) & (BIT_MASK(__bf)))) 
#define GET_BITS(__src, __bf)   \
        (((__src) & BIT_MASK(__bf)) >> (bs ## __bf))

//RTC_CONFIG_REG bits define
#define bsDataAddr              0
#define beDataAddr              13
#define bsIntRegAddr            14
#define beIntRegAddr            16
#define bsRWmodeAddr            17
#define beRWmodeAddr            21

//RealTime Register1 bits define
#define bsSecAddr              0
#define beSecAddr              5
#define bsMinAddr              6
#define beMinAddr              11

//RealTime Register2 bits define
#define bsHourAddr              0
#define beHourAddr              4
#define bsDayMonthAddr          5
#define beDayMonthAddr          9
#define bsDayWeekAddr           10
#define beDayWeekAddr           12

//RealTime Register3 bits define
#define bsMonthAddr             0
#define beMonthAddr             3
#define bsYearAddr              4
#define beYearAddr              10

//watchdog time configuration bits define
#define bsWDTimeAddr            0
#define beWDTimeAddr            12

//RTC_BACK_DATA_REG bits define
#define bsBackDataAddr          0
#define beBackDataAddr          13

//RTC module includes 6 internal registers defined as follows
#define REALTIME1_REG           0
#define REALTIME2_REG           1
#define REALTIME3_REG           2
#define ALARMTIME1_REG          3
#define ALARMTIME2_REG          4
#define ALARMTIME3_REG          5
#define WDOG_TIMER_REG          6
#define RTC_SETTING_REG         7

#define ALARM_ENABLE            (1<<13)

//rtc setting bits
#define WDOG_TIMER_SEL          (1<<10)
#define REAL_TIME_WR            (1<<3)
#define REAL_TIME_RE            (1<<4)

#define WAKEUP_EN               (1<<2)
#define TIMER_STATUS_CLEAR      (1<<1)
#define ALARM_STATUS_CLEAR      (1<<0)

//RTC Module Command
#define WRITE_MODE              0x14
#define READ_MODE               0x15

#define RTC_ENABLE_WAKEUP_FROM_STANDBY          (0x1<<16)
#define RTC_ENTER_STANDBY_MODE                  (0x1<<13)

//watchdog function bits
#define WD_ENABLE                               (0x1<<13)
#define WD_DISABLE                              (0x0<<13)

#define BASE_YEAR    1970
#define BASE_WEEK    3         /*week 0 - 6*/


/*
    对于AK880x，设置rtc寄存器时rtc ready interrupt不能被mask，否则读不到status
    但如果一直打开rtc ready interrupt又会一直有中断来
    所以设置rtc寄存器时的顺序为：
    1.mask system control interrupt
    2.unmask rtc ready interrupt
    3.set rtc register and check rtc ready status
    4.mask rtc ready interrupt
    5.unmask system control interrupt
*/
#define RTC_ENABLE_RDY_INTR     \
        {store_all_int(); \
        REG32(INT_SYS_MODULE_REG) |= (1<<7); \
        while(!(REG32(INT_STATUS_REG_2)|(1<<7)));\
        REG32(RTC_CONFIG_REG) |= (1<<25);}
#define RTC_DISABLE_RDY_INTR     \
        REG32(RTC_CONFIG_REG) &= ~(1<<25); \
        {REG32(INT_SYS_MODULE_REG) &= ~(1<<7); \
        restore_all_int();}
        
        
//clear all wakeup gpio status
#define CLEAR_ALL_WGPIO (REG32(RTC_WAKEUP_GPIO_C_REG) = 0xffffffff)

//default wgpio polarity value, must be set according to actual harware
#define DEFAULT_POL             0x00

//default wakeup pin level
#define DEFAULT_WPIN_LEVEL      0

//rtc message, used in message map
#define RTC_MESSAGE             3

#define RTC_PROTECT \
do{ \
    DrvModule_Protect(DRV_MODULE_RTC); \
}while(0)

#define RTC_UNPROTECT \
do{ \
    DrvModule_UnProtect(DRV_MODULE_RTC);\
}while(0)

/*
wakup pin active level. Because rtc_set_wpinLevel() is a new interface,
if user doesn't call it.Driver must set a default value according to actual hardware
*/
static bool   s_bSetWpinLevel = false;

static bool   rtc_normal_flag = false;
/*system init time 1970-1-1 00:00:00:000 Thursday*/
static unsigned long g_start_seconds = 0;


//rtc call back function
static T_fRTC_CALLBACK m_RTCCallback = NULL;

static unsigned long    read_rtc_inter_reg(unsigned char reg);
static void   write_rtc_inter_reg(unsigned char reg, unsigned long value);

static bool   rtc_interrupt_handler(void);
static void   rtc_callback(unsigned long *param, unsigned long len);

static T_SYSTIME CalcSystimeByRTC(unsigned long RealTimeRet1, unsigned long RealTimeRet2, unsigned long RealTimeRet3);
static void   ConvertSecondsToSysTime(unsigned long seconds, T_SYSTIME *SysTime);
static unsigned long    ConvertTimeToSeconds(T_SYSTIME *SysTime);

/**
 * @convert seconds counted from 1970-01-01 00:00:00 to system time
 * @author YiRuoxiang
 * @date 2006-02-16
 * @param  the seconds want to added, which are counted from 1970-01-01 00:00:00
 * @the seconds can't be more than 4102444799 (2099-12-31 23:59:59)
 * @param T_SYSTIME *SysTime: system time structure pointer
 * @return unsigned char
 * @retval if system time is more than 2099-12-31 23:59:59, return false
 *          else return true;
 */
static void ConvertSecondsToSysTime(unsigned long seconds, T_SYSTIME *SysTime)
{
	unsigned int TmpVal;
	unsigned int year, mouth;
	
	const  unsigned char month_std_day[13]	= {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	const  unsigned char month_leap_day[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	if (NULL == SysTime)
	{
		return;
	}

	TmpVal		 = seconds % 60;
	SysTime->second = (unsigned char)TmpVal;
	TmpVal		 = seconds / 60;
	SysTime->minute = (unsigned char)(TmpVal % 60);
	TmpVal		 = TmpVal / 60;
	SysTime->hour	= (unsigned char)(TmpVal % 24);

	/* day is from the number: 1, so we should add one after the devision. */
	TmpVal	= (seconds / (24 * 3600)) + 1;
	year	= BASE_YEAR;
	while (TmpVal > 365)
	{
		if (0 == (year % 4) && ((year % 100) != 0 || 0 == (year % 400)))
		{
			TmpVal -= 366;
			/* Added to avoid the case of "day==0" */
			if (0 == TmpVal)
			{
				/* resume the value and stop the loop */
				TmpVal = 366;
				break;
			}
		}
		else
		{
			TmpVal -= 365;
		}
		year++;
	}
	
	mouth = 1;
	if (0 == (year % 4) && ((year % 100) != 0 || 0 == (year % 400)))
	{
		while (TmpVal > month_leap_day[mouth])
		{
			TmpVal -= month_leap_day[mouth];
			mouth++;
		}
	}
	else
	{
		/* Dec23,06 - Modified from month_leap_day[], since here is not leap year */
		while (TmpVal > month_std_day[mouth])
		{
			TmpVal -= month_std_day[mouth];
			mouth++;
		}
	}

	SysTime->year  = (unsigned short)year;
	SysTime->month = (unsigned char)mouth;
	SysTime->day   = (unsigned char)TmpVal;

	/*week*/
	TmpVal	= (seconds / (24 * 3600)) % 7;
	SysTime->week = BASE_WEEK + TmpVal;
	if(SysTime->week > 7)
	{
		SysTime->week -= 7;
	}
	return;
}

/**
 * @convert system time to seconds counted from 1970-01-01 00:00:00
 * @author YiRuoxiang
 * @date 2006-02-17
 * @param T_SYSTIME SysTime: system time structure
 * @return unsigned long: seconds counted from 1970-01-01 00:00:00
 */
static unsigned long ConvertTimeToSeconds(T_SYSTIME *SysTime)
{
		unsigned int current_time = 0;
		unsigned short MonthToDays	= 0;
		unsigned short std_month_days[13]  = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		unsigned short leap_month_days[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		unsigned short TempYear;
		unsigned short month;
		unsigned short i;
	
		if (NULL == SysTime)
		{
			return 0;
		}
		month = SysTime->month;
	
		if(SysTime->year < BASE_YEAR) 
			SysTime->year= BASE_YEAR;
		for (TempYear = SysTime->year - 1; TempYear >= BASE_YEAR; TempYear--)
		{
			/* the case of leap year */
			if (0 == (TempYear % 4) && ((TempYear % 100) != 0 || 0 == (TempYear % 400)))
			{
				current_time += 31622400; // the seconds of a leap year
			}
			else
			{
				/* not a leap year */
				current_time += 31536000;  // the seconds of a common year(not a leap one)
			}
		}
	
		/* calculate the current year's seconds */
	
		if ((month < 1) || (month > 12))
		{
			/* get the default value. */
			month		= 1;
			SysTime->month = 1;
		}
	
		/* the current year is a leap one */
		if (0 == (SysTime->year % 4) && ((SysTime->year % 100) != 0 || 0 == (SysTime->year % 400)))
		{
			for (i = 1; i < month; i++)
			{
				MonthToDays += leap_month_days[i];
			}
		}
		else
		{
			/* the current year is not a leap one */
			for (i = 1; i < month; i++)
			{
				MonthToDays += std_month_days[i];
			}
		}
	
		if ((SysTime->day < 1) || (SysTime->day > 31))
		{
			/* get the default value */
			SysTime->day = 1;
		}
		MonthToDays += (SysTime->day - 1);
	
		/* added the past days of this year(change to seconds) */
		current_time += MonthToDays * 24 * 3600;
	
		/* added the current day's time(seconds) */
		current_time += SysTime->hour * 3600 + SysTime->minute * 60 + SysTime->second;
	
		return current_time;
	}


/**
 * @brief get system time from rtc
 * @author liao_zhijun
 * @date 2010-05-28
 * @param RealTimeRet1[in]: rtc register1 value
 * @param RealTimeRet2[in]: rtc register2 value
 * @return T_SYSTIME SysTime: system time structure
 */
static T_SYSTIME CalcSystimeByRTC(unsigned long RealTimeRet1, unsigned long RealTimeRet2, unsigned long RealTimeRet3)
{
    T_SYSTIME systime;   

    systime.year =  ((RealTimeRet3 >> 4) & 0x7f) + BASE_YEAR;
    systime.month = RealTimeRet3 & 0xf;
    systime.day = (RealTimeRet2 >> 5) & 0x1f;
    systime.week = ((RealTimeRet2 >> 10) & 0x7);
    if(systime.week > 0)
    {
        systime.week -= 1;
    }
    
    systime.hour = RealTimeRet2 & 0x1f;
    systime.minute = (RealTimeRet1 >> 6) &0x3f;
    systime.second = RealTimeRet1 & 0x3f;

#ifdef RTC_DEBUG
    akprintf(C3, M_DRVSYS, "CalcSystimeByRTC(): year = %d, month = %d, day = %d, hour = %d, minute = %d \
        second = %d, week = %d\n", systime.year, systime.month, systime.day, \
        systime.hour, systime.minute, systime.second, systime.week);
#endif    

    return systime;
}
/**
* @brief judge rtc  whether normal or not
 * 
 * @author
 * @date 2017-06-30
 * @return bool
 * @retval true rtc normal
 * @retval false rtc abnormal

**/
static void rtc_init_judge(void)
{
	T_SYSTIME systime1,systime2;
	
	systime1.year    =  1997; 
    systime1.month   =  7;
    systime1.day     =  1;  
    systime1.hour    =  12;    
    systime1.minute  =  12;  
    systime1.second  =  12; 
	systime1.week    =  6;	

	systime2 = rtc_get_RTCsystime();
	if(ConvertTimeToSeconds(&systime2) >= ConvertTimeToSeconds(&systime1))
	{		
		rtc_normal_flag = true;
	}
	else
	{
		rtc_set_RTCbySystime(&systime1);
		
		systime2 = rtc_get_RTCsystime();
		
		if((systime1.year == systime2.year)&&(systime1.month == systime2.month)&&\
			(systime1.day == systime2.day)&&(systime1.hour == systime2.hour)&&\
			(systime1.hour == systime2.hour)&&(systime1.minute == systime2.minute)&&\
			(systime1.second == systime2.second))
		{
			rtc_normal_flag = true;
		}
		else
		{			
			rtc_normal_flag = false;
		}
	}
}
/**
 * @brief   init rtc module
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param year [in] current year
 * @return  void
 */
void rtc_init(unsigned long year)
{
    unsigned long ret;

	RTC_PROTECT;
    //open rtc module
    REG32(RTC_CONFIG_REG) |= (1<<24);
    //enable wakeup function and clear wake up signal status
    ret = read_rtc_inter_reg(RTC_SETTING_REG);
    ret |= WAKEUP_EN | ALARM_STATUS_CLEAR;   
    write_rtc_inter_reg(RTC_SETTING_REG, ret);

	
	RTC_UNPROTECT;
	rtc_init_judge();
	
}
/**
 * @brief   rtc call back function used in drv module
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param param: param passed from message, not used here
 * @param len: param length, not used here
 * @return  void
 */
void rtc_callback(unsigned long *param, unsigned long len)
{
    if (NULL != m_RTCCallback)
    {    
        m_RTCCallback();
    }    
}

/**
 * @brief   set rtc event callback handler
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param cb [in]  the callback handler
 * @return  void
 */
void rtc_set_callback(T_fRTC_CALLBACK cb)
{
    m_RTCCallback = cb;

    //create task
    DrvModule_Create_Task(DRV_MODULE_RTC);
    DrvModule_Map_Message(DRV_MODULE_RTC, RTC_MESSAGE, rtc_callback);

    INTR_ENABLE_L2(IRQ_MASK_RTC_BIT);

    int_register_irq(INT_VECTOR_RTC, rtc_interrupt_handler);   
}

//10ms timeout, but there is register access, timeout value maybe various
#define MAX_WAIT_TIME       (get_cpu_freq()/7/100) 

//check if intern rtc works or not
static bool test_rtc_inter_reg(unsigned char reg)
{
    unsigned long status;
    unsigned long ConfigValue = 0, timeout = MAX_WAIT_TIME;
    bool ret=true;
    
    RTC_ENABLE_RDY_INTR   

    //read back  reg
    ConfigValue = REG32(RTC_CONFIG_REG);
    SET_BITS(ConfigValue, RWmodeAddr, READ_MODE);
    SET_BITS(ConfigValue, IntRegAddr, reg);
    REG32(RTC_CONFIG_REG) = ConfigValue;
    do {
        status = REG32(INT_STATUS_REG_2);
    }while(!(status & INT_STATUS_RTC_RDY_BIT) && timeout--);
        
    RTC_DISABLE_RDY_INTR

    return ret;
}

//read rtc interner register
unsigned long read_rtc_inter_reg(unsigned char reg)
{
    unsigned long status;
    unsigned long ConfigValue = 0, timeout=MAX_WAIT_TIME;

    RTC_ENABLE_RDY_INTR    

    //read back  reg
    ConfigValue = REG32(RTC_CONFIG_REG);
	ConfigValue |= (1 << 25);
    SET_BITS(ConfigValue, RWmodeAddr, READ_MODE);
    SET_BITS(ConfigValue, IntRegAddr, reg);
    REG32(RTC_CONFIG_REG) = ConfigValue;

    do {
        status = REG32(INT_STATUS_REG_2);
    }while(!(status & INT_STATUS_RTC_RDY_BIT) && timeout--);
    
    RTC_DISABLE_RDY_INTR
    
    //wait 1/32K s here
    us_delay(312);

    if (!timeout)
        return 0;
    return GET_BITS(REG32(RTC_BACK_DAT_REG), BackDataAddr);
}

//write rtc interner register
void write_rtc_inter_reg(unsigned char reg, unsigned long value)
{
    unsigned long status;
    unsigned long ConfigValue = 0, timeout=MAX_WAIT_TIME;
        
    RTC_ENABLE_RDY_INTR

    //write setdata to wakeupset_reg
    ConfigValue = REG32(RTC_CONFIG_REG);
	ConfigValue |= (1 << 25);
    SET_BITS(ConfigValue, DataAddr, value);
    SET_BITS(ConfigValue, RWmodeAddr, WRITE_MODE);
    SET_BITS(ConfigValue, IntRegAddr, reg);
    REG32(RTC_CONFIG_REG) = ConfigValue;
    do {
        status = REG32(INT_STATUS_REG_2);
    }while(!(status & INT_STATUS_RTC_RDY_BIT) && timeout--);

    RTC_DISABLE_RDY_INTR
    
    return;
}



//rtc callback function, called by IRQ interrupt handler
static bool   rtc_interrupt_handler(void)
{       
    unsigned long ret, reg;
	
    reg = REG32(INT_STATUS_REG_2);

    if((reg & INT_STATUS_RTC_RDY_BIT))
    {
        REG32(INT_SYS_MODULE_REG) &= ~IRQ_MASK_RTC_RDY_BIT;
        //akprintf(C3, M_DRVSYS, "error rtc int!!\n");
    }

	if((reg & INT_STATUS_RTC_BIT))
    {
        REG32(INT_SYS_MODULE_REG) &= ~IRQ_MASK_RTC_BIT;
    }

    if((reg & INT_STATUS_RTC_BIT) == 0)
        return false;
    
    /* the int_status bit ust be cleared here, do not delete it*/
    ret = read_rtc_inter_reg(RTC_SETTING_REG);
    ret |= ALARM_STATUS_CLEAR;
    write_rtc_inter_reg(RTC_SETTING_REG, ret);
	
    //drv module send message
    DrvModule_Send_Message(DRV_MODULE_RTC, RTC_MESSAGE, NULL);

    return true;
}
/**
*@brief: set gpio wakeup polarity
*@author: jiankui
*@date: 2016-09-26
*@param: polarity[in]: 0:rising  1:falling
*@param: wgpio_mask[in]: the wakeup gpio value
*@return: void
*/
void rtc_set_gpio_p(unsigned char polarity, unsigned long wgpio_mask)
{
	if(0 == polarity)
	{
		//set rising wake-up
		REG32(RTC_WAKEUP_GPIO_P_REG) &= ~wgpio_mask;
	}
	else
	{
		//set falling wake-up
		REG32(RTC_WAKEUP_GPIO_P_REG) |= wgpio_mask;
	}
}
/**
*@brief: set AIN wakeup polarity
*@author:jiankui
*@date: 2016-09-26
*@param: polarity[in]: wakeup polarity  0:low level 1:high level
*@param: type[in]: wakeup type 
*@return: void
*/
void rtc_set_AIN_p(unsigned char polarity, T_WU_TYPE type)
{
	if(0 == polarity)
	{
		if(WU_AIN0 & type)
		{
			//set low wake-up
			REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(1<<1);
		}
		if(WU_AIN1 & type)
		{
			//set low level wake-up
			REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(1<<0);
		}
	}
	else
	{
		if(WU_AIN0 & type)
		{
			//set high level wake-up
			REG32(RTC_WAKEUP_OTHER_CON_REG) |= (1<<1);
		}
		if(WU_AIN1 & type)
		{
			//set high level wake-up
			REG32(RTC_WAKEUP_OTHER_CON_REG) |= (1<<0);
		}
	}
}

/**
 * @brief   enter standby mode
 *
 * @author jian_kui
 * @date 2016-09-26
 * @return  void
 */
void rtc_enter_standby(void)
{

    unsigned long ret,i;
    unsigned long asic_freq, state=0;   //state 1: cpu2x, 2:cpu3x, 0:cpu=asic
    unsigned long lcd_readback_reg;


    //NOTE:fix chip bug, IR drop problem. to avoid this, enter standby with low clock.
    asic_freq = get_asic_freq();
        
    l2_specific_exebuf(enter_standby, 0);

    //restore clock setting
    set_asic_freq(asic_freq);

}


/**
 * @brief   exit standby mode and return the reason
 *
 * @author jian_kui
 * @date 2016-09-21
 * @return  unsigned short the reason of exitting standby
 * @retval  low 8-bit is wakeup type, refer to T_WU_TYPE
 * @retval  upper 8-bit stands for gpio number if WGPIO wakeup
 */
 unsigned short rtc_exit_standby(void)
{
    unsigned long  reason = 0;
	unsigned short  retval = 0;
	unsigned long ret;
    //usb wakeup
    if((REG32(RTC_WAKEUP_OTHER_CON_REG)&(1<<14))&&(REG32(RTC_WAKEUP_OTHER_S_REG) & (1<<4)))
    {
        akprintf(C3, M_DRVSYS, "wakeup by usb vbus\n");
		retval = WU_USB;
    }
	//ALARM wakeup
	if((REG32(RTC_WAKEUP_OTHER_CON_REG)&(1<<12))&&(REG32(RTC_WAKEUP_OTHER_S_REG) & (1<<2)))
    {
        akprintf(C3, M_DRVSYS, "wakeup by Alarm\n");
		retval = WU_ALARM;
    }
    //AIN0 
    if ((REG32(RTC_WAKEUP_OTHER_CON_REG)&(1<<11))&&(REG32(RTC_WAKEUP_OTHER_S_REG) & (1<<1)))
    {
        akprintf(C3, M_DRVSYS, "wakeup by AIN0\n");
		retval = WU_AIN0;
    }
	//AIN1
	if ((REG32(RTC_WAKEUP_OTHER_CON_REG)&(1<<10))&&(REG32(RTC_WAKEUP_OTHER_S_REG) & (1<<0)))
    {
        akprintf(C3, M_DRVSYS, "wakeup by AIN1\n");
		retval = WU_AIN1;
    }	
	
	//disable all wakeup
    REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(0x1f<<10);
	//clear all wakeup status
	REG32(RTC_WAKEUP_OTHER_CON_REG) |= (0x1F<<5);
	REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(0x1F<<5);
	
	/* the int_status bit ust be cleared here, do not delete it*/
	/*清除报警中断状态*/
	RTC_PROTECT;
    ret = read_rtc_inter_reg(7);
    ret |= (1<<0);
    write_rtc_inter_reg(7, ret);
	
	RTC_UNPROTECT;
	rtc_set_powerdownalarm(false);// RTC close output wakeup signal

	if(retval)
	{
		return retval;
	}
    reason = REG32(RTC_WAKEUP_GPIO_S_REG) & REG32(RTC_WAKEUP_GPIO_E_REG);
    REG32(RTC_WAKEUP_GPIO_C_REG) = 0xffffffff;
    REG32(RTC_WAKEUP_GPIO_C_REG) = 0;
	
    if (0 == reason)
    {
   	 	//disable all wakeup
   	 	REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(0x1f<<10);
        akprintf(C1, M_DRVSYS, "error wakeup \n");
        return WU_ALARM;
    }
    else
    {
    	//disable all wakeup
    	REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(0x1f<<10);
        akprintf(C3, M_DRVSYS, "wakeup by wgpio, gpio is %d \n", gpio_get_wakeup_pin(reason));
        return (WU_GPIO|(gpio_get_wakeup_pin(reason)<<8));
    }

	return 1;
}


/**
 * @brief   set wakeup gpio of standby mode
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param wgpio_mask [in]  the wakeup gpio value
 * @return      void
 */
void rtc_set_wgpio(unsigned long wgpio_mask)
{
    //enable RTC wakeup function and clear wakupgpio status.
    REG32(RTC_WAKEUP_GPIO_C_REG) |= wgpio_mask;
	REG32(RTC_WAKEUP_GPIO_S_REG) &= ~wgpio_mask;
    REG32(RTC_WAKEUP_GPIO_C_REG) &= ~wgpio_mask;
	
	

	//enable gpio wake-up
    REG32(RTC_WAKEUP_GPIO_E_REG) = wgpio_mask;
}

/**
 * @brief   set adc wakeup cfg of standby mode
 *
 * @author liu_huadong
 * @date 2011-05-19
 * @param wgpio_mask [in]  the wakeup gpio value
 * @return      void
 */

void ADC_CfgVoiceW(unsigned char freq, unsigned char ref, unsigned char time)
{
#if 0
    REG32(ANALOG_CTRL_REG4) &= ~(0x3UL << 30);    
    REG32(ANALOG_CTRL_REG4) &= ~(0x7UL << 27);   
    REG32(ANALOG_CTRL_REG4) &= ~(0x3 << 25);
    REG32(ANALOG_CTRL_REG4) |= ((freq & 0x3UL) << 30);    
    REG32(ANALOG_CTRL_REG4) |= ((ref & 0x7UL) << 27);    
    REG32(ANALOG_CTRL_REG4) |= ((time & 0x3) << 25);
#endif

}

/**
 * @brief  open or close voice wakeup function
 *
 * @author liao_Zhijun
 * @date 2011-02-28
 * @return  void
 */
static void voice_wakeup(bool enable)
{
#if 0
    REG32(ANALOG_CTRL_REG4) |= (1 << 24);    //poweroff mic_n   
    REG32(ANALOG_CTRL_REG4) |= (1 << 23);    //poweroff mic_p    
    REG32(ANALOG_CTRL_REG4) &= ~(0x7 << 2);  // set no input to adc2
    REG32(ANALOG_CTRL_REG3) &= ~(0x7 << 12); // set hp mute       
    
    //power off voice wakeup before    
    REG32(ANALOG_CTRL_REG4) |= (1 << 15);    //power down voice wake up    
    REG32(ANALOG_CTRL_REG3) |= (0x4 << 12);  // set hp from mic    
    REG32(ANALOG_CTRL_REG3) |= (0x1 << 17);  // HP with 3k to gnd    
    
    mini_delay(5);
    
    REG32(ANALOG_CTRL_REG3) &= ~(0x1 << 2);  // no pl_vcm2 with 2k to gnd    
    REG32(ANALOG_CTRL_REG4) &= ~(0x1<<0);    // no Pl_vcm3 with 2k to gnd   
    REG32(ANALOG_CTRL_REG3) &= ~(0x1F << 4); //disable descharge for VCM2    
    REG32(ANALOG_CTRL_REG3) &= ~(1 << 0);    // power on bias   
    REG32(ANALOG_CTRL_REG3) &= ~(1 << 1);    // power on vcm2  
    REG32(ANALOG_CTRL_REG3) &= ~(1 << 23);   // vcm3 from refrec1.5    
    REG32(ANALOG_CTRL_REG3) &= ~(1 << 3);    // power on  vcm3            
    REG32(USB_DETECT_CTRL_REG) &= ~(1 << 5);    //disable voice wakeup interrupt    
    REG32(USB_DETECT_CTRL_REG) |= (1 << 3);     //cle ar voice wakeup interrupt status    
    REG32(USB_DETECT_CTRL_REG) &= ~(1 << 3);     
    
    mini_delay(500);    
    
    REG32(ANALOG_CTRL_REG3) |= (1 << 1);     // power off vcm2    
    REG32(ANALOG_CTRL_REG3) |= (1 << 3);     // power off vcm3    
    REG32(ANALOG_CTRL_REG3) |= (1 << 0);     // power off bias           
    REG32(ANALOG_CTRL_REG3) &= ~(0x7 << 12); // set hp mute    
    REG32(ANALOG_CTRL_REG3) &= ~(0x1 << 17); //no HP with 3k to gnd            
    REG32(USB_DETECT_CTRL_REG) &= ~(1 << 1);    //rising    
    
    ADC_CfgVoiceW(0x0, 0x0, 0x0);             
    
    REG32(USB_DETECT_CTRL_REG) |= (1 << 5);      //enable voice wakeup interrupt    
    REG32(ANALOG_CTRL_REG4) &= ~(1 << 15);    //power on pd_vw, vcm3 from avcc    

    mini_delay(10);
#endif
}


/**
 * @brief   usb vbus wakeup control.
 *
 * @author Huang Xin
 * @date 2010-12-16
 * @param enable [IN] true enable wakeup by usb vbus; false disable wakeup by usb vbus
 * @return  void
 */
static void usb_vbus_wakeup(bool enable)
{
    //open usb wake up
    if(enable)
    {
        REG32(RTC_WAKEUP_OTHER_CON_REG) |= (1 << 14);     //enable usb DP wake up
    }
    else
    {
        REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(1 << 14);    //disable usb DP wake up
    }
}

/**
 * @brief  set wakeup type for exiting standby mode
 *
 * @author xuchang
 * @param type [in] wakeup type, WU_GPIO and WU_ALARM default opened
 * @return void
 */
void rtc_set_wakeup_type(T_WU_TYPE type)
{
	unsigned long ret;
	//clean all wakeup status
	REG32(RTC_WAKEUP_OTHER_CON_REG) |=(0x1F<<5);
	REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(0x1F<<5);

	//usb vbus wakeup
    if(type & WU_USB)  
    {
        usb_vbus_wakeup(true);     
    }
    else
    {
    	usb_vbus_wakeup(false);     
	}
	//alarm 
	if(type & WU_ALARM)
	{
		REG32(RTC_WAKEUP_OTHER_CON_REG) |= (1<<12);
		rtc_set_powerdownalarm(true);
		REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(1<<7);
	}
	else 
	{
		REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(1<<12);
		rtc_set_powerdownalarm(false);
		REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(1<<7);
	}
	//AIN0
    if(type & WU_AIN0) 
    {
    	
    	REG32(RTC_WAKEUP_OTHER_CON_REG) |=(1<<11); //enable AIN0 wake up function
    	REG32(AUDIO_CODEC_CFG2_REG) |= (1<<8); //enable AIN0 wakeup signal,high ative
    }
	else
	{
		REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(1<<11);
		REG32(AUDIO_CODEC_CFG2_REG) &= ~(1<<8); //disable AIN0 wakeup signal,high ative
	}
    //AIN1
	if(type & WU_AIN1)
	{
		
		REG32(RTC_WAKEUP_OTHER_CON_REG) |= (1<<10);//enable AIN1 wake up function
		REG32(AUDIO_CODEC_CFG2_REG) |= (1<<9);  //enable AIN1 wakeup signal,high ative
	}
	else
	{
		REG32(RTC_WAKEUP_OTHER_CON_REG) &= ~(1<<10);
		REG32(AUDIO_CODEC_CFG2_REG) &= ~(1<<9);  //enable AIN1 wakeup signal,high ative
	}
}

/**
 * @brief   set wakeuppin active leval
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param wpinLevel [in]  the wakeup signal active level 1:low active,0:high active
 * @return      void
 */
void  rtc_set_wpinLevel(bool wpinLevel)
{
    unsigned long ret = 0;
    unsigned long bit;
#if 0
    //init rtc
    REG32(RTC_CONFIG_REG) |= (1<<24);
    //check rtc works or not
    if (!test_rtc_inter_reg(0))
        return;
        
    if(wpinLevel) bit = 8;
    else bit = 9;

    ret = read_rtc_inter_reg(RTC_SETTING_REG);
    ret |= (1<<bit);
    write_rtc_inter_reg(RTC_SETTING_REG, ret);

    do
    {
        ret = read_rtc_inter_reg(RTC_SETTING_REG);
    }
    while(ret & (1<<bit));

    return;
	#endif
}

/**
 * @brief set rtc register value by system time
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param systime T_SYSTIME : system time structure
 * @return unsigned long day num
 */
void rtc_set_RTCbySystime(T_SYSTIME *systime)
{
    unsigned long RealTimeData1 = 0;
    unsigned long RealTimeData2 = 0;
    unsigned long RealTimeData3 = 0;
    unsigned long rtc_setting = 0;
	unsigned char month_std_day[12]  = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    unsigned char month_leap_day[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if((systime->year % 4 == 0 && systime->year % 100 != 0) || (systime->year % 400 == 0))
    {
		
	    if ((systime->year < BASE_YEAR) || (systime->year > 2099) || (systime->month < 1) || (systime->month > 12) || \
	       (systime->day < 1) || (systime->day > month_leap_day[(systime->month)-1]) || (systime->hour > 23) || (systime->minute > 59) \
	       || (systime->second > 59) || (systime->week > 7))
	    {
	        akprintf(C3, M_DRVSYS, "year = %d, month = %d, day = %d, hour = %d, minute = %d, second = %d, week = %d\n", \
	            systime->year, systime->month, systime->day, systime->hour, systime->minute, systime->second, systime->week);
	        akprintf(C3, M_DRVSYS, "it is a wrong systime format, set system time fail\n");

	        return;
	    }
	}
	else
	{
		if ((systime->year < BASE_YEAR) || (systime->year > 2099) || (systime->month < 1) || (systime->month > 12) || \
	       (systime->day < 1) || (systime->day > month_std_day[(systime->month)-1]) || (systime->hour > 23) || (systime->minute > 59) \
	       || (systime->second > 59) || (systime->week > 7))
	    {
	        akprintf(C3, M_DRVSYS, "year = %d, month = %d, day = %d, hour = %d, minute = %d, second = %d, week = %d\n", \
	            systime->year, systime->month, systime->day, systime->hour, systime->minute, systime->second, systime->week);
	        akprintf(C3, M_DRVSYS, "it is a wrong systime format, set system time fail\n");

	        return;
	    }
	}

    RTC_PROTECT;

    //check rtc works or not
    if (!test_rtc_inter_reg(0))
    {
        RTC_UNPROTECT;
        return;
    }
    
    //config Real Time Reg1
    SET_BITS(RealTimeData1, SecAddr, systime->second);
    SET_BITS(RealTimeData1, MinAddr, systime->minute);

    //config Real Time Reg2
    SET_BITS(RealTimeData2, HourAddr, systime->hour);
    SET_BITS(RealTimeData2, DayMonthAddr, systime->day);
    SET_BITS(RealTimeData2, DayWeekAddr, (systime->week+1));

    //config Real Time Reg3
    SET_BITS(RealTimeData3, MonthAddr, systime->month);
    SET_BITS(RealTimeData3, YearAddr, (systime->year-BASE_YEAR));

    //write RealTime register
    write_rtc_inter_reg(REALTIME1_REG, RealTimeData1);
    write_rtc_inter_reg(REALTIME2_REG, RealTimeData2);
    write_rtc_inter_reg(REALTIME3_REG, RealTimeData3);

    rtc_setting = read_rtc_inter_reg(RTC_SETTING_REG);
    rtc_setting |= REAL_TIME_WR;
    write_rtc_inter_reg(RTC_SETTING_REG, rtc_setting);

    do
    {
        rtc_setting = read_rtc_inter_reg(RTC_SETTING_REG);
    }
    while(rtc_setting & REAL_TIME_WR);

    RTC_UNPROTECT;
}


void rtc_set_RTCcount(unsigned long rtc_value)
{
    T_SYSTIME systime;

    memset(&systime, 0x00, sizeof(systime));
    ConvertSecondsToSysTime(rtc_value, &systime);

#ifdef RTC_DEBUG    
    akprintf(C3, M_DRVSYS, "rtc_set_RTCcount(): year = %d, month = %d, day = %d, hour = %d, minute = %d \
        second = %d, week = %d\n", systime.year, systime.month, systime.day, \
        systime.hour, systime.minute, systime.second, systime.week);
#endif

    rtc_set_RTCbySystime(&systime);

    return ;
}


/**
 * @brief set alarm by system time
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @param systime system time structure
 * @return unsigned long: day num
 */
void rtc_set_AlarmBySystime(T_SYSTIME *systime)
{
    unsigned long RealTimeData1 = 0;
    unsigned long RealTimeData2 = 0;
    unsigned long RealTimeData3 = 0;
	unsigned long val;
	unsigned char month_std_day[12]  = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    unsigned char month_leap_day[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if((systime->year % 4 == 0 && systime->year % 100 != 0) || (systime->year % 400 == 0))
    {
		
	    if ((systime->year < BASE_YEAR) || (systime->year > 2099) || (systime->month < 1) || (systime->month > 12) || \
	       (systime->day < 1) || (systime->day > month_leap_day[(systime->month)-1]) || (systime->hour > 23) || (systime->minute > 59) \
	       || (systime->second > 59) || (systime->week > 6))
	    {
	        akprintf(C3, M_DRVSYS, "year = %d, month = %d, day = %d, hour = %d, minute = %d, second = %d, week = %d\n", \
	            systime->year, systime->month, systime->day, systime->hour, systime->minute, systime->second, systime->week);
	        akprintf(C3, M_DRVSYS, "it is a wrong systime format, set system time fail\n");

	        return;
	    }
	}
	else
	{
		if ((systime->year < BASE_YEAR) || (systime->year > 2099) || (systime->month < 1) || (systime->month > 12) || \
	       (systime->day < 1) || (systime->day > month_std_day[(systime->month)-1]) || (systime->hour > 23) || (systime->minute > 59) \
	       || (systime->second > 59) || (systime->week > 6))
	    {
	        akprintf(C3, M_DRVSYS, "year = %d, month = %d, day = %d, hour = %d, minute = %d, second = %d, week = %d\n", \
	            systime->year, systime->month, systime->day, systime->hour, systime->minute, systime->second, systime->week);
	        akprintf(C3, M_DRVSYS, "it is a wrong systime format, set system time fail\n");

	        return;
	    }
	}

    RTC_PROTECT;
	//clear alarm status before set alarm time    /*important*/  
	val = read_rtc_inter_reg(7);
	val |= (1<<0);
	write_rtc_inter_reg(7, val);

    //config Alarm Time Reg1
    SET_BITS(RealTimeData1, SecAddr, systime->second);
    SET_BITS(RealTimeData1, MinAddr, systime->minute);
    RealTimeData1 |= ALARM_ENABLE;

    //config Alarm Time Reg2
    SET_BITS(RealTimeData2, HourAddr, systime->hour);
    SET_BITS(RealTimeData2, DayMonthAddr, systime->day);
    RealTimeData2 |= ALARM_ENABLE;

    //config Alarm Time Reg3
    SET_BITS(RealTimeData3, MonthAddr, systime->month);
    SET_BITS(RealTimeData3, YearAddr, systime->year - BASE_YEAR);
    RealTimeData3 |= ALARM_ENABLE;

    //write Alarm Time register
    write_rtc_inter_reg(ALARMTIME1_REG, RealTimeData1);
    write_rtc_inter_reg(ALARMTIME2_REG, RealTimeData2);
    write_rtc_inter_reg(ALARMTIME3_REG, RealTimeData3);

    RTC_UNPROTECT;
}


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
 void rtc_set_alarmcount(unsigned long rtc_wakeup_value)
{
    T_SYSTIME systime;

    memset(&systime, 0x00, sizeof(systime));
    ConvertSecondsToSysTime(rtc_wakeup_value, &systime);

#ifdef RTC_DEBUG    
    akprintf(C3, M_DRVSYS, "rtc_set_alarmcount(): year = %d, month = %d, day = %d, hour = %d, minute = %d \
        second = %d, week = %d\n", systime.year, systime.month, systime.day, \
        systime.hour, systime.minute, systime.second, systime.week);
#endif

    rtc_set_AlarmBySystime(&systime);

    return ;
}

/**
 * @brief enable or disable power down alarm
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param alarm_on [in]  enable power down alarm or not
 * @return void
 */
void rtc_set_powerdownalarm(bool alarm_on)
{
    /* four standard steps(if alarm_on)
            a. to write current time and alarm time
            b. to clear alarm status bit
            c. to enable the alarm function
            d.power down
            in this version, 'a' can be omitted
            note: should check corresponding status bits to confirm the 
                    operation has been implemented successfully
    */
    unsigned long setdata = 0;

    RTC_PROTECT;

    //read back  wakeupset_reg
    setdata = read_rtc_inter_reg(RTC_SETTING_REG);

    if (true == alarm_on)
    {
        //clear alarm status, enable alarm function, set wakeuppin active gpio
        setdata |= WAKEUP_EN;
    }
    else
    {
        //disable the alarm function
        setdata &= ~WAKEUP_EN;               
    }

    //write setdata to wakeupset_reg
    write_rtc_inter_reg(RTC_SETTING_REG, setdata);

    RTC_UNPROTECT;
    return;
}

/**
 * @brief get system time from rtc
 * @author liao_zhijun
 * @date 2010-05-28
 * @param 
 * @return T_SYSTIME SysTime: system time structure
 */
T_SYSTIME rtc_get_RTCsystime(void)
{
    unsigned long RealTimeRet1, RealTimeRet2, RealTimeRet3;
    T_SYSTIME systime;
    unsigned long rtc_setting;

    RTC_PROTECT;
    
    rtc_setting = read_rtc_inter_reg(RTC_SETTING_REG);
    rtc_setting |= REAL_TIME_RE;
    write_rtc_inter_reg(RTC_SETTING_REG, rtc_setting);

    //read RealTime register 
    RealTimeRet1 = read_rtc_inter_reg(REALTIME1_REG);
    RealTimeRet2 = read_rtc_inter_reg(REALTIME2_REG);
    RealTimeRet3 = read_rtc_inter_reg(REALTIME3_REG);

    systime = CalcSystimeByRTC(RealTimeRet1, RealTimeRet2, RealTimeRet3);

    RTC_UNPROTECT;

    return systime;
}
/**
 * @brief   get rtc passed count in seconds
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return unsigned long the rtc count
 */
unsigned long rtc_get_RTCcount( void )
{
    unsigned long rtc_count = 0;
    T_SYSTIME systime;

    systime = rtc_get_RTCsystime();
    rtc_count = ConvertTimeToSeconds(&systime);
    
    return rtc_count;
}

/**
 * @brief get system time from rtc
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @param void
 * @return T_SYSTIME system time structure
 */
T_SYSTIME rtc_get_AlarmSystime(void)
{
    unsigned long TimeRet1, TimeRet2, TimeRet3;
    T_SYSTIME systime;

    RTC_PROTECT;

    //read RealTime register 1
    TimeRet1 = read_rtc_inter_reg(ALARMTIME1_REG);
    TimeRet2 = read_rtc_inter_reg(ALARMTIME2_REG);
    TimeRet3 = read_rtc_inter_reg(ALARMTIME3_REG);
        
    systime = CalcSystimeByRTC(TimeRet1, TimeRet2, TimeRet3);

    RTC_UNPROTECT;

    return systime;
}

/**
 * @brief get alarm count that has been set.
 *
 * @author liao_zhijun
 * @date 2010-04-29
 * @return unsigned long
 * @retval the alarm count in seconds
 */
unsigned long rtc_get_alarmcount(void)
{
    unsigned long rtc_count = 0;
    T_SYSTIME systime;

    systime = rtc_get_AlarmSystime();
    rtc_count = ConvertTimeToSeconds(&systime);
    
    return rtc_count;
}

/**
 * @brief query alarm status
 * 
 * @author liao_zhijun
 * @date 2010-04-29
 * @return bool
 * @retval true alarm has occured
 * @retval false alarm hasn't occured
 */
bool rtc_get_alarm_status()
{
    return false;
}


/**
* @brief rtc get time interface for user whether have rtc or not
 * 
 * @author wumingjin
 * @date 2017-07-06
 * @return T_SYSTIME
 
**/
T_SYSTIME rtc_get_systime(void)
{
	T_SYSTIME systime;
	unsigned long sys_curr_seconds;
	if(rtc_normal_flag)
	{
		return rtc_get_RTCsystime();
	}
	else
	{	
		sys_curr_seconds = get_tick_count()/1000;
		sys_curr_seconds += g_start_seconds;
		ConvertSecondsToSysTime(sys_curr_seconds, &systime);
		return systime;
	}	
}

/**
* @brief rtc set time interface for user whether have rtc or not
 * 
 * @author wumingjin
 * @date 2017-07-06
 * @return void
 
**/
void rtc_set_systime(T_SYSTIME *systime)
{
	
	unsigned long sys_curr_seconds, seconds;
	if(rtc_normal_flag)
	{
		rtc_set_RTCbySystime(systime);
	}
	else
	{	
		seconds = ConvertTimeToSeconds(systime);
		sys_curr_seconds = get_tick_count()/1000;
		g_start_seconds = (seconds - sys_curr_seconds);
	}	
}



