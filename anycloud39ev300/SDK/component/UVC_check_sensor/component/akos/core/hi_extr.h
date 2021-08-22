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
/*      hi_extr.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      HI - History Management                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes of all functions          */
/*      accessible to other components.                                  */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
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
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Moved include files outside of                   */
/*                      the file #ifndef to allow the                    */
/*                      use of actual data structures,                   */
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

#include        "hi_defs.h"                 /* Include HI constants      */


/* Check to see if the file has been included already.  */

#ifndef HI_EXTR
#define HI_EXTR


/*  Initialization functions.  */

VOID            HII_Initialize(VOID);


/* Core processing functions.  */

VOID            HIC_Disable_History_Saving(VOID);
VOID            HIC_Enable_History_Saving(VOID);
VOID            HIC_Make_History_Entry_Service(UNSIGNED param1, 
                                        UNSIGNED param2, UNSIGNED param3);
VOID            HIC_Make_History_Entry(DATA_ELEMENT id, UNSIGNED param1,
                                        UNSIGNED param2, UNSIGNED param3);
STATUS          HIC_Retrieve_History_Entry(DATA_ELEMENT *id, UNSIGNED *param1,
                                        UNSIGNED *param2, UNSIGNED *param3,
                                        UNSIGNED *time, NU_TASK **task,
                                        NU_HISR **hisr);

#endif





