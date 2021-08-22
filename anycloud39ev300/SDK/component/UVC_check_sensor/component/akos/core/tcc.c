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
/*      tcc.c                                          Nucleus PLUS 1.14 */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TC - Thread Control                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the core routines for the Thread Control      */
/*      component.                                                       */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      TCC_Create_Task                     Create a task                */
/*      TCC_Delete_Task                     Delete a task                */
/*      TCC_Create_HISR                     Create HISR                  */
/*      TCC_Delete_HISR                     Delete HISR                  */
/*      TCC_Reset_Task                      Reset the specified task     */
/*      TCC_Terminate_Task                  Terminate the specified task */
/*      TCC_Resume_Task                     Resume a task                */
/*      TCC_Resume_Service                  Resume a task service call   */
/*      TCC_Suspend_Task                    Suspend a task               */
/*      TCC_Suspend_Service                 Suspend a task service call  */
/*      TCC_Task_Timeout                    Task timeout function        */
/*      TCC_Task_Sleep                      Task sleep request           */
/*      TCC_Relinquish                      Relinquish task execution    */
/*      TCC_Time_Slice                      Process task time-slice      */
/*      TCC_Current_Task_Pointer            Retrieve current task pointer*/
/*      TCC_Current_HISR_Pointer            Retrieve current HISR pointer*/
/*      TCC_Task_Shell                      Task execution shell         */
/*      TCC_Signal_Shell                    Signal execution shell       */
/*      TCC_Dispatch_LISR                   Dispatch LISR routine        */
/*      TCC_Register_LISR                   Register an LISR             */
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
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      05-15-1993      Corrected comment problems in                    */
/*                      TCC_Control_Signals and                          */
/*                      TCC_Register_Signal_Handler,                     */
/*                      making version 1.0a                              */
/*      05-15-1993      Verified version 1.0a                            */
/*      06-01-1993      Modified a multi-line comment                    */
/*                      after a #include directive                       */
/*                      that gave some compilers a                       */
/*                      problem, making version 1.0b                     */
/*      06-01-1993      Verified version 1.0b                            */
/*      08-09-1993      Corrected pointer retrieval                      */
/*                      loop, resulting in version 1.0c                  */
/*      08-09-1993      Verified version 1.0c                            */
/*      09-19-1993      Corrected an initialization                      */
/*                      problem of de-referencing a                      */
/*                      NULL pointer, resulting in                       */
/*                      version 1.0d                                     */
/*      09-19-1993      Verified version 1.0d                            */
/*      11-01-1993      Added logic to save unhandled                    */
/*                      interrupt vector number in                       */
/*                      a global variable, resulting                     */
/*                      in version 1.0e                                  */
/*      11-01-1993      Verified version 1.0e                            */
/*      02-01-1994      Corrected a suspension with                      */
/*                      timeout problem that caused                      */
/*                      unconditional task suspension                    */
/*                      for timeouts and sleeps for 1                    */
/*                      tick, resulting in version 1.0f                  */
/*      02-01-1994      Verified version 1.0f                            */
/*      03-01-1994      Added another routine that                       */
/*                      validates resume calls with                      */
/*                      the appropriate protection and                   */
/*                      added code to clear cleanup                      */
/*                      information at task create and                   */
/*                      task resume time, resulting in                   */
/*                      version 1.0g                                     */
/*      03-01-1994      Verified version 1.0g                            */
/*      03-01-1994      Moved non-core functions into                    */
/*                      supplemental files, changed                      */
/*                      function interfaces to match                     */
/*                      those in prototype, added                        */
/*                      register options, fixed bug                      */
/*                      in suspending current task from                  */
/*                      a HISR, added a system protect                   */
/*                      structure to reduce overhead                     */
/*                      in protection, moved resume                      */
/*                      validate to error checking                       */
/*                      file, resulting in version 1.1                   */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      12-18-1995      Modified TCC_Resume_Service,                     */
/*                      resulting in version 1.1+                        */
/*                      (SPR 36, 64, 66, 77)                             */
/*      04-17-1996      updated to version 1.2                           */
/*      06-01-1996      Modified TCC_Suspend_Task                        */
/*                      (SPR152) creating version 1.2a                   */
/*      10-29-1997      Modified TCC_Resume_Task                         */
/*                      (SPR115) creating vresion 1.2b                   */
/*      01-23-1998      Released version 1.2b                            */
/*      03-20-1998      Corrected problem where tasks                    */
/*                      created with NU_NO_PREEMPT in                    */
/*                      Application_Initialize would                     */
/*                      start in the order of creation                   */
/*                      and not in priority order.                       */
/*                      SPR455 creates version 1.2c                      */
/*      03-24-1998      Released version 1.3.                            */
/*      10-02-1998      Another protect problem (1.3a)                   */
/*      10-02-1998      Corrected a problem where a                      */
/*                      a timer expiration occurs                        */
/*                      during signal handler execution                  */
/*                      and causes the task to never                     */
/*                      run again (SPR 923) creating                     */
/*                      version 1.3b.                                    */
/*      03-26-1999      Released 1.11m (new release                      */
/*                        numbering scheme)                              */
/*      04-17-2002      Released version 1.13m                           */
/*      11-07-2002      Released version 1.14                            */
/*************************************************************************/
#define         NU_SOURCE_FILE

#include        "in_defs.h"                 /* Initialization defines    */
#include        "cs_extr.h"                 /* Common service functions  */
#include        "tc_extr.h"                 /* Thread control functions  */
#include        "in_extr.h"                 /* Initialization/Interrupt  */
                                            /*   functions               */
#include        "tm_extr.h"                 /* Timer control functions   */
#include        "er_extr.h"                 /* Error handling function   */
#include        "hi_extr.h"                 /* History functions         */
#include        "profiler.h"                /* Nucleus Profiling header  */
#include 		"wd_extr.h"


#if ((defined(NU_MODULE_SUPPORT)) && (NU_MODULE_SUPPORT > 0))
#include        "module/inc/ms_defs.h"      /* MS_Module typedef */
#endif

/* Define external inner-component global data references.  */

extern INT              INC_Initialize_State;
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
extern TC_PROTECT       TCD_System_Protect;
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


/* Define internal function calls.  */

VOID            TCC_Signal_Shell(VOID);


#if defined(NU_MODULE_SUPPORT) && (NU_MODULE_SUPPORT > 0)

/* Define service routines "privately" imported from other components */
extern MS_MODULE* msd_current_module;

STATUS MSC_Bind_Module_HISR(MS_MODULE* module, TC_HCB* hisr);
STATUS MSC_Bind_Module_Task(MS_MODULE* module, TC_TCB* task);

#endif /* NU_MODULE_SUPPORT */
 
#ifdef DEBUG_TRACE_TASK_INFO
//typedef	unsigned long			T_U32;		/* unsigned 32 bit integer */

UNSIGNED get_tick_count_us(T_VOID);

#define MAX_TASK_NUMBER 100
typedef struct
{
	T_U32 taskid;
	T_U32 total_sec;
	T_U32 us;
}T_TASK_INFO, *TP_TASK_INFO;

T_TASK_INFO sys_task_info[MAX_TASK_NUMBER];
long long pre_task_start_time = -1;
long pre_task_pos_id= -1;

void TCC_InitSysTaskInfo()
{
	memset(sys_task_info, 0, sizeof(sys_task_info));
}

void TCC_ClearSysTaskTimeInfo()
{
	int i;
	
	for(i = 0; i < MAX_TASK_NUMBER; i++)
	{
			sys_task_info[i].total_sec = 0;
			sys_task_info[i].us = 0;
	}
	pre_task_start_time = get_tick_count_us();
}

void TCC_InsertTaskInfo(T_U32 taskid)
{
	int i;
	
	for(i = 0; i < MAX_TASK_NUMBER; i++)
	{
		if(sys_task_info[i].taskid == 0)
		{
			sys_task_info[i].taskid = taskid;
			sys_task_info[i].total_sec = 0;
			sys_task_info[i].us = 0;
			return;
		}
	}
}

void TCC_RemoveTaskInfo(T_U32 taskid)
{
	int i;

	for(i = 0; i < MAX_TASK_NUMBER; i++)
	{
		if(sys_task_info[i].taskid == taskid)
		{
			sys_task_info[i].taskid = 0;
			sys_task_info[i].total_sec = 0;
			sys_task_info[i].us = 0;
			return;
		}
	}
}

int TCC_FindTaskInfo(T_U32 taskid)
{
	int i;
	for(i = 0; i < MAX_TASK_NUMBER; i++)
	{
		if(sys_task_info[i].taskid == taskid)
		{			
			return i;
		}
	}

	return -1;
}

unsigned long TCC_GetTaskTimeInfo(T_U32 taskid, int sec_flag)
{
	int i;
	for(i = 0; i < MAX_TASK_NUMBER; i++)
	{
		if(sys_task_info[i].taskid == taskid)
		{
			if (sec_flag)
				return sys_task_info[i].total_sec + sys_task_info[i].us /1000/1000;
			else
				return sys_task_info[i].total_sec * 1000 + sys_task_info[i].us /1000;
		}
	}
	printf("no task info,");
	return -1;
}

extern TC_HCB         *TCD_Execute_HISR;

VOID TCT_Schedule_exit_time(VOID)
{
	TC_TCB *CurrTask = (TC_TCB *)TCD_Execute_Task;
	int taskid;
	long long tick;
	
	if(CurrTask)
	{
		if(pre_task_start_time != -1 && pre_task_pos_id != -1)
		{
			tick = get_tick_count_us();
			if (tick >= pre_task_start_time)
			{
				sys_task_info[pre_task_pos_id].us += tick - pre_task_start_time;
				if (sys_task_info[pre_task_pos_id].us >= 1000000)
				{
					sys_task_info[pre_task_pos_id].total_sec++;
					sys_task_info[pre_task_pos_id].us -= 1000000;
				}
			}
		}
		taskid = TCC_FindTaskInfo((T_U32)CurrTask);
		if(-1 != taskid)
		{
			pre_task_pos_id = taskid;
			pre_task_start_time = get_tick_count_us();
		}
	}
} 

#else
VOID TCT_Schedule_exit_time(VOID)
{
}
#endif
/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Create_Task                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a task and then places it on the list of   */
/*      created tasks.  All the resources necessary to create the task   */
/*      are supplied to this routine.  If specified, the newly created   */
/*      task is started.  Otherwise, it is left in a suspended state.    */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCCE_Create_Task                    Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Add TCB to linked-list       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Start the created task       */
/*      TCT_Build_Task_Stack                Build an initial task stack  */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Protect                         Protect created task list    */
/*      TCT_Unprotect                       Release protection of list   */
/*      TMC_Init_Task_Timer                 Initialize the task's timer  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*      name                                Task name                    */
/*      task_entry                          Entry function of the task   */
/*      argc                                Optional task parameter      */
/*      argv                                Optional task parameter      */
/*      stack_address                       Pointer to start of stack    */
/*      stack_size                          Size of task stack in bytes  */
/*      priority                            Task priority                */
/*      time_slice                          Task time slice              */
/*      preempt                             Task preemptability flag     */
/*      auto_start                          Automatic task start         */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      added system protection logic,                   */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCC_Create_Task(NU_TASK *task_ptr, CHAR *name,
          VOID (*task_entry)(UNSIGNED, VOID *), UNSIGNED argc, VOID *argv,
          VOID *stack_address, UNSIGNED stack_size,
          OPTION priority, UNSIGNED time_slice,
          OPTION preempt, OPTION auto_start)
{

R1 TC_TCB      *task;                       /* Task control block ptr    */
R2 INT          i;                          /* Working index variable    */
STATUS          status = NU_SUCCESS;

NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input task pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CREATE_TASK_ID, (UNSIGNED) task,
                                (UNSIGNED) name, (UNSIGNED) task_entry);

#endif

    /* First, clear the task ID just in case it is an old Task
       Control Block.  */
    task -> tc_id =             0;

    /* Fill in the task name.  */
    for (i = 0; i < NU_MAX_NAME; i++)
        task -> tc_name[i] =  name[i];

    /* Fill in the basic task information.  */
    task -> tc_entry =                  task_entry;
    task -> tc_argc =                   argc;
    task -> tc_argv =                   argv;
    task -> tc_status =                 NU_PURE_SUSPEND;
    task -> tc_delayed_suspend =        NU_FALSE;
    task -> tc_scheduled =              0;
    task -> tc_time_slice =             time_slice;
    task -> tc_cur_time_slice =         time_slice;
    task -> tc_current_protect =        NU_NULL;
    task -> tc_suspend_protect =        NU_NULL;
    task -> tc_cleanup =                NU_NULL;
    task -> tc_cleanup_info =           NU_NULL;
#ifdef   WATCHDOG
	task -> filename		=			"Unknown";		   	/* Watch dog feed file*/
	task -> fileline		=			0;		   	   		/* Watch dog feed line*/
    task -> tc_wd_counter =             0;
#endif

	
    /* Setup task's preemption posture.  */
    if (preempt == NU_PREEMPT)
        task -> tc_preemption =  NU_TRUE;
    else
        task -> tc_preemption =  NU_FALSE;

    /* Fill in information about the task's stack.  */
    task -> tc_stack_start =            stack_address;
    task -> tc_stack_end =              0;
    task -> tc_stack_size =             stack_size;
    task -> tc_stack_minimum =          stack_size;

    /* Setup priority information for the task.  There are two bit maps
       associated with each task.  The first bit map indicates which group
       of 8-priorities it is.  The second bit map indicates the actual
       priority within the group.  */
    task -> tc_priority =               priority;
    task -> tc_priority_head =          &(TCD_Priority_List[priority]);
    task -> tc_sub_priority =           (DATA_ELEMENT) (1 << (priority & 7));
    priority =                          priority >> 3;
    task -> tc_priority_group =         ((UNSIGNED) 1) << priority;
    task -> tc_sub_priority_ptr =       &(TCD_Sub_Priority_Groups[priority]);

    /* Initialize link pointers.  */
    task -> tc_created.cs_previous =    NU_NULL;
    task -> tc_created.cs_next =        NU_NULL;
    task -> tc_ready_previous =         NU_NULL;
    task -> tc_ready_next =             NU_NULL;

    /* Build a stack frame for this task by calling TCT_Build_Task_Stack.  */
    TCT_Build_Task_Stack(task);

    /* Initialize the signal information of the task.  */
    task -> tc_signals =                0;
    task -> tc_enabled_signals =        0;
    task -> tc_signal_handler =         0;
    task -> tc_signal_active =          NU_FALSE;
    /* Initialize additional kernel options data */

#if (NU_SUPERV_USER_MODE == 1)
    task->tc_su_mode = 0;              /* Initially in User mode */
    task->tc_module = 0;               /* Not initially bound to a module */
#endif

    /* Initialize the task timer.  */
    task -> tc_timer_active =           NU_FALSE;
    TMC_Init_Task_Timer(&(task -> tc_timer_control), (VOID *) task);

	#ifdef DEBUG_TRACE_TASK_INFO
	TCC_InsertTaskInfo((T_U32)task);
	#endif

    /* Protect the list of created tasks.  */
    TCT_Protect(&TCD_List_Protect);

    /* At this point the task is completely built.  The ID can now be
       set and it can be linked into the created task list.  */
    task -> tc_id =                     TC_TASK_ID;

#if defined(NU_MODULE_SUPPORT) && (NU_MODULE_SUPPORT > 0)
    /* If executing in a thread's context, bind to that thread's module */
    if(TCD_Current_Thread != NU_NULL)
    {
        status = MSC_Bind_Module_Task(
          (MS_MODULE*)(((TC_TCB*)(TCD_Current_Thread))->tc_module), task);
    }
    else /* It must be initialization time, so use the current module */
    {
        status = MSC_Bind_Module_Task(msd_current_module, task);
    }
#endif /* NU_MODULE_SUPPORT */

    /* Link the task into the list of created tasks and increment the
       total number of tasks in the system.  */
    CSC_Place_On_List(&TCD_Created_Tasks_List, &(task -> tc_created));
    TCD_Total_Tasks++;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_CREATE_TASK);
#endif

    /* Release the protection.  */
    TCT_Unprotect();

    /* Determine if the task should be automatically started.  */
    if (auto_start == NU_START)
    {

        /* Protect the system data structures.  */
        TCT_Protect(&TCD_System_Protect);

        /* Start the task by resuming it.  If the preemption is required,
           leave the current task.  */
        if (TCC_Resume_Task(task_ptr, NU_PURE_SUSPEND))


            /* Transfer control back to the system.  */
            TCT_Control_To_System();
        else

            /* Release the protection.  */
            TCT_Unprotect();
    }

    /* Return to user mode */
    NU_USER_MODE();

    /* Return successful completion.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Create_HISR                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function creates a High-Level Interrupt Service Routine     */
/*      (HISR) and then places it on the list of created HISRs.  All     */
/*      the resources necessary to create the HISR are supplied to this  */
/*      routine.  HISRs are always created in a dormant state.           */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCCE_Create_HISR                    Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Place_On_List                   Add TCB to linked-list       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCT_Build_HISR_Stack                Build an initial HISR stack  */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created HISR list    */
/*      TCT_Unprotect                       Release protection of list   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hisr_ptr                            HISR control block pointer   */
/*      name                                HISR name                    */
/*      hisr_entry                          Entry function of the HISR   */
/*      priority                            Task priority                */
/*      stack_address                       Pointer to start of stack    */
/*      stack_size                          Size of HISR stack in bytes  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      added more control block                         */
/*                      initialization, resulting in                     */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCC_Create_HISR(NU_HISR *hisr_ptr, CHAR *name,
          VOID (*hisr_entry)(VOID), OPTION priority,
          VOID *stack_address, UNSIGNED stack_size)
{

R1 TC_HCB      *hisr;                       /* HISR control block ptr    */
R2 INT          i;                          /* Working index variable    */
STATUS          status = NU_SUCCESS;

NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input HISR pointer into internal pointer.  */
    hisr =  (TC_HCB *) hisr_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_CREATE_HISR_ID, (UNSIGNED) hisr,
                                (UNSIGNED) name, (UNSIGNED) hisr_entry);

#endif

    /* First, clear the HISR ID just in case it is an old HISR
       Control Block.  */
    hisr -> tc_id =             0;

    /* Fill in the HISR name.  */
    for (i = 0; i < NU_MAX_NAME; i++)
        hisr -> tc_name[i] =  name[i];

    /* Fill in the basic HISR information.  */
    hisr -> tc_entry =                  hisr_entry;
    hisr -> tc_scheduled =              0;
    hisr -> tc_activation_count =       0;
    hisr -> tc_cur_time_slice =         0;

#ifdef   WATCHDOG
	hisr -> filename		=			"Unknown";		   	/* Watch dog feed file*/
	hisr -> fileline		=			0;		   	   		/* Watch dog feed line*/
	hisr -> tc_wd_counter =             0;
#endif		


    /* Fill in information about the HISR's stack.  */
    hisr -> tc_stack_start =            stack_address;
    hisr -> tc_stack_end =              0;
    hisr -> tc_stack_size =             stack_size;
    hisr -> tc_stack_minimum =          stack_size;

    /* Setup priority information for the HISR.  Priorities range from 0 to
       TC_HISR_PRIORITIES - 1.  */
    hisr -> tc_priority =               priority & 3;

    /* Initialize link pointers.  */
    hisr -> tc_created.cs_previous =    NU_NULL;
    hisr -> tc_created.cs_next =        NU_NULL;
    hisr -> tc_active_next =            NU_NULL;

    /* Clear protect pointer.  */
    hisr -> tc_current_protect =        NU_NULL;

    /* Initialize additional kernel options data */
#if (NU_SUPERV_USER_MODE == 1)
    hisr->tc_su_mode = 1;                   /* TCT_HISR_Shell in Supervisor mode */
    hisr->tc_module = 0;                    /* Not initially bound to a module */
#endif

    /* Build a stack frame for this HISR by calling TCT_Build_HISR_Stack.  */
    TCT_Build_HISR_Stack(hisr);

	#ifdef DEBUG_TRACE_TASK_INFO
	TCC_InsertTaskInfo((T_U32)hisr);
	#endif
    /* Protect the list of created HISRs.  */
    TCT_Protect(&TCD_HISR_Protect);

    /* At this point the HISR is completely built.  The ID can now be
       set and it can be linked into the created HISR list.  */
    hisr -> tc_id =                     TC_HISR_ID;

#if defined(NU_MODULE_SUPPORT) && (NU_MODULE_SUPPORT > 0)
    /* If executing in a thread's context, bind to that thread's module */
    if(TCD_Current_Thread != NU_NULL)
    {
        status = MSC_Bind_Module_HISR(
          (MS_MODULE*)(((TC_TCB*)(TCD_Current_Thread))->tc_module), hisr);
    }
    else /* It must be initialization time, so use the current module */
    {
        status = MSC_Bind_Module_HISR(msd_current_module, hisr);
    }
#endif /* NU_MODULE_SUPPORT */

    /* Link the HISR into the list of created HISRs and increment the
       total number of HISRs in the system.  */
    CSC_Place_On_List(&TCD_Created_HISRs_List, &(hisr -> tc_created));
    TCD_Total_HISRs++;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpHisr(hisr,RT_PROF_CREATE_HISR);
#endif

    /* Release the protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return successful completion.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Delete_Task                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deletes a task and removes it from the list of     */
/*      created tasks.  It is assumed by this function that the task is  */
/*      in a finished or terminated state.  Note that this function      */
/*      does not free memory associated with the task's control block or */
/*      its stack.  This is the responsibility of the application.       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCCE_Delete_Task                    Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove node from list        */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created task list    */
/*      TCT_Unprotect                       Release protection of list   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCC_Delete_Task(NU_TASK *task_ptr)
{

R1 TC_TCB  *task;                           /* Task control block ptr    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input task pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;

	#ifdef DEBUG_TRACE_TASK_INFO
	TCC_RemoveTaskInfo((T_U32)task);
	#endif


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_DELETE_TASK_ID, (UNSIGNED) task,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect the list of created tasks.  */
    TCT_Protect(&TCD_List_Protect);

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_DELETE_TASK);
#endif /*INCLUDE_PROVIEW*/

    /* Remove the task from the list of created tasks.  */
    CSC_Remove_From_List(&TCD_Created_Tasks_List, &(task -> tc_created));

    /* Decrement the total number of created tasks.  */
    TCD_Total_Tasks--;

    /* Clear the task ID just in case.  */
    task -> tc_id =  0;

    /* Release protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return a successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Delete_HISR                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function deletes a HISR and removes it from the list of     */
/*      created HISRs.  It is assumed by this function that the HISR is  */
/*      in a non-active state.  Note that this function does not free    */
/*      memory associated with the HISR's control block or its stack.    */
/*      This is the responsibility of the application.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCCE_Delete_HISR                    Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      CSC_Remove_From_List                Remove node from list        */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created HISR list    */
/*      TCT_Unprotect                       Release protection of list   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hisr_ptr                            HISR control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCC_Delete_HISR(NU_HISR *hisr_ptr)
{

R1 TC_HCB  *hisr;                           /* HISR control block ptr    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input HISR pointer into internal pointer.  */
    hisr =  (TC_HCB *) hisr_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_DELETE_HISR_ID, (UNSIGNED) hisr,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect the list of created HISRs.  */
    TCT_Protect(&TCD_HISR_Protect);

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpHisr(hisr,RT_PROF_DELETE_HISR);
#endif /*INCLUDE_PROVIEW*/

    /* Remove the HISR from the list of created HISRs.  */
    CSC_Remove_From_List(&TCD_Created_HISRs_List, &(hisr -> tc_created));

    /* Decrement the total number of created HISRs.  */
    TCD_Total_HISRs--;

    /* Clear the HISR ID just in case.  */
    hisr -> tc_id =  0;

	#ifdef DEBUG_TRACE_TASK_INFO
	TCC_RemoveTaskInfo((T_U32)hisr);
	#endif

    /* Release protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return a successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Reset_Task                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function resets the specified task.  Note that a task reset */
/*      can only be performed on tasks in a finished or terminated state.*/
/*      The task is left in an unconditional suspended state.            */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCCE_Reset_Task                     Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCT_Build_Task_Stack                Build an initial task stack  */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created task list    */
/*      TCT_Unprotect                       Release protection of list   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*      argc                                Optional task parameter      */
/*      argv                                Optional task parameter      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          Indicates successful request */
/*      NU_NOT_TERMINATED                   Indicates task was not       */
/*                                            finished or terminated     */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      added system protection logic,                   */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCC_Reset_Task(NU_TASK *task_ptr, UNSIGNED argc, VOID *argv)
{

R1 TC_TCB      *task;                       /* Task control block ptr   */
STATUS          status;                     /* Status of the request    */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move input task pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_RESET_TASK_ID, (UNSIGNED) task,
                                        (UNSIGNED) argc, (UNSIGNED) argv);

#endif

    /* Protect system structures.  */
    TCT_Protect(&TCD_System_Protect);

    /* Determine if the task is in the proper state.  */
    if ((task -> tc_status == NU_FINISHED) ||
                        (task -> tc_status == NU_TERMINATED))
    {

        /* Yes, a valid reset is present.  Indicate this in the status.  */
        status =  NU_SUCCESS;

        /* Fill in the new argument information and reset some of the other
           fields.  */
        task -> tc_argc =                   argc;
        task -> tc_argv =                   argv;
        task -> tc_status =                 NU_PURE_SUSPEND;
        task -> tc_delayed_suspend =        NU_FALSE;
        task -> tc_scheduled =              0;
        task -> tc_stack_minimum =          task -> tc_stack_size;

#if (NU_SUPERV_USER_MODE == 1)
        /* Since we are doing a complete reset we need to ensure 
           that this field is 0 since the task will be started in
           user mode.  TCC_Task_Shell can not return and therefore
           left the task in supervisor mode when the task completed.
           If we were to not re-initialize this field the task would
           become locked in user mode and API would fail. */
        task -> tc_su_mode = 0;
#endif

        /* Build a stack frame for this task by calling
           TCT_Build_Task_Stack.  */
        TCT_Build_Task_Stack(task);
    }
    else

        /* The requested task is not in a finished or terminated state.  */
        status =  NU_NOT_TERMINATED;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_RESET_TASK);
#endif /*INCLUDE_PROVIEW*/

    /* Release the protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Return completion status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Terminate_Task                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function terminates the specified task.  If the task is     */
/*      already terminated, this function does nothing.  If the task     */
/*      to terminate is currently suspended, the specified cleanup       */
/*      routine is also invoked to cleanup suspension data structures.   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCCE_Terminate_Task                 Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      Cleanup routine                     Task's suspend cleanup funct */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Suspend_Task                    Suspend a ready task         */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect created task list    */
/*      TCT_Unprotect                       Release protection of list   */
/*      TCT_Unprotect_Specific              Specific unprotection        */
/*      TMC_Stop_Task_Timer                 Stop a task timer            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      added system protection logic,                   */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCC_Terminate_Task(NU_TASK *task_ptr)
{

R1 TC_TCB      *task;                       /* Task control block ptr    */
TC_PROTECT     *suspend_protect;            /* Suspension protection ptr */
DATA_ELEMENT    status;                     /* Task status               */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Move task pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_TERMINATE_TASK_ID, (UNSIGNED) task,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Determine if the calling task is the current task.  */
    if (task == (TC_TCB *) TCD_Current_Thread)
    {

        /* Protect system  data structures.  */
        TCT_Protect(&TCD_System_Protect);

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_TERMINATE_TASK);
#endif /*INCLUDE_PROVIEW*/

        /* Suspend the calling task with the NU_TERMINATED status.  */
        TCC_Suspend_Task(task_ptr, NU_TERMINATED, NU_NULL, NU_NULL,
                                                            NU_SUSPEND);

        /* No need to un-protect, since control never comes back to this
           point and the protection is cleared in TCT_Control_To_System.  */
    }
    else
    {

        /* Protect scheduling structures.  */
        TCT_Protect(&TCD_System_Protect);

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_TERMINATE_TASK);
#endif /*INCLUDE_PROVIEW*/

        /* Keep trying to terminate the specified task until its status
           indicates that it is terminated or finished.  */
        while ((task -> tc_status != NU_FINISHED) &&
                  task -> tc_status != NU_TERMINATED)
        {

            /* Is the task in a ready state?  */
            if (task -> tc_status == NU_READY)
            {

                /* Terminate the specified task.  */
                TCC_Suspend_Task(task_ptr, NU_TERMINATED, NU_NULL,
                                                        NU_NULL,NU_SUSPEND);

                /* Clear system protection.  */
                TCT_Unprotect();
            }
            else
            {

                /* Task is suspended currently.  Pickup the suspension
                   protection.  */
                suspend_protect =  task -> tc_suspend_protect;

                /* Save the current status.  */
                status =           task -> tc_status;

                /* Release protection on system structures.  */
                TCT_Unprotect();

                /* Determine if there was a suspension protection.  If so
                   protect it first before the scheduling list protection.
                   This avoids a deadlock situation.  */
                if (suspend_protect)

                    /* Protect the terminated task's last suspension
                       structures.  */
                    TCT_Protect(suspend_protect);

                /* Protect the system structures again.  */
                TCT_Protect(&TCD_System_Protect);

                /* Now determine if the same suspension is in force.  */
                if ((task -> tc_status == status) &&
                    (task -> tc_suspend_protect == suspend_protect))
                {

                    /* Yes, same suspension is in force.  */

                    /* Call cleanup routine, if there is one.  */
                    if (task -> tc_cleanup)

                        /* Call cleanup function.  */
                        (*(task -> tc_cleanup)) (task -> tc_cleanup_info);

                    /* Status the task as terminated.  */
                    task -> tc_status =  NU_TERMINATED;

                    /* Determine if there is a timer active.  */
                    if (task -> tc_timer_active)
                    {

                        /* Call the stop timer function.  */
                        TMC_Stop_Task_Timer(&(task -> tc_timer_control));

                        /* Clear the timer active flag.  */
                        task -> tc_timer_active =  NU_FALSE;
                    }
                }

                /* Cleanup the protection.  */
                if (suspend_protect)
                {

                    /* Release specific protection.  */
                    TCT_Unprotect_Specific(suspend_protect);

                    /* Clear the suspend protect field.  */
                    task -> tc_suspend_protect =  NU_NULL;
                }

                /* Release current protection.  */
                TCT_Unprotect();
            }

            /* Protect the scheduling list again.  */
            TCT_Protect(&TCD_System_Protect);
        }

        /* Release the protection.  */
        TCT_Unprotect();
    }

    /* Return to user mode */
    NU_USER_MODE();

    /* Return successful completion.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Resume_Task                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function resumes a previously suspended task.  The task     */
/*      task must currently be suspended for the same reason indicated   */
/*      by this request.  If the task resumed is higher priority than    */
/*      the calling task and the current task is preemptable, this       */
/*      function returns a value of NU_TRUE.  Otherwise, if no           */
/*      preemption is required, a NU_FALSE is returned.  This routine    */
/*      must be called from Supervisor mode in a Supervisor/User mode    */
/*      switching kernel.                                                */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Other Components                                                 */
/*      TCC_Resume_Service                  Resume service function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Set_Current_Protect             Set current protection field */
/*      TCT_Set_Execute_Task                Set TCD_Execute_Task pointer */
/*      TMC_Stop_Task_Timer                 Stop task timer              */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*      suspend_type                        Type of suspension to lift   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_TRUE                             A higher priority task is    */
/*                                            ready to execute           */
/*      NU_FALSE                            No change in the task to     */
/*                                            execute                    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      09-19-1993      Corrected an initialization                      */
/*                      problem of de-referencing a                      */
/*                      NULL pointer, resulting in                       */
/*                      version 1.0d                                     */
/*      09-19-1993      Verified version 1.0d                            */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      modified protection logic to                     */
/*                      assume that system protection                    */
/*                      is already in force, resulting                   */
/*                      in version 1.1                                   */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      10-29-1997      Changed so that tc_cleanup,                      */
/*                      tc_cleanup_info, and                             */
/*                      tc_suspend_protect are cleared                   */
/*                      only if a signal is not active                   */
/*                      (SPR115)                                         */
/*      03-20-1998      Corrected SPR455.                                */
/*                                                                       */
/*************************************************************************/
STATUS  TCC_Resume_Task(NU_TASK *task_ptr, OPTION suspend_type)
{

R1 TC_TCB      *task;                       /* Task control block ptr    */
R2 TC_TCB      *head;                       /* Pointer to priority list  */
STATUS          status =  NU_FALSE;         /* Status variable           */

    /* Move task pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif


    /* Check to see if the task is suspended for the reason that this
       resume is attempting to clear.  */
    if (task -> tc_status == suspend_type)
    {

        /* Yes, this resume call is valid.  */

        /* If signals are not active, clear any suspend or cleanup
           information (SPR115).  */
        if (!task -> tc_signal_active)
        {
            task -> tc_suspend_protect =        NU_NULL;
            task -> tc_cleanup =                NU_NULL;
            task -> tc_cleanup_info =           NU_NULL;
        }

        /* Determine if there is a timer active and the task is not being
           resumed to handle a signal.  */
        if ((task -> tc_timer_active) && (!task -> tc_signal_active))
        {

            /* Call the stop timer function.  */
            TMC_Stop_Task_Timer(&(task -> tc_timer_control));

            /* Clear the timer active flag.  */
            task -> tc_timer_active =  NU_FALSE;
        }

        /* Check to see if there is a pending pure suspension.  If so,
           change the cause of the suspension and leave in a suspended
           state.  */
        if (task -> tc_delayed_suspend)
        {

            /* Leave suspended but change the task's status and clear the
               delayed suspension flag.  */
            task -> tc_delayed_suspend =  NU_FALSE;
            task -> tc_status =  NU_PURE_SUSPEND;
        }
        else
        {

            /* Lift the suspension of the specified task.  */

            /* Clear the status of the task.  */
            task -> tc_status =  NU_READY;

#ifdef INCLUDE_PROVIEW
            _RTProf_TaskStatus(task,RT_TASK_READY);
#endif /*INCLUDE_PROVIEW*/

            /* Link the task into the appropriate priority list.  */
            head =  *(task -> tc_priority_head);

            /* Determine if the list is non-empty.  */
            if (head)
            {

                /* Add the new TCB to the end of the ready list.  */
                task -> tc_ready_previous =      head -> tc_ready_previous;
                (task -> tc_ready_previous) -> tc_ready_next =  task;
                task -> tc_ready_next =          head;
                (task -> tc_ready_next) -> tc_ready_previous =  task;

                /* Note that the priority bit map does not need to be
                   modified since there are other active tasks at the
                   same priority.  */
            }
            else
            {

                /* Add the new TCB to an empty list.  */
                task -> tc_ready_previous =  task;
                task -> tc_ready_next =      task;
                *(task -> tc_priority_head)= task;

                /* Update the priority group bit map to indicate that this
                   priority now has a task ready.  */
                TCD_Priority_Groups =
                        TCD_Priority_Groups | (task -> tc_priority_group);

                /* Update the sub-priority bit map to show that this priority
                   is ready.  */
                *(task -> tc_sub_priority_ptr) =
                   (*(task -> tc_sub_priority_ptr)) | task -> tc_sub_priority;

                /* Determine if this newly ready task is higher priority
                   than the current task.  */
                if ((INT) (task -> tc_priority) < TCD_Highest_Priority)
                {

                    /* Update the highest priority field.  */
                    TCD_Highest_Priority = (INT) task -> tc_priority;

                    /* See if there is a task to execute.  */
                    if (TCD_Execute_Task == NU_NULL)

                        /* Make this task the current.  */
                        TCT_Set_Execute_Task(task);

                    /* Check to see if the task to execute is preemptable.  */
                    /* SPR455 checks if we are in Application_Initialize */
                    else if ((TCD_Execute_Task -> tc_preemption)
                             || (INC_Initialize_State == INC_START_INITIALIZE))
                    {

                        /* Yes, the task to execute is preemptable.  Replace
                           it with the new task.  */
                        TCT_Set_Execute_Task(task);

                        /* Now, check and see if the current thread is a task.
                           If so, return a status that indicates a context
                           switch is needed.  */
                        if ((TCD_Current_Thread) &&
                           (((TC_TCB *) TCD_Current_Thread) -> tc_id ==
                                TC_TASK_ID))

                            /* Yes, a context switch is needed.  */
                            status =  NU_TRUE;
                    }
                }
            }
        }
    }
    else
    {

        /* Check for a resumption of a delayed pure suspend.  */
        if (suspend_type == NU_PURE_SUSPEND)

            /* Clear the delayed suspension.  */
            task -> tc_delayed_suspend =  NU_FALSE;

        /* Check for a signal active and the saved status the same as
           the resume request.  */
        if ((suspend_type == task -> tc_saved_status) &&
            (task -> tc_signal_active))
        {

            /* Indicate the saved status as ready.  */
            task -> tc_saved_status =  NU_READY;

            /* Determine if the task's timer is active.  */
            if (task -> tc_timer_active)
            {

                /* Stop the timer.  */
                TMC_Stop_Task_Timer(&(task -> tc_timer_control));

                /* Clear the timer active flag.  */
                task -> tc_timer_active =  NU_FALSE;
            }
        }
    }

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_RESUME_TASK);
#endif /*INCLUDE_PROVIEW*/

    /* Return back the status.  */
    return(status);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Resume_Service                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function provides an interface identical to the application */
/*      service call to resume a task.                                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCCE_Resume_Service                 Error checking function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Resume_Task                     Resume a task                */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Protect                         Protect system structures    */
/*      TCT_Unprotect                       Release system protection    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          Always returns success       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      added system protection logic,                   */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      12-19-1995      Changed the "task" parameter to                  */
/*                      "task_ptr" in the                                */
/*                      HIC_Make_History_Entry call,                     */
/*                      resulting in version 1.1+                        */
/*                      (SPR 36, 64, 66, 77)                             */
/*      10-02-1998      Another protect problem (1.3a)                   */
/*                                                                       */
/*************************************************************************/
STATUS  TCC_Resume_Service(NU_TASK *task_ptr)
{

TC_PROTECT     *save_protect;               /* Save current protection   */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_RESUME_TASK_ID, (UNSIGNED) task_ptr,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Save current protection.  */
    if (TCD_Current_Thread != NU_NULL)
    {
        save_protect = TCT_Get_Current_Protect();
    }
    else
    {
        save_protect = NU_NULL;
    }

    /* Protect system structures.  */
    TCT_Protect(&TCD_System_Protect);

    /* Call the actual resume task function.  If the function returns a
       NU_TRUE, context switching is needed.  */
    if (TCC_Resume_Task(task_ptr, NU_PURE_SUSPEND))
    {

        /* Transfer control back to the system for a context switch.  */
        TCT_Control_To_System();
    }

    /* Determine how to get out of protection.  */
    if (save_protect)
    {

        /* Restore current protection.  */
        TCT_Set_Current_Protect(save_protect);

        /* Release system protect.  */
        TCT_Unprotect_Specific(&TCD_System_Protect);
    }
    else

        /* Release protection of system structures.  */
        TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Always return a successful status.  */
    return(NU_SUCCESS);
}




/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Suspend_Task                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function suspends the specified task.  If the specified     */
/*      task is the calling task, control is transferred back to the     */
/*      system.                                                          */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Other Components                                                 */
/*      TCC_Suspend_Service                 Task suspend service         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Protect                         Protect system structures    */
/*      TCT_Set_Execute_Task                Set TCD_Execute_Task pointer */
/*      TCT_Protect_Switch                  Allow protected task to run  */
/*                                            briefly                    */
/*      TCT_Unprotect                       Release system protection    */
/*      TMC_Start_Task_Timer                Start a task timer           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*      suspend_type                        Type of suspension to lift   */
/*      cleanup                             Cleanup routine              */
/*      information                         Information for cleanup      */
/*      timeout                             Timeout on the suspension    */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      02-01-1994      Corrected a suspension with                      */
/*                      timeout problem that caused                      */
/*                      unconditional task suspension                    */
/*                      for timeouts and sleeps for 1                    */
/*                      tick, resulting in version 1.0f                  */
/*      02-01-1994      Verified version 1.0f                            */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      fixed a possible suspending an                   */
/*                      executing task from a HISR,                      */
/*                      removed excessive protection                     */
/*                      logic since protection is now                    */
/*                      in force upon function entry,                    */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*      06-01-1996      Checked to see whether                           */
/*                      task == TCD_Current_Thread                       */
/*                      regardless of whether task ==                    */
/*                      TCD_Execute_Task (SPR152)                        */
/*                                                                       */
/*************************************************************************/
VOID  TCC_Suspend_Task(NU_TASK *task_ptr, OPTION suspend_type,
                VOID (*cleanup) (VOID *), VOID *information, UNSIGNED timeout)
{

R1 TC_TCB      *task;                       /* Task control block ptr    */
R2 INT          index;                      /* Working index variable    */
DATA_ELEMENT    temp;                       /* Temporary variable        */



    /* Move input task pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif


    /* Determine if there is a timeout to initiate.  */
    if (timeout != NU_SUSPEND)
    {

        /* Indicate that a task timer is active.  */
        task -> tc_timer_active =  NU_TRUE;

        /* Start a timeout on the suspension.  */
        TMC_Start_Task_Timer(&(task -> tc_timer_control),timeout);
    }


    /* Check for a non self-suspension.  In such cases, the target task
       cannot have any type of protection in force.  */
    if (task != (TC_TCB *) TCD_Current_Thread)
    {

        do
        {

            /* Check for protection.  Remember that system protection is
               in effect.  */
            if (task -> tc_current_protect)
            {

                /* Yes, target task is in a protected mode.  Release
                   the protection on the scheduling list and transfer
                   control briefly to the target task.  */
                TCT_Unprotect();

                /* Switch to the protected task and wait until the
                   task is not protected.  */
                TCT_Protect_Switch(task);

                /* Restore protection of the system structures.  */
                TCT_Protect(&TCD_System_Protect);
            }
        } while (task -> tc_current_protect);
    }

    /* Check to see if the task is currently ready.  */
    if (task -> tc_status == NU_READY)
    {

        /* Mark the task with the appropriate suspension code.  */
        task -> tc_status =        suspend_type;

        /* Store off termination information in the tasks control block. */
        task -> tc_cleanup =       cleanup;
        task -> tc_cleanup_info =  information;

        /* Remove the task from the ready list.  */

        /* Determine if the task is the only one on the list.  */
        if (task -> tc_ready_next == task)
        {

            /* Only task on the list.  Clear the task's pointers and
               clear the entry in the priority table.  */
            task -> tc_ready_next =        NU_NULL;
            task -> tc_ready_previous =    NU_NULL;
            *(task -> tc_priority_head) =  NU_NULL;

            /* Clear the sub-priority group.  */
            *(task -> tc_sub_priority_ptr) =
                (*(task -> tc_sub_priority_ptr)) & ~(task -> tc_sub_priority);

            /* Determine if the main priority group needs to be cleared.
               This is only true if there are no other bits set in this
               sub-priority.  */
            if (*(task -> tc_sub_priority_ptr) == 0)

                /* Clear the main priority group bit.  */
                TCD_Priority_Groups =
                    TCD_Priority_Groups & ~(task -> tc_priority_group);

            /* Determine if this priority group was the highest in the
               system.  */
            if (task -> tc_priority == (DATA_ELEMENT) TCD_Highest_Priority)
            {

                /* Determine the highest priority task in the system.  */
                if (TCD_Priority_Groups == 0)
                {

                    /* Re-initialize the highest priority variable and
                       clear the current task pointer.  */
                    TCD_Highest_Priority =  TC_PRIORITIES;
                }
                else
                {

                    /* Find the next highest priority task.  */
                    if (TCD_Priority_Groups & TC_HIGHEST_MASK)

                        /* Base of sub-group is 0.  */
                       index =  0;

                    else if (TCD_Priority_Groups & TC_NEXT_HIGHEST_MASK)

                        /* Base of sub-group is 8.  */
                        index =  8;

                    else if (TCD_Priority_Groups & TC_NEXT_LOWEST_MASK)

                        /* Base of sub-group is 16.  */
                        index =  16;
                    else

                        /* Base of sub-group is 24.  */
                        index =  24;

                    /* Calculate the highest available priority.  */
                    index =  index + TCD_Lowest_Set_Bit[(INT)
                        ((TCD_Priority_Groups >> index) & TC_HIGHEST_MASK)];

                    /* Get the mask of the priority within the group of
                       8 priorities.  */
                    temp =  TCD_Sub_Priority_Groups[index];

                    /* Calculate the actual priority.  */
                    TCD_Highest_Priority =
                        (index << 3) + TCD_Lowest_Set_Bit[temp];
                }
            }
        }
        else
        {

            /* Not the only task ready at the same priority level.  */

            /* Remove from the linked-list.  */
            (task -> tc_ready_previous) -> tc_ready_next =
                                                 task -> tc_ready_next;
            (task -> tc_ready_next) -> tc_ready_previous =
                                                 task -> tc_ready_previous;

            /* See if the task being suspended is the current.  */
            if (*(task -> tc_priority_head) == task)

                /* Update the head of this priority list.  */
                *(task -> tc_priority_head) =  task -> tc_ready_next;

            /* Clear the task's pointers.  */
            task -> tc_ready_next =        NU_NULL;
            task -> tc_ready_previous =    NU_NULL;
        }

        /* Determine if this task the highest priority task.  */
        if (task == TCD_Execute_Task)
        {

            /* Determine the next task to execute.  */
            if (TCD_Highest_Priority < TC_PRIORITIES)

                /* Put the next task to execute in TCD_Execute_Task.  */
                TCT_Set_Execute_Task(TCD_Priority_List[TCD_Highest_Priority]);
            else

                /* No other task is ready for execution.  */
                TCT_Set_Execute_Task(NU_NULL);
        }

#ifdef INCLUDE_PROVIEW
        _RTProf_DumpTask(task,RT_PROF_SUSPEND_TASK);

        if (suspend_type == NU_SLEEP_SUSPEND)
        {
            _RTProf_TaskStatus(task,RT_TASK_SLEEPING);
        }
        else if (suspend_type == NU_PURE_SUSPEND)
        {
            _RTProf_TaskStatus(task,RT_TASK_SUSPENDED);
        }
        else
        {
            _RTProf_TaskStatus(task,RT_TASK_WAITING);
        }
#endif /*INCLUDE_PROVIEW*/

        /* See if the suspending task is the current thread. (SPR152) */
        if (task == (TC_TCB *) TCD_Current_Thread)

            /* Leave the task, transfer control to the system.  */
            TCT_Control_To_System();

    }
    else
    {

        /* Check for a pure suspension request.  If present, the delayed
           suspension flag is set.  */
        if (suspend_type == NU_PURE_SUSPEND)

            /* Setup the delayed suspension flag.  */
            task -> tc_delayed_suspend =  NU_TRUE;
#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_SUSPEND_TASK);
#endif /*INCLUDE_PROVIEW*/

    }
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Suspend_Service                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function provides a suitable interface to the actual        */
/*      service to suspend a task.                                       */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCCE_Suspend_Service                Error checking function      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Suspend_Task                    Suspend a task               */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect system structures    */
/*      TCT_Unprotect                       Release system structures    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          Always a successful status   */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      added system protection logic,                   */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCC_Suspend_Service(NU_TASK *task_ptr)
{

NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_SUSPEND_TASK_ID, (UNSIGNED) task_ptr,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif


    /* Protect system data structures.  */
    TCT_Protect(&TCD_System_Protect);

    /* Call the actual routine to suspend the task.  */
    TCC_Suspend_Task(task_ptr, NU_PURE_SUSPEND, NU_NULL, NU_NULL, NU_SUSPEND);

    /* Release system protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();

    /* Always return a successful status.  */
    return(NU_SUCCESS);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Task_Timeout                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function processes task suspension timeout conditions.      */
/*      Note that task sleep requests are also considered a timeout      */
/*      condition.  This routine must be called from Supervisor mode in  */
/*      a Supervisor/User mode switching kernel.                         */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TMC_Timer_Task                      Timer expiration task        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      Caller's cleanup function                                        */
/*      TCC_Resume_Task                     Resume a suspended task      */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect scheduling list      */
/*      TCT_Set_Current_Protect             Setup current protection     */
/*      TCT_Unprotect                       Release protection           */
/*      TCT_Unprotect_Specific              Release specific protection  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task_ptr                            Task control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      02-01-1994      Added logic to clear the timer                   */
/*                      active flag when the target                      */
/*                      task is already in a ready                       */
/*                      state, resulting in                              */
/*                      version 1.0f                                     */
/*      02-01-1994      Verified version 1.0f                            */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      removed logic for timeout                        */
/*                      before suspension because new                    */
/*                      protection logic eliminates                      */
/*                      the possibility,                                 */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  TCC_Task_Timeout(NU_TASK *task_ptr)
{

R1 TC_TCB       *task;                      /* Task control block ptr    */
TC_PROTECT      *suspend_protect;           /* Suspension protect ptr    */
DATA_ELEMENT     task_status;               /* Task status variable      */



    /* Move task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Protect system data structures.  */
    TCT_Protect(&TCD_System_Protect);

    /* Pickup the suspension protection saved-off when the task was
       suspended.  */
    suspend_protect =  task -> tc_suspend_protect;

    /* Is a signal handler currently running? */
    if (task -> tc_signal_active)

        /* Use the saved status for current task status */
        task_status =      task -> tc_saved_status;
    else

        /* Just use the current task status */
        task_status =      task -> tc_status;

    /* Release protection of the scheduling list.  */
    TCT_Unprotect();

    /* Determine if there is a suspend protect.  */
    if (suspend_protect)

        /* Protect the suspended protection.  */
        TCT_Protect(suspend_protect);

    /* Now protect the system structures again.  Note that the order the
       protections are made prevents deadlocks.  */
    TCT_Protect(&TCD_System_Protect);

    /* Determine if the task is still suspended in the same manner.  */
    if ((task -> tc_status == task_status) ||
      ((task -> tc_signal_active) && (task -> tc_saved_status == task_status)))
    {

        /* Make sure that this timeout processing is still valid. */
        if ((task -> tc_timer_active) &&
                 (task -> tc_timer_control.tm_remaining_time == 0))
        {

            /* Clear the timer active flag.  */
            task -> tc_timer_active =  NU_FALSE;

            /* Call the cleanup function, if there is one.  */
            if (task -> tc_cleanup)

                /* Call cleanup function.  */
                (*(task -> tc_cleanup)) (task -> tc_cleanup_info);

            /* Resume the task.  */
            TCC_Resume_Task(task_ptr, task_status);
        }
    }

    /* Determine if a suspend protection was in force.  */
    if (suspend_protect)
    {

        /* Set the current protection to the suspend protect.  */
        TCT_Set_Current_Protect(suspend_protect);
        TCT_Unprotect_Specific(&TCD_System_Protect);
    }

    /* Release current protection.  */
    TCT_Unprotect();
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Task_Sleep                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function provides task sleep suspensions.  Its primary      */
/*      purpose is to interface with the actual task suspension function.*/
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      TCC_Suspend_Task                    Suspend a task               */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect system structures    */
/*      TCT_Unprotect                       Release system structures    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      ticks                               Number of timer ticks        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified call to suspend the                     */
/*                      calling task, resulting in                       */
/*                      version 1.1                                      */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  TCC_Task_Sleep(UNSIGNED ticks)
{

NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_SLEEP_ID, (UNSIGNED) ticks,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif


    /* Protect system data structures.  */
    TCT_Protect(&TCD_System_Protect);

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask((TC_TCB *)TCD_Current_Thread,RT_PROF_SLEEP);
#endif /*INCLUDE_PROVIEW*/

    /* Call the actual routine to suspend the task.  */
    TCC_Suspend_Task((NU_TASK *) TCD_Current_Thread, NU_SLEEP_SUSPEND,
                                                NU_NULL, NU_NULL, ticks);

    /* Release system protection.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Relinquish                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function moves the calling task to the end of other tasks   */
/*      at the same priority level.  The calling task does not execute   */
/*      again until all the other tasks of the same priority get a       */
/*      chance to execute.                                               */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      TCCE_Relinquish                     Error checking shell         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Control_To_System               Transfer control to system   */
/*      TCT_Protect                         Protect system structures    */
/*      TCT_Set_Execute_Task                Set TCD_Execute_Task pointer */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified protection logic,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  TCC_Relinquish(VOID)
{

TC_TCB         *task;                       /* Pointer to task           */
NU_SUPERV_USER_VARIABLES

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_RELINQUISH_ID, (UNSIGNED) 0,
                                        (UNSIGNED) 0, (UNSIGNED) 0);

#endif

    /* Protect against multiple access to the system structures.  */
    TCT_Protect(&TCD_System_Protect);

    /* Pickup the current thread and place it in the task pointer.  */
    task =  (TC_TCB *) TCD_Current_Thread;

#ifdef INCLUDE_PROVIEW
    _RTProf_DumpTask(task,RT_PROF_RELINQUISH);
#endif /*INCLUDE_PROVIEW*/
     
    /* Determine if another task is ready to run.  */
    if ((task -> tc_ready_next != task) ||
        (task -> tc_priority != (DATA_ELEMENT) TCD_Highest_Priority))
    {

        /* Move the executing task to the end of tasks having the same
           priority. */
        *(task -> tc_priority_head) =  task -> tc_ready_next;

        /* Setup the next task to execute.  */
        TCT_Set_Execute_Task(TCD_Priority_List[TCD_Highest_Priority]);

        /* Transfer control back to the system.  */
        TCT_Control_To_System();
    }

    /* Release protection of system structures.  */
    TCT_Unprotect();

    /* Return to user mode */
    NU_USER_MODE();
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Time_Slice                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function moves the specified task to the end of the other   */
/*      tasks at the same priority level.  If the specified task is no   */
/*      longer ready, this request is ignored.  This routine must be     */
/*      called from Supervisor mode in a Supervisor/User mode            */
/*      switching kernel.                                                */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TMC_Timer_HISR                      Time-slice interrupt         */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect the scheduling data  */
/*      TCT_Set_Execute_Task                Set TCD_Execute_Task pointer */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      task                                Task control block pointer   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      added register optimizations,                    */
/*                      slightly modified protection                     */
/*                      logic, resulting in version 1.1                  */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  TCC_Time_Slice(NU_TASK *task_ptr)
{

R1 TC_TCB      *task;                       /* Task control block ptr    */


    /* Move input task control block pointer into internal pointer.  */
    task =  (TC_TCB *) task_ptr;


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Protect against multiple access to the system structures.  */
    TCT_Protect(&TCD_System_Protect);

    /* Determine if another task is ready to run.  */
    if (((task -> tc_status == NU_READY) && (task -> tc_preemption)) &&
        ((task -> tc_ready_next != task) ||
         (task -> tc_priority != (DATA_ELEMENT) TCD_Highest_Priority)))
    {

        /* Move the executing task to the end of tasks having the same
           priority. */
        *(task -> tc_priority_head) =  task -> tc_ready_next;

        /* Setup the next task to execute.  */
        TCT_Set_Execute_Task(TCD_Priority_List[TCD_Highest_Priority]);

    }

    /* Release protection of the system structures.  */
    TCT_Unprotect();
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Current_Task_Pointer                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the pointer of the currently executing     */
/*      task.  If the current thread is not a task thread, a NU_NULL     */
/*      is returned.                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      Other Components                                                 */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      Task Pointer                        Active tasks pointer or      */
/*                                            NU_NULL if not a task      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
NU_TASK  *TCC_Current_Task_Pointer(VOID)
{


    /* Determine if a task thread is executing.  */
    if ((TCD_Current_Thread) &&
        (((TC_TCB *) TCD_Current_Thread) -> tc_id == TC_TASK_ID))

        /* Task thread is running, return the pointer.  */
        return((NU_TASK *) TCD_Current_Thread);
    else

        /* No, task thread is not running, return a NU_NULL.  */
        return(NU_NULL);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Current_HISR_Pointer                                         */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function returns the pointer of the currently executing     */
/*      HISR.  If the current thread is not a HISR thread, a NU_NULL     */
/*      is returned.                                                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*      Other Components                                                 */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      HISR Pointer                        Active HISR pointer or       */
/*                                            NU_NULL if not a HISR      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Modified function interface,                     */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
NU_HISR  *TCC_Current_HISR_Pointer(VOID)
{


    /* Determine if a HISR thread is executing.  */
    if ((TCD_Current_Thread) &&
        (((TC_HCB *) TCD_Current_Thread) -> tc_id == TC_HISR_ID))

        /* HISR thread is running, return the pointer.  */
        return((NU_HISR *) TCD_Current_Thread);
    else

        /* No, HISR thread is not running, return a NU_NULL.  */
        return(NU_NULL);
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Task_Shell                                                   */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is shell from which all application tasks are      */
/*      initially executed.  The shell causes the task to finish when    */
/*      control is returned from the application task.  Also, the shell  */
/*      passes argc and argv arguments to the task's entry function.     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TCC_Control_To_Task                 Control to task routine      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      Task Entry Function                                              */
/*      TCC_Suspend_Task                    Suspend task when finished   */
/*      TCT_Protect                         Protect system structures    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Added protection logic prior to                  */
/*                      suspending the task, resulting                   */
/*                      in version 1.1                                   */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  TCC_Task_Shell(VOID)
{
NU_SUPERV_USER_VARIABLES

    /* Call the task's entry function with the argc and argv parameters
       supplied during task creation or reset.  */
    (*(TCD_Execute_Task -> tc_entry)) (TCD_Execute_Task -> tc_argc,
                                       TCD_Execute_Task -> tc_argv);

    /* Switch to supervisor mode */
    NU_SUPERVISOR_MODE();

    /* Protect system data structures.  */
    TCT_Protect(&TCD_System_Protect);

    /* If the task returns, suspend it in a finished state.  Note that
       the task cannot execute again until it is reset.  Therefore, this
       call never returns.  */
    TCC_Suspend_Task((NU_TASK *) TCD_Execute_Task, NU_FINISHED,
                                NU_NULL, NU_NULL, NU_SUSPEND);

    /* Return to user mode */
    NU_USER_MODE();
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Signal_Shell                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function processes signals by calling the task supplied     */
/*      signal handling function.  When signal handling is completed,    */
/*      the task is placed in the appropriate state.                     */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TCC_Control_Signals                 Control task's signals       */
/*      TCC_Register_Signal_Handler         Register a signal handler    */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      task's signal handling routine                                   */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Signal_Exit                     Signal handling exit routine */
/*      TCT_Protect                         Protect against other access */
/*      TCT_Set_Execute_Task                Set TCD_Execute_Task pointer */
/*      TCT_Unprotect                       Release protection           */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Added register optimizations,                    */
/*                      modified protection logic,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
VOID  TCC_Signal_Shell(VOID)
{

R2 UNSIGNED     signals;                    /* Signals to send to task   */
INT             index;                      /* Working index variable    */
DATA_ELEMENT    temp;                       /* Temporary variable        */
R1 TC_TCB      *task;                       /* Task pointer              */


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

    /* Point at the current task.  */
    task =  (TC_TCB *) TCD_Current_Thread;

    /* Protect against simultaneous access.  */
    TCT_Protect(&TCD_System_Protect);

    /* Process while there are signals to handle.  */
    while (task -> tc_signals & task -> tc_enabled_signals)
    {

        /* Pickup the signals and clear them.  */
        signals =  task -> tc_signals;
        task -> tc_signals =  0;

        /* Release protection.  */
        TCT_Unprotect();

        /* Call the application signal handling function, if there still is
           one.  */
        if (task -> tc_signal_handler)
        {
            NU_SUPERV_USER_VARIABLES

#if (defined(NU_SUPERV_USER_MODE)) && (NU_SUPERV_USER_MODE > 0)
            UNSIGNED savedMode = task->tc_su_mode;
            task->tc_su_mode = 1;         /* Force transition to User mode */
#endif
            /* Switch to user mode */
            NU_USER_MODE();

            /* Call signal handler.  (always in User mode) */
            (*(task -> tc_signal_handler))(signals);

            /* Return to supervisor mode */
            NU_SUPERVISOR_MODE();

#if (defined(NU_SUPERV_USER_MODE)) && (NU_SUPERV_USER_MODE > 0)
            task->tc_su_mode = savedMode;   /* Restore original nesting count */
#endif
        }

        /* Protect against simultaneous access again.  */
        TCT_Protect(&TCD_System_Protect);
    }

    /* At this point, signals have been exhausted and protection is in
       force.  */

    /* Clear the signal in process flag.  */
    task -> tc_signal_active =  NU_FALSE;

    /* Determine how the signal handler was called.  Either in a solicited or
       an unsolicited manner.  */
    if (task -> tc_saved_stack_ptr)
    {

        /* Determine if the saved status still indicates that the task should
           be suspended.  */
        if (task -> tc_saved_status != NU_READY)
        {

            /* Suspend the task.  */
            task -> tc_status =  task -> tc_saved_status;

            /* Remove the task from the ready list.  */

            /* Determine if the task is the only one on the list.  */
            if (task -> tc_ready_next == task)
            {

                /* Only task on the list.  Clear the task's pointers and
                   clear the entry in the priority table.  */
                task -> tc_ready_next =        NU_NULL;
                task -> tc_ready_previous =    NU_NULL;
                *(task -> tc_priority_head) =  NU_NULL;

                /* Clear the sub-priority group.  */
                *(task -> tc_sub_priority_ptr) =
                 (*(task -> tc_sub_priority_ptr)) & ~(task -> tc_sub_priority);

                /* Determine if the main priority group needs to be cleared.
                   This is only true if there are no other bits set in this
                   sub-priority.  */
                if (*(task -> tc_sub_priority_ptr) == 0)

                /* Clear the main priority group bit.  */
                TCD_Priority_Groups =
                    TCD_Priority_Groups & ~(task -> tc_priority_group);

                /* Determine if this priority group was the highest in the
                   system.  */
                if (task -> tc_priority == (DATA_ELEMENT) TCD_Highest_Priority)
                {

                    /* Determine the highest priority task in the system.  */
                    if (TCD_Priority_Groups == 0)
                    {

                        /* Re-initialize the highest priority variable and
                           clear the current task pointer.  */
                        TCD_Highest_Priority =  TC_PRIORITIES;
                    }
                    else
                    {

                        /* Find the next highest priority task.  */
                        if (TCD_Priority_Groups & TC_HIGHEST_MASK)

                            /* Base of sub-group is 0.  */
                            index =  0;

                        else if (TCD_Priority_Groups & TC_NEXT_HIGHEST_MASK)

                            /* Base of sub-group is 8.  */
                            index =  8;

                        else if (TCD_Priority_Groups & TC_NEXT_LOWEST_MASK)

                            /* Base of sub-group is 16.  */
                            index =  16;
                        else

                            /* Base of sub-group is 24.  */
                            index =  24;

                        /* Calculate the highest available priority.  */
                        index =  index + TCD_Lowest_Set_Bit[(INT)
                           ((TCD_Priority_Groups >> index) & TC_HIGHEST_MASK)];

                        /* Get the mask of the priority within the group of
                           8 priorities.  */
                        temp =  TCD_Sub_Priority_Groups[index];

                        /* Calculate the actual priority.  */
                        TCD_Highest_Priority =
                            (index << 3) + TCD_Lowest_Set_Bit[temp];
                    }
                }
            }
            else
            {

                /* Not the only task ready at the same priority level.  */

                /* Remove from the linked-list.  */
                (task -> tc_ready_previous) -> tc_ready_next =
                                                 task -> tc_ready_next;
                (task -> tc_ready_next) -> tc_ready_previous =
                                                 task -> tc_ready_previous;

                /* See if the task being suspended is the current.  */
                if (*(task -> tc_priority_head) == task)

                    /* Update the head of this priority list.  */
                    *(task -> tc_priority_head) =  task -> tc_ready_next;

                /* Clear the task's pointers.  */
                task -> tc_ready_next =        NU_NULL;
                task -> tc_ready_previous =    NU_NULL;
            }

            /* Determine the next task to execute.  */
            if (TCD_Highest_Priority < TC_PRIORITIES)

                /* Put the next task to execute in TCD_Execute_Task.  */
                TCT_Set_Execute_Task(TCD_Priority_List[TCD_Highest_Priority]);
            else

                /* No other task is ready for execution.  */
                TCT_Set_Execute_Task(NU_NULL);
        }

        /* At this point, just exit back to the system.  Note that the
           signal exit routine clears the scheduling protection.  */
        TCT_Signal_Exit();
    }

    /* A signal handler was called from the current task.  Nothing needs
       to be done except to release protection.  */
    TCT_Unprotect();
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Dispatch_LISR                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function dispatches the LISR associated with the specified  */
/*      interrupt vector.  Note that this function is called during      */
/*      the interrupt thread.  This routine must be called from          */
/*      Supervisor mode in a Supervisor/User mode switching kernel.      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      INT_Interrupt_Shell                 Shell of interrupt routine   */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      application LISR                                                 */
/*      ERC_System_Error                    Unhandled interrupt error    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      vector                              Vector number of interrupt   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      11-01-1993      Added logic to save unhandled                    */
/*                      interrupt vector number in                       */
/*                      a global variable, resulting                     */
/*                      in version 1.0e                                  */
/*      11-01-1993      Verified version 1.0e                            */
/*                                                                       */
/*************************************************************************/
VOID  TCC_Dispatch_LISR(INT vector)
{

INT             index;                      /* Working index variable    */


    /* Determine if the specified vector has an LISR registered to it.  */
    index =  (INT)  TCD_Registered_LISRs[vector];
    if (index <= NU_MAX_LISRS)
    {
#ifdef INCLUDE_PROVIEW
        _RTProf_Dispatch_LISR_No_INT_Lock(vector);
#endif /*INCLUDE_PROVIEW*/

        /* Yes, an LISR is associated with this vector.  Call the actual
           registered LISR routine.  */
        (*(TCD_LISR_Pointers[index])) (vector);
    }
    else
    {

        /* Save interrupt vector number in TCD_Unhandled_Interrupt.  */
        TCD_Unhandled_Interrupt =  vector;

        /* System error, unhandled interrupt.  */
        ERC_System_Error(NU_UNHANDLED_INTERRUPT);
    }
}


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TCC_Register_LISR                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function registers the supplied LISR with the supplied      */
/*      vector number.  If the supplied LISR is NU_NULL, the supplied    */
/*      vector is de-registered.  The previously registered LISR is      */
/*      returned to the caller, along with the completion status.  This  */
/*      routine must be called from Supervisor mode in a Supervisor/     */
/*      User mode switching kernel.                                      */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      [HIC_Make_History_Entry]            Make entry in history log    */
/*      INT_Retrieve_Shell                  Retrieve vector shell pointer*/
/*      INT_Setup_Vector                    Setup the actual vector      */
/*      INT_Vectors_Loaded                  Determine if interrupt shell */
/*                                            routines are loaded        */
/*      [TCT_Check_Stack]                   Stack checking function      */
/*      TCT_Protect                         Protect LISR registration    */
/*      TCT_Unprotect                       Release LISR protection      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      vector                              Vector number of interrupt   */
/*      new_lisr                            New LISR function            */
/*      old_lisr                            Previous LISR function ptr   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                          Successful registration      */
/*      NU_INVALID_VECTOR                   Invalid interrupt vector     */
/*      NU_NO_MORE_LISRS                    LISR registration table is   */
/*                                            full                       */
/*      NU_NOT_REGISTERED                   LISR was not registered      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*        DATE                    REMARKS                                */
/*                                                                       */
/*      03-01-1993      Created initial version 1.0                      */
/*      04-19-1993      Verified version 1.0                             */
/*      03-01-1994      Added appropriate casting,                       */
/*                      resulting in version 1.1                         */
/*                                                                       */
/*      03-18-1994      Verified version 1.1                             */
/*                                                                       */
/*************************************************************************/
STATUS  TCC_Register_LISR(INT vector, VOID (*new_lisr)(INT),
                                        VOID (**old_lisr)(INT))
{

INT             index;                      /* Working index variable    */
STATUS          status;                     /* Completion status         */


#ifdef  NU_ENABLE_STACK_CHECK

    /* Call stack checking function to check for an overflow condition.  */
    TCT_Check_Stack();

#endif

#ifdef  NU_ENABLE_HISTORY

    /* Make an entry that corresponds to this function in the system history
       log.  */
    HIC_Make_History_Entry(NU_REGISTER_LISR_ID, (UNSIGNED) vector,
                                (UNSIGNED) new_lisr, (UNSIGNED) old_lisr);

#endif

    /* Determine if the vector is legal. */
    if (vector > NU_MAX_VECTORS)
        return(NU_INVALID_VECTOR);

    /* Initialize the completion status to successful.  */
    status =  NU_SUCCESS;

    /* Protect against LISR registration list access.  */
    TCT_Protect(&TCD_LISR_Protect);

    /* Determine if a registration or deregistration is requested.  This is
       determined by the value of new_lisr.  A NULL value indicates
       deregistration.  */
    if (new_lisr)
    {

        /* Register the new LISR.  */

        /* Determine if the vector already has a registration.  */
        if (TCD_Registered_LISRs[vector])
        {

            /* Yes, a registration exists.  */

            /* Pickup the index into the LISR pointer list.  */
            index =  (INT) TCD_Registered_LISRs[vector];

            /* Temporarily indicate that the LISR is not registered.  */
            TCD_Registered_LISRs[vector] =  0;

            /* Copy the currently registered LISR into the old_lisr return
               area.  */
            *old_lisr =  TCD_LISR_Pointers[index];

            /* Place the new LISR into the list. */
            TCD_LISR_Pointers[index] =  new_lisr;

            /* Indicate the LISR is registered again.  */
            TCD_Registered_LISRs[vector] =  (UNSIGNED_CHAR) index;
        }
        else
        {

            /* An empty slot needs to be found in the LISR pointers list.  */

            index =  0;
            while ((index <= NU_MAX_LISRS) && 
                   (TCD_LISR_Pointers[index] != NU_NULL))
                      index++;

            /* Determine if an empty slot was found.  */
            if (index <= NU_MAX_LISRS)
            {

                /* Yes, an empty slot was found.  */

                /* Place the new LISR in the LISR pointers list.  */
                TCD_LISR_Pointers[index] =  new_lisr;

                /* Associate the index into the pointers list to the actual
                   vector.  */
                TCD_Registered_LISRs[vector] =  (UNSIGNED_CHAR) index;

                /* Indicate that there was no previous LISR registered.  */
                *old_lisr =  NU_NULL;

                /* Determine if the actual vector needs to be stolen.  */
                if (!INT_Vectors_Loaded())

                    /* Actual vector needs to be replaced with the
                       appropriate ISR shell.  */
                    INT_Setup_Vector(vector, INT_Retrieve_Shell(vector));
            }
            else

                /* Return the completion status that indicates that there
                   is no more room in the LISR pointers list.  */
                status =  NU_NO_MORE_LISRS;
        }
    }
    else
    {

        /* De-register the specified vector.  */

        /* Determine if the vector has a registration current.  */
        if (TCD_Registered_LISRs[vector])
        {

            /* Pickup the index into the LISR pointer list.  */
            index =  (INT) TCD_Registered_LISRs[vector];

            /* Clear the registration table.  */
            TCD_Registered_LISRs[vector] =  0;

            /* Return the previously registered LISR.  */
            *old_lisr =  TCD_LISR_Pointers[index];

            /* Clear the LISR pointer list entry.  */
            TCD_LISR_Pointers[index] =  NU_NULL;
        }
        else

            /* The vector is not registered.  Return an error completion
               status.  */
            status =  NU_NOT_REGISTERED;
    }

#ifdef INCLUDE_PROVIEW
    _RTProf_RegisterLisr(vector);
#endif /*INCLUDE_PROVIEW*/

    /* Release protection on the LISR registration list.  */
    TCT_Unprotect();

    /* Return the completion status.  */
    return(status);
}






