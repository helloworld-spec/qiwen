#ifndef _DANA_ATOMIC_H_
#define _DANA_ATOMIC_H_

#include "dana_base.h"

#ifdef __cplusplus
extern "C"
{
#endif


extern void dana_sync();

/*
 * 原子操作 对一个数进行原子加减
 */
extern int32_t dana_atomic(int *ptr, int val);

/*
 * 随机数自增 对一个随机数进行自增(随机数一次后固定)
 */
extern int32_t dana_seq_auto();

#ifdef __cplusplus
extern "C"
{
#endif
#endif
