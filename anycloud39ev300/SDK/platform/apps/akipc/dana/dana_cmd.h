#ifndef _DANA_CMD_H_
#define _DANA_CMD_H_

#include "ak_global.h"

#define DANA_TALK_FULL_MODE  0x80
#define DANA_MIC_ENABLE 0x40
#define DANA_SPEAK_ENABLE 0x20

/**
 * dana_cmd_get_freq - get power freq 
 * @vi_handle[IN]: 	vi handle  
 * return:    >0 , power freq ;  -1 , failed;
 */
int dana_cmd_get_freq( void *vi_handle );

/**
 * dana_cmd_set_freq - set power freq 
 * @vi_handle[IN]: vi handle  
 * @freq[IN]: 		freq  
 * return:    		0 , success ;  -1 , failed;
 */
int dana_cmd_set_freq(void *vi_handle, int freq);

/**
 * dana_cmd_get_video_flip - get video flip status   
 * return:    	video flip status;
 */
int dana_cmd_get_video_flip(void);

/**
 * dana_cmd_set_video_flip - set video flip status 
 * @vi_handle[IN]: vi handle  
 * @state[IN]: 	 video flip status
 * return:    		0 , success ;  -1 , failed;
 */
int dana_cmd_set_video_flip(void *vi_handle,int state);

/**
 * dana_cmd_get_alarm - get alarm configure 
 * @alarm_info[OUT]: 	alarm configure  
 * return:   void;
 */
void dana_cmd_get_alarm(DANAVIDEOCMD_SETALARM_ARG *alarm_info);

/**
 * dana_cmd_set_alarm - set alarm configure 
 * @alarm_info[IN]: 	alarm configure  
 * return:   void;
 */
void dana_cmd_set_alarm(DANAVIDEOCMD_SETALARM_ARG *alarm_info);

/**
 * dana_cmd_get_reclist - get record video list info 
 * @rec_info[IN]: 	param to search record video list info  
 * @rec_num[OUT]: record video number  
 * return:   record video list info;
 */
libdanavideo_reclist_recordinfo_t* dana_cmd_get_reclist(
			DANAVIDEOCMD_RECLIST_ARG *rec_info, int *rec_num);

/**
 * ak_get_sdcard_total_size - get sd card total size 
 * @path[IN]: 	 sd card mount point path   
 * return:   >0 ,   sd card total size;  -1 , failed;
 */
uint64_t ak_get_sdcard_total_size(char *path);

/**
 * ak_get_sdcard_free_size - get sd card free size 
 * @path[IN]: 	 sd card mount point path   
 * return:   >0 ,   sd card free size;  -1 , failed;
 */
uint64_t ak_get_sdcard_free_size(char *path);

/**
 * dana_cmd_g_rec_plan - get record video plan 
 * @plan_num[OUT]: 	 record video plan number   
 * return:   record video plan list or null ;
 */
libdanavideo_recplanget_recplan_t* dana_cmd_g_rec_plan(uint32_t *plan_num);

/**
 * dana_cmd_s_rec_plan - set record video plan 
 * @pnet_record_plan[IN]: 	 record video plan info   
 * return:   void;
 */
void dana_cmd_s_rec_plan(DANAVIDEOCMD_RECPLANSET_ARG *pnet_record_plan);

/**
 * dana_cmd_send_audio - send audio data to danale  
 * @param[IN]: 	 net  info  
 * @data[IN]: 	 audio data 
 * @encode_type[IN]: 	 audio encode type 
 * return:   void;
 */
void dana_cmd_send_audio(void *param, void *data, int encode_type);

/**
 * dana_cmd_get_talk_data - get audio data from danale  
 * @param[IN]: 	 net  info  
 * @data[IN]: 	 audio data 
 * return:    audio data or  null ;
 */
void *dana_cmd_get_talk_data(void *param );

/**
 * dana_cmd_send_video - send video data to danale  
 * @param[IN]: 	 net  info  
 * @data[IN]: 	 video data 
 * @encode_type[IN]: 	 video encode type 
 * return:   void;
 */
void dana_cmd_send_video(void *param, void *data, int encode_type);

/**
 * dana_cmd_record_play - start local record video  to play  
 * @start_time[IN]: the start time to play   
 * @mydata[IN]: 	  connect private  info
 * return:   0 , success ;  -1 , failed;
 */
int dana_cmd_record_play(unsigned long start_time, void *mydata);

/**
 * dana_cmd_pause_rec_play - not impliment  
 */
void dana_cmd_pause_rec_play(void *mydata, int play);

/**
 * dana_cmd_stop_rec_play - stop local record video   play   
 * @mydata[IN]: 	  connect private  info
 * return:   void;
 */
void dana_cmd_stop_rec_play(void *mydata);

#endif
