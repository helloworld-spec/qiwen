/**
 * @FILENAME: sys_delay.c
 * @BRIEF sys_delay driver file
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @VERSION 1.0
 * @REF
 */
#include "anyka_cpu.h"
#include "anyka_types.h"
#include "drv_api.h"

#ifdef AKOS
#include "akos_api.h"
#endif

//asm code in int.s
#ifndef BURNTOOL
extern void asm_loop(unsigned long loop);
#endif


/**
 * @BRIEF minisecond delay
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM unsigned long minisecond: minisecond delay number
 * @RETURN void
 * @RETVAL
 */
void mini_delay(unsigned long minisecond)
{
    unsigned long clocks;
#ifdef AKOS    //0
    if (minisecond >= 10)
    {
        //akprintf(C3, M_DRVSYS, "mini_delay:%d, use sleep\n", minisecond);
        AK_Sleep(minisecond/5); //each tick 5ms
        return;
    }
#endif

    clocks = get_cpu_freq() / 5000 * minisecond;
#ifndef BURNTOOL
    asm_loop(clocks);
#endif

}

/**
 * @BRIEF microsecond delay
 * @AUTHOR guoshaofeng
 * @DATE 2007-04-23
 * @PARAM unsigned long us: microsecond delay number
 * @RETURN void
 * @RETVAL
 */
void us_delay(unsigned long us)
{
    unsigned long clocks;
    
    clocks = get_cpu_freq() / 1000000 * us / 5;
    #ifndef BURNTOOL
    asm_loop(clocks);
    #endif
}

