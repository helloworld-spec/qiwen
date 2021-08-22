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
/*      io_extr.h                                       PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      IO - Input/Output Driver Management                              */
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
/*      io_defs.h                           I/O Driver Management consts */
/*      tc_defs.h                           Thread control constants     */
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
/*                                        changed function interfaces,   */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/

#include        "io_defs.h"                 /* Include IO constants      */
#include        "tc_defs.h"                 /* Thread control constants  */


/* Check to see if the file has been included already.  */

#ifndef IO_EXTR
#define IO_EXTR


/*  Initialization functions.  */

VOID            IOI_Initialize(VOID);


/* Error checking functions.  */

STATUS          IOCE_Create_Driver(NU_DRIVER *driver, CHAR *name, 
                    VOID (*driver_entry)(NU_DRIVER *, NU_DRIVER_REQUEST *));
STATUS          IOCE_Delete_Driver(NU_DRIVER *driver);
STATUS          IOCE_Request_Driver(NU_DRIVER *driver, 
                                                NU_DRIVER_REQUEST *request);
STATUS          IOCE_Resume_Driver(NU_TASK *task);
STATUS          IOCE_Suspend_Driver(VOID (*terminate_routine)(VOID *),
                                      VOID *information, UNSIGNED timeout);


/* Core processing functions.  */

STATUS          IOC_Create_Driver(NU_DRIVER *driver, CHAR *name, 
                    VOID (*driver_entry)(NU_DRIVER *, NU_DRIVER_REQUEST *));
STATUS          IOC_Delete_Driver(NU_DRIVER *driver);
STATUS          IOC_Request_Driver(NU_DRIVER *driver, 
                                                NU_DRIVER_REQUEST *request);
STATUS          IOC_Resume_Driver(NU_TASK *task);
STATUS          IOC_Suspend_Driver(VOID (*terminate_routine)(VOID *),
                                        VOID *information, UNSIGNED timeout);


/* Information retrieval functions.  */

UNSIGNED        IOF_Established_Drivers(VOID);
UNSIGNED        IOF_Driver_Pointers(NU_DRIVER **pointer_list, 
                                                UNSIGNED maximum_pointers);

#endif
