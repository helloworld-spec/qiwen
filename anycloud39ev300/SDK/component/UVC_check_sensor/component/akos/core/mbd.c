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
/*      mbd.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      MB - Mailbox Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      mailbox management component.                                    */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      MBD_Created_Mailboxes_List          Pointer to the linked-list   */
/*                                            of created mailboxes       */
/*      MBD_Total_Mailboxes                 Total number of created      */
/*                                            mailboxes                  */
/*      MBD_List_Protect                    Mailbox list protection      */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      mb_defs.h                           Mailbox Management constants */
/*      tc_defs.h                           Thread Control constants     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified file header, resulting                  */
/*                      in version 1.1                                   */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE

#include        "mb_defs.h"                 /* Mailbox constants         */


/* MBD_Created_Mailboxes_List is the head pointer of the linked list of
   created mailboxes.  If the list is NU_NULL, there are no mailboxes
   created.  */

CS_NODE        *MBD_Created_Mailboxes_List;


/* MBD_Total_Mailboxes contains the number of currently created
   mailboxes.  */

UNSIGNED        MBD_Total_Mailboxes;


/* MBD_List_Protect is a list protection structure used to block any other
   thread from access to the created mailbox list.  */

TC_PROTECT      MBD_List_Protect;






