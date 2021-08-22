/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 * Sukumar Ghorai <s-ghorai@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch-ak39/anyka_cpu.h>
#include <asm/arch-ak39/anyka_types.h> 
#include <asm/errno.h>
#include <asm/arch/anyka_cpu.h>
#include <asm/arch-ak39/module_reset.h>
#include <asm/arch-ak39/ak39_timer_wdt.h>




#define UNIT_NS  21333
#define WATCH_DOG_1_SECOND_SET	(1000000000 / UNIT_NS)   // 1000000*64/3

static unsigned int def_heartbeat = 8 * WATCH_DOG_1_SECOND_SET;

/**
 * @brief enable watch_dog timer for system reset function.
 *
 * enable watch_dog timer for system reset function.
 * @author CaoDonghua
 * @date 2016-12-29
 * @param[in] void none
 * @return void none
 * @retval  none
 */
void wdt_enable(void)
{
	unsigned int cfg_val = (unsigned int)def_heartbeat;

	/* set watchdog time*/
	REG32(MODULE_WDT_CFG1) = ((0x55 << 24) | cfg_val);

	/*enable watchdog */
	REG32(MODULE_WDT_CFG2) = ((0xaa << 24) | 0x1);

	/*start watchdog */
	REG32(MODULE_WDT_CFG2) = ((0xaa << 24) | 0x3);
	
}

/**
 * @brief set watch_dog alive time.
 *
 * set watch_dog alive time, after that make a reset to system
 * @author CaoDonghua
 * @date 2016-09-30
 * @param[in] unsigned int heartbeat: watch dog keep alive time.
 * @return void none
 * @retval  none
 */
void wdt_keepalive(unsigned int heartbeat)
{
	unsigned int cfg_val = (unsigned int)heartbeat;	
	
	printf("heartbeat = %x\n", heartbeat);

	REG32(MODULE_WDT_CFG1) = ((0x55 << 24) | cfg_val);
	REG32(MODULE_WDT_CFG2) = ((0xaa << 24) | 0x1);
	REG32(MODULE_WDT_CFG2) = ((0xaa << 24) | 0x3);

}




