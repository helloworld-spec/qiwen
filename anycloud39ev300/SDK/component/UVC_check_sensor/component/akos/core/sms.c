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
/*      sms.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      SM - Semaphore Management                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the supplemental routines for the Semaphore   */
/*      Management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      SMS_Reset_Semaphore                 Reset semaphore              */
/*                                            terminate condition        */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      sm_extr.h                           Semaphore functions          */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1994      Created initial version 1.1 from                 */
/*                        routines originally in core                    */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
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
#include        "sm_extr.h"                 /* Semaphore functions       */
#include        "hi_extr.h"                 /* History functions         */
#include        "profiler.h"                /* ProView interface         */



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      SMS_Reset_Semaphore                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function resets a semaphore back to the initial state.  All */
/*      tasks suspended on the semaphore are resumed with the reset      */
/*      completion status.                                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      SMSE_Reset_Semaphore                Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_System_Protect                  Protect semaphore            */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      semaphore_ptr                       Semaphore control block ptr  */
/*      initial_count                       Initial count to reset the   */
/*                                            semaphore to               */
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
/*      03-01-1994      Changed function interface to                    */
/*                        match the prototype, added                     */
/*                        register variable logic,                       */
/*                        optimized protection logic,                    */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  SMS_Reset_Semaphore(NU_SEMAPHORE *semaphore_ptr,
                                                    UNSIGNED initial_count)
{

R1 SM_SCB      *semaphore;                  /* Semaphore control blk ptr */
R2 SM_SUSPEND  *suspend_ptr;                /* Suspend block pointer     */
R3 SM_SUSPEND  *next_ptr;                   /* Next suspend block pointer*/
STATUS          preempt;                    /* Status for resume call    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input semaphore pointer into internal pointer.  */
    semaphore =  (SM_SCB *) semaphore_ptr;

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_RESET_SEMAPHORE_ID, (UNSIGNED) semaphore,
                                       (UNSIGNED) initial_count, (UNSIGNED) 0);

#endif

    /* Protect against access to the semaphore.  */
    TCT_System_Protect();

    /* Pickup the suspended task pointer list.  */
    suspend_ptr =  semaphore -> sm_suspension_list;

    /* Walk the chain task(s) currently suspended on the semaphore.  */
    preempt =  0;
    while (suspend_ptr)
    {

        /* Resume the suspended task.  Insure that the status returned is
           NU_SEMAPHORE_RESET.  */
        suspend_ptr -> sm_return_status =  NU_SEMAPHORE_RESET;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (SM_SUSPEND *) (suspend_ptr -> sm_suspend_link.cs_next);

        /* Resume the specified task.  */
        preempt =  preempt |
                TCC_Resume_Task((NU_TASK *) suspend_ptr -> sm_suspended_task,
                                                NU_SEMAPHORE_SUSPEND);

        /* Determine if the next is the same as the current pointer.  */
        if (next_ptr == semaphore -> sm_suspension_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  NU_NULL;
        else

            /* Move the next pointer into the suspend block pointer.  */
            suspend_ptr =  next_ptr;
    }

    /* Initialize the semaphore.  */
    semaphore -> sm_semaphore_count =  initial_count;
    semaphore -> sm_tasks_waiting =    0;
    semaphore -> sm_suspension_list =  NU_NULL;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpSema(RT_PROF_RESET_SEMAPHORE,(SM_SCB *) semaphore, RT_PROF_OK);
#endif

    /* Determine if preemption needs to occur.  */
    if (preempt)

        /* Transfer control to system to facilitate preemption.  */
        TCT_Control_To_System();

    /* Release protection against access to the semaphore.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return a successful completion.  */
    return(NU_SUCCESS);
}




