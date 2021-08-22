#define ITS_IMPLEMENT			1

#if	ITS_IMPLEMENT
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <signal.h>

#include "isp_cfg_file.h"
#include "isp_basic.h"
#include "list.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_vi.h"
#include "isp_vi.h"

#define ISP_PORT				8000		/** tcp port **/
#define ISP_ATTR_ID_SIZE		2	//sub file number
#define ISP_CMD_TYPE_SIZE		2

#define ISP_PACKET_HEAD_SIZE	(ISP_ATTR_ID_SIZE+ISP_CMD_TYPE_SIZE)	//head size
#define ISP_PARM_LEN_SIZE		4
#define ISP_RET_SIZE			4
#define ISP_PARM_CNT_SIZE		4

#define HEARTBEAT_CHECKTIME		(15)

#define exit_err(msg) \
	do{ perror(msg); return 0; } while(0);

#define YUV_SAVE_PATH	"/tmp/"	//default yuv save path

/* command define */
typedef enum {
	CMD_GET = 0,
	CMD_REPLY,
	CMD_SET,
	CMD_RET,
	CMD_GET_TXT,
	CMD_REPLY_TXT,
	CMD_TYPE_NUM
} T_CMD_TYPE;

/* head state define */
typedef enum {
	HEAD_ERR = 0,
	HEAD_NO_DATA,
	HEAD_HAVE_DATA,
	HEAD_TYPE_NUM
} T_HEAD_TYPE;

/* command define */
typedef enum {
	ISP_PARM_CODE = ISP_SENSOR + 1,	//����������
	ISP_REGISTER,					//�Ĵ�������
	ISP_RAW_IMG,					//һ֡rawͼ������
	ISP_YUV_IMG,					//һ֡yuvͼ������
	ISP_ENCODE_IMG,					//һ֡encodeͼ������
	ISP_CFG_DATA,					//cfg data
	ISP_HEARTBEAT,					//heartbeat
	ISP_ATTR_TYPE_NUM
} T_ATTR_TYPE;

/* global handle */
typedef struct _ctl_handle
{
	int		sock;				//socket handle
	sem_t	isp_sem;			//operate semaphore
	struct list_head queue;		//command queue
	ak_mutex_t lock;			//mutex lock
}CTL_HANDLE, *PCTL_HANDLE;

/* command head define */
typedef struct _cmd_head
{
	unsigned short attr_id;
	unsigned short cmd_type;
}CMD_HEAD;

/* transform data node */
typedef struct _trans_data
{
	CMD_HEAD	head;
	unsigned long		datalen;
	unsigned char* 		data;
}TRANS_DATA, *PTRANS_DATA;

/* command node define */
struct cmd_node {
	unsigned char *send_buf;
	struct list_head list;
};

static unsigned char its_run_flag = AK_FALSE;	//global run flag
static int cfd = -1;	/** socket file descriptor **/
static int sfd = -1;	/** server socket file descriptor **/
static const char its_version[] = "libplat_its V1.0.03";

/**
 * brief: set check timer, to check heartbeat timeout
 * @sec[IN]:second
 * return: void
 * notes:
 */
static void set_check_timer(int sec)
{
	struct itimerval s_time;

	/* just use secord scope */
	s_time.it_value.tv_sec  = sec;
	s_time.it_value.tv_usec = 0;
	s_time.it_interval.tv_sec  = 0;
	s_time.it_interval.tv_usec = 0;

	setitimer(ITIMER_REAL, &s_time, NULL);

	return ;
}

/**
 * brief: alarm signal callback 
 * @msg[IN]:message
 * return: void
 * notes:
 */
static void heartbeat_timeout(int msg)
{
	switch (msg){
		case SIGALRM:
			printf("heartbeat timeout!\n");
			if (-1 != cfd) {
				close(cfd);
				cfd = -1;
			}
			set_check_timer(0);
			break;

		default:
			break;
	}
	return ;
}

/**
 * brief: check cmd head info
 * @cmd[IN]:cmd info
 * return: enum T_HEAD_TYPE
 * notes:
 */
static T_HEAD_TYPE check_cmd_head(PTRANS_DATA cmd)
{
	/* first check attr id */
	if (ISP_HEARTBEAT != cmd->head.attr_id)
		ak_print_info("attr_id : %d, cmd_type : %d\n",
				cmd->head.attr_id, cmd->head.cmd_type);

	/* attr id error */
	if ((cmd->head.attr_id >= ISP_ATTR_TYPE_NUM)
			|| ((cmd->head.cmd_type != CMD_GET)
				&& (cmd->head.cmd_type != CMD_SET)
				&& (cmd->head.cmd_type != CMD_GET_TXT))) {
		ak_print_warning_ex("recv attr id or cmd type error:%d, %d!\n",
				cmd->head.attr_id, cmd->head.cmd_type);
		return HEAD_ERR;
	}

	/* check whether it is data */
	if ((CMD_SET == cmd->head.cmd_type)
			|| ((ISP_PARM_CODE == cmd->head.attr_id)
				|| (ISP_REGISTER == cmd->head.attr_id))) {
		return HEAD_HAVE_DATA;
	}

	return HEAD_NO_DATA;
}

/**
 * brief: get yuv img
 * @buf[OUT]:buf to load img data
 * @size[OUT]:data size
 * return: void
 * notes:
 */
static void Get_YUV_Img(unsigned char* buf, unsigned int* size)
{
    int ret = -1;
    int cnt = 0;
    
	ak_print_notice_ex("entry\n");

	/* get vi handle */
	void *vi_handle = ak_vi_get_handle(VIDEO_DEV0);
	if (!vi_handle) {
		ak_print_warning_ex("get vi handle failed\n");
		return;
	}
	/* alloc frame struct */
	struct video_input_frame *frame = (struct video_input_frame *)calloc(1,
			sizeof(struct video_input_frame));
	if (!frame) {
		ak_print_warning_ex("calloc video_input_frame failed\n");
		return;
	}

	/* get yuv and store to buf */
    while (cnt < 20) {
    	ret = ak_vi_get_frame(vi_handle, frame);
    	if (!ret) {
    		ak_print_notice_ex("get frame ok\n");

    		*size = frame->vi_frame[VIDEO_CHN_MAIN].len;
    		memcpy(buf, frame->vi_frame[VIDEO_CHN_MAIN].data, *size);
    		ak_vi_release_frame(vi_handle, frame);
            break;    	
    	}
        else {
            ak_sleep_ms(10);
            cnt++;
        }
    }

	/* free frame struct */
	free(frame);
}

/**
 * brief: get raw img
 * @buf[OUT]:buf to load img data
 * @size[OUT]:data size
 * return: void
 * notes:
 */
static void Get_raw_Img(unsigned char* buf, unsigned int* size)
{
    int cnt = 0;
	ak_print_notice_ex("entry\n");

    isp_vi_get_raw_data();

    while (ak_check_file_exist(RAW_SAVE_PATH) && cnt < 500)
    {
        ak_sleep_ms(10);
        cnt++;
    }

	if (0 == ak_check_file_exist(RAW_SAVE_PATH))
    {
        FILE *fd = NULL;

        fd = fopen(RAW_SAVE_PATH, "r+b");
   
      	if (NULL == fd)
      	{
      		ak_print_error("raw file open failed.\n");
          	return;
      	}
        
        fseek(fd, 0, SEEK_END);
        *size = ftell(fd);

        fseek(fd, 0, SEEK_SET);

        fread(buf, 1, *size, fd);

        fclose(fd);

        remove(RAW_SAVE_PATH);

        ak_print_notice_ex("get raw file ok.\n");
    }   
    else
    {
        ak_print_notice_ex("here is no raw file.\n");
    }
}


/**
 * brief: get img, only support yuv img
 * @module_id[IN]:img type
 * @buf[OUT]:buf to load img data
 * @size[OUT]:data size
 * return: 
 * notes:
 */
static int ImgData_Get(unsigned short module_id, unsigned char* buf,
		unsigned int* size)
{
	if (module_id > ISP_ENCODE_IMG || module_id < ISP_RAW_IMG
			|| NULL == buf || NULL == size) {
		printf("Isp_ImgData_Get param err\n");
		return 0;
	}

	switch (module_id) {
		case ISP_RAW_IMG:
			printf("get RAW IMG!\n");
            Get_raw_Img(buf, size);
			break;
		case ISP_YUV_IMG:
			printf("get YUV IMG!\n");
			Get_YUV_Img(buf, size);
			break;
		case ISP_ENCODE_IMG:
			printf("get Encoded IMG!\n");
			break;
		default:
			break;
	}

	return 0;
}



/**
 * brief: get cmd deal
 * @cmd[IN]:cmd info
 * @buf[OUT]:buf to load data
 * return: 
 * notes:
 */
static int get_cmd(PTRANS_DATA cmd, unsigned char* buf)
{
	unsigned int size = 0;

	if (NULL == cmd || NULL == buf) {
		printf("get_cmd param err\n");
		return 0;
	}

	if (cmd->head.attr_id <= ISP_HUE) {
		/*get basic isp module data*/
		isp_get_attr(cmd->head.attr_id, buf + ISP_PARM_LEN_SIZE, &size);
		memcpy(buf, &size, ISP_PARM_LEN_SIZE);
	} else if (cmd->head.attr_id >= ISP_3DSTAT
			&& cmd->head.attr_id <= ISP_AWBSTAT) {
		/*get isp stat info data*/
		isp_get_statinfo(cmd->head.attr_id, buf + ISP_PARM_LEN_SIZE, &size);
		memcpy(buf, &size, ISP_PARM_LEN_SIZE);
	} else if (cmd->head.attr_id >= ISP_RAW_IMG
			&& cmd->head.attr_id <= ISP_ENCODE_IMG) {
		ImgData_Get(cmd->head.attr_id, buf + ISP_PARM_LEN_SIZE, &size);
		memcpy(buf, &size, ISP_PARM_LEN_SIZE);
	} else if (cmd->head.attr_id >= ISP_PARM_CODE
			&& cmd->head.attr_id <= ISP_REGISTER) {
		unsigned long cnt = 0;
		unsigned long i = 0;
		unsigned short addr = 0;
		unsigned char  addrlen = 0;
		unsigned short value = 0;
		unsigned char  valuelen = 2;
		unsigned char* p = NULL;
		unsigned char* pdst = buf + ISP_PARM_LEN_SIZE;

		memcpy(&cnt, cmd->data, ISP_PARM_CNT_SIZE);
		printf("cnt = %lu\n", cnt);
		memcpy(pdst, &cnt, ISP_PARM_CNT_SIZE);
		pdst += ISP_PARM_CNT_SIZE;
		size += ISP_PARM_CNT_SIZE;

		p = cmd->data+ ISP_PARM_CNT_SIZE;

		for (i=0; i<cnt; i++) {
			memcpy(&addrlen, p, 1);
			p += 1;
			memcpy(&addr, p, addrlen);
			p += addrlen;

			printf("addr = 0x%x\n", addr);

			//get
			isp_get_sensor_value(addr, &value);
			memcpy(pdst, &addrlen, 1);
			pdst += 1;
			size += 1;

			memcpy(pdst, &addr, addrlen);
			pdst += addrlen;
			size += addrlen;

			memcpy(pdst, &valuelen, 1);
			pdst += 1;
			size += 1;

			memcpy(pdst, &value, valuelen);
			pdst += valuelen;
			size += valuelen;
		}

		memcpy(buf, &size, ISP_PARM_LEN_SIZE);
	} else if (ISP_CFG_DATA == cmd->head.attr_id) {
		size = ISP_CFG_MAX_SIZE - ISP_PACKET_HEAD_SIZE - ISP_PARM_LEN_SIZE;
		/*get isp cfg file data , load mode must be LOAD_MODE_WHOLE_FILE*/
		isp_cfg_file_load(LOAD_MODE_WHOLE_FILE, (char *)(buf + ISP_PARM_LEN_SIZE),
				&size);
		memcpy(buf, &size, ISP_PARM_LEN_SIZE);
	} else if (ISP_HEARTBEAT == cmd->head.attr_id) {
		/*reply heartbeat cmd*/
		char str[] = "isp heartbeat!";

		size = strlen(str);

		memcpy(buf+ISP_PARM_LEN_SIZE, str, size);
		memcpy(buf, &size, ISP_PARM_LEN_SIZE);

		set_check_timer(HEARTBEAT_CHECKTIME);
	}

	return 0;
}

/**
 * brief: set cmd deal
 * @cmd[IN]:cmd info
 * return: 
 * notes:
 */
static int set_cmd(PTRANS_DATA cmd)
{
	int ret = 0;

	if (NULL == cmd) {
		printf("set_cmd param err\n");
		return -1;
	}

	if (cmd->head.attr_id <= ISP_HUE) {
		/*set basic isp module data*/
		ret = isp_set_attr(cmd->head.attr_id, (char *)cmd->data, cmd->datalen);
	}
	else if (cmd->head.attr_id >= ISP_PARM_CODE && cmd->head.attr_id <= ISP_REGISTER) {
		unsigned long cnt = 0;
		unsigned long i = 0;
		unsigned short addr = 0;
		unsigned char  addrlen = 0;
		unsigned short value = 0;
		unsigned char  valuelen = 0;
		unsigned char* p = NULL;

		memcpy(&cnt, cmd->data, ISP_PARM_CNT_SIZE);
		printf("cnt = %lu\n", cnt);

		p = cmd->data + ISP_PARM_CNT_SIZE;

		for (i=0; i<cnt; i++) {
			memcpy(&addrlen, p, 1);
			p += 1;
			memcpy(&addr, p, addrlen);
			p += addrlen;

			memcpy(&valuelen, p, 1);
			p += 1;
			memcpy(&value, p, valuelen);
			p += valuelen;

			printf("addr = 0x%x, value = %d\n", addr, value);
			//set
			ret |= isp_set_sensor_value(addr, value);
		}
	} else if (ISP_CFG_DATA == cmd->head.attr_id) {
	/*store isp cfg file data, overwrite old file*/
		ret = isp_cfg_file_store((char *)cmd->data, cmd->datalen);
	}

	return ret;
}


/**
 * brief: add reply data to send list
 * @pctrl[IN]:pctrl handle
 * @cmd[IN]:cmd info
 * return: 
 * notes:
 */
static void send_cmd(PCTL_HANDLE pctrl, struct cmd_node *cmd)
{
	ak_thread_mutex_lock(&pctrl->lock);
	list_add_tail(&cmd->list, &pctrl->queue);
	ak_thread_mutex_unlock(&pctrl->lock);

	sem_post(&pctrl->isp_sem);
}


/**
 * brief: recv cmd execute
 * @pctrl[IN]:pctrl handle
 * @cmd[IN]:cmd info
 * return: 
 * notes:
 */
static int exec_cmd(PCTL_HANDLE pctrl, PTRANS_DATA cmd)
{
	int cmd_type = cmd->head.cmd_type;
	int ret = 0;
	//unsigned char* send_buf = NULL;
	unsigned long send_size = 0;
	struct video_channel_attr attr;
	void *vi_handle = NULL;
	int width = 0, height = 0;

	struct cmd_node *cmd_t = (struct cmd_node *)calloc(1,
			sizeof(struct cmd_node));
	if (!cmd_t) {
		ak_print_warning_ex("calloc failed\n");
		return -1;
	}
	
	/*analysis com info and execute*/
	switch (cmd_type) {
		case CMD_GET:
		case CMD_GET_TXT:
			//printf("isp get cmd, attr id : %d\n", cmd->head.attr_id);
			/* to get system arguments */
			vi_handle = ak_vi_get_handle(VIDEO_DEV0);
			ak_vi_get_channel_attr (vi_handle, &attr);
			width = attr.res[VIDEO_DEV0].max_width;
			height = attr.res[VIDEO_DEV0].max_height;
//			width = 2048;
//			height = 1536;

			/*img data, malloc img buf*/
			if ((cmd->head.attr_id >= ISP_RAW_IMG) && (cmd->head.attr_id <= ISP_ENCODE_IMG))
				cmd_t->send_buf = (unsigned char*)calloc(1, width * height *3/2 + ISP_PACKET_HEAD_SIZE + ISP_PARM_LEN_SIZE);
			else /*malloc isp cfg file size buf*/
				cmd_t->send_buf = (unsigned char*)calloc(1, ISP_CFG_MAX_SIZE);
			
			if (NULL == cmd_t->send_buf) {
				exit_err("send_buf");
			}

			memcpy(cmd_t->send_buf, &cmd->head.attr_id, ISP_ATTR_ID_SIZE);
			if (CMD_GET == cmd_type) {
				cmd_t->send_buf[2] = CMD_REPLY;
			} else {
				cmd_t->send_buf[2] = CMD_REPLY_TXT;
			}

			/*get cmd deal*/
			get_cmd(cmd, cmd_t->send_buf + ISP_PACKET_HEAD_SIZE);

			/*add reply data to send list*/
			send_cmd(pctrl, cmd_t);
			break;

		case CMD_SET:
			//printf("isp set cmd, attr id : %d\n", cmd->head.attr_id);

			send_size = ISP_PACKET_HEAD_SIZE + ISP_PARM_LEN_SIZE + ISP_RET_SIZE;
			cmd_t->send_buf = (unsigned char*)calloc(1, send_size);

			if (NULL == cmd_t->send_buf) {
				exit_err("send_buf");
			}

			//memset(send_buf, 0, send_size);
			memcpy(cmd_t->send_buf, &cmd->head.attr_id, ISP_ATTR_ID_SIZE);
			cmd_t->send_buf[2] = CMD_RET;
			cmd_t->send_buf[4] = ISP_RET_SIZE;

			/*set cmd deal*/
			ret = set_cmd(cmd);
			memcpy(cmd_t->send_buf + ISP_PACKET_HEAD_SIZE + ISP_PARM_LEN_SIZE,
					&ret, ISP_RET_SIZE);
			
			/*add reply data to send list*/
			send_cmd(pctrl, cmd_t);
			break;

		default:
			printf("cmd unrecognized\n");
			break;
	}

	return 0;
}

/**  
 * isp_send_thread - thread for send reply data
 * @arg[IN]:arg
 * notes: 
 */
static void *isp_send_thread(void *arg)
{
	int ret;
	unsigned long size = 0;
	struct cmd_node *cmd = NULL;
	PCTL_HANDLE pctrl = (PCTL_HANDLE)arg;

	ak_print_normal_ex("isp cmd thread id: %ld\n", ak_thread_get_tid());
	while (its_run_flag) {
		sem_wait(&pctrl->isp_sem);

		if (!list_empty(&pctrl->queue)) {
			cmd = list_first_entry(&pctrl->queue, struct cmd_node, list);

			if (cmd->send_buf) {
				memcpy(&size, cmd->send_buf + ISP_PACKET_HEAD_SIZE, ISP_PARM_LEN_SIZE);
				size += ISP_PACKET_HEAD_SIZE + ISP_PARM_LEN_SIZE;

				ret = send(pctrl->sock, cmd->send_buf, size, 0);
				if (ret <= 0) {
					exit_err("send");
				}

				ak_thread_mutex_lock(&pctrl->lock);
				free(cmd->send_buf);
				list_del(&cmd->list);
				free(cmd);
				ak_thread_mutex_unlock(&pctrl->lock);
			}
		}
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

/**  
 * isp_set_net_srv - tcp sever socket and bind
 * notes: 
 */
static int isp_set_net_srv(void)
{
	/** create socket **/
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0) {
		exit_err("sfdet");
	}

	/** initialize and fill in the message to the sfdet addr infor struct **/
	struct sockaddr_in server_addr;	/** server socket addr information struct **/
	memset(&server_addr, 0, sizeof(struct sockaddr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(ISP_PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	/** set socket attribute **/
	int sinsize = 1;
	int ret = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &sinsize, sizeof(int));
	if (ret < 0) {
		close(sfd);
		exit_err("setsockopt");
	}

	/** set close-on-exec flag **/
	ret = fcntl(sfd, F_SETFD, FD_CLOEXEC);
	if (ret < 0) {
		close(sfd);
		exit_err("fcntl");
	}

	/** bind sfdet with this program **/
	ret = bind(sfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if (ret < 0) {
		close(sfd);
		exit_err("bind");
	}

	return sfd;
}

/**  
 * isp_srv_start - tcp sever accept and recv data and execute
 * notes: 
 */
static int isp_srv_start(void)
{
	int ret; 	/** call function return val **/
	int sinsize; 	/** use for set socket option **/
	pthread_t respons_tid;	/** response data thread id **/
	PCTL_HANDLE pctrl = NULL;	/** define a isp handle **/
	TRANS_DATA cmd;
	int recvlen = 0;
	int size = 0;
	T_HEAD_TYPE headret = HEAD_ERR;

	/* set heart beat timeout */
	signal(SIGALRM, heartbeat_timeout);

	/* start net server to receive connection */
	sfd = isp_set_net_srv();
	if (sfd <= 0) {
		ak_print_warning_ex("set net srv failed\n");
		return -1;
	}

	/** listen, create wait_queue **/
	ret = listen(sfd, 1);	/* only one client can connect to this server */
	if (ret < 0) {
		close(sfd);
		exit_err("listen");
	}

	struct sockaddr_in peer_addr;		/** server socket addr information struct **/
	memset(&peer_addr, 0, sizeof(struct sockaddr));
	//accept
	while (its_run_flag) {
		ak_print_notice_ex("waiting for connect ...\n");

		sinsize = sizeof(struct sockaddr);
		/** accept the client to connect **/
		cfd = accept(sfd, (struct sockaddr *)&peer_addr, (socklen_t *)&sinsize);
		if (cfd < 0) {
			close(sfd);
			exit_err("accept");
		}
		printf("Client connected, socket cfd = %d\n", cfd);

		/** init the handle and others **/
		pctrl = calloc(1, sizeof(CTL_HANDLE));
		pctrl->sock = cfd;
		sem_init(&pctrl->isp_sem, 0, 0);
		INIT_LIST_HEAD(&pctrl->queue);
		ak_thread_mutex_init(&pctrl->lock, NULL);

		/** command parse thread **/
		if (pthread_create(&respons_tid, NULL, isp_send_thread, pctrl))
			pthread_detach(respons_tid);

		/* heart bead sending */
		set_check_timer(HEARTBEAT_CHECKTIME);

		/** receive data **/
		while (its_run_flag) {
			size = ISP_PACKET_HEAD_SIZE;
			memset(&cmd, 0, sizeof(TRANS_DATA));

			//printf("waiting for recv head...\n");
			ret = recv(cfd, &cmd.head, size, 0);
			if(ret <= 0)
			{
				break;
			}

			/* head check */
			headret = check_cmd_head(&cmd);

			/* drop invalid packet */
			if (HEAD_ERR == headret) {
				continue;
			} else if (HEAD_HAVE_DATA == headret) {
				size = ISP_PARM_LEN_SIZE;
				ret = recv(cfd, &cmd.datalen, size, 0);

				if(ret <= 0) {
					break;
				}
				printf("cmd.datalen = %lu\n", cmd.datalen);

				recvlen = 0;
				size = cmd.datalen;
				cmd.data = (unsigned char *)calloc(1, size);
				if (NULL == cmd.data) {
					break;
				}

				memset(cmd.data, 0, size);
				while (recvlen < cmd.datalen) {
					ret = recv(cfd, cmd.data + recvlen, size, 0);
					if(ret <= 0) {
						goto RECV_ERROR;
					}
					recvlen += ret;
					size -= ret;
					printf("recvlen = %d\n", recvlen);
				}
			}

			exec_cmd(pctrl, &cmd);
			if (NULL != cmd.data) {
				free(cmd.data);
				cmd.data = NULL;
			}
		}

RECV_ERROR:
		printf("recv error, ret = %d\n", ret);

		ak_thread_mutex_lock(&pctrl->lock);
		struct cmd_node *cmd_t = NULL, *tmp = NULL;
		list_for_each_entry_safe(cmd_t, tmp, &pctrl->queue, list) {
			if (cmd_t->send_buf)
				free(cmd_t->send_buf);
			list_del(&cmd_t->list);
			free(cmd_t);
		}
		ak_thread_mutex_unlock(&pctrl->lock);
		ak_thread_mutex_destroy(&pctrl->lock);

		sem_destroy(&pctrl->isp_sem);
		free(pctrl);

		if (-1 != cfd) {
			close(cfd);
			cfd = -1;
		}

		pthread_cancel(respons_tid);
		if (NULL != cmd.data) {
			free(cmd.data);
			cmd.data = NULL;
		}

		set_check_timer(0);
	}

	if (-1 != sfd)
	{
		close(sfd);
		sfd = -1;
	}
	ak_print_normal_ex("isp server exit\n");

	return 0;
}

/**  
 * isp_tool_server - its thread function
 * @arg[IN]:arg
 * notes: 
 */
static void *isp_tool_server(void *arg)
{
	long int tid = ak_thread_get_tid();
	ak_print_normal_ex("thread tid: %ld\n", tid);
	ak_thread_set_name("isp_tool_server");

	/* start isp tool server */
	isp_srv_start();

	ak_print_normal_ex("exit thread tid: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

/**
 * ak_its_get_version - get ISP tool server version
 * return: version string
 * notes: 
 */
const char* ak_its_get_version(void)
{
	return its_version;
}

/**  
 * ak_its_start - start anyka ISP tool server
 * notes: If you wan't use ISP tool to debug, do not start this server.
 */
void ak_its_start(void)
{
	if (!its_run_flag) {
		ak_pthread_t isp_svr_tid = 0;
		/* create server thread */
		ak_thread_create(&isp_svr_tid, isp_tool_server, NULL, 
			ANYKA_THREAD_MIN_STACK_SIZE, -1);
		its_run_flag = AK_TRUE;
	}
}

/**  
 * ak_its_stop - stop anyka ISP tool server
 * notes:
 */
void ak_its_stop(void)
{
	if (its_run_flag) {
		its_run_flag = AK_FALSE;
		shutdown(sfd, 2);
		close(sfd);
		sfd = -1;
	}
}

#else

/* empty implement */
void ak_its_start(void)
{
}

#endif
