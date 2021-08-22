#ifndef _RECORD_CTRL_H_
#define _RECORD_CTRL_H_

#include "ak_dvr_record.h"
#include "ak_config.h"
#include "ak_thread.h"

/* record plan action */
enum record_plan_action {
    RECORD_PLAN_NONE = 0X00,		//no record plan
    RECORD_PLAN_NO_CONFIG,			//record plan info not config
	RECORD_PLAN_START,				//record plan start
	RECORD_PLAN_END,				//record play end
};

unsigned char record_ctrl_is_recording(void);
void record_ctrl_pause_record(void);
void record_ctrl_resume_record(int trigger_type);
void record_ctrl_stop_record(void);

/**
 * record_ctrl_stop_detect - stop appointed detection
 * @detect_type[IN]: detection type 
 * return: none
 */
//void record_ctrl_stop_detect(enum sys_detect_type detect_type);

/** 
 * record_ctrl_init - init record control env and other sub-modules. 
 *      start record control thread.
 * @vi_handle[IN]: success opened video input handle
 * @ai_handle[IN]: success opened audio input handle
 * @file_type[IN]: record file type
 * return: none
 */
ak_pthread_t record_ctrl_init(void *vi_handle, void *ai_handle,
					enum record_file_type file_type , struct video_config * video_config);

/** 
 * record_ctrl_exit - clear record control env and other sub-modules. 
 *      stop record control thread and release resource.
 * @void
 * return: none
 */
void record_ctrl_exit(void);

#endif
