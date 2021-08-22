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
/*      hi_defs.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      HI - History Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data structure definitions and constants for  */
/*      the History Management component.                                */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      HI_HISTORY_ENTRY                    Entry in the history table   */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
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

#include        "tc_defs.h"                 /* Thread control constants  */


/* Check to see if the file has been included already.  */

#ifndef HI_DEFS
#define HI_DEFS


/* Define constants local to this component.  */

#define         HI_MAX_ENTRIES          30
#define         HI_TASK                 1
#define         HI_HISR                 2
#define         HI_INITIALIZE           3


/* Define the History Entry data type.  */

typedef struct HI_HISTORY_ENTRY_STRUCT 
{
    DATA_ELEMENT        hi_id;              /* ID of the history entry  */
    DATA_ELEMENT        hi_caller;          /* Task, HISR, or Initialize*/
    UNSIGNED            hi_param1;          /* First parameter          */
    UNSIGNED            hi_param2;          /* Second parameter         */
    UNSIGNED            hi_param3;          /* Third parameter          */
    UNSIGNED            hi_time;            /* Clock tick time for entry*/
    VOID               *hi_thread;          /* Calling thread's pointer */
} HI_HISTORY_ENTRY;    

#endif





