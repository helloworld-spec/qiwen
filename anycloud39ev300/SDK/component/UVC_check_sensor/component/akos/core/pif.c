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
/*      pif.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PI - Pipe Management                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains routines to obtain facts about the Pipe       */
/*      management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      PIF_Established_Pipes               Number of created pipes      */
/*      PIF_Pipe_Information                Retrieve pipe information    */
/*      PIF_Pipe_Pointers                   Build pipe pointer list      */
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
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1994      Initial version of pipe fact                     */
/*                      service file, version 1.1                        */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      11-18-1996      Protected Informational service                  */
/*                      from NULL Control Block pointers                 */
/*                      creating 1.2a. (SPR220)                          */
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


/* Define external inner-component global data references.  */

extern CS_NODE         *PID_Created_Pipes_List;
extern UNSIGNED         PID_Total_Pipes;
extern TC_PROTECT       PID_List_Protect;



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PIF_Established_Pipes                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the current number of established          */
/*      pipes.  Pipes previously deleted are no longer considered        */
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
/*      PID_Total_Pipes                     Number of established        */
/*                                            pipes                      */
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
UNSIGNED  PIF_Established_Pipes(VOID)
{


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Return the number of established pipes.  */
    return(PID_Total_Pipes);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      PIF_Pipe_Pointers                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function builds a list of pipe pointers, starting at the    */
/*      specified location.  The number of pipe pointers placed in the   */
/*      list is equivalent to the total number of pipes or the maximum   */
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
/*      pointers                            Number of pipe pointers      */
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
UNSIGNED  PIF_Pipe_Pointers(NU_PIPE **pointer_list,UNSIGNED maximum_pointers)
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

    /* Protect against access to the list of created pipes.  */
    TCT_Protect(&PID_List_Protect);

    /* Loop until all pipe pointers are in the list or until the maximum
       list size is reached.  */
    node_ptr =  PID_Created_Pipes_List;
    while ((node_ptr) && (pointers < maximum_pointers))
    {

        /* Place the node into the destination list.  */
        *pointer_list++ =  (NU_PIPE *) node_ptr;

        /* Increment the pointers variable.  */
        pointers++;

        /* Position the node pointer to the next node.  */
        node_ptr =  node_ptr -> cs_next;

        /* Determine if the pointer is at the head of the list.  */
        if (node_ptr == PID_Created_Pipes_List)

            /* The list search is complete.  */
            node_ptr =  NU_NULL;
    }

    /* Release protection against access to the list of created pipes.  */
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
/*      PIF_Pipe_Information                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns information about the specified pipe.      */
/*      However, if the supplied pipe pointer is invalid, the            */
/*      function simply returns an error status.                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect pipe                 */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pipe_ptr                            Pointer to the pipe          */
/*      name                                Destination for the name     */
/*      start_address                       Destination for the start    */
/*                                            address of the pipe        */
/*      pipe_size                           Destination for pipe size    */
/*      available                           Destination for available    */
/*                                            room in pipe               */
/*      messages                            Destination for number of    */
/*                                            messages piped             */
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
/*      NU_SUCCESS                          If a valid pipe pointer      */
/*                                            is supplied                */
/*      NU_INVALID_PIPE                     If pipe pointer invalid      */
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
/*      11-18-1996      Corrected SPR220                                 */
/*                                                                       */
/*************************************************************************/
STATUS  PIF_Pipe_Information(NU_PIPE *pipe_ptr, CHAR *name,
                  VOID **start_address, UNSIGNED *pipe_size,
                  UNSIGNED *available, UNSIGNED *messages,
                  OPTION *message_type, UNSIGNED *message_size,
                  OPTION *suspend_type, UNSIGNED *tasks_waiting,
                  NU_TASK **first_task)
{

PI_PCB         *pipe;                       /* Pipe control block ptr    */
INT             i;                          /* Working integer variable  */
STATUS          completion;                 /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input pipe pointer into internal pointer.  */
    pipe =  (PI_PCB *) pipe_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Determine if this pipe id is valid.  */
    if ((pipe != NU_NULL) && (pipe -> pi_id == PI_PIPE_ID))
    {

        /* Setup protection of the pipe.  */
        TCT_System_Protect();

        /* The pipe pointer is valid.  Reflect this in the completion
           status and fill in the actual information.  */
        completion =  NU_SUCCESS;

        /* Copy the pipe's name.  */
        for (i = 0; i < NU_MAX_NAME; i++)
            *name++ =  pipe -> pi_name[i];

        /* Determine the suspension type.  */
        if (pipe -> pi_fifo_suspend)
            *suspend_type =  NU_FIFO;
        else
            *suspend_type =  NU_PRIORITY;

        /* Determine the message type.  */
        if (pipe -> pi_fixed_size)
            *message_type =  NU_FIXED_SIZE;
        else
            *message_type =  NU_VARIABLE_SIZE;

        /* Get various information about the pipe.  */
        *start_address =  (VOID *) pipe -> pi_start;
        *pipe_size =      pipe -> pi_pipe_size;
        *available =      pipe -> pi_available;
        *messages =       pipe -> pi_messages;
        *message_size =   pipe -> pi_message_size;

        /* Retrieve the number of tasks waiting and the pointer to the
           first task waiting.  */
        *tasks_waiting =  pipe -> pi_tasks_waiting;
        if (pipe -> pi_suspension_list)

            /* There is a task waiting.  */
            *first_task =  (NU_TASK *)
                (pipe -> pi_suspension_list) -> pi_suspended_task;
        else

            /* There are no tasks waiting.  */
            *first_task =  NU_NULL;

        /* Release protection of the pipe.  */
        TCT_Unprotect();
    }
    else

        /* Indicate that the pipe pointer is invalid.   */
        completion =  NU_INVALID_PIPE;

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the appropriate completion status.  */
    return(completion);
}






