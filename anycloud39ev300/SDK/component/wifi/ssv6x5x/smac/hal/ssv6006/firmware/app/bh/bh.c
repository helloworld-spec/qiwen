#include <stdio.h>
#include "hal.h"
#include <intc/intc.h>
#if defined(CONFIG_OS_UCOS_II) || defined(CONFIG_OS_UCOS_III)
#define bh_test_TASK_PRIORITY		8
#elif defined(CONFIG_OS_FREERTOS)
#define bh_test_TASK_PRIORITY		2
#elif defined(CONFIG_OS_THREADX)
#define bh_test_TASK_PRIORITY           8
#else
#  error "No valid OS is defined!"
#endif



void bh_test(void *arg)
{
	hal_bh_t *bh = arg;
	char c;
	printf("begin bh=0x%08x\n", bh);
	while (1) {
		c = (char)getchar();
		if ((int)c) {
			printf("'%c' pressed\n", c);
			//hal_raise_bh(bh);
			hal_post_semaphore(&bh->sem);
		}
	}
}

void TaskBH(void *arg)
{
	hal_bh_t *bh = arg;
	printf("Task BH\n", bh);

	while(1){
		hal_pend_semaphore(&bh->sem, HAL_SUSPEND);
		printf("Bottom Half invoked\n");
	}
}


hal_bh_t bh;
void BH_Init(void)
{
	bh.th.fn	= TaskBH;
	bh.th.prio	= 2;
	bh.th.stack_size = 0x400;
	bh.th.name	= "Key BH";
	bh.th.arg 	= &bh;
	bh.th.ptos	= NULL;
	
	if(hal_create_bh(&bh) == HAL_SUCCESS)
		printf("BH success\n");
	else
		printf("BH failed\n");

}

void APP_Init(void)
{
	hal_thread_t thread;
    hal_console_init(&console_serial_drv);
	hal_init_os();

	thread.fn 		= bh_test;
	thread.name 		= "bh_test";
	thread.stack_size 	= CONFIG_MINIMAL_STACK_SIZE * 2;
	thread.arg 		= &bh;
	thread.prio 		= bh_test_TASK_PRIORITY;
	thread.task		= NULL;
	thread.ptos		= NULL;
	hal_create_thread(&thread);

	BH_Init();

	hal_start_os();
}

void stop_and_halt (void)
{
    while (1) {}
}
