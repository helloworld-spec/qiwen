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
/*      quce.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      QU - Queue Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains error checking routines for core functions    */
/*      of the Queue component.  This permits easy removal of error      */
/*      checking logic when it is not needed.                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      QUCE_Create_Queue                   Create a queue               */
/*      QUCE_Delete_Queue                   Delete a queue               */
/*      QUCE_Send_To_Queue                  Send a queue message         */
/*      QUCE_Receive_From_Queue             Receive a queue message      */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      qu_extr.h                           Queue functions              */
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
/*      04-17-1996      updated to version 1.2                           */
/*      10-28-1997      Modified QUCE_Receive_From_Queue                 */
/*                      to correct SPR142. This                          */
/*                      created version 1.2a.                            */
/*      03-24-1998      Released version 1.3.                            */
/*      06-04-1998      Modified QUCE_Send_To_Queue to                   */
/*                      check for a size of 0, created                   */
/*                      version 1.3a. (SPR493)                           */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "qu_extr.h"                 /* Queue functions           */


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUCE_Create_Queue                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the queue create function.                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      QUC_Create_Queue                    Actual create queue function */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      queue_ptr                           Queue control block pointer  */
/*      name                                Queue name                   */
/*      start_address                       Starting address of actual   */
/*                                            queue area                 */
/*      queue_size                          Total size of queue          */
/*      message_type                        Type of message supported by */
/*                                            the queue (fixed/variable) */
/*      message_size                        Size of message.  Variable   */
/*                                            message-length queues, this*/
/*                                            represents the maximum size*/
/*      suspend_type                        Suspension type              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_QUEUE                    Invalid queue pointer        */
/*      NU_INVALID_MEMORY                   Invalid queue starting addr  */
/*      NU_INVALID_SIZE                     Invalid queue size and/or    */
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
STATUS  QUCE_Create_Queue(NU_QUEUE *queue_ptr, CHAR *name, 
                      VOID *start_address, UNSIGNED queue_size, 
                      OPTION message_type, UNSIGNED message_size,
                      OPTION suspend_type)
{

QU_QCB         *queue;
STATUS          status;
INT             overhead;

    
    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;
    
    /* Determine if queue supports variable length messages.  If so, an
       additional word of overhead is required.  */
    if (message_type == NU_VARIABLE_SIZE)
    
        /* Variable-size queues require an additional word of overhead.  */
        overhead =  1;
    else
    
        /* Fixed-size message queues require no additional overhead.  */
        overhead =  0;

    /* Determine if there is an error with the queue pointer.  */
    if ((queue == NU_NULL) || (queue -> qu_id == QU_QUEUE_ID))
    
        /* Indicate that the queue pointer is invalid.  */
        status =  NU_INVALID_QUEUE;
        
    else if (start_address == NU_NULL)
    
        /* Indicate that the starting address of the queue is invalid.  */
        status =  NU_INVALID_MEMORY;
        
    else if ((queue_size == 0) || (message_size == 0) || 
                                ((message_size+overhead) > queue_size))
             
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
           a queue.  */
        status =  QUC_Create_Queue(queue_ptr, name, start_address, queue_size,
                                  message_type, message_size, suspend_type);
                                  
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUCE_Delete_Queue                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameter supplied  */
/*      to the queue delete function.                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      QUC_Delete_Queue                    Actual delete queue function */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      queue_ptr                           Queue control block pointer  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_QUEUE                    Invalid queue pointer        */
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
STATUS  QUCE_Delete_Queue(NU_QUEUE *queue_ptr)
{

QU_QCB         *queue;                      
STATUS          status;


    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Determine if there is an error with the queue pointer.  */
    if (queue == NU_NULL)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  NU_INVALID_QUEUE;
        
    else if (queue -> qu_id != QU_QUEUE_ID)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  NU_INVALID_QUEUE;

    else
    
        /* All the parameters are okay, call the actual function to delete
           a queue.  */
        status =  QUC_Delete_Queue(queue_ptr);
                                  
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUCE_Send_To_Queue                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the send message to queue function.                           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      QUC_Send_To_Queue                   Actual send queue message    */
/*                                            function                   */
/*      TCCE_Suspend_Error                  Check suspend validity       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      queue_ptr                           Queue control block pointer  */
/*      message                             Pointer to message to send   */
/*      size                                Size of message to send      */
/*      suspend                             Suspension option if full    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_QUEUE                    Invalid queue pointer        */
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
/*      06-04-1998      Corrected SPR493                                 */
/*                                                                       */
/*************************************************************************/
STATUS  QUCE_Send_To_Queue(NU_QUEUE *queue_ptr, VOID *message, UNSIGNED size, 
                                                        UNSIGNED suspend)
{

QU_QCB         *queue;
STATUS          status;


    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Determine if there is an error with the queue pointer.  */
    if (queue == NU_NULL)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  NU_INVALID_QUEUE;
        
    else if (queue -> qu_id != QU_QUEUE_ID)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  NU_INVALID_QUEUE;

    else if (message == NU_NULL)
    
        /* Indicate that the pointer to the message is invalid.  */
        status =  NU_INVALID_POINTER;

    else if (size == 0)
    
        /* Indicate that the message size is invalid.  */
        status =  NU_INVALID_SIZE;

    else if ((queue -> qu_fixed_size) && (size != queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  NU_INVALID_SIZE;

    else if ((!queue -> qu_fixed_size) && (size > queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  NU_INVALID_SIZE;
        
    else if ((suspend) && (TCCE_Suspend_Error()))
    
        /* Indicate that the suspension is only allowed from a task thread. */
        status =  NU_INVALID_SUSPEND;

    else
    
        /* All the parameters are okay, call the actual function to send
           a message to a queue.  */
        status =  QUC_Send_To_Queue(queue_ptr, message, size, suspend);
                              
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUCE_Receive_From_Queue                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the receive message from queue function.                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      QUC_Receive_From_Queue              Actual receive message from  */
/*                                            queue                      */
/*      TCCE_Suspend_Error                  Check suspend validity       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      queue_ptr                           Queue control block pointer  */
/*      message                             Pointer to message to send   */
/*      size                                Size of the message          */
/*      actual_size                         Size of message received     */
/*      suspend                             Suspension option if empty   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_INVALID_QUEUE                    Invalid queue pointer        */
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
/*      05-24-1996      Changed the variable queue check                 */
/*                      from "message size not equal                     */
/*                      to pipe message size" to                         */
/*                      "message size greater than                       */
/*                      pipe message size" (SPR142).                     */
/*                                                                       */
/*************************************************************************/
STATUS  QUCE_Receive_From_Queue(NU_QUEUE *queue_ptr, VOID *message,
                UNSIGNED size, UNSIGNED *actual_size, UNSIGNED suspend)
{

QU_QCB         *queue;
STATUS          status;


    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;
    
    /* Determine if there is an error with the queue pointer.  */
    if (queue == NU_NULL)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  NU_INVALID_QUEUE;
        
    else if (queue -> qu_id != QU_QUEUE_ID)
    
        /* Indicate that the queue pointer is invalid.  */
        status =  NU_INVALID_QUEUE;

    else if (message == NU_NULL)
    
        /* Indicate that the pointer to the message is invalid.  */
        status =  NU_INVALID_POINTER;

    else if (size == 0)
    
        /* Indicate that the message size is invalid.  */
        status =  NU_INVALID_SIZE;

    else if ((queue -> qu_fixed_size) && (size != queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  NU_INVALID_SIZE;

    else if ((!queue -> qu_fixed_size) && (size > queue -> qu_message_size))
    
        /* Indicate that the message size is invalid.  */
        status =  NU_INVALID_SIZE;
        
    else if ((suspend) && (TCCE_Suspend_Error()))
    
        /* Indicate that the suspension is only allowed from a task thread. */
        status =  NU_INVALID_SUSPEND;

    else
    
        /* All the parameters are okay, call the actual function to receive
           a message from a queue.  */
        status =  QUC_Receive_From_Queue(queue_ptr, message, size, 
                                                actual_size, suspend);
                                  
    /* Return completion status.  */
    return(status);
}







