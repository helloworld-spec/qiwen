/**
 * N1 设备实现用例。\n
 * 用例中通过调用 N1 设备模块（下简称模块）相关接口，\n
 * 实现局域网下 N1 设备与 N1 客户端（下简称客户端）交互的过程。\n
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <NkUtils/log.h>
#include <NkUtils/macro.h>

#include "n1_version.h"
#include "n1_device.h"
#include "script.h"
#include "ja_media.h"
#include "ak_cmd_exec.h"
#include "ak_vi.h"
#include "ak_ai.h"
#include "ak_vpss.h"
#include "ak_net.h"

#include "n1_device_user.h"
#include "n1_device_upgrade.h"
#include "n1_device_production.h"

#define CONFIG_JAONVIF_SUPPORT
#ifdef  CONFIG_JAONVIF_SUPPORT
#include "onvif.h"
#include "mini_rtsp.h"
#include "bind_onvif.h"
#include "net_dhcp.h"
#include "n1_device_onvif.h"
#include "n1_device_hikvision.h"
#include "n1_device_rtsp.h"

#endif

#include "env_common.h"
#include "ja_type.h"
#include "ak_osd_ex.h"
#include "ja_osd.h"
#include "ja_net.h"
#include "ja_md.h"

/* new platform add */
#include "ak_common.h"
#include "ak_venc.h"
#include "ak_onvif_config.h"

#include <sys/reboot.h>

#define USR_NAME_SIZE_MAX	(64)
#define USR_PASSWORD_SIZE_MAX	(64)

#ifdef CONFIG_N1_WIFI_SUPPORT
#define HARDWARE_CODE		391808		//制作升级包时要匹配此硬件码
#else
#define HARDWARE_CODE		391807		//制作升级包时要匹配此硬件码
#endif



#define N1_USR_FILE_PATH	"/etc/jffs2/usr.conf"
#define UPDATE_PACkAGE		"/mnt/update.tar"
#define UPDATE_PMEM_DEV    "/dev/video_ram0"
#define UPDATE_PMEM_MODULE  "/usr/modules/ak_videobuf_block.ko"


typedef struct {
	unsigned int conn_id;
	enum onvif_send_stream_id stream_id;
    unsigned long long audio_ts_ms;
	unsigned long long video_ts_ms;
	unsigned char *buf;
}N1_USR_STRUCT;

static NK_Boolean p2p_enabled = AK_FALSE;
T_VIDEO_SET_VALUE g_video_set = {0};
static int g_auth = AK_FALSE;
static unsigned long update_totalsize = 0;
volatile int update_finish = AK_FALSE;
static NK_N1LanSetup _LanSetupVideoImage;


extern const char* ak_ipc_get_version(void);
extern int WEBS_set_resource_dir(const char *resource_dir);
unsigned int n1_get_dhcp_status(void)
{
	return ja_net_get_dhcp_status();
}

NK_Void
n1_UseronEdit(NK_PVoid ctx, const NK_PChar username, const NK_PChar passphrase, NK_N1UserClassify classify)
{
	NK_TermTable Table;
	NK_Size user_count = 0;
	NK_Int i = 0;
	FILE *fd = NULL;

	user_count = NK_N1Device_NumberOfUser();

	if (0 == user_count)
	{
		ak_print_error_ex("[%s]user_count is 0!\n", __func__);
		return;
	}

	ak_thread_mutex_lock(&pja_ctrl->usrFilelock);

	fd = fopen (N1_USR_FILE_PATH, "w+b");

	if (NULL == fd)
	{
		ak_print_error_ex("n1 usr fd open failed!\n");
	}

	NK_CHECK_POINT();
	NK_TermTbl_BeginDraw(&Table, "N1 User Set", 64, 4);
	NK_TermTbl_PutKeyValue(&Table, NK_True, "Count", "%u", user_count);

	for (i = 0; i < user_count; ++i)
	{
		NK_TermTbl_PutKeyValue(&Table, NK_False, username, "%s", passphrase);

		if (NULL != fd)
		{
			fwrite(username, 1, USR_NAME_SIZE_MAX, fd);
			fwrite(passphrase, 1, USR_PASSWORD_SIZE_MAX, fd);
#ifdef CONFIG_N1_WIFI_SUPPORT
			fwrite("0", 1, USR_CLASSIFY_SIZE_MAX, fd);
#endif
		}
	}
	NK_TermTbl_EndDraw(&Table);
	if (NULL != fd)
	{
		fclose(fd);
	}
	ak_thread_mutex_unlock(&pja_ctrl->usrFilelock);

	return;
}

void n1_usr_load(void)
{
	NK_Size user_count = 0;
	NK_Int i = 0;
	NK_Char username[USR_NAME_SIZE_MAX+1], password[USR_PASSWORD_SIZE_MAX+1];
	NK_Char *buf = NULL;
	FILE *fd = NULL;
	NK_Int size = 0;

	ak_thread_mutex_lock(&pja_ctrl->usrFilelock);

	fd = fopen (N1_USR_FILE_PATH, "rb");
	if (!fd) {
		ak_thread_mutex_unlock(&pja_ctrl->usrFilelock);
		ak_print_error_ex("n1 usr fd open failed!\n");
		NK_N1Device_AddUser("admin", "", 0);
	} else {
		fseek(fd, 0, SEEK_END);
		size = ftell(fd);
		user_count = size / (USR_NAME_SIZE_MAX + USR_PASSWORD_SIZE_MAX);

		ak_print_normal_ex("n1 user_count = %d!\n", user_count);
		if (!user_count) {
			fclose(fd);
			ak_thread_mutex_unlock(&pja_ctrl->usrFilelock);
			NK_N1Device_AddUser("admin", "", 0);
			return;
		}

		buf = (NK_Char *)malloc(size);
		if (!buf) {
			fclose(fd);
			ak_thread_mutex_unlock(&pja_ctrl->usrFilelock);
			ak_print_error_ex("n1 usr buf malloc failed!\n");
			NK_N1Device_AddUser("admin", "", 0);
		} else {
			fseek(fd, 0, SEEK_SET);
			fread(buf, 1, size, fd);
			fclose(fd);
			ak_thread_mutex_unlock(&pja_ctrl->usrFilelock);

			for (i = 0; i<user_count; i++) {
				memset(username, 0, USR_NAME_SIZE_MAX + 1);
				memset(password, 0, USR_PASSWORD_SIZE_MAX + 1);
				memcpy(username,
					   	buf + i * (USR_NAME_SIZE_MAX + USR_PASSWORD_SIZE_MAX),
						USR_NAME_SIZE_MAX);
				memcpy(password,
					   	buf + i * (USR_NAME_SIZE_MAX + USR_PASSWORD_SIZE_MAX)
					   	+ USR_NAME_SIZE_MAX,
					   	USR_PASSWORD_SIZE_MAX);
				NK_N1Device_AddUser(username, password, 0);
			}
			free(buf);
		}
	}
}

/**
 * 现场抓图事件。
 *
 */
static NK_N1Ret
n1_onLiveSnapshot(NK_Int channel_id, NK_Size width, NK_Size height, NK_PByte pic, NK_Size *size)
{
	struct video_input_frame f = {{{0}, {0}}};
	struct video_stream s = {0};
	struct encode_param param = {0};
	struct onvif_camera_config *camera = onvif_config_get_camera();
	void *picture = NULL;
	NK_N1Ret ret = NK_N1_ERR_UNDEF;
	int i = 0;
	int cnt = 0;

	ak_print_normal_ex("Snapshot (Channel = %d, Size = %d).\n", channel_id, *size);

	param.width = camera->sub_width;
	param.height = camera->sub_height;
	param.minqp = 20;
	param.maxqp = 51;
	param.fps = 10;
	param.goplen = param.fps * 2;
	param.bps = 500;	//kbps
	param.profile = PROFILE_MAIN;
	param.use_chn = ENCODE_SUB_CHN;
	param.enc_grp = ENCODE_PICTURE;		//jpeg encode
	param.br_mode = BR_MODE_CBR;
	param.enc_out_type = MJPEG_ENC_TYPE;	//jpeg encode

	for (i=0; i<10; i++) {
		/* get yuv by sigle step */
		if (!ak_vi_get_frame(pja_ctrl->media[MEDIA_TYPE_VIDEO_SUB].input_handle, &f)) {
			cnt ++;
			
			if (cnt < 4) {
				ak_vi_release_frame(pja_ctrl->media[MEDIA_TYPE_VIDEO_SUB].input_handle, &f);
				continue;
			}
				
			/* open encoder */
			picture = ak_venc_open(&param);

			/* sigle encode mode */
			ak_venc_send_frame(picture, f.vi_frame[VIDEO_CHN_SUB].data,
					f.vi_frame[VIDEO_CHN_SUB].len, &s);
			/*
			 * release yuv, notice that the time you occupy
			 * yuv should as little as possible
			 */
			ak_vi_release_frame(pja_ctrl->media[MEDIA_TYPE_VIDEO_SUB].input_handle, &f);

			if (s.len < *size) {
				memcpy(pic, s.data, s.len);
				*size = s.len;
				ak_print_normal_ex("Snapshot Completed (Size = %d).\n", *size);
				ret =  NK_N1_ERR_NONE;
			}
			else {
				ak_print_normal_ex("Snapshot Size = %d is not enough, need %d.\n",
					   	*size, s.len);
			}

			/* after procee, close encoder */
			ak_venc_close(picture);
			picture = NULL;
			free(s.data);
			break;
		}
	}

	return ret;
}

/**
 * 流媒体直播相关事件。\n
 * 当客户端发现到设备时，由于两者网络配置本身存在差异而无法交互，\n
 * 客户端会通过配置设备方式使两者在局域网下满足交互条件。\n
 * 客户端配置时会触发以下事件。\n
 */
static NK_N1Ret
n1_onLiveConnected(NK_N1LiveSession *Session, NK_PVoid ctx)
{
	/**
	 *
	 * 主码流分辨率不建议大于 1920x1080 像素，帧率不建议大于 30 帧\n
	 * 辅码流分辨率不建议大于 704x480 像素，帧率不建议大于 30 帧，\n
	 * 否则使客户端处理器资源消耗过大影响体验。
	 *
	 */
	N1_USR_STRUCT *usr = NULL;
	int i = 0;

	if (pja_ctrl->update_flag)
		return NK_N1_ERR_DEVICE_BUSY;

	ak_thread_mutex_lock(&pja_ctrl->lock);

	if (CONNECT_NUM_MAX == pja_ctrl->conn_cnt) {
		ak_print_normal_ex("** conn_cnt is %d\n", pja_ctrl->conn_cnt);
		ak_thread_mutex_unlock(&pja_ctrl->lock);
		return NK_N1_ERR_DEVICE_BUSY;
	}

	usr = (N1_USR_STRUCT*)calloc(1, sizeof(N1_USR_STRUCT));
	if (!usr) {
		ak_print_error_ex("usr malloc failed!\n");
		ak_thread_mutex_unlock(&pja_ctrl->lock);
		return NK_N1_ERR_UNDEF;
	}

	for (i = 0; i < CONNECT_NUM_MAX; i++) {
	    //ak_thread_mutex_lock(&pja_ctrl->conn[i].lock);
		if (AK_FALSE == pja_ctrl->conn[i].bValid) {
			usr->conn_id = i;
			pja_ctrl->conn[i].bValid = AK_TRUE;
			pja_ctrl->conn_cnt++;
			//ak_thread_mutex_unlock(&pja_ctrl->conn[i].lock);
			break;
		}
		//ak_thread_mutex_unlock(&pja_ctrl->conn[i].lock);
	}

	if (i >= CONNECT_NUM_MAX) {
		ak_print_error_ex("can't find empty channel, max connect number: %d!\n",
				CONNECT_NUM_MAX);
		ak_thread_mutex_unlock(&pja_ctrl->lock);
		free(usr);
		return NK_N1_ERR_DEVICE_BUSY;
	}

	struct onvif_camera_config *camera = onvif_config_get_camera();

	if (VIDEO_CHN_MAIN == Session->stream_id) {
		Session->Video.width = camera->main_width;
		Session->Video.heigth = camera->main_height;
		if (NK_N1_VENC_CODEC_HEVC == g_video_set.main_enctype || NK_N1_VENC_CODEC_HEVC_PLUS == g_video_set.main_enctype)
			Session->Video.payload_type = NK_N1_DATA_PT_HEVC_NALUS;
		else
			Session->Video.payload_type = NK_N1_DATA_PT_H264_NALUS;
	} else {
		Session->Video.width = camera->sub_width;
		Session->Video.heigth = camera->sub_height;
		if (NK_N1_VENC_CODEC_HEVC == g_video_set.sub_enctype || NK_N1_VENC_CODEC_HEVC_PLUS == g_video_set.sub_enctype)
			Session->Video.payload_type = NK_N1_DATA_PT_HEVC_NALUS;
		else
			Session->Video.payload_type = NK_N1_DATA_PT_H264_NALUS;
	}
	Session->Audio.payload_type = NK_N1_DATA_PT_G711A;
	Session->Audio.sample_rate = 8000;
	Session->Audio.sample_bitwidth = 16;
	Session->Audio.stereo = NK_False;
	Session->user_session = usr;

	ak_print_normal_ex("** n1_onLiveConnected **conn : %d,** stream : %d **\n",
		   	usr->conn_id, Session->stream_id);

	ak_thread_mutex_lock(&pja_ctrl->conn[i].video_lock);
	if (VIDEO_CHN_MAIN == Session->stream_id) {
		pja_ctrl->conn[i].bMainStream = AK_TRUE;
		INIT_LIST_HEAD(&pja_ctrl->conn[i].video_queue);
		pja_ctrl->conn[i].video_run_flag = 1;
		pja_ctrl->main_cnt++;
		/*display video stat on osd */
		if (pja_ctrl->video_stat_conn[VIDEO_CHN_MAIN] < 0)
			pja_ctrl->video_stat_conn[VIDEO_CHN_MAIN] = i;
	} else if (VIDEO_CHN_SUB == Session->stream_id) {
		pja_ctrl->conn[i].bMainStream = AK_FALSE;
		INIT_LIST_HEAD(&pja_ctrl->conn[i].video_queue);
        pja_ctrl->conn[i].video_run_flag = 1;
		pja_ctrl->sub_cnt++;
		/*display video stat on osd */
		if (pja_ctrl->video_stat_conn[VIDEO_CHN_SUB] < 0)
			pja_ctrl->video_stat_conn[VIDEO_CHN_SUB] = i;
	}

	pja_ctrl->conn[i].video_stream_size = 0;
	pja_ctrl->conn[i].audio_stream_size = 0;

	/* request video */
	ja_media_venc_request_stream(pja_ctrl, Session->stream_id);
	ak_venc_set_iframe(pja_ctrl->media[Session->stream_id].enc_handle);

	pja_ctrl->conn[i].biFrameflag = AK_FALSE;

	ak_thread_mutex_unlock(&pja_ctrl->conn[i].video_lock);

	ak_thread_mutex_lock(&pja_ctrl->conn[i].audio_lock);
	INIT_LIST_HEAD(&pja_ctrl->conn[i].audio_queue);
	ak_thread_mutex_unlock(&pja_ctrl->conn[i].audio_lock);
	if (pja_ctrl->ai_enable) {
		ak_thread_mutex_lock(&pja_ctrl->conn[i].audio_lock);
		//INIT_LIST_HEAD(&pja_ctrl->conn[i].audio_queue);
		pja_ctrl->conn[i].audio_run_flag = 1;
		/* request audio */
		ja_media_aenc_request_stream(pja_ctrl);
		ak_thread_mutex_unlock(&pja_ctrl->conn[i].audio_lock);
	}
	
	ak_print_normal_ex("conn_cnt = %d, main_cnt = %d, sub_cnt = %d\n",
		pja_ctrl->conn_cnt, pja_ctrl->main_cnt, pja_ctrl->sub_cnt);
	ak_thread_mutex_unlock(&pja_ctrl->lock);

	/* init osd main and sub channel */
	ak_osd_ex_turn_on(VIDEO_CHN_MAIN);
	ak_osd_ex_turn_on(VIDEO_CHN_SUB);

	ak_print_normal_ex("Live Media(Channel=%d,Stream=%d,Session=%08x) Connected\n",
				Session->channel_id, Session->stream_id, Session->session_id);

	return NK_N1_ERR_NONE;
}

/**
 * n1_onLiveDisconnected - disconnect
 * @Session[IN]: Session struct pointer
 * @ctx[IN]: ctx
 * return: 0 success, other failed
 */
static NK_N1Ret
n1_onLiveDisconnected(NK_N1LiveSession *Session, NK_PVoid ctx)
{
	N1_USR_STRUCT *usr = Session->user_session;
	int i = 0;

	ak_thread_mutex_lock(&pja_ctrl->lock);
	if (0 == pja_ctrl->conn_cnt) {
		ak_thread_mutex_unlock(&pja_ctrl->lock);
		return NK_N1_ERR_UNDEF;
	}

	if (usr) {
		i = usr->conn_id;
	} else {
        ak_print_error_ex("n1_onLiveDisconnected invalid\n");
		ak_thread_mutex_unlock(&pja_ctrl->lock);
		return NK_N1_ERR_UNDEF;
	}
	ak_print_normal_ex("conn: %d,** stream:%d **\n",
			usr->conn_id, Session->stream_id);

	pja_ctrl->conn_cnt--;
	if (pja_ctrl->conn[i].bMainStream) {
		pja_ctrl->main_cnt--;
		/* cancel display video on osd */
		ja_media_check_video_conn_stat(pja_ctrl, VIDEO_CHN_MAIN, i);
		if (0 == pja_ctrl->main_cnt) {
			ak_osd_ex_turn_off(VIDEO_CHN_MAIN);
		}
	} else {
		pja_ctrl->sub_cnt--;
		/* cancel display video on osd */
		ja_media_check_video_conn_stat(pja_ctrl, VIDEO_CHN_SUB, i);
	}

	if ((0 == pja_ctrl->sub_cnt) && (0 == pja_ctrl->main_cnt)) {	
		/* 
		 * for snapshot, 
		 * only when there has no connecting can turn off osd 
		 */
		ak_osd_ex_turn_off(VIDEO_CHN_SUB);
	}

	if (pja_ctrl->ai_enable) {
		ak_thread_mutex_lock(&pja_ctrl->conn[i].audio_lock);
		pja_ctrl->conn[i].audio_run_flag = 0;
		ak_thread_mutex_unlock(&pja_ctrl->conn[i].audio_lock);

		if (!pja_ctrl->conn_cnt) {
			ak_print_notice_ex("going to stop aenc\n");
			ja_media_aenc_cancel_stream();
		}

		if (!ja_media_is_queue_empty(i, AK_FALSE)) {
			ak_print_normal_ex("N1 stop audio %d \n", i);

			ja_media_destroy_audio_stream_queue(i, &pja_ctrl->conn[i].audio_queue);
		}
	}

	ak_thread_mutex_lock(&pja_ctrl->conn[i].video_lock);
	pja_ctrl->conn[i].video_run_flag = 0;
	ak_thread_mutex_unlock(&pja_ctrl->conn[i].video_lock);

	if (!pja_ctrl->main_cnt && pja_ctrl->conn[i].bMainStream) {
		ak_print_notice_ex("going to stop venc main\n");
		ja_media_venc_cancel_stream(MEDIA_TYPE_VIDEO_MAIN);
	}

	if (!pja_ctrl->sub_cnt && !pja_ctrl->conn[i].bMainStream) {
		ak_print_notice_ex("going to stop venc sub\n");
		ja_media_venc_cancel_stream(MEDIA_TYPE_VIDEO_SUB);
	}

	if (!ja_media_is_queue_empty(i, AK_TRUE)) {
		ak_print_normal_ex("N1 stop ch %d, stream %s \n", i, (!Session->stream_id) ? "main" : "sub");

		/* release video stream */
		ja_media_destroy_video_stream_queue(i, &pja_ctrl->conn[i].video_queue);
	}

	pja_ctrl->conn[i].bValid = AK_FALSE;

	if (usr->buf) {
		free(usr->buf);
		usr->buf = NULL;
	}
	free(Session->user_session);
	Session->user_session = NULL;

	ak_print_normal_ex("Channel: %d Stream: %d Disconnected, conn_cnt: %d, main_cnt = %d, sub_cnt = %d\n",
			Session->channel_id, Session->stream_id, pja_ctrl->conn_cnt, pja_ctrl->main_cnt, pja_ctrl->sub_cnt);

	ak_thread_mutex_unlock(&pja_ctrl->lock);

	return NK_N1_ERR_NONE;
}

/**
 * drop_extra_audio_stream - drop audio stream
 * @idx[IN]: conn id
 * @vstr_ts[IN]: timestamp
 * return: void
 */
static void drop_extra_audio_stream(int idx, unsigned long long vstr_ts)
{
	struct aenc_entry *audio = NULL;
	struct aenc_entry *ptr = NULL;

	ak_thread_mutex_lock(&pja_ctrl->conn[idx].audio_lock);
	list_for_each_entry_safe(audio, ptr,
		&(pja_ctrl->conn[idx].audio_queue), list) {
		if (audio->stream.ts < vstr_ts) {
			list_del_init(&(audio->list));
			pja_ctrl->conn[idx].audio_stream_size -= audio->stream.len;
			ja_media_free_audio_stream(audio, AK_TRUE);
		}
	}
	ak_thread_mutex_unlock(&pja_ctrl->conn[idx].audio_lock);
}

/**
 * send_audio_stream - send audio stream
 * @usr[IN/OUT]: usr struct pointer
 * @payload_type[OUT]: payload_type
 * @ts_ms[OUT]: timestamp
 * @data[OUT]: audio stream data
 * return: size
 */
static NK_SSize send_audio_stream(N1_USR_STRUCT *usr,
								NK_N1DataPayload *payload_type,
								NK_UInt32 *ts_ms,
								NK_PByte *data)
{
	int i = usr->conn_id;
	int free_data = AK_TRUE;
	
	struct aenc_entry *audio = list_first_entry_or_null(
		&pja_ctrl->conn[i].audio_queue,	struct aenc_entry, list);
	if (NULL == audio) {
		return 0;
	}

	NK_SSize len = 0;
	if (0 == audio->stream.len) {
		ak_print_error_ex("audio->stream.len = 0\n");
		goto audio_stream_end;
	}

	if (NULL != usr->buf) {
		free(usr->buf);
		usr->buf = NULL;
	}
#if 0
	usr->buf = (unsigned char *)malloc(audio->stream.len);
	if (NULL == usr->buf) {
		ak_print_normal_ex("usr->buf malloc failed!\n");
		goto audio_stream_end;
	}

    memcpy(usr->buf, audio->stream.data, audio->stream.len);
#else
	usr->buf = audio->stream.data;
	free_data = AK_FALSE;
#endif
	*data = usr->buf;
	*ts_ms = audio->stream.ts;
	*payload_type = NK_N1_DATA_PT_G711A;
	usr->audio_ts_ms = audio->stream.ts;
	len = audio->stream.len;
	pja_ctrl->conn[i].audio_stream_size -= len;

audio_stream_end:
	ja_media_release_audio_stream(i, audio, free_data);

	return len;
}

/**
 * send_video_stream - send video stream
 * @usr[IN/OUT]: usr struct pointer
 * @payload_type[OUT]: payload_type
 * @ts_ms[OUT]: timestamp
 * @data[OUT]: video stream data
 * @type[OUT]: frame type
 * return: size
 */
static NK_SSize send_video_stream(N1_USR_STRUCT *usr,
								NK_N1DataPayload *payload_type,
								NK_UInt32 *ts_ms,
								NK_PByte *data,
								int *type)
{
	int i = usr->conn_id;
	int free_data = AK_TRUE;
	/*
	 * get video stream
	 */
	ja_video_stream *pstr_node = list_first_entry_or_null(
		&pja_ctrl->conn[i].video_queue,	ja_video_stream, list);
	if (NULL == pstr_node) {
		return 0;
	}

	NK_SSize len = 0;
	if (0 == pstr_node->vstr.len) {
		ak_print_error_ex("vstr.len = 0\n");
		goto video_stream_end;
	}

	/* if need I frame, and current frame is I, set flag, otherwise drop it */
	if (!pja_ctrl->conn[i].biFrameflag) {
		if (FRAME_TYPE_I == pstr_node->vstr.frame_type) {
			pja_ctrl->conn[i].biFrameflag = AK_TRUE;
			if (pja_ctrl->ai_enable)
				drop_extra_audio_stream(i, pstr_node->vstr.ts);
		} else {
			pja_ctrl->conn[i].video_stream_size -= pstr_node->vstr.len;
			goto video_stream_end;
		}
	}
	*type = pstr_node->vstr.frame_type;

	if (pja_ctrl->conn[i].bMainStream) {
		if (NK_N1_VENC_CODEC_HEVC == g_video_set.main_enctype || NK_N1_VENC_CODEC_HEVC_PLUS == g_video_set.main_enctype)
			*payload_type = NK_N1_DATA_PT_HEVC_NALUS;
		else
			*payload_type = NK_N1_DATA_PT_H264_NALUS;
	}else {
		if (NK_N1_VENC_CODEC_HEVC == g_video_set.sub_enctype || NK_N1_VENC_CODEC_HEVC_PLUS == g_video_set.sub_enctype)
			*payload_type = NK_N1_DATA_PT_HEVC_NALUS;
		else
			*payload_type = NK_N1_DATA_PT_H264_NALUS;
	}

	if (NULL != usr->buf) {
		free(usr->buf);
		usr->buf = NULL;
	}
#if 0
	usr->buf = (unsigned char *)malloc(pstr_node->vstr.len);
	if (NULL == usr->buf) {
		ak_print_normal_ex("usr->buf malloc failed!\n");
		pja_ctrl->conn[i].biFrameflag = AK_FALSE;
		goto video_stream_end;
	}

    memcpy(usr->buf, pstr_node->vstr.data, pstr_node->vstr.len);	
#else
	usr->buf = pstr_node->vstr.data;
	free_data = AK_FALSE;
#endif
	
	*data = usr->buf;
	*ts_ms = pstr_node->vstr.ts;
	usr->video_ts_ms = pstr_node->vstr.ts;
	len = pstr_node->vstr.len;

	pja_ctrl->conn[i].video_stream_size -= len;

	struct onvif_camera_config *camera = onvif_config_get_camera();
	/* show osd rate info */
	if (camera->rate_position > 0) {
	/* display video stat on osd */
		if (((pja_ctrl->conn[i].bMainStream) && ( i == pja_ctrl->video_stat_conn[VIDEO_CHN_MAIN])) ||
				((!pja_ctrl->conn[i].bMainStream) && ( i == pja_ctrl->video_stat_conn[VIDEO_CHN_SUB]))){
			int channel = (pja_ctrl->conn[i].bMainStream)? VIDEO_CHN_MAIN: VIDEO_CHN_SUB;
			ak_osd_ex_stat_video(channel, pja_ctrl->media[channel].stream_handle);
		}
	}

video_stream_end:
	ja_media_release_video_stream(i, pstr_node, free_data);
	return len;
}

/**
 * n1_onLiveReadFrame - get audio/video stream
 * @Session[IN/OUT]: Session struct pointer
 * @ctx[IN]: ctx
 * @payload_type[OUT]: payload_type
 * @ts_ms[OUT]: timestamp
 * @data[OUT]: audio/video stream data
 * return: size
 */
static NK_SSize
n1_onLiveReadFrame(NK_N1LiveSession *Session, NK_PVoid ctx,
		NK_N1DataPayload *payload_type, NK_UInt32 *ts_ms, NK_PByte *data,NK_N1VideoFrameType *frametype)
{
	/**
	 *
	 * 主码流单数据包总大小不建议大于 1M 字节，\n
	 * 辅码流单数据包总大小不建议大于 300k 字节，\n
	 * 否则客户端可能由于瞬时内存不足造成解码异常。
	 *
	 */
	NK_SSize len = 0;
	N1_USR_STRUCT *usr = Session->user_session;
	if (!usr) {
        ak_print_error_ex("n1_onLiveReadFrame invalid\n");
		return -1;
	}

	switch (usr->stream_id) {
	case ONVIF_STREAM_VIDEO: {
		int ak_frame_type = -1;
		if (pja_ctrl->conn[usr->conn_id].video_run_flag) {
			ak_thread_mutex_lock(&pja_ctrl->conn[usr->conn_id].video_lock);
			len = send_video_stream(usr, payload_type, ts_ms, data, &ak_frame_type);
			ak_thread_mutex_unlock(&pja_ctrl->conn[usr->conn_id].video_lock);
			if (pja_ctrl->ai_enable) {
				/* after transfer started, we switch to send audio according to ts. */
				if ((pja_ctrl->conn[usr->conn_id].biFrameflag)
					&& (usr->video_ts_ms >= usr->audio_ts_ms)) {
					usr->stream_id = ONVIF_STREAM_AUDIO;
				}
			}
		} else {
			ak_print_error_ex("run_flag 0\n");
		}

		/* set video frame tpye */
		switch (ak_frame_type) {
			case FRAME_TYPE_I:
				*frametype = NK_N1_DATA_FRAME_BASE_IDRSLICE;
				break;
			case FRAME_TYPE_P:
				*frametype = NK_N1_DATA_FRAME_BASE_PSLICE_REFBYBASE;
				break;
			case FRAME_TYPE_PI:
				*frametype = NK_N1_DATA_FRAME_BASE_PSLICE_REFTOIDR;
				break;
			default:
				*frametype = NK_N1_DATA_FRAME_UNUSE;
				break;
		}
		break;
	}
	case ONVIF_STREAM_AUDIO:
		if (pja_ctrl->ai_enable) {
			if (pja_ctrl->conn[usr->conn_id].audio_run_flag) {
				ak_thread_mutex_lock(&pja_ctrl->conn[usr->conn_id].audio_lock);
				len = send_audio_stream(usr, payload_type, ts_ms, data);
				ak_thread_mutex_unlock(&pja_ctrl->conn[usr->conn_id].audio_lock);
				/* after transfer started, we switch to send video according to ts. */
				if (usr->audio_ts_ms >= usr->video_ts_ms)
					usr->stream_id = ONVIF_STREAM_VIDEO;
	        } else {
				ak_print_error_ex("run_flag 0\n");
				usr->stream_id = ONVIF_STREAM_VIDEO;
			}
		}
		else {
			ak_print_error_ex("pja_ctrl->ai_enable 0\n");
			usr->stream_id = ONVIF_STREAM_VIDEO;
		}
			
		break;
	default:
		break;
	}

	return len;
}

static NK_N1Error
n1_onLiveAfterReadFrame(NK_N1LiveSession *Session, NK_PVoid ctx,
				NK_PByte *data_r, NK_Size size)
{
	//NK_Log()->debug("Test: Read Frame( Size = %u ) Finished.", size);
	return NK_N1_ERR_NONE;
}

/**
 * n1_set_aenc_enable - set aenc enable
 * @LanSetup[IN]: LanSetup struct pointer
 * return: void
 */
static void n1_set_aenc_enable(NK_N1LanSetup *LanSetup)
{
	struct onvif_audio_config *audio = onvif_config_get_audio();
	
	ak_print_normal_ex("cur ai_enable=%d, set value=%d\n", audio->ai_enable, LanSetup->AudioEncoder.enabled);

	if(LanSetup->AudioEncoder.enabled != audio->ai_enable){
		audio->ai_enable = LanSetup->AudioEncoder.enabled;
		onvif_config_set_audio(audio);
		onvif_config_flush_data();
		//ak_cmd_exec("sleep 2; killall -9 anyka_ipc", NULL, 0);

		pja_ctrl->ai_enable = audio->ai_enable;
		if (pja_ctrl->ai_enable) {
			ak_print_normal_ex("\t######### start audio ########\n");
			/* start audio process */
			ja_media_init_audio(pja_ctrl->media[MEDIA_TYPE_AUDIO].input_handle, pja_ctrl);
			if (pja_ctrl->conn_cnt) {
				ja_media_aenc_request_stream(pja_ctrl);
				for (int i=0; i<CONNECT_NUM_MAX; i++) {
					if (pja_ctrl->conn[i].bValid)
						pja_ctrl->conn[i].audio_run_flag = 1;
					}
				}
			} else {
			ak_print_normal_ex("\t######### close audio ########\n");
			if (pja_ctrl->conn_cnt) {
				for (int i=0; i<CONNECT_NUM_MAX; i++) {
					if (pja_ctrl->conn[i].bValid)
						pja_ctrl->conn[i].audio_run_flag = 0;
					}
				}
			ja_media_close_audio(pja_ctrl);
		}
	}
}

/**
 * n1_get_aenc_enable - get aenc enable
 * @LanSetup[OUT]: LanSetup struct pointer
 * return: void
 */
static void n1_get_aenc_enable(NK_N1LanSetup *LanSetup)
{
	struct onvif_audio_config *audio = onvif_config_get_audio();
	LanSetup->AudioEncoder.enabled = audio->ai_enable;
}

/**
 * n1_set_time - set time
 * @LanSetup[IN]: LanSetup struct pointer
 * return: void
 */
static void n1_set_time(NK_N1LanSetup *LanSetup)
{
	struct tm setup_tm;
	struct onvif_sys_config *pSys_info = onvif_config_get_sys();
			
	/* 转换成可阅读的时间格式。*/
	gmtime_r((time_t *)(&LanSetup->Time.utc), &setup_tm);
	ak_print_normal_ex("Test: Set Time %04d:%02d:%02d %02d:%02d:%02d GMT\n",
			setup_tm.tm_year + 1900, setup_tm.tm_mon + 1, setup_tm.tm_mday,
			setup_tm.tm_hour, setup_tm.tm_min, setup_tm.tm_sec);

	int tz_hour = LanSetup->Time.gmt / 3600;
	int tz_min = (LanSetup->Time.gmt - tz_hour * 3600) / 60;

	char cmd[256] = {0};	//cmd buffer
	sprintf(cmd, "date \"%d-%d-%d %d:%d:%d\"", setup_tm.tm_year+1900,
		   	setup_tm.tm_mon+1, setup_tm.tm_mday,
			setup_tm.tm_hour + tz_hour, setup_tm.tm_min + tz_min, setup_tm.tm_sec);
	ak_cmd_exec(cmd, NULL, 0);

	pSys_info->tzone = tz_hour * 100 + tz_min;
	onvif_config_set_sys(pSys_info);
	onvif_config_flush_data();
}

/**
 * n1_get_time - get time
 * @LanSetup[OUT]: LanSetup struct pointer
 * return: void
 */
static void n1_get_time(NK_N1LanSetup *LanSetup)
{
	struct onvif_sys_config *pSys_info = onvif_config_get_sys();
	
	int tz_hour = pSys_info->tzone / 100;
	int tz_min = (pSys_info->tzone - tz_hour * 100);

	LanSetup->Time.utc = time(NULL);
	LanSetup->Time.gmt = tz_hour * 3600 + tz_min * 60;
	LanSetup->Time.dst = NK_False;
}

/**
 * n1_init_img - init img params and options
 * @LanSetupVideoImage[OUT]: LanSetup struct pointer
 * return: void
 */
static void n1_init_img(NK_N1LanSetup *LanSetupVideoImage)
{
	if (LanSetupVideoImage) {
		LanSetupVideoImage->classify = NK_N1_LAN_SETUP_VIMG;
		LanSetupVideoImage->channel_id = 0;
		LanSetupVideoImage->stream_id = 0;

		LanSetupVideoImage->VideoImage.PowerLineFrequenceMode.val = 60;
		LanSetupVideoImage->VideoImage.PowerLineFrequenceMode.def = 60;
		NK_N1_PROP_ADD_OPT(&LanSetupVideoImage->VideoImage.PowerLineFrequenceMode, 50);
		NK_N1_PROP_ADD_OPT(&LanSetupVideoImage->VideoImage.PowerLineFrequenceMode, 60);
		NK_N1_PROP_ADD_OPT(&LanSetupVideoImage->VideoImage.PowerLineFrequenceMode, 100);
		NK_N1_PROP_ADD_OPT(&LanSetupVideoImage->VideoImage.PowerLineFrequenceMode, 120);

		int main_img_sz = NK_N1_IMG_SZ_1920X1080;

		LanSetupVideoImage->VideoImage.CaptureResolution.val = main_img_sz;
		LanSetupVideoImage->VideoImage.CaptureResolution.def = main_img_sz;
		NK_N1_PROP_ADD_OPT(&LanSetupVideoImage->VideoImage.CaptureResolution, main_img_sz);

		LanSetupVideoImage->VideoImage.CaptureFrameRate.val = 30;
		LanSetupVideoImage->VideoImage.CaptureFrameRate.def = 30;
		NK_N1_PROP_ADD_OPT(&LanSetupVideoImage->VideoImage.CaptureFrameRate, 25);
		NK_N1_PROP_ADD_OPT(&LanSetupVideoImage->VideoImage.CaptureFrameRate, 30);

		LanSetupVideoImage->VideoImage.HueLevel.min = LanSetupVideoImage->VideoImage.BrightnessLevel.min \
				= LanSetupVideoImage->VideoImage.SharpnessLevel.min \
				= LanSetupVideoImage->VideoImage.ContrastLevel.min \
				= LanSetupVideoImage->VideoImage.SaturationLevel.min = 0;
		LanSetupVideoImage->VideoImage.HueLevel.max = LanSetupVideoImage->VideoImage.BrightnessLevel.max \
				= LanSetupVideoImage->VideoImage.SharpnessLevel.max \
				= LanSetupVideoImage->VideoImage.ContrastLevel.max \
				= LanSetupVideoImage->VideoImage.SaturationLevel.max = 100;

		LanSetupVideoImage->VideoImage.HueLevel.def = IMG_EFFECT_DEF_VAL;
		LanSetupVideoImage->VideoImage.BrightnessLevel.def = IMG_EFFECT_DEF_VAL;
		LanSetupVideoImage->VideoImage.ContrastLevel.def = IMG_EFFECT_DEF_VAL;
		LanSetupVideoImage->VideoImage.SaturationLevel.def = IMG_EFFECT_DEF_VAL;
		LanSetupVideoImage->VideoImage.SharpnessLevel.def = IMG_EFFECT_DEF_VAL;

		struct onvif_image_config *img = onvif_config_get_image();

		LanSetupVideoImage->VideoImage.HueLevel.val = img->hue;
		LanSetupVideoImage->VideoImage.BrightnessLevel.val = img->brightness;
		LanSetupVideoImage->VideoImage.ContrastLevel.val = img->contrast;
		LanSetupVideoImage->VideoImage.SaturationLevel.val = img->saturation;
		LanSetupVideoImage->VideoImage.SharpnessLevel.val = img->sharp;

		LanSetupVideoImage->VideoImage.Flip.val = img->flip;
		LanSetupVideoImage->VideoImage.Mirror.val = img->mirror;

		/**
		 * 运动侦测。
		 */
		LanSetupVideoImage->VideoImage.MotionDetection.Enabled.val = NK_FALSE;
		LanSetupVideoImage->VideoImage.MotionDetection.SensitivityLevel.val = 80;
		LanSetupVideoImage->VideoImage.MotionDetection.SensitivityLevel.min = 0;
		LanSetupVideoImage->VideoImage.MotionDetection.SensitivityLevel.max = 100;
		LanSetupVideoImage->VideoImage.MotionDetection.Mask.width = 32;
		LanSetupVideoImage->VideoImage.MotionDetection.Mask.height = 24;

		int i = 0;
		int ii = 0;
		for (i = 0; i < LanSetupVideoImage->VideoImage.MotionDetection.Mask.height; ++i) {
			for (ii = 0; ii < LanSetupVideoImage->VideoImage.MotionDetection.Mask.width; ++ii) {
				LanSetupVideoImage->VideoImage.MotionDetection.Mask.matrix[i][ii] = NK_True;
			}
		}

		struct NK_MotionDetection motion;

		if (AK_SUCCESS == ja_md_load(&motion))
		{
			memcpy(&LanSetupVideoImage->VideoImage.MotionDetection, &motion, sizeof(motion));
		}

		/**
		 * 图像风格
		 */
		LanSetupVideoImage->VideoImage.ColorStyle.val = ja_media_get_styleId() + 1;
		LanSetupVideoImage->VideoImage.ColorStyle.def = 1;
		LanSetupVideoImage->VideoImage.ColorStyle.min = 1;
		LanSetupVideoImage->VideoImage.ColorStyle.max = 3;
	}
}

/**
 * n1_set_img - set img params
 * @LanSetupVideoImage[OUT]: LanSetup struct pointer
 * return: void
 */
static void n1_set_img(NK_N1LanSetup *LanSetupVideoImage)
{
	struct 	NK_VideoImage *VideoImage = &LanSetupVideoImage->VideoImage;
	//isp
	ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_EFFECT_HUE, VideoImage->HueLevel.val - VideoImage->HueLevel.def);
	ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_EFFECT_BRIGHTNESS, VideoImage->BrightnessLevel.val - VideoImage->BrightnessLevel.def);
	ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_EFFECT_SATURATION, VideoImage->SaturationLevel.val - VideoImage->SaturationLevel.def);
	ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_EFFECT_CONTRAST, VideoImage->ContrastLevel.val - VideoImage->ContrastLevel.def);
	ak_vpss_effect_set(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VPSS_EFFECT_SHARP, VideoImage->SharpnessLevel.val - VideoImage->SharpnessLevel.def);

	ak_vi_set_flip_mirror(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, VideoImage->Flip.val, VideoImage->Mirror.val);

	struct onvif_image_config *img = onvif_config_get_image();

	img->hue = VideoImage->HueLevel.val;
	img->brightness = VideoImage->BrightnessLevel.val;
	img->contrast = VideoImage->ContrastLevel.val;
	img->saturation = VideoImage->SaturationLevel.val;
	img->sharp = VideoImage->SharpnessLevel.val;

	img->flip = VideoImage->Flip.val;
	img->mirror = VideoImage->Mirror.val;

    onvif_config_set_image(img);
	onvif_config_flush_data();

	//motion detection
	struct NK_MotionDetection motion;

	ja_md_load(&motion);
	ja_md_store(&VideoImage->MotionDetection);

	if (NK_False == motion.Enabled.val && NK_True == VideoImage->MotionDetection.Enabled.val)
	{
		ja_md_start_movedetection();
	}
	else if (NK_True == motion.Enabled.val && NK_False == VideoImage->MotionDetection.Enabled.val)
	{
		ja_md_stop_movedetection();
	}
	else if (NK_True == motion.Enabled.val && NK_True == VideoImage->MotionDetection.Enabled.val)
	{
		ja_md_set_move_ratio(&VideoImage->MotionDetection);
	}

	//style
	if (LanSetupVideoImage->VideoImage.ColorStyle.val != ja_media_get_styleId() + 1)
	{
		ja_media_set_styleId(LanSetupVideoImage->VideoImage.ColorStyle.val - 1);
	}
}

/**
 * n1_get_img - get img params
 * @LanSetupVideoImage[OUT]: LanSetup struct pointer
 * return: void
 */
static void n1_get_img(NK_N1LanSetup *LanSetupVideoImage)
{
	ak_print_normal_ex("Test: Get Video Image Attributes.\n");

	struct onvif_image_config *img = onvif_config_get_image();
	LanSetupVideoImage->VideoImage.HueLevel.val = img->hue;
	LanSetupVideoImage->VideoImage.BrightnessLevel.val = img->brightness;
	LanSetupVideoImage->VideoImage.ContrastLevel.val = img->contrast;
	LanSetupVideoImage->VideoImage.SaturationLevel.val = img->saturation;
	LanSetupVideoImage->VideoImage.SharpnessLevel.val = img->sharp;
	LanSetupVideoImage->VideoImage.ColorStyle.val = ja_media_get_styleId() + 1;
}

/**
 * n1_set_wired - set wired params
 * @LanSetupWired[IN]: LanSetup struct pointer
 * return: void
 */
static NK_N1Ret n1_set_wired(NK_N1LanSetup *LanSetupWired)
{
	NK_Char ipaddr[32], netmask[32], gateway[32];

	/**
	 * 以下以 Linux 系统中使用 eth0:1 网卡模拟。
	 */
	NK_N1_PROP_IPV4_STR(&LanSetupWired->NetWired.IPAddress, ipaddr, sizeof(ipaddr));
	NK_N1_PROP_IPV4_STR(&LanSetupWired->NetWired.Netmask, netmask, sizeof(netmask));
	NK_N1_PROP_IPV4_STR(&LanSetupWired->NetWired.Gateway, gateway, sizeof(gateway));

	return ja_net_set_wired_net_parm(ipaddr, netmask, gateway);
}

/**
 * n1_get_wired - get wired params
 * @LanSetupWired[OUT]: LanSetup struct pointer
 * return: void
 */
static void n1_get_wired(NK_N1LanSetup *LanSetupWired)
{
	char mac[64]={0};
	struct onvif_net_config *net = onvif_config_get_net();

	ak_net_get_mac("eth0", mac, sizeof(mac));

	LanSetupWired->classify = NK_N1_LAN_SETUP_NET_WIRED;
	LanSetupWired->channel_id = 0;
	LanSetupWired->stream_id = 0;
	LanSetupWired->NetWired.EnableDHCP.val = n1_get_dhcp_status();

	NVP_IP_INIT_FROM_STRING(LanSetupWired->NetWired.IPAddress.val, net->ipaddr);
	NVP_IP_INIT_FROM_STRING(LanSetupWired->NetWired.Netmask.val, net->netmask);
	NVP_IP_INIT_FROM_STRING(LanSetupWired->NetWired.Gateway.val, net->gateway);
	NVP_IP_INIT_FROM_STRING(LanSetupWired->NetWired.DomainNameServer.val, net->gateway);
    NVP_MAC_INIT_FROM_STRING(LanSetupWired->NetWired.HwAddr.val, mac);
}

/**
 * n1_set_dns - set dns
 * @LanSetupDns[IN]: LanSetup struct pointer
 * return: void
 */
static void n1_set_dns(NK_N1LanSetup *LanSetupDns)
{
	NK_Char dns1[32], dns2[32];
	NK_N1_PROP_IPV4_STR(&LanSetupDns->DNS.Preferred, dns1, sizeof(dns1));
	NK_N1_PROP_IPV4_STR(&LanSetupDns->DNS.Alternative, dns2, sizeof(dns2));


	struct onvif_net_config *net_info = onvif_config_get_net();

	strcpy(net_info->firstdns, dns1);
	strcpy(net_info->backdns, dns2);

	char cmd[128] = {0};

	sprintf(cmd, "echo \"nameserver %s\" > /etc/resolv.conf", dns1);
	ak_cmd_exec(cmd, NULL, 0);

	sprintf(cmd, "echo \"nameserver %s\" >> /etc/resolv.conf", dns2);
	ak_cmd_exec(cmd, NULL, 0);

	ak_print_normal_ex("n1 set dns1:%s, dns2:%s\n", dns1, dns2);

	onvif_config_set_net(net_info);
	onvif_config_flush_data();
}

/**
 * n1_get_dns - get dns
 * @LanSetupDns[OUT]: LanSetup struct pointer
 * return: void
 */
static void n1_get_dns(NK_N1LanSetup *LanSetupDns)
{
	struct onvif_net_config *net = onvif_config_get_net();

	NVP_IP_INIT_FROM_STRING(LanSetupDns->DNS.Preferred.val, net->firstdns);
	NVP_IP_INIT_FROM_STRING(LanSetupDns->DNS.Alternative.val, net->backdns);
	LanSetupDns->classify = NK_N1_LAN_SETUP_DNS;
}

/**
 * n1_set_p2p_enable - set p2p enable
 * @LanSetup[IN]: LanSetup struct pointer
 * return: void
 */
static void n1_set_p2p_enable(NK_N1LanSetup *LanSetup)
{
#ifdef  CONFIG_P2P_SUPPORT
	if (p2p_enabled != LanSetup->PtoP.Enabled.val) {
		p2p_enabled = LanSetup->PtoP.Enabled.val;
		ja_media_set_p2p_enable(p2p_enabled);
		ak_print_normal_ex("set p2p: %d. Restart anyka_ipc!\n", p2p_enabled);
	        //exit(0);
			if (p2p_enabled) {
				p2p_init();
			} else {
				P2PSDKDeinit();
			}
		}
#endif
}

/**
 * n1_get_p2p_enable - get p2p enable
 * @LanSetup[OUT]: LanSetup struct pointer
 * return: void
 */
static void n1_get_p2p_enable(NK_N1LanSetup *LanSetup)
{
#ifdef  CONFIG_P2P_SUPPORT
	LanSetup->PtoP.Enabled.val = p2p_enabled;
#else
	LanSetup->PtoP.Enabled.val = 0;
#endif
}

/**
 * n1_set_osd - set osd
 * @LanSetup[IN]: LanSetup struct pointer
 * return: void
 */
static void n1_set_osd(NK_N1LanSetup *LanSetup)
{
	if (AK_FALSE == g_auth)
		return;

	char utf8[OSD_TITLE_LEN_MAX+1] = {0};
	struct onvif_camera_config *pcamera_info = onvif_config_get_camera();
	LanSetup->VideoOnScreenDisplay.Title.Enabled.read_only = NK_False;
	
	if (LanSetup->VideoOnScreenDisplay.Title.Enabled.val) {
		pcamera_info->osd_switch = 1;
		ja_osd_init(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle);

		if (LanSetup->VideoOnScreenDisplay.Title.textUTF8)
		{
			ak_print_normal_ex("\n");
			ak_osd_ex_update_name(LanSetup->VideoOnScreenDisplay.Title.Text.val);
			strncpy(pcamera_info->osd_name,
				LanSetup->VideoOnScreenDisplay.Title.Text.val,
				sizeof(pcamera_info->osd_name) - 1);
		}
		else
		{
			ak_print_normal_ex("\n");
			ak_osd_ex_g2u(LanSetup->VideoOnScreenDisplay.Title.Text.val,
				strlen(LanSetup->VideoOnScreenDisplay.Title.Text.val),
				utf8, OSD_TITLE_LEN_MAX);
			ak_osd_ex_update_name(utf8);
			strncpy(pcamera_info->osd_name, utf8, sizeof(pcamera_info->osd_name) - 1);
		}
	}
	else
	{
		pcamera_info->osd_switch = 0;
		ja_osd_exit();
	}

	onvif_config_set_camera(pcamera_info);
	onvif_config_flush_data();
}

/**
 * n1_get_osd - get osd
 * @LanSetup[OUT]: LanSetup struct pointer
 * return: void
 */
static void n1_get_osd(NK_N1LanSetup *LanSetup)
{
	char gbk[OSD_TITLE_LEN_MAX+1] = {0};
	struct onvif_camera_config *pcamera_info = onvif_config_get_camera();
	
	LanSetup->VideoOnScreenDisplay.Title.Enabled.read_only = NK_False;
	LanSetup->VideoOnScreenDisplay.Title.Enabled.val = pcamera_info->osd_switch;
	LanSetup->VideoOnScreenDisplay.Title.textUTF8 = NK_False;
	LanSetup->VideoOnScreenDisplay.Title.Text.max_len = OSD_TITLE_LEN_MAX;
	ak_osd_ex_u2g(pcamera_info->osd_name, strlen(pcamera_info->osd_name)
		, gbk, OSD_TITLE_LEN_MAX);
	NK_N1_PROP_STR_SET(&LanSetup->VideoOnScreenDisplay.Title.Text, gbk);
}

/**
 * n1_set_ai_source - set ai source
 * @LanSetup[IN]: LanSetup struct pointer
 * return: void
 */
static void n1_set_ai_source(NK_N1LanSetup *LanSetup)
{
	struct onvif_audio_config *audio_info = onvif_config_get_audio();
	int set_audio_src[3] = {0, 2, 1};
			
	audio_info->ai_source = LanSetup->AudioIO.InputMode.val;
	ak_print_normal_ex("set audio source is %d\n", LanSetup->AudioIO.InputMode.val);
	onvif_config_set_audio(audio_info);
	onvif_config_flush_data();
	ak_ai_set_source(pja_ctrl->media[MEDIA_TYPE_AUDIO].input_handle, set_audio_src[audio_info->ai_source]);
}

/**
 * n1_get_ai_source - get ai source
 * @LanSetup[OUT]: LanSetup struct pointer
 * return: void
 */
static void n1_get_ai_source(NK_N1LanSetup *LanSetup)
{
	struct onvif_audio_config *audio_info = onvif_config_get_audio();
	NK_N1_PROP_ADD_ENUM(&LanSetup->AudioIO.InputMode, N1AudioInputMode, NK_N1_AUDIO_INPUT_MODE_LINE);
	NK_N1_PROP_ADD_ENUM(&LanSetup->AudioIO.InputMode, N1AudioInputMode, NK_N1_AUDIO_INPUT_MODE_MIC);
	LanSetup->AudioIO.InputMode.val = audio_info->ai_source;
	ak_print_normal_ex("get audio source is %d\n", LanSetup->AudioIO.InputMode.val);
}

/**
 * 设备局域网配置相关事件。\n
 *
 */
NK_N1Ret
n1_onLanSetup(NK_PVoid ctx, NK_Boolean is_set, NK_N1LanSetup *LanSetup)
{
	/**
	 * 设置的时候打印设置信息。
	 */
	if (is_set) {
		NK_N1_LAN_SETUP_DUMP(LanSetup);
	}

	/**
	 * 参考配置选项。
	 * 根据各个平台的具体工作能力，配置各个参数的能力集。
	 * 客户端会根据能力集生成用户界面。
	 * 以下用静态变量的方式模拟设备保存配置的过程。
	 */

	ak_print_info_ex("n1_onLanSetup is_set = %d, LanSetup->classify = %d\n",
		   	is_set, LanSetup->classify);

	switch (LanSetup->classify) {
		
		case NK_N1_LAN_SETUP_AENC: {
			if (is_set)
				n1_set_aenc_enable(LanSetup);
			else
				n1_get_aenc_enable(LanSetup);
		}
		break;
		
		case NK_N1_LAN_SETUP_TIME:
		{
			/**
			 * 配置设备时间。
			 */
			if (is_set) {
				/**
				 * 配置设备时间参数。
				 */
				n1_set_time(LanSetup);
			} else {
				/**
				 * 获取设备时间参数配置。
				 */
				n1_get_time(LanSetup);
			}
			ak_print_normal_ex("Test: Get / Set on NK_N1LanSetup::Time.\n");
		}
		break;


		/**
		 * @name 配置视频图像相关用例。
		 */
		case NK_N1_LAN_SETUP_VIMG:
		{
			if (is_set) {
				/**
				 * 配置图像参数。
				 */
				ak_print_normal_ex("Test: Set Video Image Attributes.\n");
				memcpy(&_LanSetupVideoImage, LanSetup, sizeof(NK_N1LanSetup));
				n1_set_img(&_LanSetupVideoImage);				
			} else {
				/**
				 * 获取图像参数配置。
				 */
				n1_get_img(&_LanSetupVideoImage);
				memcpy(LanSetup, &_LanSetupVideoImage, sizeof(NK_N1LanSetup));
			}

			ak_print_normal_ex("Test: Get / Set on NK_N1LanSetup::VideoImage.\n");
		}
		break;

		/**
		 * @name 云台控制配置相关用例。
		 */
		case NK_N1_LAN_SETUP_PTZ:
		{
			NK_N1_LAN_SETUP_DUMP(LanSetup);

			NK_Log()->debug("Test: Get / Set on NK_N1LanSetup::PanTiltZoom.");
		}
		break;

		/**
		 * @name 有线网络配置。
		 */
		case NK_N1_LAN_SETUP_NET_WIRED:{
			/**
			 * 模拟本地缓存。
			 */
			if (is_set) {
				/**
				 * 设置 IP 地址。
				 */
				n1_set_wired(LanSetup);
				ak_print_normal_ex("Test: Set Wired Network.\n");
			} else {
				n1_get_wired(LanSetup);
				ak_print_info_ex("Test: Get Wired Network.\n");
			}

		}
		break;


		case NK_N1_LAN_SETUP_DNS: {
			if (is_set)
				n1_set_dns(LanSetup);
			else 
				n1_get_dns(LanSetup);
		}
		break;

		/**
		 * @ P2P 开关接口。
		 */

		case NK_N1_LAN_SETUP_P2P: {
			if (is_set)
				n1_set_p2p_enable(LanSetup);
			else
				n1_get_p2p_enable(LanSetup);
		}
		break;

		case NK_N1_LAN_SETUP_VOSD:
		{
			if (is_set)
				n1_set_osd(LanSetup);
			else
				n1_get_osd(LanSetup);
		}
		break;

		/**
		* @name ??? */
		case NK_N1_LAN_SETUP_AIO:
		{
			if (is_set)
				n1_set_ai_source(LanSetup);
			else
				n1_get_ai_source(LanSetup);
		}
		break;

		default:
			break;
	}

	if (NK_False) {
		/**
		 * 如果设置命令中存在错误参数返回以下数值。
		 */
		return NK_N1_ERR_INVALID_PARAM;
	}
	/**
	 * 配置成功。
	 */

	return NK_N1_ERR_NONE;
}

static NK_Char
_deviceID[32] = {0};


/** * 当设备在出厂设置须要录入设备序列号的时候会触发此事件。
* 这里传入的序列号须要设备负责托管保存，
* 用于当前设备各种唯一码认证场合使用。
*
*/
static NK_Void
n1_onWriteUID(NK_PVoid ctx, NK_PChar deviceID, NK_Size len)
{
	struct onvif_camera_config *camera = onvif_config_get_camera();

    memset(_deviceID, 0, sizeof(_deviceID));
	snprintf(_deviceID, sizeof(_deviceID), "%s", deviceID);
	ak_print_normal_ex("Test: Set Device ID %s.\n", _deviceID);

	if (!ja_media_sn_save(_deviceID)) {
		return ;
	}

	strcpy(camera->osd_name, "CAM1");
	onvif_config_set_camera(camera);
	onvif_config_flush_data();

	g_auth = AK_TRUE;
}

/**
 * 读取 NK_N1DeviceEventProduction::EvtProduction::onWriteUID 事件写入的序列号。
 *
 */
static NK_Void
n1_onReadUID(NK_PVoid ctx, NK_PChar deviceID, NK_Size len)
{
	len = snprintf(deviceID, len, "%s", _deviceID);
	//printf("Test: Get Device ID %s.\n", deviceID);
}

/**
 * free_pmem - free_pmem to update
 * return: 
 */
static int free_pmem(void)
{
	if (!pja_ctrl) {
        return AK_SUCCESS;
    }
	ja_osd_exit();
	ja_md_destroy();

	ja_media_destroy_ir_switch();

	ja_media_stop_check_fps();

	ja_net_destroy_ip_adapt();

	ja_media_close_video(pja_ctrl);
	if (pja_ctrl->ai_enable)
		ja_media_close_audio(pja_ctrl);
	/* vi capture_off to free pmem */
	ak_vi_capture_off(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle );
	/* close vi to free pmem */
	ak_vi_close(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle);
	pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle = NULL;

	return AK_SUCCESS;
}

/**
 * @brief
 *  固件升级申请大块内存实现。
 * @details
 *  在固件升级过程中，模块内部可能需要使用到大块内存用于文件缓冲，\n
 *  此时模块会向用户申请内存数据，参数 @ref len 描述模块需要使用的内存大小，\n
 *  用户可以分配栈区内存，或者一些既定的物理内存（如海思系列 SoC 的 MMZ 内存）提供给模块使用，\n
 *  通过参数 @ref mem 返回指针，如果用户申请内存失败不予赋值。
 *
 */
static NK_Void
UpgradeOnGetUserMem(NK_PVoid ctx, NK_Size len, NK_PVoid *mem)
{
	char result[2] = {0};
	char cmd[100] = {0};
	int size_mb = (len + 1024 * 1024 -1) /(1024 * 1024);

	if (len <= 0) {
		ak_print_error_ex("arg len error.\n");
		return;
	}
	ak_print_normal_ex("Get User Memory( Size = %u ).\n", len);

	pja_ctrl->update_flag = AK_TRUE;

	free_pmem();

	snprintf(cmd, 99, "insmod %s disk_addr=0x80000000 disk_size=0x2000000",UPDATE_PMEM_MODULE);
	ak_cmd_exec(cmd, result, sizeof(result));

	if(access(UPDATE_PMEM_DEV, F_OK) < 0){
		ak_print_error_ex("insmod ak_videobuf_block.ko fail, not find %s.\n",UPDATE_PMEM_DEV);
		return;
	}
	memset(cmd, 0, 100);
	snprintf(cmd, 99, "mkfs.vfat %s", UPDATE_PMEM_DEV);
	ak_cmd_exec(cmd, result, sizeof(result));

	memset(cmd, 0, 100);
	snprintf(cmd, 99, "mount %s /mnt", UPDATE_PMEM_DEV);
	ak_cmd_exec(cmd, result, sizeof(result));

	snprintf(cmd, 99, "dd if=/dev/zero of=%s bs=1M count=%d", UPDATE_PACkAGE, size_mb);
	/* use result only let complete to execute cmd and return. */
	ak_cmd_exec(cmd, result, sizeof(result));
	do {
		int fd;
		fd = open(UPDATE_PACkAGE, O_RDWR);
		*mem = mmap(NULL, 1024 * 1024 * size_mb, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		close(fd);
	} while (0);

	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "echo  %d > /tmp/len_file", len);
	system(cmd);

	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "echo  %ld > /tmp/mem_file", (long int)mem);
	system(cmd);

}

/**
 * @brief
 *  固件升级释放大块内存实现。
 * @details
 *  对应 UpgradeOnGetUserMem() 事件实现，模块对大块内存使用完毕以后通过触发此事件通知用户释放该内存，\n
 *  用户必须根据 @ref mem 内存地址信息释放该块内存数据，否则可能会造成应用程序内存泄漏。
 *
 */
static NK_Void
UpgradeOnPutUserMem(NK_PVoid ctx, NK_Size len, NK_PVoid mem)
{
#if 0
	char cmd[100] = {0};

	snprintf(cmd, 99, "rm %s ", UPDATE_PACkAGE);
	ak_print_normal_ex("Put User Memory( Size = %u ).\n", len);
	munmap(mem, len);
	ak_cmd_exec(cmd, NULL, 0);
#endif
}

static NK_N1Error n1_update_storefile(NK_PByte file, NK_Size size)
{
	char cmd[100] = {0};
	char mem[16] = {0};
	char result[2];

	update_totalsize = size;
	snprintf(cmd, sizeof(cmd), "echo  %d > /tmp/size_file", size);
	system(cmd);
	memset(cmd, 0, 100);
	
	update_finish = AK_FALSE;

	if (0 == size)
	{
	    ak_print_normal_ex("there is no valid update file!\n");
        return NK_N1_ERR_FIRMWARE_FILE_ERROR;
	}

    //stop camera to free memory before updating firmware
	//camera_off();
	sleep(3);

	/* use result only let complete to execute cmd and return. */
	snprintf(cmd, 99, "tar xf %s -C /mnt/", UPDATE_PACkAGE);
	ak_cmd_exec(cmd, result, sizeof(result));

	bzero(mem, sizeof(mem));
	bzero(cmd, sizeof(cmd));

	ak_cmd_exec("free | grep 'Mem' | awk '{print $4}'", mem, 16);
    ak_print_normal_ex("free mem:%s\n", mem);

	snprintf(cmd, 99, "rm %s ", UPDATE_PACkAGE);
	ak_cmd_exec(cmd, NULL, 0);

	return NK_N1_ERR_NONE;
}

/**
 * 固件镜像升级导入文件事件。
 *
 */
static NK_N1Error
UpgradeOnQualifyFile(NK_PVoid ctx, NK_PByte file, NK_Size size)
{
	/**
	 * 当前设备不支持固件升级，返回 @ref NK_N1_ERR_UPGRADE_NOT_SUPPORT。
	 * 当收到固件文件，经过校验以后发现文件错误，返回 @ref NK_N1_ERR_FIRMWARE_FILE_ERROR。
	 * 接收到固件没有任何错误，返回 @ref NK_N1_ERR_NONE, 模块会继续触发 @ref EventSet::UpgradeOnUpgradeFile 事件。
	 *
	 */
	 ak_print_normal_ex("update store file size:%u !\n", size);

	return n1_update_storefile(file, size);
}

static void n1_Firmwareupdate(NK_Size *rate)
{
	unsigned long totalsec = 22 * update_totalsize / 1024 / 1024;	//按照1M需要22秒估算所需时间
	unsigned long time = 0;

	ak_print_normal_ex("update need about %lu seconds!\n", totalsec);
	*rate = 0;

    //ak_cmd_exec("update_online.sh &", NULL, 0);

	while(time < totalsec) {
		*rate = 100 * time / totalsec;
		ak_print_normal_ex("rate = %d\n", *rate);

		if (update_finish)
            		time += 5;
		else
		    	time++;

		sleep(1);
	}

	while (!update_finish) {
		sleep(1);
	}
	*rate = 100;
	ak_print_normal_ex("rate = %d\n", *rate);
	sleep(1);
	ak_print_normal_ex("ok!\n");

}


/**
 * 固件镜像升级时获取升级进度事件。
 *
 */
static  NK_N1Error
UpgradeOnUpgradeFile(NK_PVoid ctx, NK_PByte file, NK_Size size, NK_Size *rate)
{
	/**
	 * 升级过程中一直更新 @ref rate 的数值。
	 *
	 */
    	n1_Firmwareupdate(rate);

	return NK_N1_ERR_NONE;
}

/**
 * 固件镜像升级时获取升级进度事件。
 *
 */
static NK_N1Error
UpgradeOnUpgradeROM(NK_PVoid ctx, NK_Size version, NK_PByte file, NK_Size size, NK_Size *rate)
{
	/**
	 * 升级过程中一直更新 @ref rate 的数值。
	 *
	 */
	 enum Nk_N1Result ret = NK_N1_ERR_NONE;

	ak_print_normal_ex("[%s] update version = %d, size = %d!\n", __func__, version, size);

	ret = n1_update_storefile(file, size);

	if (NK_N1_ERR_NONE != ret)
	{
		return ret;
	}

    //n1_Firmwareupdate(rate);
    *rate = 0;
	ak_cmd_exec("update_online.sh &", NULL, 0);
	sleep(5);

	return NK_N1_ERR_NONE;
}

/**
 * 固件镜像升级完成事件。
 *
 */
static NK_N1Error
UpgradeOnUpgradeEnd(NK_PVoid ctx)
{
#if 0
	ak_print_normal_ex("Upgrade end!\n");

	if (update_finish)
	{
		ak_print_normal_ex("reboot!\n");
		sleep(2);
		//ak_cmd_exec("sleep 2; reboot", NULL, 0);
		reboot(RB_AUTOBOOT);
	}
#endif

	sleep(5);

	return NK_N1_ERR_NONE;
}

/**
 * @brief
 *	获取监听端口事件。
 */
NK_UInt16
n1_HikonGetPort(NK_PVoid ctx)
{
	struct onvif_net_config *net_info = onvif_config_get_net();
	
	if(net_info->hikonport < 8000){
		return 8000;
	}
	
	return net_info->hikonport;
}
NK_Void
n1_HikonSetPort(NK_PVoid ctx, NK_UInt16 port)
{
	if(port < 8000)
		return;

	struct onvif_net_config *net_info = onvif_config_get_net();
	net_info->hikonport = port;

	onvif_config_set_net(net_info);
	onvif_config_flush_data();
}

/**
 * @brief
 *	获取监听端口事件。
 */
NK_UInt16
n1_RTSPonGetPort(NK_PVoid ctx)
{
	NK_UInt16 rtspVal = 554;

	return rtspVal;
}

/**
 * @brief
 *	恢复出厂设置
 */
NK_Void
n1_onReset(NK_PVoid ctx)
{
	ak_print_normal_ex("Test: Factroy Reset.\n");
	ak_cmd_exec("recover_cfg.sh", NULL, 0);
	sleep(3);
	ak_cmd_exec("sleep 2; reboot", NULL, 0);
}

/**
 * @brief
 *	系统重启
 */
NK_Void
n1_onReboot(NK_PVoid ctx)
{
	ak_print_normal_ex("reboot!\n");
	ak_cmd_exec("sleep 2; reboot", NULL, 0);
}

/**
 * @brief
 *	获取设备能力集事件
 */
NK_Void
n1_onCapabilities(NK_PVoid ctx, NK_N1DeviceCapabilities *Capabilities)
{
	Capabilities->hwCode = HARDWARE_CODE;
	strcpy(Capabilities->swVersion, ak_ipc_get_version());

#if CONFIG_CONSUMER_SUPPORT
	Psystem_wifi_set_info pWifi_info = anyka_get_sys_wifi_setting();
	strcpy(Capabilities->name, pWifi_info->ssid);
#else
	strcpy(Capabilities->name, "IPCAM");
#endif

#if defined CONFIG_P_R
	Capabilities->supportRJ45 = AK_FALSE;
#else
	Capabilities->supportRJ45 = AK_TRUE;
#endif

#if defined CONFIG_N1_WIFI_SUPPORT
	Capabilities->supportWiFiStation = AK_TRUE;
	Capabilities->supportWiFiAP = AK_TRUE;
#else
	Capabilities->supportWiFiStation = AK_FALSE;
	Capabilities->supportWiFiAP = AK_FALSE;
#endif

#if defined CONFIG_BRIDGE_SUPPORT
	Capabilities->supportWiFiRepeater = AK_TRUE;
#else
	Capabilities->supportWiFiRepeater = AK_FALSE;
#endif
}

/**
 * @brief
 *	设备异常事件
 */
NK_Void
n1_onCatchException(NK_PVoid ctx, NK_N1DeviceException *Excp)
{
	ak_print_error("classify->>%d\n", Excp->classify);

	ak_print_error("FrameTimeout chid ->> %d\n", Excp->ReadFrameTimeout.chid);
	ak_print_error("FrameTimeout streamid ->> %d\n", Excp->ReadFrameTimeout.streamid);
	ak_print_error("FrameTimeout seconds ->> %d\n", Excp->ReadFrameTimeout.seconds);
}

/**
 * @brief
 *	获取视频编码器参数
 * @details
 */
NK_Boolean
n1_onGetVideoEncoder(NK_PVoid ctx, NK_Int chid, NK_Int streamid, NK_N1VideoEncoder *Encoder)
{
	Encoder->Codec.def = NK_N1_VENC_CODEC_HEVC;

	NK_N1_PROP_ADD_ENUM(&Encoder->Codec, N1VideoEncCodec, NK_N1_VENC_CODEC_H264);
	NK_N1_PROP_ADD_ENUM(&Encoder->Codec, N1VideoEncCodec, NK_N1_VENC_CODEC_H264_PLUS);
	NK_N1_PROP_ADD_ENUM(&Encoder->Codec, N1VideoEncCodec, NK_N1_VENC_CODEC_HEVC);
	NK_N1_PROP_ADD_ENUM(&Encoder->Codec, N1VideoEncCodec, NK_N1_VENC_CODEC_HEVC_PLUS);

	Encoder->H264.KeyFrameInterval.val = 50;
	Encoder->H264.KeyFrameInterval.def = 50;
	Encoder->H264.KeyFrameInterval.min = 25;
	Encoder->H264.KeyFrameInterval.max = 50;

	Encoder->H264.BitRateCtrlMode.def = NK_N1_BR_CTRL_MODE_CBR;
	NK_N1_PROP_ADD_ENUM(&Encoder->H264.BitRateCtrlMode, N1BitRateCtrlMode, NK_N1_BR_CTRL_MODE_CBR);
	NK_N1_PROP_ADD_ENUM(&Encoder->H264.BitRateCtrlMode, N1BitRateCtrlMode, NK_N1_BR_CTRL_MODE_VBR);

	int main_img_sz = NK_N1_IMG_SZ_1920X1080;
	int sub_img_sz = NK_N1_IMG_SZ_640X360;

	struct onvif_camera_config *camera = onvif_config_get_camera();

	if (VIDEO_CHN_MAIN == streamid) {
		Encoder->Codec.val = g_video_set.main_enctype;

		if (1536 == camera->main_max_height)
			NK_N1_PROP_ADD_OPT(&Encoder->H264.Resolution, NK_N1_IMG_SZ_2048X1536);

		if (camera->main_max_width >= 1920)
			NK_N1_PROP_ADD_OPT(&Encoder->H264.Resolution, NK_N1_IMG_SZ_1920X1080);
		
		NK_N1_PROP_ADD_OPT(&Encoder->H264.Resolution, NK_N1_IMG_SZ_1280X720);

		if (1920 == camera->main_width)
			main_img_sz = NK_N1_IMG_SZ_1920X1080;
		else if (1536 == camera->main_height)
			main_img_sz = NK_N1_IMG_SZ_2048X1536;
		else
			main_img_sz = NK_N1_IMG_SZ_1280X720;

		Encoder->H264.Resolution.val = main_img_sz;
		Encoder->H264.Resolution.def = main_img_sz;

		Encoder->H264.BitRate.val = g_video_set.mainbps;
		Encoder->H264.BitRate.def = MAIN_BPS_DEF;
		Encoder->H264.BitRate.min = MAIN_BPS_MIN;
		Encoder->H264.BitRate.max = MAIN_BPS_MAX;

		Encoder->H264.FrameRate.val = g_video_set.mainfps;
		Encoder->H264.FrameRate.def = FPS_DEF;
		Encoder->H264.FrameRate.min = FPS_MIN;
		Encoder->H264.FrameRate.max = FPS_MAX;

		Encoder->H264.BitRateCtrlMode.val = g_video_set.main_videomode;

		Encoder->H264.BitRate.val = g_video_set.mainbps;
		Encoder->H264.FrameRate.val = g_video_set.mainfps;
		Encoder->H264.BitRateCtrlMode.val = g_video_set.main_videomode;
	} else {
		Encoder->Codec.val = g_video_set.sub_enctype;

		NK_N1_PROP_ADD_OPT(&Encoder->H264.Resolution, NK_N1_IMG_SZ_640X360);
		NK_N1_PROP_ADD_OPT(&Encoder->H264.Resolution, NK_N1_IMG_SZ_352X288);
		NK_N1_PROP_ADD_OPT(&Encoder->H264.Resolution, NK_N1_IMG_SZ_320X240);

		if (640 == camera->sub_width)
			sub_img_sz = NK_N1_IMG_SZ_640X360;
		else if (352 == camera->sub_width)
			sub_img_sz = NK_N1_IMG_SZ_352X288;
		else
			sub_img_sz = NK_N1_IMG_SZ_320X240;


		Encoder->H264.Resolution.val = sub_img_sz;
		Encoder->H264.Resolution.def = sub_img_sz;

		Encoder->H264.BitRate.val = g_video_set.subbps;
		Encoder->H264.BitRate.def = SUB_BPS_DEF;
		Encoder->H264.BitRate.min = SUB_BPS_MIN;
		Encoder->H264.BitRate.max = SUB_BPS_MAX;

		Encoder->H264.FrameRate.val = g_video_set.subfps;
		Encoder->H264.FrameRate.def = FPS_DEF;
		Encoder->H264.FrameRate.min = FPS_MIN;
		Encoder->H264.FrameRate.max = FPS_MAX;

		Encoder->H264.BitRateCtrlMode.val = g_video_set.sub_videomode;

		Encoder->H264.BitRate.val = g_video_set.subbps;
		Encoder->H264.FrameRate.val = g_video_set.subfps;
		Encoder->H264.BitRateCtrlMode.val = g_video_set.sub_videomode;
	}

	//ak_print_normal_ex("Test: Get Video Encoder Attributes.\n");

	return AK_TRUE;
}


/**
 * @brief
 *	设置视频编码器参数
 */
NK_Boolean
n1_onSetVideoEncoder(NK_PVoid ctx, NK_Int chid, NK_Int streamid, NK_N1VideoEncoder *Encoder)
{
	ak_print_normal_ex("Test: Set Video Encoder Attributes.streamid %d\n", streamid);

	ja_media_set_venc_parm(streamid, Encoder->H264.BitRate.val, 
										Encoder->H264.FrameRate.val, 
										Encoder->H264.BitRateCtrlMode.val);

	ja_media_set_venc_type(streamid, Encoder->Codec.val);

	int width = 0;
	int height = 0;

	if (VIDEO_CHN_MAIN == streamid)
	{
		switch (Encoder->H264.Resolution.val) {
		case NK_N1_IMG_SZ_1920X1080:
			width = 1920;
			height = 1080;
			break;
			
		case NK_N1_IMG_SZ_1280X720:
			width = 1280;
			height = 720;
			break;

		case NK_N1_IMG_SZ_2048X1536:
			width = 2048;
			height = 1536;
			break;

		default:
			return AK_FALSE;
			break;
		}

		ja_media_change_resolution(width, height, VIDEO_CHN_MAIN);
	}
	else
	{
		switch (Encoder->H264.Resolution.val) {
		case NK_N1_IMG_SZ_640X360:
			width = 640;
			height = 360;
			break;
			
		case NK_N1_IMG_SZ_320X240:
			width = 320;
			height = 240;
			break;

		case NK_N1_IMG_SZ_352X288:
			width = 352;
			height = 288;
			break;

		default:
			return AK_FALSE;
			break;
		}

		ja_media_change_resolution(width, height, VIDEO_CHN_SUB);
	}

	return AK_TRUE;
}

NK_Void
n1_onGetEther(NK_PVoid ctx, NK_Boolean wifi, NK_N1EthConfig *EthCfg)
{
	NK_Char ipaddr[32] = {0}, netmask[32] = {0}, gateway[32] = {0}, dns[64] = {0};

	/*wifi为false表示有线,为true表示无线*/
	if (NK_False == wifi) {
		ak_net_get_ip("eth0", ipaddr);
		ak_net_get_netmask("eth0", netmask);
		ak_net_get_route("eth0", gateway);
		ak_net_get_dns(0, dns);
	} else {
		ak_net_get_ip("wlan0", ipaddr);
		ak_net_get_netmask("wlan0", netmask);
		ak_net_get_route("wlan0", gateway);
		ak_net_get_dns(0, dns);
	}

	int tmp[4] = {0};
	sscanf(ipaddr, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]);
	NK_N1_PROP_IPV4_SET(&EthCfg->IPAddress, tmp[0], tmp[1], tmp[2], tmp[3]);
}

NK_Void
n1_onSetEther(NK_PVoid ctx, NK_Boolean wifi, NK_N1EthConfig *EthCfg)
{
	NK_Char ipaddr[32] = {0}, netmask[32] = {0}, gateway[32] = {0};
	
	NK_N1_PROP_IPV4_STR(&EthCfg->IPAddress, ipaddr, sizeof(ipaddr));
	NK_N1_PROP_IPV4_STR(&EthCfg->Netmask, netmask, sizeof(netmask));
	NK_N1_PROP_IPV4_STR(&EthCfg->Gateway, gateway, sizeof(gateway));

	/*wifi为false表示有线,为true表示无线*/
	if (NK_False == wifi) {
		ja_net_set_wired_net_parm(ipaddr, netmask, gateway);
	} else {
		ak_net_set_ip("wlan0", ipaddr);
		ak_net_set_netmask("wlan0", netmask);
		ak_net_set_default_gateway(gateway);
	}
}

/**
 * 日志擦写回调。
 */
static NK_Void
n1_onFlushLog(NK_PByte bytes, NK_Size len)
{
	/**
	 * 注意：此回调接口内不能调用 NK_Log() 相关接口。
	 */

#if 0

	FILE *fID = NK_Nil;
	fID = fopen("/tmp/n1.log", "ab");
	if (!fID) {
		ak_print_normal_ex("    Open Log Failed.\r\n");
		return;
	}
	if (len != fwrite(bytes, 1, len, fID)) {
		ak_print_normal_ex("    Write Log Failed.\r\n");
	}
	fclose(fID);
	fID = NK_Nil;
#endif
	int size = len;
	int printlen = 0;
	char buf[260] = {0};

	while(size > 0)
	{
		if (size > 256)
		{
			printlen = 256;
		}
		else
		{
			printlen = size;
		}
		memset(buf, 0, 256);
		memcpy(buf, bytes + len - size, printlen);

		ak_print_normal_ex("%s", buf);
		size -= printlen;

	}
}


/**
 * @brief
 *	设置IP自适应开关
 */
NK_Void
n1_onSetAutoIPAdapt(NK_PVoid ctx, NK_Boolean on)
{
	struct onvif_net_config *net_info = onvif_config_get_net();
	ak_print_normal_ex("Test: set IPAutoAdaption: %d.\n", on);
	if (net_info->ip_adjust_en != on)
	{
		net_info->ip_adjust_en = on;
		onvif_config_set_net(net_info);
	}
}

/**
 * @brief
 *	获取IP自适应开关
 */
NK_Boolean
n1_onGetAutoIPAdapt(NK_PVoid ctx)
{
	struct onvif_net_config *net_info = onvif_config_get_net();

	return net_info->ip_adjust_en;
}

/**
 * @brief
 *	活动 Onvif IP 自适应功能
 */
NK_Void
n1_onActiveAutoIPAdapt(NK_PVoid ctx, NK_Boolean actived)
{
	ak_print_normal_ex("Test: set ipAutoAdaptionActived: %d.\n", actived);
	ja_net_set_ip_adapt_tmp_enable(actived);
}


/**
 * n1_OnGetIRCutFilter - get ircut mode
 * @ctx[IN]: 
 * @chid[IN]: 
 * @mode[OUT]: ircut mode
 * @writable[OUT]: writable
 * return: void
 */
static NK_Void
n1_OnGetIRCutFilter(NK_PVoid ctx, NK_Int chid, NK_N1IRCutFilterMode *mode, NK_Boolean *writable)
{
	struct onvif_image_config *img = onvif_config_get_image();

	ak_print_normal_ex("[%s] Get IRcut Mode = %d \n", __FUNCTION__, img->ircut_mode);
	
	NK_N1_PROP_ADD_ENUM(mode, N1IRCutMode, NK_N1_IRCUT_MODE_AUTO);
	NK_N1_PROP_ADD_ENUM(mode, N1IRCutMode, NK_N1_IRCUT_MODE_DAYLIGHT);
	NK_N1_PROP_ADD_ENUM(mode, N1IRCutMode, NK_N1_IRCUT_MODE_NIGHT);
	
	switch(img->ircut_mode){
		case 0:
			mode->val = NK_N1_IRCUT_MODE_AUTO;
			break;
		case 1:
			mode->val = NK_N1_IRCUT_MODE_DAYLIGHT;
			break;
		case 2:
			mode->val = NK_N1_IRCUT_MODE_NIGHT;
			break;
		default:
			mode->val = NK_N1_IRCUT_MODE_AUTO;
			break;
	}

	*writable = NK_True;
}

/**
 * n1_OnSetIRCutFilter - set ircut mode
 * @ctx[IN]: 
 * @chid[IN]: 
 * @mode[IN]: ircut mode
 * return: void
 */
static NK_Void
n1_OnSetIRCutFilter(NK_PVoid ctx, NK_Int chid, NK_N1IRCutFilterMode mode)
{
	int IRcut_mode = IR_MODE_AUTO;
	struct onvif_image_config* img = onvif_config_get_image();

	switch (mode.val) {
	case NK_N1_IRCUT_MODE_DAYLIGHT:
		ja_media_set_switch_ir(AK_FALSE);
		ja_media_set_video_day_night(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, AK_TRUE);
		IRcut_mode = IR_MODE_DAYLIGHT;
		ak_print_normal_ex("IRcut switch to Daylight !\n");
		break;

	case NK_N1_IRCUT_MODE_NIGHT:
		ja_media_set_switch_ir(AK_FALSE);
		ja_media_set_video_day_night(pja_ctrl->media[MEDIA_TYPE_VIDEO_MAIN].input_handle, AK_FALSE);
		IRcut_mode = IR_MODE_NIGHT;
		ak_print_normal_ex("IRcut switch to Night Mode!\n");
		break;

	default:
		ja_media_set_switch_ir(AK_TRUE);
		ak_print_normal_ex("IRcut switch to Auto Mode !\n");
		break;
	}

	img->ircut_mode = IRcut_mode;
	onvif_config_set_image(img);
	onvif_config_flush_data();
}

static NK_Void
onMakeIFrame(NK_PVoid ctx,  NK_Int chid, NK_Int streamid)
{
	if((0 == streamid) || (1 == streamid))
	{
		ak_venc_set_iframe(pja_ctrl->media[streamid].enc_handle);

	}
}


/* *将序列号转化为MAC地址 */
static const char *netsdk_sn_to_mac(const char *sn, char *result, int result_max)
{
	unsigned int serialNumber = atoi(sn + strlen(sn) - 9);
	snprintf(result, result_max, "00:9a:%02x:%02x:%02x:%02x",
		(serialNumber >> 24) & 0xff, (serialNumber >> 16) & 0xff, (serialNumber >> 8) & 0xff, (serialNumber >> 0) & 0xff);
	return result;
}

/**
 * n1_ipcam_authentication - authentication
 * return: int
 */
static unsigned int n1_ipcam_authentication(void)
{
	unsigned int ret = 0x00100000;
	char sn[32];

    memset(sn,0,sizeof(sn));
	char mac_format[100] = {0};

	//char utf8[OSD_TITLE_LEN_MAX+1] = {0};
	struct onvif_camera_config *camera = onvif_config_get_camera();

	ak_print_normal_ex("No encryption chip\n");

	if(!access("/etc/jffs2/sn.conf",F_OK))
	{
		ak_print_normal_ex("sn.conf exist!\n");
		g_auth = AK_TRUE;
	}
	else
	{
	    strcpy(camera->osd_name,"(Unauthorized)");
		//ak_osd_ex_g2u(camera->osd_name, strlen(camera->osd_name), utf8, OSD_TITLE_LEN_MAX);
		ak_osd_ex_update_name(camera->osd_name);
		//anyka_set_camera_info(pcamera_info);

		g_auth = AK_FALSE;
	}

    ret = -1;
	
	char reSN[32] = {0};
	int re = ja_media_sn_read(reSN);
	if (AK_TRUE == re) {
		char cmd[256] = {0};	//cmd buffer
		
		netsdk_sn_to_mac(reSN, mac_format, sizeof(mac_format));
		sprintf(cmd, "ifconfig eth0 hw ether %s", mac_format);
		ak_cmd_exec(cmd, NULL, 0);
	}

	return ret;
}

/**
 * n1_init - init n1sdk
 * @listen_port[IN]: listen port
 * return: void
 */
void n1_init(int listen_port)
{
	NK_UInt32 ver_maj = 0, ver_min = 0, ver_rev = 0, ver_num = 0;
	NK_N1Device N1Device;

	/**
	 * 配置随机数种子。
	 * 建议用户在外部调用此接口，以确保库内部调用 rand 产生随机数重复性降低。
	 */
	//srand(time(NULL));
	usleep(10000);

	struct ak_timeval tv;
	ak_get_ostime(&tv);
	srand(tv.sec * tv.usec);

	/**
	 * 通过版本号判断一下二进制库与头文件是否匹配。
	 * 避免由于数据结构定义差异产生潜在的隐患。
	 */
	NK_N1Device_Version(&ver_maj, &ver_min, &ver_rev, &ver_num);
	if (NK_N1_VER_MAJ != ver_maj
			|| NK_N1_VER_MIN != ver_min
			|| NK_N1_VER_REV != ver_rev
			|| NK_N1_VER_NUM != ver_num) {
		ak_print_error_ex("Unmatched Library( %u.%u.%u-%u ) "
				"and C-Headers( %u.%u.%u-%u ) !!!\r\n",
				ver_maj, ver_min, ver_rev, ver_num,
				NK_N1_VER_MAJ, NK_N1_VER_MIN, NK_N1_VER_REV, NK_N1_VER_NUM);
		return;
	}

	/**
	 * 配置日志终端输出等级。
	 */
	/*NK_Log()->setTerminalLevel(NK_LOG_LV_INFO);
	NK_Log()->setLogLevel(NK_LOG_LV_INFO);*/
	NK_Log()->onFlush = n1_onFlushLog;

	/**
	 * 清空数据结构，可以有效避免各种内存错误。
	 */
	NK_BZERO(&N1Device, sizeof(N1Device));

	/**
	 * 设置设备的设备号，此设备号用于在局域网中作为设备的唯一标识。\n
	 * 因此每一个设备必须不一样。
	 */
	n1_ipcam_authentication();
	ja_media_sn_read(_deviceID);

	NK_BZERO(N1Device.device_id, sizeof(N1Device.device_id));
	snprintf(N1Device.device_id, sizeof(N1Device.device_id), _deviceID);

	/**
	 * 协议使用端口。
	 * 模块通过使用此端口进行 TCP/IP 数据请求监听，
	 * 因此在设计上应该避免此端口与模块以外的 TCP/IP 端口冲突。
	 */
	N1Device.port = listen_port;
	N1Device.user_ctx = NULL;

	N1Device.EventSet.onLiveSnapshot = n1_onLiveSnapshot;
	N1Device.EventSet.onLiveConnected = n1_onLiveConnected;
	N1Device.EventSet.onLiveDisconnected = n1_onLiveDisconnected;
	N1Device.EventSet.onLiveReadFrame = n1_onLiveReadFrame;
	N1Device.EventSet.onLiveAfterReadFrame = n1_onLiveAfterReadFrame;
	N1Device.EventSet.onLanSetup  = n1_onLanSetup;
	//N1Device.EventSet.onDetectRJ45Connected = n1_onDetectRJ45Connected;
	N1Device.EventSet.onReset = n1_onReset;
	N1Device.EventSet.onReboot = n1_onReboot;
	N1Device.EventSet.onCapabilities = n1_onCapabilities;
	N1Device.EventSet.onCatchException = n1_onCatchException;
	N1Device.EventSet.onGetVideoEncoder = n1_onGetVideoEncoder;
	N1Device.EventSet.onSetVideoEncoder = n1_onSetVideoEncoder;
	N1Device.EventSet.onGetEther = n1_onGetEther;
	N1Device.EventSet.onSetEther = n1_onSetEther;
	N1Device.EventSet.onGetIRCutFilter = n1_OnGetIRCutFilter;
	N1Device.EventSet.onSetIRCutFilter = n1_OnSetIRCutFilter;
	N1Device.EventSet.onMakeIFrame = onMakeIFrame;

	/**
	 * 初始化 N1 设备环境。
	 */
	if (0 != NK_N1Device_Init("/usr/local/Anyka.lic", &N1Device)) {
		ak_print_normal_ex("NK_N1Device_Init failed!\n");
		//exit(EXIT_FAILURE);
	} else {
		WEBS_set_resource_dir("/usr/share/www");

		n1_usr_load();
#ifdef CONFIG_JAONVIF_SUPPORT
		NK_SPOOK_add_service("onvif", ONVIF_nvt_probe, ONVIF_nvt_loop);
#endif
	}

	/**
	 * 扩展固件升级事件。
	 */
	NK_N1DeviceEventUpgrade EvtUpgrade;
	NK_BZERO(&EvtUpgrade, sizeof(EvtUpgrade));
	EvtUpgrade.onGetUserMem   = UpgradeOnGetUserMem;
	EvtUpgrade.onPutUserMem   = UpgradeOnPutUserMem;
	EvtUpgrade.onQualifyFile  = UpgradeOnQualifyFile;
	EvtUpgrade.onUpgradeFile  = UpgradeOnUpgradeFile;
	EvtUpgrade.onUpgradeROM   = UpgradeOnUpgradeROM;
	EvtUpgrade.onUpgradeEnd   = UpgradeOnUpgradeEnd;
	NK_N1Device_Upgrade(&EvtUpgrade);

	/**************ip adjust event*****************/
	NK_N1DeviceEventOnvif EvtOnvif;
	NK_BZERO(&EvtOnvif, sizeof(EvtOnvif));
	EvtOnvif.onSetAutoIPAdapt = n1_onSetAutoIPAdapt;
	EvtOnvif.onGetAutoIPAdapt = n1_onGetAutoIPAdapt;
	EvtOnvif.onActiveAutoIPAdapt = n1_onActiveAutoIPAdapt;
	NK_N1Device_Onvif(&EvtOnvif);

	/**************user manage event*****************/
	NK_N1DeviceEventUserManage EvtUserManage;
	NK_BZERO(&EvtUserManage, sizeof(EvtUserManage));
	EvtUserManage.onAdd = NULL;
	EvtUserManage.onEdit = n1_UseronEdit;
	EvtUserManage.onRemove = NULL;//n1_onUserChanged
	NK_N1Device_UserManage(&EvtUserManage);


	/**	 *配置 N1 设备生产相关事件。*/
	NK_N1DeviceEventProduction EvtProduction;
	NK_BZERO(&EvtProduction, sizeof(EvtProduction));
	EvtProduction.onReadUID = n1_onReadUID;
	EvtProduction.onWriteUID = n1_onWriteUID;
	EvtProduction.onGetIO = NULL;
	EvtProduction.onPutIO = NULL;
	EvtProduction.onE2PROM = NULL;
	EvtProduction.onSetSoundPrompt = NULL;
	EvtProduction.onGetSoundPrompt = NULL;
	NK_N1Device_Production(&EvtProduction);
	/**
	 * @brief
	 *	配置海康威视协议相关事件。
	 */
	
	do {
		NK_N1DeviceEventHikvision EvtHikvision;
		NK_BZERO(&EvtHikvision, sizeof(EvtHikvision));
		EvtHikvision.onGetPort = n1_HikonGetPort;
		EvtHikvision.onSetPort = n1_HikonSetPort;
		NK_N1Device_Hikvision(&EvtHikvision);
	}while(0);
	
	/**
	 * @brief
	 *	配置RTSP相关事件。
	 */
	
	do {
		NK_N1DeviceEventRTSP EvtRTSP;
		NK_BZERO(&EvtRTSP, sizeof(EvtRTSP));
		EvtRTSP.onGetPort = n1_RTSPonGetPort;
		NK_N1Device_RTSP(&EvtRTSP);
	}while(0);

	p2p_enabled = AK_FALSE;
	
	n1_init_img(&_LanSetupVideoImage);	/*init image params only one time*/
}

/**
 * n1_destroy - destroy n1sdk
 * return: void
 */
void n1_destroy(void)
{
	NK_N1Device N1Device;
	
	NK_N1Device_Destroy(&N1Device);
}

