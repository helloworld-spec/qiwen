#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <signal.h>
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif
#include <time.h>

#ifdef DEBUG_ANYKA
#include "ak_common.h"
#endif
#include "printcolor.h"

#ifndef GLOBAL_VALUE
#define GLOBAL_VALUE
char gac_debug[ LEN_DEBUG ] , gac_time[ LEN_TIME ] ;
char *gpc_prog_name = NULL , gc_run = PROG_TRUE ;
#endif

char *get_color( int i_color_mode , int i_color_back , int i_color_front )
{
    static char ac_color[ COLOR_LEN ] ;
    snprintf( ac_color , COLOR_LEN , COLOR_BEGIN , i_color_mode , i_color_back , i_color_front ) ;
    return ac_color ;
}

char *get_time_now( char *pc_time )
{
    static struct tm tm_now ;
    static struct timeval timeval_now ;
    char *pc_now = gac_time ;

    if ( pc_time != NULL ) {
        pc_now = pc_time ;
    }
    gettimeofday( &timeval_now , NULL ) ;
    localtime_r( &timeval_now.tv_sec , &tm_now ) ;
    snprintf( pc_now , LEN_TIME , "%04d-%02d-%02d %02d:%02d:%02d.%06u",
                       tm_now.tm_year + 1900 , tm_now.tm_mon + 1 , tm_now.tm_mday ,
                       tm_now.tm_hour, tm_now.tm_min , tm_now.tm_sec , ( unsigned int )timeval_now.tv_usec ) ;
    return pc_now ;
}

/*
获取线程id
*/
pid_t gettid()
{
    return syscall( SYS_gettid ) ;
}

void signal_exit( int i_sig )
{
    gc_run = PROG_FALSE ;
    DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED )
    return ;
}
