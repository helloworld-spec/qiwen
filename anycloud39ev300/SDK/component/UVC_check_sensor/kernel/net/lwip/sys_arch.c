//#define DEBUG
//#define LWIP_MEM_LIMIT 

#include "mem.h"
#include "sys_arch.h"
#include "err.h"
#include "sys.h"
#include "hal_print.h"

//#include "xrf_api.h" 

#include "akos_api.h"
#include "os_malloc.h"

#include "akos_api.h"

#define EVENT_POST_MSG					(1)
#define EVENT_FREE						(2)

#define OS_TICK_RATE_HZ		200



unsigned long sys_start_time = 0;
struct timeoutnode 
{
	struct sys_timeouts timeouts;
	unsigned char prio;
	struct timeoutnode *next;
};

struct timeoutnode nulltimeouts;

struct timeoutnode *timeoutslist;

const void * const pvNullPointer = 0;


#define assert(x)                                                               \
do{                                                                              		 \
if (!(x))                                                                   	\
{                                                                           			\
printf( "%s:%d assert " #x "failed\r\n", __FILE__, __LINE__);	\
while(1);										\
}                                                                           			\
}while(0)


#ifdef LWIP_MEM_LIMIT
static T_hSemaphore m_memMutex = AK_INVALID_SEMAPHORE;
static unsigned int m_lwipMemSize=0;
#endif

void sys_init_timing()
{ 
	sys_start_time = get_tick_count();
}

#if 1
#define sys_get_ms_longlong()   get_tick_count()
#else
static unsigned long sys_get_ms_longlong()
{  
	//unsigned int ret;  
	//unsigned int now;  
	//now = get_tick_count();  
	//ret = (now - sys_start_time) * 5;  
	//return ret;
	return get_tick_count();
}
#endif
unsigned int sys_ms2ticks(unsigned int ms)
{
	if(ms == 0)
		return AK_SUSPEND;
	else{
		if(ms < 10)
			ms = 10;
			ms = ms * OS_TICK_RATE_HZ / 1000uL;
	}
	return ms;
}


/*
u32_t sys_jiffies()
{  
	return (u32_t)get_tick_count();
}
*/
u32_t sys_now(void)
{
	return (u32_t)get_tick_count();
}

err_t sys_sem_new(sys_sem_t *sem,u8_t count)
{
	*sem = AK_Create_Event_Group();
	if(*sem == 0)
	{
		printf("sys_sem_new err");
		return ERR_VAL;
	}
	return ERR_OK;
	
}

T_hSemaphore critSec = AK_INVALID_SEMAPHORE;
void InitSysArchProtect()
{  
	critSec = AK_Create_Semaphore(1, AK_PRIORITY);
}

#pragma arm section code ="_video_server_"
u32_t sys_arch_protect()
{  
	AK_Obtain_Semaphore(critSec, AK_SUSPEND);  
	return 0;
}

void sys_arch_unprotect(u32_t pval)
{  
	AK_Release_Semaphore(critSec);
}
#pragma arm section code 

void FreeSysArchProtect()
{  
	AK_Delete_Semaphore(critSec);
}

int sys_sem_valid(sys_sem_t *sem)
{
	if(*sem == 0)
		return 0;
	return 1;
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
	*sem = 0;
}
/*
;*****************************************************************************************************
;* 函数名称 : sys_sem_free
;* 描    述 : 删除一个信号
;* 输　	 入 : sem: 信号句柄
;*        
;* 输　	 出 : 无
;*****************************************************************************************************
;*/
void sys_sem_free(sys_sem_t *sem)
{
	AK_Delete_Event_Group(*sem);
}

#pragma arm section code ="_bootcode_"

/*
;*****************************************************************************************************
;* 函数名称 : sys_sem_signal
;* 描    述 : 发送一个信号
;* 输　	 入 : sem: 信号句柄
;*        
;* 输　	 出 : 无
;*****************************************************************************************************
;*/
void sys_sem_signal(sys_sem_t *sem)
{
//	printf("sys_sem_signal:%x", sem);
	int ret = AK_Set_Events(*sem, EVENT_FREE, AK_OR);
	if(ret != AK_SUCCESS)
		printf("sem post err %d", ret);
	
}
#pragma arm section code 

#pragma arm section code ="_video_server_"

/*
;*****************************************************************************************************
;* 函数名称 : sys_arch_sem_wait
;* 描    述 : 等待一个信号
;* 输　	 入 : sem: 信号句柄, timeout: 等待超时的微秒数
;*        
;* 输　	 出 : 等待所用的微秒数
;*****************************************************************************************************
;*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	int ret = SYS_ARCH_TIMEOUT;
	
	u32_t wait_ms = sys_get_ms_longlong();
	u32_t retrieved_events;
	timeout = sys_ms2ticks(timeout);
	
	ret = AK_Retrieve_Events(*sem, EVENT_FREE, AK_OR_CONSUME, &retrieved_events, timeout);
	
	if(ret == AK_SUCCESS)
		ret = sys_get_ms_longlong() - wait_ms;
	else if(ret == AK_TIMEOUT)
		ret = SYS_ARCH_TIMEOUT;
//	printf("sys_arch_sem_wait ret:%d", ret);
	return ret;
}
#pragma arm section code 

#define MSG_QUEUE_MAX 20

struct _msg_q_manage
{
	msg_q_t msg;
	void **q_start;
};

struct _msg_q_manage q_man[MSG_QUEUE_MAX];

int os_msgget(msg_q_t *msg_q, int q_size)
{
	unsigned int *q_start;
	int i, msg;

	for (i = 0; i < MSG_QUEUE_MAX; i++)
	{
		if (q_man[i].msg == 0)
			break;
	} 
	if (i >= MSG_QUEUE_MAX)
	{
		printf("msgget: no more msg queen");
		return  - 1;
	}
	
	q_start = (unsigned int*)mem_calloc(q_size, sizeof(unsigned int*));
	if(!q_start){
		printf("no mem:%d, ", q_size);
		return -1;
	}
	
	msg = AK_Create_Queue((void *)q_start,
                            q_size*sizeof(unsigned int*),
                            AK_FIXED_SIZE,
                            sizeof(unsigned int*),
                            AK_FIFO);

	if(AK_IS_INVALIDHANDLE(msg))
	{
		printf("create queue err:%d", msg);
		mem_free(q_start);
		return -1;
	}

	//wmj-
	//q_man[i].msg = (msg_q_t)*msg_q = msg;;
	q_man[i].msg = msg;
	*msg_q = msg;;

	q_man[i].q_start = q_start;
	
	return 0;
}

int os_msgsnd(msg_q_t msgid, void *msgbuf)
{

	int status = AK_Send_To_Queue(msgid, &msgbuf,
                sizeof(unsigned int*), AK_SUSPEND/*AK_NO_SUSPEND*/);
	if(status != AK_SUCCESS)
        {
        	if(status == AK_QUEUE_FULL)
			return 1;
			
		printf("msgsnd: err %d", status);
		return -1;
	}
	return  0;
}

int os_msgrcv(msg_q_t msgid, void **msgbuf, unsigned int timeout)
{
	int status;
	unsigned int actual_size;

	assert(msgid);
	timeout = sys_ms2ticks(timeout);
	
	status = AK_Receive_From_Queue(msgid, msgbuf, sizeof(unsigned int*), &actual_size, timeout);
        if(status != AK_SUCCESS)
        {
			if(status == AK_TIMEOUT)
				return 1;
		
		printf("msgrcv: err %d", status);
		return  - 1;
        }
	 return 0;
}

#pragma arm section code 

int os_msgfree(msg_q_t msgid)
{

	int status, i;

	for (i = 0; i < MSG_QUEUE_MAX; i++)
	{
		if (q_man[i].msg == msgid)
			break;
	}
	if (i >= MSG_QUEUE_MAX)
	{
		printf("msgfree: err no match msg_q %x", msgid);
		return  - 1;
	}

	if (!q_man[i].q_start)
	{
		assert(0);
	}

	mem_free(q_man[i].q_start);
	q_man[i].q_start = 0;
	q_man[i].msg = 0;
	
	status = AK_Delete_Queue(msgid);
        if(status != AK_SUCCESS)
        {
        	printf("del: err %d", status);
		return  - 1;
        }
	
	return 0;
}

void os_msg_q_init(void)
{
	int i;
	for (i = 0; i < MSG_QUEUE_MAX; i++)
	{
		q_man[i].msg = 0;
		q_man[i].q_start = 0;
	}
}


/*
;*****************************************************************************************************
;* 函数名称 : sys_mbox_new
;* 描    述 : 创建一个邮箱
;* 输　	 入 : size: 邮箱容量(实际不起作用)
;*        
;* 输　	 出 : sys_mbox_t: 邮箱句柄
;*****************************************************************************************************
;*/

#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
err_t sys_mbox_new(sys_mbox_t *pMbox, int size)
{

	err_t ret = ERR_OK;
	int real_size = min(size,MAX_MSG_IN_LWIP_MBOX);
	
	*pMbox = (sys_mbox_t)mem_calloc(sizeof(LWIP_MBOX),1);
	
	if (*pMbox == NULL)
	{
		ret = 1;
		goto end;
	}
	os_msgget(&((*pMbox)->lwip_mbox_e), real_size);
	
	if ((*pMbox)->lwip_mbox_e == (int)NULL)
	{
		mem_free(*pMbox);
		*pMbox = 0;
		ret = 2;
		goto end;
	}
end:
	if(ret != ERR_OK)
		printf("sys_mbox_new err:%d\n",ret);
	return ret;
}
#pragma arm section code ="_video_server_"

int sys_mbox_valid(sys_mbox_t *pMbox)
{
	if(*pMbox == 0)
		return 0;
	return 1;
}
#pragma arm section code 

void sys_mbox_set_invalid(sys_mbox_t *pMbox)
{
	*pMbox = 0;
}
/*
;*****************************************************************************************************
;* 函数名称 : sys_mbox_post
;* 描    述 : 发送邮件到邮箱
;* 输　	 入 : mbox: 邮箱句柄, msg: 邮件
;*        
;* 输　	 出 : 无
;*****************************************************************************************************
;*/
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
	int ret;
	int retry_cnt = 0;
	assert(mbox && *mbox);
	if (msg == NULL)
		msg = (void*)&pvNullPointer;
again:
	ret = os_msgsnd((*mbox)->lwip_mbox_e, msg);
	if(ret == 1) 	//OS_ERR_Q_FULL
	{
		printf("OS_ERR_Q_FULL");
	}

}


/*
;*****************************************************************************************************
;* ???? : sys_mbox_trypost
;* ?    ? : ?????????
;* ? 	 ? : mbox: ????, msg: ??
;*        
;* ? 	 ? : ERROR: ERR_MEM | OK: ERR_OK
;*****************************************************************************************************
;*/

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	err_t ret = ERR_OK;
	assert(mbox && *mbox);
	if (msg == NULL)
		msg = (void*)&pvNullPointer;

	if (os_msgsnd((*mbox)->lwip_mbox_e, msg) != 0)
	{
		ret = ERR_MEM;
		goto end;
	}
end:

	return ret;
}
/*
;*****************************************************************************************************
;* 函数名称 : sys_arch_mbox_fetch
;* 描    述 : 从邮箱等待一封邮件
;* 输　	 入 : mbox: 邮箱句柄, msg: 邮件, timeout: 等待超时的微秒数
;*        
;* 输　	 出 : 等待所用的微秒数
;*****************************************************************************************************
;*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	int err;
	u32_t wait_ms = sys_get_ms_longlong();
	void *Data;
	int ret = 0;
	assert(mbox && *mbox);

	err = os_msgrcv((*mbox)->lwip_mbox_e, &Data, timeout);
	if (err == 0)		
	{
	 	if (Data == (void*)&pvNullPointer)
	  {
			*msg = NULL;
	  }
		else
		{
			*msg = Data;
		}
		ret = sys_get_ms_longlong() - wait_ms;
	}else{
		ret = SYS_ARCH_TIMEOUT;
	}
		
	return ret;
}


/*
;*****************************************************************************************************
;* ???? : sys_arch_mbox_tryfetch
;* ?    ? : ???????????
;* ? 	 ? : mbox: ????, msg: ??
;*        
;* ? 	 ? : ERROR: SYS_MBOX_EMPTY | OK: 0
;*****************************************************************************************************
;*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	int err;
	void *Data;
	int ret;
	
	assert(mbox && *mbox);

	//Data = OSQAccept((*mbox)->lwip_mbox_e, &err);
	err = os_msgrcv((*mbox)->lwip_mbox_e, &Data, 1);
	if (err == 0)			
	{
	        if (Data == (void*)&pvNullPointer)
	        {
	            *msg = NULL;
	        }
	        else
	        {
	            *msg = Data;
	        }
		ret = 0;
	}	
	else
	{
        	ret = SYS_MBOX_EMPTY;
    	}
	//if(ret != 0)
	//	printf("sys_arch_mbox_tryfetch err:%d\n",ret);
	return ret;
}

	
/*
;*****************************************************************************************************
;* 函数名称 : sys_mbox_free
;* 描    述 : 删除一个邮箱
;* 输　	 入 : mbox: 邮箱句柄
;*        
;* 输　	 出 : 无
;*****************************************************************************************************
;*/
void sys_mbox_free(sys_mbox_t *mbox)
{
	assert(mbox && *mbox);
	
	os_msgfree((*mbox)->lwip_mbox_e);
	
	mem_free(*mbox);
}


sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio) 
{
	T_hTask ret;
	unsigned long *stack = (unsigned long*)mem_calloc(stacksize, sizeof(unsigned long));
    
	if(!stack)
	{
		printf("sys_thread_new can't malloc!\n");
		return -1;
	}
	
	ret = AK_Create_Task((void*)thread, name, arg, arg, stack, stacksize*sizeof(unsigned int), prio, 0, AK_PREEMPT, AK_START);
	if(AK_IS_INVALIDHANDLE(ret))
		printf("AK_Create_Task err:%d", ret);
		
	return ret;
	
}	

void msvc_sys_init()
{  
	sys_init_timing();  
	InitSysArchProtect();
}

void sys_init(void)
{
    timeoutslist = &nulltimeouts;
    
    nulltimeouts.timeouts.next = NULL; 
    
    nulltimeouts.next = NULL;

	msvc_sys_init();

#ifdef LWIP_MEM_LIMIT
	m_memMutex = AK_Create_Semaphore(1, AK_PRIORITY);
#endif	
}



err_t sys_mutex_new(sys_mutex_t *mutex)
{
	
	*mutex  = AK_Create_Semaphore(1, AK_PRIORITY);

	return  ERR_OK;
}
/** Lock a mutex
 * @param mutex the mutex to lock */
void sys_mutex_lock(sys_mutex_t *mutex)
{
	AK_Obtain_Semaphore(*mutex, AK_SUSPEND);  

}
/** Unlock a mutex
 * @param mutex the mutex to unlock */
void sys_mutex_unlock(sys_mutex_t *mutex)
{
	AK_Release_Semaphore(*mutex);

}
/** Delete a semaphore
 * @param mutex the mutex to delete */
void sys_mutex_free(sys_mutex_t *mutex)
{
	AK_Delete_Semaphore(*mutex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void lwip_freeAndTrace(void * pData , char * file_name, int line)
{

#ifdef LWIP_MEM_LIMIT
	unsigned int size;

	AK_Obtain_Semaphore(m_memMutex, AK_SUSPEND);  
	
	size = Fwl_GetPtrSize(pData);
	m_lwipMemSize -=size;
//	printf("lwip f size =%d, lwip size=%d\n",size,m_lwipMemSize);
//	printf("l f %d\n",size);
	
	AK_Release_Semaphore(m_memMutex);
#endif
	
	Fwl_FreeAndTrace(pData, file_name, line);
	
	

}

void * lwip_mallocAndTrace(unsigned int  size, char * file_name, int line )
{
	void * pData;


#ifdef LWIP_MEM_LIMIT

	unsigned int real_size;

	AK_Obtain_Semaphore(m_memMutex, AK_SUSPEND);  
	if (m_lwipMemSize + size> 10000)
	{
		printf("lwip_Malloc is not enought!\n");
		return AK_NULL;
	}
#endif
		
	pData = Fwl_MallocAndTrace(size, file_name, line);

#ifdef LWIP_MEM_LIMIT
	real_size = Fwl_GetPtrSize(pData);
	m_lwipMemSize +=real_size;
//	printf("lwip m req size=%d, lwip size=%d\n",real_size, m_lwipMemSize);
	AK_Release_Semaphore(m_memMutex);

	
	printf("l m %d\n",real_size);
#endif 

	return pData;

}


void * lwip_callocAndTrace(unsigned int  count , unsigned int size,char * file_name, int line)
{
	return lwip_mallocAndTrace(size * count , file_name,line);
		
}



/*
;*****************************************************************************************************
;*                            			End Of File
;*****************************************************************************************************
;*/


	 

