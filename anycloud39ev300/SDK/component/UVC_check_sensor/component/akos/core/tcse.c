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
/*      tcse.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TC - Thread Control                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains error checking routines for the supplemental  */
/*      functions in the Thread Control component.  This permits easy    */
/*      removal of error checking logic when it is not needed.           */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      TCSE_Change_Priority                Change task's priority       */
/*      TCSE_Change_Preemption              Change task's preemption     */
/*      TCSE_Change_Time_Slice              Change task's time slice     */
/*      TCSE_Control_Signals                Enable and disable signals   */
/*      TCSE_Receive_Signals                Receive current signals      */
/*      TCSE_Register_Signal_Handler        Register a signal handler    */
/*      TCSE_Send_Signals                   Send signals to a task       */
/*      TCSE_Activate_HISR                  Activate an HISR             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      tc_extr.h                           Thread Control functions     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1994      Created initial version 1.1 from                 */
/*                        routines originally in core                    */
/*                        error checking file                            */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-17-1997      Protected Send Signals service                   */
/*                      from NULL Control Block pointers                 */
/*                      creating 1.2a. (SPR220)                          */
/*      03-24-1998      Released version 1.3.                            */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "tc_extr.h"                 /* Thread control functions  */

/* Define external inner-component global data references.  */

extern TC_TCB          *TCD_Execute_Task;
extern VOID            *TCD_Current_Thread;


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCSE_Change_Priority                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking for the change priority    */
/*      service.  If an error is detected, this service is ignored and   */
/*      the requested priority is returned.                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCS_Change_Priority                 Actual change priority       */
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
/*      03-01-1994      Changed function interface,                      */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
OPTION   TCSE_Change_Priority(NU_TASK *task_ptr, OPTION new_priority)
{

TC_TCB         *task;                       /* Task control block ptr    */
OPTION          old_priority;               /* Previous priority of task */


    /* Move input task pointer into internal task pointer.  */
    task =  (TC_TCB *) task_ptr;

    /* Determine if the task pointer is valid.  */
    if (task -> tc_id == TC_TASK_ID) 
    
        /* Nothing seems to be wrong, change the priority as specified.  */
        old_priority =  TCS_Change_Priority(task_ptr, new_priority);
        
    else
    
        /* Copy the new priority into the old priority.  */
        old_priority =  new_priority;
    
    /* Return the previous priority.  */
    return(old_priority);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCSE_Change_Preemption                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the change preemption   */
/*      service.  If the current thread is not a task thread, this       */
/*      request is ignored.                                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCS_Change_Preemption               Change the preemption posture*/
/*                                            of the calling task        */
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
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
OPTION   TCSE_Change_Preemption(OPTION preempt)
{

TC_TCB         *task;                       /* Pointer to task           */
OPTION          old_preempt;

    /* Pickup the current thread and place it in the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;

    /* Determine if the current thread is really a task thread.  */
    if (task -> tc_id == TC_TASK_ID)
    
        /* Yes, change the preemption posture.  */
        old_preempt =  TCS_Change_Preemption(preempt);

    else
    
        /* Return the original request.  */
        old_preempt =  preempt;
        
    /* Return the previous preemption posture.  */
    return(old_preempt);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCSE_Change_Time_Slice                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the change time slice   */
/*      service.  If the specified task pointer is invalid, this         */
/*      request is ignored.                                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCS_Change_Time_Slice               Change the time slice of the */
/*                                            specified task             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*      time_slice                          New time slice value         */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      old_time_slice                      Old time slice value         */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interface,                      */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED   TCSE_Change_Time_Slice(NU_TASK *task_ptr, UNSIGNED time_slice)
{

TC_TCB         *task;                       /* Task control block ptr   */
UNSIGNED        old_time_slice;             /* Old time slice value     */


    /* Move input task pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;

    /* Determine if the task pointer is valid.  */
    if (task -> tc_id == TC_TASK_ID)
    
        /* Yes, change the time slice.  */
        old_time_slice =  TCS_Change_Time_Slice(task_ptr, time_slice);

    else
    
        /* Return the current request.  */
        old_time_slice =  time_slice;
        
    /* Return the previous time slice value.  */
    return(old_time_slice);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCSE_Control_Signals                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function checks to see if the call is being made from a     */
/*      non-task thread.  If so, the request is simply ignored.          */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCS_Control_Signals                 Actual control signals func  */
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
/*                                                                       */
/*************************************************************************/
UNSIGNED  TCSE_Control_Signals(UNSIGNED enable_signal_mask)
{

UNSIGNED         return_mask;               /* Return signal mask        */
TC_TCB          *task;                      /* Task pointer              */


    /* Pickup the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;

    /* Determine if the call is valid.  */
    if (task -> tc_id == TC_TASK_ID)
    
        /* Valid request- call actual routine to control signals.  */
        return_mask =  TCS_Control_Signals(enable_signal_mask);
    else
    
        /* Return a cleared mask.  */
        return_mask =  0;        

    /* Return the old enable mask.  */
    return(return_mask);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCSE_Receive_Signals                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function determines whether or not the call is being made   */
/*      from a task thread of execution.  If not, the call is ignored.   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCS_Receive_Signals                 Actual receive signals func  */
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
/*                                                                       */
/*************************************************************************/
UNSIGNED  TCSE_Receive_Signals(VOID)
{

TC_TCB          *task;                      /* Task pointer              */
UNSIGNED        signals;                    /* Current signals           */

    /* Pickup the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;

    /* Determine if the call is valid.  */
    if (task -> tc_id == TC_TASK_ID)
    
        /* Valid request- call actual routine to receive signals.  */
        signals =  TCS_Receive_Signals();
    else
    
        /* Return cleared signals.  */
        signals =  0;        
        
    /* Return the signals to the caller.  */
    return(signals);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCSE_Register_Signal_Handler                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function determines whether or not the caller is a task.    */
/*      If the caller is not a task and/or if the supplied signal        */
/*      handling function pointer is NULL, an appropriate error status   */
/*      is returned.                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCS_Register_Signal_Handler         Actual function to register  */
/*                                            the signal handler         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      signal_handler                      Signal execution shell       */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_TASK                     Not called from task thread  */
/*      NU_INVALID_POINTER                  Signal handler pointer NULL  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCSE_Register_Signal_Handler(VOID (*signal_handler)(UNSIGNED))
{

STATUS           status;                    /* Return status             */
TC_TCB          *task;                      /* Task pointer              */

    /* Pickup the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;

    /* Determine if the caller is a task.  */
    if (task -> tc_id != TC_TASK_ID)
    
        /* Indicate that the caller is invalid.  */
        status =  NU_INVALID_TASK;
        
    else if (signal_handler == NU_NULL)
    
        /* Indicate that the signal handler is invalid.  */
        status =  NU_INVALID_POINTER;

    else
    
        /* Everything is fine, call the actual function.  */
        status =  TCS_Register_Signal_Handler(signal_handler);
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCSE_Send_Signals                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function checks for an invalid task.  If an invalid task    */
/*      is selected and error is returned.                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCS_Send_Signals                    Actual send signal function  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task pointer                 */
/*      signals                             Signals to send to the task  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_TASK                     Task pointer is invalid      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interface,                      */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      03-17-1997      Corrected SPR220.                                */
/*                                                                       */
/*************************************************************************/
STATUS  TCSE_Send_Signals(NU_TASK *task_ptr, UNSIGNED signals)
{

TC_TCB         *task;                       /* Task control block ptr    */
STATUS          status;                     /* Completion status         */


    /* Move input task pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;

    /* Determine if the task pointer is valid.  */
    if ((task != NU_NULL)  && (task -> tc_id == TC_TASK_ID))
    
        /* Task pointer is valid, call the actual function.  */
        status =  TCS_Send_Signals(task_ptr, signals);
    else
    
        /* Task pointer is invalid, return an error status.  */
        status =  NU_INVALID_TASK;
        
    /* Return the completion status.  */
    return(status);
}




