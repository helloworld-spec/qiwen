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
/*      pise.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PI - Pipe Management                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains error checking routines for supplemental      */
/*      functions of the Pipe component.  This permits easy removal of   */
/*      error checking logic when it is not needed.                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      PISE_Reset_Pipe                     Reset a pipe                 */
/*      PISE_Send_To_Front_Of_Pipe          Send message to pipe's front */
/*      PISE_Broadcast_To_Pipe              Broadcast message to pipe    */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      pi_extr.h                           Pipe functions               */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1994      Created initial version 1.1 from                 */
/*                      routines originally in core                      */
/*                      error checking file                              */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "pi_extr.h"                 /* Pipe functions            */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PISE_Reset_Pipe                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameter supplied  */
/*      to the pipe reset function.                                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      PIS_Reset_Pipe                      Actual reset pipe function   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pipe_ptr                            Pipe control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_PIPE                     Invalid pipe pointer         */
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
STATUS  PISE_Reset_Pipe(NU_PIPE *pipe_ptr)
{

PI_PCB         *pipe;
STATUS          status;


    /* Move input pipe pointer into internal pointer.  */
    pipe =  (PI_PCB *) pipe_ptr;

    /* Determine if there is an error with the pipe pointer.  */
    if (pipe == NU_NULL)
    
        /* Indicate that the pipe pointer is invalid.  */
        status =  NU_INVALID_PIPE;
        
    else if (pipe -> pi_id != PI_PIPE_ID)
    
        /* Indicate that the pipe pointer is invalid.  */
        status =  NU_INVALID_PIPE;

    else
    
        /* All the parameters are okay, call the actual function to reset
           a pipe.  */
        status =  PIS_Reset_Pipe(pipe_ptr);
                                  
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PISE_Send_To_Front_Of_Pipe                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the send message to front of pipe function.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      PIS_Send_To_Front_Of_Pipe           Actual send to front of pipe */
/*                                            function                   */
/*      TCCE_Suspend_Error                  Check suspend validity       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pipe_ptr                            Pipe control block pointer   */
/*      message                             Pointer to message to send   */
/*      size                                Size of message to send      */
/*      suspend                             Suspension option if full    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_PIPE                     Invalid pipe pointer         */
/*      NU_INVALID_POINTER                  Invalid message pointer      */
/*      NU_INVALID_SIZE                     Invalid message size         */
/*      NU_INVALID_SUSPEND                  Invalid suspend request      */
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
STATUS  PISE_Send_To_Front_Of_Pipe(NU_PIPE *pipe_ptr, VOID *message, 
                                    UNSIGNED size, UNSIGNED suspend)
{

PI_PCB         *pipe;
STATUS          status;


    /* Move input pipe pointer into internal pointer.  */
    pipe =  (PI_PCB *) pipe_ptr;

    /* Determine if there is an error with the pipe pointer.  */
    if (pipe == NU_NULL)
    
        /* Indicate that the pipe pointer is invalid.  */
        status =  NU_INVALID_PIPE;
        
    else if (pipe -> pi_id != PI_PIPE_ID)
    
        /* Indicate that the pipe pointer is invalid.  */
        status =  NU_INVALID_PIPE;

    else if (message == NU_NULL)
    
        /* Indicate that the pointer to the message is invalid.  */
        status =  NU_INVALID_POINTER;

    else if (size == 0)

        /* Indicate that the message size is invalid   */
        status =  NU_INVALID_SIZE;

    else if ((pipe -> pi_fixed_size) && (size != pipe -> pi_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  NU_INVALID_SIZE;

    else if ((!pipe -> pi_fixed_size) && (size > pipe -> pi_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  NU_INVALID_SIZE;
        
    else if ((suspend) && (TCCE_Suspend_Error()))
    
        /* Indicate that suspension is only valid from a non-task thread.  */
        status =  NU_INVALID_SUSPEND;
    
    else
    
        /* All the parameters are okay, call the actual function to send
           a message to a pipe.  */
        status =  PIS_Send_To_Front_Of_Pipe(pipe_ptr, message, size, suspend);
                                  
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PISE_Broadcast_To_Pipe                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the broadcast message to pipe function.                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      PIS_Broadcast_To_Pipe               Actual broadcast message to  */
/*                                            pipe function              */
/*      TCCE_Suspend_Error                  Check suspend validity       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pipe_ptr                            Pipe control block pointer   */
/*      message                             Pointer to message to send   */
/*      size                                Size of message to send      */
/*      suspend                             Suspension option if full    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_PIPE                     Invalid pipe pointer         */
/*      NU_INVALID_POINTER                  Invalid message pointer      */
/*      NU_INVALID_SIZE                     Invalid message size         */
/*      NU_INVALID_SUSPEND                  Invalid suspend request      */
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
STATUS  PISE_Broadcast_To_Pipe(NU_PIPE *pipe_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend)
{

PI_PCB         *pipe;
STATUS          status;


    /* Move input pipe pointer into internal pointer.  */
    pipe =  (PI_PCB *) pipe_ptr;

    /* Determine if there is an error with the pipe pointer.  */
    if (pipe == NU_NULL)
    
        /* Indicate that the pipe pointer is invalid.  */
        status =  NU_INVALID_PIPE;
        
    else if (pipe -> pi_id != PI_PIPE_ID)
    
        /* Indicate that the pipe pointer is invalid.  */
        status =  NU_INVALID_PIPE;

    else if (message == NU_NULL)
    
        /* Indicate that the pointer to the message is invalid.  */
        status =  NU_INVALID_POINTER;

    else if (size == 0)

        /* Indicate that the message size is invalid   */
        status =  NU_INVALID_SIZE;

    else if ((pipe -> pi_fixed_size) && (size != pipe -> pi_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  NU_INVALID_SIZE;

    else if ((!pipe -> pi_fixed_size) && (size > pipe -> pi_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  NU_INVALID_SIZE;
        
    else if ((suspend) && (TCCE_Suspend_Error()))
    
        /* Indicate that suspension is only valid from a non-task thread.  */
        status =  NU_INVALID_SUSPEND;
    
    else
    
        /* All the parameters are okay, call the actual function to broadcast
           a message to a pipe.  */
        status =  PIS_Broadcast_To_Pipe(pipe_ptr, message, size, suspend);
                                  
    /* Return completion status.  */
    return(status);
}







