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
/*      sm_extr.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      SM - Semaphore Management                                        */
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
/*      sm_defs.h                           Semaphore Management constant*/
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

#include        "sm_defs.h"                 /* Include SM constants      */


/* Check to see if the file has been included already.  */

#ifndef SM_EXTR
#define SM_EXTR


/*  Initialization functions.  */

VOID            SMI_Initialize(VOID);


/* Error checking core functions.  */

STATUS          SMCE_Create_Semaphore(NU_SEMAPHORE *semaphore_ptr, CHAR *name,
                        UNSIGNED initial_count, OPTION suspend_type);
STATUS          SMCE_Delete_Semaphore(NU_SEMAPHORE *semaphore_ptr);
STATUS          SMCE_Obtain_Semaphore(NU_SEMAPHORE *semaphore_ptr, 
                                                        UNSIGNED suspend);
STATUS          SMCE_Release_Semaphore(NU_SEMAPHORE *semaphore_ptr);


/* Error checking supplemental functions.  */

STATUS          SMSE_Reset_Semaphore(NU_SEMAPHORE *semaphore_ptr, 
                                                    UNSIGNED initial_count);


/* Core processing functions.  */

STATUS          SMC_Create_Semaphore(NU_SEMAPHORE *semaphore_ptr, CHAR *name,
                        UNSIGNED initial_count, OPTION suspend_type);
STATUS          SMC_Delete_Semaphore(NU_SEMAPHORE *semaphore_ptr);
STATUS          SMC_Obtain_Semaphore(NU_SEMAPHORE *semaphore_ptr, 
                                                        UNSIGNED suspend);
STATUS          SMC_Release_Semaphore(NU_SEMAPHORE *semaphore_ptr);


/* Supplemental processing functions.  */

STATUS          SMS_Reset_Semaphore(NU_SEMAPHORE *semaphore_ptr, 
                                                UNSIGNED initial_count);


/* Information retrieval functions.  */

UNSIGNED        SMF_Established_Semaphores(VOID);
STATUS          SMF_Semaphore_Information(NU_SEMAPHORE *semaphore_ptr, 
                  CHAR *name, UNSIGNED *current_count, OPTION *suspend_type, 
                  UNSIGNED *tasks_waiting, NU_TASK **first_task);
UNSIGNED        SMF_Semaphore_Pointers(NU_SEMAPHORE **pointer_list, 
                                                UNSIGNED maximum_pointers);
#endif




