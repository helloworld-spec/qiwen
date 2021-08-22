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
/*      tcce.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TC - Thread Control                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains error checking routines for the functions in  */
/*      the Thread Control component.  This permits easy removal of      */
/*      error checking logic when it is not needed.                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      TCCE_Create_Task                    Create a task                */
/*      TCCE_Create_HISR                    Create HISR                  */
/*      TCCE_Delete_HISR                    Delete HISR                  */
/*      TCCE_Delete_Task                    Delete a task                */
/*      TCCE_Reset_Task                     Reset a task                 */
/*      TCCE_Terminate_Task                 Terminate a task             */
/*      TCCE_Resume_Service                 Resume a task service call   */
/*      TCCE_Suspend_Service                Suspend a task service call  */
/*      TCCE_Relinquish                     Relinquish task execution    */
/*      TCCE_Task_Sleep                     Task sleep request           */
/*      TCCE_Suspend_Error                  Check for suspend req error  */
/*      TCCE_Activate_HISR                  Activate an HISR             */
/*      TCCE_Validate_Resume                Validates resume requests    */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      tc_extr.h                           Thread Control functions     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified logic that checked task                 */
/*                        status without protection of                   */
/*                        scheduling structures,                         */
/*                        resulting in version 1.0a                      */
/*      03-01-1994      Verified version 1.0a                            */
/*      03-01-1994      Moved non-core error checking                    */
/*                        functions to a supplemental                    */
/*                        file, and modified function                    */
/*                        interfaces, added validate                     */
/*                        resume service, resulting in                   */
/*                        version 1.1                                    */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      03-19-1996      Added error checking to                          */
/*                        TCCE_Task_Sleep, resulting                     */
/*                        in version 1.1+ (spr037)                       */
/*      04-17-1996      updated to version 1.2                           */
/*      10-16-1996      Modified to save the current                     */
/*                        thread's protection rather                     */
/*                        than that of the task being                    */
/*                        resumed (SPR212)(SPR268)                       */
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
/*      TCCE_Create_Task                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the create task function.                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCC_Create_Task                     Actual create task function  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*      name                                Task name                    */
/*      task_entry                          Entry function of the task   */
/*      argc                                Optional task parameter      */
/*      argv                                Optional task parameter      */
/*      stack_address                       Pointer to start of stack    */
/*      stack_size                          Size of task stack in bytes  */
/*      priority                            Task priority                */
/*      time_slice                          Task time slice              */
/*      preempt                             Task preemptability flag     */
/*      auto_start                          Automatic task start         */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          Successful request           */
/*      NU_INVALID_TASK                     Task control block pointer   */
/*                                            is NULL                    */
/*      NU_INVALID_ENTRY                    Task entry function is NULL  */
/*      NU_INVALID_MEMORY                   Stack pointer is NULL        */
/*      NU_INVALID_SIZE                     Stack size is too small      */
/*      NU_INVALID_PRIORITY                 Invalid task priority        */
/*      NU_INVALID_PREEMPT                  Invalid preemption selection */
/*      NU_INVALID_START                    Invalid start selection      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCCE_Create_Task(NU_TASK *task_ptr, CHAR *name, 
          VOID (*task_entry)(UNSIGNED, VOID *), UNSIGNED argc, VOID *argv, 
          VOID *stack_address, UNSIGNED stack_size,
          OPTION priority, UNSIGNED time_slice, 
          OPTION preempt, OPTION auto_start)
{

TC_TCB         *task;                       /* Task control block ptr    */
STATUS          status;                     /* Completion status         */


    /* Move input task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;

    /* Check each parameter.  */
    if ((task == NU_NULL) || (task -> tc_id == TC_TASK_ID))
    
        /* Invalid task control block pointer.  */
        status =  NU_INVALID_TASK;

    else if (task_entry == NU_NULL)
    
        /* Invalid task entry function pointer.  */
        status =  NU_INVALID_ENTRY;
        
    else if (stack_address == NU_NULL)
    
        /* Invalid stack starting address.  */
        status =  NU_INVALID_MEMORY;
        
    else if (stack_size < NU_MIN_STACK_SIZE)
    
        /* Invalid stack size.  */
        status =  NU_INVALID_SIZE;
        
   
    else if ((preempt != NU_PREEMPT) && (preempt != NU_NO_PREEMPT))
    
        /* Invalid preemption.  */
        status =  NU_INVALID_PREEMPT;
        
    else if ((auto_start != NU_START) && (auto_start != NU_NO_START))
    
        /* Invalid start selection. */
        status =  NU_INVALID_START;
        
    else
    
        /* Call the actual function to create a task.  All the parameters 
           appear to be correct.  */
        status =  TCC_Create_Task(task_ptr, name, task_entry, argc, argv,
         stack_address, stack_size, priority, time_slice, preempt, auto_start);
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Create_HISR                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the create HISR function.                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCC_Create_HISR                     Actual create HISR function  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hisr_ptr                            HISR control block pointer   */
/*      name                                HISR name                    */
/*      hisr_entry                          Entry function of the HISR   */
/*      priority                            Task priority                */
/*      stack_address                       Pointer to start of stack    */
/*      stack_size                          Size of HISR stack in bytes  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_HISR                     Invalid HISR pointer         */
/*      NU_INVALID_ENTRY                    Invalid HISR entry point     */
/*      NU_INVALID_PRIORITY                 Invalid HISR priority        */
/*      NU_INVALID_MEMORY                   Indicates stack pointer NULL */
/*      NU_INVALID_SIZE                     Indicates stack size is too  */
/*                                            small                      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCCE_Create_HISR(NU_HISR *hisr_ptr, CHAR *name, 
                            VOID (*hisr_entry)(VOID), OPTION priority, 
                            VOID *stack_address, UNSIGNED stack_size)
{

TC_HCB         *hisr;                       /* HISR control block ptr    */
STATUS          status;                     /* Completion status         */



    /* Move input HISR pointer into internal pointer.  */
    hisr =  (TC_HCB *) hisr_ptr;

    /* Check each parameter.  */
    if ((hisr == NU_NULL) || (hisr -> tc_id == TC_HISR_ID))
    
        /* Invalid HISR control block pointer.  */
        status =  NU_INVALID_HISR;

    else if (hisr_entry == NU_NULL)
    
        /* Invalid HISR entry function pointer.  */
        status =  NU_INVALID_ENTRY;
        
    else if (stack_address == NU_NULL)
    
        /* Invalid stack starting address.  */
        status =  NU_INVALID_MEMORY;
        
    else if (stack_size < NU_MIN_STACK_SIZE)
    
        /* Invalid stack size.  */
        status =  NU_INVALID_SIZE;
        
    else if (((INT) priority) >= TC_HISR_PRIORITIES)
    
        /* Invalid HISR priority.  */
        status =  NU_INVALID_PRIORITY;

    else
    
        /* Call the actual function to create a HISR.  All the parameters 
           appear to be correct.  */
        status =  TCC_Create_HISR(hisr_ptr, name, hisr_entry, priority,
                                                stack_address, stack_size);
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Delete_Task                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the delete task function.                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCC_Delete_Task                     Actual delete task function  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If successful completion     */
/*      NU_INVALID_TASK                     Task pointer is invalid      */
/*      NU_INVALID_DELETE                   Task not in a finished or    */
/*                                            terminated state           */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCCE_Delete_Task(NU_TASK *task_ptr)
{

TC_TCB         *task;                       /* Task control block ptr    */
STATUS          status;                     /* Completion status         */


    /* Move input task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;

    /* Determine if the supplied task pointer is valid.  */
    if ((task == NU_NULL) || (task -> tc_id != TC_TASK_ID))
    
        /* Invalid task pointer supplied.  */
        status =  NU_INVALID_TASK;

    else if ((task -> tc_status != NU_FINISHED) &&
             (task -> tc_status != NU_TERMINATED))
             
        /* A task that is not in the finished or terminated state cannot
           be deleted.  */
        status =  NU_INVALID_DELETE;

    else
    
        /* Valid task pointer, call the function to delete the task.  */
        status =  TCC_Delete_Task(task_ptr);
        
    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Delete_HISR                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the delete HISR function.                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCC_Delete_HISR                     Actual delete HISR function  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hisr_ptr                            HISR control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_HISR                     Indicates HISR pointer is    */
/*                                            invalid                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCCE_Delete_HISR(NU_HISR *hisr_ptr)
{

TC_HCB         *hisr;                       /* HISR control block ptr    */
STATUS          status;                     /* Completion status         */


    /* Move input HISR control block pointer into internal pointer.  */
    hisr =  (TC_HCB *) hisr_ptr;
    
    /* Determine if the supplied HISR pointer is valid.  */
    if ((hisr) && (hisr -> tc_id == TC_HISR_ID))
    
        /* Valid HISR pointer, call the function to delete the HISR.  */
        status =  TCC_Delete_HISR(hisr_ptr);
    else
    
        /* Invalid HISR pointer, indicate with the status.  */
        status =  NU_INVALID_HISR;
        
    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Reset_Task                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the reset task function.                                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCC_Reset_Task                      Actual reset task function   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*      argc                                Optional task parameter      */
/*      argv                                Optional task parameter      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_TASK                     Indicates task pointer is    */
/*                                            invalid                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCCE_Reset_Task(NU_TASK *task_ptr, UNSIGNED argc, VOID *argv)
{

TC_TCB         *task;                       /* Task control block ptr   */
STATUS          status;                     /* Status of the request    */


    /* Move input task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;

    /* Determine if the task pointer is valid.  */
    if ((task == NU_NULL) || (task -> tc_id != TC_TASK_ID))
    
        /* Task pointer is invalid.  */
        status =  NU_INVALID_TASK;
    else
    
        /* Call actual function to reset the task.  */
        status =  TCC_Reset_Task(task_ptr, argc, argv);

    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Terminate_Task                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the terminate task function.                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCC_Terminate_Task                  Actual terminate task funct  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_TASK                     Indicates task pointer is    */
/*                                            invalid                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCCE_Terminate_Task(NU_TASK *task_ptr)
{

TC_TCB         *task;                       /* Task control block ptr    */
STATUS          status;                     /* Status return             */


    /* Move input task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;

    /* Determine if the task pointer is valid.  */
    if ((task == NU_NULL) || (task -> tc_id != TC_TASK_ID))
    
        /* Task pointer is invalid.  */
        status =  NU_INVALID_TASK;
    else
    
        /* Call actual function to terminate the task.  */
        status =  TCC_Terminate_Task(task_ptr);

    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Resume_Service                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the resume task function.                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCCE_Validate_Resume                Function that checks the     */
/*                                            current task status for a  */
/*                                            valid resume request       */
/*      TCC_Resume_Service                  Actual task resume service   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If successful completion     */
/*      NU_INVALID_TASK                     Task pointer is invalid      */
/*      NU_INVALID_RESUME                   Not previously suspended     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified logic that checked task                 */
/*                        status without protection of                   */
/*                        scheduling structures,                         */
/*                        resulting in version 1.0a                      */
/*      03-01-1994      Verified version 1.0a                            */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        moved validate resume function                 */
/*                        to this file, resulting in                     */
/*                        version 1.1                                    */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCCE_Resume_Service(NU_TASK *task_ptr)
{

TC_TCB         *task;                       /* Task control block ptr    */
STATUS          status;                     /* Completion status         */



    /* Move task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;

    /* Determine if the task pointer is valid.  */
    if ((task == NU_NULL) || (task -> tc_id != TC_TASK_ID))
    
        /* Task pointer is invalid.  */
        status =  NU_INVALID_TASK;
        
    /* Make sure that the task is suspended in an identical manner.  */
    else if (TCCE_Validate_Resume(NU_PURE_SUSPEND, task_ptr))
    
        /* Task is not unconditionally suspended, return error status.  */
        status =  NU_INVALID_RESUME;
    
    else
    
        /* Call the actual resume service.  */
        status =  TCC_Resume_Service(task_ptr);
        
    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Suspend_Service                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the suspend service.    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCC_Suspend_Service                 Actual suspend service       */
/*                                              function                 */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If successful completion     */
/*      NU_INVALID_TASK                     Task pointer is invalid      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCCE_Suspend_Service(NU_TASK *task_ptr)
{

TC_TCB         *task;                       /* Task control block ptr    */
STATUS          status;                     /* Completion status         */



    /* Move task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;

    /* Determine if the task pointer is valid.  */
    if ((task == NU_NULL) || (task -> tc_id != TC_TASK_ID))
    
        /* Task pointer is invalid.  */
        status =  NU_INVALID_TASK;
        
    else

       if ((task->tc_status == NU_FINISHED) ||  (task->tc_status == NU_TERMINATED))

             /* Can't suspend a task in a finished or terminated state */
             status =  NU_INVALID_SUSPEND;


    else
     
        /* Call the actual service routine.  */
        status =  TCC_Suspend_Service(task_ptr);    
    
    /* Return completion status.  */                             
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Relinquish                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking for the relinquish         */
/*      function.  If the current thread is not a task, this request     */
/*      is ignored.                                                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCC_Relinquish                      Actual relinquish function   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
VOID  TCCE_Relinquish(VOID)
{

TC_TCB         *task;                       /* Pointer to task           */

    /* Pickup the current thread and place it in the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;
     
    /* Determine if the current thread is a task.  If so, call the actual
       relinquish routine.  Otherwise, ignore the request.  */
    if ((task) && (task -> tc_id == TC_TASK_ID))
    
        /* Valid request, call the relinquish function.  */
        TCC_Relinquish();
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Task_Sleep                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking for the task sleep         */
/*      function.  If the current thread is not a task, this request     */
/*      is ignored.                                                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCC_Task_Sleep                      Actual task sleep function   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ticks                               Number of ticks to sleep for */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-19-1996      Added check for parameter of 0                   */
/*                        or negative number, resulting                  */
/*                        in version 1.1+ (spr037)                       */
/*                                                                       */
/*************************************************************************/
VOID  TCCE_Task_Sleep(UNSIGNED ticks)
{

TC_TCB         *task;                       /* Pointer to task           */

    /* If parameter is zero, return */
    if (ticks == 0)
        return;

    /* Pickup the current thread and place it in the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;
     
    /* Determine if the current thread is a task.  If so, call the actual
       task sleep routine.  Otherwise, ignore the request.  */
    if ((task) && (task -> tc_id == TC_TASK_ID))
    
        /* Valid request, call the sleep function.  */
        TCC_Task_Sleep(ticks);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Suspend_Error                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function checks for a suspend request error.  Suspension    */
/*      requests are only allowed from task threads.  A suspend request  */
/*      from any other thread is an error.                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Other Components                                                 */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_TRUE                             If an error is detected      */
/*      NU_FALSE                            If no error is detected      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
INT   TCCE_Suspend_Error(VOID)
{

TC_TCB          *task;                      /* Task pointer              */
INT              status =  NU_FALSE;        /* Initialize to no error    */


    /* Setup the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;

    /* Check for suspension errors.  */
    if (task == NU_NULL)
    
        /* Error, suspend request probably from initialization.  */
        status =  NU_TRUE;
        
    else if (task -> tc_id != TC_TASK_ID)

        /* Control block is probably an HISR not a task.  */
        status =  NU_TRUE;

    else if (task -> tc_signal_active)
    
        /* Called from a signal handler.  */
        status =  NU_TRUE;
        
    /* Return status to caller.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Activate_HISR                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the activate HISR function.                                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCT_Activate_HISR                   Actual HISR activate call    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hisr_ptr                            HISR control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_HISR                     Invalid HISR pointer         */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCCE_Activate_HISR(NU_HISR *hisr_ptr)
{

TC_HCB         *hisr;                       /* HISR control block ptr    */
STATUS          status;                     /* Completion status         */
NU_SUPERV_USER_VARIABLES

    NU_SUPERVISOR_MODE();
    /* Move input HISR control block pointer into internal pointer.  */
    hisr =  (TC_HCB *) hisr_ptr;

    /* Check each parameter.  */
    if (hisr == NU_NULL)
    
        /* Invalid HISR control block pointer.  */
        status =  NU_INVALID_HISR;

    else if (hisr -> tc_id != TC_HISR_ID)
    
        /* Invalid HISR control block pointer.  */
        status =  NU_INVALID_HISR;
        
    else
    
        /* Call the routine to activate the HISR.  */
        status =  TCT_Activate_HISR(hisr_ptr);

    /* Return to user mode */
    NU_USER_MODE();
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCCE_Validate_Resume                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function validates the resume service and resume driver     */
/*      calls with scheduling protection around the examination of the   */
/*      task status.                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      IOCE_Resume_Driver                  Driver error checking funct. */
/*      TCCE_Resume_Service                 Error checking function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCT_Set_Current_Protect             Setup current protect pointer*/
/*      TCT_System_Protect                  Protect from system access   */
/*      TCT_System_Unprotect                Release system protection    */
/*      TCT_Unprotect                       Release current protection   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      resume_type                         Type of resume request       */
/*      task_ptr                            Task control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_TRUE                             Invalid resume               */
/*      NU_FALSE                            Valid resume                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1994      Created initial version of                       */
/*                        function for version 1.0g                      */
/*      03-01-1994      Verified version 1.0g                            */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        added system protection logic,                 */
/*                        moved to TCCE since it is an                   */
/*                        error interface function,                      */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      10-16-1996      Modified to save the current                     */
/*                        thread's protection rather                     */
/*                        than that of the task being                    */
/*                        resumed (SPR212)(SPR268)                       */
/*                                                                       */
/*************************************************************************/
STATUS  TCCE_Validate_Resume(OPTION resume_type, NU_TASK *task_ptr)
{

R1 TC_TCB      *task;                       /* Task control block ptr    */
TC_PROTECT     *save_protect;               /* Save current protection   */
STATUS          status;                     /* Return status variable    */
NU_SUPERV_USER_VARIABLES

    NU_SUPERVISOR_MODE();

    /* Move input task pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;

    /* Save current protection.  */
    if (TCD_Current_Thread != NU_NULL)
    {
        save_protect = TCT_Get_Current_Protect();
    }
    else
    {
        save_protect = NU_NULL;
    }

    /* Protect the scheduling structures from multiple access.  */
    TCT_System_Protect();
    
    /* Does the resume type match the current status?  */
    if (task -> tc_status == resume_type)

        /* Indicate that there is no error.  */
        status =  NU_FALSE;
    
    /* Check for a resumption of a delayed pure suspend.  */
    else if ((resume_type == NU_PURE_SUSPEND) && (task -> tc_delayed_suspend))
        
        /* Indicate that there is no error.  */
        status =  NU_FALSE;
            
    /* Check for a signal active and the saved status the same as
       the resume request.  */
    else if ((resume_type == task -> tc_saved_status) &&
            (task -> tc_signal_active))

        /* Indicate that there is no error.  */
        status =  NU_FALSE;
        
    else
    
        /* Indicate that there is an error.  */
        status =  NU_TRUE;

    /* Determine how to get out of protection.  */
    if (save_protect)
    {

        /* Restore current protection.  */
        TCT_Set_Current_Protect(save_protect);

        /* Release system protect.  */
        TCT_System_Unprotect();
    }
    else

        /* Release protection of system structures.  */
        TCT_Unprotect();
            
    /* Return to user mode */
    NU_USER_MODE();
    
    /* Return status to caller.  */
    return(status);
}




