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
/*      qu_defs.h                                       PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      QU - Queue Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data structure definitions and constants for  */
/*      the message Queue component.                                     */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      QU_QCB                              Queue control block          */
/*      QU_SUSPEND                          Queue suspension block       */
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
/*                                        removed protection structure,  */
/*                                        put padding into structure,    */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/

#include        "cs_defs.h"                 /* Common service constants  */
#include        "tc_defs.h"                 /* Thread control constants  */


/* Check to see if the file has been included already.  */

#ifndef QU_DEFS
#define QU_DEFS


/* Define constants local to this component.  */

#define         QU_QUEUE_ID             0x51554555UL


/* Define the Queue Control Block data type.  */

typedef struct QU_QCB_STRUCT 
{
    CS_NODE             qu_created;            /* Node for linking to    */
                                               /*   created queue list   */
    UNSIGNED            qu_id;                 /* Internal QCB ID        */
    CHAR                qu_name[NU_MAX_NAME];  /* Queue name             */
    DATA_ELEMENT        qu_fixed_size;         /* Fixed-size messages?   */
    DATA_ELEMENT        qu_fifo_suspend;       /* Suspension type flag   */
#if     PAD_2
    DATA_ELEMENT        qu_padding[PAD_2];
#endif
    UNSIGNED            qu_queue_size;         /* Total size of queue    */
    UNSIGNED            qu_messages;           /* Messages in queue      */
    UNSIGNED            qu_message_size;       /* Size of each message   */
    UNSIGNED            qu_available;          /* Available words        */
    UNSIGNED_PTR        qu_start;              /* Start of queue area    */
    UNSIGNED_PTR        qu_end;                /* End of queue area + 1  */
    UNSIGNED_PTR        qu_read;               /* Read pointer           */
    UNSIGNED_PTR        qu_write;              /* Write pointer          */
    UNSIGNED            qu_tasks_waiting;      /* Number of waiting tasks*/
    struct QU_SUSPEND_STRUCT
                       *qu_urgent_list;        /* Urgent message suspend */
    struct QU_SUSPEND_STRUCT
                       *qu_suspension_list;    /* Suspension list        */
} QU_QCB;    


/* Define the queue suspension structure.  This structure is allocated off of
   the caller's stack.  */
   
typedef struct QU_SUSPEND_STRUCT
{
    CS_NODE             qu_suspend_link;       /* Link to suspend blocks */
    QU_QCB             *qu_queue;              /* Pointer to the queue   */
    TC_TCB             *qu_suspended_task;     /* Task suspended         */
    UNSIGNED_PTR        qu_message_area;       /* Pointer to message area*/
    UNSIGNED            qu_message_size;       /* Message size requested */
    UNSIGNED            qu_actual_size;        /* Actual size of message */
    STATUS              qu_return_status;      /* Return status          */
} QU_SUSPEND;

#endif
