#ifndef _ATOMIC_H_
#define _ATOMIC_H_
#include "xrf_api.h"
#include <stdio.h>

#define atomic_t atomic


typedef struct _atomic
{
	volatile unsigned char val;
} atomic;

#pragma arm section code ="_bootcode_"
static inline unsigned int local_irq_save(void)
{
	unsigned int cpu_sr = 0;
	//irq_mask();
	store_all_int();
	return (unsigned int)cpu_sr;
}
#pragma arm section code

static inline void local_irq_restore(unsigned int cpu_sr)
{
	//irq_unmask();
	restore_all_int();
}

static inline void enter_critical()
{
	irq_mask();
}

static inline void exit_critical()
{
	irq_unmask();
}


int atomic_test_set(atomic *at, int val);
void atomic_set(atomic *at, int val);
int atomic_read(volatile atomic_t *v);
void atomic_add(int i, volatile atomic_t *v);
void atomic_sub(int i, volatile atomic_t *v);
int atomic_add_return(int i, atomic_t *v);
int  atomic_sub_return(int i, atomic_t *v);

#endif
