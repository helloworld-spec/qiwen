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
/*      eri.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      ER - Error Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the initialization routine for the Error      */
/*      Management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      ERI_Initialize                      Error Management Initialize  */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      er_extr.h                           Error management interfaces  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*          DATE                    REMARKS                              */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Replaced void with VOID,                         */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-07-1999      Release 1.11mA                                   */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "er_extr.h"               /* Error management interfaces */
#include        "er_defs.h"

/* Define inter-component global data references.  */

extern  INT     ERD_Error_Code;

#ifdef NU_DEBUG_MEMORY
    extern UINT32 ERD_AllocationCount;
    extern UINT32 ERD_AllocationSequenceCounter;
    extern UINT32 ERD_TotalMemoryAllocated;
    extern UINT32 ERD_TotalMemoryAllocations;
    extern UINT32 ERD_MaxTotalMemoryAllocated;
    extern UINT32 ERD_MaxTotalMemoryAllocations;

    extern ER_DEBUG_ALLOCATION *ERD_RecentAllocation;
#endif /* NU_DEBUG_MEMORY */

#ifdef NU_DEBUG
extern UNSIGNED ERD_Assert_Count;
#endif

#ifdef          NU_ERROR_STRING
extern  CHAR    ERD_Error_String[];
#endif

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      ERI_Initialize                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the data structures of the Error       */
/*      management component (ER).                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      INC_Initialize                      System initialization        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      ERD_Error_Code                      Contains system error code   */
/*      ERD_Error_String                    ASCII error code string      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Replaced void with VOID,                         */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  ERI_Initialize(VOID)
{

    /* Clear the system error code flag.  */
    ERD_Error_Code =  0;

#ifdef NU_DEBUG_MEMORY
    /* Clear variables that help find memory problems */
    ERD_AllocationCount              = 0;
    ERD_AllocationSequenceCounter    = 0;
    ERD_TotalMemoryAllocated         = 0;
    ERD_TotalMemoryAllocations       = 0;
    ERD_MaxTotalMemoryAllocated      = 0;
    ERD_MaxTotalMemoryAllocations    = 0;
    ERD_RecentAllocation = 0;
#endif /* NU_DEBUG_MEMORY */

#ifdef NU_DEBUG
    /* Clear count of failed assertions. */
    ERD_Assert_Count = 0;
#endif

#ifdef          NU_ERROR_STRING
    /* Make the error string null.  */
    ERD_Error_String[0] =  0;
#endif
}




