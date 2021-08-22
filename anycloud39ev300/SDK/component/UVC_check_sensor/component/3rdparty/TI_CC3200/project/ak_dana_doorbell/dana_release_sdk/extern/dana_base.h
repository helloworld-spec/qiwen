#ifndef __DANA_BASE_H__
#define __DANA_BASE_H__

// 不同平台定义
// __WIN32__
// __UCOSII__
// __LITEOS__
// __LINUX__

#include <stdint.h>
#include <stdbool.h>

#ifdef __WIN32__
#define DANAVIDEO_LIBSPEC _declspec(dllexport)
#define INLINE __inline
#define __ALIGN__(x)
#define __PACKED__

#if _MSC_VER
#define snprintf _snprintf //vs标准c/c++没有snprintf 但有_snprintf 同时需要注意最后一个\0的问题(vs不会自动添加)
#endif



#elifdef __UCOSII__
#define DANAVIDEO_LIBSPEC   __attribute__ ((visibility("default")))
#define __ALIGN__(x)        __attribute__((aligned(x)))
#define __PACKED__          __attribute__((__packed__))
#define INLINE 


#elif defined __LITEOS__
#define DANAVIDEO_LIBSPEC   __attribute__ ((visibility("default")))
#define __ALIGN__(x)        __attribute__((aligned(x)))
#define __PACKED__          __attribute__((__packed__))
#define INLINE              inline


#else
#define DANAVIDEO_LIBSPEC   __attribute__ ((visibility("default")))
#define __ALIGN__(x)        __attribute__((aligned(x)))
#define __PACKED__          __attribute__((__packed__))
#define INLINE              inline


#endif

// danaplatform.h 编译器相关的
//
// dana_error.h

// dana_base.h // CXX_EXTERN_BEGIN/编译器相关[inline/export]/ 数据类型定义

// dana_time.h // ? 本机启动时间 // core中需要注意频率的处理要么采用内部计时器

// dana_mem.h // malloc/free/memset/memcpy  注意core代码里分配内存后需要memset(0)

// dana_task.h // 任务; 锁; 信号量

// dana_semaphore.h

// 随机数 

// dana_ioops.h

// dana_sock.h // ntoh/hton; ntoa_r; dns; nonblock; set[get]sockopt; getsockname; select; sock recv[from] send[to] connect bind accept listen close;
//             // 减少函数调用

// airlink: channel[以后是要支持5G的,所以需要遍历所有支持的频段]; 帧头[可以看libpcap--源MAC/目的MAC]

// 针对关闭UDP功能,设备需要上报不支持UDP,App可以立刻relay





//#ifndef __cplusplus
//#define bool                     _Bool
//#define true                     1
//#define false                    0
//#else
//#define _Bool                    bool
//#endif

#endif
