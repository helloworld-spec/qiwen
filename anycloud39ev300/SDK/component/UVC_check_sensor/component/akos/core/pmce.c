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
/*      pmce.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PM - Partition Memory Management                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the error checking routines for the functions */
/*      in the Partition component.  This permits easy removal of error  */
/*      checking logic when it is not needed.                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      PMCE_Create_Partition_Pool          Create a Partition Pool      */
/*      PMCE_Delete_Partition_Pool          Delete a Partition Pool      */
/*      PMCE_Allocate_Partition             Allocate a partition from a  */
/*                                            pool                       */
/*      PMCE_Deallocate_Partition           Deallocate a partition from  */
/*                                            a pool                     */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      pm_extr.h                           Partition functions          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed name original error                      */
/*                      checking file and changed                        */
/*                      function interfaces, resulting                   */
/*                      in version 1.1                                   */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "pm_extr.h"                 /* Partition functions       */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PMCE_Create_Partition_Pool                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the create partition pool function.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      PMC_Create_Partition_Pool           Actual create partition pool */
/*                                            function                   */
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
/*      NU_INVALID_POOL                     Pool control block pointer   */
/*                                            is NULL                    */
/*      NU_INVALID_MEMORY                   Pool starting address is NULL*/
/*      NU_INVALID_SIZE                     Partition size is 0 or it is */
/*                                            larger than the pool area  */
/*      NU_INVALID_SUSPEND                  Suspension selection is not  */
/*                                            valid                      */
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
STATUS  PMCE_Create_Partition_Pool(NU_PARTITION_POOL *pool_ptr, CHAR *name,
                        VOID *start_address, UNSIGNED pool_size, 
                        UNSIGNED partition_size, OPTION suspend_type)
{

PM_PCB         *pool;                       /* Pool control block ptr    */
STATUS          status;                     /* Completion status         */
UNSIGNED        size;                       /* Adjusted size of partition*/


    /* Move input pool pointer into internal pointer.  */
    pool =  (PM_PCB *) pool_ptr;

    /* Adjust the partition size to something that is evenly divisible by
       the number of bytes in an UNSIGNED data type.  */
    size =  ((partition_size + sizeof(UNSIGNED) - 1)/sizeof(UNSIGNED)) *
                sizeof(UNSIGNED);

    /* Check for a NULL partition pool control block pointer or a control
       block that is already created.  */
    if ((pool == NU_NULL) || (pool -> pm_id == PM_PARTITION_ID))
    
        /* Invalid partition pool control block pointer.  */
        status =  NU_INVALID_POOL;
        
    else if (start_address == NU_NULL)
    
        /* Invalid memory pointer.  */
        status =  NU_INVALID_MEMORY;
        
    else if ((size == 0) || ((size + PM_OVERHEAD) > pool_size))
    
        /* Pool could not even accommodate one partition.  */
        status =  NU_INVALID_SIZE;
        
    else if ((suspend_type != NU_FIFO) && (suspend_type != NU_PRIORITY))
    
        /* Invalid suspension type.  */
        status =  NU_INVALID_SUSPEND;
        
    else
    
        /* Call the actual service to create the partition pool.  */
        status =  PMC_Create_Partition_Pool(pool_ptr, name, start_address, 
                                pool_size, partition_size, suspend_type);
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PMCE_Delete_Partition_Pool                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the delete partition pool function.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      PMC_Delete_Partition_Pool           Actual function to delete a  */
/*                                            partition pool             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pool_ptr                            Partition pool control block */
/*                                            pointer                    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_POOL                     Indicates the supplied pool  */
/*                                            pointer is invalid         */
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
STATUS  PMCE_Delete_Partition_Pool(NU_PARTITION_POOL *pool_ptr)
{

PM_PCB         *pool;                       /* Pool control block ptr    */
STATUS          status;                     /* Completion status         */


    /* Move input pool pointer into internal pointer.  */
    pool =  (PM_PCB *) pool_ptr;

    /* Determine if the partition pool pointer is valid.  */
    if ((pool) && (pool -> pm_id == PM_PARTITION_ID))
    
        /* Partition pool pointer is valid, call function to delete it.  */
        status =  PMC_Delete_Partition_Pool(pool_ptr);
        
    else
    
        /* Partition pool pointer is invalid, indicate in completion status. */
        status =  NU_INVALID_POOL;
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PMCE_Allocate_Partition                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the allocate partition function.                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      PMC_Allocate_Partition              Actual partition allocate    */
/*                                            function                   */
/*      TCCE_Suspend_Error                  Check for a task suspension  */
/*                                            error                      */
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
/*      NU_INVALID_POOL                     Indicates the pool pointer   */
/*                                            is invalid                 */
/*      NU_INVALID_POINTER                  Indicates the return pointer */
/*                                            is NULL                    */
/*      NU_INVALID_SUSPEND                  Indicates the suspension is  */
/*                                            invalid                    */
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
STATUS  PMCE_Allocate_Partition(NU_PARTITION_POOL *pool_ptr, 
                             VOID **return_pointer, UNSIGNED suspend)
{

PM_PCB         *pool;                       /* Pool control block ptr    */
STATUS          status;                     /* Completion status         */


    /* Move input pool pointer into internal pointer.  */
    pool =  (PM_PCB *) pool_ptr;

    /* Determine if partition pool pointer is invalid.  */
    if (pool == NU_NULL)
    
        /* Partition pool pointer is invalid, indicate in completion status. */
        status =  NU_INVALID_POOL;

    else if (pool -> pm_id != PM_PARTITION_ID)
    
        /* Partition pool pointer is invalid, indicate in completion status. */
        status =  NU_INVALID_POOL;

    else if (return_pointer == NU_NULL)

        /* Return pointer is invalid.  */
        status =  NU_INVALID_POINTER;
        
    else if ((suspend) && (TCCE_Suspend_Error()))
    
        /* Suspension from a non-task thread.  */
        status =  NU_INVALID_SUSPEND;

    else 
    
        /* Parameters are valid, call actual function.  */
        status =  PMC_Allocate_Partition(pool_ptr, return_pointer, suspend);

    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PMCE_Deallocate_Partition                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the deallocate partition function.                            */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      PMC_Deallocate_Partition            Deallocate a partition       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      partition                           Pointer to partition memory  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_POINTER                  Indicates the supplied       */
/*                                            partition pointer is NULL, */
/*                                            or otherwise invalid.      */
/*      NU_SUCCESS                                                       */
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
STATUS  PMCE_Deallocate_Partition(VOID *partition)
{

PM_PCB         *pool;                       /* Pool pointer              */
PM_HEADER      *header_ptr;                 /* Pointer to partition hdr  */
STATUS          status;                     /* Completion status         */


    /* Pickup the associated pool's pointer.  It is inside the header of
       each partition.  */
    header_ptr =  (PM_HEADER *) (((BYTE_PTR) partition) - PM_OVERHEAD);
        
    /* Determine if the pointer(s) are NULL.  */
    if ((header_ptr == NU_NULL) || (partition == NU_NULL))
    
        /* Partition pointer is invalid.  */
        status =  NU_INVALID_POINTER;

    /* Determine if partition pool pointer is invalid.  */
    else if ((pool =  header_ptr -> pm_partition_pool) == NU_NULL)
    
        /* Partition pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_POINTER;

    else if (pool -> pm_id != PM_PARTITION_ID)
    
        /* Partition pool pointer is invalid, indicate in completion status. */
        status =  NU_INVALID_POINTER;
        
    else if (header_ptr -> pm_next_available)
    
        /* Partition is still linked on the available list- must not be
           allocated.  */
        status =  NU_INVALID_POINTER;

    else 
    
        /* Parameters are valid, call actual function.  */
        status =  PMC_Deallocate_Partition(partition);

    /* Return the completion status.  */
    return(status);
}






