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
/*      tms.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TM - Timer Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains supplemental routines for the timer           */
/*      management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      TMS_Create_Timer                    Create an application timer  */
/*      TMS_Delete_Timer                    Delete an application timer  */
/*      TMS_Reset_Timer                     Reset application timer      */
/*      TMS_Control_Timer                   Enable/Disable application   */
/*                                            timer                      */
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
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1994      Created initial version 1.1 from                 */
/*                        previous version of TMC.C                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
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
#include        "profiler.h"                /* ProView interface         */


/* Define external inner-component global data references.  */

extern CS_NODE        *TMD_Created_Timers_List;
extern UNSIGNED        TMD_Total_Timers;
extern TM_TCB         *TMD_Active_Timers_List;
extern INT             TMD_Active_List_Busy;
extern TC_PROTECT      TMD_Created_List_Protect;


/* Define internal function prototypes.  */

VOID            TMC_Start_Timer(TM_TCB *timer, UNSIGNED time);
VOID            TMC_Stop_Timer(TM_TCB *timer);



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMS_Create_Timer                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates an application timer and places it on the  */
/*      list of created timers.  The timer is activated if designated by */
/*      the enable parameter.                                            */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TMSE_Create_Timer                   Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Add node to linked-list      */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Data structure protect       */
/*      TCT_Unprotect                       Un-protect data structure    */
/*      TMS_Control_Timer                   Enable the new timer         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      timer_ptr                           Timer control block pointer  */
/*      name                                Timer name                   */
/*      expiration_routine                  Timer expiration routine     */
/*      id                                  Timer expiration ID          */
/*      initial_time                        Initial expiration time      */
/*      reschedule_time                     Reschedule expiration time   */
/*      enable                              Automatic enable option      */
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
/*      03-01-1994      Changed function prototype,                      */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TMS_Create_Timer(NU_TIMER *timer_ptr, CHAR *name,
                VOID (*expiration_routine)(UNSIGNED), UNSIGNED id,
                UNSIGNED initial_time, UNSIGNED reschedule_time, OPTION enable)
{

R1 TM_APP_TCB  *timer;                      /* Timer control block ptr   */
INT             i;                          /* Working index variable    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input timer pointer into internal pointer.  */
    timer =  (TM_APP_TCB *) timer_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CREATE_TIMER_ID, (UNSIGNED) timer,
                               (UNSIGNED) name, (UNSIGNED) expiration_routine);

#endif

    /* First, clear the timer ID just in case it is an old Timer
       Control Block.  */
    timer -> tm_id =             0;

    /* Fill in the timer name.  */
    for (i = 0; i < NU_MAX_NAME; i++)
        timer -> tm_name[i] =  name[i];

    /* Load the timer with the appropriate values.  */
    timer -> tm_expiration_routine =            expiration_routine;
    timer -> tm_expiration_id =                 id;
    timer -> tm_expirations =                   0;
    timer -> tm_initial_time =                  initial_time;
    timer -> tm_reschedule_time =               reschedule_time;
    timer -> tm_actual_timer.tm_timer_type =    TM_APPL_TIMER;
    timer -> tm_enabled =                       NU_FALSE;

    /* Initialize link pointers.  */
    timer -> tm_created.cs_previous =           NU_NULL;
    timer -> tm_created.cs_next =               NU_NULL;
    timer -> tm_actual_timer.tm_next_timer =    NU_NULL;
    timer -> tm_actual_timer.tm_previous_timer= NU_NULL;
    timer -> tm_actual_timer.tm_information =   (VOID *) timer;

    /* Protect against access to the list of created timers.  */
    TCT_Protect(&TMD_Created_List_Protect);

    /* At this point the timer is completely built.  The ID can now be
       set and it can be linked into the created timer list.  */
    timer -> tm_id =                     TM_TIMER_ID;

    /* Link the timer into the list of created timers and increment the
       total number of timers in the system.  */
    CSC_Place_On_List(&TMD_Created_Timers_List, &(timer -> tm_created));
    TMD_Total_Timers++;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTimer(RT_PROF_CREATE_TIMER,timer,RT_PROF_OK);
#endif

    /* Release protection against access to the list of created timers.  */
    TCT_Unprotect();

    /* Determine if the timer should be enabled.  */
    if (enable == NU_ENABLE_TIMER)

        /* Activate the timer.  */
        TMS_Control_Timer(timer_ptr, NU_ENABLE_TIMER);

    /* Return to user mode */
    NU_USER_MODE();

    /* Return successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMS_Delete_Timer                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deletes an application timer and removes it from   */
/*      the list of created timers.                                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TMSE_Delete_Timer                   Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove node from list        */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created list         */
/*      TCT_System_Protect                  Protect active list          */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      timer_ptr                           Timer control block pointer  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_NOT_DISABLED                     Timer not disabled first     */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified protection logic to use                 */
/*                        system protection, changed                     */
/*                        function prototype, resulting                  */
/*                        in version 1.1                                 */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TMS_Delete_Timer(NU_TIMER *timer_ptr)
{

TM_APP_TCB     *timer;                      /* Timer control block ptr  */
STATUS          status;                     /* Completion status        */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input timer pointer into internal pointer.  */
    timer =  (TM_APP_TCB *) timer_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_DELETE_TIMER_ID, (UNSIGNED) timer,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Initialize the status.  */
    status =  NU_SUCCESS;

    /* Use system protect to protect the active timer list temporarily.  */
    TCT_System_Protect();

    /* Determine if the timer is currently disabled. */
    if (timer -> tm_enabled)
    {
        /* Error, indicate to the caller that the timer is currently active. */
        status =  NU_NOT_DISABLED;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpTimer(RT_PROF_DELETE_TIMER,timer,RT_PROF_FAIL);
#endif

    }
    else
    {
        /* Clear the timer ID.  */
        timer -> tm_id =  0;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpTimer(RT_PROF_DELETE_TIMER,timer,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

    }

    /* Release protection.  */
    TCT_Unprotect();

    /* Determine if an error was detected.  */
    if (status == NU_SUCCESS)
    {

        /* Protect against access to the list of created timers.  */
        TCT_Protect(&TMD_Created_List_Protect);

        /* Remove the timer from the list of created timers.  */
        CSC_Remove_From_List(&TMD_Created_Timers_List, &(timer -> tm_created));

        /* Decrement the total number of created timers.  */
        TMD_Total_Timers--;

        /* Release protection against access to the list of created timers.  */
        TCT_Unprotect();
    }

    /* Return to user mode */
    NU_USER_MODE();

    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMS_Reset_Timer                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function resets the specified application timer.  Note that */
/*      the timer must be in a disabled state prior to this call.  The   */
/*      timer is activated after it is reset if the enable parameter     */
/*      specifies automatic activation.                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TMSE_Reset_Timer                    Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect active list          */
/*      TCT_Unprotect                       Release protection           */
/*      TMS_Control_Timer                   Enable/disable timer         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      timer_ptr                           Timer control block pointer  */
/*      expiration_routine                  Timer expiration routine     */
/*      initial_time                        Initial expiration time      */
/*      reschedule_time                     Reschedule expiration time   */
/*      enable                              Automatic enable option      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_NOT_DISABLED                     Timer not disabled first     */
/*      NU_SUCCESS                          Successful completion        */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified protection logic to use                 */
/*                        system protection, changed                     */
/*                        function prototype, resulting                  */
/*                        in version 1.1                                 */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TMS_Reset_Timer(NU_TIMER *timer_ptr,
                VOID (*expiration_routine)(UNSIGNED),
                UNSIGNED initial_time, UNSIGNED reschedule_time, OPTION enable)
{

R1 TM_APP_TCB  *timer;                      /* Timer control block ptr  */
STATUS          status;                     /* Completion status        */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input timer pointer into internal pointer.  */
    timer =  (TM_APP_TCB *) timer_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_RESET_TIMER_ID, (UNSIGNED) timer,
                       (UNSIGNED) expiration_routine, (UNSIGNED) initial_time);

#endif

    /* Protect against access to the active timer list.  */
    TCT_System_Protect();

    /* Determine if this timer is active.  An active timer cannot be
       reset.  */
    if (timer -> tm_enabled)
    {

        /* Indicate that the timer is active by returning the proper status. */
        status =  NU_NOT_DISABLED;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpTimer(RT_PROF_RESET_TIMER,timer,RT_PROF_FAIL);
#endif /* INCLUDE_PROVIEW */

    }
    else
    {

        /* Load the timer with the appropriate values.  */
        timer -> tm_expiration_routine =    expiration_routine;
        timer -> tm_expirations =           0;
        timer -> tm_initial_time =          initial_time;
        timer -> tm_reschedule_time =       reschedule_time;

        /* Indicate successful completion status.  */
        status =  NU_SUCCESS;
#ifdef INCLUDE_PROVIEW
        _RTProf_DumpTimer(RT_PROF_RESET_TIMER,timer,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

    }

    /* Release protection.  */
    TCT_Unprotect();

    /* Determine if the timer needs to be enabled.  */
    if ((status == NU_SUCCESS) && (enable == NU_ENABLE_TIMER))

        /* Activate the timer.  */
        TMS_Control_Timer(timer_ptr, NU_ENABLE_TIMER);

    /* Return to user mode */
    NU_USER_MODE();

    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMS_Control_Timer                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function either enables or disables the specified timer.    */
/*      If the timer is already in the desired state, simply leave it    */
/*      alone.                                                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TMSE_Control_Timer                Error checking shell           */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect the active list      */
/*      TCT_Unprotect                       Release protection           */
/*      TMC_Start_Timer                     Start a timer                */
/*      TMC_Stop_Timer                      Stop a timer                 */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      app_timer                           Timer control block pointer  */
/*      enable                              Disable/enable timer option  */
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
/*      03-01-1994      Modified protection logic to use                 */
/*                        system protection, changed                     */
/*                        function prototype, resulting                  */
/*                        in version 1.1                                 */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TMS_Control_Timer(NU_TIMER *app_timer, OPTION enable)
{

R1 TM_APP_TCB  *timer;                      /* Timer control block ptr   */
TM_TCB         *timer_ptr;                  /* Actual timer pointer      */
UNSIGNED        time;                       /* Variable to hold request  */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input timer pointer into internal pointer.  */
    timer =  (TM_APP_TCB *) app_timer;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CONTROL_TIMER_ID, (UNSIGNED) timer,
                                        (UNSIGNED) enable, (UNSIGNED) 0);

#endif

    /* Protect against simultaneous access to the active timer list.  */
    TCT_System_Protect();

    /* Setup pointer to actual timer part of the control block.  */
    timer_ptr =  &(timer -> tm_actual_timer);

    /* Determine what type of request is present.  */
    if ((enable == NU_ENABLE_TIMER) && (!timer -> tm_enabled))
    {

        /* Enable timer request is present and timer is currently disabled.  */

        /* Determine how to setup the remaining field in the actual timer. */
        if (timer -> tm_expirations)

            /* Use reschedule time since this timer has expired previously. */
            time =  timer -> tm_reschedule_time;
        else

            /* Use initial time since this timer has never expired.  */
            time =  timer -> tm_initial_time;

        /* Mark the application timer as enabled.  */
        timer -> tm_enabled =  NU_TRUE;

        /* Call the start timer routine to actually start the timer.  */
        TMC_Start_Timer(&(timer -> tm_actual_timer), time);
    }
    else if ((enable == NU_DISABLE_TIMER) && (timer -> tm_enabled))
    {

        /* Disable timer request is present and timer is currently enabled.  */
        TMC_Stop_Timer(timer_ptr);

        /* Mark the timer as disabled.  */
        timer -> tm_enabled =  NU_FALSE;
    }

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpTimer(RT_PROF_CONTROL_TIMER,timer,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */
    /* Release protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the completion status.  */
    return(NU_SUCCESS);
}




