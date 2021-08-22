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
/*      hii.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      HI - History Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the initialization routine for the History    */
/*      Management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      HII_Initialize                      History Management Initialize*/
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      hi_defs.h                           History component constants  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Replaced void with VOID,                         */
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
#include        "hi_extr.h"                 /* History interfaces        */


/* Define external inner-component global data references.  */

extern INT              HID_History_Enable;
extern INT              HID_Write_Index;
extern INT              HID_Read_Index;
extern INT              HID_Entry_Count;
extern TC_PROTECT       HID_History_Protect;


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      HII_Initialize                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the data structures that control the   */
/*      operation of the History component (HI).  This routine must be   */
/*      called from Supervisor mode in Supervisor/User mode switching    */
/*      kernels.                                                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      INC_Initialize                      System initialization        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      HID_Current_Index                   Next available table index   */
/*      HID_Entry_Count                     Counter of entries in table  */
/*      HID_Table_Protect                   History table protection     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Replaced void with VOID,                         */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  HII_Initialize(VOID)
{
    /* Initialize the history enable flag to false.  */
    HID_History_Enable =  NU_FALSE;

    /* Initialize the starting indices to the first entry.  */
    HID_Write_Index =  0;
    HID_Read_Index =  0;

    /* Initialize the entry counter to 0.  */
    HID_Entry_Count =  0;

    /* Initialize the history protection structure.  */
    HID_History_Protect.tc_tcb_pointer =      NU_NULL;
}






