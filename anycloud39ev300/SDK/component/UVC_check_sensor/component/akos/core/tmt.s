@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@		Avalent Technologes, Inc. 		02. 07. 2001	
@@@	File Name :	TMT.S	
@@@ 	Component :	TM - Timer Management	
@@@
@@@	Description :
@@@		This file contaims the target dependent routines of the timer  	
@@@		management component.	
@@@
@@@	Author : Yean-sun Park,  Avalent Technologes, Inc. 
@@@
@@@	Data Structure	: None	
@@@	Function :
@@@		TMT_Set_Clock			- set system clock
@@@		TMT_Retrieve_Clock		- retrive system clock
@@@		TMT_Read_Timer			- read count-down timer	
@@@		TMT_Enable_Timer		- enable count-down timer
@@@		TMT_Adjust_Timer		- Adjust count-down timer
@@@		TMT_Disable_Timer		- Disable count-down timer
@@@		TMT_Retrieve_TS_Task		- Retrieve time-sliced task ptr
@@@		TMT_Timer_Interrupt		- Process timer interrupt
@@@		Dependencies:
@@@		tc_extr.h			- Thread Control functions 
@@@		tm_extr.h			- Timer functions
@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@		.TEXT	

@@@	.define	NU_SOURCE_FILE
@@@
@@@	.include	"tc_extr.h"		@@@ tread control function
@@@	.include	"tm_extr.h" 	@@@ timer function

#include	"boot.h"

@begin asm execute TAKTMT

@	AREA	TAKTMT, CODE, READONLY	
.TEXT
	
@@@ Define constants used in low-level initialization.  

@@@ Define activate HISR function.
@@@ STATUS	TCT_Activate_HISR(TC_HCB *hisr)@

	.extern	TCT_Activate_HISR
	.extern  AK_Kick_Watchdog
@wmj replace __rt_udiv to div
@	.extern  __rt_udiv
	.extern  __aeabi_uidiv

@@@ Define various data structure pointers so their address can be obtained
@@@ in a PC_relative manner

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@ 	TMT_Set_Clock:
@@@	This function sets the system clock to the specified value.
@@@	TMD_System_Clock = new_value@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global		TMT_Set_Clock

TMT_Set_Clock:
@@@ Set the system clock to the specified value. TMD_System_Clock = new_value@
	LDR	R1, =TMD_System_Clock 		@@@ Build address of system clock
	STR	R0, [R1, #0]
@@@
	BX	LR

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 
@@@
@@@ 	TMT_Retrieve_Clock :
@@@	This function returns the current value of the 
@@@	system clock.
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global		TMT_Retrieve_Clock

TMT_Retrieve_Clock:

@@@ Return the current value of the system clock. return(TMD_System_Clock)@
	
	LDR	R0, =TMD_System_Clock
	LDR	R0, [R0, #0]

	BX	LR

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@ 
@@@	TMT_Read_Timer: 
@@@	
@@@	Called By:	TMC_Start_Timer
@@@	This function returns the current value of the countdown timer
@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.global		TMT_Read_Timer
	
TMT_Read_Timer:

@@@ Return the current value of the count-down timer. return(TMD_Timer)@
		
	LDR	R0, =TMD_Timer			@@@ Build address to timer
	LDR	R0, [R0, #0]		@@@ Pickup timer contents

	BX	LR

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@ 
@@@ 	TMT_Enable_Timer: 
@@@	Called by : TMC_Start_Timer, TMC_Timer_Task	
@@@	This function enables the count- down timer with 
@@@	specified value. ( TMD_Timer = Time )	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 

.global		TMT_Enable_Timer

TMT_Enable_Timer:

@@@ Place the new time value into the count-down timer.  
@@@ TMD_Timer =  time@

	LDR	R1, =TMD_Timer
	STR	R0, [R1, #0]				@@@ Store new timer value

@@@ Indicate that timer is active. TMD_Timer_State = TM_ACTIVE@
	MOV	R0, #0						@@@ Build TM_ACTIVE value
	LDR	R1, =TMD_Timer_State			@@@ Build address of timer state var
	STR	R0, [R1, #0]				@@@ Change the state to active

	BX	LR

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 
@@@ 
@@@	TMT_Adjust_Timer:
@@@	This function adjusts the count down timer with the
@@@	specified value, if the new value is less than the current.
@@@
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		
.global		TMT_Adjust_Timer

TMT_Adjust_Timer:

@@@ Lockout all interrupts. TMD_Timer_State =  TM_NOT_ACTIVE@

	MRS	R2, cpsr				@@@ Pickup current CPSR
	ORR	R2, R2, #I_Bit			@@@ Build  lockout CPSR
	MSR	cpsr_cxsf, R2			@@@ Setup new CPSR interrupt
	
@@@ check for the new value is less than the current time value 
@@@ if (time < TMD_Timer)
	
	LDR	R1, =TMD_Timer				@@@ Build address to timer var 	
	LDR	R2, [R1, #0]			@@@ read value of the timer value
	CMP	R2, R0					@@@ Do Timer- Timer > 0, means

	BLT	TMT_No_Adjust			@@@ time < Timer

@@@ adjust the time, TMD_Timer = time@

	STR	R0, [R1, #0]			@@@ load passed in timer value

@@@ return to caller after restoring interrupts
		
TMT_No_Adjust:
	LDR	R1, =TCD_Interrupt_Level				
	LDR	R1, [R1, #0]			@@@ read the interrupt level
	MRS	R2, cpsr				@@@ Pick up current CPSR
	BIC	R2, R2, #LOCK_MSK		@@@ Clear lockout mask
	ORR	R2, R2, R1				@@@ Build new CPSR with

	MSR	cpsr_cxsf, R2   			@@@ Setup new CPSR enable bits
	
	BX	LR

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@ 	TMT_Disable_Timer:
@@@	Called by : TMC_Start_Timer, TMC_Timer_Task
@@@	This function disables the count_down timer.
@@@				   	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global		TMT_Disable_Timer

TMT_Disable_Timer:

@@@ Disable the count-down timer. TMD_Timer_State =  TM_NOT_ACTIVE@

	MOV	R1, #1				@@@ build TM_NOT_ACTIVE value
	LDR	R0, =TMD_Timer_State	@@@ build address to timer state var
	STR	R1, [R0, #0]		@@@ Change timer state to not active

	BX	LR
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@ 	TMT_Retreive_TS_Timer:
@@@	Called by	: TMC_Timer_HISR  	
@@@	OutPuts 	: TMD_Time_Slice_Task	
@@@	This function returns the time-sliced task pointer.
@@@				   	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 

.global		TMT_Retrieve_TS_Task

TMT_Retrieve_TS_Task:

@@@ Read the current TMD_Time_Slice_Task variable and load for return to caller.

	LDR	R1, =TMD_Time_Slice_Task		
	LDR	R0, [R1, #0]		@@@ Get task pointer to be returned
@@@ return to caller time slice value back to caller

	BX	LR
	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@
@@@  	TMT_Timer0_Interrupt
@@@ 	 	This function processes the actual hardware interrupt. Processing includes
@@@ 	 	updating the system clock and the count-down timer and time-slice timer. 
@@@ 	 	If one or both of the timers expire, the timer HISR is activated.
@@@	Called by:	Interrupt Vector
@@@	Calls	 :	TCT_Activate_HISR
@@@		 :	TCT_Interrupt_Context_Save
@@@		 :	TCT_Interrupt_Context_Restore
@@@	
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.global		TMT_Timer0_Interrupt

TMT_Timer0_Interrupt:

@@@ It is assumed that interrupts are locked out by the caller of this funtion.
@@@ Increment the system clock.   TMD_System_Clock++

    LDR     R0, =TCD_Interrupt_Count	 	@@@ Pickup address of interrupt count
    LDR     R1, [R0, #0]             		@@@ Pickup interrupt counter
    CMP     R1, #1                   		@@@ Is it nested?
        
    BGT     TMT_Nested_Interrupt        	@@@ Yes
    B       TMT_Not_Nested_Interrupt    	@@@ No

TMT_Nested_Interrupt:
@@@ lockout all interrupts
        
     MRS     R1, cpsr              			@@@ Pickup current CPSR
     ORR     R1, R1, #I_Bit             	@@@ Build lockout value
     MSR     cpsr_cxsf, R1                	@@@ Lockout interrupts

TMT_Not_Nested_Interrupt:
	LDR	R0, =TMD_System_Clock					@@@ Pickup system clock address
	LDR	R1, [R0, #0]						@@@ Pickup system clock contents
	ADD	R1, R1, #1							@@@ Increment system clock
	STR	R1, [R0, #0]						@@@ Storen new system clock value

@@@ Kick the watch dog.....  if ((TMD_System_Clock%200) == 0) per second(Timer interrupt interval 5 ms).
    STR	lr, [sp, #-4]!	            		@@@ Save lr on the stack
    MOV R0,#0xC8
@wmj replace with div
@    BL  __rt_udiv                           @@@ R1%R0 -> R1
    BL  __aeabi_uidiv                           @@@ R1%R0 -> R1
    CMP R1,#0
    BLEQ AK_Kick_Watchdog	                @@@ watchdog counter-- and check it.
    LDR	lr, [sp], #4	            		@@@ Recover return address

@@@ Determine if the count-down timer is actived. TMD_Timer_State == TM_ACTIVE 

	LDR	R1, =TMD_Timer_State
	LDR	R0, [R1, #0]						@@@ Pickup timer sate
	MOV	R3, #2								@@@ Build expired value
	CMP	R0, #0								@@@ Is there a timer active ?

	BNE	TMT_No_Timer_Active					@@@ No, skip timer processing

@@@ Decrement the count-down timer. TMD_Timer--@

	LDR	R0, =TMD_Timer							@@@ Build timer address	
	LDR	R2, [R0, #0]						@@@ Pickup the current timer value
	CMP	R2, #0
	BEQ	EXPIRED
											@@@ Decrement the count- down timer
	SUBS	R2, R2, #1						@@@ Decrement the timer value
	CMP	R2, #0
	BEQ	EXPIRED
	STR	R2, [R0, #0]						@@@ Store the new timer value

	B	TMT_No_Timer_Active

@@@ Determine if the timer has expired. If so, modify the state to indicate
@@@ that it has expired. if (TMD_Timer == 0)
@@@ TMD_Timer_State =  TM_EXPIRED@
						@@@ If TMD_Timer_state == 0
EXPIRED:		
	STREQ	R3, [R1, #0]					@@@ Change the timer state to  expired	

TMT_No_Timer_Active:

@@@ Determine if the time-slice timer is active.
@@@ Note that the parameters for the time-slice are controlled by Thread Control(TC)component
@@@ TMD_Time_Slice_State == TM_ACTIVE.

	LDR	R0, =TMD_Time_Slice_State					@@@ Build time slice state address 
	LDR	R2, [R0, #0]   						@@@ Pickup time slice state
	CMP	R2, #0								@@@ Is there a time slice active?
	
	BNE	TMT_No_Time_Slice_Active			@@@ No, skip time slice.

@@@ Decrement the time slice counter. TMD_Time_Slice--@
	
	LDR	R2, =TMD_Time_Slice
	LDR	R3, [R2, #0]				@@@ Pickup the time slice value
	SUBS	R3, R3, #1				@@@ Decrement the time slice value
	STR	R3, [R2, #0]				@@@ Store the new time slice.
								
@@@ Determine if the time-slice timer has expired. If so, modify the time_slice
@@@ state to indicate that it has.  if (TMD_Time_Slice == 0)

	BNE	TMT_No_Time_Slice_Active  	@@@ Has time  slice expired?
	
@@@ TMD_Time_Slice_State = TM_EXPIRED@		
	MOV	R3, #2						@@@ Build TM_EXPIRED value.
	STR	R3, [R0, #0]				@@@ Indicate time slice is expired.
@@@ Copy the  current thread into the time-slice task pointer. 
@@@ TMD_Time_Slice_Task = TCD_Current_Thread.
	
	LDR	R2, =TCD_Current_Thread	 		@@@ Pickup current thread pointer adr
	LDR	R2, [R2, #0]				@@@ Pickup current thread pointer
	LDR	R3, =TMD_Time_Slice_Task   	 		@@@ Pickup time slice task pointer adr
	STR	R2, [R3, #0]		 		@@@ Store current thread pointer
	
@@@ ((TC_TCB *) TCD_Current_Thread) -> tc_cur_time_slice =  1@
	MOV	R3, #1			 			@@@ For safety, place a minimal time slice
	STR	R3, [R2, #0x20]!         	@@@ into  the Task's control block.

TMT_No_Time_Slice_Active:

@@@ Determine if either of the basic timers have expired. If so, activate the 
@@@ timer HISR. if((TMD_Timer_State==TM_EXPIRED)||(TMD_Time_Slice_State==TM_EXPIRED))

	LDR	R1, [R1, #0]
	CMP	R1, #2						@@@ Does it indicate expiration ?
	LDRNE	R0, [R0, #0]			@@@ Pickup time slice state
	CMPNE	R0, #2					@@@ Does it indicate expiration ?
@@@ unlock to allow for nested interrupts
@@@	MRS	R0, cpsr					@@@ Pickup current CPSR
@@@	BIC	R0, R0, #LOCKOUT			@@@ Clear the interrupt lockout bits
@@@	MSR	cpsr_all, R0				

	BXNE	lr						@@@ return if no expirration

@@@ Active the HISR timer function. TCT_Activate_HISR(&TMD_HISR)@

	STR	lr, [sp, #-4]!				@@@ Save lr on the stack
	LDR	R0,  =TMD_HISR					@@@ Build address of timer HISR
	BL	TCT_Activate_HISR			@@@ Activate timer HISR
	
	LDR	lr, [sp], #4				@@@ Recover return address

	BX	LR

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@@@##############################################################################
@@@  Read Write Data. Define external inner-component global data references.  
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

@@@extern VOID           *TCD_Current_Thread@
@@@extern UNSIGNED        TMD_System_Clock@
@@@extern UNSIGNED        TMD_Timer@
@@@extern INT             TMD_Timer_State@
@@@extern UNSIGNED        TMD_Time_Slice@
@@@extern TC_TCB         *TMD_Time_Slice_Task@
@@@extern INT             TMD_Time_Slice_State@
@@@extern TC_HCB          TMD_HISR@

	.extern		TCD_Current_Thread
	.extern		TMD_System_Clock
	.extern		TMD_Timer
	.extern		TMD_Timer_State
	.extern		TMD_Time_Slice
	.extern 		TMD_Time_Slice_Task
	.extern		TMD_Time_Slice_State
	.extern		TMD_HISR
	.extern		TCD_Interrupt_Level
	.extern		TCD_Interrupt_Count
@@@

.equ  Current_Thread,			TCD_Current_Thread
.equ  System_Clock	,		TMD_System_Clock
.equ  Timer			,		TMD_Timer
.equ  Timer_State		,		TMD_Timer_State
.equ  Slice_State		,		TMD_Time_Slice_State
.equ  Time_Slice		,		TMD_Time_Slice
.equ  Slice_Task		,		TMD_Time_Slice_Task
.equ  HISR			,		TMD_HISR
.equ  Int_Level		,		TCD_Interrupt_Level
.equ  Int_Count		,		TCD_Interrupt_Count

@@@@@ end of tmt1.s @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@.end
