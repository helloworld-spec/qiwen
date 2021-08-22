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
/*      pis.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PI - Pipe Management                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the supplemental routines for the pipe        */
/*      management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      PIS_Reset_Pipe                      Reset a pipe                 */
/*      PIS_Send_To_Front_Of_Pipe           Send message to pipe's front */
/*      PIS_Broadcast_To_Pipe               Broadcast a message to pipe  */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      pi_extr.h                           Pipe functions               */
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
#include        "pi_extr.h"                 /* Pipe functions            */
#include        "hi_extr.h"                 /* History functions         */
#include        "profiler.h"                /* ProView interface         */


/* Define external inner-component global data references.  */

extern CS_NODE         *PID_Created_Pipes_List;
extern UNSIGNED         PID_Total_Pipes;
extern TC_PROTECT       PID_List_Protect;


/* Define internal component function prototypes.  */

VOID    PIC_Cleanup(VOID *information);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PIS_Reset_Pipe                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function resets the specified pipe back to the original     */
/*      state.  Any messages in the pipe are discarded.  Also, any       */
/*      tasks currently suspended on the pipe are resumed with the       */
/*      reset status.                                                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      PISE_Reset_Pipe                     Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_System_Protect                  Protect against system access*/
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pipe_ptr                            Pipe control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
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
STATUS  PIS_Reset_Pipe(NU_PIPE *pipe_ptr)
{

R1 PI_PCB      *pipe;                       /* Pipe control block ptr    */
PI_SUSPEND     *suspend_ptr;                /* Suspend block pointer     */
PI_SUSPEND     *next_ptr;                   /* Next suspend block pointer*/
STATUS          preempt;                    /* Status for resume call    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pipe pointer into internal pointer.  */
    pipe =  (PI_PCB *) pipe_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_RESET_PIPE_ID, (UNSIGNED) pipe,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect against access to the pipe.  */
    TCT_System_Protect();

    /* Pickup the suspended task suspension list.  */
    suspend_ptr =  pipe -> pi_suspension_list;

    /* Walk the chain task(s) currently suspended on the pipe.  */
    preempt =  0;
    while (suspend_ptr)
    {

        /* Resume the suspended task.  Insure that the status returned is
           NU_PIPE_RESET.  */
        suspend_ptr -> pi_return_status =  NU_PIPE_RESET;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (PI_SUSPEND *) (suspend_ptr -> pi_suspend_link.cs_next);

        /* Resume the specified task.  */
        preempt =  preempt |
            TCC_Resume_Task((NU_TASK *) suspend_ptr -> pi_suspended_task,
                                                NU_PIPE_SUSPEND);

        /* Determine if the next is the same as the head pointer.  */
        if (next_ptr == pipe -> pi_suspension_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  NU_NULL;
        else

            /* Position the suspend pointer to the next block.  */
            suspend_ptr =  next_ptr;
    }

    /* Pickup the urgent message suspension list.  */
    suspend_ptr =  pipe -> pi_urgent_list;

    /* Walk the chain task(s) currently suspended on the pipe.  */
    while (suspend_ptr)
    {

        /* Resume the suspended task.  Insure that the status returned is
           NU_PIPE_RESET.  */
        suspend_ptr -> pi_return_status =  NU_PIPE_RESET;

        /* Point to the next suspend structure in the link.  */
        next_ptr =  (PI_SUSPEND *) (suspend_ptr -> pi_suspend_link.cs_next);

        /* Resume the specified task.  */
        preempt =  preempt |
            TCC_Resume_Task((NU_TASK *) suspend_ptr -> pi_suspended_task,
                                                NU_PIPE_SUSPEND);

        /* Determine if the next is the same as the head pointer.  */
        if (next_ptr == pipe -> pi_urgent_list)

            /* Clear the suspension pointer to signal the end of the list
               traversal.  */
            suspend_ptr =  NU_NULL;
        else

            /* Position the suspend pointer to the next active block.  */
            suspend_ptr =  next_ptr;
    }

    /* Initialize various elements of the pipe.  */
    pipe -> pi_available =             pipe -> pi_end - pipe -> pi_start;
    pipe -> pi_messages =              0;
    pipe -> pi_read =                  pipe -> pi_start;
    pipe -> pi_write =                 pipe -> pi_start;
    pipe -> pi_tasks_waiting =         0;
    pipe -> pi_suspension_list =       NU_NULL;
    pipe -> pi_urgent_list =           NU_NULL;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpPipe(RT_PROF_RESET_PIPE,pipe,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

    /* Determine if preemption needs to occur.  */
    if (preempt)

        /* Transfer control to system to facilitate preemption.  */
        TCT_Control_To_System();

    /* Release protection against access to the pipe.  */
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
/*      PIS_Send_To_Front_Of_Pipe                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends a message to the front of the specified      */
/*      message pipe.  The message length is determined by the caller.   */
/*      If there are any tasks suspended on the pipe for a message, the  */
/*      message is copied into the message area of the first waiting     */
/*      task and that task is resumed.  If there is enough room in the   */
/*      pipe, the message is copied in front of all other messages.      */
/*      If there is not enough room in the pipe, suspension of the       */
/*      caller is possible.                                              */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      PISE_Send_To_Front_Of_Pipe          Error checking shell         */
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
/*      TCT_System_Protect                  Protect pipe                 */
/*      TCT_Unprotect                       Release protection           */
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
/*      NU_SUCCESS                          If service is successful     */
/*      NU_PIPE_FULL                        If pipe is currently full    */
/*      NU_TIMEOUT                          If timeout on service expires*/
/*      NU_PIPE_DELETED                     If pipe was deleted during   */
/*                                            suspension                 */
/*      NU_PIPE_RESET                       If pipe was reset during     */
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
STATUS  PIS_Send_To_Front_Of_Pipe(NU_PIPE *pipe_ptr, VOID *message,
                                        UNSIGNED size, UNSIGNED suspend)
{

R1 PI_PCB      *pipe;                       /* Pipe control block ptr    */
PI_SUSPEND      suspend_block;              /* Allocate suspension block */
PI_SUSPEND     *suspend_ptr;                /* Pointer to suspend block  */
R2 BYTE_PTR     source;                     /* Pointer to source         */
R3 BYTE_PTR     destination;                /* Pointer to destination    */
UNSIGNED        copy_size;                  /* Partial copy size         */
R4 INT          i;                          /* Working counter           */
UNSIGNED        pad = 0;                    /* Number of pad bytes       */
TC_TCB         *task;                       /* Task pointer              */
STATUS          preempt;                    /* Preempt flag              */
STATUS          status;                     /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pipe pointer into internal pointer.  */
    pipe =  (PI_PCB *) pipe_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_SEND_TO_FRONT_OF_PIPE_ID, (UNSIGNED) pipe,
                                        (UNSIGNED) message, (UNSIGNED) size);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Protect against simultaneous access to the pipe.  */
    TCT_System_Protect();

    /* Determine if an extra word of overhead needs to be added to the
       calculation.  */
    if (pipe -> pi_fixed_size)

        /* No overhead.  */
        i =  0;
    else
    {

        /* Variable messages have one additional word of overhead.  */
        i =  sizeof(UNSIGNED);

        /* Calculate the number of pad bytes necessary to keep the pipe
           write pointer on an UNSIGNED data element alignment.  */
        pad =  (((size + sizeof(UNSIGNED) - 1)/sizeof(UNSIGNED)) *
                        sizeof(UNSIGNED)) - size;

        /* Insure that padding is included in the overhead.  */
        i =  i + ((INT) pad);

        /* Make special check to see if a suspension needs to be
           forced for a variable length message.  */
        if ((pipe -> pi_suspension_list) && (pipe -> pi_messages))
        {

            /* Pickup task control block pointer.  */
            task =  (TC_TCB *) TCT_Current_Thread();

            /* Now we know that there are other task(s) are suspended trying
               to send a variable length message.  Determine whether or not
               a suspension should be forced.  */
            if ((pipe -> pi_fifo_suspend) ||
                (suspend == NU_NO_SUSPEND) ||
                ((pipe -> pi_suspension_list) -> pi_suspend_link.cs_priority <=
                                                    TCC_Task_Priority(task)))

                /* Bump the computed size to avoid placing the new variable
                   length message ahead of the suspended tasks.  */
                i =  (INT) pipe -> pi_available;
        }
    }

    /* Determine if there is enough room in the pipe for the message.  */
    if (pipe -> pi_available < (size + i))
    {

        /* pipe does not have room for the message.  Determine if
           suspension is required.  */
        if (suspend)
        {

            /* Suspension is requested.   */

            /* Increment the number of tasks waiting.  */
            pipe -> pi_tasks_waiting++;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpPipe(RT_PROF_SEND_TO_FRONT_OF_PIPE,pipe,RT_PROF_WAIT);
#endif /* INCLUDE_PROVIEW */

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> pi_pipe =                     pipe;
            suspend_ptr -> pi_suspend_link.cs_next =     NU_NULL;
            suspend_ptr -> pi_suspend_link.cs_previous = NU_NULL;
            suspend_ptr -> pi_message_area =       (BYTE_PTR) message;
            suspend_ptr -> pi_message_size =             size;
            task =                            (TC_TCB *) TCT_Current_Thread();
            suspend_ptr -> pi_suspended_task =           task;

            /* Place the task on the urgent message suspension list.  */
            CSC_Place_On_List((CS_NODE **) &(pipe -> pi_urgent_list),
                                        &(suspend_ptr -> pi_suspend_link));

            /* Move the head pointer of the list to make this suspension the
               first in the list.  */
            pipe -> pi_urgent_list =  (PI_SUSPEND *)
                (pipe -> pi_urgent_list) -> pi_suspend_link.cs_previous;

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the pipe.  */
            TCC_Suspend_Task((NU_TASK *) task, NU_PIPE_SUSPEND,
                                        PIC_Cleanup, suspend_ptr, suspend);

            /* Pickup the return status.  */
            status =  suspend_ptr -> pi_return_status;
        }
        else
        {

            /* Return a status of NU_PIPE_FULL because there is no
               room in the pipe for the message.  */
            status =  NU_PIPE_FULL;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpPipe(RT_PROF_SEND_TO_FRONT_OF_PIPE,pipe,RT_PROF_FAIL);
#endif /* INCLUDE_PROVIEW */

        }
    }
    else
    {

        /* Determine if a task is waiting on an empty pipe.  */
        if ((pipe -> pi_suspension_list) && (pipe -> pi_messages == 0))
        {

            /* Task is waiting on pipe for a message.  */

            /* Decrement the number of tasks waiting on pipe.  */
            pipe -> pi_tasks_waiting--;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpPipe(RT_PROF_SEND_TO_FRONT_OF_PIPE,pipe,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

            /* Remove the first suspended block from the list.  */
            suspend_ptr =  pipe -> pi_suspension_list;
            CSC_Remove_From_List((CS_NODE **) &(pipe -> pi_suspension_list),
                                          &(suspend_ptr -> pi_suspend_link));

            /* Setup the source and destination pointers.  */
            source =       (BYTE_PTR) message;
            destination =  suspend_ptr -> pi_message_area;

            /* Initialize the return status.  */
            suspend_ptr -> pi_return_status =  NU_SUCCESS;

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
            suspend_ptr -> pi_actual_size =  size;

            /* Wakeup the waiting task and check for preemption.  */
            preempt =
                TCC_Resume_Task((NU_TASK *) suspend_ptr -> pi_suspended_task,
                                                     NU_PIPE_SUSPEND);

            /* Determine if preemption needs to take place. */
            if (preempt)

                 /* Transfer control to the system if the resumed task function
                   detects a preemption condition.  */
                TCT_Control_To_System();
        }
        else
        {

            /* There is enough room in the pipe and no task is waiting.  */

            /* Setup the source pointer.  */
            source =       (BYTE_PTR) message;
            destination =  pipe -> pi_read;

            /* Process according to the type of message supported.  */
            if (pipe -> pi_fixed_size)
            {

                /* Fixed-size message pipe.  */

                /* Determine if the read pointer is at the top of the pipe
                   area.  */
                if (destination == pipe -> pi_start)

                    /* Prepare to place the message in the lower part of the
                       pipe area.  */
                    destination =  pipe -> pi_end - size;
                else

                    /* Backup the length of the message from the current
                       read pointer.  */
                    destination =  destination - size;

                /* Adjust the actual read pointer before the copy is done.  */
                pipe -> pi_read =  destination;

                /* Copy the message into the pipe area.  */
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

                /* Variable-length message pipe.  */

                /* Calculate the number of bytes remaining from the write
                   pointer to the bottom of the pipe.  */
                copy_size =  destination - pipe -> pi_start;

                /* Determine if part of the message needs to be placed at the
                   bottom of the pipe area.  */
                if (copy_size < (size + i))

                    /* Compute the starting location for the message.  */
                    destination =  pipe -> pi_end - ((size + i) - copy_size);
                else

                    /* Compute the starting location for the message.  */
                    destination =  destination - (size + i);

                /* Adjust the actual pipe read pointer also.  */
                pipe -> pi_read =  destination;

                /* Place message size in first location.  */
                *((UNSIGNED *) destination) =  size;
                destination =  destination + sizeof(UNSIGNED);

                /* Check for a wrap-around condition on the pipe.  */
                if (destination >= pipe -> pi_end)

                    /* Wrap the write pointer back to the top of the pipe
                       area.  */
                    destination =  pipe -> pi_start;

                /* Decrement the number of bytes remaining for this
                   extra word of overhead.  */
                pipe -> pi_available =  pipe -> pi_available -
                                                        sizeof(UNSIGNED);

                /* Calculate the number of bytes remaining from the write
                   pointer to the bottom of the pipe.  */
                copy_size =  pipe -> pi_end - destination;

                /* Determine if the message needs to be wrapped around the
                   edge of the pipe area.  */
                if (copy_size >= (size + pad))
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
                    destination =  pipe -> pi_start;

                    /* Determine if there is anything left to copy.  */
                    if (size > copy_size)
                    {
                        /* Yes, there is something to copy.  */
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
                /* Decrement the number of available bytes.  */
                pipe -> pi_available =  pipe -> pi_available - pad;
            }

            /* Decrement the number of available bytes.  */
            pipe -> pi_available =  pipe -> pi_available - size;

            /* Increment the number of messages in the pipe.  */
            pipe -> pi_messages++;

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpPipe(RT_PROF_SEND_TO_FRONT_OF_PIPE,pipe,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

        }
    }

    /* Release protection against access to the pipe.  */
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
/*      PIS_Broadcast_To_Pipe                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function sends a message to all tasks waiting for a message */
/*      from the specified pipe.  If there are no tasks waiting for a    */
/*      message the service performs like a standard send request.       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      PISE_Broadcast_To_Pipe              Error checking shell         */
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
/*      TCT_System_Protect                  Protect pipe                 */
/*      TCT_Unprotect                       Release protection           */
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
/*      NU_SUCCESS                          If service is successful     */
/*      NU_PIPE_FULL                        If pipe is currently full    */
/*      NU_TIMEOUT                          If timeout on service expires*/
/*      NU_PIPE_DELETED                     If pipe was deleted during   */
/*                                            suspension                 */
/*      NU_PIPE_RESET                       If pipe was reset during     */
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
STATUS  PIS_Broadcast_To_Pipe(NU_PIPE *pipe_ptr, VOID *message,
                                        UNSIGNED size, UNSIGNED suspend)
{

R1 PI_PCB      *pipe;                       /* Pipe control block ptr    */
PI_SUSPEND      suspend_block;              /* Allocate suspension block */
PI_SUSPEND     *suspend_ptr;                /* Pointer to suspend block  */
R2 BYTE_PTR     source;                     /* Pointer to source         */
R3 BYTE_PTR     destination;                /* Pointer to destination    */
UNSIGNED        copy_size;                  /* Partial copy size         */
UNSIGNED        pad =  0;                   /* Number of pad bytes       */
R4 INT          i;                          /* Working counter           */
TC_TCB         *task;                       /* Task pointer              */
STATUS          preempt;                    /* Preempt flag              */
STATUS          status;                     /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pipe pointer into internal pointer.  */
    pipe =  (PI_PCB *) pipe_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_BROADCAST_TO_PIPE_ID, (UNSIGNED) pipe,
                                        (UNSIGNED) message, (UNSIGNED) size);

#endif

    /* Initialize the status as successful.  */
    status =  NU_SUCCESS;

    /* Protect against simultaneous access to the pipe.  */
    TCT_System_Protect();

    /* Determine if an extra word of overhead needs to be added to the
       calculation.  */
    if (pipe -> pi_fixed_size)

        /* No overhead.  */
        i =  0;
    else
    {

        /* Variable messages have one additional word of overhead.  */
        i =  sizeof(UNSIGNED);

        /* Calculate the number of pad bytes necessary to keep the pipe
           write pointer on an UNSIGNED data element alignment.  */
        pad =  (((size + sizeof(UNSIGNED) - 1)/sizeof(UNSIGNED)) *
                sizeof(UNSIGNED)) - size;

        /* Insure that padding is included in the overhead.  */
        i =  i + ((INT) pad);

        /* Make special check to see if a suspension needs to be
           forced for a variable length message.  */
        if ((pipe -> pi_suspension_list) && (pipe -> pi_messages))
        {

            /* Pickup task control block pointer.  */
            task =  (TC_TCB *) TCT_Current_Thread();

            /* Now we know that there are other task(s) are suspended trying
               to send a variable length message.  Determine whether or not
               a suspension should be forced.  */
            if ((pipe -> pi_fifo_suspend) ||
                (suspend == NU_NO_SUSPEND) ||
                ((pipe -> pi_suspension_list) -> pi_suspend_link.cs_priority <=
                                                    TCC_Task_Priority(task)))

                /* Bump the computed size to avoid placing the new variable
                   length message ahead of the suspended tasks.  */
                i =  (INT) pipe -> pi_available;
        }
    }

    /* Determine if there is enough room in the pipe for the message.  */
    if (pipe -> pi_available < (size + i))
    {

        /* pipe does not have room for the message.  Determine if
           suspension is required.  */
        if (suspend)
        {

            /* Suspension is requested.   */

            /* Increment the number of tasks waiting.  */
            pipe -> pi_tasks_waiting++;

#ifdef INCLUDE_PROVIEW
            _RTProf_DumpPipe(RT_PROF_BROADCAST_TO_PIPE,pipe,RT_PROF_WAIT);
#endif /* INCLUDE_PROVIEW */

            /* Setup the suspend block and suspend the calling task.  */
            suspend_ptr =  &suspend_block;
            suspend_ptr -> pi_pipe =                     pipe;
            suspend_ptr -> pi_suspend_link.cs_next =     NU_NULL;
            suspend_ptr -> pi_suspend_link.cs_previous = NU_NULL;
            suspend_ptr -> pi_message_area =             (BYTE_PTR) message;
            suspend_ptr -> pi_message_size =             size;
            task =                            (TC_TCB *) TCT_Current_Thread();
            suspend_ptr -> pi_suspended_task =           task;

            /* Determine if priority or FIFO suspension is associated with the
               pipe.  */
            if (pipe -> pi_fifo_suspend)
            {

                /* FIFO suspension is required.  Link the suspend block into
                   the list of suspended tasks on this pipe.  */
                CSC_Place_On_List((CS_NODE **) &(pipe -> pi_suspension_list),
                                        &(suspend_ptr -> pi_suspend_link));
            }
            else
            {

                /* Get the priority of the current thread so the suspend block
                   can be placed in the appropriate place.  */
                suspend_ptr -> pi_suspend_link.cs_priority =
                                                    TCC_Task_Priority(task);

                CSC_Priority_Place_On_List((CS_NODE **)
                                &(pipe -> pi_suspension_list),
                                        &(suspend_ptr -> pi_suspend_link));
            }

            /* Finally, suspend the calling task. Note that the suspension call
               automatically clears the protection on the pipe.  */
            TCC_Suspend_Task((NU_TASK *) task, NU_PIPE_SUSPEND,
                                        PIC_Cleanup, suspend_ptr, suspend);

            /* Pickup the return status.  */
            status =  suspend_ptr -> pi_return_status;
        }
        else
        {

            /* Return a status of NU_PIPE_FULL because there is no
               room in the pipe for the message.  */
            status =  NU_PIPE_FULL;


#ifdef INCLUDE_PROVIEW
            _RTProf_DumpPipe(RT_PROF_BROADCAST_TO_PIPE,pipe,RT_PROF_FAIL);
#endif /* INCLUDE_PROVIEW */

        }
    }
    else
    {

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpPipe(RT_PROF_BROADCAST_TO_PIPE,pipe,RT_PROF_OK);
#endif /* INCLUDE_PROVIEW */

        /* Determine if a task is waiting on an empty pipe.  */
        if ((pipe -> pi_suspension_list) && (pipe -> pi_messages == 0))
        {

            /* Yes, one or more tasks are waiting for a message from this
               pipe.  */
            preempt =  0;
            do
            {

                /* Decrement the number of tasks waiting on pipe.  */
                pipe -> pi_tasks_waiting--;

                /* Remove the first suspended block from the list.  */
                suspend_ptr =  pipe -> pi_suspension_list;
                CSC_Remove_From_List((CS_NODE **)
                                &(pipe -> pi_suspension_list),
                                          &(suspend_ptr -> pi_suspend_link));

                /* Setup the source and destination pointers.  */
                source =       (BYTE_PTR) message;
                destination =  suspend_ptr -> pi_message_area;

                /* Initialize the return status.  */
                suspend_ptr -> pi_return_status =  NU_SUCCESS;

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
                suspend_ptr -> pi_actual_size =  size;

                /* Wakeup the waiting task and check for preemption.  */
                preempt =  preempt |
                 TCC_Resume_Task((NU_TASK *) suspend_ptr -> pi_suspended_task,
                                                         NU_PIPE_SUSPEND);

                /* Move the suspend pointer to the next node, which is now
                   at the head of the list.  */
                suspend_ptr =  pipe -> pi_suspension_list;
            } while (suspend_ptr);

            /* Determine if preemption needs to take place. */
            if (preempt)

                 /* Transfer control to the system if the resumed task function
                   detects a preemption condition.  */
                TCT_Control_To_System();
        }
        else
        {

            /* There is enough room in the pipe and no task is waiting.  */

            /* Setup the source pointer.  */
            source =       (BYTE_PTR) message;
            destination =  pipe -> pi_write;

            /* Process according to the type of message supported.  */
            if (pipe -> pi_fixed_size)
            {

                /* Fixed-size messages are supported by this pipe.  */

                /* Loop to copy the message into the pipe area.  */
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
                   check for pipe wrap-around conditions.  */

                /* Place message size in first location.  */
                *((UNSIGNED *) destination) =  size;
                destination =  destination + sizeof(UNSIGNED);

                /* Check for a wrap-around condition on the pipe.  */
                if (destination >= pipe -> pi_end)

                    /* Wrap the write pointer back to the top of the pipe
                       area.  */
                    destination =  pipe -> pi_start;

                /* Decrement the number of bytes remaining for this
                   extra word of overhead.  */
                pipe -> pi_available =  pipe -> pi_available -
                                                        sizeof(UNSIGNED);

                /* Calculate the number of bytes remaining from the write
                   pointer to the bottom of the pipe.  */
                copy_size =  pipe -> pi_end - destination;

                /* Determine if the message needs to be wrapped around the
                   edge of the pipe area.  */
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
                    destination =  pipe -> pi_start;
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
            if (destination >= pipe -> pi_end)

                /* Move the write pointer to the top of the pipe area.  */
                destination =  pipe -> pi_start;

            /* Determine if the pipe supports variable-length messages.  If
               so, pad bytes are needed to keep UNSIGNED alignment.  */
            if (pad)
            {

                /* Variable-size message.  Add pad bytes to the write
                   pointer.  */

                /* Calculate the number of bytes remaining from the write
                   pointer to the bottom of the pipe.  */
                copy_size =  pipe -> pi_end - destination;

                /* If there is not enough room at the bottom of the pipe, the
                   pad bytes must be wrapped around to the top.  */
                if (copy_size <= pad)

                    /* Move write pointer to the top of the pipe and make the
                       necessary adjustment.  */
                    destination =  pipe -> pi_start + (pad - copy_size);
                else

                    /* There is enough room in the pipe to simply add the
                       the pad bytes to the write pointer.  */
                    destination =  destination + pad;

                /* Decrement the number of available bytes.  */
                pipe -> pi_available =  pipe -> pi_available - pad;
            }

            /* Update the actual write pointer.  */
            pipe -> pi_write =  destination;

            /* Decrement the number of available bytes.  */
            pipe -> pi_available =  pipe -> pi_available - size;

            /* Increment the number of messages in the pipe.  */
            pipe -> pi_messages++;
        }
    }

    /* Release protection against access to the pipe.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the completion status.  */
    return(status);
}







