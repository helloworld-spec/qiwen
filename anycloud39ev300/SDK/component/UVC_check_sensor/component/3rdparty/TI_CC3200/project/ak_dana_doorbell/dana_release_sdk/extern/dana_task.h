#ifndef _DANA_TASK_H_
#define _DANA_TASK_H_

#include "dana_base.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************ thread 接口 *********************************/

#define dana_thread_return 
typedef void  dana_thread_return_t;


typedef struct dana_thread_handler dana_thread_handler_t;

// pthread_t_is_valid
// detach
// join

typedef dana_thread_return_t (*dana_thread_start_routine)(void *);

/*
 * 创建线程
 * thread: 线程句柄 ，包含线程控制块 和线程回调函数
 * priority ：SDK若期望使用系统默认值， 则在调用时会传入0
 * stack_size ：线程需要使用的栈大小
 * callback ：线程的执行函数体
 * arg ：callback的参数
 */
extern dana_thread_handler_t* dana_thread_create(uint8_t priority, int32_t stack_size, dana_thread_start_routine start_routine, void* arg);

/*
 * 销毁由txd_thread_create创建的线程
 */
extern int32_t dana_thread_destroy(dana_thread_handler_t *thread);


/************************ mutex 接口 *********************************/
typedef struct dana_mutex_handler dana_mutex_handler_t;

/*
 * 创建mutex
 */
extern dana_mutex_handler_t* dana_mutex_create();

/*
 * 锁住mutex
 */
extern int32_t dana_mutex_lock(dana_mutex_handler_t *mutex);

/*
 * 解锁mutex
 */
extern int32_t dana_mutex_unlock(dana_mutex_handler_t *mutex);

/*
 * 销毁mutex
 */
extern int32_t dana_mutex_destroy(dana_mutex_handler_t *mutex);



#ifdef __cplusplus
extern "C"
{
#endif
#endif
