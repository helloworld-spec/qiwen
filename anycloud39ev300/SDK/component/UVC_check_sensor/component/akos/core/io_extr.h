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
/*      io_extr.h                                      Nucleus PLUS 1.14 */
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
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Moved include files outside of                   */
/*                      the file #ifndef to allow the                    */
/*                      use of actual data structures,                   */ 
/*                      changed function interfaces,                     */
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





