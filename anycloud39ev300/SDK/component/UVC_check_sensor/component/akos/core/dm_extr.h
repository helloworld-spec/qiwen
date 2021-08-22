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
/*      dm_extr.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DM - Dynamic Memory Management                                   */
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
/*      dm_defs.h                           Dynamic Management constants */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                   REMARKS                                */
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
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      05-01-2000      Corrected a prefix problem for                   */
/*                      DMC_Established_Memory_Pools,                    */
/*                      DMC_Memory_Pool_Information,                     */
/*                      DMC_Memory_Pool_Pointers by                      */
/*                      changing them to DMF_                            */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/

#include        "dm_defs.h"                 /* Include DM constants      */


/* Check to see if the file has been included already.  */

#ifndef DM_EXTR
#define DM_EXTR


/*  Initialization functions.  */

VOID            DMI_Initialize(VOID);


/* Error checking functions.  */

STATUS          DMCE_Create_Memory_Pool(NU_MEMORY_POOL *pool_ptr, CHAR *name,
                        VOID *start_address, UNSIGNED pool_size,
                        UNSIGNED min_allocation, OPTION suspend_type);
STATUS          DMCE_Delete_Memory_Pool(NU_MEMORY_POOL *pool_ptr);
STATUS          DMCE_Allocate_Memory(NU_MEMORY_POOL *pool_ptr, VOID **return_pointer,
                                UNSIGNED size, UNSIGNED suspend);
STATUS          DMCE_Deallocate_Memory(VOID *memory);


/* Core processing functions.  */

STATUS          DMC_Create_Memory_Pool(NU_MEMORY_POOL *pool_ptr, CHAR *name, 
                        VOID *start_address, UNSIGNED pool_size,
                        UNSIGNED min_allocation, OPTION suspend_type);
STATUS          DMC_Delete_Memory_Pool(NU_MEMORY_POOL *pool_ptr);
STATUS          DMC_Allocate_Memory(NU_MEMORY_POOL *pool_ptr, 
                     VOID **return_pointer, UNSIGNED size, UNSIGNED suspend);
STATUS          DMC_Deallocate_Memory(VOID *memory);


/* Supplemental service routines */
STATUS DMS_Allocate_Aligned_Memory(NU_MEMORY_POOL *pool_ptr,
                                   VOID **return_pointer, UNSIGNED size,
                                   UNSIGNED alignment, UNSIGNED suspend);


/* Information retrieval functions.  */

UNSIGNED        DMF_Established_Memory_Pools(VOID);
STATUS          DMF_Memory_Pool_Information(NU_MEMORY_POOL *pool_ptr, 
                  CHAR *name, VOID **start_address, UNSIGNED *pool_size,
                  UNSIGNED *min_allocation, UNSIGNED *available,
                  OPTION *suspend_type, UNSIGNED *tasks_waiting, 
                  NU_TASK **first_task);
UNSIGNED        DMF_Memory_Pool_Pointers(NU_MEMORY_POOL **pointer_list, 
                                                UNSIGNED maximum_pointers);
#endif





