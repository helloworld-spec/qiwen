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
/*      inc.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      IN - Initialization                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains initialization and setup routines associated  */
/*      with the initialization component.                               */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      INC_Initialize                      Common system initialization */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      in_extr.h                           Initialization functions     */
/*      er_extr.h                           Error handling function      */
/*      hi_extr.h                           History functions            */
/*      tc_extr.h                           Thread Control functions     */
/*      mb_extr.h                           Mailbox functions            */
/*      qu_extr.h                           Queue functions              */
/*      pi_extr.h                           Pipe functions               */
/*      sm_extr.h                           Semaphore functions          */
/*      ev_extr.h                           Event group functions        */
/*      pm_extr.h                           Partition memory functions   */
/*      dm_extr.h                           Dynamic memory functions     */
/*      tm_extr.h                           Timer functions              */
/*      io_extr.h                           I/O Driver functions         */
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
/*      03-20-1998      Moved the INC_Initialize_State                   */
/*                      define values into their own                     */
/*                      in_defs.h include file as part                   */
/*                      of SPR455. This creates                          */
/*                      version 1.2a.                                    */
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "in_defs.h"                 /* Initialization defines    */
#include        "in_extr.h"                 /* Initialization functions  */
#include        "hi_extr.h"                 /* History functions         */
#include        "er_extr.h"                 /* Error handling function   */
#include        "tc_extr.h"                 /* Thread Control functions  */
#include        "mb_extr.h"                 /* Mailbox functions         */
#include        "qu_extr.h"                 /* Queue functions           */
#include        "pi_extr.h"                 /* Pipe functions            */
#include        "sm_extr.h"                 /* Semaphore functions       */
#include        "ev_extr.h"                 /* Event group functions     */
#include        "pm_extr.h"                 /* Partition memory functions*/
#include        "dm_extr.h"                 /* Dynamic memory functions  */
#include        "tm_extr.h"                 /* Timer functions           */
#include        "io_extr.h"                 /* I/O Driver functions      */


/* Define global variable that contains the state of initialization.  This
   flag is for information use only.  */

INT             INC_Initialize_State;


/* Define external functions that access the release and license
   information.  */

CHAR    *RLC_Release_Information(VOID);
CHAR    *LIC_License_Information(VOID);

#if defined(NU_MODULE_SUPPORT) && (NU_MODULE_SUPPORT > 0)

/* Prototypes for Module Initialization funcitons */
STATUS MRC_Initialize(VOID);
STATUS MSC_Initialize(VOID);

#endif

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      INC_Initialize                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is the main initialization function of the system. */
/*      All components are initialized by this function.  After system   */
/*      initialization is complete, the Application_Initialize routine   */
/*      is called.  After all initialization is complete, this function  */
/*      calls TCT_Schedule to start scheduling tasks.                    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      INT_Initialize                      Target dependent initialize  */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      Application_Initialize              Application initialize       */
/*      RLC_Release_Information             Release information          */
/*      LIC_License_Information             License information          */
/*      ERI_Initialize                      Error handling initialize    */
/*      HII_Initialize                      History initialization       */
/*      TCI_Initialize                      Thread control initialize    */
/*      MBI_Initialize                      Mailbox initialize           */
/*      QUI_Initialize                      Queue initialize             */
/*      PII_Initialize                      Pipe initialize              */
/*      SMI_Initialize                      Semaphore initialize         */
/*      EVI_Initialize                      Event flag initialize        */
/*      PMI_Initialize                      Partition memory initialize  */
/*      DMI_Initialize                      Dynamic memory initialize    */
/*      TMI_Initialize                      Timer initialize             */
/*      IOI_Initialize                      I/O Driver initialize        */
/*      MRC_Initialize                      Memory Region initialize     */
/*      MSC_Initialize                      Module Support initialize    */
/*      TCT_Schedule                        Thread scheduling loop       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      first_available_memory              Pointer to available memory  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
VOID  INC_Initialize(VOID  *first_available_memory)
{

    /* Indicate that initialization is starting.  */
    INC_Initialize_State =  INC_START_INITIALIZE;
    
    /* Call release information function.  */
    RLC_Release_Information();

    /* Call license information function.  */
    LIC_License_Information();

    /* Initialize the Error handling (ER) component.  */
    ERI_Initialize();

    /* Initialize the History (HI) component.  */
    HII_Initialize();

#if defined(NU_MODULE_SUPPORT) && (NU_MODULE_SUPPORT > 0)

    MRC_Initialize();                /* Initialize Memory Region component */
    MSC_Initialize();                /* Initialize Module Support component */

#endif

    /* Initialize the Thread Control (TC) component.  */
    TCI_Initialize();

    /* Initialize the Mailbox (MB) component. */
    MBI_Initialize();

    /* Initialize the Queue (QU) component. */
    QUI_Initialize();

    /* Initialize the Pipe (PI) component. */
    PII_Initialize();

    /* Initialize the Semaphore (SM) component. */
    SMI_Initialize();

    /* Initialize the Event Group (EV) component.  */
    EVI_Initialize();

    /* Initialize the Partition memory (PM) component.  */
    PMI_Initialize();

    /* Initialize the Dynamic memory (DM) component.  */
    DMI_Initialize();

    /* Initialize the Timer (TM) component.  */
    TMI_Initialize();

    /* Initialize the I/O Driver (IO) component.  */
    IOI_Initialize();

    /* Invoke the application-supplied initialization function.  */
    Application_Initialize(first_available_memory);

    /* Indicate that initialization is finished.  */
    INC_Initialize_State =  INC_END_INITIALIZE;
    /* Start scheduling threads of execution.  */
    TCT_Schedule();
}



