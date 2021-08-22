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
/*      hid.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      HI - History Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within the     */
/*      History Management component.                                    */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      HID_History_Enable                  History saving enabled flag  */
/*      HID_Write_Index                     Current write index into     */
/*                                            history table              */
/*      HID_Read_Index                      Current read index into      */
/*                                            history table              */
/*      HID_Entry_Count                     Number of entries in the     */
/*                                            table counter              */
/*      HID_History_Protect                 History protection           */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      hi_defs.h                           History Management constants */
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

#include        "hi_defs.h"                 /* History constants         */


/* HID_History_Enable is a flag that indicates whether or not history saving
   is enabled.  If this value is NU_FALSE, history saving is disabled.
   Otherwise, history saving is enabled.  */

INT     HID_History_Enable;


/* HID_Write_Index is the index of the next available entry in the history
   table.  */

INT     HID_Write_Index;


/* HID_Read_Index is the index of the oldest entry in the history table.  */

INT     HID_Read_Index;


/* HID_Entry_Count keeps track of the number of entries currently
   in the history table.  */

INT     HID_Entry_Count;


/* HID_History_Protect is a protection structure used to block any other
   thread from access to the history data structures.  */

TC_PROTECT      HID_History_Protect;




