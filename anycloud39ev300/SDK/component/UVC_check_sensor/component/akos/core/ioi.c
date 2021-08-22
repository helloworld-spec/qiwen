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
/*      ioi.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      IO - Input/Output Driver Management                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the initialization routine for the I/O Driver */
/*      Management component.                                            */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      IOI_Initialize                      I/O Driver Initialize        */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      io_defs.h                           I/O Driver component consts  */
/*      tc_defs.h                           Thread control constants     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Updated copyright notice,                        */
/*                      changed "void" to VOID,                          */
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


#include        "io_defs.h"                 /* I/O driver constants      */
#include        "io_extr.h"                 /* I/O driver interfaces     */
#include        "tc_defs.h"                 /* Thread control constants  */



/* Define external inner-component global data references.  */

extern CS_NODE         *IOD_Created_Drivers_List;
extern UNSIGNED         IOD_Total_Drivers;
extern TC_PROTECT       IOD_List_Protect;


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IOI_Initialize                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the data structures that control the   */
/*      operation of the I/O driver component (IO).  There are no I/O    */
/*      drivers initially.  This routine must be called from Supervisor  */
/*      mode in Supervisor/User mode switched kernels.                   */
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
/*      IOD_Created_Drivers_List            List of created I/O drivers  */
/*      IOD_Total_Drivers                   Number of created I/O drivers*/
/*      IOD_List_Protect                    Protection for driver list   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Updated copyright notice,                        */
/*                      changed "void" to "VOID",                        */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  IOI_Initialize(VOID)
{

    /* Initialize the created I/O driver list to NU_NULL.  */
    IOD_Created_Drivers_List =  NU_NULL;

    /* Initialize the total number of created I/O drivers to 0.  */
    IOD_Total_Drivers =  0;

    /* Initialize the list protection structure.  */
    IOD_List_Protect.tc_tcb_pointer =      NU_NULL;
}






