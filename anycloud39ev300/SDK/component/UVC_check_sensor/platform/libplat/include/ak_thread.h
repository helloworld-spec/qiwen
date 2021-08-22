#ifndef _AK_THREAD_H_
#define _AK_THREAD_H_

#ifdef AK_RTOS
typedef long        ak_pthread_t;
typedef long        ak_mutex_t;
typedef long        ak_sem_t;

#define ANYKA_THREAD_NORMAL_STACK_SIZE 	(200*1024)
#define ANYKA_THREAD_MIN_STACK_SIZE 	(100*1024)

#else
#include <semaphore.h>
#include <pthread.h>

#define ANYKA_THREAD_NORMAL_STACK_SIZE 	(200*1024)
#define ANYKA_THREAD_MIN_STACK_SIZE 	(100*1024)

/**
typedef union
{
	char __size[__SIZEOF_SEM_T];
	long int __align;
} sem_t;
*/
typedef sem_t				ak_sem_t;

/**
typedef unsigned long int pthread_t;
*/
typedef pthread_t 			ak_pthread_t;

/**
typedef union
{
	char __size[__SIZEOF_PTHREAD_ATTR_T];
	long int __align;} pthread_attr_t;
*/
typedef pthread_attr_t		ak_pthread_attr_t;

/**
typedef union
{
	struct __pthread_mutex_s
	{
		int __lock;
		unsigned int __count;
		int __owner;    //KIND must stay at this position in the structure to maintain binary compatibility.
		int __kind;
		unsigned int __nusers;
		__extension__ union
		{
			int __spins;
			__pthread_slist_t __list;
		};
	} __data;
	char __size[__SIZEOF_PTHREAD_MUTEX_T];
	long int __align;
} pthread_mutex_t;
*/
typedef pthread_mutex_t 	ak_mutex_t;

/**
typedef union
{
	char __size[__SIZEOF_PTHREAD_MUTEXATTR_T];
	long int __align;
} pthread_mutexattr_t;
*/
typedef pthread_mutexattr_t	ak_mutexattr_t;

#endif	//end of AK_RTOS

typedef void* (* thread_func)(void *param);

/**
 * ak_thread_get_version - get thread module version
 * return: version string
 */
const char* ak_thread_get_version(void);

/**
 * ak_thread_create - create a posix thread.
 * @thread_id[OUT]: thread id
 * @func[IN]: thread function
 * @arg[IN]: argument for thread
 * @stack_size[IN]: thread stack size
 * @priority[IN]: thread priority, default -1
 * return: 0 success; -1 failed
 */
int ak_thread_create(ak_pthread_t *id, thread_func func, void *arg,
		int stack_size, int priority);

/**
 * ak_thread_join - join the thread
 * @id[IN]: thread id
 * return: 0 success; otherwise error number
 */
int ak_thread_join(ak_pthread_t id);

/**
 * ak_thread_cancel - cancel the thread
 * @id[IN]: thread id
 * return: 0 success; otherwise error number
 */
int ak_thread_cancel(ak_pthread_t id);

/**
 * ak_thread_exit - thread exit
 * return: do not return
 */
void ak_thread_exit(void);

/**
 * ak_thread_get_tid - get thread id
 * return: thread id belongs to call function
 */
long int ak_thread_get_tid(void);

/**
 * ak_thread_mutex_init - init mutex
 * @mutex[OUT]: mutex pointer
 * return: 0 success; otherwise error number
 */
int ak_thread_mutex_init(ak_mutex_t *mutex);

/**
 * ak_thread_mutex_lock - lock mutex
 * @mutex[IN]: mutex pointer
 * return: 0 success; otherwise error number
 */
int ak_thread_mutex_lock(ak_mutex_t *mutex);

/**
 * ak_thread_mutex_unlock - unlock mutex
 * @mutex[IN]: mutex pointer
 * return: 0 success; otherwise error number
 */
int ak_thread_mutex_unlock(ak_mutex_t *mutex);

/**
 * ak_thread_mutex_destroy - destroy mutex
 * @mutex[IN]: mutex pointer
 * return: 0 success; otherwise error number
 */
int ak_thread_mutex_destroy(ak_mutex_t *mutex);

/**
 * ak_thread_sem_init - init semaphore
 * @sem[IN]: semaphore pointer
 * @value[IN]: semaphore initial value
 * return: 0 success; -1 failed
 */
int ak_thread_sem_init(ak_sem_t *sem, unsigned int value);

/**
 * ak_thread_sem_post - post semaphore
 * @sem[IN]: semaphore pointer
 * return: 0 success; -1 failed
 */
int ak_thread_sem_post(ak_sem_t *sem);

/**
 * ak_thread_sem_wait - wait semaphore
 * @sem[IN]: semaphore pointer
 * return: 0 success; -1 failed
 */
int ak_thread_sem_wait(ak_sem_t *sem);

/**
 * ak_thread_sem_destroy - destroy semaphore
 * @sem[IN]: semaphore pointer
 * return: 0 success; -1 failed
 */
int ak_thread_sem_destroy(ak_sem_t *sem);

#endif	//end of _AK_THREAD_H_
