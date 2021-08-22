/*************************************************

rtsp.h
used by rtsp

**************************************************/

#ifndef RTSP_H
#define RTSP_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "ak_dvs.h"


int rtsp_init_client (rtsp_client_t * client);
void *rtsp_client_listen (void *data);
void *rtsp_client_recv (void *data);
void *rtsp_client_send (void *data);
#ifdef __cplusplus
}
#endif
#endif
