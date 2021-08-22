/******************************************************
 * @brief  hardware move detect demo
 
 * @Copyright (C), 2016-2017, Anyka Tech. Co., Ltd.
 
 * @Modification  2017-5-17 
*******************************************************/
#include "command.h"
#include "ak_common.h"
#include "ak_vi.h"
#include "ak_md.h"
#include "libc_mem.h"
#include "kernel.h"

/******************************************************
  *                    Constant         
  ******************************************************/
static int detect_time =0;        //demo run time
static int sensitivity = 0;

static char *help[]={
	"hw md module demo",
	"Usage: hwmddemo <detect_time> <sensitivity>\n\n"
	"       detect_time  test time (second),default 20s\n"
	"       sensitivity to md ,value [1-100],default 50.\n"	

};

/******************************************************
  *                    Macro         
  ******************************************************/
#define AREA_H_NUM 4
#define AREA_V_NUM 2

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

/*****************************************
 * @brief match sensor
 * @param [void]  
 * @return on success return AK_SUCCESS, fail return AK_FAILED
 *****************************************/
static int match_sensor(void)
{
	char *main_addr ="ISPCFG";
	if (ak_vi_match_sensor(main_addr)) {
		ak_print_normal(" match sensor first_cfg fail!\n");
		return AK_FAILED;
	} else {
		ak_print_normal(" ak_vi_match_sensor ok!\n");
	}

	return AK_SUCCESS;
}

/*****************************************
 * @brief video input init
 * @param handle[in]  video hander
 * @return on success return 0, fail return -1
 *****************************************/
static int init_video_input(void * handle)
{	
	
	struct video_resolution video_res;
	struct video_channel_attr ch_attr ;
	int ret = 0;

	/* get sensor resolution */	
	memset(&video_res,0,sizeof(struct video_resolution));
	if (ak_vi_get_sensor_resolution(handle, &video_res)) 
		ak_print_error_ex("ak_mpi_vi_get_sensor_res fail!\n");
	else
		ak_print_normal("ak_mpi_vi_get_sensor_res ok! w:%d, h:%d\n",
			video_res.width, video_res.height);

	/* set video resolution and  crop information */	
	memset(&ch_attr,0,sizeof(struct video_channel_attr));
	ch_attr.res[VIDEO_CHN_MAIN].width = 1280;
	ch_attr.res[VIDEO_CHN_MAIN].height = 720;
	ch_attr.res[VIDEO_CHN_SUB].width = 640;
	ch_attr.res[VIDEO_CHN_SUB].height = 360;	
	ch_attr.crop.width = 1280;
	ch_attr.crop.height = 720;

	if (ret |= ak_vi_set_channel_attr(handle, &ch_attr)) {
		ak_print_normal("ak_vi_set_channel_attr fail!\n");
	} else {
		ak_print_normal("ak_vi_set_channel_attr ok!\n");
	}
	
	/* get one frame like  ak_vi_capture_on in linux */
	struct video_input_frame frame;
	ret = ak_vi_get_frame(handle, &frame);
	if (!ret)
		/* the frame has used,release the frame data */
		ak_vi_release_frame(handle, &frame);
	return ret ;
}

/*****************************************
 * @brief move detect
 * @param handle[in]  video hander
 * @return on success return 0, fail return -1
 *****************************************/
static int move_detect_test(void *vi_handle)
{
	struct ak_timeval systime_bak,systime_now ;
	
	int md_time = 0;
	int *area_sens = NULL ;
	int v,h;
	char * pch;
	ak_get_ostime(&systime_bak);
 
	ak_print_normal_ex("start\n");   		

	/* md init */
	if(ak_md_init(&vi_handle) == 0){

		/* Global move detech */
		if(AK_SUCCESS != ak_md_set_global_sensitivity(sensitivity))  
			ak_print_normal_ex("fail\n"); 
		if(AK_SUCCESS != ak_md_enable(1))  
			ak_print_normal_ex("fail\n"); 

		
		do{
			if( 1 == ak_md_get_result(&md_time, NULL, 1))
				ak_print_normal_ex("get md. time:%d\n",md_time);

				ak_get_ostime(&systime_now);
			
				if(( systime_now.sec-systime_bak.sec) > (detect_time/2))
				{	
					ak_print_notice_ex("sddsds:%d,%d\n",systime_now.sec-systime_bak.sec,detect_time);
					break;
				}

		}while(1); 
		ak_print_normal_ex("exit global,enter area md\n"); 
		ak_md_enable(0);


		/* Area move detech */
		area_sens = calloc(sizeof(int),AREA_V_NUM*AREA_H_NUM); 
		for(v = 0; v < AREA_V_NUM; v++)
			for(h=0 ; h < AREA_H_NUM; h++) 
				*(area_sens+v*AREA_H_NUM+h) = sensitivity;
		
    	if(AK_SUCCESS != ak_md_set_area_sensitivity(AREA_H_NUM,AREA_V_NUM,area_sens))
			ak_print_normal_ex("fail\n");
		
		if(AK_SUCCESS != ak_md_enable(1))  
			ak_print_normal_ex("fail\n"); 

		ak_get_ostime(&systime_bak);
		pch = (char *)area_sens;
		do{  
			if( 1 == ak_md_get_result(&md_time, pch, 1)){
				ak_print_normal_ex("get md. time:%d\n",md_time); 
				for(v = 0; v < AREA_V_NUM; v++)
					for(h=0 ; h< AREA_H_NUM; h++)
						if(*(pch+v*AREA_H_NUM+h) > 0)
						ak_print_normal_ex("(v %d h %d)%d\n",v,h,*(pch+v*AREA_H_NUM+h)); 
			} 
			
			ak_get_ostime(&systime_now);
			
			if(( systime_now.sec-systime_bak.sec) > detect_time/2)
			{	
				ak_print_notice_ex("sddsds:%d,%d\n",systime_now.sec-systime_bak.sec,detect_time);
				break;
			}
		}while(1);  
		free(area_sens);
	}

	/* md destroy */
	ak_md_destroy(); 
	ak_print_normal_ex("exit\n");
   
	return 0;  
}

/*****************************************
 * @brief init params
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
static int init_params(int argc, char **args)
{


	int i = 0;
	if (argc > 2)
	{
		ak_print_error_ex("i err2:%d\n", argc);
		return -1;
	}
	/* param defult */
	detect_time =20;        //demo run time
	sensitivity = 50;
	
	for (i = 0; i < argc; i++) {
		switch (i) {
			case 0:
				detect_time = atoi(args[i]);

				break;
			case 1:
				sensitivity = atoi(args[i]);
				if (sensitivity <1 || sensitivity > 100) 
				{
					ak_print_error_ex("i err1:%d\n", i);
					return -1;
				}
				break;

			default:
				ak_print_error_ex("i err:%d\n", i);
		}
	}
	return 0;
}

/*****************************************
 * @brief start function for command
 * @param argc[in]  the count of command param
 * @param args[in]  the command param
 * @return void
 *****************************************/
void cmd_hw_md_test(int argc, char **argv)
{
	void *vi_handle = NULL;

	int ch;
	
	ak_print_normal("***md demo compile:%s %s %s***********\n",
		__DATE__,__TIME__,ak_md_get_version());
	/** step1: init params*/
	if(init_params(argc,argv))
	{
		ak_print_error("%s",help[1]);
		goto EXIT;
	}
	ak_print_normal("detect_time:%d sensitivity:%d\n",
		detect_time,sensitivity);
	/** step2: match sensor*/		 	
	if (match_sensor()) {
		ak_print_normal("match sensor fail!\n");
		goto EXIT;
	}
	/** step3: open sensor  */
	vi_handle = ak_vi_open(VIDEO_DEV0);
	if (NULL == vi_handle) {	
		ak_print_normal("ak_vi_open fail!\n");
		goto EXIT;
	} else {
		ak_print_normal("ak_vi_open ok!\n");
	}

	/** step4: init sensor*/
	if(init_video_input(vi_handle) != 0){
		ak_print_normal("init_video_input fail!\n");
		goto EXIT2;
	} else {
		ak_print_normal("init_video_input ok!\n");
	}	

	/** move detech start */
	move_detect_test(vi_handle);
	
EXIT2:	
	ak_vi_close(vi_handle);
EXIT:
	
	ak_print_normal("###### exit: akuio ######\n");

}

/*****************************************
 * @brief register hwmddemo command
 * @param [void]  
 * @return 0
 *****************************************/
static int cmd_hw_md_reg(void)
{
    cmd_register("hwmddemo", cmd_hw_md_test, help);
    return 0;
}

cmd_module_init(cmd_hw_md_reg)
