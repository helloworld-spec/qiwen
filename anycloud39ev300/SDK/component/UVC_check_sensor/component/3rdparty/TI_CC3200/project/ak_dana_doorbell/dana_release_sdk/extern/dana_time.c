#include <stdlib.h>
#include <time.h>

#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "dana_time.h"
#include "osi.h"

//大拿时间API接口， 厂商自行实现

/*
 * 睡眠 单位us 
 * 
 */
extern uint32_t dana_usleep(uint32_t microseconds)
{
	osi_Sleep(microseconds/1000);		//���ߺ���
        return 0;
}


/*
 *获取系统启动时间
 */
extern int64_t dana_update_time()//dana_update_time���ص�ʱ�䵥λΪ΢�뼶
{
    int64_t  timecnt = xTaskGetTickCount();//ϵͳxTaskGetTickCount���ص�λΪ���뼶
    
    return timecnt*1000;
}

