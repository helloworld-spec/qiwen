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
/*      tmse.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TM - Timer Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the error checking routines for the functions */
/*      in the Timer component.  This permits easy removal of error      */
/*      checking logic when it is not needed.                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      TMSE_Create_Timer                   Create an application timer  */
/*      TMSE_Delete_Timer                   Delete an application timer  */
/*      TMSE_Reset_Timer                    Reset application timer      */
/*      TMSE_Control_Timer                  Enable/Disable application   */
/*                                            timer                      */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tm_extr.h                           Timer functions              */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed names of error checking                  */
/*                      shell to match new conventions,                  */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tm_extr.h"                 /* Timer functions           */



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMSE_Create_Timer                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the create timer function.                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TMS_Create_Timer                    Actual create timer function */
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
/*      NU_INVALID_TIMER                    Indicates timer pointer is   */
/*                                            NULL                       */
/*      NU_INVALID_FUNCTION                 Indicates timer expiration   */
/*                                            function pointer is NULL   */
/*      NU_INVALID_ENABLE                   Indicates enable parameter   */
/*                                            is invalid                 */
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
STATUS  TMSE_Create_Timer(NU_TIMER *timer_ptr, CHAR *name, 
                VOID (*expiration_routine)(UNSIGNED), UNSIGNED id,
                UNSIGNED initial_time, UNSIGNED reschedule_time, OPTION enable)
{

TM_APP_TCB     *timer;                      /* Timer control block ptr  */
STATUS          status;                     /* Completion status        */


    /* Move input timer pointer into internal pointer.  */
    timer =  (TM_APP_TCB *) timer_ptr;

    /* Check the parameters to the create timer function.  */
    if ((timer == NU_NULL) || (timer -> tm_id == TM_TIMER_ID))
    
        /* Invalid timer pointer.  */
        status =  NU_INVALID_TIMER;
    
    else if (expiration_routine == NU_NULL)
    
        /* Invalid expiration function pointer.  */
        status =  NU_INVALID_FUNCTION;

    else if (initial_time == 0)
    
        /* Invalid time value.  */
        status =  NU_INVALID_OPERATION;

        
    else if ((enable != NU_ENABLE_TIMER) && (enable != NU_DISABLE_TIMER))
    
        /* Invalid enable parameter.  */
        status =  NU_INVALID_ENABLE;
        
    else
    
        /* Call the actual create timer function.  */
        status =  TMS_Create_Timer(timer_ptr, name, expiration_routine, id,
                                      initial_time, reschedule_time, enable);

    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMSE_Delete_Timer                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the delete timer function.                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TMS_Delete_Timer                    Actual delete timer function */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      timer_ptr                           Timer control block pointer  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_TIMER                    Indicates the timer pointer  */
/*                                            is NULL or not a timer     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interface,                      */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TMSE_Delete_Timer(NU_TIMER *timer_ptr)
{

TM_APP_TCB     *timer;                      /* Timer control block ptr  */
STATUS          status;                     /* Completion status        */

    
    /* Move input timer pointer into internal pointer.  */
    timer =  (TM_APP_TCB *) timer_ptr;

    /* Check the parameters to the delete timer function.  */
    if (timer == NU_NULL)
    
        /* Invalid timer pointer.  */
        status =  NU_INVALID_TIMER;
    
    else if (timer -> tm_id != TM_TIMER_ID)
    
        /* Invalid timer pointer.  */
        status =  NU_INVALID_TIMER;
    
    else
    
        /* Call the actual delete timer function.  */
        status =  TMS_Delete_Timer(timer_ptr);

    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMSE_Reset_Timer                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the reset timer function.                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TMS_Reset_Timer                     Actual reset timer function  */
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
/*      NU_INVALID_TIMER                    Indicates timer pointer is   */
/*                                            invalid                    */
/*      NU_INVALID_FUNCTION                 Indicates that expiration    */
/*                                            function pointer is NULL   */
/*      NU_INVALID_ENABLE                   Indicates enable parameter   */
/*                                            is invalid                 */
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
STATUS  TMSE_Reset_Timer(NU_TIMER *timer_ptr,  
                VOID (*expiration_routine)(UNSIGNED), 
                UNSIGNED initial_time, UNSIGNED reschedule_time, OPTION enable)
{

TM_APP_TCB     *timer;                      /* Timer contorl block ptr  */
STATUS          status;                     /* Completion status        */


    /* Move input timer pointer into internal pointer.  */
    timer =  (TM_APP_TCB *) timer_ptr;

    /* Check the parameters to the reset timer function.  */
    if (timer == NU_NULL)
    
        /* Invalid timer pointer.  */
        status =  NU_INVALID_TIMER;
    
    else if (timer -> tm_id != TM_TIMER_ID)
    
        /* Invalid timer pointer.  */
        status =  NU_INVALID_TIMER;

    else if (initial_time == 0)
    
        /* Invalid time value. */
        status =  NU_INVALID_OPERATION;


    else if (expiration_routine == NU_NULL)
    
        /* Invalid expiration function pointer.  */
        status =  NU_INVALID_FUNCTION;
        
    else if ((enable != NU_ENABLE_TIMER) && (enable != NU_DISABLE_TIMER))
    
        /* Invalid enable parameter.  */
        status =  NU_INVALID_ENABLE;
        
    else

        /* Call the actual reset timer function.  */
        status =  TMS_Reset_Timer(timer_ptr, expiration_routine, initial_time,
                                                   reschedule_time, enable);

    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TMSE_Control_Timer                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the control timer function.                                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TMS_Control_Timer                   Actual control timer function*/
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      timer_ptr                           Timer control block pointer  */
/*      enable                              Disable/enable timer option  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_TIMER                    Indicates the timer pointer  */
/*                                            is invalid                 */
/*      NU_INVALID_ENABLE                   Indicates enable parameter   */
/*                                            is invalid                 */
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
STATUS  TMSE_Control_Timer(NU_TIMER *timer_ptr, OPTION enable)
{

TM_APP_TCB     *timer;                      /* Timer control block ptr  */
STATUS          status;                     /* Completion status        */


    /* Move input timer pointer to internal pointer.  */
    timer =  (TM_APP_TCB *) timer_ptr;

    /* Check the parameters to the reset timer function.  */
    if (timer == NU_NULL)
    
        /* Invalid timer pointer.  */
        status =  NU_INVALID_TIMER;
    
    else if (timer -> tm_id != TM_TIMER_ID)
    
        /* Invalid timer pointer.  */
        status =  NU_INVALID_TIMER;

    else if ((enable != NU_ENABLE_TIMER) && (enable != NU_DISABLE_TIMER))
    
        /* Invalid enable parameter.  */
        status =  NU_INVALID_ENABLE;
        
    else

        /* Call actual control timer function.  */
        status =  TMS_Control_Timer(timer_ptr, enable);
        
    /* Return completion status.  */
    return(status);
}





