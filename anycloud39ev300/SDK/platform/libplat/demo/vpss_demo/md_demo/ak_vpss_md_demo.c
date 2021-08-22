#include <stdio.h>
#include <stdlib.h>

#include "ak_common.h"
#include "ak_vpss.h"

/************ for vi start ****************/
#include <signal.h>
#include "ak_vi.h"
#define DEFAULT_ISP_CONFIG "/etc/jffs2/"

struct vi_demo {
	void *vi_handle;
	int cap_num;
};
/************ for vi end ****************/

/*
 * defined sensitivity per block
 */
struct md_judge_param {
	int level_threshold;		//value range: [0, 65536)
	int blocks_threshold;		//md blocks trigger md
};

enum md_judge_result {
	JUDGE_RESULT_ERR = 0,
	JUDGE_RESULT_MD,
	JUDGE_RESULT_NO_MD
};

/*
 * do you md algorithm processing
 */
static int md_demo(const void *vi_handle, const struct md_judge_param *judge)
{
	struct vpss_md_info md;

	int ret = ak_vpss_md_get_stat(vi_handle, &md);
	if (ret) {
		ak_print_error_ex("ak_md get fail\n");
		ret = JUDGE_RESULT_ERR;
	} else {
		int i, j;
		int md_cnt = 0;

		for(i=0; i<16; i++) {
			for(j=0; j<32; j++) {
				if(md.stat[i][j] > judge->level_threshold)
					md_cnt++;
			}
		}

		if (md_cnt > judge->blocks_threshold)
			ret = JUDGE_RESULT_MD;
		else
			ret = JUDGE_RESULT_NO_MD;
	}

	return ret;
}

static int match_sensor(void)
{
	if (ak_vi_match_sensor(DEFAULT_ISP_CONFIG)) {
		ak_print_error_ex(" match sensor cfg fail!%s\n",DEFAULT_ISP_CONFIG);
		return AK_FAILED;
	} else {
		ak_print_normal_ex(" ak_vi_match_sensor ok!\n");
	}

	return AK_SUCCESS;
}

static void *init_video_in(void)
{
	/* get sensor id, match config file */
	if (match_sensor()) {
		ak_print_error_ex("match sensor fail!\n");
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
	attr = (struct video_channel_attr *)calloc(1,
		sizeof(struct video_channel_attr));

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
	cur_attr = (struct video_channel_attr *)calloc(1,
		sizeof(struct video_channel_attr));

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
	struct md_judge_param judge;
	int i = 0;
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

	judge.level_threshold = 5000;
	judge.blocks_threshold = 12;

	/* the first 5 frame have to drop,get the sixth frame */
    for (i = 0; i < pvi->cap_num; i++) {
		int ret = ak_vi_get_frame(pvi->vi_handle, &frame);
		if (ret) {
			ak_print_normal_ex("get frame failed!\n");
			continue;
		} else {
			if (JUDGE_RESULT_MD == md_demo(pvi->vi_handle, &judge))
				ak_print_normal("movtion detect!!\n");

			/* the frame has used,release the frame data */
			ak_vi_release_frame(pvi->vi_handle, &frame);
		}
	}
	ak_print_normal_ex("over\n");
}

int main(int argc, char **argv)
{
	ak_print_normal("**********md demo begin************\n\n");

	register_signal();

	/* thread1 open */
	struct vi_demo *pvi = (struct vi_demo *)calloc(1, sizeof(struct vi_demo));
	pvi->cap_num = 500;
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
	ak_print_normal("**********md demo finish************\n\n");

	return 0;
}
