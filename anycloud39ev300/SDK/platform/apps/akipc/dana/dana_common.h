#ifndef _AK_DANA_INIT_H_
#define _AK_DANA_INIT_H_

#include "ak_thread.h"
#include "danavideo.h"
/*
 * this  is for danale connect , it is private data.
 * when danale pass one connect to platform,  
 * we creat one of MYDATA to it.  
 */
typedef struct _MyData {
    pdana_video_conn_t *danavideoconn; /*it get from dana*/
    int device_type;
    unsigned char chan_no;
    char appname[32]; 
	unsigned int fps ;
	int video_conn_flg;
	int audio_conn_flg;	
} MYDATA;

#endif
