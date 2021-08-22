/*
 * Broadcom Proprietary and Confidential. Copyright 2016 Broadcom
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/* lwIP includes. */
#include <pthread.h>
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wiced_utilities.h"
#include "wiced_crypto.h"

/* Message queue constants. */
#define archMESG_QUEUE_LENGTH     ( (unsigned long) 6 )
#define archPOST_BLOCK_TIME_MS    ( ( unsigned long ) 10000 )

/* The timeout code seems to be unused */
#if LWIP_SYS_ARCH_TIMEOUTS
struct timeout_list
{
    struct sys_timeouts timeouts;
    xTaskHandle pid;
};
static struct timeout_list timeout_list[SYS_THREAD_MAX];
static uint16_t next_thread = 0;
#endif /* if LWIP_SYS_ARCH_TIMEOUTS */

/* This is the number of threads that can be started with sys_thread_new() */
#define SYS_THREAD_MAX                 (4)

#define lwipTCP_STACK_SIZE             (600)
#define lwipBASIC_SERVER_STACK_SIZE    (250)

/*-----------------------------------------------------------------------------------
 * Creates an empty mailbox.
 */
err_t sys_mbox_new( /*@special@*/ /*@out@*/ sys_mbox_t *mbox, /*@unused@*/int size ) /*@allocates *mbox@*/  /*@defines **mbox@*/
{
    /*@-noeffect@*/
    (void) size; /* unused parameter */
    /*@+noeffect@*/

    host_rtos_init_queue( mbox, NULL, archMESG_QUEUE_LENGTH* sizeof(void *) , sizeof(void *) );
    /*@-compdef@*/ /* Lint doesnt realise allocation has occurred */
    return ERR_OK;
    /*@+compdef@*/
}

/*-----------------------------------------------------------------------------------
 * Deallocates a mailbox. If there are messages still present in the
 * mailbox when the mailbox is deallocated, it is an indication of a
 * programming error in lwIP and the developer should be notified.
 */
void sys_mbox_free( /*@special@*/ sys_mbox_t *mbox ) /*@releases *mbox@*/
{
    host_rtos_deinit_queue( mbox );
}

/*-----------------------------------------------------------------------------------
 * Posts the "msg" to the mailbox.
 */
void sys_mbox_post( sys_mbox_t *mbox, void *msg )
{
    wwd_result_t retval;
    retval = host_rtos_push_to_queue( mbox, &msg, 20000 );
    LWIP_ASSERT("Error posting to LwIP mailbox", retval == WWD_SUCCESS );
}

/*-----------------------------------------------------------------------------------
 * Blocks the thread until a message arrives in the mailbox, but does
 * not block the thread longer than "timeout" milliseconds (similar to
 * the sys_arch_sem_wait() function). The "msg" argument is a result
 * parameter that is set by the function (i.e., by doing "*msg =
 * ptr"). The "msg" parameter maybe NULL to indicate that the message
 * should be dropped.
 *
 * The return values are the same as for the sys_arch_sem_wait() function:
 * Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
 * timeout.
 *
 * Note that a function with a similar name, sys_mbox_fetch(), is
 * implemented by lwIP.
 */
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, /*@null@*/ /*@out@*/ void **msg, u32_t timeout)
{
    void *dummyptr;
    void ** tmp_ptr;
    wwd_time_t start_time, end_time, elapsed_time;

    start_time = host_rtos_get_time( );

    if ( msg == NULL )
    {
        tmp_ptr = &dummyptr;
    }
    else
    {
        tmp_ptr = msg;
    }

    if ( timeout != 0 )
    {
        if ( WWD_SUCCESS == host_rtos_pop_from_queue( mbox, tmp_ptr, timeout ) )
        {
            end_time = host_rtos_get_time( );
            elapsed_time = end_time - start_time;
            if ( elapsed_time == 0 )
            {
                elapsed_time = (wwd_time_t) 1;
            }
            return ( elapsed_time );
        }
        else /* timed out blocking for message */
        {
            if ( msg != NULL )
            {
                *msg = NULL;
            }
            return SYS_ARCH_TIMEOUT;
        }
    }
    else /* block forever for a message. */
    {
        if ( host_rtos_pop_from_queue( mbox, &(*tmp_ptr), NEVER_TIMEOUT ) == WWD_TIMEOUT )
        {
            *tmp_ptr = NULL;
            return SYS_ARCH_TIMEOUT;
        }

        end_time = host_rtos_get_time( );
        elapsed_time = end_time - start_time;
        if ( elapsed_time == 0 )
        {
            elapsed_time = (wwd_time_t) 1;
        }
        return ( elapsed_time ); /* return time blocked TBD test */
    }
}

/*-----------------------------------------------------------------------------------
 * Creates and returns a new semaphore. The "count" argument specifies
 * the initial state of the semaphore. TBD finish and test
 */
err_t sys_sem_new( /*@special@*/ /*@out@*/ sys_sem_t *sem, u8_t count) /*@allocates *sem@*/
{
    wwd_result_t result;
    if ( sem == NULL )
    {
        /*@-mustdefine@*/ /*@-compdef@*/ /* do not need to allocate or set *sem */
        return ERR_VAL;
        /*@+mustdefine@*/ /*@+compdef@*/
    }

    result = host_rtos_init_semaphore( sem );
    if ( result != WWD_SUCCESS )
    {
        /*@-mustdefine@*/ /*@-compdef@*/ /* allocation failed - dont allocate or set *sem */
        return ERR_MEM;
        /*@+mustdefine@*/ /*@+compdef@*/
    }

//    if ( count == (u8_t) 0 ) /* Means it can't be taken */
//    {
//        if ( WWD_SUCCESS != host_rtos_get_semaphore( sem, NEVER_TIMEOUT, WICED_FALSE ) )
//        {
//            host_rtos_deinit_semaphore( sem );
//            return ERR_VAL;
//        }
//    }

    /*@-compdef@*/ /* Lint doesnt realise allocation has occurred */
    return ERR_OK;
    /*@+compdef@*/
}

/*-----------------------------------------------------------------------------------
 * Blocks the thread while waiting for the semaphore to be
 * signaled. If the "timeout" argument is non-zero, the thread should
 * only be blocked for the specified time (measured in
 * milliseconds).
 *
 * If the timeout argument is non-zero, the return value is the number of
 * milliseconds spent waiting for the semaphore to be signaled. If the
 * semaphore wasn't signaled within the specified time, the return value is
 * SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
 * (i.e., it was already signaled), the function may return zero.
 *
 * Notice that lwIP implements a function with a similar name,
 * sys_sem_wait(), that uses the sys_arch_sem_wait() function.
 */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    wwd_result_t result;
    wwd_time_t start_time, end_time, elapsed_time;

    start_time = host_rtos_get_time( );

    if ( timeout != 0 )
    {
        result = host_rtos_get_semaphore( sem, timeout, WICED_FALSE );
        if ( result == WWD_SUCCESS )
        {
            end_time = host_rtos_get_time( );
            elapsed_time = end_time - start_time;
            if ( elapsed_time == 0 )
            {
                elapsed_time = (wwd_time_t) 1;
            }
            return ( elapsed_time ); /* return time blocked TBD test */
        }
        else
        {
            return SYS_ARCH_TIMEOUT;
        }
    }
    else /* must block without a timeout */
    {
        host_rtos_get_semaphore( sem, NEVER_TIMEOUT, WICED_FALSE );

        end_time = host_rtos_get_time( );
        elapsed_time = end_time - start_time;
        if ( elapsed_time == 0 )
        {
            elapsed_time = (wwd_time_t) 1;
        }

        return ( elapsed_time ); /* return time blocked */

    }
}

/*-----------------------------------------------------------------------------------
 * Signals a semaphore
 */
void sys_sem_signal( sys_sem_t *sem )
{
    wwd_result_t result;

    result = host_rtos_set_semaphore( sem, WICED_FALSE );

    /*@-noeffect@*/
    (void) result;  /* unused in release build */
    /*@+noeffect@*/

    LWIP_ASSERT( "FreeRTOS failed to set semaphore for LwIP", result == WWD_SUCCESS );
}

/*-----------------------------------------------------------------------------------
 * Deallocates a semaphore
 */
void sys_sem_free( /*@special@*/ sys_sem_t *sem ) /*@releases *sem@*/
{
    host_rtos_deinit_semaphore( sem );
}

/*-----------------------------------------------------------------------------------
 * Initialize sys arch
 */
void sys_init( void )
{
#if LWIP_SYS_ARCH_TIMEOUTS
    int i;
    /* Initialize the the per-thread sys_timeouts structures
     * make sure there are no valid pids in the list
     */
    for(i = 0; i < SYS_THREAD_MAX; i++)
    {
        timeout_list[i].pid = 0;
    }

    /* keep track of how many threads have been created */
    next_thread = 0;
#endif /* if LWIP_SYS_ARCH_TIMEOUTS */
}

void sys_deinit( void )
{
}

#if LWIP_SYS_ARCH_TIMEOUTS
/*-----------------------------------------------------------------------------------
 * Returns a pointer to the per-thread sys_timeouts structure. In lwIP,
 * each thread has a list of timeouts which is represented as a linked
 * list of sys_timeout structures. The sys_timeouts structure holds a
 * pointer to a linked list of timeouts. This function is called by
 * the lwIP timeout scheduler and must not return a NULL value.
 *
 * In a single threaded sys_arch implementation, this function will
 * simply return a pointer to a global sys_timeouts variable stored in
 * the sys_arch module.
 */

struct sys_timeouts *
sys_arch_timeouts(void)
{
    int i;
    xTaskHandle pid;
    struct timeout_list *tl;

    pid = xTaskGetCurrentTaskHandle( );

    for(i = 0; i < next_thread; i++)
    {
        tl = &timeout_list[i];
        if(tl->pid == pid)
        {
            return &(tl->timeouts);
        }
    }

    /* Error */
    return NULL;
}
#endif /* if LWIP_SYS_ARCH_TIMEOUTS */

/*-----------------------------------------------------------------------------------
 * Starts a new thread with priority "prio" that will begin its execution in the
 * function "thread()". The "arg" argument will be passed as an argument to the
 * thread() function. The id of the new thread is returned. Both the id and
 * the priority are system dependent.
 */
/*@null@*/ int sys_thread_new( const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio, sys_thread_t* thread_out )
{
    wwd_result_t result;

    /* The first time this is called we are creating the lwIP handler. */
    result = host_rtos_create_thread( thread_out, (void(*)(wwd_thread_arg_t))thread, name, NULL, stacksize, prio );

#if LWIP_SYS_ARCH_TIMEOUTS
    /* For each task created, store the task handle (pid) in the timers array.
     * This scheme doesn't allow for threads to be deleted
     */
    timeout_list[next_thread++].pid = created_task;
#endif /* if LWIP_SYS_ARCH_TIMEOUTS */

    return ( result == WWD_SUCCESS )? 0 : -1;
}

void sys_thread_exit( void )
{
    malloc_leak_check(NULL, LEAK_CHECK_THREAD);
    host_rtos_finish_thread( NULL );
}

void sys_thread_free( sys_thread_t* task )
{
#if ( configFREE_TASKS_IN_IDLE == 0 )
    host_rtos_delete_terminated_thread( task );
#endif /* if ( configFREE_TASKS_IN_IDLE == 0 ) */
}

/*-----------------------------------------------------------------------------------
 * This optional function does a "fast" critical region protection and returns
 * the previous protection level. This function is only called during very short
 * critical regions. An embedded system which supports ISR-based drivers might
 * want to implement this function by disabling interrupts. Task-based systems
 * might want to implement this by using a mutex or disabling tasking. This
 * function should support recursive calls from the same task or interrupt. In
 * other words, sys_arch_protect() could be called while already protected. In
 * that case the return value indicates that it is already protected.
 *
 * sys_arch_protect() is only required if your port is supporting an operating
 * system.
 */
static int sys_arch_protect_semaphore_inited = 0;
static pthread_mutex_t sys_arch_protect_semaphore;
static int sys_arch_protect_semaphore_locked = 0;

sys_prot_t sys_arch_protect( void )
{
    int prev_locked;
    if (sys_arch_protect_semaphore_inited == 0 )
    {
        pthread_mutex_init( &sys_arch_protect_semaphore, NULL );

        sys_arch_protect_semaphore_inited = 1;
    }

    pthread_mutex_lock( &sys_arch_protect_semaphore );
    prev_locked = sys_arch_protect_semaphore_locked;
    sys_arch_protect_semaphore_locked = 1;
    return prev_locked;
}

/*-----------------------------------------------------------------------------------
 * This optional function does a "fast" set of critical region protection to the
 * value specified by pval. See the documentation for sys_arch_protect() for
 * more information. This function is only required if your port is supporting
 * an operating system.
 */
void sys_arch_unprotect( sys_prot_t pval )
{
    if ( pval == 0 )
    {
        sys_arch_protect_semaphore_locked = 0;
        pthread_mutex_unlock( &sys_arch_protect_semaphore );
    }
}

/*-----------------------------------------------------------------------------------
 * Similar to sys_arch_mbox_fetch, but if message is not ready immediately, we'll
 * return with SYS_MBOX_EMPTY.  On success, 0 is returned.
 */
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    void *dummy_ptr;
    void ** tmp_ptr = msg;

    if ( msg == NULL )
    {
        tmp_ptr = &dummy_ptr;
    }

    if ( WWD_SUCCESS == host_rtos_pop_from_queue( mbox, tmp_ptr, 0 ) )
    {
        return ERR_OK;
    }
    else
    {
        return SYS_MBOX_EMPTY;
    }
}


/*-----------------------------------------------------------------------------------
 * Try to post the "msg" to the mailbox.
 */
err_t sys_mbox_trypost( sys_mbox_t *mbox, void *msg )
{
    err_t result;

    if ( host_rtos_push_to_queue( mbox, &msg, 0 ) == WWD_SUCCESS )
    {
        result = ERR_OK;
    }
    else
    {
        /* could not post, queue must be full */
        result = ERR_MEM;

#if SYS_STATS
        lwip_stats.sys.mbox.err++;
#endif /* SYS_STATS */

    }

    return result;
}

int sys_mbox_valid( sys_mbox_t *mbox )
{
    return (int) ( mbox->queue_data != NULL );
}

int sys_sem_valid( sys_sem_t *sem )
{
    return sem->valid;
}

void sys_mbox_set_invalid( sys_mbox_t *mbox )
{
//    host_rtos_deinit_queue( mbox );
}

void sys_sem_set_invalid( sys_sem_t *sem )
{
    sem->valid = 0;
}




uint16_t sys_rand16( void )
{
    uint16_t output;
    wiced_crypto_get_random( &output, 2 );
    return output;
}
