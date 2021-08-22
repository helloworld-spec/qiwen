/*
*
*ring_main.c  first app statarted for ring call
*author:wmj
*date: 20161214
*/
#include "command.h"
#include "ak_common.h"
#include "cmd_api.h"
#include "ak_thread.h"
#include "ak_vi.h"
#include "ak_venc.h"
#include "ak_drv_ircut.h"
#include "ak_ao.h"
#include "ak_ai.h"
#include "dev_info.h"
#include "kernel.h"

#include "photo.h"


#define AO_SAVE_TO_SD       0
#define AI_SAVE_TO_SD       0
#define AO_READ_FROM_SD     0
static bool sd_mount_ok = 0;

#define START_TIME_DEBUG    1

#if START_TIME_DEBUG
    static struct ak_timeval g_main_start;
#endif

#define FRAM_CHECK_SUM_ENABLE     0


#define SAVE_PATH        "a:"
#define FILE_NAME        "video.h264"

#define FIRST_PATH       "ISPCFG"
#define BACK_PATH        "ISPCFG"

#define VIDEO_VGA_WIDTH				640
#define VIDEO_VGA_HEIGHT			360
#define VIDEO_VGA_HEIGHT_960P		480

#define RESOLU_VGA          		0
#define RESOLU_720P         		1
#define RESOLU_960P        			2



/*system will shutdown after  a idle time in seconds */
#define SYS_IDLE_BEFORE_SHUTDOWN   10

#define TASK_STACK_SIZE 			4096

#define MAIN_TASK_PRIO 				60//60
#define CMD_TASK_PRIO 				85
#define VIDEO_TASK_PRIO 			70
#define IDLE_TASK_PRIO				20
#define SOUND_PLAY_TASK_PRIO		21
#define AO_TASK_PRIO		        82
#define AI_TASK_PRIO		        82

#define SPI_HEADER_MAGIC            "SPIS"
#define SPI_MAX_TRANSFER_SIZE       (30*1024) // max is 65535

/*max frame len allowed on CC3200*/
#define FRAME_SEND_BUF_LEN          (200*1024)


/*  frame  type define */
enum frame_type {
    FRAME_TYPE_VIDEO_P = 0,
    FRAME_TYPE_VIDEO_I,
    FRAME_TYPE_AUDIO,
    FRAME_TYPE_PICTURE

};

#define FRAME_HEADER_MAGIC    "FRAM" //0x4652414D  //"FRAM"
#define FRAME_END_MAGIC       "FEND" //0x46454E44  //"FEND"

#define CMD_HEADER_MAGIC      "VCMD" //0x56434D44  //"VCMD"
#define CMD_END_MAGIC         "CEND" //0x43454E44  //"CEND"

// audio recv
#define AUDIO_RECV_BUF_SIZE         (4096)
#define AUDIO_RECV_BUF_NUM          16

#define AUDIO_RECV_BUF_EMPTY        0
#define AUDIO_RECV_BUF_READY        1
#define AUDIO_RECV_BUF_USE          2

typedef struct audio_recv_buf{
    unsigned char data[AUDIO_RECV_BUF_SIZE];
    unsigned int time;
    unsigned int len;
    int cnt;
    unsigned char state;
}audio_recv_buf_t;

typedef struct audio_recv_ctrl {
    bool wait_recv_ok;
        
    // cyc buf ctrl
    unsigned int num;
    unsigned int index;
    audio_recv_buf_t buf[AUDIO_RECV_BUF_NUM];
}audio_recv_ctrl_t;
audio_recv_ctrl_t audioRecv;

// audio send
typedef struct audio_send_ctrl {
    int  cnt; 
    char data[4096+16];
}audio_send_ctrl_t;
audio_send_ctrl_t audioSend; 

// audio synchronize
static ak_sem_t sem_audio_wait;




/** Find minimum value */
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*globle flag for video reques*/

static volatile int sys_video_start = 0;

static ak_sem_t  sem_video_start;
static ak_sem_t  sem_dingdong_start;

static ak_mutex_t devif_send_mutex;           //device interface mutex


static int sys_idle_time =0;


extern volatile void *ai_handle_global;
extern volatile int ai_handle_count_global;
extern volatile int vi_handle_count_global;

//#define VI_HANDEL_CLOSE(x) if((x)>1){x--}


struct  video_param
{
    unsigned char  resolution;
    unsigned char  fps;
    unsigned short kbps;
};

struct  Venc_Demo_Para
{
    int width;
    int height;
    int fps;
    int kbps;
};

static struct video_param g_video_para = {1, 25, 500};

static struct Venc_Demo_Para g_rec_para;

static void *g_venc_handle = NULL;

static bool is_vga = false;


typedef struct ring_device_manage {
    int *ao_handle;
    ak_sem_t  sem_ao_dev;

}ring_device_manage_t;
static ring_device_manage_t ring_dev_manage = {
    .ao_handle = NULL,
};



static short audio_doorbell_buf[]=
{
	#include "4204.txt"
};


static void *open_file()
{
	char file[100] = {0};
	sprintf(file, "%s/%s", SAVE_PATH, FILE_NAME);

	FILE *filp = NULL;
	filp = fopen(file, "w");
	if (!filp) {
		ak_print_error_ex("fopen, error\n");
	}

	return filp;
}

static void close_file(FILE *filp)
{
	fclose(filp);
}

static void save_stream(FILE *filp, unsigned char *data, int len)
{
	int ret = len;
	
	do {
		ret -= fwrite(data, 1, ret, filp);
	} while (ret > 0);
}



int sys_shutdown()
{
	/*close camera*/
	//camera_close();

	/*send sleep req to CC3200, and CC3200 will set H240 power off pin*/
	send_cmd_sleep();

	/*close uart*/
	//cmd_uart_deinit();

	/*close spi*/
	//data_spi_deinit();

	/*exit all thread*/
	
	/*power off*/
	ak_print_info("system to be power off now\n");
	
	
}


int sys_config()
{
	/*play sys config audio*/

	/*stop idle timer*/
	sys_idle_time = 0;

}

int sys_config_complete()
{
	/*play sys config complete audio*/

	/*start idle timer*/
	
	
}

/*send alarm info to cloud */
int send_alarm(int type)
{
	
	
	switch(type)
	{
		case ALARM_BATTERY_TOO_LOW:
			/*send low battery alarm*/
			ak_print_info("send battery too low alarm\n");
			send_cmd_battery_low_alarm();
			break;
		case ALARM_MOVE_DETECT:
			/*send move detect alarm*/
			ak_print_info("send move detect alarm\n");
			break;
		default:
			ak_print_error("unknown alarm type\n");
			break;
			
	}
}

int send_cmd_sleep()
{
	int ret;
	CMD_INFO cmd;
	cmd.preamble = CMD_PREAMBLE;
	cmd.cmd_id = CMD_SLEEP_REQ;
	cmd.cmd_len = (EVENT_CMD_LEN);
	cmd.cmd_seq = (0);
	cmd.cmd_result = (0);
	cmd.param.event = (0);

	ak_print_info("send cmd id = %d\n", cmd.cmd_id);
	ret = send_one_cmd((char *)&cmd, EVENT_CMD_LEN + CMD_HEAD_LEN);
	if(ret < 0)
	{
		ak_print_error_ex("send cmd sleep error\n");
	}
	return ret;
}


int send_cmd_battery_low_alarm()
{
	int ret;
	CMD_INFO cmd;
	cmd.preamble = CMD_PREAMBLE;
	cmd.cmd_id = CMD_BATTERY_LOW_ALARM;
	cmd.cmd_len = (EVENT_CMD_LEN);
	cmd.cmd_seq = (0);
	cmd.cmd_result = (0);
	cmd.param.event = (0);

	ak_print_info("send cmd id = %d\n", cmd.cmd_id);
	ret = send_one_cmd((char *)&cmd, EVENT_CMD_LEN + CMD_HEAD_LEN);
	if(ret < 0)
	{
		ak_print_error("send battery too low alarm error\n");
	}
	return ret;
}


	
int send_cmd_wakeup_source_check()
{
	int ret;
	CMD_INFO cmd;
	cmd.preamble = CMD_PREAMBLE;
	cmd.cmd_id = CMD_WAKEUP_SRC_REQ;
	cmd.cmd_len = (EVENT_CMD_LEN);
	cmd.cmd_seq = (0);
	cmd.cmd_result = (0);
	cmd.param.event = (0);

	ak_print_normal("send cmd id = %d\n", cmd.cmd_id);
	ret = send_one_cmd((char *)&cmd, EVENT_CMD_LEN + CMD_HEAD_LEN);
	if(ret < 0)
	{
		ak_print_error("send wake up source req error\n");
	}
	return ret;
}


int ak_battery_get()
{
	/*return battery*/
	return 100;
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
    
    fd = dev_open(DEV_CAMERA);
    if (fd <0)
    {
		ak_print_error_ex("dev_open fail!\n");
		return -1;
    }
    
    ret = dev_ioctl(fd, IO_CAMERA_IRFEED_GPIO_OPEN, &status);
    if (0 !=ret)
    {
		ak_print_error_ex("dev_ioctl IO_CAMERA_IRFEED_GPIO_OPEN fail!\n");
		return -1;
    }
    
    ret = dev_ioctl(fd, IO_CAMERA_IRFEED_GPIO_GET, &status);
    if (0 !=ret)
    {
		ak_print_error_ex("dev_ioctl IO_CAMERA_IRFEED_GPIO_GET fail!\n");
        dev_ioctl(fd, IO_CAMERA_IRFEED_GPIO_CLOSE, &status); 
		return -1;
    }
    
    
    if (status == CAMERA_IRFEED_GPIO_STATUS_DAY) {
        ak_print_notice("set day isp config and ircut!\n");

        ak_drv_ir_set_ircut(IRCUT_STATUS_DAY);
        ak_vi_switch_mode(vi_handle, VI_MODE_DAYTIME); 
    } else {
        ak_print_notice("set night isp config and ircut!\n");
        ak_vi_switch_mode(vi_handle, VI_MODE_NIGHTTIME); 
        ak_drv_ir_set_ircut(IRCUT_STATUS_NIGHT);
    }
    dev_ioctl(fd, IO_CAMERA_IRFEED_GPIO_CLOSE, &status); 
    
    return 0;
}

int set_video_param(void *enc_handle, struct  video_param *p_video_param )
{
	int ret;
	
	if(enc_handle != NULL)
	{
		ret = ak_venc_set_fps(enc_handle, p_video_param->fps);
		ret = ak_venc_set_rc(enc_handle, p_video_param->kbps);
	}
	else
	{
		ak_print_error("video encode handle no opened\n");
		ret = -1;
	}
	return ret;
}


static void *vi_initial(const char *first, const char *second)
{
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
			g_rec_para.height	 =480;
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
	//ak_vi_set_fps(handle, g_rec_para.fps);
	ak_print_normal("capture fps: %d\n", ak_vi_get_fps(handle));

	return handle;
}



static void *venc_initial()
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
	param->br_mode = BR_MODE_CBR;//BR_MODE_CBR
	param->enc_out_type = H264_ENC_TYPE;
	void * encode_video = ak_venc_open(param);
	ak_print_normal("YMX : free 2: size = %d\n", sizeof(struct encode_param));
	free(param);
	return encode_video;
}


/*
*@brief:  send audio data
*@param: time 
*@param: data_len
*@param: data_len
*@return; send len
*/
static int send_audio_data(int time,int data_len,char *data)
{
    static char *opt_ptr;
    static unsigned int check_sum = 0;
    int ret = 0;
    SPI_STREAM_SEND_INFO  spi_send_header;

#if 1

    if(audioSend.cnt == 0) {
        opt_ptr = &audioSend.data[0];
        memcpy(opt_ptr, FRAME_HEADER_MAGIC, 4);
        opt_ptr += 4;
        *(int*)opt_ptr  = FRAME_TYPE_AUDIO;
        opt_ptr += 4;
        
        memcpy(opt_ptr, &check_sum, 4);
        //memcpy(opt_ptr, &time, 4);
        opt_ptr += 4;
        
        memcpy(opt_ptr, &data_len, 4);
    	opt_ptr += 4;
    } 
    
    memcpy(opt_ptr, data, data_len);
    audioSend.cnt += data_len;
    opt_ptr += data_len;

    if(audioSend.cnt >= 512) {
        memcpy(&audioSend.data[sizeof(AUDIO_STREAM_HEADER)-4], &audioSend.cnt, 4); // re input len
#if FRAM_CHECK_SUM_ENABLE
        check_sum = 0;
        for(i = 12; i < (audioSend.cnt+16);i++){
            check_sum += audioSend.data[i];
        }
#else
        check_sum++;
#endif	
        memcpy(opt_ptr, &check_sum, 4);
        opt_ptr += 4;
        memcpy(opt_ptr, FRAME_END_MAGIC, 4);

        memcpy(spi_send_header.header_magic, SPI_HEADER_MAGIC, 4);
        spi_send_header.reserve= FRAME_TYPE_AUDIO;
        spi_send_header.pack_id= check_sum;
		spi_send_header.data_len = audioSend.cnt + sizeof(AUDIO_STREAM_HEADER) + sizeof(AUDIO_STREAM_END);

        int i;
        char *p_str = (char*)&audioSend.data[0];

        //ak_print_normal("send audio len %d\n",data_len);
        //for(i =0;i < 16;i++)
        //     ak_print_normal(" %0x",p_str[i]);
        //ak_print_normal("\n");
        
        ak_thread_mutex_lock(&devif_send_mutex);
        ret = data_send((char*)&spi_send_header, sizeof(SPI_STREAM_SEND_INFO));
        if(ret != sizeof(SPI_STREAM_SEND_INFO))
		{
			ak_print_error("send audio frame header error %d\n" ,ret);
		}
        
        ret = data_send(&audioSend.data[0],spi_send_header.data_len);
        //ret = data_send(&audioSend.data[0],audioSend.cnt + sizeof(AUDIO_STREAM_HEADER)+8);
        ak_thread_mutex_unlock(&devif_send_mutex);
        audioSend.cnt = 0;

    }
#endif

    return ret;
}


#if 1
void* ao_task()
{
	//ao parma
	struct pcm_param info;
	int ret = 0;
	
	unsigned long pcm_indx = 0;
	
	unsigned int tmpBuf_size = 0;
	int *handle = NULL;
	int send_size, size;
	unsigned char *tmpBuf = NULL;
	info.sample_bits = 16;
	info.sample_rate = 8000;
	info.channel_num = 1;


	//ao operate
    if(ring_dev_manage.ao_handle == NULL)
        handle = ak_ao_open(&info);
    else
        handle = ring_dev_manage.ao_handle;

#if START_TIME_DEBUG
    struct ak_timeval ao_open_t1;
    ak_get_ostime(&ao_open_t1);
    long diff = ak_diff_ms_time(&ao_open_t1,&g_main_start);
    ak_print_normal("-----ao time %lu\n", diff);
#endif

	tmpBuf_size = ak_ao_get_frame_size(handle);
	ak_print_normal("ao frame size =%d\n", tmpBuf_size);
	
	tmpBuf = (unsigned char *)malloc(tmpBuf_size);
	if(NULL == tmpBuf)
	{
		ak_print_error("ao demo malloc false!\n");
		goto ao_end;
	}
	
	//set volume
	ak_ao_set_volume(handle, 0x06);	
	ak_ao_enable_speaker(handle,1);
	
	ak_print_normal("begin ao............\n");

#if AO_READ_FROM_SD
    FILE * fd = fopen("001.pcm", "r" );
#endif

#if AO_SAVE_TO_SD
	static int recv_frame_cnt = 0;
	struct ak_date systime;
	  unsigned long free_size = 0; 
	  char * path = "a:/";
	  char file_path[255] = {0};
	  
	  free_size = ai_get_disk_free_size(path);//get T_card free size  
	  if(free_size < 1024)
	  { 	  
		  ak_print_error_ex("free_size < AI_TCARD_FREE_SIZEKB\n",1024);
		  //goto pcai_unmount;
	  }

	  memset(&systime, 0, sizeof(systime));
	  ak_get_localdate(&systime);
	  sprintf(file_path, "%sPCAI_%04d%02d%02d_%02d%02d%02d.pcm", path,systime.year, systime.month + 1, 
	  systime.day + 1, systime.hour, systime.minute, systime.second);
	  
	  FILE *p;
	  p = fopen(file_path, "w+");
#endif

    memset(&audioRecv,0,sizeof(audio_recv_ctrl_t));
    audioRecv.num = 0;
    audioRecv.index = 0;

    audioRecv.wait_recv_ok = 1;
	
	ak_thread_sem_wait(&sem_audio_wait);			
	while(1)
	{		
		//wait for next time  play	
		//ak_thread_sem_wait(&sem_audio_wait);			
		if(!sys_video_start)
		{
		    ak_print_normal("video is not start\n");
			break;
		}
		//ak_sleep_ms(20);
        //ak_sleep_ms(5);

        ret = 0;
	#if 0	
		if(1 == info.channel_num)    
           {
               memcpy(tmpBuf, readpcmBuf+pcm_indx, tmpBuf_size/2);
               ret = tmpBuf_size/2;    

               pcm_indx += tmpBuf_size/2;            
              // aRecv->cnt -= tmpBuf_size/2;
               //ak_print_normal("cnt %d\n",aRecv->cnt);
           }
           else
           {
               memcpy(tmpBuf, readpcmBuf+pcm_indx, tmpBuf_size);
               ret = tmpBuf_size;
               pcm_indx += tmpBuf_size;
                //aRecv->cnt -= tmpBuf_size;
           }
      #endif         
		#if 1
        if(audioRecv.num > 0) {
            audio_recv_buf_t *aRecv = &audioRecv.buf[(audioRecv.index+\
                AUDIO_RECV_BUF_NUM-audioRecv.num)%AUDIO_RECV_BUF_NUM];
            
            //ak_print_normal("ao num %d\n",audioRecv.num);
            if(aRecv->state == AUDIO_RECV_BUF_USE) 
            {
               if(1 == info.channel_num)    
               {
				   
                   memcpy(tmpBuf, aRecv->data+pcm_indx, tmpBuf_size/2);
                   ret = tmpBuf_size/2;    
					
                   pcm_indx += tmpBuf_size/2;            
                   aRecv->cnt -= tmpBuf_size/2;
               }
               else
               {
                   memcpy(tmpBuf, aRecv->data+pcm_indx, tmpBuf_size);
                   ret = tmpBuf_size;
                   pcm_indx += tmpBuf_size;
                   aRecv->cnt -= tmpBuf_size;
               }
               
               if(aRecv->cnt <= 0) {
                   aRecv->state = AUDIO_RECV_BUF_EMPTY;
                   memset(aRecv->data,0,AUDIO_RECV_BUF_SIZE);
				   pcm_indx = 0;
                   audioRecv.num--;
               }
            }
        }
		else
		{
			ak_sleep_ms(5);
			continue;
		}
	#endif
        
#if AO_SAVE_TO_SD
		//if(sd_mount_ok)
		{
			if(recv_frame_cnt < 500)
			{
				//ak_print_normal("audio get frame cnt %d\n",recv_frame_cnt);
				if(NULL == p) 
				{
					ak_print_error_ex("create pcm file err\n");
				}
				if(fwrite(tmpBuf,ret,1,p) <= 0)
				{
					ak_print_error_ex("write file err\n");		  
				}
			} else if(recv_frame_cnt == 500){
				 ak_print_normal("####################################\n");
				 ak_print_normal("		   ao file close		   \n");
				 ak_print_normal("####################################\n");
				 fclose(p);
			}
			recv_frame_cnt++;
		}
#endif
#if AO_READ_FROM_SD
        //ret = fread(tmpBuf, ret,1,fd);
        //if(ret <= 0) {
       //   fd = fopen("001.pcm", "r" );
       // }
#endif
        //ak_print_normal("audio speaker lenght %d\n",ret);
		send_size = 0;
		while(ret > 0)
		{
		    long tick1;
            long tick2;
            
            //tick1 = get_tick_count();
			size = ak_ao_send_frame(handle, tmpBuf + send_size, ret, 0);
            //tick2 = get_tick_count();
           // ak_print_normal("ao time %d\n",tick2-tick1);
            
			if(size < 0)
			{
				ak_print_error("ao send frame error!  %d \n", size);
				break;
			}
			else
			{
				send_size += size; 
				ret = ret - size;
			}
		}			
	}

	//close SPK
	ak_ao_enable_speaker(handle,0);
	

tmpbuf_end:
	if(tmpBuf != NULL)
	{	
		free(tmpBuf);
	}

ao_end:	
    memset(audioRecv,0,sizeof(audio_recv_ctrl_t));
    //pcm_indx = 0;
	ak_ao_close(handle);
	ak_print_normal("ao close !\n");
	ak_thread_exit();	
}		

#if 0
void* ao_task()
{
	//ao parma
	struct pcm_param info;
	int ret = 0;	
   	unsigned long pcm_indx = 0;
    
	unsigned int tmpBuf_size = 0;
	int *handle = NULL;
	int send_size, size;
	unsigned char *tmpBuf = NULL;
	info.sample_bits = 16;
	info.sample_rate = 8000;
	info.channel_num = 1;

    struct ak_timeval cnt2;
    struct ak_timeval cnt1;
	//ao operate
	handle = ak_ao_open(&info);

	tmpBuf_size = ak_ao_get_frame_size(handle);
	ak_print_normal("ao frame size =%d\n", tmpBuf_size);
	
	tmpBuf = (unsigned char *)malloc(tmpBuf_size);
	if(NULL == tmpBuf)
	{
		ak_print_error("ao demo malloc false!\n");
		goto ao_end;
	}
	
	//set volume
	ak_ao_set_volume(handle, 0x05);	
	ak_ao_enable_speaker(handle,1);
	
	ak_print_normal("begin ao............\n");

#if AO_READ_FROM_SD
    FILE * fd = fopen("001.pcm", "r" );
#endif

    memset(&audioRecv,0,sizeof(audio_recv_ctrl_t));
    audioRecv.num = 0;
    audioRecv.index = 0;

    audioRecv.wait_recv_ok = 1;
    
    //wait for next time  play	
    ak_thread_sem_wait(&sem_audio_wait);	
	while(1)
	{		
		if(!sys_video_start)
		{
			break;
		}
		//ak_sleep_ms(10);
        //ak_sleep_ms(2);

        ret = 0;
        if(audioRecv.num > 0) {
            audio_recv_buf_t *aRecv = &audioRecv.buf[(audioRecv.index+\
                AUDIO_RECV_BUF_NUM-audioRecv.num)%AUDIO_RECV_BUF_NUM];
            if(aRecv->state == AUDIO_RECV_BUF_USE) 
            {
               if(1 == info.channel_num)    
               {
                   memcpy(tmpBuf, aRecv->data+pcm_indx, tmpBuf_size/2);
                   ret = tmpBuf_size/2;    

                   pcm_indx += tmpBuf_size/2;            
                   aRecv->cnt -= tmpBuf_size/2;
                   //ak_print_normal("cnt %d\n",aRecv->cnt);
               }
               else
               {
                   memcpy(tmpBuf, aRecv->data+aRecv->cnt, tmpBuf_size);
                   ret = tmpBuf_size;
                   pcm_indx += tmpBuf_size;
                    aRecv->cnt -= tmpBuf_size;
               }
               
               if(aRecv->cnt <= 0) {
                   aRecv->state = AUDIO_RECV_BUF_EMPTY;
                   memset(aRecv->data,0,AUDIO_RECV_BUF_SIZE);
                   pcm_indx = 0;
                   audioRecv.num--;
               }
            }
        }else {
            ak_sleep_ms(5);
			continue;
        }

        
#if AO_READ_FROM_SD
        ret = fread(tmpBuf, ret,1,fd);
       // if(ret <= 0) {
       //   fd = fopen("001.pcm", "r" );
       // }
#endif
        //ak_print_normal("audio speaker lenght %d\n",ret);
		send_size = 0;
		while(ret > 0)
		{
		    long tick1;
            long tick2;
            
            tick1 = get_tick_count();
			size = ak_ao_send_frame(handle, tmpBuf + send_size, ret, 0);
            tick2 = get_tick_count();
            ak_print_normal("ao time %d\n",tick2-tick1);
            
			if(size < 0)
			{
				ak_print_error("ao send frame error!  %d \n", size);
				break;
			}
			else
			{
				send_size += size; 
				ret = ret - size;
			}
		}			
	}

	//close SPK
	ak_ao_enable_speaker(handle,0);
	

tmpbuf_end:
	if(tmpBuf != NULL)
	{	
		free(tmpBuf);
	}

ao_end:	
    memset(audioRecv,0,sizeof(audio_recv_ctrl_t));
    //pcm_indx = 0;
	ak_ao_close(handle);
	ak_print_normal("ao close !\n");
	ak_thread_exit();	
}		
#endif
void* ai_task()
{
	//ai parma
	int ret = -1;
	struct pcm_param input;
	struct frame frame = {0};	
	
	//ai operate	
	input.sample_bits = 16;
	input.channel_num = 1;
	input.sample_rate = 8000;
   
#if AI_SAVE_TO_SD  
    static int ai_frame_cnt = 0;
    struct ak_date systime;
    unsigned long free_size = 0; 
    char * path = "a:/";
    char file_path[255] = {0};

    free_size = ai_get_disk_free_size(path);//get T_card free size  
    if(free_size < 1024)
    {       
      ak_print_error_ex("free_size < AI_TCARD_FREE_SIZEKB\n",1024);
      //goto pcai_unmount;
    }

    memset(&systime, 0, sizeof(systime));
    ak_get_localdate(&systime);
    sprintf(file_path, "%sAOPC_%04d%02d%02d_%02d%02d%02d.pcm", path,systime.year, systime.month + 1, 
    systime.day + 1, systime.hour, systime.minute, systime.second);

    FILE *p;
    p = fopen(file_path, "w+");
#endif


	//g_rec_end = 0;
	ak_print_normal("wait ai open...\n");

    void *adc_drv = NULL;
    if(ai_handle_count_global < 1) {
    	adc_drv = ak_ai_open(&input);
        if(adc_drv == NULL){
            return;
        }
        ai_handle_global = adc_drv;
    }
    ai_handle_count_global++;

    
    //ak_print_normal("---ai open ok---\n");
    
	ak_ai_set_source(adc_drv, AI_SOURCE_MIC);
	/*
		AEC_FRAME_SIZE	is	256¡ê?in order to prevent droping data,
		it is needed  (frame_size/AEC_FRAME_SIZE = 0), at  the same 
		time, it  need	think of DAC  L2buf  , so  frame_interval  should  
		be	set  32ms?¡ê
	*/
	if(ak_ai_set_frame_interval(adc_drv, 32) != 0)
	{
		ak_print_error_ex("set_frame_size error!\n");
		goto ai_close;
	}

	ak_ai_clear_frame_buffer(adc_drv);

	if(ak_ai_set_volume(adc_drv, 7)!=0)
	{	
		ak_print_error_ex("set gain error!\n");
		goto ai_close;
		
	}
    ak_print_normal("ai open ...\n");
	ak_ai_set_aec(adc_drv, 1);
    
	while(1)
	{
		if(!sys_video_start)
		{
			ak_thread_sem_post(&sem_audio_wait);				
			ak_ai_release_frame(adc_drv, &frame);			
			goto ai_close;
		}
        
		ak_thread_sem_post(&sem_audio_wait);
         
		ret = ak_ai_get_frame(adc_drv, &frame, 1);       
		if(ret == -1)
		{
			ak_sleep_ms(5);
			continue;							
		}
		
		//memcpy(&aecBuf[aec_index], frame.data, frame.len);
#if AI_SAVE_TO_SD
                    if(sd_mount_ok)
                    {
                    
                        if(ai_frame_cnt < 200)
                        {
                            //ak_print_normal("ai frame cnt %d\n",file_cnt);
                            if(NULL == p) 
                            {
                                ak_print_error_ex("create pcm file err\n");
                                //goto pcai_unmount ;
                            }
                            if(fwrite(frame.data,frame.len,1,p) <= 0)
                            {
                                ak_print_error_ex("write file err\n");
                                //goto pcai_close;          
                            }
                        } else if(ai_frame_cnt == 200){
                             ak_print_normal("####################################\n");
                             ak_print_normal("         ai file close           \n");
                             ak_print_normal("####################################\n");
                             fclose(p);
                        }
                        ai_frame_cnt++;
                    }
#endif


#if 1
        if(frame.len > 0) {
            if(send_audio_data(0,frame.len,frame.data)< 0)
            {
                ak_print_error("send audio frame to pc error\n");
            }
        }
#endif
  
		ak_ai_release_frame(adc_drv, &frame);		
	}
	
ai_close:
	ak_ai_release_frame(adc_drv, &frame);
    if(ai_handle_count_global == 1) {
        ak_ai_close(adc_drv);
    }
    ai_handle_count_global--;
    
	adc_drv = NULL;
	ak_print_normal("ai close !\n");
	ak_thread_exit();	
}

#endif


/*
*start a thread to encode and send video
*/
void *video_encode_task(void *arg)
{
//	int enc_ch = *(int *)arg;
	void *vi_handle, *venc_handle, *stream_handle;
	SPI_STREAM_SEND_INFO	spi_trans_head;
	int i;
	int transfer_size, transfer_progress;
	char *send_buf = NULL;
	char *opt_ptr;
	static unsigned int check_sum = 0;
    int frame_len;
  
	switch(g_video_para.resolution)
	{
		case RESOLU_VGA:
			g_rec_para.width  = 640; 
			g_rec_para.height= 360;
			break;
		case RESOLU_720P:
			g_rec_para.width  = 1280; 
			g_rec_para.height= 720;
			break;
		case RESOLU_960P:
			g_rec_para.width  = 1280; 
			g_rec_para.height= 720;
			break;
		default:
			g_rec_para.width  = 1280; 
			g_rec_para.height= 720;
			break;
	}
	if(g_video_para.fps == 8
		|| g_video_para.fps == 10
		|| g_video_para.fps == 12
		|| g_video_para.fps == 13
		|| g_video_para.fps == 25
		|| g_video_para.fps == 30)
	{
		g_rec_para.fps = g_video_para.fps;
	}
	else
	{
		ak_print_error("video fps %d not supported\n", g_video_para.fps);
	}
	g_rec_para.kbps = g_video_para.kbps;
	


	//is_vga = true;
	
	ak_print_normal_ex("start a work encode\n");

    if(vi_handle_count_global < 1) {
        vi_handle = vi_initial(FIRST_PATH, BACK_PATH);
        if (vi_handle == NULL) {
            ak_print_error_ex("video input init faild, exit\n");
            ak_thread_exit();
            return NULL;
    //      goto exit1;
        }
    }
    vi_handle_count_global++;

	venc_handle = venc_initial();
	if (venc_handle == NULL) {
		ak_print_error_ex("video encode open failed!");
		goto exit1;
	}
	g_venc_handle = venc_handle;
	
#if 0
	FILE *fp = open_file();
	if (fp ==NULL)
	{
		ak_print_error_ex("open file failed!");
		goto exit2; 	
	}
#endif 


#if 1
    ak_print_normal("-------ak_venc_request_stream start--------\n");
    
	stream_handle = ak_venc_request_stream(vi_handle,venc_handle );
	if (stream_handle == NULL) {
		ak_print_error_ex("request stream failed\n");
		goto exit;
	}

	send_buf = malloc(FRAME_SEND_BUF_LEN);
	if(send_buf == NULL)
	{
		ak_print_error("malloc buff for sending frame error\n");
		goto exit;
	}
    
	sys_video_start = 1;
    ak_print_normal("sys_video_start = 1\n");

    while (sys_video_start) {
		struct video_stream stream = {0};
		int ret = ak_venc_get_stream(stream_handle, &stream);
		if (ret != 0) {
			//ak_print_error_ex("get stream failed\n");
			ak_sleep_ms(3);
			continue;
		}
        //ak_print_normal("get stream, size: %d, type=%d, ts=%d\n",
		//	 stream.len, stream.frame_type, stream.ts);

#if 1
		frame_len = stream.len + sizeof(VIDEO_STREAM_HEADER) + sizeof(VIDEO_STREAM_END);

        //video_send_buf.header_magic = FRAME_HEADER_MAGIC;
        memcpy(spi_trans_head.header_magic, SPI_HEADER_MAGIC, 4);
        spi_trans_head.reserve= (stream.frame_type);
        spi_trans_head.pack_id= (check_sum);
		spi_trans_head.data_len = (frame_len);
        
		//test
		//save_stream(fp, stream.data, stream.len);

		//ak_print_normal("send frame header\n");

        ak_thread_mutex_lock(&devif_send_mutex);
       // spi packet header
		if(data_send((char*)&spi_trans_head, sizeof(SPI_STREAM_SEND_INFO)) != sizeof(SPI_STREAM_SEND_INFO))
		{
			ak_thread_mutex_unlock(&devif_send_mutex);
			ak_print_error("send frame header error\n");
            ret = ak_venc_release_stream(stream_handle, &stream);
			continue;
		}
        // frame packet
        opt_ptr = send_buf;
        memcpy(opt_ptr, FRAME_HEADER_MAGIC, 4);
		opt_ptr += 4;
        if(stream.frame_type == FRAME_TYPE_I) 
             *(int*)opt_ptr = FRAME_TYPE_VIDEO_I;
        else
             *(int*)opt_ptr = FRAME_TYPE_VIDEO_P;
        opt_ptr += 4;
        
        memcpy(opt_ptr, &check_sum, 4);
		//memcpy(opt_ptr, &stream.ts, 4);
		opt_ptr += 4;

        memcpy(opt_ptr, &stream.len, 4);
		opt_ptr += 4;

		memcpy(opt_ptr, stream.data, stream.len);
		opt_ptr = send_buf + sizeof(VIDEO_STREAM_HEADER) + stream.len;

#if FRAM_CHECK_SUM_ENABLE
        check_sum = 0;
		for(i = 12; i < (stream.len+16);i++){
            check_sum += send_buf[i];
        }
#else
        check_sum++;
#endif	
        memcpy(opt_ptr, &check_sum, 4);
		opt_ptr += 4;
        
		memcpy(opt_ptr, FRAME_END_MAGIC, 4);
        
        #if 0
        ak_print_normal("send vframe type %d id %d lenght %d\n", spi_trans_head.reserve,spi_trans_head.pack_id,spi_trans_head.data_len);
        for(i = 0; i < 16; i++)
			ak_print_normal("%0x ", *(send_buf + i));
        ak_print_normal("\n");
        #endif
        
        //if(frame_len >= 1024*100) {
        //    ak_print_warning("vframe langer 100K");
        //    frame_len = 100*1024;
        //}
        if(frame_len >= 1024*100) {
            ak_print_warning("====fram len larger 100k=======\n");
        }
        opt_ptr = send_buf;
		for ( transfer_progress = 0; transfer_progress < frame_len; transfer_progress += transfer_size, opt_ptr += transfer_size)
    	{
			transfer_size = MIN( SPI_MAX_TRANSFER_SIZE, ( frame_len - transfer_progress ) );
			//ak_print_normal("send frame data %d\n", transfer_size);
			if(data_send(opt_ptr , transfer_size) != transfer_size)
			{
				ak_print_error("send frame data error\n");
				break;
			}
		}
        ak_thread_mutex_unlock(&devif_send_mutex);
#endif
		ret = ak_venc_release_stream(stream_handle, &stream);
		if (ret != 0) {
			ak_print_error_ex("release stream failed\n");
		}
		
	}
	sys_video_start = 0;
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
	if(send_buf != NULL)
        free(send_buf);
	//close_file(fp);
exit2:
	ak_venc_close(venc_handle);
exit1:	
    if(vi_handle_count_global == 1)
    	ak_vi_close(vi_handle);
    vi_handle_count_global--;
	
	ak_thread_exit();
	
	return NULL;
}


/*
*read cmd from uart and process cmd
*/
void* cmd_process_task(void *arg)
{
	char buf[1024+24], *pbuf;
	unsigned short len = 1024+24;
	int ret;
	CMD_INFO *cmd;
    unsigned int frameType;
    int i;

#if AO_SAVE_TO_SD
        static int recv_frame_cnt = 0;
        struct ak_date systime;
          unsigned long free_size = 0; 
          char * path = "a:/";
          char file_path[255] = {0};
          
          free_size = ai_get_disk_free_size(path);//get T_card free size  
          if(free_size < 1024)
          {       
              ak_print_error_ex("free_size < AI_TCARD_FREE_SIZEKB\n",1024);
              //goto pcai_unmount;
          }
    
          memset(&systime, 0, sizeof(systime));
          ak_get_localdate(&systime);
          sprintf(file_path, "%sPCAI_%04d%02d%02d_%02d%02d%02d.pcm", path,systime.year, systime.month + 1, 
          systime.day + 1, systime.hour, systime.minute, systime.second);
          
          FILE *p;
          p = fopen(file_path, "w+");
#endif

    memset(&audioRecv,0,sizeof(audio_recv_ctrl_t));
    audioRecv.num = 0;
    audioRecv.index = 0;
    audioRecv.wait_recv_ok = 0;
    
	ak_print_normal("waiting for cmd...\n");
	while(1)
	{
		memset(buf, 0 ,sizeof(buf));
		ret = read_one_cmd(buf, sizeof(buf));

        ak_print_normal("recv ret %d\n",ret);
        //ak_print_normal("recv audio 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x\n",
        //buf[0], buf[1], buf[2], buf[3], buf[4], buf[5],
        //buf[6], buf[7], buf[12], buf[13], buf[14], buf[15] );

		if(ret == AK_OK)
		{
		    cmd = (CMD_INFO *)buf;
			ak_print_normal("recv cmd %d\n", cmd->cmd_id);
			switch(cmd->cmd_id)
			{
				case CMD_WAKEUP_SRC_RESP:
					wakeup_process(cmd);
					break;
				case CMD_VIDEO_PARAM:
					ak_print_normal("set video param\n");
					g_video_para.resolution = cmd->param.video_param.resolution;
					g_video_para.fps = cmd->param.video_param.fps;
					g_video_para.kbps = cmd->param.video_param.kbps;
					set_video_param(g_venc_handle, &g_video_para);
					break;
				case CMD_VIDEO_PREVIEW:
					ak_print_normal("video preview cmd\n");
					if(!sys_video_start)
						{
							ak_print_normal("post video start semaphore\n");
							ak_thread_sem_post(&sem_video_start);
						}
					break;	
				case CMD_RING_CALL_END:
				case CMD_VIDEO_PREVIEW_END:
					ak_print_normal("video stop\n");
					sys_video_start = 0;
					break;
				case CMD_SYS_CONFIG:
					sys_config();
					break;
				case CMD_SYS_CONFIG_COMPLETE:
					sys_config_complete();
					break;
				default:
					ak_print_normal("unknown cmd id %d\n", cmd->cmd_id);
				
			}
		}
#if 1
        else if(ret > 0) {  // audio 
		    audio_recv_buf_t *aRecv = &audioRecv.buf[audioRecv.index];
      

            for(i = 0; i < ret; i++) {
                if(AUDIO_RECV_BUF_READY == aRecv->state && audioRecv.wait_recv_ok){
                    if(aRecv->len < AUDIO_RECV_BUF_SIZE-24) {
                       if( buf[i] == 'F' && buf[i + 1] == 'E' && buf[i + 2] == 'N' && buf[i + 3] == 'D') 
                       {
                            aRecv->cnt -= 4;    //crc out
                            aRecv->state = AUDIO_RECV_BUF_USE;

                            ak_print_normal("audio recv id %d\n",*(unsigned int*)&(aRecv->data[aRecv->cnt]));
                            
                            //ak_print_normal("audio recv frame ok\n");
                            if(audioRecv.num < AUDIO_RECV_BUF_NUM) {
                                audioRecv.num++;
                                audioRecv.index++;
                                audioRecv.index %= AUDIO_RECV_BUF_NUM; 
                                //ak_print_normal("audio recv buf num %d\n", audioRecv.num);
                            } else {
                                ak_print_warning("audio recv buffer flowOver\n");
                            }
#if AO_SAVE_TO_SD
                            if(sd_mount_ok)
                            {
                                if(recv_frame_cnt < 300)
                                {
                                    //ak_print_normal("audio get frame cnt %d\n",recv_frame_cnt);
                                    if(NULL == p) 
                                    {
                                        ak_print_error_ex("create pcm file err\n");
                                    }
                                    if(fwrite(aRecv->buf,aRecv->len,1,p) <= 0)
                                    {
                                        ak_print_error_ex("write file err\n");        
                                    }
                                } else if(recv_frame_cnt == 300){
                                     ak_print_normal("####################################\n");
                                     ak_print_normal("         ao file close           \n");
                                     ak_print_normal("####################################\n");
                                     fclose(p);
                                }
                                recv_frame_cnt++;
                            }
#endif
                       }
                       else 
                       {
                            if(aRecv->cnt < AUDIO_RECV_BUF_SIZE)
                                aRecv->data[aRecv->cnt++] = buf[i];
                            else
                                ak_print_error_ex("audio frame lenger than recv buffer\n");
                       }
                    } else {
                       aRecv->state = AUDIO_RECV_BUF_EMPTY;
                        ak_print_error("no far recv buf\n");
                    }
                    
                }
                else if(buf[i] == 'F' && buf[i + 1] == 'R' && buf[i + 2] == 'A' && buf[i + 3] == 'M'){
                    i += 4; // header(4) and time(4) = 0
                    
                    memcpy(&frameType,&buf[i],4); // get frame type
                    i += 4;

                    if(frameType == FRAME_TYPE_AUDIO)
                    {
                      memcpy(&aRecv->time,&buf[i],4);
                      i += 4;
                      
                      /*audio fram lenght*/
                      memcpy(&aRecv->len,&buf[i],4);
                      i += 3;   // for{}  have one i++

                      //ak_print_normal("audio recv len %d\n",aRecv->len);
                      if(aRecv->state == AUDIO_RECV_BUF_EMPTY) {
                          aRecv->state = AUDIO_RECV_BUF_READY;
                          aRecv->cnt   = 0;
                      }
                  }

                } 
            }
       
        }
#endif
		else
		{
            continue;
		}
		//ak_sleep_ms(200);
	}
	
}

/*
*read cmd from uart and process cmd
*/
void* idle_check_task(void *arg)
{
	ak_print_normal_ex("id=%d\n", ak_thread_get_tid());
	
	while(1)
	{
		if(!sys_video_start)
		{
			sys_idle_time++;
			
		}
		if(sys_idle_time >= SYS_IDLE_BEFORE_SHUTDOWN )
		{
			ak_print_info("sys idle timeout %d\n", sys_idle_time);
			sys_shutdown();
		}
		ak_sleep_ms(1000);
	}
	
}

/*video thread*/
void start_video()
{
	ak_pthread_t  encode_id,audio_play_id, audio_in_id;
	int enc_ch = 2; //0:720P 1:720P 2:VGA
	
#if  (AO_SAVE_TO_SD)|| (AI_SAVE_TO_SD)||(AO_READ_FROM_SD)
    
        if (0 ==ak_mount_fs(DEV_MMCBLOCK, 0, ""))
        {
            ak_print_normal("mount sd ok!\n");
            sd_mount_ok = 1;
        }
        else
        {
            ak_print_error("mount sd fail!\n"); 
            sd_mount_ok = 0;
        }
#endif

    ak_thread_sem_init(&sem_audio_wait,1);

	ak_print_normal("start_video ***************\n");
	ak_thread_create(&encode_id,  video_encode_task, &enc_ch, 32*1024, VIDEO_TASK_PRIO);
  
    // wait for dingdong open audio
    ak_thread_sem_wait(&ring_dev_manage.sem_ao_dev);
    ak_print_normal("start_audio ***************\n");
    ak_thread_create(&audio_play_id, ao_task, NULL, ANYKA_THREAD_MIN_STACK_SIZE, AO_TASK_PRIO);
    ak_thread_create(&audio_in_id, ai_task, NULL, ANYKA_THREAD_MIN_STACK_SIZE, AI_TASK_PRIO);

    
	ak_print_normal("main encode wait work encode exit ...\n");
    
	ak_thread_join(encode_id);
    ak_print_normal("###### exit: video task ######\n");
    
	ak_thread_join(audio_play_id);
    ak_print_normal("###### exit: ao task ######\n");
    
	ak_thread_join(audio_in_id);
    ak_print_normal("###### exit: ai task ######\n");

	ak_thread_sem_destroy(&(sem_audio_wait));
    //ak_thread_mutex_destroy(&devif_send_mutex);
	
    
	
	return;
	
}

static void send_picture_data(struct video_stream *p_stream)
{

    // send
     SPI_STREAM_SEND_INFO    spi_send_buf;
     int i;
     int transfer_size, transfer_progress;
     char *send_buf = NULL;
     char *opt_ptr;
     int check_sum;
     int frame_len;

     send_buf = malloc(FRAME_SEND_BUF_LEN);
     if(send_buf == NULL)
     {
         ak_print_error("malloc buff for sending photo error\n");
         return;
     }

     frame_len = p_stream->len + sizeof(VIDEO_STREAM_HEADER) + sizeof(VIDEO_STREAM_END);
     
     //video_send_buf.header_magic = FRAME_HEADER_MAGIC;
     memcpy(spi_send_buf.header_magic, SPI_HEADER_MAGIC, 4);
     spi_send_buf.reserve= FRAME_TYPE_PICTURE;
     spi_send_buf.pack_id= (p_stream->ts);
     spi_send_buf.data_len = (frame_len);

      ak_thread_mutex_lock(&devif_send_mutex);
     // spi packet header
      if(data_send((char*)&spi_send_buf, sizeof(SPI_STREAM_SEND_INFO)) != sizeof(SPI_STREAM_SEND_INFO))
      {
          ak_thread_mutex_unlock(&devif_send_mutex);
          ak_print_error("send picture frame header error\n");
          return;
      }
      // frame packet
      opt_ptr = send_buf;
      memcpy(opt_ptr, FRAME_HEADER_MAGIC, 4);
      opt_ptr += 4;
      *(int*)opt_ptr = FRAME_TYPE_PICTURE;
      opt_ptr += 4;
      
      memcpy(opt_ptr, &p_stream->ts, 4);
      opt_ptr += 4;
     
      memcpy(opt_ptr, &p_stream->len, 4);
      opt_ptr += 4;
     
      memcpy(opt_ptr, p_stream->data, p_stream->len);
      opt_ptr = send_buf + sizeof(VIDEO_STREAM_HEADER) + p_stream->len;
      check_sum++;
      memcpy(opt_ptr, &check_sum, 4);
      opt_ptr += 4;
      memcpy(opt_ptr, FRAME_END_MAGIC, 4);

      opt_ptr = send_buf;

        
      ak_print_normal("send picture type%d time %d lenght %d\n", spi_send_buf.reserve,spi_send_buf.pack_id,frame_len);
      for(i =0;i < 16;i++)
           ak_print_normal(" %0x",opt_ptr[i]);
      ak_print_normal("\n");

      
      for ( transfer_progress = 0; transfer_progress < frame_len; transfer_progress += transfer_size, opt_ptr += transfer_size)
      {
          transfer_size = MIN( SPI_MAX_TRANSFER_SIZE, ( frame_len - transfer_progress ) );
          //ak_print_normal("send frame data %d\n", transfer_size);
          if(data_send(opt_ptr , transfer_size) != transfer_size)
          {
              ak_print_error("send picture frame data error\n");
              break;
          }
      }
      ak_thread_mutex_unlock(&devif_send_mutex);

       free(send_buf);
}
/*
*wakeup src process rutine
*
*/
int wakeup_process(CMD_INFO *cmd)
{
	if(cmd->cmd_id == CMD_WAKEUP_SRC_RESP)
	{
		switch(cmd->param.event)
		{
			case EVENT_RTC_WAKEUP:
			/*RTC wake up process*/
				ak_print_normal("RTC WAKEUP\n");
				break;
			case EVENT_PIR_WAKEUP:
                ak_thread_sem_post(&sem_dingdong_start);
				ak_print_normal("PIR WAKEUP\n exp_time: %d\n",cmd->cmd_result);
				cam_set_sensor_ae_info(&(cmd->cmd_result));
#if 1
                struct video_stream stream;
                int ret;

                ak_print_normal("----photo open...----\n");
                ret = photo_open(PHOTO_PIXEL_VGA);
                
#if START_TIME_DEBUG
                struct ak_timeval t1;
                ak_get_ostime(&t1);
                long diff = ak_diff_ms_time(&t1,&g_main_start);
                ak_print_normal("photo time %lu\n", diff);
#endif
                if(ret) {
                    ak_print_normal("photo open ok\n");
                    ret = photo_one(&stream);
                    if(ret == 0){
                        ak_print_warning("photo fail\n");
                        goto PIR_WAKEUP_exit;
                    }
                    send_picture_data(&stream);
                }
PIR_WAKEUP_exit:
                photo_close();
#endif

				ak_thread_sem_post(&sem_video_start);
				break;
			case EVENT_RING_CALL_WAKEUP:
				ak_print_normal("Ring call WAKEUP\n");
				ak_thread_sem_post(&sem_video_start);
				break;
			case EVENT_VIDEO_PREVIEW_WAKEUP:
				ak_print_normal("video preview WAKEUP\n");
                cam_set_sensor_ae_info(&(cmd->cmd_result));
				ak_thread_sem_post(&sem_video_start);
				break;
			case EVENT_SYS_CONFIG_WAKEUP:
				sys_config();
				break;
			default:
				ak_print_normal("unknown wakeup source, sleep again\n");
		}
		
	}
	
}


static void* doorbell_thread(void *arg)
{
	struct pcm_param info;
	int ret;
	unsigned int fram_size = 0;
	int *fd = NULL;
	unsigned char file_over = 0;
	int send_size, size;
	
	unsigned char* curPtr = (unsigned char*)audio_doorbell_buf;
	unsigned char* endPtr = (unsigned char*)audio_doorbell_buf + sizeof(audio_doorbell_buf);
	
	unsigned char *tmpBuf;
	info.sample_bits = 16;
	info.sample_rate = 8000;
	info.channel_num = 1;

#if START_TIME_DEBUG
        struct ak_timeval ao_open_t0;
        ak_get_ostime(&ao_open_t0);
        long diff = ak_diff_ms_time(&ao_open_t0,&g_main_start);
        ak_print_normal("ao open time0 %lu\n", diff);
#endif
	//open ao
	fd = ak_ao_open(&info);
#if START_TIME_DEBUG
            struct ak_timeval ao_open_t1;
            ak_get_ostime(&ao_open_t1);
            diff = ak_diff_ms_time(&ao_open_t1,&g_main_start);
            ak_print_normal("ao open time1 %lu\n", diff);
#endif

    ring_dev_manage.ao_handle = fd;
	if(fd == NULL)
	{
		ak_print_error("ao open false\n");
		return ;
	}
    ak_thread_sem_post(&ring_dev_manage.sem_ao_dev);
	fram_size = ak_ao_get_frame_size(fd);
	ak_print_normal("ao frame size =%d\n", fram_size);
	
	tmpBuf = malloc(fram_size);
	if(NULL == tmpBuf)
	{
		ak_print_error("ao demo malloc false!\n");
		return ;
	}
	//set volume
	ak_ao_set_volume(fd, 0x05);
    
    ak_thread_sem_wait(&sem_dingdong_start);
	ak_print_normal("star trans\n");
	ak_ao_enable_speaker(fd, 1);

#if START_TIME_DEBUG
        struct ak_timeval ding_t;
        ak_get_ostime(&ding_t);
        diff = ak_diff_ms_time(&ding_t,&g_main_start);
        ak_print_normal("dingdong time %lu\n", diff);
#endif


	while (1)
	{
		if(endPtr - curPtr>fram_size)
		{
			memcpy(tmpBuf, curPtr, fram_size);
			ret = fram_size;
			curPtr += fram_size;
		}
		else
		{
			ret = endPtr - curPtr;
			memcpy(tmpBuf, curPtr, ret);
			file_over = 1;
		}		
		send_size = 0;
		while(ret > 0)
		{
			size = ak_ao_send_frame(fd, tmpBuf + send_size, ret, 0);
			if(size < 0)
			{
				ak_print_error("ao send frame error!  %d \n", size);
				break;
			}
			else
			{
				send_size += size; 
				ret = ret - size;

			}
		}
		if(file_over)
		{
			break;
		}
	}
	mini_delay(900);
	//ak_ao_enable_speaker(fd, 0);
	//close ao
	//ak_ao_close(fd);
	free(tmpBuf);

    ak_print_normal("*****doorbell exit ******\n");
}




/*
*main process to start up
*/
void* ring_main(void *arg)
{
	ak_pthread_t id;
	ak_pthread_t doorbell_id;
	int ret;

    ret = ak_thread_sem_init(&sem_dingdong_start, 0);  
    ret = ak_thread_sem_init(&ring_dev_manage.sem_ao_dev, 0);
	/*start cmd recv thread */
	ret = ak_thread_create(&doorbell_id, doorbell_thread, NULL, 16*1024, SOUND_PLAY_TASK_PRIO);
	/*init uart for cmd communication*/
	cmd_uart_init();
	/*init spi for video data communication*/
	data_spi_init();

	
	/*create a idle timer*/
	//ak_timer_create(idle_timer);

    ret = ak_thread_mutex_init(&devif_send_mutex);
	ret = ak_thread_sem_init(&sem_video_start, 0);  
	if (ret < 0)
	{
		ak_print_error("int video start semaphore fail!\n");
		return ;
	}
	
	/*1,check battery*/
	if(ak_battery_get() < BATTERY_ALARM_THRESHOLD)
	{
		/*send alarm and then shutdown*/
		send_alarm(ALARM_BATTERY_TOO_LOW);
		sys_shutdown();
	}
	else
	{
		/*start cmd recv thread */
		ret = ak_thread_create(&id, cmd_process_task, NULL, TASK_STACK_SIZE, CMD_TASK_PRIO);
		if(ret < 0)
		{
			ak_print_error("create cmd process task fail!\n");
			return ;
		}
		/*get wake up source*/
		send_cmd_wakeup_source_check();
			
	}

	
	/*start idle timer, if time out call sys_shutdown*/
	//ak_timer_start(idle_timer, sys_shutdown);
	#if 0
	ret = ak_thread_create(&id, idle_check_task, NULL, TASK_STACK_SIZE, IDLE_TASK_PRIO);
	if(ret < 0)
	{
		ak_print_error("create idle check task fail!\n");
		return ;
	}
	#endif
	
	/*if video started system will shutdown immediately after video stop, 
	*else  system will shutdown aflter a idle time.
	*/
	while(1)
	{
		ak_print_normal("waiting for video start semaphore\n");
		ak_thread_sem_wait(&sem_video_start);
		/*start_video will run util video stop*/
		start_video();
		/*after video stop system shutdown*/
		//sys_shutdown();
		ak_sleep_ms(100);
	}
			
}


/*regist module start into _initcall */
static int ring_main_start(void) 
{
	ak_pthread_t id;
    
#if START_TIME_DEBUG
    ak_get_ostime(&g_main_start);
#endif

    //we just support g711 decode/encode here
    //if need to add other format, call funcs in ak_login.h
    ak_login_g711_decode();
    ak_login_g711_encode();
	ak_login_all_filter();
    
    //ak_login_all_encode();
	//ak_login_all_decode();

	ak_thread_create(&id, ring_main, NULL, TASK_STACK_SIZE, MAIN_TASK_PRIO);
	return 0;
}


cmd_module_init(ring_main_start)


