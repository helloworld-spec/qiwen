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
/*      pmd.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PM - Partition Memory Management                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      Partition Memory Management component.                           */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      PMD_Created_Pools_List              Pointer to the linked-list   */
/*                                            of created partition pools */
/*      PMD_Total_Pools                     Total number of created      */
/*                                            partition pools            */
/*      PMD_List_Protect                    Partition pool list protect  */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      pm_defs.h                           Partition Management constant*/
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
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

#include        "pm_defs.h"                 /* Partition constants       */


/* PMD_Created_Pools_List is the head pointer of the linked list of
   created partition pools.  If the list is NU_NULL, there are no partition
   pools created.  */

CS_NODE        *PMD_Created_Pools_List;


/* PMD_Total_Pools contains the number of currently created
   partition pools.  */

UNSIGNED        PMD_Total_Pools;


/* PMD_List_Protect is a list protection structure used to block any other
   thread from access to the created partition pool list.  */

TC_PROTECT      PMD_List_Protect;





