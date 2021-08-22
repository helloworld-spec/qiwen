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

#include <linux/netlink.h>
#include <linux/version.h>
#include <linux/input.h>

#include "ak_thread.h"
#include "ak_common.h"
#include "ak_cmd_exec.h"
#include "ak_net.h"

#include "ak_ini.h"
#ifdef CONFIG_ONVIF_SUPPORT
	#define CONFIG_ANYKA_FILE_NAME  	"/etc/jffs2/onvif_cfg.ini"
#else
	#define CONFIG_ANYKA_FILE_NAME  	"/etc/jffs2/anyka_cfg.ini"
#endif

#define FIFO_PATH			"/tmp/daemon_fifo"
#define	RESTART_AK_IPC		"/usr/sbin/anyka_ipc.sh"
#define UPDATE_SH			"/usr/sbin/update.sh"
#define SERVICE				"/usr/sbin/service.sh"

#define FIFO_SZ             (50)
#define INTERVAL            (3) //3seconds

#define KEY_GPIO_DEV		"/dev/input/event0"
#define SDP1_DEV_NAME       "/dev/mmcblk0p1"
#define SD_DEV_NAME         "/dev/mmcblk0"

#define MOUNT_SDP1			"mount -rw /dev/mmcblk0p1 /mnt"
#define MOUNT_SD			"mount -rw /dev/mmcblk0 /mnt"
#define UMOUNT_SD			"umount /mnt -l"

#define UEVENT_BUFFER_SIZE  2048

/* key 1: soft reset */
#define RECOVER_DEV			(5*1000)
#define UPDATE_IMAGE		(10*1000)   //10 seconds

#define TCP_PORT 		    6789
#define TCP_HB_PORT		    6790
#define UPD_PORT 		    8192
#define SUCCESS			    "1"
#define FAILURE			    "0"
#define TEST_STAT 		    "/var/stat"
#define TCP_BUF_SIZE 	    1024

#define DAEMON_DEBUG 	    0       //for test print
#define STANDBY_TEST 	    0       //for test standby
#define UPDATE_TEST 	    0       //for test update

/****************  watch_dog *********************/
#ifdef WATCHDOG_ENABLE
#define WATCHDOG_IOCTL_BASE 'W'
#define WDIOC_KEEPALIVE     _IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define	WDIOC_SETTIMEOUT    _IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_GETTIMEOUT    _IOR(WATCHDOG_IOCTL_BASE, 7, int)

static int wdt_fd = 0;
static ak_pthread_t watchdog_id;
#endif

static int watchdog_en = 1;
static int system_start_flag = 1;

/*************  TCP/IP TEST  DATA STRUCT **************/
enum {
	COMMAND = 1,
	RESPONSE,
	HEARTBEAT,
	FINISH,
};

enum key_press_status_e {
    KEY_PRESS_RESERVED = -1,
	KEY_PRESS_UP = 0x00,
	KEY_PRESS_DOWN,
	KEY_PRESS_NONE,
};

/***** tcp communite data struct  *****/
struct data_struct {
	unsigned short len;
	unsigned short arg_len;
	unsigned short file_len;
	unsigned short check_sum;
	unsigned char type;
	unsigned char auto_test;
	char *file_name;
	char *arg_data;
};

struct socket_data {
	int sock_fd;
	int pth_runflags;
};

struct config_handle {
	void *handle;
	char osd_name[50];
	char dev_name[100];
	int dhcp;
	unsigned int soft_version;
};

static int monitor_runflags = 0;
static int test_running = 0;

static unsigned char timer_run_flag = AK_FALSE;
static unsigned char dev_di = AK_FALSE;
static unsigned char image_di = AK_FALSE;

static struct timeval start_time;
static struct config_handle ini_config = {0};

enum key_press_status_e key_press_status = KEY_PRESS_RESERVED;
char last_file[20] = {0};

static void daemon_init_ini(void)
{
	ini_config.handle = ak_ini_init(CONFIG_ANYKA_FILE_NAME);
}

static void daemon_config_get_value(void)
{
	char value[50] = {0};

	/* get dhcp config */
    ak_ini_set_item_value(ini_config.handle, "ethernet", "dhcp", value);
	ini_config.dhcp = atoi(value);

	/* get osd name */
	ak_ini_get_item_value(ini_config.handle, "camera", "osd_name", 
			ini_config.osd_name);

	/* get device name */
	ak_ini_set_item_value(ini_config.handle, "global", "dev_name", 
			ini_config.dev_name);

	/* get global software version */
	bzero(value, sizeof(value));
	ak_ini_set_item_value(ini_config.handle, "global", "soft_version", value);
	ini_config.soft_version = atoi(value);
}

static void daemon_exit_ini(void)
{
	if (!ini_config.handle) {
		ak_ini_destroy(ini_config.handle);
		ini_config.handle = NULL;
	}
}

/*
* @daemon_init, make current process to be a daemon process
* @return: 0 on success, -1 on faield
*/
static int daemon_init(void)
{
	int a, max_fd, i;

	/** 1 **/
	signal(SIGHUP, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	/*** 2 ***/
	a = fork();
	if (a > 0)
		exit(EXIT_SUCCESS);
	/*** 3 ***/
	setsid();
	/*** 4 ***/
	a = fork();
	if (a > 0)
		exit(EXIT_SUCCESS);
	/*** 5 ***/
	setpgrp();
	/*** 6 ***/
	max_fd = sysconf(_SC_OPEN_MAX);
	for (i = 3; i < max_fd; i++)
		close(i);
	/*** 7 ***/
	umask(0);
	/*** 8 ***/
	chdir("/");

	return 0;
}

/**
 * *  @brief		enable_watch_dog pthread
 * *  @author	chen yanhong
 * *  @date		2014-09-17
 * *  @param	not used
 * *  @return		void  *
 * */
#ifdef WATCHDOG_ENABLE
static void *feed_watchdog(void *param)
{
	int time_val = -1;
	struct ak_timeval tv_start;
	struct ak_timeval tv_end;
	unsigned long ts_interval = 0;
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("thread id: %ld\n", tid);

	ak_get_ostime(&tv_start);
	while (watchdog_en) {
        ioctl(wdt_fd, WDIOC_KEEPALIVE, 0);
        ak_sleep_ms(2*1000);

		ak_get_ostime(&tv_end);
		ts_interval = ak_diff_ms_time(&tv_end, &tv_start);
		if (ts_interval > 2500)
			ak_print_normal_ex("schedule to long, interval: %lu\n",
					ts_interval);
		ak_get_ostime(&tv_start);
    }

	ak_print_normal_ex("going to close watch_dog\n");
	ioctl(wdt_fd, WDIOC_SETTIMEOUT, &time_val);

	if (close(wdt_fd) < 0) {
		perror("close watch_dog:");
	}

	ak_print_notice_ex("watch_dog closed, thread id: %ld\n", tid);
	ak_thread_exit();

    return NULL;
}

/**
 * *  @brief		enable_watch_dog
 * *  @author	chen yanhong
 * *  @date		2014-09-17
 * *  @param	null
 * *  @return		void
 * */
static void watchdog_enable(void)
{
	ak_print_normal_ex("********** Watch Dog Enabled! **********\n");

    wdt_fd = open("/dev/watchdog", O_RDONLY);
    if (wdt_fd < 0) {
        ak_print_error_ex("open wdt_fd failed\n");
    } else {
		watchdog_en = 1;
		fcntl(wdt_fd, F_SETFD, FD_CLOEXEC);

		int timeout = 0;
        ioctl(wdt_fd, WDIOC_GETTIMEOUT, &timeout);
        ak_print_notice_ex("watchdog timeout = %d(s)\n", timeout);
        ak_thread_create(&watchdog_id, feed_watchdog, NULL,
				ANYKA_THREAD_MIN_STACK_SIZE, -1);
    }
}

/**
 * *  @brief		close_watch_dog && close_monitor_anyka_ipc
 * *  @author	chen yanhong
 * *  @date		2016-03-30
 * *  @param	signal id
 * *  @return		void
 * */
static void close_watch_dog(int signo)
{
	watchdog_en = 0;
}
#endif

/**
 * *  @brief		set_timer
 * *  @author	chen yanhong
 * *  @date		2014-09-17
 * *  @param	sec, second you want to set
 * *  @return		void
 * */
void daemon_set_timer(int sec)
{
	struct itimerval s_time;

	s_time.it_value.tv_sec  = sec;
	s_time.it_value.tv_usec = 0;

	s_time.it_interval.tv_sec  = 0; //是否需要多次使用?
	s_time.it_interval.tv_usec = 0;

	setitimer(ITIMER_REAL, &s_time, NULL);

	return ;
}

static void play_ding_voice(void)
{
    ak_cmd_exec("ccli misc --tips \"/usr/share/anyka_di_cn.mp3\"", NULL, 0);
}

static void print_key_hold_msg(int hold_time)
{
    ak_print_normal("***********************************************\n");
	ak_print_normal("KEY Press down over %d second, You can loosen it!\n",
	    hold_time);
	ak_print_normal("***********************************************\n");
}

/**
 * *  @brief		alarm_print,signal SIGALRM callback
 * *  @author	chen yanhong
 * *  @date		2014-09-17
 * *  @param	msg
 * *  @return		void
 * */
static void daemon_alarm_print(int msg)
{
	if(!timer_run_flag)
		return;

	switch(msg){
	case SIGALRM:
	    switch (key_press_status) {
	    case KEY_PRESS_UP:
            play_ding_voice();
	        print_key_hold_msg(10);
	        break;
	    case KEY_PRESS_DOWN:
	        play_ding_voice();
            key_press_status = KEY_PRESS_NONE;
            daemon_set_timer(5);
			print_key_hold_msg(5);
	        break;
	    default:
	        break;
	    }
		break;
	default:
		break;
	}
}

/**
 * *  @brief		creat_fifo
 * *  @author		chen yanhong
 * *  @date		2014-09-17
 * *  @param		void
 * *  @return		void
 * */
/********************* 创建管道**************************/
static void daemon_create_fifo()
{
	if(access(FIFO_PATH, F_OK) < 0) {
		mkfifo(FIFO_PATH, 0777);
	}
}

/**
 * *  @brief		monitor
 * *  @author		chen yanhong
 * *  @date		2014-09-17
 * *  @param		void
 * *  @return		NULL
 * */
/******************* 监控线程 ***********************/
static void *monitor_thread(void *arg)
{
	char cmd[128] = {0};
	char recv_hb[FIFO_SZ] = {0};
	int fifo_fd, rd_ret;		//chech heart beat fd
	long int tid = ak_thread_get_tid();

	ak_print_normal_ex("thread id: %ld\n", tid);

	daemon_create_fifo();
	ak_print_normal_ex("interval: %d(sec), fifo[%s].size=%d\n",
			INTERVAL, FIFO_PATH, FIFO_SZ);

	fifo_fd = open(FIFO_PATH, O_RDONLY);	//open fifo
	fcntl(fifo_fd, F_SETFD, FD_CLOEXEC);	//set attrubitu

	while (monitor_runflags && watchdog_en) {
		rd_ret = read(fifo_fd, recv_hb, FIFO_SZ);

		if (rd_ret <= 0) {
			ak_print_normal("*********************************\n");
			ak_print_normal("The MainApp was dead, restart it!\n");

			sprintf(cmd, "%s %s", RESTART_AK_IPC, "restart");
			ak_print_normal("call: %s\n", cmd);
			ak_cmd_exec(cmd, NULL, 0);
		}
		sleep(INTERVAL);
	}
	close(fifo_fd);

	ak_print_normal_ex("Exit monitor, thread id: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

static long daemon_diff_timeval(struct timeval *new_tv, struct timeval *old_tv)
{
	return ((new_tv->tv_sec - old_tv->tv_sec) * 1000)
	    + ((new_tv->tv_usec - old_tv->tv_usec) / 1000);
}

/**
 * *  @brief       daemon_mount_sd
 * *  @author
 * *  @date
 * *  @param[in]   int flag, 1->mmcblkp0,    0->mmcblk
 * *  @return      void
 * */
/* create a sd_test dir and mount the sd card in it */
void daemon_mount_sd(int flag)
{
	char cmd[128];

    if(flag == 0) {
        if(access(SDP1_DEV_NAME, R_OK) >= 0) {
            ak_print_normal("**********we will skip mount /dev/mmcblk0*******\n");
            return;
        }
    }

	if (flag) {
		sprintf(cmd, "%s", MOUNT_SDP1);
	} else {
		sprintf(cmd, "%s", MOUNT_SD);
	}
	char result[10] = {0};
	ak_cmd_exec(cmd, result, 10);

	ak_print_normal_ex("*** mount the sd to /mnt ***\n");
}

/**
 * daemon_umount_sd - 卸载SD 卡
 */
/* umount the sd card and delete the sd_test dir */
void daemon_umount_sd(void)
{
	// the card had extracted, so the sync is no use actually
	char result[10] = {0};
	ak_cmd_exec(UMOUNT_SD, result, 10);
	ak_print_normal_ex("*** umount the sd ***\n");
}

/**
 * daemon_init_hotplug_socka - 热插拔检测
 * return int， -1， 失败；else 成功
 */
/* create the socket to recevie the uevent */
static int daemon_init_hotplug_sock(void)
{
	struct sockaddr_nl snl;
    const int buffersize = 2048;
    int retval;

    memset(&snl, 0x00, sizeof(struct sockaddr_nl));

    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;

	int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (hotplug_sock == -1)	{
        ak_print_normal_ex("socket: %s\n", strerror(errno));
        return -1;
    }

    /* set receive buffersize */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE,
			&buffersize, sizeof(buffersize));

    retval = bind(hotplug_sock, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
    if (retval < 0) {
	    ak_print_normal_ex("bind failed: %s\n", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
    }

    return hotplug_sock;
}

static void check_sd_card(int hotplug_sock)
{
    char buf[UEVENT_BUFFER_SIZE * 2] = {0};

	/* block here to wait signal */
    recv(hotplug_sock, &buf, sizeof(buf), 0);

    int i = 0;
    int p1_removed = 0;
    char temp_buf[20] = {0};	//find action
    char *p = strrchr(buf, '/');	//get block name

    for (i = 0; buf[i] != '@' && buf[i] != 0; i++)
        temp_buf[i] = buf[i];
    temp_buf[i] = 0;

	if (strcmp(temp_buf, "change"))
		ak_print_normal("%s\n", buf);

	//card insert
	if (!strcmp(temp_buf, "add")) {
		if (!strcmp(p, "/mmcblk0p1")) {
			//sleep(1);
			daemon_mount_sd(1);
		} else if (!strcmp(p, "/mmcblk0")) {
			//sleep(1);
			daemon_mount_sd(0);
		}

		p1_removed = 0;
		return;
	}

	//card extract
	if (!strcmp(temp_buf, "remove")) {

		//if p1 removed, than we do need to umount k0
		if(!strcmp(p, "/mmcblk0p1")) {
			daemon_umount_sd();
			p1_removed = 1;
		} else if((!strcmp(p, "/mmcblk0")) && (!p1_removed))
			daemon_umount_sd();
	}
}

/* waitting the uevent and do something */
static void *daemon_pth_func(void *data)
{
	long int tid = ak_thread_get_tid();
	ak_print_normal_ex("thread id: %ld\n", tid);

    /* hotplug socket handle */
	int hotplug_sock = daemon_init_hotplug_sock();
    if (hotplug_sock < 0)
		return NULL;

    system_start_flag = 1;
	sleep(1);
    if (access (SDP1_DEV_NAME, F_OK) == 0)
		daemon_mount_sd(1);
	else if (access (SD_DEV_NAME, F_OK) == 0)
		daemon_mount_sd(0);
    system_start_flag = 0;

    while(1) {
        check_sd_card(hotplug_sock);
    }

	ak_print_normal_ex("Exit thread, id : %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

static int do_gpio_key_0(double period)
{
    if (period >= UPDATE_IMAGE) {
        if (image_di) {
            image_di = AK_FALSE;
            print_key_hold_msg(10);
            play_ding_voice();
            sleep(3);
        }

		/***** update device *****/
		ak_print_normal_ex("update device, call:%s\n", UPDATE_SH);
		ak_cmd_exec(UPDATE_SH, NULL, 0);
    } else if (period >= RECOVER_DEV) {
        if (dev_di) {
            dev_di = AK_FALSE;
            print_key_hold_msg(5);
            play_ding_voice();
            sleep(3);
        }

        ak_print_normal_ex("*** recover system config ***\n");
		/***** recover device *****/
		char result[10] = {0};
		ak_cmd_exec("/usr/sbin/recover_cfg.sh", result, 10);
		ak_cmd_exec("reboot", NULL, 0);
    } else {
        ak_print_normal_ex("switch wps\n");
		/*do switch wps*/
    }

	return 0;
}

/**
 * do_gpio_key_0
 * @event[IN]: input_event struct
 * return: 1 key press down; 0 key press up
 *
 */
static int daemon_do_gpio_key_0(struct input_event *event)
{
    int ret = 0;
	long period  = 0;

	switch (event->value) {
    case KEY_PRESS_UP:
        timer_run_flag = AK_FALSE;
		period = daemon_diff_timeval(&event->time, &start_time);
		ak_print_normal("recover key press up, period=%ld(ms)\n", period);
		do_gpio_key_0(period);
		ret = 0;
        break;
    case KEY_PRESS_DOWN:
        start_time.tv_sec = event->time.tv_sec;
		start_time.tv_usec = event->time.tv_usec;
        key_press_status = KEY_PRESS_DOWN;
        ak_print_normal("recover key press down, start calculating time...\n\n");
		timer_run_flag = AK_TRUE;
		dev_di = AK_TRUE;
        image_di = AK_TRUE;
		ret = 1;
        break;
    default:
        break;
	}

	return ret;
}

#if UPDATE_TEST
static int do_gpio_key_1(double period)
{
	if (period < UPDATE_IMAGE) {
		timer_run_flag = AK_FALSE;
		if(period < 0) {
			perror("Error period");
			return -1;
		}
	} else {
		ak_print_normal("update call:%s\n", UPDATE_SH);
		ak_cmd_exec(UPDATE_SH, NULL, 0);
	}

	return 0;
}
#endif

/**
 * *  @brief		do_gpio_key_1
 * *  @author		chen yanhong
 * *  @date		2014-09-17
 * *  @param		input_event struct
 * *  @return		key press down,return 1;else return 0.
 * */
static int daemon_do_gpio_key_1(struct input_event *event)
{
    int ret = 0;

    switch (event->value) {
    case KEY_PRESS_UP:
    {
#if UPDATE_TEST
		long period = daemon_diff_timeval(&event->time, &start_time);
		ak_print_normal("period=%ld(ms)\n", period);
		do_gpio_key_0(period);
#endif
		ret = 0;
		break;
	}
	case KEY_PRESS_DOWN:
#if UPDATE_TEST
		daemon_set_timer(10);
		timer_run_flag = AK_TRUE;
		start_time.tv_sec = event->time.tv_sec;
		start_time.tv_usec = event->time.tv_usec;
#endif
        ret = 1;
	    break;
    default:
        break;
    }

	return ret;
}

#if STANDBY_TEST
/**
 * *  @brief			standby mode
 * *  @author		chen yanhong
 * *  @date			2014-12-08
 * *  @param		input_event struct
 * *  @return		void.
 * */
static int daemon_do_standby(struct input_event *event)
{
#define PRESS_STANDBY_TIME	2
	long period = 0;

    switch (event->value) {
    case KEY_PRESS_UP:      //松开停止计时，获取按键时长
		period = daemon_diff_timeval(&event->time, &start_time);
		ak_print_normal("period=%ld(ms)\n", period);
		if (period >= PRESS_STANDBY_TIME) {
#if 0
			/* 设置RTC alarm唤醒时间,单位秒 */
			rtc_wakeup_standby(60 * 2);
#endif
			/* 进入standby */
			ak_cmd_exec("/usr/sbin/standby.sh &", NULL, 0);
		}
        break;
    case KEY_PRESS_DOWN:    //按下开始计时
        start_time.tv_sec = event->time.tv_sec;
		start_time.tv_usec = event->time.tv_usec;
        break;
    default:
        break;
    }

	return 0;
}
#endif

/**
 * do_key
 * @key_event[IN]   struct input_event *event,
 * @key_cnt[IN] event count
 * return 0 on success
 * */
static int do_key(struct input_event *key_event, int key_cnt)
{
	int i = 0;
	int ret = -1;
	struct input_event *event;

	if(test_running)
		return 0;

	if (key_cnt < (int) sizeof(struct input_event)) {
		ak_print_normal("expected %d bytes, got %d\n",
				sizeof(struct input_event), key_cnt);
		return -1;
	}

	for (i = 0; (i < key_cnt/sizeof(struct input_event)); i++) {
		event = (struct input_event *)&key_event[i];
		if (EV_KEY != event->type) {
			continue;
		}
		ak_print_normal("count=%d, code=%d, value=%d\n",
				key_cnt/sizeof(struct input_event), event->code, event->value);

		ak_print_normal_ex("handler event:");
		switch (event->code) {
		case KEY_0:
			ak_print_normal("KEY_0\n");
			ret = daemon_do_gpio_key_0(event);
			break;
		case KEY_1:
			ak_print_normal(" KEY_1\n");
			ret = daemon_do_gpio_key_1(event);

#if STANDBY_TEST
			/* standby test */
			daemon_do_standby(event);
#endif
			break;
		default:
			ak_print_normal_ex("event->code: %d, Error key code!", event->code);
			ret = -1;
			break;
		}
		if (!ret){
			break;
		}
	}

	return ret;
}

/**
 * *  @brief       daemon_sig_fun, now no used
 * *  @author      chen yanhong
 * *  @date        2014-10-28
 * *  @param[in]   send buf, a pointer to data
 * *  @return      void
 * */

void daemon_sig_fun(int signo)
{
	ak_print_normal("SIGUSR1 call /usr/sbin/hello.sh\n");
	ak_cmd_exec("/usr/sbin/hello.sh", NULL, 0);
}


/**
 * cp the struct data pointer by data to the buf
 *param[in]   send buf, a pointer to data
 *return      void
 */
static void daemon_cp_to_buf(char * send_buf, struct data_struct *data)
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
 *calculate the data's sum which pointer by data
 *@param[in]   a pointer to data
 *@return      result after calculate
 */
static int daemon_calc_checksum(struct data_struct *data)
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
static int daemon_response(int send_to_fd, int result)
{
	char send_buf[200] = {0};
	int send_fd = send_to_fd;
	int ret;

	struct data_struct *response_data = (struct data_struct *)calloc(1,
			sizeof(struct data_struct));
	if(response_data == NULL) {
		ak_print_normal_ex("fail to calloc\n");
		return -1;
	}

	response_data->type = RESPONSE;		//message type
	response_data->auto_test = 1;
	response_data->file_len = 0;
	response_data->file_name = NULL;
	response_data->arg_len = 1;		//just report one bit, 1 -- > success, 0, failed

	response_data->len = (sizeof(response_data->len)) + (sizeof(response_data->type))
		+ (sizeof(response_data->auto_test)) + (sizeof(response_data->file_len))
		+ (sizeof(response_data->arg_len)) + (response_data->file_len)
	   	+ (response_data->arg_len) +(sizeof(response_data->check_sum));
	//success
	if (result == 0) {
		response_data->arg_data = (char *)calloc(1, 2);
		strncpy(response_data->arg_data, SUCCESS, 1);
	} else { //failure
		response_data->arg_data = (char *)calloc(1, 2);
		strncpy(response_data->arg_data, FAILURE, 1);
	}
	response_data->check_sum = daemon_calc_checksum(response_data);
	daemon_cp_to_buf(send_buf, response_data);

#if DAEMON_DEBUG
	int i;
	for (i = 0; i < 10; i++) {
		ak_print_normal("%02x ", send_buf[i]);
	}
	ak_print_normal("\n");
#endif

	/* send message */
	ret = send(send_fd, send_buf, response_data->len, 0);
	if(ret < 0) {
		ak_print_normal_ex("send response data faild, error:%s\n", strerror(errno));
	}

#if DAEMON_DEBUG
	ak_print_normal("respons len : %d\n", response_data->len);
	ak_print_normal("respons type : %d\n", response_data->type);
	ak_print_normal("respons auto_test : %d\n", response_data->auto_test);
	ak_print_normal("respons file_len : %d\n", response_data->file_len);
	ak_print_normal("respons arg_len : %d\n", response_data->arg_len);
	ak_print_normal("respons chenck_sum : %d\n", response_data->check_sum);
#endif

	/* release resource */
	if(response_data->arg_data)
		free(response_data->arg_data);
	if(response_data->file_name)
		free(response_data->file_name);
	free(response_data);

	return ret;
}

/**
 *  @brief       	 get the test program's execute result
 *  @param[in]   void
 *  @return        result = 0,success; -1,failure
 */
static int daemon_get_exec_result()
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

/**
 * *  @brief       	 fill the info to the struct
 * *  @author      	 chen yanhong
 * *  @date       	 2014-10-28
 * *  @param[in]   info buf which will be filled to the struct
 * *  @return        a pointer to data
 * */
struct data_struct *daemon_create_struct_info(char *buf)
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
static void daemon_destroy_struct_info(struct data_struct *data)
{
	if(data->file_name)
		free(data->file_name);
	if(data->arg_data)
		free(data->arg_data);
	if(data)
		free(data);
	return ;
}

/**
 *  parse receive data, then execute it
 *@ info buf which was receive by system call recv, client socket file descriptor
 *@return        execute result, 0 succed ;else failure
 * */
int daemon_parse_exec(char *buf, int cfd)
{
	char cmd_buf[256] = {0};
	char res[10] = {0};
	short check_sum_test;
	int ret = 0;
	struct data_struct *parse_data;
	const char *work_path = "/tmp";

	/** change word dir the /tmp **/
	if (chdir(work_path))
		ak_print_error_ex("change dir to tmp fail, %s\n", strerror(errno));

	parse_data = daemon_create_struct_info(buf);
	if(parse_data == NULL)
		return -1;

	/* debug print */
#if DAEMON_DEBUG
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
	if(parse_data->len == 0) {
		ak_print_normal_ex("receive data error, data size is 0\n");
		ret = -1;
		goto bye_bye;
	}

	//check sum
	check_sum_test = daemon_calc_checksum(parse_data);
	if(check_sum_test != parse_data->check_sum)
	{
		ak_print_normal_ex("receive data error, the check sum is not coincident.\n");
		ret = -1;
		goto bye_bye;
	}

	//kill last program
	if(last_file[0]) {
		bzero(cmd_buf, sizeof(cmd_buf));

		sprintf(cmd_buf, "/usr/sbin/kill_pro.sh %s", last_file);
		ak_cmd_exec(cmd_buf, res, 10); 	//kill program

		bzero(last_file, sizeof(last_file));
		bzero(cmd_buf, sizeof(cmd_buf));
	}

	//check finish
	if (parse_data->type == FINISH) {
		ak_print_normal_ex("going to restart service\n");
		ret = 2;
		goto bye_bye;
	}

	if (access(parse_data->file_name, F_OK) < 0) {
		ak_print_error_ex("the executable file: %s not exist.\n",
				parse_data->file_name);
		ret = -1;
		goto bye_bye;
	}

	/* 1 */
	sprintf(cmd_buf, "chmod u+x %s/%s", work_path, parse_data->file_name);
	ak_cmd_exec(cmd_buf, res, 10);
	bzero(cmd_buf, sizeof(cmd_buf));

	/* 2 remove status file */
	remove(TEST_STAT);

	/* 3 */
	//manual test mode, response message to PC
	if((!(parse_data->auto_test)) || (2 == parse_data->auto_test)) {
		daemon_response(cfd, 0);	//response the message immediately on manual test mode
		bzero(cmd_buf, sizeof(cmd_buf));
		if(parse_data->arg_data) {
			//run background
			sprintf(cmd_buf, "%s/%s %s/%s %s %s", work_path, parse_data->file_name,
						work_path, parse_data->arg_data, TEST_STAT, "&");
		} else {	//run background
			sprintf(cmd_buf, "%s/%s %s %s", work_path,
					parse_data->file_name, TEST_STAT, "&");
		}
		ret = 1;
	} else {
		if(parse_data->arg_data) { //has arg
			sprintf(cmd_buf, "%s/%s %s/%s %s", work_path, parse_data->file_name,
					work_path, parse_data->arg_data, TEST_STAT);
		} else { //has not arg
			sprintf(cmd_buf, "%s/%s %s", work_path, parse_data->file_name, TEST_STAT);
		}
	}
	ak_print_normal_ex("cmd: %s\n", cmd_buf);
	if(2 != parse_data->auto_test)
	    strcpy(last_file, parse_data->file_name);	//save last file name
	ak_print_normal_ex("last file name: %s, %d\n", last_file, ret);

	if (strcmp(parse_data->file_name,"test_update") == 0){
		ak_print_normal_ex("test_update\n");
		ak_cmd_exec(cmd_buf, NULL, 0); 	//execute the command
	}
	else {
		ak_cmd_exec(cmd_buf, res, 10); //execute the command
		if(ret != 1) {
			ret = daemon_get_exec_result();	//get the command execute result
		}
	}

bye_bye:
	daemon_destroy_struct_info(parse_data);		//free the message resource
	return ret;
}

static void check_key_hold(void)
{
    struct timeval cur_time;

    gettimeofday(&cur_time, NULL);
    long period = daemon_diff_timeval(&cur_time, &start_time);
    ak_print_normal_ex("period=%ld(ms)\n", period);
    if (image_di && (period >= UPDATE_IMAGE)) {
        image_di = AK_FALSE;
        print_key_hold_msg(10);
        play_ding_voice();
    } else if (dev_di && (period >= RECOVER_DEV)) {
        dev_di = AK_FALSE;
        print_key_hold_msg(5);
        play_ding_voice();
    }
}

static int daemon_handle_key(void)
{
    int ret = 0;
    int gpio_fd;

	/* open gpio key device */
	if ((gpio_fd = open(KEY_GPIO_DEV, O_RDONLY)) < 0) {
		perror("Open gpio key dev fail");
		return -ENOENT;
	}

    int rd = 0;
	struct input_event key_event[64];
	struct timeval tv;
    fd_set readfds, tempfds;

	FD_ZERO(&readfds);
	/* add the gpio_fd to the readfds set */
	FD_SET(gpio_fd, &readfds);
	/* set the gpio_fd's close-on-exec attribute */
	fcntl(gpio_fd, F_SETFD, FD_CLOEXEC);

	/* check key pressed */
	while (1) {
    	tv.tv_sec = 1;
    	tv.tv_usec = 0;

		tempfds = readfds;
		/**** monitor the tempfds no-blocking ****/
		ret = select((gpio_fd + 1), &tempfds, NULL, NULL, &tv);
		switch (ret) {
    	case -1:
    		perror("select");
    	case 0:     //timeout
            if (timer_run_flag) {
                check_key_hold();
            }
    		break;
    	default:
    		/** To test whether the gpio_fd's status has changed **/
    		if (FD_ISSET(gpio_fd, &tempfds)) {
    			/** read the event to the buf **/
    			rd = read(gpio_fd, key_event,
    					sizeof(struct input_event) * sizeof(key_event));
    			/*** parse the event ***/
    			do_key(key_event, rd);
    		}
    		break;
    	}
	}

	close(gpio_fd);
	return 0;
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
	struct socket_data *pc_data = (struct socket_data *)arg;
	int *run_flag = (int *)&pc_data->pth_runflags;
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
	hb.check_sum = daemon_calc_checksum((struct data_struct *)&hb);
	hb.len = (sizeof(hb.len)) + (sizeof(hb.type)) + (sizeof(hb.auto_test))
	   	+ (sizeof(hb.file_len)) + (sizeof(hb.arg_len)) +
		(hb.file_len)+(hb.arg_len) + (sizeof(hb.check_sum));

	/* copy to send buf */
	daemon_cp_to_buf(send_buf, &hb);

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
	pc_sock = accept(sock_fd, (struct sockaddr *)&peer_addr, (socklen_t *)&sinsize);
	if(pc_sock == -1) {
		ak_print_error_ex("accept: %s\n", strerror(errno));
		goto close_sk;
	}
	ak_print_normal_ex("start send heart beat.\n");

	while (*run_flag) {
		/* send the heart beat package pointer by send_buf to PC */
		send_size = send(pc_sock, send_buf, hb.len, 0);
		if(send_size < 0) {
			ak_print_error_ex("send: %s.\n", strerror(errno));
			break;
		}
		sleep(2);	// two sec interval
	}

close_sk:
	close(sock_fd);

	ak_print_normal_ex("Exit heart beat thread, id: %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

/**
 * tcp_pth -
 * @monitor_pid[IN]   monitor pthread id
 * return void *
 */
static void *tcp_pth(void *monitor_pid)
{
	int sock_fd;	//sock fd
	int sinsize;
	int cmd_ret = 0;
	int recv_size;
	//int close_flags = 1;	//kill anyka_ipc and watch dog flag
	//ak_pthread_t m_pid = *((ak_pthread_t *)monitor_pid);	//get monitor pid
	ak_pthread_t send_hb_pid;	//send heart beat pid
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

	while (1) {
		/*** create socket ***/
		sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(sock_fd == -1) {
			ak_print_error_ex("fail to create TCP socket.\n");
			continue;
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
		pc_communicate->sock_fd = accept(sock_fd,
				(struct sockaddr *)&peer_addr, (socklen_t *)&sinsize);
		if (pc_communicate->sock_fd == -1) {
			ak_print_error_ex("accept: %s\n", strerror(errno));
			goto close_sk;
		} else {
			ak_print_normal_ex("This machine has been connnected\n");
			/* the test is running, make the keys lose efficacy */
			test_running = 1;

			/* create pthread to send heart beat */
			pc_communicate->pth_runflags = 1;
			ak_thread_create(&send_hb_pid, send_hb_func, (void *)pc_communicate,
				   ANYKA_THREAD_MIN_STACK_SIZE, -1);
#if 0
			if(close_flags) {
				monitor_runflags = 0;
				system("killall -12 daemon");	//close watch dog
				system("killall -9 net_manage.sh");
				system("killall -9 anyka_ipc");
				ak_print_normal_ex("Start Test Mode.\n");
				close_flags = 0;
				//sleep(1);
			}
#endif
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

				/* analyse command, than execute it. */
				cmd_ret = daemon_parse_exec(receive_buf, pc_communicate->sock_fd);
				if(cmd_ret == -1) { //after execute the cmd, return  error
					ak_print_error_ex("execute command failed.\n");
					if(daemon_response(pc_communicate->sock_fd, (-1)) < 0) {
						ak_print_error_ex("send response failed.\n");
					}
				} else if(cmd_ret == 0) { //pc socket close
					if(daemon_response(pc_communicate->sock_fd, (0))) {
						ak_print_normal("send response success.\n");
					}
				} else if(cmd_ret == 2) { //it has receive a finish signal
					daemon_response(pc_communicate->sock_fd, (0));
					close(sock_fd);
					ak_print_normal_ex("####!!!! Restart device !!!!####\n");
					ak_cmd_exec("reboot", NULL, 0);
					exit(0);	//no use actually, cause the system has been reboot
				} else { //manual test
					ak_print_normal_ex("Manual test mode, it has send report to PC\n");
				}
				memset(receive_buf, 0, TCP_BUF_SIZE);
			}
			pc_communicate->pth_runflags = 0;	//close heart beat pthread
			ak_thread_join(send_hb_pid);
		}

close_sk:
		close(sock_fd);
        if(cmd_ret == 2)
        {
			daemon_response(pc_communicate->sock_fd, (0));
            ak_print_normal("Receive Finish Signal, reboot the device.\n");
			free(pc_communicate);
			free(receive_buf);
			ak_cmd_exec("reboot", NULL, 0);
            exit(1);
        }
	}
	free(pc_communicate);
	free(receive_buf);

	ak_print_normal("Exit the socket thread, id : %ld\n", tid);
	ak_thread_exit();
	return NULL;
}

/*
 * get_machine_net - 获取机器当前使用的网卡的IP，子网掩码和网关
 * ifname[IN], pointer to current working net interface name
 * net_buf[OUT], store ip/netmask/gateway/dns info
 * return: none
 */
static void get_machine_net(char *ifname, char *net_buf)
{
	char ipaddr[32] = {0};
	char netmask[32] = {0};
	char gateway[32] = {0};
	char dns1[32] = {0};
	char dns2[32] = {0};

	if (!ifname || !net_buf) {
		ak_print_error_ex("invalid argument, %p, %p\n", ifname, net_buf);
		return;
	}

	/********* get ip  ********/
	if (ak_net_get_ip(ifname, ipaddr))
		sprintf(ipaddr, "0.0.0.0");

	if (ak_net_get_netmask(ifname, netmask))
		sprintf(netmask, "0.0.0.0");

	if (ak_net_get_route(ifname, gateway))
		sprintf(gateway, "%s", ipaddr);

	/* get first and sub dns */
	if (ak_net_get_dns(0, dns1))
		sprintf(dns1, "0.0.0.0");
	if (ak_net_get_dns(1, dns2))
		sprintf(netmask, "%s", dns1);

	/* ip@netmask@gateway@firstdns@seconddns@ */
	sprintf(net_buf, "%s@%s@%s@%s@%s@", ipaddr, netmask, gateway, dns1, dns2);
}

/**
 * daemon_get_local_info - 接收到特定广播包信息后，通过本函数获取一些系统上的信息。
 *@param[in]   char * send_buf，存放系统需要发送的系统信息的buf 指针
 *@return        void
 */
static void daemon_get_local_info(char * send_buf)
{
	char *p = send_buf;
	char dhcp[4] = {0};
	char soft_version[10] = {0};
	char mac[64]={0};
	char ifname[100] = {0};

	if (ak_net_get_cur_iface(ifname)) {
		ak_print_error_ex("no net interface working\n");
		return;
	}

	/*************** get channel name *****************/
	daemon_config_get_value();

	strncpy(p, ini_config.osd_name, strlen(ini_config.osd_name));
	strcat(p, "@");	//add segmentation
	p += strlen(p);

	/*********  get net ip & netmask & gateway & dns ***********/
	get_machine_net(ifname, p);
	p += strlen(p);

	/*********  get net dhcp status ***********/

	sprintf(dhcp, "%d", ini_config.dhcp);
	strncpy(p, dhcp, strlen(dhcp));
	strcat(p, "@");	//add segmentation
	p += strlen(p);

	/*********	get dev name ***********/
	strncpy(p, ini_config.dev_name, strlen(ini_config.dev_name));
	strcat(p, "@");	//add segmentation
	p += strlen(p);

	/*********	get soft version ***********/
	sprintf(soft_version, "%d", ini_config.soft_version);
	strncpy(p, soft_version, strlen(soft_version));
	strcat(p, "@");	//add segmentation
	p += strlen(p);

	/*********	get mac addr ***********/
	ak_net_get_mac(ifname, mac, 64);
	strncpy(p, mac, strlen(mac));
	strcat(p, "@");	//add segmentation
	p += strlen(p);
}

/**
 * broadcast_pth 接受广播包线程，对信息进行处理并返回给上位机
 * @return        NULL
 */
static void *broadcast_pth(void *arg)
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
				daemon_get_local_info(snd_buf);

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

static void process_signal(unsigned int sig, siginfo_t *si, void *ptr)
{
	ak_backtrace(sig, si, ptr);
	if(sig == SIGINT || sig == SIGSEGV) {

	}

	if(sig == SIGTERM) {
	}
	exit(1);
}

static int register_signal(void)
{
	struct sigaction s;

	s.sa_flags = SA_SIGINFO;
	s.sa_sigaction = (void *)process_signal;

	sigaction(SIGSEGV, &s, NULL);
	sigaction(SIGINT, &s, NULL);
	sigaction(SIGTERM, &s, NULL);

	signal(SIGALRM, daemon_alarm_print);	//for timer
	signal(SIGUSR1, daemon_sig_fun);		//now no use
	signal(SIGPIPE, SIG_IGN);

	return 0;
}

/* main program has became a daemon program before daemon_init operate */
int main (int argc, char **argv)
{
	daemon_init();
	register_signal();

	ak_print_normal("***************************************\n");
	ak_print_normal("*****A monitor daemon has running!*****\n");
	ak_print_normal("***************************************\n");

	/* watch_dog */
#ifdef WATCHDOG_ENABLE
	/* register signal for close watch dog and close monitor anyka_ipc */
	signal(SIGUSR2, close_watch_dog);
	watchdog_enable();		//enable watchdog
#else
	signal(SIGUSR2, SIG_IGN);
#endif

	daemon_init_ini();

    ak_pthread_t monitor_pth_id;
    ak_pthread_t tcp_sock_id;
    ak_pthread_t broadcast_sk_id;
    ak_pthread_t pth;

	/* daemon pthread */
	monitor_runflags = 1;
	ak_thread_create(&monitor_pth_id, monitor_thread,
				NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);

	/* TCP/IP work thread */
	ak_thread_create(&tcp_sock_id, tcp_pth,
		(void *)&monitor_pth_id, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);

	/* BROADCAST thread */
	/* 接收PC端广播，返回当前机器的信息 */
	ak_thread_create(&broadcast_sk_id, broadcast_pth,
		NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);

	/*** 检测热插拔状态并做相应处理的线程 ***/
	ak_thread_create(&pth, daemon_pth_func, NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);

    daemon_handle_key();

	/** 停止热插拔检测线程 **/
	ak_thread_cancel(pth);

	daemon_exit_ini();

	return 0;
}
