#include <stdlib.h>

#include "anyka_types_internal.h"
#include "akuio.h"
#include "ak_thread.h"
#include "ak_common.h"
#include "ak_venc_cb.h"

//#define DEBUG_CB
#ifndef DEBUG_CB
	#define debug_cb(fmt...) do {} while (0);
#else
	#define debug_cb(fmt...) ak_print_normal_ex(fmt)
#endif

/* event struct for encode-lib */
typedef struct
{
	ak_mutex_t mutex;
	ak_cond_t  cond;
	int bsignaled;
} evt_t;

/* video encode read-write register option struct */
struct venc_reg_rw
{
	unsigned long va_start;	//start of virtual address 
	unsigned long pa_start;	//start of physical address 
	unsigned long len;		//lenght of memory
};

static struct venc_reg_rw reg_rw_ctrl = {0};

/****************************************************************************/
/* mutex lock init */
void *venc_cb_init_mutex(int binitialstate)
{
	ak_mutex_t *mutex = (ak_mutex_t *)malloc(sizeof(ak_mutex_t));

	debug_cb("mutex: %p, initstate: %d\n", mutex, binitialstate);
	if (mutex) {
#ifndef AK_RTOS
		/* on Linux, it implement for recursive mutex lock */
		ak_mutexattr_t mutexattr;
		ak_thread_mutexattr_init(&mutexattr);
		ak_thread_mutexattr_settype(&mutexattr, AK_THREAD_MUTEX_RECURSIVE);
        ak_thread_mutex_init(mutex, &mutexattr);
#else
		ak_thread_mutex_init(mutex, NULL);
#endif

		/* if necessary, lock on init */
		if (binitialstate)
			venc_cb_lock_mutex(mutex);
	}
	return (void *)mutex;
}

/****************************************************************************/
/* mutex lock destroy */
void venc_cb_destroy_mutex(void *pmutex)
{
	ak_mutex_t *mutex = (ak_mutex_t *)pmutex;

	debug_cb("mutex: %p\n", mutex);
	if (mutex) {
		ak_thread_mutex_destroy(mutex);
		free(mutex);
	}
}

/****************************************************************************/
/* mutex lock */
int venc_cb_lock_mutex(void *pmutex)
{
	ak_mutex_t *mutex = (ak_mutex_t *)pmutex;

	if (!mutex)
		return AK_FALSE;

	if (ak_thread_mutex_lock(mutex) < 0)
		return AK_FALSE;
	else
		return AK_TRUE;
}

/****************************************************************************/
/* mutex unlock */
int venc_cb_unlock_mutex(void *mutex)
{
	int ret = AK_TRUE;

	debug_cb("mutex: %p\n", mutex);
	if (mutex)
		ret = ((ak_thread_mutex_unlock((ak_mutex_t *)mutex)) < 0) ? AK_FALSE : AK_TRUE;
	else
		ret = AK_FALSE;

	return ret;
}

/********************* semaphore init **********************/
/* semaphore init */
void *venc_cb_sem_init(int initialcount)
{
	ak_sem_t *sem = (ak_sem_t *)malloc(sizeof(ak_sem_t));

	if (sem)
		ak_thread_sem_init(sem, initialcount);
	debug_cb("sem: %p, init count: %d\n", sem, initialcount);

	return (void *)sem;
}

/***************** semaphore destroy ********************/
/* semaphore destroy */
void venc_cb_sem_destroy(void *semaphore)
{
	ak_sem_t *sem = (ak_sem_t *)semaphore;

	if (sem) {
		debug_cb("sem: %p\n", sem);
		ak_thread_sem_destroy(sem);
		free(sem);
	}
}

/***************** semaphore wait ********************/
/* 
 * semaphore wait
 * wait: block or not or block with timeout
 */
int venc_cb_sem_wait(void *semaphore, unsigned long wait)
{
	int ret = AK_TRUE;
	ak_sem_t *sem = (ak_sem_t *)semaphore;

	debug_cb("sem: %p\n", sem);
	if (sem) {
		if(wait == AL_NO_WAIT) {	//not block
			debug_cb("no wait\n");
			if (ak_thread_sem_trywait(sem) < 0)
				ret = AK_FALSE;
		} else if (wait == AL_WAIT_FOREVER) {	//block
			debug_cb("wait forever\n");
			if (ak_thread_sem_wait(sem) < 0)
				ret = AK_FALSE;
		} else {	//block for timeout
			debug_cb("wait timeout\n");
			struct timespec ts;
			ts.tv_sec  = wait / 1000;
			ts.tv_nsec = (wait % 1000) * 1000000;
			if (ak_thread_sem_timedwait(sem, &ts) < 0)
				ret = AK_FALSE;
		}
	}
	else
		ret = AK_FALSE;
	return ret;
}

/**************** semaphore post ********************/
/*
 * semaphore post
 */
int venc_cb_sem_post(void *semaphore)
{
	int ret = AK_TRUE;
	ak_sem_t *sem = (ak_sem_t *)semaphore;

	debug_cb("sem: %p\n", sem);
	if (sem)
		ak_thread_sem_post(sem);
	else
		ret = AK_FALSE;
	return ret;
}

/*
 * **************************
 *  	 register api
 * **************************
 */
void venc_cb_init_reg(unsigned long regstart, unsigned long size)
{
	/* create local v-p address structure */
	if (reg_rw_ctrl.va_start != 0) {
		return;
	}

	/* map */
	debug_cb("mapping, start: 0x%08x, len: 0x%08x\n",
			(unsigned int)regstart, (unsigned int)size);
	reg_rw_ctrl.va_start = (unsigned long)akuio_map_regs(regstart, size);
	if (!reg_rw_ctrl.va_start) {
		ak_print_error_ex("map failed\n");
		return;
	}

	/* store vaddr/paddr start address */
	reg_rw_ctrl.pa_start = regstart;
	reg_rw_ctrl.len = size;

	debug_cb("init venc reg, va: 0x%08x, pa: 0x%08x, len: 0x%08x\n",
			(unsigned int)reg_rw_ctrl.va_start,
			(unsigned int)reg_rw_ctrl.pa_start,
			(unsigned int)reg_rw_ctrl.len);
}

/* read register */
unsigned long venc_cb_read_reg(unsigned long reg)
{
	/* paddress to vaddress: offset = reg - paddr_start */
	unsigned long offset = reg - reg_rw_ctrl.pa_start;

	/* read data to vaddress: read_adddr = vaddr + offset */
	unsigned long read_addr = reg_rw_ctrl.va_start + offset;

	/* read */
	unsigned long reg_val = *((volatile unsigned long *)(read_addr));
	debug_cb("reg: %08x, val: %08x\n",
			(unsigned int )reg, (unsigned int)reg_val);
	return reg_val;
}

/* write register */
void venc_cb_write_reg(unsigned long reg, unsigned long val)
{
	debug_cb("reg: %08x, val: %08x\n",
			(unsigned int )reg, (unsigned int)val);

	/* paddress to vaddress: offset = reg - paddr_start */
	unsigned long offset = reg - reg_rw_ctrl.pa_start;

	/* write data to vaddress: write_addr = vaddr + offset */
	unsigned long write_addr = reg_rw_ctrl.va_start + offset;

	debug_cb("write_addr: 0x%08x, start: 0x%08x, off: 0x%08x\n",
			(unsigned int)write_addr,
			(unsigned int)reg_rw_ctrl.va_start,
			(unsigned int)offset);
	/* write */
	(*(volatile unsigned long *)(write_addr)) = val;
}

/* flush D cache */
void venc_cb_flush_dcache(void)
{
	return ;
}

void venc_cb_module_clock(T_BOOL benable)
{
	/* not implement */
	return ;
}

int venc_cb_atomic_increment(int *val)
{
	/* system sync add function */
	return __sync_add_and_fetch(val, 1);
}
 
int venc_cb_atomic_decrement(int *val)
{
	/* system sync sub function */
	return __sync_sub_and_fetch(val, 1);
}

/****************************************************************************/
/*
 * event create
 */
void *venc_cb_create_event(int binitialstate)
{
	evt_t *evt = (evt_t *)malloc(sizeof(evt_t));
	if (evt) {
		debug_cb("evt: %p, initstate: %d\n", evt, binitialstate);
		ak_thread_mutex_init(&evt->mutex, NULL);
		ak_thread_cond_init(&evt->cond);
		evt->bsignaled = binitialstate;
	}
	return (void *)evt;
}

/****************************************************************************/
/*
 * event delete
 */
void venc_cb_delete_event(void *event)
{
	evt_t *evt = (evt_t *)event;
	if (evt) {
		debug_cb("evt: %p\n", evt);
		ak_thread_cond_destroy(&evt->cond);
		ak_thread_mutex_destroy(&evt->mutex);
		free(evt);
	}
}

/****************************************************************************/
/*
 * event wait
 */
int venc_cb_wait_event(void *event, unsigned long wait)
{
	int ret = AK_TRUE;
	evt_t *evt = (evt_t *)event;

	if (evt) {
		debug_cb("evt: %p, wait: %lu\n", evt, wait);
		ak_thread_mutex_lock(&evt->mutex);
		if (wait == AL_WAIT_FOREVER) {	//block
			while(ret && !evt->bsignaled)
				ret = (ak_thread_cond_wait(&evt->cond, &evt->mutex) == 0);
		} else {	//timeout
			struct timespec ts;
			ts.tv_sec  = wait / 1000;
			ts.tv_nsec = (wait % 1000) * 1000000;
			while(ret && !evt->bsignaled)
				ret = (ak_thread_cond_timedwait(&evt->cond, &evt->mutex, &ts) == 0);
		}
		if(ret)
			evt->bsignaled = AK_FALSE;
		ak_thread_mutex_unlock(&evt->mutex);
	} else
		ret = AK_FALSE;

	return ret;
}

/****************************************************************************/
/*
 * event set
 */
int venc_cb_set_event(void *event)
{
	int ret;
	evt_t *evt = (evt_t *)event;

	debug_cb("evt: %p\n", evt);
	ak_thread_mutex_lock(&evt->mutex);
	evt->bsignaled = AK_TRUE;
	ret = ak_thread_cond_signal(&evt->cond) == 0;
	ak_thread_mutex_unlock(&evt->mutex);

	return ret;
}
