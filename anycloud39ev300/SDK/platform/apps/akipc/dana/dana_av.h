#ifndef _DANA_AV_H_
#define _DANA_AV_H_

#include "list.h"

#include "ak_global.h"
#include "ak_thread.h"
#include "ak_vi.h"

#define MAX_USER 4
typedef void (*net_send_func)(void *param, void *pstream, int encode_type);
typedef void* (*get_data_func)(void *param);

typedef struct {
	void *cb_func;
	void *pri_data;
} cb_t;

enum dana_callback_id{
	DANA_PRE_VIDEO = 0,
	DANA_PRE_AUDIO,
	DANA_CLOUD_VIDEO,
	DANA_CLOUD_AUDIO,
	DANA_PLAY_AUDIO,
	CALLBACK_MAX
};

/**
 * ak_dana_init - init dana audio and video
 * @vi_handle[IN]: opened vi_handle
 * @ai_handle[IN]: opened ai_handle
 * @ao_handle[IN]: opened ao_handle
 * return: void
 */
int dana_av_init(void *vi_handle,void *ai_handle,void *ao_handle);

/**
 * dana_av_exit -  free dana audio and video resource and exit
 * return: void
 */
void dana_av_exit(void);

/**
 * dana_av_start_audio - start  audio  send to danale or receive from danale
 * @id[IN]: 		request function id
 * @func[IN]: 		audio  send  or receive function
 * @pri_data[IN]: 	private data for send  or receive function
 * return:   successful return 0 ;  fail return -1
 */
int dana_av_start_audio(enum dana_callback_id id, void *func, void *pri_data);

/**
 * dana_av_start_video - start  video  send to danale
 * @id[IN]: 		request function id
 * @func[IN]: 		video  send  function
 * @pri_data[IN]: 	private data for send   function
 * @quality[IN]: 	video quality [1-100]
 * return:   successful return frames ;  fail return 0
 */
int dana_av_start_video(enum dana_callback_id id, void *func, void *pri_data,
						int quality);

/**
 * dana_av_switch_gop - swtich video gop
 * return: void
 */
 void dana_av_switch_gop(void);

/**
 * dana_av_stop - stop  video or audio which  dana_av_start_audio( or video) request
 * @id[IN]: 		request function id
 * @pri_data[IN]: 	private data for send or receive  function
 * return:   successful return 0 ;  fail return -1
 */
int dana_av_stop(enum dana_callback_id id, void *pri_data);

int request_set_i_frame(void);


#endif
