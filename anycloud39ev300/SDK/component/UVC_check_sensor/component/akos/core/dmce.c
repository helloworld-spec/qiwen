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
/*      dmce.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DM - Dynamic Memory Management                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the error checking routines for the functions */
/*      in the dynamic memory management component.  This permits easy   */
/*      removal of the error checking logic when it is not needed.       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      DMCE_Create_Memory_Pool             Create a dynamic memory pool */
/*      DMCE_Delete_Memory_Pool             Delete a dynamic memory pool */
/*      DMCE_Allocate_Memory                Allocate a memory block from */
/*                                            a dynamic memory pool      */
/*      DMCE_Deallocate_Memory              Deallocate a memory block    */
/*                                            from a dynamic memory pool */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      dm_extr.h                           Partition functions          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      11-01-1993      Corrected call of actual                         */
/*                        function to delete a memory                    */
/*                        pool, resulting in version 1.0a                */
/*      11-01-1993      Verfied version 1.0a                             */
/*      03-01-1994      Changed name original error                      */
/*                        checking file and changed                      */
/*                        function interfaces, resulting                 */
/*                        in version 1.1                                 */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "dm_extr.h"                 /* Dynamic memory functions  */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      DMCE_Create_Memory_Pool                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the create dynamic memory pool function.                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DMC_Create_Memory_Pool              Actual create dynamic memory */
/*                                            pool function              */
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
/*      NU_INVALID_POOL                     Indicates the pool control   */
/*                                            block pointer is invalid   */
/*      NU_INVALID_MEMORY                   Indicates the starting       */
/*                                            memory address is NULL     */
/*      NU_INVALID_SIZE                     Indicates that either the    */
/*                                            pool size and/or the       */
/*                                            minimum allocation size is */
/*                                            invalid                    */
/*      NU_INVALID_SUSPEND                  Indicate the suspension type */
/*                                            is invalid                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  DMCE_Create_Memory_Pool(NU_MEMORY_POOL *pool_ptr, CHAR *name, 
                        VOID *start_address, UNSIGNED pool_size, 
                        UNSIGNED min_allocation, OPTION suspend_type)
{

DM_PCB         *pool;                       /* Pool control block ptr    */
STATUS          status;                     /* Completion status         */
UNSIGNED        adjusted_min;               /* Adjusted size of minimum  */
UNSIGNED        adjusted_pool;              /* Adjusted size of pool     */


    /* Move input pool pointer into internal pointer.  */
    pool =  (DM_PCB *) pool_ptr;

    /* Adjust the minimum allocation size to something that is evenly 
       divisible by the number of bytes in an UNSIGNED data type.  */
    adjusted_min = ((min_allocation + sizeof(UNSIGNED) - 1)/sizeof(UNSIGNED)) *
                                                        sizeof(UNSIGNED);

    /* Adjust the pool size to something that is evenly divisible by the 
       number of bytes in an UNSIGNED data type.  */
    adjusted_pool = ((pool_size + sizeof(UNSIGNED) - 1)/sizeof(UNSIGNED)) *
                                                        sizeof(UNSIGNED);

    /* Check for a NULL dynamic memory pool control block pointer or a control
       block that is already created.  */
    if ((pool == NU_NULL) || (pool -> dm_id == DM_DYNAMIC_ID))
    
        /* Invalid dynamic memory pool control block pointer.  */
        status =  NU_INVALID_POOL;
        
    else if (start_address == NU_NULL)
    
        /* Invalid memory pointer.  */
        status =  NU_INVALID_MEMORY;
        
    else if ((adjusted_min == 0) || 
                ((adjusted_min + (2 * DM_OVERHEAD)) > adjusted_pool))
    
        /* Pool could not even accommodate one allocation.  */
        status =  NU_INVALID_SIZE;
        
    else if ((suspend_type != NU_FIFO) && (suspend_type != NU_PRIORITY))
    
        /* Invalid suspension type.  */
        status =  NU_INVALID_SUSPEND;
        
    else
    
        /* Call the actual service to create the dynamic memory pool.  */
        status =  DMC_Create_Memory_Pool(pool_ptr, name, start_address, 
                                    pool_size, min_allocation, suspend_type);
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      DMCE_Delete_Memory_Pool                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the delete dynamic memory pool function.                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DMC_Delete_Memory_Pool              Actual function to delete a  */
/*                                            dynamic memory pool        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pool_ptr                            Memory pool control block    */
/*                                            pointer                    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_POOL                     Indicates the pool pointer   */
/*                                            is invalid                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      11-01-1993      Corrected call of actual                         */
/*                        function to delete a memory                    */
/*                        pool, resulting in version 1.0a                */
/*      11-01-1993      Verfied version 1.0a                             */
/*      03-01-1994      Modified function interface,                     */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  DMCE_Delete_Memory_Pool(NU_MEMORY_POOL *pool_ptr)
{

DM_PCB         *pool;                       /* Pool control block ptr    */
STATUS          status;                     /* Completion status         */

    
    /* Move input pool pointer into internal pointer.  */
    pool =  (DM_PCB *) pool_ptr;

    /* Determine if the dynamic memory pool pointer is valid.  */
    if ((pool) && (pool -> dm_id == DM_DYNAMIC_ID))
    
        /* Dynamic memory pool pointer is valid, call the function to 
           delete it.  */
        status =  DMC_Delete_Memory_Pool(pool_ptr);
        
    else
    
        /* Dynamic memory pool pointer is invalid, indicate in 
           completion status. */
        status =  NU_INVALID_POOL;
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      DMCE_Allocate_Memory                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the allocate memory function.                                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DMC_Allocate_Memory                 Actual memory allocation     */
/*                                            function                   */
/*      TCCE_Suspend_Error                  Check for suspension error   */
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
/*      NU_INVALID_POOL                     Indicates the supplied pool  */
/*                                            pointer is invalid         */
/*      NU_INVALID_POINTER                  Indicates the return pointer */
/*                                            is NULL                    */
/*      NU_INVALID_SIZE                     Indicates the size is 0 or   */
/*                                            larger than the pool       */
/*      NU_INVALID_SUSPEND                  Invalid suspension requested */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface                ,     */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  DMCE_Allocate_Memory(NU_MEMORY_POOL *pool_ptr, VOID **return_pointer,
                                        UNSIGNED size, UNSIGNED suspend)
{

DM_PCB         *pool;                       /* Pool control block ptr    */
STATUS          status;                     /* Completion status         */


    /* Move input pool pointer into internal pointer.  */
    pool =  (DM_PCB *) pool_ptr;

    /* Determine if dynamic memory pool pointer is invalid.  */
    if (pool == NU_NULL)
    
        /* Dynamic memory pool pointer is invalid, indicate in 
           completion status. */
        status =  NU_INVALID_POOL;

    else if (pool -> dm_id != DM_DYNAMIC_ID)
    
        /* Dynamic memory pool pointer is invalid, indicate in 
           completion status. */
        status =  NU_INVALID_POOL;

    else if (return_pointer == NU_NULL)

        /* Return pointer is invalid.  */
        status =  NU_INVALID_POINTER;
        
    else if ((size == 0) || 
        (size > (pool -> dm_pool_size - (2 * DM_OVERHEAD))))
        
        /* Return the invalid size error.  */
        status =  NU_INVALID_SIZE;

    else if ((suspend) && (TCCE_Suspend_Error()))
    
        /* Suspension from an non-task thread.  */
        status =  NU_INVALID_SUSPEND;

    else 
    
        /* Parameters are valid, call actual function.  */
        status = DMC_Allocate_Memory(pool_ptr, return_pointer, size, suspend);

    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      DMCE_Deallocate_Memory                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the deallocate memory function.                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      DMC_Deallocate_Memory               Actual deallocate memory     */
/*                                            function                   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      memory                              Pointer to dynamic memory    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_POINTER                  Indicates the supplied       */
/*                                            pointer is invalid         */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  DMCE_Deallocate_Memory(VOID *memory)
{

DM_PCB         *pool;                       /* Pool pointer              */
DM_HEADER      *header_ptr;                 /* Pointer to memory block   */
STATUS          status;                     /* Completion status         */


    /* Pickup the associated pool's pointer.  It is inside the header of
       each memory block.  */
    header_ptr =  (DM_HEADER *) (((BYTE_PTR) memory) - DM_OVERHEAD);
        
    /* Determine if the pointer(s) are NULL.  */
    if ((header_ptr == NU_NULL) || (memory == NU_NULL))
    
        /* Dynamic memory pointer is invalid.  */
        status =  NU_INVALID_POINTER;

    /* Determine if dynamic memory pool pointer is invalid.  */
    else if ((pool =  header_ptr -> dm_memory_pool) == NU_NULL)
    
        /* Dynamic memory pointer is invalid, indicate in completion 
           status.  */
        status =  NU_INVALID_POINTER;

    else if (pool -> dm_id != DM_DYNAMIC_ID)
    
        /* Dynamic memory pool pointer is invalid, indicate in completion 
           status. */
        status =  NU_INVALID_POINTER;
        
    else if (header_ptr -> dm_memory_free)
    
        /* Dynamic memory is free - must not be allocated.  */
        status =  NU_INVALID_POINTER;

    else 
    
        /* Parameters are valid, call actual function.  */
        status =  DMC_Deallocate_Memory(memory);

    /* Return the completion status.  */
    return(status);
}








