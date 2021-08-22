!********************************************************************************************************
!
!                               (c) Copyright 2005-2014, Andes Techonology
!                                          All Rights Reserved
!
!                                           NDS32 Generic Port
!                                            GNU C Compiler
!
!********************************************************************************************************

!********************************************************************************************************
! INCLUDE ASSEMBLY CONSTANTS
!********************************************************************************************************

#include <nds32_init.inc>
#include "n12_def.h"
		
	.global main
	.global __pre_c_init
	.global __post_c_init
	.global OS_Init_Nds32
	.global OS_Int_Vectors
	.global OS_Int_Vectors_End

	.macro CallFn fn
		push	$lp
		addi    $sp, $sp, -4 	! keep 8 byte align
		bal	\fn
		addi    $sp, $sp, 4
		pop	$lp
	.endm



!******************************************************************************************************
! 	Start Entry
!******************************************************************************************************
	.section .text
	.global _start
 OS_Init_Nds32:
	_start:
	!************************** Begin of do-not-modify **************************
	! Please don't modify this code
	! Initialize the registers used by the compiler

	nds32_init	! NDS32 startup initial macro in <nds32_init.inc>

	!*************************** End of do-not-modify ***************************

#ifdef XIP_MODE	
	bal _nds32_init_mem 
#endif
#ifdef CONFIG_OSC_SUPPORT
	bal	_osc_init
#endif
	bal	__pre_c_init
	bal	__post_c_init
	bal	main


!********************************************************************************************************
!                   __pre_c_init
!
! Note(s) : 1) __pre_c_init init SP pointer, cache,
!
!********************************************************************************************************
	.align  2
	.type   __pre_c_init, @function
__pre_c_init:

	! Processor should be in Super User mode
	! Disable interrupts, change to interruption level 0

	mfsr	$r0, $PSW
	li	$r1, #~PSW_MSK
	and	$r0, $r0, $r1
	li	$r1, #PSW_INIT
	or	$r0, $r0, $r1
	mtsr	$r0, $PSW
	isb

	mfsr	$r0, $INT_MASK
	li	$r1, #~INT_MASK_MSK
	and	$r0, $r0, $r1
	li	$r1, #INT_MASK_INIT
	or	$r0, $r0, $r1
	mtsr	$r0, $INT_MASK

#if(NO_EXTERNAL_INT_CTL==1)
	li	$r0, (0x1 << 9)
!	li	$r0, #IVIC_MSK
	mtsr	$r0, $INT_MASK2

	#if 0
		li	$r0, #INT_PRIO_MASK1
		mtsr	$r0, $INT_PRI
		li	$r0, #INT_PRIO_MASK2
		mtsr	$r0, $INT_PRI2
		li	$r0, 0
		mtsr    $r0, $INT_CTRL
	#endif
#endif

	mfsr	$r0, $IVB
	li	$r1, #~IVB_MSK
	and	$r0, $r0, $r1
	li	$r1, #IVB_INIT
	or	$r0, $r0, $r1
	mtsr	$r0, $IVB

#if (defined(CONFIG_CPU_ICACHE_ENABLE) || defined(CONFIG_CPU_DCACHE_ENABLE))
	mfsr	$r0, $MMU_CTL
	li	$r1, #~MMU_CTL_MSK
	and	$r0, $r0, $r1
	li	$r1, #MMU_CTL_INIT
	or	$r0, $r0, $r1
	mtsr	$r0, $MMU_CTL

	! check $CACHE_CTL existing or not
	mfsr	$r0, $ICM_CFG		! $r0 = ICM_CFG
	mfsr	$r1, $DCM_CFG		! $r1 = DCM_CFG
	or	$r0, $r0, $r1		! $r0 = $r0 | $r1
	andi	$r0, $r0, 0x1c0		! $r0 = $r0 & 0x1c0 , take (DSZ | ISZ)
	beqz	$r0, 3f

	mfsr	$r0, $CACHE_CTL
	li	$r1, #~CACHE_CTL_MSK
	and	$r0, $r0, $r1
	li	$r1, #CACHE_CTL_INIT
	or	$r0, $r0, $r1
	mtsr	$r0, $CACHE_CTL
#endif

3:
#if defined(__NDS32_EXT_FPU_DP__) || defined(__NDS32_EXT_FPU_SP__)
#if 0
// We don't need to do FPU initial anymore, since it is done in nds32_init macro.
	! check fpu existing or not and enable if existing
	li	$r1, #0x80000001
	mfsr	$r2, $FUCOP_EXIST
	and	$r2, $r2, $r1
	bne	$r1, $r2, 4f
	mfsr	$r0, $FUCOP_CTL
	ori	$r0, $r0, #0x1
	mtsr	$r0, $FUCOP_CTL
	dsb

	! Enable denormalized flush-to-Zero mode
	fmfcsr	$r0
	ori	$r0, $r0, #0x1000
	fmtcsr	$r0
	dsb
#endif
4:
#endif

	! Set-up the stack pointer before any function call
	la	$sp, _stack
	! Execute HAL initiaitization
	CallFn	HAL_pre_init

	! The C runtime initialization could have been done
	! or will be done somewhere else. But do it again
	! does not hurt.

	! We do this on a word basis.
	! Currently, the default linker script guarantee
	! the __bss_start/_end boundary word-aligned.

	! Clear bss
	la      $r0, __bss_start
	la      $r1, _end
        sub     $r2, $r1, $r0		! $r2: Size of .bss
	beqz    $r2, clear_end

        andi    $r7, $r2, 0x1f		! $r7 = $r2 mod 32
        movi    $r3, 0
        movi    $r4, 0
        movi    $r5, 0
        movi    $r6, 0
        movi    $r8, 0
        movi    $r9, 0
        movi    $r10, 0
        beqz    $r7, clear_loop         !if $r7 == 0, bss_size%32 == 0
        sub     $r2, $r2, $r7

first_clear:
	swi.bi  $r3, [$r0], #4		!clear each word
        addi    $r7, $r7, -4
        bnez    $r7, first_clear
        li      $r1, 0xffffffe0
        and     $r2, $r2, $r1           !check bss_size/32 == 0 or not
        beqz    $r2, clear_end          !if bss_size/32 == 0 , needless to clear

clear_loop:
        smw.bim $r3, [$r0], $r10        !clear each 8 words
        addi    $r2, $r2, -32
        bgez    $r2, clear_loop

clear_end:

	ret
	.size   __pre_c_init, .-__pre_c_init

	.align  2
	.type   __post_c_init, @function
__post_c_init:


#if (defined(CONFIG_CPU_ICACHE_ENABLE) || defined(CONFIG_CPU_DCACHE_ENABLE))
	! check $CACHE_CTL existing or not
	mfsr	$r0, $ICM_CFG		! $r0 = ICM_CFG
	mfsr	$r1, $DCM_CFG		! $r1 = DCM_CFG
	or	$r0, $r0, $r1		! $r0 = $r0 | $r1
	andi	$r0, $r0, 0x1c0		! $r0 = $r0 & 0x1c0 , take (DSZ | ISZ)
	beqz	$r0, 1f

	mfsr	$r0, $MMU_CTL
	li	$r1, #~MMU_CTL_MSK
	and	$r0, $r0, $r1
	li	$r1, #MMU_CTL_INIT_CACHE_ON
	or	$r0, $r0, $r1
	mtsr	$r0, $MMU_CTL

	mfsr	$r0, $CACHE_CTL
	li	$r1, #~CACHE_CTL_MSK
	and	$r0, $r0, $r1
	li	$r0, #CACHE_CTL_INIT

#ifdef CONFIG_CPU_ICACHE_ENABLE
	push	$r0
	CallFn	n12_icache_flush;
	pop	$r0
	li	$r1, (0x1 << CACHE_CTL_offIC_EN)
	or	$r0, $r0, $r1
#endif

#ifdef CONFIG_CPU_DCACHE_ENABLE
	push	$r0
	CallFn	n12_dcache_invalidate;
	pop	$r0

	li	$r1, (0x1 << CACHE_CTL_offDC_EN)
	or	$r0, $r0, $r1
#endif

	mtsr	$r0, $CACHE_CTL
#endif
1:
	! Perform BSP initialization
	CallFn	BSP_Init
	ret
	.size   __post_c_init, .-__post_c_init

	.align	2
	.type	main, @function
main:
	! Call APP_Init(fisrt_available_memory), and never go back

	la	$r0, _end
	CallFn	APP_Init

	! Infinite loop, if returned from APP_Init accidently
1:
	b	1b
	.size   main, .-main

	.end

