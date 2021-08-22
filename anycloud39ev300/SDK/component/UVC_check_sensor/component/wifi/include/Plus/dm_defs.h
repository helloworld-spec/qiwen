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
/*      dm_defs.h                                       PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      DM - Dynamic Memory Management                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data structure definitions and constants for  */
/*      the Dynamic Memory component.                                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      DM_PCB                              Dynamic Pool control block   */
/*      DM_HEADER                           Header of each memory block  */
/*      DM_SUSPEND                          Memory suspension block      */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_defs.h                           Common service definitions   */
/*      tc_defs.h                           Thread Control definitions   */
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
/*                                        added padding logic,           */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       01-07-1997      Added missing PAD_1 field to     */
/*                                        DM_HEADER_STRUCT to be         */
/*                                        consistent with other PLUS     */
/*                                        structures creating 1.2a.      */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/

#include        "cs_defs.h"                 /* Common service constants  */
#include        "tc_defs.h"                 /* Thread control constants  */


/* Check to see if the file has been included already.  */

#ifndef DM_DEFS
#define DM_DEFS


/* Define constants local to this component.  */

#define         DM_DYNAMIC_ID          0x44594e41UL
#define         DM_OVERHEAD            ((sizeof(DM_HEADER) + sizeof(UNSIGNED) \
                                        - 1)/sizeof(UNSIGNED)) *    \
                                        sizeof(UNSIGNED)


/* Define the Dynamic Pool Control Block data type.  */

typedef struct DM_PCB_STRUCT 
{
    CS_NODE             dm_created;            /* Node for linking to    */
                                               /* created dynamic pools  */
    TC_PROTECT          dm_protect;            /* Protection structure   */
    UNSIGNED            dm_id;                 /* Internal PCB ID        */
    CHAR                dm_name[NU_MAX_NAME];  /* Dynamic Pool name      */
    VOID               *dm_start_address;      /* Starting pool address  */
    UNSIGNED            dm_pool_size;          /* Size of pool           */
    UNSIGNED            dm_min_allocation;     /* Minimum allocate size  */
    UNSIGNED            dm_available;          /* Total available bytes  */
    struct DM_HEADER_STRUCT    
                       *dm_memory_list;        /* Memory list            */
    struct DM_HEADER_STRUCT
                       *dm_search_ptr;         /* Search pointer         */
    DATA_ELEMENT        dm_fifo_suspend;       /* Suspension type flag   */
#if     PAD_1
    DATA_ELEMENT        dm_padding[PAD_1];
#endif 
    UNSIGNED            dm_tasks_waiting;      /* Number of waiting tasks*/
    struct DM_SUSPEND_STRUCT
                       *dm_suspension_list;    /* Suspension list        */
} DM_PCB;    


/* Define the header structure that is in front of each memory block.  */

typedef struct DM_HEADER_STRUCT
{
    struct DM_HEADER_STRUCT
                       *dm_next_memory,        /* Next memory block      */
                       *dm_previous_memory;    /* Previous memory block  */
    DATA_ELEMENT        dm_memory_free;        /* Memory block free flag */
#if     PAD_1
    DATA_ELEMENT        dm_padding[PAD_1];
#endif 
    DM_PCB             *dm_memory_pool;        /* Dynamic pool pointer   */
} DM_HEADER;


/* Define the dynamic memory suspension structure.  This structure is 
   allocated off of the caller's stack.  */
   
typedef struct DM_SUSPEND_STRUCT
{
    CS_NODE             dm_suspend_link;       /* Link to suspend blocks */
    DM_PCB             *dm_memory_pool;        /* Pointer to pool        */
    UNSIGNED            dm_request_size;       /* Size of memory request */
    TC_TCB             *dm_suspended_task;     /* Task suspended         */
    VOID               *dm_return_pointer;     /* Return memory address  */
    STATUS              dm_return_status;      /* Return status          */
} DM_SUSPEND;

#endif
