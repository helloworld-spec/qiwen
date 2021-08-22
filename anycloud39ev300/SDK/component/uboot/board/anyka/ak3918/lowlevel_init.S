/*
 * Board specific setup info
 *
 * (C) Copyright 2003, ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>


_TEXT_BASE:
	.word CONFIG_SYS_TEXT_BASE 	/* sdram load addr from config.mk */

.globl lowlevel_init
lowlevel_init:
	mov	r11, lr

/*
	ldr   r1,  =0x018213C8      @ CPU=400M, ASIC=100M 
	ldr   r0,  =0x08000008
	str   r1,  [r0]

	ldr   r0,  =0x08000004      @set bit28 valid
	ldr   r1,  [r0]
	mov   r0,  #1
	orr   r1,  r1, r0, lsl #28
	str   r1,  [r0]
looppll:
	ldr   r0,  =0x08000004
	ldr   r1,  [r0]
	mov   r0,  #1
	and   r1,  r1, r0, lsl #28
	cmp   r1,  #0
	bne   looppll
*/

	/*enable the clock: cdh check ok*/
    ldr		r1,=0x0
    ldr     r0,=0x0800001c
    str     r1,[r0]

	/*release the reset: cdh check ok*/
    ldr		r1,=0x0
    ldr     r0,=0x08000020
    str     r1,[r0]	

	/* back to arch calling code */
	mov	lr, r11
	mov pc, lr


/* the literal pools origin */
.ltorg


