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
/*      pmf.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PM - Partition Memory Management                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains routines to obtain facts about the Partition  */
/*      Memory Management component.                                     */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      PMF_Established_Partition_Pools     Number of partition pools    */
/*      PMF_Partition_Pool_Pointers         Build partition pool pointer */
/*                                            list                       */
/*      PMF_Partition_Pool_Information      Retrieve partition pool info */
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
/*        DATE                    REMARKS                                */
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
#include        "pm_extr.h"                 /* Partition functions       */
#include        "hi_extr.h"                 /* History functions         */


/* Define external inner-component global data references.  */

extern CS_NODE         *PMD_Created_Pools_List;
extern UNSIGNED         PMD_Total_Pools;
extern TC_PROTECT       PMD_List_Protect;



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PMF_Established_Partition_Pools                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the current number of established          */
/*      partition pools.  Pools previously deleted are no longer         */
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
/*      PMD_Total_Pools                     Number of established        */
/*                                            partition pools            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  PMF_Established_Partition_Pools(VOID)
{


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Return the number of established partition pools.  */
    return(PMD_Total_Pools);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PMF_Partition_Pool_Pointers                                      */
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
/*      pointers                            Number of partition pools    */
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
UNSIGNED  PMF_Partition_Pool_Pointers(NU_PARTITION_POOL **pointer_list,
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

    /* Protect against access to the list of created partition pools.  */
    TCT_Protect(&PMD_List_Protect);

    /* Loop until all pool pointers are in the list or until the maximum
       list size is reached.  */
    node_ptr =  PMD_Created_Pools_List;
    while ((node_ptr) && (pointers < maximum_pointers))
    {

        /* Place the node into the destination list.  */
        *pointer_list++ =  (NU_PARTITION_POOL *) node_ptr;

        /* Increment the pointers variable.  */
        pointers++;

        /* Position the node pointer to the next node.  */
        node_ptr =  node_ptr -> cs_next;

        /* Determine if the pointer is at the head of the list.  */
        if (node_ptr == PMD_Created_Pools_List)

            /* The list search is complete.  */
            node_ptr =  NU_NULL;
    }

    /* Release protection against access to the list of created pools. */
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
/*      PMF_Partition_Pool_Information                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns information about the specified partition  */
/*      pool.  However, if the supplied partition pool pointer is        */
/*      invalid, the function simply returns an error status.            */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect partition pool       */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pool_ptr                            Pointer to the partition pool*/
/*      name                                Destination for the name     */
/*      start_address                       Destination for the starting */
/*                                            memory address of the pool */
/*      pool_size                           Destination for the pool's   */
/*                                            total size                 */
/*      partition_size                      Destination for the size of  */
/*                                            each partition             */
/*      available                           Destination for the available*/
/*                                            number of partitions       */
/*      allocated                           Destination for the number   */
/*                                            of allocated partitions    */
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
/*        DATE                    REMARKS                                */
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
STATUS  PMF_Partition_Pool_Information(NU_PARTITION_POOL *pool_ptr, CHAR *name,
                  VOID **start_address, UNSIGNED *pool_size,
                  UNSIGNED *partition_size, UNSIGNED *available,
                  UNSIGNED *allocated, OPTION *suspend_type,
                  UNSIGNED *tasks_waiting, NU_TASK **first_task)
{

PM_PCB         *pool;                       /* Pool control block ptr    */
INT             i;                          /* Working integer variable  */
STATUS          completion;                 /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to superisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pool pointer into internal pointer.  */
    pool =  (PM_PCB *) pool_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Determine if this partition pool id is valid.  */
    if ((pool != NU_NULL) && (pool -> pm_id == PM_PARTITION_ID))
    {

        /* Setup protection of the partition pool.  */
        TCT_System_Protect();

        /* The partition pool pointer is valid.  Reflect this in the completion
           status and fill in the actual information.  */
        completion =  NU_SUCCESS;

        /* Copy the partition pool's name.  */
        for (i = 0; i < NU_MAX_NAME; i++)
            *name++ =  pool -> pm_name[i];

        /* Determine the suspension type.  */
        if (pool -> pm_fifo_suspend)
            *suspend_type =          NU_FIFO;
        else
            *suspend_type =          NU_PRIORITY;

        /* Retrieve information directly out of the control structure.  */
        *start_address =        pool -> pm_start_address;
        *pool_size =            pool -> pm_pool_size;
        *partition_size =       pool -> pm_partition_size;
        *available =            pool -> pm_available;
        *allocated =            pool -> pm_allocated;

        /* Retrieve the number of tasks waiting and the pointer to the
           first task waiting.  */
        *tasks_waiting =  pool -> pm_tasks_waiting;
        if (pool -> pm_suspension_list)

            /* There is a task waiting.  */
            *first_task =  (NU_TASK *)
                (pool -> pm_suspension_list) -> pm_suspended_task;
        else

            /* There are no tasks waiting.  */
            *first_task =  NU_NULL;

        /* Release protection of the partition pool.  */
        TCT_Unprotect();
    }
    else

        /* Indicate that the partition pool pointer is invalid.   */
        completion =  NU_INVALID_POOL;

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the appropriate completion status.  */
    return(completion);
}




