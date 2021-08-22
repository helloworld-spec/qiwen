#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ak_common.h"
#include "ak_vpss.h"

/************ for vi start ****************/
#include <signal.h>
#include "ak_vi.h"

struct vi_demo {
	void *vi_handle;
	int cap_num;
	char *save_path;
};
/************ for vi end ****************/

static void *init_video_in(void)
{
	/* get sensor id, match config file */
	const char *path[2] = {"/etc/jffs2/isp_sc1135.conf", "/usr/local/isp_sc1135.conf"};
	int ret = 0, i = 0;
	do {
		ret = ak_vi_match_sensor(path[i]);
		i++;
	} while (ret != 0 && i < 2);

	if (ret != 0 && i == 2) {
		ak_print_error_ex("vi match config failed\n");
		return NULL;
	}

	/* vi open mast after match success*/
	void * handle = NULL;
	handle = ak_vi_open(VIDEO_DEV0);
	if (NULL == handle) {
    	ak_print_normal("########## ak_vi_open failed!\n");
    	return NULL;
	}

 	/* get sensor resolution */
	struct video_resolution sensor_res = {0};
	if (ak_vi_get_sensor_resolution(handle, &sensor_res)) {
		ak_print_normal("ak_mpi_vi_get_sensor_res failed!\n");
	}

	/* set crop information */
	struct video_channel_attr *attr  = NULL;
	attr = (struct video_channel_attr *)calloc(1, sizeof(struct video_channel_attr));

	attr->res[VIDEO_CHN_MAIN].width = 1280;
	attr->res[VIDEO_CHN_MAIN].height = 720;

	attr->res[VIDEO_CHN_SUB].width = 640;
	attr->res[VIDEO_CHN_SUB].height = 360;

	attr->res[VIDEO_CHN_MAIN].max_width = 1920;
	attr->res[VIDEO_CHN_MAIN].max_height = 1080;
	attr->res[VIDEO_CHN_SUB].max_width = 640;
	attr->res[VIDEO_CHN_SUB].max_height = 480;

	attr->crop.left = 0;
	attr->crop.top = 0;
	attr->crop.width = 1280;
	attr->crop.height = 720;

	if (ak_vi_set_channel_attr(handle, attr)) {
		ak_print_normal("ak_vi_set_channel_attr failed!\n");
	}

	struct video_channel_attr *cur_attr = NULL;
	cur_attr = (struct video_channel_attr *)calloc(1, sizeof(struct video_channel_attr));

	if (ak_vi_get_channel_attr(handle, cur_attr)) {
		ak_print_normal("ak_vi_get_channel_attr failed!\n");
	}

	/* set fps */
    ak_vi_set_fps(handle, 25);

    /* get fps */
    int fps = 0;
    fps = ak_vi_get_fps(handle);
	ak_print_normal("cur fps:%d\n", fps);

    return handle;
}

static void process_signal(int sig)
{
	ak_print_notice("\t signal %d caught", sig);
	if((SIGTERM == sig) || (SIGINT == sig) || (SIGSEGV == sig)){
		exit(EXIT_FAILURE);
	}
}

static int register_signal(void)
{
	signal(SIGSEGV, process_signal);
	signal(SIGINT, process_signal);
	signal(SIGTERM, process_signal);
	signal(SIGCHLD, SIG_IGN);

	return 0;
}

static void vi_work(void *arg)
{
	struct vi_demo *pvi = (struct vi_demo *)arg;
	struct video_input_frame frame = {{{0}, {0}}};
	int i = 0;
	enum vpss_effect_type type;
	int effect;
	int drop_num;

    drop_num = ak_vi_get_fps(pvi->vi_handle);
	if (drop_num > 0)
		drop_num *= 2;
	else
		drop_num = 40;

    for (i = 0; i < drop_num; i++) {
		int ret = ak_vi_get_frame(pvi->vi_handle, &frame);
		if (!ret)
			/* the frame has used,release the frame data */
			ak_vi_release_frame(pvi->vi_handle, &frame);
	}

	type = VPSS_EFFECT_HUE;
	effect = 50;
	if (ak_vpss_effect_set(pvi->vi_handle, type, effect)) {
		ak_print_error_ex("set effect fail\n");
	}

	/* the first 5 frame have to drop,get the sixth frame */
    for (i = 0; i < pvi->cap_num; i++) {
		int ret = ak_vi_get_frame(pvi->vi_handle, &frame);
		if (ret) {
			ak_print_normal_ex("get frame failed!\n");
			continue;
		} else {
			#if 1
			/* save the YUV frame data */
			ak_print_normal_ex("saving frame ....\n");

			FILE *fd = NULL;
			struct ak_date date;
			char time_str[32] = {0};
			char file_path[255] = {0};

			ak_get_localdate(&date);
			ak_date_to_string(&date, time_str);
			sprintf(file_path, "%s%s_%d.yuv", pvi->save_path, time_str, i);

			fd = fopen(file_path, "w+b");
			if (fd) {
				unsigned char *buf = frame.vi_frame[VIDEO_CHN_MAIN].data;
				unsigned int len = frame.vi_frame[VIDEO_CHN_MAIN].len;
				ak_print_normal("yuv len: %u\n", len);
				do {
					len -= fwrite(buf, 1, len, fd);
				} while (len != 0);
			} else {
				ak_print_normal("open YUV file failed!!\n");
			}
			if (fd)
				fclose(fd);
			#endif

			/* the frame has used,release the frame data */
			ak_vi_release_frame(pvi->vi_handle, &frame);

			if (i == 10) {
				effect = -50;
				if (ak_vpss_effect_set(pvi->vi_handle, type, effect)) {
					ak_print_error_ex("set effect fail\n");
				}
			}
		}
	}
	ak_print_normal_ex("over\n");
}

int main(int argc, char **argv)
{
	ak_print_normal("**********mask demo begin************\n\n");
	if (argc != 3) {
		ak_print_normal("%s capture frames and store to 'save_path'\n", argv[0]);
		ak_print_normal("Usage: %s n save_path\n", argv[0]);
		ak_print_normal("e.g: %s 10 /mnt/\n", argv[0]);
		return -1;
	}

	int capture_num = atoi(argv[1]);
	if (capture_num <= 0) {
		ak_print_normal("invalid arguments, 'n' must big than zero\n");
		return -1;
	} else
		ak_print_normal("capture %d frame\n", capture_num);

	register_signal();

	/* thread1 open */
	struct vi_demo *pvi = (struct vi_demo *)calloc(1, sizeof(struct vi_demo));
	pvi->cap_num = capture_num;
	pvi->save_path = argv[2];
	pvi->vi_handle = init_video_in();
	if (!pvi->vi_handle) {
		free(pvi);
		return -1;
	}
	ak_print_normal("open vi ok\n");

	ak_vi_capture_on(pvi->vi_handle);

	vi_work(pvi);

	/* close video in model */
	ak_vi_capture_off(pvi->vi_handle);
	ak_vi_close(pvi->vi_handle);
	pvi->vi_handle = NULL;
	free(pvi);
	ak_print_normal("**********mask demo finish************\n\n");

	return 0;
}
