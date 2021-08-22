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
/*      ev_defs.h                                       PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      EV - Event Group Management                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data structure definitions and constants for  */
/*      the Event Flag Group component.                                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      EV_GCB                              Event Group control block    */
/*      EV_SUSPEND                          Event Group suspension block */
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
/*                                        removed protect structure,     */
/*                                        added padding logic,           */
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

#ifndef EV_DEFS
#define EV_DEFS


/* Define constants local to this component.  */

#define         EV_EVENT_ID         0x45564e54UL
#define         EV_AND              0x2        
#define         EV_CONSUME          0x1


/* Define the Event Group Control Block data type.  */

typedef struct EV_GCB_STRUCT 
{
    CS_NODE             ev_created;            /* Node for linking to    */
                                               /*   created Events list  */
    UNSIGNED            ev_id;                 /* Internal EV ID         */
    CHAR                ev_name[NU_MAX_NAME];  /* Event group name       */
    UNSIGNED            ev_current_events;     /* Current event flags    */
    UNSIGNED            ev_tasks_waiting;      /* Number of waiting tasks*/
    struct EV_SUSPEND_STRUCT
                       *ev_suspension_list;    /* Suspension list        */
} EV_GCB;    


/* Define the Event Group suspension structure.  This structure is allocated 
   off of the caller's stack.  */
   
typedef struct EV_SUSPEND_STRUCT
{
    CS_NODE             ev_suspend_link;       /* Link to suspend blocks */
    EV_GCB             *ev_event_group;        /* Pointer to Event group */
    UNSIGNED            ev_requested_events;   /* Requested event flags  */
    DATA_ELEMENT        ev_operation;          /* Event operation        */
#if     PAD_1
    DATA_ELEMENT        ev_padding[PAD_1];
#endif 
    TC_TCB             *ev_suspended_task;     /* Task suspended         */
    STATUS              ev_return_status;      /* Return status          */
    UNSIGNED            ev_actual_events;      /* Event flags returned   */
} EV_SUSPEND;

#endif
