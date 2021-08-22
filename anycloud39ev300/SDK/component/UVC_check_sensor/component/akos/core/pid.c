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
/*      pid.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PI - Pipe Management                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      pipe management component.                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      PID_Created_Pipe_List               Pointer to the linked-list   */
/*                                            of created pipes           */
/*      PID_Total_Pipes                     Total number of created      */
/*                                            pipes                      */
/*      PID_List_Protect                    Pipe list protection         */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      pi_defs.h                           Pipe Management constants    */
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

#include        "pi_defs.h"                 /* Pipe constants            */


/* PID_Created_Pipes_List is the head pointer of the linked list of
   created pipes.  If the list is NU_NULL, there are no pipes
   created.  */

CS_NODE        *PID_Created_Pipes_List;


/* PID_Total_Pipes contains the number of currently created
   pipes.  */

UNSIGNED        PID_Total_Pipes;


/* PID_List_Protect is a list protection structure used to block any other
   thread from access to the created pipe list.  */

TC_PROTECT      PID_List_Protect;






