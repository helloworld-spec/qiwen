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
/*      qus.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      QU - Queue Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains supplemental routines for the Queue           */
/*      Management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      QUS_Reset_Queue                     Reset a queue                */
/*      QUS_Send_To_Front_Of_Queue          Send message to queue's front*/
/*      QUS_Broadcast_To_Queue              Broadcast a message to queue */
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
/*      03-01-1994      Created initial version 1.1 from                 */
/*                      routines originally in core                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      02-04-1998      Corrected SPR434 resulting in                    */
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
#include        "profiler.h"                /* ProView interface         */


/* Define internal component function prototypes.  */

VOID    QUC_Cleanup(VOID *information);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUS_Reset_Queue                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function resets the specified queue back to the original    */
/*      state.  Any messages in the queue are discarded.  Also, any      */
/*      tasks currently suspended on the queue are resumed with the      */
/*      reset status.                                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      QUSE_Reset_Queue                    Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_System_Protect                  Protect queue data structures*/
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
/*                      overhead, fixed read and write                   */
/*                      pointers to both point at the                    */
/*                      start, resulting in version 1.1                  */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      02-04-1998      Corrected SPR434.                                */
/*                                                                       */
/*************************************************************************/
STATUS  QUS_Reset_Queue(NU_QUEUE *queue_ptr)
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
    HIC_Make_History_Entry(NU_RESET_QUEUE_ID, (UNSIGNED) queue,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect against access to the queue.  */
    TCT_System_Protect();

    /* Pickup the suspended task suspension list.  */
    suspend_ptr =  queue -> qu_suspension_list;

    /* Walk the chain task(s) currently suspended on the queue.  */
    preempt =  0;
    while (suspend_ptr)
    {

        /* Resume the suspended task.  Insure that the status returned is
           NU_QUEUE_RESET.  */
        suspend_ptr -> qu_return_status =  NU_QUEUE_RESET;

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

            /* Position the suspend pointer to the next suspend block.  */
            suspend_ptr =  next_ptr;
    }

    /* Pickup the urgent message suspension list.  */
    suspend_ptr =  queue -> qu_urgent_list;

    /* Walk the chain task(s) currently suspended on the queue.  */
    while (suspend_ptr)
    {

        /* Resume the suspended task.  Insure that the status returned is
           NU_QUEUE_RESET.  */
        suspend_ptr -> qu_return_status =  NU_QUEUE_RESET;

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

            /* Position suspend pointer to the next suspend block.  */
            suspend_ptr =  next_ptr;
    }

    /* Initialize various elements of the queue.  */
    queue -> qu_available =             queue -> qu_end - queue -> qu_start;
    queue -> qu_messages =              0;
    queue -> qu_read =                  queue -> qu_start;
    queue -> qu_write =                 queue -> qu_start;
    queue -> qu_tasks_waiting =         0;
    queue -> qu_suspension_list =       NU_NULL;
    queue -> qu_urgent_list =           NU_NULL;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpQueue(RT_PROF_RESET_QUEUE,queue,RT_PROF_OK);
#endif

    /* Determine if preemption needs to occur.  */
    if (preempt)

        /* Transfer control to system to facilitate preemption.  */
        TCT_Control_To_System();

    /* Release protection against access to the queue.  */
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
/*      QUS_Send_To_Front_Of_Queue                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends a message to the front of the specified      */
/*      message queue.  The message length is determined by the caller.  */
/*      If there are any tasks suspended on the queue for a message, the */
/*      message is copied into the message area of the first waiting     */
/*      task and that task is resumed.  If there is enough room in the   */
/*      queue, the message is copied in front of all other messages.     */
/*      If there is not enough room in the queue, suspension of the      */
/*      caller is possible.                                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      QUSE_Send_To_Front_Of_Queue         Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Place on suspend list        */
/*      CSC_Remove_From_List                Remove from suspend list     */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      TCC_Suspend_Task                    Suspend calling task         */
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
/*         DATE                    REMARKS                               */
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
STATUS  QUS_Send_To_Front_Of_Queue(NU_QUEUE *queue_ptr, VOID *message,
                                        UNSIGNED size, UNSIGNED suspend)
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
    HIC_Make_History_Entry(NU_SEND_TO_FRONT_OF_QUEUE_ID, (UNSIGNED) queue,
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

    /* Determine if there is enough room in the queue for the message.  */
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
            _RTProf_DumpQueue(RT_PROF_SEND_TO_FRONT_OF_QUEUE,queue,RT_PROF_WAIT);
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

            /* Place the task on the urgent message suspension list.  */
            CSC_Place_On_List((CS_NODE **) &(queue -> qu_urgent_list),
                                        &(suspend_ptr -> qu_suspend_link));

            /* Move the head pointer of the list to make this suspension the
               first in the list.  */
            queue -> qu_urgent_list =  (QU_SUSPEND *)
                (queue -> qu_urgent_list) -> qu_suspend_link.cs_previous;

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
            _RTProf_DumpQueue(RT_PROF_SEND_TO_FRONT_OF_QUEUE,queue,RT_PROF_FAIL);
#endif
        }
    }
    else
    {

        /* Determine if a task is waiting on an empty queue.  */
        if ((queue -> qu_suspension_list) && (queue -> qu_messages == 0))
        {

            /* Task is waiting on queue for a message.  */

            /* Decrement the number of tasks waiting on queue.  */
            queue -> qu_tasks_waiting--;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpQueue(RT_PROF_SEND_TO_FRONT_OF_QUEUE,queue,RT_PROF_OK);
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
            i =  (INT) size;
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
            destination =  queue -> qu_read;

            /* Process according to the type of message supported.  */
            if (queue -> qu_fixed_size)
            {

                /* Fixed-size message queue.  */

                /* Determine if the read pointer is at the top of the queue
                   area.  */
                if (destination == queue -> qu_start)

                    /* Prepare to place the message in the lower part
                       of the queue area.  */
                    destination =  queue -> qu_end - size;
                else

                    /* Backup the length of a message from the current
                       read pointer.  */
                    destination =  destination - size;

                /* Adjust the actual read pointer before the copy is done.  */
                queue -> qu_read =  destination;

                /* Copy the message into the queue area.  */
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

                /* Variable-size message queue.  */

                /* Calculate the number of words remaining at the top of the
                   queue.  */
                copy_size =  destination - queue -> qu_start;

                /* Determine if part of the message needs to be placed at the
                   bottom of the queue area.  */
                if (copy_size < (size + i))

                    /* Compute the starting location for the message.  */
                    destination =  queue -> qu_end - ((size +i) - copy_size);
                else

                    /* Compute the starting location for the message.  */
                    destination =  destination - (size + i);

                /* Adjust the actual queue read pointer also.  */
                queue -> qu_read =  destination;

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

                /* Calculate the number of words remaining from the
                   destination pointer to the bottom of the queue.  */
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

            /* Decrement the number of available words.  */
            queue -> qu_available =  queue -> qu_available - size;

            /* Increment the number of messages in the queue.  */
            queue -> qu_messages++;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpQueue(RT_PROF_SEND_TO_FRONT_OF_QUEUE,queue,RT_PROF_OK);
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
/*      QUS_Broadcast_To_Queue                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends a message to all tasks waiting for a message */
/*      from the specified queue.  If there are no tasks waiting for a   */
/*      message the service performs like a standard send request.       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      QUSE_Broadcast_To_Queue             Error checking shell         */
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
STATUS  QUS_Broadcast_To_Queue(NU_QUEUE *queue_ptr, VOID *message,
                                        UNSIGNED size, UNSIGNED suspend)
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
    HIC_Make_History_Entry(NU_BROADCAST_TO_QUEUE_ID, (UNSIGNED) queue,
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

    /* Determine if there is enough room in the queue for the message.  */
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
            _RTProf_DumpQueue(RT_PROF_BROADCAST_TO_QUEUE,queue,RT_PROF_WAIT);
#endif

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr = &suspend_block;
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

            /* Pickup the return status.  */
            status =  suspend_ptr -> qu_return_status;
        }
        else
        {

            /* Return a status of NU_QUEUE_FULL because there is no
               room in the queue for the message.  */
            status =  NU_QUEUE_FULL;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpQueue(RT_PROF_BROADCAST_TO_QUEUE,queue,RT_PROF_FAIL);
#endif

        }
    }
    else
    {

        /* Determine if a task is waiting on an empty queue.  */
        if ((queue -> qu_suspension_list) && (queue -> qu_messages == 0))
        {

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpQueue(RT_PROF_BROADCAST_TO_QUEUE,queue,RT_PROF_OK);
#endif

            /* Yes, one or more tasks are waiting for a message from this
               queue.  */
            preempt =  0;
            do
            {

                /* Decrement the number of tasks waiting on queue.  */
                queue -> qu_tasks_waiting--;

                /* Remove the first suspended block from the list.  */
                suspend_ptr =  queue -> qu_suspension_list;
                CSC_Remove_From_List((CS_NODE **)
                                &(queue -> qu_suspension_list),
                                          &(suspend_ptr -> qu_suspend_link));

                /* Setup the source and destination pointers.  */
                source =       (UNSIGNED_PTR) message;
                destination =  suspend_ptr -> qu_message_area;

                /* Initialize the return status.  */
                suspend_ptr -> qu_return_status =  NU_SUCCESS;

                /* Loop to actually copy the message.  */
                i =  (INT) size;
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
                preempt =  preempt |
                 TCC_Resume_Task((NU_TASK *) suspend_ptr -> qu_suspended_task,
                                                         NU_QUEUE_SUSPEND);

                /* Move the suspend pointer to the next node, which is now
                   at the head of the list.  */
                suspend_ptr =  queue -> qu_suspension_list;
            } while (suspend_ptr);

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
        _RTProf_DumpQueue(RT_PROF_BROADCAST_TO_QUEUE,queue,RT_PROF_OK);
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




