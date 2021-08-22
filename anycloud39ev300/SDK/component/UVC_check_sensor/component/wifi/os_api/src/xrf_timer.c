//#define DEBUG
#include "xrf_api.h"

#define TIMER_NUM_MAX                           32

typedef struct
{
	timer_callback_func callback_func;
        void   *callback_arg;                  
	bool 	  pending;
} T_TIMER_DATA_EX;

T_TIMER_DATA_EX timer_data_ex[TIMER_NUM_MAX];

void _comm_timer_callback(unsigned int id, unsigned int delay)
{
	if(id < TIMER_NUM_MAX)
	{
		if(timer_data_ex[id].callback_func)
			timer_data_ex[id].callback_func(timer_data_ex[id].callback_arg);
		else
			p_err_fun;
	}
}


int os_create_timer(int *timer_id, int type, timer_callback_func callback, void *callback_arg)
{
	T_TIMER tmr;
	p_dbg_enter;

	tmr = vtimer_start(1000, type, _comm_timer_callback);
	if(tmr >=0 && tmr < TIMER_NUM_MAX)
	{
		timer_data_ex[tmr].callback_func = callback;
		timer_data_ex[tmr].callback_arg = callback_arg;
		timer_data_ex[tmr].pending = FALSE;
		//vtimer_pause(tmr);
	}else
		p_err_fun;
	*timer_id = tmr;
	p_dbg("os_create_timer ret: timer id %d, type:%d", tmr , type);
	
	return tmr;
}

//expires ----n*ms
int os_start_timer(int timer_id, unsigned int expires)
{
	int ret;
	p_dbg_enter;
	if(timer_id >=0 && timer_id < TIMER_NUM_MAX)
	{
		//TODO: should use this pending? when one thread stop a timer and another thread start the timer?
		//if(timer_data_ex[timer_id].pending != TRUE)
		{
			vtimer_reset(timer_id, expires);
			return  0;
		}
	}
	return  -1;
}


//停止定时器
int os_stop_timer(int timer_id)
{
	p_dbg_enter;
	vtimer_pause(timer_id);
	return  0;
}

//释放删除定时器
int os_free_timer(int timer_id)
{
	p_dbg_enter;
	vtimer_stop(timer_id);
	timer_data_ex[timer_id].callback_func = 0;
	timer_data_ex[timer_id].callback_arg = 0;
	timer_data_ex[timer_id].pending = FALSE;
	return 0;
}

void sleep(uint32_t ms)
{
	u32 s = 0;
	/*
	if(in_interrupt()){
	
		delay_us(ms*1000);
		return;
	}*/
	ms = ms2ticks(ms);

	AK_Sleep(ms);
}

#pragma arm section code ="_bootcode_"
//ms
unsigned int os_time_get(void)
{
	return (unsigned int)get_tick_count();
}
#pragma arm section code 

unsigned int os_ticks_get(void)
{
	return (unsigned int)os_time_get()/(1000uL/OS_TICK_RATE_HZ);
}

#pragma arm section code ="_video_server_"

unsigned int ms2ticks(unsigned int ms)
{
	if(ms == 0)
		return AK_SUSPEND;
	else{
		if(ms < 10)
			ms = 10;
			ms = ms * OS_TICK_RATE_HZ / 1000uL;
	}
	return ms;
}
#pragma arm section code 

