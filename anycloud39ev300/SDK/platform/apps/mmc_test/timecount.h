#ifndef SEC2USEC
#define SEC2USEC 1000000
#endif

#ifndef SEC2NSEC
#define SEC2NSEC 1000000000
#endif

struct timeval *timeval_mark( struct timeval *p_timeval ) ;
long long timeval_count( struct timeval *p_timeval_begin , struct timeval *p_timeval_end ) ;
struct timespec *timespec_mark( struct timespec *p_timespec ) ;
long long timespec_count( struct timespec *p_timespec_begin , struct timespec *p_timespec_end ) ;