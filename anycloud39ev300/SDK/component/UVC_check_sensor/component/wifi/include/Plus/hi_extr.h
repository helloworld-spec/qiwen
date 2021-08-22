/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1998 Accelerated Technology, Inc.           */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      hi_extr.h                                       PLUS  1.3        */
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
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
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
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      W. Lamie        03-01-1993      Created initial version 1.0      */
/*      D. Lamie        04-19-1993      Verified version 1.0             */
/*      W. Lamie        03-01-1994      Moved include files outside of   */
/*                                        the file #ifndef to allow the  */
/*                                        use of actual data structures, */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
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
