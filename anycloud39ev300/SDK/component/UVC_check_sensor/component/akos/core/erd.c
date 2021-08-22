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
/*      erd.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      ER - Error Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      Error Management component.                                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      ERD_Error_Code                      Contains the system error    */
/*                                            code                       */
/*      ERD_Error_String                    Contains the ASCII system    */
/*                                            error string               */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      nucleus.h                           System definitions           */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified copyright notice,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
/*      11-24-1998      Added ERD_Assert_Count.                          */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-07-1999      Release 1.11mA                                   */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE

#include        "nucleus.h"                 /* System definitions        */


/* ERD_Error_Code contains the system error code detected by the system.  */

INT     ERD_Error_Code;

#ifdef NU_DEBUG

/* ERD_Assert_Count contains the number of detected failed assertions. */
UNSIGNED ERD_Assert_Count;

#endif


#ifdef          NU_ERROR_STRING

/* ERD_Error_String is an area for building an ASCII string representation of
   the system error.  */

CHAR    ERD_Error_String[80];

#endif

#ifdef  NU_DEBUG_MEMORY

#include "er_defs.h"

/* NU_DEBUG_MEMORY wraps the calles to DMCE_Allocate_Memory and 
   DMCE_Deallocate_Memory with a set of calls that help track memory 
   problems in a memory pool.  The memory pool that will be examined
   is determined by the NU_MEMORY_POOL macro. */

/* The functions in NU_DEBUG_MEMORY require error checking.  
   NU_DEBUG_MEMORY can not be defined with NU_NO_ERROR_CHECKING */

#ifdef NU_NO_ERROR_CHECKING
#error Can not define NU_DEBUG_MEMORY and NU_NO_ERROR_CHECKING at the same time!
#endif /* NU_NO_ERROR_CHECKING */

#ifndef NU_DEBUG
#error NU_DEBUG must be defined when NU_DEBUG_MEMORY is enabled!
#endif /* NU_NO_ERROR_CHECKING */

/* ERD_AllocationCount is the current number of sucessful allocations that
        have not been deallocated.
   ERD_AllocationSequenceCounter identifies each successful allocation by 
        numbering them in the order they are created.
   ERD_TotalMemoryAllocated is the number of sucessful calls to 
        NU_Allocate_Memory.
   ERD_TotalMemoryAllocations is the sum of the sizes of each sucessful 
        allocation.
   ERD_MaxTotalMemoryAllocated is the most memory ever allocated at any
        point in time.
   ERD_MaxTotalMemoryAllocations is the most outstanding memory allocations
        (those that are not deallocated) at any point in time. */

UINT32 ERD_AllocationCount;
UINT32 ERD_AllocationSequenceCounter;
UINT32 ERD_TotalMemoryAllocated;
UINT32 ERD_TotalMemoryAllocations;
UINT32 ERD_MaxTotalMemoryAllocated;
UINT32 ERD_MaxTotalMemoryAllocations;

/* Constants to mark the header and footer */
const UINT8 ERD_MemoryAllocationHead[] = {'H','E','A','D'};
const UINT8 ERD_MemoryAllocationFoot[] = {'F','O','O','T'};

/* This is the head of a linked list that holds all the currently
   outstanding allocations in reverse chronological order.  
   RED_RecentAllocation is the most recent.  The 'prev' field always
   points to the allocation made beforehand. */
ER_DEBUG_ALLOCATION *ERD_RecentAllocation;

#endif /* NU_DEBUG_MEMORY */





