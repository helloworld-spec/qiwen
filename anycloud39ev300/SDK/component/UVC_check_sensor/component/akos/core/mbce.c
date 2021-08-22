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
/*      mbce.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      MB - Mailbox Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the error checking routines for the core      */
/*      functions in the Mailbox component.  This permits easy removal   */
/*      of error checking logic when it is not needed.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      MBCE_Create_Mailbox                 Create a mailbox             */
/*      MBCE_Delete_Mailbox                 Delete a mailbox             */
/*      MBCE_Send_To_Mailbox                Send a mailbox message       */
/*      MBCE_Receive_From_Mailbox           Receive a mailbox message    */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      mb_extr.h                           Mailbox functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Created new file that contains                   */
/*                      error checking shells for core                   */
/*                      mailbox functions, also                          */
/*                      changed interface to exactly                     */
/*                      match prototype, resulting in                    */
/*                      version 1.1                                      */
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
#include        "mb_extr.h"                 /* Mailbox functions         */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBCE_Create_Mailbox                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the mailbox create function.                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      MBC_Create_Mailbox                  Actual create mailbox funct. */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      mailbox_ptr                         Mailbox control block pointer*/
/*      name                                Mailbox name                 */
/*      suspend_type                        Suspension type              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_MAILBOX                  Mailbox pointer is NULL      */
/*      NU_INVALID_SUSPEND                  Suspension type is invalid   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified interface to exactly                    */
/*                      match prototype, resulting in                    */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  MBCE_Create_Mailbox(NU_MAILBOX *mailbox_ptr, CHAR *name, 
                                                OPTION suspend_type)
{

MB_MCB         *mailbox;                    /* Mailbox control block ptr */
STATUS          status;                     /* Completion status         */


    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Check for a NULL mailbox pointer or an already created mailbox.  */
    if ((mailbox == NU_NULL) || (mailbox -> mb_id == MB_MAILBOX_ID))
    
        /* Invalid mailbox control block pointer.  */
        status =  NU_INVALID_MAILBOX;
        
    else if ((suspend_type != NU_FIFO) && (suspend_type != NU_PRIORITY))
    
        /* Invalid suspension type.  */
        status =  NU_INVALID_SUSPEND;
        
    else
    
        /* Call the actual service to create the mailbox.  */
        status =  MBC_Create_Mailbox(mailbox_ptr, name, suspend_type);
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBCE_Delete_Mailbox                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the actual delete mailbox function.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      MBC_Delete_Mailbox                  Actual delete mailbox funct. */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      mailbox_ptr                         Mailbox control block pointer*/
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_MAILBOX                  Invalid mailbox supplied     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified interface to exactly                    */
/*                      match prototype, resulting in                    */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  MBCE_Delete_Mailbox(NU_MAILBOX *mailbox_ptr)
{

MB_MCB         *mailbox;                    /* Mailbox control block ptr */
STATUS          status;                     /* Completion status         */

    
    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Determine if the mailbox pointer is valid.  */
    if ((mailbox) && (mailbox -> mb_id == MB_MAILBOX_ID))
    
        /* Mailbox pointer is valid, call function to delete it.  */
        status =  MBC_Delete_Mailbox(mailbox_ptr);
        
    else
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_MAILBOX;
        
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBCE_Send_To_Mailbox                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the send-to-mailbox function.                                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      MBC_Sent_To_Mailbox                 Actual mailbox send function */
/*      TCCE_Suspend_Error                  Check suspend validity       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      mailbox_ptr                         Mailbox control block pointer*/
/*      message                             Pointer to message to send   */
/*      suspend                             Suspension option if full    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_MAILBOX                  Mailbox pointer is invalid   */
/*      NU_INVALID_POINTER                  Message pointer is invalid   */
/*      NU_INVALID_SUSPEND                  Invalid suspend request      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified interface to exactly                    */
/*                      match prototype, resulting in                    */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  MBCE_Send_To_Mailbox(NU_MAILBOX *mailbox_ptr, VOID *message, 
                                                          UNSIGNED suspend)
{

MB_MCB         *mailbox;                    /* Mailbox control block ptr */
STATUS          status;                     /* Completion status         */


    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Determine if mailbox pointer is invalid.  */
    if (mailbox == NU_NULL)
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_MAILBOX;

    else if (mailbox -> mb_id != MB_MAILBOX_ID)
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_MAILBOX;

    else if (message == NU_NULL)
    
        /* Message pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_POINTER;

    else if ((suspend) && (TCCE_Suspend_Error()))
    
        /* Indicate that suspension is only valid from a task thread.  */
        status =  NU_INVALID_SUSPEND;

    else
    
        /* Parameters are valid, call actual function.  */
        status =  MBC_Send_To_Mailbox(mailbox_ptr, message, suspend);

    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBCE_Receive_From_Mailbox                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the receive message from mailbox function.                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      MBC_Receive_From_Mailbox            Actual receive function      */
/*      TCCE_Suspend_Error                  Check suspend validity       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      mailbox_ptr                         Mailbox control block pointer*/
/*      message                             Pointer to message to send   */
/*      suspend                             Suspension option if empty   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_MAILBOX                  Mailbox pointer is invalid   */
/*      NU_INVALID_POINTER                  Message pointer is invalid   */
/*      NU_INVALID_SUSPEND                  Invalid suspend request      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified interface to exactly                    */
/*                      match prototype, resulting in                    */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  MBCE_Receive_From_Mailbox(NU_MAILBOX *mailbox_ptr, VOID *message, 
                                                        UNSIGNED suspend)
{

MB_MCB         *mailbox;                    /* Mailbox control block ptr */
STATUS          status;                     /* Completion status         */


    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;

    /* Determine if mailbox pointer is invalid.  */
    if (mailbox == NU_NULL)
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_MAILBOX;

    else if (mailbox -> mb_id != MB_MAILBOX_ID)
    
        /* Mailbox pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_MAILBOX;

    else if (message == NU_NULL)
    
        /* Message pointer is invalid, indicate in completion status.  */
        status =  NU_INVALID_POINTER;

    else if ((suspend) && (TCCE_Suspend_Error()))
    
        /* Indicate that suspension is only valid from a task thread.  */
        status =  NU_INVALID_SUSPEND;

    else
    
        /* Parameters are valid, call actual function.  */
        status =  MBC_Receive_From_Mailbox(mailbox_ptr, message, suspend);

    /* Return the completion status.  */
    return(status);
}






