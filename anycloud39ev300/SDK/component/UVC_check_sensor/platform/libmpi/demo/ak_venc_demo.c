#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ak_thread.h"
#include "ak_common.h"
#include "ak_vi.h"
#include "ak_venc.h"
#include "drv_api.h"
#include "ak_drv_ircut.h"
#include "kernel.h"



#define FIRST_PATH       "ISPCFG"
#define BACK_PATH        "ISPCFG"



static char *help[]={
	"encode module demo",
	"usage:vencdemo [framecount] <vga/720p/960p> <fps> <bitrate>\n"
	"      please insert t card , vencdemo will save data to a:/xxx.bin\n"
};

int enc_count;	//encode this number frames

struct  Venc_Demo_Para
{
    int frame_count;
    int width;
    int height;
    int fps;
    int kbps;
};

static struct Venc_Demo_Para g_rec_para;

static bool is_vga = false;

static void *open_file()
{
	char file[100] = {0};
    
    struct ak_date systime;
	ak_get_localdate(&systime);
    sprintf(file, "a:/venc_%04d%02d%02d%02d%02d%02d.h264"
        ,systime.year, systime.month, systime.day, 
        systime.hour, systime.minute, systime.second);

    ak_print_normal("file name=%s\n",file);
    
	FILE *filp = NULL;
	filp = fopen(file, "w");
	if (!filp) {
		ak_print_error_ex("fopen error\n");
		return NULL;
	}

	return filp;
}

static void close_file(FILE *filp)
{
	fclose(filp);
}

void save_stream(FILE *filp, unsigned char *data, int len)
{
	int ret = len;
	
	do {
		ret -= fwrite(data, 1, ret, filp);
	} while (ret > 0);
}

void *video_encode_init()
{
	struct encode_param *param = (struct encode_param *)calloc(1,
			sizeof(struct encode_param));


	param->width = g_rec_para.width;
	param->height = g_rec_para.height;
	param->minqp = 20;
	param->maxqp = 51;
	param->fps = g_rec_para.fps;
	param->goplen = param->fps * 2;
	param->bps = g_rec_para.kbps ; 
	param->profile = PROFILE_MAIN;
	if (param->width <=640)
	    param->use_chn = ENCODE_SUB_CHN ; 
	else
    	param->use_chn = ENCODE_MAIN_CHN;
	param->enc_grp = ENCODE_RECORD;
	param->br_mode = BR_MODE_CBR;
	param->enc_out_type = H264_ENC_TYPE;
	void * encode_video = ak_venc_open(param);
	ak_print_normal("YMX : free 2: size = %d\n", sizeof(struct encode_param));
	free(param);
	return encode_video;
}

static int  set_dayornight_cfg(void * vi_handle)
{
    int status;
    int fd;
    int ret;

	ret = ak_drv_ir_init();
	if (0 != ret) {
		ak_print_error_ex("ircut init fail\n");
		return -1;
	}
	
	status = ak_drv_ir_get_input_level();
	
	if(status < 0)
	{
		ak_print_error_ex("ak_drv_ir_get_input_level fail\n");
		return -1;
	}
	
	 
    if (status == 0) {
        ak_print_notice("set day isp config and ircut!\n");

        ak_drv_ir_set_ircut(IRCUT_STATUS_DAY);
        ak_vi_switch_mode(vi_handle, VI_MODE_DAYTIME); 
    } else {
        ak_print_notice("set night isp config and ircut!\n");
        ak_vi_switch_mode(vi_handle, VI_MODE_NIGHTTIME); 
        ak_drv_ir_set_ircut(IRCUT_STATUS_NIGHT);
    }
    
    return 0;
}

void *video_input_init(const char *first, const char *second)
{

    unsigned short exptime = 896;
    cam_set_sensor_ae_info(&(exptime));

    if (ak_vi_match_sensor(first) < 0)
    {
        ak_print_error_ex("match sensor failed\n");
        return NULL;
    }
    

	/* open device */
	void *handle = ak_vi_open(VIDEO_DEV0);
	if (handle == NULL) {
		ak_print_error_ex("vi open failed\n");
		return NULL;
	}

    if (0 != set_dayornight_cfg(handle))
    {
        ak_vi_close(handle);
        return NULL;
    }


	/* get camera resolution */
	struct video_resolution resolution = {0};
	if (ak_vi_get_sensor_resolution(handle, &resolution))
	{
		ak_print_error_ex("get sensor resolution failed\n");
        ak_vi_close(handle);
        return NULL;
	}	
	else
		ak_print_normal("sensor resolution height:%d,width:%d.\n",resolution.height,resolution.width);

    //judge resolution and user need
    if (g_rec_para.width > resolution.width || g_rec_para.height > resolution.height )
    {
        ak_print_error("sensor resolution is too smaller!\n"
                       "request video width=%d,height=%d;sensor width=%d,height=%d.\n"
                       ,g_rec_para.width,g_rec_para.height,resolution.width,resolution.height);
        ak_vi_close(handle);
        return NULL;
    }
    
	/* set crop information */
	struct video_channel_attr attr;

    //set default crop
	attr.crop.left = 0;
	attr.crop.top = 0;
	attr.crop.width = resolution.width;
	attr.crop.height = resolution.height;

    //set channel default pixel
	attr.res[VIDEO_CHN_MAIN].width = resolution.width;
	attr.res[VIDEO_CHN_MAIN].height = resolution.height;
	attr.res[VIDEO_CHN_SUB].width = 640;
	attr.res[VIDEO_CHN_SUB].height= 360;

    //change pixerl by user need
    if (g_rec_para.width >640 ) //use  main channel
    {
    	attr.res[VIDEO_CHN_MAIN].width = g_rec_para.width;
    	attr.res[VIDEO_CHN_MAIN].height = g_rec_para.height;
    }else  //use sub channel
    {
    	attr.res[VIDEO_CHN_SUB].width  = g_rec_para.width;
    	attr.res[VIDEO_CHN_SUB].height = g_rec_para.height;

        //change vga height if sensor height is 960 or 480
    	if (g_rec_para.height==360 &&  
    	    (resolution.height == 960 || resolution.height==480))
        {    	    
        	attr.res[VIDEO_CHN_SUB].height = 480;
            g_rec_para.height    =480;
        }
        
    }
	
	if (ak_vi_set_channel_attr(handle, &attr))
		ak_print_error_ex("set channel attribute failed\n");

	/* get crop */
	struct video_channel_attr cur_attr ;
	if (ak_vi_get_channel_attr(handle, &cur_attr)) {
		ak_print_normal("ak_vi_get_channel_attr failed!\n");
	}else
	{

        ak_print_normal("channel crop:left=%d,top=%d, width=%d,height=%d\n"
            ,cur_attr.crop.left,cur_attr.crop.top,cur_attr.crop.width,cur_attr.crop.height);

        ak_print_normal("channel pixel:main width=%d,height=%d;sub width=%d,height=%d\n"
            ,attr.res[VIDEO_CHN_MAIN].width,attr.res[VIDEO_CHN_MAIN].height
            ,attr.res[VIDEO_CHN_SUB].width,attr.res[VIDEO_CHN_SUB].height);

    }        
	
	/* don't set camera fps in demo */
//	ak_vi_set_fps(handle, 25);
	ak_print_normal("capture fps: %d\n", ak_vi_get_fps(handle));

	return handle;
}

void *work_encode(void *arg)
{
//	int enc_ch = *(int *)arg;
	void *vi_handle, *venc_handle, *stream_handle;
	int looptimes = g_rec_para.frame_count;

	ak_print_normal_ex("start a work encode\n");

	vi_handle = video_input_init(FIRST_PATH, BACK_PATH);
	if (vi_handle == NULL) {
		ak_print_error_ex("video input init faild, exit\n");
        ak_thread_exit();
        return NULL;
	}

	
	venc_handle = video_encode_init();
	if (venc_handle == NULL) {
		ak_print_error_ex("video encode open failed!");
		goto exit1;
	}
    
	FILE *fp = open_file();
	if (fp ==NULL)
	{
		ak_print_error_ex("open file failed!");
        goto exit2;	    
	}
    
#if 1
	stream_handle = ak_venc_request_stream(vi_handle,venc_handle );
	if (stream_handle == NULL) {
		ak_print_error_ex("request stream failed\n");
		goto exit;
	}

	while (looptimes > 0) {
		struct video_stream stream = {0};
		int ret = ak_venc_get_stream(stream_handle, &stream);
		if (ret != 0) {
			//ak_print_error_ex("get stream failed\n");
			ak_sleep_ms(3);
			continue;
		}

		ak_print_normal("[%d]get stream, size: %d, type=%d, ts=%d\n",
			 looptimes, stream.len, stream.frame_type, stream.ts);
		save_stream(fp, stream.data, stream.len);
            
		ret = ak_venc_release_stream(stream_handle, &stream);
		if (ret != 0) {
			ak_print_error_ex("release stream failed\n");
		}
		looptimes--;
	}
	ak_print_normal_ex("cancel stream ...\n");
	ak_venc_cancel_stream(stream_handle);
#else
    struct video_input_frame frame;
    struct video_stream stream;
    
	while (looptimes > 0) {
        
        if (0 == ak_vi_get_frame(vi_handle, &frame))
        {
            ak_print_debug("ak_venc_send_frame:enc_handle=%x, buf=%x\n", venc_handle, frame.vi_frame[0].data);
            if (0 == ak_venc_send_frame(venc_handle, frame.vi_frame[0].data, frame.vi_frame[0].len, &stream))
            {
                ak_print_normal("frame type=%d, sz=%d, ts=%d\n", stream.frame_type, stream.len, frame.vi_frame[0].ts);
                save_stream(fp, stream.data, stream.len);
            }
            else
                ak_print_error("encode failed!\n");
    
            ak_vi_release_frame(vi_handle, &frame);
        }
        else
            ak_print_error("get frame failed!\n");
        
		looptimes--;
	}

#endif

exit:
	close_file(fp);
exit2:
	ak_venc_close(venc_handle);
exit1:	
	ak_vi_close(vi_handle);
	
    ak_thread_exit();
    
	return NULL;
}

static int init_param(int argc, char **args )
{
    int i;
    
    g_rec_para.frame_count =10;
    g_rec_para.width = 1280;
    g_rec_para.height =720;
    g_rec_para.fps =25;
    g_rec_para.kbps =1500;

    if (argc <1 || argc > 4)
    {
        ak_print_error("%s\n", help[1]);
        return -1;
    }

	is_vga = false;
    for(i=0 ;i< argc;i++)
    {
        switch(i)
        {
            case 0:
                //save frame count
                g_rec_para.frame_count= atoi(args[0]);
                if (g_rec_para.frame_count<=0)
                {
                    ak_print_error("[framecount] para error![framecount] must be >0\n");
                    return -1;
                }
               break;
            case 1:            
                //definition
                if (0 == strcmp(args[1], "vga") )
                {
                    g_rec_para.width = 640;
                    g_rec_para.height =360;
                    g_rec_para.fps = 10 ;
                    g_rec_para.kbps = 512;
                    is_vga = true;
                }else if (0 == strcmp(args[1], "720p") )
                {
                    g_rec_para.width = 1280;
                    g_rec_para.height = 720;
                    g_rec_para.fps =  25;
                    g_rec_para.kbps = 2048;
                    
                }else if (0 == strcmp(args[1], "960p") )
                {
                    g_rec_para.width = 1280;
                    g_rec_para.height = 960;
                    g_rec_para.fps =  25;
                    g_rec_para.kbps = 3072;
                }else
                {
                    ak_print_error("pixel para error!\n%s\n",help[1]);
                    return -1;
                }
	
                break;
            case 2:            
                //fps
                g_rec_para.fps= atoi(args[2]);
                if (!(g_rec_para.fps>=1 && g_rec_para.fps <=25))
                {
                    ak_print_error("<fps> para error!fps must be >=1 and <=25.\n");
                    return -1;
                    
                }
                break;
                
            case 3:
                 //bitrate
                g_rec_para.kbps= atoi(args[3]);
                if (!(g_rec_para.kbps>=1 && g_rec_para.kbps <=4096) )
                {
                    ak_print_error("bitrate para error!bitrate must be >=1 and <=25.\n");
                    return -1;
                 }
                 break;
            
        }
    }

    ak_print_normal("input para frame_count=%d,width=%d,height=%d,fps=%d,kbps=%d\n"
        , g_rec_para.frame_count,g_rec_para.width,g_rec_para.height
        , g_rec_para.fps,g_rec_para.kbps);
    
    return 0;
}
void cmd_encode_demo(int argc, char **args)
{
	ak_pthread_t  encode_id;

    if (0 != init_param(argc, args))
        return ;

	if (0 !=ak_mount_fs(DEV_MMCBLOCK, 0, "a:") )
	{
	    ak_print_error("sd card mount failed!\n");
	    return ;
	}


	ak_thread_create(&encode_id,  work_encode, 0, 32*1024, 50);
    
	ak_print_normal("main encode wait work encode exit ...\n");
	ak_thread_join(encode_id);
	
	ak_unmount_fs(DEV_MMCBLOCK, 0, "a:");
	
}


static int cmd_encode_reg(void)
{
    cmd_register("vencdemo", cmd_encode_demo, help);
    return 0;
}

cmd_module_init(cmd_encode_reg)


