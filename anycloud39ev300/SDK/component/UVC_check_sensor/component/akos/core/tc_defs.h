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
/*      tc_defs.h                                      Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TC - Thread Control                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data structure definitions and constants for  */
/*      the component that controls the various threads of execution in  */
/*      system.  Threads include tasks, HISRs, signal handlers, etc.     */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      TC_TCB                              Task Control Block           */
/*      TC_HCB                              HISR Control Block           */
/*      TC_PROTECT                          Task/HISR protection struct  */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_defs.h                           Common service definitions   */
/*      tm_defs.h                           Timer control definitions    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      06-01-1993      Added padding conditional into                   */
/*                      TC_TCB structure, making                         */
/*                      version 1.0a                                     */
/*      06-01-1993      Verified version 1.0a                            */
/*      03-01-1994      Moved include files outside of                   */
/*                      the file #ifndef to allow the                    */
/*                      use of actual data structures,                   */
/*                      added four reserved words in                     */
/*                      both the task and HISR blocks,                   */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3                             */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-07-1999      Release 1.11mA                                   */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/

#include        "cs_defs.h"                 /* Common service constants  */
#include        "tm_defs.h"                 /* Timer control structures  */
#include 		"wd_extr.h"


/* Check to see if the file has been included already.  */

#ifndef TC_DEFS
#define TC_DEFS


/* Define constants local to this component.  */

#define         TC_TASK_ID              0x5441534bUL
#define         TC_HISR_ID              0x48495352UL
#define         TC_PRIORITIES           256
#define         TC_HISR_PRIORITIES      3
#define         TC_MAX_GROUPS           (TC_PRIORITIES/8)
#define         TC_HIGHEST_MASK         0x000000FFUL
#define         TC_NEXT_HIGHEST_MASK    0x0000FF00UL
#define         TC_NEXT_LOWEST_MASK     0x00FF0000UL
#define         TC_LOWEST_MASK          0xFF000000UL

/* Define the Task Control Block data type.  */

typedef struct TC_TCB_STRUCT
{
    /* Standard thread information first.  This information is used by
       the target dependent portion of this component.  Changes made
       to this area of the structure can have undesirable side effects.  */

    CS_NODE             tc_created;            /* Node for linking to    */
                                               /*   created task list    */
    UNSIGNED            tc_id;                 /* Internal TCB ID        */
    CHAR                tc_name[NU_MAX_NAME];  /* Task name              */
    DATA_ELEMENT        tc_status;             /* Task status            */
    BOOLEAN             tc_delayed_suspend;    /* Delayed task suspension*/
    DATA_ELEMENT        tc_priority;           /* Task priority          */
    BOOLEAN             tc_preemption;         /* Task preemption enable */
    UNSIGNED            tc_scheduled;          /* Task scheduled count   */
    UNSIGNED            tc_cur_time_slice;     /* Current time slice     */
    VOID               *tc_stack_start;        /* Stack starting address */
    VOID               *tc_stack_end;          /* Stack ending address   */
    VOID               *tc_stack_pointer;      /* Task stack pointer     */
    UNSIGNED            tc_stack_size;         /* Task stack's size      */
    UNSIGNED            tc_stack_minimum;      /* Minimum stack size     */
    struct TC_PROTECT_STRUCT
                       *tc_current_protect;    /* Current protection     */
    VOID               *tc_saved_stack_ptr;    /* Previous stack pointer */
    UNSIGNED            tc_time_slice;         /* Task time slice value  */

    /* Information after this point is not used in the target dependent
       portion of this component.  Hence, changes in the following section
       should not impact assembly language routines.  */
    struct TC_TCB_STRUCT
                       *tc_ready_previous,     /* Previously ready TCB   */
                       *tc_ready_next;         /* next and previous ptrs */

    /* Task control information follows.  */

    UNSIGNED            tc_priority_group;     /* Priority group mask bit*/
    struct TC_TCB_STRUCT
                      **tc_priority_head;      /* Pointer to list head   */
    DATA_ELEMENT       *tc_sub_priority_ptr;   /* Pointer to sub-group   */
    DATA_ELEMENT        tc_sub_priority;       /* Mask of sub-group bit  */
    DATA_ELEMENT        tc_saved_status;       /* Previous task status   */
    BOOLEAN             tc_signal_active;      /* Signal active flag     */

#if     PAD_3
    DATA_ELEMENT        tc_padding[PAD_3];
#endif

                                               /* Task entry function    */
    VOID                (*tc_entry)(UNSIGNED, VOID *);
    UNSIGNED            tc_argc;               /* Optional task argument */
    VOID               *tc_argv;               /* Optional task argument */
    VOID                (*tc_cleanup) (VOID *);/* Clean-up routine       */
    VOID               *tc_cleanup_info;       /* Clean-up information   */
    struct TC_PROTECT_STRUCT
                       *tc_suspend_protect;    /* Protection at time of  */
                                               /*   task suspension      */

    /* Task timer information.  */
    INT                 tc_timer_active;       /* Active timer flag      */
    TM_TCB              tc_timer_control;      /* Timer control block    */

    /* Task signal control information.  */

    UNSIGNED            tc_signals;            /* Current signals        */
    UNSIGNED            tc_enabled_signals;    /* Enabled signals        */

    /* tc_saved_status and tc_signal_active are now defined above in an
       attempt to keep DATA_ELEMENT types together.  */

    /* Signal handling routine.  */
    VOID                (*tc_signal_handler) (UNSIGNED);

    /* Reserved words for the system and a single reserved word for the
       application.  */
    UNSIGNED            tc_system_reserved_1;  /* System reserved word   */
    UNSIGNED            tc_system_reserved_2;  /* System reserved word   */
    UNSIGNED            tc_system_reserved_3;  /* System reserved word   */
    UNSIGNED            tc_app_reserved_1;     /* Application reserved   */

    /* This information is accessed in assembly */
#if ((NU_SUPERV_USER_MODE == 1)||(NU_MODULE_SUPPORT == 1))
    UNSIGNED            tc_su_mode;            /* Supervisor/User mode indicator */
    UNSIGNED            tc_module;             /* Module identifier */
#endif

#ifdef   WATCHDOG
	char*				filename;		   	   /* Watch dog feed file*/
	unsigned			fileline;		   	   /* Watch dog feed line*/
	UNSIGNED			tc_wd_counter;		   /* Watch dog counter*/		
#endif

} TC_TCB;


/* Define the High-Level Interrupt Service Routine Control Block data type.  */

typedef struct TC_HCB_STRUCT
{
    /* Standard thread information first.  This information is used by
       the target dependent portion of this component.  Changes made
       to this area of the structure can have undesirable side effects.  */

    CS_NODE             tc_created;            /* Node for linking to    */
                                               /*   created task list    */
    UNSIGNED            tc_id;                 /* Internal TCB ID        */
    CHAR                tc_name[NU_MAX_NAME];  /* HISR name              */
    DATA_ELEMENT        tc_not_used_1;         /* Not used field         */
    DATA_ELEMENT        tc_not_used_2;         /* Not used field         */
    DATA_ELEMENT        tc_priority;           /* HISR priority          */
    DATA_ELEMENT        tc_not_used_3;         /* Not used field         */
    UNSIGNED            tc_scheduled;          /* HISR scheduled count   */
    UNSIGNED            tc_cur_time_slice;     /* Not used in HISR       */
    VOID               *tc_stack_start;        /* Stack starting address */
    VOID               *tc_stack_end;          /* Stack ending address   */
    VOID               *tc_stack_pointer;      /* HISR stack pointer     */
    UNSIGNED            tc_stack_size;         /* HISR stack's size      */
    UNSIGNED            tc_stack_minimum;      /* Minimum stack size     */
    struct TC_PROTECT_STRUCT
                       *tc_current_protect;    /* Current protection     */
    struct TC_HCB_STRUCT
                       *tc_active_next;        /* Next activated HISR    */
    UNSIGNED            tc_activation_count;   /* Activation counter     */
    VOID                (*tc_entry)(VOID);     /* HISR entry function    */
    
    /* Information after this point is not used in the target dependent
       portion of this component.  Hence, changes in the following section
       should not impact assembly language routines.  */


    /* Reserved words for the system and a single reserved word for the
       application.  */
    UNSIGNED            tc_system_reserved_1;  /* System reserved word   */
    UNSIGNED            tc_system_reserved_2;  /* System reserved word   */
    UNSIGNED            tc_system_reserved_3;  /* System reserved word   */
    UNSIGNED            tc_app_reserved_1;     /* Application reserved   */

    /* This information is accessed in assembly */
#if ((NU_SUPERV_USER_MODE == 1)||(NU_MODULE_SUPPORT == 1))
    UNSIGNED            tc_su_mode;            /* Supervisor/User mode indicator */
    UNSIGNED            tc_module;             /* Module identifier */
#endif

#ifdef   WATCHDOG
	char*				filename;			   /* Watch dog feed file*/
	UNSIGNED			fileline;			   /* Watch dog feed line*/
	UNSIGNED			tc_wd_counter;		   /* Watch dog counter*/		
#endif
} TC_HCB;


/* Define the Task/HISR protection structure type.  */

typedef struct TC_PROTECT_STRUCT
{
    TC_TCB             *tc_tcb_pointer;        /* Owner of the protection */
    UNSIGNED            tc_thread_waiting;     /* Waiting thread flag     */
} TC_PROTECT;

#endif




