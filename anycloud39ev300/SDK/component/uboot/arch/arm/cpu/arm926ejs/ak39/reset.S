/*
 *  armboot - Startup Code for ARM926EJS CPU-core
 *
 *  Copyright (c) 2003  Texas Instruments
 *
 *  ----- Adapted for OMAP1610 OMAP730 from ARM925t code ------
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
 *  Copyright (c) 2003	Richard Woodruff <r-woodruff2@ti.com>
 *  Copyright (c) 2003	Kshitij <kshitij@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

	.align	5
.globl reset_cpu
reset_cpu:
1:
	mrc p15, 0, pc, c7, c14, 3  /*flush dcache all*/
	bne 1b
	mov r1, #0x0
	mcr p15, 0, r1, c8, c7, 0
	mcr p15, 0, r1, c7, c5, 0
	mrc p15, 0, r1, c1, c0, 0
	bic r1, r1, #0x3000
	bic r1, r1, #0x0005
	mcr p15, 0, r1, c1, c0, 0
	mov pc, r0
_loop_forever:
	b	_loop_forever

rstctl1:
	.word	0xfffece10
