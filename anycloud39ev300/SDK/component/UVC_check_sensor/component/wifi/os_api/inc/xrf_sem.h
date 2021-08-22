#ifndef _SEM_H_
#define _SEM_H_
#include "xrf_api.h"


void sem_init(sem_t *sem, int value);
int sem_wait(sem_t *sem, unsigned int timeout);
int sem_post(sem_t *sem);
void sem_destory(sem_t *sem);
int is_sem_empty(sem_t *sem);
#endif
