#ifndef _RECORD_CTRL_H_
#define _RECORD_CTRL_H_

#include "ak_dvr_common.h"

/**
 * record_ctrl_update_dvr_plan_time - tell dvr, dvr plan time have been update.
 * @ void:  
 * return: 0, success; -1, failed
 */
int record_ctrl_update_dvr_plan_time(void);

/**
 * record_ctrl_request_format_card - tell dvr to do format card.
 * @ void:  
 * return: 0, success; -1, failed
 */
int record_ctrl_request_format_card(void);

/**
 * record_ctrl_get_format_status - get status of format card.
 * @ void:
 * return: 0, success; 1, in progress; -1, failed
 */
int record_ctrl_get_format_status(void);

/** 
 * record_ctrl_set_mode - set record control mode. 
 * @new_mode[IN]:  new record control mode to be set
 * return: 0, success; -1, failed
 */
int record_ctrl_set_mode(enum dvr_record_type new_mode);

/** 
 * record_ctrl_init - init record control env and other sub-modules. 
 *      start record control thread.
 * @vi_handle[IN]: success opened video input handle
 * @ai_handle[IN]: success opened audio input handle
 * @file_type[IN]: record file type
 * return: none
 */
void record_ctrl_init(void *vi_handle, void *ai_handle,
					enum dvr_file_type file_type);

/** 
 * record_ctrl_exit - clear record control env and other sub-modules. 
 *      stop record control thread and release resource.
 * @void
 * return: none
 */
void record_ctrl_exit(void);

#endif
