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
/*      pi_extr.h                                       PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PI - Pipe Management                                             */
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
/*      pi_defs.h                           Pipe Management constants    */
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
/*                                        modified function prototypes,  */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/

#include        "pi_defs.h"                 /* Include Pipe constants    */


/* Check to see if the file has been included already.  */

#ifndef PI_EXTR
#define PI_EXTR


/*  Initialization functions.  */

VOID            PII_Initialize(VOID);


/* Error checking core functions.  */

STATUS          PICE_Create_Pipe(NU_PIPE *pipe_ptr, CHAR *name, 
                      VOID *start_address, UNSIGNED pipe_size, 
                      OPTION message_type, UNSIGNED message_size,
                      OPTION suspend_type);
STATUS          PICE_Delete_Pipe(NU_PIPE *pipe_ptr);
STATUS          PICE_Send_To_Pipe(NU_PIPE *pipe_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend);
STATUS          PICE_Receive_From_Pipe(NU_PIPE *pipe_ptr, VOID *message,
                      UNSIGNED size, UNSIGNED *actual_size, UNSIGNED suspend);


/* Error checking supplemental functions.  */

STATUS          PISE_Reset_Pipe(NU_PIPE *pipe_ptr);
STATUS          PISE_Send_To_Front_Of_Pipe(NU_PIPE *pipe_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend);
STATUS          PISE_Broadcast_To_Pipe(NU_PIPE *pipe_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend);


/* Core processing functions.  */

STATUS          PIC_Create_Pipe(NU_PIPE *pipe_ptr, CHAR *name, 
                      VOID *start_address, UNSIGNED pipe_size, 
                      OPTION message_type, UNSIGNED message_size,
                      OPTION suspend_type);
STATUS          PIC_Delete_Pipe(NU_PIPE *pipe_ptr);
STATUS          PIC_Send_To_Pipe(NU_PIPE *pipe_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend);
STATUS          PIC_Receive_From_Pipe(NU_PIPE *pipe_ptr, VOID *message,
                      UNSIGNED size, UNSIGNED *actual_size, UNSIGNED suspend);


/* Supplemental processing functions.  */

STATUS          PIS_Reset_Pipe(NU_PIPE *pipe_ptr);
STATUS          PIS_Send_To_Front_Of_Pipe(NU_PIPE *pipe_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend);
STATUS          PIS_Broadcast_To_Pipe(NU_PIPE *pipe_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend);


/* Information retrieval functions.  */

UNSIGNED        PIF_Established_Pipes(VOID);
STATUS          PIF_Pipe_Information(NU_PIPE *pipe_ptr, CHAR *name, 
                  VOID **start_address, UNSIGNED *pipe_size, 
                  UNSIGNED *available, UNSIGNED *messages, 
                  OPTION *message_type, UNSIGNED *message_size,
                  OPTION *suspend_type, UNSIGNED *tasks_waiting,
                  NU_TASK **first_task);
UNSIGNED        PIF_Pipe_Pointers(NU_PIPE **pointer_list, 
                                                UNSIGNED maximum_pointers);

#endif
