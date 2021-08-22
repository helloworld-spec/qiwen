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
/*      pmc.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PM - Partition Memory Management                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core routines for the Partition Memory    */
/*      Management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      PMC_Create_Partition_Pool           Create a Partition Pool      */
/*      PMC_Delete_Partition_Pool           Delete a Partition Pool      */
/*      PMC_Allocate_Partition              Allocate a partition from a  */
/*                                            pool                       */
/*      PMC_Deallocate_Partition            Deallocate a partition from  */
/*                                            a pool                     */
/*      PMC_Cleanup                         Cleanup on timeout or a      */
/*                                            terminate condition        */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      pm_extr.h                           Partition functions          */
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
#include        "pm_extr.h"                 /* Partition functions       */
#include        "hi_extr.h"                 /* History functions         */
#include        "profiler.h"                /* ProView interface         */


/* Define external inner-component global data references.  */

extern CS_NODE         *PMD_Created_Pools_List;
extern UNSIGNED         PMD_Total_Pools;
extern TC_PROTECT       PMD_List_Protect;


/* Define internal component function prototypes.  */

VOID    PMC_Cleanup(VOID *information);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PMC_Create_Partition_Pool                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a memory partition pool and then places it */
/*      on the list of created partition pools.                          */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      PMCE_Create_Partition_Pool          Error checking shell         */
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
/*      pool_ptr                            Partition pool control block */
/*                                            pointer                    */
/*      name                                Partition pool name          */
/*      start_address                       Starting address of the pool */
/*      pool_size                           Number of bytes in the pool  */
/*      partition_size                      Number of bytes in each      */
/*                                            partition of the pool      */
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
STATUS  PMC_Create_Partition_Pool(NU_PARTITION_POOL *pool_ptr, CHAR *name,
                        VOID *start_address, UNSIGNED pool_size,
                        UNSIGNED partition_size, OPTION suspend_type)
{

R1 PM_PCB      *pool;                       /* Pool control block ptr    */
INT             i;                          /* Working index variable    */
BYTE_PTR        pointer;                    /* Working byte pointer      */
PM_HEADER      *header_ptr;                 /* Partition block header ptr*/
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pool pointer into internal pointer.  */
    pool =  (PM_PCB *) pool_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CREATE_PARTITION_POOL_ID, (UNSIGNED) pool,
                                (UNSIGNED) name, (UNSIGNED) start_address);

#endif

    /* First, clear the partition pool ID just in case it is an old
       pool control block.  */
    pool -> pm_id =             0;

    /* Fill in the partition pool name.  */
    for (i = 0; i < NU_MAX_NAME; i++)
        pool -> pm_name[i] =  name[i];

    /* Save the starting address and size parameters in the partition control
       block.  */
    pool -> pm_start_address =   start_address;
    pool -> pm_pool_size =       pool_size;
    pool -> pm_partition_size =  partition_size;

    /* Setup the partition pool suspension type.  */
    if (suspend_type == NU_FIFO)

        /* FIFO suspension is selected, setup the flag accordingly.  */
        pool -> pm_fifo_suspend =  NU_TRUE;
    else

        /* Priority suspension is selected.  */
        pool -> pm_fifo_suspend =  NU_FALSE;

    /* Clear the suspension list pointer.  */
    pool -> pm_suspension_list =  NU_NULL;

    /* Clear the number of tasks waiting on the partition pool.  */
    pool -> pm_tasks_waiting =  0;

    /* Initialize link pointers.  */
    pool -> pm_created.cs_previous =    NU_NULL;
    pool -> pm_created.cs_next =        NU_NULL;

    /* Initialize the partition parameters.  */
    pool -> pm_available =       0;
    pool -> pm_allocated =       0;
    pool -> pm_available_list =  NU_NULL;

    /* Convert the supplied partition size into something that is evenly
       divisible by the sizeof an UNSIGNED data element.  This insures
       UNSIGNED alignment.  */
    partition_size =
        ((partition_size + sizeof(UNSIGNED) - 1)/sizeof(UNSIGNED)) *
                                                        sizeof(UNSIGNED);

    /* Loop to build and link as many partitions as possible from within the
       specified memory area.  */
    pointer =  (BYTE_PTR)  start_address;
    while (pool_size >= (PM_OVERHEAD + partition_size))
    {

        /* There is room for another partition.  */

        /* Cast the current pointer variable to a header pointer.  */
        header_ptr =  (PM_HEADER *) pointer;

        /* Now, build a header and link it into the partition pool
           available list- at the front.  */
        header_ptr -> pm_partition_pool =  pool;
        header_ptr -> pm_next_available =  pool -> pm_available_list;
        pool -> pm_available_list =        header_ptr;

        /* Increment the number of partitions available in the pool.  */
        pool -> pm_available++;

        /* Decrement the number of bytes remaining in the pool.  */
        pool_size =  pool_size - (PM_OVERHEAD + partition_size);

        /* Increment the working pointer to the next partition position.  */
        pointer =  pointer + (PM_OVERHEAD + partition_size);
    }

    /* Protect against access to the list of created partition pools.  */
    TCT_Protect(&PMD_List_Protect);

    /* At this point the partition pool is completely built.  The ID can
       now be set and it can be linked into the created partition pool list. */
    pool -> pm_id =  PM_PARTITION_ID;

    /* Link the partition pool into the list of created partition pools and
       increment the total number of pools in the system.  */
    CSC_Place_On_List(&PMD_Created_Pools_List, &(pool -> pm_created));
    PMD_Total_Pools++;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpPartitionPool(RT_PROF_CREATE_PARTITION_POOL,pool,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */
    /* Release protection against access to the list of created partition
       pools.  */
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
/*      PMC_Delete_Partition_Pool                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deletes a memory partition pool and removes it from*/
/*      the list of created partition pools.  All tasks suspended on the */
/*      partition pool are resumed with the appropriate error status.    */
/*      Note that this function does not free any memory associated with */
/*      either the pool area or the pool control block.                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      PMCE_Delete_Partition_Pool          Error checking shell         */
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
/*      pool_ptr                            Partition pool control block */
/*                                            pointer                    */
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
STATUS  PMC_Delete_Partition_Pool(NU_PARTITION_POOL *pool_ptr)
{

R1 PM_PCB      *pool;                       /* Pool control block ptr    */
PM_SUSPEND     *suspend_ptr;                /* Suspend block pointer     */
PM_SUSPEND     *next_ptr;                   /* Next suspend block        */
STATUS          preempt;                    /* Status for resume call    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pool pointer into internal pointer.  */
    pool =  (PM_PCB *) pool_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_DELETE_PARTITION_POOL_ID, (UNSIGNED) pool,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect against simultaneous access to the partition pool.  */
    TCT_System_Protect();

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpPartitionPool(RT_PROF_DELETE_PARTITION_POOL,pool,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

    /* Clear the partition pool ID.  */
    pool -> pm_id =  0;

    /* Release protection.  */
    TCT_Unprotect();

    /* Protect against access to the list of created partition pools.  */
    TCT_Protect(&PMD_List_Protect);

    /* Remove the partition pool from the list of created partition pools.  */
    CSC_Remove_From_List(&PMD_Created_Pools_List, &(pool -> pm_created));

    /* Decrement the total number of created partition pools.  */
    PMD_Total_Pools--;

    /* Pickup the suspended task pointer list.  */
    suspend_ptr =  pool -> pm_suspension_list;

    /* Walk the chain task(s) currently suspended on the partition pool.  */
    preempt =  0;
    while (suspend_ptr)
    {

        /* Protect against system access.  */
        TCT_System_Protect();

        /* Resume the suspended task.  Insure that the status returned is
           NU_POOL_DELETED.  */
        suspend_ptr -> pm_return_pointer =  NU_NULL;
        suspend_ptr -> pm_return_status =   NU_POOL_DELETED;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (PM_SUSPEND *) (suspend_ptr -> pm_suspend_link.cs_next);

        /* Resume the specified task.  */
        preempt =  preempt |
            TCC_Resume_Task((NU_TASK *) suspend_ptr -> pm_suspended_task,
                                                NU_PARTITION_SUSPEND);

        /* Determine if the next is the same as the current pointer.  */
        if (next_ptr == pool -> pm_suspension_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  NU_NULL;
        else

            /* Move the next pointer into the suspend block pointer.  */
            suspend_ptr =  next_ptr;

        /* Modify current protection.  */
        TCT_Set_Current_Protect(&PMD_List_Protect);

        /* Clear the system protection.  */
        TCT_System_Unprotect();
    }

    /* Determine if preemption needs to occur.  */
    if (preempt)

        /* Transfer control to system to facilitate preemption.  */
        TCT_Control_To_System();

    /* Release protection against access to the list of created partition
       pools. */
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
/*      PMC_Allocate_Partition                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function allocates a memory partition from the specified    */
/*      memory partition pool.  If a memory partition is currently       */
/*      available, this function is completed immediately.  Otherwise,   */
/*      if there are no partitions currently available, suspension is    */
/*      possible.                                                        */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      PMCE_Allocate_Partition             Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Place on suspend list        */
/*      CSC_Priority_Place_On_List          Place on priority list       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Suspend_Task                    Suspend calling task         */
/*      TCC_Task_Priority                   Pickup task's priority       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Current_Thread                  Pickup current thread pointer*/
/*      TCT_System_Protect                  Protect partition pool       */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pool_ptr                            Memory partition pool pointer*/
/*      return_pointer                      Pointer to the destination   */
/*                                            memory pointer             */
/*      suspend                             Suspension option if full    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If service is successful     */
/*      NU_NO_PARTITION                     No partitions are available  */
/*      NU_TIMEOUT                          If timeout on service        */
/*      NU_POOL_DELETED                     If partition pool deleted    */
/*                                            during suspension          */
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
STATUS  PMC_Allocate_Partition(NU_PARTITION_POOL *pool_ptr,
                                VOID **return_pointer, UNSIGNED suspend)
{

R1 PM_PCB      *pool;                       /* Pool control block ptr    */
R2 PM_SUSPEND  *suspend_ptr;                /* Suspend block pointer     */
PM_SUSPEND      suspend_block;              /* Allocate suspension block */
R3 PM_HEADER   *partition_ptr;              /* Pointer to partition      */
TC_TCB         *task;                       /* Task pointer              */
STATUS          status;                     /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pool pointer into internal pointer.  */
    pool =  (PM_PCB *) pool_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_ALLOCATE_PARTITION_ID, (UNSIGNED) pool,
                                (UNSIGNED) return_pointer, (UNSIGNED) suspend);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Protect against simultaneous access to the partition pool.  */
    TCT_System_Protect();

    /* Determine if there is an available memory partition.  */
    if (pool -> pm_available)
    {

        /* Partition available.  */

        /* Decrement the available count.  */
        pool -> pm_available--;

        /* Increment the allocated count.  */
        pool -> pm_allocated++;

        /* Unlink the first memory partition and return the pointer to the
           caller.  */
        partition_ptr =              pool -> pm_available_list;
        pool -> pm_available_list =  partition_ptr -> pm_next_available;
        partition_ptr -> pm_next_available =  NU_NULL;

        /* Return a memory address to the caller.  */
        *return_pointer =  (VOID *) (((BYTE_PTR) partition_ptr) + PM_OVERHEAD);

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpPartitionPool(RT_PROF_ALLOCATE_PARTITION,pool,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

    }
    else
    {

        /* A partition is not available.  Determine if suspension is
           required. */
        if (suspend)
        {

            /* Suspension is selected.  */

            /* Increment the number of tasks waiting.  */
            pool -> pm_tasks_waiting++;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpPartitionPool(RT_PROF_ALLOCATE_PARTITION,pool,RT_PROF_WAIT);
#endif /* INCLUDE_PROVIEW */
            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> pm_partition_pool =           pool;
            suspend_ptr -> pm_suspend_link.cs_next =     NU_NULL;
            suspend_ptr -> pm_suspend_link.cs_previous = NU_NULL;
            task =                            (TC_TCB *) TCT_Current_Thread();
            suspend_ptr -> pm_suspended_task =           task;

            /* Determine if priority or FIFO suspension is associated with the
               partition pool.  */
            if (pool -> pm_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this partition pool.  */
                CSC_Place_On_List((CS_NODE **)
                        &(pool -> pm_suspension_list),
                                        &(suspend_ptr -> pm_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> pm_suspend_link.cs_priority =
                                                     TCC_Task_Priority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                        &(pool -> pm_suspension_list),
                                        &(suspend_ptr -> pm_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the partition pool.  */
            TCC_Suspend_Task((NU_TASK *) task, NU_PARTITION_SUSPEND,
                                        PMC_Cleanup, suspend_ptr, suspend);

            /* Pickup the return status.  */
            status =            suspend_ptr -> pm_return_status;
            *return_pointer =   suspend_ptr -> pm_return_pointer;
        }
        else
        {
            /* No suspension requested.  Simply return an error status.  */
            status =  NU_NO_PARTITION;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpPartitionPool(RT_PROF_ALLOCATE_PARTITION,pool,RT_PROF_FAIL);
#endif /* INCLUDE_PROVIEW */
        }
    }

    /* Release protection of the partition pool.  */
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
/*      PMC_Deallocate_Partition                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deallocates a previously allocated partition.  If  */
/*      there is a task waiting for a partition, the partition is simply */
/*      given to the waiting task and the waiting task is resumed.       */
/*      Otherwise, the partition is returned to the partition pool.      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      PMCE_Deallocate_Partition           Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove from suspend list     */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_System_Protect                  Protect partition pool       */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      partition                           Pointer to partition memory  */
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
/*      03-01-1994      Added register options, changed                  */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  PMC_Deallocate_Partition(VOID *partition)
{

R1 PM_PCB      *pool;                       /* Pool pointer              */
R3 PM_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
R2 PM_HEADER   *header_ptr;                 /* Pointer to partition hdr  */
STATUS          preempt;                    /* Preemption flag           */
STATUS          status;                     /* Completion status         */
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
    HIC_Make_History_Entry(NU_DEALLOCATE_PARTITION_ID, (UNSIGNED) partition,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Pickup the associated pool's pointer.  It is inside the header of
       each partition.  */
    header_ptr =  (PM_HEADER *) (((BYTE_PTR) partition) - PM_OVERHEAD);
    pool =        header_ptr -> pm_partition_pool;

    /* Protect against simultaneous access to the partition pool.  */
    TCT_System_Protect();

    /* Determine if another task is waiting for a partition from the pool.  */
    if (pool -> pm_tasks_waiting)
    {

        /* Yes, another task is waiting for a partition from the pool.  */

        /* Decrement the number of tasks waiting counter.  */
        pool -> pm_tasks_waiting--;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpPartitionPool(RT_PROF_DEALLOCATE_PARTITION,pool,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

        /* Remove the first suspended block from the list.  */
        suspend_ptr =  pool -> pm_suspension_list;
        CSC_Remove_From_List((CS_NODE **) &(pool -> pm_suspension_list),
                                &(suspend_ptr -> pm_suspend_link));

        /* Setup the appropriate return value.  */
        suspend_ptr -> pm_return_status =   NU_SUCCESS;
        suspend_ptr -> pm_return_pointer =  partition;

        /* Resume the suspended task.  */
        preempt =
           TCC_Resume_Task((NU_TASK *) suspend_ptr -> pm_suspended_task,
                                                       NU_PARTITION_SUSPEND);

        /* Determine if a preempt condition is present.  */
        if (preempt)

            /* Transfer control to the system if the resumed task function
               detects a preemption condition.  */
            TCT_Control_To_System();
    }
    else
    {

        /* Increment the available partitions counter.  */
        pool -> pm_available++;

        /* Decrement the allocated partitions counter.  */
        pool -> pm_allocated--;

        /* Place the partition back on the available list.  */
        header_ptr -> pm_next_available =  pool -> pm_available_list;
        pool -> pm_available_list =        header_ptr;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpPartitionPool(RT_PROF_DEALLOCATE_PARTITION,pool,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

    }

    /* Release protection of the partition pool.  */
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
/*      PMC_Cleanup                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible for removing a suspension block     */
/*      from a partition pool.  It is not called unless a timeout or     */
/*      a task terminate is in progress.  Note that protection is        */
/*      already in effect - the same protection at suspension time.      */
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
VOID  PMC_Cleanup(VOID *information)
{

PM_SUSPEND      *suspend_ptr;               /* Suspension block pointer  */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Use the information pointer as a suspend pointer.  */
    suspend_ptr =  (PM_SUSPEND *) information;

    /* By default, indicate that the service timed-out.  It really does not
       matter if this function is called from a terminate request since
       the task does not resume.  */
    suspend_ptr -> pm_return_status =   NU_TIMEOUT;
    suspend_ptr -> pm_return_pointer =  NU_NULL;

    /* Decrement the number of tasks waiting counter.  */
    (suspend_ptr -> pm_partition_pool) -> pm_tasks_waiting--;

    /* Unlink the suspend block from the suspension list.  */
    CSC_Remove_From_List((CS_NODE **)
                &((suspend_ptr -> pm_partition_pool) -> pm_suspension_list),
                                &(suspend_ptr -> pm_suspend_link));

    /* Return to user mode */
    NU_USER_MODE();
}





