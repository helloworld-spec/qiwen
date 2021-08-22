#include <stdarg.h>
#include <stdio.h>
#include "ak_common.h"
#include "drv_api.h"
#include "fat_sdif.h"
#include "file.h"

#include "kernel.h"

#define LIBPLAT_COMMON_VERSION "libplat_common_1.0.01"

static char m_printbuffer[1024];
static int  m_loglevel=LOG_LEVEL_NORMAL;

/*
*  color define 
*/
#define NONE          "\033[m"			//close
#define NONE_N        "\033[m\n"		//close and new_line
#define RED           "\033[0;32;31m"	//red
#define LIGHT_RED     "\033[1;31m"		//red high light
#define GREEN         "\033[0;32;32m"	//green
#define LIGHT_GREEN   "\033[1;32m"		//green high light

const char* ak_common_get_version(void)
{
	return LIBPLAT_COMMON_VERSION;
}

/**
 * ak_print_null: NULL function
 * return: void
 * note:use for the lib print callback function
 */
void ak_print_null(void)
{

}

/* use for register call */
void print_normal(const char *fmt, ...)
{
    va_list args;   
    static char printbuffer[1024];

    va_start ( args, fmt );
    vsnprintf ( printbuffer, sizeof(printbuffer)-1, fmt, args );
    va_end ( args );        

    ak_print_normal(printbuffer);

}


/**
 * @brief: print function we defined for debugging
 *           the message will print only the level is smaller or equal than current level
 * @level[IN]: print level 
 * @fmt[IN]: format like printf()
 * @...[IN]: variable arguments list
 * @return: we return 0 always.
 */
int ak_print(int level, const char *fmt, ...) 
{
    va_list args;
    
	if (level > m_loglevel)
		return 0;
    
    va_start ( args, fmt );
    vsprintf ( m_printbuffer, fmt, args );
    va_end ( args );        

    if (level < LOG_LEVEL_NORMAL)
        printf(LIGHT_RED"%s"NONE, m_printbuffer);		//read color print
	else if (level > LOG_LEVEL_NORMAL)
        printf(LIGHT_GREEN"%s"NONE, m_printbuffer);		//green color print
	else
        printf(NONE"%s"NONE, m_printbuffer);		//normal color print

	return 0;
}

/**
 * @brief  set current print level
 * 
 * @param level  current level
 * @return 0 always
 */
int ak_print_set_level(int level)
{
    if (level <= LOG_LEVEL_DEBUG && level > LOG_LEVEL_RESERVED)
        m_loglevel = level;
        
    return 0;    
}

/**
 * @brief: sleep certain time
 * @param ms[IN]: milli-seconds
 * @return: none
 */
void ak_sleep_ms(const int ms) 
{
	int tick;

	tick =  ms /5;
	if (ms % 5 !=0)
		tick+=1;
		
	AK_Sleep(tick);
}

/**
 * @brief: mount file system 
 *
 * @param dev_id           device id , value is enum DEV_BLKID
 * @param partition_id     partition id
 * @param mountpoint       the path to mount point
 * @return 0: success, -1 :fail
 * @notice: partition_id and mountpoint  isn't useful in rtos
 */
int ak_mount_fs(int dev_id, int partition_id, const char *mountpoint)
{
	if (dev_id	== DEV_MMCBLOCK)
	{
		if (mount_sd())
		{
            if (FS_SetAsynWriteBufSize(2*1024*1024,0))
                printk("FS_SetAsynWriteBufSize ok!\n");
            else
                printk("FS_SetAsynWriteBufSize failed!\n");
		
			return 0;
		}
	}

	return -1;
}

/**
 * @brief: unmount file system 
 *
 * @param dev_id           device id , value is enum DEV_BLKID
 * @param partition_id     partition id
 * @param mountpoint       the path to mount point
 * @return 0: success, -1 :fail
 * @notice: partition_id and mountpoint  isn't useful in rtos
 */
int ak_unmount_fs(int dev_id, int partition_id, const char *mountpoint)
{
	if (dev_id	== DEV_MMCBLOCK)
	{
		if (unmount_sd())
		{
			return 0;
		}
	}

	return -1;

}

/**
 * @convert system time to seconds counted from 1970-01-01 00:00:00
 * @author YiRuoxiang
 * @date 2006-02-17
 * @param T_SYSTIME SysTime: system time structure
 * @return unsigned long: seconds counted from 1970-01-01 00:00:00
 */

static unsigned long convert_time_to_seconds(T_SYSTIME *SysTime)
{
	unsigned int current_time = 0;
	unsigned short MonthToDays  = 0;
	unsigned short std_month_days[13]  = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	unsigned short leap_month_days[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	unsigned short TempYear;
	unsigned short month;
	unsigned short i;

	if (NULL == SysTime)
	{
		return 0;
	}
	month = SysTime->month;

	if(SysTime->year < BASE_YEAR) 
		SysTime->year= BASE_YEAR;
	for (TempYear = SysTime->year - 1; TempYear >= BASE_YEAR; TempYear--)
	{
		/* the case of leap year */
		if (0 == (TempYear % 4) && ((TempYear % 100) != 0 || 0 == (TempYear % 400)))
		{
			current_time += 31622400; // the seconds of a leap year
		}
		else
		{
			/* not a leap year */
			current_time += 31536000;  // the seconds of a common year(not a leap one)
		}
	}

	/* calculate the current year's seconds */

	if ((month < 1) || (month > 12))
	{
		/* get the default value. */
		month		= 1;
		SysTime->month = 1;
	}

	/* the current year is a leap one */
	if (0 == (SysTime->year % 4) && ((SysTime->year % 100) != 0 || 0 == (SysTime->year % 400)))
	{
		for (i = 1; i < month; i++)
		{
			MonthToDays += leap_month_days[i];
		}
	}
	else
	{
		/* the current year is not a leap one */
		for (i = 1; i < month; i++)
		{
			MonthToDays += std_month_days[i];
		}
	}

	if ((SysTime->day < 1) || (SysTime->day > 31))
	{
		/* get the default value */
		SysTime->day = 1;
	}
	MonthToDays += (SysTime->day - 1);

	/* added the past days of this year(change to seconds) */
	current_time += MonthToDays * 24 * 3600;

	/* added the current day's time(seconds) */
	current_time += SysTime->hour * 3600 + SysTime->minute * 60 + SysTime->second;

	return current_time;
}



/**
 * @convert seconds counted from 1970-01-01 00:00:00 to system time
 * @author YiRuoxiang
 * @date 2006-02-16
 * @param  the seconds want to added, which are counted from 1970-01-01 00:00:00
 * @the seconds can't be more than 4102444799 (2099-12-31 23:59:59)
 * @param T_SYSTIME *SysTime: system time structure pointer
 * @return unsigned char
 * @retval if system time is more than 2099-12-31 23:59:59, return false
 *          else return true;
 */
static void convert_seconds_to_systime(unsigned long seconds, T_SYSTIME *SysTime)
 {
 	unsigned int TmpVal;
    unsigned int year, mouth;
	
	const  unsigned char month_std_day[13]  = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	const  unsigned char month_leap_day[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
    if (NULL == SysTime)
    {
        return;
    }

    TmpVal       = seconds % 60;
    SysTime->second = (unsigned char)TmpVal;
    TmpVal       = seconds / 60;
    SysTime->minute = (unsigned char)(TmpVal % 60);
    TmpVal       = TmpVal / 60;
    SysTime->hour   = (unsigned char)(TmpVal % 24);

    /* day is from the number: 1, so we should add one after the devision. */
    TmpVal  = (seconds / (24 * 3600)) + 1;
    year    = BASE_YEAR;
    while (TmpVal > 365)
    {
        if (0 == (year % 4) && ((year % 100) != 0 || 0 == (year % 400)))
        {
            TmpVal -= 366;
            /* Added to avoid the case of "day==0" */
            if (0 == TmpVal)
            {
                /* resume the value and stop the loop */
                TmpVal = 366;
                break;
            }
        }
        else
        {
            TmpVal -= 365;
        }
        year++;
    }
    
    mouth = 1;
    if (0 == (year % 4) && ((year % 100) != 0 || 0 == (year % 400)))
    {
        while (TmpVal > month_leap_day[mouth])
        {
            TmpVal -= month_leap_day[mouth];
            mouth++;
        }
    }
    else
    {
        /* Dec23,06 - Modified from month_leap_day[], since here is not leap year */
        while (TmpVal > month_std_day[mouth])
        {
            TmpVal -= month_std_day[mouth];
            mouth++;
        }
    }

	SysTime->year  = (unsigned short)year;
	SysTime->month = (unsigned char)mouth;
	SysTime->day   = (unsigned char)TmpVal;

	/*week*/
	TmpVal  = (seconds / (24 * 3600)) % 7;
	SysTime->week = BASE_WEEK + TmpVal;
	if(SysTime->week > 7)
    {
        SysTime->week -= 7;
    }
    return;
}



/**
 * @brief get date
 * 
 * @param date [out] pointer to date struct
 * @return int 0: success, -1 :fail
 */
int ak_get_localdate(struct ak_date * date)
{
    T_SYSTIME systime;
	unsigned long sys_curr_seconds;

	systime = rtc_get_systime();
	
    date->year = systime.year;
    date->month = systime.month;
    date->day = systime.day;
	
    date->hour = systime.hour;
    date->minute = systime.minute;
    date->second = systime.second;

    date->timezone = 8 ;//beijin time

    return 0;           
}

/**
 * ak_set_localdate - set local date time
 * @date[IN]: set date time
 * return: 0 success, -1 failed
 */
int ak_set_localdate(const struct ak_date *date)
{
    T_SYSTIME systime1,systime2;
	unsigned long sys_curr_seconds;
    
    unsigned long seconds = 0;
    systime1.year    =  date->year; 
    systime1.month   =  date->month;
    systime1.day     =  date->day;  
    systime1.hour    =  date->hour;    
    systime1.minute  =  date->minute;  
    systime1.second  =  date->second ; 
	seconds = convert_time_to_seconds(& systime1);
	memset(&systime2, 0, sizeof(systime2));
	convert_seconds_to_systime(seconds, &systime2);

	rtc_set_systime(&systime2);
	
    return 0;                
}


/** 
 * ak_get_ostime - get OS time since cpu boot
 * @tv[OUT]: time value since startup
 * return: void
 */
void ak_get_ostime(struct ak_timeval *tv)
{
    unsigned long long tick;

    tick = get_tick_count_us();

    if (NULL !=tv)
    {
        tv->sec  = tick / 1000000;
        tv->usec = tick % 1000000;
    }
    return;
}

/** 
 * ak_diff_ms_time - diff value of ms time between cur_time and pre_time
 * @cur_time[IN]: current time 
 * @pre_time[IN]: previous time 
 * return: diff time, uint: ms
 */
long ak_diff_ms_time(const struct ak_timeval *cur_time,
					const struct ak_timeval *pre_time)
{
    unsigned long pre_ms,cur_ms;
    long diff;
    
	if (NULL==cur_time || NULL==pre_time) {
		return 0;
	}
	pre_ms = pre_time->sec * 1000 + pre_time->usec/1000;
	cur_ms = cur_time->sec * 1000 + cur_time->usec/1000;

    diff = cur_ms - pre_ms +0xffffffff+1;

    return diff;
	
}
/** 
 * ak_date_to_seconds - transfer date time value to seconds
 * @seconds[IN]: seconds from 1970-01-01 00:00:00
 * @date[OUT]: date time value
 * return: 0 success; -1 failed
 * notes: seconds from 1970-01-01 00:00:00 +0000(UTC)
 */
long ak_seconds_to_date(long seconds, struct ak_date *date)
{
    T_SYSTIME sys_time;
    
    if(!date){
        return -1;
    }

    convert_seconds_to_systime(seconds, &sys_time);
    
    date->year  = sys_time.year;
    date->month = sys_time.month;
    date->day   = sys_time.day;
    date->hour  = sys_time.hour;
    date->minute= sys_time.minute;
    date->second= sys_time.second;
    
    return 0;
}

/** 
 * ak_date_to_seconds - transfer date time value to seconds
 * @date[IN]: date time value
 * return: seconds after transferred; failed -1
 * notes: seconds from 1970-01-01 00:00:00 +0000(UTC)
 */
long ak_date_to_seconds(const struct ak_date *date)
{
    T_SYSTIME sys_time;
    
	if(!date){
        return -1;
    }

    sys_time.year   = date->year;
    sys_time.month  = date->month;
    sys_time.day    = date->day;
    sys_time.hour   = date->hour;
    sys_time.minute = date->minute;
    sys_time.second = date->second;
    
    return convert_time_to_seconds(&sys_time);
}
					
/** 
 * ak_date_to_string - transfer date time value to time string
 * @date[IN]: date time value
 * @str[OUT]: date time string after transfer OK
 * return: 0 success; -1 failed
 * notes: 1. string format: yyyyMMdd-HHmmss, ex: 20160406-100606
 *		2. MAKE SURE str has enough space outside
 */
int ak_date_to_string(const struct ak_date *date, char *str)
{
    if(!date || !str) {
        return AK_FAILED;
    }
   
    sprintf(str, "%4.4d%2.2d%2.2d-%2.2d%2.2d%2.2d",
    	date->year, (date->month), (date->day),
    	date->hour, date->minute, date->second);

    return AK_SUCCESS;
}

/** 
 * ak_string_to_date - transfer date time string to date time value
 * @time_str[IN]: time string
 * @date[OUT]: date time value after transfer OK
 * return: 0 success; otherwise -1
 * notes: string format: yyyyMMdd-HHmmss, ex: 20160406-100606
 */
int ak_string_to_date(const char *time_str, struct ak_date *date)
{
    if(!time_str || !date){
        return AK_FAILED;
    }

    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int min = 0;
    int sec = 0; 

    /* match 6 letters exactly */
	if(6 != sscanf(time_str, "%4d%2d%2d-%2d%2d%2d", 
    	&year, &month, &day, &hour, &min, &sec)){
		return AK_FAILED;
    }
	
    date->year = year;
    date->month = month;
    date->day = day;
    date->hour = hour;
    date->minute = min;
    date->second = sec;
    date->timezone = 0;

    return AK_SUCCESS;
}

/** 
 * ak_print_date - print date time value
 * @date[IN]: date time value
 * return: 0 success; -1 failed
 * notes: format: yyyyMMdd-HHmmss, ex: 20160406-100606
 */
int ak_print_date(const struct ak_date *date)
{
	if(!date){
        return AK_FAILED;
    }
    
	ak_print_normal(" [%4.4d%2.2d%2.2d-%2.2d%2.2d%2.2d]",
    	date->year, (date->month), (date->day),
    	date->hour, date->minute, date->second);

	return AK_SUCCESS;    	
}


/**
 * ak_check_file_exist - check appointed path dir/file exist
 * @file[IN]: absolutely path
 * return: 0 on exist, others not
 */
int ak_check_file_exist(const char *path)
{
//	return access(path, F_OK);
    FILE * f;

    f=fopen(path,"r");
    if (NULL ==f)
        return 1;
    else
    {
        fclose(f);
        return 0;
    }
}

/**
 * ak_is_regular_file - Check whether appointed file is a regular file 
 * @file_path[IN]: absolutely file path
 * return: 1 regular file, 0 not regular file
 */
int ak_is_regular_file(const char *file_path)
{
	unsigned long ak_handle;
    int ret=0;

	ak_handle = File_OpenAsc( 0 , file_path, FILE_MODE_READ);

    if (0 ==ak_handle)
    {
        return 0;
    }
    else
    {
       
        if (File_IsFile(ak_handle))
            ret = 1;
        else          
            ret = 0;
            
        File_Close(ak_handle);
        return ret;
    }
    
}


