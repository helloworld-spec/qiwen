#ifndef _MUTEX_H_
#define _MUTEX_H_
#include "xrf_api.h"

#ifdef OS_UCOS
#ifdef UCOS_V3
typedef OS_MUTEX* mutex_t;
#else
typedef OS_EVENT* mutex_t;
#endif
#endif
#ifdef OS_FREE_RTOS
typedef void* mutex_t;
#endif

mutex_t mutex_init(const char *name);

int mutex_lock(mutex_t mutex);

int mutex_unlock(mutex_t mutex);

void mutex_destory(mutex_t mutex);

#define mutex_free mutex_destory

#endif
