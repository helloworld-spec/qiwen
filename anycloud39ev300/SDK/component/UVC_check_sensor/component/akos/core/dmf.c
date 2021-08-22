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
/*      dmf.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DM - Dynamic Memory Management                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains routines to obtain facts about the Dynamic    */
/*      Memory Management component.                                     */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      DMF_Established_Memory_Pools        Number of dynamic pools      */
/*      DMF_Memory_Pool_Pointers            Build memory pool pointer    */
/*                                            list                       */
/*      DMF_Memory_Pool_Information         Retrieve memory pool info    */
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
/*          DATE                    REMARKS                              */
/*                                                                       */
/*      03-01-1994      Initial version of partition fact                */
/*                      service file, version 1.1                        */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      11-18-1996      Protected Informational service                  */
/*                      from NULL Control Block pointers                 */
/*                      creating 1.2a. (SPR220)                          */
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


/* Define external inner-component global data references.  */

extern CS_NODE         *DMD_Created_Pools_List;
extern UNSIGNED         DMD_Total_Pools;
extern TC_PROTECT       DMD_List_Protect;



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      DMF_Established_Memory_Pools                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the current number of established          */
/*      memory pools.  Pools previously deleted are no longer            */
/*      considered established.                                          */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      DMD_Total_Pools                     Number of established        */
/*                                            dynamic memory pools       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  DMF_Established_Memory_Pools(VOID)
{


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Return the number of established dynamic memory pools.  */
    return(DMD_Total_Pools);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      DMF_Memory_Pool_Pointers                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function builds a list of pool pointers, starting at        */
/*      the specified location.  The number of pool pointers             */
/*      placed in the list is equivalent to the total number of          */
/*      pools or the maximum number of pointers specified in the         */
/*      call.                                                            */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created list         */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pointer_list                        Pointer to the list area     */
/*      maximum_pointers                    Maximum number of pointers   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      pointers                            Number of memory pools       */
/*                                            placed in the list         */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Corrected pointer retrieval                      */
/*                      loop, resulting in version 1.0a                  */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  DMF_Memory_Pool_Pointers(NU_MEMORY_POOL **pointer_list,
                                                UNSIGNED maximum_pointers)
{

CS_NODE         *node_ptr;                  /* Pointer to each PCB       */
UNSIGNED         pointers;                  /* Number of pointers in list*/
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Initialize the number of pointers returned.  */
    pointers =  0;

    /* Protect against access to the list of created memory pools.  */
    TCT_Protect(&DMD_List_Protect);

    /* Loop until all pool pointers are in the list or until the maximum
       list size is reached.  */
    node_ptr =  DMD_Created_Pools_List;
    while ((node_ptr) && (pointers < maximum_pointers))
    {

        /* Place the node into the destination list.  */
        *pointer_list++ =  (NU_MEMORY_POOL *) node_ptr;

        /* Increment the pointers variable.  */
        pointers++;

        /* Position the node pointer to the next node.  */
        node_ptr =  node_ptr -> cs_next;

        /* Determine if the pointer is at the head of the list.  */
        if (node_ptr == DMD_Created_Pools_List)

            /* The list search is complete.  */
            node_ptr =  NU_NULL;
    }

    /* Release protection of the list of created pools.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the number of pointers in the list.  */
    return(pointers);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      DMF_Memory_Pool_Information                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns information about the specified memory     */
/*      pool.  However, if the supplied memory pool pointer is           */
/*      invalid, the function simply returns an error status.            */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect memory pool          */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pool_ptr                            Pointer to the memory pool   */
/*      name                                Destination for the name     */
/*      start_address                       Destination for the starting */
/*                                            memory address of the pool */
/*      pool_size                           Destination for the pool's   */
/*                                            total size                 */
/*      min_allocation                      Destination for the minimum  */
/*                                            block allocation size      */
/*      available                           Destination for the available*/
/*                                            number of bytes in pool    */
/*      suspend_type                        Destination for the type of  */
/*                                            suspension                 */
/*      tasks_waiting                       Destination for the tasks    */
/*                                            waiting count              */
/*      first_task                          Destination for the pointer  */
/*                                            to the first task waiting  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If a valid pool pointer      */
/*                                            is supplied                */
/*      NU_INVALID_POOL                     If pool pointer invalid      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      11-18-1996      Corrected SPR220.                                */
/*                                                                       */
/*************************************************************************/
STATUS  DMF_Memory_Pool_Information(NU_MEMORY_POOL *pool_ptr, CHAR *name,
                  VOID **start_address, UNSIGNED *pool_size,
                  UNSIGNED *min_allocation, UNSIGNED *available,
                  OPTION *suspend_type, UNSIGNED *tasks_waiting,
                  NU_TASK **first_task)
{

DM_PCB         *pool;                       /* Pool control block ptr    */
INT             i;                          /* Working integer variable  */
STATUS          completion;                 /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pool pointer into internal pointer.  */
    pool =  (DM_PCB *) pool_ptr;

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Determine if this memory pool id is valid.  */
    if ((pool != NU_NULL) && (pool -> dm_id == DM_DYNAMIC_ID))
    {

        /* Setup protection of the memory pool.  */
        TCT_Protect(&(pool -> dm_protect));

        /* The memory pool pointer is valid.  Reflect this in the completion
           status and fill in the actual information.  */
        completion =  NU_SUCCESS;

        /* Copy the memory pool's name.  */
        for (i = 0; i < NU_MAX_NAME; i++)
            *name++ =  pool -> dm_name[i];

        /* Determine the suspension type.  */
        if (pool -> dm_fifo_suspend)
            *suspend_type =          NU_FIFO;
        else
            *suspend_type =          NU_PRIORITY;

        /* Retrieve information directly out of the control structure.  */
        *start_address =        pool -> dm_start_address;
        *pool_size =            pool -> dm_pool_size;
        *min_allocation =       pool -> dm_min_allocation;
        *available =            pool -> dm_available;

        /* Retrieve the number of tasks waiting and the pointer to the
           first task waiting.  */
        *tasks_waiting =  pool -> dm_tasks_waiting;
        if (pool -> dm_suspension_list)

            /* There is a task waiting.  */
            *first_task =  (NU_TASK *)
                (pool -> dm_suspension_list) -> dm_suspended_task;
        else

            /* There are no tasks waiting.  */
            *first_task =  NU_NULL;

        /* Release protection of the memory pool.  */
        TCT_Unprotect();
    }
    else

        /* Indicate that the memory pool pointer is invalid.   */
        completion =  NU_INVALID_POOL;

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the appropriate completion status.  */
    return(completion);
}




