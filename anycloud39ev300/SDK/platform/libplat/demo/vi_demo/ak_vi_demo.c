/**
* Copyright (C) 2018 Anyka(Guangzhou) Microelectronics Technology CO.,LTD.
* File Name: ak_vi_demo.c
* Description: This is a simple example to show how the VI module working.
* Notes:
* History: V1.0.0
*/
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ak_common.h"
#include "ak_vi.h"

/* 
 * check_dir: check whether the 'path' was exist.
 * path[IN]: pointer to the path which will be checking.
 * return: 1 on exist, 0 is not.
 */
static int check_dir(const char *path)
{
	struct stat buf = {0};

	if (NULL == path)
		return 0;

	stat(path, &buf);
	if (S_ISDIR(buf.st_mode)) {
		return 1;
	} else {
		return 0;
	}
}

/* 
 * save_yuv_data - use to save yuv data to file
 * path[IN]: pointer to saving directory
 * index[IN]: frame number index
 * frame[IN]: pointer to yuv data, include main and sub channel data
 * attr[IN]:  vi channel attribute.
 */
static void save_yuv_data(const char *path, int index, 
		struct video_input_frame *frame, struct video_channel_attr *attr)
{
	FILE *fd = NULL;
	unsigned int len = 0;
	unsigned char *buf = NULL;
	struct ak_date date;
	char time_str[32] = {0};
	char file_path[255] = {0};

	ak_print_normal("saving frame, index=%d\n", index);

	/* construct file name */
	ak_get_localdate(&date);
	ak_date_to_string(&date, time_str);
	sprintf(file_path, "%smain_%s_%d_%dx%d.yuv", path, time_str, index,
			attr->res[VIDEO_CHN_MAIN].width, attr->res[VIDEO_CHN_MAIN].height);

	/* 
	 * open appointed file to save YUV data
	 * save main channel yuv here
	 */
	fd = fopen(file_path, "w+b");
	if (fd) {
		buf = frame->vi_frame[VIDEO_CHN_MAIN].data;
		len = frame->vi_frame[VIDEO_CHN_MAIN].len;
		do {
			len -= fwrite(buf, 1, len, fd);
		} while (len != 0);
		
		fclose(fd);
	} else {
		ak_print_normal("open YUV file failed!!\n");
	}
		

	/* generate YUV file name */
	sprintf(file_path, "%ssub_%s_%d_%dx%d.yuv", path, time_str, index,
			attr->res[VIDEO_CHN_SUB].width, attr->res[VIDEO_CHN_SUB].height);

	/* 
	 * save sub channel yuv here
	 */
	fd = fopen(file_path, "w+b");
	if (fd) {
		buf = frame->vi_frame[VIDEO_CHN_SUB].data;
		len = frame->vi_frame[VIDEO_CHN_SUB].len;
		do {
			len -= fwrite(buf, 1, len, fd);
		} while (len != 0);

		fclose(fd);
	} else {
		ak_print_normal("open YUV file failed!!\n");
	}
}

/*
 * vi_capture_loop: loop to get and release yuv, between get and release,
 *                  here we just save the frame to file, on your platform,
 *                  you can rewrite the save_function with your code.
 * vi_handle[IN]: pointer to vi handle, return by ak_vi_open()
 * number[IN]: save numbers
 * path[IN]:   save directory path, if NULL, will not save anymore.
 * attr[IN]:   vi channel attribute.
 */
static void vi_capture_loop(void *vi_handle, int number, const char *path,
		struct video_channel_attr *attr)
{
	int count = 0;
	struct video_input_frame frame = {{{0}, {0}}};

	ak_print_normal("capture start\n");

	/*
	 * To get frame by loop
	 */
    while (count < number) {
		memset(&frame, 0x00, sizeof(frame));

		/* to get frame */
		int ret = ak_vi_get_frame(vi_handle, &frame);
		if (!ret) {
			/* 
			 * Here, you can implement your code to use this frame.
			 * Notice, do not occupy it too long.
			 */
			ak_print_normal("[%d] main chn yuv len: %u\n", count,
					frame.vi_frame[VIDEO_CHN_MAIN].len);
			ak_print_normal("[%d] sub  chn yuv len: %u\n\n", count,
					frame.vi_frame[VIDEO_CHN_SUB].len);

			/* 
			 * TODO: Write your code here to replace save_yuv_data() 
			 * to implement your things.
			 */
			if (check_dir(path))
				save_yuv_data(path, count, &frame, attr);
			else if (path)
				ak_print_warning("Please create directory: %s to save yuvs\n", path);

			/* 
			 * in this context, this frame was useless,
			 * release frame data
			 */
			ak_vi_release_frame(vi_handle, &frame);

			/* add counter */
			count++;
		} else {
			/* 
			 *	If getting too fast, it will have no data,
			 *	just take breath.
			 */
			ak_sleep_ms(10);
		}
	}

	ak_print_normal("capture finish\n\n");
}

/**
 * Preconditions:
 * 1、TF card is already mounted
 * 2、yuv_data is already created in /mnt
 * 3、ircut is already opened at day mode
 * 4、your main video progress must stop
 */
int main(int argc, char **argv)
{
    ak_print_normal("*****************************************\n");
	ak_print_normal("** vi demo version: %s **\n", ak_vi_get_version());
    ak_print_normal("*****************************************\n");
	
	/* 
	 * step 0: global value initialize
	 */
	int ret = -1;								//return value
	int save_num = 3;							//save frame numbers
	void *vi_handle = NULL;						//vi operating handle
	struct video_channel_attr attr;				//vi channel attribute
	struct video_resolution res;				//max sensor resolution
	const char *cfg = "/etc/jffs2/";			//isp config file storage path
	const char *yuv_path = "/mnt/yuv_data/";	//yuv save path

	if (argc > 1) {
		ak_print_warning("%s don't support any argument\n", argv[0]);	
		return -1;
	}

	attr.crop.left = 0;
	attr.crop.top  = 0;
	attr.crop.width = 1920;
	attr.crop.height = 1080;

	attr.res[VIDEO_CHN_MAIN].width = 1920;
	attr.res[VIDEO_CHN_MAIN].height = 1080;
	attr.res[VIDEO_CHN_SUB].width = 640;
	attr.res[VIDEO_CHN_SUB].height = 360;

	attr.res[VIDEO_CHN_MAIN].max_width = 1920;
	attr.res[VIDEO_CHN_MAIN].max_height = 1080;
	attr.res[VIDEO_CHN_SUB].max_width = 640;
	attr.res[VIDEO_CHN_SUB].max_height = 480;

	/* 
	 * step 1: match sensor
	 * the location of isp config can either a file or a directory 
	 */
	ret = ak_vi_match_sensor(cfg);
	if (ret) {
		ak_print_normal("match sensor failed\n");	
		goto exit;
	}

	/* 
	 * step 2: open video input device
	 */
	vi_handle = ak_vi_open(VIDEO_DEV0);
	if (NULL == vi_handle) {
		ak_print_normal("vi device open failed\n");	
		goto exit;
	}

	/* 
	 * step 3: get sensor support max resolution
	 */
	ret = ak_vi_get_sensor_resolution(vi_handle, &res);
	if (ret) {
		ak_vi_close(vi_handle);
		goto exit;
	} else {
		attr.crop.width = res.width;
		attr.crop.height = res.height;
	}

	/* 
	 * step 4: set vi working parameters 
	 * default parameters: 25fps, day mode, auto frame-control
	 */
	ret = ak_vi_set_channel_attr(vi_handle, &attr);
	if (ret) {
		ak_vi_close(vi_handle);
		goto exit;
	}

	/* 
	 * step 5: start capture frames
	 */
	ret = ak_vi_capture_on(vi_handle);
	if (ret) {
		ak_vi_close(vi_handle);
		goto exit;
	}

	/* 
	 * step 6: start to capture and save yuv frames 
	 */
	vi_capture_loop(vi_handle, save_num, yuv_path, &attr);

	/*
	 * step 7: release resource
	 */
	ak_vi_capture_off(vi_handle);

	ret = ak_vi_close(vi_handle);

exit:
	/* exit */
	ak_print_normal("exit vi demo\n");

	return ret;
}
