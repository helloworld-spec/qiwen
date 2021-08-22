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
/*      tcf.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TC - Thread Control                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains information (fact) routines for the Thread    */
/*      Control component.                                               */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      TCF_Established_Tasks               Number of created tasks      */
/*      TCF_Established_HISRs               Number of created HISRs      */
/*      TCF_Task_Pointers                   Build list of task pointers  */
/*      TCF_HISR_Pointers                   Build list of HISR pointers  */
/*      TCF_Task_Information                Retrieve task information    */
/*      TCF_HISR_Information                Retrieve HISR information    */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      cs_extr.h                           Common Service functions     */
/*      tc_extr.h                           Thread Control functions     */
/*      in_extr.h                           Initialization/Interrupt     */
/*                                            functions                  */
/*      tm_extr.h                           Timer Control function       */
/*      er_extr.h                           Error handling function      */
/*      hi_extr.h                           History functions            */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1994      Created initial version 1.1 from                 */
/*                        original 1.0g version of TCC.C                 */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      04-17-1996      updated to version 1.2                           */
/*      11-18-1996      Protected Informational service                  */
/*                      from NULL Control Block pointers                 */
/*                      creating 1.2a. (SPR220)                          */
/*      03-24-1998      Released version 1.3.                            */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE


#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "in_extr.h"                 /* Initialization/Interrupt  */
                                            /*   functions               */
#include        "tm_extr.h"                 /* Timer control functions   */
#include        "er_extr.h"                 /* Error handling function   */
#include        "hi_extr.h"                 /* History functions         */


/* Define external inner-component global data references.  */

extern CS_NODE         *TCD_Created_Tasks_List;
extern UNSIGNED         TCD_Total_Tasks;
extern TC_TCB          *TCD_Priority_List[TC_PRIORITIES];
extern UNSIGNED         TCD_Priority_Groups;
extern DATA_ELEMENT     TCD_Sub_Priority_Groups[TC_MAX_GROUPS];
extern UNSIGNED_CHAR    TCD_Lowest_Set_Bit[];
extern INT              TCD_Highest_Priority;
extern TC_TCB          *TCD_Execute_Task;
extern VOID            *TCD_Current_Thread;
extern UNSIGNED_CHAR    TCD_Registered_LISRs[NU_MAX_VECTORS+1];
extern VOID           (*TCD_LISR_Pointers[NU_MAX_LISRS+1])(INT vector);
extern INT              TCD_Interrupt_Count;
extern INT              TCD_Stack_Switched;
extern TC_PROTECT       TCD_List_Protect;
extern TC_PROTECT       TCD_Schedule_Protect;
extern TC_PROTECT       TCD_LISR_Protect;
extern CS_NODE         *TCD_Created_HISRs_List;
extern UNSIGNED         TCD_Total_HISRs;
extern TC_PROTECT       TCD_HISR_Protect;
extern INT              TCD_Unhandled_Interrupt;


/* Define external inner-component function calls that are not available to
   other components.  */

VOID            TCT_Build_Task_Stack(TC_TCB *task_ptr);
VOID            TCT_Build_HISR_Stack(TC_HCB *hisr_ptr);
VOID            TCT_Build_Signal_Frame(TC_TCB *task_ptr);
VOID            TCT_Protect_Switch(TC_TCB *task);
VOID            TCT_Signal_Exit(VOID);



/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCF_Established_Tasks                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the current number of established tasks.   */
/*      Tasks previously deleted are no longer considered established.   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      TCD_Total_Tasks                     Number of established tasks  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  TCF_Established_Tasks(VOID)
{


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Return the number of established tasks.  */
    return(TCD_Total_Tasks);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCF_Established_HISRs                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the current number of established HISRs.   */
/*      HISRs previously deleted are no longer considered established.   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      TCD_Total_HISRs                     Number of established HISRs  */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  TCF_Established_HISRs(VOID)
{


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Return the number of established HISRs.  */
    return(TCD_Total_HISRs);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCF_Task_Pointers                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function builds a list of task pointers, starting at the    */
/*      specified location.  The number of task pointers placed in the   */
/*      list is equivalent to the total number of tasks or the maximum   */
/*      number of pointers specified in the call.                        */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect task created list    */
/*      TCT_Unprotect                       Release protection of list   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pointer_list                        Pointer to the list area     */
/*      maximum_pointers                    Maximum number of pointers   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      pointers                            Number of tasks placed in    */
/*                                            list                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Corrected pointer retrieval                      */
/*                       loop, resulting in version 1.0c                 */
/*      08-09-1993      Verified version 1.0c                            */
/*      03-01-1994      Modified function interface,                     */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  TCF_Task_Pointers(NU_TASK **pointer_list, UNSIGNED maximum_pointers)
{

CS_NODE         *node_ptr;                  /* Pointer to each TCB       */
UNSIGNED         pointers;                  /* Number of pointers in list*/
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Initialize the number of pointers returned.  */
    pointers =  0;

    /* Protect the task created list.  */
    //TCT_Protect(&TCD_List_Protect);   //主要用于异常检测，此时调用Protect会导致死机而进行不下去

    /* Loop until all task pointers are in the list or until the maximum
       list size is reached.  */
    node_ptr =  TCD_Created_Tasks_List;
    while ((node_ptr) && (pointers < maximum_pointers))
    {

        /* Place the node into the destination list.  */
        *pointer_list++ =  (NU_TASK *) node_ptr;

        /* Increment the pointers variable.  */
        pointers++;

        /* Position the node pointer to the next node.  */
        node_ptr =  node_ptr -> cs_next;

        /* Determine if the pointer is at the head of the list.  */
        if (node_ptr == TCD_Created_Tasks_List)

            /* The list search is complete.  */
            node_ptr =  NU_NULL;
    }

    /* Release protection.  */
    //TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the number of pointers in the list.  */
    return(pointers);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCF_HISR_Pointers                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function builds a list of HISR pointers, starting at the    */
/*      specified location.  The number of HISR pointers placed in the   */
/*      list is equivalent to the total number of HISRs or the maximum   */
/*      number of pointers specified in the call.                        */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect HISR created list    */
/*      TCT_Unprotect                       Release protection of list   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      pointer_list                        Pointer to the list area     */
/*      maximum_pointers                    Maximum number of pointers   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*                                          Number of HISRs placed in    */
/*                                            list                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      08-09-1993      Corrected pointer retrieval                      */
/*                      loop, resulting in version 1.0c                  */
/*      08-09-1993      Verified version 1.0c                            */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
UNSIGNED  TCF_HISR_Pointers(NU_HISR **pointer_list, UNSIGNED maximum_pointers)
{

CS_NODE         *node_ptr;                  /* Pointer to each TCB       */
UNSIGNED         pointers;                  /* Number of pointers in list*/
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Initialize the number of pointers returned.  */
    pointers =  0;

    /* Protect the HISR created list.  */
    TCT_Protect(&TCD_HISR_Protect);

    /* Loop until all HISR pointers are in the list or until the maximum
       list size is reached.  */
    node_ptr =  TCD_Created_HISRs_List;
    while ((node_ptr) && (pointers < maximum_pointers))
    {

        /* Place the node into the destination list.  */
        *pointer_list++ =  (NU_HISR *) node_ptr;

        /* Increment the pointers variable.  */
        pointers++;

        /* Position the node pointer to the next node.  */
        node_ptr =  node_ptr -> cs_next;

        /* Determine if the pointer is at the head of the list.  */
        if (node_ptr == TCD_Created_HISRs_List)

            /* The list search is complete.  */
            node_ptr =  NU_NULL;
    }

    /* Release protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the number of pointers in the list.  */
    return(pointers);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCF_Task_Information                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns information about the specified task.      */
/*      However, if the supplied task pointer is invalid, the function   */
/*      simply returns an error status.                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect scheduling info      */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Pointer to the task          */
/*      name                                Destination for the name     */
/*      status                              Destination for task status  */
/*      scheduled_count                     Destination for scheduled    */
/*                                            count of the task          */
/*      priority                            Destination for task priority*/
/*      preempt                             Destination for preempt flag */
/*      time_slice                          Destination for time slice   */
/*      stack_base                          Destination for pointer to   */
/*                                            base of task's stack       */
/*      stack_size                          Destination for stack size   */
/*      minimum_stack                       Destination for the minimum  */
/*                                            running size of the stack  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If a valid task pointer is   */
/*                                            supplied                   */
/*      NU_INVALID_TASK                     If task pointer is invalid   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        changed protection logic,                      */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      11-18-1996      Corrected SPR220.                                */
/*                                                                       */
/*************************************************************************/
STATUS  TCF_Task_Information(NU_TASK *task_ptr, CHAR *name,
            DATA_ELEMENT *status, UNSIGNED *scheduled_count,
            DATA_ELEMENT *priority, OPTION *preempt, UNSIGNED *time_slice,
            VOID **stack_base, UNSIGNED *stack_size, UNSIGNED *minimum_stack)
{

R1 TC_TCB      *task;                       /* Task control block ptr    */
INT             i;                          /* Working index             */
STATUS          completion;                 /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Determine if this task is valid.  */
    if ((task != NU_NULL) && (task -> tc_id == TC_TASK_ID))
    {

        /* Protect against scheduling changes.  */
        TCT_System_Protect();

        /* The task pointer is successful.  Reflect this in the completion
           status and fill in the actual information.  */
        completion =  NU_SUCCESS;

        /* Copy the task's name.  */
        for (i = 0; i < NU_MAX_NAME; i++)
            *name++ =  task -> tc_name[i];

        /* Determine the preemption posture.  */
        if (task -> tc_preemption)
            *preempt =          NU_PREEMPT;
        else
            *preempt =          NU_NO_PREEMPT;

        /* Setup the remaining fields.  */
        *status =           task -> tc_status;
        *scheduled_count =  task -> tc_scheduled;
        *priority =         task -> tc_priority;
        *time_slice =       task -> tc_time_slice;
        *stack_base =       task -> tc_stack_start;
        *stack_size =       task -> tc_stack_size;
        *minimum_stack =    task -> tc_stack_minimum;

        /* Release protection.  */
        TCT_Unprotect();
    }
    else

        /* Indicate that the task pointer is invalid.   */
        completion =  NU_INVALID_TASK;

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the appropriate completion status.  */
    return(completion);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCF_HISR_Information                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns information about the specified HISR.      */
/*      However, if the supplied HISR pointer is invalid, the function   */
/*      simply returns an error status.                                  */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_System_Protect                  Protect scheduling info      */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hisr_ptr                            Pointer to the hisr          */
/*      name                                Destination for the name     */
/*      scheduled_count                     Destination for scheduled    */
/*                                            count of the HISR          */
/*      priority                            Destination for HISR priority*/
/*      stack_base                          Destination for pointer to   */
/*                                            base of HISR's stack       */
/*      stack_size                          Destination for stack size   */
/*      minimum_stack                       Destination for the minimum  */
/*                                            running size of the stack  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          If a valid HISR pointer is   */
/*                                            supplied                   */
/*      NU_INVALID_HISR                     If HISR pointer is invalid   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                        added register optimizations,                  */
/*                        changed protection logic,                      */
/*                        resulting in version 1.1                       */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      11-18-1996      Corrected SPR220.                                */
/*                                                                       */
/*************************************************************************/
STATUS  TCF_HISR_Information(NU_HISR *hisr_ptr, CHAR *name,
           UNSIGNED *scheduled_count, DATA_ELEMENT *priority,
           VOID **stack_base, UNSIGNED *stack_size, UNSIGNED *minimum_stack)
{

R1 TC_HCB      *hisr;                       /* HISR control block ptr    */
INT             i;                          /* Working index             */
STATUS          completion;                 /* Completion status         */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input HISR control block pointer into internal pointer.  */
    hisr =  (TC_HCB *) hisr_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Determine if this HISR is valid.  */
    if ((hisr != NU_NULL) && (hisr -> tc_id == TC_HISR_ID))
    {

        /* Protect against scheduling changes.  */
        TCT_System_Protect();

        /* The HISR pointer is successful.  Reflect this in the completion
           status and fill in the actual information.  */
        completion =  NU_SUCCESS;

        /* Copy the hisr's name.  */
        for (i = 0; i < NU_MAX_NAME; i++)
            *name++ =  hisr -> tc_name[i];

        /* Setup the remaining fields.  */
        *scheduled_count =  hisr -> tc_scheduled;
        *priority =         hisr -> tc_priority;
        *stack_base =       hisr -> tc_stack_start;
        *stack_size =       hisr -> tc_stack_size;
        *minimum_stack =    hisr -> tc_stack_minimum;

        /* Release protection.  */
        TCT_Unprotect();
    }
    else

        /* Indicate that the HISR pointer is invalid.   */
        completion =  NU_INVALID_HISR;

    /* Return to user mode */
    NU_USER_MODE();

    /* Return the appropriate completion status.  */
    return(completion);
}




