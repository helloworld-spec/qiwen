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
/*      tmi.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TM - Timer Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the initialization routine for the timer      */
/*      management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      TMI_Initialize                      Timer Management Initialize  */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      tc_extr.h                           Task function interfaces     */
/*      er_extr.h                           Error handling function      */
/*      tm_defs.h                           Timer component constants    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Removed timer task creation,                     */
/*                        cleared the ID of the HISR                     */
/*                        control block, and casted the                  */
/*                        HISR priority, resulting in                    */
/*                        version 1.0a                                   */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Changed names of HISR create                     */
/*                        function, removed reference to                 */
/*                        timer list protect since it                    */
/*                        was deleted, resulting in                      */
/*                        version 1.1                                    */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "tc_extr.h"                 /* Task control functions    */
#include        "er_extr.h"                 /* Error handling function   */
#include        "tm_defs.h"                 /* Timer constants           */
#include        "tm_extr.h"                 /* Timer interfaces          */


/* Define external inner-component global data references.  */

extern CS_NODE        *TMD_Created_Timers_List;
extern UNSIGNED        TMD_Total_Timers;
extern TM_TCB         *TMD_Active_Timers_List;
extern INT             TMD_Active_List_Busy;
extern TC_PROTECT      TMD_Created_List_Protect;
extern UNSIGNED        TMD_System_Clock;
extern UNSIGNED        TMD_Timer;
extern INT             TMD_Timer_State;
extern UNSIGNED        TMD_Time_Slice;
extern TC_TCB         *TMD_Time_Slice_Task;
extern INT             TMD_Time_Slice_State;
extern TC_HCB          TMD_HISR;
extern VOID           *TMD_HISR_Stack_Ptr;
extern UNSIGNED        TMD_HISR_Stack_Size;
extern INT             TMD_HISR_Priority;


/* Define inner-component function prototypes.  */

VOID    TMC_Timer_HISR(VOID);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMI_Initialize                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the data structures that control the   */
/*      operation of the timer component (TM).  There are no application */
/*      timers created initially.  This routine must be called from      */
/*      Supervisor mode in Supervisor/User mode switching kernels.       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      INC_Initialize                      System initialization        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      ERC_System_Error                    System error handing function*/
/*      TCC_Create_HISR                     Create timer HISR            */
/*      TCCE_Create_HISR                    Create timer HISR (error chk)*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      (Indirect)                                                       */
/*      TMD_HISR_Stack_Ptr                  Pointer to HISR stack area   */
/*      TMD_HISR_Stack_Size                 Size of HISR stack           */
/*      TMD_HISR_Priority                   Priority of timer HISR       */
/*                                                                       */
/* OUTPUTS                                                               */
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
/*      TMD_Timer                           Timer count-down value       */
/*      TMD_Timer_State                     State of timer               */
/*      TMD_Time_Slice                      Time slice count-down value  */
/*      TMD_Time_Slice_Task                 Pointer to task to time-slice*/
/*      TMD_Time_Slice_State                State of time slice          */
/*      TMD_HISR                            Timer HISR control block     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Removed timer task creation,                     */
/*                        cleared the ID of the HISR                     */
/*                        control block, and casted the                  */
/*                        HISR priority, resulting in                    */
/*                        version 1.0a                                   */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Modified HISR create function                    */
/*                        interface, removed reference                   */
/*                        to timer list protect since it                 */
/*                        was deleted, resulting in                      */
/*                        version 1.1                                    */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  TMI_Initialize(VOID)
{

STATUS          status;                     /* Return status of creates */


    /* Initialize the created application timer's list to NU_NULL.  */
    TMD_Created_Timers_List =  NU_NULL;

    /* Initialize the total number of created application timers to 0.  */
    TMD_Total_Timers =  0;

    /* Initialize the active timer's list to NU_NULL.  */
    TMD_Active_Timers_List =  NU_NULL;

    /* Clear the active timer list busy flag.  */
    TMD_Active_List_Busy =  NU_FALSE;

    /* Initialize the system clock to 0.  */
    TMD_System_Clock =  0;

    /* Initialize the timer to 0 and the timer state to "not active."  */
    TMD_Timer =        0;
    TMD_Timer_State =  TM_NOT_ACTIVE;

    /* Initialize the time-slice timer, task pointer, and the associated
       state variable.  */
    TMD_Time_Slice =        0;
    TMD_Time_Slice_Task =   NU_NULL;
    TMD_Time_Slice_State =  TM_NOT_ACTIVE;

    /* Initialize the list protection structures.  */
    TMD_Created_List_Protect.tc_tcb_pointer =      NU_NULL;

    /* Clear out the timer HISR control block.  */
    TMD_HISR.tc_id =  0;

    /* Create the timer HISR.  The timer HISR is responsible for performing
       the time slice function and activating the timer task if necessary.  */
#ifdef  NU_NO_ERROR_CHECKING

    status =  TCC_Create_HISR((NU_HISR *) &TMD_HISR, "SYSTEM H",
                    TMC_Timer_HISR, (OPTION) TMD_HISR_Priority,
                    TMD_HISR_Stack_Ptr, TMD_HISR_Stack_Size);
#else

    status =  TCCE_Create_HISR((NU_HISR *) &TMD_HISR, "SYSTEM H",
                    TMC_Timer_HISR, (OPTION) TMD_HISR_Priority,
                    TMD_HISR_Stack_Ptr, TMD_HISR_Stack_Size);

#endif

    /* Check for a system error creating the timer HISR.  */
    if (status != NU_SUCCESS)

        /* Call the system error handler.  */
        ERC_System_Error(NU_ERROR_CREATING_TIMER_HISR);
}





