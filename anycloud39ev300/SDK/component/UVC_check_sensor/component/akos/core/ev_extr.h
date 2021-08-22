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
/*      ev_extr.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      EV - Event Group Management                                      */
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
/*      ev_defs.h                           Event Flag management consts */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
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

#include        "ev_defs.h"                 /* Include EV constants      */


/* Check to see if the file has been included already.  */

#ifndef EV_EXTR
#define EV_EXTR


/*  Initialization functions.  */

VOID            EVI_Initialize(VOID);


/* Error checking functions.  */

STATUS          EVCE_Create_Event_Group(NU_EVENT_GROUP *group_ptr, CHAR *name);
STATUS          EVCE_Delete_Event_Group(NU_EVENT_GROUP *group_ptr);
STATUS          EVCE_Set_Events(NU_EVENT_GROUP *group_ptr, UNSIGNED events, 
                                                OPTION operation);
STATUS          EVCE_Retrieve_Events(NU_EVENT_GROUP *group_ptr, 
                        UNSIGNED requested_flags, OPTION operation, 
                        UNSIGNED *retrieved_flags, UNSIGNED suspend);

/* Core processing functions.  */


STATUS          EVC_Create_Event_Group(NU_EVENT_GROUP *group_ptr, CHAR *name);
STATUS          EVC_Delete_Event_Group(NU_EVENT_GROUP *group_ptr);
STATUS          EVC_Set_Events(NU_EVENT_GROUP *group_ptr, UNSIGNED events, 
                                                OPTION operation);
STATUS          EVC_Retrieve_Events(NU_EVENT_GROUP *group_ptr, 
                        UNSIGNED requested_flags, OPTION operation, 
                        UNSIGNED *retrieved_flags, UNSIGNED suspend);


/* Information retrieval functions.  */

UNSIGNED        EVF_Established_Event_Groups(VOID);
STATUS          EVF_Event_Group_Information(NU_EVENT_GROUP *group_ptr, 
                        CHAR *name, UNSIGNED *event_flags, 
                        UNSIGNED *tasks_waiting, NU_TASK **first_task);
UNSIGNED        EVF_Event_Group_Pointers(NU_EVENT_GROUP **pointer_list, 
                                                UNSIGNED maximum_pointers);
#endif





