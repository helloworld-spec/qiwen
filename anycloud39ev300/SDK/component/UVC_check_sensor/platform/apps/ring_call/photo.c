/*******************************************************************
 * @brief  a simple example to take photo.
           cmd_photo_demo is the entrance function to the shell command
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-4 create 
*******************************************************************/
 
#include <stdio.h>
#include <string.h>

#include "command.h"
#include "drv_api.h"

#include "ak_common.h"
#include "ak_vi.h"
#include "ak_venc.h"
#include "ak_drv_ircut.h"

#include "kernel.h"
#include "photo.h"



#define PHOTO_SAVE_2_SD     0


struct  photo_demo_para
{
    int width;
    int height;
    int contrast;
    int brightness;
    int saturation;
    int osd_pos;
    int vi_chn;
};


static struct photo_demo_para g_demo_para;
static void *vi_handle;
static void *venc_handle;
static struct video_stream *stream_handle = NULL;


/**
 * @brief   initialize the user command parameter
 *
 * @param   argc[in] the amount of command parameters excluding the command itself
 * @param   args[in] the command parameters
 * @return  0:success , -1:fail
 */
static int init_para(int pixel)
{
    int i;

    memset(&g_demo_para, 0, sizeof(g_demo_para));
    g_demo_para.width   =1280;
    g_demo_para.height  =720;

    if (PHOTO_PIXEL_VGA == pixel )
   {
       g_demo_para.width = 640;
       g_demo_para.height =360;
    }else if (PHOTO_PIXEL_720P == pixel )
   {
       g_demo_para.width = 1280;
       g_demo_para.height = 720;
        
   }else
   {
       ak_print_error("pixel para error!\n%d\n",pixel);
       return -1;
   }

    return 0;
}


/**
 * @brief   set the ircut and isp config by day or night detecting
 *
 * @param   vi_handle[in] video input handle
 * @return  0: success , -1: fail
 */
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


/**
 * @brief   initialize the video input handle
 *
 * @param   no
 * @return  the video input handle
 */
static void *video_input_init(void)
{
    
    if (ak_vi_match_sensor("ISPCFG") < 0)
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
    if (g_demo_para.width > resolution.width || g_demo_para.height > resolution.height )
    {
        ak_print_error("sensor resolution is too smaller!\n"
                       "request video width=%d,height=%d;sensor width=%d,height=%d.\n"
                       ,g_demo_para.width,g_demo_para.height,resolution.width,resolution.height);
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
    if (g_demo_para.width >640 ) //use  main channel
    {
    	g_demo_para.vi_chn = VIDEO_CHN_MAIN;
    	attr.res[VIDEO_CHN_MAIN].width = g_demo_para.width;
    	attr.res[VIDEO_CHN_MAIN].height = g_demo_para.height;
    }else  //use sub channel
    {
    	g_demo_para.vi_chn = VIDEO_CHN_SUB;
    	attr.res[VIDEO_CHN_SUB].width  = g_demo_para.width;
    	attr.res[VIDEO_CHN_SUB].height = g_demo_para.height;

        //change vga height if sensor height is 960 or 480
    	if (g_demo_para.height==360 &&  
    	    (resolution.height == 960 || resolution.height==480))
        {    	    
        	attr.res[VIDEO_CHN_SUB].height = 480;
            g_demo_para.height    =480;
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


/**
 * @brief   initialize the video encoding handle
 *
 * @param   no
 * @return  the video encoding handle
 */
static void *video_encode_init()
{
	struct encode_param param ;


    memset(&param, 0 , sizeof(param));
	param.width = g_demo_para.width;
	param.height = g_demo_para.height;

    if (param.width <=640)
        param.use_chn = ENCODE_SUB_CHN;
    else
        param.use_chn = ENCODE_MAIN_CHN;
    
	param.enc_out_type = MJPEG_ENC_TYPE;
    param.enc_grp = ENCODE_PICTURE;
	
	void * encode_video = ak_venc_open(&param);
	
	return encode_video;
}

#if PHOTO_SAVE_2_SD

/**
 * @brief   open a file to write a photo data
 *
 * @param   no
 * @return  the file handle
 */
static void *open_file(void)
{
	char file[100] = {0};
    struct ak_date systime;
    FILE * fDir;        

    fDir = fopen("a:/photo","r");
    if (NULL == fDir )
    {
        ak_print_normal("photo dir don't exists!");
        if (0 != mkdir("a:/photo",0))
            return NULL;
        
    }else
    {
        ak_print_normal("photo dir exists!");
        fclose(fDir);
    }
        

    ak_get_localdate(&systime);
    sprintf(file, "a:/photo/IMG_%04d%02d%02d%02d%02d%02d.jpg"
        ,systime.year, systime.month+1, systime.day+1, 
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


/**
 * @brief   close a file
 *
 * @param   filp[in] the file handle
 * @return  no
 */
static void close_file(FILE *filp)
{
	fclose(filp);
}
#endif
/**
 * @brief   save data to a file
 *
 * @param   filp[in] the file handle
 * @param   data[in] the data address to write
 * @param   len[in] the data size to write
 * @return  no
 */
static void save_stream(FILE *filp, unsigned char *data, int len)
{
	int ret = len;
	
	do {
		ret -= fwrite(data, 1, ret, filp);
	} while (ret > 0);
}


/**
 * @brief   the main process to take a photo
 *
 * @param   no
 * @return  no
 */
static int encode_jpg(void)
{

#if PHOTO_SAVE_2_SD
    //open a file to save photos
	FILE *fp = open_file();
	if (fp == NULL)
	{
		ak_print_error_ex("open file failed!");
        return 0;	    
	}
#endif

    int skip_count=0;
    int use_chn =g_demo_para.vi_chn;
    
	while (1) {
		//struct video_stream stream = {0};
		struct video_input_frame input_frame;
		
		//get a video frame from video input
        int ret = ak_vi_get_frame(vi_handle, &input_frame);
		if (ret != 0) {
//			ak_print_error_ex("get stream failed\n");
			ak_sleep_ms(3);
			continue;
		}
		//skip the first some frames to get a better quality frame
        if (skip_count <25)
        {   
            skip_count++;
            ak_vi_release_frame(vi_handle,&input_frame);
            continue;
        }else
        {
            //encode a frame
            ret = ak_venc_send_frame(venc_handle , input_frame.vi_frame[use_chn].data,input_frame.vi_frame[use_chn].len
                , stream_handle);                            

            ak_vi_release_frame(vi_handle,&input_frame);

            if (0 !=ret)
            {
                ak_print_error_ex("encode error!\n");
            }else
            {
                ak_print_normal("get stream, size: %d,  ts=%d\n",
        			 stream_handle->len, stream_handle->ts);
#if PHOTO_SAVE_2_SD
                //save a video frame     			 
        		save_stream(fp, stream_handle->data, stream_handle->len);
#endif
        		//if (NULL !=stream.data )
            		//free(stream.data);
    		}
    		break;
		}
            
		
	}

    
#if PHOTO_SAVE_2_SD
    close_file(fp);
#endif

	return 1;

}


int photo_open(int pixel)
{
    //initialize 
    if (0 !=init_para(pixel))
        return 0;

#if PHOTO_SAVE_2_SD
    //mount the fat file system of tf card 
	if (0 !=ak_mount_fs(DEV_MMCBLOCK, 0, "a:") )
	{
	    ak_print_error("sd card mount failed!\n");
        return 0;
	}
#endif

    //initialize a video input handle
    vi_handle = video_input_init();
    if (vi_handle == NULL) {
        ak_print_error_ex("video input init faild, exit\n");
        return 0;
    }

    //initialize a video encode handle
    venc_handle = video_encode_init();
    if (venc_handle == NULL) {
        ak_print_error_ex("video encode open failed!");
        ak_vi_close(vi_handle);
        return 0;
    }
    
	return 1;
}

/**
 * @brief   the function to take a photo
 * @param   pixel the pixel para
 * @param   pVideo the video stream
 * @return  no
 */
int photo_one(struct video_stream *p_stream)
{

    //the main process to get a jpg format photo
    
    if(vi_handle == NULL)
        return 0;
    if(venc_handle == NULL)
        return 0;
       
    stream_handle = p_stream;
    return encode_jpg();
}

void photo_close(void)
{
    if(stream_handle != NULL){
        if (NULL != stream_handle->data )
            free(stream_handle->data);
    }

    if(venc_handle != NULL)
	    ak_venc_close(venc_handle);
    if(vi_handle != NULL)
	    ak_vi_close(vi_handle);

#if PHOTO_SAVE_2_SD
    //unmount the fat file system of tf card 
	ak_unmount_fs(DEV_MMCBLOCK, 0, "a:");
#endif
}



