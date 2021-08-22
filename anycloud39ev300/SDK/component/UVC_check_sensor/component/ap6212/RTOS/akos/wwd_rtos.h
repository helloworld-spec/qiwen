/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */
#pragma once

#ifndef INCLUDED_WWD_RTOS_H_
#define INCLUDED_WWD_RTOS_H_

#include <stdint.h>
#include "anyka_types.h"
#include "akos_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

//wmj+
#define SCHED_PRIORITY_MAX     255
#define SCHED_PRIORITY_DEFAULT 100
#define SCHED_PRIORITY_MIN       1
#define SCHED_PRIORITY_IDLE      0
//wmj<<

#define RTOS_HIGHER_PRIORTIY_THAN(x)     ((x) < RTOS_HIGHEST_PRIORITY ? (x)+1 : RTOS_HIGHEST_PRIORITY)
#define RTOS_LOWER_PRIORTIY_THAN(x)      ((x) > RTOS_LOWEST_PRIORITY  ? (x)-1 : RTOS_LOWEST_PRIORITY )
#define RTOS_LOWEST_PRIORITY             SCHED_PRIORITY_MIN
#define RTOS_HIGHEST_PRIORITY            SCHED_PRIORITY_MAX
#define RTOS_DEFAULT_THREAD_PRIORITY     SCHED_PRIORITY_DEFAULT

/* RTOS allocates stack, no pre-allocated stacks */
#define RTOS_USE_DYNAMIC_THREAD_STACK

/* The number of system ticks per second */
#define SYSTICK_FREQUENCY  (1000 * 1000 / CONFIG_USEC_PER_TICK)

#ifndef WWD_LOGGING_STDOUT_ENABLE
#ifdef DEBUG
#define WWD_THREAD_STACK_SIZE        (1248 + 1400)
#else /* ifdef DEBUG */
#define WWD_THREAD_STACK_SIZE        (1024 + 1400)
#endif /* ifdef DEBUG */
#else /* if WWD_LOGGING_STDOUT_ENABLE */
#define WWD_THREAD_STACK_SIZE        (1024 + 4096 + 1400) /* WWD_LOG uses printf and requires minimum 4K stack space */
#endif /* WWD_LOGGING_STDOUT_ENABLE */

/******************************************************
 *             Structures
 ******************************************************/

//typedef sem_t    host_semaphore_type_t;
typedef long        host_thread_type_t;
typedef void*       (*thread_func)(void * arg);
typedef long        host_mutex_type_t;
typedef long        host_semaphore_type_t;


typedef struct
{
    uint32_t              message_num;
    uint32_t              message_size;
    uint8_t*              buffer;
    uint32_t              push_pos;
    uint32_t              pop_pos;
    host_semaphore_type_t push_sem;
    host_semaphore_type_t pop_sem;
    uint32_t              occupancy;
} host_queue_type_t;
/*
typedef struct
{
    pthread_t handle;
    uint32_t  arg;
    void(*entry_function)( uint32_t );
} host_thread_type_t;
*/



struct thread_struct {
    T_hTask handle;
    host_thread_type_t id;
    void *stack_addr;
    host_semaphore_type_t sem;
};


typedef struct
{
    uint8_t info;    /* not supported yet */
} host_rtos_thread_config_type_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ifndef INCLUDED_WWD_RTOS_H_ */
