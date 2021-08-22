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
/*      quf.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      QU - Queue Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains routines to obtain facts about the Queue      */
/*      management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      QUF_Established_Queues              Number of created queues     */
/*      QUF_Queue_Information               Retrieve queue information   */
/*      QUF_Queue_Pointers                  Build queue pointer list     */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      qu_extr.h                           Queue functions              */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1994      Initial version of queue fact                    */
/*                        service file, version 1.1                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      11-18-1996      Protected Informational service                  */
/*                      from NULL Control Block pointers                 */
/*                      creating 1.2a. (SPR220)                          */
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "qu_extr.h"                 /* Queue functions           */
#include        "hi_extr.h"                 /* History functions         */


/* Define external inner-component global data references.  */

extern CS_NODE         *QUD_Created_Queues_List;
extern UNSIGNED         QUD_Total_Queues;
extern TC_PROTECT       QUD_List_Protect;



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUF_Established_Queues                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the current number of established          */
/*      queues.  Queues previously deleted are no longer considered      */
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
/*      QUD_Total_Queues                    Number of established        */
/*                                            queues                     */
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
UNSIGNED  QUF_Established_Queues(VOID)
{


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Return the number of established queues.  */
    return(QUD_Total_Queues);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUF_Queue_Pointers                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function builds a list of queue pointers, starting at the   */
/*      specified location.  The number of queue pointers placed in the  */
/*      list is equivalent to the total number of queues or the maximum  */
/*      number of pointers specified in the call.                        */
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
/*      pointers                            Number of queue pointers     */
/*                                            placed in the list         */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Corrected pointer retrieval                      */
/*                      loop, resulting in version 1.0a                  */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  QUF_Queue_Pointers(NU_QUEUE **pointer_list,UNSIGNED maximum_pointers)
{
CS_NODE         *node_ptr;                  /* Pointer to each QCB       */
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

    /* Protect against access to the list of created queues.  */
    TCT_Protect(&QUD_List_Protect);

    /* Loop until all queue pointers are in the list or until the maximum
       list size is reached.  */
    node_ptr =  QUD_Created_Queues_List;
    while ((node_ptr) && (pointers < maximum_pointers))
    {

        /* Place the node into the destination list.  */
        *pointer_list++ =  (NU_QUEUE *) node_ptr;

        /* Increment the pointers variable.  */
        pointers++;

        /* Position the node pointer to the next node.  */
        node_ptr =  node_ptr -> cs_next;

        /* Determine if the pointer is at the head of the list.  */
        if (node_ptr == QUD_Created_Queues_List)

            /* The list search is complete.  */
            node_ptr =  NU_NULL;
    }

    /* Release protection against access to the list of created queues.  */
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
/*      QUF_Queue_Information                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns information about the specified queue.     */
/*      However, if the supplied queue pointer is invalid, the           */
/*      function simply returns an error status.                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect queue                */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      queue_ptr                           Pointer to the queue         */
/*      name                                Destination for the name     */
/*      start_address                       Destination for the start    */
/*                                            address of the queue       */
/*      queue_size                          Destination for queue size   */
/*      available                           Destination for available    */
/*                                            room in queue              */
/*      messages                            Destination for number of    */
/*                                            messages queued            */
/*      message_type                        Destination for message type */
/*      message_size                        Destination for message size */
/*      suspend_type                        Destination for suspension   */
/*                                            type                       */
/*      tasks_waiting                       Destination for the tasks    */
/*                                            waiting count              */
/*      first_task                          Destination for the pointer  */
/*                                            to the first task waiting  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If a valid queue pointer     */
/*                                            is supplied                */
/*      NU_INVALID_QUEUE                    If queue pointer invalid     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      changed protection logic,                        */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      11-18-1996      Corrected SPR220.                                */
/*                                                                       */
/*************************************************************************/
STATUS  QUF_Queue_Information(NU_QUEUE *queue_ptr, CHAR *name,
                  VOID **start_address, UNSIGNED *queue_size,
                  UNSIGNED *available, UNSIGNED *messages,
                  OPTION *message_type, UNSIGNED *message_size,
                  OPTION *suspend_type, UNSIGNED *tasks_waiting,
                  NU_TASK **first_task)
{

QU_QCB         *queue;                      /* Queue control block ptr   */
INT             i;                          /* Working integer variable  */
STATUS          completion;                 /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Determine if this queue id is valid.  */
    if ((queue != NU_NULL) && (queue -> qu_id == QU_QUEUE_ID))
    {

        /* Setup protection of the queue.  */
        TCT_System_Protect();

        /* The queue pointer is valid.  Reflect this in the completion
           status and fill in the actual information.  */
        completion =  NU_SUCCESS;

        /* Copy the queue's name.  */
        for (i = 0; i < NU_MAX_NAME; i++)
            *name++ =  queue -> qu_name[i];

        /* Determine the suspension type.  */
        if (queue -> qu_fifo_suspend)
            *suspend_type =  NU_FIFO;
        else
            *suspend_type =  NU_PRIORITY;

        /* Determine the message type.  */
        if (queue -> qu_fixed_size)
            *message_type =  NU_FIXED_SIZE;
        else
            *message_type =  NU_VARIABLE_SIZE;

        /* Get various information about the queue.  */
        *start_address =  (UNSIGNED *) queue -> qu_start;
        *queue_size =     queue -> qu_queue_size;
        *available =      queue -> qu_available;
        *messages =       queue -> qu_messages;
        *message_size =   queue -> qu_message_size;

        /* Retrieve the number of tasks waiting and the pointer to the
           first task waiting.  */
        *tasks_waiting =  queue -> qu_tasks_waiting;
        if (queue -> qu_suspension_list)

            /* There is a task waiting.  */
            *first_task =  (NU_TASK *)
                (queue -> qu_suspension_list) -> qu_suspended_task;
        else

            /* There are no tasks waiting.  */
            *first_task =  NU_NULL;

        /* Release protection of the queue.  */
        TCT_Unprotect();
    }
    else

        /* Indicate that the queue pointer is invalid.   */
        completion =  NU_INVALID_QUEUE;

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the appropriate completion status.  */
    return(completion);
}




