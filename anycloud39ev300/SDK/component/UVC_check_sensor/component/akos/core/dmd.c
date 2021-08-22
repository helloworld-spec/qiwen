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
/*      dmd.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DM - Dynamic Memory Management                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      Dynamic Memory Management component.                             */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      DMD_Created_Pools                   Pointer to the linked-list   */
/*                                            of created dynamic pools   */
/*      DMD_Total_Pools                     Total number of created      */
/*                                            dynamic pools              */
/*      DMD_List_Protect                    Dynamic pool list protect    */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      dm_defs.h                           Dynamic memory constants     */
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
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE

#include        "dm_defs.h"                 /* Dynamic memory constants  */


/* DMD_Created_Pools_List is the head pointer of the linked list of
   created dynamic memory pools.  If the list is NU_NULL, there are no
   dynamic memory pools created.  */

CS_NODE        *DMD_Created_Pools_List;


/* DMD_Total_Pools contains the number of currently created
   dynamic memory pools.  */

UNSIGNED        DMD_Total_Pools;


/* DMD_List_Protect is a list protection structure used to block any other
   thread from access to the created dynamic memory pool list.  */

TC_PROTECT      DMD_List_Protect;






