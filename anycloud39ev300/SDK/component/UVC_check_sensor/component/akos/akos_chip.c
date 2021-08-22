#ifdef OS_ANYKA

#include "nucleus.h"
#include "akos_api.h"
#include "Fwl_osMalloc.h"
#include "string.h"
#include "qu_defs.h"
#include "tc_extr.h"


#define CBYTE(size)         ((size)>>2)

typedef T_VOID (*THREADFUNC)(T_U32, T_VOID *);

// Task Control API
T_hTask AK_Create_Task(T_VOID *task_entry, T_U8 *name, T_U32 argc, T_VOID *argv,
                        	T_VOID *stack_address, T_U32 stack_size, T_OPTION priority, 
                        	T_U32 time_slice, T_OPTION preempt, T_OPTION auto_start)
{
    T_S32 status, task;
    
	//printf("create task name:%s, stack_size:%d, priority:%d\n",name,stack_size,priority);
	task = (T_hTask)Fwl_Malloc(sizeof(NU_TASK));
	if (AK_NULL == (T_VOID*)task)
    {   
    	return AK_MEMORY_CORRUPT;
    }
    
    memset((T_VOID*)task, 0, sizeof(NU_TASK));	
    memset((T_U8*)stack_address, 0x5a, stack_size); //fill stack with magic word
    
	status = NU_Create_Task((NU_TASK*)task, name, (THREADFUNC)task_entry,
                    argc, argv, stack_address, stack_size, 
                    priority, time_slice, preempt, auto_start);
    if (AK_SUCCESS == status)
    {
        return task;
    }


    task = (T_hTask)Fwl_Free((T_VOID*)task);    
    return status;

}


T_S32 AK_Delete_Task(T_hTask task)
{
	T_S32 status;

	status = NU_Delete_Task((NU_TASK*)task);
    if (AK_SUCCESS == status)
    {
        task = (T_hTask)Fwl_Free((T_VOID*)task);
    }
    return status;
}

T_S32 AK_Suspend_Task(T_hTask task)
{
    return NU_Suspend_Task((NU_TASK *)task);
}

T_S32 AK_Resume_Task(T_hTask task)
{
	return NU_Resume_Task((NU_TASK*)task);
}

T_S32 AK_Terminate_Task(T_hTask task)
{
	return NU_Terminate_Task((NU_TASK*)task);
}

T_S32 AK_Reset_Task(T_hTask task, T_U32 argc, T_VOID *argv)
{
    return NU_Reset_Task((NU_TASK*)task, argc, argv);
}

T_VOID AK_Sleep(T_U32 ticks)
{
    NU_Sleep(ticks);
}

T_VOID AK_Relinquish(T_VOID)
{
    NU_Relinquish();
}
/*
T_U32 AK_Change_Time_Slice(T_hTask task, T_U32 time_slice)
{
    return NU_Change_Time_Slice((NU_TASK*)task, time_slice);
}
*/
T_OPTION AK_Change_Priority(T_hTask task, T_OPTION new_priority)
{
    return NU_Change_Priority((NU_TASK*)task, new_priority);
}

T_S32 AK_Task_Status(T_hTask task)
{    
    if ((task != NU_NULL) && (((TC_TCB *)task) -> tc_id == TC_TASK_ID))
    {
        return ((TC_TCB *)task) -> tc_status;
    }
    else
        return AK_INVALID_TASK;
}

T_U32 AK_Check_Task_Stack(T_VOID)
{
    NU_TASK *list[30];
    T_U32 i, num,ret=1;
    T_S8 name[64];
    T_U8 status;
    T_U32 cnt;
    T_U8 prio, pree;
    T_U32 slice;
    T_VOID *base;
    T_U32 stack_size;
    T_U32 mini_size;
    
    num = NU_Task_Pointers(&list[0], 30);
    for (i=0; i<num; i++)
    {
        if (NU_SUCCESS == NU_Task_Information(list[i], name, &status, &cnt, &prio, &pree, &slice, &base, &stack_size, &mini_size))
        {
            if (*((T_U8*)base) != 0x5a)
            {
                printf("name:%s, ", name);
                printf("stat:%d, ", status);
                printf("prio:%d, ", prio);
                printf("pree:%d, ", pree);
                printf("sched:%d, ", cnt);
                printf("slice:%d, ", slice);
                printf("stack:%x, ", base);
                printf("size:%d, ", stack_size);
                printf("stack overflow!!\n");
                ret = 0;
            }
        }
    }
    return ret;
}


/*
T_OPTION AK_Change_Preemption(T_OPTION preempt)
{
    return NU_Change_Preemption(preempt);
}
*/
/*
T_U32 AK_Check_Stack(T_VOID)
{
    return NU_Check_Stack();
}
*/
//T_hTask AK_Current_Task_Pointer(T_VOID)
/*
T_U32 AK_Established_Tasks(T_VOID)
{
    return NU_Established_Tasks();
}
*/
/*
T_U32 AK_Task_Pointers(T_hTask *pointer_list, T_U32 maximum_pointers)
{
    T_U32 task_num, for_count, i;
    NU_TASK *task_list_pointer, *pointer_bak;
    
    task_list_pointer = Fwl_Malloc(sizeof(NU_TASK*)*maximum_pointers);
    pointer_bak = task_list_pointer;

    task_num = NU_Task_Pointers(&task_list_pointer, maximum_pointers);

    if (task_num >= maximum_pointers)
        for_count = maximum_pointers;
    else
        for_count = task_num;

    for (i=0; i<for_count; i++)
    {
        pointer_list[i] = *task_list_pointer;
        task_list_pointer += sizeof(NU_TASK*);
    }

    Fwl_Free(pointer_bak);

    return task_num;
}
*/
T_hTask AK_GetCurrent_Task(T_VOID)
{
    return (T_hTask)NU_Current_Task_Pointer();
}

T_S32 AK_Task_Information(T_hTask task, CHAR *name, T_OPTION *task_status, T_U32 *scheduled_count,
                               T_OPTION *priority, T_OPTION *preempt, T_U32 *time_slice,
                                T_VOID **stack_base, T_U32 *stack_size, T_U32 *minimum_stack)
{
    return NU_Task_Information((NU_TASK*)task, name, task_status, scheduled_count, 
                         priority, preempt, time_slice, stack_base, stack_size, minimum_stack);
}

#ifdef DEBUG_TRACE_TASK_INFO

unsigned long TCC_GetTaskTimeInfo(T_U32 taskid, int Printflag);

T_VOID AK_Task_clear_time(T_VOID)
{
	TCC_ClearSysTaskTimeInfo();
}
#endif


T_VOID AK_List_Task(T_U8 prio_format, T_U8 time_format)
{
    NU_TASK *list[30];
    T_U32 i, num,j;
    T_S8 name[64];
    T_U8 status;
    T_U32 cnt;
    T_U8 prio, pree;
    T_U32 slice;
    T_VOID *base;
    T_U32 stack_size;
    T_U32 mini_size;
	#ifdef DEBUG_TRACE_TASK_INFO
    T_U32 tol_time;
    
	TCT_Schedule_exit_time();
	#endif
    num = NU_Task_Pointers(&list[0], 30);
    printf("akos lib:%s\n", LIB_VER);
    printf("--------------------------------------- task number %d ---------------------------------------\n", num);
    //AK_Check_Task_Stack();
    #ifdef DEBUG_TRACE_TASK_INFO
    printf("name\tstat\tprio(%d)\tslice\tsched\t\t\ttime(%d)\t\tstack\t\tsize\tused\r\n", prio_format, time_format);
	#else
    printf("name\tstat\tprio(%d)\tslice\tsched\t\t\tstack\t\tsize\tused\r\n", prio_format, time_format);
	#endif
	for (i=0; i<num; i++)
    {
        if (NU_SUCCESS == NU_Task_Information(list[i], name, &status, &cnt, &prio, &pree, &slice, &base, &stack_size, &mini_size))
        {
			#ifdef DEBUG_TRACE_TASK_INFO
            tol_time = TCC_GetTaskTimeInfo((T_U32)list[i], time_format);
			#endif

            //priority format convert from [0,255] to [0,99]
            if (prio_format > 0)
            {
            	if (0 == prio)
            		prio = 99;
            	else if (255 == prio )
            		prio = 0;
            	else
            	{
            	    //first map 0-255
            	    prio = prio * 100 /256 ;
            		//second reverse
            		prio = 100 - prio;
            	}
            }
            
			name[7] = '\0';
            printf("%s\t", name);
            printf("%d\t", status);
            printf("%d\t", prio);
            printf("%d\t", slice);
            //printf("%d\t", pree);
            printf("%lu\t\t", cnt);
			#ifdef DEBUG_TRACE_TASK_INFO
			//if (time_format > 0)
            	printf("\t%lu\t", tol_time);
			//else
				//printf("%lu\t", tol_time);
			#endif
            printf("\t%x\t", base);
            printf("%d\t", stack_size);
			
            for (j=0; j<stack_size; j++)
            {
                if (*((T_U8*)base+j) != 0x5a && j==0)
                {
                    printf("stack overflow!!\n");
                    break;
                }
                
                if (*((T_U8*)base+j) != 0x5a)
                {
                    printf("%d\n", stack_size-j);
                    break;
                }
            }
        }
        else
        {
            printf("get task i failed\n", i);
        }
    }
    //printf("----------------------------------- task list end -----------------------------------\n");
}

// Queue Control API
T_hQueue AK_Create_Queue(T_VOID *start_address, T_U32 queue_size, 
                         T_OPTION message_type, T_U32 message_size, T_OPTION suspend_type)
{
    T_S32 status, queue;
	
	queue = (T_hQueue)Fwl_Malloc(sizeof(NU_QUEUE));
	if (AK_NULL == (T_pVOID*)queue)
    {
        return AK_MEMORY_CORRUPT;
    }
    memset((T_VOID*)queue, 0, sizeof(NU_QUEUE));
	status = NU_Create_Queue((NU_QUEUE*)queue, "queue", start_address, 
                              CBYTE(queue_size), message_type, 
                              CBYTE(message_size), suspend_type);
    
    if (AK_SUCCESS == status)
    {
        return queue;
    }
    queue = (T_hQueue)Fwl_Free((T_VOID*)queue);
    return status;
}

T_S32 AK_Delete_Queue(T_hQueue queue)
{
	T_S32 status;

	status = NU_Delete_Queue((NU_QUEUE*)queue);
    if(AK_SUCCESS == status)
    {
        queue = (T_hQueue)Fwl_Free((T_VOID*)queue);
    }

    return status;
}

T_S32 AK_Send_To_Queue(T_hQueue queue, T_VOID *message, 
                              T_U32 size, T_U32 suspend)
{
    return NU_Send_To_Queue((NU_QUEUE*)queue, message, CBYTE(size), suspend);
}

T_S32 AK_Send_To_Front_of_Queue(T_hQueue queue, T_VOID *message, 
                              T_U32 size, T_U32 suspend)
{
    return NU_Send_To_Front_Of_Queue((NU_QUEUE*)queue, message, CBYTE(size), suspend);
}

T_S32 AK_Broadcast_To_Queue(T_hQueue queue, T_VOID *message, 
                              T_U32 size, T_U32 suspend)
{
    return NU_Broadcast_To_Queue((NU_QUEUE*)queue, message, CBYTE(size), suspend);
}

T_S32 AK_Receive_From_Queue(T_hQueue queue, T_VOID *message, T_U32 size, 
                            T_U32 *actual_size, T_U32 suspend)
{
    T_S32 status;
    status = NU_Receive_From_Queue((NU_QUEUE*)queue, message, CBYTE(size), actual_size, suspend);
    *actual_size = (*actual_size)<<2;
    return status;
}

T_S32 AK_Send_Unique_To_Queue(T_hQueue queue_ptr, T_VOID *message, T_U32 size, 
                                    T_U32 suspend, CallbakCompare Function)
{
    QU_QCB *queue;
    T_U32 *source, *end;
    T_BOOL wrap;
    T_U32 t_size;
    
    t_size = CBYTE(size);
    wrap = AK_FALSE;
    queue = (QU_QCB*) queue_ptr;

    /* Determine if there is an error with the queue pointer.  */
    if (queue == AK_NULL)
    
        /* Indicate that the queue pointer is invalid.  */
        return  AK_INVALID_QUEUE;
        
    else if (queue -> qu_id != QU_QUEUE_ID)
    
        /* Indicate that the queue pointer is invalid.  */
        return  AK_INVALID_QUEUE;
	
    else if (message == AK_NULL)
    
        /* Indicate that the pointer to the message is invalid.  */
        return AK_INVALID_POINTER;

    source = queue->qu_read;
    end = queue->qu_write;

    if(source > end)
        wrap = AK_TRUE;

    // no message    
    if( 0 == queue->qu_messages )
        return AK_Send_To_Queue(queue_ptr, message, size, suspend);;

    while(1)
    {        
        if(!Function((T_VOID*)source, (T_VOID*)message))
        {
            source += t_size;

            // check for the end of the queue
            if((!wrap) && (source >= end))
            {
                break;
            }

            // check again for wrap-around condition on the read pointer.
            if((wrap) && (source >= queue->qu_end))
            {
                source =  queue->qu_start;
                wrap = AK_FALSE;

                // to avoid two pointer all at qu_start and has got the end
                if (source == end)
                    break;
            }
        }
        else
        {
        	// do not pass the function check, replace the same message
			memcpy((T_VOID*)source, (T_VOID*)message, size);
			return AK_EXIST_MESSAGE;
        }
    }
	// no same message, send to queue
    return AK_Send_To_Queue(queue_ptr, message, size, suspend);
}

T_S32 AK_Send_Unique_To_Front_of_Queue(T_hQueue queue_ptr, T_VOID *message, 
                                    T_U32 size, T_U32 suspend, CallbakCompare Function)
{
    QU_QCB *queue;
    T_U32 *source, *end;
    T_BOOL wrap = AK_FALSE;
    T_U32 t_size;
    
    t_size = CBYTE(size);
    queue = (QU_QCB*) queue_ptr;

    /* Determine if there is an error with the queue pointer.  */
    if (queue == AK_NULL)
    
        /* Indicate that the queue pointer is invalid.  */
        return  AK_INVALID_QUEUE;
        
    else if (queue -> qu_id != QU_QUEUE_ID)
    
        /* Indicate that the queue pointer is invalid.  */
        return  AK_INVALID_QUEUE;
	
    else if (message == AK_NULL)
    
        /* Indicate that the pointer to the message is invalid.  */
        return  AK_INVALID_POINTER;
    source = queue->qu_read;
    end = queue->qu_write;

    if(source > end)
        wrap = AK_TRUE;
    
    // no message    
    if( 0 == queue->qu_messages )
        return AK_Send_To_Front_of_Queue(queue_ptr, message, size, suspend);;

    while(1)
    {        
        if(!Function((T_VOID*)source, (T_VOID*)message))
        {
            source += t_size;

            // check for the end of the queue
            if((!wrap) && (source >= end))
            {
                break;
            }

            // check again for wrap-around condition on the read pointer.
            if((wrap) && (source >= queue->qu_end))
            {
                source =  queue->qu_start;
                wrap = AK_FALSE;

                // to avoid two pointer all at qu_start and has got the end
                if (source == end)
                    break;
            }
        }
        else
        {
        	// do not pass the function check, replace the same message
        	memcpy((T_VOID*)source, (T_VOID*)message, size);
            return AK_EXIST_MESSAGE;
       	}
    }

	// no same message, send to queue
    return AK_Send_To_Front_of_Queue(queue_ptr, message, size, suspend);
}


T_S32 AK_Reset_Queue(T_hQueue queue)
{
    return NU_Reset_Queue((NU_QUEUE*)queue);
}


T_S32 AK_Queue_Information( T_hQueue queue, T_VOID **start_address, T_U32 *Queue_size, 
                                T_U32 *available, T_U32 *messages, T_OPTION *message_type, T_U32 *message_size, 
                                T_OPTION *suspend_type, T_U32 *tasks_waiting, T_hTask *first_task)
{
	CHAR name[8];
    return NU_Queue_Information((NU_QUEUE*)queue, name, start_address, Queue_size, 
									available, messages, message_type, message_size, 
									suspend_type, tasks_waiting, (NU_TASK**)first_task);
}


T_U32 AK_Established_Queues(T_VOID)
{
    return NU_Established_Queues();
}

// Mailbox API
T_hMailbox AK_Create_Mailbox(T_OPTION suspend_type)
{
    T_S32 status, mailbox;
    
    mailbox = (T_hMailbox)Fwl_Malloc(sizeof(NU_MAILBOX));
    if (AK_NULL == (T_VOID*)mailbox)
    {
        return AK_MEMORY_CORRUPT;
    }
    memset((T_VOID*)mailbox, 0, sizeof(NU_MAILBOX));
    status = NU_Create_Mailbox((NU_MAILBOX*)mailbox, "Mailbox", suspend_type);

    if(AK_SUCCESS == status)
    {
        return mailbox;
    }
    mailbox = (T_hMailbox)Fwl_Free((T_VOID*)mailbox);
    return status;
}

T_S32 AK_Delete_Mailbox(T_hMailbox mailbox)
{
	T_S32 status;

	status = NU_Delete_Mailbox((NU_MAILBOX*)mailbox);
    if(AK_SUCCESS == status)
    {
        mailbox = (T_hMailbox)Fwl_Free((T_VOID*)mailbox);
    }

    return status;
}

/*T_S32 AK_Reset_Mailbox(T_hMailbox mailbox)
{
    return NU_Reset_Mailbox((NU_MAILBOX*)mailbox);
}
*/

T_S32 AK_Broadcast_To_Mailbox(T_hMailbox mailbox, T_U32 *message, T_OPTION suspend_type)
{
    return NU_Broadcast_To_Mailbox((NU_MAILBOX*)mailbox, message, suspend_type);
}

T_S32 AK_Receive_From_Mailbox(T_hMailbox mailbox, T_U32 *message, T_OPTION suspend_type)
{
    return NU_Receive_From_Mailbox((NU_MAILBOX*)mailbox, message, suspend_type);
}

T_S32 AK_Send_To_Mailbox(T_hMailbox mailbox, T_U32 *message, T_OPTION suspend_type)
{
    return NU_Send_To_Mailbox((NU_MAILBOX*)mailbox, message, suspend_type);
}

T_U32 AK_Established_Mailboxes()
{
    return NU_Established_Mailboxes();
}

T_S32 AK_Mailbox_Information(T_hMailbox mailbox, T_OPTION *suspend_type, 
                             T_OPTION *message_present, T_U32 *tasks_waiting, T_hTask *first_task)
{
	CHAR name[8];
    return NU_Mailbox_Information((NU_MAILBOX*)mailbox, name, suspend_type, 
                                   message_present, tasks_waiting, (NU_TASK**)first_task);
}


// Semaphore Control API
T_hSemaphore AK_Create_Semaphore(T_U32 initial_count, T_OPTION suspend_type)
{
    T_S32 status, semaphore;

    semaphore = (T_hSemaphore)Fwl_Malloc(sizeof(NU_SEMAPHORE));
    if (AK_NULL == (T_VOID*)semaphore)
    {
        return AK_MEMORY_CORRUPT;
    }
    memset((T_VOID*)semaphore, 0, sizeof(NU_SEMAPHORE));

    status = NU_Create_Semaphore((NU_SEMAPHORE*)semaphore,"smph", 
                                    initial_count, suspend_type);
    if(AK_SUCCESS == status)
    {
        return semaphore;
    }
    
    semaphore = (T_hSemaphore)Fwl_Free((T_VOID*)semaphore);

    return status;


}

T_S32 AK_Delete_Semaphore(T_hSemaphore semaphore)
{
    T_S32 status = AK_SUCCESS;
    status = NU_Delete_Semaphore((NU_SEMAPHORE*)semaphore);
    if (AK_SUCCESS == status)
    {
        semaphore = (T_hSemaphore)Fwl_Free((T_VOID*)semaphore);
    }
    
    return status;
}

T_S32 AK_Obtain_Semaphore(T_hSemaphore semaphore, T_U32 suspend)
{
    return NU_Obtain_Semaphore((NU_SEMAPHORE*)semaphore, suspend);
}

T_S32 AK_Release_Semaphore(T_hSemaphore semaphore)
{
    return NU_Release_Semaphore((NU_SEMAPHORE*)semaphore);
}


T_S32 AK_Reset_Semaphore(T_hSemaphore semaphore, T_U32 initial_count)
{
    return NU_Reset_Semaphore((NU_SEMAPHORE *)semaphore, initial_count);
}
/*
T_U32 AK_Established_Semaphores(T_VOID)
{
    return NU_Established_Semaphores();
}
*/
T_S32 AK_Semaphore_Information(T_hSemaphore semaphore, T_U32 *current_count,
                                    T_OPTION *suspend_type, T_U32 *tasks_waiting, T_hTask *first_task)
{
	CHAR name[8];
    return NU_Semaphore_Information((NU_SEMAPHORE *)semaphore, name, current_count,
                                    suspend_type, tasks_waiting, (NU_TASK**)first_task);
}

// Event Group API
T_hEventGroup AK_Create_Event_Group(T_VOID)
{
    T_S32 status, eventgroup;

    eventgroup = (T_hEventGroup)Fwl_Malloc(sizeof(NU_EVENT_GROUP));
    if (AK_NULL == (T_VOID*)eventgroup)
    {
        return AK_MEMORY_CORRUPT;
    }
    memset((T_VOID*)eventgroup, 0, sizeof(NU_EVENT_GROUP));

    status = NU_Create_Event_Group((NU_EVENT_GROUP*)eventgroup, "name");
    
    if(AK_SUCCESS == status)
    {
        return eventgroup;
    }

    eventgroup = (T_hEventGroup)Fwl_Free((T_VOID*)eventgroup);

    return status;


}
T_S32 AK_Delete_Event_Group(T_hEventGroup eventgroup)
{
    T_S32 status = AK_SUCCESS;
    status = NU_Delete_Event_Group((NU_EVENT_GROUP*)eventgroup);
    if (AK_SUCCESS == status)
    {
        eventgroup = (T_hEventGroup)Fwl_Free((T_VOID*)eventgroup);
    }
    return status;
}

T_S32 AK_Retrieve_Events(T_hEventGroup eventgroup, T_U32 requested_events, 
                    T_OPTION operation, T_U32 *retrieved_events, T_U32 suspend)
{
    return NU_Retrieve_Events((NU_EVENT_GROUP*)eventgroup, requested_events,
                                operation, retrieved_events, suspend);
}

T_S32 AK_Set_Events(T_hEventGroup eventgroup, T_U32 event_flags, OPTION operation)
{
    return NU_Set_Events((NU_EVENT_GROUP*)eventgroup, event_flags, operation);
}
T_U32 AK_Established_Event_Groups(T_VOID)
{
    return NU_Established_Event_Groups();
}


T_S32 AK_Event_Group_Information
	(T_hEventGroup group, T_U32 *event_flags, T_U32 *tasks_waiting, T_hTask *first_task)
{
	CHAR name[8];
    return NU_Event_Group_Information((NU_EVENT_GROUP*)group, name,
                           event_flags, tasks_waiting, first_task);
}


// Timer API
T_hTimer AK_Create_Timer(T_VOID (*expiration_routine)(T_U32), 
                T_U32 id, T_U32 initial_time, T_U32 reschedule_time, T_OPTION enable)
{
    T_S32 status, timer;

    timer = (T_hTimer)Fwl_Malloc(sizeof(NU_TIMER));
    if (AK_NULL == (T_VOID*)timer)
        return AK_MEMORY_CORRUPT;

    memset((T_VOID*)timer, 0, sizeof(NU_TIMER));
    
    status = NU_Create_Timer((NU_TIMER*)timer, "name", expiration_routine,
                        id, initial_time, reschedule_time, enable);
    
    if(AK_SUCCESS == status)
    {
        return timer;
    }

    timer = (T_hTimer)Fwl_Free((T_VOID*)timer);
    
    return status;


}

T_S32 AK_Delete_Timer(T_hTimer timer)
{
    T_S32 status;
    status = NU_Control_Timer((NU_TIMER*)timer, NU_DISABLE_TIMER);
    status = NU_Delete_Timer((NU_TIMER*)timer);
    if(AK_SUCCESS == status)
    {
        timer = (T_hTimer)Fwl_Free((T_VOID*)timer);
    }
    return status;    
}

T_S32 AK_Control_Timer(T_hTimer timer, T_OPTION enable)
{
    return NU_Control_Timer((NU_TIMER*)timer, enable);
}

/*
T_S32 AK_Reset_Timer(T_hTimer timer, T_VOID (*expiration_routine)(T_U32), T_U32 initial_time, T_U32 reschedule_time, T_OPTION enable)
{
    
    return NU_Reset_Timer(timer, expiration_routine, initial_time, reschedule_time, enable);
}

T_VOID AK_Set_Clock(T_U32 new_value)
{
    NU_Set_Clock(new_value);
}

T_U32 AK_Retrieve_Clock(T_VOID)
{
    return NU_Retrieve_Clock();
}

T_U32 AK_Established_Timers(T_VOID)
{
    return NU_Established_Timers();
}
*/
T_S32 AK_Timer_Information(T_hTimer timer, OPTION *enable, T_U32 *expirations,
                            	T_U32 *id, T_U32 *initial_time,	T_U32 *reschedule_time)
{
    T_U8 name[8];
    return NU_Timer_Information((NU_TIMER*)timer, name, enable, expirations,
                                id, initial_time, reschedule_time);
}

// Interrupt Control API
T_hHisr AK_Create_HISR(T_VOID (*hisr_entry)(T_VOID), T_U8 *name, 
                 T_OPTION priority, T_VOID *stack_address, T_U32 stack_size)
{
    T_S32 status, hisr;
    hisr = (T_hHisr)Fwl_Malloc(sizeof(NU_HISR));
    if  (AK_NULL == (T_VOID*)hisr)
    {
        return AK_MEMORY_CORRUPT;
    }

    memset((T_VOID*)hisr, 0, sizeof(NU_HISR));

    status = NU_Create_HISR((NU_HISR*)hisr, name, hisr_entry, 
                            priority, stack_address, stack_size);
    
    if(AK_SUCCESS == status)
    {
        return hisr;
    }

    hisr = (T_hHisr)Fwl_Free((T_VOID*)hisr);
    return status;

}

T_S32 AK_Activate_HISR(T_hHisr hisr)
{
    return NU_Activate_HISR((NU_HISR*)hisr);
}

T_S32 AK_Delete_HISR(T_hHisr hisr)
{
    T_S32 status = AK_SUCCESS;

    status = NU_Delete_HISR((NU_HISR*)hisr);
    if (AK_SUCCESS == status)
    {
        hisr = (T_hHisr)Fwl_Free((T_VOID*)hisr);
        
    }
    return status;
}

T_S32 AK_Register_LISR( T_S32 vector, T_VOID (*list_entry)(T_S32), 
                        T_VOID (**old_lisr)(T_S32))
{
    return NU_Register_LISR(vector, list_entry, old_lisr);
}

T_VOID *AK_Setup_Vector(T_S32 vector, T_VOID *new_vector)
{
    return NU_Setup_Vector(vector, new_vector);
}

T_U32 AK_Established_HISRs()
{
    return NU_Established_HISRs();
}

T_S32 AK_HISR_Information(  T_hHisr hisr, T_U8 *name, T_U32 *scheduled_cout, 
                            T_OPTION *priority, T_VOID **stack_base,
                            T_U32 *stack_size, T_U32 *minimum_stack)
{
    return NU_HISR_Information((NU_HISR*)hisr, name, scheduled_cout, priority,
                                stack_base, stack_size, minimum_stack);
    
}

static NU_TASK				TmcTask;
T_U32 AK_System_Init(T_VOID *task_entry, T_U8 *name, T_VOID *stack_address, T_U32 stack_size)
{
	T_S32 status;
	T_U32 ret = AK_FALSE;

	memset(&TmcTask, 0, sizeof(TmcTask));
	memset(stack_address, 0x5a, stack_size);
	
	#ifdef DEBUG_TRACE_TASK_INFO
	TCC_InitSysTaskInfo();
	#endif
	status = NU_Create_Task(&TmcTask, name, (THREADFUNC)task_entry,
                    0, AK_NULL, stack_address, stack_size, 
                    10, 1, AK_PREEMPT, AK_START);
    if (AK_SUCCESS == status)
    {
        ret = AK_TRUE;
    }

	return ret;
}

//T_U32 AK_HISR_Pointers();
//T_hHisr *AK_Current_HISR_Pointer(T_VOID);
/* system protect fuction */

T_VOID AK_System_Protect(T_VOID)
{
    TCT_System_Protect();	
}

T_VOID AK_System_Unprotect(T_VOID)
{
    TCT_System_Unprotect();
}
T_VOID AK_Drv_Protect(T_VOID)
{
    TCT_System_Protect();	
}

T_VOID AK_Drv_Unprotect(T_VOID)
{
    TCT_System_Unprotect();
}

//only use for other core moduls
T_VOID AK_Printf(const char *s)
{
	printf("%s",s);
}
#endif

