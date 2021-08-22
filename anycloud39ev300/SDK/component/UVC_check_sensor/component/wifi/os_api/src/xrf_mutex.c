#include "xrf_api.h"

mutex_t mutex_init(const char *name)
{
	mutex_t ret;

 	ret = AK_Create_Semaphore(1, AK_PRIORITY);

	return ret;
}

int mutex_lock(mutex_t mutex)
{
	int perr;
	perr = AK_Obtain_Semaphore(mutex, AK_SUSPEND);
	if(perr == 0)
		return 0;
	
	p_err("mutex_lock err %d", perr);
	return  - 1;
}

int mutex_unlock(mutex_t mutex)
{
	int perr = AK_Release_Semaphore(mutex);
	if(perr == 0)
		return 0;
	p_err("mutex_unlock err %d", perr);
	return  - 1;
}

void mutex_destory(mutex_t mutex)
{
	uint32_t err = AK_Delete_Semaphore(mutex);
	if (err != 0)
		p_err("mutex_destory err %d", err);
	
}
