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
/*      qu_extr.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      QU - Queue Management                                            */
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
/*      qu_defs.h                           Queue Management constants   */
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
/*                      modified function prototypes,                    */
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

#include        "qu_defs.h"                 /* Include QU constants      */


/* Check to see if the file has been included already.  */

#ifndef QU_EXTR
#define QU_EXTR


/*  Initialization functions.  */

VOID            QUI_Initialize(VOID);


/* Core error checking functions.  */

STATUS          QUCE_Create_Queue(NU_QUEUE *queue_ptr, CHAR *name, 
                      VOID *start_address, UNSIGNED queue_size, 
                      OPTION message_type, UNSIGNED message_size,
                      OPTION suspend_type);
STATUS          QUCE_Delete_Queue(NU_QUEUE *queue_ptr);
STATUS          QUCE_Send_To_Queue(NU_QUEUE *queue_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend);
STATUS          QUCE_Receive_From_Queue(NU_QUEUE *queue_ptr, VOID *message,
                      UNSIGNED size, UNSIGNED *actual_size, UNSIGNED suspend);

/* Supplemental error checking functions.  */

STATUS          QUSE_Reset_Queue(NU_QUEUE *queue_ptr);
STATUS          QUSE_Send_To_Front_Of_Queue(NU_QUEUE *queue_ptr, VOID *message,
                                        UNSIGNED size, UNSIGNED suspend);
STATUS          QUSE_Broadcast_To_Queue(NU_QUEUE *queue_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend);

/* Core processing functions.  */

STATUS          QUC_Create_Queue(NU_QUEUE *queue_ptr, CHAR *name, 
                      VOID *start_address, UNSIGNED queue_size, 
                      OPTION message_type, UNSIGNED message_size,
                      OPTION suspend_type);
STATUS          QUC_Delete_Queue(NU_QUEUE *queue_ptr);
STATUS          QUC_Send_To_Queue(NU_QUEUE *queue_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend);
STATUS          QUC_Receive_From_Queue(NU_QUEUE *queue_ptr, VOID *message,
                      UNSIGNED size, UNSIGNED *actual_size, UNSIGNED suspend);

/* Supplemental processing functions.  */

STATUS          QUS_Reset_Queue(NU_QUEUE *queue_ptr);
STATUS          QUS_Send_To_Front_Of_Queue(NU_QUEUE *queue_ptr, VOID *message,
                                        UNSIGNED size, UNSIGNED suspend);
STATUS          QUS_Broadcast_To_Queue(NU_QUEUE *queue_ptr, VOID *message, 
                                        UNSIGNED size, UNSIGNED suspend);

/* Information gathering functions.  */


UNSIGNED        QUF_Established_Queues(VOID);
STATUS          QUF_Queue_Information(NU_QUEUE *queue_ptr, CHAR *name, 
                  VOID **start_address, UNSIGNED *queue_size, 
                  UNSIGNED *available, UNSIGNED *messages, 
                  OPTION *message_type, UNSIGNED *message_size,
                  OPTION *suspend_type, UNSIGNED *tasks_waiting,
                  NU_TASK **first_task);
UNSIGNED        QUF_Queue_Pointers(NU_QUEUE **pointer_list, 
                                                UNSIGNED maximum_pointers);

#endif




