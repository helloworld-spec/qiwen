#include "xrf_api.h"

#define EVENT_POST_MSG					(1)
#define EVENT_FREE						(2)

void sem_init(sem_t *sem, int value)
{
	sem_t _sem;
	_sem = AK_Create_Event_Group();
	*sem = _sem;
}

int sem_wait(sem_t *sem, unsigned int timeout)
{
	int ret;
	T_U32 retrieved_events;
	
	timeout = ms2ticks(timeout);
	//p_err("sem_wait %x %d", sem  , timeout);	
	//ret = AK_Obtain_Semaphore(*sem, AK_SUSPEND);
	ret = AK_Retrieve_Events(*sem, EVENT_FREE, AK_OR_CONSUME, &retrieved_events, timeout);
	//p_err("sem_wait ret %d, %d", ret, retrieved_events);
	if(ret == AK_TIMEOUT)
		return WAIT_EVENT_TIMEOUT;
	else if(ret == AK_SUCCESS)
		return 0;
	
	p_err("sem_wait err %d", ret);
	return  - 1;
}
#pragma arm section code ="_bootcode_"

int sem_post(sem_t *sem)
{
	int ret = AK_Set_Events(*sem, EVENT_FREE, AK_OR);
	if(ret == AK_SUCCESS)
		return 0;

	p_err("sem post err %d", ret);
	return -1;
}
#pragma arm section code 

void sem_destory(sem_t *sem)
{
	AK_Delete_Event_Group(*sem);
}
