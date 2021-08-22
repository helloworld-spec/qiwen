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
/*      dms.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DM - Dynamic Memory Management                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the supplemental routines for the Dynamic     */
/*      Memory Management component.                                     */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      DMS_Allocate_Aligned_Memory         Allocate an aligned memory   */
/*                                            block from  a dynamic      */
/*                                            memory pool                */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      dm_extr.h                           Partition functions          */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      01-15-1999      Created initial revision                         */
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


/* Define internal component function prototypes.  */
VOID    DMC_Cleanup(VOID *information);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      DMS_Allocate_Aligned_Memory                                      */
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
/*      alignment                           Required alignment of        */
/*                                            destination memory pointer */
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
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      T. Larsen       01-15-1999      Created initial revision         */
/*      R. Baum         06-06-2002      Fixed problem with DM_OVERHEAD   */
/*                                        allocation.  Added Proview     */
/*                                        support.                       */
/*                                                                       */
/*************************************************************************/
STATUS  DMS_Allocate_Aligned_Memory(NU_MEMORY_POOL *pool_ptr,
                                    VOID **return_pointer, UNSIGNED size,
                                    UNSIGNED alignment, UNSIGNED suspend)
{
R1 DM_PCB      *pool;                       /* Pool control block ptr    */
R2 DM_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
DM_SUSPEND      suspend_block;              /* Allocate suspension block */
R4 DM_HEADER   *memory_ptr;                 /* Pointer to memory         */
R3 DM_HEADER   *new_ptr;                    /* New split block pointer   */
UNSIGNED        free_size;                  /* Size of block found       */
TC_TCB         *task;                       /* Task pointer              */
STATUS          status;                     /* Completion status         */
UNSIGNED        address;                    /* Address of start of block */
UNSIGNED        split_size;                 /* Bytes for front split     */
UNSIGNED        next_aligned;               /* Next aligned block addr   */
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
    HIC_Make_History_Entry(NU_ALLOCATE_ALIGNED_ID, (UNSIGNED) pool,
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

    /* Adjust the requested alignment to one evenly divisible by the number of
       bytes in an UNSIGNED data element. */
    alignment =
        ((alignment + sizeof(UNSIGNED) - 1)/sizeof(UNSIGNED)) * sizeof(UNSIGNED);

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

        /* Free block may be large enough, now check alignment */
        if (free_size >= size)
        {
            address = ((UNSIGNED)(memory_ptr)) + DM_OVERHEAD;

            /* Is this free block, minus the overhead, already aligned? */
            if (address % alignment != 0)
            {
                /* Not aligned, can the free block be split in front? */
                next_aligned = address + (alignment - 1);
                next_aligned /= alignment;
                next_aligned *= alignment;
                split_size = next_aligned - address;

                /* Is space from start of block to aligned location large enough
                   to contain 2 DM_OVERHEAD plus pool -> dm_min_allocation? */
                if (split_size < ((2 * DM_OVERHEAD) + pool -> dm_min_allocation))
                {
                    /* No, so try to make space for overhead and dm_min_allocation */
                    next_aligned = address + (2 * DM_OVERHEAD) +
                                  (pool -> dm_min_allocation) + (alignment - 1);
                    next_aligned /= alignment;
                    next_aligned *= alignment;
                    split_size = next_aligned - address;
                }

                /* Adjust free_size for result of front split */
                if (free_size > split_size)

                    free_size -= split_size;

                else

                    /* Can't adjust block beginning, so keep searching */
                    free_size = 0;
            }
        }

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

        /* Is a front split required? The front split will represent the chunk
           of memory that goes from the last pointer to the aligned address. */
        if(address % alignment != 0)
        {
            /* Not aligned, front split the block, leaving an allocated block. */
            new_ptr = (DM_HEADER*)(((UNSIGNED)(memory_ptr)) + split_size);

            /* Mark the new block as free.  */
            new_ptr -> dm_memory_free =  NU_TRUE;

            /* Put the pool pointer into the new block.  */
            new_ptr -> dm_memory_pool =  pool;

            /* Build the necessary pointers.  */
            new_ptr -> dm_previous_memory =  memory_ptr;
            new_ptr -> dm_next_memory =      memory_ptr -> dm_next_memory;
            (new_ptr -> dm_next_memory) -> dm_previous_memory =  new_ptr;
            memory_ptr -> dm_next_memory =   new_ptr;

            /* Decrement the available byte count by one DM_OVERHEAD. */
            pool -> dm_available = pool -> dm_available - DM_OVERHEAD;

            /* Point to new aligned free block. */
            memory_ptr = new_ptr;
        }

        /* Determine if the remaining block needs to be tail split. */
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






