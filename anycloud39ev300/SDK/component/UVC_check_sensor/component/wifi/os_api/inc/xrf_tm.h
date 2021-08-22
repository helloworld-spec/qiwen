#ifndef _TM_H_
#define _TM_H_
#include "xrf_api.h"

typedef struct
{
	unsigned int year; //从2000年开始计算,0:表示2000年
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;
} tm_t;

typedef unsigned int time_t;

void sys_set_tm(tm_t *tm);

void sys_get_tm(tm_t *tm);

time_t sys_get_sec(void);

time_t sys_get_date(void);

int os_time_get(void);

#endif
