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

#include "n12_def.h"

		
	.global OS_Int_Vectors
	.global OS_Int_Vectors_End

        .macro WEAK_DEFAULT weak_sym, default_handler
		.weak \weak_sym
		.set \weak_sym ,\default_handler
        .endm

	WEAK_DEFAULT OS_Trap_TLB_Fill,		OS_Exception
	WEAK_DEFAULT OS_Trap_PTE_Not_Present,	OS_Exception
	WEAK_DEFAULT OS_Trap_TLB_Misc,		OS_Exception
	WEAK_DEFAULT OS_Trap_TLB_VLPT_Miss,	OS_Exception
	WEAK_DEFAULT OS_Trap_Machine_Error,	OS_Exception
	WEAK_DEFAULT OS_Trap_Debug_Related,	OS_Exception
	WEAK_DEFAULT OS_Trap_General_Exception,	OS_Exception
	WEAK_DEFAULT OS_Trap_Syscall,		OS_Exception

	WEAK_DEFAULT OS_Trap_Interrupt_SWI,	OS_Exception

! Define standard NDS32 vector table entry point of interruption vectors
	.macro VECTOR handler
	.align 4
		pushm	$r0, $r1
		sethi	$r0, hi20(\handler)
		ori	$r0, $r0, lo12(\handler)
		jral	$r0, $r0
	.endm



!********************************************************************************************************
! 	Interrupt Entry Table
!********************************************************************************************************

	.section .nds32_init, "ax"

OS_Int_Vectors:
	b	OS_Init_Nds32			!  (0) Trap Reset/NMI
	VECTOR	OS_Trap_TLB_Fill		!  (1) Trap TLB fill
	VECTOR	OS_Trap_PTE_Not_Present		!  (2) Trap PTE not present
	VECTOR	OS_Trap_TLB_Misc		!  (3) Trap TLB misc
	VECTOR	OS_Trap_TLB_VLPT_Miss		!  (4) Trap TLB VLPT miss
	VECTOR	OS_Trap_Machine_Error		!  (5) Trap Machine error
	VECTOR	OS_Trap_Debug_Related		!  (6) Trap Debug related
	VECTOR	OS_Trap_General_Exception	!  (7) Trap General exception
	VECTOR	OS_Trap_Syscall			!  (8) Syscall

        VECTOR  OS_Trap_Interrupt_HW0           !  (9) Interrupt HW0   
        VECTOR  OS_Trap_Interrupt_HW1           ! (10) Interrupt HW1
        VECTOR  OS_Trap_Interrupt_HW2           ! (11) Interrupt HW2
        VECTOR  OS_Trap_Interrupt_HW3           ! (12) Interrupt HW3
        VECTOR  OS_Trap_Interrupt_HW4           ! (13) Interrupt HW4
        VECTOR  OS_Trap_Interrupt_HW5           ! (14) Interrupt HW5
        VECTOR  OS_Trap_Interrupt_SWI           ! (15) Interrupt SWI/HW6    
OS_Int_Vectors_End:


	.section .text

	! Redirect unhandled exceptions / interrupts
OS_Exception:
	b OS_Exception


! Interrupt wrapper function
OS_Trap_Interrupt_HW0:
	li	$r1, 0x0
	j	OS_Trap_Interrupt_HW0
!OS_Trap_Interrupt_HW1:
!	li	$r1, 0x01
!	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW2:
	li	$r1, 0x02
	j	OS_Trap_Interrupt_HW2
OS_Trap_Interrupt_HW3:
	li	$r1, 0x03
	j	OS_Trap_Interrupt_HW3
OS_Trap_Interrupt_HW4:
	li	$r1, 0x04
	j	OS_Trap_Interrupt_HW4
OS_Trap_Interrupt_HW5:
	li	$r1, 0x05 
	j	OS_Trap_Interrupt_HW5
OS_Trap_Interrupt_HW6:
	li	$r1, 0x06 
	j	OS_Trap_Interrupt_HW6

