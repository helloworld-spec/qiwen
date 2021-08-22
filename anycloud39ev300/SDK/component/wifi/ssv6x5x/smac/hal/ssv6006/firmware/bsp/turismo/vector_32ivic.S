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
    b       OS_Init_Nds32               !  (0) Trap Reset/NMI
    VECTOR  OS_Trap_TLB_Fill            !  (1) Trap TLB fill
    VECTOR  OS_Trap_PTE_Not_Present     !  (2) Trap PTE not present
    VECTOR  OS_Trap_TLB_Misc            !  (3) Trap TLB misc
    VECTOR  OS_Trap_TLB_VLPT_Miss       !  (4) Trap TLB VLPT miss
    VECTOR  OS_Trap_Machine_Error       !  (5) Trap Machine error
    VECTOR  OS_Trap_Debug_Related       !  (6) Trap Debug related
    VECTOR  OS_Trap_General_Exception   !  (7) Trap General exception
    VECTOR  OS_Trap_Syscall             !  (8) Syscall

    VECTOR  OS_Trap_Interrupt_HW0       !  (9) Interrupt HW0
    VECTOR  OS_Trap_Interrupt_SWI       ! (10) Turismo uses INT1 for software interrupt
    VECTOR  OS_Trap_Interrupt_HW2       ! (11) Interrupt HW2
    VECTOR  OS_Trap_Interrupt_HW3       ! (12) Interrupt HW3
    VECTOR  OS_Trap_Interrupt_HW4       ! (13) Interrupt HW4
    VECTOR  OS_Trap_Interrupt_HW5       ! (14) Interrupt HW5
    VECTOR  OS_Trap_Interrupt_HW6       ! (15) Interrupt HW6
    VECTOR  OS_Trap_Interrupt_HW7       ! (16) Interrupt HW7
    VECTOR  OS_Trap_Interrupt_HW8       ! (17) Interrupt HW8
    VECTOR  OS_Trap_Interrupt_HW9       ! (18) Interrupt HW9
    VECTOR  OS_Trap_Interrupt_HW10      ! (19) Interrupt HW10
    VECTOR  OS_Trap_Interrupt_HW11      ! (20) Interrupt HW11
    VECTOR  OS_Trap_Interrupt_HW12      ! (21) Interrupt HW12
    VECTOR  OS_Trap_Interrupt_HW13      ! (22) Interrupt HW13
    VECTOR  OS_Trap_Interrupt_HW14      ! (23) Interrupt HW14
    VECTOR  OS_Trap_Interrupt_HW15      ! (24) Interrupt HW15
    VECTOR  OS_Trap_Interrupt_HW16      ! (25) Interrupt HW16
    VECTOR  OS_Trap_Interrupt_HW17      ! (26) Interrupt HW17
    VECTOR  OS_Trap_Interrupt_HW18      ! (27) Interrupt HW18
    VECTOR  OS_Trap_Interrupt_HW19      ! (28) Interrupt HW19
    VECTOR  OS_Trap_Interrupt_HW20      ! (29) Interrupt HW20
    VECTOR  OS_Trap_Interrupt_HW21      ! (30) Interrupt HW21
    VECTOR  OS_Trap_Interrupt_HW22      ! (31) Interrupt HW22
    VECTOR  OS_Trap_Interrupt_HW23      ! (32) Interrupt HW23
    VECTOR  OS_Trap_Interrupt_HW24      ! (33) Interrupt HW24
    VECTOR  OS_Trap_Interrupt_HW25      ! (34) Interrupt HW25
    VECTOR  OS_Trap_Interrupt_HW26      ! (35) Interrupt HW26
    VECTOR  OS_Trap_Interrupt_HW27      ! (36) Interrupt HW27
    VECTOR  OS_Trap_Interrupt_HW28      ! (37) Interrupt HW28
    VECTOR  OS_Trap_Interrupt_HW29      ! (38) Interrupt HW29
    VECTOR  OS_Trap_Interrupt_HW30      ! (39) Interrupt HW30
    VECTOR  OS_Trap_Interrupt_HW31      ! (40) Interrupt HW31
OS_Int_Vectors_End:

    .section .text

    ! Redirect unhandled exceptions / interrupts
OS_Exception:
die:
    b die


! Interrupt wrapper function
OS_Trap_Interrupt_HW0:
	li	$r1, 0x0
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW1:
	li	$r1, 0x01
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW2:
	li	$r1, 0x02
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW3:
	li	$r1, 0x03
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW4:
	li	$r1, 0x04
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW5:
	li	$r1, 0x05 
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW6:
	li	$r1, 0x06 
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW7:
	li	$r1, 0x07
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW8:
	li	$r1, 0x08
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW9:
	li	$r1, 0x09
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW10:
	li	$r1, 0x0a
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW11:
	li	$r1, 0x0b
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW12:
	li	$r1, 0x0c
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW13:
	li	$r1, 0x0d
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW14:
	li	$r1, 0x0e
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW15:
	li	$r1, 0x0f
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW16:
	li	$r1, 0x10
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW17:
	li	$r1, 0x11
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW18:
	li	$r1, 0x12
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW19:
	li	$r1, 0x13
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW20:
	li	$r1, 0x14
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW21:
	li	$r1, 0x15
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW22:
	li	$r1, 0x16
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW23:
	li	$r1, 0x17
	j	OS_Trap_Int_Comm

OS_Trap_Interrupt_HW24:
	li	$r1, 0x18
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW25:
	li	$r1, 0x19
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW26:
	li	$r1, 0x1a
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW27:
	li	$r1, 0x1b
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW28:
	li	$r1, 0x1c
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW29:
	li	$r1, 0x1d
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW30:
	li	$r1, 0x1e
	j	OS_Trap_Int_Comm
OS_Trap_Interrupt_HW31:
	li	$r1, 0x1f
	j	OS_Trap_Int_Comm


.global firmware_checksum
.global firmware_block_count

	.data
	.align 3
	.skip 4
firmware_checksum:

	.data
	.align 3
	.skip 4
firmware_block_count:

