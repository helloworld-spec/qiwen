
#define DEBUG
//#define WIFI_MEM_LIMIT 

#include "xrf_api.h"


#ifdef 	WIFI_MEM_LIMIT
static unsigned int m_wifiMemSize=0;
static T_hSemaphore m_wifiMemMutex = AK_INVALID_SEMAPHORE;
#endif

#pragma arm section code ="_video_server_"

void wifi_freeAndTrace(void * pData, char * file_name, int line)
{

#ifdef 	WIFI_MEM_LIMIT
	unsigned int size;

	AK_Obtain_Semaphore(m_wifiMemMutex, AK_SUSPEND);  
	size = Fwl_GetPtrSize(pData);
	m_wifiMemSize -=size;
	AK_Release_Semaphore(m_wifiMemMutex);
	
	printf("w f %d\n",size);
#endif
	
	Fwl_FreeAndTrace(pData, file_name, line);
	

}

void * wifi_mallocAndTrace(unsigned int  size, char * file_name, int line )
{
	void * pData;

#ifdef WIFI_MEM_LIMIT

	unsigned int real_size;
	if (m_wifiMemMutex ==AK_INVALID_SEMAPHORE)
	{
		m_wifiMemMutex = AK_Create_Semaphore(1, AK_PRIORITY);
		
	}

	AK_Obtain_Semaphore(m_wifiMemMutex, AK_SUSPEND);  

	if (m_wifiMemSize + size> 64000)
	{
		printf("wifi_Malloc is not enought!\n");
		return AK_NULL;
	}
#endif

	pData = Fwl_MallocAndTrace(size, file_name, line);

#ifdef 	WIFI_MEM_LIMIT
	real_size = Fwl_GetPtrSize(pData);
	m_wifiMemSize +=real_size;
	
	AK_Release_Semaphore(m_wifiMemMutex);
	
	printf("w m %d\n", real_size);

	if (m_wifiMemSize > 20000)
		printf("b %d\n",m_wifiMemSize);
#endif	

	return pData;

}



void *wifi_callocAndTrace(unsigned int count, unsigned int size ,char * file_name, int line)
{
	void *p;

	/* allocate 'count' objects of size 'size' */
	p = wifi_mallocAndTrace(count *size, file_name, line);
	if (p)
	{
		/* zero the memory */
		memset(p, 0, count *size);
	}
	return p;
}
#pragma arm section code 

uint32_t mem_get_size()
{
	return 0;
}

uint32_t mem_get_free()
{
	return 0;
}
