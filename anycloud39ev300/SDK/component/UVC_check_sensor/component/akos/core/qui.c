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
/*      qui.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      QU - Queue Management                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the initialization routine for the queue      */
/*      management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      QUI_Initialize                      Queue Management Initialize  */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      qu_defs.h                           Queue component constants    */
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
#include        "qu_extr.h"                 /* Queue interfaces          */


/* Define external inner-component global data references.  */

extern CS_NODE         *QUD_Created_Queues_List;
extern UNSIGNED         QUD_Total_Queues;
extern TC_PROTECT       QUD_List_Protect;


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      QUI_Initialize                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the data structures that control the   */
/*      operation of the Queue component (QU).  There are no queues      */
/*      initially.  This routine must be called from Supervisor mode in  */
/*      Supervisor/User mode switching kernels.                          */
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
/*      QUD_Created_Queues_List             List of created queues       */
/*      QUD_Total_Queues                    Number of created queues     */
/*      QUD_List_Protect                    Protection for queue list    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
VOID  QUI_Initialize(VOID)
{

    /* Initialize the created queue list to NU_NULL.  */
    QUD_Created_Queues_List =  NU_NULL;

    /* Initialize the total number of created queues to 0.  */
    QUD_Total_Queues =  0;

    /* Initialize the list protection structure.  */
    QUD_List_Protect.tc_tcb_pointer =      NU_NULL;
}





