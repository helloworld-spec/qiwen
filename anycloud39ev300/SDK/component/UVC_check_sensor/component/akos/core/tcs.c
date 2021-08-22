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
/*      tcs.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TC - Thread Control                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains supplemental routines for the Thread Control  */
/*      component.                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      TCS_Change_Priority                 Change task's priority       */
/*      TCS_Change_Preemption               Change task's preemption     */
/*      TCS_Change_Time_Slice               Change task's time-slice     */
/*      TCS_Control_Signals                 Control signals              */
/*      TCS_Receive_Signals                 Receive signals              */
/*      TCS_Register_Signal_Handler         Register signal handler      */
/*      TCS_Send_Signals                    Send signals to a task       */
/*                                                                       */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      in_extr.h                           Initialization/Interrupt     */
/*                                            functions                  */
/*      tm_extr.h                           Timer Control function       */
/*      er_extr.h                           Error handling function      */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1994      Created initial version 1.1 from                 */
/*                      previous version of TCC.C                        */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-04-1996      Modified TCS_Send_Signals,                       */
/*                      resulting in version 1.1+                        */
/*                      (spr 107)                                        */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "in_extr.h"                 /* Initialization/Interrupt  */
                                            /*   functions               */
#include        "tm_extr.h"                 /* Timer control functions   */
#include        "er_extr.h"                 /* Error handling function   */
#include        "hi_extr.h"                 /* History functions         */
#include        "profiler.h"                /* ProView interface         */


/* Define external inner-component global data references.  */

extern CS_NODE         *TCD_Created_Tasks_List;
extern TC_TCB          *TCD_Priority_List[TC_PRIORITIES];
extern UNSIGNED         TCD_Priority_Groups;
extern DATA_ELEMENT     TCD_Sub_Priority_Groups[TC_MAX_GROUPS];
extern UNSIGNED_CHAR    TCD_Lowest_Set_Bit[];
extern INT              TCD_Highest_Priority;
extern TC_TCB          *TCD_Execute_Task;
extern VOID            *TCD_Current_Thread;
extern TC_PROTECT       TCD_System_Protect;
extern INT              TMD_Time_Slice_State;



/* Define external inner-component function calls that are not available to
   other components.  */

VOID            TCT_Build_Signal_Frame(TC_TCB *task_ptr);
VOID            TCT_Protect_Switch(TC_TCB *task);
VOID            TCT_Signal_Exit(VOID);


/* Define internal function calls.  */

VOID            TCC_Signal_Shell(VOID);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCS_Change_Priority                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function changes the priority of the specified task.  The   */
/*      priority of a suspended or a ready task can be changed.  If the  */
/*      new priority necessitates a context switch, control is           */
/*      transferred back to the system.                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCSE_Change_Priority                Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Protect                         Protect scheduling data      */
/*      TCT_Set_Execute_Task                Set TCD_Execute_Task pointer */
/*      TCT_Unprotect                       Release protection of data   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*      new_priority                        New priority for task        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      old_priority                        Original task priority       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      modified protection logic,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*      10-4-1999       Bug fixes - return if new                        */
/*                      priority equals old priority                     */
/*                      and don't move the head pointer                  */
/*                      unless the head node is changing                 */
/*************************************************************************/
OPTION   TCS_Change_Priority(NU_TASK *task_ptr, OPTION new_priority)
{

R1 TC_TCB      *task;                       /* Task control block ptr    */
R2 TC_TCB      *head;                       /* Head list pointer         */
R3 INT          index;                      /* Working index variable    */
OPTION          old_priority;               /* Previous priority of task */
DATA_ELEMENT    temp;                       /* Temporary variable        */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CHANGE_PRIORITY_ID, (UNSIGNED) task,
                                        (UNSIGNED) new_priority, (UNSIGNED) 0);

#endif

    /* Protect against multiple access to the scheduling list.  */
    TCT_Protect(&TCD_System_Protect);

    /* Save the old priority of the task.  */
    old_priority =  task -> tc_priority;


    /* BUG FIX  this should probably go into an error checking routine instead of here  */
    if (!(task -> tc_priority == new_priority))
    {


       /* Check to see if the task is currently ready.  */
       if (task -> tc_status == NU_READY)
       {

           /* Remove the task from the ready list.  */

           /* Determine if the task is the only one on the list.  */
           if (task -> tc_ready_next == task)
           {

               /* Only task on the list.  Clear the task's pointers and
               clear the entry in the priority table.  */
               task -> tc_ready_next =        NU_NULL;
               task -> tc_ready_previous =    NU_NULL;
               *(task -> tc_priority_head) =  NU_NULL;

               /* Clear the sub-priority group.  */
               *(task -> tc_sub_priority_ptr) =
                  *(task -> tc_sub_priority_ptr) & ~(task -> tc_sub_priority);

               /* Determine if the main priority group needs to be cleared.
                  This is only true if there are no other bits set in this
                  sub-priority.  */
               if (*(task -> tc_sub_priority_ptr) == 0)

                   /* Clear the main priority group bit.  */
                   TCD_Priority_Groups =
                       TCD_Priority_Groups & ~(task -> tc_priority_group);
           }
           else
           {

               /* Not the only task ready at the same priority level.  */

               /* Remove from the linked-list.  */
               (task -> tc_ready_previous) -> tc_ready_next =
                                                 task -> tc_ready_next;
               (task -> tc_ready_next) -> tc_ready_previous =
                                                 task -> tc_ready_previous;



               /* Update the head pointer.  */
               /* BUG FIX - update head if head is changing priority - leave
                  it alone otherwise! */
               if(*(task -> tc_priority_head) ==  task )
               *(task -> tc_priority_head) =  task -> tc_ready_next;

               /* Clear the next and previous pointers.  */
               task -> tc_ready_next =        NU_NULL;
               task -> tc_ready_previous =    NU_NULL;
           }

           /* Now add in the task at the new priority.  */
           task -> tc_priority =  new_priority;

           /* Build the other priority information.  */
           task -> tc_priority =      new_priority;
           task -> tc_priority_head = &(TCD_Priority_List[new_priority]);
           task -> tc_sub_priority =  (DATA_ELEMENT) (1 << (new_priority & 7));
           task -> tc_priority_group =   ((UNSIGNED) 1) << (new_priority >> 3);
           task -> tc_sub_priority_ptr =
                   &(TCD_Sub_Priority_Groups[(new_priority >> 3)]);

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpTask(task,RT_PROF_CHANGE_PRIORITY);
#endif
           /* Link the task into the new priority list.  */
           head =  *(task -> tc_priority_head);

           /* Determine if the list is non-empty.  */
           if (head)
           {

               /* Add the TCB to the end of the ready list.  */
               task -> tc_ready_previous =      head -> tc_ready_previous;
               (task -> tc_ready_previous) -> tc_ready_next =  task;
               task -> tc_ready_next =          head;
               (task -> tc_ready_next) -> tc_ready_previous =  task;

               /* Note that the priority bit map does not need to be
                  modified since there are other active tasks at the
                  same priority.  */
           }
           else
           {

               /* Add the TCB to an empty list.  */
               task -> tc_ready_previous =  task;
               task -> tc_ready_next =      task;
               *(task -> tc_priority_head)= task;

               /* Update the priority group bit map to indicate that this
                  priority now has a task ready.  */
               TCD_Priority_Groups =
                           TCD_Priority_Groups | (task -> tc_priority_group);

               /* Update the sub-priority bit map to show that this priority
                  is ready.  */
               *(task -> tc_sub_priority_ptr) =
                   (*(task -> tc_sub_priority_ptr)) | task -> tc_sub_priority;
           }

           /* Determine the highest priority task in the system.  */
           if (TCD_Priority_Groups & TC_HIGHEST_MASK)

               /* Base of sub-group is 0.  */
               index =  0;

           else if (TCD_Priority_Groups & TC_NEXT_HIGHEST_MASK)

               /* Base of sub-group is 8.  */
               index =  8;

           else if (TCD_Priority_Groups & TC_NEXT_LOWEST_MASK)

               /* Base of sub-group is 16.  */
               index =  16;
           else

               /* Base of sub-group is 24.  */
               index =  24;

           /* Calculate the highest available priority.  */
           index =  index + TCD_Lowest_Set_Bit[(INT)
                   ((TCD_Priority_Groups >> index) & TC_HIGHEST_MASK)];

           /* Get the mask of the priority within the group of 8 priorities.  */
           temp =  TCD_Sub_Priority_Groups[index];

           /* Calculate the actual priority.  */
           TCD_Highest_Priority =  (index << 3) + TCD_Lowest_Set_Bit[temp];

           /* Check for preemption.  */
           if ((TCD_Highest_Priority <= ((INT) TCD_Execute_Task -> tc_priority))
               && (TCD_Execute_Task -> tc_preemption))
           {

               /* Update the current task pointer.  */
               TCT_Set_Execute_Task(TCD_Priority_List[TCD_Highest_Priority]);

               /* Now, check and see if the current thread is a task.
                  If so, return a status that indicates a context
                  switch is needed.  */
               if ((TCD_Current_Thread) &&
                  (((TC_TCB *) TCD_Current_Thread) -> tc_id == TC_TASK_ID))

                   /* Transfer control to the system.  */
                   TCT_Control_To_System();
           }
       }
       else
       {

           /* Just modify the priority.  */
           task -> tc_priority =  new_priority;

           /* Build the other priority information.  */
           task -> tc_priority =      new_priority;
           task -> tc_priority_head = &(TCD_Priority_List[new_priority]);
           task -> tc_sub_priority =  (DATA_ELEMENT) (1 << (new_priority & 7));
           task -> tc_priority_group =   ((UNSIGNED) 1) << (new_priority >> 3);
           task -> tc_sub_priority_ptr =
                   &(TCD_Sub_Priority_Groups[(new_priority >> 3)]);
       }
#ifdef INCLUDE_PROVIEW
        _RTProf_DumpTask(task,RT_PROF_CHANGE_PRIORITY);
#endif
    }

    /* Release the protection of the scheduling list.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the old priority.  */
    return(old_priority);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCS_Change_Preemption                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function changes the preemption posture of the calling      */
/*      task.  Preemption for a task may be enabled or disabled.  If     */
/*      it is disabled, the task runs until it suspends or relinquishes. */
/*      If a preemption is pending, a call to this function to enable    */
/*      preemption causes a context switch.                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCSE_Change_Preemption              Error checking function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Protect                         Protect scheduling info      */
/*      TCT_Set_Execute_Task                Set TCD_Execute_Task pointer */
/*      TCT_Unprotect                       Release protection of info   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      preempt                             Preempt selection parameter  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      old_preempt                         Original preempt value       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified protection logic,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
OPTION   TCS_Change_Preemption(OPTION preempt)
{

TC_TCB         *task;                       /* Pointer to task           */
OPTION          old_preempt;
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CHANGE_PREEMPTION_ID, (UNSIGNED) preempt,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect the scheduling information.  */
    TCT_Protect(&TCD_System_Protect);

    /* Pickup the current thread and place it in the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;

    /* Save the old preempt value.  */
    if (task -> tc_preemption)

        /* Previously enabled.  */
        old_preempt =  NU_PREEMPT;
    else

        /* Previously disabled.  */
        old_preempt =  NU_NO_PREEMPT;

    /* Process the new value.  */
    if (preempt == NU_NO_PREEMPT)

        /* Disable preemption.  */
        TCD_Execute_Task -> tc_preemption =  NU_FALSE;
    else
    {

        /* Enable preemption.  */
        task -> tc_preemption =  NU_TRUE;

        /* Check for a preemption condition.  */
        if ((task == TCD_Execute_Task) &&
            (TCD_Highest_Priority < ((INT) TCD_Execute_Task -> tc_priority)))
        {

            /* Preempt the current task.  */
            TCT_Set_Execute_Task(TCD_Priority_List[TCD_Highest_Priority]);

            /* Transfer control to the system.  */
            TCT_Control_To_System();
        }
    }

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_CHANGE_PREEMPTION);
#endif

    /* Release protection of information.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the previous preemption posture.  */
    return(old_preempt);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCS_Change_Time_Slice                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function changes the time slice of the specified task.  A   */
/*      time slice value of 0 disables time slicing.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCES_Change_Preemption              Error checking function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect scheduling info      */
/*      TCT_Unprotect                       Release protection of info   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*      time_slice                          New time slice value         */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      old_time_slice                      Original time slice value    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      modified protection logic,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED   TCS_Change_Time_Slice(NU_TASK *task_ptr, UNSIGNED time_slice)
{

TC_TCB         *task;                       /* Task control block ptr    */
UNSIGNED        old_time_slice;             /* Old time slice value      */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CHANGE_TIME_SLICE_ID, (UNSIGNED) task,
                                        (UNSIGNED) time_slice, (UNSIGNED) 0);

#endif

    /* Protect the scheduling information.  */
    TCT_Protect(&TCD_System_Protect);

    /* Save the old time slice value.  */
    old_time_slice =  task -> tc_time_slice;

    /* Store the new time slice value.  */
    task -> tc_time_slice =      time_slice;
    task -> tc_cur_time_slice =  time_slice;

    /* Bug fix. Let the system know we have started a new time slice */
    TMD_Time_Slice_State = TM_ACTIVE;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_CHANGE_TIME_SLICE);
#endif
    /* Release protection of information.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the previous time slice value.  */
    return(old_time_slice);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCS_Control_Signals                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function enables the specified signals and returns the      */
/*      previous enable signal value back to the caller.  If a newly     */
/*      enabled signal is present and a signal handler is registered,    */
/*      signal handling is started.                                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCSE_Control_Signals                Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Signal_Shell                    Task signal execution        */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect against other access */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      enable_signal_mask                  Enable signal mask           */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Previous signal enable mask                                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      05-15-1993      Corrected problem with a comment                 */
/*      05-15-1993      Verified comment repair                          */
/*      03-01-1994      Added register optimizations,                    */
/*                      modified protection logic,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  TCS_Control_Signals(UNSIGNED enable_signal_mask)
{

R1 TC_TCB      *task;                      /* Task pointer              */
UNSIGNED        old_enable_mask;            /* Old enable signal mask    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CONTROL_SIGNALS_ID,(UNSIGNED) enable_signal_mask,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Pickup the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;

    /* Protect against simultaneous access.  */
    TCT_Protect(&TCD_System_Protect);

    /* Pickup the old signal mask.  */
    old_enable_mask =  task -> tc_enabled_signals;

    /* Put the new mask in.  */
    task -> tc_enabled_signals =  enable_signal_mask;

    /* Now, determine if the signal handler needs to be invoked.  */
    if ((enable_signal_mask & task -> tc_signals) &&
        (!task -> tc_signal_active) &&
        (task -> tc_signal_handler))
    {

        /* Signal processing is required.  */

        /* Indicate that signal processing is in progress.  */
        task -> tc_signal_active =  NU_TRUE;

        /* Clear the saved stack pointer to indicate that this is an
           in line signal handler call.  */
        task -> tc_saved_stack_ptr =  NU_NULL;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_CONTROL_SIGNALS);
#endif

        /* Release protection from multiple access.  */
        TCT_Unprotect();

        /* Call the signal handling shell. */
        TCC_Signal_Shell();
    }
    else
    {

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_CONTROL_SIGNALS);
#endif

        /* Release protection.  */
        TCT_Unprotect();
    }

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the old enable mask.  */
    return(old_enable_mask);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCS_Receive_Signals                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the current signals back to the caller.    */
/*      Note that the signals are cleared automatically.                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCSE_Receive_Signals                Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect against other access */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Current signals                                                  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified protection logic,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  TCS_Receive_Signals(VOID)
{

TC_TCB          *task;                      /* Task pointer              */
UNSIGNED        signals;                    /* Current signals           */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_RECEIVE_SIGNALS_ID, (UNSIGNED) 0,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Pickup the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;

    /* Protect against simultaneous access.  */
    TCT_Protect(&TCD_System_Protect);

    /* Pickup the current events.  */
    signals =  task -> tc_signals;

    /* Clear the current signals.  */
    task -> tc_signals =  0;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_RECEIVE_SIGNALS);
#endif

    /* Release protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the signals to the caller.  */
    return(signals);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCS_Register_Signal_Handler                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function registers a signal handler for the calling task.   */
/*      Note that if an enabled signal is present and this is the first  */
/*      registered signal handler call, the signal is processed          */
/*      immediately.                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCSE_Register_Signal_Handler        Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Signal_Shell                    Signal execution shell       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect against other access */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      signal_handler                      Signal execution shell       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      05-15-1993      Corrected problem with a comment                 */
/*      05-15-1993      Verified comment repair                          */
/*      03-01-1994      Added register optimizations,                    */
/*                      modified protection logic,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCS_Register_Signal_Handler(VOID (*signal_handler)(UNSIGNED))
{

R1 TC_TCB      *task;                       /* Task pointer              */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_REGISTER_SIGNAL_HANDLER_ID,
                        (UNSIGNED) signal_handler, (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Pickup the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;

    /* Protect against simultaneous access.  */
    TCT_Protect(&TCD_System_Protect);

    /* Put the new signal handler in.  */
    task -> tc_signal_handler =  signal_handler;

    /* Now, determine if the signal handler needs to be invoked.  */
    if ((task -> tc_enabled_signals & task -> tc_signals) &&
        (!task -> tc_signal_active) &&
        (task -> tc_signal_handler))
    {

        /* Signal processing is required.  */

        /* Indicate that signal processing is in progress.  */
        task -> tc_signal_active =  NU_TRUE;

        /* Clear the saved stack pointer to indicate that this is an
           in line signal handler call.  */
        task -> tc_saved_stack_ptr =  NU_NULL;

        /* Release protection from multiple access.  */
        TCT_Unprotect();

        /* Call the signal handling shell. */
        TCC_Signal_Shell();
    }
    else

        /* Release protection.  */
        TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return success.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCS_Send_Signals                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends the specified task the specified signals.    */
/*      If enabled, the specified task is setup in order to process the  */
/*      signals.                                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCSE_Send_Signals                   Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume task that is suspended*/
/*      TCC_Signal_Shell                    Signal execution shell       */
/*      TCT_Build_Signal_Frame              Build a signal frame         */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Control to system            */
/*      TCT_Protect                         Protect against other access */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task pointer                 */
/*      signals                             Signals to send to the task  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      modified protection logic,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-04-1996      On line 995, changed tc_signals                  */
/*                      to tc_enabled_signals,                           */
/*                      resulting in version 1.1+                        */
/*                      (spr 107)                                        */
/*                                                                       */
/*************************************************************************/
STATUS  TCS_Send_Signals(NU_TASK *task_ptr, UNSIGNED signals)
{

R1 TC_TCB      *task;                       /* Task control block ptr    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_SEND_SIGNALS_ID, (UNSIGNED) signals,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect against simultaneous access.  */
    TCT_Protect(&TCD_System_Protect);

    /* Or the new signals into the current signals.  */
    task -> tc_signals =  task -> tc_signals | signals;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_SEND_SIGNALS);
#endif
    /* Now, determine if the signal handler needs to be invoked.  */
    if ((task -> tc_signals & task -> tc_enabled_signals) &&
        (!task -> tc_signal_active) &&
        (task -> tc_status != NU_TERMINATED) &&
        (task -> tc_status != NU_FINISHED) &&
        (task -> tc_signal_handler))
    {

        /* Indicate that signal processing is in progress.  */
        task -> tc_signal_active =  NU_TRUE;

        /* Signal processing is required.  Determine if the task is sending
           signals to itself or if the calling thread is not the current
           task.  */
        if (task == (TC_TCB *) TCD_Current_Thread)
        {

            /* Task sending signals to itself.  */

            /* Clear the saved stack pointer to indicate that this is an
               in line signal handler call.  */
            task -> tc_saved_stack_ptr =  NU_NULL;

            /* Release protection from multiple access.  */
            TCT_Unprotect();

            /* Call the signal handling shell. */
            TCC_Signal_Shell();
        }
        else
        {

            /* Target task must be prepared to receive the signals.  */

            /* First, insure that the target task is not in a protected
               area.  */
            do
            {

                /* Check for protection.  Remember that protection is still
                   in effect.  */
                if (task -> tc_current_protect)
                {

                    /* Yes, target task is in a protected mode.  Release
                       the protection on the scheduling list and transfer
                       control briefly to the target task.  */
                    TCT_Unprotect();

                    /* Switch to the protected task and wait until the
                       task is not protected.  */
                    TCT_Protect_Switch(task);

                    /* Restore protection on the scheduling structures.  */
                    TCT_Protect(&TCD_System_Protect);
                }
            } while (task -> tc_current_protect);

            /* Copy the current status and stack pointer to the signal save
               areas.  */
            task -> tc_saved_status =           task -> tc_status;
            task -> tc_saved_stack_ptr =        task -> tc_stack_pointer;

            /* Build a stack frame for the signal handling shell function. */
            TCT_Build_Signal_Frame(task);

            /* Determine if the target task is currently suspended.  If it is
               suspended for any other reason than a pure suspend, resume
               it.  */
            if ((task -> tc_status != NU_READY) &&
                (task -> tc_status != NU_PURE_SUSPEND))
            {

                /* Resume the target task and check for preemption.  */
                if (TCC_Resume_Task(task_ptr, task -> tc_status))

                    /* Preemption needs to take place.  */
                    TCT_Control_To_System();
            }
        }
    }

    /* Release protection, no signals are currently enabled.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return a successful status.  */
    return(NU_SUCCESS);
}




