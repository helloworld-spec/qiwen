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
/*      mbs.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      MB - Mailbox Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core routines for the Mailbox management  */
/*      component.                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      MBS_Reset_Mailbox                   Reset a mailbox              */
/*      MBS_Broadcast_To_Mailbox            Broadcast a mailbox message  */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      mb_extr.h                           Mailbox functions            */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1994      Initial version of supplemental                  */
/*                      mailbox service file,                            */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-23-2001      Fixed problem with resuming task in              */
/*                        MBS_Broadcast_To_Mailbox                       */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "mb_extr.h"                 /* Mailbox functions         */
#include        "hi_extr.h"                 /* History functions         */
#include        "profiler.h"                /* ProView interface         */


/* Define internal component function prototypes.  */

VOID    MBC_Cleanup(VOID *information);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBS_Reset_Mailbox                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function resets a mailbox back to the initial state.  Any   */
/*      message in the mailbox is discarded.  Also, all tasks suspended  */
/*      on the mailbox are resumed with the reset completion status.     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      MBSE_Reset_Mailbox                  Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_System_Protect                  Protect mailbox              */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      mailbox_ptr                         Mailbox control block pointer*/
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interface to                    */
/*                      match the prototype, added                       */
/*                      register variable logic,                         */
/*                      optimized protection logic,                      */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  MBS_Reset_Mailbox(NU_MAILBOX *mailbox_ptr)
{

R1 MB_MCB      *mailbox;                    /* Mailbox control block ptr */
R2 MB_SUSPEND  *suspend_ptr;                /* Suspend block pointer     */
MB_SUSPEND     *next_ptr;                   /* Next suspend block pointer*/
STATUS          preempt;                    /* Status for resume call    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_RESET_MAILBOX_ID, (UNSIGNED) mailbox,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect against access to the mailbox.  */
    TCT_System_Protect();

    /* Pickup the suspended task pointer list.  */
    suspend_ptr =  mailbox -> mb_suspension_list;

    /* Walk the chain task(s) currently suspended on the mailbox.  */
    preempt =  0;
    while (suspend_ptr)
    {

        /* Resume the suspended task.  Insure that the status returned is
           NU_MAILBOX_RESET.  */
        suspend_ptr -> mb_return_status =  NU_MAILBOX_RESET;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (MB_SUSPEND *) (suspend_ptr -> mb_suspend_link.cs_next);

        /* Resume the specified task.  */
        preempt =  preempt |
                TCC_Resume_Task((NU_TASK *) suspend_ptr -> mb_suspended_task,
                                                        NU_MAILBOX_SUSPEND);

        /* Determine if the next is the same as the head pointer.  */
        if (next_ptr == mailbox -> mb_suspension_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  NU_NULL;
        else

            /* Position the suspend pointer to the next suspend block.  */
            suspend_ptr =  next_ptr;
    }

    /* Initialize the mailbox.  */
    mailbox -> mb_message_present =  NU_FALSE;
    mailbox -> mb_tasks_waiting =    0;
    mailbox -> mb_suspension_list =  NU_NULL;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpMailBox(RT_PROF_RESET_MAILBOX,mailbox,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

    /* Determine if preemption needs to occur.  */
    if (preempt)

        /* Transfer control to system to facilitate preemption.  */
        TCT_Control_To_System();

    /* Release protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return a successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBS_Broadcast_To_Mailbox                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends a message to all tasks currently waiting for */
/*      a message from the mailbox.  If no tasks are waiting, this       */
/*      service behaves like a normal send message.                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      MBSE_Broadcast_To_Mailbox           Broadcast to a mailbox       */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Place on suspend list        */
/*      CSC_Priority_Place_On_List          Place on priority list       */
/*      CSC_Remove_From_List                Remove from suspend list     */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      TCC_Suspend_Task                    Suspend calling task         */
/*      TCC_Task_Priority                   Priority of specified task   */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Current_Thread                  Pickup current thread pointer*/
/*      TCT_System_Protect                  Protect mailbox              */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      mailbox_ptr                         Mailbox control block pointer*/
/*      message                             Pointer to message to send   */
/*      suspend                             Suspension option if full    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If service is successful     */
/*      NU_MAILBOX_FULL                     If mailbox is currently full */
/*      NU_TIMEOUT                          If timeout on service expires*/
/*      NU_MAILBOX_DELETED                  If mailbox is deleted during */
/*                                            suspension                 */
/*      NU_MAILBOX_RESET                    If mailbox is deleted during */
/*                                            suspension                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interface to                    */
/*                      match the prototype, added                       */
/*                      register variable logic,                         */
/*                      optimized protection logic,                      */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  MBS_Broadcast_To_Mailbox(NU_MAILBOX *mailbox_ptr, VOID *message,
                                                        UNSIGNED suspend)
{

R1 MB_MCB      *mailbox;                    /* Mailbox control block ptr */
MB_SUSPEND      suspend_block;              /* Allocate suspension block */
R2 MB_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
MB_SUSPEND     *suspend_head;               /* Pointer to suspend head   */
MB_SUSPEND     *next_suspend_ptr;           /* Get before restarting task*/
STATUS          preempt;                    /* Preemption flag           */
R3 UNSIGNED    *source_ptr;                 /* Pointer to source         */
R4 UNSIGNED    *destination_ptr;            /* Pointer to destination    */
TC_TCB         *task;                       /* Task pointer              */
STATUS          status;                     /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_BROADCAST_TO_MAILBOX_ID, (UNSIGNED) mailbox,
                                       (UNSIGNED) message, (UNSIGNED) suspend);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Protect against simultaneous access to the mailbox.  */
    TCT_System_Protect();

    /* Determine if the mailbox is empty or full.  */
    if (mailbox -> mb_message_present)
    {

        /* Mailbox already has a message.  Determine if suspension is
           required.  */
        if (suspend)
        {

            /* Suspension is requested.  */

            /* Increment the number of tasks suspended on the mailbox. */
            mailbox -> mb_tasks_waiting++;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpMailBox(RT_PROF_BROADCAST_TO_MAILBOX,mailbox,RT_PROF_WAIT);
#endif /* INCLUDE_PROVIEW */

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> mb_mailbox =                  mailbox;
            suspend_ptr -> mb_suspend_link.cs_next =     NU_NULL;
            suspend_ptr -> mb_suspend_link.cs_previous = NU_NULL;
            suspend_ptr -> mb_message_area =             (UNSIGNED *) message;
            task =                           (TC_TCB *)  TCT_Current_Thread();
            suspend_ptr -> mb_suspended_task =           task;

            /* Determine if priority or FIFO suspension is associated with the
               mailbox.  */
            if (mailbox -> mb_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this mailbox.  */
                CSC_Place_On_List((CS_NODE **) &(mailbox ->mb_suspension_list),
                                        &(suspend_ptr -> mb_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> mb_suspend_link.cs_priority =
                                                    TCC_Task_Priority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                        &(mailbox -> mb_suspension_list),
                                        &(suspend_ptr -> mb_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the mailbox.  */
            TCC_Suspend_Task((NU_TASK *) task, NU_MAILBOX_SUSPEND,
                                        MBC_Cleanup, suspend_ptr, suspend);

            /* Pickup the return status.  */
            status =  suspend_ptr -> mb_return_status;
        }
        else
        {

            /* Return a status of NU_MAILBOX_FULL because there is no
               room in the mailbox for the message.  */
            status =  NU_MAILBOX_FULL;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpMailBox(RT_PROF_BROADCAST_TO_MAILBOX,mailbox,RT_PROF_FAIL);
#endif /* INCLUDE_PROVIEW */

        }
    }
    else
    {

        /* Determine if a task is waiting on the mailbox.  */
        if (mailbox -> mb_suspension_list)
        {

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpMailBox(RT_PROF_BROADCAST_TO_MAILBOX,mailbox,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

            /* At least one task is waiting on mailbox for a message.  */

            /* Save off the suspension list and and then clear out the
               mailbox suspension.  */
            suspend_head =  mailbox -> mb_suspension_list;
            mailbox -> mb_suspension_list =  NU_NULL;

            /* Loop to wakeup all of the tasks waiting on the mailbox for
               a message.  */
            suspend_ptr =  suspend_head;
            preempt =      0;
            do
            {

                /* Setup the source and destination pointers.  */
                source_ptr =       (UNSIGNED *) message;
                destination_ptr =  suspend_ptr -> mb_message_area;

                /* Copy the message directly into the waiting task's
                   destination.  */
                *destination_ptr =        *source_ptr;
                *(destination_ptr + 1) =  *(source_ptr + 1);
                *(destination_ptr + 2) =  *(source_ptr + 2);
                *(destination_ptr + 3) =  *(source_ptr + 3);

                /* Setup the appropriate return value.  */
                suspend_ptr -> mb_return_status =  NU_SUCCESS;

                /* Move the suspend pointer along to the next block. */
                next_suspend_ptr =  (MB_SUSPEND *)
                                suspend_ptr -> mb_suspend_link.cs_next;

                /* Wakeup each task waiting.  */
                preempt =  preempt |
                 TCC_Resume_Task((NU_TASK *) suspend_ptr -> mb_suspended_task,
                                                        NU_MAILBOX_SUSPEND);
                suspend_ptr = next_suspend_ptr;

            } while (suspend_ptr != suspend_head);

            /* Clear the number of tasks waiting counter of the mailbox.  */
            mailbox -> mb_tasks_waiting =  0;

            /* Determine if a preempt condition is present.  */
            if (preempt)

                /* Transfer control to the system if the resumed task function
                   detects a preemption condition.  */
                TCT_Control_To_System();
        }
        else
        {

            /* Mailbox is empty and no task is waiting.  */

            /* Setup the source and destination pointers.  */
            source_ptr =       (UNSIGNED *) message;
            destination_ptr =  &(mailbox -> mb_message_area[0]);

            /* Place the message in the mailbox. */
            *destination_ptr =        *source_ptr;
            *(destination_ptr + 1) =  *(source_ptr + 1);
            *(destination_ptr + 2) =  *(source_ptr + 2);
            *(destination_ptr + 3) =  *(source_ptr + 3);

            /* Indicate that the mailbox has a message.  */
            mailbox -> mb_message_present =  NU_TRUE;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpMailBox(RT_PROF_BROADCAST_TO_MAILBOX,mailbox,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */
        }
    }

    /* Release protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the completion status.  */
    return(status);
}






