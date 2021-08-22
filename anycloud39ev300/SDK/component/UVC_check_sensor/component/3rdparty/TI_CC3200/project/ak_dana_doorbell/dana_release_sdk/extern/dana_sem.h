#ifndef _DANA_SEM_H_
#define _DANA_SEM_H_

#include "dana_base.h"

#ifdef __cplusplus
extern "C"
{
#endif

//信号量 厂商自行实现 有名或者无名

typedef struct dana_sem_handler dana_sem_handler_t;

/*
 * 创建一个信号量(库内部不管是有名还是无名)
 */
dana_sem_handler_t * dana_sem_create();


/*
 * 等待信号量触发
 */
int32_t dana_sem_wait(dana_sem_handler_t *sem);

/*
 * 触发信号量
 */
int32_t dana_sem_post(dana_sem_handler_t *sem);

/*
 * 销毁信号量
 */
int32_t dana_sem_destroy(dana_sem_handler_t *sem);

#ifdef __cplusplus
extern "C"
{
#endif
#endif
