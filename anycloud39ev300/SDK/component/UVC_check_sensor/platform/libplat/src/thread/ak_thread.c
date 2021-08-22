#include "akos_api.h"
#include "ak_thread.h"
#include "libc_mem.h"
#include "ak_common.h"

struct thread_struct {
    T_hTask handle;
    ak_pthread_t id;
    void *stack_addr;
    ak_sem_t sem;
};

#define LIBPLAT_THREAD_VERSION "libplat_thread_1.0.00"

#define MAX_TASK_NUM    (128)
static struct thread_struct thread_list[MAX_TASK_NUM]={0};
static unsigned long m_id = 100;

const char* ak_thread_get_version(void)
{

	return LIBPLAT_THREAD_VERSION;
}

/**
 * @brief create thread and get thread id
 * 
 * @param id[out]         pointer to thread id
 * @param func[in]        thread function
 * @param arg[in]         thread parameter
 * @param stack_size[in]  thread stack size
 * @param priority[in]    0-99 , larger value  means priority higher
 * @return  0: success, -1 :fail
 */
 int ak_thread_create(ak_pthread_t *id, thread_func func,
		void *arg, int stack_size, int priority) 
{
	T_hTask handle;
	char thread_name[16];
	int i;
	int priority_map;
	ak_pthread_t thread_id;

    thread_id = ((unsigned long)m_id << 12) >> 12;
	sprintf(thread_name, "[%d]", thread_id);
    m_id++;
    
    //check id exist or not
    for (i=0; i<MAX_TASK_NUM; i++)
    {
        if (thread_list[i].id == thread_id)
        {
            ak_print_error_ex("same id");
            return -1;
        }       
    }
    
    //find a place for task handle	
    for (i=0; i<MAX_TASK_NUM; i++)
    {
        if (thread_list[i].stack_addr == NULL)
            break;
    }
    if (i == MAX_TASK_NUM)
    {
        ak_print_error_ex("err");
        return -1;
    }

    //init task sem
    if (ak_thread_sem_init(&thread_list[i].sem, 0) < 0)
    {
        ak_print_error_ex("err");
        return -1;
    }
   
    //malloc stack space
    thread_list[i].stack_addr = malloc(stack_size);
    if (thread_list[i].stack_addr == NULL)
    {
        ak_thread_sem_destroy(&thread_list[i].sem);
        ak_print_error_ex("err");
        return -1;
    }

    //priority map , 0--99 map 255-0
	if (0 == priority)
		priority_map = 255;
	else if (99 == priority )
		priority_map = 0;
    else if (priority < 0)
		priority_map = 255/2;
	else
	{
	    //first map 0-255
	    priority_map = priority * 256 /100 ;
		//second reverse
		priority_map = 255 - priority_map;
	}

    ak_print_debug_ex("priority=%d, map=%d\n", priority, priority_map);
	
    
    //create task
	handle = AK_Create_Task(func,thread_name, (unsigned long)arg, arg, 
	            thread_list[i].stack_addr, (unsigned long)stack_size, priority_map, 
	            1, AK_PREEMPT,AK_NO_START);
	if (AK_IS_INVALIDHANDLE(handle))
	{
	    thread_list[i].stack_addr = free(thread_list[i].stack_addr);
        ak_thread_sem_destroy(&thread_list[i].sem);
        ak_print_error_ex("err");
	    return  -1;
	}

    //record handle and then resume task
    thread_list[i].handle = handle;
    thread_list[i].id     = thread_id;
	*id = thread_id;

	AK_Resume_Task(handle);
	
    ak_print_debug_ex("[%d]handle=%x, id=%d\n", i, handle, thread_id);
    
	return 0;
	
}

/**
 * @brief join one thread
 *        the thread will suspend until the thread it waits exits.
 *        the memory of the exiting thread will release until now.
 * @param id[in]         thread id
 * @return  0: success, -1 :fail
 */
int ak_thread_join(ak_pthread_t id)
{
	signed long ret=0;
    int i;

     //find by id
    for (i=0; i<MAX_TASK_NUM; i++)
    {
        if (thread_list[i].id == id)
            break;
    }
    if (i == MAX_TASK_NUM)
    {
        ak_print_error_ex("err");
        return -1;
    }

    //wait thread quit
    ak_thread_sem_wait(&thread_list[i].sem);

    //release resouce
    ak_print_debug_ex("[%d]handle=%x, id=%d\n", i, 
    	thread_list[i].handle, thread_list[i].id);
	AK_Terminate_Task(thread_list[i].handle);
	if (AK_SUCCESS != (ret=AK_Delete_Task(thread_list[i].handle)))
	{
		ak_print_error_ex("err %d\n", ret);
		ret = -1;
	}
	thread_list[i].stack_addr = free(thread_list[i].stack_addr);
    ak_thread_sem_destroy(&thread_list[i].sem);
    thread_list[i].id = 0;
	
	return ret;
}

/**
 * @brief detach one thread from it's father , direct return here
 * 
 * @param id[in]         thread id
 * @return  0: success, 
 */
int ak_thread_detach(ak_pthread_t id)
{
	return 0;
}

/**
 * @brief thread exit
 *        the thread function must vork it when the function return.
 * @param no
 * @return no
 */
void ak_thread_exit(void)
{
	T_hTask handle;
    int i;

	handle = AK_GetCurrent_Task();

     //find by id
    for (i=0; i<MAX_TASK_NUM; i++)
    {
        if (thread_list[i].handle == handle)
            break;
    }
    if (i == MAX_TASK_NUM)
    {
        ak_print_error_ex("err");
        return;
    }

    //send exit signal
    ak_thread_sem_post(&thread_list[i].sem);
	
	return;
}

/**
 * @brief cancel thread
 * 
 * @param id[in]         thread id
 * @return  0: success, -1 :fail
 */
int ak_thread_cancel(ak_pthread_t id)
{
	signed long ret=0;
    int i;
	
    //find by id
    for (i=0; i<MAX_TASK_NUM; i++)
    {
        if (thread_list[i].id == id)
            break;
    }
    if (i == MAX_TASK_NUM)
    {
        ak_print_error_ex("err");
        return -1;
    }

    //terminate task
	if (AK_SUCCESS != AK_Terminate_Task(thread_list[i].handle))
		ret = -1;

	if (AK_SUCCESS != AK_Delete_Task(thread_list[i].handle))
		ret = -1;
		
    //release resouce
    thread_list[i].stack_addr = free(thread_list[i].stack_addr);
    ak_thread_sem_destroy(&thread_list[i].sem);
    thread_list[i].id = 0;

    return 0;
}

/**
 * @brief get current thread id
 * 
 * @param no
 * @return int 0: success, -1 :fail
 */
long int ak_thread_get_tid(void)
{
	T_hTask handle;
    int i;
	
	handle = AK_GetCurrent_Task();

     //find by id
    for (i=0; i<MAX_TASK_NUM; i++)
    {
        if (thread_list[i].handle == handle)
            break;
    }
    if (i == MAX_TASK_NUM)
    {
        ak_print_error_ex("err");
        return -1;
    }
    
	return thread_list[i].id;
	
}

/**
 * @brief init mutex
 * 
 * @param mutex[out]      pointer to mutex
 * @return 0: success, -1 :fail
 */
int ak_thread_mutex_init(ak_mutex_t *mutex)
{
	ak_mutex_t  handle;
	handle = AK_Create_Semaphore(1, AK_PRIORITY);

	if (handle !=AK_INVALID_SUSPEND)
	{
		*mutex = handle;
		return 0 ; 
	}else
		return -1;
	
}

/**
 * @brief  get mutex lock 
          thread will go ahead if it vork this function first.
 *        it will suspend thread if another thread lock first until unlock.
 * 
 * @param mutex[in]      pointer to mutex
 * @return 0: success, -1 :fail
 */
int ak_thread_mutex_lock(ak_mutex_t *mutex)
{
	int ret;
	ret = AK_Obtain_Semaphore(*mutex , AK_SUSPEND);
	if (AK_SUCCESS == ret)
		return 0;
	else
		return -1;
	
}

/**
 * @brief  release mutex lock
 *         the one of suspended threads for vorking lock will go ahead.
 * 
 * @param mutex[in]      pointer to mutex
 * @return int 0: success, -1 :fail
 */
int ak_thread_mutex_unlock(ak_mutex_t *mutex)
{
	int ret;
	ret = AK_Release_Semaphore(*mutex);
	if (AK_SUCCESS == ret)
		return 0;
	else
		return -1;
		
}

/**
 * @brief  free mutex 
 * 
 * @param mutex[in]      pointer to mutex
 * @return int 0: success, -1 :fail
 */
int ak_thread_mutex_destroy(ak_mutex_t *mutex)
{
	int ret;
	ret = AK_Delete_Semaphore(*mutex);
	if (AK_SUCCESS == ret)
		return 0;
	else
		return -1;

}

/**
 * @brief  init one semaphore and set original value
 * 
 * @param sem[out]   pointer to semaphore
 * @param value[in]  original value
 * @return 0: success, -1 :fail
 */
int ak_thread_sem_init(ak_sem_t *sem, unsigned int value)
{
	ak_sem_t  handle;
	handle = AK_Create_Semaphore(value, AK_PRIORITY);

	if (handle !=AK_INVALID_SUSPEND)
	{
		*sem = handle;
		return 0 ; 
	}else
		return -1;
	
}

/**
 * @brief  the semaphore value add 1
 * 
 * @param sem[in]   pointer to semaphore
 * @return 0: success, -1 :fail
 */
int ak_thread_sem_post(ak_sem_t *sem)
{
	int ret;
	ret = AK_Release_Semaphore(*sem);
	if (AK_SUCCESS == ret)
		return 0;
	else
		return -1;
	
}

/**
 * @brief  the semaphore value sub 1. If the value is bigger or equal  than 0 , then the thread
           will go ahead .Otherwize,  it will suspend until the value is bigger or equal than 0 because another 
           thread post to the semaphore.           
 * 
 * @param sem[in]   pointer to semaphore
 * @return 0: success, -1 :fail
 */
int ak_thread_sem_wait(ak_sem_t *sem)
{
	int ret;
	ret = AK_Obtain_Semaphore(*sem , AK_SUSPEND);
	if (AK_SUCCESS == ret)
		return 0;
	else
		return -1;
	
}


/**
 * @brief  free semaphore
 * 
 * @param sem[in]      pointer to semaphore
 * @return int 0: success, -1 :fail
 */
int ak_thread_sem_destroy(ak_sem_t *sem)
{
	int ret;
	ret = AK_Delete_Semaphore(*sem);
	if (AK_SUCCESS == ret)
		return 0;
	else
		return -1;

}
