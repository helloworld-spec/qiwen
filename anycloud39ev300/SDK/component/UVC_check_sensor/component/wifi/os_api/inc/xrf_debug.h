#ifndef _XRF_DEBUG_H
#define _XRF_DEBUG_H

#include "stdio.h"
#include "xrf_type.h"
#include "xrf_timer.h"

extern int dbg_level;

#define SYS_MONITOR    		1		//MONITOR开关
#define RELEASE_VERSION		0		//置1后将关闭所有打印信息


#if RELEASE_VERSION		
#undef DEBUG
#endif

#ifdef DEBUG
#define ERR_PRINT_TIME	printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000)
#define DBG_PRINT_TIME	printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000)
#define p_info(...) do{if(!dbg_level)break;printf("[I: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf(__VA_ARGS__); printf("\r\n");}while(0)
#define p_err(...) do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf(__VA_ARGS__); printf("\r\n");}while(0)
#define p_dbg_track do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s,%d",  __func__ , __LINE__, ); printf("\r\n");}while(0)
#define p_dbg(...) do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf(__VA_ARGS__); printf("\r\n");}while(0)
#define p_dbg_enter do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("enter %s\n", __func__ ); printf("\r\n");}while(0)
#define p_dbg_exit do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("exit %s\n", __func__ ); printf("\r\n");}while(0)
#define p_dbg_status do{if(!dbg_level)break;printf("[D: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("status %d\n", status); printf("\r\n");}while(0)
#define p_err_miss do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s miss\n", __func__ ); printf("\r\n");}while(0)
#define p_err_mem do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s mem err\n", __func__ ); printf("\r\n");}while(0)
#define p_err_fun do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s err in %d\n", __func__ , __LINE__); printf("\r\n");}while(0)
#define dump_hex(tag, buff, size) do{	\
	int dump_hex_i;\
	if(!dbg_level)break;	\
	printf("%s[", tag);\
	for (dump_hex_i = 0; dump_hex_i < size; dump_hex_i++)\
	{\
		printf("%02x ", ((char*)buff)[dump_hex_i]);\
	}\
	printf("]\r\n");\
}while(0)
#else
#define p_info(...) 	do{printf("[I: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000); printf(__VA_ARGS__); printf("\r\n");}while(0)
#define p_err(...) 	do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000); printf(__VA_ARGS__); printf("\r\n");}while(0)
#define p_err_miss do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s miss\n", __func__ ); printf("\r\n");}while(0)
#define p_err_mem do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s mem err\n", __func__ ); printf("\r\n");}while(0)
#define p_err_fun do{printf("[E: %d.%03d] ",  os_time_get()/1000, os_time_get()%1000);printf("%s err in %d\n", __func__ , __LINE__); printf("\r\n");}while(0)

#define p_dbg_track
#define p_dbg(...)
#define p_dbg_exit
#define p_dbg_enter
#define p_dbg_status

#define dump_hex(tag, buff, size)

#endif


#define p_hex(X, Y) dump_hex("", (unsigned char*)X, Y)

#define assert(x)                                                               \
do{                                                                              		 \
if (!(x))                                                                   	\
{                                                                           			\
p_err( "%s:%d assert " #x "failed\r\n", __FILE__, __LINE__);	\
while(1);										\
}                                                                           			\
}while(0)

typedef enum
{
	PRINT_TYPE_D, PRINT_TYPE_H, PRINT_TYPE_P_D, PRINT_TYPE_P_H,
} STAT_PRINT_TYPE;

struct dev_monitor_item
{
	const char *name;
	uint32_t value;
	//  STAT_PRINT_TYPE type;
};

void send_test_pkg(int cmd, const void *data, int size);
void dev_monitor_task(void *arg);
void switch_dbg(void);
void init_monitor(void);

#endif
