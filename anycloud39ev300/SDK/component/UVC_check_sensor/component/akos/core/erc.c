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
/*      erc.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      ER - Error Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core routines for the Error management    */
/*      component.                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      ERC_System_Error                    System error function        */
/*      ERC_Assert                          System assertion routine     */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      tc_defs.h                           Thread control definitions   */
/*      er_extr.h                           Error handling functions     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified copyright notice,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
/*      11-24-1998      Added ERC_Assert routine.                        */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-07-1999      Release 1.11mA                                   */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE

#ifdef          NU_ERROR_STRING
#include        <stdio.h>                   /* Standard I/O functions    */
#endif
#include        "tc_defs.h"                 /* Thread control constants  */
#include        "er_extr.h"                 /* Error handling functions  */

#ifdef NU_DEBUG_MEMORY

#include        "er_defs.h"               /* Error management structures */
#include        "dm_extr.h"               /* Memory management           */
#include        "ncl\inc\string.h"        /* memcmp & memcpy functions   */
#include        "ncl\inc\nu_ncl.h"        /* memcmp & memcpy functions   */

extern NU_MEMORY_POOL  NU_DEBUG_POOL;
extern const UINT8 ERD_MemoryAllocationHead[];
extern const UINT8 ERD_MemoryAllocationFoot[];
extern ER_DEBUG_ALLOCATION *ERD_RecentAllocation;

extern UINT32 ERD_AllocationCount;
extern UINT32 ERD_AllocationSequenceCounter;
extern UINT32 ERD_TotalMemoryAllocated;
extern UINT32 ERD_TotalMemoryAllocations;
extern UINT32 ERD_MaxTotalMemoryAllocated;
extern UINT32 ERD_MaxTotalMemoryAllocations;

#endif /* NU_DEBUG_MEMORY */


/* Define external inner-component global data references.  */

extern  INT     ERD_Error_Code;

#ifdef          NU_ERROR_STRING
extern  CHAR    ERD_Error_String[];
#endif

#ifdef NU_DEBUG
extern UNSIGNED ERD_Assert_Count;
#endif


/* Define direct access to a thread component variable.  */

extern  VOID   *TCD_Current_Thread;
T_VOID AK_Printf(const char *s);

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ERC_System_Error                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function processes system errors detected by various        */
/*      system components.  Typically an error of this type is           */
/*      considered fatal.                                                */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Various Components                                               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      error_code                          Code of detected system error*/
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
VOID  ERC_System_Error(INT error_code)
{
#ifdef          NU_ERROR_STRING
INT     i;
CHAR   *pointer;
CHAR    name[NU_MAX_NAME+1];
#endif

NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* First place the error code into the global variable.  */
    ERD_Error_Code =  error_code;

#ifdef          NU_ERROR_STRING
    /* Build string that corresponds to the error.  */
    switch(error_code)
    {

    case        NU_ERROR_CREATING_TIMER_HISR:

        /* Build string that indicates an error occurred creating the timer
           HISR.  */
        sprintf(ERD_Error_String,"%s\n", "Error Creating Timer HISR");
        break;

    case        NU_ERROR_CREATING_TIMER_TASK:

        /* Build string that indicates an error occurred creating the timer
           Task.  */
        sprintf(ERD_Error_String,"%s\n", "Error Creating Timer Task");
        break;

    case        NU_STACK_OVERFLOW:

        /* Build string that indicates a stack overflow occurred.  */
        name[NU_MAX_NAME] =  (CHAR) 0;
        pointer =  (((TC_TCB *) TCD_Current_Thread) -> tc_name);
        for (i = 0; i < NU_MAX_NAME; i++)
            name[i] =  *pointer++;
        sprintf(ERD_Error_String,"%s %s\n", "Stack Overflow in task/HISR: ",
                                                                        name);
        break;


    case        NU_UNHANDLED_INTERRUPT:

        /* Build string that indicates an error occurred because of an
           unhandled interrupt.  */
        sprintf(ERD_Error_String,"%s\n", "Unhandled interrupt error");
        break;
    }
#endif


	AK_Printf("akos:");
	AK_Printf(ERD_Error_String);
    /* This function cannot return, since the error is fatal.  */
    while(1)
    {
    }

    /* No need to return to user mode because of the infinite loop. */
    /* Returning to user mode will cause warnings with some compilers. */
}

#ifdef NU_DEBUG_MEMORY

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ERC_Memory_To_Debug                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns a pointer to the ER_DEBUG_ALLOCATION that  */
/*      contains the memory allocation specified by the caller (target). */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ERC_Deallocate_Memory                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      memcmp (CLIB_memcmp)                                             */
/*      ERC_System_Error                                                 */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      target                          memory allocation to find        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      ER_DEBUG_ALLOCATION             ER_DEBUG_ALLOCATION that contains*/
/*                                        target.                        */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*************************************************************************/
static ER_DEBUG_ALLOCATION *ERC_Memory_To_Debug(VOID *target)
{
    INT dataOffset;
    ER_DEBUG_ALLOCATION *walk;
    ER_DEBUG_ALLOCATION *to_find;

    dataOffset = (INT)(((ER_DEBUG_ALLOCATION *)0)->data);
    to_find = (ER_DEBUG_ALLOCATION *)(((UNSIGNED_CHAR *)target) - dataOffset);

    /* Invalid pointer, report no match found */
    if((target == NULL) && (to_find == NULL))
        return(NULL);

    for (walk = ERD_RecentAllocation; ((walk != to_find) && (walk !=  NULL));
        walk = walk->prev);

    /* if no match was found */
    if (walk != NULL)
    {
        /* Has the "HEAD" or "FOOT" been disturbed by a rouge pointer? */
        if (memcmp((void *)ERD_MemoryAllocationHead,(void *)walk->head,4) != 0)
            ERC_System_Error(NU_MEMORY_CORRUPT);
        if (memcmp((void *)ERD_MemoryAllocationFoot,(void *)&(walk->data[walk->size]),4) != 0)
            ERC_System_Error(NU_MEMORY_CORRUPT);
    }

    return(walk);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ERC_Remove_Debug_Allocation                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function removes an ER_DEBUG_ALLOCATION from the linked list*/
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ERC_Deallocate_Memory                                            */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      target                          ER_DEBUG_ALLOCATION to remove    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      STATUS                          status of the operation          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*************************************************************************/
static STATUS ERC_Remove_Debug_Allocation(ER_DEBUG_ALLOCATION *target)
{
    ER_DEBUG_ALLOCATION *walk;

    if (target == NULL)
        return(NU_INVALID_POINTER);

    /* If the list is empty nothing can be removed! */
    if (ERD_RecentAllocation == NULL)
        return(NU_EMPTY_DEBUG_ALLOCATION_LIST);

    /* If there is only one item on the list. */
    if (ERD_RecentAllocation->prev == NULL)
    {
        /* If the only item on the list is the one to be removed...*/
        if (ERD_RecentAllocation == target)
        {
            ERD_RecentAllocation = NULL;
            return(NU_SUCCESS);
        }
        else
            return(NU_INVALID_DEBUG_ALLOCATION);
    }

    if (ERD_RecentAllocation == target)
    {
        ERD_RecentAllocation->prev = target->prev;
        return(NU_SUCCESS);
    }
    
    /* Walk the entire list until walk->prev is the target. */
    walk = ERD_RecentAllocation;
    while (NU_TRUE)
    {
        if (walk->prev == target)
            break;
        if (walk->prev == NULL)
            break;
        walk = walk->prev;
    }

    /* target is last item on the list */
    if (walk->prev == target)
    {
        walk->prev = target->prev;
        return(NU_SUCCESS);
    }
    return(NU_INVALID_DEBUG_ALLOCATION);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ERC_Append_Debug_Allocation                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function appends an ER_DEBUG_ALLOCATION to the linked list. */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      ERC_Allocate_Memory                                              */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      new_guy                         ER_DEBUG_ALLOCATION to append    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      STATUS                          status of the operation          */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*************************************************************************/
static STATUS ERC_Append_Debug_Allocation(ER_DEBUG_ALLOCATION *new_guy)
{

    /* Either this is the first ER_DEBUG_ALLOCATION ever to be appended
       or this is the first on a list that shrank to 0 element. */
    if (ERD_AllocationCount == 0)
    {
        ERD_RecentAllocation = new_guy;
        ERD_RecentAllocation->prev = NULL;
    }
    else
    {
        new_guy->prev = ERD_RecentAllocation;
        ERD_RecentAllocation = new_guy;
    }

    return(NU_SUCCESS);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ERC_Allocate_Memory                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function tracks additional information regarding the memory */
/*      allocation                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ERC_Append_Debug_Allocation                                      */
/*      DMCE_Allocate_Memory                                             */
/*      memcpy                                                           */
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
/*************************************************************************/
STATUS ERC_Allocate_Memory(NU_MEMORY_POOL *pool_ptr, VOID **ptr,
                           UNSIGNED size, UNSIGNED suspend, 
                           unsigned long line, const char* file)
{
    ER_DEBUG_ALLOCATION **debug_ptr;
    STATUS status = NU_SUCCESS;

    /* If the allocation is not from the pool specified in the 
       NU_DEBUG_POOL macro then allocate memory normally (no meta-data) */
    if(&NU_DEBUG_POOL != pool_ptr)
        return(DMCE_Allocate_Memory(pool_ptr, ptr, size, suspend));

    /* This call allocates memory for a structure that will contain the 
       users data and the meta-data used to find memory problems */
    status = DMCE_Allocate_Memory(pool_ptr, ptr,
        (sizeof(ER_DEBUG_ALLOCATION) + size + 4), suspend);
    if (status != NU_SUCCESS)
        return status;

    /* From here out, debug_ptr is used because it is typed.  In the end
       ptr will be set to point to debug_ptr->data, where the user will
       put the data. */
    debug_ptr = (ER_DEBUG_ALLOCATION **)ptr;

    /* Record file and line where the application made the allocation */
    (*debug_ptr)->line = line;
    (*debug_ptr)->file = file;

    /* Set "HEAD" and "FOOT" boundary markers */
    memcpy((*debug_ptr)->head,ERD_MemoryAllocationHead,4);
    memcpy(&((*debug_ptr)->data[size]),ERD_MemoryAllocationFoot,4);

    /* Record the size */
    (*debug_ptr)->size = size;

    /* This links debug_ptr to a linked list that holds all the
       ER_DEBUG_ALLOCATION structures. */
    ERC_Append_Debug_Allocation((*debug_ptr));

    (*debug_ptr)->AllocSequenceCounter = ERD_AllocationSequenceCounter++;

    ERD_TotalMemoryAllocated += size;
    ERD_TotalMemoryAllocations++;
    ERD_AllocationCount++;

    if (ERD_MaxTotalMemoryAllocated < ERD_TotalMemoryAllocated)
        ERD_MaxTotalMemoryAllocated = ERD_TotalMemoryAllocated;
    if (ERD_MaxTotalMemoryAllocations < ERD_TotalMemoryAllocations)
        ERD_MaxTotalMemoryAllocations = ERD_TotalMemoryAllocations;

    /* Return pointer to the data field of debug allocation by reference */
    (*ptr) = (*debug_ptr)->data;
   return(status);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ERC_Deallocate_Memory                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function tracks additional information regarding the memory */
/*      deallocation.                                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ERC_Memory_To_Debug                                              */
/*      ERC_Remove_Debug_Allocation                                      */
/*      DMCE_Deallocate_Memory                                           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ptr                                 Pointer to dynamic memory    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*      NU_INVALID_POINTER                  Returned when ptr is null or */
/*                                            when there is no           */
/*                                            corresponding              */
/*                                            ER_DEBUG_ALLOCATION        */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*************************************************************************/
STATUS ERC_Deallocate_Memory(VOID *ptr)
{
    ER_DEBUG_ALLOCATION *target;
    STATUS status;

    if (ptr == NULL)
        return(NU_INVALID_POINTER);

    /* Find the NU_DEBUG_ALLOCATION ptr refers to.  After this call, 
       (&(target->data) == ptr) or (target == NULL). */
    target = ERC_Memory_To_Debug(ptr);

    /* Remove target from the linked list of ER_DEBUG_ALLOCATIONs */
    status = ERC_Remove_Debug_Allocation(target);

    if ((status != 0) || (target == NULL))
        return(NU_INVALID_POINTER);
        
    /* Maintain status variables */
    ERD_TotalMemoryAllocated -= target->size;
    ERD_TotalMemoryAllocations--;
    ERD_AllocationCount--;

    return(DMCE_Deallocate_Memory(target));
}

#endif /* NU_DEBUG_MEMORY */

/**************************************************************************
  This routine should appear last in this file and must *NOT* use the
  NU_ASSERT macro.
**************************************************************************/

#ifdef NU_ASSERT                  /* Don't use NU_ASSERT past this point */
#undef NU_ASSERT
#define NU_ASSERT(ignore) ((void) 0)
#endif

#ifdef NU_ASSERT2
#undef NU_ASSERT2
#define NU_ASSERT2(ignore) ((void) 0)
#endif

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ERC_Assert                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This public routine is called when an assertion made by the      */
/*      NU_ASSERT (or NU_ASSERT2) macro fails.  By default, this routine */
/*      simply counts the number of failed assertions.  A breakpoint can */
/*      be set in the routine to observe failed assertions, or the       */
/*      routine can be customized to perform some action, such as        */
/*      printing the failed assertion as a message, etc.                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      NU_ASSERT macro                                                  */
/*      NU_ASSERT2 macro                                                 */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      test               Pointer to string of failed assertion test    */
/*      name               File name of file containing failed assertion */
/*      line               Location of failed assertion in above file    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*          NAME            DATE                    REMARKS              */
/*                                                                       */
/*      Todd C. Larsen    09-01-1998        Created initial revision     */
/*                                                                       */
/*************************************************************************/
#ifdef NU_DEBUG

void ERC_Assert(CHAR *test, CHAR *name, UNSIGNED line)
{
NU_SUPERV_USER_VARIABLES

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_ASSERT_ID, (UNSIGNED) test,
                        (UNSIGNED) name, line);

#endif

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Set breakpoint here to catch failed assertions. */
    ERD_Assert_Count += 1;

    /* Return to user mode */
    NU_USER_MODE();
}

#endif /* NU_DEBUG */





