/******************************************************
 * @brief  move detect  demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-17 
*******************************************************/
#include "command.h"
#include "ak_common.h"
#include "ak_vpss.h"
#include "ak_vi.h"
#include "libc_mem.h"

/******************************************************
  *                    Constant         
  ******************************************************/
static char *help[]={
	"md module demo",
	"usage:mddemo\n"
};

/******************************************************
  *                    Macro         
  ******************************************************/
  
/******************************************************
  *                    Type Definitions         
  ******************************************************/
struct vi_demo {
	void *vi_handle;
	int cap_num;
};

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
/******************************************************
  *                    Global Variables         
  ******************************************************/

/******************************************************
*               Function prototype                           
******************************************************/

/******************************************************
*               Function Declarations
******************************************************/

 /*****************************************
 * @brief video move judge
 * @param handle[in]  video hander
 * @return judge result
 *****************************************/
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

/*****************************************
 * @brief video input init
 * @param void
 * @return on success return video handler, fail return NULL
 *****************************************/
static void *init_video_in(void)
{
	/* get sensor id, match config file */
	char *main_addr ="ISPCFG";
	int ret = 0;
	
	ret = ak_vi_match_sensor(main_addr);		
	if (ret) {
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
		ak_vi_close(handle);
		handle=  NULL;
	}

	/* set crop information */
	struct video_channel_attr *attr  = NULL;
	attr = (struct video_channel_attr *)calloc(1, 
		sizeof(struct video_channel_attr));

	attr->res[VIDEO_CHN_MAIN].width = 1280;
	attr->res[VIDEO_CHN_MAIN].height = 720;

	attr->res[VIDEO_CHN_SUB].width = 640;
	attr->res[VIDEO_CHN_SUB].height = 360;

	attr->crop.left = 0;
	attr->crop.top = 0;
	attr->crop.width = 1280;
	attr->crop.height = 720;

	if (ak_vi_set_channel_attr(handle, attr)) {
		ak_print_normal("ak_vi_set_channel_attr failed!\n");
		ak_vi_close(handle);
		handle = NULL;
	}

	free(attr);

    return handle;
	
}

/*****************************************
 * @brief move judge function
 * @param arg[in]  vi_handler and capture number
 * @return void
 *****************************************/
static void vi_work(void *arg)
{
	struct vi_demo *pvi = (struct vi_demo *)arg;
	struct video_input_frame frame;
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

/*****************************************
 * @brief start function for command
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
void cmd_md_test(int argc, char **argv)
{
	ak_print_normal("**********md demo begin************\n\n");


	/* thread1 open */
	struct vi_demo *pvi = (struct vi_demo *)calloc(1, sizeof(struct vi_demo));
	pvi->cap_num = 500;
	pvi->vi_handle = init_video_in();
	if (!pvi->vi_handle) {
		free(pvi);
		return ;
	}
	ak_print_normal("open vi ok\n");

	vi_work(pvi);

	/* close video in model */
	ak_vi_close(pvi->vi_handle);
	free(pvi);
	ak_print_normal("**********md demo finish************\n\n");

}

/*****************************************
 * @brief register mddemo command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_md_reg(void)
{
    cmd_register("mddemo", cmd_md_test, help);
    return 0;
}

cmd_module_init(cmd_md_reg)
