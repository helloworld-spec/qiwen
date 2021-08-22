#ifndef _AK_DVR_RECORD_H_
#define _AK_DVR_RECORD_H_

#include "ak_venc.h"
#include "ak_mux.h"

/* record trigger type */
enum record_trigger_type {
    RECORD_TRIGGER_TYPE_RESERVED = 0x00,        //record trigger type reserved
    RECORD_TRIGGER_TYPE_PLAN,                  	//plan record trigger
    RECORD_TRIGGER_TYPE_ALARM,                  //alarm record trigger
};

/* record status, each record modules should check current status */
enum record_status {
    RECORD_STATUS_INIT = 0X00,      //0, init status
    RECORD_STATUS_IDLE,             //idle: ready/wait record
    RECORD_STATUS_WORK,             //working
    RECORD_STATUS_SLEEP,            //sleep: record pause
    RECORD_STATUS_FILTER,           //filter: video & audio datastream
    RECORD_STATUS_EXCEPT,           //5, exception
    RECORD_STATUS_STOP,             //stop: record stop
    RECORD_STATUS_OP,             	//operation: record outside operation
    RECORD_STATUS_EXIT,             //exit: record exit
};

/* DVR stop operation type */
enum dvr_op_stop_type {
	DVR_OP_TYPE_RESERVED = 0x00,
	DVR_OP_TYPE_FORMAT_CARD,
	DVR_OP_TYPE_MODIFY_PLAN
};

typedef void (* FP_VIDEO_RECORD_ERROR)(int error_code, const char *error_desc);

struct record_param {
	void *vi_handle;		//video input handle
	void *ai_handle;		//audio input handle

	/* venc param */
	unsigned long width;	//real encode width, to be divisible by 4
	unsigned long height;	//real encode height, to be divisible by 4
	enum encode_use_chn enc_chn;	//encode use channel
	int file_fps;			//encode fps
	int file_bps;			//target bps
	int gop_len;			//GOP len

	/* aenc param */
	unsigned int sample_rate;
	int frame_interval;		//audio frame interval

	/* record file param */
	unsigned char cyc_flag;	//cycle record flag
	int file_type;			//save file type
    time_t duration;		//record duration time(ms)
    char rec_prefix[RECORD_FILE_PREFIX_MAX_LEN];//record file name prefix
    char rec_path[RECORD_FILE_PATH_MAX_LEN];//record file saving path

    FP_VIDEO_RECORD_ERROR error_report;
};

/**
 * ak_dvr_record_get_version - get dvr version
 * return: version string
 */
const char* ak_dvr_record_get_version(void);

/** 
 * ak_dvr_record_get_status - get current video record status
 * @void
 * return: current video record status
 */
enum record_status ak_dvr_record_get_status(void);

/** 
 * ak_dvr_record_pause
 * @void
 * return: none
 * notes: call by record_ctrl_pause_record
 */
void ak_dvr_record_pause(void);

/** 
 * ak_dvr_record_resume
 * @trigger_type: record trigger type
 * return: none
 * notes: call by record_ctrl_resume_record
 */
void ak_dvr_record_resume(int trigger_type);

/** 
 * ak_dvr_record_start - start video record
 * @void
 * return: none
 */
void ak_dvr_record_start(void);

/** 
 * ak_dvr_record_stop
 * @void
 * return: none
 * notes: call by record_ctrl_stop_record
 */
void ak_dvr_record_stop(void);

/** 
 * ak_dvr_record_op_stop - stop current record because of operating outside
 * @op_type[IN]: stop operation type
 * return: none
 * notes: operation such as SD card format, record plan change...
 */
void ak_dvr_record_op_stop(enum dvr_op_stop_type op_type);

/** 
 * ak_dvr_record_switch_fps
 * @switch_fps: fps value to be switched
 * return: none
 * notes: we'll switch the record frame between normal and min fps.
 */
void ak_dvr_record_switch_fps(int switch_fps);

/** 
 * ak_dvr_record_start_trigger - start alarm trigger
 * @trigger_time[IN]: alarm trigger time, we'll save stream filp 
 *			after alarm happened.
 * return: none
 * notes: we'll filter the video & audio datastream if no record triggered.
 *		can call by alarm detected
 */
void ak_dvr_record_start_trigger(int trigger_time);

/** 
 * ak_dvr_record_init - init video record env and start video record thread.
 * @param: video record param
 * return: 0 success; otherwise -1
 */
int ak_dvr_record_init(const struct record_param *param);

/** 
 * ak_dvr_record_exit - clear video record env and stop video record thread.
 * @void
 * return: none
 */
void ak_dvr_record_exit(void);

#endif
