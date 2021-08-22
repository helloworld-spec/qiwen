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
/*      tmf.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TM - Timer Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains information (fact) routines for the Timer     */
/*      Management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      TMF_Established_Timers              Number of established timers */
/*      TMF_Timer_Pointers                  Return list of application   */
/*                                            timer pointers             */
/*      TMF_Timer_Information               Return information about the */
/*                                              application timer        */
/*                                                                       */
/*      TMF_Get_Remaining_Time              Return remaining timer until */
/*                                          a timer expires              */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      tm_extr.h                           Timer functions              */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1994      Created initial version 1.1 from                 */
/*                      previous version of TMC.C                        */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      11-18-1996      Protected Informational service                  */
/*                      from NULL Control Block pointers                 */
/*                      creating 1.2a. (SPR220)                          */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "tm_extr.h"                 /* Timer functions           */
#include        "hi_extr.h"                 /* History functions         */


/* Define external inner-component global data references.  */

extern CS_NODE        *TMD_Created_Timers_List;
extern UNSIGNED        TMD_Total_Timers;
extern TM_TCB         *TMD_Active_Timers_List;
extern INT             TMD_Active_List_Busy;
extern TC_PROTECT      TMD_Created_List_Protect;



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMF_Established_Timers                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the current number of established          */
/*      timers.  Timers previously deleted are no longer considered      */
/*      established.                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      TMD_Total_Timers                    Number of established        */
/*                                            timers                     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interface,                      */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  TMF_Established_Timers(VOID)
{


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Return the number of established timers.  */
    return(TMD_Total_Timers);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMF_Timer_Pointers                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function builds a list of timer pointers, starting at the   */
/*      specified location.  The number of timer pointers placed in      */
/*      the list is equivalent to the total number of timers or the      */
/*      maximum number of pointers specified in the call.                */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created list         */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pointer_list                        Pointer to the list area     */
/*      maximum_pointers                    Maximum number of pointers   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      pointers                            Number of timers placed      */
/*                                            in the list                */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Corrected problem in pointer                     */
/*                      retrieval loop, resulting in                     */
/*                      version 1.0a                                     */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Changed function interface,                      */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  TMF_Timer_Pointers(NU_TIMER **pointer_list,
                                                UNSIGNED maximum_pointers)
{

CS_NODE         *node_ptr;                  /* Pointer to each TCB       */
UNSIGNED         pointers;                  /* Number of pointers in list*/
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Initialize the number of pointers returned.  */
    pointers =  0;

    /* Protect against access to the list of created timers.  */
    TCT_Protect(&TMD_Created_List_Protect);

    /* Loop until all timer pointers are in the list or until the maximum
       list size is reached.  */
    node_ptr =  TMD_Created_Timers_List;
    while ((node_ptr) && (pointers < maximum_pointers))
    {

        /* Place the node into the destination list.  */
        *pointer_list++ =  (NU_TIMER *) node_ptr;

        /* Increment the pointers variable.  */
        pointers++;

        /* Position the node pointer to the next node.  */
        node_ptr =  node_ptr -> cs_next;

        /* Determine if the pointer is at the head of the list.  */
        if (node_ptr == TMD_Created_Timers_List)

            /* The list search is complete.  */
            node_ptr =  NU_NULL;
    }

    /* Release protection against access to the list of created timers.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the number of pointers in the list.  */
    return(pointers);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMF_Timer_Information                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns information about the specified timer.     */
/*      However, if the supplied timer pointer is invalid, the           */
/*      function simply returns an error status.                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect active timer         */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      timer_ptr                           Pointer to the timer         */
/*      name                                Destination for the name     */
/*      enable                              Destination for the enable   */
/*                                            posture                    */
/*      expirations                         Destination for the total    */
/*                                            number of expirations      */
/*      id                                  Destination for the timer id */
/*      initial_time                        Destination for the initial  */
/*                                            time                       */
/*      reschedule_time                     Destination for the          */
/*                                            reschedule time            */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If a valid timer pointer     */
/*                                            is supplied                */
/*      NU_INVALID_TIMER                    If timer pointer invalid     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Corrected problem that caused an                 */
/*                      invalid application timer ID                     */
/*                      to be returned to the caller,                    */
/*                      resulting in version 1.0a                        */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Changed function interface,                      */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      11-18-1996      Corrected SPR220.                                */
/*                                                                       */
/*************************************************************************/
STATUS  TMF_Timer_Information(NU_TIMER *timer_ptr, CHAR *name,
                  OPTION *enable, UNSIGNED *expirations, UNSIGNED *id,
                  UNSIGNED *initial_time, UNSIGNED *reschedule_time)
{

TM_APP_TCB     *timer;                      /* Timer control block ptr   */
INT             i;                          /* Working integer variable  */
STATUS          completion;                 /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input timer pointer into internal pointer.  */
    timer =  (TM_APP_TCB *) timer_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Protect the active list.  */
    TCT_System_Protect();

    /* Determine if this timer ID is valid.  */
    if ((timer != NU_NULL) && (timer -> tm_id == TM_TIMER_ID))
    {

        /* The timer pointer is valid.  Reflect this in the completion
           status and fill in the actual information.  */
        completion =  NU_SUCCESS;

        /* Copy the timer's name.  */
        for (i = 0; i < NU_MAX_NAME; i++)
            *name++ =  timer -> tm_name[i];

        /* Determine if the timer is enabled or disabled.  */
        if (timer -> tm_enabled)

            *enable =  NU_ENABLE_TIMER;
        else

            *enable =  NU_DISABLE_TIMER;

        /* Fill in the remaining information.  */
        *expirations =          timer -> tm_expirations;
        *id =                   timer -> tm_expiration_id;
        *initial_time =         timer -> tm_initial_time;
        *reschedule_time =      timer -> tm_reschedule_time;
    }
    else

        /* Indicate that the timer pointer is invalid.   */
        completion =  NU_INVALID_TIMER;

    /* Release protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the appropriate completion status.  */
    return(completion);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMF_Get_Remaining_Time                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the remaining time before expiration for   */
/*      the specified timer.                                             */
/*                                                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TCT_System_Protect                  Protect active timer         */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      timer_ptr                           Pointer to the timer         */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      remaining_time                      time until timer expiration  */
/*                                                                       */
/*      NU_INVALID_TIMER                    If timer pointer invalid     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*       07-02-1998               Created service call                   */
/*                                                                       */
/*************************************************************************/


STATUS  TMF_Get_Remaining_Time(NU_TIMER *timer_ptr, UNSIGNED *remaining_time)
{
R1  TM_APP_TCB  *timer;
    TM_TCB      *real_TCB;
    TM_TCB      *list_ptr;
    INT         done = 0;
    STATUS      status;
    NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Protect against simultaneous access to the active timers list*/
    TCT_System_Protect();

    list_ptr = TMD_Active_Timers_List;

    /* Get the application timer Control Block */
    timer = (TM_APP_TCB*) timer_ptr;


    /* Determine if this timer ID is valid.  */
    if ((timer != NU_NULL) && (timer -> tm_id == TM_TIMER_ID))
    {
        /* The timer pointer is valid.  Reflect this in the completion
           status and fill in the actual information.  */
        status =  NU_SUCCESS;

        /* Get the actual timer Control block */
        real_TCB = &(timer->tm_actual_timer);

        if (list_ptr == real_TCB)
            *remaining_time = list_ptr -> tm_remaining_time;
        else
        {
            if( list_ptr == NU_NULL)        /* in case no active timers exist */
                *remaining_time = 0;
            else
            {

            *remaining_time = list_ptr -> tm_remaining_time;

            do
            {
                /* Move the list pointer to the next timer in the list.  */
                list_ptr =  list_ptr -> tm_next_timer;

                *remaining_time += list_ptr -> tm_remaining_time;

                /* Check to see if we have gotten to the specified timer yet */
                if (list_ptr == real_TCB)
                    /* Searching is done.  */
                    done =  NU_TRUE;

            }while (!done);
        }
    }
    }
    else
        /* Indicate that the timer pointer is invalid.   */
        status =  NU_INVALID_TIMER;

    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    return (status);
}




