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
/*      dmc.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DM - Dynamic Memory Management                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core routines for the Dynamic Memory      */
/*      Management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      DMC_Create_Memory_Pool              Create a dynamic memory pool */
/*      DMC_Delete_Memory_Pool              Delete a dynamic memory pool */
/*      DMC_Allocate_Memory                 Allocate a memory block from */
/*                                            a dynamic memory pool      */
/*      DMC_Deallocate_Memory               Deallocate a memory block    */
/*                                            from a dynamic memory pool */
/*      DMC_Cleanup                         Cleanup on timeout or a      */
/*                                            terminate condition        */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      dm_extr.h                           Partition functions          */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      DATE                    REMARKS                                  */
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
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "dm_extr.h"                 /* Dynamic memory functions  */
#include        "hi_extr.h"                 /* History functions         */
#include        "profiler.h"                /* ProView interface         */


/* Define external inner-component global data references.  */

extern CS_NODE         *DMD_Created_Pools_List;
extern UNSIGNED         DMD_Total_Pools;
extern TC_PROTECT       DMD_List_Protect;


/* Define internal component function prototypes.  */

VOID    DMC_Cleanup(VOID *information);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      DMC_Create_Memory_Pool                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a dynamic memory pool and then places it   */
/*      on the list of created dynamic memory pools.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      DMCE_Create_Memory_Pool             Error checking shell         */
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
/*      pool_ptr                            Memory pool control block    */
/*                                            pointer                    */
/*      name                                Memory pool name             */
/*      start_address                       Starting address of the pool */
/*      pool_size                           Number of bytes in the pool  */
/*      min_allocation                      Minimum allocation size      */
/*      suspend_type                        Suspension type              */
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
/*                      added register options,                          */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  DMC_Create_Memory_Pool(NU_MEMORY_POOL *pool_ptr, CHAR *name,
                        VOID *start_address, UNSIGNED pool_size,
                        UNSIGNED min_allocation, OPTION suspend_type)
{

R1 DM_PCB      *pool;                       /* Pool control block ptr    */
INT             i;                          /* Working index variable    */
DM_HEADER      *header_ptr;                 /* Partition block header ptr*/
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pool pointer into internal pointer.  */
    pool =  (DM_PCB *) pool_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CREATE_MEMORY_POOL_ID, (UNSIGNED) pool,
                        (UNSIGNED) start_address, (UNSIGNED) pool_size);

#endif

    /* First, clear the partition pool ID just in case it is an old
       pool control block.  */
    pool -> dm_id =             0;

    /* Fill in the partition pool name.  */
    for (i = 0; i < NU_MAX_NAME; i++)
        pool -> dm_name[i] =  name[i];

    /* Convert the pool's size into something that is evenly divisible by
       the sizeof an UNSIGNED data element.  */
    pool_size =  (pool_size/sizeof(UNSIGNED)) * sizeof(UNSIGNED);

    /* Save the starting address and size parameters in the dynamic memory
       control block.  */
    pool -> dm_start_address =   start_address;
    pool -> dm_pool_size =       pool_size;
    pool -> dm_min_allocation =
                ((min_allocation + sizeof(UNSIGNED) - 1)/sizeof(UNSIGNED)) *
                                                              sizeof(UNSIGNED);

    /* Setup the dynamic memory pool suspension type.  */
    if (suspend_type == NU_FIFO)

        /* FIFO suspension is selected, setup the flag accordingly.  */
        pool -> dm_fifo_suspend =  NU_TRUE;
    else

        /* Priority suspension is selected.  */
        pool -> dm_fifo_suspend =  NU_FALSE;

    /* Clear the suspension list pointer.  */
    pool -> dm_suspension_list =  NU_NULL;

    /* Clear the number of tasks waiting on the dynamic memory pool.  */
    pool -> dm_tasks_waiting =  0;

    /* Initialize link pointers.  */
    pool -> dm_created.cs_previous =    NU_NULL;
    pool -> dm_created.cs_next =        NU_NULL;

    /* Build a single block that has all of the memory.  */
    header_ptr =  (DM_HEADER *) start_address;

    /* Initialize the memory parameters.  */
    pool -> dm_available =       pool_size - (2 * DM_OVERHEAD);
    pool -> dm_memory_list =     header_ptr;
    pool -> dm_search_ptr =      header_ptr;

    /* Build the block header.  */
    header_ptr -> dm_memory_pool =  pool;
    header_ptr -> dm_memory_free =  NU_TRUE;
    header_ptr -> dm_next_memory =  (DM_HEADER *)
           (((BYTE_PTR) header_ptr) + pool -> dm_available + DM_OVERHEAD);
    header_ptr -> dm_previous_memory =  header_ptr -> dm_next_memory;

    /* Build the small trailer block that prevents block merging when the
       pool wraps around.  Note that the list is circular so searching can
       wrap across the physical end of the memory pool.  */
    header_ptr =  header_ptr -> dm_next_memory;
    header_ptr -> dm_next_memory =  (DM_HEADER *) start_address;
    header_ptr -> dm_previous_memory =  (DM_HEADER *) start_address;
    header_ptr -> dm_memory_pool =  pool;
    header_ptr -> dm_memory_free =  NU_FALSE;

    /* Initialize the protection structure.  */
    pool -> dm_protect.tc_tcb_pointer =  NU_NULL;

    /* Protect against access to the list of created memory pools.  */
    TCT_Protect(&DMD_List_Protect);

    /* At this point the dynamic memory pool is completely built.  The ID can
       now be set and it can be linked into the created dynamic memory
       pool list. */
    pool -> dm_id =  DM_DYNAMIC_ID;

    /* Link the memory pool into the list of created memory pools and
       increment the total number of pools in the system.  */
    CSC_Place_On_List(&DMD_Created_Pools_List, &(pool -> dm_created));
    DMD_Total_Pools++;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpMemoryPool(RT_PROF_CREATE_MEMORY_POOL,pool,RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/
    /* Release protection against access to the list of created memory
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
/*      DMC_Delete_Memory_Pool                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deletes a dynamic memory pool and removes it from  */
/*      the list of created memory pools.  All tasks suspended on the    */
/*      memory pool are resumed with the appropriate error status.       */
/*      Note that this function does not free any memory associated with */
/*      either the pool area or the pool control block.                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      DMCE_Delete_Memory_Pool             Error checking shell         */
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
/*      pool_ptr                            Memory pool control block    */
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
STATUS  DMC_Delete_Memory_Pool(NU_MEMORY_POOL *pool_ptr)
{

R1 DM_PCB      *pool;                       /* Pool control block ptr    */
DM_SUSPEND     *suspend_ptr;                /* Suspend block pointer     */
DM_SUSPEND     *next_ptr;                   /* Next suspend block        */
STATUS          preempt;                    /* Status for resume call    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pool pointer into internal pointer.  */
    pool =  (DM_PCB *) pool_ptr;

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_DELETE_MEMORY_POOL_ID, (UNSIGNED) pool,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect against simultaneous access to the memory pool.  */
    TCT_Protect(&(pool -> dm_protect));

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpMemoryPool(RT_PROF_DELETE_MEMORY_POOL,pool,RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/
    /* Clear the memory pool ID.  */
    pool -> dm_id =  0;

    /* Release protection.  */
    TCT_Unprotect();

    /* Protect against access to the list of created memory pools.  */
    TCT_Protect(&DMD_List_Protect);

    /* Remove the memory pool from the list of created memory pools.  */
    CSC_Remove_From_List(&DMD_Created_Pools_List, &(pool -> dm_created));

    /* Decrement the total number of created memory pools.  */
    DMD_Total_Pools--;

    /* Pickup the suspended task pointer list.  */
    suspend_ptr =  pool -> dm_suspension_list;

    /* Walk the chain task(s) currently suspended on the memory pool.  */
    preempt =  0;
    while (suspend_ptr)
    {

        /* Protect against system access.  */
        TCT_System_Protect();

        /* Resume the suspended task.  Insure that the status returned is
           NU_POOL_DELETED.  */
        suspend_ptr -> dm_return_pointer =  NU_NULL;
        suspend_ptr -> dm_return_status =   NU_POOL_DELETED;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (DM_SUSPEND *) (suspend_ptr -> dm_suspend_link.cs_next);

        /* Resume the specified task.  */
        preempt =  preempt |
            TCC_Resume_Task((NU_TASK *) suspend_ptr -> dm_suspended_task,
                                                NU_MEMORY_SUSPEND);

        /* Determine if the next is the same as the current pointer.  */
        if (next_ptr == pool -> dm_suspension_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  NU_NULL;
        else

            /* Move the next pointer into the suspend block pointer.  */
            suspend_ptr =  next_ptr;

        /* Modify current protection.  */
        TCT_Set_Current_Protect(&DMD_List_Protect);

        /* Clear the system protection.  */
        TCT_System_Unprotect();
    }

    /* Determine if preemption needs to occur.  */
    if (preempt)

        /* Transfer control to system to facilitate preemption.  */
        TCT_Control_To_System();

    /* Release protection against access to the list of created memory
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
/*      DMC_Allocate_Memory                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function allocates memory from the specified dynamic memory */
/*      pool.  If dynamic memory is currently available, this function   */
/*      is completed immediately.  Otherwise, if there is not enough     */
/*      memory currently available, task suspension is possible.         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      DMCE_Allocate_Memory                Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Place on suspend list        */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Suspend_Task                    Suspend calling task         */
/*      TCC_Task_Priority                   Pickup task priority         */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Current_Thread                  Pickup current thread pointer*/
/*      TCT_Protect                         Protect memory pool          */
/*      TCT_Set_Suspend_Protect             Save suspend protection      */
/*      TCT_System_Protect                  Protect system structures    */
/*      TCT_Unprotect                       Release protection           */
/*      TCT_Unprotect_Specific              Release specific protection  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pool_ptr                            Memory pool pointer          */
/*      return_pointer                      Pointer to the destination   */
/*                                            memory pointer             */
/*      size                                Number of bytes requested    */
/*      suspend                             Suspension option if full    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If service is successful     */
/*      NU_NO_MEMORY                        Memory not available         */
/*      NU_TIMEOUT                          If timeout on service        */
/*      NU_POOL_DELETED                     If memory pool deleted       */
/*                                            during suspension          */
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
STATUS  DMC_Allocate_Memory(NU_MEMORY_POOL *pool_ptr, VOID **return_pointer,
                                        UNSIGNED size, UNSIGNED suspend)
{

R1 DM_PCB      *pool;                       /* Pool control block ptr    */
R2 DM_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
DM_SUSPEND      suspend_block;              /* Allocate suspension block */
R4 DM_HEADER   *memory_ptr;                 /* Pointer to memory         */
R3 DM_HEADER   *new_ptr;                    /* New split block pointer   */
UNSIGNED        free_size;                  /* Size of block found       */
TC_TCB         *task;                       /* Task pointer              */
STATUS          status;                     /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pool pointer into internal pointer.  */
    pool =  (DM_PCB *) pool_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_ALLOCATE_MEMORY_ID, (UNSIGNED) pool,
                                (UNSIGNED) return_pointer, (UNSIGNED) size);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Adjust the request to a size evenly divisible by the number of bytes
       in an UNSIGNED data element.  Also, check to make sure it is of the
       minimum size.  */
    if (size < pool -> dm_min_allocation)

        /* Change size to the minimum allocation.  */
        size =  pool -> dm_min_allocation;
    else

        /* Insure that size is a multiple of the UNSIGNED size.  */
        size =
           ((size + sizeof(UNSIGNED) - 1)/sizeof(UNSIGNED)) * sizeof(UNSIGNED);

    /* Protect against simultaneous access to the memory pool.  */
    TCT_Protect(&(pool -> dm_protect));

    /* Search the memory list for the first available block of memory that
       satisfies the request.  Note that blocks are merged during the
       deallocation function.  */
    memory_ptr =  pool -> dm_search_ptr;
    do
    {

        /* Determine if the block is free and if it can satisfy the request. */
        if (memory_ptr -> dm_memory_free)

            /* Calculate the free block size.  */
            free_size =  (((BYTE_PTR) (memory_ptr -> dm_next_memory)) -
                           ((BYTE_PTR) memory_ptr)) - DM_OVERHEAD;
        else

            /* There are no free bytes available.  */
            free_size =  0;

        /* Determine if the search should continue.  */
        if (free_size < size)

            /* Large enough block has not been found.  Move the search
               pointer to the next block.  */
            memory_ptr =  memory_ptr -> dm_next_memory;
    } while((free_size < size) && (memory_ptr != pool -> dm_search_ptr));

    /* Determine if the memory is available.  */
    if (free_size >= size)
    {

        /* A block that satisfies the request has been found.  */

        /* Determine if the block needs to be split.  */
        if (free_size >= (size + DM_OVERHEAD + pool -> dm_min_allocation))
        {

            /* Yes, split the block.  */
            new_ptr =  (DM_HEADER *) (((BYTE_PTR) memory_ptr) + size +
                                                DM_OVERHEAD);

            /* Mark the new block as free.  */
            new_ptr -> dm_memory_free =  NU_TRUE;

            /* Put the pool pointer into the new block.  */
            new_ptr -> dm_memory_pool =  pool;

            /* Build the necessary pointers.  */
            new_ptr -> dm_previous_memory =  memory_ptr;
            new_ptr -> dm_next_memory =      memory_ptr -> dm_next_memory;
            (new_ptr -> dm_next_memory) -> dm_previous_memory =  new_ptr;
            memory_ptr -> dm_next_memory =   new_ptr;

            /* Decrement the available byte count.  */
            pool -> dm_available =  pool -> dm_available - size - DM_OVERHEAD;
        }
        else

            /* Decrement the entire free size from the available bytes
               count.  */
            pool -> dm_available =  pool -> dm_available - free_size;

        /* Mark the allocated block as not available.  */
        memory_ptr -> dm_memory_free =  NU_FALSE;

        /* Should the search pointer be moved?   */
        if (pool -> dm_search_ptr == memory_ptr)

            /* Move the search pointer to the next free memory slot.  */
            pool -> dm_search_ptr =  memory_ptr -> dm_next_memory;

        /* Return a memory address to the caller.  */
        *return_pointer =  (VOID *) (((BYTE_PTR) memory_ptr) + DM_OVERHEAD);
#ifdef INCLUDE_PROVIEW
        _RTProf_DumpMemoryPool(RT_PROF_ALLOCATE_MEMORY,pool,RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/
    }
    else
    {

        /* Enough dynamic memory is not available.  Determine if suspension is
           required. */
        if (suspend)
        {

            /* Suspension is selected.  */

            /* Increment the number of tasks waiting.  */
            pool -> dm_tasks_waiting++;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpMemoryPool(RT_PROF_ALLOCATE_MEMORY,pool,RT_PROF_WAIT);
#endif /*INCLUDE_PROVIEW*/
            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> dm_memory_pool =              pool;
            suspend_ptr -> dm_request_size =             size;
            suspend_ptr -> dm_suspend_link.cs_next =     NU_NULL;
            suspend_ptr -> dm_suspend_link.cs_previous = NU_NULL;
            task =                            (TC_TCB *) TCT_Current_Thread();
            suspend_ptr -> dm_suspended_task =           task;

            /* Determine if priority or FIFO suspension is associated with the
               memory pool.  */
            if (pool -> dm_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this memory pool.  */
                CSC_Place_On_List((CS_NODE **)
                        &(pool -> dm_suspension_list),
                                        &(suspend_ptr -> dm_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> dm_suspend_link.cs_priority =
                                                     TCC_Task_Priority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                        &(pool -> dm_suspension_list),
                                        &(suspend_ptr -> dm_suspend_link));
            }

            /* Protect against system access.  */
            TCT_System_Protect();

            /* Save the list protection in preparation for suspension.  */
            TCT_Set_Suspend_Protect(&(pool -> dm_protect));

            /* Release protection of dynamic memory pool.  */
            TCT_Unprotect_Specific(&(pool -> dm_protect));

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the system protection.  */
            TCC_Suspend_Task((NU_TASK *) task, NU_MEMORY_SUSPEND,
                                        DMC_Cleanup, suspend_ptr, suspend);

            /* Pickup the return status.  */
            status =            suspend_ptr -> dm_return_status;
            *return_pointer =   suspend_ptr -> dm_return_pointer;
        }
        else
        {

            /* No suspension requested.  Simply return an error status.  */
            status =            NU_NO_MEMORY;
            *return_pointer =   NU_NULL;
#ifdef INCLUDE_PROVIEW
            _RTProf_DumpMemoryPool(RT_PROF_ALLOCATE_MEMORY,pool,RT_PROF_FAIL);
#endif /*INCLUDE_PROVIEW*/
        }
    }

    /* Release protection of the memory pool.  */
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
/*      DMC_Deallocate_Memory                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deallocates a previously allocated dynamic memory  */
/*      block.  The deallocated dynamic memory block is merged with any  */
/*      adjacent neighbors.  This insures that there are no consecutive  */
/*      blocks of free memory in the pool (makes the search easier!).    */
/*      If there is a task waiting for dynamic memory, a determination   */
/*      of whether or not the request can now be satisfied is made after */
/*      the deallocation is complete.                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      DMCE_Deallocate_Memory              Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove from suspend list     */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Set_Current_Protect             Set current protection       */
/*      TCT_System_Protect                  Protect system structures    */
/*      TCT_System_Unprotect                Release system protection    */
/*      TCT_Protect                         Protect dynamic memory pool  */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      memory                              Pointer to dynamic memory    */
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
/*      03-01-1994      Added register options, changed                  */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  DMC_Deallocate_Memory(VOID *memory)
{

R1 DM_PCB      *pool;                       /* Pool pointer              */
R3 DM_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
R2 DM_HEADER   *header_ptr;                 /* Pointer to memory hdr     */
R4 DM_HEADER   *new_ptr;                    /* New memory block pointer  */
UNSIGNED        size;                       /* Suspended task request    */
UNSIGNED        free_size;                  /* Amount of free bytes      */
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
    HIC_Make_History_Entry(NU_DEALLOCATE_MEMORY_ID, (UNSIGNED) memory,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Pickup the associated pool's pointer.  It is inside the header of
       each memory.  */
    header_ptr =  (DM_HEADER *) (((BYTE_PTR) memory) - DM_OVERHEAD);
    pool =        header_ptr -> dm_memory_pool;

    /* Protect against simultaneous access to the memory pool.  */
    TCT_Protect(&(pool -> dm_protect));

    /* Mark the memory as available.  */
    header_ptr -> dm_memory_free =  NU_TRUE;

    /* Adjust the available number of bytes.  */
    pool -> dm_available =  pool -> dm_available +
                        (((BYTE_PTR) (header_ptr -> dm_next_memory)) -
                           ((BYTE_PTR) header_ptr)) - DM_OVERHEAD;

    /* Determine if the block can be merged with the previous neighbor.  */
    if ((header_ptr -> dm_previous_memory) -> dm_memory_free)
    {

        /* Adjust the available number of bytes.  */
        pool -> dm_available =  pool -> dm_available + DM_OVERHEAD;

        /* Yes, merge block with previous neighbor.  */
        (header_ptr -> dm_previous_memory) -> dm_next_memory =
                                header_ptr -> dm_next_memory;
        (header_ptr -> dm_next_memory) -> dm_previous_memory =
                                header_ptr -> dm_previous_memory;

        /* Move header pointer to previous.  */
        header_ptr =  header_ptr -> dm_previous_memory;

        /* Adjust the search pointer to the new merged block.  */
        pool -> dm_search_ptr =  header_ptr;
    }

    /* Determine if the block can be merged with the next neighbor.  */
    if ((header_ptr -> dm_next_memory) -> dm_memory_free)
    {

        /* Adjust the available number of bytes.  */
        pool -> dm_available =  pool -> dm_available + DM_OVERHEAD;

        /* Yes, merge block with next neighbor.  */
        new_ptr =  header_ptr -> dm_next_memory;
        (new_ptr -> dm_next_memory) -> dm_previous_memory =
                                                header_ptr;
        header_ptr -> dm_next_memory = new_ptr -> dm_next_memory;

        /* Adjust the search pointer to the new merged block.  */
        pool -> dm_search_ptr =  header_ptr;
    }

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpMemoryPool(RT_PROF_DEALLOCATE_MEMORY,pool,RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/
    /* Determine if another task is waiting for memory from the pool.  */
    suspend_ptr =  pool -> dm_suspension_list;
    preempt =      0;
    while (suspend_ptr)
    {

        /* Yes, another task is waiting for memory from the pool.  Search
           the pool in the same manner as the memory allocation function.  */
        size =        suspend_ptr -> dm_request_size;
        header_ptr =  pool -> dm_search_ptr;
        do
        {

            /* Determine if the block is free and if it can satisfy the request
               of the first task waiting. */
            if (header_ptr -> dm_memory_free)

                /* Calculate the free block size.  */
                free_size =  (((BYTE_PTR) (header_ptr -> dm_next_memory)) -
                               ((BYTE_PTR) header_ptr)) - DM_OVERHEAD;
            else

                /* There are no free bytes available.  */
                free_size =  0;

            /* Determine if the search should continue.  */
            if (free_size < size)

                /* Large enough block has not been found.  Move the search
                   pointer to the next block.  */
                header_ptr =  header_ptr -> dm_next_memory;
        } while((free_size < size) && (header_ptr != pool -> dm_search_ptr));

        /* Determine if the memory is available.  */
        if (free_size >= size)
        {

            /* A block that satisfies the request has been found.  */

            /* Determine if the block needs to be split.  */
            if (free_size >= (size + DM_OVERHEAD + pool -> dm_min_allocation))
            {

                /* Yes, split the block.  */
                new_ptr =  (DM_HEADER *) (((BYTE_PTR) header_ptr) + size +
                                                                  DM_OVERHEAD);

                /* Mark the new block as free.  */
                new_ptr -> dm_memory_free =  NU_TRUE;

                /* Put the pool pointer into the new block.  */
                new_ptr -> dm_memory_pool =  pool;

                /* Build the necessary pointers.  */
                new_ptr -> dm_previous_memory =  header_ptr;
                new_ptr -> dm_next_memory =      header_ptr -> dm_next_memory;
                (new_ptr -> dm_next_memory) -> dm_previous_memory =  new_ptr;
                header_ptr -> dm_next_memory =   new_ptr;

                /* Decrement the available byte count.  */
                pool -> dm_available =  pool -> dm_available -
                                                        size - DM_OVERHEAD;
            }
            else

                /* Decrement the entire free size from the available bytes
                   count.  */
                pool -> dm_available =  pool -> dm_available - free_size;

            /* Mark the allocated block as not available.  */
            header_ptr -> dm_memory_free =  NU_FALSE;

            /* Should the search pointer be moved?   */
            if (pool -> dm_search_ptr == header_ptr)

                /* Move the search pointer to the next free memory slot.  */
                pool -> dm_search_ptr =  header_ptr -> dm_next_memory;

            /* Decrement the number of tasks waiting counter.  */
            pool -> dm_tasks_waiting--;

            /* Remove the first suspended block from the list.  */
            CSC_Remove_From_List((CS_NODE **) &(pool -> dm_suspension_list),
                                        &(suspend_ptr -> dm_suspend_link));

            /* Setup the appropriate return value.  */
            suspend_ptr -> dm_return_status =   NU_SUCCESS;
            suspend_ptr -> dm_return_pointer =  (VOID *)
                                (((BYTE_PTR) header_ptr) + DM_OVERHEAD);

            /* Setup system protect while task is being resumed.  */
            TCT_System_Protect();

            /* Resume the suspended task.  */
            preempt =  preempt |
              TCC_Resume_Task((NU_TASK *) suspend_ptr -> dm_suspended_task,
                                                        NU_MEMORY_SUSPEND);

            /* Switch back to the pool protection.  */
            TCT_Set_Current_Protect(&(pool -> dm_protect));

            /* Release system protection.  */
            TCT_System_Unprotect();

            /* Pickup the next suspension pointer.  */
            suspend_ptr =  pool -> dm_suspension_list;
        }
        else

            /* Not enough memory for suspended task. */
            suspend_ptr =  NU_NULL;
    }

    /* Determine if a preempt condition is present.  */
    if (preempt)

        /* Transfer control to the system if the resumed task function
           detects a preemption condition.  */
        TCT_Control_To_System();

    /* Release protection of the memory pool.  */
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
/*      DMC_Cleanup                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible for removing a suspension block     */
/*      from a memory pool.  It is not called unless a timeout or        */
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
VOID  DMC_Cleanup(VOID *information)
{

DM_SUSPEND      *suspend_ptr;               /* Suspension block pointer  */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Use the information pointer as a suspend pointer.  */
    suspend_ptr =  (DM_SUSPEND *) information;

    /* By default, indicate that the service timed-out.  It really does not
       matter if this function is called from a terminate request since
       the task does not resume.  */
    suspend_ptr -> dm_return_status =   NU_TIMEOUT;
    suspend_ptr -> dm_return_pointer =  NU_NULL;

    /* Decrement the number of tasks waiting counter.  */
    (suspend_ptr -> dm_memory_pool) -> dm_tasks_waiting--;

    /* Unlink the suspend block from the suspension list.  */
    CSC_Remove_From_List((CS_NODE **)
                &((suspend_ptr -> dm_memory_pool) -> dm_suspension_list),
                                &(suspend_ptr -> dm_suspend_link));

    /* Return to user mode */
    NU_USER_MODE();
}







