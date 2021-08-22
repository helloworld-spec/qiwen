#ifndef _RTOS_H_
#define _RTOS_H_

#if 0
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>

typedef pdTASK_CODE                 OsTask;
typedef xTaskHandle                 OsTaskHandle;
typedef xSemaphoreHandle            OsMutex;
typedef xSemaphoreHandle            OsSemaphore;
typedef xQueueHandle                OsMsgQ;
typedef xTimerHandle                OsTimer;
typedef tmrTIMER_CALLBACK           OsTimerHandler;
#else
typedef void *                       OsTaskHandle;
typedef void *                       OsMutex;
typedef void *                       OsSemaphore;
typedef void *                       OsMsgQ;
typedef void *                       OsTimer;
typedef void (*OsTimerHandler)(OsTimer);
typedef void (*OsTask)(void *);
#endif

/* Define Task Priority: 0 is the lowest priority */
#define OS_TASK_PRIO0               0
#define OS_TASK_PRIO1               1
#define OS_TASK_PRIO2               2
#define OS_TASK_PRIO3               3

/* Define OS error values */
#define OS_SUCCESS                  0
#define OS_FAILED                   1

#define OS_MS2TICK(_ms)             ( (_ms)/portTICK_RATE_MS )

/* Message Commands: */
#define OS_MSG_FRAME_TRAPPED        1

typedef struct OsMsgQEntry_st
{
    u32         MsgCmd;
    void        *MsgData;
} OsMsgQEntry, *POsMsgQEntry;


typedef struct task_info_st 
{
    const s8   *task_name;
    OsMsgQ       qevt;
    u32          qlength;
    u32          prio;
    u32          stack_size;  /* unit: 16 */
    void       *args;
    TASK_FUNC    task_func;
} task_info;

/**
 *  Flag to indicate whether ISR handler is running or not.
 */
extern volatile u32 gOsFromISR;

OS_APIs s32  OS_Init( void );

/* Task: */
OS_APIs s32  OS_TaskCreate( OsTask task, const s8 *name, u16 stackSize, void *param, u32 pri, OsTaskHandle *taskHandle );
OS_APIs void OS_TaskDelete( OsTaskHandle taskHandle );
OS_APIs void OS_StartScheduler( void );
OS_APIs void OS_Terminate( void );
OS_APIs const char * OS_TaskGetName (void);

/* Mutex: */
OS_APIs s32  OS_MutexInit( OsMutex *mutex );
OS_APIs void OS_MutexLock( OsMutex mutex );
OS_APIs void OS_MutexUnLock( OsMutex mutex );

/* Delay: */
OS_APIs void OS_MsDelay(u32 ms);

/* Timer: */
OS_APIs s32 OS_TimerCreate( OsTimer *timer, u32 ms, u8 autoReload, void *args, OsTimerHandler timHandler );
OS_APIs s32 OS_TimerSet( OsTimer timer, u32 ms, u8 autoReload, void *args );
OS_APIs s32 OS_TimerStart( OsTimer timer );
OS_APIs s32 OS_TimerStop( OsTimer timer );
OS_APIs void *OS_TimerGetData( OsTimer timer );

//OS_APIs void OS_TimerGetSetting( OsTimer timer, u8 *autoReload, void **args );
//OS_APIs bool OS_TimerIsRunning( OsTimer timer );

/* Message Queue: */
OS_APIs s32 OS_MsgQCreate( OsMsgQ *MsgQ, u32 QLen );
OS_APIs s32 OS_MsgQEnqueue( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, bool fromISR );
OS_APIs s32 OS_MsgQDequeue( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, bool block, bool fromISR );
OS_APIs s32 OS_MsgQWaitingSize( OsMsgQ MsgQ );
#if 0
OS_APIs void *OS_MsgAlloc( void );
OS_APIs void OS_MsgFree( void *Msg );
#endif
OS_APIs s32 OS_MsgQEnqueueFront( OsMsgQ MsgQ, OsMsgQEntry *MsgItem, bool fromISR );

/* Memory: */
OS_APIs void *OS_MemAlloc( u32 size );
OS_APIs void OS_MemFree( void *m );

/* Critical Sections */
OS_APIs void OS_EnterCritical (void);
OS_APIs void OS_LeaveCritical (void);


#endif /* _RTOS_H_ */

