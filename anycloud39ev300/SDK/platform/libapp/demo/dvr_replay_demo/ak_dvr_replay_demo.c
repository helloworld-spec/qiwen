#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "ak_common.h"
#include "ak_demux.h"
#include "ak_dvr_file.h"
#include "ak_dvr_replay.h"

#define REPLAY_DATA_SAVE 			0
#define PRINT_GET_REPLAY_LIST		1
#define DEFAULT_START 		        "20171020-155959"
#define DEFAULT_END 		        "20171120-155959"

static FILE *video_fp = NULL;
static FILE *audio_fp = NULL;
static int save_data = 0;
static int disp_list = 0;
static char date_str[100] = {0};
static int video_0_count = 0;    /* empty video frame  count*/

static void init_record_file(void)
{
	/* video record param init */
	struct dvr_file_param record;

	record.cyc_flag = 1;
	record.type = DVR_FILE_TYPE_AVI;
    strcpy(record.rec_prefix, RECORD_DEFAULT_PREFIX);
    strcpy(record.rec_path, RECORD_DEFAULT_PATH);
    ak_dvr_file_init(&record);
   	ak_dvr_replay_init();
}

static int get_replay_list(unsigned long start, unsigned long end)
{
   	ak_print_normal_ex("*** get file begin ***\n");
	unsigned long ak_start_time = start;
	unsigned long ak_end_time = end;

	ak_print_notice("request start_time: %s\n",
		ak_seconds_to_string(ak_start_time));
	ak_print_notice("request end_time: %s\n",
		ak_seconds_to_string(ak_end_time));

	int rec_num = 0;
	struct list_head *replay_list = NULL;
	struct dvr_replay_param param = {0};
	int list_count = 0;

	param.type = DVR_FILE_TYPE_AVI;
	param.start_time = ak_start_time;
	param.end_time = ak_end_time;


	list_count =  ak_dvr_replay_fetch_list(&param, &replay_list);;
	ak_print_notice_ex("list_count=%d\n", list_count);

	if ((replay_list && !list_empty(replay_list)) && list_count > 0) {
		if (disp_list){
			struct dvr_file_entry *entry = NULL;

			list_for_each_entry(entry, replay_list, list) {
				if (NULL != entry) {
					/* entry->total_time is ms */
					ak_print_normal("rec_num=%.3d, file: %s\n",
						++rec_num, entry->path);
				}
			}
		}
		ak_dvr_replay_free_fetch_list(replay_list);
	}

	return AK_SUCCESS;
}

static void write_replay_data(enum demux_data_type type,
									struct demux_stream *record_data)
{
	static unsigned long long ts_audio = 0;
	static unsigned long long ts_video = 0;

	/* save video data to file */
	if ((DEMUX_DATA_VIDEO == type) && video_fp) {
		ts_video = record_data->ts;
		fwrite(record_data->data, 1, record_data->len, video_fp);
	}

	/* save audio data to file */
	if ((DEMUX_DATA_AUDIO == type) && audio_fp) {
		ts_audio = record_data->ts;
		fwrite(record_data->data, 1, record_data->len, audio_fp);
	}

	/* check video ts and audio ts difference */
	if (ts_video > ts_audio) {
		if (ts_video - ts_audio > 2000)
			ak_print_error_ex("audio ts =%ld, video ts=%ld,diff=%ld\n",
			(unsigned long)ts_audio, (unsigned long)ts_video,
			(unsigned long)(ts_video - ts_audio));
	} else {
		if (ts_audio- ts_video > 2000)
			ak_print_error_ex("audio ts =%ld, video ts=%ld,diff=%ld\n",
			(unsigned long)ts_audio, (unsigned long)ts_video,
			(unsigned long)(ts_audio - ts_video));
	}
}

static void send_record_data(void *param, int type, struct demux_stream *stream, int encode_type)
{
	static long video_count = 0;
	static long audio_count = 0;
	static unsigned long long v_ts_bak= 0 ;	
	static unsigned long long a_ts_bak= 0 ;
	
 	if (save_data) {
		write_replay_data(type,	stream);
 	}
    if (type == 1) { /* video */
    	video_count++;
		ak_print_normal("V%s ts:%09llums diff:%llums %ld s:%d\n", (FRAME_TYPE_I == stream->frame_type)?"I":
		(FRAME_TYPE_B == stream->frame_type)?"B":"P",
		stream->ts,stream->ts - v_ts_bak,video_count,stream->len);			
		v_ts_bak = stream->ts;
		if(stream->len == 8) video_0_count++;			
    } else { /* audio */
       audio_count++;	   
		ak_print_normal("                                         Ad ts:%09llums diff:%llums %ld s:%d\n", 
			stream->ts,stream->ts - a_ts_bak,audio_count,stream->len);			
		a_ts_bak = stream->ts;	 
    }
	if ((audio_count % 1000) == 1){
		ak_print_normal_ex("video count:%ld audio_count:%ld video_0_count:%d\n", video_count, audio_count, video_0_count);
	}
}

static void usage(const char *name)
{
	ak_print_normal("Usage: %s [options]\n\n"
		"options:\n"
		"-d   start time and end time."
		"       example: 20171003-155959 \n"
		"-l   display record file list.\n"
		"-s   save replay data to /mnt.\n"
		"-h   help\n"
		"this program is to get record file list and do replay.\n"
		"record param get from /etc/jffs2/anyka_cfg.ini.\n"
		"example: ak_dvr_replay_demo -d 20171024-173456,20171124-173456\n"
		,name);
}

static int parse_args(int argc, char **argv)
{
	int ch;
	/* parse input options */
	while ((ch = getopt(argc, argv, "d:lsh")) != EOF) {
		switch (ch) {
		case 'd':
		case 'D':
			ak_print_normal_ex("date:%s\n", optarg);
			strncpy(date_str, optarg, 99);
			break;
		case 'l':
		case 'L':
			disp_list = 1;
			break;
		case 's':
		case 'S':
			save_data = 1;
			break;
		case 'h':
		case 'H':
		default:
			usage( argv[0] );
			exit( 0 );
			break;
		}
	}

	return 0;
}

int main (int argc, char **argv)
{
	unsigned long start_time = 0;
	unsigned long end_time = 0;

	ak_print_normal_ex("*** dvr replay demo start ***\n");
	memset(date_str, 0, 100);

	/* parse the arg which come from commad line*/
	parse_args(argc, argv);

	/* step 1 init dvr replay param  */
	init_record_file();
	ak_print_normal_ex("init_record OK\n");

	/* create record file list from card  */
	if (AK_SUCCESS == ak_dvr_file_create_list()) {
		ak_print_normal_ex("ak_dvr_file_fetch_list OK\n");
		ak_sleep_ms(2000);
	}

	if (0 == strlen(date_str)) {
		snprintf(date_str,99,"%s,%s", DEFAULT_START, DEFAULT_END);
	}
	struct ak_date date;
	char *time_ptr = strchr(date_str, ',');
	if (!time_ptr) {
		ak_print_error_ex("time failed\n");
		return AK_FAILED;
	}
	/* ak_print_normal_ex("%s\n", time_ptr); */
	ak_string_to_date(time_ptr + 1, &date);
	end_time = ak_date_to_seconds(&date);
	*time_ptr = 0;
	ak_string_to_date(date_str, &date);
	/* ak_print_normal_ex("%s\n", date_str); */
	start_time = ak_date_to_seconds(&date);

	/* get record file list according time  arrange  */
	get_replay_list(start_time, end_time);

	if (save_data) {
		if (!video_fp)
			video_fp = fopen("/mnt/video.h264", "w+");
		if (!video_fp)
			ak_print_error_ex("open /mnt/video.h264 failed\n");

		if (!audio_fp)
			audio_fp = fopen("/mnt/audio.g711a", "w+");
		if (!audio_fp)
			ak_print_error_ex("open /mnt/audio.g711a failed\n");
	}

	struct dvr_replay_request request = {0};

	request.start_time = (unsigned long)start_time;
	request.end_time = end_time;
	request.user_data = (void*)0;
	request.record_type = DVR_FILE_TYPE_AVI;

	/* start to play record file according time arrange */
    ak_dvr_replay_start(&request, send_record_data);
	ak_sleep_ms(20000);

	ak_print_normal_ex(" video_0_count:%d\n", video_0_count);
	
	/* stop to play record file  */
	ak_dvr_replay_stop((void*)0);
	/* step  exit  replay and release resource */
	ak_dvr_replay_exit();
	ak_print_normal_ex("*** dvr replay demo end ***\n");

	if (save_data) {
		if (video_fp) {
			fclose(video_fp);
			video_fp = NULL;
		}

		if (audio_fp) {
			fclose(audio_fp);
			audio_fp = NULL;
		}
	}

	return AK_SUCCESS;
}
