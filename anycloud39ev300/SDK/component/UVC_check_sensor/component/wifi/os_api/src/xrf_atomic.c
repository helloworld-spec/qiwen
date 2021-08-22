#include "xrf_api.h"

int atomic_test_set(atomic *at, int val)
{
	uint32_t cpu_sr;
	cpu_sr = local_irq_save();
	if (at->val)
	{
		local_irq_restore(cpu_sr);
		return 1;
	}
	at->val = val;
	local_irq_restore(cpu_sr);
	return 0;
}
#pragma arm section code ="_video_server_"

void atomic_set(atomic *at, int val)
{
	uint32_t cpu_sr;
	cpu_sr = local_irq_save();
	at->val = val;
	local_irq_restore(cpu_sr);
}
#pragma arm section code


int atomic_read(volatile atomic_t *v)
{
	int val;
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	val = v->val;
	local_irq_restore(cpu_sr);
	return val;
}

void atomic_add(int i, volatile atomic_t *v)
{
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	v->val += i;
	local_irq_restore(cpu_sr);
}

void atomic_sub(int i, volatile atomic_t *v)
{
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	v->val -= i;
	local_irq_restore(cpu_sr);
}

int atomic_add_return(int i, atomic_t *v)
{
	int temp;
	uint32_t cpu_sr;
    	cpu_sr = local_irq_save();
	temp = v->val;
	temp += i;
	v->val = temp;
	local_irq_restore(cpu_sr);

	return temp;
}
#pragma arm section code ="_video_server_"

int  atomic_sub_return(int i, atomic_t *v)
{
	unsigned long temp;
	uint32_t cpu_sr;
	cpu_sr = local_irq_save();
	temp = v->val;
	temp -= i;
	v->val = temp;
	local_irq_restore(cpu_sr);
	
	return temp;
}
#pragma arm section code 


