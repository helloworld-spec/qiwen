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
/*      tm_defs.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TM - Timer Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data structure definitions and constants for  */
/*      the Timer Management component.                                  */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      TM_TCB                              Timer control block          */
/*      TM_APP_TCB                          Application timer control    */
/*                                            block                      */   
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_defs.h                           Common service definitions   */
/*      tc_defs.h                           Thread control definitions   */
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
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/

#include        "cs_defs.h"                 /* Common service constants  */


/* Check to see if the file has been included already.  */

#ifndef TM_DEFS
#define TM_DEFS


/* Define constants local to this component.  */

#define         TM_TIMER_ID             0x54494d45UL
#define         TM_ACTIVE               0
#define         TM_NOT_ACTIVE           1
#define         TM_EXPIRED              2
#define         TM_TASK_TIMER           0
#define         TM_APPL_TIMER           1

/* Define the Timer Control Block data type.  */

typedef struct TM_TCB_STRUCT 
{
    INT                 tm_timer_type;         /* Application/Task      */
    UNSIGNED            tm_remaining_time;     /* Remaining time        */
    VOID               *tm_information;        /* Information pointer   */
    struct TM_TCB_STRUCT
                       *tm_next_timer,         /* Next timer in list    */
                       *tm_previous_timer;     /* Previous timer in list*/
} TM_TCB;    


/* Define Application's Timer Control Block data type.  */

typedef struct TM_APP_TCB_STRUCT
{
    CS_NODE             tm_created;            /* Node for linking to   */
                                               /*   created timer list  */
    UNSIGNED            tm_id;                 /* Internal TCB ID       */
    CHAR                tm_name[NU_MAX_NAME];  /* Timer name            */
    VOID  (*tm_expiration_routine)(UNSIGNED);  /* Expiration function   */
    UNSIGNED            tm_expiration_id;      /* Expiration ID         */
    BOOLEAN             tm_enabled;            /* Timer enabled flag    */

#if     PAD_3
    DATA_ELEMENT        tc_padding[PAD_3];
#endif

    UNSIGNED            tm_expirations;        /* Number of expirations */
    UNSIGNED            tm_initial_time;       /* Initial time          */
    UNSIGNED            tm_reschedule_time;    /* Reschedule time       */
    TM_TCB              tm_actual_timer;       /* Actual timer internals*/
} TM_APP_TCB;


/* Include this file here, since it contains references to the timer definition
   structure that is defined by this file.  */
   
#include        "tc_defs.h"

#endif /* TM_DEFS */




