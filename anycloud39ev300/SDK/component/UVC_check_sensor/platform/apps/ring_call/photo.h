/*
* photo.h
*/

#ifndef __PHOTO_H__
#define __PHOTO_H__

#include "ak_venc.h"

#define PHOTO_PIXEL_720P        0
#define PHOTO_PIXEL_VGA         1

int photo_open(int pixel);
int photo_one(struct video_stream *p_stream);
void photo_close(void);


#endif
