/* 
 *	mach/reset.h
 */
#ifndef _AK39_TIMER_WDT_H_
#define _AK39_TIMER_WDT_H_

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch-ak39/anyka_cpu.h>
#include <asm/arch-ak39/anyka_types.h> 

#include <asm/arch/anyka_cpu.h>

void wdt_enable(void);
void wdt_keepalive(unsigned int heartbeat);


#endif
