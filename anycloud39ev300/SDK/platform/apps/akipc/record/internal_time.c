#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "internal_time.h"
#include "ak_common.h"

/** 
 * day_start_seconds - get current day total start seconds
 * @void
 * return: total seconds from 1970-1-1 00:00:00 to current day time 00:00:00
 */
time_t day_start_seconds(void)
{
	struct tm *cur_day = get_tm_time();

	/* set current day 00:00:00 */
	cur_day->tm_hour = 0x00;
	cur_day->tm_min = 0x00;
	cur_day->tm_sec = 0x00;

	return mktime(cur_day);
}

/** 
 * get_day_seconds - current day time from today's 00:00:00
 * @void
 * return: diff seconds from today's 00:00:00
 */
time_t get_day_seconds(void)
{
	return (get_passed_seconds() - day_start_seconds());
}

/** 
 * day_secs_to_total - transfer day seconds to passed total seconds 
 *		from 1970-01-01 00:00:00
 * @void
 * return: passed total seconds after transferred
 */
time_t day_secs_to_total(time_t day_secs)
{
	return (day_start_seconds() + day_secs);
}

/** 
 * add_show_time - add current message show time
 * return: none
 */
void add_show_time(void)
{
	struct tm pCur;
	struct timeval tCurTime;

	gettimeofday(&tCurTime, NULL);
	/* localtime_r is safe for thread */
    localtime_r(&(tCurTime.tv_sec), &pCur);

    printf(" [%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d-%3.3ld]\n",
    	(1900+pCur.tm_year), (1+pCur.tm_mon), pCur.tm_mday,
    	pCur.tm_hour, pCur.tm_min, pCur.tm_sec, (tCurTime.tv_usec/1000));
}

/** 
 * get_readable_time_string - get current localtime and transfer to 
 *      readable time string
 * @void
 * return: readable time string after transferred
 * notes: string format: yyyy-MM-dd HH:mm:ss, ex: 2016-04-06 10:06:06
 *      time_str min len 20 bytes.
 */
char* get_readable_time_string(void)
{
	return ak_seconds_to_string(get_passed_seconds());
}

/** 
 * show_timeval
 * @cur_time: current time of struct timeval
 * return: none
 * notes: show at format: 2016-06-06 12:00:00-256
 */
void show_timeval(struct timeval *cur_time)
{
	struct tm cur;
	/* localtime_r is safe for thread */
    localtime_r(&(cur_time->tv_sec), &cur);	
    printf(" [%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d-%3.3ld]\n",
    	(1900+cur.tm_year), (1+cur.tm_mon), cur.tm_mday,
    	cur.tm_hour, cur.tm_min, cur.tm_sec, (cur_time->tv_usec/1000));
}
