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
/*      pm_defs.h                                       PLUS  1.3        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      PM - Partition Memory Management                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data structure definitions and constants for  */
/*      the Partition Memory component.                                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      William E. Lamie, Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      PM_PCB                              Partition Pool control block */
/*      PM_HEADER                           Header of each partition     */
/*      PM_SUSPEND                          Partition suspension block   */
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
/*                                        removed protect structure,     */
/*                                        added padding logic,           */
/*                                        resulting in version 1.1       */
/*      R. Pfaff -                                                       */
/*      D. Lamie        03-18-1994      Verified version 1.1             */
/*      M.Q. Qian       04-17-1996      updated to version 1.2           */
/*      M. Trippi       03-24-1998      Released version 1.3.            */
/*                                                                       */
/*************************************************************************/

#include        "cs_defs.h"                 /* Common service constants  */
#include        "tc_defs.h"                 /* Thread control constants  */


/* Check to see if the file has been included already.  */

#ifndef PM_DEFS
#define PM_DEFS


/* Define constants local to this component.  */

#define         PM_PARTITION_ID         0x50415254UL
#define         PM_OVERHEAD            ((sizeof(PM_HEADER) + sizeof(UNSIGNED) \
                                        - 1)/sizeof(UNSIGNED)) *    \
                                        sizeof(UNSIGNED)


/* Define the Partition Pool Control Block data type.  */

typedef struct PM_PCB_STRUCT 
{
    CS_NODE             pm_created;            /* Node for linking to    */
                                               /* created partition list */
    UNSIGNED            pm_id;                 /* Internal PCB ID        */
    CHAR                pm_name[NU_MAX_NAME];  /* Partition Pool name    */
    VOID               *pm_start_address;      /* Starting pool address  */
    UNSIGNED            pm_pool_size;          /* Size of pool           */
    UNSIGNED            pm_partition_size;     /* Size of each partition */
    UNSIGNED            pm_available;          /* Available partitions   */
    UNSIGNED            pm_allocated;          /* Allocated partitions   */
    struct PM_HEADER_STRUCT    
                       *pm_available_list;     /* Available list         */
    DATA_ELEMENT        pm_fifo_suspend;       /* Suspension type flag   */
#if     PAD_1
    DATA_ELEMENT        pm_padding[PAD_1];
#endif 
    UNSIGNED            pm_tasks_waiting;      /* Number of waiting tasks*/
    struct PM_SUSPEND_STRUCT
                       *pm_suspension_list;    /* Suspension list        */
} PM_PCB;    


/* Define the header structure that is in front of each memory partition.  */

typedef struct PM_HEADER_STRUCT
{
    struct PM_HEADER_STRUCT
                       *pm_next_available;     /* Next available memory  */
                                               /*   partition            */
    PM_PCB             *pm_partition_pool;     /* Partition pool pointer */
} PM_HEADER;


/* Define the partition suspension structure.  This structure is allocated 
   off of the caller's stack.  */
   
typedef struct PM_SUSPEND_STRUCT
{
    CS_NODE             pm_suspend_link;       /* Link to suspend blocks */
    PM_PCB             *pm_partition_pool;     /* Pointer to pool        */
    TC_TCB             *pm_suspended_task;     /* Task suspended         */
    VOID               *pm_return_pointer;     /* Return memory address  */
    STATUS              pm_return_status;      /* Return status          */
} PM_SUSPEND;

#endif
