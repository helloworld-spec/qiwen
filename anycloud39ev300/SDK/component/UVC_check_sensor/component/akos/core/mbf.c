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
/*      mbf.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      MB - Mailbox Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains routines to obtain facts about the Mailbox    */
/*      management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      MBF_Established_Mailboxes           Number of created mailboxes  */
/*      MBF_Mailbox_Pointers                Build mailbox pointer list   */
/*      MBF_Mailbox_Information             Retrieve mailbox information */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      mb_extr.h                           Mailbox functions            */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1994      Initial version of mailbox fact                  */
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
#include        "mb_extr.h"                 /* Mailbox functions         */
#include        "hi_extr.h"                 /* History functions         */


/* Define external inner-component global data references.  */

extern CS_NODE         *MBD_Created_Mailboxes_List;
extern UNSIGNED         MBD_Total_Mailboxes;
extern TC_PROTECT       MBD_List_Protect;


/* Define internal component function prototypes.  */

VOID    MBC_Cleanup(VOID *information);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBF_Established_Mailboxes                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the current number of established          */
/*      mailboxes.  Mailboxes previously deleted are no longer           */
/*      considered established.                                          */
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
/*      MBD_Total_Mailboxes                 Number of established        */
/*                                            mailboxes                  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  MBF_Established_Mailboxes(VOID)
{


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Return the number of established mailboxes.  */
    return(MBD_Total_Mailboxes);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBF_Mailbox_Pointers                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function builds a list of mailbox pointers, starting at the */
/*      specified location.  The number of mailbox pointers placed in    */
/*      the list is equivalent to the total number of mailboxes or the   */
/*      maximum number of pointers specified in the call.                */
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
/*      pointers                            Number of mailboxes placed   */
/*                                            in the list                */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Corrected pointer retrieval                      */
/*                      loop, resulting in version 1.0a                  */
/*      08-09-1993      Verified version 1.0a                            */
/*      03-01-1994      Changed function interface to                    */
/*                      match the prototype,                             */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  MBF_Mailbox_Pointers(NU_MAILBOX **pointer_list,
                                                UNSIGNED maximum_pointers)
{
CS_NODE         *node_ptr;                  /* Pointer to each MCB       */
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

    /* Protect against access to the list of created mailboxes.  */
    TCT_Protect(&MBD_List_Protect);

    /* Loop until all mailbox pointers are in the list or until the maximum
       list size is reached.  */
    node_ptr =  MBD_Created_Mailboxes_List;
    while ((node_ptr) && (pointers < maximum_pointers))
    {

        /* Place the node into the destination list.  */
        *pointer_list++ =  (NU_MAILBOX *) node_ptr;

        /* Increment the pointers variable.  */
        pointers++;

        /* Position the node pointer to the next node.  */
        node_ptr =  node_ptr -> cs_next;

        /* Determine if the pointer is at the head of the list.  */
        if (node_ptr == MBD_Created_Mailboxes_List)

            /* The list search is complete.  */
            node_ptr =  NU_NULL;
    }

    /* Release protection against access to the list of created mailboxes.  */
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
/*      MBF_Mailbox_Information                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns information about the specified mailbox.   */
/*      However, if the supplied mailbox pointer is invalid, the         */
/*      function simply returns an error status.                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect mailbox              */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      mailbox_ptr                         Pointer to the mailbox       */
/*      name                                Destination for the name     */
/*      suspend_type                        Destination for the type of  */
/*                                            suspension                 */
/*      message_present                     Destination for the message  */
/*                                            present flag               */
/*      tasks_waiting                       Destination for the tasks    */
/*                                            waiting count              */
/*      first_task                          Destination for the pointer  */
/*                                            to the first task waiting  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If a valid mailbox pointer   */
/*                                            is supplied                */
/*      NU_INVALID_MAILBOX                  If mailbox pointer invalid   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Changed function interface to                    */
/*                      match the prototype,                             */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      11-18-1996      Corrected SPR220.                                */
/*                                                                       */
/*************************************************************************/
STATUS  MBF_Mailbox_Information(NU_MAILBOX *mailbox_ptr, CHAR *name,
                  OPTION *suspend_type, DATA_ELEMENT *message_present,
                  UNSIGNED *tasks_waiting, NU_TASK **first_task)
{

MB_MCB         *mailbox;                    /* Mailbox control block ptr */
int             i;                          /* Working integer variable  */
STATUS          completion;                 /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input mailbox pointer into internal pointer.  */
    mailbox =  (MB_MCB *) mailbox_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Determine if this mailbox id is valid.  */
    if ((mailbox != NU_NULL) && (mailbox -> mb_id == MB_MAILBOX_ID))
    {

        /* Setup protection of the mailbox.  */
        TCT_System_Protect();

        /* The mailbox pointer is valid.  Reflect this in the completion
           status and fill in the actual information.  */
        completion =  NU_SUCCESS;

        /* Copy the mailbox's name.  */
        for (i = 0; i < NU_MAX_NAME; i++)
            *name++ =  mailbox -> mb_name[i];

        /* Determine the suspension type.  */
        if (mailbox -> mb_fifo_suspend)
            *suspend_type =          NU_FIFO;
        else
            *suspend_type =          NU_PRIORITY;

        /* Indicate whether or not there is a message in the mailbox.  */
        *message_present =  mailbox -> mb_message_present;

        /* Retrieve the number of tasks waiting and the pointer to the
           first task waiting.  */
        *tasks_waiting =  mailbox -> mb_tasks_waiting;
        if (mailbox -> mb_suspension_list)

            /* There is a task waiting.  */
            *first_task = (NU_TASK *)
                (mailbox -> mb_suspension_list) -> mb_suspended_task;
        else

            /* There are no tasks waiting.  */
            *first_task =  NU_NULL;

        /* Release protection.  */
        TCT_Unprotect();
    }
    else

        /* Indicate that the mailbox pointer is invalid.   */
        completion =  NU_INVALID_MAILBOX;

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the appropriate completion status.  */
    return(completion);
}





