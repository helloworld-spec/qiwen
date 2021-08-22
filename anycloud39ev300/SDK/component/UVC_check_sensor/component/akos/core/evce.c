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
/*      evce.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      EV - Event Group Management                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the error checking routines for the functions */
/*      in the Event Group component.  This permits easy removal of      */
/*      error checking logic when it is not needed.                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      EVCE_Create_Event_Group             Create an event group        */
/*      EVCE_Delete_Event_Group             Delete an event group        */
/*      EVCE_Set_Events                     Set events in a group        */
/*      EVCE_Retrieve_Events                Retrieve events from a group */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      ev_extr.h                           Event Group functions        */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed name original error                      */
/*                      checking file and changed                        */
/*                      function interfaces, resulting                   */
/*                      in version 1.1                                   */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "ev_extr.h"                 /* Event Group functions     */



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      EVCE_Create_Event_Group                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the create event group function.                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      EVC_Create_Event_Group              Actual create event group    */
/*                                            function                   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      event_group_ptr                     Event Group control block ptr*/
/*      name                                Event Group name             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_GROUP                    Event group control block    */
/*                                            pointer is NULL            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  EVCE_Create_Event_Group(NU_EVENT_GROUP *event_group_ptr, CHAR *name)
{

EV_GCB         *event_group;                /* Event control block ptr   */
STATUS          status;                     /* Completion status         */


    /* Move input event group pointer into internal pointer.  */
    event_group =  (EV_GCB *) event_group_ptr;

    /* Check for a NULL event group pointer or an already created event 
       group.  */
    if ((event_group == NU_NULL) || (event_group -> ev_id == EV_EVENT_ID))
    
        /* Invalid event group control block pointer.  */
        status =  NU_INVALID_GROUP;
        
    else
    
        /* Call the actual service to create the event group.  */
        status =  EVC_Create_Event_Group(event_group_ptr, name);
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      EVCE_Delete_Event_Group                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the delete event group function.                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      EVC_Delete_Event_Group              Actual delete event group    */
/*                                            function                   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      event_group_ptr                     Event Group control block ptr*/
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_GROUP                    Event group control block    */
/*                                            pointer is invalid         */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  EVCE_Delete_Event_Group(NU_EVENT_GROUP *event_group_ptr)
{

EV_GCB         *event_group;                /* Event control block ptr   */
STATUS          status;                     /* Completion status         */


    /* Move input event group pointer into internal pointer.  */
    event_group =  (EV_GCB *) event_group_ptr;

    /* Determine if the event group pointer is valid.  */
    if ((event_group) && (event_group -> ev_id == EV_EVENT_ID))
    
        /* Event group pointer is valid, call function to delete it.  */
        status =  EVC_Delete_Event_Group(event_group_ptr);
        
    else
    
        /* Event group pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_GROUP;
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      EVCE_Set_Events                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the set events function.                                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      EVC_Set_Events                      Actual set events function   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      event_group_ptr                     Event Group control block ptr*/
/*      events                              Event flag setting           */
/*      operation                           Operation to perform on the  */
/*                                            event flag group (AND/OR)  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_GROUP                    Event group control block    */
/*                                            pointer is invalid         */
/*      NU_INVALID_OPERATION                Event operation is invalid   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  EVCE_Set_Events(NU_EVENT_GROUP *event_group_ptr, UNSIGNED events, 
                                                OPTION operation)
{

EV_GCB         *event_group;                /* Event control block ptr   */
STATUS          status;                     /* Completion status         */


    /* Move input event group pointer into internal pointer.  */
    event_group =  (EV_GCB *) event_group_ptr;

    /* Determine if event group pointer is invalid.  */
    if (event_group == NU_NULL)
    
        /* Event group pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_GROUP;

    else if (event_group -> ev_id != EV_EVENT_ID)
    
        /* Event group pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_GROUP;
        
    else if ((operation != NU_AND) && (operation != NU_OR))
             
        /* Invalid operation on the event flag group.  */
        status =  NU_INVALID_OPERATION;

    else 
    
        /* Parameters are valid, call actual function.  */
        status =  EVC_Set_Events(event_group_ptr, events, operation);

    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      EVCE_Retrieve_Events                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameter supplied  */
/*      to the retrieve events function.                                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      EVC_Retrieve_Events                 Retrieve event flags         */
/*      TCCE_Suspend_Error                  Check for suspend validity   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      event_group_ptr                     Event Group control block ptr*/
/*      requested_events                    Requested event flags        */
/*      operation                           AND/OR selection of flags    */
/*      retrieved_events                    Pointer to destination for   */
/*                                            actual flags retrieved     */
/*      suspend                             Suspension option            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_GROUP                    Event group control block    */
/*                                            pointer is invalid         */
/*      NU_INVALID_POINTER                  Received event flag pointer  */
/*                                            is NULL                    */
/*      NU_INVALID_OPERATION                Event operation is invalid   */
/*      NU_INVALID_SUSPEND                  Invalid suspension request   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  EVCE_Retrieve_Events(NU_EVENT_GROUP *event_group_ptr, 
                        UNSIGNED requested_events, OPTION operation, 
                        UNSIGNED *retrieved_events, UNSIGNED suspend)
{

EV_GCB         *event_group;                /* Event control block ptr   */
STATUS          status;                     /* Completion status         */


    /* Move input event group pointer into internal pointer.  */
    event_group =  (EV_GCB *) event_group_ptr;

    /* Determine if event group pointer is invalid.  */
    if (event_group == NU_NULL)
    
        /* Event group pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_GROUP;

    else if (event_group -> ev_id != EV_EVENT_ID)
    
        /* Event group pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_GROUP;
        
    else if ((operation != NU_AND) &&
             (operation != NU_AND_CONSUME) &&
             (operation != NU_OR) &&
             (operation != NU_OR_CONSUME))
             
        /* Invalid operation on the event flag group.  */
        status =  NU_INVALID_OPERATION;

    else if ((suspend) && (TCCE_Suspend_Error()))
    
        /* Suspension from an non-task thread.  */
        status =  NU_INVALID_SUSPEND;

    else if (retrieved_events == NU_NULL)
    
        /* Retrieved events pointer is NULL.  */
        status =  NU_INVALID_POINTER;

    else 
    
        /* Parameters are valid, call actual function.  */
        status =  EVC_Retrieve_Events(event_group_ptr, requested_events,
                                        operation, retrieved_events, suspend);

    /* Return the completion status.  */
    return(status);
}






