/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1998 Accelerated Technology, Inc.           */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      mb_defs.h                                       PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      MB - Mailbox Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data structure definitions and constants for  */
/*      the message Mailbox component.                                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      MB_MCB                              Mailbox control block        */
/*      MB_SUSPEND                          Mailbox suspension block     */
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
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Moved include files outside of   */
/*                                        the file #ifndef to allow the  */
/*                                        use of actual data structures, */
/*                                        removed protection structure   */
/*                                        inside of each mailbox, added  */
/*                                        padding logic, resulting in    */
/*                                        version 1.1                    */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/

#include        "cs_defs.h"                 /* Common service constants  */
#include        "tc_defs.h"                 /* Thread control constants  */


/* Check to see if the file has been included already.  */

#ifndef MB_DEFS
#define MB_DEFS


/* Define constants local to this component.  */

#define         MB_MAILBOX_ID           0x4d424f58UL
#define         MB_MESSAGE_SIZE         4


/* Define the Mailbox Control Block data type.  */

typedef struct MB_MCB_STRUCT 
{
    CS_NODE             mb_created;            /* Node for linking to    */
                                               /*   created mailbox list */
    UNSIGNED            mb_id;                 /* Internal MCB ID        */
    CHAR                mb_name[NU_MAX_NAME];  /* Mailbox name           */
    DATA_ELEMENT        mb_message_present;    /* Message present flag   */
    DATA_ELEMENT        mb_fifo_suspend;       /* Suspension type flag   */
#if     PAD_2
    DATA_ELEMENT        mb_padding[PAD_2];
#endif
    UNSIGNED            mb_tasks_waiting;      /* Number of waiting tasks*/
    UNSIGNED                                   /* Message area           */
                        mb_message_area[MB_MESSAGE_SIZE];
    struct MB_SUSPEND_STRUCT
                       *mb_suspension_list;    /* Suspension list        */
} MB_MCB;    


/* Define the mailbox suspension structure.  This structure is allocated off of
   the caller's stack.  */
   
typedef struct MB_SUSPEND_STRUCT
{
    CS_NODE             mb_suspend_link;       /* Link to suspend blocks */
    MB_MCB             *mb_mailbox;            /* Pointer to the mailbox */
    TC_TCB             *mb_suspended_task;     /* Task suspended         */
    UNSIGNED           *mb_message_area;       /* Pointer to message area*/
    STATUS              mb_return_status;      /* Return status          */
} MB_SUSPEND;

#endif
