#include <stdlib.h>
#include "osi.h"

#include "dana_task.h"
#include "uart_if.h"


//大拿任务API接口， 厂商自行实现



typedef void*  dana_pthread_return_t;

struct dana_thread_handler{
    OsiTaskHandle thread_id;
};


/*
 * 创建线程
 * thread: 线程句柄 ，包含线程控制块 和线程回调函数
 * priority ：SDK若期望使用系统默认值， 则在调用时会传入
 * stack_size ：线程需要使用的栈大小
 * callback ：线程的执行函数体
 * arg ：callback的参数
 */
extern dana_thread_handler_t* dana_thread_create(uint8_t priority, int32_t stack_size, dana_thread_start_routine start_routine, void* arg)
{
    static char task_cnt = 0;
    if (NULL == start_routine) {
        return NULL; 
    }
    
    char task_name[3];
	task_name[0] = 'T';
	task_name[1] = (task_cnt++) + '0';
    task_name[2] = 0;

    dana_thread_handler_t *thread = (dana_thread_handler_t *)mem_Malloc(sizeof(dana_thread_handler_t));
    if (NULL == thread) {
        return NULL; 
    }

    //设置默认栈的大小 暂时不设置 TODO
    if(osi_TaskCreate((P_OSI_TASK_ENTRY)start_routine, (const signed char*)task_name, stack_size, arg, 1,& (thread->thread_id))<0){ 
			mem_Free(thread);      
			return NULL;
    }
     
    return thread;
}

/*
 * 销毁由txd_thread_create创建的线程
 */
extern int32_t dana_thread_destroy(dana_thread_handler_t *thread)
{
    if (NULL == thread) {
        return -1; 
    }

    osi_TaskDelete(&(thread->thread_id));
    mem_Free(thread);
    return 0;
}

struct dana_mutex_handler {
    OsiLockObj_t mutex;
};

/*
 * 创建mutex
 */
extern dana_mutex_handler_t* dana_mutex_create()
{
    dana_mutex_handler_t *mutex = (dana_mutex_handler_t *)mem_Malloc(sizeof(dana_mutex_handler_t));
    if (NULL == mutex) {
        return NULL; 
    }

	if(osi_LockObjCreate(&(mutex->mutex)) != 0){
				return NULL;
	}
    return mutex;
}

/*
 * 锁住mutex
 */
extern int32_t dana_mutex_lock(dana_mutex_handler_t *mutex)
{
    if (NULL == mutex) {
        return -1; 
    }	
    return   osi_LockObjLock(&(mutex->mutex),OSI_WAIT_FOREVER);
}

/*
 * 解锁mutex
 */
extern int32_t dana_mutex_unlock(dana_mutex_handler_t *mutex)
{
    if (NULL == mutex) {
        return -1; 
    }
    return  osi_LockObjUnlock(&(mutex->mutex));
}

/*
 * 销毁mutex
 */
extern int32_t dana_mutex_destroy(dana_mutex_handler_t *mutex)
{
	if (NULL == mutex) {
		return -1;
    }
    osi_SyncObjDelete(&(mutex->mutex));
	mem_Free(mutex);
    return 0;
}


