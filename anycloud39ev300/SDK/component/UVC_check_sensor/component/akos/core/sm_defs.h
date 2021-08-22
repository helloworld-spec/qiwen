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
/*      sm_defs.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      SM - Semaphore Management                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data structure definitions and constants for  */
/*      the Semaphore component.                                         */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      SM_SCB                              Semaphore control block      */
/*      SM_SUSPEND                          Semaphore suspension block   */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_defs.h                           Common service definitions   */
/*      tc_defs.h                           Thread Control definitions   */
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
/*                      removed protection structure,                    */
/*                      put padding into structure,                      */
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

#include        "cs_defs.h"                 /* Common service constants  */
#include        "tc_defs.h"                 /* Thread control constants  */


/* Check to see if the file has been included already.  */

#ifndef SM_DEFS
#define SM_DEFS


/* Define constants local to this component.  */

#define         SM_SEMAPHORE_ID         0x53454d41UL


/* Define the Semaphore Control Block data type.  */

typedef struct SM_SCB_STRUCT 
{
    CS_NODE             sm_created;            /* Node for linking to       */
                                               /* created semaphore list */
    UNSIGNED            sm_id;                 /* Internal SCB ID           */
    CHAR                sm_name[NU_MAX_NAME];  /* Semaphore name        */
    UNSIGNED            sm_semaphore_count;    /* Counting semaphore    */
    UNSIGNED            sm_init_count;         /* semaphore init number */
    BOOLEAN             sm_fifo_suspend;       /* Suspension type flag   */
#if     PAD_1
    DATA_ELEMENT        sm_padding[PAD_1];
#endif
    UNSIGNED            sm_tasks_waiting;      /* Number of waiting tasks*/
    struct SM_SUSPEND_STRUCT
                       *sm_suspension_list;    /* Suspension list        */
} SM_SCB;    


/* Define the semaphore suspension structure.  This structure is allocated 
   off of the caller's stack.  */
   
typedef struct SM_SUSPEND_STRUCT
{
    CS_NODE             sm_suspend_link;       /* Link to suspend blocks */
    SM_SCB             *sm_semaphore;          /* Pointer to semaphore   */
    TC_TCB             *sm_suspended_task;     /* Task suspended         */
    STATUS              sm_return_status;      /* Return status          */
} SM_SUSPEND;

#endif




