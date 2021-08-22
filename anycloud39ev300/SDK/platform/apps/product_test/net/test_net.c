#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <syslog.h>
#include <stdarg.h>

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <linux/netlink.h>
#include <linux/version.h>
#include <linux/input.h>

#include "ak_thread.h"
#include "ak_common.h"
#include "test_config.h"
#include "test_common.h"


#define FIFO_PATH			"/tmp/test_fifo"
#define	RESTART_AK_IPC		"/usr/sbin/anyka_ipc.sh"
#define UPDATE_SH			"/usr/sbin/update.sh"
#define SERVICE				"/usr/sbin/service.sh"

#define FIFO_SZ (50)
#define INTERVAL (3) //3seconds

#define KEY_GPIO_DEV		"/dev/input/event0"
#define SDP1_DEV_NAME       "/dev/mmcblk0p1"
#define SD_DEV_NAME         "/dev/mmcblk0"

#define SD_TOOL				"dsk_repair"

#define MOUNT_SDP1			"mount -rw /dev/mmcblk0p1 /mnt"
#define MOUNT_SD			"mount -rw /dev/mmcblk0 /mnt"
#define UMOUNT_SD			"umount /mnt -l"

#define UPDATE_IMAGE			10 //10 seconds
#define UEVENT_BUFFER_SIZE      2048
/****** key 1 : reset *****/
#define RECOVER_DEV				5
#define UPDATE_IMAGE			10

#define TCP_PORT 		6789
#define TCP_HB_PORT		6790
#define UPD_PORT 		8192
#define SUCCESS			"1"
#define FAILURE			"0"
#define TCP_BUF_SIZE 	1024
#define DEBUG 	//for test print
#define QQ_VOICE_MODE_SWITCH

/*************  TCP/IP TEST  DATA STRUCT **************/
enum
{
	COMMAND = 1,
	RESPONSE,
	HEARTBEAT,
	FINISH,
};

enum test_info
{
	CASE_SPEAKER = 0,   //喇叭
	CASE_CLOUD_DECK,   //云台
	CASE_WIFI_CONNECT,   //wifi连接
	CASE_SD_CARD,    //SD卡
	CASE_MONITOR,   //监听
	//CASE_INTERPHONE,  //对讲这一项不需要
	CASE_FOCUS_EFFECT,   //调焦和效果
	CASE_IRCUT,     //IRCUT和红外灯
	CASE_RESET,    //复位
	CASE_MAC_BURN,  //mac烧录
	CASE_UID_BURN,  //uid烧录
	CASE_ATTR_TYPE_NUM
};

enum cmd_type
{
	 CMD_COMMAND = 1,	
	 CMD_RESPONSE,
     CMD_TYPE_NUM
};

struct test_data
{
	char *head_name ;  //包头标识“VCMD”
	enum test_info  test_type;
	enum cmd_type  cmd_type;
	uint   datalen;//数据长度
	char*   data;    //数据
	uint  check_sum;
	char   *end_name;  //包尾标识“VEND”
};


/***** tcp communite data struct  *****/
struct data_struct
{
	unsigned short len;
	unsigned short arg_len;
	unsigned short file_len;
	unsigned short check_sum;
	unsigned char type;
	unsigned char auto_test;
	char *file_name;
	char *arg_data;
};

struct socket_data
{
	int sock_fd;
	int pth_runflags;
};

static int test_running = 0;
char last_file[20] = {0};

static ak_pthread_t tcp_sock_id;
static ak_pthread_t send_hb_pid;	//send heart beat pid
static ak_pthread_t broadcast_sk_id;	
static ak_pthread_t ircut_pth_id;
static ak_pthread_t mmc_pth_id;
static ak_pthread_t recover_pth_id;
static ak_pthread_t wifi_pth_id;
static ak_pthread_t send_recover_ret_pth_id;
static ak_pthread_t send_mmc_ret_pth_id;
static ak_pthread_t send_wifi_ret_pth_id;


static void *vi_handle = NULL;
static void *ai_handle = NULL;
static void *ao_handle = NULL;
static int test_param;
static int mmc_test_finish = 0;
static int conn_state = 0;

void test_ircut_background(void);
void test_mmc_background(void);
void test_wifi_background(void);
void test_recover_background(void);

static int do_syscmd(char *cmd, char *result)
{
	char buf[512];
	FILE *filp;

	filp = popen(cmd, "r");
	if (NULL == filp) {
		ak_print_normal_ex("popen fail!\n");
		return -2;
	}

	/* fgets(buf, sizeof(buf)-1, filp); */
	memset(buf, '\0', sizeof(buf));
	fread(buf, sizeof(char), sizeof(buf)-1, filp);

	sprintf(result, "%s", buf);

	pclose(filp);
	return strlen(result);
}

/**
 * cp the struct data pointer by data to the buf
 *param[in]   send buf, a pointer to data
 *return      void
 */
static void test_cp_to_buf(char * send_buf, struct data_struct *data)
{
	//len
	memcpy(send_buf, &data->len, sizeof(data->len));
	send_buf+=sizeof(data->len);
	//type
	memcpy(send_buf, &data->type, sizeof(data->type));
	send_buf+=sizeof(data->type);
	//auto_test
	memcpy(send_buf, &data->auto_test, sizeof(data->auto_test));
	send_buf+=sizeof(data->auto_test);
	//file len
	memcpy(send_buf, &data->file_len, sizeof(data->file_len));
	send_buf+=sizeof(data->file_len);

	//file name
	if(data->file_len > 0) {
		memcpy(send_buf, data->file_name, data->file_len);
		send_buf+=data->file_len;
	}
	memcpy(send_buf, &data->arg_len, sizeof(data->arg_len));	//arg len
	send_buf+=sizeof(data->arg_len);

	if(data->arg_len > 0) {
		memcpy(send_buf, data->arg_data, data->arg_len);	//arg-data
		send_buf+=data->arg_len;
	}
	memcpy(send_buf, &data->check_sum, sizeof(data->check_sum));	//check sum data
	send_buf+=sizeof(data->check_sum);

	return ;
}

/**
 * cp the test_data data pointer by data to the buf
 *param[in]   send buf, a pointer to data
 *return      void
 */
static void test_data_to_buf(char * send_buf, struct test_data *data)
{
	//headname
	memcpy(send_buf, data->head_name, 4);
	send_buf += 4;
	
	//test_type
	memcpy(send_buf, &data->test_type, 1);
	send_buf += 1;
	
	//cmd_type
	memcpy(send_buf, &data->cmd_type, 1);
	send_buf += 1;
	
	//data len
	memcpy(send_buf, &data->datalen, sizeof(data->datalen));
	send_buf += sizeof(data->datalen);

	//data
	memcpy(send_buf, data->data, data->datalen);
	send_buf += data->datalen;

	//check_sum
	memcpy(send_buf, &data->check_sum, sizeof(data->check_sum));	//check sum data
	send_buf += sizeof(data->check_sum);

	//endname
	memcpy(send_buf, data->end_name, 4);
	send_buf += 4;

	return ;
}

/**
 *calculate the data's sum which pointer by data
 *@param[in]   a pointer to data
 *@return      result after calculate
 */
static int test_calc_checksum(struct data_struct *data)
{
	int i;

	data->check_sum = (data->type) + (data->auto_test) + (data->file_len) +
		(data->arg_len);
	for (i = 0; i < (data->file_len); i++) {
		data->check_sum += (data->file_name[i]);
	}
	for(i = 0; i < (data->arg_len); i++) {
		data->check_sum += (unsigned short)(data->arg_data[i]);
	}

	return data->check_sum;
}

/**
 * cp the struct data pointer by data to the buf
 * @send_to_fd[IN],a descripter which you want to send
 * return: when send success info,result = 1,else 0
 */
static int test_response(int send_to_fd, int test_type, int result)
{
	char send_buf[200] = {0};
	int send_fd = send_to_fd;
	int ret;
	char retval = 0;
	int i = 0;
	int send_len = 0;

	struct test_data *response_data = (struct test_data *)calloc(1,
			sizeof(struct test_data));
	if(response_data == NULL) {
		ak_print_normal_ex("fail to calloc\n");
		return -1;
	}

	response_data->head_name = "VCMD";
	response_data->test_type = test_type;		//test type
	response_data->cmd_type = CMD_RESPONSE;		//cmd type
	response_data->end_name = "VEND";
	
	if (result == 0)
		retval = 1;
	else //failure
		retval = 2;


	if (CASE_SD_CARD == test_type) {
		uint64_t mmc_size = 0;
		int mmc_size_len = 8;
		
		for (i=0; i<10; i++) {
			if (1 == mmc_test_finish)
				break;
			else
				ak_sleep_ms(500);
		}
			
		mmc_size = test_mmc_get_total_size();

		if (0 == mmc_size)
			retval = 2;	//failure
			
		response_data->datalen = 9;	//1+8

		response_data->data = (char *)calloc(1, response_data->datalen);
		memcpy(response_data->data, &retval, 1);
		memcpy(&response_data->data[1], &mmc_size, mmc_size_len);
	} else if (CASE_WIFI_CONNECT == test_type) {
		struct wifi_list_info wifi_info = {0};

		for (i=0; i<20; i++) {
			if (1 == is_test_wifi_finish())
				break;
			else
				ak_sleep_ms(1000);
		}
		
		get_wifi_info_list(&wifi_info);

		if (0 == wifi_info.cnt)
			retval = 2;	//failure
		
		response_data->datalen = 1 + 4 + wifi_info.cnt * sizeof(struct ssid_info);
		response_data->data = (char *)calloc(1, response_data->datalen);
		memcpy(response_data->data, &retval, 1);
		memcpy(&response_data->data[1], &wifi_info.cnt, 4);
		if (wifi_info.cnt > 0) {
			memcpy(&response_data->data[5], wifi_info.info, 
					wifi_info.cnt * sizeof(struct ssid_info));
		}
		
	} else {
		response_data->datalen = 1;
		response_data->data = (char *)calloc(1, response_data->datalen);
		memcpy(response_data->data, &retval, 1);
	}

	response_data->check_sum = response_data->test_type;
	response_data->check_sum += response_data->cmd_type;
	response_data->check_sum += response_data->datalen & 0xff;
	response_data->check_sum += (response_data->datalen & 0xff00) >> 8;
	response_data->check_sum += (response_data->datalen & 0xff0000) >> 16;
	response_data->check_sum += (response_data->datalen & 0xff000000) >> 24;

	for (i=0; i<response_data->datalen; i++) {
		response_data->check_sum += response_data->data[i];
	}

	test_data_to_buf(send_buf, response_data);

	send_len = 4 + 1 + 1 + 4 + response_data->datalen + 4 + 4;


	/*** send message ***/
	ret = send(send_fd, send_buf, send_len, 0);
	if(ret < 0) {
		ak_print_normal_ex("send response data faild, error:%s\n", strerror(errno));
	}

#ifdef DEBUG
	ak_print_normal("respons test_type : %d\n", response_data->test_type);
	ak_print_normal("respons cmd_type : %d\n", response_data->cmd_type);
	ak_print_normal("respons datalen : %d\n", response_data->datalen);
	ak_print_normal("respons check_sum : %d\n", response_data->check_sum);
#endif
	//release resource
	if(response_data->data)
		free(response_data->data);
	free(response_data);

	return ret;
}

#if 0
/**
 *  @brief       	 get the test program's execute result
 *  @param[in]   void
 *  @return        result = 0,success; -1,failure
 */
static int test_get_exec_result()
{
	char res[4] = {0};
	if(access(TEST_STAT, F_OK) < 0) {
		ak_print_normal_ex("the program has not run.\n");
		return -1;
	}

	int fd = open(TEST_STAT, O_RDONLY);
	if(fd < 0) {
		ak_print_normal_ex("open result file failed.\n");
		return -1;
	}

	if(read(fd, res, sizeof(res)) < 0) {
		ak_print_normal_ex("read result file failed.\n");
		return -1;
	}

	ak_print_normal_ex("after execute test program, return value is : %d\n",
		atoi(res));
	return atoi(res);
}
#endif

/**
 * *  @brief       	 fill the info to the struct
 * *  @author      	 chen yanhong
 * *  @date       	 2014-10-28
 * *  @param[in]   info buf which will be filled to the struct
 * *  @return        a pointer to data
 * */
struct data_struct *test_create_struct_info(char *buf)
{
	struct data_struct *data = (struct data_struct *)calloc(1,
		sizeof(struct data_struct));
	if (data == NULL) {
		ak_print_normal_ex("fail to calloc\n");
		return NULL;
	}
	memset(data, 0, sizeof(struct data_struct));
	//len
	memcpy(&data->len, buf, sizeof(data->len));
	buf+=sizeof(data->len);
	//type
	memcpy(&data->type, buf, sizeof(data->type));
	buf+=sizeof(data->type);
	//auto ?
	memcpy(&data->auto_test, buf, sizeof(data->auto_test));
	buf+=sizeof(data->auto_test);
	//file len
	memcpy(&data->file_len, buf, sizeof(data->file_len));
	buf+=sizeof(data->file_len);
	//file name
	if(data->file_len > 0)
	{
		data->file_name = (char *)calloc(1, (data->file_len + 1));
		memcpy(data->file_name, buf, data->file_len);
		data->file_name[data->file_len] = 0;
		buf+=data->file_len;
	}
	//argument len
	memcpy(&data->arg_len, buf, sizeof(data->arg_len));
	buf+=sizeof(data->arg_len);
	//argument data
	if(data->arg_len > 0)
	{
		data->arg_data = (char *)calloc(1, (data->arg_len + 1));
		memcpy(data->arg_data, buf, data->arg_len);
		data->arg_data[data->arg_len] = 0;
		buf+=data->arg_len;
	}
	//check sum
	memcpy(&data->check_sum, buf, sizeof(data->check_sum));
	buf+=sizeof(data->arg_len);

	return data;
}

/**
 * *  @brief       	 free the struct memery
 * *  @author      	 chen yanhong
 * *  @date       	 2014-10-28
 * *  @param[in]   a pointer to data while you want to free it's memery
 * *  @return        void
 * */
static void test_destroy_struct_info(struct data_struct *data)
{
	if(data->file_name)
		free(data->file_name);
	if(data->arg_data)
		free(data->arg_data);
	if(data)
		free(data);
	return ;
}

int parse_uid_info(char *buf, char *uid, int *uid_len, char *data, int *data_len)
{
	int uidlen = 0;
	int datalen = 0;
	
	if (NULL == buf || NULL == uid || NULL == data 
		|| NULL == uid_len || NULL == data_len)
		return AK_FAILED;

	for (int i=0; i<40; i++) {
		ak_print_normal("%d, ", buf[i]);
		if (3 == i || 35 == i)
			ak_print_normal("\n");
	}

	ak_print_normal("\n");

	memcpy(&uidlen, buf, 4);
	ak_print_normal_ex("uidlen :%d\n", uidlen);

	if (uidlen <= 0 || uidlen > *uid_len)
		return AK_FAILED;

	*uid_len = uidlen;

	memcpy(uid, buf + 4, uidlen);

	ak_print_normal_ex("uid :%s\n", uid);
	
	memcpy(&datalen, buf + 4 + uidlen, 4);
	
	ak_print_normal_ex("datalen :%d\n", datalen);

	if (datalen <= 0 || datalen > *data_len)
		return AK_FAILED;

	*data_len = datalen;

	memcpy(data, buf + 4 + uidlen + 4, datalen);
	return AK_SUCCESS;
}

/**
 *  parse receive data, then execute it
 *@ info buf which was receive by system call recv, client socket file descriptor
 *@return        execute result, 0 succed ;else failure
 * */
int test_parse_exec(char *buf, int cfd, int *test_type)
{
	char cmd_buf[256] = {0};
	short check_sum_test;
	int ret = 0;
	struct data_struct *parse_data;

	parse_data = test_create_struct_info(buf);
	if(parse_data == NULL)
		return -1;

	/** debug print **/
#ifdef DEBUG
	ak_print_normal_ex("recv len :%d\n", parse_data->len);
	ak_print_normal_ex("recv type :%d\n", parse_data->type);
	ak_print_normal_ex("recv auto_ts :%d\n", parse_data->auto_test);
	ak_print_normal_ex("recv file_len :%d\n", parse_data->file_len);
	ak_print_normal_ex("recv arg_len :%d\n", parse_data->arg_len);
	ak_print_normal_ex("recv check :%d\n", parse_data->check_sum);
	ak_print_normal_ex("recv file name :%s\n", parse_data->file_name);
	ak_print_normal_ex("recv arg name : %s\n", parse_data->arg_data);
#endif
	//check len
	if(parse_data->len == 0 ) {
		ak_print_normal_ex(" error,data len:0.\n");
		ret = -1;
		goto bye_bye;
	}
	//check sum
	check_sum_test = test_calc_checksum(parse_data);
	
	if(check_sum_test != parse_data->check_sum)
	{
		ak_print_normal_ex("receive data error, the check sum is not coincident.\n");
		ret = -1;
		goto bye_bye;
	}

	
	//check finish
	if (parse_data->type == FINISH) {
		ak_print_normal_ex("going to restart service\n");
		ret = 2;
		goto bye_bye;
	}
	if( parse_data->file_name == NULL) {
		ak_print_normal_ex(" error, file_name:NULL\n");
		ret = -1;
		goto bye_bye;
	}		
	bzero(cmd_buf, sizeof(cmd_buf));
	
	if(strcmp(parse_data->file_name,"test_recover_dev") == 0) {
		*test_type = CASE_RESET;
		ret = test_recover_dev(atoi(parse_data->arg_data));
	}
	else if(strcmp(parse_data->file_name,"test_mac") == 0) {
		*test_type = CASE_MAC_BURN;
		ret = test_mac(parse_data->arg_data);
	}
	else if(strcmp(parse_data->file_name,"test_pushid") == 0){
		char uid[64] = {0};
		char data[1024] = {0};
		int uid_len = 64;
		int data_len = 1024;
		
		struct config_handle *config = test_config_get_value();
		*test_type = CASE_UID_BURN;

		if (AK_SUCCESS == parse_uid_info(parse_data->arg_data, uid, &uid_len, data, &data_len)) {
			if(0 == config->uid_name[0]){
	    		/* TEST_CLOUD_PLUS	*/	
				ret = test_pushid(uid, data, data_len, "danale.conf");
			}else{			
				ret = test_pushid(uid, data, data_len, config->uid_name);
			}
		} else
			ret = -1;
	}
	else if(strcmp(parse_data->file_name,"test_ircut_on_off") == 0) {
		*test_type = CASE_IRCUT;
		test_param = atoi(parse_data->arg_data);
		test_ircut_background();
	}
	else if(strcmp(parse_data->file_name,"test_mmc") == 0) {
		*test_type = CASE_SD_CARD;
		test_mmc_background();
	}
	else if(strcmp(parse_data->file_name,"test_wifi") == 0) {
		*test_type = CASE_WIFI_CONNECT;
		test_wifi_clean_flag();
		test_wifi_background();
	}
bye_bye:
	test_destroy_struct_info(parse_data);		//free the message resource
	return ret;
}

/**
 * *  @brief       	 send heart beat to client
  @param[in] struct which include client socket file descriptor and a pthread run flags
 * *  @return        void *
 * */
static void *send_hb_func(void *arg)
{
	char send_buf[200] = {0};
	struct data_struct hb;
	int sock_fd = -1, pc_sock, sinsize, send_size = 0;
	//struct socket_data *pc_data = (struct socket_data *)arg;
	//int *run_flag = (int *)&pc_data->pth_runflags;
	struct sockaddr_in my_addr, peer_addr;
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("thread id: %ld\n", tid);

	/* fill the info to the struct */
	hb.type = HEARTBEAT;
	hb.auto_test = 1;
	hb.file_len = 0;
	hb.arg_len = 0;
	hb.file_name = NULL;
	hb.arg_data = NULL;
	/* calculate the check sum */
	hb.check_sum = test_calc_checksum((struct data_struct *)&hb);
	hb.len = (sizeof(hb.len)) + (sizeof(hb.type)) + (sizeof(hb.auto_test))
	   	+ (sizeof(hb.file_len)) + (sizeof(hb.arg_len)) +
		(hb.file_len)+(hb.arg_len) + (sizeof(hb.check_sum));

	/* copy to send buf */
	test_cp_to_buf(send_buf, &hb);

	/* init tcp struct */
	memset(&peer_addr, 0, sizeof(struct sockaddr));
	memset(&my_addr, 0, sizeof(struct sockaddr));

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(TCP_HB_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	/* create send heart beat socket */
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1) {
		ak_print_error_ex("socket: %s\n", strerror(errno));
		return NULL;
	}

	/* set socket options */
	sinsize = 1;
	if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sinsize, sizeof(int)) != 0) {
		ak_print_error_ex("setsockopt: %s\n", strerror(errno));
		goto close_sk;
	}
	/* set close-on-exec flag */
	if(fcntl(sock_fd, F_SETFD, FD_CLOEXEC) == -1) {
		ak_print_error_ex("fcntl: %s\n", strerror(errno));
		goto close_sk;
	}

	/*** bind ***/
	if(bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		ak_print_error_ex("bind: %s.\n", strerror(errno));
		goto close_sk;
	}

	/**** listen ****/
	if(listen(sock_fd, 1) == -1) {
		ak_print_error_ex("listen: %s\n", strerror(errno));
		goto close_sk;
	}

	/**** accept ****/
	sinsize = sizeof(struct sockaddr);
	while(1){
		ak_print_normal_ex("start to accept heart sock\n");
		pc_sock = accept(sock_fd, (struct sockaddr *)&peer_addr, (socklen_t *)&sinsize);
		if(pc_sock == -1) {
			ak_print_error_ex("accept: %s\n", strerror(errno));
			goto close_sk;
		}
		ak_print_normal_ex("start send heart beat.\n");

		while (1) {
			/* send the heart beat package pointer by send_buf to PC */
			send_size = send(pc_sock, send_buf, hb.len, 0);
			if(send_size < 0) {
				ak_print_error_ex("send: %s.\n", strerror(errno));				
				close(pc_sock);
				break;
			}
			sleep(2);	// two sec interval
		}
	}

close_sk:
	close(sock_fd);

	ak_print_normal_ex("Exit heart beat thread, id: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

static void *send_recover_ret_pth(void *param)
{
	int *sock_fd = (int *)param;
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("thread id: %ld\n", tid);
	
	while (1)
	{
		if (test_get_press_key()) {
			test_response(*sock_fd, CASE_RESET, 0);
			break;
		} else 
			ak_sleep_ms(500);
	}
	
	return NULL;
}

static void *send_mmc_ret_pth(void *param)
{
	int *sock_fd = (int *)param;
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("thread id: %ld\n", tid);
	
	while (1)
	{
		if (mmc_test_finish) {
			test_response(*sock_fd, CASE_SD_CARD, 0);
			break;
		} else 
			ak_sleep_ms(500);
	}
	
	return NULL;
}

static void *send_wifi_ret_pth(void *param)
{
	int *sock_fd = (int *)param;
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("thread id: %ld\n", tid);
	
	while (1)
	{
		if (is_test_wifi_finish()) {
			test_response(*sock_fd, CASE_WIFI_CONNECT, 0);
			break;
		} else 
			ak_sleep_ms(500);
	}
	
	return NULL;
}




/**
 * tcp_pth -
 * @monitor_pid[IN]   monitor pthread id
 * return void *
 */
static void *test_tcp_pth(void *monitor_pid)
{
	int sock_fd;	//sock fd
	int sinsize;
	int cmd_ret = 0;
	int recv_size;
	int test_type = CASE_ATTR_TYPE_NUM;
	//int close_flags = 1;	//kill anyka_ipc and watch dog flag
	//ak_pthread_t m_pid = *((ak_pthread_t *)monitor_pid);	//get monitor pid
	
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("tcp thread id: %ld\n", tid);

	//heart beat, pc socket fd;
	struct socket_data *pc_communicate = (struct socket_data *)calloc(1,
			sizeof(struct socket_data));
	char *receive_buf = (char *)calloc(1, TCP_BUF_SIZE);
	struct sockaddr_in my_addr, peer_addr;

	//clear the struct
	memset(&my_addr, 0, sizeof(struct sockaddr));
	memset(&peer_addr, 0, sizeof(struct sockaddr));

	//init tcp struct
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(TCP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	/*** create socket ***/
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1) {
		ak_print_error_ex("fail to create TCP socket.\n");
		goto exit2;
	}
	ak_print_normal_ex("Success to create TCP socket.\n");

	sinsize = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &sinsize,
				sizeof(int)) != 0) {
		ak_print_error_ex("setsockopt, %s\n", strerror(errno));
		sleep(3);
		goto close_sk;
	}
	if (fcntl(sock_fd, F_SETFD, FD_CLOEXEC) == -1) {
		ak_print_error_ex("fcntl: %s\n", strerror(errno));
		sleep(3);
		goto close_sk;
	}

	/*** bind ***/
	if (bind(sock_fd, (struct sockaddr *)&my_addr,
			   	sizeof(struct sockaddr)) == -1) {
		ak_print_error_ex("bind: %s\n", strerror(errno));
		sleep(3);
		goto close_sk;
	}

	/**** listen ****/
	if(listen(sock_fd, 1) == -1) {
		ak_print_error_ex("listen: %s\n", strerror(errno));
		goto close_sk;
	}
	//ak_print_normal_ex("Set Listen success.\n");
	ak_print_normal_ex("Waiting for connect......\n");

	/**** accept ****/
	sinsize = sizeof(struct sockaddr);
	while (1) {	
		ak_print_normal_ex("start to accept sock\n");
		pc_communicate->sock_fd = accept(sock_fd,
				(struct sockaddr *)&peer_addr, (socklen_t *)&sinsize);
		if (pc_communicate->sock_fd == -1) {
			ak_print_error_ex("accept: %s\n", strerror(errno));
			continue;
		} else {
			ak_print_normal_ex("This machine has been connnected\n");
			/* the test is running, make the keys lose efficacy */
			test_running = 1;

			/* create pthread to send heart beat */
			pc_communicate->pth_runflags = 1;
			conn_state = 1;

			test_reset_press_key();
			test_recover_background();
			
			ak_thread_create(&send_recover_ret_pth_id , send_recover_ret_pth, 
							(void *)&pc_communicate->sock_fd, ANYKA_THREAD_MIN_STACK_SIZE, -1);
			ak_thread_create(&send_mmc_ret_pth_id , send_mmc_ret_pth, 
							(void *)&pc_communicate->sock_fd, ANYKA_THREAD_MIN_STACK_SIZE, -1);
			ak_thread_create(&send_wifi_ret_pth_id , send_wifi_ret_pth, 
							(void *)&pc_communicate->sock_fd, ANYKA_THREAD_MIN_STACK_SIZE, -1);
			
			while (1) {
				ak_print_normal_ex("ready to receive data\n");
				/* receive data */
				recv_size = recv(pc_communicate->sock_fd, receive_buf, TCP_BUF_SIZE, 0);
				if (recv_size == -1) {
					ak_print_error_ex("receive data error\n");
					break;
				}
				if(recv_size == 0) {
					ak_print_normal_ex("client out of line\n");
					break;
				}
				ak_print_normal_ex("do received data\n");
				/* analyse command, than execute it. */
				cmd_ret = test_parse_exec(receive_buf, pc_communicate->sock_fd, &test_type);
				if(cmd_ret == -1) { //after execute the cmd, return  error
					ak_print_error_ex("execute command failed.\n");
					if(test_response(pc_communicate->sock_fd, test_type, (-1)) < 0) {
						ak_print_error_ex("send response failed.\n");
						break;
					}
				} else if(cmd_ret == 0) { //pc socket close
					if(test_response(pc_communicate->sock_fd, test_type, (0)) < 0) {
						ak_print_error_ex("send response failed.\n");
						break;
					}
				} else if(cmd_ret == 2) { //it has receive a finish signal
					test_response(pc_communicate->sock_fd, test_type, (0));
					close(sock_fd);
					ak_print_normal_ex("####!!!! Restart device !!!!####\n");
					system("reboot");
					exit(0);	//no use actually, cause the system has been reboot
				} else { //manual test
					ak_print_normal_ex("Manual test mode, it has send report to PC\n");
				}
				memset(receive_buf, 0, TCP_BUF_SIZE);
			}
			pc_communicate->pth_runflags = 0;	//close heart beat pthread
			conn_state = 0;
			
		}

close_sk:		
        if(cmd_ret == 2)
        {
			test_response(pc_communicate->sock_fd, test_type, (0));
            ak_print_normal("Receive Finish Signal, reboot the device.\n");
			free(pc_communicate);
			free(receive_buf);
            system("reboot");
            exit(1);
        }
		close(pc_communicate->sock_fd);
	}
	close(sock_fd);
exit2:
	free(pc_communicate);
	free(receive_buf);

	ak_print_normal("Exit the socket thread, id : %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

/**
 * *  @brief		get_machine_ip
 				获取机器当前使用的网卡的IP，子网掩码和网关
 * *  @author
 * *  @date       	 2014-10-28
 * *  @param[in]   char *send_buf， 存放ip等信息的bug 指针
 * *  @return        void
 * */
static void get_machine_ip(char *send_buf)
{
	char *p = NULL;
	char *q = NULL;
	char cmdbuf[64] = {0};
	char result[128] = {0};
	char gateway[32] = {0};

	/********* check current net status **********/
	sprintf(cmdbuf, "ifconfig eth0 | grep RUNNING");
	do_syscmd(cmdbuf, result);
	bzero(cmdbuf, sizeof(cmdbuf));

	if (strlen(result) > 0)
		sprintf(cmdbuf, "ifconfig eth0 | grep inet");
	else
		sprintf(cmdbuf, "ifconfig wlan0 | grep inet");
	bzero(result, sizeof(result));
	do_syscmd(cmdbuf, result);

	/********* get ip  ********/
	p = strchr(result, ':');
	if (p) {
		p += 1;
		q = strchr(p, ' ');
		if (q) {
			strncpy(send_buf, p, q - p);
			strncpy(gateway, p, q - p);
			strcat(send_buf, "@");
			//ak_print_normal("get ip: [%s]\n", send_buf);
			send_buf += strlen(send_buf);
		}

		/******* get net mask *******/
		p = strrchr(p, ':');
		if (p) {
			p += 1;
			q = strchr(p, '\n');
			if (q) {
				strncpy(send_buf, p, q - p);
				strcat(send_buf, "@");
				//ak_print_normal_ex("get net mask :[%s]\n", send_buf);
				send_buf += strlen(send_buf);
			}
		}

		/******* get net gateway *******/
		p = strrchr(gateway, '.');
		if (p) {
			bzero(result, sizeof(result));
			strncpy(result, gateway, p - gateway);

			bzero(gateway, sizeof(gateway));
			sprintf(gateway, "%s.%s", result, "1");
		}
		strncat(send_buf, gateway, strlen(gateway));
		strcat(send_buf, "@");
	} else {
		strcat(send_buf, "0.0.0.0@0.0.0.0@0.0.0.0@");
		ak_print_normal("no ip/netmask/gateway !!!");
	}
}

/**
 * get_net_dns - 获取系统DNS  信息
 * @param[in]   char *dns_buf， 存放dns信息的bug 指针
 * return        NULL
 */
static void get_net_dns(char *dns_buf)
{
	char cmd[32] = {0};
	char result[128] = {0};
	char tmp_dns[32] = {0};
	char *p = NULL;
	int i, j;

	/*** to get dns message from system  ***/
	sprintf(cmd, "cat /etc/resolv.conf");

	/*** execute the command, the return message store in "result" ***/
	do_syscmd(cmd, result);
	p = result;

	/** cause the dns message format is : e.g. nameserver 192.168.xx.xx
	**   so we parse it as below, usually this msg has two tips, so we parse twice
	**/
	for (i = 0; i < 2; i++) {
		/** find the lable:nameserver at first **/
		p = strstr(p, "nameserver");

		/* If  the lable was found, do next */
		if(p) {
			j = 0;
			while(!isdigit(p[j]))	//skip no-number
				j++;
			p += j;

			if(p) { //find the first number
				j = 0;
				/** get the address by loop,  skip no-number and '.' **/
				while (1) {
					if(isdigit(p[j]))
						j++;
					else if(p[j] == '.')
						j++;
					else {
						strncpy(dns_buf, p, j);		//copy the address to the buf
						strcat(dns_buf, "@");		//add division lable
						//store the first dns
						strncpy(tmp_dns, dns_buf, strlen(dns_buf));
						dns_buf += strlen(dns_buf);
						p += j;
						break;
					}
				}
			} else
				break;
		} else if(i == 1) { //only one dns message, use the same
			strncpy(dns_buf, tmp_dns, strlen(tmp_dns));
			dns_buf += strlen(dns_buf);
			break;
		} else {	//no dns message
			sprintf(dns_buf, "0.0.0.0@0.0.0.0@");
			ak_print_normal_ex("It dose not get DNS.\n");
			break;
		}
	}
	return;
}

/*this is only for denghong Co.*/
static int get_uid(const char *uid_path,char * uid)
{
	int ret = AK_FAILED;

#if 0
	DIR *dir = opendir(uid_path);
	if (NULL == dir) {
		ak_print_normal_ex("it fails to open directory %s\n", uid_path);
		return -1;
	}

			
	struct dirent *dir_ent = NULL;

	while (NULL != (dir_ent = readdir(dir))) {
		if (NULL == dir_ent->d_name)
			continue;
		ak_print_normal_ex("%s \n",dir_ent->d_name);
		/* fine next when we get dir */
        if ((dir_ent->d_type & DT_DIR)) {
            continue;
        }
		if( strstr(dir_ent->d_name, "AJWL") && strstr(dir_ent->d_name, ".sn"))
		 {
			memcpy(uid,dir_ent->d_name,32);
			ret = AK_SUCCESS;
			break;
		}
	}
	closedir(dir);
#endif

	int fd = open("/etc/jffs2/custom_uid.txt", O_RDONLY);
	if(fd < 0) {
		ak_print_normal_ex(" open  file failed.\n");
		return ret;
	}

	if(32 != read(fd, uid, 32)) {
		ak_print_normal_ex(" read  file failed.\n");
		return ret;
	}

	ret = AK_SUCCESS;
	ak_print_normal(" uid: %s\n",uid);
	
	return ret;
}

/**
 * test_get_local_info - 接收到特定广播包信息后，通过本函数获取一些系统上的信息。
 *@param[in]   char * send_buf，存放系统需要发送的系统信息的buf 指针
 *@return        void
 */
static void test_get_local_info(char * send_buf)
{
	char result[64] = {0};
	char *p = send_buf;
	char dhcp[4] = {0};
	char soft_version[10] = {0};
	int i = 0;
	char mac[64]={0};	
	char uid[64] = {0};

	/*************** get channel name *****************/
	struct config_handle *config = test_config_get_value();
	strncpy(p, config->osd_name, strlen(config->osd_name));
	strcat(p, "@");	//add segmentation
	p += strlen(p);

	/*********  get net ip & netmask & gateway ***********/
	get_machine_ip(p);
	p += strlen(p);

	/*********  get net  dns ***********/
	get_net_dns(p);
	p += strlen(p);

	/*********  get net dhcp status ***********/
	sprintf(dhcp, "%d", config->dhcp);
	strncpy(p, dhcp, strlen(dhcp));
	strcat(p, "@");	//add segmentation
	p += strlen(p);

	/*********	get dev name ***********/
	strncpy(p, config->dev_name, strlen(config->dev_name));
	strcat(p, "@");	//add segmentation
	p += strlen(p);

	/*********	get soft version ***********/
	sprintf(soft_version, "%d", config->soft_version);
	strncpy(p, soft_version, strlen(soft_version));
	strcat(p, "@");	//add segmentation
	p += strlen(p);

	/*********	get mac addr ***********/
	do_syscmd("ifconfig | grep wlan0", result);
	if(strlen(result) > 0)  /*wifi mode*/
		do_syscmd("ifconfig wlan0 | grep 'HWaddr' | awk '{print $5}'", mac);
	else
		do_syscmd("ifconfig eth0 | grep 'HWaddr' | awk '{print $5}'", mac);

	while (mac[i]) {
        if (mac[i]=='\n' || mac[i]=='\r' || mac[i]==' ')
            mac[i] = '\0';
        i++;
    }
	strncpy(p, mac, strlen(mac));
	strcat(p, "@");	//add segmentation
	p += strlen(p);

	/*********	get UID ***********/
    if(AK_SUCCESS == get_uid("/etc/jffs2",uid))
		strncpy(p, uid, 32);
	strcat(p, "@"); //add segmentation
	p += strlen(p);

	return;
}

/**
 * broadcast_pth 接受广播包线程，对信息进行处理并返回给上位机
 * @return        NULL
 */
static void *test_broadcast_pth(void *arg)
{
	int udp_fd;	//sock fd
	int on = 1;
	char rec_buf[128] = {0};
	char snd_buf[256] = {0};
	char * flgs = "Anyka IPC ,Get IP Address!";
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("thread id: %ld\n", tid);

	socklen_t sockaddr_len;
	sockaddr_len = sizeof(struct sockaddr);

	struct sockaddr_in udp_addr, server_addr;
	memset(&udp_addr, 0, sizeof(struct sockaddr));
	memset(&server_addr, 0, sizeof(struct sockaddr));

	/*** set socket attribute ***/
	udp_addr.sin_family = AF_INET;			/** IPV4 **/
	udp_addr.sin_port = htons(UPD_PORT); 	/** port 8192 **/
	udp_addr.sin_addr.s_addr = INADDR_ANY;	/** any ip  **/

	while (1) {
		/** create socket  **/
		if ((udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
			perror("udp socket");
			ak_print_normal_ex("Create broadcast socket failed.\n");
			usleep(500);
			continue;
		} else {
			ak_print_normal_ex("create udp socket success, socket fd:%d.\n", udp_fd);
		}

		/** set socket options **/
		if (setsockopt(udp_fd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR,
				   	&on, sizeof(on)) != 0) {
			perror("setsockopt");
			close(udp_fd);
			usleep(500);
			continue;
		}

		/*** set close-on-exec flag ***/
		if (fcntl(udp_fd, F_SETFD, FD_CLOEXEC) == -1) {
			ak_print_normal_ex("error:%s\n", strerror(errno));
			close(udp_fd);
			usleep(500);
			continue;
		}

		/** bind the socket  **/
		if (bind(udp_fd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
			perror("bind udp");
			close(udp_fd);
			sleep(3);
			continue;
		}
		ak_print_normal_ex("ready to receive broadcast\n");

		while (1) {
			/*** receive the broadcast package blocking ***/
			if (recvfrom(udp_fd, rec_buf, sizeof(rec_buf), 0,
				(struct sockaddr *)&server_addr, &sockaddr_len) == -1) {
				ak_print_normal_ex("recv broadcast failed\n");
				perror("receive date failed\n");
				continue;
			} else {
				rec_buf[127] = 0;
				ak_print_normal_ex("recv PC broadcast: %s\n", rec_buf);

				/*
				 * compare the flag to make sure that
				 * the broadcast package was sent from our software \
				 */
				if (strncmp(rec_buf, flgs, strlen(flgs)) != 0) {
					ak_print_normal_ex("recv data is not match\n");
					bzero(rec_buf, sizeof(rec_buf));
					continue;
				}
				/*** get the machine info fill into the snd_buf ***/
				test_get_local_info(snd_buf);

				/** send the message back to the PC **/
				if(sendto(udp_fd, snd_buf, strlen(snd_buf), 0,
					(struct sockaddr *)&server_addr, sockaddr_len) == -1) {
					ak_print_normal_ex("send broadcast failed, %s\n", strerror(errno));
				}
				ak_print_normal_ex("Send Broadcast Success, info: [%s]\n", snd_buf);
				bzero(snd_buf, sizeof(snd_buf));
				bzero(rec_buf, sizeof(rec_buf));
			}
		}
		/** finish, close the socket **/
		close(udp_fd);
		break;
	}

	ak_print_normal_ex("Exit thread, id: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

static void *test_ircut_pth(void *param)
{
	int *run_flag = (int *)param;
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("thread id: %ld\n", tid);

	struct config_handle *config = test_config_get_value();
	test_ircut_on_off(vi_handle, config->day_ctrl);
	
	*run_flag = 0;
	return NULL;
}

static void *test_mmc_pth(void *param)
{
	int *run_flag = (int *)param;
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("thread id: %ld\n", tid);
	test_mmc();

	mmc_test_finish = 1;
	
	*run_flag = 0;
	return NULL;
}

static void *test_recover_pth(void *param)
{
	int *run_flag = (int *)param;
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("thread id: %ld\n", tid);
	test_recover_dev(1000);
	
	*run_flag = 0;
	return NULL;
}


static void *test_wifi_pth(void *param)
{
	int *run_flag = (int *)param;
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("thread id: %ld\n", tid);
	test_wifi();

	*run_flag = 0;
	return NULL;
}

void test_ircut_background(void)
{
	static int run_flag = 0;
	if(run_flag != 0) {
		ak_print_normal_ex("test_ircut is still running\n");
		return;
	}
	
	run_flag = 1;
	/* test ircut background */
	ak_thread_create(&ircut_pth_id , test_ircut_pth, 
			(void *)&run_flag, ANYKA_THREAD_MIN_STACK_SIZE, -1);
}

void test_mmc_background(void)
{
	mmc_test_finish = 0;
	
	static int run_flag = 0;
	if(run_flag != 0) {
		ak_print_normal_ex("test_mmc is still running\n");
		return;
	}	
	run_flag = 1;
	/* test card background */
	ak_thread_create(&mmc_pth_id , test_mmc_pth, 
			(void *)&run_flag, ANYKA_THREAD_MIN_STACK_SIZE, -1);
}

void test_recover_background(void)
{	
	static int run_flag = 0;
	if(run_flag != 0) {
		ak_print_normal_ex("test_reset is still running\n");
		return;
	}	
	run_flag = 1;
	/* test card background */
	ak_thread_create(&recover_pth_id , test_recover_pth, 
			(void *)&run_flag, ANYKA_THREAD_MIN_STACK_SIZE, -1);
}


void test_wifi_background(void)
{	
	static int run_flag = 0;

	if(run_flag != 0) {
		ak_print_normal_ex("test_wifi is still running\n");
		return;
	}
	run_flag = 1;
	/* test wifi background */
	ak_thread_create(&wifi_pth_id , test_wifi_pth, 
			(void *)&run_flag, ANYKA_THREAD_MIN_STACK_SIZE, -1);
}


int test_get_conn_state(void)
{
	return conn_state;
}

/**
 * start_test - start  test
 * @vi_h[IN]: opened vi handle 
 * @ai_h[IN]: opened ai handle
 * @ao_h[IN]: opened ao handle
 * return:   0 , success ;  -1 , failed;
 * notes: main program has became a test program before test_init operate
 */
int start_test (void *vi_h, void *ai_h, void *ao_h)
{
	int ret = 0;
	
	vi_handle = vi_h; 
	ai_handle = ai_h;
	ao_handle = ao_h;	
	
	test_mmc_background();
	test_wifi_background();
	test_ptz_init();
	test_ircut_background();
	test_recover_background();
	/*
	 * test main  thread
	 * connect to pc tool,  get cmd from pc tool and send result to it
	 */
	ak_thread_create(&tcp_sock_id, test_tcp_pth,  
		(void *)0, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	/*
	 * heartbeat  thread
	 * connect to pc tool, notice it is alive
	 */
	ak_thread_create(&send_hb_pid, send_hb_func, (void *)NULL,
				   ANYKA_THREAD_MIN_STACK_SIZE, -1);	
	/*
	 * BROADCAST  thread
	 * receive pc tool broadcast message, and send paltform info
	*/
	ak_thread_create(&broadcast_sk_id, test_broadcast_pth, 
		NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);

	return ret;
}
