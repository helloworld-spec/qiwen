/*************************************************************************/
/*                                                                       */
/*               Copyright Mentor Graphics Corporation 2002              */
/*                         All Rights Reserved.                          */
/*                                                                       */
/* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS  */
/* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS   */
/* SUBJECT TO LICENSE TERMS.                                             */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                               VERSION       */
/*                                                                       */
/*      tc_extr.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TC - Thread Control                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes of all functions          */
/*      accessible to other components.                                  */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      tc_defs.h                           Thread Control constants     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Added validation utility for                     */
/*                      error checking functions for                     */
/*                      resume service and resume                        */
/*                      driver, resulting in                             */
/*                      version 1.0a                                     */
/*      03-01-1994      Verified version 1.0a                            */
/*      03-01-1994      Moved include files outside of                   */
/*                      the file #ifndef to allow the                    */
/*                      use of actual data structures,                   */
/*                      added system protect pseudo                      */
/*                      function, added set suspend                      */
/*                      protect pseudo function,                         */
/*                      added optimization for targets                   */ 
/*                      able to access pointers in a                     */
/*                      single instruction, changed                      */
/*                      function names due to changes                    */
/*                      in file structure, resulting                     */
/*                      in version 1.1                                   */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*      01-24-1996      Corrected a type on line 200:                    */
/*                      TCT_Control_Interrupts,                          */
/*                      resulting in version 1.1+                        */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/

#include        "tc_defs.h"                 /* Include TC constants      */


/* Check to see if the file has been included already.  */

#ifndef TC_EXTR
#define TC_EXTR


/* Define external reference for the system protection.   */

extern TC_PROTECT TCD_System_Protect;
extern VOID    *TCD_Current_Thread;


/* Initialization functions.  */

VOID            TCI_Initialize(VOID);


/* Core error checking functions.  */

#ifndef NU_NO_ERROR_CHECKING
BOOLEAN         TCC_Validate_Resume(OPTION resume_type, NU_TASK *task_ptr);
#endif

STATUS          TCCE_Activate_HISR(NU_HISR *hisr_ptr);
STATUS          TCCE_Create_Task(NU_TASK *task_ptr, CHAR *name, 
                        VOID (*task_entry)(UNSIGNED, VOID *), UNSIGNED argc,
                        VOID *argv, VOID *stack_address, UNSIGNED stack_size,
                        OPTION priority, UNSIGNED time_slice, 
                        OPTION preempt, OPTION auto_start);
STATUS          TCCE_Reset_Task(NU_TASK *task_ptr, UNSIGNED argc, VOID *argv);
STATUS          TCCE_Create_HISR(NU_HISR *hisr_ptr, CHAR *name,
                        VOID (*hisr_entry)(VOID), OPTION priority,
                        VOID *stack_address, UNSIGNED stack_size);
STATUS          TCCE_Terminate_Task(NU_TASK *task_ptr);
STATUS          TCCE_Delete_Task(NU_TASK *task_ptr);
STATUS          TCCE_Delete_HISR(NU_HISR *hisr_ptr);
STATUS          TCCE_Resume_Service(NU_TASK *task_ptr);
STATUS          TCCE_Suspend_Service(NU_TASK *task_ptr);
VOID            TCCE_Relinquish(VOID);
VOID            TCCE_Task_Sleep(UNSIGNED ticks);
STATUS          TCCE_Validate_Resume(OPTION resume_type, NU_TASK *task_ptr);
INT             TCCE_Suspend_Error(VOID);
STATUS          TCFE_Task_Information(NU_TASK *task_ptr, CHAR *name,
                    DATA_ELEMENT *status, UNSIGNED *scheduled_count,
                    DATA_ELEMENT *priority, OPTION *preempt, UNSIGNED *time_slice,
                    VOID **stack_base, UNSIGNED *stack_size, 
                    UNSIGNED *minimum_stack);


/* Supplemental error checking functions.  */

OPTION          TCSE_Change_Priority(NU_TASK *task_ptr, OPTION new_priority);
OPTION          TCSE_Change_Preemption(OPTION preempt);
UNSIGNED        TCSE_Change_Time_Slice(NU_TASK *task_ptr, UNSIGNED time_slice);
UNSIGNED        TCSE_Control_Signals(UNSIGNED signal_enable_mask);
UNSIGNED        TCSE_Receive_Signals(VOID);
STATUS          TCSE_Register_Signal_Handler(VOID (*signal_handler)(UNSIGNED));
STATUS          TCSE_Send_Signals(NU_TASK *task_ptr, UNSIGNED signals);


/* Core processing functions.  */

STATUS          TCT_Activate_HISR(NU_HISR *hisr_ptr);
STATUS          TCC_Create_Task(NU_TASK *task_ptr, CHAR *name, 
                        VOID (*task_entry)(UNSIGNED, VOID *), UNSIGNED argc,
                        VOID *argv, VOID *stack_address, UNSIGNED stack_size,
                        OPTION priority, UNSIGNED time_slice, 
                        OPTION preempt, OPTION auto_start);
STATUS          TCC_Reset_Task(NU_TASK *task_ptr, UNSIGNED argc, VOID *argv);
STATUS          TCC_Create_HISR(NU_HISR *hisr_ptr, CHAR *name,
                        VOID (*hisr_entry)(VOID), OPTION priority,
                        VOID *stack_address, UNSIGNED stack_size);
STATUS          TCC_Terminate_Task(NU_TASK *task_ptr);
STATUS          TCC_Delete_Task(NU_TASK *task_ptr);
STATUS          TCC_Delete_HISR(NU_HISR *hisr_ptr);
STATUS          TCC_Resume_Task(NU_TASK *task_ptr, OPTION suspend_type);
STATUS          TCC_Resume_Service(NU_TASK *task_ptr);
VOID            TCC_Suspend_Task(NU_TASK *task_ptr, OPTION suspend_type,
                                VOID (*cleanup)(VOID *), 
                                VOID *information, UNSIGNED timeout);
STATUS          TCC_Suspend_Service(NU_TASK *task_ptr);
VOID            TCC_Task_Timeout(NU_TASK *task_ptr);
VOID            TCC_Task_Sleep(UNSIGNED ticks);
VOID            TCC_Relinquish(VOID);
VOID            TCC_Time_Slice(NU_TASK *task_ptr);
NU_TASK        *TCC_Current_Task_Pointer(VOID);
NU_HISR        *TCC_Current_HISR_Pointer(VOID);
VOID            TCC_Dispatch_LISR(INT vector);
STATUS          TCC_Register_LISR(INT vector, VOID (*new_lisr)(INT), 
                                        VOID (**old_lisr)(INT));


/* Supplemental functions.  */

OPTION          TCS_Change_Priority(NU_TASK *task_ptr, OPTION new_priority);
OPTION          TCS_Change_Preemption(OPTION preempt);
UNSIGNED        TCS_Change_Time_Slice(NU_TASK *task_ptr, UNSIGNED time_slice);
UNSIGNED        TCS_Control_Signals(UNSIGNED signal_enable_mask);
UNSIGNED        TCS_Receive_Signals(VOID);
STATUS          TCS_Register_Signal_Handler(VOID (*signal_handler)(UNSIGNED));
STATUS          TCS_Send_Signals(NU_TASK *task, UNSIGNED signals);


/* Information retrieval functions.  */

UNSIGNED        TCF_Established_Tasks(VOID);
UNSIGNED        TCF_Established_HISRs(VOID);
STATUS          TCF_Task_Information(NU_TASK *task, CHAR *name, 
                        DATA_ELEMENT *status, UNSIGNED *scheduled_count,
                        OPTION *priority, OPTION *preempt,
                        UNSIGNED *time_slice, VOID **stack_base,
                        UNSIGNED *stack_size, UNSIGNED *minimum_stack);
STATUS          TCF_HISR_Information(NU_HISR *hisr, CHAR *name,
                        UNSIGNED *scheduled_count, DATA_ELEMENT *priority,
                        VOID **stack_base, UNSIGNED *stack_size,
                        UNSIGNED *minimum_stack);
UNSIGNED        TCF_Task_Pointers(NU_TASK **pointer_list, 
                                                UNSIGNED maximum_pointers);
UNSIGNED        TCF_HISR_Pointers(NU_HISR **pointer_list,
                                                UNSIGNED maximum_pointers);
                                        
/* Target dependent functions.  */

UNSIGNED        TCT_Check_Stack(VOID);
VOID            TCT_Schedule(VOID);
VOID            TCT_Restore_Interrupts(VOID);
VOID            TCT_Protect(TC_PROTECT *protect);
VOID            TCT_Unprotect(VOID);
VOID            TCT_Unprotect_Specific(TC_PROTECT *protect);
VOID            TCT_Set_Current_Protect(TC_PROTECT *protect);
VOID            TCT_Control_To_System(VOID);
INT             TCT_Control_Interrupts(INT new_level);
INT             TCT_Local_Control_Interrupts(INT new_level);

#ifdef NU_ENABLE_STACK_CHECK
#define NU_CHECK_STACK() TCT_Check_Stack()
#else /* no stack checking */
#define NU_CHECK_STACK() ((void)0)
#endif /* NU_ENABLE_STACK_CHECK */


/* Determine if pointers are accessible with a single instruction.  If so,
   just reference the pointer directly.  Otherwise, call the assembly 
   service.  */

#if             NU_POINTER_ACCESS == 1
extern TC_TCB  *TCD_Execute_Task;
#define         TCT_Current_Thread()        TCD_Current_Thread
#define         TCT_Set_Execute_Task(task)  TCD_Execute_Task =  task
#else
VOID           *TCT_Current_Thread(VOID);
VOID            TCT_Set_Execute_Task(TC_TCB *task);
#endif

/* Define pseudo function used by internal components of the system.  */

#define         TCT_System_Protect()        TCT_Protect(&TCD_System_Protect)
#define         TCT_System_Unprotect()      \
                                TCT_Unprotect_Specific(&TCD_System_Protect)
#define         TCT_Set_Suspend_Protect(protect)  \
                   ((TC_TCB *) TCD_Current_Thread) -> tc_suspend_protect = \
                   (TC_PROTECT *) protect
#define         TCT_Get_Current_Protect()   \
                   ((TC_TCB *) TCD_Current_Thread) -> tc_current_protect
#define         TCC_Task_Priority(task)     ((TC_TCB *) (task)) -> tc_priority
#endif





