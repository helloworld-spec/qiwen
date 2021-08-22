/**
* @FILENAME rtc.c
* @BRIEF    rtc driver
* Copyright (C) 2011 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR   lixiaoping
* @DATE     2011-2-15
* @VERSION  1.0
* @REF      Please refer to¡­
* @NOTE     
*/

#include "anyka_types.h"
#include "anyka_cpu.h"

#include "rtc.h"

#define RTC_WAIT_TIME_OUT			100000

/**
 * @brief: RTC initialization
 */
T_VOID rtc_init(T_VOID)
{
	REG32(RTC_CONFIG_REG) |= (1<<24);
}

T_VOID rtc_wait_ready(T_VOID)
{
	while (!(REG32(INT_SYS_MODULE_REG) & INT_STATUS_RTC_READY_BIT))
		;
}

T_VOID rtc_ready_irq_enable(T_VOID)
{
	REG32(INT_SYS_MODULE_REG) |= (1<<8);

	rtc_wait_ready();

	REG32(RTC_CONFIG_REG) |= (1<<25);
}

T_VOID rtc_ready_irq_disable(T_VOID)
{
	REG32(RTC_CONFIG_REG) &= ~(1<<25);
	
	REG32(INT_SYS_MODULE_REG) &= ~(1<<8);
}


T_U32 rtc_read(T_U32 reg)
{
	T_U32 value;

	rtc_ready_irq_enable();

	REG32(RTC_CONFIG_REG) &= ~(0x3FFFFF);
	REG32(RTC_CONFIG_REG) |= (1 << 21) | (2 << 18) | (1 << 17) | (reg << 14);

	rtc_wait_ready();

	value = REG32(RTC_BACK_DAT_REG) & 0x3FFF;

	rtc_ready_irq_disable();

	return value;
}

T_VOID rtc_write(T_U32 reg, T_U32 value)
{
	rtc_ready_irq_enable();

	REG32(RTC_CONFIG_REG) &= ~(0x3FFFFF);
	REG32(RTC_CONFIG_REG) |= (1 << 21) | (2 << 18) | (0 << 17) | (reg << 14) | value;

	rtc_wait_ready();

	rtc_ready_irq_disable();
}

T_VOID rtc_set_wpin(T_BOOL level)
{
	T_U32 value;
	T_U32 bit;

	REG32(RTC_CONFIG_REG) |= (1<<24);

	bit = level ? 8 : 9;

	value = rtc_read(RTC_SETTING_REG);
	value |= (1 << bit);
	rtc_write(RTC_SETTING_REG, value);

	while (rtc_read(RTC_SETTING_REG) & (1 << bit))
		;
}

T_BOOL test_rtc_inter_reg(T_U8 reg)
{
    T_U32 status;
    T_U32 ConfigValue = 0;
    T_U32 timeout = 0;
    T_BOOL ret=AK_TRUE;

	// unmask rtc_ready irq
	REG32(INT_SYS_MODULE_REG) |= (1<<8);
    
    while (!(REG32(INT_SYS_MODULE_REG) & INT_STATUS_RTC_READY_BIT)) {
        timeout += 1;
        if (timeout >= RTC_WAIT_TIME_OUT) {
			ret = AK_FALSE;
			break;
		}
    } 

	// mask rtc_ready irq
    REG32(INT_SYS_MODULE_REG) &= ~(1<<8);
    
	return ret;
}

T_VOID sys_power_on(T_VOID)
{
	rtc_init();		/* Initialize RTC block */
	rtc_set_wpin(1);	/* Set WAKEUP Power Pin to high to maintain system power */
}
