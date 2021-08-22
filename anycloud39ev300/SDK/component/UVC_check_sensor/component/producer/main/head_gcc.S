@;/*
@;*  head.S
@; *
@; * bios entry for M3 test boad
@; */
#include "anyka_bsp.h"
#include "boot.h"
#include "version.h"

.extern  CMain
.extern  irq_handler
.extern  fiq_handler
.extern  undefined_handler
.extern  prefetch_handler
.extern  abort_handler
.extern  swi_dispatch
.extern  intr_mask_irq_fiq
    
@.extern |Image$$ER_RO$$Base|
@.extern |Image$$ER_RO$$Limit|
@.extern |Image$$ER_ZI$$ZI$$Base|	@;; MUST bss$$ZI, not bss
@.extern |Image$$ER_ZI$$ZI$$Limit|  @;; MUST bss$$ZI, not bss
	
@    AREA BOOT, CODE, READONLY
@    CODE32
@    ENTRY
@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@;;;;;;;;;;;;;;;;;;;;;;   Exception vectors   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.global start
_start:
    b 	reset_handler		  @;Start point: Jump to first instruction: reset_handler
    b	undefined_handler	  @;Exception(Undefined Instruction): Dead Loop
    b 	swi_handler			  @;Software Interrupt: Jump to entry of SWI
    b	prefetch_handler		@;Exception(Instruction Prefetch Failure): Dead Loop
    b	abort_handler				@;Exception(Data Abort): Dead Loop
    nop								    @;Reserved
    b	irq_handler					@;Interrupt Request: Jump to entry of IRQ
    nop							      @;Fast Interrupt Request: Reserved

@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.byte    VER_CHECK0    @;ver_check0    VER_CHECK0
.byte    VER_CHECK1    @;ver_check1    VER_CHECK1
.byte    VER_CHECK2    @;ver_check2    VER_CHECK2
.byte    VER_CHECK3    @;ver_check3    VER_CHECK3
.word    MAIN_VERSION  @;main_version  MAIN_VERSION
.word    SUB1_VERSION  @;sub1_version  SUB1_VERSION
.word    SUB2_VERSION  @;sub2_version  SUB2_VERSION
.word    SUB3_VERSION  @;reserve       SUB3_VERSION

@;;;;;;;;;;;;;;;;;;;;;;   Entry of Reset   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
reset_handler:
	@;Change processor mode to IRQ Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_IRQ 
	msr		cpsr, r0
	ldr     r1, =IRQ_MODE_STACK
	mov		sp, r1

	@;Change processor mode to fiq Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_FIQ 
	msr		cpsr, r0
	ldr     r1, =FIQ_MODE_STACK
	mov		sp, r1

	@;Change processor mode to Abort Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_ABT
	msr		cpsr, r0
	ldr     r1, =ABORT_MODE_STACK
	mov		sp, r1

	@;Change processor mode to Undefied Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_UNDEF
	msr		cpsr, r0
	ldr     r1, =UNDEF_MODE_STACK
	mov		sp, r1	
	
	@;Change processor mode to User Mode, and initialize the stack pointer
	mov		r0, #ANYKA_CPU_Mode_SVC
	msr		cpsr, r0
	ldr     r1, =SVC_MODE_STACK
	mov		sp, r1


clear_bss:    
    @;clear bss
    ldr  r0, = __bss_start @;|Image$$ER_ZI$$ZI$$Base|
    ldr  r2, =0x0
    ldr  r3, =__bss_end @;|Image$$ER_ZI$$ZI$$Limit|

clear_bss_loop:
    cmp  r0, r3
    strcc r2, [r0], #4
    bcc  clear_bss_loop
    

	@;disable all interrupt
    bl      intr_mask_irq_fiq
	
	@;@ we will copy data , copy inner, and clear zi in __main
	@;@ then go to main function
	B       main_s

@;==================================================================================== 
	
	.global __user_initial_stackheap

@__user_initial_stackheap    
@    LDR   r0,= __bss_end    @;|Image$$ER_ZI$$ZI$$Limit|
@   MOV   pc,lr
	
	.global main_s

main_s:
    ldr		pc,	=CMain
    MOV   pc,lr
	
	@;end of user function, enter dead loop
reset_handler_loop:
	b		reset_handler_loop	

@;====================================================================================

@;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@;;;;;;;;;;;;;;;;;;;;;   Entry of SWI   ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

swi_handler:
    @;check SWI 0x18, ReportException SWI
    cmp		r0, #0x18
	  bne	    swi_handler_exit
	
	@;fetch number of SWI in ARM state
    ldr     r2, [lr, #-4]
    bic     r2, r2, #0xff000000

    bl      swi_dispatch
    
swi_handler_exit:							@;exit point of SWI
	movs	r15, r14

    @END