#ifndef _AK_VENC_CB_H_
#define _AK_VENC_CB_H_

#include "anyka_types_internal.h"

#define AL_NO_WAIT (0)
#define AL_WAIT_FOREVER (0xFFFFFFFF)

/* mutex lock init */
void *venc_cb_init_mutex(int binitialstate);

/* mutex lock destroy */
void venc_cb_destroy_mutex(void *pmutex);

/* mutex lock */
int venc_cb_lock_mutex(void *pmutex);

/* mutex unlock */
int venc_cb_unlock_mutex(void *mutex);

/* semaphore operation api */
void *venc_cb_sem_init(int initialcount);

/* semaphore destroy */
void venc_cb_sem_destroy(void *semaphore);

/* 
 * semaphore wait
 * wait: block or not or block with timeout
 */
int venc_cb_sem_wait(void *semaphore, unsigned long wait);

/*
 * semaphore post
 */
int venc_cb_sem_post(void *semaphore);

/* init, read and write register */
void venc_cb_init_reg(unsigned long regstart, unsigned long size);

/* read register */
unsigned long venc_cb_read_reg(unsigned long reg);

/* write register */
void venc_cb_write_reg(unsigned long reg, unsigned long val);

/* others api */
void venc_cb_flush_dcache(void);

void venc_cb_module_clock(T_BOOL benable);

/* system sync add function */
int venc_cb_atomic_increment(int *val);

/* system sync sub function */
int venc_cb_atomic_decrement(int *val);

/*
 * event create
 */
void *venc_cb_create_event(int binitialstate);

/*
 * event delete
 */
void venc_cb_delete_event(void *event);

/*
 * event wait
 */
int venc_cb_wait_event(void *event, unsigned long wait);

/*
 * event set
 */
int venc_cb_set_event(void *event);

#endif
