/*
*
*ring_main.c  first app statarted for ring call
*author:wmj
*date: 20161214
*/
#include "command.h"
#include "sockets.h"
#include "ipv4/inet.h"
#include "ipv4/ip.h"
#include "ipv4/ip_addr.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_vi.h"
#include "ak_venc.h"
#include "ak_drv_ircut.h"
#include "ak_ao.h"
#include "ak_ai.h"
#include "dev_info.h"
#include "kernel.h"


#define AO_SAVE_TO_SD       0
#define AI_SAVE_TO_SD       0
#define AO_READ_FROM_SD     0
static bool sd_mount_ok = 0;


#define VIDEO_SAVE_PATH        "a:"
#define VIDEO_FILE_NAME        "video.h264"

#define FIRST_PATH       "ISPCFG"
#define BACK_PATH        "ISPCFG"

#define VIDEO_VGA_WIDTH				640
#define VIDEO_VGA_HEIGHT			360
#define VIDEO_VGA_HEIGHT_960P		480


/*task priority */
#define MAIN_TASK_PRIO 				80
#define VIDEO_TASK_PRIO 			75
#define IDLE_TASK_PRIO				20
//#define AO_TASK_PRIO		        71
#define AO_TASK_PRIO		        85

#define AI_TASK_PRIO		        70


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

#define VIDEO_TCP_PORT  5005;
#define VIDEO_TCP_ADDR  "192.168.0.2"

#define TCP_SERVER_TASK_SIZE  (4096)
#define TCP_SERVER_TASK_PRIO   20

#define CMD_VIDEO_ARG       0x01
#define CMD_VIDEO_START     0x02
#define CMD_VIDEO_STOP      0x03

#define RESOLU_VGA          0
#define RESOLU_720P         1
#define RESOLU_960P         2

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


// tcp server
static unsigned int    g_ipaddr;
static unsigned short  g_port;
static int             g_tcp_socket_id;


static char *help_camview[]={
	"camview demo.",
	"usage: camview [ip] [port]\n"
};


/** Find minimum value */
#define MIN(a, b) ((a) < (b) ? (a) : (b))


/*globle flag for video reques*/

static int sys_video_stop = 0;
static int sys_video_start = 0;

static ak_sem_t  sem_video_start;
static ak_mutex_t tcp_socket_mutex;           //tcp socket mutex


static int sys_idle_time =0;

struct  video_param
{
    unsigned char  resolution;
    unsigned char  fps;
    unsigned short kbps;
};

struct  venc_param
{
    int width;
    int height;
    int fps;
    int kbps;
};


static struct video_param g_video_para = {1, 8, 800};

static struct venc_param g_rec_para;


static bool is_vga = false;


/*video stream define*/
typedef struct _stream_header {
	unsigned int header_magic;
    unsigned int frameType;
    unsigned int time;
	unsigned int frame_len;
} VIDEO_STREAM_HEADER,AUDIO_STREAM_HEADER;

/*video stream define*/
typedef struct _stream_end {
	unsigned int frame_crc;
	unsigned int end_magic;
}  VIDEO_STREAM_END,AUDIO_STREAM_END;



static void *open_file()
{
	char file[100] = {0};
	sprintf(file, "%s/%s", VIDEO_SAVE_PATH, VIDEO_FILE_NAME);

	FILE *filp = NULL;
	
	if (0 !=ak_mount_fs(DEV_MMCBLOCK, 0, ""))
    {
		ak_print_error("sd card mount failed!\n");
        return ;
    }
	
	filp = fopen(file, "w");
	if (!filp) {
		ak_print_error_ex("fopen, error\n");
	}

	return filp;
}

static void close_file(FILE *filp)
{
	fclose(filp);
	ak_unmount_fs(DEV_MMCBLOCK, 0, "");
}

static unsigned int crc_checksum(char *data, int len)
{
	int i = 0;
	unsigned int checksum = 0;
	for(i = 0; i < len; i++)
	{
		checksum += data[i];
	}
	ak_print_normal("crc begin %0x %0x %0x %0x len %d\n", data[0], data[1], data[2], data[3], len);
	return checksum;
}


/*
*@brief:  tcp_client_task
*@return; socket id
*/
int tcp_client_connect(void *args)
{
    int socket_s = -1;
	int err = -1;
	struct sockaddr_in *net_cfg = (struct sockaddr_in *)args;
		
	net_cfg->sin_family = AF_INET;
	net_cfg->sin_len = sizeof(struct sockaddr_in);
	
	socket_s = socket(AF_INET, SOCK_STREAM, 0);//tcp socket
	if(socket_s < 0)
	{
		ak_print_error("create socket error.\n");
		
	}
	else
	{
		err = connect(socket_s, (struct sockaddr*)net_cfg, sizeof(struct sockaddr));
		if (err != 0)
		{
			ak_print_error("connect to %s:%d error\n", inet_ntoa(net_cfg->sin_addr), net_cfg->sin_port);
			closesocket(socket_s);
			socket_s = -1;
		}
		else
		{
			g_tcp_socket_id = socket_s;
			ak_thread_sem_post(&sem_video_start);
			/*start cmd recv task*/
			//tcp_recv_task(NULL);
		}
	}
	
	return socket_s;
}

/*********************************
*@brief:     tcp_server_task
*@param:  args  (struct sockaddr_in)
*@return: 
*********************************/
int tcp_server_task(void *args)
{
	
    int socket_s = -1;
	int new_socket = -1;
	struct sockaddr remote_addr;
	struct sockaddr_in *net_cfg;
	int sockaddr_len;
	unsigned long  sendcnt=0;
	int opt;
	int err = -1;
	
	net_cfg = (struct sockaddr_in *)args;
	
	net_cfg->sin_family = AF_INET;
	net_cfg->sin_len = sizeof(struct sockaddr_in);
	net_cfg->sin_addr.s_addr = INADDR_ANY;
	
	socket_s = socket(AF_INET, SOCK_STREAM, 0);//tcp socket
	if (socket_s < 0)
	{
		ak_print_error("create socket error.\n");
		
	}
	else
	{
	    //使server 能立即重用
		opt = SOF_REUSEADDR;
		if (setsockopt(socket_s, SOL_SOCKET, SOF_REUSEADDR, &opt, sizeof(int)) != 0) 
		{
			printf("set opt error\n");
		}
        else
        {
			err = bind(socket_s, (struct sockaddr*) net_cfg, sizeof(struct sockaddr_in));
			if (err !=0)
			{
				ak_print_error("bind socket error\n");
			}
			else
			{
				err = listen(socket_s, 4);
				if (err !=0 )
				{
					ak_print_error("listen socket error\n");
				}
	            else
	            {
	            	while(1)
	            	{
		                sockaddr_len = sizeof(struct sockaddr);
						ak_print_normal("waiting client to connect...\n");
						new_socket = accept((int)socket_s, (struct sockaddr*) &remote_addr, (socklen_t*) &sockaddr_len);
						if (new_socket < 0)
						{
							ak_print_error("accept socket error\n");
						}
		                else
		                {
		                	ak_print_debug("remote addr:%s. connected\n", remote_addr.sa_data);
		                	//return new_socket;
		                	g_tcp_socket_id = new_socket;
							//ak_thread_sem_post(&sem_video_start);
							tcp_recv_task(NULL);
		                }
	            	}
				}
			}
        }
	}
	
	if (-1 != socket_s)
	{
		closesocket(socket_s);
	}	
}

/*********************************
*@brief:     tcp_recv_task
*@param:  args  
*@return: 
*********************************/

int tcp_recv_task(void *args)
{
	char recv_buf[1024+32] = {0};
	int len;
    unsigned int frameType;
	unsigned short cmd_id;
	unsigned short cmd_arg_len;
	int i;
#if 0
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
#endif
    memset(&audioRecv,0,sizeof(audio_recv_ctrl_t));
    audioRecv.num = 0;
    audioRecv.index = 0;
    audioRecv.wait_recv_ok = 0;
	while(1)
	{
		len = recv_data(recv_buf, sizeof(recv_buf));
		if(len > 0)
		{
		    audio_recv_buf_t *aRecv = &audioRecv.buf[audioRecv.index];
			/*parse data check header  "VCMD" + cmd id + cmd arg len + cmd arg + crc + "CEND"*/
		//ak_print_normal("recv from socket 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x 0x%0x\n",
		//recv_buf[0], recv_buf[1], recv_buf[2], recv_buf[3], recv_buf[4], recv_buf[5],
		//recv_buf[6], recv_buf[7], recv_buf[8], recv_buf[9], recv_buf[11], recv_buf[12] );
			for(i = 0; i < len; i++)
			{
				if(AUDIO_RECV_BUF_READY == aRecv->state && audioRecv.wait_recv_ok){
                    if(aRecv->len > 0) {
                       if( recv_buf[i] == 'F' && recv_buf[i + 1] == 'E'\
                        && recv_buf[i + 2] == 'N' && recv_buf[i + 3] == 'D') 
                       {
                            aRecv->cnt -= 4;    //crc out
                            aRecv->state = AUDIO_RECV_BUF_USE;
                            
                            if(audioRecv.num < AUDIO_RECV_BUF_NUM) {
                                audioRecv.num++;
                                audioRecv.index++;
                                audioRecv.index %= AUDIO_RECV_BUF_NUM; 
                                //ak_print_normal("audio recv buf num %d\n", audioRecv.num);
                            } else {
                                ak_print_warning("audio recv buffer flowOver\n");
                         }
#if 0
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
                                    if(fwrite(aRecv->data,aRecv->len,1,p) <= 0)
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
#endif
                       }
                       else 
                       {
                            if(aRecv->cnt < AUDIO_RECV_BUF_SIZE)
                                aRecv->data[aRecv->cnt++] = recv_buf[i];
                            else
                                ak_print_error_ex("audio frame lenger than recv buffer\n");
                       }
                    } else {
                       aRecv->state = AUDIO_RECV_BUF_EMPTY;
                        ak_print_error("no far recv buf\n");
                    }
                  
                }
                else if(recv_buf[i] == 'F' && recv_buf[i + 1] == 'R' && recv_buf[i + 2] == 'A' && recv_buf[i + 3] == 'M') 
				{
                    i += 4; // header(4) and time(4) = 0

                    memcpy(&frameType,&recv_buf[i],4); // get frame type
                    i += 4;
                    
                    if(frameType == FRAME_TYPE_AUDIO)
                    {
                        memcpy(&aRecv->time,&recv_buf[i],4);
                        i += 4;
                        
                        /*audio fram lenght*/
                        memcpy(&aRecv->len,&recv_buf[i],4);
                        i += 3;   // for{}  have one i++
						if(aRecv->state == AUDIO_RECV_BUF_EMPTY) {
							aRecv->state = AUDIO_RECV_BUF_READY;
							aRecv->cnt	 = 0;  
						}
             
                    }

				}
                else if(recv_buf[i] == 'V' && recv_buf[i + 1] == 'C' && recv_buf[i + 2] == 'M' && recv_buf[i + 3] == 'D') 
                {
    				i += 4;
    				/*check crc and cmd end*/
    				
    				
    				/*parse cmd id*/
    				cmd_id = recv_buf[i];
    				ak_print_normal("Cmd id 0x%0x\n", cmd_id);
    				i += 1;  /*1 bytes cmd id */
    				cmd_arg_len = recv_buf[i];
    				i += 1;  /*1 bytes cmd arg len*/
    				switch(cmd_id)
    				{
    					case CMD_VIDEO_ARG:
    						g_video_para.resolution = recv_buf[i];
    						i += 1;
    						g_video_para.fps = recv_buf[i];
    						i += 1;
    						g_video_para.kbps = *(unsigned short*)&recv_buf[i];
    						break;
    					case CMD_VIDEO_START:
    						ak_print_normal("video start\n");
    						if(!sys_video_start)
    						{
    							ak_print_normal("post video start semaphore\n");
    							ak_thread_sem_post(&sem_video_start);
    						}
    						break;
    					case CMD_VIDEO_STOP:
    						ak_print_normal("video stop\n");
    					    sys_video_start = 0;
    						break;
    					default:
    						ak_print_normal("unkown cmd\n");
    						break;	
				    }
                }
                else 
                {
                    continue;
                }

			}
			
		}
		else
		{
			ak_print_error("recv data from tcp socket error\n");
			break;
		}
	}
}


/*
*@brief:  send_data
*@param: data
*@param: data_len
*@return; send len
*/
int send_data(char *data, int data_len)
{
	int sendlen = 0;

    ak_thread_mutex_lock(&tcp_socket_mutex);

	sendlen = send(g_tcp_socket_id, data, data_len, MSG_WAITALL);
	if (sendlen < 0)
	{
		ak_print_error("send data from socket %d error\n", g_tcp_socket_id);
	}
    
    ak_thread_mutex_unlock(&tcp_socket_mutex);
	return sendlen;
}


/*
*@brief:  send audio data
*@param: data
*@param: data_len
*@return; send len
*/
static int send_audio_data(int time,int data_len,char *data)
{
    static char *opt_ptr;
    unsigned int check_sum = 0;
    int ret = 0;

    if(audioSend.cnt == 0) {
        opt_ptr = &audioSend.data[0];
        memcpy(opt_ptr, FRAME_HEADER_MAGIC, 4);
        opt_ptr += 4;
        *(int*)opt_ptr  = FRAME_TYPE_AUDIO;
        opt_ptr += 4;
        memcpy(opt_ptr, &time, 4);
        opt_ptr += 4;
        memcpy(opt_ptr, &data_len, 4);
    	opt_ptr += 4;
    } 
    
    memcpy(opt_ptr, data, data_len);
    audioSend.cnt += data_len;
    opt_ptr += data_len;

    if(audioSend.cnt >= 512) {
        //ak_print_normal("ai send to pc lenght %d\n",data_len);
        check_sum++;
        memcpy(opt_ptr, &check_sum, 4);
        opt_ptr += 4;
        memcpy(opt_ptr, FRAME_END_MAGIC, 4);
        ret = send_data(&audioSend.data[0],audioSend.cnt + sizeof(AUDIO_STREAM_HEADER)+8);
        audioSend.cnt = 0;
    }

    return ret;
}


/*
*@brief:  recv_data
*@param: buff
*@param: buff_len
*@return; recv len
*/
int recv_data(char *buff, int buff_len)
{
	int len;
	
	len = recv(g_tcp_socket_id, buff, buff_len, MSG_WAITALL);
	return len;
	
}



static void save_stream(FILE *filp, unsigned char *data, int len)
{
	int ret = len;
	
	do {
		ret -= fwrite(data, 1, ret, filp);
	} while (ret > 0);
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

static void *camera_initial(const char *first, const char *second)
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

	/* set crop information */
	struct video_channel_attr *attr = (struct video_channel_attr *)calloc(1,
			sizeof(struct video_channel_attr));
	attr->res[VIDEO_CHN_MAIN].width = g_rec_para.width;
	attr->res[VIDEO_CHN_MAIN].height = g_rec_para.height;
	attr->res[VIDEO_CHN_SUB].width = VIDEO_VGA_WIDTH;

	/* if sensor support 960P, vga height need to be 480 */
	if(resolution.height == 960)
	{
		attr->res[VIDEO_CHN_SUB].height = VIDEO_VGA_HEIGHT_960P;
		if( is_vga )
			g_rec_para.height = VIDEO_VGA_HEIGHT_960P;
	}
	else
	{
		attr->res[VIDEO_CHN_SUB].height = VIDEO_VGA_HEIGHT;
	}
	
	attr->crop.left = 0;
	attr->crop.top = 0;
	attr->crop.width = resolution.width;
	attr->crop.height = resolution.height;
	if (ak_vi_set_channel_attr(handle, attr))
		ak_print_error_ex("set channel attribute failed\n");

	/* get crop */
	struct video_channel_attr *cur_attr = NULL;
	cur_attr = (struct video_channel_attr *)malloc(sizeof(struct video_channel_attr));
	if (ak_vi_get_channel_attr(handle, cur_attr)) {
		ak_print_normal("ak_vi_get_channel_attr failed!\n");
	}

	free(cur_attr);
	free(attr);
	
	/* don't set camera fps in demo */
	ak_vi_set_fps(handle, g_rec_para.fps);
	ak_print_normal("capture fps: %d\n", ak_vi_get_fps(handle));

	return handle;
}


static void *video_encode_initial()
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

//unsigned char *readpcmBuf = NULL;

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
	ak_ao_set_volume(handle, 3);	
	ak_ao_enable_speaker(handle,1);
	
	//ak_print_normal("begin ao............\n");

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
	  sprintf(file_path, "%sPCAI_%04d%02d%02d_%02d%02d%02d.pcm", path,systime.year, systime.month , 
	  systime.day , systime.hour, systime.minute, systime.second);
	  
	  FILE *p;
	  p = fopen(file_path, "w+");
#endif

    memset(&audioRecv,0,sizeof(audio_recv_ctrl_t));
    audioRecv.num = 0;
    audioRecv.index = 0;

    audioRecv.wait_recv_ok = 1;
	ak_print_normal("begin ao............\n");
	ak_thread_sem_wait(&sem_audio_wait);
	
	while(1)
	{		
				
		if(!sys_video_start)
		{
			break;
		}

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
			
			//ak_print_error_ex("cnt %d\n",aRecv->cnt);
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
    pcm_indx = 0;
	ak_ao_close(handle);
	ak_print_normal("ao close !\n");
	ak_thread_exit();	
}		
#endif
#if 1
void* ai_task()
{
	//ai parma	
	int ai_ret = -1;
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
    sprintf(file_path, "%sAOPC_%04d%02d%02d_%02d%02d%02d.pcm", path,systime.year, systime.month , 
    systime.day , systime.hour, systime.minute, systime.second);

    FILE *p;
    p = fopen(file_path, "w+");
#endif


	//g_rec_end = 0;
	//ak_print_normal("wait ai open...\n");
	
	void *adc_drv = ak_ai_open(&input);
	ak_ai_set_source(adc_drv, AI_SOURCE_MIC);
	/*
		AEC_FRAME_SIZE	is	256，in order to prevent droping data,
		it is needed  (frame_size/AEC_FRAME_SIZE = 0), at  the same 
		time, it  need	think of DAC  L2buf  , so  frame_interval  should  
		be	set  32ms。
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
	
	ak_ai_set_aec(adc_drv, 1);
	
    ak_print_normal("ai open ...\n");
	ak_thread_sem_post(&sem_audio_wait);
	
	while(1)
	{
		if(!sys_video_start)
		{
			ak_ai_release_frame(adc_drv, &frame);			
			goto ai_close;
		}
        
		

		#if 1
		ai_ret = ak_ai_get_frame(adc_drv, &frame, 1);       
		if(ai_ret == -1)
		{
			ak_sleep_ms(5);
			continue;							
		}
		#endif
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
	ak_ai_close(adc_drv);
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
	VIDEO_STREAM_HEADER video_frame_header;
	VIDEO_STREAM_END video_frame_end;
	struct video_resolution resolution = {0};
	char *send_buf = NULL;
	char *opt_ptr;
	int check_sum;
	int i;

    unsigned short result = 896;
    cam_set_sensor_ae_info(&(result));

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
	g_rec_para.fps = g_video_para.fps;
	g_rec_para.kbps = g_video_para.kbps;
	
	ak_print_normal_ex("start a work encode\n");

	vi_handle = camera_initial(FIRST_PATH, BACK_PATH);
	if (vi_handle == NULL) {
		ak_print_error_ex("video input init faild, exit\n");
		ak_thread_exit();
		return NULL;
//		goto exit1;
	}
	
	/* get camera resolution */
	if (ak_vi_get_sensor_resolution(vi_handle, &resolution))
		ak_print_error_ex("get sensor resolution failed\n");
		
	
	/* validate camera resolution*/
	if(g_rec_para.width > resolution.width
					|| g_rec_para.height > resolution.height)
	{
		ak_print_error_ex("camera resolution not support\n");
		goto exit1;
	}
	
	venc_handle = video_encode_initial();
	if (venc_handle == NULL) {
		ak_print_error_ex("video encode open failed!");
		goto exit1;
	}
	
#if 0
	FILE *fp = open_file();
	if (fp ==NULL)
	{
		ak_print_error_ex("open file failed!");
		goto exit2; 	
	}
#endif 

	stream_handle = ak_venc_request_stream(vi_handle,venc_handle );
	if (stream_handle == NULL) {
		ak_print_error_ex("request stream failed\n");
		goto exit;
	}

	
#if 1
	send_buf = malloc(FRAME_SEND_BUF_LEN);
	if(send_buf == NULL)
	{
		ak_print_error("malloc buff for sending frame error\n");
		goto exit;
	}
#endif

	sys_video_start = 1;
	
	check_sum = 0;
   	while (sys_video_start) {
		struct video_stream stream = {0};
		int ret = ak_venc_get_stream(stream_handle, &stream);
		if (ret != 0) {
			//ak_print_error_ex("get stream failed\n");
			ak_sleep_ms(3);
			continue;
		}

		//ak_print_normal("get stream, size: 0x%0x, type=%d, ts=%d\n",
		//	 stream.len, stream.frame_type, stream.ts);
		//test
		//save_stream(fp, stream.data, stream.len);

#if 1
		opt_ptr = send_buf;
		//video_frame_header = (VIDEO_STREAM_HEADER*)send_buf;
		//video_frame_header->header_magic = htonl(FRAME_HEADER_MAGIC);
		//video_frame_header->frame_len = (stream.len);
		memcpy(opt_ptr, FRAME_HEADER_MAGIC, 4);
		opt_ptr += 4;
        if(stream.frame_type == FRAME_TYPE_I) 
             *(int*)opt_ptr = FRAME_TYPE_VIDEO_I;
        else
             *(int*)opt_ptr = FRAME_TYPE_VIDEO_P;
        opt_ptr += 4;
        
		memcpy(opt_ptr, &stream.ts, 4);
		opt_ptr += 4;

        memcpy(opt_ptr, &stream.len, 4);
		opt_ptr += 4;

		memcpy(opt_ptr, stream.data, stream.len);
		opt_ptr = send_buf + sizeof(VIDEO_STREAM_HEADER) + stream.len;
		//video_frame_end = (VIDEO_STREAM_END*)(send_buf + sizeof(VIDEO_STREAM_HEADER) + stream.len);
		//video_frame_end->end_magic = htonl(FRAME_END_MAGIC);
		//check_sum = crc_checksum(send_buf + sizeof(video_frame_header->header_magic), sizeof(video_frame_header->frame_len) + stream.len);
		//video_frame_end->frame_crc = (check_sum);
		check_sum++;
		memcpy(opt_ptr, &check_sum, 4);
		opt_ptr += 4;
		memcpy(opt_ptr, FRAME_END_MAGIC, 4);

		#if 0
		//ak_print_normal("send frame len %d, crc 0x%0x\n",video_frame_header->frame_len, check_sum);
		ak_print_normal("frame header\n");
		for(i = 0; i < 12; i++)
			ak_print_normal("%0x ", *(send_buf + i));
		ak_print_normal("\n");

		ak_print_normal("frame end crc 0x%0x \n", check_sum);
		for(i = 0 ; i < 8; i++)
			ak_print_normal("%0x ", *(send_buf + stream.len + 8 + i));
		ak_print_normal("\n");
		#endif

        //ak_print_normal("video type 0x%0x  ",stream.frame_type);
        //ak_print_normal("time 0x%0x   ",stream.ts);
        //ak_print_normal("lenght 0x%0x\n",stream.len);
		if(send_data(send_buf, stream.len + sizeof(VIDEO_STREAM_HEADER) + sizeof(VIDEO_STREAM_END)) < 0)
		{
			ak_print_error("send video frame error\n");
			ak_venc_release_stream(stream_handle, &stream);
			break;
		}
        //ak_print_normal("video end\n");
#endif 	

#if 0
		memcpy(&video_frame_header.header_magic, FRAME_HEADER_MAGIC, 4);
		video_frame_header.frame_len = stream.len;

		//check_sum = crc_checksum(&video_frame_header.frame_len, 4);
		//check_sum += crc_checksum(stream.data, stream.len);
		check_sum++;
		video_frame_end.frame_crc = check_sum;
		memcpy(&video_frame_end.end_magic, FRAME_END_MAGIC, 4);
		
		
		if(send_data((char*)&video_frame_header, sizeof(VIDEO_STREAM_HEADER)) < 0)
		{
			ak_print_error("send frame header error\n");
			ak_venc_release_stream(stream_handle, &stream);
			break;
		}
		if(send_data(stream.data, stream.len) < 0)
		{
			ak_print_error("send frame data error\n");
			ak_venc_release_stream(stream_handle, &stream);
			break;
		}
		if(send_data((char*)&video_frame_end, sizeof(VIDEO_STREAM_END)) < 0)
		{
			ak_print_error("send frame end error\n");
			ak_venc_release_stream(stream_handle, &stream);
			break;
		}
#endif

		ret = ak_venc_release_stream(stream_handle, &stream);
		if (ret != 0) {
			ak_print_error_ex("release stream failed\n");
		}
		//break; /*test*/
		
	}
	sys_video_start = 0;
	ak_print_normal_ex("cancel stream ...\n");
	ak_venc_cancel_stream(stream_handle);

exit:
	//close_file(fp);
	
exit2:
	ak_venc_close(venc_handle);
exit1:	
	ak_vi_close(vi_handle);
	
	
	ak_thread_exit();
	
	return NULL;
}


/*video thread*/
void start_video()
{
	ak_pthread_t  encode_id, audio_play_id, audio_in_id;
	int enc_ch = 2; //0:720P 1:720P 2:VGA
		
	ak_print_info("start_video ***************\n");

    //memset(audioRecv,0,sizeof(audio_recv_ctrl_t));

#if  (AO_SAVE_TO_SD)|| (AI_SAVE_TO_SD || AO_READ_FROM_SD)

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

	ak_thread_mutex_init(&tcp_socket_mutex);

	ak_thread_create(&encode_id,  video_encode_task, &enc_ch, 32*1024, VIDEO_TASK_PRIO);
	ak_thread_create(&audio_play_id, ao_task, NULL, ANYKA_THREAD_MIN_STACK_SIZE, AO_TASK_PRIO);
	ak_thread_create(&audio_in_id, ai_task, NULL, ANYKA_THREAD_MIN_STACK_SIZE, AO_TASK_PRIO);
	
	ak_print_normal("main encode wait work encode exit ...\n");
	ak_thread_join(encode_id);	
	ak_thread_join(audio_play_id);
	ak_thread_join(audio_in_id);

	ak_thread_mutex_destroy(&tcp_socket_mutex);
#if (AO_SAVE_TO_SD)|| (AI_SAVE_TO_SD || AO_READ_FROM_SD) 
    if (0 ==ak_unmount_fs(DEV_MMCBLOCK, 0, ""))
		ak_print_normal("unmount sd ok!\n");
	else
		ak_print_error("unmount sd fail!\n");
#endif

	ak_print_normal("###### exit: video task ######\n");	
	ak_print_normal("###### exit: ai task ######\n");
	ak_print_normal("###### exit: ao task ######\n");
	return;

}


/*
*main process to start up
*/

void* camview_main(int argc, char **args)
{
	int ret;
	int id;
#if 0	
	unsigned long file_len;
	FILE * read ;
	read = fopen("a:/test1_8k.pcm","r");
	fseek(read, 0, SEEK_END);
	file_len = ftell(read);
	fseek(read, 0, SEEK_SET);
	readpcmBuf = (unsigned short *)malloc(file_len+1);
	fread(readpcmBuf, file_len, 1, read);
	fclose(read);
#endif	
	struct sockaddr_in sockaddr;
	
	if(argc < 2)
	{
		ak_print_error("%s\n", help_camview[1]);
		return NULL;
	}
	
	//ipaddr
	if ((char *)args[0] != NULL)
	{
		g_ipaddr = inet_addr((char *)args[0]);
		if (IPADDR_NONE == g_ipaddr)
		{
		   ak_print_error("set remote_ipaddr wrong.\n");
		   return NULL;
		}
	}
	
	//port
	if ((int *)args[1] != NULL)
	{
		g_port = atoi(args[1]);
		if(g_port > 65535)
		{
			ak_print_error("port should less than 65535\n");
			return NULL;
		}
	}
		
	sockaddr.sin_addr.s_addr = g_ipaddr;
	sockaddr.sin_port = htons(g_port);
	
	ret = ak_thread_sem_init(&sem_video_start, 0);
	if(ret < 0)
	{
		ak_print_error("init sem_video_start error \n");
		return NULL;
	}
	ak_thread_sem_init(&sem_audio_wait,0);//set  semaphore is 1,in order to prevent dac data not enough

#if 1
	ret = tcp_client_connect((void*)&sockaddr);
	if(ret < 0)
	{
		ak_print_error("connect to video server error \n");
		return NULL;
	}
	ret = ak_thread_create(&id, tcp_recv_task, NULL, TCP_SERVER_TASK_SIZE, TCP_SERVER_TASK_PRIO);
	if(ret < 0)
	{
		ak_print_error("create tcp_client_task error \n");
		return NULL;
	}
#endif


#if 0
	ret = ak_thread_create(&id, tcp_server_task, (void*)&sockaddr, TCP_SERVER_TASK_SIZE, TCP_SERVER_TASK_PRIO);
	if(ret < 0)
	{
		ak_print_error("create tcp_server_task error \n");
		return NULL;
	}
#endif	
	while(1)
	{
		ak_print_normal("waiting for video start semaphore\n");
		if(ak_thread_sem_wait(&sem_video_start)) {
            continue;
        }
          
		/*start_video will run util video stop*/
		start_video();
		/*after video stop system shutdown*/
		//sys_shutdown();
		ak_sleep_ms(100);
	}

    ak_thread_sem_destroy(&(sem_audio_wait));
    ak_thread_sem_destroy(&(sem_video_start));

    //free(readpcmBuf);
}


/*regist module start into _initcall */
static int camview_main_start(void) 
{
	ak_pthread_t id;

    //login all audio types and filter here
    //switch to call individual login func if need to reduce bin size
	ak_login_all_filter();
	ak_login_all_encode();
	ak_login_all_decode();

	cmd_register("camview", camview_main, help_camview);
	
	return 0;
}


cmd_module_init(camview_main_start)


