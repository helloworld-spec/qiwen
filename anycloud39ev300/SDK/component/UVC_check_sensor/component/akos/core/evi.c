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
/*      evi.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      EV - Event Group Management                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the initialization routine for the Event      */
/*      Group Management component.                                      */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      EVI_Initialize                      Event Group Management Init  */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      ev_defs.h                           Event Group component const. */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*            DATE                    REMARKS                            */
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


#include        "ev_defs.h"                 /* Event Group constants     */
#include        "ev_extr.h"                 /* Event Group interfaces    */


/* Define external inner-component global data references.  */

extern CS_NODE         *EVD_Created_Event_Groups_List;
extern UNSIGNED         EVD_Total_Event_Groups;
extern TC_PROTECT       EVD_List_Protect;


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      EVI_Initialize                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the data structures that control the   */
/*      operation of the Event Group component (EV).  There are no       */
/*      event groups initially.  This routine must be called from        */
/*      Supervisor mode in Supervisor/User mode switching kernels.       */
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
/*      EVD_Created_Event_Groups_List       List of created event groups */
/*      EVD_Total_Event_Groups              Number of created event      */
/*                                            groups                     */
/*      EVD_List_Protect                    Protection for event group   */
/*                                            list                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
VOID  EVI_Initialize(VOID)
{
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Initialize the created event group list to NU_NULL.  */
    EVD_Created_Event_Groups_List =  NU_NULL;

    /* Initialize the total number of created event groups to 0.  */
    EVD_Total_Event_Groups =  0;

    /* Initialize the list protection structure.  */
    EVD_List_Protect.tc_tcb_pointer =      NU_NULL;

    /* Return to user mode */
    NU_USER_MODE();
}






