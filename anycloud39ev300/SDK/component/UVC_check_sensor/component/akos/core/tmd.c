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
/*      tmd.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TM - Timer Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      timer management component.                                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      TMD_Created_Timers_List             Pointer to the linked-list   */
/*                                            of created application     */
/*                                            timers                     */
/*      TMD_Total_Timers                    Total number of created      */
/*                                            application timers         */
/*      TMD_Active_Timers_List              Pointer to the linked-list   */
/*                                            of active timers.          */
/*      TMD_Active_List_Busy                Flag indicating that the     */
/*                                            active timer list is in use*/
/*      TMD_Created_List_Protect            Created timer list protect   */
/*                                            structure                  */
/*      TMD_System_Clock                    System clock                 */
/*      TMD_Timer_Start                     Starting value of timer      */
/*      TMD_Timer                           Timer count-down value       */
/*      TMD_Timer_State                     State of timer               */
/*      TMD_Time_Slice                      Time slice count-down value  */
/*      TMD_Time_Slice_Task                 Pointer to task to time-slice*/
/*      TMD_Time_Slice_State                State of time slice          */
/*      TMD_HISR                            Timer HISR control block     */
/*      TMD_HISR_Stack_Ptr                  Pointer to HISR stack area   */
/*      TMD_HISR_Stack_Size                 Size of HISR stack area      */
/*      TMD_HISR_Priority                   Priority of timer HISR       */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      tm_defs.h                           Timer Management constants   */
/*      tc_defs.h                           Thread Control constants     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Removed timer task structures,                   */
/*                      making version 1.0a                              */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Removed active list protect data                 */
/*                      structure since system protect                   */
/*                      is now used, resulting in                        */
/*                      version 1.1                                      */
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

#include        "tm_defs.h"                 /* Timer constants           */


/* TMD_Created_Timers_List is the head pointer of the linked list of
   created application timers.  If the list is NU_NULL, there are no timers
   currently created.  */

CS_NODE        *TMD_Created_Timers_List;


/* TMD_Total_Timers contains the total number of created application timers
   in the system.  */

UNSIGNED        TMD_Total_Timers;


/* TMD_Active_Timers_List is the head pointer of the linked list of active
   timers.  This includes both the application timers and the system timers
   used for task sleeping and timeouts.  If the list is NU_NULL, there are
   no timers currently active.  */

TM_TCB         *TMD_Active_Timers_List;


/* TMD_Active_List_Busy is a flag that indicates that the active timer list
   is being processed.  This is used to prevent multiple updates to the
   active timer list.  */

INT             TMD_Active_List_Busy;


/* TMD_Created_List_Protect is used to protect the created application timers
   list from multiple accesses.  */

TC_PROTECT      TMD_Created_List_Protect;


/* TMD_System_Clock is a continually incrementing clock.  One is added to
   the clock each timer interrupt.  */

UNSIGNED        TMD_System_Clock;


/* TMD_Timer_Start represents the starting value of the last set timer
   request.  */

UNSIGNED        TMD_Timer_Start;


/* TMD_Timer is a count-down timer that is used to represent the smallest
   active timer value in the system.  Once this counter goes to zero, a
   timer has expired.  */

UNSIGNED        TMD_Timer;


/* TMD_Timer_State indicates the state of the timer variable.  If the state
   is active, the timer counter is decremented.  If the state is expired,
   the timer HISR and timer task are initiated to process the expiration.  If
   the state indicates that the timer is not-active, the timer counter is
   ignored.  */

INT             TMD_Timer_State;


/* TMD_Time_Slice contains the count-down value for the currently executing
   task's time slice.  When this value goes to zero, time slice processing
   is started.  */

UNSIGNED        TMD_Time_Slice;


/* TMD_Time_Slice_Task is a pointer to the task to time-slice.  This pointer
   is built in the portion of the timer interrupt that determines if a time-
   slice timer has expired.  */

TC_TCB         *TMD_Time_Slice_Task;


/* TMD_Time_Slice_State indicates the state of the time slice variable.  If
   the state is active, the time slice counter is decremented.  If the
   state is expired, the timer HISR is initiated to process the expiration.
   If the state indicates that the time slice is not-active, the time slice
   counter is ignored.  */

INT             TMD_Time_Slice_State;


/* TMD_HISR is the timer HISR's control block.  */

TC_HCB          TMD_HISR;


/* TMD_HISR_Stack_Ptr points to the memory area reserved for the timer HISR.
   Note that this is setup in INT_Initialize.  */

VOID           *TMD_HISR_Stack_Ptr;


/* TMD_HISR_Stack_Size represents the size of the allocated timer HISR stack.
   Note that this is setup in INT_Initialize.  */

UNSIGNED        TMD_HISR_Stack_Size;


/* TMD_HISR_Priority indicates the priority of the timer HISR.  Priorities
   range from 0 to 2, where priority 0 is the highest.  Note that this is
   also initialized in INT_Initialize.  */

INT             TMD_HISR_Priority;






