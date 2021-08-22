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
/*      quc.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      QU - Queue Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core routines for the Queue management    */
/*      component.                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      QUC_Create_Queue                    Create a message queue       */
/*      QUC_Delete_Queue                    Delete a message queue       */
/*      QUC_Send_To_Queue                   Send message to a queue      */
/*      QUC_Receive_From_Queue              Receive a message from queue */
/*      QUC_Cleanup                         Cleanup on timeout or a      */
/*                                            terminate condition        */
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
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Corrected pointer retrieval                      */
/*                      loop, resulting in version 1.0a                  */
/*      08-09-1993      Verified version 1.0a                            */
/*      11-01-1993      Corrected a problem with fixed-                  */
/*                      size queues of a size equal to                   */
/*                      one message, resulting in                        */
/*                      version 1.0b                                     */
/*      11-01-1993      Verified version 1.0b                            */
/*      03-01-1994      Moved non-core functions into                    */
/*                      supplemental files, changed                      */
/*                      function interfaces to match                     */
/*                      those in prototype, added                        */
/*                      register options, changed                        */
/*                      protection logic to reduce                       */
/*                      overhead, corrected bug in                       */
/*                      queue reset, optimized item                      */
/*                      copy loops, resulting in                         */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      01-28-1998      Corrected SPR412 resulting in                    */
/*                      version 1.2a.                                    */
/*      03-24-1998      Released version 1.3                             */
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


/* Define internal component function prototypes.  */

VOID    QUC_Cleanup(VOID *information);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUC_Create_Queue                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a queue and then places it on the list     */
/*      of created queues.                                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      QUCE_Create_Queue                   Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Add node to linked-list      */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created list         */
/*      TCT_Unprotect                       Un-protect data structure    */
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
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options,                          */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  QUC_Create_Queue(NU_QUEUE *queue_ptr, CHAR *name,
                      VOID *start_address, UNSIGNED queue_size,
                      OPTION message_type, UNSIGNED message_size,
                      OPTION suspend_type)
{

R1 QU_QCB      *queue;                      /* Queue control block ptr   */
INT             i;                          /* Working index variable    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CREATE_QUEUE_ID, (UNSIGNED) queue,
                                (UNSIGNED) name, (UNSIGNED) start_address);

#endif

    /* First, clear the queue ID just in case it is an old Queue
       Control Block.  */
    queue -> qu_id =             0;

    /* Fill in the queue name.  */
    for (i = 0; i < NU_MAX_NAME; i++)
        queue -> qu_name[i] =  name[i];

    /* Setup the queue suspension type.  */
    if (suspend_type == NU_FIFO)

        /* FIFO suspension is selected, setup the flag accordingly.  */
        queue -> qu_fifo_suspend =  NU_TRUE;

    else

        /* Priority suspension is selected.  */
        queue -> qu_fifo_suspend =  NU_FALSE;

    /* Setup the queue message type.  */
    if (message_type == NU_FIXED_SIZE)

        /* Fixed-size messages are required.  */
        queue -> qu_fixed_size =  NU_TRUE;
    else

        /* Variable-size messages are required.  */
        queue -> qu_fixed_size =  NU_FALSE;

    /* Setup the message size.  */
    queue -> qu_message_size =  message_size;

    /* Clear the messages counter.   */
    queue -> qu_messages =  0;

    /* Setup the actual queue parameters.  */
    queue -> qu_queue_size =    queue_size;

    /* If the queue supports fixed-size messages, make sure that the queue
       size is an even multiple of the message size.  */
    if (queue -> qu_fixed_size)

        /* Adjust the area of the queue being used.  */
        queue_size =  (queue_size / message_size) * message_size;

    queue -> qu_available =     queue_size;
    queue -> qu_start =         (UNSIGNED *) start_address;
    queue -> qu_end =           queue -> qu_start + queue_size;
    queue -> qu_read =          (UNSIGNED *) start_address;
    queue -> qu_write =         (UNSIGNED *) start_address;

    /* Clear the suspension list pointer.  */
    queue -> qu_suspension_list =  NU_NULL;

    /* Clear the number of tasks waiting on the queue counter.  */
    queue -> qu_tasks_waiting =  0;

    /* Clear the urgent message list pointer.  */
    queue -> qu_urgent_list =  NU_NULL;

    /* Initialize link pointers.  */
    queue -> qu_created.cs_previous =    NU_NULL;
    queue -> qu_created.cs_next =        NU_NULL;

    /* Protect against access to the list of created queues.  */
    TCT_Protect(&QUD_List_Protect);

    /* At this point the queue is completely built.  The ID can now be
       set and it can be linked into the created queue list.  */
    queue -> qu_id =                     QU_QUEUE_ID;

    /* Link the queue into the list of created queues and increment the
       total number of queues in the system.  */
    CSC_Place_On_List(&QUD_Created_Queues_List, &(queue -> qu_created));
    QUD_Total_Queues++;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpQueue(RT_PROF_CREATE_QUEUE,queue,RT_PROF_OK);
#endif

    /* Release protection against access to the list of created queues.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUC_Delete_Queue                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deletes a queue and removes it from the list of    */
/*      created queues.  All tasks suspended on the queue are            */
/*      resumed.  Note that this function does not free the memory       */
/*      associated with the queue.                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      QUCE_Delete_Queue                   Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove node from list        */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Protect                         Protect created list         */
/*      TCT_Set_Current_Protect             Setup current protect pointer*/
/*      TCT_System_Protect                  Protect against system access*/
/*      TCT_System_Unprotect                Release system protection    */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      queue_ptr                           Queue control block pointer  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options, changed                  */
/*                      protection logic to reduce                       */
/*                      overhead, resulting in                           */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  QUC_Delete_Queue(NU_QUEUE *queue_ptr)
{

R1 QU_QCB      *queue;                      /* Queue control block ptr   */
QU_SUSPEND     *suspend_ptr;                /* Suspend block pointer     */
QU_SUSPEND     *next_ptr;                   /* Next suspend block pointer*/
STATUS          preempt;                    /* Status for resume call    */
NU_SUPERV_USER_VARIABLES

    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_DELETE_QUEUE_ID, (UNSIGNED) queue,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect against access to the queue.  */
    TCT_System_Protect();

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpQueue(RT_PROF_DELETE_QUEUE,queue,RT_PROF_OK);
#endif

    /* Clear the queue ID.  */
    queue -> qu_id =  0;

    /* Release protection.  */
    TCT_Unprotect();

    /* Protect against access to the list of created queues.  */
    TCT_Protect(&QUD_List_Protect);

    /* Remove the queue from the list of created queues.  */
    CSC_Remove_From_List(&QUD_Created_Queues_List, &(queue -> qu_created));

    /* Decrement the total number of created queues.  */
    QUD_Total_Queues--;

    /* Pickup the suspended task pointer list.  */
    suspend_ptr =  queue -> qu_suspension_list;

    /* Walk the chain task(s) currently suspended on the queue.  */
    preempt =  0;
    while (suspend_ptr)
    {

        /* Protect against system access.  */
        TCT_System_Protect();

        /* Resume the suspended task.  Insure that the status returned is
           NU_QUEUE_DELETED.  */
        suspend_ptr -> qu_return_status =  NU_QUEUE_DELETED;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (QU_SUSPEND *) (suspend_ptr -> qu_suspend_link.cs_next);

        /* Resume the specified task.  */
        preempt =  preempt |
            TCC_Resume_Task((NU_TASK *) suspend_ptr -> qu_suspended_task,
                                                NU_QUEUE_SUSPEND);

        /* Determine if the next is the same as the head pointer.  */
        if (next_ptr == queue -> qu_suspension_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  NU_NULL;
        else

            /* Position suspend pointer to the next pointer.  */
            suspend_ptr =  next_ptr;

        /* Modify current protection.  */
        TCT_Set_Current_Protect(&QUD_List_Protect);

        /* Clear the system protection.  */
        TCT_System_Unprotect();
    }

    /* Pickup the urgent message suspension list.  */
    suspend_ptr =  queue -> qu_urgent_list;

    /* Walk the chain task(s) currently suspended on the queue.  */
    while (suspend_ptr)
    {

        /* Protect against system access.  */
        TCT_System_Protect();

        /* Resume the suspended task.  Insure that the status returned is
           NU_QUEUE_DELETED.  */
        suspend_ptr -> qu_return_status =  NU_QUEUE_DELETED;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (QU_SUSPEND *) (suspend_ptr -> qu_suspend_link.cs_next);

        /* Resume the specified task.  */
        preempt =  preempt |
                TCC_Resume_Task((NU_TASK *) suspend_ptr -> qu_suspended_task,
                                                NU_QUEUE_SUSPEND);

        /* Determine if the next is the same as the head pointer.  */
        if (next_ptr == queue -> qu_urgent_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  NU_NULL;
        else

            /* Position to the next suspend block in the list.  */
            suspend_ptr =  next_ptr;

        /* Modify current protection.  */
        TCT_Set_Current_Protect(&QUD_List_Protect);

        /* Clear the system protection.  */
        TCT_System_Unprotect();
    }

    /* Determine if preemption needs to occur.  */
    if (preempt)

        /* Transfer control to system to facilitate preemption.  */
        TCT_Control_To_System();

    /* Release protection against access to the list of created queues.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return a successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUC_Send_To_Queue                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends a message to the specified queue.  The       */
/*      message length is determined by the caller.  If there are one    */
/*      or more tasks suspended on the queue for a message, the message  */
/*      is copied into the message area of the first waiting task.  If   */
/*      the task's request is satisfied, it is resumed.  Otherwise, if   */
/*      the queue cannot hold the message, suspension of the calling     */
/*      task is an option of the caller.                                 */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      QUCE_Send_To_Queue                  Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Place on suspend list        */
/*      CSC_Priority_Place_On_List          Place on priority list       */
/*      CSC_Remove_From_List                Remove from suspend list     */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      TCC_Suspend_Task                    Suspend calling task         */
/*      TCC_Task_Priority                   Pickup task's priority       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Current_Thread                  Pickup current thread pointer*/
/*      TCT_System_Protect                  Protect queue                */
/*      TCT_Unprotect                       Release protection           */
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
/*      NU_SUCCESS                          If service is successful     */
/*      NU_QUEUE_FULL                       If queue is currently full   */
/*      NU_TIMEOUT                          If timeout on service expires*/
/*      NU_QUEUE_DELETED                    If queue was deleted during  */
/*                                            suspension                 */
/*      NU_QUEUE_RESET                      If queue was reset during    */
/*                                            suspension                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options, changed                  */
/*                      protection logic to reduce                       */
/*                      overhead, optimized copy loop,                   */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  QUC_Send_To_Queue(NU_QUEUE *queue_ptr, VOID *message, UNSIGNED size,
                                                        UNSIGNED suspend)
{

R1 QU_QCB      *queue;                      /* Queue control block ptr   */
QU_SUSPEND      suspend_block;              /* Allocate suspension block */
QU_SUSPEND     *suspend_ptr;                /* Pointer to suspend block  */
R3 UNSIGNED_PTR source;                     /* Pointer to source         */
R4 UNSIGNED_PTR destination;                /* Pointer to destination    */
UNSIGNED        copy_size;                  /* Partial copy size         */
R2 INT          i;                          /* Working counter           */
TC_TCB         *task;                       /* Task pointer              */
STATUS          preempt;                    /* Preempt flag              */
STATUS          status;                     /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_SEND_TO_QUEUE_ID, (UNSIGNED) queue,
                                        (UNSIGNED) message, (UNSIGNED) size);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Protect against simultaneous access to the queue.  */
    TCT_System_Protect();

    /* Determine if an extra word of overhead needs to be added to the
       calculation.  */
    if (queue -> qu_fixed_size)

        /* No overhead.  */
        i =  0;
    else
    {
        /* Variable messages have one additional word of overhead.  */
        i =  1;

        /* Make special check to see if a suspension needs to be
           forced for a variable length message.  */
        if ((queue -> qu_suspension_list) && (queue -> qu_messages))
        {

            /* Pickup task control block pointer.  */
            task =  (TC_TCB *) TCT_Current_Thread();

            /* Now we know that there are other task(s) are suspended trying
               to send a variable length message.  Determine whether or not
               a suspension should be forced.  */
            if ((queue -> qu_fifo_suspend) ||
                (suspend == NU_NO_SUSPEND) ||
                ((queue -> qu_suspension_list) -> qu_suspend_link.cs_priority <=
                                                    TCC_Task_Priority(task)))

                /* Bump the computed size to avoid placing the new variable
                   length message ahead of the suspended tasks.  */
                i =  (INT) queue -> qu_available;
        }
    }

    /* Determine if there is enough room in the queue for the message.  The
       extra logic is to prevent a variable-length message from sn*/
    if (queue -> qu_available < (size + i))
    {

        /* Queue does not have room for the message.  Determine if
           suspension is required.  */
        if (suspend)
        {

            /* Suspension is requested.   */

            /* Increment the number of tasks waiting.  */
            queue -> qu_tasks_waiting++;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpQueue(RT_PROF_SEND_TO_QUEUE,queue,RT_PROF_WAIT);
#endif

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> qu_queue =                    queue;
            suspend_ptr -> qu_suspend_link.cs_next =     NU_NULL;
            suspend_ptr -> qu_suspend_link.cs_previous = NU_NULL;
            suspend_ptr -> qu_message_area =             (UNSIGNED_PTR) message;
            suspend_ptr -> qu_message_size =             size;
            task =                            (TC_TCB *) TCT_Current_Thread();
            suspend_ptr -> qu_suspended_task =           task;

            /* Determine if priority or FIFO suspension is associated with the
               queue.  */
            if (queue -> qu_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this queue.  */
                CSC_Place_On_List((CS_NODE **) &(queue -> qu_suspension_list),
                                        &(suspend_ptr -> qu_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> qu_suspend_link.cs_priority =
                                                 TCC_Task_Priority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                                &(queue -> qu_suspension_list),
                                        &(suspend_ptr -> qu_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the queue.  */
            TCC_Suspend_Task((NU_TASK *) task, NU_QUEUE_SUSPEND,
                                        QUC_Cleanup, suspend_ptr, suspend);

            /* Pickup the return status.  */
            status =  suspend_ptr -> qu_return_status;
        }
        else
        {

            /* Return a status of NU_QUEUE_FULL because there is no
               room in the queue for the message.  */
            status =  NU_QUEUE_FULL;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpQueue(RT_PROF_SEND_TO_QUEUE,queue,RT_PROF_FAIL);
#endif

        }
    }
    else
    {

        /* Determine if a task is waiting on an empty queue.  */
        if ((queue -> qu_suspension_list) && (queue -> qu_messages == 0))
        {

            /* Task is waiting on an empty queue for a message.  */

            /* Decrement the number of tasks waiting on queue.  */
            queue -> qu_tasks_waiting--;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpQueue(RT_PROF_SEND_TO_QUEUE,queue,RT_PROF_OK);
#endif

            /* Remove the first suspended block from the list.  */
            suspend_ptr =  queue -> qu_suspension_list;
            CSC_Remove_From_List((CS_NODE **) &(queue -> qu_suspension_list),
                                          &(suspend_ptr -> qu_suspend_link));

            /* Setup the source and destination pointers.  */
            source =       (UNSIGNED_PTR) message;
            destination =  suspend_ptr -> qu_message_area;

            /* Initialize the return status.  */
            suspend_ptr -> qu_return_status =  NU_SUCCESS;

            /* Loop to actually copy the message.  */
            i = (INT) size;
            do
            {
                *(destination++) =  *(source);
                if ((--i) == 0)
                    break;
                source++;
            } while (1);

            /* Return the size of the message copied.  */
            suspend_ptr -> qu_actual_size =  size;

            /* Wakeup the waiting task and check for preemption.  */
            preempt =
                TCC_Resume_Task((NU_TASK *) suspend_ptr -> qu_suspended_task,
                                                     NU_QUEUE_SUSPEND);

            /* Determine if preemption needs to take place. */
            if (preempt)

                 /* Transfer control to the system if the resumed task function
                   detects a preemption condition.  */
                TCT_Control_To_System();
        }
        else
        {

            /* There is enough room in the queue and no task is waiting.  */

            /* Setup the source pointer.  */
            source =       (UNSIGNED_PTR) message;
            destination =  queue -> qu_write;

            /* Process according to the type of message supported.  */
            if (queue -> qu_fixed_size)
            {

                /* Fixed-size messages are supported by this queue.  */

                /* Loop to copy the message into the queue area.  */
                i =  (INT) size;
                do
                {
                    *(destination++) =  *(source);
                    if ((--i) == 0)
                        break;
                    source++;
                } while (1);
            }
            else
            {

                /* Variable-size messages are supported.  Processing must
                   check for queue wrap-around conditions.  */

                /* Place message size in first location.  */
                *(destination++) =  size;

                /* Check for a wrap-around condition on the queue.  */
                if (destination >= queue -> qu_end)

                    /* Wrap the write pointer back to the top of the queue
                       area.  */
                    destination =  queue -> qu_start;

                /* Decrement the number of words remaining by 1 for this
                   extra word of overhead.  */
                queue -> qu_available--;

                /* Calculate the number of words remaining from the write
                   pointer to the bottom of the queue.  */
                copy_size =  queue -> qu_end - destination;

                /* Determine if the message needs to be wrapped around the
                   edge of the queue area.  */
                if (copy_size >= size)
                {

                    /* Copy the whole message at once.  */
                    i =  (INT) size;
                    do
                    {
                        *(destination++) =  *(source);
                        if ((--i) == 0)
                            break;
                        source++;
                    } while (1);
                }
                else
                {

                    /* Copy the first half of the message.  */
                    i =  (INT) copy_size;
                    do
                    {
                        *(destination) =  *(source++);
                        if ((--i) == 0)
                            break;
                        destination++;
                    } while (1);

                    /* Copy the second half of the message.  */
                    destination =  queue -> qu_start;
                    i =  (INT) (size - copy_size);
                    do
                    {
                        *(destination++) =  *(source);
                        if ((--i) == 0)
                            break;
                        source++;
                    } while (1);
                }
            }

            /* Check again for wrap-around condition on the write pointer. */
            if (destination >= queue -> qu_end)

                /* Move the write pointer to the top of the queue area.  */
                queue -> qu_write =  queue -> qu_start;
            else

                /* Simply copy the last position of the destination pointer
                   into the write pointer.  */
                queue -> qu_write =  destination;

            /* Decrement the number of available words.  */
            queue -> qu_available =  queue -> qu_available - size;

            /* Increment the number of messages in the queue.  */
            queue -> qu_messages++;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpQueue(RT_PROF_SEND_TO_QUEUE,queue,RT_PROF_OK);
#endif

        }
    }

    /* Release protection against access to the queue.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUC_Receive_From_Queue                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function receives a message from the specified queue.  The  */
/*      size of the message is specified by the caller.  If there is a   */
/*      message currently in the queue, the message is removed from the  */
/*      queue and placed in the caller's area.  Suspension is possible   */
/*      if the request cannot be satisfied.                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      QUCE_Receive_From_Queue             Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Place on suspend list        */
/*      CSC_Priority_Place_On_List          Place on priority list       */
/*      CSC_Remove_From_List                Remove from suspend list     */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      TCC_Suspend_Task                    Suspend calling task         */
/*      TCC_Task_Priority                   Pickup task's priority       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Current_Thread                  Pickup current thread pointer*/
/*      TCT_System_Protect                  Protect queue                */
/*      TCT_Unprotect                       Release protection           */
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
/*      NU_SUCCESS                          If service is successful     */
/*      NU_QUEUE_EMPTY                      If queue is currently empty  */
/*      NU_TIMEOUT                          If timeout on service expires*/
/*      NU_QUEUE_DELETED                    If queue was deleted during  */
/*                                            suspension                 */
/*      NU_QUEUE_RESET                      If queue was reset during    */
/*                                            suspension                 */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      11-01-1993      Corrected a problem resuming a                   */
/*                      task suspended on a full queue                   */
/*                      that only has a capacity of a                    */
/*                      single message, resulting in                     */
/*                      version 1.0b                                     */
/*      11-01-1993      Verified version 1.0b                            */
/*      03-01-1994      Changed function interfaces to                   */
/*                      match those in prototype,                        */
/*                      added register options, changed                  */
/*                      protection logic to reduce                       */
/*                      overhead, optimized copy loop,                   */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      01-28-1998      Corrected SPR412.                                */
/*                                                                       */
/*************************************************************************/
STATUS  QUC_Receive_From_Queue(NU_QUEUE *queue_ptr, VOID *message,
                UNSIGNED size, UNSIGNED *actual_size, UNSIGNED suspend)
{

R1 QU_QCB      *queue;                      /* Queue control block ptr   */
QU_SUSPEND      suspend_block;              /* Allocate suspension block */
QU_SUSPEND     *suspend_ptr;                /* Pointer to suspend block  */
R3 UNSIGNED_PTR source;                     /* Pointer to source         */
R4 UNSIGNED_PTR destination;                /* Pointer to destination    */
TC_TCB         *task;                       /* Task pointer              */
UNSIGNED        copy_size;                  /* Number of words to copy   */
R2 INT          i;                          /* Working counter           */
STATUS          preempt;                    /* Preemption flag           */
STATUS          status;                     /* Completion status         */
NU_SUPERV_USER_VARIABLES


    /* Move input queue pointer into internal pointer.  */
    queue =  (QU_QCB *) queue_ptr;

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_RECEIVE_FROM_QUEUE_ID, (UNSIGNED) queue,
                                        (UNSIGNED) message, (UNSIGNED) size);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Protect against simultaneous access to the queue.  */
    TCT_System_Protect();

    /* Determine if an urgent message request is currently suspended.  */
    if (queue -> qu_urgent_list)
    {

        /* If so, copy the message from the suspended request block and
           resume the associated task.  */

        /* Decrement the number of tasks waiting on queue.  */
        queue -> qu_tasks_waiting--;

        /* Remove the first suspended block from the list.  */
        suspend_ptr =  queue -> qu_urgent_list;
        CSC_Remove_From_List((CS_NODE **) &(queue -> qu_urgent_list),
                                          &(suspend_ptr -> qu_suspend_link));

        /* Setup the source and destination pointers.  */
        destination =   (UNSIGNED_PTR) message;
        source =        suspend_ptr -> qu_message_area;

        /* Initialize the return status.  */
        suspend_ptr -> qu_return_status =  NU_SUCCESS;

        /* Loop to actually copy the message.  */
        i =  (INT) suspend_ptr -> qu_message_size;
        do
        {
            *(destination++) =  *(source);
            if ((--i) == 0)
                break;
            source++;
        } while (1);

        /* Return the size of the message copied.  */
        *actual_size =  suspend_ptr -> qu_message_size;

        /* Wakeup the waiting task and check for preemption.  */
        preempt =
            TCC_Resume_Task((NU_TASK *) suspend_ptr -> qu_suspended_task,
                                                     NU_QUEUE_SUSPEND);

        /* Determine if preemption needs to take place. */
        if (preempt)

             /* Transfer control to the system if the resumed task function
                detects a preemption condition.  */
             TCT_Control_To_System();
    }

    /* Determine if there are messages in the queue.  */
    else if (queue -> qu_messages)
    {

        /* Copy message from queue into the caller's area.  */

        /* Setup the source and destination pointers.  */
        source =       queue -> qu_read;
        destination =  (UNSIGNED_PTR) message;

        /* Process according to the type of message supported by the queue. */
        if (queue -> qu_fixed_size)
        {

            /* Queue supports fixed-size messages.  */

            /* Copy the message from the queue area into the destination.  */
            i =  (INT) size;
            do
            {
                *(destination) =  *(source++);
                if ((--i) == 0)
                    break;
                destination++;
            } while (1);
        }
        else
        {

            /* Queue supports variable-size messages.  */

            /* Variable length message size is actually in the queue area. */
            size =  *(source++);

            /* Check for a wrap-around condition on the queue.  */
            if (source >= queue -> qu_end)

                /* Wrap the read pointer back to the top of the queue
                   area.  */
                source =  queue -> qu_start;

            /* Increment the number of available words in the queue.  */
            queue -> qu_available++;

            /* Calculate the number of words remaining from the read pointer
               to the bottom of the queue.  */
            copy_size =  queue -> qu_end - source;

            /* Determine if the message needs to be wrapped around the
               edge of the queue area.  */
            if (copy_size >= size)
            {

                /* Copy the whole message at once.  */
                i =  (INT) size;
                do
                {
                    *(destination) =  *(source++);
                    if ((--i) == 0)
                        break;
                    destination++;
                } while (1);
            }
            else
            {

                /* Copy the first half of the message.  */
                i =  (INT) copy_size;
                do
                {
                    *(destination++) =  *(source);
                    if ((--i) == 0)
                        break;
                    source++;
                } while (1);

                /* Copy the second half of the message.  */
                source =  queue -> qu_start;
                i =  (INT) (size - copy_size);
                do
                {
                    *(destination) =  *(source++);
                    if ((--i) == 0)
                        break;
                    destination++;
                } while (1);
            }
        }

        /* Check again for wrap-around condition on the read pointer. */
        if (source >= queue -> qu_end)

            /* Move the read pointer to the top of the queue area.  */
            queue -> qu_read =  queue -> qu_start;
        else

            /* Move the read pointer to where the copy left off.  */
            queue -> qu_read =  source;

        /* Increment the number of available words.  */
        queue -> qu_available =  queue -> qu_available + size;

        /* Decrement the number of messages in the queue.  */
        queue -> qu_messages--;

        /* Return the number of words received.  */
        *actual_size =  size;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpQueue(RT_PROF_RECEIVE_FROM_QUEUE,queue,RT_PROF_OK);
#endif

        /* Determine if any tasks suspended on a full queue can be woken
           up.  */
        if (queue -> qu_suspension_list)
        {

            /* Overhead of each queue message.  */
            if (!queue -> qu_fixed_size)

                i =  1;
            else

                i =  0;

            /* Pickup the suspension list and examine suspension blocks
               to see if the message could now fit in the queue.  */
            suspend_ptr =  queue -> qu_suspension_list;
            preempt =      NU_FALSE;
            while ((suspend_ptr) &&
              ((suspend_ptr -> qu_message_size + i) <= queue -> qu_available))
            {

                /* Place the suspended task's message into the queue.  */

                /* Setup the source and destination pointers.  */
                source =        suspend_ptr -> qu_message_area;
                destination =   queue -> qu_write;
                size =          suspend_ptr -> qu_message_size;

                /* Process according to the type of message supported.  */
                if (queue -> qu_fixed_size)
                {

                    /* Fixed-size messages are supported by this queue.  */

                    /* Loop to copy the message into the queue area.  */
                    i =  (INT) size;
                    do
                    {
                        *(destination++) =  *(source);
                        if ((--i) == 0)
                            break;
                        source++;
                    } while (1);
                }
                else
                {

                    /* Variable-size messages are supported.  Processing must
                       check for queue wrap-around conditions.  */

                    /* Place message size in first location.  */
                    *(destination++) =  size;

                    /* Check for a wrap-around condition on the queue.  */
                    if (destination >= queue -> qu_end)

                        /* Wrap the write pointer back to the top of the queue
                           area.  */
                        destination =  queue -> qu_start;

                    /* Decrement the number of words remaining by 1 for this
                       extra word of overhead.  */
                    queue -> qu_available--;

                    /* Calculate the number of words remaining from the write
                       pointer to the bottom of the queue.  */
                    copy_size =  queue -> qu_end - destination;

                    /* Determine if the message needs to be wrapped around the
                       edge of the queue area.  */
                    if (copy_size >= size)
                    {

                        /* Copy the whole message at once.  */
                        i =  (INT) size;
                        do
                        {
                            *(destination++) =  *(source);
                            if ((--i) == 0)
                                break;
                            source++;
                        } while(1);
                    }
                    else
                    {

                        /* Copy the first half of the message.  */
                        i =  (INT) copy_size;
                        do
                        {
                            *(destination) =  *(source++);
                            if ((--i) == 0)
                                break;
                            destination++;
                        } while (1);

                        /* Copy the second half of the message.  */
                        destination =  queue -> qu_start;
                        i =  (INT) (size - copy_size);
                        do
                        {
                            *(destination++) =  *(source);
                            if ((--i) == 0)
                                break;
                            source++;
                        } while (1);
                    }
                }

                /* Check again for wrap-around condition on the write
                   pointer. */
                if (destination >= queue -> qu_end)

                    /* Move the write pointer to the top of the queue area.  */
                    queue -> qu_write =  queue -> qu_start;
                else

                    /* Simply copy the last position of the destination pointer
                       into the write pointer.  */
                    queue -> qu_write =  destination;

                /* Decrement the number of available words.  */
                queue -> qu_available =  queue -> qu_available - size;

                /* Increment the number of messages in the queue.  */
                queue -> qu_messages++;

                /* Decrement the number of tasks waiting counter.  */
                queue -> qu_tasks_waiting--;

                /* Remove the first suspended block from the list.  */
                CSC_Remove_From_List((CS_NODE **)
                    &(queue -> qu_suspension_list),
                                &(suspend_ptr -> qu_suspend_link));

                /* Return a successful status.  */
                suspend_ptr -> qu_return_status =  NU_SUCCESS;

                /* Resume the suspended task.  */
                preempt =  preempt |
                  TCC_Resume_Task((NU_TASK *) suspend_ptr -> qu_suspended_task,
                                                       NU_QUEUE_SUSPEND);

                /* Setup suspend pointer to the head of the list.  */
                suspend_ptr =  queue -> qu_suspension_list;

                /* Overhead of each queue message.  */
                if (!queue -> qu_fixed_size)

                    i =  1;
                else

                    i =  0;
            }

            /* Determine if a preempt condition is present.  */
            if (preempt)

                /* Transfer control to the system if the resumed task function
                   detects a preemption condition.  */
                TCT_Control_To_System();
        }
    }
    else
    {

        /* Queue is empty.  Determine if the task wants to suspend.  */
        if (suspend)
        {

            /* Increment the number of tasks waiting on the queue counter. */
            queue -> qu_tasks_waiting++;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpQueue(RT_PROF_RECEIVE_FROM_QUEUE,queue,RT_PROF_WAIT);
#endif

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> qu_queue =                    queue;
            suspend_ptr -> qu_suspend_link.cs_next =     NU_NULL;
            suspend_ptr -> qu_suspend_link.cs_previous = NU_NULL;
            suspend_ptr -> qu_message_area =           (UNSIGNED_PTR) message;
            suspend_ptr -> qu_message_size =             size;
            task =                            (TC_TCB *) TCT_Current_Thread();
            suspend_ptr -> qu_suspended_task =           task;

            /* Determine if priority or FIFO suspension is associated with the
               queue.  */
            if (queue -> qu_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this queue.  */
                CSC_Place_On_List((CS_NODE **) &(queue -> qu_suspension_list),
                                        &(suspend_ptr -> qu_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> qu_suspend_link.cs_priority =
                                                    TCC_Task_Priority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                            &(queue -> qu_suspension_list),
                                            &(suspend_ptr -> qu_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the queue.  */
            TCC_Suspend_Task((NU_TASK *) task, NU_QUEUE_SUSPEND,
                                        QUC_Cleanup, suspend_ptr, suspend);

            /* Pickup the status of the request.  */
            status =  suspend_ptr -> qu_return_status;
            *actual_size =  suspend_ptr -> qu_actual_size;
        }
        else
        {

            /* Return a status of NU_QUEUE_EMPTY because there are no
               messages in the queue.  */
            status =  NU_QUEUE_EMPTY;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpQueue(RT_PROF_RECEIVE_FROM_QUEUE,queue,RT_PROF_FAIL);
#endif

        }
    }

    /* Release protection against access to the queue.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUC_Cleanup                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible for removing a suspension block     */
/*      from a queue.  It is not called unless a timeout or a task       */
/*      terminate is in progress.  Note that protection is already in    */
/*      effect - the same protection at suspension time.  This routine   */
/*      must be called from Supervisor mode in Supervisor/User mode      */
/*      switching kernels.                                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TCC_Timeout                         Task timeout                 */
/*      TCC_Terminate                       Task terminate               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove suspend block from    */
/*                                            the suspension list        */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      information                         Pointer to suspend block     */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
VOID  QUC_Cleanup(VOID *information)
{

QU_SUSPEND      *suspend_ptr;               /* Suspension block pointer  */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Use the information pointer as a suspend pointer.  */
    suspend_ptr =  (QU_SUSPEND *) information;

    /* By default, indicate that the service timed-out.  It really does not
       matter if this function is called from a terminate request since
       the task does not resume.  */
    suspend_ptr -> qu_return_status =  NU_TIMEOUT;

    /* Decrement the number of tasks waiting counter.  */
    (suspend_ptr -> qu_queue) -> qu_tasks_waiting--;

    /* Unlink the suspend block from the suspension list.  */
    if ((suspend_ptr -> qu_queue) -> qu_urgent_list)
    {
       /* Unlink the suspend block from the suspension list.  */
       CSC_Remove_From_List((CS_NODE **) 
                   &((suspend_ptr -> qu_queue) -> qu_urgent_list), 
                                   &(suspend_ptr -> qu_suspend_link));
    }
    else
    {
       /* Unlink the suspend block from the suspension list.  */
       CSC_Remove_From_List((CS_NODE **) 
                   &((suspend_ptr -> qu_queue) -> qu_suspension_list), 
                                   &(suspend_ptr -> qu_suspend_link));
    }

    /* Return to user mode */
    NU_USER_MODE();
}






