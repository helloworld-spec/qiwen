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
/*      qud.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      QU - Queue Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      queue management component.                                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      QUD_Created_Queue_List              Pointer to the linked-list   */
/*                                            of created queues          */
/*      QUD_Total_Queues                    Total number of created      */
/*                                            queues                     */
/*      QUD_List_Protect                    Queue list protection        */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      qu_defs.h                           Queue Management constants   */
/*      tc_defs.h                           Thread Control constants     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified copyright, resulting in                 */
/*                      version 1.1                                      */
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

#include        "qu_defs.h"                 /* Queue constants           */


/* QUD_Created_Queues_List is the head pointer of the linked list of
   created queues.  If the list is NU_NULL, there are no queues
   created.  */

CS_NODE        *QUD_Created_Queues_List;


/* QUD_Total_Queues contains the number of currently created
   queues.  */

UNSIGNED        QUD_Total_Queues;


/* QUD_List_Protect is a list protection structure used to block any other
   thread from access to the created queue list.  */

TC_PROTECT      QUD_List_Protect;





