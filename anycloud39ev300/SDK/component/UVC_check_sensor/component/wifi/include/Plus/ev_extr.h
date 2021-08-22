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
/*      ev_extr.h                                       PLUS  1.3        */
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
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
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
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Moved include files outside of   */
/*                                        the file #ifndef to allow the  */
/*                                        use of actual data structures, */
/*                                        modified function prototypes,  */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
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
