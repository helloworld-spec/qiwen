/** @file anyka_bsp.h
 * @brief BSP(board support packet) file
 *
 * User must define the physical info of the board here. 
 * such as  FLASH/RAM/STACK etc
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author 
 * @date 2006-01-16
 * @version 1.0
 */
#ifndef _ANYKA_BSP_H_
#define _ANYKA_BSP_H_

/* Memory start address */
#ifdef CHIP_AK980X
#define RAM_BASE_ADDR		0x80000000	// RAM start address
#else
#ifdef CHIP_AK39XX
#define RAM_BASE_ADDR		0x80000000	// RAM start address
#else
#define RAM_BASE_ADDR		0x30000000	// RAM start address
#endif

#endif

#ifdef CHIP_37XX_37CW
#define SDRAM_SIZE			0x200000	// 8Mbyte
#else
#define SDRAM_SIZE			0x800000	// 8Mbyte
#endif

#define RAM_SIZE	SDRAM_SIZE
	
/** @} */

/* Memory distribution for MMU, TimerHISR, and each stacks 
 * --- Top of memory address----
 * |	   		mmu
 * |----------------------------
 * |	      TimerHISR(Optional)
 * |----------------------------
 * |	  Undefined mode stack
 * |----------------------------
 * |	   Abort mode stack
 * |----------------------------
 * |	    FIQ mode stack
 * |----------------------------
 * |	    IRQ mode stack
 * |----------------------------
 * |	    SVC mode stack
 * |----------------------------
 * |	   	    Heap
 * |----------------------------
 */

/* define the MMU size and start address */
#define _MMUTT_SIZE				(0x4000)
#define _MMUTT_STARTADDRESS     (RAM_BASE_ADDR + RAM_SIZE - _MMUTT_SIZE)

#define TMC_STACK_BASE          (_MMUTT_STARTADDRESS)


/* each stack size and start address */
#define IRQ_MODE_STACK_SIZE     (0x10000)
#define FIQ_MODE_STACK_SIZE     (0x10)  /* if has FIQ, enlarge it */
#define ABORT_MODE_STACK_SIZE   (0x1000)
#define UNDEF_MODE_STACK_SIZE  	(0x1000)

#define SVC_MODE_STACK			(IRQ_MODE_STACK - IRQ_MODE_STACK_SIZE)	 
#define IRQ_MODE_STACK			(FIQ_MODE_STACK - FIQ_MODE_STACK_SIZE)
#define FIQ_MODE_STACK			(ABORT_MODE_STACK - ABORT_MODE_STACK_SIZE)
#define ABORT_MODE_STACK		(UNDEF_MODE_STACK - UNDEF_MODE_STACK_SIZE)	
#define UNDEF_MODE_STACK		(TMC_STACK_BASE-8)
/** @} */

/*@}*/


#endif	// #ifndef _ANYKA_BSP_H_


