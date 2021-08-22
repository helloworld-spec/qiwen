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
/*      pice.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PI - Pipe Management                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains error checking routines for core functions    */
/*      of the Pipe component.  This permits easy removal of error       */
/*      checking logic when it is not needed.                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      PICE_Create_Pipe                    Create a pipe                */
/*      PICE_Delete_Pipe                    Delete a pipe                */
/*      PICE_Send_To_Pipe                   Send a pipe message          */
/*      PICE_Receive_From_Pipe              Receive a pipe message       */
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
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Split original error checking                    */
/*                      file and changed function                        */
/*                      interfaces, resulting in                         */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      12-19-1995      Modified PICE_Receive_From_Pipe,                 */
/*                      resulting in version 1.1+                        */
/*                      (spr065)                                         */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      06-04-1998      Modified PICE_Send_To_Pipe to                    */
/*                      check for a size of 0, created                   */
/*                      version 1.3a. (SPR493)                           */
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
/*      PICE_Create_Pipe                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the pipe create function.                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      PIC_Create_Pipe                     Actual create pipe function  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pipe_ptr                            Pipe control block pointer   */
/*      name                                Pipe name                    */
/*      start_address                       Starting address of actual   */
/*                                            pipe area                  */
/*      pipe_size                           Total size of pipe           */
/*      message_type                        Type of message supported by */
/*                                            the pipe (fixed/variable)  */
/*      message_size                        Size of message.  Variable   */
/*                                            message-length queues, this*/
/*                                            represents the maximum size*/
/*      suspend_type                        Suspension type              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_PIPE                     Invalid pipe pointer         */
/*      NU_INVALID_MEMORY                   Invalid pipe starting addr   */
/*      NU_INVALID_SIZE                     Invalid pipe size and/or     */
/*                                            size of message            */
/*      NU_INVALID_MESSAGE                  Invalid message type         */
/*      NU_INVALID_SUSPEND                  Invalid suspend type         */
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
STATUS  PICE_Create_Pipe(NU_PIPE *pipe_ptr, CHAR *name, 
                      VOID *start_address, UNSIGNED pipe_size, 
                      OPTION message_type, UNSIGNED message_size,
                      OPTION suspend_type)
{

PI_PCB         *pipe;
STATUS          status;
UNSIGNED        overhead;


    /* Move input pipe pointer into internal pointer.  */
    pipe =  (PI_PCB *) pipe_ptr;


    /* Determine if pipe supports variable length messages.  If so, 
           additional bytes of overhead are required.  */
    if (message_type == NU_VARIABLE_SIZE)

        /* Calculate the number of overhead bytes necessary for the additional
           word of overhead and the pad-bytes required to keep the pipe
           write pointer on an UNSIGNED data element alignment.  */
        overhead =  sizeof(UNSIGNED) +
                (((message_size + sizeof(UNSIGNED) - 1)/sizeof(UNSIGNED)) * 
                        sizeof(UNSIGNED)) - message_size;

    else
    
        /* Fixed-size message queues require no additional overhead.  */
        overhead =  0;


        /* Determine if there is an error with the pipe pointer.  */
    if ((pipe == NU_NULL) || (pipe -> pi_id == PI_PIPE_ID))
    
        /* Indicate that the pipe pointer is invalid.  */
        status =  NU_INVALID_PIPE;
        
    else if (start_address == NU_NULL)
    
        /* Indicate that the starting address of the pipe is invalid.  */
        status =  NU_INVALID_MEMORY;
        
    else if ((pipe_size == 0) || (message_size == 0) || 
                                ((message_size + overhead) > pipe_size))
             
        /* Indicate that one or both of the size parameters are invalid.  */
        status =  NU_INVALID_SIZE;
        
    else if ((message_type != NU_FIXED_SIZE) && 
                        (message_type != NU_VARIABLE_SIZE))
                                
        /* Indicate that the message type is invalid.  */
        status =  NU_INVALID_MESSAGE;
        
    else if ((suspend_type != NU_FIFO) && (suspend_type != NU_PRIORITY))
    
        /* Indicate that the suspend type is invalid.  */
        status =  NU_INVALID_SUSPEND;
        
    else
    
        /* All the parameters are okay, call the actual function to create
           a pipe.  */
        status =  PIC_Create_Pipe(pipe_ptr, name, start_address, pipe_size, 
                                  message_type, message_size, suspend_type);
                                  
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PICE_Delete_Pipe                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameter supplied  */
/*      to the pipe delete function.                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      PIC_Delete_Pipe                     Actual delete pipe function  */
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
STATUS  PICE_Delete_Pipe(NU_PIPE *pipe_ptr)
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
    
        /* All the parameters are okay, call the actual function to delete
           a pipe.  */
        status =  PIC_Delete_Pipe(pipe_ptr);
                                  
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PICE_Send_To_Pipe                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the send message to pipe function.                            */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      PIC_Send_To_Pipe                    Actual send pipe message     */
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
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */ 
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      06-04-1998      Corrected SPR493                                 */
/*                                                                       */
/*************************************************************************/
STATUS  PICE_Send_To_Pipe(NU_PIPE *pipe_ptr, VOID *message, UNSIGNED size, 
                                                        UNSIGNED suspend)
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
    
        /* Indicate that the message size is invalid.  */
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
        status =  PIC_Send_To_Pipe(pipe_ptr, message, size, suspend);
                                  
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PICE_Receive_From_Pipe                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the receive message from pipe function.                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      PIC_Receive_From_Pipe               Actual receive message from  */
/*                                            pipe                       */
/*      TCCE_Suspend_Error                  Check suspend validity       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pipe_ptr                            Pipe control block pointer   */
/*      message                             Pointer to message to send   */
/*      size                                Size of the message          */
/*      actual_size                         Size of message received     */
/*      suspend                             Suspension option if empty   */
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
/*      12-19-1995      Changed the variable pipe check                  */
/*                      from "message size not equal                     */
/*                      to pipe message size" to                         */
/*                      "message size greater than                       */
/*                      pipe message size",resulting                     */
/*                      in version 1.1+ (spr065)                         */
/*                                                                       */
/*************************************************************************/
STATUS  PICE_Receive_From_Pipe(NU_PIPE *pipe_ptr, VOID *message,
                UNSIGNED size, UNSIGNED *actual_size, UNSIGNED suspend)
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
    
        /* Indicate that the message size is invalid.  */
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
    
        /* All the parameters are okay, call the actual function to receive
           a message from a pipe.  */
        status =  PIC_Receive_From_Pipe(pipe_ptr, message, size, actual_size, 
                                                                suspend);
                                  
    /* Return completion status.  */
    return(status);
}







