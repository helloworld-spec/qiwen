#ifndef _TIMER_H_
#define _TIMER_H_
#include "xrf_api.h"
 

typedef void(*timer_callback_func)(void *parg);

int os_create_timer(int *timer_id, int type, timer_callback_func callback, void *callback_arg);

int os_start_timer(int timer_id, unsigned int expires);

int os_stop_timer(int timer_id);

int os_free_timer(int timer_id);

void sleep(uint32_t ms);

unsigned int os_time_get(void);
unsigned int os_ticks_get(void);

unsigned int ms2ticks(unsigned int ms);

//extern volatile uint32_t jiffies, tick_ms;


#endif
