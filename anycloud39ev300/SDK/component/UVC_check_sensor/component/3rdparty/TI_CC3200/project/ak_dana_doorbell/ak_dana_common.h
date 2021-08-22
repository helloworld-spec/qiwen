
#ifndef __AK_DANA_COMMON_H__
#define __AK_DANA_COMMON_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif
#include"dana_task.h"

#define VIDEO_BUF_SIZE	(100*1024)

//extern char video_buf[1024*30];
//extern dana_mutex_handler_t *danavideotest_lock;
//extern OsiMsgQ_t spi2videomsg;
extern OsiSyncObj_t  door_ring_sem;
void ak2dana_stream(char *video_buf);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //  __AK_DANA_COMMON_H__





