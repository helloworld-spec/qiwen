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
/*      mb_extr.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      MB - Mailbox Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes of all functions          */
/*      accessible to other components.                                  */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      mb_defs.h                           Mailbox Management constants */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Moved include files outside of                   */
/*                      the file #ifndef to allow the                    */
/*                      use of actual data structures,                   */
/*                      changed the names of several                     */
/*                      mailboxes services to reflect                    */
/*                      the new file structure,                          */
/*                      modified function interface to                   */
/*                      exactly match the prototype,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/

#include        "mb_defs.h"                 /* Include MB constants      */


/* Check to see if the file has been included already.  */

#ifndef MB_EXTR
#define MB_EXTR


/*  Initialization functions.  */

VOID            MBI_Initialize(VOID);


/* Core error checking functions.  */

STATUS          MBCE_Create_Mailbox(NU_MAILBOX *mailbox_ptr, CHAR *name, 
                                                OPTION suspend_type);
STATUS          MBCE_Delete_Mailbox(NU_MAILBOX *mailbox_ptr);
STATUS          MBCE_Send_To_Mailbox(NU_MAILBOX *mailbox_ptr, VOID *message, 
                                                        UNSIGNED suspend);
STATUS          MBCE_Receive_From_Mailbox(NU_MAILBOX *mailbox_ptr, 
                                        VOID *message, UNSIGNED suspend);

/* Supplemental error checking functions.  */

STATUS          MBSE_Reset_Mailbox(NU_MAILBOX *mailbox_ptr);
STATUS          MBSE_Broadcast_To_Mailbox(NU_MAILBOX *mailbox_ptr, 
                                        VOID *message, UNSIGNED suspend);

/* Core processing functions.  */

STATUS          MBC_Create_Mailbox(NU_MAILBOX *mailbox_ptr, CHAR *name, 
                                                OPTION suspend_type);
STATUS          MBC_Delete_Mailbox(NU_MAILBOX *mailbox_ptr);
STATUS          MBC_Send_To_Mailbox(NU_MAILBOX *mailbox_ptr, VOID *message, 
                                                        UNSIGNED suspend);
STATUS          MBC_Receive_From_Mailbox(NU_MAILBOX *mailbox_ptr, 
                                        VOID *message, UNSIGNED suspend);

/* Supplemental mailbox functions.  */

STATUS          MBS_Reset_Mailbox(NU_MAILBOX *mailbox_ptr);
STATUS          MBS_Broadcast_To_Mailbox(NU_MAILBOX *mailbox_ptr, 
                                        VOID *message, UNSIGNED suspend);

/* Mailbox fact retrieval functions.  */

UNSIGNED        MBF_Established_Mailboxes(VOID);
STATUS          MBF_Mailbox_Information(NU_MAILBOX *mailbox_ptr, CHAR *name, 
                  OPTION *suspend_type, DATA_ELEMENT *message_present,
                  UNSIGNED *tasks_waiting, NU_TASK **first_task);
UNSIGNED        MBF_Mailbox_Pointers(NU_MAILBOX **pointer_list, 
                                                UNSIGNED maximum_pointers);

#endif





