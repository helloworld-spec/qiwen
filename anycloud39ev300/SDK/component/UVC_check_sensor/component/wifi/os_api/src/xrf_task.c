#define DEBUG

#include "xrf_api.h"

static struct workqueue_struct *timer_tasklet = NULL;

static void os_workqueue_thread(struct workqueue_struct *workqueue);

int thread_create(void(*task)(void *p_arg), void *p_arg, unsigned int prio, unsigned int *pbos, unsigned int stk_size, char *name)
{

	T_hTask ret;

	assert(pbos != 0);
	ret = AK_Create_Task((void*)task, name, p_arg, p_arg, pbos, stk_size*sizeof(unsigned int), prio, 0, AK_PREEMPT, AK_START);
	if(AK_IS_INVALIDHANDLE(ret))
		p_err("AK_Create_Task err:%d", ret);
		
	return ret;
}

int thread_exit(int thread_id)
{
	int ret = 0;
	AK_Terminate_Task(thread_id);
	if(ret < 0)
		p_err("AK_Delete_Task err:%d", ret);
	ret = AK_Delete_Task(thread_id);
	if(ret < 0)
		p_err("AK_Delete_Task err:%d", ret);
	//AK_SUCCESS
	
	return ret;
}

//返回任务自身的ID号
int thread_myself()
{
	p_err_miss;
	return -1;
}


void initList(PLIST_HEADER pList)
{
	pList->pHead = pList->pTail = NULL;
	pList->size = 0;
	return ;
}
#pragma arm section code ="_video_server_"

void insertTailList(PLIST_HEADER pList, PLIST_ENTRY pEntry)
{
	uint32_t cpu_sr;
	assert(pList);
    	cpu_sr = local_irq_save();
	pEntry->pNext = NULL;
	if (pList->pTail)
		pList->pTail->pNext = pEntry;
	else
		pList->pHead = pEntry;
	pList->pTail = pEntry;
	pList->size++;
	local_irq_restore(cpu_sr);
	return ;
}
#pragma arm section code 

PLIST_ENTRY removeHeadList(PLIST_HEADER pList)
{
	PLIST_ENTRY pNext;
	PLIST_ENTRY pEntry = 0;
	uint32_t cpu_sr;
	assert(pList);
    	cpu_sr = local_irq_save();
	pEntry = pList->pHead;
	if (pList->pHead != NULL)
	{
		pNext = pList->pHead->pNext;
		pList->pHead = pNext;
		if (pNext == NULL)
			pList->pTail = NULL;
		pList->size--;
		if(pList->size == 0)
			pList->pHead = pList->pTail = NULL;
	}else{
		pList->size = 0;
		pList->pHead = pList->pTail = NULL;
	}
	local_irq_restore(cpu_sr);
	return pEntry;
}

int getListSize(PLIST_HEADER pList)
{
	return pList->size;
}

PLIST_ENTRY delEntryList(PLIST_HEADER pList, PLIST_ENTRY pEntry)
{
	PLIST_ENTRY pCurEntry;
	PLIST_ENTRY pPrvEntry;

	if (pList->pHead == NULL)
		return NULL;

	if (pEntry == pList->pHead)
	{
		pCurEntry = pList->pHead;
		pList->pHead = pCurEntry->pNext;

		if (pList->pHead == NULL)
			pList->pTail = NULL;

		pList->size--;
		return pCurEntry;
	}

	pPrvEntry = pList->pHead;
	pCurEntry = pPrvEntry->pNext;
	while (pCurEntry != NULL)
	{
		if (pEntry == pCurEntry)
		{
			pPrvEntry->pNext = pCurEntry->pNext;

			if (pEntry == pList->pTail)
				pList->pTail = pPrvEntry;

			pList->size--;
			break;
		}
		pPrvEntry = pCurEntry;
		pCurEntry = pPrvEntry->pNext;
	}

	return pCurEntry;
}

struct workqueue_struct *create_workqueue(const char *name, uint32_t stack_size, int prio)
{
	struct workqueue_struct *new_queue = (struct workqueue_struct *)mem_calloc(sizeof(struct workqueue_struct ) + stack_size*sizeof(uint32_t), 1);
	if(!new_queue){
		p_err_fun;
		return NULL;
	}
	new_queue->thread_event = init_event();
	if(!new_queue->thread_event)
		goto faild1;
	
	initList(&new_queue->list);
	new_queue->name = name;
	new_queue->task_id = prio; //thread_create调用的时候可能马上会切换到新线程，这里预先赋好值
	new_queue->stack = new_queue+1;
	new_queue->task_id = thread_create((void (*)(void*))os_workqueue_thread,
                        new_queue,
                        prio,
                        new_queue->stack,
                        stack_size,
                        (char*)name);
	if(AK_IS_INVALIDHANDLE(new_queue->task_id))
		goto faild;

	return new_queue;
faild:
	del_event(new_queue->thread_event);;
faild1:
	mem_free(new_queue);
//	OSSchedUnlock();
	return NULL;
}

void destroy_workqueue(struct workqueue_struct *workqueue)
{
	if(!workqueue)
		return;
	p_dbg("destroy %s", workqueue->name);
	del_event(workqueue->thread_event);
	thread_exit(workqueue->task_id);
	mem_free(workqueue);
}

/*
*工作线程
*参考linux的workqueue机制
*为异步事件的处理提供一种很好的方式
*/
static void os_workqueue_thread(struct workqueue_struct *workqueue)
{
	int ret;
	uint32_t irq_flag;
	p_dbg("start workqueue %x, %s id:%d", workqueue,  workqueue->name, workqueue->task_id);
	while (1)
	{
		ret = wait_event_timeout(workqueue->thread_event, 1000);
		if (ret != 0)
		{
			if (ret != WAIT_EVENT_TIMEOUT)
			{
				p_err("os_tasklet_thread wait err:%d, %x\n", ret, workqueue->thread_event);
				sleep(2000);
				continue;
			}
			//continue;
		}
		
		if (workqueue->list.size > 0)
		{
			while (workqueue->list.size)
			{
				PLIST_ENTRY entry = NULL;
				irq_flag = local_irq_save();
				if(workqueue->list.size)
					entry = removeHeadList(&workqueue->list);
				local_irq_restore(irq_flag);
					
				if (entry)
				{
					struct work_struct *work;
					work = (struct work_struct*)((char*)entry - offsetof(struct work_struct, entry));
					if (work->fun)
						work->fun(work->data);
				}
				else
				{
					p_err("no entry:%d\n", workqueue->list.size);
					break;
				} 
			}
		}
	}
}

/*
*定时器服务线程
*因ucos自带的定时器函数不允许使用信号等待操作
*这里单独建一个线程
*注意:已经对ucos内核做了一点修改
*/
int init_timer_tasklet()
{
	timer_tasklet = create_workqueue("timer_tasklet", TIMER_TASKLET_STACK_SIZE, TASK_TIMER_TASKLET_PRIO);
	if(timer_tasklet)
		return 0;
	else
		return -1;
}
void init_work(struct work_struct *work, void *fun, void *data)
{
	work->fun = (void (*)(void *))fun;
	work->data = data;
}

#pragma arm section code ="_video_server_"
int schedule_work(struct workqueue_struct *queue, struct work_struct *work)
{
#if 1
	uint32_t irq_flg;
	LIST_ENTRY *entry;
	irq_flg = local_irq_save();
		
	for(entry = queue->list.pHead; entry != NULL; entry = entry->pNext)
	{
		if(entry == &work->entry)
		{
			break;
		}
	}
	if(entry == NULL)
		insertTailList(&queue->list, &work->entry);
	local_irq_restore(irq_flg);
#else
	insertTailList(&queue->list, &work->entry);
#endif
	wake_up(queue->thread_event);
	
	return 0;
}
#pragma arm section code 


int schedule_timer_tasklet(struct work_struct *work)
{
	int ret = -1;
	if(timer_tasklet)
		ret = schedule_work(timer_tasklet,  work);
	return ret;
}

void timer_tasklet_callback(struct work_struct *work)
{
	schedule_timer_tasklet(work);
}

void init_work_thread(void)
{
	init_timer_tasklet();		
}

