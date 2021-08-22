/**
 * @FILENAME: ak_drv_uvc.c
 * @BRIEF uvc driver file
 * Copyright (C) 2016 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @AUTHOR 
 * @DATE 2016-12-08
 * @VERSION 1.0
 * @REF
 */
//#include "akdefine.h"
//#include "gbl_global.h"
//#include "command.h"
#include "anyka_types.h"
#include "hal_usb_s_UVC.h"
#include "hal_usb_s_state.h"
#include "ak_drv_uvc.h"
#include "command.h"
//#include "ak_vi.h"
//#include "utils.h"
#include "ak_common.h"
#include "libc_mem.h"

struct uvc_uvc_info {
	int width;
	int height;
	int format;
	unsigned char *pPayload;
	unsigned char *pPayload1;
};
static struct uvc_uvc_info uvc_info;

volatile static bool uvc_sendfinish = false;
volatile static bool uvc_sendstart = 0;

static void uvc_sendfinish_func(void)
{
	//putch('I');
	uvc_sendfinish = true;
}

static void vc_control_cb(T_eUVC_CONTROL dwControl, unsigned int value1,unsigned int value2)
{
	ak_print_normal_ex("vc ctrl cb 0x%x  ", dwControl);
}

static void vs_control_cb(T_eUVC_CONTROL dwControl, unsigned int value1,unsigned int value2)
{
	unsigned int ulformat,ulFrameId;
	T_UVC_FRAME_RES FrameRes;
	ak_print_normal_ex("vs ctrl cb 0x%x\n", dwControl);

	if (UVC_CTRL_RESOLUTION == dwControl)
	{
		ulformat = (value1>>16)&0xff;
		ulFrameId = (value1>>24)&0xff;
		uvc_get_frame_res(&FrameRes,ulFrameId);

		//UVC_CapSize(FrameRes.unWidth,FrameRes.unHeight);
		uvc_sendstart = 1;
	}
	else
	{
		ak_print_normal_ex("unsupported UVC vs control\n");
	}
}

static void drv_uvc_buf_free(void)
{
	if (uvc_info.pPayload != NULL)
	{
		free(uvc_info.pPayload);
		uvc_info.pPayload = NULL;
	}

	if (uvc_info.pPayload1 != NULL)
	{
		free(uvc_info.pPayload1);
		uvc_info.pPayload1 = NULL;
	}
}

static bool drv_uvc_buf_allocate(int width, int height)
{
	bool ret = true;

	uvc_info.pPayload = (unsigned char *)malloc(width * height * 3);
	if (NULL == uvc_info.pPayload)
	{
		ak_print_error_ex("malloc pPayload fail\n");
		ret = false;
	}

	if (uvc_info.format == UVC_STREAM_MJPEG)
	{
		uvc_info.pPayload1 = (unsigned char *)malloc(width * height / 2);
	}
	else
	{
		uvc_info.pPayload1 = (unsigned char *)malloc(width * height * 2);
	}

	if (NULL == uvc_info.pPayload1)
	{
		ak_print_error_ex("malloc pPayload1 fail\n");
		ret = false;
	}

	if (false == ret)
	{
		drv_uvc_buf_free();
	}

	return ret;
}

/**
 * ak_uvc_open: open uvc device
 * @output_width: output video width
 * @output_height: output video height
 * @buffer_num: use how many buffers to capture video, max=4
 * @type: uvc output video type: yuv/mjpeg/h264 etc
 * return: 0: success; others: failed
 * notes:
 */
int ak_drv_uvc_start(int output_width, int output_height, enum stream_type type)
{
	switch (type) {
		case STREAM_YUV:
			uvc_info.format = UVC_STREAM_YUV;
			break;

			
	   case STREAM_MJPEG:
    	   uvc_info.format = UVC_STREAM_MJPEG;
    	   break;
			   	
		default:
			ak_print_error_ex(" uvc format no defined\n");
			return -1;
			break;
	}

	uvc_info.width = output_width;
	uvc_info.height = output_height;

	//disable usb at the beginning
	usb_slave_device_disable();

	ak_print_normal_ex("start pc camera\n");

	if (!drv_uvc_buf_allocate(output_width, output_height))
	{
		ak_print_error_ex("uvc buf alloc failed\n");
		return ;
	}

	if (!uvc_init(USB_MODE_20, uvc_info.format)) {
		ak_print_error_ex("uvc init failed\n");
		drv_uvc_buf_free();
		return ;
	}

	//set callback
	uvc_set_callback((T_pUVC_VC_CTRL_CALLBACK)vc_control_cb, (T_pUVC_VS_CTRL_CALLBACK)vs_control_cb, (T_pUVC_FRAME_SENT_CALLBACK)uvc_sendfinish_func);

	//init desc
	uvc_init_desc();

	ak_print_normal_ex("uvc init successful\r\n");

	uvc_start();

	uvc_sendfinish = true;

	return 0;
}

/**
 * ak_uvc_close: close uvc device
 * return: 0: success; others: failed
 * notes:
 */
int ak_drv_uvc_stop()
{
	uvc_stop();
	drv_uvc_buf_free();

	return 0;
}

/**
 * ak_uvc_wait_stream: waiting for pc connect
 * return: 0: success; others: failed
 * notes:
 */
int ak_drv_uvc_wait_stream(void)
{
    if(uvc_check_open() && uvc_sendfinish)
    	return 0;
    else
        return -1;
}

/**
 * ak_uvc_close: close uvc device
 * return: 0: success; others: failed
 * notes:
 */
int ak_drv_uvc_send_stream(struct uvc_stream *stream_data)
{
	int nsize;
	int ntmp;

	if (uvc_info.format == UVC_STREAM_YUV) {
		nsize = uvc_parse_yuv(uvc_info.pPayload1,
				stream_data->data,
				stream_data->data + uvc_info.width * uvc_info.height,
				stream_data->data + uvc_info.width * uvc_info.height * 5/ 4,
				uvc_info.width,
				uvc_info.height,
				YUV_FORMAT_420);
	}
	else if(uvc_info.format == UVC_STREAM_MJPEG)
	{
	    nsize = stream_data->len;
	    memcpy(uvc_info.pPayload1, stream_data->data, stream_data->len);
	}

	while (!uvc_sendfinish) 
		;

	uvc_sendfinish = false;

	ntmp = uvc_payload(uvc_info.pPayload, uvc_info.pPayload1, nsize);


	while (!uvc_send(uvc_info.pPayload, ntmp)) 
		;

	return 0;
}
