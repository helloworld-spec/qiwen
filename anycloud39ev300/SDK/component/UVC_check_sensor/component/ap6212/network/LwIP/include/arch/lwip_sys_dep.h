/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__

#if 0
struct _sys_mutex {
  void* sem;
};

typedef struct _sys_mutex sys_mutex_t;
#define sys_mutex_valid(mutex) (((mutex) != NULL) && ((mutex)->sem != NULL)  && ((mutex)->sem != (void*)-1))
#define sys_mutex_set_invalid(mutex) ((mutex)->sem = NULL)


/* HANDLE is used for sys_sem_t but we won't include windows.h */
struct _sys_sem {
  s32_t	hEvent;
};
typedef struct _sys_sem sys_sem_t;
#define SYS_SEM_NULL NULL
#define sys_sem_valid(sema) (((sema) != NULL) && (((sema)->hEvent > 0) || ((sema)->hEvent < -100)))
#define sys_sem_set_invalid(sema) ((sema)->hEvent = 0)

/* let sys.h use binary semaphores for mutexes */
#define LWIP_COMPAT_MUTEX 0

#ifndef MAX_QUEUE_ENTRIES
#define MAX_QUEUE_ENTRIES 100
#endif
struct lwip_mbox {
  s32_t hEvent;
  void* q_mem[MAX_QUEUE_ENTRIES];
  u32_t head, tail;
};
typedef struct lwip_mbox sys_mbox_t;
#define SYS_MBOX_NULL NULL
#define sys_mbox_valid(mbox) ((mbox != NULL) && (((mbox)->hEvent > 0)  || ((mbox)->hEvent < -100)))
#define sys_mbox_set_invalid(mbox) ((mbox)->hEvent = 0)

/* DWORD (thread id) is used for sys_thread_t but we won't include windows.h */
typedef u32_t sys_thread_t;
#endif

#include  "err.h"

typedef signed long msg_q_t;
typedef  signed long mutex_t;
typedef signed long  sem_t;


#define MAX_MSG_IN_LWIP_MBOX  32

typedef struct tag_LWIP_MBOX
{
	msg_q_t			lwip_mbox_e;
	//void *lwip_msg_q[MAX_MSG_IN_LWIP_MBOX];
} LWIP_MBOX, *sys_mbox_t;


typedef mutex_t 	sys_mutex_t;
typedef sem_t 		sys_sem_t;
typedef int     	sys_thread_t;


#ifndef NULL
#define NULL 0
#endif


#define SYS_MBOX_NULL  (sys_mbox_t)NULL
#define SYS_SEM_NULL   (sys_sem_t)NULL

err_t sys_sem_new(sys_sem_t *sem,u8_t count);
void sys_sem_free(sys_sem_t *sem);
void sys_sem_signal(sys_sem_t *sem);

#endif /* __ARCH_SYS_ARCH_H__ */

