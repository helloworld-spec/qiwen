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
/*      evd.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      EV - Event Group Management                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      Event Group Management component.                                */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      EVD_Created_Event_Groups_List       Pointer to the linked-list   */
/*                                            of created event groups    */
/*      EVD_Total_Event_Groups              Total number of created      */
/*                                            event groups               */
/*      EVD_List_Protect                    Event Group list protection  */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      ev_defs.h                           Event Group Management const.*/
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified copyright, resulting in                 */
/*                        version 1.1                                    */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE

#include        "ev_defs.h"                 /* Event Group constants     */


/* EVD_Created_Event_Groups_List is the head pointer of the linked list of
   created event groups.  If the list is NU_NULL, there are no event groups
   created.  */

CS_NODE        *EVD_Created_Event_Groups_List;


/* EVD_Total_Event_Groups contains the number of currently created
   event groups.  */

UNSIGNED        EVD_Total_Event_Groups;


/* EVD_List_Protect is a list protection structure used to block any other
   thread from access to the created event group list.  */

TC_PROTECT      EVD_List_Protect;






