@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@	Avalent Technologes, Inc. 			02. 07. 2001
@@@
@@@	File Name :	TCT.S	
@@@ 	Component :	TM - Thread Control
@@@
@@@	Description :
@@@	This file contains the target processor dependent routines for performing taget- 
@@@	dependent scheduling functions.
@@@
@@@	Author : Yean-sun Park, Avalent Technologes, Inc.
@@@
@@@	Data Structure	: None
@@@	Function :
@@@
@@@		TCT_Control_Interrupts		Enable / disable interrupts
@@@						by changing.TCD_Interrupt_Level.
@@@		TCT_Local_Control_Interrupts	Enable/disable interrupts
@@@						by not changing.TCD_Interrupt_Lvel.
@@@		TCT_Restore_Interrupts		restores interrupts to the
@@@						level in TCD_Interrupt_Level.
@@@		TCT_Build_Task_Stack		Build initial task stack.
@@@		TCT_Build_HISR_Stack		Build initial HISR stack.
@@@		TCT_Build_Signal_Frame		Build signal handler frame.
@@@		TCT_Check_Stack			Check current stack.
@@@		TCT_Schedule 			Schedule the next thread.
@@@		TCT_Control_To_Thread           Transfer control to a thread.
@@@		TCT_Control_To_System           Transfer control from thread.
@@@		TCT_Signal_Exit                 Exit from signal handler.
@@@		TCT_Current_Thread              Returns a pointer to current thread.
@@@		TCT_Set_Execute_Task            Sets TCD_Execute_Task under  
@@@						protection from interrupts 
@@@		TCT_Protect                     Protect critical section     
@@@		TCT_Unprotect                   Unprotect critical section   
@@@		TCT_Unprotect_Specific          Release specific protection  
@@@		TCT_Set_Current_Protect         Set the thread's current    
@@@						protection field.
@@@		CT_Protect_Switch		Switch to protected thread.
@@@		TCT_Schedule_Protected		Schedule the protected thread	
@@@ 		TCT_Interrupt_Context_Save      Save interrupted context
@@@		TCT_Interrupt_Context_Restore   Restore interrupted context
@@@		TCT_Activate_HISR               Activate a HISR.
@@@		TCT_HISR_Shell                  HISR execution shell.
@@@
@@@	Depenencies :
@@@
@@@		cs_extr.h			Common Service functions.
@@@		tc_extr.h			Thread Control functions.
@@@
@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@		.TEXT

@@@.include	"cs.extr.h"
@@@.include	"tc_extr.h"

@	"boot.h"
#include "boot.h"

	.extern		TCD_Execute_Task
	.extern		TCD_Execute_HISR
	.extern		TCD_Current_Thread
	.extern		TCD_System_Stack
	.extern		TCD_Interrupt_Count
	.extern     	TCD_Active_HISR_Heads
	.extern		TCD_Active_HISR_Tails
	.extern		TCD_Interrupt_Level
	.extern		TMD_Time_Slice
	.extern		TMD_Time_Slice_State


@@@	VOID	*TCT_System_Limit

@@@  Define internal variables so the C compiler can provide meaningful 
@@@	code with offsets into data structures.  Typically, this section is
@@@	removed after this file is compiled down to assembly language.  
   
@@@	BYTE_PTR        REG_Stack_Base@
@@@	BYTE_PTR        REG_Stack_End@
@@@	BYTE_PTR        REG_Stack_Ptr@
@@@	UNSIGNED        REG_Stack_Size@
@@@	TC_TCB         *REG_Thread_Ptr@
@@@	TC_HCB         *REG_HISR_Ptr@
@@@	TC_PROTECT     *REG_Protect_Ptr@
@@@	VOID           *REG_Function_Ptr@


@EXPORT		TCT_Schedule_Protected
@@@ Define external function references.

@	AREA   INITAK, CODE, READONLY      @ name this block of code
.TEXT
	 @ENTRY
@.globl _start

@_start:


	 		  
@CODE16 EQU 0
@CODE32 EQU 1
@.global CODE_16
@.global CODE_32
        @GBLL    CODE_16
        @GBLL    CODE_32

@.equ CODE_16 ,    0 @{FALSE}
@.equ CODE_32 ,    1  @{TRUE}



	.extern		TCC_Task_Shell
	.extern		TCC_Signal_Shell
	.extern		ERC_System_Error
 	.extern		TCT_Schedule_exit_time
 
@HISR_Shell
@       DCD     TCT_HISR_Shell
@Signal_Shell
@		DCD		TCC_Signal_Shell
@Task_Shell
@        DCD     TCC_Task_Shell

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@	TCT_Control_Interrupts :
@@@	This function enables and disables interrupts as 
@@@	specified by the caller. Interrupts disabled by this left
@@@	disabled until the another call is made to enable them.
@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global			TCT_Control_Interrupts

TCT_Control_Interrupts:

@@@ INT	old_level@
@@@ Lock out all interrupts before any checking or change
@@@ Obtain the current interrupt lockout posture
@@@ old_level = TCD_Interrupt_Level
@@@ Setup new interrupt lockout posture
@@@ TCD_Interrupt_Level = new_level
@@@ renable interrupts of the specified lockout
@@@ Return old interrupt lockout level. return(old_level)
	
	MRS	R2, cpsr
	ORR	R2, R2, #LOCKOUT
	MSR	CPSR_cxsf, R2  
	LDR	R1, =TCD_Interrupt_Level
	LDR	R3, [R1, #0]
	BIC	R2, R2, #LOCK_MSK		@@@ Clear lockout mask
	ORR	R2, R2, R0				@@@ Build new CPSR with appropriate
								@@@ interrupts locked out
	STR	R0, [R1, #0]			@@@ Save current lockout
	MSR	cpsr_cxsf, R2  			@@@ Setup new CPSR lockout bits
	AND	R0, R3, #LOCK_MSK		@@@ Return previous lockout (SPR0252)

	BX	LR
		
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@	TCT_Local_Control_Interrupts: 
@@@	 This function enables and disables interrupts
@@@	 as specified by the caller.
@@@ 
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global	TCT_Local_Control_Interrupts

TCT_Local_Control_Interrupts:

@@@ read in the old level.( old_level = current interrupt level of processor).
	
	MRS	R2, cpsr				@@@ Pickup current CPSR
	MOV	R3, R2					@@@ save the old level

@@@ clear out the old level and set the new level
@@@ current interrupt level of processor &= ~LOCKOUT
@@@ current interrupt level of processor |= new_level

	BIC	R2, R2, #LOCK_MSK		@@@ Clear all current interrupts
	ORR	R2, R2, R0				@@@ Build new CPSR with new interrupt level
	MSR	cpsr_cxsf, R2 			@@@ Setup new CPSR interrupt bits
		
@@@ Return old interrupt lockout level. (return(old_level))

	AND	R0, R3, #LOCK_MSK	@@@ Return previous lockout (SPR0252)

	BX	 LR
	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@ 	TCT_Restore_Interrupts:
@@@	 	 This function restores interrupts to that specified
@@@  		 in the TCD_Interrupt_Level variable.
@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 

.global	TCT_Restore_Interrupts

TCT_Restore_Interrupts:

@@@ lock out all interrupts before any checking or changing
@@@ obtain the current interrupt lockout posture
@@@ reload the level base on the TCD_Interrupt_Level variable

	MRS	R1, cpsr				@@@ Pickup current CPSR
	MOV	R2, R1					@@@ save the CPSR value
	ORR	R1, R1, #LOCKOUT
	MSR	cpsr_cxsf, R1  			@@@ Lockout interrupt temporarily

	BIC	R2, R2, #LOCK_MSK		@@@ Clear current interrupt levels
	LDR	R1, =TCD_Interrupt_Level
	LDR	R0, [R1, #0]			@@@ Pickup current interrupt
	ORR	R2, R2, R0				@@@ Build new CPSR with appropriate
		
	MSR	cpsr_cxsf, R2  			@@@ Setup new CPSR lockout bits

	BX	LR

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@ 	TCT_Build_Task_Stack:
@@@	  This function builds an initial stack frame for a task.
@@@	  The initial stack contains information conerning initial
@@@	  values of registers and the task's point of entry. Furth-
@@@	  ermore, the initial stack frame is in the same form as an
@@@	  interrupt stackframe.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 
.global		TCT_Build_Task_Stack

TCT_Build_Task_Stack:

@@@ Pickup the stack base ( REG_Stack_Base = (BYTE_PTR) task -> tc_stack_start

	LDR	R2, [R0, #0x24]
	
@@@ Pickup the stack size ( REG_Stack_Size = Task -> tc_stack_size

	LDR	R1, [R0, #0x30]

@@@ Calculate the stack ending address.
@@@( REG_Stack_End = REG_Stack_Base + REG_Stack_Size - 1 )

	ADD	R3, R1, R2				@@@ Compute the beginning of stack
	BIC	R3, R3, #3				@@@ Insure word alignment Reserve aword
	SUB	R3, R3, #4

@@@ Save the stack ending address. ( task -> tc_stack_end = REG_Stack_End )

	STR	R3, [R0, #0x28]			@@@ Save the stack ending address

@@@ 	Reference the task shell.
@@@	REG_Function_Ptr =  (VOID *) TCC_Task_Shell@
@@@	Build an initial stack.  This initial stack frame facilitates an 
@@@	interrupt return to the TCC_Task_Shell function, which in turn 
@@@	invokes the application task.  The initial stack frame has the
@@@	following format:
@@@  				(Lower Address) Stack Top -> 1       Interrupt stack type)
@@@                                               CPSR    Saved CPSR
@@@                                               R0      Saved R0
@@@                                               R1      Saved R1
@@@                                               R2      Saved R2
@@@                                               R3      Saved R3
@@@                                               R4      Saved R4
@@@                                               R5      Saved R5
@@@                                               R6      Saved R6
@@@                                               R7      Saved R7
@@@                                               R8      Saved R8
@@@                                               R9/sb   Saved R9/sl
@@@                                               R10/sl  Saved R10/sl
@@@                                               fp      Saved fp
@@@                                               ip      Saved ip
@@@                                               sp      Saved sp
@@@                                               lr      Saved lr
@@@               (Higher Address) Stack Bottom-> pc      Saved pc
@@@

	LDR	R2, =TCC_Task_Shell

	@[ CODE_16 	

	@BIC	R2, R2, #1				@@@ Clear low bit

	@]

	STR	R2, [R3], #-4			@@@ Store entry address on stack
	MOV	R2, #0
	STR	R2, [R3], #-4			@@@ Store initial LR
	ADD	R2, R3, #0x8			@@@ Compute initial SP
	STR	R2, [R3], #-4			@@@ Store initial SP(Stack Bottom)
	STR	R2, [R3], #-4			@@@ Sotre initial ip
	STR	R2, [R3], #-4			@@@ Store initial fp
	LDR	R2, [R0, #0x24]			@@@ Pickup the stack starting address
	STR	R2, [R3], #-4			@@@ Store initial R10/S1		
	MOV	R2, #0					@@@ Clear value for initial registers
	STR	R2, [R3], #-4			@@@ Store initial R9/sb
	STR	R2, [R3], #-4			@@@ Store initial R8
	STR	R2, [R3], #-4			@@@  	"		 R7
	STR	R2, [R3], #-4			@@@		"		 R6
	STR	R2, [R3], #-4			@@@		"		 R5
	STR	R2, [R3], #-4			@@@		"		 R4
	STR	R2, [R3], #-4			@@@  	"		 R3
	STR	R2, [R3], #-4			@@@		"		 R2
	STR	R2, [R3], #-4			@@@		"		 R1
	STR	R2, [R3], #-4			@@@		"		 R0
	MSR	CPSR_f, R2				@@@ Clear the flags
	MRS	R2, cpsr   				@@@ Pickup the CPSR
	BIC	R2, R2, #LOCK_MSK		@@@ Clear initial interrupt lockout

	@[ CODE_16

	@ORR	R2, R2, #0x20			@@@ Set to THUMB state

	@]

	STR	R2, [R3], #-4			@@@ Store CPSR on the initial stack
	MOV	R2, #1					@@@ Build interrupt stack type
	STR	R2, [R3, #0]			@@@ Store stack type on the top

@@@ Save the minimum amount of remaining stack memory
@@@ task -> tc_stack_minimum = REG_Stack_Size - 72@

	MOV	R2, #72					@@@ Size of interrupt stack
	SUB	R1, R1, R2				@@@ Compute minimum available bytes
	STR	R1, [R0, #0x34]			@@@ Save in minimum stack area

@@@ Save the new stack pointer into the task's control block.
@@@ task -> tc_stack_pointer = (VOID *) Stack_Top

	STR	R3, [R0, #0x2c]			@@@ Save stack pointer
	
	BX	LR

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@ 	TCT_Build_HISR_Stack: 
@@@	Called by: TCC_create_HISR
@@@	 	This function builds an HISR stack frame that allows
@@@		quick scheduling of the HISR.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 

.global		TCT_Build_HISR_Stack

TCT_Build_HISR_Stack:

@@@ Pickup the stack base.
@@@ REG_Stack_Base = (BYTE_PTR) hisr -> tc _stack_start

	LDR	R2, [R0, #0x24]			@@@ Pickup the stack starting address
								@@@ REG_Stack_Size = hisr -> tc_stack_size
	LDR	R1, [R0, #0x30]			@@@ Pickup the atack size in bytes

@@@ Calculate the stack ending address.
@@@ REG_Stack_End = REG_Stack_Base + REG_Stack_Size

	ADD	R3, R1, R2				@@@ Compute the begining of stack
	BIC	R3, R3, #3				@@@ Insure word alignment
	SUB	R3, R3, #4				@@@ Reserve a word	

@@@	Save the stack ending address.
@@@	hisr -> tc_stack_end = REG_Stack_End

	STR	R3, [R0, #0x28]			@@@ Save the stack ending addess
	
@@@	Reference the HISR shell.  
@@@	REG_Function_Ptr =  (VOID *) TCT_HISR_Shell@
@@@
@@@	Build an initial stack.  This initial stack frame facilitates an 
@@@	solicited return to the TCT_HISR_Shell function, which in turn 
@@@	invokes the appropriate HISR.  The initial HISR stack frame has the 
@@@	following format:
@@@               (Lower Address) Stack Top ->    0       (Solicited stack type)
@@@                !!FOR THUMB ONLY!!             0/0x20  Saved state mask
@@@                                               R4      Saved R4
@@@                                               R5      Saved R5
@@@                                               R6      Saved R6
@@@                                               R7      Saved R7
@@@                                               R8      Saved R8
@@@                                               R9/sb   Saved R9/sl
@@@                                               R10/sl   Saved R10/sl
@@@                                               fp      Saved fp
@@@                                               ip      Saved ip
@@@               (Higher Address) Stack Bottom-> pc      Saved pc
@@@

	LDR	R2, =TCT_HISR_Shell			@@@ Pickup address of shell entry.
	STR	R2, [R3], #-4			@@@ Store entry address on stack
	ADD	R2, R3, #0x4			@@@ Compute initial SP
	STR	R2, [R3], #-4			@@@ Sotre initial ip
	STR	R2, [R3], #-4			@@@ Store initial fp
	LDR	R2, [R0, #0x24]			@@@ Pickup the stack starting address
	STR	R2, [R3], #-4			@@@ Store initial R10/S1
	MOV	R2, #0					@@@ Clear value for initial registers
	STR	R2, [R3], #-4			@@@ Store initial R9/sb
	STR	R2, [R3], #-4			@@@ Store initial R8
	STR	R2, [R3], #-4			@@@  	"		 R7
	STR	R2, [R3], #-4			@@@		"	 R6
	STR	R2, [R3], #-4			@@@		"		 R5
	STR	R2, [R3], #-4			@@@		"		 R4

	@[	CODE_16 
	@STR	R2, [R3], #-4			@@@ Store initial state mask	

	@]
	STR	R2, [R3, #0]			@@@ Store solicited stack type on the
								
@@@ on the top of the stack.
@@@ Save the minimum amount of remaing stack memory
@@@ hisr -> tc_stack_minmium = REG_Stack_Size -	44 

	@[ CODE_16

	@MOV	R2, #48

	@|

	MOV	R2, #44					@@@ size of solicited stack

	@]

	SUB	R1, R1, R2
	STR	R1, [R0, #0x34]			@@@ Save in minimum stack area.
	
@@@ Save the new stack pointer into the task's control block.
@@@ hisr -> tc_stack_pointer = (VOID*) stack_Top

	STR		R3, [R0, #0x2c]		@@@ Save stack pointer	

	BX	LR

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@ TCT_Build_Signal_Frame:
@@@	Called by : TCC_Send_Signals
@@@	This function builds a frame on top of the task's stact to
@@@	cause the task's signal handler to execute the next time the task is 
@@@	executed.
@@@ 
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global		TCT_Build_Signal_Frame

TCT_Build_Signal_Frame:

@@@	Pickup the stack pointer.
@@@	REG_Stack_Ptr = (BYTE_PTR) task -> tc_stack_pointer

	LDR	R3, [R0, #0x2c]			@@@ Pickup the current stack pointer

@@@ Reference the Signal shell. 
@@@ REG_Function_Ptr =  (VOID *) TCC_Signal_Shell@
@@@
@@@ Build a signal stack.  This signal stack frame facilitates an solicited 
@@@ return to the TCC_Signal_Shell function, which in turn  invokes the appropr-
@@@ iate invokes the appropriate signal handler.  The initial HISR stack frame 
@@@ has the following format:
@@@               (Lower Address) Stack Top ->    0       (Solicited stack type)
@@@                !!FOR THUMB ONLY!!             0/0x20  Saved state mask
@@@                                               R4      Saved R4
@@@                                               R5      Saved R5
@@@                                               R6      Saved R6
@@@                                               R7      Saved R7
@@@                                               R8      Saved R8
@@@                                               R9/sb   Saved R9/sl
@@@                                               R10/sl  Saved R10/sl
@@@                                               fp      Saved fp
@@@                                               ip      Saved ip
@@@               (Higher Address) Stack Bottom-> pc      Saved pc

	
	LDR	R2, =TCC_Signal_Shell

	@[ CODE_16
	@BIC	R2, R2, #1				@@@ Clear low bit
	@]

	STR	R2, [R3], #-4			@@@ store entry address on stack.
	ADD	R2, R3, #0x4			@@@ Compute initial sp	
	STR	R2, [R3], #-4			@@@ Sotre initial ip
	STR	R2, [R3], #-4			@@@ Store initial fp
	LDR	R2, [R0, #0x24]			@@@ Pickup the stack starting address
	STR	R2, [R3], #-4			@@@ Store initial R10/S1
	MOV	R2, #0 					@@@ Clear value for initial registers
	STR	R2, [R3], #-4			@@@ Store initial R9/sb
	STR	R2, [R3], #-4			@@@ Store initial R8
	STR	R2, [R3], #-4			@@@  	"		 R7
	STR	R2, [R3], #-4			@@@		"		 R6
	STR	R2, [R3], #-4			@@@		"		 R5
	STR	R2, [R3], #-4			@@@		"		 R4

	@[ CODE_16 

	@MOV	R1, #0x20				@@@ Get initial state mask
	@STR	R1, [R3], #-4			@@@ Store initial state mask

	@]

	STR	R2, [R3, #0]			@@@ Store solicited stack type on the
								@@@ on the top of the stack.

@@@	Save the new stack pointer into the task's control block
@@@	task -> tc_stack_pointer = (VOID) (REG_Stack_Ptr - REG_Stack_Size)

	STR	R3, [R0, #0x2c]			@@@ save the stack pointer

	BX	LR
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@ 	TCT_Check_Stack - 
@@@	This function checks the current stack for overflow condi-
@@@	tions. Additionally, this function keeps track of the minim-
@@@ 	um amount of stack space for the calling thread and returns
@@@	the current available stack space.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global		TCT_Check_Stack

TCT_Check_Stack:

@@@ TC_TCB *thread,  UNSIGNED remaining@
@@@ Pickup the current task/HISR pointer.
@@@ thread = (TC_TCB *) TCD_Current_Thread

	LDR	R0, =TCD_Current_Thread
	LDR	R0, [R0, #0]

@@@ Determine if there is a current thread.  if (thread) {

	CMP	R0, #0						@@@ Determin if a thread is active
	MOV	R3, #0						@@@ Default remaining value
	
	BEQ	TCT_Skip_Stack_Check		@@@ If NU_NULL, skip atack checking.

@@@	Determine if the stack pointers are out of range
@@@ if ((thread -> tc_stack_pointer < thread -> tc_stack_start) ||
@@@      (thread -> tc_stack_pointer > thread -> tc_stack_end))

	LDR	R2, [R0, #0x24]
	CMP	sp, R2						@@@ Compare with current stack ptr

	BLT	TCT_Stack_Range_Error		@@@ if less, stack is out of range.
	
	LDR	R1, [R0, #0x28]				@@@ Pickup end of stack area
	CMP	sp, R1						@@@ Compare with current stack ptr

	BLE	TCT_Stack_Range_Okay		@@@ If less, stack range is okay
	
@@@ Stack overflow condition exits.
@@@ ERC_System_Error(NU_STACK_OVERFLOW)@

TCT_Stack_Range_Error:

	STR	  LR, [sp, #4]!				@@@ store LR on the stack
	MOV	  R0, #3					@@@ Build NU_STACK_OVERFLOW code

	@[ CODE_16

	@LDR	R3, =ERC_System_Error	 	@@@ Call system error handler, Note
	@BX	R3							@@@ control is not return!

@@@ Examine stack to find return.
@@@ address of this routine.		

	@|
	
	BL	ERC_System_Error			@@@ Call system error handler.

	@]

TCT_Stack_Range_Okay:

@@@ Calculate the amount of available space on the stack.
@@@ remaining = (BYTE_PTR) thread -> tc_stack_pointer - (BYTE_PTR) thread
@@@				-> tc_stack_start@

	SUB	R3, sp, R2					@@@ Calculate remianing stack size

@@@ Determine if there is enough memory on the stack to save all registers
@@@	if (remaining < 80 )

	CMP	R3, #80						@@@ Is there enough room for an
									@@@ interrupt frame?
	BCS	TCT_No_Stack_Error			@@@ If so, no stack overflow yet.

@@@ Stack overflow condition is about to happen
@@@ ERC_System_Error(NU_STACK_OVERFLOW)

	STR	lr, [sp, #4]!				@@@ Store LR on the stack
	MOV	R0, #3						@@@ Build NU_STACK_OVERFLOW code

	@[ CODE_16

	@LDR	R3, =ERC_System_Error		@@@ Call system error handler
	@BX	R3							@@@ control is not returned!

	@|
	
	BL	ERC_System_Error 			@@@ Call system error handler
									@@@ control in not returned.
	@]								@@@ Examins stack to find return
									@@@ address of this routine.
TCT_No_Stack_Error:

@@@ Determine if this is a new minimum amount of stack space
@@@ if (remaining < thread -> tc_stack_minimum)

	LDR	R2, [R0, #0x34]
	CMP	R3, R2
	STRCC	R3, [R0, #0x34]			@@@ Save the new stack minimum

@@@ thread->tc_stack_minimum=remainiing
@@@ Set the remaining byte to 0. remaining = 0@
@@@ Return the remaining number of bytes on the stack. return(remaining)@

TCT_Skip_Stack_Check:

	MOV	R0, R3						@@@ return remaining bytes

	BX	LR

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@ 	TCT_Schedule:
@@@	Called by: INC_Initialize
@@@	Calls : TCT_Control_To_Thread 
@@@	This function waits for a thread to become ready. Once a 
@@@	thread is ready, this function initiates a transfer of control
@@@	to that thread.
@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global		TCT_Schedule

TCT_Schedule:
@@@ Restore interrupts according to the value contained in TCD_ Interrupt_Level.
 	STMDB	sp!, {R0 - R12, lr}		@ Save minimal context of thread on

	bl TCT_Schedule_exit_time

	LDMIA	sp!, {R0 - R12, lr}		@ Save minimal context of thread on
 	LDR	R1, =TCD_Interrupt_Level			@@@ Build address of interrupt level
	MRS	R0, cpsr				@@@ Pickup current CPSR
	LDR	R2, [R1, #0]			@@@ pickup current interrupt lockout
	BIC	R0, R0, #LOCK_MSK		@@@ clear the interrupt lockout bits
	ORR	R0, R0, R2				@@@ Build new interrupt lockout CPSR	
	MSR	CPSR_cxsf, R0			@@@ Setup new CPSR
	LDR	R2, =TCD_Execute_HISR		@@@ Pickup TCD_execute_HISR address
	LDR	R3, =TCD_Execute_Task		@@@ Pickup TCD_Execute_task address

@@@ Wait until a thread (task or HISR) is available to execute.

TCT_Schedule_Loop:

	LDR	R0, [R2, #0]			@@@ Pickup highest priority HISR ptr
	CMP	R0, #0					@@@ Is there a HISR active?
	
	BNE	TCT_Schedule_Thread		@@@ Found an HISR
	
	LDR	R0, [R3, #0]			@@@ Pickup highest priority Task ptr
	CMP	R0, #0					@@@ Is there a task active?

	BEQ		TCT_Schedule_Loop	@@@ If not, continue the search

@@@ Yes, either a task or an HISR is ready to execute. Lockout interrupts while 
@@@ the thread is transferred to.

TCT_Schedule_Thread:

	MRS	R1, cpsr  
	ORR	R1, R1, #I_Bit			@@@ Build interrupt lockout value
	MSR	cpsr_cxsf, R1  			@@@ Lockout interrupts

@@@ Transfer control to the thread by falling through to the following routine.	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@ 	TCT_Control_To_Thread: 
@	Called by : 	TCT_Schedule - Indirectly called
@			TCT_Protect  - Protection task switch
@	Inputs: thread
@	This function transfers control to the specified thread. Each time control
@	is transferred to a thread, its scheduled counter is incremented. Additionlly,
@	time-slicing for task threads is enabled in this routine.
@ 	The TCD_Current_Thread pointer is setup by this function.
@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global		TCT_Control_To_Thread

TCT_Control_To_Thread:

@ Setup the Current thread pointer. TCD_Current_Thread = (VOID *) thread

	LDR	R1, =TCD_Current_Thread
	LDR	R2, [R0, #0x1c]			@ Pickup scheduled count
	STR	R0, [R1, #0]			@ setup current thread pointer

@ Increment the thread schedule counter. thread -> tc_scheduled++@

	LDR	R3, [R0, #0x20]			@ Pickup time slice value
	ADD	R2, R2, #1				@ Increment the scheduled count
	STR	R2, [R0, #0x1c]			@ Store new scheduled count

@ Check for time slice option. if (thread -> tc_cur_time_slice)

	CMP	R3, #0					@ Is there a time slice ?

	BEQ	TCT_No_Start_TS_1		@ If 0, there is no time slice
	
@ Start a time slice
@ TMD_Time_Slice = thread -> tc_cur_time_slice@
@ TMD_Time_Slice_State = 0@

	LDR	R2, =TMD_Time_Slice
	LDR	R1, =TMD_Time_Slice_State
	STR	R3, [R2, #0]			@ Setup the time slice
	MOV	R2, #0					@ Build active state flag
	STR	R2, [R1, #0]			@ Set the active flag

TCT_No_Start_TS_1:

@ Pickup the stack pointer and resume the thread.
@ REG_Stack_Ptr = thread -> tc_stack_pointer@

	LDR	SP, [R0, #0x2c]			@ Switch to thread's stack pointer
		
@	Pop off the saved information associated with the thread. After we
@	determine which type of stack is present.  A 1 on the top of the 
@	stack indicates an interrupt stack, while a 0 on the top of the
@	stack indicates a solicited type of stack.  
@        
@	Remember that the interrupt level that is restored must represent
@	the interrupt level in TCD_Interrupt_Level.

	LDR	R1, [sp], #4			@ Pop off the stack type
	CMP	R1, #1					@ If so, it is an interrupt stack 

	BEQ	TCT_Interrupt_Resume	@ If so, an interrupt resume of
								@ thread is required.
	LDR	R1, =TCD_Interrupt_Level
	MRS	R0, cpsr  				@ Pickup current CPSR
	BIC	R0, R0, #LOCK_MSK		@ Clear lockout mask
 	LDR	R2, [R1, #0]			@ Pickup interrupt lockout mask
 	ORR	R0, R0, R2				@ Build new interruptlockup mask

	@[ CODE_16
	@LDR	R2, [sp], #4
	@ORR	R0, R0, R2
	@]
	
	MSR	spsr_cxsf, R0  			@ place it into the SPSR
	LDMIA	sp!, {R4-ip, pc}^	@ A solicited return is required.
							@ This type of return only recover
								@ R4 - SP & PC
TCT_Interrupt_Resume:
	
	LDR	R0, [sp], #4			@ Pop off the CPSR
	LDR	R1, =TCD_Interrupt_Level	
	BIC	R0, R0, #LOCK_MSK		@ Clear lockout mask
	LDR	R2, [R1, #0]			@ Pickup interrupt lockout mask
	ORR	R0, R0, R2				@ Build new interrut lockout mask
	MSR	spsr_cxsf, R0  			@ Place it into the SPSR
	LDMIA	sp, {R0 - pc}^		@ Recover all registers and resume 
	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@	TCT_Control_To System :
@	Called by : Other Commponents
@	Calls	  : TCT_Schedule	- Schedule the next thread
@	This function returns control from a thread to the system system. 
@	Note that this service is called in a solicited manner, i.e. it is not 
@	called from an interrupt thread. 
@	Registers required by the compiler to be preserved across
@	function boundaries are saved by this routine. Note that this
@	is usually a sub-set of the total number of available registers. 
@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 
.global	 TCT_Control_To_System

TCT_Control_To_System:
@ Lockout interrupts
	
	MRS	R0, cpsr  					@ Pickup currrent CPSR
	ORR	R0, R0, #I_Bit				@ Build interrupt lockout value
	MSR	cpsr_cxsf, R0  				@ Lockout interrupt

@ Save a minimal context of the thread

	STMDB	sp!, {R4 - ip, lr}		@ Save minimal context of thread on

	@[ CODE_16			 				@ the current stack

	@MOV	R2,lr						@ Determin what state the caller
	@MOV	R2, R2, LSL #31
	@MOV	R2, R2, LSR #26				@ appropriate state mask
	@STR	R2,[sp, #-4]!				@ Place it on the stack

	@]
	MOV	R2, #0						@ Build solicited stack type
									@ and NU_NULL value
	STR	R2, [sp, #-4]!				@ Place it on the top of the stack

@ Setup a pointer to the thread control block. 
@ REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread@

	LDR	R1, =TCD_Current_Thread 
	LDR	R0, [R1, #0]				@ Pickup current thread pointer

@ Clear the current thread control block pointer.  
@ TCD_Current_Thread =  NU_NULL@

	LDR	R3, =TMD_Time_Slice_State		
	STR	R2, [R1, #0]				@ Set current thread pointer to 
									@ NU_NULL
@ Check to see if a time slice is active.  If so, copy the original time
@ slice into the current time slice field of the task's control block. 

	LDR	R1, [R3, #0]				@ Pickup time slice state flag
	CMP   	R1, #0					@ Compare with active value

	BNE     TCT_No_Stop_TS_1		@ If non-active, don't disable

@ Insure that the next time the task runs it gets a fresh time slice
@ REG_Thread_Ptr -> tc_cur_time_slice =  REG_Thread_Ptr -> tc_time_slice@

	LDR     R1, [R0, #0x40]			@ Pickup original time slice

@ 	Clear any active time slice by setting the state to NOT_ACTIVE.
@  TMD_Time_Slice_State =  1@

	MOV     R2, #1		           	@ Build disable value
	STR     R2, [R3, #0]		        @ Disable time slice
	STR     R1, [R0, #0x20]			@ Reset current time slice
	
TCT_No_Stop_TS_1:

@ Save off the current stack pointer in the control block.  
@ REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr@

	STR     sp, [R0, #0x2c]			@ Save the thread's stack pointer

@ Clear the task's current protection.  
@ (REG_Thread_Ptr -> tc_current_protect) -> tc_tcb_pointer =  NU_NULL@
@ REG_Thread_Ptr -> tc_current_protect =  NU_NULL@

	LDR     R1, [R0, #0x38]			@ Pickup current thread pointer
	MOV     R2, #0 				@ Build NU_NULL value
	STR     R2, [R0, #0x38]			@ Clear the protect pointer field
	STR     R2, [R1, #0]   			@ Release the actual protection

@ Switch to the system stack.  
@ REG_Stack_Ptr =  TCD_System_Stack@

	LDR	R1, =TCD_System_Stack
	LDR	R2, =TCT_System_Limit
	LDR	sp, [R1, #0]			@ Switch to system stack
	LDR	R10,[R2, #0]			@ Setup system stack limit
@ Finished, return to the scheduling loop.  

	B       TCT_Schedule    
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ 
@ 	TCT_Signal_Exit :
@	Called by: TCC_Signal_Shell - Signal handling shell func
@	Calls	 : TCT_Schedule     - Scheduler	
@		This function exits from a signal handler. The primary
@		purpose of this function is to clear the scheduler protection
@		and switch the stack pointer back to normal task's stack pointer.
@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global		TCT_Signal_Exit

TCT_Signal_Exit:

@ Lockout interrupts.  

	MRS	R3, cpsr			@ Pickup current CPSR	
	ORR	R3, R3, #LOCKOUT	      	@ Build lockout value
	MSR	cpsr_cxsf, R3  			@ Lockout interrupts

@ Setup a pointer to the thread control block.
@ REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread@
@
	LDR	R1, =TCD_Current_Thread	
	MOV     R2, #0				@ Build NU_NULL value
	LDR    	R0, [R1, #0]  			@ Pickup current thread pointer

@ Clear the current thread control block. 
@ TCD_Current_Thread =  NU_NULL@

	LDR	R3, =TMD_Time_Slice_State 
	STR	R2, [R1, #0]			@ Clear current thread pointer

@ Check to see if a time slice is active.  If so, copy the original time
@ slice into the current time slice field of the task's control block.
@  if (TMD_Time_Slice_State == 0)

	LDR	R1, [R3, #0]			@ Pickup time slice state flag
	CMP	R1, #0 					@ Compare with active value

	BNE	TCT_No_Stop_TS_2   		@ If non-active, don't disable

@ Insure that the next time the task runs it gets a fresh time slice.  
@ REG_Thread_Ptr -> tc_cur_time_slice =  REG_Thread_Ptr -> tc_time_slice@

	LDR	R1, [R0, #0x40]			@ Pickup original time slice

@ Clear any active time slice by setting the state to NOT_ACTIVE.
@ TMD_Time_Slice_State =  1@

	MOV	R2, #1					@ Build disable value
	STR	R2, [R3, #0]			@ Disable time slice
	STR	R1, [R0, #0x20]			@ Reset current time slice

TCT_No_Stop_TS_2:

@ Switch back to the saved stack.  The saved stack pointer was saved
@ before the signal frame was built. 
@ REG_Thread_Ptr -> tc_stack_pointer =  
@ REG_Thread_Ptr -> tc_saved_stack_ptr@

	LDR	R1, [R0, #0x3c] 	 	@ Pickup saved stack pointer
	STR	R1, [R0, #0x2c] 		@ Place in current stack pointer

@ Clear the task's current protection.  
@ REG_Thread_Ptr -> tc_current_protect) -> tc_tcb_pointer =  NU_NULL@
@ REG_Thread_Ptr -> tc_current_protect =  NU_NULL@

	LDR	R1, [R0, #0x38] 		@ Pickup current thread pointer
	MOV	R2, #0					@ Build NU_NULL value
	STR	R2, [R0, #0x38] 		@ Clear the protect pointer field
	STR	R2, [R1, #0]			@ Release the actual protection
	
@ Switch to the system stack. 
@ REG_Stack_Ptr =  (BYTE_PTR) TCD_System_Stack@

	LDR	R1, =TCD_System_Stack  
	LDR	R2, =TCT_System_Limit 
	LDR	sp, [R1, #0]			@ Switch to system stack
	LDR	R10,[R2, #0]			@ Setup system stack limit

@ Finished, return to the scheduling loop.

	B	TCT_Schedule			@ Return to scheduling loop

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@	TCT_Current_Thread :
@	Called by :	Application, system commponents
@	OutPuts: Pointer to current thread
@	This function  returns the current thread pointer.
@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 
.global	 TCT_Current_Thread

TCT_Current_Thread:

@ Return the current thread pointer.  
@ return(TCD_Current_Thread)@

	LDR	R0, =TCD_Current_Thread
	LDR	R0, [R0, #0]			@ Pickup current thread pointer

	BX	LR
        
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ 
@	TCT_Set_Execute_Task :
@	Called by : TCC Scheduling Routines
@	Outputs	  : TCD_Execute_Task	
@		    This function sets the current task to execute variabl
@		    under protecttion against interrupts.
@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global TCT_Set_Execute_Task

TCT_Set_Execute_Task:

@ Now setup the TCD_Execute_Task pointer.  
@ TCD_Execute_Task =  task@

	LDR	R1, =TCD_Execute_Task 	 	@ Pickup execute task ptr address
	STR	R0, [R1, #0]			@ Setup new task to execute

	BX		LR

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@	TCT_Protect :
@	Called by : Application, System components
@	Inputs	  : protect	
@	This function protects against multiple thread access.
@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global TCT_Protect

TCT_Protect:

@ Determine if the caller is in a task or HISR thread.  
@ if (TCD_Current_Thread) {
	
	LDR     R1, =TCD_Current_Thread
	LDR     R3, [R1, #0] 			@ Pickup current thread pointer
	CMP     R3, #0 					@ Check to see if it is non-NULL

	BEQ     TCT_Skip_Protect		@ If NULL, skip protection
    
@ Lockout interrupts.  

	MRS	R1, cpsr      				@ Pickup current CPSR
	ORR	R1, R1, #LOCKOUT 			@ Place lockout value in
	MSR	cpsr_cxsf, R1				@ Lockout interrupts
	
@ Wait until the protect structure is available.  
@ while (protect -> tc_tcb_pointer != NU_NULL)

TCT_Protect_Loop:

	LDR 	R1, [R0, #0]			@ Pickup protection owner field
	CMP	R1, #0 						@ Is there any protection?
	
	BEQ	TCT_Protect_Available  		@ If NU_NULL, no current protection

@ Protection structure is not available.  
@ Indicate that another thread is waiting. 
@ protect -> tc_thread_waiting =  1@

	MOV	R2, #1 						@ Build thread waiting flag
	STR	R2, [R0, #4]            	@ Set waiting field

@ Directly schedule the thread waiting.  
@ TCT_Schedule_Protected(protect -> tc_tcb_pointer)@

	STR	R0, [sp, #-4]!				@ Save a1 on the stack
	STR	lr, [sp, #-4]!				@ Save lr on the stack
	MOV	R0, R3						@ Place current thread into R0

	BL	TCT_Schedule_Protected		@ Call routine to schedule the

	LDR	lr, [sp], #4 				@ Recover saved lr
	LDR	R0, [sp], #4 				@ Recover saved R0

@ Lockout interrupts. 

	LDR     R1, =TCD_Current_Thread 
	LDR     R3, [R1, #0]			@ Pickup current thread pointer
	MRS     R1, cpsr 				@ Pickup current CPSR
	ORR     R1, R1, #LOCKOUT		@ Place lockout value in
	MSR     cpsr_cxsf, R1        	@ Lockout interrupts

	B	TCT_Protect_Loop			@ Examine protect flags again

TCT_Protect_Available:

@ Protection structure is available.  

@ Indicate that this thread owns the protection.  
@  protect -> tc_tcb_pointer =  TCD_Current_Thread@

	STR     R3, [R0, #0]			@ Indicate calling thread owns this
									@ protection
@ Clear the thread waiting flag.  
@ protect -> tc_thread_waiting =  0@

	MOV     R2, #0					@ Clear value
	STR     R2, [R0, #4]			@ Clear the thread waiting flag

@ Save the protection pointer in the thread's control block.  Note
@ that both task and HISR threads share the same control block format.  
@ REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread@
@ REG_Thread_Ptr -> tc_current_protect =  protect@

	STR     R0, [R3, #0x38]   		@ Setup current protection
@ Restore interrupts.  

	LDR     R2, =TCD_Interrupt_Level
	MRS     R1, cpsr 		 	 	@ Pickup current CPSR
	LDR     R3, [R2, #0]			@ Pickup interrupt lockout level
	BIC     R1, R1, #LOCK_MSK		@ Clear lockout bits
	ORR     R1, R1, R3 			 	@ Build new interrupt lockout
	MSR     cpsr_cxsf, R1  			@ Setup CPSR appropriately

TCT_Skip_Protect:
	BX	lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@	TCT_Unprotect:
@	Called by: Application, System Components
@	This function releases protection of the currently active 
@	thread. If the caller is not an active thread, than this request
@	is ignored.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global		TCT_Unprotect

TCT_Unprotect:

@ Determine if the caller is in a task or HISR thread.  
@ if (TCD_Current_Thread)

	LDR	R1, =TCD_Current_Thread
	LDR	R3, [R1, #0]			@ Pickup current thread pointer
	CMP	R3, #0  				@ Check to see if it is non-NULL

	BEQ	TCT_Skip_Unprotect		@ If NULL, skip unprotection

@ Setup a thread control block pointer.  
@ REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread@

@ Determine if there is a currently active protection.  
@ if (REG_Thread_Ptr -> tc_current_protect)

	LDR     R0, [R3, #0x38]			@ Pickup current protect field
	CMP     R0, #0        			@ Is there a protection in force?
	BEQ     TCT_Skip_Unprotect		@ If not, nothing is protected
        
@ Lockout interrupts.  

	MRS     R1, cpsr 			@ Pickup current CPSR
	ORR     R1, R1, #LOCKOUT   		@ Place lockout value in
	MSR     cpsr_cxsf, R1   			@ Lockout interrupts

@ Yes, this thread still has this protection structure.  
@ REG_Protect_Ptr =  REG_Thread_Ptr -> tc_current_protect@

@ Is there a higher priority thread waiting for the protection structure? 
@ if (REG_Protect_Ptr -> tc_thread_waiting)
            
	LDR     R2, [R0, #4]                	@ Pickup thread waiting flag
	CMP     R2, #0                   	@ Are there any threads waiting?

	BEQ     TCT_Not_Waiting_Unpr		@ If not, just release protection

@ Transfer control to the system.  Note that this automatically clears the 
@ current protection and it returns to the caller of this routine instead of 
@ this routine.  TCT_Control_To_System()@

	B       TCT_Control_To_System       @ Return control to the system


TCT_Not_Waiting_Unpr:
            
@ Clear the protection.  
@ REG_Thread_Ptr -> tc_current_protect =  NU_NULL@
@ REG_Protect_Ptr -> tc_tcb_pointer =  NU_NULL@

	MOV     R2, #0                     	@ Build NU_NULL value
	STR     R2, [R0, #0]				@ Release the protection
	STR     R2, [R3, #0x38]             @ Clear protection pointer in the
 	                                    @ control block 
TCT_Not_Protected:

@ Restore interrupts again. 

	LDR     R2, =TCD_Interrupt_Level    		
	MRS     R1, cpsr                    @ Pickup current CPSR
	LDR     R3, [R2, #0]				@ Pickup interrupt lockout level
	BIC     R1, R1, #LOCK_MSK           @ Clear lockout bits
	ORR     R1, R1, R3                  @ Build new interrupt lockout
	MSR     cpsr_cxsf, R1   		      	@ Setup CPSR appropriately

TCT_Skip_Unprotect:

	BX		lr	

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@	TCT_Unprotect_Specific :
@	Called by :	Application, system Components	
@ 		This function releases a specific protection structure.
@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global	TCT_Unprotect_Specific

TCT_Unprotect_Specific:

@ Determine if the caller is in a task or HISR thread.  
@ if (TCD_Current_Thread)

	LDR     R1, =TCD_Current_Thread
	LDR     R3, [R1, #0]                @ Pickup current thread pointer
	CMP     R3, #0                   	@ Check to see if it is non-NULL

	BEQ     TCT_Skip_Unprot_Spec       	@ If NULL, skip unprotect specific
     
@ Lockout interrupts.  

	MRS     R1, cpsr                    @ Pickup current CPSR
	ORR     R1, R1, #LOCKOUT         	@ Place lockout value in
	MSR     cpsr_cxsf, R1  				@ Lockout interrupts

@ Clear the protection pointer.  
@  protect -> tc_tcb_pointer =  NU_NULL@
        
	MOV     R2, #0                      @ Build NU_NULL value
	STR     R2, [R0, #0]                @ Clear protection ownership

@ Determine if a thread is waiting.  
@ if (protect -> tc_thread_waiting)

	LDR     R1, [R0, #4]               	@ Pickup the waiting field
	CMP     R1, #0                      @ Is there another thread waiting?

	BEQ     TCT_Not_Waiting_Unspec 		@ No, restore interrupts and return
        
@ A higher-priority thread is waiting.  
@ Save a minimal context of the thread.  

	STMDB   sp!, {R4-ip, lr}            @ Save minimal context of thread on

	@[ CODE_16 	
	@MOV     R2, lr                			@ Determine what state the caller
	@MOV     R2, R2, LSL #31 	        	@ was in and build an
	@MOV     R2, R2, LSR #26       			@ appropriate state mask
	@STR     R2,[sp, #-4]!               	@ Place it on the stack
	
	@]        
	                            	        @ the current stack
	MOV     R2, #0                     		@ Build solicited stack type value
	                                        @ and NU_NULL value
	STR     R2,[sp, #-4]!              		@ Place it on the top of the stack
	
@ Setup a pointer to the thread control block.  
@ REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread@

	LDR     R1, =TCD_Current_Thread
	LDR     R0, [R1, #0]                	@ Pickup current thread pointer

@ Clear the current thread control block pointer.  
@ TCD_Current_Thread =  NU_NULL@
    
	LDR     R3, =TMD_Time_Slice_State  
	STR     R2, [R1, #0]              		@ Set current thread pointer to 
	                                    	@ NU_NULL
@ Check to see if a time slice is active.  If so, copy the original time slice 
@ into the current time slice field of the thread's control block.
@ if (TMD_Time_Slice_State == 0)

	LDR     R1, [R3, #0]              		@ Pickup time slice state flag
	CMP     R1, #0                      	@ Compare with active value

	BNE     TCT_No_Stop_TS_3           		@ If non-active, don't disable

@ Insure that the next time the task runs it gets a fresh time slice.  
@ REG_Thread_Ptr -> tc_cur_time_slice =  
@ REG_Thread_Ptr -> tc_time_slice@

	LDR     R1, [R0, #0x40]   	 			@ Pickup original time slice

@ Clear any active time slice by setting the state to NOT_ACTIVE.
@ TMD_Time_Slice_State =  1@

	MOV     R2, #1							@ Build disable value
	STR     R2, [R3, #0]                 	@ Disable time slice
	STR     R1, [R0, #0x20]             	@ Reset current time slice

TCT_No_Stop_TS_3:

@ Save off the current stack pointer in the control block.  
@ REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr@

	STR     sp, [R0, #0x2c]            		@ Save the thread's stack pointer

@ Switch to the system stack.  
@ REG_Stack_Ptr =  TCD_System_Stack@

	LDR     R1, =TCD_System_Stack  
	LDR     R2, =TCT_System_Limit  
	LDR     sp, [R1, #0]               		@ Switch to system stack
	LDR     R10,[R2, #0]                 	@ Setup system stack limit

@ Finished, return to the scheduling loop.  

	B       TCT_Schedule 					@ Return to scheduling loop

TCT_Not_Waiting_Unspec:
        
@ No higher-priority thread is waiting.  
@ Restore interrupts.  

	LDR     R2, =TCD_Interrupt_Level
	MRS     R1, cpsr                    	@ Pickup current CPSR
	LDR     R3, [R2, #0]                 	@ Pickup interrupt lockout level
	BIC     R1, R1, #LOCK_MSK         		@ Clear lockout bits
	ORR     R1, R1, R3                   	@ Build new interrupt lockout
	MSR     cpsr_cxsf, R1  					@ Setup CPSR appropriately

TCT_Skip_Unprot_Spec:

	BX	lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@	TCT_Set_Current_Protect :
@		 This function sets the current protection field of 
@		the current thread's control block to the specified
@		protection pointer.
@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global		TCT_Set_Current_Protect

TCT_Set_Current_Protect:

@ Determine if the caller is in a task or HISR thread.  
@ if (TCD_Current_Thread)

	LDR     R1, =TCD_Current_Thread 
	LDR     R3, [R1, #0]          			@ Pickup current thread pointer
	CMP     R3, #0                    		@ Check to see if a thread is active 
	                                          	
@ Point at the current thread control block. 
@ REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread@
    
@ Modify the current protection.  
@ REG_Thread_Ptr -> tc_current_protect =  protect@

	STRNE   R0, [R3, #0x38]               	@ Setup new protection

	BX	lr
	
@ =============================================================================
@ TCT_Protect_Switch -This function waits until a specific task no longer has
@ any protection associated with it.  This is necessary since task's cannot
@ be suspended or terminated unless they have released all of their protection.
@ =============================================================================

.global		TCT_Protect_Switch

TCT_Protect_Switch:

@ Lockout interrupts.  
    
	MRS     R1, cpsr			@ Pickup current CPSR
	ORR     R1, R1, #LOCKOUT		@ Place lockout value in
	MSR     cpsr_cxsf, R1  			@ Lockout interrupts

@ REG_Thread_Ptr =  (TC_TCB *) thread@
    
@ Wait until the specified task has no protection associated with it. 
@ while (REG_Thread_Ptr -> tc_current_protect)

	LDR     R1, [R0, #0x38]             	@ Pickup protection of specified
                                          	@ thread
	CMP     R1, #0							@ Does the specified thread have
	                                        @ an active protection?
	                                          
    BEQ     TCT_Switch_Done					@ If not, protect switch is done
  
@ Let the task run again in an attempt to clear its protection.  
@ Indicate that a higher priority thread is waiting.  
@ (REG_Thread_Ptr -> tc_current_protect) -> tc_thread_waiting =  1@
        
	MOV     R2, #1                 			@ Build waiting flag value
	STR     R2, [R1, #4]                	@ Set waiting flag of the 
											@ protection owned by the other
											@ thread
@ Directly schedule the thread waiting.  
@ TCT_Schedule_Protected((REG_Thread_Ptr->tc_current_protect->tc_tcb_pointer)

	LDR     R2, =TCD_Current_Thread
	STR     R0, [sp, #-4]!             		@ Save a1 on the stack
	STR     lr, [sp, #-4]!        			@ Save lr on the stack
	MOV     R1,	R0                     		@ Move new thread into a2
	LDR     R0, [R2, #0]                 	@ Pickup current thread pointer
	
	BL      TCT_Schedule_Protected  		@ Call routine to schedule the
	                                        @ owner of the thread
	LDR     lr, [sp], #4               		@ Recover saved lr
	LDR     R0, [sp], #4          			@ Recover saved a1
	            
@ Lockout interrupts.  

	B       TCT_Protect_Switch      		@ Branch to top of routine and 
		                                	@ start
TCT_Switch_Done:

@ Restore interrupts. 

	LDR     R2, =TCD_Interrupt_Level     
	MRS     R1, cpsr                 		@ Pickup current CPSR
	LDR     R3, [R2, #0]                	@ Pickup interrupt lockout level
	BIC     R1, R1, #LOCK_MSK             	@ Clear lockout bits
	ORR     R1, R1, R3                    	@ Build new interrupt lockout
	MSR     cpsr_cxsf, R1   					@ Setup CPSR appropriately

	BX	lr

@ =============================================================================
@  TCT_Schedule_Protected - This function saves the minimal context of the 
@	thread and then directly schedules the thread that has protection
@	over thee thread that called this routine. .
@ =============================================================================

.global		TCT_Schedule_Protected

TCT_Schedule_Protected:

@ Interrupts are already locked out by the caller. 
@ Save minimal context required by the system.  

	STMDB   sp!, {R4-ip, lr}	        @ Save minimal context of thread on

	@[ CODE_16 

	@MOV     R2, lr						@ Determine what state the caller
	@MOV     R2, R2, LSL #31            	@ was in and build an
	@MOV     R2, R2, LSR #26             @ appropriate state mask
	@STR     R2, [sp, #-4]!            	@ Place it on the stack

	@]
										@ the current stack
	MOV   R2, #0          		        @ Build solicited stack type value
	   	                                @ and NU_NULL value
	STR	R2, [sp, #-4]!             		@ Place it on the top of the stack
	MOV	R4, R1                    		@ Save thread to schedule
	
@@@ Setup a pointer to the thread control block. 
@@@ REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread@
    
	LDR	R1, =TCD_Current_Thread

@@@ Clear the current thread control block.  
@@@ TCD_Current_Thread =  NU_NULL@

	LDR	R3, =TMD_Time_Slice_State
	STR	R2, [R1, #0]               		@ Set current thread pointer to
										@ NU_NULL
@@@ Check to see if a time slice is active.  If so, copy the original time
@@@ slice into the current time slice field of the task's control block. 
@@@ if (TMD_Time_Slice_State == 0)
	LDR     R1, [R3, #0]	            @ Pickup time slice state flag
	CMP     R1, #0						@ Compare with active value
	
	BNE     TCT_No_Stop_TS_4 			@ If non-active, don't disable
    
@@@ Insure that the next time the task runs it gets a fresh time slice.  
@@@ REG_Thread_Ptr -> tc_cur_time_slice =  REG_Thread_Ptr -> tc_time_slice@

	LDR     R1, [R0, #0x40]          	@ Pickup original time slice

@@@ Clear any active time slice by setting the state to NOT_ACTIVE.  
@@@ TMD_Time_Slice_State =  1@

	MOV     R2, #1  	             	@ Build disable value
	STR     R2, [R3, #0]	            @ Disable time slice
	STR     R1, [R0, #0x20]   	        @ Reset current time slice

TCT_No_Stop_TS_4:

@@@ Save off the current stack pointer in the control block.  
@@@ REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr@

	STR     sp, [R0, #0x2c]           	@ Save the thread's stack pointer

@@@ Switch to the system stack.  
@@@ TCD_System_Stack =  (VOID *) REG_Stack_Ptr@

	LDR     R1, =TCD_System_Stack
										@ Pickup address of stack pointer
	LDR     R2, =TCT_System_Limit
	
	LDR     sp, [R1, #0] 	            @ Switch to system stack
	LDR     R10, [R2, #0] 	        	@ Setup system stack limit
	
@@@ Transfer control to the specified thread directly. 
@@@ TCT_Control_To_Thread(thread)@

	LDR     R2, =TCD_Interrupt_Level    
	MRS     R1, cpsr                	@ Pickup current CPSR
	LDR     R3, [R2, #0]               	@ Pickup interrupt lockout level
	BIC     R1, R1, #LOCK_MSK    		@ Clear lockout bits
	ORR     R1, R1, R3                    	@ Build new interrupt lockout
	MOV     R0, R4                     	@ Indicate thread to schedule
	MSR     cpsr_cxsf, R1  		  	@ Setup CPSR appropriately
	ORR     R1, R1, #LOCKOUT              	@ Build lockout value again
	MSR     cpsr_cxsf, R1  			@ Lockout interrupts again
	
	B       TCT_Control_To_Thread 	    	@ Schedule the thread indirectly
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@  TCT_Interrupt_Context_Save - This function saves the interrupted thread's
@@@  context.  Nested interrupts are also supported.  If a task or HISR 
@@@  thread was interrupted, the stack pointer is switched to the system stack  
@@@  after the context is saved. 
@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global		TCT_Interrupt_Context_Save

TCT_Interrupt_Context_Save:

@@@ This routine is designed to handle IRQ interrupts.  The IRQ
@@@ stack is used as a temporary area.  Actual context is saved either on the
@@@ interrupted thread's stack or the system stack- both of which are in the
@@@ Supervisor (SVC) mode.  Note:  upon entry to this routine R0 - R3 are
@@@ saved on the current stack and R3 contains the original (interrupt return 
@@@ address) LR value.  The current lr contains the ISR return address.  

@@@ Determine if this is a nested interrupt.  
	MRS	R1, CPSR					@ Pickup Current CPSR
	ORR	R1, R1, #I_Bit				@ #LOCKOUT Build lockout variable
	MSR	cpsr_cxsf, R1        		@ Lockout interrupts

@@@ Nested so mark the Int_Count Flag            
	LDR     R0, =TCD_Interrupt_Count
	LDR     R1, [R0, #0]			@ Pickup interrupt counter
	
	CMP	R1, #0						@ Is it nested ?
	BGT	TCT_Nested_Save				@ Yes
	B	TCT_Not_Nested_Save			@ No

@@@ Nested interrupt.  Save complete context on the current stack.  
TCT_Nested_Save:
	ADD     R1, R1, #1              @ Increment the interrupt counter
	STR     R1, [R0, #0]            @ Store in interrupt counter
	STMDB   sp!,{R4 - R6}        	@ Save more registers on current
	MRS     R4, SPSR                @ Pickup and save current SPSR
	MOV     R5, LR                  @ Save current LR
	MOV     R6, SP                 	@ Save current SP
	ADD     SP, SP, #28             @ Adjust sp for future interrupts
	MRS     R0, CPSR                @ Pickup current CPSR
	BIC     R0, R0, #MODE_MASK      @ Clear the mode bits
	ORR     R0, R0, #SVC_MODE       @ Prepare to switch to supervisor
                      				@ mode (SVC)
	MSR     cpsr_cxsf, R0  			@ Switch to SVC mode
	MOV     R1, SP                 	@ Use a non sp register
	                                 
	STR     R3, [R1, #-4]!          @ Save interrupted pc on system stack
	MOV     R3, R9                  @ Save sb in R3
	STMDB   R1!, {R7 - lr}          @ Save R7-LR on the system stack
	MOV     SP, R1					@ Setup sp again
	LDMIA   R6!, {R7 - R9}          @ Recover R4-R6 from int stack
	STMDB   SP!, {R7 - R9}          @ Save R4-R6 on the system stack
	MOV     R9, R3					@ Recover sb
	LDMIA   R6, {R0 - R3}           @ Recover R0 - R3
	STMDB   SP!,{R0 - R3}           @ Save R0-R3 on the system stack
	STR     R4, [sp, #-4]!      	@ Save CPSR on the stack

	BX	R5

TCT_Not_Nested_Save:

@@@ Determine if a thread was interrupted. 
@@@ if (TCD_Current_Thread)

	ADD	R1, R1, #1						@ Increment the interrupt counter
	STR	R1,[R0, #0]						@ Store in interrupt counter
	
	LDR     R1, =TCD_Current_Thread
	LDR     R1, [R1, #0]    			@ Pickup the current thread pointer
	CMP     R1, #0        				@ Is it NU_NULL?

	BEQ     TCT_Idle_Context_Save     	@ If no, no real save is necessary

@@@ Yes, a thread was interrupted.  Save complete context on the thread's stack.  

	STMDB   sp!,{R4 - R6}          		@ Save more registers on temp stack
	MOV     R5, lr           	        @ Save interrupt LR in R5
	MRS     R4, spsr           			@ Save interrupt SPSR in R4
	MOV     R6, SP						@ Save current sp in v3
	ADD     SP, SP, #28                 @ Adjust sp for future interrupts
	
	MRS     R0, CPSR                  	@ Pickup current CPSR
	BIC     R0, R0, #MODE_MASK         	@ Clear the mode bits
	ORR     R0, R0, #SVC_MODE        	@ Prepare to switch to supervisor
                           				@ mode (SVM)
	MSR     cpsr_cxsf, R0   				@ Switch to supervisor mode (SVC)
	MOV     R1, SP             			@ Use a non-stack pointer register
	NOP                                 
	STR     R3, [R1, #-4]!        		@ Save interrupted pc on the stack
	STMDB   R1!, {R7 - LR}              @ Save R7-lr on the stack
	MOV     SP, R1						@ Setup sp again
	LDMIA   R6!,{R7 - R9}   	        @ Recover R4-R6 into R7-R9
	STMDB   sp!,{R7 - R9}       	    @ Save R4-R6 on the stack
	LDMIA   R6 ,{R0 - R3}           	@ Recover R0-R3
	STMDB   sp!,{R0 - R3} 				@ Save R0-R3 on the stack
	STR     R4, [sp, #-4]!          	@ Save CPSR on the stack
	MOV     R1, #1                     	@ Interrupt stack type
	STR     R1, [sp, #-4]!            	@ Put interrupt stack type on top
                                        @ of stack
@@@  Save the thread's stack pointer in the control block.  
@@@  REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread@
@@@  REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr@

	LDR     R1, =TCD_Current_Thread 
	LDR     R0, [R1, #0]       			@ Pickup current thread pointer
	STR     SP, [R0, #0x2c]             @ Save stack pointer

@@@ Switch to the system stack.  
@@@ REG_Stack_Ptr =  TCD_System_Stack@

	LDR     R1, =TCD_System_Stack
	LDR     R2, =TCT_System_Limit
	LDR     sp, [R1, #0]				@ Switch to system stack
	LDR     R10,[R2, #0]                @ Setup system stack limit

@@@ Unlock interrupts.
@@@      MRS     R1, CPSR                @ Pickup current CPSR
@@@      BIC     R1, R1,#LOCKOUT         @ Build lockout value
@@@      MSR     CPSR_all, R1            @ Lockout interrupts

@@@ Return to caller ISR.  

	BX	R5								@ Return to caller ISR

TCT_Idle_Context_Save:

	MOV     R5, LR                    		@ Save lr in R5
	ADD     SP, SP, #16                		@ Adjust sp for future interrupts
	MRS     R0, CPSR    				@ Pickup current CPSR
	BIC     R0, R0, #MODE_MASK            		@ Clear the current mode
	ORR     R0, R0, #SVC_MODE            		@ Prepare to switch to supervisor
                                    	        	@ mode (SVC)
	MSR     cpsr_cxsf, R0  		      	 	@ Switch to supervisor mode (SVC)
	
	BX	R5

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@ 
@@@  TCT_Interrupt_Context_Restore - This function restores the interrupt context
@@@	if a nested interrupt condition is present. Otherwise, this routine 
@@@	transfers control to the scheduling function. 
@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global TCT_Interrupt_Context_Restore

TCT_Interrupt_Context_Restore:

@@@ It is assumed that anything pushed on the stack by ISRs has been
@@@ removed upon entry into this routine.  

@@@ Lockout interrupts.  

	MRS     R1, CPSR 				@ Pickup current CPSR
	ORR     R1, R1,#LOCKOUT 		@#I_Bit Build lockout value
	MSR     cpsr_cxsf, R1   			@ Lockout interrupts

@@@ Decrement and check for nested interrupt conditions.  
@@@ if (--TCD_Interrupt_Count)

	LDR     R0, =TCD_Interrupt_Count			@ Pickup address of interrupt count
	LDR     R1, [R0, #0]            @ Pickup interrupt counter
	SUB	R1, R1, #1					@ Decrement interrupt counter
	STR	R1, [R0, #0]				@ Store interrupt counter
	CMP     R1, #0

	BEQ     TCT_Not_Nested_Restore
	           						@ Store interrupt counter
@@@ Restore previous context.  
	
	LDR	R0, [SP], #4
	MSR	spsr_cxsf, R0
	LDMIA   sp, {R0 - pc}^          @ Return to the point of interrupt


TCT_Not_Nested_Restore:

@@@ Determine if a thread is active.  
@@@ if (TCD_Current_Thread)

	LDR     R1, =TCD_Current_Thread
	LDR     R0, [R1, #0]  				@ Pickup current thread pointer
	CMP     R0, #0              		@ Determine if a thread is active
	
	BEQ     TCT_Idle_Context_Restore	@ If not, idle system restore

@@@ Clear the current thread pointer.  
@@@ TCD_Current_Thread =  NU_NULL@

	
@@@ Determine if a time slice is active.  If so, the remaining time left on the
@@@ time slice must be saved in the task's control block.  
@@@ if (TMD_Time_Slice_State == 0)
	
	LDR     R3, =TMD_Time_Slice_State
	MOV	R2, #0
	STR	R2, [R1, #0]
	
	LDR     R1, [R3, #0]             	@ Pickup time slice state
	CMP     R1, #0               		@ Determine if time slice active
	
	BNE     TCT_Idle_Context_Restore 	@ If not, skip time slice reset

@@@ Pickup the remaining portion of the time slice and save it in the task's
@@@ control block.  
@@@ REG_Thread_Ptr -> tc_cur_time_slice =  TMD_Time_Slice@
@@@ TMD_Time_Slice_State =  1@

	LDR     R2, =TMD_Time_Slice
	MOV   	R1, #1                      	@ Build disable time slice value
	LDR		R2, [R2, #0]            			@ Pickup remaining time slice
	STR    	R1, [R3, #0]     				@ Disable time slice
	STR    	R2, [R0, #0x20]    				@ Store remaining time slice


TCT_Idle_Context_Restore:

@@@ Reset the system stack pointer.  

	LDR     R1, =TCD_System_Stack
	LDR     R2, =TCT_System_Limit
	LDR     sp, [R1, #0]      			@ Switch to system stack
	LDR     R10,[R2, #0]				@ Setup system stack limit
										@ Return to scheduler.  

	B       TCT_Schedule       			@ Return to scheduling loop

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@ TCT_Activate_HISR -This function activates the specified HISR.  If the HISR
@@@ is already activated, the HISR's activation count is simply incremented. 
@@@ Otherwise, the HISR is placed on the appropriate HISR priority list in pre-
@@@ paration for execution. 
@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global		TCT_Activate_HISR

TCT_Activate_HISR:

@@@ INT     priority@
@@@ Lockout interrupts.  

	STR     R4, [sp, #-4]!           		@ Save R4
	MRS     R4, CPSR                     	@ Pickup current CPSR
	ORR     R1, R4, #LOCKOUT             	@ Build interrupt lockout value
	MSR     cpsr_cxsf, R1  					@ Lockout interrupts

@@@ Determine if the HISR is already active.  
@@@ if (hisr -> tc_activation_count)@

	LDR     R1, [R0, #0x40]	            	@ Pickup current activation count
	CMP     R1, #0          	        	@ Is it the first activation?

	BEQ     TCT_First_Activate  			@ Yes, place it on the correct list

@@@ Increment the activation count.  Make sure that it does not go to zero.
@@@ hisr -> tc_activation_count++@

	ADDS    R1, R1, #1                  	@ Increment the activation count
	STR     R1, [R0, #0x40]           		@ Store new activation count

@@@ if (hisr -> tc_activation_count == 0)
@@@ hisr -> tc_activation_count =  0xFFFFFFFFUL@

	MVNEQ   R1, #0							@ If counter rolled-over reset
	STREQ   R1, [R0, #0x40]   	        	@ Store all ones count

	B       TCT_Activate_Done				@ Finished with activation
	
TCT_First_Activate:

@@@ Set the activation count to 1.  
@@@ hisr -> tc_activation_count =  1@

	MOV     R1, #1							@ Initial activation count
	STR     R1, [R0, #0x40]            		@ Store initial activation count

@@@ Pickup the HISR's priority.  
@@@ priority =  hisr -> tc_priority@

@@@ Determine if there is something in the given priority list.  
@@@ if (TCD_Active_HISR_Tails[priority])

	LDRB    R1, [R0, #0x1a]	        	@ Pickup priority of HISR
	LDR     R2, =TCD_Active_HISR_Tails 
	LDR     R3, [R2, R1, LSL #2]       	@ Pickup tail pointer for priority
	CMP     R3, #0              		@ Is this first HISR at priority?

	BEQ     TCT_First_HISR				@ No, append to end of HISR list

@@@ Something is already on this list.  Add after the tail.  
@@@ (TCD_Active_HISR_Tails[priority]) -> tc_active_next =  hisr@
@@@ TCD_Active_HISR_Tails[priority] =  hisr@

	STR     R0, [R3, #0x3c]             @ Setup the active next pointer
	STR     R0, [R2, R1, LSL #2]    	@ Setup the tail pointer

	B       TCT_Activate_Done 			@ Finished with activate processing
	
TCT_First_HISR:

@@@ Nothing is on this list.  
@@@ TCD_Active_HISR_Heads[priority] =  hisr@
@@@ TCD_Active_HISR_Tails[priority] =  hisr@

	LDR     R3, =TCD_Active_HISR_Heads    
	STR     R0, [R2, R1, LSL #2]          	@ Set tail pointer to this HISR
	STR     R0, [R3, R1, LSL #2]           	@ Set head pointer to this HISR

@@@ Determine the highest priority HISR.  
@@@ if (TCD_Active_HISR_Heads[0])
@@@ TCD_Execute_HISR =  TCD_Active_HISR_Heads[0]@
@@@ else if (TCD_Active_HISR_Heads[1])
@@@ TCD_Execute_HISR =  TCD_Active_HISR_Heads[1]@
@@@ TCD_Execute_HISR =  TCD_Active_HISR_Heads[2]@

	LDR     R1, [R3, #0]			@ Pickup priority 0 head pointer
	LDR     R0, =TCD_Execute_HISR
	CMP     R1, #0          		@ Is priority 0 active?
	LDREQ   R1, [R3, #4]               	@ If not, pickup priority 1 head 
	CMPEQ   R1, #0     			@ Is priority 1 active?
	LDREQ   R1, [R3, #8]			@ Else, must be priority 2 active
	STR     R1, [R0, #0]                 	@ Store which ever priority is the
	                                      	@ active one
TCT_Activate_Done:

	MSR     cpsr_cxsf, R4 	               	@ Restore interrupt lockout
	LDR     R4, [sp], #4      		@ Restore corrupted R4

	MOV	  R0, #0                     	@ Always return NU_SUCCESS

	BX	lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@ 
@@@  TCT_HISR_Shell-This function is the execution shell of each and every HISR. 
@@@  If the HISR has completed its processing, this shell routine exits back to
@@@  the system.  Otherwise, it sequentially calls the HISR routine until the
@@@  activation count goes to zero. .
@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global TCT_HISR_Shell

TCT_HISR_Shell:

@@@ Point at the HISR.  
@@@ REG_HISR_Ptr =  (TC_HCB *) TCD_Current_Thread@

	LDR     R0, =TCD_Current_Thread
	LDR     R5, [R0, #0] 					@ Pickup control block pointer
TCT_HISR_Loop:

@@@ Call the HISR's entry routine.  
@@@ (*(REG_HISR_Ptr -> tc_entry)) ()@

	@[ CODE_32

	MOV     LR, PC     	             		@ Setup return value
    LDR     pc, [R5, #0x44]               	@ Call HISR entry function

	@|
@	LDR	R4, [R5, #0x44]						@ Get HISR entry function
@	TST	R4, #1								@ See if calling Thumb or ARM
@	BNE	Thumb	
@	MOV	lr,pc
@	BX	R4
@	B	ARMCODE

@Thumb :
@
@	LDR		lr, =ThumbAfterHisr + 1
@	BX		R4

@.CODE 16	

@ThumbAfterHisr:
@
@	LDR     R1, =ARMCODE
@	BX      R1

@.CODE 32

	@]

@@@ Lockout interrupts.  

ARMCODE:
        MRS     R1, CPSR          			@ Pickup current CPSR
        ORR     R1, R1, #LOCKOUT          	@ Build interrupt lockout 
        MSR     cpsr_cxsf, R1   				@ Lockout interrupts

@@@ On return, decrement the activation count and check to see if it is 0.
@@@ Once it reaches 0, the HISR should be made inactive. 
@@@ REG_HISR_Ptr -> tc_activation_count--@

	LDR     R0, [R5, #0x40]			@ Pickup current activation count
	SUBS    R0, R0, #1                	@ Subtract and set condition codes
	STR     R0, [R5, #0x40]          	@ Store new activation count
	BEQ     TCT_HISR_Finished         	@ Finished processing HISR

@ Restore interrupts.  

	LDR     R2, =TCD_Interrupt_Level
	MRS     R1, CPSR	                @ Pickup current CPSR
	LDR     R3, [R2, #0]			@ Pickup interrupt lockout level
	BIC     R1, R1, #LOCK_MSK         	@ Clear lockout bits
	ORR     R1, R1, R3			@ Build new interrupt lockout
	MSR     cpsr_cxsf, R1   			@ Setup CPSR appropriately
	
	B       TCT_HISR_Loop			@ Return to HISR loop

@ while (REG_HISR_Ptr -> tc_activation_count)@

TCT_HISR_Finished:

@ At this point, the HISR needs to be made inactive.  

@ Determine if this is the only HISR on the given priority list. 
@ if (REG_HISR_Ptr == TCD_Active_HISR_Tails[REG_HISR_Ptr -> tc_priority])

	LDR     lr, =TCD_Active_HISR_Tails
	LDRB    R3, [R5, #0x1a]             	@ Pickup priority
	LDR     R6, [lr, R3, LSL #2]           	@ Pickup this priority tail pointer
	LDR     R2, =TCD_Execute_HISR
	MOV     IP, #0							@ Clear ip
	LDR     R1, =TCD_Active_HISR_Heads 
	CMP     R6, R5 							@ Is this priority tail the same as
											@ the current HISR?
    BNE     TCT_More_HISRs 				@ If not, more HISRs at this 
     	                                        @ priority
@ The only HISR on the list.  Clean up the list and check for the highest priority HISR.
@ TCD_Active_HISR_Heads[REG_HISR_Ptr -> tc_priority] =  NU_NULL@
@ TCD_Active_HISR_Tails[REG_HISR_Ptr -> tc_priority] =  NU_NULL@

	STR     IP, [R1, R3, LSL #2]			@ Set head pointer to NU_NULL
	STR     IP, [lr, R3, LSL #2]			@ Set tail pointer to NU_NULL

@ Determine the highest priority HISR.  
@ if (TCD_Active_HISR_Heads[0])
@ TCD_Execute_HISR =  TCD_Active_HISR_Heads[0]@
@ else if (TCD_Active_HISR_Heads[1])
@ TCD_Execute_HISR =  TCD_Active_HISR_Heads[1]@
@ else
@ TCD_Execute_HISR =  TCD_Active_HISR_Heads[2]@

	LDR     R3, [R1, #0]                 	@ Pickup priority 0 head pointer
	CMP     R3, #0                    		@ Is there an HISR active?
	LDREQ   R3, [R1, #4]          			@ If not, pickup priority 1 pointer
	CMPEQ   R3, #0                    		@ Is there an HISR active?
	LDREQ   R3, [R1, #8] 					@ If not, pickup priority 2 pointer
	STR     R3, [R2, #0] 					@ Setup execute HISR pointer
	
	B       TCT_HISR_Exit               	@ Exit HISR processing
TCT_More_HISRs:

@ Move the head pointer to the next HISR in the list.  
@ TCD_Active_HISR_Heads[REG_HISR_Ptr -> tc_priority] =  
@ REG_HISR_Ptr -> tc_active_next@

@ Also set the TCD_Execute_HISR pointer.  
@ TCD_Execute_HISR =  REG_HISR_Ptr -> tc_active_next@

	@[ CODE_16 

	@LDR	LR, [R5, #0x3c] 					@ Pickup next HISR to activate
	@|

	LDR     LR, [R5, #0x3c]    	        	@ Pickup next HISR to activate
	@]       

	STR     LR, [R1, R3, LSL #2]   			@ Setup new head pointer
	STR     LR, [R2, #0]           			@ Setup execute HISR pointer

TCT_HISR_Exit:

@ Build fake return to the top of this loop.  The next time the HISR
@ is activated, it will return to the top of this function.  

	LDR     LR, =TCT_HISR_Shell
	STMDB   sp!,{R4 - ip, lr}         		@ Save minimal context of thread on
                      		 				@ the current stack
	MOV     R2, #0 	                  		@ Build solicited stack type value
	                                        @ and NU_NULL value
	@[ CODE_16 

	@STR     R2, [sp, #-4]! 					@ Save state mask
	
	@]
	
	STR     R2, [sp, #-4]! 					@ Place it on the top of the stack	

@ Clear the current thread control block.  
@ TCD_Current_Thread =  NU_NULL@

	LDR     R1, =TCD_Current_Thread
	STR     R2, [R1, #0]            		@ Set current thread pointer to
											@ NU_NULL
						
@ Save off the current stack pointer in the control block.  
@ REG_HISR_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr@

	@[ CODE_16 

	@STR     sp, [R5, #0x2c]					@ Save the thread's stack pointer
	@|
 	STR     sp, [R5, #0x2c]					@ Save the thread's stack pointer
     
	@]

@ Switch to the system stack.  
@ REG_Stack_Ptr =  (BYTE_PTR) TCD_System_Stack@

	LDR     R1, =TCD_System_Stack
	LDR     R2, =TCT_System_Limit
	LDR     SP, [R1, #0]					@ Switch to system stack
	LDR     R10,[R2, #0]					@ Setup system stack limit

@ Transfer control to the main scheduling loop.

	B	TCT_Schedule						@ Return to main scheduling loop


@###############################################################################
@	.data
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global TCT_System_Limit
TCT_System_Limit:
        .word     0x0		@&00000000


@ Define external inner-component global data references.
@	extern TC_TCB          *TCD_Execute_Task@
@	extern TC_HCB          *TCD_Execute_HISR@
@	extern VOID            *TCD_Current_Thread@
@	extern VOID            *TCD_System_Stack@
@	extern INT              TCD_Interrupt_Count@
@	extern TC_HCB          *TCD_Active_HISR_Heads[TC_HISR_PRIORITIES]@
@	extern TC_HCB          *TCD_Active_HISR_Tails[TC_HISR_PRIORITIES]@
@	extern INT              TCD_Interrupt_Level@
@	extern UNSIGNED         TMD_Time_Slice@
@	extern INT              TMD_Time_Slice_State@
#if 0
	.extern		TCD_Execute_Task
	.extern		TCD_Execute_HISR
	.extern		TCD_Current_Thread
	.extern		TCD_System_Stack
	.extern		TCD_Interrupt_Count
	.extern     	TCD_Active_HISR_Heads
	.extern		TCD_Active_HISR_Tails
	.extern		TCD_Interrupt_Level
	.extern		TMD_Time_Slice
	.extern		TMD_Time_Slice_State
#endif
@ Define internal function references.

@	VOID    TCC_Task_Shell(VOID)@
@	VOID    TCC_Signal_Shell(VOID)@
@	VOID    TCT_HISR_Shell(VOID)@
@	VOID    ERC_System_Error(INT error)@


@	VOID	TCT_Schedule_Protected(VOID *thread)@ 
@ Define pointers to system variables so their addresses may be obtained in a
@ pc-relative manner.  

#if 1
.equ	System_Limit	,	TCT_System_Limit
.equ	Int_Level		,	TCD_Interrupt_Level
.equ	Current_Thread	,		TCD_Current_Thread
.equ	Execute_HISR	,		TCD_Execute_HISR
.equ	Execute_Task	,		TCD_Execute_Task
.equ	Time_Slice		,		TMD_Time_Slice
.equ	Slice_State		,		TMD_Time_Slice_State
.equ	System_Stack 	,	  	TCD_System_Stack
.equ	Int_Count		,		TCD_Interrupt_Count
.equ	HISR_Tails		,		TCD_Active_HISR_Tails
.equ	HISR_Heads		,		TCD_Active_HISR_Heads

#else
.equ	System_Limit	,	TCT_System_Limit
.equ	Int_Level		,	TCD_Interrupt_Level
.equ	Current_Thread	,		TCD_Current_Thread
.equ	Execute_HISR	,		TCD_Execute_HISR
.equ	Execute_Task	,		TCD_Execute_Task
.equ	Time_Slice		,		TMD_Time_Slice
.equ	Slice_State		,		TMD_Time_Slice_State
.equ	System_Stack 	,	  	TCD_System_Stack
.equ	Int_Count		,		TCD_Interrupt_Count
.equ	HISR_Tails		,		TCD_Active_HISR_Tails
.equ	HISR_Heads		,		TCD_Active_HISR_Heads
#endif
@@@@ End of tct1.S @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@.end 
