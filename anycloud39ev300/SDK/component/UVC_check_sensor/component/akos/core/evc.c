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
/*      evc.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      EV - Event Group Management                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core routines for the Event Group         */
/*      Management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      EVC_Create_Event_Group              Create an event group        */
/*      EVC_Delete_Event_Group              Delete an event group        */
/*      EVC_Set_Events                      Set events in a group        */
/*      EVC_Retrieve_Events                 Retrieve events from a group */
/*      EVC_Cleanup                         Cleanup on timeout or a      */
/*                                            terminate condition        */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      ev_extr.h                           Event Group functions        */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Corrected pointer retrieval                      */
/*                      loop, resulting in version 1.0a                  */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Moved non-core functions into                    */
/*                      supplemental files, changed                      */
/*                      function interfaces to match                     */
/*                      those in prototype, added                        */
/*                      register options, changed                        */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      10-28-1997      Corrected a problem where                        */
/*                      NU_Set_Events may not resume all                 */
/*                      waiting tasks. (SPR190) Created                  */
/*                      version 1.2a.                                    */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "ev_extr.h"                 /* Event Group functions     */
#include        "hi_extr.h"                 /* History functions         */
#include        "profiler.h"                /* ProView interface         */


/* Define external inner-component global data references.  */

extern CS_NODE         *EVD_Created_Event_Groups_List;
extern UNSIGNED         EVD_Total_Event_Groups;
extern TC_PROTECT       EVD_List_Protect;


/* Define internal component function prototypes.  */

VOID    EVC_Cleanup(VOID *information);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      EVC_Create_Event_Group                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates an event group and then places it on the   */
/*      list of created event groups.                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      EVCE_Create_Event_Group             Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Add node to linked-list      */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Data structure protect       */
/*      TCT_Unprotect                       Un-protect data structure    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      event_group_ptr                     Event Group control block ptr*/
/*      name                                Event Group name             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options,                          */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  EVC_Create_Event_Group(NU_EVENT_GROUP *event_group_ptr, CHAR *name)
{

R1 EV_GCB      *event_group;                /* Event control block ptr   */
INT             i;                          /* Working index variable    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input event group pointer into internal pointer.  */
    event_group =  (EV_GCB *) event_group_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CREATE_EVENT_GROUP_ID, (UNSIGNED) event_group,
                                        (UNSIGNED) name, (UNSIGNED) 0);

#endif

    /* First, clear the event group ID just in case it is an old Event Group
       Control Block.  */
    event_group -> ev_id =             0;

    /* Fill in the event group name.  */
    for (i = 0; i < NU_MAX_NAME; i++)
        event_group -> ev_name[i] =  name[i];

    /* Clear the flags of the event group.  */
    event_group -> ev_current_events =  0;

    /* Clear the suspension list pointer.  */
    event_group -> ev_suspension_list =  NU_NULL;

    /* Clear the number of tasks waiting on the event_group counter.  */
    event_group -> ev_tasks_waiting =  0;

    /* Initialize link pointers.  */
    event_group -> ev_created.cs_previous =    NU_NULL;
    event_group -> ev_created.cs_next =        NU_NULL;

    /* Protect against access to the list of created event_groups.  */
    TCT_Protect(&EVD_List_Protect);

    /* At this point the event_group is completely built.  The ID can now be
       set and it can be linked into the created event_group list.  */
    event_group -> ev_id =  EV_EVENT_ID;

    /* Link the event group into the list of created event groups and
       increment the total number of event groups in the system.  */
    CSC_Place_On_List(&EVD_Created_Event_Groups_List,
                                        &(event_group -> ev_created));
    EVD_Total_Event_Groups++;
#ifdef INCLUDE_PROVIEW
    _RTProf_DumpEventGroup(RT_PROF_CREATE_EVENT_GROUP,event_group, RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/

    /* Release protection against access to the list of created event
       groups.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      EVC_Delete_Event_Group                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deletes an event group and removes it from the     */
/*      list of created event groups.  All tasks suspended on the        */
/*      event group are resumed.  Note that this function does not       */
/*      free the memory associated with the event group control block.   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      EVCE_Delete_Event_Group             Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove node from list        */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Protect                         Protect created list         */
/*      TCT_Set_Current_Protect             Modify current protection    */
/*      TCT_System_Protect                  Setup system protection      */
/*      TCT_System_Unprotect                Release system protection    */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      event_group_ptr                     Event Group control block ptr*/
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options, changed                  */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  EVC_Delete_Event_Group(NU_EVENT_GROUP *event_group_ptr)
{

R1 EV_GCB      *event_group;                /* Event control block ptr   */
EV_SUSPEND     *suspend_ptr;                /* Suspend block pointer     */
EV_SUSPEND     *next_ptr;                   /* Next suspend block        */
STATUS          preempt;                    /* Status for resume call    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input event group pointer into internal pointer.  */
    event_group =  (EV_GCB *) event_group_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_DELETE_EVENT_GROUP_ID, (UNSIGNED) event_group,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect against simultaneous access to the event group.  */
    TCT_System_Protect();

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpEventGroup(RT_PROF_DELETE_EVENT_GROUP, event_group, RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/

    /* Clear the event group ID.  */
    event_group -> ev_id =  0;

    /* Release protection.  */
    TCT_Unprotect();

    /* Protect against access to the list of created event groups.  */
    TCT_Protect(&EVD_List_Protect);

    /* Remove the event_group from the list of created event groups.  */
    CSC_Remove_From_List(&EVD_Created_Event_Groups_List,
                                        &(event_group -> ev_created));

    /* Decrement the total number of created event groups.  */
    EVD_Total_Event_Groups--;

    /* Pickup the suspended task pointer list.  */
    suspend_ptr =  event_group -> ev_suspension_list;

    /* Walk the chain task(s) currently suspended on the event_group.  */
    preempt =  0;
    while (suspend_ptr)
    {

        /* Protect against system access.  */
        TCT_System_Protect();

        /* Resume the suspended task.  Insure that the status returned is
           NU_GROUP_DELETED.  */
        suspend_ptr -> ev_return_status =  NU_GROUP_DELETED;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (EV_SUSPEND *) (suspend_ptr -> ev_suspend_link.cs_next);

        /* Resume the specified task.  */
        preempt =  preempt |
            TCC_Resume_Task((NU_TASK *) suspend_ptr -> ev_suspended_task,
                                                NU_EVENT_SUSPEND);

        /* Determine if the next is the same as the current pointer.  */
        if (next_ptr == event_group -> ev_suspension_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  NU_NULL;
        else

            /* Move the next pointer into the suspend block pointer.  */
            suspend_ptr =  next_ptr;

        /* Modify current protection.  */
        TCT_Set_Current_Protect(&EVD_List_Protect);

        /* Clear the system protection.  */
        TCT_System_Unprotect();
    }

    /* Determine if preemption needs to occur.  */
    if (preempt)

        /* Transfer control to system to facilitate preemption.  */
        TCT_Control_To_System();

    /* Release protection against access to the list of created
       event groups. */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return a successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      EVC_Set_Events                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sets event flags within the specified event flag   */
/*      group.  Event flags may be ANDed or ORed against the current     */
/*      events of the group.  Any task that is suspended on the group    */
/*      that has its request satisfied is resumed.                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      EVCE_Set_Events                     Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove from suspend list     */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_System_Protect                  Protect event group          */
/*      TCT_Unprotect                       Release protection           */
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
/*      NU_SUCCESS                          If service is successful     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options, changed                  */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      11-19-1996      Corrected SPR190.                                */
/*                                                                       */
/*************************************************************************/
STATUS  EVC_Set_Events(NU_EVENT_GROUP *event_group_ptr, UNSIGNED events,
                                                OPTION operation)
{

R1 EV_GCB      *event_group;                /* Event control block ptr   */
R2 EV_SUSPEND  *suspend_ptr;                /* Pointer to suspension blk */
R3 EV_SUSPEND  *next_ptr;                   /* Pointer to next suspend   */
R4 EV_SUSPEND  *last_ptr;                   /* Last suspension block ptr */
UNSIGNED        consume;                    /* Event flags to consume    */
UNSIGNED        compare;                    /* Event comparison variable */
INT             preempt;                    /* Preemption required flag  */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input event group pointer into internal pointer.  */
    event_group =  (EV_GCB *) event_group_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_SET_EVENTS_ID, (UNSIGNED) event_group,
                                      (UNSIGNED) events, (UNSIGNED) operation);

#endif

    /* Protect against simultaneous access to the event group.  */
    TCT_System_Protect();

    /* Perform the specified operation on the current event flags in the
       group.  */
    if (operation & EV_AND)

        /* AND the specified events with the current events.  */
        event_group -> ev_current_events =
                        event_group -> ev_current_events & events;
    else

        /* OR the specified events with the current events.  */
        event_group -> ev_current_events =
                        event_group -> ev_current_events | events;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpEventGroup(RT_PROF_SET_EVENTS,event_group , RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/
    /* Determine if there are any tasks suspended for events from this
       event flag group.  */
    if (event_group -> ev_suspension_list)
    {

        /* Initialize the consumption bits to 0.  */
        consume =  0;

        /* Now, walk the chain of tasks suspended on this event flag group to
           determine if any of their requests can be satisfied.  */
        suspend_ptr =  event_group -> ev_suspension_list;

        /* Setup a pointer to the last suspension block.  */
        last_ptr =  (EV_SUSPEND *) suspend_ptr -> ev_suspend_link.cs_previous;

        /* Clear the preempt flag.  */
        preempt =  0;
        do
        {

            /* Determine if this request has been satisfied.  */

            /* First, find the event flags in common.  */
            compare =  event_group -> ev_current_events &
                                        suspend_ptr -> ev_requested_events;

            /* Second, determine if all the event flags must match.  */
            if (suspend_ptr -> ev_operation & EV_AND)

                /* Yes, an AND condition is present.  All requested events
                   must be present.  */
                compare =  (compare == suspend_ptr -> ev_requested_events);

            /* Setup the next pointer.  Note that this must be done before
               the suspended task is resumed, since its suspend block could
               get corrupted.  */
            next_ptr =  (EV_SUSPEND *) suspend_ptr -> ev_suspend_link.cs_next;

            /* If compare is non-zero, the suspended task's event request is
               satisfied.  */
            if (compare)
            {

                /* Decrement the number of tasks waiting counter.  */
                event_group -> ev_tasks_waiting--;

                /* Determine if consumption is requested.  */
                if (suspend_ptr -> ev_operation & EV_CONSUME)

                    /* Keep track of the event flags to consume.  */
                    consume =  consume | suspend_ptr -> ev_requested_events;

                /* Remove the first suspended block from the list.  */
                CSC_Remove_From_List((CS_NODE **)
                        &(event_group -> ev_suspension_list),
                                &(suspend_ptr -> ev_suspend_link));

                /* Setup the appropriate return value.  */
                suspend_ptr -> ev_return_status =  NU_SUCCESS;
                suspend_ptr -> ev_actual_events =
                                        event_group -> ev_current_events;

                /* Resume the suspended task.  */
                preempt = preempt |
                 TCC_Resume_Task((NU_TASK *) suspend_ptr -> ev_suspended_task,
                                                       NU_EVENT_SUSPEND);

            }

            /* Determine if there is another suspension block to examine.  */
            if (suspend_ptr != last_ptr)

                /* More to examine in the suspension list.  Look at the
                   next suspend block.  */
                suspend_ptr =  next_ptr;
            else

                /* End of the list has been reached.  Set the suspend pointer
                   to NULL to end the search.  */
                suspend_ptr =  NU_NULL;

        } while (suspend_ptr);

        /* Apply all of the gathered consumption bits.  */
        event_group -> ev_current_events =
           event_group -> ev_current_events & ~consume;

        /* Determine if a preempt condition is present.  */
        if (preempt)

            /* Transfer control to the system if the resumed task function
               detects a preemption condition.  */
            TCT_Control_To_System();
    }

    /* Release protection of the event_group.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return a successful status.  */
    return(NU_SUCCESS);
}



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      EVC_Retrieve_Events                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function retrieves various combinations of event flags from */
/*      the specified event group.  If the group does not contain the    */
/*      necessary flags, suspension of the calling task is possible.     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      EVCE_Retrieve_Events                Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Place on suspend list        */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Suspend_Task                    Suspend calling task         */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Current_Thread                  Pickup current thread pointer*/
/*      TCT_System_Protect                  Protect event group          */
/*      TCT_Unprotect                       Release protection           */
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
/*      NU_SUCCESS                          If successful completion     */
/*      NU_TIMEOUT                          If timeout on suspension     */
/*      NU_NOT_PRESENT                      If event flags are not       */
/*                                            present                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options, changed                  */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  EVC_Retrieve_Events(NU_EVENT_GROUP *event_group_ptr,
                        UNSIGNED requested_events, OPTION operation,
                        UNSIGNED *retrieved_events, UNSIGNED suspend)
{

R1 EV_GCB      *event_group;                /* Event control block ptr   */
R2 EV_SUSPEND  *suspend_ptr;                /* Pointer to suspend block  */
EV_SUSPEND      suspend_block;              /* Suspension block          */
R3 UNSIGNED     compare;                    /* Event comparison variable */
TC_TCB         *task;                       /* Pointer to task           */
STATUS          status;                     /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input event group pointer into internal pointer.  */
    event_group =  (EV_GCB *) event_group_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_RETRIEVE_EVENTS_ID, (UNSIGNED) event_group,
                            (UNSIGNED) requested_events, (UNSIGNED) operation);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Protect against simultaneous access to the event group.  */
    TCT_System_Protect();

    /* Determine if the events requested are present.  */

    /* Isolate common event flags.  */
    compare =  event_group -> ev_current_events & requested_events;

    /* Determine if all of the events must be present.  */
    if (operation & EV_AND)

        /* Yes, all events must be present.  See if the compare value is
           the same as the requested value.  */
        compare =  (compare == requested_events);

    /* Determine if the requested combination of event flags are present.  */
    if (compare)
    {

        /* Yes, necessary event flags are present.  */

        /* Copy the current event flags into the appropriate destination.  */
        *retrieved_events =  event_group -> ev_current_events;

        /* Determine if consumption is required.  If so, consume the event
           flags present in the group.  */
        if (operation & EV_CONSUME)

            event_group -> ev_current_events =
                event_group -> ev_current_events & ~requested_events;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpEventGroup(RT_PROF_RETRIEVE_EVENTS,event_group, RT_PROF_OK);
#endif /*INCLUDE_PROVIEW*/

    }
    else
    {

        /* Determine if the task requested suspension.  */
        if (suspend)
        {

            /* Suspension is selected.  */

            /* Increment the number of tasks waiting.  */
            event_group -> ev_tasks_waiting++;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpEventGroup(RT_PROF_RETRIEVE_EVENTS,event_group, RT_PROF_WAIT);
#endif /*INCLUDE_PROVIEW*/

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> ev_event_group =              event_group;
            suspend_ptr -> ev_suspend_link.cs_next =     NU_NULL;
            suspend_ptr -> ev_suspend_link.cs_previous = NU_NULL;
            task =                            (TC_TCB *) TCT_Current_Thread();
            suspend_ptr -> ev_suspended_task =           task;
            suspend_ptr -> ev_requested_events =         requested_events;
            suspend_ptr -> ev_operation =                operation;

            /* Link the suspend block into the list of suspended tasks on this
               event group.  */
            CSC_Place_On_List((CS_NODE **)
                        &(event_group -> ev_suspension_list),
                                        &(suspend_ptr -> ev_suspend_link));

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the event group.  */
            TCC_Suspend_Task((NU_TASK *) task, NU_EVENT_SUSPEND,
                                        EVC_Cleanup, suspend_ptr, suspend);

            /* Pickup the return status and the actual retrieved events.  */
            status =             suspend_ptr -> ev_return_status;
            *retrieved_events =  suspend_ptr -> ev_actual_events;
        }
        else
        {

            /* No suspension requested.  Simply return an error status
               and zero the retrieved events variable.  */
            status =             NU_NOT_PRESENT;
            *retrieved_events =  0;
#ifdef INCLUDE_PROVIEW
            _RTProf_DumpEventGroup(RT_PROF_RETRIEVE_EVENTS,event_group,RT_PROF_FAIL);
#endif /*INCLUDE_PROVIEW*/
        }
    }

    /* Release protection of the event_group.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      EVC_Cleanup                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible for removing a suspension block     */
/*      from a event group.  It is not called unless a timeout or a task */
/*      terminate is in progress.  Note that protection is already in    */
/*      effect - the same protection at suspension time.  This routine   */
/*      must be called from Supervisor mode in Supervisor/User mode      */
/*      switching kernels.                                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TCC_Timeout                         Task timeout                 */
/*      TCC_Terminate                       Task terminate               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove suspend block from    */
/*                                            the suspension list        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      information                         Pointer to suspend block     */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
VOID  EVC_Cleanup(VOID *information)
{

EV_SUSPEND      *suspend_ptr;               /* Suspension block pointer  */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Use the information pointer as a suspend pointer.  */
    suspend_ptr =  (EV_SUSPEND *) information;

    /* By default, indicate that the service timed-out.  It really does not
       matter if this function is called from a terminate request since
       the task does not resume.  */
    suspend_ptr -> ev_return_status =  NU_TIMEOUT;

    /* Decrement the number of tasks waiting counter.  */
    (suspend_ptr -> ev_event_group) -> ev_tasks_waiting--;

    /* Unlink the suspend block from the suspension list.  */
    CSC_Remove_From_List((CS_NODE **)
                &((suspend_ptr -> ev_event_group) -> ev_suspension_list),
                                &(suspend_ptr -> ev_suspend_link));

    /* Return to user mode */
    NU_USER_MODE();
}







