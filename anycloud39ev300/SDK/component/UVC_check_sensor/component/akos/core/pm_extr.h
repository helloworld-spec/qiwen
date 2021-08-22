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
/*      pm_extr.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PM - Partition Memory Management                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes of all functions          */
/*      accessible to other components.                                  */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      pm_defs.h                           Partition Management const.  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Moved include files outside of                   */
/*                      the file #ifndef to allow the                    */
/*                      use of actual data structures,                   */
/*                      modified function prototypes,                    */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/

#include        "pm_defs.h"                 /* Include PM constants      */


/* Check to see if the file has been included already.  */

#ifndef PM_EXTR
#define PM_EXTR


/*  Initialization functions.  */

VOID            PMI_Initialize(VOID);


/* Core error checking functions.  */

STATUS          PMCE_Create_Partition_Pool(NU_PARTITION_POOL *pool_ptr, 
                        CHAR *name, VOID *start_address, UNSIGNED pool_size,
                        UNSIGNED partition_size, OPTION suspend_type);
STATUS          PMCE_Delete_Partition_Pool(NU_PARTITION_POOL *pool_ptr);
STATUS          PMCE_Allocate_Partition(NU_PARTITION_POOL *pool_ptr, 
                        VOID **return_pointer, UNSIGNED suspend);
STATUS          PMCE_Deallocate_Partition(VOID *partition);


/* Core processing functions.  */

STATUS          PMC_Create_Partition_Pool(NU_PARTITION_POOL *pool_ptr, 
                        CHAR *name, VOID *start_address, UNSIGNED pool_size,
                        UNSIGNED partition_size, OPTION suspend_type);
STATUS          PMC_Delete_Partition_Pool(NU_PARTITION_POOL *pool_ptr);
STATUS          PMC_Allocate_Partition(NU_PARTITION_POOL *pool_ptr, 
                        VOID **return_pointer, UNSIGNED suspend);
STATUS          PMC_Deallocate_Partition(VOID *partition);


/* Information retrieval functions.  */

UNSIGNED        PMF_Established_Partition_Pools(VOID);
STATUS          PMF_Partition_Pool_Information(NU_PARTITION_POOL *pool_ptr, 
                  CHAR *name, VOID **start_address, UNSIGNED *pool_size,
                  UNSIGNED *partition_size, UNSIGNED *available,
                  UNSIGNED *allocated, OPTION *suspend_type, 
                  UNSIGNED *tasks_waiting, NU_TASK **first_task);
UNSIGNED        PMF_Partition_Pool_Pointers(NU_PARTITION_POOL **pointer_list,
                                                UNSIGNED maximum_pointers);

#endif




