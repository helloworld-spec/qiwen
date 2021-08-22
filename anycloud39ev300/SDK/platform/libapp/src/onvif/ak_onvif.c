#include "ak_common.h"

#include "ja_media.h"
#include "ja_osd.h"
#include "ja_md.h"
#include "ja_net.h"
#include "ja_type.h"
#include "ak_net.h"
#include "n1.h"
#include "n1_device.h"

#define CONFIG_JAONVIF_SUPPORT
#ifdef  CONFIG_JAONVIF_SUPPORT
#include "onvif.h"
#include "mini_rtsp.h"
#include "net_dhcp.h"

#endif

#ifdef  CONFIG_P2P_SUPPORT
#include "p2p.h"
#endif

/* new platform add */
#include "ak_onvif_config.h"

/**
 * ¼àÌý¶Ë¿Ú¡£
 */
#define LISTEN_PORT 		(80)

JA_CTRL *pja_ctrl = NULL;

#ifdef  CONFIG_JAONVIF_SUPPORT

/**
 * onvif_wsdd_event_hook - callback function for onvif search event, support ip adapt
 * @type[IN]: client type
 * @xaddr[IN]: client ip addr
 * @scopes[IN]: 
 * @event[IN]: event type
 * @customCtx[IN]: 
 * return: void
 */
int onvif_wsdd_event_hook(char *type, char *xaddr, char *scopes ,int event, void *customCtx)
{
	char ipstr[64]={0};
	unsigned int client_ip[4];

	struct onvif_net_config *net_info = onvif_config_get_net();

	if (event == WSDD_EVENT_PROBE)
	{
		if ((1 == net_info->ip_adjust_en) && (1 == ja_net_get_ip_adapt_tmp_enable())
			&& (0 == pja_ctrl->conn_cnt) && (time(NULL) - ja_net_get_ip_adapt_time() > ja_net_get_ip_adapt_peroid()))
		{
			ak_print_normal_ex("got wsdd probe from:%s type:%s\n", xaddr, type);

			sscanf(xaddr, "%u.%u.%u.%u", &client_ip[0], &client_ip[1], &client_ip[2], &client_ip[3]);

			if ((client_ip[0] > 255)
				|| (client_ip[1] > 255)
				|| (client_ip[2] > 255)
				|| (client_ip[3] > 255)
				|| ((255 == client_ip[0]) && (255 == client_ip[1]) && (255 == client_ip[2]) && (255 == client_ip[3]))
				|| ((0 == client_ip[0]) && (0 == client_ip[1]) && (0 == client_ip[2]) && (0 == client_ip[3])))
			{
				ak_print_normal_ex("client ip err:%u.%u.%u.%u\n", client_ip[0], client_ip[1], client_ip[2], client_ip[3]);
				return 0;
			}

			ak_net_get_ip("eth0", ipstr);
			NET_adapt_ip(ipstr, xaddr, ja_net_found_ip);
			ja_net_update_ip_adapt_time();
			ja_net_set_ip_adapt_peroid(60);
		}
		return 1;  // probe response
		//return 0; // no probe response
	}
	else
	{
		//ak_print_normal_ex("wsdd hook %s, %s event:%d\n\t\tscope:%s\n", type ? type : "", xaddr, event, scopes ? scopes : "");
		return 0;
	}

}
#endif

/**
 * md_alarm - md alarm, callback function for ja md module
 * return: void
 */
void md_alarm(void)
{
    static unsigned int check_time = 0;
    unsigned int cur_time = time(0);
    NK_N1Notification Notif;

    if (cur_time - check_time > 5)
    {
        ak_print_normal_ex("[%s] pass the moving notify!\n", __func__);
        check_time = cur_time;
        Notif.type = NK_N1_NOTF_MOTION_DETECTED;
        NK_N1Device_Notify(0, &Notif);
#ifdef  CONFIG_JAONVIF_SUPPORT
		ONVIF_notify_event(JA_EVENT_MD);
		ONVIF_notify_event(JA_EVENT_MD_EX);
#endif
#ifdef CONFIG_ZHONGWEI_SUPPORT
		ZW_protocol_server_notify_md(ZW_NOTF_MOTION_DETECTED);
#endif

    }
}

/**
 * init_onvif_rtsp - init minirtsp and onvif
 * @listen_port[IN]: listen port
 * return: void
 */
static void init_onvif_rtsp(int listen_port)
{
#ifdef CONFIG_JAONVIF_SUPPORT
	minirtsp_server_run();
	setenv("DEF_ETHER", "eth0", AK_TRUE);
	setenv("ONVIF_PORT", "80", AK_TRUE);

	ak_print_normal_ex("onvif run as server, v :%s, port : %d\n",
			ONVIF_version(), listen_port);
	ONVIF_SERVER_init(ONVIF_DEV_NVT, "IPC");
	//ONVIF_SERVER_start(listen_port);
	ONVIF_search_daemon_start(onvif_wsdd_event_hook, NULL);

	ja_net_init_ip_adapt();
#endif
}

/**
 * init_ir - init ircut switch and irled
 * @vi[IN]: opened vi handle
 * return: void
 */
static void init_ir(void *vi)
{
	struct onvif_image_config* img = onvif_config_get_image();
	
	ja_media_init_ir_switch();

	ja_media_irled_init();

	ja_media_start_check_fps();

	switch (img->ircut_mode) {
	case IR_MODE_AUTO:
		ja_media_set_switch_ir(AK_TRUE);
		break;
		
	case IR_MODE_DAYLIGHT:
		ja_media_set_switch_ir(AK_FALSE);
		ja_media_set_video_day_night(vi, AK_TRUE);
		break;
		
	case IR_MODE_NIGHT:
		ja_media_set_switch_ir(AK_FALSE);
		ja_media_set_video_day_night(vi, AK_FALSE);
		break;
		
	default:
		ja_media_set_switch_ir(AK_TRUE);
		break;
	}
}

/**
 * init_osd - init osd
 * @vi[IN]: opened vi handle
 * return: void
 */
static void init_osd(void *vi)
{	
	if (!pja_ctrl)
        return;
	
	struct onvif_camera_config *camera = onvif_config_get_camera();
	
	ja_osd_init_mutex();
	
	if (camera->osd_switch)
		ja_osd_init(vi);
}

/**
 * init_mutex - init all locks
 * return: void
 */
static void init_mutex(void)
{
	if (!pja_ctrl)
        return;
	
	ak_thread_mutex_init(&pja_ctrl->lock, NULL);
	ak_thread_mutex_init(&pja_ctrl->usrFilelock, NULL);

	int i;
	for (i = 0; i<CONNECT_NUM_MAX; i++) {
		ak_thread_mutex_init(&pja_ctrl->conn[i].video_lock, NULL);
		ak_thread_mutex_init(&pja_ctrl->conn[i].audio_lock, NULL);
		ak_thread_mutex_init(&pja_ctrl->conn[i].streamlock, NULL);
	}

	for (i = 0; i<MEDIA_TYPE_NUM; i++) {
		ak_thread_mutex_init(&pja_ctrl->media[i].lock, NULL);
		ak_thread_mutex_init(&pja_ctrl->media[i].str_lock, NULL);
	}
}

/**
 * destroy_mutex - destroy all locks
 * return: void
 */
static void destroy_mutex(void)
{
	if (!pja_ctrl)
        return;

	int i;
	ak_thread_mutex_destroy(&pja_ctrl->lock);
	ak_thread_mutex_destroy(&pja_ctrl->usrFilelock);

	for (i = 0; i<CONNECT_NUM_MAX; i++) {
		ak_thread_mutex_destroy(&pja_ctrl->conn[i].video_lock);
		ak_thread_mutex_destroy(&pja_ctrl->conn[i].audio_lock);
		ak_thread_mutex_destroy(&pja_ctrl->conn[i].streamlock);
	}

	for (i = 0; i<MEDIA_TYPE_NUM; i++) {
		ak_thread_mutex_destroy(&pja_ctrl->media[i].lock);
		ak_thread_mutex_destroy(&pja_ctrl->media[i].str_lock);
	}
}

/**
 * init_video - init video
 * @vi[IN]: opened vi handle
 * return: void
 */
static void init_video(void *vi)
{
	int i = 0;
	
	if (!pja_ctrl || !vi)
		return;

	/* start video process */
	ak_print_normal_ex("\t######### start video ########\n");
	ja_media_init_video(vi, pja_ctrl);

	for (i=0; i<VIDEO_CHN_NUM; i++)
		pja_ctrl->video_stat_conn[i] = -1;
}

/**
 * init_audio - init audio
 * @ai[IN]: opened ai handle
 * return: void
 */
static void init_audio(void *ai)
{
	if (!pja_ctrl || !ai)
		return;

	struct onvif_audio_config *audio = onvif_config_get_audio();
	pja_ctrl->ai_enable = audio->ai_enable;
	pja_ctrl->media[MEDIA_TYPE_AUDIO].input_handle = ai;
	ak_print_normal_ex("\tai_enable = %d\n", pja_ctrl->ai_enable);
	if (pja_ctrl->ai_enable) {
		ak_print_normal_ex("\t######### start audio ########\n");
		/* start audio process */
		ja_media_init_audio(ai, pja_ctrl);
	}
}

static const char onvif_version[] = "libapp_onvif V1.1.01";

/**
 * ak_onvif_get_version - get onvif version
 * return: version string
 */
const char* ak_onvif_get_version(void)
{
	return onvif_version;
}

/**
 * ak_onvif_init - init onvif
 * @vi[IN]: opened vi handle
 * @ai[IN]: opened ai handle
 * return: void
 */
int ak_onvif_init(void *vi, void *ai)
{
	ak_print_notice_ex("############# %s #############\n", ak_onvif_get_version());

	pja_ctrl = (JA_CTRL*)calloc(1, sizeof(JA_CTRL));
    if (!pja_ctrl) {
        return AK_FAILED;
    }

	init_mutex();

	ja_media_init_img_effect(vi);

	init_video(vi);

	init_ir(vi);

	init_audio(ai);
	
	init_osd(vi);

	/* onvif and minirtsp */
	init_onvif_rtsp(LISTEN_PORT);

	/* n1 sdk */
	n1_init(LISTEN_PORT);

	ja_md_init(vi, md_alarm);

	ja_net_set_ip_adapt_tmp_enable(AK_TRUE);

	return AK_SUCCESS;
}

/**
 * ak_onvif_destroy - destory onvif
 * return: void
 */
void ak_onvif_destroy(void)
{
	ak_print_normal_ex("destroy ....\n");

	if (!pja_ctrl) {
        return;
    }

	ja_md_destroy();

	ja_media_destroy_ir_switch();

	ja_media_stop_check_fps();

	ja_net_destroy_ip_adapt();

	ja_media_close_video(pja_ctrl);
	
	if (pja_ctrl->ai_enable)
		ja_media_close_audio(pja_ctrl);

	ONVIF_SERVER_deinit();
	MINIRTSP_server_stop();

	n1_destroy();

	destroy_mutex();
	
	ja_osd_exit();
	ja_osd_destroy_mutex();

	free (pja_ctrl);
	pja_ctrl = NULL;

	ak_print_normal_ex("destroy done!\n");
}

/**
 * ak_onvif_set_day_night - set day night mode
 * @vi_handle[IN]: opened vi handle
 * @ir_val[IN]: 0 night, 1 day
 * return: -1 failed , 0 success
 */
int ak_onvif_set_day_night(void *vi_handle, int ir_val)
{
	return ja_media_set_video_day_night(vi_handle, ir_val);
}

/**
 * ak_onvif_set_switch_ir - set ircut auto switch enable
 * @enable[IN]: enable or not
 * return: void
 */
void ak_onvif_set_switch_ir(int enable)
{
	ja_media_set_switch_ir(enable);
}

