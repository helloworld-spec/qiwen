!********************************************************************************************************
!                                               FreeRTOS
!
!                               (c) Copyright 2010, Andes Techonology
!                                          All Rights Reserved
!
!                                           NDS32 Generic Port
!                                            GNU C Compiler
!
! File    : os_cpu_a.S
! Version : V1.0
! By      : greentime @ Andes Technology
!
!********************************************************************************************************
!********************************************************************************************************
! INCLUDE ASSEMBLY CONSTANTS
!********************************************************************************************************

#include "n12_def.h"
#include "FreeRTOSConfig.h"
#include "os_cpu_common.h"

!********************************************************************************************************
! IMPORT and EXPORT
!********************************************************************************************************

	.global OS_Trap_Interrupt_SWI
	.global CtxSave
	.global CtxRestore
#if (NO_EXTERNAL_INT_CTL==1)
	.global OS_Trap_Int_Comm
#else	
	.global OS_Trap_Interrupt_HW1
#endif
	.global OS_Trap_Syscall
#if defined(CONFIG_OSC_SUPPORT)
	.global OS_Trap_General_Exception
#endif
	
	.macro TCBSaveMacro
		! Store the new top of stack for the task.
		la	$r3, pxCurrentTCB	! pxCurrentTCB->pxTopOfStack = $SP
		lwi	$r4, [$r3]
		swi	$sp, [$r4]
	.endm



	! Restore to new task's context
	.macro TCBRestoreMacro
		! use new pxCurrentTCB to get new stack pointer
		la	$r0, pxCurrentTCB
		lwi	$r1, [$r0]
		lwi	$sp, [$r1]		! $SP = pxCurrentTCB->pxTopOfStack
	.endm	



!*************************************************************************
!
!	PortMacro Function, Call by C function
!	Used by First start Scheduler
!	Warm up a Initalized Task
!
!*************************************************************************
!CtxSave:
!
!		mfsr	$r0, $PSW
!		mtsr	$r0, $IPSW
!		mtsr	$lp, $IPC
!		pushm $r0, $r1
!		SAVE_ALL
!		TCBSaveMacro
!		ret

	! Restore to new task's context
CtxRestore:
		TCBRestoreMacro
		RESTORE_ALL
		popm $r0, $r1
		iret



!********************************************************************************************************
!                   PERFORM A CONTEXT SWITCH - OSCtxSw()
!
! Note(s) : 1) OSIntCtxSw() is called with IRQ DISABLED
!
!           2) The pseudo-code for OSCtxSw() is:
!              a) $IPSW <- $PSW, $IPC <- $lp
!              b) CtxSaveMacro		: Save current task's context
!	       c) TCBSaveMacro		: Save current task's sp into TCB
!              d) vTaskSwitchContext	: get the highest priority of Task
!              e) IntlSwitch		: Switch to interruption level 1
!	       f) TCBRestoreMacro	: Restore to the highest priority task's sp from TCB
!              g) CtxRestoreMacro	: Restore to the highest priority task's context 
!
!********************************************************************************************************

OS_Trap_Interrupt_SWI:
	movi    $r0, 0x0
	mtsr    $r0, $INT_PEND                  ! clean SWI pending
	
	IntlSwitch #0                   	! Switch to interrupt level 0

	SAVE_ALL
	TCBSaveMacro				! Save current task's sp into TCB

	CallFn	vTaskSwitchContext		! get the highest priority of Task

	TCBRestoreMacro				! Restore to original task's TCB
	RESTORE_ALL
	popm $r0, $r1
	iret


!********************************************************************************************************
! IRQ Interrupt Service Routine
!********************************************************************************************************

#if ( NO_EXTERNAL_INT_CTL==1 )
OS_Trap_Int_Comm:

!	! Check whether or not IRQ really happened
!	mfsr	$r0, $INT_PEND2
!	beqz	$r0, 1f

	! Save interrupt vector number
	la	$r0, OSIntVectorNo
	swi	$r1, [$r0]
	CALLER_SAVE
	IntlSwitch #0                   ! Switch to interrupt level 0

	! Handle nesting counter
	la	$r0, OSIntNesting	! if (OSIntNesting == 0) {
	lbi	$r1, [$r0]
	bnez	$r1, 2f

	mfsr	$r2, $INT_MASK2		! set INT_MASK2.SWI = 0
	li      $r3, (~(0x1<<1))
	and	$r2, $r2, $r3
	mtsr	$r2, $INT_MASK2

	TCBSaveMacro
	la	$sp, __OS_Int_Stack	! }

2:
	addi    $r1, $r1, 1		! OSIntNesting++
	swi	$r1, [$r0]

Arg_r0:	
	la	$r2, OSIntVectorNo	! Load interrupt vector no. as function argument
	lwi	$r0, [$r2]

Call_ISR: 
	la	$lp, OS_CPU_Vector_Table! $lp = &OS_CPU_Vector_Table[]
	lw	$fp, [$lp+($r0<<2)]	! $fp = OS_CPU_Vector[int_no]
	jral	$fp			! call ISR
	setgie.d
	dsb

        !Check for non-nested interruption return
	la      $r0, OSIntNesting	! OSIntNesting--
	lwi	$r1, [$r0]		
	addi    $r1, $r1, -1		
	swi	$r1, [$r0]
	
	bnez    $r1, 3f

					! if (OSIntNesting == 0)
	mfsr    $r0, $INT_MASK2		! set INT_MASK2.SWI = 1
    li      $r1, (0x1 << 1)
    or	$r0, $r0, $r1
	mtsr	$r0, $INT_MASK2

	TCBRestoreMacro
3:

	CALLER_RESTORE
!1:
	popm	$r0, $r1
	iret

#else
OS_Trap_Interrupt_HW1:

	! Check whether or not IRQ really happened

!	mfsr	$r0, $INT_PEND
!	andi	$r0, $r0, #INT_PEND_mskH1I
!	beqz	$r0, 1f
	CALLER_SAVE
	IntlSwitch #0                   ! Switch to interrupt level 0


	! Handle nesting counter
	la	$r0, OSIntNesting	! if (OSIntNesting == 0) {
	lbi	$r1, [$r0]
	bnez	$r1, 2f

	mfsr	$r2, $INT_MASK		! set INT_MASK.SIM = 0
	li	$r3, (~(0x1<<16))
	and	$r2, $r2, $r3
	mtsr	$r2, $INT_MASK

	TCBSaveMacro
	la	$sp, __OS_Int_Stack	! }

2:
	addi    $r1, $r1, 1		! OSIntNesting++
	swi	$r1, [$r0]
        CallFn	OS_CPU_IRQ_ISR_Handler	! call ISR in bsp.c
	setgie.d			! GIE off
	dsb

	la      $r0, OSIntNesting	! OSIntNesting--
	lwi	$r1, [$r0]		
	addi    $r1, $r1, -1		
	swi	$r1, [$r0]

	bnez	$r1, 3f
					! if (OSIntNesting == 0)
	mfsr    $r0, $INT_MASK		! set INT_MASK.SIM = 1
	li	$r1, (0x1 <<16)
	or	$r0, $r0, $r1
	mtsr	$r0, $INT_MASK

	TCBRestoreMacro

       ! Check for non-nested interruption return
3:
	CALLER_RESTORE
1:
	popm $r0, $r1
	iret

#endif   

#if defined(CONFIG_OSC_SUPPORT)
OS_Trap_General_Exception:

        SAVE_ALL
        IntlSwitch #0                   ! Switch to interrupt level 0


        ! Handle nesting counter
        la      $a0, OSIntNesting       ! if (OSIntNesting == 0) {
        lbi     $a1, [$a0]
        bnez    $a1, 2f
        TCBSaveMacro
        la      $sp, __OS_Int_Stack     ! }

2:
        addi    $a1, $a1, 1             ! OSIntNesting++
        swi     $a1, [$a0]

        mfsr    $r0, $IPC
        dsb

        CallFn  OS_CPU_EXCEPTION_Dispatcher     ! call ISR in bsp.c
        !setgie.d                               ! GIE off
        la      $a0, OSIntNesting               ! OSIntNesting--
        lwi     $a1, [$a0]
        addi    $a1, $a1, -1
        swi     $a1, [$a0]

        bnez    $a1, 3f
        TCBRestoreMacro

       ! Check for non-nested interruption return
3:
        RESTORE_ALL

        popm $a0, $a1
        iret
#endif

!********************************************************************************************************
! POINTERS TO VARIABLES
!********************************************************************************************************
.section .bss
!	.data
	.align 3
	.skip IRQ_STACK_SIZE
__OS_Int_Stack:

!	.data
	.align 3
	.skip 4
OSIntVectorNo:

.global gOsFromISR
.global OSIntNesting
	.data
	.align 3
	.skip 4
OSIntNesting:
gOsFromISR:
	.end
