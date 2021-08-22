/**
 * @file watchdog.c
 * @brief watchdog source file
 * Copyright (C) 2004 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Jian_kui
 * @date 2016-09-05
 * @version 1.0
 */

#include "anyka_cpu.h"
#include "anyka_types.h"
#include "arch_watchdog.h"


/**
 * @brief watch dog function init
 * @author Jian_Kui
 * @date 2016-09-05
 * @param unsigned short feedtime:watch dog feed time, feedtime unit:ms, MAX = 304722ms
 * @return void
  */
void watchdog_timer_start(unsigned long feed_time)
{
    unsigned long time_num;
	
	/*calculate feed time num*/
	time_num = feed_time*46;
    irq_mask();
	
    /* set watchdog time*/
    REG32(WATCHDOG_TIME_REG) = (0x55UL << 24) | (0xFFFFFF & time_num);
	/*enable watchdog */
    REG32(WATCHDOG_CTRL_REG) = (0xAAUL << 24) | 0x1;
  	/*start watchdog */
    REG32(WATCHDOG_CTRL_REG) = (0xAAUL << 24) | 0x3;

    irq_unmask();
}

/**
 * @brief watch dog feed
 * @author Jian_Kui
 * @date 2016-09-05
 * @return void
  */
void watchdog_timer_feed(void)
{
    irq_mask();
    REG32(WATCHDOG_CTRL_REG) = (0xAAUL << 24) | 0x3;
    irq_unmask();
}

/**
 * @brief watch dog stop
 * @author Jian_kui
 * @date 2016-09-05
 * @return void
  */
void watchdog_timer_stop(void)
{
    irq_mask();
    
    REG32(WATCHDOG_CTRL_REG) = (0xAAUL << 24);
	REG32(WATCHDOG_CTRL_REG) &= ~(0x1UL<<0);

    irq_unmask();
}

