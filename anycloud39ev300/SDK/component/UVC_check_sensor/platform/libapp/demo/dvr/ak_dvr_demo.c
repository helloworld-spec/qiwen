#include <stdio.h>

#include "command.h"
#include "string.h"
#include "ak_common.h"


#include "ak_ai.h"
#include "ak_vi.h"
#include "ak_record_common.h"
#include "ak_dvr_record.h"
#include "record_ctrl.h"
#include "ak_config.h"
#include "ak_login.h"

#include "ak_drv_ircut.h"

#include "kernel.h"

#define PER_AUDIO_DURATION		    20	// AMR 20ms one frame


static  int g_rec_audio= 1 ;//0 : no  audio ; 1:  have audio 

static char *help_dvr[]={
	"video record demo",
	"usage:dvrdemo [time] <vga/720p> <rec_fps> <cap_fps> <bitrate>\n"
//	"<h264amr/h264acc/h264g711> <avi/mp4>"
};

static char *help_cycdvr[]={
	"video cyc record demo",
	"usage:cycdvrdemo [time] <vga/720p> <rec_fps> <cap_fps> <bitrate>\n"
//	"<h264amr/h264acc/h264g711> <avi/mp4>"
};

static void *vi_handle = NULL;

static struct video_config  g_video_config;
static int g_cap_fps=0;

static bool is_vga = false;

volatile void *ai_handle_global = NULL;
volatile int ai_handle_count_global=0;
volatile int vi_handle_count_global = 0;

static int init_param(int cyc ,int argc, char **args )
{
    int i;
    char * (* help)[];

    if (cyc)
    {
        help = &help_cycdvr;
        g_video_config.save_cyc_flag =1;
        strcpy(g_video_config.recpath, "a:/cyc/");
    }
    else
    {
        help = &help_dvr;
        g_video_config.save_cyc_flag =0;
        strcpy(g_video_config.recpath, "a:/normal/");
    }
    is_vga = false;   
    g_video_config.saveWidth = 1280;
    g_video_config.saveHeight= 720;
    g_video_config.minQp = 20;
    g_video_config.maxQp= 51;
    g_video_config.savefilefps = 25;
    g_video_config.savefilekbps =2048;
    g_video_config.gopLen = 2;
    g_video_config.saveAudioEncode = AK_AUDIO_TYPE_AMR;//amr
    g_cap_fps = 0;
    
    if (argc <1 || argc > 5)
    {
        ak_print_error("%s\n", (*help)[1]);
        return -1;
    }

    ak_print_normal("input para:%s %s %s %s %s\n",args[0],args[1],args[2],args[3],args[4]);
    for(i=0 ;i< argc;i++)
    {
        switch(i)
        {
            case 0:
                //save time
                g_video_config.saveTime = atoi(args[0]);
                if (g_video_config.saveTime <=0)
                {
                    ak_print_error("[time] para error![time] must be >0\n");
                    return -1;
                }
               break;
            case 1:            
                //definition
                if (0 == strcmp(args[1], "vga") )
                {
                	is_vga = true;
                    g_video_config.saveWidth = 640;
                    g_video_config.saveHeight= 360;
                    g_video_config.savefilefps = 10 ;
                    g_video_config.savefilekbps = 512 ;
                    
                }else if (0 == strcmp(args[1], "720p") )
                {
                    g_video_config.saveWidth = 1280;
                    g_video_config.saveHeight= 720;        
                    g_video_config.savefilefps = 25 ;
                    g_video_config.savefilekbps = 2048 ;
                    
                }else
                {
                    ak_print_error("pixel para error!\n%s\n",(*help)[1]);
                    return -1;
                }
                break;
            case 2:            
                //rec_fps
                g_video_config.savefilefps = atoi(args[2]);
                if (!(g_video_config.savefilefps >=1 && g_video_config.savefilefps <=30))
                {
                    ak_print_error("<rec_fps> para error!rec_fps must be >=1 and <=30.\n");
                    return -1;
                    
                }
                break;
            case 3:            
                //cap_fps
                g_cap_fps = atoi(args[3]);
                if (!(g_cap_fps >=0 && g_cap_fps <=30))
                {
                    ak_print_error("<cap_fps> para error!cap_fps must be >=0 and <=30.\n");
                    return -1;
                    
                }
                break;
                
            case 4:
                 //bitrate
                g_video_config.savefilekbps = atoi(args[4]);
                if (!(g_video_config.savefilekbps>=1 && g_video_config.savefilekbps <=10240) )
                {
                    ak_print_error("bitrate para error!bitrate must be >=1 and <=10240.\n");
                    return -1;
                 }
                 break;
        #if 0
            case 4:
                //encode
                if (0 == strcmp(args[4], "h264amr") )
                {
                    g_video_config.saveAudioEncode = AUDIO_ENC_TYPE_AMR;
                }else if (0 == strcmp(args[4], "h264aac"))
                {
                    g_video_config.saveAudioEncode = AUDIO_ENC_TYPE_AAC;
                
                }
                else if (0 == strcmp(args[4], "h264g711"))
                {
                    g_video_config.saveAudioEncode = AUDIO_ENC_TYPE_G711;
                
                }
                else
                {
                    ak_print_error("encode para error!\n%s\n.",help_dvr);
                    return -1;  
                }
                break;

            case 5:
                //format
                if (0 == strcmp(args[5], "avi"))
                {
                    g_video_config.saveFileType = RECORD_TYPE_AVI_NORMAL;
                    
                }else if (0 == strcmp(args[5], "mp4") )
                {
                    g_video_config.saveFileType = RECORD_TYPE_MP4;
                    
                }else
                {
                    ak_print_error("file type para error!\n%s\n",help_dvr);
                    return -1;
                    
                }
                break;
        #endif            
            
        }
    }
                
    return 0;
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

static int init_vi(void)
{

    if (ak_vi_match_sensor("ISPCFG") < 0)
    {
        ak_print_error_ex("match sensor failed\n");
        return AK_FAILED;
    }

	/* open device */
	vi_handle = ak_vi_open(VIDEO_DEV0);//ak_vi_get_handle(VIDEO_DEV0);
	if (NULL == vi_handle) {
		ak_print_error_ex("vi open failed\n");
		return AK_FAILED;
	}
	ak_print_normal("vi open ok\n");

    if ( 0!=set_dayornight_cfg(vi_handle) )
    {
        ak_vi_close(vi_handle);
		return AK_FAILED;
        
    }

	/* get camera resolution */
	struct video_resolution resolution = {0};
	if (ak_vi_get_sensor_resolution(vi_handle, &resolution))
	{
		ak_print_error_ex("get sensor resolution failed\n");
        ak_vi_close(vi_handle);
        return AK_FAILED;
	}	
	else
		ak_print_normal("sensor resolution height:%d,width:%d.\n",resolution.height,resolution.width);

    //judge resolution and user need
    if (g_video_config.saveWidth > resolution.width || g_video_config.saveHeight > resolution.height )
    {
        ak_print_error("sensor resolution is too smaller!\n"
                       "request video width=%d,height=%d;sensor width=%d,height=%d.\n"
                       ,g_video_config.saveWidth,g_video_config.saveHeight,resolution.width,resolution.height);
        ak_vi_close(vi_handle);
        return AK_FAILED;
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
    if (g_video_config.saveWidth >640 ) 
    {
    	attr.res[VIDEO_CHN_MAIN].width = g_video_config.saveWidth;
    	attr.res[VIDEO_CHN_MAIN].height = g_video_config.saveHeight;
    }else  
    {
    	attr.res[VIDEO_CHN_SUB].width  = g_video_config.saveWidth;
    	attr.res[VIDEO_CHN_SUB].height = g_video_config.saveHeight;

        //change vga height if sensor height is 960 or 480
    	if (g_video_config.saveHeight==360 &&  
    	    (resolution.height == 960 || resolution.height==480))
        { 
        	attr.res[VIDEO_CHN_SUB].height = 480;
        	g_video_config.saveHeight= 480;
        }
        
    }
	
	if (ak_vi_set_channel_attr(vi_handle, &attr))
		ak_print_error_ex("set channel attribute failed\n");

	/* get crop */
	struct video_channel_attr cur_attr ;
	if (ak_vi_get_channel_attr(vi_handle, &cur_attr)) {
		ak_print_normal("ak_vi_get_channel_attr failed!\n");
	}else
	{

        ak_print_normal("channel crop:left=%d,top=%d, width=%d,height=%d\n"
            ,cur_attr.crop.left,cur_attr.crop.top,cur_attr.crop.width,cur_attr.crop.height);

        ak_print_normal("channel pixel:main width=%d,height=%d;sub width=%d,height=%d\n"
            ,attr.res[VIDEO_CHN_MAIN].width,attr.res[VIDEO_CHN_MAIN].height
            ,attr.res[VIDEO_CHN_SUB].width,attr.res[VIDEO_CHN_SUB].height);

    }        

	if (g_cap_fps >0) //set capture fps
	{
    	ak_vi_set_fps(vi_handle, g_cap_fps);
    }

	ak_print_normal("capture fps: %d\n", ak_vi_get_fps(vi_handle));

	return AK_SUCCESS;
}

static int init_video(void)
{
	int ret = AK_FAILED;

	/* one video input device, only open one time vi for encode */
	ret = init_vi();
	if (ret) {
		ak_print_error_ex("video input init faild, exit\n");
	} else {
		ak_print_normal_ex("start capture ...\n");
	}

	if ((AK_FAILED == ret) && (NULL != vi_handle)) {
		ak_vi_close(vi_handle);
		vi_handle = NULL;
	}

	return ret;
}

static int init_audio(void)
{

	struct pcm_param audio_param = {0};

	audio_param.sample_bits = 16;
	audio_param.channel_num = AUDIO_CHANNEL_MONO;
	audio_param.sample_rate = 8000;

	ai_handle_global= ak_ai_open(&audio_param);
	if(NULL == ai_handle_global){
		return AK_FAILED;
	}
#if 0
	unsigned int sample_bytes = (audio_param.sample_bits / 8);
	int frame_size = (audio_param.sample_rate * audio_param.channel_num
			* PER_AUDIO_DURATION * sample_bytes ) / 1000;
	if(frame_size & 1){
		frame_size++;
	}
#endif
	ak_ai_set_frame_interval(ai_handle_global, 100);
	ak_ai_set_nr_agc(ai_handle_global, AUDIO_FUNC_ENABLE);
	ak_ai_set_resample(ai_handle_global, AUDIO_FUNC_ENABLE);
	ak_ai_set_volume(ai_handle_global, 5);
	ak_ai_clear_frame_buffer(ai_handle_global);

	return AK_SUCCESS;
}


static void init_demo(void)
{
//    ak_config_init_ini();
    if (0 !=ak_mount_fs(DEV_MMCBLOCK, 0, ""))
    {
		ak_print_error("sd card mount failed!\n");
        return ;
    }


	vi_handle = ak_vi_get_handle(0);
	if(vi_handle == NULL)
	{
		if(AK_FAILED == init_video()) 
			return;
	}	
    ++vi_handle_count_global;
    //judge if record audio
    if (g_rec_audio)
    {
    	if(ai_handle_global == NULL)
	    	if(AK_FAILED == init_audio()) {
	    		if (NULL != ai_handle_global) {
	    			ak_ai_close(ai_handle_global);
	    			ai_handle_global = NULL;
	    		}
	    		return;
	    	}
		++ai_handle_count_global;
	}
/*
    if (FS_SetAsynWriteBufSize(2*1024*1024,0))
        ak_print_normal("FS_SetAsynWriteBufSize ok!");
    else
        ak_print_normal("FS_SetAsynWriteBufSize failed!");
*/    

	ak_pthread_t dvr_thread_id = record_ctrl_init(vi_handle, ai_handle_global, RECORD_FILE_TYPE_AVI,&g_video_config);
	ak_thread_join(dvr_thread_id);

	

	if(vi_handle_count_global == 1)
	{
		ak_vi_close(vi_handle);
		vi_handle = NULL;
	}
	--vi_handle_count_global;

	if(ai_handle_count_global == 1)
	{
		ak_ai_close(ai_handle_global);
		ai_handle_global = NULL;
	}
	--ai_handle_count_global;
	//ak_unmount_fs(DEV_MMCBLOCK, 0, "");
	
  
}

static void cmd_dvr_demo(int argc, char **args)
{
    int ret;
    
    ret = init_param(0 , argc,args);
    if (ret !=0)
        return ;
        
    init_demo();
      
}

static void cmd_cycdvr_demo(int argc, char **args)
{
    int ret;
    
    ret = init_param(1,argc,args);
    if (ret !=0)
        return ;
        
    init_demo();
      
}

static int cmd_dvr_reg(void)
{
    cmd_register("dvrdemo", cmd_dvr_demo, help_dvr);
    cmd_register("cycdvrdemo", cmd_cycdvr_demo, help_cycdvr);
    return 0;
}

cmd_module_init(cmd_dvr_reg)

