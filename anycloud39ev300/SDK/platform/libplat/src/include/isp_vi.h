#ifndef _ISP_VI_H_
#define _ISP_VI_H_

#include "ak_common.h"

#define RAW_SAVE_PATH	"/tmp/frame.raw"


enum camera_pcid {
	PCID_CH2_OUTPUT_FMT	 = 1,
    PCID_A_FRAME_RAW,
};


/* isp vi status */
enum isp_vi_status {
	ISP_VI_STATUS_RESERVED = 0x00,	//reserved flag
	ISP_VI_STATUS_OPEN,				//just open
	ISP_VI_STATUS_RESET,			//just reset after open
	ISP_VI_STATUS_CLOSE,			//just close
};

struct isp_frame {
	void *private_data; 		//YUV data buffer address
	unsigned long long ts;		//timestamp(ms)
	unsigned char *buf;			//stream data buffer begin address
	unsigned long seq_no;		//current frame sequence no.
	struct ak_timeval get_time;	//get frame time
};

/**
 * isp_vi_open: open video0 handle
 * @void
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_open(void);

/**
 * isp_vi_close: close video0 handle
 * @void
 * return: void
 * notes:
 */
void isp_vi_close(void);

/**
 * isp_vi_get_sensor_attr: get the resolution of sensor support
 * @width[OUT]: the width of sensor resolution
 * @height[OUT]: the height of sensor resolution
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_get_sensor_attr(int *width, int *height);

/**
 * isp_vi_set_main_attr: set the property of channel main resolution
 * @width[IN]: the width of the resolution
 * @height[IN]: the height of the resolution
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_set_main_attr(const int width, const int height);

/**
 * isp_vi_set_sub_attr: set the property of channel sub resolution
 * @width[IN]: the width of the resolution
 * @height[IN]: the height of the resolution
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_set_sub_attr(const int width, const int height);

/**
 * isp_vi_get_raw_data: send cmd to get raw data
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_get_raw_data(void);


/**
 * isp_vi_set_crop_attr: set the position and resolution of crop
 * @left[IN]: the left position of the crop
 * @top[IN]: the top position of the crop
 * @width[IN]: the width of the crop resolution
 * @height[IN]: the height of the crop resolution
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_set_crop_attr(const int left, const int top,
								const int width, const int height);

/**
 * isp_vi_capture_on - open isp capture and set 4 buffer for isp driver
 * @status[IN]: isp vi operate status
 * @buffer_size[IN]: buffer_size = (main channel width *main channel)*3/2 
 *						+ (sub channel width *sub channel)*3/2
 * return: 0 success, -1 failed
 * notes:
 */
int isp_vi_capture_on(enum isp_vi_status status, const int buffer_size);

/**
 * isp_vi_capture_off: close isp capture
 * @status[IN]: isp vi operate status
 * return: 0 success, -1 failed
 * notes:
 */
int isp_vi_capture_off(enum isp_vi_status status);

/**
 * isp_vi_get_fps: get fps
 * @void
 * return: fps value
 * notes:
 */
int isp_vi_get_fps(void);

/**
 * isp_vi_set_fps: set fps
 * @fps[IN]: the fps value to be set
 * return: 0 success, -1 failed
 * notes:
 */
int isp_vi_set_fps(const int fps);

/**
 * isp_vi_get_frame: get frame information
 * @frame_info[IN]: the frame information
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_get_frame(struct isp_frame *frame);

/**
 * isp_vi_reset_drop_frame: reset vi isp drop frame count
 * @void
 * return: void
 * notes: reset drop frame before get frame to encode again.
 */
void isp_vi_reset_drop_frame(void);

/**
 * isp_vi_release_frame: release the frame buffer
 * @pbuf[IN]: buffer address
 * return: 0 success, otherwise failed
 * notes:
 */
int isp_vi_release_frame(void *pbuf);

int isp_vi_stream_ctrl_off(void);
int isp_vi_stream_ctrl_on(void);
#endif
