#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <ctype.h>
#include <sys/statfs.h>
#include <errno.h>

#include "dana_common.h"
#include "danavideo_cmd.h"
#include "danavideo.h"
#include "debug.h"
#include "dana_cmd.h"
#include "list.h"

#include "ak_global.h"
#include "ak_common.h"
#include "ak_vi.h"
#include "ak_vpss.h"
#include "ak_thread.h"
#include "ak_config.h"
#include "ak_dvr_common.h"
#include "ak_dvr_replay.h"
#include "ak_dana.h"
#include "ak_dvr_record.h"
#include "ak_demux.h"
#include "record_ctrl.h"

#define DANA_RECORD_REPLAY_DBG  0
extern int dana_time_zone;	//dana time zone
unsigned int dana_func_mode = 0;
static int	video_flip_tate = 0;
unsigned long replay_end_time = 0;
static struct list_head *replay_list = NULL;

/**
 * dana_cmd_set_alarm - set alarm configure
 * @alarm_info[IN]: 	alarm configure
 * return:   void;
 */
void dana_cmd_set_alarm(DANAVIDEOCMD_SETALARM_ARG *alarm_info)
{
	struct sys_alarm_config *sys_alarm = ak_config_get_sys_alarm();

	sys_alarm->md_set = alarm_info->motion_detection;
	sys_alarm->sd_set = alarm_info->opensound_detection;
	sys_alarm->i2o_detection = alarm_info->openi2o_detection;
	sys_alarm->smoke_detection = alarm_info->smoke_detection;
	sys_alarm->shadow_detection = alarm_info->shadow_detection;
	sys_alarm->other_detection = alarm_info->other_detection;

	ak_config_set_sys_alarm();
	ak_config_flush_data();
}

/**
 * ak_get_sdcard_total_size - get sd card total size
 * @path[IN]: 	 sd card mount point path
 * return:   >0 ,   sd card total size;  -1 , failed;
 */
uint64_t ak_get_sdcard_total_size(char *path)
{

	uint64_t total_size = 0;
	struct statfs disk_statfs;

	memset(&disk_statfs,0, sizeof( struct statfs ) );
	while ( statfs( path, &disk_statfs ) == -1 ) {
		if ( errno != EINTR ) {
			ak_print_normal( "statfs: %s Last error == %s\n", path, strerror(errno) );
			return -1;
		}
	}
	total_size = disk_statfs.f_bsize;
	total_size = total_size * disk_statfs.f_blocks;

	return total_size;
}

/**
 * ak_get_sdcard_free_size - get sd card free size
 * @path[IN]: 	 sd card mount point path
 * return:   >0 ,   sd card free size;  -1 , failed;
 */
uint64_t ak_get_sdcard_free_size(char *path)
{
	uint64_t free_size = 0;
	struct statfs disk_statfs;

	bzero(&disk_statfs, sizeof( struct statfs ) );
	while ( statfs( path, &disk_statfs ) == -1 ) {
		if ( errno != EINTR ) {
			ak_print_normal( "statfs: %s Last error == %s\n", path, strerror(errno) );
			return -1;
		}
	}
	free_size = disk_statfs.f_bsize;
	free_size = free_size * disk_statfs.f_bavail;

	return free_size;
}

/**
 * dana_cmd_get_alarm - get alarm configure
 * @alarm_info[OUT]: 	alarm configure
 * return:   void;
 */
void dana_cmd_get_alarm(DANAVIDEOCMD_SETALARM_ARG *alarm_info)
{

	struct sys_alarm_config *sys_alarm = ak_config_get_sys_alarm();

   	alarm_info->motion_detection = sys_alarm->md_set;
	alarm_info->opensound_detection = sys_alarm->sd_set;
	alarm_info->openi2o_detection = sys_alarm->i2o_detection ;
	alarm_info->smoke_detection = sys_alarm->smoke_detection ;
	alarm_info->shadow_detection = sys_alarm->shadow_detection;
	alarm_info->other_detection = sys_alarm->other_detection ;
}

/**
 * dana_cmd_g_rec_plan - get record video plan
 * @plan_num[OUT]: 	 record video plan number
 * return:   record video plan list or null ;
 */
libdanavideo_recplanget_recplan_t* dana_cmd_g_rec_plan(uint32_t *plan_num)
{
	libdanavideo_recplanget_recplan_t *local_plan = NULL;
	*plan_num = 0;

	int i, index, hour, min, sec, j;
	struct video_record_config * psys_setting;

	psys_setting = ak_config_get_record();		//get sys record plan
	*plan_num = 0;
	for(i = 0, index = 0; i < MAX_RECORD_PLAN_NUM; i++)
	{
	    ak_print_normal("haveplan :%d, %d\n", i, psys_setting->plan[i].active);
	    if(psys_setting->plan[i].active)
	    {
	        (*plan_num) ++;
	    }
	}

	if(*plan_num == 0)
	{
	    ak_print_normal("have no plan\n");
	    return NULL;
	}
	local_plan = calloc(1, sizeof(libdanavideo_recplanget_recplan_t)*(*plan_num));

	for(i = 0, index = 0; i < MAX_RECORD_PLAN_NUM; i++)
	{
	    if(psys_setting->plan[i].active)
	    {
	        local_plan[index].status = psys_setting->plan[i].active;
	        hour = psys_setting->plan[i].end_time / 3600 ;
	        min = (psys_setting->plan[i].end_time % 3600) / 60 ;
	        sec = psys_setting->plan[i].end_time % 60;
	        sprintf(local_plan[index].end_time, "%02d:%02d:%02d", hour, min, sec);
	        hour = psys_setting->plan[i].start_time / 3600 ;
	        min = (psys_setting->plan[i].start_time % 3600) / 60 ;
	        sec = psys_setting->plan[i].start_time % 60;
	        sprintf(local_plan[index].start_time, "%02d:%02d:%02d", hour, min, sec);
	        local_plan[index].record_no = index;
	        local_plan[index].week_count = 0;


	        for(j = 0; j < 7; j ++)
	        {
	            if(psys_setting->plan[i].week_enalbe[j])
	            {
	                if(j == 0)
	                {
	                    local_plan[index].week[local_plan[index].week_count ++] = DANAVIDEO_REC_WEEK_SUN;
	                }
	                else
	                {
	                    local_plan[index].week[local_plan[index].week_count ++] = j;
	                }
	            }
	        }
	        index ++ ;
	    }
	}

	return local_plan;
}

static void set_record_plan(struct video_record_config * psys_record_plan,
	struct record_plan_config *plan)
{
	int index = 0;

	if(plan->active)
	{
	    for(index = 0; index < MAX_RECORD_PLAN_NUM; index ++)
	    {
	        if(psys_record_plan->plan[index].active == 1)
	        {
	            break;
	        }
	    }

	    if(index == MAX_RECORD_PLAN_NUM)/*new add*/
	    {
	    	index =0;
	    }
	    memcpy(&psys_record_plan->plan[index], plan, sizeof(struct record_plan_config));

	}
	else
	{
	    for(index = 0; index < MAX_RECORD_PLAN_NUM; index ++)
	    {
	    	 if(psys_record_plan->plan[index].active == 1)
	        {
	            break;
	        }
	    }
	    if(index < MAX_RECORD_PLAN_NUM){
	        psys_record_plan->plan[index].active = 0;
	    }
	}
	ak_config_set_record_plan(psys_record_plan);
}

/**
 * dana_cmd_s_rec_plan - set record video plan
 * @pnet_record_plan[IN]: 	 record video plan info
 * return:   void;
 */
void dana_cmd_s_rec_plan(DANAVIDEOCMD_RECPLANSET_ARG * pnet_record_plan)
{
	int i;
	int hour, min, sec;
	struct record_plan_config local_plan;
	struct video_record_config * psys_setting;
	psys_setting = ak_config_get_record();

	ak_print_normal_ex("start:%s, end:%s, week count:%d\n",pnet_record_plan->start_time,
		pnet_record_plan->end_time,pnet_record_plan->week_count);
	local_plan.active = pnet_record_plan->status;
	sscanf(pnet_record_plan->start_time, "%d:%d:%d", &hour, &min, &sec);
	local_plan.start_time = hour * 3600 + min * 60 + sec;
	sscanf(pnet_record_plan->end_time, "%d:%d:%d", &hour, &min, &sec);
	/*
	ak_print_normal_ex("end_time:%s hour:%d, min:%d, sec:%d\n",pnet_record_plan->end_time,
		hour, min, sec);
		*/
	/*dana app can only set hour & minutes . if we set whole day to record,
	  dana app send 23:59:00, lost 1 minutes record.
	  */
	if((23 == hour) && (59 == min) && (00 == sec))
		sec = 59;
	/*ak_print_normal_ex(" hour:%d, min:%d, sec:%d\n",hour, min, sec);*/
	local_plan.end_time =  hour * 3600 + min * 60 + sec;

	for(i = 0; i < pnet_record_plan->week_count; i++)
	{
		ak_print_normal_ex("%d, end:%d\n",i,pnet_record_plan->week[i]);

	    if(pnet_record_plan->week[i] == DANAVIDEO_REC_WEEK_SUN)
	    {
	        local_plan.week_enalbe[0]= 1;
	    }
	    else
	    {
	        local_plan.week_enalbe[pnet_record_plan->week[i]]= 1;
	    }
	}
	set_record_plan(psys_setting,&local_plan);
	ak_config_flush_data();

	record_ctrl_update_dvr_plan_time();
}

/**
 * dana_cmd_send_video - send video data to danale
 * @param[IN]: 	 net  info
 * @data[IN]: 	 video data
 * return:   void;
 */
void dana_cmd_send_video(void *param,void *data, int encode_type)
{
	if (NULL == param || NULL == data) {
		ak_print_error_ex("NULL -----param=%p, data=%p  \n",param, data);
		return;
	}

    MYDATA *mydata = (MYDATA *)param;
	struct video_stream *pstream = (struct video_stream *)data ;
	uint8_t is_keyframe = (FRAME_TYPE_I == pstream->frame_type)? 1 : 0;
	static int senddata_f = 0;
	static unsigned int fail_count = 0;

	if (FRAME_TYPE_I == pstream->frame_type) {
		 senddata_f = ak_dana_get_send_flag();
	}
    if ((1 == senddata_f) && (NULL != (pdana_video_conn_t)mydata->danavideoconn)) {
    	if (!lib_danavideoconn_send((pdana_video_conn_t)mydata->danavideoconn,
       		video_stream, encode_type, mydata->chan_no, is_keyframe,
       		pstream->ts, (const char*)pstream->data,
       		pstream->len, 0)) {
			fail_count++ ;
			if (fail_count % 10 == 0)
				ak_print_info_ex("send fail. count:%d\n", fail_count);
		}
	}

}

void ak_dana_send_pictrue(void *parm, struct video_stream *pstream)
{
	if (NULL == parm || NULL == pstream) {
		return;
	}
	MYDATA *mydata = (MYDATA *)parm;

  	if( NULL != (pdana_video_conn_t)mydata->danavideoconn)
	{
          lib_danavideoconn_send((pdana_video_conn_t)mydata->danavideoconn, pic_stream, JPG, mydata->chan_no,
        	1, pstream->ts, (const char*)pstream->data, pstream->len, 0);
  	}
}

/**
 * dana_cmd_send_audio - send audio data to danale
 * @param[IN]: 	 net  info
 * @data[IN]: 	 audio data
 * return:   void;
 */
void dana_cmd_send_audio(void *param, void *data, int encode_type)
{
	if (NULL == param || NULL == data) {
		return;
	}

	MYDATA *mydata = (MYDATA *)param;
	struct audio_stream *pstream = (struct audio_stream *)data ;

	/*if half mode for audio and speak enable, listen disable */
	if (((dana_func_mode & DANA_TALK_FULL_MODE) == 0)
			&& (dana_func_mode & DANA_SPEAK_ENABLE)) {
		return;
	}

	if (ak_dana_get_send_flag() &&
			(NULL != (pdana_video_conn_t)mydata->danavideoconn)) {

		lib_danavideoconn_send((pdana_video_conn_t)mydata->danavideoconn,
				audio_stream, encode_type, mydata->chan_no, 0, pstream->ts,
				(const char*)pstream->data, pstream->len, 0);
	}
}

/**
 * dana_cmd_get_talk_data - get audio data from danale
 * @param[IN]: 	 net  info
 * @data[IN]: 	 audio data
 * return:    audio data or  null ;
 */
void* dana_cmd_get_talk_data(void *param )
{
	if (NULL == param) {
		return NULL;
	}
	MYDATA *mydata = (MYDATA *)param;
	struct audio_stream *ptalk_data = NULL;

	dana_audio_packet_t *pmsg = lib_danavideoconn_readaudio(mydata->danavideoconn, 1000);
	if (pmsg)
	{
	    ptalk_data = calloc(1, sizeof(struct audio_stream));
	    ptalk_data->data = (unsigned char *)calloc(1, pmsg->len);
	    ptalk_data->len = pmsg->len;
	    memcpy((void *)ptalk_data->data, pmsg->data, ptalk_data->len);
	    lib_danavideo_audio_packet_destroy(pmsg);
	}
	return (void *)ptalk_data;
}

static int copy_record_info(struct list_head *head,
		libdanavideo_reclist_recordinfo_t *record_info,int *rec_num,
							int cpy_index, int cpy_count)
{
	int index = 0;
	int dana_count = 0;
	int recordlist_count = 0;
	struct dvr_replay_entry *entry = NULL;
/* max of cpy_count is 30, we need to upload more to dana to save time
 * danale suggestion: if 2 record file is continusly, we can reunit it as 1 to upload
 */
	list_for_each_entry(entry, head, list) {
		if (NULL != entry) {
			if (index < cpy_index) {
				index++;
				continue;
			}
			if (dana_count >= cpy_count)
				break;
			if ((dana_count) && ((entry->start_time - dana_time_zone - 2) <
				(record_info[dana_count - 1].length + record_info[dana_count - 1].start_time))){
				record_info[dana_count - 1].length += entry->total_time/1000;
				//ak_print_normal_ex("%s length:%u\n",entry->path,record_info[dana_count - 1].length);
			} else {
				/* entry->total_time is ms */
				record_info[dana_count].length = entry->total_time/1000;
				record_info[dana_count].start_time = entry->start_time - dana_time_zone;
				record_info[dana_count].record_type = DANAVIDEO_REC_TYPE_NORMAL;
				dana_count++;
				//ak_print_normal_ex("index:%d dana_count:%d %s\n",index,dana_count, entry->path);
			}
			index++;
			recordlist_count++;
		}
	}
	*rec_num = dana_count;

	return recordlist_count;
}

/**
* @brief 	date_time_begin
* 			check whether the time is the date begin time(00:00:00)
* @time 	time is the linux second time
* @return 	if is the date begin time return AK_TRUE,else return AK_FAISE
*/
static int date_time_begin(unsigned long time)
{
	struct ak_date date = {0};

	ak_seconds_to_date(time, &date);
	if (0 == date.hour && 0 == date.minute && 0 == date.second) {
		return AK_TRUE;
	}
	else
		return AK_FALSE;
}

/**
 * dana_cmd_get_reclist - get record video list info
 * @rec_info[IN]: 	param to search record video list info
 * @rec_num[OUT]: record video number
 * return:   record video list info;
 */
libdanavideo_reclist_recordinfo_t* dana_cmd_get_reclist(
		DANAVIDEOCMD_RECLIST_ARG *rec_info, int *rec_num)
{
	static int cpy_index = 0;
	static int list_count = 0; /* replay file count */
	libdanavideo_reclist_recordinfo_t *record_info = NULL;
	unsigned long check_time = rec_info->last_time;
	/* maximum size number of record file */
   	int max_count = rec_info->get_num;
	int info_count = 0;
	int cpy_num = 0;
	struct dvr_replay_param param = {0};
	struct video_record_config *record_param = ak_config_get_record();

	*rec_num = 0;
	check_time += dana_time_zone;
	param.type = record_param->file_type;	//DVR_FILE_TYPE_MP4; //DVR_FILE_TYPE_AVI;
   	ak_print_normal_ex("get file begin, check_time=%ld, max_count=%d\n",
			check_time, max_count);


	/*
	 * replay_list is empty or check_time is 00:00:00
	 * we fetch list only one time for each day
	 */
	if (date_time_begin(check_time) || !replay_list) {
		/* some video file is from last night to this day */
		param.start_time = check_time;
		/* save the replay end time */
		replay_end_time = check_time + 24*60*60;
		param.end_time = replay_end_time;

		if (replay_list) {
			ak_dvr_replay_free_fetch_list(replay_list);
		}
		replay_list = NULL;
		list_count = ak_dvr_replay_fetch_list(&param, &replay_list);
		cpy_index = 0;
	}
	/*
	 * when list count>30,the dana will give a time to get next list,
	 */
	info_count = list_count - cpy_index;
	if (!replay_list || (info_count <= 0)) {
		ak_print_error_ex("replay list is null\n");
		goto get_reclist_end;
	}
	info_count = (info_count > max_count) ? max_count : info_count;
	record_info = (libdanavideo_reclist_recordinfo_t *)calloc(1,
		(sizeof(libdanavideo_reclist_recordinfo_t) * info_count));
	if (!record_info) {
		ak_print_error_ex("record_info is null!!\n");
		*rec_num = 0;
		goto get_reclist_end;
	}

	cpy_num = copy_record_info(replay_list, record_info,
			rec_num, cpy_index, info_count);
	cpy_index += cpy_num;

get_reclist_end:
	ak_print_normal_ex("upload_num=%d cpy_index:%d list_count:%d rec_num:%d\n",
			cpy_num, cpy_index, list_count, *rec_num);
	/* last time to get, should free list */
	if ((cpy_index >= list_count) && (replay_list)) {
		ak_dvr_replay_free_fetch_list(replay_list);
		replay_list = NULL;
	}

    return record_info;
}

#if DANA_RECORD_REPLAY_DBG
static void dbg_record_data(void *param, int type, struct demux_stream *stream)
{
	static long video_count = 0;
	static long audio_count = 0;
	static unsigned long long v_ts_bak= 0 ;
	static unsigned long long a_ts_bak= 0 ;
	static int video_0_count = 0;    /* empty video frame  count*/

    if (type == 1) { /* video */
    	video_count++;
		//if (FRAME_TYPE_I == stream->frame_type)
		if (1) {
			ak_print_normal("V%s ts:%09llums diff:%llums %ld s:%d\n",
				   	(FRAME_TYPE_I == stream->frame_type) ? "I" :
					(FRAME_TYPE_B == stream->frame_type) ? "B" :"P",
			stream->ts,stream->ts - v_ts_bak,video_count,stream->len);
			v_ts_bak = stream->ts;
			if (stream->len == 8) 
				video_0_count++;

		}
    } else { /* audio */
		audio_count++;
		ak_print_normal("Ad ts:%09llums diff:%llums %ld s:%d\n",
			stream->ts,stream->ts - a_ts_bak,audio_count,stream->len);
		a_ts_bak = stream->ts;

    }
	if ((audio_count % 1000) == 1) {
		ak_print_normal_ex("video count:%ld audio_count:%ld video_0_count:%d\n",
				video_count, audio_count, video_0_count);
	}
}
#endif

static void send_record_data(void *param, int type,
		struct demux_stream *stream, int encode_type)
{
#if DANA_RECORD_REPLAY_DBG
	dbg_record_data(param, type, stream);
#endif
	if (type == 1) { /* video */
		/*platform type to dana type*/
		int dana_enc_type = (encode_type == VIDEO_DEMUX_H264) ? H264 : H265; //1, 4
		struct video_stream demux_vstream = {0}; /* demux video stream */
		/*  4bytes(frame ID) + 4bytes(timestamp) + frame length (data)
		 *	4bytes(frame ID) + 4bytes(timestamp) should donot upload to cloud platform
		 */
		demux_vstream.data = stream->data + 8;
		demux_vstream.frame_type = stream->frame_type;
		demux_vstream.len = stream->len - 8;
		demux_vstream.ts = stream->ts;
	    dana_cmd_send_video(param, &demux_vstream, dana_enc_type);
	} else { /* audio */
		/*platform type to dana type*/
		int dana_enc_type = (AK_AUDIO_TYPE_PCM_ALAW == encode_type) ? G711A : PCM;
	    struct audio_stream demux_astream = {0}; /* demux audio stream */
		demux_astream.data = stream->data;
		demux_astream.len = stream->len;
		demux_astream.ts = stream->ts;
	    dana_cmd_send_audio(param, &demux_astream, dana_enc_type);
	}
}

/**
 * dana_cmd_record_play - start local record video  to play
 * @start_time[IN]: the start time to play
 * @mydata[IN]:  connect private  info
 * return:   0 , success ;  -1 , failed;
 */
int dana_cmd_record_play(unsigned long start_time, void *mydata)
{
	unsigned long end_time;
	time_t tmp;
	struct tm dest_time;

	start_time += dana_time_zone;
	tmp = start_time;
	/* localtime_r is safe for thread */
	localtime_r(&tmp, &dest_time);
	dest_time.tm_hour = 0;
	dest_time.tm_sec = 0;
	dest_time.tm_min = 0;
	end_time = mktime(&dest_time) + 24*60*60;

	/* stop pre-replay */
	dana_cmd_stop_rec_play(mydata);

	struct video_record_config *record_param = ak_config_get_record();
	struct dvr_replay_request request = {0};
	request.start_time = (unsigned long)start_time;
	request.end_time = end_time;
	request.user_data = (void*)mydata;
	request.record_type = record_param->file_type;
	return ak_dvr_replay_start(&request, send_record_data);
}

/**
 * dana_cmd_stop_rec_play - stop local record video   play
 * @mydata[IN]: 	  connect private  info
 * return:   void;
 */
void dana_cmd_stop_rec_play(void *mydata)
{
	ak_dvr_replay_stop(mydata);
	if (replay_list != NULL) {
		ak_dvr_replay_free_fetch_list(replay_list);
		replay_list = NULL;
	}
}

void dana_cmd_pause_rec_play(void *mydata, int play)
{
	//ak_dvr_replay_play_status(mydata, play);
}

/**
 * dana_cmd_get_video_flip - get video flip status
 * return:    	video flip status;
 */
int dana_cmd_get_video_flip(void)
{
	return video_flip_tate;
}

/**
 * dana_cmd_set_video_flip - set video flip status
 * @vi_handle[IN]: vi handle
 * @state[IN]: 	 video flip status
 * return:    		0 , success ;  -1 , failed;
 */
int dana_cmd_set_video_flip(void *vi_handle,int state)
{
	if (video_flip_tate == state)
		return 0;

	ak_print_normal_ex("Set the flip %d \n", state);
	video_flip_tate = state;
	switch(video_flip_tate)
	{
		case 0:
			/*normal video*/
			ak_vi_set_flip_mirror(vi_handle, 0 , 0);
			break;
		case 1:
			/*horizontal mirror video*/
			ak_vi_set_flip_mirror(vi_handle, 0 , 1);
			break;
		case 2:
			/*vertical mirror video*/
			ak_vi_set_flip_mirror(vi_handle, 1 , 0);
			break;
		case 3:
			/*horizontal&vertical  mirror video*/
			ak_vi_set_flip_mirror(vi_handle, 1 , 1);
			break;
		default:
			break;
	}

	return 0;
}

/**
 * dana_cmd_get_freq - get power freq
 * @vi_handle[IN]: 	vi handle
 * return:    >0 , power freq ;  -1 , failed;
 */
int dana_cmd_get_freq( void *vi_handle)
{
	int ret = 0;
	int hz = 50;
	ret = ak_vpss_effect_get(vi_handle,VPSS_POWER_HZ,&hz);
	if( 0 == ret )
		return hz;
	else
		return ret;
}

/**
 * dana_cmd_set_freq - set power freq
 * @vi_handle[IN]: vi handle
 * @freq[IN]: 		freq
 * return:    		0 , success ;  -1 , failed;
 */
int dana_cmd_set_freq(void *vi_handle, int hz)
{
	int ret = ak_vpss_effect_set(vi_handle,VPSS_POWER_HZ,hz);
	return ret;
}

