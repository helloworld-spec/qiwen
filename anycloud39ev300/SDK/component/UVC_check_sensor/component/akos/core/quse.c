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
/*      quse.c                                         Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      QU - Queue Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains error checking routines for supplemental      */
/*      functions in the Queue component.  This permits easy removal of  */
/*      error checking logic when it is not needed.                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      QUSE_Reset_Queue                    Reset a queue                */
/*      QUSE_Send_To_Front_Of_Queue         Send message to queue's front*/
/*      QUSE_Broadcast_To_Queue             Broadcast message to queue   */
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
/*      03-01-1994      Created initial version 1.1 from                 */
/*                      routines originally in core                      */
/*                      error checking file                              */
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
#include        "qu_extr.h"                 /* Queue functions           */



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUSE_Reset_Queue                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameter supplied  */
/*      to the queue reset function.                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      QUS_Reset_Queue                     Actual reset queue function  */
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
STATUS  QUSE_Reset_Queue(NU_QUEUE *queue_ptr)
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
    
        /* All the parameters are okay, call the actual function to reset
           a queue.  */
        status =  QUS_Reset_Queue(queue_ptr);
                                  
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUSE_Send_To_Front_Of_Queue                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the send message to front of queue function.                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      QUS_Send_To_Front_Of_Queue          Actual send to front of      */
/*                                            queue function             */
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
/*      NU_INVALID_SUSPEND                  Invalid suspension request   */
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
STATUS  QUSE_Send_To_Front_Of_Queue(NU_QUEUE *queue_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend)
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

        /* Indicate that the message size is invalid.   */
        status = NU_INVALID_SIZE;

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
        status =  QUS_Send_To_Front_Of_Queue(queue_ptr, message, size, 
                                                                    suspend);
                                  
    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUSE_Broadcast_To_Queue                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function performs error checking on the parameters supplied */
/*      to the broadcast message to queue function.                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      QUS_Broadcast_To_Queue              Actual broadcast message     */
/*                                            to queue function          */
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
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-1-1994       Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  QUSE_Broadcast_To_Queue(NU_QUEUE *queue_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend)
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

        /* Indicate that the message size is invalid.   */
        status = NU_INVALID_SIZE;

    
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
    
        /* Broadcast a message to a queue.  */
        status =  QUS_Broadcast_To_Queue(queue_ptr, message, size, suspend);
                                  
    /* Return completion status.  */
    return(status);
}




