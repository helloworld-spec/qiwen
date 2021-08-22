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
#include "main.h"
#include "printcolor.h"
#include "timecount.h"
#include "regexpr.h"
#include "fat_repair.h"
#include "speed_test.h"
#include "card_test.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

char ac_card_status[ ][ LEN_HINT ] = {
	HINT_CARD_GOOD ,
	HINT_CARD_WARNING ,
	HINT_CARD_ERROR ,
};

char ac_test_type[ ][ LEN_HINT ] = {
	HINT_TEST_GLOBAL,
	HINT_TEST_READ,
	HINT_TEST_WRITE,
	HINT_TEST_ERASE,
	HINT_TEST_REWRITE,
};

unsigned char *pc_read_buff= NULL;
unsigned char *pc_read_align= NULL;
unsigned char *pc_erase_buff= NULL;
unsigned char *pc_erase_align= NULL;
unsigned char *pc_write_buff= NULL;
unsigned char *pc_write_align= NULL;
unsigned char *pc_verify_buff= NULL;
unsigned char *pc_verify_align= NULL;

int start_check_disk( )
{
	int i_color, i_hint, i_status, i;
	unsigned long long i_us = 0 , i_us_total = 0, i_sector_now, i_sector_num, i_sector_modify;
	double f_percent_error;
	struct timeval timeval_begin, timeval_end;
	struct timeval timeval_start, timeval_last;                                                     //TF卡坏卡检测的测试时间

	timeval_mark( &timeval_start );

	pc_read_buff = ( unsigned char * )calloc( 1, gi_test_sector * SEC_SIZE + LEN_OFFSET );
	pc_read_align = pc_read_buff + SIZE_SECTOR - ((unsigned long)pc_read_buff & SIZE_SECTOR_ALIGN);

	pc_erase_buff = ( unsigned char * )calloc( 1, SEC_SIZE + LEN_OFFSET );                          //分配一个扇区长度,擦除后仅对第一个扇区进行分析
	pc_erase_align = pc_erase_buff + SIZE_SECTOR - ((unsigned long)pc_erase_buff & SIZE_SECTOR_ALIGN);

	if( gc_write_check == AK_TRUE ) {
		pc_write_buff = ( unsigned char * )calloc( 1, gi_test_sector * SEC_SIZE + LEN_OFFSET );
		pc_write_align = pc_write_buff + SIZE_SECTOR - ((unsigned long)pc_write_buff & SIZE_SECTOR_ALIGN);
		fill_random_bytes( pc_write_buff, SEC_SIZE + LEN_OFFSET );

		pc_verify_buff= ( unsigned char * )calloc( 1, gi_test_sector * SEC_SIZE + LEN_OFFSET );
		pc_verify_align= pc_verify_buff + SIZE_SECTOR - ((unsigned long)pc_verify_buff & SIZE_SECTOR_ALIGN);
	}

	timeval_mark( &timeval_begin );

	i_sector_now = get_sector_offset( );                                                            //获取扇区开始的偏移量
	//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE, "i_sector_now= %llu", i_sector_now )

	while ( ( gc_prog_run == AK_TRUE ) && ( i_sector_now < g_card_result.i_total_sector ) ) {
		if ( ( gc_sector_align == AK_TRUE ) && ( gc_full_test != AK_TRUE ) ) {                      //进行扇区1MB对齐
			if ( ( i_sector_modify = get_sector_align( i_sector_now ) ) >= g_card_result.i_total_sector ) {//获取对齐的扇区号
				break;
			}
			//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE, "i_sector_now= %llu i_sector_modify= %llu", i_sector_now , i_sector_modify )
		}
		else {                                                                                      //不进行扇区对齐
			i_sector_modify = i_sector_now;
		}

		if ( i_sector_modify + gi_test_sector < g_card_result.i_total_sector ) {                    //获取当前可以测试的sector数量
			i_sector_num = gi_test_sector;
		}
		else {
			i_sector_num = g_card_result.i_total_sector - i_sector_modify;
		}

		g_card_result.ai_card_test[ CARD_TEST_TYPE_GLOBAL ] ++;
		if ( test_sector( i_sector_modify, i_sector_num) == AK_FAILED ) {                           //***** 对tf卡进行检测的入口函数 *****
			g_card_result.ai_card_error[ CARD_TEST_TYPE_GLOBAL ] ++;
			i_color = COLOR_FRONT_RED;
		}
		else {
			i_color = COLOR_FRONT_GREEN;
		}

		i_us_total = timeval_count( &timeval_start, timeval_mark( &timeval_last ) ) ;               //目前总体耗时

		if ( ( gi_test_timeout > 0 ) && ( i_us_total >= gi_test_timeout * SEC2USEC ) ) {            //超过检测时间退出
			if ( gc_view_detail == AK_TRUE ) {
				DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
				           "[%llu] %s (%llums)\n",
				           i_sector_now, HINT_TEST_TIMEOUT , i_us_total / MSEC2USEC )
			}
			if( i_color == COLOR_FRONT_GREEN ) {
				i_color = COLOR_FRONT_RED;
				//g_card_result.ai_card_error[ CARD_TEST_TYPE_GLOBAL ] ++;
			}
			break;
		}
		if ( ( i_us = timeval_count( &timeval_begin, timeval_mark( &timeval_end ) ) ) <= 0 ) {       //当前测试耗时
			timeval_begin = timeval_end;
			continue ;
		}

		if( gc_full_test == AK_TRUE ) {                                                             //全卡扫描
			if ( ( i_sector_now + i_sector_num ) % COUNT_SECTOR == 0 ) {                            //扫描COUNT_SECTOR的扇区后才打印相应的信息
				if ( gc_view_detail == AK_TRUE ) {
					DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color, "[%llu] %.2f KB %.2f ms %.2f M/s %.2f%%\n",
					           i_sector_modify,
					           ( double )COUNT_SECTOR * SEC_SIZE / 1024,
					           ( double )i_us / MSEC2USEC,
					           ( double )COUNT_SECTOR * SEC_SIZE / BYTE2MB / i_us * SEC2USEC,
					           ( double )i_sector_now / g_card_result.i_total_sector * 100 )
				}
				timeval_begin = timeval_end;
			}
			i_sector_now += i_sector_num;
		}
		else if ( gc_per_mb == AK_TRUE ) {                                                          //根据容量取点进行扫描
			if ( i_us >= gi_test_point_timeout * MSEC2USEC ) {
				if ( gc_view_detail == AK_TRUE ) {
					DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
					           "[%llu] %s (%llums)\n",
					           i_sector_modify, HINT_TEST_TIMEOUT , i_us / MSEC2USEC )
				}
				/*
				if( i_color == COLOR_FRONT_GREEN ) {
					i_color = COLOR_FRONT_RED;
					g_card_result.ai_card_error[ CARD_TEST_TYPE_GLOBAL ] ++;
				}
				if( gc_erase == AK_TRUE ) {
					gc_erase = AK_FALSE;
				}
				else {
					break;
				}
				*/
			}
			if ( gc_view_detail == AK_TRUE ) {
				DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color, "[%llu] %.2f KB %.2f ms %.2f M/s %.2f%%\n",
					       i_sector_modify,
					       ( double )i_sector_num * SEC_SIZE / 1024,
					       ( double )i_us / MSEC2USEC,
					       ( double )i_sector_num * SEC_SIZE / BYTE2MB / i_us * SEC2USEC,
					       ( double )i_sector_modify / g_card_result.i_total_sector * 100 )
			}
			timeval_begin = timeval_end;
			i_sector_now += gi_test_per_mb * SECTOR_PER_MB;
		}
		else {                                                                                      //根据设定采集点数进行扫描(默认模式)
			if ( i_us >= gi_test_point_timeout * MSEC2USEC ) {
				if ( gc_view_detail == AK_TRUE ) {
					DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
					           "[%llu] %s (%llums)\n",
					           i_sector_modify, HINT_TEST_TIMEOUT , i_us / MSEC2USEC )
				}
				/*
				if( i_color == COLOR_FRONT_GREEN ) {
					i_color = COLOR_FRONT_RED;
					g_card_result.ai_card_error[ CARD_TEST_TYPE_GLOBAL ] ++;
				}

				if( gc_erase == AK_TRUE ) {
					gc_erase = AK_FALSE;
					if ( gc_view_detail == AK_TRUE ) {
						DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
						           "[%llu] %s\n", i_sector_now, HINT_NO_ERASE )
					}
				}
				else {
					break;
				}
				*/
			}
			if ( gc_view_detail == AK_TRUE ) {
				DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color, "[%llu] %.2f KB %.2f ms %.2f M/s %.2f%%\n",
					       i_sector_modify,
					       ( double )i_sector_num * SEC_SIZE / 1024,
					       ( double )i_us / MSEC2USEC,
					       ( double )i_sector_num * SEC_SIZE / BYTE2MB / i_us * SEC2USEC,
					       ( double )i_sector_modify / g_card_result.i_total_sector * 100 )
			}
			timeval_begin = timeval_end;
			i_sector_now += ( g_card_result.i_total_sector - gi_test_offset ) / gi_test_num;
		}
		if ( gi_test_sleep > 0 ) {
			sleep_ms( gi_test_sleep ) ;
		}
	}
	f_percent_error = ( double )g_card_result.ai_card_error[ CARD_TEST_TYPE_GLOBAL ]  / g_card_result.ai_card_test[ CARD_TEST_TYPE_GLOBAL ] * 100;
	if ( f_percent_error <= PERCENT_GOOD ) {
		i_color = COLOR_FRONT_GREEN;
		i_hint = CARD_STATUS_GOOD;
		i_status = RESULT_STATUS_PASS;
	}
	else if ( f_percent_error <= PERCENT_WARNING ) {
		i_color = COLOR_FRONT_RED;
		i_hint = CARD_STATUS_WARNING;
		i_status = RESULT_STATUS_FAIL;
	}
	else {
		i_color = COLOR_FRONT_RED;
		i_hint = CARD_STATUS_ERROR;
		i_status = RESULT_STATUS_FAIL;
	}
	if ( ( g_card_result.ai_status_us[ TEST_STATUS_CARD ] = timeval_count( &timeval_start, timeval_mark( &timeval_last ) ) ) > 0 ) {
	}
	if( gc_key_value_res == AK_FALSE ) {
		DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color,
		           "###########################################\n"
		           "CARD %s SEC= %.2f %s\n",
		           gac_hint_status[ i_status ], ( double )g_card_result.ai_status_us[ TEST_STATUS_CARD] / SEC2USEC, ac_card_status[ i_hint ] )
	}

	for( i= 0 ; i < CARD_TEST_TYPE_NUM; i ++ ) {
		if ( g_card_result.ai_card_test[ i ] != 0 ) {
			f_percent_error = ( double )g_card_result.ai_card_error[ i ]  / g_card_result.ai_card_test[ i ] * 100;
		}
		else {
			f_percent_error = 0;
		}
		g_card_result.af_card_percent[ i ] = f_percent_error ;
		if( gc_key_value_res == AK_FALSE ) {
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color,
			           "[%s]\t %d\t ERROR %d\t  ERROR_PERCENT %.2f%%\n" ,
			           ac_test_type[ i ], g_card_result.ai_card_test[ i ], g_card_result.ai_card_error[ i ] , f_percent_error )
		}
	}
	if( gc_key_value_res == AK_FALSE ) {
		DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color,
		           "###########################################\n" )
	}
	FREE_POINT( pc_read_buff );
	FREE_POINT( pc_erase_buff );
	FREE_POINT( pc_write_buff );
	FREE_POINT( pc_verify_buff );
	return i_status;
}

unsigned long long get_sector_offset( )                                                             //开始测试时候,设定开始检测的扇区号
{
	double f_rand;
	int i_sector_range ;

	srand( ( unsigned int )time( 0 ) );

	if( gc_full_test == AK_TRUE ) {                                                                 //根据偏移值设置开始检测的扇区
		return 0 ;                                                                                  //全卡扫描则从开始块开始
	}
	else if ( gc_per_mb == AK_TRUE ) {                                                              //根据容量扫描
		if ( gi_test_offset < 0 ) {                                                                 //随机设定一个扇区偏移值
			f_rand = rand( ) / ( double )RAND_MAX ;
			i_sector_range = gi_test_per_mb * SECTOR_PER_MB - gi_test_sector + 1 ;
			//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE, "SECTOR_NUM= %d RAND= %f", i_sector_range , f_rand )
			return ( unsigned long long )( i_sector_range * f_rand );
		}
		else {
			return gi_test_offset ;
		}
	}
	else {                                                                                          //固定采样点测试
		if ( gi_test_offset < 0 ) {                                                                 //随机设定一个扇区偏移值
			f_rand = rand( ) / ( double )RAND_MAX ;
			i_sector_range = g_card_result.i_total_sector / gi_test_num - gi_test_sector + 1 ;
			DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE, "SECTOR_NUM= %d RAND= %f", i_sector_range , f_rand )
			return ( unsigned long long )( i_sector_range * f_rand );
		}
		else {
			return gi_test_offset ;
		}
	}
}

int fill_random_bytes( unsigned char *pc_buff , int i_len )
{
	int i;
	for( i = 0 ; i < i_len ; i ++ ) {
		pc_buff[ i ] = rand( ) % NUM_ASCII ;
		//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE, "pc_buff[ %d ]= %d", i , pc_buff[ i ] )
	}
	return 0;
}


int test_sector( off64_t i_sector, off64_t i_num )                                                  //***** 对tf卡进行检测的入口函数 *****
{
	int i;
	int c_flag = AK_SUCCESS;

	if ( gc_read == AK_TRUE ){
		g_card_result.ai_card_test[ CARD_TEST_TYPE_READ ] ++;
		if ( read_disk(gi_fd_dev, i_sector * SEC_SIZE, pc_read_align, i_num * SEC_SIZE) <= 0 ) {
			g_card_result.ai_card_error[ CARD_TEST_TYPE_READ ] ++;
			return AK_FAILED;
		}
		/*
		for( i = 0 ; i < i_num * SEC_SIZE ; i ++ ) {
			DEBUG_VAL(COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED, "%02x", pc_verify_align[ i ] )
			if ( ( i + 1 ) % 64 == 0 ) {
				printf( "\n" );
			}
		}
		*/
	}

	if ( ( gc_write == AK_TRUE ) &&
		 ( gc_write_check == AK_TRUE ) ) {                                                          //写入随机数据检查是否真实可写
		g_card_result.ai_card_test[ CARD_TEST_TYPE_WRITE ] ++;
		if ( ( write_disk( gi_fd_dev, i_sector * SEC_SIZE, pc_write_align, i_num * SEC_SIZE) <= 0 ) ||
		     ( read_disk( gi_fd_dev, i_sector * SEC_SIZE, pc_verify_align, i_num * SEC_SIZE) <= 0 ) ||
		     ( memcmp( pc_write_align , pc_verify_align , i_num * SEC_SIZE ) != 0 ) ) {
			if ( gc_view_detail == AK_TRUE ) {
				DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
				           "[WRITE:RANDOM] i_sector= %llu strerror(%d)= '%s'\n",
				           i_sector, errno, strerror(errno))
			}
			g_card_result.ai_card_error[ CARD_TEST_TYPE_WRITE ] ++;
			return AK_FAILED;
		}
		/*
		for( i = 0 ; i < i_num * SEC_SIZE ; i ++ ) {
			DEBUG_VAL(COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN, "%02x", pc_write_align[ i ] )
			if ( ( i + 1 ) % 64 == 0 ) {
				printf( "\n" );
			}
		}
		for( i = 0 ; i < i_num * SEC_SIZE ; i ++ ) {
			DEBUG_VAL(COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "%02x", pc_verify_align[ i ] )
			if ( ( i + 1 ) % 64 == 0 ) {
				printf( "\n" );
			}
		}
		*/
	}

	if ( gc_erase == AK_TRUE ) {
		g_card_result.ai_card_test[ CARD_TEST_TYPE_ERASE ] ++;
		if ( erase_disk( gi_fd_dev, i_sector * SEC_SIZE, i_num * SEC_SIZE) == AK_FAILED ) {         //擦写
			g_card_result.ai_card_error[ CARD_TEST_TYPE_ERASE ] ++;
			c_flag = AK_FAILED;
		}
		else if ( gc_erase_check == AK_TRUE ) {                                                     //擦写检查,判断是否进行擦除后的检测
			if( read_disk( gi_fd_dev, i_sector * SEC_SIZE, pc_erase_align, SEC_SIZE ) <= 0 ) {      //仅仅对一个扇区进行读取
				g_card_result.ai_card_error[ CARD_TEST_TYPE_ERASE ] ++;
				c_flag = AK_FAILED;
			}
			else {
				for( i = 0 ; i < SIZE_SECTOR ; i ++ ){
					if ( ( pc_erase_align[ i ] != 0xff ) && ( pc_erase_align[i] != 0x00 ) ) {
						if ( gc_view_detail == AK_TRUE ) {
							DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
							           "[ERASE] strerror(%d)= '%s' pc_erase_align[%d] = %02x\n",
							           errno, strerror(errno) , i, pc_erase_align[i]);
						}
						g_card_result.ai_card_error[ CARD_TEST_TYPE_ERASE ] ++;
						c_flag = AK_FAILED;
						break;
					}
				}
			}
		}
	}


	if ( gc_write == AK_TRUE ) {
		g_card_result.ai_card_test[ CARD_TEST_TYPE_REWRITE ] ++;
		if ( write_disk(gi_fd_dev, i_sector * SEC_SIZE, pc_read_align, i_num * SEC_SIZE) <= 0 ) {   //回写数据
			g_card_result.ai_card_error[ CARD_TEST_TYPE_REWRITE ] ++;
			return AK_FAILED;
		}
		else if ( gc_write_check == AK_TRUE ){                                                      //回写检查
			if ( ( read_disk( gi_fd_dev, i_sector * SEC_SIZE, pc_verify_align, i_num * SEC_SIZE) <= 0 ) ||
			     ( memcmp( pc_read_align , pc_verify_align , i_num * SEC_SIZE ) != 0 ) ) {
				if ( gc_view_detail == AK_TRUE ) {
					DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
					           "[WRITE:BACKUP] i_sector= %llu strerror(%d)= '%s'\n", i_sector, errno, strerror(errno))
				}
				g_card_result.ai_card_error[ CARD_TEST_TYPE_REWRITE ] ++;
				return AK_FAILED;
			}
		}
	}

	return c_flag;
}

int write_disk(int fd, off64_t offset, unsigned char *buf, int len)
{
	int ret=-1;
	//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE,"[READ] fd= %d offset= %llu buf= %x len= %d\n", fd, offset, buf,len)
	//seek
	if (lseek64(fd, offset, SEEK_SET) < 0) {
		if ( gc_view_detail == AK_TRUE ) {
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,"[WRITE] sector= %llu offset= %llu strerror(%d)= '%s'\n", offset / SIZE_SECTOR , offset, errno, strerror(errno))
		}
		return -1;
	}

	//write data
	if ((ret = write(fd, buf, len)) < 0) {
		if ( gc_view_detail == AK_TRUE ) {
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,"[WRITE] sector= %llu offset= %llu strerror(%d)= '%s'\n", offset / SIZE_SECTOR , offset, errno, strerror(errno))
		}
		return -1;
	}

	return ret;
}

int read_disk(int fd, off64_t offset, unsigned char *buf, int len)
{
	int ret=-1;

	//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE,"[READ] fd= %d offset= %llu buf= %x len= %d\n", fd, offset, buf,len)
	//seek
	if (lseek64(fd, offset, SEEK_SET) < 0) {
		if ( gc_view_detail == AK_TRUE ) {
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,"[READ] sector= %llu offset= %llu strerror(%d)= '%s'\n", offset / SIZE_SECTOR , offset, errno, strerror(errno))
		}
		return -1;
	}

	//read data
	if ((ret = read(fd, buf, len)) < 0) {
		if ( gc_view_detail == AK_TRUE ) {
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,"[READ] sector= %llu offset= %llu strerror(%d)= '%s'\n", offset / SIZE_SECTOR , offset, errno, strerror(errno))
		}
		return -1;
	}

	return ret;
}

int erase_disk(int fd, off64_t offset, off64_t len)
{
	int ret = -1;
	off64_t range[2];

	range[0] = offset;
	range[1] = len;

	if ((ret = ioctl(fd, BLKDISCARD, range)) < 0) {
		if ( gc_view_detail == AK_TRUE ) {
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,"[ERASE] sector= %llu offset= %llu strerror(%d)= '%s'\n", offset / SIZE_SECTOR , offset, errno, strerror(errno))
		}
		return AK_FAILED;
	}
	return ret;
}

void sleep_ms(int ms)
{
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = (ms * 1000);
	select(0, NULL, NULL, NULL, &tv);
}

unsigned long long get_sector_align( int i_sector )
{
	unsigned long long i_multiple;

	if ( ( i_sector % TEST_ALIGN ) == 0 ) {
		return i_sector;
	}
	else {
		i_multiple = ( i_sector + TEST_ALIGN ) / TEST_ALIGN ;
		return i_multiple * TEST_ALIGN ;
	}
}

int is_dev_file(const char *file_path)
{
	struct stat stat_file;
	//if (lstat(file_path, &stat_file)) {
	if (stat(file_path, &stat_file)) {
		return AK_FALSE;
	}
	//return ( S_ISBLK(stat_file.st_mode) || S_ISCHR(stat_file.st_mode) ) ;
	//DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE, "S_ISBLK(stat_file.st_mode)= %d", S_ISBLK(stat_file.st_mode) )
	if( S_ISBLK(stat_file.st_mode) || S_ISCHR(stat_file.st_mode) ) {
		return AK_TRUE;
	}
	else {
		return AK_FALSE;
	}
}

int ak_format_tf( )                                                                                 //格式化T卡
{
	int i_color= COLOR_FRONT_RED;
	int i_status = RESULT_STATUS_FAIL;
	int i_res = FORMAT_STATUS_FAIL;
	struct timeval timeval_begin, timeval_end ;
	ULL i_us = 0;
	int i_size = NUM_FORMAT_SECTOR * SEC_SIZE ;
	//int i_data;
	timeval_mark( &timeval_begin );
	//char ac_fdisk[ LEN_SHELL_RES ];
	char ac_cmd_mkfs[ LEN_CMD ];
	char c_mkfs = AK_FALSE;
	//int i;
	unsigned char *pc_bak_buff = NULL, *pc_bak_align = NULL;

	pc_read_buff = ( unsigned char * )calloc( 1, i_size + LEN_OFFSET );
	pc_read_align = pc_read_buff + SIZE_SECTOR - ( ( unsigned long )pc_read_buff & SIZE_SECTOR_ALIGN);

	pc_write_buff = ( unsigned char * )calloc( 1, i_size + LEN_OFFSET );
	pc_write_align = pc_write_buff + SIZE_SECTOR - ( ( unsigned long )pc_write_buff & SIZE_SECTOR_ALIGN);

	pc_bak_buff = ( unsigned char * )calloc( 1, i_size + LEN_OFFSET );
	pc_bak_align = pc_bak_buff + SIZE_SECTOR - ( ( unsigned long )pc_bak_buff & SIZE_SECTOR_ALIGN);

	fill_random_bytes( pc_write_align, i_size );

	/*
	DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_GREEN, "gi_fd_dev= %d" , gi_fd_dev )
	i_data = write_disk( gi_fd_dev, 0, pc_write_align, i_size );
	DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_GREEN, "i_data= %d" , i_data )
	i_data = read_disk( gi_fd_dev, 0, pc_read_align, i_size ) ;
	DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_GREEN, "i_data= %d" , i_data )
	*/


	if ( ( read_disk( gi_fd_dev, 0, pc_bak_align , i_size ) != i_size ) ||
		 ( write_disk( gi_fd_dev, 0, pc_write_align, i_size ) != i_size ) ||
	     ( read_disk( gi_fd_dev, 0, pc_read_align, i_size ) != i_size ) ||
	     ( memcmp( pc_write_align , pc_read_align , i_size ) != 0 ) ) {                             //写入随机数据判断是否写入正常
		DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
		           "[WRITE_PARTITION_TABLE:RANDOM] strerror(%d)= '%s'\n",
		           errno, strerror(errno))
		goto print_format_res;
	}

#if 0
	if (is_dev_file(DEV_MMCBLK0P1) != AK_TRUE) {                                                    //判断是否需要重新分区
		DEBUG_HINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE )
		memset( pc_write_align , 0 , i_size );
		if ( ( write_disk( gi_fd_dev, 0, pc_write_align, i_size ) != i_size ) ||
		     ( read_disk( gi_fd_dev, 0, pc_read_align, i_size ) != i_size ) ||
		     ( memcmp( pc_write_align , pc_read_align , i_size ) != 0 ) ) {                         //写入全0数据判断是否写入正常(清空分区表)
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
			           "[WRITE_PARTITION_TABLE:ZERO] strerror(%d)= '%s'\n",
			           errno, strerror(errno))
			goto print_format_res;
		}
		DEBUG_HINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE )
		get_shell_res( CMD_FDISK , ac_fdisk , LEN_SHELL_RES );                                      //重新分区,仅仅建立一个分区
		ioctl( gi_fd_dev, BLKRRPART, NULL );

		for( i = 0 ; i < SEC_WAIT_PT_FRESH ; i ++ ) {                                               //尝试读取p1分区
			sleep( 1 );
			sync();
			ioctl( gi_fd_dev, BLKRRPART, NULL );
			if ( is_dev_file( DEV_MMCBLK0P1 ) == AK_TRUE ) {
				c_mkfs = AK_TRUE;
				break;
			}
		}
		if ( c_mkfs != AK_TRUE ) {
			goto print_format_res;
		}
		/*
		i_data = read_disk( gi_fd_dev, 0, pc_read_align, i_size ) ;
		DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_GREEN, "i_data= %d i_size= %d" , i_data , i_size )
		i_data = memcmp( pc_write_align , pc_read_align, i_size );
		DEBUG_PRINT( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_GREEN, "i_data= %d i_size= %d" , i_data , i_size )
		*/
		if ( ( read_disk( gi_fd_dev, 0, pc_read_align, i_size ) != i_size ) ||
		     ( memcmp( pc_write_align , pc_read_align, i_size ) == 0 ) ) {                          //写入分区表后和全0缓冲区比较,如果比较!=0判断是否写入成功
			c_mkfs = AK_FALSE;
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
			           "[FDISK:MKFS] strerror(%d)= '%s'\n",
			           errno, strerror(errno))
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
		               "###########################################\n"
		               "READ\n"
		               "###########################################\n" )
			print_hex( pc_read_align , i_size );
			DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_RED,
			           "###########################################\n"
		               "WRITE\n"
		               "###########################################\n" )
			print_hex( pc_write_align , i_size );
			goto print_format_res;
		}
		/*
		DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE,
	               "###########################################\n"
	               "READ\n"
	               "###########################################\n" )
		print_hex( pc_read_align , i_size );
		DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE,
		           "###########################################\n"
	               "WRITE\n"
	               "###########################################\n" )
		print_hex( pc_write_align , i_size );
		*/
	}
#endif

	if ( ( write_disk( gi_fd_dev, 0, pc_bak_align, i_size ) == i_size ) &&
		     ( read_disk( gi_fd_dev, 0, pc_read_align, i_size ) == i_size ) &&
		     ( memcmp( pc_bak_align , pc_read_align , i_size ) == 0 ) ) {                           //回写备份分区表判断是否写入成功
		c_mkfs = AK_TRUE;
	}

	if ( c_mkfs == AK_TRUE ) {                                                                      //关闭mmcblk0的描述符,并创建文件系统
		close( gi_fd_dev );
		gi_fd_dev = 0;
		snprintf( ac_cmd_mkfs , LEN_CMD , CMD_MKFS , gac_mount_dev );
		DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, COLOR_FRONT_BLUE, "%s\n", ac_cmd_mkfs )
		system( ac_cmd_mkfs );
	}

	i_color= COLOR_FRONT_GREEN;
	i_status = RESULT_STATUS_PASS;
	i_res = FORMAT_STATUS_SUCCESS;

print_format_res:
	//ioctl( gi_fd_dev, BLKRRPART, NULL );
	//system( CMD_FDISK_FRESH );
	FREE_POINT( pc_read_buff );
	FREE_POINT( pc_write_buff );
	FREE_POINT( pc_bak_buff );
	if ( ( i_us = timeval_count( &timeval_begin, timeval_mark( &timeval_end ) ) ) > 0 ) {
	}
	DEBUG_VAL( COLOR_MODE_NORMAL, COLOR_BACK_BLACK, i_color,
	           "###########################################\n"
	           "FORMAT %s\n"
	           "SEC= %.2f\n"
	           "###########################################\n",
	           gac_hint_status[ i_res ],
	           ( double )i_us / SEC2USEC );

	return i_status;
}

int print_hex( unsigned char *pc_data , int i_len )
{
	int i;
	for( i = 0 ; i < i_len ; i ++ ) {
		DEBUG_VAL(COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE, "%02x", pc_data[ i ] )
		if ( ( i + 1 ) % HEX_OUTPUT_VIEW == 0 ) {
			printf( "\n" );
		}
	}
	printf( "\n" );
	return 0;
}