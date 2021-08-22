#include "command.h"
/******************************************************
 * @brief  vi demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-17 
*******************************************************/
#include "ak_drv_ircut.h"
#include "ak_vi.h"
#include "ak_common.h"
#include "drv_api.h"
#include "kernel.h"

/******************************************************
  *                    Constant         
  ******************************************************/
static char *help[]={
	"vi module demo",
	"usage:videmo [framecount] <720p/vga> <8/10/12/25> <frameinterval> <day/night>\n"
	"      [framecount] <= 10\n"
	"      please insert t card , videmo will save data to a:/[filename]\n"
	"      frameinterval-save one frames interval of frameinterval frame(s)\n"
};

/******************************************************
  *                    Macro         
  ******************************************************/
#define SD_SAVE
#define HAVE_IRCUT

#define VIDEO_720P_WIDTH	1280
#define VIDEO_720P_HEIGHT	720
#define VIDEO_VGA_WIDTH		640
#define VIDEO_VGA_HEIGHT	480

/******************************************************
  *                    Type Definitions         
  ******************************************************/

/******************************************************
  *                    Global Variables         
  ******************************************************/

/******************************************************
*               Function prototype                           
******************************************************/


/******************************************************
*               Function Declarations
******************************************************/


#ifdef SD_SAVE
/*****************************************
 * @brief mount sd card
 * @param void
 * @return on success return 0, fail return -1
 *****************************************/
static int cmd_sd_mount(void)
{
	int ret;
	ret = ak_mount_fs(DEV_MMCBLOCK, 0, "");
	if (0 == ret)
		ak_print_normal("mount sd ok!\n");
	else
		ak_print_error("mount sd fail!\n");

	return ret;
}

/*****************************************
 * @brief unmount sd card
 * @param void
 * @return on success return 0, fail return -1
 *****************************************/
static int cmd_sd_unmount(void)
{
	int ret;
	ret = ak_unmount_fs(DEV_MMCBLOCK, 0, "");
	if (0 == ret)
		ak_print_normal("unmount sd ok!\n");
	else
		ak_print_error("unmount sd fail!\n");

	return ret;
}
#endif

/*****************************************
 * @brief switch video mode
 * @param is_day[in]  day or night mode
 * @return void
 *****************************************/
static void switch_video_mode(int is_day)
{
	if (is_day) {
#ifdef HAVE_IRCUT
		ak_drv_ir_set_ircut(IRCUT_STATUS_DAY);
#endif
		isp_conf_set_day();
	 } else {
#ifdef HAVE_IRCUT
		ak_drv_ir_set_ircut(IRCUT_STATUS_NIGHT);
#endif
		isp_conf_set_night();
	 }
}

/*****************************************
 * @brief video input init
 * @param main_width[in]  main channel width
 * @param main_height[in]  main channel height
 * @param sensor_fps[in]   sensor fps
 * @return on success return video handler, fail return NULL
 *****************************************/
static void *init_video_in(int main_width, int main_height, int sensor_fps)
{
	void *handle;

	/* open isp sdk */
	handle = ak_vi_open(VIDEO_DEV0);
	if (NULL == handle) {
		ak_print_error_ex("##########$$  ak_vi_open failed!\n");
		return NULL;
	} 

	switch_video_mode(1);
	
	/* show vi lib version */
	ak_print_normal("=========%s=========\n", ak_vi_get_version());
	
	/* get sensor resolution */
	struct video_resolution sensor_res = {0};
	if (ak_vi_get_sensor_resolution(handle, &sensor_res))
	{
		ak_print_error_ex("ak_mpi_vi_get_sensor_res failed!\n");
		ak_vi_close(handle);
		return NULL;
	}	
	else 
		;//ak_print_normal_ex("ak_mpi_vi_get_sensor_res ok! w:%d, h:%d\n",
		//		sensor_res.width, sensor_res.height);

	/* set crop information */
	struct video_channel_attr obj_set_attr = {0};
	struct video_channel_attr *attr  = &obj_set_attr;

	attr->res[VIDEO_CHN_MAIN].width = main_width;
	attr->res[VIDEO_CHN_MAIN].height = main_height;

	attr->res[VIDEO_CHN_SUB].width = 640;
	attr->res[VIDEO_CHN_SUB].height = 360;
	attr->crop.left = 0;
	attr->crop.top = 0;
	attr->crop.width = 1280;
	attr->crop.height = 720;

	if (ak_vi_set_channel_attr(handle, attr)) {
		ak_print_error_ex("ak_vi_set_channel_attr failed!\n");
	} else {
		//ak_print_normal_ex("ak_vi_set_channel_attr success!\n");
	}

	struct video_channel_attr obj_get_attr = {0};
	struct video_channel_attr *cur_attr = &obj_get_attr;

	if (ak_vi_get_channel_attr(handle, cur_attr)) {
		ak_print_error_ex("ak_vi_get_channel_attr failed!\n");
		ak_vi_close(handle);
		return NULL;
	}

	//ak_print_normal_ex("ak_vi_get_channel_attr =%d!\n",cur_attr->res[VIDEO_CHN_MAIN].width);
	/* set fps */
	ak_vi_set_fps(handle, sensor_fps);

	/* get fps */
	int fps = 0;
	fps = ak_vi_get_fps(handle);
	ak_print_normal("get default fps = %d\n",fps);

	return handle;
}

/*****************************************
 * @brief get frames
 * @param handle[in]  vi handler
 * @param framecount[in]  frame count for get
 * @param frame_interval[in]  frame interval
 * @param save[out]  all frames
 * @param save_len[out]  the len of all frames
 * @return void
 *****************************************/
static void get_frames(void *handle, int framecount, int frame_interval, int is_day, unsigned char *save, int *save_len)
{
	/* get the frame information */
	struct video_input_frame frame;
	int i = 0;
	int all_pic_cnt;
	int valid_pic_cnt;
	int used_pic_cnt;
	int fps = 0;
	int yuv_len = 0;
	int flip_enable,mirror_enable;

	switch_video_mode(is_day);
	
	for (all_pic_cnt = 0, valid_pic_cnt = 0, used_pic_cnt = 0; used_pic_cnt < framecount; all_pic_cnt++) {

		
		ak_vi_get_flip_mirror(handle, &flip_enable, &mirror_enable);
		ak_print_normal("flip_enable:%d, mirror_enable:%d!\n",flip_enable,mirror_enable);
		if( all_pic_cnt == framecount-2 )
			ak_vi_set_flip_mirror(handle, 1, 1);
		if (ak_vi_get_frame(handle, &frame)) {
			ak_print_error_ex("ak_vi_get_frame failed!\n");
			continue;
		} else {
			ak_print_debug_ex("ak_vi_get_frame success!\n");
			ak_print_debug_ex("main channel len=%d,ts=%ld\n",
					frame.vi_frame[VIDEO_CHN_MAIN].len,
					frame.vi_frame[VIDEO_CHN_MAIN].ts);
			ak_print_debug_ex("sub channel len=%d,ts=%ld\n",
					frame.vi_frame[VIDEO_CHN_SUB].len,
					frame.vi_frame[VIDEO_CHN_SUB].ts);
			fps = ak_vi_get_fps(handle);
			ak_print_debug("get fps = %d\n",fps);
	

			if (0 == (valid_pic_cnt % (frame_interval + 1))) {
#ifdef SD_SAVE
				memcpy(save + yuv_len, frame.vi_frame[VIDEO_CHN_MAIN].data, frame.vi_frame[VIDEO_CHN_MAIN].len);
#endif
				yuv_len += frame.vi_frame[VIDEO_CHN_MAIN].len;
				used_pic_cnt++;
			}
			valid_pic_cnt++;

			/* the frame has used,release the frame data */
			ak_vi_release_frame(handle, &frame);
		}
	}

	*save_len = yuv_len;

}

/*****************************************
 * @brief start function for command
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
void cmd_vi_test(int argc, char **args)  
{
	int ret = 0;
	int framecount;
	int res_720p = 1;
	int sensor_fps = 25;
	int frame_interval = 0;
	int is_day = 1;
	char filename[128];
	void *handle = NULL;
	
	struct ak_date systime;
	unsigned char *yuv;
	int yuv_len;
	int width, height;
	FILE *file_handle;
	int i;
	char *main_addr ="ISPCFG"; //"/etc/jffs2";

	if (!(argc >= 1 && argc <= 5))
	{
		ak_print_error("%s",help[1]);
		return;
	}

	for (i = 0; i < argc; i++) {
		switch (i) {
			case 0:
				framecount = atoi(args[i]);

				/* ymx : videmo  framecount no need greater than 10.*/
				if(framecount > 10)
				{
					ak_print_error_ex("framecount is invalid:%s\n", args[i]);
					ak_print_error("%s",help[1]);
					return;
				}
				break;

			case 1:
				if (0 == strcmp("720p", args[i])) {
					res_720p = 1;
					sensor_fps = 25;
				} else if (0 == strcmp("vga", args[i])) {
					res_720p = 0;
					sensor_fps = 10;
				} else {
					ak_print_error_ex("don't know resolution:%s\n", args[i]);
					ak_print_error("%s",help[1]);
					return;
				}
				break;

			case 2:
				sensor_fps = atoi(args[i]);
				break;

			case 3:
				frame_interval = atoi(args[i]);
				break;

			case 4:
				if (0 == strcmp("day", args[i]))
					is_day = 1;
				else if (0 == strcmp("night", args[i]))
					is_day = 0;
				else {
					ak_print_error_ex("don't know time mode select:%s\n", args[i]);
					ak_print_error("%s",help[1]);
					return;
				}
				break;

			default:
				ak_print_error_ex("i err:%d\n", i);
				break;
		}
	}

	if (res_720p) {
		width = VIDEO_720P_WIDTH;
		height = VIDEO_720P_HEIGHT;
	} else {
		width = VIDEO_VGA_WIDTH;
		height = VIDEO_VGA_HEIGHT;
	}
#ifdef SD_SAVE	
	ak_get_localdate(&systime);

	sprintf(filename, "a:/videmo_%d%02d%02d%02d%02d%02d_%s.yuv", systime.year, 
			systime.month, systime.day, systime.hour, systime.minute, 
			systime.second,
			res_720p ? "720p":"vga");
	ak_print_normal("filename:%s\n",filename);

	yuv = malloc(width * height * 3 / 2 * framecount);
	if (NULL == yuv) {
		ak_print_error_ex("malloc fail: width:%d, height:%d, framecount:%d\n", 
				width, height, framecount);
		return;
	}
#endif

	ak_print_info_ex("***************************************\n\n");
	ak_print_info_ex("********** This is vi demo ************\n\n");
	ak_print_info_ex("***************************************\n\n");

	if (ak_vi_match_sensor(main_addr)) {
		
		ak_print_error_ex("##########$$  match sensor main_addr failed!\n");
	
		goto finish_free;
	} else {
		//ak_print_normal_ex("##########$$  ak_vi_match_sensor success!\n");
	}

#ifdef HAVE_IRCUT
	ak_drv_ir_init();
#endif

	handle = init_video_in(width, height, sensor_fps);
	if (NULL == handle) {
		goto finish_free;
	}

	get_frames(handle, framecount, frame_interval, is_day, yuv, &yuv_len);

	/* close video in model */
	ak_vi_close(handle);

#ifdef SD_SAVE
	ret = cmd_sd_mount();
	if (ret < 0)
		goto finish_free;

	file_handle = fopen( filename, "w" );
	if (file_handle == NULL) {
		ak_print_error("open file failed!\n");
		cmd_sd_unmount();
		goto finish_free;
	} else {
		ak_print_normal("open file success!\n");
	}

	ret = fwrite(yuv, yuv_len, 1, file_handle );
	ak_print_normal("write %d size data!\n", ret);

	fclose(file_handle);
	cmd_sd_unmount();
#endif

finish_free:
#ifdef SD_SAVE
	free(yuv);
	yuv = NULL;
#endif
	ak_print_normal_ex("**********video input demo finish************\n\n");
}

void jk_vi_test()
{
#if 0
	int ret = 0;
	int framecount;
	int res_720p = 1;
	int sensor_fps = 25;
	int frame_interval = 0;
	int is_day = 1;
	char filename[128];
	void *handle = NULL;
    T_SYSTIME systime;
	unsigned char *yuv;
	int yuv_len;
	int width, height;
	FILE *file_handle;
	int i;
	char *main_addr ="ISPCFG"; //"/etc/jffs2";

	if (!(argc >= 1 && argc <= 5))
	{
		ak_print_error("%s",help[1]);
		return;
	}

	for (i = 0; i < argc; i++) {
		switch (i) {
			case 0:
				framecount = atoi(args[i]);

				// ymx : videmo  framecount no need greater than 10.
				if(framecount > 10)
				{
					ak_print_error_ex("framecount is invalid:%s\n", args[i]);
					ak_print_error("%s",help[1]);
					return;
				}
				break;

			case 1:
				if (0 == strcmp("720p", args[i])) {
					res_720p = 1;
					sensor_fps = 25;
				} else if (0 == strcmp("vga", args[i])) {
					res_720p = 0;
					sensor_fps = 10;
				} else {
					ak_print_error_ex("don't know resolution:%s\n", args[i]);
					ak_print_error("%s",help[1]);
					return;
				}
				break;

			case 2:
				sensor_fps = atoi(args[i]);
				break;

			case 3:
				frame_interval = atoi(args[i]);
				break;

			case 4:
				if (0 == strcmp("day", args[i]))
					is_day = 1;
				else if (0 == strcmp("night", args[i]))
					is_day = 0;
				else {
					ak_print_error_ex("don't know time mode select:%s\n", args[i]);
					ak_print_error("%s",help[1]);
					return;
				}
				break;

			default:
				ak_print_error_ex("i err:%d\n", i);
				break;
		}
	}

	

    systime = rtc_get_systime();
	sprintf(filename, "a:/videmo_%d%02d%02d%02d%02d%02d_%s.yuv", systime.year, 
			systime.month, systime.day, systime.hour, systime.minute, 
			systime.second,
			res_720p ? "720p":"vga");
	ak_print_normal("filename:%s\n",filename);
#else
	int ret = 0;
	int framecount;
	int res_720p = 1;
	int sensor_fps = 25;
	int frame_interval = 0;
	int is_day = 1;
	char filename[128];
	void *handle = NULL;
	T_SYSTIME systime;
	unsigned char *yuv;
	int yuv_len;
	int width, height;
	FILE *file_handle;
	int i;
	char *main_addr ="ISPCFG"; //"/etc/jffs2";

	framecount =10;

	res_720p = 1;
	
	sensor_fps = 25;

	frame_interval = 0;

	is_day = 1;
	strcpy(filename,"ak_startime.yuv");
    
	
	ak_print_normal("filename:%s\n",filename);
#endif
	if (res_720p) {
		width = VIDEO_720P_WIDTH;
		height = VIDEO_720P_HEIGHT;
	} else {
		width = VIDEO_VGA_WIDTH;
		height = VIDEO_VGA_HEIGHT;
	}

#ifdef SD_SAVE
	yuv = malloc(width * height * 3 / 2 * framecount);
	if (NULL == yuv) {
		ak_print_error_ex("malloc fail: width:%d, height:%d, framecount:%d\n", 
				width, height, framecount);
		return;
	}
#endif

	ak_print_info_ex("***************************************\n\n");
	ak_print_info_ex("********** This is vi demo ************\n\n");
	ak_print_info_ex("***************************************\n\n");

	if (ak_vi_match_sensor(main_addr)) {
		
		ak_print_error_ex("##########$$  match sensor main_addr failed!\n");
	
		goto finish_free;
	} else {
		//ak_print_normal_ex("##########$$  ak_vi_match_sensor success!\n");
	}

#ifdef HAVE_IRCUT
	ak_drv_ir_init();
#endif

	handle = init_video_in(width, height, sensor_fps);
	if (NULL == handle) {
		goto finish_free;
	}

	get_frames(handle, framecount, frame_interval, is_day, yuv, &yuv_len);
	
	/* close video in model */
	ak_vi_close(handle);

#ifdef SD_SAVE
	ret = cmd_sd_mount();
	if (ret < 0)
		goto finish_free;

	file_handle = fopen( filename, "w" );
	if (file_handle == NULL) {
		ak_print_error("open file failed!\n");
		cmd_sd_unmount();
		goto finish_free;
	} else {
		ak_print_normal("open file success!\n");
	}

	ret = fwrite(yuv, yuv_len, 1, file_handle );
	ak_print_normal("write %d size data!\n", ret);

	fclose(file_handle);
	cmd_sd_unmount();
#endif

finish_free:
#ifdef SD_SAVE
	free(yuv);
	yuv = NULL;
#endif
	ak_print_normal_ex("**********video input demo finish************\n\n");
}	
static int cmd_vi_reg(void)
{
    cmd_register("videmo", cmd_vi_test, help);
    return 0;
}

cmd_module_init(cmd_vi_reg)




