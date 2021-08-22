#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <iconv.h>

/* plat interface */
#include "ak_global.h"
#include "ak_common.h"
#include "ak_thread.h"
#include "ak_drv_ptz.h"
#include "ak_cmd_exec.h"

/* mpi interface */
#include "ak_venc.h"

/* libapp interface */
#include "ak_alarm.h"
#include "ak_dvr_record.h"

/* danale interface */
#include "dana_fw_upgrade.h"
#include "danavideo_cmd.h"
#include "danavideo.h"
#include "danavideo_cloud.h"
#include "debug.h"

/* ak-dana interface */
#include "dana_av.h"
#include "dana_cmd.h"
#include "dana_others.h"
#include "ak_dana.h"

#include "ak_config.h"
#include "ak_vpss.h"
#include "ak_mt.h"

#define STEP_MAX 					100
#define DANA_SEND_ALARM_DEBUG		0
#define DANA_PACKAGE_SIZE           3*1024*1024                                                     //设置通道总数和允许使用的最大内存(最小1M)
#define DANA_MAX_BUFFER_DATA        1024*1024

/* control ptz for danale platform */
struct ak_ptz_ctrl_info {
	unsigned char run_flag;
    sem_t ctrl_sem;
    ak_pthread_t ptz_tid;
	danavideo_ptz_ctrl_t code;
	int step_count;
	ak_mutex_t     lock;
};

/* control mt for danale platform */
struct ak_mt_ctrl_info {
	void *mt_handle;
	unsigned char run_flag;
    ak_pthread_t mt_tid;
	void *vi_handle;
	MT_POINT cur_angle;
	int cur_fps;
	int flip;
	int mirror;
};

extern const char* ak_ipc_get_version(void);
extern int dana_time_zone;	//dana time zone
static struct ak_ptz_ctrl_info ptz_ctrl = {0};
static struct ak_mt_ctrl_info mt_ctrl = {0};

volatile bool cloud_inited = false;
volatile bool run_realtime_upload;
static int alarm_flg = 0;

/**
 * anyka_send_cloud_video - send video data to danale cloud
 * @param[IN]: reserved
 * @data[IN]:  video data
 * return:  void
 */
static void anyka_send_cloud_video(void *param, void *data)
{
	struct video_stream *pstream = (struct video_stream *)data;
	static int senddata_f = 1;
	static unsigned int discard_count = 0;
	int alarm_type = DANAVIDEO_CLOUD_ALARM_NO;

	struct video_config *video_config = ak_config_get_sys_video();
	struct camera_disp_config *camera = ak_config_get_camera_info();
	int video_type = 0;

	if (camera->main_width <= 640) {
	    switch (video_config->sub_enc_type) {
        case H264_ENC_TYPE:
            video_type = H264;
            break;
        case HEVC_ENC_TYPE:
            video_type = H265;
            break;
        default:
            break;
        }
	} else {
	    switch (video_config->main_enc_type) {
        case H264_ENC_TYPE:
            video_type = H264;
            break;
        case HEVC_ENC_TYPE:
            video_type = H265;
            break;
        default:
            break;
        }
	}

   	if(alarm_flg > 0) {
		alarm_type = DANAVIDEO_CLOUD_ALARM_VB;
		alarm_flg--;
    }

	if (FRAME_TYPE_I == pstream->frame_type)
		senddata_f = ak_dana_get_send_flag();

    if (1 == senddata_f) {
		if (lib_danavideo_cloud_realtime_upload(1, video_stream, video_type,
					pstream->frame_type, pstream->ts,
					alarm_type, (const char*)pstream->data,
					pstream->len, 0)) {
			/* dbg("th_media lib_danavideo_cloud_realtime_upload succeeded\n"); */
		} else {
			ak_print_normal_ex("upload failed\n");
		}
	} else {
        discard_count++ ;
		if (discard_count % 10 == 0) {
			ak_print_notice_ex("discard data. count:%d\n", discard_count);
    	}
	}
}

/**
 * anyka_send_cloud_audio - send audio data to danale cloud
 * @param[IN]: reserved
 * @data[IN]:  audio data
 * return:  void
 */
static void anyka_send_cloud_audio(void *param, void *data )
{
	struct audio_stream *pstream = (struct audio_stream *)data;

	if (ak_dana_get_send_flag()) {
		if (lib_danavideo_cloud_realtime_upload(1, audio_stream, G711A,
					0, pstream->ts, DANAVIDEO_CLOUD_ALARM_NO,
					(const char*)pstream->data, pstream->len, 0)) {
			//dbg("th_media lib_danavideo_cloud_realtime_upload succeeded\n");
		} else {
			dbg("th_realtime_upload lib_danavideo_cloud_realtime_upload failed\n");
		}
	}
}

/**
 * anyka_cloud_enter_realtime - set function and send video & audio data to danale cloud
 * return:  0, success; -1, failed
 */
static int anyka_cloud_enter_realtime(void)
{
    /* ak_print_normal("[%s:%d]  !\n", __func__, __LINE__); */
	if (run_realtime_upload) {
		ak_print_normal("\x1b[33mdanavideo_cloud_enter_realtime_upload already started\x1b[0m\n");
		return 0;
		}
	if (!lib_danavideo_cloud_realtime_on()) {
		ak_print_normal("\x1b[33mdanavideo_cloud_enter_realtime_upload lib_danavideo_cloud_realtime_on failed\x1b[0m\n");
		return -1;
	}
	run_realtime_upload = true;

	dana_av_start_video(DANA_CLOUD_VIDEO, anyka_send_cloud_video, (void *)NULL,45);
	dana_av_start_audio(DANA_CLOUD_AUDIO, anyka_send_cloud_audio, (void *)NULL);

    return 0;
}

/**
 * anyka_cloud_leave_realtime - exit from send video & audio data to danale cloud
 * return:  void
 */
static void anyka_cloud_leave_realtime(void)
{
    /* ak_print_normal("[%s:%d]  !\n", __func__, __LINE__);*/
    if (!run_realtime_upload) {
		ak_print_normal("\x1b[33mdanavideo_cloud_leave_realtime_upload already stopped\x1b[0m\n");
		return ;
	}
	run_realtime_upload = false;
    dana_av_stop(DANA_CLOUD_VIDEO, NULL);
	dana_av_stop(DANA_CLOUD_AUDIO, NULL);
	lib_danavideo_cloud_realtime_off();

	return ;
}

/**
 * anyka_cloud_changed_callback - set cmd to ptz  and  ptz run
 * @cloud_mode[IN]:  cloud mode
 * @ch_no[IN]:  reserved
 * return:  void
 */
static void anyka_cloud_changed_callback(const danavideo_cloud_mode_t cloud_mode,const int ch_no)
{
	switch (cloud_mode) {
	case DANAVIDEO_CLOUD_MODE_UNKNOWN:
		ak_print_normal_ex("unkown mode\n");
		anyka_cloud_leave_realtime();
		break;
	case DANAVIDEO_CLOUD_MODE_REALTIME:
		ak_print_normal_ex("realtime mode\n");
		anyka_cloud_enter_realtime();
		break;
	case DANAVIDEO_CLOUD_MODE_ALARM:
		ak_print_normal_ex("alarm mode\n");
		anyka_cloud_leave_realtime();
		break;
	default:
		ak_print_normal_ex("should never\n");
		return;
	}
}

/**
 * dana_others_cloud_init - int cloud for danale
 * return:   void;
 */
void dana_others_cloud_init(void)
{
	/* after lib_danavideo_init  */
	ak_print_normal("using libdanavideo_cloud_version: %s\n",
		lib_danavideo_cloud_linked_version_str(lib_danavideo_cloud_linked_version()));

	unsigned int max_buffering_data = DANA_MAX_BUFFER_DATA;
	unsigned int package_size = DANA_PACKAGE_SIZE;
	if(!cloud_inited){
		lib_danavideo_cloud_set_cloud_mode_changed(anyka_cloud_changed_callback);

		while (!lib_danavideo_cloud_init(1, max_buffering_data, package_size,
			DANAVIDEO_CLOUD_MODE_REALTIME)) {
		   ak_print_normal("lib_danavideo_cloud_init failed, try again\n");
			ak_sleep_ms(200);
		}

		ak_print_normal("lib_danavideo_cloud_init OK\n");
		cloud_inited = true;
	}
}

static void* dana_ptz_ctrl_thread(void* param)
{
	enum ptz_status status[PTZ_DEV_NUM];
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	ak_thread_set_name("dana_ptz");

	while (ptz_ctrl.run_flag) {
		sem_wait(&ptz_ctrl.ctrl_sem);


		while(ptz_ctrl.run_flag
			&& (ptz_ctrl.code > DANAVIDEO_PTZ_CTRL_STOP)
			&& (ptz_ctrl.code <= DANAVIDEO_PTZ_CTRL_MOVE_RIGHT)
			&& (ptz_ctrl.step_count++ < STEP_MAX) ) {
			ak_thread_mutex_lock(&ptz_ctrl.lock);
			ak_mt_wait_camera_rotation(mt_ctrl.mt_handle);
			
			switch (ptz_ctrl.code) {
			case DANAVIDEO_PTZ_CTRL_MOVE_UP:
				/* run up step */
				ak_drv_ptz_turn(PTZ_TURN_UP,8);
				mt_ctrl.cur_angle.py += 8;
				break;
			case DANAVIDEO_PTZ_CTRL_MOVE_DOWN:
				/* run down step */
				ak_drv_ptz_turn(PTZ_TURN_DOWN,8);
				mt_ctrl.cur_angle.py -= 8;
				break;
			case DANAVIDEO_PTZ_CTRL_MOVE_RIGHT:
				/* run left step */
				ak_drv_ptz_turn(PTZ_TURN_LEFT,16);
				mt_ctrl.cur_angle.px += 16;
				break;
			case DANAVIDEO_PTZ_CTRL_MOVE_LEFT:
				/* run right step */
				ak_drv_ptz_turn(PTZ_TURN_RIGHT,16);
				mt_ctrl.cur_angle.px -= 16;
				break;
			default:
				break;
			}
			do{
				ak_sleep_ms(5);
				ak_drv_ptz_get_status(PTZ_DEV_H, &status[PTZ_DEV_H]);
				ak_drv_ptz_get_status(PTZ_DEV_V, &status[PTZ_DEV_V]);
			} while((status[PTZ_DEV_H] != PTZ_INIT_OK)
			    || (status[PTZ_DEV_V] != PTZ_INIT_OK));

			ak_mt_update_camera_angle(mt_ctrl.mt_handle, mt_ctrl.cur_angle.px, mt_ctrl.cur_angle.py);
			ak_thread_mutex_unlock(&ptz_ctrl.lock);
		}

		ptz_ctrl.step_count = 0;

	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
    return NULL;
}

/**
 * dana_others_ptz_init - int ptz for danale
 * return:   0 , success ;  -1 , failed;
 */
int dana_others_ptz_init(void)
{
	if (ptz_ctrl.run_flag) {
		return AK_SUCCESS;
	}

	if(ak_drv_ptz_open() < 0){
		ak_print_error_ex("ak_drv_ptz_open failed!\n");
		return AK_FAILED;
	}

    /* change motor param if you changed your machine mould */
	ak_drv_ptz_set_angle_rate(24/24.0, 21/21.0);
	ak_drv_ptz_set_degree(350, 130);
	ak_drv_ptz_check_self(PTZ_FEEDBACK_PIN_NONE);

	ak_thread_sem_init(&ptz_ctrl.ctrl_sem, 0);
	ak_thread_mutex_init(&ptz_ctrl.lock, NULL);
	
	ptz_ctrl.step_count = 0;
	ptz_ctrl.run_flag = AK_TRUE;

	int ret = ak_thread_create(&(ptz_ctrl.ptz_tid), dana_ptz_ctrl_thread,
        NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if(ret){
		ak_print_normal_ex("create dana_ptz_ctrl_thread failed, ret=%d\n", ret);
	} else {
		ak_print_normal_ex("dana ptz control start\n");
	}

	return ret;
}

/**
 * dana_others_ptz_exit - exit ptz for danale
 * return:   void;
 */
void dana_others_ptz_exit(void)
{
	if (ptz_ctrl.run_flag) {
		ptz_ctrl.run_flag = AK_FALSE;
		ak_thread_sem_post(&(ptz_ctrl.ctrl_sem));
		ak_thread_join(ptz_ctrl.ptz_tid);
		ak_thread_sem_destroy(&(ptz_ctrl.ctrl_sem));
		ak_print_notice_ex("dana_ptz_ctrl_thread join OK\n");
		ak_drv_ptz_close();
		ak_thread_mutex_destroy(&ptz_ctrl.lock);
	}
}

/**
 * dana_others_set_ptz_cmd - set cmd to ptz  and  ptz run
 * @code[IN]: cmd
 * @para1[IN]:  reserved
 * return:   0 , success ;  -1 , failed;
 */
int dana_others_set_ptz_cmd( int code , int para1)
{
    ak_print_normal_ex("ptz code:%d\n", code);

	switch (code) {
	/*after up down left right cmd, pzt move until stop cmd*/
	case DANAVIDEO_PTZ_CTRL_STOP:
	case DANAVIDEO_PTZ_CTRL_MOVE_UP:
	case DANAVIDEO_PTZ_CTRL_MOVE_DOWN:
	case DANAVIDEO_PTZ_CTRL_MOVE_LEFT:
	case DANAVIDEO_PTZ_CTRL_MOVE_RIGHT:
		ptz_ctrl.code = code;
		ptz_ctrl.step_count = 0;
		sem_post(&ptz_ctrl.ctrl_sem);
		break;
	default :
		return 0;
	}

	return 0;
}

static void* sys_update(void *user)
{
   	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	ak_thread_set_name("dana_ota");
	/* wait update confirm return to dana */
	sleep(2);
	ak_cmd_exec("/usr/sbin/update.sh &", NULL, 0);

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
	return NULL;
}

#define LOCAL_UPDATEFILE  "/tmp/update_pkg.tar"
static unsigned int new_rom_come(const uint64_t rom_size, const char *rom_ver, char save_pathname[512])
{
    ak_print_normal_ex("\n");
	strcpy(save_pathname, LOCAL_UPDATEFILE);

	return 0;
}

static void rom_download_complete(const uint32_t code, const char *rom_pathname)
{
	ak_print_normal_ex("\n");
}

static void upgrade_confirm(const char *rom_pathname, uint32_t *upgrade_timeout_sec)
{
    char cmd_str[50] = {0};
	*upgrade_timeout_sec = 6*60;

	/* stop record */
	ak_dvr_record_exit();

	ak_print_notice_ex("start upgrade...\n");
    snprintf(cmd_str,50,"tar xf %s -C /tmp/",LOCAL_UPDATEFILE);
	/* system("tar xf /tmp/update_pkg.tar -C /tmp/"); */
	/* ak_print_normal("\n%s\n",cmd_str); */
	system(cmd_str);
	/* system("ls /tmp/ -l"); */
    snprintf(cmd_str,50,"rm -f %s",LOCAL_UPDATEFILE);
	/* ak_print_normal("\n%s\n",cmd_str); */
	system(cmd_str);
	sleep(1);
	system("ls /tmp/ -l");

	ak_pthread_t thread_id;
	int ret = ak_thread_create(&thread_id, sys_update,
        NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	if(ret){
		ak_print_normal_ex("create sys_update thread failed, ret=%d\n", ret);
	} else {
		ak_print_normal_ex("dana sys update start\n");
	}
}

dana_fw_upgrade_callback_funs_t cbs = {
	.dana_fw_upgrade_new_rom_come = new_rom_come,
	.dana_fw_upgrade_rom_download_complete = rom_download_complete,
	.dana_fw_upgrade_confirm = upgrade_confirm,
};

/**
 * dana_others_upgrade_init - init OTA upgrade for danale
 * @danale_path[IN]:  danale conf path
 * return:   0 , success ;  -1 , failed;
 */
int dana_others_upgrade_init(const char *danale_path)
{
   unsigned int channel_num = 2;
   char *rom_ver = dana_others_get_verion();
   char *api_ver = lib_danavideo_linked_version_str(lib_danavideo_linked_version());
   char *rom_md5 = "1234567890abc123456";

  	/*
  	 *lib_danavideo_util_setdeviceinfo : rom_ver  api_ver rom_md5
	 * DANAVIDEOCMD_GETBASEINFO :dana_id api_ver rom_ver
	 */
	ak_print_normal_ex("rom_md5:%s api_ver:%s rom_ver:%s\n",
		rom_md5, api_ver, rom_ver);

	/* rom_ver < 129; api_ver < 129; rom_md5 < 65 */
	lib_danavideo_util_setdeviceinfo(DANAVIDEO_DEVICE_IPC, channel_num,
		rom_ver, api_ver, rom_md5);

	if (!dana_fw_upgrade_init(danale_path, &cbs)) {
	 	dbg("testdanafwupgrade dana_fw_upgrade_init failed\n");
	}

	return 0;
}


static int code_convert(char *from_charset, char *to_charset,
		char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset, from_charset);
	if (cd == (iconv_t)-1) {
		perror("iconv_open");
		return -1;
	}
	memset(outbuf, 0, outlen);
	if (iconv(cd, pin, &inlen, pout, &outlen) == (size_t)-1) {
		perror("iconv");
		return -1;
	}
	iconv_close(cd);
	*pout = '\0';

	return 0;
}

/**
 * dana_others_u2g - convert "utf-8" to "gbk"
 * @inbuf[IN]:  utf-8  code string
 * @inlen[IN]:  strlen
 * @outbuf[OUT]:  gbk code string
 * @outlen[IN]:  strlen
 * return:   0 , success ;  -1 , failed;
 */
int dana_others_u2g(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("utf-8", "gbk", inbuf, inlen, outbuf, outlen);
}
#if 0
int dana_others_g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("gbk", "utf-8", inbuf, inlen, outbuf, outlen);
}
#endif

/**
 * dana_others_settime - set device time
 * @time_arg[IN]: the time to set
 * return:   0 , success ;  -1 , failed;
 */
int dana_others_settime(long long time_arg)
{
	time_t t = (time_t)time_arg;
	struct tm *str;

	str = gmtime(&t);
	if(str == NULL)
	{
		ak_print_error_ex("get local time error\n");
		return -1;
	}

	/* construc cmd: date year-mon-mday hour:min:sec */
	char cmd[256] = {0};
	sprintf(cmd, "date \"%d-%d-%d %d:%d:%d\"", str->tm_year+1900, str->tm_mon+1, str->tm_mday,
						str->tm_hour, str->tm_min, str->tm_sec);

	if(system(cmd) < 0)
	{
		return -1;
	}
	/*
	*bzero(cmd, sizeof(cmd));  //clean buffer
    	*save to rtc
	*sprintf(cmd, "hwclock -w");
	*system(cmd);
	*/

	return 0;
}

/**
 * dana_others_send_alarm - send alarm info to danale
 * @type[IN]:  alarm type
 * @level[IN]:  sensitivity
 * @start_time[IN]:  time that alarm triggered
 * @time_len[IN]:  reserved
 * return:   void;
 */
void dana_others_send_alarm(int type,int level,int start_time, int time_len)
{
    char     *msg_title = "anyka alarm";
    char     *msg_body  = "lib_danavideo_util_pushmsg";
    int64_t  cur_time = time(0);
    uint32_t msg_type = 1;

    if (type == SYS_DETECT_MOVE_ALARM) {
        msg_type = 1;
		alarm_flg = 12 * 60;
    } else if(type == SYS_DETECT_VOICE_ALARM){
        msg_type = 2;
    } else
        msg_type = 4;

#if DANA_SEND_ALARM_DEBUG
	ak_print_normal("alarm_type=%d, alarm_level=%d, start_time=%d, time_len=%d  \n",
		type, level, start_time, time_len);

	struct ak_date date;
	ak_seconds_to_date(cur_time, &date);
	ak_print_date(&date);
#endif

    if (lib_danavideo_util_pushmsg(1,level, msg_type, msg_title,
		msg_body, cur_time, 0, NULL, NULL, 0, 0, 0, 0, NULL)) {
		ak_print_debug_ex("lib_danavideo_util_pushmsg success\n");
    } else {
      	ak_print_debug_ex("lib_danavideo_util_pushmsg failed ,not online\n");
    }
}

/**
 * dana_others_alarm_init - init alarm
 * @vi_handle[IN]:  vi handle
 * @ai_handle[IN]:  ai handle
 * return:   void;
 */
void dana_others_alarm_init(void *vi_handle, void *ai_handle)
{
	struct sys_alarm_config *alarm_cfg = ak_config_get_sys_alarm();

	ak_alarm_set_interval_time(alarm_cfg->send_msg_time, alarm_cfg->interval_time);
	ak_alarm_set_ratios(SYS_DETECT_MOVE_ALARM, alarm_cfg->md_set, 3,
			&alarm_cfg->md_level_1);
	ak_alarm_set_ratios(SYS_DETECT_VOICE_ALARM, alarm_cfg->sd_set, 3,
		   	&alarm_cfg->sd_level_1);
	ak_alarm_init(SYS_DETECT_MOVE_ALARM,vi_handle, dana_others_send_alarm, NULL);
	ak_alarm_init(SYS_DETECT_VOICE_ALARM,ai_handle, dana_others_send_alarm, NULL);
}

/**
 * dana_others_get_verion - get system version
 * return:   version string;
 */
char* dana_others_get_verion(void)
{
	int i = 0, j = 0;
	static char dana_version[20] = {0};
	const char *src = ak_ipc_get_version();

	memset(dana_version, 0, sizeof(dana_version));
	for (i= 0; i < sizeof(dana_version); ++i) {
		if ('\0' == src[i]) {
			break;
		}

		if(isdigit(src[i]) || ('.' == src[i])) {
			dana_version[j] = src[i];
			++j;
		}
	}

	return dana_version;
}


void wait_ptz_turn(void)
{
	enum ptz_status status[PTZ_DEV_NUM];
	
	do{
		ak_sleep_ms(5);
		ak_drv_ptz_get_status(PTZ_DEV_H, &status[PTZ_DEV_H]);
		ak_drv_ptz_get_status(PTZ_DEV_V, &status[PTZ_DEV_V]);
	} while((status[PTZ_DEV_H] != PTZ_INIT_OK)
	    || (status[PTZ_DEV_V] != PTZ_INIT_OK));
}

void check_flip_mirror(void)
{
	int flip = 0;
	int mirror = 0;
	
   	ak_vi_get_flip_mirror(mt_ctrl.vi_handle, &flip, &mirror);

	if (flip != mt_ctrl.flip) {
		mt_ctrl.flip = flip;

		ak_mt_reverse_rotation_direction_v(mt_ctrl.mt_handle);
	}

	if (mirror != mt_ctrl.mirror) {
		mt_ctrl.mirror = mirror;

		ak_mt_reverse_rotation_direction_h(mt_ctrl.mt_handle);
	}
}


static void* dana_mt_ctrl_thread(void* param)
{
	ak_print_normal_ex("thread id: %ld\n", ak_thread_get_tid());
	ak_thread_set_name("dana_mt");
	struct vpss_md_info md_info = {0};
	MT_POINT angle = {0};
	int ret = -1;
	struct mt_config *mt = ak_config_get_mt();
	int sensor_fps = 25;
	MT_RECTANGLE rect = {{0}};
	struct video_channel_attr attr = {{0}};  // code_updating
	int md_v_size = 0;
	
	ak_vi_get_channel_attr(mt_ctrl.vi_handle, &attr); // code_updating
	int vid_height = attr.res[VIDEO_CHN_MAIN].height; // code_updating
	int vid_width = attr.res[VIDEO_CHN_MAIN].width; // code_updating

	if (0 == vid_height % VPSS_MD_DIMENSION_V_MAX)
		md_v_size = VPSS_MD_DIMENSION_V_MAX;
	else
		md_v_size = 16;

	ak_mt_set_motion_region_params(mt_ctrl.mt_handle, md_v_size, mt->valid_size_min, mt->valid_size_max, 10, 8);

	while (mt_ctrl.run_flag) {
		/*get isp 3d stat info*/
		ret = ak_vpss_md_get_stat(mt_ctrl.vi_handle, &md_info);
		if (0 != ret) {
			ak_print_error_ex("get 3d nr stat info fail\n");
			break;
		} 

		check_flip_mirror();

		ret = ak_mt_tracking(mt_ctrl.mt_handle, md_info.stat, md_v_size, &rect, &angle);

		if (mt->draw_box_en) {
			ak_vi_set_box_rect((rect.pt.px - 1) * vid_width / MD_SIZE,
					(rect.pt.py - 1) * vid_height / md_v_size, 
					rect.width * vid_width / MD_SIZE,
					rect.height * vid_height / md_v_size);
		}
			
		if (ret) {
			ak_print_normal_ex("angle.px = %d, angle.py = %d, ret = %d\n", angle.px, angle.py, ret);
			ak_thread_mutex_lock(&ptz_ctrl.lock);
			ak_mt_wait_camera_rotation(mt_ctrl.mt_handle);

			if (mt->calibrate_en) {
				if (2 == ret){
					ak_drv_ptz_reset_dg(PTZ_DEV_H);
					ak_drv_ptz_reset_dg(PTZ_DEV_V);
					
					ak_drv_ptz_turn(PTZ_TURN_LEFT, 360);
					wait_ptz_turn();

					ak_drv_ptz_turn(PTZ_TURN_UP, 360);
					wait_ptz_turn();

					ak_drv_ptz_turn_to_pos(angle.px, angle.py);
					wait_ptz_turn();
				} else {
					ak_drv_ptz_turn_to_pos(angle.px, angle.py);
					wait_ptz_turn();
				}		
			} else {
				ak_drv_ptz_turn_to_pos(angle.px, angle.py);
				wait_ptz_turn();
			}
			
			mt_ctrl.cur_angle.px = angle.px;
			mt_ctrl.cur_angle.py = angle.py;
			ak_mt_update_camera_angle(mt_ctrl.mt_handle, mt_ctrl.cur_angle.px, mt_ctrl.cur_angle.py);
			ak_thread_mutex_unlock(&ptz_ctrl.lock);
		}		

		sensor_fps = ak_vi_get_fps(mt_ctrl.vi_handle);
		if (sensor_fps != mt_ctrl.cur_fps) {
			ak_mt_set_fps(mt_ctrl.mt_handle, sensor_fps);
			ak_mt_set_max_invalid_frames(mt_ctrl.mt_handle, mt->wait_time * sensor_fps);
			mt_ctrl.cur_fps = sensor_fps;

			if (25 == sensor_fps) {	//day high light
				ak_mt_set_max_decision_frames(mt_ctrl.mt_handle, 7);
				ak_mt_set_mrd_filters(mt_ctrl.mt_handle, mt->flt_big_day, mt->flt_small_day);
			} else {	// night, day low light
				ak_mt_set_max_decision_frames(mt_ctrl.mt_handle, 5);
				ak_mt_set_mrd_filters(mt_ctrl.mt_handle, mt->flt_big_night, mt->flt_small_night);
			}
		}
		
		if (mt_ctrl.cur_fps > 0) {	//day high light
			ak_sleep_ms(1000 / mt_ctrl.cur_fps);
		} else {
			ak_sleep_ms(100);
		}
	}

	ak_print_normal_ex("### thread id: %ld exit ###\n", ak_thread_get_tid());
	ak_thread_exit();
    return NULL;
}

/**
 * dana_others_mt_init - mt init
 * @vi_handle[IN]: opened vi_handle
 * return:  0 success, -1 failed
 */
int dana_others_mt_init(void *vi_handle)
{
	struct mt_config *mt = ak_config_get_mt();
	
	if (mt_ctrl.run_flag) {
		return AK_SUCCESS;
	}
	
	mt_ctrl.mt_handle = ak_mt_init();
	
	if (NULL == mt_ctrl.mt_handle)
		return AK_FAILED;

	ak_print_normal_ex("************mt params : **************\n");
	ak_print_normal_ex("************calibrate_en : %d*********\n", mt->calibrate_en);
	ak_print_normal_ex("************wait_time : %d************\n", mt->wait_time);
	ak_print_normal_ex("************draw_box_en : %d**********\n", mt->draw_box_en);
	ak_print_normal_ex("************switch_v : %d*************\n", mt->switch_v);
	ak_print_normal_ex("************flt_big_day : %d**********\n", mt->flt_big_day);
	ak_print_normal_ex("************flt_big_night : %d********\n", mt->flt_big_night);
	ak_print_normal_ex("************flt_small_day : %d********\n", mt->flt_small_day);
	ak_print_normal_ex("************flt_small_night : %d******\n", mt->flt_small_night);
	ak_print_normal_ex("************valid_size_min : %d*******\n", mt->valid_size_min);
	ak_print_normal_ex("************valid_size_max : %d*******\n", mt->valid_size_max);

	//ak_mt_set_motion_region_params(mt_ctrl.mt_handle, VPSS_MD_DIMENSION_V_MAX, 10, 300, 10, 8);
	ak_mt_set_camera_rotation_range(mt_ctrl.mt_handle, 0, 350, 0, 130);
	ak_mt_set_camera_init_angle(mt_ctrl.mt_handle, 180, 65);
	ak_mt_set_mrd_filters(mt_ctrl.mt_handle, mt->flt_big_day, mt->flt_small_day);
	//ak_mt_set_camera_rotation_size_h(mt_ctrl.mt_handle, 20);
	ak_mt_set_wait_time(mt_ctrl.mt_handle, 200);
	//ak_mt_set_max_decision_frames(mt_ctrl.mt_handle, 1);
	//ak_mt_set_wait_frames(mt_ctrl.mt_handle, 1);

	if (mt->switch_v)
		ak_mt_open_camera_rotation_switch_v(mt_ctrl.mt_handle);

	mt_ctrl.run_flag = AK_TRUE;
	mt_ctrl.vi_handle = vi_handle;
	mt_ctrl.cur_angle.px = 180;
	mt_ctrl.cur_angle.py = 65;
	mt_ctrl.cur_fps = 25;
	mt_ctrl.flip = 0;
	mt_ctrl.mirror = 0;

	int ret = ak_thread_create(&(mt_ctrl.mt_tid), dana_mt_ctrl_thread,
        NULL, ANYKA_THREAD_MIN_STACK_SIZE, -1);
	if(ret){
		ak_print_normal_ex("create dana_mt_ctrl_thread failed, ret=%d\n", ret);
	} else {
		ak_print_normal_ex("dana mt control start\n");
	}

	return ret;
}

/**
 * dana_others_mt_exit - mt exit
 * return:  void
 */
void dana_others_mt_exit(void)
{
	if (mt_ctrl.run_flag) {
		mt_ctrl.run_flag = AK_FALSE;
		ak_thread_join(mt_ctrl.mt_tid);
		ak_print_notice_ex("dana_mt_ctrl_thread join OK\n");
		ak_mt_destroy(mt_ctrl.mt_handle);
		mt_ctrl.mt_handle = NULL;
	}
}

