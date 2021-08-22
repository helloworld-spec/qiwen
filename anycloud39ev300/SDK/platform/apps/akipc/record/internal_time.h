#ifndef _ANYKA_INTERNAL_TIME_H_
#define _ANYKA_INTERNAL_TIME_H_

#include <time.h>
#include <sys/time.h>

/** 
 * get_passed_seconds - passed total seconds from 1970-01-01 00:00:00
 * @void
 * return: passed total seconds
 */
static inline time_t get_passed_seconds(void)
{
	return time((time_t *)NULL);
}

/** 
 * get_tm_time - transfer cur_secs to real world date & time
 * @void
 * return: date & time from struct tm
 */
static inline struct tm* get_tm_time(void)
{
	time_t cur_secs = get_passed_seconds();
	return localtime(&cur_secs);
}

/** 
 * day_start_seconds - get current day total start seconds
 * @void
 * return: total seconds from 1970-1-1 00:00:00 to current day time 00:00:00
 */
time_t day_start_seconds(void);

/** 
 * get_day_seconds - current day time from today's 00:00:00
 * @void
 * return: diff seconds from today's 00:00:00
 */
time_t get_day_seconds(void);

/** 
 * day_secs_to_total - transfer day seconds to passed total seconds 
 *		from 1970-01-01 00:00:00
 * @day_secs[IN]:day seconds
 * return: passed total seconds after transferred
 */
time_t day_secs_to_total(time_t day_secs);

/** 
 * add_show_time - add current message show time
 * return: none
 */
void add_show_time(void);

/** 
 * get_readable_time_string - get current localtime and transfer to 
 *      readable time string
 * @void
 * return: readable time string after transferred
 * notes: string format: yyyy-MM-dd HH:mm:ss, ex: 2016-04-06 10:06:06
 *      time_str min len 20 bytes.
 */
char* get_readable_time_string(void);

/** 
 * show_timeval
 * @cur_time: current time of struct timeval
 * return: none
 * notes: show at format: 2016-06-06 12:00:00-256
 */
void show_timeval(struct timeval *cur_time);

#endif
