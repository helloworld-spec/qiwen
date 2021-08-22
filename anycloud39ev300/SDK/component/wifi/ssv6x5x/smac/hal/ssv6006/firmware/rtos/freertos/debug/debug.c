/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "debug.h"

#if configCHECK_FOR_STACK_OVERFLOW
void vApplicationStackOverflowHook( xTaskHandle pxCurrentTCB, char * pcTaskName ,unsigned int top, unsigned int cur);
void vApplicationStackOverflowHook( xTaskHandle pxCurrentTCB, char * pcTaskName ,unsigned int top, unsigned int cur)
{
	DEBUG(1,1,"pxCurrentTCB=%08x,name=%s,pxTopOfStack=%08x <= pxStack=%08x\n",pxCurrentTCB,pcTaskName,top,cur);
	return;
}
#endif

#if configUSE_MALLOC_FAILED_HOOK
void vApplicationMallocFailedHook();
void vApplicationMallocFailedHook()
{
	extern xTaskHandle pxCurrentTCB;
	DEBUG(1,1,"pxCurrentTCB=%08x\n");
	while (1) {}
}
#endif
