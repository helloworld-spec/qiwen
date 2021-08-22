#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/mman.h>

#include "list.h"
#include "ak_common.h"

/* print color define */
#define NONE          "\033[m"          //close
#define NONE_N        "\033[m\n"        //close and new_line
#define RED           "\033[0;32;31m"   //red
#define LIGHT_RED     "\033[1;31m"      //light red
#define GREEN         "\033[0;32;32m"   //green
#define LIGHT_GREEN   "\033[1;32m"      //light green
#define BLUE          "\033[0;32;34m"	//blue
#define LIGHT_BLUE    "\033[1;34m"		//light blue
#define DARK_GREY     "\033[1;30m"		//dark grey
#define LIGHT_GREY    "\033[0;37m"		//light grey
#define CYAN          "\033[0;36m"		//syan
#define LIGHT_CYAN    "\033[1;36m"		//light cyan
#define PURPLE        "\033[0;35m"		//purple
#define LIGHT_PURPLE  "\033[1;35m"		//light purple
#define BROWN         "\033[0;33m"		//brown
#define YELLOW        "\033[1;33m"		//yellow
#define WHITE         "\033[1;37m"		//white

#define MAX_TRACE_SIZE   		16
#define MODULE_NAME_MAX_SIZE	20
#define DATE_YEAR_DIFF			1900
#define LOG_BUF_LEN 			(1024)
#define AK_PRINT_LEVEL_IDENT 	"/var/log/ak_print_level"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

struct module_print_info {
    char name[MODULE_NAME_MAX_SIZE]; //<module name>
    unsigned char enable;   //module print flag
    struct list_head list;	//entry list
};

static const char common_version[] = "libplat_common V1.0.01";

/* default level LOG_LEVEL_NORMAL */
static int log_level = LOG_LEVEL_NORMAL;
static unsigned char set_default_level = AK_TRUE;
static struct list_head module_head = LIST_HEAD_INIT(module_head);

const char* ak_common_get_version(void)
{
	return common_version;
}

static inline int get_cur_log_level(void)
{
	char buf[10] = {0};
	static FILE *filp = NULL;
	static void *addr = NULL;

	/*
	 * If log ident file not exist, we will create this file
	 * and set to defalt level 3.
	 */
	if (set_default_level) {
		set_default_level = AK_FALSE;

		filp = fopen(AK_PRINT_LEVEL_IDENT, "w+");
		if (filp) {
			sprintf(buf, "%d", log_level);
			fwrite(buf, 1, strlen(buf), filp);
			/* map file */
			addr = mmap(NULL, 10, PROT_READ, MAP_PRIVATE, fileno(filp), 0);
			if (addr) {
				printf("map log file ok\n");
			}
			fclose(filp);
		} else {
			fprintf(stderr, "\n\tfopen() %s failed, %s\n\n",
				AK_PRINT_LEVEL_IDENT, strerror(errno));
		}
		printf("default log level=%d\n", log_level);
	} else {

		int level = log_level;
		if (addr) {
			level = atoi((const char *)addr);	
		} else {
			filp = fopen(AK_PRINT_LEVEL_IDENT, "r");
			if (filp) {
				fread(buf, 1, 9, filp);
				fclose(filp);
			}
			level = atoi(buf);
		}

		if (level > LOG_LEVEL_DEBUG)
			level = LOG_LEVEL_DEBUG;
		else if (level < LOG_LEVEL_RESERVED)
			level = LOG_LEVEL_RESERVED;

		log_level = level;
	}

	return log_level;
}

/* AK_TRUE - print; AK_FALSE - do't print */
static unsigned char check_module_print(char *print_buf)
{
    unsigned char print_flag = AK_TRUE;
    int name_len = 0;
    int tmp_len = 0;
    char name[MODULE_NAME_MAX_SIZE] = {0};
	struct module_print_info *entry = NULL;

    /* find module tag in print buffer */
    char *end = NULL;
	char *start = strchr(print_buf, '<');
	if (start) {
		end = strchr(start, '>');
		if (end) {
		    ++end;
		    tmp_len = (end - start);
		    memcpy(name, start, tmp_len);
		}
	}

	if (tmp_len > 0) {
	    list_for_each_entry(entry, &module_head, list) {
	        name_len = strlen(entry->name);
	        if ((name_len > 0) && (0 == memcmp(entry->name, name, name_len))) {
    		    print_flag = entry->enable;
    			break;
    	    }
    	}

    	/* remove module tag */
	    if (print_flag) {
	        int buf_len = strlen(print_buf);
	        int copy_len = (buf_len - tmp_len);
            memcpy(start, end, copy_len);

            /* clear extra char */
            memset(start+copy_len, 0x00, tmp_len);
	    }
	}

	return print_flag;
}

#ifndef AK_RTOS
static int backtrace(void **array, int size)
{
	if(size <= 0) {
		return 0;
	}

	int *fp_t = NULL;
	int *next_fp = NULL;
	int cnt = 0;
	int ret = 0;

	__asm__(
		"mov %0,fp\n"
		: "=r"(fp_t)
	);

	/* if compile -O2,  asm position maybe replace and fp_t == 0 */
    if(0 == fp_t )
		return 0;

	array[cnt++] = (void *) (*(fp_t ));
	next_fp = (int *)(*(fp_t - 1));

	while((cnt <= size) &&(next_fp != 0)) {
		array[cnt++] = (void *)*(next_fp);
		next_fp = (int *)(*(next_fp - 1));
	}

	ret = ((cnt <= size) ? cnt:size);
	ak_print_normal_ex("backstrace (%d deep)", ret);

	return ret;
}

static char** backtrace_symbols(void* const *array, int size)
{
#define WORD_WIDTH 8
	Dl_info info[size];
	int status[size];
	int cnt;
	size_t total = 0;

	/* fill in the information we can get from `dladdr` */
	for (cnt = 0; cnt < size; ++cnt) {
		status[cnt] = dladdr(array[cnt],&info[cnt]);
		if (status[cnt] && info[cnt].dli_fname
			&& (info[cnt].dli_fname[0] != '\0')) {
			total += strlen(info[cnt].dli_fname? :"");
			if (info[cnt].dli_sname) {
				total += (strlen(info[cnt].dli_sname) + 3 + WORD_WIDTH + 3);
			} else {
				total += 1;
			}
			total += (WORD_WIDTH + 5);
		} else {
			total += (WORD_WIDTH + 5);
		}
	}

	char **result = (char **)malloc(size * sizeof(char *) + total);
	if(result){
		char *last = (char *)(result +size);
		for(cnt = 0; cnt < size; ++cnt) {
			result[cnt] = last;
			if(status[cnt] && info[cnt].dli_fname
				&& (info[cnt].dli_fname[0] != '\0')) {
				char buf[20] = {0};
				if(array[cnt] >= (void *)info[cnt].dli_saddr) {
					sprintf(buf,"+%#1x",(unsigned int)(array[cnt] - info[cnt].dli_saddr));
				} else {
					sprintf(buf,"+%#1x",(unsigned int)(info[cnt].dli_saddr - array[cnt]));
				}

				last += (1 + sprintf(last,"%s%s%s%s%s[%p]",
					info[cnt].dli_fname ? :"",
					info[cnt].dli_sname ? "(":"",
					info[cnt].dli_sname ? :"",
					info[cnt].dli_sname ? buf:"",
					info[cnt].dli_sname ? ")":"",
					array[cnt]));

			} else {
				last += (1 + sprintf(last,"[%p]",array[cnt]));
			}
		}
	}

	return result;
}

void show_programe_maps(void)      
{
	pid_t pid = getpid();          
	char cmd[100] = {0};           
	sprintf(cmd, "cat /proc/%d/maps", (int)pid);
	system(cmd);                   
}

/**
 * ak_backtrace - backtrace current code context when caught appointed signal
 * @signal_no[IN]: signal no.
 * @si[IN]:
 * @ptr[IN]:
 * @return: none
 * notes: handle signal: SIGINT, SIGTERM, SIGSEGV
 */
void ak_backtrace(unsigned int signal_no, siginfo_t *si, void *ptr)
{
	ak_print_notice("\n\t***********************************\n");
	ak_print_notice("\t* prog: %s, signal %d caught\n",
			program_invocation_short_name, signal_no);
	ak_print_notice("\t***********************************\n\n");

	show_programe_maps();

	if ((SIGINT == signal_no) || (SIGTERM == signal_no)) {
 		return;
 	}

	if(ptr) {
		ak_print_notice("unhandled page fault (%d) at: 0x%08x\n",
			si->si_signo, (unsigned int)(si->si_addr));

		struct ucontext *ucontext = (struct ucontext *)ptr;
		int pc = ucontext->uc_mcontext.arm_pc;
		void *pc_array[1];

		pc_array[0] = (void *)pc;
		char **pc_name = backtrace_symbols(pc_array, 1);
		ak_print_notice("%d: %s\n", 0, *pc_name);
		free(pc_name);
		pc_name = NULL;

		void *array[MAX_TRACE_SIZE];
		char **strings = NULL;

		int size = backtrace(array, MAX_TRACE_SIZE);
		if(size > 0){
			strings = backtrace_symbols(array, size);
			if (strings) {
				for(int i=0;  i<size; ++i) {
					ak_print_notice("%d: %s\n",i+1, strings[i]);
				}

				free(strings);
				strings = NULL;
			}
		}
	}

	fflush(stderr);
	fflush(stdout);

	signal(signal_no, SIG_DFL);
	raise(signal_no);
}
#endif

/**
 * ak_print_null: NULL function
 * return: void
 * note: use for the lib print callback function
 */
void ak_print_null(const char *fmt, ...)
{
}

/* use for register call */
void print_normal(const char *fmt, ...)
{
	char buf[LOG_BUF_LEN] = {0};
	va_list arg;

	va_start(arg, fmt);
	vsnprintf(buf, sizeof(buf), fmt, arg);
	va_end(arg);

	ak_print_normal("%s", buf);
}

int ak_print_set_syslog_level(int level)
{
	unsigned char mask[8] = {1, 3, 7, 15, 31, 63, 127, 255};

	if (level > LOG_DEBUG)
		level = LOG_DEBUG;
	if (level < LOG_EMERG)
		level = LOG_EMERG;

	setlogmask(mask[level]);
	ak_print_normal("set log mask to: %d\n", mask[level]);

	return 0;
}

/**
 * ak_set_module_print: set module print flag.
 * @module_name[IN]: print level [0,5]
 * @enable[IN]: 0 disable, 1 enable
 * return: 0 success, -1 failed
 * notes: 1. module name max len is 20 bytes;
 *      2. if the module name is a new one, it'll add to list, and the memory
 *      won't free.
 *      3. if you don't set module print flag, the default action is print.
 */
int ak_set_module_print(const char *module_name, unsigned char enable)
{
    int ret = AK_FAILED;

    if (module_name) {
        struct module_print_info *entry = NULL;
        int name_len = strlen(module_name);

    	list_for_each_entry(entry, &module_head, list) {
    		if (entry
    		    && (0 == memcmp(entry->name, module_name, name_len))) {
    		    entry->enable = enable;
    			ret = AK_SUCCESS;
    			break;
    		}
    	}

        /* do not find in exsited list, add it as new one */
    	if (ret) {
    	    entry = (struct module_print_info *)calloc(1,
    	        sizeof(struct module_print_info));
    	    if (entry) {
    	        entry->enable = enable;
    	        memcpy(entry->name, module_name, name_len);
    	        list_add_tail(&(entry->list), &module_head);
    	        ret = AK_SUCCESS;
    	    }
    	}
    }

	return ret;
}

/**
 * ak_print: print function we defined for debugging
 * @level[IN]: print level [0,5]
 * @fmt[IN]: format like printf()
 * @...[IN]: variable arguments list
 * return: we always return 0.
 */
int ak_print(int level, const char *fmt, ...)
{
	/* do not print and wouldn't write to sys-log */
	if (level > get_cur_log_level())
		return 0;

	char buf[LOG_BUF_LEN] = {0};
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, LOG_BUF_LEN, fmt, args);
	va_end(args);

	if (!check_module_print(buf))
		return 0;

	/* color select and syslog write judge */
	switch (level) {
	case LOG_LEVEL_ERROR:
		/* error print, red color */
		printf(LIGHT_RED"%s"NONE, buf);
		syslog(LOG_ERR, "%s", buf);
		break;
	case LOG_LEVEL_WARNING:
		/* fatal print, yellow color */
		printf(PURPLE"%s"NONE, buf);
		syslog(LOG_EMERG, "%s", buf);
		break;
	case LOG_LEVEL_NOTICE:
		/* fatal print, yellow color */
		printf(YELLOW"%s"NONE, buf);
		syslog(LOG_NOTICE, "%s", buf);
		break;
	case LOG_LEVEL_NORMAL:
		/* normal print, we add no color to it */
		printf("%s", buf);
		syslog(LOG_NOTICE, "%s", buf);
		break;
	case LOG_LEVEL_INFO:
		/* info print, blue color */
		printf(BLUE"%s"NONE, buf);
		syslog(LOG_INFO, "%s", buf);
		break;
	case LOG_LEVEL_DEBUG:
		/* debug print, green color */
		printf(GREEN"%s"NONE, buf);
		syslog(LOG_DEBUG, "%s", buf);
		break;
	default:
		break;
	}

	fflush(stdout);
	fflush(stderr);

	return 0;
}

/**
 * ak_check_file_exist - check appointed path dir/file exist
 * @file[IN]: absolutely path
 * return: 0 on exist, others not
 */
int ak_check_file_exist(const char *path)
{
	return access(path, F_OK);
}

/**
 * ak_is_regular_file - Check whether appointed file is a regular file
 * @file_path[IN]: absolutely file path
 * return: 1 regular file, 0 not regular file, -1 failed
 */
int ak_is_regular_file(const char *file_path)
{
	struct stat sb;
	if (stat(file_path, &sb)) {
		ak_print_warning_ex("stat file: %s error\n", file_path);
		ak_print_warning_ex("errno=%d, desc: %s\n ", errno, strerror(errno));
		return AK_FAILED;
	}

	return S_ISREG(sb.st_mode);
}

/**
 * ak_is_dev_file - Check whether appointed file is a device file
 * @file_path[IN]: absolutely file path
 * return: 1 regular file, 0 not regular file, -1 failed
 */
int ak_is_dev_file(const char *file_path)
{
	struct stat sb;
	if (lstat(file_path, &sb)) {
		return AK_FAILED;
	}
	return ( S_ISBLK(sb.st_mode) || S_ISCHR(sb.st_mode) ) ;
}

/**
 * ak_sleep_ms - sleep certain time
 * @ms[IN]: milli-seconds
 * return: none
 */
void ak_sleep_ms(const int ms)
{
	usleep (ms * 1000);
}

/**
 * ak_get_ostime - get OS time since cpu boot
 * @tv[OUT]: time value since startup
 * return: void
 */
void ak_get_ostime(struct ak_timeval *tv)
{
	if(!tv) {
		return;
	}

	struct timespec start_time;

	clock_gettime(CLOCK_MONOTONIC, &start_time);
	tv->sec = start_time.tv_sec;
	tv->usec = (start_time.tv_nsec / 1000);
}

/**
 * ak_get_localdate - get local date time
 * @date[OUT]: get date time
 * return: 0 success, -1 failed
 * notes:
 */
int ak_get_localdate(struct ak_date *date)
{
	if (!date) {
		return AK_FAILED;
	}

	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);
	struct tm value;
	/* localtime_r is safe for thread */
	localtime_r(&(tv.tv_sec), &value);
	date->year = (DATE_YEAR_DIFF + value.tm_year);
	date->month = value.tm_mon;
	date->day = (value.tm_mday -1);
	date->hour = value.tm_hour;
	date->minute = value.tm_min;
	date->second = value.tm_sec;
	date->timezone = tz.tz_dsttime;

	return AK_SUCCESS;
}

/**
 * ak_set_localdate - set local date time
 * @date[IN]: set date time
 * return: 0 success, -1 failed
 */
int ak_set_localdate(const struct ak_date *date)
{
	if (!date) {
		return AK_FAILED;
	}

	struct tm value;
	struct timeval tv;
	struct timezone tz;

	value.tm_year = (date->year - DATE_YEAR_DIFF);
	value.tm_mon = date->month;
	value.tm_mday = (date->day + 1);
	value.tm_hour = date->hour;
	value.tm_min = date->minute;
	value.tm_sec = date->second;
	tz.tz_dsttime = date->timezone;

	tv.tv_sec = mktime(&value);
	tv.tv_usec = 0;
	settimeofday(&tv, &tz);

	return AK_SUCCESS;
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
	if (!cur_time || !pre_time) {
		return 0;
	}

	struct timeval cur_tv;
	struct timeval pre_tv;
	struct timeval res_tv;

	cur_tv.tv_sec = cur_time->sec;
	cur_tv.tv_usec = cur_time->usec;
	pre_tv.tv_sec = pre_time->sec;
	pre_tv.tv_usec = pre_time->usec;

	timersub(&cur_tv, &pre_tv, &res_tv);

	return ((1000 * res_tv.tv_sec) + (res_tv.tv_usec / 1000));
}

/**
 * ak_seconds_to_string - transfer total seconds to readable time string
 * @secs: passed total seconds from 1970-01-01 00:00:00
 * return: readable time string after transferred
 * notes: string format: yyyy-MM-dd HH:mm:ss, ex: 2016-04-06 10:06:06
 *      time_str min len 20 bytes.
 * IMPORTANT: use ONLY ONCE after call this function.
 */
char* ak_seconds_to_string(time_t secs)
{
	static char time_str[20] = {0};

	struct tm value;
	/* localtime_r is safe for thread */
	localtime_r(&secs, &value);
	sprintf(time_str, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
    	(1900+value.tm_year), (value.tm_mon + 1), value.tm_mday,
    	value.tm_hour, value.tm_min, value.tm_sec);

    return time_str;
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
	if(!date){
        return AK_FAILED;
    }

	struct tm value;
	/* localtime_r is safe for thread */
	localtime_r(&seconds, &value);
	date->year = value.tm_year + DATE_YEAR_DIFF;
    date->month = value.tm_mon;
    date->day = value.tm_mday - 1;
    date->hour = value.tm_hour;
    date->minute = value.tm_min;
    date->second = value.tm_sec;
    date->timezone = 0;

    return AK_SUCCESS;
}

/**
 * ak_date_to_seconds - transfer date time value to seconds
 * @date[IN]: date time value
 * return: seconds after transferred; failed -1
 * notes: seconds from 1970-01-01 00:00:00 +0000(UTC)
 */
long ak_date_to_seconds(const struct ak_date *date)
{
	if(!date){
        return AK_FAILED;
    }

	struct tm tmp;
	time_t utc = 0;

	memset (&tmp, 0 ,sizeof (tmp));
    tmp.tm_year = date->year - DATE_YEAR_DIFF;
    tmp.tm_mon = date->month;
    tmp.tm_mday = date->day + 1;
    tmp.tm_hour = date->hour;
    tmp.tm_min = date->minute;
    tmp.tm_sec = date->second;
    tmp.tm_isdst = 0;
    utc = mktime(&tmp);

    /// 由于这里忽略了夏时制的问题，有可能会出现偏差，因此需要使用本地时再校准一次。
    localtime_r (&utc, &tmp);
    if (!tmp.tm_isdst) {
    	/// 这个时间本地时没有使用夏时制，可以直接返回。
    	return utc;
    }

    tmp.tm_year = date->year - DATE_YEAR_DIFF;
	tmp.tm_mon = date->month;
	tmp.tm_mday = date->day + 1;
	tmp.tm_hour = date->hour;
	tmp.tm_min = date->minute;
	tmp.tm_sec = date->second;
	//tmp.tm_isdst = 1; ///< 保留 tm_isdst 参数不要动，再次生成一次 UTC 时间。

	return mktime(&tmp);
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
    	date->year, (date->month + 1), (date->day + 1),
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
    date->month = month - 1;
    date->day = day - 1;
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

	ak_print_normal(" [%4.4d%2.2d%2.2d-%2.2d%2.2d%2.2d]\n",
    	date->year, (date->month + 1), (date->day + 1),
    	date->hour, date->minute, date->second);

	return AK_SUCCESS;
}

int ak_print_set_level(int level)
{
	int old_level = 0;
	FILE *filp = NULL;
	char buf[2] = {0};

	filp = fopen(AK_PRINT_LEVEL_IDENT, "w+");
	if (filp) {

		fread(buf, 1, sizeof(buf), filp);
		old_level = atoi(buf);

		sprintf(buf, "%d", level);
		fwrite(buf, 1, sizeof(buf), filp);
		fclose(filp);
	}

	if (log_level != level) {
	    log_level = level;
	}

	return old_level;
}
