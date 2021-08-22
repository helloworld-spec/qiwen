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
/*      tcfe.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TC - Thread Control                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains routines that check parameters to the         */
/*      routines that return information about a thread.                 */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      TCFE_Task_Information                Retrieve task information   */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      tc_extr.h                           Thread Control functions     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      11-07-2002      Released version 1.14                            */
/*                                                                       */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "tc_extr.h"                 /* Thread control functions  */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCFE_Task_Information                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the function that returns information about a task.           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect scheduling info      */
/*      TCT_Unprotect                       Release protection           */
/*      TCF_Task_Information                Returns task information     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Pointer to the task          */
/*      name                                Destination for the name     */
/*      status                              Destination for task status  */
/*      scheduled_count                     Destination for scheduled    */
/*                                            count of the task          */
/*      priority                            Destination for task priority*/
/*      preempt                             Destination for preempt flag */
/*      time_slice                          Destination for time slice   */
/*      stack_base                          Destination for pointer to   */
/*                                            base of task's stack       */
/*      stack_size                          Destination for stack size   */
/*      minimum_stack                       Destination for the minimum  */
/*                                            running size of the stack  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If a valid task pointer is   */
/*                                            supplied                   */
/*      NU_INVALID_TASK                     If task pointer is invalid   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*                                                                       */
/*************************************************************************/
STATUS  TCFE_Task_Information(NU_TASK *task_ptr, CHAR *name,
            DATA_ELEMENT *status, UNSIGNED *scheduled_count,
            DATA_ELEMENT *priority, OPTION *preempt, UNSIGNED *time_slice,
            VOID **stack_base, UNSIGNED *stack_size, UNSIGNED *minimum_stack)
{

STATUS          completion;                 /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Check if parameters are valid.  task is tested in TCF_Task_Inforation */
    if (name == NU_NULL)
        completion = NU_INVALID_POINTER;
    else if (preempt == NU_NULL)
        completion = NU_INVALID_POINTER;
    else if (status == NU_NULL)
        completion = NU_INVALID_POINTER;
    else if (scheduled_count == NU_NULL)
        completion = NU_INVALID_POINTER;
    else if (priority == NU_NULL)
        completion = NU_INVALID_POINTER;
    else if (time_slice == NU_NULL)
        completion = NU_INVALID_POINTER;
    else if (stack_base == NU_NULL)
        completion = NU_INVALID_POINTER;
    else if (stack_size == NU_NULL)
        completion = NU_INVALID_POINTER;
    else if (minimum_stack == NU_NULL)
        completion = NU_INVALID_POINTER;
    else
        completion = TCF_Task_Information(task_ptr, name, status, scheduled_count,
                                      priority, preempt, time_slice, stack_base,
                                      stack_size, minimum_stack);
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    return(completion);
}




