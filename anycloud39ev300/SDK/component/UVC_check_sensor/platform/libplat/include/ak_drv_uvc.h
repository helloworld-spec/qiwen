#ifndef __AK_DRV_UVC_H__
#define __AK_DRV_UVC_H__

#ifdef __cplusplus
extern "C" {
#endif

enum stream_type {
	STREAM_YUV = 0,
	STREAM_MJPEG,
	STREAM_H264
};

struct uvc_stream {
	unsigned char *data;
	unsigned int len;
};

int ak_drv_uvc_start(int output_width, int output_height, enum stream_type type);
int ak_drv_uvc_stop();
int ak_drv_uvc_wait_stream(void);
int ak_drv_uvc_send_stream(struct uvc_stream *stream_data);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif
