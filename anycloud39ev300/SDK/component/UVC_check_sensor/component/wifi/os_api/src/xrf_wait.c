#include "xrf_api.h"

wait_event_t init_event()
{
	wait_event_t event;

	event = AK_Create_Semaphore(0, AK_PRIORITY);
	
	return (wait_event_t)event;
}

int wait_event(wait_event_t wq)
{
	int ret = AK_Obtain_Semaphore(wq, AK_SUSPEND);
	if(ret == AK_SUCCESS)
		return 0;
	p_err("add_wait_queue err:%d\n", ret);
	return  - 1;
}

int wait_event_timeout(wait_event_t wq, unsigned int timeout)
{
	int ret;
	
	timeout = ms2ticks(timeout);
		
	ret = AK_Obtain_Semaphore(wq, timeout);
	
	if(ret == AK_TIMEOUT)
		return WAIT_EVENT_TIMEOUT;
	else if(ret == AK_SUCCESS)
		return 0;
		
	return  - 1;

}
#pragma arm section code ="_video_server_"

void wake_up(wait_event_t wq)
{
	AK_Release_Semaphore(wq);
}
#pragma arm section code 


void del_event(wait_event_t wq)
{

	AK_Delete_Semaphore(wq);
}


void clear_wait_event(wait_event_t wq)
{
	AK_Reset_Semaphore(wq, 0);
}
