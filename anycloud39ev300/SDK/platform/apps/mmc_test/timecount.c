#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif
#include <time.h>

#include "timecount.h"
#include "printcolor.h"

struct timeval *timeval_mark( struct timeval *p_timeval )
{
    gettimeofday( p_timeval , 0 ) ;
    return p_timeval ;
}

long long timeval_count( struct timeval *p_timeval_begin , struct timeval *p_timeval_end )
{
    long long i_time = ( p_timeval_end->tv_sec - p_timeval_begin->tv_sec ) * SEC2USEC + ( p_timeval_end->tv_usec - p_timeval_begin->tv_usec ) ;
    if ( i_time > 0 ) {
        return i_time ;
    }
    else {
        return 0 ;
    }
}

struct timespec *timespec_mark( struct timespec *p_timespec )
{
	clock_gettime( CLOCK_MONOTONIC , p_timespec ) ;
	return p_timespec ;
}

long long timespec_count( struct timespec *p_timespec_begin , struct timespec *p_timespec_end )
{
    long long i_time = ( p_timespec_end->tv_sec - p_timespec_begin->tv_sec ) * ( long long )SEC2NSEC + ( ( long long )p_timespec_end->tv_nsec - p_timespec_begin->tv_nsec ) ;
    if ( i_time > 0 ) {
        return i_time ;
    }
    else {
        return 0 ;
    }
}