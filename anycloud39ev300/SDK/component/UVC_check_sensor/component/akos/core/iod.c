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
/*      iod.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      IO - Input/Output Driver Management                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      Input/Output Driver Management component.                        */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      IOD_Created_Drivers_List            Pointer to the linked-list   */
/*                                            of created I/O drivers     */
/*      IOD_Total_Drivers                   Total number of created      */
/*                                            I/O drivers                */
/*      IOD_List_Protect                    I/O driver list protection   */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      io_defs.h                           I/O Driver Management consts */
/*      tc_defs.h                           Thread Control constants     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Updated copyright notice,                        */
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
#define         NU_SOURCE_FILE

#include        "io_defs.h"                 /* I/O Driver constants      */
#include        "tc_defs.h"                 /* Thread control constants  */


/* IOD_Created_Drivers_List is the head pointer of the linked list of
   created I/O drivers.  If the list is NU_NULL, there are no I/O drivers
   created.  */

CS_NODE        *IOD_Created_Drivers_List;


/* IOD_Total_Drivers contains the number of currently created
   I/O drivers.  */

UNSIGNED        IOD_Total_Drivers;


/* IOD_List_Protect is a list protection structure used to block any other
   thread from access to the created drivers list.  */

TC_PROTECT      IOD_List_Protect;






