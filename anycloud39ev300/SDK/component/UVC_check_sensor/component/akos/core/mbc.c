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
/*      mbc.c                                          Nucleus PLUS 1.14 */
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
/*      MBC_Create_Mailbox                  Create a mailbox             */
/*      MBC_Delete_Mailbox                  Delete a mailbox             */
/*      MBC_Send_To_Mailbox                 Send a mailbox message       */
/*      MBC_Receive_From_Mailbox            Receive a mailbox message    */
/*      MBC_Cleanup                         Cleanup on timeout or a      */
/*                                          terminate condition          */
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
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Corrected pointer retrieval                      */
/*                      loop, resulting in version 1.0a                  */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Moved non-core functions into                    */
/*                      supplemental files, changed                      */
/*                      function interfaces to match                     */
/*                      those in prototype, added                        */
/*                      register options, changed                        */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "mb_extr.h"                 /* Mailbox functions         */
#include        "hi_extr.h"                 /* History functions         */
#include        "profiler.h"                /* ProView interface         */

/* Define external inner-component global data references.  */

extern CS_NODE         *MBD_Created_Mailboxes_List;
extern UNSIGNED         MBD_Total_Mailboxes;
extern TC_PROTECT       MBD_List_Protect;


/* Define internal component function prototypes.  */

VOID    MBC_Cleanup(VOID *information);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBC_Create_Mailbox                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a mailbox and then places it on the list   */
/*      of created mailboxes.                                            */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      MBCE_Create_Mailbox                 Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Add node to linked-list      */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Data structure protect       */
/*      TCT_Unprotect                       Un-protect data structure    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      mailbox_ptr                         Mailbox control block pointer*/
/*      name                                Mailbox name                 */
/*      suspend_type                        Suspension type              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
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
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  MBC_Create_Mailbox(NU_MAILBOX *mailbox_ptr, CHAR *name,
                                                OPTION suspend_type)
{

R1 MB_MCB      *mailbox;                    /* Mailbox control block ptr */
INT             i;                          /* Working index variable    */
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
    HIC_Make_History_Entry(NU_CREATE_MAILBOX_ID, (UNSIGNED) mailbox,
                                (UNSIGNED) name, (UNSIGNED) suspend_type);

#endif

    /* First, clear the mailbox ID just in case it is an old Mailbox
       Control Block.  */
    mailbox -> mb_id =             0;

    /* Fill in the mailbox name.  */
    for (i = 0; i < NU_MAX_NAME; i++)
        mailbox -> mb_name[i] =  name[i];

    /* Clear the message present flag- initial it to "no message present." */
    mailbox -> mb_message_present =  NU_FALSE;

    /* Setup the mailbox suspension type.  */
    if (suspend_type == NU_FIFO)

        /* FIFO suspension is selected, setup the flag accordingly.  */
        mailbox -> mb_fifo_suspend =  NU_TRUE;
    else

        /* Priority suspension is selected.  */
        mailbox -> mb_fifo_suspend =  NU_FALSE;

    /* Clear the suspension list pointer.  */
    mailbox -> mb_suspension_list =  NU_NULL;

    /* Clear the number of tasks waiting on the mailbox counter.  */
    mailbox -> mb_tasks_waiting =  0;

    /* Initialize link pointers.  */
    mailbox -> mb_created.cs_previous =    NU_NULL;
    mailbox -> mb_created.cs_next =        NU_NULL;

    /* Protect against access to the list of created mailboxes.  */
    TCT_Protect(&MBD_List_Protect);

    /* At this point the mailbox is completely built.  The ID can now be
       set and it can be linked into the created mailbox list.  */
    mailbox -> mb_id =                     MB_MAILBOX_ID;

    /* Link the mailbox into the list of created mailboxes and increment the
       total number of mailboxes in the system.  */
    CSC_Place_On_List(&MBD_Created_Mailboxes_List, &(mailbox -> mb_created));
    MBD_Total_Mailboxes++;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpMailBox(RT_PROF_CREATE_MAILBOX,mailbox,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */
    /* Release protection against access to the list of created mailboxes.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBC_Delete_Mailbox                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deletes a mailbox and removes it from the list of  */
/*      created mailboxes.  All tasks suspended on the mailbox are       */
/*      resumed.  Note that this function does not free the memory       */
/*      associated with the mailbox control block.                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      MBCE_Delete_Mailbox                 Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove node from list        */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Protect                         Protect created list         */
/*      TCT_Set_Current_Protect             Setup current protection ptr */
/*      TCT_System_Protect                  Protect against system access*/
/*      TCT_System_Unprotect                Release system protection    */
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
/*         DATE                    REMARKS                               */
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
STATUS  MBC_Delete_Mailbox(NU_MAILBOX *mailbox_ptr)
{

R1 MB_MCB      *mailbox;                    /* Mailbox control block ptr */
MB_SUSPEND     *suspend_ptr;                /* Suspend block pointer     */
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
    HIC_Make_History_Entry(NU_DELETE_MAILBOX_ID, (UNSIGNED) mailbox,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Call protection just in case another thread is using the mailbox.  */
    TCT_System_Protect();

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpMailBox(RT_PROF_DELETE_MAILBOX,mailbox,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */
    /* Clear the mailbox ID.  */
    mailbox -> mb_id =  0;

    /* Clear protection.  */
    TCT_Unprotect();

    /* Protect against access to the list of created mailboxes.  */
    TCT_Protect(&MBD_List_Protect);

    /* Remove the mailbox from the list of created mailboxes.  */
    CSC_Remove_From_List(&MBD_Created_Mailboxes_List,
                                        &(mailbox -> mb_created));

    /* Decrement the total number of created mailboxes.  */
    MBD_Total_Mailboxes--;

    /* Pickup the suspended task pointer list.  */
    suspend_ptr =  mailbox -> mb_suspension_list;

    /* Walk the chain task(s) currently suspended on the mailbox.  */
    preempt =  0;
    while (suspend_ptr)
    {

        /* Protect against system access.  */
        TCT_System_Protect();

        /* Resume the suspended task.  Insure that the status returned is
           NU_MAILBOX_DELETED.  */
        suspend_ptr -> mb_return_status =  NU_MAILBOX_DELETED;

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

            /* Setup current protection pointer.  */
        TCT_Set_Current_Protect(&MBD_List_Protect);

        /* Unprotect the system protection.  */
        TCT_System_Unprotect();
    }

    /* Determine if preemption needs to occur.  */
    if (preempt)

        /* Transfer control to system to facilitate preemption.  */
        TCT_Control_To_System();

    /* Release protection against access to the list of created mailboxes.  */
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
/*      MBC_Send_To_Mailbox                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends a 4-word message to the specified mailbox.   */
/*      If there are one or more tasks suspended on the mailbox for a    */
/*      message, the message is copied into the message area of the      */
/*      first task waiting and that task is resumed.  If the mailbox     */
/*      is full, suspension of the calling task is possible.             */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      MBCE_Send_To_Mailbox                Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Place on suspend list        */
/*      CSC_Priority_Place_On_List          Place on priority list       */
/*      CSC_Remove_From_List                Remove from suspend list     */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      TCC_Suspend_Task                    Suspend calling task         */
/*      TCC_Task_Priority                   Pickup task's priority       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Current_Thread                  Pickup current thread pointer*/
/*      TCT_System_Protect                  Protect against system access*/
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
STATUS  MBC_Send_To_Mailbox(NU_MAILBOX *mailbox_ptr, VOID *message,
                                                        UNSIGNED suspend)
{

R1 MB_MCB      *mailbox;                    /* Mailbox control block ptr */
MB_SUSPEND      suspend_block;              /* Allocate suspension block */
R2 MB_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
R3 UNSIGNED    *source_ptr;                 /* Pointer to source         */
R4 UNSIGNED    *destination_ptr;            /* Pointer to destination    */
TC_TCB         *task;                       /* Task pointer              */
STATUS          preempt;                    /* Preempt flag              */
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
    HIC_Make_History_Entry(NU_SEND_TO_MAILBOX_ID, (UNSIGNED) mailbox,
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

            /* Suspension is requested.   */

            /* Increment the number of tasks waiting.  */
            mailbox -> mb_tasks_waiting++;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpMailBox(RT_PROF_SEND_TO_MAILBOX,mailbox,RT_PROF_WAIT);
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
            _RTProf_DumpMailBox(RT_PROF_SEND_TO_MAILBOX,mailbox,RT_PROF_FAIL);
#endif /* INCLUDE_PROVIEW */

        }
    }
    else
    {

        /* Determine if a task is waiting on the mailbox.  */
        if (mailbox -> mb_suspension_list)
        {

            /* Task is waiting on mailbox for a message.  */

            /* Decrement the number of tasks waiting on mailbox.  */
            mailbox -> mb_tasks_waiting--;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpMailBox(RT_PROF_SEND_TO_MAILBOX,mailbox,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

            /* Remove the first suspended block from the list.  */
            suspend_ptr =  mailbox -> mb_suspension_list;
            CSC_Remove_From_List((CS_NODE **)
                &(mailbox -> mb_suspension_list),
                                &(suspend_ptr -> mb_suspend_link));

            /* Setup the source and destination pointers.  */
            source_ptr =       (UNSIGNED *) message;
            destination_ptr =  suspend_ptr -> mb_message_area;

            /* Copy the message directly into the waiting task's
               destination.  */
            *destination_ptr       =  *source_ptr;
            *(destination_ptr + 1) =  *(source_ptr + 1);
            *(destination_ptr + 2) =  *(source_ptr + 2);
            *(destination_ptr + 3) =  *(source_ptr + 3);

            /* Setup the appropriate return value.  */
            suspend_ptr -> mb_return_status =  NU_SUCCESS;

            /* Wakeup the waiting task and check for preemption.  */
            preempt =
               TCC_Resume_Task((NU_TASK *) suspend_ptr -> mb_suspended_task,
                                                NU_MAILBOX_SUSPEND);

            /* Determine if preemption needs to take place. */
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
            _RTProf_DumpMailBox(RT_PROF_SEND_TO_MAILBOX,mailbox,RT_PROF_OK);
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


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBC_Receive_From_Mailbox                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function receives a message from the specified mailbox.     */
/*      If there is a message currently in the mailbox, the message is   */
/*      removed from the mailbox and placed in the caller's area.        */
/*      Otherwise, if no message is present in the mailbox, suspension   */
/*      of the calling task is possible.                                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      MBCE_Receive_From_Mailbox           Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Place on suspend list        */
/*      CSC_Priority_Place_On_List          Place on priority list       */
/*      CSC_Remove_From_List                Remove from suspend list     */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      TCC_Suspend_Task                    Suspend calling task         */
/*      TCC_Task_Priority                   Pickup task priority         */
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
/*      suspend                             Suspension option if empty   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If service is successful     */
/*      NU_MAILBOX_EMPTY                    If mailbox is currently empty*/
/*      NU_TIMEOUT                          If timeout on service expires*/
/*      NU_MAILBOX_DELETED                  If mailbox is deleted during */
/*                                            suspension                 */
/*      NU_MAILBOX_RESET                    If mailbox is deleted during */
/*                                            suspension                 */
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
STATUS  MBC_Receive_From_Mailbox(NU_MAILBOX *mailbox_ptr, VOID *message,
                                                        UNSIGNED suspend)
{

R1 MB_MCB      *mailbox;                    /* Mailbox control block ptr */
MB_SUSPEND      suspend_block;              /* Allocate suspension block */
R2 MB_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
R3 UNSIGNED    *source_ptr;                 /* Pointer to source         */
R4 UNSIGNED    *destination_ptr;            /* Pointer to destination    */
TC_TCB         *task;                       /* Task pointer              */
STATUS          preempt;                    /* Preemption flag           */
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
    HIC_Make_History_Entry(NU_RECEIVE_FROM_MAILBOX_ID, (UNSIGNED) mailbox,
                                       (UNSIGNED) message, (UNSIGNED) suspend);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Protect against simultaneous access to the mailbox.  */
    TCT_System_Protect();

    /* Determine if the mailbox is empty or full.  */
    if (mailbox -> mb_message_present)
    {

        /* Copy message from mailbox into the caller's area.  */

        /* Setup the source and destination pointers.  */
        source_ptr =       &(mailbox -> mb_message_area[0]);
        destination_ptr =  (UNSIGNED *) message;

        /* Copy the message directly into the waiting task's
           destination.  */
        *destination_ptr =        *source_ptr;
        *(destination_ptr + 1) =  *(source_ptr + 1);
        *(destination_ptr + 2) =  *(source_ptr + 2);
        *(destination_ptr + 3) =  *(source_ptr + 3);

        /* Determine if another task is waiting to place something into the
           mailbox.  */
        if (mailbox -> mb_suspension_list)
        {

            /* Yes, another task is waiting to send something to the
               mailbox.  */

            /* Decrement the number of tasks waiting counter.  */
            mailbox -> mb_tasks_waiting--;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpMailBox(RT_PROF_RECEIVE_FROM_MAILBOX,mailbox,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */
            /* Remove the first suspended block from the list.  */
            suspend_ptr =  mailbox -> mb_suspension_list;
            CSC_Remove_From_List((CS_NODE **)
                &(mailbox -> mb_suspension_list),
                                &(suspend_ptr -> mb_suspend_link));

            /* Setup the source and destination pointers.  */
            source_ptr =       suspend_ptr -> mb_message_area;
            destination_ptr =  &(mailbox -> mb_message_area[0]);

            /* Copy the message directly into the waiting task's
               destination.  */
            *destination_ptr =        *source_ptr;
            *(destination_ptr + 1) =  *(source_ptr + 1);
            *(destination_ptr + 2) =  *(source_ptr + 2);
            *(destination_ptr + 3) =  *(source_ptr + 3);

            /* Setup the appropriate return value.  */
            suspend_ptr -> mb_return_status =  NU_SUCCESS;

            /* Resume the suspended task.  */
            preempt =
               TCC_Resume_Task((NU_TASK *) suspend_ptr -> mb_suspended_task,
                                                       NU_MAILBOX_SUSPEND);

            /* Determine if a preempt condition is present.  */
            if (preempt)

                /* Transfer control to the system if the resumed task function
                   detects a preemption condition.  */
                TCT_Control_To_System();
        }
        else
        {

            /* Clear the message present flag.  */
            mailbox -> mb_message_present =  NU_FALSE;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpMailBox(RT_PROF_RECEIVE_FROM_MAILBOX,mailbox,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

        }

    }
    else
    {

        /* Mailbox is empty.  Determine if suspension is required.  */
        if (suspend)
        {

            /* Suspension is required.  */

            /* Increment the number of tasks waiting on the mailbox counter. */
            mailbox -> mb_tasks_waiting++;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpMailBox(RT_PROF_RECEIVE_FROM_MAILBOX,mailbox,RT_PROF_WAIT);
#endif /* INCLUDE_PROVIEW */

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> mb_mailbox =                  mailbox;
            suspend_ptr -> mb_suspend_link.cs_next =     NU_NULL;
            suspend_ptr -> mb_suspend_link.cs_previous = NU_NULL;
            suspend_ptr -> mb_message_area =             (UNSIGNED *) message;
            task =                            (TC_TCB *) TCT_Current_Thread();
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
                                        &(suspend_block.mb_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the mailbox.  */
            TCC_Suspend_Task((NU_TASK *) task, NU_MAILBOX_SUSPEND,
                                        MBC_Cleanup, &suspend_block, suspend);

            /* Pickup the return status.  */
            status =  suspend_ptr -> mb_return_status;
        }
        else
        {

            /* Return a status of NU_MAILBOX_EMPTY because there is
               nothing in the mailbox.  */
            status =  NU_MAILBOX_EMPTY;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpMailBox(RT_PROF_RECEIVE_FROM_MAILBOX,mailbox,RT_PROF_FAIL);
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


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBC_Cleanup                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible for removing a suspension block     */
/*      from a mailbox.  It is not called unless a timeout or a task     */
/*      terminate is in progress.  Note that protection is already in    */
/*      effect - the same protection at suspension time.                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TCC_Timeout                         Task timeout                 */
/*      TCC_Terminate                       Task terminate               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove suspend block from    */
/*                                            the suspension list        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      information                         Pointer to suspend block     */
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
VOID  MBC_Cleanup(VOID *information)
{

MB_SUSPEND      *suspend_ptr;               /* Suspension block pointer  */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Use the information pointer as a suspend pointer.  */
    suspend_ptr =  (MB_SUSPEND *) information;

    /* By default, indicate that the service timed-out.  It really does not
       matter if this function is called from a terminate request since
       the task does not resume.  */
    suspend_ptr -> mb_return_status =  NU_TIMEOUT;

    /* Decrement the number of tasks waiting counter.  */
    (suspend_ptr -> mb_mailbox) -> mb_tasks_waiting--;

    /* Unlink the suspend block from the suspension list.  */
    CSC_Remove_From_List((CS_NODE **)
                &((suspend_ptr -> mb_mailbox) -> mb_suspension_list),
                                &(suspend_ptr -> mb_suspend_link));

    /* Return to user mode */
    NU_USER_MODE();
}







