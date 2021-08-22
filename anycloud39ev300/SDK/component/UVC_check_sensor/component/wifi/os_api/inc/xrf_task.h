#ifndef _TASK_H_
#define _TASK_H_
#include "xrf_api.h"


typedef struct _LIST_ENTRY
{
	struct _LIST_ENTRY *pNext;
} LIST_ENTRY, *PLIST_ENTRY;

typedef struct _LIST_HEADR
{
	PLIST_ENTRY pHead;
	PLIST_ENTRY pTail;
	int size;
} LIST_HEADER, *PLIST_HEADER;

struct work_struct {
	LIST_HEADER * pTaskletList;
	LIST_ENTRY entry;
	void (*fun) (void *data);
	void	*data;
	//OS_EVENT *event;
};

void initList(
	PLIST_HEADER pList);

void insertTailList(
	PLIST_HEADER pList,
	PLIST_ENTRY pEntry);

PLIST_ENTRY removeHeadList(
	PLIST_HEADER pList);

int getListSize(
	PLIST_HEADER pList);

PLIST_ENTRY delEntryList(
	PLIST_HEADER pList,
	PLIST_ENTRY pEntry);

struct workqueue_struct {
	unsigned int		flags;		/* I: WQ_* flags */
	int 				task_id;
	wait_event_t 	 	thread_event;
	LIST_HEADER 	list;
	const char		*name;	
	uint32_t *stack;
};

//#ifndef offsetof
//#define offsetof(type, member) ((long) &((type *) 0)->member)
//#endif


int thread_create(void(*task)(void *p_arg), void *p_arg, unsigned int prio, unsigned int *pbos, unsigned int stk_size, char *name);

int thread_exit(int thread_id);
int thread_myself(void);

//struct workqueue_struct *create_workqueue(const char *name, uint32_t stack_size, int prio);
void destroy_workqueue(struct workqueue_struct *workqueue);
int init_workqueue(void);
void init_work(struct work_struct *work, void *fun, void *data);
int schedule_work(struct workqueue_struct *queue, struct work_struct *work);
int schedule_timer_tasklet(struct work_struct *work);
void timer_tasklet_callback(struct work_struct *work);
void init_work_thread(void);

#define queue_work	schedule_work

//extern volatile uint32_t tick_ms;

#endif
