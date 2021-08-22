#include <stdlib.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "common.h"

#include "linux_list.h"
#include "dana_base.h"
#include "dana_task.h"
#include "dana_sock.h"
#include "dana_mem.h"

#include "dana_debug.h"
#include "danavideo_cmd.h"
#include "danavideo.h"

#include "dana_time.h"

#include "ak_dana_common.h"
#include "ak_uart.h"


typedef struct _danavideo_test_s{
   void *danavideoconn; 
   bool   is_video;
   bool   is_audio;
   char   ch_no;
   char   client_type;
   char   video_quality;
   struct list_head list;
}danavideotest_t;


// Application specific status/error codes
typedef enum{
    // Choosing this number to avoid overlap w/ host-driver's error codes 
    LAN_CONNECTION_FAILED = -0x7D0,
    SOCKET_CREATE_ERROR = LAN_CONNECTION_FAILED -1,
    BIND_ERROR = SOCKET_CREATE_ERROR - 1,
    LISTEN_ERROR = BIND_ERROR -1,
    SOCKET_OPT_ERROR = LISTEN_ERROR -1,
    CONNECT_ERROR = SOCKET_OPT_ERROR -1,
    ACCEPT_ERROR = CONNECT_ERROR - 1,
    SEND_ERROR = ACCEPT_ERROR -1,
    RECV_ERROR = SEND_ERROR -1,
    SOCKET_CLOSE_ERROR = RECV_ERROR -1,
    CLIENT_CONNECTION_FAILED = SOCKET_CLOSE_ERROR - 1,
    DEVICE_NOT_IN_STATION_MODE = CLIENT_CONNECTION_FAILED - 1,
    
    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

//tcp test
#define PORT_NUM			4801
#define TCP_PACKET_COUNT    100

extern int dana_service_start;


static danavideotest_t danavideotest;
dana_mutex_handler_t *danavideotest_lock = NULL;

dana_thread_handler_t *dana_video_stream_test = NULL;
dana_thread_handler_t *dana_video_talkback_test = NULL;
static bool run_th_stream = false;
static bool run_th_audio = false;

static uint32_t danavideoconn_created(void *danavideoconn) // dana_video_conn_t
{
    dbg("dana_video_conn_created\n");
#if 1
    danavideotest_t *tmp_node = (danavideotest_t *) dana_malloc(sizeof(danavideotest_t));
    if (NULL == tmp_node) {
        dbg("danavideoconn_created dana_malloc tmp_node failed\n"); 
        return -1;
    }

    dana_memset(tmp_node, 0, sizeof(danavideotest_t));
    tmp_node->danavideoconn = danavideoconn;
    tmp_node->is_video = false;
    tmp_node->is_audio = false;
	//º”Ω¯»•
	dbg("dana_video_add list success*********************\n");
    dana_mutex_lock(danavideotest_lock);
    list_add_tail(&(tmp_node->list), &(danavideotest.list));
    dana_mutex_unlock(danavideotest_lock);

#if 0
    if(lib_danavideo_set_userdata((dana_video_conn_t *)danavideoconn,(void *)tmp_node) != 0){
		return -1;
   }
   dbg("danavideoconn_created : %p\n", tmp_node->danavideoconn);	
#endif
#endif
    return 0;
}

static void danavideoconn_aborted(void *danavideoconn) //dana_video_conn_t
{
	//…æµÙ
   dbg("danavideoconn_aborted\n");
   struct list_head *pos = NULL;
   struct list_head *n = NULL;
   danavideotest_t *node = NULL;
   dana_mutex_lock(danavideotest_lock);
   list_for_each_safe(pos, n, &(danavideotest.list)) {
	   node = list_entry(pos, danavideotest_t, list); 
	   if (danavideoconn == node->danavideoconn) {
		   //…æ≥˝µÙ
		   list_del(&node->list);//…æ≥˝∏√Ω⁄µ„
		   dana_free(node);
		   //dbg("danavideoconn_aborted del 3 danavideoconn: %p\n", danavideoconn);
		   break;
	   }
   }
   dana_mutex_unlock(danavideotest_lock);

	
#if 0
      void * _tmp;
     _tmp = NULL;
	 if(lib_danavideo_get_userdata((dana_video_conn_t *)danavideoconn,&_tmp) != 0){
			return ;
	 }
	 dana_free(_tmp);		// Õ∑≈”√ªß π”√µƒƒ⁄¥Ê
#endif
}

volatile unsigned char ak_start_flg = 0;
static  void danavideoconn_command_handler(void *danavideoconn, dana_video_cmd_t cmd, uint32_t trans_id, void* cmd_arg, uint32_t cmd_arg_len) 
{
  
#if 1
    uint32_t i;
    uint32_t error_code;
    char *code_msg;
	CMD_INFO cmd_uart ;

	struct list_head *pos = NULL;
    danavideotest_t *node = NULL;
	#if 0
	if(lib_danavideo_get_userdata((dana_video_conn_t *)danavideoconn,(void **)&node)<0){
			   return ;
	}
    #endif 
    // ÂèëÈÄÅÂëΩ‰ª§ÂõûÂ∫îÁöÑÊé•Âè£ËøõË°å‰∏?
    switch (cmd) {
        case DANAVIDEOCMD_STARTVIDEO:
            dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTVIDEO\n");
            DANAVIDEOCMD_STARTVIDEO_ARG *startvideo_arg = (DANAVIDEOCMD_STARTVIDEO_ARG *)cmd_arg;
            dbg("startvideo_arg\n");
            dbg("ch_no: %d\n", startvideo_arg->ch_no);
            dbg("client_type: %d\n", startvideo_arg->client_type);
            dbg("video_quality: %d\n", startvideo_arg->video_quality);
            dbg("vstrm: %d\n", startvideo_arg->vstrm);
            dbg("\n");


			//////////////////////////////////////////////////////////////////
			///////////      ∑¢ÀÕ‘§¿¿√¸¡Ó                                           ////////////////
			//////////////////////////////////////////////////////////////////
			
			if(0 == wakeup_type)
			{
				wakeup_type = EVENT_VIDEO_PREVIEW_WAKEUP;
				power_ak(1);
			}
			if(1 == ak_start_flg)
			{
				cmd_uart.cmd_id = CMD_VIDEO_PREVIEW;
				send_cmd(&cmd_uart);//			
			}
			
			
			
            pos = NULL;
            node = NULL;
            dana_mutex_lock(danavideotest_lock);
            list_for_each(pos, &(danavideotest.list)) {
                node = list_entry(pos, danavideotest_t, list); 
                if (node->danavideoconn == danavideoconn) {
					dbg("set video true*********************\n");
                    node->is_video = true; 
                    break;
                }
				dbg("no video found*********************\n");
            }
			
            dana_mutex_unlock(danavideotest_lock);
            error_code = 0;
            code_msg = (char *)"";
            uint32_t start_video_fps = 30;
            Report("VIDEO 1\n");
            if (lib_danavideo_cmd_startvideo_response(danavideoconn, trans_id, error_code, code_msg, start_video_fps)) {
                Report("TEST danavideoconn_command_handler DANAVIDEOCMDSTARTVIDEO send response succeeded\n");
            } else {
                Report("TEST danavideoconn_command_handler DANAVIDEOCMDSTARTVIDEO send response failed\n");
            }
        
            break;
        case DANAVIDEOCMD_STOPVIDEO:
            dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPVIDEO\n");
            DANAVIDEOCMD_STOPVIDEO_ARG *stopvideo_arg = (DANAVIDEOCMD_STOPVIDEO_ARG *)cmd_arg;
            dbg("stopvideo_arg\n");
            dbg("ch_no: %d\n", stopvideo_arg->ch_no);
            dbg("\n"); 
            pos = NULL;
            node = NULL;
			
			//////////////////////////////////////////////////////////////////
			///////////      ∑¢ÀÕ‘§¿¿Ω· ¯√¸¡Ó                                      ////////////////
			//////////////////////////////////////////////////////////////////
			/*set dana video server flag for sleep*/
			dbg("set can sleep flag true\n");
			dana_service_start = 0;
			
			if(1 == ak_start_flg)
			{
				cmd_uart.cmd_id = CMD_VIDEO_PREVIEW_END;
				send_cmd(&cmd_uart);
			}
			
            dana_mutex_lock(danavideotest_lock);
            list_for_each(pos, &(danavideotest.list)) {
                node = list_entry(pos, danavideotest_t, list); 
                if (node->danavideoconn == danavideoconn) {
					dbg("set video False*********************\n");
                    node->is_video = false;  
                    break;
                }
            }
            dana_mutex_unlock(danavideotest_lock);
            error_code = 0;
            code_msg = (char *)"";
            dbg("STOPVIDEO\n");
            if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMDSTOPVIDEO send response succeeded\n");
            } else {

                dbg("TEST danavideoconn_command_handler DANAVIDEOCMDSTOPVIDEO send response failed\n");
            } 
            break;
         
        case DANAVIDEOCMD_SETVIDEO:
            dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO\n");
            DANAVIDEOCMD_SETVIDEO_ARG *setvideo_arg = (DANAVIDEOCMD_SETVIDEO_ARG *)cmd_arg;
            dbg("setvideo_arg\n");
            dbg("ch_no: %d\n", setvideo_arg->ch_no);
            dbg("video_quality: %d\n", setvideo_arg->video_quality);
            dbg("\n");
            error_code = 0;
            code_msg = (char *)"";
            uint32_t set_video_fps = 30;
            if (lib_danavideo_cmd_setvideo_response(danavideoconn, trans_id, error_code, code_msg, set_video_fps)) {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO send response succeeded\n");
            } else {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO send response failed\n");
            }
            break;
        case DANAVIDEOCMD_GETBASEINFO:
            dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETBASEINFO\n");
            DANAVIDEOCMD_GETBASEINFO_ARG *getbaseinfo_arg = (DANAVIDEOCMD_GETBASEINFO_ARG *)cmd_arg;
            dbg("getbaseinfo_arg\n");
            dbg("ch_no: %d\n", getbaseinfo_arg->ch_no);
            dbg("\n");
            error_code = 0;
            code_msg = (char *)"";
            char *dana_id = (char *)"hgy_1";
            char *api_ver = (char *)"hgy_2";
            char *sn      = (char *)"hgy_3";
            char *device_name = (char *)"hgy_4";
            char *rom_ver = (char *)"hgy_5";
            uint32_t device_type = 1;
            uint32_t ch_num = 25;
            uint64_t sdc_size = 1024;
            uint64_t sdc_free = 512;
            if (lib_danavideo_cmd_getbaseinfo_response(danavideoconn, trans_id, error_code, code_msg, dana_id, api_ver, sn, device_name, rom_ver, device_type, ch_num, sdc_size, sdc_free)) {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETBASEINFO send response succeeded\n");
            } else {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETBASEINFO send response failed\n");
            }
            break;
        case DANAVIDEOCMD_SETFLIP:
            dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETFLIP\n");
            DANAVIDEOCMD_SETFLIP_ARG *setflip_arg = (DANAVIDEOCMD_SETFLIP_ARG *)cmd_arg;
            dbg("setflip_arg\n");
            dbg("ch_no: %d\n", setflip_arg->ch_no);
            dbg("flip_type: %d\n", setflip_arg->flip_type);
            dbg("\n");
            error_code = 0;
            code_msg = (char *)"";
            if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETFLIP send response succeeded\n");
            } else {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETFLIP send response failed\n");
            }
            break;
        case DANAVIDEOCMD_GETFLIP:
            dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFLIP\n");
            DANAVIDEOCMD_GETFLIP_ARG *getflip_arg = (DANAVIDEOCMD_GETFLIP_ARG *)cmd_arg;
            dbg("getflip_arg_arg\n");
            dbg("ch_no: %d\n", getflip_arg->ch_no);
            dbg("\n");
            error_code = 0;
            code_msg = (char *)"";
            uint32_t flip_type = 1;
            if (lib_danavideo_cmd_getflip_response(danavideoconn, trans_id, error_code, code_msg, flip_type)) {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFLIP send response succeeded\n");
            } else {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFLIP send response failed\n");
            }
            break;
#if 0       
        case DANAVIDEOCMD_DEVDEF:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVDEF\n");
                DANAVIDEOCMD_DEVDEF_ARG *devdef_arg = (DANAVIDEOCMD_DEVDEF_ARG *)cmd_arg;
                dbg("devdef_arg\n");
                dbg("ch_no: %u\n", devdef_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVDEF send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVDEF send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_DEVREBOOT:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT\n");
                DANAVIDEOCMD_DEVREBOOT_ARG *devreboot_arg = (DANAVIDEOCMD_DEVREBOOT_ARG *)cmd_arg;
                dbg("devreboot_arg\n");
                dbg("ch_no: %u\n", devreboot_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT send response failed\n");
                } 
            }
            break; 
        case DANAVIDEOCMD_GETSCREEN:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSCREEN\n");
                DANAVIDEOCMD_GETSCREEN_ARG *getscreen_arg = (DANAVIDEOCMD_GETSCREEN_ARG *)cmd_arg;
                dbg("getcreen_arg\n");
                dbg("ch_no: %u\n", getscreen_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSCREEN send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSCREEN send response failed\n");
                }
                // Ëé∑Âèñ‰∏ÄÂâØÂõæÁâáË∞ÉÁî®lib_danavideoconn_send()ÊñπÊ≥ïÂèëÈÄ?
				

            }
            break;
        case DANAVIDEOCMD_GETALARM:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETALARM\n");
                DANAVIDEOCMD_GETALARM_ARG *getalarm_arg = (DANAVIDEOCMD_GETALARM_ARG *)cmd_arg;
                dbg("getalarm_arg\n");
                dbg("ch_no: %u\n", getalarm_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                uint32_t motion_detection = 1;
                uint32_t opensound_detection = 1;
                uint32_t openi2o_detection = 1;
                uint32_t smoke_detection = 1;
                uint32_t shadow_detection = 1;
                uint32_t other_detection = 1;
                if (lib_danavideo_cmd_getalarm_response(danavideoconn, trans_id, error_code, code_msg, motion_detection, opensound_detection, openi2o_detection, smoke_detection, shadow_detection, other_detection)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETALARM send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETALARM send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_GETCOLOR:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCOLOR\n");
                DANAVIDEOCMD_GETCOLOR_ARG *getcolor_arg = (DANAVIDEOCMD_GETCOLOR_ARG *)cmd_arg;
                dbg("getcolor_arg\n");
                dbg("ch_no: %u\n", getcolor_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                uint32_t brightness = 1;
                uint32_t contrast = 1;
                uint32_t saturation = 1;
                uint32_t hue = 1;
                if (lib_danavideo_cmd_getcolor_response(danavideoconn, trans_id, error_code, code_msg, brightness, contrast, saturation, hue)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCOLOR send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCOLOR send response failed\n");
                }
            }
            break; 

        case DANAVIDEOCMD_GETFUNLIST:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFUNLIST\n");
                DANAVIDEOCMD_GETFUNLIST_ARG *getfunlist_arg = (DANAVIDEOCMD_GETFUNLIST_ARG *)cmd_arg;
                dbg("getfunlist_arg\n");
                dbg("ch_no: %u\n", getfunlist_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                uint32_t methodes_count = 49;
                char *methods[] = { (char *)"DANAVIDEOCMD_DEVDEF", 
                    (char *)"DANAVIDEOCMD_DEVREBOOT", 
                    (char *)"DANAVIDEOCMD_GETSCREEN",
                    (char *)"DANAVIDEOCMD_GETALARM",
                    (char *)"DANAVIDEOCMD_GETBASEINFO",
                    (char *)"DANAVIDEOCMD_GETCOLOR",
                    (char *)"DANAVIDEOCMD_GETFLIP",
                    (char *)"DANAVIDEOCMD_GETFUNLIST",
                    (char *)"DANAVIDEOCMD_GETNETINFO",
                    (char *)"DANAVIDEOCMD_GETPOWERFREQ",
                    (char *)"DANAVIDEOCMD_GETTIME",
                    (char *)"DANAVIDEOCMD_GETWIFIAP",
                    (char *)"DANAVIDEOCMD_GETWIFI",
                    (char *)"DANAVIDEOCMD_PTZCTRL",
                    (char *)"DANAVIDEOCMD_SDCFORMAT",
                    (char *)"DANAVIDEOCMD_SETALARM",
                    (char *)"DANAVIDEOCMD_SETCHAN",
                    (char *)"DANAVIDEOCMD_SETCOLOR",
                    (char *)"DANAVIDEOCMD_SETFLIP",
                    (char *)"DANAVIDEOCMD_SETNETINFO",
                    (char *)"DANAVIDEOCMD_SETPOWERFREQ",
                    (char *)"DANAVIDEOCMD_SETTIME",
                    (char *)"DANAVIDEOCMD_SETVIDEO",
                    (char *)"DANAVIDEOCMD_SETWIFIAP",
                    (char *)"DANAVIDEOCMD_SETWIFI",
                    (char *)"DANAVIDEOCMD_STARTAUDIO",
                    (char *)"DANAVIDEOCMD_STARTTALKBACK",
                    (char *)"DANAVIDEOCMD_STARTVIDEO",
                    (char *)"DANAVIDEOCMD_STOPAUDIO",
                    (char *)"DANAVIDEOCMD_STOPTALKBACK",
                    (char *)"DANAVIDEOCMD_STOPVIDEO",
                    (char *)"DANAVIDEOCMD_RECLIST",
                    (char *)"DANAVIDEOCMD_RECPLAY",
                    (char *)"DANAVIDEOCMD_RECSTOP",
                    (char *)"DANAVIDEOCMD_RECACTION",
                    (char *)"DANAVIDEOCMD_RECSETRATE",
                    (char *)"DANAVIDEOCMD_RECPLANGET",
                    (char *)"DANAVIDEOCMD_RECPLANSET",
                    (char *)"DANAVIDEOCMD_EXTENDMETHOD",
                    (char *)"DANAVIDEOCMD_SETOSD",
                    (char *)"DANAVIDEOCMD_GETOSD",
                    (char *)"DANAVIDEOCMD_SETCHANNAME",
                    (char *)"DANAVIDEOCMD_GETCHANNAME",
                    (char *)"DANAVIDEOCMD_CALLPSP",
                    (char *)"DANAVIDEOCMD_GETPSP",
                    (char *)"DANAVIDEOCMD_SETPSP",
                    (char *)"DANAVIDEOCMD_SETPSPDEF",
                    (char *)"DANAVIDEOCMD_GETLAYOUT",
                    (char *)"DANAVIDEOCMD_SETCHANADV" };
                if (lib_danavideo_cmd_getfunlist_response(danavideoconn, trans_id, error_code, code_msg, methodes_count, (const char**)methods)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFUNLIST send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFUNLIST send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_GETNETINFO:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETNETINFO\n");
                DANAVIDEOCMD_GETNETINFO_ARG *getnetinfo_arg = (DANAVIDEOCMD_GETNETINFO_ARG *)cmd_arg;
                dbg("getnetinfo_arg\n");
                dbg("ch_no: %u\n", getnetinfo_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                uint32_t ip_type = 1; // 1 "fixed"; 2 "dhcp"
                char *ipaddr = (char *)"192.168.0.189";
                char *netmask = (char *)"255.255.255.255";
                char *gateway = (char *)"192.168.0.19";
                uint32_t dns_type = 1;
                char *dns_name1 = (char *)"8.8.8.8";
                char *dns_name2 = (char *)"114.114.114.114";
                uint32_t http_port = 21045;
                if (lib_danavideo_cmd_getnetinfo_response(danavideoconn, trans_id, error_code, code_msg, ip_type, ipaddr, netmask, gateway, dns_type, dns_name1, dns_name2, http_port)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETNETINFO send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETNETINFO send response failed\n");
                }
            }
            break;
        case DANAVIDEOCMD_GETPOWERFREQ:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPOWERFREQ\n");
                DANAVIDEOCMD_GETPOWERFREQ_ARG *getpowerfreq_arg = (DANAVIDEOCMD_GETPOWERFREQ_ARG *)cmd_arg;
                dbg("getpowerfreq_arg\n");
                dbg("ch_no: %u\n", getpowerfreq_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                uint32_t freq = DANAVIDEO_POWERFREQ_50HZ;
                if (lib_danavideo_cmd_getpowerfreq_response(danavideoconn, trans_id, error_code, code_msg, freq)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPOWERFREQ send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPOWERFREQ send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_GETTIME:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETTIME\n");
                DANAVIDEOCMD_GETTIME_ARG *gettime_arg = (DANAVIDEOCMD_GETTIME_ARG *)cmd_arg;
                dbg("gettime_arg_arg\n");
                dbg("ch_no: %u\n", gettime_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                int64_t now_time = 1;
                char *time_zone = (char *)"shanghai";
                char *ntp_server_1 = "NTP_SRV_1";
                char *ntp_server_2 = "NTP_SRV_2";
                if (lib_danavideo_cmd_gettime_response(danavideoconn, trans_id, error_code, code_msg, now_time, time_zone, ntp_server_1, ntp_server_2)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETTIME send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETTIME send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_GETWIFIAP:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFIAP\n");
                DANAVIDEOCMD_GETWIFIAP_ARG *getwifiap_arg = (DANAVIDEOCMD_GETWIFIAP_ARG *)cmd_arg;
                dbg("getwifiap_arg\n");
                dbg("ch_no: %u\n", getwifiap_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                uint32_t wifi_device = 1;
                uint32_t wifi_list_count = 3;
                libdanavideo_wifiinfo_t wifi_list[] = {{"danasz", 1, 5}, {"danasz5G", 1, 6}, {"ÊùéÂ∞πabc123", 1, 6}};
                if (lib_danavideo_cmd_getwifiap_response(danavideoconn, trans_id, error_code, code_msg, wifi_device, wifi_list_count, wifi_list)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFIAP send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFIAP send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_GETWIFI:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFI\n");
                DANAVIDEOCMD_GETWIFI_ARG *getwifi_arg = (DANAVIDEOCMD_GETWIFI_ARG *)cmd_arg;
                dbg("getwifi_arg\n");
                dbg("ch_no: %u\n", getwifi_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                char *essid = (char *)"danasz";
                char *auth_key = (char *)"wps";
                uint32_t enc_type = 1;
                if (lib_danavideo_cmd_getwifi_response(danavideoconn, trans_id, error_code, code_msg, essid, auth_key, enc_type)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFI send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFI send response failed\n");
                }
            }
            break;
        case DANAVIDEOCMD_PTZCTRL:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL\n");
                DANAVIDEOCMD_PTZCTRL_ARG *ptzctrl_arg = (DANAVIDEOCMD_PTZCTRL_ARG *)cmd_arg;
                dbg("ptzctrl_arg\n");
                dbg("ch_no: %u\n", ptzctrl_arg->ch_no);
                dbg("code: %u\n", ptzctrl_arg->code);
                dbg("para1: %u\n", ptzctrl_arg->para1);
                dbg("para2: %u\n", ptzctrl_arg->para2);
                dbg("\n");
                switch (ptzctrl_arg->code) {
                    case DANAVIDEO_PTZ_CTRL_MOVE_UP:
                        dbg("DANAVIDEO_PTZ_CTRL_MOVE_UP\n");
                        break;
                    case DANAVIDEO_PTZ_CTRL_MOVE_DOWN:
                        dbg("DANAVIDEO_PTZ_CTRL_MOVE_DOWN\n");
                        break;
                    case DANAVIDEO_PTZ_CTRL_MOVE_LEFT:
                        dbg("DANAVIDEO_PTZ_CTRL_MOVE_LEFT\n");
                        break;
                    case DANAVIDEO_PTZ_CTRL_MOVE_RIGHT:
                        dbg("DANAVIDEO_PTZ_CTRL_MOVE_RIGHT\n");
                        break;
                        // ...
                }
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SDCFORMAT:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SDCFORMAT\n");
                DANAVIDEOCMD_SDCFORMAT_ARG *sdcformat_arg = (DANAVIDEOCMD_SDCFORMAT_ARG *)cmd_arg; 
                dbg("sdcformat_arg\n");
                dbg("ch_no: %u\n", sdcformat_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SDCFORMAT send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SDCFORMAT send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SETALARM:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM\n");
                DANAVIDEOCMD_SETALARM_ARG *setalarm_arg = (DANAVIDEOCMD_SETALARM_ARG *)cmd_arg;
                dbg("setalarm_arg\n");
                dbg("ch_no: %u\n", setalarm_arg->ch_no);
                dbg("motion_detection: %u\n", setalarm_arg->motion_detection);
                dbg("opensound_detection: %u\n", setalarm_arg->opensound_detection);
                dbg("openi2o_detection: %u\n", setalarm_arg->openi2o_detection);
                dbg("smoke_detection: %u\n", setalarm_arg->smoke_detection);
                dbg("shadow_detection: %u\n", setalarm_arg->shadow_detection);
                dbg("other_detection: %u\n", setalarm_arg->other_detection);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SETCHAN:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN\n");
                DANAVIDEOCMD_SETCHAN_ARG *setchan_arg = (DANAVIDEOCMD_SETCHAN_ARG *)cmd_arg; 
                dbg("setchan_arg\n");
                dbg("ch_no: %u\n", setchan_arg->ch_no);
                dbg("chans_count: %zd\n", setchan_arg->chans_count);
                for (i=0; i<setchan_arg->chans_count; i++) {
                    dbg("chans[%u]: %u\n", i, setchan_arg->chans[i]);
                }
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SETCOLOR:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR\n"); 
                DANAVIDEOCMD_SETCOLOR_ARG *setcolor_arg = (DANAVIDEOCMD_SETCOLOR_ARG *)cmd_arg;
                dbg("setcolor_arg\n");
                dbg("ch_no: %u\n", setcolor_arg->ch_no);
                dbg("video_rate: %u\n", setcolor_arg->video_rate);
                dbg("brightness: %u\n", setcolor_arg->brightness);
                dbg("contrast: %u\n", setcolor_arg->contrast);
                dbg("saturation: %u\n", setcolor_arg->saturation);
                dbg("hue: %u\n", setcolor_arg->hue);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR send response failed\n");
                }
            }
            break;
        case DANAVIDEOCMD_SETNETINFO:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO\n"); 
                DANAVIDEOCMD_SETNETINFO_ARG *setnetinfo_arg = (DANAVIDEOCMD_SETNETINFO_ARG *)cmd_arg;
                dbg("setnetinfo_arg\n");
                dbg("ch_no: %u\n", setnetinfo_arg->ch_no);
                dbg("ip_type: %u\n", setnetinfo_arg->ip_type);
                dbg("ipaddr: %s\n", setnetinfo_arg->ipaddr);
                dbg("netmask: %s\n", setnetinfo_arg->netmask);
                dbg("gateway: %s\n", setnetinfo_arg->gateway);
                dbg("dns_type: %u\n", setnetinfo_arg->dns_type);
                dbg("dns_name1: %s\n", setnetinfo_arg->dns_name1);
                dbg("dns_name2: %s\n", setnetinfo_arg->dns_name2);
                dbg("http_port: %u\n", setnetinfo_arg->http_port);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SETPOWERFREQ:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ\n"); 
                DANAVIDEOCMD_SETPOWERFREQ_ARG *setpowerfreq_arg = (DANAVIDEOCMD_SETPOWERFREQ_ARG *)cmd_arg;
                dbg("setpowerfreq_arg\n");
                dbg("ch_no: %u\n", setpowerfreq_arg->ch_no);
                if (DANAVIDEO_POWERFREQ_50HZ == setpowerfreq_arg->freq) {
                    dbg("freq: DANAVIDEO_POWERFREQ_50HZ\n");
                } else if (DANAVIDEO_POWERFREQ_60HZ == setpowerfreq_arg->freq) {
                    dbg("freq: DANAVIDEO_POWERFREQ_60HZ\n");
                } else {
                    dbg("UnKnown freq: %u\n", setpowerfreq_arg->freq);
                }
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SETTIME:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETTIME\n"); 
                DANAVIDEOCMD_SETTIME_ARG *settime_arg = (DANAVIDEOCMD_SETTIME_ARG *)cmd_arg;
                dbg("settime_arg\n");
                dbg("ch_no: %u\n", settime_arg->ch_no);
                dbg("now_time: %lu\n", settime_arg->now_time);
                dbg("time_zone: %s\n", settime_arg->time_zone);
                if (settime_arg->has_ntp_server1) {
                    dbg("ntp_server_1: %s\n", settime_arg->ntp_server1);
                }
                if (settime_arg->has_ntp_server2) {
                    dbg("ntp_server_2: %s\n", settime_arg->ntp_server2);
                }
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETTIME send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETTIME send response failed\n");
                }
            }
            break;
        case DANAVIDEOCMD_SETWIFIAP:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFIAP\n"); 
                DANAVIDEOCMD_SETWIFIAP_ARG *setwifiap_arg = (DANAVIDEOCMD_SETWIFIAP_ARG *)cmd_arg;
                dbg("setwifiap_arg\n");
                dbg("ch_no: %u\n", setwifiap_arg->ch_no);
                dbg("ip_type: %u\n", setwifiap_arg->ip_type);
                dbg("ipaddr: %s\n", setwifiap_arg->ipaddr);
                dbg("netmask: %s\n", setwifiap_arg->netmask);
                dbg("gateway: %s\n", setwifiap_arg->gateway);
                dbg("dns_name1: %s\n", setwifiap_arg->dns_name1);
                dbg("dns_name2: %s\n", setwifiap_arg->dns_name2);
                dbg("essid: %s\n", setwifiap_arg->essid);
                dbg("auth_key: %s\n", setwifiap_arg->auth_key);
                dbg("enc_type: %u\n", setwifiap_arg->enc_type);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFIAP send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFIAP send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SETWIFI:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFI\n");
                DANAVIDEOCMD_SETWIFI_ARG *setwifi_arg = (DANAVIDEOCMD_SETWIFI_ARG *)cmd_arg; 
                dbg("setwifi_arg\n");
                dbg("ch_no: %u\n", setwifi_arg->ch_no);
                dbg("essid: %s\n", setwifi_arg->essid);
                dbg("auth_key: %s\n", setwifi_arg->auth_key);
                dbg("enc_type: %u\n", setwifi_arg->enc_type);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFI send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFI send response failed\n");
                }
            }
            break; 
     
        case DANAVIDEOCMD_STARTAUDIO:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTAUDIO\n"); 
                DANAVIDEOCMD_STARTAUDIO_ARG *startaudio_arg = (DANAVIDEOCMD_STARTAUDIO_ARG *)cmd_arg;
                dbg("startaudio_arg\n");
                dbg("ch_no: %u\n", startaudio_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";

                uint32_t audio_codec = G711A; // danavidecodec_t
                uint32_t sample_rate = 8000; // Âçï‰ΩçHz
                uint32_t sample_bit  = 16; // Âçï‰Ωçbit
                uint32_t track = 2; // (1 mono; 2 stereo)
                
                dana_mutex_lock(danavideotest_lock);
                list_for_each(pos, &(danavideotest.list)) {
                    node = list_entry(pos, danavideotest_t, list); 
                    if (node->danavideoconn == danavideoconn) {
                        node->is_audio = true; 
                        break;
                    }
                }
                dana_mutex_unlock(danavideotest_lock);
                
                if (lib_danavideo_cmd_startaudio_response(danavideoconn, trans_id, error_code, code_msg, &audio_codec, &sample_rate, &sample_bit, &track)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTAUDIO send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTAUDIO send response failed\n");
                }

                //TODO ÈúÄË¶ÅÂèëÈÄÅÈü≥È¢?
            }
            break;
            
        case DANAVIDEOCMD_STOPAUDIO:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPAUDIO\n");
                DANAVIDEOCMD_STOPAUDIO_ARG *stopaudio_arg = (DANAVIDEOCMD_STOPAUDIO_ARG *)cmd_arg;
                dbg("stopaudio_arg\n");
                dbg("ch_no: %u\n", stopaudio_arg->ch_no);
                dbg("\n");
                code_msg = (char *)"";
                error_code = 0;
                //TODO  ÂÖ≥Èó≠Èü≥È¢ëÁîü‰∫ßËÄÖÁ∫øÁ®?
                dbg("TEST danavideoconn_command_handler stop th_audio_media\n");
                
                dana_mutex_lock(danavideotest_lock);
                list_for_each(pos, &(danavideotest.list)) {
                    node = list_entry(pos, danavideotest_t, list); 
                    if (node->danavideoconn == danavideoconn) {
                        node->is_audio = false; 
                        break;
                    }
                }
                dana_mutex_unlock(danavideotest_lock);
                
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPAUDIO send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPAUDIO send response failed\n");
                }
            }
            break; 

        case DANAVIDEOCMD_STARTTALKBACK:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK\n"); 
                DANAVIDEOCMD_STARTTALKBACK_ARG *starttalkback_arg = (DANAVIDEOCMD_STARTTALKBACK_ARG *)cmd_arg;
                dbg("starttalkback_arg\n");
                dbg("ch_no: %u\n", starttalkback_arg->ch_no);
                dbg("\n");
                
                code_msg = (char *)"";
                uint32_t audio_codec = G711A;
                
                //TODO  ÈúÄË¶ÅÂèëÈÄÅÈü≥È¢?
                
                dana_mutex_lock(danavideotest_lock);
                list_for_each(pos, &(danavideotest.list)) {
                    node = list_entry(pos, danavideotest_t, list); 
                    if (node->danavideoconn == danavideoconn) {
                        node->is_audio = true; 
                        break;
                    }
                }
                dana_mutex_unlock(danavideotest_lock);
                
                error_code = 0;
                if (lib_danavideo_cmd_starttalkback_response(danavideoconn, trans_id, error_code, code_msg, audio_codec)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK send response failed\n");
                }
            }
            break; 
            
        case DANAVIDEOCMD_STOPTALKBACK:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPTALKBACK\n");
                DANAVIDEOCMD_STOPTALKBACK_ARG *stoptalkback_arg = (DANAVIDEOCMD_STOPTALKBACK_ARG *)cmd_arg; 
                dbg("stoptalkback_arg\n");
                dbg("ch_no: %u\n", stoptalkback_arg->ch_no);
                dbg("\n");
                
                //TODO ÂÖ≥Èó≠Èü≥È¢ëËØªÂèñÁ∫øÁ®ã 
                dbg("TEST danavideoconn_command_handler stop th_talkback\n");
                
                dana_mutex_lock(danavideotest_lock);
                list_for_each(pos, &(danavideotest.list)) {
                    node = list_entry(pos, danavideotest_t, list); 
                    if (node->danavideoconn == danavideoconn) {
                        node->is_audio = false; 
                        break;
                    }
                }
                dana_mutex_unlock(danavideotest_lock);
                
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPTALKBACK send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPTALKBACK send response failed\n");
                }
            }
            break;
#endif            
#if 0
        case DANAVIDEOCMD_RECLIST:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST\n");
                DANAVIDEOCMD_RECLIST_ARG *reclist_arg = (DANAVIDEOCMD_RECLIST_ARG *)cmd_arg;
                dbg("reclist_arg\n");
                dbg("ch_no: %u\n", reclist_arg->ch_no);
                if (DANAVIDEO_REC_GET_TYPE_NEXT == reclist_arg->get_type) {
                    dbg("get_type: DANAVIDEO_REC_GET_TYPE_NEXT\n");
                } else if (DANAVIDEO_REC_GET_TYPE_PREV == reclist_arg->get_type) {
                    dbg("get_type: DANAVIDEO_REC_GET_TYPE_PREV\n");
                } else {
                    dbg("Unknown get_type: %u\n", reclist_arg->get_type);
                }
                dbg("get_num: %u\n", reclist_arg->get_num);
                dbg("last_time: %ld\n", reclist_arg->last_time);
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                uint32_t rec_lists_count = 2;
                libdanavideo_reclist_recordinfo_t rec_lists[] = {{123, 123, DANAVIDEO_REC_TYPE_NORMAL}, {456, 456, DANAVIDEO_REC_TYPE_ALARM}};
                if (lib_danavideo_cmd_reclist_response(danavideoconn, trans_id, error_code, code_msg, rec_lists_count, rec_lists)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST send response succeeded\n");
                } else {

                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST send response failed\n");
                } 
            }
            break;
        case DANAVIDEOCMD_RECPLAY:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLAY\n");
                DANAVIDEOCMD_RECPLAY_ARG *recplay_arg = (DANAVIDEOCMD_RECPLAY_ARG *)cmd_arg;
                dbg("recplay_arg\n");
                dbg("ch_no: %u\n", recplay_arg->ch_no);
                dbg("time_stamp: %ld\n", recplay_arg->time_stamp);
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLAY send response succeeded\n");
                } else {

                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLAY send response failed\n");
                } 
            }
            break;
        case DANAVIDEOCMD_RECSTOP:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP\n");
                DANAVIDEOCMD_RECSTOP_ARG *recstop_arg = (DANAVIDEOCMD_RECSTOP_ARG *)cmd_arg;
                dbg("recstop_arg\n");
                dbg("ch_no: %u\n", recstop_arg->ch_no);
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP send response succeeded\n");
                } else {

                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_RECACTION:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION\n");
                DANAVIDEOCMD_RECACTION_ARG *recaction_arg = (DANAVIDEOCMD_RECACTION_ARG *)cmd_arg;
                dbg("recaction_arg\n");
                dbg("ch_no: %u\n", recaction_arg->ch_no);
                if (DANAVIDEO_REC_ACTION_PAUSE == recaction_arg->action) {
                    dbg("action: DANAVIDEO_REC_ACTION_PAUSE\n");
                } else if (DANAVIDEO_REC_ACTION_PLAY == recaction_arg->action) {
                    dbg("action: DANAVIDEO_REC_ACTION_PLAY\n");
                } else {
                    dbg("Unknown action: %u\n", recaction_arg->action);
                }
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION send response succeeded\n");
                } else {

                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION send response failed\n");
                } 
            }
            break;
        case DANAVIDEOCMD_RECSETRATE:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE\n");
                DANAVIDEOCMD_RECSETRATE_ARG *recsetrate_arg = (DANAVIDEOCMD_RECSETRATE_ARG *)cmd_arg;
                dbg("recsetrate_arg\n");
                dbg("ch_no: %u\n", recsetrate_arg->ch_no);
                if (DANAVIDEO_REC_RATE_HALF == recsetrate_arg->rec_rate) {
                    dbg("rec_rate: DANAVIDEO_REC_RATE_HALF\n");
                } else if (DANAVIDEO_REC_RATE_NORMAL == recsetrate_arg->rec_rate) {
                    dbg("rec_rate: DANAVIDEO_REC_RATE_NORMAL\n");
                } else if (DANAVIDEO_REC_RATE_DOUBLE == recsetrate_arg->rec_rate) {
                    dbg("rec_rate: DANAVIDEO_REC_RATE_DOUBLE\n");
                } else if (DANAVIDEO_REC_RATE_FOUR == recsetrate_arg->rec_rate) {
                    dbg("rec_rate: DANAVIDEO_REC_RATE_FOUR\n");
                } else {
                    dbg("Unknown rec_rate: %u\n", recsetrate_arg->rec_rate);
                }
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE send response succeeded\n");
                } else {

                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE send response failed\n");
                } 
            }
            break;
        case DANAVIDEOCMD_RECPLANGET:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET\n");
                DANAVIDEOCMD_RECPLANGET_ARG *recplanget_arg = (DANAVIDEOCMD_RECPLANGET_ARG *)cmd_arg;
                dbg("recplanget_arg\n");
                dbg("ch_no: %u\n", recplanget_arg->ch_no);
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                uint32_t rec_plans_count = 2;
                libdanavideo_recplanget_recplan_t rec_plans[] = {{0, 2, {DANAVIDEO_REC_WEEK_MON, DANAVIDEO_REC_WEEK_SAT}, "12:23:34", "15:56:01", DANAVIDEO_REC_PLAN_OPEN}, {1, 3, {DANAVIDEO_REC_WEEK_MON, DANAVIDEO_REC_WEEK_SAT, DANAVIDEO_REC_WEEK_SUN}, "22:23:24", "23:24:25", DANAVIDEO_REC_PLAN_CLOSE}};
                if (lib_danavideo_cmd_recplanget_response(danavideoconn, trans_id, error_code, code_msg, rec_plans_count, rec_plans)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET send response succeeded\n");
                } else {

                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_RECPLANSET:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET\n");
                DANAVIDEOCMD_RECPLANSET_ARG *recplanset_arg = (DANAVIDEOCMD_RECPLANSET_ARG *)cmd_arg;
                dbg("recplanset_arg\n");
                dbg("ch_no: %u\n", recplanset_arg->ch_no);
                dbg("record_no: %u\n", recplanset_arg->record_no);
                size_t i;
                for (i=0; i<recplanset_arg->week_count; i++) {
                    if (DANAVIDEO_REC_WEEK_MON == recplanset_arg->week[i]) {
                        dbg("week: DANAVIDEO_REC_WEEK_MON\n");
                    } else if (DANAVIDEO_REC_WEEK_TUE == recplanset_arg->week[i]) {
                        dbg("week: DANAVIDEO_REC_WEEK_TUE\n");
                    } else if (DANAVIDEO_REC_WEEK_WED == recplanset_arg->week[i]) {
                        dbg("week: DANAVIDEO_REC_WEEK_WED\n");
                    } else if (DANAVIDEO_REC_WEEK_THU == recplanset_arg->week[i]) {
                        dbg("week: DANAVIDEO_REC_WEEK_THU\n");
                    } else if (DANAVIDEO_REC_WEEK_FRI == recplanset_arg->week[i]) {
                        dbg("week: DANAVIDEO_REC_WEEK_FRI\n");
                    } else if (DANAVIDEO_REC_WEEK_SAT == recplanset_arg->week[i]) {
                        dbg("week: DANAVIDEO_REC_WEEK_SAT\n");
                    } else if (DANAVIDEO_REC_WEEK_SUN == recplanset_arg->week[i]) {
                        dbg("week: DANAVIDEO_REC_WEEK_SUN\n");
                    } else {
                        dbg("Unknown week: %u\n", recplanset_arg->week[i]);
                    } 
                } 
                dbg("start_time: %s\n", recplanset_arg->start_time);
                dbg("end_time: %s\n", recplanset_arg->end_time);
                dbg("status: %u\n", recplanset_arg->status);
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET send response succeeded\n");
                } else {

                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET send response failed\n");
                } 
            }
            break;
        case DANAVIDEOCMD_EXTENDMETHOD:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_EXTENDMETHOD\n");
                DANAVIDEOCMD_EXTENDMETHOD_ARG *extendmethod_arg = (DANAVIDEOCMD_EXTENDMETHOD_ARG *)cmd_arg;
                dbg("extendmethod_arg\n");
                dbg("ch_no: %u\n", extendmethod_arg->ch_no);
                dbg("extend_data_size: %zd\n", extendmethod_arg->extend_data.size);
                // extend_data_bytes access via extendmethod_arg->extend_data.bytes
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_EXTENDMETHOD send response succeeded\n");
                } else {

                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_EXTENDMETHOD send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_SETOSD:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETOSD\n");
                DANAVIDEOCMD_SETOSD_ARG *setosd_arg = (DANAVIDEOCMD_SETOSD_ARG *)cmd_arg;
                dbg("ch_no: %u\n", setosd_arg->ch_no);
                dbg("osd_info:\n");
                if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.chan_name_show) {
                    dbg("chan_name_show OPEN\n");
                    if (setosd_arg->osd.has_show_name_x) {
                        dbg("show_name_x: %u\n", setosd_arg->osd.show_name_x);
                    }
                    if (setosd_arg->osd.has_show_name_y) {
                        dbg("show_name_y: %u\n", setosd_arg->osd.show_name_y);
                    } 
                } else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.chan_name_show) {
                    dbg("chan_name_show CLOSE\n");
                } else {
                    dbg("chan_name_show unknown type[%u]\n", setosd_arg->osd.chan_name_show);
                }
                if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.datetime_show) {
                    dbg("datetime_show OPEN\n");
                    if (setosd_arg->osd.has_show_datetime_x) {
                        dbg("show_datetime_x: %u\n", setosd_arg->osd.show_datetime_x);
                    }
                    if (setosd_arg->osd.has_show_datetime_y) {
                        dbg("show_datetime_y: %u\n", setosd_arg->osd.show_datetime_y);
                    }
                    if (setosd_arg->osd.has_show_format) {
                        dbg("show_format:\n");
                        switch (setosd_arg->osd.show_format) {
                            case DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD:
                                dbg("DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD\n");
                                break;
                            case DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY:
                                dbg("DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY\n");
                                break;
                            case DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD_CH:
                                dbg("DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD_CH\n");
                                break;
                            case DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY_CH:
                                dbg("DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY_CH\n");
                                break;
                            case DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY:
                                dbg("DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY\n");
                                break;
                            case DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY_CH:
                                dbg("DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY_CH\n");
                                break;
                            default:
                                dbg("DANAVIDEO_OSD_DATE_FORMAT_XXXX\n");
                                break;
                        }
                    }
                    if (setosd_arg->osd.has_hour_format) {
                        dbg("hour_format:\n");
                        switch (setosd_arg->osd.hour_format) {
                            case DANAVIDEO_OSD_TIME_24_HOUR:
                                dbg("DANAVIDEO_OSD_TIME_24_HOUR\n");
                                break;
                            case DANAVIDEO_OSD_TIME_12_HOUR:
                                dbg("DANAVIDEO_OSD_TIME_12_HOUR\n");
                                break;
                            default:
                                dbg("DANAVIDEO_OSD_TIME_XXXX\n");
                                break;
                        }
                    }
                    if (setosd_arg->osd.has_show_week) {
                        dbg("show_week:\n");
                        switch (setosd_arg->osd.show_week) {
                            case DANAVIDEO_OSD_SHOW_CLOSE:
                                dbg("DANAVIDEO_OSD_SHOW_CLOSE\n");
                                break;
                            case DANAVIDEO_OSD_SHOW_OPEN:
                                dbg("DANAVIDEO_OSD_SHOW_OPEN\n");
                                break;
                            default:
                                dbg("DANAVIDEO_OSD_SHOW_XXXX\n");
                                break;
                        }
                    }
                    if (setosd_arg->osd.has_datetime_attr) {
                        dbg("datetime_attr:\n");
                        switch (setosd_arg->osd.datetime_attr) {
                            case DANAVIDEO_OSD_DATETIME_TRANSPARENT:
                                dbg("DANAVIDEO_OSD_DATETIME_TRANSPARENT\n");
                                break;
                            case DANAVIDEO_OSD_DATETIME_DISPLAY:
                                dbg("DANAVIDEO_OSD_DATETIME_DISPLAY\n");
                                break;
                            default:
                                dbg("DANAVIDEO_OSD_DATETIME_XXXX\n");
                                break;
                        }
                    }
                } else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.datetime_show) {
                    dbg("datetime_show CLOSE\n");
                } else {
                    dbg("datetime_show unknown type[%u]\n", setosd_arg->osd.datetime_show);
                }
                if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.custom1_show) {
                    dbg("custom1_show OPEN\n");
                    if (setosd_arg->osd.has_show_custom1_str) {
                        dbg("show_custom1_str: %s\n", setosd_arg->osd.show_custom1_str);
                    }
                    if (setosd_arg->osd.has_show_custom1_x) {
                        dbg("show_custom1_x: %u\n", setosd_arg->osd.show_custom1_x);
                    }
                    if (setosd_arg->osd.has_show_custom1_y) {
                        dbg("show_custom1_y: %u\n", setosd_arg->osd.show_custom1_y);
                    }
                } else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.custom1_show) {
                    dbg("custom1_show CLOSE\n");
                } else {
                    dbg("custom1_show unknown type[%u]\n", setosd_arg->osd.custom1_show);
                }
                if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.custom2_show) {
                    dbg("custom2_show OPEN\n");
                    if (setosd_arg->osd.has_show_custom2_str) {
                        dbg("show_custom2_str: %s\n", setosd_arg->osd.show_custom2_str);
                    }
                    if (setosd_arg->osd.has_show_custom2_x) {
                        dbg("show_custom2_x: %u\n", setosd_arg->osd.show_custom2_x);
                    }
                    if (setosd_arg->osd.has_show_custom2_y) {
                        dbg("show_custom2_y: %u\n", setosd_arg->osd.show_custom2_y);
                    }
                } else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.custom2_show) {
                    dbg("custom2_show CLOSE\n");
                } else {
                    dbg("custom2_show unknown type[%u]\n", setosd_arg->osd.custom2_show);
                }
                //
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETOSD send response succeeded\n");
                } else {

                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETOSD send response failed\n");
                }
            }
            break;
        case DANAVIDEOCMD_GETOSD:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETOSD\n");
                DANAVIDEOCMD_GETOSD_ARG *getosd_arg = (DANAVIDEOCMD_GETOSD_ARG *)cmd_arg;
                dbg("ch_no: %u\n", getosd_arg->ch_no);
                dbg("\n");
                error_code = 0;
                code_msg = (char *)"";
                libdanavideo_osdinfo_t osdinfo;

                osdinfo.chan_name_show = DANAVIDEO_OSD_SHOW_CLOSE;
                osdinfo.show_name_x = 1;
                osdinfo.show_name_y = 2;

                osdinfo.datetime_show = DANAVIDEO_OSD_SHOW_CLOSE;
                osdinfo.show_datetime_x = 3;
                osdinfo.show_datetime_y = 4;
                osdinfo.show_format = DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD_CH;
                osdinfo.hour_format = DANAVIDEO_OSD_TIME_24_HOUR;
                osdinfo.show_week = DANAVIDEO_OSD_SHOW_OPEN;
                osdinfo.datetime_attr = DANAVIDEO_OSD_DATETIME_DISPLAY;

                osdinfo.custom1_show = DANAVIDEO_OSD_SHOW_OPEN;
                strncpy(osdinfo.show_custom1_str, "show_custom1_str", sizeof(osdinfo.show_custom1_str) -1);
                osdinfo.show_custom1_x = 5;
                osdinfo.show_custom1_y = 6;

                osdinfo.custom2_show = DANAVIDEO_OSD_SHOW_CLOSE;
                strncpy(osdinfo.show_custom2_str, "show_custom2_str", sizeof(osdinfo.show_custom2_str) -1);
                osdinfo.show_custom2_x = 7;
                osdinfo.show_custom2_y = 8;

                if (lib_danavideo_cmd_getosd_response(danavideoconn, trans_id, error_code, code_msg, &osdinfo)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETOSD send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETOSD send response failed\n");
                }
            }
            break;
        case DANAVIDEOCMD_SETCHANNAME:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANNAME\n");
                DANAVIDEOCMD_SETCHANNAME_ARG *setchanname_arg = (DANAVIDEOCMD_SETCHANNAME_ARG *)cmd_arg;
                dbg("setchanname_arg\n");
                dbg("ch_no: %u\n", setchanname_arg->ch_no);
                dbg("chan_name: %s\n", setchanname_arg->chan_name);
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANNAME send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANNAME send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_GETCHANNAME:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCHANNAME\n");
                DANAVIDEOCMD_GETCHANNAME_ARG *getchanname_arg = (DANAVIDEOCMD_GETCHANNAME_ARG *)cmd_arg;
                dbg("getchanname_arg\n");
                dbg("ch_no: %u\n", getchanname_arg->ch_no);
                dbg("\n"); 
                error_code = 0;
                code_msg = "";

                char *chan_name = "Di3";
                if (lib_danavideo_cmd_getchanname_response(danavideoconn, trans_id, error_code, code_msg, chan_name)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCHANNAME send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCHANNAME send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_CALLPSP:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_CALLPSP\n");
                DANAVIDEOCMD_CALLPSP_ARG *callpsp_arg = (DANAVIDEOCMD_CALLPSP_ARG *)cmd_arg;
                dbg("callpsp_arg\n");
                dbg("ch_no: %u\n", callpsp_arg->ch_no);
                dbg("psp_id: %u\n", callpsp_arg->psp_id);
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_CALLPSP send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_CALLPSP send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_GETPSP:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPSP\n");
                DANAVIDEOCMD_GETPSP_ARG *getpsp_arg = (DANAVIDEOCMD_GETPSP_ARG *)cmd_arg;
                dbg("getpsp_arg\n");
                dbg("ch_no: %u\n", getpsp_arg->ch_no);
                dbg("page: %u\n", getpsp_arg->page);
                dbg("page_size: %u\n", getpsp_arg->page_size);
                dbg("\n"); 
                error_code = 0;
                code_msg = "";

                uint32_t total = 20;
                uint32_t psp_count = 2;
                libdanavideo_pspinfo_t psp[] = {{1, "Psp_1", true, true}, {2, "Psp_2", false, true}};
                if (lib_danavideo_cmd_getpsp_response(danavideoconn, trans_id, error_code, code_msg, total, psp_count, psp)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPSP send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPSP send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_SETPSP:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSP\n");
                DANAVIDEOCMD_SETPSP_ARG *setpsp_arg = (DANAVIDEOCMD_SETPSP_ARG *)cmd_arg;
                dbg("setpsp_arg\n");
                dbg("ch_no: %u\n", setpsp_arg->ch_no);
                dbg("psp_id: %u\n", setpsp_arg->psp.psp_id);
                dbg("psp_name: %s\n", setpsp_arg->psp.psp_name);
                dbg("psp_default: %s\n", setpsp_arg->psp.psp_default?"true":"false");
                dbg("is_set: %s\n", setpsp_arg->psp.is_set?"true":"false");
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSP send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSP send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_SETPSPDEF:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSPDEF\n");
                DANAVIDEOCMD_SETPSPDEF_ARG *setpspdef_arg = (DANAVIDEOCMD_SETPSPDEF_ARG *)cmd_arg;
                dbg("setpspdef_arg\n");
                dbg("ch_no: %u\n", setpspdef_arg->ch_no);
                dbg("psp_id: %u\n", setpspdef_arg->psp_id);
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSPDEF send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSPDEF send response failed\n");
                }
            } 
            break; 
        case DANAVIDEOCMD_GETLAYOUT:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETLAYOUT\n");
                DANAVIDEOCMD_GETLAYOUT_ARG *getlayout_arg = (DANAVIDEOCMD_GETLAYOUT_ARG *)cmd_arg;
                dbg("getlayout_arg\n");
                dbg("ch_no: %u\n", getlayout_arg->ch_no);
                dbg("\n"); 
                error_code = 0;
                code_msg = "";

                uint32_t matrix_x = 4; 
                uint32_t matrix_y = 4;
                size_t chans_count = 16;
                uint32_t chans[] = {1, 1, 2, 3, 1, 1, 4, 5, 6, 7, 8, 9, 10, 11, 0, 0};
                uint32_t layout_change = 0;
                uint32_t chan_pos_change = 0;
                size_t use_chs_count = 16;
                uint32_t use_chs[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

                if (lib_danavideo_cmd_getlayout_response(danavideoconn, trans_id, error_code, code_msg, matrix_x, matrix_y, chans_count, chans, layout_change, chan_pos_change, use_chs_count, use_chs)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETLAYOUT send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_GETLAYOUT send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_SETCHANADV:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANADV\n");
                DANAVIDEOCMD_SETCHANADV_ARG *setchanadv_arg = (DANAVIDEOCMD_SETCHANADV_ARG *)cmd_arg;
                dbg("setchanadv_arg\n");
                dbg("ch_no: %u\n", setchanadv_arg->ch_no);
                dbg("matrix_x: %u\n", setchanadv_arg->matrix_x);
                dbg("matrix_y: %u\n", setchanadv_arg->matrix_y);
                size_t i;
                for (i=0; i<setchanadv_arg->chans_count; i++) {
                    dbg("chans[%zd]: %u\n", i, setchanadv_arg->chans[i]);
                }
                dbg("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANADV send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANADV send response failed\n");
                }
            } 
            break;

        case DANAVIDEOCMD_RESOLVECMDFAILED:
            {
                dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED\n");
                // Ê†πÊçÆ‰∏çÂêåÁöÑmethod,Ë∞ÉÁî®lib_danavideo_cmd_response
                error_code = 20145;
                code_msg = (char *)"danavideocmd_resolvecmdfailed";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)cmd_arg, trans_id, error_code, code_msg)) {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED send response succeeded\n");
                } else {
                    dbg("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED send response failed\n");
                }
            }
            break;
#endif      
        default:
            {
                dbg("TEST danavideoconn_command_handler UnKnown CMD: %u\n", cmd);
            }
            break;
    }
#endif
    return;
}

dana_video_callback_funs_t danavideocallbackfuns = {
    .danavideoconn_created = danavideoconn_created,
    .danavideoconn_aborted = danavideoconn_aborted,
    .danavideoconn_command_handler = danavideoconn_command_handler,
};

uint32_t danavideo_local_auth_callback (const char *user_name, const char *user_pass)
{
    dbg("danavideo_local_auth_callback user_name: %s\t user_pass: %s\n", user_name, user_pass);
    return 0;
}

void danapush_service_test()
{
#if 1
    int64_t  cur_time = 0;
    int64_t  start_time = 0;
    uint32_t chan_no = 1;
    uint32_t alarm_level = DANA_VIDEO_PUSHMSG_ALARM_LEVEL_2;
    uint32_t msg_type = DANA_VIDEO_PUSHMSG_MSG_TYPE_DOOR_BELL;
    char     *msg_title = "TEST danavideo_alarm_cloud";
    char     *msg_body  = "lib_danavideo_util_pushmsg danavideo_alarm_cloud";
    cur_time = dana_update_time()/1000000;
    uint32_t att_flag = 0;
    uint32_t record_flag = 1;
    start_time = dana_update_time()/1000000;//Áß?
    uint32_t time_len = 60;
    uint32_t save_site = 5;
    char *save_path = "pshsmsg_v2_save_path";
    char *sensor_id = "pushmsg_v2_sensor_id";
#if 1
    while (1) 
	{
		osi_SyncObjWait(&door_ring_sem,OSI_WAIT_FOREVER); // ◊Ë»˚

	    if (lib_danavideo_util_pushmsg_v2(chan_no, alarm_level, msg_type, msg_title, msg_body, 
	                                      cur_time, att_flag, NULL, NULL, record_flag, 
	                                      start_time, time_len, save_site, save_path, sensor_id)) {
	        Report("\x1b[32mtestdanavideo TEST lib_danavideo_util_pushmsg_v2 success\x1b[0m\r\n");
	    } else {
	        Report("\x1b[34mtestdanavideo TEST lib_danavideo_util_pushmsg_v2 failed\x1b[0m\r\n");
	    }
		
		dana_usleep(500*1000);//

    }
#else   //lib_danavideo_util_pushmsg,Â∞ÜpushÊ∂àÊÅØÊîæÂà∞Ê∂àÊÅØÈòüÂàóÔºåÂú®Â∫ì‰∏≠ÁöÑÁ∫øÁ®ã‰∏≠Ê£ÄÊµãÂíåÊé®ÈÄ?
    while(1){

        if(lib_danavideo_util_pushmsg(ch_no, alarm_level, msg_type, msg_title, msg_body, 
                                cur_time, att_flag, NULL, NULL, record_flag, 
                                start_time, time_len, save_site, save_path)){
            Report("\x1b[32mtestdanavideo TEST lib_danavideo_util_pushmsg success\x1b[0m\r\n");
        }else{
            Report("\x1b[34mtestdanavideo TEST lib_danavideo_util_pushmsg failed\x1b[0m\r\n");
        }
    }
#endif
#endif
}

void ak2dana_stream(char *video_buf)
{
	struct list_head *pos = NULL;
    struct list_head *n = NULL;
    danavideotest_t *node = NULL;
	unsigned int data_len=0;
	unsigned int iFrame=0,timestamp=0;

		
	//get data_len iFrame timestamp
	data_len   = *(unsigned int*)video_buf;
	iFrame     = *(unsigned int*)(video_buf+4);
	timestamp  = *(unsigned int*)(video_buf+8);

	dana_mutex_lock(danavideotest_lock);
	list_for_each_safe(pos, n, &(danavideotest.list)) {
    	   node = list_entry(pos, danavideotest_t, list); 
           if(node->is_video){
               if(!lib_danavideoconn_send(node->danavideoconn,
                                      video_stream,
                                      H264,node->ch_no,
                                      iFrame,
                                      timestamp,
                                      video_buf+12,
                                      data_len,
                                      50*1000)){
                    Report("send fail\r\n");                       
                }else{
                    Report("send sucess\r\n");
                }  
           }
       }
       dana_mutex_unlock(danavideotest_lock);
       //dana_usleep(80*1000);

}

#if 0
static void th_stream(void *usPort)
{
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    danavideotest_t *node = NULL;
    //static volatile unsigned long video_frame_test = 0;
	unsigned char meg =0;
#ifdef UDP_SERVER_RCV_SUPPORT
    SlSockAddrIn_t  sAddr;
    SlSockAddrIn_t  sLocalAddr;
    int             iCounter;
    int             iAddrSize;
    int             iSockID;
    int             iStatus;
    short           sTestBufLen;

	char *g_cBsdBuf = NULL;
#endif
	unsigned int data_len=0;
	unsigned int iFrame=0,timestamp=0;
	
  
#if 0
	g_cBsdBuf = (char *)mem_Malloc(BUF_SIZE);
	if(g_cBsdBuf == NULL)
	{
       Report("th_stream Malloc GGGGGGGGGGGGGGGGGGGGGGGGGGGG falid]\r\n");
       return ;
    }
	Report("th_stream static buffer:%dk\r\n",(BUF_SIZE/1024));
    // filling the buffer
    mem_set(g_cBsdBuf,'a',BUF_SIZE);
#endif
#ifdef UDP_SERVER_RCV_SUPPORT
    sTestBufLen  = VIDEO_BUF_SIZE;
    //filling the UDP server socket address
    sLocalAddr.sin_family = SL_AF_INET;
    sLocalAddr.sin_port = sl_Htons((unsigned short)usPort);
    sLocalAddr.sin_addr.s_addr = 0;

    iAddrSize = sizeof(SlSockAddrIn_t);

    // creating a UDP socket
    iSockID = sl_Socket(SL_AF_INET,SL_SOCK_DGRAM, 0);
    if( iSockID < 0 )
    {
        // error
        Report("SOCKET_CREATE_ERROR\r\n");
		return;
    }

    // binding the UDP socket to the UDP server address
    iStatus = sl_Bind(iSockID, (SlSockAddr_t *)&sLocalAddr, iAddrSize);
    if( iStatus < 0 )
    {
    	// error
        sl_Close(iSockID);
        Report("BIND_ERROR\r\n");
		return;
    }
#endif
	
    Report("TH stream\r\n");  
    while(1){
		
	  osi_MsgQRead(&spi2videomsg, &meg, OSI_WAIT_FOREVER);
	  dana_mutex_lock(danavideotest_lock);
	  //video_frame_test++;
	  //Report("@=%d\r\n",video_frame_test);
#ifdef UDP_SERVER_RCV_SUPPORT	   
	   // no listen or accept is required as UDP is connectionless protocol
	   iStatus = sl_RecvFrom(iSockID, video_buf, sTestBufLen, 0,( SlSockAddr_t *)&sAddr, (SlSocklen_t*)&iAddrSize );
       if( iStatus < 0 )
       {
       		// error
            sl_Close(iSockID);
        	Report("RECV_ERROR\r\n");
			return;	
       }
	   	Report("\nudp Rec len:->%d\n\r",iStatus);
#endif

	
		//get data_len iFrame timestamp
		#if 0
		memcpy(&data_len,video_buf, sizeof(unsigned int));
		memcpy(&iFrame, video_buf+4, sizeof(unsigned int));
		memcpy(&timestamp, video_buf+8, sizeof(unsigned int));
		#else
		data_len   = *(unsigned int*)video_buf;
		iFrame     = *(unsigned int*)(video_buf+4);
		timestamp  = *(unsigned int*)(video_buf+8);
		#endif
		//timestamp = 0;
        Report("\ndata_len:%d iFrame:%d timestamp:%d\n\r",data_len,iFrame,timestamp);
		//timestamp = 0;
        //Report("= timestamp:%d\n\r",timestamp);

		list_for_each_safe(pos, n, &(danavideotest.list)) {
           node = list_entry(pos, danavideotest_t, list); 
           if(node->is_video){
               if(!lib_danavideoconn_send(node->danavideoconn,
                                      video_stream,
                                      H264,node->ch_no,
                                      iFrame,
                                      timestamp,
                                      video_buf+12,
                                      data_len,
                                      50*1000)){
                    Report("th_stream: Send failed.datalen:%d\r\n",data_len);                       
                }else{
                    Report("th_stream: Send Successed.\r\n");
                }  
				
           }
       }
       dana_mutex_unlock(danavideotest_lock);
       //dana_usleep(80*1000);
    }

#ifdef UDP_SERVER_RCV_SUPPORT
    //closing the socket after receiving 
    sl_Close(iSockID);
#endif
	//mem_Free(g_cBsdBuf);
	
}
#else
#if 0
static void th_stream(void *args)
{
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    danavideotest_t *node = NULL;
 
    Report("TH th_stream\r\n");  
    while(1){
        dana_mutex_lock(danavideotest_lock);
        list_for_each_safe(pos, n, &(danavideotest.list)) {
            node = list_entry(pos, danavideotest_t, list); 
            if(node->is_video){
                if(!lib_danavideoconn_send(node->danavideoconn,
                                       video_stream,
                                       H264,
                                       node->ch_no,
                                       1,
                                       10,
                                       (char *)__videodata,
                                       1024,
                                       50*1000)){
                    Report("th_stream: Send failed.\r\n");
                    break;                 
                }else{
                    Report("th_stream: Send Successed.\r\n");
                }
            }
            dana_mutex_unlock(danavideotest_lock);
            dana_usleep(500*1000);
        }
    }
}
#endif
#endif

static void th_audio(void *args)
{
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    danavideotest_t *node = NULL;
    char * audiodata;
    audiodata = (char *)mem_Malloc(1024);
    if(audiodata == NULL){
      Report("th_audio Malloc GGGGGGGGGGGGGGGGGGGGGGGGGGGG falid]\r\n");
      return ;
    }
    mem_set(audiodata,'a',1024);
    Report("TH th_audio\r\n");  
    while(1){
        dana_mutex_lock(danavideotest_lock);
        list_for_each_safe(pos, n, &(danavideotest.list)){
          node = list_entry(pos, danavideotest_t, list); 
            if(node->is_audio){
                if(!lib_danavideoconn_send(node->danavideoconn,
                                       audio_stream,
                                       G711A,
                                       node->ch_no,
                                       1, 
                                       dana_update_time()/1000000,
                                       (char *)audiodata,
                                       1024,
                                       50*1000)){
                    Report("th_audio: Send failed.\r\n");
                }else{
                    Report("th_audio: Send Successed.\r\n");
                }       
            }
        }
        dana_mutex_unlock(danavideotest_lock);
	dana_usleep(500*1000);
    }
}
extern signed char g_ssid_name[40];
extern signed char g_security_key[40];
volatile bool danaairlink_set_wifi_cb_called = false;
void danavideo_setwifiap(const uint32_t ch_no, const uint32_t ip_type, const char *ipaddr, const char *netmask, const char *gateway, const char *dns_name1, const char *dns_name2, const char *essid, const char *auth_key, const uint32_t enc_type)
{
    Report("SETWIFIAP\n\tch_no: %u\n\tip_type: %u\n\tipaddr: %s\n\tnetmask: %s\n\tgateway: %s\n\tdns_name1: %s\n\tdns_name2: %s\n\tessid: %s\n\tauth_key: %s\n\tenc_type: %u\r\n", ch_no, ip_type, ipaddr, netmask, gateway, dns_name1, dns_name2, essid, auth_key, enc_type);
    switch (enc_type) {
        case DANAVIDEO_WIFI_ENCTYPE_NONE:
            dbg("DANAVIDEO_WIFI_ENCTYPE_NONE\n");
            break;
        case DANAVIDEO_WIFI_ENCTYPE_WEP:
            dbg("DANAVIDEO_WIFI_ENCTYPE_WEP\n");
            break;
        case DANAVIDEO_WIFI_ENCTYPE_WPA_TKIP:
            dbg("DANAVIDEO_WIFI_ENCTYPE_WPA_TKIP\n");
            break;
        case DANAVIDEO_WIFI_ENCTYPE_WPA_AES:
            dbg("DANAVIDEO_WIFI_ENCTYPE_WPA_AES\n");
            break;
        case DANAVIDEO_WIFI_ENCTYPE_WPA2_TKIP:
            dbg("DANAVIDEO_WIFI_ENCTYPE_WPA2_TKIP\n");
            break;
        case DANAVIDEO_WIFI_ENCTYPE_WPA2_AES:
            dbg("DANAVIDEO_WIFI_ENCTYPE_WPA2_AES\n");
            break;
        case DANAVIDEO_WIFI_ENCTYPE_INVALID:
        default:
            dbg("DANAVIDEO_WIFI_ENCTYPE_INVALID\n");
            break;
    }

    Report("Connecting WiFi...\r\n");
    
    memcpy(g_ssid_name, essid, strlen(essid));
    memcpy(g_security_key, auth_key, strlen(auth_key));
    
    danaairlink_set_wifi_cb_called = true; // ‰∏ªÁ∫øÁ®ã‰ºöÂÜçÊ¨°ËøõÂÖ•ÈÖçÂØπÁä∂ÊÄ?
}

int dana_airlink_main()
{
  
  lib_danavideo_set_local_setwifiap(danavideo_setwifiap);
  char *danale_path = "/sys";
  volatile bool danaairlink_inited = false, danaairlink_started = false;
  char *if_name = "eth0";
  if (!danaairlink_set_danalepath(danale_path)) {
        dbg("testdanavideo danaairlink_set_danalepath failed\n");
        return -1;
  }
  if (!danaairlink_init(if_name)) {
        dbg("testdanavideo danaairlink_set_danalepath failed\n");
        return -1;
  }
  danaairlink_inited = true;
  dbg("testdanavideo danaairlink_init succeeded\n");
  if (!danaairlink_start()) {
        dbg("testdanavideo danaairlink_start failed\n");
        danaairlink_deinit();
        return -1;
  }
  danaairlink_started = true;
  dbg("testdanavideo danaairlink_start succeeded\n");

    while (1) {
        if (danaairlink_set_wifi_cb_called) {
            break;
        } else {
            dbg("testdanavideo sniffing...\n");
            dana_usleep(1000*1000);
        }
    }

testdanaftcv2devexit:
    dbg("testdanaairlink stop...\r\n");
    if (danaairlink_started) {
        danaairlink_stop();
        danaairlink_started = false;
    }
    if (danaairlink_inited) {
        danaairlink_deinit();
    }
    danaairlink_inited = false;



    Report("testdanaftcv2dev exit\r\n");

    return 0;

}

void danavideo_audioplay_callback(const char *data, const int32_t len, const danavidecodec_t codec)
{
  dbg("audioplaycallback: %d\t%d\n", len, codec);
}

//#define AUDIO_SUPPORT
int dana_main()
{ 
  
    dbg_on();
  
    INIT_LIST_HEAD(&(danavideotest.list));
        
    danavideotest_lock = dana_mutex_create();	
    run_th_stream = true;
    run_th_audio = true;
#if 0
    if (NULL == (dana_video_stream_test = dana_thread_create(1, 2*1024, th_stream, (void *)PORT_NUM))) {
        dbg("main dana_thread_create stream thread failed\r\n"); 
        dana_mutex_destroy(danavideotest_lock);
        return -1;
    } else {
        dbg("main dana_thread_create stream thread success\r\n"); 
    }
#endif
#ifdef AUDIO_SUPPORT
    if (NULL == (dana_video_talkback_test = dana_thread_create(1, 2*1024, th_audio, NULL))) {
        dbg("main dana_thread_create stream thread failed\r\n"); 
        dana_mutex_destroy(danavideotest_lock);
        return -1;
    } else {
        dbg("main dana_thread_create stream thread success\r\n"); 
    }
#endif
    const char *danale_path = "/sys";
    const char *agent_user = "";
    const char *agent_pass = "";
    const char *chip_code = "";
    const char *scheme_code = "";
    const char *product_code = "";

    lib_danavideo_set_local_auth(danavideo_local_auth_callback);
    lib_danavideo_set_audioplay(danavideo_audioplay_callback);

    if (lib_danavideo_init(danale_path, agent_user, agent_pass, chip_code, scheme_code, product_code, &danavideocallbackfuns)) {
        lib_danavideo_start(); // DONE
    }
	
	danapush_service_test();

    
    while (1) {
        dana_usleep(10*1000);
    }

testdanavideoexit:

    //ÂÅúÊ≠¢

    lib_danavideo_stop();
    lib_danavideo_deinit();

    run_th_stream = false;
    if (NULL != dana_video_stream_test) {
        dana_thread_destroy(dana_video_stream_test);
    }
    if (NULL != dana_video_talkback_test) {
        dana_thread_destroy(dana_video_talkback_test);
    }
    dana_mutex_destroy(danavideotest_lock);

    //ÈîÄÊØÅÈìæË°?

    return 0;
}
