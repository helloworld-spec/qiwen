#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <signal.h>
#include <regex.h>

#include "global_val.h"
#include "printcolor.h"
#include "timecount.h"
#include "speed_test.h"
#include "card_test.h"
#include "main.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

unsigned char *gpc_buff_speed, *gpc_addr_speed;
unsigned long long i_us_total_read, i_us_total_write, i_sector_total;

int start_speed_test( )
{
	int i , i_color, i_status ;
	unsigned long long i_sector_now, i_times_test = 0, i_times_error = 0, i_sector_num;
	double f_percent_error;
	struct timeval timeval_start, timeval_last;                                                     //TF卡坏卡检测的测试时间

	timeval_mark( &timeval_start );

	gpc_buff_speed = ( unsigned char * )calloc( 1, gi_speed_sector * SEC_SIZE + LEN_OFFSET );
	gpc_addr_speed = gpc_buff_speed + SIZE_SECTOR - ((unsigned long)gpc_buff_speed & SIZE_SECTOR_ALIGN); //获取一个取整地址的指针地址

	i_sector_now = gi_speed_offset;                                                                 //获取扇区开始的偏移量
	for( i = 0 ; i < g_card_result.ai_status_point[ TEST_STATUS_SPEED ] ; i ++ ) {
		if( ( gc_prog_run != AK_TRUE ) || ( i_sector_now >= g_card_result.i_total_sector ) ) {
			break;
		}
		if ( i_sector_now + gi_speed_sector < g_card_result.i_total_sector ) {                      //设置当前可以读取的sector数量
			i_sector_num = gi_speed_sector;
		}
		else {
			i_sector_num = g_card_result.i_total_sector - i_sector_now;
		}
		i_times_test ++;
		if ( speed_sector( i_sector_now, i_sector_num) == AK_FAILED ) {                             //***** 对tf卡进行速度检测检测的入口函数 *****
			i_times_error ++;
		}
		i_sector_now += gi_speed_per_mb * SECTOR_PER_MB;
	}

	f_percent_error = ( double )i_times_error  / i_times_test * 100;
	if ( ( g_card_result.ai_status_us[ TEST_STATUS_SPEED ] = timeval_count( &timeval_start, timeval_mark( &timeval_last ) ) ) > 0 ) {
	}
	g_card_result.af_speed_read[ SPEED_TYPE_AVG ] = ( double )i_sector_total * SEC_SIZE / BYTE2MB / i_us_total_read * SEC2USEC;
	g_card_result.af_speed_write[ SPEED_TYPE_AVG ] = ( double )i_sector_total * SEC_SIZE / BYTE2MB / i_us_total_write * SEC2USEC;
	if ( ( g_card_result.af_speed_write[ SPEED_TYPE_MAX ] >= gf_speed_require ) && ( f_percent_error <= PERCENT_GOOD ) ) {
		i_color = COLOR_FRONT_GREEN;
		i_status = RESULT_STATUS_PASS;
	}
	else {
		i_color = COLOR_FRONT_RED;
		i_status = RESULT_STATUS_FAIL;
	}
	g_card_result.f_speed_size = ( double )i_sector_total * SEC_SIZE / BYTE2MB ;
	g_card_result.f_speed_percent = f_percent_error;
	if( gc_key_value_res == AK_FALSE ) {
		DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color,
		           "###########################################\n"
		           "SPEED %s\n"
		           "TEST= %llu SEC= %.2f ERROR_PERCENT= %.2f%%\n"
		           "SIZE= %.2fM\n"
		           "READ  (AVG MIN MAX) %.2fM/s %.2fM/s %.2fM/s\n"
		           "WRITE (AVG MIN MAX) %.2fM/s %.2fM/s %.2fM/s\n"
		           "###########################################\n",
		           gac_hint_status[ i_status ],
		           i_times_test, ( double )g_card_result.ai_status_us[ TEST_STATUS_SPEED ] / SEC2USEC, f_percent_error,
		           g_card_result.f_speed_size,
		           g_card_result.af_speed_read[ SPEED_TYPE_AVG ], g_card_result.af_speed_read[ SPEED_TYPE_MIN ], g_card_result.af_speed_read[ SPEED_TYPE_MAX ],
		           g_card_result.af_speed_write[ SPEED_TYPE_AVG ], g_card_result.af_speed_write[ SPEED_TYPE_MIN ], g_card_result.af_speed_write[ SPEED_TYPE_MAX ] )
	}
	FREE_POINT( gpc_buff_speed );

	return i_status;
}

int speed_sector( off64_t i_sector, off64_t i_num )
{
	int i_color = COLOR_FRONT_GREEN;
	struct timeval timeval_begin, timeval_end;
	unsigned long long i_us_read, i_us_write;
	double f_speed;

	timeval_mark( &timeval_begin );
	if ( read_disk(gi_fd_dev, i_sector * SEC_SIZE, gpc_addr_speed, i_num * SEC_SIZE) <= 0) {
		i_color = COLOR_FRONT_RED;
	}
	if ( ( i_us_read = timeval_count( &timeval_begin, timeval_mark( &timeval_end ) ) ) > 0 ) {
		f_speed = ( double )i_num * SEC_SIZE / BYTE2MB / i_us_read * SEC2USEC;
		if( ( g_card_result.af_speed_read[ SPEED_TYPE_MIN ] == 0 ) || ( f_speed < g_card_result.af_speed_read[ SPEED_TYPE_MIN ] ) ) {
			g_card_result.af_speed_read[ SPEED_TYPE_MIN ] = f_speed;
		}
		if( ( g_card_result.af_speed_read[ SPEED_TYPE_MAX ] == 0 ) || ( f_speed > g_card_result.af_speed_read[ SPEED_TYPE_MAX ] ) ) {
			g_card_result.af_speed_read[ SPEED_TYPE_MAX ] = f_speed;
		}
		if ( gc_view_detail == AK_TRUE ) {
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color, "[READ] - [%llu] %.2f KB %.2f ms %.2f M/s %.2f%%\n",
				       i_sector,
				       ( double )i_num * SEC_SIZE / 1024,
				       ( double )i_us_read / MSEC2USEC,
				       f_speed,
				       ( double )i_sector / g_card_result.i_total_sector * 100 )
		}
	}
	timeval_begin = timeval_end;
	if ( write_disk(gi_fd_dev, i_sector * SEC_SIZE, gpc_addr_speed, i_num * SEC_SIZE) <= 0 ) {
		i_color = COLOR_FRONT_RED;
	}

	if ( ( i_us_write = timeval_count( &timeval_begin, timeval_mark( &timeval_end ) ) ) > 0 ) {
		f_speed = ( double )i_num * SEC_SIZE / BYTE2MB / i_us_write * SEC2USEC;
		if( ( g_card_result.af_speed_write[ SPEED_TYPE_MIN ] == 0 ) || ( f_speed < g_card_result.af_speed_write[ SPEED_TYPE_MIN ] ) ) {
			g_card_result.af_speed_write[ SPEED_TYPE_MIN ] = f_speed;
		}
		if( ( g_card_result.af_speed_write[ SPEED_TYPE_MAX ] == 0 ) || ( f_speed > g_card_result.af_speed_write[ SPEED_TYPE_MAX ] ) ) {
			g_card_result.af_speed_write[ SPEED_TYPE_MAX ] = f_speed;
		}
		if ( gc_view_detail == AK_TRUE ) {
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color, "[WRITE] - [%llu] %.2f KB %.2f ms %.2f M/s %.2f%%\n",
				       i_sector,
				       ( double )i_num * SEC_SIZE / 1024,
				       ( double )i_us_write / MSEC2USEC,
				       f_speed,
				       ( double )i_sector / g_card_result.i_total_sector * 100 )
		}
	}
	if ( i_color == COLOR_FRONT_RED ) {
		return AK_FAILED;
	}
	else {
		i_sector_total += i_num;
		i_us_total_read += i_us_read;
		i_us_total_write += i_us_write;
		return AK_SUCCESS;
	}
}