
/** @file
 *  Implementation of wwd_rtos.c for akos
 *
 *  This is the akos implementation of the RTOS
 *  abstraction layer.
 *  It provides Wiced with standard ways of using threads,
 *  semaphores and time functions
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "platform_toolchain.h"

#include "wwd_assert.h"
#include "wwd_rtos.h"
//#include "wwd_rtos_isr.h"
#include "RTOS/wwd_rtos_interface.h"

#include "akos_api.h"
#include "AKerror.h"

#define MAX_TASK_NUM    (128)
static struct thread_struct thread_list[MAX_TASK_NUM]={0};
static unsigned long m_id = 100;



/******************************************************
 *             Function definitions
 ******************************************************/
/*
static pthread_addr_t host_rtos_thread_wrapper( pthread_addr_t arg )
{
    host_thread_type_t* thread = arg;

    thread->entry_function( thread->arg );

    return NULL;
}
*/

wwd_result_t host_rtos_create_thread( host_thread_type_t* thread, void(*entry_function)( wwd_thread_arg_t ), const char* name, void* stack, uint32_t stack_size, uint32_t priority )
{
    return host_rtos_create_thread_with_arg( thread, entry_function, name, stack, stack_size, priority, NULL );
}

wwd_result_t host_rtos_create_configed_thread( host_thread_type_t* thread, void(*entry_function)( wwd_thread_arg_t ), const char* name, void* stack, uint32_t stack_size, uint32_t priority, host_rtos_thread_config_type_t* config )
{
    UNUSED_PARAMETER( config );
    return host_rtos_create_thread( thread, entry_function, name, stack, stack_size, priority );
}

wwd_result_t host_rtos_create_thread_with_arg( host_thread_type_t* thread, void(*entry_function)( wwd_thread_arg_t ), const char* name, void* stack, uint32_t stack_size, uint32_t priority, wwd_thread_arg_t arg )
{
    T_hTask handle;
	char thread_name[16];
	int i;
	int priority_map;
	host_thread_type_t thread_id;
	wwd_result_t result  = WWD_THREAD_CREATE_FAILED;

    thread_id = ((unsigned long)m_id << 12) >> 12;
	sprintf(thread_name, "%s", name);
    m_id++;
    
    //check id exist or not
    for (i = 0; i < MAX_TASK_NUM; i++)
    {
        if (thread_list[i].id == thread_id)
        {
            WPRINT_RTOS_ERROR(("same id"));
            return result;
        }       
    }
    
    //find a place for task handle	
    for (i = 0; i < MAX_TASK_NUM; i++)
    {
        if (thread_list[i].stack_addr == NULL)
            break;
    }
    if (i == MAX_TASK_NUM)
    {
        WPRINT_RTOS_ERROR(("err"));
        return result;
    }

    //init task sem
    if (host_rtos_init_semaphore(&thread_list[i].sem) != WWD_SUCCESS)
    {
        WPRINT_RTOS_ERROR(("err"));
        return result;
    }
   
    //malloc stack space
    thread_list[i].stack_addr = malloc(stack_size);
    if (thread_list[i].stack_addr == NULL)
    {
        host_rtos_deinit_semaphore(&thread_list[i].sem);
        WPRINT_RTOS_ERROR(("err"));
        return result;
    }

    //priority map , 0--99 map 255-0
	if (0 == priority)
		priority_map = 255;
	else if (99 == priority )
		priority_map = 0;
	else
	{
	    //first map 0-255
	    priority_map = priority * 256 /100 ;
		//second reverse
		priority_map = 255 - priority_map;
	}

    
    //create task
	handle = AK_Create_Task(entry_function, thread_name, (unsigned long)arg, arg, 
	            thread_list[i].stack_addr, (unsigned long)stack_size, priority_map, 
	            1, AK_PREEMPT, AK_NO_START);
	if (AK_IS_INVALIDHANDLE(handle))
	{
	    free(thread_list[i].stack_addr);
		thread_list[i].stack_addr = NULL;
        host_rtos_deinit_semaphore(&thread_list[i].sem);
        WPRINT_RTOS_ERROR(("err"));
	    return  result;
	}

    //record handle and then resume task
    thread_list[i].handle = handle;
    thread_list[i].id     = thread_id;
	*thread = thread_id;

	AK_Resume_Task(handle);
	result = WWD_SUCCESS;
	 
	return result;

	
#if 0	
	wwd_result_t       result      = WWD_THREAD_CREATE_FAILED;
    struct sched_param sched_param = { .sched_priority = (int)priority };
    int                err;
    pthread_attr_t     attr;

    UNUSED_PARAMETER( stack );

#ifndef RTOS_USE_DYNAMIC_THREAD_STACK
#error "Only dynamic thread stack supported"
#endif
    wiced_assert( "use dynamic stack allocation, no pre-allocated stacks", stack == NULL );

    err = pthread_attr_init( &attr );
    if ( err != 0 )
    {
        return result;
    }

    err = pthread_attr_setstacksize( &attr, (long)stack_size );
    if ( err != 0 )
    {
        goto error_exit;
    }

    err = pthread_attr_setschedparam( &attr, &sched_param );
    if ( err != 0 )
    {
        goto error_exit;
    }

    err = pthread_attr_setschedpolicy( &attr, SCHED_FIFO );
    if ( err != 0 )
    {
        goto error_exit;
    }

    err = pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
    if ( err != 0 )
    {
        goto error_exit;
    }

    thread->entry_function = entry_function;
    thread->arg            = arg;
    err = pthread_create( &thread->handle, &attr, host_rtos_thread_wrapper, thread );
    if ( err != 0 )
    {
        goto error_exit;
    }

    pthread_setname_np( thread->handle, name );

    result = WWD_SUCCESS;

error_exit:
    pthread_attr_destroy( &attr );

    return result;
#endif 
}

wwd_result_t host_rtos_finish_thread( host_thread_type_t* thread )
{
	T_hTask handle;
    int i;

	handle = AK_GetCurrent_Task();

     //find by id
    for (i = 0; i < MAX_TASK_NUM; i++)
    {
        if (thread_list[i].handle == handle)
            break;
    }
    if (i == MAX_TASK_NUM)
    {
        WPRINT_RTOS_ERROR(("err\n"));
        return WWD_THREAD_FINISH_FAIL;
    }

    //send exit signal
    host_rtos_set_semaphore(&thread_list[i].sem, 0);
	
	return WWD_SUCCESS;
#if 0
    int err;

    if (( thread == NULL ) || ( pthread_equal( thread->handle, pthread_self() ) != 0 ) )
    {
        pthread_exit( NULL );
        wiced_assert( "must never be here", 0 );
    }

    err = pthread_cancel( thread->handle ); /* Newly created thread has PTHREAD_CANCEL_ENABLE state */
    if ( err != 0 )
    {
        return WWD_THREAD_FINISH_FAIL;
    }

    return WWD_SUCCESS;
#endif
}

wwd_result_t host_rtos_delete_terminated_thread( host_thread_type_t* thread )
{
	
    return host_rtos_join_thread(thread);
	
#if 0
    int err;

    wiced_assert( "must be valid thread", thread != NULL );
    wiced_assert( "must not join itself", pthread_equal( thread->handle, pthread_self() ) == 0 );

    /* Thread is not detached, so free join structure otherwise memory leak. */
    err = pthread_join( thread->handle, NULL );
    if ( ( err != 0 ) && ( err != ESRCH ) )
    {
        return WWD_THREAD_FINISH_FAIL;
    }

    return WWD_SUCCESS;
#endif
}

wwd_result_t host_rtos_join_thread( host_thread_type_t* thread )
{
	
	wwd_result_t ret = WWD_SUCCESS;
	int i;

	 //find by id
	for (i=0; i<MAX_TASK_NUM; i++)
	{
		if (thread_list[i].id == *thread)
			break;
	}
	if (i == MAX_TASK_NUM)
	{
		WPRINT_RTOS_ERROR(("err\n"));
		return WWD_THREAD_DELETE_FAIL;
	}

	//wait thread quit
	host_rtos_get_semaphore(&thread_list[i].sem, NEVER_TIMEOUT, 0);

	//release resouce
	AK_Terminate_Task(thread_list[i].handle);
	if (AK_SUCCESS != (ret=AK_Delete_Task(thread_list[i].handle)))
	{
		WPRINT_RTOS_ERROR(("err\n"));
		ret = WWD_THREAD_DELETE_FAIL;
	}
	free(thread_list[i].stack_addr);
	thread_list[i].stack_addr = NULL;
	host_rtos_deinit_semaphore(&thread_list[i].sem);
	thread_list[i].id = 0;
	
	return ret;
#if 0
    int err;

    err = pthread_join( thread->handle, NULL );
    if ( err != 0 )
    {
        return WWD_THREAD_FINISH_FAIL;
    }

    return WWD_SUCCESS;
#endif 
}

wwd_result_t host_rtos_init_semaphore( host_semaphore_type_t* semaphore )
{
    int err;

	host_semaphore_type_t  handle;
	handle = AK_Create_Semaphore(0, AK_PRIORITY);

	if (handle !=AK_INVALID_SUSPEND)
	{
		*semaphore = handle;
		return WWD_SUCCESS ; 
	}else
		return WWD_SEMAPHORE_ERROR;
#if 0
    err = sem_init( semaphore, 0, 0 );
    if ( err != OK )
    {
        return WWD_SEMAPHORE_ERROR;
    }

    return WWD_SUCCESS;
#endif 
}

wwd_result_t host_rtos_get_semaphore( host_semaphore_type_t* semaphore, uint32_t timeout_ms, wiced_bool_t will_set_in_isr )
{
	int ret;
	
	UNUSED_PARAMETER( will_set_in_isr );
	
	if ( timeout_ms == NEVER_TIMEOUT )
    {
    	ret = AK_Obtain_Semaphore(*semaphore , AK_SUSPEND);
		
	}
	else if ( timeout_ms == 0 )
	{
		ret = AK_Obtain_Semaphore(*semaphore , AK_NO_SUSPEND);
	}
	else
	{
		//TODO: AK_Obtain_Semaphore suspend time interval is ms?
		ret = AK_Obtain_Semaphore(*semaphore , timeout_ms);
	}
	if (AK_SUCCESS == ret)
		return WWD_SUCCESS;
	else if(AK_TIMEOUT == ret)
		return WWD_TIMEOUT;
	else
		return WWD_SEMAPHORE_ERROR;


#if 0
	UNUSED_PARAMETER( will_set_in_isr );

    if ( timeout_ms == NEVER_TIMEOUT )
    {
        return ( sem_wait( semaphore ) == OK ) ? WWD_SUCCESS : WWD_SEMAPHORE_ERROR;
    }
    else if ( timeout_ms == 0 )
    {
      int err = sem_trywait( semaphore );

      if ( err != ERROR )
      {
        return WWD_SUCCESS;
      }
      else if ( errno == EAGAIN )
      {
        return WWD_TIMEOUT;
      }
      else
      {
        return WWD_SEMAPHORE_ERROR;
      }
    }
    else
    {
        irqstate_t      flags;
        struct timespec abstime;
        time_t          sec;
        int             err;

        flags = irqsave( );

        err = clock_gettime( CLOCK_REALTIME, &abstime );
        if ( err == ERROR )
        {
            wiced_assert( "get time failed", 0 );
        }
        else
        {
            sec              = timeout_ms / 1000;
            abstime.tv_sec  += sec;
            abstime.tv_nsec += (long)( timeout_ms - ( sec * 1000 ) ) * 1000000;
            if ( abstime.tv_nsec > 1000 * 1000 * 1000 )
            {
                abstime.tv_sec++;
                abstime.tv_nsec -= 1000 * 1000 * 1000;
            }

            err = sem_timedwait( semaphore, &abstime );
        }

        irqrestore( flags );

        if ( err != ERROR )
        {
            return WWD_SUCCESS;
        }
        else if ( ( errno == ETIMEDOUT ) || ( errno == EINTR ) )
        {
            return WWD_TIMEOUT;
        }
        else
        {
            return WWD_SEMAPHORE_ERROR;
        }
    }
#endif    
}

wwd_result_t host_rtos_set_semaphore( host_semaphore_type_t* semaphore, wiced_bool_t called_from_ISR )
{
	int ret;
	
	UNUSED_PARAMETER( called_from_ISR );
	
	ret = AK_Release_Semaphore(*semaphore);
	if (AK_SUCCESS == ret)
		return WWD_SUCCESS;
	else
		return WWD_SEMAPHORE_ERROR;

#if 0
    int err;

    UNUSED_PARAMETER( called_from_ISR );

    err = sem_post( semaphore );
    if ( err != OK )
    {
        return WWD_SEMAPHORE_ERROR;
    }

    return WWD_SUCCESS;
#endif 
}

wwd_result_t host_rtos_deinit_semaphore( host_semaphore_type_t* semaphore )
{
	int ret;
	ret = AK_Delete_Semaphore(*semaphore);
	if (AK_SUCCESS == ret)
		return WWD_SUCCESS;
	else
		return WWD_SEMAPHORE_ERROR;
#if 0
    int err;

    err = sem_destroy( semaphore );
    if ( err != OK )
    {
        return WWD_SEMAPHORE_ERROR;
    }

    return WWD_SUCCESS;
#endif 
}

wwd_time_t host_rtos_get_time( void )
{
	//TODO: time value 
	return get_tick_count();
#if 0
    return (wwd_time_t) ( clock_systimer_proxy( ) * ( 1000 / SYSTICK_FREQUENCY ) );
#endif
}

wwd_result_t host_rtos_delay_milliseconds( uint32_t num_ms )
{
	mini_delay(num_ms);
	return WWD_SUCCESS;
#if 0
    if ( num_ms )
    {
#ifdef CONFIG_DISABLE_SIGNALS
        static sem_t sleep_sem = SEM_INITIALIZER(0);
        wwd_result_t result;

        result = host_rtos_get_semaphore( &sleep_sem, num_ms, WICED_FALSE );
        if ( result == WWD_SUCCESS )
        {
            return WWD_SLEEP_ERROR;
        }
        else if ( result == WWD_TIMEOUT )
        {
            return WWD_SUCCESS;
        }
        else
        {
            return result;
        }
#else
        struct timespec tm;
        time_t          sec;
        int             err;

        sec        = num_ms / 1000;
        tm.tv_sec  = sec;
        tm.tv_nsec = (long)( num_ms - ( sec * 1000 ) ) * 1000000;

        err = nanosleep( &tm, NULL );
        if ( err != 0 )
        {
            return WWD_SLEEP_ERROR;
        }
#endif
    }

    return WWD_SUCCESS;
#endif
}

wwd_result_t host_rtos_init_queue( host_queue_type_t* queue, void* buffer, uint32_t buffer_size, uint32_t message_size )
{

#if 0
    uint32_t     message_num = buffer_size / message_size;
    wwd_result_t result;

    queue->message_num  = message_num;
    queue->message_size = message_size;
    queue->buffer       = buffer;

    queue->push_pos = 0;
    queue->pop_pos  = 0;

    result = host_rtos_init_semaphore( &queue->push_sem );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    result = host_rtos_init_semaphore( &queue->pop_sem );
    if ( result != WWD_SUCCESS )
    {
        if ( host_rtos_deinit_semaphore( &queue->push_sem ) != WWD_SUCCESS )
        {
            wiced_assert( "semaphore deinit failed", 0 );
        }
        return result;
    }

    return WWD_SUCCESS;
#endif
}

static wwd_result_t host_rtos_push_pop_queue( host_queue_type_t* queue, void* message, uint32_t timeout_ms, wiced_bool_t push  )
{
//TODO:
	
	wiced_assert( "must not exceeds max size", queue->occupancy < queue->message_num );

#if 0
    const bool             in_interrupt   = up_interrupt_context( );
    const uint32_t         occupancy_wait = push ? queue->message_num : 0;
    host_semaphore_type_t* get_sem        = push ? &queue->push_sem   : &queue->pop_sem;
    host_semaphore_type_t* set_sem        = push ? &queue->pop_sem    : &queue->push_sem;
    wwd_result_t           result;
    irqstate_t             flags;
    int                    op_result;
    int                    sem_value;

    if ( timeout_ms && in_interrupt )
    {
        return WWD_BADARG;
    }

    flags = irqsave( );

    if ( queue->occupancy == occupancy_wait )
    {
        if ( in_interrupt )
        {
            result = WWD_TIMEOUT;
        }
        else
        {
            result = host_rtos_get_semaphore( get_sem, timeout_ms, WICED_FALSE );
        }

        if ( result != WWD_SUCCESS )
        {
            irqrestore( flags );
            return result;
        }
    }

    if ( push == WICED_TRUE )
    {
        wiced_assert( "must not exceeds max size", queue->occupancy < queue->message_num );

        memcpy( &queue->buffer[queue->push_pos * queue->message_size], message, queue->message_size );
        queue->push_pos = (queue->push_pos + 1U) % queue->message_num;

        queue->occupancy++;
    }
    else
    {
        wiced_assert( "must be non zero", queue->occupancy > 0 );

        memcpy( message, &queue->buffer[queue->pop_pos * queue->message_size], queue->message_size );
        queue->pop_pos = (queue->pop_pos + 1U) % queue->message_num;

        queue->occupancy--;
    }

    op_result = sem_getvalue( set_sem, &sem_value );
    if ( op_result != OK )
    {
        result = WWD_SEMAPHORE_ERROR;
        wiced_assert( "semaphore get value failed", 0 );
    }
    else if ( sem_value < 0 )
    {
        result = host_rtos_set_semaphore( set_sem, WICED_FALSE );
        wiced_assert( "semaphore set failed", result == WWD_SUCCESS );
    }
    else
    {
        result = WWD_SUCCESS;
    }

    irqrestore( flags );

    return result;
#endif
}

wwd_result_t host_rtos_push_to_queue( host_queue_type_t* queue, void* message, uint32_t timeout_ms )
{
    return host_rtos_push_pop_queue( queue, message, timeout_ms, WICED_TRUE );
}

wwd_result_t host_rtos_pop_from_queue( host_queue_type_t* queue, void* message, uint32_t timeout_ms )
{
    return host_rtos_push_pop_queue( queue, message, timeout_ms, WICED_FALSE );
}

wwd_result_t host_rtos_deinit_queue( host_queue_type_t* queue )
{
	//TODO:
	
#if 0
    wwd_result_t result = WWD_SUCCESS;
    wwd_result_t op_result;

    op_result = host_rtos_deinit_semaphore( &queue->push_sem );
    if ( op_result != WWD_SUCCESS )
    {
        result = op_result;
    }

    op_result = host_rtos_deinit_semaphore( &queue->pop_sem );
    if ( op_result != WWD_SUCCESS )
    {
        result = op_result;
    }

    return result;
#endif
}
