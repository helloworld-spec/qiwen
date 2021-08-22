/**
 * @file idle.c
 * @brief Idle Thread Implementation for Running A Idle Thread to Save Power
 *
 * Copyright (C) 2016 Anyka (Guangzhou) Microelectronics Technology Co., Ltd.
 * @version 2.0
 */
#include "os_malloc.h"
#include "drv_api.h"
#ifdef AKOS
#include "akos_api.h"
#endif
#include "print.h"

#ifdef OS_ANYKA

#define IDLE_THREAD_NAME        "IdleThread"
#define IDLE_THREAD_PRIORITY    254
#define IDLE_THREAD_TIME_SLICE    1
#define IDLE_THREAD_STACK_SIZE  512

//update idle state every 2S
#define IDLE_SLICE              (2000)

#define CPU_HALT
//#undef CPU_HALT

typedef struct tagIdleThread{
    T_hTask         handle;
    unsigned long   *stack;
    bool            bRun;
}T_IDLE_THREAD;

static T_IDLE_THREAD idleThread = {0};
static volatile unsigned long m_idle = 0;
static volatile bool m_idle_update = false;

extern unsigned long cpu_halt(void);
    
static void idle_thread_entry(void * pData)
{
    unsigned long idle     = 0;
    
#if (defined CPU_HALT)
    unsigned long tick;    
    unsigned long sec     = 0;     
    
    sec = get_tick_count();
#endif

    //printk("enter idle thread\r\n");  
        
    while(idleThread.bRun)
    {
#if (defined CPU_HALT)

        AK_Drv_Protect();    
        tick = get_tick_count();
        
        cpu_halt();
        
        idle += get_tick_count()-tick;    
        AK_Drv_Unprotect();
        
        if (get_tick_count()- sec > IDLE_SLICE)    
        {    
            idle = idle * 100 / (get_tick_count()-sec);
        
            m_idle = idle;
            m_idle_update = true;
                
            sec = get_tick_count();
        
            idle = 0;    
        }
#else
        //read register to reduce power
        idle = *(volatile unsigned long *)0x08000000;
#endif
    }
    
    printk("exit idle thread\r\n");
}

unsigned long idle_get_cpu_idle(void)
{
#if (defined CPU_HALT)
    return m_idle;
#else
    printk("not supported\r\n");
    return 0;
#endif
}

unsigned long idle_get_cpu_usage(void)
{     
    if (m_idle_update)
    {
        m_idle_update = false; 
        return (100 - m_idle);
    }
    else
    {
        return 100;
    }
}

bool idle_thread_create(void)
{
    if (!idleThread.bRun)
    {          
        idleThread.stack = Fwl_Malloc(IDLE_THREAD_STACK_SIZE);
        idleThread.handle = AK_Create_Task((void *)idle_thread_entry, (unsigned char*)"IDLE", 
                0, NULL,
                idleThread.stack, IDLE_THREAD_STACK_SIZE,
                IDLE_THREAD_PRIORITY, IDLE_THREAD_TIME_SLICE,
                AK_PREEMPT, AK_START);
    	
        if(AK_IS_INVALIDHANDLE(idleThread.handle))
        {
            printk("idle_thread_create():Invalid Handle!\r\n");
            return true;
        }
        idleThread.bRun = true;
    }

    return true;
}

bool idle_thread_destroy(void)
{
    if(!idleThread.bRun)
    {
        return true;
    }

    idleThread.bRun = false;    
    AK_Sleep(2);
    
    AK_Terminate_Task(idleThread.handle);  

    return true;
}

bool idle_thread_is_created(void)
{
    if (!idleThread.bRun)
    {
        return false;
    }

    return true;
}

#endif    // OS_ANYKA
