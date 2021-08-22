#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#include "danavideo_cmd.h"
#include "danavideo.h"
#include "danavideo_cloud.h"
#include "debug.h"

#include "dana_common.h"
#include "dana_wifi.h"
#include "dana_cmd.h"
#include "dana_others.h"
#include "dana_av.h"
#include "dana_osd.h"

#include "ak_global.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_config.h"
#include "ak_dana.h"
#include "ak_net.h"
#include "ak_misc.h"
#include "ak_alarm.h"
#include "ak_cmd_exec.h"
#include "ak_dvr_common.h"
#include "record_ctrl.h"
#include "ak_dvr_replay.h"

#define SDC_PATH 			"/mnt/"
#define CONFIG_PATH			"/etc/jffs2/anyka_cfg.ini"

/* 网络连接成功，应用上线提示 */
#define SYS_CONNECTED 		"/usr/share/anyka_net_connected.mp3"
/* 摄像机上线提示音 */
#define SYS_ONLINE			"/usr/share/anyka_camera_online.mp3"
/* 摄像机下线提示音 */
#define SYS_OFFLINE			"/usr/share/anyka_camera_off_line.mp3"
/* 正在连接 */
#define SYS_CONNECTING_NET	"/usr/share/anyka_connecting_net.mp3"
/* 获取智能声控传递的信息 */
#define SYS_GET_CONFIG		"/usr/share/anyka_camera_get_config.mp3"

#define WIRE_LESS_TMP_DIR   "/tmp/wireless/"
//#define DANA_TMP_OFF
#define MAX_ANYKA_IPC_MEM   (9*1024) //If over this size. Then stop recv video and audio buff to send(kBytes)
#define DANA_CONN_MAX (6)            //max dana app client number
#define NUM_GETWIFIAP_RESPONSE 5                                                                    //尝试将wifi列表上传的失败重试次数
#define DANA_VIDEO_BUFFER_SIZE 3*1024*1024                                                          //default size is 2mb(设置大拿最大视频缓存大小)

struct ak_dana {
	unsigned char run_flag;
	bool airlink_called;
	int preview_status;
	DANAVIDEOCMD_SETTIME_ARG dana_time;

	ak_mutex_t lock;
	MYDATA mydata[DANA_CONN_MAX];
	void *vi_handle;
};

enum dana_newwork_status {
	NETWORK_INIT = 0 ,
	NETWORK_UNDONE ,
	NETWORK_FINISH ,
} ;

/* danale platform audio function flag */
extern unsigned int dana_func_mode;
const char *danale_path = "/etc/jffs2/";
/* time zone which is got from danale */
int dana_time_zone = 0;
static int send_data_flag = 1;
static struct ak_dana dana_ctrl = {0};
static const char dana_version[] = "libcloud_dana V1.1.08";
static int dana_flag = NETWORK_INIT;

/**
 * ak_dana_get_version - get dana version
 * return: version string
 */
const char* ak_dana_get_version(void)
{
	return dana_version;
}

/**
 * dana_sleep - sleep function,but exit quickly
 * @msec[IN]:  msecond
 * return: void
 */
static void dana_sleep(int msec)
{
	int msecond = msec;
	if(msecond <= 0)
		return ;
	ak_sleep_ms(msec);
	/* in order to exit quickly */
	//while(dana_ctrl.run_flag && (msecond > 0)) {
	//	ak_sleep_ms(50);
	//	msecond -= 50;
	//}
}

/**
 * convert_timezone - convert time_zone string to seconds
 * @time_zone[IN]: time zone string
 * return:  seconds of time zone
 */
static int convert_timezone(const char *time_zone)
{
	int seconds = 0;
	int hour = 0, min = 0;
	if(time_zone) {
		/* sscanf(ntp_server1+4, "%d:%d", &hour, &min); */
		hour = atoi(time_zone+4);
		min = atoi(time_zone+7);
		seconds = hour * 3600 + min * 60;
		if(time_zone[3]!='+'){
			seconds = 0 - seconds;
		}
	}
	return seconds;
}

/**
 * danavideoconn_created - callback function,
 *	when danale create new connection, call this
 * @arg[IN]: ptr to danale connection
 * return:  0
 */
static uint32_t danavideoconn_created(void *arg) // pdana_video_conn_t
{
	pdana_video_conn_t *danavideoconn = (pdana_video_conn_t *)arg;
	struct _MyData *mydata = NULL;

	ak_print_normal_ex("conn:%p\n", danavideoconn);
	ak_thread_mutex_lock(&dana_ctrl.lock) ;
	for (int i = 0; i < DANA_CONN_MAX; i++){
		if ( NULL == dana_ctrl.mydata[i].danavideoconn) {
			mydata = &(dana_ctrl.mydata[i]);
			break;
		}
	}
	if (NULL == mydata){
		ak_thread_mutex_unlock(&dana_ctrl.lock);
		dbg(" danavideoconn_created conn over %d failed\n", DANA_CONN_MAX);
		ak_print_normal_ex("dana conn over %d\n", DANA_CONN_MAX);
		return -1;
	}
	mydata->danavideoconn = danavideoconn;
	mydata->chan_no = 1;
	mydata->video_conn_flg = 0;
	mydata->audio_conn_flg = 0;
	strncpy(mydata->appname, "anyka_video", strlen("anyka_video"));
	/* set user data */
	if (0 != lib_danavideo_set_userdata(danavideoconn, mydata)) {
			mydata->danavideoconn = NULL;
			ak_thread_mutex_unlock(&dana_ctrl.lock);
			dbg(" danavideoconn_created lib_danavideo_set_userdata failed\n");
			return -1;
	}
	ak_thread_mutex_unlock(&dana_ctrl.lock);
	dbg("TEST danavideoconn_created succeed\n");
	ak_print_normal_ex("succeed\n");

	return 0;
}

/**
 * danavideoconn_aborted - callback function,
 *	when danale destroy old connection, call this
 * @arg[IN]: ptr to danale connection
 * return:  void
 */
static void danavideoconn_aborted(void *arg) // pdana_video_conn_t
{
	dbg("\n\n\n\n\n\x1b[34mTEST danavideoconn_aborted\x1b[0m\n\n\n\n\n\n");
	pdana_video_conn_t *danavideoconn = (pdana_video_conn_t *)arg;
	struct _MyData *mydata = NULL;

	ak_print_normal_ex("arg:%p\n",arg);
	ak_thread_mutex_lock(&dana_ctrl.lock) ;
	for (int i = 0; i < DANA_CONN_MAX; i++){
		if (dana_ctrl.mydata[i].danavideoconn == danavideoconn) {
			ak_print_normal_ex("find conn ok:%p\n", danavideoconn);
			mydata = &(dana_ctrl.mydata[i]);
			 /*
			 * stop any data that send to danale.
			 * otherwise, maybe cause signal 11.
			 */
			dana_cmd_stop_rec_play(mydata);
			dana_av_stop(CALLBACK_MAX, mydata);
			mydata->danavideoconn = NULL;
			mydata->video_conn_flg = 0;
			mydata->audio_conn_flg = 0;
			break;
		}
	}
	ak_thread_mutex_unlock(&dana_ctrl.lock);
	dbg("TEST danavideoconn_aborted succeed\n");

	return;
}

/**
 * danavideoconn_command_handler - callback function,
 *	when danale excute one command, call this
 * @arg[IN]: ptr to danale connection
 * @cmd[IN]: cmd
 * @trans_id[IN]: id of this command
 * @cmd_arg[IN]: command arg
 * @cmd_arg_len[IN]: command arg length
 * return:  void
 */
static void danavideoconn_command_handler(void *arg, dana_video_cmd_t cmd,
	   	uint32_t trans_id, void *cmd_arg, uint32_t cmd_arg_len)
{
	pdana_video_conn_t *danavideoconn = (pdana_video_conn_t *)arg;
	MYDATA *mydata;
	uint32_t i;
	uint32_t error_code = 0;
	char *code_msg = (char *)"";
	struct ak_date date = {0};

	ak_get_localdate(&date);
	ak_print_date(&date);
	ak_print_normal("danavideoconn_command_handler: cmd=%d, arg: %p\n", cmd,arg);

	/** get userdata **/
	if (0 != lib_danavideo_get_userdata(danavideoconn, (void**)&mydata)) {
		ak_print_warning("lib_danavideo_get_userdata failed\n");
		return;
	}
	if(NULL == mydata){
		ak_print_normal_ex("mydata:%p arg:%p !\n", mydata,arg);
		return ;
	}

	switch (cmd) {
		/* Recover Device */
		case DANAVIDEOCMD_DEVDEF: {
				dana_av_stop(CALLBACK_MAX,mydata);
				/* video_record_err_stop();
					        * fisrt response app , otherwise timeout.
					        */
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd,
							(char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler "
							"DANAVIDEOCMD_DEVDEF send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler "
							"DANAVIDEOCMD_DEVDEF send response failed\n");
				}
				sleep(1);
				ak_print_normal("***Ready to recover system configure!!!***\n");

				/* script to recover configure file */
				char result[4] = {0};
				ak_cmd_exec("/usr/sbin/recover_cfg.sh", result, sizeof(result));
				sync();
				ak_print_normal("***Ready to reboot!!!***\n");
				/* reboot and use new configure file */
				ak_cmd_exec("reboot", NULL, 0);
			}
			break;
		/* reboot Device */
		case DANAVIDEOCMD_DEVREBOOT: {
				dana_av_stop(CALLBACK_MAX,mydata);

				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT send response failed\n");
				}
				sleep(1);
				ak_cmd_exec("reboot", NULL, 0);
			}
			break;
		/* get screen, take photo */
		case DANAVIDEOCMD_GETSCREEN: {
				 if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSCREEN send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSCREEN send response failed\n");
				}
				   /*  anyka_start_picture(mydata, ak_dana_send_pictrue); */
			}
			break;
		/* get cloud alarm info */
		case DANAVIDEOCMD_GETALARM: {
				DANAVIDEOCMD_SETALARM_ARG sys_alarm;
				/* get cloud alarm info */
				uint32_t pir_detection = 1;
				dana_cmd_get_alarm(&sys_alarm);
				if (lib_danavideo_cmd_getalarm_response(danavideoconn, trans_id, error_code, code_msg,
					sys_alarm.motion_detection, sys_alarm.opensound_detection, sys_alarm.openi2o_detection,
					sys_alarm.smoke_detection, sys_alarm.shadow_detection, sys_alarm.other_detection,
					&pir_detection)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETALARM send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETALARM send response failed\n");
				}
			}
			break;
		/* get device base info */
		case DANAVIDEOCMD_GETBASEINFO: {
				char *dana_id =  lib_danavideo_deviceid_from_conf("/etc/jffs2");
				char *api_ver = lib_danavideo_linked_version_str(lib_danavideo_linked_version());
				char *sn      = (char *)"di3_3";
				char *device_name = (char *)"di3_4";
				char *rom_ver = dana_others_get_verion();
				ak_print_normal("dana_id:%s api_ver:%s rom_ver:%s\n",dana_id,api_ver,rom_ver);
				uint32_t device_type = 1;
				uint32_t ch_num = 25;
				uint64_t sdc_size = ak_get_sdcard_total_size(SDC_PATH);	/** get sd card total size **/

				uint64_t sdc_free = ak_get_sdcard_free_size(SDC_PATH);	/** get sd card free size **/
				size_t work_channel_count = 2;
				uint32_t work_channel = 1;
				if (lib_danavideo_cmd_getbaseinfo_response(danavideoconn, trans_id, error_code, code_msg, dana_id, api_ver, sn,\
					device_name, rom_ver, device_type, ch_num, sdc_size, sdc_free, work_channel_count, &work_channel)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETBASEINFO send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETBASEINFO send response failed\n");
				}
			}
			break;
		/* get system color */
		case DANAVIDEOCMD_GETCOLOR: {
				int brightness =0, contrast = 0, saturation = 0, hue =0;
				/*
					 * you will get lum/contrast/saturation/hue  from ipc,and send to client
					 * the value is between 0 to 100
					 */
				/************************************************************************/
				ak_print_normal("GET_COLOR\n");
				/* get system color */
				/* Get_Color( &brightness, &contrast, &saturation, &hue ); */
				if (lib_danavideo_cmd_getcolor_response(danavideoconn, trans_id, error_code, code_msg, brightness, contrast, saturation, hue)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCOLOR send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCOLOR send response failed\n");
				}
			}
			break;
		/* get camera flip */
		case DANAVIDEOCMD_GETFLIP: {
				uint32_t flip_type = dana_cmd_get_video_flip();
				if (lib_danavideo_cmd_getflip_response(danavideoconn, trans_id, error_code, code_msg, flip_type)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFLIP send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFLIP send response failed\n");
				}
			}
			break;
		/* get function list */
		case DANAVIDEOCMD_GETFUNLIST: {
				uint32_t methodes_count = 43;
				char *methods[] = { (char *)"DANAVIDEOCMD_DEVDEF",       /* Recover Device */	//0
						(char *)"DANAVIDEOCMD_DEVREBOOT",        /* reboot Device */
						(char *)"DANAVIDEOCMD_GETSCREEN",        /* get screen, take photo */
						(char *)"DANAVIDEOCMD_GETALARM",         /* get cloud alarm info */
						(char *)"DANAVIDEOCMD_GETBASEINFO",      /* get device base info */
						(char *)"DANAVIDEOCMD_GETCOLOR",         /* get system color */			//5
						(char *)"DANAVIDEOCMD_GETFLIP",          /* get camera flip */
						(char *)"DANAVIDEOCMD_GETFUNLIST",       /* get function list */
						(char *)"DANAVIDEOCMD_GETNETINFO",       /* get net info */
						(char *)"DANAVIDEOCMD_GETPOWERFREQ",     /* get power frequece */
						(char *)"DANAVIDEOCMD_GETTIME",          /* get system times */			//10
						(char *)"DANAVIDEOCMD_GETWIFIAP",        /* get ap , use system wlan0 to scan */
						(char *)"DANAVIDEOCMD_GETWIFI",          /* get wifi message from system */
						(char *)"DANAVIDEOCMD_PTZCTRL",          /* ptz control */
						(char *)"DANAVIDEOCMD_SDCFORMAT",        /* format sd card */
						(char *)"DANAVIDEOCMD_SETALARM",         /* set sys alarm */			//15
						(char *)"DANAVIDEOCMD_SETCHAN",          /* set channal */
						(char *)"DANAVIDEOCMD_SETCOLOR",         /* set color */
						(char *)"DANAVIDEOCMD_SETFLIP",          /* set flip */
						(char *)"DANAVIDEOCMD_SETNETINFO",       /* set net info */
						(char *)"DANAVIDEOCMD_SETPOWERFREQ",     /* set power frequece */		//20
						(char *)"DANAVIDEOCMD_SETTIME",          /* set sys time */
						(char *)"DANAVIDEOCMD_SETVIDEO",         /* set sys video */
						(char *)"DANAVIDEOCMD_SETWIFIAP",        /* set ap */
						(char *)"DANAVIDEOCMD_SETWIFI",          /* set wifi */
						(char *)"DANAVIDEOCMD_STARTAUDIO",       /* start audio */				//25
						(char *)"DANAVIDEOCMD_STARTTALKBACK",    /* start talk */
						(char *)"DANAVIDEOCMD_STARTVIDEO",       /* start video */				//27
						(char *)"DANAVIDEOCMD_STOPAUDIO",        /* stop audio */				//28
						(char *)"DANAVIDEOCMD_STOPTALKBACK",     /* stop talk */
						(char *)"DANAVIDEOCMD_STOPVIDEO",        /* stop video */				//30
						(char *)"DANAVIDEOCMD_RECLIST",          /* get record list */
						(char *)"DANAVIDEOCMD_RECPLAY",          /* record replay start */
						(char *)"DANAVIDEOCMD_RECSTOP",          /* record replay stop */
						(char *)"DANAVIDEOCMD_RECACTION",        /* record replay pause or go on */
						(char *)"DANAVIDEOCMD_RECSETRATE",       /* record replay set rates */	//35
						(char *)"DANAVIDEOCMD_RECPLANGET",       /* record replay get plan */
						(char *)"DANAVIDEOCMD_RECPLANSET",       /* record replay set plan */
						(char *)"DANAVIDEOCMD_EXTENDMETHOD" ,    /* extend method */
						(char *)"DANAVIDEOCMD_SETOSD",           /* set osd */
						(char *)"DANAVIDEOCMD_GETOSD",           /* get osd message */			//40
						(char *)"DANAVIDEOCMD_SETCHANNAME",      /* get channal name */
						(char *)"DANAVIDEOCMD_GETCHANNAME"       /* set channal name */
					};
				if (lib_danavideo_cmd_getfunlist_response(danavideoconn, trans_id, error_code, code_msg, methodes_count, (const char**)methods)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFUNLIST send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFUNLIST send response failed\n");
				}
			}
			break;
		/* get net info */
		case DANAVIDEOCMD_GETNETINFO: {
				struct sys_net_config *sys_net = ak_config_get_sys_net(); //get system network config
				if (!sys_net) {
					ak_print_error_ex("get sys net failed\n");
					break;
				}

				/*
				 * we use the run-time network config
				 * send to dana
				 */
				/* IPv4 16bit */
				char if_name[10] = {0};
				char ip[16] = {0};
				char mask[16] = {0};
				char route[16] = {0};
				char dns1[16] = {0};
				char dns2[16] = {0};

				if (ak_net_get_cur_iface(if_name)) {
					ak_print_error_ex("no working net face\n");
					break;
				}
				ak_net_get_ip(if_name, ip);
				ak_net_get_netmask(if_name, mask);
				ak_net_get_route(if_name, route);
				ak_net_get_dns(0, dns1);
				ak_net_get_dns(1, dns2);
				ak_print_notice_ex("get net info: \n"
						"ip: %s, mask: %s, route: %s, dns1: %s, dns2: %s, dhcp: %d\n",
						ip, mask, route, dns1, dns2, sys_net->dhcp);

				error_code = 0;
				code_msg = (char *)"";
				if (lib_danavideo_cmd_getnetinfo_response( danavideoconn, trans_id,
							error_code, code_msg, sys_net->dhcp,
							ip, mask, route,
							sys_net->dhcp, dns1, dns2, 80)) {
					dbg("TEST danavideoconn_command_handler "
							"DANAVIDEOCMD_GETNETINFO send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler"
							" DANAVIDEOCMD_GETNETINFO send response failed\n");
				}
			}
			break;
		/* get power frequece */
		case DANAVIDEOCMD_GETPOWERFREQ: {
			   int freq = 0;
				/*
				* you can get the power freq from ipc,and send to client
				* 0: 50HZ
				* 1: 60HZ
				*/
				freq = ((dana_cmd_get_freq(dana_ctrl.vi_handle) == 50)?DANAVIDEO_POWERFREQ_50HZ : DANAVIDEO_POWERFREQ_60HZ) ;	/** get power frequece **/

				if (lib_danavideo_cmd_getpowerfreq_response(danavideoconn, trans_id, error_code, code_msg, freq)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPOWERFREQ send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPOWERFREQ send response failed\n");
				}
			}
			break;
		/* get system times */
		case DANAVIDEOCMD_GETTIME: {
				int64_t now_time = dana_ctrl.dana_time.now_time;

				char *time_zone = dana_ctrl.dana_time.time_zone;
				char *ntp_server_1 = dana_ctrl.dana_time.ntp_server1;
				char *ntp_server_2 = dana_ctrl.dana_time.ntp_server2;
				if (lib_danavideo_cmd_gettime_response(danavideoconn, trans_id, error_code, code_msg,
													now_time, time_zone, ntp_server_1, ntp_server_2)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETTIME send response succeed\n");
					ak_print_normal("TEST danavideoconn_command_handler DANAVIDEOCMD_GETTIME send response succeed\n");
				}
				else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETTIME send response failed\n");
					ak_print_normal("TEST danavideoconn_command_handler DANAVIDEOCMD_GETTIME send response failed\n");
				}
			}
			break;
		/* get ap , use system wlan0 to scan */
		case DANAVIDEOCMD_GETWIFIAP: {
				uint32_t wifi_device = 1;
				int ap_num = 0, j = 0 , i ;
				libdanavideo_wifiinfo_t *wifi_list = NULL;
				/* firstly check wifi driver if install ok */
				if(dana_wifi_driver_chk() < 0) {                                                    //判断是否启动了wifi驱动
					lib_danavideo_cmd_getwifiap_response(danavideoconn, trans_id, error_code, code_msg, wifi_device,
						ap_num, (const libdanavideo_wifiinfo_t *)NULL);
					break;
				}
				/* get ap list , use system wlan1 to scan */
				struct ak_ap_list *ap_list = dana_wifi_get_ap_list(); //local format ap list
				if( NULL != ap_list) {
					wifi_list = (libdanavideo_wifiinfo_t *)calloc(1,
							sizeof(libdanavideo_wifiinfo_t) * ap_list->ap_num); //upload format libdanavideo_wifiinfo_t
				}
				if(wifi_list == NULL) {
					ak_print_normal("wifi_list NULL.\n");
				}
				else {
					/* strore wifi list message by loop */
					for(j = 0; j < ap_list->ap_num; j++) {
						wifi_list[j].enc_type = ap_list->ap_info[j].enc_type;  //ssid加密类型
						wifi_list[j].quality  = ap_list->ap_info[j].quality;   //信号质量
						strcpy(wifi_list[j].essid, ap_list->ap_info[j].essid); //ssid名称
						ak_print_normal("wifi:%d, enc:%d, quality:%d, ssid:%s\n",
							j+1, wifi_list[j].enc_type, wifi_list[j].quality, wifi_list[j].essid);
					}
					ap_num = ap_list->ap_num;
				}
				for( i = 0 ; i < NUM_GETWIFIAP_RESPONSE ; i ++ ) {                  //上传wifi列表失败则重试上传
					if (lib_danavideo_cmd_getwifiap_response(danavideoconn, trans_id, error_code, code_msg, wifi_device, ap_num, (const libdanavideo_wifiinfo_t *)wifi_list)) {
						dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFIAP send response succeed\n");
						break ;
					} else {
						dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFIAP send response failed\n");
					}
				}
				if(ap_list){
					free(ap_list);
				}
				if(wifi_list){
					free(wifi_list);
				}
			}
			break;
		/* get wifi message from system */
		case DANAVIDEOCMD_GETWIFI: {
				struct sys_wifi_config *resp = ak_config_get_sys_wifi(); //get system wifi config
				if (lib_danavideo_cmd_getwifi_response(danavideoconn, trans_id, error_code, code_msg, resp->ssid, resp->passwd, resp->enc_type)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFI send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFI send response failed\n");
				}
			}
			break;
		/* ptz control */
		case DANAVIDEOCMD_PTZCTRL: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL\n");
				DANAVIDEOCMD_PTZCTRL_ARG *ptzctrl_arg = (DANAVIDEOCMD_PTZCTRL_ARG *)cmd_arg;
				dbg("code: %ld\n", ptzctrl_arg->code);
				dbg("para1: %ld\n", ptzctrl_arg->para1);
				dbg("para2: %ld\n", ptzctrl_arg->para2);
				dbg("\n");
				dana_others_set_ptz_cmd(ptzctrl_arg->code, ptzctrl_arg->para1); //set cmd to ptz  and  ptz run

				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL send response failed\n");
				}
			}
			break;
		/* format sd card */
		case DANAVIDEOCMD_SDCFORMAT: {
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SDCFORMAT send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SDCFORMAT send response failed\n");
				}
				ak_print_info_ex("format card function on!\n");
				record_ctrl_request_format_card(); //tell dvr to do format card.
			}
			break;
		/* set sys alarm */
		case DANAVIDEOCMD_SETALARM: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM\n");
				DANAVIDEOCMD_SETALARM_ARG *setalarm_arg = (DANAVIDEOCMD_SETALARM_ARG *)cmd_arg;

				if(setalarm_arg->motion_detection) {
					/* start sys motion_detection */
					ak_alarm_start(SYS_DETECT_MOVE_ALARM,
							setalarm_arg->motion_detection);
				} else {
					/* stop sys motion_detection */
					ak_print_normal_ex("stop md\n");
					ak_alarm_stop(SYS_DETECT_MOVE_ALARM);
				}
				if(setalarm_arg->opensound_detection) {
					/* start sys opensound_detection */
					ak_alarm_start(SYS_DETECT_VOICE_ALARM,
							setalarm_arg->opensound_detection);
				} else {
					/* stop sys opensound_detection */
					ak_print_normal_ex("stop aed\n");
					ak_alarm_stop(SYS_DETECT_VOICE_ALARM);
				}

				dana_cmd_set_alarm(setalarm_arg); //set alarm configure

				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
						   	trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM send response failed\n");
				}
			}
			break;
		/* set channal */
		case DANAVIDEOCMD_SETCHAN: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN\n");
				DANAVIDEOCMD_SETCHAN_ARG *setchan_arg = (DANAVIDEOCMD_SETCHAN_ARG *)cmd_arg;

				for (i=0; i<setchan_arg->chans_count; i++) {
					dbg("chans[%ld]: %ld\n", i, setchan_arg->chans[i]);
				}
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN send response failed\n");
				}
			}
			break;
		/* set color */
		case DANAVIDEOCMD_SETCOLOR: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR\n");

				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR send response failed\n");
				}
			}
			break;
		/* set flip */
		case DANAVIDEOCMD_SETFLIP: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETFLIP\n");
				DANAVIDEOCMD_SETFLIP_ARG *setflip_arg = (DANAVIDEOCMD_SETFLIP_ARG *)cmd_arg;
				//ak_print_normal_ex("filp:%d \n", setflip_arg->flip_type);
				dana_cmd_set_video_flip(dana_ctrl.vi_handle, setflip_arg->flip_type);
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETFLIP send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETFLIP send response failed\n");
				}
			}
			break;
		/* set net info */
		case DANAVIDEOCMD_SETNETINFO: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO\n");
				DANAVIDEOCMD_SETNETINFO_ARG *setnetinfo_arg = (DANAVIDEOCMD_SETNETINFO_ARG *)cmd_arg;

				ak_print_normal_ex("ip:%s mask:%s gateway:%s\n", setnetinfo_arg->ipaddr,
					setnetinfo_arg->netmask, setnetinfo_arg->gateway);

				struct sys_net_config *net_info = ak_config_get_sys_net(); /** get net info **/
				net_info->dhcp = setnetinfo_arg->ip_type;
				if (strlen( setnetinfo_arg->ipaddr)> 0)
					strcpy(net_info->ipaddr, setnetinfo_arg->ipaddr);
				if (strlen(setnetinfo_arg->netmask) > 0)
					strcpy(net_info->netmask, setnetinfo_arg->netmask);
				if (strlen( setnetinfo_arg->gateway) > 0)
					strcpy(net_info->gateway, setnetinfo_arg->gateway);
				if (!setnetinfo_arg->dns_type) {
					strcpy(net_info->firstdns, setnetinfo_arg->dns_name1);
					strcpy(net_info->backdns, setnetinfo_arg->dns_name2);
				}
				/* set net info */
				ak_config_set_sys_net(net_info);
				/* flush net info to configure file */
				ak_config_flush_data();

				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO send response failed\n");
				}
			}
			break;
		/* set power frequece */
		case DANAVIDEOCMD_SETPOWERFREQ: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ\n");
				DANAVIDEOCMD_SETPOWERFREQ_ARG *setpowerfreq_arg = (DANAVIDEOCMD_SETPOWERFREQ_ARG *)cmd_arg;

				/* set power frequece */
				dana_cmd_set_freq(dana_ctrl.vi_handle,
						(setpowerfreq_arg->freq == DANAVIDEO_POWERFREQ_50HZ) ? 50:60);
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ send response failed\n");
				}
			}
			break;
		/* set sys time */
		case DANAVIDEOCMD_SETTIME: {
				ak_print_normal("\nSetting times...\n");
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETTIME\n");
				DANAVIDEOCMD_SETTIME_ARG *settime_arg = (DANAVIDEOCMD_SETTIME_ARG *)cmd_arg;
				dbg("settime_arg\n");
				dbg("ch_no: %ld\n", settime_arg->ch_no);
				ak_print_normal_ex("\n\tnow_time: %lld\n"
					"\ttime_zone: %s\n\tntp_server1: %s\n\tntp_server2: %s\n",
		   				 settime_arg->now_time, settime_arg->time_zone,
		   				 settime_arg->ntp_server1, settime_arg->ntp_server2);
				dana_ctrl.dana_time.now_time = settime_arg->now_time;
				strncpy(dana_ctrl.dana_time.time_zone ,settime_arg->time_zone,64);
				strncpy(dana_ctrl.dana_time.ntp_server1, settime_arg->ntp_server1,256);
				strncpy(dana_ctrl.dana_time.ntp_server2, settime_arg->ntp_server2,256);
				/* set sys time */
				dana_time_zone = convert_timezone(dana_ctrl.dana_time.time_zone);
				dana_others_settime(settime_arg->now_time + dana_time_zone) ; //set device time

				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETTIME send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETTIME send response failed\n");
				}
			}
			break;
		/* set sys video */
		case DANAVIDEOCMD_SETVIDEO: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO\n");
				DANAVIDEOCMD_SETVIDEO_ARG *setvideo_arg = (DANAVIDEOCMD_SETVIDEO_ARG *)cmd_arg;

				/* set video quality */
				int video_quality = setvideo_arg->video_quality;
				ak_print_normal("mydata->device_type:%d video_quality:%d \n",mydata->device_type,video_quality);


				/* set video quality */
				uint32_t set_video_fps = dana_av_start_video(DANA_PRE_VIDEO, NULL, mydata,video_quality);
				if (lib_danavideo_cmd_setvideo_response(danavideoconn, trans_id, error_code, code_msg, set_video_fps)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO send response failed\n");
				}
			}
			break;
		/* set ap */
		case DANAVIDEOCMD_SETWIFIAP: {
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFIAP send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFIAP send response failed\n");
				}
			}
			break;
		/* set a new wifi connection*/
		case DANAVIDEOCMD_SETWIFI: {
			DANAVIDEOCMD_SETWIFI_ARG *setwifi_arg = (DANAVIDEOCMD_SETWIFI_ARG *)cmd_arg;
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFI send response succeed\n");
			} else {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFI send response failed\n");
			}

			struct sys_wifi_config *psys_wifi = ak_config_get_sys_wifi();
			struct sys_wifi_config *wifi_ifo = (struct sys_wifi_config *)calloc(
				1, sizeof(struct sys_wifi_config));
			memset(wifi_ifo, 0, sizeof(struct sys_wifi_config));

			strcpy(wifi_ifo->ssid, setwifi_arg->essid);
			strcpy(wifi_ifo->passwd, setwifi_arg->auth_key);
			bzero(wifi_ifo->mode, sizeof(wifi_ifo->mode));
			wifi_ifo->enc_type = setwifi_arg->enc_type;
			/*
			if((setwifi_arg->essid != NULL) && (0 != strcmp(psys_wifi->ssid,setwifi_arg->essid))){
				// set wifi
				if(dana_wifi_net_chk("eth0") == 0){
					if(1 == dana_wifi_conn_new_ssid(wifi_ifo->ssid,wifi_ifo->passwd)){
						ak_config_set_sys_wifi(wifi_ifo,1);
					}
					else{
						ak_print_normal("\nset wifi ap fail\n");
					}
				}
				else{
					ak_config_set_sys_wifi(wifi_ifo,1);
				}
			}
			*/
			if((setwifi_arg->essid != NULL) && (0 != strcmp(psys_wifi->ssid,setwifi_arg->essid))){
				/*
				try to connect a new wifi .
				if connect success,set the config file of new wifi.
				if connect failed then connet the old config.
				*/
				if( AK_TRUE == dana_wifi_conn_new_ssid(wifi_ifo->ssid,wifi_ifo->passwd)){ //尝试连接当前设置的wifi
					ak_config_set_sys_wifi(wifi_ifo,1);                         //写入当前的wifi配置到全局配置文件
				}
				else{
					ak_print_normal("\nset wifi ap fail\n");
				}
			}
			if( wifi_ifo ) {
				free(wifi_ifo);
			}
		}
		break;
		/* start audio */
		case DANAVIDEOCMD_STARTAUDIO: {
				ak_thread_mutex_lock(&dana_ctrl.lock) ;
				for (int i = 0; i < DANA_CONN_MAX; i++){
					if (dana_ctrl.mydata[i].danavideoconn == danavideoconn) {
						mydata = &(dana_ctrl.mydata[i]);

						/* close previous connect */
						if(1 == mydata->audio_conn_flg)
							dana_av_stop(DANA_PRE_AUDIO, mydata);

						dana_func_mode |= DANA_MIC_ENABLE;
						if((dana_func_mode & DANA_TALK_FULL_MODE) == 0)
							dana_func_mode &= ~DANA_SPEAK_ENABLE;
						mydata->audio_conn_flg = 1;
						dana_av_start_audio(DANA_PRE_AUDIO, dana_cmd_send_audio, mydata);
						break;
					}
				}
				ak_thread_mutex_unlock(&dana_ctrl.lock);
				/*refer to dana demo to modify*/
				uint32_t audio_codec = G711A; // danavidecodec_t
				uint32_t sample_rate = 8000;
				uint32_t sample_bit  = 16;
				uint32_t track = 1; // (1 mono; 2 stereo)
				if (lib_danavideo_cmd_startaudio_response(danavideoconn, trans_id, error_code, code_msg, \
					&audio_codec, &sample_rate, &sample_bit, &track)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTAUDIO send response succeeded\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTAUDIO send response failed\n");
				}
			}
			break;
		/* start talk */
		case DANAVIDEOCMD_STARTTALKBACK: {
				ak_print_notice_ex("receive start audio talk from dana...\n");
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK\n");

				uint32_t audio_codec = G711A;
				uint32_t sample_rate = 8000;
				uint32_t sample_bit  = 16;
				uint32_t track = 1; // (1 mono; 2 stereo)
				if (lib_danavideo_cmd_starttalkback_response(danavideoconn, trans_id, error_code, code_msg, \
					audio_codec	, &sample_rate, &sample_bit, &track)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK send response failed\n");
				}
				/* start talk */
				dana_func_mode |= DANA_SPEAK_ENABLE;
				dana_av_start_audio(DANA_PLAY_AUDIO, dana_cmd_get_talk_data, mydata);
			}
			break;
		/* start video */
		case DANAVIDEOCMD_STARTVIDEO: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMDSTARTVIDEO\n");
				DANAVIDEOCMD_STARTVIDEO_ARG *startvideo_arg = (DANAVIDEOCMD_STARTVIDEO_ARG *)cmd_arg;

				mydata->device_type = startvideo_arg->client_type;
				int video_quality = startvideo_arg->video_quality;
				ak_print_normal_ex("start video: device_type=%d, video_quality=%d\n",
					mydata->device_type, video_quality);

                /* start video */
                uint32_t start_video_fps = 0;
				request_set_i_frame();
				ak_thread_mutex_lock(&dana_ctrl.lock) ;
				for (int i = 0; i < DANA_CONN_MAX; i++){
					if (dana_ctrl.mydata[i].danavideoconn == danavideoconn) {
						mydata = &(dana_ctrl.mydata[i]);
						if(1 == mydata->video_conn_flg) {
							dana_av_stop(DANA_PRE_VIDEO,mydata);
							ak_print_normal_ex("start video: stop previous video first\n");
						}
						mydata->video_conn_flg = 1;
						start_video_fps = dana_av_start_video(DANA_PRE_VIDEO, dana_cmd_send_video, mydata, video_quality);

						/* set fps default. */
						if((start_video_fps > 30) || (start_video_fps < 10))
							start_video_fps = 10;
						mydata->fps = start_video_fps;
						ak_print_normal_ex("start_video_fps:%d\n", start_video_fps);
						break;
					}
				}
				ak_thread_mutex_unlock(&dana_ctrl.lock);
				/* may donot find  danavideoconn*/
				if((start_video_fps > 30) || (start_video_fps < 10))
					start_video_fps = 10;
				if (lib_danavideo_cmd_startvideo_response(danavideoconn,
							trans_id, error_code, code_msg, start_video_fps)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMDSTARTVIDEO send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMDSTARTVIDEO send response failed\n");
				}
				usleep(10*1000);
			}
			break;
		/* stop audio */
		case DANAVIDEOCMD_STOPAUDIO: {
				/* stop speek */
				int audio_conn = 0;
				ak_thread_mutex_lock(&dana_ctrl.lock) ;
				for (int i = 0; i < DANA_CONN_MAX; i++){
					if ( dana_ctrl.mydata[i].danavideoconn == danavideoconn  ) {
						mydata = &(dana_ctrl.mydata[i]);
						if (mydata->audio_conn_flg){
							dana_av_stop(DANA_PRE_AUDIO, mydata);
							mydata->audio_conn_flg = 0;
						}
					} else if (dana_ctrl.mydata[i].danavideoconn){
						mydata = &(dana_ctrl.mydata[i]);
						if (mydata->audio_conn_flg){
							audio_conn++;
						}
					}
				}
				ak_thread_mutex_unlock(&dana_ctrl.lock);
				if (0 == audio_conn)
				 	dana_func_mode &= ~DANA_MIC_ENABLE;
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPAUDIO send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPAUDIO send response failed\n");
				}
			}
			break;
		/* stop talk */
		case DANAVIDEOCMD_STOPTALKBACK: {
				ak_print_normal_ex("receive stop audio talk from dana...\n");
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPTALKBACK send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPTALKBACK send response failed\n");
				}

				dana_av_stop(DANA_PLAY_AUDIO, mydata);
				/* stop talk, clear talk bit and can send audio data to dana  */
				dana_func_mode &= ~DANA_SPEAK_ENABLE;
			}
			break;
		/* stop video */
		case DANAVIDEOCMD_STOPVIDEO: {
				ak_print_normal_ex("dana: stop video\n");
				ak_thread_mutex_lock(&dana_ctrl.lock) ;
				for (int i = 0; i < DANA_CONN_MAX; i++){
					if (dana_ctrl.mydata[i].danavideoconn == danavideoconn) {
						mydata = &(dana_ctrl.mydata[i]);
						if (mydata->video_conn_flg){
							dana_av_stop(DANA_PRE_VIDEO, mydata);
							mydata->video_conn_flg = 0;
						}
						break;
					}
				}
				ak_thread_mutex_unlock(&dana_ctrl.lock);

				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMDSTOPVIDEO send response succeed\n");
				} else {

					dbg("TEST danavideoconn_command_handler DANAVIDEOCMDSTOPVIDEO send response failed\n");
				}
			}
			break;
		/* get record list */
		case DANAVIDEOCMD_RECLIST: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST\n");
				ak_print_normal_ex("DANAVIDEOCMD_RECLIST\n");

				DANAVIDEOCMD_RECLIST_ARG *reclist_arg = (DANAVIDEOCMD_RECLIST_ARG *)cmd_arg;

				dbg("get_num: %ld,get_num=%ld,get_type=%ld\n",
					   	(unsigned long)reclist_arg->get_num, (unsigned long)reclist_arg->get_num,(unsigned long)reclist_arg->get_type);

				dbg("last_time: %ld\n",	(unsigned long)reclist_arg->last_time);

				int rec_lists_count =0;
				/* get record list */
				libdanavideo_reclist_recordinfo_t *rec_lists = dana_cmd_get_reclist(reclist_arg, &rec_lists_count);

				if (lib_danavideo_cmd_reclist_response(danavideoconn, trans_id,
						   	error_code, code_msg, rec_lists_count, rec_lists)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST send response succeed\n");
					dbg("[%s] DANAVIDEOCMD_RECLIST send succeed\n", __func__);
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST send response failed\n");
					ak_print_error_ex("DANAVIDEOCMD_RECLIST send failed\n");
				}
				if (rec_lists) {
					free(rec_lists);
				}
			}
			break;
		/* record replay start */
		case DANAVIDEOCMD_RECPLAY: {
				DANAVIDEOCMD_RECPLAY_ARG *recplay_arg = (DANAVIDEOCMD_RECPLAY_ARG *)cmd_arg;
				ak_print_normal_ex("## record replay start time_stamp: %lld ##\n", 
						recplay_arg->time_stamp);
				/** record replay **/
				dana_cmd_record_play(recplay_arg->time_stamp, mydata);

				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLAY send response succeed\n");
					ak_print_normal("[%s:%d] DANAVIDEOCMD_RECPLAY send response succeed\n", __func__, __LINE__);
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLAY send response failed\n");
					ak_print_normal("[%s:%d] DANAVIDEOCMD_RECPLAY send response failed\n", __func__, __LINE__);
				}
			}
			break;
		/* record replay stop */
		case DANAVIDEOCMD_RECSTOP: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP\n");
				ak_print_normal("[%s:%d] DANAVIDEOCMD_RECSTOP\n", __func__, __LINE__);
				 /* record replay stop */

				dana_cmd_stop_rec_play(mydata);

				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP send response succeed\n");
					ak_print_normal("[%s:%d] DANAVIDEOCMD_RECSTOP send response succeed\n", __func__, __LINE__);
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP send response failed\n");
					ak_print_normal("[%s:%d] DANAVIDEOCMD_RECSTOP send response failed\n", __func__, __LINE__);
				}
			}
			break;
		/* record replay pause or go on */
		case DANAVIDEOCMD_RECACTION: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION\n");
				ak_print_normal("[%s:%d] DANAVIDEOCMD_RECACTION\n", __func__, __LINE__);
				DANAVIDEOCMD_RECACTION_ARG *recaction_arg = (DANAVIDEOCMD_RECACTION_ARG *)cmd_arg;
				dbg("recaction_arg\n");
				dbg("ch_no: %ld\n", recaction_arg->ch_no);
				if (DANAVIDEO_REC_ACTION_PAUSE == recaction_arg->action) {
					dbg("action: DANAVIDEO_REC_ACTION_PAUSE\n");
					ak_print_normal("[%s:%d] DANAVIDEO_REC_ACTION_PAUSE\n", __func__, __LINE__);
					/* record replay pause */
					dana_cmd_pause_rec_play(mydata, 0);
				} else if (DANAVIDEO_REC_ACTION_PLAY == recaction_arg->action) {
					dbg("action: DANAVIDEO_REC_ACTION_PLAY\n");
					ak_print_normal("[%s:%d] DANAVIDEO_REC_ACTION_PLAY\n", __func__, __LINE__);
					/* record replay go on */
					dana_cmd_pause_rec_play(mydata, 1);
				} else {
					dbg("Unknown action: %ld\n", recaction_arg->action);
					ak_print_normal("[%s:%d] Unknown action: %u\n", __func__, __LINE__,
						   	recaction_arg->action);
				}
				dbg("\n");
				error_code = 0;
				code_msg = "";
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION send response failed\n");
				}
			}
			break;
		/* record replay set rates */
		case DANAVIDEOCMD_RECSETRATE: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE\n");
				ak_print_normal("[%s:%d] DANAVIDEOCMD_RECSETRATE\n", __func__, __LINE__);

				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE send response failed\n");
				}
			}
			break;
		/* record replay get plan */
		case DANAVIDEOCMD_RECPLANGET: {
				uint32_t rec_plans_count;
				/* record replay get plan */
				libdanavideo_recplanget_recplan_t *rec_plans = dana_cmd_g_rec_plan(&rec_plans_count);
				if (lib_danavideo_cmd_recplanget_response(danavideoconn, trans_id, error_code, code_msg, rec_plans_count, rec_plans)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET send response succeed\n");
				} else {

					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET send response failed\n");
				}
				free(rec_plans);
			}
			break;
		/* record replay set plan */
		case DANAVIDEOCMD_RECPLANSET: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET\n");

				DANAVIDEOCMD_RECPLANSET_ARG *recplanset_arg = (DANAVIDEOCMD_RECPLANSET_ARG *)cmd_arg;
				/* record replay set plan */
				ak_print_notice_ex("set record plan function on!\n");
				dana_cmd_s_rec_plan(recplanset_arg);

				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET send response succeed\n");
				} else {

					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET send response failed\n");
				}
			}
			break;
		/* extend method */
		case DANAVIDEOCMD_EXTENDMETHOD:
			break;
		/* set osd */
		case DANAVIDEOCMD_SETOSD: {
				 if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETOSD send response succeeded\n");
				} else {

					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETOSD send response failed\n");
				}
			}
			break;
		/* get osd message */
		case DANAVIDEOCMD_GETOSD: {
				libdanavideo_osdinfo_t osdinfo;
				/* get osd message from config file */
				if (lib_danavideo_cmd_getosd_response(danavideoconn, trans_id, error_code, code_msg, &osdinfo)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETOSD send response succeeded\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETOSD send response failed\n");
				}
			}
			break;
		/* get channal name */
		case DANAVIDEOCMD_SETCHANNAME: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET\n");

#if 0
				Pcamera_disp_setting camera_info;
				camera_info = anyka_get_camera_info();	/* get camera info */
				strncpy(camera_info->osd_name, setchanname_arg->chan_name, 49);
				anyka_set_camera_info(camera_info);	/* set camera info */
#endif
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)cmd_arg, trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANNAME send response succeeded\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANNAME send response failed\n");
				}
			}
			break;
		/* set channal name */
		case DANAVIDEOCMD_GETCHANNAME: {
				char *chan_name = "test";
				if (lib_danavideo_cmd_getchanname_response(danavideoconn, trans_id, error_code, code_msg, chan_name)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCHANNAME send response succeeded\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCHANNAME send response failed\n");
				}
			}
			break;
		/* cmd err */
		case DANAVIDEOCMD_RESOLVECMDFAILED: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED\n");
				// 根据不同的method,调用lib_danavideo_cmd_response
				error_code = 20145;
				code_msg = (char *)"danavideocmd_resolvecmdfailed";
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)cmd_arg, trans_id, error_code, code_msg)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED send response succeed\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED send response failed\n");
				}
			}
			break;
		/* get sd card status */
		/* The App get sd status that every 2 seconds */
		case DANAVIDEOCMD_GETSDCSTATUS: {
				dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSDCSTATUS\n");
				int format_status = record_ctrl_get_format_status();
				uint32_t status = DANA_SDC_STATUS_NO_SDC;
				uint32_t format_progress = 0;
				uint32_t sd_size = 0;
				uint32_t sd_free = 0;
				switch (format_status) {
					/* format  ok	*/
					case 0:
						sd_size = 1024 * 1024;
						sd_free = 1024 * 1024;
						status = DANA_SDC_STATUS_NOMAL;
						format_progress = 0;
						break;
					/* formating	*/
	 				case 1:
						format_progress += 10;
						status = DANA_SDC_STATUS_FORMAT;
						break;
					/* format  fail	*/
					case -1:
						format_progress = 0;
						status = DANA_SDC_STATUS_DAMAGE;
						break;
					default:
						break;
				}
				if (lib_danavideo_cmd_getsdcstatus_response(danavideoconn, trans_id, error_code, code_msg, status,
					&format_progress, &sd_size, &sd_free)) {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSDCSTATUS send response succeeded\n");
				} else {
					dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSDCSTATUS send response failed\n");
				}
			}
			break;
		default:
			ak_print_warning_ex(" cmd not excute. cmd=%d, arg: %p\n", cmd,arg);
			break;
	}

	return;
}

/* danale callback function */
dana_video_callback_funs_t dana_video_cb_funs = {
	.danavideoconn_created = danavideoconn_created,
	.danavideoconn_aborted = danavideoconn_aborted,
	.danavideoconn_command_handler = danavideoconn_command_handler,
};

/**
 * match_ssid_pswd - check ssid and passwd  as same as that of configure
 * @essid[IN]:  ssid string
 * @auth_key[IN]:  auth_key
 * return:  0, match; 1, not match
 */
static int match_ssid_pswd(const char * essid, const char * auth_key)
{
	struct sys_wifi_config * wifi_info = ak_config_get_sys_wifi();

	return (strcmp(wifi_info->ssid, essid) || strcmp(wifi_info->passwd, auth_key));
}

/**
 * save_ssid_to_tmp - save ssid to tmp file
 * @ssid[IN]:  ssid string
 * @ssid_len[IN]:  length of ssid
 * return:  0, success; -1, failed
 */
static int save_ssid_to_tmp(char *ssid, unsigned int ssid_len)
{
	int ret;
	char gbk_exist = 0, utf8_exist = 0;
	char gbk_path_name[32] = {0};
	char utf8_path_name[32] = {0};
	char gbk_ssid[32] = {0};

	sprintf(gbk_path_name, "%s/gbk_ssid", WIRE_LESS_TMP_DIR);
	sprintf(utf8_path_name, "%s/utf8_ssid", WIRE_LESS_TMP_DIR);

	ret = mkdir(WIRE_LESS_TMP_DIR, O_CREAT | 0666);
	if(ret < 0) {
		if(errno == EEXIST) {
			ak_print_normal_ex("the %s exist\n", WIRE_LESS_TMP_DIR);
			/** check file exist **/
			if(access(gbk_path_name, F_OK) == 0)
				gbk_exist = 1;
			if(access(utf8_path_name, F_OK) == 0)
				utf8_exist = 1;
			if(gbk_exist && utf8_exist) {
				ak_print_normal_ex("the wireless config has been saved,now do nothing\n");
				return 0;
			}
		} else {
			ak_print_error_ex(" make directory %s, %s\n", WIRE_LESS_TMP_DIR, strerror(errno));
			return -1;
		}
	}
	if (utf8_exist == 0) {
		FILE *filp_utf8 = fopen(utf8_path_name, "w+");
		if(!filp_utf8) {
			ak_print_error_ex(" open: %s, %s\n", utf8_path_name, strerror(errno));
			return -1;
		}

		ret = fwrite(ssid, 1, ssid_len + 1, filp_utf8);
		if(ret != ssid_len + 1) {
			ak_print_error_ex("fails write data\n");
			fclose(filp_utf8);
			return -1;
		}
		fclose(filp_utf8);
	}

	if (gbk_exist == 0) {
		/** utf-8 to gbk code change **/
#if 1
		ret = dana_others_u2g(ssid, ssid_len, gbk_ssid, 32);

		if(ret < 0) {
			ak_print_error_ex("faile to change code from utf8 to gbk\n");
			return -1;
		}
#else
 		ret = 0;
		memcpy(gbk_ssid, ssid, 32);
#endif
		ak_print_normal_ex("*** u2g changed[%d], %s\n", ret, gbk_ssid);

		FILE *filp_gbk = fopen(gbk_path_name, "w+");
		if (!filp_gbk) {
			ak_print_error_ex(" open: %s, %s\n", gbk_path_name, strerror(errno));
			return -1;
		}

		ret = fwrite(gbk_ssid, 1, strlen(gbk_ssid) + 1, filp_gbk);
		if (ret != strlen(gbk_ssid) + 1) {
			ak_print_error_ex(" fails write data\n");
			fclose(filp_gbk);
			return -1;
		}
		fclose(filp_gbk);
	}

	return 0;
}

/**
 * setwifiap_cb - set wifi configure by airlink
 * @ch_no[IN]:  channel no
 * @ip_type[IN]:  ip type
 * @ipaddr[IN]:   ip addr
 * @netmask[IN]:  net mask
 * @gateway[IN]:  gateway
 * @dns_name1[IN]:  1nst dns name
 * @dns_name2[IN]:  2nst dns name
 * @essid[IN]:  ssid name
 * @auth_key[IN]: auth key
 * @enc_type[IN]: encrypt type
 * return:  void
 */
static void setwifiap_cb(const uint32_t ch_no, const uint32_t ip_type,
	   	const char *ipaddr, const char *netmask, const char *gateway,
	   	const char *dns_name1, const char *dns_name2, const char *essid,
	   	const char *auth_key, const uint32_t enc_type)
{

	dbg("SETWIFIAP\n\tch_no: %ld\n\tip_type: %ld\n\tipaddr: %s\n"
			"\tnetmask: %s\ngateway: %s\n\tdns_name1: %s\n\tdns_name2: %s\n"
			"\tessid: %s\n\tauth_key: %s\n\tenc_type: %ld\n",
			ch_no, ip_type, ipaddr, netmask, gateway, dns_name1,
			dns_name2, essid, auth_key, enc_type);

	ak_print_normal_ex(" ssid:%s, authkey: %s, enctype: %d\n",
			   essid, auth_key, enc_type);

	if(match_ssid_pswd(essid, auth_key) == 0){
		dana_ctrl.airlink_called = true;
		ak_print_normal_ex("the ssid and password is same,"
				  " do nothing with it.\n");
		return;
	}

	if(dana_wifi_net_chk("eth0") == 1) return;

	struct sys_wifi_config * wifi_info = ak_config_get_sys_wifi();
	strncpy(wifi_info->passwd, auth_key, 31);
	ak_config_set_sys_wifi(wifi_info,0);

	if (save_ssid_to_tmp((char *)essid, strlen(essid))) {
		ak_print_error_ex("[airlink] fails to save ssid to tmp\n");
	}

	dana_ctrl.airlink_called = true;

}

/**
 * dana_airlink_thread - init and start airlink
 * @param[IN]:  null
 * return:  null
 */
static void* dana_airlink_thread(void *param)
{
	char *if_name = "wlan0";
	struct ifreq ifr;
	struct sockaddr_in *sai = NULL;
	struct stat st_buf;
	char *chk_dir = "/tmp/wireless";
	int sockfd;

	ak_thread_set_name("dana_airlink");

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&ifr,sizeof(struct ifreq));
	strcpy(ifr.ifr_name,if_name);
	dana_flag = NETWORK_UNDONE;
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	if (!danaairlink_set_danalepath(danale_path)) {
		dbg("testdanavideo danaairlink_set_danalepath failed\n");
		ak_print_error_ex("danaairlink_set_danalepath failed\n");
		close(sockfd);
		return NULL;
	}
	ak_print_normal_ex("danaairlink_set_danalepath OK\n");

	if (!danaairlink_init(DANAAIRLINK_CHIP_TYPE_RLT8188, if_name)) {
		dbg("testdanavideo danaairlink_init failed\n");
		ak_print_error_ex("danaairlink_init failed\n");
		close(sockfd);
		return NULL;
	}
	dbg("testdanavideo danaairlink_init succeeded\n");
	ak_print_normal_ex("danaairlink_init OK\n");

	if (!danaairlink_start_once()) {
		dbg("testdanavideo danaairlink_start_once failed\n");
		ak_print_error_ex("danaairlink_start_once failed\n");
		danaairlink_deinit();
		close(sockfd);
		return NULL;
	}
	dbg("testdanavideo danaairlink_start_once succeeded\n");
	ak_print_normal_ex("danaairlink_start_once OK\n\n");

	while (dana_ctrl.run_flag) {
		if (dana_ctrl.airlink_called) {
			/*wait udhcpc to configure wifi-net */
			dana_sleep(500);

			/*if ip is not configure , ioctl(sockfd, SIOCGIFADDR, &ifr) return fail*/
			if(ioctl(sockfd, SIOCGIFADDR, &ifr) >=0){
				sai = (struct sockaddr_in *) &ifr.ifr_addr;
				ak_print_normal_ex("%s ip: %s\n", if_name, inet_ntoa(sai->sin_addr));
				if(strlen(inet_ntoa(sai->sin_addr)) > 6){
					//ak_config_update_ssid();
					break;
				}
			}

			/* at first to check wifi ssid file exist? and then redo start airlink */
			if(stat(chk_dir, &st_buf) < 0){
				/*wait script  reload airlink wifi mod */
				//dana_sleep(1000);
				if ((ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) ||
						(!(ifr.ifr_flags & IFF_RUNNING))) {
					ak_print_normal_ex("get ifflags fail or not RUNNING.\n ");
					continue;
				}

				if(!danaairlink_start_once()) {
					ak_print_error("danaairlink_start_once failed\n");
					danaairlink_deinit();
					close(sockfd);
					return NULL;
				}
				dana_ctrl.airlink_called = false;
			}
		}
		else {
			dana_sleep(500);
		}
	}

	close(sockfd);
	danaairlink_stop();
	danaairlink_deinit();
	dana_ctrl.airlink_called = false;
	dana_flag = NETWORK_FINISH;

	ak_print_normal_ex("### dana_airlink_thread   thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

/**
 * airlink_set_wifi - create airlink thread
 * return:  void
 */
static void airlink_set_wifi(void)
{
	struct sys_wifi_config *p_wifi_info = ak_config_get_sys_wifi();

	if (0 != strlen(p_wifi_info->ssid)){
		ak_print_normal_ex("wifi ssid: %s\n", p_wifi_info->ssid);
		return;
	}
	dana_flag = NETWORK_UNDONE;
	ak_pthread_t airlink_thread_id;
	ak_thread_create(&airlink_thread_id, dana_airlink_thread,
		NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
}

/**
 * delay_init_thread - init and start danale lib function
 * @param[IN]:  null
 * return:  null
 */
static void* delay_init_thread(void *param)
{
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	ak_thread_set_name("dana_init");

	int net_interface = 0;

	while(dana_ctrl.run_flag) {
		if((dana_wifi_net_chk("wlan0") == AK_TRUE)
			&& !dana_ctrl.airlink_called) {
			net_interface = 1;	//wlan	
			break;
		}

		if(dana_wifi_net_chk("eth0") == AK_TRUE)
			break;
		dana_sleep(500);
	}

	uint32_t libdanavideo_startup_timeout_sec = 30;
	lib_danavideo_set_startup_timeout(libdanavideo_startup_timeout_sec);

#if 1
	//  在lib_danavideo_start之前调用 默认是 fixed 34102
	bool listen_port_fixed = true; // false
	//listen_port_fixed = true;
	uint16_t listen_port = 12349;
	lib_danavideo_set_listen_port(listen_port_fixed, listen_port);
#endif
	while(dana_ctrl.run_flag &&
			!lib_danavideo_init(danale_path, NULL, NULL, NULL, NULL, NULL,
				&dana_video_cb_funs)) {
		ak_print_normal_ex("lib_danavideo_init waiting...\n");
		dana_sleep(1000);
	}

	/* while dana lib start , do next things */
	while(dana_ctrl.run_flag && !lib_danavideo_start()) {
		ak_print_normal_ex("lib_danavideo_start waiting...\n");
		dana_sleep(1000);
	}
/*
#if 0
	//  在lib_danavideo_start之后调用 默认是 fixed 34102
	//bool listen_port_fixed = true; // false
	listen_port_fixed = true;
	//uint16_t listen_port = 12345;
	listen_port = 12345;
	lib_danavideo_set_listen_port(listen_port_fixed, listen_port);
	dbg("testdanavideo 4 lib_danavideo_get_listen_port: %\n", lib_danavideo_get_listen_port());
#endif
*/
	//lib_danavideo_cloud_set_cloud_mode_changed(anyka_cloud_changed_callback); // 注册云存储计划改变回调函数
	/*only net configure is ok , we do cloud init*/
	if (dana_ctrl.run_flag) {
		dana_others_cloud_init();
		ak_print_notice("\n************ dana app runs ****************\n");
	}

	/* 
	 * after connect dana ok, save wifi info to ini
	 * actualy, the scripts was save ssid and passwd to ini, BUT, 
	 * the ssid and passwd message record on the ini-handle was not
	 * update, so we update it now.
	 */
	if (net_interface == 1)
		ak_config_update_ssid();

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

/**
 * hb_is_ok_cb - callback function.
 *                    when heartbeat is ok , call this.
 * return:  void
 */
static void hb_is_ok_cb(void)
{
	static unsigned short online_status = 0;

	if(online_status != 1){
		ak_print_normal_ex("connect to danale successfully\n");
		if(ak_misc_ipc_first_run()) {
			struct audio_param file_param = {AK_AUDIO_TYPE_MP3, 8000, 16, 1};
			ak_misc_add_voice_tips(SYS_ONLINE, &file_param);
		}
		online_status = 1;
	}
}

/**
 * hb_is_not_ok_cb - callback function.
 *                    when heartbeat lost , call this.
 * return:  void
 */
static void hb_is_not_ok_cb(void)
{
	ak_print_normal_ex("connect to danale failed\n");
}

static void upgrade_rom_cb(const char *rom_path,
		const char *rom_md5, const uint32_t rom_size)
{
	dbg("NEED UPGRADE ROM rom_path: %s\trom_md5: %s\trom_size: %ld\n",
			rom_path, rom_md5, rom_size);
}

/**
 * autoset_cb - callback function.
 *                    when danale set time , call this .
 * @power_freq[IN]:  power frequency
 * @now_time[IN]:   calendar time
 * @time_zone[IN]:  time zone
 * @ntp_server1[IN]:  1nst ntp time server addr
 * @ntp_server2[IN]:  2nst ntp time server addr
 * return:  void
 */
static void autoset_cb(const uint32_t power_freq, const int64_t now_time,
	   	const char *time_zone, const char *ntp_server1, const char *ntp_server2)
{
	ak_print_normal_ex("power_freq: %d\n\tnow_time: %lld\n"
			"\ttime_zone: %s\n\tntp_server1: %s\n\tntp_server2: %s\n",
		   	power_freq, now_time, time_zone, ntp_server1, ntp_server2);

	dana_ctrl.dana_time.now_time = now_time;
	strncpy(dana_ctrl.dana_time.time_zone ,time_zone,64);
	strncpy(dana_ctrl.dana_time.ntp_server1, ntp_server1,256);
	strncpy(dana_ctrl.dana_time.ntp_server2, ntp_server2,256);
	dana_time_zone = convert_timezone(time_zone);
	dana_others_settime(now_time + dana_time_zone);
}

static unsigned int local_auth_cb(const char *user_name,
		const char *user_pass)
{
#if 0
	system_user_info *sys_user = anyka_get_system_user_info();
	if((strcmp(user_name, sys_user->user) == 0) &&
			(strcmp(user_pass, sys_user->secret) == 0))
		return 0;
	else {
		anyka_err("[%s:%d] user and secret is error: name=%s, pwd=%s\n",
				__func__, __LINE__, user_name, user_pass);
		return 1;
	}
#else
	return 0;
#endif
}

static void set_conf_create_or_updated_cb(const char *conf_absolute_pathname)
{
	dbg("CONF_create_or_updated  conf_absolute_pathname: %s\n", conf_absolute_pathname);
}

static unsigned int get_connected_wifi_quality_cb()
{
	int wifi_quality = 45;
	return wifi_quality;
}

static void set_productsetdeviceinfo_cb(const char *model, const char *sn, const char *hw_mac)
{
	dbg("danavideo_productsetdeviceinfo\n");
	dbg("model: %s\tsn: %s\thw_mac: %s\n", model, sn, hw_mac);
}

/**
 * set_device_feature - set device feature
 * return:  0, sucess;  -1, failed.
 */
static int set_device_feature(void)
{
	int ret = 0;

	dana_video_feature_t feature_list1[] = {
		DANAVIDEO_FEATURE_HAVE_PTZ_L_R_U_D,
		DANAVIDEO_FEATURE_HAVE_SD,
		DANAVIDEO_FEATURE_HAVE_MIC,
		DANAVIDEO_FEATURE_HAVE_SPEAKER,
		DANA_VIDEO_HAVE_CLOUD_STORAGE,
	};

	dana_video_feature_t feature_list2[] = {
		DANAVIDEO_FEATURE_HAVE_PTZ_L_R_U_D,
		DANAVIDEO_FEATURE_HAVE_SD,
		DANAVIDEO_FEATURE_HAVE_MIC,
		DANAVIDEO_FEATURE_HAVE_SPEAKER,
		DANA_VIDEO_HAVE_CLOUD_STORAGE,
		/*dana suggestion for aec 20160831 */
		4146,
	};

	int aec_enable = 1;

	if( 1 == aec_enable){
		dana_func_mode |= DANA_TALK_FULL_MODE;
		ret = lib_danavideo_util_setdevicefeature(
			sizeof(feature_list1)/sizeof(feature_list1[0]), feature_list1);
	} else {
		 /* feature_list <= 152 */
		ret = lib_danavideo_util_setdevicefeature(
			sizeof(feature_list2)/sizeof(feature_list2[0]), feature_list2);
	}
	ak_print_normal_ex("lib_danavideo_util_setdevicefeature ret=%d\n", ret);

	return ret;
}

/**
 * set_dana_video_callback - set dana lib callback funciton
 * return:  void.
 */
static void set_dana_video_callback(void)
{
	int maximum_size = DANA_VIDEO_BUFFER_SIZE;
	lib_danavideo_set_hb_is_ok(hb_is_ok_cb);
	lib_danavideo_set_hb_is_not_ok(hb_is_not_ok_cb);

	lib_danavideo_set_upgrade_rom(upgrade_rom_cb);

	lib_danavideo_set_autoset(autoset_cb);
	lib_danavideo_set_local_setwifiap(setwifiap_cb);

	lib_danavideo_set_local_auth(local_auth_cb);

	lib_danavideo_set_conf_created_or_updated(set_conf_create_or_updated_cb);

	lib_danavideo_set_get_connected_wifi_quality(get_connected_wifi_quality_cb);

	lib_danavideo_set_productsetdeviceinfo(set_productsetdeviceinfo_cb);
	lib_danavideo_set_maximum_buffering_data_size(maximum_size);
}

/**
 * update_send_flag - check used memory.
 *  				 if it overload, send flag set 1; otherwise, set 0.
 * return:  0, sucess;  -1, failed.
 */
static int update_send_flag(void)
{
	int fd = -1,ret = 0;
	char checkmem[100];
	char *p=NULL, *p2=NULL,*p3=NULL;
	int memsize;

	snprintf(checkmem,99,"/proc/%d/status",getpid());
	fd = open(checkmem,O_RDONLY);

	if(fd < 0){
		perror("open");
		return -1;
	}
	p=calloc(1,512);
	if( p != NULL){

		read(fd, p, 511);
		/* ak_print_normal_ex("p:%s\n",p); */
		p2 = strstr(p,"VmRSS");
		/* ak_print_normal_ex("p2:%p\n",p2); */
		if(p2 != NULL){
			p2 += 6;
			p3 = strstr(p2,"kB");
		}

		if( p3 != NULL) {
			p3[0] = '\0';

			memsize = atoi(p2);
			if((1 == send_data_flag) && ( memsize > MAX_ANYKA_IPC_MEM)){
				ak_print_normal_ex("memsize:%d\n",memsize);
				send_data_flag = 0;
			}
			else if ((0 == send_data_flag) && ( memsize < (MAX_ANYKA_IPC_MEM - 1024))){
				ak_print_normal_ex("memsize:%d\n",memsize);
				send_data_flag = 1;
			}
		}

	}else{
		ak_print_error_ex("calloc fail\n");
		ret = -1;
	}

	free(p);
	close(fd);
	return ret;
}

/**
 * dana_get_send_flag - get send_flag .
 		if memory is enought, send data to danale; otherwise not send.
 * return: send flag. 1, send ok; 0, not send
 */
int ak_dana_get_send_flag(void)
{
	static struct ak_timeval pre_time = {0};
	struct ak_timeval cur_time;

	ak_get_ostime(&cur_time);
	long diff_time = ak_diff_ms_time(&cur_time, &pre_time);
	/* 2000 ms to update flag*/
	if (diff_time > 2000) {
		update_send_flag();
		ak_get_ostime(&pre_time);
	}

	return send_data_flag;
}

/**
 * ak_dana_init: init dana app
 * @vi_handle[IN]: opened vi_handle
 * @ai_handle[IN]: opened ai_handle
 * @ao_handle[IN]: opened ao_handle
 * return: void
 */
void ak_dana_init(void *vi_handle, void *ai_handle, void *ao_handle)
{
	if (dana_ctrl.run_flag) {
		ak_print_notice("\t----- dana cloud has started -----\n");
		return;
	}

	ak_thread_mutex_init(&dana_ctrl.lock, NULL);

	ak_print_notice("\n\t----- start dana cloud -----\n");
	ak_print_normal("using libdanavideo_version: %s\n",
		lib_danavideo_linked_version_str(lib_danavideo_linked_version()));
	/* close dana lib debug	 */
	dbg_off();

	if (dana_av_init(vi_handle, ai_handle, ao_handle)) {
		ak_print_error_ex("\t----- start dana failed -----\n\n");
		return;
	}

	dana_ctrl.run_flag = AK_TRUE;
	dana_ctrl.vi_handle = vi_handle;
	set_dana_video_callback();
	set_device_feature();

	dana_others_upgrade_init(danale_path);

	dana_others_ptz_init();
	
	struct mt_config *mt = ak_config_get_mt();
	if (mt->mt_en)
		dana_others_mt_init(vi_handle);
	
	dana_osd_init(vi_handle);
	dana_others_alarm_init(vi_handle,ai_handle);
	ak_dvr_replay_init();
	airlink_set_wifi();

	int pcountNum =0;
	for (;;) {
		if((dana_flag == NETWORK_UNDONE) && (dana_wifi_net_chk("eth0") == AK_FALSE)) {
			dana_sleep(200);
			pcountNum++;
			if((pcountNum%25) ==0)
				ak_print_normal_ex("waiting network state ................ \n");
			continue;
		}
		else if((dana_wifi_net_chk("eth0") == AK_FALSE) && (dana_flag != NETWORK_UNDONE)) {
			ak_print_normal_ex("network state is ok  ................ \n");
		}
		break;
	}

	ak_pthread_t dana_thread_id;
	ak_thread_create(&dana_thread_id, delay_init_thread,
		NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	ak_print_notice_ex("--- start dana ok ---\n\n");
}

/* dana switch gop len */
void ak_dana_switch_gop(void)
{
	dana_av_switch_gop();
}

/**
 * ak_dana_exit: exit dana app
 * return: void
 */
void ak_dana_exit(void)
{
	if (!dana_ctrl.run_flag){
		ak_print_notice("\t----- dana not started -----\n");
		return;
	}

	ak_print_info_ex("entry ...\n");
	dana_ctrl.run_flag = AK_FALSE;
	ak_alarm_stop(SYS_DETECT_VOICE_ALARM);
	ak_alarm_stop(SYS_DETECT_MOVE_ALARM);
	ak_dvr_replay_exit();
	dana_osd_exit();
	dana_others_ptz_exit();
	dana_others_mt_exit();
	dana_av_exit();

	lib_danavideo_stop();
	ak_thread_mutex_destroy(&dana_ctrl.lock);
	ak_print_info_ex("leave ...\n");
}
