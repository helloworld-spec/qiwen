#include <string.h>
#include "command.h"
#include "ak_thread.h"
#include "ak_common.h"
#include "kernel.h"


static ak_mutex_t lock_test;
static ak_sem_t  event_test;


void     thread_demo_1(void * arg)
{
	int i,j,k;
	
	ak_print_normal("I am %s ,id=%d\n", arg,ak_thread_get_tid());

	ak_print_normal("%s: I try to get lock\n",arg);
	ak_thread_mutex_lock(&lock_test);
	ak_print_normal("%s:I get lock\n",arg);
	ak_sleep_ms(1000);
	ak_print_normal("%s:sleep end\n",arg);
	ak_thread_mutex_unlock(&lock_test);
	ak_print_normal("%s:I release lock\n",arg);

	ak_print_normal("%s:I wait one notify...\n",arg);
	ak_thread_sem_wait(&event_test);
	ak_print_normal("%s:I get one notify...\n",arg);

	ak_thread_mutex_destroy(&lock_test);
	ak_thread_sem_destroy(&event_test);	
	
}

void     thread_demo_2(void * arg)
{
	ak_print_normal("I am %s ,id=%d\n", arg,ak_thread_get_tid());
	ak_print_normal("%s:I try to get lock\n",arg);
	ak_thread_mutex_lock(&lock_test);
	ak_print_normal("%s:I get lock\n",arg);
	ak_thread_mutex_unlock(&lock_test);
	ak_print_normal("%s:I release lock\n",arg);


	ak_print_normal("%s:I am working\n",arg);
	ak_sleep_ms(1000);
	
	ak_thread_sem_post(&event_test);
	ak_print_normal("%s:I post one event to notify work finish.\n",arg);
	
}


void cmd_malloc_demo(int argc, char **args)  
{
	char * data1 ;
	char * data2 ;

	ak_print_normal("I am testing memory...\n");

	data1 = malloc(4*1024);
	ak_print_normal("malloc data pointer=0x%x\n",data1);
	memcpy(data1, "hello",5);
	data2 = remalloc(data1, 8 * 1024);
	ak_print_normal("remalloc data pointer=0x%x\n",data2);
	memcpy(data2, "hello",5);
	free(data2);
	ak_print_normal("free data pointer=0x%x\n",data2);

	data1 = calloc(4, 1024);
	ak_print_normal("calloc data pointer=0x%x\n",data1);
	free(data1);
	
	ak_print_normal("test finish\n");
	
	
	
}

void cmd_thread_demo(int argc, char **args)  
{
	ak_pthread_t  thread_id;
	int ret;
	
	ret = ak_thread_mutex_init(&lock_test);
	if (ret ==-1)
	{
		ak_print_normal("ak_thread_mutex_init fail!\n");
		return ;
	}else
		ak_print_normal("ak_thread_mutex_init ok!\n");
	
	ret = ak_thread_sem_init(&event_test, 0);
	if (ret ==-1)
	{
		ak_print_normal("ak_thread_sem_init fail!\n");
		ak_thread_mutex_destroy(&lock_test);
		return ;
	}else
		ak_print_normal("ak_thread_sem_init ok!\n");
	
	ak_thread_create(&thread_id , (void*)thread_demo_1 , "thread_1", 4096, 50);
	ak_thread_create(&thread_id , (void*)thread_demo_2 , "thread_2", 4096, 50);
}

static char *help_thread[]={
	"thread module demo",
	""
};
static char *help_malloc[]={
	"malloc module demo",
	""
};

static int cmd_thread_reg(void)
{
    cmd_register("threaddemo", cmd_thread_demo, help_thread);
    cmd_register("mallocdemo", cmd_malloc_demo, help_malloc);
    return 0;
}

cmd_module_init(cmd_thread_reg)




