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
/*      dmi.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DM -  Dynamic Memory Management                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the initialization routine for the Dynamic    */
/*      Memory Management component.                                     */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      DMI_Initialize                      Dynamic memory initialize    */
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
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "dm_defs.h"                 /* Dynamic memory constants  */
#include        "dm_extr.h"                 /* Dynamic memory interfaces */


/* Define external inner-component global data references.  */

extern CS_NODE         *DMD_Created_Pools_List;
extern UNSIGNED         DMD_Total_Pools;
extern TC_PROTECT       DMD_List_Protect;


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      DMI_Initialize                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function initializes the data structures that control the   */
/*      operation of the Dynamic Memory component (DM).  There are no    */
/*      dynamic memory pools initially.                                  */
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
/*      DMD_Created_Pools_List              List of created pools        */
/*      DMD_Total_Pools                     Number of created pools      */
/*      DMD_List_Protect                    Protection for pool list     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
VOID  DMI_Initialize(VOID)
{
    /* Initialize the created dynamic memory pool list to NU_NULL.  */
    DMD_Created_Pools_List =  NU_NULL;

    /* Initialize the total number of created pools to 0.  */
    DMD_Total_Pools =  0;

    /* Initialize the list protection structure.  */
    DMD_List_Protect.tc_tcb_pointer =      NU_NULL;
}






