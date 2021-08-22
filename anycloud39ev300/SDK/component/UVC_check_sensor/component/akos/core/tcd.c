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
/*      tcd.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TC - Thread Control                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains global data structures for use within this    */
/*      component.                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      TCD_Created_Tasks_List              Pointer to the linked-list   */
/*                                            of created tasks           */
/*      TCD_Total_Tasks                     Total number of created tasks*/
/*      TCD_Priority_List                   Array of pointers to ready   */
/*                                            tasks, indexed by priority */
/*      TCD_Execute_Task                    Highest priority task to     */
/*                                            execute                    */
/*      TCD_Priority_Groups                 Bit map of 32 groups of task */
/*                                            priority                   */
/*      TCD_Sub_Priority_Groups             An array of 32 sub-priority  */
/*                                            groups                     */
/*      TCD_Lowest_Set_Bit                  Lookup table to find the     */
/*                                            lowest bit set in a byte   */
/*      TCD_Highest_Priority                Highest priority ready       */
/*      TCD_Created_HISRs_List              Pointer to the linked-list   */
/*                                            of created HISRs           */
/*      TCD_Total_HISRs                     Total number of created HISRs*/
/*      TCD_Active_HISR_Heads               Active HISR list head ptrs   */
/*      TCD_Active_HISR_Tails               Active HISR list tail ptrs   */
/*      TCD_Execute_HISR                    Highest priority HISR to     */
/*                                            execute                    */
/*      TCD_Current_Thread                  Pointer to the currently     */
/*                                            executing thread           */
/*      TCD_Registered_LISRs                List of registered LISRs     */
/*      TCD_LISR_Pointers                   Actual LISR pointers         */
/*      TCD_Interrupt_Count                 Count of ISRs in progress    */
/*      TCD_Stack_Switched                  Flag indicating that stack   */
/*                                            was switched in an ISR     */
/*      TCD_List_Protect                    Task list protection         */
/*      TCD_System_Protect                  System protection            */
/*      TCD_System_Stack                    System stack pointer - top   */
/*      TCD_LISR_Protect                    Protect LISR registration    */
/*      TCD_HISR_Protect                    Protect the list of created  */
/*                                            HISRs                      */
/*      TCD_Interrupt_Level                 Enable interrupt level       */
/*      TCD_Unhandled_Interrupt             Contains the most recent     */
/*                                            unhandled interrupt in     */
/*                                            system error conditions    */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_defs.h                           Common Service constants     */
/*      tc_defs.h                           Thread Control constants     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      01-01-1993      Added variable to save last                      */
/*                        unhandled interrupt in system                  */
/*                        error conditions, resulting in                 */
/*                        version 1.0a                                   */
/*      11-01-1993      Verified version 1.0a                            */
/*      03-01-1994      Change schedule protection to a                  */
/*                        system protection to improve                   */
/*                        performance, resulting in                      */
/*                        version 1.1                                    */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-23-2001      Made TCD_LISR_Pointers array an exclusive count  */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE

#include        "cs_defs.h"                 /* Common Service constants  */
#include        "tc_defs.h"                 /* Thread Control constants  */


/* TCD_Created_Tasks_List is the head pointer of the linked list of
   created tasks.  If the list is NU_NULL, there are no tasks created.  */

CS_NODE        *TCD_Created_Tasks_List;


/* TCD_Total_Tasks contains the number of currently created tasks.  */

UNSIGNED        TCD_Total_Tasks;


/* TCD_Priority_List is an array of TCB pointers.  Each element of the array
   is effectively the head pointer of the list of tasks ready for execution
   at that priority.  If the pointer is NULL, there are no tasks ready
   for execution at that priority.  The array is indexed by priority.  */

TC_TCB         *TCD_Priority_List[TC_PRIORITIES];


/* TCD_Priority_Groups is a 32-bit unsigned integer that is used as a bit
   map.  Each bit corresponds to an 8-priority group.  For example, if bit 0
   is set, at least one task of priority 0 through 8 is ready for execution. */

UNSIGNED        TCD_Priority_Groups;


/* TCD_Sub_Priority_Groups is an array of sub-priority groups.  These are
   also used as bit maps.  Index 0 of this array corresponds to priorities
   0 through 8.  Bit 0 of this element represents priority 0, while bit 7
   represents priority 7.  */

DATA_ELEMENT    TCD_Sub_Priority_Groups[TC_MAX_GROUPS];


/* TCD_Lowest_Set_Bit is nothing more than a standard lookup table.  The
   table is indexed by values ranging from 1 to 255.  The value at that
   position in the table indicates the number of the lowest set bit.  This is
   used to determine the highest priority task represented in the previously
   defined bit maps.  */

UNSIGNED_CHAR  TCD_Lowest_Set_Bit[] = {0,
   0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0,
   1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1,
   0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0,
   2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2,
   0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0,
   1, 0, 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1,
   0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
   4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3,
   0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0,
   1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1,
   0, 2, 0, 1, 0};


/* TCD_Highest_Priority contains the highest priority task ready for execution.
   Note that this does not necessarily represent the priority of the currently
   executing task.  This is true if the currently executing task has preemption
   disabled.  If no tasks are executing, this variable is set to the maximum
   priority.  */

INT             TCD_Highest_Priority;


/* TCD_Execute_Task is a pointer to the task to execute.  Note that this
   pointer does not necessarily point to the currently executing task.  There
   are several points in the system where this is true.  One situation is
   when preemption is about to take place.  Another situation can result from
   a internal protection conflict.  */

TC_TCB         *TCD_Execute_Task;


/* TCD_Created_HISRs_List is the head pointer of the list of created High-
   Level Interrupt Service Routines (HISR).  If this pointer is NU_NULL, there
   are no HISRs currently created.  */

CS_NODE        *TCD_Created_HISRs_List;


/* TCD_Total_HISRs contains the number of currently created HISRs.  */

UNSIGNED        TCD_Total_HISRs;


/* TCD_Active_HISR_Heads is an array of active HISR list head pointers.
   There are three HISR priorities available.  The HISR priority is an index
   into this table.  Priority/index 0 represents the highest priority.  */

TC_HCB         *TCD_Active_HISR_Heads[TC_HISR_PRIORITIES];


/* TCD_Active_HISR_Tails is an array of active HISR list tail pointers.
   There are three HISR priorities available.  The HISR priority is an index
   into this table.  Priority/index 0 represents the highest priority.  */

TC_HCB          *TCD_Active_HISR_Tails[TC_HISR_PRIORITIES];


/* TCD_Execute_HISR contains a pointer to the highest priority HISR to execute.
   If this pointer is NU_NULL, no HISRs are currently activated.  Note that
   the current thread pointer is not always equal to this pointer.  */

TC_HCB         *TCD_Execute_HISR;


/* TCD_Current_Thread points to the control block of the currently executing
   thread of execution.  Therefore, this variable points at either a TC_TCB
   or a TC_HCB structure.  Except for initialization, this variable is set
   and cleared in the target dependent portion of this component.  */

VOID           *TCD_Current_Thread;


/* TCD_System_Stack contains the system stack base pointer.  When the system
   is idle or in interrupt processing the system stack pointer is used.  This
   variable is usually setup during target dependent initialization.  */

VOID           *TCD_System_Stack;


/* TCD_Registered_LISRs is a list that specifies whether or not a
   LISR is registered for a given interrupt vector.  If the value in the
   list indexed by the vector is non-zero, then that value can be used
   as the index into the list of LISR pointers to find the actual registered
   LISR.  */

UNSIGNED_CHAR   TCD_Registered_LISRs[NU_MAX_VECTORS+1];


/* TCD_LISR_Pointers is a list of LISR pointers that indicate the LISR function
   to call when the interrupt occurs.  If the entry is NULL, it is
   available.  */

VOID    (*TCD_LISR_Pointers[NU_MAX_LISRS+1])(INT vector);


/* TCD_Interrupt_Count contains the number of Interrupt Service Routines (ISRs)
   currently in progress.  If the contents of this variable is zero, then no
   interrupts are in progress.  If the contents are greater than 1, nested
   interrupts are being processed.  */

INT     TCD_Interrupt_Count;


/* TCD_Stack_Switched contains a flag indicating that the system stack was
   switched to after the thread's context was saved.  This variable is not
   used in all ports.  */

INT     TCD_Stack_Switched;


/* TCD_List_Protect is a structure that is used to protect against multiple
   access to the list of established tasks.  */

TC_PROTECT      TCD_List_Protect;


/* TCD_System_Protect is a structure that is used to provide protection
   against multiple threads accessing the same system structures at the
   same time.  */

TC_PROTECT      TCD_System_Protect;


/* TCD_LISR_Protect is a structure that is used to provide protection against
   multiple threads accessing the LISR registration structures at the same
   time.  */

TC_PROTECT      TCD_LISR_Protect;


/* TCD_HISR_Protect is a structure that is used to provide protection against
   multiple threads accessing the created HISR linked-list at the same time. */

TC_PROTECT      TCD_HISR_Protect;


/* TCD_Interrupt_Level is a variable that contains the enabled interrupt
   level.  If the target processor does not have multiple enable interrupt
   levels, this variable is a boolean.  */

INT             TCD_Interrupt_Level;


/* TCD_Unhandled_Interrupt is a variable that contains the last unhandled
   interrupt in system error conditions.  */

INT             TCD_Unhandled_Interrupt;





