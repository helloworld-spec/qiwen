#ifndef _DANA_TIME_H_
#define _DANA_TIME_H_

#include "dana_base.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * 获取开机启动时间 单位us 
 * 
 */
extern int64_t dana_update_time();

/*
 * 睡眠 单位us 
 * 
 */
extern uint32_t dana_usleep(uint32_t microseconds);

#ifdef __cplusplus
extern "C"
{
#endif
#endif
