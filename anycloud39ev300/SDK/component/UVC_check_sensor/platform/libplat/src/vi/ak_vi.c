#include <stdio.h>
#include "list.h"
#include "internal_error.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "hal_camera.h"
#include "isp_conf.h"
#include "ak_isp_char.h"
#include "ak_isp_drv.h"


#include "isp_h62.conf.h"

#include "isp_sc1135.conf.h"

#include "isp_sc1145.conf.h"

#include "isp_sc1235.conf.h"

static T_CAMERA_BUFFER ch1_YUVs[4];
static T_CAMERA_BUFFER ch2_YUVs[4];
static struct camstream_info stream_info = {0};
static int isp_init = 0;

static int isp_frame_start = 0;
static int isp_fvpr_start = 0;
static struct isp_flip_mirror_info flip_mirror = {0,0};

static int old_i = -1;
static int old_save_arr[4] = {0,0,0,0};//save buf id0/1/2/3 is save, 0 no save 1 had saved
static int old_save_i_count=0;

#define SKIP_FRAME_COUNT 1 //定义isp启动后跳过开始的帧数
#define FVPR_FRAME_COUNT 20

#define VI_DEV_MAX_USER		(sizeof(int) * 8)

/* chip supported max and min resolution of main channel */
#define CHIP_MAIN_CHN_MAX_WIDTH 	(1280)
#define CHIP_MAIN_CHN_MAX_HEIGHT 	(960)

#define CHIP_MAIN_CHN_MIN_WIDTH 	(640)
#define CHIP_MAIN_CHN_MIN_HEIGHT 	(480)

/* chip supported max and min resolution of sub channel */
#define CHIP_SUB_CHN_MAX_WIDTH 		(640)
#define CHIP_SUB_CHN_MAX_HEIGHT 	(480)

#define CHIP_SUB_CHN_MIN_WIDTH 		(18)
#define CHIP_SUB_CHN_MIN_HEIGHT 	(18)


struct user_t {
	int dev_num; 			//video input device number
	int user_flg; 			//video input device number
	struct list_head list;	//list head, hang to device's user_list
	struct vi_device *pdev;
};

struct vi_device {
	int user_ref;
	int user_map;		//open user bit map
	int capture_on;
	int frame_count;
	int dev_num;
	void *isp_mem;		//user for 3D

	struct video_resolution *chn_main;
	struct video_resolution *chn_sub;
	struct crop_info *crop_info;
	struct list_head frame_list;  //store frames
	struct list_head dev;	      //hang to video_input_handle's dev_list
	struct list_head user_list;	  //if each dev has only 1 user, it is no use

	struct ak_timeval pre_get_time;	//previous get frame time
	ak_mutex_t vi_lock;		//this module's mutex lock
	ak_mutex_t frame_lock;	//frame list operate mutex lock
};

struct video_input_handle {
	unsigned char dma_flag;
	int dev_count;
	ak_mutex_t lock;
	struct list_head dev_list;
};

static const char vi_version[] = "libplat_vi_2.0.00";

static struct video_input_handle vi_ctrl = {
	.dma_flag = 0,
	.dev_count = 0,
	.lock = 0,
	.dev_list = LIST_HEAD_INIT(vi_ctrl.dev_list)
};
ak_mutex_t *g_vi_open_lock __attribute__((__section__(".lock_init"))) = &vi_ctrl.lock;

static int read_isp_conf_init(const char *file_name);

static bool vi_buf_init(unsigned int dstWidth, unsigned int dstHeight, T_CAMERA_BUFFER * YUVs, unsigned int num)
{
	unsigned int i;

	for (i = 0; i < num; i++) {
		YUVs[i].id = i;
		YUVs[i].dYUV = (unsigned char *)malloc(dstWidth*dstHeight * 3 / 2);

		if (NULL == YUVs[i].dYUV) {
			return false;
		}
		//ak_print_normal_ex("index:%d, %p\n",i, YUVs[i].dYUV);
	}

	return true;
}

static void vi_buf_free(T_CAMERA_BUFFER * YUVs, unsigned int num)
{
	unsigned int i;

	for (i = 0; i < num; i++) {
		if (YUVs[i].dYUV) {
			//ak_print_normal_ex("i:%d, %p\n",i, YUVs[i].dYUV);
			free(YUVs[i].dYUV);
			YUVs[i].dYUV = NULL;
		}
	}
}

static int vi_stream_info_deinit(void)
{
	vi_buf_free(ch1_YUVs, 4);
	vi_buf_free(ch2_YUVs, 4);

	memset(&stream_info, 0, sizeof(stream_info));

	return 0;
}

static int vi_stream_info_init(int ch1_width, int ch1_height, int ch2_width, int ch2_height,
		int crop_left, int crop_top, int crop_width, int crop_height)
{
	vi_stream_info_deinit();

	if (!vi_buf_init(ch1_width, ch1_height, ch1_YUVs, 4)) {
		ak_print_error_ex("malloc for ch1 fail\n");
		return -1;
	}

	if (!vi_buf_init(ch2_width, ch2_height, ch2_YUVs, 4)) {
		ak_print_error_ex("malloc for ch2 fail\n");
		vi_buf_free(ch1_YUVs, 4);
		return -1;
	}

	stream_info.ch1_enable = 1;
	stream_info.ch2_enable = 1;
	stream_info.single_mode = 0;
	stream_info.buffer_num = 4;
	stream_info.ch1_dstWidth = ch1_width;
	stream_info.ch1_dstHeight = ch1_height;
	stream_info.ch2_dstWidth = ch2_width;
	stream_info.ch2_dstHeight = ch2_height;
	stream_info.ch1_YUV[0] = &ch1_YUVs[0];
	stream_info.ch1_YUV[1] = &ch1_YUVs[1];
	stream_info.ch1_YUV[2] = &ch1_YUVs[2];
	stream_info.ch1_YUV[3] = &ch1_YUVs[3];
	stream_info.ch2_YUV[0] = &ch2_YUVs[0];
	stream_info.ch2_YUV[1] = &ch2_YUVs[1];
	stream_info.ch2_YUV[2] = &ch2_YUVs[2];
	stream_info.ch2_YUV[3] = &ch2_YUVs[3];
	stream_info.crop_left = crop_left;
	stream_info.crop_top = crop_top;
	stream_info.crop_width = crop_width;
	stream_info.crop_height = crop_height;

	return 0;
}

static int vi_check_main_channel_resolution(int width, int height)
{
	if (width <= 0 || height <= 0
			|| width > CHIP_MAIN_CHN_MAX_WIDTH
			|| height > CHIP_MAIN_CHN_MAX_HEIGHT
			|| width < CHIP_MAIN_CHN_MIN_WIDTH
			|| height < CHIP_MAIN_CHN_MIN_HEIGHT)
		return AK_FAILED;
	return AK_SUCCESS;
}

static int vi_check_sub_channel_resolution(int width, int height)
{
	if (width <= 0 || height <= 0
			|| width > CHIP_SUB_CHN_MAX_WIDTH
			|| height > CHIP_SUB_CHN_MAX_HEIGHT
			|| width < CHIP_SUB_CHN_MIN_WIDTH
			|| height < CHIP_SUB_CHN_MIN_HEIGHT)
		return AK_FAILED;
	return AK_SUCCESS;
}

static int vi_check_channel_attr(const struct video_channel_attr *attr)
{
	/* check main chn */
	if (vi_check_main_channel_resolution(attr->res[VIDEO_CHN_MAIN].width,
				attr->res[VIDEO_CHN_MAIN].height)) {
		ak_print_error_ex("main channel argument error, w: %d, h: %d\n",
			   attr->res[VIDEO_CHN_MAIN].width, attr->res[VIDEO_CHN_MAIN].height);
		return AK_FAILED;
	}

	/* check sub chn */
	if (vi_check_sub_channel_resolution(attr->res[VIDEO_CHN_SUB].width,
				attr->res[VIDEO_CHN_SUB].height)) {
		ak_print_error_ex("sub channel argument error, w: %d, h: %d\n",
			   attr->res[VIDEO_CHN_SUB].width, attr->res[VIDEO_CHN_SUB].height);
		return AK_FAILED;
	}

	/* check main and sub chn */
	if (attr->res[VIDEO_CHN_SUB].width > attr->res[VIDEO_CHN_MAIN].width
			|| attr->res[VIDEO_CHN_SUB].height > attr->res[VIDEO_CHN_MAIN].height) {
		ak_print_error_ex("main channel resolution must bigger than sub's,"
				"main w: %d, h: %d, sub w: %d, h: %d\n",
				attr->res[VIDEO_CHN_MAIN].width, attr->res[VIDEO_CHN_MAIN].height,
				attr->res[VIDEO_CHN_SUB].width, attr->res[VIDEO_CHN_SUB].height);
		return AK_FAILED;

	}

	if (attr->crop.width <= 0 || attr->crop.height <= 0) {
		ak_print_error_ex("crop argument error, w: %d, h: %d\n",
				attr->crop.width, attr->crop.height);
		return AK_FAILED;
	}

	return AK_SUCCESS;
}

/**
 * vi_get_device - match device by device number
 * @dev[IN]: appointed video input device number
 * return: pointer to device on success, fail on NULL
 */
static void *vi_get_device(int dev_num)
{
	struct vi_device *pdev = NULL;

	list_for_each_entry(pdev, &vi_ctrl.dev_list, dev) {
		if (pdev->dev_num == dev_num)
			return pdev;
	}

	return NULL;
}

static void *vi_new_user(void *dev, int dev_num)
{
	struct user_t *user = (struct user_t *)calloc(1, sizeof(struct user_t));
	if (!user) {
		ak_print_error_ex("No memory\n");
		return NULL;
	}

	int i = 0;
	struct vi_device *pdev = (struct vi_device *)dev;
	
	for (i=0; i<VI_DEV_MAX_USER; ++i) {
		if (!test_bit(i, &(pdev->user_map))) {
			user->user_flg = i;
			break;
		}
	}
	if (i >= VI_DEV_MAX_USER) {
		free(user);
		user = NULL;
		return NULL;
	}

	ak_thread_mutex_lock(&pdev->vi_lock);
	user->pdev = pdev;
	user->dev_num = dev_num;
	set_bit(&(pdev->user_map), user->user_flg); 
	add_ref(&(pdev->user_ref), 1);
	list_add_tail(&(user->list), &(pdev->user_list));
	ak_thread_mutex_unlock(&pdev->vi_lock);

	ak_print_notice_ex("now user_flg=%d, user_ref=%d\n",
		user->user_flg, pdev->user_ref);

	return user;
}

/**
 * vi_unregister_device -
 * @dev[IN]: pointer to device
 * return: 0 success, -1 failed
 * notes:
 */
static int vi_unregister_device(void *dev)
{
	struct vi_device *pdev = (struct vi_device *)dev;

	/* capture stop */
	ak_thread_mutex_lock(&pdev->vi_lock);
#if 0
	/* releas all frames */
	vi_release_all_frames(pdev);

	vi_release_channel_resource(pdev);

	/* close devices(isp and camera) */
	isp_module_deinit();
	int ret = isp_vi_capture_off();
	isp_vi_close();
	if (pdev->isp_mem) {
		akuio_free_pmem(pdev->isp_mem);
		pdev->isp_mem = NULL;
	}

	isp_dev_close();
#endif
	vi_stream_info_deinit();
	if (isp_init) {
		camstream_stop();
		isp_init = 0;
	}
	// run isp_conf_init when match sensor,isp_init==0
	isp_conf_deinit();
	cam_close();
	
	if (pdev->isp_mem) {
		akuio_free_pmem(pdev->isp_mem);
		pdev->isp_mem = NULL;
	}

	ak_thread_mutex_unlock(&pdev->vi_lock);
	ak_thread_mutex_destroy(&pdev->vi_lock);
	//vi_release_dma();
	vi_ctrl.dev_count--;
	list_del_init(&pdev->dev);
	free(pdev);
	pdev = NULL;
	ak_print_normal_ex("unregister done\n");

	return 0;
}

static void *vi_register_device(int dev_num)
{
	struct vi_device *pdev;

	if (!list_empty(&vi_ctrl.dev_list)) {
		pdev = vi_get_device(dev_num);
		if (pdev) {
			ak_print_normal("old device\n");
			return pdev;
		}
	}
	old_i = -1;	
	isp_conf_set_day();
	isp_conf_set_sensor_init_conf();
	/* is a new device */
	ak_print_normal("new device\n");
	pdev = (struct vi_device *)calloc(1, sizeof(struct vi_device));
	if (!pdev) {
		ak_print_error_ex("No memory\n");
		return NULL;
	}
	ak_print_normal_ex("new dev=%p\n", pdev);
#if 0 //linux
	vi_init_dma();

	if (isp_device_open()) {
		free(pdev);
		vi_release_dma();
		ak_print_error_ex("isp open deviece failed\n");
		return NULL;
	}

	int ret = isp_init();
	if (ret) {
		ak_print_error_ex("isp init failed, video cannot work\n");
		isp_device_close();
		vi_release_dma();
		free(pdev);
		return NULL;
	}
#endif
	pdev->dev_num = dev_num;
	pdev->frame_count = 0;
	INIT_LIST_HEAD(&(pdev->frame_list));
	INIT_LIST_HEAD(&(pdev->user_list));
	ak_thread_mutex_init(&(pdev->vi_lock));
	ak_thread_mutex_init(&(pdev->frame_lock));

	list_add_tail(&(pdev->dev), &vi_ctrl.dev_list);
	++vi_ctrl.dev_count;

	ak_print_normal("register vi device ok, dev_count=%d\n", vi_ctrl.dev_count);

	return pdev;
}

const char* ak_vi_get_version(void)
{
	return vi_version;
}

/**
 * ak_vi_match_sensor: match sensor according to appointed config file.
 * @config_file[IN]: it can be config file absolutely path, or a directory name.
 * If it is a directory name, it will search config file which format is
 * correct to match.
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_match_sensor(const char *config_file)
{
	int ret = AK_SUCCESS;

	ak_print_normal_ex("config_file: %s\n", config_file);
	if (!config_file) {
		ak_print_error_ex("invalid argument\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	unsigned long sensor_id;

	/*
	 * be sure get sensor id correctlly before call read_isp_conf_init().
	 */
	
	sensor_id = cam_get_sensor_id();
	if (sensor_id == 0) {
		if (false == cam_open())
			return AK_FAILED;
		sensor_id = cam_get_sensor_id();
		ret = read_isp_conf_init(config_file);
		if(ret < 0)
		{
			ak_print_error_ex(" read data from flash to init ISP fail\r\n");		
		}
	}
	
	if (sensor_id == 0) {
		cam_close();
		ret = AK_FAILED;
	}

	return ret;
}

/**
 * ak_vi_open: open video input device
 * @dev[IN]: video input device ID
 * return: 0 success, -1 failed
 * notes:
 */
void* ak_vi_open(enum video_dev_type dev)
{
	/*
	 * module init,
	 * if device not register, create it
	 */
	if (dev >= VIDEO_DEV_NUM) {
		ak_print_normal_ex("Unsupportted dev type: %d\n", dev);
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return NULL;
	}

	ak_print_notice_ex("register device, dev type: %d\n", dev);
	ak_thread_mutex_lock(&vi_ctrl.lock);
	struct vi_device *pdev = vi_register_device(dev);
	if (!pdev) {
		ak_print_error_ex("register video device failed\n");
		ak_thread_mutex_unlock(&vi_ctrl.lock);
		set_error_no(ERROR_TYPE_DEV_OPEN_FAILED);
		return NULL;
	}
	ak_thread_mutex_unlock(&vi_ctrl.lock);

	ak_print_notice_ex("register device OK, pdev: %p\n", pdev);

	/* check if we need to add user */
	struct user_t *user = vi_new_user(pdev, dev);

	return user;
}

/**
 * ak_vi_get_channel_attr: get channel attribution
 * @handle[IN]: opened vi handle
 * @attr[OUT]: channel attribution
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_get_channel_attr(void *handle, struct video_channel_attr *attr)
{
	if (handle == NULL) {
		ak_print_error_ex("Invalid handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return -1;
	}
	if (attr == NULL) {
		ak_print_error_ex("Invalid arguments\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return -1;
	}

	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;

	if (!pdev) {
		ak_print_error_ex("invalid user\n");
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}
	attr->res[VIDEO_CHN_MAIN].width = stream_info.ch1_dstWidth;
	attr->res[VIDEO_CHN_MAIN].height = stream_info.ch1_dstHeight;
	attr->res[VIDEO_CHN_SUB].width = stream_info.ch2_dstWidth;
	attr->res[VIDEO_CHN_SUB].height = stream_info.ch2_dstHeight;

	attr->crop.left = stream_info.crop_left;
	attr->crop.top = stream_info.crop_top;
	attr->crop.width = stream_info.crop_width;
	attr->crop.height = stream_info.crop_height;

	return 0;
}

/**
 * ak_vi_set_channel_attr: set channel attribution
 * @handle[IN]: opened vi handle
 * @attr[IN]: channel attribution
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_set_channel_attr(void *handle,
						const struct video_channel_attr *attr)
{
	if (handle == NULL) {
		ak_print_error_ex("Invalid handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return -1;
	}

	if (attr == NULL) {
		ak_print_error_ex("Invalid arguments\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return -1;
	}

	/* check resolution */
	if (vi_check_channel_attr(attr)) {
		ak_print_error_ex("check attribute argument failed\n");
		return AK_FAILED;
	}
	int ch1_width, ch1_height, ch2_width, ch2_height;
	int ret = AK_SUCCESS;
	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;
	if (!pdev) {
		ak_print_error_ex("invalid user\n");
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

	ak_thread_mutex_lock(&pdev->vi_lock);
	if (!pdev->isp_mem) {
		pdev->isp_mem = isp_3D_NR_create(CHIP_MAIN_CHN_MAX_WIDTH,
				CHIP_MAIN_CHN_MAX_HEIGHT);
		//isp_fps_main();
	}
 	if( pdev->user_ref == 1 )
 	{
		int crop_left, crop_top, crop_width, crop_height;

		ch1_width = attr->res[VIDEO_CHN_MAIN].width;
		ch1_height = attr->res[VIDEO_CHN_MAIN].height;
		ch2_width = attr->res[VIDEO_CHN_SUB].width;
		ch2_height = attr->res[VIDEO_CHN_SUB].height;
		crop_left = attr->crop.left;
		crop_top = attr->crop.top;
		crop_width = attr->crop.width;
		crop_height = attr->crop.height;

		ret =  vi_stream_info_init(ch1_width, ch1_height, ch2_width, ch2_height,
					crop_left, crop_top, crop_width, crop_height);
		
 	}

	ak_thread_mutex_unlock(&pdev->vi_lock);

	return ret;
}

int ak_vi_get_flip_mirror(void *handle, int *flip_enable, int *mirror_enable)
{

	*flip_enable = flip_mirror.flip_en;
	*mirror_enable = flip_mirror.mirror_en;

	return 0;
}

int ak_vi_set_flip_mirror(void *handle, int flip_enable, int mirror_enable)
{
	if (!handle) {
		ak_print_error_ex("invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	struct isp_flip_mirror_info value = {0};

	value.flip_en = flip_enable;
	value.mirror_en = mirror_enable;

	flip_mirror.flip_en = flip_enable;
	flip_mirror.mirror_en = mirror_enable;
	return akisp_ioctl( AK_ISP_SET_FLIP_MIRROR, &value);
}

/**
 * ak_vi_change_channel_attr - change channel attribution
 * @handle[IN]: opened vi handle
 * @attr[IN]: channel attribution
 * return: 0 success, -1 failed
 * notes: IMPORTANT-you can change channel attribution in real time.
 */
int ak_vi_change_channel_attr(const void *handle,
		const struct video_channel_attr *attr)
{
	if (!handle) {
		ak_print_error_ex("invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	int ret;
	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;

	int i;
	const int lost_frame = 3;
	struct video_input_frame frame_lost;

	if (!pdev) {
		ak_print_error_ex("invalid user\n");
		set_error_no(ERROR_TYPE_INVALID_USER);
		return AK_FAILED;
	}

	ak_thread_mutex_lock(&pdev->vi_lock);

	/*
	 * If video resolution switch from high to low, and video flip is enable,
	 * and osd rectangle size equal to old video resolution, we should
	 * close osd first, and then switch video resolution. After that, user
	 * should call osd set rect function to change to a suitable rectangle and
	 * call draw osd to enable osd out put.
	 */
	/* get flip */
	int flip, mirror;
   	ak_vi_get_flip_mirror(user, &flip, &mirror);

	/* when flip = 1, and resolution is high to low, close osd */
	if (flip == 1) {
		ak_print_notice_ex("##### close osd #####\n");

		int origin_main_w, origin_main_h, origin_sub_w, origin_sub_h;
		struct isp_osd_context_attr *osd_context_attr;
		struct vpss_osd_param param = {0};

		/* main channel check */
		param.id = OSD_SET_MAIN_CHANNEL_DATA;
		ak_vpss_osd_get_param(user, &param);
		osd_context_attr = (struct isp_osd_context_attr *)param.data;
		origin_main_w = osd_context_attr->osd_width;
		origin_main_h = osd_context_attr->osd_height;

		if (origin_main_w > attr->res[VIDEO_CHN_MAIN].width
				|| origin_main_h > attr->res[VIDEO_CHN_MAIN].height) {
			param.id = OSD_SET_MAIN_CHANNEL_DATA;
			ak_vpss_osd_close(user, &param);
		}

		/* sub channel check */
		memset(&param, 0, sizeof(struct vpss_osd_param));
		param.id = OSD_SET_SUB_CHANNEL_DATA;
		ak_vpss_osd_get_param(user, &param);
		osd_context_attr = (struct isp_osd_context_attr *)param.data;
		origin_sub_w = osd_context_attr->osd_width;
		origin_sub_h = osd_context_attr->osd_height;

		if (origin_sub_w > attr->res[VIDEO_CHN_SUB].width
				|| origin_sub_h > attr->res[VIDEO_CHN_SUB].height) {
			param.id = OSD_SET_SUB_CHANNEL_DATA;
			ak_vpss_osd_close(user, &param);
		}
	}
	
	camctrl_pause_isp();// isp pause
	
	AK_ISP_CHANNEL_SCALE scale;
	AK_ISP_CHANNEL_SCALE scale_sub;
	AK_ISP_CROP crop;

	scale.width = attr->res[VIDEO_CHN_MAIN].width;
	scale.height = attr->res[VIDEO_CHN_MAIN].height;
	ret += akisp_ioctl( AK_ISP_SET_MAIN_CHANNEL_SCALE, &scale);

	scale_sub.width = attr->res[VIDEO_CHN_SUB].width;
	scale_sub.height = attr->res[VIDEO_CHN_SUB].height;
	ret += akisp_ioctl( AK_ISP_SET_SUB_CHANNEL_SCALE, &scale);

	crop.left = attr->crop.left;
	crop.top = attr->crop.top;
	crop.width = attr->crop.width;
	crop.height = attr->crop.height;
	ret += akisp_ioctl( AK_ISP_SET_CROP, &crop);

	camctrl_resume_isp();// isp resume
	if (AK_SUCCESS != ret) {
		ak_print_error_ex("stream ctrl on error\n");
		ak_thread_mutex_unlock(&pdev->vi_lock);
		set_error_no(ERROR_TYPE_IOCTL_FAILED);
		return AK_FAILED;
	}


	for (i = 0; i < lost_frame;) {
		ret = ak_vi_get_frame(user,&frame_lost);
		if (AK_SUCCESS == ret) {
			i++;
			ak_print_normal_ex("########## release vi ##########\n");
			ak_vi_release_frame(user, &frame_lost);
		}
	}
	ak_thread_mutex_unlock(&pdev->vi_lock);

	return AK_SUCCESS;
}



static int read_isp_conf_init(const char *file_name)
{


#if 1

		int ret;
		unsigned long sensor_id;

		sensor_id = cam_get_sensor_id();
		
		ak_print_notice("===============ID %x\r\n",sensor_id);


#define SENSOR_ID_62               0xa062
#define SENSOR_ID_1135               0x1135
#define SENSOR_ID_1235               0x1235
#define SENSOR_ID_1145               0x1145



		
		switch (sensor_id)
		{
			case SENSOR_ID_62:
				
				ret = isp_conf_init(isp_conf_file_h62, sizeof(isp_conf_file_h62));
				if (ret == false) {
					ak_print_error_ex("isp init fail\n");
					while(1);
				} 
				break;
				
			case SENSOR_ID_1135:
				ret = isp_conf_init(isp_conf_file_1135, sizeof(isp_conf_file_1135));
				if (ret == false) {
					ak_print_error_ex("isp init fail\n");
					while(1);
				} 
				break;

			case SENSOR_ID_1235:
				ret = isp_conf_init(isp_conf_file_1235, sizeof(isp_conf_file_1235));
				if (ret == false) {
					ak_print_error_ex("isp init fail\n");
					while(1);
				} 
				
				break;

			case SENSOR_ID_1145:
				ret = isp_conf_init(isp_conf_file_1145, sizeof(isp_conf_file_1145));
				if (ret == false) {
					ak_print_error_ex("isp init fail\n");
					while(1);
				} 
				break;

			defualt :

				ak_print_error_ex("sensor_id NOT match! \r\n");
				while(1);



		}
		
		

	return 0;

#else

	void *handle = NULL;
	char * tmp = NULL;
	unsigned long data_len,read_len ;
	int i,ret;

	CFGFILE_HEADINFO *p_isp_conf_header;
	unsigned long tmp_len = 0;
	unsigned long sensor_id;
		
	handle = (void *)ak_partition_open(file_name); 
	
	if(handle == NULL)
	{
		ak_print_error("ak_partition_open err!\r\n");
		ret = -1;
		goto ERR;
	}

	data_len = ak_partition_get_dat_size(handle);
	if(0 == data_len)
	{
		ak_print_error_ex("ISPCFG  partition get data fail\r\n");
		ret = -1;
		goto ERR;
	}
	
	tmp = malloc(data_len);
	if(NULL == tmp)
	{
		ak_print_error_ex("isp conf buffer malloc fail\n");
		ret = -4;
		goto ERR;
	}	
		
	ret = ak_partition_read(handle, tmp, data_len);
	
	if((ret < 0) || (ret != data_len))
	{
		ak_print_error("ak_partition_read err! read=%d,real=%d\r\n",data_len,ret);
		ret = -1;
		goto ERR1;
	}

	p_isp_conf_header = (CFGFILE_HEADINFO *)tmp;
	
	sensor_id = cam_get_sensor_id();

	for(i = 0; i < data_len; i = tmp_len)
	{

		ak_print_debug("ID %x,size:%d\r\n",sensor_id,p_isp_conf_header->subFileLen);

		if(sensor_id == p_isp_conf_header->sensorId)
		{
			read_len =  p_isp_conf_header->subFileLen;
			p_isp_conf_header = (CFGFILE_HEADINFO *)(tmp + tmp_len +read_len);
			read_len +=  p_isp_conf_header->subFileLen; 
						
			if(isp_conf_init((unsigned char*)(tmp + tmp_len), read_len))
			{
				ret = 0;
			}
			else
			{
				ak_print_error_ex("isp init fail\r\n");
				ret = -1;
			}
			goto ERR1;
		}
		tmp_len +=  p_isp_conf_header->subFileLen;
		p_isp_conf_header = (CFGFILE_HEADINFO *)(tmp + tmp_len);
		
		tmp_len +=  p_isp_conf_header->subFileLen; //白天和黑夜配置长度有可能不一样
		p_isp_conf_header = (CFGFILE_HEADINFO *)(tmp + tmp_len);

	}
	
	ak_print_error("NO sensor match\r\n");
	ret = -1;	
ERR1:
	free(tmp);
ERR:	
	ak_partition_close(handle);
	return ret;	
#endif
}

/**
 * ak_vi_get_frame: get frame
 * @handle[IN]: return by open
 * @frame[OUT]: store frames
 * return: 0 success, otherwise failed
 */
int ak_vi_get_frame(void *handle, struct video_input_frame *frame)
{
	if (NULL == handle) {
		ak_print_error_ex("Invalid handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return -1;
	}
	if (NULL == frame) {
		ak_print_error_ex("Invalid arguments\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return -1;
	}

	int ret = AK_FAILED;
	// linux struct ak_frame *pos = NULL;
	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;

	if (!pdev) {
		ak_print_error_ex("un register handle: %p\n", user);
		set_error_no(ERROR_TYPE_INVALID_USER);
		goto get_frame_end;
	}
#if 0
	if (!pdev->capture_on) {
		ak_print_error_ex("must call after capture on\n");
		set_error_no(ERROR_TYPE_NOT_INIT);
		goto get_frame_end;
	}
#endif
	ak_thread_mutex_lock(&(pdev->frame_lock));
	int timeout_ms= 2000;
	T_CAMERA_BUFFER *pBuffer;

	if (!isp_init) {
		if (!camstream_init(&stream_info)) {
			ak_print_error_ex("init & start isp fail\n");
			return -1;
		}
		isp_init = 1;
		isp_frame_start = 0;
		isp_fvpr_start = 0;
		isp_conf_fvpr_start();
	}

	while (1) {
		//ak_print_normal("ready.\n");
		
		if (camstream_ready()) {
	
			
			pBuffer = camstream_get();
			// ak_print_normal("gid:%d.\n",pBuffer->id);
			frame->vi_frame[VIDEO_CHN_MAIN].data = stream_info.ch1_YUV[pBuffer->id]->dYUV;
			frame->vi_frame[VIDEO_CHN_MAIN].len = stream_info.ch1_dstWidth * stream_info.ch1_dstHeight * 3 / 2;
			frame->vi_frame[VIDEO_CHN_MAIN].ts = pBuffer->ts;
			frame->vi_frame[VIDEO_CHN_SUB].data = stream_info.ch2_YUV[pBuffer->id]->dYUV;
			frame->vi_frame[VIDEO_CHN_SUB].len = stream_info.ch2_dstWidth * stream_info.ch2_dstHeight * 3 / 2;
			frame->vi_frame[VIDEO_CHN_SUB].ts = pBuffer->ts;
			#if 0 
			/*
			  *跳过sensor启动开始前5帧，前5帧图像偏红
			  *后65帧由暗转明的过程
			  *ymx : 2017.3.6
			  */
			
			if(isp_frame_start <= SKIP_FRAME_COUNT)
			{
				isp_frame_start++;	
				ak_vi_release_frame(handle,frame);
				continue;
			}
			#endif
			if (0 == pBuffer->ts) {
				ak_print_notice("ts=0\n");
				ak_thread_mutex_unlock(&(pdev->frame_lock));
				ak_vi_release_frame(handle,frame);
				ak_thread_mutex_lock(&(pdev->frame_lock));
				continue;
			}

			if (isp_fvpr_start < FVPR_FRAME_COUNT) {
				isp_fvpr_start++;
				if (isp_fvpr_start >= FVPR_FRAME_COUNT) {
					isp_conf_fvpr_finish();
				}
			}
			ak_thread_mutex_unlock(&(pdev->frame_lock));
			return 0;
		}

		if (timeout_ms <= 0) {
			break;
		} else {
			ak_sleep_ms(2);
			timeout_ms -= 2;
		}
		
	}
	ak_thread_mutex_unlock(&(pdev->frame_lock));
get_frame_end:	
	return ret;
}

/**
 * ak_vi_release_frame - release frame buffer after used
 * @pbuf[IN]: stream data buffer begin address
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_vi_release_frame(void *handle, struct video_input_frame *frame)
{
	if (NULL == handle) {
		ak_print_error_ex("Invalid handle\n");
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return -1;
	}

	if (NULL == frame) {
		ak_print_error_ex("Invalid arguments\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return -1;
	}

	int ret = AK_FAILED;
	//unsigned char found = AK_FALSE;
	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;
	//struct ak_frame *pos = NULL;
	//struct ak_frame *n = NULL;

	ak_thread_mutex_lock(&(pdev->frame_lock));
	int i;
	for (i = 0; i < 4; i++)
		if (stream_info.ch1_YUV[i]->dYUV == frame->vi_frame[VIDEO_CHN_MAIN].data)
			break;

	if (i >= 4) {
		ak_print_error_ex("un-recognized frame\n");
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_thread_mutex_unlock(&(pdev->frame_lock));
		return -1;
	}

	if(old_i == -1 )
	{
		// printk("ri:%d\n",i);
		camstream_queue_buffer(i);
		old_i = i;		
	}else if(i == (old_i + 1)%4){

		// printk("ri2:%d\n",i);
		camstream_queue_buffer(i);
		old_i = i;

		int next_buf_id = (old_i + 1)%4;
		while(old_save_i_count > 0)
		{
			if( old_save_arr[next_buf_id])
			{
				
					camstream_queue_buffer(next_buf_id);
					old_i = next_buf_id;
					old_save_arr[next_buf_id] = 0;
					old_save_i_count --;

					next_buf_id = (old_i + 1)%4;
					continue;
			}else{
				break;
			}
		}

	}else {
		
		// printk("rsi:%d\n",i);
		old_save_arr[i] = 1;
		old_save_i_count++;
		
	}
	ak_thread_mutex_unlock(&(pdev->frame_lock));
	return 0;
}

/**
 * ak_vi_get_fps - get current capture fps
 * @handle[IN]: video opened handle
 * return: fps value
 * notes:
 */
int ak_vi_get_fps(void *handle)
{
	if (!handle) {
		ak_print_error_ex("invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}

	return cam_get_sensor_fps();
}

/**
 * ak_vi_set_fps - set capture fps
 * @handle[IN]: video opened handle
 * @fps[IN]: the fps value to be set
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_set_fps(void *handle, int fps)
{
	if (!handle) {
		ak_print_error_ex("invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	bool ret; 
	
	ret = cam_set_manual_switch_fps(fps);
	if (true == ret)
		return 0;
	return -1;
}

/**
 * ak_vi_switch_mode - switch day/night mode
 * @handle[IN]: video opened handle
 * @mode[IN]:
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_switch_mode(void *handle, enum video_daynight_mode mode)
{
	if (!handle) {
		ak_print_error_ex("invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	bool ret;

	switch (mode) {
		case VI_MODE_DAYTIME:
			ret = isp_conf_set_day();
			break;
		case VI_MODE_NIGHTTIME:
			ret = isp_conf_set_night();
			break;
		default:
			ret = false;
			ak_print_error_ex("don't defined mode:%d\n", mode);
			break;
	}

	if (ret == true)
		return 0;
	return -1;
}

/**
 * ak_vi_get_sensor_resolution - get sensor max resolution supported
 * @handle[IN]: video opened handle
 * @res[OUT]:
 * return: 0 success, -1 failed
 * notes:
 */
int ak_vi_get_sensor_resolution(void *handle, struct video_resolution *res)
{
	if (handle == NULL) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	if (NULL == res) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		return AK_FAILED;
	}
	
	bool ret; 
	
	ret = cam_get_sensor_resolution(&(res->height), &(res->width));
	if (true == ret)
	{
		return 0;
	}
	return -1;
}
int ak_vi_set_switch_fps_enable(void *handle, int enable)
{
	if (!handle) {
		ak_print_error_ex("invalid handle: %p\n", handle);
		set_error_no(ERROR_TYPE_POINTER_NULL);
		return AK_FAILED;
	}
	bool ret;
	int fps;

	fps = enable ? -1:0;
	ret = cam_set_manual_switch_fps(fps);
	if (true == ret)
		return 0;
	return -1;
}

void* ak_vi_get_handle(enum video_dev_type dev)
{
	/* 1. get device */
	struct vi_device *pdev = vi_get_device(dev);
	if (!pdev) {
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_normal("the dev=%d had not opened\n", dev);
		return NULL;
	}

	/* 2. get handle by device */
	void *handle = NULL;
	struct user_t *user = NULL;

	list_for_each_entry(user, &pdev->user_list, list) {
		if (user->dev_num == dev) {
			handle = (void *)user;
			break;
		}
	}

	return handle;
}

enum video_work_scene ak_vi_get_work_scene(enum video_dev_type dev)
{
	struct vi_device *pdev = vi_get_device(dev);
	if (!pdev) {
		ak_print_normal("the dev=%d had not opened\n", dev);
		return VIDEO_SCENE_UNKNOWN;
	}

	int scene = akisp_ioctl(AK_ISP_GET_WORK_SCENE, &scene);
	if ((scene < VIDEO_SCENE_INDOOR) || (scene > VIDEO_SCENE_OUTDOOR)) {
		scene = VIDEO_SCENE_INDOOR;
	}

	return scene;
}

/**
 * ak_vi_close -
 * @handle[IN]:return by ak vi open
 * return: 0 success, otherwise failed
 * notes:
 */
int ak_vi_close(void *handle)
{
	ak_print_notice_ex("enter close vi\n");

	if (handle == NULL) {
		set_error_no(ERROR_TYPE_POINTER_NULL);
		ak_print_error_ex("invalid arguments\n");
		return AK_FAILED;
	}

	int ret = AK_SUCCESS;
	struct user_t *user = (struct user_t *)handle;
	struct vi_device *pdev = user->pdev;

	if (!pdev) {
		set_error_no(ERROR_TYPE_INVALID_USER);
		ak_print_error_ex("un init user: %p\n", user);
		return AK_FAILED;
	}

	/* release vi device data */
	ak_thread_mutex_lock(&vi_ctrl.lock);
	del_ref(&(pdev->user_ref), 1);
	ak_print_notice_ex("user_ref=%d\n", pdev->user_ref);
	if (pdev->user_ref <= 0) {
		ret = vi_unregister_device(pdev);
	}	

	/* release vi user data */
	list_del_init(&user->list);
	free(user);
	user = NULL;
	ak_thread_mutex_unlock(&vi_ctrl.lock);
	ak_print_normal_ex("leave\n");

	return ret;
}

