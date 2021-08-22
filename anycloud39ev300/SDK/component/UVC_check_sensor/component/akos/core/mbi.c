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
/*      mbi.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      MB - Mailbox Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the initialization routine for the mailbox    */
/*      management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      MBI_Initialize                      Mailbox Management Initialize*/
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      mb_defs.h                           Mailbox component constants  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Update file copyright informaion,                */
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


#include        "mb_defs.h"                 /* Mailbox constants         */
#include        "mb_extr.h"                 /* Mailbox interfaces        */


/* Define external inner-component global data references.  */

extern CS_NODE         *MBD_Created_Mailboxes_List;
extern UNSIGNED         MBD_Total_Mailboxes;
extern TC_PROTECT       MBD_List_Protect;


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      MBI_Initialize                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the data structures that control the   */
/*      operation of the Mailbox component (MB).  There are no mailboxes */
/*      initially.  This routine must be called from Supervisor mode in  */
/*      Supervisor/User mode switched kernels.                           */
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
/*      MBD_Created_Mailboxes_List          List of created mailboxes    */
/*      MBD_Total_Mailboxes                 Number of created mailboxes  */
/*      MBD_List_Protect                    Protection for mailbox list  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
VOID  MBI_Initialize(VOID)
{
    /* Initialize the created mailbox list to NU_NULL.  */
    MBD_Created_Mailboxes_List =  NU_NULL;

    /* Initialize the total number of created mailboxes to 0.  */
    MBD_Total_Mailboxes =  0;

    /* Initialize the list protection structure.  */
    MBD_List_Protect.tc_tcb_pointer =      NU_NULL;
}






