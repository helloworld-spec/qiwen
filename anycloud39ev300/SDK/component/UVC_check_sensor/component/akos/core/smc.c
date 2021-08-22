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
/*      smc.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      SM - Semaphore Management                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core routines for the Semaphore Management*/
/*      component.                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      SMC_Create_Semaphore                Create a semaphore           */
/*      SMC_Delete_Semaphore                Delete a semaphore           */
/*      SMC_Obtain_Semaphore                Obtain instance of semaphore */
/*      SMC_Release_Semaphore               Release instance of semaphore*/
/*      SMC_Cleanup                         Cleanup on timeout or a      */
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
#include        "sm_extr.h"                 /* Semaphore functions       */
#include        "hi_extr.h"                 /* History functions         */
#include        "profiler.h"                /* ProView interface         */


/* Define external inner-component global data references.  */

extern CS_NODE         *SMD_Created_Semaphores_List;
extern UNSIGNED         SMD_Total_Semaphores;
extern TC_PROTECT       SMD_List_Protect;


/* Define internal component function prototypes.  */

VOID    SMC_Cleanup(VOID *information);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      SMC_Create_Semaphore                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a semaphore and then places it on the list */
/*      of created semaphores.                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      SMCE_Create_Semaphore               Error checking shell         */
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
/*      semaphore_ptr                       Semaphore control block ptr  */
/*      name                                Semaphore name               */
/*      initial_count                       Initial semaphore instance   */
/*                                            count                      */
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
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options,                          */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  SMC_Create_Semaphore(NU_SEMAPHORE *semaphore_ptr, CHAR *name,
                           UNSIGNED initial_count, OPTION suspend_type)
{

R1 SM_SCB      *semaphore;                  /* Semaphore control blk ptr */
INT             i;                          /* Working index variable    */
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
    HIC_Make_History_Entry(NU_CREATE_SEMAPHORE_ID, (UNSIGNED) semaphore,
                                (UNSIGNED) name, (UNSIGNED) initial_count);

#endif

    /* First, clear the semaphore ID just in case it is an old Semaphore
       Control Block.  */
    semaphore -> sm_id =             0;

    /* Fill in the semaphore name.  */
    for (i = 0; i < NU_MAX_NAME; i++)
        semaphore -> sm_name[i] =  name[i];

    /* Setup the initial semaphore instance count.  */
    semaphore -> sm_semaphore_count =  initial_count;

    /* Regist  semaphore initial count */
    semaphore -> sm_init_count = initial_count;

    /* Setup the semaphore suspension type.  */
    if (suspend_type == NU_FIFO)

        /* FIFO suspension is selected, setup the flag accordingly.  */
        semaphore -> sm_fifo_suspend =  NU_TRUE;
    else

        /* Priority suspension is selected.  */
        semaphore -> sm_fifo_suspend =  NU_FALSE;

    /* Clear the suspension list pointer.  */
    semaphore -> sm_suspension_list =  NU_NULL;

    /* Clear the number of tasks waiting on the semaphore counter.  */
    semaphore -> sm_tasks_waiting =  0;

    /* Initialize link pointers.  */
    semaphore -> sm_created.cs_previous =    NU_NULL;
    semaphore -> sm_created.cs_next =        NU_NULL;

    /* Protect against access to the list of created semaphores.  */
    TCT_Protect(&SMD_List_Protect);

    /* At this point the semaphore is completely built.  The ID can now be
       set and it can be linked into the created semaphore list.  */
    semaphore -> sm_id =  SM_SEMAPHORE_ID;

    /* Link the semaphore into the list of created semaphores and increment the
       total number of semaphores in the system.  */
    CSC_Place_On_List(&SMD_Created_Semaphores_List,&(semaphore -> sm_created));
    SMD_Total_Semaphores++;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpSema(RT_PROF_CREATE_SEMAPHORE,semaphore, RT_PROF_OK);
#endif

    /* Release protection against access to the list of created semaphores.  */
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
/*      SMC_Delete_Semaphore                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deletes a semaphore and removes it from the list   */
/*      of created semaphores.  All tasks suspended on the semaphore are */
/*      resumed.  Note that this function does not free the memory       */
/*      associated with the semaphore control block.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      SMCE_Delete_Semaphore               Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove node from list        */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Protect                         Protect created list         */
/*      TCT_Set_Current_Protect             Modify current protection    */
/*      TCT_System_Protect                  Setup system protection      */
/*      TCT_System_Unprotect                Release system protection    */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      semaphore_ptr                       Semaphore control block ptr  */
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
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options, changed                  */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  SMC_Delete_Semaphore(NU_SEMAPHORE *semaphore_ptr)
{

R1 SM_SCB      *semaphore;                  /* Semaphore control blk ptr */
SM_SUSPEND     *suspend_ptr;                /* Suspend block pointer     */
SM_SUSPEND     *next_ptr;                   /* Next suspend block        */
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
    HIC_Make_History_Entry(NU_DELETE_SEMAPHORE_ID, (UNSIGNED) semaphore,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect against access to the semaphore.  */
    TCT_System_Protect();

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpSema(RT_PROF_DELETE_SEMAPHORE,semaphore, RT_PROF_OK);
#endif

    /* Clear the semaphore ID.  */
    semaphore -> sm_id =  0;

    /* Release protection.  */
    TCT_Unprotect();

    /* Protect against access to the list of created semaphores.  */
    TCT_Protect(&SMD_List_Protect);

    /* Remove the semaphore from the list of created semaphores.  */
    CSC_Remove_From_List(&SMD_Created_Semaphores_List,
                                        &(semaphore -> sm_created));

    /* Decrement the total number of created semaphores.  */
    SMD_Total_Semaphores--;

    /* Pickup the suspended task pointer list.  */
    suspend_ptr =  semaphore -> sm_suspension_list;

    /* Walk the chain task(s) currently suspended on the semaphore.  */
    preempt =  0;
    while (suspend_ptr)
    {

        /* Protect against system access.  */
        TCT_System_Protect();

        /* Resume the suspended task.  Insure that the status returned is
           NU_SEMAPHORE_DELETED.  */
        suspend_ptr -> sm_return_status =  NU_SEMAPHORE_DELETED;

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

        /* Modify current protection.  */
        TCT_Set_Current_Protect(&SMD_List_Protect);

        /* Clear the system protection.  */
        TCT_System_Unprotect();
    }

    /* Determine if preemption needs to occur.  */
    if (preempt)

        /* Transfer control to system to facilitate preemption.  */
        TCT_Control_To_System();

    /* Release protection against access to the list of created semaphores. */
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
/*      SMC_Obtain_Semaphore                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function obtains an instance of the semaphore.  An instance */
/*      corresponds to decrementing the counter by 1.  If the counter is */
/*      greater than zero at the time of this call, this function can be */
/*      completed immediately.  Otherwise, suspension is possible.       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      SMCE_Obtain_Semaphore               Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Place on suspend list        */
/*      CSC_Priority_Place_On_List          Place on priority list       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Suspend_Task                    Suspend calling task         */
/*      TCC_Task_Priority                   Obtain task's priority       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Current_Thread                  Pickup current thread pointer*/
/*      TCT_System_Protect                  Protect semaphore            */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      semaphore_ptr                       Semaphore control block ptr  */
/*      suspend                             Suspension option if full    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If service is successful     */
/*      NU_UNAVAILABLE                      If an instance of the        */
/*                                            semaphore is not available */
/*      NU_TIMEOUT                          If timeout on service        */
/*      NU_SEMAPHORE_DELETED                If semaphore deleted during  */
/*                                            suspension                 */
/*      NU_SEMAPHORE_RESET                  If semaphore reset during    */
/*                                            suspension                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options, changed                  */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  SMC_Obtain_Semaphore(NU_SEMAPHORE *semaphore_ptr,  UNSIGNED suspend)
{

R1 SM_SCB      *semaphore;                  /* Semaphore control blk ptr */
R2 SM_SUSPEND  *suspend_ptr;                /* Suspend block pointer     */
SM_SUSPEND      suspend_block;              /* Allocate suspension block */
TC_TCB         *task;                       /* Task pointer              */
STATUS          status;                     /* Completion status         */
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
    HIC_Make_History_Entry(NU_OBTAIN_SEMAPHORE_ID, (UNSIGNED) semaphore,
                                        (UNSIGNED) suspend, (UNSIGNED) 0);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Protect against simultaneous access to the semaphore.  */
    TCT_System_Protect();

    /* Determine if the semaphore has an instance (can be decremented).  */
    if (semaphore -> sm_semaphore_count)
    {

        /* Semaphore available.  Decrement and return to the caller.  */
        semaphore -> sm_semaphore_count--;
#ifdef INCLUDE_PROVIEW
        _RTProf_DumpSema(RT_PROF_OBTAIN_SEMAPHORE,semaphore, RT_PROF_OK);
#endif

    }
    else
    {

        /* Semaphore is not available.  Determine if suspension is required. */
        if (suspend)
        {

            /* Suspension is selected.  */

            /* Increment the number of tasks waiting.  */
            semaphore -> sm_tasks_waiting++;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpSema(RT_PROF_OBTAIN_SEMAPHORE,semaphore , RT_PROF_WAIT);
#endif

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> sm_semaphore =                semaphore;
            suspend_ptr -> sm_suspend_link.cs_next =     NU_NULL;
            suspend_ptr -> sm_suspend_link.cs_previous = NU_NULL;
            task =                            (TC_TCB *) TCT_Current_Thread();
            suspend_ptr -> sm_suspended_task =           task;

            /* Determine if priority or FIFO suspension is associated with the
               semaphore.  */
            if (semaphore -> sm_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this semaphore.  */
                CSC_Place_On_List((CS_NODE **)
                        &(semaphore -> sm_suspension_list),
                                        &(suspend_ptr -> sm_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> sm_suspend_link.cs_priority =
                                                    TCC_Task_Priority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                        &(semaphore -> sm_suspension_list),
                                        &(suspend_ptr -> sm_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the semaphore.  */
            TCC_Suspend_Task((NU_TASK *) task, NU_SEMAPHORE_SUSPEND,
                                        SMC_Cleanup, suspend_ptr, suspend);

            /* Pickup the return status.  */
            status =  suspend_ptr -> sm_return_status;
        }
        else
        {
            /* No suspension requested.  Simply return an error status.  */
            status =  NU_UNAVAILABLE;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpSema(RT_PROF_OBTAIN_SEMAPHORE, semaphore,RT_PROF_FAIL);
#endif
        }
    }

    /* Release protection against access to the semaphore.  */
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
/*      SMC_Release_Semaphore                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function releases a previously obtained semaphore.  If one  */
/*      or more tasks are waiting, the first task is given the released  */
/*      instance of the semaphore.  Otherwise, the semaphore instance    */
/*      counter is simply incremented.                                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      SMCE_Release_Semaphore              Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove from suspend list     */
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
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options, changed                  */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  SMC_Release_Semaphore(NU_SEMAPHORE *semaphore_ptr)
{

R1 SM_SCB      *semaphore;                  /* Semaphore control blk ptr */
R2 SM_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
STATUS          preempt;                    /* Preemption flag           */
STATUS          status;                     /* Completion status         */
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
    HIC_Make_History_Entry(NU_RELEASE_SEMAPHORE_ID, (UNSIGNED) semaphore,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Protect against simultaneous access to the semaphore.  */
    TCT_System_Protect();

    /* Determine if another task is waiting on the semaphore.  */
    if (semaphore -> sm_tasks_waiting)
    {

        /* Yes, another task is waiting for an instance of the semaphore.  */

        /* Decrement the number of tasks waiting counter.  */
        semaphore -> sm_tasks_waiting--;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpSema(RT_PROF_RELEASE_SEMAPHORE, semaphore,RT_PROF_OK);
#endif

        /* Remove the first suspended block from the list.  */
        suspend_ptr =  semaphore -> sm_suspension_list;
        CSC_Remove_From_List((CS_NODE **) &(semaphore -> sm_suspension_list),
                                &(suspend_ptr -> sm_suspend_link));

        /* Setup the appropriate return value.  */
        suspend_ptr -> sm_return_status =  NU_SUCCESS;

        /* Resume the suspended task.  */
        preempt =
            TCC_Resume_Task((NU_TASK *) suspend_ptr -> sm_suspended_task,
                                                       NU_SEMAPHORE_SUSPEND);

        /* Determine if a preempt condition is present.  */
        if (preempt)

            /* Transfer control to the system if the resumed task function
               detects a preemption condition.  */
            TCT_Control_To_System();
    }
    else
    {

        /* Check if the semaphore count larger than initial number.  */
		/* 由于标准的多任务还是允许release的情况下不断的增加的，所以不能加入条件 */
        //if (semaphore->sm_semaphore_count < semaphore->sm_init_count)

            /* Increment the semaphore instance counter.  */
            semaphore -> sm_semaphore_count++;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpSema(RT_PROF_RELEASE_SEMAPHORE, semaphore,RT_PROF_OK);
#endif
    }

    /* Release protection against access to the semaphore.  */
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
/*      SMC_Cleanup                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible for removing a suspension block     */
/*      from a semaphore.  It is not called unless a timeout or a task   */
/*      terminate is in progress.  Note that protection is already in    */
/*      effect - the same protection at suspension time.  This routine   */
/*      must be called from Supervisor mode in Supervisor/User mode      */
/*      switching kernels.                                               */
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
VOID  SMC_Cleanup(VOID *information)
{

SM_SUSPEND      *suspend_ptr;               /* Suspension block pointer  */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Use the information pointer as a suspend pointer.  */
    suspend_ptr =  (SM_SUSPEND *) information;

    /* By default, indicate that the service timed-out.  It really does not
       matter if this function is called from a terminate request since
       the task does not resume.  */
    suspend_ptr -> sm_return_status =  NU_TIMEOUT;

    /* Decrement the number of tasks waiting counter.  */
    (suspend_ptr -> sm_semaphore) -> sm_tasks_waiting--;

    /* Unlink the suspend block from the suspension list.  */
    CSC_Remove_From_List((CS_NODE **)
                &((suspend_ptr -> sm_semaphore) -> sm_suspension_list),
                                &(suspend_ptr -> sm_suspend_link));

    /* Return to user mode */
    NU_USER_MODE();
}






