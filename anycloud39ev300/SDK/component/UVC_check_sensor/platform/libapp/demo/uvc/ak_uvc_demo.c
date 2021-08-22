#include "anyka_types.h"
#include "ak_drv_uvc.h"
#include "ak_vi.h"
#include "command.h"
#include "ak_common.h"
#include "ak_drv_ircut.h"
#include "kernel.h"
#include "ak_venc.h"
#include "ak_vpss.h"
#define CH1_WIDTH	1280
#define CH1_HEIGHT	720
#define CH2_WIDTH	640
#define CH2_HEIGHT	480

volatile static bool s_break = false;

static char *help[]={
	"uvc module demo",
	"usage:uvcdemo [yuv/mjpeg]\n"
};

static void cmd_uvc_signal(int signo)
{
    s_break = true;
}

static void *init_video_in(void)
{
	void *handle;

	/* open isp sdk */
	handle = ak_vi_open(VIDEO_DEV0);
	if (NULL == handle) {
		ak_print_error_ex("##########$$  ak_vi_open failed!\n");
		return NULL;
	} else {
		ak_print_normal("##$$  ak_vi_open success!--handle=%d\n",(int)handle);
	}

	/* get sensor resolution */
	struct video_resolution sensor_res = {0};
	if (ak_vi_get_sensor_resolution(handle, &sensor_res))
		ak_print_error_ex("ak_mpi_vi_get_sensor_res failed!\n");
	else
		ak_print_normal("ak_mpi_vi_get_sensor_res ok! w:%d, h:%d\n",
				sensor_res.width, sensor_res.height);

	/* set crop information */
	struct video_channel_attr obj_set_attr = {0};
	struct video_channel_attr *attr  = &obj_set_attr;

	attr->res[VIDEO_CHN_MAIN].width = CH1_WIDTH;
	attr->res[VIDEO_CHN_MAIN].height = CH1_HEIGHT;

	attr->res[VIDEO_CHN_SUB].width = CH2_WIDTH;
	attr->res[VIDEO_CHN_SUB].height = CH2_HEIGHT;
	attr->crop.left = 0;
	attr->crop.top = 0;
	attr->crop.width = sensor_res.width;
	attr->crop.height = sensor_res.height;

	if (ak_vi_set_channel_attr(handle, attr)) {
		ak_print_error_ex("ak_vi_set_channel_attr failed!\n");
	} else {
		ak_print_normal("ak_vi_set_channel_attr success!\n");
	}

	struct video_channel_attr obj_get_attr = {0};
	struct video_channel_attr *cur_attr = &obj_get_attr;

	if (ak_vi_get_channel_attr(handle, cur_attr)) {
		ak_print_error_ex("ak_vi_get_channel_attr failed!\n");
	}

	ak_print_normal("ak_vi_get_channel_attr =%d!\n",cur_attr->res[VIDEO_CHN_MAIN].width);
	/* set fps */
	ak_vi_set_fps(handle, 25);

	/* get fps */
	int fps = 0;
	fps = ak_vi_get_fps(handle);
	ak_print_normal("fps = %d\n",fps);

	return handle;
}

static void *init_video_encode(void)
{
	struct encode_param *param = (struct encode_param *)calloc(1,
			sizeof(struct encode_param));


	param->width = CH1_WIDTH;
	param->height = CH1_HEIGHT;
	param->minqp = 20;
	param->maxqp = 51;
	param->fps = 25;
	param->goplen = param->fps * 2;
	param->bps = 1500; 
	param->profile = PROFILE_MAIN;
	if (param->width <=640)
	    param->use_chn = ENCODE_SUB_CHN ; 
	else
    	param->use_chn = ENCODE_MAIN_CHN;
	param->enc_grp = ENCODE_RECORD;
	param->br_mode = BR_MODE_CBR;
	param->enc_out_type = MJPEG_ENC_TYPE;
	void * encode_video = ak_venc_open(param);
	ak_print_normal("YMX : free 2: size = %d\n", sizeof(struct encode_param));
	free(param);
	return encode_video;
}

int check_uvc_send_status(void)
{
    unsigned long t1, t2, offset;

    t1 = get_tick_count();
    while(ak_drv_uvc_wait_stream())
    {
        if(s_break)
            return -1;

        t2 = get_tick_count();
        offset = (t2 >= t1) ? (t2- t1) : (t2 + 0xFFFFFFFF - t1);
        if(offset > 500){
            printf("[uvc timeout: %d]\n", offset);
            return -1;
        }
    }

    return 0;
}

static void handle_mjpeg_stream(void *vi_handle, void *enc_handle)
{
    int ret, i = 0;
	void *stream_handle;
    struct video_stream video_stream = {0};
    struct uvc_stream uvc_stream = {0};

	stream_handle = ak_venc_request_stream(vi_handle, enc_handle);
	if (stream_handle == NULL) {
		ak_print_error_ex("request stream failed\n");
		return;
	}

	while(1)
	{
		ret = ak_venc_get_stream(stream_handle, &video_stream);
		if (ret != 0) {
			ak_sleep_ms(3);
			continue;
		}

        if(check_uvc_send_status() < 0)
        {
            ak_venc_release_stream(stream_handle, &video_stream);
            break;
		}
		
		uvc_stream.data = video_stream.data;
		uvc_stream.len = video_stream.len;

		ak_drv_uvc_send_stream(&uvc_stream);
        
		i++;
		if(i == 1)
			if(ak_vpss_effect_set(vi_handle,VPSS_POWER_HZ,50))
				ak_print_error("set hz error\n");
			else
				ak_print_normal("set hz ok\n");
		//ak_print_normal("[%d]get stream, size: %d, ts=%d\n",
		//	 i, video_stream.len, video_stream.ts);
            
		ret = ak_venc_release_stream(stream_handle, &video_stream);
		if (ret != 0) {
			ak_print_error_ex("release stream failed\n");
		}
	}
	
	ak_print_normal_ex("cancel stream ...\n");
	ak_venc_cancel_stream(stream_handle);
}

static void chang_attr(void *vi_handle)
{
	struct video_channel_attr attr;
	ak_vi_get_channel_attr(vi_handle, &attr);
	attr.res[0].width= 640;
	attr.res[0].height = 480;
	int ret = ak_vi_change_channel_attr(vi_handle, &attr);
	if(ret == 0)
		ak_print_notice("ymx: %s.success.\n",__func__);
	else
		ak_print_notice("ymx: %s.fail.\n",__func__);
}
static void chang_attr2(void *vi_handle)
{
	struct video_channel_attr attr;
	ak_vi_get_channel_attr(vi_handle, &attr);
	attr.res[0].width= 1280;
	attr.res[0].height = 720;
	int ret = ak_vi_change_channel_attr(vi_handle, &attr);
	if(ret == 0)
		ak_print_notice("ymx: %s.success.\n",__func__);
	else
		ak_print_notice("ymx: %s.fail.\n",__func__);
}

static void handle_yuv_stream(void *vi_handle)
{
	int i=0;
    int ret;
    struct uvc_stream stream;
    struct video_input_frame frame;

	while(1)
	{
		ret = ak_vi_get_frame(vi_handle, &frame);
		if (ret != 0) {
			ak_print_error_ex("get frame fail\n");
			continue;
		}
		ak_print_normal("get frame[%d] ok\n",i++);
			
		stream.data = frame.vi_frame[VIDEO_CHN_MAIN].data;
		stream.len = frame.vi_frame[VIDEO_CHN_MAIN].len;

        if(check_uvc_send_status() < 0)
        {
            ak_vi_release_frame(vi_handle, &frame);
            break;
        }
        
		ak_drv_uvc_send_stream(&stream);

		ak_vi_release_frame(vi_handle, &frame);
#if 1
		if(i == 30)
		{
			chang_attr(vi_handle);
		}

		if(i == 60)
			chang_attr2(vi_handle);
#endif
	}
}

void cmd_uvc_demo(int argc, char **args)
{
	void *vi_handle = NULL, *venc_handle = NULL;
	int i;
	char *main_addr = "ISPCFG";
    enum stream_type uvc_type; 

#if 1
	uvc_type = STREAM_MJPEG;
	s_break = false;
#else
	if (argc != 1)
	{
		ak_print_error("%s",help[1]);
		return ;
	}else
	{
		if (strcmp(args[0],"yuv") ==0)
		{
            uvc_type = STREAM_YUV;
		}
		else if (strcmp(args[0],"mjpeg")==0)
		{
            uvc_type = STREAM_MJPEG;
			
		}else
		{
			ak_print_error("%s",help[1]);
			return ;
		}
	}

    s_break = false;

    //register signal
    cmd_signal(CMD_SIGTERM, cmd_uvc_signal);



#endif
	//sensor init
	ak_drv_ir_init();
	ak_drv_ir_set_ircut(IRCUT_STATUS_DAY);
	ak_print_normal("set ircut day\n");

	if (ak_vi_match_sensor(main_addr)) {
		ak_print_error_ex("##########$$  match sensor main_addr failed!\n");

		return;
	} else {
		ak_print_normal("##########$$  ak_vi_match_sensor success!\n");
	}

    //vi venc init
	vi_handle = init_video_in();
    if(NULL == vi_handle)
        goto EXIT;
    
	if(uvc_type == STREAM_MJPEG)
	{
        venc_handle = init_video_encode();
        if(NULL == venc_handle)
            goto EXIT;
    }
	/* start isp tool service */
	ak_its_start();
	
    //uvc start
	ak_drv_uvc_start(CH1_WIDTH, CH1_HEIGHT, uvc_type);

    //wait pc open
    while (ak_drv_uvc_wait_stream() && !s_break);

	/*******************thread works start *****************************/
	if(uvc_type == STREAM_MJPEG)
        handle_mjpeg_stream(vi_handle, venc_handle);
    else
        handle_yuv_stream(vi_handle);
	/*******************thread work send *****************************/

	ak_drv_uvc_stop();

EXIT:
	/* stop isp tool service */
	ak_its_stop();
	ak_venc_close(venc_handle);
	ak_vi_close(vi_handle);
}

static int cmd_uvc_demo_reg(void)
{
    cmd_register("uvcdemo", cmd_uvc_demo, help);
    return 0;
}

cmd_module_init(cmd_uvc_demo_reg)
